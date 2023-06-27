
//=========================================================
// FLOATER (floater)
//=========================================================

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "basemonster.h"
#include "flyingmonster.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef FLOATER_H
#define FLOATER_H


class CFloater : public CFlyingMonster{
public:

	static const char* pDeathSounds[];
	static const char* pAlertSounds[];
	static const char* pIdleSounds[];
	static const char* pPainSounds[];
	static const char* pAttackSounds[];
	static const char* pAttackHitSounds[];
	static const char* pAttackMissSounds[];

	
	float explodeDelay;
	float m_flightSpeed;
	BOOL tempCheckTraceLineBlock;
	Vector m_velocity;
	float lastVelocityChange;

	float shootCooldown;
	BOOL explodedYet;
	
	//save info
	//////////////////////////////////////////////////////////////////////////////////
	static TYPEDESCRIPTION m_SaveData[];
	virtual int Save( CSave &save ); 
	virtual int Restore( CRestore &restore );
	//////////////////////////////////////////////////////////////////////////////////
	

	CFloater(void);

	CUSTOM_SCHEDULES;



	void DeathSound ( void );
	void AlertSound ( void );
	void IdleSound ( void );
	void PainSound ( void );
	void AttackSound ( void );
	
	void Precache(void);
	void Spawn(void);

	Activity	GetStoppedActivity( void );
	void	Stop( void );

	int CheckLocalMove ( const Vector &vecStart, const Vector &vecEnd, CBaseEntity *pTarget, BOOL doZCheck, float *pflDist );
	
	void Move( float flInterval );
	BOOL ShouldAdvanceRoute( float flWaypointDist, float flInterval );
	void MoveExecute( CBaseEntity *pTargetEnt, const Vector &vecDir, float flInterval );

	void SetEyePosition(void);
	
	Schedule_t *GetSchedule( void );
	Schedule_t* GetScheduleOfType( int Type);

	void ScheduleChange();
	Schedule_t* GetStumpedWaitSchedule(void);
	
	void StartTask( Task_t *pTask );
	void RunTask( Task_t *pTask );

	
	BOOL CheckMeleeAttack1 ( float flDot, float flDist );
	BOOL CheckMeleeAttack2 ( float flDot, float flDist );
	BOOL CheckRangeAttack1 ( float flDot, float flDist );
	BOOL CheckRangeAttack2 ( float flDot, float flDist );
	
	void EXPORT CustomTouch ( CBaseEntity *pOther );
	
	void MonsterThink( void );
	void PrescheduleThink(void);

	int Classify(void);
	BOOL isOrganic(void);
	int IRelationship ( CBaseEntity *pTarget );
	
	void ReportAIState( void );

	//NOTICE - make these the "_VIRTUAL" versions if this monster could possibly have child classes made.
	//         Such as, the CTalkMonster having child classes CBarney and CScientist.
	GENERATE_TRACEATTACK_PROTOTYPE
	GENERATE_TAKEDAMAGE_PROTOTYPE

	GENERATE_DEADTAKEDAMAGE_PROTOTYPE
	
	GENERATE_GIBMONSTER_PROTOTYPE

	GENERATE_GIBMONSTERGIB_PROTOTYPE

	//uncomment and implement these if needed. The defaults are good for most cases.
	//GENERATE_GIBMONSTERSOUND_PROTOTYPE
	//GENERATE_GIBMONSTEREND_PROTOTYPE

	GENERATE_KILLED_PROTOTYPE

	void SetYawSpeed(void);

	BOOL getMonsterBlockIdleAutoUpdate(void);
	BOOL forceIdleFrameReset(void);
	BOOL usesAdvancedAnimSystem(void);
	


	void SetActivity ( Activity NewActivity );

	int LookupActivityHard(int activity);
	int tryActivitySubstitute(int activity);

	void HandleEventQueueEvent(int arg_eventID);
	void HandleAnimEvent(MonsterEvent_t *pEvent );
	



	void checkTraceLine(const Vector& vecSuggestedDir, const float& travelMag, const float& flInterval, const Vector& vecStart, const Vector& vecRelativeEnd, const int& moveDist);
	void checkTraceLine(const Vector& vecSuggestedDir, const float& travelMag, const float& flInterval, const Vector& vecStart, const Vector& vecRelativeEnd, const int& moveDist, const BOOL canBlockFuture);
	
	void checkTraceLineTest(const Vector& vecSuggestedDir, const float& travelMag, const float& flInterval, const Vector& vecStart, const Vector& vecRelativeEnd, const int& moveDist);
	void checkTraceLineTest(const Vector& vecSuggestedDir, const float& travelMag, const float& flInterval, const Vector& vecStart, const Vector& vecRelativeEnd, const int& moveDist, const BOOL canBlockFuture);
	
	void checkFloor(const Vector& vecSuggestedDir, const float& travelMag, const float& flInterval);

	int getLoopingDeathSequence(void);

	Vector BodyTarget(const Vector &posSrc);
	Vector BodyTargetMod(const Vector &posSrc);

	void onDeathAnimationEnd(void);
	int getNodeTypeAllowed(void);

	int getHullIndexForNodes(void);
	int getHullIndexForGroundNodes(void);

	
	void SetObjectCollisionBox( void ){

		if(pev->deadflag != DEAD_NO){
			//no need to do anything special anymore I think.
			//CBaseMonster::SetObjectCollisionBox();
			pev->absmin = pev->origin + Vector(-26, -26, 0);
			pev->absmax = pev->origin + Vector(26, 26, 36);
		}else{
			// = DEAD_NO
			pev->absmin = pev->origin + Vector(-13, -13, 0);
			pev->absmax = pev->origin + Vector(13, 13, 52);
		}
		
	}//END OF SetObjectCollisionBox




};//END OF class CFloater






#endif //END OF #ifdef FLOATER_H






