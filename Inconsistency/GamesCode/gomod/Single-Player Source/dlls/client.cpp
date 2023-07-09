/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
// Robin, 4-22-98: Moved set_suicide_frame() here from player.cpp to allow us to 
//				   have one without a hardcoded player.mdl in tf_client.cpp

/*

===== client.cpp ========================================================

  client/server game specific stuff

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "saverestore.h"
#include "player.h"
#include "spectator.h"
#include "client.h"
#include "soundent.h"
#include "gamerules.h"
#include "game.h"
#include "customentity.h"
#include "weapons.h"
#include "weaponinfo.h"
#include "usercmd.h"
#include "netadr.h"
#include "render.h"


///////
#include "trains.h"
#include "nodes.h"

#include "monsters.h"
#include "shake.h"
#include "decals.h"
#include "hltv.h"
#include "basemonster.h"
//////
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>

// include the Logitech LCD SDK header
#include "../apiG15/lglcd.h"
// make sure we use the library
#pragma comment(lib, "lgLcd.lib")

// and include our sample bitmap
#include "samplebitmap.inl"

extern DLL_GLOBAL ULONG		g_ulModelIndexPlayer;
extern DLL_GLOBAL BOOL		g_fGameOver;
extern DLL_GLOBAL int		g_iSkillLevel;
extern DLL_GLOBAL ULONG		g_ulFrameCount;

extern void CopyToBodyQue(entvars_t* pev);
extern int giPrecacheGrunt;
extern int gmsgSayText;

extern int g_teamplay;
extern int m_allied;

void LinkUserMessages( void );

/*
 * used by kill command and disconnect command
 * ROBIN: Moved here from player.cpp, to allow multiple player models
 */
void set_suicide_frame(entvars_t* pev)
{       
	if (!FStrEq(STRING(pev->model), "models/player.mdl"))
		return; // allready gibbed

//	pev->frame		= $deatha11;
	pev->solid		= SOLID_NOT;
	pev->movetype	= MOVETYPE_TOSS;
	pev->deadflag	= DEAD_DEAD;
	pev->nextthink	= -1;
}


/*
===========
ClientConnect

called when a player connects to a server
============
*/
BOOL ClientConnect( edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[ 128 ]  )
{	
	return g_pGameRules->ClientConnected( pEntity, pszName, pszAddress, szRejectReason );

// a client connecting during an intermission can cause problems
//	if (intermission_running)
//		ExitIntermission ();

}


/*
===========
ClientDisconnect

called when a player disconnects from a server

GLOBALS ASSUMED SET:  g_fGameOver
============
*/
void ClientDisconnect( edict_t *pEntity )
{
	if (g_fGameOver)
		return;

	char text[256];
	sprintf( text, "- %s has left the game\n", STRING(pEntity->v.netname) );
	MESSAGE_BEGIN( MSG_ALL, gmsgSayText, NULL );
		WRITE_BYTE( ENTINDEX(pEntity) );
		WRITE_STRING( text );
	MESSAGE_END();

	CSound *pSound;
	pSound = CSoundEnt::SoundPointerForIndex( CSoundEnt::ClientSoundIndex( pEntity ) );
	{
		// since this client isn't around to think anymore, reset their sound. 
		if ( pSound )
		{
			pSound->Reset();
		}
	}

// since the edict doesn't get deleted, fix it so it doesn't interfere.
	pEntity->v.takedamage = DAMAGE_NO;// don't attract autoaim
	pEntity->v.solid = SOLID_NOT;// nonsolid
	UTIL_SetOrigin ( &pEntity->v, pEntity->v.origin );

	g_pGameRules->ClientDisconnected( pEntity );
}


// called by ClientKill and DeadThink
void respawn(entvars_t* pev, BOOL fCopyCorpse)
{
	if (gpGlobals->coop || gpGlobals->deathmatch)
	{
		if ( fCopyCorpse )
		{
			// make a copy of the dead body for appearances sake
			CopyToBodyQue(pev);
		}

		// respawn player
		GetClassPtr( (CBasePlayer *)pev)->Spawn( );
	}
	else
	{       // restart the entire server
		SERVER_COMMAND("reload\n");
	}
}

/*
============
ClientKill

Player entered the suicide command

GLOBALS ASSUMED SET:  g_ulModelIndexPlayer
============
*/
void ClientKill( edict_t *pEntity )
{
	entvars_t *pev = &pEntity->v;

	CBasePlayer *pl = (CBasePlayer*) CBasePlayer::Instance( pev );

	if ( pl->m_fNextSuicideTime > gpGlobals->time )
		return;  // prevent suiciding too ofter

	pl->m_fNextSuicideTime = gpGlobals->time + 1;  // don't let them suicide for 5 seconds after suiciding

	// have the player kill themself
	pev->health = 0;
	pl->Killed( pev, GIB_NEVER );

//	pev->modelindex = g_ulModelIndexPlayer;
//	pev->frags -= 2;		// extra penalty
//	respawn( pev );
}

/*
===========
ClientPutInServer

called each time a player is spawned
============
*/
void ClientPutInServer( edict_t *pEntity )
{
	CBasePlayer *pPlayer;

	entvars_t *pev = &pEntity->v;

	pPlayer = GetClassPtr((CBasePlayer *)pev);
	pPlayer->SetCustomDecalFrames(-1); // Assume none;

	// Allocate a CBasePlayer for pev, and call spawn
	pPlayer->Spawn() ;

	// Reset interpolation during first frame
	pPlayer->pev->effects |= EF_NOINTERP;
}

#include "voice_gamemgr.h"
extern CVoiceGameMgr g_VoiceGameMgr;

//// HOST_SAY
// String comes in as
// say blah blah blah
// or as
// blah blah blah
//
void Host_Say( edict_t *pEntity, int teamonly )
{
	CBasePlayer *client;
	int		j;
	char	*p;
	char	text[128];
	char    szTemp[256];
	const char *cpSay = "say";
	const char *cpSayTeam = "say_team";
	const char *pcmd = CMD_ARGV(0);

	// We can get a raw string now, without the "say " prepended
	if ( CMD_ARGC() == 0 )
		return;

	entvars_t *pev = &pEntity->v;
	CBasePlayer* player = GetClassPtr((CBasePlayer *)pev);

	//Not yet.
	if ( player->m_flNextChatTime > gpGlobals->time )
		 return;

	if ( !stricmp( pcmd, cpSay) || !stricmp( pcmd, cpSayTeam ) )
	{
		if ( CMD_ARGC() >= 2 )
		{
			p = (char *)CMD_ARGS();
		}
		else
		{
			// say with a blank message, nothing to do
			return;
		}
	}
	else  // Raw text, need to prepend argv[0]
	{
		if ( CMD_ARGC() >= 2 )
		{
			sprintf( szTemp, "%s %s", ( char * )pcmd, (char *)CMD_ARGS() );
		}
		else
		{
			// Just a one word command, use the first word...sigh
			sprintf( szTemp, "%s", ( char * )pcmd );
		}
		p = szTemp;
	}

// remove quotes if present
	if (*p == '"')
	{
		p++;
		p[strlen(p)-1] = 0;
	}

// make sure the text has content
	for ( char *pc = p; pc != NULL && *pc != 0; pc++ )
	{
		if ( isprint( *pc ) && !isspace( *pc ) )
		{
			pc = NULL;	// we've found an alphanumeric character,  so text is valid
			break;
		}
	}
	if ( pc != NULL )
		return;  // no character found, so say nothing

// turn on color set 2  (color on,  no sound)
	if ( teamonly )
		sprintf( text, "%c(TEAM) %s: ", 2, STRING( pEntity->v.netname ) );
	else
		sprintf( text, "%c%s: ", 2, STRING( pEntity->v.netname ) );

	j = sizeof(text) - 2 - strlen(text);  // -2 for /n and null terminator
	if ( (int)strlen(p) > j )
		p[j] = 0;

	strcat( text, p );
	strcat( text, "\n" );


	player->m_flNextChatTime = gpGlobals->time + CHAT_INTERVAL;

	// loop through all players
	// Start with the first player.
	// This may return the world in single player if the client types something between levels or during spawn
	// so check it, or it will infinite loop

	client = NULL;
	while ( ((client = (CBasePlayer*)UTIL_FindEntityByClassname( client, "player" )) != NULL) && (!FNullEnt(client->edict())) ) 
	{
		if ( !client->pev )
			continue;
		
		if ( client->edict() == pEntity )
			continue;

		if ( !(client->IsNetClient()) )	// Not a client ? (should never be true)
			continue;

		// can the receiver hear the sender? or has he muted him?
		if ( g_VoiceGameMgr.PlayerHasBlockedPlayer( client, player ) )
			continue;

		if ( teamonly && g_pGameRules->PlayerRelationship(client, CBaseEntity::Instance(pEntity)) != GR_TEAMMATE )
			continue;

		MESSAGE_BEGIN( MSG_ONE, gmsgSayText, NULL, client->pev );
			WRITE_BYTE( ENTINDEX(pEntity) );
			WRITE_STRING( text );
		MESSAGE_END();

	}

	// print to the sending client
	MESSAGE_BEGIN( MSG_ONE, gmsgSayText, NULL, &pEntity->v );
		WRITE_BYTE( ENTINDEX(pEntity) );
		WRITE_STRING( text );
	MESSAGE_END();

	// echo to server console
	g_engfuncs.pfnServerPrint( text );

	char * temp;
	if ( teamonly )
		temp = "say_team";
	else
		temp = "say";
	
	// team match?
	if ( g_teamplay )
	{
		UTIL_LogPrintf( "\"%s<%i><%s><%s>\" %s \"%s\"\n", 
			STRING( pEntity->v.netname ), 
			GETPLAYERUSERID( pEntity ),
			GETPLAYERAUTHID( pEntity ),
			g_engfuncs.pfnInfoKeyValue( g_engfuncs.pfnGetInfoKeyBuffer( pEntity ), "model" ),
			temp,
			p );
	}
	else
	{
		UTIL_LogPrintf( "\"%s<%i><%s><%i>\" %s \"%s\"\n", 
			STRING( pEntity->v.netname ), 
			GETPLAYERUSERID( pEntity ),
			GETPLAYERAUTHID( pEntity ),
			GETPLAYERUSERID( pEntity ),
			temp,
			p );
	}
}













	








/*
===========
ClientCommand
called each time a player uses a "cmd" command
============
*/

extern float g_flWeaponCheat;

CBaseEntity *FindEntityForwardClient( CBaseEntity *pMe )
		{
		TraceResult tr;
	
		UTIL_MakeVectors(pMe->pev->v_angle);
		UTIL_TraceLine(pMe->pev->origin + pMe->pev->view_ofs,pMe->pev->origin + pMe->pev->view_ofs + gpGlobals->v_forward * 8192,dont_ignore_monsters, pMe->edict(), &tr );
		if ( tr.flFraction != 1.0 && !FNullEnt( tr.pHit) )
		{
			CBaseEntity *pHit = CBaseEntity::Instance( tr.pHit );
			return pHit;
		}
		return NULL;
		}

