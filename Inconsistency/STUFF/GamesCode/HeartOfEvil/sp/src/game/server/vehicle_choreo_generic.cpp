//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "npcevent.h"
#include "vehicle_base.h"
#include "engine/IEngineSound.h"
#include "in_buttons.h"
#include "soundenvelope.h"
#include "soundent.h"
#include "physics_saverestore.h"
#include "vphysics/constraints.h"
#include "vcollide_parse.h"
#include "ndebugoverlay.h"
#include "hl2_player.h"
#include "props.h"
#include "vehicle_choreo_generic_shared.h"
#include "ai_utils.h"

#ifdef HOE_DLL
#include "animation.h"
#include "collisionutils.h"
#define HOE_PVCG_WITH_NPCS
#include "BaseAnimatingOverlay.h"
#include "ai_basenpc.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define	VEHICLE_HITBOX_DRIVER		1

#define CHOREO_VEHICLE_VIEW_FOV		90
#define CHOREO_VEHICLE_VIEW_YAW_MIN	-60
#define CHOREO_VEHICLE_VIEW_YAW_MAX	60
#define CHOREO_VEHICLE_VIEW_PITCH_MIN	-90
#define CHOREO_VEHICLE_VIEW_PITCH_MAX	38	

BEGIN_DATADESC_NO_BASE( vehicleview_t )
	DEFINE_FIELD( bClampEyeAngles, FIELD_BOOLEAN ),
	DEFINE_FIELD( flPitchCurveZero, FIELD_FLOAT ),
	DEFINE_FIELD( flPitchCurveLinear, FIELD_FLOAT ),
	DEFINE_FIELD( flRollCurveZero, FIELD_FLOAT ),
	DEFINE_FIELD( flRollCurveLinear, FIELD_FLOAT ),
	DEFINE_FIELD( flFOV, FIELD_FLOAT ),
	DEFINE_FIELD( flYawMin, FIELD_FLOAT ),
	DEFINE_FIELD( flYawMax, FIELD_FLOAT ),
	DEFINE_FIELD( flPitchMin, FIELD_FLOAT ),
	DEFINE_FIELD( flPitchMax, FIELD_FLOAT ),
END_DATADESC()

//
// Anim events.
//
enum
{
	AE_CHOREO_VEHICLE_OPEN = 1,	
	AE_CHOREO_VEHICLE_CLOSE = 2,
};


extern ConVar g_debug_vehicledriver;


class CPropVehicleChoreoGeneric;

static const char *pChoreoGenericFollowerBoneNames[] =
{
	"base",
};


//-----------------------------------------------------------------------------
// Purpose: A KeyValues parse for vehicle sound blocks
//-----------------------------------------------------------------------------
class CVehicleChoreoViewParser : public IVPhysicsKeyHandler
{
public:
	CVehicleChoreoViewParser( void );

private:
	virtual void ParseKeyValue( void *pData, const char *pKey, const char *pValue );
	virtual void SetDefaults( void *pData );
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CChoreoGenericServerVehicle : public CBaseServerVehicle
{
	typedef CBaseServerVehicle BaseClass;

// IServerVehicle
public:
	void GetVehicleViewPosition( int nRole, Vector *pAbsOrigin, QAngle *pAbsAngles, float *pFOV = NULL );
	virtual void ItemPostFrame( CBasePlayer *pPlayer );
#ifdef HOE_DLL
	virtual bool IsPassengerUsingStandardWeapons( int nRole );
	virtual bool PlayerEntryExitAffectsSound( void );
	virtual void ParseEntryExitAnims( void );
#endif
protected:

	CPropVehicleChoreoGeneric *GetVehicle( void );
};


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
#ifdef HOE_PVCG_WITH_NPCS
class CPropVehicleChoreoGeneric : public CDynamicProp, public IDrivableVehicle, public INPCPassengerCarrier
#else
class CPropVehicleChoreoGeneric : public CDynamicProp, public IDrivableVehicle
#endif
{
	DECLARE_CLASS( CPropVehicleChoreoGeneric, CDynamicProp );

public:
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	CPropVehicleChoreoGeneric( void )
	{
		m_ServerVehicle.SetVehicle( this );
		m_bIgnoreMoveParent = false;
		m_bForcePlayerEyePoint = false;
	}

	~CPropVehicleChoreoGeneric( void )
	{
	}

	// CBaseEntity
	virtual void	Precache( void );
	void			Spawn( void );
#ifdef HOE_PVCG_WITH_NPCS
	void			OnRestore( void );
#endif
	void			Think(void);
	virtual int		ObjectCaps( void ) { return BaseClass::ObjectCaps() | FCAP_IMPULSE_USE; };
	virtual void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	virtual void	DrawDebugGeometryOverlays( void );

	virtual Vector	BodyTarget( const Vector &posSrc, bool bNoisy = true );
	virtual void	TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr );
	virtual int		OnTakeDamage( const CTakeDamageInfo &info );

	void			PlayerControlInit( CBasePlayer *pPlayer );
	void			PlayerControlShutdown( void );
	void			ResetUseKey( CBasePlayer *pPlayer );

	virtual bool OverridePropdata() { return true; }

	bool			ParseViewParams( const char *pScriptName );

	void			GetVectors(Vector* pForward, Vector* pRight, Vector* pUp) const;

	bool CreateVPhysics()
	{
		SetSolid(SOLID_VPHYSICS);
		SetMoveType(MOVETYPE_NONE);
		return true;
	}
	bool ShouldForceExit() { return m_bForcedExit; }
	void ClearForcedExit() { m_bForcedExit = false; }

	// CBaseAnimating
	void HandleAnimEvent( animevent_t *pEvent );

	// Inputs
	void InputEnterVehicleImmediate( inputdata_t &inputdata );
	void InputEnterVehicle( inputdata_t &inputdata );
	void InputExitVehicle( inputdata_t &inputdata );
#ifdef HOE_DLL
	void InputExitVehicleImmediate( inputdata_t &inputdata );
#endif
	void InputLock( inputdata_t &inputdata );
	void InputUnlock( inputdata_t &inputdata );
	void InputOpen( inputdata_t &inputdata );
	void InputClose( inputdata_t &inputdata );
	void InputViewlock( inputdata_t &inputdata );

#ifdef HOE_DLL
	virtual bool IsPassengerUsingStandardWeapons( int nRole ) { return false; }
	virtual bool PlayerEntryExitAffectsSound( void ) { return true; }
	virtual void PreParseEntryExitAnims( void ) {};
	virtual void PostParseEntryExitAnims( void ) {};
#endif

	bool ShouldIgnoreParent( void ) { return m_bIgnoreMoveParent; }

	// Tuned to match HL2s definition, but this should probably return false in all cases
	virtual bool	PassengerShouldReceiveDamage( CTakeDamageInfo &info ) { return (info.GetDamageType() & (DMG_BLAST|DMG_RADIATION)) == 0; }

	CNetworkHandle( CBasePlayer, m_hPlayer );

	CNetworkVarEmbedded( vehicleview_t, m_vehicleView );
private:
	vehicleview_t m_savedVehicleView; // gets saved out for viewlock/unlock input

// IDrivableVehicle
public:

	virtual CBaseEntity *GetDriver( void );
	virtual void ProcessMovement( CBasePlayer *pPlayer, CMoveData *pMoveData ) { return; }
	virtual void FinishMove( CBasePlayer *player, CUserCmd *ucmd, CMoveData *move ) { return; }
	virtual bool CanEnterVehicle( CBaseEntity *pEntity );
	virtual bool CanExitVehicle( CBaseEntity *pEntity );
	virtual void SetVehicleEntryAnim( bool bOn );
	virtual void SetVehicleExitAnim( bool bOn, Vector vecEyeExitEndpoint ) { m_bExitAnimOn = bOn; if ( bOn ) m_vecEyeExitEndpoint = vecEyeExitEndpoint; }
	virtual void EnterVehicle( CBaseCombatCharacter *pPassenger );

