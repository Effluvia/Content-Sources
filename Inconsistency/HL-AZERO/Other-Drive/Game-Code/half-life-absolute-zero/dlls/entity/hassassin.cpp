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


//ANIMATION COMMENT. Why are fly_down and fly_attack the exact same animation? May have been something preserved in retail.
//                   They are both for being midair and moving downwards, but fly_attack has events for firing while going downwards.
//                   Even though it can only happen once since this crosshow needs reloading between single use.

//MODDD TODO - the melee attack seems to be completely unused, even as of retail there wasn't any case of checking for an area in front to do a
//             melee attack (CheckTraceHullAttack, TakeDamage, punchangle, velocity shove).
//             So the melee attack anim went completely unused and wouldn't do anything (physically) even if it did play.
//             Check to see if it has any events in HandleAnimEvent if played at all, and if not can create our own hardcoded ones the usual way.

//MODDD TODO - FUTURE. Perhaps the HAssassin should do a check for  nearby undetonated grenades, player or hgrunt / hassassin-thrown, 
//                     and make an effort not to jump into their blast radius? Looks silly when that happens.

//MODDD - TODO. Notice that SCHED_INVESTIGATE_SOUND goes unused in most places. Could it be part of more human / intelligent alien NPC GetSchedule's?

#pragma once

//=========================================================
// hassassin - Human assassin, fast and stealthy
//=========================================================

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "basemonster.h"
#include "schedule.h"
#include "squadmonster.h"
#include "weapons.h"
#include "soundent.h"
#include "game.h"
#include "crossbowbolt.h"
#include "util_debugdraw.h"


//#define HASSASSIN_CROSSBOW_RELOAD_APPLY_DELAY 1.1
EASY_CVAR_EXTERN_DEBUGONLY(hassassinCrossbowReloadSoundDelay)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(thatWasntPunch)
EASY_CVAR_EXTERN_DEBUGONLY(hassassinCrossbowDebug)


extern DLL_GLOBAL int g_iSkillLevel;

#define HASSASSIN_CROSSBOW_RELOAD_ANIM "reload"
#define bits_MEMORY_BADJUMP		(bits_MEMORY_CUSTOM1)

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define ASSASSIN_AE_SHOOT1	1
#define ASSASSIN_AE_TOSS1	2
#define ASSASSIN_AE_JUMP	3


#define BODYGROUP_GUN 1
#define GUN_CROSSBOW 0
#define GUN_NONE 1






//idle1 looks like we're kinda bored and fidgeting with the crossbow.
//idle2 is crouched over like in hiding, or maybe surprised at being found.
//idle 3 is just standing.

enum hAssassin_sequence {
	HASSASSIN_IDLE1_CROSSBOW, //181, 30
	HASSASSIN_IDLE3, //61, 30
	HASSASSIN_IDLE2, //31, 22
	HASSASSIN_RUN,  //21, 40
	HASSASSIN_WALK,  //82, 35
	HASSASSIN_SHOOT,  //9, 30
	HASSASSIN_RELOAD, //31, 30
	HASSASSIN_GRENADETHROW, //16, 30
	HASSASSIN_KICK, //33, 35
	HASSASSIN_KICKSHORT, //22, 30
	HASSASSIN_DEATH_DURING_RUN,  //121, 37
	HASSASSIN_DIE_BACKWARDS, //23, 24
	HASSASSIN_DIE_SIMPLE, //61, 28
	HASSASSIN_JUMP, //19, 25
	HASSASSIN_FLY_UP, //12, 25
	HASSASSIN_FLY_DOWN, //26, 30
	HASSASSIN_FLY_ATTACK, //26, 30
	HASSASSIN_LANDFROMJUMP, //26, 30
	HASSASSIN_VICTORY_IDLE2, //61, 15

};








//=========================================================
// monster-specific schedule types
//=========================================================
enum
{
	SCHED_ASSASSIN_EXPOSED = LAST_COMMON_SCHEDULE + 1,// cover was blown.
	SCHED_ASSASSIN_JUMP,	// fly through the air
	SCHED_ASSASSIN_JUMP_ATTACK,	// fly through the air and shoot
	SCHED_ASSASSIN_JUMP_LAND, // hit and run away
	SCHED_ASSASSIN_TAKE_COVER_RELOAD,   //MODDD - new
	SCHED_ASSASSIN_RELOAD,
	SCHED_ASSASSIN_RANGE_ATTACK1_CANRELOAD,
	SCHED_ASSASSIN_RANGE_ATTACK2_CANRELOAD,  //special, to allow reloading in case the grenade can't work.
};

//=========================================================
// monster-specific tasks
//=========================================================

enum
{
	TASK_ASSASSIN_FALL_TO_GROUND = LAST_COMMON_TASK + 1, // falling and waiting to hit ground
};







class CHAssassin : public CBaseMonster
{
public:
	CHAssassin();
	
	void MonsterThink(void);
	int IRelationship ( CBaseEntity *pTarget );
	


	void EXPORT HAssassinTouch ( CBaseEntity *pOther );
	
	void Spawn( void );
	void Precache( void );
	void SetYawSpeed ( void );
	int  Classify ( void );
	int  ISoundMask ( void);
	void Shoot( void );

	
	GENERATE_TRACEATTACK_PROTOTYPE
	GENERATE_TAKEDAMAGE_PROTOTYPE

	GENERATE_KILLED_PROTOTYPE
	
	GENERATE_GIBMONSTER_PROTOTYPE
	
	BOOL attemptDropWeapon(void);

	BOOL canResetBlend0(void);
	BOOL onResetBlend0(void);




	void HandleAnimEvent( MonsterEvent_t *pEvent );
	Schedule_t* GetSchedule ( void );
	Schedule_t* GetScheduleOfType ( int Type );

	//MODDD - melee attack #1 used to be jump. Now that's #2, #1 is a real melee attack.
	BOOL CheckMeleeAttack1 ( float flDot, float flDist );	// melee attack
	BOOL CheckMeleeAttack2 ( float flDot, float flDist );   // jump
	BOOL CheckRangeAttack1 ( float flDot, float flDist );	// shoot
	BOOL CheckRangeAttack2 ( float flDot, float flDist );	// throw grenade
	void StartTask ( Task_t *pTask );
	void RunAI( void );
	void RunTask ( Task_t *pTask );
	void DeathSound ( void );
	void IdleSound ( void );
	
	Vector GetGunPosition(void);
	Vector GetGunPositionAI(void);
	
	CUSTOM_SCHEDULES;

	int Save( CSave &save ); 
	int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];


	
	BOOL usesAdvancedAnimSystem(void);
	int tryActivitySubstitute(int activity);
	int LookupActivityHard(int activity);
	
	void HandleEventQueueEvent(int arg_eventID);

	BOOL predictRangeAttackEnd(void);

	float m_flLastShot;
	float m_flDiviation;

	float m_flNextJump;
	//MODDD - longer alternate time.  The original above is for physically refusing to jump that often, this is just "I'd rather not yet".
	float m_flNextJumpPrefer;

	Vector m_vecJumpVelocity;

	float m_flNextGrenadeCheck;
	Vector	m_vecTossVelocity;
	BOOL	m_fThrowGrenade;

	int	m_iTargetRanderamt;

	int	m_iFrustration;

	int	m_iShell;

	//NEW
	float meleeAttackCooldown;
	float reloadApplyTime;
	BOOL droppedWeapon;
	float groundTouchCheckDuration;


};










//=========================================================
// AI Schedules Specific to this monster
//=========================================================

//=========================================================
// Fail Schedule
//=========================================================
Task_t	tlAssassinFail[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_WAIT_FACE_ENEMY,		(float)2		},
	// { TASK_WAIT_PVS,			(float)0		},
	{ TASK_SET_SCHEDULE,		(float)SCHED_CHASE_ENEMY },
};

Schedule_t	slAssassinFail[] =
{
	{
		tlAssassinFail,
		ARRAYSIZE ( tlAssassinFail ),
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_PROVOKED			|
		bits_COND_CAN_RANGE_ATTACK1 |
		bits_COND_CAN_RANGE_ATTACK2 |
		bits_COND_CAN_MELEE_ATTACK1 |
		bits_COND_CAN_MELEE_ATTACK2 |
		bits_COND_HEAR_SOUND,
	
		bits_SOUND_DANGER |
		bits_SOUND_PLAYER,
		"AssassinFail"
	},
};


//=========================================================
// Enemy exposed Agrunt's cover
// (MODDD NOTE - perhaps you mean assassin's? dang'd copy+pasted comments)
//=========================================================
Task_t	tlAssassinExposed[] =
{
	{ TASK_STOP_MOVING,			(float)0							},
	
	//Set activity to looking ready to fire?
	//MODDD - removed. We don't seem to have any activity mapped to IDLE_ANGRY.
	//{ TASK_SET_ACTIVITY,		(float)ACT_IDLE_ANGRY },
	
	//...but setting the activity at all is wise at least.
	{ TASK_SET_ACTIVITY,		    (float)ACT_COMBAT_IDLE			    },
	{ TASK_WAIT,  (float) 0.2										},


	//MODDD - Woa  hey, at least face the enemy first! How are you firing that crossbow, even pistol before sideways?
	{ TASK_FACE_ENEMY,			(float)0							},
	{ TASK_WAIT,  (float) 0.3										},

	//Special task. This will FAIL the schedule if we are unable to make a ranged attack.
	{ TASK_CHECK_RANGED_ATTACK_1,(float)0							},

	{ TASK_RANGE_ATTACK1,		(float)0							},


	{ TASK_SET_FAIL_SCHEDULE,	(float)SCHED_ASSASSIN_JUMP			},

	{ TASK_SET_SCHEDULE,		(float)SCHED_TAKE_COVER_FROM_ENEMY	},
};

