// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: d_netfil.h,v 1.4 2000/04/16 18:38:07 bpereira Exp $
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
// $Log: d_netfil.h,v $
// Revision 1.4  2000/04/16 18:38:07  bpereira
// no message
//
// Revision 1.3  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.2  2000/02/26 00:28:42  hurdler
// Mostly bug fix (see borislog.txt 23-2-2000, 24-2-2000)
//
//
// DESCRIPTION:
//
//
//-----------------------------------------------------------------------------

#ifndef __D_NETFIL__
#define __D_NETFIL__

#include "d_net.h"
#include "d_clisrv.h"
#include "w_wad.h"

typedef enum {
    SF_FILE    = 0,
    SF_Z_RAM      ,
    SF_RAM        ,
    SF_NOFREERAM
} freemethode_t;

typedef enum {
    FS_NOTFOUND,
    FS_FOUND,
    FS_REQUESTED,
    FS_DOWNLOADING,
    FS_OPEN, // Is opened and used in w_wad
    FS_MD5SUMBAD
} filestatus_t;

typedef struct {
    char    filename[MAX_WADPATH];
    // used only for download
    FILE    *phandle;
    UINT32   currentsize;
    UINT32   totalsize;
    filestatus_t status;        // the value returned by recsearch
} fileneeded_t;

extern int fileneedednum;
extern fileneeded_t fileneeded[MAX_WADFILES];
extern char downloaddir[512];

char *PutFileNeeded(void);
void Got_FileneededPak(void);
void CL_PrepareDownloadSaveGame(const char *tmpsave);

boolean CL_CheckFiles(void);
void CL_LoadServerFiles(void);
void SendFile(int node,char *filename, char fileid);
void SendRam(int node,UINT8 *data, UINT32 size,freemethode_t freemethode, char fileid);

void FiletxTicker(void);
void Got_Filetxpak(void);

boolean SendRequestFile(void);
void Got_RequestFilePak(int node);

void AbortSendFiles(int node);
void CloseNetFile(void);

void nameonly(char *s);

// Search a file in the wadpath, return FS_FOUND when found
filestatus_t findfile(char *filename, const UINT8 *wantedmd5sum, boolean completepath);
filestatus_t checkfilemd5(char *filename, const UINT8 *wantedmd5sum);

#endif // __D_NETFIL__
