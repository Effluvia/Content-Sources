


#ifndef CHUMTOAD_H
#define CHUMTOAD_H

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "basemonster.h"

//v_chub.mdl
//chumtoad.mdl
//w_sqknest.mdl

//NOTES ABOUT SNARK:
//class of weapon and nest (pickup-able): CSqueak
//class of spawned creature on throw: CSqueakGrenade

//TODO: make CSQueak nest use the "Walk" animation and travel a short distance from spawn in random directions?

/*

chumtoad small walking npc that flees from preys, should attract bullsquids and agrunts - maps going to be featured in (c1a1 onwards) + weapon (should behave like the snark weapon)
    world pickable entity should roam around (or make it being able to follow paths like monster_generics) apply the same for the snarks counterpart
    note: add blinking system on both the npc and player weapon, also apply to snarks
    should feign death if the hunter npc gets real close, faking death should have a 50% chance of making the prey to buying it, and thus scaring the hunter away (or make it chase another chum or entity, including the player)(edited)
	*/



//or a parent of "CGrenade" like the CSqueakGrenade does?

class CChumToad : public CBaseMonster{


	/*
	void SetObjectCollisionBox( void )
	{
		//EASY_CVAR_PRINTIF_PRE(gargantuaPrintout, easyPrintLine( "YOU indubitable father-ignorant child %d", pev->deadflag));
		//could it be re-adjusted for "DEAD_DEAD" too?
			//if we are dead?
			pev->absmin = pev->origin + Vector( -6, -6, 0 );
			pev->absmax = pev->origin + Vector( 6, 6, 6 );
	}
	*/

    public:

	float testTimer;
	EHANDLE m_hEntitySittingOn;


	static const char* pDeathSounds[];
	static const char* pAlertSounds[];
	static const char* pIdleSounds[];
	static const char* pPainSounds[];



	Vector vecHopDest;

	BOOL initFall;
	BOOL generalFall;
	float previousZ;


	float forceStopInitFallTimer;
	float landTimer;
	float stopHopDelay;
	float delayTimer;
	float passiveCroakDelay;
	float playDeadForbiddenTimer;
	float panicTimer;

	float toadPlayDeadTimer;
	float toadPlayDeadAnimationTimer;

	float playDeadSendoffTimer;


	int m_iMyClass;
	static int numberOfEyeSkins;

	EHANDLE m_hOwner;

	BOOL playerFriend;
	BOOL playerAllyFriend;

	BOOL playDeadSuccessful;



    CChumToad(void);
	BOOL usesSoundSentenceSave(void);


	int ObjectCaps(void) { return CBaseMonster::ObjectCaps() | FCAP_IMPULSE_USE; }

	void HandleEventQueueEvent(int arg_eventID);

	void SetYawSpeed(void);
	BOOL getMonsterBlockIdleAutoUpdate(void);
	int Classify(void);
	BOOL getForceAllowNewEnemy(CBaseEntity* pOther);



	//save info
	//////////////////////////////////////////////////////////////////////////////////
	virtual int	Save( CSave &save ); 
	virtual int	Restore( CRestore &restore );
	
	static	TYPEDESCRIPTION m_SaveData[];
	//////////////////////////////////////////////////////////////////////////////////
	
	void setModel(void);
	void setModel(const char* m);

	void Spawn(void);
	void Precache(void);

	void firstLand(void);
	void Land(void);

	void forwardHop(void);
	void aimlessHop(void);
	void randomDelay(void);

	BOOL playDeadFooling(CBaseEntity* whoWantsToKnow);



	void onPlayDead();
	void playDeadSendMonstersAway();
	void onDeathAnimationEnd(void);

	CBaseEntity* getEntityBelow(void);


	BOOL IsAlive_FromAI( CBaseMonster* whoWantsToKnow );
	
	void ReportAIState( void );

	BOOL isForceHated(CBaseEntity *pBy);
	int forcedRelationshipWith(CBaseEntity *pWith);

	int IRelationship ( CBaseEntity *pTarget );

	
	BOOL forceIdleFrameReset(void);
	BOOL usesAdvancedAnimSystem(void);

	
	int LookupActivityHard(int activity);
	int tryActivitySubstitute(int activity);

	float massInfluence(void);
	int GetProjectileType(void);
	
	Vector GetVelocityLogical(void);
	void SetVelocityLogical(const Vector& arg_newVelocity);

	void OnDeflected(CBaseEntity* arg_entDeflector);


	void OnTakeDamageSetConditions(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType, int bitsDamageTypeMod);
	
	int getHullIndexForNodes(void);
	
	
	//????
	void DeathSound ( void );
	void AlertSound ( void );
	void IdleSound ( void );
	void PainSound ( void );

	

	void setAnimationSmart(const char* arg_animName);
	void setAnimationSmart(const char* arg_animName, float arg_frameRate);
	void setAnimationSmart(int arg_animIndex, float arg_frameRate);
	void setAnimationSmartAndStop(const char* arg_animName);
	void setAnimationSmartAndStop(const char* arg_animName, float arg_frameRate);
	void setAnimationSmartAndStop(int arg_animIndex, float arg_frameRate);

	
	GENERATE_TRACEATTACK_PROTOTYPE
	GENERATE_TAKEDAMAGE_PROTOTYPE


	
	// No attacks
	BOOL CheckRangeAttack1 ( float flDot, float flDist ) { return FALSE; }
	BOOL CheckRangeAttack2 ( float flDot, float flDist ) { return FALSE; }
	BOOL CheckMeleeAttack1 ( float flDot, float flDist ) { return FALSE; }
	BOOL CheckMeleeAttack2 ( float flDot, float flDist ) { return FALSE; }

	//MODDD
	//int TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType );
	//int TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType, int bitsDamageTypeMod );
	
	void MonsterThink ( void );

	
	void EXPORT ChumToadTouch ( CBaseEntity *pOther );
	void EXPORT PickupUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	void SetActivity ( Activity NewActivity );


	Schedule_t *GetSchedule( void ) override;
	Schedule_t* GetScheduleOfType( int Type) override;

	void StartTask ( Task_t *pTask );
	void RunTask ( Task_t *pTask );
	
	float HearingSensitivity();
	BOOL bypassAllowMonstersSpawnCheck(void);


	
	CUSTOM_SCHEDULES;


	GENERATE_DEADTAKEDAMAGE_PROTOTYPE
	GENERATE_GIBMONSTER_PROTOTYPE
	GENERATE_GIBMONSTERGIB_PROTOTYPE
	GENERATE_KILLED_PROTOTYPE

	void onDelete(void);


};//END OF CChumToad


/*
class CChumToadRespawnable : public CChumToad, public CRespawnable {
public:
	CChumToadRespawnable::CChumToadRespawnable(void);
	void Spawn(void);
	BOOL isRespawnable(void) { return TRUE; }

};
*/





#endif //END OF #ifdef CHUMTOAD_H
