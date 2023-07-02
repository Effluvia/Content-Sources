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
// Agrunt - Dominant, warlike alien grunt monster
//=========================================================

// MODDD - TODO!  Try adding slAGruntHiddenRangeAttack back in, maybe?
// Commented out as-is and fixed up a little since.  Unsure if it would ever get called as not sure how 
// a range attack would be called for without seeing the enemy.  Make sure it gets picked ever or determine
// how to make CheckRangedAttack show TRUE when the player isn't visible.
// (also need to override FCanCheckAttacks to not stop at the enemy being hidden, like the hgrunt/hassault do
// in order to allow firing grenades).

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "basemonster.h"
#include "schedule.h"
#include "squadmonster.h"
#include "weapons.h"
#include "soundent.h"
#include "hornet.h"
//MODDD - new includes
#include "activity.h"
#include "util_model.h"
#include "defaultai.h"
#include "util_debugdraw.h"

EASY_CVAR_EXTERN_DEBUGONLY(noFlinchOnHard);
EASY_CVAR_EXTERN_DEBUGONLY(thatWasntPunch);
EASY_CVAR_EXTERN_DEBUGONLY(agrunt_muzzleflash);




extern unsigned short g_sCustomBallsPowerup;


int iAgruntMuzzleFlash;



#define AGRUNT_MELEE_DIST	100

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define AGRUNT_AE_HORNET1	( 1 )
#define AGRUNT_AE_HORNET2	( 2 )
#define AGRUNT_AE_HORNET3	( 3 )
#define AGRUNT_AE_HORNET4	( 4 )
#define AGRUNT_AE_HORNET5	( 5 )
// some events are set up in the QC file that aren't recognized by the code yet.
#define AGRUNT_AE_PUNCH		( 6 )
#define AGRUNT_AE_BITE		( 7 )
#define AGRUNT_AE_LEFT_FOOT	 ( 10 )
#define AGRUNT_AE_RIGHT_FOOT ( 11 )
#define AGRUNT_AE_LEFT_PUNCH ( 12 )
#define AGRUNT_AE_RIGHT_PUNCH ( 13 )


//MODDD - macro
#define HITGROUP_AGRUNT_ARMOR 10




class CAGrunt : public CSquadMonster
{
public:

	static const char* pAttackHitSounds[];
	static const char* pAttackMissSounds[];
	static const char* pAttackSounds[];
	static const char* pDieSounds[];
	static const char* pPainSounds[];
	static const char* pIdleSounds[];
	static const char* pAlertSounds[];

	BOOL	m_fCanHornetAttack;
	float m_flNextHornetAttackCheck;
	float m_flNextPainTime;
	// three hacky fields for speech stuff. These don't really need to be saved.
	float m_flNextSpeakTime;
	float m_flNextWordTime;
	int	m_iLastWord;

	BOOL m_fIsPoweredUp;
	float nextPoweredUpParticleTime;
	float poweredUpTimeEnd;
	EHANDLE poweredUpCauseEnt;
	float poweredUpCauseEntChangeCooldown;
	EHANDLE powerupCauseEntDirectedEnemy;

	float forceNewEnemyCooldownTightTime;
	float forceNewEnemyCooldownTime;
	EHANDLE directedEnemyIssuer;
	float forgetPowerupCauseEntDirectedEnemyTime;



	CAGrunt();

	void MonsterThink(void);

	void Spawn( void );
	void Precache( void );

	BOOL needsMovementBoundFix(void);

	void SetYawSpeed ( void );
	int  Classify ( void );
	BOOL isOrganic(void);
	int  ISoundMask ( void );
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	
	GENERATE_KILLED_PROTOTYPE
	
	BOOL canResetBlend0(void);
	BOOL onResetBlend0(void);

	//MODDD
	void SetObjectCollisionBox( void )
	{
		if(pev->deadflag != DEAD_NO){
			pev->absmin = pev->origin + Vector(-98, -98, 0);
			pev->absmax = pev->origin + Vector(98, 98, 80);
		}else{	
			pev->absmin = pev->origin + Vector( -32, -32, 0 );
			pev->absmax = pev->origin + Vector( 32, 32, 85 );
		}
	}

	Schedule_t* GetSchedule ( void );
	Schedule_t* GetScheduleOfType ( int Type );
	BOOL FCanCheckAttacks ( void );
	BOOL CheckMeleeAttack1 ( float flDot, float flDist );
	BOOL CheckRangeAttack1 ( float flDot, float flDist );
	void StartTask ( Task_t *pTask );
	void RunTask( Task_t* pTask );  //MODDD - new
	void AlertSound( void );
	void DeathSound ( void );
	void PainSound ( void );
	void AttackSound ( void );
	void PrescheduleThink ( void );
	
	//MODDD
	GENERATE_TRACEATTACK_PROTOTYPE
	GENERATE_TAKEDAMAGE_PROTOTYPE
	

	void forceNewEnemy(CBaseEntity* argIssuing, CBaseEntity* argNewEnemy, BOOL argPassive);
	void forgetForcedEnemy(CBaseMonster* argIssuing, BOOL argPassive);
	void ReportAIState(void);

	int IRelationship( CBaseEntity *pTarget );
	void StopTalking ( void );
	BOOL ShouldSpeak( void );
	CUSTOM_SCHEDULES;

	virtual int	Save( CSave &save );
	virtual int	Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];
	
	//MODDD - some new methods.
	void CAGrunt::setPoweredUpOff(void);
	void CAGrunt::setPoweredUpOn(CBaseMonster* argPoweredUpCauseEnt, float argHowLong );

	BOOL getMonsterBlockIdleAutoUpdate(void);
	BOOL forceIdleFrameReset(void);
	BOOL usesAdvancedAnimSystem(void);
	int LookupActivityHard(int activity);
	int tryActivitySubstitute(int activity);
	void HandleEventQueueEvent(int arg_eventID);
	//void HandleAnimEvent(MonsterEvent_t *pEvent );

	void setChaseSpeed(void);
	
	Vector GetGunPosition( void );
	Vector GetGunPositionAI(void);
	void OnTakeDamageSetConditions(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType, int bitsDamageTypeMod);
	int getHullIndexForNodes(void);

	BOOL predictRangeAttackEnd(void);


};

#if REMOVE_ORIGINAL_NAMES != 1
	LINK_ENTITY_TO_CLASS( monster_alien_grunt, CAGrunt );
#endif

#if EXTRA_NAMES > 0
	LINK_ENTITY_TO_CLASS( agrunt, CAGrunt );

	#if EXTRA_NAMES == 2
		LINK_ENTITY_TO_CLASS( alien_grunt, CAGrunt );
	#endif
#endif


TYPEDESCRIPTION	CAGrunt::m_SaveData[] = 
{
	DEFINE_FIELD( CAGrunt, m_fCanHornetAttack, FIELD_BOOLEAN ),
	DEFINE_FIELD( CAGrunt, m_flNextHornetAttackCheck, FIELD_TIME ),
	DEFINE_FIELD( CAGrunt, m_flNextPainTime, FIELD_TIME ),
	DEFINE_FIELD( CAGrunt, m_flNextSpeakTime, FIELD_TIME ),
	DEFINE_FIELD( CAGrunt, m_flNextWordTime, FIELD_TIME ),
	DEFINE_FIELD( CAGrunt, m_iLastWord, FIELD_INTEGER ),
	
	//MODDD - new
	DEFINE_FIELD( CAGrunt, m_fIsPoweredUp, FIELD_BOOLEAN ),
	//DEFINE_FIELD( CAGrunt, nextPoweredUpParticleTime, FIELD_TIME ),   nah.
	DEFINE_FIELD( CAGrunt, poweredUpTimeEnd, FIELD_TIME ),
	DEFINE_FIELD( CAGrunt, poweredUpCauseEnt, FIELD_EHANDLE ),
	DEFINE_FIELD( CAGrunt, poweredUpCauseEntChangeCooldown, FIELD_TIME),
	DEFINE_FIELD( CAGrunt, powerupCauseEntDirectedEnemy, FIELD_EHANDLE ),
	
};

IMPLEMENT_SAVERESTORE( CAGrunt, CSquadMonster );

const char *CAGrunt::pAttackHitSounds[] = 
{
	"zombie/claw_strike1.wav",
	"zombie/claw_strike2.wav",
	"zombie/claw_strike3.wav",
};

const char *CAGrunt::pAttackMissSounds[] = 
{
	"zombie/claw_miss1.wav",
	"zombie/claw_miss2.wav",
};

const char *CAGrunt::pAttackSounds[] =
{
	"agrunt/ag_attack1.wav",
	"agrunt/ag_attack2.wav",
	"agrunt/ag_attack3.wav",
};

const char *CAGrunt::pDieSounds[] =
{
	"agrunt/ag_die1.wav",
	"agrunt/ag_die4.wav",
	"agrunt/ag_die5.wav",
};

const char *CAGrunt::pPainSounds[] =
{
	"agrunt/ag_pain1.wav",
	"agrunt/ag_pain2.wav",
	"agrunt/ag_pain3.wav",
	"agrunt/ag_pain4.wav",
	"agrunt/ag_pain5.wav",
};

const char *CAGrunt::pIdleSounds[] =
{
	"agrunt/ag_idle1.wav",
	"agrunt/ag_idle2.wav",
	"agrunt/ag_idle3.wav",
	"agrunt/ag_idle4.wav",
};

const char *CAGrunt::pAlertSounds[] =
{
	"agrunt/ag_alert1.wav",
	"agrunt/ag_alert3.wav",
	"agrunt/ag_alert4.wav",
	"agrunt/ag_alert5.wav",
};









//=========================================================
// monster-specific schedule types
//=========================================================
enum
{
	SCHED_AGRUNT_SUPPRESS = LAST_COMMON_SCHEDULE + 1,
	SCHED_AGRUNT_THREAT_DISPLAY,
};

