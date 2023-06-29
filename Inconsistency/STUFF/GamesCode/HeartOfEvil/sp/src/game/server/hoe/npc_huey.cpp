#include "cbase.h"
#include "ammodef.h"
#include "cbasehelicopter.h"
#include "soundenvelope.h"
#include "trains.h"
#include "datacache/imdlcache.h"
#include "movevars_shared.h" // sv_gravity
#include "explode.h"
#include "beam_flags.h"
#include "hoe_human.h"
#include "effect_dispatch_data.h"
#include "te_effect_dispatch.h"
#include "SoundEmitterSystem/isoundemittersystembase.h"
#include "gib.h"
#include "smoke_trail.h"
#define LOGIC_HUEY_DEPLOY
#ifdef LOGIC_HUEY_DEPLOY
#include "saverestore_utlvector.h"
#include "logic_huey_deploy.h"
#endif
#include "weapon_rpg7.h"
#include "ai_senses.h"
#include "grenade_ar2.h"

#define RAPPEL_PHYSICSxxx
#ifdef RAPPEL_PHYSICS
#include "vphysics/constraints.h"
#include "physics_prop_ragdoll.h"
#endif

// chunks
#include "vphysics/constraints.h"
#include "physics_saverestore.h"
#include "ar2_explosion.h"
#include "IEffects.h"
#include "npc_attackchopper.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar hoe_debug_huey("hoe_debug_huey", "0");
#define DbgHuey() hoe_debug_huey.GetBool()
#define DbgHueyMsg(pszMsg,...) \
	do \
	{ \
		if (DbgHuey()) \
			DevMsg( pszMsg, __VA_ARGS__ ); \
	} while (0)

#define INVALID_ATTACHMENT 0

// The HL1 huey spawnflags are screwy:
//   SF_ROCKET    0x0032 should be 32==0x0020
//   SF_PASSENGER 0x0064 should be 64==0x0040
// As a result the huey in HL1 would gets rockets/passenger when it wasn't wanted by the mapmaker
#define SF_PASSENGER		128
#define SF_ROCKET			512
#define SF_GRENADE			1024

#define BODY_HUEY			0
#define BODY_PASSENGER		1
#define BODY_GRENADE		2

#define HUEY_PBODY_NONE		0
#define HUEY_PBODY_GRUNT	1
#define HUEY_PBODY_MIKE		2

#define HUEY_PASSENGER_GRUNT	0
#define HUEY_PASSENGER_GRUNT_FR	1
#define HUEY_PASSENGER_MIKE		2

#define HUEY_ROCKET_RANGE 4096
#define HUEY_DEFAULT_ROCKET_TOLERANCE 200.0f

#ifndef ATTACK_CHOPPER
ConVar sk_huey_speed("sk_huey_speed", "600.0");
ConVar sk_huey_accel("sk_huey_accel", "50.0");
ConVar sk_huey_min_speed("sk_huey_min_speed", "50.0");
ConVar sk_huey_brake_dist("sk_huey_brake_dist", "500.0");
#endif

static const char *s_ChunkModels[] =
{
	"models/gibs/huey_part_cabin.mdl",
	"models/gibs/huey_part_engine.mdl",
	"models/gibs/huey_part_left_gun.mdl",
	"models/gibs/huey_part_left_strut.mdl",
	"models/gibs/huey_part_roof.mdl",
	"models/gibs/huey_part_tail.mdl",
};

enum HueyChunkID
{
	CHUNK_CABIN = 0,
	CHUNK_ENGINE,
	CHUNK_LEFT_GUN,
	CHUNK_LEFT_STRUT,
	CHUNK_ROOF,
	CHUNK_TAIL,
	CHUNK_MAX
};

// Specifies the chunk that a given chunk is constrained to
static HueyChunkID s_ChunkConstraint[CHUNK_MAX] =
{
	CHUNK_ENGINE, // cabin
	CHUNK_MAX, // engine
	CHUNK_ENGINE, // left gun
	CHUNK_CABIN, // left strut
	CHUNK_CABIN, // roof
	CHUNK_ENGINE // tail
};

// Specifies the time after the huey dies that a given chunk will break away
static float s_ChunkTime[CHUNK_MAX] =
{
	1.0, // cabin
	0, // engine
	1.5, // left gun
	2.0, // left strut
	2.0, // roof
	1.5, // tail
};

// Specifies number of flaming chunks that a given chunk will finally break into
static float s_ChunkChunks[CHUNK_MAX] =
{
	4, // cabin
	4, // engine
	2, // left gun
	1, // left strut
	2, // roof
	4, // tail
};

//-----------------------------------------------------------------------------
class CHueyChunk : public CBaseAnimating
{
	DECLARE_DATADESC();

public:
	DECLARE_CLASS( CHueyChunk, CBaseAnimating );

	virtual void Spawn( void );
	virtual void Precache( void );
	virtual void VPhysicsCollision( int index, gamevcollisionevent_t *pEvent );

	static CHueyChunk *CreateHelicopterChunk( CBaseEntity *pHuey, const Vector &vecPos, const QAngle &vecAngles, const Vector &vecVelocity, HueyChunkID chunkID );

	int		m_nChunkID;
	
	CHandle<CHueyChunk>	m_hMaster;
	IPhysicsConstraint	*m_pConstraints[CHUNK_MAX];

protected:

	void	CollisionCallback( CHueyChunk *pCaller );

	void	FallThink( void );
	void	SparkThink( void );
	void	ExplodeThink( void );

	bool	m_bLanded;
	float	m_flStartTime;
};

//-----------------------------------------------------------------------------
class CNPC_Huey : public CBaseHelicopter
{
public:
	DECLARE_CLASS( CNPC_Huey, CBaseHelicopter );
	DECLARE_DATADESC();
	DEFINE_CUSTOM_AI;

	void Spawn( void );
	void Precache( void );
	void InitializeRotorSound( void );
	float GetRotorVolume( void );
	void InputSetRotorVolume( inputdata_t &inputdata );

	Class_T Classify ( void ) { if (m_iPassenger > HUEY_PASSENGER_GRUNT) return CLASS_HUEY_FRIEND; else return CLASS_HUEY; }
	bool ClassifyPlayerAlly( void ) { return (m_iPassenger > HUEY_PASSENGER_GRUNT); }
	bool ClassifyPlayerAllyVital( void ) { return false; }

	Vector BodyTarget( const Vector &posSrc, bool bNoisy );

	void InitPassenger( void );
	int IRelationPriority( CBaseEntity *pTarget );
	void UpdatePassengerEnemy( void );
	CBaseEntity *BestPassengerEnemy( void );
	bool PosePassengerTowardTargetDirection( const Vector &vTargetDir );
	void FirePassengerWeapons( void );

	void UpdateFacingDirection( void );
	void Hunt( void );
	void Flight( void );

#define ATTACK_CHOPPER
#ifdef ATTACK_CHOPPER
	void PrescheduleThink( void );
	void GetMaxSpeedAndAccel( float *pMaxSpeed, float *pAccelRate );
	void ComputeAngularVelocity( const Vector &vecGoalUp, const Vector &vecFacingDirection );
	void ComputeVelocity( const Vector &deltaPos, float flAdditionalHeight, float flMinDistFromSegment, float flMaxDistFromSegment, Vector *pVecAccel );
	float UpdatePerpPathDistance( float flMaxPathOffset );

	QAngle		m_vecAngAcceleration;
	float		m_flCurrPathOffset;

	// Fun damage effects
	float		m_flGoalRollDmg;
	float		m_flGoalYawDmg;
#endif

	void InputSpeedHack( inputdata_t &inputdata ) { m_flMaxSpeed = inputdata.value.Float(); } /* FIXME: namd0 hack */

	bool FireGun( void );
	bool DoGunFiring( const Vector &vBasePos, const Vector &vGunDir, const Vector &vecFireAtPosition );
	void ShootAtFacingDirection( const Vector &vBasePos, const Vector &vGunDir, bool bFirstShotAccurate );
	void ComputeActualTargetPosition( float flSpeed, float flTime, float flPerpDist, Vector *pDest, bool bApplyNoise );
	void ComputeFireAtPosition( Vector *pVecActualTargetPosition );
	bool PoseGunTowardTargetDirection( const Vector &vTargetDir );

	void AimRocketGun( void );
	void FireRocket( Vector vLaunchPos, Vector vLaunchDir );
	bool CheckRocketImpactZone( Vector vImpactPos );

	void FireGrenade( void );
	void InputGrenadeOn( inputdata_t &inputdata );
	void InputGrenadeOff( inputdata_t &inputdata );

	void InputFireRockets( inputdata_t &inputdata );
	void InputSetRocketTolerance( inputdata_t &inputdata );
	void InputAllowDamageMovement( inputdata_t &inputdata );
	void InputMinimalDamageMovement( inputdata_t &inputdata );

	void InputDeploy( inputdata_t &inputdata );
	bool IsNearDeployPosition( float flDist )
	{
		/*float flSpeed = GetAbsVelocity().Length();
		if (flSpeed > 10)
			return false;*/
		return (WorldSpaceCenter() - m_vecDeployPosition).Length() < flDist;
	}
#ifdef LOGIC_HUEY_DEPLOY
	CAI_BaseNPC *SpawnGrunt( CLogicHueyDeploy *pDeploy, const char *szAttachment0, const char *szAttachment1, Activity iDeployActivity );
#else
	CAI_BaseNPC *SpawnGrunt( const Vector &vecOrigin );
#endif
	void DeathNotice( CBaseEntity *pVictim );
	void DetachRappellers( void );

	void TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr );
	int OnTakeDamage_Alive( const CTakeDamageInfo &info );
	void Event_Killed( const CTakeDamageInfo &info );
	void BecomeChunks( void );

	void DyingThink( void );
	void CrashTouch( CBaseEntity *pOther );
	void StartTask( const Task_t *pTask );
	void RunTask( const Task_t *pTask );

	void StartSmoking( void );

	int DrawDebugTextOverlays(void);

	float		m_flNextRocket;
	float		m_flNextGrenade;
	int			m_iAmmoType;
#ifndef LOGIC_HUEY_DEPLOY
	int			m_iMaxDeploy; // max number of grunts to deploy at one time; 0-4
	int			m_iMaxDeployed; // keep deploying to maintain this number of grunts on the ground; 0-MAX_GRUNTS
#endif
	float		m_flGracePeriod; // wait time before shooting a new enemy
	Vector		m_angMiniGuns;
	Vector		m_angPassenger;
	Vector		m_angGren;
	float		m_flRocketTolerance; // How close a rocket must strike the enemy
	bool		m_bAllowDamageMovement;
	bool		m_bMinimalDamageMovement;
	bool		m_bDeploy;
	bool		m_bHasReachedTarget;
	Vector		m_vecDeployFaceDir;
	Vector		m_vecDeployPosition;
	EHANDLE		m_hDeployTrack; // bloody heck
	EHANDLE		m_hRappel[4]; // currently-deploying grunts
#ifndef LOGIC_HUEY_DEPLOY
#define MAX_GRUNTS 11
	EHANDLE		m_hDeployed[MAX_GRUNTS]; // grunts on the ground
#endif
	int			m_iBeamRingEffectIndex;
	int			m_iFireballSprite;
	int			m_iSmokeSprite;
	int			m_iBodyGibs;
	float		m_flTimeToDie;
	Vector		m_vecVelocityLastThink;
	float		m_flRotorVolume;

	int					m_iPassenger;
	float				m_flPassengerHealth;
	EHANDLE				m_hPassengerEnemy;
	bool				m_bChoosingPassengerEnemy;
	CAI_ShotRegulator	m_PassengerShotRegulator;

	CHandle<SmokeTrail> m_hSmoke;

//	float m_flPrevSpeed;

	enum DeployState_t {
		DEPLOY_STATE_NONE,
		DEPLOY_STATE_MOVE_TO_PATHTRACK,
		DEPLOY_STATE_ORIENT_WITH_PATHTRACK,
		DEPLOY_STATE_DEPLOY,
		DEPLOY_STATE_WAIT_FOR_GRUNTS
	};

	DeployState_t m_eDeployState;

	void SetDeployState( DeployState_t state) { m_eDeployState = state; }
	DeployState_t GetDeployState( void ) const { return m_eDeployState; }

#ifdef LOGIC_HUEY_DEPLOY
	void Activate( void );
	void FindDeployLogics( void );
	EHANDLE m_hDeployLogic; // for current deployment
	CUtlVector<EHANDLE> m_DeployLogics;
#endif

#ifdef RAPPEL_PHYSICS
	void CreateRappelPhysics( int slot, CAI_BaseNPC *pNPC );
	IPhysicsSpring		*m_pRappelPhysics[4]; // spring connecting huey to grunt
#endif

	EHANDLE m_hPilot;
	EHANDLE m_hCopilot;

	CBaseEntity *FindCrashPoint( void );
	CBaseEntity *GetCrashPoint( void ) { return m_hCrashPoint.Get(); }
	EHANDLE m_hCrashPoint;
	float m_flCrashTime;
	static Activity ACT_HUEY_CRASHING;

	void InputReportAtRest( inputdata_t &inputdata );
	COutputEvent m_OnAtRest;
	bool m_bReportAtRest;

	void InputAllowNoise( inputdata_t &inputdata );
	bool m_bAllowNoise; // Whether slop in destination is allowed

	// Attachments
	int m_iAttachmentGrenadeBase;
	int m_iAttachmentGrenadeMuzzle;
	int m_iAttachmentLeftGunBase;
	int m_iAttachmentLeftGunMuzzle;
	int m_iAttachmentRightGunBase;
	int m_iAttachmentRightGunMuzzle;
	int m_iAttachmentPassengerGunBase;
	int m_iAttachmentPassengerGunMuzzle;
	int m_iAttachmentLeftMissile;
	int m_iAttachmentRightMissile;
};

LINK_ENTITY_TO_CLASS( npc_huey, CNPC_Huey );

BEGIN_DATADESC( CNPC_Huey )
	DEFINE_KEYFIELD( m_flGracePeriod,	FIELD_FLOAT, "GracePeriod" ),
#ifndef LOGIC_HUEY_DEPLOY
	DEFINE_KEYFIELD( m_iMaxDeploy,		FIELD_INTEGER, "maxgrunts" ),
	DEFINE_KEYFIELD( m_iMaxDeployed,	FIELD_INTEGER, "maxdeployed" ),
#endif

	DEFINE_FIELD( m_flRocketTolerance,	FIELD_FLOAT ),
	DEFINE_FIELD( m_flNextRocket,		FIELD_TIME ),
	DEFINE_FIELD( m_flNextGrenade,		FIELD_TIME ),
	DEFINE_FIELD( m_iAmmoType,			FIELD_INTEGER ),
	DEFINE_FIELD( m_angMiniGuns,		FIELD_VECTOR ),
	DEFINE_FIELD( m_angPassenger,		FIELD_VECTOR ),
	DEFINE_FIELD( m_bAllowDamageMovement,	FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bMinimalDamageMovement,	FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bDeploy,			FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bHasReachedTarget,	FIELD_BOOLEAN ),
	DEFINE_FIELD( m_vecDeployFaceDir,	FIELD_VECTOR ),
	DEFINE_FIELD( m_vecDeployPosition,	FIELD_POSITION_VECTOR ),
	DEFINE_ARRAY( m_hRappel,			FIELD_EHANDLE, 4 ),
	DEFINE_FIELD( m_hDeployTrack,		FIELD_EHANDLE ),
	DEFINE_FIELD( m_eDeployState,		FIELD_INTEGER ),
	DEFINE_FIELD( m_flTimeToDie,		FIELD_TIME ),
	DEFINE_FIELD( m_vecVelocityLastThink,	FIELD_VECTOR ),
	DEFINE_FIELD( m_flRotorVolume,		FIELD_FLOAT ),

	DEFINE_KEYFIELD( m_iPassenger,		FIELD_INTEGER, "passenger" ),
	DEFINE_FIELD( m_flPassengerHealth,	FIELD_FLOAT ),
	DEFINE_EMBEDDED( m_PassengerShotRegulator ),

	DEFINE_FIELD( m_hSmoke,	FIELD_EHANDLE ),

#ifdef LOGIC_HUEY_DEPLOY
	DEFINE_FIELD( m_hDeployLogic,		FIELD_EHANDLE ),
//	DEFINE_UTLVECTOR( m_DeployLogics,	FIELD_EHANDLE ),
#endif

	DEFINE_FIELD( m_hPilot,				FIELD_EHANDLE ),
	DEFINE_FIELD( m_hCopilot,			FIELD_EHANDLE ),
	DEFINE_FIELD( m_hCrashPoint,		FIELD_EHANDLE ),

	DEFINE_FIELD( m_bReportAtRest,		FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bAllowNoise,		FIELD_BOOLEAN ),

	DEFINE_INPUTFUNC( FIELD_EHANDLE, "Deploy", InputDeploy ),
	DEFINE_INPUTFUNC( FIELD_FLOAT, "SpeedHack", InputSpeedHack ),
	DEFINE_INPUTFUNC( FIELD_VOID, "GrenadeOn", InputGrenadeOn ),
	DEFINE_INPUTFUNC( FIELD_VOID, "GrenadeOff", InputGrenadeOff ),
	DEFINE_INPUTFUNC( FIELD_VOID, "FireRockets", InputFireRockets ),
	DEFINE_INPUTFUNC( FIELD_FLOAT, "SetRocketTolerance", InputSetRocketTolerance ),
	DEFINE_INPUTFUNC( FIELD_BOOLEAN, "AllowDamageMovement", InputAllowDamageMovement ),
	DEFINE_INPUTFUNC( FIELD_BOOLEAN, "MinimalDamageMovement", InputMinimalDamageMovement ),
	DEFINE_INPUTFUNC( FIELD_FLOAT, "SetRotorVolume", InputSetRotorVolume ),
	DEFINE_INPUTFUNC( FIELD_BOOLEAN, "ReportAtRest", InputReportAtRest ),
	DEFINE_INPUTFUNC( FIELD_BOOLEAN, "AllowNoise", InputAllowNoise ),

	DEFINE_OUTPUT( m_OnAtRest, "OnAtRest" ),