// Use CMD_ARGV,  CMD_ARGV, and CMD_ARGC to get pointers the character string command.
void ClientCommand( edict_t *pEntity )
{
	const char *pcmd = CMD_ARGV(0);
	const char *pstr;

	// Is the client spawned yet?
	if ( !pEntity->pvPrivateData )
		return;

	entvars_t *pev = &pEntity->v;

	if ( FStrEq(pcmd, "say" ) )
	{
		Host_Say( pEntity, 0 );
	}
	else if ( FStrEq(pcmd, "say_team" ) )
	{
		Host_Say( pEntity, 1 );
	}
	else if ( FStrEq(pcmd, "fullupdate" ) )
	{
		GetClassPtr((CBasePlayer *)pev)->ForceClientDllUpdate(); 
	}
	else if ( FStrEq(pcmd, "give" ) )
	{
		if ( g_flWeaponCheat != 0.0)
		{
			int iszItem = ALLOC_STRING( CMD_ARGV(1) );	// Make a copy of the classname
			GetClassPtr((CBasePlayer *)pev)->GiveNamedItem( STRING(iszItem) );
		}
	}

	else if ( FStrEq(pcmd, "drop" ) )
	{
		// player is dropping an item. 
		GetClassPtr((CBasePlayer *)pev)->DropPlayerItem((char *)CMD_ARGV(1));
	}
	else if ( FStrEq(pcmd, "fov" ) )
	{
		if ( g_flWeaponCheat && CMD_ARGC() > 1)
		{
			GetClassPtr((CBasePlayer *)pev)->m_iFOV = atoi( CMD_ARGV(1) );
		}
		else
		{
			CLIENT_PRINTF( pEntity, print_console, UTIL_VarArgs( "\"fov\" is \"%d\"\n", (int)GetClassPtr((CBasePlayer *)pev)->m_iFOV ) );
		}
	}
	else if ( FStrEq(pcmd, "use" ) )
	{
		GetClassPtr((CBasePlayer *)pev)->SelectItem((char *)CMD_ARGV(1));
	}
	else if (((pstr = strstr(pcmd, "weapon_")) != NULL)  && (pstr == pcmd))
	{
		GetClassPtr((CBasePlayer *)pev)->SelectItem(pcmd);
	}
	else if (FStrEq(pcmd, "lastinv" ))
	{
		GetClassPtr((CBasePlayer *)pev)->SelectLastItem();
	}

	 
    else if ( FStrEq(pcmd, "vguimenu" ) )
    {
        if (CMD_ARGC() >= 1)
            GetClassPtr((CBasePlayer *)pev)->ShowVGUIMenu(atoi(CMD_ARGV(1)));
    }



	 // SPAWN HGRUNT
    else if ( FStrEq(pcmd, "hgrunt_button" ) )
    {
        
		UTIL_MakeVectors( Vector( 0, pev->v_angle.y, 0 ) );
		CBaseEntity::Create("monster_human_grunt", pev->origin + gpGlobals->v_forward * 128, pev->angles);
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "button.wav", 0.94, ATTN_NORM);
    }

    // End - SPAWN HGRUNT

	
	 // SPAWN HEADCRAB
    else if ( FStrEq(pcmd, "headcrab_button" ) )
    {
        
		UTIL_MakeVectors( Vector( 0, pev->v_angle.y, 0 ) );
		CBaseEntity::Create("monster_headcrab", pev->origin + gpGlobals->v_forward * 128, pev->angles);
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "button.wav", 0.94, ATTN_NORM);
	
    }

    // End - SPAWN HEADCRAB






	// SPAWN ALIEN SLAVE
    else if ( FStrEq(pcmd, "alienslave_button" ) )
    {
        
		UTIL_MakeVectors( Vector( 0, pev->v_angle.y, 0 ) );
		CBaseEntity::Create("monster_alien_slave", pev->origin + gpGlobals->v_forward * 128, pev->angles);
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "button.wav", 0.94, ATTN_NORM);
    }

    // End - SPAWN ALIEN SLAVE



	// SPAWN LEECH
    else if ( FStrEq(pcmd, "leech_button" ) )
    {
        
		UTIL_MakeVectors( Vector( 0, pev->v_angle.y, 0 ) );
		CBaseEntity::Create("monster_leech", pev->origin + gpGlobals->v_forward * 128, pev->angles);
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "button.wav", 0.94, ATTN_NORM);
    }

    // End - SPAWN LEECH



	// SPAWN zombie
    else if ( FStrEq(pcmd, "zombie_button" ) )
    {
        
		UTIL_MakeVectors( Vector( 0, pev->v_angle.y, 0 ) );
		CBaseEntity::Create("monster_zombie", pev->origin + gpGlobals->v_forward * 128, pev->angles);
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "button.wav", 0.94, ATTN_NORM);
    }

    // End - SPAWN zombie



		// SPAWN bullchicken
    else if ( FStrEq(pcmd, "bull_button" ) )
    {
        
		UTIL_MakeVectors( Vector( 0, pev->v_angle.y, 0 ) );
		CBaseEntity::Create("monster_bullchicken", pev->origin + gpGlobals->v_forward * 128, pev->angles);
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "button.wav", 0.94, ATTN_NORM);
    }

    // End - SPAWN bullchicken


	// SPAWN Barney
    else if ( FStrEq(pcmd, "barney_button" ) )
    {
        
		UTIL_MakeVectors( Vector( 0, pev->v_angle.y, 0 ) );
		if (!WALK_MOVE ( ENT(pev), 0, 0, WALKMOVE_NORMAL ) )
		{

		}
		else
		{

			CBaseEntity::Create("monster_barney", pev->origin + gpGlobals->v_forward * 128, pev->angles);
		}
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "button.wav", 0.94, ATTN_NORM);
    }

    // End - SPAWN Barney


	// SPAWN sentry
    else if ( FStrEq(pcmd, "sentry_button" ) )
    {
        
		UTIL_MakeVectors( Vector( 0, pev->v_angle.y, 0 ) );
		CBaseEntity::Create("monster_sentry", pev->origin + gpGlobals->v_forward * 128, pev->angles);
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "button.wav", 0.94, ATTN_NORM);
    }

    // End - SPAWN sentry

	
	// SPAWN tentacle
    else if ( FStrEq(pcmd, "tentacle_button" ) )
    {
        
		UTIL_MakeVectors( Vector( 0, pev->v_angle.y, 0 ) );
		CBaseEntity::Create("monster_tentacle", pev->origin + gpGlobals->v_forward * 128, pev->angles);
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "button.wav", 0.94, ATTN_NORM);
    }

    // End - SPAWN tentacle



	// SPAWN nihilant
    else if ( FStrEq(pcmd, "nihilant_button" ) )
    {
        
		UTIL_MakeVectors( Vector( 0, pev->v_angle.y, 0 ) );
		CBaseEntity::Create("monster_nihilanth", pev->origin + gpGlobals->v_forward * 200, pev->angles);
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "button.wav", 0.94, ATTN_NORM);
    }

    // End - SPAWN nihilant




		// SPAWN scientist
    else if ( FStrEq(pcmd, "scientist_button" ) )
    {
        
		UTIL_MakeVectors( Vector( 0, pev->v_angle.y, 0 ) );
		CBaseEntity::Create("monster_scientist", pev->origin + gpGlobals->v_forward * 128, pev->angles);
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "button.wav", 0.94, ATTN_NORM);
    }

    // End - SPAWN scientist


	// SPAWN houndeye
    else if ( FStrEq(pcmd, "houndeye_button" ) )
    {
        
		UTIL_MakeVectors( Vector( 0, pev->v_angle.y, 0 ) );
		CBaseEntity::Create("monster_houndeye", pev->origin + gpGlobals->v_forward * 128, pev->angles);
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "button.wav", 0.94, ATTN_NORM);
    }

    // End - SPAWN houndeye



	// SPAWN gman
    else if ( FStrEq(pcmd, "gman_button" ) )
    {
        
		UTIL_MakeVectors( Vector( 0, pev->v_angle.y, 0 ) );
		CBaseEntity::Create("monster_tripod", pev->origin + gpGlobals->v_forward * 128, pev->angles);
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "button.wav", 0.94, ATTN_NORM);
    }

    // End - SPAWN gman



	// SPAWN bigmomma
    else if ( FStrEq(pcmd, "bigmomma_button" ) )
    {
        
		UTIL_MakeVectors( Vector( 0, pev->v_angle.y, 0 ) );
		CBaseEntity::Create("monster_bigmomma", pev->origin + gpGlobals->v_forward * 128, pev->angles);
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "button.wav", 0.94, ATTN_NORM);
    }

    // End - SPAWN bigmomma



	// SPAWN aliengrunt
    else if ( FStrEq(pcmd, "aliengrunt_button" ) )
    {
        
		UTIL_MakeVectors( Vector( 0, pev->v_angle.y, 0 ) );
		CBaseEntity::Create("monster_alien_grunt", pev->origin + gpGlobals->v_forward * 128, pev->angles);
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "button.wav", 0.94, ATTN_NORM);
    }

    // End - SPAWN aliengrunt


	// SPAWN babycrab
    else if ( FStrEq(pcmd, "babycrab_button" ) )
    {
        
		UTIL_MakeVectors( Vector( 0, pev->v_angle.y, 0 ) );
		CBaseEntity::Create("monster_babycrab", pev->origin + gpGlobals->v_forward * 128, pev->angles);
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "button.wav", 0.94, ATTN_NORM);
    }

    // End - SPAWN babycrab



	
	// SPAWN aliencontroller
    else if ( FStrEq(pcmd, "aliencontroller_button" ) )
    {
        
		UTIL_MakeVectors( Vector( 0, pev->v_angle.y, 0 ) );
		CBaseEntity::Create("monster_alien_controller", pev->origin + gpGlobals->v_forward * 128, pev->angles);
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "button.wav", 0.94, ATTN_NORM);

    }

    // End - SPAWN aliencontroller


	// SPAWN gargantua
    else if ( FStrEq(pcmd, "gargantua_button" ) )
    {
        
		UTIL_MakeVectors( Vector( 0, pev->v_angle.y, 0 ) );
		CBaseEntity::Create("monster_gargantua", pev->origin + gpGlobals->v_forward * 128, pev->angles);
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "button.wav", 0.94, ATTN_NORM);

    }

    // End - SPAWN aliencontroller



	// SPAWN cockroach
    else if ( FStrEq(pcmd, "cockroach_button" ) )
    {
        
		UTIL_MakeVectors( Vector( 0, pev->v_angle.y, 0 ) );
		CBaseEntity::Create("monster_cockroach", pev->origin + gpGlobals->v_forward * 128, pev->angles);

			EMIT_SOUND(ENT(pev), CHAN_VOICE, "button.wav", 0.94, ATTN_NORM);
    }

    // End - SPAWN cockroach



		// SPAWN Hassassin
    else if ( FStrEq(pcmd, "hassassin_button" ) )
    {
        
		UTIL_MakeVectors( Vector( 0, pev->v_angle.y, 0 ) );
		CBaseEntity::Create("monster_human_assassin", pev->origin + gpGlobals->v_forward * 128, pev->angles);

			EMIT_SOUND(ENT(pev), CHAN_VOICE, "button.wav", 0.94, ATTN_NORM);
    }

    // End - SPAWN Hassassin


	
		// SPAWN Gonome
    else if ( FStrEq(pcmd, "gonome_button" ) )
    {
        
		UTIL_MakeVectors( Vector( 0, pev->v_angle.y, 0 ) );
		CBaseEntity::Create("monster_gonome", pev->origin + gpGlobals->v_forward * 128, pev->angles);

			EMIT_SOUND(ENT(pev), CHAN_VOICE, "button.wav", 0.94, ATTN_NORM);
    }

    // End - SPAWN Gonome




		// SPAWN Massn
    else if ( FStrEq(pcmd, "massn_button" ) )
    {
        
		UTIL_MakeVectors( Vector( 0, pev->v_angle.y, 0 ) );
		CBaseEntity::Create("monster_human_massn", pev->origin + gpGlobals->v_forward * 128, pev->angles);
	//	r_duplicatemode = 1;


			EMIT_SOUND(ENT(pev), CHAN_VOICE, "button.wav", 0.94, ATTN_NORM);
    }

    // End - SPAWN Massn



	
	// SPAWN Pit Drone
    else if ( FStrEq(pcmd, "pitdrone_button" ) )
    {
        
		UTIL_MakeVectors( Vector( 0, pev->v_angle.y, 0 ) );
		CBaseEntity::Create("monster_pit_drone", pev->origin + gpGlobals->v_forward * 128, pev->angles);

			EMIT_SOUND(ENT(pev), CHAN_VOICE, "button.wav", 0.94, ATTN_NORM);
    }

    // End - SPAWN Pit Drone



		// SPAWN Sentry Ally
    else if ( FStrEq(pcmd, "ally_sentry_button" ) )
    {
        
		UTIL_MakeVectors( Vector( 0, pev->v_angle.y, 0 ) );
		CBaseEntity::Create("monster_sentry_ally", pev->origin + gpGlobals->v_forward * 200, pev->angles);
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "button.wav", 0.94, ATTN_NORM);
    }

    // End - SPAWN Sentry Ally


	 // SPAWN Otis
    else if ( FStrEq(pcmd, "otis_button" ) )
    {
        
		UTIL_MakeVectors( Vector( 0, pev->v_angle.y, 0 ) );
		CBaseEntity::Create("barrel", pev->origin + gpGlobals->v_forward * 128, pev->angles);
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "button.wav", 0.94, ATTN_NORM);
    }

	// End - SPAWN Otis


		// SPAWN zombie_hl2
    else if ( FStrEq(pcmd, "hl2_zombie_button" ) )
    {
        
		UTIL_MakeVectors( Vector( 0, pev->v_angle.y, 0 ) );
		CBaseEntity::Create("monster_zombie_hl2", pev->origin + gpGlobals->v_forward * 128, pev->angles);
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "button.wav", 0.94, ATTN_NORM);
    }

    // End - SPAWN zombie_hl2

			// SPAWN Hl2_Headcrab
    else if ( FStrEq(pcmd, "hl2_headcrab_button" ) )
    {
        
		UTIL_MakeVectors( Vector( 0, pev->v_angle.y, 0 ) );
		CBaseEntity::Create("monster_hl2headcrab", pev->origin + gpGlobals->v_forward * 128, pev->angles);

			EMIT_SOUND(ENT(pev), CHAN_VOICE, "button.wav", 0.94, ATTN_NORM);
    }

    // End - SPAWN Hl2_Headcrab


	// SPAWN ichthyosaur
    else if ( FStrEq(pcmd, "ichthyosaur_button" ) )
    {
        
		UTIL_MakeVectors( Vector( 0, pev->v_angle.y, 0 ) );
		CBaseEntity::Create("monster_ichthyosaur", pev->origin + gpGlobals->v_forward * 128, pev->angles);

			EMIT_SOUND(ENT(pev), CHAN_VOICE, "button.wav", 0.94, ATTN_NORM);
    }

    // End - SPAWN ichthyosaur


	// GIVE 9mmhandgun
    else if ( FStrEq(pcmd, "9mm_button" ) )
    {
        
	edict_t	*pent;
	int istr = MAKE_STRING("weapon_9mmhandgun");
	

	pent = CREATE_NAMED_ENTITY(istr);
	if ( FNullEnt( pent ) )
	{
		ALERT ( at_console, "NULL Ent in GiveNamedItem!\n" );
		return;
	}
	VARS( pent )->origin = pev->origin;
	pent->v.spawnflags |= SF_NORESPAWN;

	DispatchSpawn( pent );
	DispatchTouch( pent, ENT( pev ) );

		EMIT_SOUND(ENT(pev), CHAN_VOICE, "button.wav", 0.94, ATTN_NORM);

    }

    // End - GIVE 9mmhandgun



	
	// GIVE 9mmAR
    else if ( FStrEq(pcmd, "9mmAR_button" ) )
    {
        
	edict_t	*pent;
	int istr = MAKE_STRING("weapon_9mmAR");
	

	pent = CREATE_NAMED_ENTITY(istr);
	if ( FNullEnt( pent ) )
	{
		ALERT ( at_console, "NULL Ent in GiveNamedItem!\n" );
		return;
	}
	VARS( pent )->origin = pev->origin;
	pent->v.spawnflags |= SF_NORESPAWN;

	DispatchSpawn( pent );
	DispatchTouch( pent, ENT( pev ) );

		EMIT_SOUND(ENT(pev), CHAN_VOICE, "button.wav", 0.94, ATTN_NORM);

    }

    // End - GIVE 9mmAR


	
	// GIVE shotgun
    else if ( FStrEq(pcmd, "shotgun_button" ) )
    {
        
	edict_t	*pent;
	int istr = MAKE_STRING("weapon_shotgun");
	

	pent = CREATE_NAMED_ENTITY(istr);
	if ( FNullEnt( pent ) )
	{
		ALERT ( at_console, "NULL Ent in GiveNamedItem!\n" );
		return;
	}
	VARS( pent )->origin = pev->origin;
	pent->v.spawnflags |= SF_NORESPAWN;

	DispatchSpawn( pent );
	DispatchTouch( pent, ENT( pev ) );

		EMIT_SOUND(ENT(pev), CHAN_VOICE, "button.wav", 0.94, ATTN_NORM);

    }

    // End - GIVE shotgun


		// GIVE gauss
    else if ( FStrEq(pcmd, "gauss_button" ) )
    {
        
	edict_t	*pent;
	int istr = MAKE_STRING("weapon_gauss");
	

	pent = CREATE_NAMED_ENTITY(istr);
	if ( FNullEnt( pent ) )
	{
		ALERT ( at_console, "NULL Ent in GiveNamedItem!\n" );
		return;
	}
	VARS( pent )->origin = pev->origin;
	pent->v.spawnflags |= SF_NORESPAWN;

	DispatchSpawn( pent );
	DispatchTouch( pent, ENT( pev ) );

		EMIT_SOUND(ENT(pev), CHAN_VOICE, "button.wav", 0.94, ATTN_NORM);

    }

    // End - GIVE gauss


			// GIVE Grenade
    else if ( FStrEq(pcmd, "grenade_button" ) )
    {
        
	edict_t	*pent;
	int istr = MAKE_STRING("weapon_handgrenade");
	

	pent = CREATE_NAMED_ENTITY(istr);
	if ( FNullEnt( pent ) )
	{
		ALERT ( at_console, "NULL Ent in GiveNamedItem!\n" );
		return;
	}
	VARS( pent )->origin = pev->origin;
	pent->v.spawnflags |= SF_NORESPAWN;

	DispatchSpawn( pent );
	DispatchTouch( pent, ENT( pev ) );

		EMIT_SOUND(ENT(pev), CHAN_VOICE, "button.wav", 0.94, ATTN_NORM);

    }

    // End - GIVE Grenade




		// GIVE crowbar
    else if ( FStrEq(pcmd, "crowbar_button" ) )
    {
        
	edict_t	*pent;
	int istr = MAKE_STRING("weapon_crowbar");
	

	pent = CREATE_NAMED_ENTITY(istr);
	if ( FNullEnt( pent ) )
	{
		ALERT ( at_console, "NULL Ent in GiveNamedItem!\n" );
		return;
	}
	VARS( pent )->origin = pev->origin;
	pent->v.spawnflags |= SF_NORESPAWN;

	DispatchSpawn( pent );
	DispatchTouch( pent, ENT( pev ) );

		EMIT_SOUND(ENT(pev), CHAN_VOICE, "button.wav", 0.94, ATTN_NORM);

    }

    // End - GIVE crowbar
	


	
		// GIVE Egon
    else if ( FStrEq(pcmd, "egon_button" ) )
    {
        
	edict_t	*pent;
	int istr = MAKE_STRING("weapon_egon");
	

	pent = CREATE_NAMED_ENTITY(istr);
	if ( FNullEnt( pent ) )
	{
		ALERT ( at_console, "NULL Ent in GiveNamedItem!\n" );
		return;
	}
	VARS( pent )->origin = pev->origin;
	pent->v.spawnflags |= SF_NORESPAWN;

	DispatchSpawn( pent );
	DispatchTouch( pent, ENT( pev ) );

		EMIT_SOUND(ENT(pev), CHAN_VOICE, "button.wav", 0.94, ATTN_NORM);

    }

    // End - GIVE Egon



		// GIVE satchel
    else if ( FStrEq(pcmd, "satchel_button" ) )
    {
        
	edict_t	*pent;
	int istr = MAKE_STRING("weapon_satchel");
	

	pent = CREATE_NAMED_ENTITY(istr);
	if ( FNullEnt( pent ) )
	{
		ALERT ( at_console, "NULL Ent in GiveNamedItem!\n" );
		return;
	}
	VARS( pent )->origin = pev->origin;
	pent->v.spawnflags |= SF_NORESPAWN;

	DispatchSpawn( pent );
	DispatchTouch( pent, ENT( pev ) );

		EMIT_SOUND(ENT(pev), CHAN_VOICE, "button.wav", 0.94, ATTN_NORM);

    }

    // End - GIVE satchel



	

		// GIVE rpg
    else if ( FStrEq(pcmd, "rpg_button" ) )
    {
     
	edict_t	*pent;
	int istr = MAKE_STRING("weapon_rpg");
	

	pent = CREATE_NAMED_ENTITY(istr);
	if ( FNullEnt( pent ) )
	{
		ALERT ( at_console, "NULL Ent in GiveNamedItem!\n" );
		return;
	}
	VARS( pent )->origin = pev->origin;
	pent->v.spawnflags |= SF_NORESPAWN;

	DispatchSpawn( pent );
	DispatchTouch( pent, ENT( pev ) );

	EMIT_SOUND(ENT(pev), CHAN_VOICE, "button.wav", 0.94, ATTN_NORM);
	


    }

    // End - GIVE rpg






	// GIVE 357
    else if ( FStrEq(pcmd, "357_button" ) )
    {
        
	edict_t	*pent;
	int istr = MAKE_STRING("weapon_357");
	

	pent = CREATE_NAMED_ENTITY(istr);
	if ( FNullEnt( pent ) )
	{
		ALERT ( at_console, "NULL Ent in GiveNamedItem!\n" );
		return;
	}
	VARS( pent )->origin = pev->origin;
	pent->v.spawnflags |= SF_NORESPAWN;

	DispatchSpawn( pent );
	DispatchTouch( pent, ENT( pev ) );

    }

    // End - GIVE 357

		
	// GIVE CROSSBOW
    else if ( FStrEq(pcmd, "crossbow_button" ) )
    {
        
	edict_t	*pent;
	int istr = MAKE_STRING("weapon_crossbow");
	

	pent = CREATE_NAMED_ENTITY(istr);
	if ( FNullEnt( pent ) )
	{
		ALERT ( at_console, "NULL Ent in GiveNamedItem!\n" );
		return;
	}
	VARS( pent )->origin = pev->origin;
	pent->v.spawnflags |= SF_NORESPAWN;

	DispatchSpawn( pent );
	DispatchTouch( pent, ENT( pev ) );

		EMIT_SOUND(ENT(pev), CHAN_VOICE, "button.wav", 0.94, ATTN_NORM);

    }

    // End - GIVE CROSSBOW


	// GIVE Removetool
    else if ( FStrEq(pcmd, "removetool_button" ) )
    {
        
	edict_t	*pent;
	int istr = MAKE_STRING("weapon_removetool");
	

	pent = CREATE_NAMED_ENTITY(istr);
	if ( FNullEnt( pent ) )
	{
		ALERT ( at_console, "NULL Ent in GiveNamedItem!\n" );
		return;
	}
	VARS( pent )->origin = pev->origin;
	pent->v.spawnflags |= SF_NORESPAWN;

	DispatchSpawn( pent );
	DispatchTouch( pent, ENT( pev ) );

    }

    // End - GIVE Removetool



		// GIVE CSMP5
    else if ( FStrEq(pcmd, "csmp5_button" ) )
    {
        
	edict_t	*pent;
	int istr = MAKE_STRING("weapon_hkmp5");
	

	pent = CREATE_NAMED_ENTITY(istr);
	if ( FNullEnt( pent ) )
	{
		ALERT ( at_console, "NULL Ent in GiveNamedItem!\n" );
		return;
	}
	VARS( pent )->origin = pev->origin;
	pent->v.spawnflags |= SF_NORESPAWN;

	DispatchSpawn( pent );
	DispatchTouch( pent, ENT( pev ) );

    }

    // End - GIVE CSMP5


	// GIVE P228
    else if ( FStrEq(pcmd, "csp228_button" ) )
    {
        
	edict_t	*pent;
	int istr = MAKE_STRING("weapon_p228");
	

	pent = CREATE_NAMED_ENTITY(istr);
	if ( FNullEnt( pent ) )
	{
		ALERT ( at_console, "NULL Ent in GiveNamedItem!\n" );
		return;
	}
	VARS( pent )->origin = pev->origin;
	pent->v.spawnflags |= SF_NORESPAWN;

	DispatchSpawn( pent );
	DispatchTouch( pent, ENT( pev ) );

    }

    // End - GIVE P228


	
	// GIVE CSM3
    else if ( FStrEq(pcmd, "csm3_button" ) )
    {
        
	edict_t	*pent;
	int istr = MAKE_STRING("weapon_m3");
	

	pent = CREATE_NAMED_ENTITY(istr);
	if ( FNullEnt( pent ) )
	{
		ALERT ( at_console, "NULL Ent in GiveNamedItem!\n" );
		return;
	}
	VARS( pent )->origin = pev->origin;
	pent->v.spawnflags |= SF_NORESPAWN;

	DispatchSpawn( pent );
	DispatchTouch( pent, ENT( pev ) );

    }

    // End - GIVE M3



	// GIVE CSGLOCK18
    else if ( FStrEq(pcmd, "csglock18_button" ) )
    {
        
	edict_t	*pent;
	int istr = MAKE_STRING("weapon_glock18");
	

	pent = CREATE_NAMED_ENTITY(istr);
	if ( FNullEnt( pent ) )
	{
		ALERT ( at_console, "NULL Ent in GiveNamedItem!\n" );
		return;
	}
	VARS( pent )->origin = pev->origin;
	pent->v.spawnflags |= SF_NORESPAWN;

	DispatchSpawn( pent );
	DispatchTouch( pent, ENT( pev ) );

    }

    // End - GIVE P228



	// GIVE CSMAC10
    else if ( FStrEq(pcmd, "csmac10_button" ) )
    {
        
	edict_t	*pent;
	int istr = MAKE_STRING("weapon_mac10");
	

	pent = CREATE_NAMED_ENTITY(istr);
	if ( FNullEnt( pent ) )
	{
		ALERT ( at_console, "NULL Ent in GiveNamedItem!\n" );
		return;
	}
	VARS( pent )->origin = pev->origin;
	pent->v.spawnflags |= SF_NORESPAWN;

	DispatchSpawn( pent );
	DispatchTouch( pent, ENT( pev ) );

    }

    // End - GIVE CSMAC10



	// GIVE P90
    else if ( FStrEq(pcmd, "csp90_button" ) )
    {
        
		
	edict_t	*pent;
	int istr = MAKE_STRING("weapon_p90");
	

	pent = CREATE_NAMED_ENTITY(istr);
	if ( FNullEnt( pent ) )
	{
		ALERT ( at_console, "NULL Ent in GiveNamedItem!\n" );
		return;
	}
	VARS( pent )->origin = pev->origin;
	pent->v.spawnflags |= SF_NORESPAWN;

	DispatchSpawn( pent );
	DispatchTouch( pent, ENT( pev ) );
	
		CBasePlayer *pPlayer;
		pPlayer = GetClassPtr((CBasePlayer *)pev);
	
    }

    // End - GIVE p90











	// GIVE AK47
    else if ( FStrEq(pcmd, "csak47_button" ) )
    {
        
	edict_t	*pent;
	int istr = MAKE_STRING("weapon_ak47");
	

	pent = CREATE_NAMED_ENTITY(istr);
	if ( FNullEnt( pent ) )
	{
		ALERT ( at_console, "NULL Ent in GiveNamedItem!\n" );
		return;
	}
	VARS( pent )->origin = pev->origin;
	pent->v.spawnflags |= SF_NORESPAWN;

	DispatchSpawn( pent );
	DispatchTouch( pent, ENT( pev ) );

    }

    // End - GIVE AK47



		// GIVE M4A1
    else if ( FStrEq(pcmd, "csm4a1_button" ) )
    {
        
	edict_t	*pent;
	int istr = MAKE_STRING("weapon_m4a1");
	

	pent = CREATE_NAMED_ENTITY(istr);
	if ( FNullEnt( pent ) )
	{
		ALERT ( at_console, "NULL Ent in GiveNamedItem!\n" );
		return;
	}
	VARS( pent )->origin = pev->origin;
	pent->v.spawnflags |= SF_NORESPAWN;

	DispatchSpawn( pent );
	DispatchTouch( pent, ENT( pev ) );

    }

    // End - GIVE M4A1







		// GIVE USP
    else if ( FStrEq(pcmd, "csusp_button" ) )
    {
        
	edict_t	*pent;
	int istr = MAKE_STRING("weapon_usp");
	

	pent = CREATE_NAMED_ENTITY(istr);
	if ( FNullEnt( pent ) )
	{
		ALERT ( at_console, "NULL Ent in GiveNamedItem!\n" );
		return;
	}
	VARS( pent )->origin = pev->origin;
	pent->v.spawnflags |= SF_NORESPAWN;

	DispatchSpawn( pent );
	DispatchTouch( pent, ENT( pev ) );

    }

    // End - GIVE USP








	
	// GIVE FIVESEVEN
    else if ( FStrEq(pcmd, "csfiveseven_button" ) )
    {
        
	edict_t	*pent;
	int istr = MAKE_STRING("weapon_fiveseven");
	

	pent = CREATE_NAMED_ENTITY(istr);
	if ( FNullEnt( pent ) )
	{
		ALERT ( at_console, "NULL Ent in GiveNamedItem!\n" );
		return;
	}
	VARS( pent )->origin = pev->origin;
	pent->v.spawnflags |= SF_NORESPAWN;

	DispatchSpawn( pent );
	DispatchTouch( pent, ENT( pev ) );

    }

    // End - GIVE FIVESEVEN



	// GIVE XM1014
    else if ( FStrEq(pcmd, "csxm1014_button" ) )
    {
        
	edict_t	*pent;
	int istr = MAKE_STRING("weapon_xm1014");
	

	pent = CREATE_NAMED_ENTITY(istr);
	if ( FNullEnt( pent ) )
	{
		ALERT ( at_console, "NULL Ent in GiveNamedItem!\n" );
		return;
	}
	VARS( pent )->origin = pev->origin;
	pent->v.spawnflags |= SF_NORESPAWN;

	DispatchSpawn( pent );
	DispatchTouch( pent, ENT( pev ) );

    }

    // End - GIVE XM1014





	// GIVE UMP45
    else if ( FStrEq(pcmd, "csump_button" ) )
    {
        
	edict_t	*pent;
	int istr = MAKE_STRING("weapon_ump45");
	

	pent = CREATE_NAMED_ENTITY(istr);
	if ( FNullEnt( pent ) )
	{
		ALERT ( at_console, "NULL Ent in GiveNamedItem!\n" );
		return;
	}
	VARS( pent )->origin = pev->origin;
	pent->v.spawnflags |= SF_NORESPAWN;

	DispatchSpawn( pent );
	DispatchTouch( pent, ENT( pev ) );



    }

    // End - GIVE XM1014


		// GIVE PHYSGUN
    else if ( FStrEq(pcmd, "physgun" ) )
    {
        
	edict_t	*pent;
	int istr = MAKE_STRING("weapon_physgun");
	

	pent = CREATE_NAMED_ENTITY(istr);
	if ( FNullEnt( pent ) )
	{
		ALERT ( at_console, "NULL Ent in GiveNamedItem!\n" );
		return;
	}
	VARS( pent )->origin = pev->origin;
	pent->v.spawnflags |= SF_NORESPAWN;

	DispatchSpawn( pent );
	DispatchTouch( pent, ENT( pev ) );



    }

    // End - GIVE XM1014


	//////// END WEAPONS \\\\\\\\


		// ENABLE IA
    else if ( FStrEq(pcmd, "enableia_button" ) )
    {
		CBasePlayer * pPlayer = GetClassPtr((CBasePlayer *)pev);

		CVAR_SET_FLOAT( "disable_ia", 0 );

		char szText[201];
            hudtextparms_t     hText;

            sprintf(szText, "I.A Enabled", "");
            
            memset(&hText, 0, sizeof(hText));
            hText.channel = 1;
     // These X and Y coordinates are just above
    //  the health meter.
            hText.x = 0.01;
            hText.y = 0.5;
    
            hText.effect = 0;    // Fade in/out
            
            hText.r1 = hText.g1 = hText.b1 = 255;
            hText.a1 = 255;

            hText.r2 = hText.g2 = hText.b2 = 255;
            hText.a2 = 255;

            hText.fadeinTime = 0.2;
            hText.fadeoutTime = 1;
            hText.holdTime = 1.5;
            hText.fxTime = 0.5;

            UTIL_HudMessage(pPlayer, hText, szText);

	
    }

    // End - ENABLE IA





	// DISABLE IA
    else if ( FStrEq(pcmd, "disableia_button" ) )
    {
			CBasePlayer * pPlayer = GetClassPtr((CBasePlayer *)pev);
		CVAR_SET_FLOAT( "disable_ia", 1 );
		char szText[201];
            hudtextparms_t     hText;

            sprintf(szText, "I.A DISABLED", "");
            
            memset(&hText, 0, sizeof(hText));
            hText.channel = 1;
            // These X and Y coordinates are just above
    //  the health meter.
            hText.x = 0.01;
            hText.y = 0.5;
    
            hText.effect = 0;    // Fade in/out
            
            hText.r1 = hText.g1 = hText.b1 = 255;
            hText.a1 = 255;

            hText.r2 = hText.g2 = hText.b2 = 255;
            hText.a2 = 255;

            hText.fadeinTime = 0.2;
            hText.fadeoutTime = 1;
            hText.holdTime = 1.5;
            hText.fxTime = 0.5;

            UTIL_HudMessage(pPlayer, hText, szText);
    }

    // End - DISABLE IA


	// GRASS ON

 else if ( FStrEq(pcmd, "grass_on" ) )
    {
	 CBasePlayer *pPlayer;
		pPlayer = GetClassPtr((CBasePlayer *)pev);
	// clear all particlesystems with this hijacked message
		extern int gmsgParticles;
		MESSAGE_BEGIN(MSG_ONE, gmsgParticles, NULL, pPlayer->pev);
			WRITE_SHORT(0);
			WRITE_BYTE(0);
			WRITE_COORD(0);
			WRITE_COORD(0);
			WRITE_COORD(0);
			WRITE_COORD(0);
			WRITE_COORD(0);
			WRITE_COORD(0);
			WRITE_SHORT(9999);
			WRITE_STRING("");
		MESSAGE_END();

	//	pPlayer->m_bSpawnPS = true;
		pPlayer->m_bSpawnGrass = true;
	//	pPlayer->m_flLastPSSpawn = 0.0;
		pPlayer->m_flLastGrassSpawn = 0.0;
		pPlayer->pLastGrassSpawned = NULL;
		pPlayer->pLastPSSpawned = NULL;

 }

 // End - GRASS ON



 	// GRASS OFF

 else if ( FStrEq(pcmd, "grass_off" ) )
    {
	 CBasePlayer *pPlayer;
		pPlayer = GetClassPtr((CBasePlayer *)pev);
	// clear all particlesystems with this hijacked message
		extern int gmsgParticles;
		MESSAGE_BEGIN(MSG_ONE, gmsgParticles, NULL, pPlayer->pev);
			WRITE_SHORT(0);
			WRITE_BYTE(0);
			WRITE_COORD(0);
			WRITE_COORD(0);
			WRITE_COORD(0);
			WRITE_COORD(0);
			WRITE_COORD(0);
			WRITE_COORD(0);
			WRITE_SHORT(9999);
			WRITE_STRING("");
		MESSAGE_END();

	//	pPlayer->m_bSpawnPS = true;
		pPlayer->m_bSpawnGrass = false;
	//	pPlayer->m_flLastPSSpawn = 0.0;
		pPlayer->m_flLastGrassSpawn = 0.0;
		pPlayer->pLastGrassSpawned = NULL;
		pPlayer->pLastPSSpawned = NULL;

 }

 // End - GRASS OFF


	// RED RENDER+ COLOR
    else if ( FStrEq(pcmd, "red+" ) )
    {
        
		r_colorx = r_colorx + 1;
		UTIL_MakeVectors( Vector( 0, pev->v_angle.y, 0 ) );
		
	
    }

    // End - RED RENDER+ COLOR







		// RED RENDER- COLOR
    else if ( FStrEq(pcmd, "red-" ) )
    {
        
	
		r_colorx = r_colorx - 1;

		UTIL_MakeVectors( Vector( 0, pev->v_angle.y, 0 ) );


    }

    // End - RED RENDER- COLOR



		// RED GREEN+ COLOR
    else if ( FStrEq(pcmd, "green+" ) )
    {
        
		r_colory = r_colory + 1;
    }
    // End -  GREEN+ COLOR




	// RED GREEN- COLOR
    else if ( FStrEq(pcmd, "green-" ) )
    {
        
		r_colory = r_colory - 1;


    }

    // End -  GREEN- COLOR











	// RED BLUE+ COLOR
    else if ( FStrEq(pcmd, "blue+" ) )
    {
        
		r_colorz = r_colorz + 1;


    }

    // End -  BLUE+ COLOR




	// RED BLUE- COLOR
    else if ( FStrEq(pcmd, "blue-" ) )
    {
        
		r_colorz = r_colorz - 1;

    }

    // End -  BLUE- COLOR



	// KNORMAL  
    else if ( FStrEq(pcmd, "knormal" ) )
    {
        
		r_render = 1;
	}

    // End -  KNORMAL


		
	
	// KGLOW 
    else if ( FStrEq(pcmd, "kglow" ) )
    {
        
	
		r_render = 2;

    }

    // End -  KGLOW






	// KTRANSCOLOR 
    else if ( FStrEq(pcmd, "ktranscolor" ) )
    {
        
	
		r_render = 3;


    }

    // End -  KTRANSCOLOR




	
	// KTRANSALPHA 
    else if ( FStrEq(pcmd, "ktransalpha" ) )
    {
        
	
		r_render = 4;


    }

    // End -  KTRANSALPHA




	// KTRANSADD
    else if ( FStrEq(pcmd, "ktransadd" ) )
    {
        
	
		r_render = 5;


    }

    // End -  KTRANSADD





	

	// KTRANSTEXTURE
    else if ( FStrEq(pcmd, "ktranstext" ) )
    {
        
	
		r_render = 6;


    }

    // End -  KTRANSTEXTURE





		// KRENDERNONE
    else if ( FStrEq(pcmd, "krendernone" ) )
    {
        
	
		r_renderfx = 1;


    }

    // End -  KRENDERNONE






			// KRENDERPULSESLOW
    else if ( FStrEq(pcmd, "krenderpulseslow" ) )
    {
        
	
		r_renderfx = 2;


    }

    // End -  KRENDERPULSESLOW






	// krenderpulsefast
    else if ( FStrEq(pcmd, "krenderpulsefast" ) )
    {
        
	
		r_renderfx = 3;


    }

    // End -  krenderpulsefast





	// krenderpulseslowwide
    else if ( FStrEq(pcmd, "krenderpulseslowwide" ) )
    {
        
	
		r_renderfx = 4;


    }

    // End -  krenderpulseslowwide





		// kRenderFxPulseFastWide
    else if ( FStrEq(pcmd, "krenderpulsefastwide" ) )
    {
        
	
		r_renderfx = 5;


    }

    // End -  kRenderFxPulseFastWide






			// krenderfadeslow
    else if ( FStrEq(pcmd, "krenderfadeslow" ) )
    {
        
	
		r_renderfx = 6;


    }

    // End -  krenderfadeslow





		// kRenderFxFadeFast
    else if ( FStrEq(pcmd, "krenderfadefast" ) )
    {
        
	
		r_renderfx = 7;


    }

    // End -  kRenderFxFadeFast





	
		// kRenderFxSolidSlow
    else if ( FStrEq(pcmd, "krendersolidslow" ) )
    {
        
	
		r_renderfx = 8;


    }

    // End -  kRenderFxSolidSlow








			// kRenderFxSolidFast
    else if ( FStrEq(pcmd, "krendersolidfast" ) )
    {
        
	
		r_renderfx = 9;


    }

    // End -  kRenderFxSolidFast






	// kRenderFxStrobeSlow
    else if ( FStrEq(pcmd, "krenderstrobeslow" ) )
    {
        
	
		r_renderfx = 10;


    }

    // End -  kRenderFxStrobeSlow







		// kRenderFxStrobeFast
    else if ( FStrEq(pcmd, "krenderstrobefast" ) )
    {
        
	
		r_renderfx = 11;


    }

    // End -  kRenderFxStrobeFast





			// kRenderFxFlickerSlow
    else if ( FStrEq(pcmd, "krenderflickerslow" ) )
    {
        
	
		r_renderfx = 12;


    }

    // End -  kRenderFxFlickerSlow






	// kRenderFxFlickerFast
    else if ( FStrEq(pcmd, "krenderflickerfast" ) )
    {
        
	
		r_renderfx = 13;


    }

    // End -  kRenderFxFlickerFast






	// kRenderFxNoDissipation
    else if ( FStrEq(pcmd, "krendernodissipation" ) )
    {
        
	
		r_renderfx = 14;


    }

    // End -  kRenderFxNoDissipation





	
	// kRenderFxDistort
    else if ( FStrEq(pcmd, "krenderdistort" ) )
    {
        
	
		r_renderfx = 15;


    }

    // End -  kRenderFxDistort






	// kRenderFxHologram
    else if ( FStrEq(pcmd, "krenderhologram" ) )
    {
        
	
		r_renderfx = 16;


    }

    // End -  kRenderFxHologram




		// kRenderFxExplode
    else if ( FStrEq(pcmd, "krenderexplode" ) )
    {
        
	
		r_renderfx = 17;


    }

    // End -  kRenderFxExplode





	
		// kRenderFxGlowShell
    else if ( FStrEq(pcmd, "krenderglowshell" ) )
    {
        
	
		r_renderfx = 18;


    }

    // End -  kRenderFxGlowShell




	// SOLID UP
    else if ( FStrEq(pcmd, "solid_up" ) )
    {

		r_solid_pos = 1;
    }

    // End - SOLID UP




	// SOLID DOWN
    else if ( FStrEq(pcmd, "solid_down" ) )
    {

		r_solid_pos = 2;
    }

    // End - SOLID DOWN



	
	// SOLID LEFT
    else if ( FStrEq(pcmd, "solid_left" ) )
    {

		r_solid_pos = 3;
    }

    // End - SOLID LEFT





	// SOLID RIGHT
    else if ( FStrEq(pcmd, "solid_right" ) )
    {

		r_solid_pos = 4;
    }

    // End - SOLID RIGHT






	
	// SOLID ROTATE X +
    else if ( FStrEq(pcmd, "solid_rotate_x_+" ) )
    {

		CBaseEntity *pEntity;
		CBasePlayer * pPlayer = GetClassPtr((CBasePlayer *)pev);

		pEntity = FindEntityForwardClient( pPlayer );

		if ( pEntity )
		{
			pEntity->pev->angles.x = pEntity->pev->angles.x + 1;
		}
    }

    // End - SOLID ROTATE X +





		// SOLID ROTATE X -
    else if ( FStrEq(pcmd, "solid_rotate_x_-" ) )
    {

		CBaseEntity *pEntity;
		CBasePlayer * pPlayer = GetClassPtr((CBasePlayer *)pev);

		pEntity = FindEntityForwardClient( pPlayer );
	
		if ( pEntity )
		{
			pEntity->pev->angles.x = pEntity->pev->angles.x - 1;
		}
    }

    // End - SOLID ROTATE X -



	// SOLID ROTATE Y +
    else if ( FStrEq(pcmd, "solid_rotate_y_+" ) )
    {

		CBaseEntity *pEntity;
		CBasePlayer * pPlayer = GetClassPtr((CBasePlayer *)pev);

		pEntity = FindEntityForwardClient( pPlayer );
	
		if ( pEntity )
		{
			pEntity->pev->angles.y = pEntity->pev->angles.y + 1;
		}
    }

    // End - SOLID ROTATE Y +






		// SOLID ROTATE Y -
    else if ( FStrEq(pcmd, "solid_rotate_y_-" ) )
    {

		CBaseEntity *pEntity;
		CBasePlayer * pPlayer = GetClassPtr((CBasePlayer *)pev);

		pEntity = FindEntityForwardClient( pPlayer );
	
		if ( pEntity )
		{
	
			pEntity->pev->angles.y = pEntity->pev->angles.y - 1;
		}
    }

    // End - SOLID ROTATE Y -







		// SOLID ROTATE Z +
    else if ( FStrEq(pcmd, "solid_rotate_z_+" ) )
    {

		CBaseEntity *pEntity;
		CBasePlayer * pPlayer = GetClassPtr((CBasePlayer *)pev);

		pEntity = FindEntityForwardClient( pPlayer );
	
		if ( pEntity )
		{
		
			pEntity->pev->angles.z = pEntity->pev->angles.z + 1;
		}
    }

    // End - SOLID ROTATE Z +






		// SOLID ROTATE Z -
    else if ( FStrEq(pcmd, "solid_rotate_z_-" ) )
    {

		CBaseEntity *pEntity;
		CBasePlayer * pPlayer = GetClassPtr((CBasePlayer *)pev);

		pEntity = FindEntityForwardClient( pPlayer );
	
		if ( pEntity )
		{
	
			pEntity->pev->angles.z = pEntity->pev->angles.z - 1;
		}
    }

    // End - SOLID ROTATE Z -





		
	//  DuplicateMode
    else if ( FStrEq(pcmd, "duplicatemode" ) )
    {
        
		r_duplicatemode = 1;
		r_removemode = 0;
		r_rendermode = 0;

    }

    // End - DuplicateMode 






	//  RemoveMode
    else if ( FStrEq(pcmd, "removemode" ) )
    {
        
		r_duplicatemode = 0;
		r_removemode = 1;
		r_rendermode = 0;

    }

    // End - RemoveMode 





	//  RenderMode
    else if ( FStrEq(pcmd, "rendermode" ) )
    {
        
		r_duplicatemode = 0;
		r_removemode = 0;
		r_rendermode = 1;

    }

    // End - RenderMode 


	// Ally monster
	 else if ( FStrEq(pcmd, "allied" ) )
	 {
		 m_allied = 1;
	 }


	 	
	 else if ( FStrEq(pcmd, "allied_no" ) )
	 {
		 m_allied = 0;
	 }

		//BP particle system reiniting
	else if ( FStrEq(pcmd, "reinit_particles" ) )
	{
		CBasePlayer *pPlayer;
		pPlayer = GetClassPtr((CBasePlayer *)pev);
	
		// clear all particlesystems with this hijacked message
		extern int gmsgParticles;
		MESSAGE_BEGIN(MSG_ONE, gmsgParticles, NULL, pPlayer->pev);
			WRITE_SHORT(0);
			WRITE_BYTE(0);
			WRITE_COORD(0);
			WRITE_COORD(0);
			WRITE_COORD(0);
			WRITE_COORD(0);
			WRITE_COORD(0);
			WRITE_COORD(0);
			WRITE_SHORT(9999);
			WRITE_STRING("");
		MESSAGE_END();

	//	pPlayer->m_bSpawnPS = true;
		pPlayer->m_bSpawnGrass = true;
	//	pPlayer->m_flLastPSSpawn = 0.0;
		pPlayer->m_flLastGrassSpawn = 0.0;
		pPlayer->pLastGrassSpawned = NULL;
		pPlayer->pLastPSSpawned = NULL;
	}

	
	else if ( FStrEq( pcmd, "spectate" ) && (pev->flags & FL_PROXY) )	// added for proxy support
	{
		CBasePlayer * pPlayer = GetClassPtr((CBasePlayer *)pev);

		edict_t *pentSpawnSpot = g_pGameRules->GetPlayerSpawnSpot( pPlayer );
		pPlayer->StartObserver( pev->origin, VARS(pentSpawnSpot)->angles);
	}




	else if ( g_pGameRules->ClientCommand( GetClassPtr((CBasePlayer *)pev), pcmd ) )
	{
		// MenuSelect returns true only if the command is properly handled,  so don't print a warning
	}
	else
	{
		// tell the user they entered an unknown command
		char command[128];

		// check the length of the command (prevents crash)
		// max total length is 192 ...and we're adding a string below ("Unknown command: %s\n")
		strncpy( command, pcmd, 127 );
		command[127] = '\0';

		// tell the user they entered an unknown command
		ClientPrint( &pEntity->v, HUD_PRINTCONSOLE, UTIL_VarArgs( "Unknown command: %s\n", command ) );
	}
}




