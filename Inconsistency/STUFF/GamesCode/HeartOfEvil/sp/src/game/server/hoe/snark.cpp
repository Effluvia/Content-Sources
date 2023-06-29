#include "cbase.h"
#include "ai_basenpc.h"
#include "npcevent.h"
#include "movevars_shared.h" // sv_gravity
#include "npc_bullseye.h"
#include "SoundEmitterSystem/isoundemittersystembase.h"
#include "CollisionUtils.h"

// weapon_snark is at the bottom of this file
#include "basehlcombatweapon.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

const int SNARK_MIN_JUMP_DIST = 48;
const int SNARK_MAX_JUMP_DIST = 256;

class CNPC_Snark : public CAI_BaseNPC
{
public:
	DECLARE_CLASS( CNPC_Snark, CAI_BaseNPC );
	DECLARE_DATADESC();
	DEFINE_CUSTOM_AI;

	void Spawn( void );
	void Precache( void );

	Class_T Classify( void );
	bool ClassifyPlayerAlly( void ) { return false; }
	bool ClassifyPlayerAllyVital( void ) { return false; }

	void AttackSound( void );
	void AttackHitSound( void );
	void AttackMissSound( void );
	void BiteSound( void );
	void ImpactSound( void );

	void PrescheduleThink( void );
	int TranslateSchedule( int scheduleType );
	void StartTask( const Task_t *pTask );
	void RunTask( const Task_t *pTask );

	Activity NPC_TranslateActivity( Activity eNewActivity ) { return ACT_RUN; }

	CBaseEntity *BestEnemy( void );
	bool QueryHearSound( CSound *pSound );
	bool QuerySeeEntity( CBaseEntity *pEntity, bool bOnlyHateOrFearIfNPC );

	float GetIdealSpeed( void ) const;

	virtual float InnateRange1MinRange( void ) { return SNARK_MIN_JUMP_DIST; }
	virtual float InnateRange1MaxRange( void ) { return SNARK_MAX_JUMP_DIST; }

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
	void Event_Killed( const CTakeDamageInfo &info );

	bool	m_bMidJump;
	bool	m_bAttackFailed;		// whether we ran into a wall during a jump.
	Vector	m_vecCommittedJumpPos;	// The position of our enemy when we locked in our jump attack.
	float	m_flNextNPCThink;
	float	m_flIgnoreWorldCollisionTime;
	float	m_flTimeDie;
	int		m_iDamage;
	float	m_flTimeIgnoreThrower; // time after thrown that we stop ignoring our thrower
	EHANDLE m_hThrower; // player that threw us

	//-----------------------------------------------------------------------------
	// Custom schedules.
	//-----------------------------------------------------------------------------
	enum
	{
		SCHED_SNARK_RANGE_ATTACK1 = BaseClass::NEXT_SCHEDULE,
		NEXT_SCHEDULE
	};

	//-----------------------------------------------------------------------------
	// Tasks
	//-----------------------------------------------------------------------------
	enum
	{
		TASK_SNARK_RANGE_ATTACK1 = BaseClass::NEXT_TASK,
		NEXT_TASK
	};

};

LINK_ENTITY_TO_CLASS( npc_snark, CNPC_Snark );

//-----------------------------------------------------------------------------
// Spawnflags
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Animation events.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Conditions 
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Activities
//-----------------------------------------------------------------------------

