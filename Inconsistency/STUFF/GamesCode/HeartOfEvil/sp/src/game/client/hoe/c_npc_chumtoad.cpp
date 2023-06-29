#include "cbase.h"
#ifdef CHUMTOAD_FLASHLIGHT_EFFECT
#include "flashlighteffect.h"
#else
#include "dlight.h"
#include "iefx.h"
#endif
#include "c_AI_BaseNPC.h"
#include "beam_shared.h"
#include "beamdraw.h"
#include "iviewrender_beams.h"
#include "dlight.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define HEADLIGHT_DISTANCE 768.0f
#define HEADLIGHT_WIDTH 32

class C_NPC_Chumtoad : public C_AI_BaseNPC
{
public:
	DECLARE_CLASS( C_NPC_Chumtoad, C_AI_BaseNPC );
	DECLARE_CLIENTCLASS();
 	DECLARE_DATADESC();

	C_NPC_Chumtoad();
	~C_NPC_Chumtoad();
	void UpdateHeadlight( void );
	virtual void Simulate( void );
	virtual int DrawModel( int flags );
	void DrawWake( void );

	int m_nExactWaterLevel;
	Vector m_vecPhysVelocity;

	float m_flEngineSpeed;

	bool m_bHeadlightIsOn;
#ifdef CHUMTOAD_FLASHLIGHT_EFFECT
	CHeadlightEffect *m_pHeadlight; // expensive shadow-casting light
#else
	dlight_t *m_pDynamicLight;
#endif
	dlight_t *m_pSelfLight;
	Vector m_vecHeadlightBeamStart;
	Vector m_vecHeadlightBeamEnd;
	Beam_t *m_pHeadlightBeam;
};

//-----------------------------------------------------------------------------
// Save/restore
//-----------------------------------------------------------------------------
BEGIN_DATADESC( C_NPC_Chumtoad )
END_DATADESC()

//-----------------------------------------------------------------------------
// Networking
//-----------------------------------------------------------------------------
IMPLEMENT_CLIENTCLASS_DT(C_NPC_Chumtoad, DT_NPC_Chumtoad, CNPC_Chumtoad)
	RecvPropBool( RECVINFO( m_bHeadlightIsOn ) ),
	RecvPropInt( RECVINFO( m_nExactWaterLevel ) ),
	RecvPropInt( RECVINFO( m_nWaterLevel ) ),
	RecvPropVector( RECVINFO( m_vecPhysVelocity ) ),
END_RECV_TABLE()

//-----------------------------------------------------------------------------
C_NPC_Chumtoad::C_NPC_Chumtoad()
{
#ifdef CHUMTOAD_FLASHLIGHT_EFFECT
	m_pHeadlight = NULL;
#endif
	m_flEngineSpeed = 100.0f;
}

//-----------------------------------------------------------------------------
C_NPC_Chumtoad::~C_NPC_Chumtoad()
{
#ifdef CHUMTOAD_FLASHLIGHT_EFFECT
	if (m_pHeadlight)
	{
		delete m_pHeadlight;
	}
#else
	if ( m_pHeadlightBeam )
	{
		m_pHeadlightBeam->die = gpGlobals->curtime; // or beams->FreeBeam?
		m_pHeadlightBeam->flags &= ~FBEAM_FOREVER;
	}
#endif
}

//-----------------------------------------------------------------------------
void C_NPC_Chumtoad::Simulate( void )
{
	UpdateHeadlight();

	BaseClass::Simulate();
}

