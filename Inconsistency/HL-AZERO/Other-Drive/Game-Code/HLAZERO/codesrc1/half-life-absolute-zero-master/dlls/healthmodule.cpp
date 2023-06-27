
#include "extdll.h"
#include "healthmodule.h"

#include "util.h"

#include "player.h"

#include "skill.h"
#include "gamerules.h"




//mODDD TODO MAJOR - are there any other things that need StopSound to be better checked for needing to hard-wire use of soundSentenceSave or not?




//MODDD - NEW HealthModule class
HealthModule::HealthModule(void){
	parentEntity_entity = NULL;  //give me one soon?
	parentEntity_event = NULL;  //give me one soon?
	firstUseSinceEnd = TRUE;
	establishedParentYet = FALSE;

	turnOffDelay = -1;
	rechargeDelay = -1;
	waitingForRecharge = FALSE;
}//END OF HealthModule constructor



void HealthModule::setupSpawn(CBaseEntity* arg_parentEntity_entity, I_HealthModule_Parent* arg_parentEntity_event){
	parentEntity_entity = arg_parentEntity_entity;
	parentEntity_event = arg_parentEntity_event;
	establishedParentYet = TRUE;
	
	// Get them thinking.
	parentEntity_entity->pev->nextthink = gpGlobals->time + 0.1;
	parentEntity_event->I_HealthModule_SetThink_Custom();

}//END OF setup

void HealthModule::setupRestore(CBaseEntity* arg_parentEntity_entity, I_HealthModule_Parent* arg_parentEntity_event){
	parentEntity_entity = arg_parentEntity_entity;
	parentEntity_event = arg_parentEntity_event;
	establishedParentYet = TRUE;
	
	// Don't get them thinking.  They will remember from the save file.  And for whatever reason we don't get the map's global time in time
	// to set nextthink properly, killing the think cycle completely.
	//parentEntity_entity->pev->nextthink = gpGlobals->time + 0.1;
	//parentEntity_event->I_HealthModule_SetThink_Custom();

}//END OF setup



	

TYPEDESCRIPTION HealthModule::m_SaveData[] ={
	DEFINE_FIELD( HealthModule, m_flNextCharge, FIELD_TIME),
	DEFINE_FIELD( HealthModule, m_iReactivate, FIELD_INTEGER),
	DEFINE_FIELD( HealthModule, m_iJuice, FIELD_INTEGER),
	DEFINE_FIELD( HealthModule, m_iOn, FIELD_INTEGER),
	DEFINE_FIELD( HealthModule, m_flSoundTime, FIELD_TIME),
	
	DEFINE_FIELD( HealthModule, turnOffDelay, FIELD_TIME),
	DEFINE_FIELD( HealthModule, rechargeDelay, FIELD_TIME),
	DEFINE_FIELD(HealthModule, waitingForRecharge, FIELD_BOOLEAN),
};

int HealthModule::Save( CSave &save ){
	//if ( !CBaseEntity::Save(save) )
	//	return 0;
	int iWriteFieldsResult = save.WriteFields( "HealthModule", this, m_SaveData, ARRAYSIZE(m_SaveData) );

	return iWriteFieldsResult;
}
int HealthModule::Restore( CRestore &restore ){
	//if ( !CBaseEntity::Restore(restore) )
	//	return 0;
	int iReadFieldsResult = restore.ReadFields( "HealthModule", this, m_SaveData, ARRAYSIZE(m_SaveData) );


	return iReadFieldsResult;
}


void HealthModule::KeyValue(KeyValueData* pkvd){
	if (	FStrEq(pkvd->szKeyName, "style") ||
				FStrEq(pkvd->szKeyName, "height") ||
				FStrEq(pkvd->szKeyName, "value1") ||
				FStrEq(pkvd->szKeyName, "value2") ||
				FStrEq(pkvd->szKeyName, "value3"))
	{
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "dmdelay"))
	{
		m_iReactivate = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else{
		//Nothing to do here, it's up to the parent on seeing that pkvd->fHandled remained FALSE.
	}
}//END OF KeyValue


