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
// Panthereye - started by Osiris / OsirisGodoftheDead / THE_YETI
//=========================================================

#include "panthereye.h"
#include "schedule.h"
#include "activity.h"
#include "util_model.h"
#include "defaultai.h"
#include "soundent.h"
#include "game.h"
#include "util_debugdraw.h"

EASY_CVAR_EXTERN_DEBUGONLY(noFlinchOnHard)
EASY_CVAR_EXTERN_DEBUGONLY(panthereyeHasCloakingAbility)
EASY_CVAR_EXTERN_DEBUGONLY(panthereyeJumpDotTol)
EASY_CVAR_EXTERN_DEBUGONLY(panthereyePrintout)
EASY_CVAR_EXTERN_DEBUGONLY(animationFramerateMulti)
EASY_CVAR_EXTERN_DEBUGONLY(drawDebugPathfinding2)

extern DLL_GLOBAL int g_iSkillLevel;

#define newPathDelayDuration 0.5;

//TODO - same auto jumpoff mechanic for getting foes off my back as the Mr. Friendly does?



int g_panthereye_crouch_to_jump_sequenceID = -1;
int g_panthereye_walk_sequenceID = -1;
int g_panthereye_run_sequenceID = -1;




#if REMOVE_ORIGINAL_NAMES != 1
	LINK_ENTITY_TO_CLASS( monster_panthereye, CPantherEye );
#endif

#if EXTRA_NAMES > 0
	LINK_ENTITY_TO_CLASS( panthereye, CPantherEye );
	
	#if EXTRA_NAMES == 2
		LINK_ENTITY_TO_CLASS( panther, CPantherEye );
	#endif
#endif



//=========================================================
// monster-specific schedule types
//=========================================================
enum
{
	SCHED_PANTHEREYE_GENERIC_FAIL = LAST_COMMON_SCHEDULE + 1,
	SCHED_PANTHEREYE_CHASE_ENEMY,
	SCHED_PANTHEREYE_SNEAK_TO_LOCATION,
	SCHED_PANTHEREYE_JUMP_AT_ENEMY_UNSTUCK,
	SCHED_PANTHEREYE_COVER_FAIL,
	//follow?
	

};

//=========================================================
// monster-specific tasks
//=========================================================
enum 
{
	TASK_PANTHEREYE_PLAYANIM_GETBUG = LAST_COMMON_TASK + 1,
	TASK_PANTHEREYE_PLAYANIM_CROUCHTOJUMP,
	TASK_PANTHEREYE_PLAYANIM_CROUCHTOJUMP_UNSTUCK,
	TASK_PANTHEREYE_SNEAK_WAIT,
	TASK_PANTHER_NORMALMOVE,
	TASK_PANTHEREYE_FIND_COVER_FROM_ENEMY,
	TASK_PANTHEREYE_COVER_FAIL_WAIT,
	TASK_PANTHEREYE_DETERMINE_UNSTUCK_IDEAL,
	TASK_PANTHEREYE_ATTEMPT_ROUTE,
	TASK_PANTHEREYE_USE_ROUTE,

};




/*
Task_t	tlGruntTakeCoverFromBestSound[] =
{
	{ TASK_SET_FAIL_SCHEDULE,			(float)SCHED_COWER			},// duck and cover if cannot move from explosion
	{ TASK_STOP_MOVING,					(float)0					},
	{ TASK_FIND_COVER_FROM_BEST_SOUND,	(float)0					},
	{ TASK_RUN_PATH,					(float)0					},
	{ TASK_WAIT_FOR_MOVEMENT,			(float)0					},
	{ TASK_REMEMBER,					(float)bits_MEMORY_INCOVER	},
	{ TASK_TURN_LEFT,					(float)179					},
};

Schedule_t	slGruntTakeCoverFromBestSound[] =
{
	{ 
		tlGruntTakeCoverFromBestSound,
		ARRAYSIZE ( tlGruntTakeCoverFromBestSound ), 
		0,
		0,
		"GruntTakeCoverFromBestSound"
	},
};
*/


//isn't "panthereyeSneakToLocation" based off of this?  Just simpler.  ah well.
Task_t	tlPanthereyeTakeCoverFromEnemy[] =
{
	{ TASK_SET_FAIL_SCHEDULE,	(float)SCHED_PANTHEREYE_COVER_FAIL	},
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_WAIT,					(float)0.2					},
	{ TASK_FIND_COVER_FROM_ENEMY,	(float)0					},
	{ TASK_RUN_PATH,				(float)0					},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0					},
	{ TASK_REMEMBER,				(float)bits_MEMORY_INCOVER	},
//	{ TASK_TURN_LEFT,				(float)179					},
	{ TASK_FACE_ENEMY,				(float)0					},
	{ TASK_WAIT,					(float)1					},
};

Schedule_t	slPanthereyeTakeCoverFromEnemy[] =
{
	{ 
		tlPanthereyeTakeCoverFromEnemy,
		ARRAYSIZE ( tlPanthereyeTakeCoverFromEnemy ), 
		bits_COND_NEW_ENEMY |
		//MODDD CRITICAL - now interruptable by heavy damage.  May or may not be a good thing.
		bits_COND_HEAVY_DAMAGE,
		0,
		"PanthereyeTakeCoverFromEnemy"
	},
};



//=========================================================
// move away from where you're currently standing. 
//=========================================================
Task_t	tlPanthereyeTakeCoverFromOrigin[] =
{
	{ TASK_SET_FAIL_SCHEDULE,	(float)SCHED_PANTHEREYE_COVER_FAIL	},
	{ TASK_STOP_MOVING,					(float)0					},
	{ TASK_FIND_COVER_FROM_ORIGIN,		(float)0					},
	{ TASK_RUN_PATH,					(float)0					},
	{ TASK_WAIT_FOR_MOVEMENT,			(float)0					},
	{ TASK_REMEMBER,					(float)bits_MEMORY_INCOVER	},
	{ TASK_TURN_LEFT,					(float)179					},
};

Schedule_t	slPanthereyeTakeCoverFromOrigin[] =
{
	{ 
		tlPanthereyeTakeCoverFromOrigin,
		ARRAYSIZE ( tlPanthereyeTakeCoverFromOrigin ), 
		bits_COND_NEW_ENEMY |
		//MODDD CRITICAL - now interruptable by heavy damage.  May or may not be a good thing.
		bits_COND_HEAVY_DAMAGE,
		0,
		"PanthereyeTakeCoverFromOrigin"
	},
};
//=========================================================
// hide from the loudest sound source
//=========================================================
Task_t	tlPanthereyeTakeCoverFromBestSound[] =
{
	{ TASK_SET_FAIL_SCHEDULE,	(float)SCHED_PANTHEREYE_COVER_FAIL	},
	{ TASK_STOP_MOVING,					(float)0					},
	{ TASK_FIND_COVER_FROM_BEST_SOUND,	(float)0					},
	{ TASK_RUN_PATH,					(float)0					},
	{ TASK_WAIT_FOR_MOVEMENT,			(float)0					},
	{ TASK_REMEMBER,					(float)bits_MEMORY_INCOVER	},
	{ TASK_TURN_LEFT,					(float)179					},
};

Schedule_t	slPanthereyeTakeCoverFromBestSound[] =
{
	{ 
		tlPanthereyeTakeCoverFromBestSound,
		ARRAYSIZE ( tlPanthereyeTakeCoverFromBestSound ), 
		bits_COND_NEW_ENEMY |
		//MODDD CRITICAL - now interruptable by heavy damage.  May or may not be a good thing.
		bits_COND_HEAVY_DAMAGE,
		0,
		"PanthereyeTakeCoverFromBestSound"
	},
};




//MODDD NOTE - funny, the "TASK_PANTHEREYE_FIND_COVER_FROM_ENEMY" has no call for "TaskCompletion", so anything after "TASK_PANTHEREYE_FIND_COVER_FROM_ENEMY" never happens.
//Perhaps this rule of interruption is ok?
//MODDD NOTE UPDATE - NO. Look at the "panthereye_findCoverFromEnemy" method. It calls "TaskComplete" if it determines a path. So what comes after
//                    TASK_PANTHEREYE_FIND_COVER_FROM_ENEMY still runs and the schedule finishes normally.
Task_t	tlPanthereyeSneakToLocation[] =
{
	{ TASK_SET_FAIL_SCHEDULE,	(float)SCHED_PANTHEREYE_COVER_FAIL	},
	{ TASK_STOP_MOVING,			0				},
	//{ TASK_WAIT,				(float)0.2					},
	{ TASK_PANTHEREYE_FIND_COVER_FROM_ENEMY,	(float)0					},
	{ TASK_RUN_PATH,				(float)0					},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0					},
	{ TASK_PANTHEREYE_SNEAK_WAIT,	(float)0					},
	
};

Schedule_t	slPanthereyeSneakToLocation[] =
{
	{
		tlPanthereyeSneakToLocation,
		ARRAYSIZE ( tlPanthereyeSneakToLocation ),
		//TODO: much more!  sound_  stuff too.
		bits_COND_ENEMY_DEAD |
		bits_COND_CAN_RANGE_ATTACK1	|
		bits_COND_CAN_MELEE_ATTACK1	|
		bits_COND_CAN_RANGE_ATTACK2	|
		bits_COND_CAN_MELEE_ATTACK2 |
		bits_COND_LIGHT_DAMAGE  |
		bits_COND_HEAVY_DAMAGE,

		bits_SOUND_DANGER,
		"Panther_sneakToLocation"
	},
};



//Go straight to sneakwait. Has its uses.
Task_t	tlPanthereyeSneakWait[] =
{
	{ TASK_UPDATE_LKP,	(float)0					},
	{ TASK_PANTHEREYE_SNEAK_WAIT,	(float)0					},
	
};

Schedule_t	slPanthereyeSneakWait[] =
{
	{
		tlPanthereyeSneakWait,
		ARRAYSIZE ( tlPanthereyeSneakWait ),
		//TODO: much more!  sound_  stuff too.
		bits_COND_ENEMY_DEAD |
		bits_COND_CAN_RANGE_ATTACK1	|
		bits_COND_CAN_MELEE_ATTACK1	|
		bits_COND_CAN_RANGE_ATTACK2	|
		bits_COND_CAN_MELEE_ATTACK2 |
		bits_COND_LIGHT_DAMAGE  |
		bits_COND_HEAVY_DAMAGE,

		bits_SOUND_DANGER,
		"Panther_sneakWait"
	},
};






Task_t	tlPanthereyeJumpAtEnemy[] =
{
	{ TASK_SET_FAIL_SCHEDULE,	(float)SCHED_PANTHEREYE_GENERIC_FAIL	},
	{ TASK_STOP_MOVING,			0				},
	//{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	//{ TASK_WAIT,				(float)0.4		},
	{ TASK_FACE_IDEAL,			(float)25		},   //set before going to this sched
	{ TASK_PANTHEREYE_PLAYANIM_CROUCHTOJUMP,			(float)0		},  //includes wait for anim finish

};

Schedule_t	slPanthereyeJumpAtEnemy[] =
{
	{
		tlPanthereyeJumpAtEnemy,
		ARRAYSIZE ( tlPanthereyeJumpAtEnemy ),
		//TODO: much more!  sound_  stuff too.
		0,
		0,
		"Panther_JumpAtEnemy"
	},
};




// Very similar to the chaseenemy schedule, but if this fail
Task_t tlPanthereyeJumpAtEnemyUnstuck_PreCheck[] = 
{
	// If this fails, unstuck jump time
	{ TASK_SET_FAIL_SCHEDULE,	(float)SCHED_PANTHEREYE_JUMP_AT_ENEMY_UNSTUCK	},
	{ TASK_STOP_MOVING,			0				},

	//!!!
	// TASK_SET_ACTIVITY_PLAIN_IDLE  ???   pick only the simplest idle to avoid looking flinchy if
	// planning on changing from that sequence soon.
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },

	{ TASK_WAIT_FACE_ENEMY,				(float)1.8		},

	// is this ok?
	{TASK_PANTHEREYE_ATTEMPT_ROUTE, (float)60			},
	// assuming success if the schedule didn't fail at ATTEMPT_ROUTE. Use that schedule then.
	// This also includes setting the fail schedule back to generic, since failing at any other
	// point might not necessarily mean being stuck
	{TASK_PANTHEREYE_USE_ROUTE, (float)60			},
	{TASK_CHECK_STUMPED, (float)0					},
};

Schedule_t slPanthereyeJumpAtEnemyUnstuck_PreCheck[] =
{
	{ 
		tlPanthereyeJumpAtEnemyUnstuck_PreCheck,
		ARRAYSIZE ( tlPanthereyeJumpAtEnemyUnstuck_PreCheck ),
		bits_COND_NEW_ENEMY			|
		//MODDD - added, the bullsquid counts this.  Why doesn't everything?
		bits_COND_ENEMY_DEAD |

		bits_COND_CAN_RANGE_ATTACK1	|
		bits_COND_CAN_MELEE_ATTACK1	|
		bits_COND_CAN_RANGE_ATTACK2	|
		bits_COND_CAN_MELEE_ATTACK2	|
		bits_COND_TASK_FAILED		|
		bits_COND_HEAR_SOUND |
		//bits_COND_LIGHT_DAMAGE  |
		bits_COND_HEAVY_DAMAGE,
		
		bits_SOUND_DANGER,
		"Panther_JumpAtEnemy_unstuck_PC"
	},
};








Task_t	tlPanthereyeJumpUnstuck[] =
{
	{ TASK_SET_FAIL_SCHEDULE,	(float)SCHED_PANTHEREYE_GENERIC_FAIL	},
	{ TASK_STOP_MOVING,			0				},

	//{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	//{ TASK_WAIT,				(float)1.3		},

	{ TASK_PANTHEREYE_DETERMINE_UNSTUCK_IDEAL,  0},

	// Why was there a 30 for the option here?  TASK_FACE_IDEAL doesn't look at that.
	// Maybe it was intended to be tolerance?  That would be best for a new task at this point.
	// Oh.  In here, TASK_FACE_IDEAL is overridden to check for that.  30 still wasn't one of the
	// values though.
	{ TASK_FACE_IDEAL,			(float)0		},   //set before going to this sched...
	{ TASK_PANTHEREYE_PLAYANIM_CROUCHTOJUMP_UNSTUCK,			(float)0		},  //includes wait for anim finish

};

