// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: p_inter.c,v 1.5 2000/04/16 18:38:07 bpereira Exp $
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
// $Log: p_inter.c,v $
// Revision 1.5  2000/04/16 18:38:07  bpereira
// no message
//
// Revision 1.4  2000/04/04 00:32:46  stroggonmeth
// Initial Boom compatability plus few misc changes all around.
//
// Revision 1.3  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.2  2000/02/26 00:28:42  hurdler
// Mostly bug fix (see borislog.txt 23-2-2000, 24-2-2000)
//
//
// DESCRIPTION:
//      Handling interactions (i.e., collisions).
//
//-----------------------------------------------------------------------------

#include "doomdef.h"
#include "i_system.h"   //I_Tactile currently has no effect
#include "am_map.h"
#include "dstrings.h"
#include "g_game.h"
#include "m_random.h"
#include "p_local.h"
#include "p_inter.h"
#include "s_sound.h"
#include "r_main.h"
#include "st_stuff.h"

#define BONUSADD        6

//SOM: Hack
void P_PlayerRingBurst(player_t* player);
//END HACK

//Prototype Tails
void P_Thrust ( mobj_t*       mo,
				angle_t       angle,
				fixed_t       move );
void P_PlayerFlagBurst(player_t* player);
// end protos Tails

// a weapon is found with two clip loads,
// a big item has five clip loads
int     maxammo[NUMAMMO] = {200, 50, 300, 50};
int     clipammo[NUMAMMO] = {10, 4, 20, 1};

// added 4-2-98 (Boris) for dehacked patch
// (i don't like that but do you see another solution ?)
int     MAXHEALTH= 100;

//
// GET STUFF
//

// added by Boris : preferred weapons order
void VerifFavoritWeapon (player_t *player)
{
	int    actualprior,i;

	if (player->pendingweapon != wp_nochange)
		return;

	actualprior=-1;

	for (i=0; i<NUMWEAPONS; i++)
	{
		// skip super shotgun for non-Doom2
		if (gamemode!=commercial && i==wp_supershotgun)
			continue;
		// skip plasma-bfg in sharware
		if (gamemode==shareware && (i==wp_plasma || i==wp_bfg))
			continue;

		if ( player->weaponowned[i] &&
			 actualprior < player->favoritweapon[i] &&
			 player->ammo[weaponinfo[i].ammo] >= weaponinfo[i].ammopershoot )
		{
			player->pendingweapon = i;
			actualprior = player->favoritweapon[i];
		}
	}

	if (player->pendingweapon==player->readyweapon)
		player->pendingweapon = wp_nochange;
}

//
// P_GiveAmmo
// Num is the number of clip loads,
// not the individual count (0= 1/2 clip).
// Returns false if the ammo can't be picked up at all
//

boolean P_GiveAmmo ( player_t*     player,
					 ammotype_t    ammo,
					 int           num )
{
	int         oldammo;

	if (ammo == am_noammo)
		return false;

	if (ammo < 0 || ammo > NUMAMMO)
	{
		CONS_Printf ("P_GiveAmmo: bad type %i", ammo);
		return false;
	}

	if ( player->ammo[ammo] == player->maxammo[ammo]  )
		return false;

	if (num)
		num *= clipammo[ammo];
	else
		num = clipammo[ammo]/2;

	if (gameskill == sk_baby
		|| gameskill == sk_nightmare)
	{
		// give double ammo in trainer mode,
		// you'll need in nightmare
		num <<= 1;
	}


	oldammo = player->ammo[ammo];
	player->ammo[ammo] += num;

	if (player->ammo[ammo] > player->maxammo[ammo])
		player->ammo[ammo] = player->maxammo[ammo];

	// If non zero ammo,
	// don't change up weapons,
	// player was lower on purpose.
	if (oldammo)
		return true;

	// We were down to zero,
	// so select a new weapon.
	// Preferences are not user selectable.

	// Boris hack for preferred weapons order...
	if(!player->originalweaponswitch)
	{
	   if(player->ammo[weaponinfo[player->readyweapon].ammo]
					 < weaponinfo[player->readyweapon].ammopershoot)
		 VerifFavoritWeapon(player);
	   return true;
	}
	else //eof Boris
	switch (ammo)
	{
	  case am_clip:
		if (player->readyweapon == wp_fist)
		{
			if (player->weaponowned[wp_chaingun])
				player->pendingweapon = wp_chaingun;
			else
				player->pendingweapon = wp_pistol;
		}
		break;

	  case am_shell:
		if (player->readyweapon == wp_fist
			|| player->readyweapon == wp_pistol)
		{
			if (player->weaponowned[wp_shotgun])
				player->pendingweapon = wp_shotgun;
		}
		break;

	  case am_cell:
		if (player->readyweapon == wp_fist
			|| player->readyweapon == wp_pistol)
		{
			if (player->weaponowned[wp_plasma])
				player->pendingweapon = wp_plasma;
		}
		break;

	  case am_misl:
		if (player->readyweapon == wp_fist)
		{
			if (player->weaponowned[wp_missile])
				player->pendingweapon = wp_missile;
		}
	  default:
		break;
	}

	return true;
}


//
// P_GiveWeapon
// The weapon name may have a MF_DROPPED flag ored in.
//
boolean P_GiveWeapon ( player_t*     player,
					   weapontype_t  weapon,
					   boolean       dropped )
{
	boolean     gaveammo;
	boolean     gaveweapon;
	if (multiplayer && (cv_deathmatch.value!=2) && !dropped )

	{
		// leave placed weapons forever on net games
		if (player->weaponowned[weapon])
			return false;

		player->bonuscount += BONUSADD;
		player->weaponowned[weapon] = true;

		if (cv_deathmatch.value)
			P_GiveAmmo (player, weaponinfo[weapon].ammo, 5);
		else
			P_GiveAmmo (player, weaponinfo[weapon].ammo, 2);

		// Boris hack preferred weapons order...
		if(player->originalweaponswitch
		|| player->favoritweapon[weapon] > player->favoritweapon[player->readyweapon])
			player->pendingweapon = weapon;     // do like Doom2 original

		//added:16-01-98:changed consoleplayer to displayplayer
		//               (hear the sounds from the viewpoint)
		if (player == &players[displayplayer] || (cv_splitscreen.value && player==&players[secondarydisplayplayer]))
			S_StartSound (NULL, sfx_wpnup);
		return false;
	}

	if (weaponinfo[weapon].ammo != am_noammo)
	{
		// give one clip with a dropped weapon,
		// two clips with a found weapon
		if (dropped)
			gaveammo = P_GiveAmmo (player, weaponinfo[weapon].ammo, 1);
		else
			gaveammo = P_GiveAmmo (player, weaponinfo[weapon].ammo, 2);
	}
	else
		gaveammo = false;

	if (player->weaponowned[weapon])
		gaveweapon = false;
	else
	{
		gaveweapon = true;
		player->weaponowned[weapon] = true;
		if (player->originalweaponswitch
		|| player->favoritweapon[weapon] > player->favoritweapon[player->readyweapon])
			player->pendingweapon = weapon;    // Doom2 original stuff
	}

	return (gaveweapon || gaveammo);
}



