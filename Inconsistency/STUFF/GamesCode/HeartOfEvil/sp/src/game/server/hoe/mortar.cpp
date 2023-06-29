#include "cbase.h"
#include "beam_shared.h"
#include "explode.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define TRAIL_LIFETIME 1.0f
#define TRAIL_WIDTH 8.0f

class CMortarField : public CBaseEntity
{
public:
	DECLARE_CLASS( CMortarField, CBaseEntity );
	DECLARE_DATADESC();

	void Spawn( void );
	void Precache( void );
	void InputShoot( inputdata_t &inputdata );

	void ShootThink( void );
	void FadeThink( void );

	CHandle< CBeam > m_hBeam;
	float m_flFadeTime;
	int m_iFireballSprite;
};

LINK_ENTITY_TO_CLASS( func_mortar_field, CMortarField );

BEGIN_DATADESC( CMortarField )
	DEFINE_FIELD( m_hBeam, FIELD_EHANDLE ),
	DEFINE_FIELD( m_flFadeTime, FIELD_TIME ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Shoot", InputShoot ),
	DEFINE_THINKFUNC( ShootThink ),
	DEFINE_THINKFUNC( FadeThink ),
END_DATADESC()

void CMortarField::Spawn( void )
{
	Precache();

	BaseClass::Spawn();

	SetSolid( SOLID_BSP );	
	AddSolidFlags( FSOLID_NOT_SOLID );
	SetMoveType( MOVETYPE_NONE );
	SetModel( STRING( GetModelName() ) ); // so CollisionProp() gives us its bounds
	AddEffects( EF_NODRAW );
}

void CMortarField::Precache( void )
{
	PrecacheScriptSound( "MortarField.Incoming" );
	PrecacheScriptSound( "MortarField.Explode" );

	m_iFireballSprite = PrecacheModel( "sprites/zerogxplode.vmt" );

	BaseClass::Precache();
}

void CMortarField::InputShoot( inputdata_t &inputdata )
{
	if ( m_hBeam != NULL )
		return;
#if 1
	EmitSound( "MortarField.Incoming" );
#else
	Vector vecStart = CollisionProp()->WorldSpaceCenter();

	trace_t tr;
	UTIL_TraceLine( vecStart, vecStart - Vector(0, 0, 4096), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr );
	EmitAmbientSound( entindex(), tr.endpos + Vector(0, 0, 512), "MortarField.Incoming" );
#endif
	SetThink( &CMortarField::ShootThink );
	SetNextThink( gpGlobals->curtime + 2.5f ); // HL1 waits 2.5 seconds before exploding
}

void CMortarField::ShootThink( void )
{
	Vector vecSurroundMins, vecSurroundMaxs;
	CollisionProp()->WorldSpaceSurroundingBounds( &vecSurroundMins, &vecSurroundMaxs );

	Vector vecStart;
	vecStart.x = random->RandomFloat( vecSurroundMins.x, vecSurroundMaxs.x );
	vecStart.y = random->RandomFloat( vecSurroundMins.y, vecSurroundMaxs.y );
	vecStart.z = vecSurroundMaxs.z;

	trace_t tr;
	UTIL_TraceLine( vecStart, vecStart - Vector(0, 0, 4096), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr );

	m_hBeam = CBeam::BeamCreate( "sprites/laserbeam.vmt", 1 );
	m_hBeam->PointsInit( vecStart, tr.endpos );
	m_hBeam->SetColor( 255, 160, 100 );
	m_hBeam->SetBrightness( 128 );
	m_hBeam->SetNoise( 0 );
	m_hBeam->SetBeamFlag( FBEAM_SHADEOUT );
	m_hBeam->SetWidth( TRAIL_WIDTH );
	m_hBeam->SetEndWidth( TRAIL_WIDTH );

	StopSound( "MortarField.Incoming" );
	EmitSound( "MortarField.Explode" );

	int magnitude = 800; // HL1 value was 200, but we want this to explode zombies
	int radius = 200 * 2.5; // HL1 value

	ExplosionCreate( tr.endpos, vec3_angle, this, magnitude, radius, SF_ENVEXPLOSION_NOSOUND );

	// If we hit water, create another explosion above the water
	if ( UTIL_PointContents( tr.endpos ) & CONTENTS_WATER )
	{
		UTIL_TraceLine( vecStart, vecStart - Vector(0, 0, 4096), MASK_SOLID_BRUSHONLY | MASK_WATER, this, COLLISION_GROUP_NONE, &tr );
		ExplosionCreate( tr.endpos + Vector(0,0,1), vec3_angle, this, magnitude, radius, SF_ENVEXPLOSION_NOSOUND | SF_ENVEXPLOSION_NODAMAGE );
	}

	UTIL_ScreenShake( tr.endpos, 25.0, 150.0, 1.0, 750, SHAKE_START );

	m_flFadeTime = gpGlobals->curtime;

	SetThink( &CMortarField::FadeThink );
	SetNextThink( gpGlobals->curtime + 0.05f );
}

void CMortarField::FadeThink( void )
{
	if ( m_flFadeTime <= gpGlobals->curtime - TRAIL_LIFETIME )
	{
		SetThink( NULL );
		UTIL_Remove( m_hBeam );
		return;
	}

	SetNextThink( gpGlobals->curtime + 0.05f );

	float lifePerc = 1.0f - ( ( gpGlobals->curtime - m_flFadeTime  ) / TRAIL_LIFETIME );
	lifePerc = clamp( lifePerc, 0.0f, 1.0f );
	float curve1 = Bias( lifePerc, 0.1f );

	m_hBeam->SetBrightness( 255 * curve1 );
	m_hBeam->SetWidth( TRAIL_WIDTH * curve1 );
	m_hBeam->SetEndWidth( TRAIL_WIDTH * curve1 );
}