END_DATADESC()

Activity CNPC_Huey::ACT_HUEY_CRASHING;

#define HUEY_MODEL_NAME "models/huey/huey.mdl"
#define HUEY_HEALTH 300 // 200, 300, 400
#define HUEY_ROUNDS_PER_BURST 1

//-----------------------------------------------------------------------------
// Purpose :
//-----------------------------------------------------------------------------
void CNPC_Huey::Spawn( void )
{
	Precache( );
	SetModel( STRING( GetModelName() ) );

	ExtractBbox( SelectHeaviestSequence( ACT_IDLE ), m_cullBoxMins, m_cullBoxMaxs ); 

	m_flRotorVolume = 1.0;

	BaseClass::Spawn();

	SetHullType( HULL_LARGE_CENTERED );
	SetHullSizeNormal();

#ifdef RAPPEL_PHYSICS
	// remove cruft from cbasehelicopter
//	SetSolid( SOLID_VPHYSICS );
//	CollisionProp()->SetSurroundingBoundsType( USE_OBB_COLLISION_BOUNDS );
//	RemoveSolidFlags( FSOLID_CUSTOMRAYTEST | FSOLID_CUSTOMBOXTEST );

	CreateVPhysics();  //need a bonefollower manager?
#endif

	SetPauseState( PAUSE_NO_PAUSE );

	m_iMaxHealth = m_iHealth = HUEY_HEALTH;
	
	m_flFieldOfView = -1.0; // 360 degrees

	m_fHelicopterFlags |= BITS_HELICOPTER_GUN_ON;

	if ( HasSpawnFlags( SF_ROCKET ) )
		m_fHelicopterFlags |= BITS_HELICOPTER_MISSILE_ON;

	if ( HasSpawnFlags( SF_GRENADE ) )
		SetBodygroup( BODY_GRENADE, 1 );

	m_flMaxSpeed = sk_huey_speed.GetFloat();

	m_iAmmoType = GetAmmoDef()->Index("12mm");
	m_flGracePeriod = 0.5f; // override
	m_flRocketTolerance = HUEY_DEFAULT_ROCKET_TOLERANCE;
	m_bDeploy = false;
	m_bHasReachedTarget = false;
	m_eDeployState = DEPLOY_STATE_NONE;
	m_hRappel[0] = m_hRappel[1] = m_hRappel[2] = m_hRappel[3] = 0;
	m_bAllowNoise = true;

	m_hCrashPoint.Set( NULL );

#ifndef LOGIC_HUEY_DEPLOY
	m_iMaxDeploy = clamp(m_iMaxDeploy, 0, 4);
	m_iMaxDeployed = clamp(m_iMaxDeployed, 0, MAX_GRUNTS);
#endif

	InitPassenger();

	SetActivity( ACT_IDLE );

	variant_t emptyVariant;
	AcceptInput( "startpatrol", this, this, emptyVariant, USE_ON );

#ifndef ATTACK_CHOPPER
	// HL1 sv_gravity was 800, in HL2 it is 600
//	SetGravity( 384.0 * 600.0 / 800.0 );
	SetGravity(0.0);
#endif
}

//------------------------------------------------------------------------------
// Purpose :
//------------------------------------------------------------------------------
void CNPC_Huey::Precache( void )
{
	SetModelName( AllocPooledString( HUEY_MODEL_NAME ) );
	BaseClass::Precache();

	PrecacheModel( STRING( GetModelName() ) );

	UTIL_PrecacheOther( "npc_mikeforce" );
	UTIL_PrecacheOther( "npc_mikeforce_medic" );
	UTIL_PrecacheOther( "npc_human_grunt" );
	UTIL_PrecacheOther( "npc_grunt_medic" );

	UTIL_PrecacheOther( "rpg7_rocket" );
	PrecacheModel( "models/HVR.mdl" );

	PrecacheModel( "models/hpilot.mdl" );
	UTIL_PrecacheOther( "prop_dynamic", "models/hpilot.mdl" );
	PrecacheModel( "models/hcopilot.mdl" );

	UTIL_PrecacheOther( "env_fire_trail" );

	m_iBeamRingEffectIndex = PrecacheModel( "sprites/physbeam.vmt" );
	m_iFireballSprite = PrecacheModel( "sprites/fexplo.spr" );
	m_iSmokeSprite = PrecacheModel("sprites/steam1.spr");
	m_iBodyGibs = PrecacheModel( "models/metalplategibs_green.mdl" );

	PrecacheScriptSound("NPC_Huey.FireGun");
	PrecacheScriptSound("NPC_Huey.Grenade");
	PrecacheScriptSound("NPC_Huey.Rotors");
	PrecacheScriptSound("NPC_Huey.Explode");
	PrecacheScriptSound("NPC_Huey.PassengerFireGun");

	UTIL_PrecacheOther( "huey_chunk" );

#ifdef RAPPEL_PHYSICS
	UTIL_PrecacheOther( "huey_rappel_physics_tip" );
#endif
}

#ifdef LOGIC_HUEY_DEPLOY
//------------------------------------------------------------------------------
void CNPC_Huey::Activate( void )
{
	BaseClass::Activate();

	m_iAttachmentGrenadeBase = LookupAttachment( "grenadebase" );
	m_iAttachmentGrenadeMuzzle = LookupAttachment( "grenademuzzle" );
	m_iAttachmentLeftGunBase = LookupAttachment( "leftgunbase" );
	m_iAttachmentLeftGunMuzzle = LookupAttachment( "leftgunmuzzle" );
	m_iAttachmentRightGunBase = LookupAttachment( "rightgunbase" );
	m_iAttachmentRightGunMuzzle = LookupAttachment( "rightgunmuzzle" );
	m_iAttachmentPassengerGunBase = LookupAttachment( "pgunbase" );
	m_iAttachmentPassengerGunMuzzle = LookupAttachment( "pgunmuzzle" );
	m_iAttachmentLeftMissile = LookupAttachment( "leftmissile" );
	m_iAttachmentRightMissile = LookupAttachment( "rightmissile" );

	FindDeployLogics();
}

//------------------------------------------------------------------------------
void CNPC_Huey::FindDeployLogics( void )
{
	for ( CBaseEntity *pEnt = gEntList.FindEntityByClassname( NULL, HUEY_DEPLOY_CLASSNAME );
		pEnt != NULL;
		pEnt = gEntList.FindEntityByClassname( pEnt, HUEY_DEPLOY_CLASSNAME ) )
	{
		CLogicHueyDeploy *pDeploy = dynamic_cast<CLogicHueyDeploy *>(pEnt);
		if ( pDeploy && pDeploy->m_iszHueyName == GetEntityName() )
			m_DeployLogics.AddToTail( pDeploy );
	}
}
#endif

//------------------------------------------------------------------------------
// Purpose : Create our rotor sound
//------------------------------------------------------------------------------
void CNPC_Huey::InitializeRotorSound( void )
{
	if ( !m_pRotorSound )
	{
		CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
		CPASAttenuationFilter filter( this );

#if 1 // do it this way so the rotor sound doesn't play when paused
		EmitSound_t es;
		es.m_pSoundName = "NPC_Huey.Rotors";
		es.m_nChannel = CHAN_AUTO;
		es.m_nFlags = SND_SHOULDPAUSE;
		m_pRotorSound = controller.SoundCreate( filter, entindex(), es );
#else
		m_pRotorSound = controller.SoundCreate( filter, entindex(), "NPC_Huey.Rotors" );
#endif
//		m_pRotorBlast = controller.SoundCreate( filter, entindex(), "NPC_Huey.RotorBlast" );
/*
		m_pGunFiringSound = controller.SoundCreate( filter, entindex(), "NPC_AttackHelicopter.FireGun" );
		controller.Play( m_pGunFiringSound, 0.0, 100 );
*/
		}
	else
	{
//		Assert(m_pRotorSound);
//		Assert(m_pRotorBlast);
//		Assert(m_pGunFiringSound);
	}

	BaseClass::InitializeRotorSound();
}

//------------------------------------------------------------------------------
float CNPC_Huey::GetRotorVolume( void )
{
	return m_bSuppressSound ? 0.0f : m_flRotorVolume;
}

//-----------------------------------------------------------------------------
void CNPC_Huey::InputSetRotorVolume( inputdata_t &inputdata )
{
	m_flRotorVolume = clamp( inputdata.value.Float(), 0.0f, 1.0f );
}

//-----------------------------------------------------------------------------
void CNPC_Huey::InputReportAtRest( inputdata_t &inputdata )
{
	m_bReportAtRest = inputdata.value.Bool();
}

//-----------------------------------------------------------------------------
void CNPC_Huey::InputAllowNoise( inputdata_t &inputdata )
{
	m_bAllowNoise = inputdata.value.Bool();
}

//-----------------------------------------------------------------------------
Vector CNPC_Huey::BodyTarget( const Vector &posSrc, bool bNoisy ) 
{ 
	return GetAbsOrigin() - Vector(0,0,100);
}

//------------------------------------------------------------------------------
void CNPC_Huey::InitPassenger( void )
{
	if ( HasSpawnFlags( SF_PASSENGER ) )
	{
		if ( m_iPassenger < HUEY_PASSENGER_MIKE )
		{
			SetBodygroup( BODY_PASSENGER, HUEY_PBODY_GRUNT );
			m_flPassengerHealth = 60 /*gSkillData.hgruntHealth*/;
		}
		else
		{
			SetBodygroup( BODY_PASSENGER, HUEY_PBODY_MIKE );
			m_flPassengerHealth = 80 /*gSkillData.mikeforceHealth*/;
		}

		m_PassengerShotRegulator.SetBurstInterval( 0.1, 0.1 );
		m_PassengerShotRegulator.SetBurstShotCountRange( 5, 10 );
		m_PassengerShotRegulator.SetRestInterval( 0.3, 0.6 );
	}
	else
	{
		SetBodygroup( BODY_PASSENGER, HUEY_PBODY_NONE );
		m_flPassengerHealth = 0;
	}

	// Pilot
	CBaseAnimating *pAnim = (CBaseAnimating*)CreateEntityByName( "prop_dynamic" );
	if ( pAnim )
	{
		pAnim->SetModel( "models/hpilot.mdl" );
//		m_hPilot->SetName( AllocPooledString("Alyx_Emptool") );
		int iAttachment = LookupAttachment( "passenger_pilot" );
		pAnim->SetParent(this, iAttachment);
		pAnim->SetOwnerEntity(this);
		pAnim->SetSolid( SOLID_NONE );
		pAnim->SetLocalOrigin( Vector( 0, 0, 0 ) );
		pAnim->SetLocalAngles( QAngle( 0, 0, 0 ) );
		pAnim->AddSpawnFlags( SF_DYNAMICPROP_NO_VPHYSICS );
		pAnim->AddEffects( EF_PARENT_ANIMATES );
		pAnim->Spawn();
		pAnim->SetSequence( pAnim->LookupSequence( "sit_in_huey" ) );

		m_hPilot = pAnim;
	}

	// Coilot
	pAnim = (CBaseAnimating*)CreateEntityByName( "prop_dynamic" );
	if ( pAnim )
	{
		pAnim->SetModel( "models/hcopilot.mdl" );
//		m_hPilot->SetName( AllocPooledString("Alyx_Emptool") );
		int iAttachment = LookupAttachment( "passenger_copilot" );
		pAnim->SetParent(this, iAttachment);
		pAnim->SetOwnerEntity(this);
		pAnim->SetSolid( SOLID_NONE );
		pAnim->SetLocalOrigin( Vector( 0, 0, 0 ) );
		pAnim->SetLocalAngles( QAngle( 0, 0, 0 ) );
		pAnim->AddSpawnFlags( SF_DYNAMICPROP_NO_VPHYSICS );
		pAnim->AddEffects( EF_PARENT_ANIMATES );
		pAnim->Spawn();
		pAnim->SetSequence( pAnim->LookupSequence( "sit_in_huey" ) );

		m_hCopilot = pAnim;
	}
}

// see monstermaker.cpp
static void DispatchActivate( CBaseEntity *pEntity )
{
	bool bAsyncAnims = mdlcache->SetAsyncLoad( MDLCACHE_ANIMBLOCK, false );
	pEntity->Activate();
	mdlcache->SetAsyncLoad( MDLCACHE_ANIMBLOCK, bAsyncAnims );
}

//------------------------------------------------------------------------------
void CNPC_Huey::UpdateFacingDirection( void )
{
#if 1
	if ( !IsCrashing() && IsOnPathTrack() )
	{
		for ( int i = 0; i < m_DeployLogics.Count(); i++ )
		{
			if ( !m_DeployLogics[i] ) // may have been deleted
				continue;
			CLogicHueyDeploy *pDeploy = assert_cast<CLogicHueyDeploy *>(m_DeployLogics[i].Get());
			if ( pDeploy->m_iszPathTrack == GetCurrentPathTarget()->GetEntityName() )
			{
				if ( pDeploy->GetDeployCounts() &&
					GetCurrentPathTarget()->GetAbsOrigin().DistTo( GetAbsOrigin()) < 256 )
				{
					AngleVectors( GetCurrentPathTarget()->GetAbsAngles(), &m_vecDesiredFaceDir );
					return;
				}
				break;
			}
		}
	}
#endif
	if ( m_bDeploy && GetDeployState() > DEPLOY_STATE_MOVE_TO_PATHTRACK )
	{
		m_vecDesiredFaceDir = m_vecDeployFaceDir;
	}
	else
	{
		Vector targetDir = m_vecTargetPosition - GetAbsOrigin();
		Vector desiredDir = GetDesiredPosition() - GetAbsOrigin();

		VectorNormalize( targetDir ); 
		VectorNormalize( desiredDir ); 

#ifdef HOE_TRACK_SPLINE
		ComputePathTangent( GetAbsOrigin(), desiredDir );
#endif
#if 1
		// Hack for case of solo path_track: face its angles
		if ( GetCurrentPathTarget() != NULL &&
			NextAlongCurrentPath( GetCurrentPathTarget() ) == NULL &&
			PreviousAlongCurrentPath( GetCurrentPathTarget() ) == NULL )
		{
			QAngle angles = GetCurrentPathTarget()->GetLocalAngles();
			AngleVectors( angles, &desiredDir );
		}
#endif
		if ( !IsCrashing() && m_flLastSeen + 5 > gpGlobals->curtime && DotProduct( targetDir, desiredDir) > 0.25 )
		{
			// If we've seen the target recently, face the target.
			//Msg( "Facing Target \n" );
			m_vecDesiredFaceDir = targetDir;
		}
		else
		{
			// Face our desired position.
			// Msg( "Facing Position\n" );
			m_vecDesiredFaceDir = desiredDir;
		}
	}

}

