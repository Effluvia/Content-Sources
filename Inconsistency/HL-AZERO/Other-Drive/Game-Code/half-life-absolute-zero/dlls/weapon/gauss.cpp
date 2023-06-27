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
#include "gauss.h"
#include "util.h"
#include "cbase.h"
#include "basemonster.h"

#include "nodes.h"
#include "player.h"
#include "soundent.h"
#include "shake.h"
#include "gamerules.h"

#include "util_debugdraw.h"




// Bunch of stuff moved to gauss.h for including things needed by here and ev_hldm.


//NOTES ON VARS (from before the revert, may not be relevant anymore):
//m_fireState   -  the mode that the animation uses for detecting "phase" (stage towards showing the full spin anim, as opposed to the pre-delay and showing the start spin anim).
//m_flStartThrow - delay required for the charge animation to start playing
//m_flReleaseThrow  -  alternate time recording since charging.  "Charging" for a very short amount of time
//     registers as a "tap" (primary fire) instead, since (quake style) one mouse button is used for both primary
//     (tap) and secondary (hold down) attacks.


EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelay)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelaycustom)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteclip)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteammo)

EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_nogaussrecoil)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(gaussRecoilSendsUpInSP)

EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST(gauss_mode)



#ifdef CLIENT_DLL
extern BOOL g_irunninggausspred;
#endif


LINK_ENTITY_TO_CLASS(weapon_gauss, CGauss);




CGauss::CGauss(void) {
	// NOTICE - all mentions of 'm_pPlayer->m_flStartCharge' changed to no longer involve the player
	// (var of this class instead).
	// ALSO, m_flStartCharge replaced with pev->fuser1, something that counts down to 0 (0-based timer).

	fuser1_store = -1;
	ignoreIdleTime = -1;
	inAttackPrev = 0;

}//END OF CGauss constructor




// Save/restore for serverside only!
#ifndef CLIENT_DLL
TYPEDESCRIPTION	CGauss::m_SaveData[] =
{
	DEFINE_FIELD(CGauss, m_fInAttack, FIELD_INTEGER),

	//MODDD - new
	DEFINE_FIELD(CGauss, m_fireState, FIELD_INTEGER),
	//MODDD - coming back here
	DEFINE_FIELD( CGauss, fuser1_store, FIELD_FLOAT ),  // was FIELD_TIME.
	//DEFINE_FIELD( CGauss, pev->fuser1, FIELD_FLOAT ),
	
	//	DEFINE_FIELD( CGauss, m_flPlayAftershock, FIELD_TIME ),
	//	DEFINE_FIELD( CGauss, m_flNextAmmoBurn, FIELD_TIME ),
	DEFINE_FIELD(CGauss, m_fPrimaryFire, FIELD_BOOLEAN),


};
//IMPLEMENT_SAVERESTORE(CGauss, CBasePlayerWeapon);

int CGauss::Save(CSave& save){
	if (!CBasePlayerWeapon::Save(save))
		return 0;

	fuser1_store = pev->fuser1;  //commit it

	return save.WriteFields("CGauss", this, m_SaveData, ARRAYSIZE(m_SaveData));
}

// REMEMBER!!! Client does not call Restore!
int CGauss::Restore(CRestore& restore){

	if (!CBasePlayerWeapon::Restore(restore))
		return 0;

	
	// TEST!!!   Going between levels?  Forget you created effects, re-do if needed.
	// Unless this is more of a clientside issue (resend the event to start effects there).
	// Is weird the sprite still exists when you think about it...?  yea, clientisde probably.
	//effectsExist = TRUE;


	//if(m_pPlayer != NULL){
	//m_pPlayer->TabulateAmmo();
	//}

	int result = restore.ReadFields("CGauss", this, m_SaveData, ARRAYSIZE(m_SaveData));

	//easyForcePrintLine("?????????????? %d %d", m_fInAttack, m_fireState);

	pev->fuser1 = fuser1_store;  //take it

	return result;
}
#endif




// Drop the damage and range based off of the direct-hit damage (flDamage).
void getRadiusStats(float flDamage, float& radDmg, float& radRange){
	// Diminishing returns with each '200' bracket that's reached.
	// Although, more range means more damage at the same distance, even without increasing damage.
	// Range drops off steeply for that reason.
	if(flDamage <= 200){
		radDmg = flDamage * 0.60;
		radRange = flDamage * 0.60;
	}else if(flDamage <= 400){
		radDmg = (200 * 0.60) + (flDamage - 200) * 0.30;
		radRange = (200 * 0.60) + (flDamage - 200) * 0.20;
	}else{
		radDmg = (200 * 0.60) + (400 - 200) * 0.30 + (flDamage - 400) * 0.20;
		radRange = (200 * 0.60) + (400 - 200) * 0.20 + (flDamage - 400) * 0.08;
	}
}//getRadiusStats



//MODDD - beware, damage-dealt is determined by ammo used, not time passed since starting a charge.
// This can still be used to tell when to do the auto-discharge maybe, so don't ignore it completely.
// And the pitch, this is used for how to scale the pitch snce starting a charge.
// Point is, good to keep in synch with when the most ammo that could possibly be used has been added
// to the charge.
float CGauss::GetFullChargeTime(void){
	if (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(gauss_mode) != 1) {
		if (IsMultiplayer()){
			return 1.5;
		}
		return 4;
	}else{
		// ALPHA: any differences for multiplayer unknown.  How about 30% less time?
		if (IsMultiplayer()){
			// - 1.3 ?
			return (0.7*(12-1.5))*0.7;
		}
		return 0.7*(12-1.5); // - 1.3;
	}
}//GetFullChargeTime



