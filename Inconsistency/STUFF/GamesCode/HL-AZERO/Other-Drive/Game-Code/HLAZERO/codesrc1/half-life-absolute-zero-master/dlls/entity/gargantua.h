
#ifndef GARGANTUA_H
#define GARGANTUA_H
#pragma once

// Effects and attack-related classes (flame-thrower, stomp-effect, etc.) staying in gargantua.cpp

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "nodes.h"
#include "basemonster.h"
#include "schedule.h"
#include "effects.h"

class CGargantua : public CBaseMonster
{
private:
	static const char* pAttackHitSounds[];
	static const char* pBeamAttackSounds[];
	static const char* pAttackMissSounds[];
	//static const char *pRicSounds[];
	static const char* pFootSounds[];
	static const char* pIdleSounds[];
	static const char* pAlertSounds[];
	static const char* pPainSounds[];
	static const char* pAttackSounds[];
	static const char* pStompSounds[];
	static const char* pBreatheSounds[];

	CSprite* m_pEyeGlow;	// Glow around the eyes
	CBeam* m_pFlame[4];		// Flame beams

	int		m_eyeBrightness;	// Brightness target
	float	m_seeTime;			// Time to attack (when I see the enemy, I set this)
	float	m_flameTime;		// Time of next flame attack
	float	m_painSoundTime;	// Time of next pain sound
	float	m_streakTime;		// streak timer (don't send too many)
	float	m_flameX;			// Flame thrower aim
	float	m_flameY;

public:
	BOOL gargDeadBoundChangeYet;
	float fallShakeTime;

	float flameThrowerPreAttackDelay;
	float resetFlameThrowerPreAttackDelay;
	BOOL flameThrowerPreAttackInterrupted;
	float pissedRunTime;
	int consecutiveStomps;
	CBaseEntity* grabbedEnt;
	Vector grabbedEntOldMins;
	Vector grabbedEntOldMaxs;
	
	BOOL usingFastThink;
	float nextNormalThinkTime;


public:
	CGargantua(void);

	
	void setModel(void);
	void setModel(const char* m);
	BOOL getMonsterBlockIdleAutoUpdate(void);

	//MODDD - new
	CBaseEntity* GargantuaCheckTraceHullAttack(float flDist, int iDamage, int iDmgType);
	CBaseEntity* GargantuaCheckTraceHullAttack(float flDist, int iDamage, int iDmgType, int iDmgTypeMod);
	CBaseEntity* GargantuaCheckTraceHull(float flDist);

	BOOL needsMovementBoundFix(void);



	void Spawn( void );
	void Precache( void );
	void SetYawSpeed( void );
	int  Classify ( void );
	
	BOOL isSizeGiant(void);

	void SetFastThink(BOOL newVal);
	void MonsterThink(void);
	int  IRelationship( CBaseEntity *pTarget );
	
	GENERATE_TRACEATTACK_PROTOTYPE
	void TraceAttack_Traceless(entvars_t* pevAttacker, float flDamage, Vector vecDir, int bitsDamageType, int bitsDamageTypeMod);
	GENERATE_TAKEDAMAGE_PROTOTYPE



	void HandleAnimEvent( MonsterEvent_t *pEvent );

	BOOL CheckMeleeAttack1( float flDot, float flDist );		// Swipe
	BOOL CheckMeleeAttack2( float flDot, float flDist );		// Flames
	BOOL CheckRangeAttack1( float flDot, float flDist );		// Stomp attack
	

	//MODDD - made the bounds bigger, so that no matter where the death anim falls, it will be included for registering weapons hits.
	//(at the moment, the corpse is not collidable with movable entities though due to being difficult to determine with the current cheap hitbox.)
	//being able to have an offset to "pev->origin" for drawing (or the hitbox at least) would be very nice.
	/*
	void SetObjectCollisionBox( void )
	{
		pev->absmin = pev->origin + Vector( -80, -80, 0 );
		pev->absmax = pev->origin + Vector( 80, 80, 214 );
	}
	*/
	//MODDD - new.
	void SetObjectCollisionBox( void )
	{
		//EASY_CVAR_PRINTIF_PRE(gargantuaPrintout, easyPrintLine( "garg deadflag? %d", pev->deadflag));
		//could it be re-adjusted for "DEAD_DEAD" too?
		if(pev->deadflag != DEAD_NO){
			//if we are dead?
			pev->absmin = pev->origin + Vector( -280, -280, 0 );
			pev->absmax = pev->origin + Vector( 280, 280, 214 );
		}else{
			pev->absmin = pev->origin + Vector( -80, -80, 0 );
			pev->absmax = pev->origin + Vector( 80, 80, 214 );
			//CBaseMonster::SetObjectCollisionBox();
		}
	}


	Schedule_t *GetScheduleOfType( int Type );
	void StartTask( Task_t *pTask );
	void RunTask( Task_t *pTask );

	void PrescheduleThink( void );

	GENERATE_KILLED_PROTOTYPE
	void onDelete(void);



	void DeathEffect( void );

	//MODDD - Why unused?
	void DeathSound(void);

	void EyeOff( void );
	void EyeOn( int level );
	void EyeUpdate( void );
	
	// Leap?  LEAP?!    Well that's... an interesting never implemented method?   (found as-is)
	//void Leap( void );

	void StompAttack( void );
	void FlameCreate( void );
	void FlameUpdate( void );
	void FlameControls( float angleX, float angleY );
	void FlameDestroy(BOOL playOffSound );
	inline BOOL FlameIsOn( void ) { return m_pFlame[0] != NULL; }

	void FlameDamage( Vector vecStart, Vector vecEnd, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType );

	//MODDD - new. What causes heavy damage? It takes a lot to do that to me.
	void OnTakeDamageSetConditions(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType, int bitsDamageTypeMod);

	//MODDD - advanced anim.  Just to change flinch speeds per difficulty.
	BOOL usesAdvancedAnimSystem(void);
	int LookupActivityHard(int activity);
	int tryActivitySubstitute(int activity);

	void HandleEventQueueEvent(int arg_eventID);
	int getHullIndexForNodes(void);
	BOOL predictRangeAttackEnd(void);
	float getDistTooFar(void);
	
	float ScriptEventSoundAttn(void);
	float ScriptEventSoundVoiceAttn(void);

	void grabEnt(CBaseEntity* toGrab);
	void releaseGrabbedEnt(void);

	virtual int	Save( CSave &save );
	virtual int	Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];
	void PostRestore(void);

	CUSTOM_SCHEDULES;
		
};

#endif //GARGANTUA_H
