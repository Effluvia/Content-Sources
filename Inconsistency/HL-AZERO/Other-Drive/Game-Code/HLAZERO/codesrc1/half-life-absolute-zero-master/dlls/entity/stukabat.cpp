
#include "stukabat.h"
#include "effects.h"
#include "schedule.h"
#include "weapons.h"
//MODDD
#include "flyingmonster.h"
#include "util_model.h"
#include "soundent.h"
#include "defaultai.h"
#include "studio.h"
#include "player.h"
#include "nodes.h"


EASY_CVAR_EXTERN_DEBUGONLY(animationFramerateMulti)

EASY_CVAR_EXTERN_DEBUGONLY(drawDebugPathfinding2)
EASY_CVAR_EXTERN_DEBUGONLY(stukaAdvancedCombat)

EASY_CVAR_EXTERN_DEBUGONLY(STUSpeedMulti)
EASY_CVAR_EXTERN_DEBUGONLY(STUExplodeTest)
EASY_CVAR_EXTERN_DEBUGONLY(STUYawSpeedMulti)
EASY_CVAR_EXTERN_DEBUGONLY(STUDetection)

EASY_CVAR_EXTERN_DEBUGONLY(shutupstuka)

EASY_CVAR_EXTERN_DEBUGONLY(STUrepelMulti)
EASY_CVAR_EXTERN_DEBUGONLY(STUcheckDistH)
EASY_CVAR_EXTERN_DEBUGONLY(STUcheckDistV)
EASY_CVAR_EXTERN_DEBUGONLY(STUcheckDistD)

EASY_CVAR_EXTERN_DEBUGONLY(stukaPrintout)

EASY_CVAR_EXTERN_DEBUGONLY(stukaInflictsBleeding)


/*
#define STUKABAT_DIVE_DISTANCE 600
#define STUKABAT_ATTACK_DISTANCE 300

#define STUKABAT_MOVESPEED_WALK 65
#define STUKABAT_MOVESPEED_HOVER 75
#define STUKABAT_MOVESPEED_FLYING_TURN 280
#define STUKABAT_MOVESPEED_FLYING_CYCLER 350
#define STUKABAT_MOVESPEED_DIVE_CYCLER 410
#define STUKABAT_MOVESPEED_ATTACK_BOMB 480
#define STUKABAT_MOVESPEED_ATTACK_CLAW 80
*/


#define STUKABAT_DIVE_DISTANCE 330
#define STUKABAT_ATTACK_DISTANCE (260 + -(getMeleeAnimScaler()- 1)*80 )

#define STUKABAT_MOVESPEED_WALK 70
#define STUKABAT_MOVESPEED_HOVER 210
#define STUKABAT_MOVESPEED_FLYING_TURN 150
#define STUKABAT_MOVESPEED_FLYING_CYCLER 160
#define STUKABAT_MOVESPEED_DIVE_CYCLER 260
#define STUKABAT_MOVESPEED_ATTACK_BOMB 260
#define STUKABAT_MOVESPEED_ATTACK_CLAW 120




#if REMOVE_ORIGINAL_NAMES != 1
	LINK_ENTITY_TO_CLASS( monster_stukabat, CStukaBat );
#endif

#if EXTRA_NAMES > 0
	LINK_ENTITY_TO_CLASS( stukabat, CStukaBat );
	
	#if EXTRA_NAMES == 2
		LINK_ENTITY_TO_CLASS( stuka, CStukaBat );
	#endif
#endif








enum stukaBat_sequence{  //key: frames, FPS
	SEQ_STUKABAT_LAND_CEILING,
	SEQ_STUKABAT_LAND_GROUND,
	SEQ_STUKABAT_ATTACK_BOMB,
	SEQ_STUKABAT_ATTACK_CLAW,
	SEQ_STUKABAT_DIVE_CYCLER,
	SEQ_STUKABAT_DEATH_FALL_SIMPLE,
	SEQ_STUKABAT_DEATH_FALL_VIOLENT,
	SEQ_STUKABAT_FALL_CYCLER,
	SEQ_STUKABAT_FLINCH_BIG,
	SEQ_STUKABAT_FLINCH_SMALL,
	SEQ_STUKABAT_FLYING_CYCLER,
	SEQ_STUKABAT_FLYING_TURN_LEFT,
	SEQ_STUKABAT_FLYING_TURN_RIGHT,
	SEQ_STUKABAT_HOVER,
	SEQ_STUKABAT_DIE_ON_GROUND,
	SEQ_STUKABAT_FLINCH_ON_GROUND,
	SEQ_STUKABAT_EAT_ON_GROUND,
	SEQ_STUKABAT_DISPLAY_FIDGET_ON_GROUND,
	SEQ_STUKABAT_SUBTLE_FIDGET_ON_GROUND,
	SEQ_STUKABAT_GROUND_WALK,
	SEQ_STUKABAT_SUBTLE_FIDGET,
	SEQ_STUKABAT_PREEN_FIDGET,
	SEQ_STUKABAT_SWING_FIDGET,
	SEQ_STUKABAT_TAKE_OFF_FROM_LAND
	
};



enum
{
	TASK_SOUND_ATTACK = LAST_COMMON_TASK + 1,
	TASK_STUKABAT_LAND_PRE,
	TASK_STUKABAT_LAND,
	TASK_GET_PATH_TO_LANDING,
	TASK_GET_PATH_TO_BESTSCENT_FOOT,
	TASK_ACTION,
	TASK_STUKA_WAIT_FOR_ANIM,

};

//slStukaBatAnimWait

enum
{
	SCHED_STUKABAT_FINDEAT = LAST_COMMON_SCHEDULE + 1,
	SCHED_STUKABAT_ATTEMPTLAND,
	SCHED_STUKABAT_CRAWLTOFOOD,
	SCHED_STUKABAT_EAT,
	SCHED_STUKABAT_IDLE_HOVER,
	SCHED_STUKABAT_IDLE_GROUND,

	//TASK_WAIT_INDEFINITE !!!!!!!!!!!!!!!!!!!!!!
	SCHED_STUKABAT_ANIMWAIT,

	//SCHED_STUKABAT_HOVER_AIR
	//??
	//TASK_STUKA_EAT
	//others come here alone, like
	//SCHED_SCRATCH_ASS
	//...
};


//On adding a new schedule, make sure to see this too in stukabat.cpp (search exactly):
//NOTE: DEFINE_CUSTOM_SCHEDULES



//=========================================================
// AI Schedules Specific to this monster
//=========================================================
//NOTE: as always for Stukabat, based off of the controller.  Likely needs new methods.

// Chase enemy schedule
Task_t tlStukaBatChaseEnemy[] = 
{
	{ TASK_GET_PATH_TO_ENEMY,	(float)128		},

	//{TASK_STRAFE_PATH, (float)128 },
	//is that wise alone..?
	//{ TASK_WAIT_FOR_MOVEMENT,	(float)0		},
	//{ TASK_WAIT,	(float)1		},
	{ TASK_ACTION,	(float)1		},
	
	//CHECK. Is this okay??
	{ TASK_CHECK_STUMPED, (float)1		},

	//TASK_STRAFE_PATH
};

Schedule_t slStukaBatChaseEnemy[] =
{
	{ 
		tlStukaBatChaseEnemy,
		ARRAYSIZE ( tlStukaBatChaseEnemy ),

		/*
		bits_COND_NEW_ENEMY | bits_COND_TASK_FAILED
		//MODDD
		| bits_COND_CAN_RANGE_ATTACK1 | bits_COND_CAN_RANGE_ATTACK2 //???  | bits_COND_SMELL_FOOD
		//| bits_COND_SEE_DISLIKE | bits_COND_SEE_ENEMY | bits_COND_SEE_HATE | bits_COND_SEE_NEMESIS | bits_COND_HEAR_SOUND | bits_COND_SMELL
		*/
		0,

		bits_SOUND_MEAT|bits_SOUND_CARCASS,
		"StukaBatChaseEnemy"
	},
};



//unnecessary
/*
Task_t	tlStukaIdleHang[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_WAIT,				(float)5		},// repick IDLESTAND every five seconds. gives us a chance to pick an active idle, fidget, etc.
};


Schedule_t	slStukaIdleHang[] =
{
	{ 
		tlStukaIdleHang,
		ARRAYSIZE ( tlStukaIdleHang ), 
		bits_COND_NEW_ENEMY		|
		bits_COND_SEE_FEAR		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_HEAR_SOUND	|
		bits_COND_SMELL_FOOD	|
		bits_COND_SMELL			|
		bits_COND_PROVOKED		|
		//MODDD - this is the new part!  Otherwise, a plain copy of "slIdleStand"
		//NO YOU IDIOT!  IT WAS HERE ALL ALONG, LOOK ABOVE JEEZ
		//~an affectionate note to myself.
		//~on a side-note, BLOOD FOR THE BLOOD GOD
		bits_COND_HEAR_SOUND,

		bits_SOUND_COMBAT		|// sound flags
		bits_SOUND_WORLD		|
		bits_SOUND_PLAYER		|
		bits_SOUND_DANGER		|

		bits_SOUND_MEAT			|// scents
		bits_SOUND_CARCASS		|
		bits_SOUND_GARBAGE,
		"IdleStand"
	},
};
*/




Task_t	tlStukaIdleGround[] =
{
	//{ TASK_STOP_MOVING,			0				},
	//???
	{ TASK_SET_ACTIVITY,		(float)ACT_CROUCHIDLE},
	{ TASK_WAIT,				(float)2		},// repick IDLESTAND every five seconds. gives us a chance to pick an active idle, fidget, etc.
};

Schedule_t	slStukaIdleGround[] =
{
	{ 
		tlStukaIdleGround,
		ARRAYSIZE ( tlStukaIdleGround ), 
		bits_COND_NEW_ENEMY		|
		bits_COND_SEE_FEAR		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_HEAR_SOUND	|
		bits_COND_SMELL_FOOD	|
		//bits_COND_SMELL			|
		bits_COND_PROVOKED,

		bits_SOUND_COMBAT		|// sound flags
		bits_SOUND_WORLD		|
		bits_SOUND_PLAYER		|
		bits_SOUND_DANGER		|

		bits_SOUND_MEAT			|// scents
		bits_SOUND_CARCASS		|
		bits_SOUND_GARBAGE,
		"STUIdleGround"
	},
};



Task_t	tlStukaIdleHover[] =
{
	//{ TASK_STOP_MOVING,			0				},
	//???
	{ TASK_SET_ACTIVITY,		(float)ACT_HOVER },
	{ TASK_WAIT,				(float)2		},// repick IDLESTAND every five seconds. gives us a chance to pick an active idle, fidget, etc.
};

Schedule_t	slStukaIdleHover[] =
{
	{ 
		tlStukaIdleHover,
		ARRAYSIZE ( tlStukaIdleHover ), 
		bits_COND_NEW_ENEMY		|
		bits_COND_SEE_FEAR		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_HEAR_SOUND	|
		bits_COND_SMELL_FOOD	|
		//bits_COND_SMELL			|
		bits_COND_PROVOKED,

		bits_SOUND_COMBAT		|// sound flags
		bits_SOUND_WORLD		|
		bits_SOUND_PLAYER		|
		bits_SOUND_DANGER		|

		bits_SOUND_MEAT			|// scents
		bits_SOUND_CARCASS		|
		bits_SOUND_GARBAGE,
		"STUIdleHover"
	},
};



//UNUSED. And beware of "MOVE_TO_TARGET" range, that is different from the ENEMY.
//m_hTarget may or may not be set at all or match the enemy, chances are it will not even be used here.

Task_t	tlStukaBatStrafe[] =
{
	{ TASK_WAIT,					(float)0.2					},
	//{ TASK_GET_PATH_TO_ENEMY,		(float)128					},
	
	{ TASK_MOVE_TO_TARGET_RANGE,	(float)128					},
	

	//{ TASK_WAIT_FOR_MOVEMENT,		(float)0					},
	{ TASK_WAIT,					(float)0.3					},
};

Schedule_t	slStukaBatStrafe[] =
{
	{ 
		tlStukaBatStrafe,
		ARRAYSIZE ( tlStukaBatStrafe ), 
		bits_COND_NEW_ENEMY,
		0,
		"StukaBatStrafe"
	},
};


Task_t	tlStukaBatTakeCover[] =
{
	{ TASK_WAIT,					(float)0.2					},
	{ TASK_FIND_COVER_FROM_ENEMY,	(float)0					},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0					},
	{ TASK_WAIT,					(float)1					},
};

Schedule_t	slStukaBatTakeCover[] =
{
	{ 
		tlStukaBatTakeCover,
		ARRAYSIZE ( tlStukaBatTakeCover ), 
		bits_COND_NEW_ENEMY,
		0,
		"StukaBatTakeCover"
	},
};


//removed "ACT_IDLE", seems more distracting.
Task_t tlStukaBatFail[] =
{
	{ TASK_STOP_MOVING,			0				},

	//{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_WAIT,				(float)1		},
	//{ TASK_WAIT_PVS,			(float)0		},
};

Schedule_t	slStukaBatFail[] =
{
	{
		tlStukaBatFail,
		ARRAYSIZE ( tlStukaBatFail ),
		/*
		bits_COND_NEW_ENEMY			|
		bits_COND_ENEMY_DEAD		|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_ENEMY_OCCLUDED	|
		bits_COND_HEAR_SOUND,
		*/
		0,
		0,
		"StukaBatFail"
	},
};


// secondary range attack
Task_t	tlStukaBatRangeAttack2[] =
{
	//{ TASK_STOP_MOVING,			0				},
	//{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_RANGE_ATTACK2,		(float)0		},
	{ TASK_WAIT_FOR_MOVEMENT,	(float)0		},
};

Schedule_t	slStukaBatRangeAttack2[] =
{
	{
		tlStukaBatRangeAttack2,
		ARRAYSIZE ( tlStukaBatRangeAttack2 ), 
		bits_COND_NEW_ENEMY			|
		bits_COND_ENEMY_DEAD		|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_ENEMY_OCCLUDED	|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"StukaBatRangeAttack2"
	},
};



//{ TASK_GET_PATH_TO_ENEMY,	(float)128		},
//{ TASK_WAIT,	(float)1		},


Task_t tlStukaBatFindEat[] =
{
	//{ TASK_STOP_MOVING,				(float)0				},
	
	{ TASK_GET_PATH_TO_BESTSCENT,	(float)0				},
	//{ TASK_WAIT,	(float)1		}
	{ TASK_WAIT_FOR_MOVEMENT, (float) 0},

};


Schedule_t slStukaBatFindEat[] =
{
	{
		tlStukaBatFindEat,
		ARRAYSIZE( tlStukaBatFindEat ),
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_NEW_ENEMY	,
		
		// even though HEAR_SOUND/SMELL FOOD doesn't break this schedule, we need this mask
		// here or the monster won't detect these sounds at ALL while running this schedule.
		bits_SOUND_MEAT			|
		bits_SOUND_CARCASS,
		"slSBFindEat"
	}
};




Task_t tlStukaBatAttemptLand[] =
{
	//{ TASK_STOP_MOVING,				(float)0				},
	
	{ TASK_GET_PATH_TO_LANDING,	(float)0				},
	{ TASK_WAIT_FOR_MOVEMENT, (float) 0},
	{ TASK_STUKABAT_LAND_PRE, (float) 0},
	{ TASK_STUKABAT_LAND, (float) 0},
	//{ TASK_WAIT,	(float)1		}


};


Schedule_t slStukaBatAttemptLand[] =
{
	{
		tlStukaBatAttemptLand,
		ARRAYSIZE( tlStukaBatAttemptLand ),
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_NEW_ENEMY,
		
		/*
		// even though HEAR_SOUND/SMELL FOOD doesn't break this schedule, we need this mask
		// here or the monster won't detect these sounds at ALL while running this schedule.
		bits_SOUND_MEAT			|
		bits_SOUND_CARCASS,
		*/
		0,
		"slSBAttemptLand"
	}
};



Task_t tlStukaBatCrawlToFood[] =
{
	//{ TASK_STOP_MOVING,				(float)0				},
	
	{ TASK_GET_PATH_TO_BESTSCENT_FOOT,	(float)0				},
	//{ TASK_WAIT,	(float)1		}
	{ TASK_WAIT_FOR_MOVEMENT, (float) 0},


};


Schedule_t slStukaBatCrawlToFood[] =
{
	{
		tlStukaBatCrawlToFood,
		ARRAYSIZE( tlStukaBatCrawlToFood ),
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_NEW_ENEMY,
		
		
		// even though HEAR_SOUND/SMELL FOOD doesn't break this schedule, we need this mask
		// here or the monster won't detect these sounds at ALL while running this schedule.
		bits_SOUND_MEAT			|
		bits_SOUND_CARCASS,
		
		//0,
		"slSBCrawlToFood"
	}
};


Task_t tlStukaBatEat[] =
{
	{ TASK_STOP_MOVING,				(float)0				},
	
	{ TASK_SET_ACTIVITY_FORCE,			(float)ACT_EAT},
	{ TASK_FACE_IDEAL,				(float)0				},
	{ TASK_WAIT,	(float)2		},
	{ TASK_EAT,	(float)50				},
	{ TASK_WAIT,	(float)8		},
	//If in the middle of one eating cycle, go ahead and let it finish.
	{ TASK_WAIT_FOR_SEQUENCEFINISH, (float) 0 },
	//Just send a signal to pick a different activity, this should work out.
	{ TASK_SET_ACTIVITY, (float)ACT_RESET },

	//{TASK_SET_ACTIVITY_FORCE, (float)ACT_CROUCHIDLE},
};


Schedule_t slStukaBatEat[] =
{
	{
		tlStukaBatEat,
		ARRAYSIZE( tlStukaBatEat ),
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_NEW_ENEMY |
		
		/*
		// even though HEAR_SOUND/SMELL FOOD doesn't break this schedule, we need this mask
		// here or the monster won't detect these sounds at ALL while running this schedule.
		bits_SOUND_MEAT			|
		bits_SOUND_CARCASS,
		*/
		bits_COND_SEE_DISLIKE		|
		bits_COND_SEE_HATE      |
		bits_COND_SEE_FEAR		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_HEAR_SOUND	|
		//bits_COND_SMELL_FOOD	|
		//bits_COND_SMELL			|
		bits_COND_PROVOKED,

		bits_SOUND_COMBAT		|// sound flags
		bits_SOUND_WORLD		|
		bits_SOUND_PLAYER		|
		bits_SOUND_DANGER,

		"StukaBatEat"
	}
};


/*
			//no, wrap animation-changes into a schedule!
			//check if blockSetActivity is done?
			if(blockSetActivity == -1){
				TaskComplete();
				return;
			}
			*/
			
Task_t tlStukaBatAnimWait[] =
{
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_STUKA_WAIT_FOR_ANIM,	(float)0		},

};

Schedule_t slStukaBatAnimWait[] =
{
	{
		tlStukaBatAnimWait,
		ARRAYSIZE( tlStukaBatAnimWait ),
		0,
		0,
		"StukaBatWaitForAnim"
	}
};



Task_t	tlStukaPathfindStumped[] =
{
	{ TASK_STOP_MOVING,			0				},
	//{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	//TASK_FACE_IDEAL ?
	{ TASK_FACE_PREV_LKP,			(float)0	},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	//{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },  ???
	{ TASK_WAIT,				(float)10		},
	{ TASK_WAIT_PVS,			(float)0		},
};

Schedule_t	slStukaPathfindStumped[] =
{
	{
		tlStukaPathfindStumped,
		ARRAYSIZE ( tlStukaPathfindStumped ),

		bits_COND_CAN_ATTACK |
		bits_COND_SEE_ENEMY |
		bits_COND_NEW_ENEMY		|
		bits_COND_SEE_FEAR		|
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_HEAR_SOUND	|
		bits_COND_PROVOKED,

		bits_SOUND_COMBAT		|// sound flags
		//bits_SOUND_WORLD		|
		bits_SOUND_PLAYER		|
		bits_SOUND_DANGER		|
		bits_SOUND_BAIT,

		"StukaPathfindStumped"
	},
};







const char *CStukaBat::pAttackHitSounds[] = 
{
	"zombie/claw_strike1.wav",
	"zombie/claw_strike2.wav",
	"zombie/claw_strike3.wav",
};

const char *CStukaBat::pAttackMissSounds[] = 
{
	"zombie/claw_miss1.wav",
	"zombie/claw_miss2.wav",
};

const char *CStukaBat::pAttackSounds[] = 
{
	"stuka/stuka_attack1.wav",
	"stuka/stuka_attack2.wav",
	"stuka/stuka_attack3.wav"
};

const char *CStukaBat::pIdleSounds[] = 
{
	"stuka/stuka_idle1.wav",
	"stuka/stuka_idle2.wav"
};

const char *CStukaBat::pAlertSounds[] = 
{
	"stuka/stuka_alert1.wav",
	"stuka/stuka_alert2.wav"
};

const char *CStukaBat::pPainSounds[] = 
{
	"stuka/stuka_pain1.wav",
	"stuka/stuka_pain2.wav",
	"stuka/stuka_pain3.wav"
};

const char *CStukaBat::pDeathSounds[] = 
{
	"stuka/stuka_death1.wav",
	"stuka/stuka_death2.wav"
};