/*
========================
ClientUserInfoChanged

called after the player changes
userinfo - gives dll a chance to modify it before
it gets sent into the rest of the engine.
========================
*/
void ClientUserInfoChanged( edict_t *pEntity, char *infobuffer )
{
	// Is the client spawned yet?
	if ( !pEntity->pvPrivateData )
		return;

	// msg everyone if someone changes their name,  and it isn't the first time (changing no name to current name)
	if ( pEntity->v.netname && STRING(pEntity->v.netname)[0] != 0 && !FStrEq( STRING(pEntity->v.netname), g_engfuncs.pfnInfoKeyValue( infobuffer, "name" )) )
	{
		char sName[256];
		char *pName = g_engfuncs.pfnInfoKeyValue( infobuffer, "name" );
		strncpy( sName, pName, sizeof(sName) - 1 );
		sName[ sizeof(sName) - 1 ] = '\0';

		// First parse the name and remove any %'s
		for ( char *pApersand = sName; pApersand != NULL && *pApersand != 0; pApersand++ )
		{
			// Replace it with a space
			if ( *pApersand == '%' )
				*pApersand = ' ';
		}

		// Set the name
		g_engfuncs.pfnSetClientKeyValue( ENTINDEX(pEntity), infobuffer, "name", sName );

		char text[256];
		sprintf( text, "* %s changed name to %s\n", STRING(pEntity->v.netname), g_engfuncs.pfnInfoKeyValue( infobuffer, "name" ) );
		MESSAGE_BEGIN( MSG_ALL, gmsgSayText, NULL );
			WRITE_BYTE( ENTINDEX(pEntity) );
			WRITE_STRING( text );
		MESSAGE_END();

		// team match?
		if ( g_teamplay )
		{
			UTIL_LogPrintf( "\"%s<%i><%s><%s>\" changed name to \"%s\"\n", 
				STRING( pEntity->v.netname ), 
				GETPLAYERUSERID( pEntity ), 
				GETPLAYERAUTHID( pEntity ),
				g_engfuncs.pfnInfoKeyValue( infobuffer, "model" ), 
				g_engfuncs.pfnInfoKeyValue( infobuffer, "name" ) );
		}
		else
		{
			UTIL_LogPrintf( "\"%s<%i><%s><%i>\" changed name to \"%s\"\n", 
				STRING( pEntity->v.netname ), 
				GETPLAYERUSERID( pEntity ), 
				GETPLAYERAUTHID( pEntity ),
				GETPLAYERUSERID( pEntity ), 
				g_engfuncs.pfnInfoKeyValue( infobuffer, "name" ) );
		}
	}

	g_pGameRules->ClientUserInfoChanged( GetClassPtr((CBasePlayer *)&pEntity->v), infobuffer );
}