void CGauss::customAttachToPlayer(CBasePlayer* pPlayer) {
	m_pPlayer->SetSuitUpdate("!HEV_GAUSS", FALSE, SUIT_NEXT_IN_30MIN, 4.3f);
}


void CGauss::Spawn()
{
	Precache();
	m_iId = WEAPON_GAUSS;
	SET_MODEL(ENT(pev), "models/w_gauss.mdl");

	m_iClip = -1;
	m_iDefaultAmmo = GAUSS_DEFAULT_GIVE;

	FallInit();// get ready to fall down.
}


void CGauss::Precache(void)
{
	PRECACHE_MODEL("models/w_gauss.mdl");
	PRECACHE_MODEL("models/v_gauss.mdl");
	PRECACHE_MODEL("models/p_gauss.mdl");

	PRECACHE_SOUND("items/9mmclip1.wav");

	PRECACHE_SOUND("weapons/gauss2.wav");
	PRECACHE_SOUND("weapons/electro4.wav");
	PRECACHE_SOUND("weapons/electro5.wav");
	PRECACHE_SOUND("weapons/electro6.wav");
	PRECACHE_SOUND("ambience/pulsemachine.wav");

	precacheGunPickupSound();

	m_iGlow = PRECACHE_MODEL("sprites/hotglow.spr");
	m_iBalls = PRECACHE_MODEL("sprites/hotglow.spr");
	m_iBeam = PRECACHE_MODEL("sprites/smoke.spr");

	m_usGaussFire = PRECACHE_EVENT(1, "events/gauss.sc");
	m_usGaussSpin = PRECACHE_EVENT(1, "events/gaussspin.sc");
}

int CGauss::AddToPlayer(CBasePlayer* pPlayer)
{
	if (CBasePlayerWeapon::AddToPlayer(pPlayer))
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev);
		WRITE_BYTE(m_iId);
		MESSAGE_END();
		return TRUE;
	}
	return FALSE;
}

int CGauss::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "uranium";
	p->iMaxAmmo1 = URANIUM_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 3;

	//MODDD - now 0. RPG moved out. It used to take islot 3, iPosition 0.
	//p->iPosition = 1;
	p->iPosition = 0;

	p->iId = m_iId = WEAPON_GAUSS;
	p->iFlags = 0;
	p->iWeight = GAUSS_WEIGHT;

	return 1;
}

BOOL CGauss::Deploy()
{
	m_pPlayer->m_flPlayAftershock = 0.0;
	BOOL depResult = DefaultDeploy("models/v_gauss.mdl", "models/p_gauss.mdl", GAUSS_DRAW, "gauss", 0, 0, (36.0f + 1.0f) / (64.0f), -1);

	// undo the 'BlockLooping' that DefaultDeploy calls for in this case.
	stopBlockLooping();

	
	m_fInAttack = 0;
	m_fireState = 0;
	inAttackPrev = 0;

	fuser1_store = -1;
	ignoreIdleTime = -1;
	

	return depResult;
}

void CGauss::Holster(int skiplocal /* = 0 */)
{
	// NOTE - does this just stop sounds?
	PLAYBACK_EVENT_FULL(FEV_RELIABLE | FEV_GLOBAL, m_pPlayer->edict(), m_usGaussFire, 0.01, (float*)&m_pPlayer->pev->origin, (float*)&m_pPlayer->pev->angles, 0.0, 0.0, 0, 0, 0, 1);

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	SetAttackDelays(m_pPlayer->m_flNextAttack);

	SendWeaponAnim(GAUSS_HOLSTER);

	
	m_fInAttack = 0;
	m_fireState = 0;
	inAttackPrev = 0;

	fuser1_store = -1;
	ignoreIdleTime = -1;
	

	DefaultHolster(GAUSS_HOLSTER, skiplocal, 0, (31.0f + 1.0f) / (60.0f));
}


void CGauss::_PrimaryAttack()
{
	float primaryAmmoUsage;
	float attackAgainDelay;
	float attackAgainSecDelay;
	if (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(gauss_mode) != 1) {
		primaryAmmoUsage = 2;  //retail
		attackAgainDelay = 0.25;   // was 0.2, changed.
		attackAgainSecDelay = 0.45;
	}
	else {
		primaryAmmoUsage = 5;
		attackAgainDelay = 0.55;
		attackAgainSecDelay = 0.55;
	}


	// don't fire underwater
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound();
		// MODDD - changed from 0.15
		SetAttackDelays(UTIL_WeaponTimeBase() + 0.5);
		return;
	}

	if (PlayerPrimaryAmmoCount() < primaryAmmoUsage)
	{
		PlayEmptySound();
		m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
		SetAttackDelays(m_pPlayer->m_flNextAttack);
		return;
	}

	m_pPlayer->m_iWeaponVolume = GAUSS_PRIMARY_FIRE_VOLUME;
	m_fPrimaryFire = TRUE;

	//MODDD - only reduce ammo if cheats are off.
	if (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteclip) == 0 && EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteammo) == 0) {
		ChangePlayerPrimaryAmmoCount(-primaryAmmoUsage);
	}


	StartFire();
	m_fInAttack = 0;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0;


	if (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelay) == 0) {
		// And, time between primary attacks changed from 0.2 to 0.25.  Time to secondary lengthened further.
		m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + attackAgainDelay;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + attackAgainDelay;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + attackAgainSecDelay;

	}
	else {
		m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelaycustom);
		SetAttackDelays(m_pPlayer->m_flNextAttack);
	}
}