BOOL HealthModule::IsWorldAffiliated(void) {
	return TRUE;  // yeah?
}



void HealthModule::CustomThink(void){
	//parentEntity_entity->pev->nextthink = parentEntity_entity->pev->ltime + 0.25;

	//assume we're on the same think method.
	//parentEntity_entity->pev->nextthink = gpGlobals->time + 0.1
	parentEntity_entity->pev->nextthink = parentEntity_entity->pev->ltime + 0.1;
	
	if(turnOffDelay != -1 && gpGlobals->time >= turnOffDelay){
		UseEnd();
		turnOffDelay = -1;
	}
	if(rechargeDelay != -1 && gpGlobals->time >= rechargeDelay){
		ChargeRestored();
		turnOffDelay = -1;
		rechargeDelay = -1;
		waitingForRecharge = FALSE;
	}

}//END OF CustomThink



void HealthModule::Spawn(){
	//NOTICE - I don't call my own Precache method! The class hosting this HealthModule instance must call it.

	m_iJuice = gSkillData.healthchargerCapacity;
}//END OF Spawn


extern int global_useSentenceSave;

void HealthModule::Precache(void){
	global_useSentenceSave = TRUE;
	PRECACHE_SOUND("items/medshot4.wav");
	PRECACHE_SOUND("items/medshotno1.wav");
	PRECACHE_SOUND("items/medcharge4.wav");
	global_useSentenceSave = FALSE;
}

void HealthModule::turnThinkOff(void){
	//MODDD - dummied out for now.
	//parentEntity_entity->SetThink( &CBaseEntity::SUB_DoNothing );

	turnOffDelay = -1;
	rechargeDelay = -1;
}


void HealthModule::Use( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value ){

	// Make sure that we have a caller
	if (!pActivator)
		return;
	// if it's not a player, ignore
	if ( !pActivator->IsPlayer() )
		return;

	// if there is no juice left, turn it off
	if (m_iJuice <= 0){
		//How does my parent entity respond to running out of charge?
		parentEntity_event->I_HealthModule_ChargeEmpty();
		UseEnd();

		firstUseSinceEnd = FALSE;  //don't allow this now.
	}

	//GetClassPtr((CBasePlayer *)pev) ;    not needed here, not working with a PEV, but the "entity" itself.
	//since pPlayer is a child along the line ( a child of CBaseEntity), a direct cast should be okay.
	CBasePlayer* pPlayer = (CBasePlayer*) (pActivator);

	//easyPrintLine("HEALING??? %d, %d, %d", pPlayer->m_rgbTimeBasedDamage[itbd_Bleeding], (pPlayer->m_bitsDamageTypeMod & DMG_BLEEDING), pPlayer->m_bitsDamageTypeMod);
	
	
	//MODDD - any healing items / charge removes the bleeding timed damage effect, if present.
	//MODDD TODO - this does still remove bleeding even if the unit is out of health. Is that okay?
	pPlayer->attemptCureBleeding();



	// if the player doesn't have the suit, or there is no juice left, make the deny noise
	if ((m_iJuice <= 0) || (!(pActivator->pev->weapons & (1<<WEAPON_SUIT))) || (pActivator-> pev-> health == 100))
	{
		if (m_flSoundTime <= gpGlobals->time)
		{
			m_flSoundTime = gpGlobals->time + 0.62;
			stopSounds();
			//Used to be CHAN_ITEM.
			PlaySound(CHAN_STATIC, "items/medshotno1.wav", 1.0, ATTN_NORM );
		}
		return;
	}

	/*
	parentEntity_entity->pev->nextthink = parentEntity_entity->pev->ltime + 0.25;
	//parentEntity_entity->SetThink(&CWallHealth::Off);
	parentEntity_event->I_HealthModule_SetThink_UseEnd();
	*/
	//ALTERNATE WAY?
	turnOffDelay = gpGlobals->time + 0.25;



	// Time to recharge yet?

	if (m_flNextCharge >= gpGlobals->time)
		return;

	// Play the on sound or the looping charging sound
	if (!m_iOn)
	{
		m_iOn++;
		///CHAN_BODY ?
		
		stopSounds();
		//Used to be CHAN_ITEM.
		PlaySound(CHAN_STATIC, "items/medshot4.wav", 1.0, ATTN_NORM );
		m_flSoundTime = 0.56 + gpGlobals->time;
	}
	if ((m_iOn == 1) && (m_flSoundTime <= gpGlobals->time))
	{
		m_iOn++;
		PlaySound(CHAN_STATIC, "items/medcharge4.wav", 1.0, ATTN_NORM );
	}


	// charge the player
	if ( pActivator->TakeHealth( 1, DMG_GENERIC ) )
	{
		m_iJuice--;

		//actual charge use? Tell the parent entity.
		if(firstUseSinceEnd){
			//First Use, as in not a continious use?  Mark this.
			parentEntity_event->I_HealthModule_UseStart(pActivator, pCaller, useType, value);
			firstUseSinceEnd = FALSE;  //now continuous until stopped.
		}else{
			parentEntity_event->I_HealthModule_UseContinuous(pActivator, pCaller, useType, value);
		}
	}
	// govern the rate of charge
	m_flNextCharge = gpGlobals->time + 0.1;

}//END OF USE