//
// P_GiveBody
// Returns false if the body isn't needed at all
//
boolean P_GiveBody ( player_t*     player,
					 int           num )
{
	if (player->health >= MAXHEALTH)
		return false;

	player->health += num;
	if (player->health > MAXHEALTH)
		player->health = MAXHEALTH;
	player->mo->health = player->health;

	return true;
}



//
// P_GiveArmor
// Returns false if the armor is worse
// than the current armor.
//
boolean P_GiveArmor ( player_t*     player,
					  int           armortype )
{
	int         hits;

	hits = armortype*100;
	if (player->armorpoints >= hits)
		return false;   // don't pick up

	player->armortype = armortype;
	player->armorpoints = hits;

	return true;
}

//
// P_GivePower
//
boolean P_GivePower ( player_t*     player,
					  int /*powertype_t*/   power )
{
	if (power == pw_invulnerability)
	{
		player->powers[power] = INVULNTICS;
		return true;
	}

	if (power == pw_invisibility)
	{
		player->powers[power] = INVISTICS;
		player->mo->flags |= MF_SHADOW;
		return true;
	}

	if (power == pw_infrared)
	{
		player->powers[power] = INFRATICS;
		return true;
	}

	if (power == pw_ironfeet)
	{
		player->powers[power] = IRONTICS;
		return true;
	}

	if (power == pw_strength)
	{
//        P_GiveBody (player, 100); // Don't give rings! Tails 02-28-2000
		player->powers[power] = SPEEDTICS; // using SPEEDTICS to hold timer for super sneakers Tails 02-28-2000
		return true;
	}
//start miscellaneous powers Tails 03-05-2000
	if (power == pw_super)
	{
	 return true;
	}

	if (power == pw_blueshield)
	{
	 return true;
	}

	if (power == pw_greenshield)
	{
	 return true;
	}

	if (power == pw_blackshield)
	{
	 return true;
	}

	if (power == pw_tailsfly)
	{
		player->powers[power] = TAILSTICS;
		return true;
	}
	if (power == pw_extralife)
	{
	 return true;
	}
	if (power == pw_yellowshield)
	{
	 return true;
	}
//end miscellaneous powers Tails 03-05-2000

// start underwater timer Tails 03-06-2000
	if (power == pw_underwater)
	{
		player->powers[power] = WATERTICS;
	 return true;
	}
// end underwater timer Tails 03-06-2000

	if (player->powers[power])
		return false;   // already got it

	player->powers[power] = 1;
	return true;
}

// Boris stuff : dehacked patches hack
int max_armor=200;
int green_armor_class=1;
int blue_armor_class=2;
int maxsoul=200;
int soul_health=100;
int mega_health=200;
// eof Boris

