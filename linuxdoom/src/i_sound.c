// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// DOOM Source Code License for more details.
//
// $Log:$
//
// DESCRIPTION:
//	System interface for sound.
//      Refactored to use SDL2 audio instead of OSS /dev/dsp.
//
//-----------------------------------------------------------------------------

static const char rcsid[] = "$Id: i_unix.c,v 1.5 1997/02/03 22:45:10 b1 Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <math.h>

#include <sys/time.h>
#include <sys/types.h>

#include <fcntl.h>
#include <unistd.h>

#include <SDL2/SDL.h>

#include "../include/z_zone.h"

#include "../include/i_system.h"
#include "../include/i_sound.h"
#include "../include/m_argv.h"
#include "../include/m_misc.h"
#include "../include/w_wad.h"

#include "../include/doomdef.h"

#ifdef SNDSERV
FILE*	sndserver=0;
char*	sndserver_filename = "./sndserv ";
#endif

static int flag = 0;


#define SAMPLECOUNT		512
#define NUM_CHANNELS		8
#define BUFMUL                  4
#define MIXBUFFERSIZE		(SAMPLECOUNT*BUFMUL)

#define SAMPLERATE		11025
#define SAMPLESIZE		2


int 		lengths[NUMSFX];

static SDL_AudioDeviceID audio_device = 0;

signed short	mixbuffer[MIXBUFFERSIZE];


unsigned int	channelstep[NUM_CHANNELS];
unsigned int	channelstepremainder[NUM_CHANNELS];


unsigned char*	channels[NUM_CHANNELS];
unsigned char*	channelsend[NUM_CHANNELS];


int		channelstart[NUM_CHANNELS];

int 		channelhandles[NUM_CHANNELS];

int		channelids[NUM_CHANNELS];			

int		steptable[256];

int		vol_lookup[128*256];

int*		channelleftvol_lookup[NUM_CHANNELS];
int*		channelrightvol_lookup[NUM_CHANNELS];


void myioctl(int fd, int command, int *arg)
{   
}


void *getsfx(char *sfxname, int *len)
{
    unsigned char*      sfx;
    unsigned char*      paddedsfx;
    int                 i;
    int                 size;
    int                 paddedsize;
    char                name[20];
    int                 sfxlump;

    
    sprintf(name, "ds%s", sfxname);

    if ( W_CheckNumForName(name) == -1 )
      sfxlump = W_GetNumForName("dspistol");
    else
      sfxlump = W_GetNumForName(name);
    
    size = W_LumpLength( sfxlump );

    sfx = (unsigned char*)W_CacheLumpNum( sfxlump, PU_STATIC );

    paddedsize = ((size-8 + (SAMPLECOUNT-1)) / SAMPLECOUNT) * SAMPLECOUNT;

    paddedsfx = (unsigned char*)Z_Malloc( paddedsize+8, PU_STATIC, 0 );

    memcpy(  paddedsfx, sfx, size );
    for (i=size ; i<paddedsize+8 ; i++)
        paddedsfx[i] = 128;

    Z_Free( sfx );
    
    *len = paddedsize;

    return (void *) (paddedsfx + 8);
}


int addsfx(int sfxid, int volume, int step, int seperation)
{
    static unsigned short	handlenums = 0;
 
    int		i;
    int		rc = -1;
    
    int		oldest = gametic;
    int		oldestnum = 0;
    int		slot;

    int		rightvol;
    int		leftvol;

    if ( sfxid == sfx_sawup
	 || sfxid == sfx_sawidl
	 || sfxid == sfx_sawful
	 || sfxid == sfx_sawhit
	 || sfxid == sfx_stnmov
	 || sfxid == sfx_pistol	 )
    {
	for (i=0 ; i<NUM_CHANNELS ; i++)
	{
	    if ( (channels[i])
		 && (channelids[i] == sfxid) )
	    {
		channels[i] = 0;
		break;
	    }
	}
    }

    for (i=0; (i<NUM_CHANNELS) && (channels[i]); i++)
    {
	if (channelstart[i] < oldest)
	{
	    oldestnum = i;
	    oldest = channelstart[i];
	}
    }

    if (i == NUM_CHANNELS)
	slot = oldestnum;
    else
	slot = i;

    channels[slot] = (unsigned char *) S_sfx[sfxid].data;
    channelsend[slot] = channels[slot] + lengths[sfxid];

    if (!handlenums)
	handlenums = 100;

    channelhandles[slot] = rc = handlenums++;

    channelstep[slot] = step;
    channelstepremainder[slot] = 0;
    channelstart[slot] = gametic;

    seperation += 1;

    leftvol =
	volume - ((volume*seperation*seperation) >> 16);
    seperation = seperation - 257;
    rightvol =
	volume - ((volume*seperation*seperation) >> 16);	

    if (rightvol < 0 || rightvol > 127)
	I_Error("rightvol out of bounds");
    
    if (leftvol < 0 || leftvol > 127)
	I_Error("leftvol out of bounds");
    
    channelleftvol_lookup[slot] = &vol_lookup[leftvol*256];
    channelrightvol_lookup[slot] = &vol_lookup[rightvol*256];

    channelids[slot] = sfxid;

    return rc;
}





void I_SetChannels()
{
  int		i;
  int		j;
    
  int*	steptablemid = steptable + 128;
  
  for (i=-128 ; i<128 ; i++)
    steptablemid[i] = (int)(pow(2.0, (i/64.0))*65536.0);
  
  
  for (i=0 ; i<128 ; i++)
    for (j=0 ; j<256 ; j++)
      vol_lookup[i*256+j] = (i*(j-128)*256)/127;
}	
 
void I_SetSfxVolume(int volume)
{
  snd_SfxVolume = volume;
}

