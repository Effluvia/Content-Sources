
#include "chumtoad.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "soundent.h"
#include "gamerules.h"
#include "schedule.h"
#include "activity.h"
#include "util_model.h"
#include "defaultai.h"
#include "game.h"
#include "gib.h"
#include "util_debugdraw.h"

//????
//#include "effects.h"

#define CHUMTOAD_CROAK_ATTRACT_RANGE 500
#define CHUMTOAD_NORMAL_FOV 0.3
#define CHUMTOAD_PLAYDEAD_FOV VIEW_FIELD_FULL



// This is the proper way to make something fall to the ground.
//ClearBits( someEnt->pev->flags, FL_ONGROUND );
//pev->groundentity = NULL
//someEnt->pev->groundentity = NULL;


//MODDD TODO SUPER DUPER ULTRA MEGA CRITICAL ok we get it jesus

// Chumtoads. Have a "ACT_SWIM" sequence: "swim".  Awwwwwww shit.

// I guess something simple would be for chumtoads thrown that detect a pev->waterlevel of 3 to float to the top and just stay there
// while doing this activity.

// The nodes definitely do not support movement between water and air/land. HOO boy, what were those dev's thinking.

// Also, ACT_SWIM isn't even touched by the ichy.  The leech and player use it though, but no default behavior (CBaseMonster or nodes.cpp) uses it.

// ALSO a word about the flinches now supported:  They are not very likely to happen.
// Small flinch is hard to trigger overall in any running away schedules (is there just one?), and
// they're all still likely to be overshadowed by playing dead, which, if decided, gives flinching no chance of happening.


EASY_CVAR_EXTERN_DEBUGONLY(noFlinchOnHard)
EASY_CVAR_EXTERN_DEBUGONLY(chumtoadPrintout)
EASY_CVAR_EXTERN_DEBUGONLY(chumtoadPlayDeadFoolChance)


int CChumToad::numberOfEyeSkins = -1;


enum chumtoad_e {  //key: frames, FPS
	CHUMTOAD_IDLE1 = 0,  //31, 16
	CHUMTOAD_IDLE2,  //31, 16
	CHUMTOAD_IDLE3,  //31, 16
	CHUMTOAD_FLINCH1, //23, 16
	CHUMTOAD_FLINCH2, //11, 16
	CHUMTOAD_HOP1,     //19, 60
	CHUMTOAD_HOP2,     //19, 40
	CHUMTOAD_SWIM, //31, 16
	CHUMTOAD_DIE1, //36, 
	CHUMTOAD_DIE2, //51, 25
	CHUMTOAD_DIE3, //41, 30
	CHUMTOAD_PLAYDEAD_START, //26, 16
	CHUMTOAD_PLAYDEAD_IDLE, //21, 40
	CHUMTOAD_PLAYDEAD_END, //26, 16

};



//custom tasks
enum
{
	TASK_TOAD_INIT_WAIT_FOR_DROP = LAST_COMMON_TASK + 1,
	TASK_TOAD_WAIT_FOR_DROP,
	TASK_TOAD_WAIT_FOR_SLIDE,
	TASK_TOAD_LAND_START,
	//TASK_TOAD_CHECKSEEKSHORT,
	//TASK_TOAD_HOPSHORT,
	TASK_TOAD_CHECKSEEKPOSSIBLE,
	TASK_TOAD_HOPPOSSIBLE,
	TASK_TOAD_IDLEWAITRANDOM,
	TASK_TOAD_PLAYDEAD,
	TASK_TOAD_PLAYDEAD_IDLE,
	TASK_TOAD_UNPLAYDEAD_URGENT,
	TASK_TOAD_UNPLAYDEAD

};

//custom schedules
enum
{
	SCHED_TOAD_INIT_WAIT_FOR_DROP = LAST_COMMON_SCHEDULE + 1,
	SCHED_TOAD_WAIT_FOR_DROP,
	SCHED_TOAD_INIT_FALLSLIDE,
	SCHED_TOAD_LAND,
	//SCHED_TOAD_MOVESHORT,
	SCHED_TOAD_MOVEPOSSIBLE,
	SCHED_TOAD_RUNAWAY,
	SCHED_TOAD_IDLE_WAIT,
	SCHED_TOAD_PLAY_DEAD,
	SCHED_TOAD_PLAYDEAD_IDLE,
	SCHED_TOAD_UNPLAY_DEAD_URGENT,
	SCHED_TOAD_UNPLAY_DEAD,
	SCHED_TOAD_WAKE_ANGRY,
	SCHED_TOAD_ALERT_FACE,
	SCHED_TOAD_ALERT_SMALL_FLINCH,
	SCHED_TOAD_ALERT_BIG_FLINCH,
	SCHED_TOAD_ALERT_STAND,
};

	//{ TASK_GET_PATH_TO_ENEMY,	(float)0		},
	//{ TASK_RUN_PATH,			(float)0		},
	//{ TASK_WAIT_FOR_MOVEMENT,	(float)0		},

Task_t	tlInitWaitForDrop[] =
{
	{ TASK_SET_FAIL_SCHEDULE, (float)SCHED_TOAD_LAND},
	//{ TASK_STOP_MOVING,				0				},
	{ TASK_TOAD_INIT_WAIT_FOR_DROP,	0				},
	{ TASK_SET_SCHEDULE, (float)SCHED_TOAD_INIT_FALLSLIDE },
	//{ TASK_SET_ACTIVITY,			(float)ACT_IDLE	},
	//{ TASK_WAIT_PVS,				0				},
};
Schedule_t	slInitWaitForDrop[] =
{
	{
		tlInitWaitForDrop,
		ARRAYSIZE ( tlInitWaitForDrop ),
		bits_COND_NEW_ENEMY			|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE,
		0,
		"slInitWaitForDrop"
	},
};

//this just skips to the LAND schedule instead.
Task_t	tlWaitForDrop[] =
{
	{ TASK_SET_FAIL_SCHEDULE, (float)SCHED_TOAD_LAND},
	//{ TASK_STOP_MOVING,				0				},
	{ TASK_TOAD_WAIT_FOR_DROP,	0				},
	{ TASK_SET_SCHEDULE, (float)SCHED_TOAD_LAND },
	//{ TASK_SET_ACTIVITY,			(float)ACT_IDLE	},
	//{ TASK_WAIT_PVS,				0				},
};
Schedule_t	slWaitForDrop[] =
{
	{
		tlWaitForDrop,
		ARRAYSIZE ( tlWaitForDrop ),
		bits_COND_NEW_ENEMY			|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE,
		0,
		"slWaitForDrop"
	},
};

Task_t	tlToadInitFallSlide[] =
{
	{ TASK_TOAD_WAIT_FOR_SLIDE,				0				},
	{ TASK_SET_SCHEDULE,			(float)SCHED_TOAD_LAND	},
	//{ TASK_WAIT_PVS,				0				},
};
Schedule_t	slToadInitFallSlide[] =
{
	{
		tlToadInitFallSlide,
		ARRAYSIZE ( tlToadInitFallSlide ),
		//bits_COND_NEW_ENEMY			|
		//bits_COND_LIGHT_DAMAGE		|
		//bits_COND_HEAVY_DAMAGE,
		0,
		0,
		"slToadInitFallSlide"
	},
};


Task_t	tlToadLand[] =
{
	{ TASK_TOAD_LAND_START,			0				},
	//{ TASK_WAIT_PVS,				0				},
};
Schedule_t	slToadLand[] =
{
	{
		tlToadLand,
		ARRAYSIZE ( tlToadLand ),
		//bits_COND_NEW_ENEMY			|
		//bits_COND_LIGHT_DAMAGE		|
		//bits_COND_HEAVY_DAMAGE,
		0,
		0,
		"slToadLand"
	},
};


Task_t	tlMovePossible[] =
{
	{ TASK_STOP_MOVING,				0				},
	{ TASK_TOAD_CHECKSEEKPOSSIBLE,			0		},
	{ TASK_FACE_IDEAL,				(float)0				},
	{ TASK_WALK_PATH,				(float)0				},
	{ TASK_TOAD_HOPPOSSIBLE,			0			},
	{ TASK_WAIT_FOR_MOVEMENT_TIMED, 9},

	//{ TASK_SET_ACTIVITY,			(float)ACT_IDLE	},
	//{ TASK_WAIT_PVS,				0				},
};

Schedule_t	slMovePossible[] =
{
	{
		tlMovePossible,
		ARRAYSIZE ( tlMovePossible ),
		bits_COND_NEW_ENEMY			|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE,
		0,
		"slMovePossible"
	},
};


//NOTE:  STARTED AS A COPY OF slTakeCoverFromEnemy
/////////////////////////////////////////////////////////////////////////////////////////////
Task_t	tlToadRunAway[] =
{
	{ TASK_SET_FAIL_SCHEDULE,		(float)SCHED_TOAD_PLAY_DEAD},
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_WAIT,					(float)0.0					},
	{ TASK_FIND_COVER_FROM_ENEMY,	(float)0					},
	//MODDD - is that okay?
	{ TASK_FACE_IDEAL,				(float)0},
	//
	{ TASK_RUN_PATH,				(float)0					},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0					},
//	{ TASK_REMEMBER,				(float)bits_MEMORY_INCOVER	},
//	{ TASK_TURN_LEFT,				(float)179					},
//	{ TASK_FACE_ENEMY,				(float)0					},
//	{ TASK_WAIT,					(float)0					},
};

Schedule_t	slToadRunAway[] =
{
	{ 
		tlToadRunAway,
		ARRAYSIZE ( tlToadRunAway ), 
		bits_COND_NEW_ENEMY,
		bits_COND_HEAVY_DAMAGE,
		"slToadRunAway"
	},
};

/////////////////////////////////////////////////////////////////////////////////////////////

Task_t	tlToadIdleWait[] =
{
	{ TASK_STOP_MOVING,				0				},
	{ TASK_TOAD_IDLEWAITRANDOM,			0		},
	{ TASK_SET_SCHEDULE, (float)SCHED_RANDOMWANDER },

	//{ TASK_SET_ACTIVITY,			(float)ACT_IDLE	},
	//{ TASK_WAIT_PVS,				0				},
};

Schedule_t	slToadIdleWait[] =
{
	{
		tlToadIdleWait,
		ARRAYSIZE ( tlToadIdleWait ),
		//MODDD - new
		bits_COND_SEE_ENEMY			|

		bits_COND_NEW_ENEMY		|
		bits_COND_SEE_FEAR		|  //these ok?
		bits_COND_SEE_DISLIKE		|
		bits_COND_SEE_HATE		|

		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE      |
		bits_COND_HEAR_SOUND,
		bits_SOUND_COMBAT		|// sound flags
		bits_SOUND_WORLD		|
		bits_SOUND_PLAYER,
		"slToadIdleWait"
	},
};


Task_t	tlToadPlayDead[] =
{
	{ TASK_STOP_MOVING,				0				},
	{ TASK_TOAD_PLAYDEAD,			0		},
	{ TASK_WAIT_FOR_SEQUENCEFINISH, 0},
	{ TASK_SET_SCHEDULE, (float)SCHED_TOAD_PLAYDEAD_IDLE},
};

Schedule_t	slToadPlayDead[] =
{
	{
		tlToadPlayDead,
		ARRAYSIZE ( tlToadPlayDead ),
		//bits_COND_NEW_ENEMY			|
		//bits_COND_LIGHT_DAMAGE		|
		//bits_COND_HEAVY_DAMAGE,
		0,  //interrupt mask. Don't let the "playDead", flipping upside down be interruptable. Or at least wait until we've flipped before deciding to unflip.
		0,
		"slToadPlayDead"
	},
};


Task_t	tlToadPlayDeadIdle[] =
{
	{ TASK_STOP_MOVING,				0				},
	{ TASK_TOAD_PLAYDEAD_IDLE,			0		},
};

Schedule_t	slToadPlayDeadIdle[] =
{
	{
		tlToadPlayDeadIdle,
		ARRAYSIZE ( tlToadPlayDeadIdle ),
		//bits_COND_NEW_ENEMY			|
		//bits_COND_LIGHT_DAMAGE		|
		//bits_COND_HEAVY_DAMAGE,
		0,  //for the interrupt mask, just let "takeDamage" decide if we're going to interrupt to do UNPLAY_DEAD_URGENT.
		0,
		"slToadPlayDeadIdle"
	},
};


Task_t	tlToadUnPlayDeadUrgent[] =
{
	{ TASK_STOP_MOVING,				0				},
	{ TASK_TOAD_UNPLAYDEAD_URGENT,			0		},
	{ TASK_WAIT_FOR_SEQUENCEFINISH, 0},
	{ TASK_SET_SCHEDULE, (float)SCHED_TOAD_RUNAWAY},
};

