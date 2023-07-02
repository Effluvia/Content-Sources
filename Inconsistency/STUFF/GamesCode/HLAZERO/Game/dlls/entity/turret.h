

//MODDD - new file. Contains CBaseTurret and subclasses so that their static variables
//        (class-wide, accessible by class alone from anywhere else without an instance and shared among all instances)
//        can be reset on demand in case the user changes the map or loads after updating a CVar. Also neatness.

#ifndef TURRET_H
#define TURRET_H

#include "util.h"
#include "cbase.h"
#include "effects.h"
#include "basemonster.h"



typedef enum
{
	TURRET_ANIM_NONE = 0,
	TURRET_ANIM_FIRE,
	TURRET_ANIM_SPIN,
	TURRET_ANIM_DEPLOY,
	TURRET_ANIM_RETIRE,
	TURRET_ANIM_DIE,
} TURRET_ANIM;



class CBaseTurret : public CBaseMonster
{
public:

	float m_flMaxSpin;		// Max time to spin the barrel w/o a target
	int m_iSpin;

	CSprite* m_pEyeGlow;
	int	m_eyeBrightness;

	int m_iDeployHeight;
	int m_iRetractHeight;
	int m_iMinPitch;

	int m_iBaseTurnRate;	// angles per second
	float m_fTurnRate;		// actual turn rate
	int m_iOrientation;		// 0 = floor, 1 = Ceiling
	int m_iOn;
	int m_fBeserk;			// Sometimes this bitch will just freak out
	int m_iAutoStart;		// true if the turret auto deploys when a target
							// enters its range

	Vector m_vecLastSight;
	float m_flLastSight;	// Last time we saw a target
	float m_flMaxWait;		// Max time to seach w/o a target
	int m_iSearchSpeed;		// Not Used!

	// movement
	float m_flStartYaw;
	Vector	m_vecCurAngles;
	Vector	m_vecGoalAngles;


	float m_flPingTime;	// Time until the next ping, used when searching
	float m_flSpinUpTime;	// Amount of time until the barrel should spin down when searching


	float postDeathEndTime;
	float nextDeathExplosionTime;
	float postDeathBeserkDir;
	float postDeathEyeBrightnessDir;


	CBaseTurret(void);


	virtual void StartReanimation(void);
	virtual void StartReanimationPost(int preReviveSequence);
	//Child classes are supposed to have their own spawn methods too. Why wasn't that virtual?
	virtual void Spawn(void);
	virtual void Precache(void);
	void KeyValue( KeyValueData *pkvd );
	void EXPORT TurretUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	
	//NOTE: virtual removed, already made virtual in parent classes further above in heirarchy.
	//void TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType);
	//int  TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType );
	

	// NOTE - not overridable!  Should work for any turret subclasses.
	BOOL TurretDeathCheck(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType, int bitsDamageTypeMod, void (CBaseTurret::*eventMethod)() );


	GENERATE_TRACEATTACK_PROTOTYPE_VIRTUAL
	GENERATE_TAKEDAMAGE_PROTOTYPE_VIRTUAL



	int Classify(void);
	virtual BOOL isOrganic(void);
	virtual BOOL isOrganicLogic(void);
	///////////////////////////////////////////////////////////////////////////////////////

	//MODDD - turrets have this variable, why not use it? In fact just rely on default behavior.
	//int BloodColor( void ) { return m_bloodColor; }

	//Not virtual, no child classes intended.
	GENERATE_GIBMONSTER_PROTOTYPE
	
	/*
	//MODDD - new.   Nevermind, per turret now.
	virtual void SetObjectCollisionBox( void )
	{
		// also, when past the post-death beserk time.
		if(pev->deadflag != DEAD_NO && gpGlobals->time >= postDeathEndTime){
			pev->absmin = pev->origin + Vector(-46, -46, 0);
			pev->absmax = pev->origin + Vector(46, 46, 26);
			//pev->absmin = pev->origin + Vector(-4, -4, 0);
			//pev->absmax = pev->origin + Vector(4, 4, 4);
		}else{
			CBaseMonster::SetObjectCollisionBox();
		}
	}
	*/


	virtual void onDelete(void);
	virtual void BeserkAimLogic(void);

	// These are overridable.  At least dummy out for turrets that don't want the parent behavior.
	virtual void DeathStart(void);
	virtual void DeathEnd(void);

	// Think functions
	void EXPORT ActiveThink(void);
	void EXPORT SearchThink(void);
	void EXPORT AutoSearchThink(void);
	void EXPORT TurretDeathThink(void);

	void EXPORT ReviveThink(void);