	virtual bool AllowBlockedExit( CBaseCombatCharacter *pPassenger, int nRole ) { return true; }
	virtual bool AllowMidairExit( CBaseCombatCharacter *pPassenger, int nRole ) { return true; }
	virtual void PreExitVehicle( CBaseCombatCharacter *pPassenger, int nRole ) {}
	virtual void ExitVehicle( int nRole );

	virtual void ItemPostFrame( CBasePlayer *pPlayer ) {}
	virtual void SetupMove( CBasePlayer *player, CUserCmd *ucmd, IMoveHelper *pHelper, CMoveData *move ) {}
	virtual string_t GetVehicleScriptName() { return m_vehicleScript; }

	// If this is a vehicle, returns the vehicle interface
	virtual IServerVehicle *GetServerVehicle() { return &m_ServerVehicle; }

	bool ShouldCollide( int collisionGroup, int contentsMask ) const;

	bool				m_bForcePlayerEyePoint;			// Uses player's eyepoint instead of 'vehicle_driver_eyes' attachment

#ifdef HOE_PVCG_WITH_NPCS
	// INPCPassengerCarrier
	virtual bool	NPC_CanEnterVehicle( CAI_BaseNPC *pPassenger, bool bCompanion );
	virtual bool	NPC_CanExitVehicle( CAI_BaseNPC *pPassenger, bool bCompanion );
	virtual bool	NPC_AddPassenger( CAI_BaseNPC *pPassenger, string_t strRoleName, int nSeatID );
	virtual bool 	NPC_RemovePassenger( CAI_BaseNPC *pPassenger );
	virtual void	NPC_FinishedEnterVehicle( CAI_BaseNPC *pPassenger, bool bCompanion )
	{
		m_npcOn.FireOutput( pPassenger, this );
	}
	virtual void	NPC_FinishedExitVehicle( CAI_BaseNPC *pPassenger, bool bCompanion )
	{
		m_npcOff.FireOutput( pPassenger, this );
	}
#endif

protected:

	// Contained IServerVehicle
	CChoreoGenericServerVehicle m_ServerVehicle;

private:

	// Entering / Exiting
	bool				m_bLocked;
	CNetworkVar( bool,	m_bEnterAnimOn );
	CNetworkVar( bool,	m_bExitAnimOn );
	CNetworkVector(		m_vecEyeExitEndpoint );
	bool				m_bForcedExit;
	bool				m_bIgnoreMoveParent;
	bool				m_bIgnorePlayerCollisions;

	// Vehicle script filename
	string_t			m_vehicleScript;

	COutputEvent		m_playerOn;
	COutputEvent		m_playerOff;
#ifdef HOE_PVCG_WITH_NPCS
	COutputEvent		m_npcOn;
	COutputEvent		m_npcOff;
#endif
	COutputEvent		m_OnOpen;
	COutputEvent		m_OnClose;
};

LINK_ENTITY_TO_CLASS( prop_vehicle_choreo_generic, CPropVehicleChoreoGeneric );