Schedule_t	slPanthereyeJumpAtEnemyUnstuck[] =
{
	{
		tlPanthereyeJumpUnstuck,
		ARRAYSIZE ( tlPanthereyeJumpUnstuck ),
		0,
		0,
		"Panther_JumpAtEnemy_unstuck"
	},
};








// Chase enemy schedule
Task_t tlPanthereyeChaseEnemy[] = 
{
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE},   //MODDD is this okay?

	// If this fails, move a random point away
	{ TASK_SET_FAIL_SCHEDULE,	(float)SCHED_MOVE_FROM_ORIGIN	},
	//{ TASK_GET_PATH_TO_ENEMY,	(float)0		},
	//{ TASK_RUN_PATH,			(float)0		},
	//{ TASK_WAIT_FOR_MOVEMENT,	(float)0		},
	
	// is this ok?
	{TASK_MOVE_TO_ENEMY_RANGE, (float)60			},
	{TASK_CHECK_STUMPED, (float)0					},
};

Schedule_t slPanthereyeChaseEnemy[] =
{
	{ 
		tlPanthereyeChaseEnemy,
		ARRAYSIZE ( tlPanthereyeChaseEnemy ),
		bits_COND_NEW_ENEMY			|
		//MODDD - added, the bullsquid counts this.  Why doesn't everything?
		bits_COND_ENEMY_DEAD |

		bits_COND_CAN_RANGE_ATTACK1	|
		bits_COND_CAN_MELEE_ATTACK1	|
		bits_COND_CAN_RANGE_ATTACK2	|
		bits_COND_CAN_MELEE_ATTACK2	|
		bits_COND_TASK_FAILED		|
		bits_COND_HEAR_SOUND |
		//bits_COND_LIGHT_DAMAGE  |
		bits_COND_HEAVY_DAMAGE,
		
		bits_SOUND_DANGER,
		"Panthereye Chase Enemy"
	},
};




// Chase enemy schedule
Task_t tlPanthereyeGetIntoCirclingRange[] = 
{
	{ TASK_SET_FAIL_SCHEDULE,	(float)SCHED_PANTHEREYE_GENERIC_FAIL	},
	{ TASK_PANTHER_NORMALMOVE,	(float)260		},
	{ TASK_RUN_PATH,			(float)0		},
	{ TASK_WAIT_FOR_MOVEMENT,	(float)0		},
};

Schedule_t slPanthereyeGetIntoCirclingRange[] =
{
	{ 
		tlPanthereyeGetIntoCirclingRange,
		ARRAYSIZE ( tlPanthereyeGetIntoCirclingRange ),
		bits_COND_NEW_ENEMY			|
		//MODDD - added, the bullsquid counts this.  Why doesn't everything?
		bits_COND_ENEMY_DEAD |

		bits_COND_CAN_RANGE_ATTACK1	|
		bits_COND_CAN_MELEE_ATTACK1	|
		bits_COND_CAN_RANGE_ATTACK2	|
		bits_COND_CAN_MELEE_ATTACK2	|
		bits_COND_TASK_FAILED		|
		bits_COND_HEAR_SOUND	|
		bits_COND_LIGHT_DAMAGE  |
		bits_COND_HEAVY_DAMAGE,
		
		bits_SOUND_DANGER,
		"Panthereye Getintocircl"
	},
};




Task_t	tlPanthereyeGetBug[] =
{
	{ TASK_SET_FAIL_SCHEDULE,	(float)SCHED_PANTHEREYE_GENERIC_FAIL	},
	{ TASK_STOP_MOVING,			0				},
	//{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	//{ TASK_WAIT,				(float)0.4		},
	{ TASK_FACE_IDEAL,			(float)24		},   //set before going to this sched...
	{ TASK_PANTHEREYE_PLAYANIM_GETBUG,			(float)0		},  //includes wait for anim finish

};

Schedule_t	slPanthereyeGetBug[] =
{
	{
		tlPanthereyeGetBug,
		ARRAYSIZE ( tlPanthereyeGetBug ),
		//TODO: much more!  sound_  stuff too.
		bits_COND_CAN_ATTACK | 
		bits_COND_HEAVY_DAMAGE,
		0,
		"Panther_getbug"
	},
};



Task_t	tlPantherEyeCoverFail[] =
{
	{ TASK_PANTHEREYE_COVER_FAIL_WAIT,			0				},
};

Schedule_t	slPantherEyeCoverFail[] =
{
	{
		tlPantherEyeCoverFail,
		ARRAYSIZE ( tlPantherEyeCoverFail ),
		bits_COND_CAN_ATTACK | 
		bits_COND_HEAVY_DAMAGE,
		0,
		"Panther_coverFail"
	},
};



Task_t	tlPantherEyeGenericFail[] =
{
	{ TASK_SET_FAIL_SCHEDULE,	(float)SCHED_PANTHEREYE_GENERIC_FAIL	},
	{ TASK_STOP_MOVING,			0				},
	//{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_WAIT,				(float)0.4		},
	{ TASK_WAIT_PVS,			(float)0		},
	{ TASK_UPDATE_LKP, (float)0		},

	// No, only do this onfailing pathfinding methods.
	// Possible to lead to an endless loop (not crashing, just conceptually) of failing some other schedule,
	// pikcing MOVE_FROM_ORIGIN, failing that, which leads to GenericFail, which leads to MOVE_FROM_ORIGIN,
	// which fails again, etc.  GetSchedule never gets called during any of that, so any change in conditions
	// elsewhere never has a chance to take effect (no attacking something in plain sight)
	//{ TASK_SET_SCHEDULE,			(float)SCHED_MOVE_FROM_ORIGIN },
};

Schedule_t	slPantherEyeGenericFail[] =
{
	{
		tlPantherEyeGenericFail,
		ARRAYSIZE ( tlPantherEyeGenericFail ),
		bits_COND_HEAVY_DAMAGE,
		0,
		"Panther_genFail"
	},
};




DEFINE_CUSTOM_SCHEDULES( CPantherEye )
{
	//slPanthereye_follow,
	//slHAssaultGenericFail,
	slPantherEyeGenericFail,
	slPanthereyeGetBug,
	slPanthereyeJumpAtEnemy,
	slPanthereyeJumpAtEnemyUnstuck,
	slPanthereyeJumpAtEnemyUnstuck_PreCheck,
	slPanthereyeChaseEnemy,

	slPanthereyeGetIntoCirclingRange,
	slPanthereyeSneakWait,
	slPanthereyeSneakToLocation,
	//slPanthereyeFlee,
	slPanthereyeTakeCoverFromOrigin,
	slPanthereyeTakeCoverFromBestSound,
	slPanthereyeTakeCoverFromEnemy,
	slPantherEyeCoverFail

};

IMPLEMENT_CUSTOM_SCHEDULES( CPantherEye, CBaseMonster );






const char *CPantherEye::pAttackHitSounds[] = 
{
	"zombie/claw_strike1.wav",
	"zombie/claw_strike2.wav",
	"zombie/claw_strike3.wav",
};

const char *CPantherEye::pAttackMissSounds[] = 
{
	"zombie/claw_miss1.wav",
	"zombie/claw_miss2.wav",
};

const char *CPantherEye::pAttackSounds[] = 
{
	"panthereye/pa_attack1.wav"
};

const char *CPantherEye::pIdleSounds[] = 
{
	"panthereye/pa_idle1.wav",
	"panthereye/pa_idle2.wav",
	"panthereye/pa_idle3.wav",
	"panthereye/pa_idle4.wav",

};

const char *CPantherEye::pAlertSounds[] = 
{
	"zombie/zo_alert10.wav",
	"zombie/zo_alert20.wav",
	"zombie/zo_alert30.wav",
};

const char *CPantherEye::pPainSounds[] = 
{
	"zombie/zo_pain1.wav",
	"zombie/zo_pain2.wav",
};

const char *CPantherEye::pLeapAttackHitSounds[] = 
{
	//for now, clone of zombie strike sounds.
	"zombie/claw_strike1.wav",
	"zombie/claw_strike2.wav",
	"zombie/claw_strike3.wav",
};

const char *CPantherEye::pDeathSounds[] = 
{
	"panthereye/pa_death1.wav",
};


TYPEDESCRIPTION	CPantherEye::m_SaveData[] = 
{
	DEFINE_FIELD(CPantherEye, sneakMode, FIELD_INTEGER),
	DEFINE_FIELD(CPantherEye, pissedRunTime, FIELD_TIME)

};
//IMPLEMENT_SAVERESTORE( CPantherEye, CBaseMonster );
int CPantherEye::Save( CSave &save )
{
	if ( !CBaseMonster::Save(save) )
		return 0;
	return save.WriteFields( "CPantherEye", this, m_SaveData, ARRAYSIZE(m_SaveData) );
}
int CPantherEye::Restore( CRestore &restore )
{
	if ( !CBaseMonster::Restore(restore) )
		return 0;
	return restore.ReadFields( "CPantherEye", this, m_SaveData, ARRAYSIZE(m_SaveData) );
}












float CPantherEye::HearingSensitivity(void){
	return 2.2f;
}//END OF HearingSensitivity






BOOL CPantherEye::testLeapNoBlock(void){

	//jump!... or try to.
	TraceResult tr;

	if(m_hEnemy != NULL){
		// was 10 and 10
		Vector vTestStart = pev->origin + Vector(0, 0, 30);
		Vector vTestEnd = m_vecEnemyLKP + Vector(0, 0, 30);
		
		/*
		//DEBUG - player pos?
		edict_t *pPlayer = FIND_CLIENT_IN_PVS( edict() );
		CBaseEntity* someEnt = NULL;
		if(pPlayer != NULL){
			someEnt = CBaseEntity::Instance(pPlayer);
		}else{
			return FALSE;
		}
		Vector vTestEnd = someEnt->pev->origin + Vector(0, 0, 10);
		
		//vTestEnd = vTestStart + ((vTestEnd - vTestStart).Normalize() * (vTestEnd - vTestStart).Length() * 0.5);
		*/

		//old way?
		//UTIL_TraceHull( vTestStart, vTestEnd, dont_ignore_monsters, head_hull, ENT( m_hEnemy->pev ), &tr );
				
		
		//TRACE_MONSTER_HULL(edict(), vTestStart, vTestEnd, dont_ignore_monsters, m_hEnemy->edict(), &tr);

		//First paramter is the entity to use for the hull size. 5th parameter is the entity to ignore.
		//May be tempting to make it the enemy, but don't. Make it the same entity (me) checking against another and just
		//allow a collision with that target entity if it happens.
		TRACE_MONSTER_HULL(edict(), vTestStart, vTestEnd, dont_ignore_monsters, edict(), &tr);
		//is this a better check for seeing if jumping is still okay?

		if(EASY_CVAR_GET_DEBUGONLY(drawDebugPathfinding2) == 1)DebugLine_Setup(0, vTestStart, vTestEnd, tr.flFraction);

		//don't do the "!tr.fAllSolid" check.  ?
		//and !tr.fStartSolid ?  Why is that always true even if on the ground and the origin is bumped up? ugh.

		const char* hitName;
		if(tr.pHit != NULL){
			hitName = STRING(tr.pHit->v.classname);
		}else{
			hitName = "NULL";
		}
		//easyForcePrintLine(hitName);

		//NOTE - if relation R_NO with what was hit is ever included for satisfying this, be sure to exclude what's hit by a "worldspawn" classnamecheck,
		//       or more completely, as an entity, IsWorld and IsWorldAffiliated.

		//if(  tr.flFraction >= 1.0 || (tr.pHit != NULL && (tr.pHit == m_hEnemy.Get() || this->IRelationship( CBaseEntity::Instance(tr.pHit) ) > R_NO))    ){
		if(traceResultObstructionValidForAttack(tr)){
			//did not hit anything on the way or hit the enemy? or something I would have attacked anyways? it is OK.
			//proceed
		}else{
			// Wait, one more try
			Vector vTestStart = pev->origin + Vector(0, 0, 80);
			Vector vTestEnd = m_vecEnemyLKP + Vector(0, 0, 40);
			TRACE_MONSTER_HULL(edict(), vTestStart, vTestEnd, dont_ignore_monsters, edict(), &tr);
			if(traceResultObstructionValidForAttack(tr)){
				// ok
			}else{
				// failed too??
				return FALSE;
			}
		}
	}else{
		//go ahead regardless.?
	}

	return TRUE;
}//END OF testLeapNoBlock




