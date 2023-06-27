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
#include "barney.h"
#include "util.h"
#include "cbase.h"
#include "basemonster.h"
#include "talkmonster.h"
#include "schedule.h"
#include "defaultai.h"
#include "scripted.h"
#include "weapons.h"
#include "soundent.h"
#include "glock.h"
#include "util_model.h"
#include "defaultai.h"
#include "util_debugdraw.h"

EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST(sv_germancensorship)
extern BOOL globalPSEUDO_germanModel_barneyFound;
extern BOOL globalPSEUDO_iCanHazMemez;
EASY_CVAR_EXTERN_DEBUGONLY(barneyDummy)
EASY_CVAR_EXTERN_DEBUGONLY(monsterSpawnPrintout)
EASY_CVAR_EXTERN(pissedNPCs)
EASY_CVAR_EXTERN_DEBUGONLY(barneyPrintouts)
EASY_CVAR_EXTERN_DEBUGONLY(glockOldReloadLogicBarney)
EASY_CVAR_EXTERN_DEBUGONLY(barneyDroppedGlockAmmoCap)
EASY_CVAR_EXTERN_DEBUGONLY(barneyUnholsterTime)
EASY_CVAR_EXTERN_DEBUGONLY(barneyUnholsterAnimChoice)
EASY_CVAR_EXTERN_DEBUGONLY(hyperBarney)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(thatWasntPunch)



//MODDD - how many shots before reloading?
#define BARNEY_WEAPON_CLIP_SIZE 13


//MODDD - macro.
#define HITGROUP_BARNEY_HELMET 10

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
// first flag is barney dying for scripted sequences?
#define BARNEY_AE_DRAW		( 2 )
#define BARNEY_AE_SHOOT		( 3 )
#define BARNEY_AE_HOLSTER	( 4 )
//MODDD 
#define BARNEY_AE_RELOAD	( 5 )

// oh, already had these constants?  ah well, doesn't hurt to be formal and say it's part of bodygroup #1.
#define BODYGROUP_GUN 1
#define BARNEY_BODY_GUNHOLSTERED 0
#define BARNEY_BODY_GUNDRAWN 1
#define BARNEY_BODY_GUNGONE 2


//Yes, need to know this ahead of time.
extern Schedule_t slBarneyEnemyDraw[];


float g_sayBulletHitCooldown = -1;

SimpleMonsterSaveState g_duringSpamRampageState;






//MODDD - why was this section lacking?
enum
{
	SCHED_BARNEY_RELOAD = LAST_TALKMONSTER_SCHEDULE + 1,
	//MODDD - new!
	SCHED_BARNEY_DISARM_WEAPON,
};

enum
{
	TASK_RESTORE_SPAM = LAST_TALKMONSTER_TASK + 1,
	
};


//MODDD - class moved to barney.h

//MODDD - "implementation".  yea, static vars have a prototype and implementation.
float CBarney::g_barneyAlertTalkWaitTime = 0;


#if REMOVE_ORIGINAL_NAMES != 1
	LINK_ENTITY_TO_CLASS( monster_barney, CBarney );
#endif

#if EXTRA_NAMES > 0
	LINK_ENTITY_TO_CLASS( barney, CBarney );
	
	//no extras.
#endif





const char* CBarney::madInterSentences[] = {
	"!BA_POKE0",
	"!BA_POKE1",
	"!BA_POKE2",
	"!BA_POKEQ0",
	"!BA_POKE4",
	"!BA_POKEQ1",
	"!BA_POKEQ2",
	"!BA_POKE7",
	"!BA_POKEQ3",
	"!BA_POKE9",
	"!BA_POKE10",
	"!BA_POKE11",
	"!BA_POKE12",
	"!BA_POKE13",
	//"!BA_POKEQ4",
	"!BA_POKE15",
	"!BA_POKE16",
	"!BA_POKE17",
	"!BA_POKE18",
	//"!BA_POKEQ5",
	"!BA_POKE20",
	"!BA_POKE21",
	"!BA_POKE22",
	"!BA_POKE23",
	"!BA_POKE24",
	"!BA_POKE25",
	"!BA_POKE26",
	"!BA_POKE27",
	"!BA_POKE28",
	"!BA_POKE29",
	"!BA_POKE30",
	"!BA_POKE31",
	"!BA_POKE32",
	"!BA_POKE33",
	"!BA_POKE34",
	"!BA_POKE35",
	"!BA_POKE36",
	"!BA_POKE37",
	"!BA_POKE38",
	"!BA_POKE39",
	"!BA_POKE40"
};
//int CBarney::madInterSentencesMax = 41;



int CBarney::getMadInterSentencesMax(void){
	if(globalPSEUDO_iCanHazMemez == TRUE){
		return 39;
	}else{
		return 19;
	}
}
int CBarney::getMadSentencesMax(void){
	//easyForcePrintLine("YOU wonderful little sunflower you %d", (globalPSEUDO_iCanHazMemez == TRUE));
	if(globalPSEUDO_iCanHazMemez == TRUE){
		return 41;
	}else{
		return 21;
	}
}




TYPEDESCRIPTION	CBarney::m_SaveData[] = 
{
	DEFINE_FIELD( CBarney, m_fGunDrawn, FIELD_BOOLEAN ),
	DEFINE_FIELD( CBarney, m_painTime, FIELD_TIME ),
	DEFINE_FIELD( CBarney, m_checkAttackTime, FIELD_TIME ),
	DEFINE_FIELD( CBarney, m_lastAttackCheck, FIELD_BOOLEAN ),
	
};

//IMPLEMENT_SAVERESTORE( CBarney, CTalkMonster );



int CBarney::Save(CSave& save)
{

	if (forgiveMeForWhatIMustDo) {
		g_duringSpamRampageState.Save(this);
		// safe stats before all that madness.
		beforeSpamRampageState.Restore(this);

		pev->frame = 0;
		pev->framerate = 1;
		m_flFramerateSuggestion = 1;

		ResetSequenceInfo();
		SetYawSpeed();
	}

	
	//if (!CTalkMonster::Save(save)) {
	//	return 0;
	//}
	BOOL baseClassSaveSuccess = CTalkMonster::Save(save);

	int iWriteFieldsResult;
	if (baseClassSaveSuccess) {
		iWriteFieldsResult = save.WriteFields("CBarney", this, m_SaveData, ARRAYSIZE(m_SaveData));
	}
	else {
		// ???
		iWriteFieldsResult = 0;
	}


	if (forgiveMeForWhatIMustDo) {
		// back in action!
		g_duringSpamRampageState.Restore(this);

		pev->frame = 0;
		pev->framerate = 1;
		m_flFramerateSuggestion = 1;

		ResetSequenceInfo();
		SetYawSpeed();
	}


	return iWriteFieldsResult;
}



int CBarney::Restore(CRestore& restore)
{
	if (!CTalkMonster::Restore(restore)) {
		return 0;
	}
	int iReadFieldsResult = restore.ReadFields("CBarney", this, m_SaveData, ARRAYSIZE(m_SaveData));

	return iReadFieldsResult;
}




//=========================================================
// AI Schedules Specific to this monster
//=========================================================




//MODDD
Task_t	tlBaReload[] =
{
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_SET_FAIL_SCHEDULE,		(float)SCHED_RELOAD			},
	{ TASK_PLAY_SEQUENCE,			(float)ACT_RELOAD			},
};

Schedule_t slBaReload[] = 
{
	{
		tlBaReload,
		ARRAYSIZE ( tlBaReload ),
		bits_COND_HEAVY_DAMAGE,
		//bits_COND_HEAR_SOUND,

		bits_SOUND_DANGER,
		"BarneyReload"
	}
};



//MODDD TODO - why do NPCs behave so oddly on trying to reach a dead, especially gibbed, player's location?
Task_t	tlBaFollow[] =
{
	//MODDD - the Barney should use his BA_STOP lines.
	{ TASK_SET_FAIL_SCHEDULE,	(float)SCHED_CANT_FOLLOW },	// If you fail, bail out of follow
	{ TASK_MOVE_TO_TARGET_RANGE,(float)128		},	// Move within 128 of target ent (client)
	{ TASK_FOLLOW_SUCCESSFUL, (float)0		},
	//MODDD - scientists dummied out this call as of retail, so shouldn't barnies too?  No need for these schedules to call each other back-and-forth.
	//{ TASK_SET_SCHEDULE,		(float)SCHED_TARGET_FACE },
};

Schedule_t	slBaFollow[] =
{
	{
		tlBaFollow,
		ARRAYSIZE ( tlBaFollow ),
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_HEAR_SOUND |
		bits_COND_PROVOKED,
		bits_SOUND_DANGER,
		"BaFollow"
	},
};

//=========================================================
// BarneyDraw- much better looking draw schedule for when
// barney knows who he's gonna attack.
//=========================================================
Task_t	tlBarneyEnemyDraw[] =
{
	{ TASK_STOP_MOVING,					0				},
	{ TASK_FACE_ENEMY,					0				},
	{ TASK_PLAY_SEQUENCE_FACE_ENEMY,	(float) ACT_ARM },
};

Schedule_t slBarneyEnemyDraw[] = 
{
	{
		tlBarneyEnemyDraw,
		ARRAYSIZE ( tlBarneyEnemyDraw ),
		0,
		0,
		"Barney Enemy Draw"
	}
};


//MODDD - new!
Task_t	tlBarneyUnDraw[] =
{
	{ TASK_STOP_MOVING,					0				},
	{ TASK_PLAY_SEQUENCE,	(float) ACT_DISARM },
};

Schedule_t slBarneyUnDraw[] = 
{
	{
		tlBarneyUnDraw,
		ARRAYSIZE ( tlBarneyUnDraw ),
		0,
		0,
		"Barney Enemy UnDraw"
	}
};


Task_t	tlBarneyUnDrawForgiveSpam[] =
{
	{ TASK_STOP_MOVING, 0 },
	{ TASK_PLAY_SEQUENCE, (float)ACT_DISARM },
	{ TASK_RESTORE_SPAM, 0 },
};

Schedule_t slBarneyUnDrawForgiveSpam[] =
{
	{
		tlBarneyUnDrawForgiveSpam,
		ARRAYSIZE(tlBarneyUnDrawForgiveSpam),
		0,
		0,
		"Barney Enemy UnDraw"
	}
};


Task_t	tlBaFaceTarget[] =
{
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_FACE_TARGET,			(float)0		},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_SET_SCHEDULE,		(float)SCHED_TARGET_CHASE },
};

Schedule_t	slBaFaceTarget[] =
{
	{
		tlBaFaceTarget,
		ARRAYSIZE ( tlBaFaceTarget ),
		bits_COND_CLIENT_PUSH	|
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_HEAR_SOUND |
		bits_COND_PROVOKED,
		bits_SOUND_DANGER,
		"FaceTarget"
	},
};


Task_t	tlIdleBaStand[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_WAIT,				(float)2		}, // repick IDLESTAND every two seconds.
	{ TASK_TLK_HEADRESET,		(float)0		}, // reset head position
};

Schedule_t	slIdleBaStand[] =
{
	{ 
		tlIdleBaStand,
		ARRAYSIZE ( tlIdleBaStand ), 
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_HEAR_SOUND	|
		bits_COND_SMELL			|
		bits_COND_PROVOKED,

		bits_SOUND_COMBAT		|// sound flags - change these, and you'll break the talking code.
		//bits_SOUND_PLAYER		|
		//bits_SOUND_WORLD		|
		
		bits_SOUND_DANGER		|
		bits_SOUND_MEAT			|// scents
		bits_SOUND_CARCASS		|
		bits_SOUND_GARBAGE,
		"IdleStand"
	},
};

