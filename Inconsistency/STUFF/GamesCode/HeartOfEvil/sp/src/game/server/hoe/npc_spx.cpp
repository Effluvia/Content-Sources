#include "cbase.h"
#include "ai_basenpc.h"
#include "fmtstr.h"
#include "npcevent.h"
#include "props.h"
#include "weapon_physcannon.h"
#include "hoe_corpse.h"
#include "ai_navigator.h"
#include "gib.h"
#include "ammodef.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern int g_interactionChainsawed;

ConVar sk_spx_health( "sk_spx_health", "0" );

//-----------------------------------------------------------------------------
// Spawnflags
//-----------------------------------------------------------------------------
#define SF_SPX_FRIENDLY (1 << 16)

// Pass these to claw attack so we know where to draw the blood.
#define ZOMBIE_BLOOD_LEFT_HAND		0
#define ZOMBIE_BLOOD_RIGHT_HAND		1
#define ZOMBIE_BLOOD_BOTH_HANDS		2

class CNPC_Spx : public CAI_BaseNPC
{
public:
	DECLARE_CLASS( CNPC_Spx, CAI_BaseNPC );
	DECLARE_DATADESC();
	DEFINE_CUSTOM_AI;

	void Spawn( void );
	void Precache( void );
	void Activate( void );

	Class_T Classify( void );
	bool ClassifyPlayerAlly( void ) { return HasSpawnFlags( SF_SPX_FRIENDLY ); }
	bool ClassifyPlayerAllyVital( void ) { return false; }
	Disposition_t IRelationType( CBaseEntity *pTarget );

	void HandleAnimEvent( animevent_t *pEvent );

	bool ShouldGib( const CTakeDamageInfo &info );
	bool HasHumanGibs( void ) { return true; }
	bool HasAlienGibs( void ) { return false; }
	bool CorpseGib( const CTakeDamageInfo &info );

	int MeleeAttack1Conditions( float flDot, float flDist );
	CBaseEntity *ClawAttack( float flDist, int iDamage, QAngle &qaViewPunch, Vector &vecVelocityPunch, int BloodOrigin  );
	float GetClawAttackRange() const { return 55; }

	void IdleSound( void );
	void PainSound( const CTakeDamageInfo &info );
	void DeathSound( const CTakeDamageInfo &info );
	void AttackHitSound( void );
	void AttackMissSound( void );

	int GetSoundInterests( void );
	void RemoveIgnoredConditions( void );

	int SelectSchedule( void );
	int TranslateSchedule( int scheduleType );
	void BuildScheduleTestBits( void );
	void StartTask( const Task_t *pTask );
	void RunTask( const Task_t *pTask );

	int OnTakeDamage_Alive( const CTakeDamageInfo &info );

	float GetGoalRepathTolerance( CBaseEntity *pGoalEnt, GoalType_t type, const Vector &curGoal, const Vector &curTargetPos );

	bool HandleInteraction( int interactionType, void *data, CBaseCombatCharacter *sourceEnt );

	float m_flChainsawedTime; // Last time we were cut by a chainsaw
	float m_flHungryTime;
	float m_flPainSoundTime;
	float m_flEatSoundTime;

	int m_iAttachmentLeftBlood;
	int m_iAttachmentRightBlood;

	//-----------------------------------------------------------------------------
	// Custom schedules.
	//-----------------------------------------------------------------------------
	enum
	{
		SCHED_SPX_MELEE_ATTACK1 = LAST_SHARED_SCHEDULE,
		SCHED_SPX_CHAINSAWED,
		SCHED_SPX_EAT,
		SCHED_SPX_BIG_FLINCH,
		SCHED_SPX_CHASE_ENEMY
	};

	//-----------------------------------------------------------------------------
	// Tasks
	//-----------------------------------------------------------------------------
	enum
	{
		TASK_SPX_DONT_EAT = BaseClass::NEXT_TASK,
		TASK_SPX_EAT,
		TASK_SPX_SOUND_EAT,
		NEXT_TASK
	};

	//-----------------------------------------------------------------------------
	// Activities
	//-----------------------------------------------------------------------------
	static Activity ACT_SPX_CHAINSAWED;
	static Activity ACT_SPX_EAT;
};

LINK_ENTITY_TO_CLASS( npc_spx, CNPC_Spx );

BEGIN_DATADESC( CNPC_Spx )
	DEFINE_FIELD( m_flChainsawedTime, FIELD_TIME ),
	DEFINE_FIELD( m_flHungryTime, FIELD_TIME ),
	DEFINE_FIELD( m_flPainSoundTime, FIELD_TIME ),
	DEFINE_FIELD( m_flEatSoundTime, FIELD_TIME ),
