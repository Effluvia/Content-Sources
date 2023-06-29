#include "cbase.h"
#include "hoe_human.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
CBaseCombatCharacter *CHOEHuman::GetSquadLeader( void )
{
	if ( IsInPlayerSquad() )
		return AI_GetSinglePlayer();

	return IsInSquad() ? GetSquad()->GetLeader() : NULL;
}

//-----------------------------------------------------------------------------
bool CHOEHuman::HasSquadLeader( void )
{
	return GetSquadLeader() != NULL;
}

//-----------------------------------------------------------------------------
bool CHOEHuman::IsSquadLeader( void )
{
	return GetSquadLeader() == this;
}

//-----------------------------------------------------------------------------
bool CHOEHuman::IsPlayerSquadLeader( void )
{
	return IsInPlayerSquad();
}

//-----------------------------------------------------------------------------
int CHOEHuman::SelectScheduleFromSquadCommand( void )
{
	if ( !IsAlive() || m_lifeState == LIFE_DYING ) return SCHED_NONE;

	switch ( m_nLastSquadCommand )
	{
	case SQUADCMD_RETREAT:
		if ( GetEnemy() == NULL || HasCondition( COND_ENEMY_OCCLUDED ) )
		{
			// I can't see anything bad so the retreat was successful.  Yay!
			m_nLastSquadCommand = SQUADCMD_NONE;
			
			if ( IsSquadLeader() )
			{
				// Now tell everyone to regroup for another attack
				return SCHED_HUMAN_SIGNAL_COME_TO_ME;
			}
		}
		else if ( !HasMemory( bits_MEMORY_HUMAN_NO_COVER ) )
		{
			return SCHED_HUMAN_RETREAT;
		}
		break;

	case SQUADCMD_COME_HERE:
		if ( IsFollowingPlayer() && GetFollowTargetDistance() > 64 ) 
		{
			return SCHED_HUMAN_FOLLOW_CLOSE;
		}
		else if	( IsFollowingHuman() && GetFollowTargetDistance() > 256  )
		{
			return SCHED_HUMAN_FOLLOW;
		}
		else
		{
			// I am now close to my target (or have stopped following)
			m_nLastSquadCommand = SQUADCMD_NONE;
			if ( IsFollowingHuman() )
				StopFollowing( );
		}
		break;

	case SQUADCMD_CHECK_IN:
		m_nLastSquadCommand = SQUADCMD_NONE;
		if ( !HasCondition( COND_SEE_ENEMY )  )	// If I have no enemy, give the all clear
		{
			if ( !IsSquadLeader() )
				return SCHED_HUMAN_CHECK_IN;
		}
		else	// If I have an enemy, inform my squad
		{
			return SCHED_HUMAN_FOUND_ENEMY;
		}
		break;

	case SQUADCMD_DEFENSE:
		m_nLastSquadCommand = SQUADCMD_NONE;
		if ( !SquadGetWounded() )
		{
			// There is no wounded, so defense was successful.  Yay!  Or he could have died, of course.
			if ( IsFollowingHuman() )
				StopFollowing( );
		}
		else
		{
			return SCHED_TARGET_CHASE;
		}
		break;

	case SQUADCMD_ATTACK:
#if 1
#else
		if ( ( GetEnemy() != NULL && GetEnemy()->IsAlive() ) || SquadGetCommanderEnemy() )	// Try and get new enemy
		{
			if ( HasCondition( COND_CAN_MELEE_ATTACK1 ) )
			{
				return SCHED_MELEE_ATTACK1;
			}
			else if ( HasCondition( COND_CAN_RANGE_ATTACK1 ) &&
				OccupyStrategySlotRange( SQUAD_SLOT_HUMAN_ENGAGE1, SQUAD_SLOT_HUMAN_ENGAGE2 ) )
			{
				return SCHED_RANGE_ATTACK1;
			}
			else if ( HasCondition( COND_CAN_RANGE_ATTACK2 ) &&
				OccupyStrategySlotRange( SQUAD_SLOT_HUMAN_GRENADE1, SQUAD_SLOT_HUMAN_GRENADE2 ) )
			{
				return SCHED_RANGE_ATTACK2;
			}
			else
			{
				return SCHED_HUMAN_ESTABLISH_LINE_OF_FIRE;
			}
		}
#endif
		// Squad Commander has no enemy, so attack was successful.  Yay!
		m_nLastSquadCommand = SQUADCMD_NONE;
		break;

	case SQUADCMD_SEARCH_AND_DESTROY:
		if ( GetEnemy() != NULL && GetEnemy()->IsAlive() )	// If I have an enemy then go after him
		{
			return SCHED_HUMAN_ESTABLISH_LINE_OF_FIRE;
		}
		else if ( HasCondition( COND_HEAR_COMBAT ) )
		{
			// From CNPC_Combine::SelectSchedule
			CSound *pSound = GetBestSound();
			if ( pSound /* && pSound->IsSoundType( SOUND_COMBAT ) */ &&
				IsInSquad() && GetSquad()->GetSquadMemberNearestTo( pSound->GetSoundReactOrigin() ) == this &&
				OccupyStrategySlot( SQUAD_SLOT_INVESTIGATE_SOUND ) )
			{
				return SCHED_INVESTIGATE_SOUND;
			}
		}
		else
		{
			return SCHED_HUMAN_SEARCH_AND_DESTROY;
		}
		break;

	case SQUADCMD_SURPRESSING_FIRE:
		if ( GetEnemyLKP() != vec3_origin && FVisible( GetEnemyLKP() ) )
		{
			if ( NoFriendlyFire() && OccupyStrategySlotRange( SQUAD_SLOT_HUMAN_ENGAGE1, SQUAD_SLOT_HUMAN_ENGAGE2 ) )
			{
				return SCHED_HUMAN_SURPRESS;
			}
			else if ( HasCondition( COND_CAN_RANGE_ATTACK2 ) && m_fHandGrenades && 
				OccupyStrategySlotRange( SQUAD_SLOT_HUMAN_GRENADE1, SQUAD_SLOT_HUMAN_GRENADE2 ) )
			{
				return SCHED_RANGE_ATTACK2;
			}
			else
			{
				// If there is someone in the way then give up
				m_nLastSquadCommand = SQUADCMD_NONE;
			}
		}
		else
		{
			return SCHED_HUMAN_MOVE_TO_ENEMY_LKP;
		}
		break;
	}

	return SCHED_NONE;
}

