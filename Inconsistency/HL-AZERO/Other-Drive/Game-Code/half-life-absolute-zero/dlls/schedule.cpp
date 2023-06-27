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
//=========================================================
// schedule.cpp - functions and data pertaining to the 
// monsters' AI scheduling system.
//=========================================================
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "basemonster.h"
#include "util_model.h"
#include "scripted.h"
#include "nodes.h"
#include "defaultai.h"
#include "soundent.h"
#include "util_debugdraw.h"

//#include "hassault.h"  //breakpoint test
// no just do this
extern Schedule_t slHAssault_residualFire[];

EASY_CVAR_EXTERN_DEBUGONLY(sparksAIFailMulti)
EASY_CVAR_EXTERN_DEBUGONLY(crazyMonsterPrintouts)
EASY_CVAR_EXTERN_DEBUGONLY(movementIsCompletePrintout)
EASY_CVAR_EXTERN_DEBUGONLY(noFlinchOnHard)
EASY_CVAR_EXTERN_DEBUGONLY(drawCollisionBoundsAtDeath)
EASY_CVAR_EXTERN_DEBUGONLY(drawHitBoundsAtDeath)
EASY_CVAR_EXTERN_DEBUGONLY(scheduleInterruptPrintouts)
EASY_CVAR_EXTERN_DEBUGONLY(pathfindStumpedMode)
EASY_CVAR_EXTERN_DEBUGONLY(pathfindStumpedWaitTime)
EASY_CVAR_EXTERN_DEBUGONLY(pathfindStumpedForgetEnemy)
EASY_CVAR_EXTERN_DEBUGONLY(animationFramerateMulti)

extern CGraph WorldGraph;


//MODDD
#define CONDITIONS_BITMASK_USE m_afConditions


BOOL g_queueSomeCondReset = FALSE;


//MODDD - new.  Helper method for some schedules, not really for basemonster.cpp calls.
// (if taking a Task_t was any sign... name changed to be even more obvious)
BOOL CBaseMonster::SCHEDULE_attemptFindCoverFromEnemy(Task_t* pTask){
	entvars_t *pevCover;

	if ( m_hEnemy == NULL )
	{
		// Find cover from self if no enemy available
		pevCover = pev;
//				TaskFail();
//				return FALSE; //return;
	}
	else
		pevCover = m_hEnemy->pev;

	if ( FindLateralCover( pevCover->origin, pevCover->view_ofs ) )
	{
		// try lateral first
		m_flMoveWaitFinished = gpGlobals->time + pTask->flData;
		return TRUE;//TaskComplete();
	}
	else if ( FindCover( pevCover->origin, pevCover->view_ofs, 0, CoverRadius() ) )
	{
		// then try for plain ole cover
		m_flMoveWaitFinished = gpGlobals->time + pTask->flData;
		return TRUE;//TaskComplete();
	}
	else
	{
		// no coverwhatsoever.
		return FALSE;//TaskFail();
	}
}//END OF SCHEDULE_attemptFindCoverFromEnemy()





//=========================================================
// FHaveSchedule - Returns TRUE if monster's m_pSchedule
// is anything other than NULL.
//=========================================================
BOOL CBaseMonster::FHaveSchedule( void )
{
	if ( m_pSchedule == NULL )
	{
		return FALSE;
	}

	return TRUE;
}

//=========================================================
// ClearSchedule - blanks out the caller's schedule pointer
// and index.
//=========================================================
void CBaseMonster::ClearSchedule( void )
{
	m_iTaskStatus = TASKSTATUS_NEW;
	m_pSchedule = NULL;
	m_iScheduleIndex = 0;
}

//=========================================================
// FScheduleDone - Returns TRUE if the caller is on the
// last task in the schedule
//=========================================================
BOOL CBaseMonster::FScheduleDone ( void )
{
	ASSERT( m_pSchedule != NULL );
	
	if ( m_iScheduleIndex == m_pSchedule->cTasks )
	{
		return TRUE;
	}

	return FALSE;
}

//=========================================================
// ChangeSchedule - replaces the monster's schedule pointer
// with the passed pointer, and sets the ScheduleIndex back
// to 0
//=========================================================
void CBaseMonster::ChangeSchedule ( Schedule_t *pNewSchedule )
{
	if(EASY_CVAR_GET_DEBUGONLY(crazyMonsterPrintouts))easyForcePrintLine("YOU despicable person %s %d", pNewSchedule->pName, pNewSchedule->iInterruptMask);

	ASSERT( pNewSchedule != NULL );
	
	if (monsterID == 0) {
		int x = 345;
	}

	//MODDD - for now, let's count failing to change to a new schedule as a TaskFail() but be sure to talk
	//        about it in printouts, don't want to miss this happening.
	if(pNewSchedule == NULL){
		easyPrintLine("WARNING: %s:%d: ChangeSchedule called with NULL schedule!", this->getClassname(), monsterID);
		TaskFail();
		return;
	}

	g_queueSomeCondReset = TRUE;

	//MODDD - is this safe to assume when changing schedules?
	//        Don't want to feel obliged to stick with an animation that may no longer be fitting.
	this->usingCustomSequence = FALSE;

	// if this was turned on, forget.
	// Could be reset by other pathfinding methods (FRefreshRoute) at some point instead.
	// Nevermind, that would be really tricky to pull off.  All kinds of cases of Refresh getting called, RouteNew,
	// RouteClear, etc.  Best to leave this on the whole schedule once it's needed (or have a task to take it off
	// if ever necessary).  Checking to see if the current task prefers 'strict' versions is kindof the same idea
	// anyway.
	// And reset the 'goalDistTolerance', that is to be set within a schedule for that schedule too.
	goalDistTolerance = 0;
	strictNodeTolerance = FALSE;


	m_pSchedule = pNewSchedule;
	m_iScheduleIndex = 0;
	m_iTaskStatus = TASKSTATUS_NEW;

	//MODDD - important.	ChangeSchedule now no longer resets conditions alone.
	//Runtask() now resets some, if not most conditions before calling RunAI(). Or early on in RunAI() once.


	//MODDD NOTE - don't get slick.  Just clear all conditions.
	// also, DAMMIT MAN.  We have clearAllConditions for a reason!!  Breakpoints.  DAMN.
	//m_afConditions = 0;// clear all of the conditions
	
	// DANGEROUS!  Was ClearAllConditionsExcept_ThisFrame  (default includes that and NextFrame).
	//ClearAllConditionsExcept_ThisFrame(bits_COND_TASK_FAILED | bits_COND_SCHEDULE_DONE | bits_COND_NEW_ENEMY);
	//ClearAllConditionsExcept_ThisFrame(bits_COND_HEAR_SOUND | bits_COND_SEE_HATE | bits_COND_SEE_DISLIKE | bits_COND_SEE_ENEMY | bits_COND_SEE_FEAR | bits_COND_SEE_NEMESIS | bits_COND_SEE_CLIENT | bits_COND_ENEMY_OCCLUDED);
	//MODDDD
	// *bits_COND_NEW_ENEMY and bits_COND_ENEMY_DEAD removed, cleared at the end of MaintainSchedule
	// on any sched change instead*
	ClearConditions(bits_COND_NEW_ENEMY | bits_COND_ENEMY_DEAD | bits_COND_TASK_FAILED | bits_COND_SCHEDULE_DONE);
	//ClearConditions(bits_COND_TASK_FAILED | bits_COND_SCHEDULE_DONE);

	//ClearConditionsMod( );   //oh none.

	//weren't these good ideas though, sorta?
	//////////////////////////////////////////////////////////////
	//m_afConditions &= ~(bits_COND_TASK_FAILED | bits_COND_SCHEDULE_DONE);
	//m_afConditions &= (bits_COND_CAN_RANGE_ATTACK1 | bits_COND_CAN_MELEE_ATTACK1 | bits_COND_CAN_RANGE_ATTACK2 | bits_COND_CAN_MELEE_ATTACK2);
	//////////////////////////////////////////////////////////////
	//MODDD!!!!!!!!!!!!!!!!!!!!!


	//m_afConditionsNextFrame &= ~(bits_COND_TASK_FAILED | bits_COND_SCHEDULE_DONE | bits_COND_NEW_ENEMY);



	m_failSchedule = SCHED_NONE;
	hardSetFailSchedule = FALSE;  //MODDD - any fail schedule resets also turn this off.
	scheduleSurvivesStateChange = FALSE;  //MODDD - also new. Default schedule behavior: state changes force the schedule to want to change.

	m_fNewScheduleThisFrame = TRUE; //for recording. Notify if this same schedule ends up having a task complete or the schedule marked for failure before
	// this same frame (MonsterThink) ends once. If so ChangeSchedule was immediately followed by TaskComplete or TaskFail, must know not to do that.

	if ( m_pSchedule->iInterruptMask & bits_COND_HEAR_SOUND && !(m_pSchedule->iSoundMask) )
	{
		ALERT ( at_aiconsole, "COND_HEAR_SOUND with no sound mask!\n" );
	}
	else if ( m_pSchedule->iSoundMask && !(m_pSchedule->iInterruptMask & bits_COND_HEAR_SOUND) )
	{
		ALERT ( at_aiconsole, "Sound mask without COND_HEAR_SOUND!\n" );
	}

#if _DEBUG
	if ( !ScheduleFromName( pNewSchedule->pName ) )
	{
		ALERT( at_console, "Schedule %s not in table!!!\n", pNewSchedule->pName );
	}
#endif
	
// this is very useful code if you can isolate a test case in a level with a single monster. It will notify
// you of every schedule selection the monster makes.
#if 0
	if ( FClassnameIs( pev, "monster_human_grunt" ) )
	{
		Task_t *pTask = GetTask();
		
		if ( pTask )
		{
			const char *pName = NULL;

			if ( m_pSchedule )
			{
				pName = m_pSchedule->pName;
			}
			else
			{
				pName = "No Schedule";
			}
			
			if ( !pName )
			{
				pName = "Unknown";
			}

			ALERT( at_aiconsole, "%s: picked schedule %s\n", STRING( pev->classname ), pName );
		}
	}
#endif// 0

}//ChangeSchedule

//=========================================================
// NextScheduledTask - increments the ScheduleIndex
//=========================================================
void CBaseMonster::NextScheduledTask ( void )
{
	ASSERT( m_pSchedule != NULL );

	m_iTaskStatus = TASKSTATUS_NEW;
	m_iScheduleIndex++;

	if ( FScheduleDone() )
	{
		// just completed last task in schedule, so make it invalid by clearing it.
		SetConditions( bits_COND_SCHEDULE_DONE );
		//ClearSchedule();	
	}
}

//=========================================================
// IScheduleFlags - returns an integer with all Conditions
// bits that are currently set and also set in the current
// schedule's Interrupt mask.
//=========================================================
int CBaseMonster::IScheduleFlags ( void )
{
	if( !m_pSchedule )
	{
		return 0;
	}
	
	// strip off all bits excepts the ones capable of breaking this schedule.
	return CONDITIONS_BITMASK_USE & m_pSchedule->iInterruptMask;
}
//MODDD - IScheduleFlagsMod?   If we want to give every single schedule in the game a default '0' for that 2nd interrupt bitmask.
// LLLLllllllllllleeeeeeeettttttttssssss avoid that





//#bitToCheck
#define CHEAPO(nickname, bitToCheck) ,\
	#nickname, (m_pSchedule->iInterruptMask & bits_COND_##bitToCheck)!=0, (m_afConditions & bits_COND_##bitToCheck)!=0, (m_afConditionsNextFrame & bits_COND_##bitToCheck)!=0


#define CHEAPO2(nickname, bitToCheck) ,\
	#nickname, 9, (m_afConditions & bits_COND_##bitToCheck)!=0, (m_afConditionsNextFrame & bits_COND_##bitToCheck)!=0

//=========================================================
// FScheduleValid - returns TRUE as long as the current
// schedule is still the proper schedule to be executing,
// taking into account all conditions
//=========================================================
BOOL CBaseMonster::FScheduleValid ( void )
{
	if ( m_pSchedule == NULL )
	{
		if(EASY_CVAR_GET_DEBUGONLY(crazyMonsterPrintouts))easyForcePrintLine("FScheduleValid: fail A");
		// schedule is empty, and therefore not valid.
		return FALSE;
	}

	if (FClassnameIs(pev, "monster_scientist")) {
		if (HasConditions(bits_COND_LIGHT_DAMAGE)) {
			int x = 45;
		}
		if (HasConditions(bits_COND_HEAVY_DAMAGE)) {
			int x = 45;
		}
	}





	if (FClassnameIs(pev, "monster_gargantua")) {
		int x = 44325;
	}



	if ( HasConditions( m_pSchedule->iInterruptMask | bits_COND_SCHEDULE_DONE | bits_COND_TASK_FAILED ) )
	{
		// !!! NOTICE !!!
		// This is a great place for a breakpoint.  Schedule's interrupted, give what it was here, what task it was, etc.
		// It's often really fast when a schedule gets interrupted, issues from scheduels getting picked/interrupted over and over
		// can be nightmarish to figure out otherwise.


		if(EASY_CVAR_GET_DEBUGONLY(crazyMonsterPrintouts))easyForcePrintLine("FScheduleValid: fail B: %s %d :::%d %d %d", m_pSchedule->pName, m_pSchedule->iInterruptMask, (m_afConditions & m_pSchedule->iInterruptMask), (m_afConditions & bits_COND_SCHEDULE_DONE), (m_afConditions & bits_COND_TASK_FAILED) );
		

		//This is a table of all conditions, whether the interrupt mask has them (counts for interrupt), and whether we happen to
		//have the condition set at this time (only interrupts if we have the condition set AND it is in the interupt list).
		
		
		//NOTE - if the schedule is done, it is natural for its task number to be -1. It ran out of tasks, which ended the
		//schedule and is why it is now interrupting (naturally) to look for a new one.

		
		//if(FClassnameIs(this->pev, "monster_scientist"))
		if(EASY_CVAR_GET_DEBUGONLY(scheduleInterruptPrintouts) == 1)
			easyForcePrintLine("%s:%d SCHEDULE INTERRUPTED. Name:%s task:%d taskindex:%d\n"
			"%s: %d : %d %d\n"
			"%s: %d : %d %d\n"
			"%s: %d : %d %d\n"
			"%s: %d : %d %d\n"
			"%s: %d : %d %d\n"
			"%s: %d : %d %d\n"
			"%s: %d : %d %d\n"
			"%s: %d : %d %d\n"
			"%s: %d : %d %d\n"
			"%s: %d : %d %d\n"
			"%s: %d : %d %d\n"
			"%s: %d : %d %d\n"
			"%s: %d : %d %d\n"
			"%s: %d : %d %d\n"
			"%s: %d : %d %d\n"
			"%s: %d : %d %d\n"
			"%s: %d : %d %d\n"
			"%s: %d : %d %d\n"
			"%s: %d : %d %d\n"
			"%s: %d : %d %d\n"
			"%s: %d : %d %d\n"
			"%s: %d : %d %d\n"
			"%s: %d : %d %d\n"
			"%s: %d : %d %d\n"
			"%s: %d : %d %d\n"
			"%s: %d : %d %d\n",
			getClassname(), monsterID, getScheduleName(), this->getTaskNumber(), m_iScheduleIndex
			CHEAPO(noammo, NO_AMMO_LOADED)
			CHEAPO(seehat, SEE_HATE)
			CHEAPO(se_fea, SEE_FEAR)
			CHEAPO(se_dis, SEE_DISLIKE)
			CHEAPO(se_ene, SEE_ENEMY)
			CHEAPO(en_occ, ENEMY_OCCLUDED)
			CHEAPO(sm_foo, SMELL_FOOD)
			CHEAPO(en_far, ENEMY_TOOFAR)
			CHEAPO(li_dmg, LIGHT_DAMAGE)
			CHEAPO(hv_dmg, HEAVY_DAMAGE)
			CHEAPO(c_rat1, CAN_RANGE_ATTACK1)
			CHEAPO(c_mat1, CAN_MELEE_ATTACK1)
			CHEAPO(c_rat2, CAN_RANGE_ATTACK2)
			CHEAPO(c_mat2, CAN_MELEE_ATTACK2)

			CHEAPO(provok, PROVOKED)
			CHEAPO(new_en, NEW_ENEMY)
			CHEAPO(hear_s, HEAR_SOUND)
			CHEAPO(smellg, SMELL)
			CHEAPO(en_fac, ENEMY_FACING_ME)
			CHEAPO(en_dea, ENEMY_DEAD)
			CHEAPO(se_cli, SEE_CLIENT)
			CHEAPO(se_nem, SEE_NEMESIS)
			CHEAPO(spec_1, SPECIAL1)
			CHEAPO(spec_2, SPECIAL2)
			CHEAPO2(tasfai, TASK_FAILED)
			CHEAPO2(schdon, SCHEDULE_DONE)
			);

			

#ifdef DEBUG
		if ( HasConditions ( bits_COND_TASK_FAILED ) && m_failSchedule == SCHED_NONE )
		{
			// fail! Send a visual indicator.
			ALERT ( at_aiconsole, "Schedule: %s Failed\n", m_pSchedule->pName );

			Vector tmp = pev->origin;
			tmp.z = pev->absmax.z + 16;

			//MODDD
			//UTIL_Sparks( tmp );
			UTIL_Sparks( tmp, DEFAULT_SPARK_BALLS, EASY_CVAR_GET_DEBUGONLY(sparksAIFailMulti) );
		}
#endif // DEBUG

		// some condition has interrupted the schedule, or the schedule is done
		return FALSE;
	}
	
	return TRUE;
}



