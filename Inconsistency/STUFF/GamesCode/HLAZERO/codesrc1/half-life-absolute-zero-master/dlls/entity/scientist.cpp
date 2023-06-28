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
// human scientist (passive lab worker)
//=========================================================


//TODO... god even more.
//Make it so taking damage from TIMED_DAMAGE or TIMED_DAMAGE_MOD (right bitmask) doesn't unfollow. doesn't make sense to, no direct enemy to scare them to cause this damage.

//MODDD - TODO!!!
//SCIENTIST: say a line from fear's or NOOO when player dies while following!



//NOTICE - are these unused?
/*
SC_NOGO scientist/dontgothere

SC_MONST0 scientist/seeheadcrab
SC_MONST1 scientist/importantspecies

SC_HEAR0 scientist/ihearsomething
SC_HEAR1 scientist/didyouhear
SC_HEAR2 scientist/whatissound
*/



#include "extdll.h"
#include "scientist.h"
#include "util.h"
#include "cbase.h"
#include "basemonster.h"
#include "talkmonster.h"
#include "schedule.h"
#include "defaultai.h"
#include "scripted.h"
#include "util_model.h"
#include "soundent.h"
//MODDD - added, so that the player's "setSuitUpdate" may be called directly (edicts are too general
//and don't have that, nor are they safely moddable to my knowledge, very DLL intensive in transfers).
#include "player.h"


//MODDD
EASY_CVAR_EXTERN_DEBUGONLY(wildHeads)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST(sv_germancensorship)
EASY_CVAR_EXTERN_DEBUGONLY(scientistHealNPCDebug)
EASY_CVAR_EXTERN_DEBUGONLY(scientistHealNPC)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(thatWasntPunch)
EASY_CVAR_EXTERN_DEBUGONLY(scientistHealNPCFract)
EASY_CVAR_EXTERN_DEBUGONLY(scientistHealCooldown)
EASY_CVAR_EXTERN_DEBUGONLY(monsterSpawnPrintout)
extern BOOL globalPSEUDO_iCanHazMemez;
//was this model found in the client's folder too?
extern BOOL globalPSEUDO_germanModel_scientistFound;
EASY_CVAR_EXTERN_DEBUGONLY(scientistBravery)
EASY_CVAR_EXTERN(pissedNPCs)



float g_scientist_PredisasterSuitMentionAllowedTime = -1;
float g_scientist_HeadcrabMentionAllowedTime = -1;
float g_scientist_sayGetItOffCooldown = -1;
int g_scientist_crouchstand_sequenceID = -1;
int g_scientist_checktie_sequenceID = -1;



// yes, the physical is best under the allowed.  Very odd.
#define SCIENTIST_MELEE_ALLOW_RANGE 54
#define SCIENTIST_MELEE_PHYSICAL_RANGE 46

#define NUM_SCIENTIST_HEADS		3

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define SCIENTIST_AE_HEAL		( 1 )
#define SCIENTIST_AE_NEEDLEON	( 2 )
#define SCIENTIST_AE_NEEDLEOFF	( 3 )




#define BODYGROUP_HEAD 1
// no need for specifics here, head choices handled further below

#define BODYGROUP_NEEDLE 2
#define NEEDLE_OFF 0
#define NEEDLE_ON 1



//MODDD - there is a rather suble problem with this setup.
//Old head enumeration:
//enum { HEAD_GLASSES = 0, HEAD_EINSTEIN = 1, HEAD_LUTHER = 2, HEAD_SLICK = 3}
//new head enumeration:
//enum { HEAD_GLASSES = 0, HEAD_EINSTEIN = 1, HEAD_SLICK = 2 };

//And, here is the retail model's head:
//HEAD_GLASSES
//HEAD_EINSTEIN
//HEAD_LUTHER
//HEAD_SLICK
//See the issue?  If you want to remove anything BUT the last one, simply cutting the last one won't be completely effective.

//So, better idea:  handle the offset (if not using the alpha model that has 3 head models).  Otherwise, this is not necessary:
//So if there are any immediate issues, try adjusting this first.
#define headOffsetFix 0
//NOTE: the alpa model can still treat "HEAD_SLICK" as the egon head if it sticks to being "2".  What is in a name, after all?




#if headOffsetFix == 0
enum { HEAD_GLASSES = 0, HEAD_EINSTEIN = 1, HEAD_SLICK = 2 };
int scientistHeadsModelRef[] = { 0, 1, 2 };
#else
enum { HEAD_GLASSES = 0, HEAD_EINSTEIN = 1, HEAD_SLICK = 3 };
int scientistHeadsModelRef[] = { 0, 1, 3 };
#endif




// sitting scientist animation sequence aliases
typedef enum
{
	SITTING_ANIM_sitlookleft,
	SITTING_ANIM_sitlookright,
	SITTING_ANIM_sitscared,
	SITTING_ANIM_sitting2,
	SITTING_ANIM_sitting3
} SITTING_ANIM;





enum
{
	SCHED_HIDE = LAST_TALKMONSTER_SCHEDULE + 1,
	SCHED_FEAR,
	SCHED_PANIC,
	SCHED_STARTLE,
	SCHED_TARGET_CHASE_SCARED,
	SCHED_TARGET_FACE_SCARED,
	SCHED_SCIENTIST_ANGRY_CHASE_ENEMY,
	SCHED_SCIENTIST_ANGRY_CHASE_ENEMY_FAILED,
};
	
enum
{
	TASK_SAY_HEAL = LAST_TALKMONSTER_TASK + 1,
	TASK_HEAL,
	TASK_SAY_FEAR,
	TASK_RUN_PATH_SCARED,
	TASK_SCREAM,
	TASK_RANDOM_SCREAM,
	TASK_MOVE_TO_TARGET_RANGE_SCARED,
	TASK_SCIENTIST_FIGHT_OR_FLIGHT,
	TASK_SCIENTIST_ANGRY_CHASE_ENEMY_FAILED
};



extern void scientistHeadFilter( CBaseMonster& somePerson, int arg_numberOfModelBodyParts, int* trueBody);

//MODDD - classes moved to scientist.h

const char* CScientist::pDeathSounds[] = 
{
	"scientist/sci_die1.wav",
	"scientist/sci_die2.wav",
	"scientist/sci_die3.wav",
	"scientist/sci_die4.wav",
};



int CScientist::numberOfModelBodyParts = -1;

#if REMOVE_ORIGINAL_NAMES != 1
	LINK_ENTITY_TO_CLASS( monster_scientist, CScientist );
#endif

#if EXTRA_NAMES > 0
	LINK_ENTITY_TO_CLASS( scientist, CScientist );

	//no extras.

#endif

TYPEDESCRIPTION	CScientist::m_SaveData[] =
{
	DEFINE_FIELD( CScientist, m_painTime, FIELD_TIME ),
	DEFINE_FIELD( CScientist, m_healTime, FIELD_TIME ),
	DEFINE_FIELD( CScientist, m_fearTime, FIELD_TIME ),
	//MODDD
	DEFINE_FIELD( CScientist, trueBody, FIELD_INTEGER ),



};
IMPLEMENT_SAVERESTORE( CScientist, CTalkMonster );

void CScientist::PostRestore(){
	scientistHeadFilter(*this, numberOfModelBodyParts, &trueBody);
	
}


//MODDD - new data stuff.	
const char *CScientist::pAttackHitSounds[] = 
{
	"zombie/claw_strike1.wav",
	"zombie/claw_strike2.wav",
	"zombie/claw_strike3.wav",
};

const char *CScientist::pAttackMissSounds[] = 
{
	"zombie/claw_miss1.wav",
	"zombie/claw_miss2.wav",
};



const char* CScientist::madInterSentences[] = {
	"!SC_POKE0",
	"!SC_POKE1",
	"!SC_POKE2",
	"!SC_POKE3",
	"!SC_POKE4",
	"!SC_POKE5",
	"!SC_POKEQ0",
	"!SC_POKEQ1",
	////"!SC_POKE8",
	"!SC_POKE9",
	"!SC_POKE10",
	//"!SC_POKEQ2",
	"!SC_POKE12",
	//"!SC_POKEQ3",
	"!SC_POKE14",
	"!SC_POKE15",
	"!SC_POKE16",
	"!SC_POKE17",
	"!SC_POKE18",
	"!SC_POKE19",
	"!SC_POKE20",
	"!SC_POKE21",
	"!SC_POKE22",
	"!SC_POKE23",
	"!SC_POKE24",
	"!SC_POKE25",
	"!SC_POKE26",
	"!SC_POKE27",
	"!SC_POKE28",
	"!SC_POKE29",
	"!SC_POKE30",
	"!SC_POKE31",
	"!SC_POKE32",
	"!SC_POKE33",
	"!SC_POKE34",
	"!SC_POKE35",
	"!SC_POKE36"


};
//int CScientist::madInterSentencesMax = 37 - 1;



int CScientist::getMadInterSentencesMax(void){
	if(globalPSEUDO_iCanHazMemez == TRUE){
		return 34;
	}else{
		return 33;
	}
}
int CScientist::getMadSentencesMax(void){
	if(globalPSEUDO_iCanHazMemez == TRUE){
		return 37;
	}else{
		return 36;
	}
}


// Chase enemy schedule
Task_t tlAngryScientistChaseEnemyFailed[] = 
{
	{ TASK_SCIENTIST_ANGRY_CHASE_ENEMY_FAILED, (float)0 },

	// Go ahead, look scared.  Why not.
	{ TASK_PLAY_SEQUENCE_FACE_ENEMY, (float)ACT_EXCITED },	// This is really fear-stricken excitement
	{ TASK_SET_ACTIVITY, (float)ACT_IDLE },

	//no, just allow to repick a schedule.
	//{ TASK_SET_SCHEDULE,	(float)???	}
};

Schedule_t slAngryScientistChaseEnemyFailed[] =
{
	{ 
		tlAngryScientistChaseEnemyFailed,
		ARRAYSIZE ( tlAngryScientistChaseEnemyFailed ),
		0,
		0,
		"Chase Enemy ASf"
	},
};


// Chase enemy schedule
Task_t tlAngryScientistChaseEnemy[] = 
{
	{ TASK_SCREAM, (float)0 },
	{ TASK_SET_FAIL_SCHEDULE,	(float)SCHED_SCIENTIST_ANGRY_CHASE_ENEMY_FAILED	},
	//{ TASK_GET_PATH_TO_ENEMY,	(float)0		},
	//{ TASK_RUN_PATH,			(float)0		},
	//{ TASK_WAIT_FOR_MOVEMENT,	(float)0		},
	//Now modeled after the SmartFollow in schedule.cpp.
	{ TASK_MOVE_TO_ENEMY_RANGE, (float)(SCIENTIST_MELEE_ALLOW_RANGE - 5) }
};

Schedule_t slAngryScientistChaseEnemy[] =
{
	{ 
		tlAngryScientistChaseEnemy,
		ARRAYSIZE ( tlAngryScientistChaseEnemy ),
		bits_COND_NEW_ENEMY			|
		//MODDD - added, the bullsquid counts this.  Why doesn't everything?
		bits_COND_ENEMY_DEAD |

		bits_COND_CAN_RANGE_ATTACK1	|
		bits_COND_CAN_MELEE_ATTACK1	|
		bits_COND_CAN_RANGE_ATTACK2	|
		bits_COND_CAN_MELEE_ATTACK2	|
		bits_COND_TASK_FAILED		|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER|bits_SOUND_COMBAT,
		"Chase Enemy AS"
	},
};


Task_t	tlScientistPunch[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_MELEE_ATTACK2,		(float)0		},
};

Schedule_t	slScientistPunch[] =
{
	{ 
		tlScientistPunch,
		ARRAYSIZE ( tlScientistPunch ), 
		bits_COND_NEW_ENEMY			|
		bits_COND_ENEMY_DEAD		|
		//bits_COND_LIGHT_DAMAGE		|
		//bits_COND_HEAVY_DAMAGE		|
		bits_COND_ENEMY_OCCLUDED,
		0,
		"scientist punch"
	},
};



//=========================================================
// AI Schedules Specific to this monster
//=========================================================
Task_t	tlSciFollow[] =
{
	{ TASK_SET_FAIL_SCHEDULE,	(float)SCHED_CANT_FOLLOW },	// If you fail, bail out of follow
	{ TASK_MOVE_TO_TARGET_RANGE,(float)128		},	// Move within 128 of target ent (client)
	{ TASK_FOLLOW_SUCCESSFUL, (float)0		},
//	{ TASK_SET_SCHEDULE,		(float)SCHED_TARGET_FACE },
};

Schedule_t	slSciFollow[] =
{
	{
		tlSciFollow,
		ARRAYSIZE ( tlSciFollow ),
		bits_COND_NEW_ENEMY |
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_HEAR_SOUND,
		bits_SOUND_COMBAT |
		bits_SOUND_DANGER,
		"SciFollow"
	},
};

Task_t	tlFollowScared[] =
{
	{ TASK_SET_FAIL_SCHEDULE,	(float)SCHED_TARGET_CHASE },// If you fail, follow normally
	{ TASK_MOVE_TO_TARGET_RANGE_SCARED,(float)128		},	// Move within 128 of target ent (client)
	{ TASK_FOLLOW_SUCCESSFUL, (float)0		},
//	{ TASK_SET_SCHEDULE,		(float)SCHED_TARGET_FACE_SCARED },
};

Schedule_t	slFollowScared[] =
{
	{
		tlFollowScared,
		ARRAYSIZE ( tlFollowScared ),
		bits_COND_NEW_ENEMY |
		bits_COND_HEAR_SOUND |
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE,
		bits_SOUND_DANGER,
		"FollowScared"
	},
};

Task_t	tlFaceTargetScared[] =
{
	{ TASK_FACE_TARGET,			(float)0		},
	{ TASK_SET_ACTIVITY,		(float)ACT_CROUCHIDLE },
	{ TASK_SET_SCHEDULE,		(float)SCHED_TARGET_CHASE_SCARED },
};

Schedule_t	slFaceTargetScared[] =
{
	{
		tlFaceTargetScared,
		ARRAYSIZE ( tlFaceTargetScared ),
		bits_COND_HEAR_SOUND |
		bits_COND_NEW_ENEMY,
		bits_SOUND_DANGER,
		"FaceTargetScared"
	},
};

//MODDD - stopFollowing schedule moved to talkMonster


Task_t	tlHeal[] =
{
	{ TASK_SET_FAIL_SCHEDULE,	(float)SCHED_TARGET_CHASE },	// If you fail, catch up with that guy! (change this to put syringe away and then chase)
	//MODDD - NOTE ON PREVIOUS COMMENT.  Getting far away while the scientist is getting the syringe out does not make the schedule fail,
	// TASK_HEAL just refuses to play the inject anim and  ACT_DISARM happens right then.
	// So, removed the ACT_DISARM set from this schedule and leave it up to elsewhere to call ACT_DISARM when not pursuing something to heal.
	{ TASK_MOVE_TO_TARGET_RANGE,(float)56		},	// Move within 60 of target ent (client)
	{ TASK_FOLLOW_SUCCESSFUL, (float)0		},
	{ TASK_FACE_IDEAL,			(float)0		},
	{ TASK_SAY_HEAL,			(float)0		},
	{ TASK_PLAY_SEQUENCE_FACE_TARGET,		(float)ACT_ARM	},			// Whip out the needle
	{ TASK_HEAL,				(float)0	},	// Put it in the player
	//{ TASK_PLAY_SEQUENCE_FACE_TARGET,		(float)ACT_DISARM	},			// Put away the needle
};

Schedule_t	slHeal[] =
{
	{
		tlHeal,
		ARRAYSIZE ( tlHeal ),
		0,	// Don't interrupt or he'll end up running around with a needle all the time
		0,
		"Heal"
	},
};



//MODDD - new!  Just to force putting the syringe away if slHeal gets interrupted.
Task_t	tlScientistPutAwaySyringe[] =
{
	{ TASK_STOP_MOVING, 0 },
	{ TASK_PLAY_SEQUENCE, (float)ACT_DISARM },
};

Schedule_t slScientistPutAwaySyringe[] =
{
	{
		tlScientistPutAwaySyringe,
		ARRAYSIZE(tlScientistPutAwaySyringe),
		0,
		0,
		"ScientistPutAwaySyr"
	}
};





Task_t	tlFaceTarget[] =
{
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_FACE_TARGET,			(float)0		},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_SET_SCHEDULE,		(float)SCHED_TARGET_CHASE },
};

Schedule_t	slFaceTarget[] =
{
	{
		tlFaceTarget,
		ARRAYSIZE ( tlFaceTarget ),
		bits_COND_CLIENT_PUSH |
		bits_COND_NEW_ENEMY |
		bits_COND_HEAR_SOUND,
		bits_SOUND_COMBAT |
		bits_SOUND_DANGER,
		"FaceTarget"
	},
};


Task_t	tlSciPanic[] =
{
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	//MODDD - intervention.

	//{ TASK_SCREAM,				(float)0		},
	{ TASK_SAY_FEAR, (float)0 },

	{ TASK_SCIENTIST_FIGHT_OR_FLIGHT,  (float)1  },    //NOTICE - this "1" means he is more likely to attack the monster, since Panic is picked when we're cornered (no cover, no choice)
	{ TASK_PLAY_SEQUENCE_FACE_ENEMY,		(float)ACT_EXCITED	},	// This is really fear-stricken excitement
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE	},
};

Schedule_t	slSciPanic[] =
{
	{
		tlSciPanic,
		ARRAYSIZE ( tlSciPanic ),
		0,
		0,
		"SciPanic"
	},
};



// only swaps out TASK_FACE_ENEMY for TASK_FACE_IDEAL.
Task_t	tlSciPanicInPlace[] =
{
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_FACE_IDEAL,			(float)0		},
	//MODDD - intervention.

	//{ TASK_SCREAM,				(float)0		},
	{ TASK_SAY_FEAR, (float)0 },

	{ TASK_SCIENTIST_FIGHT_OR_FLIGHT,  (float)1  },    //NOTICE - this "1" means he is more likely to attack the monster, since Panic is picked when we're cornered (no cover, no choice)
	{ TASK_PLAY_SEQUENCE_FACE_ENEMY,		(float)ACT_EXCITED	},	// This is really fear-stricken excitement
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE	},
};

Schedule_t	slSciPanicInPlace[] =
{
	{
		tlSciPanicInPlace,
		ARRAYSIZE ( tlSciPanicInPlace ),
		0,
		0,
		"SciPanicInPlace"
	},
};

Task_t	tlIdleSciStand[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_WAIT,				(float)2		}, // repick IDLESTAND every two seconds.
	{ TASK_TLK_HEADRESET,		(float)0		}, // reset head position
};

Schedule_t	slIdleSciStand[] =
{
	{
		tlIdleSciStand,
		ARRAYSIZE ( tlIdleSciStand ),
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_HEAR_SOUND	|
		bits_COND_SMELL			|
		bits_COND_CLIENT_PUSH	|
		bits_COND_PROVOKED,

		bits_SOUND_COMBAT		|// sound flags
		//bits_SOUND_PLAYER		|
		//bits_SOUND_WORLD		|
		bits_SOUND_DANGER		|
		bits_SOUND_MEAT			|// scents
		bits_SOUND_CARCASS		|
		bits_SOUND_GARBAGE,
		"IdleSciStand"

	},
};


Task_t	tlScientistCover[] =
{
	{ TASK_SET_FAIL_SCHEDULE,		(float)SCHED_PANIC },		// If you fail, just panic!
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_FIND_COVER_FROM_ENEMY,	(float)0					},
	{ TASK_RUN_PATH_SCARED,			(float)0					},
	{ TASK_TURN_LEFT,				(float)179					},
	{ TASK_SET_SCHEDULE,			(float)SCHED_HIDE			},
};

Schedule_t	slScientistCover[] =
{
	{
		tlScientistCover,
		ARRAYSIZE ( tlScientistCover ),
		bits_COND_NEW_ENEMY,
		0,
		"ScientistCover"
	},
};


