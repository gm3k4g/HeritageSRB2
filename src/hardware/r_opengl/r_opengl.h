// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: r_opengl.h,v 1.10 2000/08/10 19:58:05 bpereira Exp $
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
// $Log: r_opengl.h,v $
// Revision 1.10  2000/08/10 19:58:05  bpereira
// no message
//
// Revision 1.9  2000/08/03 17:57:42  bpereira
// no message
//
// Revision 1.8  2000/05/09 21:10:04  hurdler
// update version
//
// Revision 1.7  2000/04/18 12:45:09  hurdler
// change a little coronas' code
//
// Revision 1.6  2000/04/07 23:10:15  metzgermeister
// fullscreen support under X in Linux
//
// Revision 1.5  2000/03/07 03:31:14  hurdler
// fix linux compilation
//
// Revision 1.4  2000/03/06 15:29:32  hurdler
// change version number
//
// Revision 1.3  2000/02/27 16:37:14  hurdler
// Update version number
//
// Revision 1.2  2000/02/27 00:42:11  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:33  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//
//
//-----------------------------------------------------------------------------


#ifndef _R_OPENGL_H_
#define _R_OPENGL_H_

#ifdef HAVE_SDL
#define _MATH_DEFINES_DEFINED

#ifdef _MSC_VER
#pragma warning(disable : 4214 4244)
#endif

#include "SDL_opengl.h" //Alam_GBC: Simple, yes?

#ifdef _MSC_VER
#pragma warning(default : 4214 4244)
#endif

#else
#include <GL/gl.h>
#include <GL/glu.h>

#ifdef STATIC_OPENGL // Because of the 1.3 functions, you'll need GLext to compile it if static
#define GL_GLEXT_PROTOTYPES
#include <GL/glext.h>
#endif
#endif

#define  _CREATE_DLL_  // necessary for Unix AND Windows
#include "../../doomdef.h"
#include "../hw_drv.h"

// ==========================================================================
//                                                                DEFINITIONS
// ==========================================================================

#define MIN(x,y) ( ((x)<(y)) ? (x) : (y) )
#define MAX(x,y) ( ((x)>(y)) ? (x) : (y) )

#undef DEBUG_TO_FILE            // maybe defined in previous *.h
#define DEBUG_TO_FILE           // output debugging msgs to ogllog.txt

#define DRIVER_STRING "HWRAPI Init(): DooM Legacy OpenGL renderer v1.29 pre-release 8\n"

// todo: find some way of getting SDL to log to ogllog.txt, without
// interfering with r_opengl.dll
#ifdef HAVE_SDL
#undef DEBUG_TO_FILE
#endif
//#if defined(HAVE_SDL) && !defined(_DEBUG)
//#undef DEBUG_TO_FILE
//#endif

#ifdef DEBUG_TO_FILE
extern FILE             *gllogstream;
#endif

// ==========================================================================
//                                                                     PROTOS
// ==========================================================================

boolean LoadGL(void);
void *GetGLFunc(const char *proc);
boolean SetupGLfunc(void);
void Flush(void);
INT32 isExtAvailable(const char *extension, const GLubyte *start);
void SetModelView(GLint w, GLint h);
void SetStates(void);

#ifdef STATIC_OPENGL
#define pglClear glClear
#define pglGetIntegerv glGetIntegerv
#define pglGetString glGetString
#else
/* 1.0 Miscellaneous functions */
typedef void (APIENTRY * PFNglClear) (GLbitfield mask);
extern PFNglClear pglClear;
typedef void (APIENTRY * PFNglGetIntegerv) (GLenum pname, GLint *params);
extern PFNglGetIntegerv pglGetIntegerv;
typedef const GLubyte* (APIENTRY  * PFNglGetString) (GLenum name);
extern PFNglGetString pglGetString;
#if 0
typedef void (APIENTRY * PFNglEnableClientState) (GLenum cap); // redefined in r_opengl.c
static PFNglEnableClientState pglEnableClientState;
#endif
#endif

// ==========================================================================
//                                                                     GLOBAL
// ==========================================================================

extern const GLubyte   *gl_version;
extern const GLubyte   *gl_renderer;
extern const GLubyte   *gl_extensions;

extern RGBA_t           myPaletteData[];
extern GLint            screen_width;
extern GLint            screen_height;
extern GLbyte           screen_depth;

extern GLint            textureformatGL;

#endif