//=========================================================
// monster-specific tasks
//=========================================================
enum
{
	TASK_AGRUNT_SETUP_HIDE_ATTACK = LAST_COMMON_TASK + 1,
	TASK_AGRUNT_GET_PATH_TO_ENEMY_CORPSE,
};





//=========================================================
// AI Schedules Specific to this monster
//=========================================================


//=========================================================
// Fail Schedule
//=========================================================
Task_t	tlAGruntFail[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_WAIT,				(float)2		},
	{ TASK_WAIT_PVS,			(float)0		},
};

Schedule_t	slAGruntFail[] =
{
	{
		tlAGruntFail,
		ARRAYSIZE ( tlAGruntFail ),
		bits_COND_CAN_RANGE_ATTACK1	|
		bits_COND_CAN_MELEE_ATTACK1,
		0,
		"AGrunt Fail"
	},
};

//=========================================================
// Combat Fail Schedule
//=========================================================
Task_t	tlAGruntCombatFail[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_WAIT_FACE_ENEMY,		(float)2		},
	{ TASK_WAIT_PVS,			(float)0		},
};

Schedule_t	slAGruntCombatFail[] =
{
	{
		tlAGruntCombatFail,
		ARRAYSIZE ( tlAGruntCombatFail ),
		bits_COND_CAN_RANGE_ATTACK1	|
		bits_COND_CAN_MELEE_ATTACK1,
		0,
		"AGrunt Combat Fail"
	},
};

//=========================================================
// Standoff schedule. Used in combat when a monster is 
// hiding in cover or the enemy has moved out of sight. 
// Should we look around in this schedule?
//=========================================================
Task_t	tlAGruntStandoff[] =
{
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_SET_ACTIVITY,			(float)ACT_IDLE				},
	{ TASK_WAIT_FACE_ENEMY,			(float)2					},
};

Schedule_t slAGruntStandoff[] = 
{
	{
		tlAGruntStandoff,
		ARRAYSIZE ( tlAGruntStandoff ),
		bits_COND_CAN_RANGE_ATTACK1		|
		bits_COND_CAN_MELEE_ATTACK1		|
		bits_COND_SEE_ENEMY				|
		bits_COND_NEW_ENEMY				|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"Agrunt Standoff"
	}
};

//=========================================================
// Suppress
//=========================================================
Task_t	tlAGruntSuppressHornet[] =
{
	{ TASK_STOP_MOVING,		(float)0		},
	{ TASK_RANGE_ATTACK1,	(float)0		},
};

Schedule_t slAGruntSuppress[] =
{
	{
		tlAGruntSuppressHornet,
		ARRAYSIZE ( tlAGruntSuppressHornet ),
		0,
		0,
		"AGrunt Suppress Hornet",
	},
};

//=========================================================
// primary range attacks
//=========================================================
Task_t	tlAGruntRangeAttack1[] =
{
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
};

Schedule_t	slAGruntRangeAttack1[] =
{
	{ 
		tlAGruntRangeAttack1,
		ARRAYSIZE ( tlAGruntRangeAttack1 ), 
		bits_COND_NEW_ENEMY			|
		bits_COND_ENEMY_DEAD		|
		bits_COND_HEAVY_DAMAGE,
		
		0,
		"AGrunt Range Attack1"
	},
};


Task_t	tlAGruntHiddenRangeAttack1[] =
{
	{ TASK_SET_FAIL_SCHEDULE,			(float)SCHED_STANDOFF },
	{ TASK_AGRUNT_SETUP_HIDE_ATTACK,	0				},
	{ TASK_STOP_MOVING,					0				},
	{ TASK_FACE_IDEAL,					0				},
	{ TASK_RANGE_ATTACK1_NOTURN,		(float)0		},
};

Schedule_t	slAGruntHiddenRangeAttack[] =
{
	{ 
		tlAGruntHiddenRangeAttack1,
		ARRAYSIZE ( tlAGruntHiddenRangeAttack1 ), 
		bits_COND_NEW_ENEMY			|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"AGrunt Hidden Range Attack1"
	},
};

//=========================================================
// Take cover from enemy! Tries lateral cover before node 
// cover! 
//=========================================================
Task_t	tlAGruntTakeCoverFromEnemy[] =
{
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_WAIT,					(float)0.2					},
	{ TASK_FIND_COVER_FROM_ENEMY,	(float)0					},
	{ TASK_RUN_PATH,				(float)0					},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0					},
	{ TASK_REMEMBER,				(float)bits_MEMORY_INCOVER	},
	{ TASK_FACE_ENEMY,				(float)0					},
};

Schedule_t	slAGruntTakeCoverFromEnemy[] =
{
	{ 
		tlAGruntTakeCoverFromEnemy,
		ARRAYSIZE ( tlAGruntTakeCoverFromEnemy ), 
		bits_COND_NEW_ENEMY,
		0,
		"AGruntTakeCoverFromEnemy"
	},
};

//=========================================================
// Victory dance!
//=========================================================
//MODDD - Is it possible to check to see if the recent enemy was organic (or organicLogic) and if so, only then route to the corpse
//        for the eating animation? Maybe an area check of anything organic near the LKP would be good enough?
Task_t	tlAGruntVictoryDance[] =
{
	{ TASK_STOP_MOVING,						(float)0					},
	{ TASK_SET_FAIL_SCHEDULE,				(float)SCHED_AGRUNT_THREAT_DISPLAY	},
	

	{ TASK_SET_ACTIVITY,					(float)ACT_IDLE				},
	//MODDD - was 0.2, changed to 5 to give whatever died some more time to die.
	{ TASK_WAIT,							(float)5					},
	//MODDD - and new.  Fails if there isn't even anything dead or organic near the LKP.
	{ TASK_GATE_ORGANICLOGIC_NEAR_LKP,		(float)0					},

	{ TASK_AGRUNT_GET_PATH_TO_ENEMY_CORPSE,	(float)0					},
	{ TASK_WALK_PATH,						(float)0					},
	{ TASK_WAIT_FOR_MOVEMENT,				(float)0					},
	{ TASK_FACE_ENEMY,						(float)0					},
	{ TASK_PLAY_SEQUENCE,					(float)ACT_CROUCH			},
	{ TASK_PLAY_SEQUENCE,					(float)ACT_VICTORY_DANCE	},
	{ TASK_PLAY_SEQUENCE,					(float)ACT_VICTORY_DANCE	},
	{ TASK_PLAY_SEQUENCE,					(float)ACT_STAND			},
	{ TASK_PLAY_SEQUENCE,					(float)ACT_THREAT_DISPLAY	},
	{ TASK_PLAY_SEQUENCE,					(float)ACT_CROUCH			},
	{ TASK_PLAY_SEQUENCE,					(float)ACT_VICTORY_DANCE	},
	{ TASK_PLAY_SEQUENCE,					(float)ACT_VICTORY_DANCE	},
	{ TASK_PLAY_SEQUENCE,					(float)ACT_VICTORY_DANCE	},
	{ TASK_PLAY_SEQUENCE,					(float)ACT_VICTORY_DANCE	},
	{ TASK_PLAY_SEQUENCE,					(float)ACT_VICTORY_DANCE	},
	{ TASK_PLAY_SEQUENCE,					(float)ACT_STAND			},
};

Schedule_t	slAGruntVictoryDance[] =
{
	{ 
		tlAGruntVictoryDance,
		ARRAYSIZE ( tlAGruntVictoryDance ), 
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE,
		0,
		"AGruntVictoryDance"
	},
};

//=========================================================
//=========================================================
Task_t	tlAGruntThreatDisplay[] =
{
	{ TASK_STOP_MOVING,			(float)0					},
	{ TASK_FACE_ENEMY,			(float)0					},
	{ TASK_PLAY_SEQUENCE,		(float)ACT_THREAT_DISPLAY	},
};

Schedule_t	slAGruntThreatDisplay[] =
{
	{ 
		tlAGruntThreatDisplay,
		ARRAYSIZE ( tlAGruntThreatDisplay ), 
		bits_COND_NEW_ENEMY			|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE,
		
		bits_SOUND_PLAYER			|
		bits_SOUND_COMBAT			|
		bits_SOUND_WORLD,
		"AGruntThreatDisplay"
	},
};

DEFINE_CUSTOM_SCHEDULES( CAGrunt )
{
	slAGruntFail,
	slAGruntCombatFail,
	slAGruntStandoff,
	slAGruntSuppress,
	slAGruntRangeAttack1,
	slAGruntHiddenRangeAttack,
	slAGruntTakeCoverFromEnemy,
	slAGruntVictoryDance,
	slAGruntThreatDisplay,
};

IMPLEMENT_CUSTOM_SCHEDULES( CAGrunt, CSquadMonster );









