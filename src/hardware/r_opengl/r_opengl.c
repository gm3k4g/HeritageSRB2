// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: r_opengl.c,v 1.30 2000/08/10 19:58:04 bpereira Exp $
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
// $Log: r_opengl.c,v $
// Revision 1.30  2000/08/10 19:58:04  bpereira
// no message
//
// Revision 1.29  2000/08/10 14:19:19  hurdler
// add waitvbl, fix sky problem
//
// Revision 1.28  2000/07/01 09:23:50  bpereira
// no message
//
// Revision 1.27  2000/06/08 19:41:53  hurdler
// my changes before splitting (can be reverted in development branch)
//
// Revision 1.26  2000/05/10 17:43:48  kegetys
// Sprites are drawn using PF_Environment
//
// Revision 1.25  2000/05/09 20:53:27  hurdler
// definitively fix the colormap
//
// Revision 1.24  2000/05/05 18:00:06  bpereira
// no message
//
// Revision 1.23  2000/04/30 10:30:10  bpereira
// no message
//
// Revision 1.22  2000/04/28 00:09:22  hurdler
// Full support of coronas in splitscreen mode
//
// Revision 1.21  2000/04/27 23:42:30  hurdler
// Change coronas' code for MiniGL compatibility
//
// Revision 1.20  2000/04/27 17:52:32  hurdler
// colormap code in hardware mode is now the default
//
// Revision 1.19  2000/04/24 15:20:40  hurdler
// Support colormap for text
//
// Revision 1.18  2000/04/23 15:07:37  hurdler
// quick bug fix
//
// Revision 1.17  2000/04/23 12:55:24  hurdler
// support filter mode in OpenGL
//
// Revision 1.16  2000/04/22 16:48:56  hurdler
// no message
//
// Revision 1.15  2000/04/22 16:48:00  hurdler
// support skin color
//
// Revision 1.14  2000/04/19 10:54:43  hurdler
// no message
//
// Revision 1.13  2000/04/18 16:07:47  hurdler
// better support of decals
//
// Revision 1.12  2000/04/18 14:49:25  hurdler
// fix a bug for Mesa 3.1 and previous
//
// Revision 1.11  2000/04/18 12:45:09  hurdler
// change a little coronas' code
//
// Revision 1.10  2000/04/16 18:38:07  bpereira
// no message
//
// Revision 1.9  2000/04/14 23:31:02  hurdler
// fix the bug of near clipping plane at startup
//
// Revision 1.8  2000/04/14 16:37:12  hurdler
// some nice changes for coronas
//
// Revision 1.7  2000/04/11 01:00:23  hurdler
// Better coronas support
//
// Revision 1.6  2000/04/09 17:18:24  hurdler
// modified coronas' code for 16 bits video mode
//
// Revision 1.5  2000/04/06 20:51:57  hurdler
// add support for new coronas
//
// Revision 1.4  2000/03/29 19:39:49  bpereira
// no message
//
// Revision 1.3  2000/03/07 03:31:14  hurdler
// fix linux compilation
//
// Revision 1.2  2000/02/27 00:42:11  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:33  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      OpenGL API for Doom Legacy
//
//-----------------------------------------------------------------------------

#if defined (_WIN32)
//#define WIN32_LEAN_AND_MEAN
#define RPC_NO_WINDOWS_H
#include <windows.h>
#endif
#undef GETTEXT
#ifdef __GNUC__
#include <unistd.h>
#endif

#include <stdarg.h>
#include <math.h>
#include "r_opengl.h"

// ==========================================================================
//                                                                  CONSTANTS
// ==========================================================================

// With OpenGL 1.1+, the first texture should be 1
static  GLuint NOTEXTURE_NUM   = 0;     // small white texture

#define N_PI_DEMI  (1.5707963268f)                  // PI/2

#define ASPECT_RATIO            (1.0f)  //(320.0f/200.0f)
#define FAR_CLIPPING_PLANE      6000.0f
float   NEAR_CLIPPING_PLANE =   0.9f;

#define MIPMAP_MASK     0x0100

// **************************************************************************
//                                                                    GLOBALS
// **************************************************************************


static  GLuint      tex_downloaded  = 0;
static  GLfloat     fov             = 90.0;
static  GLuint      pal_col         = 0;
static  FRGBAFloat  const_pal_col;
static  FBITFIELD   CurrentPolyFlags;

static  FTextureInfo*  gr_cachetail = NULL;
static  FTextureInfo*  gr_cachehead = NULL;

RGBA_t  myPaletteData[256];
GLint   screen_width;               // used by Draw2DLine()
GLint   screen_height;
GLbyte  screen_depth;
GLint   textureformatGL = 0;

static GLint min_filter = GL_LINEAR;
static GLint mag_filter = GL_LINEAR;

const GLubyte *gl_version = NULL;
const GLubyte *gl_renderer = NULL;
const GLubyte *gl_extensions = NULL;

//Hurdler: 04/10/2000: added for the kick ass coronas as Boris wanted ;-)
static GLfloat     modelMatrix[16];
static GLfloat     projMatrix[16];
static GLint       viewport[4];

static GLuint finalScreenTexture = 0;

static const GLfloat    int2float[256] = {
	0.000000f, 0.003922f, 0.007843f, 0.011765f, 0.015686f, 0.019608f, 0.023529f, 0.027451f,
	0.031373f, 0.035294f, 0.039216f, 0.043137f, 0.047059f, 0.050980f, 0.054902f, 0.058824f,
	0.062745f, 0.066667f, 0.070588f, 0.074510f, 0.078431f, 0.082353f, 0.086275f, 0.090196f,
	0.094118f, 0.098039f, 0.101961f, 0.105882f, 0.109804f, 0.113725f, 0.117647f, 0.121569f,
	0.125490f, 0.129412f, 0.133333f, 0.137255f, 0.141176f, 0.145098f, 0.149020f, 0.152941f,
	0.156863f, 0.160784f, 0.164706f, 0.168627f, 0.172549f, 0.176471f, 0.180392f, 0.184314f,
	0.188235f, 0.192157f, 0.196078f, 0.200000f, 0.203922f, 0.207843f, 0.211765f, 0.215686f,
	0.219608f, 0.223529f, 0.227451f, 0.231373f, 0.235294f, 0.239216f, 0.243137f, 0.247059f,
	0.250980f, 0.254902f, 0.258824f, 0.262745f, 0.266667f, 0.270588f, 0.274510f, 0.278431f,
	0.282353f, 0.286275f, 0.290196f, 0.294118f, 0.298039f, 0.301961f, 0.305882f, 0.309804f,
	0.313726f, 0.317647f, 0.321569f, 0.325490f, 0.329412f, 0.333333f, 0.337255f, 0.341176f,
	0.345098f, 0.349020f, 0.352941f, 0.356863f, 0.360784f, 0.364706f, 0.368627f, 0.372549f,
	0.376471f, 0.380392f, 0.384314f, 0.388235f, 0.392157f, 0.396078f, 0.400000f, 0.403922f,
	0.407843f, 0.411765f, 0.415686f, 0.419608f, 0.423529f, 0.427451f, 0.431373f, 0.435294f,
	0.439216f, 0.443137f, 0.447059f, 0.450980f, 0.454902f, 0.458824f, 0.462745f, 0.466667f,
	0.470588f, 0.474510f, 0.478431f, 0.482353f, 0.486275f, 0.490196f, 0.494118f, 0.498039f,
	0.501961f, 0.505882f, 0.509804f, 0.513726f, 0.517647f, 0.521569f, 0.525490f, 0.529412f,
	0.533333f, 0.537255f, 0.541177f, 0.545098f, 0.549020f, 0.552941f, 0.556863f, 0.560784f,
	0.564706f, 0.568627f, 0.572549f, 0.576471f, 0.580392f, 0.584314f, 0.588235f, 0.592157f,
	0.596078f, 0.600000f, 0.603922f, 0.607843f, 0.611765f, 0.615686f, 0.619608f, 0.623529f,
	0.627451f, 0.631373f, 0.635294f, 0.639216f, 0.643137f, 0.647059f, 0.650980f, 0.654902f,
	0.658824f, 0.662745f, 0.666667f, 0.670588f, 0.674510f, 0.678431f, 0.682353f, 0.686275f,
	0.690196f, 0.694118f, 0.698039f, 0.701961f, 0.705882f, 0.709804f, 0.713726f, 0.717647f,
	0.721569f, 0.725490f, 0.729412f, 0.733333f, 0.737255f, 0.741177f, 0.745098f, 0.749020f,
	0.752941f, 0.756863f, 0.760784f, 0.764706f, 0.768627f, 0.772549f, 0.776471f, 0.780392f,
	0.784314f, 0.788235f, 0.792157f, 0.796078f, 0.800000f, 0.803922f, 0.807843f, 0.811765f,
	0.815686f, 0.819608f, 0.823529f, 0.827451f, 0.831373f, 0.835294f, 0.839216f, 0.843137f,
	0.847059f, 0.850980f, 0.854902f, 0.858824f, 0.862745f, 0.866667f, 0.870588f, 0.874510f,
	0.878431f, 0.882353f, 0.886275f, 0.890196f, 0.894118f, 0.898039f, 0.901961f, 0.905882f,
	0.909804f, 0.913726f, 0.917647f, 0.921569f, 0.925490f, 0.929412f, 0.933333f, 0.937255f,
	0.941177f, 0.945098f, 0.949020f, 0.952941f, 0.956863f, 0.960784f, 0.964706f, 0.968628f,
	0.972549f, 0.976471f, 0.980392f, 0.984314f, 0.988235f, 0.992157f, 0.996078f, 1.000000
};