END_DATADESC()

//-----------------------------------------------------------------------------
// Animation events.
//-----------------------------------------------------------------------------
static Animevent AE_SPX_ATTACK_LEFT;
static Animevent AE_SPX_ATTACK_RIGHT;
static Animevent AE_SPX_ATTACK_BOTH;
static Animevent AE_SPX_EAT;

Activity CNPC_Spx::ACT_SPX_CHAINSAWED;
Activity CNPC_Spx::ACT_SPX_EAT;

//-----------------------------------------------------------------------------
// Tasks
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Conditions 
//-----------------------------------------------------------------------------

//BEGIN_DATADESC( CNPC_Spx )
//END_DATADESC()

#define SPX_HEALTH sk_spx_health.GetFloat() // 400, 600, 800
#define SPX_DMG_ONE_SLASH 20 // 10, 20, 20
#define SPX_DMG_BOTH_SLASH 30 // 25, 30, 35


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Spx::Spawn( void )
{
	BaseClass::Spawn();

	Precache();
	SetModel( STRING( GetModelName() ) );

	SetHullSizeNormal();

	SetSolid( SOLID_BBOX );
	SetMoveType( MOVETYPE_STEP );

	CapabilitiesClear();
	CapabilitiesAdd( bits_CAP_MOVE_GROUND | bits_CAP_DOORS_GROUP | bits_CAP_INNATE_MELEE_ATTACK1 );

	SetBloodColor( BLOOD_COLOR_RED );
	m_NPCState = NPC_STATE_NONE;

	m_flFieldOfView = 0.5;
	SetViewOffset( VEC_VIEW );		// Position of the eyes relative to NPC's origin.

	m_iHealth = SPX_HEALTH;

	NPCInit();
}

static const char *szBodies[] = {
	"namGrunt",
	"Male1",
	"Male2",
	"Male3",
	"Female1",
	"Female2",
	"Female3",
	"mike",
	"charlie",
};

