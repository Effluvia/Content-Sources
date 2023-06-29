#include "cbase.h"
#include "ai_basenpc.h"
#include "npcevent.h"
#include "movevars_shared.h" // sv_gravity
#include "npc_bullseye.h"
#include "ai_navigator.h"
#include "hoe_deathsound.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define KOPHY_BITE

const int KOPHYAEGER_MIN_JUMP_DIST = 48;
const int KOPHYAEGER_MAX_JUMP_DIST = 256;

class CNPC_Kophyaeger : public CAI_BaseNPC
{
public:
	DECLARE_CLASS( CNPC_Kophyaeger, CAI_BaseNPC );

	CNPC_Kophyaeger()
	{
		m_bAdult = false;
	}

	void Spawn( void );
	void Precache( void );

	Class_T Classify( void );
	bool ClassifyPlayerAlly( void ) { return false; }
	bool ClassifyPlayerAllyVital( void ) { return false; }

	float MaxYawSpeed( void );

	void HandleAnimEvent( animevent_t *pEvent );

	DEFINE_CUSTOM_AI;
	DECLARE_DATADESC();

	void AlertSound( void );
	void AttackSound( void );
	void IdleSound( void );
	void PainSound( const CTakeDamageInfo &info );
	void DeathSound( const CTakeDamageInfo &info );

	void AttackHitSound( void );
	void AttackMissSound( void );
	void BiteSound( void );
	void ImpactSound( void );

	int GetSoundInterests( void );
	void RemoveIgnoredConditions( void );

	int SelectSchedule( void );
	int SelectFailSchedule( int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode );
	int TranslateSchedule( int scheduleType );
	void StartTask( const Task_t *pTask );
	void RunTask( const Task_t *pTask );

	virtual float InnateRange1MinRange( void ) { return KOPHYAEGER_MIN_JUMP_DIST * (m_bAdult ? 2.0f : 1.0f); }
	virtual float InnateRange1MaxRange( void ) { return KOPHYAEGER_MAX_JUMP_DIST * (m_bAdult ? 1.5f : 1.0f); }
	virtual bool InnateWeaponLOSCondition( const Vector &ownerPos, const Vector &targetPos, bool bSetConditions );

	int CountJumpersAtTarget( CBaseEntity *pVictim );
	int RangeAttack1Conditions( float flDot, float flDist );

	void JumpAttack( bool bRandomJump, const Vector &vecPos = vec3_origin );
	void MoveOrigin( const Vector &vecDelta );
	void Leap( const Vector &vecVel );
	void TouchDamage( CBaseEntity *pOther );
	void ThrowThink( void );
	bool HasHeadroom();
	void LeapTouch( CBaseEntity *pOther );
	int CalcDamageInfo( CTakeDamageInfo *pInfo );

#ifdef KOPHY_BITE
	int MeleeAttack1Conditions( float flDot, float flDist );
#endif

	bool	m_bAdult;
	bool	m_bCommittedToJump;		// Whether we have 'locked in' to jump at our enemy.
	bool	m_bMidJump;
	bool	m_bAttackFailed;		// whether we ran into a wall during a jump.
	Vector	m_vecCommittedJumpPos;	// The position of our enemy when we locked in our jump attack.
	float	m_flNextNPCThink;
	float	m_flIgnoreWorldCollisionTime;
	float	m_flHungryTime;
	float	m_flPainSoundTime;

	//-----------------------------------------------------------------------------
	// Custom schedules.
	//-----------------------------------------------------------------------------
	enum
	{
		SCHED_KOPHYAEGER_RANGE_ATTACK1 = BaseClass::NEXT_SCHEDULE,
		SCHED_KOPHYAEGER_EAT,
		SCHED_KOPHYAEGER_SNIFF_AND_EAT,
		NEXT_SCHEDULE,
	};

	//-----------------------------------------------------------------------------
	// Tasks
	//-----------------------------------------------------------------------------
	enum
	{
		TASK_KOPHYAEGER_EAT = BaseClass::NEXT_TASK,
		NEXT_TASK,
	};

	//-----------------------------------------------------------------------------
	// Activities
	//-----------------------------------------------------------------------------
	static Activity ACT_KOPHYAEGER_DETECT_SCENT;
	static Activity ACT_KOPHYAEGER_EAT;
	static Activity ACT_KOPHYAEGER_THREAT_DISPLAY;
};

LINK_ENTITY_TO_CLASS( npc_kophyaeger, CNPC_Kophyaeger );

class CNPC_AdultKophyaeger : public CNPC_Kophyaeger
{
public:
	DECLARE_CLASS( CNPC_Kophyaeger, CNPC_Kophyaeger );

