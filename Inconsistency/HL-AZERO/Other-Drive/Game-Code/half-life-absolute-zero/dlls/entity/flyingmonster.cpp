/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "basemonster.h"
#include "schedule.h"
#include "flyingmonster.h"

#define FLYING_AE_FLAP		(8)
#define FLYING_AE_FLAPSOUND	(9)


extern DLL_GLOBAL edict_t* g_pBodyQueueHead;





TYPEDESCRIPTION	CFlyingMonster::m_SaveData[] = 
{
	DEFINE_FIELD( CFlyingMonster, m_vecTravel, FIELD_VECTOR ),
	DEFINE_FIELD( CFlyingMonster, m_flightSpeed, FIELD_FLOAT ),
	DEFINE_FIELD( CFlyingMonster, m_stopTime, FIELD_TIME ),
	DEFINE_FIELD( CFlyingMonster, m_momentum, FIELD_FLOAT ),
	DEFINE_FIELD( CFlyingMonster, m_pFlapSound, FIELD_CHARACTER ),
	
};

//IMPLEMENT_SAVERESTORE( CFlyingMonster, CBaseMonster );
int CFlyingMonster::Save( CSave &save )
{
	if ( !CBaseMonster::Save(save) )
		return 0;
	int iWriteFieldsResult = save.WriteFields( "CFlyingMonster", this, m_SaveData, ARRAYSIZE(m_SaveData) );

	return iWriteFieldsResult;
}
int CFlyingMonster::Restore( CRestore &restore )
{
	if ( !CBaseMonster::Restore(restore) )
		return 0;
	int iReadFieldsResult = restore.ReadFields( "CFlyingMonster", this, m_SaveData, ARRAYSIZE(m_SaveData) );

	return iReadFieldsResult;
}





//MODDD - some init for the new variables.
CFlyingMonster::CFlyingMonster(void){

	m_vecTravel = Vector(0, 0, 0);
	m_flightSpeed = 0;
	m_stopTime = 0;
	m_momentum = 0;
	m_pFlapSound = NULL;

}











GENERATE_TRACEATTACK_IMPLEMENTATION_ROUTETOPARENT(CFlyingMonster, CBaseMonster)
GENERATE_TAKEDAMAGE_IMPLEMENTATION_ROUTETOPARENT(CFlyingMonster, CBaseMonster)

/*
GENERATE_TRACEATTACK_IMPLEMENTATION(CFlyingMonster)
{
	GENERATE_TRACEATTACK_PARENT_CALL(CBaseMonster);
}

GENERATE_TAKEDAMAGE_IMPLEMENTATION(CFlyingMonster)
{
	return GENERATE_TAKEDAMAGE_PARENT_CALL(CBaseMonster);
}
*/








// the new 'doZCheck' is ignored here, not relevant to fliers.

int CFlyingMonster::CheckLocalMove ( const Vector &vecStart, const Vector &vecEnd, CBaseEntity *pTarget, BOOL doZCheck, float *pflDist )
{
	// UNDONE: need to check more than the endpoint
	if (FBitSet(pev->flags, FL_SWIM) && (UTIL_PointContents(vecEnd) != CONTENTS_WATER))
	{
		// ALERT(at_aiconsole, "can't swim out of water\n");
		return FALSE;
	}

	TraceResult tr;

	// why TaceHull when there's TRACE_MONSTER ???
	UTIL_TraceHull( vecStart + Vector( 0, 0, 32 ), vecEnd + Vector( 0, 0, 32 ), dont_ignore_monsters, large_hull, edict(), &tr );

	// ALERT( at_console, "%.0f %.0f %.0f : ", vecStart.x, vecStart.y, vecStart.z );
	// ALERT( at_console, "%.0f %.0f %.0f\n", vecEnd.x, vecEnd.y, vecEnd.z );

	if (pflDist)
	{
		*pflDist = ( (tr.vecEndPos - Vector( 0, 0, 32 )) - vecStart ).Length();// get the distance.
	}

	// ALERT( at_console, "check %d %d %f\n", tr.fStartSolid, tr.fAllSolid, tr.flFraction );
	if (tr.fStartSolid || tr.flFraction < 1.0)
	{
		if ( pTarget && pTarget->edict() == gpGlobals->trace_ent )
			return LOCALMOVE_VALID;
		return LOCALMOVE_INVALID;
	}

	return LOCALMOVE_VALID;
}


BOOL CFlyingMonster::FTriangulate ( const Vector &vecStart , const Vector &vecEnd, float flDist, CBaseEntity *pTargetEnt, Vector *pApex )
{
	return CBaseMonster::FTriangulate( vecStart, vecEnd, flDist, pTargetEnt, pApex );
}


Activity CFlyingMonster::GetStoppedActivity( void )
{ 
	if ( !isMovetypeFlying() )		// UNDONE: Ground idle here, IDLE may be something else
		return ACT_IDLE;

	return ACT_HOVER; 
	
	//return ACT_IDLE;

}


