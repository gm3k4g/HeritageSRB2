// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: sounds.h,v 1.2 2000/02/27 00:42:11 hurdler Exp $
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
// $Log: sounds.h,v $
// Revision 1.2  2000/02/27 00:42:11  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      Created by the sound utility written by Dave Taylor.
//      Kept as a sample, DOOM2  sounds. Frozen.
//
//-----------------------------------------------------------------------------

#ifndef __SOUNDS__
#define __SOUNDS__


// 10 customisable sounds for Skins
enum {
	SKSSPIN,
	SKSPUTPUT,
	SKSPUDPUD,
	SKSPLPAIN,
	SKSPLDETH,
	SKSTHOK,
	SKSSPNDSH,
	SKSZOOM,
	SKSRADIO,
	SKSGASP,
	SKSJUMP,
    NUMSKINSOUNDS
} skinsound_t;


// free sfx for S_AddSoundFx()
                         //MAXSKINS
#define NUMSFXFREESLOTS   (32*NUMSKINSOUNDS)

//
// SoundFX struct.
//
typedef struct sfxinfo_struct   sfxinfo_t;

struct sfxinfo_struct
{
    // up to 6-character name
    char*       name;

    // Sfx singularity (only one at a time)
    int         singularity;

    // Sfx priority
    int         priority;

    // referenced sound if a link
    sfxinfo_t*  link;

    // pitch if a link
    int         pitch;

    // volume if a link
    int        volume;

    // sound data
    void*       data;
    int         length;

    // sound that can be remapped for a skin, indexes skins[].skinsounds
    // 0 up to (NUMSKINSOUNDS-1), -1 = not skin specifc
    int         skinsound;

    // this is checked every second to see if sound
    // can be thrown out (if 0, then decrement, if -1,
    // then throw out, if > 0, then it is in use)
    int         usefulness;

    // lump number of sfx
    int         lumpnum;

};




//
// MusicInfo struct.
//
typedef struct
{
    // up to 6-character name
    char*       name;

    // lump number of music
    int         lumpnum;

    // music data
    void*       data;

} musicinfo_t;




// the complete set of sound effects
extern sfxinfo_t        S_sfx[];

// the complete set of music
extern musicinfo_t      S_music[];

//
// Identifiers for all music in game.
//

typedef enum
{
    mus_None,
    mus_e1m1,
    mus_e1m2,
    mus_e1m3,
    mus_e1m4,
    mus_e1m5,
    mus_e1m6,
    mus_e1m7,
    mus_e1m8,
    mus_e1m9,
    mus_e2m1,
    mus_e2m2,
    mus_e2m3,
    mus_e2m4,
    mus_e2m5,
    mus_e2m6,
    mus_e2m7,
    mus_e2m8,
    mus_e2m9,
    mus_e3m1,
    mus_e3m2,
    mus_e3m3,
    mus_e3m4,
    mus_e3m5,
    mus_e3m6,
    mus_e3m7,
    mus_e3m8,
    mus_e3m9,
    mus_inter,
    mus_intro,
    mus_bunny,
    mus_victor,
    mus_introa,
    mus_runnin,
    mus_stalks,
    mus_countd,
    mus_betwee,
    mus_doom,
    mus_the_da,
    mus_shawn,
    mus_ddtblu,
    mus_in_cit,
    mus_dead,
    mus_stlks2,
    mus_theda2,
    mus_doom2,
    mus_ddtbl2,
    mus_runni2,
    mus_dead2,
    mus_stlks3,
    mus_romero,
    mus_shawn2,
    mus_messag,
    mus_count2,
    mus_ddtbl3,
    mus_ampie,
    mus_theda3,
    mus_adrian,
    mus_messg2,
    mus_romer2,
    mus_tense,
    mus_shawn3,
    mus_openin,
    mus_evil,
    mus_ultima,
    mus_dm2ttl, // Moved it up Tails 01-06-2001
    mus_read_m,
    mus_dm2int,
    mus_invinc, // invincibility Tails
    mus_drown, // drowning Tails 03-06-2000
    mus_gmover, // Tails 03-14-2000
    mus_xtlife, // Tails 03-14-2000
    NUMMUSIC
} musicenum_t;


//
// Identifiers for all sfx in game.
//

