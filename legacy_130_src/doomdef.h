// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: doomdef.h,v 1.18 2000/08/03 17:57:41 bpereira Exp $
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
// $Log: doomdef.h,v $
// Revision 1.18  2000/08/03 17:57:41  bpereira
// no message
//
// Revision 1.17  2000/06/09 00:49:37  hurdler
// change version number to 1.31 beta 1
//
// Revision 1.16  2000/05/09 20:50:19  hurdler
// change version
//
// Revision 1.15  2000/04/27 17:43:19  hurdler
// colormap code in hardware mode is now the default
//
// Revision 1.14  2000/04/23 16:19:52  bpereira
// no message
//
// Revision 1.13  2000/04/18 12:53:28  hurdler
// change version number
//
// Revision 1.12  2000/04/16 18:38:07  bpereira
// no message
//
// Revision 1.11  2000/04/15 22:12:57  stroggonmeth
// Minor bug fixes
//
// Revision 1.10  2000/04/12 16:01:59  hurdler
// ready for T&L code and true static lighting
//
// Revision 1.9  2000/04/06 21:06:19  stroggonmeth
// Optimized extra_colormap code...
// Added #ifdefs for older water code.
//
// Revision 1.8  2000/04/06 20:40:22  hurdler
// Mostly remove warnings under windows
//
// Revision 1.7  2000/04/04 19:28:42  stroggonmeth
// Global colormaps working. Added a new linedef type 272.
//
// Revision 1.6  2000/04/04 00:32:45  stroggonmeth
// Initial Boom compatability plus few misc changes all around.
//
// Revision 1.5  2000/03/23 22:54:00  metzgermeister
// added support for HOME/.legacy under Linux
//
// Revision 1.4  2000/03/06 15:49:28  hurdler
// change version number
//
// Revision 1.3  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
//
// DESCRIPTION:
//      Internally used data structures for virtually everything,
//      key definitions, lots of other stuff.
//
//-----------------------------------------------------------------------------

#ifndef __DOOMDEF__
#define __DOOMDEF__


#include "doomtype.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <math.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <ctype.h>

#ifndef LINUX
// only dos need this 19990203 by Kin
#include <io.h>
#endif

#ifdef PC_DOS
#include <conio.h>
#endif

// Uncheck this to compile debugging code
//#define RANGECHECK
//#define PARANOIA              // do some test that never happens but maybe
#define LOGMESSAGES             // write message in log.txt (win32 only for the moment)
#define VERSION        130      // Game version
#define VERSIONSTRING  "Demo 4.1"

// some tests, enable or desable it if it run or not
//#define HORIZONTALDRAW        // abandoned : too slow
//#define CLIENTPREDICTION      // not finished
//#define TILTVIEW              // not finished
//#define PERSPCORRECT          // not finished
#define SPLITSCREEN
#define ABSOLUTEANGLE           // work fine, soon #ifdef and old code remove
//#define CLIENTPREDICTION2     // differant methode
#define NEWLIGHT                // compute lighting with bsp (in construction)
#define FAKEFLOORS              // SoM: Setup and clip fake floors.
#define R_FAKEFLOORS            // SoM: Render fake floors.
#define GLOBALCOLORMAPS         // SoM: Use global colormap code.
//#define OLDWATER              // SoM: Allow old legacy water.
#define TANDL                   //Hurdler: Use T&L in hardware mode