// don't care about the enemy, just see if jumping forward is possible
BOOL CPantherEye::testLeapNoBlock_Forward(void){

	//jump!... or try to.
	TraceResult tr;


	Vector vTestStart = pev->origin + Vector(0, 0, 10);

	//Vector vTestEnd = m_vecEnemyLKP + Vector(0, 0, 10);

	::UTIL_MakeAimVectors(this->pev->angles);
	Vector vTestEnd = Center() + gpGlobals->v_forward * 400; //RANDOM_FLOAT(360, 530);


	/*
	//DEBUG - player pos?
	edict_t *pPlayer = FIND_CLIENT_IN_PVS( edict() );
	CBaseEntity* someEnt = NULL;
	if(pPlayer != NULL){
	someEnt = CBaseEntity::Instance(pPlayer);
	}else{
	return FALSE;
	}
	Vector vTestEnd = someEnt->pev->origin + Vector(0, 0, 10);

	//vTestEnd = vTestStart + ((vTestEnd - vTestStart).Normalize() * (vTestEnd - vTestStart).Length() * 0.5);
	*/

	//old way?
	//UTIL_TraceHull( vTestStart, vTestEnd, dont_ignore_monsters, head_hull, ENT( m_hEnemy->pev ), &tr );


	//TRACE_MONSTER_HULL(edict(), vTestStart, vTestEnd, dont_ignore_monsters, m_hEnemy->edict(), &tr);

	//First paramter is the entity to use for the hull size. 5th parameter is the entity to ignore.
	//May be tempting to make it the enemy, but don't. Make it the same entity (me) checking against another and just
	//allow a collision with that target entity if it happens.
	TRACE_MONSTER_HULL(edict(), vTestStart, vTestEnd, dont_ignore_monsters, edict(), &tr);
	//is this a better check for seeing if jumping is still okay?

	if(EASY_CVAR_GET_DEBUGONLY(drawDebugPathfinding2) == 1)DebugLine_Setup(0, vTestStart, vTestEnd, tr.flFraction);

	//don't do the "!tr.fAllSolid" check.  ?
	//and !tr.fStartSolid ?  Why is that always true even if on the ground and the origin is bumped up? ugh.

	const char* hitName;
	if(tr.pHit != NULL){
		hitName = STRING(tr.pHit->v.classname);
	}else{
		hitName = "NULL";
	}
	//easyForcePrintLine(hitName);

	//NOTE - if relation R_NO with what was hit is ever included for satisfying this, be sure to exclude what's hit by a "worldspawn" classnamecheck,
	//       or more completely, as an entity, IsWorld and IsWorldAffiliated.

	//if(  tr.flFraction >= 1.0 || (tr.pHit != NULL && (tr.pHit == m_hEnemy.Get() || this->IRelationship( CBaseEntity::Instance(tr.pHit) ) > R_NO))    ){
	if(traceResultObstructionValidForAttack(tr)){
		//did not hit anything on the way or hit the enemy? or something I would have attacked anyways? it is OK.
		//proceed
	}else{
		//stop!
		return FALSE;
	}
	

	return TRUE;
}//END OF testLeapNoBlock_Forward



int CPantherEye::IRelationship( CBaseEntity *pTarget )
{
	/*
	if(pTarget->isProvokable() && !pTarget->isProvoked() ){
		//I have no reason to pick a fight with this unprovoked, neutral enemy.
		return R_NO;
	}
	*/

	//DOOMMARINE23 Quick fix so Panthereyes ally towards instead of hate, other Panthereyes
	if ( FClassnameIs( pTarget->pev, "monster_panthereye" ) )
	{
		return R_AL;
	}
	// END DOOOMMARINE23

	if(!pTarget->isForceHated(this) && pTarget->Classify() == CLASS_ALIEN_PREY){
		//don't hate, just "Dislike" instead.
		return R_DL;
	}



	return CBaseMonster::IRelationship( pTarget );
}








void CPantherEye::panthereye_findCoverFromEnemy( void ){
	
	entvars_t *pevCover;

	if ( m_hEnemy == NULL )
	{
		// Find cover from self if no enemy available
		pevCover = pev;
//				TaskFail();
//				return;
	}
	else
		pevCover = m_hEnemy->pev;
			

	if ( FindLateralCover( pevCover->origin, pevCover->view_ofs ) )
	{
		// try lateral first
		m_flMoveWaitFinished = gpGlobals->time + findCoverTaskDataMem;
		TaskComplete();
		EASY_CVAR_PRINTIF_PRE(panthereyePrintout, easyPrintLine("YOU coward2"));
	}
	else if ( FindCover( pevCover->origin, pevCover->view_ofs, 0, CoverRadius() ) )
	{
		// then try for plain ole cover
		m_flMoveWaitFinished = gpGlobals->time + findCoverTaskDataMem;
		TaskComplete();
		EASY_CVAR_PRINTIF_PRE(panthereyePrintout, easyPrintLine("YOU coward3"));
	}
	else
	{
				
//let's uh... let's not.
		/*
		// no coverwhatsoever.
				
		//MODDD - how do we handle failing to find cover?
				
		if( m_hEnemy != NULL && UTIL_IsFacing(m_hEnemy->pev, pev->origin, 0.2) && (m_vecEnemyLKP - pev->origin).Length() < 240 ){
			//go aggro!
			pissedOffTime = gpGlobals->time + 5;
			ChangeSchedule(GetSchedule());
			//timeTillSneakAgain = gpGlobals->time + RANDOM_LONG(7, 16);
		}

		if( m_hEnemy != NULL && UTIL_IsFacing(m_hEnemy->pev, pev->origin, 0.2) && (m_vecEnemyLKP - pev->origin).Length() < 240 ){
			//go aggro!
			pissedOffTime = gpGlobals->time + 5;
			ChangeSchedule(GetSchedule());
			return;
			//timeTillSneakAgain = gpGlobals->time + RANDOM_LONG(7, 16);
		}else{
			//try sneaking again sooner?  Be easier to piss off since couldn't update cover...  Being cornered makes most things easier to piss off.
			isPissable = TRUE;
			timeTillSneakAgain = gpGlobals->time + RANDOM_LONG(4, 8);
		}
		*/
		EASY_CVAR_PRINTIF_PRE(panthereyePrintout, easyPrintLine("YOU 4"));
		RouteClear();
		TaskFail();

	}

}




//=========================================================
// Classify - indicates this monster's place in the 
// relationship table.
//=========================================================
//Note that predators attack other predators on sight.  Panthers will attack other panthers.  Change class to "CLASS_ALIEN_MONSTER" to still make it hostile to non-aliens (to still be hostile to "Prey", may need to make a new one)
int CPantherEye::Classify ( void )
{
	return	CLASS_ALIEN_PREDATOR;
}

//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CPantherEye::SetYawSpeed ( void )
{
	int ys;

	//nimble, turn fast.  (default is 120?)
	ys = 200;

#if 0
	switch ( m_Activity )
	{
	}
#endif

	pev->yaw_speed = ys;
}




GENERATE_TRACEATTACK_IMPLEMENTATION(CPantherEye)
{

	if(flDamage > 1){
		// ow!
		pissedRunTime = gpGlobals->time + 12;
		if(pev->sequence == g_panthereye_walk_sequenceID){
			// change it
			SetActivity(ACT_RUN);
		}
	}

	GENERATE_TRACEATTACK_PARENT_CALL(CBaseMonster);
}

GENERATE_TAKEDAMAGE_IMPLEMENTATION(CPantherEye)
{

	return GENERATE_TAKEDAMAGE_PARENT_CALL(CBaseMonster);
}







//I will.  Or the issue is fixed now? probably.
BOOL CPantherEye::forceIdleFrameReset(void){
	return FALSE;
}


BOOL CPantherEye::usesAdvancedAnimSystem(void){
	return TRUE;
}




int CPantherEye::LookupActivityHard(int activity){
	int i = 0;
	int iRandChoice = 0;
	int iRandWeightChoice = 0;
	
	char* animChoiceString = NULL;
	int* weightsAbs = NULL;
	//pev->framerate = 1;
	int maxRandWeight = 30;

	

	m_flFramerateSuggestion = 1;
	m_flFrameRate = 1;
	pev->framerate = 1;
	//is this safe?

	/*
	m_flFramerateSuggestion = -1;
	//pev->frame = 6;
	return LookupSequence("get_bug");
	*/

	//any animation events in progress?  Clear it.
	resetEventQueue();

	//EASY_CVAR_PRINTIF_PRE(panthereyePrintout, easyPrintLine("AHH %d", m_fSequenceFinished));

	//no need for default, just falls back to the normal activity lookup.
	switch(activity){
		
		case ACT_WALK:
			// nothing special then
			return CBaseAnimating::LookupActivity(activity);
		break;
		case ACT_IDLE:

			iRandChoice = -1;
			if(sneakMode == -1){
				
				maxRandWeight = 10;
				weightsAbs = new int[2];
				weightsAbs[0] = 7;
				weightsAbs[1] = 10;

			
				//1 to highest possible (sum of all weights).
				iRandWeightChoice = RANDOM_LONG(1, maxRandWeight);
			
				//What range did we strike?

				for(i = 0; i < 2; i++){
					if(iRandWeightChoice <= weightsAbs[i]){
						iRandChoice = i;
						break;
					}
				}
				delete[] weightsAbs;

				//"get_bug" may make more sense when there is a cockroach nearby.
			
				//iRandChoice = 2;
				switch(iRandChoice){
					case 0:
						m_flFramerateSuggestion = 1.1f;
						animChoiceString = "itch";
					break;
					case 1:
						m_flFramerateSuggestion = 1;
						animChoiceString = "shakes";
					break;
				}


			}else{
				//sneakery...

				
				maxRandWeight = 10;
				weightsAbs = new int[2];
				weightsAbs[0] = 7;
				weightsAbs[1] = 10;

			
				//1 to highest possible (sum of all weights).
				iRandWeightChoice = RANDOM_LONG(1, maxRandWeight);
			
				//What range did we strike?

				for(i = 0; i < 2; i++){
					if(iRandWeightChoice <= weightsAbs[i]){
						iRandChoice = i;
						break;
					}
				}
				delete[] weightsAbs;

				//"get_bug" may make more sense when there is a cockroach nearby.
			
				//iRandChoice = 2;
				switch(iRandChoice){
					case 0:
						animChoiceString = "subtle_motion";
					break;
					case 1:
						animChoiceString = "idle_figit";
					break;
				}
			}//END OF else OF sneak check


			
			//animChoiceString = "subtle_motion";

			if(animChoiceString != NULL){
				EASY_CVAR_PRINTIF_PRE(panthereyePrintout, easyPrintLine("PANTHER: getIdleAnim: %s", animChoiceString));
				return LookupSequence(animChoiceString);
			}else{
				EASY_CVAR_PRINTIF_PRE(panthereyePrintout, easyPrintLine("PANTHER: getIdleAnim FAILED!!!"));
			}


		break;
		case ACT_MELEE_ATTACK1:



			if(g_iSkillLevel == SKILL_EASY){
				m_flFramerateSuggestion = 1.22;
			}else if(g_iSkillLevel == SKILL_MEDIUM){
				m_flFramerateSuggestion = 1.3;
			}else if(g_iSkillLevel == SKILL_HARD){
				m_flFramerateSuggestion = 1.45;
			}


			//randomize...
			iRandChoice = RANDOM_LONG(0, 2);
			switch(iRandChoice){
				case 0:
					
					//rise, hard-left hit, slight right hit.
					//ALSO, artificial anim event: the 2nd hand hit does not come with a model event.
					
					//EASY_CVAR_PRINTIF_PRE(panthereyePrintout, easyPrintLine("PRIMARY START!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! %.2f", gpGlobals->time));
					//m_flFramerateSuggestion = 0.2f;
					animEventQueuePush(12.0f/15.0f, 0);

					return LookupSequence("attack_primary");
				break;
				case 1:
					//slightly longer right-swipe.
					return LookupSequence("attack_main_claw");
				break;
				case 2:
					//slightly faster left-swipe.
					return LookupSequence("attack_simple_claw");
				break;
			}
		break;
		
		case ACT_MELEE_ATTACK2:
			m_flFramerateSuggestion = 0.95f;

			//jump with the delay.
			//animEventQueuePush(10.35f/(15.0f), 2 );
			
			return g_panthereye_crouch_to_jump_sequenceID;
		break;
		case ACT_SMALL_FLINCH:
		case ACT_BIG_FLINCH:
			if(g_iSkillLevel == SKILL_EASY){
				m_flFramerateSuggestion = 1.0;
			}else if(g_iSkillLevel == SKILL_MEDIUM){
				m_flFramerateSuggestion = 1.6;
			}else if(g_iSkillLevel == SKILL_HARD){
				m_flFramerateSuggestion = 2.0;
			}
			return CBaseAnimating::LookupActivity(activity);
		break;
		case ACT_DIEVIOLENT:
			return LookupSequence("death_violent");
		break;
		case ACT_TURN_LEFT:
			m_flFramerateSuggestion = 2.1f;
			return CBaseAnimating::LookupActivity(activity);
		break;
		case ACT_TURN_RIGHT:
			m_flFramerateSuggestion = 2.1f;
			return CBaseAnimating::LookupActivity(activity);
		break;
		case ACT_RUN:{

			if(m_IdealMonsterState == MONSTERSTATE_SCRIPT){
				m_flFramerateSuggestion = 1.0;
				pev->framerate = 1.0;
				return g_panthereye_run_sequenceID;
			}else{
				if(gpGlobals->time < pissedRunTime){
					if(g_iSkillLevel == SKILL_EASY){
						m_flFramerateSuggestion = 1.00;
					}else if(g_iSkillLevel == SKILL_MEDIUM){
						m_flFramerateSuggestion = 1.05;
					}else if(g_iSkillLevel == SKILL_HARD){
						m_flFramerateSuggestion = 1.10;
					}
					pev->framerate = m_flFramerateSuggestion;
					return g_panthereye_run_sequenceID;
				}else{
					// not pissed?  power-walk it
					if(g_iSkillLevel == SKILL_HARD){
						m_flFramerateSuggestion = 1.19;
					}else if(g_iSkillLevel == SKILL_MEDIUM){
						m_flFramerateSuggestion = 1.13;
					}else{
						m_flFramerateSuggestion = 1.08;
					}
					pev->framerate = m_flFramerateSuggestion;
					return g_panthereye_walk_sequenceID;
				}
			}
		}break;

	}//swithc(activity)
	
	//not handled by above?  try the real deal.
	return CBaseAnimating::LookupActivity(activity);
}


