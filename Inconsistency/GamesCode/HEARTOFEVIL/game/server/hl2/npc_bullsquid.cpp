//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implements the bullsquid
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "game.h"
#include "AI_Default.h"
#include "AI_Schedule.h"
#include "AI_Hull.h"
#include "AI_Navigator.h"
#include "AI_Motor.h"
#include "ai_squad.h"
#include "npc_bullsquid.h"
#include "npcevent.h"
#include "soundent.h"
#include "activitylist.h"
#include "weapon_brickbat.h"
#include "npc_headcrab.h"
#include "player.h"
#include "gamerules.h"		// For g_pGameRules
#include "ammodef.h"
#include "grenade_spit_bullsquid.h"
#include "grenade_brickbat.h"
#include "entitylist.h"
#include "shake.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "movevars_shared.h"
#include "particle_parse.h"
#include "AI_Hint.h"
#include "AI_Senses.h"
#include "ai_pathfinder.h"
#include "ai_waypoint.h"
#ifdef HOE_DLL
#include "hoe_deathsound.h"
#endif // HOE_DLL

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define		SQUID_SPRINT_DIST	256 // how close the squid has to get before starting to sprint and refusing to swerve

ConVar sk_bullsquid_health( "sk_bullsquid_health", "0" );
ConVar sk_bullsquid_dmg_bite( "sk_bullsquid_dmg_bite", "0" );
ConVar sk_bullsquid_dmg_whip( "sk_bullsquid_dmg_whip", "0" );
ConVar sk_bullsquid_spit_speed( "sk_bullsquid_spit_speed", "0", FCVAR_NONE, "Speed at which an bullsquid spit grenade travels." );

//=========================================================
// monster-specific schedule types
//=========================================================
enum
{
	SCHED_SQUID_HURTHOP = LAST_SHARED_SCHEDULE + 1,
	SCHED_SQUID_SEECRAB,
	SCHED_SQUID_EAT,
	SCHED_SQUID_SNIFF_AND_EAT,
	SCHED_SQUID_WALLOW,
};

//=========================================================
// monster-specific tasks
//=========================================================
enum 
{
	TASK_SQUID_HOPTURN = LAST_SHARED_TASK + 1,
	TASK_SQUID_EAT,
};

//-----------------------------------------------------------------------------
// Squid Conditions
//-----------------------------------------------------------------------------
enum
{
	COND_SQUID_SMELL_FOOD	= LAST_SHARED_CONDITION + 1,
};


//=========================================================
// Interactions
//=========================================================
int	g_interactionBullsquidThrow		= 0;

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define		BSQUID_AE_SPIT		( 1 )
#define		BSQUID_AE_BITE		( 2 )
#define		BSQUID_AE_BLINK		( 3 )
#define		BSQUID_AE_ROAR		( 4 )
#define		BSQUID_AE_HOP		( 5 )
#define		BSQUID_AE_THROW		( 6 )
#define		BSQUID_AE_WHIP_SND	( 7 )

LINK_ENTITY_TO_CLASS( npc_bullsquid, CNPC_Bullsquid );

int ACT_SQUID_EXCITED;
int ACT_SQUID_EAT;
int ACT_SQUID_DETECT_SCENT;
int ACT_SQUID_INSPECT_FLOOR;


//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CNPC_Bullsquid )

	DEFINE_FIELD( m_fCanThreatDisplay,	FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flLastHurtTime,		FIELD_TIME ),
	DEFINE_FIELD( m_flNextSpitTime,		FIELD_TIME ),
//	DEFINE_FIELD( m_nSquidSpitSprite,	FIELD_INTEGER ),
	DEFINE_FIELD( m_flHungryTime,		FIELD_TIME ),
#ifdef HOE_DLL
	DEFINE_FIELD( m_flPainTime,		FIELD_TIME ),
#endif
	DEFINE_FIELD( m_nextSquidSoundTime,	FIELD_TIME ),
	DEFINE_FIELD( m_vecSaveSpitVelocity,	FIELD_VECTOR ),

END_DATADESC()