Schedule_t	slToadUnPlayDeadUrgent[] =
{
	{
		tlToadUnPlayDeadUrgent,
		ARRAYSIZE ( tlToadUnPlayDeadUrgent ),
		//bits_COND_NEW_ENEMY			|
		//bits_COND_LIGHT_DAMAGE		|
		//bits_COND_HEAVY_DAMAGE,
		0,  //for the interrupt mask, just let "takeDamage" decide if we're going to interrupt to do UNPLAY_DEAD_URGENT.
		0,
		"slToadUnPlayDeadUrgent"
	},
};


Task_t	tlToadUnPlayDead[] =
{
	{ TASK_STOP_MOVING,				0				},
	{ TASK_TOAD_UNPLAYDEAD,			0		},
	{ TASK_WAIT_FOR_SEQUENCEFINISH, 0},
};

Schedule_t	slToadUnPlayDead[] =
{
	{
		tlToadUnPlayDead,
		ARRAYSIZE ( tlToadUnPlayDead ),
		//bits_COND_NEW_ENEMY			|
		//bits_COND_LIGHT_DAMAGE		|
		//bits_COND_HEAVY_DAMAGE,
		0,  //for the interrupt mask, just let "takeDamage" decide if we're going to interrupt to do UNPLAY_DEAD_URGENT.
		0,
		"slToadUnPlayDead"
	},
};


//=========================================================
//	Wake Schedules
//=========================================================
Task_t tlToadWakeAngry1[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_SOUND_WAKE,			(float)0	},
	{ TASK_FACE_IDEAL,			(float)0	},
};

Schedule_t slToadWakeAngry[] =
{
	{
		tlToadWakeAngry1,
		ARRAYSIZE ( tlToadWakeAngry1 ),
		bits_COND_SEE_ENEMY |
		bits_COND_NEW_ENEMY		|
		bits_COND_SEE_FEAR		|  //these ok?
		bits_COND_SEE_DISLIKE		|
		bits_COND_SEE_HATE |
		bits_COND_HEAVY_DAMAGE,
		0,
		"slToadWakeAngry"
	}
};

//=========================================================
// AlertFace Schedules
//=========================================================
Task_t	tlToadAlertFace1[] =
{
	{ TASK_STOP_MOVING,				0				},
	{ TASK_SET_ACTIVITY,			(float)ACT_IDLE	},
	{ TASK_FACE_IDEAL,				(float)0		},
};

Schedule_t	slToadAlertFace[] =
{
	{ 
		tlToadAlertFace1,
		ARRAYSIZE ( tlToadAlertFace1 ),
		//NEW!
		bits_COND_SEE_ENEMY     |

		bits_COND_NEW_ENEMY		|
		bits_COND_SEE_FEAR		|  //these ok?
		bits_COND_SEE_DISLIKE		|
		bits_COND_SEE_HATE		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_PROVOKED,
		0,
		"slToadAlertFace"
	},
};

//=========================================================
// AlertSmallFlinch Schedule - shot, but didn't see attacker,
// flinch then face
//=========================================================
Task_t	tlToadAlertSmallFlinch[] =
{
	{ TASK_STOP_MOVING,				0						},
	{ TASK_REMEMBER,				(float)bits_MEMORY_FLINCHED },
	{ TASK_SMALL_FLINCH,			(float)0				},
	{ TASK_SET_SCHEDULE,            (float)SCHED_TOAD_RUNAWAY },
	//{ TASK_SET_SCHEDULE,			(float)SCHED_TOAD_ALERT_FACE	},   no need to look, start running soon.
};

Schedule_t	slToadAlertSmallFlinch[] =
{
	{ 
		tlToadAlertSmallFlinch,
		ARRAYSIZE ( tlToadAlertSmallFlinch ),
		//0,
		//MODDD
		bits_COND_SEE_ENEMY,
		0,
		"slToadAlertSmallFlinch"
	},
};


Task_t	tlToadAlertBigFlinch[] =
{
	{ TASK_STOP_MOVING,				0						},
	{ TASK_REMEMBER,				(float)bits_MEMORY_FLINCHED },
	{ TASK_BIG_FLINCH,			(float)0				},
	{ TASK_SET_SCHEDULE,            (float)SCHED_TOAD_RUNAWAY },
	//{ TASK_SET_SCHEDULE,			(float)SCHED_TOAD_ALERT_FACE	},   no need to look, start running soon.
};

Schedule_t	slToadAlertBigFlinch[] =
{
	{ 
		tlToadAlertBigFlinch,
		ARRAYSIZE ( tlToadAlertBigFlinch ),
		0,
		0,
		"slToadAlertBigFlinch"
	},
};



//=========================================================
// AlertIdle Schedules
//=========================================================
Task_t	tlToadAlertStand1[] =
{
	{ TASK_STOP_MOVING,			0						 },
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE			 },
	{ TASK_WAIT,				(float)2				 },
	{ TASK_SUGGEST_STATE,		(float)MONSTERSTATE_IDLE },
};

Schedule_t	slToadAlertStand[] =
{
	{ 
		tlToadAlertStand1,
		ARRAYSIZE ( tlToadAlertStand1 ), 
		bits_COND_NEW_ENEMY				|
		bits_COND_SEE_ENEMY				|
		bits_COND_SEE_FEAR				|
		bits_COND_LIGHT_DAMAGE			|
		bits_COND_HEAVY_DAMAGE			|
		bits_COND_PROVOKED				|
		bits_COND_SMELL					|
		bits_COND_SMELL_FOOD			|
		bits_COND_HEAR_SOUND,

		bits_SOUND_COMBAT		|// sound flags
		bits_SOUND_WORLD		|
		bits_SOUND_PLAYER		|
		bits_SOUND_DANGER		|

		bits_SOUND_MEAT			|// scent flags
		bits_SOUND_CARCASS		|
		bits_SOUND_GARBAGE,
		"slToadAlertStand"
	},
};



DEFINE_CUSTOM_SCHEDULES( CChumToad )
{
	slInitWaitForDrop,
	slWaitForDrop,
	slToadInitFallSlide,
	slToadLand,
	//slMoveShort,
	slMovePossible,
	slToadRunAway,
	slToadIdleWait,
	slToadPlayDead,
	slToadPlayDeadIdle,
	slToadUnPlayDeadUrgent,
	slToadUnPlayDead,
	slToadWakeAngry,
	slToadAlertFace,
	slToadAlertSmallFlinch,
	slToadAlertBigFlinch,
	slToadAlertStand,

};

IMPLEMENT_CUSTOM_SCHEDULES( CChumToad, CBaseMonster );




const char* CChumToad::pDeathSounds[] =
{
	"chumtoad/cht_croak_short.wav",
};
const char* CChumToad::pAlertSounds[] =
{
	"chumtoad/cht_croak_short.wav",
	"chumtoad/cht_croak_medium.wav",
};
const char* CChumToad::pPainSounds[] =
{
	"chumtoad/cht_croak_short.wav",
	"chumtoad/cht_croak_medium.wav",
};

const char* CChumToad::pIdleSounds[] =
{
	"chumtoad/cht_croak_medium.wav",
	"chumtoad/cht_croak_long.wav",
};



CChumToad::CChumToad(void){
	testTimer = -1;
	m_hEntitySittingOn = NULL;
	playDeadSuccessful = FALSE;
	playDeadSendoffTimer = -1;
	//vecLastTryLength = -1;
	//vecLastTrySuccess = FALSE;
	//initFall = TRUE;
	//generalFall = TRUE;
	//MODDD NOTE - is this ok in the constructor?
	forceStopInitFallTimer = gpGlobals->time + 0.7;
	landTimer = -1;
	//save???
	stopHopDelay = -1;
	delayTimer = -1;
	passiveCroakDelay = -1;
	toadPlayDeadTimer = -1;
	toadPlayDeadAnimationTimer = -1;
	playDeadForbiddenTimer = -1;
	panicTimer = -1;
	//necessary?
	m_iMyClass = 0;
	playerFriend = FALSE;
	playerAllyFriend = FALSE;

}//END OF CChumToad constructor


BOOL CChumToad::usesSoundSentenceSave(void){
	return FALSE;
}




TYPEDESCRIPTION	CChumToad::m_SaveData[] = 
{

	DEFINE_FIELD( CChumToad, m_hOwner, FIELD_EHANDLE ),
	//DEFINE_FIELD( CChumToad, forceStopInitFallTimer, FIELD_TIME ),
	DEFINE_FIELD( CChumToad, initFall, FIELD_BOOLEAN ),
	DEFINE_FIELD( CChumToad, generalFall, FIELD_BOOLEAN ),
	
	//MODDD TODO - save other ehandle things? scientist NPC to heal? islave NPC to revive? mr. friendly player reference, player mr. friendly reference? etc.
	DEFINE_FIELD( CChumToad, m_hEntitySittingOn, FIELD_EHANDLE ),
	DEFINE_FIELD( CChumToad, playerFriend, FIELD_BOOLEAN ),
	DEFINE_FIELD( CChumToad, playerAllyFriend, FIELD_BOOLEAN ),
	

	
	
};

IMPLEMENT_SAVERESTORE( CChumToad, CBaseMonster );



void CChumToad::SetYawSpeed( void ){
	int ys;

	//I'm fast!
	ys = 200;
	pev->yaw_speed = ys;
	return;
}//END OF SetYawSpeed(...)




//No need seen, yet...
BOOL CChumToad::forceIdleFrameReset(void){

	return FALSE;
}


BOOL CChumToad::usesAdvancedAnimSystem(void){
	return TRUE;
}



BOOL CChumToad::getMonsterBlockIdleAutoUpdate(){
	//We are simple.  Don't allow auto idles!
	//EXPERIMENT: allow again.
	//return FALSE;


	//easyForcePrintLine("DO YOU???? %.2f", toadPlayDeadTimer);
	if(this->toadPlayDeadTimer != -1){
		//playing dead? Don't let this system interfere.
		return TRUE;
	}


	return FALSE;
}//END OF getMonsterBlockIdleAutoUpdate(...)



extern int global_useSentenceSave;
void CChumToad::Precache( void )
{
	PRECACHE_MODEL("models/chumtoad.mdl");
	//nevermind this, see "precacheAll" in util.cpp for more info.
	//global_useSentenceSave = TRUE;
	
	
	// wait what?  Why precache those?
	//PRECACHE_SOUND("chumtoad/cht_throw1.wav");
	//PRECACHE_SOUND("chumtoad/cht_throw2.wav");
	
	PRECACHE_SOUND("chumtoad/cht_croak_short.wav");
	PRECACHE_SOUND("chumtoad/cht_croak_medium.wav");
	PRECACHE_SOUND("chumtoad/cht_croak_long.wav");
	
	//global_useSentenceSave = FALSE;
}


void CChumToad::setModel(void){
	CChumToad::setModel(NULL);
}
void CChumToad::setModel(const char* m){
	CBaseMonster::setModel(m);
	
	if(numberOfEyeSkins == -1){
		//never loaded numberOfSkins? Do so.
		numberOfEyeSkins = getNumberOfSkins();
		if(numberOfEyeSkins == 0){
			easyPrintLine( "WARNING: Chumtoad (NPC) skin count is 0, error! Check chumtoad.mdl for multiple skins. If it has them, please report this.  Forcing default of 3...");
			numberOfEyeSkins = 3;
		}else if(numberOfEyeSkins != 3){
			easyPrintLine( "WARNING: Chumtoad (NPC) skin count is %d, not 3. If chumtoad.mdl does have 3 skins, please report this.", numberOfEyeSkins);
			if(numberOfEyeSkins < 1) numberOfEyeSkins = 1; //safety.
		}
	}

}//END OF setModel