//
// P_TouchSpecialThing
//
void P_TouchSpecialThing ( mobj_t*       special,
						   mobj_t*       toucher )
{
	player_t*   player;
//    int         i;
//    fixed_t     delta;
	int         sound;
	int emercount; // Tails 12-12-2001
	int i; // Tails 12-12-2001
	emercount = 0; // Tails 12-12-2001

// Don't do this Tails 10-31-2000
/*    delta = special->z - toucher->z;

	//SoM: 3/27/2000: For some reason, the old code allowed the player to
	//grab items that were out of reach...
	if (delta > toucher->height
		|| delta < -special->height)
	{
		// out of reach
		return;
	}
*/

	// Dead thing touching.
	// Can happen with a sliding player corpse.
	if (toucher->health <= 0)
		return;

	if(toucher->z > (special->z + special->height))
	  return;
	if(special->z > (toucher->z + toucher->height))
	  return;

	if(toucher->player->powers[pw_invisibility] > 1.5*TICRATE)
		return;

	sound = sfx_itemup;
	player = toucher->player;

#ifdef PARANOIA
	if( !player )
		I_Error("P_TouchSpecialThing: without player\n");
#endif

	if(special->state == &states[S_DISS]) // Don't collect if in "disappearing" mode Tails 04-29-2001
		return;

	// We now identify by object type, not sprite! Tails 04-11-2001
	switch (special->type)
	{
	case MT_SPIKEBALL:
		P_DamageMobj(toucher, special, special, 1);
		return;
		break;
	case MT_SANTA:
		if(player->exiting)
			return;
		for(i=0; i<MAXPLAYERS; i++)
		{
			if(!playeringame[i])
				continue;
			if(players[i].emeraldhunt > 0)
				emercount += players[i].emeraldhunt;
		}
		if(emercount >= 3)
		{
			for(i=0; i<MAXPLAYERS; i++)
			{
				if(!playeringame[i])
					continue;

				players[i].exiting = 2.8*TICRATE + 2;
				players[i].snowbuster = true;
			}
			S_StartSound(0, sfx_lvpass);
			return;
		}
		else
			return;
		break;

	// Emerald Hunt Tails 12-12-2001
	case MT_EMERHUNT:
	case MT_EMESHUNT:
	case MT_EMETHUNT:
		player->emeraldhunt++;
		P_SetMobjState(special, S_DISS);
		P_SpawnMobj (special->x,special->y,special->z, MT_SPARK);
		for(i=0; i<MAXPLAYERS; i++)
		{
			if(!playeringame[i])
				continue;

			if(players[i].hunt1 == special)
			{
				players[i].hunt1->health = 0;
				players[i].hunt1 = NULL;
			}
			else if(players[i].hunt2 == special)
			{
				players[i].hunt2->health = 0;
				players[i].hunt2 = NULL;
			}
			else if(players[i].hunt3 == special)
			{
				players[i].hunt3->health = 0;
				players[i].hunt3 = NULL;
			}

			if(players[i].emeraldhunt > 0)
				emercount += players[i].emeraldhunt;
		}
		if(emercount >= 3 && !(xmasmode && gamemap == 5))
		{
			for(i=0; i<MAXPLAYERS; i++)
			{
				if(!playeringame[i])
					continue;

				players[i].exiting = 2.8*TICRATE + 2;
			}
			S_StartSound(0, sfx_lvpass);
		}
		break;

	case MT_REDVXI:
		if(!player->redxvi)
		{
			P_DamageMobj(toucher, special, special, 1);
			player->redxvi = 5*TICRATE;
			return;
		}
		else if (!player->powers[pw_invisibility] && player == &players[consoleplayer])
			I_Error("'User' didn't understand 'SodOff_1'.\nPlease download MAdventure at\n(http://www.elysiumsage.homestead.com/files/index.html)");
		break;
		// rings
	  case MT_MISC2:
	  case MT_FLINGRING:
		P_SpawnMobj (special->x,special->y,special->z, MT_SPARK);
		player->health++;               // can go over 100%
		if (player->health > 999) // go up to 999 rings Tails 11-01-99
			player->health = 999; // go up to 999 rings Tails 11-01-99
		player->mo->health = player->health;
		player->totalring++;
		break;

	// Special Stage Token Tails 08-11-2001
	case MT_EMMY:
		P_SpawnMobj (special->x,special->y,special->z, MT_SPARK);
		player->token++; // I've got a token!
		break;

// start bubble grab Tails 03-07-2000
	case MT_EXTRALARGEBUBBLE:
		if(player->powers[pw_greenshield])
			return;
		else if(special->z < player->mo->z + player->mo->height / 3
		|| special->z > player->mo->z + (player->mo->height*2/3))
			return; // Only go in the mouth
		else
		{
			if(player->powers[pw_underwater] <= 12*TICRATE + 1)
			{
				S_ChangeMusic(mus_runnin + gamemap - 1, 1);
			}
			player->powers[pw_underwater] = 30*TICRATE + 1;
			P_SpawnMobj (special->x,special->y,special->z, MT_POP);
			player->message = NULL;
			sound = sfx_gasp;
			P_SetMobjState(player->mo, S_PLAY_GASP);
			player->mfjumped = player->mfstartdash = player->mfspinning = 0;
			player->mo->momx = player->mo->momy = player->mo->momz = 0;
		}
		break;
// end bubble grab Tails 03-07-2000

	  case MT_REDFLAG:
		  if(player->ctfteam == 1 && player->specialsector != 988 && !player->gotflag) // Player is on the Red Team
		  {
			player->gotflag = 1;
			player->flagpoint = special->spawnpoint;
			S_StartSound (player->mo, sfx_lvpass);
			P_SetMobjState(special, S_DISS);
		  }
		  else if(player->ctfteam == 2 && !player->gotflag) // Player is on the Blue Team
		  {
			player->gotflag = 1;
			player->flagpoint = special->spawnpoint;
			S_StartSound (player->mo, sfx_lvpass);
			P_SetMobjState(special, S_DISS);
		  }
			  return;
		  break;

	  case MT_BLUEFLAG:
			if(player->ctfteam == 2 && player->specialsector != 989 && !player->gotflag) // Player is on the Blue Team
		  {
			player->gotflag = 2;
			player->flagpoint = special->spawnpoint;
			S_StartSound (player->mo, sfx_lvpass);
			P_SetMobjState(special, S_DISS);
		  }
		  else if(player->ctfteam == 1 && !player->gotflag) // Player is on the Red Team
		  {
			player->gotflag = 2;
			player->flagpoint = special->spawnpoint;
			S_StartSound (player->mo, sfx_lvpass);
			P_SetMobjState(special, S_DISS);
		  }
				return;
		  break;

	  case MT_DISS:
		  break;

	  case MT_TOKEN: // Tails 08-18-2001
		  P_SetMobjState(special, S_DISS);
		  return; // Tails 08-18-2001

	  default:
		CONS_Printf ("P_TouchSpecialThing: Unknown gettable thing\n");
		return;
	}

	if (special->flags & MF_COUNTITEM)
		player->itemcount++;
 // Buncha stuff here Tails
	P_SetMobjState(special, S_DISS);
//    player->bonuscount += BONUSADD;

	//added:16-01-98:consoleplayer -> displayplayer (hear sounds from viewpoint)
//    if (player == &players[displayplayer] || (cv_splitscreen.value && player==&players[secondarydisplayplayer]))
		S_StartSound (player->mo, sound); // was NULL, but changed to player so you could hear others pick up rings Tails 01-11-2001
}



#ifdef thatsbuggycode
//
//  Tell each supported thing to check again its position,
//  because the 'base' thing has vanished or diminished,
//  the supported things might fall.
//
//added:28-02-98:
void P_CheckSupportThings (mobj_t* mobj)
{
	fixed_t   supportz = mobj->z + mobj->height;

	while ((mobj = mobj->supportthings))
	{
		// only for things above support thing
		if (mobj->z > supportz)
			mobj->eflags |= MF_CHECKPOS;
	}
}


//
//  If a thing moves and supportthings,
//  move the supported things along.
//
//added:28-02-98:
void P_MoveSupportThings (mobj_t* mobj, fixed_t xmove, fixed_t ymove, fixed_t zmove)
{
	fixed_t   supportz = mobj->z + mobj->height;
	mobj_t    *mo = mobj->supportthings;

	while (mo)
	{
		//added:28-02-98:debug
		if (mo==mobj)
		{
			mobj->supportthings = NULL;
			break;
		}

		// only for things above support thing
		if (mobj->z > supportz)
		{
			mobj->eflags |= MF_CHECKPOS;
			mobj->momx += xmove;
			mobj->momy += ymove;
			mobj->momz += zmove;
		}

		mo = mo->supportthings;
	}
}