Schedule_t slAssassinExposed[] =
{
	{
		tlAssassinExposed,
		ARRAYSIZE ( tlAssassinExposed ),
		bits_COND_CAN_MELEE_ATTACK1 |
		bits_COND_CAN_MELEE_ATTACK2,
		0,
		"AssassinExposed",
	},
};


//=========================================================
// Take cover from enemy! Tries lateral cover before node 
// cover! 
//=========================================================
Task_t	tlAssassinTakeCoverFromEnemy[] =
{
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_WAIT,					(float)0.2					},
	{ TASK_SET_FAIL_SCHEDULE,		(float)SCHED_ASSASSIN_RANGE_ATTACK1_CANRELOAD	},
	{ TASK_FIND_COVER_FROM_ENEMY,	(float)0					},
	{ TASK_RUN_PATH,				(float)0					},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0					},
	{ TASK_REMEMBER,				(float)bits_MEMORY_INCOVER	},
	{ TASK_FACE_ENEMY,				(float)0					},
};

Schedule_t	slAssassinTakeCoverFromEnemy[] =
{
	{ 
		tlAssassinTakeCoverFromEnemy,
		ARRAYSIZE ( tlAssassinTakeCoverFromEnemy ), 
		bits_COND_NEW_ENEMY |
		bits_COND_CAN_MELEE_ATTACK1		|
		bits_COND_CAN_MELEE_ATTACK2		|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"AssassinTakeCoverFromEnemy"
	},
};


//=========================================================
// Take cover from enemy! Tries lateral cover before node 
// cover! 
//=========================================================
Task_t	tlAssassinTakeCoverFromEnemy2[] =
{
	{ TASK_STOP_MOVING,				(float)0					},
	
	//We need a little time to turn now, don't wait.
	//{ TASK_WAIT,					(float)0.2					},

	{ TASK_FACE_ENEMY,				(float)0					},
	{ TASK_RANGE_ATTACK1,			(float)0					},
	//in case this "takeCover" fails, our only option may be to just start reloading.
	{ TASK_SET_FAIL_SCHEDULE,		(float)SCHED_ASSASSIN_RANGE_ATTACK2_CANRELOAD	},
	{ TASK_FIND_FAR_NODE_COVER_FROM_ENEMY,	(float)24			},
	{ TASK_RUN_PATH,				(float)0					},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0					},
	{ TASK_REMEMBER,				(float)bits_MEMORY_INCOVER	},
	{ TASK_FACE_ENEMY,				(float)0					},
};

Schedule_t	slAssassinTakeCoverFromEnemy2[] =
{
	{ 
		tlAssassinTakeCoverFromEnemy2,
		ARRAYSIZE ( tlAssassinTakeCoverFromEnemy2 ), 
		bits_COND_NEW_ENEMY |

		//MODDD - wait. This was originally interruptable only by MELEE_ATTACK2 despite not even having a melee attack #2? Jump used to be melee attack #1. Curious.
		//        Perhaps this wasn't meant be interruptible by being able to jump.  Changing to being interruptable by melee only now (the new #1).
		//bits_COND_CAN_MELEE_ATTACK2		|
		bits_COND_CAN_MELEE_ATTACK1		|

		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"AssassinTakeCoverFromEnemy2"
	},
};


//=========================================================
// hide from the loudest sound source
//=========================================================
Task_t	tlAssassinTakeCoverFromBestSound[] =
{
	{ TASK_SET_FAIL_SCHEDULE,			(float)SCHED_MELEE_ATTACK2	},
	{ TASK_STOP_MOVING,					(float)0					},
	{ TASK_FIND_COVER_FROM_BEST_SOUND,	(float)0					},
	{ TASK_RUN_PATH,					(float)0					},
	{ TASK_WAIT_FOR_MOVEMENT,			(float)0					},
	{ TASK_REMEMBER,					(float)bits_MEMORY_INCOVER	},
	{ TASK_TURN_LEFT,					(float)179					},
};

Schedule_t	slAssassinTakeCoverFromBestSound[] =
{
	{ 
		tlAssassinTakeCoverFromBestSound,
		ARRAYSIZE ( tlAssassinTakeCoverFromBestSound ), 
		bits_COND_NEW_ENEMY,
		0,
		"AssassinTakeCoverFromBestSound"
	},
};





//=========================================================
// AlertIdle Schedules
//=========================================================
Task_t	tlAssassinHide[] =
{
	{ TASK_STOP_MOVING,			0						 },
	//MODDD - look like we're trying to hide at least, crouched is better specifically from COMBAT_IDLE
	//{ TASK_SET_ACTIVITY,		(float)ACT_IDLE			 },
	{ TASK_SET_ACTIVITY,		(float)ACT_COMBAT_IDLE			 },
	{ TASK_WAIT,				(float)5				 },
	{ TASK_SET_SCHEDULE,		(float)SCHED_CHASE_ENEMY },
};

Schedule_t	slAssassinHide[] =
{
	{ 
		tlAssassinHide,
		ARRAYSIZE ( tlAssassinHide ), 
		bits_COND_NEW_ENEMY				|
		bits_COND_SEE_ENEMY				|
		bits_COND_SEE_FEAR				|
		bits_COND_LIGHT_DAMAGE			|
		bits_COND_HEAVY_DAMAGE			|
		bits_COND_PROVOKED		|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"AssassinHide"
	},
};



//=========================================================
// HUNT Schedules
//=========================================================
Task_t tlAssassinHunt[] = 
{
	//{ TASK_GET_PATH_TO_ENEMY,	(float)0		},
	//{ TASK_RUN_PATH,			(float)0		},

	//smart versio.
	
	{TASK_MOVE_TO_ENEMY_RANGE, (float)0					},
	{TASK_CHECK_STUMPED, (float)0						},

	{ TASK_WAIT_FOR_MOVEMENT,	(float)0		},
};

Schedule_t slAssassinHunt[] =
{
	{ 
		tlAssassinHunt,
		ARRAYSIZE ( tlAssassinHunt ),
		bits_COND_NEW_ENEMY			|
		// bits_COND_SEE_ENEMY			|
		bits_COND_CAN_RANGE_ATTACK1	|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"AssassinHunt"
	},
};


//=========================================================
// Jumping Schedules
//=========================================================
Task_t	tlAssassinJump[] =
{
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_PLAY_SEQUENCE,		(float)ACT_HOP	},
	{ TASK_SET_SCHEDULE,		(float)SCHED_ASSASSIN_JUMP_ATTACK },
};

Schedule_t	slAssassinJump[] =
{
	{ 
		tlAssassinJump,
		ARRAYSIZE ( tlAssassinJump ), 
		0, 
		0, 
		"AssassinJump"
	},
};


//MODDD TODO - would it be good to give the hassassin some sort of ariel kick attack or whatever this was meant to be...?
//=========================================================
// repel 
//=========================================================
Task_t	tlAssassinJumpAttack[] =
{
	{ TASK_SET_FAIL_SCHEDULE,	(float)SCHED_ASSASSIN_JUMP_LAND	},
	// { TASK_SET_ACTIVITY,		(float)ACT_FLY	},
	{ TASK_ASSASSIN_FALL_TO_GROUND, (float)0		},
};


Schedule_t	slAssassinJumpAttack[] =
{
	{ 
		tlAssassinJumpAttack,
		ARRAYSIZE ( tlAssassinJumpAttack ), 
		0, 
		0,
		"AssassinJumpAttack"
	},
};


//=========================================================
// repel 
//=========================================================
Task_t	tlAssassinJumpLand[] =
{
	{ TASK_SET_FAIL_SCHEDULE,		(float)SCHED_ASSASSIN_EXPOSED	},
	// { TASK_SET_FAIL_SCHEDULE,		(float)SCHED_MELEE_ATTACK1	},
	{ TASK_SET_ACTIVITY,			(float)ACT_IDLE				},
	{ TASK_REMEMBER,				(float)bits_MEMORY_BADJUMP	},
	{ TASK_FIND_NODE_COVER_FROM_ENEMY,	(float)0					},
	{ TASK_RUN_PATH,				(float)0					},
	{ TASK_FORGET,					(float)bits_MEMORY_BADJUMP	},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0					},
	{ TASK_REMEMBER,				(float)bits_MEMORY_INCOVER	},
	{ TASK_FACE_ENEMY,				(float)0					},
	{ TASK_SET_FAIL_SCHEDULE,		(float)SCHED_RANGE_ATTACK1	},
};

Schedule_t	slAssassinJumpLand[] =
{
	{ 
		tlAssassinJumpLand,
		ARRAYSIZE ( tlAssassinJumpLand ), 
		0, 
		0,
		"AssassinJumpLand"
	},
};





Task_t	tlAssassinTakeCoverReload[] =
{
	{ TASK_SET_ACTIVITY,		    (float)ACT_COMBAT_IDLE			    },
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_WAIT,					(float)0.2					},
	{ TASK_SET_FAIL_SCHEDULE,		(float)SCHED_RANGE_ATTACK1	},
	{ TASK_FIND_COVER_FROM_ENEMY,	(float)0					},
	{ TASK_RUN_PATH,				(float)0					},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0					},
	{ TASK_REMEMBER,				(float)bits_MEMORY_INCOVER	},
	{ TASK_FACE_ENEMY,				(float)0					},
	//reload!
	{ TASK_RELOAD,					(float)0					},
};

