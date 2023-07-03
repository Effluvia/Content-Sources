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

enum saw_e
{
	SAW_LONGIDLE = 0,
	SAW_IDLE1,
	SAW_RELOAD1,
	SAW_RELOAD2,
	SAW_HOLSTER,
	SAW_DEPLOY,
	SAW_FIRE1,
	SAW_FIRE2,
	SAW_FIRE3,
};

LINK_ENTITY_TO_CLASS( weapon_saw, CSAW );

void CSAW::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_saw"); // hack to allow for old names
	Precache( );
	SET_MODEL(ENT(pev), "models/w_saw.mdl");
	m_iId = WEAPON_SAW;

	m_iDefaultAmmo = SAW_DEFAULT_GIVE;

	FallInit();// get ready to fall down.
}


void CSAW::Precache( void )
{
	PRECACHE_MODEL("models/v_saw.mdl");
	PRECACHE_MODEL("models/w_saw.mdl");
	PRECACHE_MODEL("models/p_saw.mdl");

	m_iShell = PRECACHE_MODEL ("models/saw_shell.mdl");// brass shellTE_MODEL
	m_iLink = PRECACHE_MODEL ("models/saw_link.mdl");// brass shellTE_MODEL

	PRECACHE_MODEL("models/w_saw_clip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");              

	PRECACHE_SOUND("weapons/saw_reload.wav");
	PRECACHE_SOUND("weapons/saw_reload2.wav");

	PRECACHE_SOUND ("weapons/saw_shoot1.wav");
	PRECACHE_SOUND ("weapons/saw_shoot2.wav");
	PRECACHE_SOUND ("weapons/saw_shoot3.wav");

	PRECACHE_SOUND ("weapons/357_cock1.wav");

	m_usSAW = PRECACHE_EVENT( 1, "events/saw.sc" );
}

int CSAW::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "556";
	p->iMaxAmmo1 = _556_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = SAW_MAX_CLIP;
	p->iSlot = 5;
	p->iPosition = 0;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_SAW;
	p->iWeight = SAW_WEIGHT;

	return 1;
}

int CSAW::AddToPlayer( CBasePlayer *pPlayer )
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

BOOL CSAW::Deploy( )
{
	return DefaultDeploy( "models/v_saw.mdl", "models/p_saw.mdl", SAW_DEPLOY, "mp5", 0, GetBody() );
}


void CSAW::PrimaryAttack()
{
	// don't fire underwater
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound( );
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() +  0.15;
		return;
	}

	if (m_iClip <= 0)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() +  0.15;
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

#ifdef CLIENT_DLL
	if ( !bIsMultiplayer() )
#else
	if ( !g_pGameRules->IsMultiplayer() )
#endif
	{
		// optimized multiplayer. Widened to make it easier to hit a moving player
		vecDir = m_pPlayer->FireBulletsPlayer( 1, vecSrc, vecAiming, VECTOR_CONE_8DEGREES, 8192, BULLET_PLAYER_556, 2, 0, m_pPlayer->pev, m_pPlayer->random_seed );
	}
	else
	{
		// single player spread
		vecDir = m_pPlayer->FireBulletsPlayer( 1, vecSrc, vecAiming, VECTOR_CONE_4DEGREES, 8192, BULLET_PLAYER_556, 2, 0, m_pPlayer->pev, m_pPlayer->random_seed );
	}

	int flags;
	flags = 0;

	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usSAW, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, GetBody(), 0, 0, 0 );

	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.067;

	if ( m_flNextPrimaryAttack < UTIL_WeaponTimeBase() )
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.067;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
}

void CSAW::Reload( void )
{
	if ( m_pPlayer->ammo_saw <= 0 )
		return;

	if(DefaultReload( SAW_MAX_CLIP, SAW_RELOAD1, 1.525, GetBody() ))
		m_flReloadDelay = UTIL_WeaponTimeBase() + 1.525;
}

int CSAW::GetBody( int iClip )
{
	if( iClip == -1 )
		iClip = m_iClip;

	if( iClip > 8 )
		return 0;

	if( iClip == 0 )
		return 8;

	int iRounds = 9 - iClip;
	return iRounds;
}

void CSAW::ItemPostFrame( void )
{
	if(m_flReloadDelay && m_flReloadDelay <= m_flReloadDelay)
	{
		int iClip = min( SAW_MAX_CLIP - m_iClip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] );	
		iClip += m_iClip;

		SendWeaponAnim( SAW_RELOAD2, TRUE, GetBody(iClip) );
		m_flReloadDelay = NULL;

		m_flTimeWeaponIdle = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 2.47;
		return;
	}

	CBasePlayerWeapon::ItemPostFrame();
}

void CSAW::WeaponIdle( void )
{
	ResetEmptySound( );

	m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	int iAnim;
	switch ( RANDOM_LONG( 0, 1 ) )
	{
	case 0:	
		iAnim = SAW_LONGIDLE;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5.05*RANDOM_LONG(1, 3);
		break;
	
	default:
	case 1:
		iAnim = SAW_IDLE1;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 6.2;
		break;
	}

	SendWeaponAnim( iAnim, TRUE, GetBody() );
}

class CSAWAmmoClip : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/w_saw_clip.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/w_saw_clip.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		int bResult = (pOther->GiveAmmo( AMMO_MP5CLIP_GIVE, "556", _9MM_MAX_CARRY) != -1);
		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};
LINK_ENTITY_TO_CLASS( ammo_sawclip, CSAWAmmoClip );


