TYPEDESCRIPTION	CStukaBat::m_SaveData[] = 
{
	//example:
	//DEFINE_ARRAY( CController, m_pBall, FIELD_CLASSPTR, 2 ),
	DEFINE_FIELD(CStukaBat, tempThing, FIELD_INTEGER),
	DEFINE_FIELD(CStukaBat, blockSetActivity, FIELD_TIME),

	DEFINE_FIELD(CStukaBat, eating, FIELD_BOOLEAN),
	DEFINE_FIELD(CStukaBat, eatingAnticipatedEnd, FIELD_TIME),

	DEFINE_FIELD(CStukaBat, suicideAttackCooldown, FIELD_TIME),
	


	DEFINE_FIELD(CStukaBat, onGround, FIELD_BOOLEAN),
	DEFINE_FIELD(CStukaBat, snappedToCeiling, FIELD_BOOLEAN),
	
	DEFINE_FIELD(CStukaBat, queueToggleGround, FIELD_BOOLEAN),
	DEFINE_FIELD(CStukaBat, queueToggleSnappedToCeiling, FIELD_BOOLEAN),
	
	DEFINE_FIELD(CStukaBat, rotationAllowed, FIELD_BOOLEAN),
	
	DEFINE_FIELD(CStukaBat, queueAbortAttack, FIELD_BOOLEAN),
	DEFINE_FIELD(CStukaBat, chargeIndex, FIELD_INTEGER),
	
	DEFINE_FIELD( CStukaBat, m_voicePitch, FIELD_INTEGER),
};

//IMPLEMENT_SAVERESTORE( CStukaBat, CSquadMonster );
int CStukaBat::Save( CSave &save )
{
	if ( !CSquadMonster::Save(save) )
		return 0;
	return save.WriteFields( "CStukaBat", this, m_SaveData, ARRAYSIZE(m_SaveData) );
}
int CStukaBat::Restore( CRestore &restore )
{
	lastVelocityChange = gpGlobals->time;
	//assuming this is okay.

	if ( !CSquadMonster::Restore(restore) )
		return 0;
	return restore.ReadFields( "CStukaBat", this, m_SaveData, ARRAYSIZE(m_SaveData) );
}






//=========================================================
// Classify - indicates this monster's place in the 
// relationship table.
//=========================================================
int CStukaBat::Classify ( void )
{
	//would something more animal-related, like "prey" or "monster" make more sense here?
	//return CLASS_ALIEN_MILITARY;


	//return CLASS_ALIEN_PASSIVE;

	//note that stukabats are not supposed to attack the player unless provoked (I imagine the entire squad / nearby Stukas should also get provoked).
	//For now, using MONSTER to make it hostile to the player for testing aggressive AI
	return CLASS_ALIEN_MONSTER;

}



//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
//(for better or worse, Controller's SetYawSpeed method came almost blank too).
void CStukaBat::SetYawSpeed ( void )
{
	int ys;

	ys = 120;

	/*
	switch ( m_Activity )
	{

	}
	*/

	if(m_Activity == ACT_IDLE){
		ys=50;
	}
	if(m_Activity == ACT_WALK){
		ys=80;
	}
	if(m_Activity == ACT_FLY){
		ys=60;
	}
	if(m_Activity == ACT_EAT || m_Activity == ACT_CROUCHIDLE){
		//ys=260/0.88;
		//.... what
		ys = 140;
	}
	if(m_Activity == ACT_HOVER){
		ys=90;
	}

	EASY_CVAR_PRINTIF_PRE(stukaPrintout, easyForcePrintLine( "ACT SON!!!!! %d", m_Activity) );
	
	//consistent mod.
	ys *= 0.88;

	pev->yaw_speed = ys;
}


float CStukaBat::getMeleeAnimScaler(void){

	switch(g_iSkillLevel){
	case SKILL_EASY:
		return 1.16;
	break;
	case SKILL_MEDIUM:
		return 1.32;
	break;
	case SKILL_HARD:
		return 1.64;
	break;
	default:
		//easy?
		return 1.16;
	break;
	}
}


float CStukaBat::getAttackDelay(void){

	switch(g_iSkillLevel){
	case SKILL_EASY:
		return RANDOM_FLOAT(0.6, 1.1);
	break;
	case SKILL_MEDIUM:
		return RANDOM_FLOAT(0.4, 0.9);
	break;
	case SKILL_HARD:
		return RANDOM_FLOAT(0.3, 0.8);
	break;
	default:
		//easy?
		return RANDOM_FLOAT(0.6, 1.1);
	break;
	}
}



GENERATE_TRACEATTACK_IMPLEMENTATION(CStukaBat)
{
	GENERATE_TRACEATTACK_PARENT_CALL(CSquadMonster);
}

GENERATE_TAKEDAMAGE_IMPLEMENTATION(CStukaBat)
{
	// HACK HACK -- until we fix this.
	//NOTE: perhaps " if(pev->deadFlag == DEAD_NO) " would be more precise?
	//if ( IsAlive() )
	m_afMemory |= bits_MEMORY_PROVOKED;




	
	PRINTQUEUE_STUKA_SEND(stukaPrint.tookDamage, "TOOK DMG");

	//...also, why is this "CBaseMonster" and not "CSquadMonster", the direct parent of Controller OR StukaBat?  Ah well, I guess both work technically (CSquadMonster doesn't affect how takeDamage works)
	//probably just doesn't matter.
	return GENERATE_TAKEDAMAGE_PARENT_CALL(CSquadMonster);
}



GENERATE_KILLED_IMPLEMENTATION(CStukaBat)
{

	//forget everything.
	blockSetActivity = -1;
	attackEffectDelay = -1;
	maxDiveTime = -1;

	queueAbortAttack = FALSE;

	chargeIndex = -1;

	queueToggleGround = FALSE;
	
	//MODDD - not yet! We'll turn this off after picking a fitting death anim.
	//Knowing we were snapped to the ceiling at the time of death (picking an activity / sequence) is important, tells us to fall.
	//snappedToCeiling = FALSE;

	if(snappedToCeiling == TRUE){
		//still do it but...?
		snappedToCeiling = FALSE;
		onGround = FALSE;

		//Teleport me down slightly for safety.
		pev->origin = Vector(pev->origin.x, pev->origin.y, pev->origin.z - 6);
	}
	

	queueToggleSnappedToCeiling = FALSE;



	GENERATE_KILLED_PARENT_CALL(CSquadMonster);

	//just in case?
	pev->movetype = MOVETYPE_TOSS;

}

GENERATE_GIBMONSTER_IMPLEMENTATION_ROUTETOPARENT(CStukaBat, CSquadMonster)


void CStukaBat::PainSound( void )
{
	if(EASY_CVAR_GET_DEBUGONLY(shutupstuka) != 1){
		//NOTE: lifted from "Controller".  Is a 1/3 chance of pain okay?
		if (RANDOM_LONG(0,5) < 2)
			EMIT_SOUND_ARRAY_STUKA_FILTERED( CHAN_VOICE, pPainSounds ); 
	}
}	





void CStukaBat::tryDetachFromCeiling(void){
	
	if(!onGround && snappedToCeiling && blockSetActivity == -1){

		//setAnimation("Take_off_from_ceiling", TRUE, FALSE, 2);
		m_flFramerateSuggestion = -1;
		
		setAnimationSmart("Land_ceiling", -1);
		m_flFramerateSuggestion = 1;

		blockSetActivity = gpGlobals->time + (31.2f/12.0f);
		
		queueToggleSnappedToCeiling = TRUE;
	}
}



void CStukaBat::MakeIdealYaw( Vector vecTarget )
{
	
	CSquadMonster::MakeIdealYaw(vecTarget);

}
//=========================================================
// Changeyaw - turns a monster towards its ideal_yaw
//=========================================================
float CStukaBat::ChangeYaw ( int yawSpeed )
{

	//if alert?

	EASY_CVAR_PRINTIF_PRE(stukaPrintout, easyForcePrintLine("ENEMY??? A %d %d %d", m_pSchedule != slStukaBatAnimWait, this->onGround, this->m_MonsterState));

	if(m_hEnemy != NULL && m_pSchedule != slStukaBatAnimWait && this->onGround == 1 && (m_MonsterState == MONSTERSTATE_ALERT || m_MonsterState == MONSTERSTATE_COMBAT) && !seekingFoodOnGround ){
		//we can start flying...?

		EASY_CVAR_PRINTIF_PRE(stukaPrintout, easyForcePrintLine("ENEMY??? B %d", m_hEnemy != NULL, this->HasConditions(bits_COND_ENEMY_OCCLUDED), this->HasConditions(bits_COND_NEW_ENEMY), this->HasConditions(bits_COND_SEE_ENEMY)));

		//MODDD - TEST: or any "combat face" could do this too, try that if this isn't good!
		//SCHED_COMBAT_FACE ,  or TASK_FACE_COMBAT    something?  I dunno.
		BOOL passed = FALSE;
		if(!this->HasConditions(bits_COND_ENEMY_OCCLUDED) || this->HasConditions(bits_COND_NEW_ENEMY), this->HasConditions(bits_COND_SEE_ENEMY) ){
			passed = TRUE;
			//okay.
		}else{

		}
		
		//Test this, make sure we don't get up in the middle of looking for food.
		setAnimation("Take_off_from_land", TRUE, FALSE, 2);
		//41.6f/12.0f ???
		blockSetActivity = gpGlobals->time + (29.0f/12.0f);
		queueToggleGround = TRUE;

		RouteClear();
		//return GetScheduleOfType(SCHED_STUKABAT_ANIMWAIT);
		this->ChangeSchedule(slStukaBatAnimWait);
		
	}

	if(snappedToCeiling){

		if( m_afMemory & bits_MEMORY_PROVOKED){
			//Get off!
			tryDetachFromCeiling();
		}else{
			//don't care.
		}

		return 0;  //cannot rotate on the ceiling, do NOT go up the method heirarchy (turn).
	}else{
		//we can go.
	}

	return CSquadMonster::ChangeYaw(yawSpeed);
}



void CStukaBat::ForceMakeIdealYaw( Vector vecTarget )
{
	
	CSquadMonster::MakeIdealYaw(vecTarget);

}
float CStukaBat::ForceChangeYaw ( int yawSpeed )
{


	return CSquadMonster::ChangeYaw(yawSpeed);
}





//NOTE: should hgrunts / hassaults also have methods similar to "callforelp" (alert nearby AI of
//the presence of a thread) so that they alert both hgrunts and hassaults nearby, not just those
//of their exact own class (if it works by using "netname" like this)?
void CStukaBat::CallForHelp( char *szClassname, float flDist, EHANDLE hEnemy, Vector &vecLocation )
{
	
	if ( (!snappedToCeiling  ) ){
		return;
	}
	
	CBaseEntity *pEntity = NULL;

	//Netname?  Map only?  yay that yay.

	//while ((pEntity = UTIL_FindEntityByString( pEntity, "netname", STRING( pev->netname ))) != NULL)
	
	while ((pEntity = UTIL_FindEntityByClassname( pEntity, STRING( pev->classname ))) != NULL)
	{
		float d = (pev->origin - pEntity->pev->origin).Length();
		if (d < flDist)
		{
			
			CBaseMonster *pMonster = pEntity->MyMonsterPointer( );
			if (pMonster)
			{
				pMonster->m_afMemory |= bits_MEMORY_PROVOKED;
				pMonster->PushEnemy( hEnemy, vecLocation );
				if(FClassnameIs(pMonster->pev, "monster_stukabat")){
					//???
					//CStukaBat* tempStuka = (CStukaBat*)(pMonster);
					CStukaBat* tempStuka = static_cast<CStukaBat*>(pMonster);
					if(tempStuka->snappedToCeiling == TRUE){
						tempStuka->wakeUp = TRUE;

						//Wait.  Why not just unhinge right now?
						//MODDD - TESTING: is this okay???
						tempStuka->m_afMemory &= bits_MEMORY_PROVOKED;
						tempStuka->tryDetachFromCeiling();

					}

				}

			}
		}
	}
}





void CStukaBat::AlertSound( void )
{
	//EASY_CVAR_PRINTIF_PRE(stukaPrintout, easyForcePrintLine("YOU SNEEKY errr!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! %d", m_hEnemy==NULL) );
	if ( m_hEnemy != NULL )
	{
		//SENTENCEG_PlayRndSz(ENT(pev), "SLV_ALERT", 0.85, ATTN_NORM, 0, m_voicePitch);
		CallForHelp( "monster_stukabat", 512, m_hEnemy, m_vecEnemyLKP );
	}

	
	if(EASY_CVAR_GET_DEBUGONLY(shutupstuka) != 1){
	EMIT_SOUND_ARRAY_STUKA_FILTERED( CHAN_VOICE, pAlertSounds );
	}
}

void CStukaBat::IdleSound( void )
{
	if(EASY_CVAR_GET_DEBUGONLY(shutupstuka) != 1){
	EMIT_SOUND_ARRAY_STUKA_FILTERED( CHAN_VOICE, pIdleSounds ); 
	}
}

void CStukaBat::AttackSound( void )
{
	if(EASY_CVAR_GET_DEBUGONLY(shutupstuka) != 1){
	EMIT_SOUND_ARRAY_STUKA_FILTERED( CHAN_VOICE, pAttackSounds ); 
	}
}

void CStukaBat::DeathSound( void )
{
	if(EASY_CVAR_GET_DEBUGONLY(shutupstuka) != 1){
	EMIT_SOUND_ARRAY_STUKA_FILTERED( CHAN_VOICE, pDeathSounds ); 
	}
}





void CStukaBat::KeyValue( KeyValueData *pkvd )
{
	/*
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
	*/

	//var.
	//0 = automatic (default).
	//1 = on ground.
	//2 = midair
	//3 = on the ceiling.
	
	/*
	//NOTICE: we will use the flag "onGround" instead.
	if (FStrEq(pkvd->szKeyName, "spawnLoc")){
		m_iSpawnLoc = atoi( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}else{
		CBaseMonster::KeyValue( pkvd );
	}
	*/

	CBaseMonster::KeyValue( pkvd );
}



//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CStukaBat::HandleAnimEvent( MonsterEvent_t *pEvent )
{
	//easyPrintLineGroup2("STUKABAT ANIM: %d", pEvent);

	switch( pEvent->event )
	{
		/*
		case STUKABAT_SOMEEVENT
		{

		}
		*/
		//break;
		default:
			CBaseMonster::HandleAnimEvent( pEvent );
		break;
	}//END OF switch(...)
	
}


CStukaBat::CStukaBat(void) : stukaPrint(StukaPrintQueueManager("STUKA")){
	
	lastEnemey2DDistance = 0;

	recentActivity = ACT_RESET;

	attackIndex = -1;
	//-2 = still in delay b/w attacks (can still follow).
	//-1 = not attacking, ready when player is close enough.
	//0 = did "drill_cycle" or whatever, approaching player fast.
	//1 = delivering attack (claw or suicide).
	//2 = backing off (if did claw)

	attackEffectDelay = -1;
	attackAgainDelay = 0;
	maxDiveTime = -1;

	//my ID is the globalID.  Then, bump it (next bat created gets the next ID in line)
	//stukaID = globalStukaID;
	////!!! NO, using "monsterID" now.
	//globalStukaID++;

	timeToIdle = -1;

	combatCloseEnough = FALSE;

	//is that okay?
	//m_iSpawnLoc = 0;

	tempCheckTraceLineBlock = FALSE;

	//WARNING: verify loading a save does NOT cause this value to take precedence (IE, appearing w/o spawn on load making it force "false", even if the loaded game clearly wants "true").
	onGround = FALSE;  //assume false?   Determine more reasonably at spawn.


	blockSetActivity = -1;

	//Activity lastSetActivitySetting;
	//BOOL lastSetActivityforceReset;
	queueToggleGround = FALSE;
	snappedToCeiling = FALSE;
	queueToggleSnappedToCeiling = FALSE;

	queueActionIndex = -1;

	queueAbortAttack = FALSE;


	//set to creation time.  That's okay, right?
	//lastVelocityChange = gpGlobals->time;
	//be safe...
	lastVelocityChange = -1;


	moveFlyNoInterrupt = -1;
	dontResetActivity = FALSE;
	//seekingFoodOnGround = FALSE;

	chargeIndex = -1;

	wakeUp = FALSE;

	//turn off the bottom-auto-ground-push-sensors.
	turnThatOff = FALSE;


	landBrake = FALSE;

}




void CStukaBat::heardBulletHit(entvars_t* pevShooter){

	//TRIGGERED
	wakeUp = TRUE;
	m_afMemory |= bits_MEMORY_PROVOKED;

	//calling the usual routine of basemonster is okay?
	CSquadMonster::heardBulletHit(pevShooter);
}


BOOL CStukaBat::isProvokable(void){
	//ordinarily, yes.
	return TRUE;
}
BOOL CStukaBat::isProvoked(void){
	
	return (m_afMemory & bits_MEMORY_PROVOKED);
}


int CStukaBat::IRelationship( CBaseEntity *pTarget )
{

	BOOL isPlayerAlly = FALSE;
	BOOL potentialNeutral = FALSE;
	//if(FClassnameIs(pTarget->pev, "monster_scientist") || FClassnameIs(pTarget->pev, "monster_sitting_scientist") || FClassnameIs(pTarget->pev, "monster_barney") ){
	if(pTarget->isTalkMonster()){
		//all talk monsters are (usually) the player's friend, and should not alert Stukabats.
		isPlayerAlly = TRUE;
	}

	if ( (pTarget->IsPlayer()) || isPlayerAlly ) {
		potentialNeutral = TRUE;
	}
	
	//chance to disregard as not a threat, or fail to notice (like hanging, "sleeping").
	if(potentialNeutral && !(m_afMemory & bits_MEMORY_PROVOKED ) ){


		if(!snappedToCeiling || wakeUp == TRUE){
			switch( (int) EASY_CVAR_GET_DEBUGONLY(STUDetection) ){
			case 0:
				//good enough, something serious needs to provoke me.
				return R_NO;
			break;
			case 1:
				//provoke if in line of sight.
			
				if(FInViewCone( pTarget )){
					m_afMemory |= bits_MEMORY_PROVOKED;
				}else{
					return R_NO;
				}

			break;
			case 2:
				m_afMemory |= bits_MEMORY_PROVOKED;
				//pass through.  Provoke as usual.
			break;
			default:
				//???
			break;
			}

		}else{
			//if snapped, always leniant.
			return R_NO;
		}


	}//END OF if(potentialNeutral && !(m_afMemory & bits_MEMORY_PROVOKED ) )


	/*
	if ( (pTarget->IsPlayer()) || isPlayerAlly)
		if ( (TRUE ) && ! (m_afMemory & bits_MEMORY_PROVOKED ))
			return R_NO;
	*/

	BOOL defaultReturned = CBaseMonster::IRelationship( pTarget );
	return defaultReturned;
}


//=========================================================
// Spawn
//=========================================================
void CStukaBat::Spawn()
{
	Precache( );

	SET_MODEL(ENT(pev), "models/stukabat.mdl");

	//is this box okay?
	//CHECK EM

	//UTIL_printLineVector( "PROJECTION", UTIL_projectionComponent(Vector(13, -34, 21), Vector(-28, 10, -18) ) );

	UTIL_SetSize( pev, Vector( -8, -8, 0 ), Vector( 8, 8, 28 ));
	//UTIL_SetSize( pev, Vector( -34, -34, 0 ), Vector( 34, 34, 48 ));

	pev->classname = MAKE_STRING("monster_stukabat");


	//MODDD NOTE - would other types work better? Alternate between _SLIDEBOX for ground movement, _BBOX for air?
	//             ichy uses SOLID_BBOX.
	pev->solid			= SOLID_SLIDEBOX;  //SOLID_TRIGGER?  Difference?
	pev->movetype		= MOVETYPE_FLY;

	pev->movetype		= MOVETYPE_BOUNCEMISSILE;
	
	pev->flags			|= FL_FLY;
	m_bloodColor		= BLOOD_COLOR_GREEN;

	//easyPrintLineGroup2("STUAK HEALTH: %.2f", gSkillData.stukaBatHealth);
	//CHANGENOTICE: stuka bat probably has a different amount of health.  Make vars to scale with difficulty too?
	pev->health			= gSkillData.stukaBatHealth;
	EASY_CVAR_PRINTIF_PRE(stukaPrintout, easyForcePrintLine("HEALTH BE %.2f", gSkillData.stukaBatHealth) );

	//CHANGENOTICE: check the model and change if needed.
	pev->view_ofs		= Vector( 0, 0, 15 );// position of the eyes relative to monster's origin.

	//makes the eye offset (view_ofs) more obvious at spawn.
	//UTIL_drawLine(pev->origin.x - 10, pev->origin.y, pev->origin.z + 15, pev->origin.x + 10, pev->origin.y, pev->origin.z + 15);

	//why is this not on by default in this case?   NO, they are "ranged" attacks instead for whatever reason.
	//m_afCapability |= bits_CAP_MELEE_ATTACK1;

	m_afCapability		= bits_CAP_SQUAD;

	m_voicePitch = randomValueInt(101, 107);
	
	//TEST
	//pev->spawnflags &= ~ SF_SQUADMONSTER_LEADER;

	m_flFieldOfView		= VIEW_FIELD_FULL;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState		= MONSTERSTATE_NONE;

	MonsterInit();

	this->SetTouch(&CStukaBat::customTouch);
	
	checkStartSnap();
}