//=========================================================
// MaintainSchedule - does all the per-think schedule maintenance.
// ensures that the monster leaves this function with a valid
// schedule!
//=========================================================
void CBaseMonster::MaintainSchedule ( void )
{
	//MODDD - NOTE.
	// How should giving up due to picking the same schedule as the previous work?
	// Run until the end of the current loop iteration (set i to 999)?
	// Run through the 2nd picked schedule but end as soon as it wants to change the
	// schedule for any reason?   'break' as soon as the 2nd same schedule is picked?

	// And m_iTaskStatus isn't used when a task failed, right?  Be sure of that.

	// !!! BEWARE OF TRYING TO IMPROVE THIS FURTHER.  Because believe me, I tried.
	// A lot of the slight issues remaining (slight flinch to switching to ACT_IDLE
	// in slIdleStand on finishing a scripted sequence, only to recognize there is an
	// enemy to move states from MONSTERSTATE_IDLE to ALERT to COMBAT) are most likely
	// because AI does differnet checks for different monsterstates, which aren't
	// redone on schedule changes in the loop.  So it takes another think-frame for that
	// info to come from the next think-frame with a monster in ALERT/COMBAT, even if
	// 'new enemy' is forced to always being allowed (regardless of whether a schedule
	// allows it to be interruptable;  no harm as it has no effect being ticked on in a 
	// schedule not interruptable by it, and it could at least allow schedules within
	// COMBAT to update with it correctly).

	// Conditions NEW_ENEMY and ENEMY_DEAD are reset after a schedule change as most of the
	// codebase expects.  Could change almost everything that acts on them to reset
	// them after doing something based off them too, unsure if that's worthwhile.

	// ALSO.  Don't try playing with the 'm_Activity != m_IdealActivity' bit that happens
	// in the middle of the loop.  Sadly that's also very in consistent and not worthwhile.



	Schedule_t* pNewSchedule;
	Schedule_t* pPrevSchedule;
	int i;
	BOOL pickedScheduleThisCall = FALSE;
	BOOL queueStopAtScheduleEnd = FALSE;

	/*
	int oldSeq = pev->sequence;
	float oldFrame = pev->frame;
	int oldAct = m_Activity;
	//int oldIdealAct = m_IdealActivity;
	*/

	/*
	if(m_iTaskStatus == TASKSTATUS_RUNNING){
		return;
	}
	*/

	if(EASY_CVAR_GET_DEBUGONLY(crazyMonsterPrintouts) == 1){
		easyPrintLine("DOCKS1 %d", HasConditions(bits_COND_CAN_MELEE_ATTACK1));
	}

	if(monsterID == 0){
		int x = 34;
	}

	// UNDONE: Tune/fix this 10... This is just here so infinite loops are impossible
	// MODDD - let's do more!  Upped to 25.    ...nah.  6 is plenty.
	// Nope, gimmie those smarts.
	//for ( i = 0; i < 10; i++ )
	for (i = 0; i < 20; i++)
	{

		if ( m_pSchedule != NULL && TaskIsComplete() )
		{
			NextScheduledTask();                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   
		}


		if(EASY_CVAR_GET_DEBUGONLY(crazyMonsterPrintouts) == 1){
			easyPrintLine("OOPS A PLENTY 1 %d :::%d %d :::%d %d", HasConditions(bits_COND_CAN_MELEE_ATTACK1), !FScheduleValid(), m_MonsterState != m_IdealMonsterState,  m_MonsterState, m_IdealMonsterState );
		}
	// validate existing schedule 


		BOOL validSchedule = FScheduleValid();

		if(!validSchedule && queueStopAtScheduleEnd){
			// goodbye
			break;
		}

		//MODDD - a schedule with "scheduleSurvivesStateChange" set to TRUE will disregard state changes. But the schedule going invalid by having interruptible conditions from the schedule is still possible.
		//if ( (!FScheduleValid() || m_MonsterState != m_IdealMonsterState) )
		if (
			!validSchedule ||
			(!scheduleSurvivesStateChange && m_MonsterState != m_IdealMonsterState)
		)
		{
			if(EASY_CVAR_GET_DEBUGONLY(crazyMonsterPrintouts))easyForcePrintLine("MaintainSchedule: INVALID A %d %d %d", FScheduleValid(), m_MonsterState, m_IdealMonsterState);


			// if we come into this block of code, the schedule is going to have to be changed.
			// if the previous schedule was interrupted by a condition, GetIdealState will be 
			// called. Else, a schedule finished normally.

			// Notify the monster that his schedule is changing
			//!!!!! THIS IS BARELY USED! I missed it and tried implementing the same idea.
			ScheduleChange();
			

			if(EASY_CVAR_GET_DEBUGONLY(crazyMonsterPrintouts) == 1){
				easyPrintLine("OOPS A PLENTY 2 %d", HasConditions(bits_COND_CAN_MELEE_ATTACK1));
			}
			// Call GetIdealState if we're not dead and one or more of the following...
			// - in COMBAT state with no enemy (it died?)
			// - conditions bits (excluding SCHEDULE_DONE) indicate interruption,
			// - schedule is done but schedule indicates it wants GetIdealState called
			//   after successful completion (by setting bits_COND_SCHEDULE_DONE in iInterruptMask)
			// DEAD & SCRIPT are not suggestions, they are commands!


			//easyForcePrintLine("%s:%d WHAT IS UP?? State: cur:%d idea:%d", getClassname(), monsterID, m_MonsterState, m_IdealMonsterState);
			if ( m_IdealMonsterState != MONSTERSTATE_DEAD && 
				 (m_IdealMonsterState != MONSTERSTATE_SCRIPT || m_IdealMonsterState == m_MonsterState) )
			{
				//easyForcePrintLine("doooop (%d && %d) %d (%d && %d)", m_afConditions, !HasConditions(bits_COND_SCHEDULE_DONE), (m_pSchedule && (m_pSchedule->iInterruptMask & bits_COND_SCHEDULE_DONE)), (m_MonsterState == MONSTERSTATE_COMBAT), (m_hEnemy==NULL) );
				if (	(CONDITIONS_BITMASK_USE && !HasConditions(bits_COND_SCHEDULE_DONE)) ||
					// Sorry... What? Doesn't any schedule being done count as being done? They don't have to explicitly say they are interrupted by being done, 
					// it is implied they are... what.  Keeping for safety.
					(m_pSchedule && (m_pSchedule->iInterruptMask & bits_COND_SCHEDULE_DONE)) ||
					((m_MonsterState == MONSTERSTATE_COMBAT) && (m_hEnemy == NULL))	)
				{
					if(EASY_CVAR_GET_DEBUGONLY(crazyMonsterPrintouts) == 1){
						easyPrintLine("OOPS A PLENTY 3 %d", HasConditions(bits_COND_CAN_MELEE_ATTACK1));
					}
					
					//MODDD
					// WAIT.
					// What's this?
					// We... call a method.  That returns something.
					// And do nothing,  with what it returns.
					//GetIdealState();
					m_IdealMonsterState = GetIdealState();

					if(EASY_CVAR_GET_DEBUGONLY(crazyMonsterPrintouts) == 1){
						easyPrintLine("OOPS A PLENTY 4 %d", HasConditions(bits_COND_CAN_MELEE_ATTACK1));
					}
				}
			}

			//BOOL temppp = HasConditions( bits_COND_TASK_FAILED );

			//MODDD - a hard-set fail schedule will run regardless of the monster state matching the ideal or not.  Use if getting out of a schedule MUST do the fail schedule.
			//        It will even run for any kind of interruption. Typical failure (from say, seeing an enemy in the middle of a schedule marked as interruptable by that) would not do this.
			// ---CHANGE UP, this might be better.
			//if ( hardSetFailSchedule ||  (HasConditions( bits_COND_TASK_FAILED ) && ( m_MonsterState == m_IdealMonsterState)) )
			if ( HasConditions( bits_COND_TASK_FAILED ) && ( hardSetFailSchedule || ( m_MonsterState == m_IdealMonsterState)) )
			{
				// Keep track of whether this was set by a hard fail schedule or not. The schedule changes below can do some freaky things.
				BOOL wasSetByHardFail = hardSetFailSchedule;
				
				// remember this
				pPrevSchedule = m_pSchedule;

				if(EASY_CVAR_GET_DEBUGONLY(crazyMonsterPrintouts) == 1){
					easyPrintLine("OOPS A PLENTY 5 %d", HasConditions(bits_COND_CAN_MELEE_ATTACK1));
				}
				if ( m_failSchedule != SCHED_NONE ){
					pNewSchedule = GetScheduleOfType( m_failSchedule );
					//MODDD - new section.
				}else{
					pNewSchedule = GetScheduleOfType( SCHED_FAIL );
				}

				//MODDD - same schedule as the current one?  No more iterations for this loop,
				// but still continue this one until the end.
				// But wait! It is possible to pick the exact same schedule after another one has finished.
				// The point of this is to disallow doing more iterations if it's likely to stagnate (looped around to pick the same
				// schedule between two iterations), NOT disallow say, picking the melee schedule again because the enemy is still in
				// front of me.
				// But careful with even a 'i > 0' check.  Should be fine most of the time, but what if something finishes in this frame and
				// goes on to then pick the same schedule as the last run?  'i' would be above 0 and have to stop at the first task in the
				// new schedule.  Record whether this call of the method (MaintainSchedule) has ever fetched a method before intead.
				if(pickedScheduleThisCall && pNewSchedule == pPrevSchedule){
					if(EASY_CVAR_GET_DEBUGONLY(scheduleInterruptPrintouts)){easyForcePrintLine("!!! %s:%d Same schedule (%s) picked in the same frame, BLOCKED. <fail>", getClassname(), monsterID, m_pSchedule->pName);}
					//i = 999;
					// queue the end instead, may still be possible to do several or all tasks in this 2nd run of the schedule
					queueStopAtScheduleEnd = TRUE;
					break;
				}

				pickedScheduleThisCall = TRUE;  // have now

				// schedule was invalid because the current task failed to start or complete
				ALERT ( at_aiconsole, "Schedule Failed at %d!\n", m_iScheduleIndex );
				if(EASY_CVAR_GET_DEBUGONLY(crazyMonsterPrintouts) == 1){
					easyPrintLine("OOPS A PLENTY 6 %d", HasConditions(bits_COND_CAN_MELEE_ATTACK1));
				}
				ChangeSchedule( pNewSchedule );
				if(EASY_CVAR_GET_DEBUGONLY(crazyMonsterPrintouts) == 1){
					easyPrintLine("OOPS A PLENTY 7 %d", HasConditions(bits_COND_CAN_MELEE_ATTACK1));
				}

				// "hardSetFailSchedule" gets affected by the ChangeSchedule above unfortunately.  Its value before that change is what matters.
				// If so, this schedule needs to resist changing from a desire to change states.
				if(wasSetByHardFail){
					//This schedule must survive state changes.
					scheduleSurvivesStateChange = TRUE;
				}

				//MODDD - Now that hardSetFailSchedule has been involved in a decision, it does not need to stay on (if it is)
				hardSetFailSchedule = FALSE;
			}
			else
			{
				if(EASY_CVAR_GET_DEBUGONLY(crazyMonsterPrintouts) == 1){
					easyPrintLine("OOPS A PLENTY 8 %d :::%d, %d :::%d %d", HasConditions(bits_COND_CAN_MELEE_ATTACK1),  m_MonsterState == MONSTERSTATE_SCRIPT,  m_MonsterState == MONSTERSTATE_DEAD,    m_MonsterState == MONSTERSTATE_SCRIPT, m_MonsterState == MONSTERSTATE_DEAD  );
				}
				SetState( m_IdealMonsterState );

				// remember this
				pPrevSchedule = m_pSchedule;

				if ( m_MonsterState == MONSTERSTATE_SCRIPT || m_MonsterState == MONSTERSTATE_DEAD ){
					pNewSchedule = CBaseMonster::GetSchedule();
					if(EASY_CVAR_GET_DEBUGONLY(crazyMonsterPrintouts))easyForcePrintLine("MaintainSchedule: INVALID B1. Schedule was: %s", getScheduleName());
				}
				else{
					//easyForcePrintLine("%s:%d MY SCHEDULE WAS INTERRUPTED. IT WAS %s", getClassname(), monsterID, getScheduleName());
					pNewSchedule = GetSchedule();
					if(EASY_CVAR_GET_DEBUGONLY(crazyMonsterPrintouts))easyForcePrintLine("MaintainSchedule: INVALID B2. Schedule was: %s", getScheduleName());
				}

				if (pNewSchedule == NULL) {
					// give up
					break;
				}

				//MODDD - yea.
				if(pickedScheduleThisCall && pNewSchedule == pPrevSchedule){
					if(EASY_CVAR_GET_DEBUGONLY(scheduleInterruptPrintouts)){easyForcePrintLine("!!! %s:%d Same schedule (%s) picked in the same frame, BLOCKED. <normal>", getClassname(), monsterID, m_pSchedule->pName);}
					//i = 999;
					queueStopAtScheduleEnd = TRUE;
					break;
				}

				pickedScheduleThisCall = TRUE;  // have now

				if(EASY_CVAR_GET_DEBUGONLY(crazyMonsterPrintouts) == 1){
					easyPrintLine("OOPS A PLENTY 9 %d:::new sched: %s", HasConditions(bits_COND_CAN_MELEE_ATTACK1), pNewSchedule->pName);
				}
				
				if(EASY_CVAR_GET_DEBUGONLY(crazyMonsterPrintouts))easyForcePrintLine("MaintainSchedule: NEW SCHEDULE WILL BE %s", pNewSchedule->pName);

				ChangeSchedule( pNewSchedule );
				if(EASY_CVAR_GET_DEBUGONLY(crazyMonsterPrintouts) == 1){
					easyPrintLine("OOPS A PLENTY 10 %d", HasConditions(bits_COND_CAN_MELEE_ATTACK1));
				}
			}

			if(EASY_CVAR_GET_DEBUGONLY(crazyMonsterPrintouts) == 1){
				easyPrintLine("OOPS A PLENTY 11 %d", HasConditions(bits_COND_CAN_MELEE_ATTACK1));
			}
		}//END OF valid check


		if ( m_iTaskStatus == TASKSTATUS_NEW )
		{	
			Task_t *pTask = GetTask();
			ASSERT( pTask != NULL );

			// HEY THERE!!! I DO THIS
			//    m_iTaskStatus = TASKSTATUS_RUNNING;
			// ITS PRETTY <dearly> IMPORTANT.  THANKYOU.
			// ...anyway, yes, that is all "TaskBegin" does, it is more of a signal for other logic to pay attention to.
			// As opposed to "StartTask" below, that is way different.  Uhh.  Sure.
			TaskBegin();
			
			//MODDD - change-schedule task complete / schedule failure check added.
			m_fNewScheduleThisFrame = FALSE; //reset.

			StartTask( pTask );

			// Is this section relevant anymore?  Test without it
			/*
			// Do the check.
			if( m_fNewScheduleThisFrame ){
				//If we recently called ChangeSchedule (this same frame) and the task ended up completed by a TaskComplete call or the schedule ended up failed by a TaskFail() in this
				//same frame, something did not go right.
		
				 if(HasConditions(bits_COND_TASK_FAILED)){
					 easyForcePrintLine("!!!CRITICAL: %s:%d, sched:%s task:%d. Fresh schedule failed before running a single task! REPORT THIS", getClassname(), monsterID, getScheduleName(), getTaskNumber());
					
					 ClearConditions(bits_COND_TASK_FAILED);
				 }else if( m_iTaskStatus == TASKSTATUS_COMPLETE){
					 easyForcePrintLine("!!!CRITICAL: %s:%d, sched:%s task:%d. Fresh schedule completed first task before running a single task! REPORT THIS", getClassname(), monsterID, getScheduleName(), getTaskNumber());
					 m_iTaskStatus = TASKSTATUS_NEW;
				 }
				 //Some correction for now, but still need to track any calls like these down and fix them. These automatic adjustments may not be all that is needed.
			}//END OF m_fNewScheduleThisFrame check

			if (HasConditions(bits_COND_TASK_FAILED)) {
				//HACK
				m_iTaskStatus = TASKSTATUS_NEW;
			}
			*/

		}//END OF if ( m_iTaskStatus == TASKSTATUS_NEW )



		if(EASY_CVAR_GET_DEBUGONLY(crazyMonsterPrintouts) == 1){
			easyPrintLine("OOPS A PLENTY 12 %d", HasConditions(bits_COND_CAN_MELEE_ATTACK1));
		}
		// UNDONE: Twice?!!!

		
		//MODDD TODO - add a check for " || signalActivityUpdate" too? may cause things to break, careful.
		// Wait, why change the activity to the ideal one in the middle of the activity to begin with?
		// In case of a schedule that starts/stops and changes the ideal from say, WALK to IDLE to WALK
		// within this loop, it's unnecessary to have the WALK to IDLE setting reset the frame and
		// animtime/interval when the user would just see it go from WALK to WALK, no change.
		// NOPE, sadly.  Disabling this section can horrifically break AI in some cases, most often
		// in simple attack schedules.
		// Just watch the hgrunt in a2a1.bsp.
		// Pretty sure what happens is, it picks the ACT_MELEE_ATTACK1,
		// it runs, it finishes, then on picking the exact same schedule again, it never gets a chance 
		// to reset the frame so it finishes again in the same frame.  So you're left with dead AI, at least
		// while something is standing in front of it.  Any reportai gets it with the NULL task?
		// Biggest issue is breaking when a lot of the scripted stuff works, I don't really get that.
		// Maybe it's on the hgrunts side instead in there for the most part, they just ignore the reaction
		// animations more often.

		
		if ( m_Activity != m_IdealActivity ){
			SetActivity ( m_IdealActivity );
		}
		

		/*
		if ( m_Activity != m_IdealActivity )
		{
			if(m_IdealActivity == ACT_RESET){
				easyPrintLine("excuse me kind sir but W A T");
			}
			signalActivityUpdate = FALSE;
			m_Activity = m_IdealActivity;
			m_fSequenceFinished = FALSE;
			m_fSequenceFinishedSinceLoop = FALSE;
			pev->frame = 0;
		}
		*/
		// 
		/*
		if(m_MonsterState != MONSTERSTATE_SCRIPT ){
		if (m_Activity != m_IdealActivity && (m_IdealActivity == ACT_RANGE_ATTACK1 || m_IdealActivity == ACT_RANGE_ATTACK2 || m_IdealActivity == ACT_MELEE_ATTACK1 || m_IdealActivity == ACT_MELEE_ATTACK2) )
		{
			SetActivity ( m_IdealActivity );
		}
		}
		*/
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		
		if(EASY_CVAR_GET_DEBUGONLY(crazyMonsterPrintouts) == 1){
			easyPrintLine("OOPS A PLENTY 13 %d", HasConditions(bits_COND_CAN_MELEE_ATTACK1));
		}
		
		// MODDD - IDEA.  What if we could pick a new schedule in the same frame that another one picked failed in the same frame?
		// Only if it is running do we assume we're sticking with it.
		// Also, don't run a task that failed while starting, that doesn't make much sense
		//if ( !TaskIsComplete() && m_iTaskStatus != TASKSTATUS_NEW )
		if (m_iTaskStatus == TASKSTATUS_RUNNING && !HasConditions(bits_COND_TASK_FAILED)){
			//|| m_iTaskStatus == TASKSTATUS_RUNNING_TASK || m_iTaskStatus == TASKSTATUS_RUNNING_MOVEMENT) {
			//break;


	//MODDD - IMPORTANT SECTION.
	//////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////

			// that does 'm_iTaskStatus != TASKSTATUS_COMPLETE', redundant with being TASKSTATUS_RUNNING as above wants
			//if (TaskIsRunning())
			{
				Task_t* pTask = GetTask();
				ASSERT(pTask != NULL);
				//easyForcePrintLine("IM GONNA EXPLODE sched:%s task:%d act:%d", this->getScheduleName(), pTask->iTask, m_Activity);

				//MODDD - change-schedule task complete / schedule failure check added.
				m_fNewScheduleThisFrame = FALSE; //reset.

				RunTask(pTask);

				//Do the check.
				if (m_fNewScheduleThisFrame) {
					//If we recently called ChangeSchedule (this same frame) and the task ended up completed by a TaskComplete call or the schedule ended up failed by a TaskFail() in this
					//same frame, something did not go right.

					if (HasConditions(bits_COND_TASK_FAILED)) {
						easyForcePrintLine("!!!CRITICAL: %s:%d, sched:%s task:%d. Fresh schedule failed before running a single task! REPORT THIS", getClassname(), monsterID, getScheduleName(), getTaskNumber());

						ClearConditions(bits_COND_TASK_FAILED);
					}
					else if (m_iTaskStatus == TASKSTATUS_COMPLETE) {
						easyForcePrintLine("!!!CRITICAL: %s:%d, sched:%s task:%d. Fresh schedule completed first task before running a single task! REPORT THIS", getClassname(), monsterID, getScheduleName(), getTaskNumber());
						m_iTaskStatus = TASKSTATUS_NEW;
					}
					//Some correction for now, but still need to track any calls like these down and fix them. These automatic adjustments may not be all that is needed.
				}//END OF m_fNewScheduleThisFrame check

			}//END OF TaskIsRunning()

			//////////////////////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////////////////////

			//MODDD
			if (this->m_pfnThink == NULL || this->m_pfnThink == &CBaseEntity::SUB_FadeOut) {
				// Some wacky hijinks, eh?  This loop goes byebye.
				// Forget this and script stuff will try to call an idle animation through another run-thru of this loop,
				// and it looks super awkward on anims that moved the actual scientist a good way from the origin
				// (supposed to do the fade-out part out of view, or ones that don't fade suddenly go from corpses to
				// standing up and animating, just not interactive.  Pretty dang weird stuff)
				break;
			}


			if (m_iTaskStatus == TASKSTATUS_RUNNING) {
				// still running?  Get out of this loop then, no point in going through more.
				break;
			}
			else {
				// go through the loop to pick another task/schedule, perhaps

				//TEST?  Retail would break out of this loop from having a task that hadn't finished in
				// the same frame after StartTask(), even if RunTask run once would've finished.
				// The 'Runtask' call was after this loop (only one run of that per MaintainShcedule call).
			}



		}//END OF TASKSTATUS_RUNNING check
			
		if(EASY_CVAR_GET_DEBUGONLY(crazyMonsterPrintouts) == 1){
			easyPrintLine("OOPS A PLENTY 14 %d", HasConditions(bits_COND_CAN_MELEE_ATTACK1));
		}

	}//END OF THE LOOP.  Come on now, let's not try and hide this.

	
	// OLD LOCATION OF IMPORTANT SECTION



	// UNDONE: We have to do this so that we have an animation set to blend to if RunTask changes the animation
	// RunTask() will always change animations at the end of a script!
	// Don't do this twice

	//MODDD - allowing for "signalActivityUpdate" to also trigger SetActivity... once.
	

	/*
	//MODDD - EXPERIMENTAL
	///////////////////////////////////////////////////////////////////////////////////
	if(oldSeq != pev->sequence){
		// changed by any other means?   UNHOLY
		return;
	}

	if(oldAct == m_Activity){
		// no net change in activity?  Restore old frame
		//	pev->frame = oldFrame;
	}

	// .... what.
	if(m_Activity == ACT_RESET && oldAct != ACT_RESET){
		m_Activity = (Activity)oldAct;
	}

	if (oldAct != m_Activity || signalActivityUpdate )
	///////////////////////////////////////////////////////////////////////////////////
	*/

	if ( m_Activity != m_IdealActivity || signalActivityUpdate )
	{
		//MODDD - well yea.
		signalActivityUpdate = FALSE;

		SetActivity ( m_IdealActivity );
	}


	/*
	if(g_queueSomeCondReset){
		g_queueSomeCondReset = FALSE;
		ClearConditions(bits_COND_NEW_ENEMY | bits_COND_ENEMY_DEAD);
	}
	*/


	/*
	// AGH.  No good.
	if(oldAct == m_IdealActivity && !usingCustomSequence){
		if(monsterID == 0){
			int x = 45;
		}
		pev->sequence = oldSeq;
		pev->frame = oldFrame;
		//MODDD - TODO: save/restore all sequence info?
	}
	*/

	if(EASY_CVAR_GET_DEBUGONLY(crazyMonsterPrintouts) == 1){
		easyPrintLine("CAN I MELEE COND1? %d", HasConditions(bits_COND_CAN_MELEE_ATTACK1));
	}
}//MaintainSchedule

