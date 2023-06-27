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
#include "shake.h"
#include "blackhole_shared.h"

extern int gmsgBlackHole;

#define BLACK_HOLE_WALL_DISTANCE	32
#define BLACK_HOLE_LIFE				16

//====================================
//
// BLACK HOLE CODE
//
//
//====================================
LINK_ENTITY_TO_CLASS( blackhole, CBlackHole );

//=========================================================
//=========================================================
void CBlackHole::Spawn( void )
{
	Precache( );
	pev->movetype = MOVETYPE_NONE;
	pev->solid = SOLID_NOT;

	// Make it invisible
	pev->rendermode = kRenderTransAdd;
	pev->renderamt = 0;

	SET_MODEL(ENT(pev), "sprites/null.spr");
	UTIL_SetOrigin( pev, pev->origin );
};

//=========================================================
// MakeLive - Make the black hole swallow everything in sight
//=========================================================
void CBlackHole::MakeLive( float flScale )
{
	m_flScale = flScale;

	// Create the client side effect
	MESSAGE_BEGIN(MSG_ALL, gmsgBlackHole, NULL);
		WRITE_SHORT(entindex());
		WRITE_COORD(BLACK_HOLE_LIFE);
		WRITE_LONG(BLACK_HOLE_SIZE*BLACK_HOLE_SIZE*m_flScale);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z);
	MESSAGE_END();

	pev->flags |= FL_ALWAYSTHINK;

	SetThink( &CBlackHole::SuckThink );
	pev->nextthink = pev->ltime + 0.1;

	m_flDieTime = gpGlobals->time + BLACK_HOLE_LIFE;
	m_flStrength = NULL;

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "weapons/blackhole_sound.wav", 1, 0.1);
}

//=========================================================
// SuckThink - Suck everything in nearby
//=========================================================
void CBlackHole::SuckThink( void )
{
	if(m_flDieTime <= gpGlobals->time)
	{
		pev->flags &= ~FL_ALWAYSTHINK;
		pev->nextthink = gpGlobals->time + 0.1;
		STOP_SOUND(ENT(pev), CHAN_ITEM, "weapons/blackhole_sound.wav");

		SetThink( &CBaseEntity::SUB_Remove );
		return;
	}

	// Create the funnel effect
	if(m_flNextFunnel <= gpGlobals->time)
	{
		MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, pev->origin );
			WRITE_BYTE(TE_IMPLOSION);
			WRITE_COORD(pev->origin.x);
			WRITE_COORD(pev->origin.y);
			WRITE_COORD(pev->origin.z);
			WRITE_BYTE(250);  // radius
			WRITE_BYTE(RANDOM_LONG( 25, 50)); // count
			WRITE_BYTE(RANDOM_LONG( 2, 4 )); // life
		MESSAGE_END();

		m_flNextFunnel = gpGlobals->time + 0.1;
	}

	m_flStrength += BLACK_HOLE_GROWTH_RATE * gpGlobals->frametime;
	if(m_flStrength > 4)
		m_flStrength = 4;

	CBaseEntity *pObject = NULL;
	float fullRadius = BLACK_HOLE_SIZE*4*m_flScale;
	while ((pObject = UTIL_FindEntityInSphere( pObject, pev->origin, fullRadius )) != NULL)
	{
		if(!pObject->CanSuck())
			continue;

		float flDist = (pObject->pev->origin - pev->origin).Length();
		float distStrength = 1.0-(flDist/(fullRadius));
		if(distStrength < 0)
			distStrength = 0;

		float pullStrength = distStrength*BLACK_HOLE_SUCK_SPEED*m_flStrength*m_flScale;
		Vector vVelocityDir = (pev->origin - pObject->pev->origin).Normalize();
		pObject->pev->velocity = pObject->pev->velocity + vVelocityDir*pullStrength;

		if( flDist < BLACK_HOLE_SIZE/8 )
		{
			if(pObject->IsAlive() || pObject->IsPlayer())
			{
				pObject->TakeDamage( pev, pev, 50000, DMG_CRUSH );

				if(pObject->IsPlayer() && !(pObject->pev->flags & FL_GODMODE))
				{
					CBasePlayer* pPlayer = (CBasePlayer *)pObject;
					pPlayer->EnableControl(FALSE);

					UTIL_ScreenFadeAll( Vector(0, 0, 0), 0.1, 10, 255, FFADE_STAYOUT );
				}
			}
			else if( pObject->pev->movetype != MOVETYPE_FOLLOW )
				UTIL_Remove(pObject);
		}
	}

	pev->nextthink = pev->ltime + 0.1;
}