//------------------------------------------------------------------------------
void CNPC_Huey::Hunt( void )
{
	if ( m_lifeState == LIFE_DEAD )
	{
		return;
	}

	if ( m_lifeState == LIFE_DYING )
	{
		if ( GetCrashPoint() != NULL &&
			( GetAbsOrigin().DistTo( GetCrashPoint()->GetAbsOrigin() ) < 50 ||
			(m_flCrashTime && TimePassed( m_flCrashTime )) ) )
		{
			// This is from CNPC_AttackHelicopter::InputSelfDestruct
			m_lifeState = LIFE_ALIVE; // Force to die properly.
			CTakeDamageInfo info( this, this, Vector(0, 0, 1), WorldSpaceCenter(), GetMaxHealth(), CLASS_MISSILE );
			TakeDamage( info );
			return;
		}
//		UpdateFacingDirection();
		Flight();
//		UpdatePlayerDopplerShift( );
		return;
	}

#ifdef RAPPEL_PHYSICS
	if ( m_pRappelPhysics[0] )
	{
		Vector p1, p2;
		m_pRappelPhysics[0]->GetEndpoints( &p1, &p2 );
		NDebugOverlay::Line( p1, p2, 0, 255, 0, true, 0.1 );
	}
#endif

	if (m_bDeploy)
	{
		if (GetDeployState() == DEPLOY_STATE_MOVE_TO_PATHTRACK)
		{
			if ( IsNearDeployPosition( 128.0f ) )
			{
				DbgHueyMsg("DEPLOY_STATE_MOVE_TO_PATHTRACK -> DEPLOY_STATE_ORIENT_WITH_PATHTRACK\n");
				ClearForcedMove();
				SetDeployState(DEPLOY_STATE_ORIENT_WITH_PATHTRACK);

				// Align with the path_track
				m_vecDesiredFaceDir = m_vecDeployFaceDir;
				SetDesiredPosition( m_vecDeployPosition );
			}
			else
			{
				UpdateTrackNavigation();
				UpdateEnemy();
				UpdatePassengerEnemy();
				/*UpdateDesiredPosition();*/ SetDesiredPosition( m_vecDeployPosition );
				UpdateFacingDirection();
				Flight();
				UpdatePlayerDopplerShift();
				FireWeapons();
				FirePassengerWeapons();
				FireGrenade();
			}
		}
		else if (GetDeployState() == DEPLOY_STATE_ORIENT_WITH_PATHTRACK)
		{
			float flTargetYaw = UTIL_VecToYaw( m_vecDeployFaceDir );
			float flDeltaYaw = UTIL_AngleDiff( flTargetYaw, GetAbsAngles().y );
#if 1
			Vector velocity = GetAbsVelocity();
			float flVelocityZ = velocity.z;
			velocity.z = 0;
			float flVelocityXY = velocity.Length();
#else
			float flVelocity = GetAbsVelocity().Length();
#endif
			DbgHueyMsg("DEPLOY_STATE_ORIENT_WITH_PATHTRACK yaw %0.2f velocity xy=%0.2f,z=%0.2f\n", flDeltaYaw, flVelocityXY, flVelocityZ );
			if ( IsNearDeployPosition( 8.0f ) &&
				flDeltaYaw >= -5 && flDeltaYaw < 5 && flVelocityXY < 10 && flVelocityZ < 30 )
			{
				DbgHueyMsg("DEPLOY_STATE_ORIENT_WITH_PATHTRACK -> DEPLOY_STATE_DEPLOY\n");
				SetDeployState(DEPLOY_STATE_DEPLOY);
			}
			else
			{
				UpdateEnemy();
				UpdatePassengerEnemy();
				Flight();
				UpdatePlayerDopplerShift();
				FireWeapons();
				FirePassengerWeapons();
				FireGrenade();
			}
		}
		else if (GetDeployState() == DEPLOY_STATE_DEPLOY)
		{
			Vector forward, right, up;
			AngleVectors( GetAbsAngles(), &forward, &right, &up );

#ifdef LOGIC_HUEY_DEPLOY
			CLogicHueyDeploy *pDeploy = dynamic_cast<CLogicHueyDeploy *>(m_hDeployLogic.Get());
			if ( pDeploy )
			{
				m_hRappel[0] = m_hRappel[1] = m_hRappel[2] = m_hRappel[3] = NULL;
				m_hRappel[0] = SpawnGrunt( pDeploy, "rappel_left", "rappel0", ACT_RAPPEL_JUMP0 );
#ifdef RAPPEL_PHYSICS
				if ( m_hRappel[0] )
					CreateRappelPhysics( 0, m_hRappel[0]->MyNPCPointer() );
#endif
				m_hRappel[1] = SpawnGrunt( pDeploy, "rappel_right", "rappel1", ACT_RAPPEL_JUMP2 );
				m_hRappel[2] = SpawnGrunt( pDeploy, "rappel_left", "rappel2", ACT_RAPPEL_JUMP2 );
				m_hRappel[3] = SpawnGrunt( pDeploy, "rappel_right", "rappel3", ACT_RAPPEL_JUMP0 );
			}
			else
				DevWarning("CNPC_Huey::Hunt CLogicHueyDeploy disappeared\n");
#else
			if ( m_iMaxDeploy > 0 )
				m_hRappel[0] = SpawnGrunt( GetAbsOrigin() + forward * -16 + right * 48 + up * -240 );

			if ( m_iMaxDeploy > 1 )
				m_hRappel[1] = SpawnGrunt( GetAbsOrigin() + forward * 96 + right * 48 + up * -240 );

			if ( m_iMaxDeploy > 2 )
				m_hRappel[2] = SpawnGrunt( GetAbsOrigin() + forward * -16 + right * -36 + up * -240 );

			if ( m_iMaxDeploy > 3 )
				m_hRappel[3] = SpawnGrunt( GetAbsOrigin() + forward * 96 + right * -36 + up * -240 );
#endif
			// TODO: scare AIs away from landing location

			DbgHueyMsg("DEPLOY_STATE_DEPLOY -> DEPLOY_STATE_WAIT_FOR_GRUNTS\n");
			SetDeployState(DEPLOY_STATE_WAIT_FOR_GRUNTS);
		}
		else if ( GetDeployState() == DEPLOY_STATE_WAIT_FOR_GRUNTS )
		{
			bool waiting = false;
			for ( int i = 0; i < 4; i++ )
			{
				if ( m_hRappel[i] && !(m_hRappel[i]->GetFlags() & FL_ONGROUND) )
				{
#ifdef RAPPEL_PHYSICS
					if ( m_pRappelPhysics[i] )
					{
						Vector p1, p2;
						m_pRappelPhysics[i]->GetEndpoints( &p1, &p2 );
						NDebugOverlay::Line( p1, p2, 0, 255, 0, true, 0.1 );
						m_pRappelPhysics[i]->SetSpringLength( (p2 - p1).Length() + 10 );
					}
#endif
					waiting = true;
					break;
				}
			}
			if ( !waiting )
			{
				DbgHueyMsg("DEPLOY_STATE_WAIT_FOR_GRUNTS done\n");
				CPathTrack *pTrack = dynamic_cast<CPathTrack *>(m_hDeployTrack.Get());
				Assert( pTrack != NULL );
				Assert( pTrack->GetNext() != NULL );
				SetPauseState( PAUSE_NO_PAUSE );
				SetDesiredPosition( pTrack->GetNext()->GetAbsOrigin() );
				m_hDeployTrack = NULL;
				m_bDeploy = false;
			}
			else
			{
				UpdateEnemy();
				UpdatePassengerEnemy();
				Flight();
				UpdatePlayerDopplerShift();
				UpdatePassengerEnemy();
				// No shooting guns unless we check for not hitting deploying grunts
				FirePassengerWeapons();
				FireGrenade();
			}
		}
		else
		{
			AssertMsg(0, "unhandled deploy state\n");
			m_bDeploy = false;
		}
		return;
	}

	UpdateTrackNavigation();
	UpdateEnemy();
	UpdatePassengerEnemy();
	UpdateDesiredPosition();
	UpdateFacingDirection();
	Flight();
	UpdatePlayerDopplerShift();
	FireWeapons();
	FirePassengerWeapons();
	FireGrenade();

	if ( m_bReportAtRest )
	{
		float flTargetYaw = UTIL_VecToYaw( m_vecDesiredFaceDir );
		float flDeltaYaw = UTIL_AngleDiff( flTargetYaw, GetAbsAngles().y );
		float flVelocity = GetAbsVelocity().Length();
		if ( GetAbsOrigin().DistTo( GetDesiredPosition() ) < 8 &&
			flDeltaYaw >= -5 && flDeltaYaw < 5
			&& flVelocity < 10 )
		{
			m_bReportAtRest = false;
			m_OnAtRest.FireOutput( this, this );
		}
	}
}

#ifdef ATTACK_CHOPPER
//-----------------------------------------------------------------------------
void CNPC_Huey::PrescheduleThink( void )
{
	if ( m_flGoalRollDmg != 0.0f )
	{
		m_flGoalRollDmg = UTIL_Approach( 0, m_flGoalRollDmg, 2.0f );
	}

	if ( m_lifeState == LIFE_DYING )
	{
		if ( GetCrashPoint() != NULL )
		{
			// Stay on this, no matter what.
			SetDesiredPosition( GetCrashPoint()->WorldSpaceCenter() );
		}
#if 0
		if ( random->RandomInt( 0, 4 ) == 0 )
		{
			Vector	explodePoint;		
			CollisionProp()->RandomPointInBounds( Vector(0.25,0.25,0.25), Vector(0.75,0.75,0.75), &explodePoint );
			
			ExplodeAndThrowChunk( explodePoint );
		}
#endif
	}

	BaseClass::PrescheduleThink();
}

//------------------------------------------------------------------------------
void CNPC_Huey::Flight( void )
{
	if( GetFlags() & FL_ONGROUND )
	{
		// This would be really bad.
		SetGroundEntity( NULL );
	}

	// Determine the distances we must lie from the path
	float flMaxPathOffset = MaxDistanceFromCurrentPath();
	float flPerpDist = UpdatePerpPathDistance( flMaxPathOffset );

	float flMinDistFromSegment, flMaxDistFromSegment;
	if ( !IsLeading() )
	{
		flMinDistFromSegment = 0.0f;
		flMaxDistFromSegment = 0.0f;
	}
	else
	{
		flMinDistFromSegment = fabs(flPerpDist) + 100.0f;
		flMaxDistFromSegment = fabs(flPerpDist) + 200.0f;
		if ( flMaxPathOffset != 0.0 )
		{
			if ( flMaxDistFromSegment > flMaxPathOffset - 100.0f )
			{
				flMaxDistFromSegment = flMaxPathOffset - 100.0f;
			}

			if ( flMinDistFromSegment > flMaxPathOffset - 200.0f )
			{
				flMinDistFromSegment = flMaxPathOffset - 200.0f;
			}
		}
	}

	float maxSpeed, accelRate;
	GetMaxSpeedAndAccel( &maxSpeed, &accelRate );

	Vector vecTargetPosition;
	float flCurrentSpeed = GetAbsVelocity().Length();
	float flDist = min( flCurrentSpeed + accelRate, maxSpeed );
	float dt = 1.0f;
	bool bNoise = !m_bDeploy; // no noise while deploying
	ComputeActualTargetPosition( flDist, dt, flPerpDist, &vecTargetPosition, bNoise );
	// Raise high in the air when doing the shooting attack
	float flAdditionalHeight = 0.0f;
#if 0
	if ( m_nAttackMode == ATTACK_MODE_BULLRUSH_VEHICLE )
	{
		flAdditionalHeight = clamp( m_flBullrushAdditionalHeight, 0.0f, flMaxPathOffset );
		vecTargetPosition.z += flAdditionalHeight;
	}
#endif
	Vector accel;
#if 0
	UpdateFacingDirection( vecTargetPosition );
#endif
	ComputeVelocity( vecTargetPosition, flAdditionalHeight, flMinDistFromSegment, flMaxDistFromSegment, &accel );
	ComputeAngularVelocity( accel, m_vecDesiredFaceDir );
}

//-----------------------------------------------------------------------------
// Computes the max speed + acceleration:	
//-----------------------------------------------------------------------------
#define CHOPPER_ACCEL_RATE			500
void CNPC_Huey::GetMaxSpeedAndAccel( float *pMaxSpeed, float *pAccelRate )
{
	if ( m_lifeState == LIFE_DYING && GetCrashPoint() != NULL )
	{
		*pAccelRate = CHOPPER_ACCEL_RATE * 6.0f;
		*pMaxSpeed = 400.0f;
	}
	else
	{
		*pAccelRate = CHOPPER_ACCEL_RATE;
		*pMaxSpeed = GetMaxSpeed();
	}
#if 0
	if ( GetEnemyVehicle() )
	{
		*pAccelRate *= 9.0f;
	}
#endif
}

//-----------------------------------------------------------------------------
// Computes the acceleration:	
//-----------------------------------------------------------------------------
#define HELICOPTER_GRAVITY	384
#define HELICOPTER_DT		0.1f
#define HELICOPTER_MIN_DZ_DAMP	-500.0f
#define HELICOPTER_MAX_DZ_DAMP	-1000.0f
#define HELICOPTER_FORCE_BLEND 0.8f
#define HELICOPTER_FORCE_BLEND_VEHICLE 0.2f

void CNPC_Huey::ComputeVelocity( const Vector &vecTargetPosition, 
	float flAdditionalHeight, float flMinDistFromSegment, float flMaxDistFromSegment, Vector *pVecAccel )
{
	Vector deltaPos;
	VectorSubtract( vecTargetPosition, GetAbsOrigin(), deltaPos ); 

	// calc goal linear accel to hit deltaPos in dt time.
	// This is solving the equation xf = 0.5 * a * dt^2 + vo * dt + xo
	float dt = 1.0f;
	pVecAccel->x = 2.0f * (deltaPos.x - GetAbsVelocity().x * dt) / (dt * dt);
	pVecAccel->y = 2.0f * (deltaPos.y - GetAbsVelocity().y * dt) / (dt * dt);
	pVecAccel->z = 2.0f * (deltaPos.z - GetAbsVelocity().z * dt) / (dt * dt) + HELICOPTER_GRAVITY;

	float flDistFromPath = 0.0f;
	Vector vecPoint, vecDelta;
	if ( flMaxDistFromSegment != 0.0f )
	{
		// Also, add in a little force to get us closer to our current line segment if we can
		ClosestPointToCurrentPath( &vecPoint );

		if ( flAdditionalHeight != 0.0f )
		{
			Vector vecEndPoint, vecClosest;
			vecEndPoint = vecPoint;
			vecEndPoint.z += flAdditionalHeight;
			CalcClosestPointOnLineSegment( GetAbsOrigin(), vecPoint, vecEndPoint, vecClosest );
			vecPoint = vecClosest;
		}

		VectorSubtract( vecPoint, GetAbsOrigin(), vecDelta );
 		flDistFromPath = VectorNormalize( vecDelta );
		if ( flDistFromPath > flMaxDistFromSegment )
		{
			// Strongly constrain to an n unit pipe around the current path
			// by damping out all impulse forces that would push us further from the pipe
			float flAmount = (flDistFromPath - flMaxDistFromSegment) / 200.0f;
			flAmount = clamp( flAmount, 0, 1 );
			VectorMA( *pVecAccel, flAmount * 200.0f, vecDelta, *pVecAccel );
		}
	}

	// Apply avoidance forces
	if ( 1/*!HasSpawnFlags( SF_HELICOPTER_IGNORE_AVOID_FORCES )*/ )
	{
		Vector vecAvoidForce;
		CAvoidSphere::ComputeAvoidanceForces( this, 350.0f, 2.0f, &vecAvoidForce );
		*pVecAccel += vecAvoidForce;
		CAvoidBox::ComputeAvoidanceForces( this, 350.0f, 2.0f, &vecAvoidForce );
		*pVecAccel += vecAvoidForce;
	}

	// don't fall faster than 0.2G or climb faster than 2G
	pVecAccel->z = clamp( pVecAccel->z, HELICOPTER_GRAVITY * 0.2f, HELICOPTER_GRAVITY * 2.0f );

	// The lift factor owing to horizontal movement
	float flHorizLiftFactor = fabs( pVecAccel->x ) * 0.10f + fabs( pVecAccel->y ) * 0.10f;

	// If we're way above the path, dampen horizontal lift factor
	float flNewHorizLiftFactor = clamp( deltaPos.z, HELICOPTER_MAX_DZ_DAMP, HELICOPTER_MIN_DZ_DAMP );
	flNewHorizLiftFactor = SimpleSplineRemapVal( flNewHorizLiftFactor, HELICOPTER_MIN_DZ_DAMP, HELICOPTER_MAX_DZ_DAMP, flHorizLiftFactor, 2.5f * (HELICOPTER_GRAVITY * 0.2) );
	float flDampening = (flNewHorizLiftFactor != 0.0f) ? (flNewHorizLiftFactor / flHorizLiftFactor) : 1.0f;
	if ( flDampening < 1.0f )
	{
		pVecAccel->x *= flDampening;
		pVecAccel->y *= flDampening;
		flHorizLiftFactor = flNewHorizLiftFactor;
	}

	Vector forward, right, up;
	GetVectors( &forward, &right, &up );

	// First, attenuate the current force
#if 1
	float flForceBlend = HELICOPTER_FORCE_BLEND;
#else
	float flForceBlend = GetEnemyVehicle() ? HELICOPTER_FORCE_BLEND_VEHICLE : HELICOPTER_FORCE_BLEND;
#endif
	m_flForce *= flForceBlend;

	// Now add force based on our acceleration factors
	m_flForce += ( pVecAccel->z + flHorizLiftFactor ) * HELICOPTER_DT * (1.0f - flForceBlend);

	// The force is always *locally* upward based; we pitch + roll the chopper to get movement
	Vector vecImpulse;
	VectorMultiply( up, m_flForce, vecImpulse );
	
	// NOTE: These have to be done *before* the additional path distance drag forces are applied below
	ApplySidewaysDrag( right );
	ApplyGeneralDrag();

	// If LIFE_DYING, maintain control as long as we're flying to a crash point.
	if ( m_lifeState != LIFE_DYING || (m_lifeState == LIFE_DYING && GetCrashPoint() != NULL) )
	{
		vecImpulse.z += -HELICOPTER_GRAVITY * HELICOPTER_DT;

		if ( flMinDistFromSegment != 0.0f && ( flDistFromPath > flMinDistFromSegment ) )
		{
			Vector	vecVelDir = GetAbsVelocity();

			// Strongly constrain to an n unit pipe around the current path
			// by damping out all impulse forces that would push us further from the pipe
			float flDot = DotProduct( vecImpulse, vecDelta );
			if ( flDot < 0.0f )
			{
				VectorMA( vecImpulse, -flDot * 0.1f, vecDelta, vecImpulse );
			}

			// Also apply an extra impulse to compensate for the current velocity
			flDot = DotProduct( vecVelDir, vecDelta );
			if ( flDot < 0.0f )
			{
				VectorMA( vecImpulse, -flDot * 0.1f, vecDelta, vecImpulse );
			}
		}
	}
	else
	{
		// No more upward lift...
		vecImpulse.z = -HELICOPTER_GRAVITY * HELICOPTER_DT;

		// Damp the horizontal impulses; we should pretty much be falling ballistically
		vecImpulse.x *= 0.1f;
		vecImpulse.y *= 0.1f;
	}

	// Add in our velocity pulse for this frame
	ApplyAbsVelocityImpulse( vecImpulse );
}