BEGIN_DATADESC( CNPC_Snark )
	DEFINE_FIELD( m_bMidJump, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_vecCommittedJumpPos, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( m_flNextNPCThink, FIELD_TIME ),
	DEFINE_FIELD( m_flTimeDie, FIELD_TIME ),
	DEFINE_FIELD( m_iDamage, FIELD_INTEGER ),
	DEFINE_FIELD( m_flTimeIgnoreThrower, FIELD_TIME ),
	DEFINE_FIELD( m_hThrower, FIELD_EHANDLE ),

	DEFINE_THINKFUNC( ThrowThink ),
	DEFINE_ENTITYFUNC( LeapTouch ),
END_DATADESC()

#define SNARK_HEALTH 2
#define SNARK_IGNORE_WORLD_COLLISION_TIME 0.5
#define SNARK_DMG_BITE_NPC 5
#define SNARK_DMG_BITE_PLR 10
#define SNARK_DMG_POP 5 // in HL1 the damage goes up by 5 for each bite until the snark explodes
#define SNARK_DETONATE_DELAY 15.0

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Snark::Spawn( void )
{
	BaseClass::Spawn();

	Precache();
	SetModel( STRING( GetModelName() ) );

	SetHullType( HULL_TINY ); // 24x24 is too big actually
	UTIL_SetSize( this, Vector( -4, -4, 0), Vector(6, 4, 10) );

	SetSolid( SOLID_BBOX );
	AddSolidFlags( FSOLID_NOT_STANDABLE ); // you can't stand on this
	SetMoveType( MOVETYPE_STEP );

	CapabilitiesClear();
	CapabilitiesAdd( bits_CAP_MOVE_GROUND | bits_CAP_INNATE_RANGE_ATTACK1 );

	SetBloodColor( BLOOD_COLOR_YELLOW );
	m_NPCState = NPC_STATE_NONE;

	m_iHealth = SNARK_HEALTH;

	m_flTimeDie = gpGlobals->curtime + SNARK_DETONATE_DELAY;
	m_iDamage = SNARK_DMG_POP;

	NPCInit();

	// AFTER NPCInit()

	m_flFieldOfView = 0; // 180 degrees
	m_flFieldOfView = 0.5;
	SetViewOffset( Vector ( 0, 0, 4 ) );		// Position of the eyes relative to NPC's origin.
}

//-----------------------------------------------------------------------------
// Purpose: Precaches all resources this monster needs.
//-----------------------------------------------------------------------------
void CNPC_Snark::Precache( void )
{
	SetModelName( AllocPooledString( "models/w_squeak.mdl" ) );
	PrecacheModel( STRING( GetModelName() ) );

	PrecacheScriptSound( "Snark.Die" );
	PrecacheScriptSound( "Snark.Gibbed" );
	PrecacheScriptSound( "Snark.Squeak" );
	PrecacheScriptSound( "Snark.Deploy" );
	PrecacheScriptSound( "Snark.Bounce" );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Indicates this monster's place in the relationship table.
// Output : 
//-----------------------------------------------------------------------------
Class_T	CNPC_Snark::Classify( void )
{
	return CLASS_SNARK; 
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Snark::ThrowThink( void )
{
	if ( gpGlobals->curtime > m_flNextNPCThink )
	{
		NPCThink();
		m_flNextNPCThink = gpGlobals->curtime + 0.1;
	}

	if ( GetFlags() & FL_ONGROUND )
	{
		SetThink( &CNPC_Snark::CallNPCThink );
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
void CNPC_Snark::JumpAttack( bool bRandomJump, const Vector &vecPos, bool bThrown )
{
	Vector vecJumpVel;
	if ( !bRandomJump )
	{
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

	AttackSound();
	Leap( vecJumpVel );
}

//-----------------------------------------------------------------------------
void CNPC_Snark::MoveOrigin( const Vector &vecDelta )
{
	UTIL_SetOrigin( this, GetLocalOrigin() + vecDelta );
}

//-----------------------------------------------------------------------------
void CNPC_Snark::Leap( const Vector &vecVel )
{
	SetTouch( &CNPC_Snark::LeapTouch );

	SetCondition( COND_FLOATING_OFF_GROUND );
	SetGroundEntity( NULL );

	m_flIgnoreWorldCollisionTime = gpGlobals->curtime + SNARK_IGNORE_WORLD_COLLISION_TIME;

	if ( HasHeadroom() )
	{
		// Take him off ground so engine doesn't instantly reset FL_ONGROUND.
		MoveOrigin( Vector( 0, 0, 1 ) );
	}

	SetAbsVelocity( vecVel );

	// Think every frame so the player sees the headcrab where he actually is...
	m_bMidJump = true;
	SetThink( &CNPC_Snark::ThrowThink );
	SetNextThink( gpGlobals->curtime );
}

//-----------------------------------------------------------------------------
// Before jumping, headcrabs usually use SetOrigin() to lift themselves off the 
// ground. If the headcrab doesn't have the clearance to so, they'll be stuck
// in the world. So this function makes sure there's headroom first.
//-----------------------------------------------------------------------------
bool CNPC_Snark::HasHeadroom()
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
void CNPC_Snark::LeapTouch( CBaseEntity *pOther )
{
	m_bMidJump = false;

	if ( IRelationType( pOther ) == D_HT )
	{
		// Don't hit if back on ground
		if ( !( GetFlags() & FL_ONGROUND ) )
		{
			// Check for a bite and not jumping on someone's head etc
			Vector forward;
			GetVectors( &forward, NULL, NULL );
			trace_t tr;
			AI_TraceLine( EyePosition(), EyePosition() + forward * 32, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );

			if ( pOther->m_takedamage != DAMAGE_NO && tr.fraction < 1.0f && tr.m_pEnt == pOther )
			{
				BiteSound();
				TouchDamage( pOther );

				if ( pOther->BloodColor() != DONT_BLEED )
				{
//					UTIL_BloodSpray( tr.endpos, tr.plane.normal, pOther->BloodColor(), 1, FX_BLOODSPRAY_CLOUD );
					UTIL_BloodImpact( tr.endpos, tr.plane.normal, pOther->BloodColor(), 5 );
			}

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

		ImpactSound();

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
	else
	{
		ImpactSound();
	}

	// Shut off the touch function.
	SetTouch( NULL );
	SetThink ( &CNPC_Snark::CallNPCThink );
}

//-----------------------------------------------------------------------------
int CNPC_Snark::CalcDamageInfo( CBaseEntity *pVictim, CTakeDamageInfo *pInfo )
{
	Assert(pVictim != NULL);
	pInfo->Set( this, this, pVictim->IsPlayer() ? SNARK_DMG_BITE_PLR : SNARK_DMG_BITE_NPC, DMG_SLASH );
	CalculateMeleeDamageForce( pInfo, GetAbsVelocity(), GetAbsOrigin() );
	return pInfo->GetDamage();
}

//-----------------------------------------------------------------------------
void CNPC_Snark::TouchDamage( CBaseEntity *pOther )
{
	CTakeDamageInfo info;
	CalcDamageInfo( pOther, &info );
	pOther->TakeDamage( info  );

	 // in HL1 the damage goes up by 5 for each bite until the snark explodes
	m_iDamage += SNARK_DMG_POP;
}

//-----------------------------------------------------------------------------
void CNPC_Snark::AttackSound( void )
{
//	EmitSound( "Snark.Attack" );
}

//-----------------------------------------------------------------------------
void CNPC_Snark::AttackHitSound( void )
{
//	EmitSound( "Snark.Deploy" );
}

//-----------------------------------------------------------------------------
void CNPC_Snark::AttackMissSound( void )
{
//	EmitSound( "NPC_SpxBaby.AttackMiss" );
}

//-----------------------------------------------------------------------------
void CNPC_Snark::BiteSound( void )
{
	CSoundParameters params;
	if ( GetParametersForSound( "Snark.Deploy", params, NULL ) )
	{
		EmitSound_t ep( params );

		// higher pitch as squeeker gets closer to detonation time
		ep.m_nPitch = 155.0 - 60.0 * ((m_flTimeDie - gpGlobals->curtime) / SNARK_DETONATE_DELAY);

		CPASAttenuationFilter filter( this );
		EmitSound( filter, entindex(), ep );
	}
}

//-----------------------------------------------------------------------------
void CNPC_Snark::ImpactSound( void )
{
	CSoundParameters params;
	if ( GetParametersForSound( "Snark.Bounce", params, NULL ) )
	{
		EmitSound_t ep( params );

		// higher pitch as squeeker gets closer to detonation time
		ep.m_nPitch = 155.0 - 60.0 * ((m_flTimeDie - gpGlobals->curtime) / SNARK_DETONATE_DELAY);

		CPASAttenuationFilter filter( this );
		EmitSound( filter, entindex(), ep );
	}
}

//-----------------------------------------------------------------------------
float CNPC_Snark::GetIdealSpeed( void ) const
{
	// Affects the ground distance covered by ACT_RUN
	return BaseClass::GetIdealSpeed() * 3.0f;
}

//---------------------------------------------------------
void CNPC_Snark::PrescheduleThink( void )
{
	BaseClass::PrescheduleThink();

	// Become solid to our thrower as soon as we have left their body
	// FIXME: fugly
	if ( GetOwnerEntity() )
	{
		Vector ownerMins, ownerMaxs;
		GetOwnerEntity()->CollisionProp()->WorldSpaceSurroundingBounds( &ownerMins, &ownerMaxs );

		Vector myMins, myMaxs;
		CollisionProp()->WorldSpaceSurroundingBounds( &myMins, &myMaxs );

		if ( !IsBoxIntersectingBox( ownerMins, ownerMaxs, myMins, myMaxs ) )
			SetOwnerEntity( NULL );
	}

	// explode when ready
	if ( m_flTimeDie < gpGlobals->curtime )
	{
		TakeDamage( CTakeDamageInfo( this, this, GetHealth(), DMG_GENERIC ) );
		return;
	}

	// squeek if it's about time blow up
	if ((m_flTimeDie - gpGlobals->curtime <= 0.5) && (m_flTimeDie - gpGlobals->curtime >= 0.3))
	{
		EmitSound( "Snark.Squeak" );
		CSoundEnt::InsertSound( SOUND_COMBAT, GetAbsOrigin(), 256, 0.25, this );
	}
}

//---------------------------------------------------------
int CNPC_Snark::TranslateSchedule( int scheduleType )
{
	switch( scheduleType )
	{
	case SCHED_RANGE_ATTACK1:
		return SCHED_SNARK_RANGE_ATTACK1;
	case SCHED_TAKE_COVER_FROM_ENEMY:
		return SCHED_CHASE_ENEMY;
	}

	return BaseClass::TranslateSchedule( scheduleType );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Snark::StartTask( const Task_t *pTask )
{
	switch( pTask->iTask )
	{
	case TASK_SNARK_RANGE_ATTACK1:
		{
			// Ignore if we're in mid air
			if ( m_bMidJump )
				return;

			CBaseEntity *pEnemy = GetEnemy();
				
			if ( pEnemy )
			{
				JumpAttack( false, pEnemy->BodyTarget( GetAbsOrigin() ) );				
			}
			else
			{
				// Jump hop, don't care where.
				JumpAttack( true );
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
void CNPC_Snark::RunTask( const Task_t *pTask )
{
	switch (pTask->iTask)
	{
		case TASK_SNARK_RANGE_ATTACK1:
			//if ( IsActivityFinished() )
			if ( m_pfnTouch == NULL )
			{
				TaskComplete();
				m_bMidJump = false;
				SetTouch( NULL );
				SetThink( &CNPC_Snark::CallNPCThink );
				SetIdealActivity( ACT_IDLE );

				if ( m_bAttackFailed )
				{
					// our attack failed because we just ran into something solid.
					// delay attacking for a while so we don't just repeatedly leap
					// at the enemy from a bad location.
					m_bAttackFailed = false;
					SetNextAttack( gpGlobals->curtime + 1.2f );
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
CBaseEntity *CNPC_Snark::BestEnemy( void )
{
	if ( GetEnemy() != NULL && HasCondition( COND_SEE_ENEMY ) )
		return GetEnemy();

	return BaseClass::BestEnemy();
}

//-----------------------------------------------------------------------------
bool CNPC_Snark::QueryHearSound( CSound *pSound )
{
	if ( GetEnemy() != NULL )
		return false;

	if ( /*m_hThrower && ( pSound->m_hOwner == m_hThrower ) &&*/
		( m_flTimeIgnoreThrower > gpGlobals->curtime ) )
		return false;

	return BaseClass::QueryHearSound( pSound );
}

//-----------------------------------------------------------------------------
bool CNPC_Snark::QuerySeeEntity( CBaseEntity *pEntity, bool bOnlyHateOrFearIfNPC )
{
	if ( m_hThrower && ( pEntity == m_hThrower ) &&
		( m_flTimeIgnoreThrower > gpGlobals->curtime ) )
		return false;

	return BaseClass::QuerySeeEntity( pEntity, bOnlyHateOrFearIfNPC );
}

//-----------------------------------------------------------------------------
// Purpose: For innate melee attack
// Input  :
// Output :
//-----------------------------------------------------------------------------
int CNPC_Snark::RangeAttack1Conditions( float flDot, float flDist )
{
	if ( gpGlobals->curtime < GetNextAttack() )
		return COND_NONE;

	if ( ( GetFlags() & FL_ONGROUND ) == false )
		return COND_NONE;
#if 0
	// When we're burrowed ignore facing, because when we unburrow we'll cheat and face our enemy.
	if ( !m_bBurrowed && ( flDot < 0.65 ) )
		return COND_NOT_FACING_ATTACK;

	// This code stops lots of headcrabs swarming you and blocking you
	// whilst jumping up and down in your face over and over. It forces
	// them to back up a bit. If this causes problems, consider using it
	// for the fast headcrabs only, rather than just removing it.(sjb)
	if ( flDist < SNARK_MIN_JUMP_DIST )
		return COND_TOO_CLOSE_TO_ATTACK;
#endif

	if ( flDist > SNARK_MAX_JUMP_DIST )
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
				return COND_NONE;
		}

		if ( GetEnemy()->EyePosition().z - 36.0f > GetAbsOrigin().z )
		{
			// Only run this test if trying to jump at a player who is higher up than me, else this 
			// code will always prevent a headcrab from jumping down at an enemy, and sometimes prevent it
			// jumping just slightly up at an enemy.
			Vector vStartHullTrace = GetAbsOrigin();
			vStartHullTrace.z += 1.0;

			Vector vEndHullTrace = GetEnemy()->EyePosition() - WorldSpaceCenter();
//			vEndHullTrace.NormalizeInPlace();
//			vEndHullTrace *= 8.0;
			vEndHullTrace += GetAbsOrigin();

			AI_TraceHull( vStartHullTrace, vEndHullTrace,GetHullMins(), GetHullMaxs(), MASK_NPCSOLID, this, GetCollisionGroup(), &tr );

			if ( tr.m_pEnt != NULL && tr.m_pEnt != GetEnemy() )
			{
				return COND_WEAPON_SIGHT_OCCLUDED;
			}
		}
	}

	return COND_CAN_RANGE_ATTACK1;
}

//-----------------------------------------------------------------------------
void CNPC_Snark::TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr )
{
	// NPCs tend to peg these guys off in mid-jump with laser-like accuracy
	// so force some shots to "miss".
	if ( info.GetDamageType() & DMG_BULLET &&
		info.GetAttacker() &&
		info.GetAttacker()->IsNPC() )
	{
		float flChanceToMiss = 0.95;
//		if ( !(GetFlags() & FL_ONGROUND ) )
//			flChanceToMiss = 0.7;
//		else if ( IsMoving() )
//			flChanceToMiss = 0.5;
		if ( random->RandomFloat(0,1) <= flChanceToMiss )
			return;
	}

	return BaseClass::TraceAttack( info, vecDir, ptr, 0 );
}

//-----------------------------------------------------------------------------
void CNPC_Snark::Event_Killed( const CTakeDamageInfo &info )
{
	m_takedamage = DAMAGE_NO;

	EmitSound( "Snark.Die" );

	UTIL_BloodImpact( WorldSpaceCenter(), vec3_origin, BLOOD_COLOR_YELLOW, 100 );

	CTakeDamageInfo dmgInfo( this, this, m_iDamage, DMG_BLAST );
	RadiusDamage( dmgInfo, GetAbsOrigin(), m_iDamage * 2.5 /* HL1 */, CLASS_NONE, NULL );

	EmitSound( "Snark.Gibbed" );

//	CTakeDamageInfo	info( this, this, 1, DMG_GENERIC );
	BaseClass::Event_Killed( info );

	// Remove myself a frame from now to avoid doing it in the middle of running AI
	RemoveDeferred();
}

AI_BEGIN_CUSTOM_NPC( npc_snark, CNPC_Snark )

	DECLARE_TASK( TASK_SNARK_RANGE_ATTACK1 )

	DEFINE_SCHEDULE
	(
		SCHED_SNARK_RANGE_ATTACK1,

		"	Tasks"
		"		TASK_FACE_ENEMY				0"
		"		TASK_ANNOUNCE_ATTACK		0"
		"		TASK_SNARK_RANGE_ATTACK1	0"
		"		TASK_WAIT					0.2"
		"		TASK_FACE_ENEMY				0"
		"		TASK_SNARK_RANGE_ATTACK1	0"
		"		TASK_WAIT					0.2"
		"		TASK_FACE_ENEMY				0"
		"		TASK_SNARK_RANGE_ATTACK1	0"
		""
		"	Interrupts"
		"		COND_ENEMY_OCCLUDED"
	)

AI_END_CUSTOM_NPC()

//-----------------------------------------------------------------------------

class CWeaponSnark : public CBaseHLCombatWeapon
{
public:
	DECLARE_CLASS( CWeaponSnark, CBaseHLCombatWeapon );
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	int CapabilitiesGet( void ) { return bits_CAP_WEAPON_RANGE_ATTACK1; }

	void Precache( void );
	void Spawn( void );

	void PrimaryAttack( void );
	void CheckThrowPosition( CBasePlayer *pPlayer, const Vector &vecEye, Vector &vecSrc );

	void Equip( CBaseCombatCharacter *pOwner );
	void AnimateThink( void );
};

IMPLEMENT_SERVERCLASS_ST(CWeaponSnark, DT_WeaponSnark)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_snark, CWeaponSnark );
PRECACHE_WEAPON_REGISTER(weapon_snark);

BEGIN_DATADESC( CWeaponSnark )
	DEFINE_THINKFUNC( AnimateThink )
END_DATADESC()

#define SNARK_ANIMATE_THINK_CONTEXT "SnarkAnimateThinkContext"

//-----------------------------------------------------------------------------
void CWeaponSnark::Spawn( void )
{
	BaseClass::Spawn();

	SetSize( Vector( -12, -12, 0 ), Vector( 12, 12, 16 ) );

	SetSequence( LookupSequence( "idle" ) );
	ResetSequenceInfo();

	SetContextThink( &CWeaponSnark::AnimateThink, gpGlobals->curtime, SNARK_ANIMATE_THINK_CONTEXT );
}

//-----------------------------------------------------------------------------
void CWeaponSnark::Precache( void )
{
	UTIL_PrecacheOther( "npc_snark" );

	PrecacheScriptSound( "Snark.Bounce" );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
void CWeaponSnark::PrimaryAttack( void )
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if ( pPlayer == NULL )
		return;

	SendWeaponAnim( ACT_VM_THROW );

	Vector	vecEye = pPlayer->EyePosition();
	Vector	vForward, vRight;

	pPlayer->EyeVectors( &vForward, &vRight, NULL );
	Vector vecSrc = vecEye + vForward * 20.0f - Vector( 0, 0, 10 );
	CheckThrowPosition( pPlayer, vecEye, vecSrc );
	vForward[2] += 0.1f;

	Vector vecThrow;
	pPlayer->GetVelocity( &vecThrow, NULL );
	vecThrow += vForward * 200;

	QAngle angles = GetAbsAngles();
	angles.x = angles.z = 0;

	CNPC_Snark *pSnark = (CNPC_Snark *) CBaseEntity::Create( "npc_snark", vecSrc, angles, pPlayer );

	pSnark->AddSpawnFlags( SF_NPC_FALL_TO_GROUND );
	pSnark->SetAbsVelocity( vecThrow );

	pSnark->m_hThrower = pPlayer;
	pSnark->m_flTimeIgnoreThrower = gpGlobals->curtime + 3.0;

	EmitSound( "Snark.Bounce" );

	m_flNextPrimaryAttack = gpGlobals->curtime + 0.3;

	pPlayer->RemoveAmmo( 1, m_iPrimaryAmmoType );

	// If I'm now out of ammo, switch away
	if ( !HasPrimaryAmmo() )
	{
		pPlayer->SwitchToNextBestWeapon( this );
	}
}

//-----------------------------------------------------------------------------
// check a throw from vecSrc.  If not valid, move the position back along the line to vecEye
void CWeaponSnark::CheckThrowPosition( CBasePlayer *pPlayer, const Vector &vecEye, Vector &vecSrc )
{
	trace_t tr;

#define GRENADE_RADIUS	6.0f // inches

	UTIL_TraceHull( vecEye, vecSrc,
		-Vector(GRENADE_RADIUS+2,GRENADE_RADIUS+2,GRENADE_RADIUS+2),
		Vector(GRENADE_RADIUS+2,GRENADE_RADIUS+2,GRENADE_RADIUS+2), 
		pPlayer->PhysicsSolidMaskForEntity(), pPlayer, pPlayer->GetCollisionGroup(), &tr );
	
	if ( tr.DidHit() )
	{
		vecSrc = tr.endpos;
	}
}

//-----------------------------------------------------------------------------
void CWeaponSnark::Equip( CBaseCombatCharacter *pOwner )
{
	SetContextThink( NULL, TICK_NEVER_THINK, SNARK_ANIMATE_THINK_CONTEXT );

	BaseClass::Equip( pOwner );
}

//-----------------------------------------------------------------------------
void CWeaponSnark::AnimateThink( void )
{
	if ( !UTIL_FindClientInPVS( edict() ) )
	{
		SetNextThink( gpGlobals->curtime + random->RandomFloat( 1.0f, 1.5f ), SNARK_ANIMATE_THINK_CONTEXT );
		return;
	}

	StudioFrameAdvance();
	DispatchAnimEvents( this );

	SetNextThink( gpGlobals->curtime + 0.1, SNARK_ANIMATE_THINK_CONTEXT );
}