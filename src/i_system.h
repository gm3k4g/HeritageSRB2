// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: i_system.h,v 1.4 2000/04/25 19:49:46 metzgermeister Exp $
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
// $Log: i_system.h,v $
// Revision 1.4  2000/04/25 19:49:46  metzgermeister
// support for automatic wad search
//
// Revision 1.3  2000/04/16 18:38:07  bpereira
// no message
//
// Revision 1.2  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      System specific interface stuff.
//
//-----------------------------------------------------------------------------


#ifndef __I_SYSTEM__
#define __I_SYSTEM__

#include "d_ticcmd.h"
#include "d_event.h"
#include "doomtype.h"

#ifdef __GNUG__
#pragma interface
#endif

// See Shutdown_xxx() routines.
extern UINT8 graphics_started;
extern UINT8 keyboard_started;
extern UINT8 sound_started;
//extern UINT8 music_installed;

/* flag for 'win-friendly' mode used by interface code */
extern int i_love_bill;
extern volatile UINT32 ticcount;

void I_GetKeyboardEvents (void);
void I_GetMouseEvents (void);
void I_GetJoystickEvents (void);

#ifdef LOGMESSAGES
extern FILE *logstream;
extern char logfilename[1024];
#endif

#ifdef __WIN32__
extern boolean winnt;
extern BOOL   bDX0300;
#endif

// Called by DoomMain.
void I_InitJoystick (void);
void I_JoyScale (void);
INT32 I_NumJoys (void);
const char *I_GetJoyName (INT32 joyindex);

UINT32 I_GetFreeMem (UINT32 *total);

void I_GetEvent (void);
void I_OsPolling (void);
void I_UpdateNoVsync (void);

// Called by D_DoomLoop,
// returns current time in tics.
UINT32 I_GetTime (void);

// Returns precise time value for performance measurement.
UINT64 I_GetPreciseTime (void);

// Returns the difference between precise times as microseconds.
int I_PreciseToMicros (UINT64 d);


//
// Called by D_DoomLoop,
// called before processing each tic in a frame.
// Quick syncronous operations are performed here.
// Can call D_PostEvent.
void I_StartTic (void);

// Asynchronous interrupt functions should maintain private queues
// that are read by the synchronous functions
// to be converted into events.

// Either returns a null ticcmd,
// or calls a loadable driver to build it.
// This ticcmd will then be modified by the gameloop
// for normal input.
ticcmd_t* I_BaseTiccmd (void);


// Called by M_Responder when quit is selected, return code 0.
void I_Quit (void) FUNCNORETURN;

void I_Error (const char *error, ...) FUNCIERROR;

void I_Tactile (int on, int off, int total);

//added:18-02-98: write a message to stderr (use before I_Quit)
//                for when you need to quit with a msg, but need
//                the return code 0 of I_Quit();
void I_OutputMsg (const char *error, ...) FUNCPRINTF;

void I_StartupMouse (void);
void I_StartupMouse2(void);
void I_UpdateMouseGrab (void);

// setup timer irq and user timer routine.
void I_TimerISR (void);      //timer callback routine.
void I_StartupTimer (void);

/* list of functions to call at program cleanup */
void I_AddExitFunc (void (*func)());
void I_RemoveExitFunc (void (*func)());

// Setup signal handler, plus stuff for trapping errors and cleanly exit.
int  I_StartupSystem (void);
void I_ShutdownSystem (void);
void I_RegisterSysCommands (void);

void I_GetDiskFreeSpace(INT64 *freespace);
char *I_GetUserName(void);
int  I_mkdir(const char *dirname, int unixright);

char *I_GetEnv(const char *name);
INT32 I_PutEnv(char *variable);

INT32 I_ClipboardCopy(const char *data, size_t size);
const char *I_ClipboardPaste(void);

const char *I_LocateWad(void);

void I_ShowErrorBox(const char *title, const char *message);

#endif