void CStukaBat::checkStartSnap(){
	
	TraceResult trDown;
	TraceResult trUp;
	Vector vecStart = pev->origin;
	Vector vecStartLower = pev->origin + Vector(0, 0, -5);  //start a little below in case I got spawned against the ceiling.
	Vector vecEnd;
	
	float distFromUp = -1;
	float distFromDown = -1;

	BOOL certainOfLoc = FALSE;
	
	onGround = FALSE;
	//can be proven wrong (make "TRUE").

	if(spawnedDynamically && !flagForced){

		//default; find the best way.
		m_iSpawnLoc = 0;
	}else{
		if(!(pev->spawnflags & SF_MONSTER_STUKA_ONGROUND) ){
			m_iSpawnLoc = 3;
		}else{
			m_iSpawnLoc = 1;
		}
	}

	if(m_iSpawnLoc == 0){
		//Auto-choose if the player spawned me (cheats).  Need to determine whether to snap to the ground, ceiling, or nothing (if neither is close)
		//this time, "ignore_monsters" as opposed to "dont_ignore_monsters".
		vecEnd = vecStartLower + Vector(0, 0, 33 + 5 + 32);
		UTIL_TraceLine(vecStartLower, vecEnd, ignore_monsters, ENT(pev), &trUp);

		vecEnd = vecStart + Vector(0, 0, -32);
		UTIL_TraceLine(vecStart, vecEnd, ignore_monsters, ENT(pev), &trDown);

		distFromUp = -1;
		distFromDown = -1;
	
		// hit something?
		if(trUp.flFraction < 1.0){
			distFromUp = (vecStartLower - trUp.vecEndPos).Length();
			if(distFromUp < 28+5){
				//Force us a little lower...
				//NO don't do it here, we do it later just fine.
				//The "vecStartLower" above is all that was needed.
				//UTIL_SetOrigin(pev, pev->origin + Vector(0, 0, ));
				//float shiftDown = (28+5) - distFromUp;
				//pev->origin = pev->origin + Vector(0, 0, -shiftDown);
			}
		}
		if(trDown.flFraction < 1.0){
			distFromDown = (vecStart - trDown.vecEndPos).Length();
		}
		//don't re-do the distance-check logic for any choice down the road.
		certainOfLoc = TRUE;
		easyPrintLineGroup2("STUKA SPAWN VERTICAL DIST: from up: %.2f | from down: %.2f", distFromUp, distFromDown);

		if(distFromUp == -1 && distFromDown == -1){
			//cannot snap  to either surface in a short distance.  Giving up; hover mid-air at spawn.
			m_iSpawnLoc = 2;
		}else if(distFromUp == -1){
			m_iSpawnLoc = 1;
		}else if(distFromDown == -1){
			m_iSpawnLoc = 3;
		}else if(distFromDown <= distFromUp){
			m_iSpawnLoc = 1;
		}else{
			m_iSpawnLoc = 3;
		}
	}//END OF if m_iSpawnLoc == 0)

	if(m_iSpawnLoc == 1){
		//ground, snap to bottom.
		if(!certainOfLoc){
			//check for the ground.
			vecEnd = vecStart + Vector(0, 0, -160);
			UTIL_TraceLine(vecStart, vecEnd, ignore_monsters, ENT(pev), &trDown);

			if(trDown.flFraction < 1.0){
				certainOfLoc = TRUE;
			}
		}
		if(certainOfLoc){
			canSetAnim = FALSE;
			SetActivity(ACT_CROUCHIDLE, TRUE );   //idle for ground.
			easyPrintLineGroup2("BUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU");

			onGround = TRUE;
			pev->movetype = MOVETYPE_STEP;
			pev->flags &= ~FL_FLY;

			pev->origin = trDown.vecEndPos;
		}else{
			//give up.
			m_iSpawnLoc = 2;
		}
	}else if(m_iSpawnLoc == 3){
		//ceiling, snap to top

		if(!certainOfLoc){
			//check for the ceiling.
			vecEnd = vecStartLower + Vector(0, 0, 160);
			UTIL_TraceLine(vecStartLower, vecEnd, ignore_monsters, ENT(pev), &trUp);

			if(trUp.flFraction < 1.0){
				certainOfLoc = TRUE;
			}
		}
		if(certainOfLoc){
			snappedToCeiling = TRUE;  //allow the stuka to start from the ceiling.
			SetActivity(ACT_IDLE);  //hanging from the ceiling.
			pev->origin = trUp.vecEndPos + Vector(0, 0, -33);
		}else{
			//give up.
			m_iSpawnLoc = 2;
		}
	}
	else if(m_iSpawnLoc == 2){
		//mid-air, no snapping.  Essentialy a "give-up" value on either of the above failing too.

		SetActivity(ACT_HOVER);  //correct.
		//setAnimation("Hover");

		if(!(pev->spawnflags & SF_MONSTER_STUKA_ONGROUND) ){
			easyPrintLineGroup2("MAP ERROR: Stuka Bat spawned wrongly. Could not find ceiling to snap to (OnGround flag is set to \"false\").");
		}else{
			easyPrintLineGroup2("MAP ERROR: Stuka Bat spawned wrongly. Could not find ground to snap to (OnGround flag is set to \"true\").");
		}
	}
	
	EASY_CVAR_PRINTIF_PRE(stukaPrintout, easyForcePrintLine("STUKA SPAWN END %d: %d", monsterID, m_iSpawnLoc) );


	//okay?
	lastVelocityChange = gpGlobals->time;
}


extern int global_useSentenceSave;
//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CStukaBat::Precache()
{	
	//CHANGENOTICE: also add to "precacheAll" in cbase.cpp.
	PRECACHE_MODEL("models/stukabat.mdl");

	iPoisonSprite = PRECACHE_MODEL( "sprites/poison.spr" );



	global_useSentenceSave = TRUE;
	PRECACHE_SOUND_ARRAY( pAttackHitSounds );
	PRECACHE_SOUND_ARRAY( pAttackMissSounds );
	PRECACHE_SOUND_ARRAY( pAttackSounds );
	PRECACHE_SOUND_ARRAY( pIdleSounds );
	PRECACHE_SOUND_ARRAY( pAlertSounds );
	PRECACHE_SOUND_ARRAY( pPainSounds );
	PRECACHE_SOUND_ARRAY( pDeathSounds );
	//add anything else the stukabat needs (if applicable), especially sounds to benefit from the sound-sentence-save fix.
	
	global_useSentenceSave = FALSE;
	
}




//NOTE: DEFINE_CUSTOM_SCHEDULES
DEFINE_CUSTOM_SCHEDULES( CStukaBat)
{

	slStukaBatChaseEnemy,
	slStukaBatStrafe,
	slStukaBatTakeCover,
	slStukaBatFail,
	slStukaBatRangeAttack2,

	slStukaBatFindEat,
	slStukaBatAttemptLand,
	slStukaBatCrawlToFood,
	slStukaBatEat,


	
	slStukaIdleHover,
	slStukaIdleGround,

	slStukaBatAnimWait,

	slStukaPathfindStumped,

};

IMPLEMENT_CUSTOM_SCHEDULES( CStukaBat, CSquadMonster );





void CStukaBat::getPathToEnemyCustom(){


	if(FRouteClear()){
		FRefreshRouteChaseEnemySmart();
	}

	return;
	///////////////////////////////////////////////////////////////////////////////////////////


	/*
	if(queueToggleGround){
		//nothin
		TaskComplete();
		return;
	}
	*/
	//CBaseEntity *pEnemy = m_hEnemy;

	BOOL enemyPresent = !(m_hEnemy==NULL||m_hEnemy.Get()==NULL|| CBaseMonster::Instance(m_hEnemy.Get())==NULL  || m_hEnemy->pev->deadflag != DEAD_NO);

	//EASY_CVAR_PRINTIF_PRE(stukaPrintout, easyForcePrintLine("ererererer %d", (m_hEnemy==NULL) ) );

	if ( !enemyPresent )
	{
		
		//???
		EASY_CVAR_PRINTIF_PRE(stukaPrintout, easyForcePrintLine("FAIL TO GET PATH 1") );
		TaskFail();

		return;
	}else{
		
	}
	

	//Vector vecDest = m_hEnemy->pev->origin;
	Vector vecDest = this->m_vecEnemyLKP;

	if ( BuildRoute ( vecDest, bits_MF_TO_ENEMY, m_hEnemy )  )
	{
		int x = 66;
		//TaskComplete();
		//???!!!
	}
	//else if (BuildNearestRouteSimple( vecDest, m_hEnemy->pev->view_ofs, 0, (vecDest - pev->origin).Length() ), DEFAULT_randomNodeSearchStart  )
	else if (BuildNearestRoute( vecDest, m_hEnemy->pev->view_ofs, 0, (m_hEnemy->pev->origin - pev->origin).Length() + 1024, DEFAULT_randomNodeSearchStart, bits_MF_TO_ENEMY, m_hEnemy  )  )
	{
		int x = 66;
		//TaskComplete();
		//???!!!
	}
	else
	{
		// no way to get there =(
		//ALERT ( at_aiconsole, "GetPathToEnemy failed!!\n" );
		EASY_CVAR_PRINTIF_PRE(stukaPrintout, easyForcePrintLine("FAIL TO GET PATH 2") );
		//TaskFail();
		//no, say stumped.
		TaskComplete();
	}

}

//=========================================================
// StartTask
//=========================================================
void CStukaBat::StartTask ( Task_t *pTask )
{
	EASY_CVAR_PRINTIF_PRE(stukaPrintout, easyForcePrintLine("STUKA STARTTASK: %s %d", getScheduleName(), pTask->iTask) );
	//easyForcePrintLine("STUKA STARTTASK: %s %d", getScheduleName(), pTask->iTask);


	//EHANDLE* test = getEnemy();
	//??
	switch ( pTask->iTask )
	{
	//MODDD - copied here, terminating early would be more convenient.
	case TASK_STUKA_WAIT_FOR_ANIM:
		//this->m_fSequenceFinished || 
		if (blockSetActivity == -1) {
			//done!  Try something else.
			TaskComplete();
		}
		else {
			//nothing to do here.
			return;
		}
	break;

		
	case TASK_STUKABAT_LAND:{
		

		turnThatOff = TRUE;
				
		
		PRINTQUEUE_STUKA_SEND(stukaPrint.eatRelated, "ANI: Land_ground!");
				
		turnThatOff = TRUE;
				

		setAnimation("Land_ground", TRUE, FALSE, 2);

		//blockSetActivity = gpGlobals->time + (48.0f/12.0f);
		blockSetActivity = gpGlobals->time + (26.0f/12.0f);
		queueToggleGround = TRUE;
				
		/*
		//We're landing, just force it to ACT_CROUCHIDLE
		if ( m_IdealActivity == m_movementActivity )
		{
			m_IdealActivity = GetStoppedActivity();
		}
		*/
		m_IdealActivity = ACT_CROUCHIDLE;


		RouteClear();
		ChangeSchedule(slStukaBatAnimWait);

		/*
		setAnimation("Take_off_from_land", TRUE, FALSE, 2);
		//41.6f/12.0f ???
		blockSetActivity = gpGlobals->time + (29.0f/12.0f);
		queueToggleGround = TRUE;
		return GetScheduleOfType(SCHED_STUKABAT_ANIMWAIT);
		*/
	break;}


	case TASK_GET_PATH_TO_BESTSCENT_FOOT:{
		CSound *pScent;
		Vector tempGoal;
		Vector scentSpot;
		Vector toSpot;
		Vector dirToSpot;
		float distToSpot;

	
		

		pScent = PBestScent();
		//easyForcePrintLine("SO IS MY SCENT NULL? %d", (pScent==NULL));
		if(pScent != NULL){
			//tempGoal = Vector(pev->origin.x, pev->origin.y, pScent->m_vecOrigin.z + 6);
			
			scentSpot = pScent->m_vecOrigin;
			//if we use this for by foot for some reason?
			scentLocationMem = scentSpot;



			seekingFoodOnGround = TRUE;
		}else{
			//no scent? we're done.
			seekingFoodOnGround = FALSE;
			TaskFail();
			return;
		}



		//should there be a better way to turn off "seekingFoodOnGround"?

		//UTIL_drawLineFrameBoxAround(pev->origin, 3, 24, 255, 255, 0);
		//UTIL_drawLineFrameBoxAround(tempGoal, 3, 24, 255, 125, 0);

		//easyForcePrintLine("TELL ME %.2f %.2f %.2f", pScent->m_vecOrigin.x, pScent->m_vecOrigin.y, pScent->m_vecOrigin.z);
		
		toSpot = (scentSpot - pev->origin);
		dirToSpot = toSpot.Normalize();
		distToSpot = toSpot.Length();

		tempGoal = scentSpot + dirToSpot * -20;

		
		if(distToSpot < 30){
			//we've gotten this far, we're doing this anyways.
			m_failSchedule = SCHED_STUKABAT_EAT;  //just start eating, it's okay to.
		}



		MakeIdealYaw( tempGoal );
		//not flying, perhaps "Move" or "MoveExecute" should handle this...?
		ChangeYaw( pev->yaw_speed );

		//RouteClear();
		this->m_movementGoal = MOVEGOAL_LOCATION;
		m_vecMoveGoal = tempGoal;

		
		
		if ( BuildRoute ( tempGoal, bits_MF_TO_LOCATION, NULL ) )
		{	
			//Is that okay?
			this->SetActivity(ACT_WALK);

			TaskComplete();
		}
		else if (BuildNearestRoute( tempGoal, pev->view_ofs, 0, distToSpot, DEFAULT_randomNodeSearchStart ))
		{	
			//Is that okay?
			this->SetActivity(ACT_WALK);

			TaskComplete();
		}
		else
		{
			m_failSchedule = SCHED_STUKABAT_EAT;  //just start eating, it's okay to.

			//means, already facing the ideal Yaw.  So that "is facing?"  just goes with this.
			//uh no, just go ahead and face whatever we wanted to, it is ok.
			//pev->ideal_yaw = this->pev->angles.y;

			eating = TRUE;
			eatingAnticipatedEnd = gpGlobals->time + 7;

			RouteClear();
			TaskFail();
		}


		
	break;}





	case TASK_GET_PATH_TO_BESTSCENT:{
	
		CSound *pScent;
	
		//getPathToEnemyCustom();

		pScent = PBestScent();
			
		if(pScent == NULL){
			TaskFail();
			//???
			seekingFoodOnGround = FALSE;
			return;
		}

		float scent_ZOffset = 5;
			
		Vector scent_Loc = pScent->m_vecOrigin + Vector(0, 0, scent_ZOffset);


		MakeIdealYaw( scent_Loc );
		ChangeYaw( pev->yaw_speed );



			
		this->m_movementGoal = MOVEGOAL_LOCATION;
		m_vecMoveGoal = scent_Loc;
		if ( BuildRoute ( scent_Loc, bits_MF_TO_LOCATION, NULL ) )
		{

			TaskComplete();
		}
		//No need for viewoffset, the 2nd argument. scent_loc already has this.
		else if (BuildNearestRoute( scent_Loc, Vector(0,0,0), 0, (scent_Loc - pev->origin).Length(), DEFAULT_randomNodeSearchStart ))
		{
			TaskComplete();
		}
		else
		{
			// no way to get there =(
			PRINTQUEUE_STUKA_SEND(stukaPrint.eatRelated, "PathToBestScent failed!!" );
			TaskFail();
		}

	break;}

	case TASK_GET_PATH_TO_LANDING:{
		CSound *pScent;
		BOOL timetostop;
		Vector tempGoal;
		BOOL tryPath = TRUE;
		Vector scentSpot;

		//m_IdealActivity = ACT_HOVER;  //good idea?

		//going to land soon, stall movement on the X-Y plane.
		//NOTICE - is this still effective?
		
		Vector toSpot;
		float distanceToSpot;
		float distanceToSpot2D;
		int triesLeft = 4;

		landBrake = TRUE;


		/*
		if(onGround || queueToggleGround){
			///????
			TaskComplete();
			return;
		}
		*/


		//timetostop = FALSE;
		/*
		if(blockSetActivity > -1 && queueToggleGround == TRUE){
			PRINTQUEUE_STUKA_SEND(stukaPrint.eatRelated, "timeTo Stop" );
			//TaskComplete();
			//tryPath = FALSE;
				
			timetostop = TRUE;
			return;
		}
		*/

		//if(!timetostop){
			pScent = PBestScent();
			if(pScent != NULL){
				//Pick a random spot away from the scent.
				//tempGoal = Vector(pev->origin.x, pev->origin.y, pScent->m_vecOrigin.z + 6);

				scentSpot = pScent->m_vecOrigin;
				scentLocationMem = scentSpot;


			}else{
				//no scent? we're done.
				TaskFail();
				return;
			}
		//}
			
		toSpot = (scentSpot - pev->origin);
		distanceToSpot = toSpot.Length();
		distanceToSpot2D = toSpot.Length2D();
			
		//If we're close enough to our goal we don't care, just end early.
		if( distanceToSpot <= 36 || distanceToSpot2D <= 28){ //&& blockSetActivity == -1){

			TaskComplete();
			this->MovementComplete(); //let the next task be skipped too.
			return;
		}


		while(triesLeft > 0){
			float randomAng = RANDOM_FLOAT(0, 360);
			Vector randomDir = UTIL_YawToVec(randomAng);

			tempGoal = scentSpot + Vector(0, 0, 48) + randomDir * RANDOM_FLOAT(30, 50);


			PRINTQUEUE_STUKA_SEND(stukaPrint.eatRelated, "vecDiffLength:%.2f", (tempGoal - pev->origin).Length() );
			PRINTQUEUE_STUKA_SEND(stukaPrint.eatRelated, "blockSetAct:%.2f", blockSetActivity );
			//PRINTQUEUE_STUKA_SEND(stukaPrint.eatRelated, "timetostop:%d", timetostop );

			//UTIL_drawLineFrameBoxAround(pev->origin, 3, 24, 255, 255, 0);
			//UTIL_drawLineFrameBoxAround(tempGoal, 3, 24, 255, 125, 0);
			
			//if(timetostop){
			//	return;
			//}

			BOOL queueFailure = FALSE;




			this->m_movementGoal = MOVEGOAL_LOCATION;
			//is it redundatn to say this now?
			m_vecMoveGoal = tempGoal;



			if ( BuildRoute ( tempGoal, bits_MF_TO_LOCATION, NULL ) )
			{
				PRINTQUEUE_STUKA_SEND(stukaPrint.eatRelated, "LANDROUTE_1!"  );

				
				TaskComplete();
				return;
			}
			else if (BuildNearestRoute( tempGoal, pev->view_ofs, 0, (tempGoal - pev->origin).Length(), DEFAULT_randomNodeSearchStart ))
			{
				PRINTQUEUE_STUKA_SEND(stukaPrint.eatRelated, "LANDROUTE_2!"  );

				TaskComplete();
				return;
			}
			else
			{
				// no way to get there =(
				PRINTQUEUE_STUKA_SEND(stukaPrint.eatRelated, "LANDROUTE_hay"  );

				//queueFailure = TRUE;  //only if we're not already close enough... doing the check below.
				
			}

			triesLeft--;

		}//END OF while(triesLeft)


		//didn't get it in enough tries? Give up this time.
		this->RouteClear();
		TaskFail();



		//that blockSetActivity == -1 was good, right?



						/*
			if( (m_vecMoveGoal - pev->origin).Length() <= 16){ //&& blockSetActivity == -1){
				

			}else{
				if(queueFailure){
					//Not just too close for pathing but counts for success? Then fail.
					TaskFail();
					//??? ok.
					return;
				}
			}
			*/

	break;}








	case TASK_CHECK_STUMPED:
	{

		if(!HasConditions(bits_COND_SEE_ENEMY)){
			abortAttack();  //can't see them, we're not attacking.
			recentActivity = ACT_RESET;
			//just in case we drop this from abortAttack.
		}

		CSquadMonster::StartTask(pTask);
		break;
	}
	case TASK_SET_ACTIVITY:{
		blockSetActivity = -1;

		if( this->m_pSchedule == slStukaBatEat){
			//stop doing this now!
			eating = FALSE;
		}

		CSquadMonster::StartTask(pTask);
	break;}
	case TASK_SET_ACTIVITY_FORCE:{
		blockSetActivity = -1;
		CSquadMonster::StartTask(pTask);
	break;}
	case TASK_SMALL_FLINCH:
	{
		m_IdealActivity = GetSmallFlinchActivity();
		break;
	}
	case TASK_WAIT:
		//this is all schedule.cpp does here.  Why did I not just do this?

		//is this okay?
		//Get what idle activity is fitting for me in this state.

		if(m_pSchedule != slStukaBatEat){
			this->m_IdealActivity = getIdleActivity();
		}else{
			//proceed with eating I guess.
			this->m_IdealActivity = ACT_EAT;
		}

		EASY_CVAR_PRINTIF_PRE(stukaPrintout, easyForcePrintLine("TASK_WAIT %s GT:%.2f WF:%.2f", m_pSchedule->pName, gpGlobals->time, m_flWaitFinished));
		m_flWaitFinished = gpGlobals->time + pTask->flData;	
		break;
	case TASK_EAT:
		
		ChangeYaw(16);

		CSquadMonster::StartTask ( pTask );
		break;
		//see schedule.cpp for the originals.
	//OKAY this is horrendous. luckily they are never called. they should at least call the parent,  CSquadMonster::StartTask(pTask)
	case TASK_FIND_NEAR_NODE_COVER_FROM_ENEMY:
		{
			return;
		}
		break;
	case TASK_FIND_FAR_NODE_COVER_FROM_ENEMY:
		{
			return;
		}
		break;
	case TASK_FIND_NODE_COVER_FROM_ENEMY:
		{
			return;
		}
		break;
	case TASK_FIND_COVER_FROM_ENEMY:
		{
			return;
		}
		break;
	case TASK_FIND_COVER_FROM_ORIGIN:
		{
			return;
		}
		break;
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

			if ( pBestSound && FindCover( pBestSound->m_vecOrigin, g_vecZero, pBestSound->m_iVolume, CoverRadius() ) )
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
		/*
	case TASK_GET_PATH_TO_ENEMY_LKP:
		{
			if (BuildNearestRoute( m_vecEnemyLKP, pev->view_ofs, pTask->flData, (m_vecEnemyLKP - pev->origin).Length() + 1024, DEFAULT_randomNodeSearchStart ))
			{
				TaskComplete();
			}
			else
			{
				// no way to get there =(
				ALERT ( at_aiconsole, "GetPathToEnemyLKP failed!!\n" );
				TaskFail();
			}
			break;
		}
		*/

		/*
	case TASK_GET_PATH_TO_ENEMY:
		{
			CBaseEntity *pEnemy = m_hEnemy;

			if ( pEnemy == NULL )
			{
				TaskFail();
				return;
			}

			if (BuildNearestRoute( pEnemy->pev->origin, pEnemy->pev->view_ofs, pTask->flData, (pEnemy->pev->origin - pev->origin).Length() + 1024, DEFAULT_randomNodeSearchStart ))
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
	//pasted from monsters.cpp.
	case TASK_GET_PATH_TO_ENEMY_LKP:
	{
		CBaseEntity* enemyTest;

		if(m_hEnemy != NULL){
			enemyTest = m_hEnemy.GetEntity();
		}else{
			enemyTest = NULL;
		}
		

		//changed too.
		//if ( BuildRouteSimple ( m_vecEnemyLKP, bits_MF_TO_LOCATION, NULL ) )
		if ( BuildRoute ( m_vecEnemyLKP, bits_MF_TO_ENEMY, enemyTest ) )
		{
			TaskComplete();
		}
			
		else if (BuildNearestRoute( m_vecEnemyLKP, pev->view_ofs, 0, (m_vecEnemyLKP - pev->origin).Length() + 1024, DEFAULT_randomNodeSearchStart, bits_MF_TO_ENEMY, enemyTest ))
		{
			TaskComplete();
		}
			
		else
		{
			// no way to get there =(
			//EASY_CVAR_PRINTIF_PRE(stukaPrintout, easyForcePrintLine("PATH FAIL 0") );
			//addToPrintQueue_path("PATH FAIL 0");



			ALERT ( at_aiconsole, "GetPathToEnemyLKP failed!!\n" );
			TaskFail();
		}
		break;
	}
	case TASK_GET_PATH_TO_ENEMY:
		{
			EASY_CVAR_PRINTIF_PRE(stukaPrintout, easyForcePrintLine("yayFACE 4") );
			getPathToEnemyCustom();
			break;
		}
	/*
	case TASK_STOP_MOVING:


		//WARNING: IS THIS SAFE?
		pev->velocity = Vector(0,0,0);
		break;
	*/
	case TASK_RANGE_ATTACK1:
		//Ranges were blanked. Is that wise? Not even parent calls like "default" below does?
		//CSquadMonster::StartTask ( pTask );
		break;
	case TASK_RANGE_ATTACK2:
		//CSquadMonster::StartTask ( pTask );
		break;

		
	default:
		CSquadMonster::StartTask ( pTask );
		break;
	}
}

