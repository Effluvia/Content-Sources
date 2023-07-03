#include "cbase.h"
#include "hoe_human_follower.h"
#include "hl2_player.h"
#include "ai_pathfinder.h"
#include "ai_route.h"
#include "ai_senses.h"
#include "hoe_human_medic.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar g_debug_transitions;

#define PLAYERCOMPANION_TRANSITION_SEARCH_DISTANCE		(100*12)

// fixme: also in hoe_human.cpp
#define COMBINE_GRENADE_FLUSH_TIME	3.0		// Don't try to flush an enemy who has been out of sight for longer than this.
#define COMBINE_GRENADE_FLUSH_DIST	256.0	// Don't try to flush an enemy who has moved farther than this distance from the last place I saw him.

BEGIN_DATADESC( CHOEHumanFollower )
	DEFINE_FIELD( m_flTimeShotByPlayer, FIELD_TIME ),
	DEFINE_FIELD( m_flTimePlayerShotFriend, FIELD_TIME ),
	DEFINE_FIELD( m_flOutWayTime, FIELD_TIME ),
	DEFINE_FIELD( m_flPlayerPushTime, FIELD_TIME ),
	DEFINE_FIELD( m_iPlayerPushCount, FIELD_INTEGER ),
	DEFINE_FIELD( m_bMovingAwayFromPlayer, FIELD_BOOLEAN ),

	DEFINE_INPUTFUNC( FIELD_VOID, "AddToPlayerSquad", InputAddToPlayerSquad ),
	DEFINE_INPUTFUNC( FIELD_VOID, "OutsideTransition", InputOutsideTransition ),

	DEFINE_OUTPUT( m_OnPlayerUse, "OnPlayerUse" ),

	DEFINE_USEFUNC( UseFollower ),
END_DATADESC()

// Make sure these formations support 8 slots.
// Also the FOLLOW_POINT_FORMATION must not have the AIFF_REQUIRE_LOS_OUTSIDE_COMBAT
// flag set.
#define FOLLOW_PLAYER_FORMATION		AIF_VORTIGAUNT //AIF_SIDEKICK
#define FOLLOW_POINT_FORMATION		AIF_COMMANDER

//-----------------------------------------------------------------------------
void CHOEHumanFollower::Spawn( void )
{
	BaseClass::Spawn();

	m_flPlayerPushTime = FLT_MIN;

	SetUse( &CHOEHumanFollower::UseFollower );
}

//-----------------------------------------------------------------------------
bool CHOEHumanFollower::CreateBehaviors( void )
{
	AddBehavior( &m_FollowBehavior );
	AddBehavior( &m_LeadBehavior );	
	
	return BaseClass::CreateBehaviors();
}

//-----------------------------------------------------------------------------
int CHOEHumanFollower::ObjectCaps( void ) 
{ 
	// Set a flag indicating this NPC is usable by the player
	int caps = UsableNPCObjectCaps( BaseClass::ObjectCaps() );
	return caps; 
}

//-----------------------------------------------------------------------------
Disposition_t CHOEHumanFollower::IRelationType( CBaseEntity *pTarget )
{
	if ( pTarget->IsPlayer() && HasMemory( bits_MEMORY_PROVOKED ) )
		return D_HT;

	return BaseClass::IRelationType( pTarget );
}

//-----------------------------------------------------------------------------
int CHOEHumanFollower::OnTakeDamage_Alive( const CTakeDamageInfo &info )
{
	int ret = BaseClass::OnTakeDamage_Alive( info );
	if ( m_lifeState != LIFE_ALIVE )
		return ret;

	if ( GetState() != NPC_STATE_PRONE &&
		 info.GetAttacker() &&
		 info.GetAttacker()->IsPlayer() &&
		 TimeNotRecent( GetLastEnemyTime(), 2.0f ) &&
		 IRelationType( info.GetAttacker() ) == D_LI )
	{
		// This is a heurstic to determine if the player intended to harm me
		// If I have an enemy, we can't establish intent (may just be crossfire)
		if ( GetEnemy() == NULL && info.GetDamageType() == DMG_BULLET )
		{
			// If I'm already suspicious, get mad
			if ( m_afMemory & bits_MEMORY_SUSPICIOUS )
			{
				// Alright, now I'm pissed!
				Remember( bits_MEMORY_PROVOKED );

				// Allowed to hit the player now!
//				CapabilitiesRemove(bits_CAP_NO_HIT_PLAYER);

#ifdef PLAYER_SQUAD_STUFF
				if ( IsInPlayerSquad() )
				{
					ClearFollowTarget();
					ClearCommandGoal();
					RemoveFromSquad(); // FIXME: add to original squad if any
				}
#else
				if ( m_FollowBehavior.GetFollowTarget() == info.GetAttacker() )
					m_FollowBehavior.SetFollowTarget( NULL );
#endif
			}
			else
			{
				// Hey, be careful with that
				Remember( bits_MEMORY_SUSPICIOUS );
			}

			m_flTimeShotByPlayer = gpGlobals->curtime;
			NotifyFriendsOfPlayerDamage( info.GetAttacker() );
		}
	}

	return ret;
}

