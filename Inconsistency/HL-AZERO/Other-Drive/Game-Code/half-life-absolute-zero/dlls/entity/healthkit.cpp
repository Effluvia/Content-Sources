/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
#include "extdll.h"
#include "healthkit.h"


#include "util.h"

#include "basetoggle.h"
#include "weapons.h"
#include "nodes.h"

#include "skill.h"
#include "gamerules.h"

extern int gmsgItemPickup;


//MODDD - class CHealthKit moved to healthkit.h

LINK_ENTITY_TO_CLASS( item_healthkit, CHealthKit );

/*
TYPEDESCRIPTION	CHealthKit::m_SaveData[] = 
{

};


IMPLEMENT_SAVERESTORE( CHealthKit, CItem);
*/

CHealthKit::CHealthKit(void){

}
BOOL CHealthKit::usesSoundSentenceSave(void){
	return TRUE;
}



void CHealthKit::Spawn( void )
{
	Precache( );
	SET_MODEL(ENT(pev), "models/w_medkit.mdl");

	CItem::Spawn();
}

extern int global_useSentenceSave;

void CHealthKit::Precache( void )
{
	PRECACHE_MODEL("models/w_medkit.mdl");
	global_useSentenceSave = TRUE;
	PRECACHE_SOUND("items/smallmedkit1.wav");
	global_useSentenceSave = FALSE;
}


BOOL CHealthKit::MyTouch( CBasePlayer *pPlayer )
{
	if ( pPlayer->pev->deadflag != DEAD_NO )
	{
		return FALSE;
	}

	if ( pPlayer->TakeHealth( gSkillData.healthkitCapacity, DMG_GENERIC ) )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgItemPickup, NULL, pPlayer->pev );
			WRITE_STRING( STRING(pev->classname) );
		MESSAGE_END();

		UTIL_PlaySound(ENT(pPlayer->pev), CHAN_ITEM, "items/smallmedkit1.wav", 1, ATTN_NORM, TRUE);


		pPlayer->SetSuitUpdate("!HEV_MEDKIT", FALSE, SUIT_NEXT_IN_30MIN);

		//pPlayer->SetSuitUpdate("!HEV_BATTERY", FALSE, SUIT_NEXT_IN_1HOUR);

		//MODDD - any healing items / charge removes the bleeding timed damage effect, if present.
		pPlayer->attemptCureBleeding();



		if ( g_pGameRules->ItemShouldRespawn( this ) )
		{
			Respawn();
		}
		else
		{
			UTIL_Remove(this);	
		}

		return TRUE;
	}

	return FALSE;
}












//-------------------------------------------------------------
// Wall mounted health kit
//-------------------------------------------------------------

CWallHealth::CWallHealth(void){
	
}




//MODDD - class CWallHealth moved to healthkit.h

//MODDD - save data moved to the HealthModule class, of which CWallHealth has one of..
/*
TYPEDESCRIPTION CWallHealth::m_SaveData[] =
{
	DEFINE_FIELD( CWallHealth, m_flNextCharge, FIELD_TIME),
	DEFINE_FIELD( CWallHealth, m_iReactivate, FIELD_INTEGER),
	DEFINE_FIELD( CWallHealth, m_iJuice, FIELD_INTEGER),
	DEFINE_FIELD( CWallHealth, m_iOn, FIELD_INTEGER),
	DEFINE_FIELD( CWallHealth, m_flSoundTime, FIELD_TIME),
};
*/

//IMPLEMENT_SAVERESTORE( CWallHealth, CBaseEntity );

