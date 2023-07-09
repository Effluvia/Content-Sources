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
#if !defined( OEM_BUILD )

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "gamerules.h"




enum dosh_e {
	DOSH_IDLE = 0,
	DOSH_FIDGET,
	DOSH_RELOAD,		// to reload
	DOSH_FIRE2,		// to empty
	DOSH_HOLSTER1,	// loaded
	DOSH_DRAW1,		// loaded
	DOSH_HOLSTER2,	// unloaded
	DOSH_DRAW_UL,	// unloaded
	DOSH_IDLE_UL,	// unloaded idle
	DOSH_FIDGET_UL,	// unloaded fidget
};

LINK_ENTITY_TO_CLASS( weapon_dosh, CDosh );

#ifndef CLIENT_DLL
//LINK_ENTITY_TO_CLASS( laser_spot, CLazerSpot );

//=========================================================
//=========================================================
//CLazerSpot *CLazerSpot::CreateSpot( void )
//{
//	CLazerSpot *pSpot = GetClassPtr( (CLazerSpot *)NULL );
//	pSpot->Spawn();
//
//	pSpot->pev->classname = MAKE_STRING("laser_spot");
//
//	return pSpot;
//}/

//=========================================================
//=========================================================
//void CLazerSpot::Spawn( void )
//{
//	Precache( );
//	pev->movetype = MOVETYPE_NONE;
//	pev->solid = SOLID_NOT;
//
//	pev->rendermode = kRenderGlow;
//	pev->renderfx = kRenderFxNoDissipation;
//	pev->renderamt = 255;
//
//	SET_MODEL(ENT(pev), "sprites/laserdot.spr");
//	UTIL_SetOrigin( pev, pev->origin );
//};/

//=========================================================
// Suspend- make the laser sight invisible. 
//=========================================================
//void CLazerSpot::Suspend( float flSuspendTime )
//{
//	pev->effects |= EF_NODRAW;
//	
//	SetThink( &CLazerSpot::Revive );
//	pev->nextthink = gpGlobals->time + flSuspendTime;
//}

//=========================================================
// Revive - bring a suspended laser sight back.
//=========================================================
//void CLazerSpot::Revive( void )
//{
//	pev->effects &= ~EF_NODRAW;
//
//	SetThink( NULL );
//}/

//void CLazerSpot::Precache( void )
//{
//	PRECACHE_MODEL("sprites/laserdot.spr");
//};

LINK_ENTITY_TO_CLASS( dosh_rocket, CDoshRocket );
#endif

//=========================================================
//=========================================================
CDoshRocket *CDoshRocket::CreateDoshRocket( Vector vecOrigin, Vector vecAngles, CBaseEntity *pOwner, CDosh *pLauncher )
{
	CDoshRocket *pRocket = GetClassPtr( (CDoshRocket *)NULL );

	UTIL_SetOrigin( pRocket->pev, vecOrigin );
	pRocket->pev->angles = vecAngles;
	pRocket->Spawn();
	//pRocket->SetTouch( &CDoshRocket::RocketTouch );
	pRocket->m_pLauncher = pLauncher;// remember what RPG fired me. 
	pRocket->m_pLauncher->m_cActiveRockets++;// register this missile as active for the launcher
	pRocket->pev->owner = pOwner->edict();

	return pRocket;
}

//=========================================================
//=========================================================
void CDoshRocket :: Spawn( void )
{
	Precache( );
	// motor
	pev->movetype = MOVETYPE_BOUNCE;
	pev->solid = SOLID_BBOX;

	SET_MODEL(ENT(pev), "models/doshrocket.mdl");
	UTIL_SetSize(pev, Vector( 0, 0, 0), Vector(0, 0, 0));
	UTIL_SetOrigin( pev, pev->origin );

	pev->classname = MAKE_STRING("dosh_rocket");

	SetThink( &CDoshRocket::IgniteThink );
	//SetTouch( &CGrenade::ExplodeTouch );

	pev->angles.x -= 30;
	UTIL_MakeVectors( pev->angles );
	pev->angles.x = -(pev->angles.x + 30);

	pev->velocity = gpGlobals->v_forward * 150;
	pev->gravity = 0.5;

	pev->nextthink = gpGlobals->time + 0.4;

	pev->dmg = gSkillData.plrDmgDosh;
}