//=========================================================
// RunTask 
//=========================================================
void CBaseMonster::RunTask ( Task_t *pTask )
{
	if(monsterID == 2){
		int x = 45;
	}
	
	//if(m_pSchedule == slPathfindStumped){
	//	easyForcePrintLine("STUMPED RunTask: %s:%d task: %d", getClassname(), monsterID, pTask->iTask);
	//}


	switch ( pTask->iTask )
	{
	case TASK_TURN_RIGHT:
	case TASK_TURN_LEFT:
	case TASK_TURN_RIGHT_FORCE_ACT:
	case TASK_TURN_LEFT_FORCE_ACT:
		{
			ChangeYaw( pev->yaw_speed );

			if ( FacingIdeal() )
			{
				TaskComplete();
			}
			break;
		}

	case TASK_PLAY_SEQUENCE_FACE_ENEMY:
	case TASK_PLAY_SEQUENCE_FACE_TARGET:
		{
			CBaseEntity *pTarget;

			if ( pTask->iTask == TASK_PLAY_SEQUENCE_FACE_TARGET )
				pTarget = m_hTargetEnt;
			else
				pTarget = m_hEnemy;
			if ( pTarget )
			{
				pev->ideal_yaw = UTIL_VecToYaw( pTarget->pev->origin - pev->origin );
				ChangeYaw( pev->yaw_speed );
			}
			//MODDD - and why do we need m_fSequenceFinishedSinceLoop now?  WHO KNOWS
			if ( m_fSequenceFinished || m_fSequenceFinishedSinceLoop)
				TaskComplete();
		}
		break;

	case TASK_PLAY_SEQUENCE:
	case TASK_PLAY_ACTIVE_IDLE:
		{
			if ( m_fSequenceFinished )
			{
				TaskComplete();
			}
			break;
		}
	case TASK_FACE_ENEMY:
		{
			// ??? Why did I do this?  It's still fine to face the LKP regardless, often the intent
			/*
			if (m_hEnemy == NULL) {
				//oops.
				TaskComplete();
				return;
			}
			*/

			//if(monsterID == 1)easyForcePrintLine("HOO MANN sched:%s ang:%.2f ideal:%.2f", m_pSchedule->pName, UTIL_AngleMod( pev->angles.y ), pev->ideal_yaw );
			MakeIdealYaw( m_vecEnemyLKP );
			ChangeYaw( pev->yaw_speed );
			//easyForcePrintLine("TASK_FACE_ENEMY: %s%d WHAT? yawdif:%.2f yaw_spd:%.2f", this->getClassname(), this->monsterID,    FlYawDiff(), pev->yaw_speed);
			if ( FacingIdeal() )
			{
				TaskComplete();
			}
			break;
		}
	//MODDD - new. No need
	case TASK_FACE_POINT:
	case TASK_FACE_HINTNODE:
	case TASK_FACE_LASTPOSITION:
	case TASK_FACE_TARGET:
	case TASK_FACE_IDEAL:
	case TASK_FACE_ROUTE:
	case TASK_FACE_PREV_LKP:
	case TASK_FACE_BEST_SOUND:
	case TASK_FACE_GOAL:
		{
			if(pTask->iTask == TASK_FACE_TARGET && this->m_hTargetEnt == NULL){
				// if told to face a target that does not / no longer exists, stop.
				TaskFail();
				break;
			}  

			ChangeYaw( pev->yaw_speed );

			if ( FacingIdeal() )
			{
				TaskComplete();
			}
			break;
		}
	case TASK_FACE_IDEAL_IF_VISIBLE:{
		// Clone, but another requirement:  is the ideal 'point' visible to me?
		// Should be set (assumed m_vecMoveGoal)
		if(pTask->iTask == TASK_FACE_TARGET && this->m_hTargetEnt == NULL){
			// if told to face a target that does not / no longer exists, stop.
			// (skip in this case, nothing to face)
			TaskComplete();
			break;
		}  

		if(!this->FVisible(m_vecMoveGoal)){
			// no line to the given point?  Don't fail, but don't change where you're facing.
			TaskComplete();
		}else{
			ChangeYaw( pev->yaw_speed );

			if ( FacingIdeal() )
			{
				TaskComplete();
			}
		}
		break;
	}

	case TASK_CHECK_STUMPED:
		{
			//Nothing yet, but if this has its own delay to wait for before letting the "unstumping" commence (re-route to the updated LKP that matches the real enemy position),
			//that would go here and call "TaskComplete()" when the delay is up.
		}
	break;
	case TASK_WAIT_PVS:
		{
			//MODDD - wait.  Why wasn't this skipped if the monsterstate is COMBAT?
			// The check in monsterstate.cpp lets being in COMBAT skip its PVS check for turning
			// enemy-checks on & off.
			if(m_MonsterState == MONSTERSTATE_COMBAT){
				TaskComplete();
				return;
			}

			//MODDD - alternate way to pass. If we're marked to ignore PVS checks, ignore this too.
			if(noncombat_Look_ignores_PVS_check()){
				TaskComplete();
				return;
			}

			if ( !FNullEnt(FIND_CLIENT_IN_PVS(edict())) )
			{
				TaskComplete();
			}
			break;
		}
	case TASK_WAIT_INDEFINITE:
		{
			// don't do anything.
			break;
		}


	case TASK_WAIT_ENEMY_LOOSE_SIGHT:{

		//keep looking at the enemy.
		MakeIdealYaw ( m_vecEnemyLKP );
		ChangeYaw( pev->yaw_speed ); 

		//only continue if we loose sight of the enemy. Presumably to start chasting to get them back in sight?
		if(!HasConditions(bits_COND_SEE_ENEMY)){
			TaskComplete();
		}	
	break;}
	case TASK_WAIT_ENEMY_ENTER_WATER:{
		MakeIdealYaw ( m_vecEnemyLKP );
		ChangeYaw( pev->yaw_speed ); 

		//Is the enemy in the water, presumably like ourselves?
		if(m_hEnemy == NULL){
			//what?
			TaskFail();
			return;
		}

		if(m_hEnemy->pev->waterlevel == 3){
			//They are in the water? Done waiting.
			//Is there a need for a bits_COND_SEE_ENEMY check too?  Probably not, we can route to them most likely.
			TaskComplete();
		}

	break;}

	case TASK_WAIT:
	case TASK_WAIT_RANDOM:
		{
			//MODDD NOTE - it may seem like a good idea to make any case of TASK_WAIT face
			// m_vecEnemyLKP, but it's hard to generalize when that is a good idea, best leave
			// up to being a more specific task like TASK_WAIT_LOOK or something, I forget what.
			// Anyway, there could be other times something is waiting, like hgrunts going
			// behind cover.
			// Turning to the direction of enemyLKP could lead them to staring at a wall for a bit.

			/*
			//MODDD - !!! Don't leave this on for long, testing.
			///////////////////////////////////////////////////////////////
			//if(m_fEnemyLKP_EverSet){
				// assume I'm trying to face that.
				MakeIdealYaw ( m_vecEnemyLKP );
				ChangeYaw( pev->yaw_speed );
			//}
			///////////////////////////////////////////////////////////////
			*/

			if ( gpGlobals->time >= m_flWaitFinished )
			{
				TaskComplete();
			}
			break;
		}
	case TASK_WAIT_STUMPED:
		{
			// (TASK_FACE_ENEMY)  Why not look at the LKP in the meantime?  Most likely place for the enemy to pop back in, right?
			///////////////////////////////////////////////////////////////
			if(m_fEnemyLKP_EverSet){
				// assume I'm trying to face that.
				ChangeYaw( pev->yaw_speed );
			}
			///////////////////////////////////////////////////////////////

			if ( gpGlobals->time >= m_flWaitFinished )
			{
				TaskComplete();
			}
			break;
		}
	case TASK_WAIT_FACE_ENEMY:
		{
			MakeIdealYaw ( m_vecEnemyLKP );
			ChangeYaw( pev->yaw_speed ); 

			if ( gpGlobals->time >= m_flWaitFinished )
			{
				TaskComplete();
			}
			break;
		}
	case TASK_WAIT_FACE_IDEAL:
		// Wait, but while waiting, face ideal.

		ChangeYaw( pev->yaw_speed );

		//if ( FacingIdeal() )
		//{
		//	TaskComplete();
		//}

		if ( gpGlobals->time >= m_flWaitFinished )
		{
			TaskComplete();
		}
	break;
	case TASK_MOVE_TO_TARGET_RANGE:
		{
			float distance;
			float targetDistToGoal_ChangeReq = pTask->flData*0.5;
			

			//easyForcePrintLine("WELL WHAT IS IT? NOTNULL? %d", (m_hTargetEnt != NULL));

			if ( m_hTargetEnt == NULL ){
				//easyPrintLine("TASK_MOVE_TO_TARGET_RANGE FAILED. null enemy.  My ID: %d", this->monsterID);
				TaskFail();
			}else
			{
				float targetDistToGoal = (m_vecMoveGoal - m_hTargetEnt->pev->origin).Length();
				float myDistToTargetEnt = (m_hTargetEnt->pev->origin - pev->origin).Length();

				//easyForcePrintLine("TARGETNAME: %s", m_hTargetEnt->getClassname());

				distance = ( m_vecMoveGoal - pev->origin ).Length();  //current distance from me to the movegoal, which may or may not exactly match my distance to the targetEnt's origin.
				// Re-evaluate when you think your finished, or the target has moved too far


				
				//"(distance < pTask->flData)" ? wouldn't that already end the method soon? Even with the distance changing a little bit?
				if(EASY_CVAR_GET_DEBUGONLY(movementIsCompletePrintout))easyForcePrintLine("RunTask TASK_MOVE_TO_TARGET_RANGE. I am %s:%d sched:%s myDistToGoal: %.2f myDistToGoalReq: %.2f myDistToTargetEnt: %.2f targetDistToGoal:%.2f reqToChange:%.2f",
					getClassname(),
					monsterID,
					getScheduleName(),
					distance,
					pTask->flData,
					myDistToTargetEnt,
					targetDistToGoal,
					targetDistToGoal_ChangeReq);

				if ( (FALSE) || (targetDistToGoal > targetDistToGoal_ChangeReq) )
				{
					if(EASY_CVAR_GET_DEBUGONLY(movementIsCompletePrintout))easyForcePrintLine("REEEEEROUTED");
					m_vecMoveGoal = m_hTargetEnt->pev->origin;
					
					//distance = ( m_vecMoveGoal - pev->origin ).Length2D();
					//distance = myDistToTargetEnt;

					BOOL successfulRefresh = FRefreshRoute();
					if(!successfulRefresh){TaskFail();/*easyForcePrintLine("YEAAAHGHHHHHHHHHHH")*/;break;} //Not a good refresh? Stop.
				}

				// Set the appropriate activity based on an overlapping range
				// overlap the range to prevent oscillation
				//MODDD - new possible satisfying condition. I am closer to my target's absolute position than the goaldist. I know, stunning.
				//if ( distance < pTask->flData || myDistToTargetEnt < pTask->flData )
				if(myDistToTargetEnt < pTask->flData)
				{
					if(EASY_CVAR_GET_DEBUGONLY(movementIsCompletePrintout))easyForcePrintLine("FINISH!!!");
					TaskComplete();
					RouteClear();		// Stop moving
					return;
				}
				else if ( distance < 190 && m_movementActivity != ACT_WALK )
					m_movementActivity = ACT_WALK;
				else if ( distance >= 270 && m_movementActivity != ACT_RUN )
					m_movementActivity = ACT_RUN;

			}//END OF m_hTargetEnt check

			break;
		}
	case TASK_MOVE_TO_ENEMY_RANGE:
		{
			//see TASK_WAIT_FOR_MOVEMENT_RANGE... interesting stuff.
			
			// NOTE - it is assumed pTask->flData and goalDistTolerance are one and the same here


			//...is this a good idea?  And why wasn't it needed all this time?!
			//ANSWER: because MoveExecute in basemonster.cpp, and (should?) in any child class implementing MoveExecute, needs to 
			//        tell it to set the ideal activity to the movement activity if it is not set that way.
			//        There are no checks against the movement activity being ACT_IDLE however, something else in there must change that.
			//        Look at FRefreshRouteChaseEnemySmart, and possibly FRefreshRoute. At least the former does force the movement activity 
			//        to ACT_RUN.
			//        TODO MAJOR - Perhaps asking the monster what move activity is preferred would be best so flyers can say ACT_HOVER or ACT_FLY instead?

			/* 
			if(this->m_IdealActivity != this->m_movementActivity || this->m_movementActivity == ACT_IDLE){
				//we need to force a fitting move activity.
				if ( LookupActivity( ACT_RUN ) != ACTIVITY_NOT_AVAILABLE ){
					m_movementActivity = ACT_RUN;
				}
				else{
					m_movementActivity = ACT_WALK;
				}
			}
			*/


			if (MovementIsComplete()){
				//easyForcePrintLine("I GOT YOU!");
				//TaskComplete();
				//RouteClear();
				//MODDD - no, restsart! If there is a case this never causes this task to end to say, re-route at an apparent dead-end, need to adjust this.

				//No, just TaskComplete() as usual...
				//BOOL test = FRefreshRoute();
				//if(!test){if(EASY_CVAR_GET_DEBUGONLY(movementIsCompletePrintout))easyForcePrintLine("TASK_MOVE_TO_ENEMY_RANGE: FAILURE 2"); TaskFail();}else{if(EASY_CVAR_GET_DEBUGONLY(movementIsCompletePrintout))easyForcePrintLine("TASK_MOVE_TO_ENEMY_RANGE: SUCCESS 2");};

				//break; //is stopping this early this frame here necessary?

				if(EASY_CVAR_GET_DEBUGONLY(movementIsCompletePrintout))easyPrintLine("TASK_MOVE_TO_ENEMY_RANGE success??? My ID: %d", this->monsterID);
				
				TaskComplete();
				return;
			}



			//float distance;

			if ( m_hEnemy == NULL ){
				if(EASY_CVAR_GET_DEBUGONLY(movementIsCompletePrintout))easyPrintLine("TASK_MOVE_TO_ENEMY_RANGE FAILED THE EMPRAH. null enemy. My ID: %d", this->monsterID);
				TaskFail();
			}else{


				//or m_hEnemy->pev->origin ???
				Vector& vecDestination = m_vecEnemyLKP;

				//distance = ( m_vecMoveGoal - pev->origin ).Length2D();


				// Re-evaluate when you think your finished, or the ENEMY has moved too far
				

				//only re-route early (2nd condition here) IF we can physically see the enemy (in view cone)
				//

				//2000 200
				//1000 100
				//400 40

				BOOL redoPass = FALSE;




				//which one?
				// ALSO STILL... headcrab jumpiness and hassassin backwards anim events + restore out.
				// --- 2D and Z-wise to compare separately maybe?
				//const float enemyRealDist = (pev->origin - vecDestination).Length();

				// How about goal node and ZCheck?
				//const float enemyRealDist = (pev->origin - m_vecMoveGoal).Length();
				float distToGoal2D = -1;
				BOOL theZCheck = FALSE;

				/*
				WayPoint_t* waypointGoalRef;
				waypointGoalRef = GetGoalNode();

				if(waypointGoalRef != NULL){
					distToGoal2D = (waypointGoalRef->vecLocation - pev->origin ).Length2D();
					// Not yet...
					//theZCheck = ZCheck(pev->origin, waypointGoalRef->vecLocation);
				}
				*/
				// GAH.  We do want to m_vecMoveGoal, location of the enemy itself.
				distToGoal2D = (pev->origin - m_vecMoveGoal).Length();




				//interestingly enough, "FInViewCone" does not count as a Visible check.
				//As in, if there is a solid wall between this monster and the enemy, FInViewCone will still report "true" if this monster is facing the enemy regardless.
				//ALSO, if any damage has been sustained, this monster will forcibly re-route. Kind of like an "OH there you are!" moment.

				// how often is this the reason?
				BOOL hasDamageCond = HasConditions(bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE);

				if( (FInViewCone( m_hEnemy ) && FVisible(m_hEnemy)) || hasDamageCond ){

					//setEnemyLKP(m_hEnemy);

					//MODDD NOTE - should we check to see there is a clear line of sight with the enemy, regardless of facing direction, before forcing a reroute? ){

					if(EASY_CVAR_GET_DEBUGONLY(movementIsCompletePrintout)){
						//include it. Clearly we care.
						::DebugLine_Setup(0, m_vecMoveGoal - Vector(0,0,8), m_vecMoveGoal+Vector(0, 0, 8), 0, 255, 0);
						::DebugLine_Setup(1, vecDestination - Vector(0,0,8), vecDestination+Vector(0, 0, 8), 255, 0, 0);

					}
					
					/*
					if(pTask->flData <= 1){
						//code! Use the flexible test.
						const float enemyRealDist = (pev->origin - vecDestination).Length();
						redoPass = ((m_vecMoveGoal - vecDestination).Length() > enemyRealDist*pTask->flData);
					}else{
						redoPass = ((m_vecMoveGoal - vecDestination).Length() > pTask->flData);
					}
					*/
					//easyForcePrintLine("TEST: e:%.2f m:%.2f a:%.2f", enemyRealDist, (m_vecMoveGoal - vecDestination).Length2D(), enemyRealDist*0.2);
					//distance between the enemy and where I was told to go (position of the enemy since doing a path-find before)

					if(this->m_movementGoal == MOVEGOAL_LOCATION){
						// Picked a 'buildNearestRoute', do NOT refresh from being a distance.  That's kinda the point.
						// Attempting to route every single frame an enemy is within sight but in an unreachable area 
						// (pathfind guaranteed to fail) sound terrifically wasteful.
						// Let routine route checks for a better direct one work at 1-second intervals, much cheaper.
						// In fact, right here!
						
						if(gpGlobals->time >= nextDirectRouteAttemptTime){
							nextDirectRouteAttemptTime = gpGlobals->time + 1;
							
							// ok, here we go.  It applies the new route automatically if it worked.
							// return code is more for curiosity's sake.
							BOOL theResult = FRefreshRouteChaseEnemySmartSafe();
							// PRINTOUT HERE MAYBE?

							int x = 45;
						}//nextDirectRouteAttemptTime check

						int x = 45;
					}else{
						// enemy got too far from the goal and we see they're goo far?   go ahead then, refresh.
						//redoPass = (enemyPathGoalDistance > enemyRealDist*0.2); //((m_vecMoveGoal - vecDestination).Length2D() > enemyRealDist*0.3);
						// (m_vecMoveGoal is straight to the enemy so it can be trusted)
						float enemyPathGoalDistance = (m_vecMoveGoal - vecDestination).Length();
						redoPass = (enemyPathGoalDistance > distToGoal2D * 0.2);

					}
				}// enemy in sight or damaged recently checks


				if (
					//(enemyRealDist < pTask->flData) ||
					(redoPass)
					)
				{
					//if "m_vecMoveGoal" even matters. moveGoal was set to ENEMY in the startTask case for this task.
					//m_vecMoveGoal = vecDestination;
					//distance = ( m_vecMoveGoal - pev->origin ).Length2D();
					//distance = 0; //do not allow this to pass.

					//COMPARe. Does refreshRoute handle the above, TASK_GET_PATH_TO_ENEMY, adequately?
					//FRefreshRoute();

					m_vecMoveGoal = m_vecEnemyLKP;
					//BOOL test = FRefreshRoute();
					//MODDD - WHY DID YOU USE THE OLD VERSION ANYWAYS?!
					// Or don't, hell if I know.
					//  ...  FRefreshRouteChaseEnemySmart
					BOOL test = FRefreshRouteChaseEnemySmart();
					nextDirectRouteAttemptTime = gpGlobals->time + 1;
					
					if(!test){
						if(EASY_CVAR_GET_DEBUGONLY(movementIsCompletePrintout))easyForcePrintLine("TASK_MOVE_TO_ENEMY_RANGE: FAILURE 3");
						TaskFail();
					}else{
						if(EASY_CVAR_GET_DEBUGONLY(movementIsCompletePrintout))easyForcePrintLine("TASK_MOVE_TO_ENEMY_RANGE: SUCCESS 3");

						if(MovementIsComplete()){
							TaskComplete();
							return;
						}
					}

				}//redoPass check


				//easyForcePrintLine("WHAT???!!! %.2f %.2f", distance, pTask->flData);

				// Set the appropriate activity based on an overlapping range
				// overlap the range to prevent oscillation


				BOOL distaPass;
				// Wait no!  Always be to m_vecMoveGoal, that is where the enemy is.
				// If we happen to end close to that, not any route to any nearest node, end early.
				//if(this->m_movementGoal == MOVEGOAL_ENEMY){
					distaPass = (distToGoal2D < pTask->flData);
				//}else{
				//	distaPass = (
				//}

				if ( distaPass )
				{
					//TaskComplete();
					//RouteClear();		// Stop moving
					//MODDD - NO. This alls for a re-route.
					
					m_vecMoveGoal = m_vecEnemyLKP;
					BOOL test = FRefreshRouteChaseEnemySmart();

					if(!test){
						if(EASY_CVAR_GET_DEBUGONLY(movementIsCompletePrintout))easyForcePrintLine("TASK_MOVE_TO_ENEMY_RANGE: FAILURE 4");
						TaskFail();
					}else{
						if(EASY_CVAR_GET_DEBUGONLY(movementIsCompletePrintout))easyForcePrintLine("TASK_MOVE_TO_ENEMY_RANGE: SUCCESS 4");

						if(MovementIsComplete()){
							TaskComplete();
							return;
						}
					}

				}
				else{
					if ( LookupActivity( ACT_RUN ) != ACTIVITY_NOT_AVAILABLE )
					{
						m_movementActivity = ACT_RUN;
					}
					else
					{
						m_movementActivity = ACT_WALK;
					}
				}
				/*
				else if ( distance < 190 && m_movementActivity != ACT_WALK )
					m_movementActivity = ACT_WALK;
				else if ( distance >= 270 && m_movementActivity != ACT_RUN )
					m_movementActivity = ACT_RUN;
					*/
			}
			break;
		}
	//MODDD - clone of "TASK_MOVE_TO_TARGET_RANGE".
	case TASK_MOVE_TO_POINT_RANGE:
		{
		float distance;

		//easyForcePrintLine("RUNNIN AND RUNNIN");

		/*
		if ( m_hTargetEnt == NULL ){
			easyPrintLine("I HAVE FAILED THE EMPRAH.  ID: %d", this->monsterID);
			TaskFail();
		}else
		*/
			
			distance = ( m_vecMoveGoal - pev->origin ).Length2D();
			//no updates. It is a static location that anything can update if it chooses to. 
			//  m_vecMoveGoal    that is.

			// Set the appropriate activity based on an overlapping range
			// overlap the range to prevent oscillation
			if ( distance < pTask->flData )
			{
				//easyForcePrintLine("%s:%d O NOW! %.2f %.2f", this->getClassnameShort(), this->monsterID, distance, pTask->flData);
				TaskComplete();
				RouteClear();		// Stop moving
			}
			else if ( distance < 350 && m_movementActivity != ACT_WALK )
				m_movementActivity = ACT_WALK;
			else if ( distance >= 500 && m_movementActivity != ACT_RUN )
				m_movementActivity = ACT_RUN;

			break;
		}
	case TASK_WAIT_FOR_MOVEMENT:{
		if(EASY_CVAR_GET_DEBUGONLY(movementIsCompletePrintout) == 1){
			easyPrintLine("%s:%d: IS MOVEMENT COMPLETE?: %d", getClassname(), monsterID, MovementIsComplete());
			easyPrintLine("MOVEGOAL: %d", this->m_movementGoal);

			if(this->m_movementGoal == MOVEGOAL_LOCATION){
				UTIL_printLineVector("GOAL LOC:", this->m_vecMoveGoal);
			}

		}
		if (MovementIsComplete())
		{
			TaskComplete();
			RouteClear();		// Stop moving
		}
	break;}
	case TASK_WAIT_FOR_MOVEMENT_TIMED:{
		// Same as above, but will be interrupted if too much time passes.
		if(gpGlobals->time >= waitForMovementTimed_Start){
			TaskFail();
			break;
		}
		if (MovementIsComplete())
		{
			TaskComplete();
			RouteClear();		// Stop moving
		}

	break;}
	case TASK_WAIT_FOR_MOVEMENT_RANGE:{
		if (MovementIsComplete())
		{
			TaskComplete();
			RouteClear();		// Stop moving
		}else{
			//another chance...
			//easyForcePrintLine("WHAT %d", m_iRouteIndex);

			//if m_iRouteIndex is 0, we are on our way to the GOAL node, as opposed to going past any corners.  Check the distance to see if we are close enough.
				
			/*
			if(m_iRouteIndex > -1){
				easyForcePrintLine("TASK_WAIT_FOR_MOVEMENT_RANGE mg:%d ri:%d :::F:%d g:%d e:%d d:%d", m_movementGoal, m_iRouteIndex, m_Route[m_iRouteIndex].iType, m_Route[m_iRouteIndex].iType&bits_MF_IS_GOAL, m_Route[m_iRouteIndex].iType&bits_MF_TO_ENEMY, m_Route[m_iRouteIndex].iType&bits_MF_TO_DETOUR);
			}else{
				easyForcePrintLine("TASK_WAIT_FOR_MOVEMENT_RANGE mg:%d ri:-1", m_movementGoal);
			}
			*/

			//NOTICE - the "m_iRouteIndex == 0" is bad. 0 does not always have to be the final goal of a path!
			//this->m_movementGoal
			if(m_Route[m_iRouteIndex].iType & bits_MF_IS_GOAL){
				float distToGoal = ( m_Route[ m_iRouteIndex ].vecLocation - pev->origin ).Length();
				//easyForcePrintLine("TASK_WAIT_FOR_MOVEMENT_RANGE: distToGoal:%.2f req:%.2f", distToGoal, pTask->flData);
				if(distToGoal <= pTask->flData){
					//done!
					TaskComplete();
					RouteClear();		// Stop moving
				}
			}

		}
	break;}
	// NEW.  Stop when the end of my route is in the viewcone, and within the given custom distance (if non-zero).
	case TASK_WAIT_FOR_MOVEMENT_GOAL_IN_SIGHT:{
		if (MovementIsComplete())
		{
			TaskComplete();
			RouteClear();		// Stop moving
		}else{
			// No, get the goal node instead.  Kinda dumb to get really close to the goal just because more than one
			// node separates this monster from it
			//if(m_Route[m_iRouteIndex].iType & bits_MF_IS_GOAL){
			WayPoint_t* theGoalNode = GetGoalNode();
			if(theGoalNode == NULL){
				// oops?
				TaskFail();
			}else{
				// Can I see the goal node?
				BOOL inDaViewcone = FInViewCone(&theGoalNode->vecLocation);
				if(inDaViewcone){
					// One more check: no further than flData (if non-zero)?
					float distToGoal = Distance(pev->origin, theGoalNode->vecLocation);
					if(pTask->flData==0 || distToGoal <= pTask->flData){
						TaskComplete();
						RouteClear();		// Stop moving
					}
				}
			}
		}
	break;}
	case TASK_DIE:
	{
		//Is the end 255 or 256?!
		if ( m_fSequenceFinished && pev->frame >= 255 )
		{
			//MODDD - FOR DEBUGGING
			if(EASY_CVAR_GET_DEBUGONLY(drawCollisionBoundsAtDeath) == 1){
				UTIL_drawBox(pev->origin + pev->mins, pev->origin + pev->maxs);
			}
			if(EASY_CVAR_GET_DEBUGONLY(drawHitBoundsAtDeath) == 1){
				UTIL_drawBox(pev->absmin, pev->absmax);
			}
			//MODDD - does not seem effective, scrapped.
				
			/*
			CBaseMonster::smartResize();
			//if(gpGlobals->time > 20 && gpGlobals->time < 21){
				//UTIL_printoutVector(pev->origin);
				easyPrintLine("GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG");
				easyPrintLine("GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG");
				easyPrintLine("GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG");
				easyPrintLine("GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG");
				easyPrintLine("GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG");
				easyPrintLine("ahhh %.2f %.2f %.2f, %.2f %.2f %.2f", pev->mins.x, pev->mins.y, pev->mins.z, pev->maxs.x, pev->maxs.y, pev->maxs.z );
				UTIL_drawBox(pev->origin + pev->mins, pev->origin + pev->maxs);
			//}
			*/
				
			DeathAnimationEnd();
			
			//MODDD
			//pev->solid = SOLID_TRIGGER;
			//pev->movetype = MOVETYPE_NONE;

			/*
			//pev->movetype = MOVETYPE_NOCLIP;
				
			//MODDD - set size.
			//UTIL_SetSize ( pev, Vector ( -64, -64, -64 ), Vector ( 64, 64, 64 ) );

			UTIL_SetSize( pev, Vector(-16, -16, -32), Vector(16, 16, 0) );

			//pev->classname = MAKE_STRING("monster_barnacle");

			pev->solid			= SOLID_SLIDEBOX;
			pev->movetype		= MOVETYPE_NONE;
			*/
		}
		break;
	}
	case TASK_DIE_SIMPLE:{
		//Cloned from ichy.
		//MODDD NEW - this is called for just calling TaskComplete() instead of doing a lot of standard death (resize + kill think call).
		//            Handle it yourself in the next task.
		//MODDD NOTE - this is... quite different from how the base monster handles TASK_DIE.
		//             If this schedule is completed, it must kill the think method like TASK_DIE usually would.
		//    Otherwise, GetSchedule will happen again at the end and say "I feel like being dead now... I pick a death animation!"
		//    and loop the death anim forever.

		if ( m_fSequenceFinished )
		{
			pev->deadflag = DEAD_DEAD;

			TaskComplete( );
		}
	break;}

	case TASK_DIE_LOOP:{
		//No default behavior. Runs indefinitely unless a child class implements this and tells it when to stop with "TaskComplete" and let TASK_DIE proceed as usual
		//(pick a typical death sequence, run until the end and freeze at the last frame).

		//CHANGE OF PLANS. For falling flyers, no need to give this any special behavior, the generic "KilledFinishTouch" will work fine in progressing for this.
	break;}
	//MODDD - MOVED FROM ichthyosaur.cpp, renamed from TASK_ICHTHYOSAUR_FLOAT
	case TASK_WATER_DEAD_FLOAT:{

		// If not in water and there is no velocity, assume the reason is from having landed.  Adding that flag check to be safe.
		if(pev->waterlevel == 0 && pev->velocity.Length() < 0.001 && pev->flags & FL_ONGROUND){
			//on land and stopped moving? stop.
			DeathAnimationEnd();
			pev->movetype = MOVETYPE_TOSS; //fall if whatever this is on top of stops?  Or after this, as it should be on the ground?
			// can't float again without think logic though but that doesn't need to happen anyway, expecte to sink after so long so
			// it will just stay that way once it does.
			TaskComplete();
			return;
		}

		pev->angles.x = UTIL_ApproachAngle( 0, pev->angles.x, 20 );
		pev->velocity = pev->velocity * 0.8;
		if (pev->waterlevel > 1 && pev->velocity.z < 64)
		{
			//MODDD NOTE - below water? go up.
			pev->velocity.z += floatSinkSpeed;
		}
		else 
		{
			//MODDD NOTE - above water? go down.
			pev->velocity.z -= floatSinkSpeed;

			//MODDD - make the amount of sinking velocity a variable, starting at 8 at the start of this task.
			//Reduce by half each time it reaches the surface.
			//When it is sufficiently small, end this task and maybe shift the origin.

			//if( fabs(pev->velocity.z) <= 16){
			//}
		}

		if(oldWaterLevel != -1 && pev->waterlevel != oldWaterLevel && pev->waterlevel <= 1){
			//if we have an old water level to compare to, it does not match the current, and the current water level is above water...
			//cut the floatSinkSpeed in half
			floatSinkSpeed = floatSinkSpeed / 2;
		}

		oldWaterLevel = pev->waterlevel;

		if(floatSinkSpeed < 0.4 && floatSinkSpeed != 0){
			floatSinkSpeed = 0;

			// If we're slow enough above water, go ahead and stop.
			Vector adjustedOrigin = pev->origin + Vector(0, 0, -0.4);  //does this help make it visible from below and above water?
			::UTIL_SetOrigin(pev, adjustedOrigin);
			pev->velocity.z = 0;


			// wait... water level might lower.  SIGH.
			// Could the think-time slow down, maybe?  Seems wasteful to have 'should I still be floating' cycles
			// for all eternity.
			// IDEA:  have a timer, when this expires, then sink.
			// the initial.  But don't depend on floatSinkSpeed being small, let it apply anytime.
			// It's long enough to not interrupt this anyway.

		}//floatSinkSpeed check


		if(gpGlobals->time >= floatEndTimer){
			DeathAnimationEnd();  //kill the think method. done.
			pev->movetype = MOVETYPE_TOSS; // fall if whatever this is on top of stops?

			if(pev->waterlevel == 0){
				// nothing special
			}else{
				// float more slowly to the bottom
				pev->gravity = 0.2;
			}

			TaskComplete();
		}

		// ALERT( at_console, "%f\n", pev->velocity.z );
	break;}


	case TASK_RANGE_ATTACK1_NOTURN:
	case TASK_RANGE_ATTACK2_NOTURN:
	case TASK_MELEE_ATTACK1_NOTURN:
	case TASK_MELEE_ATTACK2_NOTURN:
	case TASK_RELOAD_NOTURN:
	{
		//MODDD - BIG UGLY SECTION, make its own method if this turns out alright
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// canPredictRangedActFinish() ||  ???
		// m_fSequenceLoops || 

		if (
			( (pTask->iTask == TASK_MELEE_ATTACK1_NOTURN || pTask->iTask == TASK_MELEE_ATTACK2_NOTURN) && !m_fSequenceLoops) ||
			(predictRangeAttackEnd() && (pTask->iTask == TASK_RANGE_ATTACK1_NOTURN || pTask->iTask == TASK_RANGE_ATTACK2_NOTURN))
		) {

			float flInterval = 0.1;  //mock interval, resembles think times.
			float recentFrameAdvancePrediction = flInterval * m_flFrameRate * pev->framerate * EASY_CVAR_GET_DEBUGONLY(animationFramerateMulti);
			float tempCutoff;
			if (animFrameCutoff != -1) {
				tempCutoff = animFrameCutoff;
			}
			else {
				//depends on framerate for direction.
				tempCutoff = (pev->framerate >= 0) ? 256 : 0;
			}
			BOOL pass = FALSE;
			if (pev->framerate < 0) {
				if (pev->frame + recentFrameAdvancePrediction <= tempCutoff) {
					pass = TRUE;
				}
			}
			else {
				if (pev->frame + recentFrameAdvancePrediction >= tempCutoff) {
					pass = TRUE;
				}
			}
			if (pass) {
				//end now!
				//pev->frame = 0;   // for next time?  blah.
				//m_fSequenceFinished = TRUE;
				// re-pick me to happen again
				m_Activity = ACT_RESET;
				m_IdealActivity = ACT_RESET;

				//pev->framerate = 0;

				TaskComplete();
				return;
			}
		}
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


		if ( m_fSequenceFinished )
		{
			//MODDD - removed, see below.  REtail behavior was ONLY this line
			//m_Activity = ACT_RESET;

			if(canPredictActRepeat()){
				switch( pTask->iTask ){
					case TASK_RANGE_ATTACK1:{predictActRepeat(bits_COND_CAN_RANGE_ATTACK1); break;}
					case TASK_RANGE_ATTACK2:{predictActRepeat(bits_COND_CAN_RANGE_ATTACK2); break;}
					case TASK_MELEE_ATTACK1:{predictActRepeat(bits_COND_CAN_MELEE_ATTACK1); break;}
					case TASK_MELEE_ATTACK2:{predictActRepeat(bits_COND_CAN_MELEE_ATTACK2); break;}
					case TASK_SPECIAL_ATTACK1:{predictActRepeat(bits_COND_SPECIAL1); break;}
					case TASK_SPECIAL_ATTACK2:{predictActRepeat(bits_COND_SPECIAL2); break;}
				}//END OF inner switch
			}

			/*
			if(m_Activity != ACT_RESET){
				//HACKY MC HACKERSAXXX
				// it doesn't look like this is doing anything.
				ChangeSchedule(GetSchedule());
				return.
			}
			*/
			TaskComplete();
		}
	break;}
	case TASK_RANGE_ATTACK1:
	case TASK_RANGE_ATTACK2:
	case TASK_MELEE_ATTACK1:
	case TASK_MELEE_ATTACK2:
	case TASK_SPECIAL_ATTACK1:
	case TASK_SPECIAL_ATTACK2:
	case TASK_RELOAD:
	{
		lookAtEnemyLKP();

		//MODDD - BIG UGLY SECTION, make its own method if this turns out alright.
		// Also don't try attackEnd prediction on melee if the sequence loops, it just creates a moment of glitchess
		// on the loop
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		if (
			( (pTask->iTask == TASK_MELEE_ATTACK1 || pTask->iTask == TASK_MELEE_ATTACK2) && !m_fSequenceLoops) || 
			(predictRangeAttackEnd() && (pTask->iTask == TASK_RANGE_ATTACK1 || pTask->iTask == TASK_RANGE_ATTACK2))
		) {

			float flInterval = 0.1;  //mock interval, resembles think times.
			float recentFrameAdvancePrediction = flInterval * m_flFrameRate * pev->framerate * EASY_CVAR_GET_DEBUGONLY(animationFramerateMulti);
			float tempCutoff;
			if (animFrameCutoff != -1) {
				tempCutoff = animFrameCutoff;
			}
			else {
				//depends on framerate for direction.
				tempCutoff = (pev->framerate >= 0) ? 256 : 0;
			}
			BOOL pass = FALSE;
			if (pev->framerate < 0) {
				if (pev->frame + recentFrameAdvancePrediction <= tempCutoff) {
					pass = TRUE;
				}
			}
			else {
				if (pev->frame + recentFrameAdvancePrediction >= tempCutoff) {
					pass = TRUE;
				}
			}
			if (pass) {
				//end now!
				//pev->frame = 0;   // for next time?  blah.
				//m_fSequenceFinished = TRUE;
				// re-pick me to happen again
				m_Activity = ACT_RESET;
				m_IdealActivity = ACT_RESET;

				//pev->framerate = 0;

				TaskComplete();
				return;
			}
		}
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



		if ( m_fSequenceFinished ){
			//MODDD NOTE - BEWARE. This is likely to pick the same range attack activity again if the ideal activity remains that way.
 			//m_Activity = ACT_RESET;

			if(canPredictActRepeat()){
				switch( pTask->iTask ){
					case TASK_RANGE_ATTACK1:{predictActRepeat(bits_COND_CAN_RANGE_ATTACK1); break;}
					case TASK_RANGE_ATTACK2:{predictActRepeat(bits_COND_CAN_RANGE_ATTACK2); break;}
					case TASK_MELEE_ATTACK1:{predictActRepeat(bits_COND_CAN_MELEE_ATTACK1); break;}
					case TASK_MELEE_ATTACK2:{predictActRepeat(bits_COND_CAN_MELEE_ATTACK2); break;}
					case TASK_SPECIAL_ATTACK1:{predictActRepeat(bits_COND_SPECIAL1); break;}
					case TASK_SPECIAL_ATTACK2:{predictActRepeat(bits_COND_SPECIAL2); break;}
				}//END OF inner switch
			}
			
			/*
			if(m_Activity != ACT_RESET){
				//HACKY MC HACKERSAXXX
				// it doesn't look like this is doing anything.
				//...Now calling this and then "MaintainSchedule"?  That would be truly dastardly.
				ChangeSchedule(GetSchedule());
				return.
			}
			*/

			TaskComplete();
		}
	break;}
	case TASK_SMALL_FLINCH:
		{
			if ( m_fSequenceFinished )
			{
				TaskComplete();
			}
		}
		break;
	case TASK_BIG_FLINCH:{
		if ( m_fSequenceFinished )
		{
			TaskComplete();
		}
	break;}
	case TASK_WAIT_FOR_SCRIPT:
		{
			if ( m_pCine->m_iDelay <= 0 && gpGlobals->time >= m_pCine->m_startTime )
			{
				TaskComplete();
				m_pCine->StartSequence( (CBaseMonster *)this, m_pCine->m_iszPlay, TRUE );
				if ( m_fSequenceFinished )
					ClearSchedule();
				pev->framerate = 1.0;
				//ALERT( at_aiconsole, "Script %s has begun for %s\n", STRING( m_pCine->m_iszPlay ), STRING(pev->classname) );
			}
			break;
		}
	case TASK_PLAY_SCRIPT:
		{
			if (m_fSequenceFinished)
			{
				m_pCine->SequenceDone( this );
			}
			break;
		}

	
	//MODDD - new
	case TASK_WAIT_FOR_SEQUENCEFINISH:{

		//easyForcePrintLine("!!!!!!!!!!!!!!!!!!!!!!! %d", m_fSequenceFinished);
		//BEWARE: looping anims may just keep going!  
		//If necessary, anims could have a separate "loopedOnce" flag to be set when the anim would have usually ended but decided to loop instead, that is read HERE instead.
		if(m_fSequenceFinished || m_fSequenceFinishedSinceLoop){

			//MODDD - hoping that doesn't break anything. Not that usingCustomSequence was everpart of retail to begin with.
			this->usingCustomSequence = FALSE;

			TaskComplete();
		}


	break;}


	}//END OF switch(...)

}//END OF RunTask(...)

