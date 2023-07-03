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

enum deagle_e {
	EAGLE_IDLE1 = 0,
	EAGLE_IDLE2,
	EAGLE_IDLE3,
	EAGLE_IDLE4,
	EAGLE_IDLE5,
	EAGLE_SHOOT,
	EAGLE_SHOOT_EMPTY,
	EAGLE_RELOAD,
	EAGLE_RELOAD_NOT_EMPTY,
	EAGLE_DRAW,
	EAGLE_HOLSTER
};

LINK_ENTITY_TO_CLASS( weapon_desert_eagle, CDEagle );

void CDEagle::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_desert_eagle"); // hack to allow for old names
	Precache( );
	m_iId = WEAPON_DEAGLE;
	SET_MODEL(ENT(pev), "models/w_desert_eagle.mdl");

	m_iDefaultAmmo = DEAGLE_DEFAULT_GIVE;
	m_fSpotActive = FALSE;

	FallInit();// get ready to fall down.
}


void CDEagle::Precache( void )
{
	PRECACHE_MODEL("models/v_desert_eagle.mdl");
	PRECACHE_MODEL("models/w_desert_eagle.mdl");
	PRECACHE_MODEL("models/p_desert_eagle.mdl");

	m_iShell = PRECACHE_MODEL ("models/shell.mdl");// brass shell

	PRECACHE_SOUND("items/9mmclip1.wav");
	PRECACHE_SOUND("items/9mmclip2.wav");

	PRECACHE_SOUND ("weapons/desert_eagle_reload.wav");
	PRECACHE_SOUND ("weapons/desert_fire.wav");
	PRECACHE_SOUND ("weapons/desert_sight_off.wav");
	PRECACHE_SOUND ("weapons/desert_sight_on.wav");

	UTIL_PrecacheOther( "laser_spot" );

	m_usFireEagle = PRECACHE_EVENT( 1, "events/deagle.sc" );
}

int CDEagle::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "357";
	p->iMaxAmmo1 = _357_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = DEAGLE_MAX_CLIP;
	p->iSlot = 1;
	p->iPosition = 2;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_DEAGLE;
	p->iWeight = DEAGLE_WEIGHT;

	return 1;
}

BOOL CDEagle::Deploy( )
{
	return DefaultDeploy( "models/v_desert_eagle.mdl", "models/p_desert_eagle.mdl", EAGLE_DRAW, "onehanded", 0 );
}

void CDEagle::Holster( int skiplocal )
{
	if (m_pSpot)
	{
		m_pSpot->Killed( NULL, GIB_NEVER );
		m_pSpot = NULL;
	}
}

void CDEagle::SecondaryAttack( void )
{
	if(m_fSpotActive)
		EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/desert_sight_off.wav", 1, ATTN_NORM);
	else
		EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/desert_sight_on.wav", 1, ATTN_NORM);

	m_fSpotActive = ! m_fSpotActive;

#ifndef CLIENT_DLL
	if (!m_fSpotActive && m_pSpot)
	{
		m_pSpot->Killed( NULL, GIB_NORMAL );
		m_pSpot = NULL;
	}
#endif

	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5;
}

void CDEagle::UpdateSpot( void )
{
	if (m_fSpotActive)
	{
		if (!m_pSpot)
		{
			m_pSpot = CLaserSpot::CreateSpot( 0.25 );
		}

		UTIL_MakeVectors( m_pPlayer->pev->v_angle );
		Vector vecSrc = m_pPlayer->GetGunPosition( );
		Vector vecAiming = gpGlobals->v_forward;

		TraceResult tr;
		UTIL_TraceLine ( vecSrc, vecSrc + vecAiming * 8192, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr );
		
		UTIL_SetOrigin( m_pSpot->pev, tr.vecEndPos );
	}
}

void CDEagle::PrimaryAttack( void )
{
	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2;
		}

		return;
	}

	m_iClip--;

	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

	int flags;
	flags = 0;

	// player "shoot" animation
	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

	Vector vecSrc	 = m_pPlayer->GetGunPosition( );
	Vector vecAiming;
	
	vecAiming = m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

	float flSpread;
	float flCycleTime;
	if( m_fSpotActive )
	{
		flSpread = 0.01;
		flCycleTime = 0.5;
	}
	else
	{
		flSpread = 0.1;
		flCycleTime = 0.2;
	}

	if(m_fSpotActive && m_pSpot)
		m_pSpot->Suspend( flCycleTime );

	Vector vecDir;
	vecDir = m_pPlayer->FireBulletsPlayer( 1, vecSrc, vecAiming, Vector( flSpread, flSpread, flSpread ), 8192, BULLET_PLAYER_DEAGLE, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );

	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usFireEagle, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, ( m_iClip == 0 ) ? 1 : 0, 0 );

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
}


void CDEagle::Reload( void )
{
	if ( m_pPlayer->ammo_357 <= 0 )
		 return;

	int iResult = DefaultReload( DEAGLE_MAX_CLIP, m_iClip == 0 ? EAGLE_RELOAD : EAGLE_RELOAD_NOT_EMPTY, 1.68 );

	if( m_pSpot && m_fSpotActive )
		m_pSpot->Suspend( 1.7 );

	if (iResult)
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
	}
}

void CDEagle::WeaponIdle( void )
{
	UpdateSpot( );

	ResetEmptySound( );

	m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	// only idle if the slid isn't back
	if (m_iClip != 0)
	{
		int iAnim;
		float idleTime;

		switch( RANDOM_LONG(0, 4) )
		{
			case 0:
				iAnim = EAGLE_IDLE1;
				idleTime = 2.532*RANDOM_LONG(1, 2);
				break;
			case 1:
				iAnim = EAGLE_IDLE2;
				idleTime = 2.54;
				break;
			case 2:
				iAnim = EAGLE_IDLE3;
				idleTime = 1.65*RANDOM_LONG(1, 3);
				break;
			case 3:
				iAnim = EAGLE_IDLE4;
				idleTime = 2.52*RANDOM_LONG(1, 2);
				break;
			case 4:
				iAnim = EAGLE_IDLE5;
				idleTime = 2.02*RANDOM_LONG(1, 2);
				break;
		}

		SendWeaponAnim( iAnim, 1 );
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + idleTime;
	}
}