//=========================================================
// Precache - Precache the sprite
//=========================================================
void CBlackHole::Precache( void )
{
	PRECACHE_MODEL("sprites/null.spr");

	PRECACHE_SOUND("weapons/blackhole_sound.wav");
	PRECACHE_SOUND("ambience/port_suckin1.wav");
};

//=========================================================
//=========================================================
CBlackHole *CBlackHole::CreateBlackHole( void )
{
	CBlackHole *pHole = GetClassPtr( (CBlackHole *)NULL );
	pHole->Spawn();

	pHole->pev->classname = MAKE_STRING("blackhole");

	return pHole;
}


//====================================
//
// DISPLACER WEAPON CODE
//
//
//====================================

enum displacer_e {
	DISPLACER_IDLE1 = 0,
	DISPLACER_IDLE2,
	DISPLACER_SPINUP,
	DISPLACER_SPIN,
	DISPLACER_FIRE,
	DISPLACER_DRAW,
	DISPLACER_HOLSTER
};

enum disp_state_t {
	DISPLACER_STATE_IDLE = 0,
	DISPLACER_STATE_SPINUP,
	DISPLACER_STATE_SPIN,
	DISPLACER_STATE_FIRE
};

LINK_ENTITY_TO_CLASS( weapon_displacer, CDisplacer );

void CDisplacer::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_displacer"); // hack to allow for old names

	Precache( );
	m_iId = WEAPON_DISPLACER;
	SET_MODEL(ENT(pev), "models/w_displacer.mdl");

	m_iDefaultAmmo = DISPLACER_DEFAULT_GIVE;
	m_iState = DISPLACER_STATE_IDLE;
	m_flStateTime = NULL;
	m_pBlackHole = NULL;
	m_flScale = 0;

	FallInit();// get ready to fall down.
}


void CDisplacer::Precache( void )
{
	PRECACHE_MODEL("models/v_displacer.mdl");
	PRECACHE_MODEL("models/w_displacer.mdl");
	PRECACHE_MODEL("models/p_displacer.mdl");

	PRECACHE_SOUND("items/9mmclip1.wav");
	PRECACHE_SOUND("items/9mmclip2.wav");
	PRECACHE_SOUND("buttons/button11.wav");

	PRECACHE_SOUND ("weapons/displacer_fire.wav");
	PRECACHE_SOUND ("weapons/displacer_impact.wav");
	PRECACHE_SOUND ("weapons/displacer_self.wav");
	PRECACHE_SOUND ("weapons/displacer_spin.wav");
	PRECACHE_SOUND ("weapons/displacer_spin2.wav");
	PRECACHE_SOUND ("weapons/displacer_start.wav");

	PRECACHE_MODEL("sprites/lgtning.spr");
	PRECACHE_MODEL("sprites/laserbeam.spr");

	UTIL_PrecacheOther( "blackhole" );

	m_usDisplacer = PRECACHE_EVENT( 1, "events/displacer.sc" );
}

int CDisplacer::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "uranium";
	p->iMaxAmmo1 = URANIUM_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 5;
	p->iPosition = 1;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_DISPLACER;
	p->iWeight = DISPLACER_WEIGHT;

	return 1;
}

BOOL CDisplacer::Deploy( )
{
	return DefaultDeploy( "models/v_displacer.mdl", "models/p_displacer.mdl", DISPLACER_DRAW, "egon", 0 );
}

void CDisplacer::Holster( int skiplocal )
{
	m_iState = DISPLACER_STATE_IDLE;
	m_flStateTime = NULL;
	m_flScale = 0;

	if(m_pBlackHole)
		UTIL_Remove(m_pBlackHole);
}

void CDisplacer::CreateBlackHole( void )
{
	if( m_pBlackHole )
		return;

	m_pBlackHole = CBlackHole::CreateBlackHole();
}

void CDisplacer::FireBlackHole( void )
{
	m_pBlackHole->MakeLive(m_flScale);
	m_pBlackHole = NULL;

	m_flScale = 0;

	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_BODY, "ambience/port_suckin1.wav", 1, 0.1);

	m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= 20;

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = 0;
}

void CDisplacer::UpdateBlackHole( void )
{
	if( !m_pBlackHole )
		return;

	Vector vForward;
	UTIL_MakeVectorsPrivate( m_pPlayer->pev->v_angle, vForward, NULL, NULL );
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecEnd = vecSrc + vForward * 16384;

	TraceResult tr;
	UTIL_TraceLine( vecSrc, vecEnd, dont_ignore_monsters, dont_ignore_glass, ENT(m_pPlayer->pev), &tr);

	Vector vecOrigin = tr.vecEndPos - vForward * BLACK_HOLE_WALL_DISTANCE;
	UTIL_SetOrigin( m_pBlackHole->pev, vecOrigin );
}

