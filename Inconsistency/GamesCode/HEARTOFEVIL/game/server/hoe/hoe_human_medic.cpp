#include "cbase.h"
#include "ai_senses.h"
#include "npcevent.h"
#include "hoe_human_medic.h"
#include "schedule_hacks.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_DATADESC( CHOEHumanMedic )
	DEFINE_FIELD( m_flTimeHealedPlayer, FIELD_TIME ),
	DEFINE_USEFUNC( UseMedic ),
END_DATADESC()

//-----------------------------------------------------------------------------
// Animation events.
//-----------------------------------------------------------------------------
Animevent CHOEHumanMedic::AE_HMEDIC_SYRINGE_HELMET;
Animevent CHOEHumanMedic::AE_HMEDIC_SYRINGE_HAND;
Animevent CHOEHumanMedic::AE_HMEDIC_HEAL;

//-----------------------------------------------------------------------------
// Activities
//-----------------------------------------------------------------------------
Activity CHOEHumanMedic::ACT_HMEDIC_HEAL;
Activity CHOEHumanMedic::ACT_HMEDIC_HEAL_CROUCH;

//-----------------------------------------------------------------------------
int CHOEHumanMedic::SelectScheduleCombat( void )
{
	if ( HasCondition( COND_NEW_ENEMY ) &&
		 !EnemyIsBullseye() && // added to stop hiding from the dead-enemy bullseye
		 IsInSquad() && !IsSquadLeader() && !SquadAnyIdle() )
	{
		return SCHED_TAKE_COVER_FROM_ENEMY;
	}

	// This over-rides the default human code for occluded enemies because it establishes a line of fire
	// whereas medics are more evasive and care more about healing the wounded than attacking

	if ( HasCondition( COND_ENEMY_OCCLUDED ) )
	{
		if ( IsFollowingHuman() )
		{
			CHOEHuman *pHuman = HumanPointer( GetTarget() ); // safe because we know it's a human from above
			if ( !pHuman->IsAlive() || pHuman->HealthFraction() > 2.0/3.0 || pHuman->WasHealedRecently() )
			{
				pHuman->StopFollowing(); // FIXME: may have been following someone else
				StopFollowing();
				return SelectSchedule();
			}
			else
			{
				return SCHED_HUMAN_MEDIC_CHASE;
			}
		}
		else if ( IsFollowingPlayer() )
		{
			if ( GetTarget()->IsAlive() && HealthFraction( GetTarget() ) <= 2.0/3.0 )
			{
				return SCHED_HUMAN_MEDIC_CHASE;
			}
		}
		else if ( SquadGetWounded( true ) )
		{
			return SCHED_HUMAN_MEDIC_CHASE;
		}
		else if ( HasCondition( COND_CAN_RANGE_ATTACK2 ) && OccupyStrategySlotRange( SQUAD_SLOT_HUMAN_GRENADE1, SQUAD_SLOT_HUMAN_GRENADE2 ) )
		{
			return SCHED_RANGE_ATTACK2;
		}
		else
		{
			return SCHED_STANDOFF;
		}
	}

	return SCHED_NONE;
}

//-----------------------------------------------------------------------------
int CHOEHumanMedic::SelectScheduleHeal( void )
{
	if ( !IsFollowing() )
	{
		// If nearest friendly guy is injured, go over and heal him
		CBaseEntity *pHurt = FindHealTarget();
		if ( pHurt != NULL )
		{
			StartFollowing( pHurt );
			return SCHED_HUMAN_MEDIC_CHASE;
		}

		// Search for guy with biggest wounds in squad
		if ( SquadGetWounded( true ) )
		{
			return SCHED_HUMAN_MEDIC_CHASE;
		}
	}
	if ( IsFollowingHuman() )
	{
		CHOEHuman *pHuman = HumanPointer( GetTarget() ); // safe because we know it's a human from above
		if ( !pHuman->IsAlive() || pHuman->HealthFraction() > 2.0/3.0 || pHuman->WasHealedRecently() )
		{
			pHuman->StopFollowing(); // FIXME: may be following someone else
			StopFollowing();
			return SelectSchedule();
		}
		else
		{
			return SCHED_HUMAN_MEDIC_CHASE;
		}
	}
	else if ( IsFollowingPlayer() )
	{
		if ( GetTarget()->IsAlive() && HealthFraction( GetTarget() ) <= 2.0/3.0 )
		{
			return SCHED_HUMAN_MEDIC_CHASE;
		}
		StopFollowing();
		return SelectSchedule();
	}

	return SCHED_NONE;
}

