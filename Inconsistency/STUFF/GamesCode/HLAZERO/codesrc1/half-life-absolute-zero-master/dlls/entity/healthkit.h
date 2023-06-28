


#include "items.h"
#include "healthmodule.h"

#include "cbase.h"
//#include "basemonster.h"
#include "player.h"


//MODDD - new file to hold healthkit.cpp's classes for being included / called elsewhere.




class CHealthKit : public CItem
{

	//MODDD - why no public?
public:
	CHealthKit(void);
	BOOL usesSoundSentenceSave(void);
	void Spawn( void );
	void Precache( void );
	BOOL MyTouch( CBasePlayer *pPlayer );

/*
	virtual int	Save( CSave &save ); 
	virtual int	Restore( CRestore &restore );
	
	static	TYPEDESCRIPTION m_SaveData[];
*/

};




//MODDD - also using I_HealthModule_Parent as an interface to provide some methods for the HealthModule to call.
class CWallHealth : public CBaseToggle, public virtual I_HealthModule_Parent
{
public:

	HealthModule healthModuleInstance;  //guaranteed instance.
	
	//MODDD - moved to HealthModule.
	//static TYPEDESCRIPTION m_SaveData[];
	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );

	CWallHealth(void);
	
	void I_HealthModule_ChargeEmpty(void);
	void I_HealthModule_ChargeRestored(void);
	void I_HealthModule_UseStart(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	void I_HealthModule_UseContinuous(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	
	void I_HealthModule_UseEnd(void);
	
	//void I_HealthModule_SetThink_UseEnd(void);
	//void I_HealthModule_SetThink_ChargeRestored(void);

	void I_HealthModule_SetThink_Custom(void);

	void EXPORT CustomThink(void);

	void ReportGeneric(void);
	
	//Moved to HealthModule. This is completely internal to healing logic.
	//void EXPORT Off(void);

	//Moved to HealthModule. This is completely internal to healing logic.
	//void EXPORT Recharge(void);
	//void EXPORT UseEnd(void);
	//void EXPORT ChargeRestored(void);
	
	void KeyValue( KeyValueData *pkvd );

	void Activate();
	void Spawn( );
	void Precache( void );
	
	
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	virtual int ObjectCaps( void ) { return (CBaseToggle::ObjectCaps() | FCAP_CONTINUOUS_USE) & ~FCAP_ACROSS_TRANSITION; }
	

	
	BOOL usesSoundSentenceSave(void);
	BOOL IsWorldAffiliated(void);

};






