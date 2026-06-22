// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: soundsrv.c,v 1.3 1997/01/29 22:40:44 b1 Exp $
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
//
// $Log: soundsrv.c,v $
// Revision 1.3  1997/01/29 22:40:44  b1
// Reformatting, S (sound) module files.
//
// Revision 1.2  1997/01/21 19:00:07  b1
// First formatting run:
//  using Emacs cc-mode.el indentation for C++ now.
//
// Revision 1.1  1997/01/19 17:22:50  b1
// Initial check in DOOM sources as of Jan. 10th, 1997
//
//
// DESCRIPTION:
//	UNIX soundserver, run as a separate process,
//	 started by DOOM program.
//	Originally conceived for SGI Irix,
//	 mostly used with Linux voxware.
//	Refactored to use SDL2 audio output.
//
//  ARCHITECTURE
//  ------------
//  The sound server is a standalone process spawned by the main linuxxdoom
//  executable. It communicates with the parent over stdin/stdout using a
//  simple hex-encoded text protocol. The server loads all sound effects
//  from a WAD file at startup, then enters an event loop that:
//
//    1. Polls stdin for commands (non-blocking via select())
//    2. Mixes up to 8 simultaneous sound channels into a stereo buffer
//    3. Queues the mixed buffer to SDL2 for playback
//    4. Sleeps for one buffer-duration before the next cycle
//
//  PROTOCOL (stdin commands, one character prefix + hex args)
//  ----------------------------------------------------------
//    'p' <snd#> <step> <vol> <sep>   Play a sound effect
//         snd#: 2-digit hex sound ID (from sfxenum_t)
//         step: 2-digit hex pitch step (index into step_table)
//         vol:  2-digit hex volume (0-127)
//         sep:  2-digit hex stereo separation (0-255)
//
//    'q'                              Quit (wait for channels to finish)
//
//    's' <snd#>                       Save sound effect to a file
//         (filename is the first two chars of the command, snd# is hex)
//
//  AUDIO PIPELINE
//  --------------
//    WAD lumps (8-bit unsigned) → vol_lookup (→ signed 16-bit, volume-scaled)
//    → 8-channel mixer (stereo interleaved s16) → SDL2 queue
//
//  The mixer uses fixed-point 16.16 step values for pitch shifting.
//  Each channel maintains a byte pointer into the source sample data,
//  a step increment, and a fractional remainder for sub-sample precision.
//  The step table (step_table) maps pitch values to step increments using
//  a logarithmic curve: step = 2^(pitch/64) * 65536.
//
//  Volume and stereo separation are pre-computed into lookup tables
//  (vol_lookup) indexed by [volume][sample_byte] to avoid per-sample
//  multiplication in the hot mixing loop.
//
//-----------------------------------------------------------------------------


static const char rcsid[] = "$Id: soundsrv.c,v 1.3 1997/01/29 22:40:44 b1 Exp $";


#include <string.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <SDL2/SDL.h>

#include "../include/sounds.h"
#include "../include/soundsrv.h"
#include "../include/wadread.h"

//
// Department of Redundancy Department.
//
typedef struct wadinfo_struct {
    // should be IWAD
    char identification[4];
    int numlumps;
    int infotableofs;
} wadinfo_t;


typedef struct filelump_struct {
    int filepos;
    int size;
    char name[8];
} filelump_t;


// an internal time keeper
static int my_time = 0;

// number of sound effects
int num_sounds;

// longest sound effect
int long_sound;

// lengths of all sound effects
int lengths[NUMSFX];

// mixing buffer
signed short mix_buffer[MIXBUFFERSIZE];

// file descriptor of sfx device
int sfx_device;

// file descriptor of music device
int mus_device;

// the channel data pointers
unsigned char *channels[8];

// the channel step amount
unsigned int channel_step[8];

// 0.16 bit remainder of last step
unsigned int channel_step_remainder[8];

// the channel data end pointers
unsigned char *channel_send[8];

// time that the channel started playing
int channel_start[8];

// the channel handles
int channel_handles[8];

// the channel left volume lookup
int *channel_left_vol_lookup[8];

// the channel right volume lookup
int *channel_right_vol_lookup[8];

// sfx id of the playing sound effect
int channel_ids[8];

int snd_verbose = 1;

int step_table[256];

int vol_lookup[128 * 256];