void CAGrunt::MonsterThink(void){


	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(thatWasntPunch) == 1 && this->m_fSequenceFinished){

		switch(RANDOM_LONG(0, 33)){

			case 0:
				this->SetSequenceByName("land_hard");
			break;
			case 1:
				this->SetSequenceByName("smashrail");
			break;
			case 2:
				this->SetSequenceByName("bigopen");
			break;
			case 3:
				this->SetSequenceByName("scare");
			break;
			case 4:
				this->SetSequenceByName("scare");
			break;
			case 5:
				this->SetSequenceByName("scare");
			break;
			case 6:
				this->SetSequenceByName("float");
			break;
			case 7:
				this->SetSequenceByName("float");
			break;
			case 8:
				this->SetSequenceByName("longshoot");
			break;
			case 9:
				this->SetSequenceByName("quickshoot");
			break;
			case 10:
				this->SetSequenceByName("quickshoot");
			break;
			case 11:
				this->SetSequenceByName("quickshoot");
			break;
			case 12:
				this->SetSequenceByName("quickshoot");
			break;
			case 13:
				this->SetSequenceByName("quickshoot");
			break;
			case 14:
				this->SetSequenceByName("attack3");
			break;
			case 15:
				this->SetSequenceByName("larmflinch");
			break;
			case 16:
				this->SetSequenceByName("llegflinch");
			break;
			case 17:
				this->SetSequenceByName("rarmflinch");
			break;
			case 18:
				this->SetSequenceByName("rlegflinch");
			break;
			case 19:
				this->SetSequenceByName("victorysquat");
			break;
			case 20:
				this->SetSequenceByName("mattack3");
			break;
			case 21:
				this->SetSequenceByName("mattack3");
			break;
			case 22:
				this->SetSequenceByName("mattack2");
			break;
			case 23:
				this->SetSequenceByName("bigflinch");
			break;
			case 24:
				this->SetSequenceByName("smallflinch");
			break;
			case 25:
				this->SetSequenceByName("smallflinch");
			break;
			case 26:
				this->SetSequenceByName("smallflinch");
			break;
			case 27:
				this->SetSequenceByName("smallflinch");
			break;
			case 28:
				this->SetSequenceByName("smallflinch");
			break;
			case 29:
				this->SetSequenceByName("smallflinch");
			break;
			case 30:
				this->SetSequenceByName("turnl");
			break;
			case 31:
				this->SetSequenceByName("turnl");
			break;
			case 32:
				this->SetSequenceByName("turnr");
			break;
			case 33:
				this->SetSequenceByName("turnr");
			break;
		}
	}//END OF whatever the ...heck... that was

	

	if(m_fIsPoweredUp){
		//random tick.
		if(forgetPowerupCauseEntDirectedEnemyTime != -1 && gpGlobals->time >= forgetPowerupCauseEntDirectedEnemyTime){
			forgetPowerupCauseEntDirectedEnemyTime = -1;

			powerupCauseEntDirectedEnemy = NULL;
			directedEnemyIssuer = NULL;
		}

		if(poweredUpTimeEnd >= gpGlobals->time){

			/*
			//not needed.
			const Vector& position = this->pev->origin + Vector(0, 0, pev->size.z/2);
			const int ballsToSpawn = 2;

			if(nextPoweredUpParticleTime == -1 || gpGlobals->time >= nextPoweredUpParticleTime){
				nextPoweredUpParticleTime = gpGlobals->time + RANDOM_FLOAT(0.38, 1.2);
				PLAYBACK_EVENT_FULL (FEV_GLOBAL, NULL, g_sCustomBallsPowerup, 0.0, (float *)&position, (float *)&Vector(0,0,0), 0.0, 0.0, ballsToSpawn, 0, FALSE, FALSE);
			}
			*/
		}//END OF still poweredup check
		else{
			//turn it off.
			setPoweredUpOff();
		}

	}//END OF if poweredup check

	CSquadMonster::MonsterThink();

	/*
	//DEBUGGING
	if(this->monsterID == 15){
		pev->renderamt = 0;
		pev->rendermode = kRenderNormal;
		pev->renderfx = kRenderFxGlowShell;
		pev->rendercolor.x = 255;
		pev->rendercolor.y = 0;
		pev->rendercolor.z = 255;
	}
	if(this->monsterID == 16){
		pev->renderamt = 0;
		pev->rendermode = kRenderNormal;
		pev->renderfx = kRenderFxGlowShell;
		pev->rendercolor.x = 100;
		pev->rendercolor.y = 0;
		pev->rendercolor.z = 255;
	}
	*/

}//MonsterThink


void CAGrunt::setPoweredUpOff(void){

	if(m_fIsPoweredUp == FALSE){
		//already off. This is redundant.
		return;
	}

	pev->renderamt = 0;
	pev->rendermode = kRenderNormal;
	pev->renderfx = kRenderFxNone;
	pev->rendercolor.x = 0;
	pev->rendercolor.y = 0;
	pev->rendercolor.z = 0;
	
	powerupCauseEntDirectedEnemy = NULL;
	directedEnemyIssuer = NULL;

	BOOL isAlive = (pev->deadflag == DEAD_NO) && !(m_IdealMonsterState == MONSTERSTATE_DEAD);

	m_fIsPoweredUp = FALSE;
	poweredUpTimeEnd = -1;

	//"FNullEnt" ??
	if(  poweredUpCauseEnt != NULL && poweredUpCauseEnt.Get() != NULL){
		//you're not the boss of me!
		poweredUpCauseEnt->GetMonsterPointer()->removeFromPoweredUpCommandList(this);

		poweredUpCauseEnt = NULL;
	}

	if(!isAlive){
		//do NOT attempt the rest of the checks if dead or dying! They involve adjusting the AI if this new behavior ought to change it.
		return;
	}

	if(m_hEnemy != NULL){

		/*
		Vector vecEnemyPos = pEnemy->pev->origin;
		// distance to enemy's origin
		flDistToEnemy = ( vecEnemyPos - pev->origin ).Length();
		vecEnemyPos.z += pEnemy->pev->size.z * 0.5;
		// distance to enemy's head
		float flDistToEnemy2 = (vecEnemyPos - pev->origin).Length();
		if (flDistToEnemy2 < flDistToEnemy)
			flDistToEnemy = flDistToEnemy2;
		else
		{
			// distance to enemy's feet
			vecEnemyPos.z -= pEnemy->pev->size.z;
			float flDistToEnemy2 = (vecEnemyPos - pev->origin).Length();
			if (flDistToEnemy2 < flDistToEnemy)
				flDistToEnemy = flDistToEnemy2;
		}

		CheckAttacks(m_hEnemy, disttt);
		*/

		if ( m_hEnemy != NULL )
		{
			CheckEnemy( m_hEnemy );
		}
	}


	/*
	if( this->m_pSchedule == slChaseEnemy || this->m_pSchedule == slChaseEnemySmart && this->m_movementActivity == ACT_RUN && this->m_IdealActivity == this->m_movementActivity){
		if(HasConditions(bits_COND_CAN_RANGE_ATTACK1 | bits_COND_CAN_RANGE_ATTACK2)){
			//If we were pursuing the target to deliver a melee attack (and could have range attacked anyways), stop.
			//TODO: change schedule to the appropriate range here? Or is the momentary brain freeze on picking a new schedule ok?
			TaskFail();
		}else{
			//so keep running. Perhaps slow it down if needed?
			setChaseSpeed();
		}
	}
	*/

}//END OF setPoweredUpOff()


void CAGrunt::setPoweredUpOn(CBaseMonster* argPoweredUpCauseEnt, float argHowLong){
	
	BOOL isAlive = (pev->deadflag == DEAD_NO) && !(m_IdealMonsterState == MONSTERSTATE_DEAD);

	if(!isAlive){
		//disallowed for anything but alive and well.
		return;
	}
	
	pev->renderamt = 0;
	pev->rendermode = kRenderNormal;
	pev->renderfx = kRenderFxGlowShell;
	pev->rendercolor.x = (int)(100*0.6);
	pev->rendercolor.y = 0;
	pev->rendercolor.z = (int)(255*0.6);
	
	if(m_fIsPoweredUp == FALSE){
		//play the powerup sound?
		UTIL_PlaySound( ENT(pev), CHAN_VOICE, "agrunt/ag_powerup.wav", 1.0, ATTN_NORM, 0, RANDOM_LONG(95, 105) );
	}

	m_fIsPoweredUp = TRUE;
	poweredUpTimeEnd = gpGlobals->time + argHowLong;

	//Decide: should we switch PoweredUpCauseEnt's ?
	//This is the ent that we're obeying - if this ent commands all "its" poweredup ents to attack, are we one of those?
	if(poweredUpCauseEnt == NULL || argPoweredUpCauseEnt==NULL || (poweredUpCauseEnt.Get() == argPoweredUpCauseEnt->edict() ) ){
		//No poweredUpCauseEnt? Or the current poweredup ent sent this message again? He is mine/still mine. That was easy.
		poweredUpCauseEnt = argPoweredUpCauseEnt;

		if(poweredUpCauseEnt == NULL){
			//receiving your first powerup ever / since losing the powerup resets the cooldown.
			poweredUpCauseEntChangeCooldown = gpGlobals->time + 20;
		}
	}else{
		//A competition? Compare the distances.
		float distToOldCause = (poweredUpCauseEnt->pev->origin - this->pev->origin).Length();
		float distToNewCause = (argPoweredUpCauseEnt->pev->origin - this->pev->origin).Length();


		//Two possible reasons to change to the new leader. The cooldown time has passed and we're closer to the new leader than we are to the old,
		//OR we're too far away from the old leader period.
		if( (gpGlobals->time >= poweredUpCauseEntChangeCooldown && distToNewCause < distToOldCause)
			|| (distToOldCause > 1600) ){
			//if he is closer, I go to him instead.
			poweredUpCauseEnt = argPoweredUpCauseEnt;
			poweredUpCauseEntChangeCooldown = gpGlobals->time + 20;   //don't change again for a bit, for hte most part.
		}else{
			//no change - keep your old poweredUpCause.
		}
	}

	/*
	if(this->m_pSchedule == slRangeAttack1 || this->m_pSchedule == slRangeAttack2){
		//we're in a frenzy, <disregard> that <substance of inadequate value>!
		TaskFail();
	}else if(this->m_pSchedule == slChaseEnemy || this->m_pSchedule == slChaseEnemySmart && this->m_movementActivity == ACT_RUN && this->m_IdealActivity == this->m_movementActivity){
		//speed up maybe?
		setChaseSpeed();
	}
	*/

}//END OF setPoweredUpOn