static int g_serveractive = 0;

void ServerDeactivate( void )
{
	// It's possible that the engine will call this function more times than is necessary
	//  Therefore, only run it one time for each call to ServerActivate 
	if ( g_serveractive != 1 )
	{
		return;
	}

	g_serveractive = 0;

	// Peform any shutdown operations here...
	//
}

void ServerActivate( edict_t *pEdictList, int edictCount, int clientMax )
{
	int				i;
	CBaseEntity		*pClass;

	// Every call to ServerActivate should be matched by a call to ServerDeactivate
	g_serveractive = 1;

	// Clients have not been initialized yet
	for ( i = 0; i < edictCount; i++ )
	{
		if ( pEdictList[i].free )
			continue;
		
		// Clients aren't necessarily initialized until ClientPutInServer()
		if ( i < clientMax || !pEdictList[i].pvPrivateData )
			continue;

		pClass = CBaseEntity::Instance( &pEdictList[i] );
		// Activate this entity if it's got a class & isn't dormant
		if ( pClass && !(pClass->pev->flags & FL_DORMANT) )
		{
			pClass->Activate();
		}
		else
		{
			ALERT( at_console, "Can't instance %s\n", STRING(pEdictList[i].v.classname) );
		}
	}

	// Link user messages here to make sure first client can get them...
	LinkUserMessages();
}


