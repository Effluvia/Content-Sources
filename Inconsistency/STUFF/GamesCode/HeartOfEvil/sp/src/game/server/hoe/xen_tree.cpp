/* Cheap tree that doesn't attack */

#include "cbase.h"
#include "baseanimating.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CXenTree : public CBaseAnimating
{
public:
	DECLARE_CLASS( CXenTree, CBaseAnimating );
	DECLARE_DATADESC();

	void Spawn( void );
	void Precache( void );
	void Think(void);
};

BEGIN_DATADESC( CXenTree )
	DEFINE_THINKFUNC( Think ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( xen_tree, CXenTree );

//-----------------------------------------------------------------------------
void CXenTree::Spawn( void )
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
	SetThink( &CXenTree::Think );
}

//-----------------------------------------------------------------------------
void CXenTree::Precache( void )
{
	SetModelName( AllocPooledString( "models/tree.mdl" ) );
	PrecacheModel( STRING( GetModelName() ) );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
void CXenTree::Think( void )
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
