// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: d_netfil.c,v 1.7 2000/08/10 14:52:38 ydario Exp $
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
// $Log: d_netfil.c,v $
// Revision 1.7  2000/08/10 14:52:38  ydario
// OS/2 port
//
// Revision 1.6  2000/04/16 18:38:07  bpereira
// no message
//
// Revision 1.5  2000/03/07 03:32:24  hurdler
// fix linux compilation
//
// Revision 1.4  2000/03/05 17:10:56  bpereira
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
//      Transfer a file using HSendPacket
//
//-----------------------------------------------------------------------------


#include <stdio.h>
#include <fcntl.h>
#ifdef __OS2__
#include <sys/types.h>
#endif // __OS2__
#include <sys/stat.h>

#include <time.h>

#if !defined( LINUX) && !defined(__OS2__)
#include <io.h>
#include <direct.h>
#else
#include <sys/types.h>
#include <dirent.h>
#include <utime.h>
#endif

#ifndef __WIN32__
#include <unistd.h>
#else
#include <sys/utime.h>
#endif
#ifdef __DJGPP__
#include <dir.h>
#include <utime.h>
#endif

#include "doomdef.h"
#include "doomstat.h"
#include "d_clisrv.h"
#include "d_main.h"
#include "g_game.h"
#include "i_net.h"
#include "i_system.h"
#include "m_argv.h"
#include "d_net.h"
#include "w_wad.h"
#include "d_netfil.h"
#include "filesrch.h"
#include "z_zone.h"
#include "byteptr.h"
#include "p_setup.h"
#include "m_misc.h"

#ifndef NOMD5
#include "md5.h"
#endif

// sender structure
typedef struct filetx_s {
    int      ram;
    char     *filename;   // name of the file or ptr of the data in ram
    UINT32    size;
    char     fileid;
    int      node;        // destination
    struct filetx_s *next; // a queu
} filetx_t;

// current transfers (one for eatch node)
struct {
   filetx_t  *txlist;
   UINT32      position;
   FILE*      currentfile;
} transfer[MAXNETNODES];

// read time of file : stat _stmtime
// write time of file : utime

// receiver structure
int fileneedednum;
fileneeded_t fileneeded[MAX_WADFILES];
char downloaddir[512];

// fill the serverinfo packet with wad file loaded
char *PutFileNeeded(void)
{
    int   i;
    char *p;
    char  wadfilename[MAX_WADPATH];

    p=(char *)&netbuffer->u.serverinfo.fileneeded;
    for(i=0;wadfiles[i];i++)
    {
        WRITEULONG(p,wadfiles[i]->filesize);
        strcpy(wadfilename,wadfiles[i]->filename);
        nameonly(wadfilename);
        WRITESTRING(p,wadfilename);
    }
    netbuffer->u.serverinfo.fileneedednum=i;
    return p;
}

// fill fileneeded table on client when received the serverinfo packet
// used implicite parameter : netbuffer
void Got_FileneededPak(void)
{
    int i;
    UINT8 *p;

    fileneedednum=netbuffer->u.serverinfo.fileneedednum;
    p=netbuffer->u.serverinfo.fileneeded;
    for(i=0;i<fileneedednum;i++)
    {
        fileneeded[i].status = FS_NOTFOUND;
        fileneeded[i].totalsize = READULONG(p);
        fileneeded[i].phandle = NULL;
        READSTRING(p,fileneeded[i].filename);
    }
}

//
void CL_PrepareDownloadSaveGame(const char *tmpsave)
{
    fileneedednum = 1;
    fileneeded[0].status = FS_REQUESTED;
    fileneeded[0].totalsize = -1;
    fileneeded[0].phandle = NULL;
    strcpy(fileneeded[0].filename,tmpsave);
}