//=========================================================
// SetTurnActivity - measures the difference between the way
// the monster is facing and determines whether or not to
// select one of the 180 turn animations.
//=========================================================
void CBaseMonster::SetTurnActivity ( void ){
	float flYD;
	flYD = FlYawDiff();
	
	//MODDD - new. Remember the old activity before turning. Not that this is used in many places yet.
	// Might not need it at all.
	// Also this assumption is kindof bad, could very well already be in the TURN_LEFT or TURN_RIGHT
	// activities before calling SetTurnActivity.  A remembered TURN_LEFT/RIGHT is not very helpful.
	//m_IdealActivityBeforeTurn = m_IdealActivity;

	if ( flYD <= -45 && LookupActivity ( ACT_TURN_RIGHT ) != ACTIVITY_NOT_AVAILABLE )
	{// big right turn
		m_IdealActivity = ACT_TURN_RIGHT;
	}
	else if ( flYD > 45 && LookupActivity ( ACT_TURN_LEFT ) != ACTIVITY_NOT_AVAILABLE )
	{// big left turn
		m_IdealActivity = ACT_TURN_LEFT;
	}

}//END OF SetTurnActivity


//MODDD - same as above but forces the turn activity.
void CBaseMonster::SetTurnActivityForceAct ( void ){
	float flYD;
	flYD = FlYawDiff();

	if ( flYD <= -45 && LookupActivity ( ACT_TURN_RIGHT ) != ACTIVITY_NOT_AVAILABLE )
	{// big right turn
		//m_IdealActivity = ACT_TURN_RIGHT;
		SetActivity(ACT_TURN_RIGHT);
	}
	else if ( flYD > 45 && LookupActivity ( ACT_TURN_LEFT ) != ACTIVITY_NOT_AVAILABLE )
	{// big left turn
		//m_IdealActivity = ACT_TURN_LEFT;
		SetActivity(ACT_TURN_LEFT);
	}

}//END OF SetTurnActivityForceAct