//-----------------------------------------------------------------------------
// Computes the max speed + acceleration:	
//-----------------------------------------------------------------------------
void CNPC_Huey::ComputeAngularVelocity( const Vector &vecGoalUp, const Vector &vecFacingDirection )
{
	QAngle goalAngAccel;
	if ( m_lifeState != LIFE_DYING || (m_lifeState == LIFE_DYING && GetCrashPoint() != NULL) )
	{
		Vector forward, right, up;
		GetVectors( &forward, &right, &up );

		Vector goalUp = vecGoalUp;
		VectorNormalize( goalUp );

		// calc goal orientation to hit linear accel forces
		float goalPitch = RAD2DEG( asin( DotProduct( forward, goalUp ) ) );
		float goalYaw = UTIL_VecToYaw( vecFacingDirection );
		float goalRoll = RAD2DEG( asin( DotProduct( right, goalUp ) ) + m_flGoalRollDmg );
		goalPitch *= 0.75f;

		// clamp goal orientations
		goalPitch = clamp( goalPitch, -30, 45 );
		goalRoll = clamp( goalRoll, -45, 45 );

		// calc angular accel needed to hit goal pitch in dt time.
		float dt = 0.6;
		goalAngAccel.x = 2.0 * (AngleDiff( goalPitch, AngleNormalize( GetAbsAngles().x ) ) - GetLocalAngularVelocity().x * dt) / (dt * dt);
		goalAngAccel.y = 2.0 * (AngleDiff( goalYaw, AngleNormalize( GetAbsAngles().y ) ) - GetLocalAngularVelocity().y * dt) / (dt * dt);
		goalAngAccel.z = 2.0 * (AngleDiff( goalRoll, AngleNormalize( GetAbsAngles().z ) ) - GetLocalAngularVelocity().z * dt) / (dt * dt);

		goalAngAccel.x = clamp( goalAngAccel.x, -300, 300 );
		//goalAngAccel.y = clamp( goalAngAccel.y, -60, 60 );
		goalAngAccel.y = clamp( goalAngAccel.y, -120, 120 );
		goalAngAccel.z = clamp( goalAngAccel.z, -300, 300 );
	}
	else
	{
		goalAngAccel.x	= 0;
		goalAngAccel.y = random->RandomFloat( 50, 120 );
		goalAngAccel.z	= 0;
	}

	// limit angular accel changes to similate mechanical response times
	QAngle angAccelAccel;
	float dt = 0.1;
	angAccelAccel.x = (goalAngAccel.x - m_vecAngAcceleration.x) / dt;
	angAccelAccel.y = (goalAngAccel.y - m_vecAngAcceleration.y) / dt;
	angAccelAccel.z = (goalAngAccel.z - m_vecAngAcceleration.z) / dt;

	angAccelAccel.x = clamp( angAccelAccel.x, -1000, 1000 );
	angAccelAccel.y = clamp( angAccelAccel.y, -1000, 1000 );
	angAccelAccel.z = clamp( angAccelAccel.z, -1000, 1000 );

	// DevMsg( "pitch %6.1f (%6.1f:%6.1f)  ", goalPitch, GetLocalAngles().x, m_vecAngVelocity.x );
	// DevMsg( "roll %6.1f (%6.1f:%6.1f) : ", goalRoll, GetLocalAngles().z, m_vecAngVelocity.z );
	// DevMsg( "%6.1f %6.1f %6.1f  :  ", goalAngAccel.x, goalAngAccel.y, goalAngAccel.z );
	// DevMsg( "%6.0f %6.0f %6.0f\n", angAccelAccel.x, angAccelAccel.y, angAccelAccel.z );

	m_vecAngAcceleration += angAccelAccel * 0.1;

	QAngle angVel = GetLocalAngularVelocity();
	angVel += m_vecAngAcceleration * 0.1;
	angVel.y = clamp( angVel.y, -120, 120 );

	// Fix up pitch and yaw to tend toward small values
	if ( m_lifeState == LIFE_DYING && GetCrashPoint() == NULL )
	{
		float flPitchDiff = random->RandomFloat( -5, 5 ) - GetAbsAngles().x;
		angVel.x = flPitchDiff * 0.1f;
		float flRollDiff = random->RandomFloat( -5, 5 ) - GetAbsAngles().z;
		angVel.z = flRollDiff * 0.1f;
	}

	SetLocalAngularVelocity( angVel );
#if 0
	float flAmt = clamp( angVel.y, -30, 30 ); 
	float flRudderPose = RemapVal( flAmt, -30, 30, 45, -45 );
	SetPoseParameter( "rudder", flRudderPose );
#endif
}

//-----------------------------------------------------------------------------
// Purpose:	
//-----------------------------------------------------------------------------
float CNPC_Huey::UpdatePerpPathDistance( float flMaxPathOffset )
{
	if ( !IsLeading() || !GetEnemy() )
	{
		m_flCurrPathOffset = 0.0f;
		return 0.0f;
	}

	float flNewPathOffset = TargetDistanceToPath();

#if 0
	// Make bomb dropping more interesting
	if ( ShouldDropBombs() )
	{
		float flSpeedAlongPath = TargetSpeedAlongPath();

		if ( flSpeedAlongPath > 10.0f )
		{
			float flLeadTime = GetLeadingDistance() / flSpeedAlongPath;
			flLeadTime = clamp( flLeadTime, 0.0f, 2.0f );
			flNewPathOffset += 0.25 * flLeadTime * TargetSpeedAcrossPath();
		}

		flSpeedAlongPath = clamp( flSpeedAlongPath, 100.0f, 500.0f );
		float flSinHeight = SimpleSplineRemapVal( flSpeedAlongPath, 100.0f, 500.0f, 0.0f, 200.0f );
		flNewPathOffset += flSinHeight * sin( 2.0f * M_PI * (gpGlobals->curtime / 6.0f) );
	}
#endif

	if ( (flMaxPathOffset != 0.0f) && (flNewPathOffset > flMaxPathOffset) )
	{
		flNewPathOffset = flMaxPathOffset;
	}

	float flMaxChange = 1000.0f * (gpGlobals->curtime - GetLastThink());
	if ( fabs( flNewPathOffset - m_flCurrPathOffset ) < flMaxChange )
	{
		m_flCurrPathOffset = flNewPathOffset;
	}
	else
	{
		float flSign = (m_flCurrPathOffset < flNewPathOffset) ? 1.0f : -1.0f;
		m_flCurrPathOffset += flSign * flMaxChange;
	}

	return m_flCurrPathOffset;
}
#elif 0
void CNPC_Huey::Flight( void )
{
	float flMaxSpeed = m_flMaxSpeed; /*GetMaxSpeed(); FIXME: ignoring path_track speeds */
	float flAccel = sk_huey_accel.GetFloat();/*GetMaxSpeed()*/

	if ( GetFlags() & FL_ONGROUND )
	{
		//This would be really bad.
		SetGroundEntity( NULL );
	}

	// Yaw to the desired direction
	Vector vecFace = m_vecDesiredFaceDir;
	VectorNormalize( vecFace );
	QAngle angles;
	VectorAngles( vecFace, angles );
	angles.x = GetAbsAngles().x; angles.z = GetAbsAngles().z;
	angles.y = UTIL_ApproachAngle( angles.y, GetAbsAngles().y, 10 );
	SetAbsAngles( angles );

	//
	float flDistToGoal = GetAbsOrigin().DistTo(GetDesiredPosition());
	if ( flDistToGoal < 1 || (GetAbsOrigin() + GetAbsVelocity()).DistTo(GetDesiredPosition()) > flDistToGoal )
	{
		SetAbsOrigin( GetDesiredPosition() );
		SetAbsVelocity( vec3_origin );
		return;
	}

	// Move at max speed to desired location
	Vector pushTowards = GetDesiredPosition() - GetAbsOrigin();
	VectorNormalize( pushTowards );
	pushTowards *= flMaxSpeed;

	Vector pushAway = GetAbsOrigin() - GetDesiredPosition();
	float flBrakeDistance = sk_huey_brake_dist.GetFloat();
	if ( flDistToGoal < flBrakeDistance )
	{
		VectorNormalize( pushAway );
		pushAway *= (1.0 - flDistToGoal / flBrakeDistance) * flMaxSpeed;
	}
	else
	{
		pushAway = vec3_origin;
	}
//	NDebugOverlay::Line(GetAbsOrigin(), GetAbsOrigin() + pushTowards, 0,255,0, true, 0.1);
//	NDebugOverlay::Line(GetAbsOrigin(), GetAbsOrigin() + pushAway, 255,0,0, true, 0.1);

	Vector desiredVel = pushTowards + pushAway;
	float flSpeed = GetAbsVelocity().Length();
	if ( desiredVel.Length() > flSpeed + flAccel )
	{
		VectorNormalize(desiredVel);
		desiredVel *= flSpeed + flAccel;
	}
	if ( desiredVel.Length() < sk_huey_min_speed.GetFloat() )
	{
		VectorNormalize(desiredVel);
		desiredVel *= sk_huey_min_speed.GetFloat();
	}
	SetAbsVelocity( desiredVel );

	// Pose
//	float deltaSpeed = desiredVel.Length2D() - m_flPrevSpeed;
	angles = GetAbsAngles();
	float flDistToGoal2D = (GetDesiredPosition() - GetAbsOrigin()).Length2D();
	if ( flDistToGoal2D > flBrakeDistance )
	{
		angles.x += 4;
		if ( angles.x > 30 ) // accel
			angles.x = 30;
	}
	else
	{
		angles.x -= 4; // decel
#if 1
		float pct = flDistToGoal2D / flBrakeDistance;
		if ( angles.x < pct * -20 ) // decel
			angles.x = pct * -20;
#else
		float pctMaxSpeed = desiredVel.Length2D() / flMaxSpeed;
		if ( angles.x < pctMaxSpeed * -20 ) // decel
			angles.x = pctMaxSpeed * -20;
#endif
	}
	SetAbsAngles( angles );
//	m_flPrevSpeed = desiredVel.Length2D();
}


#else
ConVar sk_huey_push("sk_huey_push", "0.8");

void CNPC_Huey::Flight( void )
{
	float flGravity = 38.4; // 32ft/sec

	if( GetFlags() & FL_ONGROUND )
	{
		//This would be really bad.
		SetGroundEntity( NULL );
	}

	// Generic speed up
	if (m_flGoalSpeed < GetMaxSpeed())
	{
		m_flGoalSpeed += GetAcceleration();
	}
	
	//NDebugOverlay::Line(GetAbsOrigin(), m_vecDesiredPosition, 0,0,255, true, 0.1);

	// tilt model 5 degrees (why?! sjb)
	QAngle vecAdj = QAngle( 5.0, 0, 0 );

	// estimate where I'll be facing in one seconds
	Vector forward, right, up;
	AngleVectors( GetLocalAngles() + GetLocalAngularVelocity() * 2 + vecAdj, &forward, &right, &up );

	// Vector vecEst1 = GetLocalOrigin() + GetAbsVelocity() + up * m_flForce - Vector( 0, 0, 384 );
	// float flSide = DotProduct( m_vecDesiredPosition - vecEst1, right );
	QAngle angVel = GetLocalAngularVelocity();
	float flSide = DotProduct( m_vecDesiredFaceDir, right );
	if (flSide < 0)
	{
		if (angVel.y < 60)
		{
			angVel.y += 8;
		}
	}
	else
	{
		if (angVel.y > -60)
		{
			angVel.y -= 8;
		}
	}

	angVel.y *= ( 0.98 ); // why?! (sjb)

	// estimate where I'll be in two seconds
	AngleVectors( GetLocalAngles() + angVel * 1 + vecAdj, NULL, NULL, &up );
	Vector vecEst = GetAbsOrigin() + GetAbsVelocity() * 2.0 + up * m_flForce * 20 - Vector( 0, 0, flGravity * 20 );

	// add immediate force
	AngleVectors( GetLocalAngles() + vecAdj, &forward, &right, &up );
	
	// The force is always *locally* upward based; we pitch + roll the chopper to get movement
	Vector vecImpulse;
	VectorMultiply( up, m_flForce, vecImpulse );

	// add gravity
	vecImpulse.z -= flGravity;
	ApplyAbsVelocityImpulse( vecImpulse );

	float flSpeed = GetAbsVelocity().Length();
	float flDir = DotProduct( Vector( forward.x, forward.y, 0 ), Vector( GetAbsVelocity().x, GetAbsVelocity().y, 0 ) );
	if (flDir < 0)
	{
		flSpeed = -flSpeed;
	}

	float flDist = DotProduct( GetDesiredPosition() - vecEst, forward );

	// float flSlip = DotProduct( GetAbsVelocity(), right );
	float flSlip = -DotProduct( GetDesiredPosition() - vecEst, right );

	// fly sideways
	if (flSlip > 0)
	{
		if (GetLocalAngles().z > -30 && angVel.z > -15)
			angVel.z -= 4;
		else
			angVel.z += 2;
	}
	else
	{
		if (GetLocalAngles().z < 30 && angVel.z < 15)
			angVel.z += 4;
		else
			angVel.z -= 2;
	}

	// These functions contain code Ken wrote that used to be right here as part of the flight model,
	// but we want different helicopter vehicles to have different drag characteristics, so I made
	// them virtual functions (sjb)
	ApplySidewaysDrag( right );
	ApplyGeneralDrag();

	// apply power to stay correct height
	// FIXME: these need to be per class variables
#define MIN_FORCE		30
#define MAX_FORCE		80
#define FORCE_POSDELTA	12	
#define FORCE_NEGDELTA	8

	if (m_flForce < MAX_FORCE && vecEst.z < GetDesiredPosition().z) 
	{
		m_flForce += FORCE_POSDELTA;
		if (m_flForce > MAX_FORCE)
			m_flForce = MAX_FORCE;
	}
	else if (m_flForce > MIN_FORCE && vecEst.z > GetDesiredPosition().z)
	{
		m_flForce -= FORCE_NEGDELTA;
		if (m_flForce < MIN_FORCE)
			m_flForce = MIN_FORCE;
	}
	
	// pitch forward or back to get to target
	//-----------------------------------------
	// Pitch is reversed since Half-Life! (sjb)
	//-----------------------------------------
	if (flDist > 0 && flSpeed < m_flGoalSpeed /* && flSpeed < flDist */ && GetLocalAngles().x + angVel.x < 40)
	{
		// ALERT( at_console, "F " );
		// lean forward
		angVel.x += 12.0;
	}
	else if (flDist < 0 && flSpeed > -50 && GetLocalAngles().x + angVel.x  > -20)
	{
		// ALERT( at_console, "B " );
		// lean backward
		angVel.x -= 12.0;
	}
	else if (GetLocalAngles().x + angVel.x < 0)
	{
		// ALERT( at_console, "f " );
		angVel.x += 4.0;
	}
	else if (GetLocalAngles().x + angVel.x > 0)
	{
		// ALERT( at_console, "b " );
		angVel.x -= 4.0;
	}

	SetLocalAngularVelocity( angVel );
	// ALERT( at_console, "%.0f %.0f : %.0f %.0f : %.0f %.0f : %.0f\n", GetAbsOrigin().x, GetAbsVelocity().x, flDist, flSpeed, GetLocalAngles().x, m_vecAngVelocity.x, m_flForce ); 
	// ALERT( at_console, "%.0f %.0f : %.0f %0.f : %.0f\n", GetAbsOrigin().z, GetAbsVelocity().z, vecEst.z, m_vecDesiredPosition.z, m_flForce ); 

#if 0
	// If current velocity takes us further from the goal then brake
	float d1 = (GetAbsOrigin() + GetAbsVelocity()).DistToSqr(GetDesiredPosition());
	float d2 = GetAbsOrigin().DistToSqr(GetDesiredPosition());
	if (d1 > d2)
	{
		SetAbsVelocity( GetAbsVelocity() * sk_huey_push.GetFloat() );
	}
#endif
}
#endif

//-----------------------------------------------------------------------------
#ifdef LOGIC_HUEY_DEPLOY
CAI_BaseNPC *CNPC_Huey::SpawnGrunt( CLogicHueyDeploy *pDeploy, const char *szAttachment0, const char *szAttachment1, Activity iDeployActivity )
{
	int perDeploy, numGrunts, numMedics;
	if ( !pDeploy->GetDeployCounts( &perDeploy, &numGrunts, &numMedics ) )
		return NULL;

	for ( int i = 0; i < 4; i++ )
	{
		if ( m_hRappel[i] != NULL && i+1 >= perDeploy )
			return NULL;
	}

	CAI_BaseNPC	*pNPC;

	if ( m_iPassenger == HUEY_PASSENGER_MIKE )
	{
		if ( (numMedics <= 0) || (numGrunts > 0 && random->RandomInt( 0, 3 )) )
			pNPC = (CAI_BaseNPC *) CreateEntityByName( "npc_mikeforce" );
		else
			pNPC = (CAI_BaseNPC *) CreateEntityByName( "npc_mikeforce_medic" );
	}
	else
	{
		if ( (numMedics <= 0) || (numGrunts > 0 && random->RandomInt( 0, 3 )) )
			pNPC = (CAI_BaseNPC *) CreateEntityByName( "npc_human_grunt" );
		else
			pNPC = (CAI_BaseNPC *) CreateEntityByName( "npc_grunt_medic" );
	}

	if ( pNPC == NULL )
		return NULL;

	int iAttachment0 = LookupAttachment( szAttachment0 );
	int iAttachment1 = LookupAttachment( szAttachment1 );

	Vector vecAttach0;
	QAngle angAttach0;
	GetAttachment( iAttachment0, vecAttach0, angAttach0 );

	pNPC->SetAbsOrigin( vecAttach0 );
	pNPC->SetAbsAngles( angAttach0 );
//	pNPC->m_spawnEquipment = AllocPooledString( "weapon_m16" ); ApplyDefaultSettings
	pNPC->KeyValue( "body", "-1" );
	if ( pDeploy->GetSquadName() != NULL_STRING )
		pNPC->KeyValue( "squadname", STRING(pDeploy->GetSquadName()) );
	pNPC->KeyValue( "waitinghueyrappel", "1" );
	pNPC->KeyValue( "additionalequipment", "random" );
	ASSERT( pDeploy->GetEntityName() != NULL_STRING );
	pNPC->KeyValue( "DeployLogic", STRING( pDeploy->GetEntityName() ) ); // The NPC will register/unregister with the logic_huey_deploy
	if ( m_iPassenger == HUEY_PASSENGER_GRUNT_FR )
		pNPC->AddSpawnFlags( SF_HUMAN_FRIENDLY );
	pNPC->AddSpawnFlags( SF_NPC_FADE_CORPSE );
	DispatchSpawn( pNPC );
	pNPC->SetOwnerEntity( this );
	DispatchActivate( pNPC );

	pDeploy->m_OnSpawnNPC.Set( pNPC, pNPC, this );

	pNPC->SetActivity( iDeployActivity ); // should be "wait-to-deploy" activity

	// ChildPostSpawn( pent ); FIXME
	//pNPC->BeginRappel();
	((CHOEHuman *) pNPC)->m_HueyRappelBehavior.BeginRappel( this, iAttachment1, iDeployActivity );

	return pNPC;
}
#else
CAI_BaseNPC *CNPC_Huey::SpawnGrunt( const Vector &vecOrigin )
{
	int freeSlot = -1;
	for ( int i = 0; i < m_iMaxDeployed; i++ )
	{
		if ( !m_hDeployed[i] )
		{
			freeSlot = i;
			break;
		}
	}
	if ( freeSlot == -1 )
		return NULL;

	CAI_BaseNPC	*pNPC;

	// Maintain max medic population of 1/4 all deployed grunts
	int medicLimit = max(m_iMaxDeployed / 4, 1);
	int numMedics = 0;
	for ( int i = 0; i < m_iMaxDeployed; i++ )
	{
		if ( m_hDeployed[i] != NULL && m_hDeployed[i]->IsAlive() )
		{
			if ( m_hDeployed[i]->MyNPCPointer()->IsMedic() )
			{
				++numMedics;
			}
		}
	}
	bool bMedicOK = numMedics < medicLimit;

	if ( m_iPassenger == HUEY_PASSENGER_MIKE )
	{
		if ( !bMedicOK || random->RandomInt( 0, 3 ) )
			pNPC = (CAI_BaseNPC *) CreateEntityByName( "npc_mikeforce" );
		else
			pNPC = (CAI_BaseNPC *) CreateEntityByName( "npc_mikeforce_medic" );
	}
	else
	{
		if ( !bMedicOK || random->RandomInt( 0, 3 ) )
			pNPC = (CAI_BaseNPC *) CreateEntityByName( "npc_human_grunt" );
		else
			pNPC = (CAI_BaseNPC *) CreateEntityByName( "npc_grunt_medic" );
	}

	if ( pNPC == NULL )
		return NULL;

	pNPC->SetAbsOrigin( vecOrigin );
//	pNPC->m_spawnEquipment = AllocPooledString( "weapon_m16" ); ApplyDefaultSettings
	pNPC->SetSquadName( AllocPooledString( "huey_squad" ) );
	pNPC->KeyValue( "waitingtorappel", "1" );
	pNPC->KeyValue( "additionalequipment", "random" );
	if ( m_iPassenger == HUEY_PASSENGER_GRUNT_FR )
		pNPC->AddSpawnFlags( SF_HUMAN_FRIENDLY );
	DispatchSpawn( pNPC );
	pNPC->SetOwnerEntity( this );
	DispatchActivate( pNPC );
	// ChildPostSpawn( pent ); FIXME
	pNPC->BeginRappel();

	ASSERT( freeSlot >= 0 && freeSlot < MAX_GRUNTS && m_hDeployed[freeSlot] == NULL );
	m_hDeployed[freeSlot] = pNPC;

	return pNPC;
}
#endif