Task_t	tlScientistHide[] =
{
	{ TASK_SET_FAIL_SCHEDULE,		(float)SCHED_PANIC },		// If you fail, just panic!
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_PLAY_SEQUENCE,			(float)ACT_CROUCH			},
	{ TASK_SET_ACTIVITY,			(float)ACT_CROUCHIDLE		},	// FIXME: This looks lame
	{ TASK_WAIT_RANDOM,				(float)10.0					},
};

Schedule_t	slScientistHide[] =
{
	{
		tlScientistHide,
		ARRAYSIZE ( tlScientistHide ),
		bits_COND_NEW_ENEMY |
		bits_COND_HEAR_SOUND |
		bits_COND_SEE_ENEMY |
		bits_COND_SEE_HATE |
		bits_COND_SEE_FEAR |
		bits_COND_SEE_DISLIKE,
		bits_SOUND_DANGER,
		"ScientistHide"
	},
};


Task_t	tlScientistStartle[] =
{
	{ TASK_SET_FAIL_SCHEDULE,		(float)SCHED_PANIC },		// If you fail, just panic!
	{ TASK_RANDOM_SCREAM,			(float)0.3 },				// Scream 30% of the time
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_PLAY_SEQUENCE_FACE_ENEMY,(float)ACT_CROUCH			},
	{ TASK_RANDOM_SCREAM,			(float)0.1 },				// Scream again 10% of the time
	{ TASK_PLAY_SEQUENCE_FACE_ENEMY,(float)ACT_CROUCHIDLE		},
	{ TASK_WAIT_RANDOM,				(float)1.0					},
};

Schedule_t	slScientistStartle[] =
{
	{
		tlScientistStartle,
		ARRAYSIZE ( tlScientistStartle ),
		bits_COND_NEW_ENEMY |
		bits_COND_SEE_ENEMY |
		bits_COND_SEE_HATE |
		bits_COND_SEE_FEAR |
		bits_COND_SEE_DISLIKE |
		//MODDD - new
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE,
		0,
		"ScientistStartle"
	},
};


Task_t	tlSciFear[] =
{
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_FACE_ENEMY,				(float)0					},
	{ TASK_SAY_FEAR,				(float)0					},
	//MODDD - new.
	{ TASK_SCIENTIST_FIGHT_OR_FLIGHT,  (float)0   },

//MODDD: used to be commented out, no longer is.
	{ TASK_PLAY_SEQUENCE,			(float)ACT_FEAR_DISPLAY		},
};

Schedule_t	slSciFear[] =
{
	{
		tlSciFear,
		ARRAYSIZE ( tlSciFear ),
		//MODDD - added some interrupt conditions.
		//        Got it, you love gawking at the thing you're afraid of, but I think bullets and/or claws ought to make you hurry that up a bit.
		bits_COND_NEW_ENEMY |
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_HEAR_SOUND,
		bits_SOUND_DANGER,
		"Fear"
	},
};



//clone of slTakeCoverFromOrigin that uses the scientist scary run thing
Task_t	tlScientistTakeCoverFromOrigin[] =
{
	{ TASK_STOP_MOVING,					(float)0					},
	{ TASK_FIND_COVER_FROM_ORIGIN,		(float)0					},
	{ TASK_RUN_PATH_SCARED,					(float)0					},
	{ TASK_WAIT_FOR_MOVEMENT,			(float)0					},
	{ TASK_REMEMBER,					(float)bits_MEMORY_INCOVER	},
	{ TASK_TURN_LEFT,					(float)179					},
};

Schedule_t	slScientistTakeCoverFromOrigin[] =
{
	{ 
		tlScientistTakeCoverFromOrigin,
		ARRAYSIZE ( tlScientistTakeCoverFromOrigin ), 
		bits_COND_NEW_ENEMY |
		//MODDD CRITICAL - now interruptable by heavy damage.  May or may not be a good thing.
		bits_COND_HEAVY_DAMAGE,
		0,
		"SciTakeCoverFromOrigin"
	},
};



DEFINE_CUSTOM_SCHEDULES( CScientist )
{
	slSciFollow,
	slFaceTarget,
	slIdleSciStand,
	slSciFear,
	slScientistCover,
	slScientistHide,
	slScientistStartle,
	slHeal,
	slScientistPutAwaySyringe,
	//slStopFollowing,    Belongs to all of TalkMonster now!
	slSciPanic,
	slSciPanicInPlace,
	slFollowScared,
	slFaceTargetScared,
	slAngryScientistChaseEnemyFailed,
	slAngryScientistChaseEnemy,
	slScientistPunch,
	slScientistTakeCoverFromOrigin,
};


IMPLEMENT_CUSTOM_SCHEDULES( CScientist, CTalkMonster );




CScientist::CScientist(void) {
	//givenModelBody = -1;

	nextRandomSpeakCheck = -1;
	screamCooldown = -1;

	explodeDelay = -1;

	aggroOrigin = Vector(0, 0, 0); //???

	aggroCooldown = -1;
	aggro = 0;

	playFearAnimCooldown = -1;

	trueBody = -1;

	healNPCChosen = FALSE;
	healNPCCheckDelay = -1;

	// On a restore, if never seen before, this is loaded as "0" instead.  Beware of that.


	madInterSentencesLocation = madInterSentences;
	//madInterSentencesMaxLocation = &madInterSentencesMax;

}

void CScientist::DeclineFollowingProvoked(CBaseEntity* pCaller){
	
	if(this->m_pSchedule == slScientistCover || this->m_pSchedule == slSciPanic){
		//no reaction, already running.

	}else if(aggro <= 0){
		// try the fight or flight. No random chance of going agro if there is valid cover though.
		// Don't interrupt already punching or running though.
		if (m_Activity != ACT_MELEE_ATTACK2 && m_Activity != ACT_RUN_SCARED && m_Activity != ACT_RUN && m_Activity != ACT_RUN_HURT) {
			ChangeSchedule(GetScheduleOfType(SCHED_FIGHT_OR_FLIGHT));
		}

	}else{  //aggro above > 0? for now, no effect, they are already angry.

	}
}



void CScientist::SayHello(CBaseEntity* argPlayerTalkTo) {

	if (FBitSet(pev->spawnflags, SF_MONSTER_PREDISASTER)) {
		// If predisaster, and the player has the suit,
		// (pPlayer->pev->weapons & (1<<WEAPON_SUIT))
		if(argPlayerTalkTo->pev->weapons & (1 << WEAPON_SUIT)){
			// Mention the suit once, and with a decent chance later too.
			if (
				g_scientist_PredisasterSuitMentionAllowedTime == -1 ||
				(gpGlobals->time >= g_scientist_PredisasterSuitMentionAllowedTime && RANDOM_FLOAT(0, 1) <= 0.55)
			) {
				PlaySentenceSingular("SC_HELLO6", 4, VOL_NORM, ATTN_NORM);  // new HEV suit, that should be very useful
				g_scientist_PredisasterSuitMentionAllowedTime = gpGlobals->time + 16;
				return;
			}
		}//END OF suit check

		// 'c1a0_sci_gm1' is already in sentences.txt as a SC_PHELLO choice now.

	}

	CTalkMonster::SayHello(argPlayerTalkTo);
}

void CScientist::SayIdleToPlayer(CBaseEntity* argPlayerTalkTo) {

	if (FBitSet(pev->spawnflags, SF_MONSTER_PREDISASTER)) {
		// If predisaster, and the player has the suit,
		// (pPlayer->pev->weapons & (1<<WEAPON_SUIT))
		if (argPlayerTalkTo->pev->weapons & (1 << WEAPON_SUIT)) {
			// Mention the suit once, and with a decent chance later too.
			float theRandom = RANDOM_FLOAT(0, 1);
			if (g_scientist_PredisasterSuitMentionAllowedTime == -1 ||
				(gpGlobals->time >= g_scientist_PredisasterSuitMentionAllowedTime && theRandom < 0.31)
				) {
				PlaySentenceSingular("SC_HELLO6", 4, VOL_NORM, ATTN_NORM);
				g_scientist_PredisasterSuitMentionAllowedTime = gpGlobals->time + 16;
				return;
			}
		}//END OF suit check
	}
	else {
		// even after the disaster, might whine about ties... rarely.
		//if (RANDOM_FLOAT(0, 1) < 0.065) {
		if (RANDOM_FLOAT(0, 1) < 0.090) {
		//if(1){
			PlaySentenceSingular("SC_PIDLE1", 4.6, VOL_NORM, ATTN_NORM);  // weartie

			if (this->m_IdealActivity == ACT_IDLE && m_MonsterState != MONSTERSTATE_SCRIPT) {
				// only interrupt the idle activity to do this, and not in SCRIPT.  Just in case.
				this->SetSequenceByIndex(g_scientist_checktie_sequenceID, 1.0, FALSE);

				usingCustomSequence = FALSE;  // don't block returning to idle anim
				// WAIT, should not happen anymore.  Being set to a sequence that doesn't loop will fall back to idle-anim-picking after it is done now
				// NEVERMIND, requiring it again.  Explicitness is good.

				doNotResetSequence = TRUE; // don't reset myself.
			}

			return;
		}
	}

	CTalkMonster::SayIdleToPlayer(argPlayerTalkTo);
}


void CScientist::SayQuestion(CTalkMonster* argTalkTo) {

	if (FBitSet(pev->spawnflags, SF_MONSTER_PREDISASTER)) {
		// nothing special
	}
	else {
		// even after the disaster, might whine about something
		if (RANDOM_FLOAT(0, 1) < 0.032) {
			PlaySentenceSingular("SC_PQUEST15", 2.9, VOL_NORM, ATTN_NORM);  // hungryyet
			return;
		}
	}

	CTalkMonster::SayQuestion(argTalkTo);
}



void CScientist::SayAlert(void) {

	if (m_hEnemy != NULL) {
		Vector tempEnBoundDelta = (m_hEnemy->pev->absmax - m_hEnemy->pev->absmin);
		float tempEnSize = tempEnBoundDelta.x * tempEnBoundDelta.y * tempEnBoundDelta.z;

		if (tempEnSize < 16000) {  //headcrab size: 13824
			// play 60% of the time
			float someRand = RANDOM_FLOAT(0, 1);
			if (someRand < 0.6) {
				PlaySentence("SC_FEAR", 5, VOL_NORM, ATTN_NORM);
			}
		}
		else if (tempEnSize <= 500000) {  //size of agrunt: about 348160
			// fear
			PlaySentence("SC_FEAR", 5, VOL_NORM, ATTN_NORM);
		}
		else {
			// OH GOD ITS HUGE
			PlaySentence("SC_SCREAM_TRU", 5, VOL_NORM, ATTN_NORM);
		}
	}
	else {
		// forced to play the alert sound anyway?  ok.
		PlaySentence("SC_FEAR", 5, VOL_NORM, ATTN_NORM);
	}


}//SayAlert


void CScientist::SayDeclineFollowing(void) {
	//MODDD
	if (EASY_CVAR_GET(pissedNPCs) < 1) {
		//Talk( 10 );   pointless. PlaySentence already sets the same thing.
		m_hTalkTarget = m_hEnemy;

		if (recentDeclines < 30) {
			// normal.
			PlaySentence("SC_POK", 4, VOL_NORM, ATTN_NORM);
		}
		else {
			float randomVal = RANDOM_FLOAT(0, 1);
			if (randomVal < 0.25) {
				PainSound_Play();
			}
			else if (randomVal < 0.75) {
				switch (RANDOM_LONG(0, 5)) {
				case 0:PlaySentenceSingular("SC_PLFEAR1", 4, VOL_NORM, ATTN_NORM); break; //scientist/canttakemore
				case 1:PlaySentenceSingular("SC_PLFEAR3", 4, VOL_NORM, ATTN_NORM); break; //scientist/noplease
				case 2:PlaySentenceSingular("SC_PLFEAR4", 4, VOL_NORM, ATTN_NORM); break; //getoutofhere
				case 3:PlaySentenceSingular("SC_SCREAM_TRU14", 4, VOL_NORM, ATTN_NORM); break; //GORDON
				case 4:PlaySentenceSingular("SC_PLFEAR0", 4, VOL_NORM, ATTN_NORM); break; //what are you doing
				case 5:PlaySentenceSingular("SC_ANNOYED", 4, VOL_NORM, ATTN_NORM); break; //are you insane	
				}
			}
			else {
				PlaySentence("SC_SCREAM_TRU", 4, VOL_NORM, ATTN_NORM);
			}
		}

	}
	else {

		playPissed();
	}
}//SayDeclineFollowing

void CScientist::SayDeclineFollowingProvoked(void) {

	if (EASY_CVAR_GET(pissedNPCs) != 1 || !globalPSEUDO_iCanHazMemez) {
		PlaySentence("SC_SCREAM_TRU", 4, VOL_NORM, ATTN_NORM);  //OH NO YOU FOUND ME.
	}
	else {
		PlaySentence("BA_POKE_D", 8, VOL_NORM, ATTN_NORM);
	}

}




// Custom Say method for the scientist, convenience.
void CScientist::SayFear(void) {
	m_hTalkTarget = m_hEnemy;   // it is ok for m_hTalkTarget to be NULL.
	if (m_hEnemy != NULL && m_hEnemy->IsPlayer()) {
		//PlaySentence("SC_PLFEAR", 5, VOL_NORM, ATTN_NORM);
		//MODDD - call upon this, more variety
		SayProvoked();
	}
	else {
		// Enemy and talk target may be NULL here!
		//MODDD - same
		float randomVal = RANDOM_FLOAT(0, 1);
		if (randomVal < 0.7) {
			PlaySentence("SC_FEAR", 5, VOL_NORM, ATTN_NORM);
		}
		else if (randomVal < 0.9) {
			PlaySentence("SC_SCREAM", 5, VOL_NORM, ATTN_NORM);
		}
		else {
			PlaySentence("SC_SCREAM_TRU", 4, VOL_NORM, ATTN_NORM);  //OH NO YOU FOUND ME.
		}
	}
}//SayFear