// send the name of file who status is FS_NOTFOUND of the fileneeded table
boolean SendRequestFile(void)
{
    char  *p;
    int   i;
    UINT32 totalfreespaceneeded=0;
    INT64 availablefreespace;

    if( M_CheckParm("-nodownload") )
    {
        char s[1024]="";
        for(i=0;i<fileneedednum;i++)
            if( fileneeded[i].status!=FS_FOUND )
            {
                strcat(s,"\n    ");
                strcat(s,fileneeded[i].filename);
				strcat(s," not found");
            }
        I_Error("To play with this server you should have this files : %s\n",s);
    }

    netbuffer->packettype = REQUESTFILE;
    p=(char *)netbuffer->u.textcmd;
    for(i=0;i<fileneedednum;i++)
        if( fileneeded[i].status==FS_NOTFOUND)
        {
            if( fileneeded[i].status==FS_NOTFOUND )
                totalfreespaceneeded += fileneeded[i].totalsize;
            nameonly(fileneeded[i].filename);
            WRITECHAR(p,i);  // fileid
            WRITESTRING(p,fileneeded[i].filename);
            // put it in download dir
            strcatbf(fileneeded[i].filename,downloaddir,"/");
            fileneeded[i].status = FS_REQUESTED;
        }
    WRITECHAR(p,-1);
    I_GetDiskFreeSpace(&availablefreespace);
//    CONS_Printf("free byte %d\n",availablefreespace);
    if(totalfreespaceneeded>availablefreespace)
        I_Error("To play on this server you should download %sKb\n"
                "but you have only %sKb freespace on this drive\n",
                sizeu1((size_t)(totalfreespaceneeded>>10)), sizeu2((size_t)(availablefreespace>>10)));

    // prepare to download
    I_mkdir(downloaddir,0);
    return HSendPacket(servernode,true,0,p-(char *)netbuffer->u.textcmd);
}

// get request filepak and put it to the send queue
void Got_RequestFilePak(int node)
{
    char *p=(char *)netbuffer->u.textcmd;
    while(*p!=-1)
    {
        SendFile(node,p+1,*p);
        p++; // skip fileid
        SKIPSTRING(p);
    }
}


// client check if the filedeened arent already loaded or on the disk
boolean CL_CheckFiles(void)
{
    int i,j;
    char wadfilename[MAX_WADPATH];
    boolean ret=true;

    if(M_CheckParm("-nofiles"))
        return true;

    // the first is the iwad (the main wad file)
    // do not check file date, also don't donwload it (copyright problem)
    strcpy(wadfilename,wadfiles[0]->filename);
    nameonly(wadfilename);
    if( stricmp(wadfilename,fileneeded[0].filename)!=0)
        I_Error("This server use %s (you are using %s)\n",fileneeded[0].filename,wadfilename);
    fileneeded[0].status=FS_OPEN;

    for (i=1;i<fileneedednum;i++)
    {
        if(devparm) CONS_Printf("searching for '%s' ",fileneeded[i].filename);

        // check in allready loaded files
        for(j=1;wadfiles[j];j++)
        {
            strcpy(wadfilename,wadfiles[j]->filename);
            nameonly(wadfilename);
            if( stricmp(wadfilename,fileneeded[i].filename)==0)
            {
                if(devparm) CONS_Printf("already loaded\n");
                fileneeded[i].status=FS_OPEN;
                break;
            }
        }
        if( fileneeded[i].status!=FS_NOTFOUND )
           continue;

        fileneeded[i].status = findfile(fileneeded[i].filename, NULL, true);
        if(devparm) CONS_Printf("found %d\n",fileneeded[i].status);
        if( fileneeded[i].status != FS_FOUND )
            ret=false;
    }
    return ret;
}

// load it now
void CL_LoadServerFiles(void)
{
    int i;

    for (i=1;i<fileneedednum;i++)
    {
        if( fileneeded[i].status == FS_OPEN )
            // allready loaded
            continue;
        else
        if( fileneeded[i].status == FS_FOUND )
        {
            P_AddWadFile(fileneeded[i].filename,NULL);
            fileneeded[i].status = FS_OPEN;
        }
        else
            I_Error("Try to load file %s with status of %d\n",fileneeded[i].filename,fileneeded[i].status);
    }
}

// little optimization to test if there is a file in the queue
int filetosend=0;