//=========================================================
// IRelationship - overridden because Human Grunts are 
// Alien Grunt's nemesis.
//=========================================================
int CAGrunt::IRelationship ( CBaseEntity *pTarget )
{

	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(thatWasntPunch) == 1){
		return R_NO;
	}

	BOOL canPrint = ::FClassnameIs(pTarget->pev, "player");

	//MODDD - insertion for the powerUp (mainly enemy focus) logic
	//////////////////////////////////////////////////////////////////////////////////////////////////
	if(powerupCauseEntDirectedEnemy != NULL){

		//if(canPrint)easyForcePrintLine("IRelationship monsterID%d: excuse me #1", monsterID);

		//I only care about who I was told to attack.
		
		//MODDD TODO - the kingpin can keep a list of enemies that have recently dealt damage to itself, and if this "pTarget" being checked
		//             is one of those (recent kingpin attacker), give it a high priority too?
		if(pTarget == powerupCauseEntDirectedEnemy){
			//Focus!
			//if(canPrint)easyForcePrintLine("IRelationship monsterID%d: excuse me #2", monsterID);
			return R_NM;
		}else{
			//don't care nearly as much. Maybe "R_NO" ?

			int iTypicalRelationship = CSquadMonster::IRelationship(pTarget);

			if(iTypicalRelationship >= R_DL){
				//Hostile towards them to any extent? Treat it as "dislike" for now.
				//if(canPrint)easyForcePrintLine("IRelationship monsterID%d: excuse me #3", monsterID);
				return R_DL;
			}else{
				//otherwise, anything from friendly to neutral (R_NO = 0)? Just stay that way.
				//if(canPrint)easyForcePrintLine("IRelationship monsterID%d: excuse me #4 (%d)", monsterID, iTypicalRelationship);
				return iTypicalRelationship;
			}

		}
	}
	//////////////////////////////////////////////////////////////////////////////////////////////////

	if ( FClassnameIs( pTarget->pev, "monster_human_grunt" ) )
	{
		return R_NM;
	}
	
	int iRelationship = CSquadMonster::IRelationship( pTarget );
	//if(canPrint)easyForcePrintLine("IRelationship monsterID%d: excuse me #5 (%d)", monsterID, iRelationship);
	return iRelationship;
}

//=========================================================
// ISoundMask 
//=========================================================
int CAGrunt::ISoundMask ( void )
{
	return	bits_SOUND_WORLD	|
			bits_SOUND_COMBAT	|
			bits_SOUND_PLAYER	|
			bits_SOUND_DANGER	|
			//also attracted to bait.
			bits_SOUND_BAIT
			;
}


GENERATE_TRACEATTACK_IMPLEMENTATION(CAGrunt)
{
	//easyForcePrintLine("AGrunt::TraceAttack says I took %.2f damage.", flDamage);
	//easyForcePrintLine("agrunt: TraceAttack hitgroup:%d B:%d S:%d C:%d", ptr->iHitgroup, (bitsDamageType & (DMG_BULLET)), (bitsDamageType & (DMG_SLASH)), (bitsDamageType & (DMG_CLUB))   );
	//easyForcePrintLine("ILL %d %d", (ptr->iHitgroup), (bitsDamageType & (DMG_BULLET | DMG_SLASH | DMG_CLUB)) != 0 );

	
	if ((bitsDamageType & DMG_BLAST) || (bitsDamageTypeMod & DMG_HITBOX_EQUAL)) {
		//MODDD - DMG_BLAST isn't at all precise, shots for whatever hitgroup are a shot in the dark.
		// So ignore the ihtgroup check.  Have a flat 30% reduction, and a ricochet effect anyway.  ...if it makes sense?
		flDamage = flDamage * 0.70;
		if (ptr->iHitgroup == HITGROUP_AGRUNT_ARMOR){
			UTIL_Ricochet(ptr->vecEndPos, RANDOM_FLOAT(2.5, 3.4));
			if (useBulletHitSound) { *useBulletHitSound = FALSE; }
			useBloodEffect = FALSE;
		}
	}else if (ptr->iHitgroup == HITGROUP_AGRUNT_ARMOR) {
		// NOTICE - this hitgroup is armor anywhere on the agrunt, not just the helmet like several other entities.
		// ALSO, protecting against DMG_BLAST now.  HGrunt does, so why not.
		// damage under 20 won't be reduced so hard but will still be greatly changed
		// damage split up.  only BULLET should have the RicochetTracer effect.
		if (bitsDamageType & (DMG_BULLET)) {
			if (flDamage <= 20) {

				
				//flDamage = 0.1;// don't hurt the monster much, but allow bits_COND_LIGHT_DAMAGE to be generated
				// Do this instead, 12% damage.
				flDamage = flDamage * 0.12;
				//MODDD - only do ricochet effects if the damage was effectively blocked (not enough to 'pierce' the armor)
				// hit armor
				if (pev->dmgtime != gpGlobals->time || (RANDOM_LONG(0, 10) < 1)) {
					//MODDD - little tighter ricochet range, was 1 to 2.
					UTIL_Ricochet(ptr->vecEndPos, RANDOM_FLOAT(1.2, 1.8));
					pev->dmgtime = gpGlobals->time;
				}
				//MODDD - why a random chance?  Just do it.
				//if (RANDOM_LONG(0, 1) == 0) {
					//MODDD - turned into method.
					// And, sending 'ptr->vecPlaneNormal' instead of 'vecDir' for a source of better reflection.
					// Would have to be '-vecDir' now anyway, method no longer multiplies the direction by -1.
					UTIL_RicochetTracer(ptr->vecEndPos, ptr->vecPlaneNormal);
				//}
				if (useBulletHitSound) { *useBulletHitSound = FALSE; }
				useBloodEffect = FALSE;
				

			}else {
				//MODDD - Hitting the armor still shouldn't be insignificant, reduce damage by 15%.
				flDamage = flDamage * 0.85;
			}
		}else if (bitsDamageType & (DMG_SLASH | DMG_CLUB) ) {
			if (flDamage <= 20) {
				// reduce by 40% instead.
				flDamage = flDamage * 0.6;
				UTIL_Ricochet(ptr->vecEndPos, RANDOM_FLOAT(1.5, 2.3));

				if (useBulletHitSound) { *useBulletHitSound = FALSE; }
				useBloodEffect = FALSE;
			}else {
				//MODDD - Hitting the armor still shouldn't be insignificant, reduce damage by 10%.  If this even exists.
				flDamage = flDamage * 0.90;
			}
		}

	}
	else {
		// hit anywhere else?  Pretty fleshy and exposed, but kinda tough-looking I guess.  8%.
		if (bitsDamageType & (DMG_BULLET)) {
			flDamage = flDamage * 0.92;
		}
	}
	//END OF armor hitgroup check

	
	
	//easyForcePrintLine("AGrunt::TraceAttack ended with %.2f damage.", flDamage);
	
	// In as-is, we skipped call to the parent TraceAttack bbbbeeeeeecccccccaaaaaaauuuuuuussssssseeeeee?
	// The hgrunt doesn't........
	//AddMultiDamage( pevAttacker, this, flDamage, bitsDamageType, bitsDamageTypeMod );
	GENERATE_TRACEATTACK_PARENT_CALL(CSquadMonster);
}

//=========================================================
// StopTalking - won't speak again for 10-20 seconds.
//=========================================================
void CAGrunt::StopTalking( void )
{
	m_flNextWordTime = m_flNextSpeakTime = gpGlobals->time + 10 + RANDOM_LONG(0, 10);
}

//=========================================================
// ShouldSpeak - Should this agrunt be talking?
//=========================================================
BOOL CAGrunt::ShouldSpeak( void )
{
	if ( m_flNextSpeakTime > gpGlobals->time )
	{
		// my time to talk is still in the future.
		return FALSE;
	}

	if ( pev->spawnflags & SF_MONSTER_GAG )
	{
		if ( m_MonsterState != MONSTERSTATE_COMBAT )
		{
			// if gagged, don't talk outside of combat.
			// if not going to talk because of this, put the talk time 
			// into the future a bit, so we don't talk immediately after 
			// going into combat
			m_flNextSpeakTime = gpGlobals->time + 3;
			return FALSE;
		}
	}

	return TRUE;
}

//=========================================================
// PrescheduleThink 
//=========================================================
void CAGrunt::PrescheduleThink ( void )
{
	if ( ShouldSpeak() )
	{
		if ( m_flNextWordTime < gpGlobals->time )
		{
			int num = -1;

			do
			{
				num = RANDOM_LONG(0,ARRAYSIZE(pIdleSounds)-1);
			} while( num == m_iLastWord );

			m_iLastWord = num;

			// play a new sound
			UTIL_PlaySound( ENT(pev), CHAN_VOICE, pIdleSounds[ num ], 1.0, ATTN_NORM );

			// is this word our last?
			if ( RANDOM_LONG( 1, 10 ) <= 1 )
			{
				// stop talking.
				StopTalking();
			}
			else
			{
				m_flNextWordTime = gpGlobals->time + RANDOM_FLOAT( 0.5, 1 );
			}
		}
	}
}

//=========================================================
// DieSound
//=========================================================
void CAGrunt::DeathSound ( void )
{
	StopTalking();

	UTIL_PlaySound( ENT(pev), CHAN_VOICE, pDieSounds[RANDOM_LONG(0,ARRAYSIZE(pDieSounds)-1)], 1.0, ATTN_NORM );
}

//=========================================================
// AlertSound
//=========================================================
void CAGrunt::AlertSound ( void )
{
	StopTalking();
	UTIL_PlaySound( ENT(pev), CHAN_VOICE, pAlertSounds[RANDOM_LONG(0,ARRAYSIZE(pAlertSounds)-1)], 1.0, ATTN_NORM );
}

//=========================================================
// AttackSound
//=========================================================
void CAGrunt::AttackSound ( void )
{
	StopTalking();
	UTIL_PlaySound( ENT(pev), CHAN_VOICE, pAttackSounds[RANDOM_LONG(0,ARRAYSIZE(pAttackSounds)-1)], 1.0, ATTN_NORM );
}