void I_SetMusicVolume(int volume)
{
  snd_MusicVolume = volume;
}


int I_GetSfxLumpNum(sfxinfo_t* sfx)
{
    char namebuf[9];
    sprintf(namebuf, "ds%s", sfx->name);
    return W_GetNumForName(namebuf);
}

int I_StartSound(int id, int vol, int sep, int pitch, int priority)
{

  priority = 0;
  
#ifdef SNDSERV 
    if (sndserver)
    {
	fprintf(sndserver, "p%2.2x%2.2x%2.2x%2.2x\n", id, pitch, vol, sep);
	fflush(sndserver);
    }
    return id;
#else
    id = addsfx( id, vol, steptable[pitch], sep );

    return id;
#endif
}



void I_StopSound (int handle)
{
  handle = 0;
}

int I_SoundIsPlaying(int handle)
{
    return gametic < handle;
}




void I_UpdateSound( void )
{
  register unsigned int	sample;
  register int		dl;
  register int		dr;
  
  signed short*		leftout;
  signed short*		rightout;
  signed short*		leftend;
  int				step;

  int				chan;
    
    leftout = mixbuffer;
    rightout = mixbuffer+1;
    step = 2;

    leftend = mixbuffer + SAMPLECOUNT*step;

    while (leftout != leftend)
    {
	dl = 0;
	dr = 0;

	for ( chan = 0; chan < NUM_CHANNELS; chan++ )
	{
	    if (channels[ chan ])
	    {
		sample = *channels[ chan ];
		dl += channelleftvol_lookup[ chan ][sample];
		dr += channelrightvol_lookup[ chan ][sample];
		channelstepremainder[ chan ] += channelstep[ chan ];
		channels[ chan ] += channelstepremainder[ chan ] >> 16;
		channelstepremainder[ chan ] &= 65536-1;

		if (channels[ chan ] >= channelsend[ chan ])
		    channels[ chan ] = 0;
	    }
	}
	
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

    flag++;
}


void I_SubmitSound(void)
{
#ifndef SNDSERV
    if (audio_device != 0)
    {
        SDL_QueueAudio(audio_device, mixbuffer, SAMPLECOUNT*BUFMUL);
    }
#endif
}


void I_UpdateSoundParams(int handle, int vol, int sep, int pitch)
{
    handle = vol = sep = pitch = 0;
}




void I_ShutdownSound(void)
{    
#ifdef SNDSERV
  if (sndserver)
  {
    fprintf(sndserver, "q\n");
    fflush(sndserver);
  }
#else
  if (audio_device != 0)
  {
      SDL_CloseAudioDevice(audio_device);
      audio_device = 0;
  }
  SDL_Quit();
#endif

  return;
}






void
I_InitSound()
{ 
#ifdef SNDSERV
  char buffer[256];
  
  if (getenv("DOOMWADDIR"))
    sprintf(buffer, "%s/%s",
	    getenv("DOOMWADDIR"),
	    sndserver_filename);
  else
    sprintf(buffer, "%s", sndserver_filename);
  
  if ( !access(buffer, X_OK) )
  {
    strcat(buffer, " -quiet");
    sndserver = popen(buffer, "w");
  }
  else
    fprintf(stderr, "Could not start sound server [%s]\n", buffer);
#else
    
  int i;
  SDL_AudioSpec desired;
    
  fprintf( stderr, "I_InitSound: ");

  if (SDL_Init(SDL_INIT_AUDIO) < 0)
  {
      fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
      return;
  }
  
  memset(&desired, 0, sizeof(desired));
  desired.freq = SAMPLERATE;
  desired.format = AUDIO_S16LSB;
  desired.channels = 2;
  desired.samples = SAMPLECOUNT;
  desired.callback = NULL;
  desired.userdata = NULL;

  audio_device = SDL_OpenAudioDevice(NULL, 0, &desired, NULL, 0);
  if (audio_device == 0)
      fprintf(stderr, "SDL_OpenAudioDevice failed: %s\n", SDL_GetError());
  else
      SDL_PauseAudioDevice(audio_device, 0);

  fprintf(stderr, " configured audio device\n" );

    
  fprintf( stderr, "I_InitSound: ");
  
  for (i=1 ; i<NUMSFX ; i++)
  { 
    if (!S_sfx[i].link)
    {
      S_sfx[i].data = getsfx( S_sfx[i].name, &lengths[i] );
    }	
    else
    {
      S_sfx[i].data = S_sfx[i].link->data;
      lengths[i] = lengths[(S_sfx[i].link - S_sfx)/sizeof(sfxinfo_t)];
    }
  }

  fprintf( stderr, " pre-cached all sound data\n");
  
  for ( i = 0; i< MIXBUFFERSIZE; i++ )
    mixbuffer[i] = 0;
  
  fprintf(stderr, "I_InitSound: sound module ready\n");
    
#endif
}



void I_InitMusic(void)		{ }
void I_ShutdownMusic(void)	{ }

static int	looping=0;
static int	musicdies=-1;

void I_PlaySong(int handle, int looping)
{
  handle = looping = 0;
  musicdies = gametic + TICRATE*30;
}

void I_PauseSong (int handle)
{
  handle = 0;
}

void I_ResumeSong (int handle)
{
  handle = 0;
}

void I_StopSong(int handle)
{
  handle = 0;
  
  looping = 0;
  musicdies = 0;
}

void I_UnRegisterSong(int handle)
{
  handle = 0;
}

int I_RegisterSong(void* data)
{
  data = NULL;
  
  return 1;
}

int I_QrySongPlaying(int handle)
{
  handle = 0;
  return looping || musicdies > gametic;
}