Schedule_t	slAssassinTakeCoverReload[] =
{
	{ 
		tlAssassinTakeCoverReload,
		ARRAYSIZE ( tlAssassinTakeCoverReload ), 
		bits_COND_NEW_ENEMY |
		bits_COND_CAN_MELEE_ATTACK1		|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"AssassinTakeCoverReload"
	},
};



Task_t	tlAssassinReload[] =
{
	{ TASK_SET_ACTIVITY,		    (float)ACT_COMBAT_IDLE			    },
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_WAIT,					(float)0.2					},
	//{ TASK_SET_FAIL_SCHEDULE,		(float)SCHED_RANGE_ATTACK1	},
	//{ TASK_FIND_COVER_FROM_ENEMY,	(float)0					},
	//{ TASK_RUN_PATH,				(float)0					},
	//{ TASK_WAIT_FOR_MOVEMENT,		(float)0					},
	//{ TASK_REMEMBER,				(float)bits_MEMORY_INCOVER	},
	//{ TASK_FACE_ENEMY,				(float)0					},
	//reload!
	{ TASK_RELOAD,					(float)0					},
};

Schedule_t	slAssassinReload[] =
{
	{ 
		tlAssassinReload,
		ARRAYSIZE ( tlAssassinReload ), 
		bits_COND_NEW_ENEMY |
		bits_COND_CAN_MELEE_ATTACK1		|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"AssassinReload"
	},
};





//MODDD - cloned from the generic MeleeAttack1 in defaultai.cpp
Task_t	tlAssassinMeleeAttack[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_MELEE_ATTACK1,		(float)0		},
};

Schedule_t	slAssassinMeleeAttack[] =
{
	{ 
		tlAssassinMeleeAttack,
		ARRAYSIZE ( tlAssassinMeleeAttack ), 
		bits_COND_NEW_ENEMY			|
		bits_COND_ENEMY_DEAD		|
		
		//MODDD - restoring heavy damage as interruptable.
		//bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|

		bits_COND_ENEMY_OCCLUDED,
		0,
		"HAssassin Melee Attack"
	},
};


//MODDD - new. Clone of the slVictoryDance schedule in defaultai.cpp, but easier to interrupt.
Task_t tlAssassinVictoryDance[] =
{
	{ TASK_STOP_MOVING,			0							},
	{ TASK_PLAY_SEQUENCE,		(float)ACT_VICTORY_DANCE	},
	{ TASK_WAIT,				(float)0					},
};

Schedule_t slAssassinVictoryDance[] =
{
	{
		tlAssassinVictoryDance,
		ARRAYSIZE( tlAssassinVictoryDance ),
		bits_COND_NEW_ENEMY |
		bits_COND_SEE_ENEMY |
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE,
		0,
		"HAssassin Victory Dance"
	},
};







DEFINE_CUSTOM_SCHEDULES( CHAssassin )
{
	slAssassinFail,
	slAssassinExposed,
	slAssassinTakeCoverFromEnemy,
	slAssassinTakeCoverFromEnemy2,
	slAssassinTakeCoverFromBestSound,
	slAssassinHide,
	slAssassinHunt,
	slAssassinJump,
	slAssassinJumpAttack,
	slAssassinJumpLand,
	slAssassinTakeCoverReload, //MODDD - new
	slAssassinReload, //MODDD - new
	slAssassinMeleeAttack, //MODDD - me too
	slAssassinVictoryDance, //MODDD
};

IMPLEMENT_CUSTOM_SCHEDULES( CHAssassin, CBaseMonster );






















#if REMOVE_ORIGINAL_NAMES != 1
	LINK_ENTITY_TO_CLASS( monster_human_assassin, CHAssassin );
#endif

#if EXTRA_NAMES > 0
	LINK_ENTITY_TO_CLASS( hassassin, CHAssassin );

	#if EXTRA_NAMES == 2
		LINK_ENTITY_TO_CLASS( human_assassin, CHAssassin );
		LINK_ENTITY_TO_CLASS( monster_hassassin, CHAssassin );
	#endif
	
	//no extras.

#endif



TYPEDESCRIPTION	CHAssassin::m_SaveData[] = 
{
	DEFINE_FIELD( CHAssassin, m_flLastShot, FIELD_TIME ),
	DEFINE_FIELD( CHAssassin, m_flDiviation, FIELD_FLOAT ),

	DEFINE_FIELD( CHAssassin, m_flNextJump, FIELD_TIME ),
	DEFINE_FIELD( CHAssassin, m_flNextJumpPrefer, FIELD_TIME ),  //MODDD - new.
	DEFINE_FIELD( CHAssassin, m_vecJumpVelocity, FIELD_VECTOR ),

	DEFINE_FIELD( CHAssassin, m_flNextGrenadeCheck, FIELD_TIME ),
	DEFINE_FIELD( CHAssassin, m_vecTossVelocity, FIELD_VECTOR ),
	DEFINE_FIELD( CHAssassin, m_fThrowGrenade, FIELD_BOOLEAN ),

	DEFINE_FIELD( CHAssassin, m_iTargetRanderamt, FIELD_INTEGER ),
	DEFINE_FIELD( CHAssassin, m_iFrustration, FIELD_INTEGER ),
	DEFINE_FIELD( CHAssassin, droppedWeapon, FIELD_BOOLEAN ),   //MODDD - new. Have I dropped my weapon or not yet? Probably will only
	//                                                            be relevant for in the middle of a death animation. So that gibbing
	//                                                            after dropping a crossbow doesn't make another crossbow show up.

	
};

IMPLEMENT_SAVERESTORE( CHAssassin, CBaseMonster );




//SetTouch(&CChumToad::ChumToadTouch );




//MODDD - although more expensive, it may be best to move this in the MonsterThink method instead,
//or at least work on several seconds after recently touching anything to be safe then.
void CHAssassin::HAssassinTouch( CBaseEntity* pOther ){
	
	//easyForcePrintLine("OH no IM a person friend %s", pOther!=NULL?pOther->getClassname():"WTF");

	//CHEAP FIX:

	if(pOther == NULL){
		return; //??????
	}

	groundTouchCheckDuration = gpGlobals->time + 4;
	

}//END OF HAssassinTouch




void CHAssassin::MonsterThink(){

	//If we touched something a little while ago, check below for the ground for landing.
	if(groundTouchCheckDuration != -1){

		if(gpGlobals->time <= groundTouchCheckDuration){
			if(pev->movetype == MOVETYPE_TOSS){
				//MODDD TODO - a better check would be just to see if we've landed on anything that counts as a ground. Do a linetrace downwards and pass if it looks to be the case?
				TraceResult tr;
				UTIL_TraceLine( pev->origin, pev->origin + Vector(0, 0, -7), dont_ignore_monsters, dont_ignore_glass, ENT(pev), &tr);
		
				//if(FClassnameIs(pOther->pev, "worldspawn")){
				//if(tr.pHit != NULL && FClassnameIs(tr.pHit, "worldspawn")){
				if(tr.flFraction < 1.0){
					//hit something? land.
					if(m_pSchedule == slAssassinJump || m_pSchedule == slAssassinJumpAttack){
						//jumping schedules need to reset.
						TaskFail();
						pev->movetype = MOVETYPE_STEP;
						ChangeSchedule ( GetScheduleOfType(SCHED_ASSASSIN_JUMP_LAND) );
					}
				}
			}//END OF MOVETYPE_TOSS check
		}else{
			groundTouchCheckDuration = -1;
		}

	}


	//easyForcePrintLine("HASSASSIN ANIM: ID:%d seq:%d fr:%.2f", monsterID, pev->sequence, pev->frame);

	//easyForcePrintLine("AMMO??? %d", m_cAmmoLoaded);

	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(thatWasntPunch) == 1 && this->m_fSequenceFinished){

			switch(RANDOM_LONG(0, 20)){
			case 0:SetSequenceByName("idle2");break;
			case 1:SetSequenceByName("idle2");break;
			case 2:SetSequenceByName("grenadethrow");break;
			case 3:SetSequenceByName("grenadethrow");break;
			case 4:SetSequenceByName("grenadethrow");break;
			case 5:SetSequenceByName("grenadethrow");break;
			case 6:SetSequenceByName("grenadethrow");break;
			case 7:SetSequenceByName("grenadethrow");break;
			case 8:SetSequenceByName("kick");break;
			case 9:SetSequenceByName("kick");break;
			case 10:SetSequenceByName("kick");break;
			case 11:SetSequenceByName("kickshort");break;
			case 12:SetSequenceByName("jump");break;
			case 13:SetSequenceByName("jump");break;
			case 14:SetSequenceByName("fly_up");break;
			case 15:SetSequenceByName("fly_up");break;
			case 16:SetSequenceByName("fly_down");break;
			case 17:SetSequenceByName("fly_down");break;
			case 18:SetSequenceByName("land_from_jump");break;
			case 19:SetSequenceByName("land_from_jump");break;
			case 20:SetSequenceByName("land_from_jump");break;
			}

	}


	CBaseMonster::MonsterThink();
}


int CHAssassin::IRelationship ( CBaseEntity *pTarget )
{
	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(thatWasntPunch) == 1){
		//I just don't give a damn man
		return R_NO;
	}

	return CBaseMonster::IRelationship(pTarget);

}



//=========================================================
// DieSound
//=========================================================
void CHAssassin::DeathSound ( void )
{
}

//=========================================================
// IdleSound
//=========================================================
void CHAssassin::IdleSound ( void )
{
}

//=========================================================
// ISoundMask - returns a bit mask indicating which types
// of sounds this monster regards. 
//=========================================================
int CHAssassin::ISoundMask ( void) 
{
	return	bits_SOUND_WORLD	|
			bits_SOUND_COMBAT	|
			bits_SOUND_DANGER	|
			bits_SOUND_PLAYER	|
			//MODDD - new
			bits_SOUND_BAIT;
}


