#include "cbase.h"
#include "AI_BaseNPC.h"
#include "NPCEvent.h"
#include "basehlcombatweapon_shared.h"
#include "in_buttons.h"
#include "soundent.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifndef HOE_IRONSIGHTS
#define M21_ZOOMING // define if using animations to raise/lower scope to/from the player's eye
#define M21_ZOOMING_DURATION 0.2
#endif // HOE_IRONSIGHTS

//-----------------------------------------------------------------------------
// CWeaponM21
//-----------------------------------------------------------------------------

class CWeaponM21 : public CBaseHLCombatWeapon
{
	DECLARE_CLASS( CWeaponM21, CBaseHLCombatWeapon );
public:

	CWeaponM21( void );
	
	virtual void	Precache( void );
	virtual void	PrimaryAttack( void );
	virtual void	SecondaryAttack( void );
	virtual void	Drop( const Vector &vecVelocity );
	virtual bool	Deploy( void );
	virtual bool	Holster( CBaseCombatWeapon *pSwitchingTo = NULL );
	virtual bool	Reload( void );
	virtual void	ItemPostFrame( void );
	virtual void	ItemBusyFrame( void );

#ifdef HOE_IRONSIGHTS
	virtual int		GetIronSights_FOV( void ) const
	{
#ifdef HOE_THIRDPERSON
		CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
//		if ( pOwner && pOwner->IsThirdPerson() )
//			return 20; // FIXME: Separate 1st/3rd-person FOV
#endif
		return BaseClass::GetIronSights_FOV();
	}
#endif

#ifdef M21_ZOOMING
	virtual bool	IsWeaponZoomed() { return m_bInZoom; }
#endif // HOE_IRONSIGHTS
	
	bool	ShouldDisplayHUDHint() { return true; }

	virtual float GetFireRate( void ) { return 0.7; } // 1.0 == HL1 M21_FIRE_DELAY

	// This prevents the weapon firing faster than its rate-of-fire.
	// CAI_ShotRegulator::Reset(false) will allow another shot as soon as the rest interval is over
	// even if the rest interval is shorter than GetFireRate().
	virtual float GetMinRestTime() { return GetFireRate(); }
	virtual float GetMaxRestTime() { return GetFireRate(); }

	Vector	GetBulletSpread( WeaponProficiency_t proficiency )
	{
#ifdef HOE_IRONSIGHTS
		return  IsIronSightsActive() ? VECTOR_CONE_PRECALCULATED : VECTOR_CONE_1DEGREES;
#else // HOE_IRONSIGHTS
		return  m_bInZoom ? VECTOR_CONE_PRECALCULATED : VECTOR_CONE_1DEGREES;
#endif // HOE_IRONSIGHTS
	}
	int		CapabilitiesGet( void ) { return bits_CAP_WEAPON_RANGE_ATTACK1; }

	void FireNPCPrimaryAttack( CBaseCombatCharacter *pOperator, Vector &vecShootOrigin, Vector &vecShootDir );
	void Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );

	bool StartSprinting( void )
	{
#ifdef M21_ZOOMING
		if ( m_bInZoom )
			StartToggleZoom();
		return true;
#endif
#ifdef HOE_IRONSIGHTS
		return BaseClass::StartSprinting();
#endif
	}

#ifdef HOE_IRONSIGHTS
	bool CWeaponM21::CanToggleIronSights( void );
#endif // HOE_IRONSIGHTS

	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

private:
	
	void	CheckZoomToggle( void );
#ifndef HOE_IRONSIGHTS
	void	ToggleZoom( void );
#endif // HOE_IRONSIGHTS

private:

#ifndef HOE_IRONSIGHTS
	CNetworkVar( bool, m_bInZoom );
	bool m_bMustReload;
#endif // HOE_IRONSIGHTS

//	float				m_flViewBobTime;

#ifdef M21_ZOOMING
	void StartToggleZoom( void );
	enum {
		ZOOMING_NONE,
		ZOOMING_IN,
		ZOOMING_OUT,
	};
	CNetworkVar( char, m_nZooming ); // raising or lowering the scope

//	float GetViewModelCycle( void );
//	void SetViewModelCycle( float flCycle );
#endif