//=========================================================
// Spawn
//=========================================================
void CNPC_Bullsquid::Spawn()
{
	Precache( );

	SetModel( "models/bullsquid.mdl");
	SetHullType(HULL_WIDE_SHORT);
	SetHullSizeNormal();

	SetSolid( SOLID_BBOX );
	AddSolidFlags( FSOLID_NOT_STANDABLE );
	SetMoveType( MOVETYPE_STEP );
	m_bloodColor		= BLOOD_COLOR_GREEN;
	
	SetRenderColor( 255, 255, 255, 255 );
	
	m_iHealth			= sk_bullsquid_health.GetFloat();
	m_flFieldOfView		= 0.2;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_NPCState			= NPC_STATE_NONE;
	
	CapabilitiesClear();
	CapabilitiesAdd( bits_CAP_MOVE_GROUND | bits_CAP_INNATE_RANGE_ATTACK1 | bits_CAP_INNATE_MELEE_ATTACK1 | bits_CAP_INNATE_MELEE_ATTACK2 );
	
	m_fCanThreatDisplay	= TRUE;
	m_flNextSpitTime = gpGlobals->curtime;

	NPCInit();

//	m_flDistTooFar		= 784;
//	m_flDistTooFar		= InnateRange1MaxRange();
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CNPC_Bullsquid::Precache()
{
	PrecacheModel( "models/bullsquid.mdl" );
//	m_nSquidSpitSprite = PrecacheModel("sprites/greenspit1.vmt");// client side spittle.

	UTIL_PrecacheOther( "grenade_spit_bullsquid" );
	PrecacheParticleSystem( "blood_impact_yellow_01" );

	PrecacheScriptSound( "NPC_Bullsquid.Idle" );
	PrecacheScriptSound( "NPC_Bullsquid.Pain" );
	PrecacheScriptSound( "NPC_Bullsquid.Alert" );
	PrecacheScriptSound( "NPC_Bullsquid.Death" );
	PrecacheScriptSound( "NPC_Bullsquid.Attack1" );
	PrecacheScriptSound( "NPC_Bullsquid.Growl" );
	PrecacheScriptSound( "NPC_Bullsquid.TailWhip");
#ifdef HOE_DLL
	PrecacheScriptSound( "NPC_Bullsquid.TailHit");
#endif

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Indicates this monster's place in the relationship table.
// Output : 
//-----------------------------------------------------------------------------
Class_T	CNPC_Bullsquid::Classify( void )
{
	return CLASS_BULLSQUID; 
}

//=========================================================
// IdleSound 
//=========================================================
#define SQUID_ATTN_IDLE	(float)1.5
void CNPC_Bullsquid::IdleSound( void )
{
	EmitSound( "NPC_Bullsquid.Idle" );
}

//=========================================================
// PainSound 
//=========================================================
void CNPC_Bullsquid::PainSound( const CTakeDamageInfo &info )
{
#ifdef HOE_DLL
	// Make sure he doesn't just repeatedly go uh-uh-uh-uh-uh when hit by rapid fire
	if ( gpGlobals->curtime < m_flPainTime ) return;

	float flDuration;
	EmitSound( "NPC_Bullsquid.Pain", 0, &flDuration );

	flDuration = max( flDuration, 2.0f );
	m_flPainTime = gpGlobals->curtime + flDuration + random->RandomFloat( 2.0, 4.0 );
#else
	EmitSound( "NPC_Bullsquid.Pain" );
#endif
}

//=========================================================
// AlertSound
//=========================================================
void CNPC_Bullsquid::AlertSound( void )
{
	EmitSound( "NPC_Bullsquid.Alert" );
}

//=========================================================
// DeathSound
//=========================================================
void CNPC_Bullsquid::DeathSound( const CTakeDamageInfo &info )
{
#ifdef HOE_DLL
	CNPCDeathSound *pEnt = (CNPCDeathSound *) CBaseEntity::Create( "npc_death_sound", GetAbsOrigin(), GetAbsAngles(), NULL );
	if ( pEnt )
	{
		EmitSound( "AI_BaseNPC.SentenceStop" );
		Q_strcpy( pEnt->m_szSoundName.GetForModify(), "NPC_Bullsquid.Death" );
		m_hDeathSound = pEnt;
	}
	else
	{
		Assert( 0 );
		EmitSound( "NPC_Bullsquid.Death" );
	}
#else
	EmitSound( "NPC_Bullsquid.Death" );
#endif
}

//=========================================================
// AttackSound
//=========================================================
void CNPC_Bullsquid::AttackSound( void )
{
	EmitSound( "NPC_Bullsquid.Attack1" );
}

//=========================================================
// GrowlSound
//=========================================================
void CNPC_Bullsquid::GrowlSound( void )
{
	if (gpGlobals->curtime >= m_nextSquidSoundTime)
	{
		EmitSound( "NPC_Bullsquid.Growl" );
		m_nextSquidSoundTime	= gpGlobals->curtime + random->RandomInt(1.5,3.0);
	}
}


//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
float CNPC_Bullsquid::MaxYawSpeed( void )
{
	float flYS = 0;

	switch ( GetActivity() )
	{
	case	ACT_WALK:			flYS = 90;	break;
	case	ACT_RUN:			flYS = 90;	break;
	case	ACT_IDLE:			flYS = 90;	break;
	case	ACT_RANGE_ATTACK1:	flYS = 90;	break;
	default:
		flYS = 90;
		break;
	}

	return flYS;
}

//-----------------------------------------------------------------------------
// Purpose: Returns whether the enemy has been seen within the time period supplied
// Input  : flTime - Timespan we consider
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CNPC_Bullsquid::SeenEnemyWithinTime( float flTime )
{
	float flLastSeenTime = GetEnemies()->LastTimeSeen( GetEnemy() );
	return ( flLastSeenTime != 0.0f && ( gpGlobals->curtime - flLastSeenTime ) < flTime );
}

//-----------------------------------------------------------------------------
// Purpose: Test whether this antlion can hit the target
//-----------------------------------------------------------------------------
bool CNPC_Bullsquid::InnateWeaponLOSCondition( const Vector &ownerPos, const Vector &targetPos, bool bSetConditions )
{
#if 1

	if ( GetEnemy() != NULL )
	{
		// Don't spit up/down at too sharp an angle
		Vector vecToEnemy = targetPos - ownerPos;
		Vector vHeadDir = targetPos - ownerPos;
		vHeadDir.z = 0;
		VectorNormalize( vecToEnemy );
		VectorNormalize( vHeadDir );
		float flDotZ = DotProduct( vecToEnemy, vHeadDir );
		if ( flDotZ < 0.6 )
			return false;
	}

	// BUG in original antlion code?
	// 1) antlion checks next attack time but this method is for determining LOS
	//   from a node not whether we can *currently* attack from this point
	// 2) antlion checks from "mouth" attachment but that is at its *current*
	//    position not ownerPos 
	if ( ownerPos == GetAbsOrigin() )
	{
		// If we can see the enemy, or we've seen them in the last few seconds just try to lob in there
		if ( SeenEnemyWithinTime( 3.0f ) )
		{
			Vector vSpitPos;
			GetAttachment( "mouth", vSpitPos );
			
			return GetSpitVector( vSpitPos, targetPos, &m_vecSaveSpitVelocity );
		}
	}
	else
	{
			Vector vSpitPos;
			GetAttachment( "mouth", vSpitPos );
			vSpitPos -= GetAbsOrigin();
			
			// Don't bother saving m_vecSaveSpitVelocity cuz we aren't standing here
			return GetSpitVector( ownerPos + vSpitPos, targetPos, &vSpitPos );
	}
	return BaseClass::InnateWeaponLOSCondition( ownerPos, targetPos, bSetConditions );
#else
	if ( GetNextAttack() > gpGlobals->curtime )
		return false;

	// If we can see the enemy, or we've seen them in the last few seconds just try to lob in there
	if ( SeenEnemyWithinTime( 3.0f ) )
	{
		Vector vSpitPos;
		GetAttachment( "mouth", vSpitPos );
		
		return GetSpitVector( vSpitPos, targetPos, &m_vecSaveSpitVelocity );
	}

	return BaseClass::InnateWeaponLOSCondition( ownerPos, targetPos, bSetConditions );
#endif
}

// Copied from npc_antlion.cpp but allow shots through CONTENTS_GRATE
extern ConVar g_debug_antlion_worker;
static Vector VecCheckThrowTolerance( CBaseEntity *pEdict, const Vector &vecSpot1, Vector vecSpot2, float flSpeed, float flTolerance )
{
	flSpeed = max( 1.0f, flSpeed );

	float flGravity = sv_gravity.GetFloat();

	Vector vecGrenadeVel = (vecSpot2 - vecSpot1);

	// throw at a constant time
	float time = vecGrenadeVel.Length( ) / flSpeed;
	vecGrenadeVel = vecGrenadeVel * (1.0 / time);

	// adjust upward toss to compensate for gravity loss
	vecGrenadeVel.z += flGravity * time * 0.5;

	Vector vecApex = vecSpot1 + (vecSpot2 - vecSpot1) * 0.5;
	vecApex.z += 0.5 * flGravity * (time * 0.5) * (time * 0.5);


	trace_t tr;
	UTIL_TraceLine( vecSpot1, vecApex, MASK_SOLID & ~CONTENTS_GRATE, pEdict, COLLISION_GROUP_NONE, &tr );
	if (tr.fraction != 1.0)
	{
		// fail!
		if ( g_debug_antlion_worker.GetBool() )
		{
			NDebugOverlay::Line( vecSpot1, vecApex, 255, 0, 0, true, 5.0 );
		}

		return vec3_origin;
	}

	if ( g_debug_antlion_worker.GetBool() )
	{
		NDebugOverlay::Line( vecSpot1, vecApex, 0, 255, 0, true, 5.0 );
	}

	UTIL_TraceLine( vecApex, vecSpot2, MASK_SOLID_BRUSHONLY & ~CONTENTS_GRATE, pEdict, COLLISION_GROUP_NONE, &tr );
	if ( tr.fraction != 1.0 )
	{
		bool bFail = true;

		// Didn't make it all the way there, but check if we're within our tolerance range
		if ( flTolerance > 0.0f )
		{
			float flNearness = ( tr.endpos - vecSpot2 ).LengthSqr();
			if ( flNearness < Square( flTolerance ) )
			{
				if ( g_debug_antlion_worker.GetBool() )
				{
					NDebugOverlay::Sphere( tr.endpos, vec3_angle, flTolerance, 0, 255, 0, 0, true, 5.0 );
				}

				bFail = false;
			}
		}
		
		if ( bFail )
		{
			if ( g_debug_antlion_worker.GetBool() )
			{
				NDebugOverlay::Line( vecApex, vecSpot2, 255, 0, 0, true, 5.0 );
				NDebugOverlay::Sphere( tr.endpos, vec3_angle, flTolerance, 255, 0, 0, 0, true, 5.0 );
			}
			return vec3_origin;
		}
	}

	if ( g_debug_antlion_worker.GetBool() )
	{
		NDebugOverlay::Line( vecApex, vecSpot2, 0, 255, 0, true, 5.0 );
	}

	return vecGrenadeVel;
}

//-----------------------------------------------------------------------------
// Purpose: Get a toss direction that will properly lob spit to hit a target
// Input  : &vecStartPos - Where the spit will start from
//			&vecTarget - Where the spit is meant to land
//			*vecOut - The resulting vector to lob the spit
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CNPC_Bullsquid::GetSpitVector( const Vector &vecStartPos, const Vector &vecTarget, Vector *vecOut )
{
	// antlion workers exist only in episodic.
#if HL2_EPISODIC
	// Try the most direct route
	Vector vecToss = VecCheckThrowTolerance( this, vecStartPos, vecTarget, sk_bullsquid_spit_speed.GetFloat(), (10.0f*12.0f) );

	// If this failed then try a little faster (flattens the arc)
	if ( vecToss == vec3_origin )
	{
		vecToss = VecCheckThrowTolerance( this, vecStartPos, vecTarget, sk_bullsquid_spit_speed.GetFloat() * 1.5f, (10.0f*12.0f) );
		if ( vecToss == vec3_origin )
			return false;
	}


	// Save out the result
	if ( vecOut )
	{
		*vecOut = vecToss;
	}

	return true;
#else
	return false;
#endif
}

//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CNPC_Bullsquid::HandleAnimEvent( animevent_t *pEvent )
{
	switch( pEvent->event )
	{
		case BSQUID_AE_SPIT:
		{
			if ( GetEnemy() )
			{
#ifdef HOE_DLL // From acid antlion
				Vector vSpitPos;
				Vector forward, right, up;
				AngleVectors ( GetLocalAngles(), &forward, &right, &up );
				vSpitPos = GetLocalOrigin() + ( right * 6 + forward * 37 + up * 24 );		

				Vector vTarget;
				float flDist = UTIL_DistApprox( GetAbsOrigin(), GetEnemy()->GetAbsOrigin() );
				
				// If our enemy is looking at us and far enough away, lead him
				if ( HasCondition( COND_ENEMY_FACING_ME ) && flDist > (40*12) )
				{
					UTIL_PredictedPosition( GetEnemy(), 0.5f, &vTarget ); 
					vTarget.z = GetEnemy()->GetAbsOrigin().z;
vTarget.z += random->RandomFloat( 0.0f, 32.0f );
				}
				else
				{
					// Otherwise he can't see us and he won't be able to dodge
					vTarget = GetEnemy()->BodyTarget( vSpitPos, true );
				}
				
//				vTarget[2] += random->RandomFloat( 0.0f, 32.0f );
				
				// Try and spit at our target
				Vector	vecToss;				
				if ( GetSpitVector( vSpitPos, vTarget, &vecToss ) == false )
				{
					// Now try where they were
					if ( GetSpitVector( vSpitPos, m_vSavePosition, &vecToss ) == false )
					{
						// Failing that, just shoot with the old velocity we calculated initially!
						vecToss = m_vecSaveSpitVelocity;
					}
				}

				// Find what our vertical theta is to estimate the time we'll impact the ground
				Vector vecToTarget = ( vTarget - vSpitPos );
				VectorNormalize( vecToTarget );
				float flVelocity = VectorNormalize( vecToss );
				float flCosTheta = DotProduct( vecToTarget, vecToss );
				float flTime = (vSpitPos-vTarget).Length2D() / ( flVelocity * flCosTheta );

				// Emit a sound where this is going to hit so that targets get a chance to act correctly
				CSoundEnt::InsertSound( SOUND_DANGER, vTarget, (15*12), flTime, this );

				// Don't fire again until this volley would have hit the ground (with some lag behind it)
				SetNextAttack( gpGlobals->curtime + flTime + random->RandomFloat( 0.5f, 2.0f ) );
#if 0
				// Tell any squadmates not to fire for some portion of the time this volley will be in the air (except on hard)
				if ( g_pGameRules->IsSkillLevel( SKILL_HARD ) == false )
					DelaySquadAttack( flTime );
#endif
				for ( int i = 0; i < 6; i++ )
				{
					CGrenadeSpitBullsquid *pGrenade = (CGrenadeSpitBullsquid*) CreateEntityByName( "grenade_spit_bullsquid" );
					pGrenade->SetAbsOrigin( vSpitPos );
					pGrenade->SetAbsAngles( vec3_angle );
					DispatchSpawn( pGrenade );
					pGrenade->SetThrower( this );
					pGrenade->SetOwnerEntity( this );
										
					if ( i == 0 )
					{
						pGrenade->SetSpitSize( SPIT_LARGE );
						pGrenade->SetAbsVelocity( vecToss * flVelocity );
					}
					else
					{
						pGrenade->SetAbsVelocity( ( vecToss + RandomVector( -0.035f, 0.035f ) ) * flVelocity );
						pGrenade->SetSpitSize( random->RandomInt( SPIT_SMALL, SPIT_MEDIUM ) );
					}

					// Tumble through the air
					pGrenade->SetLocalAngularVelocity(
						QAngle( random->RandomFloat( -250, -500 ),
								random->RandomFloat( -250, -500 ),
								random->RandomFloat( -250, -500 ) ) );
				}

				for ( int i = 0; i < 8; i++ )
				{
					DispatchParticleEffect( "blood_impact_yellow_01", vSpitPos + RandomVector( -12.0f, 12.0f ), RandomAngle( 0, 360 ) );
				}

				AttackSound();
#else
				Vector vSpitPos;

				GetAttachment( "Mouth", vSpitPos );
				
				Vector			vTarget = GetEnemy()->GetAbsOrigin();
				Vector			vToss;
				CBaseEntity*	pBlocker;
				float flGravity  = SPIT_GRAVITY;
				ThrowLimit(vSpitPos, vTarget, flGravity, 3, Vector(0,0,0), Vector(0,0,0), GetEnemy(), &vToss, &pBlocker);

				CGrenadeSpit *pGrenade = (CGrenadeSpit*)CreateNoSpawn( "grenade_spit", vSpitPos, vec3_angle, this );
				//pGrenade->KeyValue( "velocity", vToss );
				pGrenade->Spawn( );
				pGrenade->SetThrower( this );
				pGrenade->SetOwnerEntity( this );
				pGrenade->SetSpitSize( 2 );
				pGrenade->SetAbsVelocity( vToss );

				// Tumble through the air
				pGrenade->SetLocalAngularVelocity(
					QAngle(
						random->RandomFloat( -100, -500 ),
						random->RandomFloat( -100, -500 ),
						random->RandomFloat( -100, -500 )
					)
				);
						
				AttackSound();
			
				CPVSFilter filter( vSpitPos );
				te->SpriteSpray( filter, 0.0,
					&vSpitPos, &vToss, m_nSquidSpitSprite, 5, 10, 15 );
#endif // HOE_DLL
			}
		}
		break;

		case BSQUID_AE_BITE:
		{
		// SOUND HERE!
#ifdef HOE_DLL
			int iDamage = sk_bullsquid_dmg_bite.GetFloat();
			// Reduce damage against humans to avoid mass slaughter
			extern bool HOE_IsHuman( CBaseEntity *pEnt );
			if ( GetEnemy() && HOE_IsHuman( GetEnemy() ) )
				iDamage /= 2;
			CBaseEntity *pHurt = CheckTraceHullAttack( 70, Vector(-16,-16,-16), Vector(16,16,16), iDamage, DMG_SLASH );
#else
			CBaseEntity *pHurt = CheckTraceHullAttack( 70, Vector(-16,-16,-16), Vector(16,16,16), sk_bullsquid_dmg_bite.GetFloat(), DMG_SLASH );
#endif
			if ( pHurt )
			{
				Vector forward, up;
				AngleVectors( GetAbsAngles(), &forward, NULL, &up );
				pHurt->ApplyAbsVelocityImpulse( 100 * (up-forward) );
				pHurt->SetGroundEntity( NULL );
			}
		}
		break;

		case BSQUID_AE_WHIP_SND:
		{
			EmitSound( "NPC_Bullsquid.TailWhip" ); // HOE: never reached
			break;
		}

#ifdef HOE_DLL
		case BSQUID_AE_ROAR:
#else
		case BSQUID_AE_TAILWHIP:
#endif
			{
#ifdef HOE_DLL
			int iDamage = GetTailWhipDamage( GetEnemy() );
			// Tail-whip is from right to left. Trace the hull that way so the force is applied
			// properly to the enemy/ragdoll.
			Vector forward, right, up;
			AngleVectors( GetAbsAngles(), &forward, &right, &up );
			CBaseEntity *pHurt = CheckTraceHullAttack( 70, Vector(-16,-16,-16), Vector(16,16,16), iDamage, DMG_SLASH, 2.0f, false, -right );
			if ( pHurt ) 
			{
				Vector right, up;
				AngleVectors( GetAbsAngles(), NULL, &right, &up );

				if ( pHurt->GetFlags() & ( FL_NPC | FL_CLIENT ) )
					 pHurt->ViewPunch( QAngle( 20, 0, 20 ) ); // roll to right, not left
			
				pHurt->ApplyAbsVelocityImpulse( 100 * (up-2*right) ); // should've been left, not right!
				EmitSound( "NPC_Bullsquid.TailHit" );
			}
			else
			{
				EmitSound( "NPC_Bullsquid.TailWhip" );
			}
#else
			CBaseEntity *pHurt = CheckTraceHullAttack( 70, Vector(-16,-16,-16), Vector(16,16,16), sk_bullsquid_dmg_whip.GetFloat(), DMG_SLASH | DMG_ALWAYSGIB );
			if ( pHurt ) 
			{
				Vector right, up;
				AngleVectors( GetAbsAngles(), NULL, &right, &up );

				if ( pHurt->GetFlags() & ( FL_NPC | FL_CLIENT ) )
					 pHurt->ViewPunch( QAngle( 20, 0, -20 ) );
			
				pHurt->ApplyAbsVelocityImpulse( 100 * (up+2*right) );
			}
#endif // HOE_DLL
		}
		break;

		case BSQUID_AE_BLINK:
		{
			// close eye. 
			m_nSkin = 1; // HOE: never reached
		}
		break;

		case BSQUID_AE_HOP:
		{
			float flGravity = GetCurrentGravity();

			// throw the squid up into the air on this frame.
			if ( GetFlags() & FL_ONGROUND )
			{
				SetGroundEntity( NULL );
			}

			// jump 40 inches into the air
			Vector vecVel = GetAbsVelocity();
			vecVel.z += sqrt( flGravity * 2.0 * 40 );
			SetAbsVelocity( vecVel );
		}
		break;

		case BSQUID_AE_THROW:
			{
				// squid throws its prey IF the prey is a client. 
				CBaseEntity *pHurt = CheckTraceHullAttack( 70, Vector(-16,-16,-16), Vector(16,16,16), 0, 0 );


				if ( pHurt )
				{
					pHurt->ViewPunch( QAngle(20,0,-20) );
							
					// screeshake transforms the viewmodel as well as the viewangle. No problems with seeing the ends of the viewmodels.
					UTIL_ScreenShake( pHurt->GetAbsOrigin(), 25.0, 1.5, 0.7, 2, SHAKE_START );

					// If the player, throw him around
					if ( pHurt->IsPlayer())
					{
						Vector forward, up;
						AngleVectors( GetLocalAngles(), &forward, NULL, &up );
						pHurt->ApplyAbsVelocityImpulse( forward * 300 + up * 300 );

						// Looks bad if we spit right after throwing the player (almost always misses in mid-air).
						SetNextAttack( gpGlobals->curtime + random->RandomFloat( 1.5f, 1.75f ) );
					}
					// If not the player see if has bullsquid throw interatcion
					else
					{
						CBaseCombatCharacter *pVictim = ToBaseCombatCharacter( pHurt );
						if (pVictim)
						{
							if ( pVictim->DispatchInteraction( g_interactionBullsquidThrow, NULL, this ) )
							{
								Vector forward, up;
								AngleVectors( GetLocalAngles(), &forward, NULL, &up );
								pVictim->ApplyAbsVelocityImpulse( forward * 300 + up * 250 );
							}
						}
					}
				}
			}
		break;

		default:
			BaseClass::HandleAnimEvent( pEvent );
	}
}

int CNPC_Bullsquid::RangeAttack1Conditions( float flDot, float flDist )
{
	if ( IsMoving() && flDist >= 512 )
	{
		// squid will far too far behind if he stops running to spit at this distance from the enemy.
		return ( COND_NONE );
	}

#ifdef HOE_DLL
	// Don't spit at chumtoads.  This gives them a chance to play dead.
	if ( GetEnemy() != NULL && GetEnemy()->Classify() == CLASS_CHUMTOAD )
		return COND_NONE;

	if ( GetEnemy() != NULL )
	{
		// Don't spit up/down at too sharp an angle
		Vector vHeadDir = HeadDirection3D();
		Vector vecToEnemy = GetEnemy()->GetAbsOrigin() - GetAbsOrigin();
		VectorNormalize( vecToEnemy );
		float flDotZ = DotProduct( vecToEnemy, vHeadDir );
//		DevMsg( "bullsquid dot %.2f\n", flDotZ );
		if ( flDotZ < 0.6 )
			return COND_TOO_CLOSE_TO_ATTACK;
	}

	m_flNextSpitTime = GetNextAttack(); // calculated in HandleAnimEvent
#endif

	if ( flDist > InnateRange1MinRange() && flDist <= InnateRange1MaxRange() && flDot >= 0.5 && gpGlobals->curtime >= m_flNextSpitTime )
	{
		if ( GetEnemy() != NULL )
		{
			if ( fabs( GetAbsOrigin().z - GetEnemy()->GetAbsOrigin().z ) > 256 )
			{
				// don't try to spit at someone up really high or down really low.
				return( COND_NONE );
			}
		}

		if ( IsMoving() )
		{
			// don't spit again for a long time, resume chasing enemy.
			m_flNextSpitTime = gpGlobals->curtime + 5;
		}
		else
		{
			// not moving, so spit again pretty soon.
			m_flNextSpitTime = gpGlobals->curtime + 3.0;
		}

		return( COND_CAN_RANGE_ATTACK1 );
	}

	return( COND_NONE );
}

#ifdef HOE_DLL
// BUG: 2 bullsquids meeting corner to corner will appear to freeze up as they run
// to melee attack each other if the melee range is less than the distance from hull
// center to hull corner times 2;
static float MinMeleeRange( CBaseCombatCharacter *pBCC, CBaseCombatCharacter *pEnemy )
{
	if ( pEnemy == NULL )
		return 0;
	float flBoxWidth = NAI_Hull::Width( pBCC->GetHullType() );
	float flEnemyBoxWidth = NAI_Hull::Width( pEnemy->GetHullType() );
	float flBoxDiagonal = sqrt( Square(flBoxWidth/2) + Square(flBoxWidth/2) );
	float flEnemyBoxDiagonal = sqrt( Square(flEnemyBoxWidth/2) + Square(flEnemyBoxWidth/2) );
//	DevMsg( "MinMeleeRange %f\n", flBoxDiagonal + flEnemyBoxDiagonal + 1.0f );
	return flBoxDiagonal + flEnemyBoxDiagonal + 1;
}

int CNPC_Bullsquid::GetTailWhipDamage( CBaseEntity *pVictim, int *adjustedDmg )
{
	int iDamage = sk_bullsquid_dmg_whip.GetFloat();

	if ( adjustedDmg != NULL )
		*adjustedDmg = iDamage;

	if ( pVictim == NULL )
		return iDamage;

	// Reduce damage against humans to avoid mass slaughter
	extern bool HOE_IsHuman( CBaseEntity *pEnt );
	if ( HOE_IsHuman( pVictim ) )
		iDamage /= 2;

	if ( adjustedDmg != NULL )
	{
		// Hack - The actual damage taken by the player changes based on difficulty level.
		// See CTakeDamageInfo::AdjustPlayerDamageTakenForSkillLevel() etc.
		// We need to know the actual damage to determine if the Bullsquid should do his
		// finishing move tail-spin attack.
		// Might be nice to have a DMG_NO_DIFFICULTY_SCALE flag.
		extern ConVar sk_dmg_take_scale1, sk_dmg_take_scale2, sk_dmg_take_scale3;
		if ( pVictim->IsPlayer() )
		{
			switch( g_pGameRules->GetSkillLevel() )
			{
			case SKILL_EASY:
				iDamage *= sk_dmg_take_scale1.GetFloat();
				break;

			case SKILL_MEDIUM:
				iDamage *= sk_dmg_take_scale2.GetFloat();
				break;

			case SKILL_HARD:
				iDamage *= sk_dmg_take_scale3.GetFloat();
				break;
			}
		}
		*adjustedDmg = iDamage;
	}

	return iDamage;
}
#endif // HOE_DLL

//=========================================================
// MeleeAttack2Conditions - bullsquid is a big guy, so has a longer
// melee range than most monsters. This is the tailwhip attack
//=========================================================
int CNPC_Bullsquid::MeleeAttack1Conditions( float flDot, float flDist )
{
#ifdef HOE_DLL
	int iAdjustedDamage;
	GetTailWhipDamage( GetEnemy(), &iAdjustedDamage );
	float flMinMeleeRange = MinMeleeRange( this, GetEnemy()->MyCombatCharacterPointer() );
	float flMeleeRange = max( flMinMeleeRange, 85 );
	if ( GetEnemy()->m_iHealth <= iAdjustedDamage && flDist <= flMeleeRange && flDot >= 0.7 )
#else
	if ( GetEnemy()->m_iHealth <= sk_bullsquid_dmg_whip.GetFloat() && flDist <= 85 && flDot >= 0.7 )
#endif
	{
		return ( COND_CAN_MELEE_ATTACK1 );
	}
	
	return( COND_NONE );
}

//=========================================================
// MeleeAttack2Conditions - bullsquid is a big guy, so has a longer
// melee range than most monsters. This is the bite attack.
// this attack will not be performed if the tailwhip attack
// is valid.
//=========================================================
int CNPC_Bullsquid::MeleeAttack2Conditions( float flDot, float flDist )
{
#ifdef HOE_DLL
	float flMinMeleeRange = MinMeleeRange( this, GetEnemy()->MyCombatCharacterPointer() );
	float flMeleeRange = max( flMinMeleeRange, 85 );
	if ( flDist <= flMeleeRange && flDot >= 0.7 && !HasCondition( COND_CAN_MELEE_ATTACK1 ) )
#else
	if ( flDist <= 85 && flDot >= 0.7 && !HasCondition( COND_CAN_MELEE_ATTACK1 ) )		// The player & bullsquid can be as much as their bboxes 
#endif
		return ( COND_CAN_MELEE_ATTACK2 );
	
	return( COND_NONE );
}

bool CNPC_Bullsquid::FValidateHintType( CAI_Hint *pHint )
{
	if ( pHint->HintType() == HINT_HL1_WORLD_HUMAN_BLOOD )
		 return true;

	DevMsg( "Couldn't validate hint type" );

	return false;
}

void CNPC_Bullsquid::RemoveIgnoredConditions( void )
{
#ifdef HOE_DLL
	BaseClass::RemoveIgnoredConditions();
#endif
	if ( m_flHungryTime > gpGlobals->curtime )
	{
		ClearCondition( COND_SMELL );
		ClearCondition( COND_SQUID_SMELL_FOOD );
	}

	if ( gpGlobals->curtime - m_flLastHurtTime <= 20 )
	{
		// haven't been hurt in 20 seconds, so let the squid care about stink. 
		ClearCondition( COND_SMELL );
		ClearCondition( COND_SQUID_SMELL_FOOD );
	}

	if ( GetEnemy() != NULL )
	{
#ifndef HOE_DLL
		// ( Unless after a tasty headcrab, yumm ^_^ )
		if ( FClassnameIs( GetEnemy(), "npc_headcrab" ) )
#endif
		{
			ClearCondition( COND_SMELL );
			ClearCondition( COND_SQUID_SMELL_FOOD );
		}
	}
}

Disposition_t CNPC_Bullsquid::IRelationType( CBaseEntity *pTarget )
{
	if ( gpGlobals->curtime - m_flLastHurtTime < 5 && FClassnameIs( pTarget, "npc_headcrab" ) )
	{
		// if squid has been hurt in the last 5 seconds, and is getting relationship for a headcrab, 
		// tell squid to disregard crab. 
		return D_NU;
	}

	return BaseClass::IRelationType( pTarget );
}

#ifdef HOE_DLL
//-----------------------------------------------------------------------------
int CNPC_Bullsquid::IRelationPriority( CBaseEntity *pTarget )
{
	int priority = BaseClass::IRelationPriority( pTarget );

	// An enemy I was recently unable to find a weapon los node for
	// has lower priority than an enemy I can shoot.
	if ( pTarget && IsUnshootable( pTarget ) )
		priority -= 1; // FIXME: is negative priority allowed?
	return priority;
}
#endif

//=========================================================
// TakeDamage - overridden for bullsquid so we can keep track
// of how much time has passed since it was last injured
//=========================================================
int CNPC_Bullsquid::OnTakeDamage_Alive( const CTakeDamageInfo &inputInfo )
{

#if 0 //Fix later.

	float flDist;
	Vector vecApex, vOffset;

	// if the squid is running, has an enemy, was hurt by the enemy, hasn't been hurt in the last 3 seconds, and isn't too close to the enemy,
	// it will swerve. (whew).
	if ( GetEnemy() != NULL && IsMoving() && pevAttacker == GetEnemy() && gpGlobals->curtime - m_flLastHurtTime > 3 )
	{
		flDist = ( GetAbsOrigin() - GetEnemy()->GetAbsOrigin() ).Length2D();
		
		if ( flDist > SQUID_SPRINT_DIST )
		{
			AI_Waypoint_t*	pRoute = GetNavigator()->GetPath()->Route();

			if ( pRoute )
			{
				flDist = ( GetAbsOrigin() - pRoute[ pRoute->iNodeID ].vecLocation ).Length2D();// reusing flDist. 

				if ( GetNavigator()->GetPath()->BuildTriangulationRoute( GetAbsOrigin(), pRoute[ pRoute->iNodeID ].vecLocation, flDist * 0.5, GetEnemy(), &vecApex, &vOffset, NAV_GROUND ) )
				{
					GetNavigator()->PrependWaypoint( vecApex, bits_WP_TO_DETOUR | bits_WP_DONT_SIMPLIFY );
				}
			}
		}
	}
#endif

	if ( !FClassnameIs( inputInfo.GetAttacker(), "npc_headcrab" ) )
	{
		// don't forget about headcrabs if it was a headcrab that hurt the squid.
		m_flLastHurtTime = gpGlobals->curtime;
	}

#ifdef HOE_DLL
	// Disallow damage from server-side ragdolls.
	// I see these guys attacking a human whose ragdoll then kills them.
	if ( inputInfo.GetAttacker() &&
		FClassnameIs( inputInfo.GetAttacker(), "prop_ragdoll" ) )
	{
		return 0;
	}
#endif // HOE_DLL

	return BaseClass::OnTakeDamage_Alive( inputInfo );
}

#ifdef HOE_DLL
//------------------------------------------------------------------------------
void CNPC_Bullsquid::Event_Killed( const CTakeDamageInfo &info )
{
	// Shut the eye
	m_nSkin = 1;

	BaseClass::Event_Killed( info );
}
#endif // HOE_DLL

//=========================================================
// GetSoundInterests - returns a bit mask indicating which types
// of sounds this monster regards. In the base class implementation,
// monsters care about all sounds, but no scents.
//=========================================================
int CNPC_Bullsquid::GetSoundInterests( void )
{
	return	SOUND_WORLD	|
			SOUND_COMBAT	|
		    SOUND_CARCASS	|
			SOUND_MEAT		|
			SOUND_GARBAGE	|
			SOUND_PLAYER;
}

//=========================================================
// OnListened - monsters dig through the active sound list for
// any sounds that may interest them. (smells, too!)
//=========================================================
void CNPC_Bullsquid::OnListened( void )
{
	AISoundIter_t iter;
	
	CSound *pCurrentSound;

	static int conditionsToClear[] = 
	{
		COND_SQUID_SMELL_FOOD,
	};

	ClearConditions( conditionsToClear, ARRAYSIZE( conditionsToClear ) );
	
	pCurrentSound = GetSenses()->GetFirstHeardSound( &iter );
	
	while ( pCurrentSound )
	{
		// the npc cares about this sound, and it's close enough to hear.
		int condition = COND_NONE;
		
		if ( !pCurrentSound->FIsSound() )
		{
			// if not a sound, must be a smell - determine if it's just a scent, or if it's a food scent
			if ( pCurrentSound->m_iType & ( SOUND_MEAT | SOUND_CARCASS ) )
			{
				// the detected scent is a food item
				condition = COND_SQUID_SMELL_FOOD;
			}
		}
		
		if ( condition != COND_NONE )
			SetCondition( condition );

		pCurrentSound = GetSenses()->GetNextHeardSound( &iter );
	}

	BaseClass::OnListened();
}

//========================================================
// RunAI - overridden for bullsquid because there are things
// that need to be checked every think.
//========================================================
void CNPC_Bullsquid::RunAI( void )
{
	// first, do base class stuff
	BaseClass::RunAI();

	if ( m_nSkin != 0 )
	{
		// close eye if it was open.
		m_nSkin = 0; 
	}

	if ( random->RandomInt( 0,39 ) == 0 )
	{
		m_nSkin = 1;
	}

	if ( GetEnemy() != NULL && GetActivity() == ACT_RUN )
	{
		// chasing enemy. Sprint for last bit
		if ( (GetAbsOrigin() - GetEnemy()->GetAbsOrigin()).Length2D() < SQUID_SPRINT_DIST )
		{
			m_flPlaybackRate = 1.25;
		}
	}

}

//-----------------------------------------------------------------------------
void CNPC_Bullsquid::BuildScheduleTestBits( void )
{
	// Ignore damage if we're attacking, otherwise the animation stops in progress.
	if ( GetActivity() == ACT_RANGE_ATTACK1 ||
		 GetActivity() == ACT_MELEE_ATTACK1 ||
		 GetActivity() == ACT_MELEE_ATTACK2 )
	{
		ClearCustomInterruptCondition( COND_LIGHT_DAMAGE );
		ClearCustomInterruptCondition( COND_HEAVY_DAMAGE );
	}

	BaseClass::BuildScheduleTestBits();
}

#ifdef HOE_DLL
//-----------------------------------------------------------------------------
void CNPC_Bullsquid::GatherConditions( void )
{
	// If chumtoad starts playing dead forget it as an enemy
	if ( GetEnemy() && FClassnameIs( GetEnemy(), "npc_chumtoad" ) &&
		GetEnemy()->Classify() != CLASS_CHUMTOAD )
	{
		GetEnemies()->ClearMemory( GetEnemy() );
		SetEnemy( NULL );
		SetCondition( COND_ENEMY_DEAD );
  		ClearCondition( COND_SEE_ENEMY );
  		ClearCondition( COND_ENEMY_OCCLUDED );
	}

	BaseClass::GatherConditions();
}

//------------------------------------------------------------------------------
void CNPC_Bullsquid::RememberUnshootable(CBaseEntity *pEntity, float duration )
{
	if ( pEntity == GetEnemy() )
	{
		ForceChooseNewEnemy();
	}

	const float NPC_UNREACHABLE_TIMEOUT = ( duration > 0.0 ) ? duration : 3;
	// Only add to list if not already on it
	for (int i=m_UnshootableEnts.Size()-1;i>=0;i--)
	{
		// If record already exists just update mark time
		if (pEntity == m_UnshootableEnts[i].hUnshootableEnt)
		{
			m_UnshootableEnts[i].fExpireTime	 = gpGlobals->curtime + NPC_UNREACHABLE_TIMEOUT;
			m_UnshootableEnts[i].vLocationWhenUnshootable = pEntity->GetAbsOrigin();
			return;
		}
	}

	// Add new unshootable entity to list
	int nNewIndex = m_UnshootableEnts.AddToTail();
	m_UnshootableEnts[nNewIndex].hUnshootableEnt = pEntity;
	m_UnshootableEnts[nNewIndex].fExpireTime	 = gpGlobals->curtime + NPC_UNREACHABLE_TIMEOUT;
	m_UnshootableEnts[nNewIndex].vLocationWhenUnshootable = pEntity->GetAbsOrigin();
}

//------------------------------------------------------------------------------
bool CNPC_Bullsquid::IsUnshootable(CBaseEntity *pEntity)
{
	float UNREACHABLE_DIST_TOLERANCE_SQ = (120*120);

	// Note that it's ok to remove elements while I'm iterating
	// as long as I iterate backwards and remove them using FastRemove
	for (int i=m_UnshootableEnts.Size()-1;i>=0;i--)
	{
		// Remove any dead elements
		if (m_UnshootableEnts[i].hUnshootableEnt == NULL)
		{
			m_UnshootableEnts.FastRemove(i);
		}
		else if (pEntity == m_UnshootableEnts[i].hUnshootableEnt)
		{
			// Test for shootablility on any elements that have timed out
			if ( gpGlobals->curtime > m_UnshootableEnts[i].fExpireTime ||
				  pEntity->GetAbsOrigin().DistToSqr(m_UnshootableEnts[i].vLocationWhenUnshootable) > UNREACHABLE_DIST_TOLERANCE_SQ)
			{
				m_UnshootableEnts.FastRemove(i);
				return false;
			}
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
bool CNPC_Bullsquid::ExpensiveUnreachableCheck( CBaseEntity *pEntity )
{
	AI_Waypoint_t *waypoints = NULL;
	waypoints = GetPathfinder()->BuildRoute(
		GetAbsOrigin(),
		pEntity->GetAbsOrigin(), pEntity,
		GetDefaultNavGoalTolerance(), GetNavType(), true );
	if ( !waypoints )
	{
//		DevMsg( "bullsquid expensive unreachable check: %s UNREACHABLE\n", pEntity->GetDebugName() );
		return true; // unreachable
	}
	GetPathfinder()->UnlockRouteNodes( waypoints );
	DeleteAll( waypoints );
//	DevMsg( "bullsquid expensive unreachable check: %s REACHABLE\n", pEntity->GetDebugName() );
	return false; // !unreachable
}

//-----------------------------------------------------------------------------
bool CNPC_Bullsquid::IsUnreachable( CBaseEntity *pEntity )
{
#if 1
	bool bResult = false;

	float UNREACHABLE_DIST_TOLERANCE_SQ = (120*120);
	const float NPC_UNREACHABLE_TIMEOUT = 3;

	// Note that it's ok to remove elements while I'm iterating
	// as long as I iterate backwards and remove them using FastRemove
	for (int i=m_UnreachableEnts.Size()-1;i>=0;i--)
	{
		// Remove any dead elements
		if (m_UnreachableEnts[i].hUnreachableEnt == NULL)
		{
			m_UnreachableEnts.FastRemove(i);
		}
		else if (pEntity == m_UnreachableEnts[i].hUnreachableEnt)
		{
			// Test for reachablility on any elements that have timed out
			if ( gpGlobals->curtime > m_UnreachableEnts[i].fExpireTime ||
				  pEntity->GetAbsOrigin().DistToSqr(m_UnreachableEnts[i].vLocationWhenUnreachable) > UNREACHABLE_DIST_TOLERANCE_SQ)
			{
				// If an enemy was recently unreachable it may still be, so I don't want to
				// forget it was unreachable automatically after 3 seconds otherwise I may
				// bounce between enemies if I'm stuck behind a fence.
				if ( ExpensiveUnreachableCheck( pEntity ) )
				{
					m_UnreachableEnts[i].fExpireTime = gpGlobals->curtime + NPC_UNREACHABLE_TIMEOUT;
					m_UnreachableEnts[i].vLocationWhenUnreachable = pEntity->GetAbsOrigin();
					bResult = true;
				}
				else
					m_UnreachableEnts.FastRemove(i);
				break;
			}
			bResult = true;
			break;
		}
	}
#else
	bool bResult = BaseClass::IsUnreachable( pEntity );
#endif
	// Detect rapelling/hanging enemy
	if ( pEntity && pEntity->IsNPC() && !bResult )
	{
		CAI_BaseNPC *pNPC = pEntity->MyNPCPointer();
		trace_t tr;
		AI_TraceHull( pNPC->GetAbsOrigin(),
			pNPC->GetAbsOrigin() - Vector(0,0,64),
			pNPC->GetHullMins(), pNPC->GetHullMaxs(),
			MASK_SOLID_BRUSHONLY, pNPC, COLLISION_GROUP_NONE, &tr );
		if ( !tr.startsolid && tr.fraction == 1.0 )
		{
//			RememberUnreachable( pNPC ); //

			// Add new unreachabe entity to list
			int nNewIndex = m_UnreachableEnts.AddToTail();
			m_UnreachableEnts[nNewIndex].hUnreachableEnt = pEntity;
			m_UnreachableEnts[nNewIndex].fExpireTime	 = gpGlobals->curtime + NPC_UNREACHABLE_TIMEOUT;
			m_UnreachableEnts[nNewIndex].vLocationWhenUnreachable = pEntity->GetAbsOrigin();

			DevMsg("bullsquid added %s to unreachables\n", pEntity->GetDebugName() );
			bResult = true;
		}
	}
	return bResult;
}

//-----------------------------------------------------------------------------
int CNPC_Bullsquid::TranslateSchedule( int scheduleType )
{
	switch ( scheduleType )
	{
	case SCHED_FAIL_TAKE_COVER:
		if ( SeenEnemyWithinTime( 3.0f ) )
			return SCHED_RUN_RANDOM;
		return SCHED_COMBAT_FACE;
		break;
	default:
		return BaseClass::TranslateSchedule( scheduleType );
	}
}

//-----------------------------------------------------------------------------
int CNPC_Bullsquid::SelectFailSchedule( int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode )
{
#if 1
	// 1) SCHED_CHASE_ENEMY fails due to no route
	// 2) RememberUnreachable is called
	// 3) RunAI calls GatherConditions which gets a new enemy
	// 4) MaintainSchedule calls SelectFailSchedule for the failed SCHED_CHASE_ENEMY which does not apply to the new enemy
	if ( failedSchedule == SCHED_CHASE_ENEMY )
#else
	if ( failedSchedule == SCHED_CHASE_ENEMY && HasCondition( COND_ENEMY_UNREACHABLE ) )
#endif
	{
		if ( HasCondition( COND_NEW_ENEMY ) ) // FIXME: could this apply to the same unreachable enemy?
		{
			return SelectSchedule(); // Get a schedule that applies to the new enemy
		}

		// If we don't have a route to our enemy then try to find a place
		// to shoot from.
		return SCHED_ESTABLISH_LINE_OF_FIRE;
	}

	// 1) SCHED_ESTABLISH_LINE_OF_FIRE fails due to no shoot position
	// 2) RememberUnshootable is called
	// 3) RunAI calls GatherConditions which gets a new enemy
	// 4) MaintainSchedule calls SelectFailSchedule for the failed SCHED_ESTABLISH_LINE_OF_FIRE which does not apply to the new enemy
	if ( failedSchedule == SCHED_ESTABLISH_LINE_OF_FIRE )
	{
		if ( HasCondition( COND_NEW_ENEMY ) ) // FIXME: could this apply to the same unshootable enemy?
		{
			return SelectSchedule(); // Get a schedule that applies to the new enemy
		}

		// SCHED_ESTABLISH_LINE_OF_FIRE_FALLBACK chases the enemy. Don't bother if we know
		// the enemy is unreachable.
		if ( IsUnreachable( GetEnemy() ) )
		{
			return SCHED_TAKE_COVER_FROM_ENEMY;
		}

		return SCHED_ESTABLISH_LINE_OF_FIRE_FALLBACK;
	}

	return BaseClass::SelectFailSchedule( failedSchedule,failedTask, taskFailCode );
}

#endif // HOE_DLL
//=========================================================
// GetSchedule 
//=========================================================
int CNPC_Bullsquid::SelectSchedule( void )
{
#ifdef HOE_DLL
	// When a schedule ends, conditions are set back to what GatherConditions
	// reported before selecting a new schedule.  See CAI_BaseNPC::MaintainSchedule.
	// Anything RemoveIgnoredConditions did is lost.
	if ( m_flHungryTime > gpGlobals->curtime )
	{
		ClearCondition( COND_SMELL );
		ClearCondition( COND_SQUID_SMELL_FOOD );
	}

	if ( gpGlobals->curtime - m_flLastHurtTime <= 20 )
	{
		// haven't been hurt in 20 seconds, so let the squid care about stink. 
		ClearCondition( COND_SMELL );
		ClearCondition( COND_SQUID_SMELL_FOOD );
	}

	if ( GetEnemy() != NULL )
	{
#ifndef HOE_DLL
		// ( Unless after a tasty headcrab, yumm ^_^ )
		if ( FClassnameIs( GetEnemy(), "npc_headcrab" ) )
#endif
		{
			ClearCondition( COND_SMELL );
			ClearCondition( COND_SQUID_SMELL_FOOD );
		}
	}
#endif // HOE_DLL

	switch	( m_NPCState )
	{
	case NPC_STATE_ALERT:
		{
			if ( HasCondition( COND_LIGHT_DAMAGE ) || HasCondition( COND_HEAVY_DAMAGE ) )
			{
				return SCHED_SQUID_HURTHOP;
			}

			if ( HasCondition( COND_SQUID_SMELL_FOOD ) )
			{
				CSound		*pSound;

				pSound = GetBestScent();
				
				if ( pSound && (!FInViewCone( pSound->GetSoundOrigin() ) || !FVisible( pSound->GetSoundOrigin() )) )
				{
					// scent is behind or occluded
					return SCHED_SQUID_SNIFF_AND_EAT;
				}

				// food is right out in the open. Just go get it.
				return SCHED_SQUID_EAT;
			}

			if ( HasCondition( COND_SMELL ) )
			{
				// there's something stinky. 
				CSound		*pSound;

				pSound = GetBestScent();
				if ( pSound )
					return SCHED_SQUID_WALLOW;
			}

			break;
		}
	case NPC_STATE_COMBAT:
		{
// dead enemy
			if ( HasCondition( COND_ENEMY_DEAD ) )
			{
				// call base class, all code to handle dead enemies is centralized there.
				return BaseClass::SelectSchedule();
			}

			if ( HasCondition( COND_NEW_ENEMY ) )
			{
				if ( m_fCanThreatDisplay && IRelationType( GetEnemy() ) == D_HT &&
					(FClassnameIs( GetEnemy(), "npc_spx_baby" ) ||
					FClassnameIs( GetEnemy(), "npc_chumtoad" )))
				{
					// this means squid sees a headcrab!
					m_fCanThreatDisplay = FALSE;// only do the headcrab dance once per lifetime.
					return SCHED_SQUID_SEECRAB;
				}
#ifdef HOE_DLL
				// Most NPCs play SCHED_WAKE_ANGRY twice in a row because it is so short.
				else if ( gpGlobals->curtime - GetEnemies()->FirstTimeSeen(GetEnemy()) < 2.0 &&
					gpGlobals->curtime - m_flWakeAngryTime > 5.0f )
				{
					m_flWakeAngryTime = gpGlobals->curtime;
#else
				else
				{
#endif
					return SCHED_WAKE_ANGRY;
				}
			}

#ifdef HOE_DLL
			if ( !SeenEnemyWithinTime( 10.0f ) )
#endif
			if ( HasCondition( COND_SQUID_SMELL_FOOD ) )
			{
				CSound		*pSound;

				pSound = GetBestScent();
				
				if ( pSound && (!FInViewCone( pSound->GetSoundOrigin() ) || !FVisible( pSound->GetSoundOrigin() )) )
				{
					// scent is behind or occluded
					return SCHED_SQUID_SNIFF_AND_EAT;
				}

				// food is right out in the open. Just go get it.
				return SCHED_SQUID_EAT;
			}

			if ( HasCondition( COND_CAN_RANGE_ATTACK1 ) )
			{
				if ( GetEnemy() )
				{
					m_vSavePosition = GetEnemy()->BodyTarget( GetAbsOrigin() );
				}
				return SCHED_RANGE_ATTACK1;
			}

			if ( HasCondition( COND_CAN_MELEE_ATTACK1 ) )
			{
				return SCHED_MELEE_ATTACK1;
			}

			if ( HasCondition( COND_CAN_MELEE_ATTACK2 ) )
			{
				return SCHED_MELEE_ATTACK2;
			}

#ifdef HOE_DLL
			if ( GetEnemy() )
			{
				// Stuck behind a fence? Find a spot to shoot from rather than
				// just hiding.
				// Enemy is hanging from the wrists? Find a spot to shoot from
				// rather than chasing the enemy and standing uselessly below it.
				if ( IsUnreachable( GetEnemy() ) )
				{
					if ( !HasCondition( COND_WEAPON_HAS_LOS ) )
						return SCHED_ESTABLISH_LINE_OF_FIRE;
					return SCHED_COMBAT_FACE;
				}
			}
#endif // HOE_DLL

			return SCHED_CHASE_ENEMY;

			break;
		}
	}

	return BaseClass::SelectSchedule();
}

//=========================================================
// FInViewCone - returns true is the passed vector is in
// the caller's forward view cone. The dot product is performed
// in 2d, making the view cone infinitely tall. 
//=========================================================
bool CNPC_Bullsquid::FInViewCone( Vector pOrigin )
{
	Vector los = ( pOrigin - GetAbsOrigin() );

	// do this in 2D
	los.z = 0;
	VectorNormalize( los );

	Vector facingDir = EyeDirection2D( );

	float flDot = DotProduct( los, facingDir );

	if ( flDot > m_flFieldOfView )
		return true;

	return false;
}

//=========================================================
// Start task - selects the correct activity and performs
// any necessary calculations to start the next task on the
// schedule.  OVERRIDDEN for bullsquid because it needs to
// know explicitly when the last attempt to chase the enemy
// failed, since that impacts its attack choices.
//=========================================================
void CNPC_Bullsquid::StartTask( const Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_MELEE_ATTACK2:
		{
			if (GetEnemy())
			{
				GrowlSound();

				m_flLastAttackTime = gpGlobals->curtime;

				BaseClass::StartTask( pTask );
			}
			break;
		}
	case TASK_SQUID_HOPTURN:
		{
			SetActivity( ACT_HOP );
			
			if ( GetEnemy() )
			{
				Vector	vecFacing = ( GetEnemy()->GetAbsOrigin() - GetAbsOrigin() );
				VectorNormalize( vecFacing );

				GetMotor()->SetIdealYaw( vecFacing );
			}

			break;
		}
	case TASK_SQUID_EAT:
		{
			m_flHungryTime = gpGlobals->curtime + pTask->flTaskData;
			TaskComplete();
			break;
		}
#ifdef HOE_DLL
	case TASK_PLAY_SEQUENCE:
		BaseClass::StartTask( pTask );
		if ( (IsCurSchedule( SCHED_SQUID_EAT ) ||
			IsCurSchedule( SCHED_SQUID_SNIFF_AND_EAT )) &&
			IsActivityFinished() )
		{
			ResetSequenceInfo(); // hack for repeating ACT_SQUID_EAT
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
	case TASK_GET_PATH_TO_ENEMY_LOS:
		BaseClass::StartTask( pTask );
		if ( IsCurSchedule( SCHED_ESTABLISH_LINE_OF_FIRE) && GetEnemy() && HasCondition( COND_TASK_FAILED ) )
		{
			// We couldn't go anywhere to shoot our best enemy. Remember the enemy is
			// unshootable for a few seconds during which time his relationship priority
			// is lowered.  If another enemy is shootable we want to attack him.
			RememberUnshootable( GetEnemy() );
		}
		break;
#endif // HOE_DLL
	default:
		{
			BaseClass::StartTask( pTask );
			break;
		}
	}
}

//=========================================================
// RunTask
//=========================================================
void CNPC_Bullsquid::RunTask( const Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_SQUID_HOPTURN:
		{
			if ( GetEnemy() )
			{
				Vector	vecFacing = ( GetEnemy()->GetAbsOrigin() - GetAbsOrigin() );
				VectorNormalize( vecFacing );
				GetMotor()->SetIdealYaw( vecFacing );
			}

			if ( IsSequenceFinished() )
			{
				TaskComplete(); 
			}
			break;
		}
#ifdef HOE_DLL
	case TASK_GET_PATH_TO_ENEMY_LOS:
		BaseClass::RunTask( pTask );
		if ( IsCurSchedule( SCHED_ESTABLISH_LINE_OF_FIRE) && GetEnemy() && HasCondition( COND_TASK_FAILED ) )
		{
			// We couldn't go anywhere to shoot our best enemy. Remember the enemy is
			// unshootable for a few seconds during which time his relationship priority
			// is lowered.  If another enemy is shootable we want to attack him.
			RememberUnshootable( GetEnemy() );
		}
		break;
#endif // HOE_DLL
	default:
		{
			BaseClass::RunTask( pTask );
			break;
		}
	}
}

//=========================================================
// GetIdealState - Overridden for Bullsquid to deal with
// the feature that makes it lose interest in headcrabs for 
// a while if something injures it. 
//=========================================================
NPC_STATE CNPC_Bullsquid::SelectIdealState( void )
{
	// If no schedule conditions, the new ideal state is probably the reason we're in here.
	switch ( m_NPCState )
	{
		case NPC_STATE_COMBAT:
		{
			// COMBAT goes to ALERT upon death of enemy
			if ( GetEnemy() != NULL && ( HasCondition( COND_LIGHT_DAMAGE ) || HasCondition( COND_HEAVY_DAMAGE ) ) && FClassnameIs( GetEnemy(), "npc_headcrab" ) )
			{
				// if the squid has a headcrab enemy and something hurts it, it's going to forget about the crab for a while.
				SetEnemy( NULL );
				return NPC_STATE_ALERT;
			}
			break;
		}
	}

	return BaseClass::SelectIdealState();
}


//------------------------------------------------------------------------------
//
// Schedules
//
//------------------------------------------------------------------------------

AI_BEGIN_CUSTOM_NPC( npc_bullsquid, CNPC_Bullsquid )

	DECLARE_TASK( TASK_SQUID_HOPTURN )
	DECLARE_TASK( TASK_SQUID_EAT )

	DECLARE_CONDITION( COND_SQUID_SMELL_FOOD )

	DECLARE_ACTIVITY( ACT_SQUID_EXCITED )
	DECLARE_ACTIVITY( ACT_SQUID_EAT )
	DECLARE_ACTIVITY( ACT_SQUID_DETECT_SCENT )
	DECLARE_ACTIVITY( ACT_SQUID_INSPECT_FLOOR )

	DECLARE_INTERACTION( g_interactionBullsquidThrow )

	//=========================================================
	// > SCHED_SQUID_HURTHOP
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_SQUID_HURTHOP,
	
		"	Tasks"
		"		TASK_STOP_MOVING			0"
		"		TASK_SOUND_WAKE				0"
		"		TASK_SQUID_HOPTURN			0"
		"		TASK_FACE_ENEMY				0"
		"	"
		"	Interrupts"
	)
	
	//=========================================================
	// > SCHED_SQUID_SEECRAB
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_SQUID_SEECRAB,
	
		"	Tasks"
		"		TASK_STOP_MOVING			0"
		"		TASK_SOUND_WAKE				0"
		"		TASK_PLAY_SEQUENCE			ACTIVITY:ACT_SQUID_EXCITED"
		"		TASK_FACE_ENEMY				0"
		"	"
		"	Interrupts"
		"		COND_LIGHT_DAMAGE"
		"		COND_HEAVY_DAMAGE"
	)
	
	//=========================================================
	// > SCHED_SQUID_EAT
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_SQUID_EAT,
	
		"	Tasks"
		"		TASK_STOP_MOVING					0"
		"		TASK_SQUID_EAT						10"
		"		TASK_STORE_LASTPOSITION				0"
		"		TASK_GET_PATH_TO_BESTSCENT			0"
		"		TASK_WALK_PATH_WITHIN_DIST			64"
		"		TASK_STOP_MOVING					0"
		"		TASK_FACE_SAVEPOSITION				0" // HOE_DLL: see TASK_GET_PATH_TO_BESTSCENT
		"		TASK_PLAY_SEQUENCE					ACTIVITY:ACT_SQUID_EAT"
		"		TASK_PLAY_SEQUENCE					ACTIVITY:ACT_SQUID_EAT"
		"		TASK_PLAY_SEQUENCE					ACTIVITY:ACT_SQUID_EAT"
		"		TASK_SQUID_EAT						50"
		"		TASK_GET_PATH_TO_LASTPOSITION		0"
		"		TASK_WALK_PATH						0"
		"		TASK_WAIT_FOR_MOVEMENT				0"
		"		TASK_CLEAR_LASTPOSITION				0"
		"	"
		"	Interrupts"
		"		COND_LIGHT_DAMAGE"
		"		COND_HEAVY_DAMAGE"
		"		COND_NEW_ENEMY"
//		"		COND_SMELL" // HOE_DLL
	)
	
	//=========================================================
	// > SCHED_SQUID_SNIFF_AND_EAT
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_SQUID_SNIFF_AND_EAT,
	
		"	Tasks"
		"		TASK_STOP_MOVING					0"
		"		TASK_SQUID_EAT						10"
		"		TASK_PLAY_SEQUENCE					ACTIVITY:ACT_SQUID_DETECT_SCENT"
		"		TASK_STORE_LASTPOSITION				0"
		"		TASK_GET_PATH_TO_BESTSCENT			0"
		"		TASK_WALK_PATH_WITHIN_DIST			64"
		"		TASK_STOP_MOVING					0"
		"		TASK_FACE_SAVEPOSITION				0" // HOE_DLL: see TASK_GET_PATH_TO_BESTSCENT
		"		TASK_PLAY_SEQUENCE					ACTIVITY:ACT_SQUID_EAT"
		"		TASK_PLAY_SEQUENCE					ACTIVITY:ACT_SQUID_EAT"
		"		TASK_PLAY_SEQUENCE					ACTIVITY:ACT_SQUID_EAT"
		"		TASK_SQUID_EAT						50"
		"		TASK_GET_PATH_TO_LASTPOSITION		0"
		"		TASK_WALK_PATH						0"
		"		TASK_WAIT_FOR_MOVEMENT				0"
		"		TASK_CLEAR_LASTPOSITION				0"
		"	"
		"	Interrupts"
		"		COND_LIGHT_DAMAGE"
		"		COND_HEAVY_DAMAGE"
		"		COND_NEW_ENEMY"
//		"		COND_SMELL" // HOE_DLL
	)
	
	//=========================================================
	// > SCHED_SQUID_WALLOW
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_SQUID_WALLOW,
	
		"	Tasks"
		"		TASK_STOP_MOVING				0"
		"		TASK_SQUID_EAT					10"
		"		TASK_STORE_LASTPOSITION			0"
		"		TASK_GET_PATH_TO_BESTSCENT		0"
		"		TASK_WALK_PATH_WITHIN_DIST		64"
		"		TASK_STOP_MOVING				0"
		"		TASK_FACE_SAVEPOSITION			0" // HOE_DLL: see TASK_GET_PATH_TO_BESTSCENT
		"		TASK_PLAY_SEQUENCE				ACTIVITY:ACT_SQUID_INSPECT_FLOOR"
		"		TASK_SQUID_EAT					50"
		"		TASK_GET_PATH_TO_LASTPOSITION	0"
		"		TASK_WALK_PATH					0"
		"		TASK_WAIT_FOR_MOVEMENT			0"
		"		TASK_CLEAR_LASTPOSITION			0"
		"	"
		"	Interrupts"
		"		COND_LIGHT_DAMAGE"
		"		COND_HEAVY_DAMAGE"
		"		COND_NEW_ENEMY"
	)
	
AI_END_CUSTOM_NPC()