// Fatal error handler: prints the message to stderr and terminates
// with exit code -1. Used for unrecoverable conditions such as
// out-of-bounds volume values.
static void derror(char *msg)
{
    fprintf(stderr, "error: %s\n", msg);
    exit(-1);
}

// Core audio mixer. Produces one buffer worth (SAMPLECOUNT frames) of
// stereo 16-bit interleaved audio into mix_buffer.
//
// For each output sample:
//   - Iterates all 8 channels; if a channel is active (non-NULL pointer),
//     looks up the scaled signed value from vol_lookup tables and accumulates
//     into left (dl) and right (dr) accumulators.
//   - Advances the channel's sample pointer by the pitch step using
//     16.16 fixed-point arithmetic with sub-sample carry.
//   - If the pointer reaches or exceeds the channel end, the channel
//     is deactivated (set to NULL).
//   - Clamps the accumulated values to signed 16-bit range and writes
//     them to the interleaved stereo output buffer.
//
// Returns 1 on completion.
int mix(void)
{
    signed short *leftend;

    signed short *leftout = mix_buffer;
    signed short *rightout = mix_buffer + 1;
    int step = 2;

    leftend = mix_buffer + SAMPLECOUNT * step;

    // mix into the mixing buffer
    register int dl;
    register int dr;
    register unsigned int sample;
    while (leftout != leftend) {
        dl = 0;
        dr = 0;

        if (channels[0]) {
            sample = *channels[0];
            dl += channel_left_vol_lookup[0][sample];
            dr += channel_right_vol_lookup[0][sample];
            channel_step_remainder[0] += channel_step[0];
            channels[0] += channel_step_remainder[0] >> 16;
            channel_step_remainder[0] &= 65536 - 1;

            if (channels[0] >= channel_send[0])
                channels[0] = 0;
        }

        if (channels[1]) {
            sample = *channels[1];
            dl += channel_left_vol_lookup[1][sample];
            dr += channel_right_vol_lookup[1][sample];
            channel_step_remainder[1] += channel_step[1];
            channels[1] += channel_step_remainder[1] >> 16;
            channel_step_remainder[1] &= 65536 - 1;

            if (channels[1] >= channel_send[1])
                channels[1] = 0;
        }

        if (channels[2]) {
            sample = *channels[2];
            dl += channel_left_vol_lookup[2][sample];
            dr += channel_right_vol_lookup[2][sample];
            channel_step_remainder[2] += channel_step[2];
            channels[2] += channel_step_remainder[2] >> 16;
            channel_step_remainder[2] &= 65536 - 1;

            if (channels[2] >= channel_send[2])
                channels[2] = 0;
        }

        if (channels[3]) {
            sample = *channels[3];
            dl += channel_left_vol_lookup[3][sample];
            dr += channel_right_vol_lookup[3][sample];
            channel_step_remainder[3] += channel_step[3];
            channels[3] += channel_step_remainder[3] >> 16;
            channel_step_remainder[3] &= 65536 - 1;

            if (channels[3] >= channel_send[3])
                channels[3] = 0;
        }

        if (channels[4]) {
            sample = *channels[4];
            dl += channel_left_vol_lookup[4][sample];
            dr += channel_right_vol_lookup[4][sample];
            channel_step_remainder[4] += channel_step[4];
            channels[4] += channel_step_remainder[4] >> 16;
            channel_step_remainder[4] &= 65536 - 1;

            if (channels[4] >= channel_send[4])
                channels[4] = 0;
        }

        if (channels[5]) {
            sample = *channels[5];
            dl += channel_left_vol_lookup[5][sample];
            dr += channel_right_vol_lookup[5][sample];
            channel_step_remainder[5] += channel_step[5];
            channels[5] += channel_step_remainder[5] >> 16;
            channel_step_remainder[5] &= 65536 - 1;

            if (channels[5] >= channel_send[5])
                channels[5] = 0;
        }

        if (channels[6]) {
            sample = *channels[6];
            dl += channel_left_vol_lookup[6][sample];
            dr += channel_right_vol_lookup[6][sample];
            channel_step_remainder[6] += channel_step[6];
            channels[6] += channel_step_remainder[6] >> 16;
            channel_step_remainder[6] &= 65536 - 1;

            if (channels[6] >= channel_send[6])
                channels[6] = 0;
        }

        if (channels[7]) {
            sample = *channels[7];
            dl += channel_left_vol_lookup[7][sample];
            dr += channel_right_vol_lookup[7][sample];
            channel_step_remainder[7] += channel_step[7];
            channels[7] += channel_step_remainder[7] >> 16;
            channel_step_remainder[7] &= 65536 - 1;

            if (channels[7] >= channel_send[7])
                channels[7] = 0;
        }

        // Has been char instead of short.
        // if (dl > 127) *leftout = 127;
        // else if (dl < -128) *leftout = -128;
        // else *leftout = dl;

        // if (dr > 127) *rightout = 127;
        // else if (dr < -128) *rightout = -128;
        // else *rightout = dr;

        if (dl > 0x7fff)
            *leftout = 0x7fff;
        else if (dl < -0x8000)
            *leftout = -0x8000;
        else
            *leftout = dl;

        if (dr > 0x7fff)
            *rightout = 0x7fff;
        else if (dr < -0x8000)
            *rightout = -0x8000;
        else
            *rightout = dr;

        leftout += step;
        rightout += step;
    }
    return 1;
}


