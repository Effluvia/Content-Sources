#include "cbase.h"
#include "ai_baseactor.h"
#include "ai_behavior_lead.h"
#include "ai_senses.h"
#include "ai_tacticalservices.h"
#define CHUMTOAD_ENGINE
#ifdef CHUMTOAD_ENGINE
#include "soundenvelope.h"
#include "ai_route.h"
#include "ai_waypoint.h"
#include "te_effect_dispatch.h"
#include "explode.h"
#include "ai_moveprobe.h"
#endif

#define CHUMTOAD_HEADLIGHT

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define CHUMTOAD_HEALTH 10 // 5, 10, 15
#define CHUMTOAD_EYE_FRAMES 3
#define CHUMTOAD_BLINK_INTERVAL 3.0
#define CHUMTOAD_COMFORT_RADIUS 128.0

#define CHUMTOAD_BULLSQUID_RADIUS 512.0

#ifdef CHUMTOAD_ENGINE
#define SF_CHUMTOAD_MOTORIZED (1 << 16)
#define CHUMTOAD_BODYGROUP_ENGINE 1

#define	ICH_WAYPOINT_DISTANCE	128.0f
#define	ICH_HEIGHT_PREFERENCE	16.0f
#define	ICH_DEPTH_PREFERENCE	8.0f

enum IchthyosaurMoveType_t
{
	ICH_MOVETYPE_SEEK = 0,	// Fly through the target without stopping.
	ICH_MOVETYPE_ARRIVE		// Slow down and stop at target.
};

#define ENGINE_MIN_VOLUME 0.15
#define ENGINE_MAX_VOLUME 0.35

#define ENGINE_MIN_PITCH 80
#define ENGINE_MAX_PITCH 150

class CChumtoadMotor : public CAI_Motor
{
	typedef CAI_Motor BaseClass;
public:
	CChumtoadMotor( CAI_BaseNPC *pOuter )
	 :	BaseClass( pOuter )
	{
	}

	virtual AIMotorMoveResult_t MoveGroundExecute( const AILocalMoveGoal_t &move, AIMoveTrace_t *pTraceResult );
};

class CChumtoadNavigator : public CAI_Navigator
{
	typedef CAI_Navigator BaseClass;
public:
	CChumtoadNavigator( CAI_BaseNPC *pOuter )
		: BaseClass( pOuter )
	{
	}
	bool MoveUpdateWaypoint( AIMoveResult_t *pResult );
};
#endif // CHUMTOAD_ENGINE

// I made the Chumtoad an ACTOR instead of NPC so it can lead the player during the namc4 maze
class CNPC_Chumtoad : public CAI_BaseActor, public CAI_LeadBehaviorHandler
{
public:
	DECLARE_CLASS( CNPC_Chumtoad, CAI_BaseActor );
	DEFINE_CUSTOM_AI;
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	void Spawn( void );
	void Precache( void );
	void Activate( void );

	Class_T Classify( void );
	bool ClassifyPlayerAlly( void ) { return false; }
	bool ClassifyPlayerAllyVital( void ) { return false; }

	Disposition_t IRelationType( CBaseEntity *pTarget );

	bool CreateBehaviors( void );

	void Touch( CBaseEntity *pOther );
	void DeathSound( const CTakeDamageInfo &info );
	void Event_Killed( const CTakeDamageInfo &info );

	void GatherConditions( void );
	void BuildScheduleTestBits( void );
	void PrescheduleThink( void );
	int SelectSchedule( void );
	void StartTask( const Task_t *pTask );
	void RunTask( const Task_t *pTask );

	bool IsValidMoveAwayDest( const Vector &vecDest );
	Activity NPC_TranslateActivity( Activity eNewActivity );

	float MaxYawSpeed( void )
	{
#ifdef CHUMTOAD_ENGINE
		if ( HasEngine() /*&& GetWaterLevel() > 0*/ )
		{
			return 15.0f;
		}
#endif
		return 60.0f;
	}
	virtual float GetIdealSpeed( void ) const
	{
#ifdef CHUMTOAD_ENGINE
		if ( HasEngine() && GetWaterLevel() > 0 )
		{
			return m_flEngineSpeed;
		}
#endif
		return BaseClass::GetIdealSpeed();
	}
	virtual float GetIdealAccel( ) const
	{
		// return ideal max velocity change over 1 second.
#ifdef CHUMTOAD_ENGINE
		if ( HasEngine() && GetWaterLevel() > 0 )
		{
			return m_flEngineSpeed;
		}
#endif
		return BaseClass::GetIdealAccel();
	}

	void SetSkin( int skin ) { m_nSkin = skin; } 
	int GetSkin( void ) { return m_nSkin; }

	// CAI_LeadBehaviorHandler
	virtual void OnEvent( int event );

#ifdef CHUMTOAD_ENGINE
	virtual CAI_Navigator *CreateNavigator() { return new CChumtoadNavigator( this );	}
	virtual CAI_Motor *CreateMotor( void ) { return new CChumtoadMotor( this ); }
	virtual void StopLoopingSounds( void );
	virtual float GetSequenceGroundSpeed( CStudioHdr *pStudioHdr, int iSequence )
	{
		if ( HasEngine() && GetWaterLevel() > 0 )
		{
			return m_flEngineSpeed;
		}
		return BaseClass::GetSequenceGroundSpeed( pStudioHdr, iSequence );
	}

	void MoveFlyExecute( CBaseEntity *pTargetEnt, const Vector & vecDir, float flDistance, float flInterval );
	bool OverrideMove( const Vector &MoveTarget, float flInterval );
	void DoMovement( float flInterval, const Vector &MoveTarget, int eMoveType );

	void ComputeAngularVelocity( float flInterval, float yaw );

	bool SteerAvoidObstacles( Vector &Steer, const Vector &Velocity, const Vector &Forward, const Vector &Right, const Vector &Up );
	void SteerArrive( Vector &Steer, const Vector &Target );
	void SteerSeek( Vector &Steer, const Vector &Target );
	void ClampSteer( Vector &SteerAbs, Vector &SteerRel, Vector &forward, Vector &right, Vector &up );
	void AddSwimNoise( Vector *velocity );

	void ApplySidewaysDrag( float flInterval, const Vector &vecRight );
	void ApplyGeneralDrag( float flInterval );

#define RAMP_ENGINE_THINK_CONTEXT "ChumtoadRampEngineThinkContext"
	void RampEngineThink( void );

	bool HasEngine( void ) const { return HasSpawnFlags( SF_CHUMTOAD_MOTORIZED ); };

	void InputEngineStart( inputdata_t &inputdata );
	void InputEngineStop( inputdata_t &inputdata );

	void InputEngineMalfunction( inputdata_t &inputdata );
	void EngineMalfThink( void );
	float m_flEngineMalfTime;
	float m_flEngineMalfPitch;

	bool m_bEngineStarted;
	float m_flEngineIdleTime;
	CSoundPatch *m_pEngineSound;
	float m_flEngineSpeed;
	float m_flEngineRatio; // How hard the engine is working, 0-1
	float m_flEngineRatioGoal; // How hard the engine should be working, 0-1
	int m_iEngineGestureLayer;

	CNetworkVar( Vector, m_vecPhysVelocity );
	CNetworkVar( int, m_nExactWaterLevel );
	IMPLEMENT_NETWORK_VAR_FOR_DERIVED( m_nWaterLevel );

//	Vector m_vecLastMoveTarget;
//	bool m_bHasMoveTarget;
//	bool m_bIgnoreSurface;
//	Vector m_vPrevWaypointPos;
	Vector m_vHoverPos;
	float m_NonEngineYaw; // The yaw other code set for us
	float m_EngineYaw; // The last engine-related yaw we set

	float m_flNextBackfireTime;

	int m_nHeadlightBlinks;
	float m_flHeadlightBlinkTime;

	static const Vector	m_vecAccelerationMax;
	static const Vector	m_vecAccelerationMin;

#endif // CHUMTOAD_ENGINE

#ifdef CHUMTOAD_HEADLIGHT
	bool HeadlightIsOn( void ) { return m_bHeadlightIsOn; }
	void HeadlightTurnOn( void );
	void HeadlightTurnOff( void );

	void InputHeadlightOn( inputdata_t &inputdata );
	void InputHeadlightOff( inputdata_t &inputdata );
	void InputHeadlightBlink( inputdata_t &inputdata );

	CNetworkVar( bool, m_bHeadlightIsOn );
#endif

	CAI_LeadBehavior m_LeadBehavior;
	float m_flNextBlinkTime;
	bool m_bPlayingDead;
	float m_flPlayDeadStartTime;

	//-----------------------------------------------------------------------------
	// Conditions.
	//-----------------------------------------------------------------------------
	enum 
	{
		COND_CHUMTOAD_UNCOMFORTABLE = BaseClass::NEXT_CONDITION,
		COND_CHUMTOAD_COMFORTABLE,
		COND_CHUMTOAD_PLAY_DEAD,
		NEXT_CONDITION
	};

	//-----------------------------------------------------------------------------
	// Custom schedules.
	//-----------------------------------------------------------------------------
	enum
	{
		SCHED_CHUMTOAD_PLAY_DEAD = BaseClass::NEXT_SCHEDULE,
		SCHED_CHUMTOAD_PLAY_DEAD_END,
		NEXT_SCHEDULE
	};

	//-----------------------------------------------------------------------------
	// Tasks
	//-----------------------------------------------------------------------------
	enum
	{
		TASK_CHUMTOAD_PLAY_DEAD = BaseClass::NEXT_TASK,
		NEXT_TASK
	};

	//-----------------------------------------------------------------------------
	// Activities
	//-----------------------------------------------------------------------------
	static Activity ACT_CHUMTOAD_DEAD_BEGIN;
	static Activity ACT_CHUMTOAD_DEAD_LOOP;
	static Activity ACT_CHUMTOAD_DEAD_END;
#ifdef CHUMTOAD_ENGINE
	static Activity ACT_CHUMTOAD_ENGINE;
	static Activity ACT_CHUMTOAD_ENGINE_SWIM;
#endif
};