void CDisplacer::ItemPostFrame( void )
{
	if( m_iState != DISPLACER_STATE_IDLE && m_flStateTime )
	{
		// Check if we've reached the minimum distance
		Vector vForward;
		UTIL_MakeVectorsPrivate( m_pPlayer->pev->v_angle, vForward, NULL, NULL );
		Vector vecSrc = m_pPlayer->GetGunPosition();
		Vector vecEnd = vecSrc + vForward * 16384;

		TraceResult tr;
		UTIL_TraceLine( vecSrc, vecEnd, dont_ignore_monsters, dont_ignore_glass, ENT(m_pPlayer->pev), &tr);

		if( (tr.vecEndPos - vecSrc).Length() < 128 )
		{
			EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_BODY, "buttons/button11.wav", VOL_NORM, ATTN_NORM);
			m_iState = DISPLACER_STATE_IDLE;
			m_flStateTime = 0;

			UTIL_Remove( m_pBlackHole );
			m_pBlackHole = NULL;

			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase();
			WeaponIdle();
			return;
		}

		UpdateBlackHole();
		if(m_flStateTime <= UTIL_WeaponTimeBase())
		{
			if( m_iState == DISPLACER_STATE_SPINUP || m_iState == DISPLACER_STATE_SPIN && (m_pPlayer->pev->button & IN_ATTACK) )
			{
				m_flStateTime = UTIL_WeaponTimeBase() + 0.9;
				m_iState = DISPLACER_STATE_SPIN;

				if( m_flScale < 2 )
					m_flScale += 0.25;

				CreateBlackHole();
				UpdateBlackHole();
				PLAYBACK_EVENT_FULL( 0, m_pPlayer->edict(), m_usDisplacer, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, m_iState, m_pBlackHole->entindex(), m_flScale, 0 );
			}
			else if( m_iState == DISPLACER_STATE_SPIN )
			{
				m_flStateTime = UTIL_WeaponTimeBase() + 0.9;
				m_iState = DISPLACER_STATE_FIRE;

				m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
				m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

				PLAYBACK_EVENT_FULL( 0, m_pPlayer->edict(), m_usDisplacer, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, m_iState, m_pBlackHole->entindex(), m_flScale, 0 );
				FireBlackHole();
			}
			else if( m_iState == DISPLACER_STATE_FIRE )
			{
				m_flStateTime = NULL;
				m_iState = DISPLACER_STATE_IDLE;
			}
		}
		return;
	}

	CBasePlayerWeapon::ItemPostFrame();
}

void CDisplacer::PrimaryAttack( void )
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] < 20)
	{
		EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_BODY, "buttons/button11.wav", VOL_NORM, ATTN_NORM);
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5;
		return;
	}

	// Check if we've reached the minimum distance
	Vector vForward;
	UTIL_MakeVectorsPrivate( m_pPlayer->pev->v_angle, vForward, NULL, NULL );
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecEnd = vecSrc + vForward * 16384;

	TraceResult tr;
	UTIL_TraceLine( vecSrc, vecEnd, dont_ignore_monsters, dont_ignore_glass, ENT(m_pPlayer->pev), &tr);

	if( (tr.vecEndPos - vecSrc).Length() < 128 )
	{
		EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_BODY, "buttons/button11.wav", VOL_NORM, ATTN_NORM);
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5;
		return;
	}

	// Set state
	m_flScale = 0;
	m_iState = DISPLACER_STATE_SPINUP;
	m_flStateTime = UTIL_WeaponTimeBase() + 0.9;

	PLAYBACK_EVENT_FULL( 0, m_pPlayer->edict(), m_usDisplacer, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, m_iState, m_iBeam, 0, 0 );
}

void CDisplacer::WeaponIdle( void )
{
	ResetEmptySound( );

	m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	// only idle if the slid isn't back
	int iAnim;
	float idleTime;

	switch( RANDOM_LONG(0, 1) )
	{
		case 0:
			iAnim = DISPLACER_IDLE1;
			idleTime = 3;
			break;
		case 1:
			iAnim = DISPLACER_IDLE2;
			idleTime = 3;
			break;
	}

	SendWeaponAnim( iAnim, 1 );
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + idleTime;
}














