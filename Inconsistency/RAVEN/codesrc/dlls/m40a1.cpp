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

enum m40a1_e {
	SNIPER_DRAW = 0,
	SNIPER_SLOWIDLE,
	SNIPER_FIRE,
	SNIPER_FIRE_LAST,
	SNIPER_RELOAD_EMPTY,
	SNIPER_RELOAD,
	SNIPER_SLOWIDLE2,
	SNIPER_HOLSTER
};

LINK_ENTITY_TO_CLASS( weapon_sniperrifle, CM40A1 );

void CM40A1::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_sniperrifle"); // hack to allow for old names
	Precache( );
	m_iId = WEAPON_M40A1;
	SET_MODEL(ENT(pev), "models/w_m40a1.mdl");

	m_iDefaultAmmo = M40A1_DEFAULT_GIVE;
	m_fInZoom = FALSE;

	FallInit();// get ready to fall down.
}

void CM40A1::Precache( void )
{
	PRECACHE_MODEL("models/v_m40a1.mdl");
	PRECACHE_MODEL("models/w_m40a1.mdl");
	PRECACHE_MODEL("models/p_m40a1.mdl");

	PRECACHE_SOUND("items/9mmclip1.wav");
	PRECACHE_SOUND("items/9mmclip2.wav");

	PRECACHE_SOUND ("weapons/sniper_bolt1.wav");
	PRECACHE_SOUND ("weapons/sniper_bolt2.wav");
	PRECACHE_SOUND ("weapons/sniper_fire.wav");
	PRECACHE_SOUND ("weapons/sniper_reload_first_seq.wav");
	PRECACHE_SOUND ("weapons/sniper_reload_second_seq.wav");
	PRECACHE_SOUND ("weapons/sniper_reload3.wav");
	PRECACHE_SOUND ("weapons/sniper_zoom.wav");

	m_usFireM40A1 = PRECACHE_EVENT( 1, "events/m40a1.sc" );
}

int CM40A1::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "762";
	p->iMaxAmmo1 = _762_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = M40A1_MAX_CLIP;
	p->iSlot = 5;
	p->iPosition = 2;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_M40A1;
	p->iWeight = M40A1_WEIGHT;

	return 1;
}

BOOL CM40A1::Deploy( )
{
	return DefaultDeploy( "models/v_m40a1.mdl", "models/p_m40a1.mdl", SNIPER_DRAW, "bow", 0 );
}

void CM40A1::Holster( int skiplocal )
{
	if(m_fInZoom)
		ToggleZoom();
}

void CM40A1::SecondaryAttack( void )
{
	ToggleZoom();
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5;
}

void CM40A1::ToggleZoom( void )
{
	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/sniper_zoom.wav", 1, ATTN_NORM);

	if ( m_pPlayer->pev->fov != 0 )
	{
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 0; // 0 means reset to default fov
		m_fInZoom = 0;
	}
	else if ( m_pPlayer->pev->fov != 30 )
	{
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 30;
		m_fInZoom = 1;
	}
}

void CM40A1::PrimaryAttack( void )
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

	Vector vecSrc = m_pPlayer->GetGunPosition( );
	Vector vecAiming = m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );
	Vector vecDir = m_pPlayer->FireBulletsPlayer( 1, vecSrc, vecAiming, VECTOR_CONE_1DEGREES, 8192, BULLET_PLAYER_762, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );

	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usFireM40A1, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, ( m_iClip == 0 ) ? 1 : 0, 0 );

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.78;

	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
}

void CM40A1::Reload( void )
{
	if ( m_pPlayer->ammo_762 <= 0 )
		 return;

	int iAnim;
	float flTime;
	if( m_iClip == 0 )
	{
		flTime = 3.79;
		iAnim = SNIPER_RELOAD_EMPTY;
	}
	else
	{
		flTime = 2.35;
		iAnim = SNIPER_RELOAD;
	}

	if (DefaultReload( M40A1_MAX_CLIP, iAnim, flTime ))
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
}

void CM40A1::WeaponIdle( void )
{
	ResetEmptySound( );

	m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	SendWeaponAnim( m_iClip == 0 ? SNIPER_SLOWIDLE2 : SNIPER_SLOWIDLE, 1 );
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 4.39;
}

class CM40A1Ammo : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/w_m40a1_clip.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/w_m40a1_clip.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		if (pOther->GiveAmmo( AMMO_M40A1_GIVE, "762", _762_MAX_CARRY ) != -1)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
			return TRUE;
		}
		return FALSE;
	}
};
LINK_ENTITY_TO_CLASS( ammo_m40a1_clip, CM40A1Ammo );