void CFlyingMonster::Stop( void ) 
{ 
	Activity stopped = GetStoppedActivity();
	if ( m_IdealActivity != stopped )
	{
		m_flightSpeed = 0;
		m_IdealActivity = stopped;
	}
	pev->angles.z = 0;
	pev->angles.x = 0;
	m_vecTravel = g_vecZero;
}


float CFlyingMonster::ChangeYaw( int speed )
{
	if ( isMovetypeFlying() )
	{
		float diff = FlYawDiff();
		float target = 0;

		if ( m_IdealActivity != GetStoppedActivity() )
		{
			if ( diff < -20 )
				target = 90;
			else if ( diff > 20 )
				target = -90;
		}
		pev->angles.z = UTIL_Approach( target, pev->angles.z, 220.0 * gpGlobals->frametime );
	}
	return CBaseMonster::ChangeYaw( speed );
}


//MODDD NOTE - does the stuka need to take any of this into account too?
GENERATE_KILLED_IMPLEMENTATION(CFlyingMonster)
{

	//MODDD NOTE - Dear flying monster... why MOVETYPE_STEP at killed? This freezes the monster in place instead of letting it fall to the ground.
	//             Keeping it this way in fear of side effects of changing it, but any other custom monsters should set movetype to MOVETYPE_TOSS
	//             to take gravity into effect.  "pev->gravity" of 0 still has gravity with the right movetype, and looks to be ignored otherwise anyways.
	// Question: why does this issue only happen sometimes? Why does the movetype somtimes end up TOSS anyways, and sometiems become STEP?
	// Because "Killed" can be called if a monster runs out of health again WHILE in the DEAD_DYING pev->dead choice.
	// And that time, it goes here but the monster's base KILLED method that would force the movetype to TOSS anyways is skipped this time.
	// Typically, the first time a monster is killed (pev->dead of DEAD_NO becoming DEAD_DYING), the base Killed call below will
	// force the movetype to MOVETYPE_TOSS (properly) in the BecomeDead call.
	// When in DEAD_DYTING while this is called, that never happens. So MOVETYPE_STEP is left. Eh.
	pev->movetype = MOVETYPE_STEP;

	//MODDD - only disable the ONGROUND bit if this is the first Killed call.
	// Any Killed calls while dead don't need to keep resetting this.
	// Wait... what's the point of this?  Whatever, burnt out from touching this kind of stuff.
	if(pev->deadflag == DEAD_NO){
		ClearBits( pev->flags, FL_ONGROUND );
	}

	pev->angles.z = 0;
	pev->angles.x = 0;
	GENERATE_KILLED_PARENT_CALL(CBaseMonster);
}


void CFlyingMonster::HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch( pEvent->event )
	{
	case FLYING_AE_FLAP:
		m_flightSpeed = 400;
	break;
	case FLYING_AE_FLAPSOUND:
	//MODDD - something never set.
	/*
		if ( m_pFlapSound )
			EMIT_SOUND( edict(), CHAN_BODY, m_pFlapSound, 1, ATTN_NORM );	
	*/
	break;
	default:
		CBaseMonster::HandleAnimEvent( pEvent );
	break;
	}
}


void CFlyingMonster::Move( float flInterval )
{
	if ( isMovetypeFlying() )
		m_flGroundSpeed = m_flightSpeed;
	CBaseMonster::Move( flInterval );
}


BOOL CFlyingMonster::ShouldAdvanceRoute( float flWaypointDist, float flInterval )
{
	// Get true 3D distance to the goal so we actually reach the correct height
	if ( m_Route[ m_iRouteIndex ].iType & bits_MF_IS_GOAL )
		flWaypointDist = ( m_Route[ m_iRouteIndex ].vecLocation - pev->origin ).Length();

	//MODDD - Why gpGlobals->frametime??  Doesn't measure the difference in think-times well
	// replaced with flInterval
	if ( flWaypointDist <= 64 + (m_flGroundSpeed * flInterval) )
		return TRUE;

	return FALSE;
}


