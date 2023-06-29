#include "cbase.h"
#include "baseanimating.h"
#include "activitylist.h"
#include "sprite.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define XEN_PLANT_GLOW_SPRITE		"sprites/flare3.vmt"
#define XEN_PLANT_HIDE_TIME			5

class CXenPLight : public CBaseAnimating
{
public:
	DECLARE_CLASS( CXenPLight, CBaseAnimating );
	DECLARE_DATADESC();

	void Spawn( void );
	void Precache( void );

	void CreateEffects( void );
	void OnRestore( void );

	int	ObjectCaps( void ) { return BaseClass::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	void Think(void);
	void Touch( CBaseEntity *pOther );

	void SetActivity( Activity newActivity );
	Activity GetActivity( void );

	void LightOn( void );
	void LightOff( void );

	CHandle<CSprite> m_hGlowSprite;
	float m_flHideTime;
	Activity m_Activity;
	COutputEvent m_OnLightOn;
	COutputEvent m_OnLightOff;
};

BEGIN_DATADESC( CXenPLight )
	DEFINE_FIELD( m_flHideTime, FIELD_TIME ),
	DEFINE_CUSTOM_FIELD( m_Activity, ActivityDataOps() ),
	DEFINE_FIELD( m_hGlowSprite, FIELD_EHANDLE ),
	DEFINE_THINKFUNC( Think ),
	DEFINE_OUTPUT( m_OnLightOn, "OnLightOn" ),
	DEFINE_OUTPUT( m_OnLightOff, "OnLightOff" ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( xen_plantlight, CXenPLight );

//-----------------------------------------------------------------------------
void CXenPLight::Spawn( void )
{
	BaseClass::Spawn();

	Precache();
	SetModel( STRING( GetModelName() ) );
#if 0
	CStudioHdr *pStudioHdr = GetModelPtr();
	if ( pStudioHdr && pStudioHdr->SequencesAvailable() )
	{
		for ( int nSeq = 0; nSeq < pStudioHdr->GetNumSeq(); nSeq++ )
		{
			mstudioseqdesc_t &seqdesc = pStudioHdr->pSeqdesc( nSeq );
			DevMsg("light.mdl: seq %s act %s\n", seqdesc.pszLabel(), seqdesc.pszActivityName());
//			animevent_t event;
//			while ( ( index = GetAnimationEvent( pStudioHdr, nSeq, &event, 0.0f, 1.0f, index ) ) != 0 )
//			{
//				DevMsg( "light.mdl: animevent %d %s\n", seqdesc->, event.event. event.options );
//			}
		}
	}
#endif
	SetActivity( ACT_IDLE );
	SetCycle( random->RandomFloat( 0, 1.0 ) );

	// Large touch-zone
	UTIL_SetSize( this, Vector(-80,-80,0), Vector(80,80,32));

	SetSolid( SOLID_BBOX );
	AddSolidFlags( FSOLID_TRIGGER | FSOLID_NOT_SOLID );
	SetMoveType( MOVETYPE_NONE );

	CreateEffects();

	SetNextThink( gpGlobals->curtime + 0.1f );
	SetThink( &CXenPLight::Think );
}

//-----------------------------------------------------------------------------
void CXenPLight::Precache( void )
{
	SetModelName( AllocPooledString( "models/light.mdl" ) );
	PrecacheModel( STRING( GetModelName() ) );

	PrecacheModel( XEN_PLANT_GLOW_SPRITE );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
void CXenPLight::CreateEffects( void )
{
	m_hGlowSprite = CSprite::SpriteCreate( XEN_PLANT_GLOW_SPRITE, GetAbsOrigin(), false );
	m_hGlowSprite->SetAsTemporary();
	m_hGlowSprite->SetAttachment( this, 1 );
	color32 rendercolor = GetRenderColor();
	m_hGlowSprite->SetTransparency( kRenderGlow, rendercolor.r, rendercolor.g, rendercolor.b, rendercolor.a, m_nRenderFX );
	m_hGlowSprite->SetBrightness( 255, 0.2f );
	m_hGlowSprite->SetScale( 0.25f, 0.2f );
}

//-----------------------------------------------------------------------------
void CXenPLight::OnRestore( void )
{
	CreateEffects();

	BaseClass::OnRestore();
}

//-----------------------------------------------------------------------------
void CXenPLight::Think( void )
{
	if ( ( GetActivity() == ACT_IDLE ) && !UTIL_FindClientInPVS( edict() ) )
	{
		SetNextThink( gpGlobals->curtime + random->RandomFloat( 1.0f, 1.5f ) );
		return;
	}

	StudioFrameAdvance();
	DispatchAnimEvents(this);

	switch ( GetActivity() )
	{
	case ACT_CROUCH:
		if ( IsSequenceFinished() )
		{
			SetActivity( ACT_CROUCHIDLE );
			LightOff();
		}
		break;
	case ACT_CROUCHIDLE:
		if ( gpGlobals->curtime > m_flHideTime )
		{
			SetActivity( ACT_STAND );
			LightOn();
		}
		break;
	case ACT_STAND:
		if ( IsSequenceFinished() )
		{
			SetActivity( ACT_IDLE );
		}
		break;
	}

	SetNextThink( gpGlobals->curtime + 0.1f );
}

//-----------------------------------------------------------------------------
void CXenPLight::Touch( CBaseEntity *pOther )
{
	if ( pOther && pOther->IsPlayer() )
	{
		m_flHideTime = gpGlobals->curtime + XEN_PLANT_HIDE_TIME;
		if ( GetActivity() == ACT_IDLE || GetActivity() == ACT_STAND )
			SetActivity( ACT_CROUCH );
	}
}

//-----------------------------------------------------------------------------
void CXenPLight::SetActivity( Activity newActivity )
{
	int iSequence = SelectWeightedSequence( newActivity );
	if ( iSequence != ACTIVITY_NOT_AVAILABLE )
	{
		SetCycle( 0.0 );
		ResetSequence( iSequence );
		m_Activity = newActivity;
	}
}

//-----------------------------------------------------------------------------
Activity CXenPLight::GetActivity( void )
{
	return m_Activity;
}

//-----------------------------------------------------------------------------
void CXenPLight::LightOn( void )
{
	Assert( m_hGlowSprite != NULL );
	if ( m_hGlowSprite != NULL )
		m_hGlowSprite->TurnOn();
	m_OnLightOn.FireOutput( this, this );
}

//-----------------------------------------------------------------------------
void CXenPLight::LightOff( void )
{
	Assert( m_hGlowSprite != NULL );
	if ( m_hGlowSprite != NULL )
		m_hGlowSprite->TurnOff();
	m_OnLightOff.FireOutput( this, this );
}