//-----------------------------------------------------------------------------
void CNPC_Huey::DeathNotice( CBaseEntity *pVictim )
{
	for ( int i = 0; i < 4; i++ )
	{
		if ( pVictim == m_hRappel[i] )
		{
			m_hRappel[i] = 0;
			DbgHueyMsg("CNPC_Huey::DeathNotice RAPPEL\n");
			break;
		}
	}

#ifndef LOGIC_HUEY_DEPLOY
	for ( int i = 0; i < m_iMaxDeployed; i++ )
	{
		if ( pVictim == m_hDeployed[i] )
		{
			m_hDeployed[i] = 0;
			DevMsg("CNPC_Huey::DeathNotice DEPLOYED\n");
			break;
		}
	}
#endif
//	DevWarning("CNPC_Huey::DeathNotice NOT A RAPPEL/DEPLOYED\n");
}

//-----------------------------------------------------------------------------
void CNPC_Huey::DetachRappellers( void )
{
	// Cut loose anyone rappelling from us
	for ( int i = 0; i < 4; i++ )
	{
		if ( m_hRappel[i] && !(m_hRappel[i]->GetFlags() & FL_ONGROUND) )
		{
			CHOEHuman *pNPC = (CHOEHuman *) m_hRappel[i].Get();
			pNPC->m_HueyRappelBehavior.AbortRappel();
		}
	}
}

//------------------------------------------------------------------------------
bool CNPC_Huey::FireGun( void )
{
	if ( !HasEnemy() ) { /*Msg("CNPC_Huey::FireGun: no enemy\n");*/ return false; }
	if ( HasCondition( COND_ENEMY_OCCLUDED ) )  { Msg("CNPC_Huey::FireGun: enemy occluded\n"); return false; }

	// Don't shoot if we don't have an enemy or haven't seem it for a while.
	// Don't shoot if we only recently saw our enemy.
	if ( (m_flLastSeen + 1 <= gpGlobals->curtime) || (m_flPrevSeen + m_flGracePeriod > gpGlobals->curtime) )
		return false;

	// Get gun attachment points
	Vector vBasePos;
	GetAttachment( m_iAttachmentLeftGunBase, vBasePos );

	// where to shoot at
	Vector vecFireAtPosition;
	ComputeFireAtPosition( &vecFireAtPosition );

	Vector vTargetDir = vecFireAtPosition - vBasePos;
	VectorNormalize( vTargetDir );

	if ( !PoseGunTowardTargetDirection( vTargetDir ) )
		return false;

	Vector vTipPos;
	GetAttachment( m_iAttachmentLeftGunMuzzle, vTipPos );

	Vector vGunDir = vTipPos - vBasePos;
	VectorNormalize( vGunDir );

//NDebugOverlay::Line(vBasePos, vBasePos + vGunDir*512, 0,0,255, true, 0.1);
//NDebugOverlay::Line(vBasePos, vBasePos + vTargetDir*512, 255,0,0, true, 0.1);
	if ( DotProduct( vTargetDir, vGunDir ) <= 0.98 )
		return false;

//	NDebugOverlay::Line(vTipPos, vecFireAtPosition, 0,0,255, true, 0);

	// Make AI sound. Don't use the same channel as the passenger gun.
	CSoundEnt::InsertSound( SOUND_COMBAT|SOUND_CONTEXT_GUNFIRE, vTipPos, SOUNDENT_VOLUME_MACHINEGUN, 0.2, this, SOUNDENT_CHANNEL_REPEATING, GetEnemy() );

	return DoGunFiring( vTipPos, vTargetDir /* vGunDir */, vecFireAtPosition );
}

//-----------------------------------------------------------------------------
int CNPC_Huey::IRelationPriority( CBaseEntity *pTarget )
{
	int priority = BaseClass::IRelationPriority( pTarget );

	Vector forward;
	AngleVectors( GetAbsAngles(), &forward );

	if ( m_bChoosingPassengerEnemy )
	{
		Vector vBasePos;
		GetAttachment( m_iAttachmentPassengerGunBase, vBasePos );

		Vector vTipPos;
		GetAttachment( m_iAttachmentPassengerGunMuzzle, vTipPos );

		Vector vGunDir = vTipPos - vBasePos;
		vGunDir.z = 0;

		Vector vTargetDir = pTarget->GetAbsOrigin() - vBasePos;
		vTargetDir.z = 0;

		// The passenger prefers enemies that are on the side of the huey.
		if ( DotProduct( vTargetDir, forward ) >= 0 && DotProduct( vTargetDir, forward ) < 0.7 )
			priority += 1;

		// The passenger prefers enemies that he is already facing.
		if ( DotProduct( vTargetDir, vGunDir ) > 0 )
			priority += 1;

		// The passenger prefers enemies that he has LOS to.
		trace_t tr;
		UTIL_TraceLine( vBasePos, pTarget->WorldSpaceCenter(), MASK_SHOT, NULL, COLLISION_GROUP_NONE,
			&tr );
		if ( tr.m_pEnt == pTarget )
			priority += 1;
	}

	else
	{
		Vector vTargetDir = pTarget->GetAbsOrigin() - GetAbsOrigin();
		vTargetDir.z = 0;

		// The huey prefers enemies that are in front of it
		if ( DotProduct( vTargetDir, forward ) > 0.9 )
			priority += 1;
	}

	return priority;
}

//------------------------------------------------------------------------------
void CNPC_Huey::UpdatePassengerEnemy( void )
{
	if ( !HasSpawnFlags( SF_PASSENGER ) )
		return;

	if ( m_lifeState == LIFE_ALIVE )
	{
		m_hPassengerEnemy = BestPassengerEnemy();
	}
	else
	{
		m_hPassengerEnemy = NULL;
	}
}

//------------------------------------------------------------------------------
CBaseEntity *CNPC_Huey::BestPassengerEnemy( void )
{
	m_bChoosingPassengerEnemy = true;
	CBaseEntity *pEnemy = BestEnemy();
	m_bChoosingPassengerEnemy = false;
	return pEnemy;
}

//------------------------------------------------------------------------------
void CNPC_Huey::FirePassengerWeapons( void )
{
	if ( !HasSpawnFlags( SF_PASSENGER ) )
		return;

	if ( !m_hPassengerEnemy || !FVisible( m_hPassengerEnemy ) /*HasCondition( COND_ENEMY_OCCLUDED )*/ ) return;

#if 0
	// Don't shoot if we don't have an enemy or haven't seem it for a while.
	// Don't shoot if we only recently saw our enemy.
	if ( (m_flLastSeen + 1 <= gpGlobals->curtime) || (m_flPrevSeen + m_flGracePeriod > gpGlobals->curtime) )
		return;
#endif

	Vector vBasePos;
	GetAttachment( m_iAttachmentPassengerGunBase, vBasePos );

	// where to shoot at
	Vector vecFireAtPosition = m_hPassengerEnemy->WorldSpaceCenter(); // FIXME: BodyTarget()?

	Vector vTargetDir = vecFireAtPosition - vBasePos;
	VectorNormalize( vTargetDir );

	if ( !PosePassengerTowardTargetDirection( vTargetDir ) )
		return;

	if ( m_PassengerShotRegulator.IsInRestInterval() )
		return;

	Vector vTipPos;
	GetAttachment( m_iAttachmentPassengerGunMuzzle, vTipPos );

	Vector vGunDir = vTipPos - vBasePos;
	VectorNormalize( vGunDir );

	if ( DotProduct( vTargetDir, vGunDir ) <= 0.98 || ( m_angPassenger.y > -30 && m_angPassenger.y < 30 ) )
		return;

	// ----------
	// Sound
	// ----------
	EmitSound( "NPC_Huey.PassengerFireGun" );
	CSoundEnt::InsertSound( SOUND_COMBAT|SOUND_CONTEXT_GUNFIRE, vTipPos, SOUNDENT_VOLUME_MACHINEGUN, 0.2, this, SOUNDENT_CHANNEL_WEAPON, m_hPassengerEnemy );

	// ----------
	// Bullets
	// ----------
//	NDebugOverlay::Line( vTipPos, vecFireAtPosition, 255,0,0, true, 0);
	int	bulletType = GetAmmoDef()->Index("7_62mm");
	int nShotCount = 1;
	FireBulletsInfo_t info( nShotCount, vBasePos, vGunDir, VECTOR_CONE_6DEGREES, 8192, bulletType );
	info.m_iTracerFreq = 2;
	FireBullets( info );

	// ----------
	// Muzzleflash
	// ----------
	CEffectData data;
	data.m_nEntIndex = entindex();
	data.m_nAttachmentIndex = m_iAttachmentPassengerGunMuzzle;
	data.m_flScale = 1.0f;
	data.m_fFlags = MUZZLEFLASH_SMG1;
	DispatchEffect( "MuzzleFlash", data );

	m_PassengerShotRegulator.OnFiredWeapon();
}

//------------------------------------------------------------------------------
bool CNPC_Huey::DoGunFiring( const Vector &vBasePos, const Vector &vGunDir, const Vector &vecFireAtPosition )
{
	ShootAtFacingDirection( vBasePos, vGunDir, false );
	return true;
}

//------------------------------------------------------------------------------
// Shoot where we're facing
//------------------------------------------------------------------------------
void CNPC_Huey::ShootAtFacingDirection( const Vector &vBasePos, const Vector &vGunDir, bool bFirstShotAccurate )
{
	EmitSound("NPC_Huey.FireGun");

	CEffectData data;
	data.m_nEntIndex = entindex();
	data.m_nAttachmentIndex = m_iAttachmentLeftGunMuzzle;
	data.m_flScale = 1.0f;
	data.m_fFlags = MUZZLEFLASH_SMG1;
	DispatchEffect( "MuzzleFlash", data );

//	DoMuzzleFlash();

	int nShotCount = HUEY_ROUNDS_PER_BURST;
	FireBulletsInfo_t info( nShotCount, vBasePos, vGunDir, VECTOR_CONE_8DEGREES, 8192, m_iAmmoType );
	info.m_iTracerFreq = 1;
	FireBullets( info );


	// Right gun

	data.m_nAttachmentIndex = m_iAttachmentRightGunMuzzle;
	DispatchEffect( "MuzzleFlash", data );

	QAngle angles;
	GetAttachment( data.m_nAttachmentIndex, info.m_vecSrc, angles );
	FireBullets( info );	
}

// FIXME: not virtual
void CNPC_Huey::ComputeActualTargetPosition( float flSpeed, float flTime, float flPerpDist, Vector *pDest, bool bApplyNoise )
{
	bApplyNoise = bApplyNoise && m_bAllowNoise;

	// This is used to make the helicopter drift around a bit.
	if ( bApplyNoise && m_flRandomOffsetTime <= gpGlobals->curtime )
	{
//		m_vecRandomOffset.Random( -25.0f, 25.0f );
		m_vecRandomOffset.Random( -5.0f, 5.0f );
		m_flRandomOffsetTime = gpGlobals->curtime + 1.0f;
	}

	if ( IsLeading() && GetEnemy() && IsOnPathTrack() )
	{
		ComputePointAlongCurrentPath( flSpeed * flTime, flPerpDist, pDest );
		if ( bApplyNoise )
		{
			*pDest += m_vecRandomOffset;
		}
		return;
	}

#if 1 /* HOE_TRACK_SPLINE*/
	if ( IsOnPathTrack() && !IsCrashing() )
	{
		bool bBrake = false;

		// See if we should deploy at the next track
		if ( !m_bDeploy )
		{
			for ( int i = 0; i < m_DeployLogics.Count(); i++ )
			{
				if ( !m_DeployLogics[i] ) // may have been deleted
					continue;
				CLogicHueyDeploy *pDeploy = assert_cast<CLogicHueyDeploy *>(m_DeployLogics[i].Get());
				if ( pDeploy->m_iszPathTrack == GetCurrentPathTarget()->GetEntityName() )
				{
					if ( pDeploy->GetDeployCounts() )
					{
						bBrake = true;
					}
					break;
				}
			}
			AdvanceAlongCurrentPath( flSpeed * flTime, pDest, bBrake = true );
		}
		else
		{
			ComputeClosestPoint( GetAbsOrigin(), flSpeed * flTime, GetDesiredPosition(), pDest );
		}

#if 0
		// Blend in a fake destination point based on the dest velocity 
		Vector vecDestVelocity;
		ComputeNormalizedDestVelocity( &vecDestVelocity );
		vecDestVelocity *= flSpeed;

		float flBlendFactor = 1.0f - flDistToDesired / (flSpeed * flTime);
		VectorMA( *pDest, flTime * flBlendFactor, vecDestVelocity, *pDest );
#endif
		if ( bApplyNoise )
		{
			*pDest += m_vecRandomOffset;
		}
		return;

	}
#endif // HOE_TRACK_SPLINE

	*pDest = GetDesiredPosition() - GetAbsOrigin();
	float flDistToDesired = pDest->Length();
	if (flDistToDesired > flSpeed * flTime)
	{
		float scale = flSpeed * flTime / flDistToDesired;
		*pDest *= scale;
	}
	else if ( IsOnPathTrack() && !IsCrashing() )
	{
		// Blend in a fake destination point based on the dest velocity 
		Vector vecDestVelocity;
		ComputeNormalizedDestVelocity( &vecDestVelocity );
		vecDestVelocity *= flSpeed;

		float flBlendFactor = 1.0f - flDistToDesired / (flSpeed * flTime);
		VectorMA( *pDest, flTime * flBlendFactor, vecDestVelocity, *pDest );
	}

	*pDest += GetAbsOrigin();

	if ( bApplyNoise )
	{
		//	ComputePointAlongCurrentPath( flSpeed * flTime, flPerpDist, pDest );
		*pDest += m_vecRandomOffset;
	}
}

//------------------------------------------------------------------------------
// Compute the enemy position (non-vehicle case)
//------------------------------------------------------------------------------
void CNPC_Huey::ComputeFireAtPosition( Vector *pVecActualTargetPosition )
{
	// Deal with various leading behaviors...
	*pVecActualTargetPosition = m_vecTargetPosition;
}

//------------------------------------------------------------------------------
// Various states of the helicopter firing...
//------------------------------------------------------------------------------
bool CNPC_Huey::PoseGunTowardTargetDirection( const Vector &vTargetDir )
{
	Vector vecOut;
	VectorIRotate( vTargetDir, EntityToWorldTransform(), vecOut );

	QAngle angles;
	VectorAngles(vecOut, angles);

	if (angles.y > 180)
	{
		angles.y = angles.y - 360;
	}
	else if (angles.y < -180)
	{
		angles.y = angles.y + 360;
	}
	if (angles.x > 180)
	{
		angles.x = angles.x - 360;
	}
	else if (angles.x < -180)
	{
		angles.x = angles.x + 360;
	}
#if 0
	if ( ( m_nAttackMode == ATTACK_MODE_BULLRUSH_VEHICLE ) && !IsInSecondaryMode(BULLRUSH_MODE_SHOOT_IDLE_PLAYER) && GetEnemy())
	{
		if ( GetEnemyVehicle() )
		{
			angles.x = clamp( angles.x, -12.0f, 0.0f );
			angles.y = clamp( angles.y, -10.0f, 10.0f );
		}
		else
		{
			angles.x = clamp( angles.x, -10.0f, 10.0f );
			angles.y = clamp( angles.y, -10.0f, 10.0f );
		}
	}
#endif
	if (angles.x > m_angMiniGuns.x)
	{
		m_angMiniGuns.x = min( angles.x, m_angMiniGuns.x + 12 );
	}
	if (angles.x < m_angMiniGuns.x)
	{
		m_angMiniGuns.x = max( angles.x, m_angMiniGuns.x - 12 );
	}
	if (angles.y > m_angMiniGuns.y)
	{
		m_angMiniGuns.y = min( angles.y, m_angMiniGuns.y + 12 );
	}
	if (angles.y < m_angMiniGuns.y)
	{
		m_angMiniGuns.y = max( angles.y, m_angMiniGuns.y - 12 );
	}

	/*m_angMiniGuns.x =*/ SetBoneController( 0, m_angMiniGuns.x );
	/*SetPoseParameter( "weapon_pitch", -m_angMiniGuns.x );
	SetPoseParameter( "weapon_yaw", m_angMiniGuns.y );*/

	if ( HasSpawnFlags( SF_GRENADE ) )
	{
		//Move the grenade launcher towards the target
		if ( angles.x > m_angGren.x )
			m_angGren.x = min( angles.x, m_angGren.x + 12 );
		if ( angles.x < m_angGren.x )
			m_angGren.x = max( angles.x, m_angGren.x - 12 );

		m_angGren.x = SetBoneController( 3, m_angGren.x );
	}


	return true;
}

