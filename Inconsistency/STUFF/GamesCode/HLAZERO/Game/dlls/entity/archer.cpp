
#include "archer.h"
#include "schedule.h"
#include "activity.h"
#include "util_model.h"
#include "defaultai.h"
#include "soundent.h"
#include "game.h"
#include "util_debugdraw.h"
#include "squidspit.h"
#include "weapons.h"
#include "nodes.h"
#include "archer_ball.h"


// Is there ever a reason to use the fast swim sequence?  Unsure.

// TODO - melee trace is still odd, it does damage to things left and right of it.  weird.


// !!!!
// And why's the tracehull grab so mucch, like standing to the side of an attacking archer not looking at you,
// hurts you instead (easy with autosneaky 1 while the archer's melee'ing something else in the water).


//TODO MAJOR-ER.  Huge problem with the controller head balls / same balls the archer uses.
//                If lightning effects are created while the source of lightning is still underwater but the endpoint is above.
//                FIXABLE maybe...

//TODO MAJOR - is it a problem if the death sequence replies on loading a save when floating to the top is blocked by something?
//             probably not, and this is kinda easily fixable if so.


EASY_CVAR_EXTERN_DEBUGONLY(noFlinchOnHard)
EASY_CVAR_EXTERN_DEBUGONLY(animationFramerateMulti)
EASY_CVAR_EXTERN_DEBUGONLY(drawDebugPathfinding)
EASY_CVAR_EXTERN_DEBUGONLY(drawDebugPathfinding2)
EASY_CVAR_EXTERN_DEBUGONLY(STUrepelMulti)
EASY_CVAR_EXTERN_DEBUGONLY(STUcheckDistV)
EASY_CVAR_EXTERN_DEBUGONLY(STUcheckDistH)
EASY_CVAR_EXTERN_DEBUGONLY(STUcheckDistD)
EASY_CVAR_EXTERN_DEBUGONLY(STUSpeedMulti)


//I would prefer to be this far off of the waterlevel if I intend to touch the surface.
#define DESIRED_WATERLEVEL_SURFACE_OFFSET -7


//#define ARCHER_SWIM_SEQ SEQ_ARCHER_IDLE1
#define ARCHER_SWIM_SEQ SEQ_ARCHER_SWIM





#if REMOVE_ORIGINAL_NAMES != 1
	LINK_ENTITY_TO_CLASS( monster_archer, CArcher );
#endif

#if EXTRA_NAMES > 0
	LINK_ENTITY_TO_CLASS( archer, CArcher );
	
	#if EXTRA_NAMES == 2
		//none?
	#endif
#endif


/*
//what. is this default behavior?
void CController::Stop( void ) 
{ 
	m_IdealActivity = GetStoppedActivity(); 
}
*/


//TODO - archer needs to be able to do a check to see if the enemy is past the waterlevel.
//       If so, it can try to move straight up, towards the enemy or a blend. Just route to the surface where there is empty
//       space so it makes sense to rise above the water there.  Don't goto a spot if some platform or dock is in the way.
//       Then it can do a ranged sea-land attack however many times and submerge, even be interrupted to submerge sooner
//       or more urgently on taking damage?

//       CheckLocalMove needs to see if the destination is towards a different waterlevel than the source.
//       (or just not underwater at all, archers should never move anywhere outside of water)
//       But maybe make it a little less sensitive when going above the water slightly to do a sea-land attack.

//       Finally when itself and the enemy are both underwater, it acts like an underwater bullsquid.
//       Use ranged projectiles if possible, and follow the enemy otherwise. Can melee when close enough.

//       QUESTION: there are sequences sink, dead_float, die1 and die2.  Does one of the "die"s lead to floating to the top (die2?) and the other sinking (die1)?

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//sequences in the model. Some sequences have the same display name and so should just be referenced by order
//(numbered index).
enum archer_sequence{  //key: frames, FPS
	SEQ_ARCHER_IDLE1,
	SEQ_ARCHER_IDLE2,
	SEQ_ARCHER_IDLE3,
	SEQ_ARCHER_BURST_SMALL,
	SEQ_ARCHER_SWIM,
	SEQ_ARCHER_SWIM_FAST,
	SEQ_ARCHER_BITE,
	SEQ_ARCHER_FLINCH1,
	SEQ_ARCHER_FLINCH2,
	SEQ_ARCHER_DIE1,
	SEQ_ARCHER_DIE2,
	SEQ_ARCHER_SHOOT,
	SEQ_ARCHER_SURFACE,
	SEQ_ARCHER_SINK,
	SEQ_ARCHER_DEAD_FLOAT,
	SEQ_ARCHER_180_LEFT,
	SEQ_ARCHER_180_RIGHT

};


//custom schedules
enum{
	//what.
	SCHED_ARCHER_RANGE_ATTACK = LAST_COMMON_SCHEDULE + 1,
	SCHED_ARCHER_RETREAT_INTO_WATER,
	SCHED_ARCHER_SURFACE_ATTACK_PLAN_FAIL,
	SCHED_ARCHER_FAIL_WAIT,

};

//custom tasks
enum{
	TASK_ARCHER_SEEK_BELOW_WATER_SURFACE_ATTACK_POINT = LAST_COMMON_TASK + 1,
	TASK_ARCHER_GATE_UPDATE_LKP_AT_WATER_SURFACE_IF_UNOBSCURED,
	TASK_ARCHER_SEEK_WATER_SURFACE_ATTACK_POINT,
	TASK_ARCHER_SEEK_WATER_SUBMERGE,
	TASK_ARCHER_SEEK_RETREAT_INTO_WATER,
	TASK_ARCHER_SEEK_RANDOM_WANDER_POINT,
	TASK_ARCHER_WAIT_FOR_MOVEMENT_STRICT,


};




//Original archer sound methods for reference. Just zombie sounds though, still placeholders.
/*

void CBloater::PainSound( void )
{
#if 0	
	int pitch = 95 + RANDOM_LONG(0,9);

	switch (RANDOM_LONG(0,5))
	{
	case 0: 
		UTIL_PlaySound(ENT(pev), CHAN_VOICE, "zombie/zo_pain1.wav", 1.0, ATTN_NORM, 0, pitch);
		break;
	case 1:
		UTIL_PlaySound(ENT(pev), CHAN_VOICE, "zombie/zo_pain2.wav", 1.0, ATTN_NORM, 0, pitch);
		break;
	default:
		break;
	}
#endif
}

void CBloater::AlertSound( void )
{
#if 0
	int pitch = 95 + RANDOM_LONG(0,9);

	switch (RANDOM_LONG(0,2))
	{
	case 0: 
		UTIL_PlaySound(ENT(pev), CHAN_VOICE, "zombie/zo_alert10.wav", 1.0, ATTN_NORM, 0, pitch);
		break;
	case 1:
		UTIL_PlaySound(ENT(pev), CHAN_VOICE, "zombie/zo_alert20.wav", 1.0, ATTN_NORM, 0, pitch);
		break;
	case 2:
		UTIL_PlaySound(ENT(pev), CHAN_VOICE, "zombie/zo_alert30.wav", 1.0, ATTN_NORM, 0, pitch);
		break;
	}
#endif
}

void CBloater::IdleSound( void )
{
#if 0
	int pitch = 95 + RANDOM_LONG(0,9);

	switch (RANDOM_LONG(0,2))
	{
	case 0: 
		UTIL_PlaySound(ENT(pev), CHAN_VOICE, "zombie/zo_idle1.wav", 1.0, ATTN_NORM, 0, pitch);
		break;
	case 1:
		UTIL_PlaySound(ENT(pev), CHAN_VOICE, "zombie/zo_idle2.wav", 1.0, ATTN_NORM, 0, pitch);
		break;
	case 2:
		UTIL_PlaySound(ENT(pev), CHAN_VOICE, "zombie/zo_idle3.wav", 1.0, ATTN_NORM, 0, pitch);
		break;
	}
#endif
}

void CBloater::AttackSnd( void )
{
#if 0
	int pitch = 95 + RANDOM_LONG(0,9);

	switch (RANDOM_LONG(0,1))
	{
	case 0: 
		UTIL_PlaySound(ENT(pev), CHAN_VOICE, "zombie/zo_attack1.wav", 1.0, ATTN_NORM, 0, pitch);
		break;
	case 1:
		UTIL_PlaySound(ENT(pev), CHAN_VOICE, "zombie/zo_attack2.wav", 1.0, ATTN_NORM, 0, pitch);
		break;
	}
#endif
}

*/


// PLACEHOLDER, DONT USE YET.
// Ichy sounds pitched up seem a little cheezy, don't think this needs any 'voice' sound for now.
const char* CArcher::pDeathSounds[] = 
{
	"archer/archer_death.wav",
};
const char* CArcher::pAlertSounds[] = 
{
	"archer/archer_alert.wav",
};

const char* CArcher::pIdleSounds[] = 
{
	"archer/archer_idle.wav",
};
const char* CArcher::pPainSounds[] = 
{
	"archer/archer_pain.wav",
};
const char* CArcher::pAttackSounds[] = 
{
	"archer/archer_attack.wav",
};


const char* CArcher::pAttackHitSounds[] = 
{
	"zombie/claw_strike1.wav",
	"zombie/claw_strike2.wav",
	"zombie/claw_strike3.wav",
};

// MODDD - how about the player's water sound effects?
// maybe not, player/pl_slosh, pl_swim, and pl_wade don't sound forceful enough.
// 'slosh' is the best but the shallow sound isn't fitting, more of an intense wade would work.
// Pitch-shifts?  Lower on wade?  doubt it.
// Keeping this as it is.
const char* CArcher::pAttackMissSounds[] = 
{
	"zombie/claw_miss1.wav",
	"zombie/claw_miss2.wav",
};




// clone of defaultai.cpp's primary attack for now, but TASK_MELEE_ATTACK1 can be interrupted
// by the enemy going out of range (kindof a long anim to keep playing; attacks twice throughout)
Task_t tlArcherMeleeAttack[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_MELEE_ATTACK1,		(float)0		},
};

Schedule_t slArcherMeleeAttack[] =
{
	{ 
		tlArcherMeleeAttack,
		ARRAYSIZE ( tlArcherMeleeAttack ), 
		bits_COND_NEW_ENEMY			|
		bits_COND_ENEMY_DEAD		|
		
		//MODDD - restoring heavy damage as interruptable.
		//bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|

		bits_COND_ENEMY_OCCLUDED,
		0,
		"Archer Melee Attack"
	},
};