void CGauss::_SecondaryAttack()
{
	float chargeAmmoUsage;
	float chargeAmmoStoredMax;
	float chargeAmmoUsageDelay;
	float chargeInitialDelay;
	float startPitch;
	float maxPitch;
	float pitchDelta;

	float theGaussMode = EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(gauss_mode);
	if (theGaussMode != 1) {
		//retail
		chargeAmmoUsage = 1;
		chargeAmmoStoredMax = 13;
		chargeInitialDelay = 0.5;
		startPitch = 110;
		maxPitch = 250;
		if (IsMultiplayer()){
			chargeAmmoUsageDelay = 0.1;
		}else{
			chargeAmmoUsageDelay = 0.3;
		}
	}
	else {
		chargeAmmoUsage = 5;
		chargeAmmoStoredMax = 12;
		chargeInitialDelay = 0.5;
		startPitch = 103;
		maxPitch = 290;
		if (IsMultiplayer()){
			chargeAmmoUsageDelay = 0.7*0.7;
		}else{
			chargeAmmoUsageDelay = 0.7;
		}
	}
	pitchDelta = maxPitch - startPitch;


	// don't fire underwater
	if (m_pPlayer->pev->waterlevel == 3)
	{
		if (m_fInAttack != 0)
		{
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/electro4.wav", 1.0, ATTN_NORM, 0, 80 + RANDOM_LONG(0, 0x3f));
			SendWeaponAnim(GAUSS_IDLE);
			m_fInAttack = 0;
		}
		else
		{
			PlayEmptySound();
		}

		SetAttackDelays(UTIL_WeaponTimeBase() + 0.5);
		return;
	}



	// the looping charge animation needs this
	stopBlockLooping();


	if (m_fInAttack == 0)
	{
		// only do the out-of-ammo checks and init ammo drop if cheats are off
		if (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteclip) == 0 && EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteammo) == 0) {
			if (PlayerPrimaryAmmoCount() < chargeAmmoUsage)
			{
				EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/357_cock1.wav", 0.8, ATTN_NORM);
				m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
				SetAttackDelays(m_pPlayer->m_flNextAttack);
				return;
			}
			// take one ammo just to start the spin
			ChangePlayerPrimaryAmmoCount(-chargeAmmoUsage);
		}//END OF cheats check

	// well gee, is that so
		m_fPrimaryFire = FALSE;


		if (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelay) == 0) {
			// proceed with usual charging.
		}
		else {
			// fire now, max damage!
			//m_flStartCharge = -20;  //make it think this is always a full charge.
			// ineffective, this way now
			m_fireState = chargeAmmoStoredMax-1;

			StartFire();
			m_fInAttack = 0;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0;

			m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelaycustom);
			SetAttackDelays(m_pPlayer->m_flNextAttack);
			return;
		}

		m_pPlayer->m_flNextAmmoBurn = UTIL_WeaponTimeBase() + chargeAmmoUsageDelay;

		// spin up
		m_pPlayer->m_iWeaponVolume = GAUSS_PRIMARY_CHARGE_VOLUME;

		SendWeaponAnim(GAUSS_SPINUP);
		m_fInAttack = 1;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + chargeInitialDelay;
		
		// REPLACO
		//m_flStartCharge = gpGlobals->time;
		pev->fuser1 = 100;  // keep track of how far down it has counted, knowing it started at 100.

		//MODDD - changing the purpose of this var.  Instead of when to stop adding further ammo
		// (yes, despite the name), count how much ammo has been used separately by charging
		// since the first shot.
		// ...re-using m_flAmmoStartCharge for this isn't wise, the fuser2 that this is tied to on sending
		// between server/client tries to count down with frametime (tick down by milliseconds), which we really
		// don't want for a solid counter like this.  Use m_fireState instead.
		//m_pPlayer->m_flAmmoStartCharge = UTIL_WeaponTimeBase() + GetFullChargeTime();
		m_fireState = 0;

		PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usGaussSpin, 0.0, (float*)&g_vecZero, (float*)&g_vecZero, 0.0, 0.0, (int)startPitch, 0, 0, 0);
		
		m_iSoundState = SND_CHANGE_PITCH;
	}
	else if (m_fInAttack == 1)
	{
		if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
		{
			SendWeaponAnim(GAUSS_SPIN);
			m_fInAttack = 2;
		}
	}
	else
	{
		// during the charging process, eat one bit of ammo every once in a while
		if (UTIL_WeaponTimeBase() >= m_pPlayer->m_flNextAmmoBurn && m_pPlayer->m_flNextAmmoBurn != 1000)
		{

			// Only do 'charge force end from running out of ammo' and charge ammo drop if cheats are off
			if (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteclip) == 0 && EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteammo) == 0) {
				//MODDD - moved here, was below this 'nextAmmoBurn' check.
				// Now runs at the time of the next ammo-burn cycle instead.
				if (m_fireState < chargeAmmoStoredMax - 1 && PlayerPrimaryAmmoCount() < chargeAmmoUsage)
				{
					// out of ammo! force the gun to fire
					StartFire();
					m_fInAttack = 0;
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0;
					m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1;
					//MODDD - why not?
					SetAttackDelays(m_pPlayer->m_flNextAttack);

					return;
				}

				ChangePlayerPrimaryAmmoCount(-chargeAmmoUsage);
			}//END OF cheat check

			m_fireState++;
			m_pPlayer->m_flNextAmmoBurn = UTIL_WeaponTimeBase() + chargeAmmoUsageDelay;
		}

		
		//MODDD - purpose of m_flAmmoStartCharge changed, counts ammo used by the charge, not
		// time to stop charging.
		/*
		if (UTIL_WeaponTimeBase() >= m_pPlayer->m_flAmmoStartCharge)
		{
			// don't eat any more ammo after gun is fully charged.
			m_pPlayer->m_flNextAmmoBurn = 1000;
		}
		*/
		if (m_fireState >= chargeAmmoStoredMax-1) {
			m_pPlayer->m_flNextAmmoBurn = 1000;
		}


		// REPLACO
		//int pitch = (gpGlobals->time - m_flStartCharge) * (150 / GetFullChargeTime()) + 100;

		// ALSO.  Better adjusts for different charge-delay times between gauss_mode choices.
		// Add -chargeInitialDelay in so that time lost waiting for the initial charge does not contribute
		// to the pitch.
		//int pitch = (int)(( -chargeInitialDelay + 100.0f - (pev->fuser1) ) * ((maxPitch-(startPitch) ) / GetFullChargeTime()) +(startPitch));
		
		int pitch;
		float currentChargeTime = (-chargeInitialDelay + 100.0f - (pev->fuser1));
		float daChargeTime = GetFullChargeTime();

		if(currentChargeTime >= daChargeTime){
			// max it goes
			pitch = maxPitch;
		}else{
			float chargeTimeFracto = (currentChargeTime / GetFullChargeTime());
			// go upwards
			if(theGaussMode != 1){
				pitch = chargeTimeFracto * (pitchDelta) + startPitch;
			}else{
				float pitcho;
				// move up the pitch faster at first, then more slowly
				if(chargeTimeFracto < 0.5){
					pitcho = (chargeTimeFracto/0.5) * (pitchDelta) * 0.75 + startPitch;
				}else{
					pitcho = (pitchDelta) * 0.75 + (chargeTimeFracto - 0.5)/0.5 * (pitchDelta) * 0.25 + startPitch;
				}
				pitch = (int)pitcho;
			}
		}

		if(pitch < 50){
			// WARNING!  Pitch should never go under the starting 100 for 0 charge time!
			// This is to avoid that annoying glitchy high-pitched caused by a deep negative value in some bug.
			pitch = 50;
		}

		
		if (pitch > maxPitch){
			pitch = maxPitch;
		}

		/*
		if(m_fireState < chargeAmmoStoredMax-1){
			easyForcePrintLine("OH dear PITCH %.2f / %.2f : %d", currentChargeTime, daChargeTime, pitch);
		}
		*/


		// ALERT( at_console, "%d %d %d\n", m_fInAttack, m_iSoundState, pitch );

		if (m_iSoundState == 0){
			ALERT(at_console, "sound state %d\n", m_iSoundState);
		}

		PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usGaussSpin, 0.0, (float*)&g_vecZero, (float*)&g_vecZero, 0.0, 0.0, pitch, 0, (m_iSoundState == SND_CHANGE_PITCH) ? 1 : 0, 0);

		m_iSoundState = SND_CHANGE_PITCH; // hack for going through level transitions

		m_pPlayer->m_iWeaponVolume = GAUSS_PRIMARY_CHARGE_VOLUME;
		
		// m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.1;

		// REPLACO
		//if (gpGlobals->time > m_flStartCharge + GetFullChargeTime() * 1.3 + 3)
		if (100 - pev->fuser1 > GetFullChargeTime() * 1.3 + 3)
		{
			// Player charged up too long. Zap him.
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/electro4.wav", 1.0, ATTN_NORM, 0, 80 + RANDOM_LONG(0, 0x3f));
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/electro6.wav", 1.0, ATTN_NORM, 0, 75 + RANDOM_LONG(0, 0x3f));

			m_fInAttack = 0;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0;
			m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.0;
			SetAttackDelays(m_pPlayer->m_flNextAttack);

#ifndef CLIENT_DLL
			m_pPlayer->TakeDamage(VARS(eoNullEntity), VARS(eoNullEntity), 50, DMG_SHOCK);
			UTIL_ScreenFade(m_pPlayer, Vector(255, 128, 0), 2, 0.5, 128, FFADE_IN);
#endif
			SendWeaponAnim(GAUSS_IDLE);

			// Player may have been killed and this weapon dropped, don't execute any more code after this!
			return;
		}
	}
}

