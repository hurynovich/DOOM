// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: linux.c,v 1.3 1997/01/26 07:45:01 b1 Exp $
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
//
// $Log: linux.c,v $
// Revision 1.3  1997/01/26 07:45:01  b1
// 2nd formatting run, fixed a few warnings as well.
//
// Revision 1.2  1997/01/21 19:00:01  b1
// First formatting run:
//  using Emacs cc-mode.el indentation for C++ now.
//
// Revision 1.1  1997/01/19 17:22:45  b1
// Initial check in DOOM sources as of Jan. 10th, 1997
//
//
// DESCRIPTION:
//	UNIX, soundserver for Linux i386.
//      Refactored to use SDL2 audio instead of OSS /dev/dsp.
//
//-----------------------------------------------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <SDL2/SDL.h>

#include "../include/soundsrv.h"

static SDL_AudioDeviceID audio_device = 0;

void I_InitMusic(void)
{
}

void I_InitSound(int samplerate, int samplesize)
{
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        exit(-1);
    }

    SDL_AudioSpec desired;
    memset(&desired, 0, sizeof(desired));
    desired.freq = samplerate;
    desired.format = AUDIO_S16LSB;
    desired.channels = 2;
    desired.samples = SAMPLECOUNT;
    desired.callback = NULL;
    desired.userdata = NULL;

    audio_device = SDL_OpenAudioDevice(NULL, 0, &desired, NULL, 0);
    if (audio_device == 0) {
        fprintf(stderr, "SDL_OpenAudioDevice failed: %s\n", SDL_GetError());
        exit(-1);
    }

    SDL_PauseAudioDevice(audio_device, 0);
}

void I_SubmitOutputBuffer(void *samples, int samplecount)
{
    SDL_QueueAudio(audio_device, samples, samplecount * 4);
}

void I_ShutdownSound(void)
{
    if (audio_device != 0) {
        SDL_CloseAudioDevice(audio_device);
        audio_device = 0;
    }
    SDL_Quit();
}

void I_ShutdownMusic(void)
{
}