//=========================================================
// Classify - indicates this monster's place in the 
// relationship table.
//=========================================================
int CHAssassin::Classify ( void )
{
	return	CLASS_HUMAN_MILITARY;
}

//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CHAssassin::SetYawSpeed ( void )
{
	int ys;

	switch ( m_Activity )
	{
	case ACT_TURN_LEFT:
	case ACT_TURN_RIGHT:
		ys = 360;
		break;
	default:			
		ys = 360;
		break;
	}


	ys *= 1.67;  //to compensate for having to turn to evade the player now.

	pev->yaw_speed = ys;
}


//=========================================================
// Shoot
//=========================================================
void CHAssassin::Shoot ( void )
{
	// No, still shoot, just straight forward then.  Residual shots.
	//if (m_hEnemy == NULL)
	//{
	//	return;
	//}

	Vector vecShootOrigin = GetGunPosition();
	
	if (m_flLastShot + 16 < gpGlobals->time)
	{
		m_flDiviation = 0.07;
	}
	else
	{
		m_flDiviation -= 0.03;
		if (m_flDiviation < 0.01)
			m_flDiviation = 0.01;
	}
	m_flLastShot = gpGlobals->time;
	

	UTIL_MakeVectors ( pev->angles );

	/*
	DebugLine_Setup(0, Center() + Vector(0,0,6), Center() + Vector(0,0,6) + gpGlobals->v_forward * 400, 255, 0, 255);
	UTIL_MakeAimVectors ( pev->angles );
	DebugLine_Setup(1, Center() + Vector(0,0,-6), Center() + Vector(0,0,-6) + gpGlobals->v_forward * 400, 0, 255, 0);
	*/
	Vector vecShootDir = ShootAtEnemyMod( vecShootOrigin ) + Vector( RANDOM_FLOAT(-m_flDiviation, m_flDiviation), RANDOM_FLOAT(-m_flDiviation, m_flDiviation), RANDOM_FLOAT(-m_flDiviation, m_flDiviation) );
	Vector anglesAim = UTIL_VecToAngles( vecShootDir );



	/*
	Vector	vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT(40,90) + gpGlobals->v_up * RANDOM_FLOAT(75,200) + gpGlobals->v_forward * RANDOM_FLOAT(-40, 40);
	EjectBrass ( pev->origin + gpGlobals->v_up * 32 + gpGlobals->v_forward * 12, vecShellVelocity, pev->angles.y, m_iShell, TE_BOUNCE_SHELL); 
	FireBullets(1, vecShootOrigin, vecShootDir, Vector( m_flDiviation, m_flDiviation, m_flDiviation ), 2048, BULLET_MONSTER_9MM ); // shoot +-8 degrees

	switch(RANDOM_LONG(0,1))
	{
	case 0:
		UTIL_PlaySound(ENT(pev), CHAN_WEAPON, "weapons/pl_gun1.wav", RANDOM_FLOAT(0.6, 0.8), ATTN_NORM, 0, 100, FALSE);
		break;
	case 1:
		UTIL_PlaySound(ENT(pev), CHAN_WEAPON, "weapons/pl_gun2.wav", RANDOM_FLOAT(0.6, 0.8), ATTN_NORM, 0, 100, FALSE);
		break;
	}

	pev->effects |= EF_MUZZLEFLASH;
	*/


	
	//TODO: different fire sounds? don't know if we ever want this.
	switch(RANDOM_LONG(0,0)){
	    case 0:
	        UTIL_PlaySound(ENT(pev), CHAN_WEAPON, "weapons/xbow_fire1.wav", RANDOM_FLOAT(0.78, 0.94), ATTN_NORM, 0, 100, FALSE);
	    break;
	}
	Vector arrowVelocity;
	float arrowSpeed;

	if (this->pev->waterlevel == 3)
	{
		arrowVelocity = vecShootDir * BOLT_WATER_VELOCITY;
		arrowSpeed = BOLT_WATER_VELOCITY;
	}
	else
	{
		arrowVelocity = vecShootDir * BOLT_AIR_VELOCITY;
		arrowSpeed = BOLT_AIR_VELOCITY;
	}

	CCrossbowBolt *pBolt = CCrossbowBolt::BoltCreate(arrowVelocity, arrowSpeed);
	pBolt->pev->origin = vecShootOrigin;
	pBolt->pev->angles = anglesAim;
	pBolt->pev->owner = this->edict();

	
	pBolt->pev->avelocity.z = 10;



	//already have it: anglesAim
	//Vector angDir = UTIL_VecToAngles( vecShootDir );
	SetBlending( 0, anglesAim.x );

	m_cAmmoLoaded--;

	
	/*
	//we have to stop!
	//tlAssassinTakeCoverReload
	this->m_fSequenceLoops = FALSE;  //hacky hacky!
	TaskFail();
	ChangeSchedule(slAssassinTakeCoverReload);
	this->m_fSequenceLoops = FALSE;  //hacky hacky!
	*/

}


//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//
// Returns number of events handled, 0 if none.
//=========================================================
void CHAssassin::HandleAnimEvent( MonsterEvent_t *pEvent )
{
	//easyForcePrintLine("WHAT THE heck IS THiS stuff event:%d seq:%d fr:%.2f", pEvent->event, pev->sequence, pev->frame);

	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(thatWasntPunch) == 1){
		//Best not to.
		return;
	}

	//c2a3d

	//Not using the glock anymore. And wasn't it a silenced glock anyways? Whatever.
	pev->renderfx |= NOMUZZLEFLASH;

	switch( pEvent->event )
	{
	case ASSASSIN_AE_SHOOT1:
		{
			if(m_cAmmoLoaded > 0){
				Shoot( );
			}
			break;
		}
	case ASSASSIN_AE_TOSS1:
		{
			if (pev->framerate > 0) {
				this->m_fSequenceLoops = FALSE;

				UTIL_MakeVectors(pev->angles);
				CGrenade::ShootTimed(pev, pev->origin + gpGlobals->v_forward * 34 + Vector(0, 0, 32), m_vecTossVelocity, gSkillData.plrDmgHandGrenade, 2.0);

				m_flNextGrenadeCheck = gpGlobals->time + 6;// wait six seconds before even looking again to see if a grenade can be thrown.
				m_fThrowGrenade = FALSE;
			}
			// !!!LATER - when in a group, only try to throw grenade if ordered.
			break;
		}
	case ASSASSIN_AE_JUMP:
		{
			// ALERT( at_console, "jumping");
			UTIL_MakeAimVectors( pev->angles );
			pev->movetype = MOVETYPE_TOSS;
			pev->flags &= ~FL_ONGROUND;
			pev->velocity = m_vecJumpVelocity;

			//MODDD - increase the jump delay to increase the chances of a melee kick instead from reapproaching too soon.
			//        ...in a different way.
			m_flNextJump = gpGlobals->time + 3.0;
			m_flNextJumpPrefer = gpGlobals->time + 15.0;
			break;
		}
		//return; ..why was this here?
	default:
		CBaseMonster::HandleAnimEvent( pEvent );
		break;
	}
}

CHAssassin::CHAssassin(){
	reloadApplyTime = -1;
	groundTouchCheckDuration = -1;

	//can't hurt?
	meleeAttackCooldown = 0;
	m_flNextJump = 0;
	m_flNextJumpPrefer = 0;

}

//=========================================================
// Spawn
//=========================================================
void CHAssassin::Spawn()
{
	Precache( );

	SET_MODEL(ENT(pev), "models/hassassin.mdl");
	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	pev->classname = MAKE_STRING("monster_human_assassin");

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_RED;
	pev->effects		= 0;
	pev->health			= gSkillData.hassassinHealth;
	m_flFieldOfView		= VIEW_FIELD_WIDE; // indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState		= MONSTERSTATE_NONE;

	// automatically picks these up just from having ACT's for each (MELEE_ATTACK1 and 2), but being explicit doesn't hurt.
	m_afCapability		= bits_CAP_MELEE_ATTACK1 | bits_CAP_MELEE_ATTACK2 | bits_CAP_DOORS_GROUP;
	pev->friction		= 1;

	m_HackedGunPos		= Vector( 0, 24, 48 );


	pev->renderamt		= 255; //full.

	//MODDD - shouldn't this just start at whatever makes sense for the hassassin to become anyways?
	if(g_iSkillLevel != SKILL_HARD){
		//we're not cloaking.
		m_iTargetRanderamt	= 255;
	}else{
		//we are cloaking, go ahead.
		m_iTargetRanderamt	= 20;
	}

	pev->rendermode		= kRenderTransTexture;

	MonsterInit();
	

	//come loaded.
	m_cAmmoLoaded = 1;
	droppedWeapon = FALSE;
	SetBodygroup(BODYGROUP_GUN, GUN_CROSSBOW);  //holding the crossbow.


	SetTouch( &CHAssassin::HAssassinTouch );

}

