#include "cbase.h"
#include "ai_basenpc.h"
#include "ai_memory.h"
#include "npcevent.h"
#include "movevars_shared.h" // sv_gravity
#include "npc_bullseye.h"
#include "props.h"
#include "weapon_physcannon.h"
#include "hoe_deathsound.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

const int GORILLA_MIN_JUMP_DIST = 64;
const int GORILLA_MAX_JUMP_DIST = 384 /* was 512*/;

class CNPC_Gorilla : public CAI_BaseNPC
{
public:
	DECLARE_CLASS( CNPC_Gorilla, CAI_BaseNPC );
	DECLARE_DATADESC();
	DEFINE_CUSTOM_AI;

	void Spawn( void );
	void Precache( void );

	Class_T Classify( void );
	bool ClassifyPlayerAlly( void ) { return false; }
	bool ClassifyPlayerAllyVital( void ) { return false; }

	void HandleAnimEvent( animevent_t *pEvent );

	CBaseEntity *Punch( float flDist, int iDamage, QAngle &qaViewPunch, Vector &vecVelocityPunch, int BloodOrigin  );
	float GetMeleeAttackRange() const { return 64; } // from CGorilla::CheckMeleeAttack1
	void AttackHitSound( void );
	void AttackMissSound( void );

	void JumpAttack( bool bRandomJump, const Vector &vecPos = vec3_origin );
	void Leap( const Vector &vecVel );
	int CalcJumpDamageInfo( CBaseEntity *pOther, CTakeDamageInfo *pInfo );
	void TouchDamage( CBaseEntity *pOther );
	void LeapThink( void );
	bool HasHeadroom();
	void LeapTouch( CBaseEntity *pOther );

	void IdleSound( void );
	void AlertSound( void );
	void PainSound( const CTakeDamageInfo &info );
	void DeathSound( const CTakeDamageInfo &info );
	void ImpactSound( void );

	int GetSoundInterests( void );

	void RemoveIgnoredConditions( void );

	bool SeenEnemyWithinTime( float flTime );

	int SelectSchedule( void );
	int SelectCombatSchedule( void );
	int SelectFailSchedule( int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode );
	int TranslateSchedule( int scheduleType );
	void BuildScheduleTestBits( void );
	void StartTask( const Task_t *pTask );
	void RunTask( const Task_t *pTask );

	int OnTakeDamage_Alive( const CTakeDamageInfo &info );

	virtual float InnateRange1MinRange( void ) { return GORILLA_MIN_JUMP_DIST; }
	virtual float InnateRange1MaxRange( void ) { return GORILLA_MAX_JUMP_DIST; }
	virtual bool InnateWeaponLOSCondition( const Vector &ownerPos, const Vector &targetPos, bool bSetConditions );

	int RangeAttack1Conditions( float flDot, float flDist );
	int MeleeAttack1Conditions( float flDot, float flDist );

	bool	m_bCommittedToJump;		// Whether we have 'locked in' to jump at our enemy.
	bool	m_bMidJump;
	bool	m_bAttackFailed;		// whether we ran into a wall during a jump.
	Vector	m_vecCommittedJumpPos;	// The position of our enemy when we locked in our jump attack.
	float	m_flNextNPCThink;
	float	m_flIgnoreWorldCollisionTime;
	float	m_flHungryTime;
	float	m_flPainTime;

	//-----------------------------------------------------------------------------
	// Custom schedules.
	//-----------------------------------------------------------------------------
	enum
	{
		SCHED_GORILLA_MELEE_ATTACK1 = BaseClass::NEXT_SCHEDULE,
		SCHED_GORILLA_RANGE_ATTACK1,
		SCHED_GORILLA_EAT,
		SCHED_GORILLA_VICTORY_DANCE,
		NEXT_SCHEDULE,
	};

	//-----------------------------------------------------------------------------
	// Tasks
	//-----------------------------------------------------------------------------
	enum
	{
		TASK_GORILLA_SOUND_VICTORY = BaseClass::NEXT_TASK,
		TASK_GORILLA_DONT_EAT,
		TASK_GORILLA_EAT,
		TASK_GORILLA_SOUND_EAT,
		NEXT_TASK,
	};

	//-----------------------------------------------------------------------------
	// Activities
	//-----------------------------------------------------------------------------
	static Activity ACT_GORILLA_EAT;
};

LINK_ENTITY_TO_CLASS( npc_gorilla, CNPC_Gorilla );

//-----------------------------------------------------------------------------
// Spawnflags
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Animation events.
//-----------------------------------------------------------------------------
static Animevent AE_GORILLA_ATTACK_LEFT;
static Animevent AE_GORILLA_ATTACK_RIGHT;
static Animevent AE_GORILLA_ATTACK_BOTH;
static Animevent AE_GORILLA_JUMPATTACK;

Activity CNPC_Gorilla::ACT_GORILLA_EAT;

//-----------------------------------------------------------------------------
// Tasks
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Conditions 
//-----------------------------------------------------------------------------