//=========================================================
// Start task - selects the correct activity and performs
// any necessary calculations to start the next task on the
// schedule. 
//=========================================================
void CBaseMonster::StartTask ( Task_t *pTask )
{
	//easyForcePrintLine("StartTask sched:%s: task:%d index:%d", getScheduleName(), pTask->iTask, m_iScheduleIndex);


	//if(m_pSchedule == slPathfindStumped){
	//	easyForcePrintLine("STUMPED StartTask: %s:%d task: %d", getClassname(), monsterID, pTask->iTask);
	//}


	/*
	switch(pTask->iTask){
	case TASK_PLAY_SEQUENCE_FACE_ENEMY:
	case TASK_PLAY_SEQUENCE_FACE_TARGET:
	case TASK_PLAY_SEQUENCE:
	if(pTask->flData == ACT_EXCITED || pTask->flData == ACT_CROUCH || pTask->flData == ACT_CROUCHIDLE){
		if(FClassnameIs(this->pev, "monster_scientist")){easyForcePrintLine("I WILL think unpleasant thoughts");}
	}
	break;
	}
	*/


	switch ( pTask->iTask )
	{
	case TASK_TURN_RIGHT:{
			float flCurrentYaw;
			flCurrentYaw = UTIL_AngleMod( pev->angles.y );
			pev->ideal_yaw = UTIL_AngleMod( flCurrentYaw - pTask->flData );
			SetTurnActivity();
	break;}
	case TASK_TURN_LEFT:{
			float flCurrentYaw;
			flCurrentYaw = UTIL_AngleMod( pev->angles.y );
			pev->ideal_yaw = UTIL_AngleMod( flCurrentYaw + pTask->flData );
			SetTurnActivity();
	break;}
	case TASK_TURN_RIGHT_FORCE_ACT:{
			float flCurrentYaw;
			flCurrentYaw = UTIL_AngleMod( pev->angles.y );
			pev->ideal_yaw = UTIL_AngleMod( flCurrentYaw - pTask->flData );
			SetTurnActivityForceAct();
	break;}
	case TASK_TURN_LEFT_FORCE_ACT:{
			float flCurrentYaw;
			flCurrentYaw = UTIL_AngleMod( pev->angles.y );
			pev->ideal_yaw = UTIL_AngleMod( flCurrentYaw + pTask->flData );
			SetTurnActivityForceAct();
	break;}
	case TASK_REMEMBER:
		{
			Remember ( (int)pTask->flData );
			TaskComplete();
			break;
		}
	case TASK_FORGET:
		{
			Forget ( (int)pTask->flData );
			TaskComplete();
			break;
		}
	case TASK_FIND_HINTNODE:
		{
			m_iHintNode = FindHintNode();

			if ( m_iHintNode != NO_NODE )
			{
				TaskComplete();
			}
			else
			{
				TaskFail();
			}
			break;
		}
	case TASK_STORE_LASTPOSITION:
		{
			m_vecLastPosition = pev->origin;
			TaskComplete();
			break;
		}
	case TASK_CLEAR_LASTPOSITION:
		{
			m_vecLastPosition = g_vecZero;
			TaskComplete();
			break;
		}
	case TASK_CLEAR_HINTNODE:
		{
			m_iHintNode = NO_NODE;
			TaskComplete();
			break;
		}
	case TASK_STOP_MOVING:
		{

			if(FClassnameIs(pev, "monster_panthereye")){
				int x = 45;
			}


			//easyForcePrintLine("WEEEEEEEEEEEEEEEEEEELLLLLLLLLLLLLLLLLLLLLLLLA %d %d %d", m_IdealActivity, m_movementActivity, this->usingCustomSequence);





			/*
			//MODDD - but isn't it also possible for our m_movementActivity to be set to ACT_IDLE by basemonster.cpp's RouteClear method?
			//Extra check:
			//if ( m_IdealActivity == m_movementActivity )
			Activity myStoppedActivity = GetStoppedActivity();

			//Now, if the ideal activity isn't the stopped activity, AND it is the movement activity, we can do this change.
			//I suppose if Ideal was matching both the Stopped activity and movement ACT, it would've done nothing but good to be safe.
			//MODDD - also supporting a change if we're in LEFT or RIGHT turn activities.

			// old condition (not the as-is SDK):
			//if (m_IdealActivity != myStoppedActivity && m_IdealActivity == m_movementActivity
			//	|| m_Activity == ACT_TURN_LEFT || m_Activity == ACT_TURN_RIGHT)

			//MODDD - nevermind, this idea sucks.
			if( (m_IdealActivity != myStoppedActivity || m_Activity == ACT_TURN_LEFT || m_Activity == ACT_TURN_RIGHT)

				//MODDD - what happens if we remove this requirement?
				//&& m_IdealActivity == m_movementActivity
				)
			{

				m_IdealActivity = myStoppedActivity;
			}
			*/

			// well golly gee, wouldja look at that
			// Could also check for the usual move activities (ACT_RUN, ACT_WALK, ACT_FLY, ACT_HOVER, ACT_SWIM, etc.?).
			if(IsMoving()){
				Activity myStoppedActivity = GetStoppedActivity();
				m_IdealActivity = myStoppedActivity;
			}
			//easyForcePrintLine("WEEEEEEEEEEEEEEEEEEELLLLLLLLLLLLLLLLLLLLLLLLB %d %d %d", m_IdealActivity, m_movementActivity, this->usingCustomSequence);

			RouteClear();
			TaskComplete();
			break;
		}
	case TASK_PLAY_SEQUENCE_FACE_ENEMY:
	case TASK_PLAY_SEQUENCE_FACE_TARGET:
	case TASK_PLAY_SEQUENCE:
		{
			m_IdealActivity = ( Activity )( int )pTask->flData;
			break;
		}
	case TASK_PLAY_ACTIVE_IDLE:
		{
			// monsters verify that they have a sequence for the node's activity BEFORE
			// moving towards the node, so it's ok to just set the activity without checking here.
			m_IdealActivity = ( Activity )WorldGraph.m_pNodes[ m_iHintNode ].m_sHintActivity;
			break;
		}
	case TASK_SET_SCHEDULE:
		{
			Schedule_t *pNewSchedule;
			pNewSchedule = GetScheduleOfType( (int)pTask->flData );
			
			if ( pNewSchedule )
			{
				ChangeSchedule( pNewSchedule );
			}
			else
			{
				TaskFail();
			}

			break;
		}
	case TASK_FIND_NEAR_NODE_COVER_FROM_ENEMY:
		{
			if ( m_hEnemy == NULL )
			{
				TaskFail();
				return;
			}

			if ( FindCover( m_hEnemy->pev->origin, m_hEnemy->pev->view_ofs, 0, pTask->flData ) )
			{
				// try for cover farther than the FLData from the schedule.
				TaskComplete();
			}
			else
			{
				// no coverwhatsoever.
				TaskFail();
			}
			break;
		}
	case TASK_FIND_FAR_NODE_COVER_FROM_ENEMY:
		{
			if ( m_hEnemy == NULL )
			{
				TaskFail();
				return;
			}

			if ( FindCover( m_hEnemy->pev->origin, m_hEnemy->pev->view_ofs, pTask->flData, CoverRadius() ) )
			{
				// try for cover farther than the FLData from the schedule.
				TaskComplete();
			}
			else
			{
				// no coverwhatsoever.
				TaskFail();
			}
			break;
		}
	case TASK_FIND_NODE_COVER_FROM_ENEMY:
		{
			if ( m_hEnemy == NULL )
			{
				TaskFail();
				return;
			}

			if ( FindCover( m_hEnemy->pev->origin, m_hEnemy->pev->view_ofs, 0, CoverRadius() ) )
			{
				// try for cover farther than the FLData from the schedule.
				TaskComplete();
			}
			else
			{
				// no coverwhatsoever.
				TaskFail();
			}
			break;
		}
	case TASK_FIND_COVER_FROM_ENEMY:
		{
			BOOL coverAttempt = SCHEDULE_attemptFindCoverFromEnemy(pTask);

			if(coverAttempt){
				TaskComplete(); //assume the cover is setup and ready to use.
			}else{
				TaskFail();  //try something else?

				//MODDD - I am important to stop a fidget!
				m_movementGoal = MOVEGOAL_NONE;

			}
			break;
		}
	case TASK_FIND_COVER_FROM_ENEMY_OR_CHASE:
	{
		BOOL coverAttempt = SCHEDULE_attemptFindCoverFromEnemy(pTask);

		if(coverAttempt){
			TaskComplete(); //assume the cover is setup and ready to use.
		}else{
			ChangeSchedule( GetScheduleOfType(SCHED_CHASE_ENEMY) );
		}
		break;
	}
	case TASK_FIND_COVER_FROM_ENEMY_OR_FIGHT:
	{
		BOOL coverAttempt = SCHEDULE_attemptFindCoverFromEnemy(pTask);

		//if(coverAttempt){
		//	TaskComplete(); //assume the cover is setup and ready to use.
		//}else{

		//DEBUG - always fight for now.

		{
			
			//the "fight" part, since "flight" doesn't look so good.
			// just a repeat of the "SEE" condition script (attack-condition checks) in GetSchedule.
			if ( HasConditions(bits_COND_CAN_RANGE_ATTACK1) )
			{
				ChangeSchedule(GetScheduleOfType( SCHED_RANGE_ATTACK1 ));
				break;
			}
			if ( HasConditions(bits_COND_CAN_RANGE_ATTACK2) )
			{
				ChangeSchedule(GetScheduleOfType( SCHED_RANGE_ATTACK2 ));
				break;
			}
			if ( HasConditions(bits_COND_CAN_MELEE_ATTACK1) )
			{
				ChangeSchedule(GetScheduleOfType( SCHED_MELEE_ATTACK1 ));
				break;
			}
			if ( HasConditions(bits_COND_CAN_MELEE_ATTACK2) )
			{
				ChangeSchedule(GetScheduleOfType( SCHED_MELEE_ATTACK2 ));
				break;
			}

			//if ( !HasConditions(bits_COND_CAN_RANGE_ATTACK1 | bits_COND_CAN_MELEE_ATTACK1) )
			{
				// if we can see enemy but can't use either attack type, we must need to get closer to enemy
				//easyPrintLine("ducks2");
				ChangeSchedule(GetScheduleOfType( SCHED_CHASE_ENEMY ));
				break;
			}

			//made it here, how??
			TaskFail();

		}//END OF else OF coverAttempt check

		break;
	}
	case TASK_FIND_COVER_FROM_ORIGIN:
		{
			if ( FindCover( pev->origin, pev->view_ofs, 0, CoverRadius() ) )
			{
				//easyForcePrintLine("TASK_FIND_COVER_FROM_ORIGIN: I FOUND COVER OKAYYYYYYYYYY");
				// then try for plain ole cover
				m_flMoveWaitFinished = gpGlobals->time + pTask->flData;
				TaskComplete();
			}
			else
			{
				easyPrintLine("Pathfind: TASK_FIND_COVER_FROM_ORIGIN: I FAIL HARD.");
				// no cover!
				TaskFail();
			}
		}
		break;
	case TASK_MOVE_FROM_ORIGIN:{
		//MODDD - NEW.  Modified 'FindCover' method that doesn't care about, well, being in cover.  Any other criteria is fine?
		// For now only the hassault uses this, have a separate TASK_WALK_FROM_ORIGIN for other stuff, that might look better.
		if ( FindRandom( ACT_RUN, pev->origin, pev->view_ofs, 30, CoverRadius() ) )
		{
			//easyForcePrintLine("TASK_FIND_COVER_FROM_ORIGIN: I FOUND COVER OKAYYYYYYYYYY");
			// then try for plain ole cover
			m_flMoveWaitFinished = gpGlobals->time + pTask->flData;
			TaskComplete();
		}
		else
		{
			easyPrintLine("Pathfind: TASK_MOVE_FROM_ORIGIN: I FAIL HARD.");
			// no cover!
			TaskFail();
		}
	}break;
	case TASK_FIND_COVER_FROM_BEST_SOUND:
		{
			CSound *pBestSound;

			pBestSound = PBestSound();

			ASSERT( pBestSound != NULL );
			/*
			if ( pBestSound && FindLateralCover( pBestSound->m_vecOrigin, g_vecZero ) )
			{
				// try lateral first
				m_flMoveWaitFinished = gpGlobals->time + pTask->flData;
				TaskComplete();
			}
			*/

			//MODDD - assuming this is something like a grenade, we can up the maximum distance. Going to go somewhere regardless.
			//if ( pBestSound && FindCover( pBestSound->m_vecOrigin, g_vecZero, pBestSound->m_iVolume, CoverRadius() ) )
			if ( pBestSound && FindCover( pBestSound->m_vecOrigin, g_vecZero, pBestSound->m_iVolume*0.5, CoverRadius()*3 ) )
			{
				// then try for plain ole cover
				m_flMoveWaitFinished = gpGlobals->time + pTask->flData;
				TaskComplete();
			}
			else
			{
				// no coverwhatsoever. or no sound in list
				TaskFail();
			}
			break;
		}
	//MODDD - NEW
	case TASK_FACE_POINT:
		{
		MakeIdealYaw ( this->m_vecMoveGoal );
		SetTurnActivity(); 
		break;
		}
	case TASK_FACE_HINTNODE:
		{
			pev->ideal_yaw = WorldGraph.m_pNodes[ m_iHintNode ].m_flHintYaw;
			SetTurnActivity();
			break;
		}
	
	case TASK_FACE_LASTPOSITION:
		MakeIdealYaw ( m_vecLastPosition );
		SetTurnActivity(); 
		break;
	case TASK_FACE_GOAL:
		{
		WayPoint_t* myGoalNode = GetGoalNode();
		if(myGoalNode != NULL){
			MakeIdealYaw(myGoalNode->vecLocation);
			if (FacingIdeal()){
				TaskComplete();
				return;
			}
			SetTurnActivity(); 
		}else{
			// ??????    what.  no goal node.
			TaskComplete();
		}
		break;
		}
	break;
	case TASK_FACE_TARGET:
		if ( m_hTargetEnt != NULL )
		{
			MakeIdealYaw ( m_hTargetEnt->pev->origin );

			//MODDD - added, if we can complete early we can move on with thinking in the same frame
			if (FacingIdeal())
			{
				TaskComplete();
				return;
			}

			SetTurnActivity(); 
		}
		else
			TaskFail();
		break;
	case TASK_FACE_ENEMY:
		{
			// ??? Why did I do this?  It's still fine to face the LKP regardless, often the intent
			/*
			if (m_hEnemy == NULL) {
				//oops.
				TaskComplete();
				return;
			}
			*/

			MakeIdealYaw ( m_vecEnemyLKP );

			//MODDD - added, if we can complete early we can move on with thinking in the same frame
			if (FacingIdeal())
			{
				TaskComplete();
				return;
			}

			SetTurnActivity(); 

			break;
		}
	case TASK_FACE_PREV_LKP:
		{
			//HACK - since we know this is only called by the stumped method, do a check to see
			//  if we're going to just route to the enemy's modern position anyways and skip the rest of this schedule.

			//...we should probably still do this pathFindStumpedMode == 3 check in case something that doesn't use FACE_PREV_LKP checks for being stumped.
			if(EASY_CVAR_GET_DEBUGONLY(pathfindStumpedMode) == 3){
				// Forcing the LKP to be up-to-date here.
				//MODDD - involve the ent
				setEnemyLKP(m_hEnemy);

				//TaskFail();  //is this even necessary?
				ChangeSchedule(GetSchedule());
				return;
			}

			if(m_fEnemyLKP_prev_EverSet){
				MakeIdealYaw(m_vecEnemyLKP_prev);
			}else{
				// never set.  ??? what
				TaskComplete();
				return;
			}

			//MODDD - added, if we can complete early we can move on with thinking in the same frame
			if (FacingIdeal())
			{
				TaskComplete();
				return;
			}

			SetTurnActivity(); 
			break;
		}
	case TASK_FACE_IDEAL:
		{

			//MODDD - added, if we can complete early we can move on with thinking in the same frame
			if (FacingIdeal())
			{
				TaskComplete();
				return;
			}

			SetTurnActivity();
			break;
		}

	
	case TASK_FACE_IDEAL_IF_VISIBLE:
	{
		if(!this->FVisible(m_vecMoveGoal)){
			// no line to the given point?  Don't fail, but don't change where you're facing.
			// Guess that means TaskComplete.
			TaskComplete();
		}else{
			//MODDD - added, if we can complete early we can move on with thinking in the same frame
			if (FacingIdeal())
			{
				TaskComplete();
				return;
			}

			SetTurnActivity();
		}
		break;
	}

	case TASK_FACE_ROUTE:
		{
			if (FRouteClear())
			{
				ALERT(at_aiconsole, "No route to face!\n");
				TaskFail();
			}
			else
			{
				MakeIdealYaw(m_Route[m_iRouteIndex].vecLocation);

				//MODDD - added, if we can complete early we can move on with thinking in the same frame
				if (FacingIdeal())
				{
					TaskComplete();
					return;
				}

				SetTurnActivity();
			}
			break;
		}
	case TASK_FACE_BEST_SOUND:
		{
			CSound *pSound;
			pSound = PBestSound();

			//if ( pSound && MoveToLocation( m_movementActivity, 2, pSound->m_vecOrigin ) )
			if(pSound)
			{
				//DebugLine_SetupPoint(pSound->m_vecOrigin, 0, 255, 0);

				MakeIdealYaw(pSound->m_vecOrigin);

				//MODDD - added, if we can complete early we can move on with thinking in the same frame
				if (FacingIdeal())
				{
					TaskComplete();
					return;
				}

				SetTurnActivity();
				//TaskComplete();
			}else{
				//is that fine?
				TaskFail();
			}
			
			break;
		}
	case TASK_CHECK_STUMPED:
		{
			// Completeing this task will let the monster pick a different schedule as usual.
			// Set the fail schedule to something desired and call "TaskFail" to do that instead, like staring for 15 seconds when stumped before re-getting the enemy's location from the engine.
			// You cheater!

			//easyForcePrintLine("I MUST SAY, I AM STUMPED.");

			//TaskComplete();
			//return;
			// no dont do it!


			// It is possible we're facing our last known position, but just repeatedly getting satisifed with that.
			// Do a check. Are we looking at the enemy right now? If not, we need to force the LKP to the player to seek them.

			if(!HasConditions(bits_COND_SEE_ENEMY)){
				// Not looking at the enemy, but looking at LKP (presumably)?

				// If I was checking out a place I took damage at instead of chasing the enemy directly and I fail to see the enem
				if(investigatingAltLKP){
					// no longer!  Restore the LKP to the "real" one where the enemy was last sighted. If it hasn't been updated since that is.
					// Any changes to LKP since will change "investigatingAltLKP" to false that put it at the enemy's real location again of course.
					
					// already done by setEnemyLKP now
					//investigatingAltLKP = FALSE;
					//m_vecEnemyLKP = m_vecEnemyLKP_Real;

					// Only if this was 'something' at the time we invesgiated, it might still be the default (0,0,0).
					// Also, set the EverSet to Real_EverSet.
					// This way, if there isn't an LKP to backup to after this invesgiated one has been reach,
					// the normal EverSet knows that (gets Real_EverSet being FALSE).
					m_fEnemyLKP_EverSet = m_fEnemyLKP_Real_EverSet;
					if(m_fEnemyLKP_Real_EverSet == TRUE){
						setEnemyLKP(m_vecEnemyLKP_Real, m_flEnemyLKP_Real_zOffset);

						// Is including 'ending the task early' in this Real_EverSet requirement ok?
						//TaskFail();
						TaskComplete();
						return;
					}else{
						// Uhhh... what?  LKP_Real was never set??
						easyPrintLine("AI: %s:%d WARNING!  TASK_CHECK_STUMPED: EnemyLKP_Real had never been set!  Method likely called when enemy had no prior LKP.  This may be fine.");
						// wait...  shouldn't have been an enemy, but just to be safe.
						// Nevermind, just let the rest of the method do its work, think that's the same effect of being stumped
						// without investigating.
						//m_hEnemy = NULL;
						//TaskComplete();
						//return;

					}
				}


				if(EASY_CVAR_GET_DEBUGONLY(pathfindStumpedMode) == 0){
					// Forget the enemy, we lost sight. Will pick up on an old remembered enemy pushed into memory in a stack
					// if there is one there.
					m_hEnemy = NULL;
					//GetEnemy();
					TaskComplete();
					return;
				}

				// Skip to re-routing towards the enemy, most likely.
				if(EASY_CVAR_GET_DEBUGONLY(pathfindStumpedMode) == 3){
					// make the LKP up to date
					//MODDD - Invole the ent
					setEnemyLKP(m_hEnemy);

					//TaskFail();  //is this even necessary?
					ChangeSchedule(GetSchedule());
					return;
				}

				if(m_fEnemyLKP_EverSet){
					m_vecEnemyLKP_prev = m_vecEnemyLKP; //the old to look at for a little.
					m_flEnemyLKP_prev_zOffset = m_flEnemyLKP_zOffset;
					m_fEnemyLKP_prev_EverSet = TRUE;
				}

				//easyForcePrintLine("AHHH. I don\'t see the enemy. Has enemy to seek? %d", (m_hEnemy != NULL));
				if(m_hEnemy != NULL){
					// Go look at them next time to break this cycle.

					// safety feature, do this all the time.
					//MODDD - involve the ent
					setEnemyLKP(m_hEnemy);
					
					//Before we resume, pause for a little.
					//... this will suffice, just fail regardless.
					m_failSchedule = SCHED_PATHFIND_STUMPED;  //A variant of FAIL that will be interrupted like idle, by sounds, seeing the enemy, etc.
					TaskFail();
				}else{
					m_failSchedule = SCHED_PATHFIND_STUMPED;
					TaskFail(); //no enemy... what?
				}

			}else{
				//easyForcePrintLine("But it is ok, I see the enemy at least.");
				TaskComplete();  //If we can see the enemy now, no need for corrective action.
			}

		}
	break;
	case TASK_UPDATE_LKP:
		{
		//Force me to know the enemy's location.
		if(m_hEnemy != NULL){
			//MODDD - involve the ent
			setEnemyLKP( m_hEnemy);
		}
		TaskComplete();
		break;
		}
	case TASK_WAIT_PVS:
	case TASK_WAIT_INDEFINITE:
		{
			// don't do anything.
			break;
		}
	case TASK_WAIT:
	case TASK_WAIT_FACE_ENEMY:
		{// set a future time that tells us when the wait is over.
			m_flWaitFinished = gpGlobals->time + pTask->flData;	
			break;
		}
	case TASK_WAIT_RANDOM:
		{// set a future time that tells us when the wait is over.
			m_flWaitFinished = gpGlobals->time + RANDOM_FLOAT( 0.1, pTask->flData );
			break;
		}
	case TASK_WAIT_STUMPED:
		{
			// from TASK_FACE_ENEMY
			///////////////////////////////////////
			if(m_fEnemyLKP_EverSet){
				MakeIdealYaw ( m_vecEnemyLKP );
			}

			//MODDD - added, if we can complete early we can move on with thinking in the same frame
			if (FacingIdeal())
			{
				TaskComplete();
				return;
			}

			SetTurnActivity(); 
			//////////////////////////////////////////////


			m_flWaitFinished = gpGlobals->time + EASY_CVAR_GET_DEBUGONLY(pathfindStumpedWaitTime);
			break;
		}
	case TASK_WAIT_FACE_IDEAL:{
		m_flWaitFinished = gpGlobals->time + pTask->flData;	

		//if (FacingIdeal())
		//{
		//	TaskComplete();
		//	return;
		//}

		SetTurnActivity();

	}break;
	case TASK_MOVE_TO_TARGET_RANGE:
		{
			if(m_hTargetEnt == NULL){
				//HOW DARE YOU.  HOw. DARE. YOU.
				TaskFail();
				return;
			}

			//MODDD - why not end early if we're close enough??
			//if ( (m_hTargetEnt->pev->origin - pev->origin).Length() < 1 )
			if ((m_hTargetEnt->pev->origin - pev->origin).Length() < pTask->flData) {
				TaskComplete();
			}else{
				m_vecMoveGoal = m_hTargetEnt->pev->origin;
				if (!MoveToTarget(ACT_WALK, 2)) {
					TaskFail();
				}
				else {

					//////////////////////////////////////////////////////////////////////////////////////////////////
					//////////////////////////////////////////////////////////////////////////////////////////////////
					//////////////////////////////////////////////////////////////////////////////////////////////////
					// prepare to take the route soon.
					/*
					if (isMovetypeFlying())
						m_movementActivity = ACT_FLY;
					else
						m_movementActivity = ACT_RUN;
					*/

					// or, assume m_movementActivity was set by MoveToTarget?
					// Although it forces to ACT_WALK.   ehhh why not.
					m_IdealActivity = m_movementActivity;
					//////////////////////////////////////////////////////////////////////////////////////////////////
					//////////////////////////////////////////////////////////////////////////////////////////////////
					//////////////////////////////////////////////////////////////////////////////////////////////////
				}
			}
			break;
		}

	//MODDD - this is forced to involve the enemy LKP now instead, even at the start.
	case TASK_MOVE_TO_ENEMY_RANGE:
	{
		if(m_hEnemy == NULL){
			//what??
			TaskFail();
			return;
		}

		// record this!  Pathfinding will allow a route that ends up to this
		// distance from the goal floor(2D)-wise.
		goalDistTolerance = pTask->flData;


		float distToGoal = (m_vecEnemyLKP - pev->origin).Length();

		// WAIT.  Why not allow being close enough to the enemy ahead of time to work??
		//if ( (m_hEnemy->pev->origin - pev->origin).Length() < 1 )
		//if ( (m_vecEnemyLKP - pev->origin).Length() < 1 ){
		if(distToGoal < pTask->flData){
			TaskComplete();
		}else{
			//if ( !MoveToEnemy( ACT_WALK, 2 ) )
			//	TaskFail();

			// This gets the real enemy location, which may or may not be a good idea. It can seem  like cheating if done way too often to constantly just know where you are.
			// But that's todo.
			BOOL test = FRefreshRouteChaseEnemySmart();
			// This will be used if a 'NearestRoute' was made.  A direct route already has checks
			// (if the enemy is too far from where I'm going on a direct route, that's a call for a re-route)
			nextDirectRouteAttemptTime = gpGlobals->time + 1;

			if(!test){
				//easyForcePrintLine("!!! %s:%d YOU HAVE ALREADY FAILED.", this->getClassname(), this->monsterID);
				TaskFail();
			}
			else {
				// prepare to take the route soon.

				//////////////////////////////////////////////////////////////////////////////////////////////////
				//////////////////////////////////////////////////////////////////////////////////////////////////
				//////////////////////////////////////////////////////////////////////////////////////////////////
				/*
				if (isMovetypeFlying())
					m_movementActivity = ACT_FLY;
				else
					m_movementActivity = ACT_RUN;
				*/

				// or, assume m_movementActivity was set by FRefreshRouteChaseEnemySmart?
				// Although it forces to ACT_RUN.   ehhh why not.
				
				// Wait!  Careful about setting this so early, some things depend on
				// seeing the difference between IdealActivity and movementActivity to set
				// something speed-related.
				/*
				if(!MovementIsComplete()){
					// It is possible that movement completed in the same frame in a very short route?  what?
					m_IdealActivity = m_movementActivity;
				}
				*/
				//////////////////////////////////////////////////////////////////////////////////////////////////
				//////////////////////////////////////////////////////////////////////////////////////////////////
				//////////////////////////////////////////////////////////////////////////////////////////////////
				//////////////////////////////////////////////////////////////////////////////////////////////////
			}
		}
		break;
	}
	//MODDD - new, clone of TASK_MOVE_TO_TARGET_RANGE but for a point (m_vecMoveGoal) instead.
	case TASK_MOVE_TO_POINT_RANGE:
	{
		
		//NOTICE: task assumes "m_vecMoveGoal" has been set!
		//MODDD - ahh you know the drill.
		//if ( (m_vecMoveGoal - pev->origin).Length() < 1 ){
		if ( (m_vecMoveGoal - pev->origin).Length() < pTask->flData){
			//easyForcePrintLine("HORRIBLE OKAY");
			TaskComplete();
		}else
		{
			//m_vecMoveGoal = m_vecMoveGoal;
			//if ( !MoveToTarget( ACT_WALK, 2 ) ){
			if(!MoveToLocation(m_movementActivity, 2, m_vecMoveGoal)){
				//easyForcePrintLine("HORRIBLE FFFF");
				TaskFail();
			}
			else {
				// prepare to take the route soon.

				//////////////////////////////////////////////////////////////////////////////////////////////////
				//////////////////////////////////////////////////////////////////////////////////////////////////
				//////////////////////////////////////////////////////////////////////////////////////////////////
				/*
				if (isMovetypeFlying())
					m_movementActivity = ACT_FLY;
				else
					m_movementActivity = ACT_RUN;
				*/

				// or, assume m_movementActivity was set by FRefreshRouteChaseEnemySmart?
				// Although it forces to ACT_RUN.   ehhh why not.
				m_IdealActivity = m_movementActivity;
				//////////////////////////////////////////////////////////////////////////////////////////////////
				//////////////////////////////////////////////////////////////////////////////////////////////////
				//////////////////////////////////////////////////////////////////////////////////////////////////
			}
		}
		break;
	}

	case TASK_RUN_TO_TARGET:
	case TASK_WALK_TO_TARGET:
		{
			// Expcected to be used by scripted's only, I think, to route to a pre-determined point.  Or entity with that pre-determined point.

			Activity newActivity;

			if(m_hTargetEnt == NULL){
				easyForcePrintLine("I\'m not feeling so fantastic. (null target)  %s:%d", getClassname(), monsterID);
				TaskFail();
				break;
			}

			if ((m_hTargetEnt->pev->origin - pev->origin).Length() < 1) {
				TaskComplete();
			}
			else
			{

				//MODDD - if this monster lacks a run act, this won't work out too well.

				if (pTask->iTask == TASK_WALK_TO_TARGET) {
					newActivity = ACT_WALK;
				}
				else {
					newActivity = ACT_RUN;
				}


				// This monster can't do this!
				if (LookupActivity(newActivity) == ACTIVITY_NOT_AVAILABLE) {
					// Let's have a descriptive error message if you please!
					easyForcePrintStarter();
					easyForcePrint("WARNING!  Monster in TASK_WALK_TO_TARGET could not find requested activity #%d (WALK or RUN).\n", newActivity);
					easyForcePrintStarter();
					easyForcePrint("Entity info: ");
					printBasicEntityInfo(this);
					easyForcePrintLine();

					TaskComplete();
				}
				else
				{
					if ( m_hTargetEnt == NULL )
					{
						TaskFail();
						ALERT( at_aiconsole, "%s Null target?!!!\n", STRING(pev->classname) );
						RouteClear();
					}else{
						BOOL daTest = MoveToTargetStrict(newActivity, 2);
						if(!daTest){

							TaskFail();
							ALERT( at_aiconsole, "%s Failed to reach target!!!\n", STRING(pev->classname) );
							RouteClear();
						}
					}
				}
			}
			TaskComplete();
			break;
		}
	case TASK_CLEAR_MOVE_WAIT:
		{
			m_flMoveWaitFinished = gpGlobals->time;
			TaskComplete();
			break;
		}
	case TASK_MELEE_ATTACK1_NOTURN:
	case TASK_MELEE_ATTACK1:
		{
			//MODDD - added.
			//m_Activity = ACT_RESET;  //force me to re-get this!
			//m_fSequenceFinished = FALSE;
			//pev->frame = 0;



			m_IdealActivity = ACT_MELEE_ATTACK1;
			//this->signalActivityUpdate = TRUE;
			break;
		}
	case TASK_MELEE_ATTACK2_NOTURN:
	case TASK_MELEE_ATTACK2:
		{
			//MODDD - added.
			//m_Activity = ACT_RESET;  //force me to re-get this!
			//m_fSequenceFinished = FALSE;
			//pev->frame = 0;

			//if (this->m_fSequenceLoops == FALSE) {
			//???!!!???
			//}


			m_IdealActivity = ACT_MELEE_ATTACK2;
			//this->signalActivityUpdate = TRUE;
			break;
		}
	case TASK_RANGE_ATTACK1_NOTURN:
	case TASK_RANGE_ATTACK1:
		{
			//MODDD - safety check.  If we already HAVE this ideal activity set, send a signal to re-get it.
			// COULD also do "SetActivity" to guarantee doing this but... side effects?  this has stung me one-too-many times.
			//if(this->m_fSequenceFinished && m_IdealActivity == ACT_RANGE_ATTACK1){
			//...
			//}


			/*
			//wait how about this.
			m_Activity = ACT_RESET;  //force me to re-get this!
			//m_fSequenceFinished = FALSE;
			//pev->frame = 0;
			*/
			m_IdealActivity = ACT_RANGE_ATTACK1;
			//m_Activity = ACT_RESET;
			//SetActivity(ACT_RANGE_ATTACK1);


			//int derp = this->m_fSequenceLoops;
			//int x;

			//TEST...
			//SetActivity(ACT_RANGE_ATTACK1);



			//MODDD - CRITICAL NEW.
			//this->signalActivityUpdate = TRUE;
			//Force the activity to pick a new anim even if already on that activity.
			//This stops the monster from freezing on the last set animation if already on the activity for some reason.
			//... don't do this. Look at the end of TASK_RANGE_ATTACK1 and others for setting the current activity to ACT_RESET.
			//This effectively forces the sequence to be regathered too.  Doing it here too is redundant.
			//See if a monster isn't doing the same at the end of their own TASK_RANGE_ATTACK1 or similar.
			break;
		}
	case TASK_RANGE_ATTACK2_NOTURN:
	case TASK_RANGE_ATTACK2:
		{
			//MODDD - added.
			//m_Activity = ACT_RESET;  //force me to re-get this!
			//m_fSequenceFinished = FALSE;
			//pev->frame = 0;

			m_IdealActivity = ACT_RANGE_ATTACK2;
			//m_Activity = ACT_RESET;
			//SetActivity(ACT_RANGE_ATTACK2);


			//this->signalActivityUpdate = TRUE;
			break;
		}

	//MODDD - new. This task acts as a gate: only pass if we are able to make an attack. Otherwise fail this schedule.
	case TASK_CHECK_RANGED_ATTACK_1:
		{
			if(HasConditions(bits_COND_CAN_RANGE_ATTACK1)){
				TaskComplete();
			}else{
				TaskFail();
			}

			break;
		}
	case TASK_RELOAD_NOTURN:
	case TASK_RELOAD:
		{
			m_IdealActivity = ACT_RELOAD;
			break;
		}
	case TASK_SPECIAL_ATTACK1:
		{
			m_IdealActivity = ACT_SPECIAL_ATTACK1;
			break;
		}
	case TASK_SPECIAL_ATTACK2:
		{
			m_IdealActivity = ACT_SPECIAL_ATTACK2;
			break;
		}
	case TASK_SET_ACTIVITY:
		{
			m_IdealActivity = (Activity)(int)pTask->flData;

			if(m_pSchedule == slHAssault_residualFire){
				int x = 45;
			}

			//HACK HACK - what if you do this
			//SetActivity(m_IdealActivity);

			TaskComplete();
			break;
		}
		
	case TASK_SET_ACTIVITY_FORCE:{
		//MODDD - new. Similar to TASK_SET_ACTIVITY above, but enforces picking a new sequence right now.
		SetActivity( (Activity)(int)pTask->flData );
		TaskComplete();
	break;}

		//MODDD - We're forcing even GET_PATH_TO_ENEMY to use the LastKnownPath instead.
		// OBSOLETE TASK - the smart version of chase methods (new standard) uses TASK_MOVE_TO_ENEMY_RANGE instead.
		// The same FRefreshRoute call works fine there.
	case TASK_GET_PATH_TO_ENEMY:
	case TASK_GET_PATH_TO_ENEMY_LKP:
		{
			CBaseEntity* enemyTest;

			if(m_hEnemy != NULL){
				//send along a reference to the enemy itself.  Can't hurt I imagine.
				enemyTest = m_hEnemy.GetEntity();
			}else{
				enemyTest = NULL;
			}

			//const char* schedName = m_pSchedule->pName;

			
			//if ( BuildRoute ( m_vecEnemyLKP, bits_MF_TO_LOCATION, NULL ) )

			//is it safe to use bits_MF_TO_ENEMY to get "MOVEGOAL_ENEMY" anyways?
			//Looks like it. This just says to send the current "m_hEnemy" to path methods like CheckLocalMove to mark them as exceptions for
			//colliding with. After all it would be silly to say "Path to player failed", because the "player" was in the way of the destination point!
			if ( BuildRoute ( m_vecEnemyLKP, bits_MF_TO_ENEMY, enemyTest ) )
			{
				TaskComplete();
			}
			else if (BuildNearestRoute( m_vecEnemyLKP, pev->view_ofs, 0, (m_vecEnemyLKP - pev->origin).Length(), DEFAULT_randomNodeSearchStart, bits_MF_TO_ENEMY, enemyTest )  )
			{
				TaskComplete();
			}
			else
			{
				// no way to get there =(
				ALERT ( at_aiconsole, "GetPathToEnemyLKP failed!!\n" );
				TaskFail();

				//MODDD - be sure to set the movegoal to NONE when a path-building task fails.
				//        This stops the monster from fidgeting for a frame futily moving.
				m_movementGoal = MOVEGOAL_NONE;
				//just in case...?
			}
			break;
		}

		/*
	case TASK_GET_PATH_TO_ENEMY:
		{
			CBaseEntity *pEnemy = m_hEnemy;

			if ( pEnemy == NULL )
			{
				TaskFail();
				return;
			}

			//Clearly we want to just route to the enemy. Update the LKP to not confuse the pathfinding.
			setEnemyLKP(m_hEnemy);
			

			if ( BuildRoute ( pEnemy->pev->origin, bits_MF_TO_ENEMY, pEnemy ) )
			{
				TaskComplete();
			}
			else if (BuildNearestRoute( pEnemy->pev->origin, pEnemy->pev->view_ofs, 0, (pEnemy->pev->origin - pev->origin).Length(), DEFAULT_randomNodeSearchStart ))
			{
				TaskComplete();
			}
			else
			{
				// no way to get there =(
				ALERT ( at_aiconsole, "GetPathToEnemy failed!!\n" );
				TaskFail();
			}
			break;
		}
		*/
	case TASK_GET_PATH_TO_ENEMY_CORPSE:
		{

			//MODDD NOTE - is trusting that the "m_vecEnemyLKP" is the same as the enemy corpse position okay?
			UTIL_MakeVectors( pev->angles );
			if ( BuildRoute ( m_vecEnemyLKP - gpGlobals->v_forward * 64, bits_MF_TO_LOCATION, NULL ) )
			{
				TaskComplete();
			}
			else
			{
				ALERT ( at_aiconsole, "GetPathToEnemyCorpse failed!!\n" );
				TaskFail();
				m_movementGoal = MOVEGOAL_NONE;
			}
		}
		break;
	
	//MODDD - NOTE.  TASK_GET_PATH_TO_SPOT removed.
	// It build a route to wherever the player is?  Why?  May or may not even be the enemy.
	// No wonder this went unused.

	case TASK_GET_PATH_TO_TARGET:
		{
			RouteClear();
			if ( m_hTargetEnt != NULL && MoveToTarget( m_movementActivity, 1 ) )
			{
				TaskComplete();
			}
			else
			{
				// no way to get there =(
				ALERT ( at_aiconsole, "GetPathToSpot failed!!\n" );
				TaskFail();
				m_movementGoal = MOVEGOAL_NONE;
			}
			break;
		}
	case TASK_GET_PATH_TO_HINTNODE:// for active idles!
		{
			if ( MoveToLocation( m_movementActivity, 2, WorldGraph.m_pNodes[ m_iHintNode ].m_vecOrigin ) )
			{
				TaskComplete();
			}
			else
			{
				// no way to get there =(
				ALERT ( at_aiconsole, "GetPathToHintNode failed!!\n" );
				TaskFail();
				m_movementGoal = MOVEGOAL_NONE;
			}
			break;
		}
	case TASK_GET_PATH_TO_LASTPOSITION:
		{
			m_vecMoveGoal = m_vecLastPosition;

			if ( MoveToLocation( m_movementActivity, 2, m_vecMoveGoal ) )
			{
				TaskComplete();
			}
			else
			{
				// no way to get there =(
				ALERT ( at_aiconsole, "GetPathToLastPosition failed!!\n" );
				TaskFail();
				m_movementGoal = MOVEGOAL_NONE;
			}
			break;
		}
	case TASK_GET_PATH_TO_BESTSOUND:
		{
			CSound *pSound;

			pSound = PBestSound();

			if ( pSound && MoveToLocation( m_movementActivity, 2, pSound->m_vecOrigin ) )
			{
				TaskComplete();
			}
			else
			{
				// no way to get there =(
				ALERT ( at_aiconsole, "GetPathToBestSound failed!!\n" );
				TaskFail();
				m_movementGoal = MOVEGOAL_NONE;
			}
			break;
		}
	case TASK_GET_PATH_TO_BESTSCENT:
		{
			CSound *pScent;

			pScent = PBestScent();

			if ( pScent && MoveToLocation( m_movementActivity, 2, pScent->m_vecOrigin ) )
			{
				TaskComplete();
			}
			else
			{
				// no way to get there =(
				ALERT ( at_aiconsole, "GetPathToBestScent failed!!\n" );
				
				TaskFail();
				m_movementGoal = MOVEGOAL_NONE;
			}
			break;
		}
		//MODDD - NEW.
	case TASK_GET_PATH_TO_GOALVEC:
		{
			goalDistTolerance = pTask->flData;

			if (MoveToLocationStrict( m_movementActivity, 2, this->m_vecMoveGoal ) )
			{
				TaskComplete();
			}
			else
			{
				// no way to get there =(
				ALERT ( at_aiconsole, "GetPathToGoalVec failed!!\n" );
				TaskFail();
				m_movementGoal = MOVEGOAL_NONE;
			}
			break;
		}
	case TASK_RUN_PATH:
		{
			//MODDD SUGGESTION - would it be a good idea to clear "m_flMoveWaitFinished" here too?
			//                   Or in any path generation or RouteClear?
			m_flMoveWaitFinished = gpGlobals->time;

			// UNDONE: This is in some default AI and some monsters can't run? -- walk instead?
			if ( LookupActivity( ACT_RUN ) != ACTIVITY_NOT_AVAILABLE )
			{
				m_movementActivity = ACT_RUN;
			}
			else
			{
				m_movementActivity = ACT_WALK;
			}
			TaskComplete();
			break;
		}
	case TASK_WALK_PATH:
		{
			if ( isMovetypeFlying() )
			{
				m_movementActivity = ACT_FLY;
			}

			//MODDD NOTE - was the lack of "else" here intentional? That means a check for ACT_WALK will be done regardless and used instead. Most likely so.
			if ( LookupActivity( ACT_WALK ) != ACTIVITY_NOT_AVAILABLE )
			{
				m_movementActivity = ACT_WALK;
			}
			else
			{
				m_movementActivity = ACT_RUN;
			}
			TaskComplete();
			break;
		}
	case TASK_STRAFE_PATH:
		{
			Vector2D	vec2DirToPoint; 
			Vector2D	vec2RightSide;

			// to start strafing, we have to first figure out if the target is on the left side or right side
			UTIL_MakeVectors ( pev->angles );

			vec2DirToPoint = ( m_Route[ 0 ].vecLocation - pev->origin ).Make2D().Normalize();
			vec2RightSide = gpGlobals->v_right.Make2D().Normalize();

			if ( DotProduct ( vec2DirToPoint, vec2RightSide ) > 0 )
			{
				// strafe right
				m_movementActivity = ACT_STRAFE_RIGHT;
			}
			else
			{
				// strafe left
				m_movementActivity = ACT_STRAFE_LEFT;
			}
			TaskComplete();
			break;
		}

	case TASK_WAIT_FOR_MOVEMENT:
		{
			if (FRouteClear())
			{
				TaskComplete();
			}
			break;
		}
	case TASK_WAIT_FOR_MOVEMENT_TIMED:{
		// uses the data for how long the monster may spend on this task.
		waitForMovementTimed_Start = gpGlobals->time + pTask->flData;
		if (FRouteClear())
		{
			TaskComplete();
		}
	break;}

	//MODDD - going to let the "runTask" method handle this one better
	// No, do it the right way here too so that skipping to a different schedule in the same-frame is do-able
	// if it would've finished the instant 'runTask' would
	// have looked at it.
	case TASK_WAIT_FOR_MOVEMENT_RANGE:{
		//FRouteClear() ?

		//EXPERIMENTAL - any CheckLocalMove checks can allow being this clsoe to the goal to still pass.
		goalDistTolerance = pTask->flData;

		if (MovementIsComplete()){
			TaskComplete();
			RouteClear();		// Stop moving
		}else{
			if(m_Route[m_iRouteIndex].iType & bits_MF_IS_GOAL){
				float distToGoal = ( m_Route[ m_iRouteIndex ].vecLocation - pev->origin ).Length();
				//easyForcePrintLine("TASK_WAIT_FOR_MOVEMENT_RANGE: distToGoal:%.2f req:%.2f", distToGoal, pTask->flData);
				if(distToGoal <= pTask->flData){
					//done!
					TaskComplete();
					RouteClear();		// Stop moving
				}
			}
		}
	break;}
	// NEW.  Stop when the end of my route is in the viewcone.
	case TASK_WAIT_FOR_MOVEMENT_GOAL_IN_SIGHT:{
		if (MovementIsComplete())
		{
			TaskComplete();
			RouteClear();		// Stop moving
		}else{
			WayPoint_t* theGoalNode = GetGoalNode();
			if(theGoalNode == NULL){
				// oops?
				TaskFail();
			}else{
				// Can I see the goal node?
				BOOL inDaViewcone = FInViewCone(&theGoalNode->vecLocation);
				if(inDaViewcone){
					float distToGoal = Distance(pev->origin, theGoalNode->vecLocation);
					if(pTask->flData==0 || distToGoal <= pTask->flData){
						TaskComplete();
						RouteClear();		// Stop moving
					}
				}
			}
		}
	break;}
	case TASK_EAT:
		{
			Eat( pTask->flData );
			TaskComplete();
			break;
		}
	case TASK_SMALL_FLINCH:
		{
			m_IdealActivity = GetSmallFlinchActivity();
			break;
		}
	case TASK_BIG_FLINCH:{
		m_IdealActivity = GetBigFlinchActivity();
		break;
	break;}
	case TASK_DIE:
		{
			DeathAnimationStart();
			
			break;
		}
	case TASK_DIE_SIMPLE:{
		DeathAnimationStart();
	break;}
	
	case TASK_DIE_LOOP:{
		// Starter for the task.
		// In a monster that calls for SCHED_DIE_LOOP instead of SCHED_DIE, ensure "getLoopingDeathSequence" is overridden to refer
		// to a fitting (falling?) sequence to loop until it has a reason to be interrupted (hit the ground)
		// It is still up to the monster itself to tell how TASK_DIE_LOOP calls TaskComplete (on hitting the ground).
		this->SetSequenceByIndex(getLoopingDeathSequence(), 1.0f);

		//These calls / settings are based off of "DeathAnimationStart" from basemonster.cpp.
		//It is implied this sort of thing happens at the start of death.
		RouteClear();

		// don't force re-getting an animation just yet.
		// A new animation comes from a discrepency between m_Activity and m_IdealActivity, so forcing both stops regetting an animation.
		// !!! also GetDeathActivity doesn't work if the pev->deadflag isn't DEAD_NO.
		m_IdealActivity = GetDeathActivity();
		m_Activity = m_IdealActivity;
		
		pev->deadflag = DEAD_DYING;
		
	break;}
	case TASK_SET_FALL_DEAD_TOUCH:{
		// Just set my touch to anticipate hitting the ground and moving on to the next task (TASK_DIE) for the anim for hitting the ground to finish.
		
		SetTouch(&CBaseMonster::KilledFinishTouch);
		TaskComplete();

	break;}
	//MODDD - also new
	case TASK_WATER_DEAD_FLOAT:
		// between 30 and 50 seconds, sink anyway
		floatEndTimer = gpGlobals->time + RANDOM_FLOAT(30, 50);
		floatSinkSpeed = WATER_DEAD_SINKSPEED_INITIAL;
	break;

	case TASK_SOUND_WAKE:
		{
			AlertSound();
			TaskComplete();
			break;
		}
	case TASK_SOUND_DIE:
		{
			DeathSound();
			TaskComplete();
			break;
		}
	case TASK_SOUND_IDLE:
		{
			IdleSound();
			TaskComplete();
			break;
		}
	case TASK_SOUND_PAIN:
		{
			PainSound();
			TaskComplete();
			break;
		}
	case TASK_SOUND_DEATH:
		{
			DeathSound();
			TaskComplete();
			break;
		}
	case TASK_SOUND_ANGRY:
		{
			// sounds are complete as soon as we get here, cause we've already played them.
			ALERT ( at_aiconsole, "SOUND\n" );			
			TaskComplete();
			break;
		}
	case TASK_WAIT_FOR_SCRIPT:
		{
			if (m_pCine->m_iszIdle)
			{
				m_pCine->StartSequence( (CBaseMonster *)this, m_pCine->m_iszIdle, FALSE );
				if (FStrEq( STRING(m_pCine->m_iszIdle), STRING(m_pCine->m_iszPlay)))
				{
					pev->framerate = 0;
				}
			}
			else
				m_IdealActivity = ACT_IDLE;

			break;
		}
	case TASK_PLAY_SCRIPT:
		{
			pev->movetype = MOVETYPE_FLY;
			ClearBits(pev->flags, FL_ONGROUND);
			m_scriptState = SCRIPT_PLAYING;
			break;
		}
	case TASK_ENABLE_SCRIPT:
		{
			//MODDD - for safety, force facing the exact right direction here.
			// TASK_ENABLE_SCRIPT should only be called sometime after a task that sets pev->ideal_yaw to face
			// the scripted sequence.
			// In fact re-doing even that for safty now.
			if (m_hTargetEnt != NULL) {
				pev->ideal_yaw = UTIL_AngleMod(m_hTargetEnt->pev->angles.y);
				pev->angles.y = pev->ideal_yaw;
			}
			else {
				easyForcePrintLine("?????????");
				TaskFail();
				return;
			}
			///////////////////////////////////////////////////

			m_pCine->DelayStart( 0 );
			TaskComplete();
			break;
		}
	case TASK_PLANT_ON_SCRIPT:
		{
			if ( m_hTargetEnt != NULL )
			{
				CBaseEntity* theList[32];

				Vector livingSizeMins = VEC_HUMAN_HULL_MIN + Vector(-0.5, -0.5, 0);
				Vector livingSizeMaxs = VEC_HUMAN_HULL_MAX + Vector(0.5, 0.5, -1);

				Vector targetSpot = m_hTargetEnt->pev->origin;
				
				BOOL pass = TRUE;
				int theListSoftMax = UTIL_EntitiesInBox(theList, 32, targetSpot + livingSizeMins, targetSpot + livingSizeMaxs, 0);

				//if (monsterID == 10) {
				//	UTIL_drawBoxFrame(targetSpot + livingSizeMins, targetSpot + livingSizeMaxs, 8, 0, 255, 0);
				//	int x = 45;
				//}

				
				for (int i = 0; i < theListSoftMax; i++) {
					CBaseEntity* ent = theList[i];

					//ent->pev->solid == SOLID_BSP || ent->pev->movetype == MOVETYPE_PUSH || ent->pev->movetype == MOVETYPE_NONE

					if (
						ent->pev->solid == SOLID_NOT || ent->pev->solid == SOLID_TRIGGER
						|| (ent->IsWorldOrAffiliated() && !ent->isBreakableOrChild())
					) {
						// not collidable, or part of the map?  Skip it.
						continue;
					}
					
					// "&& ent->MyMonsterPointer() != NULL" check?  unsure if that's a good idea.
					if (ent != this && !UTIL_IsDeadEntity(ent)) {
						
						pass = FALSE;
						break;
					}

				}//for-loop
				


				if (pass) {
					pev->origin = m_hTargetEnt->pev->origin;	// Plant on target
				}
				else {
					// obstruction?  no.
					// And yes, it's ok to TaskFil and TaskComplete in the same run, the TaskFail gets precedence
					// regardless of order.
					TaskFail(); 
				}
			}

			TaskComplete();
			break;
		}
	case TASK_FACE_SCRIPT:
		{
			if ( m_hTargetEnt != NULL )
			{
				pev->ideal_yaw = UTIL_AngleMod( m_hTargetEnt->pev->angles.y );

				//MODDD - for safety, how about just forcing the angles then.
				// No, later.  Give some time to try to face the right way manually.
				// On starting the script the angles get enforced.
				// ...unless this is schedule 'slFaceScript'.  Then timing seems more important in some cases
				// like a2a1.   But verify assuming an insta-force-direction like this is a good thing 
				// for this schedule always!
				// Nope, a1a0.  The barney after hitting the panel looks weird instantly facing the player.
				// SSSSSSSssssssssiiiiggggggghhh.
				// Some flag for scripted's to force facing instantly, or maybe even wait for two things
				// to face each other before playing if coordinated like the garg throwing an agrunt,
				// would be nice.  No idea if that can be detected.
				// Point is, going to have to avoid this force-fix without some flag to say when it's
				// ok or not, per scripted-ent or sequence-order or however those are given.
				//pev->angles.y = pev->ideal_yaw;
			}

			TaskComplete();
			m_IdealActivity = ACT_IDLE;
			RouteClear();
			break;
		}

	case TASK_SUGGEST_STATE:
		{

			m_IdealMonsterState = (MONSTERSTATE)(int)pTask->flData;
			TaskComplete();
			break;
		}

	case TASK_SET_FAIL_SCHEDULE:
		m_failSchedule = (int)pTask->flData;
		hardSetFailSchedule = FALSE;
		TaskComplete();
		break;

	//MODDD - new. Same as setting a fail schedule, but mark it as hard. This fail schedule cannot be skipped by changing states (like alert to combat).
	case TASK_SET_FAIL_SCHEDULE_HARD:
		m_failSchedule = (int)pTask->flData;
		hardSetFailSchedule = TRUE;
		TaskComplete();
	break;

	case TASK_CLEAR_FAIL_SCHEDULE:
		m_failSchedule = SCHED_NONE;
		hardSetFailSchedule = FALSE; //MODDD - resets turn this off.
		TaskComplete();
		break;

	case TASK_SET_SEQUENCE_BY_NUMBER:
		{
			setAnimationSmart((int)pTask->flData, 1.0f);
			break;
		}
	//MODDD - new
	case TASK_RANDOMWANDER_CHECKSEEKSHORT:
	{
		Vector vecStart;
		Vector vecEnd;
		Vector vecFinalEnd;
		Vector vecTempOff;
		int randomTries;
		Vector randomTargetVectorAttempt;
		float randomTargetYawAttempt;
		BOOL success;
		float totalDist;
		float distReg;

		BOOL vecLastTrySuccess;
		int vecLastTryLength;
		Vector vecLastTry[4];
		Vector vecLastTryOrigin;

		//SEEK SHORT wants to pick a random direction and see if we can straight-shot it for a very short distance (not path finding).
			
		//not needed?
		//UTIL_MakeVectorsPrivate(pev->angles, vec_forward, vec_right, vec_up);

		//how many tries until we give up.
		randomTries = RANDOMWANDER_TRIES;

			
			vecStart = pev->origin + Vector(0, 0, 6);

		vecLastTryOrigin = pev->origin + Vector(0, 0, 6);

		while(randomTries > 0){

			randomTargetYawAttempt = RANDOM_FLOAT(0, 359.99);

			randomTargetVectorAttempt = UTIL_YawToVec(randomTargetYawAttempt);

			//try to go in this direction.


			//at least this test.
			vecEnd = vecStart + randomTargetVectorAttempt * 120;

			totalDist = (vecEnd - vecStart).Length();


			//record for drawing for debug purposes.
			vecLastTry[ RANDOMWANDER_TRIES - randomTries ] = vecEnd; 


			//test!
			success = (this->CheckLocalMove(vecStart, vecEnd, NULL, TRUE, &distReg) == LOCALMOVE_VALID);
				
			if(success){
				//because on success, distReg is likely not written to. Bizarre.
				distReg = totalDist;
			}

			//easyForcePrintLine("IS IT OK OR NOT?! %d %.2f", success, distReg);
			if(distReg > 55){
				//if okay?
					
				pev->ideal_yaw = randomTargetYawAttempt;

				//vecHopDest = vecEnd;
				vecTempOff = randomTargetVectorAttempt * RANDOM_FLOAT(40, distReg - 15) ;

				//NOTE: is this safe?
				this->m_vecMoveGoal = vecStart + vecTempOff;

				
					
				//UTIL_printVector("ye", vecStart);
				//UTIL_printVector("ye", randomTargetVectorAttempt);
				//UTIL_printVector("ye", vecTempOff);
				//UTIL_printVector("ye", vecHopDest);


				vecLastTrySuccess = TRUE;
				vecLastTryLength = (RANDOMWANDER_TRIES - randomTries) + 1;
				//done! Have a destination.
				TaskComplete();
				return;
			}



			randomTries--;

		}//END OF while(randomTries > 0)

		vecLastTrySuccess = FALSE;
		//all four were done.
		vecLastTryLength = RANDOMWANDER_TRIES;

		//Reached here? If the loop didn't call "TaskComplete" and return, this was reached.
		//Fail.
		TaskFail();
		break;
	}
  
  
	case TASK_RANDOMWANDER_TEST:

		if ( MoveToLocation( m_movementActivity, 2, this->m_vecMoveGoal ) )
		{
			//EASY_CVAR_PRINTIF_PRE(chumtoadPrintout, easyPrintLine("TASK_TOAD_HOPSHORT: path okay") );
			TaskComplete();
		}else{
			//EASY_CVAR_PRINTIF_PRE(chumtoadPrintout, easyPrintLine("TASK_TOAD_HOPSHORT: path not okay. How at this stage?") );
			//How???
			TaskFail();
		}
	break;
	case TASK_RESTORE_FRAMERATE:{
		pev->framerate = 1;
		m_flFramerateSuggestion = 1;

		//pick anything else, please.
		m_Activity = ACT_RESET;
		m_IdealActivity = ACT_RESET;

		TaskComplete();
	break;}
	case TASK_GATE_ORGANICLOGIC_NEAR_LKP:{
		//any organicLogic's near the last known position?
		CBaseEntity* test;
		test = getNearestDeadBody(this->m_vecEnemyLKP, 200);

		//TODO - do a linetrace from a little above the current m_vecEnemyLKP to the found corpse's real location and pick a little above that
		//       for a new m_vecEnemyLKP to get guaranteed closer to that?
		//So far this is just a pass / fail gate.

		if(test != NULL){
			//there is something organicLogic - dead.  move on.
			TaskComplete();
		}else{
			//nothing? don't go there.
			TaskFail();
			return;
		}


	break;}
	case TASK_RECORD_DEATH_STATS:{
		//Record a few things before continuing with this death schedule where changes could be made by so much as being told to stop moving.
		timeOfDeath_activity = m_Activity;
		timeOfDeath_sequence = pev->sequence;

		TaskComplete();
	break;}



	default:
		{
			ALERT ( at_aiconsole, "No StartTask entry for %d\n", (SHARED_TASKS)pTask->iTask );
			break;
		}
	}
}