void HealthModule::ChargeRestored(void)
{
	firstUseSinceEnd = TRUE;
	

	stopSounds();
	//Used to be CHAN_ITEM.
	PlaySound(CHAN_STATIC, "items/medshot4.wav", 1.0, ATTN_NORM );
	m_iJuice = gSkillData.healthchargerCapacity;

	turnThinkOff();

	parentEntity_event->I_HealthModule_ChargeRestored();
}

//Play a sound using my parent entity.
void HealthModule::PlaySound(int channel, const char *sample, float volume, float attenuation){
	UTIL_PlaySound(ENT(parentEntity_entity->pev), channel, sample, volume, attenuation, TRUE );
}
void HealthModule::StopSound(int channel, const char *sample){
	UTIL_StopSound(ENT(parentEntity_entity->pev), channel, sample, TRUE );
}

//Note that I may be called even if out of charge.
void HealthModule::UseEnd(void){
	firstUseSinceEnd = TRUE;  //next use will be the first in a while.

	parentEntity_event->I_HealthModule_UseEnd();

	// Stop looping sound.
	if (m_iOn > 1)
		StopSound( CHAN_STATIC, "items/medcharge4.wav" );

	m_iOn = 0;

	//MODDD NOTE - waitasecond. Isn't setting m_iReactivate by KeyValue pointless if this just goes ahead and fetches it from the GameRules??
	if ((!m_iJuice) && ( ( m_iReactivate = g_pGameRules->FlHealthChargerRechargeTime() ) > 0) )
	//if ((!m_iJuice) && ((m_iReactivate = 10) > 0))
	{
		/*
		parentEntity_entity->pev->nextthink = parentEntity_entity->pev->ltime + m_iReactivate;
		//SetThink(&CWallHealth::Recharge);
		*/
		
		// can't have already been set.
		if (!waitingForRecharge) {
			rechargeDelay = gpGlobals->time + (float)m_iReactivate;
			waitingForRecharge = TRUE;
		}
	}
	else{
		turnThinkOff();
	}
}

void HealthModule::stopSounds(void){
	StopSound( CHAN_STATIC, "items/medshot4.wav" );
	StopSound( CHAN_STATIC, "items/medshotno1.wav" );
	StopSound( CHAN_STATIC, "items/medcharge4.wav" );

}//END OF stopSounds