extern int global_useSentenceSave;
//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CHAssassin::Precache()
{
	PRECACHE_MODEL("models/hassassin.mdl");

	global_useSentenceSave = TRUE;
	// If the sound is forced to be precached (avoid soundsentencesave) by the player,
	// let this call precache it too then.
	//PRECACHE_SOUND("weapons/pl_gun1.wav", TRUE);
	//PRECACHE_SOUND("weapons/pl_gun2.wav", TRUE);
	//MODDD - fires using this instead.
	PRECACHE_SOUND("weapons/xbow_fire1.wav", TRUE);
	PRECACHE_SOUND("weapons/xbow_reload1.wav", TRUE);


	PRECACHE_SOUND("debris/beamstart1.wav");


	//const char* what = CBaseMonster::pStandardAttackHitSounds[0];

	//MODDD - has a melee attack + the usual sounds now.
	// anything using "sizeof", a C call (maybe C++?) needs to have that thing defined or at least implemented in the same file.
	// why? no clue. But we certainly aren't doing this by calling CBaseMonster's arrays directly at least.
	// Just call a method from there to handle this instead. Never enough of those...
	//PRECACHE_SOUND_ARRAY(CBaseMonster::pStandardAttackMissSounds);
	//PRECACHE_SOUND_ARRAY(CBaseMonster::pStandardAttackHitSounds);
	precacheStandardMeleeAttackMissSounds();
	precacheStandardMeleeAttackHitSounds();



	global_useSentenceSave = FALSE;

	m_iShell = PRECACHE_MODEL ("models/shell.mdl");// brass shell
}	
	

//AI SCHEDULES MOVED TO THE TOP OF THIS FILE




//MODDD - clone of the hgrunt's CheckMeleeAttack1.
//        Also CheckMeleeAttack1 used to be for the jump back when there wasn't a melee attack at all (retail).
//        CheckMeleeAttack1 is now used for the melee attack. CheckMeleeAttack2 is for the jump  instead.
BOOL CHAssassin::CheckMeleeAttack1 ( float flDot, float flDist )
{
	CBaseMonster *pEnemy = NULL;

	//Melee attack gets a cooldown.  This allows a jump to happen immediately after.
	//Any melee attack should also reset the jump cooldown to possibly allow that immediately after.
	//And being unable to jump bypasses the melee cooldown. If it's all we can do it's better than standing and
	//looking stupid.
	if(gpGlobals->time < meleeAttackCooldown && !(this->HasConditions(bits_COND_CAN_MELEE_ATTACK2)) ){
		return FALSE;
	}


	if ( m_hEnemy != NULL )
	{
		pEnemy = m_hEnemy->MyMonsterPointer();
	}
	if ( !pEnemy )
	{
		return FALSE;
	}


	//MODDD - extra. If I am able to jump (cool down has passed, space above, etc.), and I am certain I could jump instead of this was turned down
	//        (has the COND_MELEE_ATTACK2 on), make this false to allow that to happen instead.
	//        ...too compilcated, just put this preference in choosing the melee attack or jump schedules at GetSchedule or GetScheduleOfType instead.
	/*
	if(this->HasConditionsFrame(bits_COND_MELEE_ATTACK2)){
		return FALSE;
	}
	*/


	if ( flDist <= 64 && flDot >= 0.7	&&
		 pEnemy->Classify() != CLASS_ALIEN_BIOWEAPON &&
		 pEnemy->Classify() != CLASS_PLAYER_BIOWEAPON )
	{
		return TRUE;
	}
	return FALSE;
}




//=========================================================
// CheckMeleeAttack2 - jump like crazy if the enemy gets too close. 
//=========================================================
BOOL CHAssassin::CheckMeleeAttack2 ( float flDot, float flDist )
{

	if(m_hEnemy == NULL){
		return FALSE;
	}

	//MODDD - using m_flNextJumpPrefer instead of the original m_flNextJump.
	//        m_flNextJumpPrefer has a longer cooldown timer to give melee more of a chance, and m_flNextJump is still used for emergency jumps,
	//        like failing to find cover.  A melee attack isn't always a feasible substitution for jumping, so jumping sooner for cover reasons is fine.
	if ( m_flNextJumpPrefer < gpGlobals->time && (flDist <= 128 || HasMemory( bits_MEMORY_BADJUMP )) )
	{
		TraceResult	tr;

		Vector vecDest = pev->origin + Vector( RANDOM_FLOAT( -64, 64), RANDOM_FLOAT( -64, 64 ), 160 );

		UTIL_TraceHull( pev->origin + Vector( 0, 0, 36 ), vecDest + Vector( 0, 0, 36 ), dont_ignore_monsters, human_hull, ENT(pev), &tr);

		if ( !tr.fStartSolid && (tr.flFraction >= 1.0 || (tr.pHit != NULL && tr.pHit != m_hEnemy.Get() )) )
		{
			//accetpable.
		}else{
			return FALSE;
		}

		float flGravity = g_psv_gravity->value;

		float time = sqrt( 160 / (0.5 * flGravity));
		float speed = flGravity * time / 160;
		m_vecJumpVelocity = (vecDest - pev->origin) * speed;

		return TRUE;
	}
	return FALSE;
}


//=========================================================
// CheckRangeAttack1  - drop a cap in their ass
//
//=========================================================
BOOL CHAssassin::CheckRangeAttack1 ( float flDot, float flDist )
{

	if(m_hEnemy == NULL){
		return FALSE;
	}

	if(m_cAmmoLoaded < 0){
		//you have to reload silly!
		return FALSE;
	}
	
	Vector vecShootOrigin = GetGunPositionAI();

	//NoFriendlyFireImp(vecShootOrigin, m_hEnemy->BodyTargetMod(vecShootOrigin)
	//MODDD - you do too need a dotproduct check!
	//if ( !HasConditions( bits_COND_ENEMY_OCCLUDED ) && flDist > 64 && flDist <= 2048 /* && flDot >= 0.5 */ /* && NoFriendlyFire() */ )
	if ( !HasConditions( bits_COND_ENEMY_OCCLUDED ) && flDist > 64 && flDist <= 2048  && flDot >= 0.5  && NoFriendlyFireImp(vecShootOrigin, m_hEnemy->BodyTargetMod(vecShootOrigin))  )
	{
		TraceResult	tr;

		Vector vecSrc = GetGunPositionAI();

		// verify that a bullet fired from the gun will hit the enemy before the world.
		UTIL_TraceLine( vecSrc, m_hEnemy->BodyTargetMod(vecSrc), dont_ignore_monsters, ENT(pev), &tr);

		if ( !tr.fStartSolid && (tr.flFraction >= 1 || tr.pHit == m_hEnemy->edict()) )
		{
			return TRUE;
		}
	}
	return FALSE;
}

//=========================================================
// CheckRangeAttack2 - toss grenade is enemy gets in the way and is too close. 
//=========================================================
BOOL CHAssassin::CheckRangeAttack2 ( float flDot, float flDist )
{
	m_fThrowGrenade = FALSE;
	if ( !FBitSet ( m_hEnemy->pev->flags, FL_ONGROUND ) )
	{
		// don't throw grenades at anything that isn't on the ground!
		return FALSE;
	}

	// don't get grenade happy unless the player starts to piss you off
	if ( m_iFrustration <= 2)
		return FALSE;

	if ( m_flNextGrenadeCheck < gpGlobals->time && !HasConditions( bits_COND_ENEMY_OCCLUDED ) && flDist <= 512 /* && flDot >= 0.5 */ /* && NoFriendlyFire() */ )
	{
		Vector vecToss = VecCheckThrow( pev, GetGunPositionAI( ), m_hEnemy->Center(), flDist, 0.5 ); // use dist as speed to get there in 1 second

		if ( vecToss != g_vecZero )
		{
			m_vecTossVelocity = vecToss;

			// throw a hand grenade
			m_fThrowGrenade = TRUE;

			return TRUE;
		}
	}

	return FALSE;
}


//=========================================================
// RunAI
//=========================================================
void CHAssassin::RunAI( void )
{
	CBaseMonster::RunAI();

	// always visible if moving
	// always visible is not on hard

	//MODDD - also, this cheat CVar being on means we're definitely visible.
	if (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(thatWasntPunch) == 1 || (g_iSkillLevel != SKILL_HARD || m_hEnemy == NULL || pev->deadflag != DEAD_NO || m_Activity == ACT_RUN || m_Activity == ACT_WALK || !(pev->flags & FL_ONGROUND) )  )
		m_iTargetRanderamt = 255;
	else
		m_iTargetRanderamt = 20;

	if (pev->renderamt > m_iTargetRanderamt)
	{
		//
		//easyPrintLine("HA %.2f %.2f", pev->renderamt, m_iTargetRanderamt);

		//VISIBLE, MAKE NOISE TO SIGNAL START OF LOWERING!
		if (pev->renderamt == 255)
		{
			UTIL_PlaySound(ENT(pev), CHAN_BODY, "debris/beamstart1.wav", 0.2, ATTN_NORM, 0, 100, TRUE );
		}

		pev->renderamt = max( pev->renderamt - 50, m_iTargetRanderamt );
		pev->rendermode = kRenderTransTexture;
	}
	else if (pev->renderamt < m_iTargetRanderamt)
	{
		pev->renderamt = min( pev->renderamt + 50, m_iTargetRanderamt );
		if (pev->renderamt == 255)
			pev->rendermode = kRenderNormal;
	}



	/*
	//even ACT_WALK? isn't that a tiny bit obnoxious?
	if (m_Activity == ACT_RUN)  // || m_Activity == ACT_WALK)
	{
		static int iStep = 0;
		iStep = ! iStep;
		if (iStep)
		{
			//MODDD - These are already hard-precached by the player without the soundsentencesave system, regardless of its setting.
			//        May as well take advantage of this.
			switch( RANDOM_LONG( 0, 3 ) )
			{
			case 0:	UTIL_PlaySound( ENT(pev), CHAN_BODY, "player/pl_step1.wav", 0.5, ATTN_NORM, FALSE);	break;
			case 1:	UTIL_PlaySound( ENT(pev), CHAN_BODY, "player/pl_step3.wav", 0.5, ATTN_NORM, FALSE);	break;
			case 2:	UTIL_PlaySound( ENT(pev), CHAN_BODY, "player/pl_step2.wav", 0.5, ATTN_NORM, FALSE);	break;
			case 3:	UTIL_PlaySound( ENT(pev), CHAN_BODY, "player/pl_step4.wav", 0.5, ATTN_NORM, FALSE);	break;
			}
		}
	}
	*/
}