//=========================================================
//=========================================================
/*void CDoshRocket :: RocketTouch ( CBaseEntity *pOther )
{
	if ( m_pLauncher )
	{
		// my launcher is still around, tell it I'm dead.
		m_pLauncher->m_cActiveRockets--;
	}

	STOP_SOUND( edict(), CHAN_VOICE, "weapons/rocket1.wav" );
	ExplodeTouch( pOther );
}*/

//=========================================================
//=========================================================
void CDoshRocket :: Precache( void )
{
	PRECACHE_MODEL("models/doshrocket.mdl");
	m_iTrail = PRECACHE_MODEL("sprites/smoke.spr");
	PRECACHE_SOUND ("weapons/rocket1.wav");
}


void CDoshRocket :: IgniteThink( void  )
{
	// pev->movetype = MOVETYPE_TOSS;

	pev->movetype = MOVETYPE_FLY;
	pev->effects |= EF_LIGHT;

	// make rocket sound
	EMIT_SOUND( ENT(pev), CHAN_VOICE, "weapons/rocket1.wav", 1, 0.5 );

	// rocket trail
	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );

		WRITE_BYTE( TE_BEAMFOLLOW );
		WRITE_SHORT(entindex());	// entity
		WRITE_SHORT(m_iTrail );	// model
		WRITE_BYTE( 120 ); // life
		WRITE_BYTE( 15 );  // width
		WRITE_BYTE( 224 );   // r, g, b
		WRITE_BYTE( 224 );   // r, g, b
		WRITE_BYTE( 255 );   // r, g, b
		WRITE_BYTE( 255 );	// brightness

	MESSAGE_END();  // move PHS/PVS data sending into here (SEND_ALL, SEND_PVS, SEND_PHS)

	m_flIgniteTime = gpGlobals->time;

	//// set to follow laser spot
	//SetThink( &CDoshRocket::FollowThink );
	//pev->nextthink = gpGlobals->time + 0.1;
}
void CDosh::Reload( void )
{
	int iResult;

	if ( m_iClip == 1 )
	{
		// don't bother with any of this if don't need to reload.
		return;
	}

	if ( m_pPlayer->ammo_rockets <= 0 )
		return;

	// because the RPG waits to autoreload when no missiles are active while  the LTD is on, the
	// weapons code is constantly calling into this function, but is often denied because 
	// a) missiles are in flight, but the LTD is on
	// or
	// b) player is totally out of ammo and has nothing to switch to, and should be allowed to
	//    shine the designator around
	//
	// Set the next attack time into the future so that WeaponIdle will get called more often
	// than reload, allowing the RPG LTD to be updated
	
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5;

	if ( m_cActiveRockets && m_fSpotActive )
	{
		// no reloading when there are active missiles tracking the designator.
		// ward off future autoreload attempts by setting next attack time into the future for a bit. 
		return;
	}

#ifndef CLIENT_DLL
	//if ( m_pSpot && m_fSpotActive )
	{
		//m_pSpot->Suspend( 2.1 );
	//	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.1;
	}
#endif

	if ( m_iClip == 0 )
		iResult = DefaultReload( DOSH_MAX_CLIP, DOSH_RELOAD, 2 );
	
	if ( iResult )
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
	
}

void CDosh::Spawn( )
{
	Precache( );
	m_iId = WEAPON_DOSH;

	SET_MODEL(ENT(pev), "models/w_dosh.mdl");
	//m_fSpotActive = 1;

#ifdef CLIENT_DLL
	if ( bIsMultiplayer() )
#else
	if ( g_pGameRules->IsMultiplayer() )
#endif
	{
		// more default ammo in multiplay. 
		m_iDefaultAmmo = DOSH_DEFAULT_GIVE * 2;
	}
	else
	{
		m_iDefaultAmmo = DOSH_DEFAULT_GIVE;
	}

	FallInit();// get ready to fall down.
}


void CDosh::Precache( void )
{
	PRECACHE_MODEL("models/w_dosh.mdl");
	PRECACHE_MODEL("models/v_dosh.mdl");
	PRECACHE_MODEL("models/p_dosh.mdl");

	PRECACHE_SOUND("items/9mmclip1.wav");

//	UTIL_PrecacheOther( "laser_spot" );
	UTIL_PrecacheOther( "dosh_rocket" );

	PRECACHE_SOUND("weapons/rocketfire1.wav");
	PRECACHE_SOUND("weapons/glauncher.wav"); // alternative fire sound

	m_usDosh = PRECACHE_EVENT ( 1, "events/dosh.sc" );
}


