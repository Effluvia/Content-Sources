//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		Base combat character with no AI
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================//

#ifndef NPC_BULLSQUID_H
#define NPC_BULLSQUID_H

#include "ai_basenpc.h"

#ifdef HOE_DLL
struct UnshootableEnt_t
{
	EHANDLE	hUnshootableEnt;	// Entity that's we couldn't find weapon los to
	float	fExpireTime;		// Time to forget this information
	Vector	vLocationWhenUnshootable;
	
	DECLARE_SIMPLE_DATADESC();
};
#endif

class CNPC_Bullsquid : public CAI_BaseNPC
{
	DECLARE_CLASS( CNPC_Bullsquid, CAI_BaseNPC );
	DECLARE_DATADESC();

public:
	void Spawn( void );
	void Precache( void );
	Class_T	Classify( void );
#ifdef HOE_DLL
	bool ClassifyPlayerAlly( void ) { return false; }
	bool ClassifyPlayerAllyVital( void ) { return false; }
#endif
	void IdleSound( void );
	void PainSound( const CTakeDamageInfo &info );
	void AlertSound( void );
	void DeathSound( const CTakeDamageInfo &info );
	void AttackSound( void );
	void GrowlSound( void );

	float MaxYawSpeed ( void );

	void HandleAnimEvent( animevent_t *pEvent );
#ifdef HOE_DLL
	bool GetSpitVector( const Vector &vecStartPos, const Vector &vecTarget, Vector *vecOut );
	int GetTailWhipDamage( CBaseEntity *pVictim, int *adjustedDamage = NULL );

	bool SeenEnemyWithinTime( float flTime );

	virtual float	InnateRange1MinRange( void ) { return 100/*85*/; }
	virtual float	InnateRange1MaxRange( void ) { return 1024; }
	bool InnateWeaponLOSCondition( const Vector &ownerPos, const Vector &targetPos, bool bSetConditions );
#endif // HOE_DLL

	int RangeAttack1Conditions( float flDot, float flDist );
	int MeleeAttack1Conditions( float flDot, float flDist );
	int MeleeAttack2Conditions( float flDot, float flDist );

	bool FValidateHintType ( CAI_Hint *pHint );
	void RemoveIgnoredConditions( void );
#ifdef HOE_DLL
	int IRelationPriority( CBaseEntity *pTarget );
#endif // HOE_DLL
	Disposition_t IRelationType( CBaseEntity *pTarget );
	int OnTakeDamage_Alive( const CTakeDamageInfo &inputInfo );
#ifdef HOE_DLL
	void Event_Killed( const CTakeDamageInfo &info );
#endif

	int GetSoundInterests ( void );
	void RunAI ( void );
	virtual void OnListened ( void );

#ifdef HOE_DLL
	void RememberUnshootable( CBaseEntity *pEntity, float flDuration = -1 );
	bool IsUnshootable( CBaseEntity *pEntity );
	CUtlVector<UnshootableEnt_t> m_UnshootableEnts;

	bool ExpensiveUnreachableCheck( CBaseEntity *pEntity );
	bool IsUnreachable( CBaseEntity *pEntity );
	void GatherConditions( void );
	int TranslateSchedule( int scheduleType );
	int SelectFailSchedule( int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode );

	void BuildScheduleTestBits( void );
#endif // HOE_DLL
	int SelectSchedule( void );
	bool FInViewCone ( Vector pOrigin );

	void StartTask ( const Task_t *pTask );
	void RunTask ( const Task_t *pTask );

	NPC_STATE SelectIdealState ( void );

	DEFINE_CUSTOM_AI;

private:
	
	bool  m_fCanThreatDisplay;// this is so the squid only does the "I see a headcrab!" dance one time. 
	float m_flLastHurtTime;// we keep track of this, because if something hurts a squid, it will forget about its love of headcrabs for a while.
	float m_flNextSpitTime;// last time the bullsquid used the spit attack.
#ifndef HOE_DLL
	int   m_nSquidSpitSprite;
#endif
	float m_flHungryTime;// set this is a future time to stop the monster from eating for a while. 

	float m_nextSquidSoundTime;
#ifdef HOE_DLL
	Vector	m_vecSaveSpitVelocity;	// Saved when we start to attack and used if we failed to get a clear shot once we release
	float m_flPainTime;
	float m_flWakeAngryTime;
#endif
};
#endif // NPC_BULLSQUID_H