extern void FixupAngles( QAngle &v );

//------------------------------------------------------------------------------
bool CNPC_Huey::PosePassengerTowardTargetDirection( const Vector &vTargetDir )
{
	if ( !HasSpawnFlags( SF_PASSENGER ) )
		return false;

	Vector vecOut;
	VectorIRotate( vTargetDir, EntityToWorldTransform(), vecOut );

	QAngle angles;
	VectorAngles(vecOut, angles);

	FixupAngles( angles );
#if 1
	if (angles.y > 180)
	{
		angles.y = angles.y - 360;
	}
	else if (angles.y < -180)
	{
		angles.y = angles.y + 360;
	}
	if (angles.x > 180)
	{
		angles.x = angles.x - 360;
	}
	else if (angles.x < -180)
	{
		angles.x = angles.x + 360;
	}
#endif
	if (angles.x > m_angPassenger.x)
	{
		m_angPassenger.x = min( angles.x, m_angPassenger.x + 12 );
	}
	if (angles.x < m_angPassenger.x)
	{
		m_angPassenger.x = max( angles.x, m_angPassenger.x - 12 );
	}
	if (angles.y > m_angPassenger.y)
	{
		m_angPassenger.y = min( angles.y, m_angPassenger.y + 12 );
	}
	if (angles.y < m_angPassenger.y)
	{
		m_angPassenger.y = max( angles.y, m_angPassenger.y - 12 );
	}

	m_angPassenger.y = SetBoneController( 1, m_angPassenger.y );
	m_angPassenger.x = SetBoneController( 2, m_angPassenger.x );

	// Slide the passenger to left (positive) or right (negative)
	Vector vRight;
	AngleVectors( GetAbsAngles(), NULL, &vRight, NULL );
	Vector vTarget2D = vTargetDir; vTarget2D.z = 0;
	SetPoseParameter( "grunt_slide", ( DotProduct( vTargetDir, vRight ) < 0 ) ? 1 : -1 );

	return true;
}

//------------------------------------------------------------------------------
void CNPC_Huey::AimRocketGun( void )
{
	if ( !HasEnemy() ) { /*Msg("CNPC_Huey::AimRocketGun: no enemy\n");*/ return; }
	if ( HasCondition( COND_ENEMY_OCCLUDED ) )  { Msg("CNPC_Huey::AimRocketGun: enemy occluded\n"); return; }

	if ( m_flNextRocket > gpGlobals->curtime )
		return;

	Vector forward, up;
	AngleVectors( GetAbsAngles(), &forward, NULL, &up );

	Vector originLeft, originRight;
	QAngle angDummy;
	GetAttachment( m_iAttachmentLeftMissile, originLeft, angDummy );
	GetAttachment( m_iAttachmentRightMissile, originRight, angDummy );

#if 1
	// 0, 2, 4, 6, 8, 10 degrees; tan(2)==0.03 etc
	const float tangents[] = { 0, 0.03492076949f,
		                           0.06992681194f,
								   0.10510423526f,
								   0.14054083470f,
								   0.17632698070f };
	bool shotLeft = false, shotRight = false;
	trace_t	tr;
	for ( int i = 0; i < ARRAYSIZE(tangents); i++ )
	{
		Vector shotDir = forward - tangents[i] * up;

		if ( !shotLeft )
		{
			UTIL_TraceLine( originLeft, originLeft + shotDir * HUEY_ROCKET_RANGE, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );
			if ( CheckRocketImpactZone( tr.endpos ) )
			{
				FireRocket( originLeft, shotDir );
				m_flNextRocket = gpGlobals->curtime + 1.5;
				shotLeft = true;
			}
		}

		if ( !shotRight )
		{
			UTIL_TraceLine( originRight, originRight + shotDir * HUEY_ROCKET_RANGE, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );
			if ( CheckRocketImpactZone( tr.endpos ) )
			{
				FireRocket( originRight, shotDir );
				m_flNextRocket = gpGlobals->curtime + 1.5;
				shotRight = true;
			}
		}

		if ( shotLeft && shotRight )
			break;
	}
#else
	trace_t	tr;
	UTIL_TraceLine( originLeft, originLeft + forward * HUEY_ROCKET_RANGE, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );
	if ( CheckRocketImpactZone( tr.endpos ) )
	{
		FireRocket( originLeft, forward );
		m_flNextRocket = gpGlobals->curtime + 1.5;
//		return;
	}

	UTIL_TraceLine( originRight, originRight + forward * HUEY_ROCKET_RANGE, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );
	if ( CheckRocketImpactZone( tr.endpos ) )
	{
		FireRocket( originRight, forward );
		m_flNextRocket = gpGlobals->curtime + 1.5;
//		return;
	}
#endif
}

//------------------------------------------------------------------------------
void CNPC_Huey::FireRocket( Vector vLaunchPos, Vector vLaunchDir )
{
	QAngle angles;
	VectorAngles( vLaunchDir, angles );

	CRPG7Rocket *pRocket = (CRPG7Rocket *) CBaseEntity::Create( "rpg7_rocket", vLaunchPos, angles, this );
	if ( pRocket )
	{
		pRocket->DumbFire();
		pRocket->SetModel( "models/HVR.mdl" );
//		pRocket->SetNextThink( gpGlobals->curtime + 0.1f );
		pRocket->SetAbsVelocity( vLaunchDir * 1800.0f );
	}
}

//-----------------------------------------------------------------------------
bool CNPC_Huey::CheckRocketImpactZone( Vector vImpactPos )
{
	bool bEnemyInImpactZone = false;

	CAI_BaseNPC **ppAIs = g_AI_Manager.AccessAIs();
	int nAIs = g_AI_Manager.NumAIs();

	for ( int i = 0; i < nAIs; i++ )
	{
		CAI_BaseNPC *pNPC = ppAIs[ i ];

		if ( !pNPC->IsAlive() )
			continue;

		float flDist = (vImpactPos - pNPC->GetAbsOrigin()).LengthSqr();
		if ( flDist > Square( m_flRocketTolerance ) )
			continue;

		Disposition_t disposition = IRelationType( pNPC );
		if ( disposition == D_LI )
			return false;

		// friends aren't seen by default
		if ( !GetSenses()->DidSeeEntity( pNPC ) )
			continue;

		if ( disposition == D_HT )
			bEnemyInImpactZone = true;
	}

	// finally, check the player.
	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
	if ( pPlayer && !(pPlayer->GetFlags() & FL_NOTARGET) )
	{
		float flDist = (pPlayer->GetAbsOrigin() - vImpactPos ).LengthSqr();
		if ( flDist <= Square( m_flRocketTolerance ) && FVisible( pPlayer ) )
		{
			if ( IRelationType( pPlayer ) == D_LI )
				return false;

			if ( IRelationType( pPlayer ) == D_HT )
				bEnemyInImpactZone = true;
		}
	}

	return bEnemyInImpactZone;
}

