
#include "extdll.h"
#include "func_door_health.h"
#include "util.h"
#include "basetoggle.h"
#include "basebutton.h"
#include "player.h"
#include "lights.h"


EASY_CVAR_EXTERN_DEBUGONLY(wallHealthDoor_closeDelay)



LINK_ENTITY_TO_CLASS(func_door_health, CHealthDoor);

#if FORCE_ROTDOOR_TO_HEALTHDOOR == 1
LINK_ENTITY_TO_CLASS(func_door_rotating, CHealthDoor);
#endif




// map: a2a2c 
// coords near door: Origin:(1645.69,-693.91,-627.97)

CHealthDoor::CHealthDoor(void) {

	//only used if a healing rotation door needs them.
	currentPreference = HDP_CLOSED;
	turnedOffHealLight = FALSE;

}//END OF CHealthDoor constructor



//??
//doorCloseDelay
//currentPreference
TYPEDESCRIPTION CHealthDoor::m_SaveData[] =
{
	DEFINE_FIELD(CHealthDoor, turnedOffHealLight, FIELD_BOOLEAN),

	//...
};
//IMPLEMENT_SAVERESTORE( CHealthDoor, CRotDoor );


//MODDD - NEW. Let the healthModuleInstance know to save / restore here, otherwise it has no way of getting called.
//        Only entities are called straight from the engine on saving / loading for this.
//Yes, CBaseToggle is the direct parent of CWallHealth here but CBaseEntity was chosen instead.
//Not sure if CBaseToggle was deliberately skipped, but keeping it that way.
int CHealthDoor::Save(CSave& save)
{
	if (!CRotDoor::Save(save))
		return 0;
	int iWriteFieldsResult = save.WriteFields("CHealthDoor", this, m_SaveData, ARRAYSIZE(m_SaveData));

	//return iWriteFieldsResult;

	if (iWriteFieldsResult) {
		int iWriteFields_HealthModule_Result = healthModuleInstance.Save(save);
		return iWriteFields_HealthModule_Result;
	}
	else {
		//what?
		return 0;
	}
}
int CHealthDoor::Restore(CRestore& restore)
{
	if (!CRotDoor::Restore(restore))
		return 0;


	//Establish that I'm the parent entity again.
	healthModuleInstance.setupRestore(static_cast <CBaseEntity*>(this), static_cast <I_HealthModule_Parent*>(this));
	
	int iReadFieldsResult = restore.ReadFields("CHealthDoor", this, m_SaveData, ARRAYSIZE(m_SaveData));

	//return iReadFieldsResult;

	if (iReadFieldsResult) {
		int iReadFields_HealthModule_Result = healthModuleInstance.Restore(restore);
		return iReadFields_HealthModule_Result;
	}
	else {
		//what?
		return 0;
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////










//MODDD - also new event methods required by the HealthModule instance for callbacks.
void CHealthDoor::I_HealthModule_ChargeEmpty(void) {
	//pev->frame = 1;
	//turn the door if this happens?
}
void CHealthDoor::I_HealthModule_ChargeRestored(void) {
	//pev->frame = 0;
	//if (healthModuleInstance.m_iJuice > 0) {
		turnOnHealLight();
	//}
}
void CHealthDoor::I_HealthModule_UseStart(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) {
	//CBaseDoor::Use(pActivator, pCaller, useType, value);
	onDoorUse();
}
void CHealthDoor::I_HealthModule_UseContinuous(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) {
	//CBaseDoor::Use(pActivator, pCaller, useType, value);
	onDoorUse();
}

void CHealthDoor::onDoorUse(void) {


	if (healthModuleInstance.m_iJuice > 0) {
		// old way for keeping the light on until the very end, remove all else here.
		/////////////////////////////////////
		if (m_toggle_state == TS_AT_BOTTOM || m_toggle_state == TS_GOING_DOWN) {
			//Closed or closing? Open me.
			currentPreference = HDP_OPEN;
		}
		if (m_toggle_state == TS_AT_TOP) {
			//This resets the close delay.
			doorCloseDelay = gpGlobals->time + m_flWait;
		}
		/////////////////////////////////////
	}
	else {
		if (m_toggle_state != TS_AT_BOTTOM) {
			currentPreference = HDP_CLOSED;
			doorCloseDelay = gpGlobals->time;
		}
	}
}



void CHealthDoor::I_HealthModule_UseEnd(void) {

	// Turns the light out the moment we run out of juice.
	//currentPreference == HDP_CLOSED && 
	if (healthModuleInstance.m_iJuice <= 0) {
		turnOffHealLight();  //turn off the light to signify this charger is empty.
	}

}//END OF I_HealthModule_UseEnd


//void CHealthDoor::I_HealthModule_SetThink_UseEnd(void) {
//	SetThink(static_cast <void (CBaseEntity::*)(void)>(&CHealthDoor::UseEnd));
//}
//void CHealthDoor::I_HealthModule_SetThink_ChargeRestored(void) {
//	SetThink(static_cast <void (CBaseEntity::*)(void)>(&CHealthDoor::ChargeRestored));
//}

void CHealthDoor::I_HealthModule_SetThink_Custom(void) {
	SetThink(static_cast <void (CBaseEntity::*)(void)>(&CHealthDoor::CustomThink));
}

/*
void CHealthDoor::UseEnd(void) {
	healthModuleInstance.UseEnd();
}//END OF UseEnd

void CHealthDoor::ChargeRestored(void) {
	healthModuleInstance.ChargeRestored();
}
*/

void CHealthDoor::CustomThink(void) {
	//This think method is only possible if we have the healer spawnflag.

	//int test = (this->m_pfnThink==NULL);


	if (angularMoveDoneTime != -1 && gpGlobals->time >= angularMoveDoneTime) {
		angularMoveDoneTime = -1;
		AngularMoveDone();
	}

	healthModuleInstance.CustomThink();

	//doorGoDownDelay ?
	if (doorCloseDelay != -1 && gpGlobals->time >= doorCloseDelay) {
		doorCloseDelay = -1;
		//Open for too long without use? close it.
		currentPreference = HDP_CLOSED;
	}

	if (m_toggle_state == TS_AT_BOTTOM) {
		//If closed...
		if (currentPreference == HDP_OPEN) {
			//closed but want to open?

			// play door unlock sounds
			//MODDD TODO - also play for closing, or a separate sound for closing?
			PlayLockSounds(pev, &m_ls, FALSE, FALSE);

			DoorGoUp();
		}


		// NOTE - enable this to turn the heal light at closing after running out.
		//if (currentPreference == HDP_CLOSED && healthModuleInstance.m_iJuice <= 0) {
		//	turnOffHealLight();  //turn off the light to signify this charger is empty.
		//}

	}

	if (m_toggle_state == TS_AT_TOP) {

		if (healthModuleInstance.m_iJuice <= 0) {
			//I want to shut.
			doorCloseDelay = -1;
			currentPreference = HDP_CLOSED;
		}

		if (currentPreference == HDP_CLOSED) {
			//stopped at the top but want to close? do so.
			DoorGoDown();
		}
	}

}//END OF CustomThink


void CHealthDoor::turnOffHealLight(void) {
	//If out of health, look for the light entity and turn it off.
	if (!turnedOffHealLight) {
		if (!FStringNull(pev->target)) {
			edict_t* pentTarget = NULL;
			edict_t* pentLight = NULL;
			//pentTarget = FIND_ENTITY_BY_TARGETNAME(pentTarget, STRING(pev->target));
			while ((FNullEnt(pentTarget = FIND_ENTITY_BY_TARGETNAME(pentTarget, STRING(pev->target)))) == FALSE) {
				if (VARS(pentTarget) != pev) {
					pentLight = pentTarget;
					//only makes sense to pick up this one. If there's multiple with the same name that's a map issue.
					break;
				}
			}//END OF while
			if (!FNullEnt(pentLight)) {
				//easyForcePrintLine("I got a light: %s", STRING(pentLight->v.classname) );

				if (FStrEq(STRING(pentLight->v.classname), "light") == TRUE) {
					//the cast can work.
					CBaseEntity* otherEnt = CBaseEntity::Instance(pentLight);
					CLight* otherEnt_light = static_cast<CLight*>(otherEnt);
					otherEnt_light->TurnOff();
					turnedOffHealLight = TRUE;
				}
				else {
					easyForcePrintLine("WallHealthDoor: ERROR. targetname \"%s\" found, but is not of classname \"light\". Classname: \"%s\".", STRING(pentLight->v.classname));
				}
			}
			else {
				easyForcePrintLine("WallHealthDoor: ERROR. targetname \"%s\" not found. No light to connect.", STRING(pev->target));
			}
		}
		else {
			easyForcePrintLine("WallHealthDoor: ERROR. targetname string is blank, no light to connect.");
		}
	}//END OF turnedOffHealLight check
}//END OF turnOffHealLight

void CHealthDoor::turnOnHealLight(void) {
	//If respawned (refilled), look for the light entity and turn it back on.
	if (turnedOffHealLight) {
		if (!FStringNull(pev->target)) {
			edict_t* pentTarget = NULL;
			edict_t* pentLight = NULL;
			//pentTarget = FIND_ENTITY_BY_TARGETNAME(pentTarget, STRING(pev->target));
			while ((FNullEnt(pentTarget = FIND_ENTITY_BY_TARGETNAME(pentTarget, STRING(pev->target)))) == FALSE) {
				if (VARS(pentTarget) != pev) {
					pentLight = pentTarget;
					//only makes sense to pick up this one. If there's multiple with the same name that's a map issue.
					break;
				}
			}//END OF while
			if (!FNullEnt(pentLight)) {
				//easyForcePrintLine("I got a light: %s", STRING(pentLight->v.classname) );

				if (FStrEq(STRING(pentLight->v.classname), "light") == TRUE) {
					//the cast can work.
					CBaseEntity* otherEnt = CBaseEntity::Instance(pentLight);
					CLight* otherEnt_light = static_cast<CLight*>(otherEnt);
					otherEnt_light->TurnOn();
					turnedOffHealLight = FALSE;
				}
				else {
					easyForcePrintLine("WallHealthDoor: ERROR. targetname \"%s\" found, but is not of classname \"light\". Classname: \"%s\".", STRING(pentLight->v.classname));
				}
			}
			else {
				easyForcePrintLine("WallHealthDoor: ERROR. targetname \"%s\" not found. No light to connect.", STRING(pev->target));
			}
		}
		else {
			easyForcePrintLine("WallHealthDoor: ERROR. targetname string is blank, no light to connect.");
		}
	}//END OF turnedOffHealLight check
}//END OF turnOnHealLight




void CHealthDoor::AngularMove(Vector vecDestAngle, float flSpeed)
{
	//Otherwise we need to handle this differently.  Can't affect the think method or time at all.
	//Started as a clone of CBaseToggle's AngularMove.
	ASSERTSZ(flSpeed != 0, "AngularMove:  no speed is defined!");
	//	ASSERTSZ(m_pfnCallWhenMoveDone != NULL, "AngularMove: no post-move function defined");

	m_vecFinalAngle = vecDestAngle;

	// Already there?
	if (vecDestAngle == pev->angles)
	{
		AngularMoveDone();
		return;
	}

	// set destdelta to the vector needed to move
	Vector vecDestDelta = vecDestAngle - pev->angles;

	// divide by speed to get time to reach dest
	float flTravelTime = vecDestDelta.Length() / flSpeed;

	// set nextthink to trigger a call to AngularMoveDone when dest is reached
	//MODDD - CHANGE.  Need to use a scheduled time instead.
	//pev->nextthink = pev->ltime + flTravelTime;
	//SetThink( &CBaseToggle::AngularMoveDone );
	angularMoveDoneTime = gpGlobals->time + flTravelTime;


	// scale the destdelta vector by the time spent traveling to get velocity
	pev->avelocity = vecDestDelta / flTravelTime;

}//END OF AngularMove




void CHealthDoor::OnDoorGoUp(void) {
	//MODDD - some base behavior moved from CBaseDoor.

	entvars_t* pevActivator;

	// It could be going-down, if blocked.
	ASSERT(m_toggle_state == TS_AT_BOTTOM || m_toggle_state == TS_GOING_DOWN);

	// emit door moving and stop sounds on CHAN_STATIC so that the multicast doesn't
	// filter them out and leave a client stuck with looping door sounds!
	if (!FBitSet(pev->spawnflags, SF_DOOR_SILENT))
		EMIT_SOUND(ENT(pev), CHAN_STATIC, (char*)STRING(pev->noiseMoving), 1, ATTN_NORM);

	m_toggle_state = TS_GOING_UP;

	SetMoveDone(&CBaseDoor::DoorHitTop);

	//MODDD - moved from the part of CBaseDoor's method that checks for being CHealthDoor.
	// !!! BUGBUG Triggered doors don't work with this yet
	float sign = 1.0;

	if (m_hActivator != NULL)
	{
		pevActivator = m_hActivator->pev;

		if (!FBitSet(pev->spawnflags, SF_DOOR_ONEWAY) && pev->movedir.y) 		// Y axis rotation, move away from the player
		{
			Vector vec = pevActivator->origin - pev->origin;
			Vector angles = pevActivator->angles;
			angles.x = 0;
			angles.z = 0;
			UTIL_MakeVectors(angles);
			//			Vector vnext = (pevToucher->origin + (pevToucher->velocity * 10)) - pev->origin;
			UTIL_MakeVectors(pevActivator->angles);
			Vector vnext = (pevActivator->origin + (gpGlobals->v_forward * 10)) - pev->origin;
			if ((vec.x * vnext.y - vec.y * vnext.x) < 0)
				sign = -1.0;
		}
	}
	AngularMove(m_vecAngle2 * sign, pev->speed);

}//END OF OnDoorGoUp

void CHealthDoor::OnDoorHitTop(void) {

	//We want this much of the parent class to happen.
	if (!FBitSet(pev->spawnflags, SF_DOOR_SILENT))
	{
		UTIL_StopSound(ENT(pev), CHAN_STATIC, (char*)STRING(pev->noiseMoving));
		EMIT_SOUND(ENT(pev), CHAN_STATIC, (char*)STRING(pev->noiseArrived), 1, ATTN_NORM);
	}

	ASSERT(m_toggle_state == TS_GOING_UP);
	m_toggle_state = TS_AT_TOP;


	if (currentPreference == HDP_OPEN) {
		//I want to stay open for at least this much longer.
		doorCloseDelay = gpGlobals->time + m_flWait;
	}
}//END OF OnDoorHitTop


//MODDD - moved from CBaseDoor to be more specific to here.
void CHealthDoor::OnDoorGoDown(void) {

	if (!FBitSet(pev->spawnflags, SF_DOOR_SILENT))
		EMIT_SOUND(ENT(pev), CHAN_STATIC, (char*)STRING(pev->noiseMoving), 1, ATTN_NORM);

#ifdef DOOR_ASSERT
	ASSERT(m_toggle_state == TS_AT_TOP);
#endif // DOOR_ASSERT
	m_toggle_state = TS_GOING_DOWN;

	SetMoveDone(&CBaseDoor::DoorHitBottom);

	//MODDD
	AngularMove(m_vecAngle1, pev->speed);
}//END OF OnDoorGoDown


void CHealthDoor::OnDoorHitBottom(void) {

	//Like the further barent (CBaseDoor) but a lot of that stuff is no longer necessary.

	if (!FBitSet(pev->spawnflags, SF_DOOR_SILENT))
	{
		UTIL_StopSound(ENT(pev), CHAN_STATIC, (char*)STRING(pev->noiseMoving));
		EMIT_SOUND(ENT(pev), CHAN_STATIC, (char*)STRING(pev->noiseArrived), 1, ATTN_NORM);
	}

	ASSERT(m_toggle_state == TS_GOING_DOWN);
	m_toggle_state = TS_AT_BOTTOM;
	
}//END OF OnDoorHitBottom



void CHealthDoor::ReportGeneric(void) {

	//int test = (this->m_pfnThink==NULL);

	CRotDoor::ReportGeneric();

	//HACK: turn my charge back on if out.
	//if(healthModuleInstance.m_iJuice <= 0){
	//	healthModuleInstance.ChargeRestored();
	//}

}//END OF ReportGeneric




void CHealthDoor::KeyValue(KeyValueData* pkvd)
{
	/*
	if(!healthModuleInstance.establishedParentYet){
		//incredibly hacky, but KeyValue appears to be the earliest thing called in an entity. And it may be needed to set
		//things in our healthModule.  Not that it even needs to know who the parent is at this point.  Ah well.
		healthModuleInstance.setup(static_cast <CBaseEntity*>(this), static_cast <I_HealthModule_Parent*>(this));
	}
	*/

	//Let my healthModuleInstance have a say first.

	healthModuleInstance.KeyValue(pkvd);

	if (pkvd->fHandled == FALSE) {
		CRotDoor::KeyValue(pkvd);
	}

}

void CHealthDoor::Activate() {
	CRotDoor::Activate();
}


void CHealthDoor::Spawn(void) {

	//easyForcePrintLine("CHealthDoor::WHAT ARE myyyy SPAWNFLAGS?! %d", pev->spawnflags);

	//if ((pev->spawnflags & SF_DOOR_HEAL) && !healthModuleInstance.establishedParentYet) 
	if(!healthModuleInstance.establishedParentYet){
		healthModuleInstance.setupSpawn(static_cast<CBaseEntity*>(this), static_cast<I_HealthModule_Parent*>(this));
	}

	// Calling CRotDoor::Spawn will call Precache too.
	// If calling precache later through that Spawn call is acceptable?  TEST.
	//Precache();

	//if ((pev->spawnflags & SF_DOOR_HEAL)) {

		//Is enforcing this a good idea?
		/*
		pev->spawnflags &= ~SF_DOOR_START_OPEN;
		pev->spawnflags &= ~SF_DOOR_ROTATE_BACKWARDS;
		pev->spawnflags |= SF_DOOR_PASSABLE;
		pev->spawnflags |= SF_DOOR_ONEWAY;
		pev->spawnflags &= ~SF_DOOR_NO_AUTO_RETURN;
		pev->spawnflags &= ~SF_DOOR_ROTATE_Z;
		pev->spawnflags &= ~SF_DOOR_ROTATE_X;
		pev->spawnflags |= SF_DOOR_USE_ONLY;
		pev->spawnflags |= SF_DOOR_NOMONSTERS;
		*/
		//SF_DOOR_SILENT?  exclude it.
		//pev->spawnflags = (pev->spawnflags | (SF_DOOR_PASSABLE | SF_DOOR_ONEWAY | SF_DOOR_USE_ONLY | SF_DOOR_NOMONSTERS)) & ~(SF_DOOR_START_OPEN | SF_DOOR_ROTATE_BACKWARDS | SF_DOOR_NO_AUTO_RETURN | SF_DOOR_ROTATE_Z | SF_DOOR_ROTATE_X);
		

		// Enforce these spawnflags for best behavior from the CBaseDoor class I assume?
		int test = (pev->spawnflags | (SF_DOOR_PASSABLE | SF_DOOR_ONEWAY | SF_DOOR_USE_ONLY | SF_DOOR_NOMONSTERS)) & ~(SF_DOOR_START_OPEN | SF_DOOR_ROTATE_BACKWARDS | SF_DOOR_NO_AUTO_RETURN | SF_DOOR_ROTATE_Z | SF_DOOR_ROTATE_X);
		//int test2 = ( (SF_DOOR_PASSABLE | SF_DOOR_ONEWAY | SF_DOOR_USE_ONLY | SF_DOOR_NOMONSTERS)) & ~(SF_DOOR_START_OPEN | SF_DOOR_ROTATE_BACKWARDS | SF_DOOR_NO_AUTO_RETURN | SF_DOOR_ROTATE_Z | SF_DOOR_ROTATE_X);

		pev->spawnflags = test;

		//easyForcePrint("yes:");
		//printLineIntAsBinary((unsigned int)test, 32u);
		//printLineIntAsBinary((unsigned int)test2, 32u);

		healthModuleInstance.Spawn();

		//also, THIS must be something. Defaulting to this CVar's # of seconds if not provided properly from KeyValues.
		if (m_flWait < 0) {
			m_flWait = EASY_CVAR_GET_DEBUGONLY(wallHealthDoor_closeDelay);
		}

		//UTIL_MakeVectors( pev->angles );
		//gpGlobals->v_forward ??

		//const char* stringtest1 = STRING(pev->target);
		//const char* stringtest2 = STRING(pev->targetname);


		//this->ReportGeneric();
	//}//END OF heal check

	CRotDoor::Spawn();

}//END OF Spawn



void CHealthDoor::Precache()
{
	healthModuleInstance.Precache();
	CRotDoor::Precache();  //likely had something in mind?
}


/*
	- Must be a flag
- When flag is enabled
	* Check if the player doesn't have 100 hp
		* If the player doesn't have 100hp, let the door to open
			*Give the player X amount of health (make it to use the already existing values from the healthcharger in skill.cfg
			*Make sure you can't get any more HP afterwards (I can prevent this by forcing the door to never go back to normal
			*Of course make it play a sound when using (medshot4.wav) and one when you are already full HP (medshotno1.wav)
*/
void CHealthDoor::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	//???
	//if ( useType != USE_SET )		// Momentary buttons will pass down a float in here
	//	return;

	//if ( value > 1.0 )
	//	value = 1.0;

	//DEBUG: always pass for now.?
	//pev->spawnflags |= SF_DOOR_HEAL;

	healthModuleInstance.Use(pActivator, pCaller, useType, value);

	//at least do this like the base class does. Just in case?
	m_hActivator = pActivator;

}//END OF Use


