// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: z_zone.h,v 1.4 2000/07/01 09:23:49 bpereira Exp $
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
// $Log: z_zone.h,v $
// Revision 1.4  2000/07/01 09:23:49  bpereira
// no message
//
// Revision 1.3  2000/04/30 10:30:10  bpereira
// no message
//
// Revision 1.2  2000/02/27 00:42:11  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      Zone Memory Allocation, perhaps NeXT ObjectiveC inspired.
//      Remark: this was the only stuff that, according
//       to John Carmack, might have been useful for
//       Quake.
//
//---------------------------------------------------------------------



#ifndef __Z_ZONE__
#define __Z_ZONE__


#include <stdio.h>
#include "doomtype.h"

#ifdef __GNUC__ // __attribute__ ((X))
#if (__GNUC__ > 4) || (__GNUC__ == 4 && (__GNUC_MINOR__ >= 3 || (__GNUC_MINOR__ == 2 && __GNUC_PATCHLEVEL__ >= 5)))
#define FUNCALLOC(X) __attribute__((alloc_size(X)))
#endif // odd, it is documented in GCC 4.3.0 but it exists in 4.2.4, at least
#endif

#ifndef FUNCALLOC
#define FUNCALLOC(x)
#endif

//
// ZONE MEMORY
// PU - purge tags.
// Tags < 100 are not overwritten until freed.
//
enum
{
	PU_STATIC                = 1, // static entire execution time
	PU_SOUND                 = 2, // static while playing
	PU_MUSIC                 = 3, // static while playing
	PU_DAVE                  = 4, // anything else Dave wants static

	PU_3DFXPATCHINFO         = 5, // 3Dfx GlidePatch_t struct for OpenGl/Glide texture cache
	PU_3DFXPATCHCOLMIPMAP    = 6, // 3Dfx GlideMipmap_t struct colromap variation of patch

	PU_LEVEL                 = 50, // static until level exited
	PU_LEVSPEC               = 51, // a special thinker in a level

	// Tags >= PU_PURGELEVEL are purgable whenever needed.
	PU_PURGELEVEL            = 100, // static entire execution time
	PU_CACHE                 = 101, // static entire execution time
	PU_3DFXCACHE             = 102, // 'second-level' cache for graphics, stored in 3Dfx format and downloaded  as needed
};

//
// Zone memory initialisation
//
void Z_Init(void);

//
// Zone memory allocation
//
// enable ZDEBUG to get the file + line the functions were called from
// for ZZ_Alloc, see doomdef.h
//

// Z_Free and alloc with alignment
#ifdef ZDEBUG
#define Z_Free(p)                 Z_Free2(p, __FILE__, __LINE__)
#define Z_MallocAlign(s,t,u,a)    Z_Malloc2(s, t, u, a, __FILE__, __LINE__)
#define Z_CallocAlign(s,t,u,a)    Z_Calloc2(s, t, u, a, __FILE__, __LINE__)
#define Z_ReallocAlign(p,s,t,u,a) Z_Realloc2(p,s, t, u, a, __FILE__, __LINE__)
void Z_Free2(void *ptr, const char *file, INT32 line);
void *Z_Malloc2(size_t size, INT32 tag, void *user, INT32 alignbits, const char *file, INT32 line) FUNCALLOC(1);
void *Z_Calloc2(size_t size, INT32 tag, void *user, INT32 alignbits, const char *file, INT32 line) FUNCALLOC(1);
void *Z_Realloc2(void *ptr, size_t size, INT32 tag, void *user, INT32 alignbits, const char *file, INT32 line) FUNCALLOC(2);
#else
void Z_Free(void *ptr);
void *Z_MallocAlign(size_t size, INT32 tag, void *user, INT32 alignbits) FUNCALLOC(1);
void *Z_CallocAlign(size_t size, INT32 tag, void *user, INT32 alignbits) FUNCALLOC(1);
void *Z_ReallocAlign(void *ptr, size_t size, INT32 tag, void *user, INT32 alignbits) FUNCALLOC(2);
#endif

// Alloc with no alignment
#define Z_Malloc(s,t,u)    Z_MallocAlign(s, t, u, 0)
#define Z_Calloc(s,t,u)    Z_CallocAlign(s, t, u, 0)
#define Z_Realloc(p,s,t,u) Z_ReallocAlign(p, s, t, u, 0)

// Free all memory by tag
// these don't give line numbers for ZDEBUG currently though
// (perhaps this should be changed in future?)
#define Z_FreeTag(tagnum) Z_FreeTags(tagnum, tagnum)
void Z_FreeTags(INT32 lowtag, INT32 hightag);

//
// Utility functions
//
void Z_CheckHeap(INT32 i);

//
// Zone memory modification
//
// enable PARANOIA to get the file + line the functions were called from
//
#ifdef PARANOIA
#define Z_ChangeTag(p,t) Z_ChangeTag2(p, t, __FILE__, __LINE__)
#define Z_SetUser(p,u)   Z_SetUser2(p, u, __FILE__, __LINE__)
void Z_ChangeTag2(void *ptr, INT32 tag, const char *file, INT32 line);
void Z_SetUser2(void *ptr, void **newuser, const char *file, INT32 line);
#else
void Z_ChangeTag(void *ptr, INT32 tag);
void Z_SetUser(void *ptr, void **newuser);
#endif

//
// Zone memory usage
//
// Note: These give the memory used in bytes,
// shift down by 10 to convert to KB
//
#define Z_TagUsage(tagnum) Z_TagsUsage(tagnum, tagnum)
size_t Z_TagsUsage(INT32 lowtag, INT32 hightag);
#define Z_TotalUsage() Z_TagsUsage(0, INT32_MAX)

//
// Miscellaneous functions
//
char *Z_StrDup(const char *in);

#endif
