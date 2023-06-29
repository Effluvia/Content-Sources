#include "cbase.h"
#include "baseanimating.h"
#include "fmtstr.h"
#include "model_types.h"
#include "sprite.h"
#include "datacache/imdlcache.h"
#include "gib.h"
#include "func_break.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CBushProp : public CBaseEntity
{
public:
	DECLARE_CLASS( CBushProp, CBaseEntity );
	DECLARE_DATADESC();

	void Spawn();
	void Precache();
	void Touch( CBaseEntity *pOther );
	int OnTakeDamage( const CTakeDamageInfo &info );
	void Event_Killed( const CTakeDamageInfo &info );
	void UpdateOnRemove( void );

	void DropLeaf( Vector &vecOrigin );

	float m_flLastTouched;
	int m_iModelNum;
	CHandle<CSprite> m_hSprite;
};

LINK_ENTITY_TO_CLASS( prop_bush, CBushProp );

BEGIN_DATADESC( CBushProp )
	DEFINE_KEYFIELD( m_iModelNum, FIELD_INTEGER, "modelnum" ),
	DEFINE_FIELD( m_flLastTouched, FIELD_TIME ),
	DEFINE_FIELD( m_hSprite, FIELD_EHANDLE ),
END_DATADESC()

// see monstermaker.cpp
static void DispatchActivate( CBaseEntity *pEntity )
{
	bool bAsyncAnims = mdlcache->SetAsyncLoad( MDLCACHE_ANIMBLOCK, false );
	pEntity->Activate();
	mdlcache->SetAsyncLoad( MDLCACHE_ANIMBLOCK, bAsyncAnims );
}

//-----------------------------------------------------------------------------
void CBushProp::Spawn( void )
{
	BaseClass::Spawn();

	Precache();

#if 1
	m_hSprite = (CSprite *) CreateEntityByName( "env_sprite" );
	m_hSprite->KeyValue( "model", STRING( GetModelName() ) );
	m_hSprite->KeyValue( "scale", "1.0" );
	m_hSprite->KeyValue( "rendermode", "2" );
	m_hSprite->SetAbsOrigin( GetAbsOrigin() );
	DispatchSpawn( m_hSprite );
	DispatchActivate( m_hSprite );
#else
	m_hSprite = CSprite::SpriteCreate( STRING( GetModelName() ), GetAbsOrigin(), true );
	m_hSprite->SetRenderMode( kRenderTransTexture );
#endif
	SetModelName( NULL_STRING );

	UTIL_SetSize( this, Vector(-24, -24, 0), Vector(24, 24, 64) );
	SetSolid( SOLID_BBOX );
	AddSolidFlags( FSOLID_TRIGGER | FSOLID_NOT_SOLID );

	m_takedamage = DAMAGE_YES;

	m_flLastTouched = gpGlobals->curtime;
}

//-----------------------------------------------------------------------------
void CBushProp::Precache( void )
{
	if (m_iModelNum == -1)
	{
		m_iModelNum = random->RandomInt(0, 2);
	}
	CFmtStr fmt;
	fmt.sprintf("sprites/bush%d.spr", m_iModelNum + 1);
	SetModelName( AllocPooledString( fmt ) );
	PrecacheModel( STRING( GetModelName() ) );

	PrecacheModel( "models/leafgibs/leafgibs.mdl" );

	PrecacheScriptSound( "Bush.Rustle" );
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
void CBushProp::Touch( CBaseEntity *pOther )
{
	if ( pOther->GetAbsVelocity() == vec3_origin )
		return;

	if ( m_flLastTouched > gpGlobals->curtime - 1.0 )
		return;
	m_flLastTouched = gpGlobals->curtime;

	EmitSound( "Bush.Rustle" );
}

//-----------------------------------------------------------------------------
int CBushProp::OnTakeDamage( const CTakeDamageInfo &info )
{
	int leaves = min(info.GetDamage() / 10, 10);
	for ( int i = 0; i < leaves; i++ )
	{
		Vector vecOrigin = WorldSpaceCenter();
		vecOrigin.x += random->RandomInt(-28,28);
		vecOrigin.y += random->RandomInt(-28,28);
		vecOrigin.z += random->RandomInt(-28,28);
		DropLeaf( vecOrigin );
	}

	if ( !(info.GetDamageType() & DMG_BLAST) )
		return 0;

	return BaseClass::OnTakeDamage( info );
}

//-----------------------------------------------------------------------------
void CBushProp::Event_Killed( const CTakeDamageInfo &info )
{
	for ( int i = 0; i < 20; i++ )
	{
		Vector vecOrigin = WorldSpaceCenter();
		vecOrigin.x += random->RandomInt(-28,28);
		vecOrigin.y += random->RandomInt(-28,28);
		vecOrigin.z += random->RandomInt(-28,28);
		DropLeaf( vecOrigin );
	}

	BaseClass::Event_Killed( info );
}

//-----------------------------------------------------------------------------
void CBushProp::UpdateOnRemove( void )
{
	UTIL_Remove( m_hSprite );
	BaseClass::UpdateOnRemove();
}

//-----------------------------------------------------------------------------
void CBushProp::DropLeaf( Vector &vecOrigin )
{
	CGib *pGib = CREATE_ENTITY( CGib, "gib" );
	pGib->Spawn( "models/leafgibs/leafgibs.mdl" );
	pGib->m_nBody = random->RandomInt( 0, 4 );
	pGib->SetBloodColor( DONT_BLEED );
	pGib->m_material = matNone;
	pGib->m_lifeTime = 10;
//	pGib->SetGravity( UTIL_ScaleForGravity(200) );
	pGib->AddEffects( EF_NOSHADOW );

	pGib->SetLocalOrigin( vecOrigin);

	Vector vecVel;
	vecVel.x = random->RandomFloat(-0.25,0.25);
	vecVel.y = random->RandomFloat(-0.25,0.25);
	vecVel.z = 0.25;
	vecVel *= random->RandomFloat(300,400) * 2;
	pGib->SetAbsVelocity( vecVel );

	QAngle angVel( random->RandomFloat( 100, 200 ), random->RandomFloat( 100, 300 ), 0 );
	pGib->SetLocalAngularVelocity( angVel );

	pGib->SetNextThink( gpGlobals->curtime + pGib->m_lifeTime );
	pGib->SetThink ( &CGib::SUB_Remove );
}