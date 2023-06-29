#include "cbase.h"
#include "hoe_base_zombie.h"
#include "gib.h"
#include "ai_squad.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define FOLLOW_PLAYER_FORMATION		AIF_VORTIGAUNT // AIF_SIDEKICK
#define FOLLOW_POINT_FORMATION		AIF_COMMANDER

extern int g_interactionChainsawed;

//IMPLEMENT_SERVERCLASS_ST(CHOEBaseZombie, DT_NPC_HGrunt)
//END_SEND_TABLE()

BEGIN_DATADESC( CHOEBaseZombie )
	DEFINE_FIELD( m_nHeadNum, FIELD_INTEGER ),
	DEFINE_FIELD( m_bMovingAwayFromPlayer, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flChainsawedTime, FIELD_TIME ),
	DEFINE_FIELD( m_flHungryTime, FIELD_TIME ),
	DEFINE_FIELD( m_flPainTime, FIELD_TIME ),
	DEFINE_FIELD( m_flLastKick, FIELD_TIME ),
	DEFINE_FIELD( m_flAttackedByHueyTime, FIELD_TIME ),

	DEFINE_OUTPUT( m_OnPlayerUse, "OnPlayerUse" ),

	DEFINE_USEFUNC( UseFunc ),
END_DATADESC()

//-----------------------------------------------------------------------------
// Activities
//-----------------------------------------------------------------------------
Activity CHOEBaseZombie::ACT_ZOMBIE_CHAINSAWED;
Activity CHOEBaseZombie::ACT_ZOMBIE_EAT;

//-----------------------------------------------------------------------------
// Animation events.
//-----------------------------------------------------------------------------
Animevent CHOEBaseZombie::AE_ZOMBIE_BITE;
Animevent CHOEBaseZombie::AE_ZOMBIE_KICK;

#define ZOMBIE_DMG_BITE 10 // 5, 10, 10
#define ZOMBIE_DMG_KICK 10 // 5, 10, 10

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHOEBaseZombie::Spawn( void )
{
	BaseClass::Spawn();

	Precache();
	SetModel( STRING( GetModelName() ) );
	SelectModelGroups();

	SetHullType(HULL_HUMAN);
	SetHullSizeNormal();
	SetSolid( SOLID_BBOX );

	SetBloodColor( BLOOD_COLOR_RED );
	m_flFieldOfView = 0.02;
	m_NPCState = NPC_STATE_NONE;

	m_HackedGunPos = Vector( 0, 0, 55 ); // what is this garbage?

	CapabilitiesClear();
	CapabilitiesAdd( bits_CAP_SQUAD );
	CapabilitiesAdd( /* bits_CAP_ANIMATEDFACE |*/ bits_CAP_TURN_HEAD );
	CapabilitiesAdd( bits_CAP_USE_WEAPONS | bits_CAP_AIM_GUN /* | bits_CAP_MOVE_SHOOT */ );
	CapabilitiesAdd( bits_CAP_DUCK | bits_CAP_DOORS_GROUP );
	CapabilitiesAdd( bits_CAP_USE_SHOT_REGULATOR );
	CapabilitiesAdd( /* bits_CAP_NO_HIT_PLAYER | */ bits_CAP_NO_HIT_SQUADMATES /* | bits_CAP_FRIENDLY_DMG_IMMUNE */ );
	CapabilitiesAdd( bits_CAP_MOVE_GROUND );

	// Bite
	if ( LookupActivity( "ACT_MELEE_ATTACK1" ) != ACT_INVALID )
		CapabilitiesAdd( bits_CAP_INNATE_MELEE_ATTACK1 );

	// Kick
	if ( LookupActivity( "ACT_MELEE_ATTACK2" ) != ACT_INVALID )
		CapabilitiesAdd( bits_CAP_INNATE_MELEE_ATTACK2 );

	// Combine doesn't set this (because it is expensive to check?)
//	if ( HasSpawnFlags( SF_HUMAN_GRENADES ) )
//		CapabilitiesAdd( bits_CAP_INNATE_RANGE_ATTACK2 );

	SetMoveType( MOVETYPE_STEP );

	NPCInit();

	// Must be *after* NPCInit()
	SetUse( &CHOEBaseZombie::UseFunc );
}

//-----------------------------------------------------------------------------
bool CHOEBaseZombie::CreateBehaviors( void )
{
	AddBehavior( &m_FollowBehavior );
	
	return BaseClass::CreateBehaviors();
}

//-----------------------------------------------------------------------------
int CHOEBaseZombie::ObjectCaps( void ) 
{ 
	// Set a flag indicating this NPC is usable by the player
	int caps = UsableNPCObjectCaps( BaseClass::ObjectCaps() );
	return caps; 
}