//MODDD - NEW. Let the healthModuleInstance know to save / restore here, otherwise it has no way of getting called.
//        Only entities are called straight from the engine on saving / loading for this.
//Yes, CBaseToggle is the direct parent of CWallHealth here but CBaseEntity was chosen instead.
//Not sure if CBaseToggle was deliberately skipped, but keeping it that way.
int CWallHealth::Save( CSave &save )
{
	if ( !CBaseEntity::Save(save) )
		return 0;
	//int iWriteFieldsResult = save.WriteFields( "CWallHealth", this, m_SaveData, ARRAYSIZE(m_SaveData) );

	//return iWriteFieldsResult;

	int iWriteFields_HealthModule_Result = healthModuleInstance.Save(save);
	return iWriteFields_HealthModule_Result;
}
int CWallHealth::Restore( CRestore &restore )
{
	if ( !CBaseEntity::Restore(restore) )
		return 0;

	//Establish that I'm the parent entity again.
	healthModuleInstance.setupRestore(static_cast<CBaseEntity*>(this), static_cast<I_HealthModule_Parent*>(this));

	//int iReadFieldsResult = restore.ReadFields( "CWallHealth", this, m_SaveData, ARRAYSIZE(m_SaveData) );

	//return iReadFieldsResult;

	int iReadFields_HealthModule_Result = healthModuleInstance.Restore(restore);
	return iReadFields_HealthModule_Result;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




LINK_ENTITY_TO_CLASS(func_healthcharger, CWallHealth);




//MODDD - also new event methods required by the HealthModule instance for callbacks.
void CWallHealth::I_HealthModule_ChargeEmpty(void){
	pev->frame = 1;
}
void CWallHealth::I_HealthModule_ChargeRestored(void){
	pev->frame = 0;
}
void CWallHealth::I_HealthModule_UseStart(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value){

}
void CWallHealth::I_HealthModule_UseContinuous(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value){

}
void CWallHealth::I_HealthModule_UseEnd(void){

}

//void CWallHealth::I_HealthModule_SetThink_UseEnd(void){
//	SetThink( static_cast <void (CBaseEntity::*)(void)>(&CWallHealth::UseEnd) );
//}
//void CWallHealth::I_HealthModule_SetThink_ChargeRestored(void){
//	SetThink( static_cast <void (CBaseEntity::*)(void)>(&CWallHealth::ChargeRestored) );
//}

void CWallHealth::I_HealthModule_SetThink_Custom(void){
	SetThink( static_cast <void (CBaseEntity::*)(void)>(&CWallHealth::CustomThink) );
}
void CWallHealth::CustomThink(void){
	healthModuleInstance.CustomThink();
}

void CWallHealth::ReportGeneric(void){

	int test = (this->m_pfnThink == NULL);

	CBaseToggle::ReportGeneric();

	//HACK: turn my charge back on if out.
	//if(healthModuleInstance.m_iJuice <= 0){
	//	healthModuleInstance.ChargeRestored();
	//}

}//END OF ReportGeneric


/*
void CWallHealth::UseEnd(void){
	healthModuleInstance.UseEnd();
}

void CWallHealth::ChargeRestored(void){
	healthModuleInstance.ChargeRestored();
}
*/





void CWallHealth::KeyValue( KeyValueData *pkvd )
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

	if(pkvd->fHandled == FALSE){
		CBaseToggle::KeyValue( pkvd );
	}
}


void CWallHealth::Activate(){

	//is this more appropriate?
	//healthModuleInstance.setup(static_cast <CBaseEntity*>(this), static_cast <I_HealthModule_Parent*>(this));


	CBaseToggle::Activate();
}


//MODDD MAJOR - functionality that's best re-used for use'able health in general like this moved to the HealthModule class.
//              Call relevant methods of healthModuleInstance (HealthModule) to hook it up to this entity (CWallHealth).
void CWallHealth::Spawn()
{
	if(!healthModuleInstance.establishedParentYet){
		healthModuleInstance.setupSpawn(static_cast <CBaseEntity*>(this), static_cast <I_HealthModule_Parent*>(this));
	}

	Precache( );

	healthModuleInstance.Spawn();



	pev->solid		= SOLID_BSP;
	pev->movetype	= MOVETYPE_PUSH;

	UTIL_SetOrigin(pev, pev->origin);		// set size and link into world
	UTIL_SetSize(pev, pev->mins, pev->maxs);
	SET_MODEL(ENT(pev), STRING(pev->model) );

	pev->frame = 0;


}



void CWallHealth::Precache()
{
	healthModuleInstance.Precache();
}


void CWallHealth::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{ 
	//int test = (this->m_pfnThink == &CWallHealth::CustomThink);

	//Tell my instance I am being used.
	healthModuleInstance.Use(pActivator, pCaller, useType, value);
}







BOOL CWallHealth::IsWorldAffiliated(){
    return TRUE;
}


BOOL CWallHealth::usesSoundSentenceSave(void){
	return TRUE;
}








