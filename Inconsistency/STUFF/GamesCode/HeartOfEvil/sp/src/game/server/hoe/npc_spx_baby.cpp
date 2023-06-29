#include "cbase.h"
#include "ai_basenpc.h"
#include "npcevent.h"
#include "movevars_shared.h" // sv_gravity
#include "npc_bullseye.h"
#include "hoe_deathsound.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

const int SPXBABY_MIN_JUMP_DIST = 48;
const int SPXBABY_MAX_JUMP_DIST = 256;

class CNPC_SpxBaby : public CAI_BaseNPC
{
public:
	DECLARE_CLASS( CNPC_SpxBaby, CAI_BaseNPC );
	DECLARE_DATADESC();
	DEFINE_CUSTOM_AI;

	void Spawn( void );
	void Precache( void );

	Class_T Classify( void );
	bool ClassifyPlayerAlly( void ) { return false; }
	bool ClassifyPlayerAllyVital( void ) { return false; }

	float MaxYawSpeed( void );

	void HandleAnimEvent( animevent_t *pEvent );

	void AlertSound( void );
	void AttackSound( void );
	void AttackHitSound( void );
	void AttackMissSound( void );
	void BiteSound( void );
	void DeathSound( const CTakeDamageInfo &info );
	void IdleSound( void );
	void ImpactSound( void );
	void PainSound( const CTakeDamageInfo &info );

	int TranslateSchedule( int scheduleType );
	int SelectFailSchedule( int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode );
	void StartTask( const Task_t *pTask );
	void RunTask( const Task_t *pTask );

	virtual float InnateRange1MinRange( void ) { return SPXBABY_MIN_JUMP_DIST; }
	virtual float InnateRange1MaxRange( void ) { return SPXBABY_MAX_JUMP_DIST; }
	bool InnateWeaponLOSCondition( const Vector &ownerPos, const Vector &targetPos, bool bSetConditions );

	int CountJumpersAtTarget( CBaseEntity *pVictim );
	int RangeAttack1Conditions( float flDot, float flDist );

	void JumpAttack( bool bRandomJump, const Vector &vecPos = vec3_origin, bool bThrown = false );
	void MoveOrigin( const Vector &vecDelta );
	void Leap( const Vector &vecVel );
	void TouchDamage( CBaseEntity *pOther );
	void ThrowThink( void );
	bool HasHeadroom();
	void LeapTouch( CBaseEntity *pOther );
	int CalcDamageInfo( CBaseEntity *pVictim, CTakeDamageInfo *pInfo );

	void TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr );
//	Vector BodyTarget( const Vector &posSrc, bool bNoisy );

	void InputJumpAtTarget( inputdata_t &inputdata );

	bool	m_bCommittedToJump;		// Whether we have 'locked in' to jump at our enemy.
	bool	m_bMidJump;
	bool	m_bAttackFailed;		// whether we ran into a wall during a jump.
	Vector	m_vecCommittedJumpPos;	// The position of our enemy when we locked in our jump attack.
	float	m_flNextNPCThink;

	float	m_flIgnoreWorldCollisionTime;

	//-----------------------------------------------------------------------------
	// Schedules
	//-----------------------------------------------------------------------------
	enum
	{
		SCHED_SPXBABY_RANGE_ATTACK1 = BaseClass::NEXT_SCHEDULE,
		SCHED_SPXBABY_WAKE_ANGRY,
	};

	//-----------------------------------------------------------------------------
	// Activities
	//-----------------------------------------------------------------------------
	static Activity ACT_SPXBABY_THREAT_DISPLAY;
};

LINK_ENTITY_TO_CLASS( npc_spx_baby, CNPC_SpxBaby );

//-----------------------------------------------------------------------------
// Spawnflags
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Animation events.
//-----------------------------------------------------------------------------
static int AE_SPXBABY_JUMPATTACK;

//-----------------------------------------------------------------------------
// Tasks
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Conditions 
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Activities
//-----------------------------------------------------------------------------
Activity CNPC_SpxBaby::ACT_SPXBABY_THREAT_DISPLAY;