void CScientist::SayProvoked(void){

	if(EASY_CVAR_GET(pissedNPCs) != 1 || !globalPSEUDO_iCanHazMemez){
		float randomVal = RANDOM_FLOAT(0, 1);
		if (randomVal < 0.6) {
			PlaySentence("SC_PLFEAR", 6, VOL_NORM, ATTN_NORM);
		}
		else if (randomVal < 0.72) {
			PlaySentence("SC_SCREAM_TRU", 6, VOL_NORM, ATTN_NORM);
		}
		else if (randomVal < 0.76) {
			PlaySentence("SC_SCREAM", 6, VOL_NORM, ATTN_NORM);
		}
		else if (randomVal < 0.80) {
			PlaySentence("SC_FEAR", 6, VOL_NORM, ATTN_NORM);
		}
		else {
			// SC_PLFEAR3 was included above already now
			switch (RANDOM_LONG(0, 2)) {
			case 0:UTIL_PlaySound(ENT(pev), CHAN_VOICE, "scientist/sci_pain2.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
			case 1:PlaySentenceSingular("SC_FEAR3", 6, VOL_NORM, ATTN_NORM); break;
			case 2:PlaySentenceSingular("SCI_EXTRAPROVOKED", 6, VOL_NORM, ATTN_NORM); break;
			}//END OF decision
		}

	}else{
		PlaySentence( "BA_POKE_D", 8, VOL_NORM, ATTN_NORM );
	}

	if(this->m_pSchedule == slScientistCover ){
		//no reaction, already running.

	}else if(aggro <= 0){
		//try the fight or flight. No random chance of going agro if there is valid cover though.

		//UH OH. IS THIS LINE OFFENDING? Let's say this in a different place perhaps.
		//ChangeSchedule(GetScheduleOfType(SCHED_FIGHT_OR_FLIGHT));

	}else{  //aggro above > 0? for now, no effect, they are already angry.

	}
}

void CScientist::SayStopShooting(void) {

	if (EASY_CVAR_GET(pissedNPCs) < 1 || !globalPSEUDO_iCanHazMemez) {
		float randomVal = RANDOM_FLOAT(0, 1);
		if (randomVal < 0.333) {
			// old SC_SCARED0.
			PlaySentenceSingular("SC_PLFEAR0", RANDOM_FLOAT(2.8, 3.2), VOL_NORM, ATTN_NORM);
			return;
		}
	}

	CTalkMonster::SayStopShooting();
}


void CScientist::SaySuspicious(void){
	if(EASY_CVAR_GET(pissedNPCs) != 1 || !globalPSEUDO_iCanHazMemez){
		
		float randomVal = RANDOM_FLOAT(0, 1);

		if (randomVal < 0.4) {
			PlaySentence("SC_SCREAM", 4, VOL_NORM, ATTN_NORM); //ends up tying to the sci_fear sounds.
		}
		else if (randomVal < 0.6) {
			switch (RANDOM_LONG(0, 1)) {
			case 0:PlaySentenceSingular("SC_PLFEAR1", 4, VOL_NORM, ATTN_NORM); break; //scientist/canttakemore
			case 1:PlaySentenceSingular("SC_PLFEAR3", 4, VOL_NORM, ATTN_NORM); break; //scientist/noplease
			}
		}
		else {
			switch (RANDOM_LONG(0, 8)) {
			case 0:PlaySentenceSingular("SC_FEAR0", 4, VOL_NORM, ATTN_NORM); break; //nooo
			case 1:PlaySentenceSingular("SC_FEAR5", 4, VOL_NORM, ATTN_NORM); break;
			case 2:PlaySentenceSingular("SC_FEAR6", 4, VOL_NORM, ATTN_NORM); break;
			case 3:PlaySentenceSingular("SC_FEAR7", 4, VOL_NORM, ATTN_NORM); break;
			case 4:PlaySentenceSingular("SC_FEAR8", 4, VOL_NORM, ATTN_NORM); break;
			case 5:PlaySentenceSingular("SC_FEAR9", 4, VOL_NORM, ATTN_NORM); break;
			case 6:PlaySentenceSingular("SC_FEAR10", 4, VOL_NORM, ATTN_NORM); break;
			case 7:PlaySentenceSingular("SC_FEAR11", 4, VOL_NORM, ATTN_NORM); break;
			case 8:PlaySentenceSingular("SC_FEAR12", 4, VOL_NORM, ATTN_NORM); break;
			}//END OF switch
		}

	}else{
		PlaySentence( "BA_POKE_C", 6, VOL_NORM, ATTN_NORM );
	}
}
void CScientist::SayLeaderDied(void){
	switch(RANDOM_LONG(0, 3)){
		case 0:PlaySentence( "SC_SCREAM", 4, VOL_NORM, ATTN_NORM );break;
		case 1:PlaySentenceSingular( "SC_PLFEAR3", 4, VOL_NORM, ATTN_NORM ); break; //scientist/noplease
		case 2:PlaySentenceSingular( "SC_PLFEAR4", 4, VOL_NORM, ATTN_NORM ); break; //getoutofhere
		case 3:PlaySentenceSingular( "SC_FEAR0", 4, VOL_NORM, ATTN_NORM ); break; //nooo
	}//END OF switch
}//END OF SayLeaderDied

void CScientist::SayKneel(void){
	// TODO - do better sometime?
	switch(RANDOM_LONG(0, 3)){
		case 0:PlaySentenceSingular("SC_FEAR7", 4, VOL_NORM, ATTN_NORM - 0.2); break;
		case 1:PlaySentenceSingular("SC_FEAR8", 4, VOL_NORM, ATTN_NORM - 0.2); break;
		case 2:PlaySentenceSingular("SC_FEAR10", 4, VOL_NORM, ATTN_NORM - 0.2); break;
		case 3:PlaySentenceSingular("SC_FEAR11", 4, VOL_NORM, ATTN_NORM - 0.2); break;
	}//END OF switch
	/*
	startle5
	startle6
	startle8
	startle9
	*/
}

//Say a sentence to express interest in something, like stopping to stare at a chumtoad.
void CScientist::SayNearPassive(void){
	switch(RANDOM_LONG(0, 18)){
	case 0:PlaySentenceSingular( "SC_QUESTION0", 4, VOL_NORM, ATTN_NORM );break;
	case 1:PlaySentenceSingular( "SC_QUESTION2", 4, VOL_NORM, ATTN_NORM );break;
	case 2:PlaySentenceSingular( "SC_QUESTION7", 4, VOL_NORM, ATTN_NORM );break;
	case 3:PlaySentenceSingular( "SC_QUESTION8", 4, VOL_NORM, ATTN_NORM );break;
	case 4:PlaySentenceSingular( "SC_QUESTION10", 4, VOL_NORM, ATTN_NORM );break;
	case 5:PlaySentenceSingular( "SC_QUESTION11", 4, VOL_NORM, ATTN_NORM );break;
	case 6:PlaySentenceSingular( "SC_QUESTION16", 4, VOL_NORM, ATTN_NORM );break;
	case 7:PlaySentenceSingular( "SC_QUESTION17", 4, VOL_NORM, ATTN_NORM );break;
	case 8:PlaySentenceSingular( "SC_QUESTION18", 4, VOL_NORM, ATTN_NORM );break;
	case 9:PlaySentenceSingular( "SC_QUESTION22", 4, VOL_NORM, ATTN_NORM );break;
	case 10:PlaySentenceSingular( "SC_IDLE3", 4, VOL_NORM, ATTN_NORM );break;
	case 11:PlaySentenceSingular( "SC_IDLE4", 4, VOL_NORM, ATTN_NORM );break;
	case 12:PlaySentenceSingular( "SC_IDLE5", 4, VOL_NORM, ATTN_NORM );break;
	case 13:PlaySentenceSingular( "SC_IDLE11", 4, VOL_NORM, ATTN_NORM );break;
	case 14:PlaySentenceSingular( "SC_IDLE13", 4, VOL_NORM, ATTN_NORM );break;
	case 15:PlaySentenceSingular( "SC_MONST1", 4, VOL_NORM, ATTN_NORM );break;
	case 16:PlaySentenceSingular( "SC_SMELL2", 4, VOL_NORM, ATTN_NORM );break;
	case 17:PlaySentenceSingular( "SC_SMELL3", 4, VOL_NORM, ATTN_NORM );break;
	case 18:PlaySentenceSingular("SC_PIDLE5", 4, VOL_NORM, ATTN_NORM);break;
	default:break;
	}//END OF switch
}//END OF SayNearPassive


void CScientist::SayNearCautious(void){
	switch(RANDOM_LONG(0, 23)){
	case 0:PlaySentenceSingular( "SC_HEAR0", 4, VOL_NORM, ATTN_NORM );break;
	case 1:PlaySentenceSingular( "SC_HEAR1", 4, VOL_NORM, ATTN_NORM );break;
	case 2:PlaySentenceSingular( "SC_HEAR2", 4, VOL_NORM, ATTN_NORM );break;
	case 3:PlaySentenceSingular( "SC_FEAR0", 4, VOL_NORM, ATTN_NORM );break;
	case 4:PlaySentenceSingular( "SC_FEAR1", 4, VOL_NORM, ATTN_NORM );break;
	case 5:PlaySentenceSingular( "SC_FEAR2", 4, VOL_NORM, ATTN_NORM );break;
	case 6:PlaySentenceSingular( "SC_FEAR4", 4, VOL_NORM, ATTN_NORM );break;
	case 7:PlaySentenceSingular( "SC_FEAR5", 4, VOL_NORM, ATTN_NORM );break;
	case 8:PlaySentenceSingular( "SC_FEAR6", 4, VOL_NORM, ATTN_NORM );break;
	case 9:PlaySentenceSingular( "SC_FEAR7", 4, VOL_NORM, ATTN_NORM );break;
	case 10:PlaySentenceSingular( "SC_FEAR8", 4, VOL_NORM, ATTN_NORM );break;
	case 11:PlaySentenceSingular( "SC_FEAR9", 4, VOL_NORM, ATTN_NORM );break;
	case 12:PlaySentenceSingular( "SC_FEAR10", 4, VOL_NORM, ATTN_NORM );break;
	case 13:PlaySentenceSingular( "SC_FEAR11", 4, VOL_NORM, ATTN_NORM );break;
	case 14:PlaySentenceSingular( "SC_FEAR12", 4, VOL_NORM, ATTN_NORM );break;
	case 15:PlaySentenceSingular( "SC_SCREAM1", 4, VOL_NORM, ATTN_NORM );break;
	case 16:PlaySentenceSingular( "SC_SCREAM3", 4, VOL_NORM, ATTN_NORM );break;
	case 17:PlaySentenceSingular( "SC_SCREAM4", 4, VOL_NORM, ATTN_NORM );break;
	case 18:PlaySentenceSingular( "SC_SCREAM5", 4, VOL_NORM, ATTN_NORM );break;
	case 19:PlaySentenceSingular( "SC_SCREAM10", 4, VOL_NORM, ATTN_NORM );break;
	case 20:PlaySentenceSingular( "SC_SCREAM11", 4, VOL_NORM, ATTN_NORM );break;
	case 21:PlaySentenceSingular( "SC_QUESTION4", 4, VOL_NORM, ATTN_NORM );break;
	case 22:PlaySentenceSingular( "SC_QUESTION5", 4, VOL_NORM, ATTN_NORM );break;
	case 23:PlaySentenceSingular( "SC_IDLE13", 4, VOL_NORM, ATTN_NORM );break;
	default:break;
	}//END OF switch

}//END OF SayNearCautious


void CScientist::DeclineFollowing( void )
{
	recentDeclines++;
	if (recentDeclines < 30) {
		recentDeclinesForgetTime = gpGlobals->time + 12;
	}
	else {
		recentDeclinesForgetTime = gpGlobals->time + 50;
	}

}


void CScientist::Scream( void )
{
	// Nope!  Use the custom cooldown.  Who politely declines screaming because someone else is talking anyway,
	// if it's something worth screaming at?
	// ...eh, just go ahead, not worth being interruptive or spammy a lot like it would be.
	if (FOkToSpeak()) {
		if (gpGlobals->time >= screamCooldown)
		{
			screamCooldown = gpGlobals->time + RANDOM_FLOAT(6, 20);
			//Talk( 10 );
			m_hTalkTarget = m_hEnemy;
			PlaySentence( "SC_SCREAM", RANDOM_FLOAT(3, 6), VOL_NORM, ATTN_NORM );
		}
	}
}

Activity CScientist::GetStoppedActivity( void )
{
	/*
	if (m_Activity == ACT_WALK || m_Activity == ACT_RUN) {
		//force a reset
		m_Activity = ACT_RESET;
	}
	*/

	//MODDD - if following, don't do this.
	// ALSO, don't do this fresh from another punch.
	if(m_hTargetEnt==NULL){
		if ( m_hEnemy != NULL )
			return ACT_EXCITED;

	}
	return CTalkMonster::GetStoppedActivity();
}


void CScientist::StartTask( Task_t *pTask )
{
	float dist;
	BOOL decidedToFight;
	BOOL decidedToRun;

	//"m_iScheduleIndex" is what task we picked of those in the current schedule.
	//easyForcePrintLine("StartTask sched:%s: task:%d index:%d", getScheduleName(), pTask->iTask, m_iScheduleIndex);

	/*
	if(m_hTargetEnt == NULL){
			TaskFail();
			return;
		}
		*/
	//Also I meant to ask about the chumtoad spawning, trying to avoid being able to spawn on walls where it slowly falls through the world. If there's a location where it looks clearly open but won't spawn, see what lines turn red.

	//easyPrintLine("I WILL give a report of the sci sched %s::%d", m_pSchedule->pName, pTask->iTask);

	switch( pTask->iTask )
	{
	case TASK_PLAY_KNEEL_SEQUENCE:
		SetSequenceByName("kneel");
		kneelSoundDelay = gpGlobals->time + RANDOM_FLOAT(2.1, 2.7);
	break;
	case TASK_PLAY_SEQUENCE_FACE_ENEMY:
	case TASK_PLAY_SEQUENCE_FACE_TARGET:
	case TASK_PLAY_SEQUENCE:
		easyPrintLine("sci, PLAY_SEQ: WHAT THE hey %.2f", pTask->flData);
		if(pTask->flData == ACT_EXCITED || pTask->flData == ACT_CROUCH || pTask->flData == ACT_CROUCHIDLE){
			//if(gpGlobals->time > playFearAnimCooldown ){
				//allowed.
				easyPrintLine("sci, PLAY_SEQ: I ALLOWED YOU!!! pfac:%.2f ct:%.2f", playFearAnimCooldown, gpGlobals->time);
				playFearAnimCooldown = gpGlobals->time + 14;
			//}else{
			//	//don't try to play this anim, it may get spammed and make the scientist even more useless than it already is.
			//	easyPrintLine("I BLOCKED YOU!!! pfac:%.2f ct:%.2f", playFearAnimCooldown, gpGlobals->time);
			//	TaskComplete();
			//	break;
			//}
		}

		if (pTask->flData == ACT_ARM ) {
			if (GetBodygroup(BODYGROUP_NEEDLE) == NEEDLE_ON) {
				// already have the needle out?  Skip this step.
				TaskComplete();
				break;
			}
			else {
				// Getting the syringe out? Then look at who we're supposed to be healing and say the line.
				if (m_hTargetEnt == NULL) {
					TaskFail();
					return;
				}
				//		if ( FOkToSpeak() )
				m_hTalkTarget = m_hTargetEnt;
				PlaySentence("SC_HEAL", 4, VOL_NORM, ATTN_IDLE);
			}
		}



		//otherwise base behavior is good.
		CTalkMonster::StartTask(pTask);
	break;
	case TASK_SCIENTIST_ANGRY_CHASE_ENEMY_FAILED:
		//just a notice to turn aggro off, if we can't even make it to our enemy.
		aggro = 0;
		// ...what.
		m_movementGoal = MOVEGOAL_NONE;
		TaskComplete();
		return;
	break;
	case TASK_CANT_FOLLOW:
		//MODDD - WARNING: sensitive script.  Let's be careful about this...
		if(m_hTargetEnt == NULL){
			TaskFail();
			return;
		}

		if(healNPCChosen){
			//just give up and re-check later...
			forgetHealNPC();
			healNPCCheckDelay = gpGlobals->time + 6;
			TaskFail();
			return;
		}else{
			if(m_hTargetEnt != NULL){
				CTalkMonster::StartTask( pTask );
			}
		}
		
		if(m_hTargetEnt == NULL){
			TaskFail();
			return;
		}
	break;
	case TASK_SAY_HEAL:


		/*
		//NOTICE - task dummied out!  Say "SC_HEAL" only on getting the needle out instead,
		// to avoid spam from rapidly getting close to / away from the scientist
		//MODDD
		if(m_hTargetEnt == NULL){
			TaskFail();
			return;
		}

//		if ( FOkToSpeak() )
		m_hTalkTarget = m_hTargetEnt;
		PlaySentence( "SC_HEAL", 4, VOL_NORM, ATTN_IDLE );
		*/

		TaskComplete();
	break;
	case TASK_SCIENTIST_FIGHT_OR_FLIGHT:
		//if 0, this isn't the "panic".  This replaces the scream for panic, so only scream for panic.
		if(pTask->flData == 1){
			Scream();
		}

		if(EASY_CVAR_GET_DEBUGONLY(scientistBravery) > 0){

			decidedToFight = FALSE;
			decidedToRun = FALSE;

			if (m_hEnemy != NULL) {

				//in panic-mode, we can't wuss out.
				if (aggro >= 0 || aggro == -0.5 || pTask->flData == 1) {

					aggro = 0;

					float vertDiff = fabs(m_hEnemy->pev->origin.z - pev->origin.z);

					//elevation differences?  Nope, none of that.
					if (m_hEnemy != NULL && (vertDiff < 120)) {
						dist = (m_hEnemy->pev->origin - this->pev->origin).Length();

						if (pTask->flData == 1) {
							//much easier to tick off now.
							dist *= 0.5;
						}
						if (aggro == -0.5) {
							//tighten it instead..
							dist *= 1.7;
						}
						//...
						Vector tempEnBoundDelta = (m_hEnemy->pev->absmax - m_hEnemy->pev->absmin);
						float tempEnSize = tempEnBoundDelta.x * tempEnBoundDelta.y * tempEnBoundDelta.z;

						//tempEnSize   scale:   under 200... not so important.

						float fightOddsInfluence = 1.05;
						float fearFactor = 0;

						//size of hgrunt, maybe?: 73728.
						//size of garg?   5478400.

						if (tempEnSize < 16000) {  //headcrab size: 13824
							//no change.

						}else if (tempEnSize <= 500000) {  //size of agrunt: about 348160
						   //less likely to fight, the more out of our comfort zone this is.

							fearFactor = ((tempEnSize - 16000) / (500000 - 16000));
							dist *= (1 + fearFactor);

							fightOddsInfluence = (1 - fearFactor + 0.05f);

						}else {
							// OH GOD ITS HUGE
							dist *= 7;
							fightOddsInfluence = 0.01f;
						}

						//easyPrintLine("WHATS THE SIZE   %.2f %.2f %.2f %.2f", tempEnSize, dist, fearFactor, fightOddsInfluence);
						//easyPrintLine("kk");

						if (dist < 85) {
							//fight!
							decidedToFight = TRUE;
						}
						else if (dist < 160) {
							if (aggro == -0.5) {
								decidedToFight = (RANDOM_FLOAT(0, 1) < 0.38 * fightOddsInfluence);
							}
							else {
								decidedToFight = (RANDOM_FLOAT(0, 1) < 0.7 * fightOddsInfluence);
							}

							if (!decidedToFight && pTask->flData == 0) {
								//"pTask->flData" being 1 means running is not an option... sit and drool I guess.
								decidedToRun = TRUE;
							}
						}
					}
				}
				else if (aggro == -1) {
					decidedToRun = TRUE;
					//FLEE!
				}

			}
			else {
				// has no enemy?  hm.
				decidedToRun = TRUE;
			}

			//easyPrintLine("MY CHOICE?  aggro:%.2f pTask->flData:%.2f :::decidedToFight:%d decidedToRun:%d", aggro, pTask->flData, decidedToFight, decidedToRun);


			// whoops.
			//decidedToFight = TRUE;

			if(decidedToFight == TRUE){
				aggro = 1;
				aggroOrigin = pev->origin;
				aggroCooldown = gpGlobals->time + 2;
				ChangeSchedule(this->GetScheduleOfType(SCHED_SCIENTIST_ANGRY_CHASE_ENEMY) );
				break;
			}else{
				aggro = 0;

				if (pTask->flData != 1) {
					ChangeSchedule(slScientistCover);
				}
				else {
					// we already tried finding cover to get this point.  GIVE UP, just play the panic anim as this schedule would.
					TaskComplete();
				}
				break;
			}
		}

		aggro = 0;
		TaskComplete();
	break;
	case TASK_SCREAM:
		Scream();
		TaskComplete();
	break;
	case TASK_RANDOM_SCREAM:
		if (RANDOM_FLOAT(0, 1) < pTask->flData) {
			Scream();
		}
		TaskComplete();
	break;
	case TASK_SAY_FEAR:

		//no, go ahead!
		// yea no, this is too spammy
		if ( FOkToSpeak() )
		{
			SayFear();
		}
		TaskComplete();
	break;
	case TASK_HEAL:
		m_IdealActivity = ACT_MELEE_ATTACK1;
	break;
	case TASK_RUN_PATH_SCARED:
	{
		// HOLD UP.  If we can't even navigate to the first node, it's all over before it began.
		// No, too crude of a check.  Other ways a path can work out but I forget what exactly.
		// FUCK IT


		////Vector vecGoal = m_Route[0].vecLocation;
		//Vector vecGoal;
		//WayPoint_t* test = GetGoalNode();
		//if (test != NULL) {
		//	vecGoal = test->vecLocation;


		//	if (CheckLocalMove(pev->origin, vecGoal, NULL, NULL) != LOCALMOVE_VALID) {
		//		TaskFail();
		//		return;
		//	}
			
		if (!MovementIsComplete()) {
			// okay?  How about this way?
			m_movementActivity = ACT_RUN_SCARED;
		}
		else {
			TaskFail();
		}

		//}
		//else {
		//	// ????????????????
		//	// Eh, go ahead and count as 'success', either way this leads into a new schedule.
		//	// Lacking a node at all likely means the pathfinding was skipped from being
		//	// close enough to the goal already.
		//	// or not?
		//	TaskFail();
		//	//TaskComplete();
		//}

	}
	break;
	case TASK_MOVE_TO_TARGET_RANGE_SCARED:
	{
		//MODDD
		if(m_hTargetEnt == NULL){
			TaskFail();
			return;
		}

		if ( (m_hTargetEnt->pev->origin - pev->origin).Length() < 1 )
			TaskComplete();
		else
		{
			m_vecMoveGoal = m_hTargetEnt->pev->origin;
			if ( !MoveToTarget( ACT_WALK_SCARED, 0.5 ) )
				TaskFail();
		}
	}
	break;
	case TASK_MELEE_ATTACK2_NOTURN:
	case TASK_MELEE_ATTACK2:
	{
		//MODDD - overriding what the base schedule does.
		// All the commented stuff is now not.
		m_Activity = ACT_RESET;  //force me to re-get this!
		m_fSequenceFinished = FALSE;
		pev->frame = 0;

		m_IdealActivity = ACT_MELEE_ATTACK2;
		this->signalActivityUpdate = TRUE;
	}
	break;
	default:
		CTalkMonster::StartTask( pTask );
	break;
	}
}

void CScientist::RunTask( Task_t *pTask )
{

	//MODDD
	/*
	if(m_hTargetEnt == NULL){
		TaskFail();
		return;
	}else{
					
		easyPrintLine("STUUUUFFFFFF %d   %s::%d", monsterID, m_hTargetEnt->getClassname(), m_hTargetEnt->MyMonsterPointer()->monsterID );
	}
	*/

	switch ( pTask->iTask )
	{
	case TASK_RUN_PATH_SCARED:
		if ( MovementIsComplete() )
			TaskComplete();
		if ( RANDOM_LONG(0,31) < 8 )
			Scream();
	break;



	case TASK_MOVE_TO_TARGET_RANGE: {
		//MODDD - intervention.


		if (pTask->flData <= 60) {
			// can do an extra check for very short distance checks to avoid stupidly running against my target.
			if (m_hTargetEnt != NULL) {
				float dist_3D = Distance(pev->origin, m_hTargetEnt->pev->origin);
				if (dist_3D < pTask->flData * 1.2) {
					// if the 2D distance is good enough, just say 'yes' already.
					//Vector origin2D
					float dist_2D = Distance2D(pev->origin, m_hTargetEnt->pev->origin);
					if (dist_2D < pTask->flData) {
						// If the 2D distance is good even though the 3D is a little off, just go ahead and pass.
						TaskComplete();

						// OHHHHH IF YOU FORGET THIS EVER AGAIN IM GONNA <redacted in respect of the Geneva convention>
						RouteClear();		// Stop moving
						break;
					}
				}
			}
			else {
				//easyForcePrintLine("W H A T");
			}
		}

		CTalkMonster::RunTask(pTask);
	}
	break;
	case TASK_MOVE_TO_TARGET_RANGE_SCARED:
		{
			if ( RANDOM_LONG(0,63)< 8 )
				Scream();

			if ( m_hEnemy == NULL )
			{
				TaskFail();
			}
			else
			{
				//MODDD
				if(m_hTargetEnt == NULL){
					TaskFail();
					return;
				}


				float distance;

				distance = ( m_vecMoveGoal - pev->origin ).Length2D();
				// Re-evaluate when you think your finished, or the target has moved too far
				if ( (distance < pTask->flData) || (m_vecMoveGoal - m_hTargetEnt->pev->origin).Length() > pTask->flData * 0.5 )
				{
					m_vecMoveGoal = m_hTargetEnt->pev->origin;
					distance = ( m_vecMoveGoal - pev->origin ).Length2D();
					FRefreshRoute();
				}

				// Set the appropriate activity based on an overlapping range
				// overlap the range to prevent oscillation
				if ( distance < pTask->flData )
				{
					TaskComplete();
					RouteClear();		// Stop moving
				}
				else if ( distance < 190 && m_movementActivity != ACT_WALK_SCARED )
					m_movementActivity = ACT_WALK_SCARED;
				else if ( distance >= 270 && m_movementActivity != ACT_RUN_SCARED )
					m_movementActivity = ACT_RUN_SCARED;
			}
		}
		break;

	case TASK_HEAL:
		// I can proceed for two reasons.
		// Animation finished (healed), or target got too far away.
		if ( m_fSequenceFinished )
		{
			TaskComplete();
		}
		else
		{
			//MODDD - IMPORTANT NOW!
			if(m_hTargetEnt == NULL){
				return;
			}

			if (TargetDistance() > 90) {
				// too far away?  Give up.
				TaskComplete();
			}
			pev->ideal_yaw = UTIL_VecToYaw( m_hTargetEnt->pev->origin - pev->origin );
			ChangeYaw( pev->yaw_speed );
		}
		break;

	case TASK_WAIT_FOR_SCRIPT:
		//MODDD - small chance of FIdleSpeak each frame.

		// MONSTERSTATE_SCRIPT, assumption?
		if (m_pCine == NULL || m_pCine->CanInterrupt()) {

			if (gpGlobals->time >= nextRandomSpeakCheck) {
				if (RANDOM_LONG(0, 120) < 2) {
					// It may not sound like much, but that's a little under 2% chance every half-second, 4% a second.
					// 10 seconds, 40%.  20 seconds, 80%.
					FIdleSpeak();
				}
				nextRandomSpeakCheck = gpGlobals->time + 0.5;
			}
		}
		CTalkMonster::RunTask(pTask);
	break;


	default:
		CTalkMonster::RunTask( pTask );
		break;
	}
}

//=========================================================
// Classify - indicates this monster's place in the
// relationship table.
//=========================================================
int CScientist::Classify ( void )
{
	return	CLASS_HUMAN_PASSIVE;
}

//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CScientist::SetYawSpeed ( void )
{
	int ys;
	ys = 90;

	//MODDD - let state influence the turn speeds mainly, but some others a little
	if (m_MonsterState != MONSTERSTATE_COMBAT) {
		// not combat?  Normal values
		switch (m_Activity)
		{
		case ACT_IDLE:
			ys = 120;
			break;
		case ACT_WALK:
			ys = 180;
			break;
		case ACT_RUN:
			ys = 150;
			break;
		case ACT_TURN_LEFT:
		case ACT_TURN_RIGHT:
			ys = 120;
			break;
		}
	}
	else {
		// Combat?
		// I gotta go fast!   At least improve the slow turnleft/right speeds, c'mon, we are panicking here.
		// and new default for not specified here (although barney's is 0, maybe this never gets used):
		ys = 140;
		switch (m_Activity)
		{
		case ACT_IDLE:
			ys = 150;
			break;
		case ACT_WALK:
			ys = 180;
			break;
		case ACT_RUN:
			ys = 160;
			break;
		case ACT_TURN_LEFT:
		case ACT_TURN_RIGHT:
			ys = 180;
			break;
		}
	}

	pev->yaw_speed = ys;
}

//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CScientist::HandleAnimEvent( MonsterEvent_t *pEvent )
{
	//MODDD - COUNTREMOVAL.  Count is not consistent across machines.  Scrapped.
	// Let's try this again sometime.
	//const int NUM_SCIENTIST_HEADS_MODEL = this->numberOfModelBodyParts+1;
	const int NUM_SCIENTIST_HEADS_MODEL = 3;

	int oldBody;

	switch( pEvent->event ){
	case SCIENTIST_AE_HEAL:		// Heal my target (if within range)
	{
		Heal();
		//re-pick a new TargetEnt.
		forgetHealNPC();
		break;
	}
	case SCIENTIST_AE_NEEDLEON:
	{
		SetBodygroup(BODYGROUP_NEEDLE, NEEDLE_ON);

		/*
		oldBody = pev->body;
		//easyPrintLine("OLD BODY1 %d %d", oldBody, pev->body);

		if(NUM_SCIENTIST_HEADS_MODEL > 1){
			pev->body = (oldBody % (NUM_SCIENTIST_HEADS_MODEL) ) + (NUM_SCIENTIST_HEADS_MODEL) * 1;
		}else{
			//no others, just 0.
			pev->body = 0;
		}
		*/

		//easyPrintLine("NEW BODY1 %d", pev->body);
		break;
	}
	case SCIENTIST_AE_NEEDLEOFF:
	{
		SetBodygroup(BODYGROUP_NEEDLE, NEEDLE_OFF);

		/*
		oldBody = pev->body;
		//easyPrintLine("OLD BODY2 %d %d", oldBody, pev->body);
		if(NUM_SCIENTIST_HEADS_MODEL > 1){
			pev->body = (oldBody % (NUM_SCIENTIST_HEADS_MODEL) ) + (NUM_SCIENTIST_HEADS_MODEL) * 0;
		}else{
			pev->body = 0;
		}
		*/

		//easyPrintLine("NEW BODY2 %d", pev->body);
		break;
	}
	default:
		CTalkMonster::HandleAnimEvent( pEvent );
	}//END OF switch(event)
}


//MODDD - some common logic for CScientist and similar classes for determining the head to use at spawn.
// Keeps out invalid choices for pev->body (head choice) or forces randomization if called for.
void scientistHeadFilter( CBaseMonster& somePerson, int arg_numberOfModelBodyParts, int* arg_pTrueBody){
	// 0 no longer happens naturally (all pev->body values as-is shifted up by 1), that means to randomize it.
	if ( *arg_pTrueBody == 0 ){
		*arg_pTrueBody = RANDOM_LONG(1, 3);
	}
	if(*arg_pTrueBody > 3){
		// so what do we do?  randomize it, or just force it something?
		// Forcing 0 for now.
		somePerson.pev->body = 0;
		easyForcePrintLine("SCIENTIST: BAD HEAD VALUE? body:%d arg_pTrueBody:%d", somePerson.pev->body, *arg_pTrueBody);
	}else{
		// sets the head, like SetBodygroup(BODYGROUP_HEAD, ...);
		somePerson.pev->body = *arg_pTrueBody-1;
	}
}

BOOL CScientist::getGermanModelRequirement(void){
	return globalPSEUDO_germanModel_scientistFound;
}
const char* CScientist::getGermanModel(void){
	return "models/g_scientist.mdl";
}
const char* CScientist::getNormalModel(void){
	return "models/scientist.mdl";
}

//MODDD - note that "CSittingScientist" inherits from CScientist, so this also carries over to there if left unspecified for it.
void CScientist::setModel(void){
	CScientist::setModel(NULL);
}
void CScientist::setModel(const char* m){

	//easyPrintLine("NO!!!!!!!!! PLEASE!!!!!!!!!!!!!!!!!!!!!!!!!! %d", (int)EASY_CVAR_GET(scientistModel));
	//easyPrintLine("Huh?? %.2f", CVAR_GET_FLOAT("scientistModel"));

	// let this handle model management.
	CTalkMonster::setModel(m);

	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);
	if(numberOfModelBodyParts==-1)numberOfModelBodyParts = getNumberOfBodyParts();
	//easyPrintLine("BOOT: %d", numberOfModelBodyParts);

	// It is a bad idea to depend on saved things in here like "trueBody". It might not have loaded yet and so isn't reliable, it would be better to hook this at the end of Restore.
	//scientistHeadFilter(*this, numberOfModelBodyParts, &trueBody);

	if (g_scientist_crouchstand_sequenceID == -1) {
		g_scientist_crouchstand_sequenceID = LookupSequence("crouchstand");
	}
	if (g_scientist_checktie_sequenceID == -1) {
		g_scientist_checktie_sequenceID = LookupSequence("checktie");
	}
	
}





