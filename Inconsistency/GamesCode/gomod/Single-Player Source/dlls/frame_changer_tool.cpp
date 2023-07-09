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

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "soundent.h"
#include "gamerules.h"

enum posertool_e
{

	POSERTOOL_IDLE,
	POSERTOOL_IDLE2,
	//REMOVETOOL_LAUNCH,
	POSERTOOL_RELOAD,
	POSERTOOL_SHOOT,
	POSERTOOL_SHOOT2,
	POSERTOOL_DRAW,
	
};



LINK_ENTITY_TO_CLASS( weapon_posertool, CPoserTool);




void CPoserTool::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_posertool"); // hack to allow for old names
	Precache( );
	SET_MODEL(ENT(pev), "models/w_removetool.mdl");
	m_iId = WEAPON_POSERTOOL;

	m_iDefaultAmmo = 1;

	FallInit();// get ready to fall down.
}



void CPoserTool::Precache( void )
{
	PRECACHE_MODEL("models/v_removetool.mdl");
	PRECACHE_MODEL("models/w_removetool.mdl");
	PRECACHE_MODEL("models/p_removetool.mdl");

	m_iShell = PRECACHE_MODEL ("models/shell.mdl");// brass shellTE_MODEL



	PRECACHE_MODEL("models/w_9mmARclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");              


	PRECACHE_SOUND ("weapons/tg_shot1.wav");// H to the K
	PRECACHE_SOUND ("weapons/tg_shot2.wav");// H to the K


	PRECACHE_SOUND ("weapons/357_cock1.wav");

	m_posertool = PRECACHE_EVENT( 1, "events/posertool.sc" );

}

int CPoserTool::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "9mm";
	p->iMaxAmmo1 = 1;
	p->pszAmmo2 = "NULL";
	p->iMaxAmmo2 = -1;
	p->iMaxClip = 1;
	p->iSlot = 0;
	p->iPosition = 4;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_POSERTOOL;
	p->iWeight = MP5_WEIGHT;

	return 1;
}

int CPoserTool::AddToPlayer( CBasePlayer *pPlayer )
{
	if ( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev );
			WRITE_BYTE( m_iId );
		MESSAGE_END();
		return TRUE;
	}
	return FALSE;
}

BOOL CPoserTool::Deploy( )
{
	return DefaultDeploy( "models/v_removetool.mdl", "models/p_removetool.mdl", POSERTOOL_DRAW, "mp5" );
}


void CPoserTool::PrimaryAttack()
{
	if (!(m_pPlayer->m_afButtonPressed & IN_ATTACK))
    return;

	// don't fire underwater
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound( );
		m_flNextPrimaryAttack = 0.15;
		return;
	}

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

	//m_iClip--;


	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

	// player "shoot" animation
	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

	Vector vecSrc	 = m_pPlayer->GetGunPosition( );
	Vector vecAiming = m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );
	Vector vecDir;

#ifdef CLIENT_DLL
	if ( !bIsMultiplayer() )
#else
	if ( !g_pGameRules->IsMultiplayer() )
#endif
	{
		// optimized multiplayer. Widened to make it easier to hit a moving player
		
		
		vecDir = m_pPlayer->FireBulletsPoserTool( 1, vecSrc, vecAiming, VECTOR_CONE_1DEGREES, 8192, BULLET_PLAYER_MP5, 2, 0, m_pPlayer->pev, m_pPlayer->random_seed );
	}
	else
	{
		// single player spread

		vecDir = m_pPlayer->FireBulletsPoserTool( 1, vecSrc, vecAiming, VECTOR_CONE_1DEGREES, 8192, BULLET_PLAYER_MP5, 2, 0, m_pPlayer->pev, m_pPlayer->random_seed );
	}

  int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif
	
	//pev->rendermode = kRenderGlow;
	//pev->renderamt = 192;
	//pev->renderfx = kRenderFxExplode;

	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_posertool, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0 );

	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);


	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1;

	if ( m_flNextPrimaryAttack < UTIL_WeaponTimeBase() )
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
}




void CPoserTool::WeaponIdle( void )
{

	ResetEmptySound( );

	m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	int iAnim;
	switch ( RANDOM_LONG( 0, 1 ) )
	{
	case 0:	
		iAnim = POSERTOOL_IDLE;	
		break;
	
	default:
	case 1:
		iAnim = POSERTOOL_IDLE;
		break;
	}

	SendWeaponAnim( iAnim );
	
	m_flTimeWeaponIdle = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 ); // how long till we do this again.
}