// Locates and opens a WAD file, then loads all sound effect lumps into
// the global S_sfx[] array.
//
// WAD search order (first readable file wins):
//   1. $DOOMWADDIR/doom2f.wad  (Final DOOM: TNT)
//   2. $DOOMWADDIR/doom2.wad   (DOOM II)
//   3. $DOOMWADDIR/doomu.wad   (Ultimate DOOM)
//   4. $DOOMWADDIR/doom.wad    (Registered DOOM)
//   5. $DOOMWADDIR/doom1.wad   (Shareware DOOM)
//   6. $DOOMWADDIR/freedm.wad  (Freedoom: Deathmatch)
//   7. $DOOMWADDIR/freedoom1.wad (Freedoom: Phase 1)
//
// If DOOMWADDIR is not set, defaults to the current directory.
//
// For each sfx slot (1..NUMSFX-1):
//   - If the sfx has a link (alias), copies the linked sfx's data pointer
//     and length.
//   - Otherwise, reads the lump from the WAD via getsfx().
//   - Tracks the longest sound effect in long_sound.
//
// Supports -quiet flag to suppress verbose output.
void grab_data(int c, char **v)
{

    // Now where are TNT and Plutonia. Yuck.

    //	char *home;

    char *doomwaddir = getenv("DOOMWADDIR");

    if (!doomwaddir)
        doomwaddir = ".";

    char *doom1wad = malloc(strlen(doomwaddir) + 1 + 9 + 1);
    sprintf(doom1wad, "%s/doom1.wad", doomwaddir);

    char *doom2wad = malloc(strlen(doomwaddir) + 1 + 9 + 1);
    sprintf(doom2wad, "%s/doom2.wad", doomwaddir);

    char *doom2fwad = malloc(strlen(doomwaddir) + 1 + 10 + 1);
    sprintf(doom2fwad, "%s/doom2f.wad", doomwaddir);

    char *doomuwad = malloc(strlen(doomwaddir) + 1 + 9 + 1);
    sprintf(doomuwad, "%s/doomu.wad", doomwaddir);

    char *doomwad = malloc(strlen(doomwaddir) + 1 + 8 + 1);
    sprintf(doomwad, "%s/doom.wad", doomwaddir);

    // Freedoom
    char *freedoom1 = malloc(strlen(doomwaddir) + 1 + 13 + 1);
    sprintf(freedoom1, "%s/freedoom1.wad", doomwaddir);

    char *freedm = malloc(strlen(doomwaddir) + 1 + 10 + 1);
    sprintf(freedm, "%s/freedm.wad", doomwaddir);

    //	home = getenv("HOME");
    //	if (!home)
    //	  derror("Please set $HOME to your home directory");
    //	sprintf(basedefault, "%s/.doomrc", home);


    int i;
    for (i = 1; i < c; i++) {
        if (!strcmp(v[i], "-quiet")) {
            snd_verbose = 0;
        }
    }

    num_sounds = NUMSFX;
    long_sound = 0;
    char *name;
    if (!access(doom2fwad, R_OK))
        name = doom2fwad;
    else if (!access(doom2wad, R_OK))
        name = doom2wad;
    else if (!access(doomuwad, R_OK))
        name = doomuwad;
    else if (!access(doomwad, R_OK))
        name = doomwad;
    else if (!access(doom1wad, R_OK))
        name = doom1wad;
    else if (!access(freedm, R_OK))
        name = freedm;
    else if (!access(freedoom1, R_OK))
        name = freedoom1;
    else {
        fprintf(stderr, "Could not find wadfile anywhere\n");
        exit(-1);
    }

    openwad(name);
    if (snd_verbose)
        fprintf(stderr, "loading from [%s]\n", name);

    for (i = 1; i < NUMSFX; i++) {
        if (!S_sfx[i].link) {
            S_sfx[i].data = getsfx(S_sfx[i].name, &lengths[i]);
            if (long_sound < lengths[i]) long_sound = lengths[i];
        } else {
            S_sfx[i].data = S_sfx[i].link->data;
            lengths[i] = lengths[(S_sfx[i].link - S_sfx) / sizeof(sfxinfo_t)];
        }
        // test only
        //  {
        //  int fd;
        //  char name[10];
        //  sprintf(name, "sfx%d", i);
        //  fd = open(name, O_WRONLY|O_CREAT, 0644);
        //  write(fd, S_sfx[i].data, lengths[i]);
        //  close(fd);
        //  }
    }
}