BEGIN_DATADESC( CNPC_SpxBaby )
	DEFINE_FIELD( m_bCommittedToJump, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bMidJump, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_vecCommittedJumpPos, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( m_flNextNPCThink, FIELD_TIME ),

	DEFINE_THINKFUNC( ThrowThink ),
	DEFINE_ENTITYFUNC( LeapTouch ),

	DEFINE_INPUTFUNC( FIELD_STRING, "JumpAtTarget", InputJumpAtTarget ),
END_DATADESC()

#define SPXBABY_HEALTH 10 // 10, 10, 20
#define SPXBABY_IGNORE_WORLD_COLLISION_TIME 0.5
#define SPXBABY_DMG_BITE_NPC 5
#define SPXBABY_DMG_BITE_PLR 10 // 5, 10, 10

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_SpxBaby::Spawn( void )
{
	BaseClass::Spawn();

	Precache();
	SetModel( STRING( GetModelName() ) );

	SetHullType( HULL_TINY );
	SetHullSizeNormal();

	SetSolid( SOLID_BBOX );
	AddSolidFlags( FSOLID_NOT_STANDABLE ); // you can't stand on this
	SetMoveType( MOVETYPE_STEP );

	CapabilitiesClear();
	CapabilitiesAdd( bits_CAP_MOVE_GROUND | bits_CAP_INNATE_RANGE_ATTACK1 );

	SetBloodColor( BLOOD_COLOR_GREEN );
	m_NPCState = NPC_STATE_NONE;

	m_flFieldOfView = 0.5;
	SetViewOffset( Vector ( 0, 0, 20 ) );		// Position of the eyes relative to NPC's origin.

	m_iHealth = SPXBABY_HEALTH;

	NPCInit();
}

//-----------------------------------------------------------------------------
// Purpose: Precaches all resources this monster needs.
//-----------------------------------------------------------------------------
void CNPC_SpxBaby::Precache( void )
{
	SetModelName( AllocPooledString( "models/spx_baby/spx_baby.mdl" ) );
	PrecacheModel( STRING( GetModelName() ) );

	PrecacheScriptSound("NPC_SpxBaby.Alert");
	PrecacheScriptSound("NPC_SpxBaby.Attack");
	PrecacheScriptSound("NPC_SpxBaby.Bite");
	PrecacheScriptSound("NPC_SpxBaby.Die");
	PrecacheScriptSound("NPC_SpxBaby.Idle");
	PrecacheScriptSound("NPC_SpxBaby.Jump");
	PrecacheScriptSound("NPC_SpxBaby.Pain");

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Indicates this monster's place in the relationship table.
// Output : 
//-----------------------------------------------------------------------------
Class_T	CNPC_SpxBaby::Classify( void )
{
	return CLASS_SPX_BABY; 
}

//-----------------------------------------------------------------------------
float CNPC_SpxBaby::MaxYawSpeed( void )
{
	return 60.0f;
}

//-----------------------------------------------------------------------------
void CNPC_SpxBaby::HandleAnimEvent( animevent_t *pEvent )
{
	if (pEvent->event == AE_SPXBABY_JUMPATTACK)
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
void CNPC_SpxBaby::ThrowThink( void )
{
	if (gpGlobals->curtime > m_flNextNPCThink)
	{
		NPCThink();
		m_flNextNPCThink = gpGlobals->curtime + 0.1;
	}

	if( GetFlags() & FL_ONGROUND )
	{
		SetThink( &CNPC_SpxBaby::CallNPCThink );
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
void CNPC_SpxBaby::JumpAttack( bool bRandomJump, const Vector &vecPos, bool bThrown )
{
	Vector vecJumpVel;
	if ( !bRandomJump )
	{
#if 1	
		// The old code (taken from headcrab) prevented the NPC jumping down
		CBaseEntity *pBlocker;
		float flDist = ThrowLimit( GetAbsOrigin(), vecPos - CollisionProp()->OBBCenter(), sv_gravity.GetFloat(),
			16, CollisionProp()->OBBMins(), CollisionProp()->OBBMaxs(), GetEnemy(), &vecJumpVel, &pBlocker );

		// Low ceiling perhaps?
		if ( flDist != 0 )
		{
			flDist = ThrowLimit( GetAbsOrigin() + Vector(0,0,1), GetEnemy()->EyePosition() - Vector(0,0,CollisionProp()->OBBSize().z),
				sv_gravity.GetFloat(), 0, CollisionProp()->OBBMins(), CollisionProp()->OBBMaxs(), GetEnemy(),
				&vecJumpVel, &pBlocker );
		}
#else
		float gravity = sv_gravity.GetFloat();
		if ( gravity <= 1 )
		{
			gravity = 1;
		}

		// How fast does the headcrab need to travel to reach the position given gravity?
		float flActualHeight = vecPos.z - GetAbsOrigin().z;
		float height = flActualHeight;
		if ( height < 16 )
		{
			height = 16;
		}
		else
		{
			float flMaxHeight = bThrown ? 400 : 120;
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
		VectorSubtract( vecPos, GetAbsOrigin(), vecJumpVel );
		vecJumpVel /= time;

		// Speed to offset gravity at the desired height.
		vecJumpVel.z = speed;
#endif
		// Don't jump too far/fast.
		float flJumpSpeed = vecJumpVel.Length();
		float flMaxSpeed = bThrown ? 1000.0f : 650.0f;
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
	EmitSound( "NPC_SpxBaby.Jump" );
	Leap( vecJumpVel );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_SpxBaby::MoveOrigin( const Vector &vecDelta )
{
	UTIL_SetOrigin( this, GetLocalOrigin() + vecDelta );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : vecVel - 
//-----------------------------------------------------------------------------
void CNPC_SpxBaby::Leap( const Vector &vecVel )
{
	SetTouch( &CNPC_SpxBaby::LeapTouch );

	SetCondition( COND_FLOATING_OFF_GROUND );
	SetGroundEntity( NULL );

	m_flIgnoreWorldCollisionTime = gpGlobals->curtime + SPXBABY_IGNORE_WORLD_COLLISION_TIME;

	if( HasHeadroom() )
	{
		// Take him off ground so engine doesn't instantly reset FL_ONGROUND.
		MoveOrigin( Vector( 0, 0, 1 ) );
	}

	SetAbsVelocity( vecVel );

	// Think every frame so the player sees the headcrab where he actually is...
	m_bMidJump = true;
	SetThink( &CNPC_SpxBaby::ThrowThink );
	SetNextThink( gpGlobals->curtime );
}

//-----------------------------------------------------------------------------
// Before jumping, headcrabs usually use SetOrigin() to lift themselves off the 
// ground. If the headcrab doesn't have the clearance to so, they'll be stuck
// in the world. So this function makes sure there's headroom first.
//-----------------------------------------------------------------------------
bool CNPC_SpxBaby::HasHeadroom()
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
void CNPC_SpxBaby::LeapTouch( CBaseEntity *pOther )
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
	SetThink ( &CNPC_SpxBaby::CallNPCThink );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int CNPC_SpxBaby::CalcDamageInfo( CBaseEntity *pVictim, CTakeDamageInfo *pInfo )
{
	Assert(pVictim != NULL);
	pInfo->Set( this, this, pVictim->IsPlayer() ? SPXBABY_DMG_BITE_PLR : SPXBABY_DMG_BITE_NPC, DMG_SLASH );
	CalculateMeleeDamageForce( pInfo, GetAbsVelocity(), GetAbsOrigin() );
	return pInfo->GetDamage();
}

//-----------------------------------------------------------------------------
// Purpose: Deal the damage from the headcrab's touch attack.
//-----------------------------------------------------------------------------
void CNPC_SpxBaby::TouchDamage( CBaseEntity *pOther )
{
	CTakeDamageInfo info;
	CalcDamageInfo( pOther, &info );
	pOther->TakeDamage( info  );
}

//-----------------------------------------------------------------------------
void CNPC_SpxBaby::AlertSound( void )
{
	EmitSound( "NPC_SpxBaby.Alert" );
}

//-----------------------------------------------------------------------------
void CNPC_SpxBaby::AttackSound( void )
{
	EmitSound( "NPC_SpxBaby.Attack" );
}

//-----------------------------------------------------------------------------
void CNPC_SpxBaby::AttackHitSound( void )
{
	EmitSound( "NPC_SpxBaby.AttackHit" );
}

//-----------------------------------------------------------------------------
void CNPC_SpxBaby::AttackMissSound( void )
{
	EmitSound( "NPC_SpxBaby.AttackMiss" );
}

//-----------------------------------------------------------------------------
void CNPC_SpxBaby::BiteSound( void )
{
	EmitSound( "NPC_SpxBaby.Bite" );
}

//-----------------------------------------------------------------------------
void CNPC_SpxBaby::DeathSound( const CTakeDamageInfo &info )
{
	CNPCDeathSound *pEnt = (CNPCDeathSound *) CBaseEntity::Create( "npc_death_sound", GetAbsOrigin(), GetAbsAngles(), NULL );
	if ( pEnt )
	{
		EmitSound( "AI_BaseNPC.SentenceStop" );
		Q_strcpy( pEnt->m_szSoundName.GetForModify(), "NPC_SpxBaby.Die" );
		m_hDeathSound = pEnt;
	}
	else
	{
		Assert( 0 );
		EmitSound( "NPC_SpxBaby.Die" );
	}
}

//-----------------------------------------------------------------------------
void CNPC_SpxBaby::IdleSound( void )
{
	EmitSound( "NPC_SpxBaby.Idle" );
}

//-----------------------------------------------------------------------------
void CNPC_SpxBaby::PainSound( const CTakeDamageInfo &info )
{
	EmitSound( "NPC_SpxBaby.Pain" );
}

//-----------------------------------------------------------------------------
void CNPC_SpxBaby::ImpactSound( void )
{
	EmitSound( "NPC_SpxBaby.Impact" );
}

//---------------------------------------------------------
int CNPC_SpxBaby::TranslateSchedule( int scheduleType )
{
	switch( scheduleType )
	{
	case SCHED_RANGE_ATTACK1:
		return SCHED_SPXBABY_RANGE_ATTACK1;
#if 0
	case SCHED_WAKE_ANGRY:
	{
		if ( HaveSequenceForActivity( ACT_SPXBABY_THREAT_DISPLAY ) )
			return SCHED_SPXBABY_WAKE_ANGRY;
		return SCHED_WAKE_ANGRY;
	}
#endif
	}

	return BaseClass::TranslateSchedule( scheduleType );
}

//-----------------------------------------------------------------------------
int CNPC_SpxBaby::SelectFailSchedule( int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode )
{
	if ( failedSchedule == SCHED_BACK_AWAY_FROM_ENEMY && failedTask == TASK_FIND_BACKAWAY_FROM_SAVEPOSITION )
	{
		if ( HasCondition( COND_SEE_ENEMY ) )
		{
			return SCHED_RANGE_ATTACK1;
		}
	}

	return BaseClass::SelectFailSchedule( failedSchedule, failedTask, taskFailCode );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_SpxBaby::StartTask( const Task_t *pTask )
{
	switch( pTask->iTask )
	{
	case TASK_ANNOUNCE_ATTACK:
		{
			AttackSound();
			TaskComplete();
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
void CNPC_SpxBaby::RunTask( const Task_t *pTask )
{
	switch (pTask->iTask)
	{
		case TASK_RANGE_ATTACK1:
			if ( IsActivityFinished() )
			{
				TaskComplete();
				m_bMidJump = false;
				SetTouch( NULL );
				SetThink( &CNPC_SpxBaby::CallNPCThink );
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
				//m_iHealth = 0;
				AddSolidFlags( FSOLID_NOT_SOLID );
				//SetState( NPC_STATE_DEAD );
				BaseClass::RunTask(pTask);
			}
		}
		break;
#endif
		default:
		{
			BaseClass::RunTask( pTask );
		}
		break;
	}
}

//-----------------------------------------------------------------------------
int CNPC_SpxBaby::CountJumpersAtTarget( CBaseEntity *pVictim )
{
	Assert( pVictim != NULL );
	if ( pVictim == NULL )
		return 0;

	int nJumpers = 0;

	CBaseEntity *ppEnts[256];
	Vector vecCenter = pVictim->WorldSpaceCenter();
	float flRadius = SPXBABY_MAX_JUMP_DIST * 1.5;
	int nEntCount = UTIL_EntitiesInSphere( ppEnts, 256, vecCenter, flRadius, 0 );

	for ( int i = 0; i < nEntCount; i++ )
	{
		if ( ppEnts[i] == NULL || ppEnts[i] == this || !ppEnts[i]->IsAlive() )
			continue;

		if ( ppEnts[i]->MyNPCPointer() == NULL )
			continue;

		if ( ppEnts[i]->m_iClassname != m_iClassname )
			continue;

		CNPC_SpxBaby *pJumper = dynamic_cast<CNPC_SpxBaby *>(ppEnts[i]);
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
int CNPC_SpxBaby::RangeAttack1Conditions( float flDot, float flDist )
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
	if ( flDist < SPXBABY_MIN_JUMP_DIST )
		return COND_TOO_CLOSE_TO_ATTACK;

	if ( flDist > SPXBABY_MAX_JUMP_DIST )
		return COND_TOO_FAR_TO_ATTACK;

	// Make sure the way is clear!
	CBaseEntity *pEnemy = GetEnemy();
	if( pEnemy )
	{
		bool bEnemyIsBullseye = ( dynamic_cast<CNPC_Bullseye *>(pEnemy) != NULL );
#if 1
		CBaseEntity *pBlocker;
		Vector vecJumpVel;

		float flDist = ThrowLimit( GetAbsOrigin() + Vector(0,0,1), GetEnemy()->EyePosition() - CollisionProp()->OBBCenter(),
			sv_gravity.GetFloat(), 16, CollisionProp()->OBBMins(), CollisionProp()->OBBMaxs(), GetEnemy(),
			&vecJumpVel, &pBlocker );

//		if ( pBlocker != GetEnemy() )
//		{
//			if ( !bEnemyIsBullseye || pBlocker != NULL )
//				return COND_WEAPON_SIGHT_OCCLUDED;
//		}

		// Low ceiling perhaps?
		if ( flDist != 0 )
		{
			flDist = ThrowLimit( GetAbsOrigin() + Vector(0,0,1), GetEnemy()->EyePosition() - Vector(0,0,CollisionProp()->OBBSize().z),
				sv_gravity.GetFloat(), 0, CollisionProp()->OBBMins(), CollisionProp()->OBBMaxs(), GetEnemy(),
				&vecJumpVel, &pBlocker );
		}

		if ( flDist != 0 )
			return COND_WEAPON_SIGHT_OCCLUDED;
#else
		trace_t tr;
		AI_TraceLine( EyePosition(), pEnemy->EyePosition(), MASK_SOLID, this, COLLISION_GROUP_NONE, &tr );

		if ( tr.m_pEnt != GetEnemy() )
		{
			if ( !bEnemyIsBullseye || tr.m_pEnt != NULL )
				return COND_NONE;
		}

		if( GetEnemy()->EyePosition().z - 36.0f > GetAbsOrigin().z )
		{
			// Only run this test if trying to jump at a player who is higher up than me, else this 
			// code will always prevent a headcrab from jumping down at an enemy, and sometimes prevent it
			// jumping just slightly up at an enemy.
			Vector vStartHullTrace = GetAbsOrigin();
			vStartHullTrace.z += 1.0;

			Vector vEndHullTrace = GetEnemy()->EyePosition() - GetAbsOrigin();
			vEndHullTrace.NormalizeInPlace();
			vEndHullTrace *= 8.0;
			vEndHullTrace += GetAbsOrigin();

			AI_TraceHull( vStartHullTrace, vEndHullTrace,GetHullMins(), GetHullMaxs(), MASK_NPCSOLID, this, GetCollisionGroup(), &tr );

			if ( tr.m_pEnt != NULL && tr.m_pEnt != GetEnemy() )
			{
				return COND_TOO_CLOSE_TO_ATTACK;
			}
		}
#endif
	}

	return COND_CAN_RANGE_ATTACK1;
}

//-----------------------------------------------------------------------------
bool CNPC_SpxBaby::InnateWeaponLOSCondition( const Vector &ownerPos, const Vector &targetPos, bool bSetConditions )
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

	// Low ceiling perhaps?
	if ( flDist != 0 )
	{
		flDist = ThrowLimit( GetAbsOrigin() + Vector(0,0,1), GetEnemy()->EyePosition() - Vector(0,0,CollisionProp()->OBBSize().z),
			sv_gravity.GetFloat(), 0, CollisionProp()->OBBMins(), CollisionProp()->OBBMaxs(), GetEnemy(),
			&vecJumpVel, &pBlocker );
	}

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

//-----------------------------------------------------------------------------
void CNPC_SpxBaby::TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr )
{
	// NPCs tend to peg these guys off in mid-jump with laser-like accuracy
	// so force some shots to "miss".
	if ( info.GetDamageType() & DMG_BULLET &&
		info.GetAttacker() &&
		info.GetAttacker()->IsNPC() )
	{
		float flChanceToMiss = 0.2;
		if ( !(GetFlags() & FL_ONGROUND ) )
			flChanceToMiss = 0.7;
		else if ( IsMoving() )
			flChanceToMiss = 0.5;
		if ( random->RandomFloat(0,1) <= flChanceToMiss )
			return;
	}

	return BaseClass::TraceAttack( info, vecDir, ptr, 0 );
}

#if 0
//-----------------------------------------------------------------------------
Vector CNPC_SpxBaby::BodyTarget( const Vector &posSrc, bool bNoisy ) 
{
	// Barney pegs these guys off in mid-jump so force some shots to miss.
	// Really we should track how fast the spx is moving and how long the enemy
	// has been focusing on us.
	float flChanceToMiss = 0.1;
	if ( !(GetFlags() & FL_ONGROUND ) )
		flChanceToMiss = 0.5;
	else if ( IsMoving() )
		flChanceToMiss = 0.3;
	if ( bNoisy && random->RandomFloat(0,1) <= flChanceToMiss )
	{
		Vector result;
		// Aim the shot to the side
		CollisionProp()->NormalizedToWorldSpace( Vector( 0.5f, 0.5f, 0.5f ), &result );
		return result;
	}

	return BaseClass::BodyTarget( posSrc, bNoisy );
}
#endif

//-----------------------------------------------------------------------------
void CNPC_SpxBaby::InputJumpAtTarget( inputdata_t &inputdata )
{
	const char *entName = inputdata.value.String();
	CBaseEntity *pEnt = gEntList.FindEntityByName( NULL, entName );
	if ( pEnt == NULL )
	{
		DevWarning( "CNPC_SpxBaby::InputJumpAtTarget: No such entity '%s'\n", entName );
		return;
	}
	Vector vecPos = pEnt->EyePosition();

	float flSpeed = 400.0f;
#if 1
	if ( (GetAbsOrigin() - vecPos).IsLengthLessThan( 200 ) )
		flSpeed = 100.0f;
#endif
	Vector vecJumpVel = VecCheckThrow( this, GetAbsOrigin(), vecPos, flSpeed );
	if ( vecJumpVel == vec3_origin )
	{
		DevWarning( "CNPC_SpxBaby::InputJumpAtTarget: can't jump at %s\n", entName );
		return;
	}
	Leap( vecJumpVel );
}

AI_BEGIN_CUSTOM_NPC( npc_spx_baby, CNPC_SpxBaby )
	DECLARE_ANIMEVENT( AE_SPXBABY_JUMPATTACK )

	//=========================================================
	// > SCHED_SPXBABY_RANGE_ATTACK1
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_SPXBABY_RANGE_ATTACK1,

		"	Tasks"
		"		TASK_STOP_MOVING			0"
//		"		TASK_FACE_ENEMY				0"
		"		TASK_ANNOUNCE_ATTACK		0"
		"		TASK_PLAY_SEQUENCE_FACE_ENEMY	ACTIVITY:ACT_SPXBABY_THREAT_DISPLAY"
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
	//
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_SPXBABY_WAKE_ANGRY,

		"	Tasks"
		"		TASK_STOP_MOVING				0"
		"		TASK_SET_ACTIVITY				ACTIVITY:ACT_IDLE "
		"		TASK_FACE_IDEAL					0"
		"		TASK_SOUND_WAKE					0"
		"		TASK_PLAY_SEQUENCE_FACE_ENEMY	ACTIVITY:ACT_SPXBABY_THREAT_DISPLAY"
		""
		"	Interrupts"
	)

	DECLARE_ACTIVITY( ACT_SPXBABY_THREAT_DISPLAY )

AI_END_CUSTOM_NPC()
