//=========================================================
// MR FRIENDLY (friendly)
//=========================================================

#ifndef FRIENDLY_H
#define FRIENDLY_H

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "basemonster.h"
#include "linkedlist_ehandle.h"



class CFriendly : public CBaseMonster{
public:

	LinkedListTemp shieldSapList;
	
	BOOL m_fPissedAtPlayer;
	BOOL m_fPissedAtPlayerAlly;
	BOOL m_fPissedAtHumanMilitary;

	float extraPissedFactor;

	BOOL playedVomitSoundYet;

	float nextVomitHitSoundAllowed;

	static const char* pDeathSounds[];
	static const char* pAlertSounds[];
	static const char* pIdleSounds[];
	static const char* pPainSounds[];
	static const char* pAttackSounds[];
	static const char* pAttackHitSounds[];
	static const char* pAttackMissSounds[];
	static const char* pVomitVoiceSounds[];
	static const char* pVomitHitSounds[];
	static const char* pChewSounds[];
	static const char* pVomitSounds[];
	
	float horrorPlayTime;
	float horrorPlayTimePreDelay;
	BOOL horrorSelected;

	float eatFinishTimer;
	float eatFinishPostWaitTimer;
	float nextPlayerSightCheck;
	float waitTime;
	float timeToStare;
	float vomitCooldown;
	float nextChewSound;
	EHANDLE playerToLookAt;
	EHANDLE corpseToSeek;

	BOOL rapidVomitCheck;
	BOOL rapidVomitCheck_ScheduleFinish;
	float nextNormalThinkTime;

	void stopHorrorSound(void);
	
	//save info
	//////////////////////////////////////////////////////////////////////////////////
	static TYPEDESCRIPTION m_SaveData[];
	virtual int Save( CSave &save ); 
	virtual int Restore( CRestore &restore );
	//////////////////////////////////////////////////////////////////////////////////
	void PostRestore(void);


	CFriendly(void);

	CUSTOM_SCHEDULES;

	void DeathSound ( void );
	void AlertSound ( void );
	void IdleSound ( void );
	void PainSound ( void );
	void AttackSound ( void );
	void VomitSound(void);
	void VomitVoiceSound(void);
	void VomitHitSound(edict_t* pevToPlayAt);

	
	void Precache(void);
	void Spawn(void);
	
	Schedule_t *GetSchedule( void );
	Schedule_t* GetScheduleOfType( int Type);
	
	void StartTask( Task_t *pTask );
	void RunTask( Task_t *pTask );
	
	BOOL CheckMeleeAttack1 ( float flDot, float flDist );
	BOOL CheckMeleeAttack2 ( float flDot, float flDist );
	BOOL CheckRangeAttack1 ( float flDot, float flDist );
	BOOL CheckRangeAttack2 ( float flDot, float flDist );
	
	void EXPORT CustomTouch ( CBaseEntity *pOther );
	
	void MonsterThink( void );

	int Classify(void);
	BOOL isOrganic(void);

	int IRelationship ( CBaseEntity *pTarget );
	
	void ReportAIState( void );

	GENERATE_TRACEATTACK_PROTOTYPE
	GENERATE_TAKEDAMAGE_PROTOTYPE

	GENERATE_DEADTAKEDAMAGE_PROTOTYPE
	
	GENERATE_KILLED_PROTOTYPE
	
	void OnDelete(void);
	void ForgetEnemy(void);
	
	void SetYawSpeed(void);

	BOOL getMonsterBlockIdleAutoUpdate(void);
	BOOL forceIdleFrameReset(void);
	BOOL usesAdvancedAnimSystem(void);
	
	void SetActivity ( Activity NewActivity );

	int LookupActivityHard(int activity);
	int tryActivitySubstitute(int activity);

	void HandleEventQueueEvent(int arg_eventID);
	void HandleAnimEvent(MonsterEvent_t *pEvent );

	float getRunActFramerate(void);
	CBaseEntity* getNearestDeadBody(void);
	void friendly_findCoverFromPlayer( entvars_t* pevPlayerToHideFrom, float flMoveWaitFinishedDelay );

	void attemptAddToShieldSapList(CBaseEntity* argEnt);
	void unlinkShieldSapList(void);
	

	BOOL isProvokable(void);
	BOOL isProvoked(void);
	
	void ScheduleChange( void );
	
	int getHullIndexForNodes(void);
	BOOL needsMovementBoundFix(void);

};//END OF class CFriendly




#endif //END OF #ifdef FRIENDLY_H