//-----------------------------------------------------------------------------
void CNPC_Huey::FireGrenade( void )
{
	if ( !HasSpawnFlags( SF_GRENADE ) )
		return;

	if ( m_flNextGrenade > gpGlobals->curtime )
		return;

	if ( !HasEnemy() || HasCondition( COND_ENEMY_OCCLUDED ) )
		return;

	Vector vImpactPos = GetEnemy()->WorldSpaceCenter();

	Vector vBasePos;
	GetAttachment( m_iAttachmentGrenadeBase, vBasePos );

	Vector vTipPos;
	GetAttachment( m_iAttachmentGrenadeMuzzle, vTipPos );

	if ( (vImpactPos - vTipPos).LengthSqr() > 1024 * 1024 )
		return;

	Vector vGunDir = vTipPos - vBasePos;
	VectorNormalize( vGunDir );

	Vector vTargetDir = vImpactPos - vBasePos;
	VectorNormalize( vTargetDir );

	if ( DotProduct( vGunDir, vTargetDir ) < 0.96 )
		return;

	Vector vecMins = Vector(-3,-3,-3);
	Vector vecMaxs = Vector(3,3,3);

	float flGravity = UTIL_ScaleForGravity( 400 ); // NOTE: must match AR2

	Vector vecToss = VecCheckThrow( this, vTipPos, vImpactPos,
		800.0, flGravity, &vecMins, &vecMaxs );

	if ( vecToss != vec3_origin && CheckRocketImpactZone( vImpactPos ) )
	{
		EmitSound( "NPC_Huey.Grenade" );

		CGrenadeAR2 *pGrenade = (CGrenadeAR2*)Create( "grenade_ar2", vTipPos, vec3_angle, this );
		pGrenade->SetAbsVelocity( vecToss );
		pGrenade->SetLocalAngularVelocity( QAngle( 0, 400, 0 ) );
		pGrenade->SetMoveType( MOVETYPE_FLYGRAVITY ); 
		pGrenade->SetThrower( this );
//		pGrenade->m_pMyWeaponAR2	= this;
		pGrenade->SetDamage( 50 );

		m_flNextGrenade = gpGlobals->curtime + 1.5;
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNPC_Huey::InputDeploy( inputdata_t &inputdata )
{
	// Find the specified path_track
	string_t strTrackName = MAKE_STRING( inputdata.value.String() );
	CBaseEntity *pGoalEnt = gEntList.FindEntityByName( NULL, strTrackName );
	if ( pGoalEnt == NULL )
	{
		DevWarning( "%s: Could not find path_track '%s'!\n", GetDebugName(), STRING( strTrackName ) );
		return;
	}

	// Make sure it is a path_track
	CPathTrack *pTrack = dynamic_cast<CPathTrack *>(pGoalEnt);
	if ( !pTrack )
	{
		DevWarning( "%s: Specified entity '%s' must be a path_track!\n", GetDebugName(), STRING( strTrackName ) );
		return;
	}

#ifdef LOGIC_HUEY_DEPLOY
	m_hDeployLogic = NULL;
	for ( int i = 0; i < m_DeployLogics.Count(); i++ )
	{
		if ( !m_DeployLogics[i] ) // may have been deleted
			continue;
		CLogicHueyDeploy *pDeploy = assert_cast<CLogicHueyDeploy *>(m_DeployLogics[i].Get());
		if ( pDeploy->m_iszPathTrack == pTrack->GetEntityName() )
		{
			if ( !pDeploy->GetDeployCounts() )
			{
				DbgHueyMsg( "%s: InputDeploy skipped due to no grunts to deploy\n", GetDebugName() );
				return;
			}
			m_hDeployLogic = pDeploy;
			break;
		}
	}
	if ( m_hDeployLogic == NULL )
	{
		DevWarning( "%s: InputDeploy can't find a logic_huey_deploy for path_track %s\n", GetDebugName(), STRING( strTrackName ) );
		return;
	}
#else
	// Don't deploy if all previous deployed grunts are still alive (or if we never have grunts to deploy).
	int living = 0;
	for ( int i = 0; i < m_iMaxDeployed; i++ )
		if ( m_hDeployed[i] != 0 )
			living++;
	if ( (m_iMaxDeploy <= 0) || (m_iMaxDeployed <= 0) || (living == m_iMaxDeployed))
	{
		DevMsg( "%s: InputDeploy skipped due to no grunts to deploy\n", GetDebugName() );
		return;
	}
#endif

	// Force movement to the specified path_track
//	MoveToTrackPoint( pTrack );

	// Eventually face in the forward direction of the path_track
	AngleVectors( pTrack->GetAbsAngles(), &m_vecDeployFaceDir );
	m_vecDeployPosition = pTrack->GetAbsOrigin() /*pTrack->WorldSpaceCenter()*/;
	m_hDeployTrack = pTrack;

	SetPauseState( PAUSED_AT_POSITION ); // important so UpdateCurrentTarget() doesn't select the next path_track

	SetDeployState( DEPLOY_STATE_MOVE_TO_PATHTRACK );
	m_bHasReachedTarget = false;
	m_bDeploy = true;

	DbgHueyMsg("CNPC_Huey::InputDeploy\n");
}

//------------------------------------------------------------------------------
void CNPC_Huey::InputGrenadeOn( inputdata_t &inputdata )
{
	AddSpawnFlags( SF_GRENADE );
}

//-----------------------------------------------------------------------------
void CNPC_Huey::InputGrenadeOff( inputdata_t &inputdata )
{
	RemoveSpawnFlags( SF_GRENADE );
}

//-----------------------------------------------------------------------------
void CNPC_Huey::InputFireRockets( inputdata_t &inputdata )
{
	Vector forward;
	GetVectors( &forward, NULL, NULL );

	Vector originLeft, originRight;
	QAngle angDummy;
	GetAttachment( m_iAttachmentLeftMissile, originLeft, angDummy );
	GetAttachment( m_iAttachmentRightMissile, originRight, angDummy );

	FireRocket( originLeft, forward );
	FireRocket( originRight, forward );
}

//-----------------------------------------------------------------------------
void CNPC_Huey::InputSetRocketTolerance( inputdata_t &inputdata )
{
	m_flRocketTolerance = inputdata.value.Float();
}

//-----------------------------------------------------------------------------
void CNPC_Huey::InputAllowDamageMovement( inputdata_t &inputdata )
{
	m_bAllowDamageMovement = inputdata.value.Bool();
}

//-----------------------------------------------------------------------------
void CNPC_Huey::InputMinimalDamageMovement( inputdata_t &inputdata )
{
	m_bMinimalDamageMovement = inputdata.value.Bool();
}

//-----------------------------------------------------------------------------
int CNPC_Huey::OnTakeDamage_Alive( const CTakeDamageInfo &info )
{
	if ( info.GetAttacker() == this )
	{
		return BaseClass::OnTakeDamage_Alive( info );
	}

	float damage = info.GetMaxDamage();

	// Stop NPCs having all the fun
	if ( info.GetAttacker() && !info.GetAttacker()->IsPlayer() )
	{
		damage = min( damage, 50.0f );
	}
	else
	{
		damage = min( damage, GetMaxHealth() / 3 ); // takes 3 rockets to kill
	}

	if ( m_iHealth - damage < GetMaxHealth() / 2 )
		StartSmoking();

#ifdef ATTACK_CHOPPER
	if ( (info.GetDamageType() & DMG_BLAST) != 0 && m_bAllowDamageMovement )
	{
		float flScale = m_bMinimalDamageMovement ? 0.3 : 1.0;

		// Apply a force push that makes us look like we're reacting to the damage
		Vector damageDir = info.GetDamageForce();
		VectorNormalize( damageDir );
		ApplyAbsVelocityImpulse( damageDir * 250.0f * flScale );

		// Knock the helicopter off of the level, too.
		Vector vecRight, vecForce;
		float flDot;
		GetVectors( NULL, &vecRight, NULL );
		vecForce = info.GetDamageForce();
		VectorNormalize( vecForce );

		flDot = DotProduct( vecForce, vecRight );

		m_flGoalRollDmg = random->RandomFloat( 10, 30 ) * flScale;

		if( flDot <= 0.0f )
		{
			// Missile hit the right side.
			m_flGoalRollDmg *= -1;
		}

		// Cut loose anyone rappelling from us
		DetachRappellers();
	}
#endif

	CTakeDamageInfo newInfo = info;
	newInfo.SetDamage( damage );
	newInfo.SetMaxDamage( damage );
	DbgHueyMsg("CNPC_Huey::OnTakeDamage_Alive damage=%.2f\n", damage);
	return BaseClass::OnTakeDamage_Alive( newInfo );
}

//-----------------------------------------------------------------------------
void CNPC_Huey::TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr )
{
	DbgHueyMsg("CNPC_Huey::TraceAttack hitgroup=%d damage=%.2f\n", ptr->hitgroup, info.GetMaxDamage());
	if ( ptr->hitgroup == 1 )
	{
		// If gunner is hit
		if ( HasSpawnFlags( SF_PASSENGER ) )
		{
			m_flPassengerHealth -= info.GetDamage();
			UTIL_BloodSpray( ptr->endpos, ptr->plane.normal, BLOOD_COLOR_RED, 1, FX_BLOODSPRAY_CLOUD );

			// If gunner is dead
			if ( m_flPassengerHealth <= 0 )
			{
				// Get rid of gunner
				m_flPassengerHealth = 0;
				SetBodygroup( BODY_PASSENGER, HUEY_PBODY_NONE );
				RemoveSpawnFlags( SF_PASSENGER );

				// Create a falling dead body
				Vector vBasePos;
				QAngle angBase;
				GetAttachment( m_iAttachmentPassengerGunBase, vBasePos, angBase );
				vBasePos.z -= 35;
				angBase.x = angBase.z = 0;

				CBaseEntity *pGib;
				const char *model = "models/namGrunt/namGrunt.mdl";
				if ( m_iPassenger == HUEY_PASSENGER_MIKE )
					model = "models/mikeforce/mikeforce.mdl";
				pGib = CreateRagGib( model, vBasePos, angBase, (vecDir * 600) + Vector(0, 0, 600), 0.0f );

				if ( info.GetDamageType() & DMG_BLAST )
				{
					CBaseAnimating *pBase = dynamic_cast<CBaseAnimating*>(pGib);
					if( pBase )
					{
						pBase->Ignite( 10, false );
					}
				}

				// Create a falling M60
				CBaseEntity *pGun = CreateEntityByName( "weapon_m60" );
				if ( pGun != NULL )
				{
					pGun->SetAbsOrigin( vBasePos );
					pGun->SetAbsVelocity( vecDir * 200 );
					pGun->SetLocalAngularVelocity( GetLocalAngularVelocity() );
					pGun->Spawn();
				}
			}
		}
	}

	// Take no damage from trace attacks unless it's blast damage. RadiusDamage() sometimes calls
	// TraceAttack() as a means for delivering blast damage. Usually when the explosive penetrates
	// the target. (RPG missiles do this sometimes).
	if ( ( info.GetDamageType() & DMG_BLAST ) == 0 )
		return;

	BaseClass::TraceAttack( info, vecDir, ptr, 0 );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Huey::BecomeChunks( void )
{
	CNPC_Huey *pChopper = this;

	QAngle vecChunkAngles = pChopper->GetAbsAngles();
	Vector vecForward, vecRight, vecUp;
	pChopper->GetVectors( &vecForward, &vecRight, &vecUp );

	// New for EP2, we may be tailspinning, (crashing) and playing an animation that is spinning
	// our root bone, which means our model is not facing the way our entity is facing. So we have
	// to do some attachment point math to get the proper angles to use for computing the relative
	// positions of the gibs. The attachment points called DAMAGE0 is properly oriented and attached
	// to the chopper body so we can use its angles.
	int iAttach = pChopper->LookupAttachment( "passenger_pilot" );
	if( iAttach != INVALID_ATTACHMENT )
	{
		Vector vecAttachPos;
		pChopper->GetAttachment(iAttach, vecAttachPos, vecChunkAngles );
		AngleVectors( vecChunkAngles, &vecForward, &vecRight, &vecUp );

		// We need to get a right hand vector to toss the cockpit and tail pieces
		// so their motion looks like a continuation of the tailspin animation
		// that the chopper plays before crashing.
//		pChopper->GetVectors( NULL, &vecRight, NULL );
	}

	Vector vecChunkPos = pChopper->GetAbsOrigin();

	CHueyChunk *pChunks[CHUNK_MAX];
	memset( pChunks, 0, sizeof(pChunks) );

	// Body
	pChunks[CHUNK_ENGINE] = CHueyChunk::CreateHelicopterChunk( this, vecChunkPos, vecChunkAngles, pChopper->GetAbsVelocity() + vecRight * -500.0f, CHUNK_ENGINE );
//	Chopper_CreateChunk( pChopper, vecChunkPos, RandomAngle( 0, 360 ), s_pChunkModelName[random->RandomInt( 0, CHOPPER_MAX_CHUNKS - 1 )], false );
	CHueyChunk *pMasterChunk = pChunks[CHUNK_ENGINE];

//	vecChunkPos = pChopper->GetAbsOrigin() + ( vecForward * 100.0f ) + ( vecUp * -38.0f );

	// Cockpit
	pChunks[CHUNK_CABIN] = CHueyChunk::CreateHelicopterChunk( this, vecChunkPos, vecChunkAngles, pChopper->GetAbsVelocity() + vecRight * -500.0f, CHUNK_CABIN );
//	Chopper_CreateChunk( pChopper, vecChunkPos, RandomAngle( 0, 360 ), s_pChunkModelName[random->RandomInt( 0, CHOPPER_MAX_CHUNKS - 1 )], false );

//	vecChunkPos = pChopper->GetAbsOrigin() + ( vecForward * -175.0f );

	// Tail
	pChunks[CHUNK_TAIL] = CHueyChunk::CreateHelicopterChunk( this, vecChunkPos, vecChunkAngles, pChopper->GetAbsVelocity() + vecRight * 500.0f, CHUNK_TAIL );
//	Chopper_CreateChunk( pChopper, vecChunkPos, RandomAngle( 0, 360 ), s_pChunkModelName[random->RandomInt( 0, CHOPPER_MAX_CHUNKS - 1 )], false );

	// Roof
	pChunks[CHUNK_ROOF] = CHueyChunk::CreateHelicopterChunk( this, vecChunkPos, vecChunkAngles, pChopper->GetAbsVelocity() /*+ vecRight * 800.0f*/, CHUNK_ROOF );

	// Left strut
	pChunks[CHUNK_LEFT_STRUT] = CHueyChunk::CreateHelicopterChunk( this, vecChunkPos, vecChunkAngles, pChopper->GetAbsVelocity() /*+ vecRight * 800.0f*/, CHUNK_LEFT_STRUT );

	// Left gun
	pChunks[CHUNK_LEFT_GUN] = CHueyChunk::CreateHelicopterChunk( this, vecChunkPos, vecChunkAngles, pChopper->GetAbsVelocity() /*+ vecRight * 800.0f*/, CHUNK_LEFT_GUN );

	// Constrain all the pieces together loosely
//	IPhysicsObject *pMasterPhysObj = pMasterChunk->VPhysicsGetObject();
//	Assert( pMasterPhysObj );

	IPhysicsConstraintGroup *pGroup = NULL;

	for ( int i = 0; i < CHUNK_MAX; i++ )
	{
#if 1
		if ( pChunks[i] == NULL || s_ChunkConstraint[i] == CHUNK_MAX )
			continue;

		IPhysicsObject *pPhysObj = pChunks[i]->VPhysicsGetObject();
		Assert( pPhysObj );
		if ( pPhysObj == NULL )
			continue;

		pMasterChunk = pChunks[ s_ChunkConstraint[i] ];
		if ( pMasterChunk == NULL )
			continue;

		IPhysicsObject *pMasterPhysObj = pMasterChunk->VPhysicsGetObject();
		Assert( pMasterPhysObj );
		if ( pMasterPhysObj == NULL )
			continue;

		// Create the constraint
		constraint_fixedparams_t fixed;
		fixed.Defaults();
		fixed.InitWithCurrentObjectState( pMasterPhysObj, pPhysObj );
		fixed.constraint.Defaults();

		pMasterChunk->m_pConstraints[i] = physenv->CreateFixedConstraint( pMasterPhysObj, pPhysObj, pGroup, fixed );
		pChunks[i]->m_hMaster = pMasterChunk;
#else
		if ( pChunks[i] == NULL || pChunks[i] == pMasterChunk )
			continue;

		IPhysicsObject *pPhysObj = pChunks[i]->VPhysicsGetObject();
		Assert( pPhysObj );
		if ( pPhysObj == NULL )
			continue;

		// Create the constraint
		constraint_fixedparams_t fixed;
		fixed.Defaults();
		fixed.InitWithCurrentObjectState( pMasterPhysObj, pPhysObj );
		fixed.constraint.Defaults();

		pMasterChunk->m_pConstraints[i] = physenv->CreateFixedConstraint( pMasterPhysObj, pPhysObj, pGroup, fixed );
		pChunks[i]->m_hMaster = pMasterChunk;
#endif
	}
}

//-----------------------------------------------------------------------------
CBaseEntity *CNPC_Huey::FindCrashPoint( void )
{
	CBaseEntity *pCrashPoint = gEntList.FindEntityByClassname( NULL, "info_target_helicopter_crash" );
	if ( pCrashPoint == NULL )
	{
		if ( IsOnPathTrack() )
		{
			pCrashPoint = GetCurrentPathTarget();
			m_flCrashTime = gpGlobals->curtime + 3.0f;
		}
	}
	return pCrashPoint;
}

//-----------------------------------------------------------------------------
void CNPC_Huey::Event_Killed( const CTakeDamageInfo &info )
{
	DetachRappellers();

	if ( GetCrashPoint() == NULL )
	{
		CBaseEntity *pCrashPoint = FindCrashPoint();
		if ( pCrashPoint != NULL )
		{
			m_hCrashPoint.Set( pCrashPoint );
			SetDesiredPosition( pCrashPoint->GetAbsOrigin() );
#if 1
			CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
			controller.SoundChangePitch( m_pRotorSound, 120, 2 );
#else
			// Start the failing engine sound
			CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
			controller.SoundDestroy( m_pRotorSound );

			CPASAttenuationFilter filter( this );
			m_pRotorSound = controller.SoundCreate( filter, entindex(), "NPC_AttackHelicopter.EngineFailure" );
			controller.Play( m_pRotorSound, 1.0, 100 );
#endif

			CFireTrail *pFireTrail = CFireTrail::CreateFireTrail();
			if ( pFireTrail != NULL )
			{
				pFireTrail->FollowEntity( this, "exhaust" );
				pFireTrail->SetParent( this/*, 1*/ );
				pFireTrail->SetLocalOrigin( vec3_origin );
				pFireTrail->SetMoveType( MOVETYPE_NONE );
				pFireTrail->SetLifetime( 20.0f );
			}

			// Tailspin!!
			SetActivity( ACT_HUEY_CRASHING );
			SetPlaybackRate( 1.3f );

			m_bAllowDamageMovement = false;

			// Intentionally returning with m_lifeState set to LIFE_DYING
			m_lifeState = LIFE_DYING;
			return;
		}
	}

	if ( m_hPilot )
		UTIL_Remove( m_hPilot );
	if ( m_hCopilot )
		UTIL_Remove( m_hCopilot );

#if 1
	EmitSound( "BaseExplosionEffect.Sound" );
	for ( int i = 0; i < 4; i++ )
	{
		Chopper_CreateChunk( this, GetAbsOrigin() + Vector(0,0,-64) + RandomVector(-64, 64), RandomAngle(0, 360), g_PropDataSystem.GetRandomChunkModel( "MetalChunks" ), false );
	}

	BecomeChunks();
	StopLoopingSounds();

	m_lifeState = LIFE_DEAD;

//	EmitSound( "NPC_CombineGunship.Explode" );

	SetThink( &CNPC_Huey::SUB_Remove );
	SetNextThink( gpGlobals->curtime + 0.1f );

	AddEffects( EF_NODRAW );

	// Makes the slower rotors fade back in
	SetStartupTime( gpGlobals->curtime + 99.0f );

	m_iHealth = 0;
	m_takedamage = DAMAGE_NO;

	m_OnDeath.FireOutput( info.GetAttacker(), this );
#elif 0
	SetCycle( 0 );
	ResetSequenceInfo( );
	m_flPlaybackRate = 0;

	// FIXME: until huey has physics debris don't collide with any entities just the world and static stuff.
	SetCollisionGroup( COLLISION_GROUP_DEBRIS );

	// This is to ensure the huey dies if it gets stuck on non-SOLID_BSP stuff (see CrashTouch)
	m_flTimeToDie = gpGlobals->curtime + 15.0f;

	// Crash if we stop moving (huey will hit palm trees)
	m_vecVelocityLastThink = vec3_invalid;

	BaseClass::Event_Killed( info );
#elif 0
	StopLoopingSounds();

	m_lifeState = LIFE_DEAD;

	SetThink( &CNPC_Huey::CallNPCThink );
	SetNextThink( gpGlobals->curtime + 0.1f );

	// CAI_BaseNPC::SelectSchedule will choose SCHED_FALL_TO_GROUND for us
	SetCondition( COND_FLOATING_OFF_GROUND );
	SetGravity( 1.0 );

	m_iHealth = 0;
	m_takedamage = DAMAGE_NO;

	m_OnDeath.FireOutput( info.GetAttacker(), this );
#endif
}

//-----------------------------------------------------------------------------
void CNPC_Huey::DyingThink( void )
{
	// If we hit non-world stuff and aren't falling anymore then die
	if ( m_vecVelocityLastThink == vec3_origin && GetAbsVelocity() == vec3_origin )
	{
		m_flTimeToDie = gpGlobals->curtime;
	}
	m_vecVelocityLastThink = GetAbsVelocity();

	if ( m_flTimeToDie <= gpGlobals->curtime )
	{
		CBroadcastRecipientFilter filter;
		Vector v;

		// fireball
		v = WorldSpaceCenter() + Vector(0,0,256);
		te->Sprite( filter, 0.0, &v, m_iFireballSprite, 120, 255 );

		// big smoke
		v = WorldSpaceCenter() + Vector(0,0,512);
		te->Smoke( filter, 0.0, &v, m_iSmokeSprite, 250, 5 );

		// blast circle
		te->BeamRingPoint( filter, 0.0, // FIXME: want TE_BEAMCYLINDER
			GetAbsOrigin(),						//origin
			0,									//start radius
			2000,								//end radius
			m_iBeamRingEffectIndex,				//texture
			0,									//halo index
			0,									//start frame
			0,									//framerate
			1.0,								//life
			32,									//width
			0,									//spread
			0,									//amplitude
			255,								//r
			255,								//g
			255,								//b
			50,									//a
			0,									//speed
			FBEAM_FADEOUT
			);

		EmitSound( "NPC_Huey.Explode" );

		CTakeDamageInfo dmgInfo( this, this, 300, DMG_BLAST );
		RadiusDamage( dmgInfo, GetAbsOrigin(), 300 * 2.5 /* HL1 */, CLASS_NONE, NULL );

		// gibs
		v = WorldSpaceCenter() + Vector(0,0,64);
		te->BreakModel( filter, 0.0, v, vec3_angle, Vector(400,400,128), Vector(0,0,400),
			m_iBodyGibs, 30, 200, 10, BREAK_METAL );

		SetTouch( NULL );
		SetThink( &CNPC_Huey::SUB_Remove );
		SetNextThink( gpGlobals->curtime );

		return;
	}

	BaseClass::DyingThink();

	// see CNPC_Manhack::MoveExecute_Dead
	Vector newVelocity = GetAbsVelocity();
	newVelocity.z -= 0.1 * sv_gravity.GetFloat();
	SetAbsVelocity( newVelocity );

	// see CNPC_CombineGunship::SelfDestruct
	Vector vecOrigin = GetAbsOrigin();
	Vector vecDelta = RandomVector( -150, 150 );
	ExplosionCreate( vecOrigin + vecDelta, QAngle( -90, 0, 0 ), this, 10, 10, false );
}

//-----------------------------------------------------------------------------
void CNPC_Huey::CrashTouch( CBaseEntity *pOther )
{
	// Die now if huey hit the world
	if ( pOther->GetSolid() == SOLID_BSP ) 
	{
		m_flTimeToDie = gpGlobals->curtime;
		SetNextThink( gpGlobals->curtime );
	}
}

//-----------------------------------------------------------------------------
void CNPC_Huey::StartTask( const Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_FALL_TO_GROUND:
		DbgHueyMsg("CNPC_Huey::StartTask TASK_FALL_TO_GROUND\n");
		BaseClass::StartTask( pTask );
		break;
	default:
		BaseClass::StartTask( pTask );
		break;
	}
}

//-----------------------------------------------------------------------------
void CNPC_Huey::RunTask( const Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_FALL_TO_GROUND:
		DbgHueyMsg("CNPC_Huey::RunTask TASK_FALL_TO_GROUND\n");
		BaseClass::RunTask( pTask );
		break;
	default:
		BaseClass::RunTask( pTask );
		break;
	}
}

//---------------------------------------------------------
void CNPC_Huey::StartSmoking( void )
{
	if ( m_hSmoke != NULL )
		return;

	m_hSmoke = SmokeTrail::CreateSmokeTrail();
	
	if ( m_hSmoke )
	{
		m_hSmoke->m_SpawnRate			= 32;
		m_hSmoke->m_ParticleLifetime	= 3.0;
		m_hSmoke->m_StartSize			= 16;
		m_hSmoke->m_EndSize				= 64;
		m_hSmoke->m_SpawnRadius			= 20;
		m_hSmoke->m_MinSpeed			= 8;
		m_hSmoke->m_MaxSpeed			= 64;
		m_hSmoke->m_Opacity 			= 0.3;
		
		m_hSmoke->m_StartColor.Init( 0.25f, 0.25f, 0.25f );
		m_hSmoke->m_EndColor.Init( 0, 0, 0 );
		m_hSmoke->SetLifetime( 500.0f );
		m_hSmoke->FollowEntity( this, "exhaust" );
	}
}

//-----------------------------------------------------------------------------
int CNPC_Huey::DrawDebugTextOverlays(void)
{
	int text_offset = 0;

	text_offset = BaseClass::DrawDebugTextOverlays();

//	if (m_debugOverlays & OVERLAY_TEXT_BIT)
	{
		CFmtStr fmt;
		EntityText(text_offset++, fmt.sprintf("Angles: x %.2f", (float)GetAbsAngles().x), 0);

		Msg("Huey velocity %.2f\n", GetAbsVelocity().Length() );

		if ( HasSpawnFlags( SF_PASSENGER ) && m_hPassengerEnemy )
		{
			if ( m_PassengerShotRegulator.IsInRestInterval() )
				EntityText(text_offset++, "Psgr: Rest interval", 0);
			else if ( m_angPassenger.y > -30 && m_angPassenger.y < 30 )
				EntityText(text_offset++, "Psgr: Angles", 0);
		}
	}

	return text_offset;
}

#ifdef RAPPEL_PHYSICS

//-----------------------------------------------------------------------------
class CHueyRappelPhysicsTip : public CBaseAnimating
{
	DECLARE_CLASS( CHueyRappelPhysicsTip, CBaseAnimating );

public:
	DECLARE_DATADESC();

	virtual void Spawn( void );
	virtual void Precache( void );
//	virtual void UpdateOnRemove( );
	virtual void VPhysicsUpdate( IPhysicsObject *pPhysics );

//	virtual int	UpdateTransmitState( void );
//	bool						CreateSpring( CBaseAnimating *pTongueRoot );
//	static CBarnacleTongueTip	*CreateTongueTip( CNPC_Barnacle *pBarnacle, CBaseAnimating *pTongueRoot, const Vector &vecOrigin, const QAngle &vecAngles );
//	static CBarnacleTongueTip	*CreateTongueRoot( const Vector &vecOrigin, const QAngle &vecAngles );

//	IPhysicsSpring			*m_pSpring;

//	CHandle<CNPC_Barnacle>	m_hBarnacle;
	CHandle<CAI_BaseNPC>	m_hNPC;
};

BEGIN_DATADESC( CHueyRappelPhysicsTip )
//	DEFINE_FIELD( m_hBarnacle, FIELD_EHANDLE ),
//	DEFINE_PHYSPTR( m_pSpring ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( huey_rappel_physics_tip, CHueyRappelPhysicsTip );

//-----------------------------------------------------------------------------
void CHueyRappelPhysicsTip::Spawn( void )
{
	Precache();
	SetModel( "models/props_junk/rock001a.mdl" );
//	AddEffects( EF_NODRAW );

	// We don't want this to be solid, because we don't want it to collide with the barnacle.
	SetSolid( SOLID_VPHYSICS );
	AddSolidFlags( FSOLID_NOT_SOLID );
	BaseClass::Spawn();

//	m_pSpring = NULL;
}