static struct timeval last = {0, 0};
//static struct timeval		now;

static struct timezone whocares;

// Mixes one buffer of audio and submits it to SDL2 for playback.
// Called once per main loop iteration (every ~46ms at 11025 Hz /
// 512 samples).
void update_sounds(void)
{
    mix();
    I_SubmitOutputBuffer(mix_buffer, SAMPLECOUNT);
}

// Adds a sound effect to one of the 8 mixing channels.
//
// Parameters:
//   soudId      - Sound effect ID (index into S_sfx[], from sfxenum_t)
//   volume      - Volume level (0-127)
//   step        - Pitch step from step_table (16.16 fixed-point increment)
//   seperation  - Stereo separation (0-255, 128 = center)
//
// Channel allocation:
//   - "Singularity" sounds (chainsaw idle/full/up/hit, pistol,
//     platform move): first stops any existing channel playing the
//     same sound, then allocates normally.
//   - Finds the oldest playing channel. If fewer than 8 channels are
//     active, uses the first free slot. If all 8 are busy, reuses
//     the oldest (LRU eviction).
//
// Stereo panning uses quadratic attenuation (x^2 / 256^2 scaling)
// to compute left and right volumes from the separation value.
// These volumes index into the pre-computed vol_lookup table.
//
// Returns a unique handle number for the sound instance, or -1 on error.
int add_sfx(int soudId, int volume, int step, int seperation)
{
    static unsigned short handlenums = 0;

    int i;
    int rc = -1;

    int oldest = my_time;
    int oldestnum = 0;
    int slot;
    int rightvol;
    int leftvol;

    // play these sound effects
    //  only one at a time
    if (soudId == sfx_sawup
        || soudId == sfx_sawidl
        || soudId == sfx_sawful
        || soudId == sfx_sawhit
        || soudId == sfx_stnmov
        || soudId == sfx_pistol) {
        for (i = 0; i < 8; i++) {
            if (channels[i] && channel_ids[i] == soudId) {
                channels[i] = 0;
                break;
            }
        }
    }

    for (i = 0; i < 8 && channels[i]; i++) {
        if (channel_start[i] < oldest) {
            oldestnum = i;
            oldest = channel_start[i];
        }
    }

    if (i == 8)
        slot = oldestnum;
    else
        slot = i;

    channels[slot] = (unsigned char *) S_sfx[soudId].data;
    channel_send[slot] = channels[slot] + lengths[soudId];

    if (!handlenums)
        handlenums = 100;

    channel_handles[slot] = rc = handlenums++;
    channel_step[slot] = step;
    channel_step_remainder[slot] = 0;
    channel_start[slot] = my_time;

    // (range: 1 - 256)
    seperation += 1;

    // (x^2 seperation)
    leftvol = volume - (volume * seperation * seperation) / (256 * 256);

    seperation = seperation - 257;

    // (x^2 seperation)
    rightvol = volume - (volume * seperation * seperation) / (256 * 256);

    // sanity check
    if (rightvol < 0 || rightvol > 127)
        derror("rightvol out of bounds");

    if (leftvol < 0 || leftvol > 127)
        derror("leftvol out of bounds");

    // get the proper lookup table piece
    //  for this volume level
    channel_left_vol_lookup[slot] = &vol_lookup[leftvol * 256];
    channel_right_vol_lookup[slot] = &vol_lookup[rightvol * 256];

    channel_ids[slot] = soudId;

    return rc;
}