//=========================================================
// GetTask - returns a pointer to the current 
// scheduled task. NULL if there's a problem.
//=========================================================
Task_t	*CBaseMonster::GetTask ( void ) 
{
	//MODDD - any random crashes caused by this?  Doubt it, but a null check never hurts.
	if(m_pSchedule == NULL){
		return NULL;
	}


	if ( m_iScheduleIndex < 0 || m_iScheduleIndex >= m_pSchedule->cTasks )
	{
		// m_iScheduleIndex is not within valid range for the monster's current schedule.
		return NULL;
	}
	else
	{
		return &m_pSchedule->pTasklist[ m_iScheduleIndex ];
	}
}


//MODDD - get schedule name, typically for printouts.
const char* CBaseMonster::getScheduleName(void){
	if(m_pSchedule != NULL){
		return m_pSchedule->pName;
	}else{
		return "NULL!";
	}
}
//MODDD - new, intended for printout ease (not necessarily limited to)
int CBaseMonster::getTaskNumber(void){
	Task_t* attempt = GetTask();
	if(attempt != NULL){
		return attempt->iTask;
	}else{
		return -1;
	}
}




//=========================================================
// GetSchedule - Decides which type of schedule best suits
// the monster's current state and conditions. Then calls
// monster's member function to get a pointer to a schedule
// of the proper type.
//=========================================================
Schedule_t *CBaseMonster::GetSchedule ( void )
{
	//MODDD - safety.
	//if(pev->deadflag != DEAD_NO){
	//	return GetScheduleOfType( SCHED_DIE );
	//}
	if (FClassnameIs(pev, "monster_gargantua")) {
		int x = 444;
	}


	//MODDD
	// IF schedule is NULL and we're without a pCine,
	// and the idealmonsterstate is not SCRIPT yet 'waitForScriptedTime' is not null,
	// wait to give a new schedule.  Other places accepting a schedule need
	// to be able to deal with a NULL schedule.
	if (
		m_pSchedule == NULL &&
		m_pCine == NULL &&
		// m_MonsterState != MONSTERSTATE_SCRIPT && 
		m_IdealMonsterState != MONSTERSTATE_SCRIPT
	){

		if (
			gpGlobals->time < waitForScriptedTime
		){
			// STALL IT.  The  != -1  check lets it work for at least one frame.

			easyPrintLine("!!! Script debug: Here I is! curtime:%.2f waitForScrTime:%.2f", gpGlobals->time, waitForScriptedTime);
			return NULL;
		}
		
		if (waitForScriptedTime != -1) {
			//easyPrintLine("Script debug: Here I is! surpassed frame. curtime:%.2f waitForScrTime:%.2f", gpGlobals->time, waitForScriptedTime);
			easyPrintLine("!!! Surpassed frame!");
			waitForScriptedTime = -1;

			return NULL;
		}


	}
	//////////////////////////////////////////////////////////////////




	SCHEDULE_TYPE baitSched = getHeardBaitSoundSchedule();

	if(baitSched != SCHED_NONE){
		return GetScheduleOfType ( baitSched );
	}

	switch	( m_MonsterState )
	{
	case MONSTERSTATE_PRONE:
		{
			return GetScheduleOfType( SCHED_BARNACLE_VICTIM_GRAB );
			break;
		}
	case MONSTERSTATE_NONE:
		{
			ALERT ( at_aiconsole, "MONSTERSTATE IS NONE!\n" );
			break;
		}
	case MONSTERSTATE_IDLE:
		{
			if ( HasConditions ( bits_COND_HEAR_SOUND ) )
			{
				return GetScheduleOfType( SCHED_ALERT_FACE );
			}
			else if ( FRouteClear() )
			{
				// no valid route!
				return GetScheduleOfType( SCHED_IDLE_STAND );
			}
			else
			{
				// valid route. Get moving
				return GetScheduleOfType( SCHED_IDLE_WALK );
			}
			break;
		}
	case MONSTERSTATE_ALERT:
		{
			if ( HasConditions( bits_COND_ENEMY_DEAD ) && LookupActivity( ACT_VICTORY_DANCE ) != ACTIVITY_NOT_AVAILABLE )
			{
				return GetScheduleOfType ( SCHED_VICTORY_DANCE );
			}


			//MODDD - new.  Taking a huge amount of damage forces the big flinch and look
			if(HasConditions(bits_COND_HEAVY_DAMAGE)){
				return GetScheduleOfType(SCHED_ALERT_BIG_FLINCH);
			}



			if ( HasConditions(bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE) )
			{
				if ( fabs( FlYawDiff() ) < (1.0 - m_flFieldOfView) * 60 ) // roughly in the correct direction
				{
					return GetScheduleOfType( SCHED_TAKE_COVER_FROM_ORIGIN );
				}
				else
				{
					return GetScheduleOfType( SCHED_ALERT_SMALL_FLINCH );
				}
			}

			else if ( HasConditions ( bits_COND_HEAR_SOUND ) )
			{
				return GetScheduleOfType( SCHED_ALERT_FACE );
			}
			else
			{
				return GetScheduleOfType( SCHED_ALERT_STAND );
			}
			break;
		}
	case MONSTERSTATE_COMBAT:
		{
			if ( HasConditions( bits_COND_ENEMY_DEAD ) )
			{

				if(monsterID == 9){
					int x = 45;
				}

				// clear the current (dead) enemy and try to find another.
				m_hEnemy = NULL;

				//MODDD MAJOR NOTE - hold on... how does this even work?
				//                   GetEnemy does nothing if the current (or old schedule about to be changed to something new?) forbids being interrupted by a new enemy?
				//                   Although a schedule being interruptable by conditions or not doesn't affect whether the conditions can be set period. That might make sense.
				//                   So if any monster is in sight as marked by some SEE_... condition, it's allowed to commit it to m_hEnemy and pass.
				if ( GetEnemy() )
				{
					ClearConditions( bits_COND_ENEMY_DEAD );
					return GetSchedule();
				}
				else
				{
					SetState( MONSTERSTATE_ALERT );
					return GetSchedule();
				}
			}

			//int testtt = HasConditions(bits_COND_HEAVY_DAMAGE);

			if ( HasConditions(bits_COND_NEW_ENEMY) )
			{
				return GetScheduleOfType ( SCHED_WAKE_ANGRY );
			}
			//MODDD - NEW.  Must be pasted to most monster's GetSchedule's to have an effect since most use their own custom versions completely or nearly so.
			else if(HasConditions(bits_COND_HEAVY_DAMAGE)){
				//MODDD - taking heavy damage is more drastic now with its own check.
				//It won't happen often enough to need memory for blocking.
				return GetScheduleOfType(SCHED_BIG_FLINCH);
			}
			//MODDD - other condition.  If "noFlinchOnHard" is on and the skill is hard, don't flinch from getting hit.
			else if (HasConditions(bits_COND_LIGHT_DAMAGE) && !HasMemory( bits_MEMORY_FLINCHED) && !(EASY_CVAR_GET_DEBUGONLY(noFlinchOnHard)==1 && g_iSkillLevel==SKILL_HARD)  )
			{
				return GetScheduleOfType( SCHED_SMALL_FLINCH );
			}
			else if ( !HasConditions
				(bits_COND_SEE_ENEMY)
			)
			{
				// we can't see the enemy
				if ( !HasConditions(bits_COND_ENEMY_OCCLUDED) )
				{
					
					if(!FacingIdeal()){
						// enemy is unseen, but not occluded!
						// turn to face enemy
						return GetScheduleOfType(SCHED_COMBAT_FACE);
					}else{
						//We're facing the LKP already. Then we have to go to that point and declare we're stumped there if we still see nothing.
						return GetScheduleOfType(SCHED_CHASE_ENEMY);
					}
				}
				else
				{
					// chase!
					//easyPrintLine("ducks??");

					if(m_hEnemy != NULL && IRelationship(m_hEnemy) == R_FR){
						//if I fear this enemy, and they are not seen and are occluded, just stare.
						return GetScheduleOfType(SCHED_COMBAT_FACE);
					}else{
						return GetScheduleOfType( SCHED_CHASE_ENEMY );
					}
				}
			}
			else  
			{
				//easyPrintLine("I say, what? %d %d", HasConditions(bits_COND_CAN_RANGE_ATTACK1), HasConditions(bits_COND_CAN_RANGE_ATTACK2) );

				if(m_hEnemy != NULL && IRelationship(m_hEnemy) == R_FR){
					if( HasConditions(bits_COND_CAN_MELEE_ATTACK1 | bits_COND_CAN_MELEE_ATTACK2) ){
						//if either melee attack is possible, lower chance of running away.
						if(RANDOM_LONG(0, 4) <= 0){
							return GetScheduleOfType( SCHED_FIGHT_OR_FLIGHT );
						}
					}else{
						//can't melee? higher chance of running away.
						if(RANDOM_LONG(0, 4) <= 2){
							return GetScheduleOfType( SCHED_FIGHT_OR_FLIGHT );
						}
					}
				}


				// we can see the enemy
				if ( HasConditions(bits_COND_CAN_RANGE_ATTACK1) )
				{
					return GetScheduleOfType( SCHED_RANGE_ATTACK1 );
				}
				if ( HasConditions(bits_COND_CAN_RANGE_ATTACK2) )
				{
					return GetScheduleOfType( SCHED_RANGE_ATTACK2 );
				}
				if ( HasConditions(bits_COND_CAN_MELEE_ATTACK1) )
				{
					return GetScheduleOfType( SCHED_MELEE_ATTACK1 );
				}
				if ( HasConditions(bits_COND_CAN_MELEE_ATTACK2) )
				{
					return GetScheduleOfType( SCHED_MELEE_ATTACK2 );
				}

				//MODDD - NOTE - is that intentional?  range1 & melee1,  and not say,  melee1 & melee2???
				//MODDD - ok, this condition is redundant. If all 4 condition checks above failed, each RANGE and MELEE attack, 1 and 2, failed.
				//        That means RANGE1 and MELEE1 also had to have failed. This is guaranteed true.
				// REPLACED.  How about involving the new CouldAttack conditions here instead?
				//if ( !HasConditions(bits_COND_CAN_RANGE_ATTACK1 | bits_COND_CAN_MELEE_ATTACK1) )
				if(!HasConditionsMod(bits_COND_COULD_MELEE_ATTACK1 | bits_COND_COULD_MELEE_ATTACK2 | bits_COND_COULD_RANGE_ATTACK1 | bits_COND_COULD_RANGE_ATTACK2))
				{

					if(m_hEnemy != NULL && IRelationship(m_hEnemy) == R_FR){
						//if I fear the enemy and have no possible attacks, have another 3/4 chance of running away.
						if(RANDOM_LONG(0, 3) < 3){
							//of course if we can't find cover, chase anyways.
							return GetScheduleOfType( ::SCHED_TAKE_COVER_FROM_ENEMY_OR_CHASE );
						}
					}



					// if we can see enemy but can't use either attack type, we must need to get closer to enemy
					//easyPrintLine("ducks2");
					return GetScheduleOfType( SCHED_CHASE_ENEMY );

				}
				//else if ( !FacingIdeal() )
				//MODDD - FacingIdeal() check removed.  Likely didn't update the FacingIdeal from whatever failed pathfinding check to be
				// the enemy, which is fine.  It probably won't.
				// From having any of the COULD conditions above to even reach this point, go ahead and face them to change something.
				else
				{
					//turn
					return GetScheduleOfType( SCHED_COMBAT_FACE );
				}
				//else
				//{
				//	ALERT ( at_aiconsole, "No suitable combat schedule!\n" );
				//}
			}
			break;
		}
	case MONSTERSTATE_DEAD:
		{
			return GetScheduleOfType( SCHED_DIE );
			break;
		}
	case MONSTERSTATE_SCRIPT:
		{
			//
			//ASSERT( m_pCine != NULL );

			if(m_pCine == NULL){
				easyPrintLine("WARNING: m_pCine IS NULL!");
			}

			if ( !m_pCine )
			{
				ALERT( at_aiconsole, "Script failed for %s\n", STRING(pev->classname) );
				CineCleanup();
				return GetScheduleOfType( SCHED_IDLE_STAND );
			}

			return GetScheduleOfType( SCHED_AISCRIPT );
		}
	default:
		{
			ALERT ( at_aiconsole, "Invalid State for GetSchedule!\n" );
			break;
		}
	}

	return &slError[ 0 ];
}