	//MODDD - IMPORTANT NOTE.    These were the only "virtual EXPORT's" as of retail.
	//  I... have no idea why they're even EXPORTs, they're never tied to any engine events
	// events like 'think' or 'touch'.
	// Removing the EXPORT's then, keep virtual.
	// If both virtual and EXPORT were needed, they would need to be separate methods like how
	// CBaseMonster handles CallMonsterThink and MonsterThink.
	// CallMonsterThink is not virtual but has export, and calls MonsterThink
	// (virtual but not export;  implemented per class and calls the parent MonsterThink).
	//virtual void EXPORT SpinDownCall(void) { m_iSpin = 0; }
	//virtual void EXPORT SpinUpCall(void) { m_iSpin = 1; }
	virtual void SpinDownCall(void) { m_iSpin = 0; }
	virtual void SpinUpCall(void) { m_iSpin = 1; }
	
	void EXPORT Deploy(void);
	void EXPORT Retire(void);
	
	void EXPORT Initialize(void);
	//MODDD - NEW.
	virtual void PostInit(void);

	virtual void Ping(void);
	virtual void EyeOn(void);
	virtual void EyeOff(void);

	virtual int	Save( CSave &save );
	virtual int	Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];

	// other functions
	void SetTurretAnim(TURRET_ANIM anim);
	int MoveTurret(void);
	virtual void Shoot(Vector &vecSrc, Vector &vecDirToEnemy) { };

	//MODDD - new. Child classes must have a static GibInfo reference to call upon.
	virtual GibInfo_t* getGibInfoRef(void) {return 0;};
	//Also the gibbing CVar for this class if needed.
	virtual float getGibCVar(void){return 0;}

	virtual BOOL CanMakeBloodParticles(void);

};










class CTurret : public CBaseTurret
{
private:
	int m_iStartSpin;

public:
	static GibInfo_t* gibModelRef;
	GibInfo_t* getGibInfoRef(void){return CTurret::gibModelRef;}
	float getGibCVar(void);

	void SetObjectCollisionBox( void )
	{
		// also, when past the post-death beserk time.
		if(pev->deadflag != DEAD_NO && gpGlobals->time >= postDeathEndTime){
			pev->absmin = pev->origin + Vector(-46, -46, 0);
			pev->absmax = pev->origin + Vector(46, 46, 48);
			//pev->absmin = pev->origin + Vector(-4, -4, 0);
			//pev->absmax = pev->origin + Vector(4, 4, 4);
		}else{
			CBaseMonster::SetObjectCollisionBox();
		}
	}

	void Spawn(void);
	void Precache(void);
	// Think functions
	void SpinUpCall(void);
	void SpinDownCall(void);

	virtual int	Save( CSave &save );
	virtual int	Restore( CRestore &restore );
	
	static	TYPEDESCRIPTION m_SaveData[];

	// other functions
	void Shoot(Vector &vecSrc, Vector &vecDirToEnemy);


};






class CMiniTurret : public CBaseTurret
{
public:

	static GibInfo_t* gibModelRef;
	GibInfo_t* getGibInfoRef(void){return CMiniTurret::gibModelRef;}
	float getGibCVar(void);

	void SetObjectCollisionBox( void )
	{
		// also, when past the post-death beserk time.
		if(pev->deadflag != DEAD_NO && gpGlobals->time >= postDeathEndTime){
			pev->absmin = pev->origin + Vector(-46, -46, 0);
			pev->absmax = pev->origin + Vector(46, 46, 26);
			//pev->absmin = pev->origin + Vector(-4, -4, 0);
			//pev->absmax = pev->origin + Vector(4, 4, 4);
		}else{
			CBaseMonster::SetObjectCollisionBox();
		}
	}

	void Spawn( );
	void Precache(void);
	// other functions
	void Shoot(Vector &vecSrc, Vector &vecDirToEnemy);
};



//=========================================================
// Sentry gun - smallest turret, placed near grunt entrenchments
//=========================================================
class CSentry : public CBaseTurret
{
public:
	float nextTouchCooldown;


	static GibInfo_t* gibModelRef;
	GibInfo_t* getGibInfoRef(void){return CSentry::gibModelRef;}
	float getGibCVar(void);


	CSentry(void);
	void Spawn( );
	void EXPORT PreInit(void);
	virtual void PostInit(void);
	void Precache(void);
	// other functions
	void Shoot(Vector &vecSrc, Vector &vecDirToEnemy);

	
	GENERATE_TRACEATTACK_PROTOTYPE
	GENERATE_TAKEDAMAGE_PROTOTYPE

	virtual void DeathStart(void);
	virtual void DeathEnd(void);

	void EXPORT SentryTouch( CBaseEntity *pOther );
	void EXPORT SentryDeathThink( void );

};



#endif //END OF TURRET_H