//=========================================================
// StartTask
//=========================================================
void CHAssassin::StartTask ( Task_t *pTask )
{

	const char* schedName = getScheduleName();
	int taskNumberr = pTask->iTask;
	//easyForcePrintLine("%s:%d StartTask sched:%s task:%d", getClassname(), monsterID, schedName, taskNumberr);

	switch ( pTask->iTask )
	{
	case TASK_MELEE_ATTACK1:{
		meleeAttackCooldown = gpGlobals->time + 4;
		
		//Allow jumping immediately after one melee attack.
		m_flNextJump = gpGlobals->time;
		m_flNextJumpPrefer = gpGlobals->time;

		CBaseMonster::StartTask(pTask);
	break;}

	case TASK_RANGE_ATTACK1:
		{
		
			//CBaseMonster::StartTask ( pTask );

		
			if(m_cAmmoLoaded > 0){
				//MODDD - interesting... why was this sequence ever marked to loop if the hassassin can only fire one shot
				//        before reloading (changing anims)?
				
				if(EASY_CVAR_GET_DEBUGONLY(hassassinCrossbowDebug) != 1){
					m_fSequenceLoops = FALSE;
				}

				CBaseMonster::StartTask ( pTask );
			}else{
				//nope? skip this.
				TaskComplete();
			}
		
		break;
		}
	case TASK_RELOAD:
	{

		//this->SetSequenceByName(HASSASSIN_CROSSBOW_RELOAD_ANIM);

		reloadApplyTime = gpGlobals->time + EASY_CVAR_GET_DEBUGONLY(hassassinCrossbowReloadSoundDelay);

		CBaseMonster::StartTask(pTask);

		break;
	}

	case TASK_RANGE_ATTACK2:
		if (!m_fThrowGrenade)
		{
			TaskComplete( );
		}
		else
		{
			CBaseMonster::StartTask ( pTask );
		}
		break;
	case TASK_ASSASSIN_FALL_TO_GROUND:
		break;
	default:
		CBaseMonster::StartTask ( pTask );
		break;
	}
}












GENERATE_TRACEATTACK_IMPLEMENTATION(CHAssassin)
{
	GENERATE_TRACEATTACK_PARENT_CALL(CBaseMonster);
}

GENERATE_TAKEDAMAGE_IMPLEMENTATION(CHAssassin)
{
	return GENERATE_TAKEDAMAGE_PARENT_CALL(CBaseMonster);
}


GENERATE_KILLED_IMPLEMENTATION(CHAssassin){

	//TODO - make this an event during the death animation, or some death animations?
	attemptDropWeapon();
	
	GENERATE_KILLED_PARENT_CALL(CBaseMonster);
}


//=========================================================
// GibMonster - make gun fly through the air.
//=========================================================
GENERATE_GIBMONSTER_IMPLEMENTATION(CHAssassin)
{
	//if ( GetBodygroup( GUN_GROUP ) != GUN_NONE )

	//do we need to check if we even have a crossbow, or always imply it?
	attemptDropWeapon();

	//CBaseMonster::GibMonster(spawnHeadBlock, gibsSpawnDecal);
	GENERATE_GIBMONSTER_PARENT_CALL(CBaseMonster);
}



BOOL CHAssassin ::attemptDropWeapon(void){
	Vector	vecGunPos;
	Vector	vecGunAngles;

	if(!droppedWeapon && (g_iSkillLevel==SKILL_EASY || g_iSkillLevel == SKILL_MEDIUM)  ){
		GetAttachment( 0, vecGunPos, vecGunAngles );

		CBaseEntity *pGun;
		pGun = DropItem( "weapon_crossbow", vecGunPos, vecGunAngles );

		if ( pGun )
		{
			SetBodygroup(BODYGROUP_GUN, GUN_NONE);  //no longer holding it.
			
			pGun->pev->velocity = Vector (RANDOM_FLOAT(-100,100), RANDOM_FLOAT(-100,100), RANDOM_FLOAT(200,300));
			pGun->pev->avelocity = Vector ( 0, RANDOM_FLOAT( 200, 400 ), 0 );
			droppedWeapon = TRUE;
			return TRUE;  //dropped the weapon.
		}

	}else{

	}


	return FALSE;
}//END OF attemptDropWeapon(...)




//=========================================================
// RunTask 
//=========================================================
void CHAssassin::RunTask ( Task_t *pTask )
{

	
	//easyForcePrintLine("%s:%d RunTask sched:%s task:%d", getClassname(), monsterID, getScheduleName(), pTask->iTask );


	//easyForcePrintLine("MY stuff %s %d", this->getScheduleName(), pTask->iTask);
	switch ( pTask->iTask )
	{

	case TASK_RANGE_ATTACK1:

		
		if(EASY_CVAR_GET_DEBUGONLY(hassassinCrossbowDebug) != 1){
			if(m_fSequenceFinished == TRUE){
				//done.
				SetActivity(ACT_COMBAT_IDLE);
				//m_IdealActivity = ACT_RESET;
				TaskComplete();
			}
		}else{

			if(m_fSequenceFinished == TRUE){
				//TaskComplete();
				//pev->frame = 0; //??
			}
		}
		
	break;
	case TASK_RANGE_ATTACK2:

		CBaseMonster::RunTask(pTask);


		if(pev->frame >= 200){
			// go ahead.
			TaskComplete();
		}

		
		//if(m_fSequenceFinished == TRUE){
		if(TaskIsComplete()){
			// complete from the 'RunTask' call above?  about to end?
			// TODO: would make more sense for this step to be its own task after this one
			// in a custom 'hassassinThrowGrenade' schedule.
			// done.
			SetActivity(ACT_COMBAT_IDLE);
			//m_IdealActivity = ACT_RESET;
			// alreay handled above then.
			//TaskComplete();
		}
	break;


		//MODDD - new
	case TASK_RELOAD:
		{
		if(reloadApplyTime != -1 &&  gpGlobals->time >= reloadApplyTime){
			reloadApplyTime = -1;
			m_cAmmoLoaded = 1;  //loaded.
			//notice: not using the sound sentence save. The player precaches this.
			UTIL_PlaySound(ENT(this->pev), CHAN_ITEM, "weapons/xbow_reload1.wav", RANDOM_FLOAT(0.85, 0.94), ATTN_NORM, 0, 93 + RANDOM_LONG(0,0xF), FALSE);
			//sound!!
		}
		//are we done?
		/*
		if(m_fSequenceFinished){
			TaskComplete();
		}
		*/

		CBaseMonster::RunTask(pTask);

		break;
		}
	case TASK_ASSASSIN_FALL_TO_GROUND:
		MakeIdealYaw( m_vecEnemyLKP );
		ChangeYaw( pev->yaw_speed );

		if (m_fSequenceFinished)
		{
			if (pev->velocity.z > 0)
			{
				pev->sequence = LookupSequence( "fly_up" );
			}
			//MODDD - extra requirement. Has ammo?
			else if (HasConditions ( bits_COND_SEE_ENEMY ) && this->m_cAmmoLoaded > 0)
			{
				pev->sequence = LookupSequence( "fly_attack" );
				pev->frame = 0;
			}
			else
			{
				pev->sequence = LookupSequence( "fly_down" );
				pev->frame = 0;
			}
			
			ResetSequenceInfo( );
			SetYawSpeed();
		}
		if (pev->flags & FL_ONGROUND)
		{
			// ALERT( at_console, "on ground\n");
			TaskComplete( );
		}
		break;
	default: 
		CBaseMonster::RunTask ( pTask );
		break;
	}
}