typedef enum
{
    sfx_None,
    sfx_menu1,
    sfx_shotgn,
    sfx_chchng,
    sfx_dshtgn,
    sfx_dbopn,
    sfx_dbcls,
    sfx_dbload,
    sfx_plasma,
    sfx_bfg,
    sfx_spin,
    sfx_putput,
    sfx_pudpud,
    sfx_wtrdng,
    sfx_rlaunc,
    sfx_rxplod,
    sfx_lvpass,
    sfx_firxpl,
    sfx_pstart,
    sfx_pstop,
    sfx_doropn,
    sfx_dorcls,
    sfx_stnmov,
    sfx_swtchn,
    sfx_swtchx,
    sfx_plpain,
    sfx_dmpain,
    sfx_popain,
    sfx_vipain,
    sfx_mnpain,
    sfx_pepain,
    sfx_pop,
    sfx_itemup,
    sfx_wpnup,
    sfx_spring,
    sfx_telept,
    sfx_posit1,
    sfx_posit2,
    sfx_posit3,
    sfx_bgsit1,
    sfx_bgsit2,
    sfx_sgtsit,
    sfx_cacsit,
    sfx_brssit,
    sfx_cybsit,
    sfx_spisit,
    sfx_bspsit,
    sfx_kntsit,
    sfx_vilsit,
    sfx_mansit,
    sfx_pesit,
    sfx_sklatk,
    sfx_sgtatk,
    sfx_skepch,
    sfx_vilatk,
    sfx_claw,
    sfx_skeswg,
    sfx_pldeth,
    sfx_thok,
    sfx_podth1,
    sfx_podth2,
    sfx_podth3,
    sfx_bgdth1,
    sfx_bgdth2,
    sfx_sgtdth,
    sfx_cacdth,
    sfx_skldth,
    sfx_brsdth,
    sfx_cybdth,
    sfx_spidth,
    sfx_bspdth,
    sfx_vildth,
    sfx_kntdth,
    sfx_pedth,
    sfx_skedth,
    sfx_posact,
    sfx_bgact,
    sfx_dmact,
    sfx_bspact,
    sfx_bspwlk,
    sfx_vilact,
    sfx_spndsh,
    sfx_barexp,
    sfx_zoom,
    sfx_hoof,
    sfx_metal,
    sfx_chgun,
    sfx_tink,
    sfx_bdopn,
    sfx_bdcls,
    sfx_itmbk,
    sfx_flame,
    sfx_flamst,
    sfx_shield,
    sfx_bospit,
    sfx_boscub,
    sfx_bossit,
    sfx_bospn,
    sfx_bosdth,
    sfx_manatk,
    sfx_mandth,
    sfx_sssit,
    sfx_ssdth,
    sfx_keenpn,
    sfx_keendt,
    sfx_skeact,
    sfx_skesit,
    sfx_bkpoof,
    sfx_radio,
    //added:22-02-98: player avatar jumps
    sfx_jump,
    //added:22-02-98: player hits something hard and says 'ouch!'
    sfx_gasp,
    //test water
    sfx_gloop,
    sfx_splash,
    sfx_floush,
    sfx_amwtr1, // Tails
    sfx_amwtr2, // Tails
    sfx_amwtr3, // Tails
    sfx_amwtr4, // Tails
    sfx_amwtr5, // Tails
    sfx_amwtr6, // Tails
    sfx_amwtr7, // Tails
    sfx_amwtr8, // Tails
    sfx_amwtr9, // Tails
	sfx_splish, // Tails 12-08-2000
	sfx_wslap, // Water Slap Tails 12-13-2000
	sfx_steam1, // Tails 06-19-2001
	sfx_steam2, // Tails 06-19-2001
	sfx_cgot, // Got Emerald! Tails 09-02-2001

    // free slots for S_AddSoundFx() at run-time --------------------
    sfx_freeslot0,
    //
    // ... 60 free sounds here ...
    //
    sfx_lastfreeslot=(sfx_freeslot0+NUMSFXFREESLOTS-1),
    // end of freeslots ---------------------------------------------

    NUMSFX
} sfxenum_t;


void   S_InitRuntimeSounds (void);
int    S_AddSoundFx (char *name,int singularity);
void   S_RemoveSoundFx (int id);

#endif
