
//=========================================================
// STUKABAT (file modeled after controller.cpp and flyingmonster.cpp)
//=========================================================

#ifndef STUKABAT_H
#define STUKABAT_H

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "basemonster.h"
#include "squadmonster.h"




#define PRINTQUEUE_DECLARATION(QUEUENAME) PrintQueue QUEUENAME;
#define PRINTQUEUE_NAME(QUEUENAME, DISPLAY) QUEUENAME(PrintQueue(DISPLAY))
#define PRINTQUEUE_CLEAR(QUEUENAME) QUEUENAME.clearPrintQueue();
#define PRINTQUEUE_PRINTOUT(QUEUENAME) QUEUENAME.receivePrintQueue(finalDest, &finalDestLocation);

//this->stukaPrint.tookDamage.sendToPrintQueue("TOOK DMG");



class StukaPrintQueueManager{
public:
	PRINTQUEUE_DECLARATION(pathFinding)
	PRINTQUEUE_DECLARATION(getSchedule)
	PRINTQUEUE_DECLARATION(setActivity)
	PRINTQUEUE_DECLARATION(enemyInfo)
	PRINTQUEUE_DECLARATION(general)
	PRINTQUEUE_DECLARATION(tookDamage)
	PRINTQUEUE_DECLARATION(eatRelated)
	PRINTQUEUE_DECLARATION(soundRelated)
	PRINTQUEUE_DECLARATION(moveRelated)
	
	char displayName[PRINTQUEUE_NAMESTRINGSIZE];

	StukaPrintQueueManager(const char* arg_name):
		//pathFinding(PrintQueue("pthfnd") ),
		//getSchedule(PrintQueue("getsch") ),
		//setActivity(PrintQueue("setact") )
		PRINTQUEUE_NAME(pathFinding, "PTHFND"),
		PRINTQUEUE_NAME(getSchedule, "GETSCH"),
		PRINTQUEUE_NAME(setActivity, "SETACT"),
		PRINTQUEUE_NAME(enemyInfo, "ENEINF"),
		PRINTQUEUE_NAME(general, "GENERL"),
		PRINTQUEUE_NAME(tookDamage, "TOOKDMG"),
		PRINTQUEUE_NAME(eatRelated, "EATREL"),
		PRINTQUEUE_NAME(soundRelated, "SNDREL"),
		PRINTQUEUE_NAME(moveRelated, "MOVREL")
		
	{

		int nameLength = lengthOfString(arg_name, PRINTQUEUE_NAMESTRINGSIZE);
		strncpyTerminate(&displayName[0], arg_name, nameLength);

		//not much else?
	}

	void clearAll(){
		PRINTQUEUE_CLEAR(pathFinding);
		PRINTQUEUE_CLEAR(getSchedule);
		PRINTQUEUE_CLEAR(setActivity);
		PRINTQUEUE_CLEAR(enemyInfo);
		PRINTQUEUE_CLEAR(general);
		PRINTQUEUE_CLEAR(tookDamage);
		PRINTQUEUE_CLEAR(eatRelated);
		PRINTQUEUE_CLEAR(soundRelated);
		PRINTQUEUE_CLEAR(moveRelated);
	}
	

	#define PRINTQUEUE_STUKA_NUMBEROF 8
	//No, varries per setup.

	void printOutAll(){
		int finalDestLocation = 0;
		const int max = PRINTQUEUE_TOTALEXPECTED * PRINTQUEUE_STUKA_NUMBEROF + ((PRINTQUEUE_NAMESTRINGSIZE+1) * (PRINTQUEUE_STUKA_NUMBEROF + 1) ) ;
		char finalDest[max];

		appendTo(finalDest, displayName, &finalDestLocation, ':');
		
		PRINTQUEUE_PRINTOUT(pathFinding)
		PRINTQUEUE_PRINTOUT(getSchedule)
		PRINTQUEUE_PRINTOUT(setActivity)
		PRINTQUEUE_PRINTOUT(enemyInfo)
		PRINTQUEUE_PRINTOUT(general)
		PRINTQUEUE_PRINTOUT(tookDamage)
		PRINTQUEUE_PRINTOUT(eatRelated)
		PRINTQUEUE_PRINTOUT(soundRelated)
		PRINTQUEUE_PRINTOUT(moveRelated)

		//pathFinding.receivePrintQueue(finalDest, &finalDestLocation);
		//getSchedule.receivePrintQueue(finalDest, &finalDestLocation);
		//setActivity.receivePrintQueue(finalDest, &finalDestLocation);
		finalDest[finalDestLocation] = '\0';

		easyPrintLine(finalDest);
		//cout << "~USED: " << finalDestLocation << " AVAILABLE: " << max << endl;
	}

};





