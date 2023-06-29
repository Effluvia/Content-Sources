#include "cbase.h"
#include "hoe_behavior_passenger.h"
#include "saverestore_utlvector.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar passenger_debug_entry;
extern ConVar passenger_debug_transition;

#define	PASSENGER_NEAR_VEHICLE_THRESHOLD	64.0f

BEGIN_DATADESC( CHOEHumanPassengerBehavior )
	DEFINE_UTLVECTOR( m_FailedEntryPositions, FIELD_EMBEDDED ),
	DEFINE_FIELD( m_flEntranceUpdateTime, FIELD_TIME ),
	DEFINE_FIELD( m_flNextEnterAttempt, FIELD_TIME ),
	DEFINE_FIELD( m_sRoleName, FIELD_STRING ),
END_DATADESC();

BEGIN_SIMPLE_DATADESC( dupFailPosition_t )
	DEFINE_FIELD( vecPosition, FIELD_VECTOR ),
	DEFINE_FIELD( flTime, FIELD_TIME ),
END_DATADESC();

//-----------------------------------------------------------------------------
CHOEHumanPassengerBehavior::CHOEHumanPassengerBehavior( void ) : 
	m_flNextEnterAttempt( 0.0f )
{
}

//-----------------------------------------------------------------------------
void CHOEHumanPassengerBehavior::Enable( CBaseEntity *pVehicle, bool bImmediateEnter /*= false*/ )
{
	BaseClass::Enable( pVehicle );

	// See if we want to sit in the vehicle immediately
	if ( bImmediateEnter )
	{
		// Find the seat and sit in it
		if ( ReserveEntryPoint( VEHICLE_SEAT_ANY ) )
		{
			// Attach
			AttachToVehicle();

			// This will slam us into the right position and clean up
			FinishEnterVehicle();
			GetOuter()->AddEffects( EF_NOINTERP );

			// Start our schedule immediately
			ClearSchedule( "Immediate entry to vehicle" );

			// Start us playing the correct sequence
			int iSequence = FindIdleSequence();
			if ( iSequence != -1 )
			{
				GetOuter()->ResetSequence( iSequence );
//				GetOuter()->m_iszSceneCustomMoveSeq = AllocPooledString( GetOuter()->GetSequenceName( iSequence ) );
//				GetOuter()->SetIdealActivity( ACT_SCRIPT_CUSTOM_MOVE );
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Deal with enter/exit of the vehicle
//-----------------------------------------------------------------------------
int	CHOEHumanPassengerBehavior::SelectTransitionSchedule( void )
{
	// Entering schedule
	if ( HasCondition( COND_PASSENGER_ENTERING ) || m_PassengerIntent == PASSENGER_INTENT_ENTER )
	{
		if ( GetPassengerState() == PASSENGER_STATE_INSIDE )
		{
			ClearCondition( COND_PASSENGER_ENTERING );
			m_PassengerIntent = PASSENGER_INTENT_NONE;
			return SCHED_NONE;
		}

		// Don't attempt to enter for a period of time
		if ( m_flNextEnterAttempt > gpGlobals->curtime )
			return SCHED_NONE;

		ClearCondition( COND_PASSENGER_ENTERING );

		// Failing that, run to the right place
		return SCHED_PASSENGER_RUN_TO_ENTER_VEHICLE_HOE;
	}

	return BaseClass::SelectTransitionSchedule();
}

//-----------------------------------------------------------------------------
// Purpose: Select schedules when we're riding in the car
//-----------------------------------------------------------------------------
int CHOEHumanPassengerBehavior::SelectScheduleInsideVehicle( void )
{
	return SCHED_NONE;
}

//-----------------------------------------------------------------------------
// Purpose: Select schedules while we're outside the car
//-----------------------------------------------------------------------------
int CHOEHumanPassengerBehavior::SelectScheduleOutsideVehicle( void )
{
	// FIXME: How can we get in here?
	Assert( m_hVehicle );
	if ( m_hVehicle == NULL )
		return SCHED_NONE;

	// If we want to get in, the try to do so
	if ( m_PassengerIntent == PASSENGER_INTENT_ENTER )
	{
		// If we're not attempting to enter the vehicle again, just fall to the base class
		if ( m_flNextEnterAttempt > gpGlobals->curtime )
			return BaseClass::SelectSchedule();

		// Otherwise try and enter thec car
		return SCHED_PASSENGER_RUN_TO_ENTER_VEHICLE_HOE;
	}
	
	// This means that we're outside the vehicle with no intent to enter, which should have disabled us!
	Disable();
	Assert( 0 );

	return SCHED_NONE;
}

//-----------------------------------------------------------------------------
// Purpose: Overrides the schedule selection
// Output : int - Schedule to play
//-----------------------------------------------------------------------------
int CHOEHumanPassengerBehavior::SelectSchedule( void )
{
	// First, keep track of our transition state (enter/exit)
	int nSched = SelectTransitionSchedule();
	if ( nSched != SCHED_NONE )
		return nSched;

	// Handle schedules based on our passenger state
	if ( GetPassengerState() == PASSENGER_STATE_OUTSIDE )
	{
		nSched = SelectScheduleOutsideVehicle();
		if ( nSched != SCHED_NONE )
			return nSched;
	}
	else if ( GetPassengerState() == PASSENGER_STATE_INSIDE )
	{
		nSched = SelectScheduleInsideVehicle();
		if ( nSched != SCHED_NONE )
			return nSched;
	}

	return BaseClass::SelectSchedule();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CHOEHumanPassengerBehavior::SelectFailSchedule( int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode )
{
	switch( failedTask )
	{
	case TASK_GET_PATH_TO_VEHICLE_ENTRY_POINT_HOE:
		{
			// This is not allowed!
			if ( GetPassengerState() != PASSENGER_STATE_OUTSIDE )
			{
				Assert( 0 );
				return SCHED_FAIL;
			}

			// If we're not close enough, then get nearer the target
			if ( UTIL_DistApprox( m_hVehicle->GetAbsOrigin(), GetOuter()->GetAbsOrigin() ) > PASSENGER_NEAR_VEHICLE_THRESHOLD )
				return SCHED_PASSENGER_RUN_TO_ENTER_VEHICLE_FAILED_HOE;
		}
		
		// Fall through

	case TASK_GET_PATH_TO_NEAR_VEHICLE_HOE:
		m_flNextEnterAttempt = gpGlobals->curtime + 3.0f;
		break;
	}

	return BaseClass::SelectFailSchedule( failedSchedule, failedTask, taskFailCode );
}

//-----------------------------------------------------------------------------
// Purpose: Adds a failed position to the list and marks when it occurred
// Input  : &vecPosition - Position that failed
//-----------------------------------------------------------------------------
void CHOEHumanPassengerBehavior::MarkVehicleEntryFailed( const Vector &vecPosition )
{
	dupFailPosition_t failPos;
	failPos.flTime = gpGlobals->curtime;
	failPos.vecPosition = vecPosition;
	m_FailedEntryPositions.AddToTail( failPos );

	// Show this as failed
	if ( passenger_debug_entry.GetBool() )
	{
		NDebugOverlay::Box( vecPosition, -Vector(8,8,8), Vector(8,8,8), 255, 0, 0, 0, 2.0f );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Determines if the passenger should take a radial route to the goal
// Input  : &vecEntryPoint - Point of entry
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CHOEHumanPassengerBehavior::UseRadialRouteToEntryPoint( const Vector &vecEntryPoint )
{
	// Get the center position of the vehicle we'll radiate around
	Vector vecCenterPos = m_hVehicle->WorldSpaceCenter();
	vecCenterPos.z = vecEntryPoint.z;

	// Find out if we need to go around the vehicle 
	float flDistToVehicleCenter = ( vecCenterPos - GetOuter()->GetAbsOrigin() ).Length();
	float flDistToGoal = ( vecEntryPoint - GetOuter()->GetAbsOrigin() ).Length();
	if ( flDistToGoal > flDistToVehicleCenter )
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Find the arc in degrees to reach our goal position
// Input  : &vecCenterPoint - Point around which the arc rotates
//			&vecEntryPoint - Point we're trying to reach
//			&bClockwise - If we should move clockwise or not to get there
// Output : float - degrees around arc to follow
//-----------------------------------------------------------------------------
float CHOEHumanPassengerBehavior::GetArcToEntryPoint( const Vector &vecCenterPoint, const Vector &vecEntryPoint, bool &bClockwise )
{
	// We want the entry point to be at the same level as the center to make this a two dimensional problem
	Vector vecEntryPointAdjusted = vecEntryPoint;
	vecEntryPointAdjusted.z = vecCenterPoint.z;

	// Direction from vehicle center to passenger
	Vector vecVehicleToPassenger = ( GetOuter()->GetAbsOrigin() - vecCenterPoint );
	VectorNormalize( vecVehicleToPassenger );

	// Direction from vehicle center to entry point
	Vector vecVehicleToEntry = ( vecEntryPointAdjusted - vecCenterPoint );
	VectorNormalize( vecVehicleToEntry );

	float flVehicleToPassengerYaw = UTIL_VecToYaw( vecVehicleToPassenger );
	float flVehicleToEntryYaw = UTIL_VecToYaw( vecVehicleToEntry );
	float flArcDist = UTIL_AngleDistance( flVehicleToEntryYaw, flVehicleToPassengerYaw );

	bClockwise = ( flArcDist < 0.0f );
	return fabs( flArcDist );
}

//-----------------------------------------------------------------------------
// Purpose: See if a vector is near enough to a previously failed position
// Input  : &vecPosition - position to test
// Output : Returns true if the point is near enough another to be considered equivalent
//-----------------------------------------------------------------------------
bool CHOEHumanPassengerBehavior::PointIsWithinEntryFailureRadius( const Vector &vecPosition )
{
	// Test this point against our known failed points and reject it if it's too near
	for ( int i = 0; i < m_FailedEntryPositions.Count(); i++ )
	{
		// If our time has expired, kill the position
		if ( ( gpGlobals->curtime - m_FailedEntryPositions[i].flTime ) > 3.0f )
		{
			// Show that we've cleared it
			if ( passenger_debug_entry.GetBool() )
			{
				NDebugOverlay::Box( m_FailedEntryPositions[i].vecPosition, -Vector(12,12,12), Vector(12,12,12), 255, 255, 0, 0, 2.0f );
			}

  			m_FailedEntryPositions.Remove( i );
			continue;
		}

		// See if this position is too near our last failed attempt
		if ( ( vecPosition - m_FailedEntryPositions[i].vecPosition ).LengthSqr() < Square(3*12) )
		{
			// Show that this was denied
			if ( passenger_debug_entry.GetBool() )
			{
				NDebugOverlay::Box( m_FailedEntryPositions[i].vecPosition, -Vector(12,12,12), Vector(12,12,12), 255, 0, 0, 128, 2.0f );
			}

			return true;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Find the proper sequence to use (weighted by priority or distance from current position)
//			to enter the vehicle.
// Input  : bNearest - Use distance as the criteria for a "best" sequence.  Otherwise the order of the
//					   seats is their priority.
// Output : int - sequence index
//-----------------------------------------------------------------------------
int CHOEHumanPassengerBehavior::FindEntrySequence( bool bNearest /*= false*/ )
{
	// Get a list of all our animations
	const PassengerSeatAnims_t *pEntryAnims = m_hVehicle->GetServerVehicle()->NPC_GetPassengerSeatAnims( GetOuter(), PASSENGER_SEAT_ENTRY );
	if ( pEntryAnims == NULL )
		return -1;

	// Get the ultimate position we'll end up at
	Vector	vecStartPos, vecEndPos;
	if ( m_hVehicle->GetServerVehicle()->NPC_GetPassengerSeatPosition( GetOuter(), &vecEndPos, NULL ) == false )
		return -1;

	const CPassengerSeatTransition *pTransition;
	float	flNearestDistSqr = FLT_MAX;
	float	flSeatDistSqr;
	int		nNearestSequence = -1;
	int		nSequence;

	// Test each animation (sorted by priority) for the best match
	for ( int i = 0; i < pEntryAnims->Count(); i++ )
	{
		// Find the activity for this animation name
		pTransition = &pEntryAnims->Element(i);
		nSequence = GetOuter()->LookupSequence( STRING( pTransition->GetAnimationName() ) );
		if ( nSequence == -1 )
			continue;

		// Test this entry for validity
		if ( GetEntryPoint( nSequence, &vecStartPos ) == false )
			continue;

		// See if this entry position is in our list of known unreachable places
		if ( PointIsWithinEntryFailureRadius( vecStartPos ) )
			continue;

		// Check to see if we can use this
		if ( IsValidTransitionPoint( vecStartPos, vecEndPos ) )
		{
			// If we're just looking for the first, we're done
			if ( bNearest == false )
				return nSequence;

			// Otherwise distance is the deciding factor
			flSeatDistSqr = ( vecStartPos - GetOuter()->GetAbsOrigin() ).LengthSqr();

			// Closer, take it
			if ( flSeatDistSqr < flNearestDistSqr )
			{
				flNearestDistSqr = flSeatDistSqr;
				nNearestSequence = nSequence;
			}
		}

	}

	return nNearestSequence;
}

//-----------------------------------------------------------------------------
// Purpose: Tries to build a route to the entry point of the target vehicle.
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CHOEHumanPassengerBehavior::FindPathToVehicleEntryPoint( void )
{
	// Set our custom move name
	// bool bFindNearest = ( GetOuter()->m_NPCState == NPC_STATE_COMBAT || GetOuter()->m_NPCState == NPC_STATE_ALERT );
	bool bFindNearest = true;	// For the sake of quick gameplay, just make Alyx move directly!
	int nSequence = FindEntrySequence( bFindNearest );
	if ( nSequence  == -1 )
		return false;

	// We have to do this specially because the activities are not named
	SetTransitionSequence( nSequence );

	// Get the entry position
	Vector vecEntryPoint;
	QAngle vecEntryAngles;
	if ( GetEntryPoint( m_nTransitionSequence, &vecEntryPoint, &vecEntryAngles ) == false )
	{
		MarkVehicleEntryFailed( vecEntryPoint );
		return false;
	}

	// If we're already close enough, just succeed
	float flDistToGoalSqr = ( GetOuter()->GetAbsOrigin() - vecEntryPoint ).LengthSqr();
	if ( flDistToGoalSqr < Square(3*12) )
		return true;

	// Setup our goal
	AI_NavGoal_t goal( GOALTYPE_LOCATION );
	// goal.arrivalActivity = ACT_SCRIPT_CUSTOM_MOVE;
	goal.dest = vecEntryPoint;

	// See if we need a radial route around the car, to our goal
	if ( UseRadialRouteToEntryPoint( vecEntryPoint ) )
	{
		// Find the bounding radius of the vehicle
		Vector vecCenterPoint = m_hVehicle->WorldSpaceCenter();
		vecCenterPoint.z = vecEntryPoint.z;
		bool bClockwise;
		float flArc = GetArcToEntryPoint( vecCenterPoint, vecEntryPoint, bClockwise );
		float flRadius = m_hVehicle->CollisionProp()->BoundingRadius2D();

		// Try and set a radial route
		if ( GetOuter()->GetNavigator()->SetRadialGoal( vecEntryPoint, vecCenterPoint, flRadius, flArc, 64.0f, bClockwise ) == false )
		{
			// Try the opposite way
			flArc = 360.0f - flArc;

			// Try the opposite way around
			if ( GetOuter()->GetNavigator()->SetRadialGoal( vecEntryPoint, vecCenterPoint, flRadius, flArc, 64.0f, !bClockwise ) == false )
			{
				// Try and set a direct route as a last resort
				if ( GetOuter()->GetNavigator()->SetGoal( goal ) == false )
					return false;
			}
		}

		// We found a goal
		GetOuter()->GetNavigator()->SetArrivalDirection( vecEntryAngles );
		GetOuter()->GetNavigator()->SetArrivalSpeed( 64.0f );
		return true;
	}
	else
	{
		// Try and set a direct route
		if ( GetOuter()->GetNavigator()->SetGoal( goal ) )
		{
			GetOuter()->GetNavigator()->SetArrivalDirection( vecEntryAngles );
			GetOuter()->GetNavigator()->SetArrivalSpeed( 64.0f );
			return true;
		}
	}

	// We failed, so remember it
	MarkVehicleEntryFailed( vecEntryPoint );
	return false;
}

//-----------------------------------------------------------------------------
Activity CHOEHumanPassengerBehavior::NPC_TranslateActivity( Activity activity )
{
	Activity nNewActivity = BaseClass::NPC_TranslateActivity( activity );

	if ( GetPassengerState() == PASSENGER_STATE_INSIDE )
	{
		if ( activity == ACT_IDLE )
			return (Activity) ACT_PASSENGER_IDLE;
	}

	return nNewActivity;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHOEHumanPassengerBehavior::StartTask( const Task_t *pTask )
{
	// We need to override these so we never face
	if ( GetPassengerState() == PASSENGER_STATE_INSIDE )
	{
		if ( pTask->iTask == TASK_FACE_TARGET || 
			 pTask->iTask == TASK_FACE_ENEMY ||
			 pTask->iTask == TASK_FACE_IDEAL ||
			 pTask->iTask == TASK_FACE_HINTNODE ||
			 pTask->iTask == TASK_FACE_LASTPOSITION ||
			 pTask->iTask == TASK_FACE_PATH ||
			 pTask->iTask == TASK_FACE_PLAYER ||
			 pTask->iTask == TASK_FACE_REASONABLE ||
			 pTask->iTask == TASK_FACE_SAVEPOSITION ||
			 pTask->iTask == TASK_FACE_SCRIPT )
		{
			return TaskComplete();
		}
	}

	switch ( pTask->iTask )
	{
	case TASK_RUN_TO_VEHICLE_ENTRANCE_HOE:
		{
			// Get a move on!
			GetOuter()->GetNavigator()->SetMovementActivity( ACT_RUN );
		}
		break;

	case TASK_GET_PATH_TO_VEHICLE_ENTRY_POINT_HOE:
		{
			if ( GetPassengerState() != PASSENGER_STATE_OUTSIDE )
			{
				Assert( 0 );
				TaskFail( "Trying to run while inside a vehicle!\n");
				return;
			}

			// Reserve an entry point
			if ( ReserveEntryPoint( VEHICLE_SEAT_ANY ) == false )
			{
				TaskFail( "No valid entry point!\n" );
				return;
			}

			// Find where we're going
			if ( FindPathToVehicleEntryPoint() )
			{
				TaskComplete();
				return;
			}

			// We didn't find a path
			TaskFail( "TASK_GET_PATH_TO_VEHICLE_ENTRY_POINT: Unable to run to entry point" );
		}
		break;

	case TASK_GET_PATH_TO_TARGET:
		{
			GetOuter()->SetTarget( m_hVehicle );
			BaseClass::StartTask( pTask );
		}
		break;

	case TASK_GET_PATH_TO_NEAR_VEHICLE_HOE:
		{
			if ( m_hVehicle == NULL )
			{
				TaskFail("Lost vehicle pointer\n");
				return;
			}

			// Find the passenger offset we're going for
			Vector vecRight;
			m_hVehicle->GetVectors( NULL, &vecRight, NULL );
			Vector vecTargetOffset = vecRight * 64.0f;

			// Try and find a path near there
			AI_NavGoal_t goal( GOALTYPE_TARGETENT, vecTargetOffset, AIN_DEF_ACTIVITY, 64.0f, AIN_UPDATE_TARGET_POS, m_hVehicle );
			GetOuter()->SetTarget( m_hVehicle );
			if ( GetOuter()->GetNavigator()->SetGoal( goal ) )
			{
				TaskComplete();
				return;
			}

			TaskFail( "Unable to find path to get closer to vehicle!\n" );
			return;
		}

		break;

	default:
		BaseClass::StartTask( pTask );
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CHOEHumanPassengerBehavior::IsCurTaskContinuousMove( void )
{
	const Task_t *pCurTask = GetCurTask();
	if ( pCurTask && pCurTask->iTask == TASK_RUN_TO_VEHICLE_ENTRANCE_HOE )
		return true;

	return BaseClass::IsCurTaskContinuousMove();
}

//-----------------------------------------------------------------------------
// Purpose: Update our path if we're running towards the vehicle (since it can move)
//-----------------------------------------------------------------------------
bool CHOEHumanPassengerBehavior::UpdateVehicleEntrancePath( void )
{
	// If it's too soon to check again, don't bother
	if ( m_flEntranceUpdateTime > gpGlobals->curtime )
		return true;

	// Don't attempt again for some amount of time
	m_flEntranceUpdateTime = gpGlobals->curtime + 1.0f;

	int nSequence = FindEntrySequence( true );
	if ( nSequence  == -1 )
		return false;

	SetTransitionSequence( nSequence );

	// Get the entry position
	Vector vecEntryPoint;
	QAngle vecEntryAngles;
	if ( GetEntryPoint( m_nTransitionSequence, &vecEntryPoint, &vecEntryAngles ) == false )
		return false;

	// Move the entry point forward in time a bit to predict where it'll be
	Vector vecVehicleSpeed = m_hVehicle->GetSmoothedVelocity();

	// Tack on the smoothed velocity
	vecEntryPoint += vecVehicleSpeed; // one second

	// Update our entry point
	if ( GetOuter()->GetNavigator()->UpdateGoalPos( vecEntryPoint ) == false )
		return false;

	// Reset the goal angles
	GetNavigator()->SetArrivalDirection( vecEntryAngles );
	
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHOEHumanPassengerBehavior::RunTask( const Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_RUN_TO_VEHICLE_ENTRANCE_HOE:
		{
			// Update our entrance point if we can
			if ( UpdateVehicleEntrancePath() == false )
			{
				TaskFail("Unable to find entrance to vehicle");
				break;
			}
			
			// See if we're close enough to our goal
			if ( GetOuter ()->GetNavigator()->IsGoalActive() == false )
			{
				// See if we're close enough now to enter the vehicle
				Vector vecEntryPoint;
				GetEntryPoint( m_nTransitionSequence, &vecEntryPoint );
				if ( ( vecEntryPoint - GetAbsOrigin() ).Length2DSqr() < Square( 36.0f ) )
				{
					if ( GetNavigator()->GetArrivalActivity() != ACT_INVALID )
					{
						SetActivity( GetNavigator()->GetArrivalActivity() );
					}
					
					TaskComplete();
				}
				else
				{
					TaskFail( "Unable to navigate to vehicle" );
				}
			}

			// Keep merrily going!
		}
		break;

	default:
		BaseClass::RunTask( pTask );
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: NPC needs to get to their marks, so do so with urgent navigation
//-----------------------------------------------------------------------------
bool CHOEHumanPassengerBehavior::IsNavigationUrgent( void )
{
	// If we're running to the vehicle, do so urgently
	if ( GetPassengerState() == PASSENGER_STATE_OUTSIDE && m_PassengerIntent == PASSENGER_INTENT_ENTER )
		return true;

	return BaseClass::IsNavigationUrgent();
}

//-----------------------------------------------------------------------------
// Purpose: We never want to be marked as crouching when inside a vehicle
//-----------------------------------------------------------------------------
bool CHOEHumanPassengerBehavior::IsCrouching( void )
{
	return false;
}

//-----------------------------------------------------------------------------
string_t CHOEHumanPassengerBehavior::GetRoleName( void )
{
	return ( m_sRoleName != NULL_STRING ) ? m_sRoleName : BaseClass::GetRoleName();
}

AI_BEGIN_CUSTOM_SCHEDULE_PROVIDER( CHOEHumanPassengerBehavior )
{
	DECLARE_TASK( TASK_GET_PATH_TO_VEHICLE_ENTRY_POINT_HOE )
	DECLARE_TASK( TASK_GET_PATH_TO_NEAR_VEHICLE_HOE )
	DECLARE_TASK( TASK_RUN_TO_VEHICLE_ENTRANCE_HOE )

	DEFINE_SCHEDULE
	(
		SCHED_PASSENGER_RUN_TO_ENTER_VEHICLE_HOE,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_PASSENGER_RUN_TO_ENTER_VEHICLE_FAILED_HOE"
		"		TASK_STOP_MOVING				0"
		"		TASK_SET_TOLERANCE_DISTANCE		36"	// 3 ft
		"		TASK_SET_ROUTE_SEARCH_TIME		5"
		"		TASK_GET_PATH_TO_VEHICLE_ENTRY_POINT_HOE	0"
		"		TASK_RUN_TO_VEHICLE_ENTRANCE_HOE	0"
		"		TASK_SET_SCHEDULE				SCHEDULE:SCHED_PASSENGER_ENTER_VEHICLE"
		""
		"	Interrupts"
//		"		COND_PASSENGER_CAN_ENTER_IMMEDIATELY"
		"		COND_PASSENGER_CANCEL_ENTER"
	)

	DEFINE_SCHEDULE
	(
		SCHED_PASSENGER_RUN_TO_ENTER_VEHICLE_FAILED_HOE,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_PASSENGER_ENTER_VEHICLE_PAUSE_HOE"
		"		TASK_STOP_MOVING				0"
		"		TASK_SET_TOLERANCE_DISTANCE		36"
		"		TASK_SET_ROUTE_SEARCH_TIME		3"
		"		TASK_GET_PATH_TO_NEAR_VEHICLE_HOE	0"
		"		TASK_RUN_PATH					0"
		"		TASK_WAIT_FOR_MOVEMENT			0"
		""
		"	Interrupts"
		"		COND_PASSENGER_CANCEL_ENTER"
	)

	DEFINE_SCHEDULE
	( 
		SCHED_PASSENGER_ENTER_VEHICLE_PAUSE_HOE,

		"	Tasks"
		"		TASK_STOP_MOVING			1"
		"		TASK_FACE_TARGET			0"
		"		TASK_WAIT					2"
		""
		"	Interrupts"
		"		COND_LIGHT_DAMAGE"
		"		COND_NEW_ENEMY"
		"		COND_PASSENGER_CANCEL_ENTER"
	)

	AI_END_CUSTOM_SCHEDULE_PROVIDER()
}
