#include "cbase.h"
#include "engine/IEngineSound.h"
#include "in_buttons.h"
//#include "ammodef.h"
//#include "IEffects.h"
//#include "beam_shared.h"
//#include "weapon_gauss.h"
#include "soundenvelope.h"
//#include "decals.h"
#include "soundent.h"
//#include "grenade_ar2.h"
#include "te_effect_dispatch.h"
#include "hl2_player.h"
#include "ndebugoverlay.h"
#include "movevars_shared.h"
#include "bone_setup.h"
#include "ai_basenpc.h"
#include "ai_hint.h"
#include "npc_crow.h"
#include "globalstate.h"
#include "vehicle_truck.h"
#include "rumble_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define	VEHICLE_HITBOX_DRIVER		1
#define LOCK_SPEED					10

#define CANNON_MAX_UP_PITCH			20
#define CANNON_MAX_DOWN_PITCH		20
#define CANNON_MAX_LEFT_YAW			90
#define CANNON_MAX_RIGHT_YAW		90

#define OVERTURNED_EXIT_WAITTIME	2.0f

#define TRUCK_AMMOCRATE_HITGROUP		5

#define TRUCK_STEERING_SLOW_ANGLE	50.0f
#define TRUCK_STEERING_FAST_ANGLE	15.0f

#define	TRUCK_AMMO_CRATE_CLOSE_DELAY	2.0f

#define TRUCK_DELTA_LENGTH_MAX	12.0f			// 1 foot
#define TRUCK_FRAMETIME_MIN		1e-6

// Seagull perching
const char *g_pTruckThinkContext = "TruckSeagullThink";
#define	TRUCK_SEAGULL_THINK_INTERVAL		10.0		// Interval between checks for seagull perches
#define	TRUCK_SEAGULL_POOP_INTERVAL		45.0		// Interval between checks for seagull poopage
#define TRUCK_SEAGULL_HIDDEN_TIME		15.0		// Time for which the player must be hidden from the jeep for a seagull to perch
#define TRUCK_SEAGULL_MAX_TIME			60.0		// Time at which a seagull will definately perch on the jeep

ConVar	hud_truckhint_numentries( "hud_truckhint_numentries", "10", FCVAR_NONE );
ConVar	g_truckexitspeed( "g_truckexitspeed", "100", FCVAR_CHEAT );

ConVar r_TruckViewDampenFreq( "r_TruckViewDampenFreq", "7.0", FCVAR_CHEAT | FCVAR_NOTIFY | FCVAR_REPLICATED );
ConVar r_TruckViewDampenDamp( "r_TruckViewDampenDamp", "1.0", FCVAR_CHEAT | FCVAR_NOTIFY | FCVAR_REPLICATED);
ConVar r_TruckViewZHeight( "r_TruckViewZHeight", "0.0", FCVAR_CHEAT | FCVAR_NOTIFY | FCVAR_REPLICATED );

//=============================================================================
//
// Truck water data.
//

BEGIN_SIMPLE_DATADESC( TruckWaterData_t )
	DEFINE_ARRAY( m_bWheelInWater,			FIELD_BOOLEAN,	TRUCK_WHEEL_COUNT ),
	DEFINE_ARRAY( m_bWheelWasInWater,			FIELD_BOOLEAN,	TRUCK_WHEEL_COUNT ),
	DEFINE_ARRAY( m_vecWheelContactPoints,	FIELD_VECTOR,	TRUCK_WHEEL_COUNT ),
	DEFINE_ARRAY( m_flNextRippleTime,			FIELD_TIME,		TRUCK_WHEEL_COUNT ),
	DEFINE_FIELD( m_bBodyInWater,				FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bBodyWasInWater,			FIELD_BOOLEAN ),
END_DATADESC()	

//-----------------------------------------------------------------------------
// Purpose: Four wheel physics vehicle server vehicle with weaponry
//-----------------------------------------------------------------------------
class CTruckFourWheelServerVehicle : public CFourWheelServerVehicle
{
	typedef CFourWheelServerVehicle BaseClass;

// IVehicle
public:
	virtual bool IsPassengerUsingStandardWeapons( int nRole = VEHICLE_ROLE_DRIVER );
	
// IServerVehicle
public:
	void			GetVehicleViewPosition( int nRole, Vector *pAbsOrigin, QAngle *pAbsAngles, float *pFOV = NULL );
	virtual float	GetVehicleFOV( void );
	virtual	void	ItemPostFrame( CBasePlayer *pPlayer );
	int				GetExitAnimToUse( Vector &vecEyeExitEndpoint, bool &bAllPointsBlocked );

protected:
	CPropTruck		*GetVehicle( void );
};

