// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: m_misc.h,v 1.3 2000/04/16 18:38:07 bpereira Exp $
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
// $Log: m_misc.h,v $
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
//      Default Config File.
//      PCX Screenshots.
//      File i/o
//      Common used routines
//
//-----------------------------------------------------------------------------


#ifndef __M_MISC__
#define __M_MISC__


#include "doomtype.h"
#include "command.h"
#include "w_wad.h"

// the file where all game vars and settings are saved
#define CONFIGFILENAME   "config.cfg"


//
// MISC
//
//===========================================================================

boolean FIL_WriteFile ( char const*   name,
                        void*         source,
                        int           length );

int  FIL_ReadFile ( char const*   name,
                    UINT8**        buffer );

void FIL_DefaultExtension (char *path, char *extension);

//added:11-01-98:now declared here for use by G_DoPlayDemo(), see there...
void FIL_ExtractFileBase (char* path, char* dest);

boolean FIL_CheckExtension (char *in);

boolean FIL_WriteFileOK(char const *name);
boolean FIL_ReadFileOK(char const *name);
boolean FIL_FileOK(char const *name);

//===========================================================================

typedef enum {
	MM_OFF = 0,
	MM_APNG,
	MM_GIF,
	MM_SCREENSHOT
} moviemode_t;
extern moviemode_t moviemode;

extern consvar_t cv_screenshot_option, cv_screenshot_folder, cv_screenshot_colorprofile;
extern consvar_t cv_moviemode, cv_movie_folder, cv_movie_option;
extern consvar_t cv_zlib_memory, cv_zlib_level, cv_zlib_strategy, cv_zlib_window_bits;
extern consvar_t cv_zlib_memorya, cv_zlib_levela, cv_zlib_strategya, cv_zlib_window_bitsa;
extern consvar_t cv_apng_delay;

extern boolean takescreenshot;
void M_ScreenShot(void);
void M_DoScreenShot(void);

void M_StartMovie(void);
void M_SaveFrame(void);
void M_StopMovie(void);

boolean M_SavePNG(const char *filename, void *data, int width, int height, const UINT8 *palette);

//===========================================================================

int     M_PathParts      (const char *path);
boolean M_IsPathAbsolute (const char *path);
void    M_MkdirEach      (const char *path, int start, int mode);
void    M_MkdirEachUntil (const char *path, int start, int end, int mode);

//===========================================================================

extern char   configfile[MAX_WADPATH];

void Command_SaveConfig_f (void);
void Command_LoadConfig_f (void);
void Command_ChangeConfig_f (void);

void M_FirstLoadConfig(void);
//Fab:26-04-98: save game config : cvars, aliases..
void M_SaveConfig (char *filename);

//===========================================================================

// s1=s2+s3+s1 (1024 lenghtmax)
void strcatbf(char *s1,char *s2,char *s3);

#endif