void SendFile(int node,char *filename, char fileid)
{
    filetx_t **q,*p;
    int i;
    boolean found=0;
    char  wadfilename[MAX_WADPATH];

    q=&transfer[node].txlist;
    while(*q) q=&((*q)->next);
    *q=(filetx_t *)malloc(sizeof(filetx_t));
    if(!*q)
       I_Error("SendFile : No more ram\n");
    p=*q;
    p->filename=(char *)malloc(MAX_WADPATH);
    strcpy(p->filename,filename);

    // a minimum of security, can't get only file in legacy direcory
    nameonly(p->filename);

    // check first in wads loaded the magority of case
    for(i=0;wadfiles[i] && !found;i++)
    {
        strcpy(wadfilename,wadfiles[i]->filename);
        nameonly(wadfilename);
        if(stricmp(wadfilename,p->filename)==0)
        {
            found = true;
            // copy filename with full path
            strcpy(p->filename,wadfiles[i]->filename);
        }
    }

    if( !found )
    {
        DEBFILE(va("%s not found in wadfiles\n",filename));
        if(findfile(p->filename,NULL,true)==0)
		{
			// not found
			// don't inform client (probably hacker)
			DEBFILE(va("Client %d request %s : not found\n",node,filename));
			free(p->filename);
			free(p);
			*q=NULL;
			return;
		}
		else
            return;
    }

	DEBFILE(va("Sending file %s to %d (id=%d)\n",filename,node,fileid));

    p->ram=SF_FILE;
    // size initialized at file open
    //p->size=size;
    p->fileid=fileid;
    p->next=NULL; // end of list

    filetosend++;
}

void SendRam(int node,UINT8 *data, UINT32 size,freemethode_t freemethode, char fileid)
{
    filetx_t **q,*p;

    q=&transfer[node].txlist;
    while(*q) q=&((*q)->next);
    *q=(filetx_t *)malloc(sizeof(filetx_t));
    if(!*q)
       I_Error("SendRam : No more ram\n");
    p=*q;
    p->ram=freemethode;
    p->filename=(char *)data;
    p->size=size;
    p->fileid=fileid;
    p->next=NULL; // end of list

    DEBFILE(va("Sending ram %x( size:%d) to %d (id=%d)\n",p->filename,size,node,fileid));

    filetosend++;
}

void EndSend(int node)
{
    filetx_t *p=transfer[node].txlist;
    switch (p->ram) {
    case SF_FILE:
        if( transfer[node].currentfile )
            fclose(transfer[node].currentfile);
        free(p->filename);
        break;
    case SF_Z_RAM:
        Z_Free(p->filename);
        break;
    case SF_RAM:
        free(p->filename);
    case SF_NOFREERAM:
        break;
    }
    transfer[node].txlist = p->next;
    transfer[node].currentfile = NULL;
    free(p);
    filetosend--;
}

#define PACKETPERTIC net_bandwidth/(TICRATE*software_MAXPACKETLENGTH)