int CScientist::IRelationship(CBaseEntity* pTarget)
{
	if (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(thatWasntPunch) == 1) {
		// no damns given
		return R_NO;
	}

	/*
	//Moved to TalkMonster's.
	//MODDD TODO - for provokable but unprovoked things, maybe make Barnies point their guns and stare at it when not following, or scientist do a fear anim while staring at it?
	if(pTarget->isProvokable() && !pTarget->isProvoked() ){
		//I have no reason to pick a fight with this unprovoked, neutral enemy.
		return R_NO;
	}
	*/

	return CTalkMonster::IRelationship(pTarget);
}


//MODDD - new.
void CScientist::Activate( void ){
	/*
	easyPrintLine("I AMA %d %d", pev->body, trueBody);
	//!setModelCustom();
	*/
	////TalkInit();

	CTalkMonster::Activate();
}

//=========================================================
// Spawn
//=========================================================
void CScientist::Spawn( void )
{
	Precache( );

	//easyPrintLine("I AMS %d %d", pev->body, trueBody);
	if(spawnedDynamically){
		// signal a randomization if spawned by the player.
		//pev->body = -1;    no need, getting overridden soon anyway
		trueBody = 0;
	}else{
		// leave "pev->body" to whatever the map made it
		trueBody = pev->body + 1;
	}

	CTalkMonster::Spawn();


	setModel(); //"models/scientist.mdl"  //argument unused for monsters with German versions. 
	//!setModelCustom();
	
	scientistHeadFilter(*this, numberOfModelBodyParts, &trueBody);

	//HERE instead.
	////TalkInit();


	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	pev->classname = MAKE_STRING("monster_scientist");

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor = BloodColorRedFilter();
	pev->health			= gSkillData.scientistHealth;
	pev->view_ofs		= Vector ( 0, 0, 50 );// position of the eyes relative to monster's origin.
	m_flFieldOfView		= VIEW_FIELD_WIDE; // NOTE: we need a wide field of view so scientists will notice player and say hello
	m_MonsterState		= MONSTERSTATE_NONE;

//	m_flDistTooFar		= 256.0;

	m_afCapability		= bits_CAP_HEAR | bits_CAP_TURN_HEAD | bits_CAP_OPEN_DOORS | bits_CAP_AUTO_DOORS | bits_CAP_USE;

	// White hands
	//pev->skin = 0;

	//scientistHeadFilter(*this, numberOfModelBodyParts);

	//pev->body = 0;
	//pev->modelindex = 137;
	//easyPrintLine("WHAT IS SCIENTIST MODEL %d", pev->modelindex );
	//scientist: 95
	//barney: 137

	MonsterInit();

	pev->skin = 0; //default.

	//if( (pev->spawnflags & SF_MONSTER_TALKMONSTER_BLOODY) && EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(sv_germancensorship) != 1 && EASY_CVAR_GET(scientistModel) < 2){
	if( (pev->spawnflags & SF_MONSTER_TALKMONSTER_BLOODY) && EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(sv_germancensorship) != 1){
		pev->skin = 1;
		
		if(EASY_CVAR_GET_DEBUGONLY(monsterSpawnPrintout) == 1){
			easyPrintLine("SCIHEAD: BLOODY CORPSE FLAG UNDERSTOOD!!!");
		}

		//if this spawn flag is set, start with the bloody skin.
	}

	if(EASY_CVAR_GET_DEBUGONLY(monsterSpawnPrintout) == 1){
		easyPrintLine("SCIHEAD: FINAL BODY: %d SKIN: %d", pev->body, pev->skin);
		easyPrintLine("SCIHEAD: COUNTPOST: %d", getNumberOfBodyParts( ) );
	}

	SetUse( &CTalkMonster::FollowerUse );
}



/*
// OLD - mouth-force debugging, see studiomodelrenderer.cpp for what does most of the work.
void CScientist::Spawn(void)
{
	setModel();


	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	pev->classname = MAKE_STRING("monster_scientist");

	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	m_bloodColor = BloodColorRedFilter();
	pev->health = gSkillData.scientistHealth;
	pev->view_ofs = Vector(0, 0, 50);// position of the eyes relative to monster's origin.
	m_flFieldOfView = VIEW_FIELD_WIDE; // NOTE: we need a wide field of view so scientists will notice player and say hello
	m_MonsterState = MONSTERSTATE_NONE;

	SetUse(&CTalkMonster::FollowerUse);

	m_voicePitch = 100;


	m_szGrp[TLK_ANSWER] = "SC_ANSWER";
	m_szGrp[TLK_QUESTION] = "SC_QUESTION";
	m_szGrp[TLK_IDLE] = "SC_IDLE";
	m_szGrp[TLK_STARE] = "SC_STARE";
	m_szGrp[TLK_USE] = "SC_OK";
	m_szGrp[TLK_UNUSE] = "SC_WAIT";
	m_szGrp[TLK_STOP] = "SC_STOP";
	m_szGrp[TLK_NOSHOOT] = "SC_SCARED";
	m_szGrp[TLK_HELLO] = "SC_HELLO";

	//SetThink(&CScientist::MonsterThink);
	SetThink(&CScientist::PeePee);
	pev->nextthink = gpGlobals->time + 0.1;
}

void CScientist::PeePee(void) {
	SetThink(&CScientist::PeePee);
	pev->nextthink = gpGlobals->time + 0.1;

	//SetBoneController(0, 0);
	//SetBoneController(1, 0);
	///SetBoneController(2, 0);
	///SetBoneController(3, 45);
	//SetBoneController(4, 40);

	//pev->controller[0] = 255;
	//pev->controller[1] = 255;
	//pev->controller[2] = 255;
	pev->controller[3] = 255;
	//pev->controller[4] = 255;
	//pev->blending[1] = 255;
}
*/






extern int global_useSentenceSave;
//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CScientist::Precache( void )
{

	//MODDD - LAZY RESETS.  Proper would be in world's precache!
	g_scientist_PredisasterSuitMentionAllowedTime = -1;
	g_scientist_HeadcrabMentionAllowedTime = -1;


	PRECACHE_MODEL("models/scientist.mdl");
	//PRECACHE_MODEL("models/scientist_pre_e3.mdl");
	//PRECACHE_MODEL("models/scientist_e3.mdl");

	global_useSentenceSave = TRUE;
	PRECACHE_SOUND("scientist/sci_pain1.wav");
	PRECACHE_SOUND("scientist/sci_pain2.wav");
	PRECACHE_SOUND("scientist/sci_pain3.wav");
	PRECACHE_SOUND("scientist/sci_pain4.wav");
	PRECACHE_SOUND("scientist/sci_pain5.wav");

	PRECACHE_SOUND_ARRAY(pDeathSounds);

	//MODDD - can now play "Strike" sounds on hitting something with the kick.  NEW DATA STUFF
	PRECACHE_SOUND("zombie/claw_strike1.wav");
	PRECACHE_SOUND("zombie/claw_strike2.wav");
	PRECACHE_SOUND("zombie/claw_strike3.wav");
	PRECACHE_SOUND("zombie/claw_miss1.wav");
	PRECACHE_SOUND("zombie/claw_miss2.wav");

	global_useSentenceSave = FALSE;
	// every new scientist must call this, otherwise
	// when a level is loaded, nobody will talk (time is reset to 0)

	TalkInit();
	//Just do TalkInit() in spawn.
	//Wait... why were we doing it that way instead of in Preacache like how the Barney does it?
	//Just do it in both places then.

	CTalkMonster::Precache();
}

// Init talk data
void CScientist::TalkInit()
{
	//easyPrintLine("TALKINIT S %d %d", pev->body, trueBody);

	//pev->renderfx |= 128;
	//why 128?  wasn't it 64 for "ISNPC"?   May be redundant with "monsterinit" already doing this, though.
	if(isOrganic()){
		pev->renderfx |= ISNPC;
	}else{
		pev->renderfx |= ISMETALNPC;
	}

	CTalkMonster::TalkInit();

	//MODDD - ...evidently, this method is called BEFORE "Activate".  So, this check must be done here too.

	//setModelCustom();  //!!!???
	//scientistHeadFilter(*this, numberOfModelBodyParts);

	//MODDD
	madSentencesMax = 1;
	madSentences[0] = "SC_POKE";

	//Static now!
	//madInterSentencesMax = 1;
	//madInterSentences[0] = "SC_POKEQ";

	// scientist will try to talk to friends in this order:

	m_szFriends[0] = "monster_scientist";
	m_szFriends[1] = "monster_sitting_scientist";
	m_szFriends[2] = "monster_barney";


	// scientists speach group names (group names are in sentences.txt)

	m_szGrp[TLK_ANSWER]  =	"SC_ANSWER";
	m_szGrp[TLK_QUESTION] =	"SC_QUESTION";
	m_szGrp[TLK_IDLE] =		"SC_IDLE";
	m_szGrp[TLK_STARE] =	"SC_STARE";
	m_szGrp[TLK_USE] =		"SC_OK";
	m_szGrp[TLK_UNUSE] =	"SC_WAIT";
	m_szGrp[TLK_STOP] =		"SC_STOP";
	m_szGrp[TLK_NOSHOOT] =	"SC_SCARED";
	m_szGrp[TLK_HELLO] =	"SC_HELLO";

	m_szGrp[TLK_PLHURT1] =	"!SC_CUREA";
	m_szGrp[TLK_PLHURT2] =	"!SC_CUREB";
	m_szGrp[TLK_PLHURT3] =	"!SC_CUREC";

	m_szGrp[TLK_PHELLO] =	"SC_PHELLO";
	m_szGrp[TLK_PIDLE] =	"SC_PIDLE";
	m_szGrp[TLK_PQUESTION] = "SC_PQUEST";
	m_szGrp[TLK_SMELL] =	"SC_SMELL";

	m_szGrp[TLK_WOUND] =	"SC_WOUND";
	m_szGrp[TLK_MORTAL] =	"SC_MORTAL";

	// get voice for head
	//switch (pev->body % 3)
	//should this use "% 2" instead, as "% 3", leftover from when there were 4 heads, may not be necessary?


	if(numberOfModelBodyParts == 2){

#if headOffsetFix == 0
	switch (pev->body % 3)
	{
	default:
	case HEAD_GLASSES:	m_voicePitch = 100; break;	//glasses
		//used to be 105, we want universal pitch now.
	case HEAD_EINSTEIN: m_voicePitch = 100; break;	//einstein
	//MODDD - removed.
	//case HEAD_LUTHER:	m_voicePitch = 95;  break;	//luther
	case HEAD_SLICK:	m_voicePitch = 100;  break; //egon.  (or Luther in progress).
	}
#else
	switch (pev->body % (NUM_SCIENTIST_HEADS+1)) //as "3" was "4 - 1", or "NUM_SCIENTIST_HEADS - 1".
		//...NO.   I am convinced that it should NOT be one less than NUM_..., because the range
		//of modulus is already  0 to 2nd # - 1.  So, range of  # % 4 is  0 - 3, and # % 3 --> 0 - 2 (desirable).
		//Making it "plus 1" to accept "3".
	{
	default:
	case HEAD_GLASSES:	m_voicePitch = 100; break;	//glasses, head#0
		//used to be 105, we want universal pitch now.
	case HEAD_EINSTEIN: m_voicePitch = 100; break;	//einstein, head#1

	case HEAD_SLICK:	m_voicePitch = 100;  break; //slick, head#3   (2 skipped, but the range of  # % 2 is 0, 1, 2.  Only model references need the authentic number.
	}


#endif

	}else if(numberOfModelBodyParts == 3){

		switch (pev->body % 4)
		{
		default:
			case 0:	m_voicePitch = 105; break;	//glasses
			case 1: m_voicePitch = 100; break;	//einstein
			case 2:	m_voicePitch = 95;  break;	//luther
			case 3:	m_voicePitch = 100;  break; //slick
		}

	}else{
		// otherwise, just work like alpha, pick from one of the voices based on what it would've been (#3 = 0).
		// also note that "trueBody" is always offset by 1.
		switch ((trueBody-1) % 3)
		{
		default:
			case 0:	m_voicePitch = 105; break;
			case 1: m_voicePitch = 100; break;
			case 2:	m_voicePitch = 100;  break;
		}
	}

}