class CStukaBat: public CSquadMonster
{
public:

	virtual int	Save( CSave &save );
	virtual int	Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];
	

	StukaPrintQueueManager stukaPrint;

	
	BOOL wakeUp;
	BOOL eating;
	float eatingAnticipatedEnd;
	BOOL turnThatOff;

	BOOL tempCheckTraceLineBlock;


	float blockSetActivity;
	Activity lastSetActivitySetting;
	BOOL lastSetActivityforceReset;

	float moveFlyNoInterrupt;

	int iPoisonSprite;

	Vector m_vecEstVelocity;

	Vector m_velocity;
	int m_fInCombat;

	int tempThing;
	BOOL landBrake;
	Vector scentLocationMem;

	Activity recentActivity;
	int attackIndex;

	float attackEffectDelay;
	float attackAgainDelay;
	float maxDiveTime;

	float timeToIdle;

	int m_iSpawnLoc;

	BOOL combatCloseEnough;

	BOOL onGround;
	BOOL queueToggleGround;
	BOOL snappedToCeiling;
	BOOL queueToggleSnappedToCeiling;
	BOOL queueAbortAttack;

	int queueActionIndex;

	BOOL seekingFoodOnGround;
	float lastVelocityChange;

	BOOL rotationAllowed;
	BOOL dontResetActivity;

	float suicideAttackCooldown;

	float lastEnemey2DDistance;
	//int movementHint;

	int chargeIndex;
	int m_voicePitch;



	static const char *pAttackHitSounds[];
	static const char *pAttackMissSounds[];
	static const char *pAttackSounds[];
	static const char *pIdleSounds[];
	static const char *pAlertSounds[];
	static const char *pPainSounds[];
	static const char *pDeathSounds[];



	CStukaBat(void);
	
	float getMeleeAnimScaler(void);
	float getAttackDelay(void);
	void checkStartSnap();

	void Activate(void);
	void DefaultSpawnNotice(void);
	void ForceSpawnFlag(int arg_spawnFlag);



	void MakeIdealYaw( Vector vecTarget );
	virtual float ChangeYaw ( int speed );
		
	void ForceMakeIdealYaw( Vector vecTarget );
	virtual float ForceChangeYaw ( int speed );



	void CallForHelp( char *szClassname, float flDist, EHANDLE hEnemy, Vector &vecLocation );

	int IRelationship( CBaseEntity *pTarget );
	void Spawn( void );
	void Precache( void );
	void SetYawSpeed( void );
	int  Classify ( void );
	//MODDD
	void KeyValue( KeyValueData *pkvd );
	void HandleAnimEvent( MonsterEvent_t *pEvent );

	void RunAI( void );
	BOOL CheckRangeAttack1 ( float flDot, float flDist );
	BOOL CheckRangeAttack2 ( float flDot, float flDist );
	BOOL CheckMeleeAttack1 ( float flDot, float flDist );
	
	BOOL getHasPathFindingModA();
	BOOL getHasPathFindingMod();



	//NOTE: confirm that the original "setObjectCollisionBox" method (default for monsters) is inadequate before using this override.
	/*
	void SetObjectCollisionBox( void )
	{
		pev->absmin = pev->origin + Vector( -80, -80, 0 );
		pev->absmax = pev->origin + Vector( 80, 80, 214 );
	}

	void SetObjectCollisionBox( void )
	{
		if(pev->deadflag != DEAD_NO){
			pev->absmin = pev->origin + Vector(-64, -64, 0);
			pev->absmax = pev->origin + Vector(64, 64, 50);
		}else{

			CBaseMonster::SetObjectCollisionBox();

		}
	}

	*/
	void SetObjectCollisionBox( void )
	{
		if(pev->deadflag != DEAD_NO){
			pev->absmin = pev->origin + Vector(-32, -32, 0 );
			pev->absmax = pev->origin + Vector(32, 32, 42);
		}else{
			pev->absmin = pev->origin + Vector(-28, -28, 0 );
			pev->absmax = pev->origin + Vector(28, 28, 54);
			//CBaseMonster::SetObjectCollisionBox();
		}
	}

	Schedule_t* GetSchedule ( void );
	Schedule_t* GetScheduleOfType ( int Type );
	void StartTask ( Task_t *pTask );
	void RunTask ( Task_t *pTask );
	CUSTOM_SCHEDULES;

	void Stop( void );
	void Move ( float flInterval );

	int CheckLocalMove( const Vector &vecStart, const Vector &vecEnd, CBaseEntity *pTarget, BOOL doZCheck, float *pflDist );
	
	void MoveExecute( CBaseEntity *pTargetEnt, const Vector &vecDir, float flInterval );
	
	virtual BOOL allowedToSetActivity(void);
	virtual void SetActivity ( Activity NewActivity );
	virtual void SetActivity ( Activity NewActivity, BOOL forceReset );

	BOOL ShouldAdvanceRoute( float flWaypointDist, float flInterval );
	int LookupFloat( );
	
	int ISoundMask( void );
	
	void RadiusDamageNoFriendly(entvars_t* pevInflictor, entvars_t*	pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType );
	
	void safeSetMoveFlyNoInterrupt(float timer);
	void safeSetBlockSetActivity(float timer);


	//???
	//float m_flShootTime;
	//float m_flShootEnd;

	void PainSound( void );
	void AlertSound( void );
	void IdleSound( void );
	void AttackSound( void );
	void DeathSound( void );

	
	GENERATE_TRACEATTACK_PROTOTYPE
	GENERATE_TAKEDAMAGE_PROTOTYPE


	void setAnimation(char* animationName);
	void setAnimation(char* animationName, BOOL forceException);
	void setAnimation(char* animationName, BOOL forceException, BOOL forceLoopsProperty);
	void setAnimation(char* animationName, BOOL forceException, BOOL forceLoopsProperty, int extraLogic);
		
	
	void tryDetachFromCeiling(void);


	GENERATE_KILLED_PROTOTYPE
	
	GENERATE_GIBMONSTER_PROTOTYPE

	virtual BOOL isProvokable(void);
	virtual BOOL isProvoked(void);
	virtual void heardBulletHit(entvars_t* pevShooter);

	//DO NOT FORGET "EXPORT"!
	void EXPORT customTouch(CBaseEntity *pOther);
	

	float MoveYawDegreeTolerance(void);
	

	BOOL usesAdvancedAnimSystem(void);

	//void SetActivity ( Activity NewActivity );

	int LookupActivityHard(int activity);
	int tryActivitySubstitute(int activity);


	//MODDD - extra.
	void MonsterThink();

	Activity GetStoppedActivity( void );
	void SetTurnActivity();
	void SetTurnActivityCustom();



	void updateMoveAnim();
	void getPathToEnemyCustom();
	void abortAttack();

	void checkTraceLine(const Vector& vecSuggestedDir, const float& travelMag, const float& flInterval, const Vector& vecStart, const Vector& vecRelativeEnd, const int& moveDist);
	void checkTraceLine(const Vector& vecSuggestedDir, const float& travelMag, const float& flInterval, const Vector& vecStart, const Vector& vecRelativeEnd, const int& moveDist, const BOOL canBlockFuture);
	
	void checkTraceLineTest(const Vector& vecSuggestedDir, const float& travelMag, const float& flInterval, const Vector& vecStart, const Vector& vecRelativeEnd, const int& moveDist);
	void checkTraceLineTest(const Vector& vecSuggestedDir, const float& travelMag, const float& flInterval, const Vector& vecStart, const Vector& vecRelativeEnd, const int& moveDist, const BOOL canBlockFuture);
	
	void checkFloor(const Vector& vecSuggestedDir, const float& travelMag, const float& flInterval);

	EHANDLE* getEnemy();

	void ReportAIState(void);

	Schedule_t* GetStumpedWaitSchedule(void);


	int getLoopingDeathSequence(void);

	
	BOOL violentDeathAllowed(void);
	BOOL violentDeathDamageRequirement(void);
	BOOL violentDeathClear(void);
	int violentDeathPriority(void);

	Activity getIdleActivity(void);




	float HearingSensitivity( );

	int getNodeTypeAllowed(void);
	int getHullIndexForNodes(void);
	int getHullIndexForGroundNodes(void);

};


#endif //STUKABAT_H