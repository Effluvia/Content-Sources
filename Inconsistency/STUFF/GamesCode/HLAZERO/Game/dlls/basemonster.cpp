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
/*

===== basemonster.cpp (formerly monsters.cpp) ========================================================

  Monster-related utility code

*/

// IDEA:  CheckLocalMove could check for a monster having ground-based movement and, if so, do a TRACE_MONSTER_HULL
// check straight down to get the position on the ground, unsure if that would be expensive.  Maybe whenever the LKP
// is updated as some Z offset instead ('z from ground').
// That way, jumping in place doesn't throw off some Z checks.



// NOTE!!! Bad change I made a long while ago.
// BuildNearestRoute should NOT give the 'bits_MF_TO_ENEMY' moveflag.
// This tricks other areas of pathfinding into thinking the route is directly to the enemy
// (often way off coming from BuildNearestRoute, the point is to get unscrambled hopefully to a better position it seems),
// so that it notices there is over 80 units of distance between the enemy LKP (say position) and the current goal node, so that
// it must re-route.  This ONLY makes sense for a route intended to be directly to the enemy, so that the enemy moving too far
// from the 'goal' of a direct route means it should be refreshed.
// Refreshing NearestRoutes for being too far will lead to refreshing every single frame, not good.
// hgrunts placed just right while standing can oscillate between going towards different nodes very quickly
// from taking different randomly picked goal-nodes each time.
// It is now safe to send the moveflag anyway, the method will ignore it.  Or maybe use it in some other way sometime.
// Just don't let nodes placed by BuildNearestRoute take the moveflag.




// Should 'usingCustomSequence' and 'doNotResetSequence' be saved?  I have no idea.

//MODDD - TODO! See the comment by the start of 'm_SaveData'.  Saving schedules and routes may
// be feasible now thanks to EHANDLEs, even if the devs never did.  Will try that out sometime maybe.


#include "extdll.h"
#include "basemonster.h"  //MODDD - is this addition necessary?
#include "util.h"	
#include "nodes.h"
#include "util_model.h"
#include "saverestore.h"
#include "weapons.h"
#include "scripted.h"
#include "squadmonster.h"
#include "decals.h"
#include "soundent.h"
#include "gamerules.h"
//MODDD - never included before.?
#include "defaultai.h"
#include "util_debugdraw.h"
//MODDD - why not?
#include "game.h"
#include "player.h"
	
	
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST(sv_germancensorship)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(seeMonsterHealth)
EASY_CVAR_EXTERN_DEBUGONLY(applyLKPPathFixToAll)
EASY_CVAR_EXTERN_DEBUGONLY(crazyMonsterPrintouts)
EASY_CVAR_EXTERN_DEBUGONLY(monsterSpawnPrintout)
EASY_CVAR_EXTERN_DEBUGONLY(timedDamageAffectsMonsters)
EASY_CVAR_EXTERN_DEBUGONLY(STUextraTriangH)
EASY_CVAR_EXTERN_DEBUGONLY(STUextraTriangV)
EASY_CVAR_EXTERN_DEBUGONLY(timedDamageEndlessOnHard)
extern float globalPSEUDO_canApplyGermanCensorship;
EASY_CVAR_EXTERN_DEBUGONLY(allowGermanModels)
EASY_CVAR_EXTERN_DEBUGONLY(cineAllowSequenceOverwrite)
EASY_CVAR_EXTERN_DEBUGONLY(pathfindPrintout)
EASY_CVAR_EXTERN_DEBUGONLY(pathfindFidgetFailTime)
EASY_CVAR_EXTERN_DEBUGONLY(pathfindTopRampFixDistance)
EASY_CVAR_EXTERN_DEBUGONLY(pathfindTopRampFixDraw)
EASY_CVAR_EXTERN_DEBUGONLY(pathfindLooseMapNodes)
EASY_CVAR_EXTERN_DEBUGONLY(pathfindRampFix)
EASY_CVAR_EXTERN_DEBUGONLY(pathfindNodeToleranceMulti)
EASY_CVAR_EXTERN(pathfindSnapToNode)
EASY_CVAR_EXTERN_DEBUGONLY(animationFramerateMulti)
EASY_CVAR_EXTERN_DEBUGONLY(peaceOut)
EASY_CVAR_EXTERN_DEBUGONLY(monsterAIForceFindDistance)
EASY_CVAR_EXTERN_DEBUGONLY(movementIsCompletePrintout)
EASY_CVAR_EXTERN_DEBUGONLY(animationPrintouts)
EASY_CVAR_EXTERN_DEBUGONLY(pathfindEdgeCheck)
EASY_CVAR_EXTERN_DEBUGONLY(thoroughHitBoxUpdates)
EASY_CVAR_EXTERN_DEBUGONLY(animationKilledBoundsRemoval)
EASY_CVAR_EXTERN_DEBUGONLY(drawDebugEnemyLKP)
EASY_CVAR_EXTERN_DEBUGONLY(pathfindLargeBoundFix)
EASY_CVAR_EXTERN_DEBUGONLY(flyerKilledFallingLoop)
EASY_CVAR_EXTERN_DEBUGONLY(barnacleGrabNoInterpolation)
EASY_CVAR_EXTERN_DEBUGONLY(pathfindForcePointHull)
EASY_CVAR_EXTERN_DEBUGONLY(pathfindMonsterclipFreshLogic)
EASY_CVAR_EXTERN_DEBUGONLY(pathfindIgnoreIsolatedNodes)

//MODDD - unused, now factors in the monster's current expected distance to cover this frame (groundspeed & frame time) 
//        for a better tolerance instead.  Still a hard minimum of 8 to pass just in case it would otherwise be too small.
//#define MONSTER_CUT_CORNER_DIST		8 // 8 means the monster's bounding box is contained without the box of the node in WC



BOOL g_FGetNodeRoute_recursiveCall = FALSE;




// ok... don't know what I was smoking there.  AdvanceRoute as a respose to failing in MoveExecute?...   uhhhhhh.   wat.
// Still works so long as USE_MOVEMENT_BOUND_FIX is also on now it seems.  I got nothing.
// Disabling these fixes for MoveExecute, lead to a rare bug of something moving in a way that makes the player get stuck in itself
// as it approaches.  Hard to see in normal use but much more common by getting a gargantuta to see you (make enemy), turn on autoSneaky,
// and get close to the garg.  It might path into you and keep walking so that its bounds get stuck and neither can move away.
// This looks to be because the fix changes the bounds of the garg before pathfinding.  Unfortunate this side effect happens.
// Changed into two pairs of constants: USE_MOVEMENT_BOUND_FIX_MOVEEXC + ALT, and USE_MOVEMENT_BOUND_FIX_CHKLOC + ALT.

#define USE_MOVEMENT_BOUND_FIX_CHKLOC
#define USE_MOVEMENT_BOUND_FIX_CHKLOC_ALT
//#define USE_MOVEMENT_BOUND_FIX_MOVEEXC
//#define USE_MOVEMENT_BOUND_FIX_MOVEEXC_ALT



// IMPORTANT!  In MoveExecute, Monsters going up some changes in elevation like ramps can have problems snapping
// to the surface after the ramp, at least in oddly designed cases like the top of c2a3's ramp from down to up.
// Try making UNSTUCK_STEP_SIZE larger, only used if this monster is detected stuck to try and get past something
// in the way.
// was 24.
#define UNSTUCK_STEP_SIZE 52


//MODDD - changed from 200 to 300, check out a little further.
#define DIST_TO_CHECK	300

// Turned into a constant.  (Was 8)
#define MAX_TRIANGULATION_ATTEMPTS 4


//extern DLL_GLOBAL	BOOL	g_fDrawLines;
extern DLL_GLOBAL short g_sModelIndexLaser;// holds the index for the laser beam
extern DLL_GLOBAL short g_sModelIndexLaserDot;// holds the index for the laser beam dot

extern CGraph WorldGraph;// the world node graph


extern CBaseMonster* g_routeTempMonster;
// not necesarily m_hTargetEnt, could be m_hEnemy or whatever else would usually go to CheckLocalMove
// calls within routing?
extern CBaseEntity* g_routeTempMonster_GoalEnt;



BOOL g_pathfind_preMoveOnly = FALSE;
BOOL g_CheckLocalMoveCanReportBlocker = FALSE;
BOOL g_CheckLocalMove_ExtraDebug = FALSE;


//extern Schedule_t* slAnimationSmartAndStop;
//extern Schedule_t* slAnimationSmart;


/*
float CBaseMonster::paralyzeDuration = 0;
float CBaseMonster::nervegasDuration = 0;
float CBaseMonster::poisonDuration = 0;
float CBaseMonster::radiationDuration = 0;
float CBaseMonster::acidDuration = 0;
float CBaseMonster::slowburnDuration = 0;
float CBaseMonster::slowfreezeDuration = 0;
float CBaseMonster::bleedingDuration = 0;
*/
/*
float CBaseMonster::nervegasDamage = 0;
float CBaseMonster::poisonDamage = 0;
float CBaseMonster::radiationDamage = 0;
float CBaseMonster::acidDamage = 0;
float CBaseMonster::slowburnDamage = 0;
float CBaseMonster::slowfreezeDamage = 0;
float CBaseMonster::bleedingDamage = 0;
*/


// Global Savedata for monster
// UNDONE: Save schedule data?  Can this be done?  We may
// lose our enemy pointer or other data (goal ent, target, etc)
// that make the current schedule invalid, perhaps it's best
// to just pick a new one when we start up again
//MODDD - That is an as-is comment and not a bad idea.  Perpaps that idea was made before
// EHANDLES were created?  Now the m_hEnemy, m_hTargetEnt, etc. are saved reliably just fine,
// so it may be fine to restore schedule-saving.
// See monstersavestate.h for a lot of specifics related to schedules.
// Same for routes, seems those got cut from being saved too.
TYPEDESCRIPTION	CBaseMonster::m_SaveData[] = 
{
	DEFINE_FIELD( CBaseMonster, m_hEnemy, FIELD_EHANDLE ),
	DEFINE_FIELD( CBaseMonster, m_hTargetEnt, FIELD_EHANDLE ),
	DEFINE_ARRAY( CBaseMonster, m_hOldEnemy, FIELD_EHANDLE, MAX_OLD_ENEMIES ),
	DEFINE_ARRAY( CBaseMonster, m_vecOldEnemy, FIELD_POSITION_VECTOR, MAX_OLD_ENEMIES ),
	//MODDD - new
	DEFINE_ARRAY( CBaseMonster, m_flOldEnemy_zOffset, FIELD_FLOAT, MAX_OLD_ENEMIES ),
	DEFINE_ARRAY( CBaseMonster, m_vecOldEnemy_ViewOFS, FIELD_VECTOR, MAX_OLD_ENEMIES ),
	DEFINE_FIELD( CBaseMonster, m_intOldEnemyNextIndex, FIELD_INTEGER),


	DEFINE_FIELD( CBaseMonster, m_flFieldOfView, FIELD_FLOAT ),
	DEFINE_FIELD( CBaseMonster, m_flWaitFinished, FIELD_TIME ),
	DEFINE_FIELD( CBaseMonster, m_flMoveWaitFinished, FIELD_TIME ),

	DEFINE_FIELD( CBaseMonster, m_Activity, FIELD_INTEGER ),
	DEFINE_FIELD( CBaseMonster, m_IdealActivity, FIELD_INTEGER ),
	DEFINE_FIELD( CBaseMonster, m_LastHitGroup, FIELD_INTEGER ),
	DEFINE_FIELD( CBaseMonster, m_MonsterState, FIELD_INTEGER ),
	DEFINE_FIELD( CBaseMonster, m_IdealMonsterState, FIELD_INTEGER ),
	DEFINE_FIELD( CBaseMonster, m_iTaskStatus, FIELD_INTEGER ),

	//Schedule_t			*m_pSchedule;

	DEFINE_FIELD( CBaseMonster, m_iScheduleIndex, FIELD_INTEGER ),
	DEFINE_FIELD( CBaseMonster, m_afConditions, FIELD_INTEGER ),
	//MODDD - new
	DEFINE_FIELD( CBaseMonster, m_afConditionsNextFrame, FIELD_INTEGER ),

	//MODDD - new too
	DEFINE_FIELD( CBaseMonster, m_afConditionsMod, FIELD_INTEGER ),
	DEFINE_FIELD( CBaseMonster, m_afConditionsModNextFrame, FIELD_INTEGER ),


	//WayPoint_t			m_Route[ ROUTE_SIZE ];
//	DEFINE_FIELD( CBaseMonster, m_movementGoal, FIELD_INTEGER ),
//	DEFINE_FIELD( CBaseMonster, m_iRouteIndex, FIELD_INTEGER ),
//	DEFINE_FIELD( CBaseMonster, m_moveWaitTime, FIELD_FLOAT ),

	DEFINE_FIELD( CBaseMonster, m_vecMoveGoal, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( CBaseMonster, m_movementActivity, FIELD_INTEGER ),

	//		int				m_iAudibleList; // first index of a linked list of sounds that the monster can hear.
//	DEFINE_FIELD( CBaseMonster, m_afSoundTypes, FIELD_INTEGER ),
	DEFINE_FIELD( CBaseMonster, m_vecLastPosition, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( CBaseMonster, m_iHintNode, FIELD_INTEGER ),
	DEFINE_FIELD( CBaseMonster, m_afMemory, FIELD_INTEGER ),
	DEFINE_FIELD( CBaseMonster, m_iMaxHealth, FIELD_INTEGER ),

	DEFINE_FIELD( CBaseMonster, m_vecEnemyLKP, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( CBaseMonster, m_flEnemyLKP_zOffset, FIELD_FLOAT),
	DEFINE_FIELD( CBaseMonster, m_vecEnemyLKP_ViewOFS, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( CBaseMonster, m_fEnemyLKP_EverSet, FIELD_BOOLEAN),
	//MODDD - new
	DEFINE_FIELD( CBaseMonster, m_vecEnemyLKP_Real, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( CBaseMonster, m_flEnemyLKP_Real_zOffset, FIELD_FLOAT),
	DEFINE_FIELD( CBaseMonster, m_vecEnemyLKP_Real_ViewOFS, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( CBaseMonster, m_fEnemyLKP_Real_EverSet, FIELD_BOOLEAN),

	DEFINE_FIELD( CBaseMonster, m_vecEnemyLKP_prev, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( CBaseMonster, m_flEnemyLKP_prev_zOffset, FIELD_FLOAT),
	DEFINE_FIELD( CBaseMonster, m_vecEnemyLKP_prev_ViewOFS, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( CBaseMonster, m_fEnemyLKP_prev_EverSet, FIELD_BOOLEAN),

	DEFINE_FIELD( CBaseMonster, investigatingAltLKP, FIELD_BOOLEAN),

	
	DEFINE_FIELD( CBaseMonster, m_cAmmoLoaded, FIELD_INTEGER ),
	DEFINE_FIELD( CBaseMonster, m_afCapability, FIELD_INTEGER ),

	//MODDD - removed.
	//DEFINE_FIELD( CBaseMonster, m_flNextAttack, FIELD_TIME ),
	DEFINE_FIELD( CBaseMonster, m_bitsDamageType, FIELD_INTEGER ),
	DEFINE_FIELD( CBaseMonster, m_bitsDamageTypeMod, FIELD_INTEGER ),
	

	DEFINE_FIELD( CBaseMonster, m_tbdPrev, FIELD_TIME ),
	DEFINE_ARRAY( CBaseMonster, m_rgbTimeBasedDamage, FIELD_CHARACTER, itbd_COUNT ),
	DEFINE_ARRAY( CBaseMonster, m_rgbTimeBasedFirstFrame, FIELD_BOOLEAN, itbd_COUNT ),


	DEFINE_FIELD( CBaseMonster, m_bloodColor, FIELD_INTEGER ),
	DEFINE_FIELD( CBaseMonster, m_failSchedule, FIELD_INTEGER ),
	DEFINE_FIELD( CBaseMonster, hardSetFailSchedule, FIELD_BOOLEAN ),  //MODDD - saved now too.
	DEFINE_FIELD( CBaseMonster, scheduleSurvivesStateChange, FIELD_BOOLEAN ),  //MODDD - saved now too.

	

	DEFINE_FIELD( CBaseMonster, m_flHungryTime, FIELD_TIME ),
	DEFINE_FIELD( CBaseMonster, m_flDistTooFar, FIELD_FLOAT ),
	DEFINE_FIELD( CBaseMonster, m_flDistLook, FIELD_FLOAT ),
	DEFINE_FIELD( CBaseMonster, m_iTriggerCondition, FIELD_INTEGER ),
	DEFINE_FIELD( CBaseMonster, m_iszTriggerTarget, FIELD_STRING ),

	DEFINE_FIELD( CBaseMonster, m_HackedGunPos, FIELD_VECTOR ),

	DEFINE_FIELD( CBaseMonster, m_scriptState, FIELD_INTEGER ),
	DEFINE_FIELD( CBaseMonster, m_pCine, FIELD_CLASSPTR ),

	//MODDD - is this a good idea?
	DEFINE_FIELD( CBaseMonster, monsterID, FIELD_INTEGER ),
	DEFINE_FIELD( CBaseMonster, strictNodeTolerance, FIELD_BOOLEAN),
	DEFINE_FIELD( CBaseMonster, goalDistTolerance, FIELD_FLOAT),
	
	DEFINE_FIELD( CBaseMonster, floatSinkSpeed, FIELD_FLOAT),
	DEFINE_FIELD( CBaseMonster, floatEndTimer, FIELD_TIME),

	DEFINE_FIELD( CBaseMonster, nextDirectRouteAttemptTime, FIELD_TIME),
	
};





//IMPLEMENT_SAVERESTORE( CBaseMonster, CBaseToggle );
int CBaseMonster::Save(CSave& save)
{
	if (!CBaseToggle::Save(save))
		return 0;
	return save.WriteFields("CBaseMonster", this, m_SaveData, ARRAYSIZE(m_SaveData));
}

int CBaseMonster::Restore(CRestore& restore)
{
	if (!CBaseToggle::Restore(restore))
		return 0;
	int status = restore.ReadFields("CBaseMonster", this, m_SaveData, ARRAYSIZE(m_SaveData));

	// We don't save/restore routes yet
	RouteClear();

	// We don't save/restore schedules yet
	m_pSchedule = NULL;
	m_iTaskStatus = TASKSTATUS_NEW;

	// Reset animation
	m_Activity = ACT_RESET;

	// If we don't have an enemy, clear conditions like see enemy, etc.
	if (m_hEnemy == NULL) {
		ClearAllConditionsExcept(bits_COND_TASK_FAILED | bits_COND_SCHEDULE_DONE);
		ClearAllConditionsMod();
	}
	
	PostRestore();

	return status;
}


void CBaseMonster::PostRestore() {
	//easyForcePrintLine("PostRestore: CBaseMonster: %s", getClassname(), monsterID);
}





//MODDD - moved from method iRelationship to instance scope. Accessible from other places now.
//   And why did ALIENMONSTER and ALIENPREY have a R_NO with MACHINE? Machines (like sentries, part of security or human military) certainly hate them.
//   Same for HUMAN_PASSIVE and ALIENPASSIVE?   why have a reacion of R_NO while machines attack them?.
//static int iEnemy[14][14] =
int CBaseMonster::iEnemy[14][14] =
{			 //   NONE	 MACH	 PLYR	 HPASS	 HMIL	 AMIL	 APASS	 AMONST	APREY	 APRED	 INSECT	PLRALY	PBWPN	ABWPN
/*NONE*/		{ R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO,	R_NO,	R_NO	},
/*MACHINE*/		{ R_NO	,R_NO	,R_DL	,R_DL	,R_NO	,R_DL	,R_DL	,R_DL	,R_DL	,R_DL	,R_NO	,R_DL,	R_DL,	R_DL	},
/*PLAYER*/		{ R_NO	,R_DL	,R_NO	,R_NO	,R_DL	,R_DL	,R_DL	,R_DL	,R_DL	,R_DL	,R_NO	,R_NO,	R_DL,	R_DL	},
/*HUMANPASSIVE*/{ R_NO	,R_DL	,R_AL	,R_AL	,R_HT	,R_FR	,R_NO	,R_HT	,R_DL	,R_FR	,R_NO	,R_AL,	R_NO,	R_NO	},
/*HUMANMILITAR*/{ R_NO	,R_NO	,R_HT	,R_DL	,R_NO	,R_HT	,R_DL	,R_DL	,R_DL	,R_DL	,R_NO	,R_HT,	R_NO,	R_NO	},
/*ALIENMILITAR*/{ R_NO	,R_DL	,R_HT	,R_DL	,R_HT	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_DL,	R_NO,	R_NO	},
/*ALIENPASSIVE*/{ R_NO	,R_DL	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO,	R_NO,	R_NO	},
/*ALIENMONSTER*/{ R_NO	,R_DL	,R_DL	,R_DL	,R_DL	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_DL,	R_NO,	R_NO	},
/*ALIENPREY   */{ R_NO	,R_DL	,R_DL	,R_DL	,R_DL	,R_NO	,R_NO	,R_NO	,R_NO	,R_FR	,R_NO	,R_DL,	R_NO,	R_NO	},
/*ALIENPREDATO*/{ R_NO	,R_DL	,R_DL	,R_DL	,R_DL	,R_NO	,R_NO	,R_NO	,R_HT	,R_DL	,R_NO	,R_DL,	R_NO,	R_NO	},
/*INSECT*/		{ R_FR	,R_FR	,R_FR	,R_FR	,R_FR	,R_NO	,R_FR	,R_FR	,R_FR	,R_FR	,R_NO	,R_FR,	R_NO,	R_NO	},
/*PLAYERALLY*/	{ R_NO	,R_DL	,R_AL	,R_AL	,R_DL	,R_DL	,R_DL	,R_DL	,R_DL	,R_DL	,R_NO	,R_NO,	R_NO,	R_NO	},
/*PBIOWEAPON*/	{ R_NO	,R_NO	,R_DL	,R_DL	,R_DL	,R_DL	,R_DL	,R_DL	,R_DL	,R_DL	,R_NO	,R_DL,	R_NO,	R_DL	},
/*ABIOWEAPON*/	{ R_NO	,R_NO	,R_DL	,R_DL	,R_DL	,R_AL	,R_NO	,R_NO	,R_DL	,R_NO	,R_NO	,R_DL,	R_DL,	R_NO	}
};


//MODDD.  Use on a truce between hmilitary/machine and humanpassive/humanmilitary
int CBaseMonster::iEnemyTruce[14][14] =
{			 //   NONE	 MACH	 PLYR	 HPASS	 HMIL	 AMIL	 APASS	 AMONST	APREY	 APRED	 INSECT	PLRALY	PBWPN	ABWPN
/*NONE*/		{ R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO,	R_NO,	R_NO	},
/*MACHINE*/		{ R_NO	,R_NO	,R_DL	,R_NO	,R_NO	,R_DL	,R_DL	,R_DL	,R_DL	,R_DL	,R_NO	,R_NO,	R_DL,	R_DL	},
/*PLAYER*/		{ R_NO	,R_DL	,R_NO	,R_NO	,R_DL	,R_DL	,R_DL	,R_DL	,R_DL	,R_DL	,R_NO	,R_NO,	R_DL,	R_DL	},
/*HUMANPASSIVE*/{ R_NO	,R_NO	,R_AL	,R_AL	,R_NO	,R_FR	,R_NO	,R_HT	,R_DL	,R_FR	,R_NO	,R_AL,	R_NO,	R_NO	},
/*HUMANMILITAR*/{ R_NO	,R_NO	,R_HT	,R_NO	,R_NO	,R_HT	,R_DL	,R_DL	,R_DL	,R_DL	,R_NO	,R_NO,	R_NO,	R_NO	},
/*ALIENMILITAR*/{ R_NO	,R_DL	,R_HT	,R_DL	,R_HT	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_DL,	R_NO,	R_NO	},
/*ALIENPASSIVE*/{ R_NO	,R_DL	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO,	R_NO,	R_NO	},
/*ALIENMONSTER*/{ R_NO	,R_DL	,R_DL	,R_DL	,R_DL	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_DL,	R_NO,	R_NO	},
/*ALIENPREY   */{ R_NO	,R_DL	,R_DL	,R_DL	,R_DL	,R_NO	,R_NO	,R_NO	,R_NO	,R_FR	,R_NO	,R_DL,	R_NO,	R_NO	},
/*ALIENPREDATO*/{ R_NO	,R_DL	,R_DL	,R_DL	,R_DL	,R_NO	,R_NO	,R_NO	,R_HT	,R_DL	,R_NO	,R_DL,	R_NO,	R_NO	},
/*INSECT*/		{ R_FR	,R_FR	,R_FR	,R_FR	,R_FR	,R_NO	,R_FR	,R_FR	,R_FR	,R_FR	,R_NO	,R_FR,	R_NO,	R_NO	},
/*PLAYERALLY*/	{ R_NO	,R_NO	,R_AL	,R_AL	,R_NO	,R_DL	,R_DL	,R_DL	,R_DL	,R_DL	,R_NO	,R_NO,	R_NO,	R_NO	},
/*PBIOWEAPON*/	{ R_NO	,R_NO	,R_DL	,R_DL	,R_DL	,R_DL	,R_DL	,R_DL	,R_DL	,R_DL	,R_NO	,R_DL,	R_NO,	R_DL	},
/*ABIOWEAPON*/	{ R_NO	,R_NO	,R_DL	,R_DL	,R_DL	,R_AL	,R_NO	,R_NO	,R_DL	,R_NO	,R_NO	,R_DL,	R_DL,	R_NO	}
};




const char* CBaseMonster::pStandardAttackHitSounds[] = 
{
	"zombie/claw_strike1.wav",
	"zombie/claw_strike2.wav",
	"zombie/claw_strike3.wav",
};

const char* CBaseMonster::pStandardAttackMissSounds[] = 
{
	"zombie/claw_miss1.wav",
	"zombie/claw_miss2.wav",
};



int CBaseMonster::monsterIDLatest = 0;



//MODDD - ?
//WARNING: "endVec" is unused, this still gets the enemy's BodyTargetMod manually (ignores the player's randomness of BodyTarget).
BOOL CBaseMonster::NoFriendlyFireImp(const Vector& startVec, const Vector& endVec){
	
	//easyForcePrintLine("NoFriendlyFireImp %s:%d start: %.2f %.2f %.2f", getClassname(), monsterID, m_vecEnemyLKP.x, m_vecEnemyLKP.y, m_vecEnemyLKP.z);
	//Vector shootOrigin = pev->origin + Vector( 0, 0, 55 );
	TraceResult tr;

	CBaseEntity *pEnemy = m_hEnemy;

	//Vector shootTarget = ( (pEnemy->BodyTargetMod( startVec ) - pEnemy->pev->origin) + m_vecEnemyLKP );
	Vector shootTarget = pEnemy->BodyTargetMod( startVec);
	
	
	UTIL_TraceLine( startVec, shootTarget, dont_ignore_monsters, ignore_glass, ENT(pev), &tr );
	//UTIL_TraceLine( startVec, shootTarget, ignore_monsters, ignore_glass, ENT(pev), &tr );
	
	//Vector vecSrc = GetGunPosition();
	//UTIL_TraceLine( vecSrc, m_hEnemy->BodyTargetMod(vecSrc), ignore_monsters, ignore_glass, ENT(pev), &tr);


	//m_checkAttackTime = gpGlobals->time + 1;

	if(tr.pHit == NULL){
		//wait, why is this FALSE? isn't it ok if there is nothing on the other side?
		//it says "NoFriendlyFireImp" you dip!
		//easyForcePrintLine("NoFriendlyFireImp: Flag 1");
		return TRUE;
	}
	CBaseEntity* tempTarget = CBaseEntity::Instance(tr.pHit);


	int reltemp = R_NO;  //default when there isn't a hit.
	
	if(tempTarget != NULL){
		reltemp = IRelationship(tempTarget);
	}

	//easyForcePrintLine("NoFriendlyFireImp: Flag 2: %.2f %d %d %d %d",  tr.flFraction, tempTarget==NULL, (tempTarget == pEnemy), reltemp  > R_NO, reltemp == R_FR );
	//tempTarget being NULL shouldn't be possible, but this doesn't hurt.


	BOOL isInanimate = FALSE;
	if(tr.pHit != NULL){
		//CBaseEntity* test = CBaseEntity::Instance(tr.pHit->v.class);
		if( !strcmp( STRING(tr.pHit->v.classname), "func_breakable")){
			isInanimate = TRUE;  //ok to break me.
		}
	}

	//easyForcePrintLine("NoFriendlyFireImp: Hit: %s Relationship: %d isInanimate:%d", tr.pHit!=NULL?FClassname(CBaseEntity::Instance(tr.pHit)):"NULL", reltemp, isInanimate);

	if ( (!tr.fStartSolid && tr.flFraction == 1.0) || tempTarget==NULL || (tempTarget == pEnemy) || (reltemp  > R_NO || reltemp == R_FR  ) || isInanimate   ){
		//line of fire not interrupted, whatever interrupted it was the intended enemy, or whatever interrutped it is hated to some extent (Dislike to Nemesis) or feared? We can fire.
		return TRUE;
	}else{
		if(tempTarget != NULL){
			//easyForcePrintLine("NoFriendlyFireImp: FAIL! colliding ent: %s", tempTarget->getClassname());
		}else{
			//easyForcePrintLine("NoFriendlyFireImp: FAIL! colliding ent: %s", "NULL!");
		}
		//line of fire interrupted by a non-enemy with relationship <= R_NO and != R_FR? This is a friendly or non-hostile, block this attack.
		return FALSE;
	}
	//m_checkAttackTime = gpGlobals->time + 1.5;
}//END OF NoFriendlyFireImp


//MODDD - new
CBaseMonster::CBaseMonster(void){
	
	// Just a setting to keep this from being garbage from entities that never specify it.
	// Probably paranoia at this point though, think any issues with black-and-white blood are gone anyway.
	m_bloodColor = DONT_BLEED;


	timeOfDeath_activity = ACT_RESET;
	timeOfDeath_sequence = 0;

	m_lastDamageAmount = 0;
	
	forgetSmallFlinchTime = -1;
	forgetBigFlinchTime = -1;

	floatSinkSpeed = WATER_DEAD_SINKSPEED_INITIAL;
	oldWaterLevel = -1;


	hardSetFailSchedule = FALSE;
	scheduleSurvivesStateChange = FALSE;

	disableEnemyAutoNode = FALSE;

	m_fNewScheduleThisFrame = FALSE;

	forceNoDrop = FALSE;
	debugFailColor = FALSE;

	debugVectorMode = -1;
	debugVectorsSet = FALSE;

	blockDamage = FALSE;
	buddhaMode = FALSE;
	blockTimedDamage = FALSE;

	drawPathConstant = FALSE;
	drawFieldOfVisionConstant = FALSE;

	dummyAI = FALSE;

	monsterID = -1;

	signalActivityUpdate = FALSE;

	canSetAnim = TRUE;

	//MODDD TODO - we don't yet restore the queued state if the monster loses the PRONE state. Should we? this may not be very important.
	queuedMonsterState = MONSTERSTATE_NONE;
	//by default, can auto set animations from activity changes.


	// no by default.
	canDrawDebugSurface = FALSE;
	// default.
	m_flFramerateSuggestion = 1;
	fApplyTempVelocity = FALSE;

	strictNodeTolerance = FALSE;
	goalDistTolerance = 0;
	recentTimedTriggerDamage = FALSE;

	nextDirectRouteAttemptTime = -1;
	recentMoveExecuteFailureCooldown = -1;

}//CBaseMonster constructor



// All monsters use this feature by default. Just not necessarily all entities.
BOOL CBaseMonster::usesSoundSentenceSave(void){
	return TRUE;
}



/*
// done in cbase.h (CBaseEntity) instead.
BOOL CBaseMonster::isBarnacleVictimException(){
	//by default, no monster is an "exception" to what the relationship says (mostly non-aliens).
	return FALSE;
}
*/


BOOL CBaseMonster::isSizeGiant(void){
	//most monsters are not drastically larger than the player (the Gargantua is).
	return FALSE;
}

//By default, all monsters are organic. This is easier since most fall under that category.
//Just make sentries / other robotics exceptions (return FALSE to this)
BOOL CBaseMonster::isOrganic(){
	return TRUE;
}

BOOL CBaseMonster::isOrganicLogic(){
	//This means, if this monster is organic.. of course it's OrganicLogic.
	//But if it isn't organic, and this monster is using a german model replacement, we're going to treat it like it was organic.
	//Specific monsters can also just make this method return TRUE or FALSE constantly for consistent behavior for
	//dropping carcass scents at death.
	//This includes an ordinary "isOrganic" check so checking both isn't usually necessary.
	return (isOrganic() || (GermanModelOrganicLogic() && getGermanModel()!=NULL ));
}




float CBaseMonster::getBarnaclePulledTopOffset(void){
	//by default, no offset, the default offset of -8 works.  This, if non-zero, is added to that one.
	return 0;
}
float CBaseMonster::getBarnacleForwardOffset(void){
	return 0;
}
//NOTICE: "1", 100%, is the default.
float CBaseMonster::getBarnacleAnimationFactor(void){
	return 1;
}




//MODDD - just a utility method.
//It is ripped & slightly adjusted from the houndeye's "setActivity" method portion that forces an idle animation.
//...WAIT.  Is that what "SetSequenceByName" ?.

void CBaseMonster::setAnimation(char* animationName){
	setAnimation(animationName, FALSE, -1, 0);
}

void CBaseMonster::setAnimation(char* animationName, BOOL forceException){
	setAnimation(animationName, forceException, -1, 0);
}

void CBaseMonster::setAnimation(char* animationName, BOOL forceException, BOOL forceLoopsProperty){
	setAnimation(animationName, forceException, forceLoopsProperty, 0);
}

void CBaseMonster::setAnimation(char* animationName, BOOL forceException, BOOL forceLoopsProperty, int extraLogic){

	
	int iSequence = LookupSequence( animationName );
	
	//easyPrintLine("OH YOU despicable bastard FROM HELL extralogic: %d", extraLogic);
	//easyPrintLine("OH YOU despicable bastard FROM HELL %s", animationName);

	if ( forceException || (canSetAnim == TRUE) && iSequence > ACTIVITY_NOT_AVAILABLE )
	{

		BOOL pass = FALSE;
		if(extraLogic == 2){
			//can only set animation if it isn't already the desired one, or if it is finished.
			if(iSequence != pev->sequence || m_fSequenceFinished){
				pass = TRUE;
			}

		}else if(extraLogic == 1){
			//can pass if we're interrupting something, but NOT if we already have the right sequence here.
			
			//ONLY allow to change if not already correct.
			if(iSequence != pev->sequence){
				pass = TRUE;
			}

		}else if(extraLogic == 3){

			if(iSequence != pev->sequence || m_fSequenceFinished){
				pass = TRUE;
			}

		}else if(extraLogic == 4){

			if(m_fSequenceFinished){
				//only.
				pass = TRUE;
			}

		}else if(extraLogic == 5){
			//INTERRUPT ANYTHING
			pass = TRUE;

		}else{
			pass = TRUE;
		}

		if(!pass){
			return;
		}

		pev->sequence		= iSequence;	// Set to the reset anim (if it's there)
		pev->frame			= 0;		// FIX: frame counter shouldn't be reset when its the same activity as before
		//???
		//pev->framerate = 1.0f;
		if(extraLogic <= 2 && extraLogic != 1){
			//easyPrintLine("@@@@@ SEQUENCE INFO RESET %s %d", animationName, extraLogic);
			ResetSequenceInfo();
		}else{
			//easyPrintLine("@@@@@ SEQUENCE INFO SAFE %s %d", animationName, extraLogic);
			ResetSequenceInfoSafe();
		}
		usingCustomSequence = TRUE;

		SetYawSpeed();

		if(forceLoopsProperty != -1){
			m_fSequenceLoops = forceLoopsProperty;
		}
	}
	//canSetAnim == FALSE;
	
}//END OF setAnimation



void CBaseMonster::setAnimationSmart(const char* arg_animName){
	setAnimationSmart(arg_animName, -999);
}

//modeled moreso after "setSequenceByName".
void CBaseMonster::setAnimationSmart(const char* arg_animName, float arg_frameRate){

	int iSequence;

	iSequence = LookupSequence ( arg_animName );

	setAnimationSmart(iSequence, arg_frameRate);
	
}

void CBaseMonster::setAnimationSmart(int arg_animIndex, float arg_frameRate){
	
	int iSequence = arg_animIndex;

	// Set to the desired anim, or default anim if the desired is not present
	if ( iSequence > ACTIVITY_NOT_AVAILABLE )
	{
		if(m_flFramerateSuggestion != -999){
			m_flFramerateSuggestion = arg_frameRate;
		}else{
			m_flFramerateSuggestion = 1;
		}

		if ( pev->sequence != iSequence || !m_fSequenceLoops )
		{
			resetFrame();
		}

		pev->sequence		= iSequence;	// Set to the reset anim (if it's there)
		ResetSequenceInfoSafe();
		usingCustomSequence = TRUE;
		SetYawSpeed();
	}
	else
	{
		// Not available try to get default anim
		pev->sequence		= 0;	// Set to the reset anim (if it's there)
		usingCustomSequence = FALSE;
	}
	//pev->framerate = arg_frameRate;

	//NO, ditched.
	//wait for a delay, or until m_fSequenceFinished?
	//this->ChangeSchedule(slAnimationSmart);
}


void CBaseMonster::setAnimationSmartAndStop(const char* arg_animName){
	setAnimationSmartAndStop(arg_animName, -999);
}
//modeled moreso after "setSequenceByName".
void CBaseMonster::setAnimationSmartAndStop(const char* arg_animName, float arg_frameRate){

	int iSequence;

	iSequence = LookupSequence ( arg_animName );

	setAnimationSmartAndStop(iSequence, arg_frameRate);
}


//modeled moreso after "setSequenceByName"... also stops MOVEMENT, that is (if it wasn't already?)
void CBaseMonster::setAnimationSmartAndStop(int arg_animIndex, float arg_frameRate){

	int iSequence = arg_animIndex;

	// Set to the desired anim, or default anim if the desired is not present
	if ( iSequence > ACTIVITY_NOT_AVAILABLE )
	{
		if(m_flFramerateSuggestion != -999){
			m_flFramerateSuggestion = arg_frameRate;
		}else{
			m_flFramerateSuggestion = 1;
		}


		if ( pev->sequence != iSequence || !m_fSequenceLoops )
		{
			resetFrame();
		}
		
		pev->sequence		= iSequence;	// Set to the reset anim (if it's there)
		ResetSequenceInfo();
		usingCustomSequence = TRUE;
		SetYawSpeed();
	}
	else
	{
		// Not available try to get default anim
		pev->sequence		= 0;	// Set to the reset anim (if it's there)
		usingCustomSequence = FALSE;
	}
	//pev->framerate = arg_frameRate;
	//wait for a delay, or until m_fSequenceFinished?
	//this->ChangeSchedule(slAnimationSmartAndStop);
	
}




//=========================================================
// Eat - makes a monster full for a little while.
//=========================================================
void CBaseMonster::Eat ( float flFullDuration )
{
	m_flHungryTime = gpGlobals->time + flFullDuration;
}

//=========================================================
// FShouldEat - returns true if a monster is hungry.
//=========================================================
BOOL CBaseMonster::FShouldEat ( void )
{
	if ( m_flHungryTime > gpGlobals->time )
	{
		return FALSE;
	}

	return TRUE;
}

//=========================================================
// BarnacleVictimBitten - called
// by Barnacle victims when the barnacle pulls their head
// into its mouth
//=========================================================
void CBaseMonster::BarnacleVictimBitten ( entvars_t *pevBarnacle )
{
	Schedule_t	*pNewSchedule;

	pNewSchedule = GetScheduleOfType( SCHED_BARNACLE_VICTIM_CHOMP );

	if ( pNewSchedule )
	{
		ChangeSchedule( pNewSchedule );
	}

}

//=========================================================
// BarnacleVictimReleased - called by barnacle victims when
// the host barnacle is killed.
//=========================================================
void CBaseMonster::BarnacleVictimReleased ( void )
{
	m_IdealMonsterState = MONSTERSTATE_IDLE;

	pev->velocity = g_vecZero;

	//MODDD NOTE - the old movetype is step? quite an assumption, careful.
	//  Would resetting the origin be safe? turn off a onGround flag? etc.
	//this bit is new though.
	UTIL_SetOrigin(pev, pev->origin);

	pev->movetype = MOVETYPE_STEP;

	
	//MODDD - turn interpolation back on when released
	if(EASY_CVAR_GET_DEBUGONLY(barnacleGrabNoInterpolation) == 1){
		pev->effects &= ~EF_NOINTERP;
	}

}


void CBaseMonster::testMethod(void){
	easyPrintLine("test message goes here.");
}






//TODO: put something in the bis_SOUND_BAIT sound that records who / what made the noise, and who / what is allowed to be interested in it instead?
//This method assumes it is a chumtoad croak. Only CLASS_ALIEN_MONSTER and CLASS_ALIEN_PREDATOR investigate that.
//Also, must not be in a COMBAT state. IDLE and ACTIVE are ok.
BOOL CBaseMonster::interestedInBait(int arg_classID){

	BOOL test = (arg_classID == CLASS_ALIEN_MILITARY || arg_classID == CLASS_ALIEN_MONSTER || arg_classID ==CLASS_ALIEN_PREDATOR) && (m_MonsterState == MONSTERSTATE_IDLE || m_MonsterState == MONSTERSTATE_ALERT);
	//easyForcePrintLine("SO??? %d : %d --- %d", arg_classID, this->Classify(), test);
	return test;
}


SCHEDULE_TYPE CBaseMonster::getHeardBaitSoundSchedule(CSound* pSound){
	

	//Yes, this condition is copy pasted. Preserve it to the other getHeardBaitSoundSchedule that does not take a sound!
	if( interestedInBait(this->Classify() )  ){
		_getHeardBaitSoundSchedule(pSound);
	}
	
	return SCHED_NONE;

}//END OF getHeardBaitSoundSchedule


SCHEDULE_TYPE CBaseMonster::_getHeardBaitSoundSchedule(CSound* pSound){
	if ( pSound && (pSound->m_iType & bits_SOUND_BAIT) )
	{
		//MODDD TODO - shouldn't the INVESTIGATE_SOUND_BAIT schedule better handle that?
		//if we can directly see the bait and are less than X units away (distance), we'll just look at it instead.
		//MODDD TODO - how about involving this monster's abiltiy to hear? this dist of 400 should be ok though
		if( 
			(pSound->m_vecOrigin - EarPosition()).Length() < 460 &&
			FInViewCone( &pSound->m_vecOrigin ) &&
			this->CheckLocalMove(this->pev->origin, pSound->m_vecOrigin, NULL, TRUE, NULL ) == LOCALMOVE_VALID
		){
			//look at it instead.
			easyForcePrintLine("%s:ID%d LOOKIN AT THE BAIT!", this->getClassname(), this->monsterID);
			//return GetScheduleOfType( SCHED_ALERT_FACE);
			return SCHED_ALERT_FACE;
		}	
		easyForcePrintLine("%s:ID%d BAIT NOISE, FOLLOW!", this->getClassname(), this->monsterID);
		//return GetScheduleOfType( SCHED_INVESTIGATE_SOUND_BAIT );
		return SCHED_INVESTIGATE_SOUND_BAIT;
	}
	return SCHED_NONE;
}//END OF getHeardBaitSoundSchedule(...)




SCHEDULE_TYPE CBaseMonster::getHeardBaitSoundSchedule(){
	
	//MODDD - crude addition.
	//If the monsterstate is IDLE or ALERT (not focused like in COMBAT), we need to investigate the BAIT sound.
	
	//Yes, this condition is copy pasted. Preserve it to the other getHeardBaitSoundSchedule that takes a sound!
	if( interestedInBait(this->Classify()  ) ){
		
		
	    //We need to check out bait sounds!
		if ( HasConditions ( bits_COND_HEAR_SOUND ))
		{
			CSound *pSound;
			pSound = PBestSound();

			ASSERT( pSound != NULL );

			//don't bother if we don't even have the sound.
			if(pSound == NULL) //goto skipSoundInvestigate;
			return SCHED_NONE;
			
			return CBaseMonster::_getHeardBaitSoundSchedule(pSound);

		}
		
	}//END OF m_MonsterState IDLE or ALERT check.


	//skipSoundInvestigate:
	return SCHED_NONE;
}//END OF getHeardBaitSoundSchedule(...)


//=========================================================
// Listen - monsters dig through the active sound list for
// any sounds that may interest them. (smells, too!)
//=========================================================
void CBaseMonster::Listen ( void )
{
	int	iSound;
	int	iMySounds;
	float hearingSensitivity;
	CSound	*pCurrentSound;




	m_iAudibleList = SOUNDLIST_EMPTY; 
	ClearConditions(bits_COND_HEAR_SOUND | bits_COND_SMELL | bits_COND_SMELL_FOOD);
	m_afSoundTypes = 0;

	iMySounds = ISoundMask();

	if ( m_pSchedule )
	{
		//!!!WATCH THIS SPOT IF YOU ARE HAVING SOUND RELATED BUGS!
		// Make sure your schedule AND personal sound masks agree!
		
		//TEST MAJOR
		iMySounds &= m_pSchedule->iSoundMask;
	}
	


	/*
	//bits_SOUND_COMBAT
	if(FClassnameIs(pev, "monster_stukabat")){
		//different.
		//stukabats ALWAYS hear combat.
		
		iMySounds |= bits_SOUND_PLAYER;
		iMySounds |= bits_SOUND_COMBAT;
		iMySounds |= bits_SOUND_MEAT;
		iMySounds |= bits_SOUND_CARCASS;
		
	}
	*/



	if (FClassnameIs(pev, "monster_human_assault")) {
		//different.
		int xxx = 45;
	}






	//1, 4
	//2^0, 2^2

	iSound = CSoundEnt::ActiveList();

	// UNDONE: Clear these here?
	ClearConditions( bits_COND_HEAR_SOUND | bits_COND_SMELL_FOOD | bits_COND_SMELL );
	hearingSensitivity = HearingSensitivity( );

	while ( iSound != SOUNDLIST_EMPTY )
	{
		pCurrentSound = CSoundEnt::SoundPointerForIndex( iSound );
		

		//easyForcePrintLine("%s:ID%d CAN I HEAR THAT? %d", this->getClassnameShort(), this->monsterID, pCurrentSound->m_iVolume);


		//this->testMethod();

		//was this here before?  I forget.
		//if ( ( g_pSoundEnt->m_SoundPool[ iSound ].m_iType & iMySounds ) && ( g_pSoundEnt->m_SoundPool[ iSound ].m_vecOrigin - EarPosition()).Length () <= g_pSoundEnt->m_SoundPool[ iSound ].m_iVolume * hearingSensitivity ) 

		//MY NAME IS monster_stukabat:::0 5 1     29 5
		//easyPrintLine("MY NAME IS %s:::%d %d %d     %d %d", STRING(this->pev->classname), pCurrentSound==NULL, (pCurrentSound->m_iType & iMySounds), ((pCurrentSound->m_vecOrigin - EarPosition() ).Length() <= pCurrentSound->m_iVolume * hearingSensitivity), iMySounds, pCurrentSound->m_iType ) ;
		if ( pCurrentSound	&& 
			 ( pCurrentSound->m_iType & iMySounds )	&& 
			 ( ( pCurrentSound->m_vecOrigin - EarPosition() ).Length() <= pCurrentSound->m_iVolume * hearingSensitivity ) //&&
			//MODDD - new. It is possible this sound has its own other requirement, presumably set by whatever made it. Such as, chumtoad croaks only being heard by non ALIEN_PREY.
			//((pCurrentSound->canListenHandle == NULL) || (pCurrentSound->canListenHandle(this) == TRUE) )

		)
		{
			//easyForcePrintLine("%s:ID%d I HEARD IT.", this->getClassnameShort(), this->monsterID);

 			// the monster cares about this sound, and it's close enough to hear.
			//g_pSoundEnt->m_SoundPool[ iSound ].m_iNextAudible = m_iAudibleList;
			pCurrentSound->m_iNextAudible = m_iAudibleList;
			
			if ( pCurrentSound->FIsSound() )
			{
				//DebugLine_SetupPoint(pCurrentSound->m_vecOrigin + Vector(0, 0, 12), 0, 0, 255);
				// this is an audible sound.
				
				//easyForcePrintLine("%s:ID%d I SO HEARD IT IT.", this->getClassnameShort(), this->monsterID);
				SetConditions( bits_COND_HEAR_SOUND );
			}
			else
			{
				// if not a sound, must be a smell - determine if it's just a scent, or if it's a food scent
//				if ( g_pSoundEnt->m_SoundPool[ iSound ].m_iType & ( bits_SOUND_MEAT | bits_SOUND_CARCASS ) )
				if ( pCurrentSound->m_iType & ( bits_SOUND_MEAT | bits_SOUND_CARCASS ) )
				{

					if(FClassnameIs(pev, "monster_stukabat")){
						//easyPrintLine("ahhhhhhhhhhhhhhhhhhhhhh");
					}	

					// the detected scent is a food item, so set both conditions.
					// !!!BUGBUG - maybe a virtual function to determine whether or not the scent is food?
					SetConditions( bits_COND_SMELL_FOOD );
					SetConditions( bits_COND_SMELL );
				}
				else
				{
					// just a normal scent. 
					SetConditions( bits_COND_SMELL );
				}
			}

//			m_afSoundTypes |= g_pSoundEnt->m_SoundPool[ iSound ].m_iType;
			m_afSoundTypes |= pCurrentSound->m_iType;

			m_iAudibleList = iSound;
		}

//		iSound = g_pSoundEnt->m_SoundPool[ iSound ].m_iNext;
		iSound = pCurrentSound->m_iNext;
	}
	if(FClassnameIs(pev, "monster_stukabat")){


	
		//easyPrintLine("HOLY der MOTHERderER %d %d",  HasConditions( bits_COND_HEAR_SOUND ),  HasConditions( bits_COND_SMELL_FOOD ) );
	}	

	//SetConditions(bits_COND_SMELL_FOOD);

	//easyPrintLine("MY NAME IS %s:::AND I HEAR %d", STRING(this->pev->classname), HasConditions( bits_COND_HEAR_SOUND )) ;
		

}

//=========================================================
// FLSoundVolume - subtracts the volume of the given sound
// from the distance the sound source is from the caller, 
// and returns that value, which is considered to be the 'local' 
// volume of the sound. 
//=========================================================
float CBaseMonster::FLSoundVolume ( CSound *pSound )
{
	return ( pSound->m_iVolume - ( ( pSound->m_vecOrigin - pev->origin ).Length() ) );
}

//=========================================================
// FValidateHintType - tells use whether or not the monster cares
// about the type of Hint Node given
//=========================================================
BOOL CBaseMonster::FValidateHintType ( short sHint )
{
	return FALSE;
}

//=========================================================
// Look - Base class monster function to find enemies or 
// food by sight. iDistance is distance ( in units ) that the 
// monster can see.
//
// Sets the sight bits of the m_afConditions mask to indicate
// which types of entities were sighted.
// Function also sets the Looker's m_pLink 
// to the head of a link list that contains all visible ents.
// (linked via each ent's m_pLink field)
//
//=========================================================
//MODDD - why was a distance (the parameter here) an int?  why not a decimal?
void CBaseMonster::Look ( float flDistance )
{
	int iSighted = 0;

	// DON'T let visibility information from last frame sit around!
	ClearConditions(bits_COND_SEE_HATE | bits_COND_SEE_DISLIKE | bits_COND_SEE_ENEMY | bits_COND_SEE_FEAR | bits_COND_SEE_NEMESIS | bits_COND_SEE_CLIENT);


	
	if(EASY_CVAR_GET_DEBUGONLY(peaceOut) == 1){
		//like, no way, man. Let's just smoke a fat blunt and sit on the couch all day.
		return;
	}

	/*
	int chuckTesta = 6666;
	int chuckTestb = 6667;
	const char* MYCLASSNAMEDAMN = getClassname();
	const char* MYCLASSNAMEDAMNALT = STRING(pev->classname);
	BOOL doesThatThingEqualMyThing = strcmp(MYCLASSNAMEDAMN, "monster_barney");

	BOOL thingResult = FClassnameIs(pev, "monster_barney");
	if(FClassnameIs(pev, "monster_barney") == TRUE ){
		int x = 666;
		chuckTesta = 1337;
	}

	if(FClassnameIs(pev, "monster_barney") == TRUE ){
		int x = 666;
	}

	if(thingResult){
		int x = 666;
		chuckTestb = 1338;
	}




	BOOL test2 = FStrEq(STRING(pev->classname), "monster_barney");
	BOOL test3 = strcmp(STRING(pev->classname), "monster_barney");


	BOOL whatTheHay = FClassnameIs(pev, "monster_barney");
	*/



	m_pLink = NULL;

	CBaseEntity	*pSightEnt = NULL;// the current visible entity that we're dealing with



	// See no evil if prisoner is set
	if ( !FBitSet( pev->spawnflags, SF_MONSTER_PRISONER ) )
	{
		CBaseEntity *pList[100];

		Vector delta = Vector( flDistance, flDistance, flDistance );

		// Find only monsters/clients in box, NOT limited to PVS
		int count = UTIL_EntitiesInBox( pList, 100, pev->origin - delta, pev->origin + delta, FL_CLIENT|FL_MONSTER );
		for ( int i = 0; i < count; i++ )
		{
			pSightEnt = pList[i];
			// !!!temporarily only considering other monsters and clients, don't see prisoners
			

			//Good for doing breakpoints with.
			/*
			const char* sightedClassname = "derp";
			BOOL sightedEntStillAliveByAI = FALSE;
			if(pSightEnt != NULL){
				sightedClassname = pSightEnt->getClassname();
				sightedEntStillAliveByAI = IsAlive_FromAI(this);
			}

			
			const char* enemyClassname = "DERR";
			const char* myClassname = getClassname();
			if(m_hEnemy != NULL){
				enemyClassname = m_hEnemy->getClassname();
			}else{

			}
			int yyy = 666;


			int x = 666;

			BOOL isThatThingAlive = pSightEnt->IsAlive_FromAI(this);
			
			int x2 = 666;
			*/

			if ( pSightEnt != this												&& 
				 !FBitSet( pSightEnt->pev->spawnflags, SF_MONSTER_PRISONER )	&& 
				 
				 //MODDD - exluding this for now.  Still think it's alive for a tiny bit during the death animation.
				 //pSightEnt->pev->health > 0 &&
				 
				 //MODDD - new conditions. Must be alive... wait, this wasn't here before?! Without a check like this, the conditions can still be flagged
				 //and cause the AI to act as though there is some hostile monster forwards, such as stopping a monster fooled by a chumtoad from wandering off
				 //the millisecond it gets that schedule because the chumtoad playing dead is in front of it.
				 pSightEnt->IsAlive_FromAI(this)

				 )
			{

				
				//if(FClassnameIs(pev, "monster_chumtoad"))easyForcePrintLine("DOES IT PASS??? %s %d %d %d %d", pSightEnt->getClassnameShort(), (IRelationship( pSightEnt ) != R_NO), FInViewCone( pSightEnt ), (!FBitSet( pSightEnt->pev->flags, FL_NOTARGET )), FVisible( pSightEnt )   );

				// the looker will want to consider this entity
				// don't check anything else about an entity that can't be seen, or an entity that you don't care about.
				
				//OKAY.  You are not too bright are ya.
				/*
				int daRelation = IRelationship(pSightEnt);
				BOOL isInDaViewcone = FInViewCone(pSightEnt);
				BOOL isVisaba = FVisible(pSightEnt);
				BOOL isDaFlagSet = FBitSet(pSightEnt->pev->flags, FL_NOTARGET);
				int xxx = 666;
				*/
				
				
				if ( IRelationship( pSightEnt ) != R_NO && FInViewCone( pSightEnt ) && !FBitSet( pSightEnt->pev->flags, FL_NOTARGET ) && FVisible( pSightEnt ) )
				{
					if(EASY_CVAR_GET_DEBUGONLY(crazyMonsterPrintouts)){
						easyPrintLine("FLAGGER 64 1");
					}
					if ( pSightEnt->IsPlayer() )
					{
						if ( pev->spawnflags & SF_MONSTER_WAIT_TILL_SEEN )
						{
							CBaseMonster *pClient;

							pClient = pSightEnt->MyMonsterPointer();
							// don't link this client in the list if the monster is wait till seen and the player isn't facing the monster
							if ( pSightEnt && !pClient->FInViewCone( this ) )
							{
								// we're not in the player's view cone. 
								continue;
							}
							else
							{
								// player sees us, become normal now.
								pev->spawnflags &= ~SF_MONSTER_WAIT_TILL_SEEN;
							}
						}

						// if we see a client, remember that (mostly for scripted AI)
						iSighted |= bits_COND_SEE_CLIENT;
					}
					if(EASY_CVAR_GET_DEBUGONLY(crazyMonsterPrintouts)){
						easyPrintLine("FLAGGER 64 2");
					}
					pSightEnt->m_pLink = m_pLink;
					m_pLink = pSightEnt;
					
					if(EASY_CVAR_GET_DEBUGONLY(crazyMonsterPrintouts)){
						easyPrintLine("FLAGGER 64 3::? (%d) %s %s", (pSightEnt == m_hEnemy), FClassname(pSightEnt), FClassname(m_hEnemy)  );
					}


					if ( pSightEnt == m_hEnemy )
					{
						// we know this ent is visible, so if it also happens to be our enemy, store that now.
						iSighted |= bits_COND_SEE_ENEMY;
					}

					//if(FClassnameIs(pev, "monster_chumtoad"))easyForcePrintLine("THE ENEMEH???");

					// don't add the Enemy's relationship to the conditions. We only want to worry about conditions when
					// we see monsters other than the Enemy.

					//MODDD TODO MAJOR - would it be a good idea to piggyback off of the bits_COND_SEE_NEMESIS for 
					//interrupting schedules if bait is sighted (R_BA)?
					//or should that be it's own bits_COND_SEE_BAIT, would need its own bit and need to be added to several default (and monster-specific schedules).
					switch ( IRelationship ( pSightEnt ) )
					{
					
					//MODDD - trying out treating R_BA as NEMESIS.  See if that works out.
					case R_BA:
						iSighted |= bits_COND_SEE_NEMESIS;		
					break;

					case R_NM:
						iSighted |= bits_COND_SEE_NEMESIS;		
						break;
					case R_HT:		
						iSighted |= bits_COND_SEE_HATE;		
						break;
					case R_DL:
						iSighted |= bits_COND_SEE_DISLIKE;
						break;
					case R_FR:
						iSighted |= bits_COND_SEE_FEAR;
						break;
					case R_AL:
						//nothing special about allies apparently.
						break;
					default:
						ALERT ( at_aiconsole, "%s can't assess %s\n", STRING(pev->classname), STRING(pSightEnt->pev->classname ) );
						break;
					}

				}
			}
		}
	}
	
	SetConditions( iSighted );
}

//=========================================================
// ISoundMask - returns a bit mask indicating which types
// of sounds this monster regards. In the base class implementation,
// monsters care about all sounds, but no scents.
// MODDD NOTE - and not the one fired by the anticipated landing spot of
//              grenades for knowing to flee from that, bits_SOUND_DANGER,
//              apparently.
//              Not that dumb aliens are supposed to react to that anyways
//              but include it as needed for smarter things.
//=========================================================
int CBaseMonster::ISoundMask ( void )
{
	return	bits_SOUND_WORLD	|
			bits_SOUND_COMBAT	|
			bits_SOUND_PLAYER	|
			//MODDD - new
			bits_SOUND_BAIT;
}

//=========================================================
// PBestSound - returns a pointer to the sound the monster 
// should react to. Right now responds only to nearest sound.
//=========================================================
CSound* CBaseMonster::PBestSound ( void )
{	
	int iThisSound; 
	int iBestSound = -1;
	float flBestDist = 8192;// so first nearby sound will become best so far.
	float flDist;
	CSound *pSound;

	iThisSound = m_iAudibleList; 

	if ( iThisSound == SOUNDLIST_EMPTY )
	{
		//MODDD - Enough already, stop spamming this...
		//ALERT ( at_aiconsole, "ERROR! monster %s has no audible sounds!\n", STRING(pev->classname) );
#if _DEBUG
		//ALERT( at_error, "NULL Return from PBestSound\n" );
#endif
		return NULL;
	}

	while ( iThisSound != SOUNDLIST_EMPTY )
	{
		pSound = CSoundEnt::SoundPointerForIndex( iThisSound );

		if ( pSound && pSound->FIsSound() )
		{
			flDist = ( pSound->m_vecOrigin - EarPosition()).Length();

			if ( flDist < flBestDist )
			{
				iBestSound = iThisSound;
				flBestDist = flDist;
			}
		}

		iThisSound = pSound->m_iNextAudible;
	}
	if ( iBestSound >= 0 )
	{
		pSound = CSoundEnt::SoundPointerForIndex( iBestSound );
		return pSound;
	}
#if _DEBUG
	ALERT( at_error, "NULL Return from PBestSound\n" );
#endif
	return NULL;
}

//=========================================================
// PBestScent - returns a pointer to the scent the monster 
// should react to. Right now responds only to nearest scent
//=========================================================
CSound* CBaseMonster::PBestScent ( void )
{	
	int iThisScent; 
	int iBestScent = -1;
	float flBestDist = 8192;// so first nearby smell will become best so far.
	float flDist;
	CSound *pSound;

	iThisScent = m_iAudibleList;// smells are in the sound list.

	if ( iThisScent == SOUNDLIST_EMPTY )
	{
		ALERT ( at_aiconsole, "ERROR! PBestScent() has empty soundlist!\n" );
#if _DEBUG
		ALERT( at_error, "NULL Return from PBestSound\n" );
#endif
		return NULL;
	}

	while ( iThisScent != SOUNDLIST_EMPTY )
	{
		pSound = CSoundEnt::SoundPointerForIndex( iThisScent );

		if ( pSound->FIsScent() )
		{
			flDist = ( pSound->m_vecOrigin - pev->origin ).Length();

			if ( flDist < flBestDist )
			{
				iBestScent = iThisScent;
				flBestDist = flDist;
			}
		}

		iThisScent = pSound->m_iNextAudible;
	}
	if ( iBestScent >= 0 )
	{
		pSound = CSoundEnt::SoundPointerForIndex( iBestScent );

		return pSound;
	}
#if _DEBUG
	ALERT( at_error, "NULL Return from PBestScent\n" );
#endif
	return NULL;
}







//MODDD - by default, most monsters will do the usual "takeDamage" script's reaction to investigating the source of damage.
//Note that even this method getting called requires "EASY_CVAR_GET_DEBUGONLY(bulletholeAlertRange)" to be above 0 (the distance the bullet-hit sound triggers enemies by calling this)
void CBaseMonster::heardBulletHit(entvars_t* pevShooter){

	//I heard it.
	entvars_t* pevListener = pev;
	CBaseMonster* monListener = this;


	// react to the sound
	if ( (pevListener->flags & FL_MONSTER) && !FNullEnt(pevShooter) )
	{
		if ( pevShooter->flags & (FL_MONSTER | FL_CLIENT) )
		{
			// only if the attacker was a monster or client!
			
			// enemy's last known position is somewhere down the vector that the attack came from.
			if (pevShooter)
			{
				if (monListener->m_hEnemy == NULL || pevShooter == monListener->m_hEnemy->pev || !monListener->HasConditions(bits_COND_SEE_ENEMY))
				{
					//MODDD - sent the ent itself for custom handling
					monListener->setEnemyLKP(pevShooter);
				}
			}
			else
			{
				//MODDD - sent the ent itself for custom handling, and the extra offset
				//monListener->setEnemyLKP(monListener->pev->origin + ( g_vecAttackDir * 64 )); 
				//monListener->setEnemyLKP(monListener->pev->origin, monListener->pev->mins.z, ( g_vecAttackDir * 64 )); 
				monListener->setEnemyLKP(monListener, ( g_vecAttackDir * 64 )); 
			}

			//want to face your LKP, yes?
			monListener->MakeIdealYaw( monListener->m_vecEnemyLKP );

			/*
			if ( flDamage > 0 )
			{
				SetConditions(bits_COND_LIGHT_DAMAGE);
			}

			if ( flDamage >= 20 )
			{
				SetConditions(bits_COND_HEAVY_DAMAGE);
			}
			*/

			//This makes most NPCs investigate their "m_vecEnemyLKP"'s, I suppose. Maybe flinch in surprise.
			monListener->SetConditions(bits_COND_LIGHT_DAMAGE);

		}
	}


}//END OF heardBulletHit(...)



void CBaseMonster::wanderAway(const Vector& toWalkAwayFrom){

	//you are going to walk somewhere away from the given location.
	//NOTICE - whoever called me already did the MONSTERSTATE checks to see we aren't telling something preoccupied to suddenly change its mind in
	//whatever it's doing.  Only those that are IDLE or ALERT.

	/*
	//STOP task script from schedule.cpp for reference
	case TASK_STOP_MOVING:
		{
			if ( m_IdealActivity == m_movementActivity )
			{
				m_IdealActivity = GetStoppedActivity();
			}
	*/

	//TODO: call "taskFail" and createa a delay until this monster wanders away. Break the delay on doing absolutely anything.
	//Peraps changing to a "waitForWanderDelay" schedule, interruptible by practically anything, would work?


	easyForcePrintLine("YOU, %s:%d, GOT TOLD TO WANDER AWAY SO YOU BETTER LISTEN", this->getClassname(), monsterID);
	//if(this->m_movementActivity == ACT_IDLE){
		//m_movementActivity

		//TODO: specify walking away from "toWalkAwayFrom" ?

		this->ChangeSchedule(  this->GetScheduleOfType(SCHED_TAKE_COVER_FROM_ORIGIN_WALK)  );
	//}


}//END OF wanderAway()






//MODDD - NEW.  Test the damage given.  If it would kill this monster/player, don't allow it to.
// Set the damage so that it leaves the health at most 1 instead, but don't force decimals between
// 0 and 1 to 1, that's slight healing (kinda weird to happen).  
// Like at 0.3 out of 50 health, getting negative 0.7 damage would make that 1 out of 50 health.
// Weird.
float CBaseMonster::TimedDamageBuddhaFilter(float dmgIntent) {
	if (dmgIntent >= pev->health && gSkillData.tdmg_buddha == 1 && !recentTimedTriggerDamage) {
		dmgIntent = pev->health - 1;
		if (dmgIntent < 0) dmgIntent = 0;  // no healing from this!
	}

	return dmgIntent;
}


// at the end of a frame, if a monster has 1 health and buddha mode, cancel the timed damage.
void CBaseMonster::TimedDamagePostBuddhaCheck(void) {
	if (pev->health <= 1 && gSkillData.tdmg_buddha == 1 && !recentTimedTriggerDamage) {
		// ok, don't allow another frame.
		attemptResetTimedDamage(TRUE);
	}

	recentTimedTriggerDamage = FALSE;
}




//Easy way to convert new damage types without adjusting existing constants.
//That may be okay, but I'm not taking risks just yet.
int CBaseMonster::convert_itbd_to_damage(int i){
	
	if(i < itbd_BITMASK2_FIRST){
		//The old way works fine for existing damage types.

		//DAMNIT PAST ME, comments.
		//These damage types are represented per value of i:
		//0 : DMG_PARALYZE		(1 << 15)	// slows affected creature down
		//1 : DMG_NERVEGAS		(1 << 16)	// nerve toxins, very bad
		//2 : DMG_POISON			(1 << 17)	// blood poisioning
		//3 : DMG_RADIATION		(1 << 18)	// radiation exposure
		//4 : DMG_DROWNRECOVER	(1 << 19)	// drowning recovery
		//5 : DMG_ACID			(1 << 20)	// toxic chemicals or acid burns
		//6 : DMG_SLOWBURN		(1 << 21)	// in an oven
		//7 : DMG_SLOWFREEZE		(1 << 22)	// in a subzero freezer

		return (DMG_PARALYZE << i);
	}else{
		// start at 8 = DMG_BLEEDING.
		//return (DMG_PARALYZE << (i - 8));
		// ...no, unless all future timed damages are placed so they occur
		// one-after-the-other like for 'i < itbd_BITMASK2_FIRST' above.
		// Only one yet so hard to say.
		switch(i){
		case itbd_Bleeding:
			return DMG_BLEEDING;
		break;
		}//END OF switch(i)
	}

	easyPrintLine("ERROR: failed to convert timed damage itbd #%d to damage type!", i);
	//throw error! problem!
	return -1;
}//END OF convert_itbd_to_damage


void CBaseMonster::removeTimedDamage(int arg_type, int* m_bitsDamageTypeRef) {

	//if (m_rgbTimeBasedDamage[arg_type] != 255) {
	m_rgbTimeBasedDamage[arg_type] = 0;
	//}
	//MODDD
	m_rgbTimeBasedFirstFrame[arg_type] = TRUE;
	//MODDD
	// if we're done, clear damage bits
	//m_bitsDamageType &= ~(DMG_PARALYZE << i);	
	(*m_bitsDamageTypeRef) &= ~(convert_itbd_to_damage(arg_type));
}//END OF removeTimedDamage


// Similar to removeTimedDamage, but only called on removing a timed damage very early on.
// The player has special behavior for this.
void CBaseMonster::removeTimedDamageImmediate(int arg_type, int* m_bitsDamageTypeRef, BYTE bDuration) {
	removeTimedDamage(arg_type, m_bitsDamageTypeRef);
}




// Get the duration of a type of timed damage, now a separate method.  Mysterious that it wasn't always.
// Also, "-1" is a wildcard value to signal that we should pretend like that damage type doesn't exist,
// not even the icon for a moment.
// Full support only available for bleeding for now, it takes a few other places (or at least Player's TakeDamage)
// that would need to check for the '-1' duration in order to not play FVox lines too.
BYTE CBaseMonster::parse_itbd_duration(int i) {
	switch (i)
	{
	case itbd_Paralyze:
		return ((gSkillData.tdmg_paralyze_duration >= 0) ? ((BYTE)gSkillData.tdmg_paralyze_duration) : 255);
	case itbd_NerveGas:
		return ((gSkillData.tdmg_nervegas_duration >= 0) ? ((BYTE)gSkillData.tdmg_nervegas_duration) : 255);
	case itbd_Poison: {
		if (!(m_bitsDamageTypeMod & DMG_POISONHALF)) {
			// normal duration
			return ((gSkillData.tdmg_poison_duration >= 0) ? ((BYTE)gSkillData.tdmg_poison_duration) : 255);
		}
		else {
			//has DMG_POISONHALF?  Clear it, apply that (half duration returned)
			m_bitsDamageTypeMod &= ~DMG_POISONHALF;
			return ((gSkillData.tdmg_poison_duration >= 0) ? ((BYTE)(gSkillData.tdmg_poison_duration/2)) : 255);
		}
	}
	case itbd_Radiation:
		return ((gSkillData.tdmg_radiation_duration >= 0) ? ((BYTE)gSkillData.tdmg_radiation_duration) : 255);
	//case itbd_DrownRecover:    no, for the player only!
	// ...
	//return 4;	// get up to 5*10 = 50 points back
	case itbd_Acid:
		return ((gSkillData.tdmg_acid_duration >= 0) ? ((BYTE)gSkillData.tdmg_acid_duration) : 255);
	case itbd_SlowBurn:
		return ((gSkillData.tdmg_slowburn_duration >= 0) ? ((BYTE)gSkillData.tdmg_slowburn_duration) : 255);
	case itbd_SlowFreeze:
		return ((gSkillData.tdmg_slowfreeze_duration >= 0) ? ((BYTE)gSkillData.tdmg_slowfreeze_duration) : 255);
	case itbd_Bleeding:
		return ((gSkillData.tdmg_bleeding_duration >= 0) ? ((BYTE)gSkillData.tdmg_bleeding_duration) : 255);
	default:
		return 255;  //???
	}
}//parse_itbd_duration



void CBaseMonster::parse_itbd(int i) {
	int damageType = 0;
	//
	//if(timedDamageIgnoresArmorMem == 1){
	//	//timed damage ignores armor.
	//	damageType = DMG_FALL;
	//}else{
	//	//timed damage hits armor first.
	//	damageType = DMG_GENERIC;
	//}
	//
	// no, can't just do that.

	// instead, a damgeType being "DMG_TIMEDEFFECT" means,
	// it is just generic, but use this for telling whether to
	// apply the armor-bypass or not without the sideeffect of what-
	// ever happens to DMG_FALL.
	damageType = DMG_TIMEDEFFECT;


	// NOTE: should these ignore armor or not?  I feel like they kind of should.
	// If so, make the damage type (DMG_GENERIC) become "DMG_FALL" instead (known to ignore armor).
	// ...this might have been handled since by the new dmg type DMG_TIMEDEFFECTIGNORE

	switch (i){
	case itbd_Paralyze:
		// UNDONE - flag movement as half-speed
	break;
	case itbd_NerveGas:
		//MODDD - comment undone.
		TakeDamage(pev, pev, TimedDamageBuddhaFilter(NERVEGAS_DAMAGE), 0, damageType);
	break;
	case itbd_Poison:
		TakeDamage(pev, pev, TimedDamageBuddhaFilter(POISON_DAMAGE), 0, damageType | DMG_TIMEDEFFECTIGNORE);
	break;
	case itbd_Radiation:
		//MODDD - comment on "TakeDamage" undone.
		TakeDamage(pev, pev, TimedDamageBuddhaFilter(RADIATION_DAMAGE), 0, damageType | DMG_TIMEDEFFECTIGNORE);
	break;
	//case itbd_DrownRecover:
	// ...
	//break;
	case itbd_Acid:
		//MODDD - comment undone.
		TakeDamage(pev, pev, TimedDamageBuddhaFilter(ACID_DAMAGE), 0, damageType);
		//MODDD - see above.
	break;
	case itbd_SlowBurn:
		//MODDD - comment undone.
		TakeDamage(pev, pev, TimedDamageBuddhaFilter(SLOWBURN_DAMAGE), 0, damageType);
		//MODDD - see above.
	break;
	case itbd_SlowFreeze:
		//easyPrintLine("DO YOU EVER TAKE FREEZE DAMAGE?");
		//this won't be called, as the map's called freeze effect never starts like this.
		//MODDD - comment undone.
		TakeDamage(pev, pev, TimedDamageBuddhaFilter(SLOWFREEZE_DAMAGE), 0, damageType);
		//MODDD - see above.
	break;
	//MODDD - new.
	case itbd_Bleeding:
		// this will always ignore the armor (hence DMG_TIMEDEFFECT).
		TakeDamage(pev, pev, TimedDamageBuddhaFilter(BLEEDING_DAMAGE), 0, damageType | DMG_TIMEDEFFECTIGNORE);

		UTIL_MakeAimVectors(pev->angles);
		//pev->origin + pev->view_ofs
		//BodyTargetMod(g_vecZero)
		// BLEEDING_DAMAGE

		if(CanMakeBloodParticles()){
			UTIL_SpawnBlood(
				pev->origin + pev->view_ofs + gpGlobals->v_forward * RANDOM_FLOAT(9, 13) + gpGlobals->v_right * RANDOM_FLOAT(-5, 5) + gpGlobals->v_up * RANDOM_FLOAT(4, 7),
				BloodColor(),
				RANDOM_LONG(8, 15)
			);
		}
	break;
	default:
		// ???
	break;
	}//switch(i)

}//END OF parse_itbd


// If this is not the first frame we're taking this damage type,
void CBaseMonster::timedDamage_nonFirstFrame(int i, int* m_bitsDamageTypeRef) {

	if (g_iSkillLevel == 3 && EASY_CVAR_GET_DEBUGONLY(timedDamageEndlessOnHard) == 1) {
		// Hard mode is on, and "timedDamageEndlessOnHard" is on...
		// Do NOT decrement non-curable durations.
		// However, still decrement only ONCE on curables to satisfy the one-second-passing rule for canisters to work.
		if(i == itbd_NerveGas || i == itbd_Poison || i == itbd_Radiation || i == itbd_Bleeding) {
			//DO NOTHING.  Only the appropriate cure can fix it.
		}
		else{
			// ordinary behavior then.
			if (m_rgbTimeBasedDamage[i] > 0) {
				m_rgbTimeBasedDamage[i]--;
			}
			//else {
			//	m_rgbTimeBasedDamage[i] = 255;
			//}
		}

	}
	else {
		//work as normal.  Decrement durations.
		if (m_rgbTimeBasedDamage[i] > 0) {
			m_rgbTimeBasedDamage[i]--;
		}
		//else {
		//	m_rgbTimeBasedDamage[i] = 255;
		//}
	}

	// decrement damage duration, detect when done.
	//MODDD - change to how that works.
	//if (!m_rgbTimeBasedDamage[i] || --m_rgbTimeBasedDamage[i] == 0)
	if (m_rgbTimeBasedDamage[i] <= 0) //|| m_rgbTimeBasedDamage[i] == 255)
	{
		removeTimedDamage(i, m_bitsDamageTypeRef);
	}

}



//MODDD - checkpoint.
void CBaseMonster::CheckTimeBasedDamage(void) 
{
	static float gtbdPrev = 0.0;
	int i;

	// no timed damage for 
	if (pev->health <= 0 || pev->deadflag != DEAD_NO) {
		return;
	}


	//if (IsPlayer()) {
	//	easyForcePrintLine("TIME TILL DAMAGE CHECK: %.2f abs: %.2f", (gpGlobals->time - m_tbdPrev), fabs(gpGlobals->time - m_tbdPrev));
	//}

	// only check for time based damage approx. every 2 seconds
	//MODDD - why the 'abs' that was here?  What did that add?
	// in fact that's kinda dangerous cuz uh...   "abs" can mean integer-abs, makes whatever it gets a whole number.
	// Use fabs instead anyway!
	//if (fabs(gpGlobals->time - m_tbdPrev) < 2.0) {
	if ((gpGlobals->time - m_tbdPrev) < 2.0) {
		return;
	}

	//MODDD - check other too!
	//if (!(m_bitsDamageType & DMG_TIMEBASED))
	if (!(m_bitsDamageType & DMG_TIMEBASED) && !(m_bitsDamageTypeMod & (DMG_TIMEBASEDMOD))) {
		return;
	}

	m_tbdPrev = gpGlobals->time;
	
	for (i = 0; i < itbd_COUNT; i++)
	{
		int* m_bitsDamageTypeRef = 0;
		if(i < itbd_BITMASK2_FIRST){
			//use the old bitmask.
			m_bitsDamageTypeRef = &m_bitsDamageType;
		}else{
			//use the new bitmask.
			m_bitsDamageTypeRef = &m_bitsDamageTypeMod;
		}

		int damageBit = convert_itbd_to_damage(i);

		if (damageBit == -1) {
			easyForcePrintLine("CRITICAL ERROR: MONSTER TIMED DAMAGE BOOBOO: %d", i);
			return;
		}

		// make sure bit is set for damage type
		//if (m_bitsDamageType & (DMG_PARALYZE << i))
		if ((*m_bitsDamageTypeRef) & (damageBit))
		{
			/*
			// OLD WAY
			m_rgbTimeBasedFirstFrame[i] = FALSE;

			parse_itbd(i, bDuration);

			if (m_rgbTimeBasedDamage[i] > 0)
			{
				timedDamage_nonFirstFrame(i, m_bitsDamageTypeRef);
			}
			else{
				// first time taking this damage type - init damage duration
				//MODDD - probably a bit redundant, but ah well.
				m_rgbTimeBasedFirstFrame[i] = TRUE;
				m_rgbTimeBasedDamage[i] = bDuration;
			}
			*/

			/*
			if (m_rgbTimeBasedFirstFrame[i] == FALSE) {
				// not the first frame?  Typical checks
				timedDamage_nonFirstFrame(i, m_bitsDamageTypeRef);
			}

			if (m_rgbTimeBasedDamage[i] >= 0 && m_rgbTimeBasedDamage[i] != 255) {
				parse_itbd(i, bDuration);
			}
			*/


			if(m_rgbTimeBasedFirstFrame[i] == FALSE){
				// not the first frame?  damage and checks
				parse_itbd(i);
				timedDamage_nonFirstFrame(i, m_bitsDamageTypeRef);
			}else{
				// first time taking this damage type - init damage duration
				// only if the duration received is above 0.  Otherwise there won't be a 'next' time.
				// 255 is the stand-in for "-1", a special choice to mean this damage type should not appear in any form, not even
				// the small damage icon that disappears soon without any timed damage.

				setTimedDamageDuration(i, m_bitsDamageTypeRef);
			}//END OF firstFrame check
		}
	}//END OF loop through all damage types


	TimedDamagePostBuddhaCheck();

}//END OF CheckTimeBasedDamage



void CBaseMonster::setTimedDamageDuration(int i, int* m_bitsDamageTypeRef) {
	BYTE bDuration = parse_itbd_duration(i);

	if (bDuration > 0 && bDuration != 255) {
		m_rgbTimeBasedDamage[i] = bDuration;
		m_rgbTimeBasedFirstFrame[i] = FALSE;
	}
	else {
		// don't come back!
		removeTimedDamageImmediate(i, m_bitsDamageTypeRef, bDuration);
	}//END OF duration checks
}


void CBaseMonster::attemptResetTimedDamage(BOOL forceReset) {

	if (forceReset || m_bitsDamageType != 0 || m_bitsDamageTypeMod != 0) {
		for (int i = 0; i < itbd_COUNT; i++) {
			m_rgbTimeBasedDamage[i] = 0;
			m_rgbTimeBasedFirstFrame[i] = TRUE;
		}

		m_bitsDamageType = 0;
		m_bitsDamageTypeMod = 0;
	}

}


// On a TakeDamage call, need to see what damages are timed and apply them.
// Also, note that unhandled m_bitsDamageTypes (either bitmask) don't simply 'go away' without
// something clearing them.  Use them anywhere else to see if recently-inflicted damage has some
// type (like DMG_BULLET, DMG_SLASH, etc.).  Clear a looked-for type when done or else irrelevant
// attacks after that will see it still in memory.
void CBaseMonster::applyNewTimedDamage(int arg_bitsDamageType, int arg_bitsDamageTypeMod) {

	for (int i = 0; i < itbd_COUNT; i++) {
		//MODDD
		//if (bitsDamageType & (DMG_PARALYZE << i))

		int* m_bitsDamageTypeRef = 0;
		if (i < itbd_BITMASK2_FIRST) {
			// use the old bitmask.
			m_bitsDamageTypeRef = &arg_bitsDamageType;
		}else {
			// use the new bitmask.
			m_bitsDamageTypeRef = &arg_bitsDamageTypeMod;
		}

		//
		if ((*m_bitsDamageTypeRef) & (convert_itbd_to_damage(i))) {
			
			// New damage being requested?  ok.
			// No need to reset everything about it, if this damage has
			// already been applied it will be reset and continue ticking down.
			// This way, continuously applied timed damage (such as from sitting
			// in a trigger_hurt that gives radiation damage) will still deal damage.
			// Sitting in a 0-dmg trigger no longer stops the timed damage from happening.

			//m_rgbTimeBasedDamage[i] = 0;
			//m_rgbTimeBasedFirstFrame[i] = TRUE;

			if (m_rgbTimeBasedFirstFrame[i] == FALSE) {
				// reset my duration now then.
				BYTE bDuration = parse_itbd_duration(i);
				int damageBit = convert_itbd_to_damage(i);

				if (bDuration > 0 && bDuration != 255) {
					m_rgbTimeBasedDamage[i] = bDuration;
					m_rgbTimeBasedFirstFrame[i] = FALSE;
				}
				else {
					// don't come back!
					removeTimedDamageImmediate(i, m_bitsDamageTypeRef, bDuration);
				}//END OF duration checks
			}

		}

	}//END OF for through itbd's
}//applyNewTimedDamage




BOOL CBaseMonster::forceIdleFrameReset(void){
	return FALSE;
}

void CBaseMonster::onNewRouteNode(void){
	//nothing by default
}

BOOL CBaseMonster::usesAdvancedAnimSystem(void){
	return FALSE;
}



void CBaseMonster::CallMonsterThink(void){
	// NEW, check to see if m_hEnemy is referring to a monster with invalid private memory.
	// Disconnected players will leave it this way now.
	//MODDD - SERIOUS TODO.
	// Anything that could refer to a player should do this, any other edicts, etc.
	// Doing it here for TargetEnt too out of paranoia.
	// Changed how looking for nullity in EHandles works, I think the usual way everywhere should be fine now.
	// If that has side effects undo that and restore this.   (cbase.cpp, 'MODDD - CRITICAL' )
	/*
	if (m_hEnemy != NULL && m_hEnemy.GetEntity() == NULL) {
		m_hEnemy = NULL;
	}
	if (m_hTargetEnt != NULL && m_hTargetEnt.GetEntity() == NULL) {
		m_hTargetEnt = NULL;
	}
	*/
	////////////////////////////////////////////////////////////
	// and all this used to do.
	this->MonsterThink();
}


//=========================================================
// Monster Think - calls out to core AI functions and handles this
// monster's specific animation events
//=========================================================
void CBaseMonster::MonsterThink ( void )
{
	///pev->renderfx |= NOMUZZLEFLASH;
	pev->nextthink = gpGlobals->time + 0.1;// keep monster thinking.
	

	if(dummyAI){
		// no thinking
		return;
	}


	//MODDD - Instead of sending this interval 500 different places, keep it here too for now.
	// For this think-method these two values are identical, m_flInterval is just available outside this method for CBaseMonster.
	// And it can be determined separately from StudioFrameAdvance to make the interval available for logic that runs before
	// the rest of StudioFrameAdvance runs.
	// Moved to CBaseAnimating and done inside any DetermineInterval call instead now!
	//UNFORTUNATELY, either setting the pev->animtime (not done in the _SAFE varient) or using this interval
	// instead of one determined right before StudioFrameAdvance (like it was in retail, early in the method)
	// can cause issues with anim events near/at the start of a sequence like the barney's firing animation.
	// It is still fine to do this to have a somewhat accurate m_flInterval value, but best to do retail's
	// way of handling StudioFrameAdvance because of how sensitive it is.
	//float flInterval = DetermineInterval_SAFE();
	DetermineInterval_SAFE();
	//m_flInterval = flInterval;



	// you can do the flinches again!
	if(forgetSmallFlinchTime != -1 && gpGlobals->time >= forgetSmallFlinchTime){
		forgetSmallFlinchTime = -1;
		this->Forget(bits_MEMORY_FLINCHED);
	}
	if(forgetBigFlinchTime != -1 && gpGlobals->time >= forgetBigFlinchTime){
		forgetBigFlinchTime = -1;
		// oh, there isn't a separate BIG_FLINCHED.
		//this->Forget(bits_MEMORY_BIG_FLINCHED);
		this->Forget(bits_MEMORY_FLINCHED);
	}


	//TODO - test frame. is 255 or 256 the last??
	/*
	pev->frame = EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(testVar);
	pev->framerate = 0;

	return;
	*/

	//easyForcePrintLine("FRAMEA:%.2f seq:%d loop:%d fin:%d", this->pev->frame, pev->sequence, m_fSequenceLoops, m_fSequenceFinished);

	
	/*
	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(testVar) == 1){
		//BLOCKER
		return;
	}

	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(testVar) == 2){
		this->m_fSequenceLoops = FALSE;
	}
	*/
	
	/*
	//pev->renderfx |= STOPINTR;

	
	pev->sequence = EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(testVar);
	pev->framerate = 1;

	return;
	*/

	
	if(this->drawFieldOfVisionConstant == TRUE){
		DrawFieldOfVision();
	}
	
	if(fApplyTempVelocity){
		fApplyTempVelocity = FALSE;
		pev->velocity = velocityApplyTemp;
	}


	int clrR = 0;
	int clrG = 0;
	int clrB = 0;

	if(EASY_CVAR_GET_DEBUGONLY(pathfindTopRampFixDraw)==1 && debugVectorsSet){
		
		if(debugVectorMode == 0){

			if(debugVectorPrePass){
				clrR=0;clrG=255;clrB=0;
				if(debugFailColor){clrR=255;clrG=0;clrB=0;}
				UTIL_drawLineFrame(debugVector1, debugVector2, 12, clrR, clrG, clrB);
			}else{
				clrR=255;clrG=0;clrB=0;
				if(debugFailColor){clrR=255;clrG=0;clrB=0;}
				UTIL_drawLineFrame(debugVector1, debugVector2, 12, clrR, clrG, clrB);
			}
			clrR=0;clrG=255;clrB=0;
			if(debugFailColor){clrR=255;clrG=0;clrB=0;}
			UTIL_drawLineFrame(debugVector2, debugVector3, 12, clrR, clrG, clrB);
		

		}else if(debugVectorMode == 1){
			
			clrR=0;clrG=255;clrB=0;
			if(debugFailColor){clrR=255;clrG=0;clrB=0;}
			UTIL_drawLineFrame(debugVector1, debugVector2, 12, clrR, clrG, clrB);
			
			clrR=0;clrG=0;clrB=255;
			if(debugFailColor){clrR=255;clrG=0;clrB=0;}
			UTIL_drawLineFrame(debugVector2, debugVector3, 12, clrR, clrG, clrB);
			
			clrR=0;clrG=255;clrB=0;
			if(debugFailColor){clrR=255;clrG=0;clrB=0;}
			UTIL_drawLineFrame(debugVector3, debugVector4, 12, clrR, clrG, clrB);


		}//END OF debugVectorMode checks
		else if(debugVectorMode == 2){
			
			clrR=0;clrG=255;clrB=0;
			if(debugFailColor){clrR=255;clrG=0;clrB=0;}
			UTIL_drawLineFrame(debugVector1, debugVector2, 12, clrR, clrG, clrB);

		}

	}//END OF draw debug vector check (path finding ramp fix)
	

	
	if(EASY_CVAR_GET_DEBUGONLY(animationPrintouts) == 1 && monsterID >= -1)easyForcePrintLine("%s:%d Anim info A? frame:%.2f done:%d", getClassname(), monsterID, pev->frame, m_fSequenceFinished);
	
	//if (!terminated) {
		RunAI();
	//}


	if(EASY_CVAR_GET_DEBUGONLY(animationPrintouts) == 1 && monsterID >= -1)easyForcePrintLine("%s:%d Anim info B frame:%.2f done:%d", getClassname(), monsterID, pev->frame, m_fSequenceFinished);
	

	// NOTICE: Player calls CheckTimeBasedDamage in its 'PreThink' method instead, player never calls MonsterThink.
	// Player implements some things about timed damage differently to work as expected there.
	if(EASY_CVAR_GET_DEBUGONLY(timedDamageAffectsMonsters) == 1){
		CheckTimeBasedDamage();
	}
	
	//MODDD - workings of StudioFrameAdvance changed.  Find the flInterval above through a different call.
	//float flInterval = StudioFrameAdvance( ); // animate
	
	//NOPE, See notes above, it is better to replicate retail even more directly.  Very very sensitive.
	// the _SIMPLE version is now that.
	//StudioFrameAdvance(flInterval);
	float flInterval = StudioFrameAdvance_SIMPLE();



	//MODDD MAJOR MAJOR MAJOR NOTE - is using the same interval from animation across the rest of logic bad?
	//Should gpGlobals->frametime be used instead?  This has seemed reliable 99% of the time in any case.
	//Seems changing the animation way too often (every think frame) could reset the anim time, forcing an interval of 0 which
	//breaks the tolerance checks for advancing path nodes.  But for that... don't set animations that often then.


	if(EASY_CVAR_GET_DEBUGONLY(animationPrintouts) == 1 && monsterID >= -1)easyForcePrintLine("%s:%d Anim info C? frame:%.2f done:%d", getClassname(), monsterID, pev->frame, m_fSequenceFinished);


// start or end a fidget
// This needs a better home -- switching animations over time should be encapsulated on a per-activity basis
// perhaps MaintainActivity() or a ShiftAnimationOverTime() or something.



	//easyPrintLine("HEY THERE undesirable person %d", m_fSequenceFinished);

	//Is that last part OKAY?
	//if ( m_MonsterState != MONSTERSTATE_SCRIPT && m_MonsterState != MONSTERSTATE_DEAD && m_Activity == ACT_IDLE && m_fSequenceFinished )
	
	if(monsterID == 6){
		int x = 45;
	}



	if (EASY_CVAR_GET_DEBUGONLY(animationPrintouts) == 1 && monsterID >= -1) {
		easyForcePrintLine(
			"%s:%d Anim info IDLE RESET check?: custo:%d autoblock:%d stateForbid:%d idle?%d seqfin?%d - frame:%.2f done:%d",
			getClassname(), monsterID,
			usingCustomSequence,
			getMonsterBlockIdleAutoUpdate(),
			(m_MonsterState == MONSTERSTATE_SCRIPT || m_MonsterState == MONSTERSTATE_DEAD),
			(m_Activity == ACT_IDLE),
			m_fSequenceFinished,
			pev->frame, m_fSequenceFinished
		);
	}




	// If looping and using a custom sequence (set by some "setSequenceBy..." method or similar, as opposed to selected by a new activity),
	// do NOT force a new animation! We mean to keep the current animation.
	// ALSO IMPORTANT: This will <interfere> with tasks based on waiting for for the sequence to be complete, which would be seen the next frame. <disregard> that <inconsequential substance>.
	// !!! WAIT.  Wouldn't an easy check in place of the '!usingCustomSequence' be, if using a custom sequence but it doesn't loop, go ahead and pick a new one?
	// Yea, let's try that.    Idea behind it is, if using a custom sequence with a definite end (monster stuck looking frozen),
	// ignore the usingCustomSequence and pick a new sequence anyway to fit the IDLE intent, better than looking frozen unintentionally.
	// NEVERMIND, let thing setting custom sequences that should switch over to ordinary idle tick usingCustomSequence off after calling for them.
	// This seems to cause random rare issues with idle anims getting picked for things that really, really don't want them, like right before/after
	// (unclear) hassault firing to make it a little glitchy, same for panthereye on jumps.   UGH.
	// Also, may as well let finished ACT_TURN_LEFT/RIGHT anims get re-picked as idle, staying stuck in the end-turn frame
	// is just kinda awkward.
	if (
		//!(m_pSchedule != NULL && getTaskNumber() ==  
		
		//!(m_fSequenceLoops && usingCustomSequence) && 
		!IsMoving() && pev->framerate != 0 && m_fSequenceFinished && (!usingCustomSequence) &&  //  || !m_fSequenceLoops
		!getMonsterBlockIdleAutoUpdate() && m_MonsterState != MONSTERSTATE_SCRIPT && m_MonsterState != MONSTERSTATE_DEAD
		 ) 
	{
		if(m_Activity == ACT_IDLE){
			//good to go.	
			RefreshActivity();
		}else if(m_Activity == ACT_TURN_LEFT || m_Activity == ACT_TURN_RIGHT){
			// force it to ACT_IDLE.
			SetActivity(ACT_IDLE);
		}
		
	}//lotsa conditions for when to reset a still animation

	if(EASY_CVAR_GET_DEBUGONLY(animationPrintouts) == 1 && monsterID >= -1)easyForcePrintLine("%s:%d Anim info D? frame:%.2f done:%d", getClassname(), monsterID, pev->frame, m_fSequenceFinished);
	DispatchAnimEvents( flInterval );
	if(EASY_CVAR_GET_DEBUGONLY(animationPrintouts) == 1 && monsterID >= -1)easyForcePrintLine("%s:%d Anim info E? frame:%.2f done:%d", getClassname(), monsterID, pev->frame, m_fSequenceFinished);

	if ( !MovementIsComplete() )
	{
		//easyForcePrintLine("GET good sonny A");
		Move( flInterval );
		//MODDD - flInterval may be reset on the same frame the animation is changed.  If that is ok, keep using Move(flInterval) like retail does.
		//Move(gpGlobals->frametime);

	}
#if _DEBUG	
	else 
	{
		//easyForcePrintLine("GET good sonny B");
		if ( !TaskIsRunning() && !TaskIsComplete() )
			ALERT( at_error, "Schedule stalled!!\n" );
	}
#endif
	

	/*
	//MODDD - are you insane
		pev->renderamt = 255;
		pev->rendermode = kRenderTransTexture;
		pev->renderfx = kRenderFxGlowShell;
	*/


	
	if(EASY_CVAR_GET_DEBUGONLY(peaceOut) == 1){

		if(m_hEnemy != NULL){
			//all is forgiven.
			//m_hEnemy = NULL;
			ForgetEnemy();

			if(this->m_MonsterState == MONSTERSTATE_COMBAT || this->m_IdealMonsterState == MONSTERSTATE_COMBAT){
				this->m_MonsterState = MONSTERSTATE_IDLE;
				this->m_IdealMonsterState = MONSTERSTATE_IDLE;
			}
			TaskFail();
		}
	}

	if(EASY_CVAR_GET_DEBUGONLY(drawDebugEnemyLKP)){
		//::DebugLine_Setup(2, m_vecEnemyLKP + Vector(0, 0, 8), m_vecEnemyLKP + Vector(0, 0, -8), 0, 0, 255);
		//no... just draw it this frame only.
		::UTIL_drawPointFrame(m_vecEnemyLKP + Vector(0, 0, 8), DEBUG_LINE_WIDTH, 255, 255, 0);
	}




	//MODDD - NOTICE!  This check is not reliable!
	// Could force the checked origin to be some 5 to 10 below the one checked, but eh.
	// Leave it up to the map to skill stuff falls off into some ambiguous void, probably best.
	/*
	if (
		pev->movetype == MOVETYPE_STEP ||
		pev->movetype == MOVETYPE_TOSS ||
		pev->movetype == MOVETYPE_PUSH ||
		pev->movetype == MOVETYPE_BOUNCE ||
		pev->movetype == MOVETYPE_PUSHSTEP
	){
		// if we're of some movetype that has gravity, and we touch a point that is considered "sky", remove this entity.
		if(UTIL_PointContents(pev->origin) == CONTENTS_SKY ){
			UTIL_Remove(this);
		}
	}
	*/

	//easyForcePrintLine("FRAMEB:%.2f seq:%d loop:%d fin:%d", this->pev->frame, pev->sequence, m_fSequenceLoops, m_fSequenceFinished);

}//END OF monsterThink




//MODDD
//by default, don't use alternate models.  Use one if specified in a child class.
BOOL CBaseMonster::getGermanModelRequirement(void){
	return FALSE;
}
const char* CBaseMonster::getGermanModel(void){
	return NULL;
}
const char* CBaseMonster::getNormalModel(void){
	return NULL;
}


//MODDD
void CBaseMonster::setModel(void){
	CBaseMonster::setModel(NULL);
}
void CBaseMonster::setModel(const char* m){
	// too broad to do much here.

	// if unspecified, a little default behavior:
	const char* germanModelPath = getGermanModel();
	BOOL hasGermanModel = (germanModelPath != NULL);




	//if there is a german model at all (not everything may ever involve german model script)
	if(hasGermanModel){

		const char* normalModelPath = getNormalModel();
		// This should NEVER be null, as the german & normal model paths should be set at the same time.


		// NOTICE: not sure what to do if  "getGermanModelRequirement()"  fails.  Crash?  Invisible model?    For now, just relying on retail's version.
		if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(sv_germancensorship) == 0 || EASY_CVAR_GET_DEBUGONLY(allowGermanModels) != 1 || globalPSEUDO_canApplyGermanCensorship == 0 || getGermanModelRequirement() == FALSE){
			// but we're using the german model...
			//if(usingGermanModel){
				SET_MODEL(ENT(pev), normalModelPath);
			//}
			//if german censorship is on
		}else if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(sv_germancensorship) == 1 && EASY_CVAR_GET_DEBUGONLY(allowGermanModels) == 1 && globalPSEUDO_canApplyGermanCensorship == 1){
			// but we're not using the german model (and have one)
			//if(hasGermanModel && !usingGermanModel){
			//if(hasGermanModel){   //REDUNDANT.
				SET_MODEL(ENT(pev), germanModelPath);
			//}
			//}
		}else{
			// FREAKY INBETWEEN STATE.  HELP!!!
			easyPrintLine("!!!ERROR 93r|\\|\\4|\\|.  %s:%d", this->getClassname(), this->monsterID);
		}

	}else{

		if(m != NULL && m[0] != '\0'){
			// Non-empty string? Not using the GermanModel system? Just go with what was sent.
			CBaseEntity::setModel(m);
		}
	}

}

BOOL CBaseMonster::getMonsterBlockIdleAutoUpdate(){
	// by default, monsters have no problems looping the idle anim / looking for a new one automatically when it finishes.
	// Override this method in child monster classes (children of CBaseMonster) to change this per child.
	return FALSE;
}//END OF getMonsterBlockIdleAutoUpdate


//=========================================================
// CBaseMonster - USE - will make a monster angry at whomever
// activated it.
//=========================================================
void CBaseMonster::MonsterUse ( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	//MODDD - maybe don't force to ALERT?  Monsters in a COMBAT state don't need this downgrade.
	// Instead, how about they look at whatever called USE on the monster?
	// If they hate the monster that called USE that is.
	//m_IdealMonsterState = MONSTERSTATE_ALERT;



	if (m_MonsterState == MONSTERSTATE_SCRIPT){
		if (m_pCine != NULL) {
			if (m_pCine->CanInterrupt()) {
				// allowed.
			}
			else {
				// No.
				return;
			}
		}
	}


	if (pActivator != NULL) {
		// If not already in view, look startled and turn to look at em'
		if (!FInViewCone(pActivator)){
			int theRel = IRelationship(pActivator);
			Schedule_t* myIdealFacingSched = GetScheduleOfType(TASK_FACE_IDEAL);
			if ( 
				this->m_pSchedule != myIdealFacingSched &&
				(theRel > R_NO || theRel == R_FR)
			) {
				setEnemyLKP(pActivator);
				MakeIdealYaw(m_vecEnemyLKP);

				//ChangeSchedule(GetScheduleOfType(TASK_FACE_IDEAL));

				// this is less invasive, as seen elsewhere to make a monster look at the source of a sound.
				this->SetConditions(bits_COND_LIGHT_DAMAGE);
			}
		}
	}

}

//=========================================================
// Ignore conditions - before a set of conditions is allowed
// to interrupt a monster's schedule, this function removes
// conditions that we have flagged to interrupt the current
// schedule, but may not want to interrupt the schedule every
// time. (Pain, for instance)
//=========================================================
int CBaseMonster::IgnoreConditions ( void )
{
	int iIgnoreConditions = 0;

	if(monsterID == 16){
		int cineID = -1;
		if (m_pCine != NULL) cineID = m_pCine->monsterID;
		int x = 45;
	}

	if ( !FShouldEat() )
	{
		// not hungry? Ignore food smell.
		iIgnoreConditions |= bits_COND_SMELL_FOOD;
	}

	/*
	if (FClassnameIs(pev, "monster_gargantua")) {
		int x = 666;
		if (m_MonsterState != MONSTERSTATE_SCRIPT) {
			int x2 = 666;
		}
		if (m_pCine == NULL) {
			int x2 = 666;
		}
		else {
			int theid = ENTINDEX(m_pCine->edict());
			float agg1 = m_pCine->pev->nextthink;
			float agg2 = gpGlobals->time;
			void* agg3 = &m_pCine->m_pfnThink;
		}

		int err = 444;
	}
	*/


	if (m_pCine) {
		//MODDD - if the m_pCine is not NULL and disallows interruptions, also
		// add these conditions to ignore.  May as well be easier to stay on track.
		if (m_MonsterState == MONSTERSTATE_SCRIPT || !m_pCine->CanInterrupt()) {
			iIgnoreConditions |= m_pCine->IgnoreConditions();
		}
	}

	return iIgnoreConditions;
}

// No need for iIgnoreConditionsMod, not many.



//=========================================================
// 	RouteClear - zeroes out the monster's route array and goal
//=========================================================
void CBaseMonster::RouteClear ( void )
{
	disableEnemyAutoNode = FALSE;

	//easyForcePrintLine("OH no ROUTECLEAR");

	RouteNew();
	m_movementGoal = MOVEGOAL_NONE;
	m_movementActivity = ACT_IDLE;
	Forget( bits_MEMORY_MOVE_FAILED );
}

//=========================================================
// Route New - clears out a route to be changed, but keeps
//				goal intact.
//=========================================================
void CBaseMonster::RouteNew ( void )
{
	m_Route[ 0 ].iType = 0;
	m_iRouteIndex = 0;
	m_iRouteLength = 0;  //NEW - ...somehow?  No nodes worth counting.

	// about to move?
	recentMoveExecuteFailureCooldown = -1;
}

//=========================================================
// FRouteClear - returns TRUE is the Route is cleared out
// ( invalid )
//=========================================================
BOOL CBaseMonster::FRouteClear ( void )
{
	//MODDD - NOTE
	// Note that this is not clearing the route and returning success of clearing the route. This is a check to see if the route is clear right now
	// without affecting it at all.
	// Was any difference between FRouteClear and MovementIsComplete?  Both are used by pathfinding-related tasks
	// to determine whether the monster is done with the current route, some in StartTask, some in RunTask more consistently.
	// Really don't know.
	// Although judging from other areas, it looks like the 'route[routeIndex].type == 0' is to check whether this has
	// reached an invalid part of the route (past the most recently assigned route), or for starting with an iType of 0
	// at route[0] (routeIndex still at 0).  The new m_iRouteLength is a better check for that.
	
	//disableEnemyAutoNode = FALSE;

	
	//if ( m_Route[ m_iRouteIndex ].iType == 0 || m_movementGoal == MOVEGOAL_NONE )
	if ( m_iRouteIndex >= m_iRouteLength || m_movementGoal == MOVEGOAL_NONE )
		return TRUE;

	return FALSE;
}





// Strict is more similar to retail FRefreshRoute, in that it doesn't allow BuildNearestRoute
// attempts if the main BuildRoute fails.
// 'strictNodeTolerance' may also be something that replicates retail moreso (not much tolerance
// for reaching a node).
BOOL CBaseMonster::FRefreshRouteStrict ( void )
{
	CBaseEntity *pPathCorner;
	int i;
	BOOL returnCode;

	RouteNew();

	strictNodeTolerance = TRUE;

	returnCode = FALSE;

	switch( m_movementGoal )
	{
		case MOVEGOAL_PATHCORNER:{
			// monster is on a path_corner loop
			pPathCorner = m_pGoalEnt;
			i = 0;

			while ( pPathCorner && i < ROUTE_SIZE )
			{
				m_Route[i].iType = bits_MF_TO_PATHCORNER;
				m_Route[i].vecLocation = pPathCorner->pev->origin;
				m_Route[i].iMapNodeIndex = -1;

				pPathCorner = pPathCorner->GetNextTarget();

				// Last path_corner in list?
				if ( !pPathCorner )
					m_Route[i].iType |= bits_MF_IS_GOAL;
					
				i++;
			}
			//MODDD - looks safe to treat 'i' as the route length then.
			// If there is no GoalEnt (first pPathCorner choice), there is no route (0 length).
			// If it ends after one iteration of the loop, there is 1 node (i++ makes that 0 -> 1, fitting length).
			// Any other reason it ends (no pPathCorner choice, out of ROUTE_SIZE), i is also left as a good choice.
			m_iRouteLength = i;
			returnCode = TRUE;
		}break;
		case MOVEGOAL_ENEMY:{
			setEnemyLKP(m_hEnemy);

			Vector vecEnemyLKP_LowerCenter = m_vecEnemyLKP;
			vecEnemyLKP_LowerCenter.z += m_flEnemyLKP_zOffset;

			returnCode = BuildRoute( vecEnemyLKP_LowerCenter, bits_MF_TO_ENEMY, m_hEnemy );

			//MODDD - CHECK. Is automatically setting "m_vecMoveGoal" to the goal of this path ok? m_vecMoveGoal is usually an input.
			// Think so, a successful 'FGetNodeRoute' call leads to that.
			if(returnCode){
				m_vecMoveGoal = vecEnemyLKP_LowerCenter;
			}

		}break;
		case MOVEGOAL_LOCATION:{
			returnCode = BuildRoute( m_vecMoveGoal, bits_MF_TO_LOCATION, NULL );

		}break;
		case MOVEGOAL_TARGETENT:{
			if (m_hTargetEnt != NULL)
			{
				Vector vecTarget_LowerCenter = m_hTargetEnt->pev->origin;
				vecTarget_LowerCenter.z += m_hTargetEnt->pev->mins.z;

				returnCode = BuildRoute( vecTarget_LowerCenter, bits_MF_TO_TARGETENT, m_hTargetEnt );

				//MODDD - CHECK. Is automatically setting "m_vecMoveGoal" to the goal of this path ok? m_vecMoveGoal is usually an input.
				if(returnCode){
					m_vecMoveGoal = vecTarget_LowerCenter;
				}

			}else{
				returnCode = FALSE;
			}

		}break;
		case MOVEGOAL_NODE:{
			returnCode = FGetNodeRoute(pev->origin, m_vecMoveGoal );
			//MODDD - NOTE.  Found commented out as-is.  Is simplifying a node-route really that bad, or it's just not done this soon?
			// Maybe that's already part of FGetNodeRoute.
//			if ( returnCode )
//				RouteSimplify( NULL );
		}break;
	}//movegoal?


	//MODDD
	// Now, do another check to make sure this route is actually passable for at least one frame.
	// It is possible to pick a route that will fail the very next time, leading to flinching back and forth between
	// a standing and move animation.  If this fails, that point isn't reached.
	// (some things don't want this check or don't have custom move script properly separated, they turn 'SegmentedMove' off
	// to avoid this).



	if (returnCode != FALSE && usesSegmentedMove()) {
		returnCode = CheckPreMove();
	}

	return returnCode;
}//FRefreshRouteStrict



//=========================================================
// FRefreshRoute - after calculating a path to the monster's
// target, this function copies as many waypoints as possible
// from that path to the monster's Route array
//=========================================================
//MODDD - few changes from retail, mainly taking the nearest point (buildnearestroute) if a route
// to the goal exactly can't be made.

BOOL CBaseMonster::FRefreshRoute ( void )
{
	CBaseEntity *pPathCorner;
	int i;
	BOOL returnCode;

	if(!strictNodeTolerance){
		// proceed as usual.
	}else{
		// redirect to the strict version.
		return FRefreshRouteStrict();
	}


	if(monsterID == 3){
		int x = 45;
	}

	RouteNew();

	returnCode = FALSE;

	switch( m_movementGoal )
	{
		case MOVEGOAL_PATHCORNER:{
			// monster is on a path_corner loop
			pPathCorner = m_pGoalEnt;
			i = 0;

			while ( pPathCorner && i < ROUTE_SIZE )
			{
				m_Route[i].iType = bits_MF_TO_PATHCORNER;
				m_Route[i].vecLocation = pPathCorner->pev->origin;
				m_Route[i].iMapNodeIndex = -1;

				pPathCorner = pPathCorner->GetNextTarget();

				// Last path_corner in list?
				if ( !pPathCorner )
					m_Route[i].iType |= bits_MF_IS_GOAL;
					
				i++;
			}
			//MODDD - looks safe to treat 'i' as the route length then.
			// If there is no GoalEnt (first pPathCorner choice), there is no route (0 length).
			// If it ends after one iteration of the loop, there is 1 node (i++ makes that 0 -> 1, fitting length).
			// Any other reason it ends (no pPathCorner choice, out of ROUTE_SIZE), i is also left as a good choice.
			m_iRouteLength = i;
			returnCode = TRUE;
		}break;
		case MOVEGOAL_ENEMY:{
			/*
			if(m_hEnemy == FALSE){
				// what's the point?
				// but we can still follow the existing LKP? I don't know.
				return FALSE;
			}
			*/

			Vector vecEnemyLKP_LowerCenter = m_vecEnemyLKP;
			vecEnemyLKP_LowerCenter.z += m_flEnemyLKP_zOffset;

			setEnemyLKP(m_hEnemy);
			returnCode = BuildRoute( vecEnemyLKP_LowerCenter, bits_MF_TO_ENEMY, m_hEnemy );


			//MODDD - why wasn't there a BuildNearestRoute attempt if above failed?
			if (returnCode == FALSE)
			{
				Vector vecEnemyEyePosFromBottom = m_vecEnemyLKP_ViewOFS;
				vecEnemyEyePosFromBottom.z -= m_flEnemyLKP_zOffset;

				// ?????   so sending the enemy-goal moveflag is bad (reasons why at the top of this file).
				// can we send the m_hEnemy anyway?
				// Go ahead and send both, but the method will know not to use the moveflag.
				returnCode = BuildNearestRoute( vecEnemyLKP_LowerCenter, vecEnemyEyePosFromBottom, 0, (vecEnemyLKP_LowerCenter - pev->origin).Length(), DEFAULT_randomNodeSearchStart, bits_MF_TO_ENEMY, m_hEnemy );
				//returnCode = BuildNearestRoute( vecEnemyLKP_LowerCenter, m_hEnemy->pev->view_ofs, 0, (vecEnemyLKP_LowerCenter - pev->origin).Length(), DEFAULT_randomNodeSearchStart, 0, m_hEnemy);
			}

			//MODDD - CHECK. Is automatically setting "m_vecMoveGoal" to the goal of this path ok? m_vecMoveGoal is usually an input.
			if(returnCode){
				m_vecMoveGoal = vecEnemyLKP_LowerCenter;
			}

		}break;
		case MOVEGOAL_LOCATION:{
			returnCode = BuildRoute( m_vecMoveGoal, bits_MF_TO_LOCATION, NULL );

			//MODDD - can we do a nearest instead?
			if(returnCode == FALSE){
				returnCode = BuildNearestRoute(m_vecMoveGoal, pev->view_ofs, 0, (m_vecMoveGoal - pev->origin).Length(), DEFAULT_randomNodeSearchStart, bits_MF_TO_LOCATION, NULL );
			}

		}break;
		case MOVEGOAL_TARGETENT:{
			if (m_hTargetEnt != NULL)
			{
				//MODDD - route to the lower bounds.  Mainly makes a difference for the player as a target.
				Vector vecTarget_LowerCenter = m_hTargetEnt->pev->origin;
				vecTarget_LowerCenter.z += m_hTargetEnt->pev->mins.z;

				returnCode = BuildRoute( vecTarget_LowerCenter, bits_MF_TO_TARGETENT, m_hTargetEnt );

				if(!returnCode){
					// is this okay?
					returnCode = BuildNearestRoute( vecTarget_LowerCenter, m_hTargetEnt->pev->view_ofs, 0, (vecTarget_LowerCenter - this->pev->origin).Length() + 500, DEFAULT_randomNodeSearchStart, bits_MF_TO_TARGETENT, m_hTargetEnt  );
				}

				if(returnCode){
					m_vecMoveGoal = vecTarget_LowerCenter;
				}

			}else{
				returnCode = FALSE;
			}


		}break;
		case MOVEGOAL_NODE:{
			returnCode = FGetNodeRoute(pev->origin, m_vecMoveGoal );
			//MODDD - NOTE.  Found commented out as-is.  Is simplifying a node-route really that bad, or it's just not done this soon?
			// Maybe that's already part of FGetNodeRoute.
//			if ( returnCode )
//				RouteSimplify( NULL );
		}break;
	}//movegoal?


	//MODDD
	// Now, do another check to make sure this route is actually passable for at least one frame.
	// It is possible to pick a route that will fail the very next time, leading to flinching back and forth between
	// a standing and move animation.  If this fails, that point isn't reached.
	// (some things don't want this check or don't have custom move script properly separated, they turn 'SegmentedMove' off
	// to avoid this).
	if (returnCode != FALSE && usesSegmentedMove()) {
		returnCode = CheckPreMove();
	}

	return returnCode;
}//FRefreshRoute




//MODDD - NEW.  Like FRefreshRoute but doesn't use 'BuildNearestRoute'.  Either we can reach a given node or don't.
// Checks for picking the best randon nearby node will go through many BuildRoute calls and BuildNearest route could end
// up being very expensive and redundant on top of that.
// Note that FRefreshRouteStrict also skips BuildNearestRoute calls.   However, the strict version prevents other Refresh
// calls from ever calling BuildNearestRoute either, any pathfinding leading off RefreshRouteCheap may still call
// BuildNearestRoute later if interrupted.
BOOL CBaseMonster::FRefreshRouteCheap ( void )
{
	CBaseEntity *pPathCorner;
	int i;
	BOOL returnCode;


	if(monsterID == 3){
		int x = 45;
	}

	RouteNew();

	returnCode = FALSE;

	switch( m_movementGoal )
	{
		case MOVEGOAL_PATHCORNER:{
				// monster is on a path_corner loop
				pPathCorner = m_pGoalEnt;
				i = 0;

				while ( pPathCorner && i < ROUTE_SIZE )
				{
					m_Route[i].iType = bits_MF_TO_PATHCORNER;
					m_Route[i].vecLocation = pPathCorner->pev->origin;
					m_Route[i].iMapNodeIndex = -1;

					pPathCorner = pPathCorner->GetNextTarget();

					// Last path_corner in list?
					if ( !pPathCorner )
						m_Route[i].iType |= bits_MF_IS_GOAL;
					
					i++;
				}
				//MODDD - looks safe to treat 'i' as the route length then.
				// If there is no GoalEnt (first pPathCorner choice), there is no route (0 length).
				// If it ends after one iteration of the loop, there is 1 node (i++ makes that 0 -> 1, fitting length).
				// Any other reason it ends (no pPathCorner choice, out of ROUTE_SIZE), i is also left as a good choice.
				m_iRouteLength = i;
				returnCode = TRUE;
		}break;
		case MOVEGOAL_ENEMY:{
			/*
			if(m_hEnemy == FALSE){
				//what's the point?
				//...but we can still follow the existing LKP? I don't know.
				return FALSE;
			}
			*/

			Vector vecEnemyLKP_LowerCenter = m_vecEnemyLKP;
			vecEnemyLKP_LowerCenter.z += m_flEnemyLKP_zOffset;

			//setEnemyLKP(m_hEnemy);
			returnCode = BuildRoute( vecEnemyLKP_LowerCenter, bits_MF_TO_ENEMY, m_hEnemy );


			//if (returnCode == FALSE)
			//{
			//	returnCode = BuildNearestRoute( vecEnemyLKP_LowerCenter, m_hEnemy->pev->view_ofs, 0, (vecEnemyLKP_LowerCenter - pev->origin).Length(), DEFAULT_randomNodeSearchStart, bits_MF_TO_ENEMY, m_hEnemy );
			//}

			//MODDD - CHECK. Is automatically setting "m_vecMoveGoal" to the goal of this path ok? m_vecMoveGoal is usually an input.
			if(returnCode){
				m_vecMoveGoal = vecEnemyLKP_LowerCenter;
			}

		}break;
		case MOVEGOAL_LOCATION:{
			returnCode = BuildRoute( m_vecMoveGoal, bits_MF_TO_LOCATION, NULL );

			//MODDD - can we do a nearest instead?              no
			//if(returnCode == FALSE){
			//	returnCode = BuildNearestRoute(m_vecMoveGoal, pev->view_ofs, 0, (m_vecMoveGoal - pev->origin).Length(), DEFAULT_randomNodeSearchStart, bits_MF_TO_LOCATION, NULL );
			//}

		}break;
		case MOVEGOAL_TARGETENT:{
			if (m_hTargetEnt != NULL)
			{
				//MODDD - route to the lower bounds.  Mainly makes a difference for the player as a target.
				Vector vecTarget_LowerCenter = m_hTargetEnt->pev->origin;
				vecTarget_LowerCenter.z += m_hTargetEnt->pev->mins.z;

				returnCode = BuildRoute( vecTarget_LowerCenter, bits_MF_TO_TARGETENT, m_hTargetEnt );

				//if(!returnCode){
				//	//is this okay?             no
				//	returnCode = BuildNearestRoute( vecTarget_LowerCenter, m_hTargetEnt->pev->view_ofs, 0, (vecTarget_LowerCenter - this->pev->origin).Length() + 500, DEFAULT_randomNodeSearchStart, bits_MF_TO_TARGETENT, m_hTargetEnt  );
				//}

				if(returnCode){
					m_vecMoveGoal = vecTarget_LowerCenter;
				}

			}else{
				returnCode = FALSE;
			}

		}break;
		case MOVEGOAL_NODE:{
			returnCode = FGetNodeRoute(pev->origin, m_vecMoveGoal );
//			if ( returnCode )
//				RouteSimplify( NULL );
		}break;
	}//end of switch

	if (returnCode != FALSE && usesSegmentedMove()) {
		returnCode = CheckPreMove();
	}

	return returnCode;
}//FRefreshRouteCheap





//MODDD - new. Clone of FRefreshRoute that better incorporates the method calls done in "CHASE_ENEMY"'s schedule.
// This ends up being similar to a call to MoveToEnemy followed by FRefreshRoute.
BOOL CBaseMonster::FRefreshRouteChaseEnemySmart(void){
	
	BOOL m1 = disableEnemyAutoNode;
	Activity m2 = m_movementActivity;
	float m3 = m_moveWaitTime;
	int m4 = m_movementGoal;
	Vector m5 = m_vecMoveGoal;


	if(monsterID == 3){
		int x = 45;
	}

	
	BOOL returnCode;
	
	/*	
	if(m_hEnemy == NULL){
		//what's the point?
		//...but we can still follow the existing LKP? I don't know.
		return FALSE;
	}
	*/

	
	//case TASK_GET_PATH_TO_ENEMY:
	CBaseEntity *pEnemy = m_hEnemy;


	disableEnemyAutoNode = TRUE;
	//probably unnecessary. no harm.
	m_movementActivity = ACT_RUN; //is that safe to assume?
	m_moveWaitTime = 2; //same?

	//HACKER SACKS!!!!!    was MOVEGOAL_ENEMY;
	//m_movementGoal = MOVEGOAL_LOCATION;
	//int iMoveFlaggg = bits_MF_TO_LOCATION;
	
	m_movementGoal = MOVEGOAL_ENEMY;
	int iMoveFlaggg = bits_MF_TO_ENEMY;



	// was this... missing?
	// Not really needed, BuildRoute already does that soon
	//RouteNew();


	//Mainly just to keep track of where the enemy is since last calling for a new route.
	//As in, if the enemy moves too far from the last place a path found a route TO, we need to re-route to keep the destination accurate.
	


	/*
	m_vecMoveGoal = m_hEnemy->pev->origin;

	//MODDD - new?  No need, rely on the existing LKP for now
	setEnemyLKP(m_hEnemy);
	*/

	Vector vecEnemyLKP_LowerCenter = m_vecEnemyLKP;
	vecEnemyLKP_LowerCenter.z += m_flEnemyLKP_zOffset;

	//Vector vecEnemyEyePosFromBottom = pEnemy->pev->view_ofs;
	//vecEnemyEyePosFromBottom.z -= pEnemy->pev->mins.z;

	//Vector vecEnemyEyePosFromBottom = pEnemy->pev->view_ofs;
	//vecEnemyEyePosFromBottom.z -= pEnemy->pev->mins.z;



	// wait, not just yet
	//m_vecMoveGoal = vecEnemyLKP_LowerCenter;
	
	//MODDD - used to have "pEnemy->pev->origin" as the first argument for BuildRoute and BuildNearestRoute. Now just pipes along the set "m_vecMoveGoal" above, no problem.
	
	returnCode = BuildRoute(vecEnemyLKP_LowerCenter, iMoveFlaggg, pEnemy);
	
	if(!returnCode){
		// try this
		Vector vecEnemyEyePosFromBottom = m_vecEnemyLKP_ViewOFS;
		vecEnemyEyePosFromBottom.z -= m_flEnemyLKP_zOffset;
		
		returnCode = BuildNearestRoute(vecEnemyLKP_LowerCenter, vecEnemyEyePosFromBottom, 0, (vecEnemyLKP_LowerCenter - pev->origin).Length(), DEFAULT_randomNodeSearchStart, iMoveFlaggg, pEnemy);
	}



	if(returnCode){
		// ok.  I think this is good so that the location of the enemy is used for tolerance checks instead
		// of just being close to the next 'goal' node.  If approaching a goal-node to a nearest route the
		// tolerance shouldn't be used after all (unless it happens to lead close to the enemy position).
		m_vecMoveGoal = vecEnemyLKP_LowerCenter;
	}else{
		// no way to get there =(
		//ALERT ( at_aiconsole, "GetPathToEnemy failed!!\n" );
		//TaskFail();
	}

	
	//MODDD - NEW.
	// - in case a revert is needed
	// Nevermind, let outside methods handle this.  by... maybe not setting m_movementActivity or waittimes if
	// FRefreshRoute fails...?
	//Activity oldMoveAct = m_movementActivity;
	//float oldMoveWaitTime = m_moveWaitTime;

	// Don't keep that move activity change if trying to move a single frame on this route would fail.
	// It might be possible to integrate this check into any "FRefreshRoute" call too, if necessary.
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	if (returnCode != FALSE && usesSegmentedMove()) {
		returnCode = CheckPreMove();
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	// If I didn't finish the route in one frame from the CheckPreMove check above, do another check.
	if(returnCode == TRUE && !FRouteClear()){
		if(goalDistTolerance > 0){
			// Hold on.  If I'm already close enough to my goal (within tolerance), end it already.
			// Moving a frame just to realize that is kinda weird.
			//float distToGoal2D = Distance2D(pev->origin, m_vecMoveGoal);
			//distToVecEnd = Distance2D(pev->origin, vecEndFiltered);
			//WayPoint_t* waypointGoalRef;
			//waypointGoalRef = GetGoalNode();
			//if(waypointGoalRef != NULL){
				//float distToGoal2D = (waypointGoalRef->vecLocation - pev->origin ).Length2D();
				//BOOL theZCheck = ZCheck(pev->origin, waypointGoalRef->vecLocation);
				float distToGoal2D = (m_vecMoveGoal - pev->origin ).Length2D();
				BOOL theZCheck = ZCheck(pev->origin, m_vecMoveGoal);
				//float disa2D = Distance2D(pev->origin, vecEndFiltered);
				//BOOL theZCheck = ZCheck(pev->origin, m_vecMoveGoal);
				if(theZCheck && distToGoal2D <= goalDistTolerance){
					// already done.
					easyPrintLine("Pathfind: WARNING!  %s:%d picked a route that has already been reached even after premove checks.", getClassname(), monsterID);
					MovementComplete();
					// no need to make the return code FALSE, doing whatever else the schedule wanted to do
					// should be fine but verify.  Maybe failing could be better to force a delay.
				}
			//}
		}
	}//returnCode

	if(returnCode == FALSE){
		// revert here too?
		disableEnemyAutoNode = m1;
		m_movementActivity = m2;
		m_moveWaitTime = m3;
		m_movementGoal = m4;
		m_vecMoveGoal = m5;
	}

	return returnCode;
}//END OF FRefreshRouteChaseEnemySmart




//MODDD - new.  Like FRefreshRouteChaseEnemySmart, but can't use BuildNearestRoute and doesn't affect
// the current route if it fails to find a better direct one (implied that this is called while going to 
// a BuildNearestRoute-provided node and the enemy is in sight, to see if routing straight to the enemy
// is possible)
// TODO.  Anything else route-related to be saved/restored to come back gracefully from failure?
BOOL CBaseMonster::FRefreshRouteChaseEnemySmartSafe(void){
	int i;

	BOOL m1 = disableEnemyAutoNode;
	Activity m2 = m_movementActivity;
	float m3 = m_moveWaitTime;
	int m4 = m_movementGoal;
	Vector m5 = m_vecMoveGoal;


	WayPoint_t OLD_m_Route[ROUTE_SIZE];
	int OLD_m_iRouteLength = m_iRouteLength;
	for(i = 0; i < m_iRouteLength; i++){
		OLD_m_Route[i].iType = m_Route[i].iType;
		OLD_m_Route[i].vecLocation = m_Route[i].vecLocation;
		OLD_m_Route[i].iMapNodeIndex = m_Route[i].iMapNodeIndex;
	}
	int	OLD_m_iRouteIndex = m_iRouteIndex;



	if(monsterID == 3){
		int x = 45;
	}

	BOOL returnCode;
	/*	
	if(m_hEnemy == NULL){
		//what's the point?
		//...but we can still follow the existing LKP? I don't know.
		return FALSE;
	}
	*/

	//case TASK_GET_PATH_TO_ENEMY:
	CBaseEntity *pEnemy = m_hEnemy;

	disableEnemyAutoNode = TRUE;
	//probably unnecessary. no harm.
	m_movementActivity = ACT_RUN; //is that safe to assume?
	m_moveWaitTime = 2; //same?

	//HACKER SACKS!!!!!    was MOVEGOAL_ENEMY;
	//m_movementGoal = MOVEGOAL_LOCATION;
	//int iMoveFlaggg = bits_MF_TO_LOCATION;
	
	m_movementGoal = MOVEGOAL_ENEMY;
	int iMoveFlaggg = bits_MF_TO_ENEMY;

	//Mainly just to keep track of where the enemy is since last calling for a new route.
	//As in, if the enemy moves too far from the last place a path found a route TO, we need to re-route to keep the destination accurate.
	
	/*
	m_vecMoveGoal = m_hEnemy->pev->origin;

	//MODDD - new?  No need, rely on the existing LKP for now
	setEnemyLKP(m_hEnemy);
	*/

	Vector vecEnemyLKP_LowerCenter = m_vecEnemyLKP;
	vecEnemyLKP_LowerCenter.z += m_flEnemyLKP_zOffset;

	//Vector vecEnemyEyePosFromBottom = pEnemy->pev->view_ofs;
	//vecEnemyEyePosFromBottom.z -= pEnemy->pev->mins.z;

	//Vector vecEnemyEyePosFromBottom = pEnemy->pev->view_ofs;
	//vecEnemyEyePosFromBottom.z -= pEnemy->pev->mins.z;



	// wait, not just yet
	//m_vecMoveGoal = vecEnemyLKP_LowerCenter;
	
	//MODDD - used to have "pEnemy->pev->origin" as the first argument for BuildRoute and BuildNearestRoute. Now just pipes along the set "m_vecMoveGoal" above, no problem.
	
	returnCode = BuildRoute(vecEnemyLKP_LowerCenter, iMoveFlaggg, pEnemy);
	

	if(returnCode){
		// ok
		m_vecMoveGoal = vecEnemyLKP_LowerCenter;
	}else{
		// leave the route the way it is.
		// any cleanup needed?



		// !!!



	}//returnCode checl

	

	
	// As of yet, can't use 'CheckPreMove' on this.  Have to assume what was picked was good,
	// unless the old route remained in some temporary state so that it could be restored
	// while this new one turned out to fail at one trial.
	//
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	if (returnCode != FALSE && usesSegmentedMove()) {
		returnCode = CheckPreMove();
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	

	// If I didn't finish the route in one frame from the CheckPreMove check above, do another check.
	if(returnCode == TRUE && !FRouteClear()){
		if(goalDistTolerance > 0){
			float distToGoal2D = (m_vecMoveGoal - pev->origin ).Length2D();
			BOOL theZCheck = ZCheck(pev->origin, m_vecMoveGoal);
			if(theZCheck && distToGoal2D <= goalDistTolerance){
				// already done.
				easyPrintLine("Pathfind: WARNING!  %s:%d picked a route that has already been reached even after premove checks.", getClassname(), monsterID);
				MovementComplete();
				// no need to make the return code FALSE, doing whatever else the schedule wanted to do
				// should be fine but verify.  Maybe failing could be better to force a delay.
			}
		}
	}//returnCode




	if(returnCode == FALSE){
		// revert here too?
		disableEnemyAutoNode = m1;
		m_movementActivity = m2;
		m_moveWaitTime = m3;
		m_movementGoal = m4;
		m_vecMoveGoal = m5;

		m_iRouteLength = OLD_m_iRouteLength;
		for(i = 0; i < OLD_m_iRouteLength; i++){
			m_Route[i].iType = OLD_m_Route[i].iType;
			m_Route[i].vecLocation = OLD_m_Route[i].vecLocation;
			m_Route[i].iMapNodeIndex = OLD_m_Route[i].iMapNodeIndex;
		}
		m_iRouteIndex = OLD_m_iRouteIndex;
	}

	return returnCode;
}//END OF FRefreshRouteChaseEnemySmartSafe






BOOL CBaseMonster::MoveToEnemy( Activity movementAct, float waitTime )
{
	Activity m1 = m_movementActivity;
	float m2 = m_moveWaitTime;
	int m3 = m_movementGoal;

	m_movementActivity = movementAct;
	m_moveWaitTime = waitTime;
	m_movementGoal = MOVEGOAL_ENEMY;

	//MODDD - only keep movement activity if this passes
	BOOL theResult = FRefreshRoute();
	if (!theResult) {
		m_movementActivity = m1;
		m_moveWaitTime = m2;
		m_movementGoal = m3;
	}
	return theResult;
}


/*
// EXPERIMENTAL: supply a tolerance to use when approaching the goal?
// MODDD - TODO - finish this 'argGoalDistTolerance' goes nowhere else
// NEVERMIND, reading this ent's 'goalDistTolerance' should be good enough when it's needed.
BOOL CBaseMonster::MoveToLocation( Activity movementAct, float waitTime, const Vector &goal ){
	MoveToLocation(movementAct, waitTime, goal, 0);
}
*/

//, float argGoalDistTolerance 
BOOL CBaseMonster::MoveToLocation( Activity movementAct, float waitTime, const Vector &goal)
{
	Activity m1 = m_movementActivity;
	float m2 = m_moveWaitTime;
	int m3 = m_movementGoal;
	Vector m4 = m_vecMoveGoal;

	m_movementActivity = movementAct;
	m_moveWaitTime = waitTime;
	m_movementGoal = MOVEGOAL_LOCATION;
	m_vecMoveGoal = goal;

	//MODDD - only keep movement activity if this passes
	BOOL theResult = FRefreshRoute();
	if (!theResult) {
		m_movementActivity = m1;
		m_moveWaitTime = m2;
		m_movementGoal = m3;
		m_vecMoveGoal = m4;
	}
	return theResult;
}


BOOL CBaseMonster::MoveToLocationStrict( Activity movementAct, float waitTime, const Vector &goal)
{
	Activity m1 = m_movementActivity;
	float m2 = m_moveWaitTime;
	int m3 = m_movementGoal;
	Vector m4 = m_vecMoveGoal;

	m_movementActivity = movementAct;
	m_moveWaitTime = waitTime;
	m_movementGoal = MOVEGOAL_LOCATION;
	m_vecMoveGoal = goal;

	//MODDD - only keep movement activity if this passes
	BOOL theResult = FRefreshRouteStrict();
	if (!theResult) {
		m_movementActivity = m1;
		m_moveWaitTime = m2;
		m_movementGoal = m3;
		m_vecMoveGoal = m4;
	}
	return theResult;
}

BOOL CBaseMonster::MoveToLocationCheap( Activity movementAct, float waitTime, const Vector &goal )
{
	Activity m1 = m_movementActivity;
	float m2 = m_moveWaitTime;
	int m3 = m_movementGoal;
	Vector m4 = m_vecMoveGoal;

	m_movementActivity = movementAct;
	m_moveWaitTime = waitTime;
	m_movementGoal = MOVEGOAL_LOCATION;
	m_vecMoveGoal = goal;

	//MODDD - only keep movement activity if this passes
	BOOL theResult = FRefreshRouteCheap();
	if (!theResult) {
		m_movementActivity = m1;
		m_moveWaitTime = m2;
		m_movementGoal = m3;
		m_vecMoveGoal = m4;
	}
	return theResult;
}


BOOL CBaseMonster::MoveToTarget( Activity movementAct, float waitTime ){
	Activity m1 = m_movementActivity;
	float m2 = m_moveWaitTime;
	int m3 = m_movementGoal;

	m_movementActivity = movementAct;
	m_moveWaitTime = waitTime;
	m_movementGoal = MOVEGOAL_TARGETENT;

	//MODDD - only keep movement activity if this passes
	BOOL theResult = FRefreshRoute();
	if (!theResult) {
		m_movementActivity = m1;
		m_moveWaitTime = m2;
		m_movementGoal = m3;
	}
	return theResult;
}

BOOL CBaseMonster::MoveToTargetStrict( Activity movementAct, float waitTime ){
	Activity m1 = m_movementActivity;
	float m2 = m_moveWaitTime;
	int m3 = m_movementGoal;

	m_movementActivity = movementAct;
	m_moveWaitTime = waitTime;
	m_movementGoal = MOVEGOAL_TARGETENT;

	//MODDD - only keep movement activity if this passes
	BOOL theResult = FRefreshRouteStrict();
	if (!theResult) {
		m_movementActivity = m1;
		m_moveWaitTime = m2;
		m_movementGoal = m3;
	}
	return theResult;
}

BOOL CBaseMonster::MoveToNode( Activity movementAct, float waitTime, const Vector &goal )
{
	Activity m1 = m_movementActivity;
	float m2 = m_moveWaitTime;
	int m3 = m_movementGoal;
	Vector m4 = m_vecMoveGoal;

	m_movementActivity = movementAct;
	m_moveWaitTime = waitTime;
	m_movementGoal = MOVEGOAL_NODE;
	m_vecMoveGoal = goal;


	//MODDD - only keep movement activity if this passes
	BOOL theResult = FRefreshRoute();
	if (!theResult) {
		m_movementActivity = m1;
		m_moveWaitTime = m2;
		m_movementGoal = m3;
		m_vecMoveGoal = m4;
	}
	return theResult;
}

void CBaseMonster::DrawRoute( entvars_t *pev, WayPoint_t *m_Route, int m_iRouteLength, int m_iRouteIndex, int r, int g, int b )
{
	int i;

	//MODDD - how about this check instead?
	//if ( m_Route[m_iRouteIndex].iType == 0 ){
	if ( m_iRouteLength == 0 ){
		ALERT( at_aiconsole, "No route!\n" );
		return;
	}

//	UTIL_ParticleEffect ( m_Route[ m_iRouteIndex ].vecLocation, g_vecZero, 255, 25 );

	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_BEAMPOINTS);
		WRITE_COORD( pev->origin.x );
		WRITE_COORD( pev->origin.y );
		WRITE_COORD( pev->origin.z );
		WRITE_COORD( m_Route[ m_iRouteIndex ].vecLocation.x );
		WRITE_COORD( m_Route[ m_iRouteIndex ].vecLocation.y );
		WRITE_COORD( m_Route[ m_iRouteIndex ].vecLocation.z );

		WRITE_SHORT( g_sModelIndexLaser );
		WRITE_BYTE( 0 ); // frame start
		WRITE_BYTE( 10 ); // framerate
		WRITE_BYTE( 1 ); // life
		WRITE_BYTE( 16 );  // width
		WRITE_BYTE( 0 );   // noise
		WRITE_BYTE( r );   // r, g, b
		WRITE_BYTE( g );   // r, g, b
		WRITE_BYTE( b );   // r, g, b
		WRITE_BYTE( 255 );	// brightness
		WRITE_BYTE( 10 );		// speed
	MESSAGE_END();

	//MODDD - using m_iRouteLength instead of ROUTE_SIZE
	for ( i = m_iRouteIndex ; i < m_iRouteLength - 1; i++ )
	{
		// BEWARE - routes are not 0'd before making a new route so the route after some
		// unmarked goal (last in intended route) could very well be garbage left over from 
		// a prior call.
		// Just use the new 'm_iRouteLength' instead.  Or, no need for that anymore.
		//if ( (m_Route[ i ].iType & bits_MF_IS_GOAL) || (m_Route[ i+1 ].iType == 0) )
		if ( (m_Route[ i ].iType & bits_MF_IS_GOAL) ){  // || i+1 >= m_iRouteLength
			break;
		}

		
		MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
			WRITE_BYTE( TE_BEAMPOINTS );
			WRITE_COORD( m_Route[ i ].vecLocation.x );
			WRITE_COORD( m_Route[ i ].vecLocation.y );
			WRITE_COORD( m_Route[ i ].vecLocation.z );
			WRITE_COORD( m_Route[ i + 1 ].vecLocation.x );
			WRITE_COORD( m_Route[ i + 1 ].vecLocation.y );
			WRITE_COORD( m_Route[ i + 1 ].vecLocation.z );
			WRITE_SHORT( g_sModelIndexLaser );
			WRITE_BYTE( 0 ); // frame start
			WRITE_BYTE( 10 ); // framerate
			WRITE_BYTE( 1 ); // life
			WRITE_BYTE( 8 );  // width
			WRITE_BYTE( 0 );   // noise
			WRITE_BYTE( r );   // r, g, b
			WRITE_BYTE( g );   // r, g, b
			WRITE_BYTE( b );   // r, g, b
			WRITE_BYTE( 255 );	// brightness
			WRITE_BYTE( 10 );		// speed
		MESSAGE_END();

//		UTIL_ParticleEffect ( m_Route[ i ].vecLocation, g_vecZero, 255, 25 );
	}
}//END OF DrawRoute(...)

void CBaseMonster::DrawMyRoute(int r, int g, int b){
	// redirect to that
	DrawRoute(pev, m_Route, m_iRouteLength, m_iRouteIndex, r, g, b);
}



int ShouldSimplify( int routeType )
{
	routeType &= ~bits_MF_IS_GOAL;

	if ( (routeType == bits_MF_TO_PATHCORNER) || (routeType & bits_MF_DONT_SIMPLIFY) )
		return FALSE;
	return TRUE;
}

//=========================================================
// RouteSimplify
//
// Attempts to make the route more direct by cutting out
// unnecessary nodes & cutting corners.
//
//=========================================================


void CBaseMonster::RouteSimplify( CBaseEntity *pTargetEnt )
{
	// BUGBUG: this doesn't work 100% yet
	int		i, count, outCount;
	Vector		vecStart;
	WayPoint_t	outRoute[ ROUTE_SIZE * 2 ];	// Any points except the ends can turn into 2 points in the simplified route

	// SAFETY
	for(i = 0; i < ROUTE_SIZE * 2; i++){
		outRoute[i].iMapNodeIndex = -1;
	}

	count = 0;

	//MODDD - only count up to m_iRouteLength now, no need for the entire node array (ROUTE_SIZE)
	// ...wait.  The whole point of this is to get the length of the current route.
	// Just use m_iRouteLength.
	/*
	for ( i = m_iRouteIndex; i < ROUTE_SIZE; i++ )
	{
		if ( !m_Route[i].iType )
			break;
		else
			count++;
		if ( m_Route[i].iType & bits_MF_IS_GOAL )
			break;
	}
	*/

	count = m_iRouteLength;


	// Can't simplify a direct route!
	if ( count < 2 )
	{
//		DrawRoute( pev, m_Route, m_iRouteIndex, 0, 0, 255 );
		return;
	}

	if(monsterID == 9){
		int x = 45;
	}

	outCount = 0;
	vecStart = pev->origin;

	//BOOL dontResetMapIndex = FALSE;

	//MODDD - shouldn't this avoid ever allowing i to go beyond 'count - 1' ?  At m_iRouteIndex above 0 that can happen
	//for ( i = 0; i < count-1; i++ )
	for ( i = 0; i < count-1 - m_iRouteIndex; i++ )
	{
		//dontResetMapIndex = FALSE;
		// Don't eliminate path_corners
		if ( !ShouldSimplify( m_Route[m_iRouteIndex+i].iType ) )
		{
			outRoute[outCount] = m_Route[ m_iRouteIndex + i ];
			//MODDD - pretty sure this outCount++ is a mistake.
			// In another place that gives up after all other checks fail (copy m_Route to outRoute directly),
			// it doesn't bump up outCount.
			// Often, reaching a point with the DONT_SIMPLIFY type leads to a garbage point being added to the
			// outRoute (or rather, left in as it's never changed).  Which goes to m_Route.
			//outCount++;
		}
		else if ( CheckLocalMove ( vecStart, m_Route[m_iRouteIndex+i+1].vecLocation, pTargetEnt, TRUE, NULL ) == LOCALMOVE_VALID )
		{
			// Skip vert
			continue;
		}
		else
		{
			Vector vecTest, vecSplit;

			// Halfway between this and next
			vecTest = (m_Route[m_iRouteIndex+i+1].vecLocation + m_Route[m_iRouteIndex+i].vecLocation) * 0.5;

			// Halfway between this and previous
			vecSplit = (m_Route[m_iRouteIndex+i].vecLocation + vecStart) * 0.5;

			int iType = (m_Route[m_iRouteIndex+i].iType | bits_MF_TO_DETOUR) & ~bits_MF_NOT_TO_MASK;
			if ( CheckLocalMove ( vecStart, vecTest, pTargetEnt, FALSE, NULL ) == LOCALMOVE_VALID )
			{
				outRoute[outCount].iType = iType;
				outRoute[outCount].vecLocation = vecTest;
				outRoute[outCount].iMapNodeIndex = -1;
			}
			else if ( CheckLocalMove ( vecSplit, vecTest, pTargetEnt, FALSE, NULL ) == LOCALMOVE_VALID )
			{
				outRoute[outCount].iType = iType;
				outRoute[outCount].vecLocation = vecSplit;
				outRoute[outCount].iMapNodeIndex = -1;
				outRoute[outCount+1].iType = iType;
				outRoute[outCount+1].vecLocation = vecTest;
				outRoute[outCount+1].iMapNodeIndex = -1;
				outCount++; // Adding an extra point
			}
			else
			{
				outRoute[outCount] = m_Route[ m_iRouteIndex + i ];
				//dontResetMapIndex = TRUE;
			}
		}
		// Get last point
		vecStart = outRoute[ outCount ].vecLocation;
		//outRoute[outCount].iMapNodeIndex = -1;
		outCount++;
	}
	ASSERT( i < count );
	outRoute[outCount] = m_Route[ m_iRouteIndex + i ];
	outCount++;
	
	// Terminate
	outRoute[outCount].iType = 0;
	outRoute[outCount].iMapNodeIndex = -1;
	ASSERT( outCount < (ROUTE_SIZE*2) );

	//MODDD - just make this decision once.
	// If outCount is under ROUTE_SIZE, that is all we need.
	// If outCount is over ROUTE_SIZE, went over our node limit, stick to ROUTE_SIZE.
	int outCountFiltered = min(ROUTE_SIZE, outCount);

// Copy the simplified route, disable for testing
	m_iRouteIndex = 0;
	for ( i = 0; i < outCountFiltered; i++ )
	{
		m_Route[i] = outRoute[i];
	}

	//MODDD - apply that.
	m_iRouteLength = outCountFiltered;

	// Terminate route
	if ( i < ROUTE_SIZE ){
		m_Route[i].iType = 0;
		m_Route[outCount].iMapNodeIndex = -1;
	}

// Debug, test movement code
#if 0
//	if ( CVAR_GET_FLOAT( "simplify" ) != 0 )
		DrawRoute( pev, outRoute, 0, 255, 0, 0 );
//	else
		DrawRoute( pev, m_Route, m_iRouteIndex, 0, 255, 0 );
#endif
}

//=========================================================
// FBecomeProne - tries to send a monster into PRONE state.
// right now only used when a barnacle snatches someone, so 
// may have some special case stuff for that.
//=========================================================
BOOL CBaseMonster::FBecomeProne ( void )
{
	if ( FBitSet ( pev->flags, FL_ONGROUND ) )
	{
		//MODDD NOTE - minus to remove something from a bitmask? Well you see something new everyday.
		// This is okay IF the bit is set. OTherwise subtacting would make the whole thing negative and weird.
		// weird to do it this way when  "pev->flags &= ~FL_ONGROUND" would've done it.
		pev->flags -= FL_ONGROUND;
	}

	m_IdealMonsterState = MONSTERSTATE_PRONE;


	// no script for you!  It is hopelessly broken to have run into this so what's the point.
	// MODDD - CRITICAL.  If for some reason anything hit by a barnacle is supposed to keep its script-link after
	// coming down, this might be such a good idea.

	// That is, even talking to the scientist before he runs into the barnacle or after he comes down (a1a2a or the equivalent
	// retail map), and the reported target ent is still "scripted", yet his schedule works like default (idle, panic, cowardly, etc.).
	// No attempt to follow the script after a barnacle breaks it.  If it is ever wanted to return to the script instead whenever
	// possible (out of barnacle), say so!
	m_hTargetEnt = NULL;
	
	
	//MODDD - turn off interpolation when bitten.
	if(EASY_CVAR_GET_DEBUGONLY(barnacleGrabNoInterpolation) == 1){
		pev->effects |= EF_NOINTERP;
	}
	

	return TRUE;
}

//=========================================================
// CheckRangeAttack1
//=========================================================
BOOL CBaseMonster::CheckRangeAttack1 ( float flDot, float flDist )
{
	if ( flDist > 64 && flDist <= 784 && flDot >= 0.5 )
	{
		return TRUE;
	}
	return FALSE;
}

//=========================================================
// CheckRangeAttack2
//=========================================================
BOOL CBaseMonster::CheckRangeAttack2 ( float flDot, float flDist )
{
	if ( flDist > 64 && flDist <= 512 && flDot >= 0.5 )
	{
		return TRUE;
	}
	return FALSE;
}

//=========================================================
// CheckMeleeAttack1
//=========================================================
BOOL CBaseMonster::CheckMeleeAttack1 ( float flDot, float flDist )
{
	// Decent fix to keep folks from kicking/punching hornets and snarks is to check the onground flag(sjb)

	//MODDD NOTE - isn't the above comment from the as-is script outdated, even before any modding? hornets are
	// meant to be untargetable by Classify(), preventing alien bioweapons from being targetable, I think.
	// Worst case scenario some extra "projectileIgnore()" check per enemy to see if it is worth targeting like
	// this would be good. As for snarks I don't see the problem with trying to melee them? Natural reflex to
	// swat off annoying fast-moving "bugs".
	// Zombies will also endlessly 'chase' turrets and miniturrets that are on the ground without ever
	// attacking them (runs into them over and over).  Seems to be from lacking FL_ONGROUND.
	// Probably from never touching the ground from the lack of gravity for the engine to set it.
	// Anyway, requirement removed.

	// FBitSet(m_hEnemy->pev->flags, FL_ONGROUND)
	if ( flDist <= 64 && flDot >= 0.7 && m_hEnemy != NULL )
	{
		return TRUE;
	}
	return FALSE;
}

//=========================================================
// CheckMeleeAttack2
//=========================================================
BOOL CBaseMonster::CheckMeleeAttack2 ( float flDot, float flDist )
{
	if ( flDist <= 64 && flDot >= 0.7 )
	{
		return TRUE;
	}
	return FALSE;
}

//=========================================================
// CheckAttacks - sets all of the bits for attacks that the
// monster is capable of carrying out on the passed entity.
//=========================================================
void CBaseMonster::CheckAttacks ( CBaseEntity *pTarget, float flDist )
{
	Vector2D vec2LOS;
	float flDot;

	UTIL_MakeVectors ( pev->angles );

	vec2LOS = ( pTarget->pev->origin - pev->origin ).Make2D();
	vec2LOS = vec2LOS.Normalize();

	flDot = DotProduct (vec2LOS , gpGlobals->v_forward.Make2D() );

	// we know the enemy is in front now. We'll find which attacks the monster is capable of by
	// checking for corresponding Activities in the model file, then do the simple checks to validate
	// those attack types.
	
	// Clear all attack conditions
	ClearConditions( bits_COND_CAN_RANGE_ATTACK1 | bits_COND_CAN_RANGE_ATTACK2 | bits_COND_CAN_MELEE_ATTACK1 |bits_COND_CAN_MELEE_ATTACK2 );
	ClearConditionsMod(bits_COND_COULD_RANGE_ATTACK1 | bits_COND_COULD_RANGE_ATTACK2 | bits_COND_COULD_MELEE_ATTACK1 | bits_COND_COULD_MELEE_ATTACK2);

	/*
	bits_COND_COULD_RANGE_ATTACK1
	bits_COND_COULD_RANGE_ATTACK2
	bits_COND_COULD_MELEE_ATTACK1
	bits_COND_COULD_MELEE_ATTACK2
	*/
	//couldRangedAttack1 = FALSE;


	if ( m_afCapability & bits_CAP_RANGE_ATTACK1 )
	{
		if ( CheckRangeAttack1 ( flDot, flDist ) ){
			SetConditions( bits_COND_CAN_RANGE_ATTACK1 );
			SetConditionsMod( bits_COND_COULD_RANGE_ATTACK1 );
		}

		/*
		//MODDD - new feature.  Could I range attack, IF I were facing the target?
		// NOTICE: removed.  Deemed most appropriate to work with within "CheckRangeAttack1", just include this when everything BUT the dot-check passes,
		// then check for the dot-check for total passing (returning TRUE as usual).
		// any Check___Attack method passing will set its can___Attack variable to TRUE for safety,
		// but CheckRangeAttack methods should set this themselves when everything but the dot product (facing the enemy enough)
		// is correct so that they think to face the enemy to get a better hit instead of stupidly failing pathfinding over and over,
		// which can happen for an enemy across an impassible obstacle that's facing the wrong way.
		if ( CouldRangeAttack1 ( flDot, flDist ) ){
			
			couldRangedAttack1 = TRUE;
		}
		*/

	}
	if ( m_afCapability & bits_CAP_RANGE_ATTACK2 )
	{
		if ( CheckRangeAttack2 ( flDot, flDist ) ){
			SetConditions( bits_COND_CAN_RANGE_ATTACK2 );
			SetConditionsMod( bits_COND_COULD_RANGE_ATTACK2 );
		}
	}
	if ( m_afCapability & bits_CAP_MELEE_ATTACK1 )
	{

		if ( CheckMeleeAttack1 ( flDot, flDist ) ){
			SetConditions( bits_COND_CAN_MELEE_ATTACK1 );
			SetConditionsMod( bits_COND_COULD_MELEE_ATTACK1 );
		}
	}
	if ( m_afCapability & bits_CAP_MELEE_ATTACK2 )
	{
		if ( CheckMeleeAttack2 ( flDot, flDist ) ){
			SetConditions( bits_COND_CAN_MELEE_ATTACK2 );
			SetConditionsMod( bits_COND_COULD_MELEE_ATTACK2 );
		}
	}

	//easyPrintLine("I CHECKED EM!!!!!! %d", HasConditions(bits_COND_CAN_MELEE_ATTACK1) );

}

//=========================================================
// CanCheckAttacks - prequalifies a monster to do more fine
// checking of potential attacks. 
//=========================================================
BOOL CBaseMonster::FCanCheckAttacks ( void )
{
	if ( HasConditions(bits_COND_SEE_ENEMY) && !HasConditions( bits_COND_ENEMY_TOOFAR ) )
	{
		return TRUE;
	}

	return FALSE;
}

//=========================================================
// CheckEnemy - part of the Condition collection process,
// gets and stores data and conditions pertaining to a monster's
// enemy. Returns TRUE if Enemy LKP was updated.
//=========================================================
int CBaseMonster::CheckEnemy ( CBaseEntity *pEnemy )
{
	float flDistToEnemy;
	int	iUpdatedLKP;// set this to TRUE if you update the EnemyLKP in this function.

	iUpdatedLKP = FALSE;
	ClearConditions ( bits_COND_ENEMY_FACING_ME );
	
	if(EASY_CVAR_GET_DEBUGONLY(crazyMonsterPrintouts) == 1){
	easyPrintLine("CanAttack1? %d", HasConditions(bits_COND_CAN_MELEE_ATTACK1));
	}
	if ( !FVisible( pEnemy ) )
	{
		ASSERT(!HasConditions(bits_COND_SEE_ENEMY));
		SetConditions( bits_COND_ENEMY_OCCLUDED );
	}
	else{
		ClearConditions( bits_COND_ENEMY_OCCLUDED );
	}
	
	if(EASY_CVAR_GET_DEBUGONLY(crazyMonsterPrintouts) == 1){
	easyPrintLine("CanAttack2? %d", HasConditions(bits_COND_CAN_MELEE_ATTACK1));
	}
	
	
	//FLAG 666

	//easyPrintLine("HEY WHO IS THAT ENEMY!? %s::%d %d", pEnemy->getClassname(), pEnemy->IsAlive(), pEnemy->pev->deadflag);
	//m_hEnemy!=NULL?easyPrintLine("HEY WHO IS MY ENEMY!? %s::%d %d", m_hEnemy->getClassname(), m_hEnemy->IsAlive(), m_hEnemy->pev->deadflag):easyPrintLine("NOOEEE");
	
		//IsAlive_FromAI takes "this" monster as a parameter. It already knows what itself is.

	//if(FClassnameIs(pev, "monster_barney")){
	//	int x = 666;
	//}



	/*
	if(monsterID == 9){
		BOOL tempp1 = pEnemy->IsAlive_FromAI(this);
		BOOL tempp2 = pEnemy->IsAlive();

		int daDeadflag = pEnemy->pev->deadflag;

		easyForcePrintLine("$$$FLAG3: is my enemy dead? fromAI:%d real:%d deadflag:%d", tempp1, tempp2, daDeadflag);
	}
	*/

	//Why does this wait to count an enemy as dead, but
	//bits_COND_SEE_ENEMY sure happens instantly the moment of death?
	if ( !pEnemy->IsAlive_FromAI(this) )
	{

		if(monsterID == 9){
			int x = 45;
		}

		//easyForcePrintLine("DO IT BE THO %d : %.2f", (pEnemy->pev->deadflag, pEnemy->pev->frame) );

		//MODDD - new event, called when the checked enemy is dead
		// (as bits_COND_ENEMY_DEAD is set)
		onEnemyDead(pEnemy);
		
		SetConditions ( bits_COND_ENEMY_DEAD );
		ClearConditions( bits_COND_SEE_ENEMY | bits_COND_ENEMY_OCCLUDED );
		return FALSE;
	}

	Vector vecEnemyPos = pEnemy->pev->origin;
	// distance to enemy's origin
	flDistToEnemy = ( vecEnemyPos - pev->origin ).Length();
	vecEnemyPos.z += pEnemy->pev->size.z * 0.5;

	// distance to enemy's head
	if(EASY_CVAR_GET_DEBUGONLY(crazyMonsterPrintouts) == 1){
		easyPrintLine("CanAttack3? %d", HasConditions(bits_COND_CAN_MELEE_ATTACK1));
	}

	float flDistToEnemy2 = (vecEnemyPos - pev->origin).Length();
	if (flDistToEnemy2 < flDistToEnemy)
		flDistToEnemy = flDistToEnemy2;
	else
	{
		// distance to enemy's feet
		vecEnemyPos.z -= pEnemy->pev->size.z;
		flDistToEnemy2 = (vecEnemyPos - pev->origin).Length();
		if (flDistToEnemy2 < flDistToEnemy)
			flDistToEnemy = flDistToEnemy2;
	}
	
	if(EASY_CVAR_GET_DEBUGONLY(crazyMonsterPrintouts) == 1){
		easyPrintLine("CanAttack4? %d", HasConditions(bits_COND_CAN_MELEE_ATTACK1));
	}

	if ( HasConditions( bits_COND_SEE_ENEMY ) )
	{
		CBaseMonster *pEnemyMonster;

		iUpdatedLKP = TRUE;
		
		//MODDD - involve the entity.
		setEnemyLKP(pEnemy);

		pEnemyMonster = pEnemy->MyMonsterPointer();

		if ( pEnemyMonster )
		{
			if ( pEnemyMonster->FInViewCone ( this ) )
			{
				SetConditions ( bits_COND_ENEMY_FACING_ME );
			}
			else{
				ClearConditions( bits_COND_ENEMY_FACING_ME );
			}
		}

		if (pEnemy->pev->velocity != Vector( 0, 0, 0))
		{
			// trail the enemy a bit
			//MODDD - involve the entity
			// ...WAIT.  The LKP was already set with the entity in mind above (setEnemyLKP(pEnemy)).
			// Go ahead and shift this plainly.
			//setEnemyLKP(m_vecEnemyLKP - pEnemy->pev->velocity * RANDOM_FLOAT( -0.05, 0 ));
			//setEnemyLKP(m_vecEnemyLKP, pEnemy->pev->mins.z, pEnemy->pev->velocity * RANDOM_FLOAT( -0.05, 0 ));
			setEnemyLKP(pEnemy, pEnemy->pev->velocity * RANDOM_FLOAT( -0.05, 0 ));
		}
		else
		{
			// UNDONE: use pev->oldorigin?
		}
	}
	

	//MODDD - POINT OF FRUSTRATION
	//MODDD - alteration.
	//else if ( !HasConditions(bits_COND_ENEMY_OCCLUDED|bits_COND_SEE_ENEMY) && ( flDistToEnemy <= 256 ) )
	else if ( !HasConditions(bits_COND_ENEMY_OCCLUDED|bits_COND_SEE_ENEMY) && ( flDistToEnemy <= EASY_CVAR_GET_DEBUGONLY(monsterAIForceFindDistance) ) )
	{
		// if the enemy is not occluded, and unseen, that means it is behind or beside the monster.
		// if the enemy is near enough the monster, we go ahead and let the monster know where the
		// enemy is. 
		
		iUpdatedLKP = TRUE;
		//MODDD - involve the ent
		setEnemyLKP(pEnemy);
	}
	
	if(EASY_CVAR_GET_DEBUGONLY(crazyMonsterPrintouts) == 1){
	easyPrintLine("CanAttack5? %d", HasConditions(bits_COND_CAN_MELEE_ATTACK1));
	}
	if ( flDistToEnemy >= m_flDistTooFar )
	{
		// enemy is very far away from monster
		SetConditions( bits_COND_ENEMY_TOOFAR );
	}
	else
		ClearConditions( bits_COND_ENEMY_TOOFAR );

	if(EASY_CVAR_GET_DEBUGONLY(crazyMonsterPrintouts) == 1){
	easyPrintLine("Can I Check Attacks? %d", FCanCheckAttacks());
	}


	if(EASY_CVAR_GET_DEBUGONLY(crazyMonsterPrintouts) == 1)easyForcePrintLine("ALRIGHT HOTSHOT %d %d", HasConditions(bits_COND_SEE_ENEMY), !HasConditions( bits_COND_ENEMY_TOOFAR ) );
	
	if ( FCanCheckAttacks() )	
	{
		//MODDD - what?  Why sending m_hEnemy instead of pEnemy, the one given to this method?
		// Although CheckEnemy is only ever called with m_hEnemy filling pEnemy.
		//CheckAttacks ( m_hEnemy, flDistToEnemy );
		CheckAttacks(pEnemy, flDistToEnemy);
		

		//MODDD - test?

		//if(monsterID==1){ easyForcePrintLine("WOO %d", HasConditions(bits_COND_CAN_RANGE_ATTACK2));  }


	}else{
		//MODDD MAJOR - if unable to check attacks, assume they would have failed anyways. Don't keep memory of attack conditions that now survive schedule changes.
		ClearConditions( bits_COND_CAN_RANGE_ATTACK1 | bits_COND_CAN_RANGE_ATTACK2 | bits_COND_CAN_MELEE_ATTACK1 |bits_COND_CAN_MELEE_ATTACK2 );
		ClearConditionsMod(bits_COND_COULD_RANGE_ATTACK1 | bits_COND_COULD_RANGE_ATTACK2 | bits_COND_COULD_MELEE_ATTACK1 | bits_COND_COULD_MELEE_ATTACK2);
	}

	// the smart pathfinding method does not use this
	if ( !disableEnemyAutoNode && m_movementGoal == MOVEGOAL_ENEMY )
	{
		//MODDD - use m_iRouteLength instead of ROUTE_SIZE
		for ( int i = m_iRouteIndex; i < m_iRouteLength; i++ )
		{
			//MODDD
			//if ( m_Route[ i ].iType == (bits_MF_IS_GOAL|bits_MF_TO_ENEMY) )
			// This means, the two bits MUST be within iType. Not only one or the other.
			if ( (m_Route[ i ].iType & (bits_MF_IS_GOAL|bits_MF_TO_ENEMY)) == (bits_MF_IS_GOAL|bits_MF_TO_ENEMY) )
			{
				//MODDD - TODO - should this involve goalDistTolerance?  Then again most modern pathfinding, if not all,
				// skips this area completey anyway

				// UNDONE: Should we allow monsters to override this distance (80?)
				if ( (m_Route[ i ].vecLocation - m_vecEnemyLKP).Length() > 80 )
				{
					// Refresh
					FRefreshRoute();
					return iUpdatedLKP;
				}
			}
		}
	}
	
	if(EASY_CVAR_GET_DEBUGONLY(crazyMonsterPrintouts) == 1){
	easyPrintLine("CanAttack6 %d", HasConditions(bits_COND_CAN_MELEE_ATTACK1));
	}

	return iUpdatedLKP;
}





//Remove any empty spaces from the stack and push anything to the right to the left to fill the space.
//ex: say we have these enemis in m_hOldEnemey:
//	[0]		[1]		[2]		[3]
//	alive	dead	alive	alive
// the [1], being dead, should be removed and replaced by [2]. Then [2] gets replaced by 3.
//Or,
//	[0]		[1]		[2]		[3]
//	[0]		[2]		[3]		N/A
//	alive	alive	alive	N/A
//(also reducing m_intOldEnemeyNextIndex)
void CBaseMonster::refreshStack() {

	//minus 1, because m_intOldEnemyNextIndex is the place to put a new enemy. It itself, is not the most recently added enemy. But one index less, is.
	//This is also why it must be above 0 to have anything (index of 0 - 1 = -1, meaning empty. index of 2 - 1 = 1, meaning the most recent addition is at #1).

	for(int i = m_intOldEnemyNextIndex - 1; i >= 0; i--){

		//IsAlive_FromAI takes "this" monster as a parameter. It already knows what itself is.
		if (m_hOldEnemy[i] != NULL && m_hOldEnemy[i]->IsAlive_FromAI(this))
		{
			//this is okay.
		}
		else {
			m_hOldEnemy[i] = NULL;
			//do any entries above me need to be pushed down?
			//such as, say there are three enemies, m_intOldEnemyNextIndex is 3, and indexes [0], [1], and [2] are used of m_hOldEnemy.
			//If [1] is dead and removed, [1] is set to NULL.  Shouldn't [2] move to [1]? And if there were a [3], that should replace the blank space of [2].

			for (int i2 = i + 1; i2 < m_intOldEnemyNextIndex; i2++) {
				m_hOldEnemy[i2 - 1] = m_hOldEnemy[i2];
				m_vecOldEnemy[i2 - 1] = m_vecOldEnemy[i2];
				m_flOldEnemy_zOffset[i2 - 1] = m_flOldEnemy_zOffset[i2];
				m_vecOldEnemy_ViewOFS[i2 - 1] = m_vecOldEnemy_ViewOFS[i2];
			}
			//clearly, one less enemy to remember.
			m_intOldEnemyNextIndex--;
		}
	}//END OF for

}



 //=========================================================
 // PushEnemy - remember the last few enemies, always remember the player
 //=========================================================
void CBaseMonster::PushEnemy( CBaseEntity *pEnemy, Vector &vecLastKnownPos )
{
	int i;

	if (pEnemy == NULL)
		return;


	//MODDD - NEW. Is it ok to deny if the currently suggested addition is the current enemy (m_hEnemy) already? It is possible this never happens.
	if(pEnemy == m_hEnemy){
		//no need to say this every time now.
		//easyForcePrintLine("%s:ID%d !!!PushEnemy: denied adding the current enemy to the stack! %s", this->getClassnameShort(), this->monsterID, m_hEnemy->getClassnameShort());
		return;
	}


	// UNDONE: blah, this is bad, we should use a stack but I'm too lazy to code one.
	//MODDD - I'll give you a stack! Or the idea at least (pick the most recent additon, not necessarily #0 just because it's an ok option)
	/*
	for (i = 0; i < MAX_OLD_ENEMIES; i++)
	{
	if (m_hOldEnemy[i] == pEnemy)
	return;
	if (m_hOldEnemy[i] == NULL) // someone died, reuse their slot
	break;
	}
	if (i >= MAX_OLD_ENEMIES)
	return;
	*/



	refreshStack();



	//a stack just needs to see the most recent position. Defaults to 0 of course.
	//It is actualy a sign of the next "target" index to fill. being 0 means index "0" is ready to fill. Being index 4 (MAX_OLD_ENEMIES at the time of writing) exactly means it is full.
	if (m_intOldEnemyNextIndex < MAX_OLD_ENEMIES) {
		

		
		//first a check. Is this enemy already in the list?
		for(i = 0; i < m_intOldEnemyNextIndex; i++){
			if(m_hOldEnemy[i] == pEnemy){
				//Already in the list. Do not add.
				return;
			}
		}


		m_hOldEnemy[m_intOldEnemyNextIndex] = pEnemy;
		m_vecOldEnemy[m_intOldEnemyNextIndex] = vecLastKnownPos;
		m_flOldEnemy_zOffset[m_intOldEnemyNextIndex] = pEnemy->pev->mins.z;
		m_vecOldEnemy_ViewOFS[m_intOldEnemyNextIndex] = pEnemy->pev->view_ofs;

		m_intOldEnemyNextIndex++;
		return;
	}else{
		//no room. Deny adding to my memory.
		return;
	}

}

//=========================================================
// PopEnemy - try remembering the last few enemies
//=========================================================
BOOL CBaseMonster::PopEnemy()
{
	// UNDONE: blah, this is bad, we should use a stack but I'm too lazy to code one.
	//MODDD - I'll give you a stack! Or the idea at least (pick the most recent additon, not necessarily #0 just because it's an ok option)
	/*
	for (int i = MAX_OLD_ENEMIES - 1; i >= 0; i--)
	{
	if (m_hOldEnemy[i] != NULL)
	{
	if (m_hOldEnemy[i]->IsAlive( )) // cheat and know when they die
	{
	m_hEnemy = m_hOldEnemy[i];
	m_vecEnemyLKP = m_vecOldEnemy[i];
	// ALERT( at_console, "remembering\n");
	return TRUE;
	}
	else
	{
	m_hOldEnemy[i] = NULL;
	}
	}
	}
	*/


	refreshStack();

	if (m_intOldEnemyNextIndex > 0) {

		m_hEnemy = m_hOldEnemy[m_intOldEnemyNextIndex - 1];

		//m_vecEnemyLKP = m_vecOldEnemy[m_intOldEnemyNextIndex];
		// Now, send the recorded zOffset as well
		setEnemyLKP(m_vecOldEnemy[m_intOldEnemyNextIndex], m_flOldEnemy_zOffset[m_intOldEnemyNextIndex]);
		//!!! For now, has to be set manually after:
		m_vecEnemyLKP_ViewOFS = m_vecOldEnemy_ViewOFS[m_intOldEnemyNextIndex];

		m_intOldEnemyNextIndex--;
		return TRUE;
	}

	return FALSE;
}







void CBaseMonster::DrawFieldOfVision(){
	
	//1: +-0 deg.
	//0: +-90 deg.
	//-1: +-180 deg.

	//turn FOV into a number of degrees... or radians:

	//pev->angles.x

	float degShift = m_flFieldOfView*-90 + 90;


	float angForwardRad = (pev->angles.y) * (CONST_DEG_TO_RAD_CONV);

	float fovForwardExtentX = cos(angForwardRad);
	float fovForwardExtentY = sin(angForwardRad);

	Vector vecFovExtentForward = Vector(fovForwardExtentX, fovForwardExtentY, 0) * 500;


	//get the X and Y (top-down perspective) of looking this many degrees away from what way we are facing groundwise. pev->angles.x:
	float angLeftRad = (pev->angles.y - degShift) * (CONST_DEG_TO_RAD_CONV);
	float angRightRad = (pev->angles.y + degShift) * (CONST_DEG_TO_RAD_CONV);

	float fovExtentLeftX = cos(angLeftRad);
	float fovExtentLeftY = sin(angLeftRad);

	Vector vecFovExtentLeft = Vector(fovExtentLeftX, fovExtentLeftY, 0) * 500;
	
	float fovExtentRightX = cos(angRightRad);
	float fovExtentRightY = sin(angRightRad);
	
	Vector vecFovExtentRight = Vector(fovExtentRightX, fovExtentRightY, 0) * 500;
	
	
	//UTIL_MakeVectors ( pev->angles );
	//::UTIL_drawLineFrame(pev->origin, pev->origin + giveZ(gpGlobals->v_forward.Make2D().Normalize(), this->EyePosition().z ) * 500, 12, 0, 0, 255);


	Vector eyePos = pev->origin + pev->view_ofs;
	//pev->view_ofs ?
	::UTIL_drawLineFrame(eyePos, eyePos + vecFovExtentForward, 12, 0, 0, 255);

	::UTIL_drawLineFrame(eyePos, eyePos + vecFovExtentLeft, 12, 0, 255, 0);
	::UTIL_drawLineFrame(eyePos, eyePos + vecFovExtentRight, 12, 0, 255, 0);

}//END OF DrawFieldOfVision




/*
//MODDD - new paramter: forceReset
void CBaseMonster::SetActivity ( Activity NewActivity )
{
	CBaseMonster ::SetActivity(NewActivity, FALSE);
}
*/


//MODDD - new
BOOL CBaseMonster::allowedToSetActivity(void){
	return TRUE;
}

//MODDD - new
int CBaseMonster::tryGetTaskID(void){

	//NOTICE: made "GetTask" check for "m_pSchedule" being null.  It's good over there now.
	/*
	if(m_pSchedule == NULL){
		//Note that "GetTask" does not check for m_pSchedule being Null, despite going thru it.  Possible
		//source of crashes...?
		return -3;
	}
	*/

	Task_t* taskAttempt = GetTask();
	if(taskAttempt == NULL){
		return -2;
	}else{
		return taskAttempt->iTask;
	}
}

const char* CBaseMonster::tryGetScheduleName(void){
	if(m_pSchedule == NULL){
		return "NULL!";
	}else{
		return m_pSchedule->pName;
	}
}



//=========================================================
// SetActivity 
//=========================================================
//MODDD- VIRTUAL
void CBaseMonster::SetActivity ( Activity NewActivity )
{
	if(monsterID == 0){
		int x = 6;
	}

	
	//MODDD TODO - on any loosely linked clones of SetActivity, they should also set "signalActivityUpdate" to false.
	// This method getting called is supposed to be enough to satisfy the signal, like to force resetting to itself after a possible
	// raw anim change.
	signalActivityUpdate = FALSE;

	
	if(this->allowedToSetActivity() == TRUE){

	}else{
		//NO.
		return;
	}


	int iSequence;

	//nah...
	//easyPrintLine("SMOOTH AS DER %d %d %d", HasConditions(bits_COND_CAN_MELEE_ATTACK1), m_Activity, m_IdealActivity );

	//MODDD - possible insertion.
	if(usesAdvancedAnimSystem()){
		iSequence = LookupActivityHard ( NewActivity );
	}else{
		iSequence = LookupActivity ( NewActivity );
	}

	
	
	//easyForcePrintLine("SET ACTIVITY: %d I DID THIS: %d", NewActivity, iSequence);

	BOOL allowResetFrame = TRUE;
	//easyForcePrintLine("OK SO? %d==%d : %d", pev->sequence, iSequence, doNotResetSequence);
	if (pev->sequence == iSequence && doNotResetSequence) {
		allowResetFrame = FALSE;
	}

	// Set to the desired anim, or default anim if the desired is not present
	if ( iSequence > ACTIVITY_NOT_AVAILABLE )
	{
		//BOOL sinceLoopMem = m_fSequenceFinishedSinceLoop;

		//MODDD - added "forceReset"... NO, REVERTED.
		//if ( forceReset || (pev->sequence != iSequence || !m_fSequenceLoops) )
		//if (  pev->sequence != iSequence || !m_fSequenceLoops )
		if (allowResetFrame && (pev->sequence != iSequence || !m_fSequenceLoops) )
		{
			// don't reset frame between walk and run
			if (!(m_Activity == ACT_WALK || m_Activity == ACT_RUN) || !(NewActivity == ACT_WALK || NewActivity == ACT_RUN)) {
				resetFrame();
			}

			// If a difference sequence or the previous one didn't loop,  go ahead and reset this.
			// or... not here at least.
			//sinceLoopMem = FALSE;
		}

		
		//easyPrintLine("ANIMATION CHANGE!!!! C %d");
		pev->sequence		= iSequence;	// Set to the reset anim (if it's there)

		//easyPrintLine("$ASS$ %d : %d",  NewActivity, forceReset );
		

		ResetSequenceInfo( );
		//pev->frame = 0;
		//pev->framerate = 2;
		m_fSequenceFinished = FALSE;
		m_fSequenceFinishedSinceLoop = FALSE;


		if (!allowResetFrame) {
			// keep this set then.
			doNotResetSequence = FALSE;
		}


		usingCustomSequence = FALSE;  //MODDD - automatically picked.
		SetYawSpeed();
	}
	else
	{
		// Not available try to get default anim
		ALERT ( at_aiconsole, "%s has no sequence for act:%d\n", STRING(pev->classname), NewActivity );
		
		pev->sequence		= 0;	// Set to the reset anim (if it's there)
		usingCustomSequence = FALSE;  //MODDD - automatically picked.
	}

	m_Activity = NewActivity; // Go ahead and set this so it doesn't keep trying when the anim is not present
	
	// In case someone calls this with something other than the ideal activity
	m_IdealActivity = m_Activity;

}//SetActivity



//MODDD - script that used to be in MonsterThink for what to do when an idle animation hit the last frame.
// Picks a new sequence for the current m_Activity (does not change the activity).
void CBaseMonster::RefreshActivity(void){
	int iSequence;

	if ( m_fSequenceLoops )
	{
		// animation does loop, which means we're playing subtle idle. Might need to 
		// fidget.
		if(EASY_CVAR_GET_DEBUGONLY(animationPrintouts) == 1 && monsterID >= -1)easyForcePrintLine("%s:%d Anim info IDLE RESET #1? frame:%.2f done:%d", getClassname(), monsterID, pev->frame, m_fSequenceFinished);

		if(usesAdvancedAnimSystem()){
			iSequence = LookupActivityHard ( m_Activity );
		}else{
			iSequence = LookupActivity ( m_Activity );
		}
			
	}
	else
	{
		if(EASY_CVAR_GET_DEBUGONLY(animationPrintouts) == 1 && monsterID >= -1)easyForcePrintLine("%s:%d Anim info IDLE RESET #2? frame:%.2f done:%d", getClassname(), monsterID, pev->frame, m_fSequenceFinished);

		// animation that just ended doesn't loop! That means we just finished a fidget
		// and should return to our heaviest weighted idle (the subtle one)

		if(usesAdvancedAnimSystem()){
			iSequence = LookupActivityHard ( m_Activity );
		}else{
			iSequence = LookupActivityHeaviest ( m_Activity );
		}

	}

	if ( iSequence != ACTIVITY_NOT_AVAILABLE )
	{
		int oldSequence = pev->sequence;
		//BOOL resetFrameYet = FALSE;

		// don't reset sinceLoopMem, that is remembered through this type of change.
		BOOL sinceLoopMem = m_fSequenceFinishedSinceLoop;
			
		//easyPrintLine("ANIMATION CHANGE!!!! B");
		pev->sequence = iSequence;	// Set to new anim (if it's there)
		ResetSequenceInfo( );

		m_fSequenceFinishedSinceLoop = sinceLoopMem;


		//MODDD IMPORTANT. Go ahead and let the system know this sequence finshed at least once, sometimes that matters. Even a replacement different idle sequence.
		//m_fSequenceFinishedSinceLoop = TRUE;
		// NO NOT HERE.  Leave it to animating.cpp, probably

			
		//MODDD - why wasn't this here before?!  Why do we assume the new sequence will reset the frame? We sure don't know that?
		//        Even without forceIdleFrameReset.
		if(forceIdleFrameReset() || !(iSequence == oldSequence && this->m_fSequenceLoops) ){
			//if this is the same sequence and it loops, no need to reset.
			resetFrame();
		}
	}
}//RefreshActivity







//Notice that the "ForceLoops" variants below are completely different from m_iForceLoops. 
//If m_iForceLoops is anything but -1, it still loses to the provided forceLoops parameter, but
//any animations after will still be affected by m_iForceLoops if non-negative instead.

void CBaseMonster::SetSequenceByIndex(int iSequence)
{
	SetSequenceByIndex(iSequence, FALSE);
}
void CBaseMonster::SetSequenceByName(char* szSequence)
{
	SetSequenceByName(szSequence, FALSE);
}


void CBaseMonster::SetSequenceByIndex(int iSequence, float flFramerateMulti)
{
	m_flFramerateSuggestion = flFramerateMulti;
	SetSequenceByIndex(iSequence, FALSE);
}
void CBaseMonster::SetSequenceByName(char* szSequence, float flFramerateMulti)
{
	m_flFramerateSuggestion = flFramerateMulti;
	SetSequenceByName(szSequence, FALSE);
}

//MODDD - if we have a hardcoded number (index; offset from 0, the first sequence in the model), just use that.
void CBaseMonster::SetSequenceByIndex(int iSequence, BOOL safeReset)
{
	// Set to the desired anim, or default anim if the desired is not present
	if ( iSequence > ACTIVITY_NOT_AVAILABLE )
	{
		if ( pev->sequence != iSequence || !m_fSequenceLoops )
		{
			resetFrame();
		}

		pev->sequence		= iSequence;	// Set to the reset anim (if it's there)
		
		//easyForcePrintLine("SetSequenceByIndex: ses:%d fs:%.2f", iSequence, m_flFramerateSuggestion);
		if(!safeReset){
			ResetSequenceInfo( );
		}else{
			ResetSequenceInfoSafe();
		}
		usingCustomSequence = TRUE;
		SetYawSpeed();
	}
	else
	{
		// Not available try to get default anim
		//MODDD - why is %f the 2nd arg here?  isn't that for floats, not strings?  Disabling this line.
		//ALERT ( at_aiconsole, "%s has no sequence named:%f\n", STRING(pev->classname), szSequence );
		pev->sequence		= 0;	// Set to the reset anim (if it's there)
		usingCustomSequence = FALSE;
	}
}//END OF SetSequenceByNumber()
void CBaseMonster::SetSequenceByName(char* szSequence, BOOL safeReset)
{
	int iSequence;
	iSequence = LookupSequence ( szSequence );
	SetSequenceByIndex(iSequence, safeReset);
}

void CBaseMonster::SetSequenceByIndex(int iSequence, float flFramerateMulti, BOOL safeReset)
{
	m_flFramerateSuggestion = flFramerateMulti;
	SetSequenceByIndex(iSequence, safeReset);
}
void CBaseMonster::SetSequenceByName(char* szSequence, float flFramerateMulti, BOOL safeReset)
{
	m_flFramerateSuggestion = flFramerateMulti;
	SetSequenceByName(szSequence, safeReset);
}







//MODDD - call the above methods, but force the "m_fSequenceLoops" flag to a provided value (true / false).
void CBaseMonster::SetSequenceByIndexForceLoops(int iSequence, BOOL forceLoops)
{
	SetSequenceByIndex(iSequence, FALSE);
	m_fSequenceLoops = forceLoops;
}

void CBaseMonster::SetSequenceByNameForceLoops(char* szSequence, BOOL forceLoops)
{
	SetSequenceByName(szSequence, FALSE);
	m_fSequenceLoops = forceLoops;
}

void CBaseMonster::SetSequenceByIndexForceLoops(int iSequence, float flFramerateMulti, BOOL forceLoops)
{
	m_flFramerateSuggestion = flFramerateMulti;
	SetSequenceByIndex(iSequence, FALSE);
	m_fSequenceLoops = forceLoops;
}

void CBaseMonster::SetSequenceByNameForceLoops(char* szSequence, float flFramerateMulti, BOOL forceLoops)
{
	m_flFramerateSuggestion = flFramerateMulti;
	SetSequenceByName(szSequence, FALSE);
	m_fSequenceLoops = forceLoops;
}

void CBaseMonster::SetSequenceByIndexForceLoops(int iSequence, BOOL safeReset, BOOL forceLoops)
{
	SetSequenceByIndex(iSequence, safeReset);
	m_fSequenceLoops = forceLoops;
}

void CBaseMonster::SetSequenceByNameForceLoops(char* szSequence, BOOL safeReset, BOOL forceLoops)
{
	SetSequenceByName(szSequence, safeReset);
	m_fSequenceLoops = forceLoops;
}

void CBaseMonster::SetSequenceByIndexForceLoops(int iSequence, float flFramerateMulti, BOOL safeReset, BOOL forceLoops)
{
	m_flFramerateSuggestion = flFramerateMulti;
	SetSequenceByIndex(iSequence, safeReset);
	m_fSequenceLoops = forceLoops;
}

void CBaseMonster::SetSequenceByNameForceLoops(char* szSequence, float flFramerateMulti, BOOL safeReset, BOOL forceLoops)
{
	m_flFramerateSuggestion = flFramerateMulti;
	SetSequenceByName(szSequence, safeReset);
	m_fSequenceLoops = forceLoops;
}










//MODDD - new method.  Return if an alternative to the incremental process of "WALK_MOVE" should be used instead.
BOOL CBaseMonster::getHasPathFindingModA(){
	return FALSE; //by default, false.
}
BOOL CBaseMonster::getHasPathFindingMod(){
	return FALSE; //by default, false.
}



//=========================================================
// CheckLocalMove - returns TRUE if the caller can walk a 
// straight line from its current origin to the given 
// location. If so, don't use the node graph!
//
// if a valid pointer to a int is passed, the function
// will fill that int with the distance that the check 
// reached before hitting something. THIS ONLY HAPPENS
// IF THE LOCAL MOVE CHECK FAILS!
//
// !!!PERFORMANCE - should we try to load balance this?
// DON"T USE SETORIGIN! 
//=========================================================
//MODDD - SHUN!    was 16
#define LOCAL_STEP_SIZE	16
#define LOCAL_STEP_SIZE_MOD 10

//MODDD - ref
EASY_CVAR_EXTERN_DEBUGONLY(drawDebugPathfinding)


static const float stepChoiceArray[] = {LOCAL_STEP_SIZE, LOCAL_STEP_SIZE_MOD};


int CBaseMonster::CheckLocalMoveHull(const Vector &vecStart, const Vector &vecEnd, CBaseEntity *pTarget, float *pflDist  )
{
	//MODDD - todo at some point. Is that a good idea... always TRUE here??
	return LOCALMOVE_VALID;

	
	TraceResult tr;

	UTIL_TraceHull( vecStart + Vector( 0, 0, 32), vecEnd + Vector( 0, 0, 32), dont_ignore_monsters, large_hull, edict(), &tr );
	//UTIL_TraceHull( vecStart + Vector( 0, 0, 32), vecEnd + Vector( 0, 0, 32), dont_ignore_monsters, head_hull, edict(), &tr );

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



//MODDD - NEW.  Accepts whether to do the Z-check at the end on a successful route (can change its mind
// if the intended end point is too high/low of where WALK_MOVE against the ground put me)
// Also. For now, assume 'doZCheck' being on means this is to an important point (the goal).
// Being within 'goalDistTolerance' of it 2D-wise and passing the Z check means we can call this route
// 'good enough' at where WALK_MOVE was blocked at anyway.

// Unfortunately, there are some times when USE_MOVEMENT_BOUND_FIX_CHKLOC/ALT can allow a route that shouldn't be possible,
// but it is very very rare.  It lets going up/down ramps work far better for larger entities (they re-do the 
// WALK_MOVE call with a HUMAN_HULL size, which has an easier time working with elevation changes for whatever reason).
// Unsure how to detect a WALK_MOVE failure coming from colliding with nearby map geometry properly (like running into a 
// wall it's too big to go through in a straight-line path), instead of not liking going up an incline it can move up
// perfectly fine.
// One good compromise is to disable the fix for MoveExecute, or else moving while the enemy is close can lead to
// clipping issues.  It moves forward while its bounds are temp-set to the HUMAN size, and expands back to normal bounds
// after the move in that case, although that isn't a 'pretend' scenario like CheckLocalMove; it actually happens and
// leaves both things stuck in each other.  Not great.

// Lastly, 'pflDist' is optional, and, if provided, receives the distance the monster could travel before stopping
// to the nearest step (16) interval, won't be more exact than that.
// If it isn't interrupted by anything and makes the full trip, regardless of failing the Z check later, it doesn't
// get written, not even with 0.  Keep that in mind.
// This seems strange it sounds like the assumption would be that pflDist is supposed to be read only if this call
// returned anything but LOCALMOVE_SUCCESS, even though it can fail without writing to pflDist.
// CHANGED, look through any references to CheckLocalMove, always safe to write out the flDist in case
// it never would've been written to a single time.

//=========================================================
// CheckLocalMove - returns TRUE if the caller can walk a 
// straight line from its current origin to the given 
// location. If so, don't use the node graph!
//
// if a valid pointer to a int is passed, the function
// will fill that int with the distance that the check 
// reached before hitting something. THIS ONLY HAPPENS
// IF THE LOCAL MOVE CHECK FAILS!
//
// !!!PERFORMANCE - should we try to load balance this?
// DON"T USE SETORIGIN! 
//=========================================================
#define LOCAL_STEP_SIZE	16
int CBaseMonster::CheckLocalMove ( const Vector &vecStart, const Vector &vecEnd, CBaseEntity *pTarget, BOOL doZCheck, float *pflDist ){
	Vector vecStartPos;// record monster's position before trying the move
	float flYaw;
	float flDist;
	float flStep;
	float stepSize;
	int	iReturn;
#if defined(USE_MOVEMENT_BOUND_FIX_CHKLOC) || defined(USE_MOVEMENT_BOUND_FIX_CHKLOC_ALT)
	Vector oldMins;
	Vector oldMaxs;
#endif

	vecStartPos = pev->origin;
	

	// NOTICE!  If this is a movetype step (might be a safe assumption given this method isn't overridden to be wildly different?),
	// snap vecEnd to the ground first.  Cases of the monster clearly reaching the goal but the towards point being 'too high' even
	// though it was just placed above the ground are irritating.
	
	// NEVERMIND.  Always use vecStart and vecEnd.
	// The main issue is only accuracy to the Z check at the end, being properly snapped to the ground has no bearing on WALK_MOVE's
	// success after all, it's moreso used only for yaw purposes (how WALK_MOVE travels from its start point Z-wise).
	// It is better to skip z-checks if they're partial checks (myOrigin + directionToGoal * 300), since there is no guarantee the Z
	// of the target point will be on the ramp, it could even go through the ground that's passed over to reach there, often happens
	// if the start or end point is deep behind the upward point of a ramp.  Being able to WALK_MOVE to this point at all is enough.

	// BUT.  If CheckLocalMove is being used to see if a straight-line path to the goal itself, the Z check should be involved.
	// The point of the Z check is to see that, after WALK_MOVE'ing to the destination, the target entity would be close enough for
	// this route to have been worthwhile.  Reaching the point and going 'oh, that entity is still too high/low for me to do anything',
	// or underneath/above where the entity is standing completely, is not good.

	// ALSO.  If the snap-to-ground approach were to be used again,  check for tr.fStartSolid or tr.fAllSolid, I forget if only
	// either is necessary.  Wait, no, use that "UTIL_PointContents(vec)" or whats-its-name to tell if the given vecEnd is
	// going through the map.  If so, it would make more sense to rise up and look for where the ground is instead.
	// Going down will likely return a point 100 lower and still in the map, since the trace ends at a change in 'in/out of map',
	// so it works like penetration when starting inside the map. The point hit, if any, is more like an 'exit point' for the trace.

	Vector vecStartFiltered;  // piggyback off the DROP_TO_FLOOR call below.  Or don't.
	Vector vecEndFiltered;

	/*
	
	if(this->isMovetypeFlying()){
		// no change?
		vecEndFiltered = vecEnd;
	}else{
		// snap it.
		TraceResult tr;
		Vector vecEndMod = vecEnd;
		vecEndMod.z -= 100;
		UTIL_TraceLine(vecEnd, vecEndMod, ignore_monsters, ENT(pev), &tr);

		if( tr.fStartSolid == 0 && tr.fAllSolid == 0){
			// ok
		}else{
			// ???? should not happen?
			int x = 45;
		}

		vecEndFiltered = tr.vecEndPos;
	}
	*/
	vecStartFiltered = vecStart;
	vecEndFiltered = vecEnd;



	// move the monster to the start of the local move that's to be checked.
	UTIL_SetOrigin( pev, vecStart );// !!!BUGBUG - won't this fire triggers? - nope, SetOrigin doesn't fire


	//MODDD - note.  Oh.  That's another way to check for flyer-types.  Ever a contradiction between pev->flags FL_TLY|FL_SWIM and isMovetypeFlying?
	// seems those should match (either, both or neither be present, never only either.)
	// Suppose there is one question:  Does a stukabat lose/get these flags depending on whether it is on the ground or flying?
	// I imagine the pev->movetype check would be more accurate to how it is currently moving, not just belonging to a class of inherent fliers.
	// Changing for now.
	//if ( !(pev->flags & (FL_FLY|FL_SWIM)) )
	if(!this->isMovetypeFlying())
	{
		DROP_TO_FLOOR( ENT( pev ) );//make sure monster is on the floor!
	}


	/*
	//MODDD - used to be above the 'UTIL_SetOrigin' call above.  Moved here so that vecStartFiltered can be set to
	// where the origin is now (ent to test snapped to the ground after being moved to vecStart).
	if(this->isMovetypeFlying()){
		// no change
		vecStartFiltered = vecStart;
	}else{
		// use the form snapped to the ground now
		//vecStartFiltered = pev->absmin.z
		vecStartFiltered = pev->origin;
	}
	*/

	// as-is, mostly
	//////////////////////////////////////////////////////////////////////////////////////////////////
	Vector pointDelta = vecEndFiltered - vecStartFiltered;
	flYaw = UTIL_VecToYaw ( pointDelta);// build a yaw that points to the goal.
	flDist = ( pointDelta ).Length2D();// get the distance.
	iReturn = LOCALMOVE_VALID;// assume everything will be ok.
	//////////////////////////////////////////////////////////////////////////////////////////////////

	//MODDD - NEW.  Write out the dist in case nothing ever gets in the way of the ent
	// (clearly means the full distance could be crossed).
	if ( pflDist != NULL ){
		*pflDist = flDist;
	}




	//pev->origin.z = vecStartPos.z;//!!!HACKHACK

//	pev->origin = vecStartFiltered;

/*
	if ( flDist > 1024 )
	{
		// !!!PERFORMANCE - this operation may be too CPU intensive to try checks this large.
		// We don't lose much here, because a distance this great is very likely
		// to have something in the way.

		// since we've moved the monster during the check, undo the move.
		pev->origin = vecStartPos;
		return FALSE;
	}
*/
	
#if defined(USE_MOVEMENT_BOUND_FIX_CHKLOC)
	if(needsMovementBoundFix()){
		//Plays better with the engine to do checks with these bounds against ramps. No clue why.
		//UTIL_SetSize( pev, Vector(-30, -30, 0), Vector(30, 30, 40) );

		//record the old bounds for getting them back.
		oldMins = pev->mins;
		oldMaxs = pev->maxs;
		// What?  Why change the bounds so soon?
		//UTIL_SetSize( pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX );
	}
#endif
	
	// records the name of the most recent blocking entity for breakpoints.
	const char* hitClassname = "NULL";
	int hitEntityID = -1;
	int hitMonsterID = -1;
	// placed here for hitpoints to access, but relevant to the logic.
	// If the loop is broken out of, tells whether what was hit matched the target or not.
	// Remains false if nothing was hit, still success
	BOOL hitTargetException = FALSE;



	// this loop takes single steps to the goal.
	for ( flStep = 0 ; flStep < flDist ; flStep += LOCAL_STEP_SIZE ){
		Vector oldOrigin;
		stepSize = LOCAL_STEP_SIZE;

		if ( (flStep + LOCAL_STEP_SIZE) >= (flDist-1) )
			stepSize = (flDist - flStep) - 1;
		
//		UTIL_ParticleEffect ( pev->origin, g_vecZero, 255, 25 );

		
		oldOrigin = pev->origin;

		BOOL walkMoveResult = WALK_MOVE(ENT(pev), flYaw, stepSize, WALKMOVE_CHECKONLY);

		/*
		Vector originNow = pev->origin;
		Vector pointDelta = (originNow - oldOrigin);
		float moveDisto = DistanceFromDelta(pointDelta);
		float moveDisto2D = Distance2DFromDelta(pointDelta);

		easyForcePrintLine("heyyy %.2f %.2f", moveDisto, moveDisto2D);
		*/


		if ( !walkMoveResult )
		{// can't take the next step, fail!

#ifdef USE_MOVEMENT_BOUND_FIX_CHKLOC_ALT
			if( EASY_CVAR_GET_DEBUGONLY(pathfindLargeBoundFix) == 1 && needsMovementBoundFix() ){
				// A... NULL trace_ent is valid to take the Instance of?
				// Guess so, leads to the 'World' itself.
				CBaseEntity* whut = CBaseEntity::Instance(gpGlobals->trace_ent);

				if(whut != NULL){
					/*
					easyForcePrintLine("%s:%d MBFclm: %s:%d isW:%d", getClassname(), monsterID, whut->getClassname(), 
						(whut->GetMonsterPointer() != NULL)?whut->GetMonsterPointer()->monsterID:-1
						, whut->IsWorld());
					*/
				}

				if(whut == NULL || whut->IsWorld()){
					// WAIT. You get one more chance.
					// ???  Any why record these here again?  Startup got it right the first time.
					//oldMins = pev->mins;
					//oldMaxs = pev->maxs;

					UTIL_SetSize( pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX );
					pev->origin = oldOrigin;


					if(!WALK_MOVE( ENT(pev), flYaw, stepSize, WALKMOVE_CHECKONLY )){
						// still fail? it is ok to run the script below that accepts failure.
						/*
						UTIL_SetSize(pev, oldMins, oldMaxs);
						// WAIT. Allow below to run in this case or not?  I don't know man!!
						// But slowdowns can happen sometimes from this.  Why??
						continue;
						*/
					}else{
						// success? no, allowed to keep going.
						UTIL_SetSize(pev, oldMins, oldMaxs);
						continue;
					}
					UTIL_SetSize(pev, oldMins, oldMaxs);
				}
			}

#endif

			if ( pflDist != NULL ){
				*pflDist = flStep;
			}

			//const char* daClassname = STRING(gpGlobals->trace_ent->v.classname);
			




			if(gpGlobals->trace_ent != NULL){

				hitTargetException = (pTarget && pTarget->edict() == gpGlobals->trace_ent);

				// NOTICE!  This entire area used to run outside the 'gpGlobals->trace_ent != NULL' check above.
				// Keep that in mind when looking through commented-out script

				if ( hitTargetException ){
					// if this step hits target ent, the move is legal.
					iReturn = LOCALMOVE_VALID;
				}else{

					/*
					// TEST, not a fantatic idea.
					// Now wait just a minute.
					TraceResult temp_tr;
					Vector vecStartah = pev->origin + Vector(0, 0, 2);
					Vector vecEndah = vecStartah + (vecEndFiltered - vecStartFiltered).Normalize() * LOCAL_STEP_SIZE;
					UTIL_TraceLine(vecStartah, vecEndah, dont_ignore_monsters, ENT(pev), &temp_tr);

					if (!temp_tr.fAllSolid && !temp_tr.fStartSolid && temp_tr.flFraction >= 1.0) {
						// yay.
						iReturn = LOCALMOVE_VALID;
						break;
					}
					else {
						// what???
						CBaseEntity* hittTester = CBaseEntity::Instance(temp_tr.pHit);
						const char* classYetAgain = hittTester!=NULL?hittTester->getClassname():"NULL";
						int x = 45;

						UTIL_drawLineFrame(vecStartah, vecEndah, 255, 0, 0);
					}
					*/

					//MODDD - Found commented out as-is.
					// If we're going toward an entity, and we're almost getting there, it's OK.
	//				if ( pTarget && fabs( flDist - iStep ) < LOCAL_STEP_SIZE )
	//					fReturn = TRUE;
	//				else

					iReturn = LOCALMOVE_INVALID;
				}//END OF check for trace-hit matching target

			}else{
				// Didn't hit anything, still failed?  Likely tried going over some impassable change in elevation (cliff?).
				// Or is a NULL trace_ent the world itself regardless (like hitting a BSP wall, not worldbrush entity)?
				// no idea, VERIFY.
				iReturn = LOCALMOVE_INVALID;
			}//gpGlobals->trace_ent NULL check
			



				
			// DEBUG INFO.  Should have no influence on the logic, but it is spammy.
			///////////////////////////////////////////////////////////////////////////////////////////////////
			///////////////////////////////////////////////////////////////////////////////////////////////////
			///////////////////////////////////////////////////////////////////////////////////////////////////

			hitEntityID = -1;
			hitMonsterID = -1;  // unless there is one?

			if(gpGlobals->trace_ent != NULL){
				CBaseEntity* tempEntt = CBaseEntity::Instance(gpGlobals->trace_ent);
				if(tempEntt != NULL){
					hitClassname = tempEntt->getClassname();
					hitEntityID = ENTINDEX(gpGlobals->trace_ent);
					if(tempEntt->pev->flags & FL_MONSTER){
						CBaseMonster* monTest = tempEntt->GetMonsterPointer();
						if(monTest != NULL){
							hitMonsterID = monTest->monsterID;
						}
					}


					if ( hitTargetException ){
						// woohoo

					}else{

						// draw the bounds of the thing that blocked me
						// Way too spammy, need to do this only on MoveExecute calls.  In fact...
						if(g_CheckLocalMoveCanReportBlocker){
							if(this->drawPathConstant){
								easyForcePrintLine("drawpathconstant, %s:%d: There was a %s in the way. Hit entindex:%d monsterID:%d", this->getClassname(), this->monsterID, hitClassname, hitEntityID, hitMonsterID);

								if(!tempEntt->IsWorld()){
									// don't bother drawing the world, no point
									UTIL_drawBoxFrame(tempEntt->pev->absmin, tempEntt->pev->absmax, 12, 300, 255, 255, 255);
								}
							}
						}

						//const char* mahName = testRef->getClassname();
						if( FClassnameIs(gpGlobals->trace_ent, "func_monsterclip")){
							int x = 45;
							//continue; // SKIIIIIIIP IT
						}

					}


				}else{
					hitClassname = "NULL; hit not castable to CBaseEntity?";
				}
			}else{
				hitClassname = "NULL; nothing hit (world?)";
			}


			///////////////////////////////////////////////////////////////////////////////////////////////////
			///////////////////////////////////////////////////////////////////////////////////////////////////
			///////////////////////////////////////////////////////////////////////////////////////////////////


			//DebugLine_Setup(vecStartFiltered, vecEndFiltered, 255, 0, 0);

			// Failing WALK_MOVE always leads to breaking out of the loop that iteration.
			// It was this way too in retail.
			break;
		}//WALK_MOVE check


	}//FOR LOOP through flDist

#if defined(USE_MOVEMENT_BOUND_FIX_CHKLOC)
	if(needsMovementBoundFix()){
		//undo the bound change.
		UTIL_SetSize( pev, oldMins, oldMaxs );
	}
#endif


	//MODDD - below this point, keep in mind that references to 'pev->origin' are to the position of the entity called for CheckLocalMove
	// (the one doing the 'can I get there test) at the end of the pathfind.  It should end very close to the target position, especially
	// floor(2D)-wise in typical success, or the thing hit matches the target entity.


	//MODDD - another check after the above runs:  within goalDistTolerance 2D-wise.
	// Would be better to have its own yes/no var (or bitmask into doZCheck, like require 'doZCheck & 1', but this check needs 'doZCheck & 2'),
	// but for now assuming alongside doZCheck.
	



	float distToGoal2D;
	float distToVecEnd;

	// Do all the time for breakpoints to see, for now
	//distToGoal2D = Distance2D(pev->origin, m_vecMoveGoal);
	//distToGoal2D = Distance2D(pev->origin, m_vecMoveGoal);
	distToVecEnd = Distance2D(pev->origin, vecEndFiltered);


	// UP TO DATE: Nope, all the time but compare to m_vecMoveGoal instead.
	//if(doZCheck){

		// If I got blocked by something that wasn't the target above (or there isn't a target at all to check against),
		// and the goalDistTolerance is set, see if I got close enough 2D-wise.  Force this to a VALID if so.
		// We'll trust other pathfinding knows to stop ahead of time when close enough to the goal 2D-wise, or else this will
		// keep moving against something impassible because it isn't 'done moving yet' and never thought to fail or 'MoveComplete'
		// to get out of movement.

	if(iReturn == LOCALMOVE_INVALID){
		// Also, must already have some goal in mind before allowing this tolerance check.
		// May never be the case otherwise though.
		if(m_movementGoal != MOVEGOAL_NONE && goalDistTolerance > 0){
			//WayPoint_t* waypointGoalRef;
			//waypointGoalRef = GetGoalNode();
			//if(waypointGoalRef != NULL){
				//distToGoal2D = (waypointGoalRef->vecLocation - pev->origin ).Length2D();
				distToGoal2D = (m_vecMoveGoal - pev->origin ).Length2D();
				//float disa2D = Distance2D(pev->origin, vecEndFiltered);
				if(distToGoal2D <= goalDistTolerance){
					// woohoo.  Changed your mind.  The Z check below can still fail this though.
					iReturn = LOCALMOVE_VALID;
				}
			//}
		}//goalDistTolerance
	}//iReturn
	//}





	//MODDD - the real Z-check below changed a bit.  The target not being on the ground shouldn't be important, it's just as 
	// unhittable from being too far above/below my reach.
	// ALSO. Removing the 'LOCALMOVE_VALID' requirement.  This check should happen even if WALK_MOVE failed to reach
	// its point for any other reason already (something in the way that wasn't my target or trying to go over some
	// otherwise impassible area) so that triangulation doesn't happen to try and fool us into thinking this is still
	// a good idea.  Triangulate doesn't use the Z check, and no sense doing that when this point should've realized
	// that once.
	// To be clear, this check has to run to be able to see that the enemy is too far away Z-wise and explicitly forbid
	// trying to triangulate up a better path.  Yes, I said that.

	// IDEA: be between the target ent's position and the vecEnd instead.
	// If vecEnd is close enough to the target's position, this route is clearly
	// leading to that ent itself and so needs this Z check.
	// If this route would end up with me far below the target, do not take it,
	// that would be mad silly.

	// ANOTHER IDEA: If my pTarget is a ground-mover and currently mid-air, snap em' to the ground.
	// After all, why should jumping break where I can/can't go, they'll be on the ground soon enough.
	// Unfortunately this doesn't have an easy answer, I don't know if grabbing the pEntity (player)
	// and doing a DROP_TO_FLOOR in the same frame to see what it would be against the ground would be
	// a good idea, but that would lead to the best answer.  Otherwise linetraces that would still be flimsy:
	// corners?  Stand on a tight-rope and it thinks the payer could snap to underneath that (traces miss the
	// thing blocking the player from hitting the ground).  Same for doing a traceline from the player center
	// to the ground:  stand on the edge of a box as much as possible without falling, center as a straight-
	// line path to the ground but the player is blocked by the box.

	// *** Much of the above is out of date.  The approach below has been replaced with something a lot simpler.


	/*
	//MODDD - NEW.  Skip if told to.
	if(doZCheck){
		
		//if ( iReturn == LOCALMOVE_VALID && 	!(pev->flags & (FL_FLY|FL_SWIM) ) && (!pTarget || (pTarget->pev->flags & FL_ONGROUND)) )
		if(
			//iReturn == LOCALMOVE_VALID &&
			!(pev->flags & (FL_FLY|FL_SWIM) )
			//(!pTarget || (pTarget->pev->flags & FL_ONGROUND)) )
		){
			// The monster can move to a spot UNDER the target, but not to it. Don't try to triangulate, go directly to the node graph.
			// UNDONE: Magic # 64 -- this used to be pev->size.z but that won't work for small creatures like the headcrab
			//MODDD
			// Now anything taller than 64 fails.  UGH.  Fantastic to run into. 
			// Why not involve pev->size, like pev->size * 1.3?  Looks like the big issue with using a small 
			// tolerance for z-dist is just from partial checklocalmove checks (myOrigin + vecDirToGoal * 300)
			// giving too much z-dist from going to a point that's some distance from the terrain, even though
			// the Z in partial checks isn't important, only comparing the goal to the final target does.
			// This is left up to whatever calls CheckLocalMove instead (doZCheck). If we reach here, assume it's important.
			// It does, after all, make sense to forbid a headcrab from trying to reach a point it can't stair-step up or
			// melee attack from.  It jumps to its target anyway.
			//if ( fabs(vecEndFiltered.z - pev->origin.z) > 64 )

			Vector myCenter = Center();
			// How about from my center to the target's center insteaad (if they're close)?
			//float maxZ_DistAllowed = pev->size.z * 0.4 + 6; //pev->size.z * 1.25;
			float maxZ_DistAllowed = pev->size.z * 0.65 + 6; //pev->size.z * 1.25;
			//float zDist = fabs(vecEndFiltered.z - pev->origin.z);
			float zDist;
			Vector vecTarget;
			BOOL forcePass = FALSE;
			BOOL snappedToTargetEnt = FALSE;
			
			if(pTarget != NULL){
				Vector targetCenter = pTarget->Center();
				float theDisto2D = Distance2D(myCenter, targetCenter);

				// was 0.63, 0.63 ?
				if(theDisto2D < this->pev->size.x*0.83 + pTarget->pev->size.x*0.83){
					// Close enough to the target ent?  Go ahead, snap to its center for more accuracy
					vecEndFiltered = targetCenter;
					// !
					vecEndFiltered.z = pTarget->pev->origin.z + pTarget->pev->mins.z; //pTarget->pev->absmin.z;
					snappedToTargetEnt = TRUE;
				}else{
					// normal
				}

			}//pTarget check


			if(!snappedToTargetEnt){
				// If the target ent wasn't nearby or wasn't defined, snap vecEndFiltered
				// to the ground.  That makes the Z comparison more fair.

				// CONTENTS_SOLID?  WATER?  EMPTY?
				int theContents = UTIL_PointContents(vecEndFiltered);

				if(theContents != CONTENTS_SOLID){
					//snap the vecEndFiltered to the ground then.
					TraceResult t_tr;
					Vector t_vecStart = vecEndFiltered;
					Vector t_vecEnd = vecEndFiltered;
					t_vecEnd.z -= 100;
					UTIL_TraceLine(t_vecStart, t_vecEnd, ignore_monsters, ENT(pev), &t_tr);

					vecEndFiltered = t_tr.vecEndPos;

					if(t_tr.flFraction >= 1.0){
						// ????????
						//easyPrintLine("WHGAAGREJGERIGJE %.2f %.2f %.2f", t_tr.vecEndPos.x, t_tr.vecEndPos.y, t_tr.vecEndPos.z);
						//DebugLine_SetupPoint(t_tr.vecEndPos, 255, 0, 0);
					}

				}else{
					// rise above the ground?
					// And uh.  Not going to stop on reaching the exit point?   Yeah yeah uh,  okay then.
					// Oh.  So looking at gauss.cpp, that is indeed the case: trace backwards from this end
					// point to find the exit.   UHhhhhh sure.
					// Only problem is, going too far high could land in the ceiling, which means tracing down
					// to find its exit point too.
					// So, move up in small increments and stop to go back down once a trace isn't AllSolid
					// (ends in mid-air).

					TraceResult t_tr;
					Vector t_vecStart = vecEndFiltered;
					Vector t_vecEnd = vecEndFiltered;

					int tries = 5;

					while(tries > 0){
						t_vecEnd.z += 20;

						UTIL_TraceLine(t_vecStart, t_vecEnd, ignore_monsters, ENT(pev), &t_tr);
						if(t_tr.fAllSolid == 0){
							// reached the open?  Start at that mid-air point instead (t_vecEnd)/
							// and set the end to downard to see where the ground is.
							// Yeah... at least that isn't overcomplicated or anything.
							t_vecStart = t_vecEnd;
							t_vecEnd.z -= 25;
							break;
						}

						t_vecStart.z += 20;

						tries--;
					}

					if(tries == 0){
						// oh.  Ran out of tries, not much else can be done here.
						easyPrintLine("!!! ROUTE DEBUG %s:%d NOTICE!  Could not get point (%.2f,%.2f,%.2f) out of the map into the ground above!", getClassname(), monsterID, vecEndFiltered.x, vecEndFiltered.y, vecEndFiltered.z);
						
						//DebugLine_SetupPoint(1, pev->origin, 255, 0, 255);
						//DebugLine_SetupPoint(2, vecEndFiltered, 0, 255, 0);
						//DebugLine_SetupPoint(3, t_vecEnd, 0, 255, 255);

						// There is moving in the yaw-wise dir of the monster though.  At that point just
						// skip to allowing this anyway.
						forcePass = TRUE;

					}else{
						//UTIL_TraceLine(t_tr.vecEndPos, t_vecEnd, ignore_monsters, ENT(pev), &t_tr);
						UTIL_TraceLine(t_vecStart, t_vecEnd, ignore_monsters, ENT(pev), &t_tr);
						//DebugLine_SetupPoint(3, vecEndFiltered, 0, 255, 255);

						vecEndFiltered = t_tr.vecEndPos;
					}
					
				}//point contents check
			}//snappedToTargetEnt



			// For now, just compare the bottoms of the bounds.

			//zDist = fabs(vecEndFiltered.z - myCenter.z);
			//zDist = fabs(vecEndFiltered.z - this->pev->absmin.z);
			zDist = fabs(vecEndFiltered.z - (this->pev->origin.z + this->pev->mins.z) );
			
			// Why on EARTH is pev->origin.z + pev->mins.z or maxs.z different from
			// a flat pev->absmin.z and pev->absmax.y?

			//DebugLine_SetupPoint(1, pev->origin, 255, 0, 255);
			//DebugLine_SetupPoint(2, vecEndFiltered, 0, 255, 0);

			//if ( forcePass || zDist > maxZ_DistAllowed )
			if ( !forcePass && zDist > maxZ_DistAllowed )
			{
				// too spammy in any map with lots of vertical level space (a2a1a).
				//easyPrintLine("!!! ROUTE DEBUG %s:%d NOTICE!  Route failed from reaching a point too far from the goal in Z compared to my height.  Difference in Z is: %.2f.  Most allowed: %.2f.", getClassname(), monsterID, zDist, maxZ_DistAllowed);
				
				iReturn = LOCALMOVE_INVALID_DONT_TRIANGULATE;
			}
		}//localmove check, 
	}//doZCheck
	*/


	

	if(doZCheck){
		if(!(pev->flags & (FL_FLY|FL_SWIM) )){
			BOOL result = ZCheck_Ground(pev->origin, vecEndFiltered);
			if(!result){
				iReturn = LOCALMOVE_INVALID_DONT_TRIANGULATE;
			}
		}
	}//doZCheck
	


	/*
	// uncommenting this block will draw a line representing the nearest legal move.
	WRITE_BYTE(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(MSG_BROADCAST, TE_SHOWLINE);
	WRITE_COORD(MSG_BROADCAST, pev->origin.x);
	WRITE_COORD(MSG_BROADCAST, pev->origin.y);
	WRITE_COORD(MSG_BROADCAST, pev->origin.z);
	WRITE_COORD(MSG_BROADCAST, vecStartFiltered.x);
	WRITE_COORD(MSG_BROADCAST, vecStartFiltered.y);
	WRITE_COORD(MSG_BROADCAST, vecStartFiltered.z);
	*/


	if( EASY_CVAR_GET_DEBUGONLY(drawDebugPathfinding) == 1){
		switch(iReturn){
			case LOCALMOVE_INVALID:
				//ORANGE
				//DrawRoute( pev, m_Route, m_iRouteIndex, 239, 165, 16 );
				DrawMyRoute( 48, 33, 4 );
			break;
			case LOCALMOVE_INVALID_DONT_TRIANGULATE:
				//RED
				//DrawRoute( pev, m_Route, m_iRouteIndex, 234, 23, 23 );
				DrawMyRoute( 47, 5, 5 );
			break;
			case LOCALMOVE_VALID:
				//GREEN
				//DrawRoute( pev, m_Route, m_iRouteIndex, 97, 239, 97 );
				DrawMyRoute( 20, 48, 20 );
			break;
		}
	}


	// since we've moved the monster during the check, undo the move.
	UTIL_SetOrigin( pev, vecStartPos );

	if (iReturn == LOCALMOVE_VALID) {
		// breakpoint
		int x = 45;
	}

	return iReturn;
}//CheckLocalMove





BOOL CBaseMonster::ZCheck(const Vector& vecPosition, const Vector& vecEnd){

	if(!(pev->flags & (FL_FLY|FL_SWIM) )){
		// ground type?  Use the ground check then
		BOOL result = ZCheck_Ground(vecPosition, vecEnd);
		return result;
	}else{
		// swim/flyer?  Do a simple raw Z check
		BOOL result = ZCheck_Flyer(vecPosition, vecEnd);
		return result;

	}//fly/swim check

}//ZCheck



BOOL CBaseMonster::ZCheck_Ground(const Vector& vecPosition, const Vector& vecEnd){
	// WAIT.  No need for this.  Will be given the target snapped to its bottom bound of where it is now,
	// fairest for comparisons (not the vertical-center of players that can trip it up)
	/*
	if(pTarget != NULL){
	float distanceBetweenTargetEntAndEndPos = Distance2D(pTarget->pev->origin, vecEnd);
	if(distanceBetweenTargetEntAndEndPos < 16){
	// check for the bottom bound of this ent instead
	}
	}
	*/
	const float maxZ_DistAllowed = pev->size.z * 0.65 + 6; //pev->size.z * 1.25;
	const float myLowerBoundPointZ = vecPosition.z + this->pev->mins.z;
	float zDist;

	//zDist = fabs(vecEnd.z - vecPosition.z);
	zDist = fabs(vecEnd.z - (myLowerBoundPointZ) );

	//if ( forcePass || zDist > maxZ_DistAllowed )
	if ( zDist <= maxZ_DistAllowed ){
		//DebugLine_SetupPoint(4, vecEndFiltered, 0, 255, 0);
		Vector tempVect = vecEnd;
		tempVect.z = myLowerBoundPointZ;  // ???
		//DebugLine_SetupPoint(4, tempVect , 255, 0, 255);

		// too spammy in any map with lots of vertical level space (a2a1a).
		//easyPrintLine("!!! ROUTE DEBUG %s:%d NOTICE!  Route failed from reaching a point too far from the goal in Z compared to my height.  Difference in Z is: %.2f.  Most allowed: %.2f.", getClassname(), monsterID, zDist, maxZ_DistAllowed);

		return TRUE;
	}else{
		return FALSE;
	}

}//ZCheck_Ground



BOOL CBaseMonster::ZCheck_Flyer(const Vector& vecPosition, const Vector& vecEnd){
	const float maxZ_DistAllowed = pev->size.z * 0.65 + 6;
	//pev->size.z * 1.25;
	float zToCompareTo = 0;

	float goalZ = vecEnd.z;
	float myUpperBoundPointZ = vecPosition.z + this->pev->maxs.z;
	float myLowerBoundPointZ = vecPosition.z + this->pev->mins.z;

	if(goalZ > myUpperBoundPointZ){
		// pick that one
		zToCompareTo = myUpperBoundPointZ;
	}else if(goalZ < myLowerBoundPointZ){
		zToCompareTo = myLowerBoundPointZ;
	}else{
		// inbetween?  It's a match then
		zToCompareTo = goalZ;
	}

	float zDist = fabs(goalZ - zToCompareTo);
	if(zDist <= maxZ_DistAllowed){
		return TRUE;
	}else{
		return FALSE;
	}

}//ZCheck_Flyer








float CBaseMonster::OpenDoorAndWait( entvars_t *pevDoor )
{
	float flTravelTime = 0;

	//ALERT(at_aiconsole, "A door. ");
	CBaseEntity *pcbeDoor = CBaseEntity::Instance(pevDoor);
	if (pcbeDoor && !pcbeDoor->IsLockedByMaster())
	{
		//ALERT(at_aiconsole, "unlocked! ");
		pcbeDoor->Use(this, this, USE_ON, 0.0);
		//ALERT(at_aiconsole, "pevDoor->nextthink = %d ms\n", (int)(1000*pevDoor->nextthink));
		//ALERT(at_aiconsole, "pevDoor->ltime = %d ms\n", (int)(1000*pevDoor->ltime));
		//ALERT(at_aiconsole, "pev-> nextthink = %d ms\n", (int)(1000*pev->nextthink));
		//ALERT(at_aiconsole, "pev->ltime = %d ms\n", (int)(1000*pev->ltime));
		flTravelTime = pevDoor->nextthink - pevDoor->ltime;
		//ALERT(at_aiconsole, "Waiting %d ms\n", (int)(1000*flTravelTime));
		if ( pcbeDoor->pev->targetname )
		{
			edict_t *pentTarget = NULL;
			for (;;)
			{
				pentTarget = FIND_ENTITY_BY_TARGETNAME( pentTarget, STRING(pcbeDoor->pev->targetname));

				if ( VARS( pentTarget ) != pcbeDoor->pev )
				{
					if (FNullEnt(pentTarget))
						break;

					if ( FClassnameIs ( pentTarget, STRING(pcbeDoor->pev->classname) ) )
					{
						CBaseEntity *pDoor = Instance(pentTarget);
						if ( pDoor )
							pDoor->Use(this, this, USE_ON, 0.0);
					}
				}
			}
		}
	}

	return gpGlobals->time + flTravelTime;
}


//=========================================================
// AdvanceRoute - poorly named function that advances the 
// m_iRouteIndex. If it goes beyond ROUTE_SIZE, the route 
// is refreshed. 
//=========================================================
void CBaseMonster::AdvanceRoute ( float distance, float flInterval )
{
	//MODDD - use m_iRouteLength instead of ROUTE_SIZE here.
	// WAIT!  Bad assumption, even in the as-is state.
	// What if the last route of the path is indeed the intended goal?  Assuming an incomplete
	// route to whatever goal from being at the last place in the array is a bad assumption.
	if ( m_iRouteIndex == ROUTE_SIZE - 1 )
	{
		//MODDD - new check on top of this:  If the node I'm at is the GOAL, just call for MovementComplete
		if(m_Route[ m_iRouteIndex ].iType & bits_MF_IS_GOAL){
			// That's the end.
			MovementComplete();
		}else{
			// Normal behavior:  try to re-route to the actual goal from here instead.
			// time to refresh the route.
			if ( !FRefreshRoute() )
			{
				ALERT ( at_aiconsole, "Can't Refresh Route!!\n" );
			}
		}
	}
	else
	{
		//MODDD - also, if m_iRouteIndex reaches m_iRouteLength-1 on an 'AdvanceRoute' call, the route is over.
		if ( !(m_Route[ m_iRouteIndex ].iType & bits_MF_IS_GOAL) && !(m_iRouteIndex >= m_iRouteLength-1) )
		{
			// If we've just passed a path_corner, advance m_pGoalEnt
			
			//MODDD - HMMMmmmmmmmm revert
			if ( (m_Route[ m_iRouteIndex ].iType & ~bits_MF_NOT_TO_MASK) == bits_MF_TO_PATHCORNER )
			//if ( (m_Route[ m_iRouteIndex ].iType & ~bits_MF_NOT_TO_MASK) & bits_MF_TO_PATHCORNER )
				m_pGoalEnt = m_pGoalEnt->GetNextTarget();

			// IF both waypoints are nodes, then check for a link for a door and operate it.
			//
			if (  (m_Route[m_iRouteIndex].iType   & bits_MF_TO_NODE) == bits_MF_TO_NODE
			   && (m_Route[m_iRouteIndex+1].iType & bits_MF_TO_NODE) == bits_MF_TO_NODE)
			{
				//ALERT(at_aiconsole, "SVD: Two nodes. ");

				//MODDD - wait.  Really?  Why can't the m_Route have a 'iMapNodeIndex' for this?
				// Set at the time a m_Route element is set to the location of a node from the path
				// being copied over.  Huh.
				// Done.
				//int iSrcNode  = WorldGraph.FindNearestNode(m_Route[m_iRouteIndex].vecLocation, this );
				//int iDestNode = WorldGraph.FindNearestNode(m_Route[m_iRouteIndex+1].vecLocation, this );
				int iSrcNode = m_Route[m_iRouteIndex].iMapNodeIndex;
				int iDestNode = m_Route[m_iRouteIndex+1].iMapNodeIndex;

				int iLink;

				if(iSrcNode == -1){
					// Look it up then.  Save it.
					iSrcNode  = WorldGraph.FindNearestNode(m_Route[m_iRouteIndex].vecLocation, this );
					m_Route[m_iRouteIndex].iMapNodeIndex = iSrcNode;
				}
				if(iDestNode == -1){
					iDestNode = WorldGraph.FindNearestNode(m_Route[m_iRouteIndex+1].vecLocation, this );
					m_Route[m_iRouteIndex + 1].iMapNodeIndex = iDestNode;
				}



				if(iSrcNode == -1 || iDestNode == -1){
					easyPrintLine("!!! PATHFIND ERROR: AdvanceRoute: %s:%d.  Some map-based node could not find a nearestnode, LinkEnt check skipped.  Src:%d Dest:%d", getClassname(), monsterID, iSrcNode, iDestNode);
					iLink = -1;  //force the below linkent check to be skipped
				}else{
					// work normally then
					WorldGraph.HashSearch(iSrcNode, iDestNode, iLink);
				}


				//MODDD - NOTE.
				// UHHHhhhh.  does this even work?  Does anything utilize this?
				// I think it should let entities that pick a node-route with a door go up to the door, open it by being close,
				// wait for it to finish opening, and then move through.
				if ( iLink >= 0 && WorldGraph.m_pLinkPool[iLink].m_pLinkEnt != NULL )
				{
					//ALERT(at_aiconsole, "A link. ");
					if ( WorldGraph.HandleLinkEnt ( iSrcNode, WorldGraph.m_pLinkPool[iLink].m_pLinkEnt, m_afCapability, CGraph::NODEGRAPH_DYNAMIC ) )
					{
						//ALERT(at_aiconsole, "usable.");
						entvars_t *pevDoor = WorldGraph.m_pLinkPool[iLink].m_pLinkEnt;
						if (pevDoor)
						{
							m_flMoveWaitFinished = OpenDoorAndWait( pevDoor );
//							ALERT( at_aiconsole, "Wating for door %.2f\n", m_flMoveWaitFinished-gpGlobals->time );
						}
					}
				}
				//ALERT(at_aiconsole, "\n");
			}
			//MODDD - let monsters know that this is going to a new node (HGrunt strafe is forced to stop).
			onNewRouteNode();

			m_iRouteIndex++;
		}
		else	// At goal!!!
		{
			//MODDD NOTE - hold on. This extra check before calling "MovementComplete" is retail behavior at least.
			//Factoring in framerate, interval and "pathfindNodeToleranceMulti" for passing are new.
			//But why is this check here to begin with? We can only get here (AdvanceRoute) from "ShouldAdvanceRoute" passing which already does the
			//distance check towards the next node or goal.  So it's redundant: how could this check fail if ShouldAdvanceRoute said we should advance?
			//The checks above for not being towards the goal node / point don't even do any extra checks, they just bump up the node as told.
			//I got nothing.

			/*
			const float rawMoveSpeedPerSec = (m_flGroundSpeed * pev->framerate * EASY_CVAR_GET_DEBUGONLY(animationFramerateMulti) * flInterval );
			const float moveDistTest = rawMoveSpeedPerSec * EASY_CVAR_GET_DEBUGONLY(pathfindNodeToleranceMulti);  //defaults to 1 for no effect.
			const float moveDistTol = max(moveDistTest, 8);  //must be at least 8.

			if(EASY_CVAR_GET_DEBUGONLY(pathfindPrintout)==1)easyForcePrintLine("MovementComplete Call 456: dist:%.2f spd:%.2f req:%.2f", distance, rawMoveSpeedPerSec, moveDistTol );
			//BETTER FIX!
			//if ( distance < m_flGroundSpeed * 0.2  ) // FIX
			if( distance < moveDistTol)
			{
				MovementComplete();
			}
			*/
			
			//MODDD - replaced with ShouldAdvanceRoute call since the logic's the same.  
			//...this exact check HAD to pass to even make it to this point. Just skip it and continue already.
			//   We wanted to advance, we called this method BECAUSE we were close enough, just go. Say you're done. Thank you.
			//if(ShouldAdvanceRoute(distance, flInterval)){
				MovementComplete();
			//}


		}
	}
}


int CBaseMonster::RouteClassify( int iMoveFlag )
{
	int movementGoal;

	movementGoal = MOVEGOAL_NONE;

	if ( iMoveFlag & bits_MF_TO_TARGETENT )
		movementGoal = MOVEGOAL_TARGETENT;
	else if ( iMoveFlag & bits_MF_TO_ENEMY )
		movementGoal = MOVEGOAL_ENEMY;
	else if ( iMoveFlag & bits_MF_TO_PATHCORNER )
		movementGoal = MOVEGOAL_PATHCORNER;
	else if ( iMoveFlag & bits_MF_TO_NODE )
		movementGoal = MOVEGOAL_NODE;
	else if ( iMoveFlag & bits_MF_TO_LOCATION )
		movementGoal = MOVEGOAL_LOCATION;

	return movementGoal;
}

//MODDD - NEW.  Inverse of 'RouteClassify' above.
// This takes a MoveGoal and returns the corresponding MoveFlag instead.
// Although they seem to use the same bits anyways?  Same for RouteClassify.
int CBaseMonster::MovementGoalToMoveFlag(int iMoveGoal){
	int iMoveFlag;
	iMoveFlag = 0;  // unsurprisingly, there is no bits_MF_TO_NONE choice.

	if(iMoveGoal == MOVEGOAL_TARGETENT){
		iMoveFlag = bits_MF_TO_TARGETENT;
	}else if(iMoveGoal == MOVEGOAL_ENEMY){
		iMoveFlag = bits_MF_TO_ENEMY;
	}else if(iMoveGoal == MOVEGOAL_PATHCORNER){
		iMoveFlag = bits_MF_TO_PATHCORNER;
	}else if(iMoveGoal == MOVEGOAL_NODE){
		iMoveFlag = bits_MF_TO_NODE;
	}else if(iMoveGoal == MOVEGOAL_LOCATION){
		iMoveFlag = bits_MF_TO_LOCATION;
	}

	return iMoveFlag;
}




BOOL CBaseMonster::attemptRampFix(const Vector &vecGoal, int iMoveFlag, CBaseEntity *pTarget){
	float flDist;
	Vector vecDir = (vecGoal - pev->origin).Normalize();
	
	//if( !localMovePass ){
	//still a shot...?
		
	Vector rampTopPoint1;
	Vector rampTopPoint2;
	BOOL localMovePass = FALSE;

	int rampFixAttempt = 0; 
	//-1 = to go down (I'm higher than the goal, have to go down)
	//0 = none (too flat, no ramp fix possible)
	//1 = to go up (I'm lower than the goal, have to go up)

	if(pev->origin.z > vecGoal.z + 20 ){
		//above. 
		rampFixAttempt = 1;
	}else if(pev->origin.z < vecGoal.z + -20 ){
		//below.
		rampFixAttempt = -1;
	}

	if(rampFixAttempt != 0){
		EASY_CVAR_PRINTIF_PRE(pathfindPrintout, easyForcePrintLine("%s:ID%d Pathfinding: TRYING RAMPFIX...", getClassnameShort(), monsterID) );
		//first, check the point where the first localMove check failed. Is this a ramp? Check the slope.
			
		debugVectorMode = 1;

		BOOL hasTowardsRampNode = FALSE;

		Vector vecDirFlatUnnormal = Vector(vecDir.x, vecDir.y, 0);
		//Vector initFailPoint = pev->origin + vecDir * flDist;
		Vector vecMyOrigin = pev->origin;
		//Vector vecNextNode = m_Route[m_iRouteIndex].vecLocation;
		Vector vecNextNode = vecGoal;

		/*
		TraceResult trPathFind;
	    edict_t* pentIgnore = ENT( this->pev );
		UTIL_TraceLine(vecMyOrigin, pev->origin + vecDir * flCheckDist, dont_ignore_monsters, pentIgnore, &trPathFind);
		//CBaseEntity* pEntityHit;
		*/

		TraceResult trRampBeginAttempt;
	    edict_t* pentIgnore;

		Vector vecTraceStart;
		Vector vecTraceEnd;


		if(rampFixAttempt == 1){
			//travel down the ramp.
			//(go from the goal to MY origin to collide with the ramp first)
			vecTraceStart = Vector(vecNextNode.x, vecNextNode.y, vecNextNode.z + 4);
			vecTraceEnd = Vector(vecMyOrigin.x, vecMyOrigin.y, vecNextNode.z + 4);
				
			if(pTarget != NULL){
				pentIgnore = ENT( pTarget->pev );
			}else{
				pentIgnore = ENT( this->pev );
			}
				
		}else{
			//travel up the ramp
			vecTraceStart = Vector(vecMyOrigin.x, vecMyOrigin.y, vecMyOrigin.z + 4);
			//vecTraceEnd = Vector(vecGoal.x, vecGoal.y, vecMyOrigin.z + 4);
			vecTraceEnd = Vector(vecNextNode.x, vecNextNode.y, vecMyOrigin.z + 4);
				
			pentIgnore = ENT( this->pev );
		}

		/*
		debugVector1 = vecRampLowPoint;
		debugVector2 = rampTopPoint1;
		debugVector3 = rampTopPoint2;
		debugVector4 = vecGoalOrigin;
		debugVectorsSet = TRUE;
		*/

		/*
		debugVector1 = vecTraceStart;
		debugVector2 = vecTraceEnd;
		debugVectorsSet = TRUE;
		debugFailColor = FALSE;
		debugVectorMode = 2;
		*/

		//UTIL_TraceLine(vecTraceStart, vecTraceEnd, dont_ignore_monsters, pentIgnore, &trRampBeginAttempt);
		UTIL_TraceLine(vecTraceStart, vecTraceEnd, ignore_monsters, pentIgnore, &trRampBeginAttempt);
			
		EASY_CVAR_PRINTIF_PRE(pathfindPrintout, easyForcePrintLine("%s:ID%d BOTTOM TO UP: GOALPOS? (%.2f %.2f %.2f) SLD?:%d FRAC:%.2f N:(%.2f %.2f %.2f)", getClassnameShort(), this->monsterID,
			vecGoal.x, vecGoal.y, vecGoal.z
			, !trRampBeginAttempt.fAllSolid, trRampBeginAttempt.flFraction,
			trRampBeginAttempt.vecPlaneNormal.x, trRampBeginAttempt.vecPlaneNormal.y, trRampBeginAttempt.vecPlaneNormal.z ) );

		//MODDD TODO - is it possible for a ramp to have a vecPlaneNormal.z that is negative and still a typical ramp (as opposed to an incline down from the ceiling)?

		//also, if the fracition is 1.0, that means nothing was hit.  That is important this time.
		//if (!trPathFind.fAllSolid && trPathFind.flFraction < 1.0 && ((pEntityHit = CBaseEntity::Instance(trPathFind.pHit)) != NULL) ){
		if (!trRampBeginAttempt.fAllSolid && trRampBeginAttempt.flFraction < 1.0 && (trRampBeginAttempt.vecPlaneNormal.z >= 0.92) ){

			Vector distVect = (vecGoal - vecMyOrigin);
			Vector distVectFlat = Vector(distVect.x, distVect.y, 0);
			BOOL checkLocalMovePreRampTest;

			Vector toBaseRamp;
			Vector vecRampLowPoint;
			if(rampFixAttempt == 1){
				//travel down the ramp
				toBaseRamp = -vecDirFlatUnnormal * distVectFlat.Length() * trRampBeginAttempt.flFraction;
				vecRampLowPoint = vecNextNode + Vector(0, 0, 12) + toBaseRamp + vecDirFlatUnnormal * 6;
			}else{
				//travel up the ramp.
				toBaseRamp = vecDirFlatUnnormal * distVectFlat.Length() * trRampBeginAttempt.flFraction;
				vecRampLowPoint = vecMyOrigin + Vector(0, 0, 12) + toBaseRamp + -vecDirFlatUnnormal * 6;
			}

			/*
			Vector vecRampLowPoint = vecMyOrigin + Vector(0, 0, 12) + toBaseRamp + -vecDirFlatUnnormal * 6;
			if(toBaseRamp.Length() > 20){
				hasTowardsRampNode = TRUE;
			}else{
				hasTowardsRampNode = FALSE;
				//change. "I" am the ramp low point.
				vecRampLowPoint = pev->origin;
			}

			if(!hasTowardsRampNode){
				checkLocalMovePreRampTest = TRUE;
			}else{
				//checkLocalMovePreRampTest = CheckLocalMoveHull(pev->origin + Vector(0, 0, 12), vecRampLowPoint, pTarget, &flDist);
				//HACK
				checkLocalMovePreRampTest = TRUE;
			}
			*/

			checkLocalMovePreRampTest = TRUE;

			//if close enough, skip this check. otherwise, require it to be safe.
			if(checkLocalMovePreRampTest ){
				//this passed? continue with the ramp check.
				Vector vecRampPlaneNormal = trRampBeginAttempt.vecPlaneNormal;
				Vector vecGoalOrigin;
				if(rampFixAttempt == 1){
					//travel down.
					vecGoalOrigin = vecMyOrigin;
				}else{
					//travel up.
					//vecGoalOrigin = m_Route[m_iRouteIndex].vecLocation;  ??
					vecGoalOrigin = vecGoal;
				}

				float dotTest = -DotProduct(vecRampPlaneNormal, vecDir);
				//if(dotTest < 0.5){
					//skipping the check for now.
					Vector dirRawStartToGoal = ( vecGoalOrigin - vecRampLowPoint ).Normalize();
						
					//First, get a direction from the goal to the current point, flat-ways.
					Vector dirFlatGoalToStart = projectionOntoPlane( dirRawStartToGoal, Vector(0, 0, 1) ).Normalize();
					Vector dirUpRamp = projectionOntoPlane(dirRawStartToGoal, vecRampPlaneNormal).Normalize();

					//columns by rows. NOT including the constant column.
					Vector deltaOrigin = vecGoalOrigin + -vecRampLowPoint;
					float par_t;

					Vector n = Vector(0, 0, 1); //flat plane at top of ramp
					Vector la = dirUpRamp;

					float dot1 = DotProduct(n, deltaOrigin);
					float dot2 = DotProduct(n, la);
					if (dot2 != 0) {
						par_t = dot1 / dot2;
							
						Vector vecIntersection = Vector(
							dirUpRamp.x * par_t + vecRampLowPoint.x,
							dirUpRamp.y * par_t + vecRampLowPoint.y,
							dirUpRamp.z * par_t + vecRampLowPoint.z
						);
								
						//rampTopPoint1 = vecIntersection + Vector(0, 0, 6) + -dirUpRamp * 3;
						//rampTopPoint2 = vecIntersection + Vector(0, 0, 6) + -dirFlatGoalToStart * 3;
						rampTopPoint1 = vecIntersection + Vector(0, 0, 12); //+ -dirUpRamp * 20;
						rampTopPoint2 = vecIntersection + Vector(0, 0, 12);

						BOOL passCheck1 = (CheckLocalMoveHull (vecRampLowPoint, rampTopPoint1, pTarget, &flDist) == LOCALMOVE_VALID);
						BOOL passCheck2 = FALSE; //default.
						//if(passCheck1){
							passCheck2 = (CheckLocalMoveHull (rampTopPoint2, vecGoalOrigin, pTarget, &flDist) == LOCALMOVE_VALID);
						//}
								
						if(rampFixAttempt == 1){
							EASY_CVAR_PRINTIF_PRE(pathfindPrintout, easyForcePrintLine("%s:ID%d BOTTOM TO UP: PASS? %d %d", getClassnameShort(), this->monsterID, passCheck1, passCheck2 ) );
						}else{
							EASY_CVAR_PRINTIF_PRE(pathfindPrintout, easyForcePrintLine("%s:ID%d TOP TO DOWN: PASS? %d %d", getClassnameShort(), this->monsterID, passCheck1, passCheck2) );
						}

						if(passCheck1 && passCheck2){
							//we can go "up" the ramp.
							localMovePass = TRUE;
						}
					}
					else {
						EASY_CVAR_PRINTIF_PRE(pathfindPrintout, easyForcePrintLine("%s:ID%d HORRIBLE FAILURE YOU CANNOT DIVIDE BY 0 MORTAL", getClassnameShort(), this->monsterID ) );
					}

					if(localMovePass){
						int currentNodeIndex = 0;
								

						//hasTowardsRampNode
						if( (vecGoalOrigin - vecMyOrigin ).Length2D() > 60 ){
							m_Route[ currentNodeIndex ].vecLocation = vecRampLowPoint;
							m_Route[ currentNodeIndex ].iType = (iMoveFlag | bits_MF_RAMPFIX | bits_MF_TO_DETOUR);
							m_Route[ currentNodeIndex ].iMapNodeIndex = -1;
							currentNodeIndex++;
						}

						m_Route[ currentNodeIndex ].vecLocation = rampTopPoint1;
						m_Route[ currentNodeIndex ].iType = (iMoveFlag | bits_MF_RAMPFIX | bits_MF_TO_DETOUR);
						m_Route[ currentNodeIndex ].iMapNodeIndex = -1;
						currentNodeIndex++;

						m_Route[ currentNodeIndex ].vecLocation = vecGoal;
						m_Route[ currentNodeIndex ].iType = iMoveFlag | bits_MF_RAMPFIX | bits_MF_IS_GOAL;
						m_Route[ currentNodeIndex ].iMapNodeIndex = -1;
						currentNodeIndex++;

						m_iRouteLength = currentNodeIndex;

						m_vecMoveGoal = vecGoal;
						//RouteSimplify( pTarget );

						debugVector1 = vecRampLowPoint;
						debugVector2 = rampTopPoint1;
						debugVector3 = rampTopPoint2;
						debugVector4 = vecGoalOrigin;
						debugVectorsSet = TRUE;
						return TRUE;
					}//END OF if(localMovePass)
					else{
						debugVector1 = vecRampLowPoint;
						debugVector2 = rampTopPoint1;
						debugVector3 = rampTopPoint2;
						debugVector4 = vecGoalOrigin;
						debugVectorsSet = TRUE;
						debugFailColor = TRUE;
					}

				//}//END OF dot check
			}//END OF pre move test (from current place to the bottom of the ramp, if not immediately there)
		}//END OF trace-hit-something check
	}else{
		EASY_CVAR_PRINTIF_PRE(pathfindPrintout, easyForcePrintLine("%s:ID%d Pathfinding: NO RAMP TEST.", getClassnameShort(), monsterID) );
	}

	// fell through? assume failure
	return FALSE;
}//attemptRampFix




//=========================================================
// BuildRoute
//=========================================================
BOOL CBaseMonster::BuildRoute ( const Vector &vecGoal, int iMoveFlag, CBaseEntity *pTarget )
{
	float flDist;
	Vector vecApex;
	int	iLocalMove;

	RouteNew();

	m_movementGoal = RouteClassify( iMoveFlag );


	
// so we don't end up with no moveflags
	m_Route[ 0 ].vecLocation = vecGoal;
	m_Route[ 0 ].iType = iMoveFlag | bits_MF_IS_GOAL;
	m_Route[ 0 ].iMapNodeIndex = -1;

	m_iRouteLength = 1;  //so far?


	// If a straight-line trace couldn't work, show me why
	g_CheckLocalMoveCanReportBlocker = TRUE;

// check simple local move
	iLocalMove = CheckLocalMove( pev->origin, vecGoal, pTarget, TRUE, &flDist );

	g_CheckLocalMoveCanReportBlocker = FALSE;

	if ( iLocalMove == LOCALMOVE_VALID )
	{
		EASY_CVAR_PRINTIF_PRE(pathfindPrintout, easyForcePrintLine("%s:ID%d BuildRoute: I GOT SATISIFED 1", getClassnameShort(), monsterID) );
		// monster can walk straight there!
		return TRUE;
	}
// try to triangulate around any obstacles.
	else if ( iLocalMove != LOCALMOVE_INVALID_DONT_TRIANGULATE && FTriangulate( pev->origin, vecGoal, flDist, pTarget, &vecApex ) )
	{
		EASY_CVAR_PRINTIF_PRE(pathfindPrintout, easyForcePrintLine("%s:ID%d BuildRoute: I GOT SATISIFED 2", getClassnameShort(), monsterID) );
		// there is a slightly more complicated path that allows the monster to reach vecGoal
		m_Route[ 0 ].vecLocation = vecApex;
		m_Route[ 0 ].iType = (iMoveFlag | bits_MF_TO_DETOUR);
		m_Route[ 0 ].iMapNodeIndex = -1;

		m_Route[ 1 ].vecLocation = vecGoal;
		m_Route[ 1 ].iType = iMoveFlag | bits_MF_IS_GOAL;
		m_Route[ 1 ].iMapNodeIndex = -1;

		//DebugLine_Setup(1, pev->origin, vecApex, 0, 255, 0);
		//DebugLine_Setup(1, vecApex, vecGoal, 0, 255, 0);


		m_iRouteLength = 2;

			
			//WRITE_BYTE(MSG_BROADCAST, SVC_TEMPENTITY);
			//WRITE_BYTE(MSG_BROADCAST, TE_SHOWLINE);
			//WRITE_COORD(MSG_BROADCAST, vecApex.x );
			//WRITE_COORD(MSG_BROADCAST, vecApex.y );
			//WRITE_COORD(MSG_BROADCAST, vecApex.z );
			//WRITE_COORD(MSG_BROADCAST, vecApex.x );
			//WRITE_COORD(MSG_BROADCAST, vecApex.y );
			//WRITE_COORD(MSG_BROADCAST, vecApex.z + 128 );
			

		RouteSimplify( pTarget );
		return TRUE;
	}
	

// last ditch, try nodes
	if ( FGetNodeRoute(pev->origin, vecGoal ) )
	{
//		ALERT ( at_console, "Can get there on nodes\n" );
		EASY_CVAR_PRINTIF_PRE(pathfindPrintout, easyForcePrintLine("%s:ID%d BuildRoute: I GOT SATISIFED 3", getClassnameShort(), monsterID) );
		m_vecMoveGoal = vecGoal;
		RouteSimplify( pTarget );
		return TRUE;
	}



	//Vector vecDir = (vecGoal - pev->origin).Normalize();

	//TODO IN THE PATH FINDING:
	//elevation check?  See if the goal (next node; m_Route[m_iRouteIndex].vecLocation) position is above my current position, or below.
	// If the goal is  on the same level... can't do the fix.
	// If the goal is below me, it's the top-ramp fix.  (currently in)
	// If the goal is above me, it's the bottom-ramp fix (not done)


	
	//initial attempt.
	
	/*

	//
	BOOL localMovePass = (CheckLocalMove ( pev->origin, vecGoal, pTarget, TRUE &flDist ) == LOCALMOVE_VALID);
	//BOOL localMovePass = (CheckLocalMove ( pev->origin, pev->origin + vecDir * flWaypointDist, pTargetEnt, FALSE, &flDist ) == LOCALMOVE_VALID);
	//BOOL localMovePass = (CheckLocalMove ( pev->origin, pev->origin + vecDir * flCheckDist, pTargetEnt, FALSE, &flDist ) == LOCALMOVE_VALID);
	EASY_CVAR_PRINTIF_PRE(pathfindPrintout, easyForcePrintLine("%s:ID%d Pathfinding: ROUTE FIRST PASS? %d", getClassnameShort(), monsterID, localMovePass) );
	*/
			//debugVectorMode = 0;
			//	debugVector1 = pev->origin;
			//	debugVector2 = pev->origin + vecDir * flCheckDist;
			//	debugVector3 = m_Route[ m_iRouteIndex ].vecLocation;
				//debugVectorsSet = TRUE;
		


	debugVectorsSet = FALSE;
	debugFailColor = FALSE;


	if(EASY_CVAR_GET_DEBUGONLY(pathfindRampFix) == 1){
		BOOL rampFixSuccess = attemptRampFix(vecGoal, iMoveFlag, pTarget);
		if(rampFixSuccess){
			// yipee
			return TRUE;
		}
	}//pathfindRampFix check

	
	EASY_CVAR_PRINTIF_PRE(pathfindPrintout, easyForcePrintLine("%s:ID%d BuildRoute: TOTAL FAIL!", getClassnameShort(), monsterID) );

	// b0rk
	return FALSE;
}//BuildRoute



//=========================================================
// InsertWaypoint - Rebuilds the existing route so that the
// supplied vector and moveflags are the first waypoint in
// the route, and fills the rest of the route with as much
// of the pre-existing route as possible
//=========================================================
void CBaseMonster::InsertWaypoint ( Vector vecLocation, int afMoveFlags )
{
	int i;
	int type;

	// we have to save some Index and Type information from the real
	// path_corner or node waypoint that the monster was trying to reach. This makes sure that data necessary 
	// to refresh the original path exists even in the new waypoints that don't correspond directy to a path_corner
	// or node. 
	type = afMoveFlags | (m_Route[ m_iRouteIndex ].iType & ~bits_MF_NOT_TO_MASK);

	//MODDD - involves m_iRouteLength now
	/*
	for ( i = ROUTE_SIZE-1; i > 0; i-- )
		m_Route[i] = m_Route[i-1];

	m_Route[ m_iRouteIndex ].vecLocation = vecLocation;
	m_Route[ m_iRouteIndex ].iType = type;
	*/

	// To begin at one beyond the last node in the route (or at the last node in the route if the route
	// already uses all places in m_Route)
	int copyStart;
	

	if(m_iRouteLength < ROUTE_SIZE){
		// Safe to start at m_iRouteLength exactly (increases route size by 1)
		copyStart = m_iRouteLength;

		m_iRouteLength++;
	}else{
		// Will be cutoff, no change in route length
		copyStart = ROUTE_SIZE - 1;
	}

	//MODDD - also, why stop at 0?  Why not at m_iRouteIndex, since earlier points won't be seen again?
	//for ( i = copyStart; i > 0; i-- )
	for ( i = copyStart; i > m_iRouteIndex; i-- ){
		m_Route[i] = m_Route[i-1];
	}

	m_Route[m_iRouteIndex].vecLocation = vecLocation;
	m_Route[m_iRouteIndex].iType = type;
	m_Route[m_iRouteIndex].iMapNodeIndex = -1;

}//InsertWaypoint





//=========================================================
// FTriangulate - tries to overcome local obstacles by 
// triangulating a path around them.
//
// iApexDist is how far the obstruction that we are trying
// to triangulate around is from the monster.
//=========================================================
BOOL CBaseMonster::FTriangulate ( const Vector &vecStart , const Vector &vecEnd, float flDist, CBaseEntity *pTargetEnt, Vector *pApex )
{
	//MODDD - NOTE.  Idea to keep in mind:  Direction triangulation runs in (to get forward, right vectors)
	// is from vecStart to vecEnd, yet the goal (vecFarSide) is the waypoint I'm heading towards.
	// This is fine, but unsure if there could be odd behavior at any discprency between the 'vecEnd' given here
	// and the route point (m_Route[m_iRouteIndex]).
	// I don't get the point about vecFarSide being 'the spot we move to after hitting the triang point before moving
	// on to our normal goal' comment, seems vecFarSide IS the goal for being set to the waypoint this monster's
	// headed towards.  I think that describes pApex better.
	// In the very least this means calling FTriangulate without a route in mind (garbage m_Route info) 
	// would not be very helpful.

	float furthestDist;
	Vector vecDirRight;
	Vector vecDirUp(0,0,1);
	Vector vecDirLeftUp;
	Vector vecDirRightUp;
	Vector vecForward;
	Vector vecLeft;// the spot we'll try to triangulate to on the left
	Vector vecRight;// the spot we'll try to triangulate to on the right
	Vector vecTop;// the spot we'll try to triangulate to on the top
	Vector vecBottom;// the spot we'll try to triangulate to on the bottom
	Vector vecFarSide;// the spot that we'll move to after hitting the triangulated point, before moving on to our normal goal.

	//MODDD - NEW
	Vector vecLeftTop;
	Vector vecRightTop;
	//Vector vecForwardTop;

	Vector vecRightShiftApex;
	Vector vecUpShiftApex;

	//MODDD - another idea.  How about recording the max distance left/right/up/down allowed to be traveled from
	// my pos to the target pos, and instead of completely blocking  a triangulation attept just because I can't go any
	// further in that direction, try once more snapped to the max distance I could go in that direction?
	// That is, if going 60 unts left is fine, but going 120 hits a wall at some point, where did it? 70? 80? 110? etc.
	// Try from that point instead then.

	int sourceMaxLeftHit = 0;
	int sourceMaxRightHit = 0;
	int sourceMaxUpHit = 0;
	int sourceMaxDownHit = 0;
	int sourceMaxLeftUpHit = 0;
	int sourceMaxRightUpHit = 0;

	int i;
	float sizeX;
	float sizeZ;

	// If the hull width is less than 24, use 24 because CheckLocalMove uses a min of
	// 24.
	sizeX = pev->size.x;
	if (sizeX < 24.0)
		sizeX = 24.0;
	else if (sizeX > 48.0)
		sizeX = 48.0;
	sizeZ = pev->size.z;
	//if (sizeZ < 24.0)
	//	sizeZ = 24.0;

	//MODDD - allow trying in larger increments.
	// was 1.25, not as-is though
	sizeX *= 1.70;
	sizeZ *= 1.70;

	//MODDD - added triangulation attemps for a blend of up and left/right too, but only for going upwards for now.
	// Careful about making this too expensive, arleady cut the number of attempts in half to make up for this.


	vecForward = ( vecEnd - vecStart ).Normalize();
	
	vecDirRight = CrossProduct ( vecForward, vecDirUp);

	vecDirRightUp = (vecDirRight + vecDirUp).Normalize();
	vecDirLeftUp = (-vecDirRight + vecDirUp).Normalize();

	/*
	// ALTERNATE WAY: interpret the vecForward as angles and derive dirUp & Left from that.
	// Keep in mind, alternate takes on 'vecDirUp' (any besides (0,0,1) would make more sense for
	// fliers, I doubt groundmovers get any benefit from a 'up' dir from looking at something
	// above them leaning backwards.  Groundmovers don't even use vecDirUp at all, then again.

	Vector theAng = UTIL_VecToAngles(vecForward);
	UTIL_MakeAimVectors(theAng);
	Vector vecDirUp;
	// already started from vecForward
	// Don't know why the 'gpGlobals->v_up' is wonky compared to the others, I would've expected
	// it to either always go straight up (0,0,1) or be angled behind a little ways if vecForward
	// was looking at something at a higher point.  Leaning towards it though too?  Wha?
	// AAAAGGH.   It's from using UTIL_MakeVectors instead of UTIL_MakeAimVectors, isn't it.  Fixed since.
	//vecDirUp = gpGlobals->v_up;
	vecDirRight = gpGlobals->v_right;

	vecDirUp = CrossProduct(vecForward, vecDirRight);

	Vector testVec = pev->origin + vecForward * 200;
	Vector testVec2 = pev->origin + vecDirUp * 200;
	Vector testVec3 = pev->origin + vecDirRight * 200;
	UTIL_drawLineFrame(pev->origin.x, pev->origin.y, pev->origin.z, testVec.x, testVec.y, testVec.z, 12, 30, 255, 255, 0);
	UTIL_drawLineFrame(pev->origin.x, pev->origin.y, pev->origin.z, testVec2.x, testVec2.y, testVec2.z, 12, 30, 0, 255, 0);
	UTIL_drawLineFrame(pev->origin.x, pev->origin.y, pev->origin.z, testVec3.x, testVec3.y, testVec3.z, 12, 30, 0, 0, 255);
	*/

	// start checking right about where the object is, picking two equidistant starting points, one on
	// the left, one on the right. As we progress through the loop, we'll push these away from the obstacle, 
	// hoping to find a way around on either side. pev->size.x is added to the ApexDist in order to help select
	// an apex point that insures that the monster is sufficiently past the obstacle before trying to turn back
	// onto its original course.
	
	//MODDD - unsure why sizeX is even added to flDist times vecForward, times 0.5 instead now
	vecLeft = pev->origin + ( vecForward * ( flDist + sizeX * 0.5 ) ) - (vecDirRight * ( sizeX * 3 ));
	vecRight = pev->origin + ( vecForward * ( flDist + sizeX * 0.5 ) ) + (vecDirRight * ( sizeX * 3 ));

	//MODDD - Why not add sizeZ to flDist times vecForward too?
	if (isMovetypeFlying()){
		vecTop = pev->origin + (vecForward * (flDist + sizeZ * 0.5) ) + (vecDirUp * (sizeZ * 3));
		vecBottom = pev->origin + (vecForward * (flDist + sizeZ * 0.5) ) - (vecDirUp *  (sizeZ * 3));

		//MODDD - NEW.
		vecLeftTop = pev->origin + (vecForward * flDist) + -(vecDirRight * ( sizeX * 2.25 )) + (vecDirUp * (sizeZ * 2.25));
		vecRightTop = pev->origin + (vecForward * flDist) + (vecDirRight * ( sizeX * 2.25 )) + (vecDirUp * (sizeZ * 2.25));
	}


	//MODDD - just be the vecEnd supplied dangit.
	// Was a coincidence that m_Route[m_iRouteIndex].vecLocation matched what the two retail calls sent for vecEnd here.
	//vecFarSide = m_Route[ m_iRouteIndex ].vecLocation;
	vecFarSide = vecEnd;
	
	vecRightShiftApex = vecDirRight * sizeX * 1.6;
	if (isMovetypeFlying() ){
		vecUpShiftApex = vecDirUp * sizeZ * 1.6;
	}




	// left of the target to the target, is that ok?
	if(CheckLocalMove(pev->origin, vecLeft, pTargetEnt, FALSE, &furthestDist) == LOCALMOVE_VALID){
		// passed?  That's all.
	}else{
		if(furthestDist > 24){
			// worthwhile?
			//vecLeft = pev->origin + -vecDirRight * (furthestDist - 16);
			Vector pointDelta = vecLeft - pev->origin;
			//float theLen = pointDelta.Length();
			Vector theDir = pointDelta.Normalize();
			vecLeft = pev->origin + theDir * (furthestDist - 16);
			sourceMaxLeftHit = 1;
		}else{
			//MODDD - IDEA would a line-trace against the wall hit and going along that
			// to the finished distance ( (vecLeft - pev->origin).Length() - furtestDist)
			// as much as possible be ok in some cases with a wall close to my left/right?
			// Copy to other directions if this ever gets real use

			sourceMaxLeftHit = 2;
			

			//////////////////////////////////////////////////////////////
		}
	}


	if(CheckLocalMove(pev->origin, vecRight, pTargetEnt, FALSE, &furthestDist) == LOCALMOVE_VALID){

	}else{
		if(furthestDist > 24){
			// worthwhile?
			//vecRight = pev->origin + vecDirRight * (furthestDist - 16);
			Vector pointDelta = vecRight - pev->origin;
			Vector theDir = pointDelta.Normalize();
			vecRight = pev->origin + theDir * (furthestDist - 16);
			sourceMaxRightHit = 1;
		}else{
			// no.  Don't bother with this direction from the target then
			sourceMaxRightHit = 2;
		}
	}
	if(isMovetypeFlying()){
		if(CheckLocalMove(pev->origin, vecTop, pTargetEnt, FALSE, &furthestDist) == LOCALMOVE_VALID){

		}else{
			if(furthestDist > 24){
				// worthwhile?
				//vecTop = pev->origin + vecDirUp * (furthestDist - 32);
				Vector pointDelta = vecTop - pev->origin;
				Vector theDir = pointDelta.Normalize();
				vecTop = pev->origin + theDir * (furthestDist - 16);
				sourceMaxUpHit = 1;
			}else{
				// no.  Don't bother with this direction from the target then
				sourceMaxUpHit = 2;
			}
		}
		if(CheckLocalMove(pev->origin, vecBottom, pTargetEnt, FALSE, &furthestDist) == LOCALMOVE_VALID){

		}else{
			if(furthestDist > 24){
				// worthwhile?
				//vecBottom = pev->origin + -vecDirUp * (furthestDist - 32);
				Vector pointDelta = vecBottom - pev->origin;
				Vector theDir = pointDelta.Normalize();
				vecBottom = pev->origin + theDir * (furthestDist - 16);
				sourceMaxDownHit = 1;
			}else{
				// no.  Don't bother with this direction from the target then
				sourceMaxDownHit = 2;
			}
		}

		if(CheckLocalMove(pev->origin, vecLeftTop, pTargetEnt, FALSE, &furthestDist) == LOCALMOVE_VALID){

		}else{
			if(furthestDist > 24){
				// worthwhile?
				//vecLeftTop = pev->origin + vecDirLeftUp * (furthestDist - 32);
				Vector pointDelta = vecLeftTop - pev->origin;
				Vector theDir = pointDelta.Normalize();
				vecLeftTop = pev->origin + theDir * (furthestDist - 16);
				sourceMaxLeftUpHit = 1;
			}else{
				// no.  Don't bother with this direction from the target then
				sourceMaxLeftUpHit = 2;
			}
		}
		if(CheckLocalMove(pev->origin, vecRightTop, pTargetEnt, FALSE, &furthestDist) == LOCALMOVE_VALID){

		}else{
			//pev->size.x * 0.7 + pev->size.z * 0.7
			if(furthestDist > 24){
				// worthwhile?
				//vecRightTop = pev->origin + vecDirRightUp * (furthestDist - 32);
				Vector pointDelta = vecRightTop - pev->origin;
				Vector theDir = pointDelta.Normalize();
				vecRightTop = pev->origin + theDir * (furthestDist - 16);
				sourceMaxRightUpHit = 1;
			}else{
				// no.  Don't bother with this direction from the target then
				sourceMaxRightUpHit = 2;
			}
		}
	}


	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
	//MODDD - ADJUSTMENT IDEA:  Try shifting the target we want to go to alongside the direction-away-from-self checks.
	// That is, there may be more success sometimes on,  'If I moved left 50 units to hit a point left 50 units of my
	// target' may find more success than 'If I moved left of 50 units to hit my target'.
	// BUT.  Don't allow shifting the point of a target any further than the furthest left/right/etc. it can go in that
	// direction.  If there is a brick wall 80 units left of a target, checking from 50, 100, 150, 200 units away from 
	// myself is ok,  but further than those 80 units left of the target is pointless, sotp advancing the offset from
	// the target past those 80 units.

	// for now, vecTargetBottom won't move.  All 'vecTarget''s staying at vecFarSide
	// gives retail behavior.  Besides the additional vecLeftTop, vecRightTop checks that is.

	// If I have a pTargetEnt in mind, can involve its size too.
	float targetSizeX;
	float targetSizeZ;
	if(pTargetEnt != NULL){
		targetSizeX = pTargetEnt->pev->size.x;
		targetSizeZ = pTargetEnt->pev->size.z;
	}else{
		// just use mine reduced a bit?
		targetSizeX = sizeX * 0.8;
		targetSizeZ = sizeZ * 0.8;
	}

	BOOL targetMaxLeftHit = FALSE;
	BOOL targetMaxRightHit = FALSE;
	BOOL targetMaxTopHit = FALSE;
	BOOL targetMaxBottomHit = FALSE;   //never paid attention to
	Vector vecTargetLeft = vecFarSide;
	Vector vecTargetRight = vecFarSide;
	Vector vecTargetTop = vecFarSide;
	Vector vecTargetBottom = vecFarSide;
	Vector vecTest;


	/*
	vecTest = vecFarSide + vecDirRight * (targetSizeX + sizeX * 1.2);
	// left of the target to the target, is that ok?
	if(CheckLocalMove(vecTest, vecFarSide, pTargetEnt, FALSE, &furthestDist) == LOCALMOVE_VALID){
		// ok, use exactly that to start
		//vecTargetLeft = vecTest;
		vecTargetLeft = vecFarSide + vecDirRight * (targetSizeX * 0.5 + sizeX * 0.5);
	}else{
		// Don't bother with this direction from the target then
		targetMaxLeftHit = TRUE;
	}

	vecTest = vecFarSide + -vecDirRight * (targetSizeX + sizeX * 1.2);
	if(CheckLocalMove(vecTest, vecFarSide, pTargetEnt, FALSE, &furthestDist) == LOCALMOVE_VALID){
		vecTargetRight = vecFarSide + -vecDirRight * (targetSizeX * 0.5 + sizeX * 0.5);
	}else{
		targetMaxRightHit = TRUE;
	}
	
	if (isMovetypeFlying()){
		vecTest = vecFarSide + vecDirUp * (targetSizeZ + sizeZ * 1.2);
		if(CheckLocalMove(vecTest, vecFarSide, pTargetEnt, FALSE, &furthestDist) == LOCALMOVE_VALID){
			vecTargetTop = vecFarSide + vecDirUp * (targetSizeZ * 0.5 + sizeZ * 0.5);
		}else{
			targetMaxTopHit = TRUE;
		}
	}
	*/
	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////




	for ( i = 0 ; i < MAX_TRIANGULATION_ATTEMPTS; i++ )
	{
// Debug, Draw the triangulation
#if 0
		MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
			WRITE_BYTE( TE_SHOWLINE);
			WRITE_COORD( pev->origin.x );
			WRITE_COORD( pev->origin.y );
			WRITE_COORD( pev->origin.z );
			WRITE_COORD( vecRight.x );
			WRITE_COORD( vecRight.y );
			WRITE_COORD( vecRight.z );
		MESSAGE_END();

		MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
			WRITE_BYTE( TE_SHOWLINE );
			WRITE_COORD( pev->origin.x );
			WRITE_COORD( pev->origin.y );
			WRITE_COORD( pev->origin.z );
			WRITE_COORD( vecLeft.x );
			WRITE_COORD( vecLeft.y );
			WRITE_COORD( vecLeft.z );
		MESSAGE_END();
#endif

#if 0
		if (isMovetypeFlying())
		{
			MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
				WRITE_BYTE( TE_SHOWLINE );
				WRITE_COORD( pev->origin.x );
				WRITE_COORD( pev->origin.y );
				WRITE_COORD( pev->origin.z );
				WRITE_COORD( vecTop.x );
				WRITE_COORD( vecTop.y );
				WRITE_COORD( vecTop.z );
			MESSAGE_END();

			MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
				WRITE_BYTE( TE_SHOWLINE );
				WRITE_COORD( pev->origin.x );
				WRITE_COORD( pev->origin.y );
				WRITE_COORD( pev->origin.z );
				WRITE_COORD( vecBottom.x );
				WRITE_COORD( vecBottom.y );
				WRITE_COORD( vecBottom.z );
			MESSAGE_END();
		}
#endif








		// RETAIL FOR REFERENCE
		/*
		if ( CheckLocalMove( pev->origin, vecRight, pTargetEnt, FALSE, NULL ) == LOCALMOVE_VALID )
		{
			if ( CheckLocalMove ( vecRight, vecTargetRight, pTargetEnt, TRUE, NULL ) == LOCALMOVE_VALID )
			{
				if ( pApex )
				{
					*pApex = vecRight;
				}

				return TRUE;
			}else{
				//MODDD - How about going up from that point a bit?
				if (isMovetypeFlying()){
					if ( CheckLocalMove ( pev->origin, vecRightTop, pTargetEnt, FALSE, NULL ) == LOCALMOVE_VALID ){
						if ( CheckLocalMove ( vecRightTop, vecTargetRight, pTargetEnt, TRUE, NULL ) == LOCALMOVE_VALID )
						{
							if ( pApex )
							{
								*pApex = vecRightTop;
							}
							return TRUE;
						}
					}
				}
				/////////////////////////////////////////////////////////////////////////////////////////////////

			}
		}
		if ( CheckLocalMove( pev->origin, vecLeft, pTargetEnt, FALSE, NULL ) == LOCALMOVE_VALID )
		{
			if ( CheckLocalMove ( vecLeft, vecTargetLeft, pTargetEnt, TRUE, NULL ) == LOCALMOVE_VALID )
			{
				if ( pApex )
				{
					*pApex = vecLeft;
				}

				return TRUE;
			}else{
				//MODDD - How about going up from that point a bit?
				if (isMovetypeFlying()){
					if ( CheckLocalMove ( pev->origin, vecLeftTop, pTargetEnt, FALSE, NULL ) == LOCALMOVE_VALID ){
						if ( CheckLocalMove ( vecLeftTop, vecTargetLeft, pTargetEnt, TRUE, NULL ) == LOCALMOVE_VALID )
						{
							if ( pApex )
							{
								*pApex = vecLeftTop;
							}
							return TRUE;
						}
					}
				}
				/////////////////////////////////////////////////////////////////////////////////////////////////

			}
		}

		if (isMovetypeFlying())
		{
			//UTIL_drawLineFrame(pev->origin.x, pev->origin.y, pev->origin.z, vecTop.x, vecTop.y, vecTop.z, 12, 900, 255, 255, 255);
			//UTIL_drawLineFrame(vecTop.x, vecTop.y, vecTop.z, vecTargetTop.x, vecTargetTop.y, vecTargetTop.z, 12, 900, 255, 255, 255);
			if ( CheckLocalMove( pev->origin, vecTop, pTargetEnt, FALSE, NULL ) == LOCALMOVE_VALID)
			{
				if ( CheckLocalMove ( vecTop, vecTargetTop, pTargetEnt, TRUE, NULL ) == LOCALMOVE_VALID )
				{
					if ( pApex )
					{
						*pApex = vecTop;
						//ALERT(at_aiconsole, "triangulate over\n");
					}

					return TRUE;
				}
			}
#if 1
			if ( CheckLocalMove( pev->origin, vecBottom, pTargetEnt, FALSE, NULL ) == LOCALMOVE_VALID )
			{
				if ( CheckLocalMove ( vecBottom, vecTargetBottom, pTargetEnt, TRUE, NULL ) == LOCALMOVE_VALID )
				{
					if ( pApex )
					{
						*pApex = vecBottom;
						//ALERT(at_aiconsole, "triangulate under\n");
					}

					return TRUE;
				}
			}
#endif
		}

		vecLeft = vecLeft - vecRightShiftApex;
		vecRight = vecRight + vecRightShiftApex;
		if (isMovetypeFlying()){
			vecTop = vecTop + vecUpShiftApex;
			vecBottom = vecBottom - vecUpShiftApex;

			//MODDD - new
			vecLeftTop = vecLeftTop + -vecRightShiftApex * 0.75 + vecUpShiftApex * 0.75;
			vecRightTop = vecRightTop + vecRightShiftApex * 0.75 + vecUpShiftApex * 0.75;
		}
		*/


		if ( sourceMaxRightHit < 2 && CheckLocalMove ( vecRight, vecTargetRight, pTargetEnt, TRUE, NULL ) == LOCALMOVE_VALID ){
			if ( pApex )
			{
				*pApex = vecRight;
			}

			return TRUE;
		}else{
			//MODDD - How about going up from that point a bit?
			if (isMovetypeFlying()){
				if ( sourceMaxRightUpHit < 2 && CheckLocalMove ( vecRightTop, vecTargetRight, pTargetEnt, TRUE, NULL ) == LOCALMOVE_VALID )
				{
					if ( pApex )
					{
						*pApex = vecRightTop;
					}
					return TRUE;
				}
			}

			if(sourceMaxRightHit == 2){
				// One other try:
				Vector outVec;
				BOOL newTargetSuccess = FPutFailedGoalAlongSurface(vecRight, 20, sizeX * 2.5, pTargetEnt, &outVec);
				if(newTargetSuccess){
					// try from outVec, one last shot
					if (CheckLocalMove ( outVec, vecTargetRight, pTargetEnt, TRUE, NULL ) == LOCALMOVE_VALID ){
						if ( pApex ){
							*pApex = outVec;
						}
						return TRUE;
					}
				}
			}

			/////////////////////////////////////////////////////////////////////////////////////////////////
		}//vecRight to vecTargetRight stuff


		/*
		if(sourceMaxLeftHit < 2){
			UTIL_drawLineFrame(pev->origin.x, pev->origin.y, pev->origin.z, vecLeft.x, vecLeft.y, vecLeft.z, 12, 200, 255, 255, 255);
			UTIL_drawLineFrame(vecLeft.x, vecLeft.y, vecLeft.z, vecTargetLeft.x, vecTargetLeft.y, vecTargetLeft.z, 12, 200, 255, 255, 255);
		}else{
			UTIL_drawLineFrame(pev->origin.x, pev->origin.y, pev->origin.z, vecLeft.x, vecLeft.y, vecLeft.z, 12, 200, 255, 0, 0);
			UTIL_drawLineFrame(vecLeft.x, vecLeft.y, vecLeft.z, vecTargetLeft.x, vecTargetLeft.y, vecTargetLeft.z, 12, 200, 255, 0, 0);
		}
		*/
		if ( sourceMaxLeftHit < 2 && CheckLocalMove ( vecLeft, vecTargetLeft, pTargetEnt, TRUE, NULL ) == LOCALMOVE_VALID )
		{
			if ( pApex )
			{
				*pApex = vecLeft;
			}

			return TRUE;
		}else{
			//MODDD - How about going up from that point a bit?
			if (isMovetypeFlying()){
				if ( sourceMaxLeftUpHit < 2 && CheckLocalMove ( vecLeftTop, vecTargetLeft, pTargetEnt, TRUE, NULL ) == LOCALMOVE_VALID )
				{
					if ( pApex )
					{
						*pApex = vecLeftTop;
					}
					return TRUE;
				}
			}

			if(sourceMaxLeftHit == 2){
				// One other try:
				Vector outVec;
				BOOL newTargetSuccess = FPutFailedGoalAlongSurface(vecLeft, 20, sizeX * 2.5, pTargetEnt, &outVec);
				if(newTargetSuccess){
					// try from outVec, one last shot
					if (CheckLocalMove ( outVec, vecTargetLeft, pTargetEnt, TRUE, NULL ) == LOCALMOVE_VALID ){
						if ( pApex ){
							*pApex = outVec;
						}
						return TRUE;
					}
				}
			}
			/////////////////////////////////////////////////////////////////////////////////////////////////

		}

		if (isMovetypeFlying())
		{
			//UTIL_drawLineFrame(pev->origin.x, pev->origin.y, pev->origin.z, vecTop.x, vecTop.y, vecTop.z, 12, 900, 255, 255, 255);
			//UTIL_drawLineFrame(vecTop.x, vecTop.y, vecTop.z, vecTargetTop.x, vecTargetTop.y, vecTargetTop.z, 12, 900, 255, 255, 255);
			
			if ( sourceMaxUpHit < 2 && CheckLocalMove ( vecTop, vecTargetTop, pTargetEnt, TRUE, NULL ) == LOCALMOVE_VALID )
			{
				if ( pApex )
				{
					*pApex = vecTop;
					//ALERT(at_aiconsole, "triangulate over\n");
				}

				return TRUE;
			}else{
				//MODDD - what if I go against the ceiling?
				if(sourceMaxLeftHit == 2){
					// One other try:
					Vector outVec;
					BOOL newTargetSuccess = FPutFailedGoalAlongSurface(vecTop, -15, sizeZ * 2.5, pTargetEnt, &outVec);
					if(newTargetSuccess){
						// try from outVec, one last shot
						if (CheckLocalMove ( outVec, vecTargetTop, pTargetEnt, TRUE, NULL ) == LOCALMOVE_VALID ){
							if ( pApex ){
								*pApex = outVec;
							}
							return TRUE;
						}
					}
				}
			}//vecTop attempt
#if 1
			
			if ( sourceMaxDownHit < 2 && CheckLocalMove ( vecBottom, vecTargetBottom, pTargetEnt, TRUE, NULL ) == LOCALMOVE_VALID )
			{
				if ( pApex )
				{
					*pApex = vecBottom;
					//ALERT(at_aiconsole, "triangulate under\n");
				}

				return TRUE;
			}else{
				//MODDD - what if I go against the ceiling?
				if(sourceMaxLeftHit == 2){
					// One other try:
					Vector outVec;
					BOOL newTargetSuccess = FPutFailedGoalAlongSurface(vecBottom, 15, sizeZ * 2.5, pTargetEnt, &outVec);
					if(newTargetSuccess){
						// try from outVec, one last shot
						if (CheckLocalMove ( outVec, vecTargetBottom, pTargetEnt, TRUE, NULL ) == LOCALMOVE_VALID ){
							if ( pApex ){
								*pApex = outVec;
							}
							return TRUE;
						}
					}
				}
			}//vecTargetBottom attempt
#endif
		}


		// Any set to 1 that tried above, will be 2 next time (never try again if this failed, never changes
		// so why bother)
		if (sourceMaxLeftHit >= 1) sourceMaxLeftHit++;
		if (sourceMaxRightHit >= 1) sourceMaxRightHit++;
		if (sourceMaxUpHit >= 1) sourceMaxUpHit++;
		if (sourceMaxDownHit >= 1) sourceMaxDownHit++;
		if (sourceMaxLeftUpHit >= 1) sourceMaxLeftUpHit++;
		if (sourceMaxRightUpHit >= 1) sourceMaxRightUpHit++;


		if(sourceMaxRightHit == 0){
			vecTest = vecRight + vecRightShiftApex;
			// Can I go from the current vecRight to the further extent
			if(CheckLocalMove(vecRight, vecTest, pTargetEnt, FALSE, &furthestDist) == LOCALMOVE_VALID){
				// ok, use exactly that next time.
				vecRight = vecTest;
			}else{
				// where did I stop?  Look to 'furthestDist', now guaranteed set by CheckLocalMove.
				// And don't try nudging in this direction in the future.
				if(furthestDist > 24){
					vecRight = vecRight + vecDirRight * (furthestDist - 16);
				}
				sourceMaxRightHit = 1;
			}
		}
		if(sourceMaxLeftHit == 0){
			vecTest = vecLeft + -vecRightShiftApex;
			if(CheckLocalMove(vecLeft, vecTest, pTargetEnt, FALSE, &furthestDist) == LOCALMOVE_VALID){
				// ok, use exactly that next time.
				vecLeft = vecTest;
			}else{
				// where did I stop?  Look to 'furthestDist', now guaranteed set by CheckLocalMove.
				// And don't try nudging in this direction in the future.
				if(furthestDist > 24){
					vecLeft = vecLeft + -vecDirRight * (furthestDist - 16);
				}
				sourceMaxLeftHit = 1;
			}
		}
		if (isMovetypeFlying()){
			if(sourceMaxUpHit == 0){
				vecTest = vecTop + vecUpShiftApex;
				if(CheckLocalMove(vecTop, vecTest, pTargetEnt, FALSE, &furthestDist) == LOCALMOVE_VALID){
					vecTop = vecTest;
				}else{
					if(furthestDist > 24){
						vecTop = vecTop + vecDirUp * (furthestDist - 16);
					}
					sourceMaxUpHit = 1;
				}
			}
			if(sourceMaxDownHit == 0){
				vecTest = vecBottom + -vecUpShiftApex;
				if(CheckLocalMove(vecBottom, vecTest, pTargetEnt, FALSE, &furthestDist) == LOCALMOVE_VALID){
					vecBottom = vecTest;
				}else{
					if(furthestDist > 24){
						vecBottom = vecBottom + -vecDirUp * (furthestDist - 16);
					}
					sourceMaxDownHit = 1;
				}
			}


			if(sourceMaxLeftUpHit == 0){
				vecTest = vecLeftTop + -vecRightShiftApex * 0.75 + vecUpShiftApex * 0.75;
				if(CheckLocalMove(vecLeftTop, vecTest, pTargetEnt, FALSE, &furthestDist) == LOCALMOVE_VALID){
					vecLeftTop = vecTest;
				}else{
					if(furthestDist > 24){
						vecLeftTop = vecLeftTop + vecDirLeftUp * (furthestDist - 16);
					}
					sourceMaxLeftUpHit = 1;
				}
			}
			if(sourceMaxRightUpHit == 0){
				vecTest = vecRightTop + vecRightShiftApex * 0.75 + vecUpShiftApex * 0.75;
				if(CheckLocalMove(vecRightTop, vecTest, pTargetEnt, FALSE, &furthestDist) == LOCALMOVE_VALID){
					vecRightTop = vecTest;
				}else{
					if(furthestDist > 24){
						vecRightTop = vecRightTop + vecDirRightUp * (furthestDist - 16);
					}
					sourceMaxRightUpHit = 1;
				}
			}

		}//isMovetypeFlying




		/*
		//MODDD
		/////////////////////////////////////////////////////////////////////////////////////
		if(!targetMaxLeftHit){
			vecTest = vecTargetLeft + vecDirRight * (targetSizeX * 0.23);
			// Try again, from current vecTargetLeft to itself nudged further in that direction
			if(CheckLocalMove(vecTargetLeft, vecTest, pTargetEnt, FALSE, &furthestDist) == LOCALMOVE_VALID){
				// ok, use exactly that next time.
				vecTargetLeft = vecTest;
			}else{
				// where did I stop?  Look to 'furthestDist', now guaranteed set by CheckLocalMove.
				// And don't try nudging in this direction in the future.
				if(furthestDist > 10){
					vecTargetLeft = vecTargetLeft + vecDirRight * (furthestDist - 6);
				}
				targetMaxLeftHit = TRUE;
			}
		}

		if(!targetMaxRightHit){
			vecTest = vecTargetRight + -vecDirRight * (targetSizeX * 0.23);
			if(CheckLocalMove(vecTargetRight, vecTest, pTargetEnt, FALSE, &furthestDist) == LOCALMOVE_VALID){
				vecTargetRight = vecTest;
			}else{
				if(furthestDist > 10){
					vecTargetRight = vecTargetRight + -vecDirRight * (furthestDist - 6);
				}
				targetMaxRightHit = TRUE;
			}
		}
	
		if (isMovetypeFlying()){
			vecTest = vecTargetTop + vecDirUp * (targetSizeZ * 0.18);
			if(CheckLocalMove(vecTargetTop, vecTest, pTargetEnt, FALSE, &furthestDist) == LOCALMOVE_VALID){
				vecTargetTop = vecTest;
			}else{
				if(furthestDist > 10){
					vecTargetTop = vecTargetTop + vecDirUp * (furthestDist - 6);
				}
				targetMaxTopHit = TRUE;
			}
		}
		/////////////////////////////////////////////////////////////////////////////////////
		*/

	}//loop through triangulation attempts

	return FALSE;
}//FTriangulate



//MODDD - FTriangualte assistance.
// Idea: this goal failed (some distance from the monster origin left, right, up, down failed while at as
// far of an extent as it could be), but there could be some surface in the way that the monster could have
// walked along instead.
// Idea: Take a trace in the direction of the failed goal and see if there is a blocking surface in the way.
// Get a vector that travels along the surface toward where I wanted to go.  Thank you vector-projection-
// onto-a-plane.
// Then go that ways along the rest of the distance I failed or at the earliest point it gets blocked.
// Maybe after going along that wall, there will be a straight shot to my target node or enemy.
// Unfortunately, I don't know of any simple way to detect the direction to move against any other otherwise
// impassable area, such as a cliff.  Blocks movement from too deep a change in elevation, not a concrete
// blocking surface in the way.  This trace would miss and have no effect then.
BOOL CBaseMonster::FPutFailedGoalAlongSurface(Vector vecFailedGoal, float flLookZOffset, float flAlongSurfaceDist, CBaseEntity* pTargetEnt, Vector* out_vecNewTarget){

	/*
	if(furthestDist > 16){
		TraceResult tr;
		Vector pointDelta = vecLeft - pev->origin;
		Vector theDir = pointDelta.Normalize();
		float theDist = pointDelta.Length();
		Vector vecStart = pev->origin + Vector(0, 0, 20);
		UTIL_TraceLine(vecStart, vecLeft, dont_ignore_monsters, ENT(pev), &tr);

		if(tr.flFraction < 1.0){
			Vector theNormal = tr.vecPlaneNormal;
			Vector dirAlongWall = theDir - ((DotProduct(theDir, theNormal)) / theNormal.Length()*theNormal.Length())*theNormal;
			Vector fromWall = tr.vecEndPos + pev->size.x * 0.75 * theNormal;
			Vector alongWall = fromWall + dirAlongWall * (theDist - furthestDist - 16);
					
			// Now, try from pev->origin to alongWall.
			//UTIL_drawLineFrame(pev->origin.x, pev->origin.y, pev->origin.z, alongWall.x, alongWall.y, alongWall.z, 12, 900, 255, 255, 0);
			//UTIL_drawLineFrame(alongWall.x, alongWall.y, alongWall.z, vecFarSide.x, vecFarSide.y, vecFarSide.z, 12, 900, 255, 255, 0);

			if(CheckLocalMove(pev->origin, alongWall, pTargetEnt, FALSE, &furthestDist) == LOCALMOVE_VALID){
				// ok, try this next then.
				vecLeft = alongWall;
				sourceMaxLeftHit = 1;
			}else{
				if(furthestDist > 24){
					// go up as far as possible
					Vector pointDelta = alongWall - pev->origin;
					Vector theDir = pointDelta.Normalize();
					vecLeft = pev->origin + theDir * (furthestDist - 16);
				}else{
					sourceMaxLeftHit = 2;
				}
			}

		}
	}
	*/

	//HACK: for debug feature only
	Vector vecFarSide = m_Route[m_iRouteIndex].vecLocation;

	//Vector vecWallHit, Vector vecWallNormal
	
	TraceResult tr;
	Vector pointDelta = vecFailedGoal - pev->origin;
	Vector theDir = pointDelta.Normalize();
	float theDist = pointDelta.Length();
	Vector vecStart = pev->origin;
	vecStart.z += flLookZOffset;

	// go further than the failed direction in case this is a funny angle and the wall would take aways
	// to get to (theDist + 128)
	Vector vecEnd = vecStart + theDir * (theDist + 128);
	//UTIL_TraceLine(vecStart, vecFailedGoal, dont_ignore_monsters, ENT(pev), &tr);
	UTIL_TraceLine(vecStart, vecEnd, dont_ignore_monsters, ENT(pev), &tr);
	//UTIL_drawLineFrame(vecStart.x, vecStart.y, vecStart.z, vecEnd.x, vecEnd.y, vecEnd.z, 12, 300, 0, 255, 0);
	

	if(tr.flFraction < 1.0){
		float furthestDist;

		// From here to the failed goal, how far could I get?
		// No, nevermind.  Provided by caller now (flAlongSurfaceDist)
		//float furthestDist = (tr.flFraction) * theDist;
		//float furthestDist = (tr.vecEndPos - vecStart).Length();

		Vector theNormal = tr.vecPlaneNormal;
		// Vector projection onto a plane formula.  Although 'theNormal has length 1...
		//Vector dirAlongWall = theDir - ((DotProduct(theDir, theNormal)) / theNormal.Length()*theNormal.Length())*theNormal;
		Vector dirAlongWall = theDir - ((DotProduct(theDir, theNormal)))*theNormal;

		// get far enough away from the wall so another check doesn't bump into it.
		Vector fromWall = tr.vecEndPos + pev->size.x * 0.75 * theNormal;
		// And travel along it aways.
		// was (theDist - furthestDist).LengtH() in there?
		Vector alongWall = fromWall + dirAlongWall * (flAlongSurfaceDist);
					
		// Now, try from pev->origin to alongWall.
		//UTIL_drawLineFrame(pev->origin.x, pev->origin.y, pev->origin.z, alongWall.x, alongWall.y, alongWall.z, 12, 900, 255, 255, 0);
		//UTIL_drawLineFrame(alongWall.x, alongWall.y, alongWall.z, vecFarSide.x, vecFarSide.y, vecFarSide.z, 12, 900, 255, 255, 0);

		if(CheckLocalMove(pev->origin, alongWall, pTargetEnt, FALSE, &furthestDist) == LOCALMOVE_VALID){
			// ok, try this next then.
			*out_vecNewTarget = alongWall;
			return TRUE;
			//sourceMaxLeftHit = 1;
		}else{
			if(furthestDist > 24){
				// go up as far as possible
				Vector pointDelta = alongWall - pev->origin;
				Vector theDir = pointDelta.Normalize();
				*out_vecNewTarget = pev->origin + theDir * (furthestDist - 16);
				return TRUE;
			}else{
				// sourceMaxLeftHit = 2;
			}
		}

	}

	// sign to give up
	return FALSE;
}//???










// NOTICE - monsters can opt in or out of the segmented move feature.
// Ones that rely on the Move method as it is (or just cuch call the parent CBaseMonster::Move without
// doing much else, like CFlyingMonster) can keep this True.
// Ones that replace it completely are harder to figure out.  Just say FALSE to stop other calls put in
// FRefreshRoute that usually wouldn't occur.
BOOL CBaseMonster::usesSegmentedMove(void) {
	return TRUE;
}

//MODDD - beginning portion of the as-is 'Move' method below moved here for methods to see whether the path
// will still be valid in one move from now.
// If not, setting the idealActivity to WALK or RUN will give an awkward flinching effect since the monster
// calls Move and goes  'oh wait I can't do that'.  When it should've realized that before deciding to look
// like it's moving.
// Also expects references to variables expected to be filled by the Move call this is likely to come before.
// NOTICE - this can also return a special code, '-1', which means to stop the rest of the Move method.
// Don't even bother with any "If failure do this" logic, just 'return', end the Move method right there.

// ALSO, for monsters that implemented their own versions of Move as-is.
// Implementing MovePRE is up to choice, but for monsters with very custom Move script (like CController),
// especially to the point of not even calling the parent CBaseMonster::Move, it may be best to completely 
// redo MovePRE without any reference to CBaseMonster::MovePRE.  Even if it's as simple as "return TRUE;" to
// let pre-pathfinding checks just work like they did in retail (no need to call it in the monster's Move method
// if it's that useless), best for safety for now unless there's a reason
// to do otherwise.
// For monsters that don't do much else beyond call CBaseMonster::Move in theirs (like CFlyingMonster or whichever),
// it should be fine to leave MovePRE as the default.  After all, that class relied on defaults too.
int CBaseMonster::MovePRE(float flInterval, float& flWaypointDist, float& flCheckDist, float& flDist, Vector& vecDir, CBaseEntity*& pTargetEnt ) {

	if (drawPathConstant) {
		DrawMyRoute(0, 0, 176);
	}


	/*
	// OLD WAY.
	// For a version 99% accurate to retail, see this.  Only modified the end slightly to send the result of CheckLocalMove to be returned by here
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// vars disabled, supplied by the parameter list for setting.
	//float		flWaypointDist;
	//float		flCheckDist;
	//float		flDist;// how far the lookahead check got before hitting an object.
	//Vector		vecDir;
	//Vector		vecApex;
	//CBaseEntity	*pTargetEnt;

	// Don't move if no valid route
	if ( FRouteClear() )
	{
		// If we still have a movement goal, then this is probably a route truncated by SimplifyRoute()
		// so refresh it.
		if ( m_movementGoal == MOVEGOAL_NONE || !FRefreshRoute() )
		{
			ALERT( at_aiconsole, "Tried to move with no route!\n" );
			TaskFail();
			return -1;
		}
	}

	if ( m_flMoveWaitFinished > gpGlobals->time )
		return -1;

// Debug, test movement code
#if 0
//	if ( CVAR_GET_FLOAT("stopmove" ) != 0 )
	{
		if ( m_movementGoal == MOVEGOAL_ENEMY )
			RouteSimplify( m_hEnemy );
		else
			RouteSimplify( m_hTargetEnt );
		FRefreshRoute();
		return -1;
	}
#else
// Debug, draw the route
//	DrawRoute( pev, m_Route, m_iRouteIndex, 0, 200, 0 );
#endif

	// if the monster is moving directly towards an entity (enemy for instance), we'll set this pointer
	// to that entity for the CheckLocalMove and Triangulate functions.
	pTargetEnt = NULL;

	// local move to waypoint.
	vecDir = ( m_Route[ m_iRouteIndex ].vecLocation - pev->origin ).Normalize();
	flWaypointDist = ( m_Route[ m_iRouteIndex ].vecLocation - pev->origin ).Length2D();

	MakeIdealYaw ( m_Route[ m_iRouteIndex ].vecLocation );
	ChangeYaw ( pev->yaw_speed );

	// if the waypoint is closer than CheckDist, CheckDist is the dist to waypoint
	if ( flWaypointDist < DIST_TO_CHECK )
	{
		flCheckDist = flWaypointDist;
	}
	else
	{
		flCheckDist = DIST_TO_CHECK;
	}

	if ( (m_Route[ m_iRouteIndex ].iType & (~bits_MF_NOT_TO_MASK)) == bits_MF_TO_ENEMY )
	{
		// only on a PURE move to enemy ( i.e., ONLY MF_TO_ENEMY set, not MF_TO_ENEMY and DETOUR )
		pTargetEnt = m_hEnemy;
	}
	else if ( (m_Route[ m_iRouteIndex ].iType & ~bits_MF_NOT_TO_MASK) == bits_MF_TO_TARGETENT )
	{
		pTargetEnt = m_hTargetEnt;
	}

	// !!!BUGBUG - CheckDist should be derived from ground speed.
	// If this fails, it should be because of some dynamic entity blocking this guy.
	// We've already checked this path, so we should wait and time out if the entity doesn't move
	flDist = 0;
	
	// !!! redirect to return!  was if(...)
	// Also, skipping the Z-check by sending FALSE, 2nd to last in the param list.  Retail would've always done it.
	BOOL localMoveResult = (CheckLocalMove ( pev->origin, pev->origin + vecDir * flCheckDist, pTargetEnt, FALSE, &flDist ) == LOCALMOVE_VALID);

	return localMoveResult;
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	*/


	// NEW WAY
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//MODDD - these are now provided by the one calling the method, in case this is to be continued by the
	// Move call below (expects these to have been filled)
	//float	flWaypointDist;
	//float	flCheckDist;
	//float	flDist;// how far the lookahead check got before hitting an object.
	//Vector		vecDir;
	////Vector		vecApex;
	//CBaseEntity* pTargetEnt;


	// Don't move if no valid route
	if (FRouteClear())
	{
		// If we still have a movement goal, then this is probably a route truncated by SimplifyRoute()
		// so refresh it.
		if (m_movementGoal == MOVEGOAL_NONE || !FRefreshRoute())
		{
			ALERT(at_aiconsole, "Tried to move with no route!\n");
			TaskFail();
			return -1;
		}
	}

	if (m_flMoveWaitFinished > gpGlobals->time) {
		// Wait.  Is this the same as 'abandon route' failure or just deciding not to do move logic this frame?
		return -1;
	}



	// Debug, test movement code
#if 0
//	if ( CVAR_GET_FLOAT("stopmove" ) != 0 )
	{
		if (m_movementGoal == MOVEGOAL_ENEMY)
			RouteSimplify(m_hEnemy);
		else
			RouteSimplify(m_hTargetEnt);
		FRefreshRoute();
		return;
	}
#else
// Debug, draw the route
//	DrawRoute( pev, m_Route, m_iRouteIndex, 0, 200, 0 );
#endif

	// if the monster is moving directly towards an entity (enemy for instance), we'll set this pointer
	// to that entity for the CheckLocalMove and Triangulate functions.
	pTargetEnt = NULL;

	// local move to waypoint.
	vecDir = (m_Route[m_iRouteIndex].vecLocation - pev->origin).Normalize();

	//MODDD - HOLD ON!  Why is this always "Length2D()"?  Even for flyers? 
	//        When would flyers call for Length2D instead of typical Length (includes Z axis)?
	//flWaypointDist = ( m_Route[ m_iRouteIndex ].vecLocation - pev->origin ).Length2D();

	if (!isMovetypeFlying()) {
		//not a flyer? Default behavior.
		flWaypointDist = (m_Route[m_iRouteIndex].vecLocation - pev->origin).Length2D();
	}
	else {
		//Otherwise, use 3D distance instead.  If there's ever a time the Length2D is still preferred... eh. why?
		flWaypointDist = (m_Route[m_iRouteIndex].vecLocation - pev->origin).Length();
	}


	// don't rotate if this is only checking to see whether I am up to so far along a route just by standing here
	if(!g_pathfind_preMoveOnly){
		MakeIdealYaw(m_Route[m_iRouteIndex].vecLocation);
		ChangeYaw(pev->yaw_speed);
	}

	if (m_movementGoal == MOVEGOAL_NONE) {
		//MODDD - YEAH
		//no more! Cheap trick to interrupt the movement method if ChangeYaw decides to clear the route.
		return -1;
	}


	BOOL checkTheZ;

	// if the waypoint is closer than CheckDist, CheckDist is the dist to waypoint
	if (flWaypointDist < DIST_TO_CHECK)
	{
		flCheckDist = flWaypointDist;

		checkTheZ = TRUE;
	}
	else
	{
		flCheckDist = DIST_TO_CHECK;
		// incomplete dist?  Don't check the Z, 
		// Wait!
		if(
			// Nevermind the bits_MF_TO_ENEMY and bits_MF_TO_TARGET checks.
			// If this is to the end of the route, do the Z check.
			// Nope, not even here!  This isn't to the point itself, so the Z check isn't useful at all yet.
			//m_Route[m_iRouteIndex].iType & (bits_MF_TO_ENEMY | bits_MF_TO_TARGETENT) &&
			m_Route[m_iRouteIndex].iType & (bits_MF_IS_GOAL)
		){
			//checkTheZ = TRUE;
			checkTheZ = FALSE;
		}else{
			checkTheZ = FALSE;
		}
	}



	//MODDD - nevermind this check completely now, just use 'getGoalEnt'.
	// Doesn't matter how far along I am on my path, if the m_hEnemy or m_hTargetEnt (whichever
	// applicable) is in the way, I still allow the path, right?  I should see the enemy and stop
	// to attack sooner anyway.
	// Problem with this strict of a requirement is, SimplyRoute could produce a route that has very
	// little distance between the 2nd to last node and the end node.  So the route sees it is going
	// to the 2nd ot last node a detour node) and doesn't think to allow the enemy as an exception,
	// even though both that and the goal nodes overlay with the enemy.  So pathfinding just goes
	// 'Oh, no target, but I hit something? fail', even though that 'something' is my enemy that I want
	// to reach...
	pTargetEnt = GetGoalEntity();

	/*
	//MODDD - bit checks instead now.
	// Actualy leeeeeeet's be careful about this kind of thing,  keep the strict 'bits_MF_TO_ENEMY' exclusiveness.
	if ( (m_Route[ m_iRouteIndex ].iType & (~bits_MF_NOT_TO_MASK)) == bits_MF_TO_ENEMY )
	//if ((m_Route[m_iRouteIndex].iType & (~bits_MF_NOT_TO_MASK)) & bits_MF_TO_ENEMY)
	{
		// only on a PURE move to enemy ( i.e., ONLY MF_TO_ENEMY set, not MF_TO_ENEMY and DETOUR )
		pTargetEnt = m_hEnemy;
	}
	else if ( (m_Route[ m_iRouteIndex ].iType & ~bits_MF_NOT_TO_MASK) == bits_MF_TO_TARGETENT )
	//else if ((m_Route[m_iRouteIndex].iType & ~bits_MF_NOT_TO_MASK) & bits_MF_TO_TARGETENT)
	{
		pTargetEnt = m_hTargetEnt;
	}
	*/



	//MODDD - Below is completely new!!!

	if (EASY_CVAR_GET_DEBUGONLY(crazyMonsterPrintouts) == 1) {
		if (pTargetEnt == NULL) {
			easyPrintLine("I AM OBNOXIOUS: NULL : %d", m_Route[m_iRouteIndex].iType);
		}
		else {
			easyPrintLine("I AM OBNOXIOUS: %s %d", STRING(pTargetEnt->pev->classname), m_Route[m_iRouteIndex].iType);
		}
	}

	if (EASY_CVAR_GET_DEBUGONLY(applyLKPPathFixToAll) == 1 || hasSeeEnemyFix() && !(m_Route[m_iRouteIndex].iType & bits_MF_TO_ENEMY) && HasConditions(bits_COND_SEE_ENEMY)) {
		//this is a fix to make the enemy re-route in case of seeing the enemy while taking a path not necessarily to the enemy (such as, on its way to a last known location).
		this->MovementComplete();
		return -1;
	}


	// !!!BUGBUG - CheckDist should be derived from ground speed.
	// If this fails, it should be because of some dynamic entity blocking this guy.
	// We've already checked this path, so we should wait and time out if the entity doesn't move
	flDist = 0;


	//BOOL localMovePass = (CheckLocalMove ( pev->origin, pev->origin + vecDir * flWaypointDist, pTargetEnt, checkTheZ, &flDist ) == LOCALMOVE_VALID);

	BOOL localMovePass;

	//If using a RAMPFIX or NODE type of node, use "CheckLocalMoveHull" instead. It's a bit less strict.
	int useHullCheckMask = bits_MF_RAMPFIX;
	if (EASY_CVAR_GET_DEBUGONLY(pathfindLooseMapNodes) == 1) {
		useHullCheckMask |= bits_MF_TO_NODE;
	}


	g_CheckLocalMoveCanReportBlocker = TRUE;
	// Let this check draw extra info, checking some dozens of nodes and printing out any reason they can't work is obnoxious

	Vector vecCheckStart = pev->origin;
	Vector vecCheckEnd = pev->origin + vecDir * flCheckDist;


	if ((m_Route[m_iRouteIndex].iType & ~bits_MF_NOT_TO_MASK) & (useHullCheckMask)) {
		//for now...
		localMovePass = (CheckLocalMoveHull(vecCheckStart, vecCheckEnd, pTargetEnt, &flDist) == LOCALMOVE_VALID);
		//localMovePass = TRUE;

		//DebugLine_Setup(0, pev->origin, vecCheckEnd, 0, 255, 0);

		if (!localMovePass) {
			EASY_CVAR_PRINTIF_PRE(pathfindPrintout, easyForcePrintLine("%s:ID%d Move: CheckLocalMoveHull Failed!!!", getClassnameShort(), monsterID));
		}

	}
	else {
		localMovePass = (CheckLocalMove(vecCheckStart, vecCheckEnd, pTargetEnt, checkTheZ, &flDist) == LOCALMOVE_VALID);

		
		//if (localMovePass == 1) {
		//	DebugLine_Setup(0, pev->origin, vecCheckEnd, 0, 255, 0);
		//}
		//else {
		//	DebugLine_Setup(0, pev->origin, vecCheckEnd, 255, 0, 0);
		//}
		

		if (localMovePass) {
			//if it passed, likely didn't bother writing anything to flDist. Just go ahead and assume it was full, that is what passing means.
			flDist = flCheckDist;
		}

		if (drawPathConstant) {
			//Show the result of the recent localMove?
			::DebugLine_Setup(0, pev->origin, vecCheckEnd, (flDist / flCheckDist));
		}

		if (!localMovePass) {
			EASY_CVAR_PRINTIF_PRE(pathfindPrintout, easyForcePrintLine("%s:ID%d Move: CheckLocalMove Failed!", getClassnameShort(), monsterID));
		}
	}

	g_CheckLocalMoveCanReportBlocker = FALSE;

	return localMovePass;
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}//MovePRE




void CBaseMonster::Move ( float flInterval ) 
{
	Vector vecApex;

	float flWaypointDist;
	float flCheckDist;
	float flDist;
	Vector vecDir;
	CBaseEntity* pTargetEnt;

	//MODDD - section moved to its own method, MovePRE, as a simple way to see if a route looks promising at other points.
	// Jumping to a movement activity only to give up the next frame causes twiching between moving/standing anims.


	int localMovePass = MovePRE(flInterval, flWaypointDist, flCheckDist, flDist, vecDir, pTargetEnt);

	if (localMovePass == -1) {
		// Signal to end very early.  Retail never returned this from any script that would've been there so it is safe
		// to trust this was given intentionally if testing retail script (end early, even for old logic below).
		return;
	}


	/*
	// OLD WAY
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// !!! changed to tie into the new modular form
	//if (CheckLocalMove(pev->origin, pev->origin + vecDir * flCheckDist, pTargetEnt, FALSE, &flDist) != LOCALMOVE_VALID)
	if(localMovePass != 1)
	{
		CBaseEntity* pBlocker;

		// Can't move, stop
		Stop();
		// Blocking entity is in global trace_ent
		pBlocker = CBaseEntity::Instance(gpGlobals->trace_ent);
		if (pBlocker)
		{
			DispatchBlocked(edict(), pBlocker->edict());
		}

		if (pBlocker && m_moveWaitTime > 0 && pBlocker->IsMoving() && !pBlocker->IsPlayer() && (gpGlobals->time - m_flMoveWaitFinished) > 3.0)
		{
			// Can we still move toward our target?
			if (flDist < m_flGroundSpeed)
			{
				// No, Wait for a second
				m_flMoveWaitFinished = gpGlobals->time + m_moveWaitTime;
				return;
			}
			// Ok, still enough room to take a step
		}
		else
		{
			// try to triangulate around whatever is in the way.
			if (FTriangulate(pev->origin, m_Route[m_iRouteIndex].vecLocation, flDist, pTargetEnt, &vecApex))
			{
				InsertWaypoint(vecApex, bits_MF_TO_DETOUR);
				RouteSimplify(pTargetEnt);
			}
			else
			{
				//				ALERT ( at_aiconsole, "Couldn't Triangulate\n" );
				Stop();
				// Only do this once until your route is cleared
				if (m_moveWaitTime > 0 && !(m_afMemory & bits_MEMORY_MOVE_FAILED))
				{
					FRefreshRoute();
					if (FRouteClear())
					{
						TaskFail();
					}
					else
					{
						// Don't get stuck
						if ((gpGlobals->time - m_flMoveWaitFinished) < 0.2)
							Remember(bits_MEMORY_MOVE_FAILED);

						m_flMoveWaitFinished = gpGlobals->time + 0.1;
					}
				}
				else
				{
					TaskFail();
					ALERT(at_aiconsole, "%s Failed to move (%d)!\n", STRING(pev->classname), HasMemory(bits_MEMORY_MOVE_FAILED));
					//ALERT( at_aiconsole, "%f, %f, %f\n", pev->origin.z, (pev->origin + (vecDir * flCheckDist)).z, m_Route[m_iRouteIndex].vecLocation.z );
				}
				return;
			}
		}
	}
	

	//MODDD - ShouldAdvanceRoute and AdvanceRoute now take flInterval too.
	// close enough to the target, now advance to the next target. This is done before actually reaching
	// the target so that we get a nice natural turn while moving.
	if (ShouldAdvanceRoute(flWaypointDist, flInterval))///!!!BUGBUG- magic number
	{
		AdvanceRoute(flWaypointDist, flInterval);
	}

	// Might be waiting for a door
	if (m_flMoveWaitFinished > gpGlobals->time)
	{
		Stop();
		return;
	}

	// UNDONE: this is a hack to quit moving farther than it has looked ahead.
	if (flCheckDist < m_flGroundSpeed * flInterval)
	{
		flInterval = flCheckDist / m_flGroundSpeed;
		// ALERT( at_console, "%.02f\n", flInterval );
	}
	MoveExecute(pTargetEnt, vecDir, flInterval);

	if (MovementIsComplete())
	{
		Stop();
		RouteClear();
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	*/



	
	//MODDD - new way!
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//if ( CheckLocalMove ( pev->origin, pev->origin + vecDir * flCheckDist, pTargetEnt, FALSE, &flDist ) != LOCALMOVE_VALID )
	if( localMovePass != 1 )
	{
		CBaseEntity *pBlocker;

		// Can't move, stop
		Stop();
		// Blocking entity is in global trace_ent

		EASY_CVAR_PRINTIF_PRE(pathfindPrintout, easyForcePrintLine("%s:ID%d Pathfinding: ROUTE A", getClassnameShort(), monsterID) );
		pBlocker = CBaseEntity::Instance( gpGlobals->trace_ent );

		//easyForcePrintLine("I AM THE BLOCKER AND I SUCK RAISINS %s", pBlocker->getClassname());

		if (pBlocker)
		{
			DispatchBlocked( edict(), pBlocker->edict() );
		}
		
		if ( pBlocker && m_moveWaitTime > 0 && pBlocker->IsMoving() && !pBlocker->IsPlayer() && (gpGlobals->time-m_flMoveWaitFinished) > 3.0 )
		{
			EASY_CVAR_PRINTIF_PRE(pathfindPrintout, easyForcePrintLine("%s:ID%d Pathfinding: ROUTE B1", getClassnameShort(), monsterID) );
			// Can we still move toward our target?
			if ( flDist < m_flGroundSpeed )
			{
				// No, Wait for a second
				EASY_CVAR_PRINTIF_PRE(pathfindPrintout, easyForcePrintLine("%s:ID%d Pathfinding: B11 FAIL", getClassnameShort(), monsterID) );
				m_flMoveWaitFinished = gpGlobals->time + m_moveWaitTime;
				return;
			}
			// Ok, still enough room to take a step
		}
		else 
		{
			EASY_CVAR_PRINTIF_PRE(pathfindPrintout, easyForcePrintLine("%s:ID%d Pathfinding: ROUTE B2", getClassnameShort(), monsterID) );
			// try to triangulate around whatever is in the way.
			if ( FTriangulate( pev->origin, m_Route[ m_iRouteIndex ].vecLocation, flDist, pTargetEnt, &vecApex ) )
			{
				EASY_CVAR_PRINTIF_PRE(pathfindPrintout, easyForcePrintLine("%s:ID%d Pathfinding: ROUTE C1", getClassnameShort(), monsterID) );
				InsertWaypoint( vecApex, bits_MF_TO_DETOUR );
				RouteSimplify( pTargetEnt );
			}
			else
			{
				EASY_CVAR_PRINTIF_PRE(pathfindPrintout, easyForcePrintLine("%s:ID%d Pathfinding: ROUTE C2", getClassnameShort(), monsterID) );
//				ALERT ( at_aiconsole, "Couldn't Triangulate\n" );
				Stop();
				// Only do this once until your route is cleared
				if ( m_moveWaitTime > 0 && !(m_afMemory & bits_MEMORY_MOVE_FAILED) )
				{
					EASY_CVAR_PRINTIF_PRE(pathfindPrintout, easyForcePrintLine("%s:ID%d Pathfinding: ROUTE D1", getClassnameShort(), monsterID) );
					FRefreshRoute();
					if ( FRouteClear() )
					{
						EASY_CVAR_PRINTIF_PRE(pathfindPrintout, easyForcePrintLine("%s:ID%d Pathfinding: ROUTE E1 FAIL", getClassnameShort(), monsterID) );
						TaskFail();
					}
					else
					{	
						
						EASY_CVAR_PRINTIF_PRE(pathfindPrintout, easyForcePrintLine("%s:ID%d Pathfinding: ROUTE E2", getClassnameShort(), monsterID) );
						// Don't get stuck
						
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//MODDD OF GREAT EMPHASIS
// Note that the  < #   part can be whatever other number, bigger to allow this to admit failure more easily in case of repeating a move.
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
						if ((gpGlobals->time - m_flMoveWaitFinished) < EASY_CVAR_GET_DEBUGONLY(pathfindFidgetFailTime)) {
							Remember(bits_MEMORY_MOVE_FAILED);
						}

						m_flMoveWaitFinished = gpGlobals->time + 0.1;
					}
				}
				else
				{
					//MODDD - this area is for failure, but if close enough call it success instead.

					// Also, there are two ways for failing to refresh a route to go.
					// Pass from being close enough to the goal node (goalDistTolerance being set),
					// OR pass from being within '(pev->size.y / 2.0f)' while pathfindEdgeCheck is on, strictNodeTolerance is off.
					// strictNodeTolerance does not stop goalDistTolerance from working, clearly being set means it is suposed to
					// be used.  The main point of strictNodeTolerance is to forbid letting RefreshRoute use BuildNearestRoute to
					// route to a nearby node when that isn't wanted in this case.

					// It is still up to the current schedule to do routine distance checks and end with 'MoveComplete' if the monster is close enough to its goal,  goalDistTolerance only stops for MoveComplete if pathfinding explicitly failed before reaching the point it was supposed to.
					// (add a little + 10 or so leeway if movegoaldist is set?)
					
					// CHANGE? goalDistTolerance checks moved to earlier in the pathfinding.  Why bother using
					// Triangulate and RefreshRoute calls when 'being close enough to the goal' wouldn't change
					// whether those are successful or not?
					// Wait.  Nevermind, keeping it here.  The assumption is, if the schedule is doing the routine range checks,
					// it would've stopped with MoveComplete then.  It's unlikely for goalDistMoveTolerance to even save this then
					// at this point, it's mostly for routes planned in advance to ANTICIPATE the early stopping distance we are allowed
					// from goalDistTolerance and still allow a route from failing less than 'goalDistTolerance' units from where we
					// want to go.

					
					/*
					// WAIT!  Not quite.
					if(goalDistTolerance > 0){
						// use strictNodeTolerance instead then.
						maxDist = goalDistTolerance;
					}else{
						maxDist = (pev->size.y / 2.0f) + 50.0f;
					}
					*/

					//////////////////////////////////////////////////////////////////////////////////////////////////
					if(goalDistTolerance > 0){
						//WayPoint_t* waypointGoalRef;
						//waypointGoalRef = GetGoalNode();
						
						// wait.  Couldn't we just assume m_vecMoveGoal?  oh well.
						// Nope!  Be concrete, get that goal node every time
						// Nope nope!  Go back, m_vecMoveGoal leads straight to the enemey.
						// NearestRoute goal nodes (which this could grab if on the way to one)
						// aren't supposed to be used with tolerance checks.

						//if(waypointGoalRef != NULL){
						//	float distanceee = (waypointGoalRef->vecLocation - pev->origin ).Length2D();
						//	BOOL theZCheck = ZCheck(pev->origin, waypointGoalRef->vecLocation);
							float distanceee = (m_vecMoveGoal - pev->origin ).Length2D();
							BOOL theZCheck = ZCheck(pev->origin, m_vecMoveGoal);

							if(theZCheck && distanceee <= goalDistTolerance){
								// close enough.
								MovementComplete();
								return;
							}else{
								// fall through to the pathfindEdgeCheck (maybe) for another chance or admit failure.
							}
						//}
					}//goalDistTolerance check
					//////////////////////////////////////////////////////////////////////////////////////////////////



					if((strictNodeTolerance == FALSE) && EASY_CVAR_GET_DEBUGONLY(pathfindEdgeCheck) == 1){
						// Before admitting failure, do a check. Are we close enough to the goal to let this count as success?
					
						//m_iRouteIndex
						WayPoint_t* waypointGoalRef;
						waypointGoalRef = GetGoalNode();

						////////////////////////////////////////////////
						if( ( waypointGoalRef != NULL) )
						{
							float distanceee = (waypointGoalRef->vecLocation - pev->origin ).Length2D();
							
							////////////////////////////////////////////////////////////
							//MODDD - important change!!!  This is what lets pathfinding declare finishing a little early,
							// and could be causeing the scripted-seuqences to begin playing from blindly thinking being close
							// enough to the target point is good enough.

							// How does pathfinding want to go outwards a ways on blocking the scientist when it gets close
							// to a vending machine?  that's just weird, look around for what sets nodes like that.
							
							// Anyway, try with pathfindEdgeCheck set to 0 too.
							// And how about the player still checks for being close enough to the target
							// point to see if it makes sense to snap to it, or admit failure (pause & try
							// again soon, maybe even pathfind randomly away?  dunno about that).

							// So look at AdvanceRoute, here and as-is.  As-is essentialy does this if approaching
							// a goal node:
							//if (distance < m_flGroundSpeed * 0.2)
							//	{
							//		MovementComplete();
							//	}
							//}

							// Anyway, handled now.  Any task that needs to go somewhere within pin-point tolerance like retail
							// will set strictNodeTolerance to TRUE to ignore pathfindEdgeCheck
							////////////////////////////////////////////////////////////



							//NOTICE - this allowed distance is very floaty. It is the expected distance to move in a frame times a number to go a bit further.
							//...no, our movement speed is unimportant. Use a constant distance instead, possibly factor in this monster's own size later.
							// ... < (m_flGroundSpeed * pev->framerate * EASY_CVAR_GET_DEBUGONLY(animationFramerateMulti) * flInterval * 5)

							// was 0.5f?
							float maxDist = (pev->size.y * 0.6f) + 50.0f;
							


							EASY_CVAR_PRINTIF_PRE(pathfindPrintout, easyForcePrintLine("PathfindEdgeCheck: DISTANCE TO GOAL: %.2f MAX ALLOWED: %.2f", distanceee, maxDist));
							if(distanceee <= maxDist){

								// Is there a straight line from me to the goal?
								TraceResult trTemp;
								UTIL_TraceLine ( pev->origin + Vector(0, 0, 5), waypointGoalRef->vecLocation + Vector(0, 0, 5), dont_ignore_monsters, dont_ignore_glass,  ENT(pev), &trTemp );

								if(drawPathConstant){
									DebugLine_ClearAll();
								}

								CBaseEntity* myGoalEnt = GetGoalEntity();

								if( trTemp.flFraction==1 || trTemp.pHit == NULL || (myGoalEnt != NULL && trTemp.pHit == myGoalEnt->edict()) ){
									// if nothing was hit or I happened to hit my goalent (enemy? targetent?) with this trace, pass.
									
									MovementComplete();
									return;
								}else{
									EASY_CVAR_PRINTIF_PRE(pathfindPrintout, easyForcePrintLine("PathfindEdgeCheck: TRACE FAILED?! classname:%s : fract:%.2f", STRING(trTemp.pHit->v.classname), trTemp.flFraction));
									
									if(drawPathConstant){
										DebugLine_Setup(0, pev->origin+Vector(0, 0, 5), waypointGoalRef->vecLocation + Vector(0, 0, 5), trTemp.flFraction);
									}
								}

							}
						}
						////////////////////////////////////////////////
					}


					EASY_CVAR_PRINTIF_PRE(pathfindPrintout, easyForcePrintLine("%s:ID%d Pathfinding: ROUTE D2", getClassnameShort(), monsterID) );
					TaskFail();
					ALERT( at_aiconsole, "%s Failed to move (%d)!\n", STRING(pev->classname), HasMemory( bits_MEMORY_MOVE_FAILED ) );
					//ALERT( at_aiconsole, "%f, %f, %f\n", pev->origin.z, (pev->origin + (vecDir * flCheckDist)).z, m_Route[m_iRouteIndex].vecLocation.z );

				}
				return;
			}
		}
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	

	BOOL skipMoveExecute = FALSE;
	
	EASY_CVAR_PRINTIF_PRE(pathfindPrintout, easyForcePrintLine("%s:ID%d Pathfinding: ROUTE X", getClassnameShort(), monsterID) );
	// close enough to the target, now advance to the next target. This is done before reaching
	// the target so that we get a nice natural turn while moving.

	///!!!BUGBUG- magic number
	if ( ShouldAdvanceRoute( flWaypointDist, flInterval ) )
	{
		
		////MODDD - no need. Notice the script below, "flCheckDist < distExpectedToCover". This will end up moving the monster
		////        the right amount of distance to cover the point anyways.
		//if(EASY_CVAR_GET(pathfindSnapToNode) == 1){
		//	//MODDD - if it returns true, snap to that position. Don't move this frame, this is good enough.
		//	pev->origin = m_Route[ m_iRouteIndex ].vecLocation;
		//	skipMoveExecute = TRUE;
		//}
		

		EASY_CVAR_PRINTIF_PRE(pathfindPrintout, easyForcePrintLine("%s:ID%d Pathfinding: ROUTE Z", getClassnameShort(), monsterID) );
		AdvanceRoute( flWaypointDist, flInterval );



		
		////MODDD - and, do a check since advancing the route.  Is the point after ok to traverse from?
		///////////////////////////////////////////////////////////////////////////////////////////////////
		//// Only if we're done moving though!  If already at the goal this won't work well.
		//// Can we prove if this area even makes a difference?   See the other ShouldAdvanceRoute checks
		////// in FRefreshRoute now, they seem to make the difference in stopping oscillation, maybe all of it.
		//if (!MovementIsComplete()) {
		//	int localMovePassTest = MovePRE(flInterval, flWaypointDist, flCheckDist, flDist, vecDir, pTargetEnt);
		//
		//	if (localMovePassTest != 1) {
		//		// nope!
		//		Stop();
		//		TaskFail();
		//		return;
		//	}
		//
		//	//int shouldItAgain = ShouldAdvanceRoute(flWaypointDist, flInterval);
		//	//int x = 45;
		//}
		/////////////////////////////////////////////////////////////////////////////////////////////////
		

	}else{

	}

	// Might be waiting for a door
	if ( m_flMoveWaitFinished > gpGlobals->time )
	{
		Stop();
		return;
	}

	const float distExpectedToCover = m_flGroundSpeed * pev->framerate * EASY_CVAR_GET_DEBUGONLY(animationFramerateMulti) * 1;

	//flCheckDist: 6    (just to waypoint w/ a cap of 200)
	//flCheckDist: 70
	//flCheckDist * 0.1: 7

	// UNDONE: this is a hack to quit moving farther than it has looked ahead.
	//if (flCheckDist < m_flGroundSpeed * flInterval)
	if(flCheckDist < distExpectedToCover * flInterval)
	{
		//flInterval = flCheckDist / m_flGroundSpeed;
		if(distExpectedToCover > 0){
			//no dividing by 0!
			flInterval = flCheckDist / distExpectedToCover;
		}else{
			flInterval = 0;
		}
		// ALERT( at_console, "%.02f\n", flInterval );
	}
	
	//MODDD - if we've already been snapped to the waypoint (allowed, was close enough to, and did), no need to move this frame. The logic won't move us towards the next waypoint yet,
	//        no need to go further beyond.
	//MODDD - see above, the hack on flInterval is enough to make the monster move only far enough to reach the node exactly this frame without passing it.
	//if(!skipMoveExecute){

	BOOL facingNextNode = TRUE;
	float facingTolerance = MoveYawDegreeTolerance();

	if(facingTolerance > 0){
		//One more thing. Are we facing where we want to go enough?
		//Vector vecDirToCurrentNode = (pev->origin - m_Route[ m_iRouteIndex ].vecLocation).Normalize();
		//...our ideal yaw already has to be towards this node. Just use that.
		
		//pev->ideal_yaw

		if(FacingIdeal(facingTolerance)){
			//good.
		}else{
			//not enough.
			facingNextNode = FALSE;
		}

	}else{
		//skip it, just pass (default).
	}


	BOOL movementCanAutoTurn = getMovementCanAutoTurn();

	//if movementCanAutoTurn is off, force a movement.
	if(!movementCanAutoTurn || facingNextNode){
		MoveExecute( pTargetEnt, vecDir, flInterval );
	}else{
		// try to face it?
		if(m_IdealActivity == ACT_RUN || m_IdealActivity == ACT_WALK || m_IdealActivity == ACT_HOVER || m_IdealActivity == ACT_FLY){
			Stop();
		}
		
		SetTurnActivity();
		
		if(m_IdealActivity != ACT_IDLE && m_IdealActivity != ACT_TURN_LEFT && m_IdealActivity != ACT_TURN_RIGHT){
			//force the ideal activity to IDLE for now.  No need to be frozen in any other animation and turn.
			//...is this a good idea or does it make things fidget for a frame?
			m_IdealActivity = ACT_IDLE;
		}
		

	}//END OF else OF facingNextNode

	//}

	if ( MovementIsComplete() )
	{
		EASY_CVAR_PRINTIF_PRE(pathfindPrintout, easyForcePrintLine("%s:ID%d Pathfinding: MOVEMENT IS COMPLETE??", getClassnameShort(), monsterID) );
		Stop();
		RouteClear();
	}

	// END OF THE NEW WAY.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	

}//END OF Move



//MODDD - now accepts the time eslapsed during this frame (flInterval) to see how far we could potentially move this frame. This is a better measure as to whether we are close enough
//        to the node than whether we are just less than 8 units of distance away. If this monster were too fast for whatever reason, this monster would go back / forth forever
//        without ever satisfying the " < 8" distance requirement. Base it off of the potential distance traveled in this frame and it can't fail.
BOOL CBaseMonster::ShouldAdvanceRoute( float flWaypointDist, float flInterval )
{
	const float rawMoveSpeedPerSec = (m_flGroundSpeed * pev->framerate * EASY_CVAR_GET_DEBUGONLY(animationFramerateMulti) * flInterval );
	const float moveDistTest = rawMoveSpeedPerSec * EASY_CVAR_GET_DEBUGONLY(pathfindNodeToleranceMulti);  //defaults to 1 for no effect.
	const float moveDistTol = max(moveDistTest, 8);  //must be at least 8.

	EASY_CVAR_PRINTIF_PRE(pathfindPrintout, easyForcePrintLine("%s:%d SHOULD I ADVANCE KIND SIR? d:%.2f rm:%.2f tol:%.2f pass:%d", FClassname(this), monsterID, (flWaypointDist), rawMoveSpeedPerSec, moveDistTol, (flWaypointDist <= moveDistTol) ) );
	//if ( flWaypointDist <= MONSTER_CUT_CORNER_DIST )
	

	if(flWaypointDist <= moveDistTol )
	{
		// ALERT( at_console, "cut %f\n", flWaypointDist );
		return TRUE;
	}
	return FALSE;
}






//MODDD - NEW.  For pathfinding to test whether a route will fail in the first step it is taken.
// Avoids changing to a move anim for a frame only to go 'oh wait I can't do this'.  Why not use the 
// same logic this same frame to get started and see if that gets interrupted.  If so, it's not the
// route you thought it was.
BOOL CBaseMonster::CheckPreMove(void){
	// to reach this point, we assume the route picked so far has been safe.
	// Failing the pre-move will change this.
	BOOL returnCode = TRUE;

	// careful about real changes to angles, etc. during this time, would look silly if we never move an inch
	g_pathfind_preMoveOnly = TRUE;

	int maxTimes = 2;
	while (maxTimes > 0) {
		maxTimes--;

		if (MovementIsComplete()) {
			// looks like we're already where we want to be?
			break;
		}


		// unused, but expected by the method anyway
		float flWaypointDist;
		float flCheckDist;
		float flDist;
		Vector vecDir;
		// NOTE - pTargetEnt will be filled by MovePRE, don't set it yet.
		CBaseEntity* pTargetEnt;

		// Success? try a pre-move on that new route then.
		// And interval.   uhhh.   assume 0.1 here?
		// Ahah!  We have m_flInterval now, even set before AI calls.
		float flInterval = m_flInterval;

		////////////////////////////////////////////////////////////////////////////////////
		//MODDD - TODO.  Should this all be surrounded ba "!IsMovementComplete" check?
		// That way an odd case of a route saying 'you're already there' is handled early, if that can happen.
		// Maybe if IsMovementComplete passes here, instead return a special code that
		// calls for the outside context to restore its vars and percolate up to the calling
		// script in schedule.cpp (probably) to say TaskComplete.
		// There is also right after the "AdvanceRoute" call further down.  I don't know.
		// ...  maybe later.
		////////////////////////////////////////////////////////////////////////////////////


		int localMovePass = MovePRE(flInterval, flWaypointDist, flCheckDist, flDist, vecDir, pTargetEnt);

		// A value of -1 is also possible to signify failure.  a "thing == 0" or FALSE check isn't good enough.
		if (localMovePass != 1) {
			// oh dear.  Revert and stop.  (whatever outside context that saved something should handle that)
			//m_movementActivity = oldMoveAct;
			//m_moveWaitTime = m_moveWaitTime;
			returnCode = FALSE;
			break;
		}

		// Now, see if we're near the end of the route.  If so, redo this check.
		// This can end very odd small routes that say "You can go!", only for 1 frame to pass,
		// advance that node in place, and then go "Oh nevermind."  ugly oscillation between stand and move activities.
		BOOL shouldItAgain = ShouldAdvanceRoute(flWaypointDist, flInterval);

		if (shouldItAgain) {
			// Node really close?  Advance the route and do another pre-move check!
			AdvanceRoute(flWaypointDist, flInterval);
				
			// MovementIsComplete() could go here, or at the beginning of this while loop.
			// In case it's already 'done' before even one check.
		}
		else {
			// Route left to go?  get out the loop then
			break;
		}
	}//END OF while loop

	g_pathfind_preMoveOnly = FALSE;

	return returnCode;
}//CheckPreMove





void CBaseMonster::MoveExecute( CBaseEntity *pTargetEnt, const Vector &vecDir, float flInterval )
{
#if defined(USE_MOVEMENT_BOUND_FIX_MOVEEXC) || defined(USE_MOVEMENT_BOUND_FIX_MOVEEXC_ALT)
	Vector oldMins;
	Vector oldMaxs;
	float flYaw;
		
#endif
//	float flYaw = UTIL_VecToYaw ( m_Route[ m_iRouteIndex ].vecLocation - pev->origin );// build a yaw that points to the goal.
//	WALK_MOVE( ENT(pev), flYaw, m_flGroundSpeed * flInterval, WALKMOVE_NORMAL );
	if ( m_IdealActivity != m_movementActivity )
		m_IdealActivity = m_movementActivity;

	

	// running in place when this is uncommented, can be useful sometimes to debug activity changes conveniently maybe.
	//return;


#ifdef USE_MOVEMENT_BOUND_FIX_MOVEEXC
	if(needsMovementBoundFix()){
		oldMins = pev->mins;
		oldMaxs = pev->maxs;
		UTIL_SetSize( pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX );
	}
#endif


#ifdef USE_MOVEMENT_BOUND_FIX_MOVEEXC_ALT
	flYaw = UTIL_VecToYaw ( m_Route[ m_iRouteIndex ].vecLocation - pev->origin );
#endif

	float flTotal = m_flGroundSpeed * pev->framerate * EASY_CVAR_GET_DEBUGONLY(animationFramerateMulti) * flInterval;
	float flStep;
	while (flTotal > 0.001)
	{

		/*
		// Normal way!
		///////////////////////////////////////////////////////////////////////////
		// don't walk more than 16 units or stairs stop working
		flStep = min( 16.0, flTotal );

		//Normal way!
		UTIL_MoveToOrigin ( ENT(pev), m_Route[ m_iRouteIndex ].vecLocation, flStep, MOVE_NORMAL );
		flTotal -= flStep;
		continue;
		///////////////////////////////////////////////////////////////////////////
		*/




#ifndef USE_MOVEMENT_BOUND_FIX_MOVEEXC_ALT
		
		
		// don't walk more than 16 units or stairs stop working
		flStep = min( 16.0, flTotal );

		//DebugLine_SetupPoint(m_Route[m_iRouteIndex].vecLocation, 255, 255, 255);
		
		//MODDD - using MOVE_STRAFE instead of MOVE_NORMAL.
		// Already moving in the direction I want to, and this jitters around less (if at all) when
		// moving stuck against a ramp, better to detect stagnation that way.
		// Wait... nevermind.  MOVE_NORMAL can still try to slide against whatever's in the way,
		// that might be better.


		Vector intendedLocation = pev->origin + vecDir * flStep;

		Vector oldOrigin = pev->origin;
		// the stepsize makes the difference in overcoming some ramp edges??
		UTIL_MoveToOrigin ( ENT(pev), m_Route[ m_iRouteIndex ].vecLocation, flStep, MOVE_NORMAL );


		Vector originNow = pev->origin;
		
		// Other idea:  how about instead of how much the position changed in the origin,
		// compare where the origin is now to where we expected it to be.  That's a better sign
		// of failure.  Getting stuck can move greatly sideways now.
		//Vector pointDeltaTE = (originNow - oldOrigin);
		Vector pointDeltaTE = (originNow - intendedLocation);

		//float moveDisto = DistanceFromDelta(pointDeltaTE);
		float moveDisto2D = Distance2DFromDelta(pointDeltaTE);


		//MODDD - could always back up by however much the '16' went overboard on the current step
		// but I doubt this will ever happen very often, and that could land back on the issue'd
		// area anyway.



		// Didn't move close enough to where I wanted to?
		if(moveDisto2D > flStep * 0.8){


			if(recentMoveExecuteFailureCooldown == -1){
				// first move since stopping? start the delay then.
				recentMoveExecuteFailureCooldown = gpGlobals->time + 1;
			}
			if(gpGlobals->time < recentMoveExecuteFailureCooldown){
				// Not yet, freeman!  Fail for longer.
				flTotal -= flStep;
				continue;
			}



		//if(moveDisto2D < flStep * 0.5){
			// Couldn't cover much of the step?  Try again with the full step size to try and get past this
			
			
			//if(flStep == 16){
			//	// Already a step size of 16??  What do I do here?
			//	easyPrintLine("MoveExecute ERROR: %s:%d Could not overcome obstacle at max step size!", getClassname(), monsterID);
			//	//TaskFail();
			//	//return;
			//}else
			{

				// not at the max step size?  ok, try at a higher step then (over 16 necessary sometimes?)
				// Careful about trying this the moment the monster gets stuck though, how about waiting
				// for X seconds to pass.  X seconds of solid failed movement will allow a large step turn.
				
				//easyPrintLine("MoveExecute: %s:%d: movement stagnation detected, attempting unstuck step", getClassname(), monsterID);

				oldOrigin = pev->origin;
				UTIL_MoveToOrigin ( ENT(pev), m_Route[ m_iRouteIndex ].vecLocation, UNSTUCK_STEP_SIZE, MOVE_NORMAL );
			
				originNow = pev->origin;
				//pointDeltaTE = (originNow - oldOrigin);
				pointDeltaTE = (originNow - intendedLocation);
				//float moveDisto = DistanceFromDelta(pointDeltaTE);
				moveDisto2D = Distance2DFromDelta(pointDeltaTE);
			
				//if(moveDisto2D < UNSTUCK_STEP_SIZE * 0.5){
				if(moveDisto2D > UNSTUCK_STEP_SIZE * 0.8){
					// Still stagnating?  FAIL
					easyPrintLine("MoveExecute ERROR: %s:%d Could not overcome obstacle at re-done step size!", getClassname(), monsterID);
					TaskFail();
					return;
				}
			}
		}// movement stagnation check
		else{
			// success?  Bump this up.
			recentMoveExecuteFailureCooldown = gpGlobals->time + 1.0;
		}





		/*
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// don't walk more than 16 units or stairs stop working
		flStep = min( 16.0, flTotal );


		float flWaypointDist = ( m_Route[ m_iRouteIndex ].vecLocation - pev->origin ).Length2D();
		float flCheckDist;
		BOOL checkTheZ;
		float flDist;

		// if the waypoint is closer than CheckDist, CheckDist is the dist to waypoint
		if (flWaypointDist < DIST_TO_CHECK)
		{
			flCheckDist = flWaypointDist;

			checkTheZ = TRUE;

		}
		else
		{
			flCheckDist = DIST_TO_CHECK;
			// incomplete dist?  Don't check the Z, 
			// Wait!
			if(
				m_Route[m_iRouteIndex].iType & (bits_MF_TO_ENEMY | bits_MF_TO_TARGETENT) &&
				m_Route[m_iRouteIndex].iType & (bits_MF_IS_GOAL)
				){
				checkTheZ = TRUE;
			}else{
				checkTheZ = FALSE;
			}
		}
		Vector vecCheckStart = pev->origin;
		Vector vecCheckEnd = pev->origin + vecDir * flCheckDist;
		//localMovePass = (CheckLocalMove(vecCheckStart, vecCheckEnd, pTargetEnt, checkTheZ, &flDist) == LOCALMOVE_VALID);


		UTIL_SetOrigin( pev, pev->origin );
		if(!this->isMovetypeFlying())
		{
			DROP_TO_FLOOR( ENT( pev ) );//make sure monster is on the floor!
		}
		Vector vecStartFiltered = pev->origin;
		Vector vecEndFiltered = vecCheckEnd;

		Vector pointDelta = vecEndFiltered - vecStartFiltered;
		float flYaw = UTIL_VecToYaw ( pointDelta);

		Vector oldOrigin = pev->origin;
		// the stepsize makes the difference in overcoming some ramp edges??
		BOOL walkMoveResult = WALK_MOVE(ENT(pev), flYaw, flStep, WALKMOVE_CHECKONLY);

		Vector originNow = pev->origin;
		Vector pointDeltaTE = (originNow - oldOrigin);
		float moveDisto = DistanceFromDelta(pointDeltaTE);
		float moveDisto2D = Distance2DFromDelta(pointDeltaTE);

		easyForcePrintLine("yargh %.2f %.2f", moveDisto, moveDisto2D);

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		*/








		// CHECK:  If movement is stagnating, maybe we're caught in some oddness near where the ramp
		// begins or ends.  Raise my Z a little and go again.


		// Why not use WALK_MOVE ?
		// oh, looks like the same, same flaws.  Nevermind.
		/*
		//////////////////////////////////////////////////////////////////////////////////
		Vector pointDelta = m_Route[m_iRouteIndex].vecLocation - pev->origin;
		float flYaw = UTIL_VecToYaw ( pointDelta);
		WALK_MOVE(ENT(pev), flYaw, flStep, WALKMOVE_NORMAL);
		//////////////////////////////////////////////////////////////////////////////////
		*/

#else

		if( !(EASY_CVAR_GET_DEBUGONLY(pathfindLargeBoundFix) == 1 && needsMovementBoundFix() ) ){
			//Normal way!
			UTIL_MoveToOrigin ( ENT(pev), m_Route[ m_iRouteIndex ].vecLocation, flStep, MOVE_NORMAL );

			//pev->origin = pev->origin + (m_Route[m_iRouteIndex].vecLocation - pev->origin).Normalize() * flStep;
		}else{
		//ALTERNATE.
		/////////////////////////////////////////////////////////////////////////////////////
		//MODDD - MAJOR. is that okay?
		//pTarget->edict() == gpGlobals->trace_ent
		if ( !WALK_MOVE( ENT(pev), flYaw, flStep, WALKMOVE_NORMAL ) ){

			
//			if ( pflDist != NULL )
//			{
//				*pflDist = flStep;
//			}
//			if ( pTarget && pTarget->edict() == gpGlobals->trace_ent )
//			{
//				// if this step hits target ent, the move is legal.
//				iReturn = LOCALMOVE_VALID;
//				break;
//			}
//			else
//			{
//				// If we're going toward an entity, and we're almost getting there, it's OK.
////				if ( pTarget && fabs( flDist - iStep ) < LOCAL_STEP_SIZE )
////					fReturn = TRUE;
////				else
//				iReturn = LOCALMOVE_INVALID;
//				break;
//			}
			

			//if(gpGlobals->trace_ent != NULL){
				CBaseEntity* whut = CBaseEntity::Instance(gpGlobals->trace_ent);

				if(whut != NULL){
					EASY_CVAR_PRINTIF_PRE(pathfindPrintout, easyForcePrintLine("%s:%d in the way of move execute?: %s:%d isW:%d", getClassname(), monsterID, whut->getClassname(), 
						(whut->GetMonsterPointer() != NULL)?whut->GetMonsterPointer()->monsterID:-1
						, whut->IsWorld()));
				}

				//These "AdvanceRoute" calls. Isn't that a tad drastic of an assumption?

				if(whut == NULL || whut->IsWorld()){
					//it is ok... maybe?  Check to see if the reason this was hit was because it was a ramp?
					//
					//resize me.
					oldMins = pev->mins;
					oldMaxs = pev->maxs;
					UTIL_SetSize( pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX );

					if(!WALK_MOVE( ENT(pev), flYaw, flStep, WALKMOVE_NORMAL )){
						//still no? that's the last shot you get.
						AdvanceRoute(flStep, flInterval);
						break;
					}

					UTIL_SetSize( pev, oldMins, oldMaxs );

				}else{
					//not the world? entity? etc.? assume it is not okay.
					AdvanceRoute(flStep, flInterval);
					break;
				}
			//}


		}//END OF if(!WALK_MOVE( ... ) )

		}//END OF else OF pathfindLargeBoundFix check

		/////////////////////////////////////////////////////////////////////////////////////

#endif
		flTotal -= flStep;
	}
	// ALERT( at_console, "dist %f\n", m_flGroundSpeed * pev->framerate * flInterval );

	
#ifdef USE_MOVEMENT_BOUND_FIX_MOVEEXC
	if(needsMovementBoundFix()){
		//and revert.
		UTIL_SetSize( pev, oldMins, oldMaxs );
	}
#endif

}


//=========================================================
// MonsterInit - after a monster is spawned, it needs to 
// be dropped into the world, checked for mobility problems,
// and put on the proper path, if any. This function does
// all of those things after the monster spawns. Any
// initialization that should take place for all monsters
// goes here.
//=========================================================
void CBaseMonster::MonsterInit ( void )
{

	//MODDD - DEBUG - show me the sounds in your sequences.
	//Sequence_PrintSound_All(GET_MODEL_PTR(this->edict()));


	//MODDD - extra check.
	const char* classname_test = STRING(pev->classname);
	if (classname_test == NULL || strlen(classname_test)==0) {
		easyForcePrintLine("WARNING! Monster without a classname spawned?");
	}
	
	//MODDD - "&& EASY_CVAR_GET(forceAllowMonsterSpawning) != 1" CVar removed.
	// Redundant with "mp_allowmonsters".
	// Also, added an exception to this rule for certain monsters.
	// The new 'pickup walker' monsters should not be forbidden by this setting.


	//if(CVAR_GET_FLOAT("pregame_server_cvar") == 0){
	if (!g_pGameRules->FAllowMonsters() && !this->bypassAllowMonstersSpawnCheck() ){

		easyForcePrintLine("WARNING! Request to spawn \"%s\" denied, \'mp_allowmonsters\' is off.", this->getClassname());

		//MODDD - "FL_KILLME" flag-add replaced with UTIL_Remove call, does a bit more.
		// Calls "onDelete" that way too.
		//pev->flags |= FL_KILLME;		// Post this because some monster code modifies class data after calling this function

		UTIL_Remove(this);

		//MODDD - commented out as-is.
//		REMOVE_ENTITY(ENT(pev));
		return;
	}

	const char* classnameCheat = STRING(pev->classname);

	if(monsterID == -1){
		//MODDD - unique ID just for me!
		monsterID = monsterIDLatest;
		monsterIDLatest++;
	}

	// Set fields common to all monsters
	pev->effects		= 0;
	pev->takedamage		= DAMAGE_AIM;
	pev->ideal_yaw		= pev->angles.y;

	//MODDD NOTE - this line assumes the monster's health was set during Spawn before MonsterInit was called. Just make sure of that.
	//    Also monsters that skip MonsterInit may want to do this too.
	pev->max_health		= pev->health;

	pev->deadflag		= DEAD_NO;
	m_IdealMonsterState	= MONSTERSTATE_IDLE;// Assume monster will be idle, until proven otherwise

	m_IdealActivity = ACT_IDLE;

	SetBits (pev->flags, FL_MONSTER);



	if (pev->spawnflags & SF_MONSTER_HITMONSTERCLIP) {
		pev->flags |= FL_MONSTERCLIP;
	}



	
	ClearSchedule();
	RouteClear();
	InitBoneControllers( ); // FIX: should be done in Spawn

	m_iHintNode			= NO_NODE;

	m_afMemory			= MEMORY_CLEAR;

	m_hEnemy			= NULL;

	//MODDD - these are now received from overridable methods instead so that child classes can change them.
	//        What else would the point of them being instance variables be?
	//        that and they should just be per class as methods like this instead of per individual monster (instance).
	//        When is say, one Kingpin going to have different DistTooFar or DistLook than any other Kingpin?
	m_flDistTooFar		= getDistTooFar();
	m_flDistLook		= getDistLook();

	//MODDD - flag for mirror recognition.
	// And letting clientside know whether this is organic or not for
	// crossbowbolt hits to refer to sometimes clientside.
	// Any check for "ISNPC" works for "ISMETALNPC" too.
	if(isOrganic()){
		pev->renderfx |= ISNPC;
	}else{
		pev->renderfx |= ISMETALNPC;
	}

	if(EASY_CVAR_GET_DEBUGONLY(monsterSpawnPrintout) == 1){
		easyPrintLine("I SPAWNED:::%s flags:%d", STRING(pev->classname), pev->spawnflags);
	}

	// set eye position
	SetEyePosition();

	SetThink( &CBaseMonster::MonsterInitThink );

	//!!!
	//SetThink ( &CBaseMonster::CallMonsterThink );

	pev->nextthink = gpGlobals->time + 0.1;
	SetUse ( &CBaseMonster::MonsterUse );


	//MODDD - is this a good idea to do at init?
	attemptResetTimedDamage(TRUE);


	//MODDD - potential to respawn? ok.
	// Could wait until MonsterInitThink happens though, may ease the 'spawned mid-air for a frame' issue.

	if (!(pev->spawnflags & SF_NORESPAWN)) {
		// NO.  this is baaaaaad.  The maps aren't expecting this to be depended on.
		//pev->spawnflags |= SF_MONSTER_FADECORPSE;
		respawn_origin = pev->origin;
		respawn_angles = pev->angles;
	}
	

}


//=========================================================
// MonsterInitThink - Calls StartMonster. Startmonster is 
// virtual, but this function cannot be 
//=========================================================
void CBaseMonster::MonsterInitThink ( void )
{
	StartMonster();
}



//=========================================================
// StartMonster - final bit of initization before a monster 
// is turned over to the AI. 
//=========================================================
void CBaseMonster::StartMonster ( void )
{

	//MODDD - SCRIPT MINOR - why on earth do these default to SCRIPT_PLAY (value for 0)?
#if HACKY_SCRIPT_TEST == 1
	m_scriptState = SCRIPT_WAIT;
#endif

	// update capabilities
	if ( LookupActivityFiltered ( ACT_RANGE_ATTACK1 ) != ACTIVITY_NOT_AVAILABLE )
	{
		m_afCapability |= bits_CAP_RANGE_ATTACK1;
	}
	if ( LookupActivityFiltered ( ACT_RANGE_ATTACK2 ) != ACTIVITY_NOT_AVAILABLE )
	{
		m_afCapability |= bits_CAP_RANGE_ATTACK2;
	}
	if ( LookupActivityFiltered ( ACT_MELEE_ATTACK1 ) != ACTIVITY_NOT_AVAILABLE )
	{
		m_afCapability |= bits_CAP_MELEE_ATTACK1;
	}
	if ( LookupActivityFiltered ( ACT_MELEE_ATTACK2 ) != ACTIVITY_NOT_AVAILABLE )
	{
		m_afCapability |= bits_CAP_MELEE_ATTACK2;
	}

	// Raise monster off the floor one unit, then drop to floor
	


	//MODDD NOTE - Oddly enough? missing "SF_MONSTER_FALL_TO_GROUND" causes the monster to fall to the ground.
	if ( !isMovetypeFlying() && !FBitSet( pev->spawnflags, SF_MONSTER_FALL_TO_GROUND ) )
	{
		if(EASY_CVAR_GET_DEBUGONLY(crazyMonsterPrintouts))easyForcePrintLine("YOU amazing piece of work");

		pev->origin.z += 1;
		DROP_TO_FLOOR ( ENT(pev) );
		// Try to move the monster to make sure it's not stuck in a brush.
		if ( !skipSpawnStuckCheck() && !WALK_MOVE ( ENT(pev), 0, 0, WALKMOVE_NORMAL ) )
		{
			ALERT(at_error, "Monster %s stuck in wall--level design error", STRING(pev->classname));
			pev->effects = EF_BRIGHTFIELD;
		}
	}
	else 
	{
		pev->flags &= ~FL_ONGROUND;
	}
	
	if ( !FStringNull(pev->target) )// this monster has a target
	{
		// Find the monster's initial target entity, stash it
		m_pGoalEnt = CBaseEntity::Instance( FIND_ENTITY_BY_TARGETNAME ( NULL, STRING( pev->target ) ) );

		if ( !m_pGoalEnt )
		{
			ALERT(at_error, "ReadyMonster()--%s couldn't find target %s", STRING(pev->classname), STRING(pev->target));
		}
		else
		{
			// Monster will start turning towards his destination
			MakeIdealYaw ( m_pGoalEnt->pev->origin );

			// JAY: How important is this error message?  Big Momma doesn't obey this rule, so I took it out.
#if 0
			// At this point, we expect only a path_corner as initial goal
			if (!FClassnameIs( m_pGoalEnt->pev, "path_corner"))
			{
				ALERT(at_warning, "ReadyMonster--monster's initial goal '%s' is not a path_corner", STRING(pev->target));
			}
#endif

			// set the monster up to walk a path corner path. 
			// !!!BUGBUG - this is a minor bit of a hack.
			// JAYJAY
			m_movementGoal = MOVEGOAL_PATHCORNER;
			

			//MODDD - is this appropriate for swimmers too?
			// Also, setting this early likely won't have a point.  Elsewhere handles this as these get switch back to stationary
			// acts like ACT_IDLE anyway.
			/*
			if(isMovetypeFlying())
				m_movementActivity = ACT_FLY;
			else
				m_movementActivity = ACT_WALK;
			*/


			if ( !FRefreshRoute() )
			{
				ALERT ( at_aiconsole, "Can't Create Route!\n" );
			}
			SetState( MONSTERSTATE_IDLE );
			ChangeSchedule( GetScheduleOfType( SCHED_IDLE_WALK ) );
		}
	}
	

	//SetState ( m_IdealMonsterState );
	//SetActivity ( m_IdealActivity );

	// Delay drop to floor to make sure each door in the level has had its chance to spawn
	// Spread think times so that they don't all happen at the same time (Carmack)
	SetThink ( &CBaseMonster::CallMonsterThink );
	pev->nextthink += RANDOM_FLOAT(0.1, 0.4); // spread think times.
	
	if ( !FStringNull(pev->targetname) )// wait until triggered
	{
		SetState( MONSTERSTATE_IDLE );
		// UNDONE: Some scripted sequence monsters don't have an idle?
		SetActivity( ACT_IDLE );
		ChangeSchedule( GetScheduleOfType( SCHED_WAIT_TRIGGER ) );
	}
}


void CBaseMonster::TaskComplete(void) {

	if(monsterID == 3){
		int x = 45;
	}

	if (!HasConditions(bits_COND_TASK_FAILED)) {
		m_iTaskStatus = TASKSTATUS_COMPLETE;
	}
}

// Note that this alone can no longer complete the currently running task.
// Any movement-related tasks typically checked for "MovementIsComplete" and then
// call "TaskComplete" themselves.
void CBaseMonster::MovementComplete( void ) 
{ 
	if(EASY_CVAR_GET_DEBUGONLY(movementIsCompletePrintout)==1)easyForcePrintLine("MovementComplete: %s:%d WHO CALLED THIS", getClassname(), monsterID);

	switch( m_iTaskStatus )
	{
	case TASKSTATUS_NEW:
	
	
	// YOU ARE NOW ALL POINTLESS disregard YOU
	/*
	case TASKSTATUS_RUNNING:
		m_iTaskStatus = TASKSTATUS_RUNNING_TASK;
		break;

	//MODDD - this was not even possible, nothing is ever assigned TASKSTATUS_RUNNING_MOVEMENT.
	//case TASKSTATUS_RUNNING_MOVEMENT:
	//	TaskComplete();
	//	break;
	
	case TASKSTATUS_RUNNING_TASK:
		ALERT( at_error, "Movement completed twice!\n" );

		//MODDD - new. This now also counts as TaskComplete. The Stuka needs it this way to be stumpable.  
		//...yes that seems funny, make more specific if other things are messed up as a result.
		//TaskComplete();
		break;
	*/

	case TASKSTATUS_COMPLETE:		
		break;
	}
	m_movementGoal = MOVEGOAL_NONE;
}



void CBaseMonster::TaskFail(void)
{
	//easyForcePrintLine("I PHAIL %.2f sched:%s task:%d", gpGlobals->time, getScheduleName(), getTaskNumber());
	SetConditions(bits_COND_TASK_FAILED);

	
	//if(FClassnameIs(this->pev, "monster_scientist")){
	if(monsterID == 3){
		//break point!
		const char* imLazy = (m_pSchedule!=NULL)?m_pSchedule->pName:"NULL!";
		int x = 7;
	}

}

// Keep in mind, this also counts TASKSTATUS_NEW.
int CBaseMonster::TaskIsRunning( void )
{
	//MODDD - TASKSTATUS_RUNNING_MOVEMENT is never set, that check is useless.
	//if ( m_iTaskStatus != TASKSTATUS_COMPLETE && m_iTaskStatus != TASKSTATUS_RUNNING_MOVEMENT )
	if(m_iTaskStatus != TASKSTATUS_COMPLETE)
		return 1;

	return 0;
}





//MODDD - major note here. Monsters that are feared, R_FR, are belo R_NO. It is easy to mistake R_FR for "Friend", since it's negative (-1) towards R_AL for "Ally" (-2).
//        But as-is "BestVisibleEnemy" does not allowed a feared target to be marked designated as enemy, so any monster schedule depending on having a "Feared" enemy
//        will never see it.

//BestVisibleEnemy - HEAVILY depends on relationships. Can pick a monster hated more, even if it is further away.

//=========================================================
// IRelationship - returns an integer that describes the 
// relationship between two types of monster.
//=========================================================
int CBaseMonster::IRelationship ( CBaseEntity *pTarget )
{

	//NOTICE - Bioweapon's relationship with AMONST changed from R_DL to R_NO so that snarks don't attack alien monsters that otherwise are uninsterested in them.
	//Besides, is there all that big of a difference between ALIEN_MILITARY and ALIEN_MONSTER or even ALIEN_PREDATOR?
	
	//MODDD - there is a new feature: "isForceHated".
	//        If true, we must hate this monster.
	//        May consider a "getRelationship()" later for forcing to hate.




	//CHUMTOAD NEVER CALLS ME.  R_DEFAULT DOESNT WORK RETURNED FROM A CHILD CLASS, ITS SUPPOSED TO FIGURE THAT OUT ON ITS OWN
	//  OR REDIRECT TO EITHER forceRelationshipWith OR  call the table here.
	//Heck just call the parent CBaseMonster::Irelationship instead of returning R_DEFAULT, nothing else knows what to do with that.

	int forcedRelationshipTest = pTarget->forcedRelationshipWith(this);
	
	if(forcedRelationshipTest != R_DEFAULT){
		//The other monster (pTarget) is forcing this monster to have an attitude towards it other than what the table may suggest.
		return forcedRelationshipTest;
	}else{}  //return below as usual.


	/*
	int thisThing = Classify();
	int thatThing = pTarget->Classify();
	*/

	extern BOOL g_f_playerDeadTruce;

	int generalRelationship;

	if(!g_f_playerDeadTruce){
		generalRelationship = iEnemy[ Classify() ][ pTarget->Classify() ];
	}else{
		// alt table
		generalRelationship = iEnemyTruce[ Classify() ][ pTarget->Classify() ];
	}
	

	/*
	if(FClassnameIs(this->pev, "monster_scientist")){
		easyForcePrintLine("myid %d: MY RELATIONSHIP WITH THIS DUDE %s IS %d", monsterID, pTarget->getClassname(), generalRelationship);
	}
	*/

	return generalRelationship;
}

//=========================================================
// FindCover - tries to find a nearby node that will hide
// the caller from its enemy. 
//
// If supplied, search will return a node at least as far
// away as MinDist, but no farther than MaxDist. 
// if MaxDist isn't supplied, it defaults to a reasonable 
// value
//=========================================================
// UNDONE: Should this find the nearest node?

//float CGraph::PathLength( int iStart, int iDest, int iHull, int afCapMask )

BOOL CBaseMonster::FindCover ( Vector vecThreat, Vector vecViewOffset, float flMinDist, float flMaxDist )
{
	int i;
	int iMyHullIndex;
	int iMyNode;
	int iThreatNode;
	float flDist;
	Vector	vecLookersOffset;
	TraceResult tr;

	if ( !flMaxDist )
	{
		// user didn't supply a MaxDist, so work up a crazy one.
		flMaxDist = 784;
	}

	if ( flMinDist > 0.5 * flMaxDist)
	{
#if _DEBUG
		ALERT ( at_console, "FindCover MinDist (%.0f) too close to MaxDist (%.0f)\n", flMinDist, flMaxDist );
#endif
		flMinDist = 0.5 * flMaxDist;
	}

	if ( !WorldGraph.m_fGraphPresent || !WorldGraph.m_fGraphPointersSet )
	{
		ALERT ( at_aiconsole, "Graph not ready for findcover!\n" );
		return FALSE;
	}

	iMyNode = WorldGraph.FindNearestNode( pev->origin, this );
	iThreatNode = WorldGraph.FindNearestNode ( vecThreat, this );
	iMyHullIndex = WorldGraph.HullIndex( this );

	if ( iMyNode == NO_NODE )
	{
		ALERT ( at_aiconsole, "FindCover() - %s has no nearest node!\n", STRING(pev->classname));
		return FALSE;
	}
	if ( iThreatNode == NO_NODE )
	{
		// ALERT ( at_aiconsole, "FindCover() - Threat has no nearest node!\n" );
		iThreatNode = iMyNode;
		// return FALSE;
	}

	vecLookersOffset = vecThreat + vecViewOffset;// calculate location of enemy's eyes

	
	//MODDD - cached at startup now in case any other pathfinding calls shift this in the middle of this method,
	// might not be good.
	// WAIT.  Shouldn't this have been an as-is bug???   m_iLastCoverSearch was used as the start node each iteration,
	// but changed to 'LastCoverSearch = LastCoverSearch + 1' right after.  So next iteration bumps up i, which gets
	// 1 + (LastCoverSearch + 1).
	// Such as, say m_iLastCoverSearch starts at 20.  i starts at 0.  Ignore modulus stuff.
	// Start with nodeNumber = (0 + 20). 20.
	// Then we set m_iLastCoverSearch to, nodeNumber (20) + 1.  It is 21.
	// Next iteration: i = 1.
	// nodeNumber = 1 + 21.  22.
	// nodeNumber skipped ever being '21' !   LastCoverSearch was set to that but never used for nodeNumber.
	// But wait, it gets worse.  Next iteration: i = 2.
	// nodeNumber = 2 + 22. 24.  m_iLastCoverSearch = 25.
	// Next iteration: i = 3.
	// nodeNumber = 3 + 25. 28.  LastCoverSearch = 29.
	// i = 4. 
	// nodeNumber = 4 + 29. 33.
	// More and more nodes get skipped.  Skipping some 30 nodes at a time yet doing this once from 0 to the node-count would be
	// wild if it does indeed count every single node once (and from testing, it didn't seem to).

	// Cacheing iLastCoverSearch as startSearchNode to begin with fixes that.
	// Although m_iLastCoverSearch could just as well have been set as a 'return' is being made then, no sense
	// setting 50+ times when it couldn't be referred to by anything else until after this method finishes.


	// changes to 1 and -1 with each use
	int alternatingChoice;
	int startSearchNode;
	int maxTries;



	alternatingChoice = 1;
	/*
	// ANY WAY BUT NEWEST
	startSearchNode = WorldGraph.m_iLastCoverSearch;
	maxTries = WorldGraph.m_cNodes;
	*/
	// NEWEST
	startSearchNode = iMyNode;
	maxTries = WorldGraph.m_cNodes - 1;

	int myNodeTypeAllowed = getNodeTypeAllowed();

	// we'll do a rough sample to find nodes that are relatively nearby
	for ( i = 0 ; i < maxTries ; i++ )
	{

		
		// NEWEST WAY.  Start from the marked 'iMyNode' instead and alternate in adding/subtracting from startSearchNode
		// each time.  Also move i up half as fast (since it's +1, -1, +2, -2, etc.     +1, -2, +3, -4 skips nodes, BAD)
		// Also, start i at 2 from it's normal start (0) so that the divide by 2 still ends up at a different value.
		// Using the closest node (might be standing at already) isn't very interesting, after all.
		////////////////////////////////////////////////////////////////
		int nodeNumber = ((alternatingChoice * (int)((i+2)*0.5)) + startSearchNode) % WorldGraph.m_cNodes;
		if (nodeNumber < 0) nodeNumber += WorldGraph.m_cNodes;  // in negatives, get out of the negative range, loop back around
		CNode &node = WorldGraph.Node( nodeNumber );

		if(alternatingChoice == 1){
			alternatingChoice = -1;
		}else{
			alternatingChoice = 1;
		}
		


		//MODDD - NOPE.  Do at the end.
		//WorldGraph.m_iLastCoverSearch = nodeNumber + 1; // next monster that searches for cover node will start where we left off here.
		
		////////////////////////////////////////////////////////////////

		// NEW WAY
		////////////////////////////////////////////////////////////////
		/*
		int nodeNumber = (i + startSearchNode) % WorldGraph.m_cNodes;
		CNode &node = WorldGraph.Node( nodeNumber );
		//MODDD - NOPE.  Do at the end.
		//WorldGraph.m_iLastCoverSearch = nodeNumber + 1; // next monster that searches for cover node will start where we left off here.
		*/
		////////////////////////////////////////////////////////////////

		/*
		// OLD WAY
		////////////////////////////////////////////////////////////////
		int nodeNumber = (i + WorldGraph.m_iLastCoverSearch) % WorldGraph.m_cNodes;
		CNode &node = WorldGraph.Node( nodeNumber );
		WorldGraph.m_iLastCoverSearch = nodeNumber + 1; // next monster that searches for cover node will start where we left off here.
		////////////////////////////////////////////////////////////////
		*/



		if(EASY_CVAR_GET_DEBUGONLY(pathfindIgnoreIsolatedNodes) == 1 && node.m_cNumLinks < 1){
			// also a reason to drop the node
			continue;
		}
		//MODDD - Also, new early check.  If this node clashes with this monster's type of movement
		// (mid-air nodes even though I'm a walker), deny even trying them.  What good would that route be.
		if(!(node.m_afNodeInfo & myNodeTypeAllowed)){
			// no types in common?  Skip it
			continue;
		}

		// could use an optimization here!!
		flDist = ( pev->origin - node.m_vecOrigin ).Length();

		// DON'T do the trace check on a node that is farther away than a node that we've already found to 
		// provide cover! Also make sure the node is within the mins/maxs of the search.
		if ( flDist >= flMinDist && flDist < flMaxDist )
		{
			UTIL_TraceLine ( node.m_vecOrigin + vecViewOffset, vecLookersOffset, ignore_monsters, ignore_glass,  ENT(pev), &tr );
			//return TRUE; //WARNING - SUPER DUPER HACKY SACKS

			// if this node will block the threat's line of sight to me...
			if ( tr.flFraction != 1.0 )
			{
				//DebugLine_Setup(0, node.m_vecOrigin + vecViewOffset, vecLookersOffset, tr.flFraction);


				// ..and is also closer to me than the threat, or the same distance from myself and the threat the node is good.
				if ( ( iMyNode == iThreatNode ) || WorldGraph.PathLength( iMyNode, nodeNumber, iMyHullIndex, m_afCapability ) <= WorldGraph.PathLength( iThreatNode, nodeNumber, iMyHullIndex, m_afCapability ) )
				{

					if ( FValidateCover ( node.m_vecOrigin ) && MoveToLocation( ACT_RUN, 0, node.m_vecOrigin ) )
					{
						
						//The debug line thingy.
						/*
						MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
							WRITE_BYTE( TE_SHOWLINE);
							
							WRITE_COORD( node.m_vecOrigin.x );
							WRITE_COORD( node.m_vecOrigin.y );
							WRITE_COORD( node.m_vecOrigin.z );

							WRITE_COORD( vecLookersOffset.x );
							WRITE_COORD( vecLookersOffset.y );
							WRITE_COORD( vecLookersOffset.z );
						MESSAGE_END();
						*/

						//MODDD - set this now
						WorldGraph.m_iLastCoverSearch = nodeNumber + 1;
						return TRUE;
					}
				}
			}
		}
	}
	// any point in setting m_iLastCoverSearch if this failed?
	return FALSE;
}




//MODDD - clone of FindCover that, well, doesn't really care about the 'cover' aspect.  Just move away somewhat.
// Points further away and obscurred from my origin are more likely to get picked, however.  I do want to pick a more
// interesting point to explore than 5 inches away or that I can already see, after all.
// Also takes a preference of movement activity (ACT_WALK or ACT_RUN most likely), since this might be casual for otherwise
// fast-moving entities.

// TODO: could some randomness be part of each desirability increase, so that there's a floatiness to what node is
// ideal one time and ideal another?  Hard to get it to make a worthwhile difference without sometimes picking oddly
// close nodes maybe.   Eh, little can't hurt.   thisNodeDesirability += 0.4 * RANDOM_FLOAT(0.78, 1.0);

BOOL CBaseMonster::FindRandom ( Activity movementAct, Vector vecThreat, Vector vecViewOffset, float flMinDist, float flMaxDist )
{
	int i;
	int iMyHullIndex;
	int iMyNode;
	int iThreatNode;
	float flDist;
	Vector	vecLookersOffset;
	TraceResult tr;

	if ( !flMaxDist )
	{
		// user didn't supply a MaxDist, so work up a crazy one.
		flMaxDist = 784;
	}

	if ( flMinDist > 0.5 * flMaxDist)
	{
#if _DEBUG
		ALERT ( at_console, "FindRandom MinDist (%.0f) too close to MaxDist (%.0f)\n", flMinDist, flMaxDist );
#endif
		flMinDist = 0.5 * flMaxDist;
	}

	if ( !WorldGraph.m_fGraphPresent || !WorldGraph.m_fGraphPointersSet )
	{
		ALERT ( at_aiconsole, "Graph not ready for FindRandom!\n" );
		return FALSE;
	}

	iMyNode = WorldGraph.FindNearestNode( pev->origin, this );
	iThreatNode = WorldGraph.FindNearestNode ( vecThreat, this );
	iMyHullIndex = WorldGraph.HullIndex( this );

	if ( iMyNode == NO_NODE )
	{
		// aiconsole?? >:/
		//ALERT ( at_aiconsole, "FindRandom() - %s has no nearest node!\n", STRING(pev->classname));
		easyPrintLine("???   No nearest node what");
		return FALSE;
	}
	if ( iThreatNode == NO_NODE )
	{
		// ALERT ( at_aiconsole, "FindRandom() - Threat has no nearest node!\n" );
		iThreatNode = iMyNode;
		// return FALSE;
	}

	vecLookersOffset = vecThreat + vecViewOffset;// calculate location of enemy's eyes

	// Instead of picking the first point that is 'good', let the usual cover conditions add to desirability.
	// I could pick any random point nearby, but further is better (to an extent), and so is lacking a line of sight
	// to where I started.  More interesting to explre places I can't see from here, after all.
	// At the end, the node with the highest desirability is what I want to make a path to (call 'MoveToLocation';
	// that builds the route from here to there).
	float bestNodeDesirability = -1;
	int bestNodeNumber = -1;
	CNode* bestNode = NULL;
	






	// DEBUG!  FORCE THIS
	//bestNode = &WorldGraph.Node( 25 );
	//MoveToLocation(ACT_RUN, 0, bestNode->m_vecOrigin);
	//return TRUE;




	int alternatingChoice;
	int startSearchNode;
	int maxTries;

	alternatingChoice = 1;
	// ANY WAY BUT NEWEST
	//int startSearchNode = WorldGraph.m_iLastCoverSearch;
	//int maxTries = WorldGraph.m_cNodes;
	// NEWEST
	startSearchNode = iMyNode;
	maxTries = WorldGraph.m_cNodes - 1;

	int myNodeTypeAllowed = getNodeTypeAllowed();

	// we'll do a rough sample to find nodes that are relatively nearby
	for ( i = 0 ; i < maxTries ; i++ )
	{
		float thisNodeDesirability = 0;

		// NEWEST WAY
		int nodeNumber = ((alternatingChoice * (int)((i+2)*0.5)) + startSearchNode) % WorldGraph.m_cNodes;
		if (nodeNumber < 0) nodeNumber += WorldGraph.m_cNodes;  // in negatives, get out of the negative range, loop back around
		CNode &node = WorldGraph.Node( nodeNumber );

		if(alternatingChoice == 1){
			alternatingChoice = -1;
		}else{
			alternatingChoice = 1;
		}

		if(nodeNumber == 53){
			int i = 43;
		}


		// NEW WAY
		/*
		//MODDD - TODO?   Does relying on the same 'recent' node from Cover even make sense here?
		// No harm though, probably.  But don't commit this for other monsters that want to look for real cover.
		int nodeNumber = (i + startSearchNode) % WorldGraph.m_cNodes;

		CNode &node = WorldGraph.Node( nodeNumber );
		// !!! COMMENTED OUT, related to above node
		//WorldGraph.m_iLastCoverSearch = nodeNumber + 1; // next monster that searches for cover node will start where we left off here.
		*/





		// TEST!!!
		//if(nodeNumber == 73 || nodeNumber == 77 || nodeNumber == 79 || nodeNumber == 75){
		//	easyForcePrintLine("IM present %d", nodeNumber);
		//}

		// Beware!  This node may be in the map.  Don't know why this happens sometimes,
		// see node #63 ('teleporttonode 63' in console) in a1a1b.bsp.)
		// Wait!  Now making nodes that start in bad locations have 0 nodeinfo (land, air,
		// or water), so that check is all that's needed.
		// No a type of movement able to reach it.
		/*
		Vector testVecto = node.m_vecOriginPeek;
		// a 'node.m_cNumLinks' under 1 would also be suspicious (no links out), but eh.
		// We are just trying to go anywhere anyway.
		if(UTIL_PointContents(testVecto) == CONTENTS_SOLID){
			// what.
			continue;
		}
		*/

		
		if(EASY_CVAR_GET_DEBUGONLY(pathfindIgnoreIsolatedNodes) == 1 && node.m_cNumLinks < 1){
			// also a reason to drop the node
			continue;
		}
		if(!(node.m_afNodeInfo & myNodeTypeAllowed)){
			// no types in common?  Skip it
			continue;
		}



		// could use an optimization here!!
		flDist = ( pev->origin - node.m_vecOrigin ).Length();

		// DON'T do the trace check on a node that is farther away than a node that we've already found to 
		// provide cover! Also make sure the node is within the mins/maxs of the search.
		// ALSO, beware of calling 'MoveToLocation' several times (does not actually do the move, tries to build a 
		// route to a given point/target and leaves with that route in place....... great naming guys.)
		// ANYWAY. Each time likely calls BuildNearestRoute, which leads to "WorldGraph.m_iLastCoverSearch" getting shifted.
		// So we're likely to check for some nodes twice and others none while counting through all nodes.  BLEGH.
		// Changed to 'MoveToLocationCheap', which avoids BuildNearestRoute calls if plain BuildRoute ones fail
		// (makes sense, on checking so many nodes 'BuildNearestRoute' many times will just get expensive and redundant).
		// And changed m_iLastCoverSearch to get saved to a temp-var and used out of paranoia (changes to LastCoverSearch
		// since would not affect what we're counting off of; no skipped or twice-tried nodes)
		if ( flDist >= flMinDist && flDist < flMaxDist )
		{
			UTIL_TraceLine ( node.m_vecOrigin + vecViewOffset, vecLookersOffset, ignore_monsters, ignore_glass,  ENT(pev), &tr );
			//return TRUE; //WARNING - SUPER DUPER HACKY SACKS

			// if this node will block the threat's line of sight to me...
			if ( tr.flFraction != 1.0 )
			{
				// count it
				thisNodeDesirability += 0.4;
			}

// ..and is also closer to me than the threat, or the same distance from myself and the threat the node is good.
			if ( ( iMyNode == iThreatNode ) || WorldGraph.PathLength( iMyNode, nodeNumber, iMyHullIndex, m_afCapability ) <= WorldGraph.PathLength( iThreatNode, nodeNumber, iMyHullIndex, m_afCapability ) )
			{
				// Let's call this a good thing
				thisNodeDesirability += 0.25;
			}
			if ( FValidateCover ( node.m_vecOrigin ) )
			{
				// Probably?
				thisNodeDesirability += 0.25;
			}

			// More distnace, up to a point, more desirability.  More interesting to go further in general.
			// NOTE - this is a cheap check of distance between me and the node being tested, not the length
			// of the route to get there.  Although a route that's longer is more interesting for random wandering than
			// just ending up at a further point, it might not be by much and that's wa more expensive to test
			// (instead of checking only the most 'desirable' nodes at a time for pathfinding, but only including their
			// route lengths would bias things compared to nodes that we don't attempt to route to)
			// NEVERMIND.  Seems the route's distance is only written if it failed a WALK_MOVE call, and only
			// up to the most recently failed WALK_MOVE call (moves in increments of step-size, 16 units).
			// Could fix but.    eh.    Kinda sick and tired of this 'change one insignificant thing, the world comes
			// crashing down a few months later from some obscure bug caused by that' shit.
			if(flDist <= 200){
				// eh.
				thisNodeDesirability += 0.0;
			}else if(flDist <= 600){
				// more the merrier.
				thisNodeDesirability += (flDist - 200) / (600 - 200) * (0.6 - 0.0) + 0.0;
			}else{
				// plenty fine.
				thisNodeDesirability += 0.6;
			}

			if(thisNodeDesirability > bestNodeDesirability){
				// Now wait!  Can we actually pathfind to this o wondrous node?
				BOOL canPath = MoveToLocationCheap(movementAct, 0, node.m_vecOrigin);
				if(canPath){
					// take it!
					bestNodeDesirability = thisNodeDesirability;
					bestNodeNumber = nodeNumber;
					bestNode = &WorldGraph.Node( nodeNumber );
				}
				RouteClear();  // necessary?  cheap though
			}

		}// flDist minimum requirements
	}// for loop through nodes to test

	
	// breakpoints.
	bestNodeNumber;

	//if(bestNodeDesirability > 0.3){
	if(bestNode != NULL){
		// yippee.  Leave off with this path.
		MoveToLocation(ACT_RUN, 0, bestNode->m_vecOrigin);
		return TRUE;
	}else{
		// cancel any path just in case
		RouteClear();
	}

	return FALSE;
}

// Overridable.  Most monsters like any cover that passed the usual checks
// (no line of sight to location/threat to hide or get away from, etc.), but some need a little more.
// Squadmonsters don't want cover too close to other squadies, for instance.  (yes I said that)
BOOL CBaseMonster::FValidateCover(const Vector& vecCoverLocation){
	return TRUE;
}






//=========================================================
// BuildNearestRoute - tries to build a route as close to the target
// as possible, even if there isn't a path to the final point.
//
// If supplied, search will return a node at least as far
// away as MinDist from vecThreat, but no farther than MaxDist. 
// if MaxDist isn't supplied, it defaults to a reasonable 
// value
//=========================================================
//MODDD - now supports optional moveflags and target entity just like BuildRoute does to send to it if provided.
// ALSO, must be told whether this starts from a random node (retail, although likely unintentional in most cases) or uses its current
// node to start a search.  Nodes close in number are often close along a path as placed in the map, but there is no guarantee that some
// perfectly fine node nearby varries wildly in number.
// Although, it looks like the intention was to start searching from 1 after the most recently successfully picked cover node
// game-wide (WorldGraph.m_iLastCoverSearch), but this is unlikely to be as relevant for monsters completely unrelated to the one that picked
// that number (in distance or team).  And it still didn't work quite right, skipped all over the place instead of counting up through all
// the nodes as likely intended, so it ended up being very random in real use anyway (like how default randomNodeSearchStart of TRUE would work).
BOOL CBaseMonster::BuildNearestRoute ( Vector vecThreat, Vector vecViewOffset, float flMinDist, float flMaxDist, BOOL randomNodeSearchStart ){
	//default moveflag is bits_MF_TO_LOCATION.  That was always sent to BuildRoute in BuildNearestRoute as of retail.
	return BuildNearestRoute(vecThreat, vecViewOffset, flMinDist, flMaxDist, randomNodeSearchStart, bits_MF_TO_LOCATION, NULL);
}

BOOL CBaseMonster::BuildNearestRoute ( Vector vecThreat, Vector vecViewOffset, float flMinDist, float flMaxDist, BOOL randomNodeSearchStart, int iMoveFlag, CBaseEntity* pTarget )
{
	int i;
	int iMyHullIndex;
	int iMyNode;
	float flDist;
	Vector	vecLookersOffset;
	TraceResult tr;

	//HACKY HACKY
	//flMaxDist = 9999;

	if ( !flMaxDist )
	{
		// user didn't supply a MaxDist, so work up a crazy one.
		flMaxDist = 784;
	}

	if ( flMinDist > 0.5 * flMaxDist)
	{
#if _DEBUG
		ALERT ( at_console, "FindCover MinDist (%.0f) too close to MaxDist (%.0f)\n", flMinDist, flMaxDist );
#endif
		flMinDist = 0.5 * flMaxDist;
	}

	if ( !WorldGraph.m_fGraphPresent || !WorldGraph.m_fGraphPointersSet )
	{
		ALERT ( at_aiconsole, "Graph not ready for BuildNearestRoute!\n" );
		return FALSE;
	}

	iMyNode = WorldGraph.FindNearestNode( pev->origin, this );
	iMyHullIndex = WorldGraph.HullIndex( this );

	//MODDD - CHEAT CHEAT CHEAT!
	//iMyHullIndex = NODE_POINT_HULL;

	if ( iMyNode == NO_NODE )
	{
		if(EASY_CVAR_GET_DEBUGONLY(pathfindPrintout)==1)easyForcePrintLine("Path %s:%d Here ye here ye, I could not find a nearby node! I suck!", getClassname(), monsterID);
			
		
		ALERT ( at_aiconsole, "BuildNearestRoute() - %s has no nearest node!\n", STRING(pev->classname));
		return FALSE;
	}

	vecLookersOffset = vecThreat + vecViewOffset;// calculate location of enemy's eyes



	//DebugLine_ClearAll();


	int alternatingChoice;
	int startSearchNode;
	int maxTries;
	int iOffset;




	/*
	// TEST.  Force conditions.
	WorldGraph.m_cNodes = 108;
	iOffset = 1;
	alternatingChoice = -1;
	startSearchNode = 7; //(int)RANDOM_LONG(0, WorldGraph.m_cNodes-1);
	maxTries = WorldGraph.m_cNodes;

	// record how many times each node was picked as 'nodeNumber'.  Each should be picked only once, except the start node (never).
	int daNodes[256];
	memset(&daNodes[0], 0, sizeof(int) * 256);
	
	// TEST: what nodes does this logic go through?
	for ( i = 0 ; i < maxTries ; i++ )
	{
		// NEWEST WAY.  Start from the marked 'iMyNode' instead and alternate in adding/subtracting from startSearchNode
		// each time.  Also move i up half as fast (since it's +1, -1, +2, -2, etc.     +1, -2, +3, -4 skips nodes, BAD)
		// Also, start i at 2 from it's normal start (0) so that the divide by 2 still ends up at a different value.
		// Using the closest node (might be standing at already) isn't very interesting, after all.
		////////////////////////////////////////////////////////////////
		int nodeNumber = ((alternatingChoice * (int)((i+iOffset)*0.5)) + startSearchNode) % WorldGraph.m_cNodes;
		if (nodeNumber < 0) nodeNumber += WorldGraph.m_cNodes;  // in negatives, get out of the negative range, loop back around
		
		if(alternatingChoice == 1){
			alternatingChoice = -1;
		}else{
			alternatingChoice = 1;
		}

		daNodes[nodeNumber] = 1;
		int x = 45;
	}

	for(i = 0; i < WorldGraph.m_cNodes; i++){
		easyForcePrintLine("#%d : %d", i, daNodes[i]);
	}
	for (i = WorldGraph.m_cNodes; i < WorldGraph.m_cNodes + 16;  i++) {
		easyForcePrintLine("OUT OF BOUNDS CHECK: #%d : %d", i, daNodes[i]);
	}
	*/


	if(randomNodeSearchStart){
		// If this monster wants random behavior, include the node picked since it might not be
		// the start node (no reason to specifically include it).
		// Picking a different node to start searching from may influence what location gets picked in the end,
		// if several meet the criteria.
		// This also means using a starting offset of 1 instead (since that divided by 2 is 0.5, truncates to 0,
		// means no change to startSearchNode: exactly the start node, once). Next iteration, alternatingChoice
		// goes from the -1 here to 1, and 'iOffset' (1) + i (1) makes 2.  Divide by 2, 1.  Next time,
		// -1*(i=2)/2, makes -1, then 2, then -2, 3, -3, etc.  Alternating choices continue as normal.
		// Although go up to the max number of nodes instead of '-1'.
		iOffset = 1;
		alternatingChoice = -1;
		startSearchNode = (int)RANDOM_LONG(0, WorldGraph.m_cNodes-1);
		maxTries = WorldGraph.m_cNodes;

	}else{
		// start searching from my own start node.
		// iOffset of 2 so that 'i = 0' is treated as 2.  Divided by 2 that starts at '1', add another for 3, 
		// divided by 2 makes 1.5, truncates to 1 (for being subtracted instead of added to).  4 / 2 = 2,
		// then -2, +3, -3, etc.
		iOffset = 2;
		alternatingChoice = 1;
		/*
		// ANY WAY BUT NEWEST
		startSearchNode = WorldGraph.m_iLastCoverSearch;
		maxTries = WorldGraph.m_cNodes;
		*/
		// NEWEST
		startSearchNode = iMyNode;
		maxTries = WorldGraph.m_cNodes - 1;
	}


	CBaseEntity* myGoalEnt = GetGoalEntity();

	// If this has FL_MONSTERCLIP, offer using CheckLocalMove as an alternative to looking up what nodes have good routes to other nodes.
	// The predetermined static route table was made without MONSTERCLIP in mind, so it may have paths we can't take or lack paths we can take
	// that obviously should work fine.
	BOOL theHardWay;

	if(EASY_CVAR_GET_DEBUGONLY(pathfindMonsterclipFreshLogic) != 0){
		theHardWay = (this->pev->flags & FL_MONSTERCLIP);
	}else{
		// no check
		theHardWay = FALSE;
	}


	int myNodeTypeAllowed = getNodeTypeAllowed();

	// we'll do a rough sample to find nodes that are relatively nearby
	for ( i = 0 ; i < maxTries; i++ )
	{
		// NEWEST WAY.
		////////////////////////////////////////////////////////////////
		int nodeNumber = ((alternatingChoice * (int)((i+iOffset)*0.5)) + startSearchNode) % WorldGraph.m_cNodes;

		if(monsterID == 3 && nodeNumber == 11){
			int x = 45;
		}

		//MODDD
		// Don't try the node I'm already closest to as a destination if I'm already very close.
		if (nodeNumber == iMyNode){
			float distah = Distance(pev->origin, WorldGraph.Node(iMyNode).m_vecOrigin);
			if(distah <= goalDistTolerance){
				continue;
			}
		}

		if (nodeNumber < 0) nodeNumber += WorldGraph.m_cNodes;  // in negatives, get out of the negative range, loop back around
		CNode &node = WorldGraph.Node( nodeNumber );

		if(alternatingChoice == 1){
			alternatingChoice = -1;
		}else{
			alternatingChoice = 1;
		}
		
		// NEW WAY
		/*
		int nodeNumber = (i + startSearchNode) % WorldGraph.m_cNodes;
		CNode &node = WorldGraph.Node( nodeNumber );
		//MODDD - why not do this at the end and on success?
		//WorldGraph.m_iLastCoverSearch = nodeNumber + 1; // next monster that searches for cover node will start where we left off here.
		*/

		// OLD WAY
		/*
		int nodeNumber = (i + WorldGraph.m_iLastCoverSearch) % WorldGraph.m_cNodes;
		CNode &node = WorldGraph.Node( nodeNumber );
		WorldGraph.m_iLastCoverSearch = nodeNumber + 1; // next monster that searches for cover node will start where we left off here.
		*/


		if(EASY_CVAR_GET_DEBUGONLY(pathfindIgnoreIsolatedNodes) == 1 && node.m_cNumLinks < 1){
			// also a reason to drop the node
			continue;
		}
		if(!(node.m_afNodeInfo & myNodeTypeAllowed)){
			// no types in common?  Skip it
			continue;
		}


		//MODDD - idea.  Wouldn't doing the dist check before the CheckLocalMove's make more sense?
		// Seems cheaper than CheckLocalMove.   The retail way is still best with it after though, NextNodeInRoute
		// might be cheaper than a distance check.   Really?
		BOOL passingCondition;
		// can I get there?
		if(!theHardWay){
			// usual cheaper way
			//MODDD - Wait.  Why didn't this receive this monster's 'm_afCapability'?  It might be pointless but why not.
			// (last parameter used to be 0)
			// WAIT!  Bad idea to send out m_afCapability on its own.  It should be thought of as a 'cap index', with choices
			// 0 and 1 only.  Which choice to use comes from checking m_afCapability for the presence of certain flags.
			// Still beats me why even that much wasn't done here, wandering around to a nearest point may as well
			// use whatever capability.
			int iCap = WorldGraph.CapIndex(m_afCapability);
			passingCondition = WorldGraph.NextNodeInRoute(iMyNode, nodeNumber, iMyHullIndex, iCap) != iMyNode;
		}else{
			// WARNING!  Very bad idea, many FindShortestPath calls like this lags like mad.  Don't do that.
			// If a FL_MONSTERCLIP, just try CheckLocalMove from here to the dest, they're usually pretty simple checks.
			/*
			int iPath[ MAX_PATH_SIZE ];
			// using FL_MONSTERCLIP?  can't trust no predetermined route info
			
			g_routeTempMonster = this;
			// Assume the movegoal is set if it will be relevant?
			g_routeTempMonster_GoalEnt = GetGoalEntity();
			int daRouteSize = WorldGraph.FindShortestPath(iPath, iMyNode, nodeNumber, iMyHullIndex, m_afCapability);
			g_routeTempMonster = NULL;
			if(daRouteSize > 0){
				passingCondition = TRUE;
			}else{
				passingCondition = FALSE;
			}
			*/

			if(EASY_CVAR_GET_DEBUGONLY(pathfindMonsterclipFreshLogic) == 1){
				TraceResult tr;
				UTIL_TraceLine(WorldGraph.Node(iMyNode).m_vecOrigin, WorldGraph.Node(nodeNumber).m_vecOrigin, dont_ignore_monsters, ENT(this->pev), &tr);
				if(tr.flFraction >= 1.0 || (tr.pHit != NULL && myGoalEnt != NULL && tr.pHit == myGoalEnt->edict())  ){
					// uninterrupted line, or the thing hit was the goal ent?  Allow
					passingCondition = TRUE;
				}else{
					// nope
					passingCondition = FALSE;
				}
			}else if(EASY_CVAR_GET_DEBUGONLY(pathfindMonsterclipFreshLogic) == 2){
				passingCondition = (CheckLocalMove(WorldGraph.Node(iMyNode).m_vecOrigin, WorldGraph.Node(nodeNumber).m_vecOrigin, m_hEnemy, TRUE, NULL) == LOCALMOVE_VALID);
			}
			
		}//theHardWay


		if (passingCondition)
		{
			flDist = ( vecThreat - node.m_vecOrigin ).Length();

			// is it close?
			if ( flDist > flMinDist && flDist < flMaxDist)
			{
				BOOL unobscurred = FALSE;


				if(!theHardWay){

					// can I see where I want to be from there?
					//MODDD  - WARNING!!!  A TraceLine call will go through simple thin blocking objects like fences, fooling us into thinking
					// that a route from point A to B through a fence is valid, just because a bullet can go through it.
					// How about a trace-hull check instead?
					//CHANGE.
					// That might sound like a good idea, but some AI seems to expect this to still work the way it does.
					// The controller will fail a lot more if we use TRACE_MONSTER_HULL here, but doing that anyway and
					// giving a target like m_hEnemy on the BuildNearestRoute calls isn't good either.
					// Then it goes straight for the player instead of picking routes around the map to move like it should.
					// Really weird stuff.
					UTIL_TraceLine( node.m_vecOrigin + pev->view_ofs, vecLookersOffset, ignore_monsters, edict(), &tr );

					// One of these too ?
					//UTIL_TraceHull(vecFrom, vecPos, ignore_monsters, large_hull, m_hEnemy->edict(), &tr);
					//TRACE_MONSTER_HULL(edict(), pev->origin, Probe, dont_ignore_monsters, edict(), &tr);


					/*
					// This is the most accurate TRACE_MONSTER_HULL way at least.
					// LEFT FOR DEBUGGING, what causes the differences can be interesting
					TraceResult tr2;
					TRACE_MONSTER_HULL(edict(), node.m_vecOrigin + pev->view_ofs, vecLookersOffset, dont_ignore_monsters, edict(), &tr2);

					//if (FClassnameIs(pev, "monster_alien_controller"))
					{

						if (tr.pHit != tr2.pHit) {
							// WHAT NOW
							CBaseEntity* daHit1 = NULL;
							CBaseEntity* daHit2 = NULL;
							const char* classname1 = "NULL";
							const char* classname2 = "NULL";
							if (tr.pHit != NULL) {
								daHit1 = CBaseEntity::Instance(tr.pHit);
							}
							if (tr2.pHit != NULL) {
								daHit2 = CBaseEntity::Instance(tr2.pHit);
							}
							if(daHit1!=NULL)classname1 = daHit1->getClassname();
							if(daHit2!=NULL)classname2 = daHit2->getClassname();
							int x = 45;
							DebugLine_Setup(node.m_vecOrigin + pev->view_ofs, tr.vecEndPos, tr.flFraction);
							DebugLine_Setup(node.m_vecOrigin + pev->view_ofs + Vector(0,0,20), tr2.vecEndPos + Vector(0, 0, 20), tr2.flFraction);

							easyForcePrintLine("TRACE PAIR");
							printBasicTraceInfo(tr);
							easyForcePrintLine();
							printBasicTraceInfo(tr2);
							easyForcePrintLine();

						}

					}
					*/


					// breakpoint stuff
					const char* daClassname = "NULL";

					if (tr.flFraction >= 1.0) {
						// nothing in the way?  well, yeah.
						unobscurred = TRUE;
					}else {
						// hit something?
						if (tr.pHit != NULL) {

							if (pTarget != NULL) {
								// If we have a target and what was hit matches that...  yeah.  That's kind of the point.
								unobscurred = (tr.pHit == pTarget->edict());
							}

							// DEBUG
							CBaseEntity* daHit = CBaseEntity::Instance(tr.pHit);
							if (daHit != NULL) {
								daClassname = daHit->getClassname();
							}
						}
					}


				}else{
					// Passed under 'theHardWay' ?  Already conuts.
					unobscurred = TRUE;

				}
				

				//MODDD - was "tr.flFraction == 1.0", included above too.
				if (unobscurred)
				{
					// try to get there
					//MODDD - involve the now parameterized "iMoveFlag" and "pTarget".
					// WARNING: don't give the moveflag, fools us into thinking the route is to something
					// really specific when it most likely isn't, can be refreshed at bad times (routes to enemies
					// are expected to be accurate to an enemy's locaiton with the enemy moveflag set).
					// Described moreso at the top of the file.
					// For now don't send 'iMoveFlag', the target should be fine maybe?
					// CAREFUL!  No moveflags is bad, retail way of TO_LOCATION is fine.

					

					/*
					// IDEA?  may not be necessary
					// Nope.  Whatever calls BuildNearestRoute requires the 'max' distance to any route to a picked node to be less than
					// the distance to my current enemy.
					if(UseNearestRouteStrictCheck()){
						// The node picked must also be a 'worthy' destination.  Does it even bring us closer to the target ent?
						if(pTarget != NULL){
							float myDistToEnemy = Distance(pev->origin, pTarget->pev->origin);
							float nodeDistToEnemy = Distance(node.m_vecOrigin, pTarget->pev->origin);

							if(nodeDistToEnem < myDistToEnemy * 0.9){
								// at least 10% better distance if I reach it?  Go ahead, allow it.

							}else{
								// No.
								continue;
							}

						}
					}
					*/


					if ( BuildRoute ( node.m_vecOrigin, bits_MF_TO_LOCATION, NULL ) )
					//if ( BuildRoute ( node.m_vecOrigin, iMoveFlag, pTarget ) )
					//if ( BuildRoute ( node.m_vecOrigin, 0, NULL ) )
					{
						flMaxDist = flDist;
						m_vecMoveGoal = node.m_vecOrigin;
						//MODDD - there we go
						WorldGraph.m_iLastCoverSearch = nodeNumber + 1;

						/*
						if(monsterID == 3){
							DebugLine_SetupPoint(m_vecMoveGoal, 255, 125, 255);
						}
						*/

						return TRUE; // UNDONE: keep looking for something closer!
					}
				}
			}
		}
	}

	return FALSE;
}//END OF BuildNearestRoute



//=========================================================
// BestVisibleEnemy - this functions searches the link
// list whose head is the caller's m_pLink field, and returns
// a pointer to the enemy entity in that list that is nearest the 
// caller.
//
// !!!UNDONE - currently, this only returns the closest enemy.
// we'll want to consider distance, relationship, attack types, back turned, etc.
//=========================================================
CBaseEntity *CBaseMonster::BestVisibleEnemy ( void )
{
	CBaseEntity	*pReturn;
	CBaseEntity	*pNextEnt;

	//MODDD - ...why did these used to be integers?  any length is a decimal.
	float		flNearest;
	float		flDist;

	int		iBestRelationship;

	//MODDD - new. Also keep track of all feared enemeis separately.
	CBaseEntity* pFearReturn;
	float flFearNearest;

	float flNormalNearest;  //Of all enemies from Dislike to Nemesis, what is the closest distance found? Not necessarily the distance of the most despised / picked enemy.

	flFearNearest = 8192;
	pFearReturn = NULL;



	flNearest = 8192;// so first visible entity will become the closest.
	pNextEnt = m_pLink;
	pReturn = NULL;

	flNormalNearest = 8192;


	//MODDD - MAJOR! Monsters with "R_FR" for "fear" can be marked enemies too, but AI should want to run away from feared creatures instead of confront them unless cornered.
	iBestRelationship = R_NO;
	//iBestRelationship = R_FR;

	// Note that "R_NO" relationship'd enemies can't be checked. They are not added to the m_pLink list that's populated by the "Look" method called before this BestVisibleEnemy one.

	// slightly different take. If a feared monster is the closest, it gets priority no matter what. Otherwise the usual system goes: "no" up to "nemesis", and then
	//the closest of those at the same highest hatred.

	// another change: keep track of all feared enemies and normal ones (Dislike to Nemesis) separately.
	//If the closest of all feared enemies is closer than closest of all normal enemies (not necessarily just the most despised enemy, who may be further away than that),
	//then we make our enemy the feared enemy instead.


	//R_DL 200
	//R_NE 600
	//R_FR 400


	while ( pNextEnt != NULL )
	{

		/*
		if(FClassnameIs(pev, "monster_scientist")){
			easyForcePrintLine("HERE WE GO %s", pNextEnt->getClassname() );
		}
		*/

		//hgrunt
		//          player  (hates less)
		//          garg   (hates more)

		// sees...  garg, then player
		// sees...  player, then garg.



		//IsAlive_FromAI takes "this" monster as a parameter. It already knows what itself is.
		if ( pNextEnt->IsAlive_FromAI(this) )
		{
			const int relationshipWithNextEnt = IRelationship( pNextEnt);

			BOOL is_pReturn_Enemy = FALSE;
			BOOL is_pNextEnt_Enemy = FALSE;
			if (m_hEnemy != NULL) {
				if (pReturn != NULL) {
					is_pReturn_Enemy = (m_hEnemy->edict() == pReturn->edict());
				}
				is_pNextEnt_Enemy = (m_hEnemy->edict() == pNextEnt->edict());
			}

			//MODDD
			// The return or nextEnt being the enemy plays a role in the distance tested against.
			// That is, against two monsters in sight where one is the enemy, equally hated by this monster,
			// we're going to pick the one that is already the enemy, even at slightly further distance.
			// This stops oscillating between multiple monsters of similar hate-ness coming in and out of being
			// the 'closest' monster screwing with new-enemy responses.
			// 'Oh no, a new enemy!'   'Oh no, a new enemy!'    'Oh no, a new enemy!'.  etc.
			// Relationship still plays the greatest role, but is also no longer an infinitely high priority.
			// Monsters of less relationship that are drastically closer can still be picked, being the existing
			// enemy or not is more negligible.  A hated new monster will probably get picked over a less hated enemy
			// unless it is drastically closer.

			float extraDistanceMulti;

			if (is_pReturn_Enemy && is_pNextEnt_Enemy) {
				// both the enemy?  same ent...?
				extraDistanceMulti = 1;
			}
			else if (is_pReturn_Enemy) {
				// the one I already have in mind is the enemy?  Require more distance to break that.
				extraDistanceMulti = 1.15;
			}
			else if (is_pNextEnt_Enemy) {
				// the one we're looking at is the enemy?  More easily breaks this one in the check
				// (that's 1 / #)
				extraDistanceMulti = 0.87;
			}
			else {
				// fair check then.
				extraDistanceMulti = 1;
			}




			const char* classname_BestYet = "NULL";
			const char* classname_ToTry = "NULL";

			if (pReturn != NULL)classname_BestYet = pReturn->getClassname();
			if (pNextEnt != NULL)classname_ToTry = pNextEnt->getClassname();

			if (monsterID == 1) {
				int x = 45;
			}

			
			if (relationshipWithNextEnt < R_FR) {
				//MODDD - new group/
				// -1, R_FR, is handled.  For any below this,  don't let them be pick-able as an enemy.
				// Would otherwise be possible with the allowance of relationships below the current one,
				// without this catch.

			}else if(relationshipWithNextEnt == R_FR){
				
				flDist = ( pNextEnt->pev->origin - pev->origin ).Length();

				float flDistTest = flDist * extraDistanceMulti;

				//If I fear this one and he's the closest, he's the best enemy yet.
				//MODDD - with the extra padding in the check of course.
				if (flDistTest <= flFearNearest )
				{
					flFearNearest = flDist;
					pFearReturn = pNextEnt;
				}


			}else if ( relationshipWithNextEnt > iBestRelationship )
			{
				// this entity is disliked MORE than the entity that we 
				// currently think is the best visible enemy. No need to do 
				// a distance check, just get mad at this one for now.
				
				//MODDD - CHANGED.  Relationship is a big factor, but not all there is.  See below.

				/*
				iBestRelationship = relationshipWithNextEnt;
				flNearest = ( pNextEnt->pev->origin - pev->origin ).Length();
				pReturn = pNextEnt;

				if(flNearest < flNormalNearest){
					flNormalNearest = flNearest; //just keeping track of the closest "Dislike - Nemesis" range enemy.
				}
				*/

				//MODDD
				// Rather, an entity hated is given more priority at a further distance.

				
				flDist = (pNextEnt->pev->origin - pev->origin).Length();

				float flDistTest;
				int iRelationshipDifference = min(relationshipWithNextEnt - iBestRelationship, 3);
				//flDistTest = flDist * (0.6 / iRelationshipDifference);
				flDistTest = flDist * extraDistanceMulti * (0.65 - iRelationshipDifference*0.1 );
				//0.65  0.55  0.45  0.35

				//if (monsterID == 1) {
				//	int x = 45;
				//	if (pNextEnt->IsPlayer()) {
				//		int x = 45;
				//	}
				//}
				if (flDistTest <= flNearest)
				{
					//if (monsterID == 1) {
					//	int x = 45;
					//	if (pNextEnt->IsPlayer()) {
					//		int x = 45;
					//	}
					//}
					flNearest = flDist;
					iBestRelationship = relationshipWithNextEnt;
					pReturn = pNextEnt;
				}
				if (flDistTest < flNormalNearest) {
					flNormalNearest = flDist; //just keeping track of the closest "Dislike - Nemesis" range enemy.
				}

				
			}
			else if (relationshipWithNextEnt < iBestRelationship)
			{
				//MODDD - NEW SECTION.
				// One with a lesser relationship can overtake the current one, provided it is an enemy.

				flDist = (pNextEnt->pev->origin - pev->origin).Length();

				float flDistTest;
				int iRelationshipDifference = min(iBestRelationship - relationshipWithNextEnt, 3);
				//flDistTest = flDist * (1.67 * iRelationshipDifference);
				
				//flDistTest = flDist * (1.45 + (iRelationshipDifference*0.1) );
				//1.54  1.81  2.22  2.857
				//    0.27  0.41   0.637
				//  UHHhhhhhhhhhhhh nevermind coming up with a formula for that.

				if (iRelationshipDifference == 1) {
					flDistTest = flDist * extraDistanceMulti * 1.81;
				}
				else if (iRelationshipDifference == 2) {
					flDistTest = flDist * extraDistanceMulti * 2.22;
				}
				else {
					flDistTest = flDist * extraDistanceMulti * 2.857;
				}



				if (flDistTest <= flNearest)
				{
					flNearest = flDist;
					iBestRelationship = relationshipWithNextEnt;
					pReturn = pNextEnt;
				}
				if (flDistTest < flNormalNearest) {
					flNormalNearest = flDist; //just keeping track of the closest "Dislike - Nemesis" range enemy.
				}


			}
			else if ( relationshipWithNextEnt == iBestRelationship )
			{
				// this entity is disliked just as much as the entity that
				// we currently think is the best visible enemy, so we only
				// get mad at it if it is closer.
				//MODDD - CHANGE.  A monster of equal relathionship becomes the new enemy IF 
				// it is the same distance + some percent of that distance of the other one
				flDist = ( pNextEnt->pev->origin - pev->origin ).Length();

				float flDistTest;
				flDistTest = flDist * extraDistanceMulti;


				if (flDistTest <= flNearest )
				{
					flNearest = flDist;
					iBestRelationship = relationshipWithNextEnt;
					pReturn = pNextEnt;
				}
				if(flDistTest < flNormalNearest){
					flNormalNearest = flDist; //just keeping track of the closest "Dislike - Nemesis" range enemy.
				}

			}
		}//END OF this monster alive (as far as the AI cares) check

		pNextEnt = pNextEnt->m_pLink;
	}//END OF while loop through all seen monsters list "m_pLink".



	//Now, make a decision. 
	if(pReturn == NULL && pFearReturn == NULL){
		//both null? Nothing to do.
		return NULL;
	}else if(pReturn != NULL && pFearReturn == NULL){
		//pReturn isn't null but the other is? return this.
		return pReturn;
	}else if(pReturn == NULL && pFearReturn != NULL){
		//vice versa. the not-null one.
		return pFearReturn;
	}else{
		//both are not null? Whichever one is closer, do that. pReturn, the hostile, is a tie-breaker for equal distance.

		if(flNormalNearest <= flFearNearest){
			return pReturn;
		}else{
			return pFearReturn;
		}
	}

	// should not be reached.
	return pReturn;
}//END OF BestVisibleEnemy


//=========================================================
// MakeIdealYaw - gets a yaw value for the caller that would
// face the supplied vector. Value is stuffed into the monster's
// ideal_yaw
//=========================================================
void CBaseMonster::MakeIdealYaw( Vector vecTarget )
{
	//MODDD - this kind of immitation "Strafe" is too crude and causes issues with the AI sometimes, like fiddling-around near the end point of a path..
	// strafing monster needs to face 90 degrees away from its goal
	if ( m_movementActivity == ACT_STRAFE_LEFT )
	{
		Vector	vecProjection;
		vecProjection.x = -vecTarget.y;
		vecProjection.y = vecTarget.x;


		//UTIL_drawLineFrame(pev->origin, (vecProjection + Vector(0, 0, vecTarget.z + 10) ), 7, 0, 255, 124 );
		pev->ideal_yaw = UTIL_VecToYaw( vecProjection - pev->origin );
	}
	else if ( m_movementActivity == ACT_STRAFE_RIGHT )
	{
		Vector	vecProjection;
		vecProjection.x = vecTarget.y;
		vecProjection.y = vecTarget.x;

		
		//UTIL_drawLineFrame(pev->origin, (vecProjection + Vector(0, 0, vecTarget.z + 10) ), 7, 255, 0, 124 );
		pev->ideal_yaw = UTIL_VecToYaw( vecProjection - pev->origin );
	}
	else
	{
		//vecProjection.x = vecTarget.x;
		//vecProjection.y = vecTarget.y;


		//UTIL_drawLineFrame(pev->origin, (vecProjection + Vector(0, 0, vecTarget.z + 10) ), 7, 255, 255, 255 );
		pev->ideal_yaw = UTIL_VecToYaw ( vecTarget - pev->origin );
	}


}

//=========================================================
// FlYawDiff - returns the difference ( in degrees ) between
// monster's current yaw and ideal_yaw
//
// Positive result is left turn, negative is right turn
//=========================================================
float CBaseMonster::FlYawDiff ( void )
{
	float flCurrentYaw;

	flCurrentYaw = UTIL_AngleMod( pev->angles.y );

	if ( flCurrentYaw == pev->ideal_yaw )
	{
		return 0;
	}


	return UTIL_AngleDiff( pev->ideal_yaw, flCurrentYaw );
}


//=========================================================
// Changeyaw - turns a monster towards its ideal_yaw
//=========================================================
float CBaseMonster::ChangeYaw ( int yawSpeed )
{
	float	ideal, current, move, speed;

	current = UTIL_AngleMod( pev->angles.y );
	ideal = pev->ideal_yaw;
	if (current != ideal)
	{
		speed = (float)yawSpeed * gpGlobals->frametime * 10;
		move = ideal - current;

		if (ideal > current)
		{
			if (move >= 180)
				move = move - 360;
		}
		else
		{
			if (move <= -180)
				move = move + 360;
		}

		if (move > 0)
		{// turning to the monster's left
			if (move > speed)
				move = speed;
		}
		else
		{// turning to the monster's right
			if (move < -speed)
				move = -speed;
		}
		
		pev->angles.y = UTIL_AngleMod (current + move);

		// turn head in desired direction only if they have a turnable head
		if (m_afCapability & bits_CAP_TURN_HEAD)
		{
			float yaw = pev->ideal_yaw - pev->angles.y;
			if (yaw > 180) yaw -= 360;
			if (yaw < -180) yaw += 360;
			// yaw *= 0.8;
			SetBoneController( 0, yaw );
		}
	}
	else
		move = 0;

	return move;
}

//=========================================================
// VecToYaw - turns a directional vector into a yaw value
// that points down that vector.
//=========================================================
float CBaseMonster::VecToYaw ( Vector vecDir )
{
	if (vecDir.x == 0 && vecDir.y == 0 && vecDir.z == 0)
		return pev->angles.y;

	return UTIL_VecToYaw( vecDir );

}




//=========================================================
// SetEyePosition
//
// queries the monster's model for $eyeposition and copies
// that vector to the monster's view_ofs
//
//=========================================================
void CBaseMonster::SetEyePosition ( void )
{
	Vector  vecEyePosition;
	void *pmodel = GET_MODEL_PTR( ENT(pev) );

	GetEyePosition( pmodel, vecEyePosition );

	pev->view_ofs = vecEyePosition;

	if ( pev->view_ofs == g_vecZero )
	{
		ALERT ( at_aiconsole, "%s has no view_ofs!\n", STRING ( pev->classname ) );
	}
}

void CBaseMonster::HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch( pEvent->event )
	{
	case SCRIPT_EVENT_DEAD:
		if ( m_MonsterState == MONSTERSTATE_SCRIPT )
		{
			pev->deadflag = DEAD_DYING;
			// Kill me now! (and fade out when CineCleanup() is called)
#if _DEBUG
			ALERT( at_aiconsole, "Death event: %s\n", STRING(pev->classname) );
#endif
			pev->health = 0;


			//MODDD - HACKY HACKY HACKY.
			// Look for any func_monsterclip close to where this happened and make it non-collidable.
			// Yes, this is diabolical 
			////////////////////////////////////////////////////////////////////////////////////////////////
			Vector boundCenter = Center();

			int boundExtent = 60; //50
			Vector boundCheckMin = boundCenter + Vector(-boundExtent, -boundExtent, -boundExtent);
			Vector boundCheckMax = boundCenter + Vector(boundExtent, boundExtent, boundExtent);
			CBaseEntity *pList[64];
			int count = UTIL_EntitiesInBox(
				pList,
				64,
				boundCheckMin,
				boundCheckMax,
				// (FL_CLIENT|FL_MONSTER) for players and monsters only
				0
			);
			//UTIL_drawBoxFrame(boundCheckMin, boundCheckMax, 12, 1000, 0, 255, 255);

			for(int i = 0; i < count; i++){
				if(pList[i]->pev != this->pev ){
					if(FClassnameIs(pList[i]->pev, "func_monsterclip")){
						BOOL hasMonsterClip = (pList[i]->pev->flags & FL_MONSTERCLIP) != 0;
						if(hasMonsterClip && pList[i]->pev->solid != SOLID_NOT && pList[i]->pev->solid != SOLID_TRIGGER ){
							// Wait!  One more check.  Is this bound's center close enough to mine?  Bounds whose center are off greatly
							// is a sign they are very large and not meant for just this monster.
							float distToTrigCenter = Distance(pev->origin, pList[i]->Center());
							float distToTrigCenter2 = Distance(pev->origin, VecBModelOrigin(pList[i]->pev));

							if(distToTrigCenter <= 50){

								//DebugLine_SetupPoint(5, pList[i]->Center(), 255, 255, 0);
								//DebugLine_SetupPoint(6, VecBModelOrigin(pList[i]->pev), 0, 255, 255);
								//DebugLine_SetupPoint(7, pev->origin, 255, 0, 0);
								
								// a working MONSTERCLIP that's close enough?  Disable for now.
								pList[i]->pev->solid = SOLID_NOT;
							}//close enough (distance)
						}//has monsterclip flag and bounds are obstructive
					}//is func_monsterclip
				}//not-me check
			}//loop through nearby entities

			// Make my bounds invisible too right now.  Some scripted things put visible model far away
			// very quickly, so continuing to be an obstacle doesn't make much sense.
			// If doing it this soon is ever an issue, say so.
			this->pev->solid = SOLID_NOT;

			////////////////////////////////////////////////////////////////////////////////////////////////

		}//SCRIPT_EVENT_DEAD
#if _DEBUG
		else{
			ALERT( at_aiconsole, "INVALID death event:%s\n", STRING(pev->classname) );
		}
#endif
		break;
	case SCRIPT_EVENT_NOT_DEAD:
		if ( m_MonsterState == MONSTERSTATE_SCRIPT )
		{
			pev->deadflag = DEAD_NO;
			// This is for life/death sequences where the player can determine whether a character is dead or alive after the script 
			pev->health = pev->max_health;
		}
		break;

	case SCRIPT_EVENT_SOUND: {			// Play a named wave file
		//MODDD - so do we use the soundsentencesave system here or not then?   Hm.
		// Depends on if it's a sound usually played (called for in its file, agrunt.cpp, etc.).
		// Scripted's may precache their own sounds that don't normally come from this entity.
		// So, little risky of an assumption.
		// Compromise:  If it's in a scripted state, assume the monster can play unusual sequences or invoke unusual sounds.
		// But run through a monster's commonly-played sequences (no script intervention needed to play idle anims, most
		// attack ones, etc.)
		// So don't trust the soundsentencesave system when scripted.  It's required to precahce any sounds that come
		// from sequences expected to be played in that monster while scripted anyway.
		// Otherwise (not scripted), assume soundsentencesave can catch the sound
		BOOL useSoundSentenceSave = (m_MonsterState != MONSTERSTATE_SCRIPT);
		// MODDD - scripted sounds now carry further.  Lower attenuation.  Was ATTN_IDLE (2).
		UTIL_PlaySound(edict(), CHAN_BODY, pEvent->options, 1.0, ScriptEventSoundAttn(), 0, 100, useSoundSentenceSave);
	}
	break;

	case SCRIPT_EVENT_SOUND_VOICE: {
		//MODDD - you too.
		// And a smaller attenuation change (was also ATTN_IDLE)
		BOOL useSoundSentenceSave = (m_MonsterState != MONSTERSTATE_SCRIPT);
		UTIL_PlaySound(edict(), CHAN_VOICE, pEvent->options, 1.0, ScriptEventSoundVoiceAttn(), 0, 100, useSoundSentenceSave);
	}break;

	case SCRIPT_EVENT_SENTENCE_RND1:		// Play a named sentence group 33% of the time
		if (RANDOM_LONG(0,2) == 0)
			break;
		// fall through...
	case SCRIPT_EVENT_SENTENCE:			// Play a named sentence group
		SENTENCEG_PlayRndSz( edict(), pEvent->options, 1.0, ATTN_IDLE, 0, 100 );
		break;

	case SCRIPT_EVENT_FIREEVENT:		// Fire a trigger
		FireTargets( pEvent->options, this, this, USE_TOGGLE, 0 );
		break;

	case SCRIPT_EVENT_NOINTERRUPT:		// Can't be interrupted from now on
		if ( m_pCine )
			m_pCine->AllowInterrupt( FALSE );
		break;

	case SCRIPT_EVENT_CANINTERRUPT:		// OK to interrupt now
		if ( m_pCine )
			m_pCine->AllowInterrupt( TRUE );
		break;

#if 0
	case SCRIPT_EVENT_INAIR:			// Don't DROP_TO_FLOOR()
	case SCRIPT_EVENT_ENDANIMATION:		// Set ending animation sequence to
		break;
#endif

	case MONSTER_EVENT_BODYDROP_HEAVY:
		if ( pev->flags & FL_ONGROUND )
		{
			if ( RANDOM_LONG( 0, 1 ) == 0 )
			{
				UTIL_PlaySound( ENT(pev), CHAN_BODY, "common/bodydrop3.wav", 1, ATTN_NORM, 0, 90, FALSE );
			}
			else
			{
				UTIL_PlaySound( ENT(pev), CHAN_BODY, "common/bodydrop4.wav", 1, ATTN_NORM, 0, 90, FALSE );
			}
		}
		break;

	case MONSTER_EVENT_BODYDROP_LIGHT:
		if ( pev->flags & FL_ONGROUND )
		{
			if ( RANDOM_LONG( 0, 1 ) == 0 )
			{
				UTIL_PlaySound( ENT(pev), CHAN_BODY, "common/bodydrop3.wav", 1, ATTN_NORM, FALSE );
			}
			else
			{
				UTIL_PlaySound( ENT(pev), CHAN_BODY, "common/bodydrop4.wav", 1, ATTN_NORM, FALSE );
			}
		}
		break;
	case MONSTER_EVENT_SWISHSOUND:
		{
			// NO MONSTER may use this anim event unless that monster's precache precaches this sound!!! this one uses the soundsentencesave.
			UTIL_PlaySound( ENT(pev), CHAN_BODY, "zombie/claw_miss2.wav", 1, ATTN_NORM, TRUE );
			break;
		}
	default:
		ALERT( at_aiconsole, "Unhandled animation event %d for %s\n", pEvent->event, STRING(pev->classname) );
		break;

	}
}




// Combat

//MODDD - GetGunPositionAI is a new different version of GetGunPosition for use by AI calls. Things like 
//        This version should be less precise to the literal point of the gun on the model, which may varry with what sequence or frame is
//        picked. The attachment position used by some monsters can change often enough to let the player hide behind cover just right to
//        make the AI oscillate between "my gun shifted a little to the right... obscurred by cover" and "my gun shifted a little to the 
//        left... start firing!" and back and forth.
//        By comparing a more absolute position on the monster to the enemy's position intead, just a constant offset from the monster's origin,
//        this should not happen. Actual bullets can continue to come from the more accurate "GetGunPosition" below.

//        Note that this only applies to monsters that get the offset from attachment logic in getGunPosition, not an absolute offset.
//        In those cases, GetGunPositionAI's default behavior of just calling GetGunPosition works perfectly fine. Never that much precision used there.
	
Vector CBaseMonster::GetGunPosition( )
{
	UTIL_MakeVectors(pev->angles);

	// Vector vecSrc = pev->origin + gpGlobals->v_forward * 10;
	//vecSrc.z = pevShooter->absmin.z + pevShooter->size.z * 0.7;
	//vecSrc.z = pev->origin.z + (pev->view_ofs.z - 4);
	Vector vecSrc = pev->origin 
					+ gpGlobals->v_forward * m_HackedGunPos.y 
					+ gpGlobals->v_right * m_HackedGunPos.x 
					+ gpGlobals->v_up * m_HackedGunPos.z;
	return vecSrc;
}

Vector CBaseMonster::GetGunPositionAI(){
	//return GetGunPosition();
	// For safety, this will be a clone of GetGunPosition, not a redirect. This means we need to rely on m_HackedGunPos like a Monster would have before,
	// not possibly redirect to a particular monster's own overly precise GetGunPosition.
	UTIL_MakeVectors(pev->angles);
	Vector vecSrc = pev->origin 
					+ gpGlobals->v_forward * m_HackedGunPos.y 
					+ gpGlobals->v_right * m_HackedGunPos.x 
					+ gpGlobals->v_up * m_HackedGunPos.z;
	return vecSrc;
}


//MODDD - this is commonly used to aim the torso of a monster pitch-wise to look at its enemy.  The model must
//        support this, and this is assuming blend #0 handles that.
void CBaseMonster::lookAtEnemy_pitch(void){
	Vector vecShootDir;
	Vector angDir;

	if (m_hEnemy != NULL) {
		// test.  Is the enemy too close?

		//float tempDist = Distance(pev->origin, m_hEnemy->Center());
		Vector pointDelta = (this->Center() - m_hEnemy->Center());
		float tempDist = DistanceFromDelta(pointDelta);
		float tempDistZ = Distance2DFromDelta(pointDelta);

		
		if (tempDist < 94 || tempDistZ < 60) {
			// don't use the pitch!  It gets odd looking trying to aim extremely close.
			float thePitch = 0;
			SetBlending(0, thePitch);
			return;
		}
	}

	UTIL_MakeVectors(pev->angles);
	// MODDD - is it fine to use the AI variant?
	// Want something more approximate than our actual 'gun' height to be used for determining what's looking at
	// something anyway
	vecShootDir = ShootAtEnemyMod(GetGunPositionAI() );

	/*
	if (m_hEnemy != NULL) {
		Vector shootOrigin = GetGunPositionAI();
		::DebugLine_ClearAll();
		DebugLine_Setup(0, shootOrigin, m_hEnemy->BodyTargetMod(shootOrigin), 0, 255, 0);
		DebugLine_Setup(1, shootOrigin, shootOrigin + vecShootDir * 300, 1.0, 0, 0, 255);
	}
	*/
	
	angDir = UTIL_VecToAngles( vecShootDir );
	
	// make angles +-180
	if (angDir.x > 180){
		angDir.x = angDir.x - 360;
	}


	SetBlending( 0, angDir.x );
}//lookAtEnemy_pitch



//=========================================================
// NODE GRAPH
//=========================================================

//=========================================================
// FGetNodeRoute - tries to build an entire node path from
// the callers origin to the passed vector. If this is 
// possible, ROUTE_SIZE waypoints will be copied into the
// callers m_Route. TRUE is returned if the operation 
// succeeds (path is valid) or FALSE if failed (no path 
// exists )
//=========================================================
//MODDD - New overridable versions.  Now redirects to FGetNodeRoute_Final 
// (takes the start/end nodes picked here).
//MODDD - TODO:  Send a vecStart like CheckLocalMove does, send that along as pev->origin?
// Seems neater than assuming the start is always where this entity happens to be in
// any call but it's likely hte case anyway.
BOOL CBaseMonster::FGetNodeRoute ( Vector vecStart, Vector vecDest ){
	// assume not using that feature
	return FGetNodeRoute(vecStart, vecDest, FALSE);
}
BOOL CBaseMonster::FGetNodeRoute ( Vector vecStart, Vector vecDest, BOOL asIfSnappedToGround ){
	// mark this.  If it is, being too close to the ground on 'asIfSnappedToGround'
	// stops the method (already tried close to that point in a previous run)
	//Vector vecStartPos;
	Vector vecLookForSourceNodeFrom;
	TraceResult tr;
	int iSrcNode;
	int iDestNode;
	BOOL returnCode = FALSE;

	// Record the monster checking the nodes for any node logic to be aware of (per some CVars) this call.
	// Retail was never aware of this in nodes.cpp, besides getting the hull-size of the calling monster
	// early on.

	// uses default behavior to get type if the monster doesn't override getNodeTypeAllowed
	int myAllowedNodeTypes = WorldGraph.NodeType(this); //this->getNodeTypeAllowed();
	int flyerAllowedTypes;


	//vecStartPos = pev->origin;


	// nope, no 'vecStart' provided here.  yet.
	//UTIL_SetOrigin( pev, vecStart );

	// NOTE - ground-based monsters don't even need this, it is assumed they're snapped to the ground at most times,
	// or not significanty off the ground enough to impact 'nearestNode' choice (main thing that matters).
	// This is for fliers to see if some ground node straight below is a better option if all else fails
	// (if the flyer can also take ground nodes)
	if(asIfSnappedToGround){
		/*
		// Wait.  Why not just use a TRACE_MONSTER_HULL that goes deep down?
		// DROP_TO_FLOOR even has a limit to its check.   In the retail game, 'give' commands can fail
		// if spawned while the player noclipped somewhere far above the ground (deletes itself because
		// it can't find ground to spawn on, because of this command.)
		int dropResult = DROP_TO_FLOOR( ENT( pev ) );
		if(dropResult == 0){
			// ok, try again with a TRACE_MONSTER_HULL to there
		}
		*/
		TraceResult trDrop;
		Vector vecDropStart = pev->origin;
		Vector vecDropEnd = vecDropStart;
		vecDropEnd.z = -1024;
		TRACE_MONSTER_HULL(ENT(pev), vecDropStart, vecDropEnd, dont_ignore_monsters, ENT(pev), &trDrop);

		if(trDrop.flFraction == 0){
			// Didn't reach the ground?  Not happening
			return FALSE;
		}

		if(g_FGetNodeRoute_recursiveCall){
			// Coming from another run of this method already?  Forbid if the drop was close to the ground.
			float dropDist = fabs(trDrop.vecEndPos.z - (pev->origin.z + pev->mins.z));
			if(dropDist < 240){
				// don't bother trying again, too short a distance
				return FALSE;
			}
		}
		
		// Assuming drop success at this point.
		// And that setting to the exact endpoint of TRACE_MONSTER_HULL is ok.
		// WAIT!  No need, just use this as the pos to look for nodes from instead.
		// Bump the Z a little out of pranoia.
		//UTIL_SetOrigin(pev, trDrop.vecEndPos);
		vecLookForSourceNodeFrom = trDrop.vecEndPos;
		vecLookForSourceNodeFrom.z += 2;

		// Also - forbid any node types but LAND if snapping to the ground.
		myAllowedNodeTypes = myAllowedNodeTypes &= ~(bits_NODE_AIR | bits_NODE_WATER);
		flyerAllowedTypes = 0;
	}else{
		vecLookForSourceNodeFrom = pev->origin;
		if(!map_anyAirNodes){
			// strip out the bits_NODE_AIR then, none use it.  Wasted search if it's the only flyer
			// movetype.
			myAllowedNodeTypes &= ~bits_NODE_AIR;
		}
		flyerAllowedTypes = myAllowedNodeTypes & (bits_NODE_AIR | bits_NODE_WATER);
	}//asIfSnappedToGround check


	if(!g_FGetNodeRoute_recursiveCall){
		g_routeTempMonster = this;
		// Assume the movegoal is set if it will be relevant?
		g_routeTempMonster_GoalEnt = GetGoalEntity();
	}


	// Is a flyer/swimmer (included by isMovetypeFlying), and supports at least LAND nodes?
	// Can try snapping to the ground of neither non-ground nor ground node type works.
	// Note checking for isMovetypeFlying instead of flyerAllowedTypes.  flyerAllowedTypes may get 0'd
	// from this map lacking air-nodes, but it should still try to snap to the ground to look for
	// a nearest node from there if the only ground-node lookup at its current place fails.
	//if( flyerAllowedTypes != 0 && (myAllowedNodeTypes & bits_NODE_LAND) ){
	if(isMovetypeFlying() && (myAllowedNodeTypes & bits_NODE_LAND)){

		/*
		if(map_anyAirNodes){
			// Try getting the nearest land and air node.
			// If both are found, always try the air node first for a route.  Then the land if that fails.
			// If only either is found, try that one of course.
			iSrcNode = WorldGraph.FindNearestNode ( pev->origin, flyerAllowedTypes );
			iDestNode = WorldGraph.FindNearestNode ( vecDest, flyerAllowedTypes );
			returnCode = FGetNodeRoute_Final(iSrcNode, iDestNode, vecDest);

		}else{
			// Just do a normal node search then, the nearest land-node is all that can be grabbed since
			// there are no air nodes.
			// Wait, there is one really odd case that shouldn't happen.  bits_NODE_WATER and bits_NODE_LAND?
			// The map lacking air nodes has no effect on that.  Try that first if so?
			if(flyerAllowedTypes & bits_NODE_WATER){
				iSrcNode = WorldGraph.FindNearestNode ( pev->origin, bits_NODE_WATER );
				iDestNode = WorldGraph.FindNearestNode ( vecDest, bits_NODE_WATER );
				returnCode = FGetNodeRoute_Final(iSrcNode, iDestNode, vecDest);
			}else{
				// to make the if-then below work
				returnCode = FALSE;
			}
		}
		*/

		// flyer/water node types get priority, if any are left.
		// Either this flying/swimming monster never had any (???), or it had the AIR type but it got
		// stripped from this map lacking air nodes.
		if(flyerAllowedTypes != 0){
			iSrcNode = WorldGraph.FindNearestNode ( vecLookForSourceNodeFrom, flyerAllowedTypes );
			iDestNode = WorldGraph.FindNearestNode ( vecDest, flyerAllowedTypes );
			returnCode = FGetNodeRoute_Final(vecLookForSourceNodeFrom, vecDest, asIfSnappedToGround, iSrcNode, iDestNode, flyerAllowedTypes);
		}

		// Above failed?  Try ground nodes.
		if(!returnCode){
			iSrcNode = WorldGraph.FindNearestNode ( vecLookForSourceNodeFrom, bits_NODE_LAND );
			iDestNode = WorldGraph.FindNearestNode ( vecDest, bits_NODE_LAND );
			returnCode = FGetNodeRoute_Final(vecLookForSourceNodeFrom, vecDest, asIfSnappedToGround, iSrcNode, iDestNode, bits_NODE_LAND);
		}

		// Regardless of whether the map has any air nodes or not, if any attempts to make a route above
		// failed while both node-types are accepted, try one more time: snap the flyer to the ground
		// (hypothetical) and see what the nearest ground-node only is.
		// Assumption is, the nearest air-node failing should not have made much of a difference.  Any air nodes
		// from where the flyer already was should have connected to whatever nearest air node is at ground-level,
		// maybe even the exact same air node.
		// Ground node, however, could be different, especially on maps with different floors/elevations.  A 
		// nearest ground node found while snapped to the ground could actually work instead.
		// If that succeeds, place an apex (detour) node straight to the ground to lead to this node and then
		// go into the given route.

		// If this attempt wasn't already set as 'snappedToGround' (no need to do any other checks),
		// can re-try as if were.
		// (this will set g_routeTempMonster again, but no issue, it is to the exact same monster)
		// ALTHOUGH.  Beware that it also sets g_routeTempMonster to NULL after it finishes.
		// Which is fine as it's the last place in this method that calls any WorldGraph (nodes.cpp)
		// methods anyway.  (even that's changed now, checks for the recursive call in those too)
		if(!returnCode && !asIfSnappedToGround){
			g_FGetNodeRoute_recursiveCall = TRUE;
			returnCode = FGetNodeRoute(vecStart, vecDest, TRUE);
			g_FGetNodeRoute_recursiveCall = FALSE;
		}

	}else{
		// ground mover?  only either LAND,  or  AIR and/or SWIM?  Nothing special.
		iSrcNode = WorldGraph.FindNearestNode ( vecLookForSourceNodeFrom, myAllowedNodeTypes );
		iDestNode = WorldGraph.FindNearestNode ( vecDest, myAllowedNodeTypes );

		returnCode = FGetNodeRoute_Final(vecLookForSourceNodeFrom, vecDest, asIfSnappedToGround, iSrcNode, iDestNode, myAllowedNodeTypes);
	}//myAllowedNodeTypes check

	if(!g_FGetNodeRoute_recursiveCall){
		 // done with node-based logic
		g_routeTempMonster = NULL;
	}

	
	/*
	if(asIfSnappedToGround){
		// since we've moved the monster during the check, undo the move.
		// (if 'vecStart' becomes a paramter, this happens regardless of 'asIfSnappedToGround'
		UTIL_SetOrigin( pev, vecStartPos );
	}
	*/

	return returnCode;
}//FGetNodeRoute



//MODDD - NEW, portion of FGetNodeRoute separated out.
// Takes the iSrcNode and iDestNode indeces of nodes and uses them to see 
// if there is a shortest path between the nodes and if that path is feasible
// for this monster to reach the start/end nodes.
// Takes the 'vecDest' too for comparing the destination node to (close enough gets
// a CheckLocalMove from vecDest to the node).
// If a version that doesn't do the start/end valid checks is ever wanted,
// that can be made.
BOOL CBaseMonster::FGetNodeRoute_Final(Vector vecStart, Vector vecDest, BOOL asIfSnappedToGround, int iSrcNode, int iDestNode, int iNodeTypeInfo){
	int i;
	int iPathSize;
	int iNumToCopy;
	int iPath[ MAX_PATH_SIZE ];

	//easyForcePrintLine("MY GOD   WHAT ARE YOU DOING %d %d", iSrcNode, iDestNode);

	if ( iSrcNode == -1 ){
		// no node nearest self
//		ALERT ( at_aiconsole, "FGetNodeRoute: No valid node near self!\n" );
		return FALSE;
	}else if ( iDestNode == -1 ){
		// no node nearest target
//		ALERT ( at_aiconsole, "FGetNodeRoute: No valid node near target!\n" );
		return FALSE;
	}


	//MODDD - NEW SECTION.  Disabling for testing.
	// Seems something about one of the NEW sections here in FGetNodeRoute can cause slowdowns at random,
	// probably being called too often for traces to be worthwhile or a trace-hull method going a ridiculous distance
	// (like controllers across the entire map on a route).   Should be possible to check only up to so far like 400 units though,
	// try retesting c4a1a at a high framerate (host_framerate 0.5) and waiting for lagginess with that.
	/*
	// oh.  WorldGraph.Node is just a layer for m_pNodes, ok.
	CNode& theStartNode = WorldGraph.Node(iSrcNode);
	//MODDD - HOLD ON!  Can we even get to the iSrcNode?  Let's be safe.
	TRACE_MONSTER_HULL(edict(), pev->origin + pev->view_ofs, theStartNode.m_vecOrigin + pev->view_ofs, dont_ignore_monsters, edict(), &tr);
	if (tr.flFraction < 1.0) {
		// oh dear.  Abort.  We picked a source node we can not even reach.
		//DebugLine_Setup(0, pev->origin + pev->view_ofs, theStartNode.m_vecOrigin + pev->view_ofs, tr.flFraction);
		return FALSE;
	}
	*/

	// valid src and dest nodes were found, so it's safe to proceed with
	// find shortest path
	int iNodeHull;

	if (EASY_CVAR_GET_DEBUGONLY(pathfindForcePointHull) != 1) {
		// normal way.  Get the Hull from this monster trying to pathfind.
		// Can be used to tell if some paths between nodes are invalid from this monster's size.
		iNodeHull = WorldGraph.HullIndex( this, iNodeTypeInfo ); // make this a monster virtual function
	}else{
		// force 0 size.
		iNodeHull = NODE_POINT_HULL;
	}

	//MODDD - CHEAT CHEAT CHEAT!
	//iNodeHull = NODE_POINT_HULL;

	iPathSize = WorldGraph.FindShortestPath ( iPath, iSrcNode, iDestNode, iNodeHull, m_afCapability );


	//MODDD - used to be only a 'path size not 0' check
	BOOL routeGood = FVerifyRoute(vecStart, vecDest, iPath, iPathSize);

	//if ( !iPathSize ){
	if(!routeGood){
#if 1
		ALERT ( at_aiconsole, "No Path from %d to %d!\n", iSrcNode, iDestNode );
		return FALSE;
#else
		BOOL bRoutingSave = WorldGraph.m_fRoutingComplete;
		WorldGraph.m_fRoutingComplete = FALSE;
		iPathSize = WorldGraph.FindShortestPath(iPath, iSrcNode, iDestNode, iNodeHull, m_afCapability);
		WorldGraph.m_fRoutingComplete = bRoutingSave;
		if ( !iPathSize )
		{
			ALERT ( at_aiconsole, "No Path from %d to %d!\n", iSrcNode, iDestNode );
			return FALSE;
		}
		else
		{
			ALERT ( at_aiconsole, "Routing is inconsistent!" );
		}
#endif
	}



	int iRouteWriteStart = 0;


	if(asIfSnappedToGround && g_FGetNodeRoute_recursiveCall){
		// Straight-down shot?  Then the route needs to begin with a node to say to head straight down
		m_Route[0].vecLocation = vecStart;
		// AAAaaaaaaaaaaallllllllllssssssssooooooooo don't fuck around with 
		// bits_MF_TO_PATHCORNER unless you know what you're doing.
		// Cuz uh.  I sure don't.
		m_Route[0].iType = bits_MF_TO_DETOUR; // | bits_MF_DONT_SIMPLIFY;
		m_Route[0].iMapNodeIndex = -1;
		iRouteWriteStart = 1;
	}// snaptoground + recursive check

	// there's a valid path within iPath now, so now we will fill the route array
	// up with as many of the waypoints as it will hold.
	
	// don't copy ROUTE_SIZE entries if the path returned is shorter
	// than ROUTE_SIZE!!!
	if ( iRouteWriteStart + iPathSize < ROUTE_SIZE ){
		// enough room to write everything out
		iNumToCopy = iPathSize;
	}else{
		iNumToCopy = ROUTE_SIZE - iRouteWriteStart;
	}
	
	for ( i = 0 ; i < iNumToCopy; i++ ){
		int iRouteWriteIndex = i + iRouteWriteStart;
		int theMapNodeIndex = iPath[i];
		m_Route[iRouteWriteIndex].vecLocation = WorldGraph.m_pNodes[ theMapNodeIndex ].m_vecOrigin;
		m_Route[iRouteWriteIndex].iType = bits_MF_TO_NODE;
		//MODDD - record the map node this waypoint was placed on.
		m_Route[iRouteWriteIndex].iMapNodeIndex = theMapNodeIndex;
	}
	
	if ( iNumToCopy + iRouteWriteStart < ROUTE_SIZE ){
		int iRouteWriteIndex = iNumToCopy + iRouteWriteStart;
		m_Route[ iRouteWriteIndex ].vecLocation = vecDest;
		// why not both?
		// WAIT - the final node leading to the goal should look at what type of thing
		// we're moving towards, right?  Use MoveGoal to determine that (toEnemey, etc.)
		//m_Route[ iRouteWriteIndex ].iType |= bits_MF_IS_GOAL;
		//m_Route[ iRouteWriteIndex ].iType |= (bits_MF_TO_NODE | bits_MF_IS_GOAL);
		int moveBit = MovementGoalToMoveFlag(m_movementGoal);
		m_Route[ iRouteWriteIndex ].iType |= ( moveBit | bits_MF_IS_GOAL);
		m_Route[iRouteWriteIndex].iMapNodeIndex = -1;

		// Made an extra end-node lead directly to the goal, so that counts (+1).
		m_iRouteLength = iNumToCopy + iRouteWriteStart + 1;
	}else{
		// No extra node.
		m_iRouteLength = iNumToCopy + iRouteWriteStart;
	}

	return TRUE;
}//FGetNodeRoute_Final



// Do I like this noderoute, likely from GetShortestPath?
// See if I can reach the start node from where I am now and/or the destination point from where
// the dest (last) node in the route would put me.
BOOL CBaseMonster::FVerifyRoute(Vector vecStart, Vector vecDest, int iPath[], int iPathSize){
	
	if(iPathSize <= 0){
		// no route size?  Can't work with this.
		// Granted a path size of 1 should be impossible, even on a route from one node to that same node
		// (copied twice; speical case checked fore below)
		return FALSE;
	}

	// Safe assumptions?  Why would a route ever start/end with any nodes but the
	// ones it was told to find a route between?
	int iSrcNode = iPath[0];
	int iDestNode = iPath[iPathSize-1];

	
	//MODDD - NEW SECTION.
	// Disabled, see notes above.   Don't know which of these sections is the greater cause yet.
	// Added back in.  Just don't allow this if the distance to the ent is too great, too expensive to check.
	// Biggest culprit of slowdowns seems to be CheckLocalMove calls over huge distances.
	// No wonder other pathfinding in CBaseMonster only checks for up to X units ahead being clear with CheckLocalMove.
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	if(iPathSize==2 && iPath[0] == iPath[1]){
		//MODDD TODO - 
		// That is, if the # of nodes returned is 2 and they are exactly equal, it means we returned our own position.
		// ---Should we set some flag to do something about this...? Ranged AI can just try another nearby node also close to the enemy
		// to see if that is a point with a clear shot?

		CBaseEntity* goalEnt = GetGoalEntity();

		CNode& destNode = WorldGraph.Node(iDestNode);  // same as iSrcNode here
		Vector currentNodeLoc = destNode.m_vecOrigin;

		// !!! But it is worth checking to see that the goal can be reached from this node, however.  If the goal is on
		// top of some huge cliff, a nearest node 2 feet away won't be very helpful anyway.
		// Also, just use the supplied 'vecDest' dangit, who knows if m_vecMoveGoal is even the best choice given the different
		// types of movegoals (entity, target, location, node, etc.?).
		// And should 'vecDest' have  + pev->view_ofs added?  If the movegoaltype is a location, probably.
		// Check for that later, maybe?
		float distFromMeToSource = Distance(vecStart, currentNodeLoc);
		float distFromNodeToGoal = Distance(currentNodeLoc, vecDest);


		if(distFromMeToSource > 350){
			// If we're too far from the start, go ahead and assume it's ok to reach.  A line-trace was already needed
			// to figure out that this monster has a straight-line path to the node anyway.
		}else{
			// Closer?  Go ahead, CheckLocalMove
			BOOL testPass = (CheckLocalMove(vecStart, currentNodeLoc, goalEnt, TRUE, NULL) == LOCALMOVE_VALID);
			if (!testPass){
				return FALSE;
			}
		}

		// Still good?
		if(distFromNodeToGoal > 350){
			// If we're too far from the end, go ahead and assume it's ok to reach too.
			// Could always check from 350 units away from the goal to the goal to see if the 
			// last stretch is valid too, or even only that.
		}else{
			// Closer?  Go ahead, CheckLocalMove
			BOOL testPass = (CheckLocalMove(currentNodeLoc, vecDest, goalEnt, TRUE, NULL) == LOCALMOVE_VALID);
			if(!testPass){
				//DebugLine_Setup(0, vecStart + Vector(0,0,5), m_vecMoveGoal + Vector(0, 0, 5), 0, 0, 255);
				return FALSE;
			}
		}

	}else if(iPathSize >= 2){
		// Any other route?  Just check to see that the dest-node can reach vecDest, that is important.

		CNode& destNode = WorldGraph.Node(iDestNode);
		Vector destNodeLoc = destNode.m_vecOrigin;
		float distFromNodeToGoal = Distance(destNodeLoc, vecDest);

		CBaseEntity* goalEnt = GetGoalEntity();


		// DEBUG:  Can I reach the start node? Should be safe to assume but convenient to get here too
		//////////////////////////////////////////////////////////////
		Vector srcLoc = WorldGraph.Node(iSrcNode).m_vecOrigin;
		g_CheckLocalMove_ExtraDebug = TRUE;
		float distFromMeToSource = Distance(vecStart, srcLoc);
		
		if(distFromMeToSource > 350){
			// If we're too far from the start, go ahead and assume it's ok to reach.  A line-trace was already needed
			// to figure out that this monster has a straight-line path to the node anyway.
		}else{
			// Closer?  Go ahead, CheckLocalMove
			BOOL testPass = (CheckLocalMove(vecStart, srcLoc, goalEnt, TRUE, NULL) == LOCALMOVE_VALID);
			if(!testPass){
				//DebugLine_Setup(0, vecStart + Vector(0,0,5), srcLoc + Vector(0, 0, 5), 0, 0, 255);
				g_CheckLocalMove_ExtraDebug = FALSE;
				return FALSE;
			}
		}


		g_CheckLocalMove_ExtraDebug = FALSE;
		//////////////////////////////////////////////////////////////


		if(distFromNodeToGoal > 350){
			// If we're too far from the end, go ahead and assume it's ok to reach too.
			// Could always check from 350 units away from the goal to the goal to see if the 
			// last stretch is valid too, or even only that.
		}else{
			// Closer?  Go ahead, CheckLocalMove.
			//if(!(pev->flags & FL_MONSTERCLIP)){
				BOOL testPass = (CheckLocalMove(destNodeLoc, vecDest, goalEnt, TRUE, NULL) == LOCALMOVE_VALID);
				if(!testPass){
					//DebugLine_Setup(0, vecStart + Vector(0,0,5), m_vecMoveGoal + Vector(0, 0, 5), 0, 0, 255);
					return FALSE;
				}
			//}
		}
	}// same-src-dest-node check

	// Made it through?  Pass
	return TRUE;
}//FVerifyRoute





//=========================================================
// FindHintNode
//=========================================================
int CBaseMonster::FindHintNode ( void )
{
	int i;
	TraceResult tr;

	if ( !WorldGraph.m_fGraphPresent )
	{
		ALERT ( at_aiconsole, "find_hintnode: graph not ready!\n" );
		return NO_NODE;
	}

	if ( WorldGraph.m_iLastActiveIdleSearch >= WorldGraph.m_cNodes )
	{
		WorldGraph.m_iLastActiveIdleSearch = 0;
	}

	for ( i = 0; i < WorldGraph.m_cNodes ; i++ )
	{
		int nodeNumber = (i + WorldGraph.m_iLastActiveIdleSearch) % WorldGraph.m_cNodes;
		CNode &node = WorldGraph.Node( nodeNumber );

		if ( node.m_sHintType )
		{
			// this node has a hint. Take it if it is visible, the monster likes it, and the monster has an animation to match the hint's activity.
			if ( FValidateHintType ( node.m_sHintType ) )
			{
				if ( !node.m_sHintActivity || LookupActivity ( node.m_sHintActivity ) != ACTIVITY_NOT_AVAILABLE )
				{
					UTIL_TraceLine ( pev->origin + pev->view_ofs, node.m_vecOrigin + pev->view_ofs, ignore_monsters, ENT(pev), &tr );

					if ( tr.flFraction == 1.0 )
					{
						WorldGraph.m_iLastActiveIdleSearch = nodeNumber + 1; // next monster that searches for hint nodes will start where we left off.
						return nodeNumber;// take it!
					}
				}
			}
		}
	}

	WorldGraph.m_iLastActiveIdleSearch = 0;// start at the top of the list for the next search.

	return NO_NODE;
}
			


void CBaseMonster::reportNetName(void){
	const char* netnameBetter;
	if(FStringNull(pev->netname) || ((netnameBetter = STRING(pev->netname)) == NULL)  ){
		easyForcePrintLine("%s:%d MY NETNAME IS %s", this->getClassname(), this->monsterID, "__NULL__");
	}else{
		easyForcePrintLine("%s:%d MY NETNAME IS %s", this->getClassname(), this->monsterID, netnameBetter);
	}
}


char* getActivityName(Activity arg_act){
	int i = 0;
	int actAsNumber = (int)arg_act;

	while ( activity_map[i].type != 0 )
	{
		if ( activity_map[i].type == actAsNumber )
		{
			//ALERT( level, "Activity %s, ", activity_map[i].name );
			return activity_map[i].name;
			break;
		}
		i++;
	}
	return "NONE?";
}//END OF getActivityName




void CBaseMonster::ReportAIState( void )
{
	float currentYaw = UTIL_AngleMod( pev->angles.y );

	//this->pev->movetype = MOVETYPE_TOSS;
	//ClearBits( pev->flags, FL_ONGROUND );

	//m_pfnThink m_pfnTouch
	ALERT_TYPE level = at_console;

	

	easyForcePrintLine("%s ID:%d", getClassname(), monsterID);

	int i = 0;
	

	easyForcePrintLine("ACTS: Current:%s  Ideal:%s  Movement:%s", getActivityName(m_Activity), getActivityName(m_IdealActivity), getActivityName(m_movementActivity));
		
	easyForcePrintLine("STATES: Current: %s  Ideal: %s", pStateNames[this->m_MonsterState], pStateNames[this->m_IdealMonsterState]);

	easyForcePrintLine("Can attack? m1::%d m2:%d r1:%d r2:%d", HasConditions(bits_COND_CAN_MELEE_ATTACK1), HasConditions(bits_COND_CAN_MELEE_ATTACK2), HasConditions(bits_COND_CAN_RANGE_ATTACK1), HasConditions(bits_COND_CAN_RANGE_ATTACK2) );
	easyForcePrintLine("Could attack? m1::%d m2:%d r1:%d r2:%d", HasConditionsMod(bits_COND_COULD_MELEE_ATTACK1), HasConditionsMod(bits_COND_COULD_MELEE_ATTACK2), HasConditionsMod(bits_COND_COULD_RANGE_ATTACK1), HasConditionsMod(bits_COND_COULD_RANGE_ATTACK2) );



	if ( m_pSchedule )
	{
		easyForcePrint("Schedule: %s || FailSchedType:%d\n", getScheduleName(), m_failSchedule);
		Task_t *pTask = GetTask();
		if ( pTask ){
			easyForcePrint("Task#: %d (schedindex:%d) ", pTask->iTask, m_iScheduleIndex );
		}else{
			easyForcePrint("Task#: NULL?");
		}

	}
	else{
		easyForcePrint("Schedule: NULL?!");
	}
	easyForcePrint("\n");


	if ( IsMoving() )
	{
		easyForcePrint("Moving: Yes." );
		if ( m_flMoveWaitFinished > gpGlobals->time ){
			easyForcePrint(" !!! Stopped for %.2f. ", m_flMoveWaitFinished - gpGlobals->time );
		}else if ( m_IdealActivity == GetStoppedActivity() ){
			easyForcePrint(" !!! In stopped anim. " );
		}
	}else{
		easyForcePrint("Moving: No.");
	}
	easyForcePrint("\n");

	
	
	CSquadMonster *pSquadMonster = MySquadMonsterPointer();

	if ( pSquadMonster )
	{
		easyForcePrint("SQUAD:");
		if ( !pSquadMonster->InSquad() )
		{
			easyForcePrint("not " );
		}

		easyForcePrint("In Squad, " );

		if ( !pSquadMonster->IsLeader() )
		{
			easyForcePrint("not " );
		}

		easyForcePrint("Leader." );
		easyForcePrint("\n");
	}

	char enemyInfoString[127];
	enemyInfoString[0] = '\0';
	char targetInfoString[127];
	targetInfoString[0] = '\0';

	CBaseMonster* tempMonsterPointer;

	tempMonsterPointer = NULL;
	if(m_hTargetEnt != NULL ){
		if((tempMonsterPointer = m_hTargetEnt->MyMonsterPointer()) != NULL){
			sprintf(targetInfoString, "%s:%d", m_hTargetEnt->getClassname(), tempMonsterPointer->monsterID);
		}else{
			sprintf(targetInfoString, "%s:%s", m_hTargetEnt->getClassname(), "NOT A MONSTER");
		}
	}else{
		sprintf(targetInfoString, "%s", "EMPTY");
	}

	tempMonsterPointer = NULL;
	if(m_hEnemy != NULL ){
		if((tempMonsterPointer = m_hEnemy->MyMonsterPointer()) != NULL){
			sprintf(enemyInfoString, "%s:%d", m_hEnemy->getClassname(), tempMonsterPointer->monsterID);
		}else{
			sprintf(enemyInfoString, "%s:%s", m_hEnemy->getClassname(), "NOT A MONSTER");
		}
	}else{
		sprintf(enemyInfoString, "%s", "EMPTY");
	}
	easyForcePrintLine("Enemy: %s   TargetEnt: %s", enemyInfoString, targetInfoString);
	

	/*
	//MODDD TODO - No... make this its own printout callable by the client (console) later.
	easyForcePrint("Old Enemey Queue: ");

	if(m_intOldEnemyNextIndex > 0){
	
		for(int i = 0; i < m_intOldEnemyNextIndex; i++){
			
			//IsAlive_FromAI takes "this" monster as a parameter. It already knows what itself is.
			if (m_hOldEnemy[i] != NULL && m_hOldEnemy[i]->IsAlive_FromAI(this))
			{
				//this is okay.
				if(i!=0)easyForcePrint(", ");
				easyForcePrint("#%d:%s:ID%d", i, m_hOldEnemy[i]->getClassnameShort(), m_hOldEnemy[i]->MyMonsterPointer()->monsterID);
			}
			else {
				//null or dead. Say so.
				if(i!=0)easyForcePrint(", ");
				easyForcePrint("#%d:%s", i, "EMPTY");	
			}
		}//END OF for(...)
		
		easyForcePrintLine("");
	}else{
		
		easyForcePrintLine("EMPTY");
	}
	*/
	
	easyForcePrintLine("Health: %.2f / %.2f", pev->health, pev->max_health );
	easyForcePrintLine("Sequence ID:%d Frame:%.2f Framerate:%.2f FramerateSugg:%.2f spawnflag:%d deadflag:%d loops:%d sequencefinished:%d ThinkACTIVE:%d nextThink:%.2f currentTime:%.2f targetname:%s target:%s globalname:%s", pev->sequence, pev->frame, pev->framerate, m_flFramerateSuggestion, pev->spawnflags, pev->deadflag, this->m_fSequenceLoops, this->m_fSequenceFinished, (m_pfnThink!=NULL), pev->nextthink, gpGlobals->time,  STRING(pev->targetname), STRING(pev->target), STRING(pev->globalname ) );
	easyForcePrintLine("Yaw:%.2f Ideal:%.2f yawspd:%.2f", currentYaw, pev->ideal_yaw, pev->yaw_speed);


	if(m_pCine == NULL){
		easyForcePrintLine("CINE: none");
	}else{
		easyForcePrintLine("CINE: %s:%d  sf:%d ob:%d targetname:%s target:%s iszE:%s globalname: %s", m_pCine->getClassname(), m_pCine->monsterID, m_pCine->pev->spawnflags, m_pCine->ObjectCaps(),  STRING(m_pCine->pev->targetname), STRING(m_pCine->pev->target),  STRING( m_pCine->m_iszEntity ), STRING(m_pCine->pev->globalname ) );
		
		// spread out to debug a little crash issue.
		// Great were' in 2020 and a program can't give any more info about a long line crashing other than 'crash happen here'.   >_>
		/*
		easyForcePrintLine("ok its gonnna go a lil somethin like this yall");

		easyForcePrintLine("CINE: %s:%d", m_pCine->getClassname(), m_pCine->monsterID);
		easyForcePrintLine("CINE: sf:%d", m_pCine->pev->spawnflags);

		easyForcePrintLine("CINE: ob:%d", m_pCine->ObjectCaps());
		easyForcePrintLine("CINE: targetname:%s", STRING(m_pCine->pev->targetname));
		easyForcePrintLine("CINE: target:%s", STRING(m_pCine->pev->target));
		easyForcePrintLine("CINE: iszE:%s", STRING(m_pCine->m_iszEntity));
		easyForcePrintLine("CINE: globalname: %s", STRING(m_pCine->pev->globalname));
		*/
	}
	

	easyForcePrint("Capability: ");
	printLineIntAsBinary((unsigned int)m_afCapability, 32u);


	//

	//easyForcePrintLine("isOrganic:%d", isOrganic());

	easyForcePrintLine("EXTRA: flags:%d effects:%d renderfx:%d rendermode:%d renderamt:%.2f gamestate:%d solid:%d waterlevel:%d gravity:%.2f movetype:%d", pev->flags, pev->effects, pev->renderfx, pev->rendermode, pev->renderamt, pev->gamestate, pev->solid, pev->waterlevel, pev->gravity, pev->movetype);
	easyForcePrintLine("Eyepos: (%.2f,%.2f,%.2f)", pev->view_ofs.x, pev->view_ofs.y, pev->view_ofs.z);
	easyForcePrint("BOUNDS: mins: (%.2f,%.2f,%.2f)", pev->mins.x, pev->mins.y, pev->mins.z);
	easyForcePrintLine("maxs: (%.2f,%.2f,%.2f)", pev->maxs.x, pev->maxs.y, pev->maxs.z);


	easyForcePrintLine("-------------------------------------------------------------");

}

//=========================================================
// KeyValue
//
// !!! netname entvar field is used in squadmonster for groupname!!!
//=========================================================
void CBaseMonster::KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "TriggerTarget"))
	{
		m_iszTriggerTarget = ALLOC_STRING( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "TriggerCondition") )
	{
		m_iTriggerCondition = atoi( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else
	{
		CBaseToggle::KeyValue( pkvd );
	}
}

//=========================================================
// FCheckAITrigger - checks the monster's AI Trigger Conditions,
// if there is a condition, then checks to see if condition is 
// met. If yes, the monster's TriggerTarget is fired.
//
// Returns TRUE if the target is fired.
//=========================================================
BOOL CBaseMonster::FCheckAITrigger ( void )
{
	BOOL fFireTarget;

	if ( m_iTriggerCondition == AITRIGGER_NONE )
	{
		// no conditions, so this trigger is never fired.
		return FALSE; 
	}

	fFireTarget = FALSE;

	switch ( m_iTriggerCondition )
	{
	case AITRIGGER_SEEPLAYER_ANGRY_AT_PLAYER:
		if ( m_hEnemy != NULL && m_hEnemy->IsPlayer() && HasConditions ( bits_COND_SEE_ENEMY ) )
		{
			fFireTarget = TRUE;
		}
		break;
	case AITRIGGER_SEEPLAYER_UNCONDITIONAL:
		if ( HasConditions ( bits_COND_SEE_CLIENT ) )
		{
			fFireTarget = TRUE;
		}
		break;
	case AITRIGGER_SEEPLAYER_NOT_IN_COMBAT:
		if ( HasConditions ( bits_COND_SEE_CLIENT ) && 
			 m_MonsterState != MONSTERSTATE_COMBAT	&& 
			 m_MonsterState != MONSTERSTATE_PRONE	&& 
			 m_MonsterState != MONSTERSTATE_SCRIPT)
		{
			fFireTarget = TRUE;
		}
		break;
	case AITRIGGER_TAKEDAMAGE:
		if ( HasConditions( bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE ) )
		{
			fFireTarget = TRUE;
		}
		break;
	case AITRIGGER_DEATH:
		if ( pev->deadflag != DEAD_NO )
		{
			fFireTarget = TRUE;
		}
		break;
	case AITRIGGER_HALFHEALTH:
		if ( IsAlive() && pev->health <= ( pev->max_health / 2 ) )
		{
			fFireTarget = TRUE;
		}
		break;
/*

  // !!!UNDONE - no persistant game state that allows us to track these two. 

	case AITRIGGER_SQUADMEMBERDIE:
		break;
	case AITRIGGER_SQUADLEADERDIE:
		break;
*/

	//MODDD - I really doubt that involving 'bits_SOUND_X' in HasConditions is a good idea, but maybe there's something
	// more to this that I don't get.  Just looks like that conflicts with bitflags that mean very different things to HasConditions.
	case AITRIGGER_HEARWORLD:
		if ( HasAllConditions(bits_COND_HEAR_SOUND | bits_SOUND_WORLD) )
		{
			fFireTarget = TRUE;
		}
		break;
	case AITRIGGER_HEARPLAYER:
		if ( HasAllConditions(bits_COND_HEAR_SOUND | bits_SOUND_PLAYER) )
		{
			fFireTarget = TRUE;
		}
		break;
	case AITRIGGER_HEARCOMBAT:
		if ( HasAllConditions(bits_COND_HEAR_SOUND | bits_SOUND_COMBAT) )
		{
			fFireTarget = TRUE;
		}
		break;
	}

	if ( fFireTarget )
	{
		// fire the target, then set the trigger conditions to NONE so we don't fire again
		ALERT ( at_aiconsole, "AI Trigger Fire Target\n" );
		FireTargets( STRING( m_iszTriggerTarget ), this, this, USE_TOGGLE, 0 );
		m_iTriggerCondition = AITRIGGER_NONE;
		return TRUE;
	}

	return FALSE;
}

//=========================================================	
// CanPlaySequence - determines whether or not the monster
// can play the scripted sequence or AI sequence that is 
// trying to possess it. If DisregardState is set, the monster
// will be sucked into the script no matter what state it is
// in. ONLY Scripted AI ents should allow this.
//=========================================================	
int CBaseMonster::CanPlaySentence(BOOL fDisregardState)
{
	return IsAlive();
}
int CBaseMonster::CanPlaySequence( BOOL fDisregardMonsterState, int interruptLevel )
{
	//MODDD - it is possible for "EASY_CVAR_GET_DEBUGONLY(cineAllowSequenceOverwrite)" to block "m_pCine" already being occupied to stop this from having influence.
	if ( (m_pCine && EASY_CVAR_GET_DEBUGONLY(cineAllowSequenceOverwrite) != 1) || !IsAlive() || m_MonsterState == MONSTERSTATE_PRONE )
	{
		// monster is already running a scripted sequence or dead!
		return FALSE;
	}

	if (monsterID == 70) {
		int x = 45;
	}
	
	if ( fDisregardMonsterState )
	{
		// ok to go, no matter what the monster state. (scripted AI)
		return TRUE;
	}

	if ( m_MonsterState == MONSTERSTATE_NONE || m_MonsterState == MONSTERSTATE_IDLE || m_IdealMonsterState == MONSTERSTATE_IDLE )
	{
		// ok to go, but only in these states
		return TRUE;
	}


	//MODDD - new block. force allow  SS_INTERRUPT_AI.  What else was the point of specifying that.
	if (interruptLevel >= SS_INTERRUPT_AI) {
		return TRUE;
	}
	//////////////////////////////////////////////////////////////////////////////

	
	//MODDD - and why could only ALERT be interrrupted for SS_INTERRUPT_BY_NAME?  Why not at least IDLE too?
	//if ( m_MonsterState == MONSTERSTATE_ALERT && interruptLevel >= SS_INTERRUPT_BY_NAME )
	if ( (m_MonsterState == MONSTERSTATE_ALERT || m_MonsterState == MONSTERSTATE_IDLE) && interruptLevel >= SS_INTERRUPT_BY_NAME)

		return TRUE;

	// unknown situation
	return FALSE;
}


//=========================================================
// FindLateralCover - attempts to locate a spot in the world
// directly to the left or right of the caller that will
// conceal them from view of pSightEnt
//=========================================================
#define COVER_CHECKS	5// how many checks are made
#define COVER_DELTA		48// distance between checks

BOOL CBaseMonster::FindLateralCover ( const Vector &vecThreat, const Vector &vecViewOffset )
{
	TraceResult	tr;
	Vector	vecBestOnLeft;
	Vector	vecBestOnRight;
	Vector	vecLeftTest;
	Vector	vecRightTest;
	Vector	vecStepRight;
	int	i;

	UTIL_MakeVectors ( pev->angles );
	vecStepRight = gpGlobals->v_right * COVER_DELTA;
	vecStepRight.z = 0; 
	
	vecLeftTest = vecRightTest = pev->origin;

	for ( i = 0 ; i < COVER_CHECKS ; i++ )
	{
		vecLeftTest = vecLeftTest - vecStepRight;
		vecRightTest = vecRightTest + vecStepRight;

		// it's faster to check the SightEnt's visibility to the potential spot than to check the local move, so we do that first.
		UTIL_TraceLine( vecThreat + vecViewOffset, vecLeftTest + pev->view_ofs, ignore_monsters, ignore_glass, ENT(pev)/*pentIgnore*/, &tr);
		
		if (tr.flFraction != 1.0)
		{
			if ( FValidateCover ( vecLeftTest ) && CheckLocalMove( pev->origin, vecLeftTest, NULL, FALSE, NULL ) == LOCALMOVE_VALID )
			{
				if ( MoveToLocation( ACT_RUN, 0, vecLeftTest ) )
				{
					return TRUE;
				}
			}
		}
		
		// it's faster to check the SightEnt's visibility to the potential spot than to check the local move, so we do that first.
		UTIL_TraceLine(vecThreat + vecViewOffset, vecRightTest + pev->view_ofs, ignore_monsters, ignore_glass, ENT(pev)/*pentIgnore*/, &tr);
		
		if ( tr.flFraction != 1.0 )
		{
			if (  FValidateCover ( vecRightTest ) && CheckLocalMove( pev->origin, vecRightTest, NULL, FALSE, NULL ) == LOCALMOVE_VALID )
			{
				if ( MoveToLocation( ACT_RUN, 0, vecRightTest ) )
				{
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}


Vector CBaseMonster::ShootAtEnemy( const Vector &shootOrigin )
{
	CBaseEntity *pEnemy = m_hEnemy;

	if ( pEnemy )
	{
		//MODDD NOTE ........ what is this formula?
		//    I'm guessing that the BodyTarget includes the enemy's pev->origin, but we subtract it out so we can substitute it with m_vecEnemyLKP.
		//    So why not make a separate BodyTarget method that never added pev->origin in the first place? Who knows.
		return ( (pEnemy->BodyTarget( shootOrigin ) - pEnemy->pev->origin) + m_vecEnemyLKP - shootOrigin ).Normalize();
	}
	else
		return gpGlobals->v_forward;
	//MODDD NOTICE - isn't trusting "gpGlobals->v_forward" kinda dangerous? This assumes we recently called MakeVectors and not privately for v_forward to be relevant
	//               to this monster.
}

// yowch
Vector CBaseMonster::ShootAtEnemyEyes( const Vector &shootOrigin )
{
	CBaseEntity *pEnemy = m_hEnemy;

	if ( pEnemy )
	{
		return ( (pEnemy->EyePosition() - pEnemy->pev->origin) + m_vecEnemyLKP - shootOrigin ).Normalize();
	}
	else
		return gpGlobals->v_forward;
	//MODDD NOTICE - isn't trusting "gpGlobals->v_forward" kinda dangerous? This assumes we recently called MakeVectors and not privately for v_forward to be relevant
	//               to this monster.
}





//Similar to above, but uses BodyTargetMod instead of BodyTarget. BodyTargetMod returns the true center of the player so that things that use this result to determine
//aim pitch don't appear to jitter around up and down (with changes in the slightly randomly returned BodyCenter) just to get an aim pitch.
Vector CBaseMonster::ShootAtEnemyMod( const Vector &shootOrigin )
{
	CBaseEntity *pEnemy = m_hEnemy;

	if ( pEnemy )
	{
		//MODDD NOTE ........ what is this formula?
		//    I'm guessing that the BodyTarget includes the enemy's pev->origin, but we subtract it out so we can substitute it with m_vecEnemyLKP.
		//    So why not make a separate BodyTarget method that never added pev->origin in the first place? Who knows.
		return ( (pEnemy->BodyTargetMod( shootOrigin ) - pEnemy->pev->origin) + m_vecEnemyLKP - shootOrigin ).Normalize();
	}
	else {
		UTIL_MakeAimVectors(pev->angles);
		return gpGlobals->v_forward;
	}
	//MODDD NOTICE - isn't trusting "gpGlobals->v_forward" kinda dangerous? This assumes we recently called MakeVectors and not privately for v_forward to be relevant
	//               to this monster.
	//  CHANGED,  just call it then in such a case for the love of.
}


//=========================================================
// FacingIdeal - tells us if a monster is facing its ideal
// yaw. Created this function because many spots in the 
// code were checking the yawdiff against this magic
// number. Nicer to have it in one place if we're gonna
// be stuck with it.
//=========================================================
BOOL CBaseMonster::FacingIdeal( void )
{
	
	/*
	float flCurrentYaw = UTIL_AngleMod( pev->angles.y );

	//if ( flCurrentYaw == pev->ideal_yaw )
	easyForcePrintLine("monsterID:%d WHAT?? diff:%.2f cur:%.2f ideal:%.2f", monsterID, FlYawDiff(), flCurrentYaw, pev->ideal_yaw);
	*/


	if ( fabs( FlYawDiff() ) <= 0.006 )//!!!BUGBUG - no magic numbers!!!
	{
		return TRUE;
	}

	return FALSE;
}

BOOL CBaseMonster::FacingIdeal( float argDegreeTolerance )
{
	if ( fabs( FlYawDiff() ) <= argDegreeTolerance )//Finally, that not-so-magic number.
	{
		return TRUE;
	}

	return FALSE;
}



//=========================================================
// FCanActiveIdle
//=========================================================
BOOL CBaseMonster::FCanActiveIdle ( void )
{
	/*
	if ( m_MonsterState == MONSTERSTATE_IDLE && m_IdealMonsterState == MONSTERSTATE_IDLE && !IsMoving() )
	{
		return TRUE;
	}
	*/
	return FALSE;
}


void CBaseMonster::PlaySentence( const char *pszSentence, float duration, float volume, float attenuation )
{
	if ( pszSentence && IsAlive() )
	{
		if ( pszSentence[0] == '!' )
			EMIT_SOUND_DYN( edict(), CHAN_VOICE, pszSentence, volume, attenuation, 0, PITCH_NORM );
		else
			SENTENCEG_PlayRndSz( edict(), pszSentence, volume, attenuation, 0, PITCH_NORM );
	}
}


//MODDD - bConcurrent and pListener are ignored by default, but other implementations (talkmonster and gman) do something with them.
void CBaseMonster::PlayScriptedSentence( const char *pszSentence, float duration, float volume, float attenuation, BOOL bConcurrent, CBaseEntity *pListener )
{ 
	PlaySentence( pszSentence, duration, volume, attenuation );
}


void CBaseMonster::SentenceStop( void )
{
	UTIL_PlaySound( edict(), CHAN_VOICE, "common/null.wav", 1.0, ATTN_IDLE, 0, 100, FALSE );
}


void CBaseMonster::CorpseFallThink( void )
{
	if ( pev->flags & FL_ONGROUND )
	{
		SetThink ( NULL );

		SetSequenceBox( );
		UTIL_SetOrigin( pev, pev->origin );// link into world.
	}
	else
		pev->nextthink = gpGlobals->time + 0.1;
}

// Call after animation/pose is set up
void CBaseMonster::MonsterInitDead( void )
{
	InitBoneControllers();

	pev->solid			= SOLID_BBOX;
	pev->movetype		= MOVETYPE_TOSS;// so he'll fall to ground

	//MDODD - always play death-anim at least forward (and not too slow)?
	if(m_flFramerateSuggestion <= 0.2){
		m_flFramerateSuggestion = 0.2;
	}

	resetFrame();
	ResetSequenceInfo( );
	pev->framerate = 0;
	
	// Copy health
	pev->max_health		= pev->health;
	pev->deadflag		= DEAD_DEAD;
	
	UTIL_SetSize(pev, g_vecZero, g_vecZero );
	UTIL_SetOrigin( pev, pev->origin );


	//MODDD - flag for mirror recognition.
	pev->renderfx |= ISNPC;

	// Setup health counters, etc.
	BecomeDead();
	SetThink( &CBaseMonster::CorpseFallThink );
	pev->nextthink = gpGlobals->time + 0.5;
}

//MODDD - more of a placeholder, see "NoFriendlyFireImp" for more complexity.
// As to how this was never at least minimally stubbed as-is, no clue.
BOOL CBaseMonster::NoFriendlyFire(void) {
	// Usually for squadmonsters.  Otherwise assume we can fire.
	return TRUE;
}



//=========================================================
// BBoxIsFlat - check to see if the monster's bounding box
// is lying flat on a surface (traces from all four corners
// are same length.)
//=========================================================
BOOL CBaseMonster::BBoxFlat ( void )
{
	TraceResult	tr;
	Vector		vecPoint;
	float	flXSize, flYSize;
	float	flLength;
	float	flLength2;

	flXSize = pev->size.x / 2;
	flYSize = pev->size.y / 2;

	vecPoint.x = pev->origin.x + flXSize;
	vecPoint.y = pev->origin.y + flYSize;
	vecPoint.z = pev->origin.z;

	UTIL_TraceLine ( vecPoint, vecPoint - Vector ( 0, 0, 100 ), ignore_monsters, ENT(pev), &tr );
	flLength = (vecPoint - tr.vecEndPos).Length();

	vecPoint.x = pev->origin.x - flXSize;
	vecPoint.y = pev->origin.y - flYSize;

	UTIL_TraceLine ( vecPoint, vecPoint - Vector ( 0, 0, 100 ), ignore_monsters, ENT(pev), &tr );
	flLength2 = (vecPoint - tr.vecEndPos).Length();
	if ( flLength2 > flLength )
	{
		return FALSE;
	}
	flLength = flLength2;

	vecPoint.x = pev->origin.x - flXSize;
	vecPoint.y = pev->origin.y + flYSize;
	UTIL_TraceLine ( vecPoint, vecPoint - Vector ( 0, 0, 100 ), ignore_monsters, ENT(pev), &tr );
	flLength2 = (vecPoint - tr.vecEndPos).Length();
	if ( flLength2 > flLength )
	{
		return FALSE;
	}
	flLength = flLength2;

	vecPoint.x = pev->origin.x + flXSize;
	vecPoint.y = pev->origin.y - flYSize;
	UTIL_TraceLine ( vecPoint, vecPoint - Vector ( 0, 0, 100 ), ignore_monsters, ENT(pev), &tr );
	flLength2 = (vecPoint - tr.vecEndPos).Length();
	if ( flLength2 > flLength )
	{
		return FALSE;
	}
	flLength = flLength2;

	return TRUE;
}


//MODDD - new version.  Can accept whether this should ignore the _SEE conditions and schedule interruptable by NEW_ENEMY checks.
//        RUNTASK logic wanting to search for new enemies in the middle of a task shouldn't be ignored simply because those requirements aren't met.
BOOL CBaseMonster::GetEnemy(void){
	return GetEnemy(FALSE);
}

//=========================================================
// Get Enemy - tries to find the best suitable enemy for the monster.
//=========================================================
BOOL CBaseMonster::GetEnemy (BOOL arg_forceWork )
{
	CBaseEntity *pNewEnemy;


	if(EASY_CVAR_GET_DEBUGONLY(peaceOut) == 1){
		// no enemy detection.
		return FALSE;
	}


	//MODDD - if dead, can't do this.
	if(!UTIL_IsAliveEntity(this)){
		return FALSE;// monster has no enemy
	}

	//MODDD - major. Adding FEAR as a condition here now.

	//if ( HasConditions(bits_COND_SEE_HATE | bits_COND_SEE_DISLIKE | bits_COND_SEE_NEMESIS) )
	if (arg_forceWork || HasConditions(bits_COND_SEE_FEAR | bits_COND_SEE_HATE | bits_COND_SEE_DISLIKE | bits_COND_SEE_NEMESIS) )
	{
		pNewEnemy = BestVisibleEnemy();



		if ( pNewEnemy != m_hEnemy && pNewEnemy != NULL)
		{
			// DO NOT mess with the monster's m_hEnemy pointer unless the schedule the monster is currently running will be interrupted
			// by COND_NEW_ENEMY. This will eliminate the problem of monsters getting a new enemy while they are in a schedule that doesn't care,
			// and then not realizing it by the time they get to a schedule that does. I don't feel this is a good permanent fix. 

			if ( m_pSchedule ){
				//easyPrintLine("WHAT THE schedname %s", m_pSchedule->pName);
				//easyPrintLine("OH NOI %d", m_pSchedule->iInterruptMask & bits_COND_NEW_ENEMY );

				
				//MODDD - sometimes, such as when the chumtoad is playing dead, we need to be able to pick up on new enemies (even in case the original one gets forgotten).
				//        We need a schedule that is not interruptible by bits_COND_NEW_ENEMY, and yet can still take new enemies during its run.
				//        Override "getForceAllowNewEnemy" to work for the playdead schedule.

				//MODDD - POINT OF FRUSTRATION HERE
				//if ( m_pSchedule->iInterruptMask & bits_COND_NEW_ENEMY )



				//MODDD
				// why not set this anyway, interrupt the schedule or not?  No effect on the schedule
				// if it's not part of its interrupt mask.
				// Other schedules being picked in the same frame might recognize this, although it does
				// get cleared with any ChangeSchedule call anyway
				SetConditions(bits_COND_NEW_ENEMY);

				if ( (m_pSchedule->iInterruptMask & bits_COND_NEW_ENEMY) || arg_forceWork || getForceAllowNewEnemy(pNewEnemy) )
				//if ( TRUE || arg_forceWork || getForceAllowNewEnemy(pNewEnemy) )
				{
					// DEBUG STUFF
					/*
					CBaseEntity* enemyRef = (m_hEnemy !=NULL)?m_hEnemy.GetEntity():NULL;
					CBaseMonster* monRef = NULL;
					if(enemyRef !=NULL) monRef = enemyRef->GetMonsterPointer();
					CBaseMonster* newMonRef = pNewEnemy->GetMonsterPointer();
					int oldMonsterID = -1;
					int newMonsterID = -1;

					if (monRef != NULL) {
						oldMonsterID = monRef->monsterID;
					}
					if (newMonRef != NULL) {
						newMonsterID = newMonRef->monsterID;
					}
					*/


					PushEnemy( m_hEnemy, m_vecEnemyLKP );

					//MODDD TODO, idea.  Should there be some kind of cooldown on reacting to a new enemy? It might
					// be unsafe to avoid setting this condition anyway, it's likely what lets things like long-duration ranged
					// attacks change their mind mid-sequence (kinda stupid looking to continue firing at something obscurred while 
					// something closer is tearing into them).

					SetConditions(bits_COND_NEW_ENEMY);
					m_hEnemy = pNewEnemy;
					
					//MODDD - involve the ent
					setEnemyLKP(m_hEnemy);

				}
				// if the new enemy has an owner, take that one as well
				if (pNewEnemy->pev->owner != NULL)
				{
					CBaseEntity *pOwner = GetMonsterPointer( pNewEnemy->pev->owner );
					if ( pOwner && (pOwner->pev->flags & FL_MONSTER) && IRelationship( pOwner ) != R_NO )
						PushEnemy( pOwner, m_vecEnemyLKP );
				}
			}
		}
	}

	// remember old enemies
	if (m_hEnemy == NULL && PopEnemy( ))
	{
		if ( m_pSchedule )
		{
			if ( m_pSchedule->iInterruptMask & bits_COND_NEW_ENEMY )
			{
				SetConditions(bits_COND_NEW_ENEMY);
				//MODDD - hold on. Shouldn't we also rip the old enemy LKP from the stack too?
				//        The PopEnemy() call above automatically does that. It did even change m_hEnemy to the next enemy remembered in line.

			}
		}
	}

	if ( m_hEnemy != NULL )
	{
		// monster has an enemy.
		//MODDD NOTE - returning trues just because m_hEnemy ends up "not null" isn't very informative.
		//Perhaps we should return TRUE only if a new enemy was picked, or at least an attempt was made to pick the bestVisibleEnemy regarldess
		//of it matching our old enemy or not?  Say we just... did something?
		return TRUE;
	}

	return FALSE;// monster has no enemy
}


//=========================================================
// DropItem - dead monster drops named item 
//=========================================================
CBaseEntity* CBaseMonster::DropItem ( char *pszItemName, const Vector &vecPos, const Vector &vecAng )
{
	//MODDD - difficulty can block items coming from NPCs.
	if (gSkillData.npc_drop_weapon == 0) {
		// no dropped weapons for you!
		return NULL;
	}

	if ( !pszItemName )
	{
		ALERT ( at_console, "DropItem() - No item name!\n" );
		return NULL;
	}

	CBaseEntity *pItem = CBaseEntity::Create( pszItemName, vecPos, vecAng, edict() );

	if ( pItem )
	{
		// do we want this behavior to be default?! (sjb)
		pItem->pev->velocity = pev->velocity;
		pItem->pev->avelocity = Vector ( 0, RANDOM_FLOAT( 0, 100 ), 0 );
		return pItem;
	}
	else
	{
		ALERT ( at_console, "DropItem() - Didn't create!\n" );
		return FALSE;
	}
}


BOOL CBaseMonster::ShouldFadeOnDeath( void )
{
	// if flagged to fade out or I have an owner (I came from a monster spawner)
	if ( (pev->spawnflags & SF_MONSTER_FADECORPSE) || !FNullEnt( pev->owner ) )
		return TRUE;

	return FALSE;
}


void CBaseMonster::setPhysicalHitboxForDeath(void){
	//just stuff schedule.cpp used to do at death (and still does, just more modular this way)

	//MODDD NOTE - it seems this is not always quite right, unsure.
	if ( !BBoxFlat() )
	{
		// a bit of a hack. If a corpses' bbox is positioned such that being left solid so that it can be attacked will
		// block the player on a slope or stairs, the corpse is made nonsolid. 
//					pev->solid = SOLID_NOT;
		easyPrintLine("setPhysicalHitboxForDeath: A");
		UTIL_SetSize ( pev, Vector ( -4, -4, 0 ), Vector ( 4, 4, 0.2 ) );
	}
	else
	{
		// !!!HACKHACK - put monster in a thin, wide bounding box until we fix the solid type/bounding volume problem
		//MODDD - thinner height-wise. used to be "1" tall from the mins.z (and above)
		easyPrintLine("setPhysicalHitboxForDeath: B");
		UTIL_SetSize ( pev, Vector ( pev->mins.x, pev->mins.y, pev->mins.z ), Vector ( pev->maxs.x, pev->maxs.y, pev->mins.z + 0.2 ) );
		//UTIL_SetSize ( pev, Vector ( pev->mins.x, pev->mins.y, 0), Vector ( pev->maxs.x, pev->maxs.y, 0 ) );
	}

}


// All ripped from the start of TASK_DIE in schedule.cpp.
// Called anywhere by typical "Killed" calls, skipped if gibbed (no entity/model to work with then).
void CBaseMonster::DeathAnimationStart(){
	RouteClear();

	// TODO SHITTY IDEA.
	// How about if m_Activity is already any of the DIE activities, don't call for the activity change at all.
	// Just force the pev->framerate to -1 and set the deadflag to DEAD_DEAD if necessary (better safe than sorry?).
	// Might need to change the schedule from the revive one to the death one like combat does inadvertently, the
	// one with TASK_DIE or whatever.    And then it might go well / bug-free.
	// See? shitty idea.


	m_IdealActivity = GetDeathActivity();
	
	// good idea?
	m_Activity = ACT_RESET;
	
	pev->deadflag = DEAD_DYING;
	
	// ensure the death activity we pick (or have picked) gets to run.
	signalActivityUpdate = TRUE;

	if (this->usesAdvancedAnimSystem() == FALSE) {
		// SAFE ASSUMPTION:  anytime we're playing a death animation we want to reset the frame/framerate.
		// Just for the bizarre case of being killed while being revived (happens with headcrabs or things
		// without much beyond the bare bones minimal animation-related stuff)
		pev->frame = 0;
		pev->framerate = 1;
		this->m_flFrameRate = 1;
		this->m_flFramerateSuggestion = 1;

		// is this a good idea too?
		animFrameStart = -1;
		animFrameStartSuggestion = -1;
		animFrameCutoff = -1;
		animFrameCutoffSuggestion = -1;
	}


	//easyPrintLine("ARE YOU SOME KIND OF insecure person??? %.2f %d", EASY_CVAR_GET_DEBUGONLY(thoroughHitBoxUpdates), pev->deadflag );
	//MODDD
	if(EASY_CVAR_GET_DEBUGONLY(thoroughHitBoxUpdates) == 1){
		//update the collision box now,
		this->SetObjectCollisionBox();
	}
	if(EASY_CVAR_GET_DEBUGONLY(animationKilledBoundsRemoval) == 1){
		setPhysicalHitboxForDeath();
	}
}//END OF DeathAnimationStart


//MODDD TODO - would it be possible to use "SetSequenceBox" to set the bounds
//             in a way that guarantees the corpse is hittable?
void CBaseMonster::DeathAnimationEnd(){
	
	//do we need to force the frame to 255 in here?
	pev->frame = 255;
	m_fSequenceFinished = TRUE;  //forcing too?
	m_fSequenceFinishedSinceLoop = TRUE;

	pev->deadflag = DEAD_DEAD;
	
	//MODDD - IMPORTANT NOTICE:  "SetThink" delinking MonsterThink has been re-delegated to "onDeathAnimationEnd".  The default case for all monsters does this, but some may override the method to omit it and still "think" on death, usually for planning a revival.
	//SetThink ( NULL );

	StopAnimation();

	//easyPrintLine("DEAD: boxFlat?   %d", (BBoxFlat()));

	if(EASY_CVAR_GET_DEBUGONLY(animationKilledBoundsRemoval) == 2){
		setPhysicalHitboxForDeath();
	}

	//MODDD - bound-altering script for death moved to "setPhyiscalHitboxForDeath" for better through by a CVar.
				
	if ( ShouldFadeOnDeath() )
	{
		// this monster was created by a monstermaker... fade the corpse out.
		SUB_StartFadeOut();
	}
	else
	{
		// body is gonna be around for a while, so have it stink for a bit.
		// German robots still will if they are supposed to behave like normal.
		if(isOrganicLogic() ){
			CSoundEnt::InsertSound ( bits_SOUND_CARCASS, pev->origin, 384, 30 );
		}

	}

	//Something calling us may need to know not to try resetting the activity.
	signalActivityUpdate = FALSE;
	
	//an event.  But ONLY if not fading out.  At least in the default case,
	//it is up to other monsters whether they care about that, like auto-detonators (floaters).
	onDeathAnimationEnd();
}//END OF DeathAnimationEnd

void CBaseMonster::onDeathAnimationEnd(void){
	//...nothing in the broadest case.
	//except that.
	if(!this->ShouldFadeOnDeath()){
		//Don't do it if I'm already fading out.  Let other monsters decide if they want to do something else
		//instead or depend on this at all.
		SetThink ( NULL );
	}
}

//MODDD - new.
void CBaseMonster::Activate( void ){

	CBaseToggle::Activate();

	//const char* nameCheat = STRING(pev->classname);

	/*
	if( monsterID == -1){
		monsterID = monsterIDLatest;
		monsterIDLatest++;
	}
	*/
	
	//setModelCustom();


	//MODDD - hold up. If below does effectively nothing with what's commented out (time of writing, er... typing), why bother?
	// Stop early.
	return;


	if(m_pCine == NULL && !FStringNull(pev->targetname) ){

		edict_t		*pEdicttt;
		CBaseEntity *pEntityyy;
		pEdicttt = g_engfuncs.pfnPEntityOfEntIndex( 1 );
		if ( pEdicttt ){
			for ( int i = 1; i < gpGlobals->maxEntities; i++, pEdicttt++ ){
				if ( pEdicttt->free )	// Not in use
				continue;
		
				pEntityyy = CBaseEntity::Instance(pEdicttt);
				if ( !pEntityyy )
					continue;

				if(pEntityyy->pev == this->pev){
					//same ent, move along..
					continue;
				}

				/*
				//NEVERMIND...  the m_pCine  on startup looking for its entity and verifying its own "wasAttached" as usuall still works just fine for either case (forgetting either the host ent or m_pCine that is).
				CBaseMonster* tempmon = NULL;
				if(  (tempmon = pEntityyy->GetMonsterPointer()) != NULL){

					if(FClassnameIs(pEntityyy->pev,"scripted_sequence") || FClassnameIs(pEntityyy->pev,"aiscripted_sequence")){ //|| FClassnameIs(pEntityyy->pev,"scripted_sentence")   ){
						CCineMonster* cineMon = (CCineMonster*) pEntityyy;
						if( cineMon->wasAttached >= 2 && FStrEq( STRING(cineMon->m_iszEntity), STRING(pev->targetname)) ){
							//YEAS, we have a match.
							m_pCine = cineMon;

							//pTarget->SetState(MONSTERSTATE_SCRIPT);
							m_MonsterState = MONSTERSTATE_SCRIPT;
							m_IdealMonsterState = MONSTERSTATE_SCRIPT;

							if(m_pCine->m_fMoveTo > 0){
								//pTarget->SetState(MONSTERSTATE_SCRIPT);
								m_hTargetEnt = m_pCine;    //I WILL DESTROY YOUR VERY WILL TO LIIIIIIIIIIIIIIVE
							}

						}
					}
				}
				*/
			}//END OF loop
		}//END OF pEdict check (first)

	}//END OF verify I may want a cine object.
}

void CBaseMonster::Spawn( ){
	// careful, not everything calls the parent spawn method, even if it should.
	// MonsterInit is a better place that's often called by most monster Spawn scripts. Beware of those that don't even do that.

	CBaseToggle::Spawn();
	//setModelCustom();
}


//MODDD - use "usesAdvancedAnimSystem()" to determine whether to use LookupActivityHard for this or just the ordinary LookupActivity.
int CBaseMonster::LookupActivityFiltered(int NewActivity){
	if(usesAdvancedAnimSystem()){
		//return LookupActivityHard ( NewActivity );
		return tryActivitySubstitute ( NewActivity );
	}else{
		return LookupActivity ( NewActivity );
	}
}//END OF LookupActivityFiltered



//Name is kind of odd.  Gets the (or an) animation (by integer?) related to this activity.
int CBaseMonster::LookupActivity(int activity )
{
	if( !usesAdvancedAnimSystem()){
		//not using the advanced anim? Nothing special.
	}else{
		int animationTry = tryActivitySubstitute(activity);
		
		//Nevermind this check, return regardless. We assume tryActivitySubstitute already tried using the base model (CBaseAnimating::LookupActivity) as a last resort, so no need to try that again.
		//if(animationTry != ACTIVITY_NOT_AVAILABLE){
			//EASY_CVAR_PRINTIF_PRE(hassaultPrintout, easyPrintLine( "PANTHEREYE: LOOKUP ACT %d : %d", activity, animationTry));
			return animationTry;
		//}
	}
	return CBaseAnimating::LookupActivity(activity);
}
int CBaseMonster::LookupActivityHeaviest(int activity )
{
	if( !usesAdvancedAnimSystem()){
		//not using the advanced anim? Nothing special.
	}else{
		int animationTry = tryActivitySubstitute(activity);
		
		//if(animationTry != ACTIVITY_NOT_AVAILABLE){
			//EASY_CVAR_PRINTIF_PRE(hassaultPrintout, easyPrintLine( "PANTHEREYE: LOOKUP ACTHEAV %d : %d", activity, animationTry));
			return animationTry;
		//}
	}
	return CBaseAnimating::LookupActivityHeaviest(activity);
}


//NOTICE - the methods "LookupActivityHard" and "tryActivitySubstitute" are meant to be implemented by other monsters that choose to hardcode
//         sequence choices for activities (LookupActivityHard) or indications of having activities but not adjusting themselves just from
//         such a check (tryActivitySubstitute). Their base implementations in CBaseAnimating for everything else, no different from LookupActivity,
//         are just fine.
//         Monsters no longer need to implement "LookupActivity" or "LookupActivityHeaviest", the as-is retail activity-sequence methods, since
//         the new version above for CBaseMonster already does the "usesAdvancedAnimSystem()" check. That must still be done to take advantage of
//         LookupActivityHard and tryActivitySubstitute implementations / sequence injections.



//MODDD - new method for determining whether to register a case of damage as worthy of LIGHT_DAMAGE or HEAVY_DAMAGE for the AI.
//        Can result in interrupting the current schedule.
//        This is expected to get called from CBaseMonster's TakeDamage method in combat.cpp. This method may be customized per monster,
//        should start with a copy of this method without calling the parent method, not much here.
void CBaseMonster::OnTakeDamageSetConditions(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType, int bitsDamageTypeMod){

	//MODDD - intervention. Timed damage might not affect the AI since it could get needlessly distracting.
	if(bitsDamageTypeMod & (DMG_TIMEDEFFECT|DMG_TIMEDEFFECTIGNORE) ){
		//If this is continual timed damage, don't register as any damage condition. Not worth possibly interrupting the AI.
		return;
	}




	//MODDD - HEAVY_DAMAGE was unused before.  For using the BIG_FLINCH activity that is (never got communicated)
	//    Stricter requirement:  this attack took 70% of health away.
	//    The agrunt used to use this so that its only flinch was for heavy damage (above 20 in one attack), but that's easy by overriding this OnTakeDamageSetconditions method now.
	//    Keep it to using light damage for that instead.
	//if ( flDamage >= 20 )
	if (gpGlobals->time >= forgetBigFlinchTime && (flDamage >= pev->max_health * 0.55 || flDamage >= 30))
	{
		SetConditions(bits_COND_HEAVY_DAMAGE);
		forgetSmallFlinchTime = gpGlobals->time + DEFAULT_FORGET_SMALL_FLINCH_TIME;
		forgetBigFlinchTime = gpGlobals->time + DEFAULT_FORGET_BIG_FLINCH_TIME;
		return;
	}

	//  OTHERWISE...

	//default case from CBaseMonster's TakeDamage.
	//Also count being in a non-combat state to force looking in that direction.  But maybe at least 0 damage should be a requirement too, even in cases where the minimum damage for LIGHT is above 0?
	if (m_MonsterState == MONSTERSTATE_IDLE || m_MonsterState == MONSTERSTATE_ALERT || flDamage > 0 )
	{

		if (m_pSchedule == slIdleTrigger && ((bitsDamageType & DMG_CRUSH) || (bitsDamageTypeMod & DMG_MAP_BLOCKED)) ) {
			// MODDD - WEIRD BUG.  for whatever reason, some 'func_door' can cause the hgrunts in a2a1 to flinch and then face
			// the direction the player's coming from, and does odd things to the interactions with a gargantua they should
			// be focused on instead.
			// APPARENTLY, that part of it wasn't the bug.  Turning to face the door is the bug, but it's just some way to 
			// break the hgrunts out of the starting 'slIdleTrigger' schedule set by StartMonster on noticing an entity
			// has a non-blank target name.
			// So, solution:  If this monster is in that state WHILE taking damage from map-source'd damage,
			// just force the schedule to change.  Skip the LIGHT_DAMAGE indirect nonsense.
			// Make your entities face the right way to begin with dangit, which... the hgrunts in a2a1 were already.
			// Confusing.


			// but... what's causing this??
			const char* classnameInflictor = "NULL";
			const char* classnameAttacker = "NULL";
			Vector inflictorOrigin = g_vecZero;
			Vector attackerOrigin = g_vecZero;

			CBaseEntity* inflictorTest = NULL;
			CBaseEntity* attackerTest = NULL;
			if (pevInflictor != NULL)inflictorTest = CBaseEntity::Instance(pevInflictor);
			if (pevAttacker != NULL)attackerTest = CBaseEntity::Instance(pevAttacker);
			if (inflictorTest != NULL) {
				classnameInflictor = inflictorTest->getClassname();
				inflictorOrigin = inflictorTest->GetAbsOrigin();
			}
			if (attackerTest != NULL) {
				classnameAttacker = attackerTest->getClassname();
				attackerOrigin = attackerTest->GetAbsOrigin();
			}
			int x = 45;
			easyPrintLine("HURT DEBUG: Hit by crushable.  Classname: %s Origin:(%.2f,%.2f,%.2f).", classnameInflictor, inflictorOrigin);

			// Finally.  Just pick a new schedule and get out of this 'IdleTrigger' one, clearly this was a sign just to snap out of it.
			ChangeSchedule(GetSchedule());
			
		}else {
			// Normal behavior,  set LIGHT_DAMAGE and likely to look startled and turn to face the direction suggested elsewhere
			// in TakeDamage ("ow that stings, where'd it come from?")
			SetConditions(bits_COND_LIGHT_DAMAGE);

			//MODDD NEW - set a timer to forget a flinch-preventing memory bit.
			forgetSmallFlinchTime = gpGlobals->time + DEFAULT_FORGET_SMALL_FLINCH_TIME;
		}
	}


/*
	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(testVar) == 10){
		//any damage causes me now.
		SetConditions(bits_COND_HEAVY_DAMAGE);
	}
*/
	//easyForcePrintLine("%s:%d OnTkDmgSetCond raw:%.2f fract:%.2f", getClassname(), monsterID, flDamage, (flDamage / pev->max_health));


}//END OF OnTakeDamageSetConditions



void CBaseMonster::ForgetEnemy(void) {

	m_afMemory &= ~(bits_MEMORY_SUSPICIOUS | bits_MEMORY_PROVOKED);

	if (m_hEnemy != NULL) {
		m_hEnemy = NULL;
		m_hTargetEnt = NULL;

		if (pev->deadflag == DEAD_NO) {
			// only fool around with the states or schedules of alive monsters.  Otherwise freaky stuff happens.
			// And not that this matters for state anyway, it would have to be MONSTERSTATE_DEAD.
			if (this->m_MonsterState == MONSTERSTATE_COMBAT) {
				// doesn't make sense to be in COMBAT without an enemy.
				this->m_MonsterState = MONSTERSTATE_ALERT;
			}
			TaskFail();
		}
	}

}//END OF ForgetEnemy

//by default, does nothing. Used for the Kingpin to let a monster remove itself from the Kingpin's command list of entities it powered up and can order to attack its enemy.
void CBaseMonster::removeFromPoweredUpCommandList(CBaseMonster* argToRemove){

}


void CBaseMonster::forceNewEnemy(CBaseEntity* argIssuing, CBaseEntity* argNewEnemy, BOOL argPassive){
	//Things that expect to have this called should implement this better. When to change schedules, etc.

	if(m_hEnemy != argNewEnemy){
		m_hEnemy = argNewEnemy;
		SetState(MONSTERSTATE_COMBAT);
		if(!argPassive){
			//ChangeSchedule(GetSchedule());
			TaskFail();
		}
	}

}//END OF forceNewEnemy

//dummied by default
void CBaseMonster::setPoweredUpOff(void){}
void CBaseMonster::setPoweredUpOn(CBaseMonster* argPoweredUpCauseEnt, float argHowLong ){}
// Not to be confused with "ForgetEnemy" further above.
// This is for forgetting an enemy assigned by some other entity.
void CBaseMonster::forgetForcedEnemy(CBaseMonster* argIssuing, BOOL argPassive){};



// Called by 'riseFromTheGrave' from islave.cpp, not just the 'revive' console cheat.
void CBaseMonster::StartReanimation(void){
	int i;
	
	// ???????
	//m_IdealMonsterState = MONSTERSTATE_ALERT;// Assume monster will be alert, having come back from the dead and all.
	//m_MonsterState = MONSTERSTATE_ALERT; //!!!

	//this->m_Activity = ACT_RESET;


	pev->deadflag = DEAD_NO;
	
	//before spawn or init script may interfere.
	int oldSeq = pev->sequence;
	//no recollection of that.
	//m_hEnemy = NULL;
	ForgetEnemy();

	//And clear the list of old enemies.
	//or m_intOldEnemyNextIndex - 1 ?
	for(i = 0; i < MAX_OLD_ENEMIES; i++){
		m_hOldEnemy[i] = NULL;
	}

	//This should set the monster's health to "pev->max_health".
	//And assume it calls "MonsterInit" again if it ever did before.
	Spawn();


	//Most of MonsterInit's script here just to be safe... Nah, assume Spawn calls it if it makes sense to.
	// The area mentioned ranged from
	//     pev->effects		= 0;
	// to
	//     SetEyePosition();
	
	StartReanimationPost(oldSeq);
}//END OF StartReanimation


// Overridable method. What to do when this monster is at the end of revival from StartReanimation above.
// Override to let a monster determine how to revive the monster on its own, like pick a different animation.
// Default behavior is to play the existing animation over again.
void CBaseMonster::StartReanimationPost(int preReviveSequence){

	// doing this extra early to be safe.
	m_afMemory = MEMORY_CLEAR;

	// good idea?
	m_afConditions = 0;
	m_afConditionsNextFrame = 0;
	m_afConditionsMod = 0;
	m_afConditionsModNextFrame = 0;

	m_IdealMonsterState	= MONSTERSTATE_ALERT;// Assume monster will be alert, having come back from the dead and all.
	m_MonsterState = MONSTERSTATE_ALERT; //!!!

	m_IdealActivity = ACT_IDLE;
	m_Activity = ACT_IDLE; //!!! No sequence changing, force the activity to this now.

	pev->sequence = -1; //force reset.
	SetSequenceByIndex(preReviveSequence, -1, FALSE);

	ChangeSchedule(slWaitForReviveSequence );
}//END OF StartReanimationPost


//Overridable. Whether a monster wants to turn to face a node up to so many degrees (such as +- 45 degrees before going back to walking or running).
// as this is plus or minus already, negative values make no sense. Negative 1 allows all angles as well.
//Monsters that can strafe like hgrunts or flyers may want to override this to depend on some other factor, like being mid-strafe or flight or not.
float CBaseMonster::MoveYawDegreeTolerance(){
	//return -1;
	return 35;
}


// Shortcut to UTIL_BloodColorRedFilter that lets a monster provide its own german model requirement CVar automatically.
int CBaseMonster::BloodColorRedFilter(){
	return UTIL_BloodColorRedFilter(getGermanModelRequirement()==1);
}

int CBaseMonster::CanUseGermanModel(){
	// If we can use our german model and found ours
	return (getGermanModelsAllowed() && getGermanModelRequirement()==1 );
}


// Try to find the earliest node that is marked GOAL. Sometimes this gets shifted around from 0
WayPoint_t* CBaseMonster::GetGoalNode(void){

	/*
	for(int i = 0; i < m_iRouteLength; i++){
		if(m_Route[i].iType & bits_MF_IS_GOAL){
			// it is this.
			return &m_Route[i];
		}
	}
	*/

	// Just do this now,
	if(m_iRouteLength == 0){
		// no route?  No goal node
		return NULL;
	}
	if(m_Route[m_iRouteLength-1].iType & bits_MF_IS_GOAL){
		// that's it
		return &m_Route[m_iRouteLength-1];
	}


	easyPrintLine("!!! ROUTE DEBUG %s:%d NOTICE!  Route of node-length %d had no goal! Movegoal: %d", getClassname(), monsterID, m_iRouteLength, m_movementGoal);


	// didn't find a node with bits_MF_IS_GOAL set from 0 to m_iRouteIndex?
	// Unsure if treating the node at m_Route[m_iRouteLength-1] would be fine anyway.
	// Can't think of when a route would lack a GOAL-marked node unless it ran out at 
	// the max number of nodes a route can hold (ROUTE_SIZE).
	return NULL;
}//END OF getGoalNode


// Use the current 'moveGoal' type to tell which entity handle (m_hEnemy, m_hTarget, or none)
// has the relevant entity to use as a 'goal' for pathfinding calls, most likely CheckLocalMove.
// The goal ent is able to be hit by pathfinding and still count as a 'successful' path.  With it
// missing some of the pathfind failures can seem silly.
CBaseEntity* CBaseMonster::GetGoalEntity(void){
	if(m_movementGoal == MOVEGOAL_ENEMY){
		if(m_hEnemy != NULL){
			return m_hEnemy.GetEntity();
		}
	}else if(m_movementGoal == MOVEGOAL_TARGETENT){
		if(m_hTargetEnt != NULL){
			return m_hTargetEnt.GetEntity();
		}
	}else{
		// No entity associated with any other movegoals.
	}
	return NULL;
}



//MODDD - off for most.  Some new NPCs may use this (or old could be made to).
BOOL CBaseMonster::hasSeeEnemyFix(void){
	return FALSE;
}

// Can this monster accept a new enemy, regardless of whether or not the current schedule is interruptible by "cond_NEW_ENEMY" ?
BOOL CBaseMonster::getForceAllowNewEnemy(CBaseEntity* pOther){
	return FALSE;
}


// Monsters with bigger bounds may need them to be temporarily reduced to play better with the pathfiding.
// Side effects not well understood yet, see if this is worth it.
// By default off, enable per monsters as needed.
BOOL CBaseMonster::needsMovementBoundFix(){

	return FALSE;
}//END OF needsMovementBoundFix


void CBaseMonster::cheapKilled(void){
	this->m_IdealMonsterState = MONSTERSTATE_DEAD;
	//MODDD - major HACK - pathetic stand-in death anim until there is a proper death anim.
	this->pev->gravity = 0;
	pev->movetype		= MOVETYPE_NONE;
	this->pev->origin = pev->origin + Vector(0, 0, 13.4);   //so it isn't poking through the ground. Yes this will look weird.
	this->pev->angles = Vector(0, 0, 90);
	this->pev->framerate = 0;
	this->m_flFramerateSuggestion = 0;
	this->m_flFrameRate = 0;

	//this->pev->frame = 255;
	//this->m_fSequenceFinished = TRUE;


	DeathAnimationStart();
	DeathAnimationEnd();
}//END OF cheapKilled


// Version of cheapKilled for flying monsters. They should still drop to the ground as expected, not akwardly freeze in mid-air.  Or maybe they should? dunno.
void CBaseMonster::cheapKilledFlyer(void){
	this->m_IdealMonsterState = MONSTERSTATE_DEAD;
	//MODDD - major HACK - pathetic stand-in death anim until there is a proper death anim.
	//this->pev->gravity = 0;

	// Not working out so great anymore.  okay then..
	//pev->movetype		= MOVETYPE_STEP;

	//this->pev->origin = pev->origin + Vector(0, 0, 13.4);   //so it isn't poking through the ground. Yes this will look weird.
	
	// Copied from Flyer's killed script
	ClearBits( pev->flags, FL_ONGROUND );
	//pev->angles.z = 0;
	//pev->angles.x = 0;

	this->pev->angles = Vector(0, 0, 90);
	this->pev->framerate = 0;
	this->m_flFramerateSuggestion = 0;
	this->m_flFrameRate = 0;

	//this->pev->frame = 255;
	//this->m_fSequenceFinished = TRUE;


	DeathAnimationStart();
	DeathAnimationEnd();
}//END OF cheapKilledFlyer


// When killed, how do we handle the "Touch" callback method from the engine?
// Default says to set it to NULL. Override this for different behavior.
// i.e., flyers / hover-ers can tell this to interrupt a falling cycler animation on colliding with anything (the ground?).
// In that case, default behavior of turning touch off would leave you scratching your head as to why it ignores everything after killed.
// HOWEVER the new slDieFallLoop schedule include setting Touch to KilledFinishTouch which works for all monsters.  This method (OnKilledSetTouch) may no longer be necessary.
void CBaseMonster::OnKilledSetTouch(void){
	
	//m_pfnTouch = NULL;
	//SetTouch(NULL);

	//if (this->m_pfnTouch != NULL) {
	SetTouch(NULL);
	//}
}

// I need to finish the looping death animation with the "OnKilledSetTouch" one.
void CBaseMonster::KilledFinishTouch( CBaseEntity *pOther ){
	if(pOther == NULL){
		return; //??????
	}
	
	//hitGroundDead = TRUE;
	
	// old was "slDieLoop".
	// Do we even need the schedule + task check?  Touch anything, complete my task.
	//if(m_pSchedule == slDieFallLoop && this->getTaskNumber() == TASK_DIE_LOOP){
		TaskComplete();
		SetTouch(NULL); //don't need to do this again.
	//}

}//END OF KilledFinishTouch



// Makes the most sense for flyers to have looping falling animations at deaths in air.
// By default, -1 means none.
// This is called for by the 
int CBaseMonster::getLoopingDeathSequence(void){
	return -1;
}


// Depending on the value of CVar flyerKilledFallingLoop, pick the schedule that uses the looping animation or don't.
Schedule_t* CBaseMonster::flyerDeathSchedule(void){
	
	if(EASY_CVAR_GET_DEBUGONLY(flyerKilledFallingLoop) == 1){
		// First a check. If we are very close or virtually on the ground, just skip to the typical die method to do the "hitting the ground" animation.

		TraceResult tr;
		Vector vecStart = pev->origin + Vector(0, 0, pev->mins.z);

		UTIL_TraceLine(vecStart, vecStart + Vector(0, 0, -6), dont_ignore_monsters, this->edict(), &tr);

		if(tr.fStartSolid || tr.fAllSolid || tr.flFraction < 1.0){
			// go to "slDie" below.
		}else{
			// clear? send this.
			return slDieFallLoop;
		}

	}//END OF flyerKilledFallingLoop check


	//default
	return slDie;
}

// Can this monster automatically turn to face a node better in ::Move above?
// Turn off for things that may play with direction faced (like strafing hgrunts while they are expected to strafe)
BOOL CBaseMonster::getMovementCanAutoTurn(void){
	//without a reason not to, defaults to yes all the time.
	return TRUE;
}




// NOTE - supporting what point to get from an enemy in general in other methods, such as
// Cover-related tasks in schedule.cpp referring to "m_hEnemy->pev->origin" could also
// be overridden to let a monster decide whta point to use from the entity, but that seems
// unecessary.  Whether a point is at the feet or center of a model wouldn't really affect
// that.  Precision, like the LKP used in pathfinding methods to get close to an enemy, could
// be important, especially for small melee-focused things (hornets, archers).

// If there is an enemy, set the m_vecEnemyLKP to the enemy's current position
// Involve the overridable way now.
void CBaseMonster::updateEnemyLKP(void){
	if(m_hEnemy != NULL){
		//investigatingAltLKP = FALSE; //this is the real deal.
		setEnemyLKP(m_hEnemy.GetEntity());
	}
}

// Only given a location?  All I can use, but try to involve the monster itself if 
// possible to let the entity setting the LKP decide what point to use (pev->origin or center).
// Although this isn't do-able on pulling the values from some saved source like a stack, which
// should've been retrieved with the entity's involvement to begin with so that's fine.
void CBaseMonster::setEnemyLKP(const Vector& argNewVector, float zOffset){
	m_vecEnemyLKP = argNewVector;
	m_fEnemyLKP_EverSet = TRUE;
	m_flEnemyLKP_zOffset = zOffset;
	m_vecEnemyLKP_ViewOFS = g_vecZero;  // ???
	investigatingAltLKP = FALSE; //this is the real deal.
}
// uhh.  okay?
void CBaseMonster::setEnemyLKP(const Vector& argNewVector, float zOffset, const Vector& extraAddIn){
	m_vecEnemyLKP = argNewVector + extraAddIn;
	m_fEnemyLKP_EverSet = TRUE;
	m_flEnemyLKP_zOffset = zOffset;
	m_vecEnemyLKP_ViewOFS = g_vecZero;  // ???
	investigatingAltLKP = FALSE; //this is the real deal.
}

// NEW IDEA: Monsters can override this form if they prefer to treat a different point
// of the monster as the position, like the Center() instead of the pev->origin (often
// at the lowest point of the monster's bounding box).
// Archers can use this to try and path to an enemy's center instead of its feet.
void CBaseMonster::setEnemyLKP(CBaseEntity* theEnt){
	//m_vecEnemyLKP = theEnt->pev->origin;
	m_vecEnemyLKP = theEnt->pev->origin;
	m_flEnemyLKP_zOffset = theEnt->pev->mins.z;
	m_vecEnemyLKP_ViewOFS = theEnt->pev->view_ofs;

	m_fEnemyLKP_EverSet = TRUE;
	investigatingAltLKP = FALSE; //this is the real deal.
}

// ALSO.  On being given an extra amount to add to that, can handle that differently too.
// Or the default way will call plain setEnemyLKP with the entity, and then add in extraAddIn.
// So, optional to override this, this assumption should always work fine.
// Unless there is a time the enemy LKP setting should be denied, where adding the add-in
// is pointless.  This assumes the vecEnemyLKP from some point of the ent was applied.
void CBaseMonster::setEnemyLKP(CBaseEntity* theEnt, const Vector& extraAddIn){
	setEnemyLKP(theEnt);
	m_vecEnemyLKP = m_vecEnemyLKP + extraAddIn;
}


// And, given a PEV, redirect to setEnemyLKP.
void CBaseMonster::setEnemyLKP(entvars_t* theEntPEV){
	setEnemyLKP(CBaseEntity::Instance(theEntPEV));
}
void CBaseMonster::setEnemyLKP(entvars_t* theEntPEV, const Vector& extraAddIn){
	setEnemyLKP(CBaseEntity::Instance(theEntPEV), extraAddIn);
}


// Push this new vector to m_vecEnemyLKP.
// Save the old enemy LKP to another var to back up to when done with that.
//MODDD - TODO?  Support the other overload stuff above?  Not sure if this is really used by anything
// that treats enemy point to use for LKP differently.
void CBaseMonster::setEnemyLKP_Investigate(const Vector& argToInvestigate){
	if(m_fEnemyLKP_EverSet){
		// I can trust it.
		m_vecEnemyLKP_Real = m_vecEnemyLKP;
		m_flEnemyLKP_Real_zOffset = m_flEnemyLKP_zOffset;
		m_vecEnemyLKP_Real_ViewOFS = m_vecEnemyLKP_ViewOFS;
	}else{
		// Never was set? ok.
	}
	// Say whether what is now in LKP_Real can be trusted when the time comes to decide whether
	// to set the normal LKP To that.
	m_fEnemyLKP_Real_EverSet = m_fEnemyLKP_EverSet;


	m_vecEnemyLKP = argToInvestigate;
	// should be ok to set zOffset to 0, this is to investigate a point after all
	// (likely already snapped to ground or close)
	m_flEnemyLKP_zOffset = 0;
	m_vecEnemyLKP_ViewOFS = g_vecZero;
	m_fEnemyLKP_EverSet = TRUE;
	investigatingAltLKP = TRUE;
}




//MODDD
// Moved to the base monster for usefullness.  Probably needs to be overridden per monster.
// More of a special utility per monster, bound not to be salvagable on its own here.
CBaseEntity* CBaseMonster::getNearestDeadBody(const Vector& arg_searchOrigin, const float arg_maxDist){

	CBaseEntity* pEntityScan = NULL;
	CBaseMonster* testMon = NULL;
	float thisDistance;
	CBaseEntity* bestChoiceYet = NULL;
	float leastDistanceYet = arg_maxDist; //furthest I go to a dead body.

	//does UTIL_MonstersInSphere work?
	while ((pEntityScan = UTIL_FindEntityInSphere( pEntityScan, arg_searchOrigin, 800 )) != NULL)
	{
		if(pEntityScan->pev == this->pev){
			//is it me? skip it.
			continue;
		}

		testMon = pEntityScan->MyMonsterPointer();
		//if(testMon != NULL && testMon->pev != this->pev && ( FClassnameIs(testMon->pev, "monster_scientist") || FClassnameIs(testMon->pev, "monster_barney")  ) ){

		//MODDD TODO - should this be able to be fooled by things playing dead?
		//             Use "testMon->IsAlive_FromAI()" to be fool-able instead of straight deadflag checks.  But the player one (FL_CLIENT check first) is still fine.
		

		//Don't try to eat leeches, too tiny. Nothing else this small leaves a corpse.
		if(testMon != NULL && (testMon->pev->deadflag == DEAD_DEAD || ( (testMon->pev->flags & (FL_CLIENT)) && testMon->pev->deadflag == DEAD_RESPAWNABLE && !(testMon->pev->effects &EF_NODRAW) ) ) && testMon->isSizeGiant() == FALSE && testMon->isOrganicLogic() && !(::FClassnameIs(testMon->pev, "monster_leech") ) ){
			thisDistance = (testMon->pev->origin - arg_searchOrigin).Length();
			
			if(thisDistance < leastDistanceYet){
				//WAIT, one more check. Look nearby for players.

				//UTIL_TraceLine ( node.m_vecOrigin + vecViewOffset, vecLookersOffset, ignore_monsters, ignore_glass,  ENT(pev), &tr );
				
				bestChoiceYet = testMon;
				leastDistanceYet = thisDistance;
				
			}//END OF minimum distance yet
		}//END OF entity scan null check

	}//END OF while loop through all entities in an area to see which are corpses.
	return bestChoiceYet;

}//END OF getNearestDeadBody

// Whether to do the usual "Look" method for checking for new or existing enemies.
// Needed by archers to be able to call "Look" despite the player never going underwater (causes the PVS check to pass at least once to start combat).
BOOL CBaseMonster::noncombat_Look_ignores_PVS_check(void){
	return FALSE;
}//END OF noncombat_Look_ignores_PVS_check

// Implement for monsters that should spawn regardless of the "mp_allowmonsters" CVar.
// Pickup walkers are a clear example since they're needed for some weapons intended
// by the map.
BOOL CBaseMonster::bypassAllowMonstersSpawnCheck(void){
	return FALSE;
}//END OF bypassAllowMonstersSpawnCheck



// It is up to an individual monster with a violent death sequence, regardless of whether it's mapped to activity ACT_DIEVIOLENT or not, to
// override this and say "TRUE", along with providing its own rule for a clear distance check.  That is, if whatevre distance forwards/backwards
// (typically backwards) is safe.  But it varries from monster to monster's sequence, just have to make it so the sequence can't clip through anything
// and look weird.
// It is also OK if the sequence for violent death is not tied to ACT_DIEVIOLENT in the model. Then pick it for ACT_DIEVIOLENT in the custom animation system instead.
BOOL CBaseMonster::violentDeathAllowed(void){
	return FALSE;
}//END OF hasViolentDeathSequence

// Default case should work fine for most monsters.
// Only allow a violent death animation if the last hit solidly did this much damage.
// Could do checks on m_LastHitGroup too (see method GetDeathActivity of combat.cpp)
BOOL CBaseMonster::violentDeathDamageRequirement(void){
	return (m_lastDamageAmount >= 20);
}

// This method MUST be overridden to do line traces forwards / backwards to see if there is enough space in whatever direction to play the animation
// to avoid clipping through things.  If this is unnecessary, leave this as it is.  But "hasViolentDeathSequence" must  be overridden to return TRUE;
// above regardless.
BOOL CBaseMonster::violentDeathClear(void){
	return TRUE;
}//END OF violentDeathClear

//MODDD - the priority that ACT_DIEVIOLENT gets compared to other activities.  That is, even if the conditions for a violent death act are met or could
//        be met if checked, what activities should get picked first anyways if their criteria matches at the same time?  Only applies if "violentDeathAllowed" is TRUE.
//Modes:
//1: top. violent death happens if the conditions are met period above all other death activities.
//2: mid. violent death happens before (priority over) any directional death checks (forwards and backwards), but after (loses to) the model hitbox check ones
//   (ACT_DIE_HEADSHOT and ACT_DIE_GUTSHOT)
//3: low. violent death happens only during the last hit being on the front of the enemy to compete with ACT_DIEBACKWARD, which it takes precedence over if so.
//   Otherwise, all other death acts get priority.
int CBaseMonster::violentDeathPriority(void){
	return 3;
}//END OF violentDeathPriority


// a utility for checking to see if behind a monster by up to "argDistance" is completely unobstructed and is of the same ground level as this entity.
// That way a violent death anim can't work when there's a cliff backwards. Does not know to fall as the model's movement is just part of the animation.
BOOL CBaseMonster::violentDeathClear_BackwardsCheck(float argDistance){
	TraceResult tr;
	Vector vecStart;
	Vector vecEnd;
	Vector vecGroundEnd;

	UTIL_MakeVectors ( pev->angles );
	
	vecStart = Center();
	vecEnd = vecStart - gpGlobals->v_forward * argDistance;
	vecGroundEnd = (pev->origin + Vector(0, 0, pev->mins.z) ) - gpGlobals->v_forward * argDistance;

	
	
	UTIL_TraceHull ( vecStart, vecEnd, dont_ignore_monsters, head_hull, edict(), &tr );
	
	
	// Nothing in the way? it's good.
	if ( tr.flFraction == 1.0 ){
		//return TRUE;
		//not yet, there's another hoop to go through.
	}else{
		return FALSE;
	}

	// Pretty good distance. would that make us fall either? Is there ground at this place "vecEnd"?
	//UTIL_TraceLine or UTIL_TraceLine?
	UTIL_TraceLine ( vecGroundEnd, vecGroundEnd + Vector(0, 0, -4), dont_ignore_monsters, edict(), &tr );

	float zDelta = fabs(tr.vecEndPos.z - pev->origin.z);
	if(!tr.fStartSolid && !tr.fAllSolid && tr.flFraction < 1.0 && zDelta < 3){
		//it is good!
		return TRUE;
	}
	
	return FALSE;
}//END OF violentDeathClear_BackwardsCheck


void CBaseMonster::lookAtEnemyLKP(void){
					
	MakeIdealYaw ( m_vecEnemyLKP );
	ChangeYaw ( pev->yaw_speed );
}

// Is this monster able to predict re-picking the same activity in the next frame?
// If so, and the conditions for it are met, this can be set to do so.
// BUT not all monsters play nicely with this. Some fidget for a moment on the same animation for a frame and stop
// if it does not get picked.
void CBaseMonster::predictActRepeat(int arg_bits_cond){
	if(HasConditions(arg_bits_cond)){
		m_Activity = ACT_RESET;
	}
}

// Am I able to use the "predictActRepeat" check above at all? Defaults to TRUE, set differently when problematic.
BOOL CBaseMonster::canPredictActRepeat(void){
	return TRUE;
}//END OF canPredictActRepeat




//MODDD - came from hgrunt.cpp, used to be named Kick. Now callable by other monsters too.
//        This is probably some old version of checking for entities in front before it was standardized to
//        CheckTraceHullAttack.  But whatever, this could even indirectly call that too instead.

CBaseEntity* CBaseMonster::HumanKick( void ){
	return HumanKick(70);  //default.
}

CBaseEntity* CBaseMonster::HumanKick(float argCheckDistance ){
	TraceResult tr;

	UTIL_MakeVectors( pev->angles );
	Vector vecStart = pev->origin;
	vecStart.z += pev->size.z * 0.5;
	Vector vecEnd = vecStart + (gpGlobals->v_forward * argCheckDistance);

	UTIL_TraceHull( vecStart, vecEnd, dont_ignore_monsters, head_hull, ENT(pev), &tr );

	if ( tr.pHit )
	{
		CBaseEntity *pEntity = CBaseEntity::Instance( tr.pHit );

		return pEntity;
	}

	return NULL;
}


void CBaseMonster::precacheStandardMeleeAttackMissSounds(void){
	PRECACHE_SOUND_ARRAY(pStandardAttackMissSounds);
}
void CBaseMonster::precacheStandardMeleeAttackHitSounds(void){
	PRECACHE_SOUND_ARRAY(pStandardAttackHitSounds);
}

void CBaseMonster::playStandardMeleeAttackMissSound(void){
	UTIL_PlaySound( ENT(pev), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pStandardAttackMissSounds), 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );
}
void CBaseMonster::playStandardMeleeAttackHitSound(void){
	UTIL_PlaySound( ENT(pev), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pStandardAttackHitSounds), 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );
}
void CBaseMonster::playStandardMeleeAttackMetalHitSound(void){
	UTIL_playMeleeMetalHitSound(pev);
}

// Similar to above, but offering a custom hit-sound array and volume/attn/pitch stats for the normal sounds to use if picked.
void CBaseMonster::playStandardMeleeAttackHitSound(CBaseEntity* hitEnt, const char** normalHitSounds, int normalHitSoundsSize, float vol, float attn, int pitchMin, int pitchMax){
	// a choice?  ok, is it organic or not?
	// Play the normal sound anyway on most map-related things (IsWorldOrAffiliated may have worked fine).
	if(hitEnt == NULL || hitEnt->IsWorld() || hitEnt->Classify() == CLASS_NONE || hitEnt->isOrganic()){
		// normal sounds.  Use the custom sound list and other info given.
		UTIL_PlaySound( ENT(pev), CHAN_WEAPON, normalHitSounds[ RANDOM_LONG(0, normalHitSoundsSize-1) ], vol, attn, 0, RANDOM_LONG(pitchMin, pitchMax) );
	}else{
		// metal ones
		playStandardMeleeAttackMetalHitSound();
	}
}


// Given an entity hit, can decide whether to play the metal hit sounds instead.
// A monster can override the 'playStandardMeleeAttackHitSound & MetalHitSound' methods for custom behavior each that way too.
// The TraceAttack call can then call 'determineStandardMeleeAttackHitSound(pHurt)' to pick which one.
void CBaseMonster::determineStandardMeleeAttackHitSound(CBaseEntity* hitEnt){
	// Play the normal sound anyway on most map-related things (IsWorldOrAffiliated may have worked fine).
	if(hitEnt == NULL || hitEnt->IsWorld() || hitEnt->Classify() == CLASS_NONE || hitEnt->isOrganic()){
		// normal sounds
		playStandardMeleeAttackHitSound();
	}else{
		// metal ones
		playStandardMeleeAttackMetalHitSound();
	}
}










BOOL CBaseMonster::traceResultObstructionValidForAttack(const TraceResult& arg_tr){
	//Given a TraceResult that has already been written to by some trace method,
	//did anything block the trace and, if so, was it something I can live with attacking?
	//IMPORTANT - does not include fStartSolid and fAllSolid checks, handle those separately.

	if(arg_tr.flFraction < 1.0f && arg_tr.pHit != NULL){
		//what is blocking me?
		//CBaseEntity* tempEnt = CBaseEntity::Instance(trTemp.pHit);

		const char* daName = STRING(arg_tr.pHit->v.classname);

		if(m_hEnemy != NULL && arg_tr.pHit == m_hEnemy.Get()){
			//has an enemy, matches it? this is acceptable.
		}else{
			//anything else blocking? wait... maybe I hate it anyways.
			CBaseEntity* tempEnt = CBaseEntity::Instance(arg_tr.pHit);
			if(tempEnt == NULL){
				// nothing, what?
				return TRUE;
			}else if(this->IRelationship(tempEnt) > R_NO){
				//horray, I hate it anyways.
				return TRUE;
			}else{
				//let's not.
				return FALSE;
			}
		}
	}//END OF fraction and thing-hit checks.


	return TRUE;
}//END OF traceResultObstructionValidForAttack


//default, override if necessary.
float CBaseMonster::getDistTooFar(void){
	return 1024.0;
}
//default, override if necessary.
float CBaseMonster::getDistLook(void){
	return 2048.0;
}


void CBaseMonster::ReportGeneric() {
	CBaseEntity::ReportGeneric();

	//Also call our own special report method: ReportAI.
	ReportAIState();

}//END OF ReportGeneric()

// When my marked enemy has recently died, let me know.
// m_hEnemy could be cleared at any moment after all.
void CBaseMonster::onEnemyDead(CBaseEntity* pRecentEnemy) {

}

// Can this monster try to anticipate ranged attacks ending to end the schedule early?
// This stops (or at least makes less noticeable) the monster from sticking on the last frame
// of the animation for a think-frame.  But it's not good for most monsters that rely on every
// bit of the ranged anim playing (like hgrunts, it can skip portions of the sequence for firing
// inconsistently).  Good for slow-firing ones or long ranged anims, like the agrunt's.
BOOL CBaseMonster::predictRangeAttackEnd(void) {
	return FALSE;
}




// from weapons.cpp.
void CBaseMonster::CheckRespawn(void)
{
	// TEST
	//Respawn();
	//return;

	switch (g_pGameRules->MonsterShouldRespawn(this))
	{
	case GR_WEAPON_RESPAWN_YES:
		Respawn();
		break;
	case GR_WEAPON_RESPAWN_NO:
		return;
		break;
	}
}


// A modified clone of weapons.cpp's "Respawn" method for handling a AI monstser that should stay dormant during that time and go to the first place this was seen on the map.
// WARNING - only call for things that inherit from the 'CRespawnable' class as well as CBaseMonster (basemonster.h).
// NO.  This 'CRespawnable' stuff is diabolical and evil.  Don't.
CBaseEntity* CBaseMonster::Respawn(void)
{
	// WARNING - reinterpret_cast can be evil.
	//CRespawnable* selfRespawnableRef = reinterpret_cast<CRespawnable*>(this);
	CBaseMonster* selfRespawnableRef = this;

	// make a copy of this weapon that is invisible and inaccessible to players (no touch function). The weapon spawn/respawn code
	// will decide when to make the weapon visible and touchable.
	CBaseEntity* someEnt = CBaseEntity::CreateManual((char*)STRING(pev->classname), g_pGameRules->VecMonsterRespawnSpot(this, selfRespawnableRef->respawn_origin), selfRespawnableRef->respawn_angles, pev->owner);
	//CPickupWalker* pNewMonster = static_cast<CPickupWalker*>(someEnt);

	if (someEnt == NULL)return NULL;  // ???

	DispatchCreated(someEnt->edict());

	CBaseMonster* pNewMonster = someEnt->GetMonsterPointer();
	//CRespawnable* newRespawnableRef = reinterpret_cast<CRespawnable*>(someEnt);

	//easyForcePrintLine("KNEEB %.2f %.2f %.2f", selfRespawnableRef->respawn_origin.x, selfRespawnableRef->respawn_origin.y, selfRespawnableRef->respawn_origin.z);
	if (pNewMonster)
	{
		// MODDD - TODO.  wait, shouldn't we also set pev->solid to SOLID_NOT ??
		// then again it doesn't matter, what else is SOLID_TRIGGER with a NULL TOUCH method anyway?  No impact.

		pNewMonster->pev->spawnflags &= ~SF_NORESPAWN;

		// also, fade the monster on death.  Don't want to spam the game.
		pNewMonster->pev->spawnflags |= SF_MONSTER_FADECORPSE;

		// pass on my respawn_ info:
		pNewMonster->respawn_origin = selfRespawnableRef->respawn_origin;
		pNewMonster->respawn_angles = selfRespawnableRef->respawn_angles;

		//DebugLine_SetupPoint(0, newRespawnableRef->respawn_origin, 0, 255, 0);

		pNewMonster->pev->takedamage = DAMAGE_NO;
		pNewMonster->pev->solid = SOLID_NOT;  // safety
		pNewMonster->pev->effects |= EF_NODRAW;// invisible for now
		pNewMonster->SetTouch(NULL);// no touch
		pNewMonster->SetThink(&CBaseMonster::AttemptToMaterialize);

		DROP_TO_FLOOR(ENT(pev));

		// not a typo! We want to know when the weapon the player just picked up should respawn! This new entity we created is the replacement,
		// but when it should respawn is based on conditions belonging to the weapon that was taken.

		//pNewMonster->pev->nextthink = gpGlobals->time + 6;
		pNewMonster->pev->nextthink = g_pGameRules->FlMonsterRespawnTime(this);
	}
	else
	{
		ALERT(at_console, "Respawn failed to create %s!\n", STRING(pev->classname));
	}

	return pNewMonster;
}




void CBaseMonster::AttemptToMaterialize(void)
{
	float time = g_pGameRules->FlMonsterTryRespawn(this);

	//DebugLine_SetupPoint(0, pev->origin, 0, 0, 255);
	if (time == 0)
	{
		Materialize();
		return;
	}

	pev->nextthink = gpGlobals->time + time;
}

void CBaseMonster::Materialize(void)
{
	if (pev->effects & EF_NODRAW)
	{
		// changing from invisible state to visible.
		UTIL_PlaySound(ENT(pev), CHAN_WEAPON, "items/suitchargeok1.wav", 1, ATTN_NORM, 0, 150, FALSE);
		pev->effects &= ~EF_NODRAW;
		pev->effects |= EF_MUZZLEFLASH;
	}

	//pev->solid = SOLID_TRIGGER;

	UTIL_SetOrigin(pev, pev->origin);// link into world.

	//SetTouch(&CPickupWalker::DefaultTouch);
	//SetThink(NULL);


	//SetTouch(&CPickupWalker::PickupWalkerTouch);
	//MonsterInit();
	//SetTouch(&CPickupWalker::PickupWalkerTouch);
	//pev->solid = SOLID_TRIGGER;

	// No need.  The moneter's own spawn or MonsterInit should handle this.
	//pNewMonster->pev->takedamage = DAMAGE_NO;

	// should call MonsterInitThink.
	//DispatchSpawn(this->edict());
	this->Spawn();

	//SetThink(&CBaseMonster::MonsterInitThink);
	//SetTouch(&CPickupWalker::PickupWalkerTouch);
	//pev->nextthink = gpGlobals->time + 0.1;

}


// Overridable.  Gargantua, for instance, should have lower attenuations for any sounds played this way.
// Retail defaults would be ATTN_IDLE (2.0) for both of these for everything.
float CBaseMonster::ScriptEventSoundAttn(void){
	return 0.9;
}
float CBaseMonster::ScriptEventSoundVoiceAttn(void){
	return 1.0;
}

BOOL CBaseMonster::CanMakeBloodParticles(void){
	return TRUE;
}

// Default:  is this movetype STEP?  Can be overridden by flying monsters that aren't giant (stukabat, archer) to still get pushed.
BOOL CBaseMonster::AffectedByKnockback(void){
	return (pev->movetype == MOVETYPE_STEP);
}