//=========================================================
// StartFire- since all of this code has to run and then 
// call Fire(), it was easier at this point to rip it out 
// of weaponidle() and make its own function then to try to
// merge this into Fire(), which has some identical variable names 
//=========================================================
void CGauss::StartFire(void)
{
	// NOT GOOD.  Forces Prev to m_fInAttack in whatever state it is, which likely won't be 0 as intended by this.
	// Of course as-is never set m_fInAttack to 0 in StartFire, but always did right after calling it...? what?
	//inAttackPrev = m_fInAttack;

	float flDamage;

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
	Vector vecAiming = gpGlobals->v_forward;
	Vector vecSrc = m_pPlayer->GetGunPosition(); // + gpGlobals->v_up * -8 + gpGlobals->v_right * 8;

	// How much damage will one primary-fire gauss shot do?  Useful for charged damage logic too.
	// gauss_mode affects this (2.5 times as much).
	// And clientside will use hardcoded forms for now, mostly for appearance info there anyway.
	float damagePerShot;

	if (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(gauss_mode) != 1) {
#ifdef CLIENT_DLL
		damagePerShot = 20.0f;
#else 
		damagePerShot = gSkillData.plrDmgGauss;
#endif
	}
	else {
		// ALPHA
#ifdef CLIENT_DLL
		damagePerShot = 20.0f * 2.5;
#else 
		damagePerShot = gSkillData.plrDmgGauss * 2.5;
#endif
	}


	if (!m_fPrimaryFire) {

		if (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(gauss_mode) != 1) {
			// Retail.
			//MODDD - changes here too.  Do damage in solid increments depending on how
			// much ammo was used instead, also starting at 10 damage for 1 ammo used.
			// Maxes changed too, formula goes:
			// 10 + (ammo used while charging - 1) * 14
			// so max damage is now closer to 170.  Also involve gSkillData.plrDmgGauss as a basis for that serverside at least.


			// clientside doesn't get skills.txt, use a fixed one, only used for beam appearance.

			flDamage = damagePerShot * (0.4f + 0.695f * ((float)m_fireState));

			/*
			if (gpGlobals->time - m_flStartCharge > GetFullChargeTime())
			{
				flDamage = 200;
			}
			else
			{
				flDamage = 200 * ((gpGlobals->time - m_flStartCharge) / GetFullChargeTime());
			}
			*/
		}
		else {
			flDamage = damagePerShot * (1.0f + 1.0f * ((float)m_fireState));
		}
	}
	else
	{
		// PRIMARY FIRE
		flDamage = damagePerShot;

	}

	// SAFETY.  Reset m_fireState (ammo cycles passed while charging) now that damage has been decided
	m_fireState = 0;




	if (m_fInAttack != 3)
	{
		//ALERT ( at_console, "Time:%f Damage:%f\n", gpGlobals->time - m_flStartCharge, flDamage );

#ifndef CLIENT_DLL
		// whether to include Z or not comes later.
		float knockbackAmount = 0;

		//MODDD - a cheat may disable the recoil force.
		if (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_nogaussrecoil) == 1) {
			knockbackAmount = 0;
		}
		else {
			// knockback allowed?  From what?  How to decide.
			if (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(gauss_mode) != 1) {
				// retail: secondary does knockback
				if (m_fPrimaryFire) {
					knockbackAmount = 0;
				}
				else {
					// CHANGED, scale a little less for higher damages.
					//knockbackAmount = flDamage * 5;
					if (flDamage <= 20) {
						knockbackAmount = flDamage * (4.7);
					}
					else if (flDamage <= 100) {
						knockbackAmount = 20 * 4.7 + (flDamage - 20) * 4.1;
					}
					else {
						knockbackAmount = 20 * 4.7 + (100 - 20) * 4.1 + (flDamage - 100) * 3.5;
					}
				}
			}else {
				// alpha: primary does knockback.  Should be 'several feet' according to a source.
				if (m_fPrimaryFire) {
					knockbackAmount = 50 * 5.1;
				}
				else {
					if (flDamage <= 40) {
						knockbackAmount = flDamage * 4.5;
					}
					else if (flDamage <= 150) {
						knockbackAmount = 40 * 4.5 + (flDamage - 40) * 3.5;
					}
					else {
						knockbackAmount = 40 * 4.5 + (150 - 40) * 3.5 + (flDamage - 150) * 0.87;
					}
				}
			}

		}//END OF cheat_nogaussrecoil check


		// was !m_fPrimaryFire
		if (knockbackAmount > 0)
		{
			Vector knockBackVec;
			knockBackVec = m_pPlayer->pev->velocity - gpGlobals->v_forward * knockbackAmount;

			//MODDD - the 2nd condition is extra, from a new CVar.  Could allow z-force.
			if (!IsMultiplayer() && EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(gaussRecoilSendsUpInSP) == 0)
			{
				// in deathmatch, gauss can pop you up into the air. Not in single play.
				// (only x & y)
				m_pPlayer->pev->velocity.x = knockBackVec.x;
				m_pPlayer->pev->velocity.y = knockBackVec.y;
			}
			else {
				// allow all knockback, including Z.
				m_pPlayer->pev->velocity.x = knockBackVec.x;
				m_pPlayer->pev->velocity.y = knockBackVec.y;
				m_pPlayer->pev->velocity.z = knockBackVec.z * 0.6;  // less-so though
			}

		}
#endif
		// player "shoot" animation
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
	}

	// time until aftershock 'static discharge' sound
	m_pPlayer->m_flPlayAftershock = gpGlobals->time + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 0.3, 0.8);


	//MODDD - is that good?
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (31.0 / 30.0) + randomIdleAnimationDelay();


	Fire(vecSrc, vecAiming, flDamage);
}