	CNPC_AdultKophyaeger()
	{
		m_bAdult = true;
	}
};

LINK_ENTITY_TO_CLASS( npc_kophyaeger_adult, CNPC_AdultKophyaeger );

//-----------------------------------------------------------------------------
// Spawnflags
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Animation events.
//-----------------------------------------------------------------------------
static int AE_KOPHYAEGER_JUMPATTACK;
#ifdef KOPHY_BITE
static int AE_KOPHYAEGER_BITE;
#endif

//-----------------------------------------------------------------------------
// Tasks
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Conditions 
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Activities
//-----------------------------------------------------------------------------
Activity CNPC_Kophyaeger::ACT_KOPHYAEGER_DETECT_SCENT;
Activity CNPC_Kophyaeger::ACT_KOPHYAEGER_EAT;
Activity CNPC_Kophyaeger::ACT_KOPHYAEGER_THREAT_DISPLAY;

BEGIN_DATADESC( CNPC_Kophyaeger )
	DEFINE_FIELD( m_bCommittedToJump, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bMidJump, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_vecCommittedJumpPos, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( m_flNextNPCThink, FIELD_TIME ),
	DEFINE_FIELD( m_bAdult, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flHungryTime, FIELD_TIME ),
	DEFINE_FIELD( m_flPainSoundTime, FIELD_TIME ),

	DEFINE_THINKFUNC( ThrowThink ),
	DEFINE_ENTITYFUNC( LeapTouch ),
END_DATADESC()

#define KOPHYAEGER_HEALTH 25 // 20, 25, 30
#define KOPHYAEGER_IGNORE_WORLD_COLLISION_TIME 0.5
#define KOPHYAEGER_DMG_BITE 10 // 5, 10, 15

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Kophyaeger::Spawn( void )
{
	BaseClass::Spawn();

	Precache();
	SetModel( STRING( GetModelName() ) );

	SetHullType( m_bAdult ? HULL_MEDIUM_TALL : HULL_TINY );
	SetHullSizeNormal();

	// Lower the top of the box so we can jump under low ceilings
	if ( m_bAdult )
		UTIL_SetSize( this, GetHullMins(), GetHullMaxs() - Vector(0,0,12));

	SetSolid( SOLID_BBOX );
	AddSolidFlags( FSOLID_NOT_STANDABLE ); // you can't stand on this
	SetMoveType( MOVETYPE_STEP );

	CapabilitiesClear();
	CapabilitiesAdd( bits_CAP_MOVE_GROUND | bits_CAP_INNATE_RANGE_ATTACK1 );
#ifdef KOPHY_BITE
	if ( m_bAdult )
		CapabilitiesAdd( bits_CAP_INNATE_MELEE_ATTACK1 );
#endif

	SetBloodColor( BLOOD_COLOR_GREEN );
	m_NPCState = NPC_STATE_NONE;

	m_flFieldOfView = 0.5;

	m_iHealth = KOPHYAEGER_HEALTH * ( m_bAdult ? 3 : 1 );

	NPCInit();

	SetViewOffset( Vector ( 0, 0, m_bAdult ? 72 : 26 ) );
}

//-----------------------------------------------------------------------------
// Purpose: Precaches all resources this monster needs.
//-----------------------------------------------------------------------------
void CNPC_Kophyaeger::Precache( void )
{
	const char *modelName = m_bAdult ? "models/kophadult.mdl" : "models/kophyaeger.mdl";
	SetModelName( AllocPooledString( modelName ) );
	PrecacheModel( STRING( GetModelName() ) );

	PrecacheScriptSound("NPC_Kophyaeger.Alert");
	PrecacheScriptSound("NPC_Kophyaeger.Attack");
	PrecacheScriptSound("NPC_Kophyaeger.Bite");
	PrecacheScriptSound("NPC_Kophyaeger.Idle");
	PrecacheScriptSound("NPC_Kophyaeger.Pain");
	PrecacheScriptSound("NPC_Kophyaeger.Death");
	PrecacheScriptSound("NPC_Kophyaeger.Eat");

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Indicates this monster's place in the relationship table.
// Output : 
//-----------------------------------------------------------------------------
Class_T	CNPC_Kophyaeger::Classify( void )
{
	return m_bAdult ? CLASS_KOPHYAEGER_ADULT : CLASS_KOPHYAEGER; 
}

//-----------------------------------------------------------------------------
float CNPC_Kophyaeger::MaxYawSpeed( void )
{
	return 60.0f;
}

//-----------------------------------------------------------------------------
void CNPC_Kophyaeger::HandleAnimEvent( animevent_t *pEvent )
{
	if (pEvent->event == AE_KOPHYAEGER_JUMPATTACK)
	{
		// Ignore if we're in mid air
		if ( m_bMidJump )
			return;

		CBaseEntity *pEnemy = GetEnemy();
			
		if ( pEnemy )
		{
			if ( m_bCommittedToJump )
			{
				JumpAttack( false, m_vecCommittedJumpPos );
			}
			else
			{
				// Jump at my enemy's eyes.
				JumpAttack( false, pEnemy->EyePosition() );
			}

			m_bCommittedToJump = false;
			
		}
		else
		{
			// Jump hop, don't care where.
			JumpAttack( true );
		}
		return;
	}

#ifdef KOPHY_BITE
	if ( pEvent->event == AE_KOPHYAEGER_BITE )
	{
		int iDamage = 15;
		// Reduce damage against humans to avoid mass slaughter
		extern bool HOE_IsHuman( CBaseEntity *pEnt );
		if ( GetEnemy() && HOE_IsHuman( GetEnemy() ) )
			iDamage /= 2;
		CBaseEntity *pHurt = CheckTraceHullAttack( 70, Vector(-16,-16,-16), Vector(16,16,16), iDamage, DMG_SLASH );
		if ( pHurt )
		{
			BiteSound();
			if ( UTIL_ShouldShowBlood(pHurt->BloodColor()) )
			{
				Vector origin;
				QAngle angles;
				GetAttachment( "mouth", origin, angles );
				SpawnBlood( origin, vec3_origin, pHurt->BloodColor(), min( iDamage, 20 ) );
			}
		}
		return;
	}
#endif

	switch( pEvent->event )
	{
	case 1:
	default:
		BaseClass::HandleAnimEvent( pEvent );
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Kophyaeger::ThrowThink( void )
{
	if (gpGlobals->curtime > m_flNextNPCThink)
	{
		NPCThink();
		m_flNextNPCThink = gpGlobals->curtime + 0.1;
	}

	if( GetFlags() & FL_ONGROUND )
	{
		SetThink( &CNPC_Kophyaeger::CallNPCThink );
		SetNextThink( gpGlobals->curtime + 0.1 );
		return;
	}

	SetNextThink( gpGlobals->curtime );
}

//-----------------------------------------------------------------------------
// Purpose: Does a jump attack at the given position.
// Input  : bRandomJump - Just hop in a random direction.
//			vecPos - Position to jump at, ignored if bRandom is set to true.
//			bThrown - 
//-----------------------------------------------------------------------------
void CNPC_Kophyaeger::JumpAttack( bool bRandomJump, const Vector &vecPos )
{
	Vector vecJumpVel;
	if ( !bRandomJump )
	{
#if 1	
		// The old code (taken from headcrab) prevented the NPC jumping down
		CBaseEntity *pBlocker;
		ThrowLimit( GetAbsOrigin(), vecPos - CollisionProp()->OBBCenter(), sv_gravity.GetFloat(),
			16, CollisionProp()->OBBMins(), CollisionProp()->OBBMaxs(), GetEnemy(), &vecJumpVel, &pBlocker );
#else
		float gravity = sv_gravity.GetFloat();
		if ( gravity < 1 )
		{
			gravity = 1;
		}

		// This is the point on the model we want to hit the eyes.
		// The original Headcrab code used GetAbsOrigin() which is the
		// feet of the Kophyaeger.
		Vector vecOrigin = CollisionProp()->WorldSpaceCenter();

		// How fast does the headcrab need to travel to reach the position given gravity?
		float flActualHeight = vecPos.z - vecOrigin.z;
		float height = flActualHeight;
		if ( height < 16 )
		{
			height = 16;
		}
		else
		{
			float flMaxHeight = /*bThrown ? 400 : */120;
			if ( height > flMaxHeight )
			{
				height = flMaxHeight;
			}
		}

		// overshoot the jump by an additional 8 inches
		// NOTE: This calculation jumps at a position INSIDE the box of the enemy (player)
		// so if you make the additional height too high, the crab can land on top of the
		// enemy's head.  If we want to jump high, we'll need to move vecPos to the surface/outside
		// of the enemy's box.
		
		float additionalHeight = 0;
		if ( height < 32 )
		{
			additionalHeight = 8;
		}

		height += additionalHeight;

		// NOTE: This equation here is from vf^2 = vi^2 + 2*a*d
		float speed = sqrt( 2 * gravity * height );
		float time = speed / gravity;

		// add in the time it takes to fall the additional height
		// So the impact takes place on the downward slope at the original height
		time += sqrt( (2 * additionalHeight) / gravity );

		// Scale the sideways velocity to get there at the right time
		VectorSubtract( vecPos, vecOrigin, vecJumpVel );
		vecJumpVel /= time;

		// Speed to offset gravity at the desired height.
		vecJumpVel.z = speed;
#endif
		// Don't jump too far/fast.
		float flJumpSpeed = vecJumpVel.Length();
		float flMaxSpeed = m_bAdult ? 3000.0f : 1000.0f;
		if ( flJumpSpeed > flMaxSpeed )
		{
			vecJumpVel *= flMaxSpeed / flJumpSpeed;
		}
	}
	else
	{
		//
		// Jump hop, don't care where.
		//
		Vector forward, up;
		AngleVectors( GetLocalAngles(), &forward, NULL, &up );
		vecJumpVel = Vector( forward.x, forward.y, up.z ) * 350;
	}

//	AttackSound();
	Leap( vecJumpVel );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_Kophyaeger::MoveOrigin( const Vector &vecDelta )
{
	UTIL_SetOrigin( this, GetLocalOrigin() + vecDelta );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : vecVel - 
//-----------------------------------------------------------------------------
void CNPC_Kophyaeger::Leap( const Vector &vecVel )
{
	SetTouch( &CNPC_Kophyaeger::LeapTouch );

	SetCondition( COND_FLOATING_OFF_GROUND );
	SetGroundEntity( NULL );

	m_flIgnoreWorldCollisionTime = gpGlobals->curtime + KOPHYAEGER_IGNORE_WORLD_COLLISION_TIME;

	if( HasHeadroom() )
	{
		// Take him off ground so engine doesn't instantly reset FL_ONGROUND.
		MoveOrigin( Vector( 0, 0, 1 ) );
	}

	SetAbsVelocity( vecVel );

	// Think every frame so the player sees the headcrab where he actually is...
	m_bMidJump = true;
	SetThink( &CNPC_Kophyaeger::ThrowThink );
	SetNextThink( gpGlobals->curtime );
}

//-----------------------------------------------------------------------------
// Before jumping, headcrabs usually use SetOrigin() to lift themselves off the 
// ground. If the headcrab doesn't have the clearance to so, they'll be stuck
// in the world. So this function makes sure there's headroom first.
//-----------------------------------------------------------------------------
bool CNPC_Kophyaeger::HasHeadroom()
{
	trace_t tr;
	UTIL_TraceEntity( this, GetAbsOrigin(), GetAbsOrigin() + Vector( 0, 0, 1 ), MASK_NPCSOLID, this, GetCollisionGroup(), &tr );

#if 0
	if( tr.fraction == 1.0f )
	{
		Msg("Headroom\n");
	}
	else
	{
		Msg("NO Headroom\n");
	}
#endif

	return (tr.fraction == 1.0);
}

//-----------------------------------------------------------------------------
// Purpose: LeapTouch - this is the headcrab's touch function when it is in the air.
// Input  : *pOther - 
//-----------------------------------------------------------------------------
void CNPC_Kophyaeger::LeapTouch( CBaseEntity *pOther )
{
	m_bMidJump = false;

	if ( IRelationType( pOther ) == D_HT )
	{
		// Don't hit if back on ground
		if ( !( GetFlags() & FL_ONGROUND ) )
		{
	 		if ( pOther->m_takedamage != DAMAGE_NO )
			{
				BiteSound();
				TouchDamage( pOther );

				// attack succeeded, so don't delay our next attack if we previously thought we failed
				m_bAttackFailed = false;
			}
			else
			{
				ImpactSound();
			}
		}
		else
		{
			ImpactSound();
		}
	}
	else if( !(GetFlags() & FL_ONGROUND) )
	{
		// Still in the air...
		if( !pOther->IsSolid() )
		{
			// Touching a trigger or something.
			return;
		}

		// just ran into something solid, so the attack probably failed.  make a note of it
		// so that when the attack is done, we'll delay attacking for a while so we don't
		// just repeatedly leap at the enemy from a bad location.
		m_bAttackFailed = true;

		if( gpGlobals->curtime < m_flIgnoreWorldCollisionTime )
		{
			// Headcrabs try to ignore the world, static props, and friends for a 
			// fraction of a second after they jump. This is because they often brush
			// doorframes or props as they leap, and touching those objects turns off
			// this touch function, which can cause them to hit the player and not bite.
			// A timer probably isn't the best way to fix this, but it's one of our 
			// safer options at this point (sjb).
			return;
		}
	}

	// Shut off the touch function.
	SetTouch( NULL );
	SetThink ( &CNPC_Kophyaeger::CallNPCThink );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int CNPC_Kophyaeger::CalcDamageInfo( CTakeDamageInfo *pInfo )
{
	pInfo->Set( this, this, KOPHYAEGER_DMG_BITE * (m_bAdult ? 3 : 1), DMG_SLASH );
	CalculateMeleeDamageForce( pInfo, GetAbsVelocity(), GetAbsOrigin() );
	return pInfo->GetDamage();
}

//-----------------------------------------------------------------------------
// Purpose: Deal the damage from the headcrab's touch attack.
//-----------------------------------------------------------------------------
void CNPC_Kophyaeger::TouchDamage( CBaseEntity *pOther )
{
	CTakeDamageInfo info;
	CalcDamageInfo( &info );
	pOther->TakeDamage( info  );
}

//-----------------------------------------------------------------------------
void CNPC_Kophyaeger::AlertSound( void )
{
	EmitSound( "NPC_Kophyaeger.Alert" );
}

//-----------------------------------------------------------------------------
void CNPC_Kophyaeger::AttackSound( void )
{
	EmitSound( "NPC_Kophyaeger.Attack" );
}

//-----------------------------------------------------------------------------
void CNPC_Kophyaeger::IdleSound( void )
{
	EmitSound( "NPC_Kophyaeger.Idle" );
}

//-----------------------------------------------------------------------------
void CNPC_Kophyaeger::PainSound( const CTakeDamageInfo &info )
{
	if ( m_flPainSoundTime < gpGlobals->curtime )
	{
		float flDuration;
		EmitSound( "NPC_Kophyaeger.Pain", 0, &flDuration );
		m_flPainSoundTime = gpGlobals->curtime + flDuration +
			random->RandomFloat( 2.0, 4.0 );
	}
}

//-----------------------------------------------------------------------------
void CNPC_Kophyaeger::DeathSound( const CTakeDamageInfo &info )
{
	CNPCDeathSound *pEnt = (CNPCDeathSound *) CBaseEntity::Create( "npc_death_sound", GetAbsOrigin(), GetAbsAngles(), NULL );
	if ( pEnt )
	{
		EmitSound( "AI_BaseNPC.SentenceStop" );
		Q_strcpy( pEnt->m_szSoundName.GetForModify(), "NPC_Kophyaeger.Death" );
		m_hDeathSound = pEnt;
	}
	else
	{
		Assert( 0 );
		EmitSound( "NPC_Kophyaeger.Death" );
	}
}

//-----------------------------------------------------------------------------
void CNPC_Kophyaeger::AttackHitSound( void )
{
//	EmitSound( "NPC_Kophyaeger.AttackHit" );
}

//-----------------------------------------------------------------------------
void CNPC_Kophyaeger::AttackMissSound( void )
{
//	EmitSound( "NPC_Kophyaeger.AttackMiss" );
}

//-----------------------------------------------------------------------------
void CNPC_Kophyaeger::BiteSound( void )
{
	EmitSound( "NPC_Kophyaeger.Bite" );
}

//-----------------------------------------------------------------------------
void CNPC_Kophyaeger::ImpactSound( void )
{
//	EmitSound( "NPC_Kophyaeger.Impact" );
}

//-----------------------------------------------------------------------------
int CNPC_Kophyaeger::GetSoundInterests( void ) 
{
	return BaseClass::GetSoundInterests() | ALL_SCENTS;
}

//-----------------------------------------------------------------------------
void CNPC_Kophyaeger::RemoveIgnoredConditions( void )
{
	BaseClass::RemoveIgnoredConditions();

	if ( m_flHungryTime > gpGlobals->curtime )
		 ClearCondition( COND_SMELL );
}

//-----------------------------------------------------------------------------
int CNPC_Kophyaeger::SelectSchedule( void )
{
	// When a schedule ends, conditions are set back to what GatherConditions
	// reported before selecting a new schedule.  See CAI_BaseNPC::MaintainSchedule.
	// Anything RemoveIgnoredConditions did is lost.
	if ( m_flHungryTime > gpGlobals->curtime )
		 ClearCondition( COND_SMELL );

	if ( GetState() == NPC_STATE_IDLE || GetState() == NPC_STATE_ALERT )
	{
		if ( HasCondition( COND_SMELL ) )
		{
			CSound *pSound;

			pSound = GetBestScent();
			
			if ( pSound && (!FInViewCone( pSound->GetSoundOrigin() ) || !FVisible( pSound->GetSoundOrigin() )) )
			{
				// scent is behind or occluded
				return SCHED_KOPHYAEGER_SNIFF_AND_EAT;
			}

			// food is right out in the open. Just go get it.
			return SCHED_KOPHYAEGER_EAT;
		}
	}
	return BaseClass::SelectSchedule();
}

//-----------------------------------------------------------------------------
int CNPC_Kophyaeger::SelectFailSchedule( int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode )
{
#ifdef KOPHY_BITE
	// If we can't jump then chew
	if ( m_bAdult && failedSchedule == SCHED_ESTABLISH_LINE_OF_FIRE )
		return SCHED_CHASE_ENEMY;
#endif

	return BaseClass::SelectFailSchedule( failedSchedule, failedTask, taskFailCode );
}

//-----------------------------------------------------------------------------
int CNPC_Kophyaeger::TranslateSchedule( int scheduleType )
{
	switch( scheduleType )
	{
	case SCHED_RANGE_ATTACK1:
		return SCHED_KOPHYAEGER_RANGE_ATTACK1;
	}

	return BaseClass::TranslateSchedule( scheduleType );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Kophyaeger::StartTask( const Task_t *pTask )
{
	switch( pTask->iTask )
	{
	case TASK_ANNOUNCE_ATTACK:
		{
			AttackSound();
			TaskComplete();
		}
		break;
	case TASK_GET_PATH_TO_BESTSCENT:
		{
			CSound *pScent;

			pScent = GetBestScent();

			if (!pScent) 
			{
				TaskFail( FAIL_NO_SCENT );
			}
			else
			{
				if ( GetNavigator()->SetGoal( pScent->GetSoundOrigin() ) == true )
				{
					// Don't choose a path to the floor above us etc
					if ( GetNavigator()->GetPathDistanceToGoal() > pScent->Volume() )
					{
						GetNavigator()->ClearGoal();
						TaskFail( FAIL_NO_ROUTE );
					}
				}

				// Hack - so we can face it
				m_vSavePosition = pScent->GetSoundOrigin();
			}
			break;
		}
	case TASK_KOPHYAEGER_EAT:
		{
			// NOTE: This doesn't give health
			m_flHungryTime = gpGlobals->curtime + pTask->flTaskData;
			TaskComplete();
			break;
		}

	case TASK_WALK_PATH_WITHIN_DIST:
		{
			if ( m_bAdult && ( IsCurSchedule( SCHED_KOPHYAEGER_EAT ) ||
				IsCurSchedule( SCHED_KOPHYAEGER_SNIFF_AND_EAT ) ) )
			{
				const Task_t task = { TASK_WALK_PATH_WITHIN_DIST, 64.0f };
				BaseClass::StartTask( &task );
			}
			else
			{
				BaseClass::StartTask( pTask );
			}
		}
		break;

	default:
		BaseClass::StartTask(pTask);
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Kophyaeger::RunTask( const Task_t *pTask )
{
	switch (pTask->iTask)
	{
		case TASK_RANGE_ATTACK1:
			if ( IsActivityFinished() )
			{
				TaskComplete();
				m_bMidJump = false;
				SetTouch( NULL );
				SetThink( &CNPC_Kophyaeger::CallNPCThink );
				SetIdealActivity( ACT_IDLE );

				if ( m_bAttackFailed )
				{
					// our attack failed because we just ran into something solid.
					// delay attacking for a while so we don't just repeatedly leap
					// at the enemy from a bad location.
					m_bAttackFailed = false;
					m_flNextAttack = gpGlobals->curtime + 1.2f;
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
				SetCycle( 1.0 );
				//m_iHealth = 0;
				AddSolidFlags( FSOLID_NOT_SOLID );
				//SetState( NPC_STATE_DEAD );
				BaseClass::RunTask( pTask );
			}
		}
		break;
#endif
		case TASK_WALK_PATH_WITHIN_DIST:
			{
				if ( m_bAdult && ( IsCurSchedule( SCHED_KOPHYAEGER_EAT ) ||
					IsCurSchedule( SCHED_KOPHYAEGER_SNIFF_AND_EAT ) ) )
				{
					const Task_t task = { TASK_WALK_PATH_WITHIN_DIST, 64.0f };
					BaseClass::RunTask( &task );
				}
				else
				{
					BaseClass::RunTask( pTask );
				}
			}
			break;
		default:
		{
			BaseClass::RunTask( pTask );
		}
		break;
	}
}

//-----------------------------------------------------------------------------
int CNPC_Kophyaeger::CountJumpersAtTarget( CBaseEntity *pVictim )
{
	Assert( pVictim != NULL );
	if ( pVictim == NULL )
		return 0;

	int nJumpers = 0;

	CBaseEntity *ppEnts[256];
	Vector vecCenter = pVictim->WorldSpaceCenter();
	float flRadius = KOPHYAEGER_MAX_JUMP_DIST * 1.5;
	int nEntCount = UTIL_EntitiesInSphere( ppEnts, 256, vecCenter, flRadius, 0 );

	for ( int i = 0; i < nEntCount; i++ )
	{
		if ( ppEnts[i] == NULL || ppEnts[i] == this || !ppEnts[i]->IsAlive() )
			continue;

		if ( ppEnts[i]->MyNPCPointer() == NULL )
			continue;

		if ( ppEnts[i]->m_iClassname != m_iClassname )
			continue;

		CNPC_Kophyaeger *pJumper = dynamic_cast<CNPC_Kophyaeger *>(ppEnts[i]);
		Assert( pJumper != NULL );
		if ( pJumper == NULL )
			continue;

		if ( pJumper->GetEnemy() == pVictim &&
			IsCurSchedule( SCHED_RANGE_ATTACK1 )
			/*pJumper->m_bMidJump == true*/ )
		{
			nJumpers++;
		}
	}

	return nJumpers;
}

//-----------------------------------------------------------------------------
// Purpose: For innate melee attack
// Input  :
// Output :
//-----------------------------------------------------------------------------
int CNPC_Kophyaeger::RangeAttack1Conditions( float flDot, float flDist )
{
	if ( gpGlobals->curtime < m_flNextAttack )
		return COND_NONE;

	if ( ( GetFlags() & FL_ONGROUND ) == false )
		return COND_NONE;

	if ( CountJumpersAtTarget( GetEnemy() ) > 0 )
		return COND_NONE;
#if 0
	// When we're burrowed ignore facing, because when we unburrow we'll cheat and face our enemy.
	if ( !m_bBurrowed && ( flDot < 0.65 ) )
		return COND_NOT_FACING_ATTACK;
#endif
	// This code stops lots of headcrabs swarming you and blocking you
	// whilst jumping up and down in your face over and over. It forces
	// them to back up a bit. If this causes problems, consider using it
	// for the fast headcrabs only, rather than just removing it.(sjb)
	if ( flDist < InnateRange1MinRange() )
		return COND_TOO_CLOSE_TO_ATTACK;

	if ( flDist > InnateRange1MaxRange() )
		return COND_TOO_FAR_TO_ATTACK;

	// Make sure the way is clear!
	CBaseEntity *pEnemy = GetEnemy();
	if ( pEnemy )
	{
		CBaseEntity *pBlocker;
		Vector vecJumpVel;

		float flDist = ThrowLimit( GetAbsOrigin() + Vector(0,0,1), GetEnemy()->EyePosition() - CollisionProp()->OBBCenter(),
			sv_gravity.GetFloat(), 16, CollisionProp()->OBBMins(), CollisionProp()->OBBMaxs(), GetEnemy(),
			&vecJumpVel, &pBlocker );

		if ( flDist != 0 )
			return COND_WEAPON_SIGHT_OCCLUDED;
	}

	return COND_CAN_RANGE_ATTACK1;
}

//-----------------------------------------------------------------------------
bool CNPC_Kophyaeger::InnateWeaponLOSCondition( const Vector &ownerPos, const Vector &targetPos, bool bSetConditions )
{
#if 0
	// BUG in original antlion code?
	// antlion checks next attack time but this method is for determining LOS
	//   from a node not whether we can *currently* attack from this point
	if ( GetNextAttack() > gpGlobals->curtime )
		return false;
#endif

	if ( ( GetFlags() & FL_ONGROUND ) == false )
		return false;

	CBaseEntity *pBlocker;
	Vector vecJumpVel;

	// targetPos is either EyePosition() or (if that fails) BodyTarget() - we always jump at the eyes
	Vector vTarget = GetEnemy()->GetAbsOrigin() + GetEnemy()->GetViewOffset() - CollisionProp()->OBBCenter();

	float flDist = ThrowLimit( ownerPos + Vector(0,0,1), vTarget,
		sv_gravity.GetFloat(), 16, CollisionProp()->OBBMins(), CollisionProp()->OBBMaxs(), GetEnemy(),
		&vecJumpVel, &pBlocker );

	if ( flDist != 0 )
	{
		if (bSetConditions)
		{
			SetCondition( COND_WEAPON_SIGHT_OCCLUDED );
			SetEnemyOccluder( pBlocker );
		}
		return false;
	}

	return true;
}


#ifdef KOPHY_BITE
//-----------------------------------------------------------------------------
int CNPC_Kophyaeger::MeleeAttack1Conditions( float flDot, float flDist )
{
	if ( flDist <= 45 && flDot >= 0.7 )
		return COND_CAN_MELEE_ATTACK1;
	
	return COND_NONE;
}
#endif

AI_BEGIN_CUSTOM_NPC( npc_kophyaeger, CNPC_Kophyaeger )

	DECLARE_TASK( TASK_KOPHYAEGER_EAT )

	DECLARE_ACTIVITY( ACT_KOPHYAEGER_EAT )
	DECLARE_ACTIVITY( ACT_KOPHYAEGER_DETECT_SCENT )

	DECLARE_ANIMEVENT( AE_KOPHYAEGER_JUMPATTACK )
#ifdef KOPHY_BITE
	DECLARE_ANIMEVENT( AE_KOPHYAEGER_BITE )
#endif

	//=========================================================
	// > SCHED_KOPHYAEGER_RANGE_ATTACK1
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_KOPHYAEGER_RANGE_ATTACK1,

		"	Tasks"
		"		TASK_STOP_MOVING			0"
//		"		TASK_FACE_ENEMY				0"
		"		TASK_ANNOUNCE_ATTACK		0"
		"		TASK_PLAY_SEQUENCE_FACE_ENEMY	ACTIVITY:ACT_KOPHYAEGER_THREAT_DISPLAY"
		"		TASK_RANGE_ATTACK1			0"
		"		TASK_SET_ACTIVITY			ACTIVITY:ACT_IDLE"
		"		TASK_FACE_IDEAL				0"
		"		TASK_WAIT_RANDOM			0.5"
		""
		"	Interrupts"
		"		COND_ENEMY_OCCLUDED"
//		"		COND_NO_PRIMARY_AMMO"
	)

	//=========================================================
	// > SCHED_KOPHYAEGER_EAT
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_KOPHYAEGER_EAT,
	
		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE				SCHEDULE:SCHED_IDLE_STAND"
		"		TASK_STOP_MOVING					0"
		"		TASK_KOPHYAEGER_EAT					10" // don't try eating again for 10 seconds in case this fails
		"		TASK_STORE_LASTPOSITION				0"
		"		TASK_GET_PATH_TO_BESTSCENT			0" // FIXME: pick feeding position outside corpse
		"		TASK_WALK_PATH_WITHIN_DIST			48"
		"		TASK_STOP_MOVING					0"
		"		TASK_FACE_SAVEPOSITION				0" // see TASK_GET_PATH_TO_BESTSCENT
		"		TASK_PLAY_SEQUENCE					ACTIVITY:ACT_KOPHYAEGER_EAT"
		"		TASK_PLAY_SEQUENCE					ACTIVITY:ACT_IDLE"
		"		TASK_PLAY_SEQUENCE					ACTIVITY:ACT_KOPHYAEGER_EAT"
		"		TASK_KOPHYAEGER_EAT					30"
		"		TASK_GET_PATH_TO_LASTPOSITION		0"
		"		TASK_WALK_PATH						0"
		"		TASK_WAIT_FOR_MOVEMENT				0"
		"		TASK_CLEAR_LASTPOSITION				0"
		"	"
		"	Interrupts"
		"		COND_LIGHT_DAMAGE"
		"		COND_HEAVY_DAMAGE"
		"		COND_NEW_ENEMY"
//		"		COND_SMELL"
	)
	
	//=========================================================
	// > SCHED_KOPHYAEGER_SNIFF_AND_EAT
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_KOPHYAEGER_SNIFF_AND_EAT,
	
		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE				SCHEDULE:SCHED_IDLE_STAND"
		"		TASK_STOP_MOVING					0"
		"		TASK_KOPHYAEGER_EAT					10" // don't try eating again for 10 seconds in case this fails
		"		TASK_PLAY_SEQUENCE					ACTIVITY:ACT_KOPHYAEGER_DETECT_SCENT"
		"		TASK_STORE_LASTPOSITION				0"
		"		TASK_GET_PATH_TO_BESTSCENT			0" // FIXME: pick feeding position outside corpse
		"		TASK_WALK_PATH_WITHIN_DIST			48"
		"		TASK_STOP_MOVING					0"
		"		TASK_FACE_SAVEPOSITION				0" // see TASK_GET_PATH_TO_BESTSCENT
		"		TASK_PLAY_SEQUENCE					ACTIVITY:ACT_KOPHYAEGER_EAT"
		"		TASK_PLAY_SEQUENCE					ACTIVITY:ACT_IDLE"
		"		TASK_PLAY_SEQUENCE					ACTIVITY:ACT_KOPHYAEGER_EAT"
		"		TASK_KOPHYAEGER_EAT					30"
		"		TASK_GET_PATH_TO_LASTPOSITION		0"
		"		TASK_WALK_PATH						0"
		"		TASK_WAIT_FOR_MOVEMENT				0"
		"		TASK_CLEAR_LASTPOSITION				0"
		"	"
		"	Interrupts"
		"		COND_LIGHT_DAMAGE"
		"		COND_HEAVY_DAMAGE"
		"		COND_NEW_ENEMY"
//		"		COND_SMELL"
	)

	DECLARE_ACTIVITY( ACT_KOPHYAEGER_THREAT_DISPLAY )

AI_END_CUSTOM_NPC()