BEGIN_DATADESC( CPropVehicleChoreoGeneric )

	// Inputs
	DEFINE_INPUTFUNC( FIELD_VOID, "Lock",	InputLock ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Unlock",	InputUnlock ),
	DEFINE_INPUTFUNC( FIELD_VOID, "EnterVehicle", InputEnterVehicle ),
	DEFINE_INPUTFUNC( FIELD_VOID, "EnterVehicleImmediate", InputEnterVehicleImmediate ),
	DEFINE_INPUTFUNC( FIELD_VOID, "ExitVehicle", InputExitVehicle ),
#ifdef HOE_DLL
	DEFINE_INPUTFUNC( FIELD_VOID, "ExitVehicleImmediate", InputExitVehicleImmediate ),
#endif
	DEFINE_INPUTFUNC( FIELD_VOID, "Open", InputOpen ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Close", InputClose ),
	DEFINE_INPUTFUNC( FIELD_BOOLEAN, "Viewlock", InputViewlock ),

	// Keys
	DEFINE_EMBEDDED( m_ServerVehicle ),

	DEFINE_FIELD( m_hPlayer, FIELD_EHANDLE ),
	DEFINE_FIELD( m_bEnterAnimOn, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bExitAnimOn, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bForcedExit, FIELD_BOOLEAN ),
 	DEFINE_FIELD( m_vecEyeExitEndpoint, FIELD_POSITION_VECTOR ),

	DEFINE_KEYFIELD( m_vehicleScript, FIELD_STRING, "vehiclescript" ),
	DEFINE_KEYFIELD( m_bLocked, FIELD_BOOLEAN, "vehiclelocked" ),

	DEFINE_KEYFIELD( m_bIgnoreMoveParent, FIELD_BOOLEAN, "ignoremoveparent" ),
	DEFINE_KEYFIELD( m_bIgnorePlayerCollisions, FIELD_BOOLEAN, "ignoreplayer" ),
	DEFINE_KEYFIELD( m_bForcePlayerEyePoint, FIELD_BOOLEAN, "useplayereyes" ),

	DEFINE_OUTPUT( m_playerOn, "PlayerOn" ),
	DEFINE_OUTPUT( m_playerOff, "PlayerOff" ),
#ifdef HOE_PVCG_WITH_NPCS
	DEFINE_OUTPUT( m_npcOn, "OnNPCFinishedEnter" ),
	DEFINE_OUTPUT( m_npcOff, "OnNPCFinishedExit" ),
#endif
	DEFINE_OUTPUT( m_OnOpen, "OnOpen" ),
	DEFINE_OUTPUT( m_OnClose, "OnClose" ),

	DEFINE_EMBEDDED( m_vehicleView ),
	DEFINE_EMBEDDED( m_savedVehicleView ),

END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CPropVehicleChoreoGeneric, DT_PropVehicleChoreoGeneric)
	SendPropEHandle(SENDINFO(m_hPlayer)),
	SendPropBool(SENDINFO(m_bEnterAnimOn)),
	SendPropBool(SENDINFO(m_bExitAnimOn)),
	SendPropVector(SENDINFO(m_vecEyeExitEndpoint), -1, SPROP_COORD),
	SendPropBool( SENDINFO_STRUCTELEM( m_vehicleView.bClampEyeAngles ) ),
	SendPropFloat( SENDINFO_STRUCTELEM( m_vehicleView.flPitchCurveZero ) ),
	SendPropFloat( SENDINFO_STRUCTELEM( m_vehicleView.flPitchCurveLinear ) ),
	SendPropFloat( SENDINFO_STRUCTELEM( m_vehicleView.flRollCurveZero ) ),
	SendPropFloat( SENDINFO_STRUCTELEM( m_vehicleView.flRollCurveLinear ) ),
	SendPropFloat( SENDINFO_STRUCTELEM( m_vehicleView.flFOV ) ),
	SendPropFloat( SENDINFO_STRUCTELEM( m_vehicleView.flYawMin ) ),
	SendPropFloat( SENDINFO_STRUCTELEM( m_vehicleView.flYawMax ) ),
	SendPropFloat( SENDINFO_STRUCTELEM( m_vehicleView.flPitchMin ) ),
	SendPropFloat( SENDINFO_STRUCTELEM( m_vehicleView.flPitchMax ) ),
END_SEND_TABLE();


bool ShouldVehicleIgnoreEntity( CBaseEntity *pVehicle, CBaseEntity *pCollide )
{
	if ( pCollide->GetParent() == pVehicle )
		return true;

	CPropVehicleChoreoGeneric *pChoreoVehicle = dynamic_cast <CPropVehicleChoreoGeneric *>( pVehicle );

	if ( pChoreoVehicle == NULL )
		return false;

	if ( pCollide == NULL )
		return false;

	if ( pChoreoVehicle->ShouldIgnoreParent() == false )
		return false;

	if ( pChoreoVehicle->GetMoveParent() == pCollide )
		return true;
		
	return false;
}


//------------------------------------------------
// Precache
//------------------------------------------------
void CPropVehicleChoreoGeneric::Precache( void )
{
	BaseClass::Precache();

	m_ServerVehicle.Initialize( STRING(m_vehicleScript) );
	m_ServerVehicle.UseLegacyExitChecks( true );
}


//------------------------------------------------
// Spawn
//------------------------------------------------
void CPropVehicleChoreoGeneric::Spawn( void )
{
	Precache();
	SetModel( STRING( GetModelName() ) );
	SetCollisionGroup( COLLISION_GROUP_VEHICLE );

	if ( GetSolid() != SOLID_NONE )
	{
		BaseClass::Spawn();
	}

	m_takedamage = DAMAGE_EVENTS_ONLY;

	SetNextThink( gpGlobals->curtime );

	ParseViewParams( STRING(m_vehicleScript) );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropVehicleChoreoGeneric::TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr )
{
	if ( ptr->hitbox == VEHICLE_HITBOX_DRIVER )
	{
		if ( m_hPlayer != NULL )
		{
			m_hPlayer->TakeDamage( info );
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CPropVehicleChoreoGeneric::OnTakeDamage( const CTakeDamageInfo &inputInfo )
{
	CTakeDamageInfo info = inputInfo;
	info.ScaleDamage( 25 );

	// reset the damage
	info.SetDamage( inputInfo.GetDamage() );

	// Check to do damage to prisoner
	if ( m_hPlayer != NULL )
	{
		// Take no damage from physics damages
		if ( info.GetDamageType() & DMG_CRUSH )
			return 0;

		// Take the damage
		m_hPlayer->TakeDamage( info );
	}

	return 0;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
Vector CPropVehicleChoreoGeneric::BodyTarget( const Vector &posSrc, bool bNoisy )
{
	Vector	shotPos;

	int eyeAttachmentIndex = LookupAttachment("vehicle_driver_eyes");
	GetAttachment( eyeAttachmentIndex, shotPos );

	if ( bNoisy )
	{
		shotPos[0] += random->RandomFloat( -8.0f, 8.0f );
		shotPos[1] += random->RandomFloat( -8.0f, 8.0f );
		shotPos[2] += random->RandomFloat( -8.0f, 8.0f );
	}

	return shotPos;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropVehicleChoreoGeneric::Think(void)
{
	SetNextThink( gpGlobals->curtime + 0.1 );

	if ( GetDriver() )
	{
		BaseClass::Think();
		
		// If the enter or exit animation has finished, tell the server vehicle
		if ( IsSequenceFinished() && (m_bExitAnimOn || m_bEnterAnimOn) )
		{
			GetServerVehicle()->HandleEntryExitFinish( m_bExitAnimOn, true );
		}
	}

	StudioFrameAdvance();
	DispatchAnimEvents( this );
}


//------------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
void CPropVehicleChoreoGeneric::InputOpen( inputdata_t &inputdata )
{
	int nSequence = LookupSequence( "open" );

	// Set to the desired anim, or default anim if the desired is not present
	if ( nSequence > ACTIVITY_NOT_AVAILABLE )
	{
		SetCycle( 0 );
		m_flAnimTime = gpGlobals->curtime;
		ResetSequence( nSequence );
		ResetClientsideFrame();
	}
	else
	{
		// Not available try to get default anim
		Msg( "Choreo Generic Vehicle %s: missing open sequence\n", GetDebugName() );
		SetSequence( 0 );
	}
}


//------------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
void CPropVehicleChoreoGeneric::InputClose( inputdata_t &inputdata )
{
	if ( m_bLocked || m_bEnterAnimOn )
		return;

	int nSequence = LookupSequence( "close" );

	// Set to the desired anim, or default anim if the desired is not present
	if ( nSequence > ACTIVITY_NOT_AVAILABLE )
	{
		SetCycle( 0 );
		m_flAnimTime = gpGlobals->curtime;
		ResetSequence( nSequence );
		ResetClientsideFrame();
	}
	else
	{
		// Not available try to get default anim
		Msg( "Choreo Generic Vehicle %s: missing close sequence\n", GetDebugName() );
		SetSequence( 0 );
	}
}



//------------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
void CPropVehicleChoreoGeneric::InputViewlock( inputdata_t &inputdata )
{
	if (inputdata.value.Bool()) // lock
	{
		if (m_savedVehicleView.flFOV == 0) // not already locked
		{
			m_savedVehicleView = m_vehicleView;
			m_vehicleView.flYawMax = m_vehicleView.flYawMin =  m_vehicleView.flPitchMin = m_vehicleView.flPitchMax = 0.0f;
		}
	}
	else
	{	//unlock
		Assert(m_savedVehicleView.flFOV); // is nonzero if something is saved, is zero if nothing was saved.
		if (m_savedVehicleView.flFOV)
		{
			// m_vehicleView = m_savedVehicleView;
			m_savedVehicleView.flFOV = 0;


			m_vehicleView.flYawMax.Set(  m_savedVehicleView.flYawMax);
			m_vehicleView.flYawMin.Set(  m_savedVehicleView.flYawMin);
			m_vehicleView.flPitchMin.Set(m_savedVehicleView.flPitchMin);
			m_vehicleView.flPitchMax.Set(m_savedVehicleView.flPitchMax);

			/* // note: the straight assignments, as in the lower two lines below, do not call the = overload and thus are never transmitted!
			m_vehicleView.flYawMax = 50;  // m_savedVehicleView.flYawMax;
			m_vehicleView.flYawMin = -50; // m_savedVehicleView.flYawMin;
			m_vehicleView.flPitchMin = m_savedVehicleView.flPitchMin;
			m_vehicleView.flPitchMax = m_savedVehicleView.flPitchMax;
			*/
		}
	}
}




//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropVehicleChoreoGeneric::HandleAnimEvent( animevent_t *pEvent )
{
	if ( pEvent->event == AE_CHOREO_VEHICLE_OPEN )
	{
		m_OnOpen.FireOutput( this, this );
		m_bLocked = false;
	}
	else if ( pEvent->event == AE_CHOREO_VEHICLE_CLOSE )
	{
		m_OnClose.FireOutput( this, this );
		m_bLocked = true;
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropVehicleChoreoGeneric::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	CBasePlayer *pPlayer = ToBasePlayer( pActivator );
	if ( !pPlayer )
		return;

	ResetUseKey( pPlayer );

	GetServerVehicle()->HandlePassengerEntry( pPlayer, (value > 0) );
}


//-----------------------------------------------------------------------------
// Purpose: Return true of the player's allowed to enter / exit the vehicle
//-----------------------------------------------------------------------------
bool CPropVehicleChoreoGeneric::CanEnterVehicle( CBaseEntity *pEntity )
{
	// Prevent entering if the vehicle's being driven by an NPC
	if ( GetDriver() && GetDriver() != pEntity )
		return false;

	// Prevent entering if the vehicle's locked
	return !m_bLocked;
}


//-----------------------------------------------------------------------------
// Purpose: Return true of the player is allowed to exit the vehicle.
//-----------------------------------------------------------------------------
bool CPropVehicleChoreoGeneric::CanExitVehicle( CBaseEntity *pEntity )
{
	// Prevent exiting if the vehicle's locked, rotating, or playing an entry/exit anim.
	return ( !m_bLocked && (GetLocalAngularVelocity() == vec3_angle) && !m_bEnterAnimOn && !m_bExitAnimOn );
}


//-----------------------------------------------------------------------------
// Purpose: Override base class to add display 
//-----------------------------------------------------------------------------
void CPropVehicleChoreoGeneric::DrawDebugGeometryOverlays(void) 
{
	// Draw if BBOX is on
	if ( m_debugOverlays & OVERLAY_BBOX_BIT )
	{
	}

	BaseClass::DrawDebugGeometryOverlays();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropVehicleChoreoGeneric::EnterVehicle( CBaseCombatCharacter *pPassenger )
{
	if ( pPassenger == NULL )
		return;

	CBasePlayer *pPlayer = ToBasePlayer( pPassenger );
	if ( pPlayer != NULL )
	{
		// Remove any player who may be in the vehicle at the moment
		if ( m_hPlayer )
		{
			ExitVehicle( VEHICLE_ROLE_DRIVER );
		}

		m_hPlayer = pPlayer;
		m_playerOn.FireOutput( pPlayer, this, 0 );

#ifdef HOE_DLL
		if ( PlayerEntryExitAffectsSound() )
#endif
		m_ServerVehicle.SoundStart();
	}
	else
	{
		// NPCs not supported yet - jdw
		Assert( 0 );
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropVehicleChoreoGeneric::SetVehicleEntryAnim( bool bOn )
{
	m_bEnterAnimOn = bOn;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropVehicleChoreoGeneric::ExitVehicle( int nRole )
{
	CBasePlayer *pPlayer = m_hPlayer;
	if ( !pPlayer )
		return;

	m_hPlayer = NULL;
	ResetUseKey( pPlayer );

	m_playerOff.FireOutput( pPlayer, this, 0 );
	m_bEnterAnimOn = false;

#ifdef HOE_DLL
	if ( PlayerEntryExitAffectsSound() )
#endif
	m_ServerVehicle.SoundShutdown( 1.0 );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropVehicleChoreoGeneric::ResetUseKey( CBasePlayer *pPlayer )
{
	pPlayer->m_afButtonPressed &= ~IN_USE;
}

#ifdef HOE_PVCG_WITH_NPCS
//-----------------------------------------------------------------------------
// Purpose: Do extra fix-up after restore
//-----------------------------------------------------------------------------
void CPropVehicleChoreoGeneric::OnRestore( void )
{
	BaseClass::OnRestore();

	// NOTE: This is necessary to prevent overflow of datatables on level transition
	// since the last exit eyepoint in the last level will have been fixed up
	// based on the level landmarks, resulting in a position that lies outside
	// typical map coordinates. If we're not in the middle of an exit anim, the
	// eye exit endpoint field isn't being used at all.
	if ( !m_bExitAnimOn )
	{
		m_vecEyeExitEndpoint = GetAbsOrigin();
	}

	IServerVehicle *pServerVehicle = GetServerVehicle();
	if ( pServerVehicle != NULL )
	{
		// Restore the passenger information we're holding on to
		pServerVehicle->RestorePassengerInfo();
	}
}
#endif // HOE_PVCG_WITH_NPCS

//-----------------------------------------------------------------------------
// Purpose: Vehicles are permanently oriented off angle for vphysics.
//-----------------------------------------------------------------------------
void CPropVehicleChoreoGeneric::GetVectors(Vector* pForward, Vector* pRight, Vector* pUp) const
{
	// This call is necessary to cause m_rgflCoordinateFrame to be recomputed
	const matrix3x4_t &entityToWorld = EntityToWorldTransform();

	if (pForward != NULL)
	{
		MatrixGetColumn( entityToWorld, 1, *pForward ); 
	}

	if (pRight != NULL)
	{
		MatrixGetColumn( entityToWorld, 0, *pRight ); 
	}

	if (pUp != NULL)
	{
		MatrixGetColumn( entityToWorld, 2, *pUp ); 
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CBaseEntity *CPropVehicleChoreoGeneric::GetDriver( void ) 
{ 
	return m_hPlayer; 
}

//-----------------------------------------------------------------------------
// Purpose: Prevent the player from entering / exiting the vehicle
//-----------------------------------------------------------------------------
void CPropVehicleChoreoGeneric::InputLock( inputdata_t &inputdata )
{
	m_bLocked = true;
}


//-----------------------------------------------------------------------------
// Purpose: Allow the player to enter / exit the vehicle
//-----------------------------------------------------------------------------
void CPropVehicleChoreoGeneric::InputUnlock( inputdata_t &inputdata )
{
	m_bLocked = false;
}


//-----------------------------------------------------------------------------
// Purpose: Force the player to enter the vehicle.
//-----------------------------------------------------------------------------
void CPropVehicleChoreoGeneric::InputEnterVehicle( inputdata_t &inputdata )
{
	if ( m_bEnterAnimOn )
		return;

	// Try the activator first & use them if they are a player.
	CBasePlayer *pPlayer = ToBasePlayer( inputdata.pActivator );
	if ( pPlayer == NULL )
	{
		// Activator was not a player, just grab the single-player player.
		pPlayer = AI_GetSinglePlayer();
		if ( pPlayer == NULL )
			return;
	}

	// Force us to drop anything we're holding
	pPlayer->ForceDropOfCarriedPhysObjects();

	// FIXME: I hate code like this. I should really add a parameter to HandlePassengerEntry
	//		  to allow entry into locked vehicles
	bool bWasLocked = m_bLocked;
	m_bLocked = false;
	GetServerVehicle()->HandlePassengerEntry( pPlayer, true );
	m_bLocked = bWasLocked;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &inputdata - 
//-----------------------------------------------------------------------------
void CPropVehicleChoreoGeneric::InputEnterVehicleImmediate( inputdata_t &inputdata )
{
	if ( m_bEnterAnimOn )
		return;

	// Try the activator first & use them if they are a player.
	CBasePlayer *pPlayer = ToBasePlayer( inputdata.pActivator );
	if ( pPlayer == NULL )
	{
		// Activator was not a player, just grab the singleplayer player.
		pPlayer = AI_GetSinglePlayer();
		if ( pPlayer == NULL )
			return;
	}

	if ( pPlayer->IsInAVehicle() )
	{
		// Force the player out of whatever vehicle they are in.
		pPlayer->LeaveVehicle();
	}
	
	// Force us to drop anything we're holding
	pPlayer->ForceDropOfCarriedPhysObjects();

	pPlayer->GetInVehicle( GetServerVehicle(), VEHICLE_ROLE_DRIVER );
}

//-----------------------------------------------------------------------------
// Purpose: Force the player to exit the vehicle.
//-----------------------------------------------------------------------------
void CPropVehicleChoreoGeneric::InputExitVehicle( inputdata_t &inputdata )
{
	m_bForcedExit = true;
}

#ifdef HOE_DLL
//-----------------------------------------------------------------------------
void CPropVehicleChoreoGeneric::InputExitVehicleImmediate( inputdata_t &inputdata )
{
	CBasePlayer *pPlayer = m_hPlayer;
	if ( !pPlayer )
		return;

	if ( !CanExitVehicle( pPlayer ) )
		return;

	pPlayer->LeaveVehicle( pPlayer->GetAbsOrigin(), pPlayer->GetAbsAngles() );
	ExitVehicle( VEHICLE_ROLE_DRIVER );
}
#endif

//-----------------------------------------------------------------------------
// Purpose: Parses the vehicle's script for the vehicle view parameters
//-----------------------------------------------------------------------------
bool CPropVehicleChoreoGeneric::ParseViewParams( const char *pScriptName )
{
	byte *pFile = UTIL_LoadFileForMe( pScriptName, NULL );
	if ( !pFile )
		return false;

	IVPhysicsKeyParser *pParse = physcollision->VPhysicsKeyParserCreate( (char *)pFile );
	CVehicleChoreoViewParser viewParser;
	while ( !pParse->Finished() )
	{
		const char *pBlock = pParse->GetCurrentBlockName();
		if ( !strcmpi( pBlock, "vehicle_view" ) )
		{
			pParse->ParseCustom( &m_vehicleView, &viewParser );
		}
		else
		{
			pParse->SkipBlock();
		}
	}
	physcollision->VPhysicsKeyParserDestroy( pParse );
	UTIL_FreeFile( pFile );

	Precache();

	return true;
}

#ifdef HOE_PVCG_WITH_NPCS
//=============================================================================
// Passenger carrier

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPassenger - 
//			bCompanion - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CPropVehicleChoreoGeneric::NPC_CanEnterVehicle( CAI_BaseNPC *pPassenger, bool bCompanion )
{
	// Always allowed unless a leaf class says otherwise
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPassenger - 
//			bCompanion - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CPropVehicleChoreoGeneric::NPC_CanExitVehicle( CAI_BaseNPC *pPassenger, bool bCompanion )
{
	// Always allowed unless a leaf class says otherwise
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPassenger - 
//			bCompanion - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CPropVehicleChoreoGeneric::NPC_AddPassenger( CAI_BaseNPC *pPassenger, string_t strRoleName, int nSeatID )
{
	// Must be allowed to enter
	if ( NPC_CanEnterVehicle( pPassenger, true /*FIXME*/ ) == false )
		return false;

	IServerVehicle *pVehicleServer = GetServerVehicle();
	if ( pVehicleServer != NULL )
		return pVehicleServer->NPC_AddPassenger( pPassenger, strRoleName, nSeatID );

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPassenger - 
//			bCompanion - 
//-----------------------------------------------------------------------------
bool CPropVehicleChoreoGeneric::NPC_RemovePassenger( CAI_BaseNPC *pPassenger )
{
	// Must be allowed to exit
	if ( NPC_CanExitVehicle( pPassenger, true /*FIXME*/ ) == false )
		return false;

	IServerVehicle *pVehicleServer = GetServerVehicle();
	if ( pVehicleServer != NULL )
		return pVehicleServer->NPC_RemovePassenger( pPassenger );

	return true;
}
#endif // HOE_PVCG_WITH_NPCS

//========================================================================================================================================
// CRANE VEHICLE SERVER VEHICLE
//========================================================================================================================================
CPropVehicleChoreoGeneric *CChoreoGenericServerVehicle::GetVehicle( void )
{
	return (CPropVehicleChoreoGeneric *)GetDrivableVehicle();
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : pPlayer - 
//-----------------------------------------------------------------------------
void CChoreoGenericServerVehicle::ItemPostFrame( CBasePlayer *player )
{
	Assert( player == GetDriver() );

	GetDrivableVehicle()->ItemPostFrame( player );

	if (( player->m_afButtonPressed & IN_USE ) || GetVehicle()->ShouldForceExit() )
	{
		GetVehicle()->ClearForcedExit();
		if ( GetDrivableVehicle()->CanExitVehicle(player) )
		{
			// Let the vehicle try to play the exit animation
			if ( !HandlePassengerExit( player ) && ( player != NULL ) )
			{
				player->PlayUseDenySound();
			}
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CChoreoGenericServerVehicle::GetVehicleViewPosition( int nRole, Vector *pAbsOrigin, QAngle *pAbsAngles, float *pFOV /*= NULL*/ )
{
	// FIXME: This needs to be reconciled with the other versions of this function!
	Assert( nRole == VEHICLE_ROLE_DRIVER );
	CBasePlayer *pPlayer = ToBasePlayer( GetDrivableVehicle()->GetDriver() );
	Assert( pPlayer );

	// Use the player's eyes instead of the attachment point
	if ( GetVehicle()->m_bForcePlayerEyePoint )
	{
		// Call to BaseClass because CBasePlayer::EyePosition calls this function.
	  *pAbsOrigin = pPlayer->CBaseCombatCharacter::EyePosition();
	  *pAbsAngles = pPlayer->CBaseCombatCharacter::EyeAngles();
		return;
	}

	*pAbsAngles = pPlayer->EyeAngles(); // yuck. this is an in/out parameter.

	float flPitchFactor = 1.0;
	matrix3x4_t vehicleEyePosToWorld;
	Vector vehicleEyeOrigin;
	QAngle vehicleEyeAngles;
	GetVehicle()->GetAttachment( "vehicle_driver_eyes", vehicleEyeOrigin, vehicleEyeAngles );
	AngleMatrix( vehicleEyeAngles, vehicleEyePosToWorld );

	// Compute the relative rotation between the unperterbed eye attachment + the eye angles
	matrix3x4_t cameraToWorld;
	AngleMatrix( *pAbsAngles, cameraToWorld );

	matrix3x4_t worldToEyePos;
	MatrixInvert( vehicleEyePosToWorld, worldToEyePos );

	matrix3x4_t vehicleCameraToEyePos;
	ConcatTransforms( worldToEyePos, cameraToWorld, vehicleCameraToEyePos );

	// Now perterb the attachment point
	vehicleEyeAngles.x = RemapAngleRange( PITCH_CURVE_ZERO * flPitchFactor, PITCH_CURVE_LINEAR, vehicleEyeAngles.x );
	vehicleEyeAngles.z = RemapAngleRange( ROLL_CURVE_ZERO * flPitchFactor, ROLL_CURVE_LINEAR, vehicleEyeAngles.z );

	AngleMatrix( vehicleEyeAngles, vehicleEyeOrigin, vehicleEyePosToWorld );

	// Now treat the relative eye angles as being relative to this new, perterbed view position...
	matrix3x4_t newCameraToWorld;
	ConcatTransforms( vehicleEyePosToWorld, vehicleCameraToEyePos, newCameraToWorld );

	// output new view abs angles
	MatrixAngles( newCameraToWorld, *pAbsAngles );

	// UNDONE: *pOrigin would already be correct in single player if the HandleView() on the server ran after vphysics
	MatrixGetColumn( newCameraToWorld, 3, *pAbsOrigin );
}

#ifdef HOE_DLL
bool CChoreoGenericServerVehicle::IsPassengerUsingStandardWeapons( int nRole )
{
	return GetVehicle()->IsPassengerUsingStandardWeapons( nRole );
}

bool CChoreoGenericServerVehicle::PlayerEntryExitAffectsSound( void )
{
	return GetVehicle()->PlayerEntryExitAffectsSound();
}

void CChoreoGenericServerVehicle::ParseEntryExitAnims( void )
{
	GetVehicle()->PreParseEntryExitAnims();
	BaseClass::ParseEntryExitAnims();
	GetVehicle()->PostParseEntryExitAnims();
}
#endif // HOE_DLL

bool CPropVehicleChoreoGeneric::ShouldCollide( int collisionGroup, int contentsMask ) const
{
	if ( m_bIgnorePlayerCollisions == true )
	{
		if ( collisionGroup == COLLISION_GROUP_PLAYER || collisionGroup == COLLISION_GROUP_PLAYER_MOVEMENT )
			return false;
	}

	return BaseClass::ShouldCollide( collisionGroup, contentsMask );
}


CVehicleChoreoViewParser::CVehicleChoreoViewParser( void )
{

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CVehicleChoreoViewParser::ParseKeyValue( void *pData, const char *pKey, const char *pValue )
{
	vehicleview_t *pView = (vehicleview_t *)pData;
	// New gear?
	if ( !strcmpi( pKey, "clamp" ) )
	{
		pView->bClampEyeAngles = !!atoi( pValue );
	}
	else if ( !strcmpi( pKey, "pitchcurvezero" ) )
	{
		pView->flPitchCurveZero = atof( pValue );
	}
	else if ( !strcmpi( pKey, "pitchcurvelinear" ) )
	{
		pView->flPitchCurveLinear = atof( pValue );
	}
	else if ( !strcmpi( pKey, "rollcurvezero" ) )
	{
		pView->flRollCurveZero = atof( pValue );
	}
	else if ( !strcmpi( pKey, "rollcurvelinear" ) )
	{
		pView->flRollCurveLinear = atof( pValue );
	}
	else if ( !strcmpi( pKey, "yawmin" ) )
	{
		pView->flYawMin = atof( pValue );
	}
	else if ( !strcmpi( pKey, "yawmax" ) )
	{
		pView->flYawMax = atof( pValue );
	}
	else if ( !strcmpi( pKey, "pitchmin" ) )
	{
		pView->flPitchMin = atof( pValue );
	}
	else if ( !strcmpi( pKey, "pitchmax" ) )
	{
		pView->flPitchMax = atof( pValue );
	}
	else if ( !strcmpi( pKey, "fov" ) )
	{
		pView->flFOV = atof( pValue );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CVehicleChoreoViewParser::SetDefaults( void *pData ) 
{
	vehicleview_t *pView = (vehicleview_t *)pData;

	pView->bClampEyeAngles = true;

	pView->flPitchCurveZero = PITCH_CURVE_ZERO;
	pView->flPitchCurveLinear = PITCH_CURVE_LINEAR;
	pView->flRollCurveZero = ROLL_CURVE_ZERO;
	pView->flRollCurveLinear = ROLL_CURVE_LINEAR;
	pView->flFOV = CHOREO_VEHICLE_VIEW_FOV;
	pView->flYawMin = CHOREO_VEHICLE_VIEW_YAW_MIN;
	pView->flYawMax = CHOREO_VEHICLE_VIEW_YAW_MAX;
	pView->flPitchMin = CHOREO_VEHICLE_VIEW_PITCH_MIN;
	pView->flPitchMax = CHOREO_VEHICLE_VIEW_PITCH_MAX;

}


#ifdef HOE_DLL
class CPropVehicleChoreoTruck : public CPropVehicleChoreoGeneric
{
public:
	DECLARE_CLASS( CPropVehicleChoreoTruck, CPropVehicleChoreoGeneric );
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	virtual bool IsPassengerUsingStandardWeapons( int nRole ) { return m_bUsingStandardWeapons; }
	virtual bool PlayerEntryExitAffectsSound( void ) { return false; }
	virtual void PreParseEntryExitAnims( void );
	virtual void PostParseEntryExitAnims( void );

	void Spawn( void );
	void Precache( void );
	void Activate( void );
	void Think( void );
	void ItemPostFrame( CBasePlayer *pPlayer );
	void EnterVehicle( CBaseCombatCharacter *pPassenger );
	bool CanExitVehicle( CBaseEntity *pEntity );

	int FindEntryHitbox( const Vector &vecEyePoint );
	virtual void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	void PlayLoopingSound( const char *pSoundName );
	void PlaySound( const char *pSoundName );
	void StopLoopingSound( float fadeTime = 0.25f );
	void EmitPlayerUseSound( void ) { /* nothing */ }

	CSoundPatch *m_pStateSound;
	CSoundPatch *m_pStateSoundFade;

	void InputOpenDriverDoor( inputdata_t &inputdata );
	void InputCloseDriverDoor( inputdata_t &inputdata );
	void InputPassengerOpenVehicle( inputdata_t &inputdata );
	void InputPassengerCloseVehicle( inputdata_t &inputdata );
	void InputUnlockDriverSideExit( inputdata_t &inputdata );
	void InputVehicleSound( inputdata_t &inputdata );

	COutputEvent m_driverDoorOpen;
	COutputEvent m_driverDoorClosed;
	COutputEvent m_PlayerAttemptedDriverEntry;

	struct FakeAnimation_t
	{
		bool bRunning;
		float flStartTime;
		float flDuration;
		int iPoseParameter;
		bool bReverse;
	};

	FakeAnimation_t	m_animLean;
	FakeAnimation_t	m_animDriverDoor;
	bool			m_bPlayerLeaning;
	
	CNetworkVar( bool, m_bUsingStandardWeapons );

	bool			m_bDriverSideExitLocked;

	vehicleparams_t m_VehicleParams;
};

BEGIN_DATADESC( CPropVehicleChoreoTruck )
	DEFINE_FIELD( m_animLean.bRunning, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_animLean.flStartTime, FIELD_TIME ),
	DEFINE_FIELD( m_animLean.flDuration, FIELD_FLOAT ),
	DEFINE_FIELD( m_animLean.bReverse, FIELD_BOOLEAN ),

	DEFINE_FIELD( m_animDriverDoor.bRunning, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_animDriverDoor.flStartTime, FIELD_TIME ),
	DEFINE_FIELD( m_animDriverDoor.flDuration, FIELD_FLOAT ),
	DEFINE_FIELD( m_animDriverDoor.bReverse, FIELD_BOOLEAN ),

	DEFINE_FIELD( m_bPlayerLeaning, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bUsingStandardWeapons, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bDriverSideExitLocked, FIELD_BOOLEAN ),

	DEFINE_INPUTFUNC( FIELD_VOID, "OpenDriverDoor", InputOpenDriverDoor ),
	DEFINE_INPUTFUNC( FIELD_VOID, "CloseDriverDoor", InputCloseDriverDoor ),
	DEFINE_INPUTFUNC( FIELD_VOID, "PassengerOpenVehicle", InputPassengerOpenVehicle ),
	DEFINE_INPUTFUNC( FIELD_VOID, "PassengerCloseVehicle", InputPassengerCloseVehicle ),
	DEFINE_INPUTFUNC( FIELD_STRING, "VehicleSound", InputVehicleSound ),
	DEFINE_INPUTFUNC( FIELD_VOID, "UnlockDriverSideExit", InputUnlockDriverSideExit ),

	DEFINE_OUTPUT( m_driverDoorOpen, "OnDriverDoorOpen" ),
	DEFINE_OUTPUT( m_driverDoorClosed, "OnDriverDoorClosed" ),
	DEFINE_OUTPUT( m_PlayerAttemptedDriverEntry, "OnPlayerAttemptedDriverEntry" ),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CPropVehicleChoreoTruck, DT_PropVehicleChoreoTruck)
	SendPropBool(SENDINFO(m_bUsingStandardWeapons)),
END_SEND_TABLE();

LINK_ENTITY_TO_CLASS( prop_vehicle_choreo_truck, CPropVehicleChoreoTruck );

//-----------------------------------------------------------------------------
void CPropVehicleChoreoTruck::Spawn( void )
{
	BaseClass::Spawn();

	// FIXME: create a non-driveable-vehicle model
	SetPoseParameter( "vehicle_wheel_fl_height", 0.5 );
	SetPoseParameter( "vehicle_wheel_fr_height", 0.5 );
	SetPoseParameter( "vehicle_wheel_rl_height", 0.5 );
	SetPoseParameter( "vehicle_wheel_rr_height", 0.5 );

	// The player can't exit through the driver-side door except on namd1.
	m_bDriverSideExitLocked = true;
}

//-----------------------------------------------------------------------------
void CPropVehicleChoreoTruck::Precache( void )
{
	BaseClass::Precache();

	PrecacheScriptSound( "Truck.DoorStartOpen" );
	PrecacheScriptSound( "Truck.DoorEndOpen" );
	PrecacheScriptSound( "Truck.DoorStartClose" );
	PrecacheScriptSound( "Truck.DoorEndClose" );

	PhysFindOrAddVehicleScript( STRING(GetVehicleScriptName()), &m_VehicleParams, NULL );
}

//-----------------------------------------------------------------------------
void CPropVehicleChoreoTruck::Activate( void )
{
	BaseClass::Activate();

	m_animLean.iPoseParameter = LookupPoseParameter( "player_lean" );
	m_animLean.flDuration = 0.5;

	m_animDriverDoor.iPoseParameter = LookupPoseParameter( "driver_door" );
	m_animDriverDoor.flDuration = 0.6;

	m_ServerVehicle.UseLegacyExitChecks( false );

	// When switching from a moving func_tracktrain to a non-moving prop_dynamic
	// parent my local velocity becomes equal to the old func_tracktrain velocity,
	// but it should be zero.  This prevents Barney exiting the truck on namd1
	// when transitioning from namd0.
	if ( GetMoveParent() )
		SetAbsVelocity( GetMoveParent()->GetAbsVelocity() );
}

//-----------------------------------------------------------------------------
// We have to make sure the vehicle_driver_eyes attachment is in its default position
// before getting the entry/exit points.
static float g_oldPose;
void CPropVehicleChoreoTruck::PreParseEntryExitAnims( void )
{
	g_oldPose = GetPoseParameter( m_animLean.iPoseParameter );
	SetPoseParameter( m_animLean.iPoseParameter, 0 );
}

//-----------------------------------------------------------------------------
void CPropVehicleChoreoTruck::PostParseEntryExitAnims( void )
{
	SetPoseParameter( m_animLean.iPoseParameter, g_oldPose );
}

//-----------------------------------------------------------------------------
void CPropVehicleChoreoTruck::Think( void )
{
	if ( m_animLean.bRunning )
	{
		float flFrac = ( gpGlobals->curtime - m_animLean.flStartTime ) / m_animLean.flDuration;
		flFrac = clamp( flFrac, 0.0f, 1.0f );
		if ( m_animLean.bReverse )
			SetPoseParameter( m_animLean.iPoseParameter, 1.0 - flFrac );
		else
			SetPoseParameter( m_animLean.iPoseParameter, flFrac );
		if ( flFrac == 1.0 )
		{
			m_animLean.bRunning = false;
			m_bPlayerLeaning = !m_bPlayerLeaning;
		}
	}

	if ( m_animDriverDoor.bRunning )
	{
		float flFrac = ( gpGlobals->curtime - m_animDriverDoor.flStartTime ) / m_animDriverDoor.flDuration;
		flFrac = clamp( flFrac, 0.0f, 1.0f );
		if ( m_animDriverDoor.bReverse )
			SetPoseParameter( m_animDriverDoor.iPoseParameter, 1.0 - flFrac );
		else
			SetPoseParameter( m_animDriverDoor.iPoseParameter, flFrac );
		if ( flFrac == 1.0 )
		{
			m_animDriverDoor.bRunning = false;
			if ( m_animDriverDoor.bReverse )
			{
				EmitSound( "Truck.DoorEndClose" );
				m_driverDoorClosed.FireOutput( this, this );
			}
			else
			{
				EmitSound( "Truck.DoorEndOpen" );
				m_driverDoorOpen.FireOutput( this, this );
			}
		}
	}

//	UpdateVehicleSounds();

	BaseClass::Think();
}

//-----------------------------------------------------------------------------
void CPropVehicleChoreoTruck::ItemPostFrame( CBasePlayer *pPlayer )
{
	BaseClass::ItemPostFrame( pPlayer );

	if ( pPlayer->m_afButtonPressed & IN_JUMP )
	{
		if ( !m_animLean.bRunning )
		{
			m_animLean.bRunning = true;
			m_animLean.bReverse = m_bPlayerLeaning;
			m_animLean.flStartTime = gpGlobals->curtime;

			if ( m_bPlayerLeaning )
			{
				CBaseCombatWeapon *pWeapon = pPlayer->GetActiveWeapon();
				if ( pWeapon != NULL )
				{
					pWeapon->Holster( NULL );
				}
				pPlayer->m_Local.m_iHideHUD |= HIDEHUD_INVEHICLE;
				pPlayer->ShowCrosshair( false );
				m_bUsingStandardWeapons = false;
			}
			else
			{
				CBaseCombatWeapon *pWeapon = pPlayer->GetActiveWeapon();
				if ( pWeapon != NULL && pWeapon->IsWeaponVisible() == false )
				{
					pWeapon->Deploy();
				}
				pPlayer->m_Local.m_iHideHUD &= ~HIDEHUD_INVEHICLE;
				pPlayer->ShowCrosshair( true );
				m_bUsingStandardWeapons = true;
			}
		}
	}
}

//-----------------------------------------------------------------------------
void CPropVehicleChoreoTruck::EnterVehicle( CBaseCombatCharacter *pPassenger )
{
	SetPoseParameter( "player_lean", 0.0 );
	m_bPlayerLeaning = false;
	m_bUsingStandardWeapons = false;

	BaseClass::EnterVehicle( pPassenger );
}

//-----------------------------------------------------------------------------
bool CPropVehicleChoreoTruck::CanExitVehicle( CBaseEntity *pEntity )
{
	if ( m_bPlayerLeaning || m_animLean.bRunning )
		return false;
	return BaseClass::CanExitVehicle( pEntity );
}

//-----------------------------------------------------------------------------
void CPropVehicleChoreoTruck::InputOpenDriverDoor( inputdata_t &inputdata )
{
	if ( !m_animDriverDoor.bRunning )
	{
		m_animDriverDoor.bRunning = true;
		m_animDriverDoor.bReverse = false;
		m_animDriverDoor.flStartTime = gpGlobals->curtime;
//		SetPoseParameter( m_animDriverDoor.iPoseParameter, 0.0f );
		EmitSound( "Truck.DoorStartOpen" );
	}
}

//-----------------------------------------------------------------------------
void CPropVehicleChoreoTruck::InputCloseDriverDoor( inputdata_t &inputdata )
{
	if ( !m_animDriverDoor.bRunning )
	{
		m_animDriverDoor.bRunning = true;
		m_animDriverDoor.bReverse = true;
		m_animDriverDoor.flStartTime = gpGlobals->curtime;
//		SetPoseParameter( m_animDriverDoor.iPoseParameter, 1.0f );
		EmitSound( "Truck.DoorEndOpen" );
	}
}

//-----------------------------------------------------------------------------
void CPropVehicleChoreoTruck::InputPassengerOpenVehicle( inputdata_t &inputdata )
{
	InputOpenDriverDoor( inputdata );
}

//-----------------------------------------------------------------------------
void CPropVehicleChoreoTruck::InputPassengerCloseVehicle( inputdata_t &inputdata )
{
	InputCloseDriverDoor( inputdata );
}

//-----------------------------------------------------------------------------
void CPropVehicleChoreoTruck::InputUnlockDriverSideExit( inputdata_t &inputdata )
{
	m_bDriverSideExitLocked = false;
}

extern const char *pSoundStateNames[];
const char *pVehicleSoundNames[] =
{
	"VS_SKID_FRICTION_LOW",
	"VS_SKID_FRICTION_NORMAL",
	"VS_SKID_FRICTION_HIGH",
	"VS_ENGINE2_START",
	"VS_ENGINE2_STOP",
	"VS_MISC1",
	"VS_MISC2",
	"VS_MISC3",
	"VS_MISC4"
};

//-----------------------------------------------------------------------------
void CPropVehicleChoreoTruck::InputVehicleSound( inputdata_t &inputdata )
{
	const char *s = inputdata.value.String();

	for ( int i = 0; i < SS_NUM_STATES; i++ )
	{
		if ( !strcmpi( pSoundStateNames[i], s ) )
		{
			switch ( i )
			{
			case SS_START_IDLE:
				{
					const char *pSoundName = m_ServerVehicle.m_vehicleSounds.iszStateSounds[SS_START_IDLE].ToCStr();
					PlayLoopingSound( pSoundName );
				}
				break;
			case SS_SHUTDOWN:
				{
					StopLoopingSound();
					const char *pSoundName = m_ServerVehicle.m_vehicleSounds.iszStateSounds[SS_SHUTDOWN].ToCStr();
					PlaySound( pSoundName );
				}
				break;
			default:
				{
					const char *pSoundName = m_ServerVehicle.m_vehicleSounds.iszStateSounds[(vehiclesound)i].ToCStr();
					PlayLoopingSound( pSoundName );
				}
				break;
			}
			return;
		}
	}
	for ( int i = 0; i < VS_NUM_SOUNDS; i++ )
	{
		if ( !strcmpi( pVehicleSoundNames[i], s ) )
		{
			const char *pSoundName = m_ServerVehicle.m_vehicleSounds.iszSound[(vehiclesound)i].ToCStr();
			if ( i == VS_MISC2 ) // classic engine loop
				PlayLoopingSound( pSoundName );
			else
				PlaySound( pSoundName );
			return;
		}
	}

	DevWarning( "CPropVehicleChoreoTruck::InputVehicleSound unknown sound \"%s\"\n", s );
}

//-----------------------------------------------------------------------------
void CPropVehicleChoreoTruck::PlayLoopingSound( const char *pSoundName )
{
	CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();

	CPASAttenuationFilter filter( this );
	CSoundPatch *pNewSound = NULL;
	if ( pSoundName && pSoundName[0] )
	{
		pNewSound = controller.SoundCreate( filter, entindex(), CHAN_STATIC, pSoundName, ATTN_NORM );
	}

	if ( m_pStateSound && pNewSound && controller.SoundGetName( pNewSound ) == controller.SoundGetName( m_pStateSound ) )
	{
		// if the sound is the same, don't play this, just re-use the old one
		controller.SoundDestroy( pNewSound );
		pNewSound = m_pStateSound;
		controller.SoundChangeVolume( pNewSound, 1.0f, 0.0f );
		m_pStateSound = NULL;
	}
/*
	else if ( g_debug_vehiclesound.GetInt() )
	{
		const char *pStopSound = m_pStateSound ?  controller.SoundGetName( m_pStateSound ).ToCStr() : "NULL";
		const char *pStartSound = pNewSound ?  controller.SoundGetName( pNewSound ).ToCStr() : "NULL";
		Msg("Stop %s, start %s\n", pStopSound, pStartSound );
	}
*/
	StopLoopingSound();
	m_pStateSound = pNewSound;
	if ( m_pStateSound )
	{
		controller.Play( m_pStateSound, 1.0f, 100 );
	}
}

//-----------------------------------------------------------------------------
void CPropVehicleChoreoTruck::PlaySound( const char *pSoundName )
{
	if ( pSoundName && pSoundName[0] )
	{
		CPASAttenuationFilter filter( this );

		EmitSound_t ep;
		ep.m_nChannel = CHAN_VOICE;
		ep.m_pSoundName = pSoundName;
		ep.m_flVolume = 1.0/*m_flVehicleVolume*/;
		ep.m_SoundLevel = SNDLVL_NORM;

		CBaseEntity::EmitSound( filter, entindex(), ep );
/*
		if ( g_debug_vehiclesound.GetInt() )
		{
			Msg("Playing vehicle sound: %s\n", ep.m_pSoundName );
		}
*/
	}
}

//-----------------------------------------------------------------------------
void CPropVehicleChoreoTruck::StopLoopingSound( float fadeTime )
{
	CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
	if ( m_pStateSoundFade )
	{
		controller.SoundDestroy( m_pStateSoundFade );
		m_pStateSoundFade = NULL;
	}
	if ( m_pStateSound )
	{
		m_pStateSoundFade = m_pStateSound;
		m_pStateSound = NULL;
		controller.SoundFadeOut( m_pStateSoundFade, fadeTime, false );
	}
}

//-----------------------------------------------------------------------------
int CPropVehicleChoreoTruck::FindEntryHitbox( const Vector &vecEyePoint )
{
	// Figure out which entrypoint hitbox the player is in
	CBaseAnimating *pAnimating = this;

	CStudioHdr *pStudioHdr = pAnimating->GetModelPtr();
	if (!pStudioHdr)
		return -1;
	int iHitboxSet = FindHitboxSetByName( pStudioHdr, "entryboxes" );
	mstudiohitboxset_t *set = pStudioHdr->pHitboxSet( iHitboxSet );
	if ( !set || !set->numhitboxes )
		return -1;

	// Loop through the hitboxes and find out which one we're in
	for ( int i = 0; i < set->numhitboxes; i++ )
	{
		mstudiobbox_t *pbox = set->pHitbox( i );

		Vector vecPosition;
		QAngle vecAngles;
		pAnimating->GetBonePosition( pbox->bone, vecPosition, vecAngles );

		// Build a rotation matrix from orientation
		matrix3x4_t fRotateMatrix;
		AngleMatrix( vecAngles, vecPosition, fRotateMatrix);

		Vector localEyePoint;
		VectorITransform( vecEyePoint, fRotateMatrix, localEyePoint );
		if ( IsPointInBox( localEyePoint, pbox->bbmin, pbox->bbmax ) )
		{
			return i;
		}
	}

	return -1;
}

//-----------------------------------------------------------------------------
void CPropVehicleChoreoTruck::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	CBasePlayer *pPlayer = ToBasePlayer( pActivator );
	if ( !pPlayer )
		return;

	ResetUseKey( pPlayer );

	// See if the player is attempting to enter through the driver-side door.
	// If so we want to inform him that he lacks the intelligence to drive.
	int hitbox = FindEntryHitbox( pPlayer->EyePosition() );
	if ( hitbox == 1 )
	{
		m_PlayerAttemptedDriverEntry.FireOutput( this, this );
		return;
	}

	GetServerVehicle()->HandlePassengerEntry( pPlayer, (value > 0) );
}

//-----------------------------------------------------------------------------
bool IsVehicleExitLocked( CBaseServerVehicle *pServerVehicle, int nExitAnim )
{
	CBaseEntity *pVehicleEnt = pServerVehicle->GetVehicleEnt();
	if ( pVehicleEnt == NULL )
		return false;

	CPropVehicleChoreoTruck *pChoreoVehicle = dynamic_cast <CPropVehicleChoreoTruck *>( pVehicleEnt );
	if ( pChoreoVehicle == NULL )
		return false;

	if ( nExitAnim == 1 && pChoreoVehicle->m_bDriverSideExitLocked )
		return true;

	return false;
}

#endif // HOE_DLL