//-----------------------------------------------------------------------------
// Purpose: Precaches all resources this monster needs.
//-----------------------------------------------------------------------------
void CHOEBaseZombie::Precache( void )
{
	if ( GetModelName() == NULL_STRING )
	{
		SelectModel();
	}
	Assert( GetModelName() );

	PrecacheModel( STRING( GetModelName() ) );

	PrecacheScriptSound("NPC_SuperZombie.Bite");
	PrecacheScriptSound("NPC_SuperZombie.Eat");

	InitSentences();

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
bool CHOEBaseZombie::ShouldGib( const CTakeDamageInfo &info )
{
	if ( info.GetDamageType() & (DMG_NEVERGIB|DMG_DISSOLVE) )
		return false;

	if ( info.GetDamageType() & (DMG_ALWAYSGIB|DMG_BLAST) )
		return true;

	if ( m_flChainsawedTime > gpGlobals->curtime - 0.25f )
		return true;

	return false;
}

//-----------------------------------------------------------------------------
Disposition_t CHOEBaseZombie::IRelationType( CBaseEntity *pTarget )
{
	if ( pTarget && FClassnameIs( pTarget, "npc_huey" ) )
	{
		if ( GetActiveWeapon() && FClassnameIs( GetActiveWeapon(), "weapon_rpg7" ) )
			return D_HT;
	
		if ( AfraidOfHuey() )
			return D_FR;

		return D_NU;
	}

	return BaseClass::IRelationType( pTarget );
}

//-----------------------------------------------------------------------------
bool CHOEBaseZombie::AfraidOfHuey( void )
{
	// Don't run and hide from the huey unless it attacked us recently
	return ( m_flAttackedByHueyTime > gpGlobals->curtime - 10.0 );
}

//-----------------------------------------------------------------------------
bool CHOEBaseZombie::HandleInteraction( int interactionType, void *data, CBaseCombatCharacter *sourceEnt )
{
	if ( interactionType == g_interactionChainsawed )
	{
		m_flChainsawedTime = gpGlobals->curtime;

		if ( !IsInAScript() && (GetState() != NPC_STATE_SCRIPT) &&
			!IsCurSchedule( SCHED_ZOMBIE_CHAINSAWED ) )
		{
			ClearSchedule( "chainsaw interaction" );
		}

		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
void CHOEBaseZombie::Speak( const char *pSentence )
{
	char szSentence[128];
	Q_snprintf( szSentence, sizeof(szSentence), "%s_%s", GetSentenceGroup(), pSentence );
	DevMsg( "CHOEBaseZombie::Speak %s %s\n", GetDebugName(), szSentence );

	float volume = VOL_NORM;
	float delay = 0.0;
//	int flags = 0;
//	int pitch = PITCH_NORM;
//	int index = SENTENCEG_PlayRndSz( edict(), szSentence, volume, SNDLVL_TALKING, flags, pitch);
	PlaySentence(szSentence, delay, volume, SNDLVL_TALKING, 0);
}

//-----------------------------------------------------------------------------
void CHOEBaseZombie::AlertSound( void )
{
	if ( !HasAHead() ) return;
	Speak( "ALERT" );
}

//-----------------------------------------------------------------------------
void CHOEBaseZombie::AttackSound( void )
{
	if ( !HasAHead() ) return;
	Speak( "ATTACK" );
}

//-----------------------------------------------------------------------------
void CHOEBaseZombie::IdleSound( void )
{
	if ( !HasAHead() ) return;

	if ( IsCurSchedule( SCHED_ZOMBIE_EAT ) )
		return;

	Speak( "IDLE" );
}

//-----------------------------------------------------------------------------
void CHOEBaseZombie::PainSound( const CTakeDamageInfo &info )
{
	if ( !HasAHead() ) return;

	// Make sure he doesn't just repeatedly go uh-uh-uh-uh-uh when hit by rapid fire
	if ( gpGlobals->curtime < m_flPainTime ) return;
	float flDuration = 2.0f; // FIXME: duration of speech
	m_flPainTime = gpGlobals->curtime + flDuration + random->RandomFloat( 2.0, 4.0 );

	Speak( "PAIN" );
}

//-----------------------------------------------------------------------------
void CHOEBaseZombie::DeathSound( const CTakeDamageInfo &info )
{
	SentenceStop();

	if ( ShouldGib( info ) ) return;

	if ( !HasAHead() ) return;

	Speak( "PAIN" );
}

//-----------------------------------------------------------------------------
int CHOEBaseZombie::TranslateSchedule( int scheduleType )
{
	// I want the follow behavior to have a chance of stomping schedules.
	if ( GetFollowBehavior().IsRunning() )
		scheduleType = GetFollowBehavior().TranslateScheduleHack( scheduleType );

	switch( scheduleType )
	{
	case SCHED_HIDE_AND_RELOAD: // zombies don't run, so reload on the spot
	case SCHED_RELOAD:
		return SCHED_ZOMBIE_RELOAD;
	}

	return BaseClass::TranslateSchedule( scheduleType );
}

//-----------------------------------------------------------------------------
void CHOEBaseZombie::GatherConditions( void )
{
	BaseClass::GatherConditions();

	SpeedBoost();

	PredictPlayerPush();

	if ( GetMotor()->IsDeceleratingToGoal() && IsCurTaskContinuousMove() && 
		 HasCondition( COND_PLAYER_PUSHING ) && IsCurSchedule( SCHED_MOVE_AWAY ) )
	{
		ClearSchedule( "player pushing" );
	}
}

//-----------------------------------------------------------------------------
void CHOEBaseZombie::HandleAnimEvent( animevent_t *pEvent )
{
	if ( pEvent->event == AE_ZOMBIE_BITE )
	{
		Vector forward;
		AngleVectors( GetAbsAngles(), &forward );

		Vector origin;
		QAngle angles;
		GetAttachment( LookupAttachment( "head" ), origin, angles );

		Vector vecStart = origin;
		Vector vecEnd = vecStart + forward * 50;
		CBaseEntity *pHurt = CheckTraceHullAttack( vecStart, vecEnd, Vector(-12,-12,-12), Vector(12,12,12),
			ZOMBIE_DMG_BITE, DMG_SLASH );
		if ( pHurt )
			EmitSound( "NPC_SuperZombie.Bite" );
		return;
	}

	if ( pEvent->event == AE_ZOMBIE_KICK )
	{
		m_flLastKick = gpGlobals->curtime;

		// Does no damage, because damage is applied based upon whether the target can handle the interaction
		CBaseEntity *pHurt = CheckTraceHullAttack( 70, -Vector(16,16,18), Vector(16,16,18), 0, DMG_CLUB );
		CBaseCombatCharacter *pBCC = ToBaseCombatCharacter( pHurt );
		if (pBCC)
		{
			Vector forward, up;
			AngleVectors( GetLocalAngles(), &forward, NULL, &up );

			if ( pBCC->IsPlayer() )
			{
				pBCC->ViewPunch( QAngle(-12,-7,0) );
				pHurt->ApplyAbsVelocityImpulse( forward * 100 + up * 50 );
			}

			CTakeDamageInfo info( this, this, ZOMBIE_DMG_KICK, DMG_CLUB );
			CalculateMeleeDamageForce( &info, forward, pBCC->GetAbsOrigin() );
			pBCC->TakeDamage( info );
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
// Our zombies can aim up/down using the aim_pitch pose parameter but not left/right. So
// clear out bits_CAP_AIM_GUN when seeing if our gun is pointed at the enemy.
bool CHOEBaseZombie::FInAimCone( const Vector &vecSpot )
{
	Vector los = ( vecSpot - GetAbsOrigin() );

	// do this in 2D
	los.z = 0;
	VectorNormalize( los );

	Vector facingDir = BodyDirection2D( );

	float flDot = DotProduct( los, facingDir );
#if 0
	if (CapabilitiesGet() & bits_CAP_AIM_GUN)
	{
		// FIXME: query current animation for ranges
		return ( flDot > DOT_30DEGREE );
	}
#endif
	if ( flDot > 0.994 )//!!!BUGBUG - magic number same as FacingIdeal(), what is this?
		return true;

	return false;
}

//-----------------------------------------------------------------------------
int CHOEBaseZombie::MeleeAttack1Conditions( float flDot, float flDist )
{
	// BITE

	// Cannot bite if I have no head
	if ( !HasAHead() )
		return COND_NONE;

	if ( flDist > 50 )
		return COND_NONE; // COND_TOO_FAR_TO_ATTACK;

	if ( flDot < 0.7 )
		return COND_NONE; // COND_NOT_FACING_ATTACK;

	if ( GetEnemy() == NULL )
		return COND_NONE;

	if ( FClassnameIs( GetEnemy(), "npc_snark" ) )
		return COND_NONE;

	Vector origin;
	QAngle angles;
	GetAttachment( LookupAttachment( "head" ), origin, angles );

	Vector vecMins = Vector( -12, -12, -12 );
	Vector vecMaxs = Vector( 12, 12, 12 );

	Vector forward;
	GetVectors( &forward, NULL, NULL );

	trace_t	tr;
	CTraceFilterNav traceFilter( this, false, this, COLLISION_GROUP_NONE );
	AI_TraceHull( origin, origin + forward * 50, vecMins, vecMaxs, MASK_NPCSOLID, &traceFilter, &tr );

	if ( tr.fraction == 1.0 || !tr.m_pEnt || tr.m_pEnt != GetEnemy() )
	{
		return COND_NONE;
	}

	return COND_CAN_MELEE_ATTACK1;
}

//-----------------------------------------------------------------------------
int CHOEBaseZombie::MeleeAttack2Conditions( float flDot, float flDist )
{
	// KICK

	// Don't repeat kicks too often, unless we have an explosive weapon (i.e. might not have a choice)
	if ( HasCondition( COND_CAN_RANGE_ATTACK1 ) && ( !m_flLastKick || m_flLastKick > gpGlobals->curtime - 10.0 ) )
		return COND_NONE;

	if ( flDist > 50 )
		return COND_NONE; // COND_TOO_FAR_TO_ATTACK;

	if ( flDot < 0.7 )
		return COND_NONE; // COND_NOT_FACING_ATTACK;

	if ( GetEnemy() == NULL )
		return COND_NONE;

	if ( FClassnameIs( GetEnemy(), "npc_snark" ) )
		return COND_NONE;

	// Trace hull forward from hips to just below the feet
	float flWidth = GetHullWidth();
	float flHeight = GetHullHeight();

	Vector center = GetAbsOrigin();
	center.z += flHeight / 4;

	Vector vecMins = Vector( -flWidth/2, -flWidth/2, -flHeight/4 - 6 );
	vecMins.z = -flHeight/4 + 1; // can't be below ground
	Vector vecMaxs = Vector( flWidth/2, flWidth/2, flHeight/4 );

	Vector forward;
	GetVectors( &forward, NULL, NULL );

	trace_t	tr;
	CTraceFilterNav traceFilter( this, false, this, COLLISION_GROUP_NONE );
	AI_TraceHull( center, center + forward * 50, vecMins, vecMaxs, MASK_NPCSOLID, &traceFilter, &tr );

//	NDebugOverlay::Box(center, vecMins, vecMaxs, 0, 255, 255, 0, 0);
//	NDebugOverlay::Box(center + forward * HUMAN_KICK_RANGE, vecMins, vecMaxs, 0, 0, 255, 0, 0);

	if ( tr.fraction == 1.0 || !tr.m_pEnt || tr.m_pEnt != GetEnemy() )
	{
		return COND_NONE;
	}

	return COND_CAN_MELEE_ATTACK2;
}

//-----------------------------------------------------------------------------
void CHOEBaseZombie::SpeedBoost( void )
{
	m_flBoostSpeed = 0;

	if ( !AI_IsSinglePlayer() )
		return;

	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();

	if ( GetFollowBehavior().GetFollowTarget() && 
		 ( GetFollowBehavior().GetFollowTarget()->IsPlayer() || GetCommandGoal() != vec3_invalid ) && 
		 GetFollowBehavior().IsMovingToFollowTarget() && 
		 GetFollowBehavior().GetGoalRange() > 0.1 &&
		 BaseClass::GetIdealSpeed() > 0.1 )
	{
		Vector vPlayerToFollower = GetAbsOrigin() - pPlayer->GetAbsOrigin();
		float dist = vPlayerToFollower.NormalizeInPlace();

		bool bDoSpeedBoost = false;
		if ( !HasCondition( COND_IN_PVS ) )
			bDoSpeedBoost = true;
		else if ( GetFollowBehavior().GetFollowTarget()->IsPlayer() )
		{
			if ( dist > GetFollowBehavior().GetGoalRange() * 2 )
			{
				float dot = vPlayerToFollower.Dot( pPlayer->EyeDirection3D() );
				if ( dot < 0 )
				{
					bDoSpeedBoost = true;
				}
			}
		}

		if ( bDoSpeedBoost )
		{
			float lag = dist / GetFollowBehavior().GetGoalRange();

			float mult;
			
			if ( lag > 10.0 )
				mult = 2.0;
			else if ( lag > 5.0 )
				mult = 1.5;
			else if ( lag > 3.0 )
				mult = 1.25;
			else
				mult = 1.1;

			m_flBoostSpeed = pPlayer->GetSmoothedVelocity().Length();

			if ( m_flBoostSpeed < BaseClass::GetIdealSpeed() )
				m_flBoostSpeed = BaseClass::GetIdealSpeed();

			m_flBoostSpeed *= mult;
		}
	}
}

//---------------------------------------------------------
void CHOEBaseZombie::RemoveIgnoredConditions( void )
{
	BaseClass::RemoveIgnoredConditions();

	if ( m_flHungryTime > gpGlobals->curtime )
		 ClearCondition( COND_SMELL );

	if ( IsCurSchedule( SCHED_CHASE_ENEMY ) &&
		m_flChaseEnemyTime > gpGlobals->curtime - 3 &&
		GetEnemy() && GetEnemyLKP().DistToSqr( GetAbsOrigin() ) > Square(256) )
	{
		ClearCondition( COND_CAN_RANGE_ATTACK1 );
	}
}

#ifdef PLAYER_SQUAD_STUFF
//-----------------------------------------------------------------------------
bool CHOEBaseZombie::ShouldAlwaysThink() 
{ 
	return ( BaseClass::ShouldAlwaysThink() || IsInPlayerSquad() ||
		( GetFollowBehavior().GetFollowTarget() && GetFollowBehavior().GetFollowTarget()->IsPlayer() ) ); 
}

//-----------------------------------------------------------------------------
void CHOEBaseZombie::PrescheduleThink( void )
{
	BaseClass::PrescheduleThink();
	UpdateFollowCommandPoint();
}
#endif

//-----------------------------------------------------------------------------
int CHOEBaseZombie::SelectSchedule( void )
{
	// When a schedule ends, conditions are set back to what GatherConditions
	// reported before selecting a new schedule.  See CAI_BaseNPC::MaintainSchedule.
	// Anything RemoveIgnoredConditions did is lost.
	if ( m_flHungryTime > gpGlobals->curtime )
		 ClearCondition( COND_SMELL );

	m_bMovingAwayFromPlayer = false;

	if ( m_flChainsawedTime > gpGlobals->curtime - 0.25f )
		return SCHED_ZOMBIE_CHAINSAWED;

	if ( CheckFollowPlayer() )
	{
		int schedule = SelectSchedulePlayerPush();
		if ( schedule != SCHED_NONE )
		{
			if ( GetFollowBehavior().IsRunning() )
				KeepRunningBehavior();
			return schedule;
		}

		if ( ShouldDeferToFollowBehavior() )
		{
			DeferSchedulingToBehavior( &(GetFollowBehavior()) );
		}
		else if ( !BehaviorSelectSchedule() )
		{
		}
	}

	int sched = SelectSchedulePostBehavior();
	if ( sched != SCHED_NONE )
		return sched;

	return BaseClass::SelectSchedule();
}

//-----------------------------------------------------------------------------
int CHOEBaseZombie::SelectSchedulePlayerPush( void )
{
	if ( HasCondition( COND_PLAYER_PUSHING ) && !IsInAScript() && !IgnorePlayerPushing() )
	{
		// Ignore move away before gordon becomes the man
		if ( 1 /* GlobalEntity_GetState("gordon_precriminal") != GLOBAL_ON */ )
		{
			m_bMovingAwayFromPlayer = true;
			return SCHED_MOVE_AWAY;
		}
	}

	return SCHED_NONE;
}

//-----------------------------------------------------------------------------
int CHOEBaseZombie::SelectSchedulePostBehavior( void )
{
#if 1
	if ( GetState() == NPC_STATE_COMBAT &&
		HasCondition( COND_CAN_RANGE_ATTACK1 ) &&
		m_flChaseEnemyTime < gpGlobals->curtime - 10 &&
		GetEnemy()->GetAbsOrigin().DistToSqr( GetAbsOrigin() ) > Square(256) )
	{
		m_flChaseEnemyTime = gpGlobals->curtime;
		return SCHED_CHASE_ENEMY;
	}
#endif
	if ( GetState() == NPC_STATE_COMBAT && HasCondition( COND_SMELL ) && HasCondition( COND_ENEMY_OCCLUDED ) )
		return SCHED_ZOMBIE_EAT;

//	if ( GetState() == NPC_STATE_ALERT && HasCondition( COND_ENEMY_DEAD ) )
//		return SCHED_SPX_VICTORY_DANCE;

	if ( GetState() == NPC_STATE_ALERT && HasCondition( COND_SMELL ) )
		return SCHED_ZOMBIE_EAT;

	if ( GetState() == NPC_STATE_IDLE && HasCondition( COND_SMELL ) )
		return SCHED_ZOMBIE_EAT;

	return SCHED_NONE;
}

//-----------------------------------------------------------------------------
bool CHOEBaseZombie::ShouldDeferToFollowBehavior( void )
{
	if ( !GetFollowBehavior().CanSelectSchedule() || !GetFollowBehavior().FarFromFollowTarget() )
		return false;
#if 0
	if ( m_StandoffBehavior.CanSelectSchedule() && !m_StandoffBehavior.IsBehindBattleLines( GetFollowBehavior().GetFollowTarget()->GetAbsOrigin() ) )
		return false;

	if ( HasCondition(COND_BETTER_WEAPON_AVAILABLE) && !GetActiveWeapon() )
	{
		// Unarmed allies should arm themselves as soon as the opportunity presents itself.
		return false;
	}

	// I don't know why I have to write this @#*$&# piece of code, since I've already placed Assault Behavior 
	// ABOVE the Follow behavior in precedence. (sjb)
	if( m_AssaultBehavior.CanSelectSchedule() && hl2_episodic.GetBool() )
	{
		return false;
	}
#endif
	return true;
}

//-----------------------------------------------------------------------------
void CHOEBaseZombie::BuildScheduleTestBits( void )
{
	BaseClass::BuildScheduleTestBits();

	// Ignore damage if we're attacking, otherwise the animation stops in progress.
	if ( GetActivity() == ACT_RANGE_ATTACK1 ||
		 GetActivity() == ACT_MELEE_ATTACK1 ||
		 GetActivity() == ACT_MELEE_ATTACK2 )
	{
		ClearCustomInterruptCondition( COND_LIGHT_DAMAGE );
//		ClearCustomInterruptCondition( COND_HEAVY_DAMAGE );
		ClearCustomInterruptCondition( COND_HEAR_DANGER );
	}

	if ( CheckFollowPlayer() )
	{
		if ( IsCurSchedule(SCHED_RANGE_ATTACK1) )
		{
			SetCustomInterruptCondition( COND_PLAYER_PUSHING );
		}

		if ( ( ConditionInterruptsCurSchedule( COND_GIVE_WAY ) || 
			   IsCurSchedule(SCHED_HIDE_AND_RELOAD ) || 
			   IsCurSchedule(SCHED_RELOAD ) || 
			   IsCurSchedule(SCHED_STANDOFF ) || 
			   IsCurSchedule(SCHED_TAKE_COVER_FROM_ENEMY ) || 
			   IsCurSchedule(SCHED_COMBAT_FACE ) || 
			   IsCurSchedule(SCHED_ALERT_FACE )  ||
			   IsCurSchedule(SCHED_COMBAT_STAND ) || 
			   IsCurSchedule(SCHED_ALERT_FACE_BESTSOUND) ||
			   IsCurSchedule(SCHED_ALERT_STAND) ) )
		{
			SetCustomInterruptCondition( COND_HEAR_MOVE_AWAY );
			SetCustomInterruptCondition( COND_PLAYER_PUSHING );
	//		SetCustomInterruptCondition( COND_PC_HURTBYFIRE );
		}
	}
}

//-----------------------------------------------------------------------------
float CHOEBaseZombie::GetIdealSpeed() const
{
	float baseSpeed = BaseClass::GetIdealSpeed();

	if ( baseSpeed < m_flBoostSpeed )
		return m_flBoostSpeed;

	return baseSpeed;
}

//-----------------------------------------------------------------------------
float CHOEBaseZombie::GetIdealAccel() const
{
	float multiplier = 1.0;
	if ( AI_IsSinglePlayer() )
	{
		if ( m_bMovingAwayFromPlayer && (UTIL_PlayerByIndex(1)->GetAbsOrigin() - GetAbsOrigin()).Length2DSqr() < Square(3.0*12.0) )
			multiplier = 2.0;
	}
	return BaseClass::GetIdealAccel() * multiplier;
}

//-----------------------------------------------------------------------------
bool CHOEBaseZombie::OnObstructionPreSteer( AILocalMoveGoal_t *pMoveGoal, float distClear, AIMoveResult_t *pResult )
{
	if ( pMoveGoal->directTrace.flTotalDist - pMoveGoal->directTrace.flDistObstructed < GetHullWidth() * 1.5 )
	{
		CAI_BaseNPC *pBlocker = pMoveGoal->directTrace.pObstruction->MyNPCPointer();
		if ( pBlocker && pBlocker->IsPlayerAlly() && !pBlocker->IsMoving() && !pBlocker->IsInAScript() &&
			 ( IsCurSchedule( SCHED_NEW_WEAPON ) || 
			   IsCurSchedule( SCHED_GET_HEALTHKIT ) || 
			   pBlocker->IsCurSchedule( SCHED_FAIL ) || 
			   ( IsInPlayerSquad() && !pBlocker->IsInPlayerSquad() ) ||
			   ClassifyPlayerAllyVital() ||
			   IsInAScript() ) )

		{
			if ( pBlocker->ConditionInterruptsCurSchedule( COND_GIVE_WAY ) || 
				 pBlocker->ConditionInterruptsCurSchedule( COND_PLAYER_PUSHING ) )
			{
				// HACKHACK
				pBlocker->GetMotor()->SetIdealYawToTarget( WorldSpaceCenter() );
				pBlocker->SetSchedule( SCHED_MOVE_AWAY );
			}

		}
	}

	if ( pMoveGoal->directTrace.pObstruction )
	{
	}

	return BaseClass::OnObstructionPreSteer( pMoveGoal, distClear, pResult );
}

//-----------------------------------------------------------------------------
void CHOEBaseZombie::StartTask( const Task_t *pTask )
{
	switch( pTask->iTask )
	{
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
	case TASK_MOVE_AWAY_PATH:
		{
/*			if ( m_bMovingAwayFromPlayer && OkToShout() )
				Speak( "OUTWAY" );*/

			BaseClass::StartTask( pTask );
		}
		break;
	case TASK_ZOMBIE_DONT_EAT: // Don't eat - set me to not hungry for a while
		m_flHungryTime = gpGlobals->curtime + pTask->flTaskData;
		TaskComplete();
		break;
	case TASK_ZOMBIE_EAT: // Eat - set me to not hungry for a while AND increase my health
		m_flHungryTime = gpGlobals->curtime + pTask->flTaskData;
#if 1
		TakeHealth( GetMaxHealth() / 20, DMG_GENERIC );
#else
		TakeHealth( pTask->flTaskData, DMG_GENERIC );
#endif
		TaskComplete();
		break;
	case TASK_ZOMBIE_SOUND_EAT:
		if ( pTask->flTaskData == 0 || random->RandomFloat(0, 1) <= pTask->flTaskData )
			EmitSound( "NPC_SuperZombie.Eat" );
		TaskComplete();
		break;
	default:
		BaseClass::StartTask( pTask );
		break;
	}
}

//-----------------------------------------------------------------------------
bool CHOEBaseZombie::IgnorePlayerPushing( void )
{
#if 0
	if ( hl2_episodic.GetBool() )
	{
		// Ignore player pushes if we're leading him
		if ( m_LeadBehavior.IsRunning() && m_LeadBehavior.HasGoal() )
			return true;
		if ( m_AssaultBehavior.IsRunning() && m_AssaultBehavior.OnStrictAssault() )
			return true;
	}
#endif
	return false;
}

//-----------------------------------------------------------------------------
void CHOEBaseZombie::UseFunc( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( !CheckFollowPlayer() )
	{
		return;
	}

	// Can't +USE NPCs running scripts
	if ( m_NPCState == NPC_STATE_SCRIPT )
		return;

	// Can't +USE NPCs running scripts
	if ( IsInAScript() )
		return;

	// Check that caller is the player
	if ( !pCaller->IsPlayer() )
		return;

	// FIXME: don't follow if SF_MONSTER_PREDISASTER ???

	m_OnPlayerUse.FireOutput( pActivator, pCaller );

	// Toggle follow
	if (m_FollowBehavior.GetFollowTarget() != NULL)
	{
#ifdef PLAYER_SQUAD_STUFF
		ClearFollowTarget();
		ClearCommandGoal();

		if ( IsInPlayerSquad() )
			RemoveFromSquad(); // FIXME: add to original squad if any
#else
		m_FollowBehavior.SetFollowTarget( NULL );
#endif
		//Speak( "UNUSE" );
	}
	else
	{
#ifdef PLAYER_SQUAD_STUFF
		// It is possible the player drags an NPC in a squad to another map where another squad with the
		// same name exists. In that case the dragged-from-another-map NPC should not be part of the
		// same-name squad.
		if ( !IsInPlayerSquad() )
			AddToSquad( GetPlayerSquadName() );

		// Follow what everyone else in the squad is following
		CAI_BaseNPC *pLeader = NULL;
		AISquadIter_t iter;
		for ( CAI_BaseNPC *pAllyNpc = m_pSquad->GetFirstMember(&iter); pAllyNpc; pAllyNpc = m_pSquad->GetNextMember(&iter) )
		{
			if ( pAllyNpc != this && pAllyNpc->IsCommandable() )
			{
				pLeader = pAllyNpc;
				break;
			}
		}

		if ( pLeader /*&& pLeader != this*/ )
		{
			const Vector &commandGoal = pLeader->GetCommandGoal();
			if ( commandGoal != vec3_invalid )
			{
				SetCommandGoal( commandGoal );
				SetCondition( COND_RECEIVED_ORDERS ); 
				OnMoveOrder();
			}
			else
			{
				CAI_FollowBehavior *pLeaderFollowBehavior;
				if ( pLeader->GetBehavior( &pLeaderFollowBehavior ) )
				{
					m_FollowBehavior.SetFollowTarget( pLeaderFollowBehavior->GetFollowTarget() );
					m_FollowBehavior.SetParameters( m_FollowBehavior.GetFormation() );
				}

			}
		}
		else
		{
			m_FollowBehavior.SetFollowTarget( UTIL_GetLocalPlayer() );
			m_FollowBehavior.SetParameters( FOLLOW_PLAYER_FORMATION );
		}
#else
		m_FollowBehavior.SetFollowTarget( UTIL_GetLocalPlayer() );
#endif
		//Speak( "USE" );
	}
	IdleSound();
}

//------------------------------------------------------------------------------
void CHOEBaseZombie::Touch( CBaseEntity *pOther )
{
	BaseClass::Touch( pOther );

	if ( CheckFollowPlayer() )
	{
		// Did the player touch me?
		if ( pOther->IsPlayer() || ( pOther->VPhysicsGetObject() && (pOther->VPhysicsGetObject()->GetGameFlags() & FVPHYSICS_PLAYER_HELD ) ) )
		{
			// Ignore if pissed at player
			if ( m_afMemory & bits_MEMORY_PROVOKED )
				return;
				
			TestPlayerPushing( ( pOther->IsPlayer() ) ? pOther : AI_GetSinglePlayer() );
		}
	}
}

//-----------------------------------------------------------------------------
void CHOEBaseZombie::PredictPlayerPush( void )
{
	CBasePlayer *pPlayer = AI_GetSinglePlayer();
	if ( pPlayer && pPlayer->GetSmoothedVelocity().LengthSqr() >= Square(140))
	{
		Vector predictedPosition = pPlayer->WorldSpaceCenter() + pPlayer->GetSmoothedVelocity() * .4;
		Vector delta = WorldSpaceCenter() - predictedPosition;
		if ( delta.z < GetHullHeight() * .5 && delta.Length2DSqr() < Square(GetHullWidth() * 1.414)  )
			TestPlayerPushing( pPlayer );
	}
}

//-----------------------------------------------------------------------------
void CHOEBaseZombie::OnStateChange( NPC_STATE oldState, NPC_STATE newState )
{
	BaseClass::OnStateChange( oldState, newState ); // alyx doesn't call this???

	// Stop following the player if leaving a scripted sequence so we don't move away from the script position.
	if ( oldState == NPC_STATE_SCRIPT )
	{
		// Can't clear this if newState == NPC_STATE_SCRIPT because that will cancel the script.
#ifdef PLAYER_SQUAD_STUFF
		ClearFollowTarget();
		ClearCommandGoal();

		// It should really be up to the mapmaker whether someone (usually Barney) stops following
		// the player after a scripted_sequence ends
		if ( IsInPlayerSquad() )
			RemoveFromSquad(); // FIXME: add to original squad if any
#else
		m_FollowBehavior.SetFollowTarget( NULL );
#endif
	}
}

//-----------------------------------------------------------------------------
int CHOEBaseZombie::OnTakeDamage_Alive( const CTakeDamageInfo &info )
{
	int ret = BaseClass::OnTakeDamage_Alive( info );

	if ( info.GetAttacker() && FClassnameIs( info.GetAttacker(), "npc_huey" ) )
		m_flAttackedByHueyTime = gpGlobals->curtime;

	return ret;
}

//-----------------------------------------------------------------------------
void CHOEBaseZombie::TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr )
{
	// If hit by blow from machete or chainsaw to head, chop off head
	if( ( ptr->hitgroup == HITGROUP_HEAD ) &&
		( info.GetDamageType() & DMG_SLASH ) &&
		HasAHead() &&
		!IsInAScript() &&
		( GetState() != NPC_STATE_PRONE ) &&
		( GetState() != NPC_STATE_NONE ) )
	{
		Behead();

		CTakeDamageInfo newInfo = info;
		newInfo.SetDamage( GetHealth() );
		BaseClass::TraceAttack( newInfo, vecDir, ptr, 0 );
		return;
	}

	BaseClass::TraceAttack( info, vecDir, ptr, 0 );
}

//-----------------------------------------------------------------------------
bool CHOEBaseZombie::GetHeadPosition( Vector &origin, QAngle &angles )
{
	int attachment = LookupAttachment( "head" );
	if ( attachment != -1 )
	{
		GetAttachment( attachment, origin, angles );
		origin.z += 12.0; // jump up to avoid spawning inside our body
		// Also note the origin of the head model is at its center, which
		// doesn't agree with the attachment point which is at the base of the neck
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
void CHOEBaseZombie::Behead( void )
{
	CGib *pGib = CREATE_ENTITY( CGib, "gib" );
	pGib->Spawn( GetHeadModelName() );
	pGib->m_nBody = m_nHeadNum;
	pGib->InitGib( this, 100.0, 150.0 );
	pGib->m_lifeTime = 1.0f; // no effect with physics model?

	Vector origin;
	QAngle angles;
	if ( GetHeadPosition( origin, angles ) )
	{
		NDebugOverlay::Cross3D(origin,Vector(-5,-5,-5),Vector(5,5,5),255,0,0,true,2.0);

		pGib->SetAbsOrigin( origin );
		pGib->SetAbsAngles( angles );
	}

	m_nHeadNum = GetNumHeads();
	SetBodygroup( GetHeadGroupNum(), m_nHeadNum );

	SentenceStop(); // Don't talk if you have no head
}

#ifdef PLAYER_SQUAD_STUFF

//-----------------------------------------------------------------------------

#define COMMAND_POINT_CLASSNAME "info_target_command_point"

//-----------------------------------------------------------------------------
bool CHOEBaseZombie::IsCommandable( void ) 
{
	return IsInPlayerSquad();
}

//-----------------------------------------------------------------------------
bool CHOEBaseZombie::IsFollowingCommandPoint( void )
{
	CBaseEntity *pFollowTarget = m_FollowBehavior.GetFollowTarget();
	if ( pFollowTarget )
		return FClassnameIs( pFollowTarget, COMMAND_POINT_CLASSNAME );
	return false;
}

//-----------------------------------------------------------------------------
bool CHOEBaseZombie::HaveCommandGoal( void ) const			
{	
	if (GetCommandGoal() != vec3_invalid)
		return true;
	return false;
}

//-----------------------------------------------------------------------------
struct SquadMemberInfo_t
{
	CHOEBaseZombie *pMember;
	bool				bSeesPlayer;
	float				distSq;
};

static int __cdecl SquadSortFunc( const SquadMemberInfo_t *pLeft, const SquadMemberInfo_t *pRight )
{
	if ( pLeft->bSeesPlayer && !pRight->bSeesPlayer )
	{
		return -1;
	}

	if ( !pLeft->bSeesPlayer && pRight->bSeesPlayer )
	{
		return 1;
	}

	return ( pLeft->distSq - pRight->distSq );
}

CAI_BaseNPC *CHOEBaseZombie::GetSquadCommandRepresentative( void )
{
	if ( !AI_IsSinglePlayer() )
		return NULL;

	if ( IsInPlayerSquad() )
	{
		static float lastTime;
		static AIHANDLE hCurrent;

		if ( gpGlobals->curtime - lastTime > 2.0 || !hCurrent || !hCurrent->IsInPlayerSquad() ) // hCurrent will be NULL after level change
		{
			lastTime = gpGlobals->curtime;
			hCurrent = NULL;

			CUtlVectorFixed<SquadMemberInfo_t, MAX_SQUAD_MEMBERS> candidates;
			CBasePlayer *pPlayer = UTIL_GetLocalPlayer();

			if ( pPlayer )
			{
				AISquadIter_t iter;
				for ( CAI_BaseNPC *pAllyNpc = m_pSquad->GetFirstMember(&iter); pAllyNpc; pAllyNpc = m_pSquad->GetNextMember(&iter) )
				{
					if ( pAllyNpc->IsCommandable() && dynamic_cast<CHOEBaseZombie *>(pAllyNpc) )
					{
						int i = candidates.AddToTail();
						candidates[i].pMember = (CHOEBaseZombie *)(pAllyNpc);
						candidates[i].bSeesPlayer = pAllyNpc->HasCondition( COND_SEE_PLAYER );
						candidates[i].distSq = ( pAllyNpc->GetAbsOrigin() - pPlayer->GetAbsOrigin() ).LengthSqr();
					}
				}

				if ( candidates.Count() > 0 )
				{
					candidates.Sort( SquadSortFunc );
					hCurrent = candidates[0].pMember;
				}
			}
		}

		if ( hCurrent != NULL )
		{
			Assert( dynamic_cast<CHOEBaseZombie *>(hCurrent.Get()) && hCurrent->IsInPlayerSquad() );
			return hCurrent;
		}
	}
	return NULL;
}

//-----------------------------------------------------------------------------
void CHOEBaseZombie::ClearFollowTarget( void )
{
	m_FollowBehavior.SetFollowTarget( NULL );
	m_FollowBehavior.SetParameters( AIF_SIMPLE );
}

//-----------------------------------------------------------------------------
void CHOEBaseZombie::UpdateFollowCommandPoint( void )
{
	if ( !AI_IsSinglePlayer() )
		return;

	if ( IsInPlayerSquad() )
	{
		if ( HaveCommandGoal() )
		{
			CBaseEntity *pFollowTarget = m_FollowBehavior.GetFollowTarget();
			CBaseEntity *pCommandPoint = gEntList.FindEntityByClassname( NULL, COMMAND_POINT_CLASSNAME );
			
			if( !pCommandPoint )
			{
				DevMsg("**\nVERY BAD THING\nCommand point vanished! Creating a new one\n**\n");
				pCommandPoint = CreateEntityByName( COMMAND_POINT_CLASSNAME );
			}

			if ( pFollowTarget != pCommandPoint )
			{
				pFollowTarget = pCommandPoint;
				m_FollowBehavior.SetFollowTarget( pFollowTarget );
				m_FollowBehavior.SetParameters( FOLLOW_POINT_FORMATION );
			}
			
			if ( ( pCommandPoint->GetAbsOrigin() - GetCommandGoal() ).LengthSqr() > 0.01 )
			{
				UTIL_SetOrigin( pCommandPoint, GetCommandGoal(), false );
			}
		}
		else
		{
			if ( IsFollowingCommandPoint() )
				ClearFollowTarget();
			if ( m_FollowBehavior.GetFollowTarget() != UTIL_GetLocalPlayer() )
			{
				DevMsg( "Expected to be following player, but not\n" );
				m_FollowBehavior.SetFollowTarget( UTIL_GetLocalPlayer() );
				m_FollowBehavior.SetParameters( FOLLOW_PLAYER_FORMATION );
			}
		}
	}
	else if ( IsFollowingCommandPoint() )
		ClearFollowTarget();
}

//-----------------------------------------------------------------------------
// Purpose: return TRUE if the commander mode should try to give this order
//			to more people. return FALSE otherwise. For instance, we don't
//			try to send all 3 selectedcitizens to pick up the same gun.
//-----------------------------------------------------------------------------
bool CHOEBaseZombie::TargetOrder( CBaseEntity *pTarget, CAI_BaseNPC **Allies, int numAllies )
{
	if ( pTarget->IsPlayer() )
	{
		// I'm the target! Toggle follow!
		if ( m_FollowBehavior.GetFollowTarget() != pTarget )
		{
			ClearFollowTarget();
			ClearCommandGoal();

			// Turn follow on!
//			m_AssaultBehavior.Disable();
			m_FollowBehavior.SetFollowTarget( pTarget );
			m_FollowBehavior.SetParameters( FOLLOW_PLAYER_FORMATION );			
//			SpeakCommandResponse( TLK_STARTFOLLOW );

//			m_OnFollowOrder.FireOutput( this, this );
		}
		else if ( m_FollowBehavior.GetFollowTarget() == pTarget )
		{
			// Stop following.
			m_FollowBehavior.SetFollowTarget( NULL );
//			SpeakCommandResponse( TLK_STOPFOLLOW );
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Turn off following before processing a move order.
//-----------------------------------------------------------------------------
void CHOEBaseZombie::MoveOrder( const Vector &vecDest, CAI_BaseNPC **Allies, int numAllies )
{
	if ( !AI_IsSinglePlayer() )
		return;
#if 0
	if( hl2_episodic.GetBool() && m_iszDenyCommandConcept != NULL_STRING )
	{
		SpeakCommandResponse( STRING(m_iszDenyCommandConcept) );
		return;
	}

	CHL2_Player *pPlayer = (CHL2_Player *)UTIL_GetLocalPlayer();

	m_AutoSummonTimer.Set( player_squad_autosummon_time.GetFloat() );
	m_vAutoSummonAnchor = pPlayer->GetAbsOrigin();

	if( m_StandoffBehavior.IsRunning() )
	{
		m_StandoffBehavior.SetStandoffGoalPosition( vecDest );
	}

	// If in assault, cancel and move.
	if( m_AssaultBehavior.HasHitRallyPoint() && !m_AssaultBehavior.HasHitAssaultPoint() )
	{
		m_AssaultBehavior.Disable();
		ClearSchedule();
	}
	bool spoke = false;

	CAI_BaseNPC *pClosest = NULL;
	float closestDistSq = FLT_MAX;

	for( int i = 0 ; i < numAllies ; i++ )
	{
		if( Allies[i]->IsInPlayerSquad() )
		{
			Assert( Allies[i]->IsCommandable() );
			float distSq = ( pPlayer->GetAbsOrigin() - Allies[i]->GetAbsOrigin() ).LengthSqr();
			if( distSq < closestDistSq )
			{
				pClosest = Allies[i];
				closestDistSq = distSq;
			}
		}
	}
#endif

	if( m_FollowBehavior.GetFollowTarget() && !IsFollowingCommandPoint() )
	{
		ClearFollowTarget();
#if 0
		if ( ( pPlayer->GetAbsOrigin() - GetAbsOrigin() ).LengthSqr() < Square( 180 ) &&
			 ( ( vecDest - pPlayer->GetAbsOrigin() ).LengthSqr() < Square( 120 ) || 
			   ( vecDest - GetAbsOrigin() ).LengthSqr() < Square( 120 ) ) )
		{
			if ( pClosest == this )
				SpeakIfAllowed( TLK_STOPFOLLOW );
			spoke = true;
		}
#endif
	}
#if 0
	if ( !spoke && pClosest == this )
	{
		float destDistToPlayer = ( vecDest - pPlayer->GetAbsOrigin() ).Length();
		float destDistToClosest = ( vecDest - GetAbsOrigin() ).Length();
		CFmtStr modifiers( "commandpoint_dist_to_player:%.0f,"
						   "commandpoint_dist_to_npc:%.0f",
						   destDistToPlayer,
						   destDistToClosest );

		SpeakCommandResponse( TLK_COMMANDED, modifiers );
	}

	m_OnStationOrder.FireOutput( this, this );
#endif
	BaseClass::MoveOrder( vecDest, Allies, numAllies );
}

//-----------------------------------------------------------------------------
void CHOEBaseZombie::OnMoveToCommandGoalFailed( void )
{
	// Clear the goal.
	ClearCommandGoal();

	// Announce failure.
//	SpeakCommandResponse( TLK_COMMAND_FAILED );
}

#endif // PLAYER_SQUAD_STUFF

AI_BEGIN_CUSTOM_NPC( hoe_base_zombie, CHOEBaseZombie )

	DECLARE_TASK( TASK_ZOMBIE_DONT_EAT )
	DECLARE_TASK( TASK_ZOMBIE_EAT )
	DECLARE_TASK( TASK_ZOMBIE_SOUND_EAT )

	DECLARE_ACTIVITY( ACT_ZOMBIE_CHAINSAWED )
	DECLARE_ACTIVITY( ACT_ZOMBIE_EAT )

	DECLARE_ANIMEVENT( AE_ZOMBIE_BITE )
	DECLARE_ANIMEVENT( AE_ZOMBIE_KICK )

	DEFINE_SCHEDULE
	(
		SCHED_ZOMBIE_EAT,

		"	Tasks"
		"		TASK_STOP_MOVING				0"
		"		TASK_ZOMBIE_DONT_EAT			10"
		"		TASK_STORE_LASTPOSITION			0"
		"		TASK_GET_PATH_TO_BESTSCENT		0"
		"		TASK_FACE_IDEAL					0"
		"		TASK_WALK_PATH_WITHIN_DIST		64"
		"		TASK_STOP_MOVING				0"
		"		TASK_FACE_SAVEPOSITION			0" // see TASK_GET_PATH_TO_BESTSCENT
		"		TASK_ZOMBIE_SOUND_EAT			0"
		"		TASK_PLAY_SEQUENCE				ACTIVITY:ACT_ZOMBIE_EAT"
		"		TASK_ZOMBIE_EAT					20"
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
		SCHED_ZOMBIE_CHAINSAWED,

		"	Tasks"
		"		TASK_STOP_MOVING		0"
		"		TASK_PLAY_SEQUENCE		ACTIVITY:ACT_ZOMBIE_CHAINSAWED"
		""
		"	Interrupts"
	)

	DEFINE_SCHEDULE
	(
		// base class interrupts when danger is heard
		// FIXME: just ignore SOUND_DANGER?
		SCHED_ZOMBIE_RELOAD,

		"	Tasks"
		"		TASK_STOP_MOVING		0"
		"		TASK_RELOAD				0"
		""
		"	Interrupts"
	)

AI_END_CUSTOM_NPC()


//-----------------------------------------------------------------------------
int CZombieFollowBehavior::FollowCallBaseSelectSchedule( void )
{
	extern bool g_bSelectScheduleCheck;
	// Detect recursion due to SelectSchedulePostBehavior calling BaseClass::SelectSchedule.
	if ( g_bSelectScheduleCheck )
	{
		Assert( 0 );
		DevWarning( "CZombieFollowBehavior::FollowCallBaseSelectSchedule RECURSION (%s)\n", GetOuter()->GetDebugName() );
		return SCHED_NONE;
	}

	int sched = SCHED_NONE;
	g_bSelectScheduleCheck = true;

	sched = GetOuter()->SelectSchedulePostBehavior();

	g_bSelectScheduleCheck = false;
	if ( sched != SCHED_NONE )
		return sched;
	return BaseClass::FollowCallBaseSelectSchedule();
}

//-----------------------------------------------------------------------------
int CZombieFollowBehavior::SelectSchedule( void )
{
	// The base class returns SCHED_RANGE_ATTACK1 but doesn't check the LOS
	// or AMMO conditions.  Just pick any combat schedule here.
	if ( HasCondition( COND_CAN_RANGE_ATTACK1 ) )
		return FollowCallBaseSelectSchedule();

	return BaseClass::SelectSchedule();
}

//-----------------------------------------------------------------------------
int CZombieFollowBehavior::TranslateScheduleHack( int scheduleType )
{
	switch( scheduleType )
	{
		case SCHED_FOLLOWER_IDLE_STAND:
			// If we have an enemy, at least face them!
			if ( GetEnemy() )
				return SCHED_FOLLOWER_COMBAT_FACE;
			
			break;

		case SCHED_IDLE_STAND:
		{
			if ( ShouldMoveToFollowTarget() && !IsFollowGoalInRange( GetGoalRange(), GetGoalZRange(), GetGoalFlags() ) )
			{
				return SCHED_MOVE_TO_FACE_FOLLOW_TARGET;			
			}
			if ( HasFollowPoint() && !ShouldIgnoreFollowPointFacing() )
				return SCHED_FOLLOWER_GO_TO_WAIT_POINT;
			
			// If we have an enemy, at least face them!
			if ( GetEnemy() )
				return SCHED_FOLLOWER_COMBAT_FACE;

			return SCHED_FOLLOWER_IDLE_STAND;
		}

		case SCHED_COMBAT_STAND:
		case SCHED_ALERT_STAND:
		{
			if ( ShouldMoveToFollowTarget() && !IsFollowGoalInRange( GetGoalRange(), GetGoalZRange(), GetGoalFlags() ) )
			{
				return SCHED_MOVE_TO_FACE_FOLLOW_TARGET;			
			}
			break;
		}

		case SCHED_TARGET_FACE:
		{
			if ( ( ShouldMoveToFollowTarget() || m_bFirstFacing ) && !IsFollowGoalInRange( GetGoalRange(), GetGoalZRange(), GetGoalFlags() ) )
			{
				return SCHED_MOVE_TO_FACE_FOLLOW_TARGET;			
			}
			if ( HasFollowPoint() && !ShouldIgnoreFollowPointFacing() )
				return SCHED_FOLLOWER_GO_TO_WAIT_POINT;
			if ( !m_TargetMonitor.IsMarkSet() )
				m_TargetMonitor.SetMark( m_hFollowTarget, m_FollowNavGoal.targetMoveTolerance );
			return SCHED_FACE_FOLLOW_TARGET; // @TODO (toml 03-03-03): should select a facing sched
		}

		case SCHED_TARGET_CHASE:
		{
			return SCHED_FOLLOW;
		}

		// SCHED_ESTABLISH_LINE_OF_FIRE_FALLBACK just tells the NPC to chase their enemy, so
		// forbid this unless the destination is acceptable within the parameters of the follow behavior.
		case SCHED_CHASE_ENEMY:
		case SCHED_ESTABLISH_LINE_OF_FIRE_FALLBACK:
		{
			if ( IsChaseGoalInRange() == false )
				return SCHED_FOLLOWER_IDLE_STAND;
			break;
		}

		case SCHED_RANGE_ATTACK1:
		{
			if ( GetOuter()->GetShotRegulator()->IsInRestInterval() )
			{
				if ( GetEnemy() )
					return SCHED_FOLLOWER_COMBAT_FACE;
			
				return SCHED_FOLLOWER_IDLE_STAND; // @TODO (toml 07-02-03): Should do something more tactically sensible
			}
			break;
		}

		case SCHED_CHASE_ENEMY_FAILED:
		{
			if (HasMemory(bits_MEMORY_INCOVER))
			{
				// Make sure I don't get too far from the player
				if ( GetFollowTarget() )
				{
					float fDist = (GetLocalOrigin() - GetFollowTarget()->GetAbsOrigin()).Length();
					if (fDist > 500)
					{
						return SCHED_FOLLOW;
					}
				}
			}
			break;
		}

		case SCHED_MOVE_AWAY_FAIL:
		{
			return SCHED_FOLLOWER_MOVE_AWAY_FAIL;
		}
		case SCHED_MOVE_AWAY_END:
		{
			return SCHED_FOLLOWER_MOVE_AWAY_END;
		}
	}
	return scheduleType;
}