//-----------------------------------------------------------------------------
bool CHOEHuman::NoFriendlyFire( void )
{
	if ( GetEnemy() == NULL )
		return true;

	Vector targetPos = GetEnemy()->BodyTarget( GetAbsOrigin(), false );
	return CurrentWeaponLOSCondition( targetPos, false );
}

//-----------------------------------------------------------------------------
void CHOEHuman::SquadIssueCommand( SquadCommand Cmd )
{
	CAI_Squad *pSquad = GetSquad();

	if ( pSquad == NULL )
		return;

	if ( IsPlayerSquadLeader() && !SquadCmdLegalForNonLeader( Cmd ) ) 
		return;

	CAI_BaseNPC *pSquadLeader = pSquad->GetLeader();
	if ( !pSquadLeader ) return;

	CHOEHuman *pHumanLeader = HumanPointer( pSquadLeader );
	if ( !pHumanLeader ) return;

	// Squad Leader refuses your request if the command has a lower priority than the one he is currently on
	if ( Cmd < pHumanLeader->m_nLastSquadCommand ) return;

	switch ( Cmd )
	{
	case SQUADCMD_NONE:					DevMsg( "%s Issuing Squad Command: None\n", GetDebugName() ); break;
	case SQUADCMD_CHECK_IN:				DevMsg( "%s Issuing Squad Command: Check In\n", GetDebugName() ); break;
	case SQUADCMD_SEARCH_AND_DESTROY:	DevMsg( "%s Issuing Squad Command: Search and Destroy\n", GetDebugName() ); break;
	case SQUADCMD_OUTTA_MY_WAY:			DevMsg( "%s Issuing Squad Command: Outta my way\n", GetDebugName() ); break;
	case SQUADCMD_FOUND_ENEMY:			DevMsg( "%s Issuing Squad Command: Found Enemy\n", GetDebugName() ); break;
	case SQUADCMD_DISTRESS:				DevMsg( "%s Issuing Squad Command: Distress\n", GetDebugName() ); break;
	case SQUADCMD_COME_HERE:			DevMsg( "%s Issuing Squad Command: Come here\n", GetDebugName() ); break;
	case SQUADCMD_SURPRESSING_FIRE:		DevMsg( "%s Issuing Squad Command: Surpressing Fire\n", GetDebugName() ); break;
	case SQUADCMD_ATTACK:				DevMsg( "%s Issuing Squad Command: Attack\n", GetDebugName() ); break;
	case SQUADCMD_DEFENSE:				DevMsg( "%s Issuing Squad Command: Defense\n", GetDebugName() ); break;
	case SQUADCMD_RETREAT:				DevMsg( "%s Issuing Squad Command: Retreat\n", GetDebugName() ); break;
	case SQUADCMD_GET_DOWN:				DevMsg( "%s Issuing Squad Command: Get Down\n", GetDebugName() ); break;
	case SQUADCMD_BEHIND_YOU:			DevMsg( "%s Issuing Squad Command: Behind You\n", GetDebugName() ); break;
	}

	// Make sure the squad leader remembers which command he issued, even if nobody is in a position
	// to follow the command, otherwise it may be issued repeatedly
	pHumanLeader->m_nLastSquadCommand = Cmd;
	pHumanLeader->m_flLastSquadCmdTime = gpGlobals->curtime;

	
	AISquadIter_t iter;
	CAI_BaseNPC *pSquadmate = m_pSquad->GetFirstMember( &iter );
	while ( pSquadmate )
	{
		if ( (pSquadmate->GetAbsOrigin() - GetAbsOrigin()).Length() <= SQUAD_COMMAND_RANGE )
		{
			CHOEHuman *pHuman = HumanPointer( pSquadmate );
			if ( pHuman )
			{
				pHuman->SquadReceiveCommand( Cmd );
			}
		}
		pSquadmate = pSquad->GetNextMember( &iter );
	}
}

