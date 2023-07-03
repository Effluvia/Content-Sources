


#ifndef PICKUPWALKER_H
#define PICKUPWALKER_H


#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "basemonster.h"
#include "schedule.h"
#include "activity.h"
#include "util_model.h"

#include "defaultai.h"
#include "soundent.h"
#include "game.h"

#include "weapons.h"
#include "player.h"
#include "gamerules.h"



enum
{
	TASK_PICKUPWALKER_IDLE_WAIT_RANDOM = LAST_COMMON_TASK + 1,
	

};

//=========================================================
// monster-specific schedule types
//=========================================================
enum
{
	SCHED_PICKUPWALKER_IDLE_WAIT = LAST_COMMON_SCHEDULE + 1,
};


//extern CGraph WorldGraph;
extern int gEvilImpulse101;





// This is a simple monster meant to wander around and not really react to anything or take damage. It can be picked up by the player.
// Think living throwables like squeak grenades (Snarks) or Chumtoads.  Although chumtoads no longer use this class.
class CPickupWalker : public CBaseMonster
{
public:
	CPickupWalker(void);
	BOOL usesSoundSentenceSave(void);

	void IdleSound( void );
	
	void Precache(void);
	void Spawn(void);
	

	Schedule_t *GetSchedule( void );
	Schedule_t* GetScheduleOfType( int Type);

	void StartTask ( Task_t *pTask );
	void RunTask ( Task_t *pTask );
	
	
	GENERATE_TRACEATTACK_PROTOTYPE_VIRTUAL
	GENERATE_TAKEDAMAGE_PROTOTYPE_VIRTUAL
	
	GENERATE_DEADTAKEDAMAGE_PROTOTYPE_VIRTUAL
	GENERATE_KILLED_PROTOTYPE_VIRTUAL
	

	void CheckRespawn(void);
	CBaseEntity* Respawn(void);
	void EXPORT AttemptToMaterialize(void);
	void EXPORT Materialize(void);


	// remain inoffensive / unoffending to all.  an "IRelationship" override should not be necessary, being CLASS_NONE should be good enough here.
	virtual int Classify ( void ) { return CLASS_NONE; };

	virtual BOOL isOrganic(void);
	virtual BOOL bypassAllowMonstersSpawnCheck(void);
	
	void EXPORT PickupWalkerTouch( CBaseEntity *pOther );
	void EXPORT PickupWalkerThink( void );

	void MonsterThink ( void );
	
	BOOL skipSpawnStuckCheck(void);

	//TEMPLATE
	//static const char *pIdleSounds[];
	
	CUSTOM_SCHEDULES;


	//TODO: per inherited class please!
	virtual const char* myWeaponClassname();
	virtual const char* myWeaponAmmoName();
	virtual int myWeaponSlot();
	virtual int myWeaponMaxAmmo();


};//END OF CPickupWalker

/*
const char *CPickupWalker::pIdleSounds[] = 
{
	"???/???_idle1.wav",
	"???/???_idle2.wav",
	"???/???_idle3.wav",

};
*/


class CChumToadPickupWalker : public CPickupWalker{
public:
	void IdleSound( void );

	void Precache(void);
	void Spawn(void);

	
	static const char *pIdleSounds[];

	const char* myWeaponClassname();
	const char* myWeaponAmmoName();
	int myWeaponSlot();
	int myWeaponMaxAmmo();

	
	BOOL forceIdleFrameReset(void);
	BOOL usesAdvancedAnimSystem(void);
	int LookupActivityHard(int activity);
	int tryActivitySubstitute(int activity);

};//END OF CPickupWalker



class CSqueakPickupWalker : public CPickupWalker{
public:
	void IdleSound( void );

	void Precache(void);
	void Spawn(void);

	
	static const char *pIdleSounds[];

	const char* myWeaponClassname();
	const char* myWeaponAmmoName();
	int myWeaponSlot();
	int myWeaponMaxAmmo();

	
	BOOL forceIdleFrameReset(void);
	BOOL usesAdvancedAnimSystem(void);
	int LookupActivityHard(int activity);
	int tryActivitySubstitute(int activity);


};//END OF CPickupWalker


#endif //PICKUPWALKER_H