//-----------------------------------------------------------------------------
void CHOEHumanFollower::NotifyFriendsOfPlayerDamage( CBaseEntity *pPlayer )
{
	CUtlVector<EHANDLE> friends;

	// If I'm mad, tell my friends to get mad.
	// Otherwise bump each friend one step from neutral->suspicious->provoked and
	// make everyone provoked if anyone reaches provoked.
	bool bProvoked = HasMemory( bits_MEMORY_PROVOKED );

	for ( int i = 0; GetFriendClasses()[i] ; i++ )
	{
		const char *pszFriend = GetFriendClasses()[i];

		if ( !Q_strcmp( pszFriend, "player" ) )
			continue;

		CBaseEntity *pEntity = NULL;
		while ( ( pEntity = gEntList.FindEntityByClassnameWithin( pEntity, pszFriend, GetAbsOrigin(), 768.0f ) ) != 0 )
		{
			if ( pEntity == this || !pEntity->MyNPCPointer() )
				continue;

			if ( !pEntity->IsAlive() || pEntity->m_lifeState == LIFE_DYING )
				continue;

			if ( pEntity->MyNPCPointer()->GetState() == NPC_STATE_PRONE )
				continue;

			if ( pEntity->MyNPCPointer()->IsInAScript() )
				continue;

			if ( !FVisible( pEntity ) && !pEntity->MyNPCPointer()->FVisible( this ) )
				continue;

			friends.AddToTail( pEntity );

			if ( pEntity->MyNPCPointer()->HasMemory( bits_MEMORY_SUSPICIOUS ) )
			{
				bProvoked = true;
			}
			else
			{
				pEntity->MyNPCPointer()->Remember( bits_MEMORY_SUSPICIOUS );
			}

			CHOEHumanFollower *pFollower = dynamic_cast<CHOEHumanFollower *>( pEntity );
			if ( pFollower )
			{
				pFollower->m_flTimePlayerShotFriend = gpGlobals->curtime;
			}
		}
	}

	if ( bProvoked )
	{
		for ( int i = 0; i < friends.Count(); i++ )
		{
			friends[i]->MyNPCPointer()->Remember( bits_MEMORY_PROVOKED );

			CHOEHumanFollower *pFollower = dynamic_cast<CHOEHumanFollower *>( friends[i].Get() );
			if ( pFollower )
			{
#ifdef PLAYER_SQUAD_STUFF
				if ( pFollower->IsInPlayerSquad() )
				{
					pFollower->ClearFollowTarget();
					pFollower->ClearCommandGoal();
					pFollower->RemoveFromSquad(); // FIXME: add to original squad if any
				}
#else
				if ( pFollower->m_FollowBehavior.GetFollowTarget() == pPlayer )
					pFollower->m_FollowBehavior.SetFollowTarget( NULL );
#endif
			}
		}
	}
}

//-----------------------------------------------------------------------------
void CHOEHumanFollower::PrescheduleThink( void )
{
	if ( OkToShout() && TimeRecent( m_flTimeShotByPlayer, 4 ) )
	{
		CBasePlayer *pPlayer = AI_GetSinglePlayer();
		if ( pPlayer && IsValidSpeechTarget( pPlayer ) )
		{
			SpeechSelection_t selection;
			if ( HasMemory( bits_MEMORY_PROVOKED ) &&
				!SpokeConcept( TLK_MAD ) &&
				SelectSpeechResponse( TLK_MAD, NULL, selection ) )
			{
				SetTalkTarget( pPlayer );
				DispatchSpeechSelection( selection );
			}

			if ( !HasMemory( bits_MEMORY_PROVOKED ) &&
				HasMemory( bits_MEMORY_SUSPICIOUS ) &&
				!SpokeConcept( TLK_SHOT ) &&
				SelectSpeechResponse( TLK_SHOT, NULL, selection ) )
			{
				SetTalkTarget( pPlayer );
				DispatchSpeechSelection( selection );
			}
		}
	}

	else if ( OkToShout() && TimeRecent( m_flTimePlayerShotFriend, 4 ) )
	{
		CBasePlayer *pPlayer = AI_GetSinglePlayer();
		if ( pPlayer && IsValidSpeechTarget( pPlayer ) )
		{
			SpeechSelection_t selection;
			if ( !HasMemory( bits_MEMORY_PROVOKED ) &&
				HasMemory( bits_MEMORY_SUSPICIOUS ) &&
				!SpokeConcept( TLK_NOSHOOT ) &&
				SelectSpeechResponse( TLK_NOSHOOT, NULL, selection ) )
			{
				SetTalkTarget( pPlayer );
				DispatchSpeechSelection( selection );
			}
		}
	}

	BaseClass::PrescheduleThink();

#ifdef PLAYER_SQUAD_STUFF
	UpdateFollowCommandPoint();
#endif
}

//-----------------------------------------------------------------------------
void CHOEHumanFollower::Event_Killed( const CTakeDamageInfo &info )
{
	// notify the player
	// This updates the "squad status" hud element
	if ( IsInPlayerSquad() )
	{
		CBasePlayer *player = AI_GetSinglePlayer();
		if ( player )
		{
			variant_t emptyVariant;
			player->AcceptInput( "OnSquadMemberKilled", this, this, emptyVariant, 0 );
		}
	}

	BaseClass::Event_Killed( info );
}