void CChumToad::Spawn( void )
{
	previousZ = pev->origin.z;
	forceStopInitFallTimer = gpGlobals->time + 0.6;

	Precache( );
	// motor

	pev->classname = MAKE_STRING("monster_chumtoad");

	//assume falling...
	//pev->movetype = MOVETYPE_BOUNCE;
	//pev->movetype = MOVETYPE_TOSS;
	//pev->movetype = MOVETYPE_STEP;

	//pev->solid = SOLID_BBOX;
	//pev->solid = SOLID_SLIDEBOX;


	setModel("models/chumtoad.mdl");

	//unfortunately, 8.4 is a little too short Z-wise. lots of melee hits miss it.
	//head crab's height is 24.
	//UTIL_SetSize(pev, Vector( -8.3, -8.3, 0), Vector(8.3, 8.3, 8.4));
	
	//OH YEEEAAAHHHHHHHH.
	/*
	UTIL_SetSize(pev, Vector( -5.3, -5.3, 0), Vector(5.3, 5.3, 12));
	UTIL_SetOrigin( pev, pev->origin );
	*/
	
	UTIL_SetSize(pev, Vector( -4, -4, 0), Vector(4, 4, 8));
	//SetTouch( &CSqueakGrenade::SuperBounceTouch );

	//NO MORE CUSTOM THINK.  USE  MonsterThink!!!
	//////SetThink( &CChumToad::ChumToadThink );
	//////pev->nextthink = gpGlobals->time + 0.1;

	SetTouch(&CChumToad::ChumToadTouch );

	//m_flNextHunt = gpGlobals->time + 1E6;
	//???

	pev->flags |= FL_MONSTER;
	pev->takedamage		= DAMAGE_AIM;

	pev->health = gSkillData.chumtoadHealth;
	
	//pev->gravity		= 0.5;
	//pev->friction		= 0.5;
	pev->gravity		= 0.2;
	pev->friction		= 0.0;
	pev->dmg = 0;  //I am harmless.
	//pev->dmg = gSkillData.snarkDmgPop;

	//m_flDie = gpGlobals->time + SQUEEK_DETONATE_DELAY;
	
	//SetBits(pev->spawnflags, SF_MONSTER_FALL_TO_GROUND);
	//return;
	if ( pev->owner )
		m_hOwner = Instance( pev->owner );

	if( (m_hOwner != NULL) && m_hOwner->IsPlayer()){
		//I was thrown by the player.  Don't be scared by them.
		playerFriend = TRUE;
		playerAllyFriend = TRUE;
	}
	//Don't use this system! It bad!
	pev->owner = NULL;

	//m_flNextBounceSoundTime = gpGlobals->time;// reset each time a snark is spawned.
	//NOTICE: you need "ResetSequenceInfo( )" normally after setting   pev->sequence   manually like this.
	
	m_flFramerateSuggestion = 1;
	
	////Is forcing the sequence this early ok?
	//pev->sequence = CHUMTOAD_IDLE1;
	//ResetSequenceInfo( );
	////...No. Apparently no.   

	//until it's done falling?
	
	//pev->solid			= SOLID_SLIDEBOX;
	//pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_GREEN;
	pev->effects		= 0;
	//pev->health			= gSkillData.headcrabHealth;
	pev->view_ofs		= Vector ( 0, 0, 6.8 );// position of the eyes relative to monster's origin.
	pev->yaw_speed		= 5;//!!! should we put this in the monster's changeanim function since turn rates may vary with state/anim?
	m_flFieldOfView = CHUMTOAD_NORMAL_FOV;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState		= MONSTERSTATE_NONE;
	

	EASY_CVAR_PRINTIF_PRE(chumtoadPrintout, easyPrintLine("Chumtoad SPAWN. ON GROUND? %d", ((pev->flags & FL_ONGROUND) != 0) ));

	
	m_MonsterState		= MONSTERSTATE_NONE;
	//m_afCapability		= bits_CAP_DOORS_GROUP;
	m_afCapability		= 0;   //Zilch!
	
	
	//force the monster not to drop to the ground. We're throwing it.
	//SetBits(pev->spawnflags, SF_MONSTER_FALL_TO_GROUND);
	//...yes, oddly enough this flag must be set to NOT fall to the ground soon after MonsterInit (in the "StartMonster" method, called with a slight delay)


	//This sets "pev->max_health" to the current value of "pev->health", making pev->health's value apply to both before calling MonsterInit().
	
	//If not on the ground, and if falling to the ground (this spawnflag means the opposite of what it says...)
	MonsterInit();

	// MUST happen after MonsterInit to override the default use choice, "MonsterUse".
	SetUse(&CChumToad::PickupUse);

	if(pev->spawnflags & SF_MONSTER_THROWN){
		pev->spawnflags |= SF_MONSTER_FALL_TO_GROUND;  //thrown never snaps to the ground.
	}


	if( !(pev->flags & FL_ONGROUND) && (pev->spawnflags & SF_MONSTER_FALL_TO_GROUND) ){
		pev->solid = SOLID_SLIDEBOX;
		pev->movetype = MOVETYPE_TOSS;
		this->ChangeSchedule(slInitWaitForDrop);
	}else{
		//??????		//what do if starting on the ground?  (probably won't happen though)
		pev->solid = SOLID_SLIDEBOX;
		//Land() sets movetype to MOVETYPE_STEP.
		Land();
		this->ChangeSchedule(slToadIdleWait);
		
	}
	
	//this->SetState(MONSTERSTATE_IDLE);
	m_MonsterState = MONSTERSTATE_IDLE;
	m_IdealMonsterState = MONSTERSTATE_IDLE;


	//ON A LONG LIST OF THE BILLIONS OF THINGS THAT GO COMPLETELY UNSTATED:
	//If a player tries to do direct damage (gunfire) to an entity that has "pev->owner" set to that player, it is ignored (passes through, no reaction).
	//The Snark avoids this (is nearly always targetable by the player) by quickly removing "pev->owner" after spawning, and keeping it saved to "m_hOwner",
	//a variable unrelated to the engine itself. That way, anything attacked by the snark still knows the player (m_hOwner) is what spawned the Snark, but 
	//the engine doesn't try to make the snark immune to damage ("pev->owner" connection removed).

	//Do the same here. Make "pev->owner" null when the chumtoad lands (or if it starts on the ground for some reason).

	
	
	//pev->solid			= SOLID_SLIDEBOX;
	//pev->movetype		= MOVETYPE_STEP;



	//easyForcePrintLine("AW SON %d", (FBitSet( pev->spawnflags, SF_MONSTER_FALL_TO_GROUND )) );
	
}//END OF Spawn(...);

#if REMOVE_ORIGINAL_NAMES != 1
	LINK_ENTITY_TO_CLASS( monster_chumtoad, CChumToad);
#endif

#if EXTRA_NAMES > 0
	LINK_ENTITY_TO_CLASS( chumtoad, CChumToad);
	//no extras.
#endif



/*
// above used to refer to 'CChumToadRespawnable' instead
#if REMOVE_ORIGINAL_NAMES != 1
	LINK_ENTITY_TO_CLASS(monster_chumtoad_norespawn, CChumToad);
#endif

#if EXTRA_NAMES > 0
	LINK_ENTITY_TO_CLASS(chumtoad_norespawn, CChumToad);
	//no extras.
#endif
*/

void CChumToad::ChumToadTouch( CBaseEntity *pOther ){

	if (pOther != NULL && pOther->IsPlayer() && FBitSet(pOther->pev->flags, FL_DUCKING)) {
		// if touched by a player that is ducking, go into the inventory.

		PickupUse(pOther, pOther, USE_SET, 1.0f);
		return;
	}



	// NEVERMIND FURTHER.
	return;

	/*
	if(
		pOther != NULL &&
		!(pOther->IsWorld()) && pOther != NULL &&
		!(pev->flags & FL_ONGROUND) &&
		//if already on top of something and this touch is for that same thing, this isn't going to help.
		(m_hEntitySittingOn==NULL || (pOther->edict() != m_hEntitySittingOn->edict())  )
	){
		//If not on the ground and touching an entity other than the map, need to know if what we just touched is immediately below.
		//If it is an entity (assume it can move), we need to do checks to see if the entity moves out from underneath us in the future
		//to "know" to fall. Yep.

		//TODO - routine checks again.
		CBaseEntity* entityBelow = getEntityBelow();

		if( entityBelow != NULL && entityBelow->edict() == pOther->edict() ){
			//is this non-world entity below the same one we touched? if so we fell on this touched entity.
			m_hEntitySittingOn = entityBelow;
		}

	}//END OF if(!(pev->flags & FL_ONGROUND))
	*/
}//END OF ChumToadTouch


/*
void retroGive(CBasePlayer* argPlayer, const char* pszName)
{
	edict_t* pent;

	int istr = MAKE_STRING(pszName);

	pent = CREATE_NAMED_ENTITY(istr);
	if (FNullEnt(pent))
	{
		ALERT(at_console, "NULL Ent in GiveNamedItem!\n");
		return;
	}
	VARS(pent)->origin = argPlayer->pev->origin;
	pent->v.spawnflags |= (SF_NORESPAWN | SF_PICKUP_NOREPLACE);

	DispatchSpawn(pent);
	DispatchTouch(pent, ENT(argPlayer->pev));
}
*/

// Based off of dlls/weapons.cpp's "DefaultTouch", but for the player calling "use" on me instead.
void CChumToad::PickupUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) {
	// if it's not a player, ignore
	if (!pActivator->IsPlayer())
		return;

	CBasePlayer* pPlayer = (CBasePlayer*)pActivator;

	// No, you can't pick up dead frogs you sick fuck
	if (!IsAlive()) {
		return;
	}

	// Can we even hold this chumtoad?
	if (!g_pGameRules->CanHaveAmmo(pPlayer, "Chum Toads", CHUMTOAD_MAX_CARRY))
	{
		// game rules say I can't have any more of this ammo type.
		return;
	}

	//pPlayer->GiveNamedItem("weapon_chumtoad");
	edict_t* thing = pPlayer->GiveNamedItem("weapon_chumtoad", SF_PICKUP_NOREPLACE, pPlayer->pev->origin);
	if (thing != NULL) {
		CBasePlayerWeapon* generatedRef = static_cast<CBasePlayerWeapon*>(CBaseEntity::Instance(thing));

		if (generatedRef != NULL) {
			//generatedRef->setModel("");
			generatedRef->pev->effects = EF_NODRAW;
			generatedRef->pev->solid = SOLID_NOT;
			generatedRef->m_iDefaultAmmo = 1;
		}

		//Because this is the default, sending to the origin, call "DispatchTouch" too.
		DispatchTouch(thing, ENT(pPlayer->pev));
		
		// Can this be respawned?
		CBaseMonster::CheckRespawn();

		// HACK.  Don't allow me to be respawend multiple times in the same place, MEMORY_KILLED is checked.
		Remember(bits_MEMORY_KILLED);
		UTIL_Remove(this);
	}


	//CBaseEntity* generated = CBaseEntity::Create(pickupNameTest, pev->origin, pev->angles);
	

	//retroGive(pPlayer, "weapon_chumtoad");


	/*
	int giveResult = pActivator->GiveAmmo(1, "Chum Toads", CHUMTOAD_MAX_CARRY);
	if (giveResult == -1) {
		//assume I got rejected, no need to do anything.
	}
	else {
		//Accepted?  Remove me.
		UTIL_Remove(this);
	}
	*/
}//END OF PickupUse


void CChumToad::SetActivity ( Activity NewActivity )
{

	CBaseMonster::SetActivity(NewActivity);
}//END OF SetActivity



void CChumToad::DeathSound( void )
{
	int pitch = 90 + RANDOM_LONG(0,18);

	//if (RANDOM_LONG(0,5) < 2)
	UTIL_PlaySound( ENT(pev), CHAN_VOICE, pDeathSounds[ RANDOM_LONG(0,ARRAYSIZE(pDeathSounds)-1) ], 1.0, ATTN_NORM, 0, pitch );
}


void CChumToad::PainSound( void )
{
	int pitch = 99 + RANDOM_LONG(0,5);

	if (RANDOM_LONG(0,5) < 2)
		UTIL_PlaySound( ENT(pev), CHAN_VOICE, pPainSounds[ RANDOM_LONG(0,ARRAYSIZE(pPainSounds)-1) ], 1.0, ATTN_NORM, 0, pitch );
}

void CChumToad::AlertSound( void )
{
	int pitch = 97 + RANDOM_LONG(0,6);

	UTIL_PlaySound( ENT(pev), CHAN_VOICE, pAlertSounds[ RANDOM_LONG(0,ARRAYSIZE(pAlertSounds)-1) ], 1.0, ATTN_NORM, 0, pitch );
}


//MODDD - the method to check whether this monster can listen to this sound or not.
BOOL canListenHandle_ChumToad_Bait(CBaseEntity* pOther){

	//is it prey? yes or no?

	switch(pOther->Classify()){
		//these can be baited only.
		case CLASS_ALIEN_PREDATOR:
		case CLASS_ALIEN_MONSTER:
		case CLASS_ALIEN_MILITARY:
		case CLASS_HUMAN_MILITARY:
			return TRUE;
		break;
	}//END OF switch(...)


	return FALSE;
}//END OF canListenHandle_ChumToad_Bait(...)



void CChumToad::IdleSound( void )
{
	//can not be playing dead to make the IdleSound.
	if(this->toadPlayDeadTimer == -1 || gpGlobals->time > this->toadPlayDeadTimer){
		int pitch = 93 + RANDOM_LONG(0,8);

		// Play a random idle sound
		UTIL_PlaySound( ENT(pev), CHAN_VOICE, pIdleSounds[ RANDOM_LONG(0,ARRAYSIZE(pIdleSounds)-1) ], 1.0, ATTN_NORM, 0, pitch );
		

		//This is the AI "sound" that sends a signal to attract (TODO?) monsters as intended.
		//TODO: make a unique sound,  bits_SOUDN_BAIT, to attract any enemy to my location?

		//distnace: 460?
		CSound* recentSound = CSoundEnt::InsertSound( bits_SOUND_BAIT, pev->origin + Vector(0, 0, 6), 800, 2.0 );

		//recentSound->setCanListenCheck( canListenHandle_ChumToad_Bait);
		

		//canListenHandle_ChumToad_Bait


		EASY_CVAR_PRINTIF_PRE(chumtoadPrintout, easyPrintLine("DO THEY HEAR ME??") );

	}//END OF playing dead check

}//END OF IdleSound()