// Lactozilla: Heritage patches
#ifndef NOHERITAGE
#define HERITAGE_ASPECT_RATIO            // Makes aspect ratio correction optional
#define HERITAGE_VIEWSIZE_FIX            // Fixes the screen size setting
#define HERITAGE_SCREENSHOT_SHORTCUTS    // F8 and F9 keys take screenshots and movies, respectively
#define HERITAGE_SCREENSHOT_COLORPROFILE // Enables color profile option for screenshots and movies, 2.2 port
#define HERITAGE_MOUSE_GRAB              // Mouse grab port from 2.2
#define HERITAGE_PAUSE_BEHAVIOR          // Modern pause behavior (instead of actually pausing the game)
#define HERITAGE_THREE_SCREEN_MODE       // Heritage's three screen mode behavior
#define HERITAGE_PLAYER_MOVEMENT         // Modern player movement code port from 2.2
#define HERITAGE_DIRECTIONCHAR           // directionchar port from 2.2
#define HERITAGE_ENEMY_COLLISION         // Fixes enemy collision
#define HERITAGE_ENEMY_STOP_AT_DEATH     // Stops enemies from moving when they are destroyed
#endif // NOHERITAGE

// =========================================================================
#ifdef __WIN32__
#define ASMCALL __cdecl
// warning C4146: unary minus operator applied to unsigned type, result still unsigned
// warning C4761: integral size mismatch in argument; conversion supplied
// warning C4244: 'initializing' : conversion from 'const double ' to 'int ', possible loss of data
// warning C4244: '=' : conversion from 'double ' to 'int ', possible loss of data
// warning C4018: '<' : signed/unsigned mismatch

#else
#define ASMCALL
#endif



// demo version when playback demo, or the current VERSION
// used to enable/disable selected features for backward compatibility
// (where possible)
extern UINT8    demoversion;


// The maximum number of players, multiplayer/networking.
// NOTE: it needs more than this to increase the number of players...

#define MAXPLAYERS              32      // TODO: ... more!!!
#define MAXSKINS                MAXPLAYERS
#define PLAYERSMASK             (MAXPLAYERS-1)
#define MAXPLAYERNAME           21
#define MAXSKINCOLORS           11

#define SAVESTRINGSIZE          24

// State updates, number of tics / second.
// NOTE: used to setup the timer rate, see I_StartupTimer().
#define TICRATE         35

// Special Stage level # definitions Tails 08-11-2001
#define SSSTAGE1		15
#define SSSTAGE2		16
#define SSSTAGE3		17
#define SSSTAGE4		18
#define SSSTAGE5		19
#define SSSTAGE6		20
#define SSSTAGE7		21

// Lactozilla: Title screen level
#define TITLEMAP        33

// Used for ring shield. Change this to affect
// how close you need to be to a ring to attract
// it. Tails
#define RING_DIST	512*FRACUNIT

// Name of local directory for config files and savegames
#ifdef LINUX
#define DEFAULTDIR ".legacy"
#else
#define DEFAULTDIR "legacy"
#endif

// ==================================
// Difficulty/skill settings/filters.
// ==================================

// Skill flags.
#define MTF_EASY                1
#define MTF_NORMAL              2
#define MTF_HARD                4

// Deaf monsters/do not react to sound.
#define MTF_AMBUSH              8

#include "g_state.h"

//
// Key cards.
//
typedef enum
{
	it_bluecard   =    1,
	it_yellowcard =    2,
	it_redcard    =    4,
	it_blueskull  =    8,
	it_yellowskull= 0x10,
	it_redskull   = 0x20,
	it_allkeys    = 0x3f,
	NUMCARDS      = 6

} card_t;



// The defined weapons,
//  including a marker indicating
//  user has not changed weapon.
typedef enum
{
	wp_fist,
	wp_pistol,
	wp_shotgun,
	wp_chaingun,
	wp_missile,
	wp_plasma,
	wp_bfg,
	wp_chainsaw,
	wp_supershotgun,

	NUMWEAPONS,

	// No pending weapon change.
	wp_nochange

} weapontype_t;


// Ammunition types defined.
typedef enum
{
	am_clip,    // Pistol / chaingun ammo.
	am_shell,   // Shotgun / double barreled shotgun.
	am_cell,    // Plasma rifle, BFG.
	am_misl,    // Missile launcher.
	NUMAMMO,
	am_noammo   // Unlimited for chainsaw / fist.

} ammotype_t;