//-----------------------------------------------------------------------------
void CHOEHuman::SquadReceiveCommand( SquadCommand Cmd )
{
	if ( !IsAlive() || m_lifeState == LIFE_DYING )
		return;

	m_fSquadCmdAcknowledged = false;
	SetCondition( COND_PROVOKED );

	switch ( Cmd )
	{
	case SQUADCMD_ATTACK:				// Attack a designated target, if you are not attacking something else
		m_nLastSquadCommand = Cmd;
		m_flLastSquadCmdTime = gpGlobals->curtime;
		if ( !HasCondition( COND_SEE_ENEMY ) )
		{
			if ( SquadGetCommanderEnemy() )
			{
				if ( !IsSquadLeader() && OkToShout() )
				{
#ifdef HOE_HUMAN_RR
					Speak( TLK_SQUAD_ACK_GENERIC );
#else
					Speak( "AFFIRMATIVE" );
#endif
					m_fSquadCmdAcknowledged = TRUE;
				}
			}
			else
				SquadReceiveCommand( SQUADCMD_SEARCH_AND_DESTROY ); // If I can't, try to search for a new enemy
		}
		break;
	case SQUADCMD_SEARCH_AND_DESTROY:	// Search for a target to attack
		m_nLastSquadCommand = Cmd;
		m_flLastSquadCmdTime = gpGlobals->curtime;
		break;
	case SQUADCMD_DEFENSE:				// Defend a designated squad member (i.e. someone who is wounded)
		m_nLastSquadCommand = Cmd;
		m_flLastSquadCmdTime = gpGlobals->curtime;
		break;
	case SQUADCMD_RETREAT:				// Run away from whatever you are attacking
		m_nLastSquadCommand = Cmd;
		m_flLastSquadCmdTime = gpGlobals->curtime;
		break;
	case SQUADCMD_SURPRESSING_FIRE:		// Fire at a designated place
#if 0 // FIXME
		if ( !HasCondition( COND_SEE_ENEMY ) )
		{
			if ( SquadGetCommanderEnemy() ||		// Try and get new enemy
				 SquadGetCommanderLineOfSight() )	// Fire at the position my commander is looking at
			{
				m_nLastSquadCommand = Cmd;
				m_flLastSquadCmdTime = gpGlobals->curtime;
			}
		}
#endif
		break;
	case SQUADCMD_COME_HERE:			// Come to Squad Leader
		m_nLastSquadCommand = Cmd;
		m_flLastSquadCmdTime = gpGlobals->curtime;
		if ( IsInSquad() && !IsFollowingPlayer() ) 
		{
			StartFollowing( GetSquadLeader() );
		}
		if ( !HasCondition( COND_SEE_ENEMY) )
		{
			if ( OkToShout() )
			{
#ifdef HOE_HUMAN_RR
				Speak( TLK_SQUAD_ACK_GENERIC );
#else
				Speak( "AFFIRMATIVE" );
#endif
				m_fSquadCmdAcknowledged = TRUE;
			}
			// If not fighting, dump out of current schedule
			// SetCondition( COND_SQUAD_INTERRUPT );
		}
		break;
	case SQUADCMD_GET_DOWN:				// Crouch	- likely to be only given by player
		break;
	case SQUADCMD_BEHIND_YOU:			// Turn around - likely to be only given by player
		break;
	case SQUADCMD_CHECK_IN:				// Tell everyone to check in
		m_nLastSquadCommand = Cmd;
		m_flLastSquadCmdTime = gpGlobals->curtime;
		break;
	case SQUADCMD_FOUND_ENEMY:
		break;
	case SQUADCMD_DISTRESS:
		break;
	}
}

//-----------------------------------------------------------------------------
bool CHOEHuman::SquadCmdLegalForNonLeader( SquadCommand Cmd )
{
	if ( Cmd == SQUADCMD_FOUND_ENEMY ||
		 Cmd == SQUADCMD_DEFENSE || 
		 Cmd == SQUADCMD_DISTRESS ) return true;
	
	return false;
}