//=========================================================
// PainSound
//=========================================================
void CAGrunt::PainSound ( void )
{
	if ( m_flNextPainTime > gpGlobals->time )
	{
		return;
	}

	m_flNextPainTime = gpGlobals->time + 0.6;

	StopTalking();
	UTIL_PlaySound( ENT(pev), CHAN_VOICE, pPainSounds[RANDOM_LONG(0,ARRAYSIZE(pPainSounds)-1)], 1.0, ATTN_NORM );
}

//=========================================================
// Classify - indicates this monster's place in the 
// relationship table.
//=========================================================
int CAGrunt::Classify ( void )
{
	return	CLASS_ALIEN_MILITARY;
}
BOOL CAGrunt::isOrganic(void){
	return TRUE;
}

//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CAGrunt::SetYawSpeed ( void )
{
	int ys;

	switch ( m_Activity )
	{
	//MODDD - like grunts, turn a little faster while running or walking.
	case ACT_RUN:
		ys = 140;
		break;
	case ACT_WALK:
		ys = 160;
		break;

	case ACT_TURN_LEFT:
	case ACT_TURN_RIGHT:
		ys = 110;
		break;
	default:
		ys = 100;
	}

	pev->yaw_speed = ys;
}

//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//
// Returns number of events handled, 0 if none.
//=========================================================
void CAGrunt::HandleAnimEvent( MonsterEvent_t *pEvent )
{

	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(thatWasntPunch) == 1){
		return;
	}

	switch( pEvent->event )
	{
	case AGRUNT_AE_HORNET1:
	case AGRUNT_AE_HORNET2:
	case AGRUNT_AE_HORNET3:
	case AGRUNT_AE_HORNET4:
	case AGRUNT_AE_HORNET5:
		{
			// m_vecEnemyLKP should be center of enemy body
			Vector vecArmPos;
			Vector vecDirToEnemy;
			Vector angDir;

			if (HasConditions(bits_COND_SEE_ENEMY))
			{
				vecDirToEnemy = ((m_vecEnemyLKP)-pev->origin);
				vecDirToEnemy = vecDirToEnemy.Normalize();
			}
			else {
				UTIL_MakeAimVectors(pev->angles);
				vecDirToEnemy = gpGlobals->v_forward;
			}

			if(EASY_CVAR_GET_DEBUGONLY(agrunt_muzzleflash) != 0){
				pev->effects = EF_MUZZLEFLASH;
			}

			lookAtEnemy_pitch();


			//MODDD - easier to recognize.
			//GetAttachment( 0, vecArmPos, vecArmDir );
			vecArmPos = GetGunPosition();

			//MODDD - toned down from 32.
			Vector vecArmMuzzlePos = vecArmPos + vecDirToEnemy * 20;
			Vector vecHornetSpawnPos = vecArmPos + vecDirToEnemy * 12;


			if(EASY_CVAR_GET_DEBUGONLY(agrunt_muzzleflash) != 0){
				MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, vecArmPos );
					WRITE_BYTE( TE_SPRITE );
					WRITE_COORD(vecArmMuzzlePos.x );	// pos
					WRITE_COORD(vecArmMuzzlePos.y );
					WRITE_COORD(vecArmMuzzlePos.z );
					WRITE_SHORT( iAgruntMuzzleFlash );		// model
					WRITE_BYTE( 6 );				// size * 10
					WRITE_BYTE( 128 );			// brightness
				MESSAGE_END();
			}

			CBaseEntity *pHornet = CBaseEntity::Create( "hornet", vecHornetSpawnPos, UTIL_VecToAngles( vecDirToEnemy ), edict() );
			UTIL_MakeVectors ( pHornet->pev->angles );
			
			//MODDD - change, explanation above.
			//pHornet->pev->velocity = gpGlobals->v_forward * 300;

			if(m_fIsPoweredUp){
				//MODDD - is the powerup hornet speedup + difficulty a good idea? They may have a harder time homing in on a target at higher speeds.
				if(g_iSkillLevel < SKILL_HARD){
					pHornet->pev->velocity = vecDirToEnemy * 350;
				}else{
					pHornet->pev->velocity = vecDirToEnemy * 400;
				}
			}else{
				pHornet->pev->velocity = vecDirToEnemy * 300;
			}
			
			switch ( RANDOM_LONG ( 0 , 2 ) )
			{
				case 0:	UTIL_PlaySound( ENT(pev), CHAN_WEAPON, "agrunt/ag_fire1.wav", 1.0, ATTN_NORM, 0, 100 );	break;
				case 1:	UTIL_PlaySound( ENT(pev), CHAN_WEAPON, "agrunt/ag_fire2.wav", 1.0, ATTN_NORM, 0, 100 );	break;
				case 2:	UTIL_PlaySound( ENT(pev), CHAN_WEAPON, "agrunt/ag_fire3.wav", 1.0, ATTN_NORM, 0, 100 );	break;
			}

			CBaseMonster *pHornetMonster = pHornet->MyMonsterPointer();

			if ( pHornetMonster )
			{
				pHornetMonster->m_hEnemy = m_hEnemy;
			}
		}
		break;

	//MODDD NOTE - BEWARE. These aren't in the soundSentenceSave because the player already precaches them.
	//MODDD - slightly lower pitches. was 70, now between 67 and 72.
	case AGRUNT_AE_LEFT_FOOT:
		switch (RANDOM_LONG(0,1))
		{
		// left foot
		case 0:	UTIL_PlaySound( ENT(pev), CHAN_BODY, "player/pl_ladder2.wav", 1, ATTN_NORM, 0, RANDOM_LONG(67, 72), FALSE );	break;
		case 1:	UTIL_PlaySound( ENT(pev), CHAN_BODY, "player/pl_ladder4.wav", 1, ATTN_NORM, 0, RANDOM_LONG(67, 72), FALSE );	break;
		}
		break;
	case AGRUNT_AE_RIGHT_FOOT:
		// right foot
		switch (RANDOM_LONG(0,1))
		{
		case 0:	UTIL_PlaySound( ENT(pev), CHAN_BODY, "player/pl_ladder1.wav", 1, ATTN_NORM, 0, RANDOM_LONG(67, 72), FALSE );	break;
		case 1:	UTIL_PlaySound( ENT(pev), CHAN_BODY, "player/pl_ladder3.wav", 1, ATTN_NORM, 0, RANDOM_LONG(67, 72), FALSE );	break;
		}
		break;

	case AGRUNT_AE_LEFT_PUNCH:
		{
			//MODDD - added bleeding.  All alien melee seems strong (big arms, large body structure).
			CBaseEntity *pHurt = CheckTraceHullAttack( AGRUNT_MELEE_DIST, gSkillData.agruntDmgPunch, DMG_CLUB, DMG_BLEEDING );
			
			if ( pHurt )
			{
				if(!pHurt->blocksImpact()){
					pHurt->pev->punchangle.y = -25;
					pHurt->pev->punchangle.x = 8;
					// OK to use gpGlobals without calling MakeVectors, cause CheckTraceHullAttack called it above.
					if ( pHurt->IsPlayer() )
					{
						// this is a player. Knock him around.
						pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_right * 250;
					}
				}

				UTIL_PlaySound( ENT(pev), CHAN_WEAPON, pAttackHitSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackHitSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );

				//MODDD - blood call not needed!  The CheckTraceHullAttack call already does this.
				//SpawnBlood(vecArmPos, pHurt->BloodColor(), 25);// a little surface blood.
			}
			else
			{
				// Play a random attack miss sound
				UTIL_PlaySound( ENT(pev), CHAN_WEAPON, pAttackMissSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackMissSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );
			}
		}
		break;

	case AGRUNT_AE_RIGHT_PUNCH:
		{
			//MODDD - added bleeding to all melee.
			CBaseEntity *pHurt = CheckTraceHullAttack( AGRUNT_MELEE_DIST, gSkillData.agruntDmgPunch, DMG_CLUB, DMG_BLEEDING);

			if ( pHurt )
			{
				
				if(!pHurt->blocksImpact()){
					pHurt->pev->punchangle.y = 25;
					pHurt->pev->punchangle.x = 8;
					// OK to use gpGlobals without calling MakeVectors, cause CheckTraceHullAttack called it above.
					if ( pHurt->IsPlayer() )
					{
						// this is a player. Knock him around.
						pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_right * -250;
					}
				}

				UTIL_PlaySound( ENT(pev), CHAN_WEAPON, pAttackHitSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackHitSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );

				//MODDD - blood call not needed!  The CheckTraceHullAttack call already does this.
				//SpawnBlood(vecArmPos, pHurt->BloodColor(), 25);// a little surface blood.
			}
			else
			{
				// Play a random attack miss sound
				UTIL_PlaySound( ENT(pev), CHAN_WEAPON, pAttackMissSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackMissSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );
			}
		}
		break;

	default:
		CSquadMonster::HandleAnimEvent( pEvent );
		break;
	}
}


CAGrunt::CAGrunt(){
	m_fIsPoweredUp = FALSE;
	nextPoweredUpParticleTime = -1;
	poweredUpTimeEnd = -1;
	poweredUpCauseEnt = NULL;
	poweredUpCauseEntChangeCooldown = -1;
	powerupCauseEntDirectedEnemy = NULL;
	forceNewEnemyCooldownTightTime = -1;
	forceNewEnemyCooldownTime = -1;
	directedEnemyIssuer = NULL;
	forgetPowerupCauseEntDirectedEnemyTime = -1;

}