#ifdef HOE_IRONSIGHTS
	CNetworkVar( bool, m_bActive ); // this is the active viewmodel, tells client to enable the scope render texture
#endif // HOE_IRONSIGHTS

#ifdef HOE_THIRDPERSON
	bool m_fFirstPersonSwitch; // TRUE when this weapon forced 3rd->1st-person
#endif

	DECLARE_ACTTABLE();
};

LINK_ENTITY_TO_CLASS( weapon_m21, CWeaponM21 );

PRECACHE_WEAPON_REGISTER( weapon_m21 );

IMPLEMENT_SERVERCLASS_ST( CWeaponM21, DT_WeaponM21 )
#ifdef M21_ZOOMING
	SendPropInt( SENDINFO( m_nZooming ), 3, SPROP_UNSIGNED ),
	SendPropBool( SENDINFO( m_bInZoom ) ),
#endif
#ifdef HOE_IRONSIGHTS
	SendPropBool( SENDINFO( m_bActive ) ),
#endif // HOE_IRONSIGHTS
END_SEND_TABLE()

BEGIN_DATADESC( CWeaponM21 )
#ifndef HOE_IRONSIGHTS
	DEFINE_FIELD( m_bInZoom,		FIELD_BOOLEAN ),
#endif // HOE_IRONSIGHTS
#ifdef M21_ZOOMING
	DEFINE_FIELD( m_nZooming,		FIELD_CHARACTER ),
	DEFINE_FIELD( m_bMustReload,	FIELD_BOOLEAN ),
#endif
#ifdef HOE_IRONSIGHTS
	DEFINE_FIELD( m_bActive, FIELD_BOOLEAN ),
#endif // HOE_IRONSIGHTS
END_DATADESC()

acttable_t	CWeaponM21::m_acttable[] = 
{
	{ ACT_RANGE_ATTACK1,			ACT_RANGE_ATTACK_SNIPER_RIFLE,	true },
	{ ACT_RELOAD,					ACT_RELOAD_SMG1,				true },
	{ ACT_IDLE,						ACT_IDLE_SMG1,					true },
//	{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_SMG1,			true },

	{ ACT_WALK,						ACT_WALK_RIFLE,					true },
	{ ACT_WALK_AIM,					ACT_WALK_AIM_RIFLE,				true  },
	
// Readiness activities (not aiming)
	{ ACT_IDLE_RELAXED,				ACT_IDLE_SMG1_RELAXED,			false },//never aims
	{ ACT_IDLE_STIMULATED,			ACT_IDLE_SMG1_STIMULATED,		false },
	{ ACT_IDLE_AGITATED,			ACT_IDLE_ANGRY_SMG1,			false },//always aims

	{ ACT_WALK_RELAXED,				ACT_WALK_RIFLE_RELAXED,			false },//never aims
	{ ACT_WALK_STIMULATED,			ACT_WALK_RIFLE_STIMULATED,		false },
	{ ACT_WALK_AGITATED,			ACT_WALK_AIM_RIFLE,				false },//always aims

	{ ACT_RUN_RELAXED,				ACT_RUN_RIFLE_RELAXED,			false },//never aims
	{ ACT_RUN_STIMULATED,			ACT_RUN_RIFLE_STIMULATED,		false },
	{ ACT_RUN_AGITATED,				ACT_RUN_AIM_RIFLE,				false },//always aims

// Readiness activities (aiming)
	{ ACT_IDLE_AIM_RELAXED,			ACT_IDLE_SMG1_RELAXED,			false },//never aims	
	{ ACT_IDLE_AIM_STIMULATED,		ACT_IDLE_AIM_RIFLE_STIMULATED,	false },
	{ ACT_IDLE_AIM_AGITATED,		ACT_IDLE_ANGRY_SMG1,			false },//always aims

	{ ACT_WALK_AIM_RELAXED,			ACT_WALK_RIFLE_RELAXED,			false },//never aims
	{ ACT_WALK_AIM_STIMULATED,		ACT_WALK_AIM_RIFLE_STIMULATED,	false },
	{ ACT_WALK_AIM_AGITATED,		ACT_WALK_AIM_RIFLE,				false },//always aims

	{ ACT_RUN_AIM_RELAXED,			ACT_RUN_RIFLE_RELAXED,			false },//never aims
	{ ACT_RUN_AIM_STIMULATED,		ACT_RUN_AIM_RIFLE_STIMULATED,	false },
	{ ACT_RUN_AIM_AGITATED,			ACT_RUN_AIM_RIFLE,				false },//always aims
//End readiness activities

	{ ACT_WALK_AIM,					ACT_WALK_AIM_RIFLE,				true },
	{ ACT_WALK_CROUCH,				ACT_WALK_CROUCH_RIFLE,			true },
	{ ACT_WALK_CROUCH_AIM,			ACT_WALK_CROUCH_AIM_RIFLE,		true },
	{ ACT_RUN,						ACT_RUN_RIFLE,					true },
	{ ACT_RUN_AIM,					ACT_RUN_AIM_RIFLE,				true },
	{ ACT_RUN_CROUCH,				ACT_RUN_CROUCH_RIFLE,			true },
	{ ACT_RUN_CROUCH_AIM,			ACT_RUN_CROUCH_AIM_RIFLE,		true },
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_SMG1,	true },
	{ ACT_RANGE_ATTACK1_LOW,		ACT_RANGE_ATTACK_SMG1_LOW,		true },
	{ ACT_COVER_LOW,				ACT_COVER_SMG1_LOW,				false },
	{ ACT_RANGE_AIM_LOW,			ACT_RANGE_AIM_SMG1_LOW,			false },
	{ ACT_RELOAD_LOW,				ACT_RELOAD_SMG1_LOW,			false },
	{ ACT_GESTURE_RELOAD,			ACT_GESTURE_RELOAD_SMG1,		true },
};