//CHANGENOTICE
//there is an "Intersect" method defined in controller.cpp.  It should probably be put somewhere more global (util.cpp) so that it doesn't need to be re-defined for here (usually causes compile problems).
//DONE!


//note: probably exactly the same as controller, but hard to say if that is fine.
int CStukaBat::LookupFloat( )
{

	
	UTIL_MakeAimVectors( pev->angles );
	float x = DotProduct( gpGlobals->v_forward, m_velocity );
	float y = DotProduct( gpGlobals->v_right, m_velocity );
	float z = DotProduct( gpGlobals->v_up, m_velocity );

	if (fabs(x) > fabs(y) && fabs(x) > fabs(z))
	{
		/*
		if (x > 0)
			return LookupSequence( "forward");
		else
			return LookupSequence( "backward");
			*/
	}
	else if (fabs(y) > fabs(z))
	{
		/*
		//turn??
		if (y > 0)
			return LookupSequence( "right");
		else
			return LookupSequence( "left");
			*/
	}
	else
	{
		return LookupSequence( "Hover");
		/*
		if (z > 0)
			return LookupSequence( "up");
		else
			return LookupSequence( "down");
			*/
	}

	return LookupSequence("Flying_cycler");
	//no.

}



EHANDLE* CStukaBat::getEnemy(){

	EHANDLE* targetChoice;

	if ( m_movementGoal == MOVEGOAL_ENEMY ){
		targetChoice = &m_hEnemy;
	}else{
		targetChoice = &m_hTargetEnt;
	}

	float distance;
	//copied from "TASK_MOVE_TO_TARGET_RANGE" of schedule.cpp.
	//easyPrintLineGroup2("STUKA targetChoice: %d", targetChoice);
	if ( targetChoice == NULL || targetChoice->Get() == NULL )
		return NULL;

	return targetChoice;
		//TaskFail();

}


void CStukaBat::updateMoveAnim(){

	if(EASY_CVAR_GET_DEBUGONLY(stukaAdvancedCombat) == 1 && onGround == FALSE){
		//pass.
	}else{
		return;  //Do not affect.
	}
	if(attackIndex > -1){
		//don't modify!
		return;
	}

	EHANDLE* targetChoice;

	if ( m_movementGoal == MOVEGOAL_ENEMY ){
		targetChoice = &m_hEnemy;
	}else{
		targetChoice = &m_hTargetEnt;
	}

	float distance;
	//copied from "TASK_MOVE_TO_TARGET_RANGE" of schedule.cpp.
	//easyPrintLineGroup2("STUKA targetChoice: %d", targetChoice);
	if ( targetChoice == NULL || targetChoice->Get() == NULL ){
		//TaskFail();
	}else
	{
		//m_vecMoveGoal ???
		//distance = ( m_vecMoveGoal - pev->origin ).Length2D();
		distance = ( targetChoice->Get()->v.origin - pev->origin ).Length2D();

		//easyPrintLineGroup2("DISTANCE YO %.2f", distance);
		if ( distance < 280 && m_movementActivity != ACT_HOVER )
			m_movementActivity = ACT_HOVER;
		else if ( distance >= 400 && m_movementActivity != ACT_FLY )
			m_movementActivity = ACT_FLY;
	}
}


BOOL CStukaBat::getHasPathFindingModA(){
	return TRUE; //needs something more broad as not to get stuck.
}
BOOL CStukaBat::getHasPathFindingMod(){
	return TRUE; //needs something more broad as not to get stuck.
}


void CStukaBat::abortAttack(){
	//give up if so.
	easyPrintLineGroup2("ABORT ABORT ABORT");

	//abort attack!
	canSetAnim = TRUE;

	attackIndex = -2;
	attackAgainDelay = gpGlobals->time + getAttackDelay();


	attackEffectDelay = -1;
	maxDiveTime = -1;

	blockSetActivity = -1;
	//THIS IS IMPORTANT, PAY ATTENTION!

	timeToIdle = gpGlobals->time + 4;

	//combatCloseEnough = FALSE;

	//TaskComplete();
	//this->ChangeSchedule(slIdleStand);
	//TaskFail();

	queueAbortAttack = FALSE;

	chargeIndex = -1;

	recentActivity = ACT_RESET; //allow changing this activity, even to itself.

}


//=========================================================
// RunTask 
//=========================================================
void CStukaBat::RunTask ( Task_t *pTask )
{
	//easyForcePrintLine("WHAT? %d", pTask->iTask);
	//reset, no landBrake unless specified by this task running.
	landBrake = FALSE;
	

	CBaseEntity* temper;

	//CBaseEntity *pEnemy = m_hEnemy;
	float dist2d;
	float dist3d;

	//shoot logic here?

	EASY_CVAR_PRINTIF_PRE(stukaPrintout, easyForcePrintLine("Stuka #%d RUNTASK: %d", monsterID, pTask->iTask) );

	switch ( pTask->iTask )
	{



		
	case TASK_STUKABAT_LAND_PRE:{
		//Check. Is it ok to move vertically?

		
		PRINTQUEUE_STUKA_SEND(stukaPrint.eatRelated, "LANDROUTE_close"  );

		//pev->origin = tempGoal;

		Vector floorVect = UTIL_getFloor(pev->origin, 120, ignore_monsters, ENT(pev));

		//EASY_CVAR_PRINTIF_PRE(stukaPrintout, easyForcePrintLine("WHAT THE hay IS THIS? %.2f %.2f %.2f AND THAT? %.2f %.2f %.2f", floorVect.x, floorVect.y, floorVect.z, pev->origin.x,pev->origin.y,pev->origin.z));

		if( isErrorVector(floorVect)){
			PRINTQUEUE_STUKA_SEND(stukaPrint.eatRelated, "LANDROUTE_floorFAIL"  );
			//uh, what?
			TaskFail();
			return;
		}else{
			float verticalDist;
			PRINTQUEUE_STUKA_SEND(stukaPrint.eatRelated, "LANDROUTE_floorOK"  );
			//TaskComplete();

			m_velocity.x = 0;
			m_velocity.y = 0;
			pev->velocity.x = 0;
			pev->velocity.y = 0;

			verticalDist = fabs(floorVect.z - pev->origin.z);

			if(verticalDist < 12){
				//finish up.
				m_velocity.z = 0;
				pev->velocity.z = 0;
				UTIL_MoveToOrigin ( ENT(pev), floorVect, (pev->origin - floorVect).Length(), MOVE_STRAFE );
				TaskComplete();
				return;
			}else{
				m_IdealActivity = ACT_HOVER;
				//Then get closer.
				//Keep going straight down.
				m_velocity.z = -22;
				pev->velocity.z = -22;
				//UTIL_MoveToOrigin ( ENT(pev), floorVect, 6, MOVE_STRAFE );
			}
		}
	break;}



	case TASK_STUKA_WAIT_FOR_ANIM:
		//this->m_fSequenceFinished ||
		if(blockSetActivity == -1){
			//done!  Try something else.
			TaskComplete();
		}else{
			//nothing to do here.
			return;
		}
	break;
	
	case TASK_WAIT_FOR_MOVEMENT:
	case TASK_WAIT:
	case TASK_WAIT_FACE_ENEMY:
	case TASK_WAIT_PVS:{		


		// Going to a secent? Be a little more lenient on completing this WAIT_FOR_MOVEMENT task.
		if(m_pSchedule == slStukaBatFindEat || m_pSchedule == slStukaBatCrawlToFood){
			// if we're close enough to the goal just stop.
			if(m_Route[ m_iRouteIndex ].iType & bits_MF_IS_GOAL){
				float distToGoal = (m_Route[ m_iRouteIndex ].vecLocation - pev->origin).Length();
				if(distToGoal < 32){
					TaskComplete();
					return;
				}
			}


		}

		if(m_pSchedule == slStukaBatAttemptLand){
			float distToGoal2D = (scentLocationMem - pev->origin).Length2D();
			if( distToGoal2D <= 28 && blockSetActivity == -1){
				// good enough, just stop to land already.
				TaskComplete();
				return;
			}

			/*
			if(m_Route[ m_iRouteIndex ].iType & bits_MF_IS_GOAL){
				float distToGoal2D_other = (m_Route[ m_iRouteIndex ].vecLocation - pev->origin).Length2D();
				if(distToGoal2D_other <= 26){
					//TaskComplete();
					//wat?
					return;
				}
			}
			*/

		}







		EASY_CVAR_PRINTIF_PRE(stukaPrintout, easyForcePrintLine("BEWARE - the Stuka is waiting! %d %d F", eating, getEnemy()!=NULL));
		/*
		//no, wrap animation-changes into a schedule!
		//check if blockSetActivity is done?
		if(blockSetActivity == -1){
			TaskComplete();
			return;
		}
		*/
		CSquadMonster::RunTask(pTask);
	break;}
	
	case TASK_WAIT_INDEFINITE:
	
		//likely a mistake.
		//TaskComplete();

		EASY_CVAR_PRINTIF_PRE(stukaPrintout, easyForcePrintLine("MY SCHED BE DDDDDDDDDDDDDDDD %s %d", m_pSchedule->pName, getEnemy() != NULL));

		if(getEnemy() != NULL){
			ChangeSchedule( slStukaBatChaseEnemy);
		}else{
			if(onGround){
				ChangeSchedule( slIdleStand);
			}else{
				ChangeSchedule(slStukaIdleHover);
			}
			
			//TaskComplete();
		}
		//ChangeSchedule( slStukaBatChaseEnemy);
		return;
		//CSquadMonster::RunTask(pTask);
	break;
	

	// This task runs while following the monster and re-routes to stay accurate.
	// Maybe re-routes a little too often, consider doing it every once in a while or rely more
	// on the smart follow system that reroutes at the target moving too far from the previous path
	// destination (LKP). LKP matches the enemy's real position when in plain sight of course.
	case TASK_ACTION:
		// If we're close to our LKP, we need to give up, get stumped and re-route.
		// float distanceToCorpse = (pev->origin - corpseToSeek->pev->origin).Length();  //m_vecEnemyLKP).Length();
		if((pev->origin - m_vecEnemyLKP).Length() < 20 && !this->HasConditions(bits_COND_SEE_ENEMY)){
			//huh. give up for now.
			TaskComplete();
			return;
		}


		//MODDD TODO - is "getEnemy" necessary? Would "m_hEnemy" with a null check before be sufficient?
		//if(getEnemy() != NULL && getEnemy()->Get()->v.deadflag == DEAD_NO){
		if(m_hEnemy != NULL){

			MakeIdealYaw( m_vecEnemyLKP );
			ChangeYaw( pev->yaw_speed );

			/*
			if(pev->deadflag != DEAD_NO){
				//we're not doing this anymore.
				return;
			}
			*/


			if(EASY_CVAR_GET_DEBUGONLY(stukaAdvancedCombat) == 1){
				updateMoveAnim();

			}else{
			
			}

			EASY_CVAR_PRINTIF_PRE(stukaPrintout, easyForcePrintLine("schedule is: %s", m_pSchedule->pName));
			getPathToEnemyCustom();

			//pEnemy == NULL?
			BOOL enemyPresent = !(m_hEnemy==NULL||m_hEnemy.Get()==NULL|| CBaseMonster::Instance(m_hEnemy.Get())==NULL  || m_hEnemy->pev->deadflag != DEAD_NO);
			if(!enemyPresent){
				//Any other points that need this reset to work best first?
				abortAttack();

				if(onGround){
					EASY_CVAR_PRINTIF_PRE(stukaPrintout, easyForcePrintLine("I AM AN ABJECT FAILUR1"));
					this->ChangeSchedule(slIdleStand);
				}else{
					EASY_CVAR_PRINTIF_PRE(stukaPrintout, easyForcePrintLine("I AM AN ABJECT FAILUR2"));
					//??? hover or something?
					this->ChangeSchedule(slStukaBatFail);
				}
				//???????????????????????????????????????????????????????????????????????????
			
				//???
				//m_MonsterState = MONSTERSTATE_ALERT;
			}

			if(enemyPresent){
				easyPrintLineGroup1("THE bat ID: %d - ind: %d en: %d df: %d ename: \"%s\" :::ep: %d, act: %d %d", monsterID, attackIndex, (m_hEnemy==NULL), m_hEnemy->pev->deadflag, STRING(m_hEnemy->pev->classname), enemyPresent, m_Activity, m_IdealActivity  );
			}else{
				easyPrintLineGroup1("THE bat ID: %d - ind: %d en: %d :::ep: %d,  act: %d %d", monsterID, attackIndex, (m_hEnemy==NULL)  , enemyPresent, m_Activity, m_IdealActivity );
			}
		
			if(attackIndex > -1 && !enemyPresent ){
				//no enemy and in attack-mode?  Stop.

				easyPrintLineGroup1("STUKA %d: ABORT ATTACK!!!", monsterID);
				abortAttack();
			}

			if(maxDiveTime != -1 && maxDiveTime <= gpGlobals->time){
				abortAttack();
				blockSetActivity = -1;
				//attackAgainDelay = gpGlobals->time + 3; //don't try diving again for a little.
			}

			if(!enemyPresent){

				//if(attackIndex == -1) attackIndex = -2;  //force to -2.
				attackIndex = -2;
				maxDiveTime = -1;
				combatCloseEnough = FALSE;

				//nothing to see here.
				return;
			}

			dist3d = (pev->origin - m_hEnemy->pev->origin).Length();
			dist2d = (pev->origin - m_hEnemy->pev->origin).Length2D();
			lastEnemey2DDistance = dist2d;
			if(dist3d <= 58){
				combatCloseEnough = TRUE;
			}else{
				combatCloseEnough = FALSE;
			}

			//Don't set m_flGroundSpeed in this method, leave that up to
			//MoveExecute. It can do these checks okay.
			//m_flGroundSpeed = 380;

			//also, if index is non-negative, disallow changing anims automatically (and vice versa)?

			//EASY_CVAR_PRINTIF_PRE(stukaPrintout, easyForcePrintLine("ALL THE yay %d %.2f", attackIndex, dist3d));


			//MODDD TODO - only look like we're about to attack IF we're going towards the goal node
			//(typically #0, or with the GOAL bit set). And that the enemy is in sight.
			//If not, it's just weird looking to attack thin air or when the enemy isn't even directly in front yet.

			BOOL gah = HasConditions(bits_COND_SEE_ENEMY);

			if(HasConditions(bits_COND_SEE_ENEMY) && m_Route[m_iRouteIndex].iType & bits_MF_IS_GOAL){
				//we care about the enemy being in range, proceed as usual.
			}else{
				//easyForcePrintLine("THAT AINT RIGHT");
				//end early and stop attacking if in progress.
				if(attackIndex > -1){
					abortAttack();
				}
				chargeIndex = -1; //at least this...?
				return;
			}

			/*
			if(attackIndex < 0 && this->m_fSequenceFinished){
				//don't be static, just hover.
				blockSetActivity = -1;  //nope.
				//recentActivity = ACT_RESET; //force an animation pick.
				
				//DO YOU HAVE TO DO THIS?!
				this->SetActivity(ACT_HOVER);
			}
			*/

			
			//NOTE - 3 is the melee seqeuence, right?
			if(pev->sequence == 3 && m_fSequenceFinished){
				blockSetActivity = -1;  //nope.
				this->SetActivity(ACT_HOVER);
			}
						
			if( (attackIndex == -1)){

				if(dist3d  <= 72 || (dist3d <= 114 && dist2d <= 55)  ){
					attackIndex = 1;

					//except now perhaps?
					m_flGroundSpeed = STUKABAT_MOVESPEED_ATTACK_CLAW;   //??
					setAnimation("attack_claw", TRUE, FALSE, 2);
					float animScaler = getMeleeAnimScaler();
					pev->framerate = animScaler;
					blockSetActivity = gpGlobals->time + (12.2/12.0 / animScaler);
					//SetActivity(ACT_RANGE_ATTACK2);
					attackEffectDelay = gpGlobals->time + (4.9/12.0 / animScaler);

					//attackAgainDelay =  gpGlobals->time + 0.7;

					lastSetActivitySetting = ACT_HOVER;

					maxDiveTime = -1;
					chargeIndex = -1;

					if (RANDOM_LONG(0,1))
						AttackSound();

				}else{
					attackIndex = 0;

					if(EASY_CVAR_GET_DEBUGONLY(stukaAdvancedCombat) == 1){
						m_flGroundSpeed = 330;
						if(maxDiveTime == -1){
						maxDiveTime = gpGlobals->time + 2;
						}
					}


					if(dist3d < STUKABAT_DIVE_DISTANCE){
						EASY_CVAR_PRINTIF_PRE(stukaPrintout, easyForcePrintLine("START THE DIVING stuff fine fellow! %.2f", gpGlobals->time ));
						chargeIndex = 0;
						if(maxDiveTime == -1){
							//not already that?
							maxDiveTime = gpGlobals->time + 3.4f;
						}
						//block set activity??
					}
					if(dist3d < STUKABAT_ATTACK_DISTANCE){
						chargeIndex = 1;
					}
				}

			}else if(attackIndex == 0){

				if(monsterID == 1){
					int x = 432;
				}

				if(maxDiveTime != -1 && maxDiveTime <= gpGlobals->time){
					abortAttack();
					//diving for too long, not okay.
					//TODO - if this even still works now.
				}

				//gSkillData.
				if(m_hEnemy == NULL){
					easyPrintLineGroup2("Not good, no enemy.");
					return;
				}

				easyPrintLineGroup2("pretty messed up %.2f, %.2f, ", dist3d, dist2d);


				if(dist3d  <= 72 || (dist3d <= 114 && dist2d <= 55)  ){

					attackIndex = 1;
					m_flGroundSpeed = STUKABAT_MOVESPEED_ATTACK_CLAW;  //???
					setAnimation("attack_claw", TRUE, FALSE, 2);
						
					float animScaler = getMeleeAnimScaler();
					pev->framerate = animScaler;
					blockSetActivity = gpGlobals->time + (12.2/12.0 / animScaler);
					//SetActivity(ACT_RANGE_ATTACK2);
					attackEffectDelay = gpGlobals->time + (4.9/12.0 / animScaler);


					lastSetActivitySetting = ACT_HOVER;

					//attackAgainDelay =  gpGlobals->time + 0.7;
					queueAbortAttack = TRUE;

					maxDiveTime = -1;
					chargeIndex = -1;

					if (RANDOM_LONG(0,1))
						AttackSound();
				}//END OF (dist check)
				else{
					

					if(dist3d < STUKABAT_DIVE_DISTANCE){
						EASY_CVAR_PRINTIF_PRE(stukaPrintout, easyForcePrintLine("START THE DIVING stuff fine fellow! %.2f", gpGlobals->time ));
						chargeIndex = 0;
						if(maxDiveTime == -1){
							//not already that?
							maxDiveTime = gpGlobals->time + 3.4f;
						}
						//block set activity??
					}
					if(dist3d < STUKABAT_ATTACK_DISTANCE){
						chargeIndex = 1;
					}
					easyPrintLineGroup2("YAY NO");
				}


			}else if(attackIndex == 1){

				if(attackEffectDelay != -1 && attackEffectDelay <= gpGlobals->time){
				
					attackEffectDelay = -1;
					
					if(m_hEnemy == NULL)return;

					if(dist3d  <= 72 || (dist3d <= 114 && dist2d <= 55)  ){
						m_hEnemy->TakeDamage( pev, pev, gSkillData.stukaBatDmgClaw, DMG_SLASH, ((EASY_CVAR_GET_DEBUGONLY(stukaInflictsBleeding)>0)&&(RANDOM_FLOAT(0,1)<=EASY_CVAR_GET_DEBUGONLY(stukaInflictsBleeding)))?DMG_BLEEDING:0 );   //USED TO HAVE DMG_BLEEDING, test....

						//BEWARE: TakeDamage does NOT draw blood particles.  Do it manually too.
						//DrawAlphaBlood(flDamage, ptr );
						//...huh, we never did the trace for a hit location.   Try getting "CBaseEntity *pHurt = CheckTraceHullAttack( 70, gSkillData.panthereyeDmgClaw, DMG_BLEEDING );"  to work here later.

						UTIL_fromToBlood(this, m_hEnemy, gSkillData.stukaBatDmgClaw, 75);


						if ( (m_hEnemy->pev->flags & (FL_MONSTER|FL_CLIENT)) && !m_hEnemy->blocksImpact() )
						{
							m_hEnemy->pev->punchangle.z = -18;
							m_hEnemy->pev->punchangle.x = 5;
						}
						// Play a random attack hit sound
						UTIL_PlaySound( ENT(pev), CHAN_WEAPON, pAttackHitSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackHitSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5 ) );
						//UTIL_PlaySound(ENT(pev), CHAN_WEAPON, "zombie/claw_strike3.wav", 1.0, 1.0, 0, 100, FALSE);
						
					}
					else
					{
						// Play a random attack miss sound
						UTIL_PlaySound( ENT(pev), CHAN_WEAPON, pAttackMissSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackMissSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5) );
						//UTIL_PlaySound(ENT(pev), CHAN_WEAPON, "zombie/claw_miss1.wav", 1.0, 1.0, 0, 100, FALSE);
					}
				}//END OF if(attackEffectDelay...)

				easyPrintLineGroup2("HOW IS THE SEQUENCE %d", m_fSequenceFinished);

				if(m_fSequenceFinished){
					abortAttack();
					
					//recentActivity = ACT_RESET;
					//m_Activity = ACT_RESET;  //is that ok?
					m_IdealActivity = ACT_HOVER;

					//SetActivity(ACT_HOVER);
					//pev->sequence = 13;
					//setAnimation("Hover", TRUE, TRUE, 0);

					//just in case we drop this from abortAttack.
				}
			}

			if(attackIndex == -2 && attackAgainDelay <= gpGlobals->time){
				//ready to strike again.
				attackIndex = -1;
				maxDiveTime = -1;
				chargeIndex = -1;
				//combatCloseEnough = FALSE;
			}


			//Assuming we're going towards the enemy LKP. If we're close enough, stop.

			if( (this->m_vecEnemyLKP - pev->origin).Length() < 10  ){
				//easyForcePrintLine("ILL enjoy A TURNIP");
				TaskComplete();

			}


		}//END OF if(getEnemey() != NULL)
		else{

			//easyForcePrintLine("YOUR face SURE IS very DISGUSTING");

			TaskComplete();
			//try something new.
			EASY_CVAR_PRINTIF_PRE(stukaPrintout, easyForcePrintLine("WHUT"));

		}

	break;
	case TASK_GET_PATH_TO_ENEMY_LKP:
	
		if(EASY_CVAR_GET_DEBUGONLY(stukaAdvancedCombat) == TRUE){
			updateMoveAnim();
		}
	
	break;
	case TASK_GET_PATH_TO_ENEMY:
	
		if(EASY_CVAR_GET_DEBUGONLY(stukaAdvancedCombat) == TRUE){
			updateMoveAnim();
		}
		//this is a dummy for now.
		TaskComplete();
	
	break;
	case TASK_RANGE_ATTACK2:

	break;

	


	default: 
		CSquadMonster::RunTask ( pTask );
		break;
	}
}