//=========================================================
// Spawn
//=========================================================
void CAGrunt::Spawn()
{
	Precache( );

	SET_MODEL(ENT(pev), "models/agrunt.mdl");
	UTIL_SetSize(pev, Vector(-32, -32, 0), Vector(32, 32, 64));

	pev->classname = MAKE_STRING("monster_alien_grunt");

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_GREEN;
	pev->effects		= 0;
	pev->health			= gSkillData.agruntHealth;
	m_flFieldOfView		= 0.2;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState		= MONSTERSTATE_NONE;
	m_afCapability		= 0;
	m_afCapability		|= bits_CAP_SQUAD;

	//MODDD NOTE - Bizarre, why even have a hackedGunPos? The "GetGunPosition" method was never called in the as-is script.
	//             Going to use it in GetGunPositionAI.
	m_HackedGunPos		= Vector( 24, 64, 48 );

	m_flNextSpeakTime	= m_flNextWordTime = gpGlobals->time + RANDOM_LONG(10, 20);

	MonsterInit();
}


extern int global_useSentenceSave;
//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CAGrunt::Precache()
{
	global_useSentenceSave = 1;

	PRECACHE_MODEL("models/agrunt.mdl");


	PRECACHE_SOUND_ARRAY(pAttackHitSounds)
	PRECACHE_SOUND_ARRAY(pAttackMissSounds)
	PRECACHE_SOUND_ARRAY(pIdleSounds)
	PRECACHE_SOUND_ARRAY(pDieSounds)
	PRECACHE_SOUND_ARRAY(pPainSounds)
	PRECACHE_SOUND_ARRAY(pAttackSounds)
	PRECACHE_SOUND_ARRAY(pAlertSounds)

	// what... why.
	//PRECACHE_SOUND( "hassault/hw_shoot1.wav" );
	PRECACHE_SOUND( "agrunt/ag_powerup.wav" );
	
	iAgruntMuzzleFlash = PRECACHE_MODEL( "sprites/muz4.spr" );

	global_useSentenceSave = 0;
	global_useSentenceSave = 1;

	UTIL_PrecacheOther( "hornet" );

	global_useSentenceSave = 0;	
}	


BOOL CAGrunt::needsMovementBoundFix(void){
	return TRUE;
}





//=========================================================
// FCanCheckAttacks - this is overridden for alien grunts
// because they can use their smart weapons against unseen
// enemies. Base class doesn't attack anyone it can't see.
//=========================================================
BOOL CAGrunt::FCanCheckAttacks ( void )
{
	if ( !HasConditions( bits_COND_ENEMY_TOOFAR ) )
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

//=========================================================
// CheckMeleeAttack1 - alien grunts zap the crap out of 
// any enemy that gets too close. 
//=========================================================
BOOL CAGrunt::CheckMeleeAttack1 ( float flDot, float flDist )
{
	if ( HasConditions ( bits_COND_SEE_ENEMY ) && flDist <= AGRUNT_MELEE_DIST && flDot >= 0.6 && m_hEnemy != NULL )
	{
		return TRUE;
	}
	return FALSE;
}

//=========================================================
// CheckRangeAttack1 
//
// !!!LATER - we may want to load balance this. Several
// tracelines are done, so we may not want to do this every
// server frame. Definitely not while firing. 
//=========================================================
BOOL CAGrunt::CheckRangeAttack1 ( float flDot, float flDist )
{

	//MODDD
	/*
	if(m_fIsPoweredUp){
		//TOO ENRAGED - NOPE
		//...COUNTER. not anymore, not how it works. Ranged attacks still allowed.
		return FALSE;
	}
	*/


	if ( gpGlobals->time < m_flNextHornetAttackCheck )
	{
		return m_fCanHornetAttack;
	}

	if ( HasConditions( bits_COND_SEE_ENEMY ) && flDist >= AGRUNT_MELEE_DIST && flDist <= 1024 && flDot >= 0.5 && NoFriendlyFire() )
	{
		TraceResult	tr;
		Vector	vecArmPos;

		// verify that a shot fired from the gun will hit the enemy before the world.
		// !!!LATER - we may wish to do something different for projectile weapons as opposed to instant-hit
		UTIL_MakeVectors( pev->angles );
		
		//Too precise, not necessary for just checking whether a ranged attack is allowed or not.
		//GetAttachment( 0, vecArmPos, vecArmDir );
		vecArmPos = GetGunPositionAI();


//		UTIL_TraceLine( vecArmPos, vecArmPos + gpGlobals->v_forward * 256, ignore_monsters, ENT(pev), &tr);
		UTIL_TraceLine( vecArmPos, m_hEnemy->BodyTargetMod(vecArmPos), dont_ignore_monsters, ENT(pev), &tr);

		if ( tr.flFraction == 1.0 || tr.pHit == m_hEnemy->edict() )
		{
			m_flNextHornetAttackCheck = gpGlobals->time + RANDOM_FLOAT( 2, 5 );
			m_fCanHornetAttack = TRUE;
			return m_fCanHornetAttack;
		}
	}
	
	m_flNextHornetAttackCheck = gpGlobals->time + 0.2;// don't check for half second if this check wasn't successful
	m_fCanHornetAttack = FALSE;
	return m_fCanHornetAttack;
}

//=========================================================
// StartTask
//=========================================================
void CAGrunt::StartTask ( Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_AGRUNT_GET_PATH_TO_ENEMY_CORPSE:
		{
			UTIL_MakeVectors( pev->angles );
			if ( BuildRoute ( m_vecEnemyLKP - gpGlobals->v_forward * 50, bits_MF_TO_LOCATION, NULL ) )
			{
				TaskComplete();
			}
			else
			{
				ALERT ( at_aiconsole, "AGruntGetPathToEnemyCorpse failed!!\n" );
				TaskFail();
			}
		}
		break;

	case TASK_AGRUNT_SETUP_HIDE_ATTACK:
		// alien grunt shoots hornets back out into the open from a concealed location. 
		// try to find a spot to throw that gives the smart weapon a good chance of finding the enemy.
		// ideally, this spot is along a line that is perpendicular to a line drawn from the agrunt to the enemy.

		CBaseMonster	*pEnemyMonsterPtr;

		pEnemyMonsterPtr = m_hEnemy->MyMonsterPointer();

		if ( pEnemyMonsterPtr )
		{
			Vector		vecCenter;
			TraceResult	tr;
			BOOL		fSkip;

			fSkip = FALSE;
			vecCenter = Center();

			//MODDD - whoops!  As-is, never saved this or did some kind of "MakeVectors" call.
			// What is this gpGlobals->v_forward then?
			//UTIL_VecToAngles( m_vecEnemyLKP - pev->origin );
			Vector theDir = (m_vecEnemyLKP - pev->origin).Normalize();
			Vector theAng = UTIL_VecToAngles(theDir);
			UTIL_MakeAimVectors(theAng);

			UTIL_TraceLine( Center() + gpGlobals->v_forward * 128, m_vecEnemyLKP, ignore_monsters, ENT(pev), &tr);
			if ( tr.flFraction == 1.0 )
			{
				MakeIdealYaw ( pev->origin + gpGlobals->v_right * 128 );
				fSkip = TRUE;
				TaskComplete();
			}
			
			if ( !fSkip )
			{
				UTIL_TraceLine( Center() - gpGlobals->v_forward * 128, m_vecEnemyLKP, ignore_monsters, ENT(pev), &tr);
				if ( tr.flFraction == 1.0 )
				{
					MakeIdealYaw ( pev->origin - gpGlobals->v_right * 128 );
					fSkip = TRUE;
					TaskComplete();
				}
			}
			
			if ( !fSkip )
			{
				UTIL_TraceLine( Center() + gpGlobals->v_forward * 256, m_vecEnemyLKP, ignore_monsters, ENT(pev), &tr);
				if ( tr.flFraction == 1.0 )
				{
					MakeIdealYaw ( pev->origin + gpGlobals->v_right * 256 );
					fSkip = TRUE;
					TaskComplete();
				}
			}
			
			if ( !fSkip )
			{
				UTIL_TraceLine( Center() - gpGlobals->v_forward * 256, m_vecEnemyLKP, ignore_monsters, ENT(pev), &tr);
				if ( tr.flFraction == 1.0 )
				{
					MakeIdealYaw ( pev->origin - gpGlobals->v_right * 256 );
					fSkip = TRUE;
					TaskComplete();
				}
			}
			
			if ( !fSkip )
			{
				TaskFail();
			}
		}
		else
		{
			ALERT ( at_aiconsole, "AGRunt - no enemy monster ptr!!!\n" );
			TaskFail();
		}
		break;

	default:
		CSquadMonster::StartTask ( pTask );
		break;
	}
}



void CAGrunt::RunTask( Task_t* pTask ){
	
	switch( pTask->iTask ){
		case TASK_RANGE_ATTACK1:{
			lookAtEnemy_pitch();
			CSquadMonster::RunTask(pTask);
		break;}
		default:{
			CSquadMonster::RunTask(pTask);
		break;}
	}//END OF switch

}//END OF RunTask