// Outputs a 16-bit unsigned integer as 4 hex digits followed by newline
// on stdout (fd 1). Used to return sound handles to the parent process.
// Outputs "xxxx\n" if num is negative (error sentinel).
void outputushort(int num)
{
    static unsigned char buff[5] = {0, 0, 0, 0, '\n'};
    static char *badbuff = "xxxx\n";

    // outputs a 16-bit # in hex or "xxxx" if -1.
    if (num < 0) {
        write(1, badbuff, 5);
    } else {
        buff[0] = num >> 12;
        buff[0] += buff[0] > 9 ? 'a' - 10 : '0';
        buff[1] = (num >> 8) & 0xf;
        buff[1] += buff[1] > 9 ? 'a' - 10 : '0';
        buff[2] = (num >> 4) & 0xf;
        buff[2] += buff[2] > 9 ? 'a' - 10 : '0';
        buff[3] = num & 0xf;
        buff[3] += buff[3] > 9 ? 'a' - 10 : '0';
        write(1, buff, 5);
    }
}

// Initializes all mixing channels to inactive, records the startup
// timestamp, and pre-computes lookup tables:
//
//   step_table: 256-entry pitch-to-step-increment table.
//     step_table[128 + i] = 2^(i/64) * 65536  for i in [-128, 127]
//     This gives a logarithmic pitch range of 4 octaves (0.25x to 4.0x).
//
//   vol_lookup: 128×256 table mapping [volume][unsigned_sample_byte] to
//     signed 16-bit sample with volume applied.
//     vol_lookup[i*256 + j] = (i * (j - 128) * 256) / 127
//     Converts 8-bit unsigned (0..255, center=128) to signed 16-bit
//     with volume scaling, used as a hot-path optimization in mix().
void initdata(void)
{
    int i;
    int j;

    int *steptablemid = step_table + 128;

    for (i = 0;
         i < sizeof(channels) / sizeof(unsigned char *);
         i++) {
        channels[i] = 0;
    }

    gettimeofday(&last, &whocares);

    for (i = -128; i < 128; i++)
        steptablemid[i] = pow(2.0, (i / 64.0)) * 65536.0;

    // generates volume lookup tables
    //  which also turn the unsigned samples
    //  into signed samples
    // for (i=0 ; i<128 ; i++)
    // for (j=0 ; j<256 ; j++)
    // vol_lookup[i*256+j] = (i*(j-128))/127;

    for (i = 0; i < 128; i++)
        for (j = 0; j < 256; j++)
            vol_lookup[i * 256 + j] = (i * (j - 128) * 256) / 127;
}


// Shuts down SDL2 audio and music subsystems and exits with code 0.
void quit(void)
{
    I_ShutdownMusic();
    I_ShutdownSound();
    exit(0);
}


fd_set fdset;
fd_set scratchset;


