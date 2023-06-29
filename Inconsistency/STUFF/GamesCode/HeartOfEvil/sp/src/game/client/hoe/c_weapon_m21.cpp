#include "cbase.h"
#include "c_basehlcombatweapon.h"
#include "scope_rendertarget.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifndef HOE_IRONSIGHTS
#define M21_ZOOMING
#define M21_ZOOMING_DELTA (Vector(4.276326, -2, 41.679108) - Vector(-1.764711, 8.235304, 39.525997))
#define M21_ZOOMING_DURATION 0.2
#endif // HOE_IRONSIGHTS

class C_WeaponM21 : public C_BaseHLCombatWeapon
{
public:
	DECLARE_CLASS( C_WeaponM21, C_BaseHLCombatWeapon );
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_DATADESC();

	C_WeaponM21();
	~C_WeaponM21();

	void OnRestore( void );

	bool CalcViewModelView( CBaseViewModel *viewmodel, const Vector& preOrigin, const QAngle& preAngles, Vector& origin, QAngle& angles );
	void ViewModelDrawn( C_BaseViewModel *pViewModel );

#ifdef HOE_THIRDPERSON
	DECLARE_ACTTABLE();	
#endif // HOE_THIRDPERSON

#ifdef M21_ZOOMING
	bool IsWeaponZoomed( void ) { return m_bInZoom; }
//	bool ShouldDrawCrosshair( void ) { return false; }
	bool IsZooming( void ) { return m_nZooming != ZOOMING_NONE; }

	enum {
		ZOOMING_NONE,
		ZOOMING_IN,
		ZOOMING_OUT,
	};
	bool m_nZooming;

	bool m_bInZoom;

//	Vector m_LastViewmodelOrigin;
//	QAngle m_LastViewmodelAngles;
#endif

#ifdef HOE_IRONSIGHTS
	bool m_bActive;
	bool m_bScopeTextureSetByMe;
#endif // HOE_IRONSIGHTS
};

BEGIN_PREDICTION_DATA( C_WeaponM21 )
END_PREDICTION_DATA()

#ifdef HOE_THIRDPERSON
acttable_t C_WeaponM21::m_acttable[] = 
{
	{ ACT_IDLE,						ACT_IDLE_SMG1,					true },
//	{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_SMG1,			true },
	{ ACT_RUN_AIM,					ACT_RUN_AIM_RIFLE,				true },
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_SMG1,	true },
};
IMPLEMENT_ACTTABLE(C_WeaponM21);
#endif // HOE_THIRDPERSON

#ifdef HOE_IRONSIGHTS
//-----------------------------------------------------------------------------
static void RecvProxy_M21Active( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	bool state = *((bool *)&pData->m_Value.m_Int);

	C_WeaponM21 *pWeapon = (C_WeaponM21 *) pStruct;
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if ( pPlayer && pWeapon->GetOwner() == pPlayer )
	{
		if ( state )
		{
			if ( ScopeRenderTarget != NULL )
				ScopeRenderTarget->SetScopeTextureActive( true );
			pWeapon->m_bScopeTextureSetByMe = true;
		}
		else
		{
			if ( ScopeRenderTarget != NULL )
				ScopeRenderTarget->SetScopeTextureActive( false );
			pWeapon->m_bScopeTextureSetByMe = false;
		}
	}

	*(bool *)pOut = state;
}
#endif // HOE_IRONSIGHTS

IMPLEMENT_CLIENTCLASS_DT( C_WeaponM21, DT_WeaponM21, CWeaponM21 )
#ifdef M21_ZOOMING
	RecvPropInt( RECVINFO( m_nZooming ) ),
	RecvPropBool( RECVINFO( m_bInZoom ) ),
#endif
#ifdef HOE_IRONSIGHTS
	RecvPropInt( RECVINFO(m_bActive), 0, RecvProxy_M21Active ),
#endif // HOE_IRONSIGHTS
END_RECV_TABLE()