int CPantherEye::tryActivitySubstitute(int activity){
	//EASY_CVAR_PRINTIF_PRE(panthereyePrintout, easyPrintLine("AHH %d", m_fSequenceFinished));

	//no need for default, just falls back to the normal activity lookup.
	switch(activity){
		
		case ACT_WALK:
			// just to say we have something, I forget if the model has ACT_WALK but may as well say we do.
			return CBaseAnimating::LookupActivity(ACT_RUN);
		break;
		case ACT_IDLE:
			return LookupSequence("subtle_motion");
		break;
		case ACT_MELEE_ATTACK1:
			//just say we have something, good enough.
			return LookupSequence("attack_primary");
		break;
		case ACT_MELEE_ATTACK2:
			//Don't trust the model for this.
			return g_panthereye_crouch_to_jump_sequenceID;
		break;
		case ACT_SMALL_FLINCH:
			return CBaseAnimating::LookupActivity(activity);
		break;
		case ACT_BIG_FLINCH:
			return CBaseAnimating::LookupActivity(activity);
		break;
		case ACT_DIEVIOLENT:
			return LookupSequence("death_violent");
		break;
		case ACT_TURN_LEFT:
			//gets it right, but add framerate:
			//...other version of lookup.  HARD
			return CBaseAnimating::LookupActivity(activity);
		break;
		case ACT_TURN_RIGHT:
			return CBaseAnimating::LookupActivity(activity);
		break;

	}
	
	//not handled by above?
	return CBaseAnimating::LookupActivity(activity);
}


void CPantherEye::PainSound( void )
{
//	int pitch = 95 + RANDOM_LONG(0,9);

//	if (RANDOM_LONG(0,5) < 2)
//		UTIL_PlaySound( ENT(pev), CHAN_VOICE, pPainSounds[ RANDOM_LONG(0,ARRAYSIZE(pPainSounds)-1) ], 1.0, ATTN_NORM, 0, pitch );
}

void CPantherEye::AlertSound( void )
{
//	int pitch = 95 + RANDOM_LONG(0,9);

//	UTIL_PlaySound( ENT(pev), CHAN_VOICE, pAlertSounds[ RANDOM_LONG(0,ARRAYSIZE(pAlertSounds)-1) ], 1.0, ATTN_NORM, 0, pitch );
}

void CPantherEye::IdleSound( void )
{
	int pitch = 95 + RANDOM_LONG(0,9);
	// Play a random idle sound
	// less attn, carries further
	UTIL_PlaySound( ENT(pev), CHAN_VOICE, pIdleSounds[ RANDOM_LONG(0,ARRAYSIZE(pIdleSounds)-1) ], 1.0, ATTN_NORM - 0.13, 0, 100 + RANDOM_LONG(-5,5) );
}

void CPantherEye::AttackSound( void )
{
	// Play a random attack sound
	UTIL_PlaySound( ENT(pev), CHAN_VOICE, pAttackSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );
}





void CPantherEye::HandleEventQueueEvent(int arg_eventID){
	

	switch(arg_eventID){
		case 0:{
			// do stuff for this event.
	//		ALERT( at_console, "Slash right!\n" );

				//half-damage, the second swipe is a less pronounced claw swipe.
			CBaseEntity *pHurt = CheckTraceHullAttack( 84*1.2, gSkillData.panthereyeDmgClaw/2, DMG_BLEEDING );

			// hold on.  Is the thing I intended to hit close?  Go ahead and hit that anyway, that was the intent.

			if ( pHurt )
			{
				if ( (pHurt->pev->flags & (FL_MONSTER|FL_CLIENT)) && !pHurt->blocksImpact() )
				{
					pHurt->pev->punchangle.z = -18;
					pHurt->pev->punchangle.x = 5;
					pHurt->pev->velocity = pHurt->pev->velocity - gpGlobals->v_right * 100;
				}
				
				if(pHurt->isOrganic() || pHurt->IsWorldOrAffiliated() ){
					// Play a random attack hit sound
					UTIL_PlaySound( ENT(pev), CHAN_WEAPON, pAttackHitSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackHitSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );
				}else{
					
					playMetallicHitSound(CHAN_WEAPON, 0.71f);
				}

			}
			else // Play a random attack miss sound
				UTIL_PlaySound( ENT(pev), CHAN_WEAPON, pAttackMissSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackMissSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );

			if (RANDOM_LONG(0,1))
				AttackSound();


		}break;
		case 1:{
			//
			UTIL_MakeAimVectors(pev->angles);

			//....
			Vector vecForwardFrom = pev->origin + gpGlobals->v_forward * 40;
						
			Vector vecSearchStart = vecForwardFrom;
						
			//Vector vecSearchStart = UTIL_getFloor(vecForwardFrom, 300, ignore_monsters, ENT(pev));

			//if(isErrorVector(vecSearchStart)){
			//	//no floor? just use the area...
			//	vecSearchStart = vecForwardFrom - (0, 0, 5);
			//}
			
			CBaseEntity* pEntityScan = NULL;
			while ( (pEntityScan = UTIL_FindEntityInSphere( pEntityScan, vecSearchStart, 40 ) ) != NULL)
			{

				//MySquadMonsterPointer ????
				CBaseMonster* testMon = pEntityScan->MyMonsterPointer();

				//could check for "classify" being "insect" too, if there were more insects?
				if(testMon != NULL && FClassnameIs(testMon->pev, "monster_cockroach")){
					//only one.
					testMon->TakeDamage( pev, pev, testMon->pev->health, DMG_CRUSH );
					break;
				};


			}

			bugAnimBlockedTime = gpGlobals->time + RANDOM_LONG(6, 15);

		}break;
		case 2:{

			
			JumpEvent(TRUE);

		}break;
		case 3:{
			// A jump, but I don't care about the enemy position
			
			JumpEvent(FALSE);

		}break;
	}//END OF switch(arg_eventID)

}


// the actual jump
void CPantherEye::JumpEvent(BOOL enemyAccurate){
	
	BOOL isJumpOkay;
	
	if(enemyAccurate){
		isJumpOkay = testLeapNoBlock();
	}else{
		isJumpOkay = testLeapNoBlock_Forward();
	}

	//In any case this counts for setting the leap cooldown. Don't even try again for a little if this attempt is
	//suddenly blocked and interrupted.

	if(g_iSkillLevel == SKILL_HARD){
		leapAttackCooldown = gpGlobals->time + RANDOM_FLOAT(2.8, 3.2);
	}else if(g_iSkillLevel == SKILL_MEDIUM){
		leapAttackCooldown = gpGlobals->time + RANDOM_FLOAT(3.4, 3.8);
	}else{ //easy?
		leapAttackCooldown = gpGlobals->time + RANDOM_FLOAT(4.3, 4.8);
	}


	// ahhhh screw it.  for now.
	isJumpOkay = TRUE;

	if(!isJumpOkay){
		TaskFail();
		//force a new animation to be fetched.
		m_Activity = ACT_RESET;
		this->SetActivity(ACT_IDLE);
		return;
	}

	// jumping puts me in a frenzy
	pissedRunTime = gpGlobals->time + 12;

	ClearBits( pev->flags, FL_ONGROUND );
	pev->groundentity = NULL;  //safety?

	UTIL_SetOrigin (pev, pev->origin + Vector ( 0 , 0 , 1) );// take him off ground so engine doesn't instantly reset onground 
	UTIL_MakeAimVectors ( pev->angles );

	Vector vecJumpDir;
	if (enemyAccurate && m_hEnemy != NULL)
	{
		float gravity = g_psv_gravity->value;
		
		// UHhhhh.  Is that wise, less than 1 gets interpreted as 1 anyway?  Doubt it?
		//if (gravity <= 1)
		//	gravity = 1;

		// How fast does the headcrab need to travel to reach that height given gravity?
		float height = (m_vecEnemyLKP.z + m_hEnemy->pev->view_ofs.z - pev->origin.z);

		float speedExtraMult = 1;
		//was a minimum of 15.  (was 11?)
		if (height < 18)
			height = 18;


		//MODDD - greater, was 40
		if(height > 55){
			//CRUDE. Take how much height was above 40 and let it add to speedExtraMult.
			speedExtraMult = 1 + (height - 55) / 55;
			height = 55;
		}

		//was 2 * gravity * height.
		float speed = sqrt( 1.7 * gravity * height * speedExtraMult);
		float time = speed / gravity;

		// Scale the sideways velocity to get there at the right time
		//???
		//vecJumpDir = (m_vecEnemyLKP + m_hEnemy->pev->view_ofs - pev->origin);

		//must jump at least "240" far...
		vecJumpDir = max( (m_vecEnemyLKP + m_hEnemy->pev->view_ofs - pev->origin).Length() - 80, 320 ) * gpGlobals->v_forward;
				
		vecJumpDir = vecJumpDir * ( 1.0 / time );

		// Speed to offset gravity at the desired height
		vecJumpDir.z = speed;

		// Don't jump too far/fast
		float distance = vecJumpDir.Length();
							
		//vecJumpDir = (distance -50) * vecJumpDir.Normalize();

		// I'm not entirely sure what this is doing...
		// OH, I see, forcing a dist of 650 exactly, factoring out the old value.  It's how vectors work.
		// UPPED, was 950.
		if (distance > 1100)
		{
			vecJumpDir = vecJumpDir * ( 1100.0 / distance );
		}

	}
	else
	{
		// jump hop, don't care where
		vecJumpDir = Vector( gpGlobals->v_forward.x, gpGlobals->v_forward.y, gpGlobals->v_up.z ) * 350;
	}

	int iSound = RANDOM_LONG(0,2);
	if ( iSound != 0 )
		UTIL_PlaySound( edict(), CHAN_VOICE, pAttackSounds[iSound], 1, ATTN_IDLE, 0, 100 );

	pev->velocity = vecJumpDir;

	SetTouch( &CPantherEye::LeapTouch );


}//JumpEvent








//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CPantherEye::HandleAnimEvent( MonsterEvent_t *pEvent )
{
	EASY_CVAR_PRINTIF_PRE(panthereyePrintout, easyPrintLine("BATTA-BING, BATTA-BOOM %d", pEvent->event));

	switch( pEvent->event )
	{
		case 1:
		{
			// do stuff for this event.
	//		ALERT( at_console, "Slash right!\n" );

			//TraceResult trRecord;

			CBaseEntity *pHurt = CheckTraceHullAttack( 84*1.2, gSkillData.panthereyeDmgClaw, DMG_BLEEDING); // &trRecord );
			if ( pHurt )
			{
				if ( (pHurt->pev->flags & (FL_MONSTER|FL_CLIENT)) && !pHurt->blocksImpact() )
				{
					pHurt->pev->punchangle.z = -18;
					pHurt->pev->punchangle.x = 5;
					pHurt->pev->velocity = pHurt->pev->velocity - gpGlobals->v_right * 100;
				}
				// Play a random attack hit sound
				if(pHurt->isOrganic() || pHurt->IsWorldOrAffiliated() ){
					//yes, if organic or the map. Because we have no way of seeing what plane on the map was hit for a texture sound from that.

					UTIL_PlaySound( ENT(pev), CHAN_WEAPON, pAttackHitSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackHitSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );
				}else{


					playMetallicHitSound(CHAN_WEAPON, 0.71f);




					/*
					//naa, ditch this system.  We can't reliably see what texture we hit.

					TraceResult trOK;
					Vector vecStart = trRecord.vecEndPos + trRecord.vecPlaneNormal * 5;
					Vector vecEnd = vecStart + trRecord.vecPlaneNormal * -80;
					UTIL_TraceLine(vecStart, vecEnd, dont_ignore_monsters, ENT(pev), &trOK);
					TEXTURETYPE_PlaySound(&trOK, vecStart, vecEnd, 0);

					DebugLine_Setup(1, vecStart, vecEnd, trOK.flFraction);
					*/

					/*
					::UTIL_MakeAimVectors(this->pev->angles);
					TraceResult tr;
					Vector vecStart = pev->origin + Vector(0, 0, 12);
					Vector vecEnd = vecStart + (gpGlobals->v_forward * (84*1.2) );

					
					UTIL_TraceLine(vecStart, vecEnd, dont_ignore_monsters, ENT(pev), &tr);
					//UTIL_TraceHull( vecStart, vecEnd, dont_ignore_monsters, head_hull, ENT(pev), &tr );
					//should we query what was hit for something more appropriate?

					const char* thingHit = tr.pHit!=NULL?STRING(tr.pHit->v.classname):"NULL";

					//BULLET_NONE     #include "weapons.h"
					TEXTURETYPE_PlaySound(&tr, vecStart, vecEnd, 0);
					*/

				}

			}
			else // Play a random attack miss sound
				UTIL_PlaySound( ENT(pev), CHAN_WEAPON, pAttackMissSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackMissSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );

			if (RANDOM_LONG(0,1))
				AttackSound();
		}
		break;

		default:
			CBaseMonster::HandleAnimEvent( pEvent );
			break;
	}
}

CPantherEye::CPantherEye(void){

	stareTime = 0;
	leapAttackCooldown = 0;

	maxWaitPVSTime = -1;
	chaseMode = -1;
	m_flFramerateSuggestion = 1;
	bugAnimBlockedTime = -1;
	sneakMode = -1;
	newPathDelay = -1;

	
	runawayTime = -1;
	pissedOffTime = -1;

	timeTillSneakAgain = -1;
	isPissable = FALSE;
	isCornered = FALSE;
	isEnemyLookingAtMe = FALSE;

	waitingForNewPath = FALSE;

	findCoverTaskDataMem = -1;

	pissedRunTime = -1;

}


void CPantherEye::setModel(void){
	CPantherEye::setModel(NULL);
}
void CPantherEye::setModel(const char* m){
	CBaseMonster::setModel(m);

	if (g_panthereye_crouch_to_jump_sequenceID == -1) {
		g_panthereye_crouch_to_jump_sequenceID = LookupSequence("crouch_to_jump");
		g_panthereye_walk_sequenceID = LookupSequence("walk");
		g_panthereye_run_sequenceID = LookupSequence("run");
		
	}
}



//=========================================================
// Spawn
//=========================================================
void CPantherEye::Spawn()
{
	Precache( );

	setModel("models/panthereye.mdl");
	// size made a little more snug.
	UTIL_SetSize( pev, Vector(-28, -28, 0), Vector(28, 28, 36) );
	//UTIL_SetSize( pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX );

	pev->classname = MAKE_STRING("monster_panthereye");

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_GREEN;
	pev->health		= gSkillData.panthereyeHealth;

	//MODDD - NOTE: can "view_ofs" just come from a model?  Verify by printout maybe?


	//pev->view_ofs		= VEC_VIEW;// position of the eyes relative to monster's origin.
	m_flFieldOfView		= 0.5;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState		= MONSTERSTATE_NONE;
	m_afCapability		= bits_CAP_DOORS_GROUP;

	//MODDD - force this. Apparently it's not happening otherwise if we don't have an activity mapped to MELEE_ATTACK2 or whatever.
	m_afCapability |= bits_CAP_RANGE_ATTACK2;

	MonsterInit();

	//start out not sneaking.
	sneakMode = -1;
	
}


