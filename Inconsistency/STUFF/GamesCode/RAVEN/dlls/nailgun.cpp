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

#ifndef CLIENT_DLL
#define NAIL_AIR_VELOCITY	2000
#define NAIL_WATER_VELOCITY	1000

// UNDONE: Save/restore this?  Don't forget to set classname and LINK_ENTITY_TO_CLASS()
// 
// OVERLOADS SOME ENTVARS:
//
// speed - the ideal magnitude of my velocity
class CNail : public CBaseEntity
{
	void Spawn( void );
	void Precache( void );
	void CreateStreak( void );
	int  Classify ( void );
	void EXPORT BubbleThink( void );
	void EXPORT NailTouch( CBaseEntity *pOther );

	BOOL CanSuck( void ) { return TRUE; }

	int m_iTrail;

public:
	static CNail *NailCreate( void );
};
LINK_ENTITY_TO_CLASS( nail, CNail );

CNail *CNail::NailCreate( void )
{
	// Create a new entity with CNail private data
	CNail *pNail = GetClassPtr( (CNail *)NULL );
	pNail->pev->classname = MAKE_STRING("nail");
	pNail->Spawn();
	pNail->CreateStreak();

	return pNail;
}

void CNail::Spawn( )
{
	Precache( );
	pev->movetype = MOVETYPE_FLY;
	pev->solid = SOLID_BBOX;
	pev->renderfx = 101;

	pev->gravity = 1;

	SET_MODEL(ENT(pev), "models/spike.mdl");

	UTIL_SetOrigin( pev, pev->origin );
	UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));

	SetTouch( &CNail::NailTouch );
	SetThink( &CNail::BubbleThink );
	pev->nextthink = gpGlobals->time + 0.2;
}


void CNail::Precache( )
{
	PRECACHE_MODEL ("models/spike.mdl");
	PRECACHE_SOUND("weapons/nail_hitbod.wav");
	PRECACHE_SOUND("weapons/nail_hit1.wav");
	PRECACHE_SOUND("weapons/nail_hit2.wav");
	PRECACHE_SOUND("weapons/nail_hit3.wav");

	m_iTrail = PRECACHE_MODEL("sprites/smoke.spr");
}


int	CNail :: Classify ( void )
{
	return	CLASS_NONE;
}

