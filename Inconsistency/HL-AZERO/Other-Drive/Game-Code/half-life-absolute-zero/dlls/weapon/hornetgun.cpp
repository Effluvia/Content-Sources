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

#pragma once

#include "extdll.h"
#include "hornetgun.h"
#include "util.h"
#include "cbase.h"
#include "basemonster.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "hornet.h"
#include "gamerules.h"


EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelay)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelaycustom)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteclip)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteammo)
EASY_CVAR_EXTERN_DEBUGONLY(fastHornetsInheritsPlayerVelocity);


// HACKY - redirect any references to the old 'RechargeTime' to a better synched var.
#define m_flRechargeTime m_flReleaseThrow

#ifdef CLIENT_DLL
BOOL atLeastOneHornet = FALSE;
#endif

float HGUN_prevRechargeTime = -1;
float HGUN_prevRechargeTimeTo = -1;

LINK_ENTITY_TO_CLASS( weapon_hornetgun, CHgun );



BOOL CHgun::IsUseable( void )
{
	return TRUE;
}

void CHgun::Spawn( )
{
	Precache( );
	m_iId = WEAPON_HORNETGUN;
	SET_MODEL(ENT(pev), "models/w_hgun.mdl");

	m_iClip = -1;
	m_iDefaultAmmo = HIVEHAND_DEFAULT_GIVE;
	m_iFirePhase = 0;

	FallInit();// get ready to fall down.
}


void CHgun::Precache( void )
{
	//Lots of sounds precached in the hornet's precache, including this weapon's firing sounds.
	//Not that this makes a difference, precached either way with UTIL_PrecacheOther("hornet") below.
	precacheGunPickupSound();

	PRECACHE_MODEL("models/v_hgun.mdl");
	PRECACHE_MODEL("models/w_hgun.mdl");
	PRECACHE_MODEL("models/p_hgun.mdl");

	m_usHornetFire = PRECACHE_EVENT ( 1, "events/firehornet.sc" );

	UTIL_PrecacheOther("hornet");
}


//MODDD
void CHgun::customAttachToPlayer(CBasePlayer *pPlayer ){
	m_pPlayer->SetSuitUpdate("!HEV_HORNET", FALSE, SUIT_NEXT_IN_30MIN);
}

int CHgun::AddToPlayer( CBasePlayer *pPlayer )
{
	if ( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
	{

#ifndef CLIENT_DLL
		if ( IsMultiplayer() )
		{
			// in multiplayer, all hivehands come full. 
			SetPlayerPrimaryAmmoCount(HORNET_MAX_CARRY);
		}
#endif

		MESSAGE_BEGIN( MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev );
			WRITE_BYTE( m_iId );
		MESSAGE_END();
		return TRUE;
	}
	return FALSE;
}

int CHgun::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "Hornets";
	p->iMaxAmmo1 = HORNET_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;



#if SPLIT_ALIEN_WEAPONS_INTO_NEW_SLOT != 1
	p->iSlot = 3;
	//MODDD - rpg moved out.
	//p->iPosition = 3;
	p->iPosition = 2;
#else
	// to the new slot you go!
	p->iSlot = 5;
	p->iPosition = 0;
#endif



	p->iId = m_iId = WEAPON_HORNETGUN;
	//MODDD - added ITEM_FLAG_SELECTONEMPTY.
	p->iFlags = ITEM_FLAG_SELECTONEMPTY | ITEM_FLAG_NOAUTOSWITCHEMPTY | ITEM_FLAG_NOAUTORELOAD;
	p->iWeight = HORNETGUN_WEIGHT;

	return 1;
}


BOOL CHgun::Deploy( )
{
	//MODDD - get a differnet deploy time.
	return DefaultDeploy( "models/v_hgun.mdl", "models/p_hgun.mdl", HGUN_UP, "hive", 0, 0, 30.0/30.0, 30.0/30.0 );
}

void CHgun::Holster( int skiplocal /* = 0 */ )
{
	//m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	//SendWeaponAnim( HGUN_DOWN );

	//!!!HACKHACK - can't select hornetgun if it's empty! no way to get ammo for it, either.
	//MODDD - wait wait wait wait wait.
	// You guys made... a flag, just to let weapons be select-able even without ammo.
	// What.  The.
	// See GetItemInfo...
	/*
	if ( PlayerPrimaryAmmoCount() <= 0 )
	{
		SetPlayerPrimaryAmmoCount(1);
	}
	*/

	//WARNING - this is retail's, not sure if what we use will have a different anim time.
	DefaultHolster(HGUN_DOWN, skiplocal, 0, (19.0f/16.0f) );

}