//-----------------------------------------------------------------------------
// Purpose: Creates, destroys, and updates the headlight effect as needed.
//-----------------------------------------------------------------------------
void C_NPC_Chumtoad::UpdateHeadlight( void )
{
	if (m_bHeadlightIsOn)
	{
#ifdef CHUMTOAD_FLASHLIGHT_EFFECT
		if (!m_pHeadlight)
		{
			// Turned on the headlight; create it.
			m_pHeadlight = new CHeadlightEffect();

			if (!m_pHeadlight)
				return;

			m_pHeadlight->TurnOn();
		}
#else
		if ( !m_pDynamicLight || (m_pDynamicLight->key != entindex()) )
		{
			m_pDynamicLight = effects->CL_AllocDlight( entindex() );
			if ( !m_pDynamicLight )
				return;
		}
#endif

		if ( !m_pSelfLight || (m_pSelfLight->key != -entindex()) )
		{
			m_pSelfLight = effects->CL_AllocDlight( -entindex() );
			if ( !m_pSelfLight )
				return;
		}

		int nHeadlightIndex = LookupAttachment( "headlight" );

		Vector vecLightPos;
		QAngle angLightDir;
		GetAttachment(nHeadlightIndex, vecLightPos, angLightDir);

#ifdef CHUMTOAD_FLASHLIGHT_EFFECT
		Vector vecLightDir, vecLightRight, vecLightUp;
		AngleVectors( angLightDir, &vecLightDir, &vecLightRight, &vecLightUp );

		// Update the light with the new position and direction.		
		m_pHeadlight->UpdateLight( vecLightPos, vecLightDir, vecLightRight, vecLightUp, HEADLIGHT_DISTANCE );

		m_vecHeadlightBeamStart = vecLightPos;
		m_vecHeadlightBeamEnd = vecLightPos + vecLightDir * 500;
#else
		Vector vecLightDir;
		AngleVectors( GetAbsAngles()/*angLightDir*/, &vecLightDir );

		trace_t tr;
		UTIL_TraceLine( vecLightPos, vecLightPos + vecLightDir * 2024, MASK_OPAQUE, this, COLLISION_GROUP_NONE, &tr );

		ColorRGBExp32 color;
		color.r	= 255;
		color.g	= 192;
		color.b	= 64;
		color.exponent = 8;

		float frac = (tr.endpos - vecLightPos).Length() / 500;
		m_pDynamicLight->radius		= 48 * clamp(frac,0.5,1.5);
		m_pDynamicLight->origin		= tr.endpos - vecLightDir * 10;
		m_pDynamicLight->die		= gpGlobals->curtime + 0.05f;
		m_pDynamicLight->color		= color;

		m_vecHeadlightBeamStart = vecLightPos;
		m_vecHeadlightBeamEnd = vecLightPos + vecLightDir * 500;
#endif

		m_pSelfLight->radius	= 32;
		m_pSelfLight->origin	= vecLightPos + vecLightDir * 10;
		m_pSelfLight->die		= gpGlobals->curtime + 0.05f;
		m_pSelfLight->color		= color;

		if ( m_pHeadlightBeam == NULL )
		{
			BeamInfo_t beamInfo;

			beamInfo.m_vecStart = m_vecHeadlightBeamStart;
			beamInfo.m_vecEnd = m_vecHeadlightBeamEnd;

			beamInfo.m_pStartEnt = NULL;
			beamInfo.m_pEndEnt = NULL;
			beamInfo.m_nStartAttachment = 0;
			beamInfo.m_nEndAttachment = 0;

			beamInfo.m_pszModelName = "sprites/glow_test02.vmt";
			beamInfo.m_pszHaloName = "sprites/light_glow03.vmt";
			beamInfo.m_flHaloScale = 16.0f;
			beamInfo.m_flLife = 0.0f;
			beamInfo.m_flWidth = 14;
			beamInfo.m_flEndWidth = 14;
			beamInfo.m_flFadeLength = 0.0f;
			beamInfo.m_flAmplitude = 0;
			beamInfo.m_flBrightness = 48.0;
			beamInfo.m_flSpeed = 0.0;
			beamInfo.m_nStartFrame = 0.0;
			beamInfo.m_flFrameRate = 1.0f;

			beamInfo.m_flRed = 255.0f;
			beamInfo.m_flGreen = 255.0f;
			beamInfo.m_flBlue = 255.0f;

			beamInfo.m_nSegments = -1;
			beamInfo.m_bRenderable = true;
			beamInfo.m_nFlags = FBEAM_SHADEOUT|FBEAM_NOTILE|FBEAM_HALOBEAM;
			
			m_pHeadlightBeam = beams->CreateBeamEntPoint( beamInfo );
		}
#ifdef CHUMTOAD_FLASHLIGHT_EFFECT
#else
		if ( m_pHeadlightBeam != NULL )
		{
			float flDist = (tr.endpos - vecLightPos).Length();
			flDist = clamp( flDist, 0, 500 );
			m_pHeadlightBeam->fadeLength = flDist;
		}
#endif
	}
#ifdef CHUMTOAD_FLASHLIGHT_EFFECT
	else if (m_pHeadlight)
	{
		// Turned off the headlight; delete it.
		delete m_pHeadlight;
		m_pHeadlight = NULL;
	}
#else
	else if ( m_pHeadlightBeam )
	{
		m_pHeadlightBeam->die = gpGlobals->curtime; // or beams->FreeBeam?
		m_pHeadlightBeam->flags &= ~FBEAM_FOREVER;
		m_pHeadlightBeam = NULL;
	}
#endif
}