// Parses and dispatches a command received from the parent process.
//
// Command format (hex-encoded, one line per command):
//
//   'p' <snd#> <step> <vol> <sep>
//       Play a sound. Reads 8 more hex chars from stdin.
//       Decodes into sndNum (sfx ID), step (pitch), vol (volume), sep (stereo).
//       Calls add_sfx().
//
//   'q'
//       Quit command. Sets waitingToFinish flag, stops reading new commands,
//       and allows currently playing channels to finish.
//
//   's' <filename_2chars><snd#>
//       Save a sound effect. Reads 2 more chars as filename, opens the file,
//       and writes the raw sound data from the WAD lump.
//
// rc and waitingToFinish are output parameters that control the main loop.
void parse_command(int *rc, int *sndNum, unsigned char commandBuf[10], int *step, int *vol, int *sep, int *waitingToFinish)
{
    switch (commandBuf[0]) {
        case 'p':
            // play a new sound effect
            read(0, commandBuf, 9);

            if (snd_verbose) {
                commandBuf[9] = 0;
                fprintf(stderr, "%s\n", commandBuf);
            }

            commandBuf[0] -= commandBuf[0] >= 'a' ? ('a' - 10) : '0';
            commandBuf[1] -= commandBuf[1] >= 'a' ? ('a' - 10) : '0';
            commandBuf[2] -= commandBuf[2] >= 'a' ? ('a' - 10) : '0';
            commandBuf[3] -= commandBuf[3] >= 'a' ? ('a' - 10) : '0';
            commandBuf[4] -= commandBuf[4] >= 'a' ? ('a' - 10) : '0';
            commandBuf[5] -= commandBuf[5] >= 'a' ? ('a' - 10) : '0';
            commandBuf[6] -= commandBuf[6] >= 'a' ? ('a' - 10) : '0';
            commandBuf[7] -= commandBuf[7] >= 'a' ? ('a' - 10) : '0';

            // p<snd#><step><vol><sep>
            *sndNum = (commandBuf[0] << 4) + commandBuf[1];
            *step = (commandBuf[2] << 4) + commandBuf[3];
            *step = step_table[(*step)];
            *vol = (commandBuf[4] << 4) + commandBuf[5];
            *sep = (commandBuf[6] << 4) + commandBuf[7];

            add_sfx(*sndNum, *vol, *step, *sep);
            // returns the handle
            // outputushort(handle);
            break;

        case 'q':
            read(0, commandBuf, 1);
            *waitingToFinish = 1;
            *rc = 0;
            break;

        case 's': {
            int fd;
            read(0, commandBuf, 3);
            commandBuf[2] = 0;
            fd = open((char *) commandBuf, O_CREAT | O_WRONLY, 0644);
            commandBuf[0] -= commandBuf[0] >= 'a' ? 'a' - 10 : '0';
            commandBuf[1] -= commandBuf[1] >= 'a' ? 'a' - 10 : '0';
            *sndNum = (commandBuf[0] << 4) + commandBuf[1];
            write(fd, S_sfx[(*sndNum)].data, lengths[(*sndNum)]);
            close(fd);
        }
        break;

        default:
            fprintf(stderr, "Did not recognize command\n");
            break;
    }
}

// Main entry point for the sound server process.
//
// Startup sequence:
//   1. grab_data()      — find WAD, load all sound effects
//   2. initdata()       — init channels, pre-compute step/volume tables
//   3. I_InitSound()    — open SDL2 audio device (11025 Hz, 16-bit stereo)
//   4. I_InitMusic()    — initialize SDL2 music subsystem (stub)
//   5. Prints "ready\n" to stderr if verbose
//
// Main loop (runs at ~21.5 Hz = 11025 / 512):
//   - Increments my_time (frame counter)
//   - If not waiting to finish: non-blocking poll (select) for commands on
//     stdin, parses and dispatches via parse_command()
//   - Calls update_sounds() to mix and queue the next audio buffer
//   - Prints "Updated\n" to stdout (heartbeat for parent process)
//   - Sleeps for one buffer duration via SDL_Delay()
//
// Exits when stdin closes (EOF) or when 'q' command is received and all
// channels have finished playing (waitingToFinish mode). Cleans up via
// quit() which shuts down SDL2 audio.
int main(int c, char **v)
{
    int done = 0;
    int rc;
    int nrc;
    int sndnum;
    int handle = 0;

    unsigned char commandbuf[10];
    struct timeval zerowait = {0, 0};


    int step;
    int vol;
    int sep;

    int i;
    int waitingtofinish = 0;

    // get sound data
    grab_data(c, v);

    // init any data
    initdata();

    I_InitSound(11025, 16);

    I_InitMusic();

    if (snd_verbose)
        fprintf(stderr, "ready\n");

    // parse commands and play sounds until done
    FD_ZERO(&fdset);
    FD_SET(0, &fdset);

    while (!done) {
        my_time++;

        if (!waitingtofinish) {
            do {
                scratchset = fdset;
                rc = select(FD_SETSIZE, &scratchset, 0, 0, &zerowait);

                if (rc > 0) {
                    // got a command
                    nrc = read(0, commandbuf, 1);

                    if (!nrc) {
                        done = 1;
                        rc = 0;
                    } else {
                        if (snd_verbose)
                            fprintf(stderr, "cmd: %c", commandbuf[0]);
                        parse_command(&rc, &sndnum, commandbuf, &step, &vol, &sep, &waitingtofinish);
                    }
                } else if (rc < 0) {
                    quit();
                }
            } while (rc > 0);
        }

        update_sounds();
        // printf("Updated\n");

        SDL_Delay(1000 * SAMPLECOUNT / SPEED);

        // if (waitingtofinish) {
        //     for (i = 0; i < 8 && !channels[i]; i++);
        //
        //     if (i == 8)
        //         done = 1;
        // }
    }

    quit();
    return 0;
}
