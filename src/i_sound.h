// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: i_sound.h,v 1.4 2000/04/19 15:21:02 hurdler Exp $
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Portions Copyright (C) 1998-2000 by DooM Legacy Team.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
//
// $Log: i_sound.h,v $
// Revision 1.4  2000/04/19 15:21:02  hurdler
// add SDL midi support
//
// Revision 1.3  2000/03/22 18:49:38  metzgermeister
// added I_PauseCD() for Linux
//
// Revision 1.2  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      System interface, sound.
//
//-----------------------------------------------------------------------------

#ifndef __I_SOUND__
#define __I_SOUND__

#include "doomdef.h"
#include "sounds.h"

// copied from SDL mixer, plus GME
typedef enum {
	MU_NONE,
	MU_WAV,
	MU_MOD,
	MU_MID,
	MU_OGG,
	MU_MP3,
	MU_FLAC,
	MU_GME,
	MU_MOD_EX, // libopenmpt
	MU_MID_EX // Non-native MIDI
} musictype_t;


void* I_GetSfx (sfxinfo_t*  sfx);
void  I_FreeSfx (sfxinfo_t* sfx);


// Init at program start...
void I_StartupSound(void);

// ... shut down and relase at program termination.
void I_ShutdownSound(void);


//
//  SFX I/O
//

// Starts a sound in a particular sound channel.
int
I_StartSound
( int           id,
  int           vol,
  int           sep,
  int           pitch,
  int           priority );


// Stops a sound channel.
void I_StopSound(int handle);

// Called by S_*() functions
//  to see if a channel is still playing.
// Returns 0 if no longer playing, 1 if playing.
int I_SoundIsPlaying(int handle);

// Updates the volume, separation,
//  and pitch of a sound channel.
void
I_UpdateSoundParams
( int           handle,
  int           vol,
  int           sep,
  int           pitch );


//
//  MUSIC I/O
//
void I_InitMusic(void);
void I_ShutdownMusic(void);
// Volume.
void I_SetMusicVolume(int volume);
void I_SetSfxVolume(int volume);
// PAUSE game handling.
void I_PauseSong(void);
void I_ResumeSong(void);
// Registers a song handle to song data.
boolean I_LoadSong(char* data,size_t len);
// Called by anything that wishes to start music.
//  plays a song, and when the song is done,
//  starts playing it again in an endless loop.
// Horrible thing to do, considering.
boolean
I_PlaySong
( int           looping );
// Stops a song over 3 seconds.
void I_StopSong(void);
// See above (register), then think backwards
void I_UnloadSong(void);

musictype_t I_SongType(void);
boolean I_SongPlaying(void);
boolean I_SongPaused(void);

boolean I_SetSongSpeed(float speed);

UINT32 I_GetSongLength(void);

boolean I_SetSongLoopPoint(UINT32 looppoint);
UINT32 I_GetSongLoopPoint(void);

boolean I_SetSongPosition(UINT32 position);
UINT32 I_GetSongPosition(void);

#endif