IMPLEMENT_ACTTABLE(CWeaponM21);

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponM21::CWeaponM21( void )
{
	m_bAltFiresUnderwater = true;
#ifdef HOE_IRONSIGHTS
	m_bActive			= false;
#else // HOE_IRONSIGHTS
	m_bInZoom			= false;
#endif // HOE_IRONSIGHTS
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponM21::Precache( void )
{
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponM21::PrimaryAttack( void )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	
	if ( pOwner == NULL )
		return;

#ifdef HOE_THIRDPERSON
	if ( pOwner->IsThirdPerson() )
		pOwner->ViewPunch( QAngle( -2, 0, 0 ) );
#endif

	BaseClass::PrimaryAttack();

#ifdef M21_ZOOMING
	// Auto-reload when zoomed in and we fire our last shot
	if ( m_bInZoom && !m_iClip1 )
		m_bMustReload = true;
#endif
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponM21::SecondaryAttack( void )
{
#ifdef M21_ZOOMING
	// Only toggle zoom on the initial button press
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if ( !pPlayer || (pPlayer->m_afButtonPressed & IN_ATTACK2) == 0 )
		return;
#if 0
	float flCycle = GetViewModelCycle();
	float flDuration;
#else
	if ( m_nZooming != ZOOMING_NONE )
		return;
#endif

	StartToggleZoom();
#else
	//NOTENOTE: The zooming is handled by the post/busy frames
#endif
}

#ifdef M21_ZOOMING
//-----------------------------------------------------------------------------
void CWeaponM21::StartToggleZoom( void )
{
	if ( m_bInZoom )
	{
		// Zoom out
		if ( m_bInZoom )
			ToggleZoom();
		m_nZooming = ZOOMING_OUT;
	}
	else
	{
		// Zoom in
		m_nZooming = ZOOMING_IN;
	}
	SendWeaponAnim( ACT_SPECIAL_ATTACK1 );
	m_flNextPrimaryAttack = gpGlobals->curtime + M21_ZOOMING_DURATION;
	SetWeaponIdleTime( m_flNextPrimaryAttack );
}
#endif // M21_ZOOMING

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponM21::Reload( void )
{
#ifdef M21_ZOOMING
	if ( m_bInZoom )
	{
		// Zoom out then reload
		StartToggleZoom();
		m_bMustReload = true;
		return false;
	}
	if ( m_nZooming == ZOOMING_IN || m_nZooming == ZOOMING_OUT )
		return false;

#endif

	if ( BaseClass::Reload() )
	{
#ifdef M21_ZOOMING
		if ( m_bInZoom )
			ToggleZoom();
		m_bMustReload = false;
#endif
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponM21::CheckZoomToggle( void )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	
	if ( pPlayer->m_afButtonPressed & IN_ATTACK2 )
	{
#ifdef HOE_IRONSIGHTS
		ToggleIronSights();
#else // HOE_IRONSIGHTS
		ToggleZoom();
#endif // HOE_IRONSIGHTS
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponM21::ItemBusyFrame( void )
{
#ifndef HOE_IRONSIGHTS
#ifndef M21_ZOOMING
	// Allow zoom toggling even when we're reloading
	CheckZoomToggle();
#endif
#endif // HOE_IRONSIGHTS
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponM21::ItemPostFrame( void )
{
#ifdef M21_ZOOMING
	if ( m_nZooming != ZOOMING_NONE && HasWeaponIdleTimeElapsed() )
	{
		if ( m_nZooming == ZOOMING_IN )
			ToggleZoom();
		m_nZooming = ZOOMING_NONE;
	}

	// If we tried to reload while zoomed in then we must finish zooming out before reloading.
	if ( m_bMustReload && HasWeaponIdleTimeElapsed() )
	{
		Reload();
	}
#else
	// Allow zoom toggling
	CheckZoomToggle();
#endif
#if 0 // TODO: view sway while zoomed in
	if ( m_bInZoom && m_flViewBobTime + 0.5 <= gpGlobals->curtime )
	{
		CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
		if ( pOwner != NULL )
			pOwner->ViewPunch( QAngle( random->RandomFloat(-0.1, 0.1), random->RandomFloat(-0.1, 0.1), 0 ) );
		m_flViewBobTime = gpGlobals->curtime;
	}
#endif
	BaseClass::ItemPostFrame();
#ifdef HOE_THIRDPERSON
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if ( IsIronSightsActive() && pPlayer && pPlayer->IsThirdPerson() )
		pPlayer->ThirdPersonSwitch( false );
	else if ( GetIronSightsState() == IRONSIGHT_STATE_FROM && pPlayer && !pPlayer->IsThirdPerson() )
		pPlayer->ThirdPersonSwitch( true );
#endif
}

//-----------------------------------------------------------------------------
bool CWeaponM21::Deploy( void )
{
#ifdef HOE_IRONSIGHTS
	m_bActive = true;
#endif // HOE_IRONSIGHTS

	return BaseClass::Deploy();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pSwitchingTo - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponM21::Holster( CBaseCombatWeapon *pSwitchingTo )
{
#ifdef M21_ZOOMING
	m_nZooming = ZOOMING_NONE;
	m_bMustReload = false;
#endif

	// Stop zooming
#ifdef HOE_THIRDPERSON
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
    // FIXME: only switch to 3rd-person if the player wants 3rd-person
	if ( IsIronSightsActive() && pPlayer && !pPlayer->IsThirdPerson() )
		pPlayer->ThirdPersonSwitch( true );
#elif !defined(HOE_IRONSIGHTS)
	if ( m_bInZoom )
	{
		ToggleZoom();
	}
#endif // HOE_IRONSIGHTS

#ifdef HOE_IRONSIGHTS
	m_bActive = false;
#endif // HOE_IRONSIGHTS

	return BaseClass::Holster( pSwitchingTo );
}

#ifndef HOE_IRONSIGHTS
//-----------------------------------------------------------------------------
void CWeaponM21::ToggleZoom( void )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	
	if ( pPlayer == NULL )
		return;

	if ( m_bInZoom )
	{
		// Zoom out
		if ( pPlayer->SetFOV( this, 0, 0.2f ) )
		{
			m_bInZoom = false;
#if 0
			// Send a message to hide the scope
			CSingleUserRecipientFilter filter(pPlayer);
			UserMessageBegin(filter, "ShowScope");
			WRITE_BYTE(0);
			MessageEnd();
#endif
			// Show the crosshair and quickinfo HUD elements.
  			pPlayer->ShowCrosshair( true );
		}
	}
	else
	{
		// Zoom in
		if ( pPlayer->SetFOV( this, 20, 0.1f ) )
		{
			m_bInZoom = true;
#if 0
			// Send a message to Show the scope
			CSingleUserRecipientFilter filter(pPlayer);
			UserMessageBegin(filter, "ShowScope");
			WRITE_BYTE(1);
			MessageEnd();
#endif
			// Hide the crosshair and quickinfo HUD elements.
 			pPlayer->ShowCrosshair( false );
		}
	}
}
#endif // HOE_IRONSIGHTS

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponM21::Drop( const Vector &vecVelocity )
{
#ifndef HOE_IRONSIGHTS
	// Stop zooming
	if ( m_bInZoom )
	{
		ToggleZoom();
	}
#endif // HOE_IRONSIGHTS

	BaseClass::Drop( vecVelocity );
}

//-----------------------------------------------------------------------------
void CWeaponM21::FireNPCPrimaryAttack( CBaseCombatCharacter *pOperator, Vector &vecShootOrigin, Vector &vecShootDir )
{
	WeaponSound( SINGLE_NPC );

	CSoundEnt::InsertSound( SOUND_COMBAT|SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_PISTOL, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy() );
	pOperator->FireBullets( 1, vecShootOrigin, vecShootDir, VECTOR_CONE_PRECALCULATED,
		MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2, entindex(), 0 );

	pOperator->DoMuzzleFlash();
	m_iClip1 = m_iClip1 - 1;
}

//-----------------------------------------------------------------------------
void CWeaponM21::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	switch( pEvent->event )
	{
	case EVENT_WEAPON_SNIPER_RIFLE_FIRE:
		{
			Vector vecShootOrigin, vecShootDir;
			QAngle angDiscard;

			// Support old style attachment point firing
			if ((pEvent->options == NULL) || (pEvent->options[0] == '\0') || (!pOperator->GetAttachment(pEvent->options, vecShootOrigin, angDiscard)))
			{
				vecShootOrigin = pOperator->Weapon_ShootPosition();
			}

			CAI_BaseNPC *npc = pOperator->MyNPCPointer();
			ASSERT( npc != NULL );
			vecShootDir = npc->GetActualShootTrajectory( vecShootOrigin );

			FireNPCPrimaryAttack( pOperator, vecShootOrigin, vecShootDir );
		}
		break;

	default:
		BaseClass::Operator_HandleAnimEvent( pEvent, pOperator );
		break;
	}
}

#ifdef M21_ZOOMINGxxx
//-----------------------------------------------------------------------------
float CWeaponM21::GetViewModelCycle( void )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( pOwner == NULL )
	{
		Assert( false );
		return 0;
	}
	
	CBaseViewModel *vm = pOwner->GetViewModel( m_nViewModelIndex );
	if ( vm == NULL )
	{
		Assert( false );
		return 0;
	}

	SetViewModel();
	Assert( vm->ViewModelIndex() == m_nViewModelIndex );
	return vm->GetCycle();
}

//-----------------------------------------------------------------------------
void CWeaponM21::SetViewModelCycle( float flCycle )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( pOwner == NULL )
	{
		Assert( false );
		return;
	}
	
	CBaseViewModel *vm = pOwner->GetViewModel( m_nViewModelIndex );
	if ( vm == NULL )
	{
		Assert( false );
		return;
	}

	SetViewModel();
	Assert( vm->ViewModelIndex() == m_nViewModelIndex );
vm->SetEffects( EF_NOINTERP );
	vm->SetCycle( flCycle );
vm->RemoveEffects( EF_NOINTERP );
}

#endif // M21_ZOOMING

#ifdef HOE_IRONSIGHTS
//-----------------------------------------------------------------------------
bool CWeaponM21::CanToggleIronSights( void )
{
	extern ConVar hoe_ironsights;
	if ( !hoe_ironsights.GetBool() )
		return false;

	if ( !HasIronSights() )
		return false;

	if ( IsIronSightsTransitioning() )
		 return false;

	if ( m_bInReload )
		return false;

	// Allow switch during long shoot animation
	if ( GetActivity() == ACT_VM_PRIMARYATTACK_IRONSIGHT || GetActivity() == ACT_VM_IDLE_IRONSIGHT )
		return true;

	if ( m_flNextPrimaryAttack > gpGlobals->curtime ||
		 m_flNextSecondaryAttack > gpGlobals->curtime )
		 return false;

	return true;
}
#endif // HOE_IRONSIGHTS
