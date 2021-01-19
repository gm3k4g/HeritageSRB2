// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: m_fixed.c,v 1.2 2000/02/27 00:42:10 hurdler Exp $
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
// $Log: m_fixed.c,v $
// Revision 1.2  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      Fixed point implementation.
//
//-----------------------------------------------------------------------------

#include "i_system.h"
#include "m_fixed.h"

// Fixme. __USE_C_FIXED__ or something.
#ifndef USEASM
fixed_t FixedMul (fixed_t a, fixed_t b)
{
    return ((INT64) a * (INT64) b) >> FRACBITS;
}
fixed_t FixedDiv2 (fixed_t a, fixed_t b)
{
#if 0
    INT64 c;
    c = ((INT64)a<<16) / ((INT64)b);
    return (fixed_t) c;
#endif

    double c;

    c = ((double)a) / ((double)b) * FRACUNIT;

    if (c >= 2147483648.0 || c < -2147483648.0)
        I_Error("FixedDiv: divide by zero");
    return (fixed_t) c;
}

/*
//
// FixedDiv, C version.
//
fixed_t FixedDiv ( fixed_t   a, fixed_t    b )
{
    //I_Error("<a: %ld, b: %ld>",(long)a,(long)b);

    if ( (abs(a)>>14) >= abs(b))
        return (a^b)<0 ? INT32_MIN : INT32_MAX;

    return FixedDiv2 (a,b);
}
*/
#endif // useasm

fixed_t FixedSqrt(fixed_t x)
{
	// The neglected art of Fixed Point arithmetic
	// Jetro Lauha
	// Seminar Presentation
	// Assembly 2006, 3rd- 6th August 2006
	// (Revised: September 13, 2006)
	// URL: http://jet.ro/files/The_neglected_art_of_Fixed_Point_arithmetic_20060913.pdf
	register UINT32 root, remHi, remLo, testDiv, count;
	root = 0;         /* Clear root */
	remHi = 0;        /* Clear high part of partial remainder */
	remLo = x;        /* Get argument into low part of partial remainder */
	count = (15 + (FRACBITS >> 1));    /* Load loop counter */
	do
	{
		remHi = (remHi << 2) | (remLo >> 30); remLo <<= 2;  /* get 2 bits of arg */
		root <<= 1;   /* Get ready for the next bit in the root */
		testDiv = (root << 1) + 1;    /* Test radical */
		if (remHi >= testDiv)
		{
			remHi -= testDiv;
			root += 1;
		}
	} while (count-- != 0);
	return root;
}

fixed_t FixedHypot(fixed_t x, fixed_t y)
{
	fixed_t ax, yx, yx2, yx1;
	if (abs(y) > abs(x)) // |y|>|x|
	{
		ax = abs(y); // |y| => ax
		yx = FixedDiv(x, y); // (x/y)
	}
	else // |x|>|y|
	{
		ax = abs(x); // |x| => ax
		yx = FixedDiv(y, x); // (x/y)
	}
	yx2 = FixedMul(yx, yx); // (x/y)^2
	yx1 = FixedSqrt(1 * FRACUNIT + yx2); // (1 + (x/y)^2)^1/2
	return FixedMul(ax, yx1); // |x|*((1 + (x/y)^2)^1/2)
}