DEFINE_CUSTOM_SCHEDULES( CBarney )
{
	slBaFollow,
	slBarneyEnemyDraw,
	//MODDD - new
	slBarneyUnDraw,
	slBaFaceTarget,
	slIdleBaStand,
	//MODDD - new.
	slBaReload,
};


IMPLEMENT_CUSTOM_SCHEDULES( CBarney, CTalkMonster );






void CBarney::DeclineFollowingProvoked(CBaseEntity* pCaller) {
	// no extra action needed
}


void CBarney::SayAlert(void) {
	//easyForcePrintLine("BARNEYS ALERT RAN!");

	//MODDD - this doesn't appear to work.  It expects a single sentence named exactly "BA_ATTACK", I think, not, say, picking "BA_ATTACK0", "1", "2", ...
	//PlaySentence( "BA_ATTACK", RANDOM_FLOAT(2.8, 3.2), VOL_NORM, ATTN_IDLE );

	if (EASY_CVAR_GET(pissedNPCs) != 1 || !globalPSEUDO_iCanHazMemez) {

		// MODDD - little intervention.
		// Some lines for some foes don't make sense like 
		// "Aim for the head if you can find it" against robotic or human foes.
		int enemyClassify;
		
		if (m_hEnemy != NULL) {
			enemyClassify = m_hEnemy->Classify();
		}
		else if (recentDeclinesForgetTime != -1 && recentDeclines >= 30) {
			// assume it's the player.
			enemyClassify = CLASS_PLAYER;
		}
		else {
			// what.
			enemyClassify = CLASS_NONE;
		}


		long randoRange;


		if (RANDOM_FLOAT(0, 1) < 0.09) {
			// angry barney
			switch (RANDOM_LONG(0, 2)) {
			case 0:PlaySentenceSingular("BA_ATTACK6", 4, VOL_NORM, ATTN_NORM); break;
			case 1:PlaySentenceSingular("BA_ATTACK7", 4, VOL_NORM, ATTN_NORM); break;
			case 2:PlaySentenceSingular("BA_ATTACK8", 4, VOL_NORM, ATTN_NORM); break;
			}
		}
		else if (enemyClassify == CLASS_NONE) {
			// ???
			PlaySentence("BA_ATTACK", RANDOM_FLOAT(2.8, 3.2), VOL_NORM, ATTN_IDLE);
		}
		else if (enemyClassify == CLASS_PLAYER) {
			// if the enemy is the player, any line can be said except the
			// "open fire, Gordon!" one.  That doesn't make a whole lot of sense now.

			switch (RANDOM_LONG(0, 3)) {
			case 0:PlaySentence("BA_PISSED", 3, VOL_NORM, ATTN_NORM); break;
			case 1:PlaySentenceSingular("BA_ATTACK1", 4, VOL_NORM, ATTN_NORM); break;
			case 2:PlaySentenceSingular("BA_ATTACK5", 4, VOL_NORM, ATTN_NORM); break;
			//case 3:PlaySentenceSingular("BA_ATTACK2", 4, VOL_NORM, ATTN_NORM); break;   // 'stand back'?  To who?
			case 3: {
				// another shot at these instead.  Hear my rage.
				switch (RANDOM_LONG(0, 2)) {
				case 0:PlaySentenceSingular("BA_ATTACK6", 4, VOL_NORM, ATTN_NORM); break;
				case 1:PlaySentenceSingular("BA_ATTACK7", 4, VOL_NORM, ATTN_NORM); break;
				case 2:PlaySentenceSingular("BA_ATTACK8", 4, VOL_NORM, ATTN_NORM); break;
				}
			}break;
			}//switch
		}
		else if (
			enemyClassify == CLASS_ALIEN_MILITARY ||
			enemyClassify == CLASS_ALIEN_PASSIVE ||
			enemyClassify == CLASS_ALIEN_MONSTER
			) {
			//MODDD - you can say any line (Retail behavior).
			//SENTENCEG_PlayRndSz(ENT(pev), "BA_ATTACK", VOL_NORM, ATTN_NORM, 0, m_voicePitch);
			// ...nevermind.  Want to exclude the "open fire gordon" if we hate the player.
			// And "Freeze!" doesn't make sense for animal-like aliens.
			// Although this section will be the less animal-like ones, so it works.

			randoRange = 6;
			if (HasMemory(bits_COND_PROVOKED)) {
				// We dare not speak his name of we're pissed off at him.
				randoRange -= 1;
			}

			switch (RANDOM_LONG(0, randoRange)) {
			case 0:PlaySentenceSingular("BA_ATTACK1", 4, VOL_NORM, ATTN_NORM); break;
			case 1:PlaySentenceSingular("BA_ATTACK2", 4, VOL_NORM, ATTN_NORM); break;
			case 2:PlaySentenceSingular("BA_IDLE0", 4, VOL_NORM, ATTN_NORM); break;
			case 3:PlaySentenceSingular("BA_ATTACK3", 4, VOL_NORM, ATTN_NORM); break;
			case 4:PlaySentenceSingular("BA_ATTACK4", 4, VOL_NORM, ATTN_NORM); break;
			case 5:PlaySentenceSingular("BA_ATTACK5", 4, VOL_NORM, ATTN_NORM); break;
			case 6:PlaySentenceSingular("BA_ATTACK0", 4, VOL_NORM, ATTN_NORM); break;
			}

		}
		else if (
			enemyClassify == CLASS_ALIEN_PREY ||
			enemyClassify == CLASS_ALIEN_PREDATOR ||
			enemyClassify == CLASS_BARNACLE ||
			enemyClassify == CLASS_ALIEN_BIOWEAPON ||
			enemyClassify == CLASS_PLAYER_BIOWEAPON
			) {

			randoRange = 5;
			if (HasMemory(bits_COND_PROVOKED)) {
				// We dare not speak his name of we're pissed off at him.
				randoRange -= 1;
			}

			switch (RANDOM_LONG(0, randoRange)) {
			case 0:PlaySentenceSingular("BA_ATTACK1", 4, VOL_NORM, ATTN_NORM); break;
			case 1:PlaySentenceSingular("BA_ATTACK2", 4, VOL_NORM, ATTN_NORM); break;
			case 2:PlaySentenceSingular("BA_IDLE0", 4, VOL_NORM, ATTN_NORM); break;
			case 3:PlaySentenceSingular("BA_ATTACK3", 4, VOL_NORM, ATTN_NORM); break;
			case 4:PlaySentenceSingular("BA_ATTACK4", 4, VOL_NORM, ATTN_NORM); break;
			case 5:PlaySentenceSingular("BA_ATTACK0", 4, VOL_NORM, ATTN_NORM); break;
			}

		}
		else {
			// human or robotic?

			randoRange = 3;
			if (HasMemory(bits_COND_PROVOKED)) {
				// We dare not speak his name of we're pissed off at him.
				randoRange -= 1;
			}

			switch (RANDOM_LONG(0, randoRange)) {
			case 0:PlaySentenceSingular("BA_ATTACK1", 4, VOL_NORM, ATTN_NORM); break;
			case 1:PlaySentenceSingular("BA_ATTACK2", 4, VOL_NORM, ATTN_NORM); break;
			case 2:PlaySentenceSingular("BA_ATTACK5", 4, VOL_NORM, ATTN_NORM); break;
			case 3:PlaySentenceSingular("BA_ATTACK0", 4, VOL_NORM, ATTN_NORM); break;
			}
		}
	}
	else {
		SENTENCEG_PlayRndSz(ENT(pev), "BA_POKE_A", VOL_NORM, ATTN_NORM, 0, m_voicePitch);
	}

	// minimum delay
	CBarney::g_barneyAlertTalkWaitTime = gpGlobals->time + 6;
	// Also make all NPCs wait a bit to let this "battle cry" finish.
	CTalkMonster::g_talkWaitTime = gpGlobals->time + 1.9f;
}//SayAlert

void CBarney::SayDeclineFollowing(void) {


	if (EASY_CVAR_GET(pissedNPCs) < 1) {

		if (recentDeclines < 30) {
			PlaySentence("BA_POK", 2, VOL_NORM, ATTN_NORM);
		}
		else if (recentDeclines == 35) {
			SayAlert();  //bypass some usual cooldowns?
		}
		else {
			// not 35, 30 to 34 or over 36?  Same.
			float randomVal = RANDOM_FLOAT(0, 1);
			if (randomVal < 0.05) {
				PlaySentenceSingular("BA_SCARED0", 2, VOL_NORM, ATTN_NORM);
			}
			else {
				SayProvoked();
			}
		}
	}
	else {
		playPissed();
	}
}//SayDeclineFollowing

void CBarney::SayDeclineFollowingProvoked(void) {
	if (EASY_CVAR_GET(pissedNPCs) != 1 || !globalPSEUDO_iCanHazMemez) {
		//PlaySentence("BA_PISSED", 3, VOL_NORM, ATTN_NORM);
		SayProvoked();
	}
	else {
		PlaySentence("BA_POKE_D", 8, VOL_NORM, ATTN_NORM);
	}
}



void CBarney::SayProvoked(void) {

	// Don't interrupt this angsty message at the player with an alert message.
	g_barneyAlertTalkWaitTime = gpGlobals->time + 6;

	if (EASY_CVAR_GET(pissedNPCs) != 1 || !globalPSEUDO_iCanHazMemez) {
		if (RANDOM_FLOAT(0, 1) < 0.23) {
			PlaySentence("BA_PISSED", 3, VOL_NORM, ATTN_NORM);
		}else {
			PlaySentence("BA_MAD", 4, VOL_NORM, ATTN_NORM);
		}
	}
	else {
		PlaySentence("BA_POKE_D", 8, VOL_NORM, ATTN_NORM);
	}
}

void CBarney::SayStopShooting(void) {

	CTalkMonster::SayStopShooting();
}

void CBarney::SaySuspicious(void) {
	if (EASY_CVAR_GET(pissedNPCs) != 1 || !globalPSEUDO_iCanHazMemez) {
		PlaySentence("BA_SHOT", 4, VOL_NORM, ATTN_NORM);
	}
	else {
		PlaySentence("BA_POKE_C", 6, VOL_NORM, ATTN_NORM);
	}
}
void CBarney::SayLeaderDied(void) {
	PainSound(TRUE); //force the pain sound
}

void CBarney::SayKneel(void) {
	// TODO - something like 'can't do it without you' or something
	PainSound(TRUE);
}

void CBarney::SayNearPassive(void) {

	switch (RANDOM_LONG(0, 4)) {
	case 0:PlaySentenceSingular("BA_QUESTION10", 4, VOL_NORM, ATTN_NORM);break;
	case 1:PlaySentenceSingular("BA_QUESTION12", 4, VOL_NORM, ATTN_NORM);break;
	case 2:PlaySentenceSingular("BA_QUESTION13", 4, VOL_NORM, ATTN_NORM);break;
	case 3:PlaySentenceSingular("BA_SMELL1", 4, VOL_NORM, ATTN_NORM);break;
	case 4:PlaySentenceSingular("BA_SMELL2", 4, VOL_NORM, ATTN_NORM);break;
	default:break;
	}//END OF switch

}


void CBarney::OnNearCautious(void) {

	if (m_fGunDrawn == FALSE && m_pSchedule != slBarneyEnemyDraw) {
		//Barney will have his gun out around potential hostiles. He's ready for anything.
		ChangeSchedule(slBarneyEnemyDraw);
	}

	unholsterTimer = gpGlobals->time + EASY_CVAR_GET_DEBUGONLY(barneyUnholsterTime);
}//END OF onNearCautious


