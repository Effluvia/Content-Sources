

#ifndef FUNC_DOOR_HEALTH_H
#define FUNC_DOOR_HEALTH_H


#include "basetoggle.h"
#include "doors.h"
#include "healthmodule.h"


typedef enum {
	HDP_OPEN = 0,
	HDP_CLOSED = 1

} HealDoorPreference;


//MODDD - serious problem. We can't use the same ent for healing / a door as think is needed for both. independent think times.
//        But this door can just choose to spawn an entity with only the HealthModule, and use itself for thinks. that would work fine.

//class CRotDoor : public CBaseDoor, I_HealthModule_Parent
class CHealthDoor : public CRotDoor, I_HealthModule_Parent
{
public:
	BOOL turnedOffHealLight;


	HealDoorPreference currentPreference;



	static TYPEDESCRIPTION m_SaveData[];
	virtual int Save(CSave& save);
	virtual int Restore(CRestore& restore);

	CHealthDoor(void);
	//BOOL usesSoundSentenceSave(void);

	void I_HealthModule_ChargeEmpty(void);
	void I_HealthModule_ChargeRestored(void);
	void I_HealthModule_UseStart(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void I_HealthModule_UseContinuous(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	void onDoorUse(void);  //this is my own simple common method to call for UseStart and UseContinuous above.

	void I_HealthModule_UseEnd(void);

	//void I_HealthModule_SetThink_UseEnd(void);
	//void I_HealthModule_SetThink_ChargeRestored(void);

	void I_HealthModule_SetThink_Custom(void);

	void EXPORT CustomThink(void);

	void turnOffHealLight(void);
	void turnOnHealLight(void);

	//MODDD - new override in case of something special the health wall door healer needs.
	virtual void AngularMove(Vector vecDestAngle, float flSpeed);

	virtual void OnDoorGoUp(void);
	virtual void OnDoorHitTop(void);
	virtual void OnDoorGoDown(void);
	virtual void OnDoorHitBottom(void);

	void ReportGeneric(void);



	//Moved to HealthModule. This is completely internal to healing logic.
	//void EXPORT Off(void);

	//Moved to HealthModule. This is completely internal to healing logic.
	//void EXPORT Recharge(void);
	//void EXPORT UseEnd(void);
	//void EXPORT ChargeRestored(void);

	void KeyValue(KeyValueData* pkvd);

	//MODDD - new.
	void Activate();
	virtual void Spawn(void);
	void Precache(void);
	virtual void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	virtual int ObjectCaps(void) {
		//add CONTINUOUS_USE, remove FCAP_ACROSS_TRANSITION (although CBaseDoor already does that 100% of the time)
		return (CRotDoor::ObjectCaps() | FCAP_CONTINUOUS_USE) & ~FCAP_ACROSS_TRANSITION;
	}//END OF ObjectCaps



};





#endif //FUNC_DOOR_HEALTH_H