LINK_ENTITY_TO_CLASS( npc_chumtoad, CNPC_Chumtoad );

BEGIN_DATADESC( CNPC_Chumtoad )
	DEFINE_FIELD( m_flNextBlinkTime, FIELD_TIME ),
	DEFINE_FIELD( m_bPlayingDead, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flPlayDeadStartTime, FIELD_TIME ),
#ifdef CHUMTOAD_ENGINE
	DEFINE_FIELD( m_vecPhysVelocity,	FIELD_VECTOR ),
	DEFINE_FIELD( m_nExactWaterLevel,	FIELD_INTEGER ),
	DEFINE_FIELD( m_flEngineMalfTime, FIELD_TIME ),
	DEFINE_FIELD( m_flEngineMalfPitch, FIELD_FLOAT ),
	DEFINE_FIELD( m_bEngineStarted, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flEngineIdleTime, FIELD_TIME ),
	DEFINE_FIELD( m_flEngineRatio, FIELD_FLOAT ),
	DEFINE_FIELD( m_flEngineRatioGoal, FIELD_FLOAT ),
	DEFINE_FIELD( m_vHoverPos, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( m_NonEngineYaw, FIELD_FLOAT ),
	DEFINE_FIELD( m_EngineYaw, FIELD_FLOAT ),
	DEFINE_FIELD( m_flNextBackfireTime, FIELD_TIME ),
	DEFINE_FIELD( m_nHeadlightBlinks, FIELD_INTEGER ),
	DEFINE_FIELD( m_flHeadlightBlinkTime, FIELD_TIME ),

	DEFINE_SOUNDPATCH( m_pEngineSound ),

	DEFINE_THINKFUNC( RampEngineThink ),
	DEFINE_THINKFUNC( EngineMalfThink ),

	DEFINE_INPUTFUNC( FIELD_VOID, "EngineStart", InputEngineStart ),
	DEFINE_INPUTFUNC( FIELD_VOID, "EngineStop", InputEngineStop ),
	DEFINE_INPUTFUNC( FIELD_VOID, "EngineMalfunction", InputEngineMalfunction ),
#endif
#ifdef CHUMTOAD_HEADLIGHT
	DEFINE_INPUTFUNC( FIELD_VOID, "HeadlightOn", InputHeadlightOn ),
	DEFINE_INPUTFUNC( FIELD_VOID, "HeadlightOff", InputHeadlightOff ),
	DEFINE_INPUTFUNC( FIELD_INTEGER, "HeadlightBlink", InputHeadlightBlink ),
	DEFINE_FIELD( m_bHeadlightIsOn, FIELD_BOOLEAN ),
#endif
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CNPC_Chumtoad, DT_NPC_Chumtoad )
#ifdef CHUMTOAD_ENGINE
	SendPropInt( SENDINFO( m_nExactWaterLevel ) ),
	SendPropInt( SENDINFO( m_nWaterLevel ) ),
	SendPropVector( SENDINFO( m_vecPhysVelocity ) ),
#endif
#ifdef CHUMTOAD_HEADLIGHT
	SendPropBool( SENDINFO( m_bHeadlightIsOn ) ),
#endif
END_SEND_TABLE();

Activity
#ifdef CHUMTOAD_ENGINE
	CNPC_Chumtoad::ACT_CHUMTOAD_ENGINE,
	CNPC_Chumtoad::ACT_CHUMTOAD_ENGINE_SWIM,
#endif
	CNPC_Chumtoad::ACT_CHUMTOAD_DEAD_BEGIN,
	CNPC_Chumtoad::ACT_CHUMTOAD_DEAD_LOOP,
	CNPC_Chumtoad::ACT_CHUMTOAD_DEAD_END;

#ifdef CHUMTOAD_ENGINE
//Acceleration definitions
const Vector CNPC_Chumtoad::m_vecAccelerationMax	= Vector(  256,  1024,  512 );
const Vector CNPC_Chumtoad::m_vecAccelerationMin	= Vector( -256, -1024, -512 );
#endif

//-----------------------------------------------------------------------------
void CNPC_Chumtoad::Spawn( void )
{
	BaseClass::Spawn();

	Precache();
	SetModel( STRING( GetModelName() ) );

	SetHullType(HULL_TINY);
	SetHullSizeNormal();

	SetSolid( SOLID_BBOX );
	AddSolidFlags( FSOLID_NOT_STANDABLE ); // you can't stand on this
	SetMoveType( MOVETYPE_STEP );

	CapabilitiesClear();
	CapabilitiesAdd( bits_CAP_MOVE_GROUND );

#ifdef CHUMTOAD_ENGINE
	if ( HasEngine() )
	{
//		AddFlag( FL_FLY );

		SetBodygroup( CHUMTOAD_BODYGROUP_ENGINE, 1 );

		m_vHoverPos = vec3_origin; // FIXME: teleportation
		m_NonEngineYaw = m_EngineYaw = GetMotor()->GetIdealYaw();
	}
#endif

	SetBloodColor( BLOOD_COLOR_GREEN );

	/* Weird.  NPCs generally start out with NPC_STATE_NONE and NPCInit() sets the
	  ideal state to idle.  But this causes GatherConditions() not to be called before
	  the first SelectSchedule() call.  This causes namc4 chumtoad to not choose
	  SCHED_FALL_TO_GROUND. */
	m_NPCState = NPC_STATE_IDLE;

	m_flFieldOfView = -1;
	SetViewOffset( Vector ( 0, 0, 20 ) );		// Position of the eyes relative to NPC's origin.

	m_iHealth = CHUMTOAD_HEALTH;
	m_flNextBlinkTime = gpGlobals->curtime + random->RandomFloat(0, CHUMTOAD_BLINK_INTERVAL);

	NPCInit();
}

