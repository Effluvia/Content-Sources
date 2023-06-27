/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/
#ifndef SCRIPTED_H
#define SCRIPTED_H

#ifndef SCRIPTEVENT_H
#include "scriptevent.h"
#endif

#define SF_SCRIPT_WAITTILLSEEN		1
#define SF_SCRIPT_EXITAGITATED		2
#define SF_SCRIPT_REPEATABLE		4
#define SF_SCRIPT_LEAVECORPSE		8
//#define SF_SCRIPT_INTERPOLATE		16 // don't use, old bug
#define SF_SCRIPT_NOINTERRUPT		32
#define SF_SCRIPT_OVERRIDESTATE		64
#define SF_SCRIPT_NOSCRIPTMOVEMENT	128



#define SCRIPT_BREAK_CONDITIONS		(bits_COND_LIGHT_DAMAGE|bits_COND_HEAVY_DAMAGE)
//#define SCRIPT_BREAK_CONDITIONS		(0)


enum SS_INTERRUPT
{
	SS_INTERRUPT_IDLE = 0,
	SS_INTERRUPT_BY_NAME,
	SS_INTERRUPT_AI,
};

//MODDD
EASY_CVAR_EXTERN_DEBUGONLY(cineChangelevelFix)

// when a monster finishes an AI scripted sequence, we can choose
// a schedule to place them in. These defines are the aliases to
// resolve worldcraft input to real schedules (sjb)
#define SCRIPT_FINISHSCHED_DEFAULT	0
#define SCRIPT_FINISHSCHED_AMBUSH	1

class CCineMonster : public CBaseMonster
{
public:

	CCineMonster(void);
	//BOOL wasAttached;


	void Spawn( void );
	virtual void KeyValue( KeyValueData *pkvd );
	virtual void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	virtual void Blocked( CBaseEntity *pOther );
	virtual void Touch( CBaseEntity *pOther );

	//MODDD - no; we shall allow this.  (comes with "FCAP_ACROSS_TRANSITION" unless told not to, like the as-is  did)
	//virtual int  ObjectCaps( void ) { return (CBaseMonster::ObjectCaps() & ~FCAP_ACROSS_TRANSITION); }
	virtual int  ObjectCaps( void ) { 
		
		//return CBaseMonster::ObjectCaps() | FCAP_FORCE_TRANSITION;
		//return (CBaseMonster::ObjectCaps()|FCAP_ACROSS_TRANSITION ); 
		
		if(EASY_CVAR_GET_DEBUGONLY(cineChangelevelFix) == 1 || EASY_CVAR_GET_DEBUGONLY(cineChangelevelFix) == 3){
			//return (CBaseMonster::ObjectCaps() ); 
			return (CBaseMonster::ObjectCaps()|FCAP_ACROSS_TRANSITION ); 
		}else{
			return (CBaseMonster::ObjectCaps() & ~FCAP_ACROSS_TRANSITION);
		}
	
	}
	
	virtual void Activate( void );

	virtual int	Save( CSave &save );
	virtual int	Restore( CRestore &restore );
	
	static	TYPEDESCRIPTION m_SaveData[];

	// void EXPORT CineSpawnThink( void );
	void EXPORT CineThink( void );
	void Pain( void );
	void Die( void );
	void DelayStart( int state );
	BOOL FindEntity( void );
	virtual void PossessEntity( void );

	void ReleaseEntity( CBaseMonster *pEntity );
	void CancelScript( void );
	virtual BOOL StartSequence( CBaseMonster *pTarget, int iszSeq, BOOL completeOnEmpty );
	virtual BOOL FCanOverrideState ( void );
	void SequenceDone ( CBaseMonster *pMonster );
	virtual void FixScriptMonsterSchedule( CBaseMonster *pMonster );
	BOOL CanInterrupt( void );
	void AllowInterrupt( BOOL fAllow );
	int	IgnoreConditions( void );

	//MODDD - NEW.
	static int scriptedIDLatest;

	int m_iszIdle;		// string index for idle animation
	int m_iszPlay;		// string index for scripted animation
	int m_iszEntity;	// entity that is wanted for this script
	int m_fMoveTo;
	int m_iFinishSchedule;
	float m_flRadius;		// range to search
	float m_flRepeat;	// repeat rate

	int m_iDelay;
	float m_startTime;

	int m_saved_movetype;
	int m_saved_solid;
	int m_saved_effects;
//	Vector m_vecOrigOrigin;
	BOOL m_interruptable;

#if HACKY_SCRIPT_TEST == 1
	//MODDD - NEW
	BOOL startTimeSet;
#endif


};



class CCineAI : public CCineMonster
{
	BOOL StartSequence( CBaseMonster *pTarget, int iszSeq, BOOL completeOnEmpty );
	void PossessEntity( void );
	BOOL FCanOverrideState ( void );
	virtual void FixScriptMonsterSchedule( CBaseMonster *pMonster );
};


#endif		//SCRIPTED_H