BEGIN_DATADESC( C_WeaponM21 )
	DEFINE_FIELD( m_bActive, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bScopeTextureSetByMe, FIELD_BOOLEAN ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( weapon_M21, C_WeaponM21 );

//-----------------------------------------------------------------------------
C_WeaponM21::C_WeaponM21()
{
	m_bActive = false;
	m_bScopeTextureSetByMe = false;
}

//-----------------------------------------------------------------------------
C_WeaponM21::~C_WeaponM21()
{
	if ( m_bScopeTextureSetByMe && ScopeRenderTarget != NULL )
		ScopeRenderTarget->SetScopeTextureActive( false );
}

//-----------------------------------------------------------------------------
void C_WeaponM21::OnRestore( void )
{
	BaseClass::OnRestore();
	
	if ( m_bScopeTextureSetByMe )
	{
		if ( ScopeRenderTarget != NULL )
			ScopeRenderTarget->SetScopeTextureActive( true );
	}
}

//-----------------------------------------------------------------------------
bool C_WeaponM21::CalcViewModelView( CBaseViewModel *viewmodel, const Vector& preOrigin, const QAngle& preAngles, Vector& origin, QAngle& angles )
{
#ifdef M21_ZOOMING
	if ( IsZooming() )
	{
		float flEndAnim = GetWeaponIdleTime();
		float flFrac = 1.0 - (flEndAnim - gpGlobals->curtime) / M21_ZOOMING_DURATION;
		flFrac = clamp( flFrac, 0.0f, 1.0f );

		// Interpolate between where we would be if unzoomed and where we should be when zooming.
		// The unzoomed orientation has bob/lag/shake while the zoomed orientation has none of those.
		Vector forward, right, up;
		AngleVectors( preAngles, &forward, &right, &up );
		Vector offset = M21_ZOOMING_DELTA;
		Vector originZoomed = preOrigin;
		if ( m_nZooming == ZOOMING_IN )
		{
			originZoomed += (-offset.x) * right + offset.y * forward + offset.z * up;
			VectorLerp( origin, originZoomed, flFrac, origin );
			angles = Lerp( flFrac, angles, preAngles );
		}
		else
		{
			offset *= 1.0f - flFrac;
			originZoomed += (-offset.x) * right + offset.y * forward + offset.z * up;
			VectorLerp( originZoomed, origin, flFrac, origin );
			angles = Lerp( flFrac, preAngles, angles );
		}

		// Use the given values without adding bob or lag.
		return true;
	}
#endif // M21_ZOOMING

#ifdef HOE_IRONSIGHTS
	if ( ScopeRenderTarget != NULL )
		ScopeRenderTarget->SetScopeTextureActive( true );
	m_bScopeTextureSetByMe = true;
#endif // HOE_IRONSIGHTS

	return BaseClass::CalcViewModelView( viewmodel, preOrigin, preAngles, origin, angles );
}

//-----------------------------------------------------------------------------
void C_WeaponM21::ViewModelDrawn( C_BaseViewModel *pViewModel )
{
#ifdef M21_ZOOMINGxxx
	if ( !IsZooming() )
	{
		// Remember these so we can lerp to/from them when playing our bob&lag-free zooming animations.
		m_LastViewmodelOrigin = pViewModel->GetLocalOrigin();
		m_LastViewmodelAngles = pViewModel->GetLocalAngles();
	}
#endif

	BaseClass::ViewModelDrawn( pViewModel );
}

//-----------------------------------------------------------------------------
// Called by HudScope to determine if it should draw itself.
bool IsWeaponM21Zoomed( CBasePlayer *pPlayer )
{
	Assert( pPlayer );
	if ( pPlayer == NULL )
		return false;
	C_BaseCombatWeapon *wpn = pPlayer->GetActiveWeapon();
	if ( wpn == NULL )
		return false;
	C_WeaponM21 *pM21 = dynamic_cast<C_WeaponM21 *>(wpn);
#ifdef HOE_IRONSIGHTS
	return pM21 && pM21->IsIronSightsActive();
#endif // HOE_IRONSIGHTS
#ifdef M21_ZOOMING
	return pM21 && pM21->IsWeaponZoomed();
#endif
}