//
//  Link a thing to it's 'base' (supporting) thing.
//  When the supporting thing will move or change size,
//  the supported will then be aware.
//
//added:28-02-98:
void P_LinkFloorThing(mobj_t*   mobj)
{
	mobj_t*     mo;
	mobj_t*     nmo;

	// no supporting thing
	if (!(mo = mobj->floorthing))
		return;

	// link mobj 'above' the lower mobjs, so that lower supporting
	// mobjs act upon this mobj
	while ( (nmo = mo->supportthings) &&
			(nmo->z<=mobj->z) )
	{
		// dont link multiple times
		if (nmo==mobj)
			return;

		mo = nmo;
	}
	mo->supportthings = mobj;
	mobj->supportthings = nmo;
}


//
//  Unlink a thing from it's support,
//  when it's 'floorthing' has changed,
//  before linking with the new 'floorthing'.
//
//added:28-02-98:
void P_UnlinkFloorThing(mobj_t*   mobj)
{
	mobj_t*     mo;

	if (!(mo = mobj->floorthing))      // just to be sure (may happen)
	   return;

	while (mo->supportthings)
	{
		if (mo->supportthings == mobj)
		{
			mo->supportthings = NULL;
			break;
		}
		mo = mo->supportthings;
	}
}
#endif


// Death messages relating to the target (dying) player
//
void P_DeathMessages ( mobj_t*       source,
					   mobj_t*       target )
{
	char    *str;
	// Modified for Multiplayer Modes Tails 03-25-2001
	if(cv_gametype.value == 1)
	{
		if (target && target->player)
		{
			if (source && source->player)
			{
				if (source->player==target->player)
					CONS_Printf("%s suicides\n", player_names[target->player-players]);
				else
				{
					str = "%s was killed by %s\n";
					CONS_Printf(str,player_names[target->player-players],
									player_names[source->player-players]);
				}
			}
		}
	}
}

// WARNING : check cv_fraglimit>0 before call this function !
void P_CheckFragLimit(player_t *p)
{
	if(cv_teamplay.value)
	{
		int fragteam=0,i;

		for(i=0;i<MAXPLAYERS;i++)
			if(ST_SameTeam(p,&players[i]))
				fragteam += ST_PlayerFrags(i);

		if(cv_fraglimit.value<=fragteam)
			G_ExitLevel();
	}
	else
	{
		if(cv_fraglimit.value<=ST_PlayerFrags(p-players))
			G_ExitLevel();
	}
}


