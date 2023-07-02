
#ifndef SCIENTIST_H
#define SCIENTIST_H

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "basemonster.h"
#include "talkmonster.h"



//=======================================================
// Scientist
//=======================================================

class CScientist : public CTalkMonster
{
private:
	float m_painTime;
	float m_healTime;
	float m_fearTime;
	float explodeDelay;
public:

	static const char* pDeathSounds[];
	static const char* madInterSentences[];
	static int madInterSentencesMax;
	static int numberOfModelBodyParts;
	//MODDD - new data stuff.
	static const char* pAttackHitSounds[];
	static const char* pAttackMissSounds[];
	float aggro;
	float aggroCooldown;
	float playFearAnimCooldown;
	Vector aggroOrigin;
	BOOL healNPCChosen;
	int trueBody;
	float healNPCCheckDelay;
	// Yes, a cooldown just for screaming, separate from the general talk one.
	float screamCooldown;
	float nextRandomSpeakCheck;

	//MODDD - the wounded NPC to seek out.
	// ...or not using this var anymore, whoops
	//CBaseMonster* healTargetNPC;


	CScientist(void);
	int getMadSentencesMax(void);
	int getMadInterSentencesMax(void);

	void forgetHealNPC(void);
	//MODDD
	void StartFollowing(CBaseEntity* pLeader);

	//MODDD
	void Activate(void);
	void Spawn(void);
	void Precache(void);
	void SetYawSpeed(void);
	int  Classify(void);
	BOOL isOrganic(void) { return !CanUseGermanModel(); }

	void HandleAnimEvent(MonsterEvent_t* pEvent);
	void RunTask(Task_t* pTask);
	void StartTask(Task_t* pTask);
	int ObjectCaps(void) { return CTalkMonster::ObjectCaps() | FCAP_IMPULSE_USE; }

	//MODDD
	GENERATE_TRACEATTACK_PROTOTYPE
	GENERATE_TAKEDAMAGE_PROTOTYPE

	virtual void OnAlerted(BOOL alerterWasKilled);


	virtual int FriendNumber(int arrayNumber);
	void SetActivity(Activity newActivity);
	Activity GetStoppedActivity(void);
	int ISoundMask(void);
	void DeclineFollowing(void);

	// Need more room for cover because scientists want to get far away!
	float CoverRadius(void) { return DEFAULT_COVER_SEEK_DISTANCE * 1.8; }

	//MODDD MAJOR - DEBUG. NO.
	// wanted to bring back again at some point, right?   And let's make it 20 instead anyway.  USED TO BE 15 for when to forget
	BOOL DisregardEnemy(CBaseEntity* pEnemy) { return !pEnemy->IsAlive() || (gpGlobals->time - m_fearTime) > 20; }
	//BOOL DisregardEnemy( CBaseEntity *pEnemy ) { return FALSE; }

	BOOL CanHeal(void);
	//MODDD
	BOOL CanHeal(CBaseMonster* arg_monsterTry);

	void Heal(void);
	void Scream(void);

	//MODDD
	void ScheduleChange(void);

	// Override these to set behavior
	Schedule_t* GetScheduleOfType(int Type);
	Schedule_t* GetSchedule(void);
	MONSTERSTATE GetIdealState(void);

	//MODDD - new.
	void SetObjectCollisionBox(void) {
		if (pev->deadflag != DEAD_NO) {
			pev->absmin = pev->origin + Vector(-65, -65, 0);
			pev->absmax = pev->origin + Vector(65, 65, 72);
		}
		else {
			CBaseMonster::SetObjectCollisionBox();
		}
	}

	void MonsterThink(void);

	void setModel(void);
	void setModel(const char* m);
	BOOL getGermanModelRequirement(void);
	const char* getGermanModel(void);
	const char* getNormalModel(void);

	int IRelationship(CBaseEntity* pTarget);

	void DeathSound(void);
	void PainSound(void);
	void PainSound_Play(void);

	void TalkInit(void);

	void AlertSound(void);

	GENERATE_KILLED_PROTOTYPE

	virtual int	Save(CSave& save);
	virtual int	Restore(CRestore& restore);
	static TYPEDESCRIPTION m_SaveData[];

	void PostRestore(void);

	CUSTOM_SCHEDULES;

	BOOL CheckMeleeAttack1(float flDot, float flDist);
	BOOL CheckMeleeAttack2(float flDot, float flDist);

	void HandleEventQueueEvent(int arg_eventID);
	BOOL usesAdvancedAnimSystem(void);
	int tryActivitySubstitute(int activity);
	int LookupActivityHard(int activity);

	void DeclineFollowingProvoked(CBaseEntity* pCaller);
	void SayHello(CBaseEntity* argPlayerTalkTo);
	void SayIdleToPlayer(CBaseEntity* argPlayerTalkTo);
	void SayQuestion(CTalkMonster* argTalkTo);

	void SayAlert(void);
	void SayDeclineFollowing(void);
	void SayDeclineFollowingProvoked(void);
	void SayFear(void);
	void SayProvoked(void);
	void SayStopShooting(void);
	void SaySuspicious(void);
	void SayLeaderDied(void);
	void SayKneel(void);
	void SayNearPassive(void);
	void SayNearCautious(void);

	void ReportAIState(void);

	BOOL violentDeathAllowed(void);
	BOOL violentDeathClear(void);
	int violentDeathPriority(void);
	BOOL canPredictActRepeat(void);

	void OnPlayerDead(CBasePlayer* arg_player);
	void OnPlayerFollowingSuddenlyDead(void);

	void initiateAss(void);
	void myAssHungers(void);


};






//=========================================================
// Dead Scientist PROP
//=========================================================
class CDeadScientist : public CBaseMonster
{
public:
	CDeadScientist(void);

	void Activate(void);
	void Spawn(void);
	int Classify(void) { return	CLASS_HUMAN_PASSIVE; }
	BOOL isOrganic(void) { return !CanUseGermanModel(); }

	//MODDD
	virtual int	Save(CSave& save);
	virtual int	Restore(CRestore& restore);
	static	TYPEDESCRIPTION m_SaveData[];

	void PostRestore(void);

	//MODDD
	void setModel(void);
	void setModel(const char* m);
	BOOL getGermanModelRequirement(void);
	const char* getGermanModel(void);
	const char* getNormalModel(void);

	static int numberOfModelBodyParts;

	void KeyValue(KeyValueData* pkvd);
	int m_iPose;// which sequence to display
	static char* m_szPoses[7];

	//MODDD
	int trueBody;

};





//=========================================================
// Sitting Scientist PROP
//=========================================================

class CSittingScientist : public CScientist // kdb: changed from public CBaseMonster so he can speak
{
public:
	CSittingScientist(void);

	void Activate( void );
	void Spawn( void );
	void Precache( void );

	void EXPORT SittingThink( void );
	int Classify ( void );
	virtual int	Save( CSave &save );
	virtual int	Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];

	void PostRestore(void);

	virtual void SetAnswerQuestion( CTalkMonster *pSpeaker );
	int FriendNumber( int arrayNumber );
	
	
	GENERATE_TRACEATTACK_PROTOTYPE
	GENERATE_TAKEDAMAGE_PROTOTYPE


	int FIdleSpeak ( void );
	int	m_baseSequence;
	int	m_headTurn;
	float m_flResponseDelay;
};




#endif //SCIENTIST_H
