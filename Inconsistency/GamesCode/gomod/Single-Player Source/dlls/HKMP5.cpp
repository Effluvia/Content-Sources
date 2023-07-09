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



enum hkmp5_e
{
	
	HKMP5_IDLE1 = 0,
	HKMP5_RELOAD,
	HKMP5_DRAW,
	HKMP5_SHOOT1,
	HKMP5_SHOOT2,
	HKMP5_SHOOT3,
};



LINK_ENTITY_TO_CLASS( weapon_hkmp5, CHKMP5 );


void CHKMP5::Spawn( )
{
	pev->friction = 0.55; // deading the bounce a bit
	pev->takedamage = DAMAGE_YES;
	pev->classname = MAKE_STRING("weapon_hkmp5"); // hack to allow for old names
	Precache( );
	SET_MODEL(ENT(pev), "models/w_mp5.mdl");
	m_iId = WEAPON_HKMP5;

	m_iDefaultAmmo = HKMP5_DEFAULT_GIVE;

	FallInit();// get ready to fall down.
}


void CHKMP5::Precache( void )
{
	
	PRECACHE_MODEL("models/v_mp5.mdl");
	PRECACHE_MODEL("models/w_mp5.mdl");
	PRECACHE_MODEL("models/p_mp5.mdl");

	m_iShell = PRECACHE_MODEL ("models/shell.mdl");// brass shellTE_MODEL


	PRECACHE_MODEL("models/w_9mmARclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");              

	PRECACHE_SOUND("items/clipinsert1.wav");
	PRECACHE_SOUND("items/cliprelease1.wav");

	PRECACHE_SOUND ("weapons/mp5-1.wav");// H to the K
	PRECACHE_SOUND ("weapons/mp5-1.wav");// H to the K

	
	PRECACHE_SOUND ("weapons/mp5-1slow.wav");// H to the K
	PRECACHE_SOUND ("weapons/mp5-1slow.wav");// H to the K



	PRECACHE_SOUND ("weapons/357_cock1.wav");

	m_hkMP5 = PRECACHE_EVENT( 1, "events/hkmp5.sc" );

}

int CHKMP5::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "ammo_HKMP5";
	p->iMaxAmmo1 = HKMP5_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = HKMP5_MAX_CLIP;
	p->iSlot = 5;
	p->iPosition = 0;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_HKMP5;
	p->iWeight = HKMP5_WEIGHT;

	return 1;
}

int CHKMP5::AddToPlayer( CBasePlayer *pPlayer )
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

BOOL CHKMP5::Deploy( )
{
	return DefaultDeploy( "models/v_mp5.mdl", "models/p_mp5.mdl", HKMP5_DRAW, "hkmp5" );
}


void CHKMP5::PrimaryAttack()
{
	// don't fire underwater
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound( );
		m_flNextPrimaryAttack = 0.15;
		return;
	}

	if (m_iClip <= 0)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = 0.15;
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

		vecDir = m_pPlayer->FireBulletsPlayer( 1, vecSrc, vecAiming, VECTOR_CONE_6DEGREES, 8192, BULLET_PLAYER_HKMP5, 2, 0, m_pPlayer->pev, m_pPlayer->random_seed );
	}
	else
	{
	
			// single player spread
	vecDir = m_pPlayer->FireBulletsPlayer( 1, vecSrc, vecAiming, VECTOR_CONE_3DEGREES, 8192, BULLET_PLAYER_HKMP5, 2, 0, m_pPlayer->pev, m_pPlayer->random_seed );

	}



  int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_hkMP5, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0 );

	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.08;

	if ( m_flNextPrimaryAttack < UTIL_WeaponTimeBase() )
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.08;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
}



void CHKMP5::Reload( void )
{

DefaultReload( HKMP5_MAX_CLIP, HKMP5_RELOAD, 1.5 );
	
}


void CHKMP5::WeaponIdle( void )
{
	ResetEmptySound( );

	m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	int iAnim;
	switch ( RANDOM_LONG( 0, 1 ) )
	{
	case 0:	
		iAnim = HKMP5_IDLE1;

			m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 0;
		
		break;
	
	default:
	case 1:
		iAnim = HKMP5_IDLE1;
			m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 0;
		
		break;
	}

	SendWeaponAnim( iAnim );

	m_flTimeWeaponIdle = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 ); // how long till we do this again.
}

/***************************************
Nuevas Physics Para la HKMP5
Reaccionan a explosiones etc
***************************************/
float CHKMP5 :: DamageForce( float damage )
{ 
	float force = damage * ((32 * 32 * 72.0) / (pev->size.x * pev->size.y * pev->size.z)) * 5;
	
	if ( force > 1000.0) 
	{
		force = 1000.0;
	}

	return force;
}