void CPantherEye::SetEyePosition(void){
	pev->view_ofs = VEC_VIEW;
}//END OF SetEyePosition





int CPantherEye::ISoundMask ( void )
{
	return	bits_SOUND_WORLD	|
		bits_SOUND_COMBAT	|
		bits_SOUND_PLAYER	|
		//bits_SOUND_DANGER	|
		//MODDD - new
		bits_SOUND_BAIT;
}


int CPantherEye::IgnoreConditions ( void ){

	if(m_pSchedule == slMoveFromOrigin && !HasConditionsMod(bits_COND_COULD_MELEE_ATTACK1)){
		// if it isn't the case I could range_attack1 from facing a different direction, don't bother with sound
		// during this schedule.
		return (CBaseMonster::IgnoreConditions() | bits_COND_HEAR_SOUND);
	}

	return CBaseMonster::IgnoreConditions();
}




extern int global_useSentenceSave;
//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CPantherEye::Precache()
{
	int i;

	PRECACHE_MODEL("models/panthereye.mdl");

	global_useSentenceSave = TRUE;
	
	PRECACHE_SOUND_ARRAY(pAttackHitSounds);
	PRECACHE_SOUND_ARRAY(pAttackMissSounds);
	PRECACHE_SOUND_ARRAY(pAttackSounds);
	PRECACHE_SOUND_ARRAY(pIdleSounds);
	PRECACHE_SOUND_ARRAY(pAlertSounds);
	PRECACHE_SOUND_ARRAY(pPainSounds);
	PRECACHE_SOUND_ARRAY(pLeapAttackHitSounds);
	PRECACHE_SOUND_ARRAY(pDeathSounds);
	
	global_useSentenceSave = FALSE;
}	

//=========================================================
// AI Schedules Specific to this monster
//=========================================================



void CPantherEye::DeathSound ( void )
{
	int pitch = randomValueInt(96, 102);

	UTIL_PlaySound( edict(), CHAN_VOICE, RANDOM_SOUND_ARRAY(pDeathSounds), 1, ATTN_IDLE, 0, pitch );
		
}




BOOL CPantherEye::FCanCheckAttacks ( void )
{
	// allow without the enemy in sight
	/*
	if ( HasConditions(bits_COND_SEE_ENEMY) && !HasConditions( bits_COND_ENEMY_TOOFAR ) )
	{
		return TRUE;
	}
	*/

	if ( !HasConditions( bits_COND_ENEMY_TOOFAR ) )
	{
		return TRUE;
	}

	return FALSE;
}



BOOL CPantherEye::CheckMeleeAttack1 ( float flDot, float flDist )
{
	/*
	if(m_hEnemy != NULL){
		EASY_CVAR_PRINTIF_PRE(panthereyePrintout, easyPrintLine("SO:::%d, %.2f %.2f %d", !HasConditions( bits_COND_ENEMY_OCCLUDED ), flDist, flDot, m_hEnemy ->Classify() ));
	}else{
		EASY_CVAR_PRINTIF_PRE(panthereyePrintout, easyPrintLine("NO ENEMY"));
	}
	*/

	if(m_hEnemy == NULL){
		return FALSE;  // ???
	}

	float zDist;
	//zDist = (m_hEnemy->Center().z - this->Center().z);
	if(m_vecEnemyLKP.z > pev->absmax.z){
		// If the other monster's z is above the top of my bounding box, the z dist will be from the top of my bounds to there
		zDist = fabs(m_vecEnemyLKP.z - pev->absmax.z);
	}else if(m_vecEnemyLKP.z < pev->absmin.z){
		// If it is under the absmin, take that difference instead
		zDist = fabs(pev->absmin.z - m_vecEnemyLKP.z);
	}else{
		// the enemy origin isn't outside the bounds?  ok, a 'pass' all the same
		zDist = 0;
	}

	// zdist was 40
	if ( !HasConditions( bits_COND_ENEMY_OCCLUDED ) && flDist <= 80 && zDist < 32 && 
	 m_hEnemy != NULL &&
	 m_hEnemy ->Classify() != CLASS_ALIEN_BIOWEAPON &&
	 m_hEnemy ->Classify() != CLASS_PLAYER_BIOWEAPON   )
	{
		TraceResult tr;
		Vector vecStart = Center();
		// One more check, is there a straight line from me to them?
		UTIL_TraceLine(vecStart, m_hEnemy->BodyTargetMod(vecStart), dont_ignore_monsters, edict(), &tr);

		if(tr.pHit == NULL){
			// ok?
		}else{

			/*
			CBaseEntity* theHit = CBaseEntity::Instance(tr.pHit);
			if(theHit != NULL){
				int theRel = IRelationship(theHit);
				if(tr.pHit == m_hEnemy->edict() || (theRel > R_NO || theRel == R_FR) ){
					// if whatever I hit is the enemy, or I hate it anyway, proceed
				}else{
					// hmm.
					easyForcePrintLine("BLOCKED LIKE HOW");
					return FALSE;
				}
			}
			*/
		}

		// also need to be looking closely enough.
		if(flDot >= 0.7 && HasConditions(bits_COND_SEE_ENEMY)){
			return TRUE;
		}else{
			SetConditionsMod(bits_COND_COULD_MELEE_ATTACK1);
		}

	}
	return FALSE;
}



BOOL CPantherEye::CheckMeleeAttack2 ( float flDot, float flDist )
{
	/*
	if(m_hEnemy != NULL){
		EASY_CVAR_PRINTIF_PRE(panthereyePrintout, easyPrintLine("SO:::%d, %.2f %.2f %d", !HasConditions( bits_COND_ENEMY_OCCLUDED ), flDist, flDot, m_hEnemy ->Classify() ));
	}else{
		EASY_CVAR_PRINTIF_PRE(panthereyePrintout, easyPrintLine("NO ENEMY"));
	}
	*/
	float enemyFloorZ;

	if(gpGlobals->time >= leapAttackCooldown){
		//it is ok to do the leap attack.
	}else{
		//not enough time passed since the last one.
		return FALSE;
	}
	
	/*
	if(m_hEnemy != NULL){
		enemyFloorZ = m_vecEnemyLKP.z + m_hEnemy->pev->mins.z;
	}
	*/
	
	float zDist;
	//zDist = (m_hEnemy->Center().z - this->Center().z);
	if(m_vecEnemyLKP.z > pev->absmax.z){
		// If the other monster's z is above the top of my bounding box, the z dist will be from the top of my bounds to there
		zDist = fabs(m_vecEnemyLKP.z - pev->absmax.z);
	}else if(m_vecEnemyLKP.z < pev->absmin.z){
		// If it is under the absmin, take that difference instead
		zDist = fabs(pev->absmin.z - m_vecEnemyLKP.z);
	}else{
		// the enemy origin isn't outside the bounds?  ok, a 'pass' all the same
		zDist = 0;
	}






	//MODDD - STOP ME LATER
	//flDist = 300;


	// laxxed bounds, was 270 to 410.
	if ( m_hEnemy != NULL &&
		!HasConditions( bits_COND_ENEMY_OCCLUDED ) && flDist >= 180 && flDist <= 850 &&
		zDist < 180 &&     //not too much veritcal difference allowed, this is for a long floor-wise leap.
	 m_hEnemy ->Classify() != CLASS_ALIEN_BIOWEAPON &&
	 m_hEnemy ->Classify() != CLASS_PLAYER_BIOWEAPON   )
	{
		//lastly, is the path unobstructed?
		
		BOOL isJumpOkay = testLeapNoBlock();
		if(!isJumpOkay)return FALSE;

		//also need to be looking closely enough.
		if(flDot >= 0.91 && HasConditions(bits_COND_SEE_ENEMY)){
			return TRUE;
		}else{
			// could have jumped if facing the right way, mark this!
			SetConditionsMod(bits_COND_COULD_MELEE_ATTACK2);
		}

	}

	return FALSE;
}






BOOL CPantherEye::hasSeeEnemyFix(void){
	return FALSE;
}


//Based off of headcrab's "LeapTouch".  Not a method from CBaseMonster.
void CPantherEye::LeapTouch ( CBaseEntity *pOther )
{
	float velocityLength;
	if ( !pOther->pev->takedamage || pOther->MyMonsterPointer() == NULL)
	{
		//incapabable of taking damage or not a monster? I can't damage this.
		return;
	}

	//int classifyVal = pOther->Classify();
	int relationshipVal = IRelationship( pOther );
	if ( pOther != m_hEnemy && (relationshipVal == R_NO || relationshipVal == R_AL) )
	{
		//can't hurt neutrals or ally's by accident with this.
		return;
	}

	// Don't hit if back on ground
	// But beware, looks like FL_ONGROUND doesn't get set (on) while in that MOVETYPE (toss?) for jumping and slidnig a bit.
	// Checking for a minimum velocity may be nice to help.
	// Nope, do full 3D for this.
	velocityLength = pev->velocity.Length();

	// was 180
	if(velocityLength < 50){
		//too slow, don't count.
		return;
	}

	if ( !FBitSet( pev->flags, FL_ONGROUND ) )
	{
		float damageMulti;
		//generic attack sound?
		UTIL_PlaySound( edict(), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pLeapAttackHitSounds), 1, ATTN_IDLE, 0, 100 );
		
		//Increase damage beyond default?  Probably not.?
		// let's let the damage depend on the velocity.  Full damage for over 600, goes down towards the minimum.
		//The factor must be between 0.30 and 1.25.
		damageMulti = max( (min(velocityLength, 900.0f) / 900.0f) * 1.30f, 0.25f);

		pOther->TakeDamage( pev, pev, gSkillData.panthereyeDmgClaw * damageMulti, DMG_SLASH, 0 );
	}

	SetTouch( NULL );
}





void CPantherEye::MonsterThink ( void )
{

	if(pev->deadflag == DEAD_NO){
		//ok?
		//return;


		if(m_pSchedule == slMoveFromOrigin){
			if(HasConditionsMod(bits_COND_COULD_MELEE_ATTACK2)){
				// stop!  Do that.
				m_failSchedule = SCHED_MELEE_ATTACK2;
				TaskFail();
			}
		}



		if(newPathDelay != -1){
			if(newPathDelay <= gpGlobals->time){
				//-1 is a symbol that means, no longer waiting.
				newPathDelay = -1;
			}
		}

		//RIPPED FROM hassassin.

		//m_Activity == ACT_RUN || m_Activity == ACT_WALK 
		if (EASY_CVAR_GET_DEBUGONLY(panthereyeHasCloakingAbility) <= 0 || m_hEnemy == NULL || pev->deadflag != DEAD_NO || !(pev->flags & FL_ONGROUND) || sneakMode == -1 || (m_vecEnemyLKP - pev->origin).Length() < 170 )
			m_iTargetRanderamt = 255;
		else{
			//m_iTargetRanderamt = 20;
			m_iTargetRanderamt = min(EASY_CVAR_GET_DEBUGONLY(panthereyeHasCloakingAbility), 1) * 255;
		}
		if (pev->renderamt > m_iTargetRanderamt)
		{
			pev->renderamt = max( pev->renderamt - 50, m_iTargetRanderamt );
			pev->rendermode = kRenderTransTexture;
		}
		else if (pev->renderamt < m_iTargetRanderamt)
		{
			pev->renderamt = min( pev->renderamt + 50, m_iTargetRanderamt );
			if (pev->renderamt == 255)
				pev->rendermode = kRenderNormal;
		}

	
		EASY_CVAR_PRINTIF_PRE(panthereyePrintout, easyPrintLine("PANTHERREPORT: SCHED: %s TASK: %d SEE-EN: %d CAN_ME: %d CAPABLE: %d PISSA: %d CORN: %d", tryGetScheduleName(), tryGetTaskID(), HasConditions(bits_COND_SEE_ENEMY), HasConditions(bits_COND_CAN_MELEE_ATTACK1), m_afCapability & bits_CAP_MELEE_ATTACK1, isPissable, isCornered  ));
		
		//MODDD - TODO: how about another CVar to check that we're not in the middle of real combat (getting distracted by bugs while getting shot it is a bit odd).  While chasing alone, having not been fired at and/or damaged isn't as bad though.
		if (pev->deadflag == DEAD_NO && m_IdealActivity == ACT_IDLE && m_pSchedule != slPanthereyeGetBug && bugAnimBlockedTime <= gpGlobals->time ) //do this stuff while alive
		{
			//does this get every roach on the map or anything?    Just one "roach" if I understand right?   Can that work?
			//(I'm thinking check for ents in a radius every-so-often, maybe half-a-second?).
		
			CBaseEntity* pEntityScan = NULL;
			while ( (pEntityScan = UTIL_FindEntityInSphere( pEntityScan, pev->origin, 68 ) ) != NULL)
			{

				//MySquadMonsterPointer ????
				CBaseMonster* testMon = pEntityScan->MyMonsterPointer();

				//could check for "classify" being "insect" too, if there were more insects?
				if(testMon != NULL && FClassnameIs(testMon->pev, "monster_cockroach")){
					MakeIdealYaw(testMon->pev->origin);
					ChangeSchedule(slPanthereyeGetBug);

				};

			}
		}
	}//END OF dead check


	CBaseMonster::MonsterThink();

}//MonsterThink