//-----------------------------------------------------------------------------
bool CHOEHuman::SquadAnyIdle( void )
{
	if( !HasSquadLeader() )
		return false;

	AISquadIter_t iter;
	CAI_BaseNPC *pSquadmate = m_pSquad->GetFirstMember( &iter );
	while ( pSquadmate )
	{
		if ( pSquadmate != this && pSquadmate->IsAlive() &&
			( pSquadmate->GetState() == NPC_STATE_IDLE || pSquadmate->GetState() == NPC_STATE_ALERT ) &&
			pSquadmate->FVisible( this ) )
		{
			return true;
		}
		pSquadmate = m_pSquad->GetNextMember( &iter );
	}
	return false;
}

//-----------------------------------------------------------------------------
bool CHOEHuman::SquadIsHealthy( void )
{
	if ( !HasSquadLeader() )
		return HealthFraction() > 2.0/3.0;

	int totalHealth = 0;

	AISquadIter_t iter;
	CAI_BaseNPC *pSquadmate = m_pSquad->GetFirstMember( &iter );
	while ( pSquadmate )
	{
		// If one member of the squad is very sick (lower than 33%) then squad is not healthy
		if ( HealthFraction(pSquadmate) < 1.0/3.0 )
		{
			return false;
		}
		totalHealth += pSquadmate->GetHealth();
		pSquadmate = m_pSquad->GetNextMember( &iter );
	}

	// If average health of each member is higher than 66% then squad is healthy
	return totalHealth > ( GetMaxHealth() * GetSquad()->NumMembers() ) * 2.0 / 3.0;
}

//-----------------------------------------------------------------------------
bool CHOEHuman::SquadIsScattered( void )
{
	if ( !HasSquadLeader() )
		return false;

	float flTotalDist = 0;

	AISquadIter_t iter;
	CAI_BaseNPC *pSquadmate = m_pSquad->GetFirstMember( &iter );
	while ( pSquadmate )
	{
		float flDist = (pSquadmate->GetAbsOrigin() - GetAbsOrigin()).Length();

		// If one member of the squad is very far away (further than 768) then squad is scattered
		if ( flDist > 768 ) return true;

		flTotalDist += flDist;
		pSquadmate = m_pSquad->GetNextMember( &iter );
	}

	// If average distance of each member is further than 512 then squad is scattered
	return flTotalDist > 512 * GetSquad()->NumMembers();
}

//-----------------------------------------------------------------------------
bool CHOEHuman::SquadGetCommanderEnemy( void )
{
	if ( IsPlayerSquadLeader() )
	{
		// Try and figure out what the player's enemy is
		return false;
	}

	if ( IsInSquad() )
	{
		// Get Squad Leader's enemy
		CBaseCombatCharacter *pSquadLeader = GetSquadLeader();
		if ( pSquadLeader == NULL )
			return false;

		CHOEHuman *pHumanLeader = HumanPointer( pSquadLeader );
		if ( pHumanLeader == NULL )
			return false;

		if ( pHumanLeader->GetEnemy() == NULL )
			return false;
		SetEnemy( pHumanLeader->GetEnemy() );

		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
bool CHOEHuman::SquadGetCommanderLineOfSight( void )
{
	return false;
}

//-----------------------------------------------------------------------------
bool CHOEHuman::SquadGetWounded( bool bMedic )
{
	if ( IsInSquad() && !IsFollowingPlayer() )
	{
		CAI_BaseNPC *pWounded = NULL;

		AISquadIter_t iter;
		CAI_BaseNPC *pSquadmate;
		for ( pSquadmate = m_pSquad->GetFirstMember( &iter );
			pSquadmate != NULL;
			pSquadmate = m_pSquad->GetNextMember( &iter ))
		{
			if ( pSquadmate == this || !pSquadmate->IsAlive() )
				continue;

			if ( IsUnreachable( pSquadmate ) )
				continue;

			if ( (pSquadmate->GetAbsOrigin() - GetAbsOrigin()).LengthSqr() > Square( 768.0f ) )
				continue;

			if ( bMedic && HumanPointer( pSquadmate) &&
				HumanPointer( pSquadmate)->WasHealedRecently() )
				continue;

			// Don't chase after a guy who is in active combat since the medic
			// won't heal him in that case anyway.
			if ( bMedic && pSquadmate->HasCondition( COND_SEE_ENEMY ) )
				continue;

			if ( pWounded == NULL && HealthFraction( pSquadmate ) <= 2.0/3.0 )
				pWounded = pSquadmate;
			else if ( pWounded != NULL && ( pWounded->GetHealth() > pSquadmate->GetHealth() ) )
				pWounded = pSquadmate;
		}

		if ( pWounded != NULL )
		{
			StartFollowing( pWounded );
			return true;
		}
	}
	return false;
}