// Power up artifacts.
typedef enum
{
	pw_invulnerability,
	pw_strength,
	pw_invisibility,
	pw_ironfeet,
	pw_allmap,
	pw_infrared,
	pw_blueshield, // blue shield Tails 03-04-2000
	pw_tailsfly, // tails flying Tails 03-05-2000
	pw_underwater, // underwater timer Tails 03-06-2000
	pw_extralife, // Extra Life timer Tails 03-14-2000
	pw_yellowshield, // yellow shield Tails 03-15-2000
	pw_blackshield, // black shield Tails 04-08-2000
	pw_greenshield, // green shield Tails 04-08-2000
	pw_super, // Are you super? Tails 04-08-2000
	NUMPOWERS

} powertype_t;



//
// Power up durations,
//  how many seconds till expiration,
//  assuming TICRATE is 35 ticks/second.
//
typedef enum
{
	INVULNTICS  = (20*TICRATE),
	INVISTICS   = (3*TICRATE), // just hit Tails 2-26-2000
	INFRATICS   = (120*TICRATE),
	IRONTICS    = (60*TICRATE), // Tails 02-28-2000
	SPEEDTICS   = (20*TICRATE), // for super sneaker timer Tails 02-28-2000
	TAILSTICS   = (10*TICRATE), // tails flying Tails 03-05-2000
	WATERTICS   = (30*TICRATE), // underwater timer Tails 03-07-2000

} powerduration_t;


// commonly used routines - moved here for include convenience

char *sizeu1(size_t num);
char *sizeu2(size_t num);
char *sizeu3(size_t num);
char *sizeu4(size_t num);
char *sizeu5(size_t num);

// i_system.h
void I_Error (const char *error, ...);

// console.h
void    CONS_Printf (const char *fmt, ...);

#include "m_swap.h"

// m_misc.h
char *va(const char *format, ...);

// g_game.h
extern  boolean devparm;                // development mode (-devparm)

// =======================
// Misc stuff for later...
// =======================

// Modifier key variables, accessible anywhere
extern UINT8 shiftdown, ctrldown, altdown;
extern boolean capslock;

// if we ever make our alloc stuff...
#define ZZ_Alloc(x) Z_Malloc(x,PU_STATIC,NULL)

// i_system.c, replace getchar() once the keyboard has been appropriated
int I_GetKey (void);

#ifndef min // Double-Check with WATTCP-32's cdefs.h
#define min(x, y) (((x) < (y)) ? (x) : (y))
#endif
#ifndef max // Double-Check with WATTCP-32's cdefs.h
#define max(x, y) (((x) > (y)) ? (x) : (y))
#endif

#ifndef M_PIl
#define M_PIl 3.1415926535897932384626433832795029L
#endif

// Floating point comparison epsilons from float.h
#ifndef FLT_EPSILON
#define FLT_EPSILON 1.1920928955078125e-7f
#endif

#ifndef DBL_EPSILON
#define DBL_EPSILON 2.2204460492503131e-16l
#endif

// The character that separates pathnames. Forward slash on
// most systems, but reverse solidus (\) on Windows.
#if defined (_WIN32)
	#define PATHSEP "\\"
#else
	#define PATHSEP "/"
#endif

#define PUNCTUATION "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~"

// Compile date and time and revision.
extern const char *compdate, *comptime, *comprevision, *compbranch;

// Sound system select
// This should actually be in the makefile,
// but I can't stand that gibberish. D:
#define SOUND_DUMMY   0
#define SOUND_SDL     1
#define SOUND_MIXER   2

#ifndef SOUND
#ifdef HAVE_SDL

// Use Mixer interface?
#ifdef HAVE_MIXER
	#define SOUND SOUND_MIXER
	#ifdef HW3SOUND
	#undef HW3SOUND
	#endif
#endif

// Use generic SDL interface.
#ifndef SOUND
#define SOUND SOUND_SDL
#endif

#else // No SDL.

// No more interfaces. :(
#define SOUND SOUND_DUMMY

#endif
#endif

#endif          // __DOOMDEF__