void FiletxTicker(void)
{
    static int currentnode=0;
    filetx_pak *p;
    UINT32      size;
    filetx_t   *f;
    int        ram,i;
    int        packetsent = PACKETPERTIC;

    if( filetosend==0 )
        return;
    if(packetsent==0) packetsent++;
    // (((sendbytes-nowsentbyte)*TICRATE)/(I_GetTime()-starttime)<(UINT32)net_bandwidth)
    while( packetsent-- && filetosend!=0)
    {
    for(i=currentnode,ram=0;ram<MAXNETNODES;i=(i+1)%MAXNETNODES,ram++)
        if(transfer[i].txlist)
            goto found;
    // no transfer to do
    I_Error("filetosend=%d but no filetosend found\n",filetosend);
found:
    currentnode=(i+1)%MAXNETNODES;
    f=transfer[i].txlist;
    ram=f->ram;

    if(!transfer[i].currentfile) // file not allready open
    {
        if(!ram) {
#ifdef __OS2__
            fpos_t size64;
#else
            fpos_t size64=0;
#endif
        transfer[i].currentfile=fopen(f->filename,"rb");
        if(!transfer[i].currentfile)
            I_Error("File %s not exist",f->filename);
        fseek(transfer[i].currentfile,0,SEEK_END);
        size=fgetpos(transfer[i].currentfile,&size64);
        // WARNING fpos_t 64bit in some OS
#ifndef __OS2__
        if(size64>INT32_MAX)
            I_Error("Error : %s is too large\n",f->filename);
        f->size=size64;
#else
        f->size=size64._pos;
#endif
        if( size!=0 )
            I_Error("Error %d geting filesize of %s\n",size,f->filename);
        CONS_Printf("Sending %s (%d byte)\n",f->filename,f->size);
        fseek(transfer[i].currentfile,0,SEEK_SET);
        }
        else
            transfer[i].currentfile = (FILE *)1;
        transfer[i].position=0;
    }

    p=&netbuffer->u.filetxpak;
    size=software_MAXPACKETLENGTH-(FILETXHEADER+BASEPACKETSIZE);
    if( f->size-transfer[i].position<size )
        size=f->size-transfer[i].position;
    if(ram)
        memcpy(p->data,&f->filename[transfer[i].position],size);
    else
    {
        if( fread(p->data,size,1,transfer[i].currentfile) != 1 )
            I_Error("FiletxTicker : can't get %d byte on %s at %d",size,f->filename,transfer[i].position);
    }
    p->position = transfer[i].position;
    // put flag so receiver know the totalsize
    if( transfer[i].position+size==f->size )
        p->position |= 0x80000000;
    p->fileid   = f->fileid;
    p->size=size;
    netbuffer->packettype=FILEFRAGMENT;
    if (!HSendPacket(i,true,0,FILETXHEADER+size ) ) // reliable SEND
    { // not sent for some od reason
      // retry at next call
         if( !ram )
             fseek(transfer[i].currentfile,transfer[i].position,SEEK_SET);
         // exit the while (can't sent this one why should i sent the next ?
         break;
    }
    else // success
    {
        transfer[i].position+=size;
        if(transfer[i].position==f->size) //  finish ?
            EndSend(i);
    }
    }
}

void Got_Filetxpak(void)
{
    int filenum=netbuffer->u.filetxpak.fileid;
static int time=0;

    if(filenum>=fileneedednum)
    {
        DEBFILE(va("fileframent not needed %d>%d\n",filenum,fileneedednum));
        return;
    }

    if( fileneeded[filenum].status==FS_REQUESTED )
    {
        if(fileneeded[filenum].phandle) I_Error("Got_Filetxpak : allready open file\n");
        fileneeded[filenum].phandle=fopen(fileneeded[filenum].filename,"wb");
        if(!fileneeded[filenum].phandle) I_Error("Can't create file %s : disk full ?",fileneeded[filenum].filename);
        CONS_Printf("\r%s...",fileneeded[filenum].filename);
        fileneeded[filenum].currentsize = 0;
        fileneeded[filenum].status=FS_DOWNLOADING;
    }

    if( fileneeded[filenum].status==FS_DOWNLOADING )
    {
        // use a special tric to know when file is finished (not allways used)
        // WARNING: filepak can arrive out of order so don't stop now !
        if( netbuffer->u.filetxpak.position & 0x80000000 )
        {
            netbuffer->u.filetxpak.position &= ~0x80000000;
            fileneeded[filenum].totalsize = netbuffer->u.filetxpak.position + netbuffer->u.filetxpak.size;
        }
        // we can receive packet in the wrong order, anyway all os support gaped file
        fseek(fileneeded[filenum].phandle,netbuffer->u.filetxpak.position,SEEK_SET);
        if( fwrite(netbuffer->u.filetxpak.data,netbuffer->u.filetxpak.size,1,fileneeded[filenum].phandle)!=1 )
            I_Error("Can't write %s : disk full ?\n",fileneeded[filenum].filename);
        fileneeded[filenum].currentsize+=netbuffer->u.filetxpak.size;
        if(time==0)
        {
            Net_GetNetStat();
            CONS_Printf("\r%s %dK/%dK %.1fK/s",fileneeded[filenum].filename,
                                               fileneeded[filenum].currentsize>>10,
                                               fileneeded[filenum].totalsize>>10,
                                               ((float)getbps)/1024);
        }

        // finish ?
        if(fileneeded[filenum].currentsize==fileneeded[filenum].totalsize)
        {
            fclose(fileneeded[filenum].phandle);
            fileneeded[filenum].phandle=NULL;
            fileneeded[filenum].status=FS_FOUND;
            CONS_Printf("\rDownloading %s...(done)",fileneeded[filenum].filename);
        }
    }
    else
        I_Error("Received a file not requested\n");
    // send ack back quickly

    if(++time==4)
    {
        SendAcks(servernode);
        time=0;
    }

}