//=========================================================
// GetSchedule - Decides which type of schedule best suits
// the monster's current state and conditions. Then calls
// monster's member function to get a pointer to a schedule
// of the proper type.
//=========================================================
Schedule_t *CHAssassin::GetSchedule ( void )
{

	
	//no need for extra bait script, defaults should carry over.


	switch	( m_MonsterState )
	{
	case MONSTERSTATE_IDLE:
	case MONSTERSTATE_ALERT:
		{

			//MODDD
			// That's cool and all but uhhh.   If you're damaged 'investigating' the sound should probably be
			// the least of your worries.
			// Added check that LIGHT/HEAVY_DAMAGE isn't present.
			if ( !HasConditionsEither(bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE) && HasConditions ( bits_COND_HEAR_SOUND ))
			{
				CSound *pSound;
				pSound = PBestSound();

				ASSERT( pSound != NULL );
				if ( pSound && (pSound->m_iType & bits_SOUND_DANGER) )
				{
					return GetScheduleOfType( SCHED_TAKE_COVER_FROM_BEST_SOUND );
				}
				if ( pSound && (pSound->m_iType & bits_SOUND_COMBAT) )
				{
					return GetScheduleOfType( SCHED_INVESTIGATE_SOUND );
				}
			}
		}
		break;

	case MONSTERSTATE_COMBAT:
		{
// dead enemy


			if ( HasConditions( bits_COND_ENEMY_DEAD ) )
			{
				// call base class, all code to handle dead enemies is centralized there.
				return CBaseMonster::GetSchedule();
			}

			// flying?
			//...unfortunately this is not very reactive. Maybe a touch method that interrupts the schedule on flying and touching the ground would be better.
			//FIXED, see above. There is a gound check done with the var "groundTouchCheckDuration". If anything is touched, ground checks are done every monsterThink frame
			//just in case a wall or the ceiling was touched intead, which should not trigger a landing (a linetrace to below the flying hassassin would find only the air if so).


			if ( pev->movetype == MOVETYPE_TOSS)
			{
				if (pev->flags & FL_ONGROUND)
				{
					// ALERT( at_console, "landed\n");
					// just landed
					pev->movetype = MOVETYPE_STEP;
					return GetScheduleOfType ( SCHED_ASSASSIN_JUMP_LAND );
				}
				else
				{
					// ALERT( at_console, "jump\n");
					// jump or jump/shoot
					if ( m_MonsterState == MONSTERSTATE_COMBAT ){
						return GetScheduleOfType ( SCHED_ASSASSIN_JUMP );
					}else{
						//MODDD NOTE - DERP. This is impossible to reach as we clearly must be in MONSTERSTATE_COMBAT to even be in this case statement at all.
						//This is probably the point.
						return GetScheduleOfType ( SCHED_ASSASSIN_JUMP_ATTACK );
					}
				}
			}






			if ( HasConditions ( bits_COND_HEAR_SOUND ))
			{
				CSound *pSound;
				pSound = PBestSound();

				ASSERT( pSound != NULL );
				if ( pSound && (pSound->m_iType & bits_SOUND_DANGER) )
				{
					return GetScheduleOfType( SCHED_TAKE_COVER_FROM_BEST_SOUND );
				}
			}

			if ( HasConditions ( bits_COND_LIGHT_DAMAGE ) )
			{
				m_iFrustration++;
			}
			if ( HasConditions ( bits_COND_HEAVY_DAMAGE ) )
			{
				m_iFrustration++;
			}

			


			//MODDD - new melee check.  Get close too fast before the jump can start or
			//        too soon since a recent jump away and this happens.
			if ( HasConditions ( bits_COND_CAN_MELEE_ATTACK1 ) )
			{
				// ALERT( at_console, "melee attack 1\n");
				return GetScheduleOfType ( SCHED_MELEE_ATTACK1 );
			}

			
		// jump player!
			//MODD - used to be melee attack #1. now is #2.
			if ( HasConditions ( bits_COND_CAN_MELEE_ATTACK2 ) )
			{
				// ALERT( at_console, "melee attack 2\n");
				return GetScheduleOfType ( SCHED_MELEE_ATTACK2 );
			}




		// throw grenade
			if ( HasConditions ( bits_COND_CAN_RANGE_ATTACK2 ) )
			{
				// ALERT( at_console, "range attack 2\n");
				return GetScheduleOfType ( SCHED_RANGE_ATTACK2 );
			}



			if(m_cAmmoLoaded <= 0){
				//can we reload?
				if ( HasConditions( bits_COND_ENEMY_OCCLUDED )) {
					//if occluded or we're in cover, just go ahead and reload.
					return GetScheduleOfType(SCHED_ASSASSIN_RELOAD);
				}else{
					//take cover or try to?
					return GetScheduleOfType(SCHED_ASSASSIN_TAKE_COVER_RELOAD);
				}
			}



		// spotted
			if ( HasConditions ( bits_COND_SEE_ENEMY ) && HasConditions ( bits_COND_ENEMY_FACING_ME ) )
			{
				// ALERT( at_console, "exposed\n");
				m_iFrustration++;
				return GetScheduleOfType ( SCHED_ASSASSIN_EXPOSED );
			}


			








		// can attack
			if ( HasConditions ( bits_COND_CAN_RANGE_ATTACK1 ) )
			{
				// ALERT( at_console, "range attack 1\n");
				m_iFrustration = 0;
				return GetScheduleOfType ( SCHED_RANGE_ATTACK1 );
			}

			if ( HasConditions ( bits_COND_SEE_ENEMY ) )
			{
				// ALERT( at_console, "face\n");
				return GetScheduleOfType ( SCHED_COMBAT_FACE );
			}

		// new enemy
			if ( HasConditions ( bits_COND_NEW_ENEMY ) )
			{
				// ALERT( at_console, "take cover\n");
				return GetScheduleOfType ( SCHED_TAKE_COVER_FROM_ENEMY );
			}

			// ALERT( at_console, "stand\n");


			return GetScheduleOfType ( SCHED_ALERT_STAND );
		}
		break;
	}

	return CBaseMonster::GetSchedule();
}

//=========================================================
//=========================================================
Schedule_t* CHAssassin::GetScheduleOfType ( int Type ) 
{
	// ALERT( at_console, "%d\n", m_iFrustration );
	switch	( Type )
	{

		//AssassinTakeCoverFromEnemy2
		//!!!???

	case SCHED_ASSASSIN_RANGE_ATTACK1_CANRELOAD:
		//can attack? go ahead.
		if(m_cAmmoLoaded > 0){
			//MODDD - why was this SCHED_RANGE_ATTACK2 ??? that is for grenades.
			return CBaseMonster::GetScheduleOfType( SCHED_RANGE_ATTACK1 );
		}else{
			//reload!
			return slAssassinReload;
		}

	break;
	case SCHED_ASSASSIN_RANGE_ATTACK2_CANRELOAD:


		//MODDD - extra check, for m_fThrowGrenade
		if( m_flNextGrenadeCheck < gpGlobals->time && !HasConditions( bits_COND_ENEMY_OCCLUDED ) && m_fThrowGrenade){
			//can throw a grenade probably? just try.
			return CBaseMonster::GetScheduleOfType( SCHED_RANGE_ATTACK2 );
		}else{
			//too soon since thrown a grenade / enemy is occluded? Just reload.
			if(m_cAmmoLoaded > 0){
				//MODDD - why was this SCHED_RANGE_ATTACK2 ??? that is for grenades.
				return CBaseMonster::GetScheduleOfType( SCHED_RANGE_ATTACK1 );
			}else{
				//reload!
				return slAssassinReload;
			}
		}

	break;

	case SCHED_RANGE_ATTACK1:

		if(m_cAmmoLoaded > 0){
			//acceptable to attack.
			return CBaseMonster::GetScheduleOfType( Type );
		}else{
			//you can't attack, so flee!
			return slAssassinTakeCoverFromEnemy2;
		}
	break;
	case SCHED_ASSASSIN_TAKE_COVER_RELOAD:
		return slAssassinTakeCoverReload;
	break;
	case SCHED_ASSASSIN_RELOAD:
		return slAssassinReload;
	break;


	case SCHED_TAKE_COVER_FROM_ENEMY:
		if (pev->health > 30)
			return slAssassinTakeCoverFromEnemy;
		else
			return slAssassinTakeCoverFromEnemy2;
	case SCHED_TAKE_COVER_FROM_BEST_SOUND:
		return slAssassinTakeCoverFromBestSound;
	case SCHED_ASSASSIN_EXPOSED:
		return slAssassinExposed;
	case SCHED_FAIL:
		if (m_MonsterState == MONSTERSTATE_COMBAT)
			return slAssassinFail;
		break;
	case SCHED_ALERT_STAND:
		if (m_MonsterState == MONSTERSTATE_COMBAT)
			return slAssassinHide;
		break;
	case SCHED_CHASE_ENEMY:
		return slAssassinHunt;



	case SCHED_MELEE_ATTACK1:
		return slAssassinMeleeAttack;
	break;

	//MODDD - used to be for MELEE_ATTACK1.
	case SCHED_MELEE_ATTACK2:
		if (pev->flags & FL_ONGROUND)
		{
			if (m_flNextJump > gpGlobals->time)
			{
				// can't jump yet, go ahead and fail
				//MODDD NOTE - ...wait how is this possible? That condition shouldn't have been met to allow SCHED_MELEE_ATTACK1 to even
				//             be requested?
				//             It is possible from another method saying to pick this schedule if it fails. That skips the condition check.
				//             ...Instead just go into the hassassin's melee attack I guess.
				
				//return slAssassinFail;
				if(HasConditions(bits_COND_CAN_MELEE_ATTACK1)){
					return slAssassinMeleeAttack;
				}else{
					//then I can't do anything.
					return slAssassinFail;
				}

			}
			else
			{
				return slAssassinJump;
			}
		}
		else
		{
			return slAssassinJumpAttack;
		}
	case SCHED_ASSASSIN_JUMP:
	case SCHED_ASSASSIN_JUMP_ATTACK:
		return slAssassinJumpAttack;
	case SCHED_ASSASSIN_JUMP_LAND:
		return slAssassinJumpLand;


	
	case SCHED_VICTORY_DANCE:{
		return slAssassinVictoryDance;
	break;}


	}//END OF switch

	return CBaseMonster::GetScheduleOfType( Type );
}








BOOL CHAssassin::canResetBlend0(void){
	return FALSE;
}

BOOL CHAssassin::onResetBlend0(void){
	//add something?
	
	Vector vecDirToEnemy;
	Vector angDir;


	if (HasConditions( bits_COND_SEE_ENEMY))
	{
		vecDirToEnemy = ( ( m_vecEnemyLKP ) - pev->origin );

		//okay?
		vecDirToEnemy = vecDirToEnemy.Normalize();

		angDir = UTIL_VecToAngles( vecDirToEnemy );
		//vecDirToEnemy = vecDirToEnemy.Normalize();
	}
	else
	{
		angDir = pev->angles;
		UTIL_MakeAimVectors( angDir );
		vecDirToEnemy = gpGlobals->v_forward;
	}
	// make angles +-180
	if (angDir.x > 180)
	{
		angDir.x = angDir.x - 360;
	}
	//easyForcePrintLine("YOU GOON %d :::%.2f", HasConditions( bits_COND_SEE_ENEMY), angDir.x );
	SetBlending( 0, angDir.x );


	return TRUE;
}




