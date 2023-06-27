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
/*

===== h_battery.cpp ========================================================

  battery-related code

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"

#include "basetoggle.h"

#include "saverestore.h"
#include "skill.h"
#include "gamerules.h"

//MODDD - need it now
#include "player.h"



//MODDD - NOTE.  The battery pickup-able is in items.cpp.  This file only has the wall charger.
// Go figure.



class CRecharge : public CBaseToggle
{
public:
	CRecharge();

	//If something is not a child of CBaseMonster, it needs to be told to use the soundSentenaceSave system (if it is supposed to. Non-monsters do not automatically know this)
	BOOL usesSoundSentenceSave(void);

	void Spawn( );
	void Precache( void );
	void EXPORT Off(void);
	void EXPORT Recharge(void);
	void KeyValue( KeyValueData *pkvd );
	BOOL IsWorldAffiliated(void);
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	virtual int ObjectCaps( void ) { return (CBaseToggle::ObjectCaps() | FCAP_CONTINUOUS_USE) & ~FCAP_ACROSS_TRANSITION; }
	virtual int	Save( CSave &save );
	virtual int	Restore( CRestore &restore );


	static	TYPEDESCRIPTION m_SaveData[];

	float m_flNextCharge; 
	int	m_iReactivate ; // DeathMatch Delay until reactvated
	int	m_iJuice;
	int	m_iOn;			// 0 = off, 1 = startup, 2 = going
	float   m_flSoundTime;

	//MODDD - NEW.  Record the player that most recently used me.
	EHANDLE m_hRecentUser;


};

TYPEDESCRIPTION CRecharge::m_SaveData[] =
{
	DEFINE_FIELD( CRecharge, m_flNextCharge, FIELD_TIME ),
	DEFINE_FIELD( CRecharge, m_iReactivate, FIELD_INTEGER),
	DEFINE_FIELD( CRecharge, m_iJuice, FIELD_INTEGER),
	DEFINE_FIELD( CRecharge, m_iOn, FIELD_INTEGER),
	DEFINE_FIELD( CRecharge, m_flSoundTime, FIELD_TIME ),
};

IMPLEMENT_SAVERESTORE( CRecharge, CBaseEntity );

LINK_ENTITY_TO_CLASS(func_recharge, CRecharge);


void CRecharge::KeyValue( KeyValueData *pkvd )
{
	if (	FStrEq(pkvd->szKeyName, "style") ||
				FStrEq(pkvd->szKeyName, "height") ||
				FStrEq(pkvd->szKeyName, "value1") ||
				FStrEq(pkvd->szKeyName, "value2") ||
				FStrEq(pkvd->szKeyName, "value3"))
	{
		//MODDD - NOTE. I can only assume saying "fHandled = TRUE" and nothing else means, 
		// 'no damns were given'.
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "dmdelay"))
	{
		m_iReactivate = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else{
		CBaseToggle::KeyValue( pkvd );
	}
}

BOOL CRecharge::IsWorldAffiliated(){
    return TRUE;
}


CRecharge::CRecharge(){
	
}

BOOL CRecharge::usesSoundSentenceSave(void){
	return TRUE;
}


void CRecharge::Spawn()
{
	Precache( );

	pev->solid		= SOLID_BSP;
	pev->movetype	= MOVETYPE_PUSH;

	UTIL_SetOrigin(pev, pev->origin);		// set size and link into world
	UTIL_SetSize(pev, pev->mins, pev->maxs);
	SET_MODEL(ENT(pev), STRING(pev->model) );
	m_iJuice = gSkillData.suitchargerCapacity;
	pev->frame = 0;			
}


extern int global_useSentenceSave;
void CRecharge::Precache()
{

	global_useSentenceSave = TRUE;
	PRECACHE_SOUND("items/suitcharge1.wav");
	PRECACHE_SOUND("items/suitchargeno1.wav");
	PRECACHE_SOUND("items/suitchargeok1.wav", TRUE); //not you.
	global_useSentenceSave = FALSE;

}


void CRecharge::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{ 
	// if it's not a player, ignore
	if (!FClassnameIs(pActivator->pev, "player")){
		return;
	}

	// if there is no juice left, turn it off
	if (m_iJuice <= 0)
	{

		if (m_iOn) {
			//not anymore soon.
			//MODDD - let FVox do something.
			//if (pActivator->IsPlayer()) {
			if (pActivator != NULL && pActivator->IsPlayer()) {
				CBasePlayer* thePlayer = static_cast<CBasePlayer*>(pActivator);
				m_hRecentUser = NULL;
				//pPlayer->SetSuitUpdate("!HEV_BTY_DING", FALSE, SUIT_NEXT_IN_30SEC, 0.6);
				// NOTICE - a sentence of 5000 is a special code to do the flexible power readout sentence.
				thePlayer->SetSuitUpdateNumber(5000, SUIT_REPEAT_OK, -1, TRUE);
			}//END OF IsPlayer check
		}//END OF isOn check


		pev->frame = 1;			
		Off();
	}

	// if the player doesn't have the suit, or there is no juice left, make the deny noise
	if ((m_iJuice <= 0) || (!(pActivator->pev->weapons & (1<<WEAPON_SUIT))) || ((pActivator-> pev-> armorvalue == 100)))
	{
		if (m_flSoundTime <= gpGlobals->time)
		{
			m_flSoundTime = gpGlobals->time + 0.62;
			UTIL_PlaySound(ENT(pev), CHAN_ITEM, "items/suitchargeno1.wav", 0.85, ATTN_NORM );
		}
		return;
	}

	m_hRecentUser = pActivator;

	pev->nextthink = pev->ltime + 0.25;
	SetThink(&CRecharge::Off);

	// Time to recharge yet?

	if (m_flNextCharge >= gpGlobals->time){
		return;
	}

	// Make sure that we have a caller
	if (!pActivator){
		return;
	}

	m_hActivator = pActivator;

	//only recharge the player

	if (!m_hActivator->IsPlayer() ){
		return;
	}
	
	// Play the on sound or the looping charging sound
	if (!m_iOn)
	{
		m_iOn++;
		UTIL_PlaySound(ENT(pev), CHAN_ITEM, "items/suitchargeok1.wav", 0.85, ATTN_NORM, FALSE );
		m_flSoundTime = 0.56 + gpGlobals->time;
	}
	if ((m_iOn == 1) && (m_flSoundTime <= gpGlobals->time))
	{
		m_iOn++;
		UTIL_PlaySound(ENT(pev), CHAN_STATIC, "items/suitcharge1.wav", 0.85, ATTN_NORM );
	}


	// charge the player
	if (m_hActivator->pev->armorvalue < 100)
	{
		m_iJuice--;
		m_hActivator->pev->armorvalue += 1;

		if (m_hActivator->pev->armorvalue > 100){
			m_hActivator->pev->armorvalue = 100;
		}
	}

	// govern the rate of charge
	m_flNextCharge = gpGlobals->time + 0.1;
}

void CRecharge::Recharge(void)
{
	m_iJuice = gSkillData.suitchargerCapacity;
	pev->frame = 0;			
	SetThink( &CBaseEntity::SUB_DoNothing );
}

void CRecharge::Off(void)
{
	// Stop looping sound.
	if (m_iOn > 1){
		UTIL_StopSound( ENT(pev), CHAN_STATIC, "items/suitcharge1.wav" );
	}

	m_iOn = 0;

	//MODDD - let FVox do something.
	//if (pActivator->IsPlayer()) {
	if(m_iJuice > 0 && m_hRecentUser != NULL && m_hRecentUser->IsPlayer()){
		CBasePlayer* thePlayer = static_cast<CBasePlayer*>(m_hRecentUser.GetEntity() );
		m_hRecentUser = NULL;
		//pPlayer->SetSuitUpdate("!HEV_BTY_DING", FALSE, SUIT_NEXT_IN_30SEC, 0.6);
		// NOTICE - a sentence of 5000 is a special code to do the flexible power readout sentence.
		thePlayer->SetSuitUpdateNumber(5000, SUIT_REPEAT_OK, -1, TRUE);
	}//END OF IsPlayer check


	if ((!m_iJuice) && ( ( m_iReactivate = g_pGameRules->FlHEVChargerRechargeTime() ) > 0) )
	{
		// MODDD - added check for m_pfnThink. If we aren't already on our way to recharging, do so.
		// Otherwise, this was resetting the recharge time just from using an empty wall charger. LAME.
		if (m_pfnThink != &CRecharge::Recharge) {
			pev->nextthink = pev->ltime + m_iReactivate;
			SetThink(&CRecharge::Recharge);
		}
	}
	else{
		SetThink( &CBaseEntity::SUB_DoNothing );
	}
}