//based off of GetSchedule for CBaseMonster in schedule.cpp.
Schedule_t *CChumToad::GetSchedule ( void )
{
	//easyForcePrintLine("YA DONE UP");
	//return &slError[ 0 ];

	EASY_CVAR_PRINTIF_PRE(chumtoadPrintout, easyPrintLine("IM PICKIN A SCHEDULE FOR THIS STATE: %d", m_MonsterState) );
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

			//MODDD - are these needed at all?
			////////////////////////////////////////////////////////////////////////////////////
			if( HasConditions(bits_COND_SEE_ENEMY )){
				//run away? is this force okay?
				return GetScheduleOfType( SCHED_TOAD_RUNAWAY);
			}

			/*
			if ( HasConditions(bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE) )
			{
				return GetScheduleOfType( SCHED_TOAD_RUNAWAY);
			}
			*/
			////////////////////////////////////////////////////////////////////////////////////


			if ( HasConditions ( bits_COND_HEAR_SOUND ) )
			{
				return GetScheduleOfType( SCHED_TOAD_ALERT_FACE );
			}


			//eh.. why not have the option?
			else if ( FRouteClear() )
			{
				// no valid route!
				// Try to do an idle wait + hop + croak later?
				//return GetScheduleOfType( SCHED_IDLE_STAND );
				return GetScheduleOfType(SCHED_TOAD_IDLE_WAIT);
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
				//become normal again?
				m_MonsterState = MONSTERSTATE_IDLE;
				m_IdealMonsterState = MONSTERSTATE_IDLE;
				m_hEnemy = NULL;
				return GetSchedule();

				//return GetScheduleOfType ( SCHED_VICTORY_DANCE );
				//
			}
			
			
			//MODDD - new... nevermind.  Seeing the enemy changes the state to COMBAT anyways, which handles this.
			///////////////////////////////////////////////////////////////
			/*
			if( HasConditions(bits_COND_SEE_ENEMY)){
				//run away? is this force okay?
				return GetScheduleOfType( SCHED_TOAD_RUNAWAY);
			}
			*/

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
					//return GetScheduleOfType( SCHED_ALERT_SMALL_FLINCH );
					return GetScheduleOfType( SCHED_TOAD_ALERT_SMALL_FLINCH );
					// You're cowardly. Don't stop to turn and stare at what damaged you. Just run.
					// we can still do a flinch animation.  Our own flinch schedule(s) will handle running away
					// afterwards immediately.
					//return GetScheduleOfType( SCHED_TAKE_COVER_FROM_ORIGIN );
				}
			}


			else if ( HasConditions ( bits_COND_HEAR_SOUND ) )
			{
				//MODDD TODO - do the usual ALERT_FACE if this toad hasn't had any action in a while.
				
				//If something chasing me is looking at me enough, skip to running away.
				//...oh wait, this state means we dropped the enemy.
				//if(UTIL_IsFacing(m_hEnemy->pev, pev->origin, 0.6)

				// all we care about here is not turning to stare at the player twenty times.
				//Check the best sound and see if it's PLAYER noise.  If so THEN skip looking.
				//Can't be too paranoid or else so much as a friendly walking nearby will scare me.
				//Although any combat noise scaring a chumtoad, regardless of source, does seem fitting too...
				//but not bits_SOUND_WORLD.  Rivers or droning machinery noise, whatever causes that, don't affect me.
				BOOL inPanicMode = (panicTimer != -1 && gpGlobals->time < panicTimer);
				CSound *pSound;
				pSound = PBestSound();

				ASSERT( pSound != NULL );


				if(pSound != NULL){
					if(pSound->m_iType & bits_SOUND_COMBAT){
						//AH. LOUD.  ...maybe later, do a range check to see how close the sound is.
						//direct line of sight to the sound origin?
						//MODDD - TODO.

						if(inPanicMode){
							//If I was recently hurt and I'm in a hurry, keep it up.
							return GetScheduleOfType(SCHED_TOAD_RUNAWAY);
						}

					}
					if(pSound->m_iType & bits_SOUND_PLAYER){
						//depends. is the player our friend?
						if(!playerFriend){
							return GetScheduleOfType(SCHED_TOAD_RUNAWAY);
						}
					}
				}//END OF sound null check

				
				//generic fall thru: face whatever made the sound.
				return GetScheduleOfType( SCHED_TOAD_ALERT_FACE );
				
				
			}
			else
			{
				//NOTICE: this includes changing the monster state to "MONSTERSTATE_IDLE" after 20 seconds.  Base class too, yes.
				return GetScheduleOfType( SCHED_TOAD_ALERT_STAND );
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

				//Chumtoad does not get angry. It gets scared and runs.

				//return GetScheduleOfType ( SCHED_TOAD_WAKE_ANGRY );

				return GetScheduleOfType(SCHED_TOAD_RUNAWAY);

			}
			//MODDD - new.
			else if(HasConditions(bits_COND_HEAVY_DAMAGE)){
				//MODDD - taking heavy damage is more drastic now with its own check.
				//It won't happen often enough to need memory for blocking.
				//And yes, the ALERT version because it says to run away after.  No need for separate ALERT and non-ALERT flinches.
				return GetScheduleOfType(SCHED_TOAD_ALERT_BIG_FLINCH);
			}
			//MODDD - other condition.  If "noFlinchOnHard" is on and the skill is hard, don't flinch from getting hit.
			else if (HasConditions(bits_COND_LIGHT_DAMAGE)  )
			{
				//CHANGE. We're running away whether or not we're due for a flinch animation yet.
				if(!HasMemory( bits_MEMORY_FLINCHED) && !(EASY_CVAR_GET_DEBUGONLY(noFlinchOnHard)==1 && g_iSkillLevel==SKILL_HARD)){
					
					//For the chumtoad, this  has a chance of playing dead?
					//...which will be handled separately by TakeDamage as usual.
					//   We will still do a flinch animation and then run away.
					return GetScheduleOfType( SCHED_TOAD_ALERT_SMALL_FLINCH );
				}else{
					//ordinary run away.
					return GetScheduleOfType(SCHED_TOAD_RUNAWAY);
				}
				

			}
			else if ( !HasConditions(bits_COND_SEE_ENEMY) )
			{
				//MODDD TODO.  Don't see the enemy? uh, ok? Assume we're hidden?
				
				m_MonsterState = MONSTERSTATE_ALERT;
				m_IdealMonsterState = MONSTERSTATE_ALERT;
				m_hEnemy = NULL;
				//change to an... alert? state / schedule instead.  We're okay.
				return GetSchedule();
			}
			else  
			{
				// we can see the enemy

				//yea, right...

				/*
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
				*/

				//else
				{
					//lets go runnin' and runnin'!


					EASY_CVAR_PRINTIF_PRE(chumtoadPrintout, easyPrintLine("ChumToad GetSchedule: Last RUNAWAY sched pick") );
					return GetScheduleOfType(SCHED_TOAD_RUNAWAY);

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

			//if(m_pCine == NULL){
			////	EASY_CVAR_PRINTIF_PRE(panthereyePrintout, easyPrintLine("WARNING: m_pCine IS NULL!"));
			//}

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


	EASY_CVAR_PRINTIF_PRE(chumtoadPrintout, easyPrintLine("ChumToad: GetSchedule FAIL TO PICK SCHEDULE. BROKEN.") );

	return &slError[ 0 ];
}//END OF GetSchedule(...)





Schedule_t* CChumToad::GetScheduleOfType( int Type){


	EASY_CVAR_PRINTIF_PRE(chumtoadPrintout, easyPrintLine("Chumtoad: GetScheduleOfType. Type:%d", Type) );

	switch(Type){

		
		case SCHED_TOAD_INIT_WAIT_FOR_DROP:
			return &slInitWaitForDrop[0];
		break;
		case SCHED_TOAD_WAIT_FOR_DROP:
			return &slWaitForDrop[0];
		break;
		case SCHED_TOAD_INIT_FALLSLIDE:
			return &slToadInitFallSlide[0];
		break;
		case SCHED_TOAD_LAND:
			return &slToadLand[0];
		break;
		//case SCHED_TOAD_MOVESHORT:
		//	return &slMoveShort[0];
		//break;
		case SCHED_TOAD_MOVEPOSSIBLE:
			return &slMovePossible[0];
		break;
		case SCHED_TOAD_RUNAWAY:
			//return &slToadRunAway[0];
			//NOTE: for now, ripping "SCHED_TAKE_COVER_FROM_ENEMY".
			//return &slTakeCoverFromEnemy[0];
			//return CBaseMonster::GetScheduleOfType(SCHED_TAKE_COVER_FROM_ENEMY);
			return &slToadRunAway[0];
		break;

		
		case SCHED_TOAD_IDLE_WAIT:
			return &slToadIdleWait[0];
		break;
		case SCHED_TOAD_PLAY_DEAD:
			return &slToadPlayDead[0];
		break;
		case SCHED_TOAD_PLAYDEAD_IDLE:
			return &slToadPlayDeadIdle[0];
		break;
		case SCHED_TOAD_UNPLAY_DEAD_URGENT:
			return &slToadUnPlayDeadUrgent[0];
		break;
		case SCHED_TOAD_UNPLAY_DEAD:
			return &slToadUnPlayDead[0];
		break;

		
		case SCHED_TOAD_WAKE_ANGRY:
			return &slToadWakeAngry[0];
		break;
		case SCHED_TOAD_ALERT_FACE:
			return &slToadAlertFace[0];
		break;
		case SCHED_TOAD_ALERT_SMALL_FLINCH:
			return &slToadAlertSmallFlinch[0];
		break;
		case SCHED_TOAD_ALERT_BIG_FLINCH:
			return &slToadAlertBigFlinch[0];
		break;
		case SCHED_TOAD_ALERT_STAND:
			return &slToadAlertStand[0];
		break;


		/*
		case SCHED_COMBAT_FACE:

		break;
		*/

		//case SCHED_PANTHEREYE_COVER_FAIL:
		//	return slPantherEyeCoverFail;
		//break;
		
	}//END OF switch(Type)

	//???
	return CBaseMonster::GetScheduleOfType(Type);
}//END OF GetScheduleOfType(...)


void CChumToad::HandleEventQueueEvent(int arg_eventID){

	switch(arg_eventID){
	case 0:
		{
			/*
			easyForcePrintLine("WOA EVENT 0 FIRED!!!");
			//switch to idle playdead anim.
			
			//TEST: does this loop?
			
			this->setAnimationSmart("playdead_idle", 0.9f);
			//...Is this "Loops" force necessary?
			m_fSequenceLoops = TRUE;
			m_flFieldOfView = CHUMTOAD_PLAYDEAD_FOV;
			*/

			//time:   26 / 16   ?

		break;
		}
	case 1:
		{


		break;
		}
	}//END OF switch(...)


}//END OF HandleEventQueueEvent(...)





void CChumToad::setAnimationSmart(const char* arg_animName){
	setAnimationSmart(arg_animName, -999);
}
//modeled moreso after "setSequenceByName".
void CChumToad::setAnimationSmart(const char* arg_animName, float arg_frameRate){
	m_flFieldOfView = CHUMTOAD_NORMAL_FOV;
	CBaseMonster::setAnimationSmart(arg_animName, arg_frameRate);
	
}
void CChumToad::setAnimationSmart(int arg_animIndex, float arg_frameRate){
	m_flFieldOfView = CHUMTOAD_NORMAL_FOV;
	CBaseMonster::setAnimationSmart(arg_animIndex, arg_frameRate);
	
}

void CChumToad::setAnimationSmartAndStop(const char* arg_animName){
	setAnimationSmartAndStop(arg_animName, -999);
}
//modeled moreso after "setSequenceByName"... also stops MOVEMENT, that is (if it wasn't already?)
void CChumToad::setAnimationSmartAndStop(const char* arg_animName, float arg_frameRate){
	m_flFieldOfView = CHUMTOAD_NORMAL_FOV;
	CBaseMonster::setAnimationSmartAndStop(arg_animName, arg_frameRate);
}
void CChumToad::setAnimationSmartAndStop(int arg_animIndex, float arg_frameRate){
	m_flFieldOfView = CHUMTOAD_NORMAL_FOV;
	CBaseMonster::setAnimationSmartAndStop(arg_animIndex, arg_frameRate);
}




GENERATE_TRACEATTACK_IMPLEMENTATION(CChumToad)
{
	GENERATE_TRACEATTACK_PARENT_CALL(CBaseMonster);
}

GENERATE_TAKEDAMAGE_IMPLEMENTATION(CChumToad)
{

	//do this first to see if this killed the monster or not. If it did, we're in no place to play dead.
	int tempDmg = GENERATE_TAKEDAMAGE_PARENT_CALL(CBaseMonster);
	
	if(pevAttacker != NULL){
		CBaseEntity* pEntAttacker = CBaseEntity::Instance(pevAttacker);
		
		if(pEntAttacker != NULL){
			//If one of these used to be my friend, they aren't anymore.  Player attacking scares the frog from barnies too.
			if(pEntAttacker->IsPlayer() ){
				playerFriend = FALSE;
				playerAllyFriend = FALSE;
			}else if(pEntAttacker->Classify() == CLASS_PLAYER_ALLY){
				playerAllyFriend = FALSE;
			}//NOTE: scientists are CLASS_HUMAN_PASSIVE
		}
	}

	//to see if we can play dead or stop playing dead, we must at least be on the ground.  And uh, not dead / dying.
	if(this->m_IdealMonsterState != MONSTERSTATE_DEAD && m_MonsterState != MONSTERSTATE_DEAD && pev->deadflag == DEAD_NO && pev->flags & FL_ONGROUND){

		//IM RUNNIN
		panicTimer = gpGlobals->time + 10;
		if(this->pev->sequence == CHUMTOAD_HOP1){pev->framerate = 1.5f; this->m_flFramerateSuggestion = 1.5f;}

		//get up faster, we gotta go.
		if(this->pev->sequence == CHUMTOAD_PLAYDEAD_END){pev->framerate = 1.8f; this->m_flFramerateSuggestion = 1.8f;};

		if(toadPlayDeadTimer == -1){

			//if this timer hasn't been set, or it has expired, we have a chance at playing dead.
			if(playDeadForbiddenTimer == -1 || gpGlobals->time > playDeadForbiddenTimer){
				//not already playing dead? Taking damage gives a chance to play dead.
				//The less health I have left, the greater the chances of playing dead.

				float healthFract = pev->health / pev->max_health;

				//if below 60%, guarantee playing dead.
				float playDeadChance;

				if(healthFract < 0.6){
					playDeadChance = 1.0f;
				}else{
					playDeadChance = 0.5 + (0.5 * ((healthFract - 0.6) / 0.4) );
				}


				//playDeadForbiddenTimer = gpGlobals->time + 5;
				if(RANDOM_FLOAT(0, 1) <= playDeadChance){
					EASY_CVAR_PRINTIF_PRE(chumtoadPrintout, easyPrintLine("CHUMTOAD: AM I OKAY") );
					//we're good. play dead.
					ChangeSchedule(slToadPlayDead);
					//HACK: say this frame does no damage.
					//return 0;
				}
		
			}

		}else{
			//we're already playing dead but still under attack?  OH DEAR.
			//If in the idle mode, we will react to this.
			if(m_pSchedule == slToadPlayDeadIdle){
				//get up now!
				ChangeSchedule(slToadUnPlayDeadUrgent);
				//HACK: say this frame does no damage.
				//return 0;
			}

		}
	}//END OF on ground check

	//If we take damage, see what the chances of playing dead this time are.
	return tempDmg;
}








void CChumToad::StartTask ( Task_t *pTask )
{

	Vector vec_forward;
	Vector vec_right;
	Vector vec_up;
	TraceResult tempTrace;
	float fracto;
	

	switch ( pTask->iTask )
	{
		case TASK_DIE:
			//start with the eye open.
			pev->skin = 0;
			CBaseMonster::StartTask(pTask);
		break;
		case TASK_TOAD_INIT_WAIT_FOR_DROP:

			initFall = TRUE;

		break;
		case TASK_TOAD_WAIT_FOR_DROP:
			
			//nothing, let runtask handle this

		break;
		case TASK_TOAD_WAIT_FOR_SLIDE:
			
			landTimer = gpGlobals->time + 0.37;

		break;
		case TASK_TOAD_LAND_START:

			if(initFall){
				EASY_CVAR_PRINTIF_PRE(chumtoadPrintout, easyPrintLine("THIS IS INITFALL, THROWN BY PLAYER? %d", (pev->spawnflags & SF_MONSTER_THROWN) ) );
				
				firstLand();

				//if this is an initial fall, start going forwards. If thrown by the player, that is.
				if(pev->spawnflags & SF_MONSTER_THROWN){
					ChangeSchedule(slMovePossible);
				}else{
					TaskComplete();
				}
				
			}else{
				EASY_CVAR_PRINTIF_PRE(chumtoadPrintout, easyPrintLine("THIS IS NOT INITFALL") );
				Land();
				//just pick a new schedule. This should want to hop aimlessly.
				//TaskFail();
				TaskComplete();
			}

		break;
		case TASK_TOAD_CHECKSEEKPOSSIBLE:
		{
			//Vector vecStart;
			//Vector vecEnd;
			Vector vecFinalEnd;
			Vector vecStart;
			Vector vecEnd;
			BOOL success;
			float totalDist;
			float distReg;


			//SEEK POSSIBLE wants to see how far we can go forward, up to so far. Likely several hops.

			//we ARE facing the ideal direction to hop "forward": whatever "this" direction is!
			pev->ideal_yaw = this->pev->angles.y;

			//Pick a spot far in front.
			//UTIL_MakeVectors( pev->angles );
			UTIL_MakeVectorsPrivate(pev->angles, vec_forward, vec_right, vec_up);
			//UTIL_TraceLine(pev->origin, pev->origin + vec_forward * 300, dont_ignore_monsters, dont_ignore_glass, ENT(pev), &tempTrace);
			
			vecStart = pev->origin;
			vecEnd = vecStart + vec_forward * (500 + RANDOM_LONG(-50, 50));

			success = this->CheckLocalMove(vecStart, vecEnd, NULL, TRUE, &distReg) == LOCALMOVE_VALID;
			totalDist = (vecEnd - vecStart).Length();

			if(success){
				//because on success, distReg is likely not written to. Bizarre.
				distReg = totalDist;
			}
			EASY_CVAR_PRINTIF_PRE(chumtoadPrintout, easyPrintLine("task_toad_checkseekpossible. success?:%d distTrav:%.2f", success, distReg) );

			if(distReg > 100){
				fracto = (distReg - 50) / totalDist;
			}else{
				//CANCEL!
				TaskFail();
				return;
			}
			vecFinalEnd = (vecStart * (1 + -fracto) ) + (vecEnd * fracto);
			vecHopDest = vecFinalEnd;
			TaskComplete();
			//debugPoint3 = vecStart * (1 + -fracto) + vecEnd * fracto;


			break;
		}
		case TASK_TOAD_HOPPOSSIBLE:

			if ( MoveToLocation( m_movementActivity, 2, vecHopDest ) )
			{
				TaskComplete();
			}else{
				//How???
				TaskFail();
			}

		break;
		case TASK_TOAD_IDLEWAITRANDOM:

			m_flWaitFinished = gpGlobals->time + RANDOM_LONG(10, 15);

		break;
		case TASK_TOAD_PLAYDEAD:

			//Starting the palydead anim. Start with the eye open.
			pev->skin = 0;

			
			EASY_CVAR_PRINTIF_PRE(chumtoadPrintout, easyPrintLine("Task_toad_playdead") );


			//We need to (or have a chance at) lose enemies when we decide to play dead.
			onPlayDead();
			
			//resetEventQueue();
			this->setAnimationSmart(CHUMTOAD_PLAYDEAD_START, 1.3f);
			//animEventQueuePush(26.0f/16.0f, 0);
			
			toadPlayDeadTimer = gpGlobals->time + 12;

			playDeadSendoffTimer = gpGlobals->time + 14;


			//***IMPORTANT***
			//determine the chance of fooling anyone who gets close.
			this->playDeadSuccessful = (RANDOM_FLOAT(0, 1) < EASY_CVAR_GET_DEBUGONLY(chumtoadPlayDeadFoolChance) );
			
			EASY_CVAR_PRINTIF_PRE(chumtoadPrintout, easyPrintLine("PLAYDEAD SUCCESS? %d", playDeadSuccessful) );
			


			TaskComplete();
		break;
		case TASK_TOAD_PLAYDEAD_IDLE:
			this->setAnimationSmart("playdead_idle", 1.0f);
			toadPlayDeadTimer = gpGlobals->time + 12;
			playDeadSendoffTimer = gpGlobals->time + 14;
			
			//...Is this "Loops" force necessary?
			m_fSequenceLoops = TRUE;
			m_flFieldOfView = CHUMTOAD_PLAYDEAD_FOV;
			
			toadPlayDeadAnimationTimer = gpGlobals->time + ((RANDOM_LONG(1, 4) * (26.0f / 16.0f )) / 1.0f);

			//time:   26 / 16   ?
		break;
		case TASK_TOAD_UNPLAYDEAD_URGENT:

			toadPlayDeadTimer = -1;
			playDeadSendoffTimer = -1;
			panicTimer = gpGlobals->time + 10;
			if(this->pev->sequence == CHUMTOAD_HOP1){pev->framerate = 1.7f; this->m_flFramerateSuggestion = 1.7f;}
			

			//Oh crap they're not buying it - ouch ouch OUCH
			playDeadForbiddenTimer = gpGlobals->time + 5;
			this->setAnimationSmart("playdead_end", 1.7f);
			TaskComplete();
		break;
		case TASK_TOAD_UNPLAYDEAD:

			toadPlayDeadTimer = -1;
			playDeadSendoffTimer = -1;

			//No enemies and I'm bored. Getting back up to be idle.
			playDeadForbiddenTimer = gpGlobals->time + 5;
			this->setAnimationSmart("playdead_end", 0.9f);
			TaskComplete();
		break;

		default:
			CBaseMonster::StartTask ( pTask );
		break;
	}//END OF switch(...)

}//END OF StartTask(...)




//(m_pSchedule == slMovePossible || m_pSchedule == slRandomWander)

void CChumToad::RunTask ( Task_t *pTask ){
	
	EASY_CVAR_PRINTIF_PRE(chumtoadPrintout, easyPrintLine("RunTask: sched:%s task:%d", this->m_pSchedule->pName, pTask->iTask) );
	
	switch ( pTask->iTask ){
		
		//debug!
		case TASK_WAIT_FOR_MOVEMENT:
			//::UTIL_drawPointFrame(vecHopDest, 12, 255, 255, 255);
			CBaseMonster::StartTask ( pTask );

		break;

		case TASK_DIE:

			//if far enough into a death anim, close the eyes a bit.
			if(pev->frame >= 255*0.68 && pev->skin < numberOfEyeSkins - 2){
				pev->skin = numberOfEyeSkins - 2;
			}
			//after enough, close.
			if(pev->frame >= 255*0.86 && pev->skin < numberOfEyeSkins - 1){
				pev->skin = numberOfEyeSkins - 1;
			}

			CBaseMonster::RunTask(pTask);
		break;
		case TASK_TOAD_INIT_WAIT_FOR_DROP:
		case TASK_TOAD_WAIT_FOR_DROP:
			
			//little more time for sliding?

			//TaskComplete();
			//ChangeSchedule(slToadLand);


			if(monsterID == 9){
				int x = 4;
			}
			
			if(m_hEntitySittingOn){
				
				//if sitting on something, check to see if we should fall.
				//it appears this never has a chance to be effective.
				const char* tessst = m_hEntitySittingOn->getClassname();
				//check. are we still sitting on it?
				CBaseEntity* entityBelow = getEntityBelow();
				if(entityBelow == NULL){
					//if nothing is below me, go ahead and fall.
					pev->flags &= ~FL_ONGROUND;
					m_hEntitySittingOn = NULL;
				}else if(entityBelow->IsWorld()){
					//Just move!	
					pev->angles.x = 0;
					pev->angles.z = 0;
					TaskComplete();
					break;
				}

			}else if(pev->flags & FL_ONGROUND){
				//check, what is below me.
				CBaseEntity* entityBelow = getEntityBelow();

				//Apparently this will ignore things directly tied, but never pass through the world.
				//what? who knows but this seems to work. let's just move on.
				if(entityBelow == NULL){
					//nothing.. what? interrupt?
					pev->flags &= ~FL_ONGROUND;
					break;
				}else{
					if(entityBelow->IsWorld()){
						//this is ok.
						m_hEntitySittingOn = NULL;
					}else{
						//on top of this entity?
						m_hEntitySittingOn = entityBelow;
					}
				}
			}


			



			
			//only continue if not sitting on an entity. That messes things up.
			if(pev->flags & FL_ONGROUND && m_hEntitySittingOn == NULL){

				pev->angles.x = 0;
				pev->angles.z = 0;
				TaskComplete();
				//ChangeSchedule(slToadInitFallSlide);
			}

		break;
		case TASK_TOAD_WAIT_FOR_SLIDE:

			
			EASY_CVAR_PRINTIF_PRE(chumtoadPrintout, easyPrintLine("JUST WAITIN TO SLIDE THO OKAY?"));
			if(gpGlobals->time > landTimer){
				EASY_CVAR_PRINTIF_PRE(chumtoadPrintout, easyPrintLine("JUST DONE WAITING OKAY?"));
				TaskComplete();  //done!
			}

		break;
		case TASK_TOAD_LAND_START:

		break;
		//case TASK_TOAD_CHECKSEEKSHORT:
		//
		//break;
		//case TASK_TOAD_HOPSHORT:
		//	
		//break;
		case TASK_TOAD_CHECKSEEKPOSSIBLE:
			
		break;
		case TASK_TOAD_HOPPOSSIBLE:
			//this->TaskComplete();
			//this->TaskFail();

		break;
		case TASK_TOAD_IDLEWAITRANDOM:
			if(gpGlobals->time > m_flWaitFinished){
				TaskComplete();
			}
		break;
		case TASK_TOAD_PLAYDEAD:
			
			/*
			resetEventQueue();
			this->setAnimationSmart("playdead_start", 1.3f);
			animEventQueuePush(26.0f/16.0f, 0);

			toadPlayDeadTimer = gpGlobals->time + 12;
			*/

			//playdead does this too.


		break;
		case TASK_TOAD_PLAYDEAD_IDLE:

			//easyForcePrintLine("IM PLAYIN DEAD BUT DO I SEE THINGS? %d", HasConditions(bits_COND_SEE_ENEMY));


			if(gpGlobals->time < toadPlayDeadAnimationTimer ){
				if(pev->sequence == CHUMTOAD_PLAYDEAD_IDLE){
					//if doing the play-dead animation and...
					if(pev->frame >= 255*0.68 && pev->skin < numberOfEyeSkins - 2){
						pev->skin = numberOfEyeSkins - 2;
					}
					if(pev->frame >= 255*0.86 && pev->skin < numberOfEyeSkins - 1){
						pev->skin = numberOfEyeSkins - 1;
					}
				}
			}


			if(HasConditions(bits_COND_SEE_ENEMY)){
				//NOTICE: should we require that the enemy is looking at this chumtoad to do this too?
				const BOOL isFacingAway = UTIL_IsFacingAway(m_hEnemy->pev, this->pev->origin, 1.3);
				const float distToEnemy = (m_hEnemy->pev->origin - this->pev->origin).Length();

				if(isFacingAway && distToEnemy > 90  ){
					//they won't see us if we get up, it's fine.
				}else{
					//not looking far away enough or too close? keep playing dead.
					//reset the timer.
					toadPlayDeadTimer = gpGlobals->time + 12;
				}

			}

			if(playDeadSendoffTimer == -1 || gpGlobals->time > playDeadSendoffTimer){
				playDeadSendoffTimer = gpGlobals->time + RANDOM_FLOAT(7, 12);
				//send them away!!
				this->playDeadSendMonstersAway();
			}


			EASY_CVAR_PRINTIF_PRE(chumtoadPrintout, easyPrintLine("CHUMTOAD PLAIN DEAD IDLE:::f:%.2f s?:%d pdt:%.2f tim:%.2f", m_flFieldOfView, HasConditions(bits_COND_SEE_ENEMY), toadPlayDeadTimer, gpGlobals->time) );
			
			if(toadPlayDeadAnimationTimer != -1 && gpGlobals->time > toadPlayDeadAnimationTimer){
				//stop animating.
				toadPlayDeadAnimationTimer = -1;
				pev->framerate = 0;
				pev->skin = numberOfEyeSkins - 1;  //close eye just in case.
			}

			if(gpGlobals->time > toadPlayDeadTimer){
				//the timer is up? Change to the less urgent unplay dead schedule.
				TaskComplete();
				ChangeSchedule(slToadUnPlayDead);
				
			}

		break;

		case TASK_TOAD_UNPLAYDEAD_URGENT:

		break;
		case TASK_TOAD_UNPLAYDEAD:

		break;
		case TASK_WAIT_FOR_SEQUENCEFINISH:
			
			
			if(pev->sequence == CHUMTOAD_PLAYDEAD_END){
				//if doing the play-dead animation and...
				if(pev->frame >= 255*0.4 && pev->skin > 1){
					pev->skin = 1;
				}
				if(pev->frame >= 255*0.6 && pev->skin > 0){
					pev->skin = 0;
				}
			}
			
			CBaseMonster::RunTask(pTask);

		break;



		/*
		case TASK_PANTHEREYE_FIND_COVER_FROM_ENEMY:
		{

			if(waitingForNewPath && newPathDelay == -1){

				panthereye_findCoverFromEnemy();
				newPathDelay = gpGlobals->time + newPathDelayDuration;
				waitingForNewPath = FALSE;
			}

			break;
		}
		*/

		default:
			CBaseMonster::RunTask(pTask);
		break;
	}//END OF switch(...)

}//END OF RunTask(...)



float CChumToad::HearingSensitivity(){
	//good hearing.
	return 1.8f;
}

// Players throw me.  I better show up.
BOOL CChumToad::bypassAllowMonstersSpawnCheck(void) {
	return TRUE;
}//END OF bypassAllowMonstersSpawnCheck



void CChumToad::MonsterThink ( void )
{
	//DebugLine_ClearAll();
	
	if(monsterID == 24){
		int x = 34;
	}

	if(pev->deadflag == DEAD_NO){
		//under any of these circumstances, the schedule handles this or does nothing and must stay that way.
		if(this->m_pSchedule != slToadPlayDead && this->m_pSchedule != slToadPlayDeadIdle && pev->sequence != CHUMTOAD_PLAYDEAD_END){
			//typical. Randomly blink.
			if( ( pev->skin == 0 ) && RANDOM_LONG(0,58) == 0)
			{
				pev->skin = max(numberOfEyeSkins - 1, 0);
			}
			else if ( pev->skin > 0 )
			{// already blinking
				pev->skin--;
			}

		}else{	
			//playing dead. Anything special?
			//pev->skin = 0;
			//no, let these schedules handle the eyes.

		}
	}else{
		//deadflags other than DEAD_NO ? anything?
		pev->skin = 0;
	}


	//easyForcePrintLine("WHAT IN FLYING %.2f %.2f %.2f", pev->angles.x, pev->angles.y, pev->angles.z);

	//---testTimer unused.
	if(testTimer != -1 && testTimer <= gpGlobals->time){
		testTimer = -1;
		//pev->origin = pev->origin + Vector(-60, 0, 0);
		pev->origin = pev->origin + Vector(0, 0, 150);
	}

	/*
	if(vecLastTryLength != -1){
		for(int i = 0; i < vecLastTryLength; i++){
			//easyForcePrintLine("ILL END YOUR EXISTENCE %d", vecLastTryLength);
			//UTIL_printVector("WHAT????????", vecLastTry[i]);
			int c_r, c_g, c_b;
			//red by default.
			c_r = 255;
			c_g = 0;
			c_b = 0;
			if(vecLastTrySuccess && i == vecLastTryLength - 1){
				//make it green!
				c_r = 0;
				c_g = 255;
				c_b = 0;
			}
			
			UTIL_drawLineFrame(vecLastTryOrigin, vecLastTry[i], 6, c_r, c_g, c_b);
			UTIL_drawLineFrame(vecLastTry[i] - Vector(0,0,3), vecLastTry[i] + Vector(0,0,3), 8, c_r, c_g, c_b);
		}

		if(vecLastTrySuccess){
			
			UTIL_drawLineFrame(vecLastTryOrigin, vecHopDest, 6, 0, 0, 255);
			UTIL_drawLineFrame(vecHopDest - Vector(0,0,3), vecHopDest + Vector(0,0,3), 8, 0, 0, 255);
		}

	}
	*/

	CBaseMonster::MonsterThink();
}//END OF MonsterThink(...)


void CChumToad::firstLand(){
	Land();
}//END OF firstLand(...)

void CChumToad::Land(){
	
	//no connection anymore. Mostly to let chumtoads be collidable with the player.
	//pev->owner = NULL;
	//NOPE. we change this.


	EASY_CVAR_PRINTIF_PRE(chumtoadPrintout, easyPrintLine("chumtoad: LAND CALLED! First?" ) );
	
	EASY_CVAR_PRINTIF_PRE(chumtoadPrintout, easyPrintLine("I LAND") );
	
	pev->movetype = MOVETYPE_STEP;
	
	pev->solid			= SOLID_SLIDEBOX;


	// clatter if we have an owner (i.e., dropped by someone)
	// don't clatter if the gun is waiting to respawn (if it's waiting, it is invisible!)
	//if ( !FNullEnt( pev->owner ) )
	{
		int pitch = 95 + RANDOM_LONG(0,29);
		UTIL_PlaySound(ENT(pev), CHAN_VOICE, "chumtoad/cht_croak_short.wav", 1, ATTN_NORM, 0, pitch);	
	}

	// lie flat
	pev->angles.x = 0;
	pev->angles.z = 0;
	
	Vector velocityClone = pev->velocity;
	velocityClone.z = 0;
	velocityClone = velocityClone.Normalize();

	//Vector rawVelocityAngle = UTIL_velocityToAngles(pev->velocity);
	//rawVelocityAngle.z = 0;
	//rawVelocityAngle = rawVelocityAngle.Normalize();  //flatways only.

	//Is using the z-less "velocityClone" to slide a bit okay?
	pev->velocity = Vector(0,0,0);
	//Materialize(); 

}//END OF Land(...)


void CChumToad::forwardHop(){
	//hop1 details:   19, 60
	int hops = RANDOM_LONG(12, 17);
	
	m_flFramerateSuggestion = 0.3;
	stopHopDelay = gpGlobals->time + hops * (19.0 / 60.0) / m_flFramerateSuggestion + 0.02f;

	pev->sequence = CHUMTOAD_HOP1;
	ResetSequenceInfo( );

}//END OF forwardHop(...)



//MODD TODO - make the hop "move"!!!
//m_flGroundSpeed
//void CBaseMonster::MoveExecute( CBaseEntity *pTargetEnt, const Vector &vecDir, float flInterval )
//{

void CChumToad::aimlessHop(){
	//rotate in a random direction and hop?

	pev->angles.y = RANDOM_FLOAT(0, 359.99);
	//hop1 details:   19, 60
	int hops = RANDOM_LONG(1, 2);

	m_flFramerateSuggestion = 0.3;
	stopHopDelay = gpGlobals->time + hops * (19.0 / 60.0) / m_flFramerateSuggestion + 0.02f;

	pev->sequence = CHUMTOAD_HOP1;
	ResetSequenceInfo( );


}//END OF forwardHop(...)


void CChumToad::randomDelay(){
	float delayAdd = RANDOM_FLOAT(5, 20);

	delayTimer = gpGlobals->time + delayAdd;

	//croak in here?
	BOOL croakChoice = (delayAdd > 8 && RANDOM_FLOAT(0, 10)  < 4);
	if(croakChoice){
		passiveCroakDelay = gpGlobals->time + RANDOM_FLOAT(2, delayAdd - 5);
	}
}//END OF randomDelay(...)



//MODDD - declared (prototype'd) in cbase.h and basemonster.h, just like "IsAlive".
//This can be used to make enemies treat the chumtoad as "dead" when the chumtoad wants them to.
//BOOL	IsAlive_FromAI( void ) { return (pev->deadflag != DEAD_DEAD); }
BOOL CChumToad::IsAlive_FromAI( CBaseMonster* whoWantsToKnow ){

	if(whoWantsToKnow == NULL){
		//don't know "whoWantsToKnow"? Nothing to try here.
		return CBaseMonster::IsAlive_FromAI(NULL);
	}

	if(whoWantsToKnow->pev == this->pev || this->IRelationship(whoWantsToKnow) == R_AL ){
		// A chumtoad checking itself or other chumtoads? No trickery here.
		return CBaseMonster::IsAlive_FromAI(whoWantsToKnow);
	}

	// Anyone else can be tricked.

	if(this->playDeadFooling(whoWantsToKnow)){
		// playing dead? act like it.
		EASY_CVAR_PRINTIF_PRE(chumtoadPrintout, easyPrintLine("Enemy is chumtoad. It\'s a chumtoad playing dead, but I am fooled!"));
		return FALSE;
	}else{
		// not playing dead? No filter.
		EASY_CVAR_PRINTIF_PRE(chumtoadPrintout, easyPrintLine("Enemey is chumtoad. Not playing dead."));
		return CBaseMonster::IsAlive_FromAI(whoWantsToKnow);
	}

}//END OF IsAlive_FromAI(...)


void CChumToad::ReportAIState(void){

	//call the parent, and add on to that.
	CBaseMonster::ReportAIState();

	easyForcePrintLine("playdead timer:%.2f curtime:%.2f playing dead?:%d success?:%d", toadPlayDeadTimer, gpGlobals->time, (this->toadPlayDeadTimer != -1 && gpGlobals->time < toadPlayDeadTimer), playDeadSuccessful );


}//END OF ReportAIState()


//For now, the chumtoad is no different here.
BOOL CChumToad::isForceHated(CBaseEntity *pBy){
	return (forcedRelationshipWith(pBy) > R_NO );
	//return FALSE;
}


//What is "My" (chumtoad's) relationship with the other thing asking about it (pWith)?
int CChumToad::forcedRelationshipWith(CBaseEntity* pWith){
	
	//TODO: system so that a chumtoad that's fallen (or seen for the first time by a monster?)
	//does a classify check. if it sees the toad as bait (relationship of R_BA), try to get close to it?
	//Either a crude schedule cancel (higher priority should be noted on re-getting an enemy), or
	//check to see who's looking at the chumtoad and make them do some kind of "chase_bait" sechedule,
	//interruptable by taking damage?



	
	if(FClassnameIs(pWith->pev, "monster_alien_grunt")){
		//TODO: make follow?
		return R_BA;
	}

	switch(pWith->Classify()){
	case CLASS_NONE:
		return R_DEFAULT;
	break;
	case CLASS_MACHINE:
		return R_DEFAULT;
	break;
	case CLASS_PLAYER:
		return R_DEFAULT;
	break;
	case CLASS_HUMAN_PASSIVE:
		return R_DEFAULT;
	break;
	case CLASS_HUMAN_MILITARY:
		return R_HT;
	break;
	case CLASS_ALIEN_MILITARY:
		//food?
		return R_NO;
	break;
	case CLASS_ALIEN_PASSIVE:
		return R_DEFAULT;
	break;
	case CLASS_ALIEN_MONSTER:
		//food?
		return R_NO;
	break;
	case CLASS_ALIEN_PREY:
		//my class: other non-toad prey. Does it hate me too?

		if(  FClassnameIs(pWith->pev, "monster_chumtoad" ) == 1){
			//We toads must stick together in this cold and unforgiving world.
			return R_AL;
		}

		return R_BA;
	break;
	case CLASS_ALIEN_PREDATOR:
		//food?
		return R_BA;
	break;
	case CLASS_INSECT:
		return R_DEFAULT;
	break;
	case CLASS_PLAYER_ALLY:
		return R_DEFAULT;
	break;
	case CLASS_PLAYER_BIOWEAPON:
		return R_HT;
	break;
	case CLASS_ALIEN_BIOWEAPON:
		return R_HT;
	break;
	case CLASS_BARNACLE:
		return R_DEFAULT;
	break;
	}//END OF switch(...)
	
	return CBaseMonster::forcedRelationshipWith(pWith);

}//END OF forcedRelationshipWith(...)


int CChumToad::IRelationship ( CBaseEntity *pTarget )
{

	//return R_HT;
	//see Monster.cpp's IRelationship method for the full original table.
	
	//yes, == 0  means  "matches".

	if(pTarget == NULL){
		//what?
		EASY_CVAR_PRINTIF_PRE(chumtoadPrintout, easyPrintLine("null target? DOOP.") );
		return R_HT;  
	}

	EASY_CVAR_PRINTIF_PRE(chumtoadPrintout, easyPrintLine("ChumToad: IRelationship:::%s::%d", STRING(pTarget->pev->classname), FClassnameIs(pTarget->pev, "monster_chumtoad" )) );
	
		//return R_HT;  

	switch(pTarget->Classify()){
	case CLASS_NONE:
		
		//return R_DEFAULT;  ???
	break;
	case CLASS_MACHINE:
		
		//return R_DEFAULT;
		return R_HT;   //uh.. ?
	break;
	case CLASS_HUMAN_MILITARY:
		return R_HT;
	break;

	case CLASS_ALIEN_MILITARY:
		//food?  do I try to run if they get close or look hungry maybe (agrunts)?
		return R_NO;
	break;
	case CLASS_ALIEN_MONSTER:
		//food?
		return R_NO;
	break;

	case CLASS_ALIEN_PASSIVE:
		//return R_DEFAULT;
		
		return R_NO;  //my kind of people... maybe?
	break;
	case CLASS_ALIEN_PREY:
		//my class: other non-toad prey. Does it hate me too?

		if(  FClassnameIs(pTarget->pev, "monster_chumtoad" ) == TRUE){
			//We toads must stick together in this cold and unforgiving world.
			return R_AL;
		}

		//return R_HT;
		return R_NO;
	break;
	case CLASS_ALIEN_PREDATOR:
		//food?
		return R_HT;
	break;
	case CLASS_INSECT:
		//return R_DEFAULT;

		//finally, food for me...  TODO. implement that? eat cockroaches? do toads do that? maybe freaky purple alien ones do.
		return R_NO;
	break;


	//!!!
	case CLASS_HUMAN_PASSIVE:
		//scientists?  never fear them.
		return R_NO;
	break;
	case CLASS_PLAYER:
		if(playerFriend){
			return R_NO;
		}else{
			return R_HT;
		}
	break;
	case CLASS_PLAYER_ALLY:
		if(playerAllyFriend){
			return R_NO;
		}else{
			return R_HT;
		}
	break;

	case CLASS_PLAYER_BIOWEAPON:
		if(playerFriend){
			return R_NO;
		}else{
			return R_HT;
		}
	break;

	


	case CLASS_ALIEN_BIOWEAPON:
		return R_HT;
	break;
	case CLASS_BARNACLE:
		
		//return R_DEFAULT;   //fall thru...
	break;
	}//END OF switch(...)


	return CBaseMonster::IRelationship(pTarget);

}//END OF IRelationship(...)


//MODDD TODO.
//CLASSIFY.  Make it work better for the chumtoad.
//Or is everyone hating the chumtoad okay?
int CChumToad::Classify ( void )
{

	//DISREGARD.  NOW RETURNING MONSTER_ALIEN_PREY all the time.
	return CLASS_ALIEN_PREY;
	/////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////

	int returnedClass = 0;



	if (m_iMyClass != 0){
		//return m_iMyClass; // protect against recursion
		returnedClass = m_iMyClass;
		goto ClassifyChumtoadReturn;
	}

	if (m_hEnemy != NULL)
	{
		m_iMyClass = CLASS_INSECT; // no one cares about it
		//No, make everyone hate me. I'm supposed to be hatable.


		switch( m_hEnemy->Classify( ) )
		{
			case CLASS_PLAYER:
			case CLASS_HUMAN_PASSIVE:
			case CLASS_HUMAN_MILITARY:
				m_iMyClass = 0;
				//return CLASS_ALIEN_MILITARY; // barney's get mad, grunts get mad at it
				returnedClass = CLASS_ALIEN_MILITARY;
				goto ClassifyChumtoadReturn;
			break;
		}
		m_iMyClass = 0;
	}

	//return CLASS_ALIEN_BIOWEAPON;
	returnedClass = CLASS_ALIEN_BIOWEAPON;
	goto ClassifyChumtoadReturn;



ClassifyChumtoadReturn:
	EASY_CVAR_PRINTIF_PRE(chumtoadPrintout, easyPrintLine("chumtoad Classify. CLASS: %s:ID%d is %d", this->getClassnameShort(), this->monsterID, returnedClass) ); 
	return returnedClass;

}


BOOL CChumToad::getForceAllowNewEnemy(CBaseEntity* pOther){
	
	if(!playDeadFooling(pOther)){
		//not playing dead.
		return FALSE;
	}

	//playing dead.
	return TRUE;
}


//Is this chumtoad tricking the other monster?
//If the monster checking is sufficiently far away, it can't tell anyways.
//But up close, it will become hostile if the random chance of success (playDeadSuccessful) failed.
BOOL CChumToad::playDeadFooling(CBaseEntity* whoWantsToKnow){
	BOOL distanceCheck = (this->pev->origin - whoWantsToKnow->pev->origin).Length() > 230;
	return ( (playDeadSuccessful||distanceCheck) && (this->toadPlayDeadTimer != -1 && gpGlobals->time < toadPlayDeadTimer) );
}//END OF playDeadFooling()



int CChumToad::LookupActivityHard(int activity){
	int i = 0;
	m_flFieldOfView = CHUMTOAD_NORMAL_FOV;
	m_flFramerateSuggestion = 1;
	pev->framerate = 1;
	//is this safe?

	/*
	m_flFramerateSuggestion = -1;
	//pev->frame = 6;
	return LookupSequence("get_bug");
	*/

	int iRandChoice = 0;
	int iRandWeightChoice = 0;
	
	char* animChoiceString = NULL;
	int* weightsAbs = NULL;
			
	//pev->framerate = 1;
	int maxRandWeight = 30;

	//any animation events in progress?  Clear it.
	resetEventQueue();

	//EASY_CVAR_PRINTIF_PRE(panthereyePrintout, easyPrintLine("WHUT %d", m_fSequenceFinished));

	//no need for default, just falls back to the normal activity lookup.
	switch(activity){

		case ACT_IDLE:
			iRandChoice = RANDOM_LONG(0, 12);
			if(iRandChoice <= 8){
				return CHUMTOAD_IDLE1;
			}else if(iRandChoice <= 10){
				return CHUMTOAD_IDLE2;
			}else if(iRandChoice <= 12){
				return CHUMTOAD_IDLE3;
			}
		break;
		case ACT_WALK:
		case ACT_RUN:
			m_flFramerateSuggestion = 0.4;


			//this->animFrameStartSuggestion = 18;
			//this->animFrameCutoffSuggestion = 239;
			
			//this->animEventQueuePush(5.0f / 30.0f, 0);

			if(panicTimer != -1 && gpGlobals->time < panicTimer){
				m_flFramerateSuggestion = 1.5f;
			}else if(m_MonsterState == MONSTERSTATE_IDLE){
				//slow.
				m_flFramerateSuggestion = 0.5;
			}else if(m_MonsterState == MONSTERSTATE_ALERT){
				m_flFramerateSuggestion = 0.93;
			}else{
				//assume in a hurry to get out of danger.
				m_flFramerateSuggestion = 1.1f;
			}

			EASY_CVAR_PRINTIF_PRE(chumtoadPrintout, easyPrintLine("What move act?? %d", ((int)activity) ));
			return CHUMTOAD_HOP1;
		break;
		case ACT_SMALL_FLINCH:
			//turn up the speed.
			if(panicTimer != -1 && gpGlobals->time < panicTimer){
				m_flFramerateSuggestion = 1.4f;
			}else if(m_MonsterState == MONSTERSTATE_IDLE){
				//slow.
				m_flFramerateSuggestion = 1.0f;
			}else if(m_MonsterState == MONSTERSTATE_ALERT){
				m_flFramerateSuggestion = 1.0f;
			}else{
				//assume in a hurry to get out of danger.
				m_flFramerateSuggestion = 1.14f;
			}

			return CBaseAnimating::LookupActivity(activity);
		break;
		case ACT_BIG_FLINCH:
			//turn up the speed.
			if(panicTimer != -1 && gpGlobals->time < panicTimer){
				m_flFramerateSuggestion = 1.5f;
			}else if(m_MonsterState == MONSTERSTATE_IDLE){
				//slow.
				m_flFramerateSuggestion = 1.0f;
			}else if(m_MonsterState == MONSTERSTATE_ALERT){
				m_flFramerateSuggestion = 1.1f;
			}else{
				//assume in a hurry to get out of danger.
				m_flFramerateSuggestion = 1.21f;
			}

			return CBaseAnimating::LookupActivity(activity);
		break;

		//MODDD - see notes below about these.
		case ACT_DIEVIOLENT:
			return -1;
		break;
		case ACT_FEAR_DISPLAY:
			return -1;
		break;
		case ACT_STAND:
			return -1;
		break;

	}
	//not handled by above?  try the real deal.
	return CBaseAnimating::LookupActivity(activity);
}


int CChumToad::tryActivitySubstitute(int activity){
	//no need for default, just falls back to the normal activity lookup.
	switch(activity){
		case ACT_IDLE:
			return CBaseAnimating::LookupActivity(activity);
		break;
		case ACT_WALK:
		case ACT_RUN:
			//m_flFramerateSuggestion = global_hassaultMeleeAnimSpeedMulti;
			//this->animFrameStartSuggestion = 18;
			//this->animFrameCutoffSuggestion = 239;
			
			//this->animEventQueuePush(5.0f / 30.0f, 0);
			
			return CHUMTOAD_HOP1;
		break;

		//NOTE - several sequences in the model are linked to activities, but this was already made without those in mind.
		//       Let's force the associations off just to avoid any possible side-effects from seeing them to be safe.
		//       The custom DIEVIOLENT system assumes it's turned off for all entities regardless of the presence of any ACT_DIEVIOLENT
		//       linked activity anyways for safety.  It must be forced on by script here, an implementable method.
		case ACT_DIEVIOLENT:
			return -1;
		break;
		case ACT_FEAR_DISPLAY:
			return -1;
		break;
		case ACT_STAND:
			return -1;
		break;

	}//END OF switch


	//not handled by above?
	return CBaseAnimating::LookupActivity(activity);
}


GENERATE_DEADTAKEDAMAGE_IMPLEMENTATION(CChumToad){
	
	
	return GENERATE_DEADTAKEDAMAGE_PARENT_CALL(CBaseMonster);
}


//parameters: BOOL fGibSpawnsDecal
//returns: none
GENERATE_GIBMONSTER_IMPLEMENTATION(CChumToad){
	GENERATE_GIBMONSTER_PARENT_CALL(CBaseMonster);
}

//parameters: BOOL fGibSpawnsDecal
//returns: BOOL. Did this monster spawn gibs and is safe to stop drawing?
GENERATE_GIBMONSTERGIB_IMPLEMENTATION(CChumToad){
	//replaces parent logic:

	if(CVAR_GET_FLOAT("violence_agibs") != 0)
	{
		CGib::SpawnRandomGibs( pev, 2, GIB_ALIEN_GREEN_ID, fGibSpawnsDecal );	// Throw alien gibs
		return TRUE;
	}

	return FALSE;
}//END OF GibMonsterGib




GENERATE_KILLED_IMPLEMENTATION(CChumToad){
	// no longer "playing" dead.
	toadPlayDeadTimer = -1;

	if (!HasMemory(bits_MEMORY_KILLED)) {
		// Can this be respawned?
		CBaseMonster::CheckRespawn();
	}

	GENERATE_KILLED_PARENT_CALL(CBaseMonster);
}

// Deleted through any other means?  See if this should be respawned
void CChumToad::onDelete(void) {
	if (!HasMemory(bits_MEMORY_KILLED)) {
		// Can this be respawned?   Also, don't allow this again if it already happened.
		Remember(bits_MEMORY_KILLED);
		CBaseMonster::CheckRespawn();
	}
}


//NOTICED: no longer needed. Enemeis naturally see a chumtoad playing dead (successfully) as "dead" unless the play-dead chance failed and they are too close (cover blown when looked at up close).
void CChumToad::onPlayDead(){
	

	//take a scan of enemies nearby and say I am no longer their enemy. 
			//
			//toadPlayDeadTimer


}//END OF onPlayDead()




//If playing dead (successfully), send nearby monsters away (pretending that they lost interest)
//to count, they must be close enough and looking directly at me.
void CChumToad::playDeadSendMonstersAway(){

	EASY_CVAR_PRINTIF_PRE(chumtoadPrintout, easyPrintLine("I TRIED TO SEND THEM AWAY BUT THEY WOULD NOT LISTEN?!") );

	CBaseEntity* pEntityScan = NULL;
	CBaseMonster* testMon = NULL;
	//does UTIL_MonstersInSphere work?
	while ((pEntityScan = UTIL_FindEntityInSphere( pEntityScan, pev->origin, 320 )) != NULL)
	{

		if(pEntityScan->pev != this->pev){

			testMon = pEntityScan->MyMonsterPointer();
			//if(testMon != NULL && testMon->pev != this->pev && ( FClassnameIs(testMon->pev, "monster_scientist") || FClassnameIs(testMon->pev, "monster_barney")  ) ){
		


			if(testMon != NULL){
				int relationshipTest;
				relationshipTest = IRelationship(testMon);

				if(
					//!(::FClassnameIs(testMon->pev, "monster_chumtoad")) &&   //this check is redundant with the relationship check.  Toads are friends.
					//Check.  Do I want to run away from them?  If so, I can make them wander away.
					(relationshipTest > R_NO || relationshipTest == R_FR) &&
					(testMon->m_MonsterState == MONSTERSTATE_IDLE || testMon->m_MonsterState == MONSTERSTATE_ALERT) &&
					(testMon->m_IdealActivity != testMon->m_movementActivity || testMon->m_movementActivity == ACT_IDLE ) &&   //movementActivity of ACT_IDLE is another way of saying stopped.
					UTIL_IsFacing(testMon->pev, pev->origin, 0.4)
			
				){

					testMon->wanderAway( this->pev->origin );
				
				
				}//END OF all the other checks
			}//END OF monster null check
		}//END OF "entity found is not myself" check

	}//END OF while(...)





}//END OF playDeadSendMonstersAway()


void CChumToad::onDeathAnimationEnd(void){
	//Close the eye just in case.
	pev->skin = numberOfEyeSkins - 1;
	CBaseMonster::onDeathAnimationEnd();
}






//See if there is an entity below me. Can return NULL if not too.
//NOTICE - look at how breakables see if the player is on top or not.  That might be miles better than this.
//  Or even the mr. friendly's absmax - abxmin comparison, that's not bad either.
CBaseEntity* CChumToad::getEntityBelow(void){
	//if ( FBitSet(pevToucher->flags,FL_ONGROUND) && pevToucher->groundentity && VARS(pevToucher->groundentity) == pev )
	//{
	//...
		//assume the origin is at the floor of this entity.
		const Vector backLeftBottom = pev->origin + Vector(pev->mins.x, pev->mins.y, 0);
		const Vector backRightBottom = pev->origin + Vector(pev->mins.x, pev->maxs.y, 0);
		const Vector forwardLeftBottom = pev->origin + Vector(pev->maxs.x, pev->mins.y, 0);
		const Vector forwardRightBottom = pev->origin + Vector(pev->maxs.x, pev->maxs.y, 0);

		const Vector vecDown = Vector(0, 0, -1);
		float distDown = 8;

		TraceResult tr_backLeftBottom;
		TraceResult tr_backRightBottom;
		TraceResult tr_forwardLeftBottom;
		TraceResult tr_forwardRightBottom;
		UTIL_TraceLine(backLeftBottom, backLeftBottom + vecDown * distDown, dont_ignore_monsters, dont_ignore_glass, ENT(pev), &tr_backLeftBottom);
		UTIL_TraceLine(backRightBottom, backRightBottom + vecDown * distDown, dont_ignore_monsters, dont_ignore_glass, ENT(pev), &tr_backRightBottom);
		UTIL_TraceLine(forwardLeftBottom, forwardLeftBottom + vecDown * distDown, dont_ignore_monsters, dont_ignore_glass, ENT(pev), &tr_forwardLeftBottom);
		UTIL_TraceLine(forwardRightBottom, forwardRightBottom + vecDown * distDown, dont_ignore_monsters, dont_ignore_glass, ENT(pev), &tr_forwardRightBottom);

		
		//DebugLine_Setup(0, backLeftBottom, backLeftBottom + vecDown * distDown, tr_backLeftBottom.flFraction);
		//DebugLine_Setup(1, backRightBottom, backRightBottom + vecDown * distDown, tr_backRightBottom.flFraction);
		//DebugLine_Setup(2, forwardLeftBottom, forwardLeftBottom + vecDown * distDown, tr_forwardLeftBottom.flFraction);
		//DebugLine_Setup(3, forwardRightBottom, forwardRightBottom + vecDown * distDown, tr_forwardRightBottom.flFraction);

		//MODDD NOTE - watch the required and thorw distance above (gpGlobals->forward * #) and flFraction! If it is too small, chumtoads can be thrown "through" walls, like map walls, and clip through. Looks weird and the toad leaves this universe.
		
		
		//BOOL tracesSolid;
		//BOOL tracesStartSolid;
		//tracesSolid = (tr_backLeftBottom.fAllSolid != 0 || tr_backRightBottom.fAllSolid != 0 || tr_forwardLeftBottom.fAllSolid != 0 || tr_forwardRightBottom.fAllSolid != 0);
		//tracesStartSolid = (tr_backLeftBottom.fStartSolid != 0 || tr_backRightBottom.fStartSolid != 0 || tr_forwardLeftBottom.fStartSolid != 0 || tr_forwardRightBottom.fStartSolid != 0);
		
		//if(tracesSolid == FALSE && tracesStartSolid == FALSE)
		{
			float minFraction = 1.00;
			CBaseEntity* entityBelow = NULL;
			TraceResult* trBestTrace = NULL;
		

			if(tr_backLeftBottom.fAllSolid == FALSE && tr_backLeftBottom.flFraction < minFraction){
				minFraction = tr_backLeftBottom.flFraction;
				trBestTrace = &tr_backLeftBottom;
			}
			if(tr_backRightBottom.fAllSolid == FALSE && tr_backRightBottom.flFraction < minFraction){
				minFraction = tr_backRightBottom.flFraction;
				trBestTrace = &tr_backRightBottom;
			}
			if(tr_forwardLeftBottom.fAllSolid == FALSE && tr_forwardLeftBottom.flFraction < minFraction){
				minFraction = tr_forwardLeftBottom.flFraction;
				trBestTrace = &tr_forwardLeftBottom;
			}
			if(tr_forwardRightBottom.fAllSolid == FALSE && tr_forwardRightBottom.flFraction < minFraction){
				minFraction = tr_forwardRightBottom.flFraction;
				trBestTrace = &tr_forwardRightBottom;
			}


			if(trBestTrace != NULL){
				if(trBestTrace->pHit != NULL){
					return CBaseEntity::Instance(trBestTrace->pHit);
				}
			}

		}//END OF trace basic checks

		return NULL;

}//END OF getEntityBelow
	

float CChumToad::massInfluence(void){
	return 0.24f;
}//END OF massInfluence

int CChumToad::GetProjectileType(void){
	return PROJECTILE_ORGANIC_HARMLESS;
}

Vector CChumToad::GetVelocityLogical(void){
	return pev->velocity;
}

void CChumToad::SetVelocityLogical(const Vector& arg_newVelocity){
	pev->velocity = arg_newVelocity;
}

void CChumToad::OnDeflected(CBaseEntity* arg_entDeflector){
	m_hOwner = arg_entDeflector;
}





//MODDD - new method for determining whether to register a case of damage as worthy of LIGHT_DAMAGE or HEAVY_DAMAGE for the AI.
//        Can result in interrupting the current schedule.
//        This is expected to get called from CBaseMonster's TakeDamage method in combat.cpp. This method may be customized per monster,
//        should start with a copy of this method without calling the parent method, not much here.
void CChumToad::OnTakeDamageSetConditions(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType, int bitsDamageTypeMod){

	//MODDD - intervention. Timed damage might not affect the AI since it could get needlessly distracting.
	if(bitsDamageTypeMod & (DMG_TIMEDEFFECT|DMG_TIMEDEFFECTIGNORE) ){
		//If this is continual timed damage, don't register as any damage condition. Not worth possibly interrupting the AI.
		return;
	}

	//default case from CBaseMonster's TakeDamage.
	//Also count being in a non-combat state to force looking in that direction.  But maybe at least 0 damage should be a requirement too, even in cases where the minimum damage for LIGHT is above 0?
	if (m_MonsterState == MONSTERSTATE_IDLE || m_MonsterState == MONSTERSTATE_ALERT || flDamage > 0 )
	{
		SetConditions(bits_COND_LIGHT_DAMAGE);

		//MODDD NEW - set a timer to forget a flinch-preventing memory bit.
		forgetSmallFlinchTime = gpGlobals->time + 5;
	}

	if(gpGlobals->time >= forgetBigFlinchTime && (flDamage >=  pev->max_health * 0.55 || flDamage >= 15) )
	{
		SetConditions(bits_COND_HEAVY_DAMAGE);
		forgetSmallFlinchTime = gpGlobals->time + 5;
		forgetBigFlinchTime = gpGlobals->time + 3;
	}

/*
	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(testVar) == 10){
		//any damage causes me now.
		SetConditions(bits_COND_HEAVY_DAMAGE);
	}
	*/
	
}//END OF OnTakeDamageSetConditions


int CChumToad::getHullIndexForNodes(void){
    return NODE_SMALL_HULL;  //safe?
}



/*
CChumToadRespawnable::CChumToadRespawnable(void) {

}//END OF constructor

// already done?
//LINK_ENTITY_TO_CLASS(monster_chumtoad_respawnable, CChumToadRespawnable);


void CChumToadRespawnable::Spawn(void) {
	CChumToad::Spawn();

	// SAFETY
	this->respawn_origin = pev->origin;
	this->respawn_angles = pev->angles;

	// after that call, may as well lose anything special about the name. If there was anything.
	// May be unnecessary here.
	pev->classname = MAKE_STRING("monster_chumtoad");
}
*/