/*
================
PlayerPreThink

Called every frame before physics are run
================
*/
void PlayerPreThink( edict_t *pEntity )
{
	entvars_t *pev = &pEntity->v;
	CBasePlayer *pPlayer = (CBasePlayer *)GET_PRIVATE(pEntity);

	if (pPlayer)
		pPlayer->PreThink( );
}

/*
================
PlayerPostThink

Called every frame after physics are run
================
*/
void PlayerPostThink( edict_t *pEntity )
{
	entvars_t *pev = &pEntity->v;
	CBasePlayer *pPlayer = (CBasePlayer *)GET_PRIVATE(pEntity);

	if (pPlayer)
		pPlayer->PostThink( );
}



void ParmsNewLevel( void )
{
}


void ParmsChangeLevel( void )
{
	// retrieve the pointer to the save data
	SAVERESTOREDATA *pSaveData = (SAVERESTOREDATA *)gpGlobals->pSaveData;

	if ( pSaveData )
		pSaveData->connectionCount = BuildChangeList( pSaveData->levelList, MAX_LEVEL_CONNECTIONS );
}


//
// GLOBALS ASSUMED SET:  g_ulFrameCount
//
void StartFrame( void )
{
	if ( g_pGameRules )
		g_pGameRules->Think();

	if ( g_fGameOver )
		return;

	gpGlobals->teamplay = teamplay.value;
	g_ulFrameCount++;
}


