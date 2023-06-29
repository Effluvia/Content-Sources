#include "cbase.h"
#include "baseanimating.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CXenHair : public CBaseAnimating
{
public:
	DECLARE_CLASS( CXenHair, CBaseAnimating );
	DECLARE_DATADESC();

	void Spawn( void );
	void Precache( void );
	void Think(void);
};

BEGIN_DATADESC( CXenHair )
	DEFINE_THINKFUNC( Think ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( xen_hair, CXenHair );

//-----------------------------------------------------------------------------
void CXenHair::Spawn( void )
{
	BaseClass::Spawn();

	Precache();
	SetModel( STRING( GetModelName() ) );

	SetSolid( SOLID_NONE );
	SetMoveType( MOVETYPE_NONE );

	ResetSequenceInfo();

	SetCycle( random->RandomFloat( 0, 1.0 ) );
	SetPlaybackRate( random->RandomFloat( 0.7, 1.4 ));

	SetNextThink( gpGlobals->curtime + 0.1f );
	SetThink( &CXenHair::Think );
}

//-----------------------------------------------------------------------------
void CXenHair::Precache( void )
{
	SetModelName( AllocPooledString( "models/hair.mdl" ) );
	PrecacheModel( STRING( GetModelName() ) );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
void CXenHair::Think( void )
{
	if ( !UTIL_FindClientInPVS( edict() ) )
	{
		SetNextThink( gpGlobals->curtime + random->RandomFloat( 1.0f, 1.5f ) );
		return;
	}

	StudioFrameAdvance();
	DispatchAnimEvents(this);

	SetNextThink( gpGlobals->curtime + 0.1f );
}