//-----------------------------------------------------------------------------
void CNPC_Chumtoad::Precache( void )
{
	SetModelName( AllocPooledString( "models/chumtoad.mdl" ) );
	PrecacheModel( STRING( GetModelName() ) );

	PrecacheScriptSound( "Chumtoad.Squeak" ); // squeek/sqk_die1.wav
#ifdef CHUMTOAD_ENGINE
	PrecacheScriptSound( "Chumtoad.EngineStart" );
	PrecacheScriptSound( "Chumtoad.EngineStop" );
	PrecacheScriptSound( "Chumtoad.EngineIdle" );
	PrecacheScriptSound( "Chumtoad.EngineBackfire" );
	PrecacheScriptSound( "Chumtoad.EngineMalfunction" );
	PrecacheScriptSound( "Chumtoad.EngineExplode" );
#endif
#ifdef CHUMTOAD_HEADLIGHT
	PrecacheScriptSound( "Chumtoad.HeadlightOn" );
	PrecacheScriptSound( "Chumtoad.HeadlightOff" );
#endif
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
void CNPC_Chumtoad::Activate( void )
{
	BaseClass::Activate();

#ifdef CHUMTOAD_ENGINE
	if ( HasEngine() && !m_pEngineSound )
	{
		CPASAttenuationFilter filter( this );
		CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
		m_pEngineSound = controller.SoundCreate( filter, entindex(), "Chumtoad.EngineIdle" );
		controller.Play( m_pEngineSound, 0.0, 100 ); // start silent
	}

	if ( HasEngine() )
	{
		m_iEngineGestureLayer = AddGesture( ACT_CHUMTOAD_ENGINE, false );
		if ( m_iEngineGestureLayer != -1 )
			SetLayerPlaybackRate( m_iEngineGestureLayer, 0.0 );
		m_flEngineSpeed = 200.0f;

#ifdef CHUMTOAD_HEADLIGHT
//		if ( !HeadlightIsOn() )
//			HeadlightTurnOn();
#endif // CHUMTOAD_HEADLIGHT
	}
#endif // CHUMTOAD_ENGINE
}

//-----------------------------------------------------------------------------
Class_T	CNPC_Chumtoad::Classify( void )
{
	// Bullsquid ignores us when playing dead
	if ( m_bPlayingDead && m_flPlayDeadStartTime < gpGlobals->curtime - 2.0f )
		return CLASS_NONE;
	return CLASS_CHUMTOAD;
}

//-----------------------------------------------------------------------------
Disposition_t CNPC_Chumtoad::IRelationType( CBaseEntity *pTarget )
{
	// Don't run away from player/NPCs in namc4 chumtoad maze while leading
	if ( m_LeadBehavior.HasGoal() || IsInAScript() )
		return D_NU;

	if ( pTarget->Classify() == CLASS_BULLSQUID &&
		(pTarget->GetAbsOrigin() - GetAbsOrigin()).LengthSqr() <= Square( CHUMTOAD_BULLSQUID_RADIUS ) )
		return D_FR;
	if ( !FClassnameIs( pTarget, "npc_chumtoad" ) &&
		( (pTarget->GetAbsOrigin() - GetAbsOrigin()).LengthSqr() < Square( CHUMTOAD_COMFORT_RADIUS ) ) )
		return D_FR;
	return D_NU;
}

//-----------------------------------------------------------------------------
bool CNPC_Chumtoad::CreateBehaviors( void )
{
	AddBehavior( &m_LeadBehavior );	
	return BaseClass::CreateBehaviors();
}

//-----------------------------------------------------------------------------
void CNPC_Chumtoad::Touch( CBaseEntity *pOther )
{
	if ( pOther->GetAbsVelocity() == vec3_origin || GetHealth() <= 0 )
		return;

#ifdef CHUMTOAD_ENGINE
	if ( HasEngine() )
		return;
#endif

	if ( !m_bPlayingDead )
		SetCondition( COND_CHUMTOAD_PLAY_DEAD );

#if 0
	// FIXME: blood

	CTakeDamageInfo info( pOther, pOther, GetHealth(), DMG_CRUSH );
//	CalculateMeleeDamageForce( &info, forward, GetAbsOrigin() );
	TakeDamage( info );
#endif
}

//-----------------------------------------------------------------------------
void CNPC_Chumtoad::DeathSound( const CTakeDamageInfo &info )
{
	EmitSound( "Chumtoad.Squeak" );
}

//------------------------------------------------------------------------------
void CNPC_Chumtoad::Event_Killed( const CTakeDamageInfo &info )
{
	// Almost shut the eye
	SetSkin( CHUMTOAD_EYE_FRAMES - 2 );

#ifdef CHUMTOAD_HEADLIGHT
	HeadlightTurnOff(); // plays sound... should it?
#endif

	BaseClass::Event_Killed( info );
}

//-----------------------------------------------------------------------------
void CNPC_Chumtoad::BuildScheduleTestBits( void )
{
	BaseClass::BuildScheduleTestBits();

	// Interrupt when stepped on unless playing dead
	if ( !m_bPlayingDead )
		SetCustomInterruptCondition( COND_CHUMTOAD_PLAY_DEAD );
}

#define TimeExpiredWithin(t,d) ((t) <= gpGlobals->curtime - (d))

//-----------------------------------------------------------------------------
void CNPC_Chumtoad::PrescheduleThink( void )
{
	BaseClass::PrescheduleThink();

	// at random, initiate a blink if not already blinking or sleeping
//	if ( !m_fDontBlink )
	if ( !m_bPlayingDead && IsAlive() )
	{
		if ( ( GetSkin() == 0 ) && ( m_flNextBlinkTime <= gpGlobals->curtime ) )
		{
			// start blinking!
			SetSkin( CHUMTOAD_EYE_FRAMES - 1 );

			m_flNextBlinkTime = gpGlobals->curtime + CHUMTOAD_BLINK_INTERVAL +
				random->RandomFloat(0, CHUMTOAD_BLINK_INTERVAL);
		}
		else if ( GetSkin() != 0 )
		{
			// already blinking
			SetSkin( GetSkin() - 1 );
		}
	}

#ifdef CHUMTOAD_ENGINE
	if ( HasEngine() )
	{
//		float flSpeed = GetAbsVelocity().Length();

		// Update pitch/volume based on speed
//		float flSpeedFrac = flSpeed / m_flEngineSpeed;
//		flSpeedFrac = clamp( flSpeedFrac, 0.0f, 1.0f );
//		float flSpeedFrac = max( m_flEngineRatio, 0.1 );
//		float flSpeedFrac = m_flEngineRatio;


#if 0
		// Ripple data.
		flSpeedFrac = flSpeed / m_flEngineSpeed;
		flSpeedFrac = clamp( flSpeedFrac, 0.0f, 1.0f );
		if ( flSpeedFrac > 0.2 )
		{
			CEffectData	data;
			data.m_fFlags = 0;
			data.m_vOrigin = GetAbsOrigin();
			data.m_vNormal = GetAbsVelocity();
			VectorNormalize( data.m_vNormal );
			data.m_vNormal *= -1;
			data.m_flRadius = 16 + flSpeedFrac * 48;
			data.m_flMagnitude = flSpeed;
			if ( GetWaterType() & CONTENTS_SLIME )
			{
				data.m_fFlags |= FX_WATER_IN_SLIME;
			}

			// Create the ripple.
			DispatchEffect( "chumtoadwake", data );
		}
#endif
		if ( m_nHeadlightBlinks > 0 && TimeExpiredWithin(m_flHeadlightBlinkTime,0.05) )
		{
			if ( HeadlightIsOn() )
			{
				HeadlightTurnOff();
				m_flHeadlightBlinkTime = gpGlobals->curtime + 0.2;
			}
			else
			{
				HeadlightTurnOn();
				m_nHeadlightBlinks--;
				m_flHeadlightBlinkTime = gpGlobals->curtime + random->RandomFloat( 0.2, 0.4 );
			}
		}

		if ( m_bEngineStarted && TimeExpiredWithin(m_flNextBackfireTime,0.2) )
		{
			EmitSound( "Chumtoad.EngineBackfire" );

			// ----------
			// Muzzleflash
			// ----------
			CEffectData data;
			data.m_nEntIndex = entindex();
			data.m_nAttachmentIndex = LookupAttachment("exhaust");
			data.m_flScale = 1.0f;
			data.m_fFlags = MUZZLEFLASH_SHOTGUN;
			DispatchEffect( "MuzzleFlash", data );

			m_flNextBackfireTime = gpGlobals->curtime + random->RandomInt( 4, 8 );
		}

		if ( GetWaterLevel() > 0 && m_vHoverPos == vec3_origin )
			m_vHoverPos = GetAbsOrigin();

		if ( !IsMoving() && GetWaterLevel() > 0 && m_vHoverPos != vec3_origin )
			OverrideMove( m_vHoverPos, 0.1 );

		Activity act = NPC_TranslateActivity( ACT_RUN );
		if ( act != GetIdealActivity() )
			SetIdealActivity( act );
	}

#endif // CHUMTOAD_ENGINE
}

//-----------------------------------------------------------------------------
void CNPC_Chumtoad::GatherConditions()
{
	BaseClass::GatherConditions();

#ifdef CHUMTOAD_ENGINE
	if ( HasEngine() )
	{
		if ( GetWaterLevel() > 0 )
		{
			if ( (GetFlags() & FL_FLY) == 0 )
				AddFlag( FL_FLY );
			ClearCondition( COND_FLOATING_OFF_GROUND );
		}
		else
		{
			if ( (GetFlags() & FL_FLY) != 0 )
				RemoveFlag( FL_FLY );
		}
	}
#endif

	if ( HasCondition( COND_CHUMTOAD_PLAY_DEAD ) && !m_bPlayingDead )
		return;

	// Play dead for min period of time
	if ( m_bPlayingDead && m_flPlayDeadStartTime >= gpGlobals->curtime - 3.0f )
		return;

	ClearCondition( COND_CHUMTOAD_UNCOMFORTABLE );
	SetCondition( COND_CHUMTOAD_COMFORTABLE );

	// Don't run away from player/NPCs in namc4 chumtoad maze while leading.
	if ( m_LeadBehavior.HasGoal() || IsInAScript() )
		return;

	// See if any NPCs are too close to chummy
	AISightIter_t iter;
	CBaseEntity *pSightEnt;
	pSightEnt = GetSenses()->GetFirstSeenEntity( &iter );
	while( pSightEnt )
	{
		if ( pSightEnt->m_iClassname != m_iClassname && pSightEnt->IsAlive() && pSightEnt->MyNPCPointer() != NULL )
		{
			float flDist = (pSightEnt->GetAbsOrigin() - GetAbsOrigin()).LengthSqr();

			if ( pSightEnt->Classify() == CLASS_BULLSQUID )
			{
				bool bAfraidOfSquid = m_bPlayingDead ||
					pSightEnt->MyNPCPointer()->GetEnemy() == this ||
					flDist <= Square( CHUMTOAD_BULLSQUID_RADIUS );

				if ( bAfraidOfSquid )
				{
#ifdef CHUMTOAD_ENGINE
					if ( !m_bPlayingDead && !HasEngine() )
#else
					if ( !m_bPlayingDead )
#endif
						SetCondition( COND_CHUMTOAD_PLAY_DEAD );
					else
					{
						SetCondition( COND_CHUMTOAD_UNCOMFORTABLE );
						ClearCondition( COND_CHUMTOAD_COMFORTABLE );
					}
					break;
				}
			}

			if ( flDist <= Square( CHUMTOAD_COMFORT_RADIUS ) )
			{
				SetCondition( COND_CHUMTOAD_UNCOMFORTABLE );
				ClearCondition( COND_CHUMTOAD_COMFORTABLE );
				break;
			}
		}
		if ( pSightEnt->IsPlayer() && pSightEnt->IsAlive() )
		{
			float flDist = (pSightEnt->GetAbsOrigin() - GetAbsOrigin()).LengthSqr();
			if ( flDist <= Square( CHUMTOAD_COMFORT_RADIUS ) )
			{
				SetCondition( COND_CHUMTOAD_UNCOMFORTABLE );
				ClearCondition( COND_CHUMTOAD_COMFORTABLE );
				break;
			}
		}

		pSightEnt = GetSenses()->GetNextSeenEntity( &iter );
	}
}

//-----------------------------------------------------------------------------
int CNPC_Chumtoad::SelectSchedule( void )
{
	if ( m_bPlayingDead && HasCondition( COND_CHUMTOAD_COMFORTABLE ) )
	{
		SetSkin( 0 ); // open eye
		return SCHED_CHUMTOAD_PLAY_DEAD_END;
	}

	if ( HasCondition( COND_CHUMTOAD_PLAY_DEAD ) )
	{
		ClearCondition( COND_CHUMTOAD_PLAY_DEAD );
		ClearCondition( COND_CHUMTOAD_COMFORTABLE );
		EmitSound( "Chumtoad.Squeak" );
		SetSkin( CHUMTOAD_EYE_FRAMES - 1 ); // close eye
		m_flPlayDeadStartTime = gpGlobals->curtime;
		return SCHED_CHUMTOAD_PLAY_DEAD;
	}

	if ( BehaviorSelectSchedule() )
		return BaseClass::SelectSchedule();

	if ( HasCondition( COND_CHUMTOAD_UNCOMFORTABLE ) )
	{
		return SCHED_MOVE_AWAY;
	}

	return BaseClass::SelectSchedule();
}


//-----------------------------------------------------------------------------
void CNPC_Chumtoad::StartTask( const Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_CHUMTOAD_PLAY_DEAD:
		m_bPlayingDead = pTask->flTaskData != 0;
#if 0
		if ( m_bPlayingDead )
			AddSolidFlags( FSOLID_NOT_SOLID );
		else
			RemoveSolidFlags( FSOLID_NOT_SOLID );
#endif
		TaskComplete();
		break;
	default:
		BaseClass::StartTask(pTask);
		break;
	}
}