void CBarney::SayNearCautious(void) {

	switch (RANDOM_LONG(0, 34)) {
	case 0:PlaySentenceSingular("BA_OK0", 4, VOL_NORM, ATTN_NORM);break;
	case 1:PlaySentenceSingular("BA_OK2", 4, VOL_NORM, ATTN_NORM);break;
	case 2:PlaySentenceSingular("BA_OK5", 4, VOL_NORM, ATTN_NORM);break;
	case 3:PlaySentenceSingular("BA_OK6", 4, VOL_NORM, ATTN_NORM);break;
	case 4:PlaySentenceSingular("BA_QUESTION0", 4, VOL_NORM, ATTN_NORM);break;
	case 5:PlaySentenceSingular("BA_QUESTION3", 4, VOL_NORM, ATTN_NORM);break;
	case 6:PlaySentenceSingular("BA_QUESTION4", 4, VOL_NORM, ATTN_NORM);break;
	case 7:PlaySentenceSingular("BA_QUESTION6", 4, VOL_NORM, ATTN_NORM);break;
	case 8:PlaySentenceSingular("BA_QUESTION7", 4, VOL_NORM, ATTN_NORM);break;
	case 9:PlaySentenceSingular("BA_QUESTION8", 4, VOL_NORM, ATTN_NORM);break;
	case 10:PlaySentenceSingular("BA_QUESTION9", 4, VOL_NORM, ATTN_NORM);break;
	case 11:PlaySentenceSingular("BA_QUESTION10", 4, VOL_NORM, ATTN_NORM);break;
	case 12:PlaySentenceSingular("BA_QUESTION11", 4, VOL_NORM, ATTN_NORM);break;
	case 13:PlaySentenceSingular("BA_QUESTION12", 4, VOL_NORM, ATTN_NORM);break;
	case 14:PlaySentenceSingular("BA_QUESTION13", 4, VOL_NORM, ATTN_NORM);break;
	case 15:PlaySentenceSingular("BA_QUESTION14", 4, VOL_NORM, ATTN_NORM);break;
	case 16:PlaySentenceSingular("BA_HELLO6", 4, VOL_NORM, ATTN_NORM);break;
	case 17:PlaySentenceSingular("BA_IDLE0", 4, VOL_NORM, ATTN_NORM);break;
	case 18:PlaySentenceSingular("BA_IDLE1", 4, VOL_NORM, ATTN_NORM);break;
	case 19:PlaySentenceSingular("BA_IDLE2", 4, VOL_NORM, ATTN_NORM);break;
	case 20:PlaySentenceSingular("BA_IDLE3", 4, VOL_NORM, ATTN_NORM);break;
	case 21:PlaySentenceSingular("BA_IDLE6", 4, VOL_NORM, ATTN_NORM);break;
	case 22:PlaySentenceSingular("BA_IDLE7", 4, VOL_NORM, ATTN_NORM);break;
	case 23:PlaySentenceSingular("BA_IDLE8", 4, VOL_NORM, ATTN_NORM);break;
	case 24:PlaySentenceSingular("BA_IDLE9", 4, VOL_NORM, ATTN_NORM);break;
	case 25:PlaySentenceSingular("BA_IDLE10", 4, VOL_NORM, ATTN_NORM);break;
	case 26:PlaySentenceSingular("BA_IDLE11", 4, VOL_NORM, ATTN_NORM); break;  //badarea
	case 27:PlaySentenceSingular("BA_ATTACK2", 4, VOL_NORM, ATTN_NORM);break;
	case 28:PlaySentenceSingular("BA_ATTACK4", 4, VOL_NORM, ATTN_NORM);break;
	case 29:PlaySentenceSingular("BA_HEAR0", 4, VOL_NORM, ATTN_NORM);break;
	case 30:PlaySentenceSingular("BA_HEAR1", 4, VOL_NORM, ATTN_NORM);break;
	case 31:PlaySentenceSingular("BA_HEAR2", 4, VOL_NORM, ATTN_NORM);break;
	case 32:PlaySentenceSingular("BA_HEAR3", 4, VOL_NORM, ATTN_NORM);break;
	case 33:PlaySentenceSingular("BA_STOP3", 4, VOL_NORM, ATTN_NORM);break;
	case 34:PlaySentenceSingular("BA_STOP4", 4, VOL_NORM, ATTN_NORM);break;
	default:break;
	}//END OF switch
}//END OF SayNearCautious





void CBarney::StartTask( Task_t *pTask )
{
	
	//MODDD
	///////////////////////////////////////////////////////////////////////
	///???
	//m_iTaskStatus = TASKSTATUS_RUNNING;
	switch ( pTask->iTask )
	{
		case TASK_PLAY_KNEEL_SEQUENCE:
			// TODO - no kneel animation present, this is kinda close I guess
			SetSequenceByName("intropush");
			kneelSoundDelay = gpGlobals->time + RANDOM_FLOAT(0.8, 1.5);
		break;
		case TASK_RELOAD:
			m_IdealActivity = ACT_RELOAD;
		break;
		case TASK_RESTORE_SPAM:
			// calling RestoreState changes the current schedule, task, and task status.
			// Don't call TaskComplete after this.
			CompleteRestoreState();
			forgiveMeForWhatIMustDo = FALSE;
			return;
		break;
	}
	///////////////////////////////////////////////////////////////////////
	CTalkMonster::StartTask( pTask );	
}


void CBarney::RunTask( Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	//MODDD - sadly this does nothing, it's up to the model to support pitch-adjustments
	// per sequence too, which the unholster anim just doesn't.  So this has no effect.
	/*
	case TASK_PLAY_SEQUENCE_FACE_ENEMY:
	case TASK_PLAY_SEQUENCE_FACE_TARGET:
	case TASK_FACE_ENEMY:
	case TASK_WAIT_FACE_ENEMY:

		// Set the pitch correctly every frame instead.  If I'm looking at the monster enough.
		if (m_hEnemy != NULL && UTIL_IsFacing(pev, m_hEnemy->pev->origin, 0.15)) {
			lookAtEnemy_pitch();
		}
		CTalkMonster::RunTask(pTask);
	break;
	*/

	case TASK_RANGE_ATTACK1:{
		if (m_hEnemy != NULL && (m_hEnemy->IsPlayer())){
			pev->framerate = 1.5;
		}
		
		//Set the pitch correctly every frame instead.
		lookAtEnemy_pitch();

		CTalkMonster::RunTask( pTask );
	break;}
	case TASK_RELOAD:{
		//the reload sequence does not appear to support this, ah well.
		//lookAtEnemy_pitch();

		CTalkMonster::RunTask(pTask);
	break;}

	case TASK_DIE:
		CTalkMonster::RunTask(pTask);
		//UTIL_drawBox(pev->origin + pev->mins, pev->origin + pev->maxs);
		//UTIL_drawBox(pev->origin + VEC_HUMAN_HULL_MIN, pev->origin + VEC_HUMAN_HULL_MAX);
		break;
	default:
		CTalkMonster::RunTask( pTask );
		break;
	}

}


//=========================================================
// ISoundMask - returns a bit mask indicating which types
// of sounds this monster regards. 
//=========================================================
int CBarney::ISoundMask ( void) 
{

	//MODDD - when allied, I don't react to player sounds.
	if(HasMemory(bits_COND_PROVOKED || recentDeclines >= 30)){
		// I will react to player sounds.
		return	bits_SOUND_WORLD |
			bits_SOUND_COMBAT |
			bits_SOUND_CARCASS |
			bits_SOUND_MEAT |
			bits_SOUND_GARBAGE |
			bits_SOUND_DANGER |
			bits_SOUND_PLAYER;
	}
	else {
		// friendly.
		return	bits_SOUND_WORLD |
			bits_SOUND_COMBAT |
			bits_SOUND_CARCASS |
			bits_SOUND_MEAT |
			bits_SOUND_GARBAGE |
			bits_SOUND_DANGER;
	}
	
}

//=========================================================
// Classify - indicates this monster's place in the 
// relationship table.
//=========================================================
int CBarney::Classify ( void )
{
	return	CLASS_PLAYER_ALLY;
}


BOOL CBarney::getGermanModelRequirement(void){
	return globalPSEUDO_germanModel_barneyFound;
}
const char* CBarney::getGermanModel(void){
	return "models/g_barney.mdl";
}
const char* CBarney::getNormalModel(void){
	return "models/barney.mdl";
}


int CBarney::IRelationship( CBaseEntity *pTarget )
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

	return CTalkMonster::IRelationship( pTarget );
}



//=========================================================
// AlertSound - barney says "Freeze!"
//=========================================================
void CBarney::AlertSound( void )
{
	if ( m_hEnemy != NULL )
	{
		// don't do the alert sound this way
		if (recentDeclines >= 30 && recentDeclines <= 34 && m_hEnemy->IsPlayer())return;

		//MODDD - probably had to enter combat to say this, so make that an exception this once.
		//if ( FOkToSpeak() )
		if ( FOkToSpeakAllowCombat(CBarney::g_barneyAlertTalkWaitTime) )
		{
			SayAlert();
		}
	}
}


//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CBarney::SetYawSpeed ( void )
{
	int ys = 0;
	//MODDD - why were the Barney's turn rates so crappy? They are now more in-tune with hgrunt rates.
	switch ( m_Activity )
	{
	case ACT_IDLE:
		ys = 80;
		break;
	case ACT_WALK:
		ys = 160;
		break;
	case ACT_RUN:
		ys = 140;
		break;
	case ACT_TURN_LEFT:
	case ACT_TURN_RIGHT:
		ys = 180;
		break;
	case ACT_RANGE_ATTACK1:
		ys = 140;
		break;
	default:
		ys = 90;
		break;
	}

	pev->yaw_speed = ys;
}


//=========================================================
// CheckRangeAttack1
//=========================================================
BOOL CBarney::CheckRangeAttack1 ( float flDot, float flDist )
{

	if (recentDeclinesForgetTime != -1 && recentDeclines < 35 && m_hEnemy != NULL && m_hEnemy->IsPlayer()) {
		// not yet.   Go on, try me.
		return FALSE;
	}


	if ( flDist <= 1024 && flDot >= 0.5 )
	{
		if ( gpGlobals->time > m_checkAttackTime )
		{
			TraceResult tr;
			
			Vector shootOrigin = GetGunPositionAI();
			CBaseEntity *pEnemy = m_hEnemy;
			Vector shootTarget = ( (pEnemy->BodyTarget( shootOrigin ) - pEnemy->pev->origin) + m_vecEnemyLKP );

			UTIL_TraceLine( shootOrigin, shootTarget, dont_ignore_monsters, ENT(pev), &tr );

			//DebugLine_Setup(0, shootOrigin, shootTarget, tr.flFraction);

			m_checkAttackTime = gpGlobals->time + 1;
			if ( tr.flFraction == 1.0 || (tr.pHit != NULL && CBaseEntity::Instance(tr.pHit) == pEnemy) )
				m_lastAttackCheck = TRUE;
			else
				m_lastAttackCheck = FALSE;
			m_checkAttackTime = gpGlobals->time + 1.5;
		}
		return m_lastAttackCheck;
	}
	return FALSE;
}