void CHgun::PrimaryAttack()
{
	Reload( );


	//easyForcePrintLine("A I FIIIIIRRRRRRREEEEEDDDD #1");
	if (PlayerPrimaryAmmoCount() <= 0)
	{
		return;
	}
	//easyForcePrintLine("A I FIIIIIRRRRRRREEEEEDDDD #2");

#ifdef CLIENT_DLL
	// absurdly high, prevent a HGUN_prevRechargeTimeTo from the next frame since firing took place
	HGUN_prevRechargeTimeTo = 99999999;
#endif


	//easyForcePrintLine("Well gee I tried glob:%.2f, rech:%.2f ammo:%d", gpGlobals->time, m_flRechargeTime, PlayerPrimaryAmmoCount());

#ifndef CLIENT_DLL
	UTIL_MakeVectors( m_pPlayer->pev->v_angle );

	CBaseEntity *pHornet = CBaseEntity::Create( "hornet", m_pPlayer->GetGunPosition( ) + gpGlobals->v_forward * 16 + gpGlobals->v_right * 8 + gpGlobals->v_up * -12, m_pPlayer->pev->v_angle, m_pPlayer->edict() );
	
	//MODDD - no, nevermind.  Too slow to be decent, hornets move on their own, making this seem really wonky.
	pHornet->pev->velocity = gpGlobals->v_forward * 300;

	//Hm. ?
	//pHornet->vecFlightDirTrue = gpGlobals->v_forward * 300;

	//pHornet->pev->velocity = gpGlobals->v_forward * 300 + UTIL_GetProjectileVelocityExtra(m_pPlayer->pev->velocity, m_pPlayer->someotherhornetstuffhere);

#endif



	
	//MODDD
	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteclip) == 0 && EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteammo) == 0){
		ChangePlayerPrimaryAmmoCount(-1);
	}

	//MODDD - if it's the last hit, give the client a little lee-way this frame.  For clientside.
#ifdef CLIENT_DLL
	if (PlayerPrimaryAmmoCount() <= 0) {
		m_flRechargeTime = gpGlobals->time + 0.40;
	}
	else {
		m_flRechargeTime = gpGlobals->time + 0.5;
	}
#else
	m_flRechargeTime = gpGlobals->time + 0.5;
#endif



	m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = DIM_GUN_FLASH;

	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	//SendWeaponAnimBypass(HGUN_SHOOT);

	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usHornetFire, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, FIREMODE_TRACK, 0, 0, 0 );
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (11.0/24.0) + randomIdleAnimationDelay();
	

	// player "shoot" animation
	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );


	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelay) == 0){
		//MODDD - uh...   ??  what?  Just have a fire delay like any other weapon?
		// And set the secondary attack delay too, why not really.
		// Seems doing fire-delay logic the normal way fixed sometimes firing two hornets in rapid succession
		// (sometimes so rapid that they spawn in nearly the same place and crash).  Whoops.
		/*
		m_flNextPrimaryAttack = m_flNextPrimaryAttack + 0.25;
		if (m_flNextPrimaryAttack < UTIL_WeaponTimeBase() )
		{
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.25;
		}
		*/

		SetAttackDelays(UTIL_WeaponTimeBase() + 0.25);
	}else{
		// little extra, because the primary attack seems to fail if it is too low; hornets hit each other and fall.
		SetAttackDelays(UTIL_WeaponTimeBase() + EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelaycustom) + 0.03f);
	}


	

	//m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
}



void CHgun::SecondaryAttack( void )
{

	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteammo) == 1){
		if ( pszAmmo1() && PlayerPrimaryAmmoCount() < 8 )
		{
			SetPlayerPrimaryAmmoCount(8);
		}
	}



	Reload();


	//easyForcePrintLine("B I FIIIIIRRRRRRREEEEEDDDD #1");
	if (PlayerPrimaryAmmoCount() <= 0)
	{
		return;
	}
	//easyForcePrintLine("B I FIIIIIRRRRRRREEEEEDDDD #2");

#ifdef CLIENT_DLL
	// absurdly high, prevent a HGUN_prevRechargeTimeTo from the next frame since firing took place
	HGUN_prevRechargeTimeTo = 99999999;
#endif



	//Wouldn't be a bad idea to completely predict these, since they fly so fast...