static I_Error_t I_Error_GL = NULL;


// -----------------+
// GL_DBG_Printf    : Output debug messages to debug log if DEBUG_TO_FILE is defined,
//                  : else do nothing
// Returns          :
// -----------------+

#ifdef DEBUG_TO_FILE
FILE *gllogstream;
#endif

FUNCPRINTF void GL_DBG_Printf(const char *format, ...)
{
#ifdef DEBUG_TO_FILE
	char str[4096] = "";
	va_list arglist;

	if (!gllogstream)
		gllogstream = fopen("ogllog.txt", "w");

	va_start(arglist, format);
	vsnprintf(str, 4096, format, arglist);
	va_end(arglist);

	fwrite(str, strlen(str), 1, gllogstream);
#else
	(void)format;
#endif
}

// -----------------+
// GL_MSG_Warning   : Raises a warning.
//                  :
// Returns          :
// -----------------+

static void GL_MSG_Warning(const char *format, ...)
{
	char str[4096] = "";
	va_list arglist;

	va_start(arglist, format);
	vsnprintf(str, 4096, format, arglist);
	va_end(arglist);

#ifdef HAVE_SDL
	CONS_Printf("OpenGL warning: %s", str);
#endif
#ifdef DEBUG_TO_FILE
	if (!gllogstream)
		gllogstream = fopen("ogllog.txt", "w");
	fwrite(str, strlen(str), 1, gllogstream);
#endif
}

#ifdef STATIC_OPENGL
/* 1.0 functions */
/* Miscellaneous */
#define pglClearColor glClearColor
//glClear
#define pglColorMask glColorMask
#define pglAlphaFunc glAlphaFunc
#define pglBlendFunc glBlendFunc
#define pglCullFace glCullFace
#define pglPolygonMode glPolygonMode
#define pglPolygonOffset glPolygonOffset
#define pglScissor glScissor
#define pglEnable glEnable
#define pglDisable glDisable

/* Depth Buffer */
#define pglClearDepth glClearDepth
#define pglDepthFunc glDepthFunc
#define pglDepthMask glDepthMask
#define pglDepthRange glDepthRange

/* Transformation */
#define pglMatrixMode glMatrixMode
#define pglViewport glViewport
#define pglPushMatrix glPushMatrix
#define pglPopMatrix glPopMatrix
#define pglLoadIdentity glLoadIdentity
#define pglMultMatrixf glMultMatrixf
#define pglRotatef glRotatef
#define pglScalef glScalef
#define pglTranslatef glTranslatef

/* Drawing Functions */
#define pglBegin glBegin
#define pglEnd glEnd
#define pglVertex3f glVertex3f
#define pglNormal3f glNormal3f
#define pglColor4f glColor4f
#define pglColor4fv glColor4fv
#define pglTexCoord2f glTexCoord2f

/* Lighting */
#define pglShadeModel glShadeModel
#define pglLightfv glLightfv
#define pglLightModelfv glLightModelfv
#define pglMaterialfv glMaterialfv

/* Raster functions */
#define pglPixelStorei glPixelStorei
#define pglReadPixels glReadPixels

/* Texture mapping */
#define pglTexEnvi glTexEnvi
#define pglTexParameteri glTexParameteri
#define pglTexImage2D glTexImage2D

/* Fog */
#define pglFogf glFogf
#define pglFogfv glFogfv

/* 1.1 functions */
/* texture objects */ //GL_EXT_texture_object
#define pglGenTextures glGenTextures
#define pglDeleteTextures glDeleteTextures
#define pglBindTexture glBindTexture
/* texture mapping */ //GL_EXT_copy_texture
#define pglCopyTexImage2D glCopyTexImage2D
#define pglCopyTexSubImage2D glCopyTexSubImage2D

#else //!STATIC_OPENGL