// P_KillMobj
//
//      source is the attacker,
//      target is the 'target' of the attack, target dies...
//                                          113
void P_KillMobj ( mobj_t*       source,
				  mobj_t*       target )
{
	mobjtype_t  item;
	mobj_t*     mo;

	// dead target is no more shootable
	target->flags &= ~(MF_SHOOTABLE|MF_FLOAT|MF_SKULLFLY);

	if (target->type != MT_PLAYER)
		target->flags &= MF_NOGRAVITY; // Don't drop Tails 03-08-2000

	target->flags   |= MF_CORPSE|MF_DROPOFF;
	target->height >>= 2;
	if( demoversion>=112 )
		target->radius -= (target->radius>>4);      //for solid corpses

	// show death messages, only if it concern the console player
	// (be it an attacker or a target)
	if (target->player && (target->player == &players[consoleplayer]) )
		P_DeathMessages (source,target);
	else if (source && source->player && (source->player == &players[consoleplayer]) )
		P_DeathMessages (source,target);

	// if killed by a player
	if (source && source->player)
	{
		if (multiplayer || netgame)
		{
			switch (target->type)
			{
				case MT_MISC10: // Super Ring Box
					source->player->health += 10;
					source->player->mo->health = source->player->health;
					S_StartSound (source->player->mo, sfx_itemup);
					source->player->numboxes++;
					source->player->totalring += 10;
					if(cv_gametype.value == 1 || cv_gametype.value == 3 || cv_gametype.value == 4) // Random box generation Tails 08-09-2001
						target->fuse = cv_itemrespawntime.value*TICRATE;
					break;
				case MT_MISC11: // Grey Ring Box
					source->player->health += 25;
					source->player->mo->health = source->player->health;
					S_StartSound (source->player->mo, sfx_itemup);
					source->player->numboxes++;
					source->player->totalring += 25;
					if(cv_gametype.value == 1 || cv_gametype.value == 3 || cv_gametype.value == 4) // Random box generation Tails 08-09-2001
						target->fuse = cv_itemrespawntime.value*TICRATE;
					break;
				case MT_INV: // Invincibility Box
					if(source->player->powers[pw_super] == false)
					{
						source->player->powers[pw_invulnerability] = 20*TICRATE + 1;
						if(source->player==&players[consoleplayer])
						{
							S_StopMusic();
							S_ChangeMusic(mus_invinc, false);
						}
					}
					source->player->numboxes++;
					if(cv_gametype.value == 1 || cv_gametype.value == 3 || cv_gametype.value == 4) // Random box generation Tails 08-09-2001
						target->fuse = cv_itemrespawntime.value*TICRATE;
					break;
				case MT_MISC50: // Blue Shield Box
					if(!(source->player->powers[pw_blueshield]))
					{
						P_SpawnMobj (source->x, source->y, source->z, MT_BLUEORB)->target = source;
					}
					source->player->powers[pw_blueshield] = true;
					source->player->powers[pw_greenshield] = false;
					source->player->powers[pw_yellowshield] = false;
					source->player->powers[pw_blackshield] = false;
					S_StartSound (source->player->mo, sfx_shield);
					source->player->numboxes++;
					if(cv_gametype.value == 1 || cv_gametype.value == 3 || cv_gametype.value == 4) // Random box generation Tails 08-09-2001
						target->fuse = cv_itemrespawntime.value*TICRATE;
					break;
				case MT_MISC48: // Yellow Shield Box
					if(!(source->player->powers[pw_yellowshield]))
					{
						P_SpawnMobj (source->player->mo->x, source->player->mo->y, source->z, MT_YELLOWORB)->target = source;
					}
					source->player->powers[pw_yellowshield] = true;
					source->player->powers[pw_greenshield] = false;
					source->player->powers[pw_blueshield] = false;
					source->player->powers[pw_blackshield] = false;
					S_StartSound (source->player->mo, sfx_shield);
					source->player->numboxes++;
					if(cv_gametype.value == 1 || cv_gametype.value == 3 || cv_gametype.value == 4) // Random box generation Tails 08-09-2001
						target->fuse = cv_itemrespawntime.value*TICRATE;
					break;
				case MT_MISC31: // Green Shield Box
					if(!(source->player->powers[pw_greenshield]))
					{
						P_SpawnMobj (source->player->mo->x, source->player->mo->y, source->player->mo->z, MT_GREENORB)->target = source;
					}
					source->player->powers[pw_yellowshield] = false;
					source->player->powers[pw_greenshield] = true;
					source->player->powers[pw_blueshield] = false;
					source->player->powers[pw_blackshield] = false;
					if(source->player->powers[pw_underwater] > 12*TICRATE + 1)
					{
						source->player->powers[pw_underwater] = 0;
					}
					else if (source->player->powers[pw_underwater] <= 12*TICRATE + 1 && source->player->powers[pw_underwater] > 0)
					{
						source->player->powers[pw_underwater] = 0;
						S_ChangeMusic(mus_runnin + gamemap - 1, 1);
					}
					S_StartSound (source->player->mo, sfx_shield);
					source->player->numboxes++;
					if(cv_gametype.value == 1 || cv_gametype.value == 3 || cv_gametype.value == 4) // Random box generation Tails 08-09-2001
						target->fuse = cv_itemrespawntime.value*TICRATE;
					break;
				case MT_BKTV: // Black Shield Box
					if(!(source->player->powers[pw_blackshield]))
					{
						P_SpawnMobj (source->player->mo->x, source->player->mo->y, source->player->mo->z, MT_BLACKORB)->target = source;
					}
					source->player->powers[pw_yellowshield] = false;
					source->player->powers[pw_greenshield] = false;
					source->player->powers[pw_blueshield] = false;
					source->player->powers[pw_blackshield] = true;
					S_StartSound (source->player->mo, sfx_shield);
					source->player->numboxes++;
					if(cv_gametype.value == 1 || cv_gametype.value == 3 || cv_gametype.value == 4) // Random box generation Tails 08-09-2001
						target->fuse = cv_itemrespawntime.value*TICRATE;
					break;
				case MT_MISC74: // Super Sneakers Box
					source->player->powers[pw_strength] = 20*TICRATE + 1;
					source->player->numboxes++;
					if(cv_gametype.value == 1 || cv_gametype.value == 3 || cv_gametype.value == 4) // Random box generation Tails 08-09-2001
						target->fuse = cv_itemrespawntime.value*TICRATE;
					break;
				case MT_PRUP: // 1-Up Box
					source->player->lives += 1;
					if(source->player==&players[consoleplayer])
					{
						S_StopMusic();
						S_ChangeMusic(mus_xtlife, false);
					}
					source->player->powers[pw_extralife] = 4*TICRATE + 1;
					source->player->numboxes++;
					if(cv_gametype.value == 1 || cv_gametype.value == 3 || cv_gametype.value == 4) // Random box generation Tails 08-09-2001
						target->fuse = cv_itemrespawntime.value*TICRATE;
				default:
					break;
			}
		}

		// count for intermission
		if (source->player && target->flags & MF_COUNTKILL)
			source->player->killcount++;

		if(source->player)
		{
			switch(target->type)
			{
				case MT_BLUECRAWLA:
				case MT_REDCRAWLA:
				case MT_GFZFISH:
				case MT_JETTBOMBER:
				case MT_JETTGUNNER:
					source->player->scoreadd++; // Tails 11-03-2000
					if(source->player->scoreadd == 1)
					{
						source->player->score+=100; // Score! Tails 03-01-2000
						P_SpawnMobj (target->x,target->y,target->z + (target->height / 2), MT_SCRA);
					}
					if(source->player->scoreadd == 2)
					{
						source->player->score+=200; // Score! Tails 03-01-2000
						P_SpawnMobj (target->x,target->y,target->z + (target->height / 2), MT_SCRB);
					}
					if(source->player->scoreadd == 3)
					{
						source->player->score+=500; // Score! Tails 03-01-2000
						P_SpawnMobj (target->x,target->y,target->z + (target->height / 2), MT_SCRC);
					}
					if(source->player->scoreadd >= 4)
					{
						source->player->score+=1000; // Score! Tails 03-01-2000
						P_SpawnMobj (target->x,target->y,target->z + (target->height / 2), MT_SCRD);
					}
					break;
				case MT_EGGMOBILE:
					source->player->score+=1000; // Tails 03-11-2000
					break;
				default:
					break;
			}
		}

		// count frags if player killed player
		if (target->player)
		{
			source->player->frags[target->player-players]++;

			// check fraglimit cvar
			if (cv_fraglimit.value)
				P_CheckFragLimit(source->player);
		}
	}
	else if (!multiplayer && (target->flags & MF_COUNTKILL))
	{
		// count all monster deaths,
		// even those caused by other monsters
		players[0].killcount++;
	}

	// if a player avatar dies...
	if (target->player)
	{
		// count environment kills against you (you fragged yourself!)
		if (!source)
			target->player->frags[target->player-players]++;

		target->flags &= ~MF_SOLID;                     // does not block
		target->player->playerstate = PST_DEAD;
		if(cv_gametype.value != 1)
			target->player->lives -= 1; // Lose a life Tails 03-11-2000
		if(target->player->lives <= 0) // Tails 03-14-2000
		{
			if(target->player==&players[consoleplayer])
			{
				S_StopMusic(); // Stop the Music! Tails 03-14-2000
				S_ChangeMusic(mus_gmover, false); // Yousa dead now, Okieday? Tails 03-14-2000
			}
		}
		P_DropWeapon (target->player);                  // put weapon away

		if (target->player == &players[consoleplayer])
		{
			// don't die in auto map,
			// switch view prior to dying
			if (automapactive)
				AM_Stop ();

			//added:22-02-98: recenter view for next live...
			localaiming = 0;
		}
		if (target->player == &players[secondarydisplayplayer])
		{
			//added:22-02-98: recenter view for next live...
			localaiming2 = 0;
		}
	}

	if (target->health < -target->info->spawnhealth
		&& target->info->xdeathstate)
	{
		P_SetMobjState (target, target->info->xdeathstate);
	}
	else
		P_SetMobjState (target, target->info->deathstate);

	target->tics -= P_Random()&3;

	if (target->tics < 1)
		target->tics = 1;

	//  I_StartSound (&actor->r, actor->info->deathsound);

	if(mariomode) // Mario Mode Tails 12-23-2001
		return;

	// Drop stuff.
	// This determines the kind of object spawned
	// during the death frame of a thing.
	switch (target->type)
	{
	  case MT_REDCRAWLA:
		item = MT_SQRL;
		break;

	  case MT_BLUECRAWLA:
	  case MT_JETTBOMBER:
	  case MT_GFZFISH:
		item = MT_BIRD;
		break;

	  case MT_JETTGUNNER:
		item = MT_MOUSE;
		break;

	  default:
		return;
	}

	mo = P_SpawnMobj (target->x,target->y,target->z + (target->height / 2), item);
	mo->flags |= MF_DROPPED;    // special versions of items
}