//=========================================================
// BarneyFirePistol - shoots one round from the pistol at
// the enemy barney is facing.
//=========================================================
void CBarney::BarneyFirePistol ( void )
{
	Vector vecShootOrigin;

	UTIL_MakeVectors(pev->angles);
	vecShootOrigin = GetGunPosition();


	Vector vecShootDir;
	if(m_hEnemy != NULL && FClassnameIs(m_hEnemy->pev, "monster_zombie")){
		// I'll aim for the head.  If I can find it.
		vecShootDir = ShootAtEnemyEyes(vecShootOrigin);
	}else{
		// eh, default
		vecShootDir = ShootAtEnemyMod(vecShootOrigin);
	}


	//MODDD - do this every frame with an enemy active instead.  Like TASK_RANGE_ATTACK1 ?
	//Vector angDir = UTIL_VecToAngles( vecShootDir );
	//SetBlending( 0, angDir.x );

	pev->effects = EF_MUZZLEFLASH;

	FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_2DEGREES, 1024, BULLET_MONSTER_9MM );
	

	// Only shift about half the time
	long canPitchShift = RANDOM_LONG(0, 1);
	int pitchShift;
	if (canPitchShift == 0) {
		//no pitch shift.
		pitchShift = 0;
	}else {
		pitchShift = RANDOM_LONG(-5, 5);
	}

	// This is a glock-firing sound effect.  Don't throw in voice pitch.
	UTIL_PlaySound( ENT(pev), CHAN_WEAPON, "barney/ba_attack2.wav", 1, ATTN_NORM, 0, 100 + pitchShift );

	CSoundEnt::InsertSound ( bits_SOUND_COMBAT, pev->origin, 384, 0.3 );

	// UNDONE: Reload?
	m_cAmmoLoaded--;// take away a bullet!
}

//MODDD - was "think", made "MonsterThink"...
void CBarney::MonsterThink(void){
	
	//easyForcePrintLine("IM SUPER %d : seq:%d fr:%.2f fin:%d", HasConditions( bits_COND_ENEMY_DEAD ), pev->sequence, pev->frame, this->m_fSequenceFinished);

	if(EASY_CVAR_GET_DEBUGONLY(barneyDummy)){
		//DUMMY BARNEY.
		pev->nextthink = gpGlobals->time + 0.1;
		return;
	}

	if (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(thatWasntPunch) == 1 && (this->m_fSequenceFinished || pev->frame >= 245)) {

		switch (RANDOM_LONG(0, 32)) {
		case 0:this->SetSequenceByName("idle4"); break;
		case 1:this->SetSequenceByName("turnleft"); break;
		case 2:this->SetSequenceByName("turnright"); break;
		case 3:this->SetSequenceByName("smlflinch"); break;
		case 4:this->SetSequenceByName("locked_door"); break;
		case 5:this->SetSequenceByName("fall_loop"); break;
		case 6:this->SetSequenceByName("barn_wave"); break;
		case 7:this->SetSequenceByName("beat_grunt"); break;
		case 8:this->SetSequenceByName("beat_gruntidle"); break;
		case 9:this->SetSequenceByName("flashlight"); break;
		case 10:this->SetSequenceByName("diesimple"); break;
		case 11:this->SetSequenceByName("barnaclehit"); break;
		case 12:this->SetSequenceByName("barnaclepull"); break;
		case 13:this->SetSequenceByName("barnaclecrunch"); break;
		case 14:this->SetSequenceByName("barnaclechew"); break;
		case 15:this->SetSequenceByName("barneyfallidle"); break;
		case 16:this->SetSequenceByName("standing_halt"); break;
		case 17:this->SetSequenceByName("c1a0_lean_idle"); break;
		case 18:this->SetSequenceByName("c1a0_lean"); break;
		case 19:this->SetSequenceByName("intropush"); break;
		case 20:this->SetSequenceByName("fence"); break;
		case 21:this->SetSequenceByName("almost"); break;
		case 22:this->SetSequenceByName("barney_fight"); break;
		case 23:this->SetSequenceByName("stuka_warn"); break;
		case 24:this->SetSequenceByName("train_assist"); break;
		case 25:this->SetSequenceByName("train_flung"); break;
		case 26:this->SetSequenceByName("pepsiswing"); break;
		case 27:this->SetSequenceByName("pepsipush"); break;
		case 28:this->SetSequenceByName("hambone"); break;

		case 29:this->SetSequenceByName("barn_wave"); break;
		case 30:this->SetSequenceByName("barn_wave"); break;
		case 31:this->SetSequenceByName("stuka_warn"); break;
		case 32:this->SetSequenceByName("stuka_warn"); break;
		}//END OF switch
	}



	/*
	// STOP!  Do this in the proper schedule.
	if( 
		//(m_pSchedule == slBaFollow || m_pSchedule == slBaFaceTarget) &&
		//(m_hTargetEnt == NULL || (m_hTargetEnt != NULL && !m_hTargetEnt->IsAlive()) )
		){
		//Fail if who we're supposed to follow dies.
		//m_hTargetEnt = NULL;
		leaderRecentlyDied = TRUE;
		TaskFail();
	}
	*/

	if(reloadSoundTime != -1 && gpGlobals->time >= reloadSoundTime){

		if(EASY_CVAR_GET_DEBUGONLY(glockOldReloadLogicBarney) == 0 || m_cAmmoLoaded == 0 ){
			m_cAmmoLoaded = BARNEY_WEAPON_CLIP_SIZE - 1;
		}else{
			m_cAmmoLoaded = BARNEY_WEAPON_CLIP_SIZE;
		}

		
		UTIL_PlaySound( ENT(pev), CHAN_WEAPON, "items/9mmclip1.wav", 1, ATTN_NORM, 0, 100, FALSE );
		reloadSoundTime = -1;
	}

	CTalkMonster::MonsterThink();

	if(EASY_CVAR_GET_DEBUGONLY(barneyPrintouts) == 1){
		if(m_hEnemy != NULL && m_hTargetEnt != NULL){
			easyForcePrintLine("I AM A BARNEY AND MY ENEMY IS %s TARG: %s", STRING(m_hEnemy->pev->classname), STRING(m_hTargetEnt->pev->classname) );
		}else if(m_hEnemy != NULL){
			easyForcePrintLine("I AM A BARNEY AND MY ENEMY IS %s", STRING(m_hEnemy->pev->classname) );
	
		}else if(m_hTargetEnt != NULL){
			easyForcePrintLine("I AM A BARNEY AND MY TARG IS: %s", STRING(m_hTargetEnt->pev->classname) );
	
		}else{
			easyForcePrintLine("I AM A BARNEY AND I SUCK");
	
		}
	}


}//END OF MonsterThink

//MODDD
void CBarney::SetActivity ( Activity NewActivity )
{
	int iSequence;


	//hm...
	//if ( NewActivity == m_Activity )
	//	return;


	if(NewActivity == ACT_RELOAD){

		if ( NewActivity == m_Activity ){
			//SAFETY: don't call this multiple times if it's been called already.
			return;
		}
		
		ClearConditions(bits_COND_NO_AMMO_LOADED);
		reloadSoundTime = gpGlobals->time + 0.72f;

	}else{
		//any other activities block the reload sound.
		reloadSoundTime = -1;

	}

	CTalkMonster::SetActivity(NewActivity);
}


//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//
// Returns number of events handled, 0 if none.
//=========================================================
void CBarney::HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch( pEvent->event )
	{
	case BARNEY_AE_SHOOT:
		BarneyFirePistol();
		break;

	case BARNEY_AE_DRAW:
		// barney's bodygroup switches here so he can pull gun from holster
		//MODDD - can undraw... if we're doing the negative framerate thing that is.
		//easyForcePrintLine("ARE YOU WEIRD %.2f, %.2f", pev->framerate, this->m_flFramerateSuggestion);
		if(pev->framerate < 0){
			//no, let the custom event handle this... this is way too iffy...
			//SetBodygroup(BODYGROUP_GUN, BARNEY_BODY_GUNHOLSTERED);
			//m_fGunDrawn = FALSE;
		}else{
			SetBodygroup(BODYGROUP_GUN, BARNEY_BODY_GUNDRAWN);
			m_fGunDrawn = TRUE;
		}
		break;

	case BARNEY_AE_HOLSTER:
		// change bodygroup to replace gun in holster
		SetBodygroup(BODYGROUP_GUN, BARNEY_BODY_GUNHOLSTERED);
		m_fGunDrawn = FALSE;
		break;

	//MODDD - unfortunately, this never seems to get called.  Just use "SetActivity"s intervention of "ACT_RELOAD" to do this.
	/*
	case BARNEY_AE_RELOAD:
		UTIL_PlaySound( ENT(pev), CHAN_WEAPON, "hgrunt/gr_reload1.wav", 1, ATTN_NORM );
		m_cAmmoLoaded = 4;
		ClearConditions(bits_COND_NO_AMMO_LOADED);

	break;
	*/

	default:
		CTalkMonster::HandleAnimEvent( pEvent );
	}
}

//MODDD - barney will need to reload every so often.
void CBarney::CheckAmmo ( void )
{
	if ( m_cAmmoLoaded <= 0 )
	{
		SetConditions(bits_COND_NO_AMMO_LOADED);
	}
}


CBarney::CBarney(void){

	canUnholster = FALSE;
	reloadSoundTime = -1;
	//reloadAmmoTime = -1;

	unholsterTimer = -1;
	recentDeadEnemyClass = CLASS_NONE;  //safe default?  (value: 0)

	madInterSentencesLocation = madInterSentences;

	forgiveMeForWhatIMustDo = FALSE;
	//madInterSentencesMaxLocation = &madInterSentencesMax;
}
//=========================================================
// Spawn
//=========================================================
void CBarney::Spawn()
{
	if(EASY_CVAR_GET_DEBUGONLY(monsterSpawnPrintout)){
		easyForcePrintLine("IM BARNEY, MY SPAWN FLAG BE : %d", pev->spawnflags);
	}

	Precache( );
	
	setModel(); //"models/barney.mdl"  argument unused when there's a german model equivalent.

	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);
	//MODDD - BARNEY TESt, AHH!
	//UTIL_SetSize(pev, Vector(-32, -32, -32), Vector(32, 32, 90));
	//UTIL_SetSize(pev, Vector(-64, -16, 0), Vector(64, 16, 50));


	pev->classname = MAKE_STRING("monster_barney");

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;   //test MOVETYPE_FLY   ?

	
	pev->movetype		= MOVETYPE_STEP;

	m_bloodColor = BloodColorRedFilter();
	pev->health			= gSkillData.barneyHealth;
	pev->view_ofs		= Vector ( 0, 0, 50 );// position of the eyes relative to monster's origin.
	m_flFieldOfView		= VIEW_FIELD_WIDE; // NOTE: we need a wide field of view so npc will notice player and say hello
	m_MonsterState		= MONSTERSTATE_NONE;

	pev->body			= 0; // gun in holster.  Good to start with a clean set at spawn, but the intent is below.
	//SetBodygroup(BODYGROUP_GUN, BARNEY_BODY_GUNHOLSTERED);

	m_fGunDrawn			= FALSE;



	if(EASY_CVAR_GET_DEBUGONLY(glockOldReloadLogicBarney) == 0 ){
		//not using old reload logic?  Barney just has 12 rounds.
		m_cAmmoLoaded = BARNEY_WEAPON_CLIP_SIZE - 1;
	}else{
		//Firing chamber has one.  Extra (13 total).
		m_cAmmoLoaded = BARNEY_WEAPON_CLIP_SIZE;
	}


	m_afCapability = bits_CAP_HEAR | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP;

	MonsterInit();

	SetUse( &CTalkMonster::FollowerUse );

	/**
	if(pev->spawnflags & 8){
		//for testing
		pev->health = pev->max_health * 0.35;
	}
	*/

	//MODDD new. This is a lot neater to use.
	m_HackedGunPos = Vector( 0, 0, 55 );



	//just a test..
	//pev->body = BARNEY_BODY_GUNGONE;
}