//same as "RadiusDamage", but do reduced damage to friendlies / neutrals.
void CStukaBat::RadiusDamageNoFriendly(entvars_t* pevInflictor, entvars_t*	pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType )
{
	Vector vecSrc = pev->origin;
	float flRadius = flDamage * 2.5;


	CBaseEntity *pEntity = NULL;
	TraceResult	tr;
	float	flAdjustedDamage, falloff;
	Vector		vecSpot;

	if ( flRadius )
		falloff = flDamage / flRadius;
	else
		falloff = 1.0;

	int bInWater = (UTIL_PointContents ( vecSrc ) == CONTENTS_WATER);

	vecSrc.z += 1;// in case grenade is lying on the ground

	if ( !pevAttacker )
		pevAttacker = pevInflictor;

	// iterate on all entities in the vicinity.
	while ((pEntity = UTIL_FindEntityInSphere( pEntity, vecSrc, flRadius )) != NULL)
	{
		if ( pEntity->pev->takedamage != DAMAGE_NO )
		{

			float damageMult = 1.0;

			int rel = IRelationship(pEntity);

			// UNDONE: this should check a damage mask, not an ignore
			if ( (iClassIgnore != CLASS_NONE && !pEntity->isForceHated(this) && pEntity->Classify() == iClassIgnore) ) 
			{
				//biggest reduction.
				damageMult = 0.18;
				//continue;
			}else if(rel == R_NO || rel == R_AL){
				damageMult = 0.25;
			}
			damageMult = 1.0;


			// blast's don't tavel into or out of water
			if (bInWater && pEntity->pev->waterlevel == 0)
				continue;
			if (!bInWater && pEntity->pev->waterlevel == 3)
				continue;

			vecSpot = pEntity->BodyTarget( vecSrc );
			
			UTIL_TraceLine ( vecSrc, vecSpot, dont_ignore_monsters, ENT(pevInflictor), &tr );

			if ( tr.flFraction == 1.0 || tr.pHit == pEntity->edict() )
			{// the explosion can 'see' this entity, so hurt them!
				if (tr.fStartSolid)
				{
					// if we're stuck inside them, fixup the position and distance
					tr.vecEndPos = vecSrc;
					tr.flFraction = 0.0;
				}
				
				// decrease damage for an ent that's farther from the bomb.
				flAdjustedDamage = ( vecSrc - tr.vecEndPos ).Length() * falloff;
				flAdjustedDamage = flDamage - flAdjustedDamage;
			
				if ( flAdjustedDamage < 0 )
				{
					flAdjustedDamage = 0;
				}
				
				flAdjustedDamage *= damageMult;

			
				// ALERT( at_console, "hit %s\n", STRING( pEntity->pev->classname ) );
				if (tr.flFraction != 1.0)
				{
					ClearMultiDamage( );
					pEntity->TraceAttack( pevInflictor, flAdjustedDamage, (tr.vecEndPos - vecSrc).Normalize( ), &tr, bitsDamageType );
					ApplyMultiDamage( pevInflictor, pevAttacker );
				}
				else
				{
					pEntity->TakeDamage ( pevInflictor, pevAttacker, flAdjustedDamage, bitsDamageType );
				}
			}
		}
	}


}


//CHECK ME!
//=========================================================
// GetSchedule - Decides which type of schedule best suits
// the monster's current state and conditions. Then calls
// monster's member function to get a pointer to a schedule
// of the proper type.
//=========================================================
Schedule_t *CStukaBat::GetSchedule ( void )
{
	/*
	//IS THAT OKAY?
	attackIndex = -2;
	maxDiveTime = -1;
	combatCloseEnough = FALSE;
	//...as a matter of fact, it isn't!
	*/

	//MODDD - safety.
	if(pev->deadflag != DEAD_NO){
		return GetScheduleOfType( SCHED_DIE );
	}

	//MODDD - TODO: a check for "setActivityBlock".  If not negative one (animation in progress), uh, just stall (return "wait" or something?)


	//SNDREL:1:hearstuff:1,2:SND:4 4|
	PRINTQUEUE_STUKA_SEND(stukaPrint.soundRelated, "hearstuff:%d", HasConditions( bits_COND_HEAR_SOUND ) );
	if ( HasConditions( bits_COND_HEAR_SOUND ) )
	{
		//PRINTQUEUE_STUKA_SEND(stukaPrint.soundRelated, "I HEAR STUFF??")
		CSound *pSound;
		pSound = PBestSound();

		ASSERT( pSound != NULL );

		if(pSound != NULL){
			PRINTQUEUE_STUKA_SEND(stukaPrint.soundRelated, "SND:%d %d", pSound->m_iType, pSound->m_iType & (bits_SOUND_COMBAT | bits_SOUND_PLAYER));
		}else{
			PRINTQUEUE_STUKA_SEND(stukaPrint.soundRelated, "NULL? AH.");
		}

		//if ( pSound && (pSound->m_iType & bits_SOUND_DANGER) )
		//	return GetScheduleOfType( SCHED_TAKE_COVER_FROM_BEST_SOUND );
		if (pSound != NULL &&  pSound->m_iType & (bits_SOUND_COMBAT|bits_SOUND_PLAYER) ){
			m_afMemory |= bits_MEMORY_PROVOKED;
		}else{
			//maybe bait?
			SCHEDULE_TYPE baitSched = getHeardBaitSoundSchedule(pSound);
			if(baitSched != SCHED_NONE){
				return GetScheduleOfType ( baitSched );
			}
		}
	}


	BOOLEAN canSeeEnemy = FALSE;

	Look( m_flDistLook );

	if (!HasConditions(bits_COND_SEE_HATE)&&
			!HasConditions(bits_COND_SEE_FEAR)&&
			!HasConditions(bits_COND_SEE_DISLIKE)&&
			!HasConditions(bits_COND_SEE_ENEMY)&&
			!HasConditions(bits_COND_SEE_NEMESIS)
			){
			canSeeEnemy = FALSE;

			//MODDD TODO - m_hTargetEnt check? Is that necessary?
	}else if(getEnemy() != NULL || (m_hEnemy != NULL && m_hEnemy->pev->deadflag == DEAD_NO) || m_hTargetEnt!=NULL ){

		canSeeEnemy = TRUE;
	}

	if(m_hEnemy!=NULL){
		PRINTQUEUE_STUKA_SEND(stukaPrint.enemyInfo, "SEE: %s %d", STRING(m_hEnemy->pev->classname), m_hEnemy->pev->deadflag);
	}else{
		PRINTQUEUE_STUKA_SEND(stukaPrint.enemyInfo, "SEE: NULL! %d %d %d", getEnemy()!=NULL, m_hEnemy!=NULL, m_hTargetEnt!=NULL);
	}

	PRINTQUEUE_STUKA_SEND(stukaPrint.enemyInfo, "ENESTA: %d %d %d %d %d :%d::F: %d",
			HasConditions(bits_COND_SEE_HATE) != 0,
			HasConditions(bits_COND_SEE_FEAR) != 0,
			HasConditions(bits_COND_SEE_DISLIKE) != 0,
			HasConditions(bits_COND_SEE_ENEMY) != 0,
			HasConditions(bits_COND_SEE_NEMESIS) != 0,
			(getEnemy() != NULL),
			canSeeEnemy
			);

	if(canSeeEnemy){
		//appetite is gone.  Focus on combat!
		eating = FALSE;
		eatingAnticipatedEnd = -1;
		m_flHungryTime = -1;

		//YEP.
		m_MonsterState = MONSTERSTATE_COMBAT;
		m_IdealMonsterState = MONSTERSTATE_COMBAT;
	}

	//EASY_CVAR_PRINTIF_PRE(stukaPrintout, easyForcePrintLine("Stuka %d GetSchedule: %d", monsterID, m_MonsterState));
	//EASY_CVAR_PRINTIF_PRE(stukaPrintout, easyForcePrintLine("DO I SMELL food %d %d %d ", HasConditions(bits_COND_SMELL_FOOD), canSeeEnemy, FShouldEat()));

	//MODDD - PAY ATTENTION TO THIS,
	// AND THAT "TASK_ALERT_STAND" THING, THAT IS PRETTY NEW.
	//search "whut" for the part you know in TASK_ACTION that is if no enemy is around (do a DEAD_DEAD check on pev->deadflag too)

	PRINTQUEUE_STUKA_SEND(stukaPrint.getSchedule, "FOODREQ:%d %d %d %d", m_MonsterState, (m_MonsterState == MONSTERSTATE_COMBAT || m_MonsterState == MONSTERSTATE_ALERT), canSeeEnemy == FALSE, HasConditions(bits_COND_SMELL_FOOD) || eating  );
	
	BOOL fallToIdling = FALSE;
	BOOL smellz = HasConditions(bits_COND_SMELL_FOOD);
	if(m_MonsterState == MONSTERSTATE_IDLE || m_MonsterState == MONSTERSTATE_COMBAT || m_MonsterState == MONSTERSTATE_ALERT){

		if ( snappedToCeiling == FALSE && canSeeEnemy == FALSE &&
			smellz || eating)
		{

			abortAttack();

			if(!eating){
			
			CSound *pScent;

			pScent = PBestScent();
			PRINTQUEUE_STUKA_SEND(stukaPrint.getSchedule, "scentex:%d", pScent!=NULL);
			

			if(pScent != NULL){

			if(!onGround){
			
				float dist = (pev->origin - pScent->m_vecOrigin).Length();

				if(dist >= 160){
					// food is right out in the open. Just go fly to it.
					return GetScheduleOfType( SCHED_STUKABAT_FINDEAT );

				}else{
					// land to eat

					TraceResult trDown;
					Vector vecStart = pev->origin + Vector(0, 0, 5);
					Vector vecEnd = vecStart + Vector(0, 0, -136);
					UTIL_TraceLine(vecStart, vecEnd, ignore_monsters, ENT(pev), &trDown);

					if(trDown.flFraction < 1.0){
						//found the floor, get down.
						return GetScheduleOfType( SCHED_STUKABAT_ATTEMPTLAND );
					}else{
						//get closer??
						return GetScheduleOfType( SCHED_STUKABAT_FINDEAT );
					}


					EASY_CVAR_PRINTIF_PRE(stukaPrintout, easyForcePrintLine("POO POO 5"));

				}//END of ELSE of distcheck

			}//END OF if(!onGround)
			else{

				float dist = (pev->origin - pScent->m_vecOrigin).Length();

				if(dist >= 180){
					if(onGround && blockSetActivity == -1){
						//for now, assume the only reason you want to move is you're pursuing an active enemy.
						//m_IdealActivity = ACT_LEAP;
						//SetActivity(ACT_LEAP, TRUE);
						setAnimation("Take_off_from_land", TRUE, FALSE, 2);
						//41.6f/12.0f ???
						blockSetActivity = gpGlobals->time + (29.0f/12.0f);
						queueToggleGround = TRUE;

						return GetScheduleOfType(SCHED_STUKABAT_ANIMWAIT);

						//onGround = FALSE;
					}

					EASY_CVAR_PRINTIF_PRE(stukaPrintout, easyForcePrintLine("POO POO 4"));
					
				}
				else if(dist >= 79){
					easyPrintLineGroup2("SCHED_STUKABAT_CRAWLTOFOOD DIST BE %.2f!", dist);
					return GetScheduleOfType( SCHED_STUKABAT_CRAWLTOFOOD );

				}else{
					easyPrintLineGroup2("SCHED_STUKABAT_EAT!");

					eating = TRUE;
					eatingAnticipatedEnd = gpGlobals->time + 7;

					MakeIdealYaw( pScent->m_vecOrigin );
					//EAT THAT
					return GetScheduleOfType( SCHED_STUKABAT_EAT );
				}
			}//END OF else...

			}//END OF if(pScent != NULL)
			else{
				//No enemies, no scents?  let's... try hovering / idle.  
				fallToIdling = TRUE;
			}
			}else{
				//EAT THAT
				return GetScheduleOfType( SCHED_STUKABAT_EAT );
			}
		}//END OF if can smell food
		else{
			if(!snappedToCeiling && !canSeeEnemy){
				fallToIdling = TRUE;
				//just idle out.
			}

		}
	}//END OF IF state is combat or alert

	if(canSeeEnemy){
		//let it raise.
		seekingFoodOnGround = FALSE;
	}



	if(fallToIdling){

		// path find to the enemy regardless.
		if(m_hEnemy != NULL){
			//Go to our enemy in the default monster script!
			//easyForcePrintLine("CAUGHT YOU YA STUPID butt fornicator");
		}else{
			if(onGround){
				return GetScheduleOfType( SCHED_STUKABAT_IDLE_GROUND );
			}else{
				return GetScheduleOfType( SCHED_STUKABAT_IDLE_HOVER );
			}
		}

		

	}

	switch	( m_MonsterState )
	{
	case MONSTERSTATE_IDLE:
		break;

	case MONSTERSTATE_ALERT:
		break;

	case MONSTERSTATE_COMBAT:
		{
			Vector vecTmp = UTIL_Intersect( Vector( 0, 0, 0 ), Vector( 100, 4, 7 ), Vector( 2, 10, -3 ), 20.0 );
			// dead enemy
			if ( HasConditions ( bits_COND_LIGHT_DAMAGE ) )
			{
				// m_iFrustration++;
			}
			if ( HasConditions ( bits_COND_HEAVY_DAMAGE ) )
			{
				// m_iFrustration++;
			}
			//NOTE: MONSTERSTATE_COMBAT of CBaseMonster::GetSchedule()
		}
		break;
	}

	PRINTQUEUE_STUKA_SEND(stukaPrint.getSchedule, "OUTSRC!!!");
	return CSquadMonster::GetSchedule();
}

//=========================================================
//=========================================================
Schedule_t* CStukaBat::GetScheduleOfType ( int Type ) 
{
	//TASK_STRAFE_PATH

	EASY_CVAR_PRINTIF_PRE(stukaPrintout, easyForcePrintLine("STUKA: GET SCHED OF TYPE %d!!!", Type));

	// ALERT( at_console, "%d\n", m_iFrustration );
	switch	( Type )
	{
	case SCHED_DIE:{
		if
		(
			(
			onGround == FALSE &&
			pev->sequence != SEQ_STUKABAT_LAND_GROUND
			)
			||
			snappedToCeiling
			|| (pev->sequence == SEQ_STUKABAT_LAND_CEILING) //landing on or leaving the ceiling?  You fall.
		)
		{
			//flying or hanging? leave this up to whether we support the falling cycler or cut straight to the flying dead animation.
			return flyerDeathSchedule();
		}else{
			//on the ground? handle this normally.
			return slDie;
		}
	break;}
	case SCHED_CHASE_ENEMY:
	{
		easyPrintLineGroup1("STUKA SCHED %d: slStukaBatChaseEnemy", monsterID);
		return slStukaBatChaseEnemy;
		break;
	}
	case SCHED_ALERT_STAND:
	{

		//return slStukaBatChaseEnemy;
		//unwise, just leave this up to the base class.
		break;
	}

	case SCHED_RANGE_ATTACK2:
	{
		return &slStukaBatRangeAttack2[ 0 ];
	}
	case SCHED_TAKE_COVER_FROM_ENEMY:
	case SCHED_TAKE_COVER_FROM_BEST_SOUND:
	case SCHED_TAKE_COVER_FROM_ORIGIN:
	case SCHED_COWER:
	{
		//ignore!
		//easyPrintLineGroup2("STUKA SCHED: slStukaBatTakeCover");
		//return slStukaBatTakeCover;
		easyPrintLineGroup1("STUKA %d:::HAHAHA THERE IS NO COVER FOR THE WEAK", monsterID);
		//return &slStukaBatRangeAttack2[0];
		return &slStukaBatChaseEnemy[0];
	}
	case SCHED_FAIL:
	{
		easyPrintLineGroup1("STUKA %d SCHED: slStukaBatFail", monsterID );
		return &slStukaBatFail[0];
	}
	case SCHED_STUKABAT_FINDEAT:
	{
		easyPrintLineGroup1("STUKA %d SCHED: slStukaBatFindEat", monsterID );
		return &slStukaBatFindEat[0];

	}
	case SCHED_STUKABAT_ATTEMPTLAND:
	{
		easyPrintLineGroup1("STUKA %d SCHED: slStukaBatAttemptLand", monsterID );
		return &slStukaBatAttemptLand[0];

	}
	case SCHED_STUKABAT_CRAWLTOFOOD:
	{
		easyPrintLineGroup1("STUKA %d SCHED: slStukaBatCrawlToFood", monsterID );
		return &slStukaBatCrawlToFood[0];

	}
	case SCHED_STUKABAT_EAT:
	{
		//ALSO INCLUDED: reset fail task.
		m_failSchedule = SCHED_NONE;
		easyPrintLineGroup1("STUKA %d SCHED: slStukaBatEat", monsterID );
		return &slStukaBatEat[0];
	}
	case SCHED_STUKABAT_IDLE_HOVER:
		//just in case?
		blockSetActivity = -1;
		return &slStukaIdleHover[0];
	break;
	case SCHED_STUKABAT_IDLE_GROUND:
		return &slStukaIdleGround[0];
	break;
	
	case SCHED_STUKABAT_ANIMWAIT:
		return &slStukaBatAnimWait[0];
	break;
	//case SCHED_STUKABAT_PATHFIND_STUMPED:
	case SCHED_PATHFIND_STUMPED:  // no, just override the default call for the stumped wait method.
		return &slStukaPathfindStumped[0];
	break;


	//SCHED_TARGET_CHASE
	break;
	}//END OF switch(...)
	
	easyPrintLineGroup1("STUKA %d SCHED: %d", monsterID, Type);
	return CBaseMonster::GetScheduleOfType( Type );
}