void ClientPrecache( void )
{
	// setup precaches always needed
	PRECACHE_SOUND("player/sprayer.wav");			// spray paint sound for PreAlpha
	
	// PRECACHE_SOUND("player/pl_jumpland2.wav");		// UNDONE: play 2x step sound
	
	PRECACHE_SOUND("player/pl_fallpain2.wav");		
	PRECACHE_SOUND("player/pl_fallpain3.wav");		
	
	PRECACHE_SOUND("player/pl_step1.wav");		// walk on concrete
	PRECACHE_SOUND("player/pl_step2.wav");
	PRECACHE_SOUND("player/pl_step3.wav");
	PRECACHE_SOUND("player/pl_step4.wav");

	PRECACHE_SOUND("common/npc_step1.wav");		// NPC walk on concrete
	PRECACHE_SOUND("common/npc_step2.wav");
	PRECACHE_SOUND("common/npc_step3.wav");
	PRECACHE_SOUND("common/npc_step4.wav");

	PRECACHE_SOUND("player/pl_metal1.wav");		// walk on metal
	PRECACHE_SOUND("player/pl_metal2.wav");
	PRECACHE_SOUND("player/pl_metal3.wav");
	PRECACHE_SOUND("player/pl_metal4.wav");

	PRECACHE_SOUND("player/pl_dirt1.wav");		// walk on dirt
	PRECACHE_SOUND("player/pl_dirt2.wav");
	PRECACHE_SOUND("player/pl_dirt3.wav");
	PRECACHE_SOUND("player/pl_dirt4.wav");

	PRECACHE_SOUND("player/pl_duct1.wav");		// walk in duct
	PRECACHE_SOUND("player/pl_duct2.wav");
	PRECACHE_SOUND("player/pl_duct3.wav");
	PRECACHE_SOUND("player/pl_duct4.wav");

	PRECACHE_SOUND("player/pl_grate1.wav");		// walk on grate
	PRECACHE_SOUND("player/pl_grate2.wav");
	PRECACHE_SOUND("player/pl_grate3.wav");
	PRECACHE_SOUND("player/pl_grate4.wav");

	PRECACHE_SOUND("player/pl_slosh1.wav");		// walk in shallow water
	PRECACHE_SOUND("player/pl_slosh2.wav");
	PRECACHE_SOUND("player/pl_slosh3.wav");
	PRECACHE_SOUND("player/pl_slosh4.wav");

	PRECACHE_SOUND("player/pl_tile1.wav");		// walk on tile
	PRECACHE_SOUND("player/pl_tile2.wav");
	PRECACHE_SOUND("player/pl_tile3.wav");
	PRECACHE_SOUND("player/pl_tile4.wav");
	PRECACHE_SOUND("player/pl_tile5.wav");

	PRECACHE_SOUND("player/pl_swim1.wav");		// breathe bubbles
	PRECACHE_SOUND("player/pl_swim2.wav");
	PRECACHE_SOUND("player/pl_swim3.wav");
	PRECACHE_SOUND("player/pl_swim4.wav");

	PRECACHE_SOUND("player/pl_ladder1.wav");	// climb ladder rung
	PRECACHE_SOUND("player/pl_ladder2.wav");
	PRECACHE_SOUND("player/pl_ladder3.wav");
	PRECACHE_SOUND("player/pl_ladder4.wav");

	PRECACHE_SOUND("player/pl_wade1.wav");		// wade in water
	PRECACHE_SOUND("player/pl_wade2.wav");
	PRECACHE_SOUND("player/pl_wade3.wav");
	PRECACHE_SOUND("player/pl_wade4.wav");

	PRECACHE_SOUND("debris/wood1.wav");			// hit wood texture
	PRECACHE_SOUND("debris/wood2.wav");
	PRECACHE_SOUND("debris/wood3.wav");

	PRECACHE_SOUND("plats/train_use1.wav");		// use a train

	PRECACHE_SOUND("buttons/spark5.wav");		// hit computer texture
	PRECACHE_SOUND("buttons/spark6.wav");
	PRECACHE_SOUND("debris/glass1.wav");
	PRECACHE_SOUND("debris/glass2.wav");
	PRECACHE_SOUND("debris/glass3.wav");

	PRECACHE_SOUND( SOUND_FLASHLIGHT_ON );
	PRECACHE_SOUND( SOUND_FLASHLIGHT_OFF );

// player gib sounds
	PRECACHE_SOUND("common/bodysplat.wav");	
	PRECACHE_SOUND("button.wav");

// player pain sounds
	PRECACHE_SOUND("player/pl_pain2.wav");
	PRECACHE_SOUND("player/pl_pain4.wav");
	PRECACHE_SOUND("player/pl_pain5.wav");
	PRECACHE_SOUND("player/pl_pain6.wav");
	PRECACHE_SOUND("player/pl_pain7.wav");

	PRECACHE_MODEL("models/player.mdl");

	// hud sounds

	PRECACHE_SOUND("common/wpn_hudoff.wav");
	PRECACHE_SOUND("common/wpn_hudon.wav");
	PRECACHE_SOUND("common/wpn_moveselect.wav");
	PRECACHE_SOUND("common/wpn_select.wav");
	PRECACHE_SOUND("common/wpn_denyselect.wav");


	// geiger sounds

	PRECACHE_SOUND("player/geiger6.wav");
	PRECACHE_SOUND("player/geiger5.wav");
	PRECACHE_SOUND("player/geiger4.wav");
	PRECACHE_SOUND("player/geiger3.wav");
	PRECACHE_SOUND("player/geiger2.wav");
	PRECACHE_SOUND("player/geiger1.wav");
	PRECACHE_MODEL("models/bengala.mdl");
	PRECACHE_MODEL("models/flare.mdl");

	PRECACHE_MODEL("sprites/flare3.spr");
	PRECACHE_MODEL("sprites/delete.spr");
	PRECACHE_MODEL("sprites/slowbar.spr");
	PRECACHE_MODEL("sprites/beta.spr");
	// Todos los monsters disponibles 

	UTIL_PrecacheOther("monster_human_grunt");
	UTIL_PrecacheOther("monster_human_assassin");
	UTIL_PrecacheOther("monster_barney");
	UTIL_PrecacheOther("monster_zombie");
	UTIL_PrecacheOther("monster_headcrab");
	UTIL_PrecacheOther("monster_alien_grunt");
	UTIL_PrecacheOther("monster_alien_slave");
	UTIL_PrecacheOther("monster_gargantua");
	UTIL_PrecacheOther("monster_bloater");
	UTIL_PrecacheOther("monster_cockroach");
	UTIL_PrecacheOther("monster_alien_controller");
	UTIL_PrecacheOther("monster_flyer_flock");
	UTIL_PrecacheOther("monster_gman");
	UTIL_PrecacheOther("monster_leech");
	UTIL_PrecacheOther("monster_nihilanth");
	UTIL_PrecacheOther("monster_osprey");
	UTIL_PrecacheOther("monster_tentacle");
	UTIL_PrecacheOther("monster_turret");
	UTIL_PrecacheOther("monster_bullchicken");
	UTIL_PrecacheOther("monster_apache");
	UTIL_PrecacheOther("monster_rat");
	UTIL_PrecacheOther("monster_sentry");
	UTIL_PrecacheOther("monster_houndeye");
	UTIL_PrecacheOther("monster_barnacle");
	UTIL_PrecacheOther("monster_babycrab");
	UTIL_PrecacheOther("monster_scientist");
	UTIL_PrecacheOther("monster_bigmomma");
	UTIL_PrecacheOther("monster_hl2headcrab");
	UTIL_PrecacheOther("monster_fast_zombie");
	UTIL_PrecacheOther("monster_fast_headcrab");
	UTIL_PrecacheOther("monster_black_headcrab");
	UTIL_PrecacheOther("monster_gonome");
	UTIL_PrecacheOther("monster_human_massn");
	UTIL_PrecacheOther("monster_shock_trooper");
	UTIL_PrecacheOther("xen_plantlight");
	UTIL_PrecacheOther("env_spark");
	UTIL_PrecacheOther("func_tank");
	UTIL_PrecacheOther("wall2x2");
	UTIL_PrecacheOther("ballon");
	UTIL_PrecacheOther("monster_zombie_hl2");
	UTIL_PrecacheOther("monster_pit_drone");
	UTIL_PrecacheOther("monster_otis");
	UTIL_PrecacheOther("monster_sentry_ally");
	UTIL_PrecacheOther("monster_voltigore");
	UTIL_PrecacheOther("automonstermaker");
	UTIL_PrecacheOther("info_portal_destination");
	UTIL_PrecacheOther("monster_ichthyosaur");
	UTIL_PrecacheOther("monster_tripod");
	UTIL_PrecacheOther("dynamite");
	UTIL_PrecacheOther("barrel");


}

/*
===============
GetGameDescription

Returns the descriptive name of this .dll.  E.g., Half-Life, or Team Fortress 2
===============
*/
const char *GetGameDescription()
{
	if ( g_pGameRules ) // this function may be called before the world has spawned, and the game rules initialized
		return g_pGameRules->GetGameDescription();
	else
		return "Half-Life";
}

/*
================
Sys_Error

Engine is going to shut down, allows setting a breakpoint in game .dll to catch that occasion
================
*/
void Sys_Error( const char *error_string )
{
	// Default case, do nothing.  MOD AUTHORS:  Add code ( e.g., _asm { int 3 }; here to cause a breakpoint for debugging your game .dlls
}

/*
================
PlayerCustomization

A new player customization has been registered on the server
UNDONE:  This only sets the # of frames of the spray can logo
animation right now.
================
*/
void PlayerCustomization( edict_t *pEntity, customization_t *pCust )
{
	entvars_t *pev = &pEntity->v;
	CBasePlayer *pPlayer = (CBasePlayer *)GET_PRIVATE(pEntity);

	if (!pPlayer)
	{
		ALERT(at_console, "PlayerCustomization:  Couldn't get player!\n");
		return;
	}

	if (!pCust)
	{
		ALERT(at_console, "PlayerCustomization:  NULL customization!\n");
		return;
	}

	switch (pCust->resource.type)
	{
	case t_decal:
		pPlayer->SetCustomDecalFrames(pCust->nUserData2); // Second int is max # of frames.
		break;
	case t_sound:
	case t_skin:
	case t_model:
		// Ignore for now.
		break;
	default:
		ALERT(at_console, "PlayerCustomization:  Unknown customization type!\n");
		break;
	}
}

/*
================
SpectatorConnect

A spectator has joined the game
================
*/
void SpectatorConnect( edict_t *pEntity )
{
	entvars_t *pev = &pEntity->v;
	CBaseSpectator *pPlayer = (CBaseSpectator *)GET_PRIVATE(pEntity);

	if (pPlayer)
		pPlayer->SpectatorConnect( );
}

/*
================
SpectatorConnect

A spectator has left the game
================
*/
void SpectatorDisconnect( edict_t *pEntity )
{
	entvars_t *pev = &pEntity->v;
	CBaseSpectator *pPlayer = (CBaseSpectator *)GET_PRIVATE(pEntity);

	if (pPlayer)
		pPlayer->SpectatorDisconnect( );
}

/*
================
SpectatorConnect

A spectator has sent a usercmd
================
*/
void SpectatorThink( edict_t *pEntity )
{
	entvars_t *pev = &pEntity->v;
	CBaseSpectator *pPlayer = (CBaseSpectator *)GET_PRIVATE(pEntity);

	if (pPlayer)
		pPlayer->SpectatorThink( );
}

////////////////////////////////////////////////////////
// PAS and PVS routines for client messaging
//

/*
================
SetupVisibility

A client can have a separate "view entity" indicating that his/her view should depend on the origin of that
view entity.  If that's the case, then pViewEntity will be non-NULL and will be used.  Otherwise, the current
entity's origin is used.  Either is offset by the view_ofs to get the eye position.

From the eye position, we set up the PAS and PVS to use for filtering network messages to the client.  At this point, we could
 override the actual PAS or PVS values, or use a different origin.

NOTE:  Do not cache the values of pas and pvs, as they depend on reusable memory in the engine, they are only good for this one frame
================
*/
void SetupVisibility( edict_t *pViewEntity, edict_t *pClient, unsigned char **pvs, unsigned char **pas )
{
	Vector org;
	edict_t *pView = pClient;

	// Find the client's PVS
	if ( pViewEntity )
	{
		pView = pViewEntity;
	}

	if ( pClient->v.flags & FL_PROXY )
	{
		*pvs = NULL;	// the spectator proxy sees
		*pas = NULL;	// and hears everything
		return;
	}

	org = pView->v.origin + pView->v.view_ofs;
	if ( pView->v.flags & FL_DUCKING )
	{
		org = org + ( VEC_HULL_MIN - VEC_DUCK_HULL_MIN );
	}

	*pvs = ENGINE_SET_PVS ( (float *)&org );
	*pas = ENGINE_SET_PAS ( (float *)&org );
}

#include "entity_state.h"

