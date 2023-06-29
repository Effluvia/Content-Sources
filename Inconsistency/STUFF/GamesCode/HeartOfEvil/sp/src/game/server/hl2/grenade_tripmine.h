//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef GRENADE_TRIPMINE_H
#define GRENADE_TRIPMINE_H
#ifdef _WIN32
#pragma once
#endif

#ifdef HOE_DLL
#ifdef CLIENT_DLL
#include "c_basecombatcharacter.h"
#else
#include "basecombatcharacter.h"
#endif
#else
#include "basegrenade_shared.h"
#endif

class CBeam;

#ifdef HOE_DLL
#define BASEGRENADE_EXPLOSION_VOLUME	1024

class CTripmineGrenade : public CBaseCombatCharacter
{
public:
	DECLARE_CLASS( CTripmineGrenade, CBaseCombatCharacter );
#else
class CTripmineGrenade : public CBaseGrenade
{
public:
	DECLARE_CLASS( CTripmineGrenade, CBaseGrenade );
#endif

	CTripmineGrenade();
	void Spawn( void );
	void Precache( void );

#ifdef HOE_DLL
	virtual void		Explode( trace_t *pTrace, int bitsDamageType );
	virtual Vector		GetBlastForce() { return vec3_origin; }
#endif

	int OnTakeDamage_Alive( const CTakeDamageInfo &info );
	
	void WarningThink( void );
	void PowerupThink( void );
	void BeamBreakThink( void );
	void DelayDeathThink( void );
	void Event_Killed( const CTakeDamageInfo &info );

	void MakeBeam( void );
	void KillBeam( void );

public:
	EHANDLE		m_hOwner;

private:
	float		m_flPowerUp;
	Vector		m_vecDir;
	Vector		m_vecEnd;
	float		m_flBeamLength;

	CBeam		*m_pBeam;
	Vector		m_posOwner;
	Vector		m_angleOwner;

#ifdef HOE_DLL
	CNetworkVar( bool, m_bIsLive );					// Is this grenade live, or can it be picked up?
	CNetworkVar( float, m_DmgRadius );				// How far do I do damage?
	CNetworkVar( float, m_flDamage );		// Damage to inflict.
#endif

	DECLARE_DATADESC();
};

#endif // GRENADE_TRIPMINE_H