void CPantherEye::PrescheduleThink(void){

	// not working?... oops?
	if(m_afSoundTypes & (bits_SOUND_COMBAT | bits_SOUND_PLAYER) ){
		// Hey!
		pissedRunTime = gpGlobals->time + 12;
		if(pev->sequence == g_panthereye_walk_sequenceID){
			// change it
			SetActivity(ACT_RUN);
		}
	}

	if(m_hEnemy != NULL){
		//pissedRunTime != -1 &&
		if(pissedRunTime != -1 && gpGlobals->time >= pissedRunTime){
			// Not pissed?  If I don't see the enemy, drop it.
			// FUCKING
			if(!HasConditions(bits_COND_SEE_ENEMY) && !HasConditions(bits_COND_NEW_ENEMY)){
				pissedRunTime = -1;
				ForgetEnemy();
				CBaseMonster::PrescheduleThink();
				return;
			}
		}

		if(gpGlobals->time < pissedRunTime){
			if(HasConditions(bits_COND_SEE_ENEMY)){
				// keep the frenzy going
				pissedRunTime = gpGlobals->time + 12;
			}
		}else{
			float daDistah = Distance(pev->origin, m_vecEnemyLKP);
			if(daDistah < 500){
				// in my territory?  FRENZY
				pissedRunTime = gpGlobals->time + 12;
			}
		}
	}//m_hEnemy check



	//MODDD - chunk moved from RunTask to here in PreSchedule think!
	//////////////////////////////////////////////////////////////////////////////////////////
	isEnemyLookingAtMe = FALSE;

	//NOTE: should only do this logic check if alive of course.
	// How about on the ground too?
	if(pev->deadflag == DEAD_NO && pev->flags & FL_ONGROUND && pev->sequence != g_panthereye_crouch_to_jump_sequenceID){
		if(pissedOffTime == -1 && runawayTime == -1 && m_hEnemy != NULL){
			//nothing all that strong, and an enemy about?  Sneak time!
			sneakMode = 0;
		}else{
			sneakMode = -1;
		}

		if(m_hEnemy != NULL){
			float distanceFromEnemy = (m_vecEnemyLKP - pev->origin).Length();

			BOOL pissOff = FALSE;
			BOOL pissOffHard = FALSE;
			BOOL tryRunAway = FALSE;

			//how far does the player have to be to look at me to piss me off?
			float lookAtMePissRange = 115 * 20;

			//how close does the player have to be, regardless of anything, to piss me off?
			float distancePissRange = 70 * 20;

			//if hurt (took damage), how far do I have to be to go on the attack instead of flee?
			float fightOrFlightRange = 400 * 20;


			if(isCornered){
				lookAtMePissRange = 340 * 20;
				distancePissRange = 230 * 20;
				//nowhere to go?  I'm coming for you.
				fightOrFlightRange = 1600 * 20;
			}else if(isPissable){
				//just more anxious than usual.
				lookAtMePissRange = 200 * 20;
				distancePissRange = 80 * 20;
				fightOrFlightRange = 500 * 20;
			}

			float extraSensitiveness =  1 + (1 - (pev->health / pev->max_health) ) * 0.6 ;

			lookAtMePissRange *= extraSensitiveness;
			distancePissRange *= distancePissRange;
			fightOrFlightRange *= fightOrFlightRange;
		

			if(HasConditions(bits_COND_SEE_ENEMY) ){

			}

			if(!HasConditions(bits_COND_ENEMY_OCCLUDED)  ){
				if(UTIL_IsFacing(m_hEnemy->pev, pev->origin, 0.3)){
					//if they face me anytime, face them.

					// NO. No gpGlobals->frametime, not good.
					// Interval between thinks is 0.1 (unless you have reason to think otherwise)
					//stareTime += gpGlobals->frametime;
					stareTime += 0.1;


					//they are.
					isEnemyLookingAtMe = TRUE;

					int tsknbr = getTaskNumber();
					EASY_CVAR_PRINTIF_PRE(panthereyePrintout, easyPrintLine("RUN RUN %s %d %d", m_pSchedule->pName, tsknbr, isCornered));
					if(  (isCornered || (m_pSchedule == slPanthereyeSneakToLocation || m_pSchedule == slPantherEyeCoverFail) && (tsknbr == TASK_PANTHEREYE_SNEAK_WAIT || tsknbr == TASK_PANTHEREYE_COVER_FAIL_WAIT) ) ){
						//likely waiting.
						MakeIdealYaw(m_vecEnemyLKP);

						//instant yaw change. is that a good idea?
						//ChangeYaw(99);
					}

				}else{
					stareTime = 0;
				}


				if(pissedOffTime == -1){

					//if the enemy is facing me, not behind a wall, clearly looking at me...
					if(UTIL_IsFacing(m_hEnemy->pev, pev->origin, 0.2)){
						//a lower distance can make it attack.
						if(distanceFromEnemy < lookAtMePissRange){
							pissOff = TRUE;
						}
					}else{
						//this close?  Just attack already.
						//EASY_CVAR_PRINTIF_PRE(panthereyePrintout, easyPrintLine("WELL DDDD %d", UTIL_IsFacingAway(m_hEnemy->pev, pev->origin, 0.3) )); 
						if(UTIL_IsFacingAway(m_hEnemy->pev, pev->origin, EASY_CVAR_GET_DEBUGONLY(panthereyeJumpDotTol)) ){
							//can jump...?
							if(distanceFromEnemy < 360 * 20){

								if(distanceFromEnemy < 130 * 20){
									//too close, just go as normal.
									pissOff = TRUE;
								}else{

									if(m_pSchedule != slPanthereyeJumpAtEnemy){
										//b/w 130 and 320?  JUMP
										RouteClear();
										ChangeSchedule(slPanthereyeJumpAtEnemy);
										//Best to stop this call here if we change schedules yes?
										return;
									}
								}
							}
						}else{
							if(distanceFromEnemy < distancePissRange){
								pissOff = TRUE;
							}
						}
					}
				}//END OF if(pissedOffTime == -1)
				else{
					if(!HasConditions(bits_COND_ENEMY_OCCLUDED) && HasConditions(bits_COND_SEE_ENEMY)){

						if(distanceFromEnemy < 300 * 20){
							//if mad, gets mad easier!
							pissOff = TRUE;
						}
					}
				}

			}else{
				//if occluded, reset stare time.
				stareTime = 0;
			}
			//EASY_CVAR_PRINTIF_PRE(panthereyePrintout, easyPrintLine("WHAT DA %d", HasConditions(bits_COND_LIGHT_DAMAGE)));
			//If attacked recently, be a bit more easy to attack, or flee.  "fight or flight"
			
			if(!pissOff && HasConditions(bits_COND_LIGHT_DAMAGE) || HasConditions(bits_COND_HEAVY_DAMAGE) ){
				if(distanceFromEnemy < fightOrFlightRange){
					pissOffHard = TRUE;
				}else{
					tryRunAway = TRUE;
				}
			}

			if(tryRunAway){
				BOOL startRunningAway = FALSE;
				if(runawayTime == -1){
					//initiate!
					startRunningAway = TRUE;
				}
				runawayTime = gpGlobals->time + 7;

				if(startRunningAway && pissedOffTime == -1){
					EASY_CVAR_PRINTIF_PRE(panthereyePrintout, easyPrintLine("!!!!! IM A STUPID <thing>"));
					ChangeSchedule(GetSchedule());
					return;  //let this be called freshly by the next frame of think logic.
				}
			}

			if(pissOffHard || pissOff){
				//longer!
				BOOL startRunningAway = FALSE;
				if(pissedOffTime == -1){
					//initiate!
					startRunningAway = TRUE;
				}
				if(pissOffHard){
					//isCornered
					if(pissedOffTime < gpGlobals->time + 8){
						pissedOffTime = gpGlobals->time + 8;
					}
				}else{
					//make it so.  Check so that it doesn't override "pissOffHard" pushing past 2 seconds.
					if(pissedOffTime < gpGlobals->time + 3){
						pissedOffTime = gpGlobals->time + 3;
					}
				}
				runawayTime = -1;

				if(startRunningAway){
					EASY_CVAR_PRINTIF_PRE(panthereyePrintout, easyPrintLine("!!!!! IM A RAGING <thing>"));
					ChangeSchedule(GetSchedule());
					return;  //let this be called freshly.
				}

			}
		}//END OF if(m_hEnemy != NULL)
		else{
			//no enemy?  Reset all timers.
			runawayTime = -1;
			pissedOffTime = -1;
			stareTime = 0;
		}

		if(runawayTime != -1 ){
			if(runawayTime <= gpGlobals->time){
				//end!
				runawayTime = -1;

				if(m_pSchedule == slTakeCoverFromBestSound){
					//TaskFail();
					EASY_CVAR_PRINTIF_PRE(panthereyePrintout, easyPrintLine("!!!!! IM NOT A STUPID <thing>"));
					ChangeSchedule(GetSchedule());
					return;  //let this be called freshly
				}

			}else{
				//still going.
				/*
				if(m_pSchedule != slTakeCoverFromBestSound){
					ChangeSchedule(slTakeCoverFromBestSound);
					return;  //let this be called freshly
				}
				*/
			}
		}

		if(pissedOffTime != -1){
			if(pissedOffTime <= gpGlobals->time){
				//end!
				pissedOffTime = -1;
				//too crass of an interruption, try something else...?

				//was CIRCLIN
				if(m_pSchedule == slPanthereyeChaseEnemy){
					//TaskFail();
					EASY_CVAR_PRINTIF_PRE(panthereyePrintout, easyPrintLine("!!!!! IM NOT A RAGING <thing>"));
					ChangeSchedule(GetSchedule());
					return;  //let this be called freshly
				}
			}else{
				//still going.

				/*
				if(m_pSchedule != slPanthereyeGetIntoCirclingRange){
					ChangeSchedule(slPanthereyeGetIntoCirclingRange);
					return;
				}
				*/
			}
		}


	}//END OF if(pev->deadflag == DEAD_NO &&
	else{

		//////easyForcePrintLine(".....???????? %d : %d   %d %d", monsterID, pTask->iTask, pev->deadflag, deadSetActivityBlock);
	}

	//////////////////////////////////////////////////////////////////////////////////////////


	CBaseMonster::PrescheduleThink();
}//PrescheduleThink



void CPantherEye::SetActivity ( Activity NewActivity )
{
	/*
	if(pev->deadflag != DEAD_NO &&
	*/

	// if in any dying state, and haven't picked a ACT_DIE-act, deny this.
	if(pev->deadflag != DEAD_NO &&
		NewActivity != ACT_DIESIMPLE &&
		NewActivity != ACT_DIEBACKWARD && 
		NewActivity != ACT_DIEFORWARD &&
		NewActivity != ACT_DIEVIOLENT 
	){
		//////easyForcePrintLine("OOOOOOOOOOOOOOOOOOOOOO freakin HOW %d :::%d %d %d", monsterID, NewActivity, pev->deadflag, m_MonsterState);
		return;
	}
	

	EASY_CVAR_PRINTIF_PRE(panthereyePrintout, easyPrintLine("PANTHER: SetAct: %d", NewActivity));

	int iSequence;

	//iSequence = LookupActivity ( NewActivity );
	iSequence = LookupActivityHard ( NewActivity );

	//only difference?  no need to have this copy over here then..

	// Set to the desired anim, or default anim if the desired is not present
	if ( iSequence > ACTIVITY_NOT_AVAILABLE )
	{
		//MODDD - added "forceReset"... NO, REVERTED.
		//if ( forceReset || (pev->sequence != iSequence || !m_fSequenceLoops) )
		if (  pev->sequence != iSequence || !m_fSequenceLoops )
		{
			// don't reset frame between walk and run
			if ( !(m_Activity == ACT_WALK || m_Activity == ACT_RUN) || !(NewActivity == ACT_WALK || NewActivity == ACT_RUN))
				resetFrame();
		}

		pev->sequence		= iSequence;	// Set to the reset anim (if it's there)

		//EASY_CVAR_PRINTIF_PRE(panthereyePrintout, easyPrintLine("NewActivity:%d : forceReset:%d",  NewActivity, forceReset ));
		ResetSequenceInfo( );

		//MODDD
		pev->framerate = m_flFramerateSuggestion;
		//should this always be reset?
		//m_flFramerateSuggestion = 1;

		SetYawSpeed();
	}
	else
	{
		// Not available try to get default anim
		ALERT ( at_aiconsole, "%s has no sequence for act:%d\n", STRING(pev->classname), NewActivity );
		
		pev->sequence		= 0;	// Set to the reset anim (if it's there)
	}

	m_Activity = NewActivity; // Go ahead and set this so it doesn't keep trying when the anim is not present
	
	// In case someone calls this with something other than the ideal activity
	m_IdealActivity = m_Activity;


	//CBaseMonster::SetActivity(NewActivity);
}