//-----------------------------------------------------------------------------
void CNPC_Chumtoad::RunTask( const Task_t *pTask )
{
	switch ( pTask->iTask )
	{
#ifdef CHUMTOAD_ENGINE
	case TASK_FALL_TO_GROUND:
		if ( (GetFlags() & FL_ONGROUND) || (HasEngine() && GetWaterLevel() > 0) )
		{
			TaskComplete();
		}
		else if( GetFlags() & FL_FLY )
		{
			// We're never going to fall if we're FL_FLY.
			RemoveFlag( FL_FLY );
		}
		else
		{
			if( IsWaitFinished() )
			{
				// After 4 seconds of trying to fall to ground, Assume that we're in a bad case where the NPC
				// isn't actually falling, and make an attempt to slam the ground entity to whatever's under the NPC.
				Vector maxs = WorldAlignMaxs() - Vector( .1, .1, .2 );
				Vector mins = WorldAlignMins() + Vector( .1, .1, 0 );
				Vector vecStart	= GetAbsOrigin() + Vector( 0, 0, .1 );
				Vector vecDown	= GetAbsOrigin();
				vecDown.z -= 0.2;

				trace_t trace;
				GetMoveProbe()->TraceHull( vecStart, vecDown, mins, maxs, MASK_NPCSOLID, &trace );

				if( trace.m_pEnt )
				{
					// Found something!
					SetGroundEntity( trace.m_pEnt );
					TaskComplete();
				}
				else
				{
					// Try again in a few seconds.
					SetWait(4);
				}
			}
		}
			break;
#endif
	case TASK_MOVE_AWAY_PATH:
		{
			QAngle ang = GetLocalAngles();
			Vector move;

			switch ( GetTaskInterrupt() )
			{
			case 0:
				{
#if 0
					ang.y = GetMotor()->GetIdealYaw() + 180;
					AngleVectors( ang, &move );
					if ( GetNavigator()->SetVectorGoal( move, CHUMTOAD_COMFORT_RADIUS, min(36,CHUMTOAD_COMFORT_RADIUS), true ) &&
						IsValidMoveAwayDest( GetNavigator()->GetGoalPos() ))
					{
						TaskComplete();
						break;
					}
#endif
					int sign = -random->RandomInt(0, 1);
					for ( float dy = -90; dy <= 90; dy += 10 )
					{
						ang.y = GetMotor()->GetIdealYaw() + 180 + dy * sign;
						AngleVectors( ang, &move );
						if ( GetNavigator()->SetVectorGoal( move, CHUMTOAD_COMFORT_RADIUS, min(24,CHUMTOAD_COMFORT_RADIUS), true ) &&
							IsValidMoveAwayDest( GetNavigator()->GetGoalPos() ) )
						{
							TaskComplete();
							return;
						}
					}

					TaskInterrupt();
				}
				break;

			case 1:
				{
					ang.y = GetMotor()->GetIdealYaw() + 271;
					AngleVectors( ang, &move );

					if ( GetNavigator()->SetVectorGoal( move, CHUMTOAD_COMFORT_RADIUS, min(24,CHUMTOAD_COMFORT_RADIUS), true ) &&
						IsValidMoveAwayDest( GetNavigator()->GetGoalPos() ) )
					{
						TaskComplete();
					}
					else
					{
						ang.y = GetMotor()->GetIdealYaw() + 180;
						while (ang.y < 0)
							ang.y += 360;
						while (ang.y >= 360)
							ang.y -= 360;
						if ( ang.y < 45 || ang.y >= 315 )
							ang.y = 0;
						else if ( ang.y < 135 )
							ang.y = 90;
						else if ( ang.y < 225 )
							ang.y = 180;
						else
							ang.y = 270;

						AngleVectors( ang, &move );

						if ( GetNavigator()->SetVectorGoal( move, CHUMTOAD_COMFORT_RADIUS, min(6,CHUMTOAD_COMFORT_RADIUS), false ) &&
							IsValidMoveAwayDest( GetNavigator()->GetGoalPos() ) )
						{
							TaskComplete();
						}
						else
						{
							TaskInterrupt();
						}
					}
				}
				break;

			case 2:
				{
					ClearTaskInterrupt();
					Vector coverPos;

					if ( GetTacticalServices()->FindCoverPos( GetLocalOrigin(), EyePosition(), 0, CoverRadius(), &coverPos ) &&
						IsValidMoveAwayDest( GetNavigator()->GetGoalPos() ) ) 
					{
						GetNavigator()->SetGoal( AI_NavGoal_t( coverPos, ACT_RUN ) );
						m_flMoveWaitFinished = gpGlobals->curtime + 2;
					}
					else
					{
						// no coverwhatsoever.
						TaskFail(FAIL_NO_ROUTE);
					}
				}
				break;

			}
		}
		break;
#if 0
	case TASK_DIE:
		{
			// BaseNPC::Event_Killed
			//    BaseNPC::CleanupOnDeath
			// BaseNPC::SelectDeadSchedule
			//    BaseNPC::CleanupOnDeath
			if ( IsActivityFinished() )
			{
				SetCycle(1.0); // seems to get stuck at 0.999
				//m_iHealth = 0;
				AddSolidFlags( FSOLID_NOT_SOLID );
				//SetState( NPC_STATE_DEAD );
				BaseClass::RunTask(pTask);
			}
		}
		break;
#endif
	default:
		BaseClass::RunTask(pTask);
		break;
	}
}

