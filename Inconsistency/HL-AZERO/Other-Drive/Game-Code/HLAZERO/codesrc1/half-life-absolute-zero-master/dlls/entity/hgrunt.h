
#ifndef HGRUNT_H
#define HGRUNT_H


#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "basemonster.h"
#include "squadmonster.h"
#include "talkmonster.h"
#include "skill.h"



// was 256.
#define GRENADE_SAFETY_MINIMUM 360



// original was 6.
static float getAIGrenadeCooldown(){
	if(g_iSkillLevel == SKILL_HARD){
		return RANDOM_FLOAT(6, 9);
	}else if(g_iSkillLevel == SKILL_MEDIUM){
		return RANDOM_FLOAT(11, 14);
	}else{  // SKILL_EASY
		return RANDOM_FLOAT(16, 20);
	}
}

// original was random between 2 and 5 on hard, otherwise 6.
static float getAIMP5GrenadeCooldown(){
	if(g_iSkillLevel == SKILL_HARD){
		return RANDOM_FLOAT(5.5, 8);
	}else if(g_iSkillLevel == SKILL_MEDIUM){
		return RANDOM_FLOAT(9.5, 13);
	}else{  // SKILL_EASY
		return RANDOM_FLOAT(15, 18);
	}
}




class CHGrunt : public CSquadMonster
{
public:

	static const char* pGruntSentences[];

	//MODDD
	static const char *pAttackHitSounds[];

	// checking the feasibility of a grenade toss is kind of costly, so we do it every couple of seconds,
	// not every server frame.
	float m_flNextGrenadeCheck;
	float m_flNextPainTime;
	// what.  SquadMonster already has this var you goon
	//float m_flLastEnemySightTime;

	Vector m_vecTossVelocity;
	BOOL m_fThrowGrenade;
	BOOL m_fStanding;
	BOOL m_fFirstEncounter;// only put on the handsign show in the squad's first encounter.
	int	m_cClipSize;
	int m_voicePitch;

	int	m_iBrassShell;
	int	m_iShotgunShell;

	int	m_iSentence;

	int runAndGunSequenceID;
	int tempStrafeAct;

	int strafeMode;
	int idealStrafeMode;
	//-1 = not strafing.
	//0 = strafe left.
	//1 = strafe left + fire.
	//2 = strafe right.
	//3 = strafe right + fire.

	BOOL hgruntMoveAndShootDotProductPass;

	float strafeFailTime;
	float runAndGunFailTime;

	Vector vecPrevOrigin;
	float nextPositionCheckTime;

	BOOL runAndGun;
	Vector lastHeadHit;

	float lastStrafeCoverCheck;

	BOOL missingHeadPossible;
	BOOL friendlyFireStrafeBlock;
	BOOL strafeCanFire;

	BOOL recentChaseFailedAtDistance;
	BOOL shootpref_eyes;

	float allyDeadRecentlyExpireTimeSet;



	CHGrunt(void);

	void EXPORT tempStrafeTouch(CBaseEntity *pOther);
	void EXPORT hgruntUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	int getClipSize(void);

	void updateStrafeStatus(void);
	void onNewRouteNode(void);

	BOOL outOfAmmoStrafeFireBlock(void);
	BOOL hgruntAllowStrafe(void);
	BOOL hgruntAllowStrafeFire(void);
	BOOL getIsStrafeLocked(void);
	BOOL canDoOpportunisticStrafe(void);
	float findCoverInDirection(const Vector& arg_vecStart, const float& arg_vecDistanceCompete, const Vector& arg_inDir, const float& arg_maxDist, Vector* arg_vecDirFeedback);
	float findCoverInDirection(const Vector& arg_vecStart, const float& arg_vecDistanceCompete, const Vector& arg_inDir, const float& arg_maxDist, Vector* arg_vecDirFeedback, BOOL canTryAlternateDegrees);

	void onAnimationLoop(void);

	void MoveExecute( CBaseEntity *pTargetEnt, const Vector &vecDir, float flInterval );

	//MODDD - override here, for testing.
	void MonsterThink(void);