//-----------------------------------------------------------------------------
int CHOEHumanMedic::SelectSchedulePriority( void )
{
	int sched;

	if ( GetState() == NPC_STATE_IDLE || GetState() == NPC_STATE_ALERT )
	{
		sched = SelectScheduleHeal();
		if ( sched != SCHED_NONE )
			return sched;
	}

	// Follow behavior will choose SCHED_RANGE_ATTACK1 if COND_RANGE_ATTACK1 is set.
	if ( GetState() == NPC_STATE_COMBAT )
	{
		if ( HasCondition( COND_NEW_ENEMY )
			 && !EnemyIsBullseye() // added to stop hiding from the dead-enemy bullseye
			 && IsInSquad() && !IsSquadLeader() && !SquadAnyIdle() )
		{
			return SCHED_TAKE_COVER_FROM_ENEMY;
		}
	}

	return BaseClass::SelectSchedulePriority();
}

static bool TaskRandomCheck( const Task_t *pTask )
{
	return pTask->flTaskData == 0 || random->RandomFloat(0, 1) <= pTask->flTaskData;
}

//-----------------------------------------------------------------------------
void CHOEHumanMedic::StartTask( const Task_t *pTask )
{
	switch( pTask->iTask )
	{
	case TASK_HUMAN_MEDIC_CHECK_TARGET:
		{
			CBaseEntity *pTarget = GetFollowTarget();
			if ( pTarget != NULL )
			{
				CHOEHuman *pHuman = HumanPointer( pTarget );

				// Make sure target is not dead etc
				if ( pTarget->IsPlayer() )
				{
					if ( pTarget->IsAlive() )
					{
						TaskComplete();
						break;
					}
					TaskFail( FAIL_NO_TARGET );
				}
				else if ( !pHuman )
				{
					TaskFail( FAIL_NO_TARGET );
				}
				else if ( !pHuman->OkForMedicToHealMe( this ) )
				{
					// If Target has an enemy, it's more useful to shoot the enemy than to heal target
					if ( pHuman->GetEnemy() != NULL && pHuman->HasCondition( COND_SEE_ENEMY ) )
						SetEnemy( pHuman->GetEnemy() );
					TaskFail( FAIL_NO_TARGET );
				}
				else
				{
					TaskComplete();
				}
			}
			else
			{
				TaskFail( FAIL_NO_TARGET );
			}
		}
		break;
	case TASK_HUMAN_MEDIC_SOUND_HEAL:
		{
			CBaseEntity *pTarget = GetFollowTarget();
			if ( pTarget != NULL )
			{
#if 1
				bool bSpoke = false;
				if ( /*TaskRandomCheck( pTask ) &&*/ OkToShout() )
				{
#ifdef HOE_HUMAN_RR
					SpeechSelection_t selection;
					if ( SelectSpeechResponse( TLK_HEAL, NULL, selection ) )
					{
						SetTalkTarget( pTarget );
						DispatchSpeechSelection( selection );
						if ( GetSpeechManager() )
						{
							GetSpeechManager()->ExtendSpeechCategoryTimer( SPEECH_CATEGORY_IDLE, 30 );
							GetSpeechManager()->ExtendSpeechCategoryTimer( SPEECH_CATEGORY_INJURY, 30 );
						}
						bSpoke = true;
					}
#else
					Speak( "HEAL", 0, pTarget );
#endif
				}
				if ( !bSpoke )
				{
					AddLookTarget( pTarget, 1.0, 2.0 );
				}

				CHOEHuman *pHuman = HumanPointer( pTarget );
				if ( pHuman && pHuman->OkForMedicToHealMe( this ) )
				{
					pHuman->DispatchInteraction( g_interactionHumanMedicHeal, NULL, this );
				}
#else
				CHuman * pTarget = m_hTargetEnt->MyHumanPointer();
				if (pTarget  && pTarget->SafeToChangeSchedule() && !pTarget->HasConditions( bits_COND_SEE_ENEMY ) ) 
				{
					pTarget->ChangeSchedule( pTarget->GetScheduleOfType( SCHED_HUMAN_WAIT_HEAL ) );
				}
#endif
			}
			TaskComplete();
		}
		break;
	default:
		BaseClass::StartTask( pTask );
		break;
	}
}