BEGIN_DATADESC( CNPC_Gorilla )
	DEFINE_FIELD( m_bCommittedToJump, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bMidJump, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_vecCommittedJumpPos, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( m_flNextNPCThink, FIELD_TIME ),
	DEFINE_FIELD( m_flHungryTime, FIELD_TIME ),
	DEFINE_FIELD( m_flPainTime, FIELD_TIME ),

	DEFINE_THINKFUNC( LeapThink ),
	DEFINE_ENTITYFUNC( LeapTouch ),
END_DATADESC()

#define GORILLA_HEALTH 300 // 200, 300, 400
#define GORILLA_DMG_PUNCH 25 // 20, 25, 30
#define GORILLA_DMG_JUMP 35 // 30, 35, 40

#define GORILLA_MAX_JUMP_SPEED 1000.0f
#define GORILLA_IGNORE_WORLD_COLLISION_TIME 0.5

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Gorilla::Spawn( void )
{
	BaseClass::Spawn();

	Precache();
	SetModel( STRING( GetModelName() ) );

	SetHullType( HULL_LARGE ); // modified bounds of HULL_LARGE
	SetHullSizeNormal();
//	UTIL_SetSize( this, Vector(-30, -30, 0), Vector(30, 30, 72) );

	SetSolid( SOLID_BBOX );
	SetMoveType( MOVETYPE_STEP );

	CapabilitiesClear();
	CapabilitiesAdd(
		bits_CAP_MOVE_GROUND |
		bits_CAP_INNATE_MELEE_ATTACK1 |
		bits_CAP_INNATE_RANGE_ATTACK1 );

	SetBloodColor( BLOOD_COLOR_RED );
	m_NPCState = NPC_STATE_NONE;

	m_flFieldOfView = 0.5;
	SetViewOffset( Vector( 0, 0, 60 ) );		// Position of the eyes relative to NPC's origin.

	m_iHealth = GORILLA_HEALTH;

	NPCInit();
}

//-----------------------------------------------------------------------------
// Purpose: Precaches all resources this monster needs.
//-----------------------------------------------------------------------------
void CNPC_Gorilla::Precache( void )
{
	SetModelName( AllocPooledString( "models/ape/gorilla/gorilla.mdl" ) );
	PrecacheModel( STRING( GetModelName() ) );

	PrecacheScriptSound("NPC_Gorilla.FootstepLeft");
	PrecacheScriptSound("NPC_Gorilla.FootstepRight");
	PrecacheScriptSound("NPC_Gorilla.AnnounceAttack");
	PrecacheScriptSound("NPC_Gorilla.AttackHit");
	PrecacheScriptSound("NPC_Gorilla.AttackMiss");
	PrecacheScriptSound("NPC_Gorilla.Idle");
	PrecacheScriptSound("NPC_Gorilla.Alert");
	PrecacheScriptSound("NPC_Gorilla.Pain");
	PrecacheScriptSound("NPC_Gorilla.Death");
	PrecacheScriptSound("NPC_Gorilla.Victory");
	PrecacheScriptSound("NPC_Gorilla.Eat");

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Indicates this monster's place in the relationship table.
// Output : 
//-----------------------------------------------------------------------------
Class_T	CNPC_Gorilla::Classify( void )
{
	return CLASS_GORILLA; 
}

//-----------------------------------------------------------------------------
void CNPC_Gorilla::HandleAnimEvent( animevent_t *pEvent )
{
	if (pEvent->event == AE_GORILLA_ATTACK_LEFT)
	{
		Vector right, forward;
		AngleVectors( GetLocalAngles(), &forward, &right, NULL );

		right = right * -100;
		forward = forward * 200;

		Punch( GetMeleeAttackRange(), GORILLA_DMG_PUNCH, QAngle( -15, 20, -10 ), right + forward, -1 /* ZOMBIE_BLOOD_LEFT_HAND */);
		return;
	}

	if (pEvent->event == AE_GORILLA_ATTACK_RIGHT)
	{
		Vector right, forward;
		AngleVectors( GetLocalAngles(), &forward, &right, NULL );
		
		right = right * 100;
		forward = forward * 200;

		Punch( GetMeleeAttackRange(), GORILLA_DMG_PUNCH, QAngle( -15, -20, -10 ), right + forward, -1 /* ZOMBIE_BLOOD_RIGHT_HAND */);
		return;
	}

	if (pEvent->event == AE_GORILLA_ATTACK_BOTH)
	{
		Vector forward;
		QAngle qaPunch( 45, random->RandomInt(-5,5), random->RandomInt(-5,5) );
		AngleVectors( GetLocalAngles(), &forward );
		forward = forward * 200;
		Punch( GetMeleeAttackRange(), GORILLA_DMG_PUNCH * 2, qaPunch, forward, -1 /* ZOMBIE_BLOOD_BOTH_HANDS */);
		return;
	}

	if (pEvent->event == AE_GORILLA_JUMPATTACK)
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
	case AE_NPC_LEFTFOOT: // Eh? different than NPC_EVENT_LEFTFOOT
		{/*
			It seems that to enable different footstep sounds for walking on different materials the model
			must emit CL_EVENT_MFOOTSTEP_*** events and also have attachments called "LeftFoot" and
			"RightFoot". This ends up calling MaterialFootstepSound() in c_baseanimating.cpp.

			Another option is the CL_EVENT_FOOTSTEP_*** (note lack of M) events with an option of the prefix
			which will be added to %s.FootstepLeft or %s.RunFootstepLeft. This won't play sounds based
			on the material however.
			*/

			// TODO: bloody footprints

			MakeAIFootstepSound( 180.0f ); // only antlions and zombies seem to use this
			EmitSound(pEvent->options);
		}
		break;
	case AE_NPC_RIGHTFOOT: // Eh? different than NPC_EVENT_LEFTFOOT
		{
			// TODO: bloody footprints

			MakeAIFootstepSound( 180.0f ); // only antlions and zombies seem to use this
			EmitSound(pEvent->options);
		}
		break;
	default:
		BaseClass::HandleAnimEvent( pEvent );
		break;
	}
}

//-----------------------------------------------------------------------------
int CNPC_Gorilla::MeleeAttack1Conditions( float flDot, float flDist )
{
	if (flDist > GetMeleeAttackRange() )
	{
		// Translate a hit vehicle into its passenger if found
		if ( GetEnemy() != NULL )
		{
//			CBaseCombatCharacter *pCCEnemy = GetEnemy()->MyCombatCharacterPointer();
//			if ( pCCEnemy != NULL && pCCEnemy->IsInAVehicle() )
//				return MeleeAttack1ConditionsVsEnemyInVehicle( pCCEnemy, flDot );

#if defined(HL2_DLL) && !defined(HL2MP)
			// If the player is holding an object, knock it down.
			if( GetEnemy()->IsPlayer() )
			{
				CBasePlayer *pPlayer = ToBasePlayer( GetEnemy() );

				Assert( pPlayer != NULL );

				// Is the player carrying something?
				CBaseEntity *pObject = GetPlayerHeldEntity(pPlayer);

				if( !pObject )
				{
					pObject = PhysCannonGetHeldEntity( pPlayer->GetActiveWeapon() );
				}

				if( pObject )
				{
					float flDist = pObject->WorldSpaceCenter().DistTo( WorldSpaceCenter() );

					if( flDist <= GetMeleeAttackRange() )
						return COND_CAN_MELEE_ATTACK1;
				}
			}
#endif
		}
		return COND_TOO_FAR_TO_ATTACK;
	}

	if (flDot < 0.7)
	{
		return COND_NOT_FACING_ATTACK;
	}

	// Build a cube-shaped hull, the same hull that ClawAttack() is going to use.
	Vector vecMins = GetHullMins();
	Vector vecMaxs = GetHullMaxs();
	vecMins.z = vecMins.x;
	vecMaxs.z = vecMaxs.x;

	Vector forward;
	GetVectors( &forward, NULL, NULL );

	trace_t	tr;
	CTraceFilterNav traceFilter( this, false, this, COLLISION_GROUP_NONE );
	AI_TraceHull( WorldSpaceCenter(), WorldSpaceCenter() + forward * GetMeleeAttackRange(), vecMins, vecMaxs, MASK_NPCSOLID, &traceFilter, &tr );

	if( tr.fraction == 1.0 || !tr.m_pEnt )
	{
		// This attack would miss completely. Trick the zombie into moving around some more.
		return COND_TOO_FAR_TO_ATTACK;
	}

	if( tr.m_pEnt == GetEnemy() || tr.m_pEnt->IsNPC() || (tr.m_pEnt->m_takedamage == DAMAGE_YES && (dynamic_cast<CBreakableProp*>(tr.m_pEnt))) )
	{
		// -Let the zombie swipe at his enemy if he's going to hit them.
		// -Also let him swipe at NPC's that happen to be between the zombie and the enemy. 
		//  This makes mobs of zombies seem more rowdy since it doesn't leave guys in the back row standing around.
		// -Also let him swipe at things that takedamage, under the assumptions that they can be broken.
		return COND_CAN_MELEE_ATTACK1;
	}
#if 0
	if( tr.m_pEnt->IsBSPModel() )
	{
		// The trace hit something solid, but it's not the enemy. If this item is closer to the zombie than
		// the enemy is, treat this as an obstruction.
		Vector vecToEnemy = GetEnemy()->WorldSpaceCenter() - WorldSpaceCenter();
		Vector vecTrace = tr.endpos - tr.startpos;

		if( vecTrace.Length2DSqr() < vecToEnemy.Length2DSqr() )
		{
			return COND_ZOMBIE_LOCAL_MELEE_OBSTRUCTION;
		}
	}
#endif
#ifdef HL2_EPISODIC
	if ( !tr.m_pEnt->IsWorld() && GetEnemy() && GetEnemy()->GetGroundEntity() == tr.m_pEnt )
	{
		//Try to swat whatever the player is standing on instead of acting like a dill.
		return COND_CAN_MELEE_ATTACK1;
	}
#endif

	// Move around some more
	return COND_TOO_FAR_TO_ATTACK;
}

//-----------------------------------------------------------------------------
// Purpose: Look in front and see if the claw hit anything.
//
// Input  :	flDist				distance to trace		
//			iDamage				damage to do if attack hits
//			vecViewPunch		camera punch (if attack hits player)
//			vecVelocityPunch	velocity punch (if attack hits player)
//
// Output : The entity hit by claws. NULL if nothing.
//-----------------------------------------------------------------------------
CBaseEntity *CNPC_Gorilla::Punch( float flDist, int iDamage, QAngle &qaViewPunch, Vector &vecVelocityPunch, int BloodOrigin  )
{
	// Added test because claw attack anim sometimes used when for cases other than melee
	if ( GetEnemy() )
	{
		trace_t	tr;
		AI_TraceHull( WorldSpaceCenter(), GetEnemy()->WorldSpaceCenter(), -Vector(8,8,8), Vector(8,8,8), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr );

		if ( tr.fraction < 1.0f )
			return NULL;

		// Reduce damage done to NPCs to avoid mass slaughter.
		// FIXME: this may not be the entity hit by CheckTraceHullAttack().
		if ( GetEnemy()->IsNPC() && !FClassnameIs( GetEnemy(), "npc_gorilla" ) )
			iDamage /= 2;
	}

	//
	// Trace out a cubic section of our hull and see what we hit.
	//
	Vector vecMins = GetHullMins();
	Vector vecMaxs = GetHullMaxs();
	vecMins.z = vecMins.x;
	vecMaxs.z = vecMaxs.x;

	CBaseEntity *pHurt = CheckTraceHullAttack( flDist, vecMins, vecMaxs, iDamage, DMG_CLUB );
#if 0
	if ( !pHurt && m_hPhysicsEnt != NULL && IsCurSchedule(SCHED_ZOMBIE_ATTACKITEM) )
	{
		pHurt = m_hPhysicsEnt;

		Vector vForce = pHurt->WorldSpaceCenter() - WorldSpaceCenter(); 
		VectorNormalize( vForce );

		vForce *= 5 * 24;

		CTakeDamageInfo info( this, this, vForce, GetAbsOrigin(), iDamage, DMG_SLASH );
		pHurt->TakeDamage( info );

		pHurt = m_hPhysicsEnt;
	}
#endif
	if ( pHurt )
	{
		AttackHitSound();

		CBasePlayer *pPlayer = ToBasePlayer( pHurt );

		if ( pPlayer != NULL && !(pPlayer->GetFlags() & FL_GODMODE ) )
		{
			pPlayer->ViewPunch( qaViewPunch );
			
			pPlayer->VelocityPunch( vecVelocityPunch );
		}
#if 0
		else if( !pPlayer && UTIL_ShouldShowBlood(pHurt->BloodColor()) )
		{
			// Hit an NPC. Bleed them!
			Vector vecBloodPos;

			switch( BloodOrigin )
			{
			case ZOMBIE_BLOOD_LEFT_HAND:
				if( GetAttachment( "blood_left", vecBloodPos ) )
					SpawnBlood( vecBloodPos, g_vecAttackDir, pHurt->BloodColor(), min( iDamage, 30 ) );
				break;

			case ZOMBIE_BLOOD_RIGHT_HAND:
				if( GetAttachment( "blood_right", vecBloodPos ) )
					SpawnBlood( vecBloodPos, g_vecAttackDir, pHurt->BloodColor(), min( iDamage, 30 ) );
				break;

			case ZOMBIE_BLOOD_BOTH_HANDS:
				if( GetAttachment( "blood_left", vecBloodPos ) )
					SpawnBlood( vecBloodPos, g_vecAttackDir, pHurt->BloodColor(), min( iDamage, 30 ) );

				if( GetAttachment( "blood_right", vecBloodPos ) )
					SpawnBlood( vecBloodPos, g_vecAttackDir, pHurt->BloodColor(), min( iDamage, 30 ) );
				break;

			case ZOMBIE_BLOOD_BITE:
				// No blood for these.
				break;
			}
		}
#endif
	}
	else 
	{
		AttackMissSound();
	}
#if 0
	if ( pHurt == m_hPhysicsEnt && IsCurSchedule(SCHED_ZOMBIE_ATTACKITEM) )
	{
		m_hPhysicsEnt = NULL;
		m_flNextSwat = gpGlobals->curtime + random->RandomFloat( 2, 4 );
	}
#endif
	return pHurt;
}

//-----------------------------------------------------------------------------
void CNPC_Gorilla::LeapThink( void )
{
	if (gpGlobals->curtime > m_flNextNPCThink)
	{
		NPCThink();
		m_flNextNPCThink = gpGlobals->curtime + 0.1;
	}

	if( GetFlags() & FL_ONGROUND )
	{
		SetThink( &CNPC_Gorilla::CallNPCThink );
		SetNextThink( gpGlobals->curtime + 0.1 );
		return;
	}

	SetNextThink( gpGlobals->curtime );
}

//-----------------------------------------------------------------------------
void CNPC_Gorilla::JumpAttack( bool bRandomJump, const Vector &vecPos )
{
	Vector vecJumpVel;
	if ( !bRandomJump )
	{
#if 1
		// The old code (taken from headcrab) prevented the gorilla jumping down
		CBaseEntity *pBlocker;
		ThrowLimit( GetAbsOrigin(), vecPos - CollisionProp()->OBBCenter(), sv_gravity.GetFloat(),
			3, GetHullMins(), GetHullMaxs(), GetEnemy(), &vecJumpVel, &pBlocker );
#else
		float gravity = sv_gravity.GetFloat();
		if ( gravity <= 1 )
		{
			gravity = 1;
		}

		// This is the point on the model we want to hit the eyes.
		// The original Headcrab code used GetAbsOrigin() which is the
		// feet of the Gorilla.
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
		float flMaxSpeed = GORILLA_MAX_JUMP_SPEED;
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
void CNPC_Gorilla::Leap( const Vector &vecVel )
{
	SetTouch( &CNPC_Gorilla::LeapTouch );

	SetCondition( COND_FLOATING_OFF_GROUND );
	SetGroundEntity( NULL );

	m_flIgnoreWorldCollisionTime = gpGlobals->curtime + GORILLA_IGNORE_WORLD_COLLISION_TIME;

	if ( HasHeadroom() )
	{
		// Take him off ground so engine doesn't instantly reset FL_ONGROUND.
		UTIL_SetOrigin( this, GetLocalOrigin() + Vector( 0, 0, 1 ) );
	}

	SetAbsVelocity( vecVel );

	// Think every frame so the player sees the headcrab where he actually is...
	m_bMidJump = true;
	SetThink( &CNPC_Gorilla::LeapThink );
	SetNextThink( gpGlobals->curtime );
}

//-----------------------------------------------------------------------------
bool CNPC_Gorilla::HasHeadroom()
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
void CNPC_Gorilla::LeapTouch( CBaseEntity *pOther )
{
	m_bMidJump = false;

	if ( IRelationType( pOther ) == D_HT )
	{
		// Don't hit if back on ground
		if ( !( GetFlags() & FL_ONGROUND ) )
		{
	 		if ( pOther->m_takedamage != DAMAGE_NO )
			{
				ImpactSound();
				TouchDamage( pOther );

				// attack succeeded, so don't delay our next attack if we previously thought we failed
				m_bAttackFailed = false;
			}
			else
			{
//				ImpactSound();
			}
		}
		else
		{
//			ImpactSound();
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
	SetThink ( &CNPC_Gorilla::CallNPCThink );
}

//-----------------------------------------------------------------------------
int CNPC_Gorilla::CalcJumpDamageInfo( CBaseEntity *pOther, CTakeDamageInfo *pInfo )
{
	float flDamage = GORILLA_DMG_JUMP * GetAbsVelocity().Length() / GORILLA_MAX_JUMP_SPEED;
	if ( pOther && pOther->IsNPC() && pOther->m_iClassname != m_iClassname )
		flDamage /= 2;
	DevMsg( "CNPC_Gorilla::CalcJumpDamageInfo damage %.2f\n", flDamage );
	pInfo->Set( this, this, flDamage, DMG_CLUB );
	CalculateMeleeDamageForce( pInfo, GetAbsVelocity(), GetAbsOrigin() );
	return pInfo->GetDamage();
}

//-----------------------------------------------------------------------------
// Purpose: Deal the damage from the headcrab's touch attack.
//-----------------------------------------------------------------------------
void CNPC_Gorilla::TouchDamage( CBaseEntity *pOther )
{
	CTakeDamageInfo info;
	CalcJumpDamageInfo( pOther, &info );
	pOther->TakeDamage( info  );

	pOther->ViewPunch( QAngle(20,0,-20) );
			
	// screeshake transforms the viewmodel as well as the viewangle. No problems with seeing the ends of the viewmodels.
	UTIL_ScreenShake( pOther->GetAbsOrigin(), 25.0, 1.5, 0.7, 2, SHAKE_START );

	// If the player, throw him around
	if ( pOther->IsPlayer())
	{
		Vector forward, up;
		AngleVectors( GetLocalAngles(), &forward, NULL, &up );
		pOther->ApplyAbsVelocityImpulse( forward * 300 + up * 100 );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Play a random attack sound.
//-----------------------------------------------------------------------------
void CNPC_Gorilla::AttackHitSound( void )
{
	EmitSound( "NPC_Gorilla.AttackHit" );
}

//-----------------------------------------------------------------------------
// Purpose: Play a random attack sound.
//-----------------------------------------------------------------------------
void CNPC_Gorilla::AttackMissSound( void )
{
	EmitSound( "NPC_Gorilla.AttackMiss" );
}

//-----------------------------------------------------------------------------
void CNPC_Gorilla::IdleSound( void )
{
	if ( IsCurSchedule( SCHED_GORILLA_EAT ) )
		return;
	EmitSound( "NPC_Gorilla.Idle" );
}

//-----------------------------------------------------------------------------
void CNPC_Gorilla::AlertSound( void )
{
	EmitSound( "NPC_Gorilla.Alert" );
}

//-----------------------------------------------------------------------------
void CNPC_Gorilla::PainSound( const CTakeDamageInfo &info )
{
	// Make sure he doesn't just repeatedly go uh-uh-uh-uh-uh when hit by rapid fire
	if ( gpGlobals->curtime < m_flPainTime ) return;

	float flDuration;
	EmitSound( "NPC_Gorilla.Pain", 0, &flDuration );

	flDuration = max( flDuration, 2.0f );
	m_flPainTime = gpGlobals->curtime + flDuration + random->RandomFloat( 2.0, 4.0 );
}

//-----------------------------------------------------------------------------
void CNPC_Gorilla::DeathSound( const CTakeDamageInfo &info )
{
	CNPCDeathSound *pEnt = (CNPCDeathSound *) CBaseEntity::Create( "npc_death_sound", GetAbsOrigin(), GetAbsAngles(), NULL );
	if ( pEnt )
	{
		EmitSound( "AI_BaseNPC.SentenceStop" );
		Q_strcpy( pEnt->m_szSoundName.GetForModify(), "NPC_Gorilla.Death" );
		m_hDeathSound = pEnt;
	}
	else
	{
		Assert( 0 );
		EmitSound( "NPC_Gorilla.Death" );
	}
}

//-----------------------------------------------------------------------------
void CNPC_Gorilla::ImpactSound( void )
{
	EmitSound( "NPC_Gorilla.AttackHit" );
}

//=========================================================
// GetSoundInterests - returns a bit mask indicating which types
// of sounds this monster regards. 
//=========================================================
int CNPC_Gorilla::GetSoundInterests( void ) 
{
	return BaseClass::GetSoundInterests() | ALL_SCENTS;
}

//---------------------------------------------------------
void CNPC_Gorilla::RemoveIgnoredConditions( void )
{
	BaseClass::RemoveIgnoredConditions();

	if ( m_flHungryTime > gpGlobals->curtime )
		 ClearCondition( COND_SMELL );
}

//-----------------------------------------------------------------------------
// Purpose: Returns whether the enemy has been seen within the time period supplied
// Input  : flTime - Timespan we consider
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CNPC_Gorilla::SeenEnemyWithinTime( float flTime )
{
	float flLastSeenTime = GetEnemies()->LastTimeSeen( GetEnemy() );
	return ( flLastSeenTime != 0.0f && ( gpGlobals->curtime - flLastSeenTime ) < flTime );
}

//-----------------------------------------------------------------------------
int CNPC_Gorilla::SelectSchedule( void )
{
	// When a schedule ends, conditions are set back to what GatherConditions
	// reported before selecting a new schedule.  See CAI_BaseNPC::MaintainSchedule.
	// Anything RemoveIgnoredConditions did is lost.
	if ( m_flHungryTime > gpGlobals->curtime )
		 ClearCondition( COND_SMELL );

	if ( GetState() == NPC_STATE_COMBAT )
		return SelectCombatSchedule();

	if ( GetState() == NPC_STATE_ALERT && HasCondition( COND_ENEMY_DEAD ) )
		return SCHED_GORILLA_VICTORY_DANCE;

	if ( GetState() == NPC_STATE_ALERT && HasCondition( COND_SMELL ) )
		return SCHED_GORILLA_EAT;

	if ( GetState() == NPC_STATE_IDLE && HasCondition( COND_SMELL ) )
		return SCHED_GORILLA_EAT;

	return BaseClass::SelectSchedule();
}

//-----------------------------------------------------------------------------
int CNPC_Gorilla::SelectCombatSchedule( void )
{
	if ( HasCondition( COND_SMELL ) &&
		 HasCondition( COND_ENEMY_OCCLUDED ) &&
		 !SeenEnemyWithinTime( 10.0f ) &&
		 ( m_flLastDamageTime < gpGlobals->curtime - 10.0f ) )
		return SCHED_GORILLA_EAT;

	if ( HasCondition(COND_NEW_ENEMY) && gpGlobals->curtime - GetEnemies()->FirstTimeSeen(GetEnemy()) < 2.0 )
	{
		return SCHED_WAKE_ANGRY;
	}
	
	if ( HasCondition( COND_ENEMY_DEAD ) )
	{
		// clear the current (dead) enemy and try to find another.
		SetEnemy( NULL );
		 
		if ( ChooseEnemy() )
		{
			ClearCondition( COND_ENEMY_DEAD );
			return SelectSchedule();
		}

		SetState( NPC_STATE_ALERT );
		return SelectSchedule();
	}

	// If I'm scared of this enemy run away
	if ( IRelationType( GetEnemy() ) == D_FR )
	{
		if (HasCondition( COND_SEE_ENEMY )	|| 
			HasCondition( COND_LIGHT_DAMAGE )|| 
			HasCondition( COND_HEAVY_DAMAGE ))
		{
			FearSound();
			//ClearCommandGoal();
			return SCHED_RUN_FROM_ENEMY;
		}

		// If I've seen the enemy recently, cower. Ignore the time for unforgettable enemies.
		AI_EnemyInfo_t *pMemory = GetEnemies()->Find( GetEnemy() );
		if ( (pMemory && pMemory->bUnforgettable) || (GetEnemyLastTimeSeen() > (gpGlobals->curtime - 5.0)) )
		{
			// If we're facing him, just look ready. Otherwise, face him.
			if ( FInAimCone( GetEnemy()->EyePosition() ) )
				return SCHED_COMBAT_STAND;

			return SCHED_FEAR_FACE;
		}
	}

	// Can we see the enemy?
	if ( !HasCondition(COND_SEE_ENEMY) )
	{
		// enemy is unseen, but not occluded!
		// turn to face enemy
		if ( !HasCondition(COND_ENEMY_OCCLUDED) )
			return SCHED_COMBAT_FACE;

		// chase!
		if ( GetActiveWeapon() || (CapabilitiesGet() & (bits_CAP_INNATE_RANGE_ATTACK1|bits_CAP_INNATE_RANGE_ATTACK2)))
			return SCHED_ESTABLISH_LINE_OF_FIRE;
		else if ( (CapabilitiesGet() & (bits_CAP_INNATE_MELEE_ATTACK1|bits_CAP_INNATE_MELEE_ATTACK2)))
			return SCHED_CHASE_ENEMY;
		else
			return SCHED_TAKE_COVER_FROM_ENEMY;
	}

	if ( HasCondition(COND_TOO_CLOSE_TO_ATTACK) ) 
		return SCHED_BACK_AWAY_FROM_ENEMY;
	
	if ( HasCondition( COND_WEAPON_PLAYER_IN_SPREAD ) || 
			HasCondition( COND_WEAPON_BLOCKED_BY_FRIEND ) || 
			HasCondition( COND_WEAPON_SIGHT_OCCLUDED ) )
	{
		// if this fails then avoid range attack for a few seconds and chase enemy
		return SCHED_ESTABLISH_LINE_OF_FIRE;
	}

	if ( HasCondition(COND_CAN_RANGE_ATTACK1) )
		return SCHED_RANGE_ATTACK1;

	if ( HasCondition(COND_CAN_MELEE_ATTACK1) )
		return SCHED_MELEE_ATTACK1;

	return SCHED_CHASE_ENEMY;
}

//-----------------------------------------------------------------------------
int CNPC_Gorilla::SelectFailSchedule( int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode )
{
	// If we can't jump then pound the crud outta them!
	if ( failedSchedule == SCHED_ESTABLISH_LINE_OF_FIRE )
		return SCHED_CHASE_ENEMY;

	return BaseClass::SelectFailSchedule( failedSchedule, failedTask, taskFailCode );
}

//-----------------------------------------------------------------------------
int CNPC_Gorilla::TranslateSchedule( int scheduleType )
{
	switch( scheduleType )
	{
	case SCHED_MELEE_ATTACK1:
		return SCHED_GORILLA_MELEE_ATTACK1;
	case SCHED_RANGE_ATTACK1:
		return SCHED_GORILLA_RANGE_ATTACK1;
	}

	return BaseClass::TranslateSchedule( scheduleType );
}

//-----------------------------------------------------------------------------
// Purpose: Allows for modification of the interrupt mask for the current schedule.
//			In the most cases the base implementation should be called first.
//-----------------------------------------------------------------------------
void CNPC_Gorilla::BuildScheduleTestBits( void )
{
	// Ignore damage if we were recently damaged or we're attacking.
	if ( GetActivity() == ACT_MELEE_ATTACK1 )
	{
		ClearCustomInterruptCondition( COND_LIGHT_DAMAGE );
		ClearCustomInterruptCondition( COND_HEAVY_DAMAGE );
	}
#ifndef HL2_EPISODIC
	else if ( m_flNextFlinch >= gpGlobals->curtime )
	{
		ClearCustomInterruptCondition( COND_LIGHT_DAMAGE );
		ClearCustomInterruptCondition( COND_HEAVY_DAMAGE );
	}
#endif // !HL2_EPISODIC

	BaseClass::BuildScheduleTestBits();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Gorilla::StartTask( const Task_t *pTask )
{
	switch( pTask->iTask )
	{
	case TASK_ANNOUNCE_ATTACK:
		{
			EmitSound("NPC_Gorilla.AnnounceAttack"); // could I use responserules?
			BaseClass::StartTask( pTask );
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
	case TASK_GORILLA_DONT_EAT: // Don't eat - set me to not hungry for a while
		m_flHungryTime = gpGlobals->curtime + pTask->flTaskData;
		TaskComplete();
		break;
	case TASK_GORILLA_EAT: // Eat - set me to not hungry for a while AND increase my health
		m_flHungryTime = gpGlobals->curtime + pTask->flTaskData;
		TakeHealth( pTask->flTaskData, DMG_GENERIC );
		TaskComplete();
		break;
	case TASK_GORILLA_SOUND_EAT:
		if ( pTask->flTaskData == 0 || random->RandomFloat(0, 1) <= pTask->flTaskData )
			EmitSound( "NPC_Gorilla.Eat" );
		TaskComplete();
		break;
	case TASK_GORILLA_SOUND_VICTORY:
		EmitSound( "NPC_Gorilla.Victory" );
		TaskComplete();
		break;
	case TASK_PLAY_SEQUENCE:
		BaseClass::StartTask( pTask );
		if ( IsActivityFinished() )
			ResetSequenceInfo(); // hack for repeating ACT_GORILLA_EAT
		break;
	default:
		BaseClass::StartTask(pTask);
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Gorilla::RunTask( const Task_t *pTask )
{
	switch (pTask->iTask)
	{
		case TASK_RANGE_ATTACK1:
			if ( IsActivityFinished() )
			{
				TaskComplete();
				m_bMidJump = false;
				SetTouch( NULL );
				SetThink( &CNPC_Gorilla::CallNPCThink );
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
	case TASK_DIE:
#if 0
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
		BaseClass::RunTask(pTask);
		break;
	}
}

//-----------------------------------------------------------------------------
int CNPC_Gorilla::OnTakeDamage_Alive( const CTakeDamageInfo &info )
{
	// Disallow damage from server-side ragdolls.
	// I see these guys attacking a human whose ragdoll then kills them.
	if ( info.GetAttacker() &&
		FClassnameIs( info.GetAttacker(), "prop_ragdoll" ) )
	{
		return 0;
	}

	return BaseClass::OnTakeDamage_Alive( info );
}

//-----------------------------------------------------------------------------
bool CNPC_Gorilla::InnateWeaponLOSCondition( const Vector &ownerPos, const Vector &targetPos, bool bSetConditions )
{
#if 0
	// BUG in original antlion code?
	// antlion checks next attack time but this method is for determining LOS
	//   from a node not whether we can *currently* attack from this point
	if ( GetNextAttack() > gpGlobals->curtime )
		return false;
#endif

	if ( 1/*GetEnemy()->EyePosition().z - 36.0f > GetAbsOrigin().z*/ )
	{
		// Only run this test if trying to jump at a player who is higher up than me, else this 
		// code will always prevent a headcrab from jumping down at an enemy, and sometimes prevent it
		// jumping just slightly up at an enemy.
		Vector vStartHullTrace = ownerPos;
		vStartHullTrace.z += 1.0;

		// targetPos is either EyePosition() or (if that fails) BodyTarget() - we always jump at the eyes
		Vector vecEnemyEyePos = GetEnemy()->EyePosition();
		Vector vecWSC = ownerPos + CollisionProp()->OBBCenter();
		Vector vEndHullTrace = vecEnemyEyePos - vecWSC /*GetAbsOrigin()*/;
//		vEndHullTrace.NormalizeInPlace();
//		vEndHullTrace *= 30.0 /*8.0*/;
		vEndHullTrace += ownerPos;

		trace_t tr;
//		CTraceFilterSkipTwoEntities traceFilter( this, GetEnemy(), COLLISION_GROUP_NONE );
//		AI_TraceHull( vStartHullTrace, vEndHullTrace, GetHullMins(), GetHullMaxs(), MASK_NPCSOLID, &traceFilter, &tr );
		AI_TraceHull( vStartHullTrace, vEndHullTrace, GetHullMins(), GetHullMaxs(), MASK_NPCSOLID, this, GetCollisionGroup(), &tr );

		if ( tr.m_pEnt != NULL && tr.m_pEnt != GetEnemy() )
		{
			return false;
		}
	}

	return BaseClass::InnateWeaponLOSCondition( ownerPos, targetPos, bSetConditions );
}

//-----------------------------------------------------------------------------
int CNPC_Gorilla::RangeAttack1Conditions( float flDot, float flDist )
{
	if ( gpGlobals->curtime < GetNextAttack() )
		return COND_NONE;

	if ( ( GetFlags() & FL_ONGROUND ) == false )
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
	if ( flDist < GORILLA_MIN_JUMP_DIST )
		return COND_TOO_CLOSE_TO_ATTACK;

	if ( flDist > GORILLA_MAX_JUMP_DIST )
		return COND_TOO_FAR_TO_ATTACK;

	// Make sure the way is clear!
	CBaseEntity *pEnemy = GetEnemy();
	if( pEnemy )
	{
		bool bEnemyIsBullseye = ( dynamic_cast<CNPC_Bullseye *>(pEnemy) != NULL );

		trace_t tr;
		AI_TraceLine( EyePosition(), pEnemy->EyePosition(), MASK_SOLID, this, COLLISION_GROUP_NONE, &tr );

		if ( tr.m_pEnt != GetEnemy() )
		{
			if ( !bEnemyIsBullseye || tr.m_pEnt != NULL )
				return COND_WEAPON_SIGHT_OCCLUDED;
		}

		if ( GetEnemy()->EyePosition().z - 36.0f > GetAbsOrigin().z )
		{
			// Only run this test if trying to jump at a player who is higher up than me, else this 
			// code will always prevent a headcrab from jumping down at an enemy, and sometimes prevent it
			// jumping just slightly up at an enemy.
			Vector vStartHullTrace = GetAbsOrigin();
			vStartHullTrace.z += 1.0;

			Vector vEndHullTrace = GetEnemy()->EyePosition() - CollisionProp()->WorldSpaceCenter() /*GetAbsOrigin()*/;
//			vEndHullTrace.NormalizeInPlace();
//			vEndHullTrace *= 30.0 /*8.0*/;
			vEndHullTrace += GetAbsOrigin();

//			CTraceFilterSkipTwoEntities traceFilter( this, GetEnemy(), COLLISION_GROUP_NONE );
//			AI_TraceHull( vStartHullTrace, vEndHullTrace, GetHullMins(), GetHullMaxs(), MASK_NPCSOLID, &traceFilter, &tr );
			AI_TraceHull( vStartHullTrace, vEndHullTrace, GetHullMins(), GetHullMaxs(), MASK_NPCSOLID, this, GetCollisionGroup(), &tr );

			if ( tr.m_pEnt != NULL && tr.m_pEnt != GetEnemy() )
			{
				return COND_WEAPON_SIGHT_OCCLUDED;
			}
		}
	}

	return COND_CAN_RANGE_ATTACK1;
}

AI_BEGIN_CUSTOM_NPC( npc_gorilla, CNPC_Gorilla )

	DECLARE_TASK( TASK_GORILLA_SOUND_VICTORY )
	DECLARE_TASK( TASK_GORILLA_DONT_EAT )
	DECLARE_TASK( TASK_GORILLA_EAT )
	DECLARE_TASK( TASK_GORILLA_SOUND_EAT )

	DECLARE_ANIMEVENT( AE_GORILLA_ATTACK_LEFT )
	DECLARE_ANIMEVENT( AE_GORILLA_ATTACK_RIGHT )
	DECLARE_ANIMEVENT( AE_GORILLA_ATTACK_BOTH )
	DECLARE_ANIMEVENT( AE_GORILLA_JUMPATTACK )

	DECLARE_ACTIVITY( ACT_GORILLA_EAT )

	//=========================================================
	// BaseNPC behaviour will stop in the middle of a swing if the
	// zombie takes damage (among other things).
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_GORILLA_MELEE_ATTACK1,

		"	Tasks"
		"		TASK_STOP_MOVING		0"
		"		TASK_FACE_ENEMY			0"
		"		TASK_ANNOUNCE_ATTACK	1"	// 1 = primary attack
		"		TASK_MELEE_ATTACK1		0"
//		"		TASK_SET_SCHEDULE		SCHEDULE:SCHED_ZOMBIE_POST_MELEE_WAIT"
		""
		"	Interrupts"
		"		COND_LIGHT_DAMAGE"
		"		COND_HEAVY_DAMAGE"
	)	

	DEFINE_SCHEDULE
	(
		SCHED_GORILLA_RANGE_ATTACK1,

		"	Tasks"
		"		TASK_STOP_MOVING			0"
		"		TASK_FACE_ENEMY				0"
		"		TASK_ANNOUNCE_ATTACK		0"
		"		TASK_RANGE_ATTACK1			0"
		"		TASK_SET_ACTIVITY			ACTIVITY:ACT_IDLE"
		"		TASK_FACE_IDEAL				0"
		"		TASK_WAIT_RANDOM			0.5"
		""
		"	Interrupts"
		"		COND_ENEMY_OCCLUDED"
//		"		COND_NO_PRIMARY_AMMO"
	)

	DEFINE_SCHEDULE
	(
		SCHED_GORILLA_EAT,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_IDLE_STAND"
		"		TASK_STOP_MOVING				0"
		"		TASK_GORILLA_DONT_EAT			10"
		"		TASK_STORE_LASTPOSITION			0"
		"		TASK_GET_PATH_TO_BESTSCENT		0"
		"		TASK_FACE_IDEAL					0"
		"		TASK_WALK_PATH_WITHIN_DIST		64"
		"		TASK_STOP_MOVING				0"
		"		TASK_FACE_SAVEPOSITION			0" // see TASK_GET_PATH_TO_BESTSCENT
		"		TASK_GORILLA_SOUND_EAT			0"
		"		TASK_PLAY_SEQUENCE				ACTIVITY:ACT_GORILLA_EAT"
		"		TASK_GORILLA_SOUND_EAT			0"
		"		TASK_PLAY_SEQUENCE				ACTIVITY:ACT_GORILLA_EAT"
		"		TASK_GORILLA_SOUND_EAT			0"
		"		TASK_PLAY_SEQUENCE				ACTIVITY:ACT_GORILLA_EAT"
		"		TASK_GORILLA_EAT				50"
		"		TASK_GET_PATH_TO_LASTPOSITION	0"
		"		TASK_FACE_IDEAL					0"
		"		TASK_WALK_PATH					0"
		"		TASK_WAIT_FOR_MOVEMENT			0"
		"		TASK_CLEAR_LASTPOSITION			0"
		""
		"	Interrupts"
		"		COND_LIGHT_DAMAGE"
		"		COND_HEAVY_DAMAGE"
		"		COND_NEW_ENEMY"
//		"		COND_SMELL"
	)	

	DEFINE_SCHEDULE
	(
		SCHED_GORILLA_VICTORY_DANCE,

		"	Tasks"
		"		TASK_STOP_MOVING			0"
		"		TASK_FACE_ENEMY				0"
		"		TASK_GORILLA_SOUND_VICTORY	0"
		"		TASK_PLAY_SEQUENCE			ACTIVITY:ACT_VICTORY_DANCE"
		"		TASK_WAIT					0"
		""
		"	Interrupts"
		"		COND_LIGHT_DAMAGE"
		"		COND_HEAVY_DAMAGE"
		"		COND_NEW_ENEMY"
	)

AI_END_CUSTOM_NPC()