//-----------------------------------------------------------------------------
// Purpose: Precaches all resources this monster needs.
//-----------------------------------------------------------------------------
void CNPC_Spx::Precache( void )
{
	if ( GetModelName() == NULL_STRING )
	{
#define SPX_NUM_BODIES 9
		if ( m_nBody == -1 )
		{
			m_nBody = random->RandomInt( 0, SPX_NUM_BODIES - 1 );
		}

		Assert( m_nBody >= 0 && m_nBody < SPX_NUM_BODIES );
		if ( m_nBody < 0 || m_nBody >= SPX_NUM_BODIES )
			m_nBody = 0;
		
		CFmtStr fmt;
		fmt.sprintf( "models/spx_crossbreed/%s.mdl", szBodies[m_nBody] );
		SetModelName( AllocPooledString( fmt ) );

		m_nBody = 0;
	}

	// Got to precache all the models or a monster_maker can't precache us properly
	for ( int i = 0; i < SPX_NUM_BODIES; i++ )
	{
		CFmtStr fmt;
		fmt.sprintf( "models/spx_crossbreed/%s.mdl", szBodies[i] );
		PrecacheModel( fmt );
	}

#ifdef SPX_CUSTOM_GIBS
	PrecacheModel( "Models/spx_crossbreed/gibs/Male1_gib1.mdl" );
	PrecacheModel( "Models/spx_crossbreed/gibs/Male1_gib2.mdl" );
	PrecacheModel( "Models/spx_crossbreed/gibs/Male2_gib1.mdl" );
	PrecacheModel( "Models/spx_crossbreed/gibs/Male2_gib2.mdl" );
	PrecacheModel( "Models/spx_crossbreed/gibs/Male3_gib1.mdl" );
	PrecacheModel( "Models/spx_crossbreed/gibs/Male3_gib2.mdl" );
	PrecacheModel( "Models/spx_crossbreed/gibs/namGrunt_gib1.mdl" );
	PrecacheModel( "Models/spx_crossbreed/gibs/namGrunt_gib2.mdl" );
	PrecacheModel( "Models/spx_crossbreed/gibs/namGrunt_gib3.mdl" );
#endif

	PrecacheScriptSound("NPC_Spx.FootstepLeft");
	PrecacheScriptSound("NPC_Spx.FootstepRight");
	PrecacheScriptSound("NPC_Spx.AnnounceAttack");
	PrecacheScriptSound("NPC_Spx.AttackHit");
	PrecacheScriptSound("NPC_Spx.AttackMiss");
	PrecacheScriptSound("NPC_Spx.Death");
	PrecacheScriptSound("NPC_Spx.Eat");
	PrecacheScriptSound("NPC_Spx.Pain");
	PrecacheScriptSound("NPC_Spx.Idle");

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
void CNPC_Spx::Activate( void )
{
	BaseClass::Activate();

	m_iAttachmentLeftBlood = LookupAttachment( "blood_left" );
	m_iAttachmentRightBlood = LookupAttachment( "blood_right" );
}

//-----------------------------------------------------------------------------
// Purpose: Indicates this monster's place in the relationship table.
// Output : 
//-----------------------------------------------------------------------------
Class_T CNPC_Spx::Classify( void )
{
	if ( HasSpawnFlags( SF_SPX_FRIENDLY ) )
		return CLASS_SPX_FRIEND;

	return CLASS_SPX; 
}

//-----------------------------------------------------------------------------
Disposition_t CNPC_Spx::IRelationType( CBaseEntity *pTarget )
{
	Disposition_t ret = BaseClass::IRelationType( pTarget );

	// Hack -- Don't run after rappelling solider or hanging peasant
	if ( ret == D_HT && pTarget->IsNPC() &&
		 pTarget->GetGroundEntity() == NULL &&
		 pTarget->GetAbsOrigin().z > GetAbsOrigin().z + 100 )
		return D_NU;

	return ret;
}

//-----------------------------------------------------------------------------
bool CNPC_Spx::ShouldGib( const CTakeDamageInfo &info )
{
	if ( info.GetDamageType() & (DMG_NEVERGIB|DMG_DISSOLVE) )
		return false;

	if ( info.GetDamageType() & (DMG_ALWAYSGIB|DMG_BLAST) )
		return true;

	if ( m_flChainsawedTime > gpGlobals->curtime - 0.25f )
		return true;

#ifdef SPX_CUSTOM_GIBS
	if ( GetHealth() < -40 && info.GetAmmoType() == GetAmmoDef()->Index( "ElephantShot" ) )
		return true;
#endif

	return false;
}

//-----------------------------------------------------------------------------
bool CNPC_Spx::CorpseGib( const CTakeDamageInfo &info )
{
#ifdef SPX_CUSTOM_GIBS
	const char *szModel = STRING(GetModelName());
	int i;
	for ( i = 0; i < ARRAYSIZE(szBodies); i++ )
	{
		if ( Q_stristr( szModel, szBodies[i] ) != NULL )
			break;
	}
	if ( i > 3 )
		return BaseClass::CorpseGib( info );

	Vector force = info.GetDamageForce(); //CalcDamageForceVector( info );
	force *= 0.35;

	bool bIgnite = false;
	float flFadeTime = 0.0f;

	if ( i == 0 )
	{
		CBaseEntity *pGib1 = CreateRagGib( "Models/spx_crossbreed/gibs/namGrunt_gib1.mdl",
			GetAbsOrigin(), GetAbsAngles(), force, flFadeTime, bIgnite );

		CBaseEntity *pGib2 = CreateRagGib( "Models/spx_crossbreed/gibs/namGrunt_gib2.mdl",
			GetAbsOrigin(), GetAbsAngles(), force, flFadeTime, bIgnite );

		CBaseEntity *pGib3 = CreateRagGib( "Models/spx_crossbreed/gibs/namGrunt_gib3.mdl",
			GetAbsOrigin(), GetAbsAngles(), force * 0.2, flFadeTime, bIgnite );
	}

	if ( i == 1 )
	{
		CBaseEntity *pGib1 = CreateRagGib( "Models/spx_crossbreed/gibs/Male1_gib1.mdl",
			GetAbsOrigin(), GetAbsAngles(), force, flFadeTime, bIgnite );

		CBaseEntity *pGib2 = CreateRagGib( "Models/spx_crossbreed/gibs/Male1_gib2.mdl",
			GetAbsOrigin(), GetAbsAngles(), force, flFadeTime, bIgnite );
	}
	if ( i == 2 )
	{
		CBaseEntity *pGib1 = CreateRagGib( "Models/spx_crossbreed/gibs/Male2_gib1.mdl",
			GetAbsOrigin(), GetAbsAngles(), force, flFadeTime, bIgnite );

		CBaseEntity *pGib2 = CreateRagGib( "Models/spx_crossbreed/gibs/Male2_gib2.mdl",
			GetAbsOrigin(), GetAbsAngles(), force, flFadeTime, bIgnite );
	}
	if ( i == 3 )
	{
		CBaseEntity *pGib1 = CreateRagGib( "Models/spx_crossbreed/gibs/Male3_gib1.mdl",
			GetAbsOrigin(), GetAbsAngles(), force, flFadeTime, bIgnite );

		CBaseEntity *pGib2 = CreateRagGib( "Models/spx_crossbreed/gibs/Male3_gib2.mdl",
			GetAbsOrigin(), GetAbsAngles(), force, flFadeTime, bIgnite );
	}

	return true;
#else
	return BaseClass::CorpseGib( info );
#endif
}

//-----------------------------------------------------------------------------
void CNPC_Spx::HandleAnimEvent( animevent_t *pEvent )
{
	if (pEvent->event == AE_SPX_ATTACK_LEFT)
	{
		Vector right, forward;
		AngleVectors( GetLocalAngles(), &forward, &right, NULL );

		right = right * 200;
		forward = forward * 20;

		ClawAttack( GetClawAttackRange(), SPX_DMG_ONE_SLASH, QAngle( -15, 20, -10 ), right + forward, ZOMBIE_BLOOD_LEFT_HAND );
		return;
	}

	if (pEvent->event == AE_SPX_ATTACK_RIGHT)
	{
		Vector right, forward;
		AngleVectors( GetLocalAngles(), &forward, &right, NULL );
		
		right = right * -200;
		forward = forward * 20;

		ClawAttack( GetClawAttackRange(), SPX_DMG_ONE_SLASH, QAngle( -15, -20, -10 ), right + forward, ZOMBIE_BLOOD_RIGHT_HAND );
		return;
	}

	// BUG BUG BUG in BaseZombie, it uses sk_zombie_dmg_one_slash instead of sk_zombie_dmg_both_slash
	if (pEvent->event == AE_SPX_ATTACK_BOTH)
	{
		Vector forward;
		QAngle qaPunch( 45, random->RandomInt(-5,5), random->RandomInt(-5,5) );
		AngleVectors( GetLocalAngles(), &forward );
		forward = forward * 200;
		ClawAttack( GetClawAttackRange(), SPX_DMG_BOTH_SLASH, qaPunch, forward, ZOMBIE_BLOOD_BOTH_HANDS );
		return;
	}

	// Play the eating sound when eating so it plays during the numerous eating
	// scripted_sequences.
	if (pEvent->event == AE_SPX_EAT)
	{
		if ( m_flEatSoundTime < gpGlobals->curtime )
		{
			float flDuration;
			EmitSound( "NPC_Spx.Eat", 0, &flDuration );
			m_flEatSoundTime = gpGlobals->curtime + flDuration;
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
int CNPC_Spx::MeleeAttack1Conditions( float flDot, float flDist )
{
	if (flDist > GetClawAttackRange() )
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

					if( flDist <= GetClawAttackRange() )
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
	AI_TraceHull( WorldSpaceCenter(), WorldSpaceCenter() + forward * GetClawAttackRange(), vecMins, vecMaxs, MASK_NPCSOLID, &traceFilter, &tr );

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
CBaseEntity *CNPC_Spx::ClawAttack( float flDist, int iDamage, QAngle &qaViewPunch, Vector &vecVelocityPunch, int BloodOrigin  )
{
	// Added test because claw attack anim sometimes used when for cases other than melee
	if ( GetEnemy() )
	{
#if 0 // HOE: removed so zombie can smash player-carried func_physbox breakables
		trace_t	tr;
		AI_TraceHull( WorldSpaceCenter(), GetEnemy()->WorldSpaceCenter(), -Vector(8,8,8), Vector(8,8,8), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr );

		if ( tr.fraction < 1.0f )
			return NULL;
#endif

		// Reduce damage done to NPCs to avoid mass slaughter.
		// FIXME: this may not be the entity hit by CheckTraceHullAttack().
		if ( GetEnemy()->IsNPC() )
			iDamage /= 2;
	}

	//
	// Trace out a cubic section of our hull and see what we hit.
	//
	Vector vecMins = GetHullMins();
	Vector vecMaxs = GetHullMaxs();
	vecMins.z = vecMins.x;
	vecMaxs.z = vecMaxs.x;

	CBaseEntity *pHurt = CheckTraceHullAttack( flDist, vecMins, vecMaxs, iDamage, DMG_SLASH, 2.0f, false, vecVelocityPunch );
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
		else if( !pPlayer && UTIL_ShouldShowBlood(pHurt->BloodColor()) )
		{
			// Hit an NPC. Bleed them!
			Vector vecBloodPos;

			switch( BloodOrigin )
			{
			case ZOMBIE_BLOOD_LEFT_HAND:
				if( GetAttachment( m_iAttachmentLeftBlood, vecBloodPos ) )
					SpawnBlood( vecBloodPos, g_vecAttackDir, pHurt->BloodColor(), min( iDamage, 30 ) );
				break;

			case ZOMBIE_BLOOD_RIGHT_HAND:
				if( GetAttachment( m_iAttachmentRightBlood, vecBloodPos ) )
					SpawnBlood( vecBloodPos, g_vecAttackDir, pHurt->BloodColor(), min( iDamage, 30 ) );
				break;

			case ZOMBIE_BLOOD_BOTH_HANDS:
				if( GetAttachment( m_iAttachmentLeftBlood, vecBloodPos ) )
					SpawnBlood( vecBloodPos, g_vecAttackDir, pHurt->BloodColor(), min( iDamage, 30 ) );

				if( GetAttachment( m_iAttachmentRightBlood, vecBloodPos ) )
					SpawnBlood( vecBloodPos, g_vecAttackDir, pHurt->BloodColor(), min( iDamage, 30 ) );
				break;
			}
		}
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
void CNPC_Spx::IdleSound( void )
{
	if ( IsCurSchedule( SCHED_SPX_EAT ) )
		return;
	EmitSound( "NPC_Spx.Idle" );
}

//-----------------------------------------------------------------------------
void CNPC_Spx::PainSound( const CTakeDamageInfo &info )
{
	if ( m_flPainSoundTime < gpGlobals->curtime )
	{
		float flDuration;
		EmitSound( "NPC_Spx.Pain", 0, &flDuration );
		m_flPainSoundTime = gpGlobals->curtime + flDuration +
			random->RandomFloat( 2.0, 4.0 );
	}
}

//-----------------------------------------------------------------------------
void CNPC_Spx::DeathSound( const CTakeDamageInfo &info )
{
	SentenceStop();

	if ( ShouldGib( info ) ) return;

	EmitSound( "NPC_Spx.Death" );
}

//-----------------------------------------------------------------------------
// Purpose: Play a random attack sound.
//-----------------------------------------------------------------------------
void CNPC_Spx::AttackHitSound( void )
{
	EmitSound( "NPC_Spx.AttackHit" );
}

//-----------------------------------------------------------------------------
// Purpose: Play a random attack sound.
//-----------------------------------------------------------------------------
void CNPC_Spx::AttackMissSound( void )
{
	EmitSound( "NPC_Spx.AttackMiss" );
}

//=========================================================
// GetSoundInterests - returns a bit mask indicating which types
// of sounds this monster regards. 
//=========================================================
int CNPC_Spx::GetSoundInterests( void ) 
{
	return BaseClass::GetSoundInterests() | ALL_SCENTS;
}

//---------------------------------------------------------
void CNPC_Spx::RemoveIgnoredConditions( void )
{
	BaseClass::RemoveIgnoredConditions();

	if ( m_flHungryTime > gpGlobals->curtime )
		 ClearCondition( COND_SMELL );

	// If we could do a big flinch allow heavy damage to interrupt us chasing our enemy.
	// Otherwise only flinch *gestures* will play while chasing our enemy.
	// I couldn't just SetCustomInterruptCondition(COND_HEAVY_DAMAGE) in BuildScheduleTestBits()
	// because CheckFlinches() is called in GatherConditions().
	if ( IsCurSchedule( SCHED_SPX_CHASE_ENEMY ) &&
		HasCondition( COND_HEAVY_DAMAGE ) &&
		( HasMemory(bits_MEMORY_FLINCHED) ||
		m_flNextFlinchTime >= gpGlobals->curtime ) )
	{
		ClearCondition( COND_HEAVY_DAMAGE );
	}
}

//---------------------------------------------------------
int CNPC_Spx::SelectSchedule( void )
{
	// When a schedule ends, conditions are set back to what GatherConditions
	// reported before selecting a new schedule.  See CAI_BaseNPC::MaintainSchedule.
	// Anything RemoveIgnoredConditions did is lost.
	if ( m_flHungryTime > gpGlobals->curtime )
		 ClearCondition( COND_SMELL );

	if ( m_flChainsawedTime > gpGlobals->curtime - 0.25f )
		return SCHED_SPX_CHAINSAWED;

	if ( GetState() == NPC_STATE_COMBAT && HasCondition( COND_SMELL ) && HasCondition( COND_ENEMY_OCCLUDED ) )
		return SCHED_SPX_EAT;

//	if ( GetState() == NPC_STATE_ALERT && HasCondition( COND_ENEMY_DEAD ) )
//		return SCHED_SPX_VICTORY_DANCE;

	if ( GetState() == NPC_STATE_ALERT && HasCondition( COND_SMELL ) )
		return SCHED_SPX_EAT;

	if ( GetState() == NPC_STATE_IDLE && HasCondition( COND_SMELL ) )
		return SCHED_SPX_EAT;

	return BaseClass::SelectSchedule();
}

//---------------------------------------------------------
int CNPC_Spx::TranslateSchedule( int scheduleType )
{
	switch( scheduleType )
	{
	case SCHED_MELEE_ATTACK1:
		return SCHED_SPX_MELEE_ATTACK1;

	case SCHED_BIG_FLINCH:
		return SCHED_SPX_BIG_FLINCH;

	case SCHED_CHASE_ENEMY:
		return SCHED_SPX_CHASE_ENEMY;
	}

	return BaseClass::TranslateSchedule( scheduleType );
}

//-----------------------------------------------------------------------------
// Purpose: Allows for modification of the interrupt mask for the current schedule.
//			In the most cases the base implementation should be called first.
//-----------------------------------------------------------------------------
void CNPC_Spx::BuildScheduleTestBits( void )
{
	// Ignore damage if we were recently damaged or we're attacking.
	if ( GetActivity() == ACT_MELEE_ATTACK1 )
	{
		ClearCustomInterruptCondition( COND_LIGHT_DAMAGE );
		ClearCustomInterruptCondition( COND_HEAVY_DAMAGE );
	}
#ifndef HL2_EPISODIC
	else if ( m_flNextFlinchTime >= gpGlobals->curtime )
	{
		ClearCustomInterruptCondition( COND_LIGHT_DAMAGE );
		ClearCustomInterruptCondition( COND_HEAVY_DAMAGE );
	}
#endif // !HL2_EPISODIC
		 
	BaseClass::BuildScheduleTestBits();
}

//-----------------------------------------------------------------------------
void CNPC_Spx::StartTask( const Task_t *pTask )
{
	switch( pTask->iTask )
	{
	case TASK_ANNOUNCE_ATTACK:
		{
			CSoundEnt::InsertSound( SOUND_COMBAT, EyePosition(), 512, 0.25, this );
			EmitSound("NPC_Spx.AnnounceAttack"); // could I use responserules?
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
	case TASK_SPX_DONT_EAT: // Don't eat - set me to not hungry for a while
		m_flHungryTime = gpGlobals->curtime + pTask->flTaskData;
		TaskComplete();
		break;
	case TASK_SPX_EAT: // Eat - set me to not hungry for a while AND increase my health
		m_flHungryTime = gpGlobals->curtime + pTask->flTaskData;
		TakeHealth( pTask->flTaskData, DMG_GENERIC );
		TaskComplete();
		break;
	case TASK_SPX_SOUND_EAT:
		if ( pTask->flTaskData == 0 || random->RandomFloat(0, 1) <= pTask->flTaskData )
			EmitSound( "NPC_Spx.Eat" );
		TaskComplete();
		break;
	case TASK_PLAY_SEQUENCE:
		BaseClass::StartTask( pTask );
		if ( IsActivityFinished() )
			ResetSequenceInfo(); // hack for repeating ACT_SPX_EAT
		break;
	default:
		BaseClass::StartTask(pTask);
		break;
	}
}

//-----------------------------------------------------------------------------
void CNPC_Spx::RunTask( const Task_t *pTask )
{
	switch (pTask->iTask)
	{
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
int CNPC_Spx::OnTakeDamage_Alive( const CTakeDamageInfo &info )
{
	// Disallow damage from server-side ragdolls.
	// I see these guys clawing a human whose ragdoll then kills them.
	if ( info.GetAttacker() &&
		FClassnameIs( info.GetAttacker(), "prop_ragdoll" ) )
	{
		return 0;
	}

	return BaseClass::OnTakeDamage_Alive( info );
}

//-----------------------------------------------------------------------------
// In the case of goaltype enemy, update the goal position
//-----------------------------------------------------------------------------
float CNPC_Spx::GetGoalRepathTolerance( CBaseEntity *pGoalEnt, GoalType_t type, const Vector &curGoal, const Vector &curTargetPos )
{
	float distToGoal = ( GetAbsOrigin() - curTargetPos ).Length() - GetNavigator()->GetArrivalDistance();

	// If my enemy has moved significantly, update my path.
	// It looks strange when a zombie walks a few extra steps to where
	// the player was a second ago.
	if ( type == GOALTYPE_ENEMY &&
		distToGoal < 128 )
	{
		return 40;
	}

	return BaseClass::GetGoalRepathTolerance( pGoalEnt, type, curGoal, curTargetPos );
}

//-----------------------------------------------------------------------------
bool CNPC_Spx::HandleInteraction( int interactionType, void *data, CBaseCombatCharacter *sourceEnt )
{
	if ( interactionType == g_interactionChainsawed )
	{
		m_flChainsawedTime = gpGlobals->curtime;

		if ( !IsInAScript() && (GetState() != NPC_STATE_SCRIPT) &&
			!IsCurSchedule( SCHED_SPX_CHAINSAWED ) )
		{
			ClearSchedule( "chainsaw interaction" );
		}

		return true;
	}
	return false;
}

AI_BEGIN_CUSTOM_NPC( npc_spx, CNPC_Spx )

	DECLARE_ANIMEVENT( AE_SPX_ATTACK_LEFT )
	DECLARE_ANIMEVENT( AE_SPX_ATTACK_RIGHT )
	DECLARE_ANIMEVENT( AE_SPX_ATTACK_BOTH )
	DECLARE_ANIMEVENT( AE_SPX_EAT )

	DECLARE_ACTIVITY( ACT_SPX_CHAINSAWED )
	DECLARE_ACTIVITY( ACT_SPX_EAT )

	DECLARE_TASK( TASK_SPX_DONT_EAT )
	DECLARE_TASK( TASK_SPX_EAT )
	DECLARE_TASK( TASK_SPX_SOUND_EAT )

	//=========================================================
	// BaseNPC behaviour will stop in the middle of a swing if the
	// zombie takes damage (among other things).
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_SPX_MELEE_ATTACK1,

		"	Tasks"
		"		TASK_STOP_MOVING		0"
		"		TASK_FACE_ENEMY			0"
		"		TASK_ANNOUNCE_ATTACK	1"	// 1 = primary attack
		"		TASK_MELEE_ATTACK1		0"
//		"		TASK_SET_SCHEDULE		SCHEDULE:SCHED_ZOMBIE_POST_MELEE_WAIT"
		""
		"	Interrupts"
		"		COND_ENEMY_DEAD"
		"		COND_LIGHT_DAMAGE"
		"		COND_HEAVY_DAMAGE"
		"		COND_ENEMY_OCCLUDED"
	)	

	DEFINE_SCHEDULE
	(
		SCHED_SPX_EAT,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_IDLE_STAND"
		"		TASK_STOP_MOVING				0"
		"		TASK_SPX_DONT_EAT				10"
		"		TASK_STORE_LASTPOSITION			0"
//		"		TASK_STORE_BESTSCENT_IN_SAVEPOSITION 0"
		"		TASK_GET_PATH_TO_BESTSCENT		0"
		"		TASK_FACE_IDEAL					0"
		"		TASK_WALK_PATH_WITHIN_DIST		64"
		"		TASK_STOP_MOVING				0"
		"		TASK_FACE_SAVEPOSITION			0" // see TASK_GET_PATH_TO_BESTSCENT
//		"		TASK_WAIT_FOR_MOVEMENT			0"
//		"		TASK_SPX_SOUND_EAT				0"
		"		TASK_PLAY_SEQUENCE				ACTIVITY:ACT_CROUCH"
//		"		TASK_SPX_SOUND_EAT				0"
		"		TASK_PLAY_SEQUENCE				ACTIVITY:ACT_SPX_EAT"
		"		TASK_PLAY_SEQUENCE				ACTIVITY:ACT_SPX_EAT"
//		"		TASK_SPX_SOUND_EAT				0"
		"		TASK_PLAY_SEQUENCE				ACTIVITY:ACT_SPX_EAT"
		"		TASK_PLAY_SEQUENCE				ACTIVITY:ACT_SPX_EAT"
		"		TASK_PLAY_SEQUENCE				ACTIVITY:ACT_STAND"
		"		TASK_SPX_EAT					50"
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
		SCHED_SPX_CHAINSAWED,

		"	Tasks"
		"		TASK_STOP_MOVING		0"
		"		TASK_PLAY_SEQUENCE		ACTIVITY:ACT_SPX_CHAINSAWED"
		""
		"	Interrupts"
	)	

	DEFINE_SCHEDULE
	(
		SCHED_SPX_BIG_FLINCH,

		"	Tasks"
		"		TASK_REMEMBER			MEMORY:FLINCHED"
		"		TASK_STOP_MOVING		0"
		"		TASK_BIG_FLINCH			0"
		"		TASK_SET_ACTIVITY		ACTIVITY:ACT_IDLE" // HOE: added
		"		TASK_WAIT				0.5" // HOE: added wait
		""
		"	Interrupts"
	)	

	DEFINE_SCHEDULE
	(
		SCHED_SPX_CHASE_ENEMY,

		"	Tasks"
		"		TASK_STOP_MOVING				0"
		"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_CHASE_ENEMY_FAILED"
	//	"		TASK_SET_TOLERANCE_DISTANCE		24"
		"		TASK_GET_CHASE_PATH_TO_ENEMY	300"
		"		TASK_RUN_PATH					0"
		"		TASK_WAIT_FOR_MOVEMENT			0"
		"		TASK_FACE_ENEMY			0"
		""
		"	Interrupts"
		"		COND_NEW_ENEMY"
		"		COND_ENEMY_DEAD"
		"		COND_ENEMY_UNREACHABLE"
		"		COND_CAN_RANGE_ATTACK1"
		"		COND_CAN_MELEE_ATTACK1"
		"		COND_CAN_RANGE_ATTACK2"
		"		COND_CAN_MELEE_ATTACK2"
		"		COND_TOO_CLOSE_TO_ATTACK"
		"		COND_TASK_FAILED"
		"		COND_LOST_ENEMY"
		"		COND_BETTER_WEAPON_AVAILABLE"
		"		COND_HEAR_DANGER"
		"		COND_HEAVY_DAMAGE" // HOE: added so SCHED_SPX_BIG_FLINCH will run
	)

AI_END_CUSTOM_NPC()

//-----------------------------------------------------------------------------

#if 0 // prop_ragdoll version

// see monstermaker.cpp
static void DispatchActivate( CBaseEntity *pEntity )
{
	bool bAsyncAnims = mdlcache->SetAsyncLoad( MDLCACHE_ANIMBLOCK, false );
	pEntity->Activate();
	mdlcache->SetAsyncLoad( MDLCACHE_ANIMBLOCK, bAsyncAnims );
}

class CDeadSpx : public CBaseEntity
{
public:
	DECLARE_CLASS( CDeadSpx, CBaseEntity );
	DECLARE_DATADESC();

	void Spawn( void )
	{
#define SPX_NUM_BODIES 9
		if ( m_nBody == -1 )
		{
			m_nBody = random->RandomInt( 0, SPX_NUM_BODIES - 1 );
		}

		static const char *szBodies[] = {
			"namGrunt",
			"Male1",
			"Male2",
			"Male3",
			"Female1",
			"Female2",
			"Female3",
			"mike",
			"charlie",
		};

		Assert( m_nBody >= 0 && m_nBody < SPX_NUM_BODIES );
		if ( m_nBody < 0 || m_nBody >= SPX_NUM_BODIES )
			m_nBody = 0;
		
		CFmtStr fmt;
		fmt.sprintf( "models/spx_crossbreed/%s.mdl", szBodies[m_nBody] );

		CBaseEntity *pPropRagdoll = CreateEntityByName( "prop_ragdoll" );
		pPropRagdoll->SetAbsOrigin( GetAbsOrigin() );
		pPropRagdoll->SetAbsAngles( GetAbsAngles() );
		pPropRagdoll->KeyValue( "model", fmt );
		pPropRagdoll->KeyValue( "InitialPose", "freak" );
		pPropRagdoll->KeyValue( "spawnflags", "65540" );
		DispatchSpawn( pPropRagdoll );
		DispatchActivate( pPropRagdoll );

		UTIL_Remove( this );
	}

	int m_nBody;
	int m_pose;
};

BEGIN_DATADESC( CDeadSpx )
	DEFINE_KEYFIELD( m_nBody, FIELD_INTEGER, "body" ),
	DEFINE_KEYFIELD( m_pose, FIELD_INTEGER, "pose" ),
END_DATADESC()

#else

class CDeadSpx : public CHOECorpse
{
public:
	DECLARE_CLASS( CDeadSpx, CHOECorpse );

	void Spawn( void )
	{
		CorpsePose poses[] = {
			{ "freak", false },
			{ NULL, false }
		};

		SelectModel();

		SetBloodColor( BLOOD_COLOR_RED );
		InitCorpse( STRING( GetModelName() ), poses );

		// FIXME: Probably don't want SOUND_CARCASS from this
		BaseClass::Spawn();
	}

	void SelectModel( void )
	{
#define SPX_NUM_BODIES 9
		if ( m_nBody == -1 )
		{
			m_nBody = random->RandomInt( 0, SPX_NUM_BODIES - 1 );
		}

		static const char *szBodies[] = {
			"namGrunt",
			"Male1",
			"Male2",
			"Male3",
			"Female1",
			"Female2",
			"Female3",
			"mike",
			"charlie",
		};

		Assert( m_nBody >= 0 && m_nBody < SPX_NUM_BODIES );
		if ( m_nBody < 0 || m_nBody >= SPX_NUM_BODIES )
			m_nBody = 0;
		
		CFmtStr fmt;
		fmt.sprintf( "models/spx_crossbreed/%s.mdl", szBodies[m_nBody] );
		SetModelName( AllocPooledString( fmt ) );

		m_nBody = 0;
	}
};

#endif

LINK_ENTITY_TO_CLASS( npc_spx_dead, CDeadSpx );