/*
AddToFullPack

Return 1 if the entity state has been filled in for the ent and the entity will be propagated to the client, 0 otherwise

state is the server maintained copy of the state info that is transmitted to the client
a MOD could alter values copied into state to send the "host" a different look for a particular entity update, etc.
e and ent are the entity that is being added to the update, if 1 is returned
host is the player's edict of the player whom we are sending the update to
player is 1 if the ent/e is a player and 0 otherwise
pSet is either the PAS or PVS that we previous set up.  We can use it to ask the engine to filter the entity against the PAS or PVS.
we could also use the pas/ pvs that we set in SetupVisibility, if we wanted to.  Caching the value is valid in that case, but still only for the current frame
*/
int AddToFullPack( struct entity_state_s *state, int e, edict_t *ent, edict_t *host, int hostflags, int player, unsigned char *pSet )
{
	int					i;

	// don't send if flagged for NODRAW and it's not the host getting the message
	if ( ( ent->v.effects == EF_NODRAW ) &&
		 ( ent != host ) )
		return 0;

	// Ignore ents without valid / visible models
	if ( !ent->v.modelindex || !STRING( ent->v.model ) )
		return 0;

	// Don't send spectators to other players
	if ( ( ent->v.flags & FL_SPECTATOR ) && ( ent != host ) )
	{
		return 0;
	}

	// Ignore if not the host and not touching a PVS/PAS leaf
	// If pSet is NULL, then the test will always succeed and the entity will be added to the update
	if ( ent != host )
	{
		if ( !ENGINE_CHECK_VISIBILITY( (const struct edict_s *)ent, pSet ) )
		{
			return 0;
		}
	}


	// Don't send entity to local client if the client says it's predicting the entity itself.
	if ( ent->v.flags & FL_SKIPLOCALHOST )
	{
		if ( ( hostflags & 1 ) && ( ent->v.owner == host ) )
			return 0;
	}
	
	if ( host->v.groupinfo )
	{
		UTIL_SetGroupTrace( host->v.groupinfo, GROUP_OP_AND );

		// Should always be set, of course
		if ( ent->v.groupinfo )
		{
			if ( g_groupop == GROUP_OP_AND )
			{
				if ( !(ent->v.groupinfo & host->v.groupinfo ) )
					return 0;
			}
			else if ( g_groupop == GROUP_OP_NAND )
			{
				if ( ent->v.groupinfo & host->v.groupinfo )
					return 0;
			}
		}

		UTIL_UnsetGroupTrace();
	}

	memset( state, 0, sizeof( *state ) );

	// Assign index so we can track this entity from frame to frame and
	//  delta from it.
	state->number	  = e;
	state->entityType = ENTITY_NORMAL;
	
	// Flag custom entities.
	if ( ent->v.flags & FL_CUSTOMENTITY )
	{
		state->entityType = ENTITY_BEAM;
	}

	// 
	// Copy state data
	//

	// Round animtime to nearest millisecond
	state->animtime   = (int)(1000.0 * ent->v.animtime ) / 1000.0;

	memcpy( state->origin, ent->v.origin, 3 * sizeof( float ) );
	memcpy( state->angles, ent->v.angles, 3 * sizeof( float ) );
	memcpy( state->mins, ent->v.mins, 3 * sizeof( float ) );
	memcpy( state->maxs, ent->v.maxs, 3 * sizeof( float ) );

	memcpy( state->startpos, ent->v.startpos, 3 * sizeof( float ) );
	memcpy( state->endpos, ent->v.endpos, 3 * sizeof( float ) );

	state->impacttime = ent->v.impacttime;
	state->starttime = ent->v.starttime;

	state->modelindex = ent->v.modelindex;
		
	state->frame      = ent->v.frame;

	state->skin       = ent->v.skin;
	state->effects    = ent->v.effects;

	// This non-player entity is being moved by the game .dll and not the physics simulation system
	//  make sure that we interpolate it's position on the client if it moves
	if ( !player &&
		 ent->v.animtime &&
		 ent->v.velocity[ 0 ] == 0 && 
		 ent->v.velocity[ 1 ] == 0 && 
		 ent->v.velocity[ 2 ] == 0 )
	{
		state->eflags |= EFLAG_SLERP;
	}

	state->scale	  = ent->v.scale;
	state->solid	  = ent->v.solid;
	state->colormap   = ent->v.colormap;

	state->movetype   = ent->v.movetype;
	state->sequence   = ent->v.sequence;
	state->framerate  = ent->v.framerate;
	state->body       = ent->v.body;

	for (i = 0; i < 4; i++)
	{
		state->controller[i] = ent->v.controller[i];
	}

	for (i = 0; i < 2; i++)
	{
		state->blending[i]   = ent->v.blending[i];
	}

	state->rendermode    = ent->v.rendermode;
	state->renderamt     = ent->v.renderamt; 
	state->renderfx      = ent->v.renderfx;
	state->rendercolor.r = ent->v.rendercolor.x;
	state->rendercolor.g = ent->v.rendercolor.y;
	state->rendercolor.b = ent->v.rendercolor.z;

	state->aiment = 0;
	if ( ent->v.aiment )
	{
		state->aiment = ENTINDEX( ent->v.aiment );
	}

	state->owner = 0;
	if ( ent->v.owner )
	{
		int owner = ENTINDEX( ent->v.owner );
		
		// Only care if owned by a player
		if ( owner >= 1 && owner <= gpGlobals->maxClients )
		{
			state->owner = owner;	
		}
	}

	// HACK:  Somewhat...
	// Class is overridden for non-players to signify a breakable glass object ( sort of a class? )
	if ( !player )
	{
		state->playerclass  = ent->v.playerclass;
	}

	// Special stuff for players only
	if ( player )
	{
		memcpy( state->basevelocity, ent->v.basevelocity, 3 * sizeof( float ) );

		state->weaponmodel  = MODEL_INDEX( STRING( ent->v.weaponmodel ) );
		state->gaitsequence = ent->v.gaitsequence;
		state->spectator = ent->v.flags & FL_SPECTATOR;
		state->friction     = ent->v.friction;

		state->gravity      = ent->v.gravity;
//		state->team			= ent->v.team;
//		
		state->usehull      = ( ent->v.flags & FL_DUCKING ) ? 1 : 0;
		state->health		= ent->v.health;
	}

	return 1;
}

// defaults for clientinfo messages
#define	DEFAULT_VIEWHEIGHT	28

/*
===================
CreateBaseline

Creates baselines used for network encoding, especially for player data since players are not spawned until connect time.
===================
*/
void CreateBaseline( int player, int eindex, struct entity_state_s *baseline, struct edict_s *entity, int playermodelindex, vec3_t player_mins, vec3_t player_maxs )
{
	baseline->origin		= entity->v.origin;
	baseline->angles		= entity->v.angles;
	baseline->frame			= entity->v.frame;
	baseline->skin			= (short)entity->v.skin;

	// render information
	baseline->rendermode	= (byte)entity->v.rendermode;
	baseline->renderamt		= (byte)entity->v.renderamt;
	baseline->rendercolor.r	= (byte)entity->v.rendercolor.x;
	baseline->rendercolor.g	= (byte)entity->v.rendercolor.y;
	baseline->rendercolor.b	= (byte)entity->v.rendercolor.z;
	baseline->renderfx		= (byte)entity->v.renderfx;

	if ( player )
	{
		baseline->mins			= player_mins;
		baseline->maxs			= player_maxs;

		baseline->colormap		= eindex;
		baseline->modelindex	= playermodelindex;
		baseline->friction		= 1.0;
		baseline->movetype		= MOVETYPE_WALK;

		baseline->scale			= entity->v.scale;
		baseline->solid			= SOLID_SLIDEBOX;
		baseline->framerate		= 1.0;
		baseline->gravity		= 1.0;

	}
	else
	{
		baseline->mins			= entity->v.mins;
		baseline->maxs			= entity->v.maxs;

		baseline->colormap		= 0;
		baseline->modelindex	= entity->v.modelindex;//SV_ModelIndex(pr_strings + entity->v.model);
		baseline->movetype		= entity->v.movetype;

		baseline->scale			= entity->v.scale;
		baseline->solid			= entity->v.solid;
		baseline->framerate		= entity->v.framerate;
		baseline->gravity		= entity->v.gravity;
	}
}

typedef struct
{
	char name[32];
	int	 field;
} entity_field_alias_t;

#define FIELD_ORIGIN0			0
#define FIELD_ORIGIN1			1
#define FIELD_ORIGIN2			2
#define FIELD_ANGLES0			3
#define FIELD_ANGLES1			4
#define FIELD_ANGLES2			5

static entity_field_alias_t entity_field_alias[]=
{
	{ "origin[0]",			0 },
	{ "origin[1]",			0 },
	{ "origin[2]",			0 },
	{ "angles[0]",			0 },
	{ "angles[1]",			0 },
	{ "angles[2]",			0 },
};

void Entity_FieldInit( struct delta_s *pFields )
{
	entity_field_alias[ FIELD_ORIGIN0 ].field		= DELTA_FINDFIELD( pFields, entity_field_alias[ FIELD_ORIGIN0 ].name );
	entity_field_alias[ FIELD_ORIGIN1 ].field		= DELTA_FINDFIELD( pFields, entity_field_alias[ FIELD_ORIGIN1 ].name );
	entity_field_alias[ FIELD_ORIGIN2 ].field		= DELTA_FINDFIELD( pFields, entity_field_alias[ FIELD_ORIGIN2 ].name );
	entity_field_alias[ FIELD_ANGLES0 ].field		= DELTA_FINDFIELD( pFields, entity_field_alias[ FIELD_ANGLES0 ].name );
	entity_field_alias[ FIELD_ANGLES1 ].field		= DELTA_FINDFIELD( pFields, entity_field_alias[ FIELD_ANGLES1 ].name );
	entity_field_alias[ FIELD_ANGLES2 ].field		= DELTA_FINDFIELD( pFields, entity_field_alias[ FIELD_ANGLES2 ].name );
}

/*
==================
Entity_Encode

Callback for sending entity_state_t info over network. 
FIXME:  Move to script
==================
*/
void Entity_Encode( struct delta_s *pFields, const unsigned char *from, const unsigned char *to )
{
	entity_state_t *f, *t;
	int localplayer = 0;
	static int initialized = 0;

	if ( !initialized )
	{
		Entity_FieldInit( pFields );
		initialized = 1;
	}

	f = (entity_state_t *)from;
	t = (entity_state_t *)to;

	// Never send origin to local player, it's sent with more resolution in clientdata_t structure
	localplayer =  ( t->number - 1 ) == ENGINE_CURRENT_PLAYER();
	if ( localplayer )
	{
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN0 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN1 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN2 ].field );
	}

	if ( ( t->impacttime != 0 ) && ( t->starttime != 0 ) )
	{
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN0 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN1 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN2 ].field );

		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ANGLES0 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ANGLES1 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ANGLES2 ].field );
	}

	if ( ( t->movetype == MOVETYPE_FOLLOW ) &&
		 ( t->aiment != 0 ) )
	{
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN0 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN1 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN2 ].field );
	}
	else if ( t->aiment != f->aiment )
	{
		DELTA_SETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN0 ].field );
		DELTA_SETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN1 ].field );
		DELTA_SETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN2 ].field );
	}
}

static entity_field_alias_t player_field_alias[]=
{
	{ "origin[0]",			0 },
	{ "origin[1]",			0 },
	{ "origin[2]",			0 },
};

void Player_FieldInit( struct delta_s *pFields )
{
	player_field_alias[ FIELD_ORIGIN0 ].field		= DELTA_FINDFIELD( pFields, player_field_alias[ FIELD_ORIGIN0 ].name );
	player_field_alias[ FIELD_ORIGIN1 ].field		= DELTA_FINDFIELD( pFields, player_field_alias[ FIELD_ORIGIN1 ].name );
	player_field_alias[ FIELD_ORIGIN2 ].field		= DELTA_FINDFIELD( pFields, player_field_alias[ FIELD_ORIGIN2 ].name );
}

/*
==================
Player_Encode

Callback for sending entity_state_t for players info over network. 
==================
*/
void Player_Encode( struct delta_s *pFields, const unsigned char *from, const unsigned char *to )
{
	entity_state_t *f, *t;
	int localplayer = 0;
	static int initialized = 0;

	if ( !initialized )
	{
		Player_FieldInit( pFields );
		initialized = 1;
	}

	f = (entity_state_t *)from;
	t = (entity_state_t *)to;

	// Never send origin to local player, it's sent with more resolution in clientdata_t structure
	localplayer =  ( t->number - 1 ) == ENGINE_CURRENT_PLAYER();
	if ( localplayer )
	{
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN0 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN1 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN2 ].field );
	}

	if ( ( t->movetype == MOVETYPE_FOLLOW ) &&
		 ( t->aiment != 0 ) )
	{
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN0 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN1 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN2 ].field );
	}
	else if ( t->aiment != f->aiment )
	{
		DELTA_SETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN0 ].field );
		DELTA_SETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN1 ].field );
		DELTA_SETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN2 ].field );
	}
}

#define CUSTOMFIELD_ORIGIN0			0
#define CUSTOMFIELD_ORIGIN1			1
#define CUSTOMFIELD_ORIGIN2			2
#define CUSTOMFIELD_ANGLES0			3
#define CUSTOMFIELD_ANGLES1			4
#define CUSTOMFIELD_ANGLES2			5
#define CUSTOMFIELD_SKIN			6
#define CUSTOMFIELD_SEQUENCE		7
#define CUSTOMFIELD_ANIMTIME		8

entity_field_alias_t custom_entity_field_alias[]=
{
	{ "origin[0]",			0 },
	{ "origin[1]",			0 },
	{ "origin[2]",			0 },
	{ "angles[0]",			0 },
	{ "angles[1]",			0 },
	{ "angles[2]",			0 },
	{ "skin",				0 },
	{ "sequence",			0 },
	{ "animtime",			0 },
};

void Custom_Entity_FieldInit( struct delta_s *pFields )
{
	custom_entity_field_alias[ CUSTOMFIELD_ORIGIN0 ].field	= DELTA_FINDFIELD( pFields, custom_entity_field_alias[ CUSTOMFIELD_ORIGIN0 ].name );
	custom_entity_field_alias[ CUSTOMFIELD_ORIGIN1 ].field	= DELTA_FINDFIELD( pFields, custom_entity_field_alias[ CUSTOMFIELD_ORIGIN1 ].name );
	custom_entity_field_alias[ CUSTOMFIELD_ORIGIN2 ].field	= DELTA_FINDFIELD( pFields, custom_entity_field_alias[ CUSTOMFIELD_ORIGIN2 ].name );
	custom_entity_field_alias[ CUSTOMFIELD_ANGLES0 ].field	= DELTA_FINDFIELD( pFields, custom_entity_field_alias[ CUSTOMFIELD_ANGLES0 ].name );
	custom_entity_field_alias[ CUSTOMFIELD_ANGLES1 ].field	= DELTA_FINDFIELD( pFields, custom_entity_field_alias[ CUSTOMFIELD_ANGLES1 ].name );
	custom_entity_field_alias[ CUSTOMFIELD_ANGLES2 ].field	= DELTA_FINDFIELD( pFields, custom_entity_field_alias[ CUSTOMFIELD_ANGLES2 ].name );
	custom_entity_field_alias[ CUSTOMFIELD_SKIN ].field	= DELTA_FINDFIELD( pFields, custom_entity_field_alias[ CUSTOMFIELD_SKIN ].name );
	custom_entity_field_alias[ CUSTOMFIELD_SEQUENCE ].field= DELTA_FINDFIELD( pFields, custom_entity_field_alias[ CUSTOMFIELD_SEQUENCE ].name );
	custom_entity_field_alias[ CUSTOMFIELD_ANIMTIME ].field= DELTA_FINDFIELD( pFields, custom_entity_field_alias[ CUSTOMFIELD_ANIMTIME ].name );
}