//
// P_DamageMobj
// Damages both enemies and players
// "inflictor" is the thing that caused the damage
//  creature or missile, can be NULL (slime, etc)
// "source" is the thing to target after taking damage
//  creature or NULL
// Source and inflictor are the same for melee attacks.
// Source can be NULL for slime, barrel explosions
// and other environmental stuff.
//
boolean P_DamageMobj ( mobj_t*       target,
					   mobj_t*       inflictor,
					   mobj_t*       source,
					   int           damage )
{
	unsigned    ang;
	player_t*   player;
	int         temp;
	boolean     takedamage;  // false on some case in teamplay

	if ( !(target->flags & MF_SHOOTABLE) )
		return false; // shouldn't happen...

	if (target->health <= 0)
		return false;

	if ( target->flags & MF_SKULLFLY )
	{
		target->momx = target->momy = target->momz = 0;
	}

	if(target->type == MT_EGGMOBILE)
	{
		if (target->state->nextstate == S_EGGMOBILE_PAIN || target->state->nextstate == S_EGGMOBILE_PAIN2 || target->state->nextstate == S_EGGMOBILE_PAIN3 || target->state->nextstate == S_EGGMOBILE_PAIN4 || target->state->nextstate == S_EGGMOBILE_PAIN5 || target->state->nextstate == S_EGGMOBILE_PAIN6 ||
			target->state->nextstate == S_EGGMOBILE_PAIN7 || target->state->nextstate == S_EGGMOBILE_PAIN8 || target->state->nextstate == S_EGGMOBILE_PAIN9 || target->state->nextstate == S_EGGMOBILE_PAIN10 || target->state->nextstate == S_EGGMOBILE_PAIN11 || target->state->nextstate == S_EGGMOBILE_PAIN12)
			return false;
	}

	player = target->player;

	if(player && player->exiting)
		return false;

	// 10,000 is flag for instant death! Tails
	if(player && target->type == MT_PLAYER)
	{
		if(source && source->player)
		{
			if(cv_gametype.value == 3)
			{
				if(target->player->powers[pw_invisibility] || target->player->tagzone || target->player->powers[pw_invulnerability])
					return false;

				if(source->player->tagit < 298*TICRATE && source->player->tagit > 0)
				{
					target->player->tagit = 300*TICRATE + 1;
					source->player->tagit = 0;
					target->player->tagcount++;
				}
					target->momx = 0;
					target->momy = 0;
					target->z++;
					if(target->eflags & MF_UNDERWATER)
						target->momz = 3.515384615385*FRACUNIT;
					else
						target->momz = 6*FRACUNIT;

					if(source->player->mfjumped && source->momz < 0)
						source->momz = -source->momz;

					ang = R_PointToAngle2 ( inflictor->x,
								inflictor->y,
								target->x,
								target->y);
					P_Thrust (target, ang, 4*FRACUNIT);
					target->player->mfjumped = 0;
					target->player->powers[pw_invisibility] = 105;
					P_SetMobjState(target, S_PLAY_PAIN);
				if(target->player->powers[pw_blueshield] || target->player->powers[pw_yellowshield] || target->player->powers[pw_blackshield] || target->player->powers[pw_greenshield])
				{
					target->player->powers[pw_blueshield] = false;      //Get rid of shield
					target->player->powers[pw_yellowshield] = false;
					target->player->powers[pw_blackshield] = false;
					target->player->powers[pw_greenshield] = false;
					S_StartSound (target, sfx_pldeth);
					return true;
				}
				damage = 1;
				target->player->damagecount += damage;

				if(target->health <= 1)
					S_StartSound (target, sfx_pldeth);

				if(target->health > 1)
				{
					S_StartSound (target, target->info->painsound);
					P_PlayerRingBurst(target->player);
				}

				target->player->health = 1;
				target->health = 1;
				return true;
			}
			else
			{
				if(cv_gametype.value == 4 && source && source->player)
				{
					if(target->player->ctfteam == source->player->ctfteam)
						return false;
				}
				if(damage == 10000)
				{
					damage = 10000;
					player->powers[pw_blueshield] = false;      //Get rid of shield
					player->powers[pw_yellowshield] = false;
					player->powers[pw_blackshield] = false;
					player->powers[pw_greenshield] = false;
					player->mo->momx = 0;
					player->mo->momy = 0;
					player->mo->momz = 3*JUMPGRAVITY;
					S_StartSound (target, sfx_pldeth);
					P_SetMobjState(target, S_PLAY_DIE1);
					if(cv_gametype.value == 4 && player->gotflag)
						P_PlayerFlagBurst(player);
				}
				else if ( damage < 10000 // start ignore bouncing & such in invulnerability Tails 02-26-2000
					&& ( (player->cheats&CF_GODMODE)
					|| player->powers[pw_invulnerability] || player->powers[pw_invisibility] || player->powers[pw_super]) )
				{
					damage = 0;
				} // end ignore bouncing & such in invulnerability Tails 02-26-2000

				else if ( damage < 10000 && (player->powers[pw_blueshield] || player->powers[pw_yellowshield] || player->powers[pw_blackshield] || player->powers[pw_greenshield]))  //If One-Hit Shield
				{
					player->powers[pw_blueshield] = false;      //Get rid of shield
					player->powers[pw_yellowshield] = false;
					player->powers[pw_greenshield] = false;
					if(player->powers[pw_blackshield])
					{
						player->blackow = 1;
						player->powers[pw_blackshield] = false;
						player->jumpdown = true;
					}
					damage=0;                 //Dont take rings away
					player->mo->momx = 0;
					player->mo->momy = 0;
					player->mo->z++;

					if(player->mo->eflags & MF_UNDERWATER)
							player->mo->momz = 3.515384615385*FRACUNIT;
						else
							player->mo->momz = 6*FRACUNIT;

					if(inflictor == NULL)
						P_Thrust (player->mo, -player->mo->angle, 4*FRACUNIT);
					else
					{
						ang = R_PointToAngle2 ( inflictor->x,
												inflictor->y,
												target->x,
												target->y);
						P_Thrust (target, ang, 4*FRACUNIT);
					}
					if (player->mfjumped == 1)
					{
						player->mfjumped = 0;
					}
					S_StartSound (target, sfx_pldeth);
					if(cv_gametype.value == 4 && player->gotflag)
						P_PlayerFlagBurst(player);
					if(cv_gametype.value == 1 && source && source->player) // Tails 03-13-2001
						source->player->score += 25; // Tails 03-13-2001
				}
				else if(player->mo->health > 1)
				{
					damage = player->mo->health - 1;
					player->mo->momx = 0;
					player->mo->momy = 0;
					player->mo->z++;

						if(player->mo->eflags & MF_UNDERWATER)
							player->mo->momz = 3.515384615385*FRACUNIT;
						else
							player->mo->momz = 6*FRACUNIT;

					if(inflictor == NULL)
						P_Thrust (player->mo, -player->mo->angle, 4*FRACUNIT);
					else
					{
						ang = R_PointToAngle2 ( inflictor->x,
												inflictor->y,
												target->x,
												target->y);
						P_Thrust (target, ang, 4*FRACUNIT);
					}
					if (player->mfjumped == 1)
						player->mfjumped = 0;

					S_StartSound (target, target->info->painsound);
					if(cv_gametype.value == 4 && player->gotflag)
						P_PlayerFlagBurst(player);
					if(cv_gametype.value == 1 && source && source->player) // Tails 03-13-2001
						source->player->score += 50; // Tails 03-13-2001
				}
				else
				{
					damage = 1;
					player->mo->health=1;
					player->mo->momz = JUMPGRAVITY*3;
					player->mo->momx = 0;
					player->mo->momy = 0;
					if (player->mfjumped == 1)
					{
						player->mfjumped = 0;
					}
					if(cv_gametype.value == 4 && player->gotflag)
						P_PlayerFlagBurst(player);
					if(cv_gametype.value == 1 && source && source->player) // Tails 03-13-2001
						source->player->score += 100; // Tails 03-13-2001
				}
			}
		}
		else
		{
			if(damage == 10000)
			{
				player->powers[pw_blueshield] = false;      //Get rid of shield
				player->powers[pw_yellowshield] = false;
				player->powers[pw_blackshield] = false;
				player->powers[pw_greenshield] = false;
				player->mo->momx = 0;
				player->mo->momy = 0;
				player->mo->momz = 3*JUMPGRAVITY;
				if(!(gamemap == SSSTAGE1 || gamemap == SSSTAGE2 || gamemap == SSSTAGE3
					|| gamemap == SSSTAGE4 || gamemap == SSSTAGE5 || gamemap == SSSTAGE6
					|| gamemap == SSSTAGE7))
				{
					S_StartSound (target, sfx_pldeth);
					P_SetMobjState(target, S_PLAY_DIE1);
					if(cv_gametype.value == 4 && player->gotflag)
						P_PlayerFlagBurst(player);
				}
				else
					damage = 0;
			}

			else if ( damage < 10000 // start ignore bouncing & such in invulnerability Tails 02-26-2000
				&& ( (player->cheats&CF_GODMODE)
				|| player->powers[pw_invulnerability] || player->powers[pw_invisibility] || player->powers[pw_super]) )
			{
				damage = 0;
			} // end ignore bouncing & such in invulnerability Tails 02-26-2000

			else if ( damage < 10000 && (player->powers[pw_blueshield] || player->powers[pw_yellowshield] || player->powers[pw_blackshield] || player->powers[pw_greenshield]))  //If One-Hit Shield
			{
				player->powers[pw_blueshield] = false;      //Get rid of shield
				player->powers[pw_yellowshield] = false;
				player->powers[pw_greenshield] = false;
				if(player->powers[pw_blackshield])
				{
					player->blackow = 1;
					player->powers[pw_blackshield] = false;
					player->jumpdown = true;;
				}
				damage=0;                 //Dont take rings away
				player->mo->momx = 0;
				player->mo->momy = 0;
				player->mo->z++;
					if(player->mo->eflags & MF_UNDERWATER)
						player->mo->momz = 3.515384615385*FRACUNIT;
					else
						player->mo->momz = 6*FRACUNIT;
				if(inflictor == NULL)
					P_Thrust (player->mo, -player->mo->angle, 4*FRACUNIT);
				else
				{
					ang = R_PointToAngle2 ( inflictor->x,
											inflictor->y,
											target->x,
											target->y);
					P_Thrust (target, ang, 4*FRACUNIT);
				}
				if (player->mfjumped == 1)
				{
					player->mfjumped = 0;
				}
				S_StartSound (target, sfx_pldeth);
				if(cv_gametype.value == 4 && player->gotflag)
					P_PlayerFlagBurst(player);
				if(cv_gametype.value == 1 && source && source->player) // Tails 03-13-2001
					source->player->score += 25; // Tails 03-13-2001
			}

			else if(player->mo->health > 1)
			{
				if(gamemap == SSSTAGE1 || gamemap == SSSTAGE2 || gamemap == SSSTAGE3
					|| gamemap == SSSTAGE4 || gamemap == SSSTAGE5 || gamemap == SSSTAGE6
					|| gamemap == SSSTAGE7)
					damage = (player->mo->health-1)/4;
				else
					damage = player->mo->health - 1;

				player->mo->momx = 0;
				player->mo->momy = 0;
				player->mo->z++;
				if(player->mo->eflags & MF_UNDERWATER)
					player->mo->momz = 3.515384615385*FRACUNIT;
				else
					player->mo->momz = 6*FRACUNIT;
				if(inflictor == NULL)
					P_Thrust (player->mo, -player->mo->angle, 4*FRACUNIT);
				else
				{
					ang = R_PointToAngle2 ( inflictor->x,
											inflictor->y,
											target->x,
											target->y);
					P_Thrust (target, ang, 4*FRACUNIT);
				}
				if (player->mfjumped == 1)
				{
					player->mfjumped = 0;
				}
				S_StartSound (target, target->info->painsound);
				if(cv_gametype.value == 4 && player->gotflag)
					P_PlayerFlagBurst(player);
				if(cv_gametype.value == 1 && source && source->player) // Tails 03-13-2001
					source->player->score += 50; // Tails 03-13-2001
			}
			else
			{
				player->mo->momz = JUMPGRAVITY*3;
				player->mo->momx = 0;
				player->mo->momy = 0;
				if (player->mfjumped == 1)
				{
					player->mfjumped = 0;
				}
				if(!(gamemap == SSSTAGE1 || gamemap == SSSTAGE2 || gamemap == SSSTAGE3
					|| gamemap == SSSTAGE4 || gamemap == SSSTAGE5 || gamemap == SSSTAGE6
					|| gamemap == SSSTAGE7))
				{
					damage = 1;
					player->mo->health=1;
					if(cv_gametype.value == 4 && player->gotflag)
						P_PlayerFlagBurst(player);
					if(cv_gametype.value == 1 && source && source->player) // Tails 03-13-2001
						source->player->score += 100; // Tails 03-13-2001
				}
				else
					damage = 0;
			}
		}
	}

	takedamage = false;
	// player specific
	if (player)
	{
		// end of game hell hack
		if (target->subsector->sector->special == 11
			&& damage >= target->health)
		{
			damage = target->health - 1;
		}


		// Below certain threshold,
		// ignore damage in GOD mode, or with INVUL power.
		if ( damage < 10000
			 && ( (player->cheats&CF_GODMODE)
				  || player->powers[pw_invulnerability] || player->powers[pw_invisibility] || player->powers[pw_super]) ) // flash tails 02-26-2000
		{
			return false;
		}

		// added team play and teamdamage (view logboris at 13-8-98 to understand)
		if( demoversion < 125   || // support old demoversion
			cv_teamdamage.value ||
			damage>1000         || // telefrag
			source==target      ||
			!source             ||
			!source->player     ||
			(
			 (cv_deathmatch.value || cv_gametype.value == 1 || cv_gametype.value == 4) // Tails 03-13-2001
			 &&
			 (!cv_teamplay.value ||
			  !ST_SameTeam(source->player,player)
			 )
			)
		  )
		{
			player->health -= damage;   // mirror mobj health here for Dave
			if (player->health < 0)
				player->health = 0;
			takedamage = true;

			if(damage < 10000)
			{
				player->damagecount += damage;  // add damage after armor / invuln

				P_PlayerRingBurst(player); // SoM

				target->player->powers[pw_invisibility] = 105; // Tails
			}

			if (player->damagecount > 100)
				player->damagecount = 100;  // teleport stomp does 10k points...

			temp = damage < 100 ? damage : 100;

			//added:22-02-98: force feedback ??? electro-shock???
			if (player == &players[consoleplayer])
				I_Tactile (40,10,40+temp*2);
		}
		player->attacker = source;
	}
	else
		takedamage = true;

	if( takedamage )
	{
		// do the damage
		target->health -= damage;
		if (target->health <= 0)
		{
			P_KillMobj (source, target);
			return true;
		}

		target->flags |= MF_JUSTHIT;    // fight back!
		P_SetMobjState (target, target->info->painstate);
		target->reactiontime = 0;           // we're awake now...
	}

	if(source && source != target) // Tails 06-17-2001
	{
		// if not intent on another player,
		// chase after this one
		target->target = source;
		target->threshold = BASETHRESHOLD;
		if (target->state == &states[target->info->spawnstate]
			&& target->info->seestate != S_NULL)
			P_SetMobjState (target, target->info->seestate);
	}

	return takedamage;
}

