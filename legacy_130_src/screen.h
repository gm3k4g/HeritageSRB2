// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: screen.h,v 1.3 2000/04/22 20:27:35 metzgermeister Exp $
//
// Copyright (C) 1998-2000 by DooM Legacy Team.
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
// $Log: screen.h,v $
// Revision 1.3  2000/04/22 20:27:35  metzgermeister
// support for immediate fullscreen switching
//
// Revision 1.2  2000/02/27 00:42:11  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//
//-----------------------------------------------------------------------------


#ifndef __SCREEN_H__
#define __SCREEN_H__

#include "command.h"

#ifdef __WIN32__
#include <windows.h>
#else
#define HWND    void*   //unused in DOS version
#endif

//added:26-01-98: quickhack for V_Init()... to be cleaned up
#define NUMSCREENS    5

// Size of statusbar.
#ifdef HERITAGE_VIEWSIZE_FIX
#define ST_HEIGHT    0
#else
#define ST_HEIGHT    32
#endif
#define ST_WIDTH     320

#ifdef HERITAGE_VIEWSIZE_FIX
#define MIN_VIEW_SIZE 1
#define MAX_VIEW_SIZE 10
#else
#define MIN_VIEW_SIZE 3
#define MAX_VIEW_SIZE 11
#endif

//added:20-01-98: used now as a maximum video mode size for extra vesa modes.

// we try to re-allocate a minimum of buffers for stability of the memory,
// so all the small-enough tables based on screen size, are allocated once
// and for all at the maximum size.

//#define MAXVIDWIDTH     1024  //dont set this too high because actually
//#define MAXVIDHEIGHT    768  // lots of tables are allocated with the MAX

//Tails
#define MAXVIDWIDTH     1920  //dont set this too high because actually
#define MAXVIDHEIGHT    1200  // lots of tables are allocated with the MAX
                            // size.
#define BASEVIDWIDTH    320   //NEVER CHANGE THIS! this is the original
#define BASEVIDHEIGHT   200  // resolution of the graphics.

// global video state
typedef struct viddef_s
{
    int         modenum;         // vidmode num indexes videomodes list

    UINT8       *buffer;         // invisible screens buffer
    unsigned    rowbytes;        // bytes per scanline of the VIDEO mode
    int         width;           // PIXELS per scanline
    int         height;
    union { // hurdler: don't need numpages for OpenGL, so we can
            // 15/10/99 use it for fullscreen / windowed mode
    int         numpages;        // always 1, PAGE FLIPPING TODO!!!
    int         windowed;        // windowed or fullscren mode ?
    } u; //BP: name it please soo it work with gcc
    int         recalc;          // if true, recalc vid-based stuff
    UINT8       *direct;         // linear frame buffer, or vga base mem.
    int         dupx,dupy;       // scale 1,2,3 value for menus & overlays
    int         idupx,idupy;
    float       fdupx,fdupy;
    int         centerofs;       // centering for the scaled menu gfx
    int         bpp;             // BYTES per pixel: 1=256color, 2=highcolor
    int         glstate;
} viddef_t;
#define VIDWIDTH    vid.width
#define VIDHEIGHT   vid.height

enum
{
	VID_GL_LIBRARY_NOTLOADED  = 0,
	VID_GL_LIBRARY_LOADED     = 1,
	VID_GL_LIBRARY_ERROR      = -1,
};


// internal additional info for vesa modes only
typedef struct {
    int         vesamode;         // vesa mode number plus LINEAR_MODE bit
    void        *plinearmem;      // linear address of start of frame buffer
} vesa_extra_t;
// a video modes from the video modes list,
// note: video mode 0 is always standard VGA320x200.
typedef struct vmode_s {

    struct vmode_s  *pnext;
    char         *name;
    unsigned int width;
    unsigned int height;
    unsigned int rowbytes;          //bytes per scanline
    unsigned int bytesperpixel;     // 1 for 256c, 2 for highcolor
    int          windowed;          // if true this is a windowed mode
    int          numpages;
    vesa_extra_t *pextradata;       //vesa mode extra data
    int          (*setmode)(viddef_t *lvid, struct vmode_s *pcurrentmode);
    int          misc;              //misc for display driver (r_glide.dll etc)
} vmode_t;

// ---------------------------------------------
// color mode dependent drawer function pointers
// ---------------------------------------------

extern void     (*skycolfunc) (void);
extern void     (*colfunc) (void);
#ifdef HORIZONTALDRAW
extern void     (*hcolfunc) (void);    //Fab 17-06-98
#endif
extern void     (*basecolfunc) (void);
extern void     (*fuzzcolfunc) (void);
extern void     (*transcolfunc) (void);
extern void     (*shadecolfunc) (void);
extern void     (*spanfunc) (void);
extern void     (*basespanfunc) (void);


// ----------------
// screen variables
// ----------------
extern viddef_t vid;
extern int      setmodeneeded;     // mode number to set if needed, or 0

extern boolean  fuzzymode;


extern int      scr_bpp;

extern int      scr_gamma;
extern int      scr_viewsize;      // (ex-screenblocks)

extern UINT8*    scr_borderpatch;   // patch used to fill the view borders


extern consvar_t cv_usegamma;
extern consvar_t cv_viewsize;
extern consvar_t cv_detaillevel;

extern consvar_t cv_scr_width;
extern consvar_t cv_scr_height;
extern consvar_t cv_scr_depth;
extern consvar_t cv_fullscreen;

extern consvar_t cv_vidwait, cv_stretch;

// quick fix for tall/short skies, depending on bytesperpixel
void (*skydrawerfunc[2]) (void);

void SCR_SetDrawFuncs (void);

// from vid_vesa.c : user config video mode decided at VID_Init ();
extern int      vid_modenum;

// Initialize the screen
void SCR_Startup (void);

// Change video mode, only at the start of a refresh.
void SCR_SetMode (void);

// Set drawer functions for Software
void SCR_SetDrawFuncs (void);

// Recalc screen size dependent stuff
void SCR_Recalc (void);

// Check parms once at startup
void SCR_CheckDefaultMode (void);

// Set the mode number which is saved in the config
void SCR_SetDefaultMode (void);

// Fullscreen
void SCR_ChangeFullscreen (void);

#ifdef DIRECTFULLSCREEN
extern boolean allow_fullscreen;
#endif

// move out to main code for consistency
void SCR_DisplayTicRate (void);

#endif //__SCREEN_H__