void CGauss::Fire(Vector vecOrigSrc, Vector vecDir, float flDamage)
{
	int dmgDirectBit;
	int dmgDirectBitMod;
	int dmgBlastBit;
	int dmgBlastBitMod;


	if (!m_fPrimaryFire) {

		dmgDirectBit = DMG_BULLET;
		dmgDirectBitMod = DMG_GAUSS;
		dmgBlastBit = DMG_BLAST;
		dmgBlastBitMod = DMG_GAUSS;
	}
	else {
		//MODDD - added DMG_NEVERGIB to primary fire, since alpha gauss logic damage (50) is enough
		// to gib, yet we don't want it to.
		dmgDirectBit = DMG_BULLET | DMG_NEVERGIB;
		dmgDirectBitMod = DMG_GAUSS;
		dmgBlastBit = DMG_BLAST | DMG_NEVERGIB;
		dmgBlastBitMod = DMG_GAUSS;
	}


	m_pPlayer->m_iWeaponVolume = GAUSS_PRIMARY_FIRE_VOLUME;

	Vector vecSrc = vecOrigSrc;
	Vector vecDest = vecSrc + vecDir * 8192;
	edict_t* pentIgnore;
	TraceResult tr, beam_tr;
	float flMaxFrac = 1.0;
	int	nTotal = 0;
	BOOL fHasPunched = FALSE;
	BOOL fFirstBeam = TRUE;
	int	nMaxHits = 10;

	pentIgnore = ENT(m_pPlayer->pev);

#ifdef CLIENT_DLL
	if (m_fPrimaryFire == FALSE){
		g_irunninggausspred = TRUE;
	}
#endif

	// The main firing event is sent unreliably so it won't be delayed.
	PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usGaussFire, 0.0, (float*)&m_pPlayer->pev->origin, (float*)&m_pPlayer->pev->angles, flDamage, 0.0, 0, 0, m_fPrimaryFire ? 1 : 0, 0);

	// This reliable event is used to stop the spinning sound
	// It's delayed by a fraction of second to make sure it is delayed by 1 frame on the client
	// It's sent reliably anyway, which could lead to other delays

	PLAYBACK_EVENT_FULL(FEV_NOTHOST | FEV_RELIABLE, m_pPlayer->edict(), m_usGaussFire, 0.01, (float*)&m_pPlayer->pev->origin, (float*)&m_pPlayer->pev->angles, 0.0, 0.0, 0, 0, 0, 1);


	/*ALERT( at_console, "%f %f %f\n%f %f %f\n",
		vecSrc.x, vecSrc.y, vecSrc.z,
		vecDest.x, vecDest.y, vecDest.z );*/


		//	ALERT( at_console, "%f %f\n", tr.flFraction, flMaxFrac );