extern int global_useSentenceSave;
//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CBarney::Precache()
{
	PRECACHE_MODEL("models/barney.mdl");

	global_useSentenceSave = TRUE;

	//MODDD - what? why was ba_attack1 precached? This is just a clip of Barney saying "FREEZE", which is better for the sentence system with or without the soundsentencesave. Otherwise unused.
	//        ba_attack2 is his glock's firing sound so it should be precached to play like any other sound effect if necessary (not usuing soundsentencesave).
	//PRECACHE_SOUND("barney/ba_attack1.wav" );
	
	PRECACHE_SOUND("barney/ba_attack2.wav" );

	PRECACHE_SOUND("barney/ba_pain1.wav");
	PRECACHE_SOUND("barney/ba_pain2.wav");
	PRECACHE_SOUND("barney/ba_pain3.wav");

	PRECACHE_SOUND("barney/ba_die1.wav");
	PRECACHE_SOUND("barney/ba_die2.wav");
	PRECACHE_SOUND("barney/ba_die3.wav");

	//MODDD - added
	PRECACHE_SOUND("items/9mmclip1.wav", TRUE);  //part of the player sounds, don't use the soundsentencesave.
	global_useSentenceSave = FALSE;
	
	// every new barney must call this, otherwise
	// when a level is loaded, nobody will talk (time is reset to 0)
	TalkInit();
	CTalkMonster::Precache();
}	

// Init talk data
void CBarney::TalkInit()
{
	
	CTalkMonster::TalkInit();

	//MODDD - not inclusive max.
	madSentencesMax = 3;
	madSentences[0] = "BA_POKE";
	madSentences[1] = "BA_SHOT";
	madSentences[2] = "BA_MAD";

	
	//Each sentence here instead...
	//madInterSentences[0] = "...";


	//madInterSentencesMax = 1;
	//madInterSentences[0] = "BA_POKEQ";
	//madInterSentences[0] = "";

	// scientists speach group names (group names are in sentences.txt)

	m_szGrp[TLK_ANSWER]  =	"BA_ANSWER";
	m_szGrp[TLK_QUESTION] =	"BA_QUESTION";
	m_szGrp[TLK_IDLE] =		"BA_IDLE";
	m_szGrp[TLK_STARE] =	"BA_STARE";
	m_szGrp[TLK_USE] =		"BA_OK";
	m_szGrp[TLK_UNUSE] =	"BA_WAIT";
	m_szGrp[TLK_STOP] =		"BA_STOP";

	m_szGrp[TLK_NOSHOOT] =	"BA_SCARED";
	m_szGrp[TLK_HELLO] =	"BA_HELLO";

	m_szGrp[TLK_PLHURT1] =	"!BA_CUREA";
	m_szGrp[TLK_PLHURT2] =	"!BA_CUREB"; 
	m_szGrp[TLK_PLHURT3] =	"!BA_CUREC";

	m_szGrp[TLK_PHELLO] =	NULL;	//"BA_PHELLO";		// UNDONE
	m_szGrp[TLK_PIDLE] =	NULL;	//"BA_PIDLE";			// UNDONE

	//MODDD - for whatever reason, this was found not commented out like the above.  Unsure why.
	//        Can cause a console notice if it tries to use it though (missing sentence, something like that)
	//m_szGrp[TLK_PQUESTION] = "BA_PQUEST";		// UNDONE
	m_szGrp[TLK_PQUESTION] = NULL;


	m_szGrp[TLK_SMELL] =	"BA_SMELL";
	
	m_szGrp[TLK_WOUND] =	"BA_WOUND";
	m_szGrp[TLK_MORTAL] =	"BA_MORTAL";

	// get voice for head - just one barney voice for now
	m_voicePitch = 100;
}

//MODDD - "IsFacing" method was here. Now a broader utilty method (UTIL_IsFacing) better suited for checking whether a player or monster alike is facing a point at a customizable tolerance.


GENERATE_TAKEDAMAGE_IMPLEMENTATION(CBarney)
{
	//MODDD - all Barney damage is getting delegated to general TalkMonster's.
	return GENERATE_TAKEDAMAGE_PARENT_CALL(CTalkMonster);
}


void CBarney::OnAlerted(BOOL alerterWasKilled) {
	// change schedules this forcibly only the first time we get pissed.
	if (!HasMemory(bits_MEMORY_PROVOKED) || m_MonsterState == MONSTERSTATE_IDLE || m_MonsterState == MONSTERSTATE_ALERT) {
		if (!alerterWasKilled) {
			ChangeSchedule(slBarneyEnemyDraw);
		}
		else {
			ChangeSchedule(slBarneyEnemyDraw);
		}
	}
}



