


#include "cbase.h"

//MODDD - new. Purely parent-less data class for storing info and behavior about healing to ease reuse between CWallHealth and
//        certain CRotDoor's using this.
//        So yes, not even of CBaseEntity.  Some events like 'Save' / 'Restore' must be called by some parent entity that handles those.

// -Reorganized a bit to put the healthModuleInstance in the I_HealthParentModule class instead, say so if that causes any issues


class I_HealthModule_Parent;

class HealthModule{
public:
	//NEW
	float rechargeDelay;
	float turnOffDelay;
	BOOL waitingForRecharge;


	BOOL establishedParentYet;

	BOOL firstUseSinceEnd;  //is this a continuous "use" or the first in a while?


	float m_flNextCharge;
	int	m_iReactivate; // DeathMatch Delay until reactvated
	int	m_iJuice;
	int	m_iOn;			// 0 = off, 1 = startup, 2 = going
	float   m_flSoundTime;


	// I need a way to hook into the engine for some features liky playing sounds. Some Entity to do that from.
	// I need to know it as both an entity (for its own entity variables and pev->...) and a HealthModule_Parent for sending events to.
	CBaseEntity* parentEntity_entity;
	I_HealthModule_Parent* parentEntity_event;


	
	HealthModule(void);

	// Yes sending the exact same thing twice is ugly but I don't know how else to say "I expect one object that is a subclass of 'this' and 'that'".
	void setupSpawn(CBaseEntity* arg_parentEntity_entity, I_HealthModule_Parent* arg_parentEntity_event);
	void setupRestore(CBaseEntity* arg_parentEntity_entity, I_HealthModule_Parent* arg_parentEntity_event);
	
	void CustomThink(void);

	void turnThinkOff(void);

	void Spawn( );
	void Precache( void );
	
	//void EXPORT Off(void);
	//void EXPORT Recharge(void);

	void UseEnd(void);
	void ChargeRestored(void);


	void KeyValue( KeyValueData *pkvd );
	BOOL IsWorldAffiliated(void);
	
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );


	void PlaySound(int channel, const char* sample, float volume, float attenuation);
	void StopSound(int channel, const char* sample);

	static TYPEDESCRIPTION m_SaveData[];
	int Save( CSave &save );
	int Restore( CRestore &restore );

	void stopSounds(void);



};

// A class that a parent of a HealthModule (in the sense of having a HealthModule instance at all) should take to guarantee some event methods.
// And implement those of course.
// C does multi-inheritence, not sure if there's any possible difference for that if this is more of an "Interface" like in Java than a parent class.
class I_HealthModule_Parent{
public:

	HealthModule healthModuleInstance;  //guaranteed instance.

										//MODDD - NOTICE!  If the  " = 0;"  (pure virtual) breaks VS6 compatability,
										// just replace " = 0;"  with "{};".
										// Or make a preprocessor constant somewhere that checks for visual studio version
										// and uses the right way for that version.
	virtual void I_HealthModule_ChargeEmpty(void) = 0;
	virtual void I_HealthModule_ChargeRestored(void) = 0;
	virtual void I_HealthModule_UseStart(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value) = 0;
	virtual void I_HealthModule_UseContinuous(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value) = 0;
	virtual void I_HealthModule_UseEnd(void) = 0;

	//virtual void I_HealthModule_SetThink_UseEnd(void) = 0;
	//virtual void I_HealthModule_SetThink_ChargeRestored(void) = 0;

	virtual void I_HealthModule_SetThink_Custom(void) = 0;

};