BEGIN_DATADESC( CPropTruck )
	DEFINE_FIELD( m_flDangerSoundTime, FIELD_TIME ),
	DEFINE_FIELD( m_throttleDisableTime, FIELD_TIME ),
	DEFINE_FIELD( m_flHandbrakeTime, FIELD_TIME ),
	DEFINE_FIELD( m_bInitialHandbrake, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flOverturnedTime, FIELD_TIME ),
	DEFINE_FIELD( m_vecLastEyePos, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( m_vecLastEyeTarget, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( m_vecEyeSpeed, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( m_vecTargetSpeed, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( m_bHeadlightIsOn, FIELD_BOOLEAN ),
	DEFINE_EMBEDDED( m_WaterData ),

	DEFINE_FIELD( m_iNumberOfEntries, FIELD_INTEGER ),

	DEFINE_FIELD( m_flPlayerExitedTime, FIELD_TIME ),
	DEFINE_FIELD( m_flLastSawPlayerAt, FIELD_TIME ),
	DEFINE_FIELD( m_hLastPlayerInVehicle, FIELD_EHANDLE ),
	DEFINE_FIELD( m_hSeagull, FIELD_EHANDLE ),
	DEFINE_FIELD( m_bHasPoop, FIELD_BOOLEAN ),

	DEFINE_FIELD( m_bIsPlayerDriving, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bForcedExit, FIELD_BOOLEAN ),

	DEFINE_INPUTFUNC( FIELD_VOID, "ShowHudHint", InputShowHudHint ),
	DEFINE_INPUTFUNC( FIELD_VOID, "EnterVehicle", InputEnterVehicle ),
	DEFINE_INPUTFUNC( FIELD_VOID, "EnterVehicleImmediate", InputEnterVehicleImmediate ),
	DEFINE_INPUTFUNC( FIELD_VOID, "ExitVehicle", InputExitVehicle ),
	DEFINE_INPUTFUNC( FIELD_VOID, "ExitVehicleImmediate", InputExitVehicleImmediate ),

	DEFINE_THINKFUNC( TruckSeagullThink ),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CPropTruck, DT_PropTruck )
	SendPropBool( SENDINFO( m_bHeadlightIsOn ) ),
	SendPropBool( SENDINFO( m_bIsPlayerDriving ) ),
END_SEND_TABLE();

LINK_ENTITY_TO_CLASS( prop_vehicle_truck, CPropTruck );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CPropTruck::CPropTruck( void )
{
	m_flOverturnedTime = 0.0f;
	m_iNumberOfEntries = 0;

	m_vecEyeSpeed.Init();

	InitWaterData();

	m_bUnableToFire = true;

	m_bIsPlayerDriving = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropTruck::CreateServerVehicle( void )
{
	// Create our armed server vehicle
	m_pServerVehicle = new CTruckFourWheelServerVehicle();
	m_pServerVehicle->SetVehicle( this );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropTruck::Precache( void )
{
	UTIL_PrecacheOther( "npc_seagull" );

	BaseClass::Precache();
}

//------------------------------------------------
// Spawn
//------------------------------------------------
void CPropTruck::Spawn( void )
{
	// Setup vehicle as a real-wheels car.
	SetVehicleType( VEHICLE_TYPE_CAR_WHEELS );

	BaseClass::Spawn();
	m_flHandbrakeTime = gpGlobals->curtime + 0.1;
	m_bInitialHandbrake = false;

	m_flMinimumSpeedToEnterExit = LOCK_SPEED;

	AddSolidFlags( FSOLID_NOT_STANDABLE );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CPropTruck::Activate()
{
	BaseClass::Activate();

	CBaseServerVehicle *pServerVehicle = dynamic_cast<CBaseServerVehicle *>(GetServerVehicle());
	if ( pServerVehicle )
	{
		if( pServerVehicle->GetPassenger() )
		{
			// If a jeep comes back from a save game with a driver, make sure the engine rumble starts up.
			pServerVehicle->StartEngineRumble();
		}
	}
}

//-----------------------------------------------------------------------------
void CPropTruck::VPhysicsUpdate( IPhysicsObject *pPhysics )
{
	// Parented to a tracktrain
	// Should be MOVETYPE_NONE?
	if ( GetMoveParent() )
	{
		if ( GetMoveType() != MOVETYPE_NONE )
		{
			SetMoveType( MOVETYPE_NONE );
			VPhysicsGetObject()->Sleep();
		}
		return;
	}
	else if ( GetMoveType() == MOVETYPE_NONE )
	{
		SetMoveType( MOVETYPE_VPHYSICS );
			VPhysicsGetObject()->Wake();
	}

	BaseClass::VPhysicsUpdate( pPhysics );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropTruck::TraceAttack( const CTakeDamageInfo &inputInfo, const Vector &vecDir, trace_t *ptr )
{
	CTakeDamageInfo info = inputInfo;
	if ( ptr->hitbox != VEHICLE_HITBOX_DRIVER )
	{
		if ( inputInfo.GetDamageType() & DMG_BULLET )
		{
			info.ScaleDamage( 0.0001 );
		}
	}

	BaseClass::TraceAttack( info, vecDir, ptr, 0 );
}

//-----------------------------------------------------------------------------
// Purpose: Modifies the passenger's damage taken through us
//-----------------------------------------------------------------------------
float CPropTruck::PassengerDamageModifier( const CTakeDamageInfo &info )
{
	if ( info.GetInflictor() && FClassnameIs( info.GetInflictor(), "hunter_flechette" ) )
		return 0.1f;

	return 1.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CPropTruck::OnTakeDamage( const CTakeDamageInfo &inputInfo )
{
	//Do scaled up physics damage to the car
	CTakeDamageInfo info = inputInfo;
	info.ScaleDamage( 25 );
	
	// HACKHACK: Scale up grenades until we get a better explosion/pressure damage system
	if ( inputInfo.GetDamageType() & DMG_BLAST )
	{
		info.SetDamageForce( inputInfo.GetDamageForce() * 10 );
	}
	VPhysicsTakeDamage( info );

	// reset the damage
	info.SetDamage( inputInfo.GetDamage() );

	// small amounts of shock damage disrupt the car, but aren't transferred to the player
	if ( info.GetDamageType() == DMG_SHOCK )
	{
		if ( info.GetDamage() <= 10 )
		{
			// take 10% damage and make the engine stall
			info.ScaleDamage( 0.1 );
			m_throttleDisableTime = gpGlobals->curtime + 2;
		}
	}

	//Check to do damage to driver
	if ( GetDriver() )
	{
		// Never take crush damage
		if ( info.GetDamageType() & DMG_CRUSH )
			return 0;

		// Scale the damage and mark that we're passing it in so the base player accepts the damage
		info.ScaleDamage( PassengerDamageModifier( info ) );
		info.SetDamageType( info.GetDamageType() | DMG_VEHICLE );
		
		// Deal the damage to the passenger
		GetDriver()->TakeDamage( info );
	}

	return 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
Vector CPropTruck::BodyTarget( const Vector &posSrc, bool bNoisy )
{
	Vector	shotPos;
	matrix3x4_t	matrix;

	int eyeAttachmentIndex = LookupAttachment("vehicle_driver_eyes");
	GetAttachment( eyeAttachmentIndex, matrix );
	MatrixGetColumn( matrix, 3, shotPos );

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
void CPropTruck::InitWaterData( void )
{
	m_WaterData.m_bBodyInWater = false;
	m_WaterData.m_bBodyWasInWater = false;

	for ( int iWheel = 0; iWheel < TRUCK_WHEEL_COUNT; ++iWheel )
	{
		m_WaterData.m_bWheelInWater[iWheel] = false;
		m_WaterData.m_bWheelWasInWater[iWheel] = false;
		m_WaterData.m_vecWheelContactPoints[iWheel].Init();
		m_WaterData.m_flNextRippleTime[iWheel] = 0;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropTruck::HandleWater( void )
{
	// Only check the wheels and engine in water if we have a driver (player).
	if ( !GetDriver() )
		return;

	// Check to see if we are in water.
	if ( CheckWater() )
	{
		for ( int iWheel = 0; iWheel < TRUCK_WHEEL_COUNT; ++iWheel )
		{
			// Create an entry/exit splash!
			if ( m_WaterData.m_bWheelInWater[iWheel] != m_WaterData.m_bWheelWasInWater[iWheel] )
			{
				CreateSplash( m_WaterData.m_vecWheelContactPoints[iWheel] );
				CreateRipple( m_WaterData.m_vecWheelContactPoints[iWheel] );
			}
			
			// Create ripples.
			if ( m_WaterData.m_bWheelInWater[iWheel] && m_WaterData.m_bWheelWasInWater[iWheel] )
			{
				if ( m_WaterData.m_flNextRippleTime[iWheel] < gpGlobals->curtime )
				{
					// Stagger ripple times
					m_WaterData.m_flNextRippleTime[iWheel] = gpGlobals->curtime + RandomFloat( 0.1, 0.3 );
					CreateRipple( m_WaterData.m_vecWheelContactPoints[iWheel] );
				}
			}
		}
	}

	// Save of data from last think.
	for ( int iWheel = 0; iWheel < TRUCK_WHEEL_COUNT; ++iWheel )
	{
		m_WaterData.m_bWheelWasInWater[iWheel] = m_WaterData.m_bWheelInWater[iWheel];
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CPropTruck::CheckWater( void )
{
	bool bInWater = false;

	// Check all four wheels.
	for ( int iWheel = 0; iWheel < TRUCK_WHEEL_COUNT; ++iWheel )
	{
		// Get the current wheel and get its contact point.
		IPhysicsObject *pWheel = m_VehiclePhysics.GetWheel( iWheel );
		if ( !pWheel )
			continue;

		// Check to see if we hit water.
		if ( pWheel->GetContactPoint( &m_WaterData.m_vecWheelContactPoints[iWheel], NULL ) )
		{
			m_WaterData.m_bWheelInWater[iWheel] = ( UTIL_PointContents( m_WaterData.m_vecWheelContactPoints[iWheel] ) & MASK_WATER ) ? true : false;
			if ( m_WaterData.m_bWheelInWater[iWheel] )
			{
				bInWater = true;
			}
		}
	}

	// Check the body and the BONNET.
	int iEngine = LookupAttachment( "vehicle_engine" );
	Vector vecEnginePoint;
	QAngle vecEngineAngles;
	GetAttachment( iEngine, vecEnginePoint, vecEngineAngles );

	m_WaterData.m_bBodyInWater = ( UTIL_PointContents( vecEnginePoint ) & MASK_WATER ) ? true : false;
	if ( m_WaterData.m_bBodyInWater )
	{
		if ( m_bHasPoop )
		{
			RemoveAllDecals();
			m_bHasPoop = false;
		}

		if ( !m_VehiclePhysics.IsEngineDisabled() )
		{
			m_VehiclePhysics.SetDisableEngine( true );
		}
	}
	else
	{
		if ( m_VehiclePhysics.IsEngineDisabled() )
		{
			m_VehiclePhysics.SetDisableEngine( false );
		}
	}

	if ( bInWater )
	{
		// Check the player's water level.
		CheckWaterLevel();
	}

	return bInWater;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropTruck::CheckWaterLevel( void )
{
	CBaseEntity *pEntity = GetDriver();
	if ( pEntity && pEntity->IsPlayer() )
	{
		CBasePlayer *pPlayer = static_cast<CBasePlayer*>( pEntity );
		
		Vector vecAttachPoint;
		QAngle vecAttachAngles;
		
		// Check eyes. (vehicle_driver_eyes point)
		int iAttachment = LookupAttachment( "vehicle_driver_eyes" );
		GetAttachment( iAttachment, vecAttachPoint, vecAttachAngles );

		// Add the jeep's Z view offset
		Vector vecUp;
		AngleVectors( vecAttachAngles, NULL, NULL, &vecUp );
		vecUp.z = clamp( vecUp.z, 0.0f, vecUp.z );
		vecAttachPoint.z += r_TruckViewZHeight.GetFloat() * vecUp.z;

		bool bEyes = ( UTIL_PointContents( vecAttachPoint ) & MASK_WATER ) ? true : false;
		if ( bEyes )
		{
			pPlayer->SetWaterLevel( WL_Eyes );
			return;
		}

		// Check waist.  (vehicle_engine point -- see parent function).
		if ( m_WaterData.m_bBodyInWater )
		{
			pPlayer->SetWaterLevel( WL_Waist );
			return;
		}

		// Check feet. (vehicle_feet_passenger0 point)
		iAttachment = LookupAttachment( "vehicle_feet_passenger0" );
		GetAttachment( iAttachment, vecAttachPoint, vecAttachAngles );
		bool bFeet = ( UTIL_PointContents( vecAttachPoint ) & MASK_WATER ) ? true : false;
		if ( bFeet )
		{
			pPlayer->SetWaterLevel( WL_Feet );
			return;
		}

		// Not in water.
		pPlayer->SetWaterLevel( WL_NotInWater );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropTruck::CreateSplash( const Vector &vecPosition )
{
	// Splash data.
	CEffectData	data;
	data.m_fFlags = 0;
	data.m_vOrigin = vecPosition;
	data.m_vNormal.Init( 0.0f, 0.0f, 1.0f );
	VectorAngles( data.m_vNormal, data.m_vAngles );
	data.m_flScale = 10.0f + random->RandomFloat( 0, 2 );

	// Create the splash..
	DispatchEffect( "watersplash", data );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropTruck::CreateRipple( const Vector &vecPosition )
{
	// Ripple data.
	CEffectData	data;
	data.m_fFlags = 0;
	data.m_vOrigin = vecPosition;
	data.m_vNormal.Init( 0.0f, 0.0f, 1.0f );
	VectorAngles( data.m_vNormal, data.m_vAngles );
	data.m_flScale = 10.0f + random->RandomFloat( 0, 2 );
	if ( GetWaterType() & CONTENTS_SLIME )
	{
		data.m_fFlags |= FX_WATER_IN_SLIME;
	}

	// Create the ripple.
	DispatchEffect( "waterripple", data );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropTruck::Think(void)
{
	BaseClass::Think();

	CBasePlayer	*pPlayer = UTIL_GetLocalPlayer();

	if ( m_bEngineLocked )
	{		
		if ( pPlayer != NULL )
		{
			pPlayer->m_Local.m_iHideHUD |= HIDEHUD_VEHICLE_CROSSHAIR;
		}
	}

	// Water!?
	HandleWater();

	SetSimulationTime( gpGlobals->curtime );
	
	SetNextThink( gpGlobals->curtime );
	SetAnimatedEveryTick( true );

    if ( !m_bInitialHandbrake )	// after initial timer expires, set the handbrake
	{
		m_bInitialHandbrake = true;
		m_VehiclePhysics.SetHandbrake( true );
		m_VehiclePhysics.Think();
	}

	// Check overturned status.
	if ( !IsOverturned() )
	{
		m_flOverturnedTime = 0.0f;
	}
	else
	{
		m_flOverturnedTime += gpGlobals->frametime;
	}

	StudioFrameAdvance();

	// If the enter or exit animation has finished, tell the server vehicle
	if ( IsSequenceFinished() && (m_bExitAnimOn || m_bEnterAnimOn) )
	{
		if ( m_bEnterAnimOn )
		{
			m_VehiclePhysics.ReleaseHandbrake();
			StartEngine();

			// HACKHACK: This forces the jeep to play a sound when it gets entered underwater
			if ( m_VehiclePhysics.IsEngineDisabled() )
			{
				CBaseServerVehicle *pServerVehicle = dynamic_cast<CBaseServerVehicle *>(GetServerVehicle());
				if ( pServerVehicle )
				{
					pServerVehicle->SoundStartDisabled();
				}
			}

#ifndef HOE_DLL
			// The first few time we get into the jeep, print the jeep help
			if ( m_iNumberOfEntries < hud_jeephint_numentries.GetInt() )
			{
				g_EventQueue.AddEvent( this, "ShowHudHint", 1.5f, this, this );
			}
#endif
		}
		
		if ( hl2_episodic.GetBool() )
		{
			// Set its running animation idle
			if ( m_bEnterAnimOn )
			{
				// Idle running
				int nSequence = SelectWeightedSequence( ACT_IDLE_STIMULATED );
				if ( nSequence > ACTIVITY_NOT_AVAILABLE )
				{
					SetCycle( 0 );
					m_flAnimTime = gpGlobals->curtime;
					ResetSequence( nSequence );
					ResetClientsideFrame();					
				}
			}
		}

		// Reset on exit anim
		GetServerVehicle()->HandleEntryExitFinish( m_bExitAnimOn, true );
	}

	if ( m_bLeanOutAnimOn || m_bLeanInAnimOn )
	{
#define LEAN_DURATION 0.6f
		float flFrac = ( gpGlobals->curtime - m_flLeanStartTime ) / LEAN_DURATION;
		flFrac = clamp( flFrac, 0.0f, 1.0f );

		if ( m_bLeanOutAnimOn )
		{
			if ( flFrac == 1.0 )
			{
				m_bPlayerLeaning = true;
				m_bLeanOutAnimOn = false;
			}
			SetPoseParameter( "player_lean", flFrac );
		}
		if ( m_bLeanInAnimOn )
		{
			if ( flFrac == 1.0 )
			{
				m_bPlayerLeaning = false;
				m_bLeanInAnimOn = false;
			}
			SetPoseParameter( "player_lean", 1.0 - flFrac );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropTruck::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	BaseClass::Use( pActivator, pCaller, useType, value );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CPropTruck::CanExitVehicle( CBaseEntity *pEntity )
{
	return ( !m_bEnterAnimOn && !m_bExitAnimOn && !m_bLocked && (m_nSpeed <= g_truckexitspeed.GetFloat() ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropTruck::DampenEyePosition( Vector &vecVehicleEyePos, QAngle &vecVehicleEyeAngles )
{
	// Get the frametime. (Check to see if enough time has passed to warrent dampening).
	float flFrameTime = gpGlobals->frametime;
	if ( flFrameTime < TRUCK_FRAMETIME_MIN )
	{
		vecVehicleEyePos = m_vecLastEyePos;
		DampenUpMotion( vecVehicleEyePos, vecVehicleEyeAngles, 0.0f );
		return;
	}

	// Keep static the sideways motion.

	// Dampen forward/backward motion.
	DampenForwardMotion( vecVehicleEyePos, vecVehicleEyeAngles, flFrameTime );

	// Blend up/down motion.
	DampenUpMotion( vecVehicleEyePos, vecVehicleEyeAngles, flFrameTime );
}

//-----------------------------------------------------------------------------
// Use the controller as follows:
// speed += ( pCoefficientsOut[0] * ( targetPos - currentPos ) + pCoefficientsOut[1] * ( targetSpeed - currentSpeed ) ) * flDeltaTime;
//-----------------------------------------------------------------------------
void CPropTruck::ComputePDControllerCoefficients( float *pCoefficientsOut,
												  float flFrequency, float flDampening,
												  float flDeltaTime )
{
	float flKs = 9.0f * flFrequency * flFrequency;
	float flKd = 4.5f * flFrequency * flDampening;

	float flScale = 1.0f / ( 1.0f + flKd * flDeltaTime + flKs * flDeltaTime * flDeltaTime );

	pCoefficientsOut[0] = flKs * flScale;
	pCoefficientsOut[1] = ( flKd + flKs * flDeltaTime ) * flScale;
}
 
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPropTruck::DampenForwardMotion( Vector &vecVehicleEyePos, QAngle &vecVehicleEyeAngles, float flFrameTime )
{
	// Get forward vector.
	Vector vecForward;
	AngleVectors( vecVehicleEyeAngles, &vecForward);

	// Simulate the eye position forward based on the data from last frame
	// (assumes no acceleration - it will get that from the "spring").
	Vector vecCurrentEyePos = m_vecLastEyePos + m_vecEyeSpeed * flFrameTime;

	// Calculate target speed based on the current vehicle eye position and the last vehicle eye position and frametime.
	Vector vecVehicleEyeSpeed = ( vecVehicleEyePos - m_vecLastEyeTarget ) / flFrameTime;
	m_vecLastEyeTarget = vecVehicleEyePos;	

	// Calculate the speed and position deltas.
	Vector vecDeltaSpeed = vecVehicleEyeSpeed - m_vecEyeSpeed;
	Vector vecDeltaPos = vecVehicleEyePos - vecCurrentEyePos;

	// Clamp.
	if ( vecDeltaPos.Length() > TRUCK_DELTA_LENGTH_MAX )
	{
		float flSign = vecForward.Dot( vecVehicleEyeSpeed ) >= 0.0f ? -1.0f : 1.0f;
		vecVehicleEyePos += flSign * ( vecForward * TRUCK_DELTA_LENGTH_MAX );
		m_vecLastEyePos = vecVehicleEyePos;
		m_vecEyeSpeed = vecVehicleEyeSpeed;
		return;
	}

	// Generate an updated (dampening) speed for use in next frames position extrapolation.
	float flCoefficients[2];
	ComputePDControllerCoefficients( flCoefficients, r_TruckViewDampenFreq.GetFloat(), r_TruckViewDampenDamp.GetFloat(), flFrameTime );
	m_vecEyeSpeed += ( ( flCoefficients[0] * vecDeltaPos + flCoefficients[1] * vecDeltaSpeed ) * flFrameTime );

	// Save off data for next frame.
	m_vecLastEyePos = vecCurrentEyePos;

	// Move eye forward/backward.
	Vector vecForwardOffset = vecForward * ( vecForward.Dot( vecDeltaPos ) );
	vecVehicleEyePos -= vecForwardOffset;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPropTruck::DampenUpMotion( Vector &vecVehicleEyePos, QAngle &vecVehicleEyeAngles, float flFrameTime )
{
	// Get up vector.
	Vector vecUp;
	AngleVectors( vecVehicleEyeAngles, NULL, NULL, &vecUp );
	vecUp.z = clamp( vecUp.z, 0.0f, vecUp.z );
	vecVehicleEyePos.z += r_TruckViewZHeight.GetFloat() * vecUp.z;

	// NOTE: Should probably use some damped equation here.
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropTruck::SetupMove( CBasePlayer *player, CUserCmd *ucmd, IMoveHelper *pHelper, CMoveData *move )
{
	if ( !IsPlayerDriving() )
		return;

	// If we are overturned and hit any key - leave the vehicle (IN_USE is already handled!).
	if ( m_flOverturnedTime > OVERTURNED_EXIT_WAITTIME )
	{
		if ( (ucmd->buttons & (IN_FORWARD|IN_BACK|IN_MOVELEFT|IN_MOVERIGHT|IN_SPEED|IN_JUMP|IN_ATTACK|IN_ATTACK2) ) && !m_bExitAnimOn )
		{
			// Can't exit yet? We're probably still moving. Swallow the keys.
			if ( !CanExitVehicle(player) )
				return;

			if ( !GetServerVehicle()->HandlePassengerExit( m_hPlayer ) && ( m_hPlayer != NULL ) )
			{
				m_hPlayer->PlayUseDenySound();
			}
			return;
		}
	}

	// If the throttle is disabled or we're upside-down, don't allow throttling (including turbo)
	CUserCmd tmp;
	if ( ( m_throttleDisableTime > gpGlobals->curtime ) || ( IsOverturned() ) )
	{
		m_bUnableToFire = true;
		
		tmp = (*ucmd);
		tmp.buttons &= ~(IN_FORWARD|IN_BACK|IN_SPEED);
		ucmd = &tmp;
	}
	
	BaseClass::SetupMove( player, ucmd, pHelper, move );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropTruck::DriveVehicle( float flFrameTime, CUserCmd *ucmd, int iButtonsDown, int iButtonsReleased )
{
	//Adrian: No headlights on Superfly.
/*	if ( ucmd->impulse == 100 )
	{
		if (HeadlightIsOn())
		{
			HeadlightTurnOff();
		}
        else 
		{
			HeadlightTurnOn();
		}
	}*/

	BaseClass::DriveVehicle( flFrameTime, ucmd, iButtonsDown, iButtonsReleased );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPlayer - 
//			*pMoveData - 
//-----------------------------------------------------------------------------
void CPropTruck::ProcessMovement( CBasePlayer *pPlayer, CMoveData *pMoveData )
{
	BaseClass::ProcessMovement( pPlayer, pMoveData );

	// Create dangers sounds in front of the vehicle.
	CreateDangerSounds();
}

//-----------------------------------------------------------------------------
void CPropTruck::ItemPostFrame( CBasePlayer *pPlayer )
{
	BaseClass::ItemPostFrame( pPlayer );

	if ( pPlayer->m_afButtonPressed & IN_JUMP )
	{
		if ( !m_bLeanOutAnimOn && !m_bLeanInAnimOn )
		{
			m_bLeanOutAnimOn = !m_bPlayerLeaning;
			m_bLeanInAnimOn = m_bPlayerLeaning;
			m_flLeanStartTime = gpGlobals->curtime;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Create danger sounds in front of the vehicle.
//-----------------------------------------------------------------------------
void CPropTruck::CreateDangerSounds( void )
{
//	QAngle dummy;
//	if ( GetAttachment( "Muzzle", m_vecGunOrigin, dummy ) == false )
//		return;

	if ( m_flDangerSoundTime > gpGlobals->curtime )
		return;

	QAngle vehicleAngles = GetLocalAngles();
	Vector vecStart = GetAbsOrigin();
	Vector vecDir, vecRight;

	GetVectors( &vecDir, &vecRight, NULL );

	const float soundDuration = 0.25;
	float speed = m_VehiclePhysics.GetHLSpeed();
	// Make danger sounds ahead of the jeep
	if ( fabs(speed) > 120 )
	{
		Vector	vecSpot;

		float steering = m_VehiclePhysics.GetSteering();
		if ( steering != 0 )
		{
			if ( speed > 0 )
			{
				vecDir += vecRight * steering * 0.5;
			}
			else
			{
				vecDir -= vecRight * steering * 0.5;
			}
			VectorNormalize(vecDir);
		}
		const float radius = speed * 0.4;
		// 0.3 seconds ahead of the jeep
		vecSpot = vecStart + vecDir * (speed * 1.1f);
		CSoundEnt::InsertSound( SOUND_DANGER | SOUND_CONTEXT_PLAYER_VEHICLE, vecSpot, radius, soundDuration, this, 0 );
		CSoundEnt::InsertSound( SOUND_PHYSICS_DANGER | SOUND_CONTEXT_PLAYER_VEHICLE, vecSpot, radius, soundDuration, this, 1 );
		//NDebugOverlay::Box(vecSpot, Vector(-radius,-radius,-radius),Vector(radius,radius,radius), 255, 0, 255, 0, soundDuration);

#if 0
		trace_t	tr;
		// put sounds a bit to left and right but slightly closer to Truck to make a "cone" of sound 
		// in front of it
		vecSpot = vecStart + vecDir * (speed * 0.5f) - vecRight * speed * 0.5;
		UTIL_TraceLine( vecStart, vecSpot, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );
		CSoundEnt::InsertSound( SOUND_DANGER, vecSpot, 400, soundDuration, this, 1 );

		vecSpot = vecStart + vecDir * (speed * 0.5f) + vecRight * speed * 0.5;
		UTIL_TraceLine( vecStart, vecSpot, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );
		CSoundEnt::InsertSound( SOUND_DANGER, vecSpot, 400, soundDuration, this, 2);
#endif
	}

	// Make engine sounds even when we're not going fast.
	CSoundEnt::InsertSound( SOUND_PLAYER | SOUND_CONTEXT_PLAYER_VEHICLE, GetAbsOrigin(), 800, soundDuration, this, 0 );

	m_flDangerSoundTime = gpGlobals->curtime + 0.1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropTruck::EnterVehicle( CBaseCombatCharacter *pPassenger )
{
	CBasePlayer *pPlayer = ToBasePlayer( pPassenger );
	if ( !pPlayer )
		return;

	CheckWater();
	BaseClass::EnterVehicle( pPassenger );

	// Start looking for seagulls to land
	m_hLastPlayerInVehicle = m_hPlayer;
	SetContextThink( NULL, 0, g_pTruckThinkContext );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropTruck::ExitVehicle( int nRole )
{
	SetPoseParameter( "player_lean", 0.0 );
	m_bPlayerLeaning = false;

	HeadlightTurnOff();

	BaseClass::ExitVehicle( nRole );

	// Remember when we last saw the player
	m_flPlayerExitedTime = gpGlobals->curtime;
	m_flLastSawPlayerAt = gpGlobals->curtime;

	if ( GlobalEntity_GetState( "no_seagulls_on_jeep" ) == GLOBAL_OFF )
	{
		// Look for fly nodes
		CHintCriteria hintCriteria;
		hintCriteria.SetHintType( HINT_CROW_FLYTO_POINT );
		hintCriteria.AddIncludePosition( GetAbsOrigin(), 4500 );
		CAI_Hint *pHint = CAI_HintManager::FindHint( GetAbsOrigin(), hintCriteria );
		if ( pHint )
		{
			// Start looking for seagulls to perch on me
			SetContextThink( &CPropTruck::TruckSeagullThink, gpGlobals->curtime + TRUCK_SEAGULL_THINK_INTERVAL, g_pTruckThinkContext );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: See if we should spawn a seagull on the jeep
//-----------------------------------------------------------------------------
void CPropTruck::TruckSeagullThink( void )
{
	if ( !m_hLastPlayerInVehicle )
		return;

	CBaseEntity *pBlocker;

	// Do we already have a seagull?
	if ( m_hSeagull )
	{
		CNPC_Seagull *pSeagull = dynamic_cast<CNPC_Seagull *>( m_hSeagull.Get() );

		if ( pSeagull )
		{
			// Is he still on us?
			if ( pSeagull->m_bOnJeep == true )
			{
				// Make the existing seagull spawn more poop over time
				if ( pSeagull->IsAlive() )
				{
					AddSeagullPoop( pSeagull->GetAbsOrigin() );
				}

				SetContextThink( &CPropTruck::TruckSeagullThink, gpGlobals->curtime + TRUCK_SEAGULL_POOP_INTERVAL, g_pTruckThinkContext );
			}
			else
			{
				// Our seagull's moved off us. 
				m_hSeagull = NULL;
				SetContextThink( &CPropTruck::TruckSeagullThink, gpGlobals->curtime + TRUCK_SEAGULL_THINK_INTERVAL, g_pTruckThinkContext );
			}
		}
		
		return;
	}

	// Only spawn seagulls if we're upright and out of water
	if ( m_WaterData.m_bBodyInWater || IsOverturned() )
	{
		SetContextThink( &CPropTruck::TruckSeagullThink, gpGlobals->curtime + TRUCK_SEAGULL_THINK_INTERVAL, g_pTruckThinkContext );
		return;
	}

	// Is the player visible?
	if ( FVisible( m_hLastPlayerInVehicle, MASK_SOLID_BRUSHONLY, &pBlocker ) )
	{
		m_flLastSawPlayerAt = gpGlobals->curtime;
		SetContextThink( &CPropTruck::TruckSeagullThink, gpGlobals->curtime + TRUCK_SEAGULL_THINK_INTERVAL, g_pTruckThinkContext );
		return;
	}

	// Start checking quickly
	SetContextThink( &CPropTruck::TruckSeagullThink, gpGlobals->curtime + 0.2, g_pTruckThinkContext );

	// Not taken enough time yet?
	float flHiddenTime = (gpGlobals->curtime - m_flLastSawPlayerAt);
	if ( flHiddenTime < TRUCK_SEAGULL_HIDDEN_TIME )
		return;

	// Random chance based upon the time it's taken
	float flChance = clamp( flHiddenTime / TRUCK_SEAGULL_MAX_TIME, 0.0, 1.0 );
	if ( RandomFloat(0,1) < flChance )
	{
		SpawnPerchedSeagull();

		// Don't think for a while
		SetContextThink( &CPropTruck::TruckSeagullThink, gpGlobals->curtime + TRUCK_SEAGULL_POOP_INTERVAL, g_pTruckThinkContext );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropTruck::SpawnPerchedSeagull( void )
{
	// Find a point on the car to sit
	Vector vecOrigin;
	QAngle vecAngles;
	int iAttachment = Studio_FindRandomAttachment( GetModelPtr(), "seagull_perch" );
	if ( iAttachment == -1 )
		return;

	// Spawn the seagull
	GetAttachment( iAttachment+1, vecOrigin, vecAngles );
	//vecOrigin.z += 16;

	CNPC_Seagull *pSeagull = (CNPC_Seagull*)CBaseEntity::Create("npc_seagull", vecOrigin, vecAngles, NULL );

	if ( !pSeagull )
		return;
	
	pSeagull->AddSpawnFlags( SF_NPC_FADE_CORPSE );
	pSeagull->SetGroundEntity( this );
	pSeagull->AddFlag( FL_ONGROUND );
	pSeagull->SetOwnerEntity( this );
	pSeagull->SetMoveType( MOVETYPE_FLY );
	pSeagull->m_bOnJeep = true;
	
	m_hSeagull = pSeagull;

	AddSeagullPoop( vecOrigin );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &vecOrigin - 
//-----------------------------------------------------------------------------
void CPropTruck::AddSeagullPoop( const Vector &vecOrigin )
{
	// Drop some poop decals!
	int iDecals = RandomInt( 1,2 );
	for ( int i = 0; i < iDecals; i++ )
	{
		Vector vecPoop = vecOrigin;

		// get circular gaussian spread
		float x, y, z;
		do 
		{
			x = random->RandomFloat(-0.5,0.5) + random->RandomFloat(-0.5,0.5);
			y = random->RandomFloat(-0.5,0.5) + random->RandomFloat(-0.5,0.5);
			z = x*x+y*y;
		} while (z > 1);
		vecPoop += Vector( x * 90, y * 90, 128 );
		
		trace_t tr;
		UTIL_TraceLine( vecPoop, vecPoop - Vector(0,0,512), MASK_SHOT, m_hSeagull, COLLISION_GROUP_NONE, &tr );
		UTIL_DecalTrace( &tr, "BirdPoop" );
	}

	m_bHasPoop = true;
}

//-----------------------------------------------------------------------------
// Purpose: Show people how to drive!
//-----------------------------------------------------------------------------
void CPropTruck::InputShowHudHint( inputdata_t &inputdata )
{
#ifndef HOE_DLL
	CBaseServerVehicle *pServerVehicle = dynamic_cast<CBaseServerVehicle *>(GetServerVehicle());
	if ( pServerVehicle )
	{
		if( pServerVehicle->GetPassenger( VEHICLE_ROLE_DRIVER ) )
		{
			UTIL_HudHintText( m_hPlayer, "#Valve_Hint_JeepKeys" );
			m_iNumberOfEntries++;
		}
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Force the player to enter the vehicle.
//-----------------------------------------------------------------------------
void CPropTruck::InputEnterVehicle( inputdata_t &inputdata )
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
void CPropTruck::InputEnterVehicleImmediate( inputdata_t &inputdata )
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
void CPropTruck::InputExitVehicle( inputdata_t &inputdata )
{
	m_bForcedExit = true;
}

//-----------------------------------------------------------------------------
void CPropTruck::InputExitVehicleImmediate( inputdata_t &inputdata )
{
	CBasePlayer *pPlayer = m_hPlayer;
	if ( !pPlayer )
		return;

	if ( !CanExitVehicle( pPlayer ) )
		return;

	pPlayer->LeaveVehicle( pPlayer->GetAbsOrigin(), pPlayer->GetAbsAngles() );
	ExitVehicle( VEHICLE_ROLE_DRIVER );
}

//========================================================================================================================================
// TRUCK FOUR WHEEL PHYSICS VEHICLE SERVER VEHICLE
//========================================================================================================================================

//-----------------------------------------------------------------------------
CPropTruck *CTruckFourWheelServerVehicle::GetVehicle( void )
{
	return (CPropTruck *)GetDrivableVehicle();
}

//-----------------------------------------------------------------------------
void CTruckFourWheelServerVehicle::ItemPostFrame( CBasePlayer *player )
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
bool CTruckFourWheelServerVehicle::IsPassengerUsingStandardWeapons( int nRole )
{
	return !((CPropTruck*)m_pVehicle)->IsPlayerDriving();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTruckFourWheelServerVehicle::GetVehicleViewPosition( int nRole, Vector *pAbsOrigin, QAngle *pAbsAngles, float *pFOV /*= NULL*/ )
{
////
return BaseClass::GetVehicleViewPosition( nRole, pAbsOrigin, pAbsAngles, pFOV );
////
	if ( ((CPropTruck*)m_pVehicle)->IsPlayerDriving() )
	{
		return BaseClass::GetVehicleViewPosition( nRole, pAbsOrigin, pAbsAngles, pFOV );
	}

	// HOE: This is from vehicle_choreo_generic.cpp
	// THIS DOESN'T RETURN THE FOV

	// FIXME: This needs to be reconciled with the other versions of this function!
	Assert( nRole == VEHICLE_ROLE_DRIVER );
	CBasePlayer *pPlayer = ToBasePlayer( GetDrivableVehicle()->GetDriver() );
	Assert( pPlayer );
#if 0
	// Use the player's eyes instead of the attachment point
	if ( GetVehicle()->m_bForcePlayerEyePoint )
	{
		// Call to BaseClass because CBasePlayer::EyePosition calls this function.
		*pAbsOrigin = pPlayer->BaseClass::EyePosition();
		*pAbsAngles = pPlayer->BaseClass::EyeAngles();
		return;
	}
#endif
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

//-----------------------------------------------------------------------------
float CTruckFourWheelServerVehicle::GetVehicleFOV( void )
{
	return 75.0f; // FIXME: or 90 when driving
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &vecEyeExitEndpoint - 
// Output : int
//-----------------------------------------------------------------------------
int CTruckFourWheelServerVehicle::GetExitAnimToUse( Vector &vecEyeExitEndpoint, bool &bAllPointsBlocked )
{
	return BaseClass::GetExitAnimToUse( vecEyeExitEndpoint, bAllPointsBlocked );
}