//=========================================================
// GetSchedule - Decides which type of schedule best suits
// the monster's current state and conditions. Then calls
// monster's member function to get a pointer to a schedule
// of the proper type.
//=========================================================
Schedule_t *CAGrunt::GetSchedule ( void )
{
	if ( HasConditions(bits_COND_HEAR_SOUND) )
	{
		CSound *pSound;
		pSound = PBestSound();

		ASSERT( pSound != NULL );
		if ( pSound && (pSound->m_iType & bits_SOUND_DANGER) )
		{
			// dangerous sound nearby!
			return GetScheduleOfType( SCHED_TAKE_COVER_FROM_BEST_SOUND );
		}


		//MODDD - new.
		SCHEDULE_TYPE baitSched = getHeardBaitSoundSchedule(pSound);

		if(baitSched != SCHED_NONE){
			return GetScheduleOfType ( baitSched );
		}

	}

	switch	( m_MonsterState )
	{
	case MONSTERSTATE_COMBAT:
		{

// dead enemy
			if ( HasConditions( bits_COND_ENEMY_DEAD ) )
			{
				// call base class, all code to handle dead enemies is centralized there.
				return CBaseMonster::GetSchedule();
			}


			//MODDD - heavy damage must flinch. The new standard for heavy damage (70% of max health) in one hit that is.
			if(HasConditions(bits_COND_HEAVY_DAMAGE)){
				return GetScheduleOfType(SCHED_BIG_FLINCH);
			}

			
			//MODDD - extra condition required to flinch (or, specifically forbidden)
			//        Now uses LIGHT_DAMAGE instead of HEAVY for the small flinch activity, but it still requires 20 damage to happen instead to mirror retail's intentions.
			//  ALSO, moved here from below.  If flinching is possible, do that.
			//  And added back the MEMORY_FLINCHED check seen in a lot of other areas.
			//if ( HasConditions ( bits_COND_HEAVY_DAMAGE ) && (!(global_noFlinchOnHard==1 && g_iSkillLevel==SKILL_HARD)) )
			if ( HasConditions ( bits_COND_LIGHT_DAMAGE ) && !HasMemory( bits_MEMORY_FLINCHED) && (!(EASY_CVAR_GET_DEBUGONLY(noFlinchOnHard)==1 && g_iSkillLevel==SKILL_HARD)) )
			{
 				return GetScheduleOfType( SCHED_SMALL_FLINCH );
			}

			if ( HasConditions(bits_COND_NEW_ENEMY) )
			{
				return GetScheduleOfType( SCHED_WAKE_ANGRY );
			}

	//MODDD - this comment. what? Did you get pasted from the islave, original dev's?
	// zap player!
			if ( HasConditions ( bits_COND_CAN_MELEE_ATTACK1 ) )
			{
				AttackSound();// this is a total hack. Should be parto f the schedule
				return GetScheduleOfType ( SCHED_MELEE_ATTACK1 );
			}

			//MODDD - old small flinch location.

	// can attack
			if ( HasConditions ( bits_COND_CAN_RANGE_ATTACK1 ) && OccupySlot ( bits_SLOTS_AGRUNT_HORNET ) )
			{
				return GetScheduleOfType ( SCHED_RANGE_ATTACK1 );
			}

			if ( OccupySlot ( bits_SLOT_AGRUNT_CHASE ) )
			{
				return GetScheduleOfType ( SCHED_CHASE_ENEMY );
			}

			return GetScheduleOfType ( SCHED_STANDOFF );
		}
	}

	return CSquadMonster::GetSchedule();
}

//=========================================================
//=========================================================
Schedule_t* CAGrunt::GetScheduleOfType ( int Type ) 
{
	//easyForcePrintLine("agrunt%d: GetScheduleOfType: %d", monsterID, Type);
	switch	( Type )
	{
	case SCHED_TAKE_COVER_FROM_ENEMY:
		return &slAGruntTakeCoverFromEnemy[ 0 ];
		break;
	
	case SCHED_RANGE_ATTACK1:
		if ( HasConditions( bits_COND_SEE_ENEMY ) )
		{
			//normal attack
			return &slAGruntRangeAttack1[ 0 ];
		}
		else
		{
			// attack an unseen enemy
			// return &slAGruntHiddenRangeAttack[ 0 ];
			return &slAGruntRangeAttack1[ 0 ];
		}
		break;

	case SCHED_AGRUNT_THREAT_DISPLAY:
		return &slAGruntThreatDisplay[ 0 ];
		break;

	case SCHED_AGRUNT_SUPPRESS:
		return &slAGruntSuppress[ 0 ];
		break;

	case SCHED_STANDOFF:
		return &slAGruntStandoff[ 0 ];
		break;

	case SCHED_VICTORY_DANCE:
		return &slAGruntVictoryDance[ 0 ];
		break;

	case SCHED_FAIL:
		// no fail schedule specified, so pick a good generic one.
		{
			if ( m_hEnemy != NULL )
			{
				// I have an enemy
				// !!!LATER - what if this enemy is really far away and i'm chasing him?
				// this schedule will make me stop, face his last known position for 2 
				// seconds, and then try to move again
				return &slAGruntCombatFail[ 0 ];
			}

			return &slAGruntFail[ 0 ];
		}
		break;

	}

	return CSquadMonster::GetScheduleOfType( Type );
}


GENERATE_KILLED_IMPLEMENTATION(CAGrunt){
	//easyForcePrintLine("CAGrunt: KILLED. iGib: %d", iGib);

	//if applicable.  Moved to before killed. that is ok, yes?
	setPoweredUpOff();

	GENERATE_KILLED_PARENT_CALL(CSquadMonster);
}


BOOL CAGrunt::canResetBlend0(void){
	return TRUE;
}

BOOL CAGrunt::onResetBlend0(void){
	//add something?
	lookAtEnemy_pitch();
	/*
	//MODDD - REPLACED. This should do it fine.
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
	*/

	return TRUE;
}

	
//changed these two??
BOOL CAGrunt::getMonsterBlockIdleAutoUpdate(void){
	return FALSE;
}
BOOL CAGrunt::forceIdleFrameReset(void){
	return FALSE;
}

BOOL CAGrunt::usesAdvancedAnimSystem(void){
	return TRUE;
}

int CAGrunt::LookupActivityHard(int activity){
	int i = 0;
	m_flFramerateSuggestion = 1;
	pev->framerate = 1;
	//any animation events in progress?  Clear it.
	resetEventQueue();

	//Within an ACTIVITY, pick an animation like this (with whatever logic / random check first):
	//    this->animEventQueuePush(10.0f / 30.0f, 3);  //Sets event #3 to happen at 1/3 of a second
	//    return LookupSequence("die_backwards");      //will play animation die_backwards

	//no need for default, just falls back to the normal activity lookup.
	switch(activity){
		case ACT_RUN:
			/*
			if(this->m_fIsPoweredUp && (m_pSchedule == slChaseEnemy || this->m_pSchedule == slChaseEnemySmart) ){
				setChaseSpeed();
			}
			*/
			setChaseSpeed();  //nah, just always move faster.
		break;
		case ACT_RANGE_ATTACK1:
		case ACT_RANGE_ATTACK2:
			if(g_iSkillLevel == SKILL_EASY){
				m_flFramerateSuggestion = 1.0;
			}else if(g_iSkillLevel == SKILL_MEDIUM){
				m_flFramerateSuggestion = 1.22;
			}else if(g_iSkillLevel == SKILL_HARD){
				m_flFramerateSuggestion = 1.37;
			}
		break;
		case ACT_MELEE_ATTACK1:
		case ACT_MELEE_ATTACK2:
			if(g_iSkillLevel == SKILL_EASY){
				m_flFramerateSuggestion = 1.0;
			}else if(g_iSkillLevel == SKILL_MEDIUM){
				m_flFramerateSuggestion = 1.4;
			}else if(g_iSkillLevel == SKILL_HARD){
				m_flFramerateSuggestion = 1.8;
			}
		break;
		case ACT_SMALL_FLINCH:
		case ACT_BIG_FLINCH:			//MODDD NOTE - is this effective?
		case ACT_FLINCH_HEAD:
		case ACT_FLINCH_CHEST:
		case ACT_FLINCH_STOMACH:
		case ACT_FLINCH_LEFTARM:
		case ACT_FLINCH_RIGHTARM:
		case ACT_FLINCH_LEFTLEG:
		case ACT_FLINCH_RIGHTLEG:
			if(g_iSkillLevel == SKILL_EASY){
				m_flFramerateSuggestion = 1.0;
			}else if(g_iSkillLevel == SKILL_MEDIUM){
				m_flFramerateSuggestion = 1.6;
			}else if(g_iSkillLevel == SKILL_HARD){
				m_flFramerateSuggestion = 2.0;
			}
		break;
	}//END OF switch(...)
	
	//not handled by above?  try the real deal.
	return CBaseAnimating::LookupActivity(activity);
}//END OF LookupActivityHard(...)


int CAGrunt::tryActivitySubstitute(int activity){
	int i = 0;

	//no need for default, just falls back to the normal activity lookup.
	switch(activity){
		case ACT_RUN:

		break;
	}//END OF switch(...)


	//not handled by above? We're not using the script to determine animation then. Rely on the model's anim for this activity if there is one.
	return CBaseAnimating::LookupActivity(activity);
}//END OF tryActivitySubstitute(...)

//Handles custom events sent from "LookupActivityHard", which sends events as timed delays along with picking an animation in script.
//So this handles script-provided events, not model ones.
void CAGrunt::HandleEventQueueEvent(int arg_eventID){

	switch(arg_eventID){
	case 0:
	{


	break;
	}
	case 1:
	{


	break;
	}
	}//END OF switch(...)


}//END OF HandleEventQueueEvent(...)


void CAGrunt::setChaseSpeed(void){
	if(this->m_fIsPoweredUp){
		//MODDD TODO - even higher for harder difficulties?
		if(g_iSkillLevel < SKILL_HARD){
			m_flFramerateSuggestion = 1.16;
		}else{
			m_flFramerateSuggestion = 1.28;
		}
		
	}else{
		m_flFramerateSuggestion = 1.0;
	}

	//apply immediately just in case.
	pev->framerate = m_flFramerateSuggestion;
}//END OF setChaseSpeed

GENERATE_TAKEDAMAGE_IMPLEMENTATION(CAGrunt)
{
	if(m_fIsPoweredUp){
		if(::g_iSkillLevel < SKILL_HARD){
			flDamage *= 0.94; //slight reduction.
		}else{
			flDamage *= 0.83;
		}
	}

	if(powerupCauseEntDirectedEnemy != NULL && m_hEnemy != NULL && m_hEnemy.Get() == powerupCauseEntDirectedEnemy.Get() ){
		//nothing in particular needed? Can just pick a new enemy.
	}

	if(pevAttacker != NULL){
		CBaseEntity* testEnt = CBaseEntity::Instance(pevAttacker);
		if(testEnt != NULL && testEnt != powerupCauseEntDirectedEnemy){
			//that is, if I take damage from something that isn't the directed enemy, drop the directed enemy.
			powerupCauseEntDirectedEnemy = NULL;
		}
	}

	//easyForcePrintLine("AGrunt::TOOK DAMAGE. Health:%.2f Damage:%.2f Blast:%d Gib::N:%d A:%d", pev->health, flDamage, (bitsDamageType & DMG_BLAST), (bitsDamageType & DMG_NEVERGIB), (bitsDamageType & DMG_ALWAYSGIB) );

	return GENERATE_TAKEDAMAGE_PARENT_CALL(CSquadMonster);
}