void CScientist::AlertSound(void) {

	if (m_hEnemy != NULL) {
		if (FClassnameIs(m_hEnemy->pev, "monster_headcrab")) {
			if(
				// ALSO, don't say this while running or recently taking damage, the tone of 'why look, another headcrab' isn't
				// really fitting for panic.
				((m_IdealActivity != ACT_RUN && m_IdealActivity != ACT_RUN_SCARED) && m_painTime < gpGlobals->time)
				&&
				(
					(g_scientist_HeadcrabMentionAllowedTime == -1 && FOkToSpeakAllowCombat(CTalkMonster::g_talkWaitTime)) ||
					(FOkToSpeakAllowCombat(g_scientist_HeadcrabMentionAllowedTime) && RANDOM_FLOAT(0, 1) <= 0.87)
				)
			)
			{
				// Why look.  Another headcrab.
				PlaySentenceSingular("SC_MONST0", 4, VOL_NORM, ATTN_NORM);
				g_scientist_HeadcrabMentionAllowedTime = gpGlobals->time + 40;
				return;
			}
			//else if (gpGlobals->time >= CTalkMonster::g_talkWaitTime && FOkToSpeakAllowCombat(g_scientist_HeadcrabMentionAllowedTime - 20)) {
			//	PlaySentence("SC_FEAR", 5, VOL_NORM, ATTN_NORM);
			//}
		}
		

		// not a headcrab or was, but blocked by the headcrab mention allowed time cooldown?  Proceed.
		// any other enemy? check size
		if (FOkToSpeakAllowCombat(CTalkMonster::g_talkWaitTime)) {
			SayAlert();
		}
	}//m_hEnemy check

}//AlertSound




GENERATE_TRACEATTACK_IMPLEMENTATION(CScientist)
{
	GENERATE_TRACEATTACK_PARENT_CALL(CTalkMonster);
}

GENERATE_TAKEDAMAGE_IMPLEMENTATION(CScientist)
{
	BOOL alreadyRunningAway = FALSE;
	int tkdDmgRes;


	if(this->m_MonsterState == MONSTERSTATE_SCRIPT && (m_pCine && !m_pCine->CanInterrupt()) ){
		//Exception. Don't try to tap into our schedules.
		return GENERATE_TAKEDAMAGE_PARENT_CALL(CTalkMonster);
	}


	if(pevInflictor != NULL   ){
		CBaseEntity* entInflictor = CBaseEntity::Instance(pevInflictor);

		if(entInflictor != NULL){
			int rel = IRelationship(entInflictor);
			if(rel > R_NO || rel==R_FR){
				m_fearTime = gpGlobals->time;  //something meant to hit me. 
			}


			//IS THIS LINE OFFENDING?  search for the other example.
			/*
			if(aggro != -1 && aggro < 1 && (m_hEnemy == NULL || m_hEnemy != entInflictor) ){
				//took damage from something other what I expected to run from and I'm not feeling very brave? Hurry up and run away!
				aggro = -1;
				TaskFail();
				ChangeSchedule(slScientistCover);//  ???
				//WARNING - if StopFollowing() below is called, it will ignore ths ScientistCover call. Butu that may be ok, below
				//is called only if the player (FL_CLIENT on the inflictor) attacked this scientist.

				alreadyRunningAway = TRUE;
			}
			*/
		}//END OF entInflictor NULL check
	}//END OF pevInflictor NULL check


	if(!alreadyRunningAway){
		if(aggro > 0 && (flDamage >= 6 || RANDOM_LONG(0, 2) < 2) ){
			//2/3 chance of getting scared fast.  If damage is significant, it is definite.
			
			/*
			//DEBUG - not this time.
			aggro = -1;
			TaskFail();
			ChangeSchedule(slScientistCover);//  ???
			*/
		}
	}

	//MODDD - the base talkmonster now handles this with a little tolerance.
	/*
	if ( pevInflictor && pevInflictor->flags & FL_CLIENT )
	{
		Remember( bits_MEMORY_PROVOKED );
		StopFollowing( TRUE );
	}
	*/

	// make sure friends talk about it if player hurts scientist...
	tkdDmgRes = GENERATE_TAKEDAMAGE_PARENT_CALL(CTalkMonster);

	//MODDD HACKY - don't turn around to see what hit you ever, you're too busy running.
	pev->ideal_yaw = this->pev->angles.y;

	return tkdDmgRes;
}


void CScientist::OnAlerted(BOOL alerterWasKilled) {
	// change schedules this forcibly only the first time we get pissed.
	BOOL alreadyPro = HasMemory(bits_MEMORY_PROVOKED);
	if (!alreadyPro || m_MonsterState == MONSTERSTATE_IDLE || m_MonsterState == MONSTERSTATE_ALERT) {
		if (!alerterWasKilled) {
			ChangeSchedule(slSciFear);
		}
		else {
			ChangeSchedule(slSciPanic);
		}
	}
}



//=========================================================
// ISoundMask - returns a bit mask indicating which types
// of sounds this monster regards. In the base class implementation,
// monsters care about all sounds, but no scents.
//=========================================================
int CScientist::ISoundMask ( void )
{
	return	bits_SOUND_WORLD	|
			bits_SOUND_COMBAT	|
			bits_SOUND_DANGER	|
			bits_SOUND_PLAYER;
}