//-----------------------------------------------------------------------------
void C_NPC_Chumtoad::DrawWake( void )
{
	if ( GetWaterLevel() == 0 )
		return;

	float flSpeed = m_vecPhysVelocity.Length();
	float flSpeedFrac = flSpeed / m_flEngineSpeed;
	flSpeedFrac = clamp( flSpeedFrac, 0.0f, 1.0f );

	if ( flSpeedFrac < 0.2 )
		return;

	Vector startPos = GetAbsOrigin();
	Vector wakeDir = m_vecPhysVelocity; wakeDir.NormalizeInPlace(); wakeDir *= -1;
	float wakeLength = 16 + flSpeedFrac * 48;
	float speed = flSpeed;

	float minSpeed = 150*0.2;
	float maxSpeed = 150;

///////////////////////////////////////////////////
#define	WAKE_STEPS	6

	Vector	wakeStep = wakeDir * ( wakeLength / (float) WAKE_STEPS );
	Vector	origin;
	float	scale;

	CMatRenderContextPtr pRenderContext( materials );
	IMaterial *mat = materials->FindMaterial( "effects/splashwake1", TEXTURE_GROUP_CLIENT_EFFECTS );
	IMesh* pMesh = pRenderContext->GetDynamicMesh( 0, 0, 0, mat );

	CMeshBuilder meshBuilder;
	meshBuilder.Begin( pMesh, MATERIAL_QUADS, WAKE_STEPS );

	for ( int i = 0; i < WAKE_STEPS; i++ )
	{
		origin = startPos + ( wakeStep * i );
		origin[0] += random->RandomFloat( -4.0f, 4.0f );
		origin[1] += random->RandomFloat( -4.0f, 4.0f );
		origin[2] = m_nExactWaterLevel + 2.0f;

		float scaleRange = RemapVal( i, 0, WAKE_STEPS-1, 16, 32 );
		scale = scaleRange + ( 8.0f * sin( gpGlobals->curtime * 5 * i ) );
		
		float alpha = RemapValClamped( speed, minSpeed, maxSpeed, 0.05f, 0.25f );
		float color[4] = { 1.0f, 1.0f, 1.0f, alpha };
		
		// Needs to be time based so it'll freeze when the game is frozen
		float yaw = random->RandomFloat( 0, 360 );

		Vector rRight = ( Vector(1,0,0) * cos( DEG2RAD( yaw ) ) ) - ( Vector(0,1,0) * sin( DEG2RAD( yaw ) ) );
		Vector rUp = ( Vector(1,0,0) * cos( DEG2RAD( yaw+90.0f ) ) ) - ( Vector(0,1,0) * sin( DEG2RAD( yaw+90.0f ) ) );

		Vector point;
		meshBuilder.Color4fv (color);
		meshBuilder.TexCoord2f (0, 0, 1);
		VectorMA (origin, -scale, rRight, point);
		VectorMA (point, -scale, rUp, point);
		meshBuilder.Position3fv (point.Base());
		meshBuilder.AdvanceVertex();

		meshBuilder.Color4fv (color);
		meshBuilder.TexCoord2f (0, 0, 0);
		VectorMA (origin, scale, rRight, point);
		VectorMA (point, -scale, rUp, point);
		meshBuilder.Position3fv (point.Base());
		meshBuilder.AdvanceVertex();

		meshBuilder.Color4fv (color);
		meshBuilder.TexCoord2f (0, 1, 0);
		VectorMA (origin, scale, rRight, point);
		VectorMA (point, scale, rUp, point);
		meshBuilder.Position3fv (point.Base());
		meshBuilder.AdvanceVertex();

		meshBuilder.Color4fv (color);
		meshBuilder.TexCoord2f (0, 1, 1);
		VectorMA (origin, -scale, rRight, point);
		VectorMA (point, scale, rUp, point);
		meshBuilder.Position3fv (point.Base());
		meshBuilder.AdvanceVertex();
	}

	meshBuilder.End();
	pMesh->Draw();
}

//-----------------------------------------------------------------------------
int C_NPC_Chumtoad::DrawModel( int flags )
{
	if ( BaseClass::DrawModel( flags ) == false )
		return 0;

	if ( m_bHeadlightIsOn )
	{
//		IMaterial *pMat = materials->FindMaterial( "sprites/glow_test02.vmt", NULL );

		if ( gpGlobals->frametime != 0 )
		{
			if ( m_pHeadlightBeam != NULL )
			{
				m_pHeadlightBeam->attachment[0] = m_vecHeadlightBeamStart;
				m_pHeadlightBeam->attachment[1] = m_vecHeadlightBeamEnd;

				VectorSubtract( m_pHeadlightBeam->attachment[1], m_pHeadlightBeam->attachment[0], m_pHeadlightBeam->delta );
#if 0
				if ( m_pHeadlightBeam->amplitude >= 0.50 )
					m_pHeadlightBeam->segments = VectorLength( m_pHeadlightBeam->delta ) * 0.25 + 3; // one per 4 pixels
				else
					m_pHeadlightBeam->segments = VectorLength( m_pHeadlightBeam->delta ) * 0.075 + 3; // one per 16 pixels
#endif
			}
		}
	}
	DrawWake();
	return 1;
}