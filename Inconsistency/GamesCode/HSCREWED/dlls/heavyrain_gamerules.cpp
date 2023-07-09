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
//
// heavyrain_gamerules.cpp
//
#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"player.h"
#include	"weapons.h"
#include	"gamerules.h"
#include	"heavyrain_gamerules.h"
#include	"game.h"
//#include	"mp3.h"

extern DLL_GLOBAL BOOL		g_fGameOver;
extern int gmsgScoreInfo;
extern int gmsgPlayMP3; //AJH - Killars MP3player

CHeavyRainplay :: CHeavyRainplay()
{
 //Genuflect
}

BOOL CHeavyRainplay::IsHeavyRain()
{
 return TRUE;
}

void CHeavyRainplay::PlayerSpawn( CBasePlayer *pPlayer )
{
	BOOL		addDefault;
	CBaseEntity	*pWeaponEntity = NULL;

	pPlayer->pev->weapons |= (1<<WEAPON_SUIT);
	
	addDefault = TRUE;
	
	//entvars_t *pev = &pEntity->v;

//	CBasePlayer	*pPlayer;

	edict_t *pClient = g_engfuncs.pfnPEntityOfEntIndex( 1 );

	while ( pWeaponEntity = UTIL_FindEntityByClassname( pWeaponEntity, "game_player_equip" ))
	{
		pWeaponEntity->Touch( pPlayer );
		addDefault = FALSE;
	}

	if ( addDefault )
	{
		pPlayer->GiveNamedItem( "weapon_jason" );
		pPlayer->GiveNamedItem( "weapon_goldengun" );
		pPlayer->GiveAmmo( 15, "gold", GOLDENGUN_MAX_CARRY );
			//SERVER_COMMAND("mp3 play media/jayson.mp3\n");
	}
}

void CHeavyRainplay::PlayerKilled( CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor )
{
	CBasePlayer *peKiller = NULL;
	CBaseEntity *ktmp = CBaseEntity::Instance( pKiller );
	if ( pVictim->pev == pKiller )  
	{  // killed self
		pVictim->pev->frags -= 10;
		char victext[1024] = "You killed yourself looking for Jason\nYou sacrificed your life for Jason\nYou lose 10 Jasons for revival costs.\n";
		UTIL_SayText( victext, pVictim );
		return;
	}
	else if ( ktmp && ktmp->IsPlayer() )
	{
		int FUCK = pVictim->pev->frags;
		pKiller->frags += JasonsStolen(FUCK);
		pVictim->pev->frags -= JasonsStolen(FUCK);

		char victext[1024] = "You lose your Jasons to the killer.\nKILL HIM AND GET THEM BACK! ;)\n";
		UTIL_SayText( victext, pVictim );

		MESSAGE_BEGIN( MSG_ALL, gmsgScoreInfo );
		WRITE_BYTE( ENTINDEX(pVictim->edict()) );
		WRITE_SHORT( pVictim->pev->frags );
		WRITE_SHORT( pVictim->m_iDeaths );
		WRITE_SHORT( 0 );
		WRITE_SHORT( g_pGameRules->GetTeamIndex( pVictim->m_szTeamName ) + 1 );
		MESSAGE_END();

	CBaseEntity *ep = CBaseEntity::Instance( pKiller );
	if ( ep && ep->Classify() == CLASS_PLAYER )
	{
		CBasePlayer *PK = (CBasePlayer*)ep;

		char kiltext[1024] = "You stole your victim's Jasons and it is now added to your score.\nNo douct hes out for blood, watch out ;)\n";
		UTIL_SayText( kiltext, PK );

		MESSAGE_BEGIN( MSG_ALL, gmsgScoreInfo );
			WRITE_BYTE( ENTINDEX(PK->edict()) );
			WRITE_SHORT( PK->pev->frags );
			WRITE_SHORT( PK->m_iDeaths );
			WRITE_SHORT( 0 );
			WRITE_SHORT( GetTeamIndex( PK->m_szTeamName) + 1 );
		MESSAGE_END();

		// let the killer paint another decal as soon as he'd like.
		PK->m_flNextDecalTime = gpGlobals->time;
	}
	else
	{
		// World did them in, Genuflect.
	}
	}
}

int CHeavyRainplay::JasonsStolen(int jason)
{
return jason; //JASON! I found you!
}