//-----------------------------------------------------------------------------
void CHueyRappelPhysicsTip::Precache( void )
{
	PrecacheModel( "models/props_junk/rock001a.mdl" );
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
void CHueyRappelPhysicsTip::VPhysicsUpdate( IPhysicsObject *pPhysics )
{
	BaseClass::VPhysicsUpdate( pPhysics );

	if ( m_hNPC )
	{
		m_hNPC->SetAbsOrigin( GetAbsOrigin() );
		m_hNPC->SetAbsVelocity( vec3_origin );
	}
}

//-----------------------------------------------------------------------------
class CHueyRappelPhysicsBase : public CBaseAnimating
{
	DECLARE_CLASS( CHueyRappelPhysicsBase, CBaseAnimating );

public:
	DECLARE_DATADESC();

	virtual void Spawn( void );
	virtual void Precache( void );
//	virtual void UpdateOnRemove( );
//	virtual void VPhysicsUpdate( IPhysicsObject *pPhysics );

//	virtual int	UpdateTransmitState( void );
//	bool						CreateSpring( CBaseAnimating *pTongueRoot );
//	static CBarnacleTongueTip	*CreateTongueTip( CNPC_Barnacle *pBarnacle, CBaseAnimating *pTongueRoot, const Vector &vecOrigin, const QAngle &vecAngles );
//	static CBarnacleTongueTip	*CreateTongueRoot( const Vector &vecOrigin, const QAngle &vecAngles );

//	IPhysicsSpring			*m_pSpring;

private:
//	CHandle<CNPC_Barnacle>	m_hBarnacle;
};

BEGIN_DATADESC( CHueyRappelPhysicsBase )
//	DEFINE_FIELD( m_hBarnacle, FIELD_EHANDLE ),
//	DEFINE_PHYSPTR( m_pSpring ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( huey_rappel_physics_base, CHueyRappelPhysicsBase );

//-----------------------------------------------------------------------------
void CHueyRappelPhysicsBase::Spawn( void )
{
	Precache();
	SetModel( "models/props_junk/rock001a.mdl" );
//	AddEffects( EF_NODRAW );

	// We don't want this to be solid, because we don't want it to collide with the barnacle.
	SetSolid( SOLID_VPHYSICS );
	AddSolidFlags( FSOLID_NOT_SOLID );
	BaseClass::Spawn();

	// This will be parented to the huey
	VPhysicsInitShadow( false, false );
	SetMoveType( MOVETYPE_NONE );

//	m_pSpring = NULL;
}

//-----------------------------------------------------------------------------
void CHueyRappelPhysicsBase::Precache( void )
{
	PrecacheModel( "models/props_junk/rock001a.mdl" );
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
void CNPC_Huey::CreateRappelPhysics( int slot, CAI_BaseNPC *pNPC )
{
	if ( !pNPC )
		return;

	if ( m_pRappelPhysics[slot] )
	{
		 physenv->DestroySpring( m_pRappelPhysics[slot] );
		 m_pRappelPhysics[slot] = NULL;
	}

#if 1
	CBaseEntity *pRootEnt = this;
	IPhysicsObject *pRootPhysObject = pRootEnt->VPhysicsGetObject();
	Assert( pRootPhysObject );

	CHueyRappelPhysicsTip *pTipEnt = (CHueyRappelPhysicsTip *) CBaseEntity::Create( "huey_rappel_physics_tip", pNPC->GetAbsOrigin(), pNPC->GetAbsAngles() );
	Assert( pTipEnt );
	pTipEnt->VPhysicsInitNormal( pTipEnt->GetSolid(), pTipEnt->GetSolidFlags(), false );
	IPhysicsObject *pTipPhysObject = pTipEnt->VPhysicsGetObject();
	Assert( pTipPhysObject );
	pTipPhysObject->SetMass( 100 );
	float dampingSpeed = 3;
	float dampingRotation = 10;
	pTipPhysObject->SetDamping( &dampingSpeed, &dampingRotation );
//	pTipPhysObject->EnableMotion( true );
//	pTipPhysObject->EnableGravity( true );
	pTipPhysObject->SetCallbackFlags( pTipPhysObject->GetCallbackFlags() & (~CALLBACK_DO_FLUID_SIMULATION) );

	pTipEnt->m_hNPC = pNPC;
#else
	CBaseEntity *pRootEnt = CBaseEntity::Create( "huey_rappel_physics_base", GetAbsOrigin(), vec3_angle );
	Assert( pRootEnt );
	pRootEnt->SetParent( this );
	IPhysicsObject *pRootPhysObject = pRootEnt->VPhysicsGetObject();
	Assert( pRootPhysObject );
	pRootPhysObject->SetMass( VPHYSICS_MAX_MASS );

	CBaseEntity *pTipEnt = CBaseEntity::Create( "huey_rappel_physics_tip", pNPC->GetAbsOrigin(), vec3_angle );
	Assert( pTipEnt );
	pTipEnt->VPhysicsInitNormal( pTipEnt->GetSolid(), pTipEnt->GetSolidFlags(), false );
	IPhysicsObject *pTipPhysObject = pTipEnt->VPhysicsGetObject();
	Assert( pTipPhysObject );
	pTipPhysObject->SetMass( 100 );
	float damping = 3;
	pTipPhysObject->SetDamping( &damping, &damping );
//	pTipPhysObject->EnableMotion( true );
//	pTipPhysObject->EnableGravity( true );
	pTipPhysObject->SetCallbackFlags( pTipPhysObject->GetCallbackFlags() & (~CALLBACK_DO_FLUID_SIMULATION) );
#endif

	springparams_t spring;
	spring.constant = 10000.0f;
	spring.damping = 2.0f;
	spring.naturalLength = 256;
	spring.relativeDamping = 10;
	spring.startPosition = pRootEnt->GetAbsOrigin();
	spring.endPosition = pTipEnt->GetAbsOrigin();
	spring.useLocalPositions = false;
	m_pRappelPhysics[slot] = physenv->CreateSpring( pRootPhysObject, pTipPhysObject, &spring );

	if ( m_pRappelPhysics[slot] )
	{
//		pNPC->SetMoveType( MOVETYPE_VPHYSICS );
	}
}

#endif // RAPPEL_PHYSICS

AI_BEGIN_CUSTOM_NPC( npc_huey, CNPC_Huey )
	DECLARE_ACTIVITY( ACT_HUEY_CRASHING );
AI_END_CUSTOM_NPC()




LINK_ENTITY_TO_CLASS( huey_chunk, CHueyChunk );

BEGIN_DATADESC( CHueyChunk )

	DEFINE_THINKFUNC( FallThink ),
	DEFINE_THINKFUNC( SparkThink ),
	DEFINE_THINKFUNC( ExplodeThink ),

	DEFINE_FIELD( m_bLanded, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flStartTime, FIELD_TIME ),
	DEFINE_FIELD( m_hMaster, FIELD_EHANDLE ),
	DEFINE_FIELD( m_nChunkID, FIELD_INTEGER ),
	DEFINE_PHYSPTR_ARRAY( m_pConstraints ),

END_DATADESC()

//-----------------------------------------------------------------------------
void CHueyChunk::Spawn( void )
{
	BaseClass::Spawn();

	m_flStartTime = gpGlobals->curtime;
}

//-----------------------------------------------------------------------------
void CHueyChunk::Precache( void )
{
	BaseClass::Precache();

	for ( int i = 0; i < CHUNK_MAX; i++ )
	{
		PrecacheModel( s_ChunkModels[i] );
	}

	PrecacheScriptSound( "DoSpark" );
	PrecacheScriptSound( "NPC_AttackHelicopter.Crash" );
	PrecacheScriptSound( "NPC_Huey.Explode" );
}

//-----------------------------------------------------------------------------
void CHueyChunk::FallThink( void )
{
	if ( m_bLanded )
	{
		m_flStartTime = gpGlobals->curtime;
		SetThink( &CHueyChunk::ExplodeThink );
		SetNextThink( gpGlobals->curtime + 1.0f );
		return;
	}

#if 1
	if ( (s_ChunkTime[ m_nChunkID ] > 0) &&
		(gpGlobals->curtime >= m_flStartTime + s_ChunkTime[ m_nChunkID ]) &&
		(m_hMaster != NULL) &&
		(m_hMaster->m_pConstraints[ m_nChunkID ] != NULL) )
	{
		physenv->DestroyConstraint( m_hMaster->m_pConstraints[ m_nChunkID ] );
		m_hMaster->m_pConstraints[ m_nChunkID ] = NULL;
		DbgHueyMsg( "broke constraint between %s and %s\n", s_ChunkModels[m_hMaster->m_nChunkID], s_ChunkModels[m_nChunkID] );

		m_hMaster = NULL;

		CEffectData data;
		data.m_vOrigin = GetAbsOrigin() + RandomVector( -64, 64 );
		DispatchEffect( "HelicopterMegaBomb", data );

		EmitSound( "BaseExplosionEffect.Sound" );

		Vector v;
		v.Random( 0, 360 );
		v.z = abs(v.z);
		v.NormalizeInPlace();
		ApplyAbsVelocityImpulse( v * 200.0f );

		AngularImpulse a;
		a.Random( 0, 360 );
		a.NormalizeInPlace();
		ApplyAbsVelocityImpulse( a * 100.0f );
	}
#else
	if ( random->RandomInt( 0, 8 ) == 0 )
	{
		CEffectData data;
		data.m_vOrigin = GetAbsOrigin() + RandomVector( -64, 64 );
		DispatchEffect( "HelicopterMegaBomb", data );

		EmitSound( "BaseExplosionEffect.Sound" );
	}
#endif
	SetNextThink( gpGlobals->curtime + 0.1f );
}

#define HUEY_CHUNK_THINK_CONTEXT_SPARKS "HueyChunkThinkContextSparks"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHueyChunk::SparkThink( void )
{
	Vector vecOrigin, vecDir;
	QAngle angles;
	
	if ( GetAttachment( "sparks", vecOrigin, angles ) == false )
	{
		SetContextThink( &CHueyChunk::SparkThink,
				TICK_NEVER_THINK,
				HUEY_CHUNK_THINK_CONTEXT_SPARKS );
		return;
	}

	AngleVectors( angles, &vecDir );

	int nMagnitude = 1;
	int nTrailLength = 1;
	g_pEffects->Sparks( vecOrigin, nMagnitude, nTrailLength, &vecDir );

	EmitSound( "DoSpark" );

	SetContextThink( &CHueyChunk::SparkThink,
		gpGlobals->curtime + 0.1 + random->RandomFloat( 0, 2.0 ),
		HUEY_CHUNK_THINK_CONTEXT_SPARKS );
}

//-----------------------------------------------------------------------------
void CHueyChunk::ExplodeThink( void )
{
	IPhysicsObject *pPhysObj = VPhysicsGetObject();
	if ( (pPhysObj && pPhysObj->IsAsleep()) || (m_flStartTime < gpGlobals->curtime - 5.0f) )
	{
		EmitSound( "BaseExplosionEffect.Sound" );
		if ( m_nChunkID == CHUNK_ENGINE )
		{
			EmitSound( "NPC_Huey.Explode" );

//			CTakeDamageInfo dmgInfo( this, this, 300, DMG_BLAST );
//			RadiusDamage( dmgInfo, GetAbsOrigin(), 300 * 2.5 /* HL1 */, CLASS_NONE, NULL );
		}

		for ( int i = 0; i < s_ChunkChunks[ m_nChunkID ]; i++ )
		{
			Chopper_CreateChunk( this, GetAbsOrigin(), RandomAngle(0, 360), g_PropDataSystem.GetRandomChunkModel( "MetalChunks" ), false );
		}
		RemoveDeferred();

		return;
	}
	SetNextThink( gpGlobals->curtime + 1.0f );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : index - 
//			*pEvent - 
//-----------------------------------------------------------------------------
void CHueyChunk::VPhysicsCollision( int index, gamevcollisionevent_t *pEvent )
{
	BaseClass::VPhysicsCollision( index, pEvent );

	if ( m_bLanded == false )
	{
		int otherIndex = !index;
		CBaseEntity *pOther = pEvent->pEntities[otherIndex];
		if ( !pOther )
			return;
		
		if ( pOther->IsWorld() )
		{		
			CollisionCallback( this );

			m_bLanded = true;
//			SetThink( NULL );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pCaller - 
//-----------------------------------------------------------------------------
void CHueyChunk::CollisionCallback( CHueyChunk *pCaller )
{
	if ( m_bLanded )
		return;

	if ( m_hMaster != NULL )
	{
		m_hMaster->CollisionCallback( this );
	}
//	else
	{
		// Break our other constraints
		for ( int i = 0; i < ARRAYSIZE( m_pConstraints ); i++ )
		{
			if ( m_pConstraints[i] )
			{
				physenv->DestroyConstraint( m_pConstraints[i] );
				m_pConstraints[i] = NULL;
				DbgHueyMsg( "broke constraint between %s and child\n", s_ChunkModels[m_nChunkID] );
			}
		}
		
		if ( m_nChunkID == CHUNK_ENGINE )
		{
			// Add a dust cloud
			AR2Explosion *pExplosion = AR2Explosion::CreateAR2Explosion( GetAbsOrigin() );
			if ( pExplosion != NULL )
			{
				pExplosion->SetLifetime( 10 );
			}

			// Make a loud noise
			EmitSound( "NPC_AttackHelicopter.Crash" );
		}

		m_bLanded = true;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &vecPos - 
//			&vecAngles - 
//			&vecVelocity - 
//			*pszModelName - 
// Output : CHueyChunk
//-----------------------------------------------------------------------------
CHueyChunk *CHueyChunk::CreateHelicopterChunk( CBaseEntity *pHuey, const Vector &vecPos, const QAngle &vecAngles, const Vector &vecVelocity, HueyChunkID chunkID )
{
	// Drop a flaming, smoking chunk.
	CHueyChunk *pChunk = CREATE_ENTITY( CHueyChunk, "huey_chunk" );
	
	if ( pChunk == NULL )
		return NULL;

	pChunk->Spawn();

	pChunk->SetModel( s_ChunkModels[chunkID] );

	Vector vecOffset( 0, 0, 0 );
	Vector vecChunkOrigin = vecPos;
	KeyValues *modelKeyValues = new KeyValues("");
	const char *modelName = modelinfo->GetModelName( pChunk->GetModel() );
	const char *kvText = modelinfo->GetModelKeyValueText( pChunk->GetModel() ) ;
	if ( modelKeyValues->LoadFromBuffer( modelName, kvText ) )
	{
		KeyValues *pSection = modelKeyValues->FindKey( "huey_chunk" );
		if ( pSection )
		{
			const char *s = pSection->GetString( "offset", "0 0 0" );
			UTIL_StringToVector( vecOffset.Base(), s );
//			vec_t z = vecOffset.z;
//			vecOffset.z = vecOffset.y;
//			vecOffset.y = z;

			Vector forward, right, up;
			//pHuey->GetVectors( &forward, &right, &up );
			AngleVectors( vecAngles, &forward, &right, &up );
			vecChunkOrigin = vecPos + forward * vecOffset.z + right * (-vecOffset.x) + up * vecOffset.y;
		}
	}
	modelKeyValues->deleteThis();

	pChunk->SetAbsOrigin( vecChunkOrigin );
	pChunk->SetAbsAngles( vecAngles );

	pChunk->m_nChunkID = chunkID;
//	pChunk->SetCollisionGroup( COLLISION_GROUP_INTERACTIVE );

	IPhysicsObject *pPhysicsObject = pChunk->VPhysicsInitNormal( SOLID_VPHYSICS, pChunk->GetSolidFlags(), false );
	
	// Set the velocity
	if ( pPhysicsObject )
	{
		pPhysicsObject->EnableMotion( true );
		Vector vecChunkVelocity;
		AngularImpulse angImpulse;

		vecChunkVelocity = vecVelocity;
		angImpulse = vec3_origin;

		pPhysicsObject->SetVelocity(&vecChunkVelocity, &angImpulse );
	}
	
	pChunk->SetThink( &CHueyChunk::FallThink );
	pChunk->SetNextThink( gpGlobals->curtime + 0.1f );

	pChunk->m_bLanded = false;

	// Sparks
	int iAttach = pChunk->LookupAttachment( "sparks" );
	if ( iAttach != INVALID_ATTACHMENT )
	{
		pChunk->SetContextThink( &CHueyChunk::SparkThink,
			gpGlobals->curtime + 0.1 + random->RandomFloat( 0, 2.0 ),
			HUEY_CHUNK_THINK_CONTEXT_SPARKS );
	}

	iAttach = pChunk->LookupAttachment( "damage" );
	if ( iAttach == INVALID_ATTACHMENT )
		return pChunk;

	SmokeTrail *pSmokeTrail =  SmokeTrail::CreateSmokeTrail();
	pSmokeTrail->FollowEntity( pChunk, "damage" );

	pSmokeTrail->m_SpawnRate = 4;
	pSmokeTrail->m_ParticleLifetime	= 2.0f;

	pSmokeTrail->m_StartColor.Init( 0.7f, 0.7f, 0.7f );
	pSmokeTrail->m_EndColor.Init( 0.6, 0.6, 0.6 );

	pSmokeTrail->m_StartSize	= 32;
	pSmokeTrail->m_EndSize	= 64;
	pSmokeTrail->m_SpawnRadius= 8;
	pSmokeTrail->m_MinSpeed	= 0;
	pSmokeTrail->m_MaxSpeed	= 8;
	pSmokeTrail->m_Opacity	= 0.35f;
	pSmokeTrail->m_StopEmitTime = gpGlobals->curtime + 30.0f;

	CFireTrail *pFireTrail = CFireTrail::CreateFireTrail();

	if ( pFireTrail == NULL )
		return pChunk;

	pFireTrail->FollowEntity( pChunk, "damage" );
	pFireTrail->SetParent( pChunk, 1 );
	pFireTrail->SetLocalOrigin( vec3_origin );
	pFireTrail->SetMoveType( MOVETYPE_NONE );
	pFireTrail->SetLifetime( 20.0f );

	return pChunk;
}
