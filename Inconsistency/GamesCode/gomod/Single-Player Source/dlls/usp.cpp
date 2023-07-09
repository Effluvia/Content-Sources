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



enum usp_e
{
	
	USP_IDLE = 0,
	USP_SHOOT1,
	USP_SHOOT2,
	USP_SHOOT3,
	USP_SHOOTLAST,
	USP_RELOAD,
	USP_DRAW,
	USP_ADD_SILENCER,
	USP_IDLE_UNSIL,
	USP_SHOOT1_UNSIL,
	USP_SHOOT2_UNSIL,
	USP_SHOOT3_UNSIL,
	USP_SHOOTLAST_UNSIL,
	USP_RELOAD_UNSIL,
	USP_DRAW_UNSIL,
	USP_DETACH_SILENCER,

};



LINK_ENTITY_TO_CLASS( weapon_usp, CUSP );


void CUSP::Spawn( )
{
	pev->friction = 0.55; // deading the bounce a bit
	pev->takedamage = DAMAGE_YES;
	pev->classname = MAKE_STRING("weapon_usp"); // hack to allow for old names
	Precache( );
	SET_MODEL(ENT(pev), "models/w_usp.mdl");
	m_iId = WEAPON_USP;

	m_iDefaultAmmo = 30;

	FallInit();// get ready to fall down.
}


void CUSP::Precache( void )
{
	
	PRECACHE_MODEL("models/v_usp.mdl");
	PRECACHE_MODEL("models/w_usp.mdl");
	PRECACHE_MODEL("models/p_usp.mdl");

	m_iShell = PRECACHE_MODEL ("models/shell.mdl");// brass shellTE_MODEL


	PRECACHE_MODEL("models/w_9mmARclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");              

	PRECACHE_SOUND("items/clipinsert1.wav");
	PRECACHE_SOUND("items/cliprelease1.wav");

	PRECACHE_SOUND ("weapons/usp1.wav");// H to the K
	PRECACHE_SOUND ("weapons/usp_unsil-1.wav");// H to the K
	PRECACHE_SOUND ("weapons/usp2.wav");// H to the K



	PRECACHE_SOUND ("weapons/357_cock1.wav");

	m_usp = PRECACHE_EVENT( 1, "events/usp.sc" );
	m_usp_unsil = PRECACHE_EVENT( 1, "events/usp_unsil.sc" );

}

int CUSP::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "ammo_USP";
	p->iMaxAmmo1 = 999;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = 12;
	p->iSlot = 6;
	p->iPosition = 2;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_USP;
	p->iWeight = 15;

	return 1;
}

int CUSP::AddToPlayer( CBasePlayer *pPlayer )
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

BOOL CUSP::Deploy( )
{
	if(m_silencer == 0)
	{
		return DefaultDeploy( "models/v_usp.mdl", "models/p_usp.mdl", USP_DRAW_UNSIL, "mp5" );
	}
	else
	{
		return DefaultDeploy( "models/v_usp.mdl", "models/p_usp.mdl", USP_DRAW, "mp5" );
	}
}


void CUSP::PrimaryAttack()
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

		vecDir = m_pPlayer->FireBulletsPlayer( 1, vecSrc, vecAiming, VECTOR_CONE_6DEGREES, 8192, BULLET_PLAYER_USP, 2, 0, m_pPlayer->pev, m_pPlayer->random_seed );
	}
	else
	{
	
			// single player spread
	vecDir = m_pPlayer->FireBulletsPlayer( 1, vecSrc, vecAiming, VECTOR_CONE_4DEGREES, 8192, BULLET_PLAYER_USP, 2, 0, m_pPlayer->pev, m_pPlayer->random_seed );
			switch (RANDOM_LONG(0,1))
    {
 	 case 0: m_pPlayer->pev->punchangle.y -= RANDOM_FLOAT(0.3,1); break;
 	 case 1: m_pPlayer->pev->punchangle.y += RANDOM_FLOAT(0.3,1); break;
    }
    switch (RANDOM_LONG(0,1))
    {
 	 case 0: m_pPlayer->pev->punchangle.x -= RANDOM_FLOAT(0.3,1); break;
 	 case 1: m_pPlayer->pev->punchangle.x += RANDOM_FLOAT(0.3,1); break;
    }
			}



  int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	if(m_silencer == 0)
	{
			PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usp_unsil, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0 );
	}
	else
	{
			PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usp, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0 );
	}

	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15;

	if ( m_flNextPrimaryAttack < UTIL_WeaponTimeBase() )
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
}


//Here comes the interesting part... XD

void CUSP::SecondaryAttack()
{

	if(m_silencer == 0)
	{
		m_silencer = 1; //Silencer Atach
		SendWeaponAnim( USP_ADD_SILENCER );
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.3;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 2.3;
	}
	else
	{
		m_silencer = 0; //Silencer Detach
		SendWeaponAnim( USP_DETACH_SILENCER );
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.3;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 2.3;
	}



}

void CUSP::Reload( void )
{

	if(m_silencer == 0)
	{
		DefaultReload( 12, USP_RELOAD_UNSIL, 2.0 );
	}
	else
	{
		DefaultReload( 12, USP_RELOAD, 2.0 );
	}

}


void CUSP::WeaponIdle( void )
{
	ResetEmptySound( );

	m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	int iAnim;
	if(m_silencer == 0)
	{
		iAnim = USP_IDLE_UNSIL;
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 0;
	}
	else
	{
		iAnim = USP_IDLE;
	}
			
	
	

	SendWeaponAnim( iAnim );

	m_flTimeWeaponIdle = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 ); // how long till we do this again.
}

/***************************************
Nuevas Physics Para la USP
Reaccionan a explosiones etc
***************************************/
float CUSP :: DamageForce( float damage )
{ 
	float force = damage * ((32 * 32 * 72.0) / (pev->size.x * pev->size.y * pev->size.z)) * 5;
	
	if ( force > 1000.0) 
	{
		force = 1000.0;
	}

	return force;
}

int CUSP :: TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
	float	flTake;
	Vector	vecDir;

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


class CUSPAmmoClip : public CBasePlayerAmmo
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
		int bResult = (pOther->GiveAmmo( HKMP5_MAX_CARRY, "ammo_USP", HKMP5_MAX_CARRY) != -1);
		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};
LINK_ENTITY_TO_CLASS( ammo_USP, CUSPAmmoClip );



/*
class CM4A1Chainammo : public CBasePlayerAmmo
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
		int bResult = (pOther->GiveAmmo( AMMO_CHAINBOX_GIVE, "ammo_M4A1", HKMP5_MAX_CARRY) != -1);
		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};

//LINK_ENTITY_TO_CLASS( ammo_M4A1, CM4A1Chainammo );

*/



