void CAGrunt::forceNewEnemy(CBaseEntity* argIssuing, CBaseEntity* argNewEnemy, BOOL argPassive){
	//easyForcePrintLine("AGRUNT ID%d: forceNewEnemy: issuing:%s newenemy:%s passive:%d", monsterID, FClassname(argIssuing), FClassname(argNewEnemy), argPassive);

	if(!m_fIsPoweredUp){
		//If not powered up, I ignore this call.
		return;
	}

	//also if this monster is prone, it makes no sense to do schedle changes.
	if(m_IdealMonsterState == MONSTERSTATE_PRONE || forceNewEnemyCooldownTightTime != -1 && gpGlobals->time <= forceNewEnemyCooldownTightTime){
		return;
	}

	forgetPowerupCauseEntDirectedEnemyTime = gpGlobals->time + 20;

	directedEnemyIssuer = argIssuing;
	powerupCauseEntDirectedEnemy = argNewEnemy;

	
	if(argPassive && (forceNewEnemyCooldownTime != -1 && gpGlobals->time <= forceNewEnemyCooldownTime) ){
		//still in the cooldown from the last time a new enemy was forced? Deny.
		return;
	}

	if(m_hEnemy != argNewEnemy){
		//Another chance to deny.  If passive and my existing commanding kingpin is closer than the other one giving the order, deny this order.
		if(argIssuing != NULL && poweredUpCauseEnt != NULL && argIssuing->edict() != poweredUpCauseEnt.Get() ){  //&& argIssuing->pev == &poweredUpCauseEnt.Get()->v){

			//Are we closer to the old one or the new one? If closer to the new one, accept this new enemy. Otherwise deny this order.
			float distToOldCause = (poweredUpCauseEnt->pev->origin - this->pev->origin).Length();
			float distToNewCause = (argIssuing->pev->origin - this->pev->origin).Length();

			if(distToOldCause < distToNewCause){
				//closer to the old one? Denied.
				forceNewEnemyCooldownTightTime = gpGlobals->time + 0.5;
				forceNewEnemyCooldownTime = gpGlobals->time + 2;
				return;
			}
		}

		//new enemy. Reset the cooldown.
		forceNewEnemyCooldownTightTime = gpGlobals->time + 1;
		forceNewEnemyCooldownTime = gpGlobals->time + 10;

		m_hEnemy = argNewEnemy;

		//If non-passive (immediate) and if in the middle of attacking and this was the incorrect enemy, stop attacking. Is this safe?
		if(!argPassive){

			/*
			if((this->m_pSchedule == slRangeAttack1 || this->m_pSchedule == slRangeAttack2 || this->m_pSchedule == slPrimaryMeleeAttack ||  this->m_pSchedule == slSecondaryMeleeAttack) ){
				//this should be similar to "TaskFail()", but even faster? Careful, avoids any built-in fail task that may have had a point.
				ChangeSchedule( GetSchedule() );
			}else if( !(this->m_pSchedule == slChaseEnemy || this->m_pSchedule == slChaseEnemySmart && this->m_movementActivity == ACT_RUN && this->m_IdealActivity == this->m_movementActivity) ){
				//Not the correct enemy, chase them.
				ChangeSchedule( GetSchedule() );
			}
			*/
			
			SetState(MONSTERSTATE_COMBAT);

			//no matter what, just changeschedule.
			//MODDD - fuck donuts
			//ChangeSchedule( GetSchedule() );
			//easyForcePrintLine("FUCK DONUTS 1");
			TaskFail();

		}//END OF if(!argPassive)
	}//END OF my enemy - forced enemy mismatch check
	else{
		//if they DO match, and non-passive...
		forceNewEnemyCooldownTightTime = gpGlobals->time + 2.5;

		if(!argPassive){
			//little more strict here.
			if( !(this->m_pSchedule == slRangeAttack1 || this->m_pSchedule == slRangeAttack2 || this->m_pSchedule == slPrimaryMeleeAttack ||  this->m_pSchedule == slSecondaryMeleeAttack) &&
				!( (this->m_pSchedule == slChaseEnemy || this->m_pSchedule == slChaseEnemySmart) && (this->m_movementActivity == ACT_RUN && this->m_IdealActivity == this->m_movementActivity) )
			){
				SetState(MONSTERSTATE_COMBAT);

				//You MUST be focused on the enemy right this moment!
				//easyForcePrintLine("FUCK DONUTS 2");
				//ChangeSchedule( GetSchedule() );
				TaskFail();
			}
		}

	}//END OF my enemy - forced enemy match check

}//END OF forceNewEnemy


void CAGrunt::forgetForcedEnemy(CBaseMonster* argIssuing, BOOL argPassive){
	if(this->m_fIsPoweredUp && argIssuing != NULL && directedEnemyIssuer != NULL && argIssuing->edict() == directedEnemyIssuer.Get() ){
		//If the one who issued this enemy in the first place is telling us to forget the directed enemy, obey only then.
		directedEnemyIssuer = NULL;
		powerupCauseEntDirectedEnemy = NULL;
	}
}//END OF forgetNewEnemy



void CAGrunt::ReportAIState(void){
	//call the parent, and add on to that.
	CBaseMonster::ReportAIState();

	easyForcePrintLine("CUSTOM REPORTAI: isPU:%d nextPUPartt:%.2f PUtEnd:%.2f PUCauseEnt:%s PUEntChangeCD:%.2f PUentDirEn:%s fneCDT:%.2f fneCD:%.2f direniss:%s fpcedet:%.2f curtime:%.2f",
		m_fIsPoweredUp,
		nextPoweredUpParticleTime,
		poweredUpTimeEnd,
		FClassname(poweredUpCauseEnt),
		poweredUpCauseEntChangeCooldown,
		FClassname(powerupCauseEntDirectedEnemy),
	
		forceNewEnemyCooldownTightTime,
		forceNewEnemyCooldownTime,
	
		FClassname(directedEnemyIssuer),
		forgetPowerupCauseEntDirectedEnemyTime,

		gpGlobals->time
	);
	
}//END OF ReportAIState()


Vector CAGrunt::GetGunPosition(void){
	//maybe this would be better?
	Vector vecGunPos;
	Vector vecGunAngles;
	GetAttachment( 0, vecGunPos, vecGunAngles );
	//::UTIL_printLineVector("yehhhag", vecGunPos-pev->origin);

	return vecGunPos;
}//END OF GetGunPosition


Vector CAGrunt::GetGunPositionAI(void){
	////Clone of GetGunPosition from monsters.cpp. The GetGunPositionAI method of CBaseMonster would have called Monster's GetGunPosition, but we've made ours more specific.
	//return CBaseMonster::GetGunPosition();
	////CHANGED.  Just using the hacked position to determine this.
	return CBaseMonster::GetGunPositionAI();
	//return EyePosition();

	/*
	Vector v_forward, v_right, v_up, angle;


	//TEST: if I WERE facing the enemy right now...
	if(m_hEnemy != NULL){
		angle = ::UTIL_VecToAngles(m_hEnemy->pev->origin - pev->origin);
	}else{
		angle = pev->angles;
	}
	
	angle.x = 0; //pitch is not a factor here.
	UTIL_MakeVectorsPrivate( angle, v_forward, v_right, v_up);
	
	const Vector vecSrc = pev->origin 
					+ v_forward * m_HackedGunPos.y 
					+ v_right * m_HackedGunPos.x 
					+ v_up * m_HackedGunPos.z;

	return vecSrc;
	*/

}//END OF GetGunPositionAI



//Copy, takes more to make this guy react.
void CAGrunt::OnTakeDamageSetConditions(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType, int bitsDamageTypeMod){

	//MODDD - intervention. Timed damage might not affect the AI since it could get needlessly distracting.

	if(bitsDamageTypeMod & (DMG_TIMEDEFFECT|DMG_TIMEDEFFECTIGNORE) ){
		//If this is continual timed damage, don't register as any damage condition. Not worth possibly interrupting the AI.
		return;
	}

	//default case from CBaseMonster's TakeDamage.
	//Also count being in a non-combat state to force looking in that direction.
	//if ( flDamage > 0 )
	if(m_MonsterState == MONSTERSTATE_IDLE || m_MonsterState == MONSTERSTATE_ALERT || flDamage >= 15)
	{
		SetConditions(bits_COND_LIGHT_DAMAGE);
		forgetSmallFlinchTime = gpGlobals->time + DEFAULT_FORGET_SMALL_FLINCH_TIME;
	}

	//MODDD - HEAVY_DAMAGE was unused before.  For using the BIG_FLINCH activity that is (never got communicated)
	//    Stricter requirement:  this attack took 70% of health away.
	//    The agrunt used to use this so that its only flinch was for heavy damage (above 20 in one attack), but that's easy by overriding this OnTakeDamageSetconditions method now.
	//    Keep it to using light damage for that instead.
	//if ( flDamage >= 20 )
	

	//Damage above 40 also causes bigflinch for tougher creatures like agrunts,
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

	easyPrintLine("%s:%d OnTkDmgSetCond raw:%.2f fract:%.2f", getClassname(), monsterID, flDamage, (flDamage / pev->max_health));

}//END OF OnTakeDamageSetConditions


int CAGrunt::getHullIndexForNodes(void){
    return NODE_LARGE_HULL;  //safe?
}

BOOL CAGrunt::predictRangeAttackEnd(void) {
	return TRUE;
}

