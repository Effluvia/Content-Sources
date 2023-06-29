#include "cbase.h"
#include "baseanimating.h"
#include "func_break.h"
#include "gib.h"
#include "model_types.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CJungle : public CBaseEntity
{
public:
	DECLARE_CLASS( CJungle, CBaseEntity );
	DECLARE_DATADESC();

	void Spawn();
	void Precache();
	void Touch( CBaseEntity *pOther );
	int OnTakeDamage( const CTakeDamageInfo &info );
	void TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr);
	bool KeyValue( const char *szKeyName, const char *szValue );

	void DropLeaf( Vector &vecOrigin );

	float m_flLastTouched;
	int m_iLeafGib;
};

LINK_ENTITY_TO_CLASS( func_jungle, CJungle );

BEGIN_DATADESC( CJungle )
	DEFINE_FIELD(m_flLastTouched, FIELD_TIME),
END_DATADESC()

//-----------------------------------------------------------------------------
void CJungle::Spawn( void )
{
	BaseClass::Spawn();

	Precache();

	SetSolid( SOLID_BSP );
	SetMoveType( MOVETYPE_NONE );
	SetModel( STRING( GetModelName() ) ); // brush model

	m_takedamage = DAMAGE_YES;

	m_flLastTouched = gpGlobals->curtime;
}

//-----------------------------------------------------------------------------
void CJungle::Precache( void )
{
	PrecacheModel( "models/leafgibs/leafgibs.mdl" );
	PrecacheScriptSound( "Bush.Rustle" );
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
bool CJungle::KeyValue( const char *szKeyName, const char *szValue )
{
	if ( FStrEq( szKeyName, "leafgib" ) )
	{
		m_iLeafGib = Q_atoi( szValue );
		return true;
	}
	return BaseClass::KeyValue( szKeyName, szValue );
}

//-----------------------------------------------------------------------------
void CJungle::Touch( CBaseEntity *pOther )
{
	if ( pOther->GetAbsVelocity() == vec3_origin )
		return;

	if ( m_flLastTouched > gpGlobals->curtime - 1.0 )
		return;
	m_flLastTouched = gpGlobals->curtime;

	EmitSound( "Bush.Rustle" );
}

//-----------------------------------------------------------------------------
void CJungle::TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr )
{
	for ( int i = 0; i < info.GetDamage() / 10; i++ )
		DropLeaf( ptr->endpos + Vector(random->RandomInt(-20,20),random->RandomInt(-20,20),-8) );
}

//-----------------------------------------------------------------------------
void CJungle::DropLeaf( Vector &vecOrigin )
{
	return; // These can get stuck in the jungle itself, looping sound

	CGib *pGib = CREATE_ENTITY( CGib, "gib" );
	pGib->Spawn( "models/leafgibs/leafgibs.mdl" );
	pGib->m_nBody = random->RandomInt( 0, 4 );
	pGib->SetBloodColor( DONT_BLEED );
	pGib->m_material = matNone;
	pGib->m_lifeTime = 10;
	pGib->SetGravity( UTIL_ScaleForGravity(200) );
	pGib->AddEffects( EF_NOSHADOW );

	pGib->SetLocalOrigin( vecOrigin);
//	pGib->SetAbsVelocity( Vector( );

	QAngle angVel( random->RandomFloat ( 100, 200 ), random->RandomFloat ( 100, 300 ), 0 );
	pGib->SetLocalAngularVelocity( angVel );

	pGib->SetNextThink( gpGlobals->curtime + pGib->m_lifeTime );
	pGib->SetThink ( &CGib::SUB_Remove );
}

//-----------------------------------------------------------------------------
int CJungle::OnTakeDamage( const CTakeDamageInfo &info )
{
	return 0;
}