//NOTE: being done by logic instead of the schedule system.
BOOL CStukaBat::CheckRangeAttack1 ( float flDot, float flDist )
{
	//make this require being the leader, and make it kind of rare, possibly (or have a delay b/w 'suicides' of squad members)
	return FALSE;

	/*
	BOOL toReturn;

	//check for a straight-shot too!
	if ( flDot > 0.5 && flDist > 0 && flDist <= 400 )
	{
		toReturn = TRUE;
	}else{
		toReturn = FALSE;
	}
	return toReturn;
	*/
}

BOOL CStukaBat::CheckRangeAttack2 ( float flDot, float flDist )
{
	return FALSE;
}


BOOL CStukaBat::CheckMeleeAttack1 ( float flDot, float flDist )
{
	//no activity.  Disabled.
	return FALSE;

	BOOL toReturn;

	if ( flDist <= 64 && flDot >= 0.7 && m_hEnemy != NULL) //&& FBitSet ( m_hEnemy->pev->flags, FL_ONGROUND ) )
	{
		 toReturn = TRUE;
	}else{
		toReturn = FALSE;
	}
	
	//BOOL toReturn = CBaseMonster::CheckMeleeAttack1(flDot, flDist);

	return toReturn;
}



BOOL CStukaBat::allowedToSetActivity(void){
	return TRUE;
}


void CStukaBat::SetActivity( Activity NewActivity){

	SetActivity(NewActivity, FALSE);
}

void CStukaBat::SetActivity ( Activity NewActivity, BOOL forceReset )
{

	//CBaseMonster::SetActivity(NewActivity);
	//return;

	//return;
	////easyForcePrintLine("STEP tried setact");
	
	forceReset = FALSE;

	if(recentActivity == ACT_RESET){
		//NO NO NO
		//recentActivity = NewActivity;
	}


	if(NewActivity == ACT_RESET){
		//no can do.
		//recentActivity = ACT_RESET;  //this okay?
		return;
	}

	BOOL warpRandomAnim = FALSE;


	if(eating && m_hEnemy != NULL){
		//not eating anymore, interrupted.
		eating = FALSE;
	}

	if(eating && NewActivity != ACT_EAT && NewActivity != ACT_DIESIMPLE && NewActivity != ACT_DIEVIOLENT){
		//nothing else allowed.
		return;
	}


	EASY_CVAR_PRINTIF_PRE(stukaPrintout, easyForcePrintLine("LISTEN HERE YA LITTLE man %d %d bat:%.2f : ct:%.2f", m_Activity, m_IdealActivity, blockSetActivity, gpGlobals->time));

	////easyForcePrintLine("STEP 0");
	easyPrintLineGroup3("blockSetActivity:%.2f time:%.2f", blockSetActivity, gpGlobals->time);

	if(blockSetActivity == -1){
		//pass.

	}else{
		lastSetActivitySetting = NewActivity;
		lastSetActivityforceReset = forceReset;
		return;
	}
	//canSetAnim = TRUE;

	////easyForcePrintLine("STEP 1");
	if(NewActivity == ACT_RANGE_ATTACK2){
		canSetAnim = FALSE;
		//do the drill!
		//canSetAnim = TRUE;


	}else if(NewActivity == ACT_RANGE_ATTACK1){
		canSetAnim = FALSE;

	}else{
		//not attacking now.  -2?
		//attackIndex = -1;

		//canSetAnim = TRUE;

	}
	EASY_CVAR_PRINTIF_PRE(stukaPrintout, easyForcePrintLine("STUKABAT: SETACTIVITY: %d %d %d", NewActivity, recentActivity, m_Activity)); 

	
	////easyForcePrintLine("STEP 2");
	if(!onGround){
		if(NewActivity == ACT_IDLE){
			//BOOL someBool = (getEnemy() == NULL);
			EASY_CVAR_PRINTIF_PRE(stukaPrintout, easyForcePrintLine("SO WHAT THE yay IS THAT yay %d %d %d", NewActivity, snappedToCeiling, attackIndex));

			//easyPrintLineGroup2("ATTA:::::%d %d %d", attackIndex, m_MonsterState, someBool);
			if(attackIndex > -1){
				EASY_CVAR_PRINTIF_PRE(stukaPrintout, easyForcePrintLine("I am incredibly 1"));
				return;
			}else{
				//if(m_MonsterState == MONSTERSTATE_COMBAT || someBool){
				//only when snapped to ceiling, this check isn't too bad though.

				if(!snappedToCeiling){
					//hover instead, no "hanging" idle anim while mid-air.
					NewActivity = ACT_HOVER;
				}else{
					//?
				}
			}
		}
		////easyForcePrintLine("STEP 3");


		if(NewActivity == ACT_HOVER){
			if(snappedToCeiling){
				NewActivity = ACT_IDLE;  //don't try to hover when tied to the ceiling.
			}

		}
		
	}
	////easyForcePrintLine("STEP 4");

	//easyPrintLineGroup2("STUKA %d: SET ACTIVITY: %d", monsterID, NewActivity);

	if(recentActivity == NewActivity){
		EASY_CVAR_PRINTIF_PRE(stukaPrintout, easyForcePrintLine("I am incredibly 2: OVERDONE ACT, PHAIL! %d", NewActivity));
		return;
	}else{
		EASY_CVAR_PRINTIF_PRE(stukaPrintout, easyForcePrintLine("OH SHIT, STUKA!!!"));
	}

	EASY_CVAR_PRINTIF_PRE(stukaPrintout, easyForcePrintLine("NEW Aaaaactivity is %d", NewActivity));
	
	////easyForcePrintLine("STEP 5");

	if(onGround){
		if(NewActivity == ACT_IDLE){
			NewActivity = ACT_CROUCHIDLE;
		}
	}else{
		if(!snappedToCeiling && NewActivity == ACT_IDLE){
			
			EASY_CVAR_PRINTIF_PRE(stukaPrintout, easyForcePrintLine("AHAH I SEE THAT stuff MAN"));
			//you do it in the air right?
			NewActivity = ACT_HOVER;
		}
		//If we're snapped to the ceiling, leave ACT_IDLE the way it is.
	}


	//this is for more control.
	recentActivity = NewActivity;
	////easyForcePrintLine("STEP 6");

	BOOL allowedToInterruptSelf = TRUE;
	BOOL forceLoop = FALSE;

	if(NewActivity == ACT_CROUCHIDLE){
		//warp it!
		//warpRandomAnim = TRUE;

		forceLoop = TRUE;
		allowedToInterruptSelf = FALSE;
	}

	int iSequence;
	//CBaseMonster::SetActivity( NewActivity );

	//easyPrintLineGroup2("deb3 %d %d %d", (NewActivity != ACT_CROUCHIDLE), !warpRandomAnim, m_fSequenceFinished) ;
	
	EASY_CVAR_PRINTIF_PRE(stukaPrintout, easyForcePrintLine("LISTEN HERE YA LITTLE man2 na:%d ma:%d mi:%d mf:%d", NewActivity, m_Activity, m_IdealActivity, m_fSequenceFinished));

	if(NewActivity != ACT_CROUCHIDLE || !warpRandomAnim || m_fSequenceFinished){


		//calling the parent is okay.
		CBaseMonster::SetActivity(NewActivity);

		////iSequence = LookupActivity ( NewActivity );
		//int activity = NewActivity;
		//ASSERT( activity != 0 );
		//void *pmodel = GET_MODEL_PTR( ENT(pev) );

		////easyPrintLine("YOU lucky person %d", warpRandomAnim);
		//if(!warpRandomAnim){
		//	//iSequence = ::LookupActivity( pmodel, pev, activity );
		//	iSequence = LookupActivityHard( activity );
		//}else{

		//	while(true){
		//		studiohdr_t *pstudiohdr;
	
		//		pstudiohdr = (studiohdr_t *)pmodel;
		//		if (! pstudiohdr){
		//			iSequence = 0;
		//			break;
		//		}

		//		mstudioseqdesc_t	*pseqdesc;

		//		pseqdesc = (mstudioseqdesc_t *)((byte *)pstudiohdr + pstudiohdr->seqindex);

		//		int weighttotal = 0;
		//		int seq = ACTIVITY_NOT_AVAILABLE;

		//		if(!warpRandomAnim){
		//			for (int i = 0; i < pstudiohdr->numseq; i++)
		//			{
		//				if (pseqdesc[i].activity == activity)
		//				{
		//					weighttotal += pseqdesc[i].actweight;
		//					if (!weighttotal || RANDOM_LONG(0,weighttotal-1) < pseqdesc[i].actweight){
		//						//easyPrintLineGroup2("deb4 %d %d", i, pseqdesc[i].actweight);
		//						seq = i;
		//					}
		//				}
		//			}
		//		}else{
		//			for (int i = 0; i < pstudiohdr->numseq; i++)
		//			{
		//				if (pseqdesc[i].activity == activity)
		//				{
		//					weighttotal += pseqdesc[i].actweight;
		//					if (!weighttotal || RANDOM_LONG(0,weighttotal-1) < pseqdesc[i].actweight){
		//						//easyPrintLineGroup2("yay YOU %d %d", i, pseqdesc[i].actweight);
		//						seq = i;
		//					}
		//				}
		//			}
		//			//easyPrintLineGroup2("deb5 %d", seq);
		//		}
		//		//return seq;
		//		iSequence = seq;
		//		break;
		//	}//END OF while(true)    method immitation.

		//}
		////easyPrintLineGroup2("deb6 %d %d %d", allowedToInterruptSelf, pev->sequence != iSequence, m_fSequenceFinished);

		//easyPrintLine("LISTEN HERE YA LITTLE man3 na:%d ma:%d mi:%d mf:%d atis:%d cs:%d ns:%d", NewActivity, m_Activity, m_IdealActivity, m_fSequenceFinished, allowedToInterruptSelf, pev->sequence, iSequence);



		//if(allowedToInterruptSelf || pev->sequence != iSequence || m_fSequenceFinished){
		//	// Set to the desired anim, or default anim if the desired is not present
		//	if ( iSequence > ACTIVITY_NOT_AVAILABLE )
		//	{
		//		//MODDD - added "forceReset"
		//		if ( forceReset || (pev->sequence != iSequence || !m_fSequenceLoops) )
		//		{
		//			// don't reset frame between walk and run
		//			if ( !(m_Activity == ACT_WALK || m_Activity == ACT_RUN) || !(NewActivity == ACT_WALK || NewActivity == ACT_RUN))
		//				pev->frame = 0;
		//		}

		//		
		//		//animFrameCutoffSuggestion = 255;   //in case it has leftover changes from anywhere else.

		//		easyPrintLineGroup2("ohship");
		//		easyPrintLine("I JUST GOT SEQUENCE %d", iSequence);
		//		pev->sequence		= iSequence;	// Set to the reset anim (if it's there)
		//		ResetSequenceInfo( );
		//		SetYawSpeed();

		//		//is that okay?...
		//		if(forceLoop){
		//			m_fSequenceLoops = TRUE;
		//		}else{
		//			//m_fSequenceLoops = FALSE;
		//			//???
		//		}


		//	}
		//	else
		//	{
		//		// Not available try to get default anim
		//		ALERT ( at_aiconsole, "%s has no sequence for act:%d\n", STRING(pev->classname), NewActivity );
		//		pev->sequence		= 0;	// Set to the reset anim (if it's there)
		//	}
		//}//END OF if(allowedToInterruptSelf...)

		//m_Activity = NewActivity; // Go ahead and set this so it doesn't keep trying when the anim is not present
	
		//// In case someone calls this with something other than the ideal activity
		//m_IdealActivity = m_Activity;

	}//END OF if(NewActivity != ACT_CROUCHIDLE || !warpRandomAnim || m_fSequenceFinished)



	//?????????????????????????????????????
	iSequence = ACTIVITY_NOT_AVAILABLE;


	//switch ( m_Activity)???
	switch(NewActivity)
	{
	case ACT_WALK:
		m_flGroundSpeed = STUKABAT_MOVESPEED_WALK;

		
		//iSequence = LookupActivity ( ACT_WALK_HURT );

		break;

	case ACT_HOVER:
		m_flGroundSpeed = STUKABAT_MOVESPEED_HOVER;

		break;
	case ACT_FLY:
		m_flGroundSpeed = STUKABAT_MOVESPEED_FLYING_CYCLER;
		//m_flGroundSpeed = 88;

		break;
	case ACT_FLY_LEFT:
		m_flGroundSpeed = STUKABAT_MOVESPEED_FLYING_TURN;
		break;
	case ACT_FLY_RIGHT:
		m_flGroundSpeed = STUKABAT_MOVESPEED_FLYING_TURN;
		break;




	case ACT_RANGE_ATTACK2:

		break;

	default:

		/*
		//Uhh.. is this a good idea?
		if(attackIndex <= -1){
			m_flGroundSpeed = 65;
		}
		*/
		break;
	}


}

//=========================================================
// RunAI
//=========================================================
void CStukaBat::RunAI( void )
{
	CBaseMonster::RunAI();

	// for what purpose
	//if ( HasMemory( bits_MEMORY_KILLED ) )
	//	return;
}


void CStukaBat::SetTurnActivity(){
	//this is for ground only.
	//NOTE: stukabat has no (ground) turn anim.  Oh well.

	
	//activity for any turning in flight is hovering.
	
	if(!onGround && !snappedToCeiling){
		m_IdealActivity = ACT_HOVER;
	}




	//CSquadMonster::SetTurnActivity();
}

//void CStukaBat::SetTurnActivity ( void )
void CStukaBat::SetTurnActivityCustom ( void )
{

	//easyForcePrintLine("SetTurnActivityCustom:::%d %d %d %d", m_IdealActivity, combatCloseEnough, blockSetActivity, onGround);
	if(m_IdealActivity == ACT_FLY && !combatCloseEnough  && blockSetActivity == -1 && !onGround){

	}else{
		return;
	}

	float flYD;
	//what is this garbage...
	//flYD = FlYawDiff();

	flYD = pev->ideal_yaw - pev->angles.y;

	if(fabs( flYD) > 180){
		flYD -= 360;
	}
	//return

	//if(flYD
	
	dontResetActivity = TRUE;
	//stukas turn fast, have a low tolerance...

	if ( flYD <= -6 )
	{// big right turn
		//used to set  m_idealActivity
		//m_movementActivity = ACT_FLY_RIGHT;
		if(moveFlyNoInterrupt == -1){
			setAnimation("Flying_turn_right", TRUE, TRUE, 3);
			m_flGroundSpeed = STUKABAT_MOVESPEED_FLYING_TURN;
			//recentActivity = ACT_FLY;
			moveFlyNoInterrupt = gpGlobals->time + 9.2/12.0;
		}
	}
	else if ( flYD >= 6 )
	{// big left turn
		//m_movementActivity = ACT_FLY_LEFT;
		if(moveFlyNoInterrupt == -1){
			setAnimation("Flying_turn_left", TRUE, TRUE, 3);
			m_flGroundSpeed = STUKABAT_MOVESPEED_FLYING_TURN;
			//recentActivity = ACT_FLY;
			moveFlyNoInterrupt = gpGlobals->time + 9.2/12.0;
		}
	}else{
		if(moveFlyNoInterrupt == -1){

			m_flGroundSpeed = STUKABAT_MOVESPEED_FLYING_CYCLER;
			//if(m_IdealActivity == ACT_FLY)
			//MODDD - why was this turning looping off? Are you daft man??!
			//(and yes I'm talking to myself. Classy)
				setAnimation("Flying_Cycler", TRUE, TRUE, 3);
				//recentActivity = ACT_FLY;
				//moveFlyNoInterrupt = gpGlobals->time + 11.0/12.0;
				moveFlyNoInterrupt = gpGlobals->time + (26.0-1.6)/35.0;
			//}
		}
	}
	
	dontResetActivity = FALSE;

	//float flCurrentYaw = UTIL_AngleMod( pev->angles.y );
	//MakeIdealYaw( m_vecEnemyLKP );

	ChangeYaw( pev->yaw_speed );

	/*
	easyPrintLineGroup2("STUKA #%d : FLYING yay SON %.2 %df", monsterID, flYD, m_movementActivity);
	UTIL_printLineVector("ANG", pev->angles);
	easyPrintLineGroup2("yay?  angle.y: %.2f ideal_yaw:%.2f ::::::flYD:%.2f",  pev->angles.y, pev->ideal_yaw, flYD);
	*/

}


Activity CStukaBat::GetStoppedActivity(){

	//if on the ground, "ACT_IDLE".
	//otherwise, "ACT_HOVER".
	//for now, flying all the time.  Edit later.

	Activity actreturn;
	if(onGround){
		actreturn = ACT_CROUCHIDLE;
	}else{
		//wait no...
		actreturn = ACT_HOVER;
		/*
		if(combatCloseEnough){
			
			actreturn = ACT_HOVER;
		}else{
			
			actreturn = ACT_FLY;
		}
		*/
	}
	//NO
	//damn...?
	actreturn = ACT_RESET;

	easyPrintLineGroup2("WHAT THE yay OUTTA NOWHERE %d", actreturn);
	return actreturn;

}


void CStukaBat::Stop( void ) 
{ 
	//EASY_CVAR_PRINTIF_PRE(stukaPrintout, easyForcePrintLine("YOU SHALL PERISH ALONG WITH THE REST OF YOUR amazing KIN"));
	//m_IdealActivity = GetStoppedActivity();
	//NO STOPPING fine fellow!
}



//#define DIST_TO_CHECK	200
#define DIST_TO_CHECK	3000
void CStukaBat::Move ( float flInterval ) 
{
	//CSquadMonster::Move(flInterval);
	//IS THIS WISE???
	CBaseMonster::Move( flInterval );
}



BOOL CStukaBat::ShouldAdvanceRoute( float flWaypointDist, float flInterval )
{
	//was 32?  yea let's scale this back again.
	//if ( flWaypointDist <= 500  )
	if ( flWaypointDist <= 50  )
	{
		return TRUE;
	}
	return FALSE;
	//return CSquadMonster::ShouldAdvanceRoute(flWaypointDist, flInterval);
}











//Clone of the flyer's CheckLocalMove.
int CStukaBat::CheckLocalMove ( const Vector &vecStart, const Vector &vecEnd, CBaseEntity *pTarget, BOOL doZCheck, float *pflDist )
{
	// UNDONE: need to check more than the endpoint
	if (FBitSet(pev->flags, FL_SWIM) && (UTIL_PointContents(vecEnd) != CONTENTS_WATER))
	{
		// ALERT(at_aiconsole, "can't swim out of water\n");
		return FALSE;
	}

	TraceResult tr;
	Vector vecStartTrace = vecStart + Vector(0, 0, 16);
	Vector vecEndTrace = vecEnd + Vector(0, 0, 16);
	//Vector vecStartTrace = vecStart;
	//Vector vecEndTrace = vecEnd;

	//MODDD - try smaller?
	//UTIL_TraceHull( vecStart + Vector( 0, 0, 32 ), vecEnd + Vector( 0, 0, 32 ), dont_ignore_monsters, large_hull, edict(), &tr );
	
	
	//UTIL_TraceHull( vecStartTrace, vecEnd + Vector( 0, 0, 16 ), dont_ignore_monsters, head_hull, edict(), &tr );
	TRACE_MONSTER_HULL(edict(), vecStartTrace, vecEndTrace, dont_ignore_monsters, edict(), &tr);


	// ALERT( at_console, "%.0f %.0f %.0f : ", vecStart.x, vecStart.y, vecStart.z );
	// ALERT( at_console, "%.0f %.0f %.0f\n", vecEnd.x, vecEnd.y, vecEnd.z );

	if (pflDist)
	{
		*pflDist = ( (tr.vecEndPos ) - vecStartTrace ).Length();// get the distance.
	}

	// ALERT( at_console, "check %d %d %f\n", tr.fStartSolid, tr.fAllSolid, tr.flFraction );
	

	/*
	//Uh.. is that okay?
	// hmm.. no
	if(tr.fStartSolid){
		//or should we do thsi?
		Vector vecDir = (vecEnd - vecStart).Normalize();
		UTIL_TraceHull( vecStartTrace + vecDir * 6, vecEnd + Vector( 0, 0, 32 ), dont_ignore_monsters, head_hull, edict(), &tr );
		
		return LOCALMOVE_VALID;
	}
	*/

	if(tr.flFraction >= 1.0){
		// just say yes
		return LOCALMOVE_VALID;
	}

	
	if (tr.fStartSolid || tr.flFraction < 1.0)
	//if(tr.flFraction < 1.0)
	{

		/*
		if(gpGlobals->trace_ent != NULL){
			CBaseEntity* tempEnt = CBaseEntity::Instance(gpGlobals->trace_ent );
			easyForcePrintLine("??? %s", tempEnt->getClassname());
		}
		*/

		if ( pTarget && pTarget->edict() == gpGlobals->trace_ent )
			return LOCALMOVE_VALID;
		return LOCALMOVE_INVALID;
	}

	return LOCALMOVE_VALID;
}