//=========================================================
// PainSound
//=========================================================
void CScientist::PainSound ( void )
{
	if (gpGlobals->time < m_painTime )
		return;


	// don't let other things in the same entity be so eager to interrupt my pain noises
	CTalkMonster::g_talkWaitTime = gpGlobals->time + RANDOM_FLOAT(2, 3);
	//m_flStopTalkTime = CTalkMonster::g_talkWaitTime;


	//MODDD - these conditions are set before PainSound is called, so we can determine
	if (HasConditions(bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE)) {

		m_painTime = gpGlobals->time + RANDOM_FLOAT(1.8, 2.4);

		if (gpGlobals->time >= g_scientist_sayGetItOffCooldown && m_bitsDamageType & DMG_SLASH && RANDOM_FLOAT(0, 1) < 0.8) {
			g_scientist_sayGetItOffCooldown = gpGlobals->time + 30;
			PlaySentenceSingular("SC_TENT", 5, VOL_NORM, ATTN_NORM);  // get it off, get it off get it OFFF
			m_bitsDamageType &= ~DMG_SLASH;
		}
		else {

			PainSound_Play();
		}
	}
	else {
		// takimg timed damage?  Be less saying 'no', just noises.

		m_painTime = gpGlobals->time + RANDOM_FLOAT(3, 6);

		switch (RANDOM_LONG(0, 2))
		{
		case 0: UTIL_PlaySound(ENT(pev), CHAN_VOICE, "scientist/sci_pain1.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
		case 1: UTIL_PlaySound(ENT(pev), CHAN_VOICE, "scientist/sci_pain4.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
		case 2: UTIL_PlaySound(ENT(pev), CHAN_VOICE, "scientist/sci_pain5.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
		}
	}

}


// for the audio to have its own method.  This has no other conditions.
void CScientist::PainSound_Play(void) {

	switch (RANDOM_LONG(0, 9))
	{
	case 0: UTIL_PlaySound(ENT(pev), CHAN_VOICE, "scientist/sci_pain1.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
	case 1: UTIL_PlaySound(ENT(pev), CHAN_VOICE, "scientist/sci_pain2.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
	case 2: UTIL_PlaySound(ENT(pev), CHAN_VOICE, "scientist/sci_pain3.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
	case 3: UTIL_PlaySound(ENT(pev), CHAN_VOICE, "scientist/sci_pain4.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
	case 4: UTIL_PlaySound(ENT(pev), CHAN_VOICE, "scientist/sci_pain5.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
		//MODDD - new
	case 5: UTIL_PlaySound(ENT(pev), CHAN_VOICE, "scientist/sci_pain6.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
	case 6: UTIL_PlaySound(ENT(pev), CHAN_VOICE, "scientist/sci_pain7.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
	case 7: UTIL_PlaySound(ENT(pev), CHAN_VOICE, "scientist/sci_pain8.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
	case 8: UTIL_PlaySound(ENT(pev), CHAN_VOICE, "scientist/sci_pain9.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
	case 9: UTIL_PlaySound(ENT(pev), CHAN_VOICE, "scientist/sci_pain10.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
	}
}


//=========================================================
// DeathSound
//=========================================================
void CScientist::DeathSound ( void )
{
	//sci_die1
	//PainSound();
	
	if(explodeDelay == -1){
		UTIL_PlaySound( edict(), CHAN_VOICE, RANDOM_SOUND_ARRAY(pDeathSounds), 1.0, ATTN_NORM, 0, GetVoicePitch() );
	}
}


GENERATE_KILLED_IMPLEMENTATION(CScientist)
{
	//test();
	//easyPrintLine("YEAHHHH %d", numberOfModelBodyParts);

	// turn on if the jaw on fresh-killed scientists is wanted?
	//SetBoneController(4, 20);


	if(explodeDelay != -2){
		//not doing that? ok, normal behavior.
		SetUse( NULL );
		GENERATE_KILLED_PARENT_CALL(CTalkMonster);
	}else{
		//The glitch. Skip to calling the base monster, interrupting other talkers reciting lines isn't ok.
		StopTalking();
		SetUse( NULL );
		GENERATE_KILLED_PARENT_CALL(CBaseMonster);
	}
}


void CScientist::SetActivity ( Activity newActivity )
{
	//int iSequence;
	//float framerateChoice = 1;
	
	//....why?! The base monster's SetActivity will work with this fine.
	//iSequence = LookupActivity ( newActivity );
	
	//if(monsterID==2)easyForcePrintLine("IT IS I WHO NUTS TO THAT ID:%d befr act:%d seq:%d fr:%.2f", monsterID, newActivity, iSequence, pev->frame);
	
	CTalkMonster::SetActivity( newActivity );

	//if(monsterID==2)easyForcePrintLine("IT IS I WHO NUTS TO THAT ID:%d aftr act:%d seq:%d fr:%.2f", monsterID, newActivity, pev->sequence, pev->frame);

	// Set to the desired anim, or default anim if the desired is not present
	//if ( iSequence == ACTIVITY_NOT_AVAILABLE ){
	//MODDD NOTE - if the sequence is left 0, that means the activity get failed.
	//...it's handled fine the way it is, forget this.
	/*
	if(pev->sequence == 0){
		newActivity = ACT_IDLE;
		framerateChoice = 1;
	}
	*/

}

void CScientist::ReportAIState(void){
	//call the parent, and add on to that.
	CTalkMonster::ReportAIState();

	easyPrintLine("SCIENTIST: Body: %d Truebody: %d Skin: %d aggro: %d", pev->body, trueBody, pev->skin, this->aggro);
}//END OF ReportAIState


Schedule_t* CScientist::GetScheduleOfType ( int Type )
{
	Schedule_t *psched;
	//easyForcePrintLine("IM SCIENTIST:%d AND I PICKED SCHED TYPE %d", monsterID, Type);
	
	//MODDD - is it safe to have this here no matter what?
	//   Note that we have to check to see if the enemy is dead to drop them as an enemy or else the scientist will still be scared of their corpse. Yep, gotta love computers.
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
			//MODDD - shouldn't you clear the conditions here too??
			ClearConditions( bits_COND_ENEMY_DEAD );
			
			//if not already in the ALERT state or tyring to get there, go ahead and be alert & reget your schedule.
			if(this->m_MonsterState != MONSTERSTATE_ALERT || this->m_IdealMonsterState != MONSTERSTATE_ALERT){
				SetState( MONSTERSTATE_ALERT );
				return GetSchedule();
			}
		}
	}

	switch( Type )
	{


	case SCHED_FAIL: {
		// HOLD ON.  If in combat, we'll panic here!
		if (m_MonsterState == MONSTERSTATE_COMBAT) {
			return slSciPanic;
		}

		return slFail;
	}
	case SCHED_FAIL_QUICK: {
		// HOLD ON.  If in combat, we'll panic here!
		if (m_MonsterState == MONSTERSTATE_COMBAT) {
			return slSciPanic;
		}

		return slFailQuick;
	}


	case SCHED_TAKE_COVER_FROM_ENEMY:
		// wait.  This wasn't always here, beeeeeecccccccaaaaaaauuuuussssssseeee?
		aggro = 0;
		return slScientistCover;
	break;
	case SCHED_SCIENTIST_ANGRY_CHASE_ENEMY:	
	//also, in case we pick this schedule from the TASK_FIND_COVER_FROM_ENEMY_OR_FIGHT in schedule.cpp, make sure to boost your aggro so the rest of the AI works right.
	case SCHED_CHASE_ENEMY:
		//MODDD - why was this for MELEE_ATTACK_2 as well? no, this leads into that when we get close enough..

		if(aggro < 1)aggro = 1; //clearly we are mad at the enemy to have called this method.
		return slAngryScientistChaseEnemy;
	break;
	//case SCHED_RANGE_ATTACK1:
	//case SCHED_RANGE_ATTACK2:
	//case SCHED_MELEE_ATTACK1:
	case SCHED_MELEE_ATTACK2:
		return slScientistPunch;
	break;
	case SCHED_SCIENTIST_ANGRY_CHASE_ENEMY_FAILED:
		return slAngryScientistChaseEnemyFailed;
	break;
		//same as below..
	case SCHED_TAKE_COVER_FROM_ORIGIN:
		aggro = 0;
		return CTalkMonster::GetScheduleOfType(SCHED_TAKE_COVER_FROM_ORIGIN);
	break;
		//MODDD - intercept this to add something...
	case SCHED_TAKE_COVER_FROM_BEST_SOUND:
		aggro = 0;
		return CTalkMonster::GetScheduleOfType(SCHED_TAKE_COVER_FROM_BEST_SOUND);
	break;
	// Hook these to make a looping schedule
	case SCHED_TARGET_FACE:
		// call base class default so that scientist will talk
		// when 'used'

		psched = CTalkMonster::GetScheduleOfType(Type);
		if (psched == slIdleStand){
			return slFaceTarget;	// override this for different target face behavior
		}else{
			//otherwise, the TalkMonster told us to talk.
			return psched;
		}

	case SCHED_TARGET_CHASE:
		return slSciFollow;
	case SCHED_CANT_FOLLOW:
		//return slStopFollowing;
		//MODDD - we're going to do this instead.  CTalkMonster has the common method.
		return CTalkMonster::GetScheduleOfType(Type);
	case SCHED_PANIC:
		return slSciPanic;

	case SCHED_TARGET_CHASE_SCARED:
		return slFollowScared;

	case SCHED_TARGET_FACE_SCARED:
		return slFaceTargetScared;
	case SCHED_IDLE_STAND:
		// call base class default so that scientist will talk
		// when standing during idle
		psched = CTalkMonster::GetScheduleOfType(Type);

		if (psched == slIdleStand)
			return slIdleSciStand; //substitution!
		else
			return psched;
	case SCHED_HIDE:
		{
		aggro = 0;
		return slScientistHide;
		break;
		}
	case SCHED_STARTLE:
		{
		aggro = 0;
		return slScientistStartle;
		break;
		}
	case SCHED_FEAR:
		{
		aggro = 0;
		return slSciFear;
		break;
		}
	}
	return CTalkMonster::GetScheduleOfType( Type );
}


void CScientist::ScheduleChange(void){
	forgetHealNPC();

	CTalkMonster::ScheduleChange();
}



void CScientist::OnPlayerDead(CBasePlayer* arg_player){
	CTalkMonster::OnPlayerDead(arg_player);
	// leave most of this up to dead phase
	
	if(!leaderRecentlyDied){
		// And don't make extra noise if this NPC was following the player when it died.
		// Already said something then, this would be redundant.
		//SayFear();
		// eh, do some animation in place, why not.  Unless the player was pretty close.
		if(Distance(pev->origin, arg_player->pev->origin) > 340){
			MakeIdealYaw(arg_player->pev->origin);
			ChangeSchedule(slSciPanicInPlace);
		}else{
			ChangeSchedule(slScientistTakeCoverFromOrigin);
		}
	}
}

void CScientist::OnPlayerFollowingSuddenlyDead(void){
	CTalkMonster::OnPlayerFollowingSuddenlyDead();

	//ChangeSchedule(GetScheduleOfType(SCHED_PANIC));
	ChangeSchedule(slScientistTakeCoverFromOrigin);
	
}


Schedule_t *CScientist::GetSchedule ( void )
{
	//MODDD - is this okay?   This says that, on schedule failure, forget healing.
	forgetHealNPC();

	// do in a different place now
	/*
	//MODDD - new block. If the one I was following recently died, get scared.
	if(leaderRecentlyDied){
		leaderRecentlyDied = FALSE;
		SayLeaderDied();
        StopFollowing( FALSE, FALSE );  //no generic unuse sentence.

		// enter a panic state.
		return GetScheduleOfType(SCHED_PANIC);
	}
	*/


	//return CBaseMonster::GetSchedule();


	
	if (HasConditions(bits_COND_SEE_ENEMY) ){
		m_fearTime = gpGlobals->time; // Update last fear... why not?
	}



	// It's possible to get new enemies really really fast and spam these lines, be careful.
	// And it is correct behavior, bits_COND_NEW_ENEMY is only set in basemonster.cpp on the frame a monster
	// has a different enemy.


	// Be like Barney.  Just have an AlertSound, thanks.
	if (HasConditions(bits_COND_NEW_ENEMY) ) {
		AlertSound();
	}


	while(TRUE){
		if(aggro > 0){
			//if ( HasConditions( bits_COND_HEAR_SOUND ) )
			if(HasConditions(bits_COND_HEAR_SOUND) || HasConditions(bits_COND_SEE_ENEMY) || HasConditions(bits_COND_NEW_ENEMY) ){

				if(m_hEnemy == NULL || (m_hEnemy->pev->origin - this->pev->origin).Length() > 310 ){
					//too far away? no enemy?  screw it.   "break" skips the rest.
					aggro = 0;
					break;
				}else{
					//reset aggro timer.
					aggroCooldown = gpGlobals->time + 2;
				}
			}
			if(gpGlobals->time >= aggroCooldown){
				//not so aggressive now.  Continue with a usual schedule below all this.
				aggro = 0;
			}else{
				//I CAME HERE TO DO RESEARCH AND COLLECT BLOOD SAMPLES... AND IM ALL OUTTA RESEARCH.
				return CBaseMonster::GetSchedule();
			}
		}
		break;
	}//END OF while(TRUE)...  just a procedural loop to be interrupted as needed.

	CBaseEntity *pEntityScan = NULL;
	// so we don't keep calling through the EHANDLE stuff
	CBaseEntity *pEnemy = m_hEnemy;

	if ( HasConditions( bits_COND_HEAR_SOUND ) )
	{
		CSound *pSound;
		pSound = PBestSound();

		ASSERT( pSound != NULL );
		if ( pSound && (pSound->m_iType & bits_SOUND_DANGER) )
			return GetScheduleOfType( SCHED_TAKE_COVER_FROM_BEST_SOUND );
	}



	// If I was interrupted from holding the syringe, put it away
	// TODO!!!
	//if(pev->body)
	// ...

	if (GetBodygroup(BODYGROUP_NEEDLE) == NEEDLE_ON && !CanHeal() ) {
		// Done or change my mind?  Put it back then!  
		return slScientistPutAwaySyringe;
	}






	float thisDistance;
	float leastDistanceYet;
	CBaseMonster* testMon;
	CTalkMonster* thisNameSucks;
	CTalkMonster* bestChoiceYet;

	switch( m_MonsterState )
	{
	case MONSTERSTATE_ALERT:
	case MONSTERSTATE_IDLE:

		//Scream if the party is going hard.
		if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(thatWasntPunch)){
			if ( RANDOM_FLOAT( 0, 1 ) < 0.4 ){
			//PlaySentence( "SC_SCREAM_TRU", 2, VOL_NORM, ATTN_NORM );
			SENTENCEG_PlayRndSz( edict(), "SC_SCREAM_TRU", VOL_NORM, ATTN_IDLE, 0, GetVoicePitch() );
			}
		}
		
		if ( pEnemy )
		{
			if ( HasConditions( bits_COND_SEE_ENEMY ) )
				m_fearTime = gpGlobals->time;
			else if ( DisregardEnemy( pEnemy ) )		// After 15 seconds of being hidden, return to alert
			{
				m_hEnemy = NULL;
				pEnemy = NULL;
				m_IdealMonsterState = MONSTERSTATE_ALERT;  //clearly
				return GetSchedule();  //now try again.
			}
		}

		if ( HasConditions(bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE))
		{
			// flinch if hurt
			//MODDD - Face what inflicted the damage and get scared dangit!  Do like barney now
			// ACTUALLY go a step further.  Jump to the combat state and start running if it's something I hate that attacked me.
			// No need to stupidly face it and then run away screaming, especially since this hit likely made a scream/pain noise anyway
			// ...unfortunately we don't really have a way of telling what entity did the damage leading to LIGHT/HEAVY_DAMAGE.  drat.
			//return GetScheduleOfType( SCHED_SMALL_FLINCH );

			//return GetScheduleOfType(SCHED_ALERT_SMALL_FLINCH);

			// how bout dis
			return GetScheduleOfType(SCHED_TAKE_COVER_FROM_ORIGIN);
		}


		// Cower when you hear something scary
		if ( HasConditions( bits_COND_HEAR_SOUND ) )
		{
			CSound *pSound;
			pSound = PBestSound();

			ASSERT( pSound != NULL );
			if ( pSound )
			{
				if ( pSound->m_iType & (bits_SOUND_DANGER | bits_SOUND_COMBAT) )
				{
					if ( gpGlobals->time - m_fearTime > 3 )	// Only cower every 3 seconds or so
					{
						m_fearTime = gpGlobals->time;		// Update last fear
						return GetScheduleOfType( SCHED_STARTLE );	// This will just duck for a second
					}
				}
			}
		}


		//TASK_FACE_TARGET
		if(healNPCCheckDelay < gpGlobals->time && m_hTargetEnt == NULL && EASY_CVAR_GET_DEBUGONLY(scientistHealNPC) != 0 && m_healTime <= gpGlobals->time){
			//before even doing any checks, require the heal timer to not be in place (delay between healing again, a minute as of writing)
			leastDistanceYet = 999999;  //large, so that the first distance at all is the "best".

			//check for allied NPCs to heal if not following.
			pEntityScan = NULL;

			//does UTIL_MonstersInSphere work?
			while ((pEntityScan = UTIL_FindEntityInSphere( pEntityScan, pev->origin, 800 )) != NULL)
			{
				testMon = pEntityScan->MyMonsterPointer();
				//if(testMon != NULL && testMon->pev != this->pev && ( FClassnameIs(testMon->pev, "monster_scientist") || FClassnameIs(testMon->pev, "monster_barney")  ) ){
				if(testMon != NULL && testMon->pev != this->pev && UTIL_IsAliveEntity(testMon) && testMon->isTalkMonster() ){
					thisDistance = (testMon->pev->origin - pev->origin).Length();
					
					thisNameSucks = static_cast<CTalkMonster*>(testMon);
					
					//only allow one scientist to try to reach this NPC.  That is, this NPC's own "scientistTryingToHealMe" is null, that is.
					if(thisNameSucks != NULL && thisNameSucks->scientistTryingToHealMeEHANDLE == NULL && thisDistance < leastDistanceYet && CanHeal(testMon)){
						//healTargetNPC = testMon;
						m_hTargetEnt = testMon;
						bestChoiceYet = thisNameSucks;
						healNPCChosen = TRUE;
						leastDistanceYet = thisDistance;
						//break;
					}

				}

			}

			if(healNPCChosen){
				//NOT THE thisNameSucks!
				//thisNameSucks->scientistTryingToHealMe = this;
				bestChoiceYet->scientistTryingToHealMe = this;
				bestChoiceYet->scientistTryingToHealMeEHANDLE = this;
			}


		}//END OF healTargetNPC check


		
		//!!

		// Behavior for following the player... OR tracking down an NPC to heal them.
		if ( m_hTargetEnt != NULL && (IsFollowing() || healNPCChosen == TRUE) )
		{
			if(EASY_CVAR_GET_DEBUGONLY(scientistHealNPCDebug) == 1){
				/*
				if(m_hTargetEnt == NULL){
					easyPrintLine("SCI: TARGET ENT: NULL");
				}else{
					easyPrintLine("SCI: TARGET ENT: %s", STRING(m_hTargetEnt->pev->classname) );
				}
				*/
			}


			if ( !m_hTargetEnt->IsAlive() )
			{
				// UNDONE: Comment about the recently dead player here?
				//MODDD - your wish is my command, anonymous dev!

				if(healNPCChosen == FALSE){
				    //this means we were following the player.
					SayLeaderDied();
                    StopFollowing( FALSE, FALSE );  //no generic unuse sentence.

					//enter a panic state.
					return GetScheduleOfType(SCHED_PANIC);
                }else{

                    forgetHealNPC();
                }

				break;
			}

			int relationship = R_NO;

			// Nothing scary, just me and the player
			if ( pEnemy != NULL )
				relationship = IRelationship( pEnemy );



			// UNDONE: Model fear properly, fix R_FR and add multiple levels of fear
			if ( relationship != R_DL && relationship != R_HT )
			{
				float daDistance = TargetDistance();
				// If I'm already close enough to my target
				//MODDD - changed to a little higher than 128 to avoid a possible staring glitch, maybe.
				if ( daDistance <= 140 )
				{
					if ( CanHeal() ){	//Heal opportunistically
						return slHeal;
					}else if(healNPCChosen){
						//can't heal, try to look for another or give up.
						forgetHealNPC();
					}

					if(healNPCChosen == FALSE){
					    // player only.
                        if ( HasConditions( bits_COND_CLIENT_PUSH ) )	// Player wants me to move
                            return GetScheduleOfType( SCHED_MOVE_AWAY_FOLLOW );
					}
				}
				return GetScheduleOfType( SCHED_TARGET_FACE );	// Just face and follow.
			}
			else	// UNDONE: When afraid, scientist won't move out of your way.  Keep This?  If not, write move away scared
			{
				//bits_COND_LIGHT_DAMAGE
				//if ( HasConditions( bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE ) )
				//	return slScientistCover;		// Take Cover

				easyPrintLine("OH dear %d", HasConditions( bits_COND_NEW_ENEMY ));

				if ( HasConditions( bits_COND_NEW_ENEMY ) ) // I just saw something new and scary, react
					return GetScheduleOfType( SCHED_FEAR );					// React to something scary
				return GetScheduleOfType( SCHED_TARGET_FACE_SCARED );	// face and follow, but I'm scared!


			}
		}//END OF IsFollowing OR tracking down NPC to heal



		if ( HasConditions( bits_COND_CLIENT_PUSH ) )	// Player wants me to move
			return GetScheduleOfType( SCHED_MOVE_AWAY );


		// try to say something about smells
		TrySmellTalk();
		break;
	case MONSTERSTATE_COMBAT:

		//MODDD - check present here to.  Why not anyway?
		if (pEnemy)
		{
			if (HasConditions(bits_COND_SEE_ENEMY))
				m_fearTime = gpGlobals->time;
			else if (DisregardEnemy(pEnemy))		// After 15 seconds of being hidden, return to alert
			{
				m_hEnemy = NULL;
				pEnemy = NULL;
			}
		}


		//MODDD - WHY YALL RETURNIN STRAIGHT SCHEDUELS NOWHERE ELSE DOES THIS SHIT DAMN MAN.     DAM.
		if (HasConditions(bits_COND_NEW_ENEMY)) {
			//return slSciFear;					// Point and scream!
			return GetScheduleOfType(SCHED_FEAR);
		}

		if (HasConditions(bits_COND_SEE_ENEMY)) {
			//return slScientistCover;		// Take Cover
			return GetScheduleOfType(SCHED_TAKE_COVER_FROM_ENEMY);
		}

		if (HasConditions(bits_COND_HEAR_SOUND)) {
			//return slTakeCoverFromBestSound;	// Cower and panic from the scary sound!
			return GetScheduleOfType(SCHED_TAKE_COVER_FROM_BEST_SOUND);
		}


		//return slScientistCover;			// Run & Cower
		return GetScheduleOfType(SCHED_TAKE_COVER_FROM_ENEMY);


		break;
	}//END OF Swtich ON monsterstate


	

	return CTalkMonster::GetSchedule();
}


// MODDD - base behavior possibly altered by returning the monsterstate determined
// instead of always falling down to the CTalkMonster (or CBaseMonster) GetIdealState
// call.  That may have further changed what state gets returned, but doubtful.
// Curiously, the base method never even did 'return' anywhere besides the very end of the
// method (not that the returned value used to be used, had to be set to m_IdealMonsterState
// in-method).  A few places like that would have stopped the parent GetIdealState calls.
MONSTERSTATE CScientist::GetIdealState ( void )
{
	if(aggro > 0){
		//AGGRESSIVE!
		return MONSTERSTATE_COMBAT;
	}

	switch ( m_MonsterState )
	{
	case MONSTERSTATE_ALERT:
	case MONSTERSTATE_IDLE:
		if ( HasConditions( bits_COND_NEW_ENEMY ) )
		{
			if ( IsFollowing() )
			{
				int relationship = IRelationship( m_hEnemy );
				if ( relationship != R_FR || relationship != R_HT && !HasConditions( bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE ) )
				{
					// Don't go to combat if you're following the player
					//MODDD - going to assume preventing the 'StopFollowing' call below was intended
					// behavior, but feel free to test by returning after that too.
					return MONSTERSTATE_ALERT;
				}
				StopFollowing( TRUE, FALSE );
			}
		}
		else if ( HasConditions( bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE ) )
		{
			// Stop following if you take damage
			if ( IsFollowing() )
				StopFollowing( TRUE, FALSE);
		}
		break;

	case MONSTERSTATE_COMBAT:
		{
			CBaseEntity *pEnemy = m_hEnemy;
			if ( pEnemy != NULL )
			{
				if ( DisregardEnemy( pEnemy ) )		// After 15 seconds of being hidden, return to alert
				{
					// Strip enemy when going to alert
					m_hEnemy = NULL;
					return MONSTERSTATE_ALERT;
				}
				// Follow if only scared a little.



				// MODDD.
				//    m_hTargetEnt can also be non-NULL for scripted behavior.
				//    Use the proper IsFollowing check instead.
				//    Otherise, a weak 'm_hTargetEnt != NULL' check confuses having a target-ent from scripted behavior
				//    (even without the scripted state) for following the player like this was meant for, and
				//    the schedule oscillates between ALERT and COMBAT really fast
				//    (ALERT from here, then COMBAT soon on realizing it's not actually following a player when there's
				//     an enemy nearby,  then ALERT here,  COMBAT seeing enemy, ALERT, COMBAT, ALERT, etc.)
				//    Which is bad.  Lags with pathfind checks as it bypasses the 'force panic anim' safety.
				//    Can m_pCine be checked for being NULL too?
				// (m_pCine == NULL)                                                                                                                        

				//if ( m_hTargetEnt != NULL )
				if(IsFollowing())
				{
					return MONSTERSTATE_ALERT;
				}

				if ( HasConditions ( bits_COND_SEE_ENEMY ) )
				{
					m_fearTime = gpGlobals->time;
					return MONSTERSTATE_COMBAT;
				}

			}
		}
		break;
	}

	return CTalkMonster::GetIdealState();
}



BOOL CScientist::CanHeal( CBaseMonster* arg_monsterTry ){

	if(arg_monsterTry == NULL){
		return FALSE;
	}

	//NPCs can have up to 70% of their health to be eligible for healing, since they're weaker.
	if ( (m_healTime > gpGlobals->time) || (arg_monsterTry == NULL) || (arg_monsterTry->pev->health > (arg_monsterTry->pev->max_health * EASY_CVAR_GET_DEBUGONLY(scientistHealNPCFract)) ) && ( !(arg_monsterTry->m_bitsDamageType & DMG_TIMEBASED || arg_monsterTry->m_bitsDamageTypeMod & DMG_TIMEBASEDMOD))  )
		return FALSE;

	return TRUE;
}

BOOL CScientist::CanHeal( void )
{

	if(m_hTargetEnt == NULL){
		// no target? clearly a 'no', not even trying
		return FALSE;
	}

	//MODDD - NEW REQUIREMENT.  Difficulty can disallow healing.
	if (gSkillData.scientist_can_heal == 0) {
		// nope.
		return FALSE;
	}

	//CBaseEntity* entityAttempt = CBaseEntity::Instance(m_hTargetEnt);
	CBaseEntity* entityAttempt = CBaseEntity::Instance(m_hTargetEnt->pev);
	//CBaseEntity* entityAttempt = GetClassPtr((CBaseEntity *)pev);
	
	CBaseMonster* monsterAttempt = NULL;
	if(entityAttempt != NULL){
		monsterAttempt = entityAttempt->MyMonsterPointer();
	}

	if(monsterAttempt == NULL){
		//can only heal a "Monster".
		return FALSE;
	}

	//OLD HEAL SCRIPT.
	//if ( (m_healTime > gpGlobals->time) || (m_hTargetEnt == NULL) || (m_hTargetEnt->pev->health > (m_hTargetEnt->pev->max_health * 0.5)) )
	//	return FALSE;

	float percentage = 0.5;
	if(healNPCChosen){
		percentage = EASY_CVAR_GET_DEBUGONLY(scientistHealNPCFract);
	}

	if ( (m_healTime > gpGlobals->time) || (m_hTargetEnt == NULL) || (m_hTargetEnt->pev->health > (m_hTargetEnt->pev->max_health * percentage) ) && ( !(monsterAttempt->m_bitsDamageType & DMG_TIMEBASED || monsterAttempt->m_bitsDamageTypeMod & DMG_TIMEBASEDMOD))  )
		return FALSE;

	return TRUE;
}

void CScientist::Heal( void )
{
	if (!CanHeal()) {
		return;
	}

	if(m_hTargetEnt == NULL){
		return;
	}

	Vector target = m_hTargetEnt->pev->origin - pev->origin;
	if (target.Length() > 100) {
		return;
	}

	CBaseEntity* entityAttempt = CBaseEntity::Instance(m_hTargetEnt->pev);
	CBaseMonster* monsterAttempt = NULL;
	if(entityAttempt != NULL){
		monsterAttempt = entityAttempt->MyMonsterPointer();
	}
	if(monsterAttempt == NULL){
		// can only heal a "Monster".
		return;
	}
	BOOL timedDamageFlag = FALSE;
	if(monsterAttempt->m_bitsDamageType & DMG_TIMEBASED || monsterAttempt->m_bitsDamageTypeMod & DMG_TIMEBASEDMOD){
		timedDamageFlag = TRUE;

		if(monsterAttempt->IsPlayer()){
			// send an HEV update.
			CBasePlayer* uhhhhh = static_cast<CBasePlayer*>(monsterAttempt);
			uhhhhh->SetSuitUpdate("!HEV_HEAL_GNC", FALSE, SUIT_REPEAT_OK);
		}
	}
	monsterAttempt->attemptResetTimedDamage(TRUE);
	
	float percentage = 0.5;
	if(healNPCChosen){
		percentage = EASY_CVAR_GET_DEBUGONLY(scientistHealNPCFract);
	}
	//MODDD - added this check.  Now, healing does not add health if over 50% health (before, could abuse the timed-damage-heal-trigger by taking timed damage over and over again to get enough healing to reach max health, when this was not possible before.
	// Note that if the player isn't healing for timed damage, this check is ignored (safe to assume that is just the result of multiple scientists healing at the same time, legal in the base game)
	if(!timedDamageFlag || m_hTargetEnt->pev->health <= (m_hTargetEnt->pev->max_health * percentage)){
		m_hTargetEnt->TakeHealth( gSkillData.scientistHeal, DMG_GENERIC );
	}
	
	// Don't heal again for 1 minute
	m_healTime = gpGlobals->time + EASY_CVAR_GET_DEBUGONLY(scientistHealCooldown);
	
}//Heal


int CScientist::FriendNumber( int arrayNumber )
{
	static int array[3] = { 1, 2, 0 };
	if (arrayNumber < 3) {
		return array[arrayNumber];
	}
	return arrayNumber;
}


void CScientist::MonsterThink(void){



	if(monsterID == 16){
		BOOL what = FALSE;
		if(m_pCine != NULL){
			what = m_pCine->CanInterrupt();
		}
		int x = 666;
	}



	if (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(thatWasntPunch) == 1 && (this->m_fSequenceFinished || pev->frame >= 245)) {

		switch (RANDOM_LONG(0, 74)) {
		case 0:this->SetSequenceByName("180_Left"); break;
		case 1:this->SetSequenceByName("180_Right"); break;
		case 2:this->SetSequenceByName("flinch"); break;
		case 3:this->SetSequenceByName("flinch1"); break;
		case 4:this->SetSequenceByName("laflinch"); break;
		case 5:this->SetSequenceByName("raflinch"); break;
		case 6:this->SetSequenceByName("llflinch"); break;
		case 7:this->SetSequenceByName("rlflinch"); break;
		case 8:this->SetSequenceByName("idle_brush"); break;
		case 9:this->SetSequenceByName("idle_look"); break;
		case 10:this->SetSequenceByName("idle_adjust"); break;
		case 11:this->SetSequenceByName("idle_yawn"); break;
		case 12:this->SetSequenceByName("crouchstand"); break;
		case 13:this->SetSequenceByName("crouch_idle2"); break;
		case 14:this->SetSequenceByName("crouch_idle3"); break;
		case 15:this->SetSequenceByName("panic"); break;
		case 16:this->SetSequenceByName("fear1"); break;
		case 17:this->SetSequenceByName("fear2"); break;
		case 18:this->SetSequenceByName("eye_wipe"); break;
		case 19:this->SetSequenceByName("pull_needle"); break;
		case 20:this->SetSequenceByName("return_needle"); break;
		case 21:this->SetSequenceByName("give_shot"); break;
		case 22:this->SetSequenceByName("punch"); break;
		case 23:this->SetSequenceByName("diesimple"); break;
		case 24:this->SetSequenceByName("dieviolent"); break;
		case 25:this->SetSequenceByName("diefast"); break;
		case 26:this->SetSequenceByName("barnacled1"); break;
		case 27:this->SetSequenceByName("barnacled2"); break;
		case 28:this->SetSequenceByName("barnacled3"); break;
		case 29:this->SetSequenceByName("barnacled4"); break;
		case 30:this->SetSequenceByName("console"); break;
		case 31:this->SetSequenceByName("checktie"); break;
		case 32:this->SetSequenceByName("dryhands"); break;
		case 33:this->SetSequenceByName("tieshoe"); break;
		case 34:this->SetSequenceByName("writeboard"); break;
		case 35:this->SetSequenceByName("studycart"); break;
		case 36:this->SetSequenceByName("lean"); break;
		case 37:this->SetSequenceByName("pondering"); break;
		case 38:this->SetSequenceByName("pondering2"); break;
		case 39:this->SetSequenceByName("pondering3"); break;
		case 40:this->SetSequenceByName("buysoda"); break;
		case 41:this->SetSequenceByName("yes"); break;
		case 42:this->SetSequenceByName("no"); break;
		case 43:this->SetSequenceByName("push_button"); break;
		case 44:this->SetSequenceByName("retina"); break;
		case 45:this->SetSequenceByName("coffee"); break;
		case 46:this->SetSequenceByName("franticbutton"); break;
		case 47:this->SetSequenceByName("startle"); break;
		case 48:this->SetSequenceByName("sstruggleidle"); break;
		case 49:this->SetSequenceByName("sstruggle"); break;
		case 50:this->SetSequenceByName("scicrashidle"); break;
		case 51:this->SetSequenceByName("scicrash"); break;
		case 52:this->SetSequenceByName("scientist_idlewall"); break;
		case 53:this->SetSequenceByName("crawlwindow"); break;
		case 54:this->SetSequenceByName("locked_door"); break;
		case 55:this->SetSequenceByName("pulldoor"); break;
		case 56:this->SetSequenceByName("jumpshockidle"); break;
		case 57:this->SetSequenceByName("jumpshock"); break;
		case 58:this->SetSequenceByName("ventpullidle1"); break;
		case 59:this->SetSequenceByName("ventpullidle2"); break;
		case 60:this->SetSequenceByName("beatdoor"); break;
		case 61:this->SetSequenceByName("hide_in_vent"); break;
		case 62:this->SetSequenceByName("scientist_leanhandrailidle"); break;
		case 63:this->SetSequenceByName("scientist_leanhandrail"); break;
		case 64:this->SetSequenceByName("wave"); break;
		case 65:this->SetSequenceByName("hanging_idle"); break;
		case 66:this->SetSequenceByName("keypad"); break;
		case 67:this->SetSequenceByName("quicklook"); break;
		case 68:this->SetSequenceByName("gluonshow"); break;
		case 69:this->SetSequenceByName("psst"); break;
		case 70:this->SetSequenceByName("pratfall"); break;

		case 71:this->SetSequenceByName("wave"); break;
		case 72:this->SetSequenceByName("wave"); break;
		case 73:this->SetSequenceByName("wave"); break;
		case 74:this->SetSequenceByName("wave"); break;
		}//END OF switch
	}







	//easyForcePrintLine("imascientist id:%d act:%d ideal:%d seq:%d fr:%.2f lps:%d fin:%d lfin:%d", monsterID, m_Activity, m_IdealActivity, this->pev->sequence, pev->frame, m_fSequenceLoops, this->m_fSequenceFinished, this->m_fSequenceFinishedSinceLoop);
	//easyForcePrintLine("AYY YO WHAT THE helk %.2f %.2f", gpGlobals->time, pev->dmgtime);
	int tempTaskNumber = this->getTaskNumber();
	if( 
		(
			//m_pSchedule == slSciFollow ||
			//m_pSchedule == slFaceTarget ||
			(m_pSchedule == slHeal && tempTaskNumber != TASK_HEAL && tempTaskNumber !=  TASK_PLAY_SEQUENCE_FACE_TARGET)
		) &&
		(m_hTargetEnt == NULL || (m_hTargetEnt != NULL && !m_hTargetEnt->IsAlive()) )
		)  {
		// Fail if who we're supposed to follow dies.
		// m_hTargetEnt = NULL;
		leaderRecentlyDied = TRUE;
		TaskFail();
	}


	//if(monsterID==2)easyForcePrintLine("ID:%d FRAME: %.2f SEQ: %d", monsterID, pev->frame, pev->sequence);



	
	//easyPrintLine("WELL??? t:%d e:%d COND: see:%d ne:%d sf:%d sdh:%d", m_hTargetEnt!=NULL, m_hEnemy!=NULL, this->HasConditions(bits_COND_SEE_ENEMY), this->HasConditions(bits_COND_NEW_ENEMY), this->HasConditions(bits_COND_SEE_FEAR), this->HasConditions(bits_COND_SEE_HATE|bits_COND_SEE_DISLIKE) );

	while(TRUE){
		if(aggro > 0){

			// If I travel too far from the point I decided to chase someone, get cowardly and forget.
			// Continuing to go forward is still possible after that though.
			if((aggroOrigin - pev->origin).Length() > 270 ){
				//traveled too far, just forget about this.
				aggro = 0;
				TaskFail();
				//ChangeSchedule(slScientistCover);//  ???
				break;
			}


			/*
			//IS THIS LINE OFFENDING
			CSound* eh = NULL;
			if( (eh=this->PBestSound()) != NULL && (eh->m_iType|bits_SOUND_DANGER|bits_SOUND_COMBAT) ){
				//easyPrintLine("WOOOOOOOOOOOOOOO");

				if(RANDOM_LONG(0,2) == 0){
					aggro = -1;
					TaskFail();
					ChangeSchedule(slScientistCover);//  ???
				}
				break;
			}
			*/

			//ChangeSchedule(slScientistCover);  ???


			//PBestSound
			break;
		}
	break;
	}


	/*
	//used for a test, nevermind now.
	if(pev->spawnflags & 8){
		easyPrintLine("SCIHEAD: COUNTUPDATE: %d", getNumberOfBodyParts());
	}
	*/


	if(EASY_CVAR_GET_DEBUGONLY(scientistHealNPCDebug) == 1){


		if(m_healTime > gpGlobals->time){
			//still waiting until I can heal again.  a minute has to pass before the scientist may heal again.
			UTIL_drawLineFrameBoxAround(pev->origin, 12, 20, 85, 85, 85);
		}else if(healNPCChosen){
			UTIL_drawLineFrameBoxAround(pev->origin, 7, 20, 255, 255, 255);
		}else if(IsFollowing()){
			UTIL_drawLineFrameBoxAround(pev->origin, 7, 20, 0, 255, 0);
		}else if(m_hTargetEnt != NULL){
			UTIL_drawLineFrameBoxAround(pev->origin, 7, 20, 255, 0, 0);
		}else{
			UTIL_drawLineFrameBoxAround(pev->origin, 7, 20, 0, 0, 255);
		}
	}

	if(healNPCChosen == TRUE){
		//Should be impossible.  Just a check though.
		if(!UTIL_IsAliveEntity(m_hTargetEnt)){
			forgetHealNPC();
		}else{

			//note that the default "CanHeal" refers to the target ent.  That is good.
			if(CanHeal()){
				//no problems.
			}else{
				//something changed, and we no longer need to heal this person?  Stop following / targeting.
				forgetHealNPC();
			}
		}
	}


	//MODDD - I think this section is all new?
	if( HasConditions(bits_COND_SEE_ENEMY|bits_COND_NEW_ENEMY|bits_COND_SEE_NEMESIS|bits_COND_SEE_HATE|bits_COND_SEE_FEAR|bits_COND_SEE_DISLIKE ) ){
		//any sight of the enemy makes this happen.
		m_fearTime = gpGlobals->time;
	}




	/*
	if(explodeDelay == -3){
		//start the process.
		myAssHungers();
	}

	if(explodeDelay == -2){
		this->TakeDamage(pev, pev, 99999, DMG_ALWAYSGIB, 0);
		//this->Killed(pev, pev, GIB_ALWAYS);
		//return;
	}
	*/

	CTalkMonster::MonsterThink();


	/*
	if (m_fSequenceFinished && pev->sequence == g_scientist_crouchstand_sequenceID) {
		//SAFETY.   If done with the crouch-to-stand animation and at the end, get a new animation.
		// should be unnecessary, base monster script should pick an animation when any sequence while in ACT_IDLE
		// is finished.
		m_Activity = ACT_RESET;
		SetActivity(ACT_IDLE);
	}
	*/


	/*
	if(explodeDelay > -1 && gpGlobals->time >= explodeDelay){
		pev->renderfx = kRenderFxImplode;
		pev->rendercolor.x = 255;
		pev->rendercolor.y = 0;
		pev->rendercolor.z = 0;
		StopAnimation();
		pev->nextthink = gpGlobals->time + 0.5;
		//SetThink( &CBaseEntity::SUB_Remove );
		explodeDelay = -2;
		//return;
	}
	*/

}//END OF MonsterThink




//MODDD - class moved.



char *CDeadScientist::m_szPoses[] = { "lying_on_back", "lying_on_stomach", "dead_sitting", "dead_hang", "dead_table1", "dead_table2", "dead_table3" };

int CDeadScientist::numberOfModelBodyParts = -1;



TYPEDESCRIPTION	CDeadScientist::m_SaveData[] =
{
	DEFINE_FIELD( CDeadScientist, trueBody, FIELD_INTEGER ),


};

IMPLEMENT_SAVERESTORE( CDeadScientist, CBaseMonster );


void CDeadScientist::PostRestore(){
	scientistHeadFilter(*this, numberOfModelBodyParts, &trueBody);
	
}





/*
int CDeadScientist::Save( CSave &save )
{
	if ( !CBaseMonster::Save(save) )
		return 0;

	return save.WriteFields( "CDeadScientist", this, m_SaveData, ARRAYSIZE(m_SaveData) );
}
//MODDD
int CDeadScientist::Restore( CRestore &restore )
{
	if ( !CBaseMonster::Restore(restore) )
		return 0;

	int tempResult = restore.ReadFields( "CDeadScientist", this, m_SaveData, ARRAYSIZE(m_SaveData) );
	scientistHeadFilter(*this, numberOfModelBodyParts, &trueBody);
	PostRestore();
	return tempResult;
}
*/













void CDeadScientist::KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "pose"))
	{
		m_iPose = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseMonster::KeyValue( pkvd );
}


#if REMOVE_ORIGINAL_NAMES != 1
	LINK_ENTITY_TO_CLASS( monster_scientist_dead, CDeadScientist );
#endif

#if EXTRA_NAMES > 0
	LINK_ENTITY_TO_CLASS( scientist_dead, CDeadScientist );

	//no extras.
#endif




BOOL CDeadScientist::getGermanModelRequirement(void){
	return globalPSEUDO_germanModel_scientistFound;
}
const char* CDeadScientist::getGermanModel(void){
	return "models/g_scientist.mdl";
}
const char* CDeadScientist::getNormalModel(void){
	return "models/scientist.mdl";
}

//MODDD
void CDeadScientist::setModel(void){
	CDeadScientist::setModel(NULL);
}
void CDeadScientist::setModel(const char* m){

	//let this handle model management.
	CBaseMonster::setModel(m);

	if(numberOfModelBodyParts==-1)numberOfModelBodyParts = getNumberOfBodyParts();
	

}



void CDeadScientist::Activate(void){
	CBaseMonster::Activate();

	//!setModelCustom();
	//scientistHeadFilter(*this, numberOfModelBodyParts);


}

CDeadScientist::CDeadScientist(void){

}
//
// ********** DeadScientist SPAWN **********
//
void CDeadScientist::Spawn( )
{

	PRECACHE_MODEL("models/scientist.mdl");


	if(EASY_CVAR_GET_DEBUGONLY(monsterSpawnPrintout) == 1){
		easyPrintLine("MY <dead scientist> BODYH??? %d %d", spawnedDynamically, pev->body);
	}

	if(spawnedDynamically){
		// signal a randomization if spawned by the player.
		//pev->body = -1;    no need, getting overridden soon anyway
		trueBody = 0;
	}else{
		// leave "pev->body" to whatever the map made it
		trueBody = pev->body + 1;
	}


	//MODDD - do it ahead of time for me!
	//Also, custom script related to picking the model happens here, so handle body-stuff above.
	CBaseMonster::Spawn();


	//PRECACHE_MODEL("models/scientist_pre_e3.mdl");
	//PRECACHE_MODEL("models/scientist_e3.mdl");
	

	//!setModelCustom();
	setModel(); //"models/scientist.mdl"  //argument unused for monsters with German versions. 

	scientistHeadFilter(*this, numberOfModelBodyParts, &trueBody);
	



	pev->classname = MAKE_STRING("monster_scientist_dead");

	pev->effects		= 0;
	pev->sequence		= 0;
	// Corpses have less health
	pev->health			= 8;//gSkillData.scientistHealth;

	m_bloodColor = BloodColorRedFilter();




	//MODDD - removed.
	/*
	// Luther is black, make his hands black
	if ( pev->body == HEAD_LUTHER )
		pev->skin = 1;
	else
		pev->skin = 0;
	*/

	pev->sequence = LookupSequence( m_szPoses[m_iPose] );
	if (pev->sequence == -1)
	{
		ALERT ( at_console, "Dead scientist with bad pose\n" );
	}

	//MOVED TO "setModelCustom" for the dead scientist.
	/*
	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(sv_germancensorship) != 1 && EASY_CVAR_GET(scientistModel) > 0){
		//MODDD - uncommented out, used to be commented out.
		//pev->skin += 2; // use bloody skin -- UNDONE: Turn this back on when we have a bloody skin again!
		pev->skin = 2;
		//...just force it to "2" instead.  There is no "white" or "black" skin in alpha models to offset this.
	}
	*/


	MonsterInitDead();



	pev->skin = 0; //default
	//if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(sv_germancensorship) != 1 && EASY_CVAR_GET(scientistModel) < 2){
	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(sv_germancensorship) != 1){
		//MODDD - uncommented out, used to be commented out.
		//pev->skin += 2; // use bloody skin -- UNDONE: Turn this back on when we have a bloody skin again!
		pev->skin = 2;
		//...just force it to "2" instead.  There is no "white" or "black" skin in alpha models to offset this.
	}


	// MODDD
	// forces the mouth open for scientist, ignores mouth-values from speech (so long as none are trying to
	// be played:  'mouthopen' in studiomodelrenderer being 0).
	SetBoneController(4, 20);



	//MODDD - emit a stench that eaters will pick up.
	//MODDD TODO - IMPORTANT: will this be preserved between save / restores? Need to make sure.
	
	if(isOrganicLogic()){
		CSoundEnt::InsertSound ( bits_SOUND_CARCASS, pev->origin, 384, SOUND_NEVER_EXPIRE );
	}
}


//MODDD - class moved


#if REMOVE_ORIGINAL_NAMES != 1
	LINK_ENTITY_TO_CLASS( monster_sitting_scientist, CSittingScientist );
#endif

#if EXTRA_NAMES > 0
	LINK_ENTITY_TO_CLASS( scientist_sitting, CSittingScientist );

	#if EXTRA_NAMES == 2
		LINK_ENTITY_TO_CLASS( sitting_scientist, CSittingScientist );
		LINK_ENTITY_TO_CLASS( monster_scientist_sitting, CSittingScientist );
	#endif

#endif



TYPEDESCRIPTION	CSittingScientist::m_SaveData[] =
{
	// Don't need to save/restore m_baseSequence (recalced)
	DEFINE_FIELD( CSittingScientist, m_headTurn, FIELD_INTEGER ),
	DEFINE_FIELD( CSittingScientist, m_flResponseDelay, FIELD_FLOAT ),
};

IMPLEMENT_SAVERESTORE( CSittingScientist, CScientist );

void CSittingScientist::PostRestore(){
	//NOTICE - no need to call the headFilter here. Our parent class, CScientist, still does this.
}





GENERATE_TRACEATTACK_IMPLEMENTATION(CSittingScientist)
{
	GENERATE_TRACEATTACK_PARENT_CALL(CTalkMonster);
}
GENERATE_TAKEDAMAGE_IMPLEMENTATION(CSittingScientist)
{
	//No complexity, no CTalkMonster stuff. Just the CBaseMonster's TakeDamage.
	int tkdDmgRes = GENERATE_TAKEDAMAGE_PARENT_CALL(CBaseMonster);
	//MODDD HACKY - don't turn around.
	pev->ideal_yaw = this->pev->angles.y;

	return tkdDmgRes;
}






void CSittingScientist::Activate(void){
	//!setModelCustom();
	//TalkInit();
	//CScientist does these things.

	CScientist::Activate();
	//the parent constructor already has a call to "scientistHeadFilter".
}


CSittingScientist::CSittingScientist(void){

}

//
// ********** Scientist SPAWN **********
//
//MODDD NOTE - does not call "CScientist Spawn" as one may expect. Any script in there is not inherited by the SittingScientist then.
void CSittingScientist::Spawn( )
{
	PRECACHE_MODEL("models/scientist.mdl");
	Precache();

	if(spawnedDynamically){
		// signal a randomization if spawned by the player.
		//pev->body = -1;    no need, getting overridden soon anyway
		trueBody = 0;
	}else{
		// leave "pev->body" to whatever the map made it
		trueBody = pev->body + 1;
	}

	//skip to talk monster, this script is independent of what happens in CScientist.
	
	
	

	CTalkMonster::Spawn();

	//why is this here and not in "precache" for the sittingScientist below?
	

	setModel();  //"models/scientist.mdl"  //argument unused for monsters with German versions. 
	//!setModelCustom();
	////TalkInit();
	scientistHeadFilter(*this, numberOfModelBodyParts, &trueBody);


	InitBoneControllers();

	UTIL_SetSize(pev, Vector(-14, -14, 0), Vector(14, 14, 36));

	pev->classname = MAKE_STRING("monster_sitting_scientist");

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	pev->effects		= 0;
	pev->health			= 50;

	m_bloodColor = BloodColorRedFilter();
	m_flFieldOfView		= VIEW_FIELD_WIDE; // indicates the width of this monster's forward view cone ( as a dotproduct result )

	m_afCapability		= bits_CAP_HEAR | bits_CAP_TURN_HEAD;

	SetBits(pev->spawnflags, SF_MONSTER_PREDISASTER); // predisaster only!


	//scientistHeadFilter(*this, numberOfModelBodyParts);



	m_baseSequence = LookupSequence( "sitlookleft" );
	pev->sequence = m_baseSequence + RANDOM_LONG(0,4);
	ResetSequenceInfo( );

	SetThink (&CSittingScientist::SittingThink);
	pev->nextthink = gpGlobals->time + 0.1;





	// does not call "monsterInit", the normal source of this.  So, there.
	if(isOrganic()){
		pev->renderfx |= ISNPC;
	}else{
		pev->renderfx |= ISMETALNPC;
	}
	
	DROP_TO_FLOOR ( ENT(pev) );

	pev->view_ofs		= Vector ( 0, 0, 40 );// position of the eyes relative to monster's origin.
	m_MonsterState		= MONSTERSTATE_NONE;

	//m_afCapability		= bits_CAP_HEAR | bits_CAP_TURN_HEAD | bits_CAP_OPEN_DOORS | bits_CAP_AUTO_DOORS | bits_CAP_USE;

	//MonsterInit();
	SetUse( &CTalkMonster::FollowerUse );



	if (monsterID == -1) {
		//MODDD - must do manually since init is skipped.
		monsterID = monsterIDLatest;
		monsterIDLatest++;
	}
	pev->flags |= FL_MONSTER;  //why not.

}

void CSittingScientist::Precache( void )
{
	m_baseSequence = LookupSequence( "sitlookleft" );
	TalkInit();
}

//=========================================================
// ID as a passive human
//=========================================================
int CSittingScientist::Classify ( void )
{
	return	CLASS_HUMAN_PASSIVE;
}


int CSittingScientist::FriendNumber( int arrayNumber )
{
	static int array[3] = { 2, 1, 0 };
	if ( arrayNumber < 3 )
		return array[ arrayNumber ];
	return arrayNumber;
}



//=========================================================
// sit, do stuff
//=========================================================
void CSittingScientist::SittingThink( void )
{
	CBaseEntity *pent;

	StudioFrameAdvance_SIMPLE( );

	// try to greet player
	if (FIdleHello())
	{
		pent = FindNearestFriend(TRUE);
		if (pent)
		{
			float yaw = VecToYaw(pent->pev->origin - pev->origin) - pev->angles.y;

			if (yaw > 180) yaw -= 360;
			if (yaw < -180) yaw += 360;

			if (yaw > 0)
				pev->sequence = m_baseSequence + SITTING_ANIM_sitlookleft;
			else
				pev->sequence = m_baseSequence + SITTING_ANIM_sitlookright;

		ResetSequenceInfo( );
		pev->frame = 0;

		if(EASY_CVAR_GET_DEBUGONLY(wildHeads) != 1){
			SetBoneController( 0, 0 );
		}


		}
	}
	else if (m_fSequenceFinished)
	{
		int i = RANDOM_LONG(0,99);
		m_headTurn = 0;

		if (m_flResponseDelay && gpGlobals->time > m_flResponseDelay)
		{
			// respond to question
			IdleRespond();
			pev->sequence = m_baseSequence + SITTING_ANIM_sitscared;
			m_flResponseDelay = 0;
		}
		else if (i < 30)
		{
			pev->sequence = m_baseSequence + SITTING_ANIM_sitting3;

			// turn towards player or nearest friend and speak

			if (!FBitSet(m_bitsSaid, bit_saidHelloPlayer))
				pent = FindNearestFriend(TRUE);
			else
				pent = FindNearestFriend(FALSE);

			if (!FIdleSpeak() || !pent)
			{
				m_headTurn = RANDOM_LONG(0,8) * 10 - 40;
				pev->sequence = m_baseSequence + SITTING_ANIM_sitting3;
			}
			else
			{
				// only turn head if we spoke
				float yaw = VecToYaw(pent->pev->origin - pev->origin) - pev->angles.y;

				if (yaw > 180) yaw -= 360;
				if (yaw < -180) yaw += 360;

				if (yaw > 0)
					pev->sequence = m_baseSequence + SITTING_ANIM_sitlookleft;
				else
					pev->sequence = m_baseSequence + SITTING_ANIM_sitlookright;

				//ALERT(at_console, "sitting speak\n");
			}
		}
		else if (i < 60)
		{
			pev->sequence = m_baseSequence + SITTING_ANIM_sitting3;
			m_headTurn = RANDOM_LONG(0,8) * 10 - 40;
			if (RANDOM_LONG(0,99) < 5)
			{
				//ALERT(at_console, "sitting speak2\n");
				FIdleSpeak();
			}
		}
		else if (i < 80)
		{
			pev->sequence = m_baseSequence + SITTING_ANIM_sitting2;
		}
		else if (i < 100)
		{
			pev->sequence = m_baseSequence + SITTING_ANIM_sitscared;
		}

		ResetSequenceInfo( );
		pev->frame = 0;
		if(EASY_CVAR_GET_DEBUGONLY(wildHeads) != 1){
			SetBoneController( 0, m_headTurn );
		}
	}
	pev->nextthink = gpGlobals->time + 0.1;
}

// prepare sitting scientist to answer a question
void CSittingScientist::SetAnswerQuestion( CTalkMonster *pSpeaker )
{
	m_flResponseDelay = gpGlobals->time + RANDOM_FLOAT(3, 4);
	m_hTalkTarget = (CBaseMonster *)pSpeaker;
}


//=========================================================
// FIdleSpeak
// ask question of nearby friend, or make statement
//=========================================================
int CSittingScientist::FIdleSpeak ( void )
{
	// try to start a conversation, or make statement
	int pitch;

	if (!FOkToSpeak())
		return FALSE;

	// set global min delay for next conversation
	CTalkMonster::g_talkWaitTime = gpGlobals->time + RANDOM_FLOAT(4.8, 5.2);

	pitch = GetVoicePitch();

	// if there is a friend nearby to speak to, play sentence, set friend's response time, return

	// try to talk to any standing or sitting scientists nearby
	CBaseEntity *pentFriend = FindNearestFriend(FALSE);





	//MODDD - and why were these using SENTENCEG_PlayRndSz anyway?  It's a more raw form called
	// by PlaySentence.  PlaySentence also calls "Talk()" for you, so may as well use this one.
	if (pentFriend && RANDOM_LONG(0,1))
	{
		CTalkMonster *pTalkMonster = GetClassPtr((CTalkMonster *)pentFriend->pev);
		pTalkMonster->SetAnswerQuestion( this );

		IdleHeadTurn(pentFriend->pev->origin);
		PlaySentence(m_szGrp[TLK_PQUESTION], 5, 1.0, ATTN_IDLE, pitch);
		// set global min delay for next conversation
		CTalkMonster::g_talkWaitTime = gpGlobals->time + RANDOM_FLOAT(4.8, 5.2);
		return TRUE;
	}

	// otherwise, play an idle statement
	if (RANDOM_LONG(0,1))
	{
		PlaySentence(m_szGrp[TLK_PIDLE], 5, 1.0, ATTN_IDLE, pitch);
		// set global min delay for next conversation
		CTalkMonster::g_talkWaitTime = gpGlobals->time + RANDOM_FLOAT(4.8, 5.2);
		return TRUE;
	}

	// never spoke

	//MODDD - why wasn't this here like in CTalkMonster?
	Talk(0);

	CTalkMonster::g_talkWaitTime = 0;
	return FALSE;
}


void CScientist::StartFollowing(CBaseEntity *pLeader){
	//if we start following, we're not healing another NPC.
	//healTargetNPC = NULL;
	forgetHealNPC();


	CTalkMonster::StartFollowing(pLeader);
}


void CScientist::forgetHealNPC(void){
	
	if(healNPCChosen){
		healNPCChosen = FALSE;

		//scientistTryingToHealMe

		//is it ok for "isAlive" to be replaced by "isValid" here???
		if(m_hTargetEnt != NULL && UTIL_IsValidEntity(m_hTargetEnt) && m_hTargetEnt->isTalkMonster()){
			
			CBaseEntity* testEnt = Instance(m_hTargetEnt->pev);
			if(testEnt != NULL){
				CTalkMonster* thisNameSucks = static_cast<CTalkMonster*>(testEnt);
				if(thisNameSucks != NULL){
					thisNameSucks->scientistTryingToHealMe = NULL;
					thisNameSucks->scientistTryingToHealMeEHANDLE = NULL;
				}
			}
		}

		const char* heyy = getScheduleName();
		int hooo = getTaskNumber();


		m_hTargetEnt = NULL;
	}

}



BOOL CScientist::CheckMeleeAttack1(float flDot, float flDist){
	// this is not an attack for the scientist, it is the syringe animation.  It is specifically called for, so don't allow
	// it this way.
	return FALSE;
}


//ripped from HGrunt
BOOL CScientist::CheckMeleeAttack2(float flDot, float flDist){
	CBaseMonster *pEnemy = NULL;
	if ( m_hEnemy != NULL )
	{
		pEnemy = m_hEnemy->MyMonsterPointer();
	}
	if ( !pEnemy )
	{
		return FALSE;
	}
	//couldMeleeAttack1 = TRUE;

	//easyForcePrintLine("WAAAAT %.2f", flDist);

	//allow any dot product, we're facing the enemy fast enough.
	//if ( flDist <= 47 && 
	if ( flDist <= SCIENTIST_MELEE_ALLOW_RANGE && flDot >= 0.7	&&   //0.5?
		 pEnemy->Classify() != CLASS_ALIEN_BIOWEAPON &&
		 pEnemy->Classify() != CLASS_PLAYER_BIOWEAPON )
	{

		if(flDot >= 0.7){
			//good to go.
			return TRUE;
		}else{
			return FALSE;
		}

	}else{
		//couldMeleeAttack1 = FALSE;
	}
	return FALSE;
}


void CScientist::HandleEventQueueEvent(int arg_eventID){
	int rand;
	BOOL pass;

	switch(arg_eventID){
		case 0:
			//send a punch!
			CBaseEntity *pHurt = CheckTraceHullAttack(SCIENTIST_MELEE_PHYSICAL_RANGE, gSkillData.scientistDmgPunch, DMG_CLUB );
			if ( pHurt )
			{
				if ( (pHurt->pev->flags & (FL_MONSTER|FL_CLIENT)) && !pHurt->blocksImpact() )
				{
					pHurt->pev->punchangle.z = -8;
					pHurt->pev->punchangle.x = 3;
					pHurt->pev->velocity = pHurt->pev->velocity - gpGlobals->v_right * 40;
				}
				// Play a random attack hit sound
				UTIL_PlaySound( ENT(pev), CHAN_WEAPON, pAttackHitSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackHitSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );

				//each hit contributes to fear.  raise aggro, but each time reduces the chance of staying in aggro..
				

				rand = RANDOM_LONG(0, 10);
				pass = FALSE;

				switch((int)aggro){
				case 1:
					pass = rand < 7;
				break;
				case 2:
					pass = rand < 4;
				break;
				case 3:
					pass = rand < 2;
				break;
				case 4:
					pass = rand < 1;
				break;
				default:
					//no.
				break;
				}
				if(pass){
					aggro++;
				}else{
					//wuss out.
					aggro = -1;
				}

			}
			else{ // Play a random attack miss sound
				UTIL_PlaySound( ENT(pev), CHAN_WEAPON, pAttackMissSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackMissSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );
			}

		break;
	}//END OF switch(...)

}


BOOL CScientist::usesAdvancedAnimSystem(void){
	return TRUE;
}




int CScientist::LookupActivityHard(int activity){
	pev->framerate = 1;
	resetEventQueue();

	m_flFramerateSuggestion = 1;


	if ( !(activity == ACT_IDLE || activity == ACT_CROUCH || activity == ACT_CROUCHIDLE || activity == ACT_TURN_LEFT || activity == ACT_TURN_RIGHT) ) {
		// reset it next time.
		nextIdleFidgetAllowedTime = -1;
	}


	//let's do m_IdealActivity??
	//uh... why?  nevermind then.
	switch(activity){

		case ACT_ARM:{
			if (healNPCChosen == FALSE) {
				// the heal anim for the player gets a slight boost.
				m_flFramerateSuggestion = 1.7f;
			}else {
				// For other NPC's, we need to be faster.  Barney's and scientists can't take much damage and need timed damage cured ASAP.
				// ...not as urgent now that timed damage is 15% for talkmonsters, but eh.
				m_flFramerateSuggestion = 2.1f;
			}
			return CBaseAnimating::LookupActivity(activity);
		}break;
		case ACT_DISARM:{
			if (healNPCChosen == FALSE) {
				m_flFramerateSuggestion = 2.1f;
			}else {
				m_flFramerateSuggestion = 2.5f;
			}
			return CBaseAnimating::LookupActivity(activity);
		}break;
		// actual healing animation (syringe goes in), short but eh.
		case ACT_MELEE_ATTACK1:{
			if (healNPCChosen == FALSE) {
				m_flFramerateSuggestion = 1.7f;
			}else {
				m_flFramerateSuggestion = 2.4f;
			}
			return CBaseAnimating::LookupActivity(activity);
		}break;
		// melee punch.
		case ACT_MELEE_ATTACK2:{
			m_flFramerateSuggestion = 1.24;
			animEventQueuePush(6.7f / 30.0f, 0);
			animFrameCutoffSuggestion = 240;
			return LookupSequence("punch");
		}break;

		case ACT_TURN_LEFT:
		case ACT_TURN_RIGHT:
		{
			//  || pev->sequence == g_scientist_crouchstand_sequenceID

			// if no enemy and crouching, can go to standing more smoothly.
			if (m_hEnemy == NULL && ((pev->sequence == g_scientist_crouchstand_sequenceID && !m_fSequenceFinished) || m_Activity == ACT_CROUCH || m_Activity == ACT_CROUCHIDLE)) {
				doNotResetSequence = TRUE;
				return g_scientist_crouchstand_sequenceID;
			}

			if (m_hEnemy == NULL && ((pev->sequence == g_scientist_checktie_sequenceID && !m_fSequenceFinished))) {
				// keep doing it then
				doNotResetSequence = TRUE;
				return g_scientist_checktie_sequenceID;
			}

		}break;
		case ACT_IDLE:{

			// if no enemy and crouching, can go to standing more smoothly.
			if (m_hEnemy == NULL && (m_Activity == ACT_CROUCH || m_Activity == ACT_CROUCHIDLE) ) {
				doNotResetSequence = TRUE;
				return g_scientist_crouchstand_sequenceID;
			}

			if (m_hEnemy == NULL && ((pev->sequence == g_scientist_checktie_sequenceID && !m_fSequenceFinished))) {
				// keep doing it then
				doNotResetSequence = TRUE;
				return g_scientist_checktie_sequenceID;
			}

			// First off, are we talking right now?
			if(IsTalking()){
				// Limit the animations we can choose from a little more.
				// Most people don't typically move around too much while looking at someone and talking to them,
				// compared to just standing around or listening to a long conversation.
				// BUT, simulare the wold weights.  The sum of all weights of the available animations
				// (see a scientist.qc file from decompiling the model) is used instead of course.
				// No fidgets here.
				const int animationWeightTotal = 90+20;
				const int animationWeightChoice = RANDOM_LONG(0, animationWeightTotal-1);

				if(animationWeightChoice < 90){
					m_flFramerateSuggestion = 0.9f;
					return LookupSequence("idle1");
				}else{ //if(animationWeightChoice < 90+20){
					m_flFramerateSuggestion = 0.85f;
					return LookupSequence("idle_subtle_alpha");
				}
			}else{
				// Not talking?  One more filter...
				// Are we in predisaster?
				if(FBitSet(pev->spawnflags, SF_MONSTER_PREDISASTER)){
					// Don't allow "idle_look". We have no reason to look scared yet, ordinary day so far.
					//const int animationWeightTotal = 90+20+3+2+1;
					//const int animationWeightChoice = RANDOM_LONG(0, animationWeightTotal-1);

					if (nextIdleFidgetAllowedTime == -1) {
						// set it.
						nextIdleFidgetAllowedTime = gpGlobals->time + RANDOM_FLOAT(11, 15);
					}

					if (gpGlobals->time < nextIdleFidgetAllowedTime) {
						// not there yet?  Pick a plain idle
						const int animationWeightTotal = 90 + 20;
						const int animationWeightChoice = RANDOM_LONG(0, animationWeightTotal - 1);

						if (animationWeightChoice < 90) {
							m_flFramerateSuggestion = 0.64f;
							return LookupSequence("idle1");
						}
						else if (animationWeightChoice < 90 + 20) {
							m_flFramerateSuggestion = 0.78f;
							return LookupSequence("idle_subtle_alpha");
						}

					}
					else {
						const int animationWeightTotal = 3 + 2 + 1;
						const int animationWeightChoice = RANDOM_LONG(0, animationWeightTotal - 1);

						nextIdleFidgetAllowedTime = gpGlobals->time + RANDOM_FLOAT(11, 15);

						// play a fidget
						if (animationWeightChoice < 3) {
							m_flFramerateSuggestion = 0.93f;
							return LookupSequence("idle_brush");
							//no idle_look
						}
						else if (animationWeightChoice < 3 + 2) {
							m_flFramerateSuggestion = 0.87f;
							return LookupSequence("idle_adjust");
						}
						else { //if(animationWeightChoice < 3 + 2 + 1){
							m_flFramerateSuggestion = 0.92f;
							return LookupSequence("idle_yawn");
						}
					}
				}else{
					/*
					// Just pick from the model, any idle animation is okay right now.
					int theSeq = CBaseAnimating::LookupActivity(activity);

					// idle or idle_subtle_alpha?  slow down a little.
					if (theSeq == 12 || theSeq == 13) {
						m_flFramerateSuggestion = 0.84f;
					}
					else {
						m_flFramerateSuggestion = 0.95f;
					}

					return theSeq;
					*/


					if (nextIdleFidgetAllowedTime == -1) {
						// set it.
						nextIdleFidgetAllowedTime = gpGlobals->time + RANDOM_FLOAT(11, 15);
					}

					if (gpGlobals->time < nextIdleFidgetAllowedTime) {
						// not there yet?  Pick a plain idle
						const int animationWeightTotal = 90 + 20;
						const int animationWeightChoice = RANDOM_LONG(0, animationWeightTotal - 1);

						if (animationWeightChoice < 90) {
							m_flFramerateSuggestion = 0.84f;
							return LookupSequence("idle1");
						}
						else if (animationWeightChoice < 90 + 20) {
							m_flFramerateSuggestion = 0.84f;
							return LookupSequence("idle_subtle_alpha");
						}

					}
					else {
						const int animationWeightTotal = 3 + 2 + 2 + 1;
						const int animationWeightChoice = RANDOM_LONG(0, animationWeightTotal - 1);

						nextIdleFidgetAllowedTime = gpGlobals->time + RANDOM_FLOAT(11, 15);

						// play a fidget
						if (animationWeightChoice < 3) {
							m_flFramerateSuggestion = 0.97f;
							return LookupSequence("idle_brush");
						}
						else if (animationWeightChoice < 3 + 2) {
							m_flFramerateSuggestion = 0.92f;
							return LookupSequence("idle_look");
						}
						else if (animationWeightChoice < 3 + 2 + 2) {
							m_flFramerateSuggestion = 0.90f;
							return LookupSequence("idle_adjust");
						}
						else { //if(animationWeightChoice < 3 + 2 + 2 + 1){
							m_flFramerateSuggestion = 0.94f;
							return LookupSequence("idle_yawn");
						}
					}

				}
			}//END OF IsTalking check
			
		}break;
		// ACT_CROUCH includes crouch_idle2 and crouch_idle3.
		// ACT_CROUCHIDLE includes crouch_idle.
		// Idea:  If ACT_CROUCH was picked, let default behavior happen but still mark this as a figet.
		// ACT_CROUCHIDLE will want to use crouch_idle most often but will use idle2 & 3 as fidgets.
		case ACT_CROUCH: {
			nextIdleFidgetAllowedTime = gpGlobals->time + RANDOM_FLOAT(11, 15);
		}break;
		case ACT_CROUCHIDLE: {

			if (nextIdleFidgetAllowedTime == -1) {
				// set it.
				nextIdleFidgetAllowedTime = gpGlobals->time + RANDOM_FLOAT(11, 15);
			}

			if (gpGlobals->time < nextIdleFidgetAllowedTime) {
				// idle
				return LookupSequence("crouch_idle");
			}
			else {
				const int animationWeightTotal = 1 + 1;
				const int animationWeightChoice = RANDOM_LONG(0, animationWeightTotal - 1);

				nextIdleFidgetAllowedTime = gpGlobals->time + RANDOM_FLOAT(11, 15);

				// play a fidget
				if (animationWeightChoice < 1) {
					return LookupSequence("crouch_idle2");
				}
				else { //if(animationWeightChoice < 1 + 1){
					return LookupSequence("crouch_idle3");
				}
			}
		}break;


	}//END OF switch
	//not handled by above?  try the real deal.
	return CBaseAnimating::LookupActivity(activity);
}//END OF LookupActivityHard


int CScientist::tryActivitySubstitute(int activity){
	int iRandChoice = 0;
	int iRandWeightChoice = 0;
	
	char* animChoiceString = NULL;
	int* weightsAbs = NULL;
			
	//pev->framerate = 1;
	int maxRandWeight = 30;



	//no need for default, just falls back to the normal activity lookup.
	switch(activity){
		case ACT_MELEE_ATTACK2:
			return LookupSequence("punch");
		break;
		//No need for ACT_IDLE here. The point of tryActivitySubstitute is to
		//merely see if there is any sequence available for this activity, specifics don't help.

	}
	
	//not handled by above?
	return CBaseAnimating::LookupActivity(activity);
}




//MODDD TODO - NOTICE - the scientist is a little... backwards in this regard.
//The violent death anim is a backwards flip.  The ACT_DIEBACKWARDS though is noticably backing up and then falling forward.
//Perhaps even the forward / backward distances for success/fail of DIEFORWARDS / BACKWARDS should be virtual methods per BaseMonster child class too?

BOOL CScientist::violentDeathAllowed(void){
	return TRUE;
}
BOOL CScientist::violentDeathClear(void){
	//Works for a lot of things going backwards.
	return violentDeathClear_BackwardsCheck(90);
}//END OF violentDeathAllowed
int CScientist::violentDeathPriority(void){
	return 3;
}

// TEST. This is TRUE for most monsters.
BOOL CScientist::canPredictActRepeat(void) {
	return FALSE;
}


void CScientist::initiateAss(void){
	// not directly... do this and start the method when the think method runs next.
	// Yes this fucking joke requires some design decisions. What am I doing.
	explodeDelay = -3;
}
void CScientist::myAssHungers(void){
	ShutUpFriends();
	CBaseEntity* pEntityScan = NULL;
	CBaseEntity* testMon = NULL;
	float thisDistance;
	float leastDistanceYet;
	CTalkMonster* thisNameSucks;
	CTalkMonster* bestChoiceYet;
	//I'm number 1!
	CTalkMonster* pickedNumber2 = NULL;
	//does UTIL_MonstersInSphere work?
	while ((pEntityScan = UTIL_FindEntityInSphere( pEntityScan, pev->origin, 600 )) != NULL){
		testMon = pEntityScan->MyMonsterPointer();
		//if(testMon != NULL && testMon->pev != this->pev && ( FClassnameIs(testMon->pev, "monster_scientist") || FClassnameIs(testMon->pev, "monster_barney")  ) ){
		if(testMon != NULL && testMon->pev != this->pev && UTIL_IsAliveEntity(testMon) && testMon->isTalkMonster() ){
			thisDistance = (testMon->pev->origin - pev->origin).Length();
			thisNameSucks = static_cast<CTalkMonster*>(testMon);
			if(pickedNumber2 == NULL){
				pickedNumber2 = thisNameSucks;
				break;
			}
		}
	}//END OF while(...)
	if(pickedNumber2 != NULL){
		//Other one will look at me.
		pickedNumber2->PlaySentenceNoPitchTo("!meme_my_ass_hungers_a", 21, 1.0, ATTN_NORM, TRUE, this);
		//I won't look at him.
		this->PlaySentenceNoPitchUninterruptable("!meme_my_ass_hungers_b", 21, 1.0, ATTN_NORM, TRUE);
		this->explodeDelay = gpGlobals->time + 14.1 - 0.8;
	}
}//END OF myAssHungers