//Thank you bullsquid, you know what is up dog.
Task_t	tlArcherRangeAttack1[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_FACE_IDEAL,			(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	
	//No, letting the end of TASK_RANGE_ATTACK1 handle this on our end instead.
	//We need the activity shift to idle to be instant.
	//{ TASK_SET_ACTIVITY,		(float)ACT_IDLE	},

};
Schedule_t	slArcherRangeAttack1[] =
{
	{ 
		tlArcherRangeAttack1,
		ARRAYSIZE ( tlArcherRangeAttack1 ), 
		bits_COND_NEW_ENEMY			|
		bits_COND_ENEMY_DEAD		|
		bits_COND_HEAVY_DAMAGE		|
		//bits_COND_ENEMY_OCCLUDED	|
		bits_COND_NO_AMMO_LOADED,   //er, wat?
		0,
		"Archer Range Attack1"
	},
};




// Really the ranged attack 1 surrounded by steps to ensure a random point at the water level's surface is picked that puts the enemy in a line of fire and is
//reachable by the archer can be use to make an attack, followed by returning to the same point or maybe moving randomly a bit underwater too.
Task_t	tlArcherSurfaceRangeAttack[] =
{
	//TODO - set a fail schedule that wanders to a nearby random point around the same depth in the water, or even
	//       guarantee some depth away from the surface vertically?
	
	//If I fail this early, don't spam the game with pathfind calls.  Wait a little before trying again,
	//even pick a random position underwater to go to.
	{ TASK_SET_FAIL_SCHEDULE, (float)SCHED_ARCHER_SURFACE_ATTACK_PLAN_FAIL },

	{ TASK_STOP_MOVING,			0				},
	{ TASK_ARCHER_SEEK_BELOW_WATER_SURFACE_ATTACK_POINT,  (float)0 },  //this only passes if a route could be made to the point it picked.
	{ TASK_RUN_PATH,  (float)0 },
	{ TASK_WAIT_FOR_MOVEMENT,  (float)0},

	//Cheat a little. If there's a direct line of sight to the enemy from right above at the water surface, use this to turn to face them.
	{ TASK_ARCHER_GATE_UPDATE_LKP_AT_WATER_SURFACE_IF_UNOBSCURED,  (float)0},

	//at this point, if there is a failure, know to retreat into the water. Like the enemy moving and no longer being able
	//to be attacked from this point at the surface now that I've made it here or close.
	{ TASK_SET_FAIL_SCHEDULE, (float)SCHED_ARCHER_RETREAT_INTO_WATER },

	{ TASK_ARCHER_SEEK_WATER_SURFACE_ATTACK_POINT, (float)0 },
	//{ TASK_SET_ACTIVITY, (float)ACT_FLY},
	{ TASK_RUN_PATH,  (float)0 },
	{ TASK_ARCHER_WAIT_FOR_MOVEMENT_STRICT,  (float)0},

	//now try the ranged attack.
	{ TASK_FACE_IDEAL,			(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},

	//go back into the water straight vertically without turning.
	{ TASK_ARCHER_SEEK_WATER_SUBMERGE, (float) 0  },
	//{ TASK_SET_ACTIVITY, (float)ACT_FLY},
	{ TASK_RUN_PATH,  (float)0 },
	{ TASK_ARCHER_WAIT_FOR_MOVEMENT_STRICT,  (float)0},
	
	//return to the murky depths to plot your next sinister move.
	{TASK_SET_SCHEDULE, (float)SCHED_ARCHER_RETREAT_INTO_WATER},

	
	//No, letting the end of TASK_RANGE_ATTACK1 handle this on our end instead.
	//We need the activity shift to idle to be instant.
	//{ TASK_SET_ACTIVITY,		(float)ACT_IDLE	},

};
Schedule_t	slArcherSurfaceRangeAttack[] =
{
	{ 
		tlArcherSurfaceRangeAttack,
		ARRAYSIZE ( tlArcherSurfaceRangeAttack ), 
		bits_COND_NEW_ENEMY			|
		bits_COND_ENEMY_DEAD		|
		bits_COND_HEAVY_DAMAGE		|
		//bits_COND_ENEMY_OCCLUDED	|
		bits_COND_NO_AMMO_LOADED,   //er, wat?
		0,
		"Archer Surface Range Attack"
	},
};


Task_t	tlArcherRetreatIntoWater[] =
{
	//now return to roughly around the old point. or just anywhere not at the surface to hide a bit.
	{ TASK_ARCHER_SEEK_RETREAT_INTO_WATER,  (float)0   },
	{ TASK_RUN_PATH,  (float)0 },
	{ TASK_WAIT_FOR_MOVEMENT,  (float)0},
};

Schedule_t	slArcherRetreatIntoWater[] =
{
	{ 
		tlArcherRetreatIntoWater,
		ARRAYSIZE ( tlArcherRetreatIntoWater ), 
		bits_COND_NEW_ENEMY			|
		bits_COND_ENEMY_DEAD		|
		bits_COND_HEAVY_DAMAGE		|
		//bits_COND_ENEMY_OCCLUDED	|
		bits_COND_NO_AMMO_LOADED,   //er, wat?
		0,
		"Archer Retreat Into Water"
	},
};


Task_t	tlArcherSurfaceAttackPlanFail[] =
{
	{ TASK_STOP_MOVING, (float)0 },
	//now return to roughly around the old point. or just anywhere not at the surface to hide a bit.
	{ TASK_SET_FAIL_SCHEDULE, (float)SCHED_ARCHER_FAIL_WAIT },
	
	//{ TASK_ARCHER_SEEK_RANDOM_WANDER_POINT,  (float)0   },
	//{ TASK_RUN_PATH,  (float)0 },
	//{ TASK_WAIT_FOR_MOVEMENT,  (float)0},
};

Schedule_t	slArcherSurfaceAttackPlanFail[] =
{
	{ 
		tlArcherSurfaceAttackPlanFail,
		ARRAYSIZE ( tlArcherSurfaceAttackPlanFail ), 
		bits_COND_NEW_ENEMY			|
		bits_COND_ENEMY_DEAD		|
		bits_COND_HEAVY_DAMAGE		|
		//bits_COND_ENEMY_OCCLUDED	|
		bits_COND_NO_AMMO_LOADED,   //er, wat?
		0,
		"Archer Surface Attack Plan Fail"
	},
};

Task_t	tlArcherFailWait[] =
{
	//now return to roughly around the old point. or just anywhere not at the surface to hide a bit.
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_WAIT,				(float)1		},
	//{ TASK_WAIT_PVS,			(float)0		},
};

Schedule_t	slArcherFailWait[] =
{
	{ 
		tlArcherFailWait,
		ARRAYSIZE ( tlArcherFailWait ), 
		bits_COND_NEW_ENEMY			|
		bits_COND_ENEMY_DEAD		|
		bits_COND_HEAVY_DAMAGE		|
		//bits_COND_ENEMY_OCCLUDED	|
		bits_COND_NO_AMMO_LOADED,   //er, wat?
		0,
		"Archer Fail Wait"
	},
};





Task_t	tlArcherGenericFail[] =
{
	//{ TASK_SET_FAIL_SCHEDULE,	(float)SCHED_ARCHER_GENERIC_FAIL	},
	{ TASK_STOP_MOVING,			0				},
	//{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_WAIT,				(float)0.3		},
	{ TASK_WAIT_PVS,			(float)0		},

	// still need this?
	{ TASK_UPDATE_LKP, (float)0		},

	// No, only do this onfailing pathfinding methods.
	// Possible to lead to an endless loop (not crashing, just conceptually) of failing some other schedule,
	// pikcing MOVE_FROM_ORIGIN, failing that, which leads to GenericFail, which leads to MOVE_FROM_ORIGIN,
	// which fails again, etc.  GetSchedule never gets called during any of that, so any change in conditions
	// elsewhere never has a chance to take effect (no attacking something in plain sight)
	//{ TASK_SET_SCHEDULE,			(float)SCHED_MOVE_FROM_ORIGIN },
};

Schedule_t	slArcherGenericFail[] =
{
	{
		tlArcherGenericFail,
		ARRAYSIZE ( tlArcherGenericFail ),
		bits_COND_HEAVY_DAMAGE,
		0,
		"Archer_genFail"
	},
};



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DEFINE_CUSTOM_SCHEDULES( CArcher )
{
	slArcherMeleeAttack,
	slArcherRangeAttack1,
	slArcherSurfaceRangeAttack,
	slArcherRetreatIntoWater,
	slArcherSurfaceAttackPlanFail,
	slArcherFailWait,
	slArcherGenericFail,

};
IMPLEMENT_CUSTOM_SCHEDULES( CArcher, CFlyingMonster );












TYPEDESCRIPTION	CArcher::m_SaveData[] = 
{
	DEFINE_FIELD( CArcher, preSurfaceAttackLocation, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( CArcher, lackOfWaterTimer, FIELD_TIME ),
};

//IMPLEMENT_SAVERESTORE( CArcher, CFlyingMonster );
int CArcher::Save( CSave &save )
{
	if ( !CFlyingMonster::Save(save) )
		return 0;
	int iWriteFieldsResult = save.WriteFields( "CArcher", this, m_SaveData, ARRAYSIZE(m_SaveData) );

	return iWriteFieldsResult;
}
int CArcher::Restore( CRestore &restore )
{
	if ( !CFlyingMonster::Restore(restore) )
		return 0;
	int iReadFieldsResult = restore.ReadFields( "CArcher", this, m_SaveData, ARRAYSIZE(m_SaveData) );

	return iReadFieldsResult;
}


CArcher::CArcher(void){
	shootCooldown = 0;
	m_flightSpeed = 0;
	tempCheckTraceLineBlock = FALSE;
	m_velocity = Vector(0,0,0);
	lastVelocityChange = -1;
	waterLevelMem = -1;
	lackOfWaterTimer = -1;
	meleeAttackIsDirect = FALSE;

}//CArcher constructor







	
void CArcher::DeathSound( void ){
	/*
	int pitch = 95 + RANDOM_LONG(0,9);
	UTIL_PlaySound( edict(), CHAN_VOICE, RANDOM_SOUND_ARRAY(pDeathSounds), 1.0, ATTN_IDLE, 0, pitch );
	*/
}
void CArcher::AlertSound( void ){
	/*
	int pitch = 95 + RANDOM_LONG(0,9);
	UTIL_PlaySound( edict(), CHAN_VOICE, RANDOM_SOUND_ARRAY(pAlertSounds), 1.0, ATTN_NORM, 0, pitch );
	*/
}
void CArcher::IdleSound( void ){
	/*
	int pitch = 95 + RANDOM_LONG(0,9);
	// Play a random idle sound
	UTIL_PlaySound( edict(), CHAN_VOICE, RANDOM_SOUND_ARRAY(pIdleSounds), 1.0, ATTN_NORM, 0, pitch );
	*/
}
void CArcher::PainSound( void ){
	/*
	int pitch = 95 + RANDOM_LONG(0,9);
	if (RANDOM_LONG(0,5) < 2){
		UTIL_PlaySound( edict(), CHAN_VOICE, RANDOM_SOUND_ARRAY(pPainSounds), 1.0, ATTN_NORM, 0, pitch );
	}
	*/
}
void CArcher::AttackSound( void ){
	/*
	int pitch = 95 + RANDOM_LONG(0,9);
	// Play a random attack sound
	UTIL_PlaySound( edict(), CHAN_VOICE, RANDOM_SOUND_ARRAY(pAttackSounds), 1.0, ATTN_NORM, 0, pitch );
	*/
}



extern int global_useSentenceSave;
void CArcher::Precache( void )
{
	PRECACHE_MODEL("models/archer.mdl");

	//sprite precache left to the SquidSpit file (separate).
	CSquidSpit::precacheStatic();

	global_useSentenceSave = TRUE;
	
	//NOTICE - attempting to precace files that don't exist crashes the game.
	/*
	//PRECACHE_SOUND("archer/archer_XXX.wav");
	PRECACHE_SOUND_ARRAY(pDeathSounds);
	PRECACHE_SOUND_ARRAY(pAlertSounds);
	PRECACHE_SOUND_ARRAY(pIdleSounds);
	PRECACHE_SOUND_ARRAY(pPainSounds);
	PRECACHE_SOUND_ARRAY(pAttackSounds);
	PRECACHE_SOUND_ARRAY(pAttackHitSounds);
	PRECACHE_SOUND_ARRAY(pAttackMissSounds);
	*/
	PRECACHE_SOUND("x/x_shoot1.wav");

	global_useSentenceSave = FALSE;


	UTIL_PrecacheOther( "archer_ball" );


}//END OF Precache()


void CArcher::Spawn( void )
{
	Precache( );

	setModel("models/archer.mdl");
	//UTIL_SetSize( pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX );
	UTIL_SetSize( pev, Vector( -16, -16, 0 ), Vector( 16, 16, 18 ));

	pev->classname = MAKE_STRING("monster_archer");

	//I am underwater.
	pev->flags |= FL_SWIM;

	// wait no, leave this check up to MonsterThink.
	// Who knows if whichever MOVETYPE will influence the MonsterInit.
	/*
	if ( pev->waterlevel == 0){
		// mid-air?  fallin'
		pev->movetype = MOVETYPE_TOSS;
	}else{
		pev->movetype = MOVETYPE_FLY;
	}
	*/
	pev->movetype = MOVETYPE_FLY;

	//pev->solid		= SOLID_BBOX;  //not SOLID_SLIDEBOX
	pev->solid			= SOLID_SLIDEBOX;  //SOLID_TRIGGER?  Difference?
	//pev->movetype		= MOVETYPE_BOUNCEMISSILE;
	

	m_bloodColor		= BLOOD_COLOR_GREEN;
	pev->effects		= 0;
	//NOTE - you have to make this exist over in skill.h and handle some other setup (gamerules.cpp, and the CVar in game.cpp)!
	//example: skilldata_t member "scientistHealth" and CVar "sk_scientist_health".
	pev->health			= gSkillData.archerHealth;
	pev->view_ofs		= VEC_VIEW;/// position of the eyes relative to monster's origin.
	m_flFieldOfView		= VIEW_FIELD_WIDE;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState		= MONSTERSTATE_NONE;

	pev->yaw_speed		= 100;//bound to change often from "SetYawSpeed". Likely meaningless here but a default can't hurt.

	MonsterInit();

	m_flightSpeed = 400;

	SetTouch(&CArcher::CustomTouch );
	//SetTouch( NULL );

}//END OF Spawn();




Activity CArcher::GetStoppedActivity( void ){
	return CFlyingMonster::GetStoppedActivity();
}
void CArcher::Stop(){
	//perhaps we don't want to step on the brakes so soon? It's ok to float a little in the direction we're moving,
	//naturally slow down to a stop anyways without MoveExecute forcing a velocity, or in MonsterThink if detected as stopped.

	CFlyingMonster::Stop();
}



int CArcher::CheckLocalMove ( const Vector &vecStart, const Vector &vecEnd, CBaseEntity *pTarget, BOOL doZCheck, float *pflDist )
{
	int iReturn;

	/*
	// UNDONE: need to check more than the endpoint
	if (FBitSet(pev->flags, FL_SWIM) && (UTIL_PointContents(vecEnd) != CONTENTS_WATER))
	{
		// ALERT(at_aiconsole, "can't swim out of water\n");
		return FALSE;
	}
	*/

	Vector vecEndFiltered;


	vecEndFiltered = vecEnd;
	// NO NEED ANYMORE
	/*
	if(pTarget != NULL && m_hEnemy != NULL && pTarget->edict() == m_hEnemy->edict()){
		// HACK!  If the thing I'm routing towards is my enemy (neither of those things being null),
		// and I can see my enemy, and I'm using the LKP as a goal, I'd rather use the center of the
		// enemy as a goal instead.  Might be possible to head towards their feet otherwise which is kinda odd,
		// although other monsters probably won't be in the water to begin with.
		// MODDD - TODO.  See top of file for proper fix idea
		if(vecEnd == m_vecEnemyLKP){
			// orrrr this makes no difference?...    okay?
			// debugline point it
			vecEndFiltered = pTarget->Center();
		}else{
			//nope, just use what you got
			vecEndFiltered = vecEnd;
		}
	}else{
		// nevermind
		vecEndFiltered = vecEnd;
	}
	*/


	// Now wait a moment.  We can't move to a point that's out of the water can we? Deny if so.
	// WAIT!  This isn't good for an enemy (player) who's feet is in the water.
	// If there is a target with a waterlevel above 0 (in water in any extent) and long-range
	// logic isn't on (it is pickier), accept anyway

	if(pev->spawnflags & SF_ARCHER_LONG_RANGE_LOGIC){
		// only do the point test.
		int conPosition = UTIL_PointContents(vecEndFiltered);
		if( conPosition != CONTENTS_WATER){
			//Water only!
			return LOCALMOVE_INVALID_DONT_TRIANGULATE;
		}
	}else{
		if(pTarget != NULL){
			if(pTarget->pev->waterlevel != 0){
				// good to go
			}else{
				// nope.
				return LOCALMOVE_INVALID_DONT_TRIANGULATE;
			}
		}else{
			// point test, all we got.
			int conPosition = UTIL_PointContents(vecEndFiltered);
			if( conPosition != CONTENTS_WATER){
				//Water only!
				return LOCALMOVE_INVALID_DONT_TRIANGULATE;
			}
		}
	}


	/*
	//Vector goalDir = vecEndFiltered - vecStart;

	//Gets a yaw (0 - 360 degrees).
	//To radians:
	//float goal_pitch = UTIL_VecToYaw(vecEndFiltered - vecStart) * (M_PI/180.0f);

	Vector goal_direction = (vecEndFiltered - vecStart).Normalize();
	Vector goal_direction_adjacent = CrossProduct(goal_direction, Vector(0, 0, 1) ).Normalize();

	
	UTIL_MakeVectors( pev->angles );
	

	TraceResult trTopLeft;
	TraceResult trTopRight;
	TraceResult trBottomLeft;
	TraceResult trBottomRight;
	
	Vector vecOff;
	float boundXSize = fabs(pev->mins.x);
	float boundYSize = fabs(pev->mins.y);
	float boundZSize = fabs(pev->maxs.z/2);

	//Vector vecCenter = Vector(pev->origin.x, pev->origin.y, pev->origin.z + (pev->maxs.z - pev->mins.z)/2.0);
	Vector vecCenterRel = Vector(0, 0, (pev->maxs.z - pev->mins.z)/2.0);
	
	Vector whut = pev->origin;
	Vector vecStartAlt = vecStart + vecCenterRel + goal_direction * boundXSize*1.3;
	Vector vecEndAlt = vecEndFiltered + vecCenterRel + -goal_direction * boundXSize*1.3;
	


	DebugLine_SetupPoint(7, vecStartAlt, 255, 255, 255);

	DebugLine_SetupPoint(8, vecEndAlt, 255, 0, 0);

	DebugLine_ClearAll();

	vecOff = -goal_direction_adjacent * boundYSize*1.3 + gpGlobals->v_up * boundZSize*1.3;
	UTIL_TraceHull( vecStartAlt + vecOff, vecEndAlt + vecOff, dont_ignore_monsters, point_hull, edict(), &trTopLeft );
	DebugLine_Setup(0, vecStartAlt+vecOff, vecEndAlt+vecOff, trTopLeft.flFraction);

	vecOff = goal_direction_adjacent * boundYSize*1.3 + gpGlobals->v_up * boundZSize*1.3;
	UTIL_TraceHull( vecStartAlt + vecOff, vecEndAlt + vecOff, dont_ignore_monsters, point_hull, edict(), &trTopRight );
	DebugLine_Setup(1, vecStartAlt+vecOff, vecEndAlt+vecOff, trTopRight.flFraction);

	vecOff = -goal_direction_adjacent * boundYSize*1.3 + -gpGlobals->v_up * boundZSize*1.3;
	UTIL_TraceHull( vecStartAlt + vecOff, vecEndAlt + vecOff, dont_ignore_monsters, point_hull, edict(), &trBottomLeft );
	DebugLine_Setup(4, vecStartAlt+vecOff, vecEndAlt+vecOff, trBottomLeft.flFraction);

	vecOff = goal_direction_adjacent * boundYSize*1.3 + -gpGlobals->v_up * boundZSize*1.3;
	UTIL_TraceHull( vecStartAlt + vecOff, vecEndAlt + vecOff, dont_ignore_monsters, point_hull, edict(), &trBottomRight );
	DebugLine_Setup(3, vecStartAlt+vecOff, vecEndAlt+vecOff, trBottomRight.flFraction);

	float minFraction;
	trTopLeft.flFraction<trTopRight.flFraction?minFraction=trTopLeft.flFraction:minFraction=trTopRight.flFraction;
	minFraction<trBottomLeft.flFraction?minFraction=minFraction:minFraction=trBottomLeft.flFraction;
	minFraction<trBottomRight.flFraction?minFraction=minFraction:minFraction=trBottomRight.flFraction;
	//minFraction<trCenter.flFraction?minFraction=minFraction:minFraction=trCenter.flFraction;
	BOOL tracesSolid;
	BOOL tracesStartSolid;
	tracesSolid = (trTopLeft.fAllSolid != 0 || trTopRight.fAllSolid != 0 || trBottomLeft.fAllSolid != 0 || trBottomRight.fAllSolid != 0); //|| trCenter.fAllSolid != 0);
	tracesStartSolid = (trTopLeft.fStartSolid != 0 || trTopRight.fStartSolid != 0 || trBottomLeft.fStartSolid != 0 || trBottomRight.fStartSolid != 0); //|| trCenter.fStartSolid != 0);

	
	
	if ( (tracesSolid == FALSE && tracesStartSolid == FALSE && minFraction >= 1.0)  ) //|| EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(testVar) == 2)
	//if ( tr.fAllSolid == 0 && tr.fStartSolid == 0 && tr.flFraction >= 1.0)
	{
		//if(minFractionStore != NULL){ *minFractionStore = minFraction; }  //on success, the caller wants to know the minimum fraction seen, if a place to put it is provided.
		//return TRUE;
		//no, fall through.
	}else{
		//no go.
		return LOCALMOVE_INVALID_DONT_TRIANGULATE;
	}

	//return FALSE;



	if (pflDist)
	{
		*pflDist = minFraction * (vecEndFiltered - vecStart).Length();
	}

	iReturn = LOCALMOVE_VALID;
	

	
	//if(tracesStartSolid || minFraction < 1.0)

	*/


	
	TraceResult tr;
	Vector vecStartTrace = vecStart + Vector( 0, 0, 6 );


	//UTIL_TraceHull( vecStart + Vector( 0, 0, 32 ), vecEndFiltered + Vector( 0, 0, 32 ), dont_ignore_monsters, large_hull, edict(), &tr );
	
	// AWOEGKAERIGHJTEIHGJRTYGNBIMYRFJERAHJBETISH          no
	//UTIL_TraceHull( vecStartTrace, vecEndFiltered + Vector( 0, 0, 6), dont_ignore_monsters, head_hull, edict(), &tr );
	TRACE_MONSTER_HULL(edict(), vecStartTrace, vecEndFiltered + Vector( 0, 0, 6), dont_ignore_monsters, edict(), &tr);
	



	// ALERT( at_console, "%.0f %.0f %.0f : ", vecStart.x, vecStart.y, vecStart.z );
	// ALERT( at_console, "%.0f %.0f %.0f\n", vecEndFiltered.x, vecEndFiltered.y, vecEndFiltered.z );

	if (pflDist)
	{
		*pflDist = ( (tr.vecEndPos ) - vecStartTrace ).Length();// get the distance.
	}
	

	if(tr.fStartSolid){
		//that's all?  uhhh.. pass for now. this situation sucks.
		return LOCALMOVE_VALID;
	}

	// ALERT( at_console, "check %d %d %f\n", tr.fStartSolid, tr.fAllSolid, tr.flFraction );
	if (tr.fStartSolid || tr.flFraction < 1.0)
	{
		if ( pTarget && pTarget->edict() == gpGlobals->trace_ent ){
			iReturn = LOCALMOVE_VALID;
		}else{
			iReturn = LOCALMOVE_INVALID;
		}
	}else{
		iReturn = LOCALMOVE_VALID;
	}
	


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

	return iReturn;
}//CheckLocalMove


//Copied from flyingmonster.cpp. Our sequence may not properly convey the m_flGroundSpeed.
void CArcher::Move( float flInterval )
{
	if ( pev->movetype == MOVETYPE_FLY )
		m_flGroundSpeed = m_flightSpeed;
	
	//CFlyingMonster::Move( flInterval );
	CBaseMonster::Move(flInterval);
}


BOOL CArcher::ShouldAdvanceRoute( float flWaypointDist, float flInterval )
{

	if(getTaskNumber() == TASK_ARCHER_WAIT_FOR_MOVEMENT_STRICT){
		//Tighter standards!
		
		//AND YES THIS EXPLAINS A LOOOOOOOOT.
		// Get true 3D distance to the goal so we reach the correct height
		if ( m_Route[ m_iRouteIndex ].iType & bits_MF_IS_GOAL )
			flWaypointDist = ( m_Route[ m_iRouteIndex ].vecLocation - pev->origin ).Length();

		if(m_Route[ m_iRouteIndex ].iType & bits_MF_IS_GOAL){

			if(flWaypointDist < 16.0f){
				//HACK - we want to stop here exactly, Z-wise. Stop all velocity.

				m_velocity = Vector(0, 0, 0);
				pev->velocity = Vector(0, 0, 0);

				UTIL_MoveToOrigin ( ENT(pev), Vector(pev->origin.x, pev->origin.y, m_Route[ m_iRouteIndex ].vecLocation.z), fabs(m_Route[ m_iRouteIndex ].vecLocation.z - pev->origin.z) , MOVE_STRAFE );
			
				return TRUE;
			}else{
				return FALSE;
			}

		}//END OF goal check
		else{
			//do it the way the flier would.
			return CFlyingMonster::ShouldAdvanceRoute(flWaypointDist, flInterval);
		}

	}else{
		//typical flyer way.
		return CFlyingMonster::ShouldAdvanceRoute(flWaypointDist, flInterval);

	}

	return FALSE;
}





void CArcher::MoveExecute( CBaseEntity *pTargetEnt, const Vector &vecDir, float flInterval )
{
	const float waterLevel = UTIL_WaterLevel(pev->origin, pev->origin.z - 512, pev->origin.z + 4096.0);
	const float waterLevelIdeal = waterLevel + DESIRED_WATERLEVEL_SURFACE_OFFSET;

	if ( pev->waterlevel == 0){
		// not in the water at all?  uh-oh
		return;
	}
	



	/*
	if ( m_IdealActivity != m_movementActivity )
		m_IdealActivity = m_movementActivity;

	// ALERT( at_console, "move %.4f %.4f %.4f : %f\n", vecDir.x, vecDir.y, vecDir.z, flInterval );

	// float flTotal = m_flGroundSpeed * pev->framerate * flInterval;
	// UTIL_MoveToOrigin ( ENT(pev), m_Route[ m_iRouteIndex ].vecLocation, flTotal, MOVE_STRAFE );

	//m_velocity = m_velocity * 0.8 + m_flGroundSpeed * vecDir * 0.2;
	
	//UTIL_MoveToOrigin ( ENT(pev), pev->origin + m_velocity, m_velocity.Length() * flInterval, MOVE_STRAFE );
	m_flGroundSpeed = 124;
	UTIL_MoveToOrigin ( ENT(pev), m_Route[ m_iRouteIndex ].vecLocation, (m_flGroundSpeed * flInterval), MOVE_STRAFE );
	
	


	
	*/


	

	/*
	Vector vecSuggestedDir = (m_Route[m_iRouteIndex].vecLocation - pev->origin).Normalize();
	//float velMag = flStep * global_STUSpeedMulti;
	float velMag = m_flGroundSpeed * global_STUSpeedMulti;

	CFlyingMonster::MoveExecute(pTargetEnt, vecDir, flInterval);
	*/
    //checkFloor(vecSuggestedDir, velMag, flInterval);


	if ( m_IdealActivity != m_movementActivity )
	{
		m_IdealActivity = m_movementActivity;
		m_flGroundSpeed = m_flightSpeed = 200;
	}



	if(pev->origin.z <= waterLevelIdeal){
	
		//I can move.

		//m_flGroundSpeed = m_flightSpeed = 200;

		//m_flGroundSpeed = m_flightSpeed = 10;
		//TEST - just force it?
		//m_flGroundSpeed = m_flightSpeed = 200;
		//this->SetSequenceByIndex(SEQ_ARCHER_TURN_LEFT, 1);

		//m_flGroundSpeed = 200;

		float flTotal = 0;
		float flStepTimefactored = m_flGroundSpeed * pev->framerate * EASY_CVAR_GET_DEBUGONLY(animationFramerateMulti) * flInterval;
		float flStep = m_flGroundSpeed * 1 * 1;
	


		float velMag = flStep * EASY_CVAR_GET_DEBUGONLY(STUSpeedMulti);

		float timeAdjust = (pev->framerate * EASY_CVAR_GET_DEBUGONLY(animationFramerateMulti) * flInterval);
		float distOneFrame = velMag * pev->framerate * EASY_CVAR_GET_DEBUGONLY(animationFramerateMulti) * flInterval;
	
		Vector dest = m_Route[ m_iRouteIndex ].vecLocation;
		Vector vectBetween = (dest - pev->origin);
		float distBetween = vectBetween.Length();
		Vector dirTowardsDest = vectBetween.Normalize();
		Vector _velocity;



		//MODDD - wait why don't you just use the supplied vecDir from monsterThink taht derives the direction the same way (from me to the next m_iRouteIndex waypoint)?
		dirTowardsDest = vecDir;
		//HACK, REVERT ME!!!
		distOneFrame = 0;



		if(distOneFrame <= distBetween){
			_velocity = dirTowardsDest * velMag;
		}else{
			_velocity = dirTowardsDest * distBetween/timeAdjust;
		}

		//UTIL_printLineVector("MOVEOUT", velMag);
		//easyPrintLineGroup2("HELP %.8ff %.8f", velMag, flInterval);

		//UTIL_drawLineFrame(pev->origin, dest, 64, 255, 0, 0);

	
		m_velocity = m_velocity * 0.8 + _velocity * 0.2;

		//m_velocity = m_velocity * 0.8 + m_flGroundSpeed * vecDir * 0.2;

		Vector flatVelocity = Vector(_velocity.x, _velocity.y, 0);
		Vector vertVelocity = Vector(0, 0, _velocity.z);


		//are we pushed back by the water?
		//Vector offset = DoVerticalProbe(flInterval);
		//m_velocity = m_velocity - offset;


		//or pev->velocity.z ?
		float anticipatedZ = pev->origin.z + m_velocity.z * flInterval;

		float differDence = anticipatedZ - waterLevelIdeal;

		//30


		//20---------


		//origin: 18.  velocity: 12.


		if(anticipatedZ >= waterLevelIdeal - 1){
		
			//


			/*
			//float tempEEE = ( ((float)(pev->origin.z)) + (anticipatedZ - waterLevel));
			float anticipatedMoveDistance = m_velocity.z * flInterval;

			if(anticipatedMoveDistance < differDence){
				//still only move towards the water at most by this amount.
				//m_velocity = Vector(m_velocity.x, m_velocity.y, fabs(m_velocity.z) * -1);
				m_velocity.z -= 3;
			}else{
				//puts us exactly at the water level if it would've gone past.
				m_velocity.z = 0;
				pev->origin.z = waterLevel;
			}
			*/

			m_velocity.z = 0;
			//pev->origin.z = waterLevel;

			UTIL_MoveToOrigin ( ENT(pev), Vector(pev->origin.x, pev->origin.y, waterLevelIdeal), fabs(pev->origin.z - waterLevelIdeal) , MOVE_STRAFE );
			
			//m_velocity.z = 0;
		}


		//pev->velocity = _velocity;
		pev->velocity = m_velocity;

		//Vector vecSuggestedDir = (m_Route[m_iRouteIndex].vecLocation - pev->origin).Normalize();
		//checkFloor(vecSuggestedDir, velMag, flInterval);


		lastVelocityChange = gpGlobals->time;


	}else{
		//I'm above the water.
		//what?

		//m_velocity = m_velocity * 0.8 + _velocity * 0.2;
		m_velocity.z -= 26;
		pev->velocity = m_velocity;

	}


}//END OF MoveExecute



/*

void CArcher::MoveExecute( CBaseEntity *pTargetEnt, const Vector &vecDir, float flInterval )
{
	//m_flGroundSpeed = 25;
	if ( pev->movetype == MOVETYPE_FLY )
	{
		if ( gpGlobals->time - m_stopTime > 1.0 )
		{
			if ( m_IdealActivity != m_movementActivity )
			{
				m_IdealActivity = m_movementActivity;
				m_flGroundSpeed = m_flightSpeed = 200;
			}
		}
		//Vector vecMove = pev->origin + (( vecDir + (m_vecTravel * m_momentum) ).Normalize() * (m_flGroundSpeed * flInterval));
		Vector vecMove = m_Route[ m_iRouteIndex ].vecLocation;

		if ( m_IdealActivity != m_movementActivity )
		{
			m_flightSpeed = UTIL_Approach( 100, m_flightSpeed, 75 * gpGlobals->frametime );
			if ( m_flightSpeed < 100 )
				m_stopTime = gpGlobals->time;
		}
		else
			m_flightSpeed = UTIL_Approach( 20, m_flightSpeed, 300 * gpGlobals->frametime );
		
		if ( CheckLocalMove ( pev->origin, vecMove, pTargetEnt, doZCheck, NULL ) == LOCALMOVE_VALID )
		{
			m_vecTravel = (vecMove - pev->origin);
			m_vecTravel = m_vecTravel.Normalize();
			
			//UTIL_MoveToOrigin(ENT(pev), vecMove, (m_flGroundSpeed * flInterval), MOVE_STRAFE);
			UTIL_MoveToOrigin ( ENT(pev), m_Route[ m_iRouteIndex ].vecLocation, (m_flGroundSpeed * flInterval), MOVE_NORMAL );

			//pev->origin = pev->origin + (vecMove - pev->origin).Normalize()*(m_flGroundSpeed * flInterval);
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

*/



//If the model won't tell us, we have to make one.
void CArcher::SetEyePosition(void){
	//CFlyingMonster::SetEyePosition();
	pev->view_ofs = Vector(0, 0, 42 - 8);//VEC_VIEW;
}//END OF SetEyePosition




//based off of GetSchedule for CFlyingMonster in schedule.cpp.
Schedule_t* CArcher::GetSchedule ( void )
{
	//MODDD - safety.
	if(pev->deadflag != DEAD_NO){
		return GetScheduleOfType( SCHED_DIE );
	}
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
			//MODDD - other condition.  If "noFlinchOnHard" is on and the skill is hard, don't flinch from getting hit.
			else if (HasConditions(bits_COND_LIGHT_DAMAGE) && !HasMemory( bits_MEMORY_FLINCHED) && !(EASY_CVAR_GET_DEBUGONLY(noFlinchOnHard)==1 && g_iSkillLevel==SKILL_HARD)  )
			{
				return GetScheduleOfType( SCHED_SMALL_FLINCH );
			}
			else if ( !HasConditions(bits_COND_SEE_ENEMY) )
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
					return GetScheduleOfType(SCHED_CHASE_ENEMY);
				}
			}
			else  
			{

				//easyPrintLine("I say, what? %d %d", HasConditions(bits_COND_CAN_RANGE_ATTACK1), HasConditions(bits_COND_CAN_RANGE_ATTACK2) );


				// If in melee range, just do that instead.
				if ( HasConditions(bits_COND_CAN_MELEE_ATTACK1) )
				{
					return GetScheduleOfType( SCHED_MELEE_ATTACK1 );
				}
				if ( HasConditions(bits_COND_CAN_MELEE_ATTACK2) )
				{
					return GetScheduleOfType( SCHED_MELEE_ATTACK2 );
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

				//MODDD - NOTE - is that intentional?  range1 & melee1,  and not say,  melee1 & melee2???
				if ( !HasConditions(bits_COND_CAN_RANGE_ATTACK1 | bits_COND_CAN_MELEE_ATTACK1) )
				{
					// if we can see enemy but can't use either attack type, we must need to get closer to enemy
					return GetScheduleOfType( SCHED_CHASE_ENEMY );
				}
				else if ( !FacingIdeal() )
				{
					//turn
					return GetScheduleOfType( SCHED_COMBAT_FACE );
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
}//END OF GetSchedule()




Schedule_t* CArcher::GetScheduleOfType( int Type){
	
	switch(Type){
		case SCHED_FAIL:{

			if(m_pSchedule == slPrimaryMeleeAttack || m_pSchedule == slSecondaryMeleeAttack){
				SetActivity(ACT_IDLE);
			}

			return slArcherGenericFail;
		}
		case SCHED_ARCHER_FAIL_WAIT:
			return slArcherFailWait;
		break;

		case SCHED_ARCHER_SURFACE_ATTACK_PLAN_FAIL:{
			return slArcherSurfaceAttackPlanFail;
		break;}

		case SCHED_ARCHER_RETREAT_INTO_WATER:{
			return slArcherRetreatIntoWater;
		break;}
		case SCHED_CHASE_ENEMY:{
			//HOLD UP.  Does it make sense to try this?

			BOOL validFollow = FALSE;

			if(m_hEnemy == NULL){
				// ??? what?
				return GetScheduleOfType(SCHED_FAIL);
			}

			if(pev->spawnflags & SF_ARCHER_LONG_RANGE_LOGIC){
				// must be all the way in the water
				validFollow = (m_hEnemy->pev->waterlevel == 3);
			}else{
				// not long range?  Likely in a shallow body of water, crawl to its feet
				// (more leech-like)
				validFollow = (m_hEnemy->pev->waterlevel != 0);
			}


			if(validFollow){
				// our enemy disappeared (???) or is in the water? ok. proceed as usual.
				// If they disappeared this will fail pretty fast. How'd it get called anyways?
				return slChaseEnemySmart;
			}else{
				// Enemy isn't in the water? Wait for them to come back. Can interrupt by being able to attack too.
				//slWaitForEnemyToEnterWater ?
				
				// NEW!  Make sure I have the 'can range attack1' condition.  Yeah, imagine that.
				
				if(HasConditions(bits_COND_CAN_MELEE_ATTACK1)){
					// special case from the enemy being very close (or on top of me)
					return slArcherMeleeAttack;
				}

				if(HasConditions(bits_COND_CAN_RANGE_ATTACK1)){
					return (pev->spawnflags & SF_ARCHER_LONG_RANGE_LOGIC) ? slArcherSurfaceRangeAttack : slArcherRangeAttack1;
				}else{
					// stare?  What else can we do?
					return slCombatLook;
				}
			}
		break;}
		case SCHED_CHASE_ENEMY_FAILED:{
			// Repeat from what schedule.cpp.  I'm not calling the parent method just for this.
			if(m_hEnemy != NULL){
				setEnemyLKP(m_hEnemy.GetEntity());
			}else{
				// what
				return slCombatLook;
			}

			BOOL validFollow = FALSE;


			if(pev->spawnflags & SF_ARCHER_LONG_RANGE_LOGIC){
				// must be all the way in the water
				validFollow = (m_hEnemy->pev->waterlevel == 3);
			}else{
				// not long range?  Likely in a shallow body of water, crawl to its feet
				// (more leech-like)
				validFollow = (m_hEnemy->pev->waterlevel != 0);
			}



			if(validFollow){
				// enemy is in the water and you failed?  Typical pathfind fail I guess.
				//return &slFail[ 0 ];
				return &slArcherGenericFail[0];
			}else{
				//Enemy isn't in the water?  No wonder we can't get to them.
				//Just stick to staring with continual checks for the enemy being in the water or not.
				//That is be a little more reactive while waiting than just staring into space.

				//slWaitForEnemyToEnterWater ?
				// NEW!  Make sure I have the 'can range attack1' condition.  Yeah, imagine that.
				if(HasConditions(bits_COND_CAN_RANGE_ATTACK1)){
					return (pev->spawnflags & SF_ARCHER_LONG_RANGE_LOGIC) ? slArcherSurfaceRangeAttack : slArcherRangeAttack1;
				}else{
					// stare?  What else can we do?
					return slCombatLook;
				}
			}

		break;}
		


		case SCHED_DIE:{
			//return flyerDeathSchedule();
			return slDieWaterFloat;
		break;}
		case SCHED_MELEE_ATTACK1: {
			return slArcherMeleeAttack;
		break;}
		case SCHED_RANGE_ATTACK1:{

			if(m_hEnemy == NULL || m_hEnemy->pev->waterlevel == 3){
				// Our enemy disappeared (will fail soon?) or is still in the water? Typical attack, nothing special.
				return slArcherRangeAttack1;
			}else{
				// Enemy isn't in the water?  Let's see if emerging at the surface of the water to do an attack is possible.
				//return slWaitForEnemyToEnterWater;
				//...
				// TODO. change this later.
				return (pev->spawnflags & SF_ARCHER_LONG_RANGE_LOGIC) ? slArcherSurfaceRangeAttack : slArcherRangeAttack1;
			}

		break;}



	}//END OF switch(Type)
	
	return CFlyingMonster::GetScheduleOfType(Type);
}//END OF GetScheduleOfType


void CArcher::ScheduleChange(){

	Schedule_t* endedSchedule = m_pSchedule;

	//Use this to do something when a schedule ends that must happen at the time of interruption.
	//Depending on what schedule that ended typically, whether it was interrupted by some condition or naturally
	//ran out of tasks.
	
	
	
	
	CFlyingMonster::ScheduleChange(); //Call the parent.

}//END OF ScheduleChange


Schedule_t* CArcher::GetStumpedWaitSchedule(){
	return CFlyingMonster::GetStumpedWaitSchedule();
}//END OF GetStumpedWaitSchedule




void CArcher::StartTask( Task_t *pTask ){


	switch( pTask->iTask ){
		case TASK_MELEE_ATTACK1: {
			interruptMeleeTimer = -1;  // reset this on starting.  That's all.
			CFlyingMonster::StartTask(pTask);
		}break;
		case TASK_ARCHER_SEEK_BELOW_WATER_SURFACE_ATTACK_POINT:{

			if(m_hEnemy == NULL){
				//???
				TaskFail();
				return;
			}

			//Get the water level.
			float waterLevel = UTIL_WaterLevel(pev->origin, pev->origin.z, pev->origin.z + 4096.0);

			//if my origin were at the waterlevel instead, where would it be with the X and Y unaffected?
			//points to try to surface at will be picked from this at random.
			Vector originAtWaterLevel = Vector(pev->origin.x, pev->origin.y, waterLevel);

			Vector surfaceGuess;
			int tries = 4;
			int minorTries = 64;

			//try 4 times to see if a point is able to attack the enemy (straight line-trace, unobstructed)
			
			while(tries >= 0 && minorTries >= 0){
				TraceResult trTemp;
				
				float randomSurfaceDir_x = RANDOM_FLOAT(0, 900);
				float randomSurfaceDir_y = RANDOM_FLOAT(0, 900);
				int surfaceGuessContents;
				Vector surfaceGuessAboveWaterLevel;

				tries--;
				minorTries--;

				surfaceGuess = originAtWaterLevel + Vector(randomSurfaceDir_x, randomSurfaceDir_y, 0);

				if(UTIL_PointContents(surfaceGuess - Vector(0, 0, 8)) != CONTENTS_WATER){
					//this isn't even the water? out of bounds, etc.?  Forget this.
					tries++;  //allow another plain try.
					continue;
				}

				//check. If this point is in a solid place it's no good.
				surfaceGuessContents = UTIL_PointContents(surfaceGuess);

				if(surfaceGuessContents == CONTENTS_SOLID){
					//invalid.
					continue;
				}

				//is above and below a little unobstructed?
				UTIL_TraceLine(surfaceGuess + Vector(0, 0, -60), surfaceGuess + Vector(0, 0, 38), dont_ignore_monsters, edict(), &trTemp);

				//if(trTemp.fStartSolid || trTemp.fAllSolid || trTemp.flFraction < 1.0f){
				if(trTemp.fAllSolid || trTemp.flFraction < 1.0f){
					//if this was obstructed or solid in any way, don't allow it.
					continue;
				}

				//is a line from the anticipated surface point to the enemy unobstructed?
				surfaceGuessAboveWaterLevel = surfaceGuess + Vector(0, 0, 16);

				//CHEAT - use the enemy's real position.
				UTIL_TraceLine(surfaceGuessAboveWaterLevel, m_hEnemy->Center(), dont_ignore_monsters, edict(), &trTemp);


				//trTemp.fStartSolid ||
				if( trTemp.fAllSolid){
					continue;
				}

				
				//DebugLine_Setup(6, surfaceGuessAboveWaterLevel, m_hEnemy->Center(), trTemp.flFraction );



				if(!traceResultObstructionValidForAttack(trTemp)){
					continue;
				}

				//last check: can we get a route to this point?
				if(BuildRoute(surfaceGuess + Vector(0, 0, -60), bits_MF_TO_LOCATION, NULL)){
					//okay!	
				}else{
					//oh dear.  Any other tries like BuildNearestRoute?
					continue;
				}

				//got down here? I suppose that is success, assume there is a path to follow.
				//And mark the current point for coming back to later maybe.
				preSurfaceAttackLocation = pev->origin;
				TaskComplete();
				return;

			}//END OF while tries remain

			//didn't TaskComplete() and end early above? we failed to find a good point.
			TaskFail();

		break;}
		case TASK_ARCHER_GATE_UPDATE_LKP_AT_WATER_SURFACE_IF_UNOBSCURED:{
			if(m_hEnemy == NULL){
				//???
				TaskFail();
				return;
			}

			//Is it still possible for me to do a straight-line attack from a little above the surface point I picked?
			TraceResult trTemp;
			
			float waterLevel = UTIL_WaterLevel(pev->origin, pev->origin.z, pev->origin.z + 4096.0);
			Vector originAtWaterLevel = Vector(pev->origin.x, pev->origin.y, waterLevel);
			Vector littleAboveThat = originAtWaterLevel + Vector(0, 0, 12);

			//yes, the real enemy position. cheat and just grab it straight.  instead of this->m_vecEnemyLKP
			UTIL_TraceLine(littleAboveThat, m_hEnemy->Center(), dont_ignore_monsters, edict(), &trTemp);

			//trTemp.fStartSolid || 
			if(trTemp.fAllSolid){
				//nope.
				TaskFail(); return;
			}

			if(!traceResultObstructionValidForAttack(trTemp)){
				TaskFail(); return;
			}

			// if I made it here, seems fine. pass.  And set the LKP to where the enemy is now.

			setEnemyLKP(m_hEnemy.GetEntity());
			TaskComplete();


		break;}
		case TASK_ARCHER_SEEK_WATER_SURFACE_ATTACK_POINT:{
			//we're just below. should be a clear route to the top.
			float waterLevel = UTIL_WaterLevel(pev->origin, pev->origin.z, pev->origin.z + 4096.0);
			Vector originAtWaterLevel = Vector(pev->origin.x, pev->origin.y, waterLevel + DESIRED_WATERLEVEL_SURFACE_OFFSET);
			
			//TODO - set surface animation?  wait until so far from the top? what way to do it?
			this->SetSequenceByIndex(SEQ_ARCHER_SURFACE, 1.7f);


			//TODO - can I make it there? does this need to be offset above or below a little or need a Probe exception to allow going higher?
			
			
			
			//if(BuildRoute(originAtWaterLevel, bits_MF_TO_LOCATION, NULL)){
			if(TRUE){
				int iMoveFlag = bits_MF_TO_LOCATION;
				RouteNew();
				m_movementGoal = MOVEGOAL_LOCATION;
				m_vecMoveGoal = originAtWaterLevel;
				
				m_Route[ 0 ].vecLocation = m_vecMoveGoal;
				m_Route[ 0 ].iType = iMoveFlag | bits_MF_IS_GOAL;
				m_iRouteLength = 1;


				//okay!	
			}else{
				//oh dear.  Any other tries like BuildNearestRoute?
				TaskFail(); return;
			}

			TaskComplete();

		break;}
		case TASK_ARCHER_SEEK_WATER_SUBMERGE:{
			//go back down?
			Vector originLower = pev->origin + Vector(0, 0, -8);
			float waterLevel = UTIL_WaterLevel(originLower, originLower.z, originLower.z + 4096.0);
			
			//TODO - set submerge animation? wait until so far from the top? what way to do it?
			this->SetSequenceByIndex(SEQ_ARCHER_SURFACE, -1.7f);

			Vector wellBelowOrigin = Vector(pev->origin.x, pev->origin.y, waterLevel - 60);
			
			if(BuildRoute(wellBelowOrigin, bits_MF_TO_LOCATION, NULL)){
				//okay!	
			}else{
				//oh dear.  Any other tries like BuildNearestRoute?
				TaskFail(); return;
			}

			TaskComplete();

		break;}
		case TASK_ARCHER_SEEK_RETREAT_INTO_WATER:{
			//go back to the previous point, or pick some random point below the surface if it isn't possible to.  Or scramble anyways, whatever.

			float waterLevel = UTIL_WaterLevel(pev->origin, pev->origin.z, pev->origin.z + 4096.0);
			BOOL pathSuccess;

			//::DebugLine_SetupPoint(5, preSurfaceAttackLocation, 0, 0, 255);

			if(BuildRoute(preSurfaceAttackLocation, bits_MF_TO_LOCATION, NULL)){
				//okay!	
				pathSuccess = TRUE;
			}else{
				//oh.
				pathSuccess = FALSE;
			}


			if(!pathSuccess){
				
				if(attemptBuildRandomWanderRoute(waterLevel)){
					//at least that worked.
					TaskComplete();
					return;
				}


				// this is no good, the loop above would've ended early if it passed instead.
				TaskFail();
				return;

			}//END OF pathSuccess check


			TaskComplete();

		break;}
		case TASK_ARCHER_SEEK_RANDOM_WANDER_POINT:{
			//Perhaps it would help to try from someplace else?
			
			float waterLevel = UTIL_WaterLevel(pev->origin, pev->origin.z, pev->origin.z + 4096.0);

			if(attemptBuildRandomWanderRoute(waterLevel)){
				TaskComplete();
				return;
			}

			//I fail at responding to a failure? GAH this is bad.
			TaskFail();
			return;

		break;}



		case TASK_DIE:{
			//just do what the parent does.
			CFlyingMonster::StartTask(pTask);
		break;}

		case TASK_RANGE_ATTACK1:{

			if(pev->spawnflags & SF_ARCHER_LONG_RANGE_LOGIC){
				// hmm.
				shootCooldown = gpGlobals->time + RANDOM_LONG(4.0, 6.8);
			}else{
				// not long range logic?  Don't get too spammy, this can get called for often
				if(g_iSkillLevel == SKILL_HARD){
					shootCooldown = gpGlobals->time + RANDOM_LONG(5.2, 6.2);
				}else if(g_iSkillLevel == SKILL_MEDIUM){
					shootCooldown = gpGlobals->time + RANDOM_LONG(5.8, 6.7);
				}else{
					shootCooldown = gpGlobals->time + RANDOM_LONG(6.2, 7.3);
				}
			}
			CFlyingMonster::StartTask(pTask);
		break;}
		case TASK_STOP_MOVING:{
			//why so instant?
			//this->m_velocity = Vector(0, 0, 0);
			//pev->velocity = Vector(0,0,0);
			CFlyingMonster::StartTask(pTask);

		break;}

		case TASK_WATER_DEAD_FLOAT:{
			//pev->skin = EYE_BASE;
			//SetSequenceByName( "bellyup" );
			SetSequenceByIndex(SEQ_ARCHER_DEAD_FLOAT);
			CFlyingMonster::StartTask(pTask);
		break;}


		default:{
			CFlyingMonster::StartTask( pTask );
		break;}
	}//END OF switch

}//END OF StartTask

void CArcher::RunTask( Task_t *pTask ){
	
	//EASY_CVAR_PRINTIF_PRE(templatePrintout, easyPrintLine("RunTask: sched:%s task:%d", this->m_pSchedule->pName, pTask->iTask) );
	
	switch( pTask->iTask ){
		case TASK_MELEE_ATTACK1:{

			if(!HasConditions(bits_COND_CAN_MELEE_ATTACK1)){
				// start the timer to end early, reacting instantly can be odd
				if(interruptMeleeTimer == -1){
					interruptMeleeTimer = gpGlobals->time + 0.38;
				}

				if(gpGlobals->time >= interruptMeleeTimer){
					// long enough?  end.
					TaskComplete();
					return;
				}
			}

			// proceed
			CFlyingMonster::RunTask(pTask);
		}break;
		case TASK_RANGE_ATTACK1:{
			MakeIdealYaw ( m_vecEnemyLKP );
			ChangeYaw ( pev->yaw_speed );

			if ( m_fSequenceFinished )
			{
				//m_Activity = ACT_IDLE;
				//m_IdealActivity?
				SetActivity(ACT_HOVER);
				TaskComplete();
			}
		break;}
		case TASK_MOVE_TO_ENEMY_RANGE:{
			//Should I be interrupted by noticing the enemy leaves the water?
			
			if(m_hEnemy==NULL || m_hEnemy->pev->waterlevel == 0){
				TaskFail();
				return;
			}
			
			CBaseMonster::RunTask(pTask);

		break;}

		case TASK_ARCHER_WAIT_FOR_MOVEMENT_STRICT:{
			/*
			if(EASY_CVAR_GET_DEBUGONLY(movementIsCompletePrintout) == 1){
				easyPrintLine("%s:%d: IS MOVEMENT COMPLETE?: %d", getClassname(), monsterID, MovementIsComplete());
				easyPrintLine("MOVEGOAL: %d", this->m_movementGoal);

				if(this->m_movementGoal == MOVEGOAL_LOCATION){
					UTIL_printLineVector("GOAL LOC:", this->m_vecMoveGoal);
				}

			}
			*/
			if (MovementIsComplete())
			{
				TaskComplete();
				RouteClear();		// Stop moving
			}

		break;}



		default:{
			CFlyingMonster::RunTask(pTask);
		break;}
	}//END OF switch

}//END OF RunTask



BOOL CArcher::CheckMeleeAttack1( float flDot, float flDist ){

	//of course no ev->flags, FL_ONGROUND check, they may be swimmin';
	if ( flDist <= 64 && flDot >= 0.7 && m_hEnemy != NULL )
	{
		// noting unusual?
		meleeAttackIsDirect = FALSE;
		return TRUE;
	}
	
	// ALTERNATE CONDITION:  if the enemy is standing on me, allow
	float enemyLowerBound = m_hEnemy->pev->origin.z + m_hEnemy->pev->mins.z;
	float myUpperBound = this->pev->origin.z + this->pev->maxs.z;

	if(flDist < 40 && enemyLowerBound < myUpperBound + 5){
		// mention this, the default TRACEHULL that looks in front may miss em'
		meleeAttackIsDirect = TRUE;
		return TRUE;
	}


	return FALSE;
}
BOOL CArcher::CheckMeleeAttack2( float flDot, float flDist ){
	return FALSE;
}
BOOL CArcher::CheckRangeAttack1( float flDot, float flDist ){

	//DEBUG - why you no work!!!??

	if(gpGlobals->time >= shootCooldown){
		//past cooldown? allowed.
	}else{
		// no.
		return FALSE;
	}


	//if ( flDot > 0.5 && flDist > 256 && flDist <= 2048 )
	// reduce range a it, 700 is plenty (was 1024)
	if ( flDot > 0.5 && flDist <= 700 )
	{
		//easyForcePrintLine("YAY?!");
		return TRUE;
	}
	//easyForcePrintLine("NAY?!");
	return FALSE;
}
BOOL CArcher::CheckRangeAttack2( float flDot, float flDist ){
	return FALSE;
}



void CArcher::CustomTouch( CBaseEntity *pOther ){
	int x = 46;

	if(pOther == NULL){
		return; //??????
	}

	// hitting the ground without a waterlevel?  Goodbye
	//  'pev->flags & FL_ONGROUND'  isn't set on time?  ah well.
	// WAIT.  If knocked back into a wall, even over a pool of water, this counts as dead.  Jeez.
	// Check the global trace, is this a floor plane?
	if(pev->waterlevel == 0 && pev->deadflag == DEAD_NO){
		TraceResult tr = UTIL_GetGlobalTrace();
		if(tr.vecPlaneNormal.z > 0.7){
			if(lackOfWaterTimer == -1){
				// set it
				// nah, die close to instantly, not really time to flesh this out more yet.
				// Maybe a rotate if dying on the ground, although pointless if the floating dead form sinks to hit the ground.
				// That would need to do the same check to start a rotate-into-place.
				//lackOfWaterTimer = gpGlobals->time + RANDOM_FLOAT(8, 12);
				lackOfWaterTimer = gpGlobals->time + RANDOM_FLOAT(1.1, 1.8);
			}
		}
	}
	
}//CustomTouch






void CArcher::MonsterThink(){

	
	//MODDD - DEBUG TIME. do I go above the water?

	/*
	m_flGroundSpeed = 100;
	m_flightSpeed = 100;
	MoveExecute(NULL, Vector(0, 0, 1), gpGlobals->frametime);
	pev->nextthink = gpGlobals->time + 0.1;

	return;
	*/
	////////////////////////////////////////////////////////////////////////////////



	if(pev->waterlevel != waterLevelMem){
		waterLevelMem = pev->waterlevel;
		if(pev->waterlevel == 0){
			// oh dear, that ding-dang ol' gravity's at it again
			pev->movetype = MOVETYPE_TOSS;
			pev->gravity = 1.0;
			// set just in case?
			lackOfWaterTimer = gpGlobals->time + 5;
		}else{
			// ahh, I can breathe
			lackOfWaterTimer = -1;

			pev->movetype = MOVETYPE_FLY;
			pev->gravity = 0.0;
		}
	}

	if(pev->deadflag == DEAD_NO && lackOfWaterTimer != -1 && gpGlobals->time >= lackOfWaterTimer){
		// Not -1, past it?  Goodbye
		Killed(NULL, NULL, GIB_NORMAL);
	}




	//easyForcePrintLine("IM GONNA %d %d", m_Activity, m_IdealActivity);
	//easyForcePrintLine("MY EYES: %.2f %.2f %.2f", pev->view_ofs.x,pev->view_ofs.y,pev->view_ofs.z);

	/*
	if(pev->deadflag == DEAD_NO && lastVelocityChange != -1 && (gpGlobals->time - lastVelocityChange) > 0.24  ){
		//no edits to velocity?  Start slowing down a lot.
		m_velocity = m_velocity * 0.15;
		pev->velocity = m_velocity;
		lastVelocityChange = gpGlobals->time;
	}else{

	}
	*/

	BOOL isItMovin = this->IsMoving();

	// why DEAD_NO here?  maybe this would mess with floating logic
	if(pev->waterlevel != 0 && pev->deadflag == DEAD_NO && !isItMovin){
		// reduce our velocity each turn.

		if( (m_velocity.Length() > 0 || pev->velocity.Length() > 0) ){
			m_velocity = m_velocity * 0.74;
			if(m_velocity.Length() < 0.02){
				// just stop already.
				m_velocity = Vector(0, 0, 0);
			}
			pev->velocity = m_velocity;
		}
	}



	//CFlyingMonster::MonsterThink();
	CBaseMonster::MonsterThink();
}//END OF MonsterThink

// PrescheduleThink - this function runs after conditions are collected and before scheduling code is run.
//NOTE - PrescheduleThink is called by RunAI of monsterstate.cpp, which is called from MonsterThink in the parent CFlyingMonster class (monsters.cpp).
//The "MonsterThink" below still occurs earlier than PrescheduleThink
void CArcher::PrescheduleThink (){



	CFlyingMonster::PrescheduleThink();
}//END OF PrescheduleThink










int CArcher::Classify(){
	return CLASS_ALIEN_MONSTER;
}
BOOL CArcher::isOrganic(){
	return TRUE;
}

int CArcher::IRelationship( CBaseEntity *pTarget ){

	return CFlyingMonster::IRelationship(pTarget);
}//END OF IRelationship


void CArcher::ReportAIState(){
	//call the parent, and add on to that.
	CFlyingMonster::ReportAIState();
	//print anything special with easyForcePrintLine
}//END OF ReportAIState()





GENERATE_TRACEATTACK_IMPLEMENTATION(CArcher)
{



	GENERATE_TRACEATTACK_PARENT_CALL(CFlyingMonster);
}

GENERATE_TAKEDAMAGE_IMPLEMENTATION(CArcher)
{
	
	//CFlyingMonster already calls PainSound.
	//PainSound();


	return GENERATE_TAKEDAMAGE_PARENT_CALL(CFlyingMonster);
}



//NOTE - called by CFlyingMonster's TakeDamage method. If that isn't called, DeadTakeDamage won't get called naturally.
GENERATE_DEADTAKEDAMAGE_IMPLEMENTATION(CArcher)
{



	return GENERATE_DEADTAKEDAMAGE_PARENT_CALL(CFlyingMonster);
}//END OF DeadTakeDamage




//Parameters: integer named fGibSpawnsDecal
// The parent method calls GIBMONSTERGIB, then GIBMONSTERSOUND and GIBMONSTEREND, passing along to the latter two whether GIBMONSTERGIB
// really "gibbed" or not (it must return whether it did or not).
// This default behavior is ok, change pieces at a time instead in other methods. GIBMONSTERGIB is the most likely one that needs customization,
// such as only spawning two alien gibs for the ChumToad. The others are good as defaults.
GENERATE_GIBMONSTER_IMPLEMENTATION(CArcher)
{
	GENERATE_GIBMONSTER_PARENT_CALL(CFlyingMonster);
}



//Parameters: BOOL fGibSpawnsDecal
//Returns: BOOL. Did this monster spawn gibs and is safe to stop drawing?
// If this monster has a special way of spawning gibs or checking whether to spawn gibs, handle that here and remove the parent call.
// The parent GIBMONSTERGIB method is supposed to handle generic cases like determining whether to spawn human or alien gibs based on
// Classify(), or robot gibs under german censorship. This implementation can be specific to just this monster instead.
//NOTICE - do NOT do something special here AND call the parent method through GENERATE_GIBMONSTERGIB_PARENT_CALL.
//Anything done here is meant to completely replace how the parent method gibs a monster in general. None of it is required.
GENERATE_GIBMONSTERGIB_IMPLEMENTATION(CArcher)
{
	// nothing special.
	return GENERATE_GIBMONSTERGIB_PARENT_CALL(CFlyingMonster);
}

//The other related methods, GIBMONSTERSOUND and GIBMONSTEREND, are well suited to the majority of cases.




GENERATE_KILLED_IMPLEMENTATION(CArcher)
{

	/*

	BOOL firstCall = FALSE;
	if(pev->deadflag == DEAD_NO){
		//keep this in mind...
		firstCall = TRUE;

		//Only reset the velocity if this is the first Killed call (since we stop following).
		//Any further resets will look like gravity suddenly stops with each shot (Killed call again).
		pev->velocity = Vector(0, 0, 0);
		m_velocity = Vector(0, 0, 0);
	}

	//MODDD - is still doing here ok?
	GENERATE_KILLED_PARENT_CALL(CFlyingMonster);

	//HACK. guarantee we fall to the ground, even if killed while in the DEAD_DYING state.
	//...which forces MOVETYPE_STEP, and is not very good.

	pev->movetype = MOVETYPE_TOSS;
	*/


	//Copy of how the ichy does it to want to float to the top. Or part of it.
	
	//Is calling direct parent CFlyingMonster instead of CBaseMonster ok?
	GENERATE_KILLED_PARENT_CALL(CFlyingMonster);
	pev->velocity = Vector( 0, 0, 0 );


	if(pev->waterlevel != 0){
		//MODDD NOTE
		//Forcing the movetype to MOVETYPE_STEP in CFlyingMonster's Killed above can be bad for the floating logic.
		//Or flyers in general ever, MOVETYPE_TOSS falls just fine. no idea why
		pev->movetype = MOVETYPE_FLY;
	}


	/*
	//if you have the "FL_KILLME" flag, it means this is about to get deleted (gibbed). No point in doing any of this then.
	if(firstCall && !(pev->flags & FL_KILLME) ){
		cheapKilledFlyer();
	}//END OF firstCall check
	*/

}//END OF Killed



void CArcher::SetYawSpeed( void ){
	int ys;
	ys = 120;
	//ys = 200;

	//Switch on current activity m_Activity to determine yaw speed and set it?
	
	switch ( m_Activity )
	{
	case ACT_IDLE:
		//ys = 100;
	break;
	}

	pev->yaw_speed = ys;
}//END OF SetYawSpeed



BOOL CArcher::getMonsterBlockIdleAutoUpdate(void){
	return FALSE;
}
BOOL CArcher::forceIdleFrameReset(void){
	return FALSE;
}
//not so good for me?
BOOL CArcher::canPredictActRepeat(void){
	return FALSE;
}
BOOL CArcher::usesAdvancedAnimSystem(void){
	return TRUE;
}


void CArcher::SetActivity(Activity NewActivity ){

	// Little check. If this is to change IDLE to HOVER or HOVER to IDLE, don't reset the animation.
	// it's fine the way it is, the are one and the same.

	if((NewActivity == ACT_IDLE || NewActivity == ACT_HOVER) &&
		(m_Activity == ACT_IDLE || m_Activity == ACT_HOVER)){

		m_Activity = ACT_HOVER;
		m_IdealActivity = ACT_HOVER;
		return;
	}else{
		//proceed normally.
		CFlyingMonster::SetActivity(NewActivity);
	}

}//END OF SetActivity

Activity CArcher::GetDeathActivity ( void ){

	// && pev->flags & FL_ONGROUND
	if(pev->waterlevel == 0){
		// enforce ACT_DIESIMPLE, just in case more activities are ever possible
		return ACT_DIESIMPLE;
	}

	return CFlyingMonster::GetDeathActivity();
}

//IMPORTANT. To get the animation from the model the usual way, you must use "CBaseAnimating::LookupActivity(activity);" to do so,
//           do NOT forget the "CBaseAnimating::" in front and do NOT use any other base class along the way, like CSquadMonster or CFlyingMonster.
//           Need to call CBaseAnimating's primitive version because CFlyingMonster's LookupActivity may now call LookupActivityHard or tryActivitySubstitute
//           on its own, which would trigger an infinite loop of calls (stack overflow):
//           * This class calling LookupActivityHard, persay, falling back to calling...
//           * Monster's LookupActivity, deciding to call the...
//           * Child class's LookupActiivtyHard for a possible substitution, falling back to calling...
//           * Monster's LookupActivity, deciding to call the...
//           * Child class's LookupActivityHard for a possible substitution, falling back to...
//           Repeat the last two ad infinitum. Crash.

int CArcher::tryActivitySubstitute(int activity){
	int i = 0;

	//no need for default, just falls back to the normal activity lookup.
	switch(activity){
		//Let whoever know we have these anims.
		case ACT_DIESIMPLE:
			// eh, let this default.  Any real logic's for LookupActivityHard anyway
		break;
		case ACT_IDLE:
			return SEQ_ARCHER_IDLE1;
		break;
		case ACT_WALK:
		case ACT_RUN:
			//return SEQ_ARCHER_IDLE1;
			return ARCHER_SWIM_SEQ;
		break;
		case ACT_MELEE_ATTACK1:
			return SEQ_ARCHER_BITE;
		break;
		case ACT_RANGE_ATTACK1:
			return SEQ_ARCHER_SHOOT;
		break;
		//Do these break anything?
		case ACT_FLY:
			return ARCHER_SWIM_SEQ;
		break;
		case ACT_HOVER:
			return ARCHER_SWIM_SEQ;
		break;
	}//END OF switch


	//not handled by above? Rely on the model's anim for this activity if there is one.
	return CBaseAnimating::LookupActivity(activity);
}//END OF tryActivitySubstitute


int CArcher::LookupActivityHard(int activity){
	int i = 0;
	m_flFramerateSuggestion = 1;
	pev->framerate = 1;
	m_iForceLoops = -1;  //signal to leave looping up to the model.
	//any animation events in progress?  Clear it.
	resetEventQueue();

	//Within an ACTIVITY, pick an animation like this (with whatever logic / random check first):
	//    this->animEventQueuePush(10.0f / 30.0f, 3);  //Sets event #3 to happen at 1/3 of a second
	//    return LookupSequence("die_backwards");      //will play animation die_backwards

	//no need for default, just falls back to the normal activity lookup.
	switch(activity){
		case ACT_DIESIMPLE:
			//&& pev->flags & FL_ONGROUND
			if(pev->waterlevel == 0 ){
				// Dying on land?  Only use the 2nd one, and faster
				m_flFramerateSuggestion = 1.6f;
				return SEQ_ARCHER_DIE2;
			}

			return CBaseAnimating::LookupActivity(activity);
		break;
		case ACT_IDLE:{
			// pick em'
			float randoVal = RANDOM_FLOAT(0, 1);

			if(randoVal < 0.4){
				return SEQ_ARCHER_IDLE1;
			}else if(randoVal < 0.7){
				return SEQ_ARCHER_IDLE2;
			}else{
				return SEQ_ARCHER_IDLE3;
			}
		}break;
		case ACT_WALK:
		case ACT_RUN:
			// what?  we have a swim anim, why not use it?
			// also force this, safety
			m_flGroundSpeed = m_flightSpeed = 200;
			
			return ARCHER_SWIM_SEQ;
		break;
		case ACT_TURN_LEFT:
		case ACT_TURN_RIGHT:
			m_iForceLoops = TRUE;
			//otherwise what is returned is fine.
		break;
		case ACT_MELEE_ATTACK1:
			animEventQueuePush(4.0f / 13.0f , 1);
			animEventQueuePush(14.0f / 13.0f , 1);
			return SEQ_ARCHER_BITE;
		break;
		case ACT_RANGE_ATTACK1:
			
			animEventQueuePush(10.8f / 18.0f, 0);
			m_iForceLoops = FALSE;
			return SEQ_ARCHER_SHOOT;
		break;
		//Do these break anything?
		case ACT_FLY:
			return ARCHER_SWIM_SEQ;
		break;
		case ACT_HOVER:
			return ARCHER_SWIM_SEQ;
		break;
	}//END OF switch
	
	
	//not handled by above?  try the real deal.
	return CBaseAnimating::LookupActivity(activity);
}//END OF LookupActivityHard


//Handles custom events sent from "LookupActivityHard", which sends events as timed delays along with picking an animation in script.
//So this handles script-provided events, not model ones.
void CArcher::HandleEventQueueEvent(int arg_eventID){

	switch(arg_eventID){
	case 0:
	{
		/*
		//break;

		//fire a bullsquid projectile? not psychic like the kingpin.
		//No, still can have electricity based stuff. Do that.
		Vector	vecSpitOffset;
		Vector	vecSpitDir;

		UTIL_MakeVectors ( pev->angles );

		vecSpitOffset = ( pev->origin + gpGlobals->v_right * 0 + gpGlobals->v_forward * 18 + gpGlobals->v_up * 22 );		
		
		vecSpitDir = ( ( m_vecEnemyLKP +  ((m_hEnemy!=NULL)?(m_hEnemy->EyeOffset()):(Vector(0,0,5)))   ) - vecSpitOffset ).Normalize();
		
		vecSpitDir.x += RANDOM_FLOAT( -0.05, 0.05 );
		vecSpitDir.y += RANDOM_FLOAT( -0.05, 0.05 );
		vecSpitDir.z += RANDOM_FLOAT( -0.05, 0 );

		// do stuff for this event.
		AttackSound();

		CSquidSpit::Shoot( this, vecSpitOffset, vecSpitDir, 900 );
		*/

		if(m_hEnemy == NULL){
			//stop?
			TaskFail();
			return;
		}

			
		//m_IdealActivity = ACT_RANGE_ATTACK1;

		//create the ball
		Vector vecStart, angleGun;
		Vector vecForward;
		TraceResult tr;



		//GetAttachment( 0, vecStart, angleGun );
			
		::UTIL_MakeAimVectorsPrivate(pev->angles, vecForward, NULL, NULL);

		vecStart = pev->origin + vecForward * 18 + Vector(0, 0, 60);

		//HOLD ON. Do a test. Does vecStart still go in valid space?
		UTIL_TraceLine(pev->origin, vecStart, dont_ignore_monsters, this->edict(), &tr);


		//!tr.fStartSolid && !
		if(!tr.fAllSolid && tr.flFraction >= 1.0){
			//pass, go ahead and use this "vecStart" to spawn something.
		}else{
			//no? try this intead.
			vecStart = pev->origin + vecForward * 66;
		}

		MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
			WRITE_BYTE( TE_ELIGHT );
			WRITE_SHORT( entindex( ) + 0x1000 );		// entity, attachment
			WRITE_COORD( 0 );		// origin
			WRITE_COORD( 0 );
			WRITE_COORD( 0 );
			WRITE_COORD( 32 );	// radius
			WRITE_BYTE( 0 );	// R
			WRITE_BYTE( 62 );	// G
			WRITE_BYTE( 255 );	// B
			WRITE_BYTE( 10 );	// life * 10
			WRITE_COORD( 32 ); // decay
		MESSAGE_END();


		int pitch = 113 + RANDOM_LONG(0, 8);
		UTIL_PlaySound( edict(), CHAN_VOICE, "x/x_shoot1.wav", 1.0, ATTN_NORM - 0.6f, 0, pitch );

		CBaseMonster *pBall = (CBaseMonster*)Create( "archer_ball", vecStart, pev->angles, edict() );

		pBall->pev->velocity = Vector( vecForward.x * 100, vecForward.y * 100, 0 );
		pBall->m_hEnemy = m_hEnemy;


		//should there be some generic electricity sound for launching this ball? what does the houndeye do?

		//this->playPsionicLaunchSound();
		//this->SetSequenceByIndex(KINGPIN_PSIONIC_LAUNCH);

	break;
	}
	case 1:
	{
		CBaseEntity* pHurt;
		float dmg = gSkillData.bullsquidDmgBite * 0.5;
		float flDist = 46;

		UTIL_MakeAimVectors(pev->angles);
		Vector offsetVecto = gpGlobals->v_forward * 25;
		//offsetVecto.z += pev->maxs.z * 0.5;


		if(!meleeAttackIsDirect){
			//melee bite?
			pHurt = CheckTraceHullAttack(offsetVecto, flDist, dmg, DMG_SLASH, DMG_BLEEDING );
			
		}else{
			// always hurt the enemy in this case.  
			pHurt = m_hEnemy;

			if(pHurt){
				pHurt->TraceAttack_Traceless(pev, dmg, (m_hEnemy->pev->origin - pev->origin).Normalize(), DMG_SLASH, DMG_BLEEDING);
				pHurt->TakeDamage( pev, pev, dmg, DMG_SLASH, DMG_BLEEDING );
				// Sorry, no extra blood-slash effect for weirdos who stand on top of fish.
				//UTIL_fromToBlood(this, pHurt, dmg, flDist, &targetEnd, &vecStart, &vecEnd);
			}
		}//meleeAttackIsDirect check
		
		if ( pHurt )
		{
			if ( (pHurt->pev->flags & (FL_MONSTER|FL_CLIENT)) && !pHurt->blocksImpact() )
			{
				pHurt->pev->punchangle.x = 5;
				
				pHurt->pev->velocity = pHurt->pev->velocity - gpGlobals->v_forward * 13;
				pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_up * 4;
				//MODDD TODO - Should there be a random velocity applied since this is underwater?
			}
			UTIL_PlaySound( ENT(pev), CHAN_WEAPON, pAttackHitSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackHitSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );
		}else{
			UTIL_PlaySound( ENT(pev), CHAN_WEAPON, pAttackMissSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackMissSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );
		}


		/*
		//MODDD TODO - do this later when we have the sound?
		if (RANDOM_LONG(0,1))
			AttackSound();
		*/

	break;
	}
	}//END OF switch


}//END OF HandleEventQueueEvent


//This handles events built into the model, not custom hard-coded ones (above).
void CArcher::HandleAnimEvent(MonsterEvent_t *pEvent ){
	switch( pEvent->event ){
	case 0:
	{

	break;
	}

	default:
		CFlyingMonster::HandleAnimEvent( pEvent );
	break;
	}//END OF switch
}





//inline
void CArcher::checkTraceLine(const Vector& vecSuggestedDir, const float& travelMag, const float& flInterval, const Vector& vecStart, const Vector& vecRelativeEnd, const int& moveDist){
	checkTraceLine(vecSuggestedDir, travelMag, flInterval, vecStart, vecRelativeEnd, moveDist, TRUE);
}
//Vector& const vecRelstar, ???   Vector& const reactionMove,
//inline
void CArcher::checkTraceLine(const Vector& vecSuggestedDir, const float& travelMag, const float& flInterval, const Vector& vecStart, const Vector& vecRelativeEnd, const int& moveDist, const BOOL canBlockFuture){
	

	//WELL WHAT THE whatIN what IS MOVIN YA.
	//return;

	TraceResult tr;

	//    * moveDist ??
	Vector vecRelativeEndScale = vecRelativeEnd * moveDist;

	if(!tempCheckTraceLineBlock){

		//Vector vecEnd = vecStart + Vector(0, 0, 38);
		UTIL_TraceLine(vecStart, vecStart + vecRelativeEndScale, ignore_monsters, ENT(pev), &tr);
		if(tr.flFraction < 1.0){
			//hit something!

			//Get projection
			// = sugdir - proj. of sugdir onto the normal vector.

			//does this work?
			float dist = tr.flFraction * (float)moveDist;
			float toMove = moveDist - dist;
			//pev->origin = pev->origin + -toMove*vecRelativeEnd;
		
			float timeAdjust = (pev->framerate * EASY_CVAR_GET_DEBUGONLY(animationFramerateMulti) * flInterval);
			
			Vector vecMoveParallel = UTIL_projectionComponent(vecSuggestedDir, tr.vecPlaneNormal).Normalize() * (travelMag * 1);
			//Vector vecMoveParallel = Vector(0,0,0);

			if(timeAdjust == 0){
				//easyPrintLineGroup2("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
				return;
			}else{
				//...
			}

			Vector vecMoveRepel = (tr.vecPlaneNormal*toMove*EASY_CVAR_GET_DEBUGONLY(STUrepelMulti))/1;
			
			//pev->origin = pev->origin + vecMoveParallel;
			////UTIL_MoveToOrigin ( ENT(pev), pev->origin + -toMove*vecRelativeEnd + vecMoveParallel , travelMag, MOVE_STRAFE );
		
			//Vector vecTotalAdjust = vecMoveParallel + vecMoveRepel;
			Vector vecTotalAdjust = vecMoveParallel*timeAdjust + vecMoveRepel;


			//???    + -(toMove*1)*vecRelativeEnd
			//pev->velocity = pev->velocity  + ((vecMoveParallel + vecMoveRepel)/timeAdjust);
			
			//MODDD NOTICE - We have a big problem here.
			/*
			UTIL_MoveToOrigin is nice because it only moves the origin of a given entity (this one) up to so far
			until it collides with anything, other monsters or map geometry, if anything is in the way. 
			A direct pev->origin set does not offer this at all.
			Problem is, UTIL_MoveToOrigin can also hang on the same corner we are, so it won't move the stuka
			at all past a corner it is caught on because it is "blocked" by that same corner.
			Way to get around: Move one coord at a time, all of the X-ways, then Y-ways, then Z-ways.
			
			//TODO - this still isn't perfect. It would be better to let the direction we're repelling
			//from play a role in whether the X or Y gets to run first for instance, but generally doing x, y, z
			//individually at all is still better than not.
			
			*/
			//JUST SPLIT IT UP!
			Vector vecTotalAdjustX = Vector(vecTotalAdjust.x, 0, 0);
			Vector vecTotalAdjustY = Vector(0, vecTotalAdjust.y, 0);
			Vector vecTotalAdjustZ = Vector(0, 0, vecTotalAdjust.z);

			UTIL_MoveToOrigin ( ENT(pev), pev->origin + vecTotalAdjustX , vecTotalAdjustX.Length(), MOVE_STRAFE );
			UTIL_MoveToOrigin ( ENT(pev), pev->origin + vecTotalAdjustY , vecTotalAdjustY.Length(), MOVE_STRAFE );
			UTIL_MoveToOrigin ( ENT(pev), pev->origin + vecTotalAdjustZ , vecTotalAdjustZ.Length(), MOVE_STRAFE );


			//pev->origin = pev->origin + tr.vecPlaneNormal*toMove*global_repelMulti;
			//easyPrintLineGroup2("MOOO %s: SPEED: %.2f", STRING(tr.pHit->v.classname), travelMag );
			//EASY_CVAR_PRINTIF_PRE(stukaPrintout, UTIL_printLineVector("VECCCC", tr.vecPlaneNormal ) );

			if(canBlockFuture){
				tempCheckTraceLineBlock = TRUE;
			}
			//MODDAHHH 0.91, 5, 0.44, 4.56
			//easyPrintLineGroup2("MODDAHHH %.2f, %d, %.2f, %.2f ", tr.flFraction, moveDist, toMove, (tr.vecEndPos - vecStart).Length());

		}//END OF if(tr.flFraction < 1.0)
	}//END OF if(!tempCheckTraceLineBlock)
	
	if(EASY_CVAR_GET_DEBUGONLY(drawDebugPathfinding2) == 1){
		UTIL_drawLineFrame(vecStart, vecStart + vecRelativeEndScale, 16, 0, 255, 0);
	}

}


inline
void CArcher::checkTraceLineTest(const Vector& vecSuggestedDir, const float& travelMag, const float& flInterval, const Vector& vecStart, const Vector& vecRelativeEnd, const int& moveDist){
	checkTraceLineTest(vecSuggestedDir, travelMag, flInterval, vecStart, vecRelativeEnd, moveDist, TRUE);
}
//Vector& const vecRelstar, ???   Vector& const reactionMove,
inline
void CArcher::checkTraceLineTest(const Vector& vecSuggestedDir, const float& travelMag, const float& flInterval, const Vector& vecStart, const Vector& vecRelativeEnd, const int& moveDist, const BOOL canBlockFuture){
	
	
	//WELL WHAT THE whatIN what IS MOVIN YA.
	//return;

	//tempCheckTraceLineBlock = FALSE;

	TraceResult tr;

	//    * moveDist ??
	Vector vecRelativeEndScale = vecRelativeEnd * moveDist;

	if(!tempCheckTraceLineBlock){

		//Vector vecEnd = vecStart + Vector(0, 0, 38);
		UTIL_TraceLine(vecStart, vecStart + vecRelativeEndScale, ignore_monsters, ENT(pev), &tr);
		if(tr.flFraction < 1.0){
			//hit something!

			//Get projection
			// = sugdir - proj. of sugdir onto the normal vector.

			//does this work?
			float dist = tr.flFraction * (float)moveDist;
			float toMove = moveDist - dist;
			//pev->origin = pev->origin + -toMove*vecRelativeEnd;
		
			float timeAdjust = (pev->framerate * EASY_CVAR_GET_DEBUGONLY(animationFramerateMulti) * flInterval);
			
			Vector vecMoveParallel = UTIL_projectionComponent(vecSuggestedDir, tr.vecPlaneNormal).Normalize() * (travelMag * 1);
			//Vector vecMoveParallel = Vector(0,0,0);

			if(timeAdjust == 0){
				//easyPrintLineGroup2("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
				return;
			}else{
				//...
			}

			Vector vecMoveRepel = (tr.vecPlaneNormal*toMove*EASY_CVAR_GET_DEBUGONLY(STUrepelMulti))/1;
			
			//pev->origin = pev->origin + vecMoveParallel;
			////UTIL_MoveToOrigin ( ENT(pev), pev->origin + -toMove*vecRelativeEnd + vecMoveParallel , travelMag, MOVE_STRAFE );
		
			//Vector vecTotalAdjust = vecMoveParallel + vecMoveRepel;
			Vector vecTotalAdjust = vecMoveParallel*timeAdjust + vecMoveRepel;


			//???    + -(toMove*1)*vecRelativeEnd
			//pev->velocity = pev->velocity  + ((vecMoveParallel + vecMoveRepel)/timeAdjust);
			
			//UTIL_MoveToOrigin ( ENT(pev), pev->origin + vecTotalAdjust , vecTotalAdjust.Length(), MOVE_STRAFE );
			//pev->origin = pev->origin + vecTotalAdjust;

			//MODDD NOTICE - We have a big problem here.
			/*
			UTIL_MoveToOrigin is nice because it only moves the origin of a given entity (this one) up to so far
			until it collides with anything, other monsters or map geometry, if anything is in the way. 
			A direct pev->origin set does not offer this at all.
			Problem is, UTIL_MoveToOrigin can also hang on the same corner we are, so it won't move the stuka
			at all past a corner it is caught on because it is "blocked" by that same corner.
			Way to get around: Move one coord at a time, all of the X-ways, then Y-ways, then Z-ways.
			
			//TODO - this still isn't perfect. It would be better to let the direction we're repelling
			//from play a role in whether the X or Y gets to run first for instance, but generally doing x, y, z
			//individually at all is still better than not.
			
			*/
			//JUST SPLIT IT UP!
			Vector vecTotalAdjustX = Vector(vecTotalAdjust.x, 0, 0);
			Vector vecTotalAdjustY = Vector(0, vecTotalAdjust.y, 0);
			Vector vecTotalAdjustZ = Vector(0, 0, vecTotalAdjust.z);

			UTIL_MoveToOrigin ( ENT(pev), pev->origin + vecTotalAdjustX , vecTotalAdjustX.Length(), MOVE_STRAFE );
			UTIL_MoveToOrigin ( ENT(pev), pev->origin + vecTotalAdjustY , vecTotalAdjustY.Length(), MOVE_STRAFE );
			UTIL_MoveToOrigin ( ENT(pev), pev->origin + vecTotalAdjustZ , vecTotalAdjustZ.Length(), MOVE_STRAFE );




			//easyForcePrintLine("BUT YOU MOVE????? %.2f ", vecTotalAdjust.Length());
			::UTIL_drawLineFrame(pev->origin, pev->origin + vecTotalAdjust,40, 255, 0, 0);

			//pev->origin = pev->origin + tr.vecPlaneNormal*toMove*global_repelMulti;
			//easyPrintLineGroup2("MOOO %s: SPEED: %.2f", STRING(tr.pHit->v.classname), travelMag );
			//EASY_CVAR_PRINTIF_PRE(stukaPrintout, UTIL_printLineVector("VECCCC", tr.vecPlaneNormal ) );

			if(canBlockFuture){
				tempCheckTraceLineBlock = TRUE;
			}
			//MODDAHHH 0.91, 5, 0.44, 4.56
			//easyPrintLineGroup2("MODDAHHH %.2f, %d, %.2f, %.2f ", tr.flFraction, moveDist, toMove, (tr.vecEndPos - vecStart).Length());

		}//END OF if(tr.flFraction < 1.0)
	}//END OF if(!tempCheckTraceLineBlock)
	
	if(EASY_CVAR_GET_DEBUGONLY(drawDebugPathfinding2) == 1){
		UTIL_drawLineFrame(vecStart, vecStart + vecRelativeEndScale, 16, 0, 255, 0);
	}

}




void CArcher::checkFloor(const Vector& vecSuggestedDir, const float& travelMag, const float& flInterval){

	// THIS METHOD BE DEAD YO.
	return;

	/*
	if(turnThatOff){
		//we're not doing the checks in this case.
		return;
	}
	*/
	//UTIL_drawBoxFrame(pev->absmin, pev->absmax, 16, 0, 0, 255);
	if(EASY_CVAR_GET_DEBUGONLY(drawDebugPathfinding2) == 1){
		UTIL_drawBoxFrame(pev->origin + pev->mins, pev->origin + pev->maxs, 16, 0, 0, 255);
	}
	
	int maxX = pev->maxs.x;
	int maxY = pev->maxs.y;
	int maxZ = pev->maxs.z;
	
	int minX = pev->mins.x;
	int minY = pev->mins.y;
	int minZ = pev->mins.z;
	//     Min      Max
	//z = bottom / top
	//x = left / right
	//y = back / forward

	float boundMultiple = 0.7f;
	Vector vecTopRightForward = pev->origin + pev->maxs*boundMultiple;
	
	Vector vecTopLeftForward = pev->origin + Vector(minX, maxY, maxZ)*boundMultiple;
	Vector vecTopRightBackward = pev->origin + Vector(maxX, minY, maxZ)*boundMultiple;
	Vector vecTopLeftBackward = pev->origin + Vector(minX, minY, maxZ)*boundMultiple;

	Vector vecBottomLeftBackward = pev->origin + pev->mins*boundMultiple;
	
	Vector vecBottomLeftForward = pev->origin + Vector(minX, maxY, minZ)*boundMultiple;
	Vector vecBottomRightBackward = pev->origin + Vector(maxX, minY, minZ)*boundMultiple;
	Vector vecBottomRightForward = pev->origin + Vector(maxX, maxY, minZ)*boundMultiple;
	
	//const float root2 = 1.41421356;
	//const float root3 = ?;
	const float root2rec = 0.70710678;
	const float root3rec = 0.57735027;

	/*
	int checkDist = 18;
	int checkDistV = 32;
	
	int checkDistD = 38;
	*/

	int checkDist = EASY_CVAR_GET_DEBUGONLY(STUcheckDistH);
	int checkDistV = EASY_CVAR_GET_DEBUGONLY(STUcheckDistV);
	
	int checkDistD = EASY_CVAR_GET_DEBUGONLY(STUcheckDistD);


	//float Vector push;
	
	if(vecSuggestedDir.x > 0.8){
		//checkCollisionLeft(vecTopLeftForward, 2);
		tempCheckTraceLineBlock = FALSE;
		checkTraceLine(vecSuggestedDir, travelMag, flInterval, vecTopRightForward, Vector(1, 0, 0), checkDist);
		checkTraceLine(vecSuggestedDir, travelMag, flInterval, vecTopRightBackward, Vector(1, 0, 0), checkDist);
		checkTraceLine(vecSuggestedDir, travelMag, flInterval, vecBottomRightForward, Vector(1, 0, 0), checkDist);
		checkTraceLine(vecSuggestedDir, travelMag, flInterval, vecBottomRightBackward, Vector(1, 0, 0), checkDist);
		
	}else if (vecSuggestedDir.x < -0.8){
		
		tempCheckTraceLineBlock = FALSE;
		checkTraceLine(vecSuggestedDir, travelMag, flInterval, vecTopLeftForward, Vector(-1, 0, 0), checkDist);
		checkTraceLine(vecSuggestedDir, travelMag, flInterval, vecTopLeftBackward, Vector(-1, 0, 0), checkDist);
		checkTraceLine(vecSuggestedDir, travelMag, flInterval, vecBottomLeftForward, Vector(-1, 0, 0), checkDist);
		checkTraceLine(vecSuggestedDir, travelMag, flInterval, vecBottomLeftBackward, Vector(-1, 0, 0), checkDist);
		
	}

	if(vecSuggestedDir.y > 0.8){
		//checkCollisionLeft(vecTopLeftForward, 2);
		tempCheckTraceLineBlock = FALSE;
		checkTraceLine(vecSuggestedDir, travelMag, flInterval, vecTopLeftForward, Vector(0, 1, 0), checkDist);
		checkTraceLine(vecSuggestedDir, travelMag, flInterval, vecTopRightForward, Vector(0, 1, 0), checkDist);
		checkTraceLine(vecSuggestedDir, travelMag, flInterval, vecBottomLeftForward, Vector(0, 1, 0), checkDist);
		checkTraceLine(vecSuggestedDir, travelMag, flInterval, vecBottomRightForward, Vector(0, 1, 0), checkDist);
		
	}else if (vecSuggestedDir.y < -0.8){

		tempCheckTraceLineBlock = FALSE;
		checkTraceLine(vecSuggestedDir, travelMag, flInterval, vecTopLeftBackward, Vector(0, -1, 0), checkDist);
		checkTraceLine(vecSuggestedDir, travelMag, flInterval, vecTopRightBackward, Vector(0, -1, 0), checkDist);
		checkTraceLine(vecSuggestedDir, travelMag, flInterval, vecBottomLeftBackward, Vector(0, -1, 0), checkDist);
		checkTraceLine(vecSuggestedDir, travelMag, flInterval, vecBottomRightBackward, Vector(0, -1, 0), checkDist);
	}


	BOOL onGround = FALSE;
	if(!onGround){

		if(vecSuggestedDir.z > 0){
			//checkCollisionLeft(vecTopLeftForward, 2);

			if(vecSuggestedDir.z > 0.3){
				tempCheckTraceLineBlock = FALSE;
				checkTraceLine(vecSuggestedDir, travelMag, flInterval, vecTopLeftForward, Vector(0, 0, 1), checkDistV);
				checkTraceLine(vecSuggestedDir, travelMag, flInterval, vecTopRightForward, Vector(0, 0, 1), checkDistV);
				checkTraceLine(vecSuggestedDir, travelMag, flInterval, vecTopLeftBackward, Vector(0, 0, 1), checkDistV);
				checkTraceLine(vecSuggestedDir, travelMag, flInterval, vecTopRightBackward, Vector(0, 0, 1), checkDistV);
			}
			
			BOOL topLeftForwardCheck = FALSE;
			BOOL topLeftBackwardCheck = FALSE;
			BOOL topRightForwardCheck = FALSE;
			BOOL topRightBackwardCheck = FALSE;

			if(vecSuggestedDir.x > 0.5){
				//do the right ones.
				topRightForwardCheck = TRUE;
				topRightBackwardCheck = TRUE;
			}else if(vecSuggestedDir.x < 0.5){
				topLeftForwardCheck = TRUE;
				topLeftBackwardCheck = TRUE;
			}

			if(vecSuggestedDir.y > 0.5){
				//do the forward ones.
				topRightForwardCheck = TRUE;
				topLeftForwardCheck = TRUE;
			}else if(vecSuggestedDir.y < 0.5){
				topRightBackwardCheck = TRUE;
				topLeftBackwardCheck = TRUE;
			}
			

			//TEST - ENABLE ALL
			topRightForwardCheck = TRUE;
			topLeftForwardCheck = TRUE;
			topRightBackwardCheck = TRUE;
			topLeftBackwardCheck = TRUE;


			tempCheckTraceLineBlock = FALSE; //is that okay?
			if(topRightForwardCheck)checkTraceLine(vecSuggestedDir, travelMag, flInterval, vecTopRightForward, Vector(root3rec, root3rec, -root3rec), checkDistD);
			if(topLeftForwardCheck)checkTraceLine(vecSuggestedDir, travelMag, flInterval, vecTopRightBackward, Vector(root3rec, -root3rec, -root3rec), checkDistD);
			if(topRightBackwardCheck)checkTraceLine(vecSuggestedDir, travelMag, flInterval, vecTopLeftForward, Vector(-root3rec, root3rec, -root3rec), checkDistD);
			if(topLeftBackwardCheck)checkTraceLine(vecSuggestedDir, travelMag, flInterval, vecTopLeftBackward, Vector(-root3rec, -root3rec, -root3rec), checkDistD);
			

			//easyForcePrintLine("AWWWWW SHIT %.2f %.2f", vecSuggestedDir.x, vecSuggestedDir.y);


			/*
			if(vecSuggestedDir.x > 0){
				if(vecSuggestedDir.y > 0){
					checkTraceLine(vecSuggestedDir, travelMag, flInterval, vecTopRightForward, Vector(root3rec, root3rec, -root3rec), checkDistD);
				}else if(vecSuggestedDir.y < 0){
					checkTraceLine(vecSuggestedDir, travelMag, flInterval, vecTopRightBackward, Vector(root3rec, -root3rec, -root3rec), checkDistD);
				}
			}else if(vecSuggestedDir.x < 0){
				if(vecSuggestedDir.y > 0){
					checkTraceLine(vecSuggestedDir, travelMag, flInterval, vecTopLeftForward, Vector(-root3rec, root3rec, -root3rec), checkDistD);
				}else if(vecSuggestedDir.y < 0){
					checkTraceLine(vecSuggestedDir, travelMag, flInterval, vecTopLeftBackward, Vector(-root3rec, -root3rec, -root3rec), checkDistD);
				}
			}
			*/




			//just try bottom checks at least, even with no Z direction. Diagonals can be important.
		}else if (vecSuggestedDir.z <= 0){
		
			if(vecSuggestedDir.z < -0.3){
				tempCheckTraceLineBlock = FALSE;
				checkTraceLine(vecSuggestedDir, travelMag, flInterval, vecBottomLeftForward, Vector(0, 0, -1), checkDistV);
				checkTraceLine(vecSuggestedDir, travelMag, flInterval, vecBottomRightForward, Vector(0, 0, -1), checkDistV);
				checkTraceLine(vecSuggestedDir, travelMag, flInterval, vecBottomLeftBackward, Vector(0, 0, -1), checkDistV);
				checkTraceLine(vecSuggestedDir, travelMag, flInterval, vecBottomRightBackward, Vector(0, 0, -1), checkDistV);
			}

			
			BOOL bottomLeftForwardCheck = FALSE;
			BOOL bottomLeftBackwardCheck = FALSE;
			BOOL bottomRightForwardCheck = FALSE;
			BOOL bottomRightBackwardCheck = FALSE;

			if(vecSuggestedDir.x > 0.5){
				//do the right ones.
				bottomRightForwardCheck = TRUE;
				bottomRightBackwardCheck = TRUE;
			}else if(vecSuggestedDir.x < 0.5){
				bottomLeftForwardCheck = TRUE;
				bottomLeftBackwardCheck = TRUE;
			}

			if(vecSuggestedDir.y > 0.5){
				//do the forward ones.
				bottomRightForwardCheck = TRUE;
				bottomLeftForwardCheck = TRUE;
			}else if(vecSuggestedDir.y < 0.5){
				bottomRightBackwardCheck = TRUE;
				bottomLeftBackwardCheck = TRUE;
			}
			

			//TEST - ENABLE ALL
			bottomRightForwardCheck = TRUE;
			bottomLeftForwardCheck = TRUE;
			bottomRightBackwardCheck = TRUE;
			bottomLeftBackwardCheck = TRUE;

			tempCheckTraceLineBlock = FALSE; //is that okay?
			if(bottomRightForwardCheck)checkTraceLine(vecSuggestedDir, travelMag, flInterval, vecBottomRightForward, Vector(root3rec, root3rec, -root3rec), checkDistD);
			if(bottomLeftForwardCheck)checkTraceLine(vecSuggestedDir, travelMag, flInterval, vecBottomRightBackward, Vector(root3rec, -root3rec, -root3rec), checkDistD);
			if(bottomRightBackwardCheck)checkTraceLine(vecSuggestedDir, travelMag, flInterval, vecBottomLeftForward, Vector(-root3rec, root3rec, -root3rec), checkDistD);
			if(bottomLeftBackwardCheck)checkTraceLine(vecSuggestedDir, travelMag, flInterval, vecBottomLeftBackward, Vector(-root3rec, -root3rec, -root3rec), checkDistD);
			



			/*
			if(vecSuggestedDir.x > 0){
				if(vecSuggestedDir.y > 0){
					checkTraceLine(vecSuggestedDir, travelMag, flInterval, vecBottomRightForward, Vector(root3rec, root3rec, -root3rec), checkDistD);
				}else if(vecSuggestedDir.y < 0){
					checkTraceLine(vecSuggestedDir, travelMag, flInterval, vecBottomRightBackward, Vector(root3rec, -root3rec, -root3rec), checkDistD);
				}
			}else if(vecSuggestedDir.x < 0){
				if(vecSuggestedDir.y > 0){
					checkTraceLine(vecSuggestedDir, travelMag, flInterval, vecBottomLeftForward, Vector(-root3rec, root3rec, -root3rec), checkDistD);
				}else if(vecSuggestedDir.y < 0){
					checkTraceLine(vecSuggestedDir, travelMag, flInterval, vecBottomLeftBackward, Vector(-root3rec, -root3rec, -root3rec), checkDistD);
				}
			}
			*/

			
			//checkTraceLineTest(vecSuggestedDir, travelMag, flInterval, vecBottomRightBackward, Vector(root3rec, -root3rec, -root3rec), checkDistD, FALSE);
			

		}
	}//END OF if(!onGround)


}//END OF checkFloor


int CArcher::getLoopingDeathSequence(void){
	//TODO - this is underwater, this may work a little differently. gravity? etc?
	return SEQ_ARCHER_SINK;
}

//Everything should just aim at my head.
Vector CArcher::BodyTarget(const Vector &posSrc){
	return Vector(pev->origin.x, pev->origin.y, pev->origin.z + pev->maxs.z - 8);
}
Vector CArcher::BodyTargetMod(const Vector &posSrc){
	return Vector(pev->origin.x, pev->origin.y, pev->origin.z + pev->maxs.z - 8);
}


void CArcher::onDeathAnimationEnd(){
	

	//wait. Do we want to float like the ichy does?
	//It does nothing special.  Just keep it to what the parent does.
	CFlyingMonster::onDeathAnimationEnd();

}//END OF onDeathAnimationEnd


// I can goto the surface of water to do ranged attacks too. I have to know if I can by seeing where the enemy is.
// And be able to track the enemy above water (from itself being underwater) to even begin to determine that.
BOOL CArcher::SeeThroughWaterLine(void){
	return TRUE;
}//END OF SeeThroughWaterLine

BOOL CArcher::noncombat_Look_ignores_PVS_check(void){
	return TRUE;
}//END OF ignores_PVS_check



//MODDD - mimick how the ichy does this.
void CArcher::BecomeDead( void )
{
	pev->takedamage = DAMAGE_YES;// don't let autoaim aim at corpses.

	// give the corpse half of the monster's original maximum health. 
	pev->health = pev->max_health / 2;
	pev->max_health = 5; // max_health now becomes a counter for how many blood decals the corpse can place.
}//END OF BecomeDead


//TODO - implement the probe?  Ensure the monster stays underwater like the ichy does.
Vector CArcher::DoVerticalProbe(float flInterval)
{
	/*
	float waterLevel = UTIL_WaterLevel(pev->origin, pev->origin.z - 512, pev->origin.z + 4096.0);

	//or pev->velocity.z ?
	float anticipatedZ = pev->origin.z + m_velocity.z * flInterval;

	float differDence = anticipatedZ - waterLevel;

	//30


	//20---------


	//origin: 18.  velocity: 12.



	if(anticipatedZ > waterLevel){
		
		//

		//float tempEEE = ( ((float)(pev->origin.z)) + (anticipatedZ - waterLevel));
		float anticipatedMoveDistance = m_velocity.z * flInterval;

		if(anticipatedMoveDistance < differDence){
			//still only move towards the water at most by this amount.
			return Vector(0, 0, m_velocity.z * 2);
		}else{
			//puts us exactly at the water level if it would've gone past.
			return Vector(0, 0, waterLevel);
		}


		//m_velocity.z = 0;

		return Vector(0, 0, 0);
	}
	*/


	//Vector WallNormal = Vector(0,0,-1); // WATER normal is Straight Down for fish.
	//float frac;

	////or pev->velocity.z ??  m_flightSpeed??
	//Vector Probe = pev->origin + Vector(0, 0, pev->velocity.z);

	//BOOL bBumpedSomething = ProbeZ(pev->origin, Probe, &frac);

	//TraceResult tr;


	//if (bBumpedSomething )
	//{
	//	Vector ProbeDir = Probe - pev->origin;

	//	/*
	//	Vector NormalToProbeAndWallNormal = CrossProduct(ProbeDir, WallNormal);
	//	Vector SteeringVector = CrossProduct( NormalToProbeAndWallNormal, ProbeDir);

	//	float SteeringForce = m_flightSpeed * (1-frac) * (DotProduct(WallNormal.Normalize(), m_SaveVelocity.Normalize()));
	//	if (SteeringForce < 0.0)
	//	{
	//		SteeringForce = -SteeringForce;
	//	}
	//	SteeringVector = SteeringForce * SteeringVector.Normalize();
	//	
	//	return SteeringVector;
	//	*/
	//	//return Vector(0, 0, m_flightSpeed * (1-frac));
	//	return Vector(0, 0, pev->velocity.z * (1-frac));
	//}

	return Vector(0, 0, 0);
}



//See if I can find a point sufficiently below a body of water for wandering towards.
//Nothing to do with the surface attack, but could come from failing to find a good position. It may be easier to find an attack position from a different place anyways.
//But some attempt to be remotely near the enemy may be nice... not that this is always easy to judge as a good metric for what counts as a good attack point
//(target in view, or would be if at the surface). TODO.
//Returns whether a route was successfully built or not.  Tries stop if one is built of course.
BOOL CArcher::attemptBuildRandomWanderRoute(const float& argWaterLevel){

	//perhaps we can pick a random point under the waterlevel to go to?  Even a nearby node at random?  hm.
	int tries = 4;


	while(tries >= 0){
		tries--;

		float depthChoice = RANDOM_FLOAT(450, 800);
		Vector originBelowSurface = Vector(pev->origin.x, pev->origin.y, argWaterLevel - depthChoice);

		float randomSurfaceDir_x = RANDOM_FLOAT(0, 600);
		float randomSurfaceDir_y = RANDOM_FLOAT(0, 600);

		Vector waterHideGuess = originBelowSurface + Vector(randomSurfaceDir_x, randomSurfaceDir_y, 0);

		//can we make a route to there?
		if(BuildRoute(waterHideGuess, bits_MF_TO_LOCATION, NULL)){
			//okay!

		}else{
			//oh.
			continue;
		}

		//made it? success.
		//TaskComplete();
		//return;
		return TRUE;
	}//END OF while tries left


	//ran out of tries? oh well.
	return FALSE;
}//END OF attemptBuildRandomWanderRoute


//let's be safe here...  don't exclude the player being immediately out of view.
BOOL CArcher::FCanCheckAttacks(void){

	// 'always see out of water enemy' ability for long-range logic only
	if(pev->spawnflags & SF_ARCHER_LONG_RANGE_LOGIC){
		if(m_hEnemy == NULL || (m_hEnemy->pev->waterlevel == 3)  ){
			// If there is no enemy or they are in the water with me, do whatever the base class does?
			return CBaseMonster::FCanCheckAttacks();
		}
	}


	// otherwise, for enemies out of water, we can do checks without having a direct line of sight with the enemy.
	if ( !HasConditions( bits_COND_ENEMY_TOOFAR ) )
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


void CArcher::setEnemyLKP(CBaseEntity* theEnt){
	// I want the center for going towards the bulk of the enemy, not the feet
	// Wait, how about like the hornet does, 'BodyTarget' for even more control from the entity?
	//m_vecEnemyLKP = theEnt->Center();
	m_vecEnemyLKP = theEnt->BodyTarget(pev->origin);
	// do I care about zoffset?
	m_flEnemyLKP_zOffset = 0; //theEnt->pev->mins.z;
	m_fEnemyLKP_EverSet = TRUE;
	investigatingAltLKP = FALSE;
}//

int CArcher::getNodeTypeAllowed(void){
	return bits_NODE_WATER;
}

int CArcher::getHullIndexForNodes(void){
	// standard.
	return NODE_FLY_HULL;
}
int CArcher::getHullIndexForGroundNodes(void){
	// wait, not that this should ever happen?  oops
	return NODE_SMALL_HULL;
}

/*
// wait. nevermind, go ahead.  If 'Move' is barely changed, should be safe to use segmentedmove.
void CArcher::usesSegmentedMove(void){
	// safety. Should the stuka also not?
	return FALSE;
}
*/