/* 1.0 functions */
/* Miscellaneous */
typedef void (APIENTRY * PFNglClearColor) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
static PFNglClearColor pglClearColor;
//glClear
typedef void (APIENTRY * PFNglColorMask) (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
static PFNglColorMask pglColorMask;
typedef void (APIENTRY * PFNglAlphaFunc) (GLenum func, GLclampf ref);
static PFNglAlphaFunc pglAlphaFunc;
typedef void (APIENTRY * PFNglBlendFunc) (GLenum sfactor, GLenum dfactor);
static PFNglBlendFunc pglBlendFunc;
typedef void (APIENTRY * PFNglCullFace) (GLenum mode);
static PFNglCullFace pglCullFace;
typedef void (APIENTRY * PFNglPolygonMode) (GLenum face, GLenum mode);
static PFNglPolygonMode pglPolygonMode;
typedef void (APIENTRY * PFNglPolygonOffset) (GLfloat factor, GLfloat units);
static PFNglPolygonOffset pglPolygonOffset;
typedef void (APIENTRY * PFNglScissor) (GLint x, GLint y, GLsizei width, GLsizei height);
static PFNglScissor pglScissor;
typedef void (APIENTRY * PFNglEnable) (GLenum cap);
static PFNglEnable pglEnable;
typedef void (APIENTRY * PFNglDisable) (GLenum cap);
static PFNglDisable pglDisable;
typedef void (APIENTRY * PFNglGetFloatv) (GLenum pname, GLfloat *params);
static PFNglGetFloatv pglGetFloatv;

/* Depth Buffer */
typedef void (APIENTRY * PFNglClearDepth) (GLclampd depth);
static PFNglClearDepth pglClearDepth;
typedef void (APIENTRY * PFNglDepthFunc) (GLenum func);
static PFNglDepthFunc pglDepthFunc;
typedef void (APIENTRY * PFNglDepthMask) (GLboolean flag);
static PFNglDepthMask pglDepthMask;
typedef void (APIENTRY * PFNglDepthRange) (GLclampd near_val, GLclampd far_val);
static PFNglDepthRange pglDepthRange;

/* Transformation */
typedef void (APIENTRY * PFNglMatrixMode) (GLenum mode);
static PFNglMatrixMode pglMatrixMode;
typedef void (APIENTRY * PFNglViewport) (GLint x, GLint y, GLsizei width, GLsizei height);
static PFNglViewport pglViewport;
typedef void (APIENTRY * PFNglPushMatrix) (void);
static PFNglPushMatrix pglPushMatrix;
typedef void (APIENTRY * PFNglPopMatrix) (void);
static PFNglPopMatrix pglPopMatrix;
typedef void (APIENTRY * PFNglLoadIdentity) (void);
static PFNglLoadIdentity pglLoadIdentity;
typedef void (APIENTRY * PFNglMultMatrixf) (const GLfloat *m);
static PFNglMultMatrixf pglMultMatrixf;
typedef void (APIENTRY * PFNglRotatef) (GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
static PFNglRotatef pglRotatef;
typedef void (APIENTRY * PFNglScalef) (GLfloat x, GLfloat y, GLfloat z);
static PFNglScalef pglScalef;
typedef void (APIENTRY * PFNglTranslatef) (GLfloat x, GLfloat y, GLfloat z);
static PFNglTranslatef pglTranslatef;

/* Drawing Functions */
typedef void (APIENTRY * PFNglBegin) (GLenum mode);
static PFNglBegin pglBegin;
typedef void (APIENTRY * PFNglEnd) (void);
static PFNglEnd pglEnd;
typedef void (APIENTRY * PFNglVertex3f) (GLfloat x, GLfloat y, GLfloat z);
static PFNglVertex3f pglVertex3f;
typedef void (APIENTRY * PFNglNormal3f) (GLfloat x, GLfloat y, GLfloat z);
static PFNglNormal3f pglNormal3f;
typedef void (APIENTRY * PFNglColor4f) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
static PFNglColor4f pglColor4f;
typedef void (APIENTRY * PFNglColor4fv) (const GLfloat *v);
static PFNglColor4fv pglColor4fv;
typedef void (APIENTRY * PFNglTexCoord2f) (GLfloat s, GLfloat t);
static PFNglTexCoord2f pglTexCoord2f;

/* Lighting */
typedef void (APIENTRY * PFNglShadeModel) (GLenum mode);
static PFNglShadeModel pglShadeModel;
typedef void (APIENTRY * PFNglLightfv) (GLenum light, GLenum pname, GLfloat *params);
static PFNglLightfv pglLightfv;
typedef void (APIENTRY * PFNglLightModelfv) (GLenum pname, GLfloat *params);
static PFNglLightModelfv pglLightModelfv;
typedef void (APIENTRY * PFNglMaterialfv) (GLint face, GLenum pname, GLfloat *params);
static PFNglMaterialfv pglMaterialfv;

/* Raster functions */
typedef void (APIENTRY * PFNglPixelStorei) (GLenum pname, GLint param);
static PFNglPixelStorei pglPixelStorei;
typedef void (APIENTRY  * PFNglReadPixels) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
static PFNglReadPixels pglReadPixels;

/* Texture mapping */
typedef void (APIENTRY * PFNglTexEnvi) (GLenum target, GLenum pname, GLint param);
static PFNglTexEnvi pglTexEnvi;
typedef void (APIENTRY * PFNglTexParameteri) (GLenum target, GLenum pname, GLint param);
static PFNglTexParameteri pglTexParameteri;
typedef void (APIENTRY * PFNglTexImage2D) (GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
static PFNglTexImage2D pglTexImage2D;

/* Fog */
typedef void (APIENTRY * PFNglFogf) (GLenum pname, GLfloat param);
static PFNglFogf pglFogf;
typedef void (APIENTRY * PFNglFogfv) (GLenum pname, const GLfloat *params);
static PFNglFogfv pglFogfv;

/* 1.1 functions */
/* texture objects */ //GL_EXT_texture_object
typedef void (APIENTRY * PFNglGenTextures) (GLsizei n, const GLuint *textures);
static PFNglGenTextures pglGenTextures;
typedef void (APIENTRY * PFNglDeleteTextures) (GLsizei n, const GLuint *textures);
static PFNglDeleteTextures pglDeleteTextures;
typedef void (APIENTRY * PFNglBindTexture) (GLenum target, GLuint texture);
static PFNglBindTexture pglBindTexture;
/* texture mapping */ //GL_EXT_copy_texture
typedef void (APIENTRY * PFNglCopyTexImage2D) (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
static PFNglCopyTexImage2D pglCopyTexImage2D;
typedef void (APIENTRY * PFNglCopyTexSubImage2D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
static PFNglCopyTexSubImage2D pglCopyTexSubImage2D;
#endif
/* GLU functions */
typedef GLint (APIENTRY * PFNgluBuild2DMipmaps) (GLenum target, GLint internalFormat, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *data);
static PFNgluBuild2DMipmaps pgluBuild2DMipmaps;

/* 1.2 Parms */
/* GL_CLAMP_TO_EDGE_EXT */
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif
#ifndef GL_TEXTURE_MIN_LOD
#define GL_TEXTURE_MIN_LOD 0x813A
#endif
#ifndef GL_TEXTURE_MAX_LOD
#define GL_TEXTURE_MAX_LOD 0x813B
#endif

/* 1.3 GL_TEXTUREi */
#ifndef GL_TEXTURE0
#define GL_TEXTURE0 0x84C0
#endif
#ifndef GL_TEXTURE1
#define GL_TEXTURE1 0x84C1
#endif

boolean SetupGLfunc(void)
{
#ifndef STATIC_OPENGL
#define GETOPENGLFUNC(func, proc) \
	func = GetGLFunc(#proc); \
	if (!func) \
	{ \
		GL_MSG_Warning("failed to get OpenGL function: %s", #proc); \
	} \

	GETOPENGLFUNC(pglClearColor, glClearColor)

	GETOPENGLFUNC(pglClear , glClear)
	GETOPENGLFUNC(pglColorMask , glColorMask)
	GETOPENGLFUNC(pglAlphaFunc , glAlphaFunc)
	GETOPENGLFUNC(pglBlendFunc , glBlendFunc)
	GETOPENGLFUNC(pglCullFace , glCullFace)
	GETOPENGLFUNC(pglPolygonMode , glPolygonMode)
	GETOPENGLFUNC(pglPolygonOffset , glPolygonOffset)
	GETOPENGLFUNC(pglScissor , glScissor)
	GETOPENGLFUNC(pglEnable , glEnable)
	GETOPENGLFUNC(pglDisable , glDisable)
	GETOPENGLFUNC(pglGetFloatv , glGetFloatv)
	GETOPENGLFUNC(pglGetIntegerv , glGetIntegerv)
	GETOPENGLFUNC(pglGetString , glGetString)

	GETOPENGLFUNC(pglClearDepth , glClearDepth)
	GETOPENGLFUNC(pglDepthFunc , glDepthFunc)
	GETOPENGLFUNC(pglDepthMask , glDepthMask)
	GETOPENGLFUNC(pglDepthRange , glDepthRange)

	GETOPENGLFUNC(pglMatrixMode , glMatrixMode)
	GETOPENGLFUNC(pglViewport , glViewport)
	GETOPENGLFUNC(pglPushMatrix , glPushMatrix)
	GETOPENGLFUNC(pglPopMatrix , glPopMatrix)
	GETOPENGLFUNC(pglLoadIdentity , glLoadIdentity)
	GETOPENGLFUNC(pglMultMatrixf , glMultMatrixf)
	GETOPENGLFUNC(pglRotatef , glRotatef)
	GETOPENGLFUNC(pglScalef , glScalef)
	GETOPENGLFUNC(pglTranslatef , glTranslatef)

	GETOPENGLFUNC(pglBegin , glBegin)
	GETOPENGLFUNC(pglEnd , glEnd)
	GETOPENGLFUNC(pglVertex3f , glVertex3f)
	GETOPENGLFUNC(pglNormal3f , glNormal3f)
	GETOPENGLFUNC(pglColor4f , glColor4f)
	GETOPENGLFUNC(pglColor4fv , glColor4fv)
	GETOPENGLFUNC(pglTexCoord2f , glTexCoord2f)

	GETOPENGLFUNC(pglShadeModel , glShadeModel)
	GETOPENGLFUNC(pglLightfv, glLightfv)
	GETOPENGLFUNC(pglLightModelfv , glLightModelfv)
	GETOPENGLFUNC(pglMaterialfv , glMaterialfv)

	GETOPENGLFUNC(pglPixelStorei , glPixelStorei)
	GETOPENGLFUNC(pglReadPixels , glReadPixels)

	GETOPENGLFUNC(pglTexEnvi , glTexEnvi)
	GETOPENGLFUNC(pglTexParameteri , glTexParameteri)
	GETOPENGLFUNC(pglTexImage2D , glTexImage2D)

	GETOPENGLFUNC(pglFogf , glFogf)
	GETOPENGLFUNC(pglFogfv , glFogfv)

	GETOPENGLFUNC(pglGenTextures , glGenTextures)
	GETOPENGLFUNC(pglDeleteTextures , glDeleteTextures)
	GETOPENGLFUNC(pglBindTexture , glBindTexture)

	GETOPENGLFUNC(pglCopyTexImage2D , glCopyTexImage2D)
	GETOPENGLFUNC(pglCopyTexSubImage2D , glCopyTexSubImage2D)

#undef GETOPENGLFUNC

	pgluBuild2DMipmaps = GetGLFunc("gluBuild2DMipmaps");

#endif
	return true;
}


// -----------------+
// SetNoTexture     : Disable texture
// -----------------+
static void SetNoTexture( void )
{
	// Set small white texture.
	if( tex_downloaded != NOTEXTURE_NUM )
	{
		pglBindTexture( GL_TEXTURE_2D, NOTEXTURE_NUM );
		tex_downloaded = NOTEXTURE_NUM;
	}
}

static void GLPerspective(GLfloat fovy, GLfloat aspect)
{
	GLfloat m[4][4] =
	{
		{ 1.0f, 0.0f, 0.0f, 0.0f},
		{ 0.0f, 1.0f, 0.0f, 0.0f},
		{ 0.0f, 0.0f, 1.0f,-1.0f},
		{ 0.0f, 0.0f, 0.0f, 0.0f},
	};
	const GLfloat zNear = NEAR_CLIPPING_PLANE;
	const GLfloat zFar = FAR_CLIPPING_PLANE;
	const GLfloat radians = (GLfloat)(fovy / 2.0f * M_PIl / 180.0f);
	const GLfloat sine = sin(radians);
	const GLfloat deltaZ = zFar - zNear;
	GLfloat cotangent;

	if ((fabsf((float)deltaZ) < 1.0E-36f) || fpclassify(sine) == FP_ZERO || fpclassify(aspect) == FP_ZERO)
	{
		return;
	}
	cotangent = cosf(radians) / sine;

	m[0][0] = cotangent / aspect;
	m[1][1] = cotangent;
	m[2][2] = -(zFar + zNear) / deltaZ;
	m[3][2] = -2.0f * zNear * zFar / deltaZ;

	pglMultMatrixf(&m[0][0]);
}

static void GLProject(GLfloat objX, GLfloat objY, GLfloat objZ,
                      GLfloat* winX, GLfloat* winY, GLfloat* winZ)
{
	GLfloat in[4], out[4];
	int i;

	for (i=0; i<4; i++)
	{
		out[i] =
			objX * modelMatrix[0*4+i] +
			objY * modelMatrix[1*4+i] +
			objZ * modelMatrix[2*4+i] +
			modelMatrix[3*4+i];
	}
	for (i=0; i<4; i++)
	{
		in[i] =
			out[0] * projMatrix[0*4+i] +
			out[1] * projMatrix[1*4+i] +
			out[2] * projMatrix[2*4+i] +
			out[3] * projMatrix[3*4+i];
	}
	if (fpclassify(in[3]) == FP_ZERO) return;
	in[0] /= in[3];
	in[1] /= in[3];
	in[2] /= in[3];
	/* Map x, y and z to range 0-1 */
	in[0] = in[0] * 0.5f + 0.5f;
	in[1] = in[1] * 0.5f + 0.5f;
	in[2] = in[2] * 0.5f + 0.5f;

	/* Map x,y to viewport */
	in[0] = in[0] * viewport[2] + viewport[0];
	in[1] = in[1] * viewport[3] + viewport[1];

	*winX=in[0];
	*winY=in[1];
	*winZ=in[2];
}


// -----------------+
// SetModelView     :
// -----------------+
void SetModelView( GLint w, GLint h )
{
	GL_DBG_Printf( "SetModelView(): %dx%d\n", w, h );

	// The screen textures need to be flushed if the width or height change so that they be remade for the correct size
	if (screen_width != w || screen_height != h)
		FlushScreenTextures();

	screen_width = w;
	screen_height = h;

	pglViewport( 0, 0, w, h );

	pglMatrixMode( GL_PROJECTION );
	pglLoadIdentity();

	pglMatrixMode( GL_MODELVIEW );
	pglLoadIdentity();

	GLPerspective(fov, ASPECT_RATIO);
	//glScalef(1.0f, 320.0f/200.0f, 1.0f);  // gr_scalefrustum (ORIGINAL_ASPECT)

	// added for new coronas' code (without depth buffer)
	pglGetIntegerv(GL_VIEWPORT, viewport);
	pglGetFloatv(GL_PROJECTION_MATRIX, projMatrix);
}


// -----------------+
// SetStates        : Set permanent states
// -----------------+
void SetStates( void )
{
	// Bind little white RGBA texture to ID NOTEXTURE_NUM.
	FUINT Data[8*8];
	int i;

	GL_DBG_Printf( "SetStates()\n" );

	// Hurdler: not necessary, is it?
	//pglShadeModel( GL_SMOOTH );      // iterate vertice colors
	pglShadeModel( GL_FLAT );

	pglEnable( GL_TEXTURE_2D );      // two-dimensional texturing
	pglTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE ); //MODULATE );

	pglAlphaFunc( GL_NOTEQUAL, 0.0f );
	pglEnable( GL_ALPHA_TEST );     // enable alpha testing
	//pglBlendFunc( GL_ONE, GL_ZERO ); // copy pixel to frame buffer (opaque)
	pglEnable( GL_BLEND );           // enable color blending

	pglColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );

//   pglDisable(GL_DITHER);         // faB: ??? (undocumented in OpenGL 1.1)
								  // Hurdler: yes, it is!
	pglEnable( GL_DEPTH_TEST );    // check the depth buffer
	pglDepthMask( GL_TRUE );             // enable writing to depth buffer
	pglClearDepth( 1.0 );
	pglDepthRange( 0.0f, 1.0f );
	pglDepthFunc(GL_LEQUAL);

	CurrentPolyFlags = PF_Occlude;

	for(i=0; i<64; i++ )
		Data[i] = 0xffFFffFF;       // white pixel

	pglGenTextures(1, &NOTEXTURE_NUM);
	pglBindTexture( GL_TEXTURE_2D, NOTEXTURE_NUM );
	tex_downloaded = NOTEXTURE_NUM;
	pglTexImage2D( GL_TEXTURE_2D, 0, 4, 8, 8, 0, GL_RGBA, GL_UNSIGNED_BYTE, Data );

	pglPolygonOffset(-1.0, -1.0);

	//pglEnable(GL_CULL_FACE);
	//pglCullFace(GL_FRONT);
	//pglPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//pglPolygonMode(GL_FRONT, GL_LINE);

	//pglFogi(GL_FOG_MODE, GL_EXP);
	//pglHint(GL_FOG_HINT, GL_NICEST);
	//pglFogfv(GL_FOG_COLOR, fogcolor);
	//pglFogf(GL_FOG_DENSITY, 0.0005);

	// bp : when no t&l :)
	pglLoadIdentity();
	pglScalef(1.0, 1.0f, -1.0f);
	pglGetFloatv(GL_MODELVIEW_MATRIX, modelMatrix); // added for new coronas' code (without depth buffer)
}


// -----------------+
// Flush            : flush OpenGL textures
//                  : Clear list of downloaded mipmaps
// -----------------+
void Flush( void )
{
	//GL_DBG_Printf ("HWR_Flush()\n");

	while( gr_cachehead )
	{
		// ceci n'est pas du tout necessaire vu que tu les a charger normalement et
		// donc il sont dans ta liste !
#if 0
		//Hurdler: 25/04/2000: now support colormap in hardware mode
		FTextureInfo    *tmp = gr_cachehead->nextskin;

		// The memory should be freed in the main code
		while (tmp)
		{
			pglDeleteTextures( 1, &tmp->downloaded );
			tmp->downloaded = 0;
			tmp = tmp->nextcolormap;
		}
#endif
		pglDeleteTextures( 1, &gr_cachehead->downloaded );
		gr_cachehead->downloaded = 0;
		gr_cachehead = gr_cachehead->nextmipmap;
	}
	gr_cachetail = gr_cachehead = NULL; //Hurdler: well, gr_cachehead is already NULL
	tex_downloaded = 0;
}


// -----------------+
// isExtAvailable   : Look if an OpenGL extension is available
// Returns          : true if extension available
// -----------------+
INT32 isExtAvailable(const char *extension, const GLubyte *start)
{
	GLubyte         *where, *terminator;

	if (!extension || !start) return 0;
	where = (GLubyte *) strchr(extension, ' ');
	if (where || *extension == '\0')
		return 0;

	for (;;)
	{
		where = (GLubyte *) strstr((const char *) start, extension);
		if (!where)
			break;
		terminator = where + strlen(extension);
		if (where == start || *(where - 1) == ' ')
			if (*terminator == ' ' || *terminator == '\0')
				return 1;
		start = terminator;
	}
	return 0;
}


// -----------------+
// Init             : Initialise the OpenGL interface API
// Returns          :
// -----------------+
EXPORT boolean HWRAPI( Init ) (I_Error_t FatalErrorFunction)
{
	I_Error_GL = FatalErrorFunction;
	GL_DBG_Printf (DRIVER_STRING);
	return LoadGL();
}


// -----------------+
// ClearMipMapCache : Flush OpenGL textures from memory
// -----------------+
EXPORT void HWRAPI( ClearMipMapCache ) ( void )
{
	// GL_DBG_Printf ("HWR_Flush(exe)\n");
	Flush();
}


// -----------------+
// ReadRect         : Read a rectangle region of the truecolor framebuffer
//                  : store pixels as 16bit 565 RGB
// Returns          : 16bit 565 RGB pixel array stored in dst_data
// -----------------+
EXPORT void HWRAPI( ReadRect ) (int x, int y, int width, int height,
								int dst_stride, unsigned short * dst_data)
{
	INT32 i;
	// GL_DBG_Printf ("ReadRect()\n");
	if (dst_stride == width*3)
	{
		GLubyte*top = (GLvoid*)dst_data, *bottom = top + dst_stride * (height - 1);
		GLubyte *row = malloc(dst_stride);
		if (!row) return;
		pglPixelStorei(GL_PACK_ALIGNMENT, 1);
		pglReadPixels(x, y, width, height, GL_RGB, GL_UNSIGNED_BYTE, dst_data);
		pglPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		for(i = 0; i < height/2; i++)
		{
			memcpy(row, top, dst_stride);
			memcpy(top, bottom, dst_stride);
			memcpy(bottom, row, dst_stride);
			top += dst_stride;
			bottom -= dst_stride;
		}
		free(row);
	}
	else
	{
		INT32 j;
		GLubyte *image = malloc(width*height*3*sizeof (*image));
		if (!image) return;
		pglPixelStorei(GL_PACK_ALIGNMENT, 1);
		pglReadPixels(x, y, width, height, GL_RGB, GL_UNSIGNED_BYTE, image);
		pglPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		for (i = height-1; i >= 0; i--)
		{
			for (j = 0; j < width; j++)
			{
				dst_data[(height-1-i)*width+j] =
				(UINT16)(
								 ((image[(i*width+j)*3]>>3)<<11) |
								 ((image[(i*width+j)*3+1]>>2)<<5) |
								 ((image[(i*width+j)*3+2]>>3)));
			}
		}
		free(image);
	}
}


// -----------------+
// ClipRect         : Defines the 2D hardware clipping window
// -----------------+
EXPORT void HWRAPI( ClipRect ) (int minx, int miny, int maxx, int maxy, float nearclip)
{
	// GL_DBG_Printf ("ClipRect(%d, %d, %d, %d)\n", minx, miny, maxx, maxy);

	pglViewport( minx, screen_height-maxy, maxx-minx, maxy-miny );
	NEAR_CLIPPING_PLANE = nearclip;

	//pglScissor(minx, screen_height-maxy, maxx-minx, maxy-miny);
	pglMatrixMode( GL_PROJECTION );
	pglLoadIdentity();
	GLPerspective(fov, ASPECT_RATIO);
	pglMatrixMode(GL_MODELVIEW);

	// added for new coronas' code (without depth buffer)
	pglGetIntegerv(GL_VIEWPORT, viewport);
	pglGetFloatv(GL_PROJECTION_MATRIX, projMatrix);
}


// -----------------+
// ClearBuffer      : Clear the color/alpha/depth buffer(s)
// -----------------+
EXPORT void HWRAPI( ClearBuffer ) ( FBOOLEAN ColorMask,
									FBOOLEAN DepthMask,
									FRGBAFloat * ClearColor )
{
	// GL_DBG_Printf ("ClearBuffer(%d)\n", alpha);
	FUINT   ClearMask = 0;

	if( ColorMask ) {
		if( ClearColor )
		pglClearColor( ClearColor->red,
					  ClearColor->green,
					  ClearColor->blue,
					  ClearColor->alpha );
		ClearMask |= GL_COLOR_BUFFER_BIT;
	}
	if( DepthMask ) {
		//pglClearDepth( 1.0 );     //Hurdler: all that are permanen states
		//pglDepthRange( 0.0, 1.0 );
		//pglDepthFunc( GL_LEQUAL );
		ClearMask |= GL_DEPTH_BUFFER_BIT;
	}

	SetBlend( DepthMask ? PF_Occlude | CurrentPolyFlags : CurrentPolyFlags&~PF_Occlude );

	pglClear( ClearMask );
}


// -----------------+
// HWRAPI Draw2DLine: Render a 2D line
// -----------------+
EXPORT void HWRAPI( Draw2DLine ) ( F2DCoord * v1,
								   F2DCoord * v2,
								   RGBA_t Color )
{
	FRGBAFloat c;

	// GL_DBG_Printf ("DrawLine() (%f %f %f) %d\n", v1->x, -v1->y, -v1->oow, v1->argb);
	GLfloat x1, x2, x3, x4;
	GLfloat y1, y2, y3, y4;
	GLfloat dx, dy;
	GLfloat angle;

	// BP: we should reflect the new state in our variable
	//SetBlend( PF_Modulated|PF_NoTexture );

	pglDisable( GL_TEXTURE_2D );

	c.red   = int2float[Color.s.red];
	c.green = int2float[Color.s.green];
	c.blue  = int2float[Color.s.blue];
	c.alpha = int2float[Color.s.alpha];

	// This is the preferred, 'modern' way of rendering lines -- creating a polygon.
	if (fabsf(v2->x - v1->x) > FLT_EPSILON)
		angle = (float)atan((v2->y-v1->y)/(v2->x-v1->x));
	else
		angle = N_PI_DEMI;
	dx = (float)sin(angle) / (float)screen_width;
	dy = (float)cos(angle) / (float)screen_height;

	x1 = v1->x - dx;  y1 = v1->y + dy;
	x2 = v2->x - dx;  y2 = v2->y + dy;
	x3 = v2->x + dx;  y3 = v2->y - dy;
	x4 = v1->x + dx;  y4 = v1->y - dy;

	pglColor4f(c.red, c.green, c.blue, c.alpha);
	pglBegin( GL_TRIANGLE_FAN );
		pglVertex3f( x1, -y1, 1 );
		pglVertex3f( x2, -y2, 1 );
		pglVertex3f( x3, -y3, 1 );
		pglVertex3f( x4, -y4, 1 );
	pglEnd();

	pglEnable( GL_TEXTURE_2D );
}

static void Clamp2D(GLenum pname)
{
	pglTexParameteri(GL_TEXTURE_2D, pname, GL_CLAMP_TO_EDGE);
}


// -----------------+
// SetBlend         : Set render mode
// -----------------+
// PF_Masked - we could use an ALPHA_TEST of GL_EQUAL, and alpha ref of 0,
//             is it faster when pixels are discarded ?
EXPORT void HWRAPI( SetBlend ) ( FBITFIELD PolyFlags )
{
	FBITFIELD   Xor = CurrentPolyFlags^PolyFlags;
	if( Xor & ( PF_Blending|PF_Occlude|PF_NoTexture|PF_Modulated|PF_NoDepthTest|PF_Decal|PF_Invisible ) )
	{
		if( Xor&(PF_Blending) ) // if blending mode must be changed
		{
			switch(PolyFlags & PF_Blending) {
				case PF_Translucent & PF_Blending :
					 pglBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ); // alpha = level of transparency
					 pglAlphaFunc( GL_NOTEQUAL, 0.0f );
					 break;
				case PF_Masked & PF_Blending :
					 // Hurdler: does that mean lighting is only made by alpha src?
					 // it sounds ok, but not for polygonsmooth
					 pglBlendFunc( GL_SRC_ALPHA, GL_ZERO );                // 0 alpha = holes in texture
					 pglAlphaFunc( GL_GREATER, 0.5f );
					 break;
				case PF_Additive & PF_Blending :
					 pglBlendFunc( GL_SRC_ALPHA, GL_ONE );                 // src * alpha + dest
					 pglAlphaFunc( GL_NOTEQUAL, 0.0f );
					 break;
				case PF_Environment & PF_Blending :
					 pglBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA );
					 pglAlphaFunc( GL_NOTEQUAL, 0.0f );
					 break;
				case PF_Substractive & PF_Blending :
					 // not realy but what else ?
					 pglBlendFunc( GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
					 pglAlphaFunc( GL_NOTEQUAL, 0.0f );
					 break;
				default :
					 // No blending
					 pglBlendFunc( GL_ONE, GL_ZERO );   // the same as no blending
					 pglAlphaFunc( GL_GREATER, 0.5f );
					 break;
			}
			if( Xor & PF_AlphaTest)
			{
				if( PolyFlags & PF_AlphaTest)
					pglEnable( GL_ALPHA_TEST );      // discard 0 alpha pixels (holes in texture)
				else
					pglDisable( GL_ALPHA_TEST );
			}
		}
		if( Xor & PF_Decal )
		{
			if( PolyFlags & PF_Decal )
				pglEnable(GL_POLYGON_OFFSET_FILL);
			else
				pglDisable(GL_POLYGON_OFFSET_FILL);
		}
		if( Xor&PF_NoDepthTest )
		{
			if( PolyFlags & PF_NoDepthTest )
				pglDisable( GL_DEPTH_TEST );
			else
				pglEnable( GL_DEPTH_TEST );
		}
		if( Xor&PF_Modulated )
		{
			if( PolyFlags & PF_Modulated )
			{   // mix texture colour with Surface->FlatColor
				pglTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
			}
			else
			{   // colour from texture is unchanged before blending
				pglTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
			}
		}
		if( Xor & PF_Occlude ) // depth test but (no) depth write
		{
			if (PolyFlags&PF_Occlude)
				pglDepthMask( GL_TRUE );
			else
				pglDepthMask( GL_FALSE );
		}
		if( Xor & PF_Invisible )
		{
//            glColorMask( (PolyFlags&PF_Invisible)==0, (PolyFlags&PF_Invisible)==0,
//                         (PolyFlags&PF_Invisible)==0, (PolyFlags&PF_Invisible)==0 );

			if (PolyFlags&PF_Invisible)
				pglBlendFunc( GL_ZERO, GL_ONE );         // transparent blending
			else
			{   // big hack: (TODO: manage that better)
				// be sure we are in PF_Masked mode which was overwrited
				if ((PolyFlags&PF_Blending)==PF_Masked)
					pglBlendFunc( GL_SRC_ALPHA, GL_ZERO );
			}
		}
		if( PolyFlags & PF_NoTexture )
		{
			SetNoTexture();
		}
	}
	CurrentPolyFlags = PolyFlags;
}


// -----------------+
// SetTexture       : The mipmap becomes the current texture source
// -----------------+
EXPORT void HWRAPI( SetTexture ) ( FTextureInfo *pTexInfo )
{
	if( pTexInfo->downloaded )
	{
		if (pTexInfo->downloaded != tex_downloaded)
		{
			pglBindTexture(GL_TEXTURE_2D, pTexInfo->downloaded);
			tex_downloaded = pTexInfo->downloaded;
		}
	}
	else
	{
		// Download a mipmap
		static RGBA_t   tex[256*256];
		RGBA_t          *ptex = tex;
		int             w, h;

		// GL_DBG_Printf ("DownloadMipmap()\n");
		w = pTexInfo->width;
		h = pTexInfo->height;

		if( (pTexInfo->grInfo.format==GR_TEXFMT_P_8) ||
			(pTexInfo->grInfo.format==GR_TEXFMT_AP_88) )
		{
			GLubyte *pImgData;
			int i, j;

			pImgData = (GLubyte *)pTexInfo->grInfo.data;
			for( j=0; j<h; j++ )
			{
				for( i=0; i<w; i++)
				{
					if ( (*pImgData==HWR_PATCHES_CHROMAKEY_COLORINDEX) &&
						 (pTexInfo->flags & TF_CHROMAKEYED) )
					{
						tex[w*j+i].s.red   = 0;
						tex[w*j+i].s.green = 0;
						tex[w*j+i].s.blue  = 0;
						tex[w*j+i].s.alpha = 0;
					}
					else
					{
						tex[w*j+i].s.red   = myPaletteData[*pImgData].s.red;
						tex[w*j+i].s.green = myPaletteData[*pImgData].s.green;
						tex[w*j+i].s.blue  = myPaletteData[*pImgData].s.blue;
						tex[w*j+i].s.alpha = 255;
					}

					pImgData++;

					if( pTexInfo->grInfo.format == GR_TEXFMT_AP_88 )
					{
						if( !(pTexInfo->flags & TF_CHROMAKEYED) )
							tex[w*j+i].s.alpha = *pImgData;
						pImgData++;
					}

				}
			}
		}
		else if (pTexInfo->grInfo.format==46)              //DEF46 HACK
		{
			// corona test : passed as ARGB 8888, which is not in glide formats
			// Hurdler: not used for coronas anymore, just for dynamic lighting
			ptex = (RGBA_t *) pTexInfo->grInfo.data;
		}
		else if (pTexInfo->grInfo.format==GR_TEXFMT_ALPHA_INTENSITY_88)
		{
			GLubyte *pImgData;
			int i, j;

			pImgData = (GLubyte *)pTexInfo->grInfo.data;
			for( j=0; j<h; j++ )
			{
				for( i=0; i<w; i++)
				{
					tex[w*j+i].s.red   = *pImgData;
					tex[w*j+i].s.green = *pImgData;
					tex[w*j+i].s.blue  = *pImgData;
					pImgData++;
					tex[w*j+i].s.alpha = *pImgData;
					pImgData++;
				}
			}
		}
		else
			GL_DBG_Printf ("SetTexture(bad format) %d\n", (INT32)pTexInfo->grInfo.format);

		// Lactozilla: No, don't do that, you idiot
		//pTexInfo->downloaded = NextTexAvail++;

		pglGenTextures(1, &tex_downloaded);
		pTexInfo->downloaded = tex_downloaded;
		pglBindTexture( GL_TEXTURE_2D, pTexInfo->downloaded );

		if (pTexInfo->grInfo.format==46)
		{
			if (min_filter & MIPMAP_MASK)
				pgluBuild2DMipmaps( GL_TEXTURE_2D, GL_ALPHA, w, h, GL_RGBA, GL_UNSIGNED_BYTE, ptex );
			else
				pglTexImage2D( GL_TEXTURE_2D, 0, GL_ALPHA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, ptex );
		}
		else if (pTexInfo->grInfo.format==GR_TEXFMT_ALPHA_INTENSITY_88)
		{
			int i, j;

			// hack software pour les bords de la corona
			for (i=0; i<h; i++)
				for (j=0; j<w; j++)
					if (((i-h/2)*(i-h/2))+((j-w/2)*(j-w/2)) > h*w/4)
						tex[w*j+i].s.alpha = 0;

			//pglTexImage2D( GL_TEXTURE_2D, 0, GL_ALPHA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, ptex );
			if (min_filter & MIPMAP_MASK)
				pgluBuild2DMipmaps( GL_TEXTURE_2D, GL_LUMINANCE_ALPHA, w, h, GL_RGBA, GL_UNSIGNED_BYTE, ptex );
			else
				pglTexImage2D( GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, ptex );
		}
		else if (screen_depth > 16)
		{
			if (min_filter & MIPMAP_MASK)
				pgluBuild2DMipmaps( GL_TEXTURE_2D, GL_RGBA, w, h, GL_RGBA, GL_UNSIGNED_BYTE, ptex );
			else
				pglTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, ptex );
		}
		else // this is the mode for 16 bits 3dfx's cards
		{
			if (min_filter & MIPMAP_MASK)
				pgluBuild2DMipmaps( GL_TEXTURE_2D, GL_RGB5_A1, w, h, GL_RGBA, GL_UNSIGNED_BYTE, ptex );
			else
				pglTexImage2D( GL_TEXTURE_2D, 0, GL_RGB5_A1, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, ptex );
		}
		if( pTexInfo->flags & TF_WRAPX )
			pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		else
			Clamp2D(GL_TEXTURE_WRAP_S);

		if( pTexInfo->flags & TF_WRAPY )
			pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		else
			Clamp2D(GL_TEXTURE_WRAP_T);

		pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
		pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);

		pTexInfo->nextmipmap = NULL;
		if (gr_cachetail) { // insertion en fin de liste
			gr_cachetail->nextmipmap = pTexInfo;
			gr_cachetail = pTexInfo;
		}
		else // initialisation de la liste
			gr_cachetail = gr_cachehead =  pTexInfo;
	}
}


// -----------------+
// DrawPolygon      : Render a polygon, set the texture, set render mode
// -----------------+
EXPORT void HWRAPI( DrawPolygon ) ( FSurfaceInfo  *pSurf,
									//FTextureInfo  *pTexInfo,
									FOutVector    *pOutVerts,
									FUINT         iNumPts,
									FBITFIELD     PolyFlags )
{
	FUINT i, j;
	FRGBAFloat c;

	if (PolyFlags & PF_Corona)
		PolyFlags &= ~PF_NoDepthTest;

	SetBlend( PolyFlags );    //TODO: inline (#pragma..)

	// If Modulated, mix the surface colour to the texture
	if( (CurrentPolyFlags & PF_Modulated) && pSurf)
	{
		if (pal_col) { // hack for non-palettized mode
			c.red   = (const_pal_col.red  +int2float[pSurf->FlatColor.s.red])  /2.0f;
			c.green = (const_pal_col.green+int2float[pSurf->FlatColor.s.green])/2.0f;
			c.blue  = (const_pal_col.blue +int2float[pSurf->FlatColor.s.blue]) /2.0f;
			c.alpha = int2float[pSurf->FlatColor.s.alpha];
		}
		else
		{
			c.red   = int2float[pSurf->FlatColor.s.red];
			c.green = int2float[pSurf->FlatColor.s.green];
			c.blue  = int2float[pSurf->FlatColor.s.blue];
			c.alpha = int2float[pSurf->FlatColor.s.alpha];
		}
		pglColor4fv( (float *)&c );    // is in RGBA float format
	}

	// this test is added for new coronas' code (without depth buffer)
	// I think I should do a separate function for drawing coronas, so it will be a little faster
	if (PolyFlags & PF_Corona) // check to see if we need to draw the corona
	{
		//rem: all 8 (or 8.0f) values are hard coded: it can be changed to a higher value
		GLfloat     buf[8][8];
		GLfloat     cx, cy, cz;
		GLfloat     px = 0.0f, py = 0.0f, pz = 0.0f;
		GLfloat     scalef = 0;

		cx = (pOutVerts[0].x + pOutVerts[2].x) / 2.0f; // we should change the coronas' ...
		cy = (pOutVerts[0].y + pOutVerts[2].y) / 2.0f; // ... code so its only done once.
		cz = pOutVerts[0].oow;

		// I dont know if this is slow or not
		GLProject(cx, cy, cz, &px, &py, &pz);
		//GL_DBG_Printf("Projection: (%f, %f, %f)\n", px, py, pz);

		if ( (pz <  0.0) ||
			 (px < -8.0) ||
			 (py < viewport[1]-8.0) ||
			 (px > viewport[2]+8.0) ||
			 (py > viewport[1]+viewport[3]+8.0))
			return;

		// the damned slow glReadPixels functions :(
		pglReadPixels( (int)px-4, (int)py, 8, 8, GL_DEPTH_COMPONENT, GL_FLOAT, buf );
		//GL_DBG_Printf("DepthBuffer: %f %f\n", buf[0][0], buf[3][3]);

		for (i=0; i<8; i++)
			for (j=0; j<8; j++)
				scalef += (pz > buf[i][j]) ? 0 : 1;

		// quick test for screen border (not 100% correct, but looks ok)
		if (px < 4) scalef -= 8*(4-px);
		if (py < viewport[1]+4) scalef -= 8*(viewport[1]+4-py);
		if (px > viewport[2]-4) scalef -= 8*(4-(viewport[2]-px));
		if (py > viewport[1]+viewport[3]-4) scalef -= 8*(4-(viewport[1]+viewport[3]-py));

		scalef /= 64;
		//GL_DBG_Printf("Scale factor: %f\n", scalef);

		if (scalef < 0.05f) // �a sert � rien de tracer la light
			return;

		c.alpha *= scalef; // change the alpha value (it seems better than changing the size of the corona)
		pglColor4fv( (float *)&c );
	}

	pglBegin( GL_TRIANGLE_FAN );
	for( i=0; i<iNumPts; i++ )
	{
		pglTexCoord2f( pOutVerts[i].sow, pOutVerts[i].tow );
		//Hurdler: test code: -pOutVerts[i].oow => pOutVerts[i].oow
		pglVertex3f( pOutVerts[i].x, pOutVerts[i].y, pOutVerts[i].oow );
		//pglVertex3f( pOutVerts[i].x, pOutVerts[i].y, -pOutVerts[i].oow );
	}
	pglEnd();
}


// ==========================================================================
//
// ==========================================================================
EXPORT void HWRAPI( SetSpecialState ) (hwdspecialstate_t IdState, int Value)
{
	switch (IdState)
	{
#if 0
		case 77: {
			//08/01/00: Hurdler this is a test for mirror
			if (!Value)
				ClearBuffer( false, true, 0 ); // clear depth buffer
			break;
		}
#endif

		case HWD_SET_PALETTECOLOR: {
			pal_col = Value;
			const_pal_col.blue  = int2float[((Value>>16)&0xff)];
			const_pal_col.green = int2float[((Value>>8)&0xff)];
			const_pal_col.red   = int2float[((Value)&0xff)];
			break;
		}

		case HWD_SET_FOG_COLOR: {
			GLfloat fogcolor[4];

			fogcolor[0] = int2float[((Value>>16)&0xff)];
			fogcolor[1] = int2float[((Value>>8)&0xff)];
			fogcolor[2] = int2float[((Value)&0xff)];
			fogcolor[3] = 0x0;
			pglFogfv(GL_FOG_COLOR, fogcolor);
			break;
		}
		case HWD_SET_FOG_DENSITY:
			pglFogf(GL_FOG_DENSITY, Value/1000000.0f);
			break;

		case HWD_SET_FOG_MODE:
			if (Value)
			{
				pglEnable(GL_FOG);
				// experimental code
				/*
				switch (Value)
				{
					case 1:
						glFogi(GL_FOG_MODE, GL_LINEAR);
						glFogf(GL_FOG_START, -1000.0f);
						glFogf(GL_FOG_END, 2000.0f);
						break;
					case 2:
						glFogi(GL_FOG_MODE, GL_EXP);
						break;
					case 3:
						glFogi(GL_FOG_MODE, GL_EXP2);
						break;
				}
				*/
			}
			else
				pglDisable(GL_FOG);
			break;

		case HWD_SET_FOV:
			fov = (float)Value;

			pglMatrixMode(GL_PROJECTION);
			pglLoadIdentity();
			GLPerspective(fov, ASPECT_RATIO);
			pglGetFloatv(GL_PROJECTION_MATRIX, projMatrix); // added for new coronas' code (without depth buffer)
			pglMatrixMode(GL_MODELVIEW);
			break;

		case HWD_SET_POLYGON_SMOOTH:
			if (Value)
				pglEnable(GL_POLYGON_SMOOTH);
			else
				pglDisable(GL_POLYGON_SMOOTH);
			break;

		case HWD_SET_TEXTUREFILTERMODE:
			switch (Value)
			{
				case HWD_SET_TEXTUREFILTER_TRILINEAR:
					min_filter = mag_filter = GL_LINEAR_MIPMAP_LINEAR;
					break;
				case HWD_SET_TEXTUREFILTER_BILINEAR :
					min_filter = mag_filter = GL_LINEAR;
					break;
				case HWD_SET_TEXTUREFILTER_POINTSAMPLED :
					min_filter = mag_filter = GL_NEAREST;
					break;
				case HWD_SET_TEXTUREFILTER_MIXED1 :
					mag_filter = GL_LINEAR;
					min_filter = GL_NEAREST;
					break;
				case HWD_SET_TEXTUREFILTER_MIXED2 :
					mag_filter = GL_NEAREST;
					min_filter = GL_LINEAR;
					break;
			}
			Flush(); //??? if we want to change filter mode by texture, remove this

		 default:
			break;
	}
}

static FTransform  md2_transform;

// -----------------+
// HWRAPI DrawMD2   : Draw an MD2 model with glcommands
// -----------------+
//EXPORT void HWRAPI( DrawMD2 ) (md2_model_t *model, int frame)
EXPORT void HWRAPI( DrawMD2 ) (int *gl_cmd_buffer, md2_frame_t *frame, FTransform *pos)
{
	int     val, count, index;
	GLfloat s, t;

	//TODO: Maybe we can put all this in a display list the first time it's
	//      called and after, use this display list: faster (how much?) but
	//      require more memory (how much?)

	DrawPolygon( NULL, NULL, 0, PF_Masked|PF_Modulated|PF_Occlude|PF_Clip);

	pglPushMatrix(); // should be the same as glLoadIdentity
	pglLoadIdentity();
	pglScalef(1.0, 1.6f, 1.0f);
	pglRotatef(md2_transform.anglex, -1.0f, 0.0f, 0.0f);
	pglTranslatef(pos->x, pos->y, pos->z);
	pglRotatef(pos->angley, 0.0f, 1.0f, 0.0f);

	val = *gl_cmd_buffer++;

	while (val != 0)
	{
		if (val < 0)
		{
			pglBegin (GL_TRIANGLE_FAN);
			count = -val;
		}
		else
		{
			pglBegin (GL_TRIANGLE_STRIP);
			count = val;
		}

		while (count--)
		{
			s = *(float *) gl_cmd_buffer++;
			t = *(float *) gl_cmd_buffer++;
			index = *gl_cmd_buffer++;

			pglTexCoord2f (s, t);
			pglVertex3f (frame->vertices[index].vertex[0]/2.0f,
						frame->vertices[index].vertex[1]/2.0f,
						frame->vertices[index].vertex[2]/2.0f);
		}

		pglEnd ();

		val = *gl_cmd_buffer++;
	}
	pglPopMatrix(); // should be the same as glLoadIdentity
}

// -----------------+
// SetTransform     : TANDL code (not used if TANDL is not defined in hw_xxx)
// -----------------+
EXPORT void HWRAPI( SetTransform ) (FTransform *stransform)
{
	static INT32 special_splitscreen;
	pglLoadIdentity();
	if (stransform)
	{
		boolean fovx90;
		// keep a trace of the transformation for md2
		memcpy(&md2_transform, stransform, sizeof (md2_transform));

		pglScalef(stransform->scalex, stransform->scaley, -stransform->scalez);
		pglRotatef(stransform->anglex       , 1.0f, 0.0f, 0.0f);
		pglRotatef(stransform->angley+270.0f, 0.0f, 1.0f, 0.0f);
		pglTranslatef(-stransform->x, -stransform->z, -stransform->y);

		pglMatrixMode(GL_PROJECTION);
		pglLoadIdentity();
		fovx90 = stransform->fovxangle > 0.0f && fabsf(stransform->fovxangle - 90.0f) < 0.5f;
		special_splitscreen = (stransform->splitscreen && fovx90);
		if (special_splitscreen)
			GLPerspective(53.13l, 2*ASPECT_RATIO);  // 53.13 = 2*atan(0.5)
		else
			GLPerspective(stransform->fovxangle, ASPECT_RATIO);
		pglGetFloatv(GL_PROJECTION_MATRIX, projMatrix); // added for new coronas' code (without depth buffer)
		pglMatrixMode(GL_MODELVIEW);
	}
	else
	{
		pglScalef(1.0f, 1.0f, -1.0f);

		pglMatrixMode(GL_PROJECTION);
		pglLoadIdentity();
		if (special_splitscreen)
			GLPerspective(53.13l, 2*ASPECT_RATIO);  // 53.13 = 2*atan(0.5)
		else
			//Hurdler: is "fov" correct?
			GLPerspective(fov, ASPECT_RATIO);
		pglGetFloatv(GL_PROJECTION_MATRIX, projMatrix); // added for new coronas' code (without depth buffer)
		pglMatrixMode(GL_MODELVIEW);
	}

	pglGetFloatv(GL_MODELVIEW_MATRIX, modelMatrix); // added for new coronas' code (without depth buffer)
}

// Sryder:	This needs to be called whenever the screen changes resolution in order to reset the screen textures to use
//			a new size
EXPORT void HWRAPI(FlushScreenTextures) (void)
{
	pglDeleteTextures(1, &finalScreenTexture);
	finalScreenTexture = 0;
}

EXPORT void HWRAPI(MakeScreenFinalTexture) (void)
{
	INT32 texsize = 2048;
	boolean firstTime = (finalScreenTexture == 0);

	// Use a power of two texture, dammit
	if(screen_width <= 512)
		texsize = 512;
	else if(screen_width <= 1024)
		texsize = 1024;

	// Create screen texture
	if (firstTime)
		pglGenTextures(1, &finalScreenTexture);
	pglBindTexture(GL_TEXTURE_2D, finalScreenTexture);

	if (firstTime)
	{
		pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		Clamp2D(GL_TEXTURE_WRAP_S);
		Clamp2D(GL_TEXTURE_WRAP_T);
		pglCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, texsize, texsize, 0);
	}
	else
		pglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, texsize, texsize);

	tex_downloaded = finalScreenTexture;

}