	//MODDD - new. See whether a headshot was dealt to the corpse.
	GENERATE_DEADTAKEDAMAGE_PROTOTYPE

	GENERATE_KILLED_PROTOTYPE

	BOOL usesAdvancedAnimSystem(void);
	int tryActivitySubstitute(int activity);
	int LookupActivityHard(int activity);
	void MakeIdealYaw( Vector vecTarget );
	void StartReanimation(void);

	void moveAnimUpdate(void);

	//MODDD - NEW.  Overriding 
	virtual void StartMonster(void);

	void Spawn( void );
	void Precache( void );
	void SetYawSpeed ( void );
	int  Classify ( void );
	BOOL isOrganic(void){return !CanUseGermanModel();}
	Vector EyePosition(void);
	Vector EyeOffset(void);
	Vector BodyTarget(const Vector &posSrc);
	Vector BodyTargetMod(const Vector &posSrc);
	BOOL getMovementCanAutoTurn(void);

	BOOL getGermanModelRequirement(void);
	const char* getGermanModel(void);
	const char* getNormalModel(void);

	int ISoundMask ( void );

	//MODDD
	void HandleEventQueueEvent(int arg_eventID);

	void HandleAnimEvent( MonsterEvent_t *pEvent );
	BOOL FCanCheckAttacks ( void );
	BOOL CheckMeleeAttack1 ( float flDot, float flDist );
	BOOL CheckRangeAttack1 ( float flDot, float flDist );
	BOOL CheckRangeAttack2 ( float flDot, float flDist );
	void CheckAmmo ( void );
	void SetActivity ( Activity NewActivity );
	void StartTask ( Task_t *pTask );
	void RunTask ( Task_t *pTask );

	void SayAlert(void);
	void AlertSound(void);

	void DeathSound( void );
	void PainSound( void );
	void IdleSound ( void );
	Vector GetGunPosition( void );
	Vector GetGunPositionAI(void);
	void Shoot ( void );
	void Shotgun ( void );
	void PrescheduleThink ( void );

	BOOL DetermineGibHeadBlock(void);

	GENERATE_GIBMONSTER_PROTOTYPE

	void SpeakSentence( void );

	int Save( CSave &save );
	int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];

	//MODDD - moved to CBaseMonster and renamed HumanKick to be callable by other monsters.
	//CBaseEntity	*Kick( void );

	Schedule_t	*GetSchedule( void );
	Schedule_t  *GetScheduleOfType ( int Type );

	//MODDD

	GENERATE_TRACEATTACK_PROTOTYPE
	GENERATE_TAKEDAMAGE_PROTOTYPE

	int IRelationship ( CBaseEntity *pTarget );

	//MODDD - new.
	void SetObjectCollisionBox( void )
	{
		//easyForcePrintLine("I FALLLLLLLLLLLL %d", pev->deadflag);
		if(pev->deadflag != DEAD_NO){
			pev->absmin = pev->origin + Vector(-65, -65, 0);
			pev->absmax = pev->origin + Vector(65, 65, 72);
			//pev->absmin = pev->origin + Vector(-4, -4, 0);
			//pev->absmax = pev->origin + Vector(4, 4, 4);
		}else{
			CSquadMonster::SetObjectCollisionBox();
		}
	}

	BOOL FOkToSpeak( void );
	void JustSpoke( void );

	CUSTOM_SCHEDULES;

	//MODDD - see comments below at implementation.
	//int SquadRecruit( int searchRadius, int maxMembers );

	// PlayerUse  uses this to judge whether or not this is worth sending a "touch" to.
	int ObjectCaps( void ) { return CSquadMonster::ObjectCaps() | FCAP_IMPULSE_USE; }

	BOOL canResetBlend0(void);
	BOOL onResetBlend0(void);


	void AimAtEnemy(Vector& refVecShootOrigin, Vector& refVecShootDir);
	Vector HGRUNT_ShootAtEnemyEyes(const Vector& shootOrigin);

	void checkHeadGore(void);
	void checkHeadGore(int iGib );
	void SayGrenadeThrow(void);

	float getDistTooFar(void);
	void setEnemyLKP(CBaseEntity* theEnt);

};



#endif //HGRUNT_H