void CBarney::PainSound(void){
	PainSound(FALSE); //by default, typical. Obey the cooldown.
}
//=========================================================
// PainSound
//=========================================================
void CBarney::PainSound ( BOOL bypassCooldown )
{
	if (!bypassCooldown && gpGlobals->time < m_painTime)
		return;
	
	//MODDD - barney will have the same sounds for timed or nontimed damage, but just make noise less often.
	if (HasConditions(bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE)) {
		m_painTime = gpGlobals->time + RANDOM_FLOAT(1.8, 2.4);
	}
	else {
		m_painTime = gpGlobals->time + RANDOM_FLOAT(3, 6);
	}

	if (m_bitsDamageType & DMG_BULLET || m_bitsDamageTypeMod & DMG_PROJECTILE) {
		if (gpGlobals->time >= g_sayBulletHitCooldown && RANDOM_FLOAT(0, 1) <= 0.4) {
			PlaySentenceSingular("BA_WOUND_HOT", 3, VOL_NORM, ATTN_NORM);
			g_sayBulletHitCooldown = gpGlobals->time + 25;
		}

		m_bitsDamageType &= ~DMG_BULLET;
		m_bitsDamageTypeMod &= ~DMG_PROJECTILE;
		return;
	}


	switch (RANDOM_LONG(0,2))
	{
	case 0: UTIL_PlaySound( ENT(pev), CHAN_VOICE, "barney/ba_pain1.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
	case 1: UTIL_PlaySound( ENT(pev), CHAN_VOICE, "barney/ba_pain2.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
	case 2: UTIL_PlaySound( ENT(pev), CHAN_VOICE, "barney/ba_pain3.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
	}
}



//=========================================================
// DeathSound 
//=========================================================
void CBarney::DeathSound ( void )
{
	switch (RANDOM_LONG(0,2))
	{
	case 0: UTIL_PlaySound( ENT(pev), CHAN_VOICE, "barney/ba_die1.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
	case 1: UTIL_PlaySound( ENT(pev), CHAN_VOICE, "barney/ba_die2.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
	case 2: UTIL_PlaySound( ENT(pev), CHAN_VOICE, "barney/ba_die3.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
	}
}


//MODDD TODO - should explosion damage (DMG_BLAST) even pay attention to hitbox at all?
//             should there be a check for bitsDamageTypeMod's DMG_HITBOX_EQUAL to not do any changes to damage,
//             besides maybe reductions... oh these are all reducitons. Carry on, but a "headshot" treating blast
//             damage differently still looks weird, no way that precision makes sense.
GENERATE_TRACEATTACK_IMPLEMENTATION(CBarney)
{

	//MODDD - like agrunts and hgruntsn now, blast damage does even damage.  So block 10% unconditionally.
	if ((bitsDamageType & DMG_BLAST) || (bitsDamageTypeMod & DMG_HITBOX_EQUAL)) {
		// flat 50% reduction.  Kevlar.  Easy to assume any explosive hit is a body hit.
		// Barney's don't even have the 50 health to survive a near-direct hit.
		flDamage = flDamage * 0.50;
		if (ptr->iHitgroup == HITGROUP_BARNEY_HELMET) {
			UTIL_Ricochet(ptr->vecEndPos, RANDOM_FLOAT(2.1, 2.8));
			if (useBulletHitSound) { *useBulletHitSound = FALSE; }
			useBloodEffect = FALSE;
		}
	}
	else {
		// And now normal hitgroup checks.
		switch (ptr->iHitgroup)
		{
		case HITGROUP_CHEST:
		case HITGROUP_STOMACH: {
			//MODDD - CLUB damage included now too with melee.  Blast damage removed, only from being handled unconditionally above.
			// Also, damage-cuts changed.  Damage from bullets is cut in half if high (over 16), but even more otherwise (cut by 60%).
			// Melee damage from slash is cut by 50% like it was, CLUB by 40%.
			if (bitsDamageType & (DMG_BULLET))
			{
				if (flDamage > 12) {
					// high damage?  Cut in half.  Still bleed.
					flDamage = flDamage * 0.5;
				}
				else {
					// low damage?
					// cut by 60%
					flDamage = flDamage * 0.4;
					useBloodEffect = FALSE;
				}
			}
			else if (bitsDamageType & (DMG_SLASH) ) {
				flDamage = flDamage * 0.5;
			}
			else if (bitsDamageType & (DMG_CLUB) ) {
				flDamage = flDamage * 0.4;
			}
		}break;
		case HITGROUP_BARNEY_HELMET: {

			if (bitsDamageType & (DMG_BULLET)) {

				// absorb damage
				if (flDamage <= 20) {
					//flDamage = 0.01;
					flDamage = flDamage * 0.12;
					//MODDD - why not a little random-ness in ricochet flash size like the agrunt?
					// was a flat 1.
					UTIL_Ricochet(ptr->vecEndPos, RANDOM_FLOAT(1.0, 1.6));
					//MODDD - how about you too?  bullet ricochet effect from agrunt
					UTIL_RicochetTracer(ptr->vecEndPos, ptr->vecPlaneNormal);
					if (useBulletHitSound) { *useBulletHitSound = FALSE; }
					useBloodEffect = FALSE;
				}else {
					//MODDD - Hitting the armor still shouldn't be insignificant, reduce damage by 15%.
					flDamage *= 0.85;
				}
			}else if (bitsDamageType & (DMG_SLASH | DMG_CLUB)) {
				if (flDamage <= 20) {
					// reduce by 40% instead.
					flDamage = flDamage * 0.6;
					UTIL_Ricochet(ptr->vecEndPos, RANDOM_FLOAT(1.2, 1.8));

					if (useBulletHitSound) { *useBulletHitSound = FALSE; }
					useBloodEffect = FALSE;
				}else {
					//MODDD - Hitting the armor still shouldn't be insignificant, reduce damage by 10%.  If this even exists.
					flDamage *= 0.90;
				}
			}

			// it's head shot anyways
			ptr->iHitgroup = HITGROUP_HEAD;

		}break;

		//MODDD - anywhere else, cut bullet damage by 10%.  Still some protection there.
		// just be 'default' at this point
		//case HITGROUP_LEFTARM:
		//case HITGROUP_RIGHTARM:
		//case HITGROUP_LEFTLEG:
		//case HITGROUP_RIGHTLEG:
		default:
			if (bitsDamageType & (DMG_BULLET)) {
				flDamage = flDamage * 0.1;
			}
		break;

		}//END OF switch on hitbox
	}//END OF not DMG_BLAST or EQUAL_HITBOX check

	GENERATE_TRACEATTACK_PARENT_CALL(CTalkMonster);
}


// NOTE - GibMonster is called by basemonster KILLED, so this happesn before
// the monster decides to gib.  That is, no need for a separate check in a GibMonster
// method to see if we need to drop a wepaon.
// For others (hgrunts), the DropItem call is an an anim event, which is missed
// if GibMonster gets called, thus the need for a check in GibMonster.
GENERATE_KILLED_IMPLEMENTATION(CBarney)
{
	if ( pev->body < BARNEY_BODY_GUNGONE )
	{// drop the gun!
		Vector vecGunPos;
		Vector vecGunAngles;

		GetAttachment( 0, vecGunPos, vecGunAngles );
		
		CBaseEntity* pGun = DropItem( "weapon_9mmhandgun", vecGunPos, vecGunAngles );


		//MODDD - the dropped weapon will always give the default ammount of ammo (assumes it's been fired & lost the first shot in the firing chamber.  For the not old firing method, the assumption of not + 1 still works.).
		if(pGun != NULL){
			SetBodygroup(BODYGROUP_GUN, BARNEY_BODY_GUNGONE);
			
			CGlock* g = static_cast<CGlock*>(pGun);
			if(g != NULL){
				//use this if you have to.
				//CGlock::getUsingGlockOldReloadLogic()
				if(EASY_CVAR_GET_DEBUGONLY(barneyDroppedGlockAmmoCap) == -2){
					g->m_iDefaultAmmo = BARNEY_WEAPON_CLIP_SIZE;
				}else if(EASY_CVAR_GET_DEBUGONLY(barneyDroppedGlockAmmoCap) == -1){
					g->m_iDefaultAmmo = BARNEY_WEAPON_CLIP_SIZE - 1;
				}else{
					g->m_iDefaultAmmo = min((int)EASY_CVAR_GET_DEBUGONLY(barneyDroppedGlockAmmoCap), m_cAmmoLoaded);
				}
			}
		}//END OF if(pGun != NULL)

	}

	SetUse( NULL );	
	GENERATE_KILLED_PARENT_CALL(CTalkMonster);
}

//=========================================================
// AI Schedules Specific to this monster
//=========================================================

Schedule_t* CBarney::GetScheduleOfType ( int Type )
{
	Schedule_t *psched;

	switch( Type )
	{
	case SCHED_BARNEY_DISARM_WEAPON:

		//in case interrupted by something...   this is probably pointless anyways.
		unholsterTimer = gpGlobals->time + 3;

		return slBarneyUnDraw;
	break;
	case SCHED_ARM_WEAPON:
		if ( m_hEnemy != NULL )
		{
			if(EASY_CVAR_GET_DEBUGONLY(barneyUnholsterTime) != -1){
				unholsterTimer = gpGlobals->time + EASY_CVAR_GET_DEBUGONLY(barneyUnholsterTime);
			}
			// face enemy, then draw.
			return slBarneyEnemyDraw;
		}

		//MODDD NOTE - lacking any fallback in caes the enemy is NULL may look strange, but since SCHED_ARM_WEAPON is a default SCHED_...
		//             present for all entities, the base monster class's GetScheduleOfType method of schedule.cpp has something in this case
		//             to be used.  It's... nearly identical but doesn't face the enemy.
		//             well that design choice was... interesting.  surely that task can canel itself or decide to do nothing if m_hEnemy
		//             were NULL.

		break;

	// Hook these to make a looping schedule
	case SCHED_TARGET_FACE:
		// call base class default so that barney will talk
		// when 'used' 
		psched = CTalkMonster::GetScheduleOfType(Type);

		if (psched == slIdleStand)
			return slBaFaceTarget;	// override this for different target face behavior
		else
			return psched;

	case SCHED_TARGET_CHASE:
		return slBaFollow;

	case SCHED_IDLE_STAND:
		// call base class default so that scientist will talk
		// when standing during idle
		psched = CTalkMonster::GetScheduleOfType(Type);

		if (psched == slIdleStand)
		{
			// just look straight ahead.
			return slIdleBaStand;
		}
		else
			return psched;	

	//MODDD
	case SCHED_BARNEY_RELOAD:
		return &slBaReload[ 0 ];
	break;

	
	case SCHED_CANT_FOLLOW:
		//NOTE: monsters still must say this by themselves.  In case some monster doesn't do "CANT_FOLLOW" at all...?
		//...let CTalkMonster do this.
		return CTalkMonster::GetScheduleOfType(Type);
	break;


	//MODDD - why was this not here?
	//NO, NEVERMIND.  Better method above where "SCHED_SMALL_FLINCH" is referred to.
	/*
	case SCHED_SMALL_FLINCH:
		//since this is called at the start of an attack, this makes sense.
		AlertSound();
	break;
	*/



	}//END OF switch( Type )


	return CTalkMonster::GetScheduleOfType( Type );
}


void CBarney::OnPlayerDead(CBasePlayer* arg_player){
	CTalkMonster::OnPlayerDead(arg_player);
	// leave most of this up to dead phase
	
	if(!leaderRecentlyDied){
		// And don't make extra noise if this NPC was following the player when it died.
		// Already said something then, this would be redundant.
		//SayFear();

		if(FVisible(arg_player) && FInViewCone(arg_player)){
			// can see?  Go ahead then.
			MakeIdealYaw(arg_player->pev->origin);
			ChangeSchedule(GetScheduleOfType(SCHED_ALERT_FACE));
		}
	}
}

void CBarney::OnPlayerFollowingSuddenlyDead(void){
	CTalkMonster::OnPlayerFollowingSuddenlyDead();

	MakeIdealYaw(m_hTargetEnt->pev->origin);
	ChangeSchedule(GetScheduleOfType(SCHED_ALERT_FACE));
}


//=========================================================
// GetSchedule - Decides which type of schedule best suits
// the monster's current state and conditions. Then calls
// monster's member function to get a pointer to a schedule
// of the proper type.
//=========================================================
Schedule_t *CBarney::GetSchedule ( void )
{

	//easyForcePrintLine("MY timer %d    %d %d %d :::%.2f %.2f", m_fGunDrawn, HasConditions( bits_COND_HEAR_SOUND ), HasConditions(bits_COND_SEE_ENEMY), HasConditions(bits_COND_NEW_ENEMY), gpGlobals->time, unholsterTimer);

	canUnholster=FALSE; // until proven otherwise
	if(EASY_CVAR_GET_DEBUGONLY(barneyUnholsterTime) != -1 && m_fGunDrawn){
		if( (HasConditions( bits_COND_HEAR_SOUND ) || HasConditions(bits_COND_SEE_ENEMY) || HasConditions(bits_COND_NEW_ENEMY)) ){
			//set the timer...  keep it up.
			unholsterTimer = gpGlobals->time + EASY_CVAR_GET_DEBUGONLY(barneyUnholsterTime);
		}
		//been too long since hearing anything or seeing any hostiles?  unholster when convenient.
		if(unholsterTimer != -1 && gpGlobals->time >= unholsterTimer){
			canUnholster = TRUE;
		}
	}

	if ( HasConditions( bits_COND_HEAR_SOUND ) )
	{
		CSound *pSound;
		pSound = PBestSound();

		ASSERT( pSound != NULL );
		if ( pSound && (pSound->m_iType & bits_SOUND_DANGER) )
			return GetScheduleOfType( SCHED_TAKE_COVER_FROM_BEST_SOUND );
	}
	if ( HasConditions( bits_COND_ENEMY_DEAD ) )
	{
		// BREAKPOINT?
		int x = 346;
		if (FOkToSpeak()) {
			talkAboutKilledEnemy();
		}
	}

	switch( m_MonsterState )
	{
	case MONSTERSTATE_COMBAT:
		{
// dead enemy
			if ( HasConditions( bits_COND_ENEMY_DEAD ) )
			{
				// call base class, all code to handle dead enemies is centralized there.
				return CBaseMonster::GetSchedule();
			}

			//MODDD - added.
			if( HasConditions (bits_COND_NEW_ENEMY) ){
				AlertSound();
			}

			// always act surprized with a new enemy
			if ( HasConditions( bits_COND_NEW_ENEMY ) && HasConditions( bits_COND_LIGHT_DAMAGE) ){
				return GetScheduleOfType( SCHED_SMALL_FLINCH );
			}
			//MODDD - reload call.
			else if ( HasConditions ( bits_COND_NO_AMMO_LOADED ) )
			{
				//!!!KELLY - this individual just realized he's out of bullet ammo. 
				// He's going to try to find cover to run to and reload, but rarely, if 
				// none is available, he'll drop and reload in the open here. 
				return GetScheduleOfType ( SCHED_BARNEY_RELOAD );
			}


			// wait for one schedule to draw gun
			if (!m_fGunDrawn ){
				return GetScheduleOfType( SCHED_ARM_WEAPON );
			}


			if (recentDeclines >= 30) {
				// no movement-related schedules, just see if we can shoot

				if (HasConditions(bits_COND_CAN_RANGE_ATTACK1)) {

					if (recentDeclinesForgetTime - gpGlobals->time < 30) {
						//force it up
						recentDeclinesForgetTime = gpGlobals->time + 30;
					}

					return GetScheduleOfType(SCHED_RANGE_ATTACK1);
				}
				else {
					if (HasConditions(bits_COND_HEAR_SOUND)) {
						return slCombatFaceSound;
					}

					//if (!HasConditions(bits_COND_ENEMY_OCCLUDED)) {
					if(HasConditions(bits_COND_SEE_ENEMY)){
						// I can face them.
						return GetScheduleOfType(SCHED_COMBAT_FACE);
					}
					else {
						// oh
						return GetScheduleOfType(SCHED_COMBAT_STAND);
					}
				}


			}


			if ( HasConditions( bits_COND_HEAVY_DAMAGE ) )
				return GetScheduleOfType( SCHED_TAKE_COVER_FROM_ENEMY );
		}
		break;

	case MONSTERSTATE_ALERT:	
	case MONSTERSTATE_IDLE:
		

		if ( HasConditions(bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE))
		{
			// flinch if hurt
			//return GetScheduleOfType( SCHED_SMALL_FLINCH );

			//COME ON BARNEY. FACE IT!
			return GetScheduleOfType(SCHED_ALERT_SMALL_FLINCH);
		}

		//MODDD
		//easyForcePrintLine("AMMO: %d   %d", m_cAmmoLoaded, (BARNEY_WEAPON_CLIP_SIZE/2) );
		
		if ( m_cAmmoLoaded < BARNEY_WEAPON_CLIP_SIZE / 2 )
		{
			//barney would rather reload than follow if the clip size is less than half.
			return GetScheduleOfType ( SCHED_BARNEY_RELOAD );
		}


		if ( m_hEnemy == NULL && IsFollowing() )
		{
			if ( !m_hTargetEnt->IsAlive() )
			{

				SayLeaderDied();
				// UNDONE: Comment about the recently dead player here?
				StopFollowing( FALSE, FALSE );  //no generic unuse sentence.
				break;
			}
			else
			{
				if ( HasConditions( bits_COND_CLIENT_PUSH ) )
				{
					return GetScheduleOfType( SCHED_MOVE_AWAY_FOLLOW );
				}
				return GetScheduleOfType( SCHED_TARGET_FACE );
			}
		}

		if ( HasConditions( bits_COND_CLIENT_PUSH ) )
		{
			return GetScheduleOfType( SCHED_MOVE_AWAY );
		}


		//nah, nevermind.
		/*
		//MODDD Like a smart player, reload the weapon during inactivity if the clip isn't full.  Be prepared.
		if ( m_cAmmoLoaded < BARNEY_WEAPON_CLIP_SIZE )
		{
			return GetScheduleOfType ( SCHED_BARNEY_RELOAD );
		}
		*/

		// try to say something about smells
		TrySmellTalk();
		break;
	}


	if(canUnholster){
		//try.
		return GetScheduleOfType( SCHED_BARNEY_DISARM_WEAPON );
	}

	return CTalkMonster::GetSchedule();
}

MONSTERSTATE CBarney::GetIdealState ( void )
{
	return CTalkMonster::GetIdealState();
}


void CBarney::DeclineFollowing( void )
{
	recentDeclines++;
	if (recentDeclines < 30) {
		recentDeclinesForgetTime = gpGlobals->time + 12;
	}
	else if(recentDeclines < 35){
		recentDeclinesForgetTime = gpGlobals->time + 30;
	}
	else if (recentDeclines == 35) {
		// no, do the alert sound in SayDeclineFollowing instead
		recentDeclinesForgetTime = gpGlobals->time + 50;
	}
	else {
		recentDeclinesForgetTime = gpGlobals->time + 50;
	}

	if (recentDeclines == 30) {
		// ...   hold my beer.
		//edict_t* pentPlayer = FIND_CLIENT_IN_PVS(this->edict());

		
		//if (pentPlayer != NULL) {
			beforeSpamRampageState.Save(this);
			Remember(bits_MEMORY_PROVOKED);
			m_IdealMonsterState = MONSTERSTATE_COMBAT;

			forgiveMeForWhatIMustDo = TRUE;

			//CBaseEntity* test = CBaseEntity::Instance(pentPlayer);
			//m_hEnemy = test;
		//}
			
	}

}


//=========================================================
// DEAD BARNEY PROP
//
// Designer selects a pose in worldcraft, 0 through num_poses-1
// this value is added to what is selected as the 'first dead pose'
// among the monster's normal animations. All dead poses must
// appear sequentially in the model file. Be sure and set
// the m_iFirstPose properly!
//
//=========================================================
class CDeadBarney : public CBaseMonster
{
public:
	CDeadBarney();
	void Spawn( void );
	int Classify ( void ) { return	CLASS_PLAYER_ALLY; }
	BOOL isOrganic(void){return !CanUseGermanModel();}

	void KeyValue( KeyValueData *pkvd );
	

	BOOL getGermanModelRequirement(void);
	const char* getGermanModel(void);
	const char* getNormalModel(void);


	int m_iPose;// which sequence to display	-- temporary, don't need to save
	static char *m_szPoses[3];
};

char *CDeadBarney::m_szPoses[] = { "lying_on_back", "lying_on_side", "lying_on_stomach" };

void CDeadBarney::KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "pose"))
	{
		m_iPose = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else 
		CBaseMonster::KeyValue( pkvd );
}


CDeadBarney::CDeadBarney(){

}


#if REMOVE_ORIGINAL_NAMES != 1
	LINK_ENTITY_TO_CLASS( monster_barney_dead, CDeadBarney );
#endif

#if EXTRA_NAMES > 0
	LINK_ENTITY_TO_CLASS( barney_dead, CDeadBarney );

	//no extras.
#endif


BOOL CDeadBarney::getGermanModelRequirement(void){
	return globalPSEUDO_germanModel_barneyFound;
}
const char* CDeadBarney::getGermanModel(void){
	return "models/g_barney.mdl";
}
const char* CDeadBarney::getNormalModel(void){
	return "models/barney.mdl";
}



//=========================================================
// ********** DeadBarney SPAWN **********
//=========================================================
void CDeadBarney::Spawn( )
{
	PRECACHE_MODEL("models/barney.mdl");
	setModel(); //"models/barney.mdl"  argument unused when there's a german model equivalent.

	pev->classname = MAKE_STRING("monster_barney_dead");

	pev->effects		= 0;
	pev->yaw_speed		= 8;
	pev->sequence		= 0;
	m_bloodColor = BloodColorRedFilter();


	pev->sequence = LookupSequence( m_szPoses[m_iPose] );
	if (pev->sequence == -1)
	{
		ALERT ( at_console, "Dead barney with bad pose\n" );
	}
	// Corpses have less health
	pev->health			= 8;//gSkillData.barneyHealth;


	MonsterInitDead();
	
	//MODDD - by request, spawned dead barny's lack guns.
	SetBodygroup(BODYGROUP_GUN, BARNEY_BODY_GUNGONE);

	//MODDD - see scientist.cpp, this lets the mouth stay open.
	// Hard to prove if it was meant to be this way for barnies too, but eh, why not.
	// Changed our mind, gone.
	//SetBoneController(4, 15);

	// TEST!!!     Is this readable in studiomodelrenderer?
	// no.  it is not.     dam.
	//pev->playerclass = 4;

	
	if(isOrganicLogic()){
		//MODDD - emit a stench that eaters will pick up.
		CSoundEnt::InsertSound ( bits_SOUND_CARCASS, pev->origin, 384, SOUND_NEVER_EXPIRE );
	}

}


void CBarney::HandleEventQueueEvent(int arg_eventID){
	int rand;
	BOOL pass;

					//easyForcePrintLine("EVETTTT:::%d", arg_eventID);
	switch(arg_eventID){
		case 0:
			SetBodygroup(BODYGROUP_GUN, BARNEY_BODY_GUNHOLSTERED);
			m_fGunDrawn = FALSE;
			//...
		break;
	}//END OF switch(...)
	
}


BOOL CBarney::usesAdvancedAnimSystem(void){
	return TRUE;
}

void CBarney::ReportAIState( void )
{
	CTalkMonster::ReportAIState();

	//add...
	easyForcePrintLine("BARNEY: timers: %.2f %.2f", gpGlobals->time, unholsterTimer);
}

int CBarney::LookupActivityHard(int activity){
	pev->framerate = 1;
	resetEventQueue();

	m_flFramerateSuggestion = 1;
	/*
	m_flFramerateSuggestion = -1;
	//pev->frame = 6;
	return LookupSequence("get_bug");
	*/

	if (!(activity == ACT_IDLE || activity == ACT_CROUCH || activity == ACT_CROUCHIDLE || activity == ACT_TURN_LEFT || activity == ACT_TURN_RIGHT)) {
		// reset it next time.
		nextIdleFidgetAllowedTime = -1;
	}


	int iRandChoice = 0;
	int iRandWeightChoice = 0;
	
	char* animChoiceString = NULL;
	int* weightsAbs = NULL;
			
	//pev->framerate = 1;
	int maxRandWeight = 30;

	//easyForcePrintLine("BARNEYYYYYYYYYY %d", activity);

	//let's do m_IdealActivity??
	//uh... why?  nevermind then.
	switch(activity){
		case ACT_RANGE_ATTACK1:
			if (EASY_CVAR_GET_DEBUGONLY(hyperBarney) != 1) {
				if (g_iSkillLevel == SKILL_EASY) {
					// bigger boost
					m_flFramerateSuggestion = 1.20;
				}else {
					// slight boost anyway/
					m_flFramerateSuggestion = 1.09;
				}
			}else {
				// Bow before your new God
				m_flFramerateSuggestion = 8;
			}


			// TEST!!!
			//m_flFramerateSuggestion = -1;

			//animFrameCutoffSuggestion = 220;
			return CBaseAnimating::LookupActivity(activity);
		break;
		case ACT_RELOAD:
			if (EASY_CVAR_GET_DEBUGONLY(hyperBarney) != 1) {
				if (g_iSkillLevel == SKILL_EASY) {
					// bigger boost
					m_flFramerateSuggestion = 1.23;
				}else {
					// slight boost anyway/
					m_flFramerateSuggestion = 1.11;
				}
			}else {
				// Bow before your new God
				m_flFramerateSuggestion = 13;
			}
			return CBaseAnimating::LookupActivity(activity);
			break;
		case ACT_DISARM:
			//here comes the train... the PAIN TRAIN.
			//m_flFramerateSuggestion = 1.3;
			//this->animEventQueuePush(6.7f / 30.0f, 0);

			if(EASY_CVAR_GET_DEBUGONLY(barneyUnholsterAnimChoice) == 0){
				//reverse draw.
				//m_flFramerateSuggestion = -0.62;
				this->animFrameStartSuggestion = 254;
				//this->animFrameCutoff = 16;
				//this->animFrameStart = 254;6
				this->animEventQueuePush( 6.9/20.0, 0);
				m_flFramerateSuggestion = -0.67;
				//pev->framerate = -1;
				return LookupSequence("draw");
			}else if(EASY_CVAR_GET_DEBUGONLY(barneyUnholsterAnimChoice) == 1){
				//just the disarm animation.
				m_flFramerateSuggestion = 1.07;
				return LookupSequence("disarm");
			}else{
				//???
				return CBaseAnimating::LookupActivity(activity);
			}
			
		break;
		case ACT_IDLE:{

			//First off, are we talking right now?
			if(IsTalking()){
				//Limit the animations we can choose from a little more.
				//Most people don't typically move around too much while looking at someone and talking to them,
				//compared to just standing around or listening to a long conversation.
				//BUT, simulare the wold weights.  The sum of all weights of the available animations
				//(see a scientist.qc file from decompiling the model) is used instead of course.

				//50, 1, 10, 10,
				const int animationWeightTotal = 50;
				const int animationWeightChoice = RANDOM_LONG(0, animationWeightTotal-1);

				//if(animationWeightChoice < 50){
				m_flFramerateSuggestion = 0.86f;
				return LookupSequence("idle1");
				//}
			}else{
				//Not talking?  One more filter...
				//Are we in predisaster?
				if(FBitSet(pev->spawnflags, SF_MONSTER_PREDISASTER)){
					// Idle's 2 and 4 are a little paranoid looking for a non-dangerous situation,
					// or while talking (see above, even #3 is dropped).

					if (nextIdleFidgetAllowedTime == -1) {
						// set it.
						nextIdleFidgetAllowedTime = gpGlobals->time + RANDOM_FLOAT(11, 15);
					}

					if (gpGlobals->time < nextIdleFidgetAllowedTime) {
						// not there yet?  Pick a plain idle
						return LookupSequence("idle1");
					}
					else {
						const int animationWeightTotal = 3 + 2 + 2 + 1;
						const int animationWeightChoice = RANDOM_LONG(0, animationWeightTotal - 1);

						nextIdleFidgetAllowedTime = gpGlobals->time + RANDOM_FLOAT(11, 15);

						// play a fidget
						return LookupSequence("idle3");
					}


				}else{
					/*
					//Just pick from the model, any idle animation is okay right now.
					int theSeq = CBaseAnimating::LookupActivity(activity);

					//idle1 and idle3.
					if (theSeq == 0 || theSeq == 2) {
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
						const int animationWeightTotal = 90 + 5;
						const int animationWeightChoice = RANDOM_LONG(0, animationWeightTotal - 1);

						if (animationWeightChoice < 90) {
							m_flFramerateSuggestion = 0.84f;
							return LookupSequence("idle1");
						}
						else if (animationWeightChoice < 90 + 5) {
							m_flFramerateSuggestion = 0.84f;
							return LookupSequence("idle3");
						}
					}
					else {
						const int animationWeightTotal = 6 + 4 + 1;
						const int animationWeightChoice = RANDOM_LONG(0, animationWeightTotal - 1);

						nextIdleFidgetAllowedTime = gpGlobals->time + RANDOM_FLOAT(11, 15);

						// play a fidget
						if (animationWeightChoice < 6) {
							m_flFramerateSuggestion = 0.97f;
							return LookupSequence("idle3");
						}
						else if (animationWeightChoice < 6 + 4) {
							m_flFramerateSuggestion = 0.92f;
							return LookupSequence("idle2");
						}
						else { //if(animationWeightChoice < 6 + 4 + 1){
							m_flFramerateSuggestion = 0.94f;
							return LookupSequence("idle4");
						}
					}

				}
				
			}//END OF IsTalking check
			
		break;}
		/*
		case ACT_VICTORY_DANCE:
			m_flFramerateSuggestion = 1.16;
			return LookupSequence("hambone");
		break;
		*/

	}//END OF switch
	//not handled by above?  try the real deal.
	return CBaseAnimating::LookupActivity(activity);
}


int CBarney::tryActivitySubstitute(int activity){
	int iRandChoice = 0;
	int iRandWeightChoice = 0;
	char* animChoiceString = NULL;
	int* weightsAbs = NULL;
	//pev->framerate = 1;
	int maxRandWeight = 30;

	// no need for default, just falls back to the normal activity lookup.
	switch(activity){
		case ACT_DISARM:
			if(EASY_CVAR_GET_DEBUGONLY(barneyUnholsterAnimChoice) == 0){
				//reverse draw.
				//m_flFramerateSuggestion = -1;
				return LookupSequence("draw");
			}else if(EASY_CVAR_GET_DEBUGONLY(barneyUnholsterAnimChoice) == 1){
				//just the disarm animation.
				return LookupSequence("disarm");
			}else{
				//???
				return CBaseAnimating::LookupActivity(activity);
			}
		break;
		/*
		case ACT_VICTORY_DANCE:
			// a small portion of the time, allow this.   why not.
			if (RANDOM_FLOAT(0, 1) < 0.04) {
				return LookupSequence("hambone");
			}
		break;
		*/

	}//END OF switch
	
	//not handled by above?
	return CBaseAnimating::LookupActivity(activity);
}


BOOL CBarney::canResetBlend0(void){
	return TRUE;
}


BOOL CBarney::onResetBlend0(void){
	//easyForcePrintLine("HOW IT GO   %d", (m_hEnemy!=NULL));
	if (m_hEnemy == NULL)
	{
		return FALSE;
	}

	//NOTICE: may be a good idea to do this first.  ShootAtEnemy may use the global forward vector generated by this.
	UTIL_MakeVectors(pev->angles);

	//Barney does it this way, for some reason.
	Vector vecShootDir = ShootAtEnemyMod( GetGunPosition() );

	//UTIL_MakeVectors ( pev->angles );

	Vector angDir = UTIL_VecToAngles( vecShootDir );
	//easyForcePrintLine("YOU BLASTARD %.2f", angDir.x);
	//easyForcePrintLine("ANG::::%.2f", angDir.x);
	
	
	//easyPrintLine("THIS IS BARNEY:%d HOW DO YOU DO %.2f", this->monsterID, angDir.x);
	SetBlending( 0, angDir.x );

	return TRUE;
}


BOOL CBarney::violentDeathAllowed(void){
	return TRUE;
}
BOOL CBarney::violentDeathClear(void){
	//Works for a lot of things going backwards.
	return violentDeathClear_BackwardsCheck(200);
}//END OF violentDeathAllowed
int CBarney::violentDeathPriority(void){
	return 3;
}

//MODDD - notice. No need to implement GetGunPosition and GetGunPositionAI, their defaults
// both use the hacked gun position instead.  AGrunt shows querying the model itself for GetGunPosition instead,
// which may or may not be present in the barney model or even worthwhile at this point.



// TODOish? - barney cannot talk about enemies that have been gibbed, they don't even make it to CheckEnemy in basemonster.cpp,
// which leads to calling onEnemyDead below which sets the recent class for basing comments off of for barney.
// Not too big of a deal though, barney can't gib stuff so why should he try to take credit for gibbed kills.
void CBarney::talkAboutKilledEnemy(void) {
	if (EASY_CVAR_GET(pissedNPCs) == 0 || !globalPSEUDO_iCanHazMemez) {
		//MODDD - intervention again.
		// Who says "That'll look great in my trophie room" after killing people?
		// Madman.
		// And don't get from "m_hEnemy", as the whole point is they're recently dead.
		// m_hEnemy might have already been wiped.

		int enemyClassify = recentDeadEnemyClass;
		//long randoRange;
		

		// If my health is under 40%, chance of complaining about health.
		if (pev->health < pev->max_health*0.4 && RANDOM_FLOAT(0,1) < 0.3) {
			PlaySentenceSingular("BA_ZOMBIE4", 3, VOL_NORM, ATTN_NORM);
		}else if (RANDOM_FLOAT(0, 1) < 0.06) {
			PlaySentenceSingular("BA_KILL7", 3, VOL_NORM, ATTN_NORM);
		}
		else if (enemyClassify == CLASS_NONE) {
			// nothing? Go through all quotes as usual then.
			PlaySentence("BA_KILL", 4, VOL_NORM, ATTN_NORM);
		}
		else if (enemyClassify == CLASS_PLAYER) {

			switch (RANDOM_LONG(0, 2)) {
			case 0:PlaySentenceSingular("BA_KILL3", 3, VOL_NORM, ATTN_NORM);break;
			case 1:PlaySentenceSingular("BA_KILL4", 4, VOL_NORM, ATTN_NORM);break;
			case 2:PlaySentenceSingular("BA_KILL6", 4, VOL_NORM, ATTN_NORM);break;
			}
		}
		else if (
			enemyClassify == CLASS_ALIEN_MILITARY ||
			enemyClassify == CLASS_ALIEN_PASSIVE ||
			enemyClassify == CLASS_ALIEN_MONSTER
			) {
			// smarter aliens? all good.
			PlaySentence("BA_KILL", 4, VOL_NORM, ATTN_NORM);
		}
		else if (
			enemyClassify == CLASS_ALIEN_PREY ||
			enemyClassify == CLASS_ALIEN_PREDATOR ||
			enemyClassify == CLASS_BARNACLE ||
			enemyClassify == CLASS_ALIEN_BIOWEAPON ||
			enemyClassify == CLASS_PLAYER_BIOWEAPON
			) {
			// animal aliens? all good.
			PlaySentence("BA_KILL", 4, VOL_NORM, ATTN_NORM);
		}
		else {
			// human or robotic?

			switch (RANDOM_LONG(0, 2)) {
			case 0:PlaySentenceSingular("BA_KILL3", 3, VOL_NORM, ATTN_NORM);break;
			case 1:PlaySentenceSingular("BA_KILL4", 4, VOL_NORM, ATTN_NORM);break;
			case 2:PlaySentenceSingular("BA_KILL6", 4, VOL_NORM, ATTN_NORM);break;
			}
		}//END OF enemy classify check


		if (EASY_CVAR_GET_DEBUGONLY(barneyUnholsterTime) != -1 && unholsterTimer != -1) {
			//MODDD - go ahead and add some randomness to the unholster timer... can't hurt.
			unholsterTimer += RANDOM_FLOAT(0, 3);
		}
	}
	else if (EASY_CVAR_GET(pissedNPCs) == 1) {
		if (RANDOM_FLOAT(0, 24) == 0) {
			womboCombo();
		}
		else {
			PlaySentence("BA_POKE_B", 6, VOL_NORM, ATTN_NORM);
		}
	}
	else if (EASY_CVAR_GET(pissedNPCs) == 2) {
		//guaranteed wombo combo... maybe.
		womboCombo();
	}
}


// pRecentEnemy can never be NULL.
void CBarney::onEnemyDead(CBaseEntity* pRecentEnemy) {
	recentDeadEnemyClass = pRecentEnemy->Classify();;
}




void CBarney::CompleteRestoreState(void) {

	beforeSpamRampageState.Restore(this);

	pev->frame = 0;
	pev->framerate = 1;
	m_flFramerateSuggestion = 1;

	ResetSequenceInfo();  // with the existing sequence from OLD_sequence already applied to pev->sequence
	SetYawSpeed();

	m_fGunDrawn = FALSE;
	if (EASY_CVAR_GET_DEBUGONLY(glockOldReloadLogicBarney) == 0) {
		//not using old reload logic?  Barney just has 12 rounds.
		m_cAmmoLoaded = BARNEY_WEAPON_CLIP_SIZE - 1;
	}
	else {
		//Firing chamber has one.  Extra (13 total).
		m_cAmmoLoaded = BARNEY_WEAPON_CLIP_SIZE;
	}

}


void CBarney::OnForgiveDeclineSpam(void) {

	// used the save-state feature? Bring it back.
	if (forgiveMeForWhatIMustDo) {

		if (m_fGunDrawn) {
			// undraw it
			ChangeSchedule(slBarneyUnDrawForgiveSpam);
		}
		else {
			// skip to here
			CompleteRestoreState();
			forgiveMeForWhatIMustDo = FALSE;
		}
	}


	CTalkMonster::OnForgiveDeclineSpam();
}



//MODDD .... yep.
void CBarney::womboCombo(void) {
	//wombo combo.
	//Gather' round all the NPCs.
	//check for allied NPCs to heal if not following.
	CBaseEntity* pEntityScan = NULL;
	CBaseEntity* testMon = NULL;
	float thisDistance;
	float leastDistanceYet;
	CTalkMonster* thisNameSucks;
	CTalkMonster* bestChoiceYet;
	float thisNameSucksExtraMax = 3;
	//I'm number 1!
	CTalkMonster* pickedNumber2 = NULL;
	CTalkMonster* pickedNumber3 = NULL;

	//does UTIL_MonstersInSphere work?
	while ((pEntityScan = UTIL_FindEntityInSphere(pEntityScan, pev->origin, 800)) != NULL)
	{
		testMon = pEntityScan->MyMonsterPointer();
		//if(testMon != NULL && testMon->pev != this->pev && ( FClassnameIs(testMon->pev, "monster_scientist") || FClassnameIs(testMon->pev, "monster_barney")  ) ){
		if (testMon != NULL && testMon->pev != this->pev && UTIL_IsAliveEntity(testMon) && testMon->isTalkMonster()) {
			thisDistance = (testMon->pev->origin - pev->origin).Length();
			thisNameSucks = static_cast<CTalkMonster*>(testMon);
			if (pickedNumber2 == NULL) {
				pickedNumber2 = thisNameSucks;
			}
			else if (pickedNumber3 == NULL) {
				pickedNumber3 = thisNameSucks;
			}
			else if (thisNameSucksExtraMax > 0) {
				thisNameSucks->PlaySentence("!wombocrowd", 39, 0.9, ATTN_NORM);
				thisNameSucksExtraMax--;
			}
			if (thisNameSucksExtraMax <= 0) {
				break;
			}
		}
	}//END OF while(...)

	if (pickedNumber2 != NULL && pickedNumber3 != NULL) {
		//WE GOT A COMBO!!!
		this->PlaySentence("!wombo1", 39, 0.9, ATTN_NORM);
		pickedNumber2->PlaySentence("!wombo2", 39, 0.9, ATTN_NORM);
		pickedNumber3->PlaySentence("!wombo3", 39, 0.9, ATTN_NORM);
	}
	else {
		PlaySentence("BA_POKE_B", 6, VOL_NORM, ATTN_NORM);
	}
}




int CBarney::CanPlaySentence(BOOL fDisregardState)
{
	return IsAlive();
}
int CBarney::CanPlaySequence(BOOL fDisregardMonsterState, int interruptLevel)
{
	// doesn't seem to really work, but eh.
	if (forgiveMeForWhatIMustDo) {
		// deny.
		return FALSE;
	}

	return CTalkMonster::CanPlaySequence(fDisregardMonsterState, interruptLevel);
}