#ifndef CLIENT_DLL
	//MODDD - force allow first shot to have a beam (loop run-thru), doing nothing would look odd.
	while ( (fFirstBeam || flDamage >= 10) && nMaxHits > 0)
	{
		nMaxHits--;

		// ALERT( at_console, "." );
		UTIL_TraceLine(vecSrc, vecDest, dont_ignore_monsters, pentIgnore, &tr);


		if (tr.fAllSolid) {
			break;
		}

		//MODDD - moved above the 'pEntity NULL'-check below.
		// What does hitting something or not have to do with making a muzzle flash?
		if (fFirstBeam)
		{
			m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
			fFirstBeam = FALSE;

			nTotal += 26;
		}



		//MODDD
		if (UTIL_PointContents(tr.vecEndPos) == CONTENTS_SKY) {
			// If we hit the sky, HALT!
			// Just end.  No reflecting off of that, no persistent glow, no sparks.  None of that makes sense.
			break;
		}

		// MODDD - added check for flFraction like FireBulletsPlayer does.  I assume that had a point.
		if (tr.flFraction >= 1.0) {
			break;
		}

		CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

		if (pEntity == NULL) {
			break;
		}
		///////////////////////////////////////////////////

		//MODDD - no need for this check, the world knows not to take damage.
		// Methods with TraceAttack methods will reject damage just like ones called by FirePlayerBullets do,
		// which doesn't even do a ThingHit->pev->takedamage check itself anyway
		//if (pEntity->pev->takedamage)
		{
			BOOL useBulletHitSound = TRUE;
			ClearMultiDamage();

			//MODDD - NEW.
			// This is an "AI Sound", or not a real one audible to the player, but one that checks for monsters nearby (distance) and alerts them if they are in hearing range.
			// TODO - egon can get this too, probably.
			attemptSendBulletSound(tr.vecEndPos, m_pPlayer->pev);

			const char* theName = pEntity->getClassname();

			pEntity->TraceAttack(m_pPlayer->pev, flDamage, vecDir, &tr, dmgDirectBit, dmgDirectBitMod, TRUE, &useBulletHitSound);


			//MODDD - Play a texture-hit sound, it is a bullet after all.
			// And just force a bullet type to MP5 here, point is it's not the crowbar
			// NOTICE - just use FirePlayerBullets at this point for better support, whether to do texture-sounds or decals
			// server or clientside is a little more comlpex than just tacking it on here.
			// Entities can make noise this way though, just not the world under default CVars.
			if (useBulletHitSound) {
				TEXTURETYPE_PlaySound(&tr, vecSrc, vecDest, BULLET_PLAYER_MP5);
				DecalGunshot(&tr, BULLET_PLAYER_MP5);
			}

			ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);
		}


		//MODDD NOTE
		// Notice that the default (barely ever touched) "ReflectGauss" requires the thing hit (pEntity) to not be capable of taking damage (not an enemy or breakable).
		// If this is the case (incapable of taking dmg), it is reflectable and a check is done to see whether to reflect off the thing hit (enough degrees) or attempt
		// a punch thru (secondary gauss fire can go some distance through walls).
		// OTHERWISE, capable of taking DMG, this assumes that the beam hit a monster for "pEntity", and dealt damage to it.  Don't try to reflect, just try to go right
		// through.  Notice that the pEntity just hit is set to be the "ignore" entity as to not hit the same one again.  Setting the vecSrc (source) to be the hit
		// location (of the enemy struck) a slight ways forwards for the next line-trace at the next run-thru of the loop (where it may hit another enemy or hit a surface
		// as expected) is also a good sign of this idea.
		// ...Can block the reflection this early, preventing the "punch through" (go through a wall), or allow that much.


		if (pEntity->ReflectGauss())
		{
			float n;

			//MODDD - easy there!  Don't set pentIgnore to NULL so soon, do it if the surface hit warrants a reflection
			// (hit coming at enough of an angle).  Otherwise, the punch-through check that runs instead will
			// find the player at the start of the trace and go 'Oh look, something to punch through.  DAMGE.'
			//pentIgnore = NULL;

			n = -DotProduct(tr.vecPlaneNormal, vecDir);


			// for now, always try.
			BOOL reflectCheckPossible = TRUE;




			if (reflectCheckPossible && n < 0.5) // 60 degrees
			{
				// ALERT( at_console, "reflect %f\n", n );
				// reflect
				Vector r;

				//MODDD - should be safe now
				pentIgnore = NULL;

				r = 2.0 * tr.vecPlaneNormal * n + vecDir;
				flMaxFrac = flMaxFrac - tr.flFraction;
				vecDir = r;
				vecSrc = tr.vecEndPos + vecDir * 8;
				vecDest = vecSrc + vecDir * 8192;

				// explode a bit
				//MODDD NOTE - why was this "m_pPlayer->RadiusDamage"? No need to be specific to the player in the call,
				//             we already send our own PEV and the player's PEV as arguments.
				// No difference in just "RadiusDamage( ... )" alone.
				//m_pPlayer->RadiusDamage(...)
				// Also, Damange no longer depends on the reflection angle, was 'flDamage * n'.
				// 

				// also diminishing returns for extreme damage.
				float radDmg;
				float radRange;
				getRadiusStats(flDamage, radDmg, radRange);


				RadiusDamage(tr.vecEndPos, pev, m_pPlayer->pev, radDmg, radRange, CLASS_NONE, dmgBlastBit, dmgBlastBitMod);

				nTotal += 34;

				// lose energy
				if (n == 0) n = 0.1;
				flDamage = flDamage * (1 - n);
			}
			else
			{
				nTotal += 13;

				// limit it to one hole punch
				if (fHasPunched)
					break;
				fHasPunched = TRUE;


				BOOL punchAttempt = !m_fPrimaryFire;

				// try punching through wall if secondary attack (primary is incapable of breaking through)
				if (punchAttempt)
				{
					BOOL doDirectHitRadDamage = FALSE;

					UTIL_TraceLine(tr.vecEndPos + vecDir * 8, vecDest, dont_ignore_monsters, pentIgnore, &beam_tr);
					if (!beam_tr.fAllSolid)
					{
						// trace backwards to find exit point
						UTIL_TraceLine(beam_tr.vecEndPos, tr.vecEndPos, dont_ignore_monsters, pentIgnore, &beam_tr);
						
						//MODDD - 'n' renamed to 'm' to a void a scope conflict
						float m = (beam_tr.vecEndPos - tr.vecEndPos).Length();

						if (m < flDamage)
						{
							if (m == 0) m = 1;
							flDamage -= m;

							// ALERT( at_console, "punch %f\m", m );
							nTotal += 21;

							//MODDD - redone.
							/*
							// exit blast damage
							//m_pPlayer->RadiusDamage( beam_tr.vecEndPos + vecDir * 8, pev, m_pPlayer->pev, flDamage, CLASS_NONE, DMG_BLAST );
							float damage_radius;

							if ( IsMultiplayer() )
							{
								damage_radius = flDamage * 1.75;  // Old code == 2.5
							}
							else
							{
								damage_radius = flDamage * 2.5;
							}

							::RadiusDamage( beam_tr.vecEndPos + vecDir * 8, pev, m_pPlayer->pev, flDamage, damage_radius, CLASS_NONE, DMG_BLAST, DMG_GAUSS );
							*/

							float radDmg;
							float radRange;
							getRadiusStats(flDamage, radDmg, radRange);

							::RadiusDamage(beam_tr.vecEndPos + vecDir * 8, pev, m_pPlayer->pev, radDmg, radRange, CLASS_NONE, dmgBlastBit, dmgBlastBitMod);


							CSoundEnt::InsertSound(bits_SOUND_COMBAT, pev->origin, NORMAL_EXPLOSION_VOLUME, 3.0);

							nTotal += 53;

							vecSrc = beam_tr.vecEndPos + vecDir;
						}
						else {
							//MODDD - NEW.
							// So couldn't punch through something in the way.  Why do the loop again?
							// Nothing about vecSrc nor vecDest changed, so the same trace is being done again.
							// Except this time, with a NULL pentIgnore.  Redoing the trace from the player's weapon
							// to where the player is looking is likely to register a hit on the player from occuring 
							// within the player's bounds without being told to ignore that.
							doDirectHitRadDamage = TRUE;
						}
					}
					else
					{
						//ALERT( at_console, "blocked %f\n", n );
						doDirectHitRadDamage = TRUE;
					}

					//MODDD - NEW.  Only happen if a reflection or pierce hasn't already done this by this point.
					if (doDirectHitRadDamage) {
						float radDmg;
						float radRange;
						getRadiusStats(flDamage, radDmg, radRange);

						::RadiusDamage(tr.vecEndPos + vecDir * 8, pev, m_pPlayer->pev, radDmg, radRange, CLASS_NONE, dmgBlastBit, dmgBlastBitMod);

						// And lastly, remove all damage.  doDirectHitRadDamage was only set to TRUE in places that reset this.
						flDamage = 0;
					}//END OF doDirectHitRadDamage check


				}
				else
				{
					//ALERT( at_console, "blocked solid\n" );

					flDamage = 0;
				}

				//MODDD - now it is safe to turn this off.
				pentIgnore = NULL;

			}
		}
		else
		{
			BOOL canPierce = TRUE;

			if (canPierce) {

				vecSrc = tr.vecEndPos + vecDir;
				pentIgnore = ENT(pEntity->pev);

			}
			else {
				break;  //end
			}
		}
	}