void CFlyingMonster::MoveExecute( CBaseEntity *pTargetEnt, const Vector &vecDir, float flInterval )
{
	//MODDD - any references to gpGlobals->frametime replaced with flInterval

	if ( isMovetypeFlying() )
	{
		if ( gpGlobals->time - m_stopTime > 1.0 )
		{
			if ( m_IdealActivity != m_movementActivity )
			{
				m_IdealActivity = m_movementActivity;
				m_flGroundSpeed = m_flightSpeed = 200;
			}
		}
		Vector vecMove = pev->origin + (( vecDir + (m_vecTravel * m_momentum) ).Normalize() * (m_flGroundSpeed * flInterval));

		if ( m_IdealActivity != m_movementActivity )
		{
			m_flightSpeed = UTIL_Approach( 100, m_flightSpeed, 75 * flInterval );
			if ( m_flightSpeed < 100 )
				m_stopTime = gpGlobals->time;
		}
		else
			m_flightSpeed = UTIL_Approach( 20, m_flightSpeed, 300 * flInterval );
		
		//MODDD - wait.  As-is, CheckLocalMove, and didn't check that it's LOCALMOVE_VALID
		// (since CheckLocalMove can also return LOCALMOVE_INVALID_DONT_TRIANGULATE, which is also non-zero)???
		// No idea.  Now it's checked.
		if ( CheckLocalMove ( pev->origin, vecMove, pTargetEnt, FALSE, NULL ) == LOCALMOVE_VALID)
		{
			m_vecTravel = (vecMove - pev->origin);
			m_vecTravel = m_vecTravel.Normalize();
			UTIL_MoveToOrigin(ENT(pev), vecMove, (m_flGroundSpeed * flInterval), MOVE_STRAFE);
		}
		else
		{
			m_IdealActivity = GetStoppedActivity();
			m_stopTime = gpGlobals->time;
			m_vecTravel = g_vecZero;
		}
	}
	else
		CBaseMonster::MoveExecute( pTargetEnt, vecDir, flInterval );
}


//MODDD NOTE - is this called by anywhere at all? Is this just completley unused?
//Same for ProbeZ and FloorZ??
float CFlyingMonster::CeilingZ( const Vector &position )
{
	TraceResult tr;

	Vector minUp = position;
	Vector maxUp = position;
	maxUp.z += 4096.0;

	UTIL_TraceLine(position, maxUp, ignore_monsters, NULL, &tr);
	if (tr.flFraction != 1.0)
		maxUp.z = tr.vecEndPos.z;

	if ((pev->flags) & FL_SWIM)
	{
		return UTIL_WaterLevel( position, minUp.z, maxUp.z );
	}
	return maxUp.z;
}

BOOL CFlyingMonster::ProbeZ( const Vector &position, const Vector &probe, float *pFraction)
{
	int conPosition = UTIL_PointContents(position);
	if ( (((pev->flags) & FL_SWIM) == FL_SWIM) ^ (conPosition == CONTENTS_WATER))
	{
		//    SWIMING & !WATER
		// or FLYING  & WATER
		//
		*pFraction = 0.0;
		return TRUE; // We hit a water boundary because we are where we don't belong.
	}
	int conProbe = UTIL_PointContents(probe);
	if (conProbe == conPosition)
	{
		// The probe is either entirely inside the water (for fish) or entirely
		// outside the water (for birds).
		//
		*pFraction = 1.0;
		return FALSE;
	}

	Vector ProbeUnit = (probe-position).Normalize();
	float ProbeLength = (probe-position).Length();
	float maxProbeLength = ProbeLength;
	float minProbeLength = 0;

	float diff = maxProbeLength - minProbeLength;
	while (diff > 1.0)
	{
		float midProbeLength = minProbeLength + diff/2.0;
		Vector midProbeVec = midProbeLength * ProbeUnit;
		if (UTIL_PointContents(position+midProbeVec) == conPosition)
		{
			minProbeLength = midProbeLength;
		}
		else
		{
			maxProbeLength = midProbeLength;
		}
		diff = maxProbeLength - minProbeLength;
	}
	*pFraction = minProbeLength/ProbeLength;

	return TRUE;
}

float CFlyingMonster::FloorZ( const Vector &position )
{
	TraceResult tr;

	Vector down = position;
	down.z -= 2048;

	UTIL_TraceLine( position, down, ignore_monsters, NULL, &tr );

	if ( tr.flFraction != 1.0 )
		return tr.vecEndPos.z;

	return down.z;
}


BOOL CFlyingMonster::AffectedByKnockback(void){
	return TRUE;  // for now?
}


/*
void CFlyingMonster::StartTask( Task_t *pTask ){

	switch( pTask->iTask ){
		case TASK_DIE:
		{

		}
		default:
			CBaseMonster::StartTask( pTask );
		break;
	}//END OF switch

}//END OF StartTask

void CFlyingMonster::RunTask( Task_t *pTask ){
	
	switch( pTask->iTask ){
		case TASK_DIE:
		{

		}
		default:
			CBaseMonster::RunTask(pTask);
		break;
	}//END OF switch

}//END OF RunTask
*/


// In case a flying monster ever ends up in water, go ahead and be able to target the enemy outside of it.
// If fliers (besides swimming ones like the archer of course) show up in maps with water and seeking targets
// underwater is an issue, say so, that way (on the surface, prefer not to go underwater) can be sorted out.
BOOL CFlyingMonster::SeeThroughWaterLine(void){
	return TRUE;
}//END OF SeeThroughWaterLine