int CDosh::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "dosh";
	p->iMaxAmmo1 = DOSH_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = DOSH_MAX_CLIP;
	p->iSlot = 4;
	p->iPosition = 0;
	p->iId = m_iId = WEAPON_DOSH;
	p->iFlags = 0;
	p->iWeight = DOSH_WEIGHT;

	return 1;
}

int CDosh::AddToPlayer( CBasePlayer *pPlayer )
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

BOOL CDosh::Deploy( )
{
	if ( m_iClip == 0 )
	{
		return DefaultDeploy( "models/v_dosh.mdl", "models/p_dosh.mdl", DOSH_DRAW_UL, "dosh" );
	}

	return DefaultDeploy( "models/v_dosh.mdl", "models/p_dosh.mdl", DOSH_DRAW1, "dosh" );
}


BOOL CDosh::CanHolster( void )
{
	//if ( m_fSpotActive && m_cActiveRockets )
	//{
		// can't put away while guiding a missile.
		//return FALSE;
	//}

	return TRUE;
}

void CDosh::Holster( int skiplocal /* = 0 */ )
{
	m_fInReload = FALSE;// cancel any reload in progress.

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	
	SendWeaponAnim( DOSH_HOLSTER1 );

#ifndef CLIENT_DLL
	
#endif

}



void CDosh::PrimaryAttack()
{
	if ( m_iClip )
	{
		m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

#ifndef CLIENT_DLL
		// player "shoot" animation
		m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

		UTIL_MakeVectors( m_pPlayer->pev->v_angle );
		Vector vecSrc = m_pPlayer->GetGunPosition( ) + gpGlobals->v_forward * 16 + gpGlobals->v_right * 8 + gpGlobals->v_up * -8;
		
		CDoshRocket *pRocket = CDoshRocket::CreateDoshRocket( vecSrc, m_pPlayer->pev->v_angle, m_pPlayer, this );

		UTIL_MakeVectors( m_pPlayer->pev->v_angle );// RpgRocket::Create stomps on globals, so remake.
		pRocket->pev->velocity = pRocket->pev->velocity + gpGlobals->v_forward * DotProduct( m_pPlayer->pev->velocity, gpGlobals->v_forward );
#endif

		// firing RPG no longer turns on the designator. ALT fire is a toggle switch for the LTD.
		// Ken signed up for this as a global change (sjb)

		int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

		PLAYBACK_EVENT( flags, m_pPlayer->edict(), m_usDosh );

		m_iClip--; 
				
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.5;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5;
	}
	else
	{
		PlayEmptySound( );
	}
	//UpdateSpot( );
}


void CDosh::SecondaryAttack()
{
		PlayEmptySound( );
}


void CDosh::WeaponIdle( void )
{
	//UpdateSpot( );

	ResetEmptySound( );

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	if ( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
	{
		int iAnim;
		float flRand = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 0, 1 );
		if (flRand <= 0.75 || m_fSpotActive)
		{
			if ( m_iClip == 0 )
				iAnim = DOSH_IDLE_UL;
			else
				iAnim = DOSH_IDLE;

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 90.0 / 15.0;
		}
		else
		{
			if ( m_iClip == 0 )
				iAnim = DOSH_FIDGET_UL;
			else
				iAnim = DOSH_FIDGET;

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0;
		}

		SendWeaponAnim( iAnim );
	}
	else
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1;
	}
}




class CDoshAmmo : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/w_doshammo.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/w_doshammo.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		int iGive;

#ifdef CLIENT_DLL
	if ( bIsMultiplayer() )
#else
	if ( g_pGameRules->IsMultiplayer() )
#endif
		{
			// hand out more ammo per rocket in multiplayer.
			iGive = AMMO_DOSHCLIP_GIVE * 2;
		}
		else
		{
			iGive = AMMO_DOSHCLIP_GIVE;
		}

		if (pOther->GiveAmmo( iGive, "dosh", DOSH_MAX_CARRY ) != -1)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
			return TRUE;
		}
		return FALSE;
	}
};
LINK_ENTITY_TO_CLASS( ammo_doshclip, CDoshAmmo );

#endif