#endif
	// ALERT( at_console, "%d bytes\n", nTotal );
}




void CGauss::onFreshFrame(void){

	BOOL holdingSecondary = ((m_pPlayer->pev->button & IN_ATTACK2) && m_flNextSecondaryAttack <= 0.0);
	BOOL holdingPrimary = ((m_pPlayer->pev->button & IN_ATTACK) && m_flNextPrimaryAttack <= 0.0);

	float framesSinceRestore = getFramesSinceRestore();

	if(framesSinceRestore == 0 || (holdingSecondary || holdingPrimary) ){
		ignoreIdleTime = gpGlobals->time + 0.3;
	}

	if(framesSinceRestore == 0){
		//g_firstFrameSinceRestore -= 1;

		// ???????????????????
		//m_fInAttack = 0;
		//inAttackPrev = 0;

		if(m_fInAttack == 1){
			SendWeaponAnim(GAUSS_SPINUP);
		}else if(m_fInAttack == 2){
			SendWeaponAnim(GAUSS_SPIN);
		}
	}//framesSinceRestore check

}//onFreshFrame


void CGauss::ItemPreFrame( void ){

	CBasePlayerWeapon::ItemPreFrame();
}




void CGauss::ItemPostFrame(void){

	/*
	if (!m_pPlayer->m_bHolstering) {
	// If the player is putting the weapon away, don't do this!
	UpdateHitCloud();
	}
	*/

	CBasePlayerWeapon::ItemPostFrame();
}