//NOTICE - there are three possible routes here. Comment the ones above to fall down to that route.
/*
1. (most of this method) LOOSE. like how the controller does it. TraceHull (instead of the usual WALK_MOVE).
Unfortunately any larger than point_hull can cause false positives (places the stuka mistakenly thinks it can't go
but obviously can fit through).

2. At the very end (CheckLocalMove), the usual way monsters do collision. Unsure if this is always ok for flyers.
May make them averse to going over floorless / steep gaps (thinks walking monsters can't cross).
*/
/*
int CStukaBat::CheckLocalMove ( const Vector &vecStart, const Vector &vecEnd, CBaseEntity *pTarget, float *pflDist )
{
	int iReturn = LOCALMOVE_VALID;
	//MODDD - experimental.  Using a copy of the controller's CheckLocalMove again, but with a different hull type.
	
	TraceResult tr;
	//UTIL_TraceHull( vecStart + Vector( 0, 0, 32), vecEnd + Vector( 0, 0, 32), dont_ignore_monsters, large_hull, edict(), &tr );
	//UTIL_TraceHull( vecStart + Vector( 0, 0, 32), vecEnd + Vector( 0, 0, 32), dont_ignore_monsters, head_hull, edict(), &tr );
	
	
	//UTIL_TraceHull( vecStart + Vector( 0, 0, 4), vecEnd + Vector( 0, 0, 4), dont_ignore_monsters, point_hull, edict(), &tr );
	
	edict_t* toIgnore;
	if(pTarget == NULL){
		toIgnore = NULL;
	}else{
		toIgnore = pTarget->edict();
	}
	TRACE_MONSTER_HULL(edict(), pev->origin, vecEnd, dont_ignore_monsters, toIgnore, &tr);




	if (pflDist)
	{
		*pflDist = ( (tr.vecEndPos - Vector( 0, 0, 4 )) - vecStart ).Length();// get the distance.
	}

	// ALERT( at_console, "check %d %d %f\n", tr.fStartSolid, tr.fAllSolid, tr.flFraction );
	//if (tr.fStartSolid || tr.flFraction < 1.0)
	if (tr.fAllSolid || tr.flFraction < 1.0)
	{
		//if ( pTarget && pTarget->edict() == gpGlobals->trace_ent ){
		if ( pTarget && pTarget->edict() == tr.pHit ){
			iReturn = LOCALMOVE_VALID;
		}else{
			iReturn = LOCALMOVE_INVALID;
		}
	}
	//SPECIAL DRAWS just for the stuka. uncomment to see this.
	//
	//switch(iReturn){
	//	case LOCALMOVE_INVALID:
	//		//ORANGE
	//		//DrawRoute( pev, m_Route, m_iRouteIndex, 239, 165, 16 );
	//		DrawMyRoute( 48, 33, 4 );
	//	break;
	//	case LOCALMOVE_INVALID_DONT_TRIANGULATE:
	//		//RED
	//		//DrawRoute( pev, m_Route, m_iRouteIndex, 234, 23, 23 );
	//		DrawMyRoute( 47, 5, 5 );
	//	break;
	//	case LOCALMOVE_VALID:
	//		//GREEN
	//		//DrawRoute( pev, m_Route, m_iRouteIndex, 97, 239, 97 );
	//		DrawMyRoute( 20, 48, 20 );
	//	break;
	//}
	
	return iReturn;

	//Not reached. Comment out all above to do this instead.
	return CBaseMonster::CheckLocalMove(vecStart, vecEnd, pTarget, doZCheck, pflDist);
}
*/




void CStukaBat::setAnimation(char* animationName){
	if(queueToggleSnappedToCeiling)return;
	if(!dontResetActivity){recentActivity = ACT_RESET;}
	EASY_CVAR_PRINTIF_PRE(stukaPrintout, easyForcePrintLine("ANIMATION SET1: %s ", animationName));

	pev->framerate = 1;
	animFrameCutoffSuggestion = 255;

	CBaseMonster::setAnimation(animationName, FALSE, -1, 0);
	m_flFramerateSuggestion = 1;
}

void CStukaBat::setAnimation(char* animationName, BOOL forceException){
	if(queueToggleSnappedToCeiling)return;
	if(!dontResetActivity){recentActivity = ACT_RESET;}
	EASY_CVAR_PRINTIF_PRE(stukaPrintout, easyForcePrintLine("ANIMATION SET2: %s ", animationName));
	
	pev->framerate = 1;
	animFrameCutoffSuggestion = 255;

	CBaseMonster::setAnimation(animationName, forceException, -1, 0);
	m_flFramerateSuggestion = 1;
}

void CStukaBat::setAnimation(char* animationName, BOOL forceException, BOOL forceLoopsProperty){
	if(queueToggleSnappedToCeiling)return;
	if(!dontResetActivity){recentActivity = ACT_RESET;}
	EASY_CVAR_PRINTIF_PRE(stukaPrintout, easyForcePrintLine("ANIMATION SET3: %s ", animationName));
	
	pev->framerate = 1;
	animFrameCutoffSuggestion = 255;

	CBaseMonster::setAnimation(animationName, forceException, forceLoopsProperty, 0);
	m_flFramerateSuggestion = 1;
}

void CStukaBat::setAnimation(char* animationName, BOOL forceException, BOOL forceLoopsProperty, int extraLogic){
	if(queueToggleSnappedToCeiling)return;

	//AHHH SHIT?
	if(!dontResetActivity){recentActivity = ACT_RESET;}
	EASY_CVAR_PRINTIF_PRE(stukaPrintout, easyForcePrintLine("ANIMATION SET4: %s ", animationName));
	
	pev->framerate = 1;
	animFrameCutoffSuggestion = 255;

	if( !strcmp(animationName, "attack_claw")){
	//	animFrameCutoffSuggestion = 120;
	}

	CBaseMonster::setAnimation(animationName, forceException, forceLoopsProperty, extraLogic);
	m_flFramerateSuggestion = 1;

	usingCustomSequence = TRUE;
	//???
}


void CStukaBat::safeSetMoveFlyNoInterrupt(float timer){
	moveFlyNoInterrupt = gpGlobals->time + timer;

}
void CStukaBat::safeSetBlockSetActivity(float timer){
	blockSetActivity = gpGlobals->time + timer;
	queueToggleGround = FALSE;
	snappedToCeiling = FALSE;
	queueToggleSnappedToCeiling = FALSE;
	queueAbortAttack = FALSE;
}