//-----------------------------------------------------------------------------
bool CHOEHumanFollower::IdleSpeech( void )
{
	if ( HasSpawnFlags( SF_HUMAN_PREDISASTER ) )
	{
		if ( !OkToSpeak() )
			return false;

		CBasePlayer *pPlayer = AI_GetSinglePlayer();
		if ( pPlayer && !HasMemory( bits_MEMORY_PROVOKED ) &&
			IsValidSpeechTarget( pPlayer ) )
		{
			if ( GetTimePlayerStaring() > 6 && !IsMoving() )
			{
#ifdef HOE_HUMAN_RR
				SpeechSelection_t selection;
				if ( SelectSpeechResponse( TLK_STARE, NULL, selection ) )
				{
					SetTalkTarget( pPlayer );
					DispatchSpeechSelection( selection );
				}
#else
				Speak( "STARE", HUMAN_SC_STARE, pPlayer );
#endif
				return true;
			}
#if 0 // FIXME: nama0 has scripted idle speech, but alamo0 may want this
			else if ( !random->RandomInt( 0, 4 ) )
			{
#ifdef HOE_HUMAN_RR
				SpeechSelection_t selection;
				if ( SelectSpeechResponse( TLK_PIDLE, NULL, selection ) )
				{
					SetTalkTarget( pPlayer );
					DispatchSpeechSelection( selection );
				}
#else
				Speak( "PIDLE", HUMAN_SC_IDLE, pPlayer );
#endif
				return true;
			}
#endif
		}
		return false;
	}

	return BaseClass::IdleSpeech();
}

//-----------------------------------------------------------------------------
bool CHOEHumanFollower::AfraidOfHuey( void )
{
	// Don't fear the Huey if following the player, otherwise we'll run from it
	// and leave the player behind.
	if ( GetFollowBehavior().GetFollowTarget() != NULL )
		return false;

	return BaseClass::AfraidOfHuey();
}

//-----------------------------------------------------------------------------
void CHOEHumanFollower::GatherConditions( void )
{
	BaseClass::GatherConditions();

	SpeedBoost();

	PredictPlayerPush();

	// Periodically forget the player was pushing us.
	if ( !HasCondition( COND_PLAYER_PUSHING ) && TimeNotRecent( m_flPlayerPushTime, 10.0f ) )
	{
		m_iPlayerPushCount = 0;
	}

	if ( GetMotor()->IsDeceleratingToGoal() && IsCurTaskContinuousMove() && 
		 HasCondition( COND_PLAYER_PUSHING ) && IsCurSchedule( SCHED_MOVE_AWAY ) )
	{
		ClearSchedule( "player pushing" );
	}

	if ( HasMemory( bits_MEMORY_SUSPICIOUS ) &&
		TimeNotRecent( m_flTimeShotByPlayer, 30.0f ) &&
		TimeNotRecent( m_flTimePlayerShotFriend, 30.0f ) )
	{
		Forget( bits_MEMORY_SUSPICIOUS );
		GetExpresser()->ClearSpokeConcept( TLK_SHOT );
		GetExpresser()->ClearSpokeConcept( TLK_NOSHOOT );
	}
}

//-----------------------------------------------------------------------------
void CHOEHumanFollower::SpeedBoost( void )
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

#ifdef PLAYER_SQUAD_STUFF
//-----------------------------------------------------------------------------
bool CHOEHumanFollower::ShouldAlwaysThink() 
{ 
	return ( BaseClass::ShouldAlwaysThink() || IsInPlayerSquad() ||
		( GetFollowBehavior().GetFollowTarget() && GetFollowBehavior().GetFollowTarget()->IsPlayer() ) ); 
}
#endif

//-----------------------------------------------------------------------------
int CHOEHumanFollower::SelectSchedulePriority( void )
{
	m_bMovingAwayFromPlayer = false;

	int schedule = BaseClass::SelectSchedulePriority();
	if ( schedule != SCHED_NONE )
		return schedule;

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
			return SCHED_NONE; // this has to lead to the behavior selecting a schedule
		}
	}

	return SCHED_NONE;
}