void CGauss::ItemPostFrameThink(void){
	if(getFramesSinceRestore() < 3){
		onFreshFrame();
	}

	// firestate is how much ammo has been added to a charge in progress
	// m_fInAttack is what phase charging is in (0: none, 1: startup + one-time delay, 2: charge-loop to further add ammo)

	//easyForcePrintLine("I AM gauss fs:%d ia:%d", m_fireState, m_fInAttack);

	BOOL holdingSecondary = ((m_pPlayer->pev->button & IN_ATTACK2) && m_flNextSecondaryAttack <= 0.0);
	BOOL holdingPrimary = ((m_pPlayer->pev->button & IN_ATTACK) && m_flNextPrimaryAttack <= 0.0);
	
		//easyForcePrintLine("WHATS GOOD charstag:%d ammochar:%d", m_fInAttack, m_fireState);
//#ifdef CLIENT_DLL
		//easyForcePrintLine("WHATS GOOD nextprim:%.2f", m_flNextPrimaryAttack);
//#endif

	BOOL forceIdle = FALSE;

	if(!m_pPlayer->m_bHolstering){
		if((holdingPrimary && holdingSecondary) ){
			//m_chargeReady &= ~32;
			///WeaponIdle();
			//return;
			// try me
			holdingPrimary = FALSE;
			holdingSecondary = FALSE;
			forceIdle = TRUE;
		}
	}else{
		// holstering?  Block inputs, but don't forceIdle.
		holdingPrimary = FALSE;
		holdingSecondary = FALSE;
	}


	if(holdingSecondary  ){
		_SecondaryAttack();
	}else if(holdingPrimary){
		_PrimaryAttack();
	}



	if(m_fInAttack == 0 && inAttackPrev != 0){
		// FUCK SHIT AVENUE
		StartFire();
		m_fInAttack = 0;
		inAttackPrev = 0;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0;

		//MODDD - why not?
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.4;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.6;
	}

	inAttackPrev = m_fInAttack;

	
	if(forceIdle){
		WeaponIdle();
	}else{
		CBasePlayerWeapon::ItemPostFrameThink();
	}
}





void CGauss::WeaponIdle(void)
{

	if(gpGlobals->time < ignoreIdleTime){
		return;  //not yet
	}


	ResetEmptySound();

	// play aftershock static discharge
	if (m_pPlayer->m_flPlayAftershock && m_pPlayer->m_flPlayAftershock < gpGlobals->time)
	{
		switch (RANDOM_LONG(0, 3))
		{
		case 0:	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/electro4.wav", RANDOM_FLOAT(0.7, 0.8), ATTN_NORM); break;
		case 1:	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/electro5.wav", RANDOM_FLOAT(0.7, 0.8), ATTN_NORM); break;
		case 2:	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/electro6.wav", RANDOM_FLOAT(0.7, 0.8), ATTN_NORM); break;
		case 3:	break; // no sound
		}
		m_pPlayer->m_flPlayAftershock = 0.0;
	}

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (m_fInAttack != 0)
	{
		StartFire();
		m_fInAttack = 0;
		inAttackPrev = 0;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0;

		//MODDD - why not?
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.4;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.6;
	}
	else
	{
		int iAnim;
		float flRand = RANDOM_FLOAT(0, 1);
		if (flRand <= 0.5)
		{
			iAnim = GAUSS_IDLE;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
		}
		else if (flRand <= 0.75)
		{
			iAnim = GAUSS_IDLE2;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
		}
		else
		{
			iAnim = GAUSS_FIDGET;
			//MODDD - no, use the normal fidget delay
			//m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (71.0f + 1.0f) / (30.0f);
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
		}

		forceBlockLooping();
		//MODDD - 'return' found here, as-is.   Always stopped?  Why?
		//return;
		SendWeaponAnim(iAnim);

	}
}






class CGaussAmmo : public CBasePlayerAmmo
{
	void Spawn(void)
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_gaussammo.mdl");
		CBasePlayerAmmo::Spawn();
	}
	void Precache(void)
	{
		PRECACHE_MODEL("models/w_gaussammo.mdl");
		precacheAmmoPickupSound();
	}
	BOOL AddAmmo(CBaseEntity* pOther)
	{
		if (pOther->GiveAmmo(AMMO_URANIUMBOX_GIVE, "uranium", URANIUM_MAX_CARRY) != -1)
		{
			playAmmoPickupSound();

			//MODDD - Could not find any sentences or clips involving Gauss ammo, specifically (uranium).
			//Perhaps this is okay for both the Gauss and the Egon?
			//UPDATE: apparently, the Gauss's ammo is the universal, and the egon's ammo is just the
			//mp5 chain ammo (seems really unfitting, I think it was a placeholder).
			if (pOther->IsPlayer()) {
				CBasePlayer* pPlayer = (CBasePlayer*)pOther;
				pPlayer->SetSuitUpdate("!HEV_EGONPOWER", FALSE, SUIT_NEXT_IN_20MIN);
			}

			return TRUE;
		}
		return FALSE;
	}
};
LINK_ENTITY_TO_CLASS(ammo_gaussclip, CGaussAmmo);