//-----------------------------------------------------------------------------
bool CNPC_Chumtoad::IsValidMoveAwayDest( const Vector &vecDest )
{
	CBaseEntity *ppEnts[256];
	Vector vecCenter = vecDest;
	float flRadius = CHUMTOAD_COMFORT_RADIUS;
	int nEntCount = UTIL_EntitiesInSphere( ppEnts, 256, vecCenter, flRadius, 0 );
	for ( int i = 0; i < nEntCount; i++ )
	{
		if ( ppEnts[i] == NULL || ppEnts[i] == this || !ppEnts[i]->IsAlive() )
			continue;

		if ( ppEnts[i]->MyNPCPointer() == NULL )
			continue;

		if ( ppEnts[i]->m_iClassname == m_iClassname )
			continue;

		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
Activity CNPC_Chumtoad::NPC_TranslateActivity( Activity eNewActivity )
{
	eNewActivity = BaseClass::NPC_TranslateActivity( eNewActivity );

	// Support for namc4 chumtoad water maze.
	if ( eNewActivity == ACT_WALK || eNewActivity == ACT_RUN )
	{
#ifdef CHUMTOAD_ENGINE
		if ( HasEngine() )
		{
			Vector right;
			GetVectors( NULL, &right, NULL );
			Vector forwardVelocity = CrossProduct ( m_vecPhysVelocity, right );
			if ( GetNavigator()->IsGoalActive() && forwardVelocity.IsLengthGreaterThan( 30 ) )
				return ACT_CHUMTOAD_ENGINE_SWIM;
			return ACT_IDLE;
		}
#endif
		if ( GetWaterLevel() > 0 )
			return ACT_SWIM; // NOTE: not true swimming (FL_SWIM etc)
	}
	return eNewActivity;
}

//-----------------------------------------------------------------------------
void CNPC_Chumtoad::OnEvent( int event )
{
#ifdef CHUMTOAD_ENGINE
	switch ( event )
	{
		case 100:
		{
			if ( 1/*HasCondition( COND_SEE_PLAYER )*/ )
			{
				if ( HasEngine() )
				{
					m_nHeadlightBlinks = random->RandomInt(2,3);
					// This 5 seconds is tied to SCHED_LEAD_PAUSE
					m_flHeadlightBlinkTime = gpGlobals->curtime + 2;
				}
			}
			else
			{
			}
			SetActivity( ACT_IDLE );
			break;
		}
#endif
		default:
			Assert( 0 );
			break;
	}
}

#ifdef CHUMTOAD_ENGINE
//-----------------------------------------------------------------------------
void CNPC_Chumtoad::StopLoopingSounds( void )
{
	CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();

	controller.SoundDestroy( m_pEngineSound );
	m_pEngineSound = NULL;

	SetContextThink( NULL/*&CNPC_Chumtoad::RampEngineThink*/,
		TICK_NEVER_THINK,
		RAMP_ENGINE_THINK_CONTEXT );

	BaseClass::StopLoopingSounds();
}

//-----------------------------------------------------------------------------
bool CNPC_Chumtoad::OverrideMove( const Vector &MoveTarget, float flInterval )
{
	if ( !HasEngine() || GetWaterLevel() == 0 )
		return false;

	if( GetFlags() & FL_ONGROUND )
	{
		// This would be really bad.
		SetGroundEntity( NULL );
	}

	m_flGroundSpeed = m_flEngineSpeed; /*GetGroundSpeed();*/

#if 1
	Vector vecMoveGoal = MoveTarget;
	IchthyosaurMoveType_t moveType;
	if ( GetNavigator()->IsGoalActive() )
	{
//		vecMoveGoal = GetNavigator()->GetCurWaypointPos();
		if ( GetNavigator()->CurWaypointIsGoal() )
			moveType = ICH_MOVETYPE_ARRIVE;
		else
		{
			moveType = ICH_MOVETYPE_SEEK;
#if 1
#elif 0
			AI_Waypoint_t *pCurWaypoint	 = GetNavigator()->GetPath()->GetCurWaypoint();
			Vector prevWaypointPos = m_vPrevWaypointPos != vec3_origin ? m_vPrevWaypointPos : pCurWaypoint->GetPos();
			AI_Waypoint_t *pNextWaypoint = pCurWaypoint->GetNext();

			Vector vecCurToNext;
			VectorSubtract( pNextWaypoint->GetPos(), pCurWaypoint->GetPos(), vecCurToNext );
			VectorNormalize( vecCurToNext );

			Vector vecPrevToCur;
			VectorSubtract( pCurWaypoint->GetPos(), prevWaypointPos, vecPrevToCur );
			VectorNormalize( vecPrevToCur );
			float flDot = DotProduct( vecCurToNext, vecPrevToCur );

			if ( flDot < 0.5 )
				moveType = ICH_MOVETYPE_ARRIVE;
#else
			AI_Waypoint_t *pCurWaypoint	 = GetNavigator()->GetPath()->GetCurWaypoint();
			Vector prevWaypointPos = m_vPrevWaypointPos != vec3_origin ? m_vPrevWaypointPos : pCurWaypoint->GetPos();
			AI_Waypoint_t *pNextWaypoint = pCurWaypoint->GetNext();

			Vector vecDestVelocity;
			VectorSubtract( pNextWaypoint->GetPos(), pCurWaypoint->GetPos(), vecDestVelocity );
			VectorNormalize( vecDestVelocity );

			// Slow it down if we're approaching a sharp corner
			Vector vecDelta;
			VectorSubtract( pCurWaypoint->GetPos(), prevWaypointPos, vecDelta );
			VectorNormalize( vecDelta );
			float flDot = DotProduct( vecDestVelocity, vecDelta );
			vecDestVelocity *= clamp( flDot, 0.0f, 1.0f );

			float flSpeed = GetAbsVelocity().Length();
			float flTime = 1.0f;
			float flDistToDesired = (pCurWaypoint->GetPos() - GetAbsOrigin()).Length();

			// Blend in a fake destination point based on the dest velocity 
			vecDestVelocity *= flSpeed;
			float flBlendFactor = 1.0f - flDistToDesired / (flSpeed * flTime);
			VectorMA( vecMoveGoal, flTime * flBlendFactor, vecDestVelocity, vecMoveGoal );
#endif
		}
	}
	else
	{
		vecMoveGoal = m_vHoverPos != vec3_origin ? m_vHoverPos : GetAbsOrigin();
		moveType = ICH_MOVETYPE_ARRIVE;
	}

	float waterLevel = UTIL_FindWaterSurface( GetAbsOrigin(), GetAbsOrigin().z, GetAbsOrigin().z+1024 );
	vecMoveGoal.z = waterLevel - 4;

	DoMovement( flInterval, vecMoveGoal, moveType );

	m_vecPhysVelocity = GetAbsVelocity();
	m_nExactWaterLevel = waterLevel;

	m_vHoverPos = vecMoveGoal;
#if 0
	float flDist = (GetAbsOrigin() - vecMoveGoal).Length2D();
	if ( flDist < GetHullWidth() )
	{
		if ( GetNavigator()->IsGoalActive() )
		{
			if ( GetNavigator()->CurWaypointIsGoal() )
			{
				TaskMovementComplete();
			}
			else
			{
				m_vPrevWaypointPos = GetNavigator()->GetCurWaypointPos();
				GetNavigator()->AdvancePath();
			}
		}
	}
#endif
#else
	if ( m_bHasMoveTarget )
	{
		DoMovement( flInterval, m_vecLastMoveTarget, ICH_MOVETYPE_ARRIVE );
	}
	else
	{
		DoMovement( flInterval, GetLocalOrigin(), ICH_MOVETYPE_ARRIVE );
	}
#endif
	return true;
}

//-----------------------------------------------------------------------------
void CNPC_Chumtoad::RampEngineThink( void )
{
	float flInterval = gpGlobals->curtime - GetLastThink( RAMP_ENGINE_THINK_CONTEXT );
	if ( flInterval == gpGlobals->curtime )
		flInterval = 0.05;

	// If the engine is running the prop/pistons never completely stop
	if ( m_bEngineStarted )
		m_flEngineRatioGoal = max( m_flEngineRatioGoal, 0.08 );

	float engineMinToMaxTime = 2.0; // time for engine to go from 0 to full throttle
	float delta = flInterval / engineMinToMaxTime;
	if ( m_flEngineRatio > m_flEngineRatioGoal )
	{
		m_flEngineRatio -= delta;
		m_flEngineRatio = clamp( m_flEngineRatio, m_flEngineRatioGoal, 1.0 );
	}
	else if ( m_flEngineRatio < m_flEngineRatioGoal )
	{
		m_flEngineRatio += delta;
		m_flEngineRatio = clamp( m_flEngineRatio, 0.0, m_flEngineRatioGoal );
	}

//	DevMsg( "chumtoad engine speed fraction %.2f goal %.2f\n", m_flEngineRatio, m_flEngineRatioGoal );

	CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
	if ( m_bEngineStarted )
	{
		controller.SoundChangeVolume( m_pEngineSound,
			ENGINE_MIN_VOLUME * (1.0 - m_flEngineRatio) + ENGINE_MAX_VOLUME * m_flEngineRatio, 0.1 );
		controller.SoundChangePitch( m_pEngineSound,
			ENGINE_MIN_PITCH * (1.0 - m_flEngineRatio) + ENGINE_MAX_PITCH * m_flEngineRatio, 0.1 );
	}
	else if ( m_flEngineIdleTime != 0 && m_flEngineIdleTime <= gpGlobals->curtime )
	{
		controller.SoundChangeVolume( m_pEngineSound, ENGINE_MIN_VOLUME, 0.1 );
		controller.SoundChangePitch( m_pEngineSound, ENGINE_MIN_PITCH, 0.1 );
		m_flNextBackfireTime = gpGlobals->curtime + 2.0f;
		m_flEngineIdleTime = 0;
		m_bEngineStarted = true;
	}

	if ( m_iEngineGestureLayer != -1 )
		SetLayerPlaybackRate( m_iEngineGestureLayer, m_flEngineRatio );

	SetContextThink( &CNPC_Chumtoad::RampEngineThink,
		gpGlobals->curtime + 0.05,
		RAMP_ENGINE_THINK_CONTEXT );
}

#define	LATERAL_NOISE_MAX	2.0f
#define	LATERAL_NOISE_FREQ	1.0f

#define	VERTICAL_NOISE_MAX	2.0f
#define	VERTICAL_NOISE_FREQ	1.0f

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : velocity - 
//-----------------------------------------------------------------------------
void CNPC_Chumtoad::AddSwimNoise( Vector *velocity )
{
	//////
	return;
	//////

	Vector	right, up;

	GetVectors( NULL, &right, &up );

	float	lNoise, vNoise;

	lNoise = LATERAL_NOISE_MAX * sin( gpGlobals->curtime * LATERAL_NOISE_FREQ );
	vNoise = VERTICAL_NOISE_MAX * sin( gpGlobals->curtime * VERTICAL_NOISE_FREQ );

	(*velocity) += ( right * lNoise ) + ( up * vNoise );
}

//-----------------------------------------------------------------------------
void CNPC_Chumtoad::ComputeAngularVelocity( float flInterval, float yaw )
{
	QAngle curAngles = GetAbsAngles();
	QAngle newAngles;

	newAngles.x = 0;
	newAngles.y = yaw;
	newAngles.z = 0;

	float vx, vy, vz;
	vx = UTIL_AngleDistance( UTIL_ApproachAngle( newAngles.x, curAngles.x, 10 ), curAngles.x );
	vy = UTIL_AngleDistance( UTIL_ApproachAngle( newAngles.y, curAngles.y, MaxYawSpeed() ), curAngles.y );

	float flBank = fabs(vy /*/ flInterval*/) * 2 * m_flEngineRatio;
	flBank = min( flBank, 35 );
	float flBankSpeed = 5;
	if ( vy /*/ flInterval*/ > 1 )
	{
		flBank *= -1;
	}
	else if ( vy /*/ flInterval*/ < -1 )
	{
	}
	else
	{
		flBank = 0;
	}
	vz = UTIL_AngleDistance( UTIL_ApproachAngle( flBank, curAngles.z, flBankSpeed ), curAngles.z);

	QAngle vecAngVel( vx, vy, vz );

	SetLocalAngularVelocity( vecAngVel / flInterval );
}

//-----------------------------------------------------------------------------
void CNPC_Chumtoad::DoMovement( float flInterval, const Vector &MoveTarget, int eMoveType )
{
	if (m_debugOverlays & OVERLAY_NPC_SELECTED_BIT)
		NDebugOverlay::Cross3D( MoveTarget, 16, 255, 255, 255, true, .1 );

	// dvs: something is setting this bit, causing us to stop moving and get stuck that way
	Forget( bits_MEMORY_TURNING );

	Vector Steer, SteerAvoid, SteerRel;
	Vector forward, right, up;

	//Get our orientation vectors.
	GetVectors( &forward, &right, &up);

	if ( ( GetActivity() == ACT_MELEE_ATTACK1 ) && ( GetEnemy() != NULL ) )
	{
		SteerSeek( Steer, GetEnemy()->GetAbsOrigin() );
	}
	else
	{
		//If we are approaching our goal, use an arrival steering mechanism.
		if ( eMoveType == ICH_MOVETYPE_ARRIVE )
		{
			SteerArrive( Steer, MoveTarget );
		}
		else
		{
			//Otherwise use a seek steering mechanism.
			SteerSeek( Steer, MoveTarget );
		}
	}
	
#if FEELER_COLLISION

	Vector f, u, l, r, d;

	float	probeLength = GetAbsVelocity().Length();

	if ( probeLength < 150 )
		probeLength = 150;

	if ( probeLength > 500 )
		probeLength = 500;

	f = DoProbe( GetLocalOrigin() + (probeLength * forward) );
	r = DoProbe( GetLocalOrigin() + (probeLength/3 * (forward+right)) );
	l = DoProbe( GetLocalOrigin() + (probeLength/3 * (forward-right)) );
	u = DoProbe( GetLocalOrigin() + (probeLength/3 * (forward+up)) );
	d = DoProbe( GetLocalOrigin() + (probeLength/3 * (forward-up)) );

	SteerAvoid = f+r+l+u+d;
	
	//NDebugOverlay::Line( GetLocalOrigin(), GetLocalOrigin()+SteerAvoid, 255, 255, 0, false, 0.1f );	

	if ( SteerAvoid.LengthSqr() )
	{
		Steer = (SteerAvoid*0.5f);
	}

	m_vecVelocity = m_vecVelocity + (Steer*0.5f);

	VectorNormalize( m_vecVelocity );

	SteerRel.x = forward.Dot( m_vecVelocity );
	SteerRel.y = right.Dot( m_vecVelocity );
	SteerRel.z = up.Dot( m_vecVelocity );

	m_vecVelocity *= m_flGroundSpeed;

#else

	//See if we need to avoid any obstacles.
	if ( SteerAvoidObstacles( SteerAvoid, GetAbsVelocity(), forward, right, up ) )
	{
		//Take the avoidance vector
//		Steer = SteerAvoid;
	}

	//Clamp our ideal steering vector to within our physical limitations.
	ClampSteer( Steer, SteerRel, forward, right, up );

//	NDebugOverlay::YawArrow( GetAbsOrigin(), CalcIdealYaw( GetAbsOrigin() + Steer ), 48, 12, 0, 255, 0, 200, true, flInterval );
	float flEngineSpeed = 0.0f; // How hard is the engine working? Not our current velocity.
	Vector DesiredVelocity = Steer + GetAbsVelocity();
	Vector NormalizedSteer = Steer;
	VectorNormalize( NormalizedSteer );
	float flDot = DotProduct( forward, NormalizedSteer );
#if 1 // sep 22 yaw
	bool bApplyPower = ( m_flEngineRatio > 0 );
	if ( bApplyPower )
	{
		flEngineSpeed = DesiredVelocity.Length() / m_flEngineRatio;
		flEngineSpeed *= 1.05; //compensate for drag

//		if ( flDot <= 0 )
//			flEngineSpeed = 0;
	}
#else // sep 22 yaw
	bool bApplyPower = ( m_flEngineRatio > 0 ) && ( ( flDot >= 0.707 ) || !GetNavigator()->IsGoalActive()/*( Steer.Length() < 10 )*/ );
	if ( bApplyPower )
	{
		flEngineSpeed = DesiredVelocity.Length() / m_flEngineRatio;
		flEngineSpeed *= 1.05; //compensate for drag
	}
#endif // sep 22 yaw
	// Don't stomp the goal=0.1 when starting the engine
	if ( m_bEngineStarted )
		m_flEngineRatioGoal = clamp( flEngineSpeed / m_flEngineSpeed, 0.1, 1.0 );

	ApplySidewaysDrag( flInterval, right );
	ApplyGeneralDrag( flInterval );

	if ( bApplyPower )
	{
		Steer *= 1.05; //compensate for drag
#if 1 // sep 22 yaw
		if ( eMoveType != ICH_MOVETYPE_ARRIVE )
			ApplyAbsVelocityImpulse( forward * m_flEngineSpeed * m_flEngineRatio * flInterval );
		else
			ApplyAbsVelocityImpulse( Steer * flInterval );
#else // sep 22 yaw
		ApplyAbsVelocityImpulse( Steer * flInterval * m_flEngineRatio );
#endif // sep 22 yaw
		if (m_debugOverlays & OVERLAY_NPC_SELECTED_BIT)
			NDebugOverlay::YawArrow( GetAbsOrigin(), CalcIdealYaw( GetAbsOrigin() + Steer ), Steer.Length(), 6, 0, 0, 255, 128, true, flInterval );
	}
	if (m_debugOverlays & OVERLAY_NPC_SELECTED_BIT)
		NDebugOverlay::YawArrow( GetAbsOrigin(), CalcIdealYaw( GetAbsOrigin() + GetAbsVelocity() * 100 ), GetAbsVelocity().Length(), 6, 0, 255, 0, 128, true, flInterval );
#endif

#if 0 // sep 22 yaw
#if 1
	// Apply magical braking if facing opposite of our desired engine thrust
	// to avoid overshooting the final goal position.
	Vector vecDeltaToTarget = MoveTarget - GetAbsOrigin();
	float flDot2 = DotProduct( Steer, vecDeltaToTarget );
	if ( flDot2 < 0.0f )
	{
		Vector vecBrake = Steer;
		VectorMA( vecBrake, -flDot * 0.1f, vecDeltaToTarget, vecBrake );
		ApplyAbsVelocityImpulse( vecBrake * flInterval );
		if (m_debugOverlays & OVERLAY_NPC_SELECTED_BIT)
			NDebugOverlay::YawArrow( GetAbsOrigin(), CalcIdealYaw( GetAbsOrigin() + vecBrake ), vecBrake.Length(), 6, 255, 0, 0, 128, true, flInterval );
	}
#else
	// Arbitrary-direction braking at destination
	Vector vecDeltaToTarget = MoveTarget - GetAbsOrigin();
	if ( eMoveType == ICH_MOVETYPE_ARRIVE && vecDeltaToTarget.Length() < ICH_WAYPOINT_DISTANCE )
	{
		Vector vecBrake = -GetAbsVelocity();
//		if ( (vecBrake + GetAbsVelocity() * flInterval).Length() < vecDeltaToTarget.Length() * flInterval )
//			vecBrake = vecDeltaToTarget - GetAbsVelocity();
		ApplyAbsVelocityImpulse( vecBrake * flInterval );
		NDebugOverlay::YawArrow( GetAbsOrigin(), CalcIdealYaw( GetAbsOrigin() + vecBrake ), vecBrake.Length(), 6, 255, 0, 0, 128, true, flInterval );
	}
#endif
#endif // sep 22 yaw

#if 0
	// apply magical velocity to move us toward our final goal
	float radius = 30;
	if ( eMoveType == ICH_MOVETYPE_ARRIVE &&
		vecDeltaToTarget.IsLengthLessThan( radius ) )
	{
		SetAbsVelocity( 0 );
		float timeToMoveRadius = 2.0f; // 2 seconds
		float radiusFrac = vecDeltaToTarget.Length() / radius;
		float distPerSecond = radius/timeToMoveRadius; // velocity
		Vector norm = vecDeltaToTarget; VectorNormalize( norm );
		WalkMove( norm * distPerSecond * flInterval, MASK_NPCSOLID );
	}
#endif

	Vector vecNewVelocity = GetAbsVelocity();
	float flLength = vecNewVelocity.Length();

#if 1
	// keep below the surface
//	if ( (GetAbsOrigin() + GetAbsVelocity() + Steer * flInterval).z > MoveTarget.z )
	vecNewVelocity.z = MoveTarget.z - GetAbsOrigin().z;
#endif

	//Clamp our final speed
	if ( flLength > m_flGroundSpeed )
	{
		vecNewVelocity *= ( m_flGroundSpeed / flLength );
		flLength = m_flGroundSpeed;
	}

	Vector	workVelocity = vecNewVelocity;

	AddSwimNoise( &workVelocity );

	// Pose the fish properly
//	SetPoses( SteerRel, flLength );

	// If we are close to our desired destination and aren't powering elsewhere
	// then face our ideal yaw.  Otherwise face the direction our engine should
	// be powering us.
	if ( GetMotor()->GetIdealYaw() != m_EngineYaw )
		m_NonEngineYaw = GetMotor()->GetIdealYaw();
	float yaw = m_NonEngineYaw; /*GetMotor()->GetIdealYaw()*/;
	if ( GetNavigator()->IsGoalActive()/*DesiredVelocity.Length() > 15*/ )
	{
#if 1 // sep 22 yaw
		if ( eMoveType == ICH_MOVETYPE_ARRIVE )
			yaw = UTIL_AngleMod( UTIL_VecToYaw( GetNavigator()->GetArrivalDirection() ) );
		else
		{
			Vector vecYaw;
			VectorLerp( DesiredVelocity, Steer, 0.3, vecYaw );
			yaw = UTIL_AngleMod( UTIL_VecToYaw( vecYaw ) );
		}
#else // sep 22 yaw
		if ( eMoveType == ICH_MOVETYPE_ARRIVE && Steer.IsLengthGreaterThan( 15 ) )
			yaw = UTIL_AngleMod( CalcIdealYaw( GetAbsOrigin() + Steer ) );
		else
		{
			Vector vecYaw;
			VectorLerp( DesiredVelocity, Steer, 0.3, vecYaw );
			yaw = UTIL_AngleMod( CalcIdealYaw( GetAbsOrigin() + vecYaw/*DesiredVelocity*/ ) );
		}
#endif // sep 22 yaw
		m_EngineYaw = yaw;
	}
#if 1 // sep 22 yaw
	GetMotor()->SetIdealYaw( yaw );
	ComputeAngularVelocity( flInterval, yaw );

	// Subtract velocity according to how much we are banking (the water applies braking)
	Vector vecBankVel = (Vector(0,0,1) - up) * 5000 * flInterval;
	vecBankVel.z = 0;
	Vector dir = -vecBankVel;
	dir.NormalizeInPlace();
	flDot = DotProduct( vecNewVelocity, dir );
	if ( flDot < 0 )
	{
		vecNewVelocity -= dir * flDot * 2.0 * flInterval ;
	}
#else // sep 22 yaw
	GetMotor()->SetIdealYawAndUpdate( yaw, AI_KEEP_YAW_SPEED );
#endif // sep 22 yaw

#if 0
	//Move along the current velocity vector
	if ( WalkMove( workVelocity * flInterval, MASK_NPCSOLID ) == false )
	{
		//Attempt a half-step
		if ( WalkMove( (workVelocity*0.5f) * flInterval,  MASK_NPCSOLID) == false )
		{
			//Restart the velocity
			//VectorNormalize( m_vecVelocity );
			vecNewVelocity *= 0.5f;
		}
		else
		{
			//Cut our velocity in half
			vecNewVelocity *= 0.5f;
		}
	}
#endif

	SetAbsVelocity( vecNewVelocity );
}

//-----------------------------------------------------------------------------
// Purpose: Gets a steering vector to arrive at a target location with a
//			relatively small velocity.
// Input  : Steer - Receives the ideal steering vector.
//			Target - Target position at which to arrive.
//-----------------------------------------------------------------------------
void CNPC_Chumtoad::SteerArrive(Vector &Steer, const Vector &Target)
{
	Vector Offset = Target - GetLocalOrigin();
	float fTargetDistance = Offset.Length();

	float fIdealSpeed = m_flGroundSpeed * (fTargetDistance / ICH_WAYPOINT_DISTANCE);
	float fClippedSpeed = min( fIdealSpeed, m_flGroundSpeed );
	
	Vector DesiredVelocity( 0, 0, 0 );

	if ( fTargetDistance > ICH_WAYPOINT_DISTANCE )
	{
		DesiredVelocity = (fClippedSpeed / fTargetDistance) * Offset;
	}
	else
	{
		DesiredVelocity = Offset - GetAbsVelocity();
	}

	Steer = DesiredVelocity - GetAbsVelocity();
}


//-----------------------------------------------------------------------------
// Purpose: Gets a steering vector to move towards a target position as quickly
//			as possible.
// Input  : Steer - Receives the ideal steering vector.
//			Target - Target position to seek.
//-----------------------------------------------------------------------------
void CNPC_Chumtoad::SteerSeek( Vector &Steer, const Vector &Target )
{
	Vector offset = Target - GetLocalOrigin();
	
	VectorNormalize( offset );
	
	Vector DesiredVelocity = m_flGroundSpeed * offset;
	
	Steer = DesiredVelocity - GetAbsVelocity();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &Steer - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CNPC_Chumtoad::SteerAvoidObstacles(Vector &Steer, const Vector &Velocity, const Vector &Forward, const Vector &Right, const Vector &Up)
{
	trace_t	tr;

	bool	collided = false;
	Vector	dir = Velocity;
	float	speed = VectorNormalize( dir );

	//Look ahead one second and avoid whatever is in our way.
#if 1
	dir.z = 0;
	AI_TraceHull( GetAbsOrigin() + Vector(0,0,1), GetAbsOrigin() + Vector(0,0,1) + (dir*speed), GetHullMins(), GetHullMaxs(), MASK_NPCSOLID, this, COLLISION_GROUP_NONE, &tr );
#else
	AI_TraceHull( GetAbsOrigin(), GetAbsOrigin() + (dir*speed), GetHullMins(), GetHullMaxs(), MASK_NPCSOLID, this, COLLISION_GROUP_NONE, &tr );
#endif
	Vector	forward;

	GetVectors( &forward, NULL, NULL );

	//If we're hitting our enemy, just continue on
	if ( ( GetEnemy() != NULL ) && ( tr.m_pEnt == GetEnemy() ) )
		return false;

	if ( tr.fraction < 1.0f )
	{
		CBaseEntity *pBlocker = tr.m_pEnt;
		
		if ( ( pBlocker != NULL ) && ( pBlocker->MyNPCPointer() != NULL ) )
		{
			DevMsg( 2, "Avoiding an NPC\n" );

			Vector HitOffset = tr.endpos - GetAbsOrigin();

			Vector SteerUp = CrossProduct( HitOffset, Velocity );
			Steer = CrossProduct(  SteerUp, Velocity  );
			VectorNormalize( Steer );

			/*Vector probeDir = tr.endpos - GetAbsOrigin();
			Vector normalToProbeAndWallNormal = probeDir.Cross( tr.plane.normal );
			
			Steer = normalToProbeAndWallNormal.Cross( probeDir );
			VectorNormalize( Steer );*/

			if ( tr.fraction > 0 )
			{
				Steer = (Steer * Velocity.Length()) / tr.fraction;
				//NDebugOverlay::Line( GetLocalOrigin(), GetLocalOrigin()+Steer, 255, 0, 0, false, 0.1f );
			}
			else
			{
				Steer = (Steer * 1000 * Velocity.Length());
				//NDebugOverlay::Line( GetLocalOrigin(), GetLocalOrigin()+Steer, 255, 0, 0, false, 0.1f );
			}
		}
		else
		{
			if ( ( pBlocker != NULL ) && ( pBlocker == GetEnemy() ) )
			{
				DevMsg( "Avoided collision\n" );
				return false;
			}

			DevMsg( 2, "Avoiding the world\n" );
			
			Vector	steeringVector = tr.plane.normal;

			if ( tr.fraction == 0.0f )
				return false;

			Steer = steeringVector * ( Velocity.Length() / tr.fraction );
			
			//NDebugOverlay::Line( GetLocalOrigin(), GetLocalOrigin()+Steer, 255, 0, 0, false, 0.1f );
		}

		//return true;
		collided = true;
	}

#if 1
	float waterLevel = UTIL_WaterLevel( GetAbsOrigin(), GetAbsOrigin().z, GetAbsOrigin().z+1024 );
	waterLevel -= 6;
	if ( GetAbsOrigin().z < waterLevel - 2 )
	{
		Steer += Vector( 0, 0, m_vecAccelerationMax.z );
//		collided = true;
	}
	else if ( GetAbsOrigin().z > waterLevel + 2 )
	{
		Steer += -Vector( 0, 0, m_vecAccelerationMax.z );
//		collided = true;
	}
#else
	//Try to remain 8 feet above the ground.
	AI_TraceLine( GetAbsOrigin(), GetAbsOrigin() + Vector(0, 0, -ICH_HEIGHT_PREFERENCE), MASK_NPCSOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr );

	if ( tr.fraction < 1.0f )
	{
		Steer += Vector( 0, 0, m_vecAccelerationMax.z / tr.fraction );
		collided = true;
	}
	
	//Stay under the surface
	if ( m_bIgnoreSurface == false )
	{
		float waterLevel = ( UTIL_WaterLevel( GetAbsOrigin(), GetAbsOrigin().z, GetAbsOrigin().z+ICH_DEPTH_PREFERENCE ) - GetAbsOrigin().z ) / ICH_DEPTH_PREFERENCE;

		if ( waterLevel < 1.0f )
		{
			Steer += -Vector( 0, 0, m_vecAccelerationMax.z / waterLevel );
			collided = true;
		}
	}
#endif
	return collided;
}


//-----------------------------------------------------------------------------
// Purpose: Clamps the desired steering vector based on the limitations of this
//			vehicle.
// Input  : SteerAbs - The vector indicating our ideal steering vector. Receives
//				the clamped steering vector in absolute (x,y,z) coordinates.
//			SteerRel - Receives the clamped steering vector in relative (forward, right, up)
//				coordinates.
//			forward - Our current forward vector.
//			right - Our current right vector.
//			up - Our current up vector.
//-----------------------------------------------------------------------------
void CNPC_Chumtoad::ClampSteer(Vector &SteerAbs, Vector &SteerRel, Vector &forward, Vector &right, Vector &up)
{
	float fForwardSteer = DotProduct(SteerAbs, forward);
	float fRightSteer = DotProduct(SteerAbs, right);
	float fUpSteer = DotProduct(SteerAbs, up);

	if (fForwardSteer > 0)
	{
		fForwardSteer = min(fForwardSteer, m_vecAccelerationMax.x);
	}
	else
	{
		fForwardSteer = max(fForwardSteer, m_vecAccelerationMin.x);
	}

	if (fRightSteer > 0)
	{
		fRightSteer = min(fRightSteer, m_vecAccelerationMax.y);
	}
	else
	{
		fRightSteer = max(fRightSteer, m_vecAccelerationMin.y);
	}

	if (fUpSteer > 0)
	{
		fUpSteer = min(fUpSteer, m_vecAccelerationMax.z);
	}
	else
	{
		fUpSteer = max(fUpSteer, m_vecAccelerationMin.z);
	}

	SteerAbs = (fForwardSteer*forward) + (fRightSteer*right) + (fUpSteer*up);

	SteerRel.x = fForwardSteer;
	SteerRel.y = fRightSteer;
	SteerRel.z = fUpSteer;
}

//-----------------------------------------------------------------------------
void CNPC_Chumtoad::ApplySidewaysDrag( float flInterval, const Vector &vecRight )
{
	Vector vecNewVelocity = GetAbsVelocity();
	vecNewVelocity.x *= 1.0 - fabs( vecRight.x ) * 0.5 * flInterval;
	vecNewVelocity.y *= 1.0 - fabs( vecRight.y ) * 0.5 * flInterval;
	vecNewVelocity.z *= 1.0 - fabs( vecRight.z ) * 0.5 * flInterval;
	SetAbsVelocity( vecNewVelocity );
}

//-----------------------------------------------------------------------------
void CNPC_Chumtoad::ApplyGeneralDrag( float flInterval )
{
	Vector vecNewVelocity = GetAbsVelocity();
	vecNewVelocity *= 1.0 - 0.05 * flInterval;
	SetAbsVelocity( vecNewVelocity );
}

//-----------------------------------------------------------------------------
void CNPC_Chumtoad::InputEngineStart( inputdata_t &inputdata )
{
	if ( HasEngine() && !m_bEngineStarted && !m_flEngineIdleTime )
	{
		float flDuration;
		EmitSound( "Chumtoad.EngineStart", 0.0, &flDuration );

		// Start playing the engine's idle sound as the startup sound finishes.
		m_flEngineIdleTime = gpGlobals->curtime + flDuration - 0.1;

		m_flEngineRatioGoal = 0.1; // idle speed

		SetContextThink( &CNPC_Chumtoad::RampEngineThink,
			gpGlobals->curtime + 0.05,
			RAMP_ENGINE_THINK_CONTEXT );
	}
}

//-----------------------------------------------------------------------------
void CNPC_Chumtoad::InputEngineStop( inputdata_t &inputdata )
{
}

//-----------------------------------------------------------------------------
void CNPC_Chumtoad::InputEngineMalfunction( inputdata_t &inputdata )
{
	if ( HasEngine() )
	{
		m_bEngineStarted = true;
		m_flEngineRatio = 1.0;
		m_flNextBackfireTime = gpGlobals->curtime + random->RandomInt( 0.5, 1.5 );
		m_flEngineMalfTime = gpGlobals->curtime;

		// Remember engine pitch when we started to malf
		CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
		m_flEngineMalfPitch = controller.SoundGetPitch( m_pEngineSound );

//		CPASAttenuationFilter filter( this );
//		controller.SoundDestroy( m_pEngineSound );
//		m_pEngineSound = controller.SoundCreate( filter, entindex(), "Chumtoad.EngineMalfunction" );
//		controller.Play( m_pEngineSound, 0.4, 80 );

		EmitSound( "Chumtoad.Squeak" );

		SetSkin( 0 ); // no blinking...

		SetContextThink( NULL, TICK_NEVER_THINK, RAMP_ENGINE_THINK_CONTEXT );
		if ( m_iEngineGestureLayer != -1 )
			SetLayerPlaybackRate( m_iEngineGestureLayer, 1.0 );

		SetThink( &CNPC_Chumtoad::EngineMalfThink );
	}
}

//-----------------------------------------------------------------------------
void CNPC_Chumtoad::EngineMalfThink( void )
{
#define ENGINE_MALF_DURATION 8.0f

	if ( m_flEngineMalfTime < gpGlobals->curtime - ENGINE_MALF_DURATION )
	{
		SetThink( NULL );
		Ignite( 10, true, 0.5, true );
		EmitSound( "Chumtoad.EngineExplode" );
		ExplosionCreate( GetAbsOrigin() + Vector(16), GetAbsAngles(), this, 25, 64, (int)SF_ENVEXPLOSION_NOSOUND );
		return;
	}

	Vector forward, right, up;
	GetVectors( &forward, &right, &up );

	float flInterval = 0.1;
	ApplySidewaysDrag( flInterval, right );
	ApplyGeneralDrag( flInterval );

	float ratio = (gpGlobals->curtime - m_flEngineMalfTime) / ENGINE_MALF_DURATION;

	float maxSpeed = (m_flEngineSpeed*0.5) * (1.0 - ratio) + (m_flEngineSpeed * 1.5) * ratio;
	Vector velocity(maxSpeed * m_flEngineRatio);
	ApplyAbsVelocityImpulse( forward * velocity * flInterval );

	velocity = GetAbsVelocity();
	if ( velocity.IsLengthGreaterThan( maxSpeed ) )
	{
		velocity *= maxSpeed / velocity.Length();
		SetAbsVelocity( velocity );
	}

	float yaw = sin(gpGlobals->curtime) * 360;
#if 1 // sep 22 yaw
	GetMotor()->SetIdealYaw( yaw );
	ComputeAngularVelocity( flInterval, yaw );

	// Subtract velocity according to how much we are banking (the water applies braking)
	Vector vecBankVel = (Vector(0,0,1) - up);
	vecBankVel.z = 0;
	Vector dir = -vecBankVel;
	dir.NormalizeInPlace();
	float flDot = DotProduct( velocity, dir );
	if ( flDot < 0 )
	{
		velocity -= dir * flDot * 2.0 * flInterval ;
		SetAbsVelocity( velocity );
	}
#else // sep 22 yaw
	GetMotor()->SetIdealYawAndUpdate( yaw, 40 );
#endif
	// used by the client
	m_vecPhysVelocity = GetAbsVelocity();

	// used by the client
	float waterLevel = UTIL_FindWaterSurface( GetAbsOrigin(), GetAbsOrigin().z, GetAbsOrigin().z+1024 );
	m_nExactWaterLevel = waterLevel;

	// Ramp up the engine pitch
	float pitch = m_flEngineMalfPitch * (1.0 - ratio) + (ENGINE_MAX_PITCH*1.5) * ratio;
	CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
	controller.SoundChangePitch( m_pEngineSound, pitch, 0.05 );

	if ( m_flNextBackfireTime <= gpGlobals->curtime )
	{
		EmitSound( "Chumtoad.EngineBackfire" );

		// ----------
		// Muzzleflash
		// ----------
		CEffectData data;
		data.m_nEntIndex = entindex();
		data.m_nAttachmentIndex = LookupAttachment("exhaust");
		data.m_flScale = 1.0f;
		data.m_fFlags = MUZZLEFLASH_SHOTGUN;
		DispatchEffect( "MuzzleFlash", data );

		m_flNextBackfireTime = gpGlobals->curtime + random->RandomFloat( 0.5, 1.0 );
	}

	Activity act = NPC_TranslateActivity( ACT_RUN );
	if ( act != GetIdealActivity() )
		SetIdealActivity( act );
	MaintainActivity();

	StudioFrameAdvance();
	DispatchAnimEvents( this );

	SetNextThink( gpGlobals->curtime + 0.1 );
}

//---------------------------------------------------------
extern ConVar npc_vphysics;
bool CChumtoadNavigator::MoveUpdateWaypoint( AIMoveResult_t *pResult )
{
	AI_Waypoint_t *pCurWaypoint = GetPath()->GetCurWaypoint();
	float 		   waypointDist = (pCurWaypoint->GetPos() - GetAbsOrigin()).Length2D();
	bool		   bIsGoal		= CurWaypointIsGoal();
	float		   tolerance	= ( npc_vphysics.GetBool() ) ? 0.25 : 0.0625;

	// We don't hit a waypoint if we are moving too fast (FIXME: goal waypoint only?)
	CNPC_Chumtoad *pToad =  (CNPC_Chumtoad *) GetOuter();
	if ( pToad->HasEngine() && pToad->GetAbsVelocity().IsLengthGreaterThan( 10.0f ) )
		return false;
	if ( pToad->HasEngine() )
		tolerance = 4.0f;

	if ( waypointDist <= tolerance )
	{
		if ( bIsGoal )
		{
			OnNavComplete();
			*pResult = AIMR_OK;	
		}
		else
		{
			AdvancePath();
			*pResult = AIMR_CHANGE_TYPE;
		}
		return true;
	}
	
	return false;
}

//-----------------------------------------------------------------------------
AIMotorMoveResult_t CChumtoadMotor::MoveGroundExecute( const AILocalMoveGoal_t &move, AIMoveTrace_t *pTraceResult )
{
	CNPC_Chumtoad *pToad = (CNPC_Chumtoad *) GetOuter();

	if ( !pToad->HasEngine() )
		return BaseClass::MoveGroundExecute( move, pTraceResult );

#if 1
	float dist = 0.5 * (GetCurSpeed() + (pToad->m_flEngineSpeed*pToad->m_flEngineRatio)) * GetMoveInterval();
#else
	float dist = CalcIntervalMove();
#endif
	bool bReachingLocalGoal = ( dist > move.maxDist );
	float interval = GetMoveInterval();
	// can I move farther in this interval than I'm supposed to?
	if ( bReachingLocalGoal )
	{
		if ( !(move.flags & AILMG_CONSUME_INTERVAL) )
		{
			interval = GetMoveInterval() * (move.maxDist / dist);
			// only use a portion of the time interval
			SetMoveInterval( GetMoveInterval() * (1 - move.maxDist / dist) );
		}
		else
			SetMoveInterval( 0 );
		dist = move.maxDist;
	}
	else
	{
		// use all the time
		SetMoveInterval( 0 );
	}

#if 1
	Vector target = move.target - move.dir * pToad->GetNavigator()->GetArrivalDistance() * 0.95;
	pToad->OverrideMove( target, interval );
#else
	pToad->OverrideMove( move.target, interval );
#endif
	return AIM_SUCCESS;
}

#endif // CHUMTOAD_ENGINE

#ifdef CHUMTOAD_HEADLIGHT

//-----------------------------------------------------------------------------
void CNPC_Chumtoad::HeadlightTurnOn( void )
{
	EmitSound( "Chumtoad.HeadlightOn" );
	m_bHeadlightIsOn = true;
}

//-----------------------------------------------------------------------------
void CNPC_Chumtoad::HeadlightTurnOff( void )
{
	EmitSound( "Chumtoad.HeadlightOff" );
	m_bHeadlightIsOn = false;
}

//-----------------------------------------------------------------------------
void CNPC_Chumtoad::InputHeadlightOn( inputdata_t &inputdata )
{
	if ( HasEngine() && !HeadlightIsOn() )
		HeadlightTurnOn();
}

//-----------------------------------------------------------------------------
void CNPC_Chumtoad::InputHeadlightOff( inputdata_t &inputdata )
{
	if ( HasEngine() && HeadlightIsOn() )
		HeadlightTurnOff();
}

//-----------------------------------------------------------------------------
void CNPC_Chumtoad::InputHeadlightBlink( inputdata_t &inputdata )
{
	if ( HasEngine() )
	{
		m_nHeadlightBlinks = inputdata.value.Int();
		m_flHeadlightBlinkTime = gpGlobals->curtime;
	}
}

#endif // CHUMTOAD_HEADLIGHT

AI_BEGIN_CUSTOM_NPC( npc_chumtoad, CNPC_Chumtoad )

	DECLARE_CONDITION( COND_CHUMTOAD_COMFORTABLE )
	DECLARE_CONDITION( COND_CHUMTOAD_UNCOMFORTABLE )
	DECLARE_CONDITION( COND_CHUMTOAD_PLAY_DEAD )

	DECLARE_ACTIVITY( ACT_CHUMTOAD_DEAD_BEGIN )
	DECLARE_ACTIVITY( ACT_CHUMTOAD_DEAD_LOOP )
	DECLARE_ACTIVITY( ACT_CHUMTOAD_DEAD_END )
#ifdef CHUMTOAD_ENGINE
	DECLARE_ACTIVITY( ACT_CHUMTOAD_ENGINE )
	DECLARE_ACTIVITY( ACT_CHUMTOAD_ENGINE_SWIM )
#endif

	DECLARE_TASK( TASK_CHUMTOAD_PLAY_DEAD )

	DEFINE_SCHEDULE
	(
		SCHED_CHUMTOAD_PLAY_DEAD,

		"	Tasks"
		"		TASK_CHUMTOAD_PLAY_DEAD		1"
		"		TASK_STOP_MOVING			0"
		"		TASK_PLAY_SEQUENCE			ACTIVITY:ACT_CHUMTOAD_DEAD_BEGIN"
		"		TASK_PLAY_SEQUENCE			ACTIVITY:ACT_CHUMTOAD_DEAD_LOOP"
		"		TASK_WAIT_INDEFINITE		0"
		""
		"	Interrupts"
		"		COND_CHUMTOAD_COMFORTABLE"
	)

	DEFINE_SCHEDULE
	(
		SCHED_CHUMTOAD_PLAY_DEAD_END,

		"	Tasks"
		"		TASK_PLAY_SEQUENCE			ACTIVITY:ACT_CHUMTOAD_DEAD_END"
		"		TASK_CHUMTOAD_PLAY_DEAD		0"
		""
		"	Interrupts"
	)

AI_END_CUSTOM_NPC()