EXPORT void HWRAPI(DrawScreenFinalTexture)(int width, int height)
{
	float xfix, yfix;
	float origaspect, newaspect;
	float xoff = 1, yoff = 1; // xoffset and yoffset for the polygon to have black bars around the screen
	FRGBAFloat clearColour;
	INT32 texsize = 2048;

	if(screen_width <= 1024)
		texsize = 1024;
	if(screen_width <= 512)
		texsize = 512;

	xfix = 1/((float)(texsize)/((float)((screen_width))));
	yfix = 1/((float)(texsize)/((float)((screen_height))));

	origaspect = (float)screen_width / screen_height;
	newaspect = (float)width / height;
	if (origaspect < newaspect)
	{
		xoff = origaspect / newaspect;
		yoff = 1;
	}
	else if (origaspect > newaspect)
	{
		xoff = 1;
		yoff = newaspect / origaspect;
	}

	pglViewport(0, 0, width, height);

	clearColour.red = clearColour.green = clearColour.blue = 0;
	clearColour.alpha = 1;
	ClearBuffer(true, false, &clearColour);
	pglBindTexture(GL_TEXTURE_2D, finalScreenTexture);
	pglBegin(GL_QUADS);

		pglColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		// Bottom left
		pglTexCoord2f(0.0f, 0.0f);
		pglVertex3f(-xoff, -yoff, 1.0f);

		// Top left
		pglTexCoord2f(0.0f, yfix);
		pglVertex3f(-xoff, yoff, 1.0f);

		// Top right
		pglTexCoord2f(xfix, yfix);
		pglVertex3f(xoff, yoff, 1.0f);

		// Bottom right
		pglTexCoord2f(xfix, 0.0f);
		pglVertex3f(xoff, -yoff, 1.0f);

	pglEnd();

	tex_downloaded = finalScreenTexture;
}