void CStukaBat::MoveExecute( CBaseEntity *pTargetEnt, const Vector &vecDir, float flInterval )
{

	//MODDD - TESTING?!!
	//return CSquadMonster::MoveExecute(pTargetEnt, vecDir, flInterval);



	tryDetachFromCeiling();

	PRINTQUEUE_STUKA_SEND(stukaPrint.moveRelated, "MOVE1");
	if(snappedToCeiling){
		//that okay?
		m_velocity = Vector(0,0,0);
		pev->velocity = m_velocity;
		lastVelocityChange = gpGlobals->time;
		return;   //can't move.
	}
	
	
	PRINTQUEUE_STUKA_SEND(stukaPrint.moveRelated, "MOVE2 (%d)", seekingFoodOnGround);
	//Notice that this method being called means we are trying to move.
	//Only movement on the ground to crawl to food is allowed.  Otherwise, take to flight.
	if(!seekingFoodOnGround){

		if(onGround && blockSetActivity == -1){
			//for now, assume the only reason you want to move is you're pursuing an active enemy.
			//m_IdealActivity = ACT_LEAP;
			//SetActivity(ACT_LEAP, TRUE);
			setAnimation("Take_off_from_land", TRUE, FALSE, 2);
			//41.6f/12.0f ???
			blockSetActivity = gpGlobals->time + (29.0f/12.0f);
			queueToggleGround = TRUE;
			//onGround = FALSE;

			//MODDD - EXPERIMENTAL.  Is that okay?
			ChangeSchedule( slStukaBatAnimWait);
			return;

			//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			//should we "return" here...?
			//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		}
		//"Land_ceiling"
		//easyPrintLineGroup2("EEEEEEE %.2f, %.2f", blockSetActivity, gpGlobals->time);

		if(onGround){
			easyPrintLineGroup2("YOU ARE A goodness gracious me!");
			m_velocity = m_velocity * 0.2;
			pev->velocity = m_velocity;
			lastVelocityChange = gpGlobals->time;
			return;
		}
	
	}
	
	PRINTQUEUE_STUKA_SEND(stukaPrint.moveRelated, "MOVE3 (%d)", onGround);
	
	//EASY_CVAR_PRINTIF_PRE(stukaPrintout, easyForcePrintLine("&&&&& %.2f %.2f", pev->framerate, this->m_flFrameRate));
	//pev->yaw_speed = 4;

	if(!onGround){
		//flying!
		//EASY_CVAR_PRINTIF_PRE(stukaPrintout, easyForcePrintLine("ERRR flyint: %.2f,  attackin: %d", moveFlyNoInterrupt, attackIndex));

		//charge-anims not allowed at death, needless to say.
		if(pev->deadflag == DEAD_NO){
			//if(blockSetActivity == -1 && moveFlyNoInterrupt == -1 && attackIndex <= 0 ){ //m_movementActivity != ACT_HOVER){
			if(chargeIndex == 0){
				//setAnimation("Hover");
				//moveFlyNoInterrupt = 
				//dist check....
				EHANDLE* tempEnemy = getEnemy();

				if(tempEnemy != NULL && tempEnemy->Get() != NULL){

					m_flGroundSpeed = STUKABAT_MOVESPEED_DIVE_CYCLER;
					
					dontResetActivity = TRUE;
					setAnimation("Dive_cycler", TRUE, FALSE, 3);
					dontResetActivity = FALSE;

					safeSetMoveFlyNoInterrupt(15.0f / 15.0f);
					safeSetBlockSetActivity(15.0f / 15.0f);
					

				}

				//setAnimation("Attack_bomb", TRUE, TRUE, 3);
				//moveFlyNoInterrupt = 26.0f/20.0f


				//setAnimation("Dive_cycler", TRUE, TRUE, 3);
				//moveFlyNoInterrupt = 7.0f/15.0f


			}else if(chargeIndex == 1){
				EHANDLE* tempEnemy = getEnemy();

				if(tempEnemy != NULL && tempEnemy->Get() != NULL){

					m_flGroundSpeed = STUKABAT_MOVESPEED_ATTACK_BOMB;
					dontResetActivity = TRUE;
					setAnimation("Attack_bomb", TRUE, FALSE, 3);
					dontResetActivity = FALSE;
					moveFlyNoInterrupt = gpGlobals->time + 25.0f/20.0f;
					blockSetActivity = gpGlobals->time +  25.0f/20.0f;

					//block set activity??
				}

			}
		

		}//END OF if(pev->deadflag == DEAD_NO)



		//if(moveFlyNoInterrupt == -1){
		if(chargeIndex <= -1){
		



			//UTIL_MakeAimVectors( pev->angles );
			UTIL_MakeVectors( pev->angles );
			float x = DotProduct( gpGlobals->v_forward, m_velocity );
			float y = DotProduct( gpGlobals->v_right, m_velocity );
			float z = DotProduct( gpGlobals->v_up, m_velocity );

			BOOL shouldHover = FALSE;

			if(m_hEnemy != NULL){
				
				if(
					(m_movementActivity==ACT_HOVER && lastEnemey2DDistance < 430) ||
					(m_movementActivity!=ACT_HOVER && lastEnemey2DDistance < 320)
				){
					shouldHover = TRUE;
				}
			}else{
				if(m_Route[ m_iRouteIndex ].iType & bits_MF_IS_GOAL){
					//if going towards the goal, check the distance between me and the goal node.
					//NOTICE - this does not factor in the current node that isn't neceessarily the goal node being
					//extremely close to the goal, thus not counting to hover instead. Eh whatever, unlikely scenario.
					float distToGoal2D = (m_Route[ m_iRouteIndex ].vecLocation - pev->origin).Length2D();
					if(
						(m_movementActivity==ACT_HOVER && distToGoal2D < 430) ||
						(m_movementActivity!=ACT_HOVER && distToGoal2D < 320)
					){
						shouldHover = TRUE;
					}
				}
			}


			if(shouldHover){
				m_flGroundSpeed = STUKABAT_MOVESPEED_HOVER;
				m_movementActivity = ACT_HOVER;
			}else if (fabs(x) > fabs(y) && fabs(x) > fabs(z))
			{
				
				//wait, setTurnActivityCustom is about to set the ground speed anyways.
				//m_flGroundSpeed = 350;
				m_movementActivity = ACT_FLY;

				SetTurnActivityCustom();
				/*
				if (x > 0)
					return LookupSequence( "forward");
				else
					return LookupSequence( "backward");
				*/
				//m_movementActivity = m_IdealActivity;
			}
			else if (fabs(y) > fabs(z))
			{
				//m_flGroundSpeed = 350;
				m_movementActivity = ACT_FLY;

				SetTurnActivityCustom();
				/*
				if (y > 0)
					m_movementActivity = ACT_FLY_LEFT;
				else
					m_movementActivity = ACT_FLY_RIGHT;
				*/
				//SetTurnActivity();
				/*
				//turn??
				if (y > 0)
					return LookupSequence( "right");
				else
					return LookupSequence( "left");
					*/
				//m_movementActivity = m_IdealActivity;
			}
			else
			{
				m_flGroundSpeed = STUKABAT_MOVESPEED_HOVER;
				m_movementActivity = ACT_HOVER;

				//return LookupSequence( "Hover");
				/*
				if (z > 0)
					return LookupSequence( "up");
				else
					return LookupSequence( "down");
					*/
			}
			//return LookupSequence("Flying_cycler");
		}//END OF if(moveFlyNoInterrupt == -1)

	}//END OF if(!on
	else{

		m_flGroundSpeed = STUKABAT_MOVESPEED_WALK;
		m_movementActivity = ACT_WALK;

	}

	if(onGround){
		//just to be safe...
		combatCloseEnough = FALSE;
	}

	PRINTQUEUE_STUKA_SEND(stukaPrint.moveRelated, "MOVE4 (%d)", combatCloseEnough);
	if(combatCloseEnough){
		//slow down!
		m_flGroundSpeed = STUKABAT_MOVESPEED_HOVER;
		m_movementActivity = ACT_HOVER;

		if ( m_IdealActivity != m_movementActivity )
			m_IdealActivity = m_movementActivity;

		m_velocity = m_velocity * 0.2;
		pev->velocity = m_velocity;
		lastVelocityChange = gpGlobals->time;
		return;  //no movement if so.
	}

	PRINTQUEUE_STUKA_SEND(stukaPrint.moveRelated, "MOVE5");

	//m_movementActivity = ACT_FLY;
	easyPrintLineGroup2("AAAAAAAAACTIVITY IDEAL & MOV: %d %d ", m_IdealActivity, m_movementActivity);


	if(onGround && !FacingIdeal()){

		//Trust the schedule is already telling it to turn to face the right way (but be aware if it freezes when it SHOULD be turning, then we could do it here too)
		//MakeIdealYaw( m_vecEnemyLKP );
		//ChangeYaw( pev->yaw_speed );

		if(m_fSequenceFinished){
			//can't do anything, just idle?
			setAnimation("Subtle_fidget_on_ground", TRUE, TRUE, 2);
		}

		//NOTE: stuka has no turn activity...
		//SetTurnActivity();
		//don't go, wait for turn to finish?
		return;
	}
	if(onGround){
		//let's let the usual movement handle ground movement.
		CBaseMonster::MoveExecute(pTargetEnt, vecDir, flInterval);
		return;
	}

	if ( m_IdealActivity != m_movementActivity )
		m_IdealActivity = m_movementActivity;

	//easyPrintLineGroup2("TIMEVAR %.2f::%.2f", pev->framerate, flInterval);

	float flTotal = 0;
	float flStepTimefactored = m_flGroundSpeed*EASY_CVAR_GET_DEBUGONLY(STUSpeedMulti) * pev->framerate * EASY_CVAR_GET_DEBUGONLY(animationFramerateMulti) * flInterval;
	float flStep = m_flGroundSpeed * 1 * 1;
	


	float velMag = flStep * EASY_CVAR_GET_DEBUGONLY(STUSpeedMulti);

	float timeAdjust = (pev->framerate * EASY_CVAR_GET_DEBUGONLY(animationFramerateMulti) * flInterval);
	float distOneFrame = velMag * pev->framerate * EASY_CVAR_GET_DEBUGONLY(animationFramerateMulti) * flInterval;
	
	Vector dest = m_Route[ m_iRouteIndex ].vecLocation;
	Vector vectBetween = (dest - pev->origin);
	float distBetween = vectBetween.Length();
	Vector dirTowardsDest = vectBetween.Normalize();
	Vector _velocity;

	if(distOneFrame <= distBetween){
		_velocity = dirTowardsDest * velMag;
	}else{
		_velocity = dirTowardsDest * distBetween/timeAdjust;
	}

	//UTIL_printLineVector("MOVEOUT", velMag);
	//easyPrintLineGroup2("HELP %.8ff %.8f", velMag, flInterval);

	//UTIL_drawLineFrame(pev->origin, dest, 64, 255, 0, 0);

	
	m_velocity = m_velocity * 0.8 + _velocity * 0.2;
	lastVelocityChange = gpGlobals->time;

	//m_velocity = m_velocity * 0.8 + m_flGroundSpeed * vecDir * 0.2;

	Vector flatVelocity = Vector(_velocity.x, _velocity.y, 0);
	Vector vertVelocity = Vector(0, 0, _velocity.z);

	//pev->velocity = _velocity;
	pev->velocity = m_velocity;
	lastVelocityChange = gpGlobals->time;

	Vector vecSuggestedDir = (m_Route[m_iRouteIndex].vecLocation - pev->origin).Normalize();

	checkFloor(vecSuggestedDir, velMag, flInterval);
	//CBaseMonster::MoveExecute(pTargetEnt, vecDir, flInterval);

	//easyForcePrintLine("WHATS YOUR oh hay VELOCITY?? %.2f %.2f %.2f", pev->velocity.x, pev->velocity.y, pev->velocity.z);
	
	//easyForcePrintLine("ON YOUR WAY TO THE GOAL?? %d: %d", m_iRouteIndex, m_Route[m_iRouteIndex].iType & bits_MF_IS_GOAL);


	/*
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(testVar);

	//UTIL_MoveToOrigin ( ENT(pev), pev->origin + vecTotalAdjust , vecTotalAdjust.Length(), MOVE_STRAFE );
	UTIL_MoveToOrigin ( ENT(pev), pev->origin + Vector(0, EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(testVar), 0), fabs(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(testVar)), MOVE_STRAFE );
	*/

	/*
	easyForcePrintLine("WELL ya???? %.2f : %d", EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(testVar), (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(testVar) == -0.1f));
	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(testVar) == -0.1f){
		this->FRefreshRoute();
		EASY_CVAR_SET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(testVar, 0);
	}
	*/



	if(EASY_CVAR_GET_DEBUGONLY(drawDebugPathfinding2) == 1){
		//if( ((int)gpGlobals->time) % 2 == 1){
		UTIL_drawLineFrame(pev->origin, pev->origin + vecSuggestedDir * velMag * 5, 48, 0, 255, 255);
		//}else{
			//UTIL_drawLineFrame(pev->origin, pev->origin + vecDir * flStep * 12, 16, 135, 0, 0);
		//}
	}

}


int CStukaBat::ISoundMask ( void )
{
	
	return	bits_SOUND_WORLD	|
			bits_SOUND_COMBAT	|
			bits_SOUND_CARCASS	|
			bits_SOUND_MEAT		|
			bits_SOUND_GARBAGE	|
			bits_SOUND_PLAYER	|
			//MODDD - new
			bits_SOUND_BAIT;
	//MODDD - give me a schedule for going to investigate the chumtoad croak?

}

void CStukaBat::Activate(void){
	
	//EASY_CVAR_PRINTIF_PRE(stukaPrintout, easyForcePrintLine("STU flag@@@  %d  ", pev->spawnflags ));
	CSquadMonster::Activate();
}

void CStukaBat::DefaultSpawnNotice(void){
	
	CSquadMonster::DefaultSpawnNotice();
}
void CStukaBat::ForceSpawnFlag(int arg_spawnFlag){

	//set our spawn flag first...
	CSquadMonster::ForceSpawnFlag(arg_spawnFlag);
}


void CStukaBat::MonsterThink(){
	//UTIL_drawLineFrame(pev->origin, pev->origin + UTIL_YawToVec(pev->ideal_yaw)*60, 4, 255, 255, 24);
	//Vector vecTry = UTIL_YawToVec(pev->ideal_yaw);
	//EASY_CVAR_PRINTIF_PRE(stukaPrintout, easyForcePrintLine("ang: %.2f vect: (%.2f %.2f %.2f)", pev->ideal_yaw, vecTry.x, vecTry.y, vecTry.z));
	//EASY_CVAR_PRINTIF_PRE(stukaPrintout, easyForcePrintLine("onGround:%d queueToggleGround%d", onGround, queueToggleGround));

	if(
		(this->m_Activity == ACT_HOVER || this->m_IdealActivity == ACT_HOVER) &&
		(
			this->m_pSchedule != slStukaBatChaseEnemy &&
			m_fSequenceFinished &&
			(usingCustomSequence || attackIndex==-2)
		)
	){
		//reset it?? This can hapen when in the attack schedule (TASK_ACTION is prominent).
		//It changes the animation, but keeps the activity set the way it is (likely ACT_HOVER).
		//So when it stops the attack, the game needs to be told to pick a new fitting sequence for ACT_HOVER instead.
		SetActivity(ACT_HOVER);
	}



	SetYawSpeed();

	if(pev->deadflag == DEAD_NO){

		if(pev->deadflag == DEAD_NO){
			//not dead? influences on velocity not allowed.
			//TEST...
			//pev->velocity = Vector(0,0,0);
		}
		//easyPrintLineGroup2("STUKA m_movementActivity: %d", m_movementActivity);
		//easyPrintLineGroup2("STUKA m_IdealActivity: %d %d", m_IdealActivity, m_Activity);
		////easyForcePrintLine("BLOCKSET::::%.2f", blockSetActivity);

		if(pev->deadflag == DEAD_NO && lastVelocityChange != -1 && (gpGlobals->time - lastVelocityChange) > 0.24  ){
			//no edits to velocity?  Start slowing down a lot.
			m_velocity = m_velocity * 0.15;
			pev->velocity = m_velocity;
			lastVelocityChange = gpGlobals->time;
		}else{

		}
	
		if(landBrake == TRUE){
			pev->velocity.x = pev->velocity.x * 0.07;
			pev->velocity.y = pev->velocity.y * 0.07;
			//Z is unaffected.
		}


		if(moveFlyNoInterrupt != -1 && moveFlyNoInterrupt <= gpGlobals->time){
			moveFlyNoInterrupt = -1;
		}

		if(eatingAnticipatedEnd != -1 && eatingAnticipatedEnd <= gpGlobals->time){
			eating = FALSE;
			eatingAnticipatedEnd = -1;
		}

		/*
		//Don't do it this way, that is just plain bad.
		//We have a perfectly good activity for this.
		if(eating){
			//setAnimation("Eat_on_ground", TRUE, TRUE, 2);
			//???
			setAnimation("Eat_on_ground", TRUE, TRUE, 2);
		}
		*/

		//easyPrintLineGroup2("GGGGGGG %.2f %.2f", blockSetActivity, gpGlobals->time);
		if(blockSetActivity != -1 && blockSetActivity <= gpGlobals->time){
			//done:
			blockSetActivity = -1;
			////easyForcePrintLine("lastSetActivitySetting: %d", lastSetActivitySetting);
			if(lastSetActivitySetting != ACT_RESET){

				////easyForcePrintLine("toggleGround:%d snappedToCeil:%d", queueToggleGround, queueToggleSnappedToCeiling);
				if(queueToggleGround){
					//SetActivity(ACT_HOVER, lastSetActivityforceReset);
				}else if(queueToggleSnappedToCeiling){

				}else{
				//if(!queueToggleSnappedToCeiling && !queueToggleGround){
					SetActivity(lastSetActivitySetting, lastSetActivityforceReset);
				}
			}

			EASY_CVAR_PRINTIF_PRE(stukaPrintout, easyForcePrintLine("TOGGLE GROUND!  NOW IS %d %d", onGround, queueToggleGround));
			if(queueToggleGround){
			
				onGround = !onGround;

				if(!onGround){
					//ENABLE???
					//WAIT. Do we want this or MOVETYPE_FLY ??? AHHHHH.
					pev->movetype = MOVETYPE_BOUNCEMISSILE;
					pev->flags |= FL_FLY;
				
					seekingFoodOnGround = FALSE;
					//in the air?  not interested in food, clearly.

					//pev->origin.z += 41;
					UTIL_MoveToOrigin ( ENT(pev), pev->origin + Vector(0, 0, 41), 41, MOVE_STRAFE );
				
					//you sure about that? ACT_FLY? why not ACT_HOVER?
					//m_IdealActivity = ACT_HOVER;

					//setactivity?

					//pev->movetype =


					recentActivity = ACT_RESET;
					SetActivity(ACT_HOVER);

					//turn the auto-pushers back on, don't want to snag.
					turnThatOff = FALSE;

					//adjust for the suggested change in position from that take-off anim
				}else{

					//imply we landed, adjust Z.
					//pev->origin = Vector(pev->origin.x, pev->origin.y, pev->origin.z - 8);
					//
					//UTIL_MoveToOrigin ( ENT(pev), pev->origin + Vector(0, 0, 41), 41, MOVE_STRAFE );
					//snap to ground!

					//Vector vecEnd = vecStart + Vector(0, 0, 38);


					TraceResult tr;
					UTIL_TraceLine(pev->origin, pev->origin + Vector(0, 0, -20), ignore_monsters, ENT(pev), &tr);
					if(tr.flFraction < 1.0){
						pev->origin = tr.vecEndPos;
					}else{
						UTIL_MoveToOrigin ( ENT(pev), pev->origin + Vector(0, 0, -8), 41, MOVE_STRAFE );
					}

					pev->movetype = MOVETYPE_STEP;
					pev->flags &= ~FL_FLY;

					//on-ground is true?  okay, finish task.
					seekingFoodOnGround = TRUE;
					TaskComplete();
					//queueToggleGround();


					//???  Just landed!

				}
				lastSetActivitySetting = ACT_RESET;
			}
			queueToggleGround = FALSE;

			if(queueToggleSnappedToCeiling){
				snappedToCeiling = !snappedToCeiling;

				easyPrintLineGroup2("ONCEILING IS NOW %d!", snappedToCeiling);
			

				if(!snappedToCeiling){
					//pev->origin.z += 41;  //??????
					//m_IdealActivity = ACT_FLY;
					//adjust for the suggested change in position from that take-off anim

					recentActivity = ACT_RESET;
					SetActivity(ACT_HOVER);


				}else{
					//snpped?  IDLE.

					m_IdealActivity = ACT_IDLE;
					//adjust for the suggested change in position from that take-off anim
					//pev->origin.z += 41;  //??????
					recentActivity = ACT_RESET;
					SetActivity(ACT_IDLE);


				}
				lastSetActivitySetting = ACT_RESET;

			}
			queueToggleSnappedToCeiling = FALSE;

			if(queueAbortAttack){
				abortAttack();
			}
			queueAbortAttack = FALSE;

		}else{
		
		}
		//easyPrintLineGroup2("Debug Test 12345 %.2f :::%d %d :::%d %d :::%d :::%d", blockSetActivity, onGround, snappedToCeiling, queueToggleGround, queueToggleSnappedToCeiling, m_IdealActivity, seekingFoodOnGround);
	
		EHANDLE* tempEnemy = getEnemy();
	
		//EASY_CVAR_PRINTIF_PRE(stukaPrintout, easyForcePrintLine("STET %d %d, %.2f %.2f", HasConditions( bits_COND_HEAR_SOUND ), m_MonsterState, gpGlobals->time, m_flWaitFinished));

		// STUDetection
		//0: never care.  Wait until a loud noise / getting hit by an attack provokes me.
		//1: reduced sight when not provoked and not hanging.
		//2: normal sight all the time except for when starting off hanging.

		switch( (int) EASY_CVAR_GET_DEBUGONLY(STUDetection) ){
		case 0:
			if( m_afMemory & bits_MEMORY_PROVOKED){
				m_flFieldOfView		= VIEW_FIELD_FULL;
			}else{
				m_flFieldOfView		= VIEW_FIELD_FULL;	
			}
		break;
		case 1:
			if( m_afMemory & bits_MEMORY_PROVOKED){
				m_flFieldOfView		= VIEW_FIELD_FULL;
			}else{
				m_flFieldOfView		= 0.35; //slim.
			}
		break;
		case 2:
			if( m_afMemory & bits_MEMORY_PROVOKED){
				m_flFieldOfView		= VIEW_FIELD_FULL;
			}else{
				m_flFieldOfView		= VIEW_FIELD_FULL;	
			}
		break;
		default:
			//???
		break;
		}

		//HEARING STUFF USED TO BE HERE, MOVED TO getSchedule (where hearing something forces "getSchedule" to get called again).

		if(m_hEnemy != NULL && m_hTargetEnt != NULL){
			//EASY_CVAR_PRINTIF_PRE(stukaPrintout, easyForcePrintLine("ENEMEH INFO: ENEM: %s TARG: %s", STRING(m_hEnemy->pev->classname), STRING(m_hTargetEnt->pev->classname) ));
			stukaPrint.enemyInfo.sendToPrintQueue("ENEMEH INFO: ENEM: %s TARG: %s", STRING(m_hEnemy->pev->classname), STRING(m_hTargetEnt->pev->classname)  );
		}if(m_hEnemy != NULL){
			//EASY_CVAR_PRINTIF_PRE(stukaPrintout, easyForcePrintLine("ENEMEH INFO: ENEM: %s", STRING(m_hEnemy->pev->classname) ));
			stukaPrint.enemyInfo.sendToPrintQueue("ENEMEH INFO: ENEM: %s", STRING(m_hEnemy->pev->classname)   );

		}else if(m_hTargetEnt != NULL){
			//EASY_CVAR_PRINTIF_PRE(stukaPrintout, easyForcePrintLine("ENEMEH INFO: TARG: %s", STRING(m_hTargetEnt->pev->classname) ));
			stukaPrint.enemyInfo.sendToPrintQueue("ENEMEH INFO: TARG: %s", STRING(m_hTargetEnt->pev->classname)   );
		}else{
			//EASY_CVAR_PRINTIF_PRE(stukaPrintout, easyForcePrintLine("ENEMEH INFO: NOTHING" ));
			stukaPrint.enemyInfo.sendToPrintQueue("E1: nothin" );
		}

		stukaPrint.general.sendToPrintQueue("GENERAL: ATTA: %d STAT: %d", attackIndex, m_MonsterState );

	}//END OF dead check


	if(EASY_CVAR_GET_DEBUGONLY(stukaPrintout) == 1){
		stukaPrint.printOutAll();
	}

	stukaPrint.clearAll();

	EASY_CVAR_PRINTIF_PRE(stukaPrintout, easyForcePrintLine("HOW DARE YOU %d %d %d", monsterID, m_afMemory & bits_MEMORY_PROVOKED, wakeUp));

	CSquadMonster::MonsterThink();
}

float CStukaBat::HearingSensitivity(){

	if(snappedToCeiling){
		return 0.58f;
	}else if( !(m_afMemory & bits_MEMORY_PROVOKED)){
		//not provoked?  Hearing ability depends on stuka detection.
		switch((int)EASY_CVAR_GET_DEBUGONLY(STUDetection) ){
		case 0:
			return 0.65f;
		break;
		case 1:
			return 0.89f;
		break;
		case 2:
			//always hear.
			return 1.5f;
		break;
		default:
			return 1.5f;
		break;
		}//END OF switch(EASY_CVAR_GET_DEBUGONLY(STUDetection))
	}else{
		//provoked?  We hear well.
		return 1.5f;
	}

}



void CStukaBat::customTouch(CBaseEntity *pOther){

	//easyPrintLineGroup2("STUKA %D: I\'M TOUCHIN\'", monsterID);

	//CBaseMonster::Touch(pOther); lots of other touch methods don't do this.  Unnecessary?
}



//inline
void CStukaBat::checkTraceLine(const Vector& vecSuggestedDir, const float& travelMag, const float& flInterval, const Vector& vecStart, const Vector& vecRelativeEnd, const int& moveDist){
	checkTraceLine(vecSuggestedDir, travelMag, flInterval, vecStart, vecRelativeEnd, moveDist, TRUE);
}
//Vector& const vecRelstar, ???   Vector& const reactionMove,
//inline
void CStukaBat::checkTraceLine(const Vector& vecSuggestedDir, const float& travelMag, const float& flInterval, const Vector& vecStart, const Vector& vecRelativeEnd, const int& moveDist, const BOOL canBlockFuture){
	

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

			Vector vecMoveRepel = (tr.vecPlaneNormal*toMove*EASY_CVAR_GET_DEBUGONLY(STUrepelMulti) )/1;
			
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




			//pev->origin = pev->origin + tr.vecPlaneNormal*toMove*EASY_CVAR_GET(repelMulti);
			//easyPrintLineGroup2("MOOO %s: SPEED: %.2f", STRING(tr.pHit->v.classname), travelMag );
			EASY_CVAR_PRINTIF_PRE(stukaPrintout, UTIL_printLineVector("VECCCC", tr.vecPlaneNormal ) );

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
void CStukaBat::checkTraceLineTest(const Vector& vecSuggestedDir, const float& travelMag, const float& flInterval, const Vector& vecStart, const Vector& vecRelativeEnd, const int& moveDist){
	checkTraceLineTest(vecSuggestedDir, travelMag, flInterval, vecStart, vecRelativeEnd, moveDist, TRUE);
}
//Vector& const vecRelstar, ???   Vector& const reactionMove,
inline
void CStukaBat::checkTraceLineTest(const Vector& vecSuggestedDir, const float& travelMag, const float& flInterval, const Vector& vecStart, const Vector& vecRelativeEnd, const int& moveDist, const BOOL canBlockFuture){
	
	
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

			//pev->origin = pev->origin + tr.vecPlaneNormal*toMove*EASY_CVAR_GET(repelMulti);
			//easyPrintLineGroup2("MOOO %s: SPEED: %.2f", STRING(tr.pHit->v.classname), travelMag );
			EASY_CVAR_PRINTIF_PRE(stukaPrintout, UTIL_printLineVector("VECCCC", tr.vecPlaneNormal ) );

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









void CStukaBat::checkFloor(const Vector& vecSuggestedDir, const float& travelMag, const float& flInterval){



	//No more my friend, as long as you have enough rubies.
	//what am I smoking
	return;





	if(turnThatOff){
		//we're not doing the checks in this case.
		return;
	}
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





//plus or minus, so effectively double these in degrees.
//Flyers get more tolerance at least while flying.
//The method that involves this from CBaseMonster is probably overridden here and this call is never made.
float CStukaBat::MoveYawDegreeTolerance(){
	//return -1;

	if(onGround){
		//tight, not a great ground mover.
		return 20;
	}else{
		return 60; //I shine in the skies.
	}

	
}//END OF MoveYawDegreeTolerance





BOOL CStukaBat::usesAdvancedAnimSystem(void){
	return TRUE;
}

/*
void CStukaBat::SetActivity( Activity NewActivity ){
	CBaseMonster::SetActivity(NewActivity);
}
*/


//NOTICE - the stukabat has never used this system before.
//Proceed with extreme caution.
int CStukaBat::LookupActivityHard(int activity){
	int i = 0;
	m_flFramerateSuggestion = 1;
	m_iForceLoops = -1;
	pev->framerate = 1;
	//any animation events in progress?  Clear it.
	resetEventQueue();

	animFrameCutoffSuggestion = -1; //just in case?


	//Within an ACTIVITY, pick an animation like this (with whatever logic / random check first):
	//    this->animEventQueuePush(10.0f / 30.0f, 3);  //Sets event #3 to happen at 1/3 of a second
	//    return LookupSequence("die_backwards");      //will play animation die_backwards

	//no need for default, just falls back to the normal activity lookup.
	switch(activity){
		case ACT_CROUCHIDLE:
			m_iForceLoops = TRUE;
			return CBaseAnimating::LookupActivity(activity);
		break;
		case ACT_RANGE_ATTACK1:
			//won't work, stuka directly sets the claw animation.
			animFrameCutoffSuggestion = 200; //cutoff?
			return CBaseAnimating::LookupActivity(activity);
		break;
		case ACT_FLY:
			m_iForceLoops = TRUE;
			return CBaseAnimating::LookupActivity(activity);
		break;
		case ACT_HOVER:
			//force this to loop?

			m_iForceLoops = TRUE;
			return CBaseAnimating::LookupActivity(activity);
		break;
		case ACT_SMALL_FLINCH:

			animFrameCutoffSuggestion = 140;

			return CBaseAnimating::LookupActivity(activity);
		break;
		case ACT_DIESIMPLE:
			if
			(
				(
				onGround == FALSE &&
				pev->sequence != SEQ_STUKABAT_LAND_GROUND
				)
				|| snappedToCeiling || pev->sequence == SEQ_STUKABAT_LAND_CEILING
			)
			{
				return SEQ_STUKABAT_DEATH_FALL_SIMPLE;
			}else{
				return SEQ_STUKABAT_DIE_ON_GROUND;
			}
		break;
		case ACT_DIEVIOLENT:
			//only in the air, or hanging from the ceiling.
			if
			(
				(
				onGround == FALSE &&
				pev->sequence != SEQ_STUKABAT_LAND_GROUND
				)
				|| snappedToCeiling || pev->sequence == SEQ_STUKABAT_LAND_CEILING
			)
			{
				return SEQ_STUKABAT_DEATH_FALL_VIOLENT;
			}else{
				return SEQ_STUKABAT_DIE_ON_GROUND;
			}
			
		break;

	}//END OF switch(...)
	
	//not handled by above?  try the real deal.
	return CBaseAnimating::LookupActivity(activity);
}//END OF LookupActivityHard(...)

//quick reference
/*
SEQ_STUKABAT_LAND_CEILING
SEQ_STUKABAT_LAND_GROUND
SEQ_STUKABAT_TAKE_OFF_FROM_LAND

SEQ_STUKABAT_DEATH_FALL_SIMPLE,
SEQ_STUKABAT_DEATH_FALL_VIOLENT,
SEQ_STUKABAT_DIE_ON_GROUND,
*/




int CStukaBat::tryActivitySubstitute(int activity){
	int i = 0;

	//no need for default, just falls back to the normal activity lookup.
	switch(activity){
		case ACT_HOVER:
			return CBaseAnimating::LookupActivity(activity);
		break;
	}//END OF switch(...)


	//not handled by above? Rely on the model's anim for this activity if there is one.
	return CBaseAnimating::LookupActivity(activity);
}//END OF tryActivitySubstitute(...)




void CStukaBat::ReportAIState(void){

	//call the parent, and add on to that.
	CSquadMonster::ReportAIState();

	easyForcePrintLine("attackIndex:%d chargeIndex:%d seqfin:%d seq:%d fr:%.2f frr:%.2f co:%.2f, cos:%.2f", attackIndex, chargeIndex, m_fSequenceFinished, pev->sequence, pev->frame, pev->framerate, animFrameCutoff, animFrameCutoffSuggestion);
	easyForcePrintLine("man %d %d bat:%.2f : ct:%.2f", m_Activity, m_IdealActivity, blockSetActivity, gpGlobals->time);
	

}//END OF ReportAIState()


//TODO - Pretty sure the stuka has turn activities, use them.


Schedule_t* CStukaBat::GetStumpedWaitSchedule(){
	return slStukaPathfindStumped;
}//END OF GetStumpedWaitSchedule




int CStukaBat::getLoopingDeathSequence(void){
	return SEQ_STUKABAT_FALL_CYCLER;
}



BOOL CStukaBat::violentDeathAllowed(void){
	return TRUE;
}

BOOL CStukaBat::violentDeathDamageRequirement(void){
	//little more fragile since it's a flyer so easier to knock around from impact?
	return (m_lastDamageAmount >= 10);
}

BOOL CStukaBat::violentDeathClear(void){
	//No, it's a falling death. Don't do a linetrace.
	return TRUE;
}//END OF violentDeathAllowed

//Just allow violent death all the time, doubt the other GUTSHOT and HEADSHOT ACT's even have any sequences for me.
int CStukaBat::violentDeathPriority(void){
	return 1;
}//END OF violentDeathPriority

/*
Activity CBaseMonster::GetDeathActivity ( void ){

	if(violentDeathDamageRequirement()){
		//Flyers are a little harder to hit in a direction maybe so just go ahead and register if the damage is met.
		return ACT_DIE_VIOLENT;
	}

	return CSquadMonster::GetDeathActivity(void);
}//END OF GetDeathActivity
*/


Activity CStukaBat::getIdleActivity(void){

	if(snappedToCeiling){
		return ACT_IDLE;
	}else{
		if(onGround){
			return ACT_CROUCHIDLE;
		}else{
			//a way to "stand" relatively still in mid-air?
			return ACT_HOVER;
		}

	}


}//END OF getIdleActivity)


// Override to tell whether this monster prefers to look for bits_NODE_GROUND or AIR
// nodes.  Again, going from the ground to air, returning AIR is still needed so that it tries
// those nodes.
// Although the way things work now, it just jumps to mid-air if the target is up to far away.
// So it probably works fine really.
// (Stukabat sets FL_FLY on and off which affects how picking up the _NODE to use works)
// Actually going to allow using ground nodes if in flying-mode and the map lacks them.
int CStukaBat::getNodeTypeAllowed(void){
	if(onGround){
		return bits_NODE_LAND;
	}else{
		if(map_anyAirNodes){
			// then I only want to use air nodes.
			return bits_NODE_AIR;
		}else{
			// Why miss out on what's useful.  Air nodes will still be preferred if one is available.
			return bits_NODE_LAND | bits_NODE_AIR;
		}
	}
	
	return -1;
}

int CStukaBat::getHullIndexForNodes(void){
	// standard.
	return NODE_FLY_HULL;
}
int CStukaBat::getHullIndexForGroundNodes(void){
	return NODE_SMALL_HULL;
}
