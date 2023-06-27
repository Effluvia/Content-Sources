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

enum scopedrifle_e
{
	RIFLE_IDLE1 = 0,
	RIFLE_LONGIDLE,
	RIFLE_DEPLOY,
	RIFLE_RELOAD,
	RIFLE_FIRE1,
	RIFLE_FIRE2,
	RIFLE_FIRE3,
};



LINK_ENTITY_TO_CLASS( weapon_scopedrifle, CScopedRifle );


void CScopedRifle::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_scopedrifle"); // hack to allow for old names
	Precache( );
	SET_MODEL(ENT(pev), "models/w_scopedrifle.mdl");
	m_iId = WEAPON_RIFLE;

	m_iDefaultAmmo = RIFLE_DEFAULT_GIVE;

	FallInit();// get ready to fall down.
}


void CScopedRifle::Precache( void )
{
	PRECACHE_MODEL("models/v_scopedrifle.mdl");
	PRECACHE_MODEL("models/w_scopedrifle.mdl");
	PRECACHE_MODEL("models/p_scopedrifle.mdl");

	PRECACHE_MODEL("models/w_9mmARclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");              

	PRECACHE_SOUND("items/sr_clipin.wav");
	PRECACHE_SOUND("items/sr_clipout.wav");

	PRECACHE_SOUND ("weapons/srifle1.wav");// H to the K
	PRECACHE_SOUND ("weapons/srifle2.wav");// H to the K
	PRECACHE_SOUND ("weapons/srifle3.wav");// H to the K

	PRECACHE_SOUND ("weapons/357_cock1.wav");

	m_usRifle = PRECACHE_EVENT( 1, "events/rifle.sc" );
}

int CScopedRifle::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "srifle";
	p->iMaxAmmo1 = RIFLE_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = RIFLE_MAX_CLIP;
	p->iSlot = 2;
	p->iPosition = 4;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_RIFLE;
	p->iWeight = RIFLE_WEIGHT;

	return 1;
}

int CScopedRifle::AddToPlayer( CBasePlayer *pPlayer )
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

BOOL CScopedRifle::Deploy( )
{
	return DefaultDeploy( "models/v_scopedrifle.mdl", "models/v_scopedrifle.mdl", RIFLE_DEPLOY, "mp5" );
}


void CScopedRifle::PrimaryAttack()
{
	// don't fire underwater
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound( );
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15;
		return;
	}

	if (m_iClip <= 0)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15;
		return;
	}

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

	m_iClip--;


	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

	// player "shoot" animation
	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

	Vector vecSrc	 = m_pPlayer->GetGunPosition( );
	Vector vecAiming = m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );
	Vector vecDir;

	// single player spread
	vecDir = m_pPlayer->FireBulletsPlayer( 1, vecSrc, vecAiming, VECTOR_CONE_2DEGREES, 8192, BULLET_PLAYER_RIFLE, 2, 0, m_pPlayer->pev, m_pPlayer->random_seed );

	int flags;
	flags = 0;

	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usRifle, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0 );

	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1;

	if ( m_flNextPrimaryAttack < UTIL_WeaponTimeBase() )
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
}

void CScopedRifle::Reload( void )
{
	if ( m_pPlayer->ammo_9mm <= 0 )
		return;

	DefaultReload( RIFLE_MAX_CLIP, RIFLE_RELOAD, 3.2 );
}


void CScopedRifle::WeaponIdle( void )
{
	ResetEmptySound( );

	m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	int iAnim;
	switch ( RANDOM_LONG( 0, 1 ) )
	{
	case 0:	
		iAnim = RIFLE_LONGIDLE;	
		break;
	
	default:
	case 1:
		iAnim = RIFLE_IDLE1;
		break;
	}

	SendWeaponAnim( iAnim );

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 ); // how long till we do this again.
}

class CScopedRifleAmmoClip : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/w_rifle_clip.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/w_rifle_clip.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		int bResult = (pOther->GiveAmmo( AMMO_RIFLE_CLIP_GIVE, "9mm", _9MM_MAX_CARRY) != -1);
		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};
LINK_ENTITY_TO_CLASS( ammo_rifle_clip, CScopedRifleAmmoClip );