//based off of GetSchedule for CBaseMonster in schedule.cpp.
Schedule_t *CPantherEye::GetSchedule ( void )
{


	
	SetTouch( NULL );
	//Safety. In case of picking a new schedule, don't try the damage touch for throwing myself at an enemy, has to be set freshly.



	if(pev->deadflag != DEAD_NO){
		return GetScheduleOfType( SCHED_DIE );
	}
	if(pev->deadflag == DEAD_NO){
		//////easyForcePrintLine("IM GONNA eat YOUR WHOLE box of raisins");
	}


	//MODDD - experimental. Auto get the best sound then.
	SCHEDULE_TYPE baitSched = getHeardBaitSoundSchedule(); //getHeardBaitSoundSchedule(pSound);

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

			if(HasConditions(bits_COND_HEAVY_DAMAGE)){
				return GetScheduleOfType(SCHED_BIG_FLINCH);
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
				// clear the current (dead) enemy and try to find another.
				m_hEnemy = NULL;

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

			if ( HasConditions(bits_COND_NEW_ENEMY) )
			{
				return GetScheduleOfType ( SCHED_WAKE_ANGRY );
			}
			//MODDD - new.  Can do the big flinch.
			else if(HasConditions(bits_COND_HEAVY_DAMAGE)){
				return GetScheduleOfType(SCHED_BIG_FLINCH);
			}
			//MODDD - other condition.  If "noFlinchOnHard" is on and the skill is hard, don't flinch from getting hit.
			else if (HasConditions(bits_COND_LIGHT_DAMAGE) && !HasMemory( bits_MEMORY_FLINCHED) && !(EASY_CVAR_GET_DEBUGONLY(noFlinchOnHard)==1 && g_iSkillLevel==SKILL_HARD)  )
			{
				return GetScheduleOfType( SCHED_SMALL_FLINCH );
			}
			else if ( !HasConditions(bits_COND_SEE_ENEMY) )
			{


				if(HasConditionsMod(bits_COND_COULD_MELEE_ATTACK1 | bits_COND_COULD_MELEE_ATTACK2 | bits_COND_COULD_RANGE_ATTACK1 | bits_COND_COULD_RANGE_ATTACK2))
				{
					//turn
					return GetScheduleOfType( SCHED_COMBAT_FACE );
				}



				// we can't see the enemy
				if ( !HasConditions(bits_COND_ENEMY_OCCLUDED) )
				{
					/*
					if(!FacingIdeal()){
						// enemy is unseen, but not occluded!
						// turn to face enemy
						return GetScheduleOfType(SCHED_COMBAT_FACE);
					}else{
						//We're facing the LKP already. Then we have to go to that point and declare we're stumped there if we still see nothing.
						return GetScheduleOfType(SCHED_CHASE_ENEMY);
					}
					*/

					//better handling.
					return GetScheduleOfType( SCHED_PANTHEREYE_CHASE_ENEMY );

				}
				else
				{
					// chase!
					///EASY_CVAR_PRINTIF_PRE(panthereyePrintout, easyPrintLine("ducks??"));
					return GetScheduleOfType( SCHED_PANTHEREYE_CHASE_ENEMY );
				}
			}
			else
			{

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
				if ( !HasConditions(bits_COND_CAN_RANGE_ATTACK1 | bits_COND_CAN_MELEE_ATTACK1) )
				{
					// if we can see enemy but can't use either attack type, we must need to get closer to enemy
					//EASY_CVAR_PRINTIF_PRE(panthereyePrintout, easyPrintLine("ducks2"));
					return GetScheduleOfType( SCHED_PANTHEREYE_CHASE_ENEMY );
				}
				else if ( !FacingIdeal() )
				{

					// check this anyway
					if(HasConditionsMod(bits_COND_COULD_MELEE_ATTACK2)){
						return GetScheduleOfType( SCHED_COMBAT_FACE );
					}

					//turn
					//return GetScheduleOfType( SCHED_COMBAT_FACE );

					//better handling.
					return GetScheduleOfType( SCHED_PANTHEREYE_CHASE_ENEMY );
				}
				else
				{
					ALERT ( at_aiconsole, "No suitable combat schedule!\n" );
				}
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
				EASY_CVAR_PRINTIF_PRE(panthereyePrintout, easyPrintLine("WARNING: m_pCine IS NULL!"));
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

	EASY_CVAR_PRINTIF_PRE(panthereyePrintout, easyPrintLine("PANTHER: I AM A COMPLETE DISGRACE"));
	return &slError[ 0 ];
}



Schedule_t* CPantherEye::ChaseOrCircle(void){
	
	TraceResult rt;
	Vector enemyFloor = UTIL_getFloor(m_vecEnemyLKP, 800, ignore_monsters, ENT(m_hEnemy));

	float verticalDist = enemyFloor.z - pev->origin.z;
	if(isErrorVector(enemyFloor) || (verticalDist > 18) ){
		//we're chasing as usual.
		return slPanthereyeChaseEnemy;
	}else{
		//conditions good for circling.
		return slPanthereyeGetIntoCirclingRange;
	}

}

Schedule_t* CPantherEye::GetScheduleOfType( int Type){

	
	EASY_CVAR_PRINTIF_PRE(panthereyePrintout, easyPrintLine("PANTHER: GET SCHEDULE OF TYPE: %d", Type));

	
	if( (pev->deadflag != DEAD_NO) && Type != SCHED_DIE){
		//////easyForcePrintLine("I WILL eat YOUR WHOLE box of rasinssss %d: %d :::%d", monsterID, deadSetActivityBlock, Type);
	}



	switch(Type){
		/*
		case SCHED_COMBAT_FACE:

		break;
		*/
		case SCHED_PANTHEREYE_COVER_FAIL:
			return slPantherEyeCoverFail;
		break;
		case SCHED_PANTHEREYE_GENERIC_FAIL:
			return slPantherEyeGenericFail;
		break;
		

		case SCHED_PANTHEREYE_JUMP_AT_ENEMY_UNSTUCK:
			return slPanthereyeJumpAtEnemyUnstuck;
		break;

		case SCHED_PANTHEREYE_CHASE_ENEMY:

			EASY_CVAR_PRINTIF_PRE(panthereyePrintout, easyPrintLine("the TIME dmg: %d | %.2f %.2f",  HasConditions(bits_COND_LIGHT_DAMAGE), (timeDelayFilter(pissedOffTime) ), ( timeDelayFilter(runawayTime) ))); 
			if(pissedOffTime != -1){
				
				if(pissedOffTime <= gpGlobals->time){
					//end!
					
				}else{
					//still going.
					//return slPanthereyeGetIntoCirclingRange;
					//NO.
					return slPanthereyeChaseEnemy;
				}
				
			}
			
			if(runawayTime != -1 ){
				if(runawayTime <= gpGlobals->time){
					//end!
					
				}else{
					return GetScheduleOfType(SCHED_TAKE_COVER_FROM_ENEMY);
				}
			}

			//nothing from above?  We're sneaking around.
			return slPanthereyeSneakToLocation;
		break;

		//Don't stare into space for 15 seconds. Just sneakwait for being more reactive.
		case SCHED_PATHFIND_STUMPED:
			return &slPanthereyeSneakWait[0];
		break;


		case SCHED_MELEE_ATTACK2:
			// the MELEE_ATTACK_1 sched is okay. #2 needs to use a custom schedule to
			// (be able to) turn while prepping the jump.

			MakeIdealYaw(m_vecEnemyLKP);
			return &slPanthereyeJumpAtEnemy[0];
		break;
		case SCHED_FAIL:
		case SCHED_CHASE_ENEMY_FAILED:

			if(m_pSchedule == slMoveFromOrigin && getTaskNumber() == TASK_MOVE_FROM_ORIGIN){
				// That's what failed?  Try to turn and face our enemy in case jumping at them is possible.
				if(m_hEnemy != NULL){
					// Only if the enemy is in a line of sight to me
					if(FVisible(m_hEnemy->Center())){
						// Can see them?  Go ahead
						setEnemyLKP(m_hEnemy);
						MakeIdealYaw(m_vecEnemyLKP);
						SetTurnActivity();
					}else{
						pev->ideal_yaw = pev->angles.y;
					}
				}else{
					// where I'm already facing?
					pev->ideal_yaw = pev->angles.y;
				}
				// Wait a second or so before doing the jump in case a pathfind check after that time
				// works or the enemy gets closer to become attackable.
				// If still unrouteable, the unstuck jump starts.

				// HACK: reset instantly.  Not required for the unstuck jump to work but
				// could make leaping toward the enemy to get 'unstuck' more likely, seems
				// odd to miss that opportunity.

				this->leapAttackCooldown = gpGlobals->time + 0;

				return &slPanthereyeJumpAtEnemyUnstuck_PreCheck[0];
			}

			// what is usually called on failure.  Can call something else if preferred.
			return &slPantherEyeGenericFail[ 0 ];
		break;
	}

	return CBaseMonster::GetScheduleOfType(Type);
}




BOOL CPantherEye::needsMovementBoundFix(void){
	return TRUE;
}



void CPantherEye::MoveExecute( CBaseEntity *pTargetEnt, const Vector &vecDir, float flInterval )
{
	// !!!!!!!!!!!!!!
	//return;

	/*
	if(this->m_pSchedule == slPanthereyeJumpAtEnemy){
		// dont do it.  not that this should be necessary?
		return;
	}
	*/

	//otherwise just do what the parent class does, it works.
	CBaseMonster::MoveExecute(pTargetEnt, vecDir, flInterval);
	
}//END OF MoveExecute







void CPantherEye::StartTask ( Task_t *pTask ){

	//new task?  If we were waiting, no more.
	waitingForNewPath = FALSE;

	//any new task resets this.
	isPissable = FALSE;
	isCornered = FALSE;

	switch ( pTask->iTask ){
		
		case TASK_PANTHEREYE_ATTEMPT_ROUTE:
			// Really the first part of MOVE_TO_ENEMY_RANGE:  make the route, but complete if it passes
			pTask->iTask = TASK_MOVE_TO_ENEMY_RANGE;
			CBaseMonster::StartTask(pTask);

			if(FRouteClear()){
				// No route?  Assume it failed
				TaskFail();
			}else{
				// success?  Also ditch the 'jump unstuck' fail schedule then
				m_failSchedule = SCHED_PANTHEREYE_GENERIC_FAIL;
				// MOVE_TO_ENEMY_RANGE would have set this.
				goalDistTolerance = pTask->flData;
				TaskComplete();
			}

		break;
		case TASK_PANTHEREYE_USE_ROUTE:{
			// same as MOVE_TO_ENEMY_RANGE, but doesn't start by making a route (assuming that came
			// from the previous task succeeding)
			// Do nothing here, leave the rest to RunTask.

		}break;
		case TASK_PANTHEREYE_DETERMINE_UNSTUCK_IDEAL:{
			int i;
			int alternator;
			TraceResult tr;
			Vector vecStart;
			Vector vecEnd;
			Vector vecAnglesTest(pev->angles.x, pev->angles.y, pev->angles.z);
			Vector currentForward;
			float angleShift = 0;

			// try different degrees, starting at 0 (straight forward), then alternating 45
			// degree increments left and right of that (45, -45, 90, -90, 135, -135, 180)

			alternator = -1;  // so that after the first run adds 45 degrees
			for(i = 0; i < 8; i++){
				vecAnglesTest.y = fmod(pev->angles.y + angleShift*alternator, 360);

				////DebugLine_Setup(1, vecStart, vecStart + currentForward * theDist, 255, 0, 0);

				vecStart = EyePosition();
				UTIL_MakeAimVectorsPrivate(vecAnglesTest, currentForward, NULL, NULL);
				vecEnd = vecStart + currentForward * 300;
				// test it.
				UTIL_TraceLine(vecStart, vecEnd, ignore_monsters, ENT(pev), &tr);
				//UTIL_drawLineFrame(vecStart.x, vecStart.y, vecStart.z,

				const char* hitClassname = "NULL";
				float fracto = tr.flFraction;
				if(tr.pHit != NULL){
					if(tr.pHit->v.classname != NULL){
						hitClassname = STRING(tr.pHit->v.classname);
					}
				}

				if(tr.flFraction == 1.0){
					// take it
					pev->ideal_yaw = vecAnglesTest.y;
					TaskComplete();
					return;
				}
				
				if(alternator == -1){
					// time to add 45
					angleShift += 45;
					alternator = 1;
				}else{
					// invert only
					alternator = -1;
				}
			}//for loop through angle attempts
			
			// No good degree to take?   Huh.
			easyPrintLine("WARNING: panthereye:%d could not find any way to jump from a stuck state", monsterID);
			TaskFail();
		}break;
		//Copied from schedule.cpp.
		case TASK_PANTHEREYE_FIND_COVER_FROM_ENEMY:
		{

			if(newPathDelay == -1){

				findCoverTaskDataMem = pTask->flData;

				panthereye_findCoverFromEnemy();
				newPathDelay = gpGlobals->time + newPathDelayDuration;
				waitingForNewPath = FALSE;
			}else{
				waitingForNewPath = TRUE;
			}

			break;
		}
		

		case TASK_PANTHEREYE_COVER_FAIL_WAIT:
		case TASK_PANTHEREYE_SNEAK_WAIT:
		{

			EASY_CVAR_PRINTIF_PRE(panthereyePrintout, easyPrintLine("CAN YOU EVEN JUST THIS ONCE???? %d", pTask->iTask)); 
			if(pTask->iTask == TASK_PANTHEREYE_COVER_FAIL_WAIT){
				//cover failed?  We're "cornered"...  go crazy aggro on damage.
				isCornered = TRUE;
			}

			timeTillSneakAgain = gpGlobals->time + RANDOM_LONG(5, 11);

			if( m_hEnemy != NULL &&
				UTIL_IsFacing(m_hEnemy->pev, pev->origin, 0.3) && (
				    (isCornered && (m_vecEnemyLKP - pev->origin).Length() < 350*20) ||
				    (stareTime > 0 && stareTime > 2.6 && (m_vecEnemyLKP - pev->origin).Length() < 350*20)   //(gpGlobals->timie - stareTime > 2.6)
				)
					
			){
				//go aggro!
				pissedOffTime = gpGlobals->time + 5;
				ChangeSchedule(GetSchedule());
				return;
				//timeTillSneakAgain = gpGlobals->time + RANDOM_LONG(7, 16);
			}else{
				//try sneaking again sooner?  Be easier to piss off since couldn't update cover...  Being cornered makes most things easier to piss off.
				isPissable = TRUE;
				timeTillSneakAgain = gpGlobals->time + RANDOM_LONG(4, 8);
			}


			//Can turn while waiting.
			if ( !FacingIdeal() ){
				SetTurnActivity();
				ChangeYaw( pev->yaw_speed );
			}

		break;
		}
		case TASK_PANTHER_NORMALMOVE:
		{

			
			if(m_hTargetEnt == NULL){
				//oh no.
				TaskFail();
				return;
			}
			if ( (m_hTargetEnt->pev->origin - pev->origin).Length() < 1 )
				TaskComplete();
			else
			{
				m_vecMoveGoal = m_hTargetEnt->pev->origin;
				if ( !MoveToTarget( ACT_WALK, 2 ) )
					TaskFail();
			}
			break;
		}


		case TASK_PANTHEREYE_PLAYANIM_GETBUG:

			this->setAnimationSmart("get_bug", 2.3f);
			resetEventQueue();
			animEventQueuePush(36.0f/15.0f, 1);
			
		break;
		case TASK_PANTHEREYE_PLAYANIM_CROUCHTOJUMP:
			//desired frame / framerate (factor in pev->framerate if other than "1" by multiplying default framerate by pev->framerate BEFORE division)
			//this->setAnimationSmart("crouch_to_jump", 0.95f);  //m_flFramerateSuggestion and pev->framerate become "2".
			//resetEventQueue();
			animEventQueuePush(10.35f/(15.0f), 2 );
			//animEventQueuePush(10.35f/(15.0f), 2 );
			this->SetSequenceByIndex(g_panthereye_crouch_to_jump_sequenceID, 0.92f, FALSE);
		break;
		case TASK_PANTHEREYE_PLAYANIM_CROUCHTOJUMP_UNSTUCK:
			animEventQueuePush(10.35f/(15.0f), 3 );
			this->SetSequenceByIndex(g_panthereye_crouch_to_jump_sequenceID, 0.92f, FALSE);
		break;
		case TASK_WAIT_PVS:
			maxWaitPVSTime = gpGlobals->time + 2;
			CBaseMonster::StartTask(pTask);
		break;
		case TASK_GET_PATH_TO_ENEMY:
		{
			if(newPathDelay == -1){
				CBaseMonster::StartTask(pTask);
				newPathDelay = gpGlobals->time + newPathDelayDuration;
				//got it right here.?
				waitingForNewPath = FALSE;
				
			}else{
				waitingForNewPath = TRUE;
			}

			//why copy?  Just rely on the parent case.
			/*
			if ( BuildRoute ( m_vecEnemyLKP, bits_MF_TO_ENEMY, m_hEnemy ) )
			{
				m_iTaskStatus = TASKSTATUS_COMPLETE;
			}
			else
			{
				ALERT ( at_aiconsole, "GetPathToEnemy failed!!\n" );
				TaskFail();
			}
			break;
			*/

			break;
		}
		case TASK_WAIT_FOR_MOVEMENT:

			// nothing unusual.  Let the normal "Move" method do its thing.
			
			CBaseMonster::StartTask(pTask);

		break;
		case TASK_FACE_ENEMY:

			//wtf?
			//spend this much time at most trying to face the enemy
			faceLookTime = gpGlobals->time + 0.8f;
			CBaseMonster::StartTask(pTask);

		break;

		default:
			CBaseMonster::StartTask(pTask);
		break;
	}
}



void CPantherEye::RunTask ( Task_t *pTask ){
	//easyForcePrintLine("HEY YOOOO %s %d", getScheduleName(), pTask->iTask);
	


	switch ( pTask->iTask ){
	case TASK_PANTHEREYE_USE_ROUTE:{
		pTask->iTask = TASK_MOVE_TO_ENEMY_RANGE;
		CBaseMonster::RunTask(pTask);
	}break;

	case TASK_PANTHEREYE_FIND_COVER_FROM_ENEMY:
	{
		if(waitingForNewPath && newPathDelay == -1){
			panthereye_findCoverFromEnemy();
			newPathDelay = gpGlobals->time + newPathDelayDuration;
			waitingForNewPath = FALSE;
		}

		break;
	}
		
	case TASK_FACE_ENEMY:

		//if(!m_hEnemy){
		///	// ???
		//	TaskFail();
		//	return;
		//}

		// Why do this?  Base AI should take care of this portion
		/*
		if(HasConditions(bits_COND_SEE_ENEMY)){
			setEnemyLKP(m_vecEnemyLKP);
		}else{

			if(!HasConditions(bits_COND_ENEMY_OCCLUDED)){
				//if not occluded, try to see anyways?
				setEnemyLKP(m_vecEnemyLKP);
			}else{
				//can't see the enemy?  Can't face them.
				TaskFail();
				return;
			}
		}
		*/

		if(faceLookTime <= gpGlobals->time){
			//nope.
			TaskFail();
			return;
		}

		MakeIdealYaw( m_vecEnemyLKP );

		//DEBUG!!!
		//UTIL_drawLineFrame(pev->origin, m_vecEnemyLKP, 9, 255, 0, 0);

		ChangeYaw( pev->yaw_speed );

		if ( FacingIdeal() )
		{
			TaskComplete();
		}
	break;

	case TASK_PANTHEREYE_COVER_FAIL_WAIT:
	case TASK_PANTHEREYE_SNEAK_WAIT:
		{
			EASY_CVAR_PRINTIF_PRE(panthereyePrintout, easyPrintLine("YOU ARE NOT SERIOUS."));

			if(timeTillSneakAgain <= gpGlobals->time || (!isCornered && isEnemyLookingAtMe) ){
				timeTillSneakAgain = -1;
				ChangeSchedule(GetSchedule());
				return;
			}
		break;
		}
		
		//copied from schedule.cpp & modified.
	case TASK_PANTHER_NORMALMOVE:
		{
			float distance;
			float targetDiscrepency;


			//is that okay?
			m_hTargetEnt = m_hEnemy;

			if ( m_hTargetEnt == NULL ){
				TaskFail();
				return;
			}else
			{

				distance = ( m_vecMoveGoal - pev->origin ).Length2D();
				targetDiscrepency = (m_vecMoveGoal - m_hTargetEnt->pev->origin).Length();

				
				m_vecMoveGoal = m_hTargetEnt->pev->origin;
				FRefreshRoute();

				if ( (distance < 230) || targetDiscrepency > pTask->flData * 0.5 )
				{
					m_vecMoveGoal = m_hTargetEnt->pev->origin;
					distance = ( m_vecMoveGoal - pev->origin ).Length2D();

					//MODDD - NOTE: is this necessary??
					//FRefreshRoute();
				}
				//keep goin'

			}

			m_movementActivity = ACT_RUN;

			break;
		}
		case TASK_FACE_IDEAL:
			if(pTask->flData == 24){
				//flag: turn faster than usual.
				pev->yaw_speed = 500;
			}else if(pTask->flData == 25){
				pev->yaw_speed = 70;
			}
			CBaseMonster::RunTask(pTask);
		break;

		case TASK_PANTHEREYE_PLAYANIM_GETBUG:

			//this->SetSequenceByName("get_bug");

			if(this->m_fSequenceFinished){
				//end.
				TaskComplete();
				return;
			}

		break;
		case TASK_PANTHEREYE_PLAYANIM_CROUCHTOJUMP:
			if(m_hEnemy == NULL){
				//reset anim info?  Make sure other failure is considered too,.
				TaskFail();
				break;
			}

			if(pev->sequence != g_panthereye_crouch_to_jump_sequenceID && m_fSequenceFinishedSinceLoop){
				// ?????? WHAT?  HOW?
				SetSequenceByIndex(g_panthereye_crouch_to_jump_sequenceID, 0.92f, FALSE);
			}

			//before the first event check...
			if(animEventQueueTime[0] != -1 && (animEventQueueTime[0] + animEventQueueStartTime > gpGlobals->time) ){
				//just the start of the anim, can still turn...
				MakeIdealYaw(m_vecEnemyLKP);
				ChangeYaw(55);
			}

			if( !(pev->flags & FL_ONGROUND )){
				// not on the ground?  Get a constant small velocity boost forward to help grab onto ledges
				UTIL_MakeAimVectors(pev->angles);
				if(pev->velocity.Length2D() < 24){
					pev->velocity = pev->velocity + gpGlobals->v_forward * 8;
				}
			}//ground check
			else{
				if(this->m_fSequenceFinished ){
					TaskComplete();
					//no leap-touch, if still here.
					SetTouch( NULL );
					return;
				}
			}

			EASY_CVAR_PRINTIF_PRE(panthereyePrintout, easyPrintLine("WHAT THE : %.2f %d", pev->frame, m_fSequenceFinished));
			

		break;
		case TASK_PANTHEREYE_PLAYANIM_CROUCHTOJUMP_UNSTUCK:
			if(pev->sequence != g_panthereye_crouch_to_jump_sequenceID && m_fSequenceFinishedSinceLoop){
				// ?????? WHAT?  HOW?
				SetSequenceByIndex(g_panthereye_crouch_to_jump_sequenceID, 0.92f, FALSE);
			}

			//before the first event check...
			if(animEventQueueTime[0] != -1 && (animEventQueueTime[0] + animEventQueueStartTime > gpGlobals->time) ){
				// just the start of the anim, can still turn...
				// don't!!!  Already set up ahead of time
				//MakeIdealYaw(m_vecEnemyLKP);
				ChangeYaw(55);
			}
			
			if( !(pev->flags & FL_ONGROUND )){
				// not on the ground?  Get a constant small velocity boost forward to help grab onto ledges
				UTIL_MakeAimVectors(pev->angles);
				if(pev->velocity.Length2D() < 24){
					pev->velocity = pev->velocity + gpGlobals->v_forward * 8;
				}
			}//ground check
			else{
				if(this->m_fSequenceFinished ){
					TaskComplete();
					//no leap-touch, if still here.
					SetTouch( NULL );
					return;
				}
			}

			EASY_CVAR_PRINTIF_PRE(panthereyePrintout, easyPrintLine("WHAT THE : %.2f %d", pev->frame, m_fSequenceFinished));
			
		break;
		case TASK_WAIT_PVS:
			if(gpGlobals->time >= maxWaitPVSTime){
				TaskFail();
				return;
			}
			CBaseMonster::RunTask(pTask);
		break;

		case TASK_GET_PATH_TO_ENEMY:
			
			if(waitingForNewPath && newPathDelay == -1){
				//if we're waiting to try a path, go ahead (like start would).
				CBaseMonster::StartTask(pTask);
				newPathDelay = gpGlobals->time + newPathDelayDuration;
				waitingForNewPath = FALSE;
			}

		break;

		default:
			CBaseMonster::RunTask(pTask);
		break;
	}
}



BOOL CPantherEye::violentDeathAllowed(void){
	return TRUE;
}
BOOL CPantherEye::violentDeathClear(void){
	// Works for a lot of things going backwards.
	return violentDeathClear_BackwardsCheck(430);
}//END OF violentDeathAllowed
int CPantherEye::violentDeathPriority(void){
	return 3;
}


//Copy, takes more to make this guy react.
void CPantherEye::OnTakeDamageSetConditions(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType, int bitsDamageTypeMod){

	//MODDD - intervention. Timed damage might not affect the AI since it could get needlessly distracting.

	if(bitsDamageTypeMod & (DMG_TIMEDEFFECT|DMG_TIMEDEFFECTIGNORE) ){
		//If this is continual timed damage, don't register as any damage condition. Not worth possibly interrupting the AI.
		return;
	}

	//default case from CBaseMonster's TakeDamage.
	//Also count being in a non-combat state to force looking in that direction.
	//if ( flDamage > 0 )

	if(m_MonsterState == MONSTERSTATE_COMBAT){

		if(m_pSchedule == slPanthereyeSneakToLocation || m_pSchedule == slPanthereyeSneakWait){
			// force it anyway, get out of this schedule
			SetConditions(bits_COND_LIGHT_DAMAGE);
			forgetSmallFlinchTime = gpGlobals->time + DEFAULT_FORGET_SMALL_FLINCH_TIME;
		}
	}else if(m_MonsterState == MONSTERSTATE_IDLE || m_MonsterState == MONSTERSTATE_ALERT || flDamage >= 15)
	{
		SetConditions(bits_COND_LIGHT_DAMAGE);
		forgetSmallFlinchTime = gpGlobals->time + DEFAULT_FORGET_SMALL_FLINCH_TIME;
	}

	//MODDD - HEAVY_DAMAGE was unused before.  For using the BIG_FLINCH activity that is (never got communicated)
	//    Stricter requirement:  this attack took 70% of health away.
	//    The agrunt used to use this so that its only flinch was for heavy damage (above 20 in one attack), but that's easy by overriding this OnTakeDamageSetconditions method now.
	//    Keep it to using light damage for that instead.
	//if ( flDamage >= 20 )
	
	//Damage above 40 also causes bigflinch for tougher creatures like panthereyes,
	//just to make sure a revolver shot can cause a bigflinch.
	if(gpGlobals->time >= forgetBigFlinchTime && (flDamage >=  pev->max_health * 0.55 || flDamage >= 40) )
	{
		SetConditions(bits_COND_HEAVY_DAMAGE);
		forgetSmallFlinchTime = gpGlobals->time + DEFAULT_FORGET_SMALL_FLINCH_TIME;
		forgetBigFlinchTime = gpGlobals->time + 10;  //agrunt has a longer big flinch anim than most so just do this.
	}

/*
	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(testVar) == 10){
		//any damage causes me now.
		SetConditions(bits_COND_HEAVY_DAMAGE);
	}
	*/

	easyForcePrintLine("%s:%d OnTkDmgSetCond raw:%.2f fract:%.2f", getClassname(), monsterID, flDamage, (flDamage / pev->max_health));

	
}//END OF OnTakeDamageSetConditions

int CPantherEye::getHullIndexForNodes(void){
    return NODE_LARGE_HULL;  //safe?
}

// Only to get out of the water in case it ever jumps in, no swimming animations so it just runs along the floor underwater.
// Will work out something if there's ever an issue here.
BOOL CPantherEye::SeeThroughWaterLine(void){
	return TRUE;
}//END OF SeeThroughWaterLine


// AREWIOSDI:TGBJTDFH JRSTIPEe5wtrhiok
BOOL CPantherEye::getForceAllowNewEnemy(CBaseEntity* pOther){
	
	if(m_pSchedule == slPanthereyeSneakToLocation || m_pSchedule == slPanthereyeSneakWait){
		// If they're close and looking at me, go ahead.
		// (allow less distance if they're looking at me)
		float theDist = Distance(pev->origin, pOther->pev->origin);
		float distToTickOff = 240;
		if(UTIL_IsFacing(pOther->pev, this->pev->origin, 0.034)){
			distToTickOff = 700;
		}

		if(theDist <= distToTickOff){
			return TRUE;
		}

	}

	return FALSE;
}