//-----------------------------------------------------------------------------
int CHOEHumanFollower::SelectSchedulePlayerPush( void )
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
bool CHOEHumanFollower::ShouldDeferToFollowBehavior( void )
{
	if ( !GetFollowBehavior().CanSelectSchedule() || !GetFollowBehavior().FarFromFollowTarget() )
		return false;
#if 0
	if ( m_StandoffBehavior.CanSelectSchedule() && !m_StandoffBehavior.IsBehindBattleLines( GetFollowBehavior().GetFollowTarget()->GetAbsOrigin() ) )
		return false;
#endif

	if ( HasCondition(COND_BETTER_WEAPON_AVAILABLE) && !GetActiveWeapon() )
	{
		// Unarmed allies should arm themselves as soon as the opportunity presents itself.
		return false;
	}

#if 0
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
int CHOEHumanFollower::TranslateSchedule( int scheduleType )
{
	// I want the follow behavior to have a chance of stomping schedules.
	if ( GetFollowBehavior().IsRunning() )
		scheduleType = GetFollowBehavior().TranslateScheduleHack( scheduleType );
	return BaseClass::TranslateSchedule( scheduleType );
}

//-----------------------------------------------------------------------------
void CHOEHumanFollower::BuildScheduleTestBits( void )
{
	BaseClass::BuildScheduleTestBits();
	
	if ( CheckFollowPlayer() )
	{
		// Always interrupt to get into the car
	//	SetCustomInterruptCondition( COND_PC_BECOMING_PASSENGER );

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
float CHOEHumanFollower::GetIdealSpeed() const
{
	float baseSpeed = BaseClass::GetIdealSpeed();

	if ( baseSpeed < m_flBoostSpeed )
		return m_flBoostSpeed;

	return baseSpeed;
}

//-----------------------------------------------------------------------------
float CHOEHumanFollower::GetIdealAccel() const
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
bool CHOEHumanFollower::OnObstructionPreSteer( AILocalMoveGoal_t *pMoveGoal, float distClear, AIMoveResult_t *pResult )
{
	if ( pMoveGoal->directTrace.flTotalDist - pMoveGoal->directTrace.flDistObstructed < GetHullWidth() * 1.5 )
	{
		CAI_BaseNPC *pBlocker = pMoveGoal->directTrace.pObstruction->MyNPCPointer();
		if ( pBlocker && ShouldBlockerMoveAway( pBlocker ) )
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
bool CHOEHumanFollower::ShouldBlockerMoveAway( CAI_BaseNPC *pBlocker )
{
	if ( !pBlocker )
		return false;

	if ( !pBlocker->IsPlayerAlly() && !FClassnameIs( pBlocker, "npc_peasant" ) )
		return false;
	
	if ( pBlocker->IsMoving() || pBlocker->IsInAScript() )
		return false;

	if ( IsCurSchedule( SCHED_NEW_WEAPON ) || 
		 IsCurSchedule( SCHED_GET_HEALTHKIT ) || 
		 pBlocker->IsCurSchedule( SCHED_FAIL ) || 
		 ( IsInPlayerSquad() && !pBlocker->IsInPlayerSquad() ) ||
		 ClassifyPlayerAllyVital() ||
		 IsInAScript() )
	{
		return true;
	}

	// Don't let hiding medics block us in.
	if ( GetState() == NPC_STATE_COMBAT &&
		 ( FClassnameIs( pBlocker, "npc_mikeforce_medic" ) ||
		   FClassnameIs( pBlocker, "npc_grunt_medic" ) ) &&
		   IRelationType( pBlocker ) == D_LI && 
		 ( pBlocker->IsCurSchedule( SCHED_STANDOFF ) ||
		   pBlocker->IsCurSchedule( SCHED_HUMAN_WAIT_IN_COVER ) ) )
	{
		DevMsg( "%s telling medic %s to MOVE AWAY\n", GetDebugName(), pBlocker->GetDebugName() );
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
void CHOEHumanFollower::StartTask( const Task_t *pTask )
{
	switch( pTask->iTask )
	{
	case TASK_MOVE_AWAY_PATH:
		{
			// Count the number of times we get out of the player's way. If it happens too often
			// then appologize.
			if ( TimeRecent( m_flPlayerPushTime, 3.0f ) )
				m_iPlayerPushCount++;

			CBasePlayer *pPlayer = AI_GetSinglePlayer();
			if ( m_bMovingAwayFromPlayer &&
				( GetState() != NPC_STATE_COMBAT ) &&
				OkToSpeak() &&
//				( m_flOutWayTime < gpGlobals->curtime ) &&
				( m_iPlayerPushCount >= 3 ) &&
				// If NPC is close behind and player moves forward the NPC still moves away, even though
				// the player is moving away from the NPC. So make sure the player is looking at the
				// NPC before apologizing.
				pPlayer->FInViewCone( this ) )
			{
#ifdef HOE_HUMAN_RR
				Speak( TLK_OUTWAY );
#else
				Speak( "OUTWAY" );
#endif
//				m_flOutWayTime = gpGlobals->curtime + 20.0;
			}

			BaseClass::StartTask( pTask );
		}
		break;

	default:
		BaseClass::StartTask( pTask );
		break;
	}
}

//-----------------------------------------------------------------------------
void CHOEHumanFollower::RunTask( const Task_t *pTask )
{
	switch( pTask->iTask )
	{
	case TASK_MOVE_AWAY_PATH:
		{
			BaseClass::RunTask( pTask );

			// This is so followers face the player when getting out of his/her way
			if ( GetNavigator()->IsGoalActive() && !GetEnemy() )
			{
				AddFacingTarget( EyePosition() + BodyDirection2D() * 240, 1.0, 2.0 );
			}
		}
		break;

	default:
		BaseClass::RunTask( pTask );
		break;
	}
}

//-----------------------------------------------------------------------------
bool CHOEHumanFollower::IgnorePlayerPushing( void )
{
	// Ignore player pushes if we're leading him
	if ( m_LeadBehavior.IsRunning() && m_LeadBehavior.HasGoal() )
		return true;

	return false;
}

//-----------------------------------------------------------------------------
void CHOEHumanFollower::UseFollower( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	// Check that caller is the player
	if ( !pCaller || !pCaller->IsPlayer() )
		return;

	// Pre-disaster followers can't be used
	if ( HasSpawnFlags( SF_HUMAN_PREDISASTER ) )
	{
		if ( OkToShout() )
		{
#ifdef HOE_HUMAN_RR
			SpeechSelection_t selection;
			if ( SelectSpeechResponse( TLK_POK, NULL, selection ) )
			{
				SetTalkTarget( pCaller );
				DispatchSpeechSelection( selection );
			}
			else
			{
				ToBasePlayer( pCaller )->PlayUseDenySound();
			}
#else
			Speak( "POK", 0, pCaller );
#endif
		}
		else
		{
			ToBasePlayer( pCaller )->PlayUseDenySound();
		}
		return;
	}

	if ( !CheckFollowPlayer() )
	{
		ToBasePlayer( pCaller )->PlayUseDenySound();
		return;
	}

	m_OnPlayerUse.FireOutput( pActivator, pCaller );

	// Poke the NPC to interrupt any stubborn schedules
	SetCondition( COND_PROVOKED );

	// Toggle follow
	if ( m_FollowBehavior.GetFollowTarget() != NULL )
	{
#ifdef HOE_HUMAN_RR
			SpeechSelection_t selection;
			if ( SelectSpeechResponse( TLK_UNUSE, NULL, selection ) )
		{
			if ( GetState() != NPC_STATE_COMBAT )
				SetTalkTarget( pCaller );
			DispatchSpeechSelection( selection );

			if ( GetSpeechManager() )
			{
				GetSpeechManager()->ExtendSpeechCategoryTimer( SPEECH_CATEGORY_IDLE, 15 );
			}
		}
#else
		Speak( "UNUSE", 0, pCaller );
#endif
#ifdef PLAYER_SQUAD_STUFF
		ClearFollowTarget();
		ClearCommandGoal();

		if ( IsInPlayerSquad() )
			RemoveFromSquad(); // FIXME: add to original squad if any
#else
		m_FollowBehavior.SetFollowTarget( NULL );
#endif
	}
	else
	{
#ifdef HOE_HUMAN_RR
		SpeechSelection_t selection;
		if ( SelectSpeechResponse( TLK_USE, NULL, selection ) )
		{
			if ( GetState() != NPC_STATE_COMBAT )
				SetTalkTarget( pCaller );
			DispatchSpeechSelection( selection );

			if ( GetSpeechManager() )
			{
				GetSpeechManager()->ExtendSpeechCategoryTimer( SPEECH_CATEGORY_IDLE, 15 );
			}
		}

		// We got commanded so don't say "hello" later
		GetExpresser()->SetSpokeConcept( TLK_HELLO, NULL, false );
#else
		Speak( "USE", 0, pCaller );
#endif
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
	}
}

//------------------------------------------------------------------------------
void CHOEHumanFollower::InputAddToPlayerSquad( inputdata_t &inputdata )
{
	CBasePlayer *pPlayer = AI_GetSinglePlayer();
	if ( !pPlayer || !pPlayer->IsAlive() )
		return;

	if ( IsInPlayerSquad() )
		return;

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
		m_FollowBehavior.SetFollowTarget( pPlayer );
		m_FollowBehavior.SetParameters( FOLLOW_PLAYER_FORMATION );
	}

	SetCondition( COND_PROVOKED );

	// We got commanded so don't say "hello" later
	GetExpresser()->SetSpokeConcept( TLK_HELLO, NULL, false );
}

//------------------------------------------------------------------------------
void CHOEHumanFollower::Touch( CBaseEntity *pOther )
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

			if ( HasCondition( COND_PLAYER_PUSHING ) )
				m_flPlayerPushTime = gpGlobals->curtime;
		}
	}
}

//-----------------------------------------------------------------------------
void CHOEHumanFollower::PredictPlayerPush( void )
{
	if ( CheckFollowPlayer() )
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
}

//-----------------------------------------------------------------------------
bool CHOEHumanFollower::IsFollowingPlayer( void )
{
	return BaseClass::IsFollowingPlayer();
	//return GetFollowBehavior().GetFollowTarget() != NULL;
}

//-----------------------------------------------------------------------------
void CHOEHumanFollower::OnStateChange( NPC_STATE oldState, NPC_STATE newState )
{
	BaseClass::OnStateChange( oldState, newState ); // alyx doesn't call this???

#ifdef PLAYER_SQUAD_STUFF
	if ( newState == NPC_STATE_SCRIPT )
	{
		ClearCommandGoal();

		// It should really be up to the mapmaker whether someone (usually Barney) stops following
		// the player after a scripted_sequence ends
		if ( IsInPlayerSquad() )
			RemoveFromSquad(); // FIXME: add to original squad if any
	}
#endif

	// Stop following the player if leaving a scripted sequence so we don't move away from the script position.
	if ( oldState == NPC_STATE_SCRIPT )
	{
		// Can't clear this if newState == NPC_STATE_SCRIPT because that will cancel the script.
#ifdef PLAYER_SQUAD_STUFF
		ClearFollowTarget();
#else
		m_FollowBehavior.SetFollowTarget( NULL );
#endif
	}
}

//-----------------------------------------------------------------------------
bool CHOEHumanFollower::CheckFollowPlayer( void )
{
	if ( HasMemory( bits_MEMORY_PROVOKED ) )
		return false;

	// Pre-disaster followers can't be used
	if ( HasSpawnFlags( SF_HUMAN_PREDISASTER ) )
		return false;

	// Can't +USE NPCs running scripts
	if ( IsInAScript() )
		return false;

	return ( ClassifyPlayerAlly() || ClassifyPlayerAllyVital() );
}

//-----------------------------------------------------------------------------
bool CHOEHumanFollower::FValidateHintType( CAI_Hint *pHint )
{
	switch( pHint->HintType() )
	{
	case HINT_PLAYER_SQUAD_TRANSITON_POINT:
		return true;
		break;

	default:
		break;
	}

	return BaseClass::FValidateHintType( pHint );
}

//-----------------------------------------------------------------------------
bool CHOEHumanFollower::ShouldAlwaysTransition( void )
{
	// No matter what, come through
	if ( m_bAlwaysTransition )
		return true;

#if 0 // HOE: disabled since squad member may be stationed
	// Squadmates always come with
	if ( IsInPlayerSquad() )
		return true;
#endif

	// If we're following the player, then come along
	if ( GetFollowBehavior().GetFollowTarget() && GetFollowBehavior().GetFollowTarget()->IsPlayer() )
		return true;

	return false;
}

//-----------------------------------------------------------------------------
void CHOEHumanFollower::InputOutsideTransition( inputdata_t &inputdata )
{
	if ( !AI_IsSinglePlayer() )
		return;

	// Must want to do this
	if ( ShouldAlwaysTransition() == false )
		return;

	// If we're in a vehicle, that vehicle will transition with us still inside (which is preferable)
	if ( IsInAVehicle() )
		return;

	CBaseEntity *pPlayer = UTIL_GetLocalPlayer();
	const Vector &playerPos = pPlayer->GetAbsOrigin();

	// Mark us as already having succeeded if we're vital or always meant to come with the player
	bool bAlwaysTransition = ( /*ClassifyPlayerAllyVital() ||*/ m_bAlwaysTransition );
	bool bPathToPlayer = bAlwaysTransition;

	if ( bAlwaysTransition == false )
	{
		AI_Waypoint_t *pPathToPlayer = GetPathfinder()->BuildRoute( GetAbsOrigin(), playerPos, pPlayer, 0 );

		if ( pPathToPlayer )
		{
			bPathToPlayer = true;
			CAI_Path tempPath;
			tempPath.SetWaypoints( pPathToPlayer ); // path object will delete waypoints
			GetPathfinder()->UnlockRouteNodes( pPathToPlayer );
		}
	}


#ifdef USE_PATHING_LENGTH_REQUIREMENT_FOR_TELEPORT
	float pathLength = tempPath.GetPathDistanceToGoal( GetAbsOrigin() );

	if ( pathLength > 150 * 12 )
		return;
#endif

	bool bMadeIt = false;
	Vector teleportLocation;

#if 1
	// Use the landmark name as the hint group. This avoids the problem of having 2 or more
	// level transitions near each other.
	extern string_t HOE_GetLandmarkName( void );
	CHintCriteria criteria;
	criteria.SetHintType( HINT_PLAYER_SQUAD_TRANSITON_POINT );
//	criteria.SetFlag( bits_HINT_NODE_NEAREST );
	criteria.SetGroup( HOE_GetLandmarkName() );
	criteria.AddIncludePosition( GetAbsOrigin()/*playerPos*/, PLAYERCOMPANION_TRANSITION_SEARCH_DISTANCE );

	CAI_Hint *pNearest = NULL;
	float flBestDistance = MAX_TRACE_LENGTH;

	// The original playercompanion code looked for a node nearest to the player, but I look
	// for one nearest to the NPC.  I only put the nodes close to the transition point so I know
	// they are close the the player.
	// The original playercompanion code would break if an NPC could not find a path to any hint,
	// because it would lock each hint as it tested them, preventing other NPCs using those hints.
	CUtlVector<CAI_Hint *> hints;
	int hintCount = CAI_HintManager::FindAllHints( this, GetAbsOrigin() /*playerPos*/, criteria, &hints );
	for ( int i = 0; i < hintCount; i++ )
	{
		CAI_Hint *pHint = hints[i];
#else
	CAI_Hint *pHint = CAI_HintManager::FindHint( this, HINT_PLAYER_SQUAD_TRANSITON_POINT, bits_HINT_NODE_NEAREST, PLAYERCOMPANION_TRANSITION_SEARCH_DISTANCE, &playerPos );
	while ( pHint )
	{
		pHint->Lock(this);
		pHint->Unlock(0.5); // prevent other squadmates and self from using during transition. 
#endif

		pHint->GetPosition( GetHullType(), &teleportLocation );
		if ( GetNavigator()->CanFitAtPosition( teleportLocation, MASK_NPCSOLID ) )
		{
			bMadeIt = true;
			if ( !bPathToPlayer /*&& ( playerPos - GetAbsOrigin() ).LengthSqr() > Square(40*12)*/ )
			{
				AI_Waypoint_t *pPathToTeleport = GetPathfinder()->BuildRoute( GetAbsOrigin(), teleportLocation, pPlayer, 0 );

				if ( !pPathToTeleport )
				{
					DevMsg( 2, "NPC \"%s\" failed to teleport to transition a point because there is no path\n", GetDebugName() );
					bMadeIt = false;
				}
				else
				{
					CAI_Path tempPath;
					GetPathfinder()->UnlockRouteNodes( pPathToTeleport );
					tempPath.SetWaypoints( pPathToTeleport ); // path object will delete waypoints
				}
			}

			if ( bMadeIt )
			{
#if 1
				float flDist = (teleportLocation - GetAbsOrigin()).Length();
				if ( flDist < flBestDistance )
				{
					flBestDistance = flDist;
					pNearest = pHint;
				}
#else
				DevMsg( 2, "NPC \"%s\" teleported to transition point %d\n", GetDebugName(), pHint->GetNodeId() );
				break;
#endif
			}
		}
		else
		{
			if ( g_debug_transitions.GetBool() )
			{
				NDebugOverlay::Box( teleportLocation, GetHullMins(), GetHullMaxs(), 255,0,0, 8, 999 );
			}
		}
#if 0
		pHint = CAI_HintManager::FindHint( this, HINT_PLAYER_SQUAD_TRANSITON_POINT, bits_HINT_NODE_NEAREST, PLAYERCOMPANION_TRANSITION_SEARCH_DISTANCE, &playerPos );
#endif
	}
	bMadeIt = ( pNearest != NULL );

	if ( !bMadeIt )
	{
		// Force us if we didn't find a normal route
		if ( bAlwaysTransition )
		{
			bMadeIt = FindSpotForNPCInRadius( &teleportLocation, pPlayer->GetAbsOrigin(), this, 32.0*1.414, true );
			if ( !bMadeIt )
				bMadeIt = FindSpotForNPCInRadius( &teleportLocation, pPlayer->GetAbsOrigin(), this, 32.0*1.414, false );
		}
	}

	if ( bMadeIt )
	{
		if ( pNearest != NULL )
		{
			DevMsg( 2, "NPC \"%s\" teleported to transition point %d\n", GetDebugName(), pNearest->GetNodeId() );
			pNearest->Lock( this );
			pNearest->Unlock( 0.5 );
			pNearest->GetPosition( GetHullType(), &teleportLocation );

		}
		Teleport( &teleportLocation, NULL, NULL );
	}
	else
	{
		DevMsg( 2, "NPC \"%s\" failed to find a suitable transition point\n", GetDebugName() );
	}

	BaseClass::InputOutsideTransition( inputdata );
}

#ifdef PLAYER_SQUAD_STUFF

//-----------------------------------------------------------------------------

#define COMMAND_POINT_CLASSNAME "info_target_command_point"

#if 0 //  npc_citizen17.cpp
class CCommandPoint : public CPointEntity
{
	DECLARE_CLASS( CCommandPoint, CPointEntity );
public:
	CCommandPoint()
		: m_bNotInTransition(false)
	{
		if ( ++gm_nCommandPoints > 1 )
			DevMsg( "WARNING: More than one citizen command point present\n" );
	}

	~CCommandPoint()
	{
		--gm_nCommandPoints;
	}

	int ObjectCaps()
	{
		int caps = ( BaseClass::ObjectCaps() | FCAP_NOTIFY_ON_TRANSITION );

		if ( m_bNotInTransition )
			caps |= FCAP_DONT_SAVE;

		return caps;
	}

	void InputOutsideTransition( inputdata_t &inputdata )
	{
		if ( !AI_IsSinglePlayer() )
			return;

		m_bNotInTransition = true;

		CAI_Squad *pPlayerAISquad = g_AI_SquadManager.FindSquad(AllocPooledString(PLAYER_SQUADNAME));

		if ( pPlayerAISquad )
		{
			AISquadIter_t iter;
			for ( CAI_BaseNPC *pAllyNpc = pPlayerAISquad->GetFirstMember(&iter); pAllyNpc; pAllyNpc = pPlayerAISquad->GetNextMember(&iter) )
			{
				if ( pAllyNpc->GetCommandGoal() != vec3_invalid )
				{
					bool bHadGag = pAllyNpc->HasSpawnFlags(SF_NPC_GAG);

					pAllyNpc->AddSpawnFlags(SF_NPC_GAG);
					pAllyNpc->TargetOrder( UTIL_GetLocalPlayer(), &pAllyNpc, 1 );
					if ( !bHadGag )
						pAllyNpc->RemoveSpawnFlags(SF_NPC_GAG);
				}
			}
		}
	}
	DECLARE_DATADESC();

private:
	bool m_bNotInTransition; // does not need to be saved. If this is ever not default, the object is not being saved.
	static int gm_nCommandPoints;
};

int CCommandPoint::gm_nCommandPoints;

LINK_ENTITY_TO_CLASS( info_target_command_point, CCommandPoint );
BEGIN_DATADESC( CCommandPoint )
	
//	DEFINE_FIELD( m_bNotInTransition,	FIELD_BOOLEAN ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"OutsideTransition",	InputOutsideTransition ),

END_DATADESC()

#endif

//-----------------------------------------------------------------------------
bool CHOEHumanFollower::IsCommandable( void ) 
{
	return IsInPlayerSquad();
}

//-----------------------------------------------------------------------------
bool CHOEHumanFollower::IsFollowingCommandPoint( void )
{
	CBaseEntity *pFollowTarget = m_FollowBehavior.GetFollowTarget();
	if ( pFollowTarget )
		return FClassnameIs( pFollowTarget, COMMAND_POINT_CLASSNAME );
	return false;
}

//-----------------------------------------------------------------------------
bool CHOEHumanFollower::HaveCommandGoal( void ) const			
{	
	if (GetCommandGoal() != vec3_invalid)
		return true;
	return false;
}

//-----------------------------------------------------------------------------
struct SquadMemberInfo_t
{
	CHOEHumanFollower *pMember;
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

CAI_BaseNPC *CHOEHumanFollower::GetSquadCommandRepresentative( void )
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
					if ( pAllyNpc->IsCommandable() && dynamic_cast<CHOEHumanFollower *>(pAllyNpc) )
					{
						int i = candidates.AddToTail();
						candidates[i].pMember = (CHOEHumanFollower *)(pAllyNpc);
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
			Assert( dynamic_cast<CHOEHumanFollower *>(hCurrent.Get()) && hCurrent->IsInPlayerSquad() );
			return hCurrent;
		}
	}
	return NULL;
}

//-----------------------------------------------------------------------------
void CHOEHumanFollower::ClearFollowTarget( void )
{
	m_FollowBehavior.SetFollowTarget( NULL );
	m_FollowBehavior.SetParameters( AIF_SIMPLE );
}

//-----------------------------------------------------------------------------
void CHOEHumanFollower::UpdateFollowCommandPoint( void )
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
bool CHOEHumanFollower::TargetOrder( CBaseEntity *pTarget, CAI_BaseNPC **Allies, int numAllies )
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
#if 1
			CAI_BaseNPC *pClosest = NULL;
			float closestDistSq = FLT_MAX;

			for( int i = 0 ; i < numAllies ; i++ )
			{
				if( Allies[i]->IsInPlayerSquad() )
				{
					Assert( Allies[i]->IsCommandable() );
					float distSq = ( pTarget->GetAbsOrigin() - Allies[i]->GetAbsOrigin() ).LengthSqr();
					if( distSq < closestDistSq )
					{
						pClosest = Allies[i];
						closestDistSq = distSq;
					}
				}
			}

			if ( OkToShout() && pClosest == this )
				Speak( TLK_USE );
#else
//			SpeakCommandResponse( TLK_STARTFOLLOW );

			m_OnFollowOrder.FireOutput( this, this );
#endif
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
void CHOEHumanFollower::MoveOrder( const Vector &vecDest, CAI_BaseNPC **Allies, int numAllies )
{
	if ( !AI_IsSinglePlayer() )
		return;
#if 0
	if( hl2_episodic.GetBool() && m_iszDenyCommandConcept != NULL_STRING )
	{
		SpeakCommandResponse( STRING(m_iszDenyCommandConcept) );
		return;
	}
#endif

	CHL2_Player *pPlayer = (CHL2_Player *)UTIL_GetLocalPlayer();

#if 0
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
#endif
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
	if ( !spoke && pClosest == this )
	{
#if 1
		if ( OkToShout() )
			Speak( TLK_SQUAD_ACK_GENERIC );
#else
		float destDistToPlayer = ( vecDest - pPlayer->GetAbsOrigin() ).Length();
		float destDistToClosest = ( vecDest - GetAbsOrigin() ).Length();
		CFmtStr modifiers( "commandpoint_dist_to_player:%.0f,"
						   "commandpoint_dist_to_npc:%.0f",
						   destDistToPlayer,
						   destDistToClosest );

		SpeakCommandResponse( TLK_COMMANDED, modifiers );
#endif
	}

#if 0
	m_OnStationOrder.FireOutput( this, this );
#endif
	BaseClass::MoveOrder( vecDest, Allies, numAllies );
}

//-----------------------------------------------------------------------------
void CHOEHumanFollower::OnMoveToCommandGoalFailed( void )
{
	// Clear the goal.
	ClearCommandGoal();

	// Announce failure.
//	SpeakCommandResponse( TLK_COMMAND_FAILED );
}

#endif // PLAYER_SQUAD_STUFF

AI_BEGIN_CUSTOM_NPC( hoe_human_follower, CHOEHumanFollower )
AI_END_CUSTOM_NPC()



//-----------------------------------------------------------------------------
int CHOEFollowBehavior::FollowCallBaseSelectSchedule( void )
{
	extern bool g_bSelectScheduleCheck;
	// Detect recursion due to the next CHOEHuman methods
	// calling BaseClass::SelectSchedule.
	if ( g_bSelectScheduleCheck )
	{
		Assert( 0 );
		DevWarning( "CHOEFollowBehavior::FollowCallBaseSelectSchedule RECURSION (%s)\n", GetOuter()->GetDebugName() );
		return SCHED_NONE;
	}

	int sched = SCHED_NONE;
	g_bSelectScheduleCheck = true;
	switch ( GetOuter()->GetState() )
	{
	case NPC_STATE_IDLE:
		sched = GetOuter()->SelectScheduleIdle();
		break;
	case NPC_STATE_ALERT:
		sched = GetOuter()->SelectScheduleAlert();
		break;
	case NPC_STATE_COMBAT:
		sched = GetOuter()->SelectScheduleCombat();
		break;
	}
	g_bSelectScheduleCheck = false;
	if ( sched != SCHED_NONE )
		return sched;
	return BaseClass::FollowCallBaseSelectSchedule();
}

//-----------------------------------------------------------------------------
int CHOEFollowBehavior::SelectSchedule( void )
{
	// The base class returns SCHED_RANGE_ATTACK1 but doesn't check the LOS
	// or AMMO conditions.  Just pick any combat schedule here.
	if ( HasCondition( COND_CAN_RANGE_ATTACK1 ) )
		return FollowCallBaseSelectSchedule();

	return BaseClass::SelectSchedule();
}

//-----------------------------------------------------------------------------
int CHOEFollowBehavior::TranslateScheduleHack( int scheduleType )
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

#if 0
//-----------------------------------------------------------------------------
int CHOEFollowBehavior::SelectSchedule()
{
#if 1
	int sched = GetDesiredSchedule();
	if ( sched != SCHED_NONE )
	{
		SetDesiredSchedule( SCHED_NONE );
		return sched;
	}
	return BaseClass::SelectSchedule();
#else
	// The base class also checks for possible range attack but wrongly
	// returns SCHED_RANGE_ATTACK1 (wrong because we might be in the
	// shot regulator rest interval for example).  Return SCHED_NONE
	// so we select an appropriate combat schedule.
	if ( HasCondition( COND_CAN_RANGE_ATTACK1 ) )
		return SCHED_NONE;

	if ( GetFollowTarget() )
	{
		if ( !GetFollowTarget()->IsAlive() )
		{
			// UNDONE: Comment about the recently dead player here?
			SetFollowTarget( NULL );
		}
		else if ( ShouldFollow() )
		{
			int result = SCHED_NONE;
			
			result = SelectScheduleManagePosition();
			if ( result != SCHED_NONE )
				return result;
			
			result = SelectScheduleFollowPoints();
			if ( result != SCHED_NONE )
				return result;
				
			result = SelectScheduleMoveToFormation();
			if ( result != SCHED_NONE )
				return result;
		}
	}
	else
	{
		// Should not have landed here. Follow target ent must have been destroyed
		NotifyChangeBehaviorStatus();
	}

	if ( HasCondition( COND_TARGET_MOVED_FROM_MARK ) )
	{
		m_TargetMonitor.SetMark( m_hFollowTarget, m_FollowNavGoal.targetMoveTolerance * 0.5 );
	}

	// This is the whole point of overriding the base class: the base class calls
	// CAI_BaseNPC::SelectSchedule which I don't want.
	return SCHED_NONE;
#endif
}
#endif