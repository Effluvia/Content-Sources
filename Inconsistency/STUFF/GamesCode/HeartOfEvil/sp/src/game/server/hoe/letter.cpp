#include "cbase.h"
#include "hl2_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CLetter : public CBaseEntity
{
public:
	DECLARE_CLASS( CLetter, CBaseEntity );
	DECLARE_DATADESC();

	void Spawn( void )
	{
		BaseClass::Spawn();

		SetSolid( SOLID_BSP );
		SetMoveType( MOVETYPE_NONE );
		SetModel( STRING( GetModelName() ) );

		SetUse( &CLetter::LetterUse );
		SetTouch( NULL );

		CreateVPhysics();
	}

	int ObjectCaps( void )
	{
		return (BaseClass::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_IMPULSE_USE;
	}

	bool CreateVPhysics()
	{
		VPhysicsInitShadow( false, false );
		return true;
	}

	void LetterUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	string_t m_LetterName;
};
LINK_ENTITY_TO_CLASS( func_letter, CLetter );

BEGIN_DATADESC( CLetter )
	DEFINE_KEYFIELD( m_LetterName, FIELD_STRING, "LetterName" ),
	DEFINE_USEFUNC( LetterUse ),
END_DATADESC()

//-----------------------------------------------------------------------------
void CLetter::LetterUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	CHL2_Player *pPlayer = dynamic_cast<CHL2_Player*>(pActivator);

	if ( pPlayer != NULL && m_LetterName != NULL_STRING )
	{
		pPlayer->AddLetter( m_LetterName );

		engine->ClientCommand( pPlayer->edict(), "+showscores" );

		SetUse( NULL );

		SetThink( &CLetter::SUB_Remove );
		SetNextThink( gpGlobals->curtime + 0.1f );
	}
}