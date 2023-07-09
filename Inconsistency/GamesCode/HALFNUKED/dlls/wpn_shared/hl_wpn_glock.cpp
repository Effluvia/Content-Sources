/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
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

enum glock_e {
	GLOCK_IDLE1 = 0,
	GLOCK_DRAW,
	GLOCK_RELOAD,
	GLOCK_SHOOT
};

LINK_ENTITY_TO_CLASS( weapon_glock, CGlock );
LINK_ENTITY_TO_CLASS(weapon_9mmhandgun, CGlock);
LINK_ENTITY_TO_CLASS(weapon_crowbar, CGlock);
LINK_ENTITY_TO_CLASS(weapon_pistol, CGlock);


void CGlock::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_9mmhandgun"); // hack to allow for old names
	Precache( );
	m_iId = WEAPON_GLOCK;
	SET_MODEL(ENT(pev), "models/w_pistol.mdl");

	m_iDefaultAmmo = GLOCK_DEFAULT_GIVE;

	m_iFirstDraw = TRUE;

	FallInit();// get ready to fall down.
}


void CGlock::Precache( void )
{
	PRECACHE_MODEL("models/v_pistol.mdl");
	PRECACHE_MODEL("models/w_pistol.mdl");
	PRECACHE_MODEL("models/p_pistol.mdl");

	m_iShell = PRECACHE_MODEL ("models/shell.mdl");// brass shell

	PRECACHE_SOUND("items/9mmclip1.wav");
	PRECACHE_SOUND("items/9mmclip2.wav");
	PRECACHE_SOUND("items/gunpickup4.wav");

	PRECACHE_SOUND ("weapons/pl_gun1.wav");//silenced handgun
	PRECACHE_SOUND ("weapons/pl_gun2.wav");//silenced handgun
	PRECACHE_SOUND ("weapons/pistol_fire.wav");//handgun

	m_usFireGlock1 = PRECACHE_EVENT( 1, "events/glock1.sc" );
	m_usFireGlock2 = PRECACHE_EVENT( 1, "events/glock2.sc" );
}

int CGlock::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "9mm";
	p->iMaxAmmo1 = _9MM_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 1;
	p->iPosition = 0;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_GLOCK;
	p->iWeight = GLOCK_WEIGHT;

	return 1;
}

BOOL CGlock::Deploy( )
{
	// pev->body = 1;

//	if (m_iFirstDraw)
//	{
//		
//		if (m_pPlayer->pev->weapons & (1 << WEAPON_INTRO))
//			m_pPlayer->SwitchWeapon(m_pPlayer->m_rgpPlayerItems[15]);
//
//		return DefaultDeploy("models/v_pistol.mdl", "models/p_9mmhandgun.mdl", GLOCK_RELOAD, "onehanded", /*UseDecrement() ? 1 : 0*/ 0);
//	}
//	else
		return DefaultDeploy( "models/v_pistol.mdl", "models/p_pistol.mdl", GLOCK_DRAW, "onehanded", /*UseDecrement() ? 1 : 0*/ 0 );
}

void CGlock::SecondaryAttack( void )
{
	//GlockFire( 0.1, 0.16, FALSE );
	Kick();
}

void CGlock::PrimaryAttack( void )
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
	{
		m_flNextPrimaryAttack = GetNextAttackDelay(0.5);
		PlayEmptySound();
		return;
	}
	else
	{
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;

		m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

		int flags;
#if defined( CLIENT_WEAPONS )
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);

		m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

		Vector vecSrc = m_pPlayer->GetGunPosition();
		Vector vecAiming;

		vecAiming = m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

		Vector vecDir;
		vecDir = m_pPlayer->FireBulletsPlayer(1, vecSrc, vecAiming, Vector(0.01, 0.01, 0.01), 8192, BULLET_PLAYER_9MM, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed);

		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireGlock1, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, (m_iClip == 0) ? 1 : 0, 0);

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = GetNextAttackDelay(0.15);

		SendWeaponAnim(GLOCK_SHOOT, 1);

		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			// HEV suit - indicate out of ammo condition
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 15;
	}

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] != 0 && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] % GLOCK_MAX_CLIP == 0)
	{
		FakeReload();
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = GetNextAttackDelay(1.0);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5;
	}
}

void CGlock::GlockFire( float flSpread , float flCycleTime, BOOL fUseAutoAim )
{
}


void CGlock::Reload(void)
{
}


void CGlock::FakeReload( void )
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		 return;

	SendWeaponAnim(GLOCK_RELOAD, 1);
}



void CGlock::WeaponIdle( void )
{
	m_iFirstDraw = FALSE;
	ResetEmptySound( );

	m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	// only idle if the slid isn't back
	if (m_iClip != 0)
	{
		float flRand = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 0.0, 1.0 );
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 40.0 / 16.0;
		SendWeaponAnim(GLOCK_IDLE1, 1);
	}
}



class CGlockAmmo : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/w_9mmclip.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		if (pOther->GiveAmmo( AMMO_GLOCKCLIP_GIVE, "9mm", _9MM_MAX_CARRY ) != -1)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
			pOther->pev->iuser4 = T_PICKEDUPAMMO;
			return TRUE;
		}
		return FALSE;
	}
};
LINK_ENTITY_TO_CLASS( ammo_glockclip, CGlockAmmo );
LINK_ENTITY_TO_CLASS( ammo_9mmclip, CGlockAmmo );