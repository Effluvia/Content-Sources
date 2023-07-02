

#ifndef BARNEY_H
#define BARNEY_H

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "basemonster.h"
#include "talkmonster.h"
#include "monstersavestate.h"




class CBarney : public CTalkMonster
{
public:

	//MODDD - for checking to play barney's alert, in case other sounds take precedence for some reason.
	static float g_barneyAlertTalkWaitTime;

	static const char* madInterSentences[];
	static int madInterSentencesMax;


	BOOL canUnholster;
	float unholsterTimer;
	int recentDeadEnemyClass;

	BOOL m_fGunDrawn;
	float m_painTime;
	float m_checkAttackTime;
	BOOL m_lastAttackCheck;

	float reloadSoundTime;


	SimpleMonsterSaveState beforeSpamRampageState;

	BOOL forgiveMeForWhatIMustDo;



	CBarney(void);

	void ReportAIState(void);


	int getMadSentencesMax(void);
	int getMadInterSentencesMax(void);


	void MonsterThink(void);
	int  IRelationship(CBaseEntity* pTarget);

	void Spawn(void);
	void Precache(void);
	void SetYawSpeed(void);
	int  ISoundMask(void);
	void BarneyFirePistol(void);
	void AlertSound(void);
	int  Classify(void);
	BOOL isOrganic(void) { return !CanUseGermanModel(); }

	BOOL getGermanModelRequirement(void);
	const char* getGermanModel(void);
	const char* getNormalModel(void);

	void HandleAnimEvent(MonsterEvent_t* pEvent);

	void RunTask(Task_t* pTask);
	void StartTask(Task_t* pTask);
	virtual int ObjectCaps(void) { return CTalkMonster::ObjectCaps() | FCAP_IMPULSE_USE; }


	BOOL CheckRangeAttack1(float flDot, float flDist);

	void DeclineFollowing(void);

	// Override these to set behavior
	Schedule_t* GetScheduleOfType(int Type);
	Schedule_t* GetSchedule(void);
	MONSTERSTATE GetIdealState(void);

	//MODDD - new.
	void SetObjectCollisionBox(void)
	{
		if (pev->deadflag != DEAD_NO) {
			pev->absmin = pev->origin + Vector(-65, -65, 0);
			pev->absmax = pev->origin + Vector(65, 65, 72);
		}
		else {
			CBaseMonster::SetObjectCollisionBox();
		}
	}


	void DeathSound(void);
	void PainSound(void);
	//MODDD - new version that bypasses the usual forced delay before playing another pain sound.
	void PainSound(BOOL bypassCooldown);

	void TalkInit(void);

	//MODDD
	GENERATE_TRACEATTACK_PROTOTYPE
	GENERATE_TAKEDAMAGE_PROTOTYPE

	void OnAlerted(BOOL alerterWasKilled);

	GENERATE_KILLED_PROTOTYPE


	BOOL violentDeathAllowed(void);
	BOOL violentDeathClear(void);
	int violentDeathPriority(void);

	void talkAboutKilledEnemy(void);
	void onEnemyDead(CBaseEntity* pRecentEnemy);

	void CompleteRestoreState(void);
	void OnForgiveDeclineSpam(void);


	virtual int	Save(CSave& save);
	virtual int	Restore(CRestore& restore);
	static TYPEDESCRIPTION m_SaveData[];

	//void Think(void);
	void CheckAmmo(void);
	void SetActivity(Activity NewActivity);

	//MODDD - new anim stuff.
	void HandleEventQueueEvent(int arg_eventID);
	BOOL usesAdvancedAnimSystem(void);
	int tryActivitySubstitute(int activity);
	int LookupActivityHard(int activity);

	void DeclineFollowingProvoked(CBaseEntity* pCaller);
	void SayAlert(void);
	void SayDeclineFollowing(void);
	void SayDeclineFollowingProvoked(void);
	void SayProvoked(void);
	void SayStopShooting(void);
	void SaySuspicious(void);
	void SayLeaderDied(void);
	void SayKneel(void);
	void SayNearPassive(void);

	void OnNearCautious(void);
	void SayNearCautious(void);


	BOOL canResetBlend0(void);
	BOOL onResetBlend0(void);

	void OnPlayerDead(CBasePlayer* arg_player);
	void OnPlayerFollowingSuddenlyDead(void);

	void womboCombo(void);

	int CanPlaySentence(BOOL fDisregardState);
	int CanPlaySequence(BOOL fDisregardMonsterState, int interruptLevel);

	CUSTOM_SCHEDULES;
};


#endif //BARNEY_H