#ifndef CLIENT_DLL
	CBaseEntity *pHornet;
	Vector vecSrc;

	UTIL_MakeVectors( m_pPlayer->pev->v_angle );

	vecSrc = m_pPlayer->GetGunPosition( ) + gpGlobals->v_forward * 16 + gpGlobals->v_right * 8 + gpGlobals->v_up * -12;

	m_iFirePhase++;
	switch ( m_iFirePhase )
	{
	case 1:
		vecSrc = vecSrc + gpGlobals->v_up * 8;
		break;
	case 2:
		vecSrc = vecSrc + gpGlobals->v_up * 8;
		vecSrc = vecSrc + gpGlobals->v_right * 8;
		break;
	case 3:
		vecSrc = vecSrc + gpGlobals->v_right * 8;
		break;
	case 4:
		vecSrc = vecSrc + gpGlobals->v_up * -8;
		vecSrc = vecSrc + gpGlobals->v_right * 8;
		break;
	case 5:
		vecSrc = vecSrc + gpGlobals->v_up * -8;
		break;
	case 6:
		vecSrc = vecSrc + gpGlobals->v_up * -8;
		vecSrc = vecSrc + gpGlobals->v_right * -8;
		break;
	case 7:
		vecSrc = vecSrc + gpGlobals->v_right * -8;
		break;
	case 8:
		vecSrc = vecSrc + gpGlobals->v_up * 8;
		vecSrc = vecSrc + gpGlobals->v_right * -8;
		m_iFirePhase = 0;
		break;
	}

	pHornet = CBaseEntity::Create( "hornet", vecSrc, m_pPlayer->pev->v_angle, m_pPlayer->edict() );
	
	//MODDD
	//pHornet->pev->velocity = gpGlobals->v_forward * 1200;
	pHornet->pev->velocity = gpGlobals->v_forward * 1200 + UTIL_GetProjectileVelocityExtra(m_pPlayer->pev->velocity, EASY_CVAR_GET_DEBUGONLY(fastHornetsInheritsPlayerVelocity) );

	pHornet->pev->angles = UTIL_VecToAngles( pHornet->pev->velocity );

	pHornet->SetThink( &CHornet::StartDart );

#endif


	//MODDD - if it's the last hit, give the client a little lee-way this frame.  For clientside.
#ifdef CLIENT_DLL
	if (PlayerPrimaryAmmoCount() <= 0) {
		m_flRechargeTime = gpGlobals->time + 0.40;
	}
	else {
		m_flRechargeTime = gpGlobals->time + 0.5;
	}
#else
	m_flRechargeTime = gpGlobals->time + 0.5;
#endif


	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usHornetFire, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, FIREMODE_FAST, 0, 0, 0 );


	//MODDD
	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteclip) == 0 && EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteammo) == 0){
		ChangePlayerPrimaryAmmoCount(-1);
	}

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (11.0/24.0) + randomIdleAnimationDelay();


	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = DIM_GUN_FLASH;

		// player "shoot" animation
	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );


	//MODDD 
	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelay) == 0){
		//MODDD - slightly increased between-attack delay,was 0.10.
		SetAttackDelays(UTIL_WeaponTimeBase() + 0.115);
	}else{
		SetAttackDelays(UTIL_WeaponTimeBase() + EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelaycustom));
	}
	

	//m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
}




void CHgun::Reload( void )
{
	//MODDD - hornet-loading logic moved to PostItemFrameThink, so it doesn't depend on 
	// having the weapon finished deploying (why wait until the weapon finishes deploying
	// to show the updated hornet count from the elapsed time?).
}