int CHKMP5 :: TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
	float	flTake;
	Vector	vecDir;
	Vector Angles = pev->angles; 

	if (!pev->takedamage)
		return 0;

	//!!!LATER - make armor consideration here!
	flTake = flDamage;

	// set damage type sustained
	m_bitsDamageType |= bitsDamageType;
	
	// grab the vector of the incoming attack. ( pretend that the inflictor is a little lower than it really is, so the body will tend to fly upward a bit).
	vecDir = Vector( 0, 0, 0 );
	if (!FNullEnt( pevInflictor ))
	{
		CBaseEntity *pInflictor = CBaseEntity :: Instance( pevInflictor );
		if (pInflictor)
		{
				
			vecDir = ( pInflictor->Center() - Vector ( 0, 0, 10 ) - Center() ).Normalize();
		//	vecDir = g_vecAttackDir = vecDir.Normalize();
			pev->velocity = pev->velocity + vecDir * -DamageForce(flDamage);
				Vector savedangles = Vector( 0, 0, 0 );
	TraceResult tr;		
	UTIL_TraceLine( pev->origin, pev->origin - Vector(0,0,64), ignore_monsters, edict(), &tr );
		Vector forward, right, angdir, angdiry;
				Vector Angles = pev->angles;
				UTIL_MakeVectorsPrivate( Angles, forward, right, NULL );
				angdir = forward;
				Vector left = -right;
				angdiry = left;

				pev->avelocity.y = RANDOM_FLOAT( 400, 1000 );
				pev->avelocity.x = RANDOM_FLOAT( -100, -500 );
				pev->angles.x = UTIL_VecToAngles( angdir - DotProduct(angdir, tr.vecPlaneNormal) * tr.vecPlaneNormal).x;
				pev->angles.y = UTIL_VecToAngles( angdir - DotProduct(angdir, tr.vecPlaneNormal) * tr.vecPlaneNormal).y;
				pev->angles.z = UTIL_VecToAngles( angdiry - DotProduct(angdiry, tr.vecPlaneNormal) * tr.vecPlaneNormal).x;
			
 // play bounce sound
int pitch = 95 + RANDOM_LONG(0,29);

if (RANDOM_LONG(0,1))
EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "items/weapondrop1.wav", 1, ATTN_NORM, 0, pitch);
else
EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "items/weapondrop2.wav", 1, ATTN_NORM, 0, pitch); 


		}

	}


	// add to the damage total for clients, which will be sent as a single
	// message at the end of the frame
	// todo: remove after combining shotgun blasts?
	if ( IsPlayer() )
	{
		if ( pevInflictor )
			pev->dmg_inflictor = ENT(pevInflictor);

		pev->dmg_take += flTake;

		// check for godmode or invincibility
		if ( pev->flags & FL_GODMODE )
		{
			return 0;
		}
	}


	pev->health -= flTake;

	
	return 1;
}















void CHKMP5::BounceCode() 
{ 
if (pev->flags & FL_ONGROUND) 
{ 
// add a bit of static friction 
pev->velocity = pev->velocity * 0.5; 

Vector savedangles = Vector( 0, 0, 0 ); 
int negate = 0; 
TraceResult tr; 

UTIL_TraceLine( pev->origin, pev->origin - Vector(0,0,64), ignore_monsters, edict(), &tr ); 

	for( int i = 0; i<3; i++ ) 
	{ 
	if( pev->angles.x < 0 ) 
	negate = 1; 
	} 

#ifndef M_PI 
#define M_PI 3.14159265358979 
#endif 
#define ang2rad (2 * M_PI / 360) 

	if ( tr.flFraction < 1.0 ) 
	{ 
	Vector forward, right, angdir, angdiry; 
	Vector Angles = pev->angles; 
	//Fooz 
	NormalizeAngles( Angles ); 

	UTIL_MakeVectorsPrivate( Angles, forward, right, NULL ); 
	angdir = forward; 
	Vector left = -right; 
	angdiry = left; 

 
	pev->angles.x = UTIL_VecToAngles( angdir - DotProduct(angdir, tr.vecPlaneNormal) * tr.vecPlaneNormal).x; 
	pev->angles.y = UTIL_VecToAngles( angdir - DotProduct(angdir, tr.vecPlaneNormal) * tr.vecPlaneNormal).y; 
	pev->angles.z = UTIL_VecToAngles( angdiry - DotProduct(angdiry, tr.vecPlaneNormal) * tr.vecPlaneNormal).x; 
	
	} 

#undef ang2rad 

if( negate ) 
pev->angles.x -= savedangles.x; 
else 
pev->angles.x += savedangles.x; 

pev->avelocity = Vector ( 0, RANDOM_FLOAT( 50, 100 ), 0 ); 
} 

pev->nextthink = gpGlobals->time + 0.1;
} 

void CHKMP5 :: NormalizeAngles( float *angles ) 
{ 
int i;
 
for ( i = 0; i < 3; i++ ) 
{ 
if ( angles[i] > 180.0 ) 
{ 
angles[i] -= 360.0; 
} 
else if ( angles[i] < -180.0 ) 
{ 
angles[i] += 360.0; 
} 
} 
}

class CHKMP5AmmoClip : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/w_9mmARclip.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/w_9mmARclip.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		int bResult = (pOther->GiveAmmo( HKMP5_MAX_CARRY, "ammo_HKMP5", HKMP5_MAX_CARRY) != -1);
		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};
LINK_ENTITY_TO_CLASS( ammo_HKMP5, CHKMP5AmmoClip );




class CHKMP5Chainammo : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/w_chainammo.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/w_chainammo.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		int bResult = (pOther->GiveAmmo( AMMO_CHAINBOX_GIVE, "ammo_HKMP5", HKMP5_MAX_CARRY) != -1);
		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};
LINK_ENTITY_TO_CLASS( ammo_hkmp52, CHKMP5Chainammo );





