void CNail :: CreateStreak ( void )
{
	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_BEAMFOLLOW );
		WRITE_SHORT( entindex() );	// entity
		WRITE_SHORT( m_iTrail );	// model
		WRITE_BYTE( 3 ); // life
		WRITE_BYTE( 1 );  // width
		WRITE_BYTE( 255 );   // r, g, b
		WRITE_BYTE( 192 );   // r, g, b
		WRITE_BYTE( 64 );   // r, g, b
		WRITE_BYTE( 255 );	// brightness
	MESSAGE_END();
}
void CNail::NailTouch( CBaseEntity *pOther )
{
	SetTouch( NULL );
	SetThink( NULL );

	if (UTIL_PointContents(pev->origin) == CONTENTS_SKY)
	{
		UTIL_Remove(this);
		return;
	}

	TraceResult tr = UTIL_GetGlobalTrace( );

	if (pOther->pev->takedamage)
	{
		
		entvars_t	*pevOwner;

		pevOwner = VARS( pev->owner );

		// UNDONE: this needs to call TraceAttack instead
		ClearMultiDamage( );
		pOther->TraceAttack( pevOwner, gSkillData.plrDmgNail, pev->velocity.Normalize(), &tr, DMG_NAIL | DMG_BULLET ); 
		ApplyMultiDamage( pev, pevOwner );

		pev->velocity = Vector( 0, 0, 0 );
		Killed( pev, GIB_NEVER );
	}
	else
	{
		switch(RANDOM_LONG(0, 2))
		{
			case 0: EMIT_SOUND_DYN(ENT(pev), CHAN_BODY, "weapons/nail_hit1.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 98 + RANDOM_LONG(0,7)); break;
			case 1: EMIT_SOUND_DYN(ENT(pev), CHAN_BODY, "weapons/nail_hit2.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 98 + RANDOM_LONG(0,7)); break;
			case 2: EMIT_SOUND_DYN(ENT(pev), CHAN_BODY, "weapons/nail_hit3.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 98 + RANDOM_LONG(0,7)); break;
		}

		if ( FClassnameIs( pOther->pev, "worldspawn" ) )
		{
			// if what we hit is static architecture, can stay around for a while.
			Vector vecDir = pev->velocity.Normalize( );
			UTIL_SetOrigin( pev, pev->origin - vecDir * 12 );
			pev->angles = UTIL_VecToAngles( vecDir );
			pev->solid = SOLID_NOT;
			pev->movetype = MOVETYPE_FLY;
			pev->velocity = Vector( 0, 0, 0 );
			pev->avelocity.z = 0;
			pev->angles.z = RANDOM_LONG(0,360);
			pev->nextthink = gpGlobals->time + 2.0;
		}

		DecalGunshot( &tr, BULLET_PLAYER_NAILS );

		SetThink( &CBaseEntity::SUB_Remove );
		pev->nextthink = gpGlobals->time;// this will get changed below if the bolt is allowed to stick in what it hit.
	}
}

void CNail::BubbleThink( void )
{
	pev->nextthink = gpGlobals->time + 0.1;

	if (pev->waterlevel == 0)
		return;

	UTIL_BubbleTrail( pev->origin - pev->velocity * 0.1, pev->origin, 1 );
}
#endif

enum nailgun_e
{
	NAILGUN_IDLE1 = 0,
	NAILGUN_LONGIDLE,
	NAILGUN_DEPLOY,
	NAILGUN_RELOAD,
	NAILGUN_FIRE1,
	NAILGUN_FIRE2,
	NAILGUN_FIRE3
};

LINK_ENTITY_TO_CLASS( weapon_nailgun, CNailgun );

void CNailgun::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_nailgun"); // hack to allow for old names
	Precache( );
	SET_MODEL(ENT(pev), "models/w_nailgun.mdl");
	m_iId = WEAPON_NAILGUN;

	m_iDefaultAmmo = NAILGUN_DEFAULT_GIVE;

	FallInit();// get ready to fall down.
}


void CNailgun::Precache( void )
{
	PRECACHE_MODEL("models/v_nailgun.mdl");
	PRECACHE_MODEL("models/w_nailgun.mdl");
	PRECACHE_MODEL("models/p_nailgun.mdl");

	PRECACHE_MODEL("models/w_nail_clip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");              

	PRECACHE_SOUND("weapons/nailgun_clipin.wav");
	PRECACHE_SOUND("weapons/nailgun_clipout.wav");
	PRECACHE_SOUND("weapons/nailgun_fire1.wav");
	PRECACHE_SOUND("weapons/nailgun_fire2.wav");
	PRECACHE_SOUND("weapons/nailgun_fire3.wav");

	PRECACHE_SOUND( "weapons/glauncher.wav" );
	PRECACHE_SOUND( "weapons/357_cock1.wav" );

	UTIL_PrecacheOther("nail");

	m_usNailgun = PRECACHE_EVENT( 1, "events/nailgun.sc" );
}

int CNailgun::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "nails";
	p->iMaxAmmo1 = NAILS_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = NAILGUN_MAX_CLIP;
	p->iSlot = 6;
	p->iPosition = 0;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_NAILGUN;
	p->iWeight = NAILGUN_WEIGHT;

	return 1;
}

int CNailgun::AddToPlayer( CBasePlayer *pPlayer )
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

BOOL CNailgun::Deploy( )
{
	return DefaultDeploy( "models/v_nailgun.mdl", "models/p_nailgun.mdl", NAILGUN_DEPLOY, "mp5" );
}

void CNailgun::PrimaryAttack()
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

	// Spawn from the lower right
	UTIL_MakeVectors( m_pPlayer->pev->v_angle );
	Vector vecSrc = m_pPlayer->GetGunPosition( ) + gpGlobals->v_right * 4 - gpGlobals->v_up * 8;

	// Direction is looking at the view's end position
	Vector vecEnd = m_pPlayer->GetGunPosition( ) + gpGlobals->v_forward * 16384;
	Vector vecShootDir = (vecEnd - vecSrc).Normalize();

	//Use player's random seed.
	float x = UTIL_SharedRandomFloat( m_pPlayer->random_seed, -0.5, 0.5 ) + UTIL_SharedRandomFloat( m_pPlayer->random_seed + 1 , -0.5, 0.5 );
	float y = UTIL_SharedRandomFloat( m_pPlayer->random_seed + 2, -0.5, 0.5 ) + UTIL_SharedRandomFloat( m_pPlayer->random_seed + 3, -0.5, 0.5 );
	float z = x * x + y * y;

	Vector vecSpread = VECTOR_CONE_2DEGREES;
	Vector vecDir = vecShootDir +
					x * vecSpread.x * gpGlobals->v_right +
					y * vecSpread.y * gpGlobals->v_up;

#ifndef CLIENT_DLL
	// Set for nail
	Vector nailAngles = UTIL_VecToAngles( vecShootDir );
	//nailAngles.x = -nailAngles.x;

	CNail *pNail = CNail::NailCreate();
	pNail->pev->origin = vecSrc;
	pNail->pev->angles = nailAngles;
	pNail->pev->owner = m_pPlayer->edict();

	if (m_pPlayer->pev->waterlevel == 3)
	{
		pNail->pev->velocity = vecDir * NAIL_WATER_VELOCITY;
		pNail->pev->speed = NAIL_WATER_VELOCITY;
	}
	else
	{
		pNail->pev->velocity = vecDir * NAIL_AIR_VELOCITY;
		pNail->pev->speed = NAIL_AIR_VELOCITY;
	}
	pNail->pev->avelocity.z = 10;
#endif

	PLAYBACK_EVENT_FULL( 0, m_pPlayer->edict(), m_usNailgun, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0 );

	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.08;

	if ( m_flNextPrimaryAttack < UTIL_WeaponTimeBase() )
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.08;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
}

void CNailgun::Reload( void )
{
	if ( m_pPlayer->ammo_nails <= 0 )
		return;

	DefaultReload( NAILGUN_MAX_CLIP, NAILGUN_RELOAD, 2.3 );
}


void CNailgun::WeaponIdle( void )
{
	ResetEmptySound( );

	m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	int iAnim;
	float flTime;
	switch ( RANDOM_LONG( 0, 2 ) )
	{
	case 0:	
	case 1:
		iAnim = NAILGUN_LONGIDLE;	
		flTime = 3.2375*RANDOM_LONG(1, 3);
		break;
	
	default:
	case 2:
		iAnim = NAILGUN_IDLE1;
		flTime = 3.32*RANDOM_LONG(1, 2);
		break;
	}

	SendWeaponAnim( iAnim );

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + flTime; // how long till we do this again.
}



class CNailgunClip : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/w_nail_clip.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/w_nail_clip.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		int bResult = (pOther->GiveAmmo( AMMO_NAIL_CLIP_GIVE, "nails", NAILS_MAX_CARRY) != -1);
		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};
LINK_ENTITY_TO_CLASS( ammo_nail_clip, CNailgunClip );
