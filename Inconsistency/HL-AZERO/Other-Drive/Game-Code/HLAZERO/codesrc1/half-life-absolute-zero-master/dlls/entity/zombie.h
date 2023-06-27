//MODDD - new file, for other places to include to rip its hit/miss arrays.

#ifndef ZOMBIE_H
#define ZOMBIE_H

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "basemonster.h"


EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST(sv_germancensorship)


class CZombie : public CBaseMonster
{
public:
	static const char* pAttackSounds[];
	static const char* pIdleSounds[];
	static const char* pAlertSounds[];
	static const char* pPainSounds[];
	static const char* pAttackHitSounds[];
	static const char* pAttackMissSounds[];

	//MODDD
	EHANDLE corpseToSeek;
	EHANDLE m_hEnemy_CopyRef;
	float lookForCorpseTime;
	float nextCorpseCheckTime;
	float flPushbackForceDamage;


	CZombie();

	//MODDD
	CUSTOM_SCHEDULES;

	void Spawn(void);
	void Precache(void);
	void SetYawSpeed(void);
	int  Classify(void);
	void HandleAnimEvent(MonsterEvent_t* pEvent);
	int IgnoreConditions(void);

	float m_flNextFlinch;

	void PainSound(void);
	void AlertSound(void);
	void IdleSound(void);
	void AttackSound(void);

	//MODDD
	void SetObjectCollisionBox(void)
	{
		if (pev->deadflag != DEAD_NO) {
			pev->absmin = pev->origin + Vector(-68, -68, 0);
			pev->absmax = pev->origin + Vector(68, 68, 66);
		}
		else {
			CBaseMonster::SetObjectCollisionBox();
		}
	}

	//MODDD - new new new.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void MonsterThink(void);

	Schedule_t* GetSchedule();
	Schedule_t* GetScheduleOfType(int Type);

	void StartTask(Task_t* pTask);
	void RunTask(Task_t* pTask);

	BOOL getMonsterBlockIdleAutoUpdate(void);
	BOOL forceIdleFrameReset(void);
	BOOL usesAdvancedAnimSystem(void);

	void SetActivity(Activity NewActivity);

	int tryActivitySubstitute(int activity);
	int LookupActivityHard(int activity);

	void HandleEventQueueEvent(int arg_eventID);

	int CheckLocalMove(const Vector& vecStart, const Vector& vecEnd, CBaseEntity* pTarget, BOOL doZCheck, float* pflDist);// check validity of a straight move through space

	int ISoundMask(void);

	void EXPORT ZombieTouch(CBaseEntity* pOther);

	CBaseEntity* getNearestDeadBody(Vector argSearchOrigin, float argMaxDist);

	void ScheduleChange(void);

	void OnCineCleanup(CCineMonster* pOldCine);


	Vector BodyTarget(const Vector& posSrc);
	Vector BodyTargetMod(const Vector& posSrc) {
		//redirect to our own BodyTarget method for convenience.
		return CZombie::BodyTarget(posSrc);
	};
	////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// No range attacks
	BOOL CheckRangeAttack1(float flDot, float flDist) { return FALSE; }
	BOOL CheckRangeAttack2(float flDot, float flDist) { return FALSE; }

	GENERATE_TRACEATTACK_PROTOTYPE
	void TraceAttack_Traceless(entvars_t* pevAttacker, float flDamage, Vector vecDir, int bitsDamageType, int bitsDamageTypeMod);
	GENERATE_TAKEDAMAGE_PROTOTYPE
	//MODDD - NEW.
	float hitgroupDamage(float flDamage, int bitsDamageType, int bitsDamageTypeMod, int iHitgroup);

	virtual int BloodColor(void) {
		// if german censorship, all of the body has green blood.  
		// Otherwise, only the head(crab) does.
		if (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(sv_germancensorship) == 1 || m_LastHitGroup == HITGROUP_HEAD) {
			// standard
			return BLOOD_COLOR_GREEN;
		}
		else {
			// the human body portion
			return BLOOD_COLOR_RED;
		}
	}

	GENERATE_KILLED_PROTOTYPE
	void BecomeDead(void);

};

#endif //ZOMBIE_H