/*
==================
Custom_Encode

Callback for sending entity_state_t info ( for custom entities ) over network. 
FIXME:  Move to script
==================
*/
void Custom_Encode( struct delta_s *pFields, const unsigned char *from, const unsigned char *to )
{
	entity_state_t *f, *t;
	int beamType;
	static int initialized = 0;

	if ( !initialized )
	{
		Custom_Entity_FieldInit( pFields );
		initialized = 1;
	}

	f = (entity_state_t *)from;
	t = (entity_state_t *)to;

	beamType = t->rendermode & 0x0f;
		
	if ( beamType != BEAM_POINTS && beamType != BEAM_ENTPOINT )
	{
		DELTA_UNSETBYINDEX( pFields, custom_entity_field_alias[ CUSTOMFIELD_ORIGIN0 ].field );
		DELTA_UNSETBYINDEX( pFields, custom_entity_field_alias[ CUSTOMFIELD_ORIGIN1 ].field );
		DELTA_UNSETBYINDEX( pFields, custom_entity_field_alias[ CUSTOMFIELD_ORIGIN2 ].field );
	}

	if ( beamType != BEAM_POINTS )
	{
		DELTA_UNSETBYINDEX( pFields, custom_entity_field_alias[ CUSTOMFIELD_ANGLES0 ].field );
		DELTA_UNSETBYINDEX( pFields, custom_entity_field_alias[ CUSTOMFIELD_ANGLES1 ].field );
		DELTA_UNSETBYINDEX( pFields, custom_entity_field_alias[ CUSTOMFIELD_ANGLES2 ].field );
	}

	if ( beamType != BEAM_ENTS && beamType != BEAM_ENTPOINT )
	{
		DELTA_UNSETBYINDEX( pFields, custom_entity_field_alias[ CUSTOMFIELD_SKIN ].field );
		DELTA_UNSETBYINDEX( pFields, custom_entity_field_alias[ CUSTOMFIELD_SEQUENCE ].field );
	}

	// animtime is compared by rounding first
	// see if we really shouldn't actually send it
	if ( (int)f->animtime == (int)t->animtime )
	{
		DELTA_UNSETBYINDEX( pFields, custom_entity_field_alias[ CUSTOMFIELD_ANIMTIME ].field );
	}
}

/*
=================
RegisterEncoders

Allows game .dll to override network encoding of certain types of entities and tweak values, etc.
=================
*/
void RegisterEncoders( void )
{
	DELTA_ADDENCODER( "Entity_Encode", Entity_Encode );
	DELTA_ADDENCODER( "Custom_Encode", Custom_Encode );
	DELTA_ADDENCODER( "Player_Encode", Player_Encode );
}

int GetWeaponData( struct edict_s *player, struct weapon_data_s *info )
{
#if defined( CLIENT_WEAPONS )
	int i;
	weapon_data_t *item;
	entvars_t *pev = &player->v;
	CBasePlayer *pl = ( CBasePlayer *) CBasePlayer::Instance( pev );
	CBasePlayerWeapon *gun;
	
	ItemInfo II;

	memset( info, 0, 32 * sizeof( weapon_data_t ) );

	if ( !pl )
		return 1;

	// go through all of the weapons and make a list of the ones to pack
	for ( i = 0 ; i < MAX_ITEM_TYPES ; i++ )
	{
		if ( pl->m_rgpPlayerItems[ i ] )
		{
			// there's a weapon here. Should I pack it?
			CBasePlayerItem *pPlayerItem = pl->m_rgpPlayerItems[ i ];

			while ( pPlayerItem )
			{
				gun = (CBasePlayerWeapon *)pPlayerItem->GetWeaponPtr();
				if ( gun && gun->UseDecrement() )
				{
					// Get The ID.
					memset( &II, 0, sizeof( II ) );
					gun->GetItemInfo( &II );

					if ( II.iId >= 0 && II.iId < 32 )
					{
						item = &info[ II.iId ];
					 	
						item->m_iId						= II.iId;
						item->m_iClip					= gun->m_iClip;

						item->m_flTimeWeaponIdle		= max( gun->m_flTimeWeaponIdle, -0.001 );
						item->m_flNextPrimaryAttack		= max( gun->m_flNextPrimaryAttack, -0.001 );
						item->m_flNextSecondaryAttack	= max( gun->m_flNextSecondaryAttack, -0.001 );
						item->m_fInReload				= gun->m_fInReload;
						item->m_fInSpecialReload		= gun->m_fInSpecialReload;
						item->fuser1					= max( gun->pev->fuser1, -0.001 );
						item->fuser2					= gun->m_flStartThrow;
						item->fuser3					= gun->m_flReleaseThrow;
						item->iuser1					= gun->m_chargeReady;
						item->iuser2					= gun->m_fInAttack;
						item->iuser3					= gun->m_fireState;
						
											
//						item->m_flPumpTime				= max( gun->m_flPumpTime, -0.001 );
					}
				}
				pPlayerItem = pPlayerItem->m_pNext;
			}
		}
	}
#else
	memset( info, 0, 32 * sizeof( weapon_data_t ) );
#endif
	return 1;
}

/*
=================
UpdateClientData

Data sent to current client only
engine sets cd to 0 before calling.
=================
*/
void UpdateClientData ( const struct edict_s *ent, int sendweapons, struct clientdata_s *cd )
{
	cd->flags			= ent->v.flags;
	cd->health			= ent->v.health;

	cd->viewmodel		= MODEL_INDEX( STRING( ent->v.viewmodel ) );

	cd->waterlevel		= ent->v.waterlevel;
	cd->watertype		= ent->v.watertype;
	cd->weapons			= ent->v.weapons;

	// Vectors
	cd->origin			= ent->v.origin;
	cd->velocity		= ent->v.velocity;
	cd->view_ofs		= ent->v.view_ofs;
	cd->punchangle		= ent->v.punchangle;

	cd->bInDuck			= ent->v.bInDuck;
	cd->flTimeStepSound = ent->v.flTimeStepSound;
	cd->flDuckTime		= ent->v.flDuckTime;
	cd->flSwimTime		= ent->v.flSwimTime;
	cd->waterjumptime	= ent->v.teleport_time;

	strcpy( cd->physinfo, ENGINE_GETPHYSINFO( ent ) );

	cd->maxspeed		= ent->v.maxspeed;
	cd->fov				= ent->v.fov;
	cd->weaponanim		= ent->v.weaponanim;

	cd->pushmsec		= ent->v.pushmsec;

#if defined( CLIENT_WEAPONS )
	if ( sendweapons )
	{
		entvars_t *pev = (entvars_t *)&ent->v;
		CBasePlayer *pl = ( CBasePlayer *) CBasePlayer::Instance( pev );
		CBasePlayer* ppl = (CBasePlayer*)CBasePlayer::Instance((entvars_t *)&ent->v);


		cd->iuser4 = ppl->m_iWeapons2;

		if ( pl )
		{
			cd->m_flNextAttack	= pl->m_flNextAttack;
			cd->fuser2			= pl->m_flNextAmmoBurn;
			cd->fuser3			= pl->m_flAmmoStartCharge;
			cd->vuser1.x		= pl->ammo_9mm;
			cd->vuser1.y		= pl->ammo_357;
			cd->vuser1.z		= pl->ammo_argrens;
			cd->ammo_nails		= pl->ammo_bolts;
			cd->ammo_shells		= pl->ammo_buckshot;
			cd->ammo_rockets	= pl->ammo_rockets;
			cd->ammo_cells		= pl->ammo_uranium;
			cd->vuser2.x		= pl->ammo_hornets;
			

			cd->iuser4 = ppl->m_iWeapons2;
			if ( pl->m_pActiveItem )
			{
				CBasePlayerWeapon *gun;
				gun = (CBasePlayerWeapon *)pl->m_pActiveItem->GetWeaponPtr();
				if ( gun && gun->UseDecrement() )
				{
					ItemInfo II;
					memset( &II, 0, sizeof( II ) );
					gun->GetItemInfo( &II );

					cd->m_iId = II.iId;

					cd->vuser3.z	= gun->m_iSecondaryAmmoType;
					cd->vuser4.x	= gun->m_iPrimaryAmmoType;
					cd->vuser4.y	= pl->m_rgAmmo[gun->m_iPrimaryAmmoType];
					cd->vuser4.z	= pl->m_rgAmmo[gun->m_iSecondaryAmmoType];
					
					cd->iuser4 = ppl->m_iWeapons2;
					
					if ( pl->m_pActiveItem->m_iId == WEAPON_RPG )
					{
						cd->vuser2.y = ( ( CRpg * )pl->m_pActiveItem)->m_fSpotActive;
						cd->vuser2.z = ( ( CRpg * )pl->m_pActiveItem)->m_cActiveRockets;
					}
				}
			}
		}
		
		cd->iuser4 = ppl->m_iWeapons2;
	}
	
#endif
}

/*
=================
CmdStart

We're about to run this usercmd for the specified player.  We can set up groupinfo and masking here, etc.
This is the time to examine the usercmd for anything extra.  This call happens even if think does not.
=================
*/
void CmdStart( const edict_t *player, const struct usercmd_s *cmd, unsigned int random_seed )
{
	entvars_t *pev = (entvars_t *)&player->v;
	CBasePlayer *pl = ( CBasePlayer *) CBasePlayer::Instance( pev );

	if( !pl )
		return;

	if ( pl->pev->groupinfo != 0 )
	{
		UTIL_SetGroupTrace( pl->pev->groupinfo, GROUP_OP_AND );
	}

	pl->random_seed = random_seed;
}

/*
=================
CmdEnd

Each cmdstart is exactly matched with a cmd end, clean up any group trace flags, etc. here
=================
*/
void CmdEnd ( const edict_t *player )
{
	entvars_t *pev = (entvars_t *)&player->v;
	CBasePlayer *pl = ( CBasePlayer *) CBasePlayer::Instance( pev );

	if( !pl )
		return;
	if ( pl->pev->groupinfo != 0 )
	{
		UTIL_UnsetGroupTrace();
	}
}

/*
================================
ConnectionlessPacket

 Return 1 if the packet is valid.  Set response_buffer_size if you want to send a response packet.  Incoming, it holds the max
  size of the response_buffer, so you must zero it out if you choose not to respond.
================================
*/
int	ConnectionlessPacket( const struct netadr_s *net_from, const char *args, char *response_buffer, int *response_buffer_size )
{
	// Parse stuff from args
	int max_buffer_size = *response_buffer_size;

	// Zero it out since we aren't going to respond.
	// If we wanted to response, we'd write data into response_buffer
	*response_buffer_size = 0;

	// Since we don't listen for anything here, just respond that it's a bogus message
	// If we didn't reject the message, we'd return 1 for success instead.
	return 0;
}

/*
================================
GetHullBounds

  Engine calls this to enumerate player collision hulls, for prediction.  Return 0 if the hullnumber doesn't exist.
================================
*/
int GetHullBounds( int hullnumber, float *mins, float *maxs )
{
	int iret = 0;

	switch ( hullnumber )
	{
	case 0:				// Normal player
		mins = VEC_HULL_MIN;
		maxs = VEC_HULL_MAX;
		iret = 1;
		break;
	case 1:				// Crouched player
		mins = VEC_DUCK_HULL_MIN;
		maxs = VEC_DUCK_HULL_MAX;
		iret = 1;
		break;
	case 2:				// Point based hull
		mins = Vector( 0, 0, 0 );
		maxs = Vector( 0, 0, 0 );
		iret = 1;
		break;
	}

	return iret;
}

/*
================================
CreateInstancedBaselines

Create pseudo-baselines for items that aren't placed in the map at spawn time, but which are likely
to be created during play ( e.g., grenades, ammo packs, projectiles, corpses, etc. )
================================
*/
void CreateInstancedBaselines ( void )
{
	int iret = 0;
	entity_state_t state;

	memset( &state, 0, sizeof( state ) );

	// Create any additional baselines here for things like grendates, etc.
	// iret = ENGINE_INSTANCE_BASELINE( pc->pev->classname, &state );

	// Destroy objects.
	//UTIL_Remove( pc );
}

/*
================================
InconsistentFile

One of the ENGINE_FORCE_UNMODIFIED files failed the consistency check for the specified player
 Return 0 to allow the client to continue, 1 to force immediate disconnection ( with an optional disconnect message of up to 256 characters )
================================
*/
int	InconsistentFile( const edict_t *player, const char *filename, char *disconnect_message )
{
	// Server doesn't care?
	if ( CVAR_GET_FLOAT( "mp_consistency" ) != 1 )
		return 0;

	// Default behavior is to kick the player
	sprintf( disconnect_message, "Server is enforcing file consistency for %s\n", filename );

	// Kick now with specified disconnect message.
	return 1;
}

/*
================================
AllowLagCompensation

 The game .dll should return 1 if lag compensation should be allowed ( could also just set
  the sv_unlag cvar.
 Most games right now should return 0, until client-side weapon prediction code is written
  and tested for them ( note you can predict weapons, but not do lag compensation, too, 
  if you want.
================================
*/
int AllowLagCompensation( void )
{
	return 1;
}