//-----------------------------------------------------------------------------
void CHOEHumanMedic::RunTask( const Task_t *pTask )
{
	switch( pTask->iTask )
	{
		case TASK_MOVE_TO_TARGET_RANGE:
		{
			// If we're moving to heal a target, and the target dies, or is healed, stop
			if ( IsCurSchedule( SCHED_HUMAN_MEDIC_CHASE ) &&
				 ( !GetFollowTarget() || !GetFollowTarget()->IsAlive() || HealthFraction( GetFollowTarget() ) > 2.0/3.0) )
			{
				TaskFail( FAIL_NO_TARGET );
				return;
			}

			BaseClass::RunTask( pTask );
			break;
		}
	default:
		BaseClass::RunTask( pTask );
		break;
	}
}

//-----------------------------------------------------------------------------
int CHOEHumanMedic::SelectFailSchedule( int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode )
{
	if ( failedSchedule == SCHED_HUMAN_MEDIC_CHASE )
	{
		if ( GetTarget() != NULL )
		{
			RememberUnreachable( GetTarget() );
			StopFollowing();
		}
		return SCHED_IDLE_STAND;
	}

	return BaseClass::SelectFailSchedule( failedSchedule, failedTask, taskFailCode );
}

//-----------------------------------------------------------------------------
void CHOEHumanMedic::HandleAnimEvent( animevent_t *pEvent )
{
	if ( pEvent->event == AE_HMEDIC_HEAL )
	{
		Heal();
		return;
	}
	if ( pEvent->event == AE_HMEDIC_SYRINGE_HAND )
	{
		SetBodygroup( HMEDIC_BODYGROUP_SYRINGE, 1 );
		return;
	}
	if ( pEvent->event == AE_HMEDIC_SYRINGE_HELMET )
	{
		SetBodygroup( HMEDIC_BODYGROUP_SYRINGE, 0 );
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
void CHOEHumanMedic::UseMedic( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	DevMsg("CHOEHumanMedic::UseFunc\n");
}

//-----------------------------------------------------------------------------
CBaseEntity *CHOEHumanMedic::FindHealTarget( void )
{
	CBaseEntity *pNearest = NULL;
	float flDistToClosest = FLT_MAX;

	for ( int i = 0; GetFriendClasses()[i] ; i++ )
	{
		const char *pszFriend = GetFriendClasses()[i];

		if ( HasMemory( bits_MEMORY_PROVOKED ) && !Q_strcmp( pszFriend, "player" ) )
			continue;

		CBaseEntity *pEntity = NULL;
		while ( ( pEntity = gEntList.FindEntityByClassnameWithin( pEntity, pszFriend, GetAbsOrigin(), HUMAN_SPEECH_RADIUS ) ) != 0 )
		{
			if ( pEntity == this || !pEntity->IsAlive() || HealthFraction( pEntity ) > 2.0/3.0 )
				continue;

			if ( !GetSenses()->DidSeeEntity( pEntity ) )
				continue;

			float flDist2 = (pEntity->GetAbsOrigin() - GetAbsOrigin()).LengthSqr();
			if ( flDistToClosest < flDist2 )
				continue;

			if ( IsUnreachable( pEntity ) )
				continue;

			if ( pEntity->IsPlayer() )
			{
				if ( TimeRecent( m_flTimeHealedPlayer, 20.0f ) )
				{
					// Heal the player if he/she/it has been standing and staring at us
					// but no more than every 5 seconds.
					if ( TimeRecent( m_flTimeHealedPlayer, 5.0f ) || GetTimePlayerStaring() < 2 )
						continue;
				}
			}
			else
			{
				CHOEHuman *pHuman = HumanPointer( pEntity );
				if ( pHuman && pHuman->WasHealedRecently() )
					continue;
			}

			pNearest = pEntity;
			flDistToClosest = flDist2;
		}
	}

	return pNearest;
}

//-----------------------------------------------------------------------------
void CHOEHumanMedic::Heal( void )
{
	CBaseEntity *pTarget = GetTarget();
	if ( pTarget == NULL )
		return;

	StopFollowing();

	Vector target = pTarget->GetAbsOrigin() - GetAbsOrigin();
	if ( target.Length() > 100 )
		return;

	// Heal player or human
	pTarget->TakeHealth( 25, DMG_GENERIC );

	if ( pTarget->IsPlayer() )
	{
		m_flTimeHealedPlayer = gpGlobals->curtime;
		return;
	}

	CHOEHuman *pHuman = HumanPointer( pTarget );
	if ( pHuman )
	{
		pHuman->m_hMedicThatHealedMe = this;
		pHuman->m_flLastMedicHealed = gpGlobals->curtime;
	}
}

//-----------------------------------------------------------------------------
Activity CHOEHumanMedic::NPC_TranslateActivity( Activity eNewActivity )
{
	eNewActivity = BaseClass::NPC_TranslateActivity( eNewActivity );

	bool bCrouch = IsCrouching();

	if ( eNewActivity == ACT_HMEDIC_HEAL )
	{
		if ( bCrouch ) eNewActivity = ACT_HMEDIC_HEAL_CROUCH;
	}

	return eNewActivity;
}

//-----------------------------------------------------------------------------
void CHOEHumanMedic::Event_Killed( const CTakeDamageInfo &info )
{
	if ( !HasAHead() )
		SetBodygroup( HMEDIC_BODYGROUP_SYRINGE, 2 );

	BaseClass::Event_Killed( info );
}

//-----------------------------------------------------------------------------
bool CHOEHumanMedic::ShouldBlockerMoveAway( CAI_BaseNPC *pBlocker )
{
	if ( IsCurSchedule( SCHED_HUMAN_MEDIC_CHASE ) && pBlocker != GetTarget() )
	{
		return true;
	}

	return BaseClass::ShouldBlockerMoveAway( pBlocker );
}

HOE_BEGIN_CUSTOM_NPC( hoe_human_medic, CHOEHumanMedic )

	DECLARE_TASK( TASK_HUMAN_MEDIC_SOUND_HEAL )
	DECLARE_TASK( TASK_HUMAN_MEDIC_CHECK_TARGET )

	DECLARE_SCHEDULE( SCHED_HUMAN_MEDIC_HEAL )
	DECLARE_SCHEDULE( SCHED_HUMAN_MEDIC_CHASE )

	DECLARE_ANIMEVENT( AE_HMEDIC_SYRINGE_HAND )
	DECLARE_ANIMEVENT( AE_HMEDIC_SYRINGE_HELMET )
	DECLARE_ANIMEVENT( AE_HMEDIC_HEAL )

	DECLARE_ACTIVITY( ACT_HMEDIC_HEAL )
	DECLARE_ACTIVITY( ACT_HMEDIC_HEAL_CROUCH )

	LOAD_SCHEDULES_FILE( npc_human_medic )

HOE_END_CUSTOM_NPC()