Vector CHAssassin::GetGunPosition(void){

	//NOTE - unknown if attachments are a good idea here. Use default behavior instead.
	//return CBaseMonster::GetGunPosition();
	

	Vector vecGunPos;
	Vector vecGunAngles;
	GetAttachment( 0, vecGunPos, vecGunAngles );
	::UTIL_printLineVector("yehhhag", vecGunPos-pev->origin);

	return vecGunPos;
}//END OF GetGunPosition


Vector CHAssassin::GetGunPositionAI(void){
	//Use the fixed origin given rotation instead for a little more consistency than some arm point that may float around.
	return CBaseMonster::GetGunPositionAI();

	/*
	Vector forward, angle;
	angle = pev->angles;
	
	angle.x = 0; //pitch is not a factor here.
	UTIL_MakeVectorsPrivate( angle, forward, NULL, NULL );

	return pev->origin + Vector( 0, 0, 43 ) + forward * 41;
	*/
}//END OF GetGunPositionAI





BOOL CHAssassin::usesAdvancedAnimSystem(void){
	return TRUE;
}


//NOTICE - ACT_COMBAT_IDLE may be fine as it is too.

int CHAssassin::LookupActivityHard(int activity){
	

	pev->framerate = 1;
	resetEventQueue();

	m_iForceLoops = -1;

	m_flFramerateSuggestion = 1;
	switch(activity){
		case ACT_DIESIMPLE:
		case ACT_DIEBACKWARD:
		case ACT_DIEFORWARD:
		case ACT_DIEVIOLENT:
		case ACT_DIE_HEADSHOT:
		case ACT_DIE_CHESTSHOT:
		case ACT_DIE_GUTSHOT:
		case ACT_DIE_BACKSHOT:{
			//If any death activitiy is called while running, force this to play.
			if(timeOfDeath_activity == ACT_RUN){
			//if(this->IsMoving() == TRUE && this->m_movementActivity == ACT_RUN){
				return HASSASSIN_DEATH_DURING_RUN;
			}else{
				return CBaseAnimating::LookupActivity(activity);
			}
		break;}

		case ACT_COMBAT_IDLE:{
			//Our model has no explicit ACT_COMBAT_IDLE sequences.
			//How about the crouched over idle?
			return HASSASSIN_IDLE2;
		break;}
		case ACT_MELEE_ATTACK1:{
			//NOTICE - sequence "kick" is already mapped to ACT_MELEE_ATTACK1, but sequence "kickshort" is mapped to ACT_MELEE_ATTACK2.
			//         It's going to make more sense to keep "jump" as ACT_MELEE_ATTACK2 and just pick between "kick" and "kickshort" randomly.
			float randomChoice = RANDOM_LONG(0, 1);
			
			if(randomChoice == 0){

				animEventQueuePush(5.0f / 35.0f, 5);
				animEventQueuePush(21.0f / 35.0f, 5);

				return HASSASSIN_KICK;
			}else{  //1?
				
				animEventQueuePush(10.0f / 30.0f, 6);

				return HASSASSIN_KICKSHORT;
			}

		break;}
		case ACT_RANGE_ATTACK1:{
			
			m_flFramerateSuggestion = 1;
			//m_flFramerateSuggestion = -1;


			if(EASY_CVAR_GET_DEBUGONLY(hassassinCrossbowDebug) != 1){
				m_iForceLoops = 0; //no loopping.
			}

			return HASSASSIN_SHOOT;
		break;}
		case ACT_IDLE_ANGRY:{
			//should this do anything special?  or copy ACT_RANGE_ATTACK1? probably not that.
			//maybe freeze a frame from "shoot" for a bit? probably unnecessary.
			return CBaseAnimating::LookupActivity(activity);
		break;}
		case ACT_RELOAD:{
			//MODDD - placeholder. Update this when we have a real crossbow reload animation!
			return LookupSequence(HASSASSIN_CROSSBOW_RELOAD_ANIM);
		break;}

		case ACT_WALK:{

			//events.
			this->animEventQueuePush(2.0f / 35.0f, 1);
			this->animEventQueuePush(23.0 / 35.0f, 2);
			this->animEventQueuePush(44.0f / 35.0f, 1);
			this->animEventQueuePush(65.0 / 35.0f, 2);
			return CBaseAnimating::LookupActivity(activity);
		break;}
		case ACT_RUN:{
			
			this->animEventQueuePush(8.0f / 40.0f, 3);
			this->animEventQueuePush(17.0f / 40.0f, 4);

			return CBaseAnimating::LookupActivity(activity);
		break;}
		

	}

	//not handled by above?  try the real deal.
	return CBaseAnimating::LookupActivity(activity);
}




int CHAssassin::tryActivitySubstitute(int activity){

	
	int iRandChoice = 0;
	int iRandWeightChoice = 0;
	
	char* animChoiceString = NULL;
	int* weightsAbs = NULL;
			
	//pev->framerate = 1;
	int maxRandWeight = 30;

	


	switch(activity){
		case ACT_COMBAT_IDLE:{
			return HASSASSIN_IDLE2;
		break;}
		case ACT_MELEE_ATTACK1:{
			// just to say we have something.  picks between KICK and KICKSHORT.
			return HASSASSIN_KICK;
		break;}
		case ACT_RANGE_ATTACK1:{
			return HASSASSIN_SHOOT;
		break;}
		case ACT_IDLE_ANGRY:{
			return CBaseAnimating::LookupActivity(activity);
		break;}
		case ACT_RELOAD:{
			//MODDD - placeholder. Update this when we have a real crossbow reload animation!
			return LookupSequence(HASSASSIN_CROSSBOW_RELOAD_ANIM);
		break;}

	}//END OF switch


	//not handled by above?  No animations.
	//MODDD TODO - would it be safer for all monsters to call "PARENTCLASS::LookupActivity(activity);" instead of giving up this easily? Verify.
	return CBaseAnimating::LookupActivity(activity);
}







//Handles custom events sent from "LookupActivityHard", which sends events as timed delays along with picking an animation in script.
//So this handles script-provided events, not model ones.
void CHAssassin::HandleEventQueueEvent(int arg_eventID){

	switch(arg_eventID){
	case 0:{


	break;}
	case 1:{
		//right foot, walk
		switch( RANDOM_LONG( 0, 1 ) ){
		case 0:	UTIL_PlaySound( ENT(pev), CHAN_BODY, "player/pl_step1.wav", 0.28, ATTN_NORM, 0, 100, FALSE);	break;
		case 1:	UTIL_PlaySound( ENT(pev), CHAN_BODY, "player/pl_step3.wav", 0.28, ATTN_NORM, 0, 100, FALSE);	break;
		}
	break;}
	case 2:{
		//left foot, walk
		switch( RANDOM_LONG( 0, 1 ) )
		{
		case 0:	UTIL_PlaySound( ENT(pev), CHAN_BODY, "player/pl_step2.wav", 0.28, ATTN_NORM, 0, 100, FALSE);	break;
		case 1:	UTIL_PlaySound( ENT(pev), CHAN_BODY, "player/pl_step4.wav", 0.28, ATTN_NORM, 0, 100, FALSE);	break;
		}
	break;}
	case 3:{
		//right foot, run
		switch( RANDOM_LONG( 0, 1 ) ){
		case 0:	UTIL_PlaySound( ENT(pev), CHAN_BODY, "player/pl_step1.wav", 0.5, ATTN_NORM, 0, 100, FALSE);	break;
		case 1:	UTIL_PlaySound( ENT(pev), CHAN_BODY, "player/pl_step3.wav", 0.5, ATTN_NORM, 0, 100, FALSE);	break;
		}
	break;}
	case 4:{
		//left foot, run
		switch( RANDOM_LONG( 0, 1 ) )
		{
		case 0:	UTIL_PlaySound( ENT(pev), CHAN_BODY, "player/pl_step2.wav", 0.5, ATTN_NORM, 0, 100, FALSE);	break;
		case 1:	UTIL_PlaySound( ENT(pev), CHAN_BODY, "player/pl_step4.wav", 0.5, ATTN_NORM, 0, 100, FALSE);	break;
		}
	break;}

	//shorter kick
	case 5:{
		CBaseEntity *pHurt = HumanKick();

		if ( pHurt )
		{
			// SOUND HERE!
			UTIL_MakeVectors( pev->angles );
			if(!pHurt->blocksImpact()){
				pHurt->pev->punchangle.x = 15;
				pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_forward * 80 + gpGlobals->v_up * 60;
			}

			//is taking the hgrunt's kick damage okay?
			pHurt->TakeDamage( pev, pev, gSkillData.hgruntDmgKick, DMG_CLUB );

			//derp.
			//UTIL_PlaySound( ENT(pev), CHAN_WEAPON, "hgrunt/gr_pain3.wav", 1, ATTN_NORM );
			playStandardMeleeAttackHitSound();
		}else{
			playStandardMeleeAttackMissSound();
		}
	break;}
	//longer kick
	case 6:{
		CBaseEntity *pHurt = HumanKick();
		if ( pHurt )
		{
			// SOUND HERE!
			UTIL_MakeVectors( pev->angles );
			if(!pHurt->blocksImpact()){
				pHurt->pev->punchangle.x = 15;
				pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_forward * 140 + gpGlobals->v_up * 65;
			}

			//is taking the hgrunt's kick damage okay?
			pHurt->TakeDamage( pev, pev, gSkillData.hgruntDmgKick, DMG_CLUB );

			playStandardMeleeAttackHitSound();
		}else{
			playStandardMeleeAttackMissSound();
		}
	break;}

	}//END OF switch

}//END OF HandleEventQueueEvent


BOOL CHAssassin::predictRangeAttackEnd(void) {
	return TRUE;
}