void AbortSendFiles(int node)
{
    while(transfer[node].txlist)
        EndSend(node);
}

void CloseNetFile(void)
{
    int i;
    // is sending ?
    for( i=0;i<MAXNETNODES;i++)
        AbortSendFiles(i);

    // receiving a file ?
    for( i=0;i<MAX_WADFILES;i++ )
        if( fileneeded[i].status==FS_DOWNLOADING && fileneeded[i].phandle)
        {
            fclose(fileneeded[i].phandle);
            // file is not compete delete it
            remove(fileneeded[i].filename);
        }

    // remove FILEFRAGMENT from acknledge list
    Net_Abort(FILEFRAGMENT);
}

// functions cut and pasted from doomatic :)

void nameonly(char *s)
{
  int j;

  for(j=strlen(s);j>=0;j--)
      if( (s[j]=='\\') || (s[j]==':') || (s[j]=='/') )
      {
          memcpy(s, &(s[j+1]), strlen(&(s[j+1]))+1 );
          return;
      }
}

#ifndef O_BINARY
#define O_BINARY 0
#endif

filestatus_t checkfilemd5(char *filename, const UINT8 *wantedmd5sum)
{
#if defined (NOMD5)
	(void)wantedmd5sum;
	(void)filename;
#else
	FILE *fhandle;
	UINT8 md5sum[16];

	if (!wantedmd5sum)
		return FS_FOUND;

	fhandle = fopen(filename, "rb");
	if (fhandle)
	{
		md5_stream(fhandle,md5sum);
		fclose(fhandle);
		if (!memcmp(wantedmd5sum, md5sum, 16))
			return FS_FOUND;
		return FS_MD5SUMBAD;
	}

	I_Error("Couldn't open %s for md5 check", filename);
#endif
	return FS_FOUND; // will never happen, but makes the compiler shut up
}

// Rewritten by Monster Iestyn to be less stupid
// Note: if completepath is true, "filename" is modified, but only if FS_FOUND is going to be returned
// (Don't worry about WinCE's version of filesearch, nobody cares about that OS anymore)
filestatus_t findfile(char *filename, const UINT8 *wantedmd5sum, boolean completepath)
{
	filestatus_t homecheck; // store result of last file search
	boolean badmd5 = false; // store whether md5 was bad from either of the first two searches (if nothing was found in the third)

	// first, check SRB2's "home" directory
	homecheck = filesearch(filename, srb2home, wantedmd5sum, completepath, 10);

	if (homecheck == FS_FOUND) // we found the file, so return that we have :)
		return FS_FOUND;
	else if (homecheck == FS_MD5SUMBAD) // file has a bad md5; move on and look for a file with the right md5
		badmd5 = true;
	// if not found at all, just move on without doing anything

	// next, check SRB2's "path" directory
	homecheck = filesearch(filename, srb2path, wantedmd5sum, completepath, 10);

	if (homecheck == FS_FOUND) // we found the file, so return that we have :)
		return FS_FOUND;
	else if (homecheck == FS_MD5SUMBAD) // file has a bad md5; move on and look for a file with the right md5
		badmd5 = true;
	// if not found at all, just move on without doing anything

	// finally check "." directory
	homecheck = filesearch(filename, ".", wantedmd5sum, completepath, 10);

	if (homecheck != FS_NOTFOUND) // if not found this time, fall back on the below return statement
		return homecheck; // otherwise return the result we got

	return (badmd5 ? FS_MD5SUMBAD : FS_NOTFOUND); // md5 sum bad or file not found
}