void P_PlayerRingBurst(player_t* player)
{
	int       num_rings;
	int       i;
	mobj_t*   mo;

	//If no health, don't spawn ring!
	if(player->mo->health <= 1)
		return;

	if(!player->damagecount)
		return;

	if(player->mo->health > 33)
		num_rings = 32;
	else if(gamemap == SSSTAGE1 || gamemap == SSSTAGE2 || gamemap == SSSTAGE3
	|| gamemap == SSSTAGE4 || gamemap == SSSTAGE5 || gamemap == SSSTAGE6
	|| gamemap == SSSTAGE7)
		num_rings = (player->mo->health-1)/4;
	else
		num_rings = player->mo->health - 1;

	for(i = 0; i<num_rings; i++)
	{
		mo = P_SpawnMobj(player->mo->x,
		player->mo->y,
		player->mo->z,
		MT_FLINGRING);

		// Make rings spill out around the player in 16 directions like SA, but spill like Sonic 2.
		// Technically a non-SA way of spilling rings. They just so happen to be a little similar.
		// Tails 05-11-2001
		if(i>15)
		{
			mo->momx = sin(i*22.5) * 3 * FRACUNIT;
			mo->momy = cos(i*22.5) * 3 * FRACUNIT;
			mo->momz = 8*FRACUNIT;
		}
		else
		{
			mo->momx = sin(i*22.5) * 2 * FRACUNIT;
			mo->momy = cos(i*22.5) * 2 * FRACUNIT;
			mo->momz = 6*FRACUNIT;
		}
		mo->fuse = 8*TICRATE;
	}
}

// Flag Burst for CTF Tails 08-02-2001
void P_PlayerFlagBurst(player_t* player)
{
	mobj_t*   mo = NULL;

	if(player->gotflag == 1)
		mo = P_SpawnMobj(player->mo->x,
		player->mo->y,
		player->mo->z,
		MT_REDFLAG);

	else if (player->gotflag == 2)
		mo = P_SpawnMobj(player->mo->x,
		player->mo->y,
		player->mo->z,
		MT_BLUEFLAG);

	mo->momx = sin(P_Random()) * 6 * FRACUNIT;
	mo->momy = cos(P_Random()) * 6 * FRACUNIT;
	mo->momz = 8*FRACUNIT;

	mo->spawnpoint = player->flagpoint;
	mo->fuse = cv_flagtime.value * TICRATE;
	player->gotflag = 0;
}
