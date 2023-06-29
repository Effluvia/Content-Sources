//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		Projectile shot by bullsquid 
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================//

#ifndef	GRENADESPIT_BSQUID_H
#define	GRENADESPIT_BSQUID_H

#include "basegrenade_shared.h"

class CParticleSystem;

enum SpitSize_e
{
	SPIT_SMALL,
	SPIT_MEDIUM,
	SPIT_LARGE,
};

#define SPIT_GRAVITY 600

class CGrenadeSpitBullsquid : public CBaseGrenade
{
	DECLARE_CLASS( CGrenadeSpitBullsquid, CBaseGrenade );

public:
						CGrenadeSpitBullsquid( void );

	virtual void		Spawn( void );
	virtual void		Precache( void );
	virtual void		Event_Killed( const CTakeDamageInfo &info );

	virtual	unsigned int	PhysicsSolidMaskForEntity( void ) const { return ( BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_WATER ) & ~CONTENTS_GRATE; }

	void				SpitThink( void );
	void 				GrenadeSpitTouch( CBaseEntity *pOther );
	void				SetSpitSize( int nSize );
	void				Detonate( void );

private:
	DECLARE_DATADESC();
		
	CHandle< CParticleSystem >	m_hSpitEffect;
	Vector				m_vecPrevPosition;
};

#endif	//GRENADESPIT_BSQUID_H
