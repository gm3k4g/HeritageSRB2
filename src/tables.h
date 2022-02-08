// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: tables.h,v 1.2 2000/02/27 00:42:11 hurdler Exp $
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
// $Log: tables.h,v $
// Revision 1.2  2000/02/27 00:42:11  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      Lookup tables.
//      Do not try to look them up :-).
//      In the order of appearance:
//
//      int finetangent[4096]   - Tangens LUT.
//       Should work with BAM fairly well (12 of 16bit,
//      effectively, by shifting).
//
//      int finesine[10240]             - Sine lookup.
//
//      int tantoangle[2049]    - ArcTan LUT,
//        maps tan(angle) to angle fast. Gotta search.
//
//-----------------------------------------------------------------------------


#ifndef __TABLES__
#define __TABLES__

#ifdef LINUX
#include <math.h>
#else
//#define PI                              3.141592657
#endif

#include "m_fixed.h"

#define FINEANGLES              8192
#define FINEMASK                (FINEANGLES-1)
#define ANGLETOFINESHIFT        19      // 0x100000000 to 0x2000


// Effective size is 10240.
extern  fixed_t         finesine[5*FINEANGLES/4];

// Re-use data, is just PI/2 phase shift.
extern  fixed_t*        finecosine;


// Effective size is 4096.
extern fixed_t          finetangent[FINEANGLES/2];

#define ANG45           0x20000000
#define ANG90           0x40000000
#define ANG180          0x80000000
#define ANG270          0xc0000000

#define ANGLE_45    0x20000000
#define ANGLE_90    0x40000000
#define ANGLE_180   0x80000000
#define ANGLE_MAX   0xffffffff
#define ANGLE_1     (ANGLE_45/45)
#define ANGLE_60    (ANGLE_180/3)

// Lactozilla: New names
#define ANG1   0x00B60B61 //0.B6~
#define ANG2   0x016C16C1 //.6C1~
#define ANG10  0x071C71C7 //.1C7~
#define ANG15  0x0AAAAAAB //A.AA~
#define ANG20  0x0E38E38E //.38E~
#define ANG30  0x15555555 //.555~
#define ANG60  0x2AAAAAAB //A.AA~
#define ANG64h 0x2DDDDDDE //D.DD~
#define ANG105 0x4AAAAAAB //A.AA~
#define ANG210 0x95555555 //.555~
#define ANG255 0xB5555555 //.555~
#define ANG340 0xF1C71C72 //1.C7~
#define ANG350 0xF8E38E39 //8.E3~

#define ANGLE_11hh 0x08000000
#define ANGLE_22h  0x10000000
#define ANGLE_45   0x20000000
#define ANGLE_67h  0x30000000
#define ANGLE_90   0x40000000
#define ANGLE_112h 0x50000000
#define ANGLE_135  0x60000000
#define ANGLE_157h 0x70000000
#define ANGLE_180  0x80000000
#define ANGLE_202h 0x90000000
#define ANGLE_225  0xA0000000
#define ANGLE_247h 0xB0000000
#define ANGLE_270  0xC0000000
#define ANGLE_292h 0xD0000000
#define ANGLE_315  0xE0000000
#define ANGLE_337h 0xF0000000

typedef unsigned angle_t;


// to get a global angle from cartesian coordinates, the coordinates are
// flipped until they are in the first octant of the coordinate system, then
// the y (<=x) is scaled and divided by x to get a tangent (slope) value
// which is looked up in the tantoangle[] table.
#define SLOPERANGE  2048
#define SLOPEBITS   11
#define DBITS       (FRACBITS-SLOPEBITS)

// The +1 size is to handle the case when x==y without additional checking.
extern  angle_t     tantoangle[SLOPERANGE+1];

// Utility function, called by R_PointToAngle.
int SlopeDiv ( unsigned      num,
               unsigned      den);

FUNCMATH FUNCINLINE static ATTRINLINE angle_t InvAngle(angle_t a)
{
	return (ANGLE_MAX-a)+1;
}
// angle_t to fixed_t f(ANGLE_45) = 45*FRACUNIT
FUNCMATH fixed_t AngleFixed(angle_t af);
// fixed_t to angle_t f(45*FRACUNIT) = ANGLE_45
FUNCMATH angle_t FixedAngle(fixed_t fa);


#endif
