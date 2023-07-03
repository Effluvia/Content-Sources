#include "cbase.h"
#include "beam_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CHoeSunbeam : public CPointEntity
{
	DECLARE_CLASS( CHoeSunbeam, CPointEntity );
public:
	DECLARE_DATADESC();

	CHoeSunbeam();

	void	Precache(void);
	void	Spawn(void);

	int		m_nHaloSprite;
	CHandle<CBeam>			m_hSpotlight;
	
	float	m_flBeamLength;
	float	m_flBeamWidth;
	float	m_flHDRColorScale;
	int		m_nMinDXLevel;
};

BEGIN_DATADESC( CHoeSunbeam )

	DEFINE_FIELD( m_hSpotlight,			FIELD_EHANDLE ),
	DEFINE_FIELD( m_nHaloSprite,			FIELD_INTEGER ),

	DEFINE_KEYFIELD( m_flBeamLength,FIELD_FLOAT, "SunbeamLength"),
	DEFINE_KEYFIELD( m_flBeamWidth,FIELD_FLOAT, "SunbeamWidth"),
	DEFINE_KEYFIELD( m_flHDRColorScale, FIELD_FLOAT, "HDRColorScale" ),
	DEFINE_KEYFIELD( m_nMinDXLevel, FIELD_INTEGER, "mindxlevel" ),

END_DATADESC()


LINK_ENTITY_TO_CLASS( hoe_sunbeam, CHoeSunbeam );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CHoeSunbeam::CHoeSunbeam()
{
	m_flHDRColorScale = 1.0f;
	m_nMinDXLevel = 0;
}

//-----------------------------------------------------------------------------
void CHoeSunbeam::Precache(void)
{
	BaseClass::Precache();

	// Sprites.
	m_nHaloSprite = PrecacheModel("sprites/light_glow03.vmt");
	PrecacheModel( "sprites/sunbeam.vmt" );
}

//-----------------------------------------------------------------------------
void CHoeSunbeam::Spawn(void)
{
	Precache();

	UTIL_SetSize( this,vec3_origin,vec3_origin );
	AddSolidFlags( FSOLID_NOT_SOLID );
	SetMoveType( MOVETYPE_NONE );

	// Check for user error
	if (m_flBeamLength <= 0)
	{
		DevMsg("%s (%s) has an invalid spotlight length <= 0, setting to 500\n", GetClassname(), GetDebugName() );
		m_flBeamLength = 500;
	}
	if (m_flBeamWidth <= 0)
	{
		DevMsg("%s (%s) has an invalid spotlight width <= 0, setting to 10\n", GetClassname(), GetDebugName() );
		m_flBeamWidth = 10;
	}
	
	if (m_flBeamWidth > MAX_BEAM_WIDTH )
	{
		DevMsg("%s (%s) has an invalid spotlight width %.1f (max %.1f).\n", GetClassname(), GetDebugName(), m_flBeamWidth, MAX_BEAM_WIDTH );
		m_flBeamWidth = MAX_BEAM_WIDTH; 
	}

	Vector vSpotlightDir;
	AngleVectors( GetAbsAngles(), &vSpotlightDir );

	trace_t tr;
	UTIL_TraceLine( GetAbsOrigin(), GetAbsOrigin() + vSpotlightDir * m_flBeamLength, MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr);

	//m_hSpotlight = CBeam::BeamCreate( "sprites/spotlight.vmt", m_flBeamWidth );
	m_hSpotlight = CBeam::BeamCreate( "sprites/sunbeam.vmt", m_flBeamWidth );
	// Set the temporary spawnflag on the beam so it doesn't save (we'll recreate it on restore)
	m_hSpotlight->SetHDRColorScale( m_flHDRColorScale );
//	m_hSpotlight->AddSpawnFlags( SF_BEAM_TEMPORARY );
	m_hSpotlight->SetColor( m_clrRender->r, m_clrRender->g, m_clrRender->b ); 
//	m_hSpotlight->SetHaloTexture(m_nHaloSprite);
//	m_hSpotlight->SetHaloScale(60);
	m_hSpotlight->SetEndWidth(m_flBeamWidth);
	m_hSpotlight->SetFadeLength( m_flBeamLength );
	m_hSpotlight->SetBeamFlags( (FBEAM_SHADEOUT|FBEAM_NOTILE) );
	m_hSpotlight->SetBrightness( m_clrRender->a );
	DevMsg("CHoeSunbeam::Spawn r,g,b,a %d,%d,%d,%d\n", m_clrRender->r,m_clrRender->g,m_clrRender->b,m_clrRender->a);
	m_hSpotlight->SetNoise( 0 );
	m_hSpotlight->SetMinDXLevel( m_nMinDXLevel );

	m_hSpotlight->PointsInit( GetAbsOrigin(), tr.endpos );

	RemoveDeferred();
}