void CHgun::ItemPostFrameThink(void) {




	/*
	BOOL primaryHeld = ((m_pPlayer->pev->button & IN_ATTACK) ); //&& CanAttack(m_flNextPrimaryAttack, gpGlobals->time, UseDecrement()));

		float HGUN_currentRechargeTimeTo = (m_flRechargeTime - gpGlobals->time);
		//
		
		easyForcePrintLine("aaa  pres:%dcrt:%.2f prt:%.2f", primaryHeld, HGUN_currentRechargeTimeTo, HGUN_prevRechargeTimeTo);

	if (primaryHeld) {


#ifdef CLIENT_DLL


		//easyPrintLine("HEYYYAH %.2f", (m_flRechargeTime - HGUN_prevRechargeTime));
		//(atLeastOneHornet || (m_flRechargeTime - HGUN_prevRechargeTime > 0.2) )

		if (PlayerPrimaryAmmoCount() <= 0 && (HGUN_prevRechargeTimeTo <= 0.12 && HGUN_prevRechargeTimeTo < HGUN_currentRechargeTimeTo) ) {
			ChangePlayerPrimaryAmmoCount(1);
			m_pPlayer->m_rgAmmoCLIENTHISTORY[getPrimaryAmmoType()] += 1;
			atLeastOneHornet = FALSE;
			PrimaryAttack();  // FORCE IT
		}

#endif
		HGUN_prevRechargeTimeTo = HGUN_currentRechargeTimeTo;
		HGUN_prevRechargeTime = m_flRechargeTime;
	}


	*/




	BOOL primaryHeld = ((m_pPlayer->pev->button & IN_ATTACK)); //&& CanAttack(m_flNextPrimaryAttack, gpGlobals->time, UseDecrement()));
	BOOL secondaryHeld = ((m_pPlayer->pev->button & IN_ATTACK));


	//if (primaryHeld) {
	//	easyForcePrintLine("IM HELD DOWN");
	//}

	
	//MODDD - moved from 'Reload' below, see notes there
	if (PlayerPrimaryAmmoCount() < HORNET_MAX_CARRY) {
		// This adds a hornet for every add-1-hornet-interval that passed between the last time
		// the weapon was out and now to account for frames that think logic couldn't run
		// (as it's only run on the currently equipped player weapon).


		float HGUN_currentRechargeTimeTo = (m_flRechargeTime - gpGlobals->time);
		//

		
		//  pres:1 primam:0 prt:0.01 crt:0.49 <=0.12?1 p<c?1
		//if (primaryHeld) {

#ifdef CLIENT_DLL
		//if (!IsMultiplayer()) {
			//(atLeastOneHornet || (m_flRechargeTime - HGUN_prevRechargeTime > 0.2) )
			if (PlayerPrimaryAmmoCount() <= 1 && (HGUN_prevRechargeTimeTo <= 0.12 && HGUN_prevRechargeTimeTo < HGUN_currentRechargeTimeTo)) {
				//easyForcePrintLine("hey yah");


				//easyForcePrintLine("aaa  pres:%d,%d primam:%d prt:%.2f crt:%.2f <=0.12?%d p<c?%d", primaryHeld, secondaryHeld, PlayerPrimaryAmmoCount(), HGUN_prevRechargeTimeTo, HGUN_currentRechargeTimeTo, (HGUN_prevRechargeTimeTo <= 0.12), (HGUN_prevRechargeTimeTo < HGUN_currentRechargeTimeTo));

				// fiah nao
				if (PlayerPrimaryAmmoCount() <= 0) {
					ChangePlayerPrimaryAmmoCount(1);
					m_pPlayer->m_rgAmmoCLIENTHISTORY[getPrimaryAmmoType()] += 1;
				}
				//m_flNextPrimaryAttack = 0;
				//m_flNextSecondaryAttack = 0;

				atLeastOneHornet = FALSE;

				if (secondaryHeld) {
					SecondaryAttack();  // FORCE IT
				}
				else if (primaryHeld) {
					PrimaryAttack();  // FORCE IT
				}
				else {
					// ???
					m_flNextPrimaryAttack = 0;
					m_flNextSecondaryAttack = 0;
				}
			}
		//}
#endif

		HGUN_prevRechargeTimeTo = HGUN_currentRechargeTimeTo;
		HGUN_prevRechargeTime = m_flRechargeTime;
		//}




		while (PlayerPrimaryAmmoCount() < HORNET_MAX_CARRY && m_flRechargeTime < gpGlobals->time)
		{
#ifdef CLIENT_DLL

			if (PlayerPrimaryAmmoCount() == 0) {
				atLeastOneHornet = TRUE;
				m_flRechargeTime += 0.4;
			}
			else {
				m_flRechargeTime += 0.5;
			}
#else
			m_flRechargeTime += 0.5;
#endif
			ChangePlayerPrimaryAmmoCount(1);
			//easyForcePrintLine("yay? %d", PlayerPrimaryAmmoCount());

		}//END OF while loop






	}//END OF ammo under max check


	CBasePlayerWeapon::ItemPostFrameThink();
}


void CHgun::WeaponIdle( void )
{
	Reload( );

	//MODDD
	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
	//if (m_flTimeWeaponIdle > gpGlobals->time)
		return;

	int iAnim;
	float flRand = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 0, 1 );
	if (flRand <= 0.75)
	{
		iAnim = HGUN_IDLE1;
		//MODDD
		//m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 30.0 / 16 * (2);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 30.0 / 16 * (1) + randomIdleAnimationDelay();
		//m_flTimeWeaponIdle = gpGlobals->time + 30.0 / 16 * (1);
	}
	else if (flRand <= 0.875)
	{
		iAnim = HGUN_FIDGETSWAY;
		//MODDD
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 40.0 / 16.0 + randomIdleAnimationDelay();
		//m_flTimeWeaponIdle = gpGlobals->time + 40.0 / 16.0;
	}
	else
	{
		iAnim = HGUN_FIDGETSHAKE;
		//MODDD
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 35.0 / 16.0 + randomIdleAnimationDelay();
		//m_flTimeWeaponIdle = gpGlobals->time + 35.0 / 16.0;
	}
	SendWeaponAnim( iAnim );
}
