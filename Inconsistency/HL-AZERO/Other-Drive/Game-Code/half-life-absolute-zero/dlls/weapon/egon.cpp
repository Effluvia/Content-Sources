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



//IMPORTANT NOTE: most cases of "UTIL_WeaponTimeBase" have been replaced with "gpGlobals->time".
//"UTIL_WeaponTimeBase" seems to return only 0 here, and that is not helpful for some things.


// GENERAL NOTE (although true since retail) - damage is only dealt to what's hit during frames that
// use ammo.  Any others are just for show.

// The new scorch-mark effect is often drawn during frames that don't use ammo / deal damage.
// So things that aren't damage are likely to get scorch marks (breakables most often).  If this is a problem,
// say so.   Although there could be a compromise:  if the thing hit is of BSP geometry but not the map itself
// (func_breakable or some child class), don't draw a decal UNLESS this frame dealt damage.
// ya know what that sounds right.


#pragma once

#include "extdll.h"
#include "egon.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "basemonster.h"

#include "nodes.h"
#include "effects.h"
#include "customentity.h"
#include "gamerules.h"

#include "decals.h"


EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelay)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelaycustom)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteclip)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteammo)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(egonEffectsMode);
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(egonHitCloud);
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(egonRadiusDamageMode)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(egonFireRateMode)



//MODDD - several things moved to the new egon.h for commonly including client/serverside
// (less redundancy in ev_hldm.cpp)

#define EGON_SWITCH_NARROW_TIME			0.75			// Time it takes to switch fire modes
#define EGON_SWITCH_WIDE_TIME			1.5

// how often to place scorches while firing?  The delay begins after firing, so firing for the minimum amount of time
// (enough to use 1 unit of ammo) will not leave a scorch.
#define EGON_PRIMARY_SCORCH_INTERVAL	0.044 //0.042 //0.18
#define EGON_SECONDARY_SCORCH_INTERVAL	0.058 //0.055 //0.27



// TEST?
// nope, unwise to drop that flag.  Go between levels with the egon on, looks fine at first but come back to the previous level...
// the effect is awkwardly stuck from the muzzle of the play weapon to the place the player was at the time of the transition
// away from this first level.
#define SUB_SF_BEAM_TEMPORARY SF_BEAM_TEMPORARY
//#define SUB_SF_BEAM_TEMPORARY 0



#ifdef CLIENT_DLL
// Here are all the remaining fucks I give
int fuckfuckfuckfuckfuck = 0;
BOOL FuckFlag = FALSE;
#endif



LINK_ENTITY_TO_CLASS( weapon_egon, CEgon );




//MODDD - IMPORTANT NOTICE.  All mentions of "m_fireMode" have been replaced with "m_fInAttack", as "m_fireMode" is not well synchronized between
//			the server and client.  "m_fInAttack" was never used in here before, so it may be back-replaced for whatever reason.




//BOOL usedOneAmmo = FALSE;

//MODDD
void CEgon::customAttachToPlayer(CBasePlayer *pPlayer ){
	m_pPlayer->SetSuitUpdate("!HEV_EGON", FALSE, SUIT_NEXT_IN_30MIN);
}

void CEgon::Spawn( )
{
	Precache( );
	m_iId = WEAPON_EGON;
	SET_MODEL(ENT(pev), "models/w_egon.mdl");

	m_iClip = -1;
	m_iDefaultAmmo = EGON_DEFAULT_GIVE;

	//MODDD - set default firemode
	//m_fInAttack = FIRE_WIDE;
	//MODDD - default is now "m_fInAttack" instead.
	m_fInAttack = FIRE_NARROW;

	FallInit();// get ready to fall down.
}


void CEgon::Precache( void )
{
	PRECACHE_MODEL("models/w_egon.mdl");
	PRECACHE_MODEL("models/v_egon.mdl");
	PRECACHE_MODEL("models/p_egon.mdl");

	PRECACHE_MODEL("models/w_9mmclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");

	PRECACHE_SOUND( EGON_SOUND_OFF );
	PRECACHE_SOUND( EGON_SOUND_RUN );
	PRECACHE_SOUND( EGON_SOUND_STARTUP );

	PRECACHE_SOUND ("weapons/357_cock1.wav");

	precacheGunPickupSound();


	PRECACHE_MODEL( EGON_BEAM_SPRITE );
	PRECACHE_MODEL( EGON_FLARE_SPRITE );


	m_usEgonFire = PRECACHE_EVENT ( 1, "events/egon_fire.sc" );
	m_usEgonStop = PRECACHE_EVENT ( 1, "events/egon_stop.sc" );
}


BOOL CEgon::Deploy( void )
{
	timeSinceDeployed = 0;
	currentTime = gpGlobals->time;

	m_deployed = FALSE;
	m_fireState = FIRE_OFF;
	return DefaultDeploy( "models/v_egon.mdl", "models/p_egon.mdl", EGON_DRAW, "egon", 0, 0, (16.0/30.0), -1 );
}

int CEgon::AddToPlayer( CBasePlayer *pPlayer )
{
	if ( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev );
			WRITE_BYTE( m_iId );
		MESSAGE_END();
		return TRUE;
	}
	return FALSE;
}



void CEgon::Holster( int skiplocal /* = 0 */ )
{
	//m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

	//SendWeaponAnim( EGON_HOLSTER );

	//MODDD - at holster, force to the default position.  Firemode is adjusted at fire time, not needed here.
	
	setchargeReady(5 + -4);

	m_flStartThrow = -1;


    EndAttack();

	DefaultHolster(EGON_HOLSTER, skiplocal, 0, (16.0f/30.0f) );
}

int CEgon::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "uranium";
	p->iMaxAmmo1 = URANIUM_MAX_CARRY;


	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;

	//p->pszAmmo2 = "uranium";
	//p->iMaxAmmo2 = URANIUM_MAX_CARRY;


	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 3;
	//MODDD - RPG moved out.
	//p->iPosition = 2;
	p->iPosition = 1;

	p->iId = m_iId = WEAPON_EGON;
	p->iFlags = 0;
	p->iWeight = EGON_WEIGHT;

	return 1;
}

#define EGON_PULSE_INTERVAL			0.1
#define EGON_DISCHARGE_INTERVAL		0.1

float CEgon::GetPulseInterval( void )
{
	//MODDD
	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelay) == 0){
		return EGON_PULSE_INTERVAL;
	}else{
		return EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelaycustom);
	}
}

float CEgon::GetDischargeInterval( void )
{
	//MODDD
	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelay) == 0){
		return EGON_DISCHARGE_INTERVAL;
	}else{
		return EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelaycustom);
	}
}

BOOL CEgon::HasAmmo( void )
{

	if ( m_pPlayer->ammo_uranium <= 0 )
		return FALSE;
		
	/*
	if ( PlayerPrimaryAmmoCount() <= 0 )
	{
		//PlayEmptySound( );
		//m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
		return FALSE;
	}
	*/


	return TRUE;
}

void CEgon::UseAmmo( int count )
{
	//usedOneAmmo = TRUE;
	m_chargeReady |= 32;

	//MODDD - could the "infinite ammo cheat" be useful here?
	// So yea, only use ammo if the cheat is off.  Other weapon-related behavior should happen still.
	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteammo) == 0){
		if (PlayerPrimaryAmmoCount() >= count) {
			//m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= count;
			ChangePlayerPrimaryAmmoCount(-count);
		}
		else {
			//m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = 0;
			SetPlayerPrimaryAmmoCount(0);
		}
	}

}






//MODDD - undone the "UTIL_WeaponTimeBase  --->  gpGlobals->time"   replacements seen before throughout this method.
//Apparently, this causes a glitch where the egon fire sound does not re-loop when playing for more than several seconds.
//Perhaps the client does just fine with "UTIL_WeaponTimeBase" or something?
void CEgon::Attack( void )
{

	// don't fire underwater
	if ( m_pPlayer->pev->waterlevel == 3 )
	{
		
		if ( m_fireState != FIRE_OFF || m_pBeam )
		{
			EndAttack();

			switch ( m_fInAttack )
			{
			case FIRE_NARROW:
				setchargeReady(5 + -2);
			break;
			case FIRE_WIDE:
				setchargeReady(5 + -1);
			break;
			}


		}
		else
		{
			PlayEmptySound( );
		}
		return;
	}
	
#ifdef CLIENT_DLL

	if(m_fireState != 1 && lockedFireState){
		// no changin yet.  Weird-ass server/client synch trying to force it back.
		m_fireState = 1;
	}

#endif



	UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );
	Vector vecAiming = gpGlobals->v_forward;
	Vector vecSrc	 = m_pPlayer->GetGunPosition( );
	
	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif



	//easyForcePrintLine("egon fs:%d beam?:%d  fuserreq?: %.2f <= %.2f is it? %d has dat ammo? %d amount? %d", m_fireState, (m_pBeam!=NULL), m_flReleaseThrow, UTIL_WeaponTimeBase(), (m_flReleaseThrow <= UTIL_WeaponTimeBase()), HasAmmo(), m_pPlayer->ammo_uranium );
	//easyForcePrintLine("egon fs:%d beam?:%d  fuserreq?: %.2f <= %.2f is it? %d has dat ammo? %d amount? %d", m_fireState, (m_pBeam!=NULL), pev->fuser1, UTIL_WeaponTimeBase(), (pev->fuser1 <= UTIL_WeaponTimeBase()), HasAmmo(), m_pPlayer->ammo_uranium );

	// FEV_CLIENT?
	flags |= FEV_GLOBAL | FEV_RELIABLE;


#ifdef CLIENT_DLL
	if(fuckfuckfuckfuckfuck > 0){

		if(FuckFlag == FALSE){
			//easyForcePrintLine("fuckfuckfuckfuckfuck");
			PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usEgonFire, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, m_fireState, m_fInAttack, 1, 0 );
		}else{
			PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usEgonFire, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, m_fireState, m_fInAttack, 0, 1 );
		}
		fuckfuckfuckfuckfuck--;
	}else{
		// expiyah
		FuckFlag = FALSE;

	}
#endif


	switch( m_fireState )
	{
		case FIRE_OFF:
		{
			fireExceptionStartTime = gpGlobals->time;

			if ( !HasAmmo() )
			{
				//m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.25;
				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.25;
				PlayEmptySound( );

				switch ( m_fInAttack )
				{
				case FIRE_NARROW:
					setchargeReady(5 + -2);
				break;
				case FIRE_WIDE:
					setchargeReady(5 + -1);
				break;
				}

				return;
			}

			lockedFireState = TRUE;
			
			m_flAmmoUseTime = gpGlobals->time;// start using ammo ASAP.


			easyPrintLine("PLAYBACK_EVENT_FULL egstar IgnoreIdleTime over? %d", (gpGlobals->time >= ignoreIdleTime) );
			if(gpGlobals->time >= ignoreIdleTime){
				// ok
				PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usEgonFire, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, m_fireState, m_fInAttack, 1, 0 );
			}else{
				/*
				// coming from transition, use the other flag ya hooligan
				PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usEgonFire, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, m_fireState, m_fInAttack, 0, 1 );
				// ALSO.  Don't play the continual fire sound later like normal logic would, it's meant 
				// to play after the startup sound ends but since this is starting with the cointonual 
				// sound it just overlaps with the one already playing and sounds odd.
				easyPrintLine("***BEEPO***");
				pev->fuser1 = UTIL_WeaponTimeBase() + 1000;
				*/
			}
				
#ifdef CLIENT_DLL
			// just send the event clientside 5 times, surely one of them will take.           sweet god this engine.
			fuckfuckfuckfuckfuck = 5;
			//FuckFlag = FALSE;
#endif

			m_shakeTime = 0;

			m_pPlayer->m_iWeaponVolume = EGON_PRIMARY_VOLUME;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.1;
			
			// this var does not count down, use gpGlobals->time
			//m_flReleaseThrow = UTIL_WeaponTimeBase() + 2;
			//m_flReleaseThrow = gpGlobals->time + 2;
			pev->fuser1 = UTIL_WeaponTimeBase() + 2;


			pev->dmgtime = gpGlobals->time + GetPulseInterval();
			m_fireState = FIRE_CHARGE;
			stopBlockLooping();

			// Start the scorch interval so that firing for a single frame doesn't leave a mark.
			// Also, this will be the first in this stream of firing.
			previousScorchLocSet = FALSE;
			recentlyDamagedEntity = NULL;
			if(m_fInAttack == FIRE_NARROW){
				nextScorchInterval = gpGlobals->time + EGON_PRIMARY_SCORCH_INTERVAL;
			}else{
				nextScorchInterval = gpGlobals->time + EGON_SECONDARY_SCORCH_INTERVAL;
			}

		}
		break;

		case FIRE_CHARGE:
		{
			Fire( vecSrc, vecAiming );
			//MODDD - the volume sent out to alert monsters is greater for "wide" fire.
			//m_pPlayer->m_iWeaponVolume = EGON_PRIMARY_VOLUME;


			if(m_fInAttack == FIRE_NARROW){
				m_pPlayer->m_iWeaponVolume = EGON_PRIMARY_VOLUME + 100;
			}else if(m_fInAttack == FIRE_WIDE){
				m_pPlayer->m_iWeaponVolume = EGON_PRIMARY_VOLUME + 275;
			}
			
			//if ( m_flReleaseThrow <= UTIL_WeaponTimeBase() )
			//if(gpGlobals->time >= m_flReleaseThrow)
			if(!blockContinualFireSound && pev->fuser1 <= UTIL_WeaponTimeBase())
			{
				easyPrintLine("PLAYBACK_EVENT_FULL egloop");
				PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usEgonFire, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, m_fireState, m_fInAttack, 0, 0 );
				//m_flReleaseThrow = 1000;
				//m_flReleaseThrow = gpGlobals->time + 1000;

				pev->fuser1 = UTIL_WeaponTimeBase() + 1000;

			}

			if ( !HasAmmo() )
			{
				//easyForcePrintLine("GONNA GIVE IT YO YA 1");
				EndAttack();
				// ran out cus' you be out of ammo?  Freeze until the player lets go of the mouse.
				
				//if(getchargeReady() == 5 + 1){
					
				//}
					switch ( m_fInAttack )
					{
					case FIRE_NARROW:
						setchargeReady(5 + -2);
					break;
					case FIRE_WIDE:
						setchargeReady(5 + -1);
					break;
					}


				//m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0;
				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0;

				//???
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + randomIdleAnimationDelay();
				stopBlockLooping();
			}

		}
		break;
	}
}





//MODDD - new vars
CEgon::CEgon(void){


	// VARS:
	//---------------------------------------------------------------------------------------------------------------------
	//m_flReleaseThrow ->  "animationSequence".  Which animation to perform next?
	// NOTICE - changed to ChargeReady (use accessor methods to work with this).  And start counting up from 1 instead.
	// m_flReleaseThrow will replace pev->fuser1 instead.     NEEEEeeeeevermind.   It randomly goes to a hideously out-of-date
	// time, so that's just ggggrrrrrreeeeeaaaaaaaattttt.  Nothing wrong with pev->fuser1.
	// (add 5 to all these)
	// -3 = firing PRIMARY (narrow now).   Resets to -1 when released.
	// -2 = idle (wait for release; not in effect, so same effect as -1).
	// -1 = idle (wait to be triggered by mouse press at all)
	//  0 = ALTFIREON.  turn the top lever to the bottom.
	//  1 = ALTFIRECYCLE - attacking.  Firing the "wide" beam (as opposed to the NOW default "narrow"; tradeoff for delay).
	//  2 = ALTFIREOFF.  turn the lever at the bottom back to the top.       After that, resumes at "idle", or -1.
	//---------------------------------------------------------------------------------------------------------------------
	//m_flStartThrow -> "currentDelay".  When do I "advance" this animation, if applicable?
	//-1 - non applicable.  Used for idling and firing (not timed).
	//anything else - starts as on offset of "gpGlobals->time" (the current time), and is compared against the current
	//                time to see when to change "m_flReleaseThrow" typically.

	//These existing vars with now irrelevant names are used instead of new ones since they're synced through the DLL's, which are
	//close to impossible to mod.  Just working with what I have (and that, well, works).


	lockedFireState = FALSE;
	
	canStartSecondary = TRUE;
	secondarySwitched = FALSE;

	queueAnim = -1;


	oldTime = 0;
	currentTime = 0;

	timeDelta = 0;

	lastSentAnim = -1;
	


	timeSinceDeployed = 0;

	oldTime = -1;
	currentTime = -1;

	timeDelta = 0;

	legalHoldSecondary = FALSE;
	startedSecondaryHoldAttempt = FALSE;
	holdingSecondaryCurrent = 0;
	animationTime = 0;


	holdingSecondaryTarget0 = 1.533f - 0.92;
	holdingSecondaryTarget1 = 1.533f;
	holdingSecondaryTarget2 = 0.71f;


	toggledYet = FALSE;

	altFireOn = FALSE;

	animationSequence = -1;
	//for "idle".

	//m_chargeReady = -1;  //corresponds to animationSequence, better for sync with the client.


	//Timer.
	m_flStartThrow = -1;

	//animation index.
	setchargeReady(5 + -4);
	//-4: true idle (idle locked).
	//-3: true idle.  Fire anytime.
	//-2: locked idle, primary.  Freezes to any idle anim.
	//-1: locked idle, secondary.  Freezes to secondary firing anim.
	//0: secondary: turn the switch on.
	//1: firing, primary.
	//2: firing, secondary.
	//3: turning off (secondary)

	effectsExist = FALSE;
	recentFireDirection = g_vecZero;
	recentHitPlaneNormal = g_vecZero;

	m_flReleaseThrow = 0;  // ???

	fireExceptionPrev = FALSE;
	fireExceptionStartTime = -1;
	fireStatePrev = -1;
	nextScorchInterval = -1;
	previousScorchLocSet = FALSE;
	recentlyDamagedEntity = NULL;
	ignoreIdleTime = -1;

	blockContinualFireSound = FALSE;


}//END OF CEgon constructor






// Save/restore for serverside only!
#ifndef CLIENT_DLL
TYPEDESCRIPTION	CEgon::m_SaveData[] =
{
	//	DEFINE_FIELD( CEgon, m_pBeam, FIELD_CLASSPTR ),
	//	DEFINE_FIELD( CEgon, m_pNoise, FIELD_CLASSPTR ),
	//	DEFINE_FIELD( CEgon, m_pSprite, FIELD_CLASSPTR ),
	DEFINE_FIELD(CEgon, m_shootTime, FIELD_TIME),
	DEFINE_FIELD(CEgon, m_fireState, FIELD_INTEGER),
	DEFINE_FIELD(CEgon, m_fireMode, FIELD_INTEGER),
	DEFINE_FIELD(CEgon, m_shakeTime, FIELD_TIME),
	DEFINE_FIELD(CEgon, m_flAmmoUseTime, FIELD_TIME),
	//MODDD - new synched vars used, save them
	DEFINE_FIELD(CEgon, m_fInAttack, FIELD_INTEGER),
	DEFINE_FIELD(CEgon, m_chargeReady, FIELD_INTEGER),

	// no, don't save this directly.  Use another var as a temporary place.
	// No idea why pev-> stuff is cursed when it comes to saving.
	//DEFINE_FIELD(CEgon, pev->fuser1, FIELD_FLOAT),
	DEFINE_FIELD(CEgon, fuser1_store, FIELD_FLOAT),
	
	// do we care about m_flStartThrow, pev->fuser1?  probably not.

};
// custom implementations instead, see below.
//IMPLEMENT_SAVERESTORE( CEgon, CBasePlayerWeapon );

int CEgon::Save(CSave& save){
	if (!CBasePlayerWeapon::Save(save))
		return 0;

	// Interesting, check out cbase.cpp's 'WriteEntVars', which involves util.cpp's 'gEntvarsDescription' list.
	// So it is possible to save pev-> values like 'pev->fuser1', although I don't know if that should be trusted
	// anywhere else.  This engine is mad at times, keeping the 'fuser1_store' approach.  Not a net difference
	// anyway, it's still some float getting written/restored all the same.
		
	fuser1_store = pev->fuser1;

	return save.WriteFields("CEgon", this, m_SaveData, ARRAYSIZE(m_SaveData));
}

// REMEMBER!!! Client does not call Restore!
int CEgon::Restore(CRestore& restore){

	if (!CBasePlayerWeapon::Restore(restore))
		return 0;


	// TEST!!!   Going between levels?  Forget you created effects, re-do if needed.
	// Unless this is more of a clientside issue (resend the event to start effects there).
	// Is weird the sprite still exists when you think about it...?  yea, clientisde probably.
	//effectsExist = TRUE;


	//if(m_pPlayer != NULL){
		//m_pPlayer->TabulateAmmo();
	//}

	int result = restore.ReadFields("CEgon", this, m_SaveData, ARRAYSIZE(m_SaveData));
	
	
	// Value of 0 never used, begin at 1 then.
	if(getchargeReady() == 5 + -5){
		setchargeReady(5 + -4);
	}

	pev->fuser1 = fuser1_store;


	return result;
}
#endif


void CEgon::onFreshFrame(void){

	BOOL holdingSecondary = ((m_pPlayer->pev->button & IN_ATTACK2) && m_flNextSecondaryAttack <= 0.0);
	BOOL holdingPrimary = ((m_pPlayer->pev->button & IN_ATTACK) && m_flNextPrimaryAttack <= 0.0);

	float framesSinceRestore = getFramesSinceRestore();

	if(framesSinceRestore == 0 || holdingSecondary || holdingPrimary){
		easyPrintLine("EGON: ignoreIdleTime");
		ignoreIdleTime = gpGlobals->time + 0.3;
	}

	if(framesSinceRestore == 0){

		//easyForcePrintLine("IM rather MAD");

		m_fireState = FIRE_OFF;
		fireStatePrev = FIRE_OFF;
		lockedFireState = FALSE;

		/*
		effectsExist = FALSE;
		DestroyEffect();


		if(m_pPlayer != NULL){
		BOOL bMakeNoise = FALSE;
		if ( m_fireState != FIRE_OFF ) //Checking the button just in case!.
		bMakeNoise = TRUE;
		PLAYBACK_EVENT_FULL( FEV_GLOBAL | FEV_RELIABLE, m_pPlayer->edict(), m_usEgonStop, 0, (float *)&m_pPlayer->pev->origin, (float *)&m_pPlayer->pev->angles, 0.0, 0.0, bMakeNoise, 0, 0, 0 );
		}
		*/


		int maCharge = getchargeReady();

		easyPrintLine("HOW GOES IT MY MAN %d", maCharge);

		// firing in primary or secondary?
		if(maCharge == 5 + 1 || maCharge == 6 + 1){
			if(m_pPlayer != NULL){

				BOOL bMakeNoise = FALSE;
				if ( m_fireState != FIRE_OFF ) //Checking the button just in case!
					bMakeNoise = TRUE;

				int flags;
	#if defined( CLIENT_WEAPONS )
				flags = FEV_NOTHOST;
	#else
				flags = 0;
	#endif

	#ifdef CLIENT_DLL
				fuckfuckfuckfuckfuck = 5;
				FuckFlag = TRUE;
	#endif
				flags |= FEV_GLOBAL | FEV_RELIABLE;
				// 2nd bool used instead.  That means, create the effect but use the running-sound instead of the startup sound.
				PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usEgonFire, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, m_fireState, m_fInAttack, 0, 1 );

				blockContinualFireSound = TRUE;
				// don't play the continual sound later, it's already the first playing for firing while going over a transition
				// Why isn't setting it here working??  No blimey idea
				//pev->fuser1 = UTIL_WeaponTimeBase() + 1000;
			}
		}


		m_chargeReady |= 32;  // let me fire damn you
	}//framesSinceRestore check
}



void CEgon::PrimaryAttack( void )
{

}



void CEgon::PrimaryNotHeld( void ){

}


void CEgon::NeitherHeld( void ){

}




void CEgon::BothHeld( void ){


	/*
	easyPrintLine("BOTH HELD??????");
	//pretend that everything is released.  The user should never hold both buttons down, so just give up trying to process that.
	*/

	/*
	PrimaryNotHeld();
	SecondaryNotHeld();
	NeitherHeld();

	*/

	
	
	//PrimaryNotHeld();
	//SecondaryAttack();
}

void CEgon::SecondaryNotHeld( void ){
	
}



void CEgon::SecondaryAttack( void )
{

}//END OF SecondaryAttack



void CEgon::ItemPreFrame( void ){

	CBasePlayerWeapon::ItemPreFrame();
}




void CEgon::ItemPostFrame(void){
	/*
	if (!m_pPlayer->m_bHolstering) {
	// If the player is putting the weapon away, don't do this!
	UpdateHitCloud();
	}
	*/

	CBasePlayerWeapon::ItemPostFrame();
}


void CEgon::ItemPostFrameThink(void){

	BOOL sendFidgetOnOff = TRUE;

	if(getFramesSinceRestore() < 3){
		onFreshFrame();
	}

	BOOL holdingSecondary = ((m_pPlayer->pev->button & IN_ATTACK2) && m_flNextSecondaryAttack <= 0.0);
	BOOL holdingPrimary = ((m_pPlayer->pev->button & IN_ATTACK) && m_flNextPrimaryAttack <= 0.0);
	
	BOOL fireException = (m_fireState == FIRE_CHARGE && !(m_chargeReady & 32) && HasAmmo());

	//BOOL fireException = FALSE;


	//easyForcePrintLine("HOW I BE DOIN p?%d:%d s?%d:%d char:%d", (m_pPlayer->pev->button & IN_ATTACK), (m_flNextPrimaryAttack <= 0.0), (m_pPlayer->pev->button & IN_ATTACK2), (m_flNextSecondaryAttack <= 0.0), getchargeReady() );

	BOOL forceIdle = FALSE;

	//if(!(m_pPlayer->m_bHolstering)){
		if(holdingPrimary && holdingSecondary) {
			if(!fireException &&
				(
				(m_fireState != FIRE_OFF) ||
					(fireExceptionPrev) ||
					m_pBeam != NULL
					)   // dear god I'm tired of this shit
			)
			{
				//easyForcePrintLine("ATTAK END BOTH A");
				//EndAttack(TRUE);

			}else if(fireException){
				// USE THAT AMMO
				//ChangePlayerPrimaryAmmoCount(-1);
				//m_chargeReady &= ~32;
				//easyForcePrintLine("ATTAK END BOTH B");
				//EndAttack(TRUE);

			}

			fireException = FALSE;
			m_chargeReady &= ~32;

			///WeaponIdle();
			//return;

			// try me
			holdingPrimary = FALSE;
			holdingSecondary = FALSE;
			forceIdle = TRUE;
		}
	//}else{
	//	// holstering?  Block inputs, but don't forceIdle.
	//	holdingPrimary = FALSE;
	//	holdingSecondary = FALSE;
	//}



	/*

	if(holdingPrimary && holdingSecondary){
		//m_flTimeWeaponIdle = -1;  ???
		easyForcePrintLine("IM HOLDING BOTH??? %.2f", gpGlobals->time);


		if(getchargeReady() >= 5 + 0){
			switch ( m_fInAttack )
				{
				case FIRE_NARROW:
					setchargeReady(5 + -2);
				break;
				case FIRE_WIDE:
					setchargeReady(5 + -1);
				break;
			}

			//sendFidgetOnOff = FALSE;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + randomIdleAnimationDelay();
		}
		holdingPrimary = FALSE;
		holdingSecondary = FALSE;

		//WeaponIdle();
		//return;   //block!
	}
	*/


	// Keep firing until at least one unit of ammo has been used!
	if(fireException){
		if(getchargeReady() == 5 + 2){
			// Firing in secondary?  ok
			m_fInAttack = FIRE_WIDE;
			Attack();
		}else if(getchargeReady() == 5 + 1){
			// Firing in primary? ok
			m_fInAttack = FIRE_NARROW;
			Attack();
		}else{
			// ???
			fireException = FALSE;
		}

	}else
	// if firing in secondary and haven't used a single ammo, keep firing
	if(holdingSecondary  ){
		if(getchargeReady() == 5 + -4 || getchargeReady() == 5 + -3){
			// ready to begin.
			setchargeReady(5 + 0);
			m_flStartThrow = gpGlobals->time;
			SendWeaponAnimBypass( EGON_ALTFIREON, 1 );
		}else if(getchargeReady() == 5 + 2){
			// fire!

			m_fInAttack = FIRE_WIDE;
			Attack();
		
			//CBasePlayerWeapon::ItemPostFrame();
			//return;
		}
		
	}else if(holdingPrimary ){
		if(getchargeReady() == 5 + -4 || getchargeReady() == 5 + -3){
			//may fire.

			setchargeReady(5 + 1);
			m_fInAttack = FIRE_NARROW;
			Attack();
		}else if(getchargeReady() == 5 + 1){
			m_fInAttack = FIRE_NARROW;
			Attack();
		}

	}

	//MODDD - primary fire is now narrow instead of wide, since it has no delay animation (tradeoff).
	

	//easyForcePrintLine("IM GONNA no %.2f %.2f", m_flTimeWeaponIdle, UTIL_WeaponTimeBase());


	if(getchargeReady() == 5 + 0){

		if(holdingSecondary == FALSE){
			//Forget or reverse anim?
			
			//NOTE: slightly different interpretation.
			float timePassed = gpGlobals->time - m_flStartThrow;
			
			//holdingSecondaryTarget0 = 1.533f - 0.92;
			//holdingSecondaryTarget1 = 1.533f;

			if(timePassed < holdingSecondaryTarget0){
				//jump back to idling.
				setchargeReady(5 + -4);
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + randomIdleAnimationDelay();
				//m_pPlayer->forceNoWeaponLoop = TRUE;
			}else{
				//too close to the end, skip to the switch back up.
				setchargeReady(5 + 3);
				m_flStartThrow = gpGlobals->time + holdingSecondaryTarget2;
				SendWeaponAnimBypass( EGON_ALTFIREOFF, 1 );
			}


		}else{
			//no issue.
			//... holdingSecondaryTarget1

			float timePassed = gpGlobals->time - m_flStartThrow;

			if(timePassed > holdingSecondaryTarget1 ){
				setchargeReady(5 + 2);
			}
		}

	}else if(getchargeReady() == 5 + -2){
		
		if(sendFidgetOnOff){
			SendWeaponAnimBypass( EGON_FIDGET1, 1 );
		}

		//?
	}else if(getchargeReady() == 5 + -1){
		SendWeaponAnimBypass( EGON_ALTFIREOFF, 1 );
		
	}else if(getchargeReady() == 5 + 1){

		//?
	}else if(getchargeReady() == 5 + 3){
		//... holdingSecondaryTarget2
		if(gpGlobals->time > m_flStartThrow ){
			setchargeReady(5 + -4);
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + randomIdleAnimationDelay();
		}
	}


	// PARANOIA
	if(fireExceptionStartTime != -1 && gpGlobals->time - 0.9 > fireExceptionStartTime){
		fireExceptionStartTime = -1;
		fireException = FALSE;
		m_chargeReady &= ~32;
	}



	if(m_fireState == FIRE_OFF && fireStatePrev != FIRE_OFF){
		// Fixes the issue from the beam failing to call EndAttack in clientside while paused.
		// May be called at unnecessary times but doesn't look like an issue?
		//easyForcePrintLine("GONNA GIVE IT TO YA 2: %d %d", m_fireState, fireStatePrev );
		EndAttack();
	}

	fireStatePrev = m_fireState;

	int daRecentFireMode = getchargeReady();

	// IF fireException is currently off but was on the previous frame, that also means the attack needs to end.
	//if ( (!holdingPrimary && !holdingSecondary) && 
	if( (  (!holdingPrimary && getchargeReady() == 5 + 1) || (!holdingSecondary && getchargeReady() == 5 + 2)  ) &&
		(!fireException &&
			(
				(m_fireState != FIRE_OFF) ||
				(fireExceptionPrev) ||
				m_pBeam != NULL   // || effectsExist   this changes nothing
			)   // dear god I'm tired of this shit
		)
	){
		//easyPrintLine("END O ATTACK!");
		//easyForcePrintLine("GONNA GIVE IT TO YA 3 causo1:%d causo2:%d fe:%d fs:%d fsp:%d mp:%d",
		//	(!holdingPrimary && getchargeReady() == 5 + 1),
		//	(!holdingSecondary && getchargeReady() == 5 + 2),
		//	fireException, m_fireState, fireExceptionPrev, (m_pBeam!=NULL)
		//);

		EndAttack();
		fireStatePrev = m_fireState;

		//this is a cease-fire by release.  How to act?


		switch ( m_fInAttack )
		{
		case FIRE_NARROW:
			setchargeReady(5 + -4);
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + randomIdleAnimationDelay();
		break;
		case FIRE_WIDE:
			setchargeReady(5 + 3);
			m_flStartThrow = gpGlobals->time + holdingSecondaryTarget2;
			SendWeaponAnimBypass( EGON_ALTFIREOFF, 1 );
		break;
		}
	}


	fireExceptionPrev = fireException;


	if(forceIdle){
		WeaponIdle();
	}else{
		CBasePlayerWeapon::ItemPostFrameThink();
	}
}



void CEgon::UpdateHitCloud(void){

	// NEVERMIND, already handled.
	/*
#ifndef CLIENT_DLL
	//MODDD - only restore the spot if forceHideSpotTime is inactive.
	if (m_pSprite)
	{
		// no, not up to you.
		//if (!m_pSpot){
		//	m_pSpot = CLaserSpot::CreateSpot();
		//}

		UTIL_MakeVectors( m_pPlayer->pev->v_angle );
		Vector vecSrc = m_pPlayer->GetGunPosition( );;
		Vector vecAiming = gpGlobals->v_forward;

		TraceResult tr;
		UTIL_TraceLine ( vecSrc, vecSrc + vecAiming * 8192, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr );

		UTIL_SetOrigin( m_pSprite->pev, tr.vecEndPos );


		if (UTIL_PointContents(m_pSprite->pev->origin) == CONTENTS_SKY) {
			// If we hit the sky, go invisible
			m_pSprite->pev->effects |= EF_NODRAW;
		}else{
			m_pSprite->pev->effects &= ~EF_NODRAW;
		}
	}
#endif
	*/
}


void CEgon::Fire( const Vector &vecOrigSrc, const Vector &vecDir )
{
	Vector vecDest = vecOrigSrc + vecDir * 2048;
	edict_t* pentIgnore;
	TraceResult tr;

	pentIgnore = m_pPlayer->edict();

	// ???
	Vector tmpSrc = vecOrigSrc + gpGlobals->v_up * -8 + gpGlobals->v_right * 3;

	// ALERT( at_console, "." );
	
	UTIL_TraceLine( vecOrigSrc, vecDest, dont_ignore_monsters, pentIgnore, &tr );

	recentFireDirection = (vecDest - vecOrigSrc).Normalize();
	recentHitPlaneNormal = tr.vecPlaneNormal;

	
	///////tr.vecEndPos();

	if (tr.fAllSolid)
		return;


#ifndef CLIENT_DLL

	
	CBaseEntity* pEntity;

	if(tr.pHit != NULL){
		pEntity = CBaseEntity::Instance(tr.pHit);
	}else{
		pEntity = NULL;
	}
	
	// WAIT!  Isn't ending this early from lacking something to hit weird?
	// That means just from shooting across a ridiculous distance, no ammo would be used!
	// Skipping this early skips the ammo-usage logic even though the player is still firing.
	// Check for nullity before dealing damage to 'pEntity' instead, do the other logic anyway
	//if (pEntity == NULL){
	//	return;
	//}


	//MODDD - ...  for what purpose.  (as-is, mostly.  disabled)
	/*
	if ( IsMultiplayer() )
	{
		if ( m_pSprite && pEntity->pev->takedamage )
		{
			m_pSprite->pev->effects &= ~EF_NODRAW;
		}
		else if ( m_pSprite )
		{
			//MODDD - undone, seems unnecessary.  Original left.
			m_pSprite->pev->effects |= EF_NODRAW;
			//m_pSprite->pev->effects &= ~EF_NODRAW;
		}
	}
	*/

#endif

	float timedist;

	switch ( m_fInAttack )
	{
	case FIRE_NARROW:
#ifndef CLIENT_DLL
		if ( pev->dmgtime < gpGlobals->time )
		{
			// Narrow mode only does damage to the entity it hits
			ClearMultiDamage();
			if (pEntity != NULL && pEntity->pev->takedamage)
			{
				//MODDD - added args.  Egon does not make the "bodyhit" sound. See above.
				//        ...it has since been changed how this works. The outside source no longer tells TraceAttack to play the bodyhit sound, but TraceAttack tells the caller whether
				//        to play the bodyhit and it's up to the caller to respond to that if it wants to. So by default, no bodyhit sound will play.
				pEntity->TraceAttack( m_pPlayer->pev, gSkillData.plrDmgEgonNarrow, vecDir, &tr, DMG_ENERGYBEAM, 0, TRUE );
				// AND, keep track of the thing hit that took damage
				recentlyDamagedEntity = pEntity;
				//easyForcePrintLine("HERES WHAT TOOK DAMAGE %s", pEntity->getClassname());
			}else{
				// force it off then, why remember that for unrelated frames
				recentlyDamagedEntity = NULL;
			}

			ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);


			float egonFireRateMode = EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(egonFireRateMode);
			if ( egonFireRateMode == 2 || (egonFireRateMode == 0 && IsMultiplayer()) )
			//if ( IsMultiplayer() )
			{
				// multiplayer uses 1 ammo every 1/10th second
				//MODDD - seems like a mistake, shouldn't run out faster in singleplayer, especially
				// not as fast as singleplayer's wide-fire.  How about 0.332?
				// Little high, try 0.22?
				if ( gpGlobals->time >= m_flAmmoUseTime )
				{
					UseAmmo( 1 );
					m_flAmmoUseTime = gpGlobals->time + 0.22; //0.1;
				}
			}
			else
			{
				// single player, use 3 ammo/second
				//MODDD - NOTE.  Above is an as-is comment, this is really 6 per second.
				if ( gpGlobals->time >= m_flAmmoUseTime )
				{
					UseAmmo( 1 );
					m_flAmmoUseTime = gpGlobals->time + 0.166;
				}
			}

			pev->dmgtime = gpGlobals->time + GetPulseInterval();
		}
#endif
		timedist = ( pev->dmgtime - gpGlobals->time ) / GetPulseInterval();
		break;
	
	case FIRE_WIDE:
#ifndef CLIENT_DLL
		if ( pev->dmgtime < gpGlobals->time )
		{
			// wide mode does damage to the ent, and radius damage
			ClearMultiDamage();
			if (pEntity != NULL && pEntity->pev->takedamage)
			{
				//MODDD - added args.  Egon does not make the "bodyhit" sound. See above.
				pEntity->TraceAttack( m_pPlayer->pev, gSkillData.plrDmgEgonWide, vecDir, &tr, DMG_ENERGYBEAM | DMG_ALWAYSGIB, 0, TRUE);
				recentlyDamagedEntity = pEntity;
			}else{
				recentlyDamagedEntity = NULL;
			}

			ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

			float egonRadiusDamageMode = EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(egonRadiusDamageMode);
			if (egonRadiusDamageMode == 2 || (egonRadiusDamageMode == 0 && IsMultiplayer())) {
				// multiplayer mode (yes)
				// radius damage a little more potent in multiplayer.
				//MODDD - as-is comment.  I think you mean, 'radius damage is a little more EXISTENT in multiplayer'.  Yeah...?  What else is there?
				::RadiusDamage( tr.vecEndPos, pev, m_pPlayer->pev, gSkillData.plrDmgEgonWide/4, 128, CLASS_NONE, DMG_ENERGYBEAM | DMG_BLAST | DMG_ALWAYSGIB );
			}
			else{
				// singleplayer mode?  nothing here.
			}

			if ( !m_pPlayer->IsAlive() )
				return;



			float egonFireRateMode = EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(egonFireRateMode);
			if ( egonFireRateMode == 2 || (egonFireRateMode == 0 && IsMultiplayer()) )
			{
				//multiplayer uses 5 ammo/second
				if ( gpGlobals->time >= m_flAmmoUseTime )
				{
					UseAmmo( 1 );
					m_flAmmoUseTime = gpGlobals->time + 0.2;
				}
			}
			else
			{
				// Wide mode uses 10 charges per second in single player
				if ( gpGlobals->time >= m_flAmmoUseTime )
				{
					UseAmmo( 1 );
					m_flAmmoUseTime = gpGlobals->time + 0.1;
				}
			}

			pev->dmgtime = gpGlobals->time + GetDischargeInterval();
			if ( m_shakeTime < gpGlobals->time )
			{
				//MODDD - shake behavior altered.  Shake a little less violently but more often, more consistent.
				//UTIL_ScreenShake( tr.vecEndPos, 5.0, 150.0, 0.75, 250.0 );
				//m_shakeTime = gpGlobals->time + 1.5;
				UTIL_ScreenShake( tr.vecEndPos, 5.2, 160.0, 0.6, 380.0 );
				m_shakeTime = gpGlobals->time + 0.28;
			}
		}
#endif
		timedist = ( pev->dmgtime - gpGlobals->time ) / GetDischargeInterval();
		break;
	}




#ifndef CLIENT_DLL
	if(pEntity){
		// If the recentlyDamagedEntity (if one was this frame) meets some conditions
		// (not the world, takes damage), mark this as an exception to check for a decal sooner
		BOOL hitEntityTookDamage = FALSE;
		BOOL canCheckForScorch = FALSE;

		if(pEntity->IsWorld() || pEntity->pev->takedamage == DAMAGE_NO){
			// Is world or doesn't take damage?
			// always allow checks so long as the delay is met
			canCheckForScorch = (gpGlobals->time >= nextScorchInterval);
		}else{
			// Not the world and takes damage?
			// ok if we're firing at the most recently damaged entity, likely set in this same frame but just in case,
			// another check for that.
			// (bypasses the nextScorchInterval check too if so)
			// (tr.pHit is the same as pEntity->edict(), pEntity came from tr.pHit)
			canCheckForScorch = (recentlyDamagedEntity != NULL && tr.pHit == recentlyDamagedEntity->edict());
		}


		// do we need to keep the check from being done more often from focusing fire on a damagable BSP-ish entity
		// that takes decals?...  sounds really really specific and most won't last long anyway,  nah.

		//MODDD - TODO.  Should this be done in ev_hldm? unsure.
		// liiiiitle much logic here now for that to make sense anymore and the one firing will see the
		// hitcloud in front of whatever decal anyway, nevermind.
		if(canCheckForScorch){
			BOOL canDrawScorch = FALSE;
			float decalDisto;
			if(m_fInAttack == FIRE_NARROW){
				nextScorchInterval = gpGlobals->time + EGON_PRIMARY_SCORCH_INTERVAL;
				decalDisto = 10.00;
			}else{
				nextScorchInterval = gpGlobals->time + EGON_SECONDARY_SCORCH_INTERVAL;
				//decalDisto = 37.5;
				decalDisto = 16.5;
			}

			// But, can the scorch actually be drawn?  Don't draw too close to the previously
			// made one (keep track of where the previous location a draw-call was made, the most
			// we can really do here)
			if(tr.flFraction < 1.0 && UTIL_PointContents(tr.vecEndPos) != CONTENTS_SKY ){
				// hit something?
				if(!previousScorchLocSet){
					// good to go
					canDrawScorch = TRUE;
				}else{
					// Check: Is this far enough away from the previously drawn scorch mark?
					float theDisto = Distance(tr.vecEndPos, previousScorchLoc);
					if(theDisto >= decalDisto){
						// proceed, enough distance from the previous draw location
						canDrawScorch = TRUE;
					}else{
						//easyForcePrintLine("FAIL 1");
					}
				}
			}

			if(canDrawScorch){
				//easyForcePrintLine("OK");
				// remember where this was drawn for next time
				previousScorchLocSet = TRUE;
				previousScorchLoc = tr.vecEndPos;
				if(m_fInAttack == FIRE_NARROW){
					// CHANGE: use small scorch decals 1 & 2 instead (same size).
					//UTIL_DecalTrace(&tr, DECAL_SMALLSCORCH1 + RANDOM_LONG(0, 2) );
					UTIL_DecalTrace(&tr, DECAL_SMALLSCORCH1 + RANDOM_LONG(0, 1) );
				}else{  // WIDE
					// CHANGE: use small scorch decal 3 (little larger than the first two).
					//UTIL_DecalTrace(&tr, DECAL_SCORCH1 + RANDOM_LONG(0, 1) );
					UTIL_DecalTrace(&tr, DECAL_SMALLSCORCH1 + 2 );
				}

				// forget this, no more scorch marks in the same frame since it doesn't come from doing damage without
				// being freshly updated.
				recentlyDamagedEntity = NULL;
			}else{
				//easyForcePrintLine("THA hay");
			}

		}//canCheckForScorch check
	}//pEntity check
#endif




	if ( timedist < 0 )
		timedist = 0;
	else if ( timedist > 1 )
		timedist = 1;
	timedist = 1-timedist;

	UpdateEffect( tmpSrc, tr.vecEndPos, timedist );	
}





// WARNING!  Most of what's seen here is only for players other than the local player to look at (from multiplayer).
// The hitcloud is an exception, it is only in here.  Unknown if it would be a good idea to have a ev_hldm equivalent
// and then make the hitcloud here skip the local player.
// They won't show up even in third person, still uses the same as if in first person.   Weird, but, ok.
// See the egonfire in cl_dlls/ev_hldm.cpp.  That shows up for the local plaer only instead.
void CEgon::UpdateEffect( const Vector &startPoint, const Vector &endPoint, float timeBlend )
{
	
#ifndef CLIENT_DLL
	float egonEffectsModeVar = EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(egonEffectsMode);
	float egonHitCloudVar = EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(egonHitCloud);


	//if ( !m_pBeam )
	if(!effectsExist)
	{
		//easyForcePrintLine("DO THE SHIT");
		CreateEffect();
	}

	if(m_pBeam){
		m_pBeam->SetStartPos( endPoint );
		m_pBeam->SetBrightness( 255 - (timeBlend*180) );
		m_pBeam->SetWidth( 40 - (timeBlend*20) );


		if(egonEffectsModeVar == 3){
			//pre-release colors. this is blue always (and only exists for secondary fire).
			m_pBeam->SetColor( 60 + (25*timeBlend), 120 + (30*timeBlend), 64 + 80*fabs(sin(gpGlobals->time*10)) );
		}else{
			//beam colors work as usual.
			if ( m_fInAttack == FIRE_WIDE )
				//purple?
				m_pBeam->SetColor( 30 + (25*timeBlend), 30 + (30*timeBlend), 64 + 80*fabs(sin(gpGlobals->time*10)) );
			else
				//blue?
				m_pBeam->SetColor( 60 + (25*timeBlend), 120 + (30*timeBlend), 64 + 80*fabs(sin(gpGlobals->time*10)) );

		}
	}

	if(m_pNoise){
		if(egonEffectsModeVar == 3){
			//pre-release colors. Only present for wide fire. and blue.?
			//To make less transparent multiply each by 1.7. including the multiples of variables. OR just the whole thing in parenthesis each argument.
			//m_pNoise->SetColor( 60 + (25*timeBlend), 120 + (30*timeBlend), 64 + 80*fabs(sin(gpGlobals->time*10)) );
			//...no, don't make it blue-er? I think?
			
			//m_pNoise->SetColor( 30 + (25*timeBlend), 30 + (30*timeBlend), 64 + 80*fabs(sin(gpGlobals->time*10)) );

			//...just keep it constant to what it started as. huh.
		
		}else{
			//normal... maybe.
			m_pNoise->SetColor( 30 + (25*timeBlend), 30 + (30*timeBlend), 64 + 80*fabs(sin(gpGlobals->time*10)) );
		}
	
		m_pNoise->SetStartPos( endPoint );
	
	}


	/*
	
		if ( m_fInAttack == FIRE_WIDE )
		{
			m_pNoise->SetColor( 50, 50, 255 );
			m_pNoise->SetNoise( 8 );
		}
		else
		{
			m_pNoise->SetColor( 80, 120, 255 );
			m_pNoise->SetNoise( 2 );
		}

	*/



	if(egonEffectsModeVar == 4){
		//CHRISTMAS MODE!  Makes what beam is what for egonEffectsMode #2 easier to spot.
		if(m_pBeam){
			m_pBeam->SetColor( 255,0 ,0 );
		}
		if(m_pNoise){
			m_pNoise->SetColor( 0, 255, 0 );
		}
	}


	if(m_pSprite){
		UTIL_SetOrigin( m_pSprite->pev, endPoint );


		m_pSprite->pev->frame += m_pSprite->pev->framerate * gpGlobals->frametime;

		//MODDD - NOTE. Assuming that being on the exact last frame is acceptable?  Was this way as-is.
		if ( m_pSprite->pev->frame > m_pSprite->Frames() ){
			//MODDD - proper loop around.
			//m_pSprite->pev->frame = 0;
			m_pSprite->pev->frame -= m_pSprite->Frames();
		}
		

		//MODDD - NEW
		if (UTIL_PointContents(m_pSprite->pev->origin) == CONTENTS_SKY) {
			// If we hit the sky, go invisible
			m_pSprite->pev->effects |= EF_NODRAW;
		}else{
			m_pSprite->pev->effects &= ~EF_NODRAW;
		}
	}


#endif
	
}

void CEgon::CreateEffect( void )
{
	//is "CBaseMonster::Instance" even legal here?  not sure  if the attachment qualifies as a "entity".

	//also, ENT?
	//CBaseEntity* attachment = CBaseMonster::Instance(  (INDEXENT(m_pPlayer->entindex() + 0x1000)));


	//edict_t* attachment = (  (INDEXENT(entindex()  )));

	//easyPrintLine("WHAT THE att %d", (attachment == NULL) );


#ifndef CLIENT_DLL

	//INDEXENT???
	
	float egonEffectsModeVar = EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(egonEffectsMode);
	float egonHitCloudVar = EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(egonHitCloud);

	
	//ripped from rpg right before creating the rocket at the tip of the weapon.
	Vector vecSrc = pev->origin; //m_pPlayer->GetGunPosition( ) + -gpGlobals->v_forward * 100 + gpGlobals->v_right * 8 + gpGlobals->v_up * -8;
		
	DestroyEffect();

	effectsExist = TRUE;


	//NOT if 3!
	if(egonEffectsModeVar >= 1 && egonEffectsModeVar != 3){

		m_pBeam = CBeam::BeamCreate( EGON_BEAM_SPRITE, 40 );
		m_pBeam->PointEntInit( vecSrc, m_pPlayer->entindex()  );
		//MODDD - commented out.  Causes the beam not to sufficiently shrink when nearing a surface you're firing at, causing the effect to clip backwards into the player & gun.
		//m_pBeam->SetFlags( BEAM_FSINE );
		m_pBeam->SetEndAttachment( 1 );
		m_pBeam->pev->spawnflags |= SUB_SF_BEAM_TEMPORARY;	// Flag these to be destroyed on save/restore or level transition
		//if(testVar == 0 || testVar == 2 ){
		//	m_pBeam->pev->flags |= FL_SKIPLOCALHOST;
		////...does this need to stay commented out?
		//}
		m_pBeam->pev->owner = m_pPlayer->edict();


		if ( m_fInAttack == FIRE_WIDE ){
			m_pBeam->SetScrollRate( 50 );
			m_pBeam->SetNoise( 20 );
		}else{
			m_pBeam->SetScrollRate( 110 );
			m_pBeam->SetNoise( 5 );
		}
		
	}

	if(egonEffectsModeVar >= 2 && egonEffectsModeVar != 3){

		m_pNoise = CBeam::BeamCreate( EGON_BEAM_SPRITE, 55 );
		//new?
		//m_pNoise->SetFlags( BEAM_FSINE );

		m_pNoise->PointEntInit( vecSrc, m_pPlayer->entindex() );
		m_pNoise->SetScrollRate( 25 );
		m_pNoise->SetBrightness( 100 );
		m_pNoise->SetEndAttachment( 1 );
		m_pNoise->pev->spawnflags |= SUB_SF_BEAM_TEMPORARY;
		//if(testVar == 0 || testVar == 1){
		//	m_pNoise->pev->flags |= FL_SKIPLOCALHOST;
		//}
		m_pNoise->pev->owner = m_pPlayer->edict();


		if ( m_fInAttack == FIRE_WIDE )
		{
			m_pNoise->SetColor( 50, 50, 255 );
			m_pNoise->SetNoise( 8 );
		}
		else
		{
			m_pNoise->SetColor( 80, 120, 255 );
			m_pNoise->SetNoise( 2 );
		}

		//m_pNoise->Point
	}



	

	//choice 3, pre-release, has special rules.
	//for primary fire, the two clientside lasers only: spiral and the thicker straight one.
	//For secondary fire, the clientside thicker straight and m_pBeam and m_pNoise (but of narrow fire).
	if(egonEffectsModeVar == 3){

		
		//noise beam gets some different behavior.

		//For wide only.
		
		if ( m_fInAttack == FIRE_WIDE ){
			m_pBeam = CBeam::BeamCreate( EGON_BEAM_SPRITE, 60 );
			m_pBeam->PointEntInit( vecSrc, m_pPlayer->entindex()  );
			//MODDD - commented out.  Causes the beam not to sufficiently shrink when nearing a surface you're firing at, causing the effect to clip backwards into the player & gun.
			// UHHHH.  That's because this should only be shown to other players looking at the current one or in MP dingus!  AIRJGARWIGJA
			m_pBeam->SetFlags( BEAM_FSINE );
			m_pBeam->SetEndAttachment( 1 );
			m_pBeam->pev->spawnflags |= SUB_SF_BEAM_TEMPORARY;	// Flag these to be destroyed on save/restore or level transition
			//if(testVar == 0 || testVar == 2 ){
				m_pBeam->pev->flags |= FL_SKIPLOCALHOST;
			//}
			m_pBeam->pev->owner = m_pPlayer->edict();

			m_pBeam->SetBrightness(255);
			m_pBeam->SetColor( (int)(0.2*255), (int)(0.6*255), (int)(0.8*255) );


			//if ( m_fInAttack == FIRE_WIDE )
			//{
			//	m_pBeam->SetScrollRate( 50 );
			//	m_pBeam->SetNoise( 20 );
			//}
			//else
			//{
			//// want narrow mode's features.
				m_pBeam->SetScrollRate( 140 );
				m_pBeam->SetNoise( 1 );
			//}




			m_pNoise = CBeam::BeamCreate( EGON_BEAM_SPRITE, 40 );
			//new?   yea... noise never had this
			//m_pNoise->SetFlags( BEAM_FSINE );

			m_pNoise->PointEntInit( vecSrc, m_pPlayer->entindex() );
			m_pNoise->SetScrollRate( 25 );
			m_pNoise->SetBrightness( 90 );
			m_pNoise->SetEndAttachment( 1 );
			m_pNoise->pev->spawnflags |= SUB_SF_BEAM_TEMPORARY;
			//if(testVar == 0 || testVar == 1){
				m_pNoise->pev->flags |= FL_SKIPLOCALHOST;
			//}
			m_pNoise->pev->owner = m_pPlayer->edict();


			//if ( m_fInAttack == FIRE_WIDE )
			//{
			//	m_pNoise->SetColor( 50, 50, 255 );
			//	m_pNoise->SetNoise( 8 );
			//}
			//else
			//{
			//// want narrow's features.
				m_pNoise->SetColor( 80, 160, 255 );
				m_pNoise->SetNoise( 1 );
			//}

		}else{
			// NARROW?  Just use retail for now
			m_pBeam = CBeam::BeamCreate( EGON_BEAM_SPRITE, 40 );
			m_pBeam->PointEntInit( pev->origin, m_pPlayer->entindex() );
			m_pBeam->SetFlags( BEAM_FSINE );
			m_pBeam->SetEndAttachment( 1 );
			m_pBeam->pev->spawnflags |= SUB_SF_BEAM_TEMPORARY;	// Flag these to be destroyed on save/restore or level transition
			m_pBeam->pev->flags |= FL_SKIPLOCALHOST;
			m_pBeam->pev->owner = m_pPlayer->edict();

			m_pNoise = CBeam::BeamCreate( EGON_BEAM_SPRITE, 55 );
			m_pNoise->PointEntInit( pev->origin, m_pPlayer->entindex() );
			m_pNoise->SetScrollRate( 25 );
			m_pNoise->SetBrightness( 100 );
			m_pNoise->SetEndAttachment( 1 );
			m_pNoise->pev->spawnflags |= SUB_SF_BEAM_TEMPORARY;
			m_pNoise->pev->flags |= FL_SKIPLOCALHOST;
			m_pNoise->pev->owner = m_pPlayer->edict();

			/*
			m_pSprite = CSprite::SpriteCreate( EGON_FLARE_SPRITE, pev->origin, FALSE );
			m_pSprite->pev->scale = 1.0;
			m_pSprite->SetTransparency( kRenderGlow, 255, 255, 255, 255, kRenderFxNoDissipation );
			m_pSprite->pev->spawnflags |= SF_SPRITE_TEMPORARY;
			m_pSprite->pev->flags |= FL_SKIPLOCALHOST;
			m_pSprite->pev->owner = m_pPlayer->edict();
			*/


			/*
			if ( m_fireMode == FIRE_WIDE )
			{
				m_pBeam->SetScrollRate( 50 );
				m_pBeam->SetNoise( 20 );
				m_pNoise->SetColor( 50, 50, 255 );
				m_pNoise->SetNoise( 8 );
			}
			else
			*/
			{
				m_pBeam->SetScrollRate( 110 );
				m_pBeam->SetNoise( 5 );
				m_pNoise->SetColor( 80, 120, 255 );
				m_pNoise->SetNoise( 2 );
			}

		}//END OF beam type check
		
		//m_pNoise->Point
	}


	if(egonHitCloudVar == 1){
		//MODDD - take advantage of the ability to animate.
		// ...nevermind, not as reliable as animating from direct calls from here.  Weird.  Still specifying framerate now.
		//m_pSprite = CSprite::SpriteCreate( EGON_FLARE_SPRITE, pev->origin, FALSE );
		// framerate changed (was 8)
		m_pSprite = CSprite::SpriteCreate( EGON_FLARE_SPRITE, pev->origin, FALSE, 10.5);

		// TEST!!!  Copied from RPG lasersight spawn.
		// Should improve tracking, no need for continuous moving around.
		// And yes, the only reason the hitcloud still shows up on some skyboxes is from the size of the sprite.
		// The lasersight sprite, used instead, also doesn't show up on those skyboxes just like the RPGs.
		//         IIIIIIIIiiiiiiiiiiiiiii     love this engine
		//////////////////////////////////////////////////////////////////
		m_pSprite->pev->movetype = MOVETYPE_NONE;

		//SET_MODEL(ENT(m_pSprite->pev), "sprites/laserdot.spr");
		//UTIL_SetOrigin( m_pSprite->pev, pev->origin );
		//////////////////////////////////////////////////////////////////

		//MODDD - hit cloud size will varry on wide or narrow fire.
		//m_pSprite->pev->scale = 1.0;

		// was 2.8 and 4.4
		if ( m_fInAttack == FIRE_NARROW ){
			m_pSprite->pev->scale = 1.4;
		}else{
			m_pSprite->pev->scale = 2.25;
		}
		
		// kRenderGlow makes it avoid the black background and not go through nearby geometry.
		// kRenderFxNoDissipation makes it pay attention to scale and not fade with distance.
		m_pSprite->SetTransparency( kRenderGlow, 255, 255, 255, 255, kRenderFxNoDissipation );
		m_pSprite->pev->spawnflags |= SF_SPRITE_TEMPORARY;
		//m_pSprite->pev->flags |= FL_SKIPLOCALHOST;
		m_pSprite->pev->owner = m_pPlayer->edict();

	}//egonHitCloudVar check

	

	//MODDD - note.  Colors may not matter here if they are updated constantly by "updateEffect".
	
	/*
	m_pNoise->SetColor( 50, 50, 255 );
	m_pNoise->SetColor( 80, 120, 255 );
	m_pBeam->SetColor( 30 + (25*timeBlend), 30 + (30*timeBlend), 64 + 80*fabs(sin(gpGlobals->time*10)) );
	*/

	//as seen in "updateEffect" for the main beam.  
	/*
	if ( m_fInAttack == FIRE_WIDE )
		m_pBeam->SetColor( 30 + (25*timeBlend), 30 + (30*timeBlend), 64 + 80*fabs(sin(gpGlobals->time*10)) );
	else
		m_pBeam->SetColor( 60 + (25*timeBlend), 120 + (30*timeBlend), 64 + 80*fabs(sin(gpGlobals->time*10)) );
	*/


#endif
	
}


void CEgon::DestroyEffect( void )
{

#ifndef CLIENT_DLL

	effectsExist = FALSE;

	if ( m_pBeam )
	{
		UTIL_Remove( m_pBeam );
		m_pBeam = NULL;
	}
	if ( m_pNoise )
	{
		UTIL_Remove( m_pNoise );
		m_pNoise = NULL;
	}
	if ( m_pSprite )
	{
		if ( m_fInAttack == FIRE_WIDE ){
			//MODDD - move away from the surface a bit to avoid clipping through
			// the nearby object as it expands
			
			// NO NEED FOR THIS NUDGE!  The "m_pSprite->Expand(...)" call was resetting the renderfx,
			// want glow but it forced it to add.  New variant created that keeps it as it is instead.
			// Now, it won't try
			//m_pSprite->pev->origin =
			//	m_pSprite->pev->origin +
			//	recentFireDirection * -50 +
			//	recentHitPlaneNormal * 12;
			
			//MODDD - see multiples
			// was 0.45, 0.28.  and 0.88, 1.42 for fade speed
			m_pSprite->ExpandAnimatePreserveEffects( 10 * 0.70, 500 * 0.75 );

		}else{
			//MODDD - now, even narrow fire can use the expand-out effect to remove its hitcloud.
			// However it happens more quickly.

			//m_pSprite->pev->origin =
			//	m_pSprite->pev->origin +
			//	recentFireDirection * -35 +
			//	recentHitPlaneNormal * 8;

			//MODDD - reduced further.
			m_pSprite->ExpandAnimatePreserveEffects( 10 * 0.45, 500 * 1.15 );


			//UTIL_Remove( m_pSprite );
		}
		m_pSprite = NULL;
	}
#endif

}



void CEgon::WeaponIdle( void )
{
	if(gpGlobals->time < ignoreIdleTime){
		return;  //not yet
	}

	ResetEmptySound( );


	//used to be this?  odd how it isn't using "UTIL_WeaponTimeBase()" like the other references to time here?
	//if ( m_flTimeWeaponIdle > gpGlobals->time )
	//	return;


	if(getchargeReady() <= 5 + -1){
		//only allow influence from here for idle animations.

	}else{
		return;
	}
	
	
	if(getchargeReady() == 5 + -2){
		// NO, the point is to release, so this is OK.
		setchargeReady(5 + -4);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + randomIdleAnimationDelay();
		//return;
		//TODO: give a random delay here!!!
	}


	if(getchargeReady() == 5 + -4){
		// force anim
		// ...why FIDGET1?  IDLE works fine.
		// Otherwise holding down a press while unable to fire (like primary, then instantly pressing primary again before the delay is up)
		// will let the figet play a bit and it looks odd.
		// This will still play a little if the rapid-press-again is done, but it's not as jarring.  No static frame sequence that would be best.
		// This has to be done at all because, otherwise, the sequence from the most recent attack will continue looping while not firing.
		// ALSO.  Once the idle-delay expires once since firing, the firestate changes from -4 to -3, which allows normal idle anims to be picked
		// now that this one forced static no longer needs to be enforced.
		SendWeaponAnimBypass( EGON_IDLE1, 1 );
	}
	
	if(getchargeReady() == 5 + -1){
		setchargeReady(5 + 3);
		m_flStartThrow = gpGlobals->time + holdingSecondaryTarget2;
		SendWeaponAnimBypass( EGON_ALTFIREOFF, 1 );
		return;
	}

	
	//easyPrintLine("????????............. %.2f %.2f %.2f", m_flTimeWeaponIdle, UTIL_WeaponTimeBase(), gpGlobals->time);

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	
	if(getchargeReady() == 5 + -4){
		//End.
		setchargeReady(5 + -3);
		return;
	}



	//easyPrintLine("EGON IDLE CALLED %.2f", gpGlobals->time);

	//m_flStartThrow = 0;

	



	/*
	//MOVE + MOD!!!
	if ( m_fireState != FIRE_OFF ){
		//easyPrintLine("END O ATTACK!");
		EndAttack();

	}
	*/


	int iAnim;

	//debug sound.
	//EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/357_cock1.wav", 0.8, ATTN_NORM);

	//MODDD
	if(lastSentAnim != EGON_FIDGET1){
		float flRand = RANDOM_FLOAT(0,1);

		if ( flRand <= 0.5 )
		{
			iAnim = EGON_IDLE1;
			//m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
			//m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (59.8 / 30.0) + randomIdleAnimationDelay();
		}
		else 
		{
			iAnim = EGON_FIDGET1;
			//m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3;
			//m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 4.3;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (60.0 / 30.0) + randomIdleAnimationDelay();
		
		}
	}else{
		iAnim = EGON_IDLE1;
		//m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (59.8 / 30.0) + randomIdleAnimationDelay();
		//don't fidget twice in a row.
	}

	
	

	
	forceBlockLooping();

	//SendWeaponAnim( -1, 1, m_fireState );
	/// ..... what.  what was that.


	lastSentAnim = iAnim;
	SendWeaponAnimBypass( iAnim, 1 );
	//m_deployed = TRUE;
	//???
}



void CEgon::EndAttack( void )
{
	BOOL bMakeNoise = FALSE;

	// in case turned on from going through a transition
	blockContinualFireSound = FALSE;

	//easyForcePrintLine("GONNA GIVE IT TO YA");
		
	if ( m_fireState != FIRE_OFF ) //Checking the button just in case!.
		 bMakeNoise = TRUE;

	//usedOneAmmo = FALSE;
	m_chargeReady &= ~32;
	fireExceptionStartTime = -1;

	//MODDD - 
	//if(m_fireState == FIRE_OFF){
	//	return; //already off?
	//}

	lockedFireState = FALSE;

	PLAYBACK_EVENT_FULL( FEV_GLOBAL | FEV_RELIABLE, m_pPlayer->edict(), m_usEgonStop, 0, (float *)&m_pPlayer->pev->origin, (float *)&m_pPlayer->pev->angles, 0.0, 0.0, bMakeNoise, 0, 0, 0 );

	//MODDD - this part should use "UTIL_WeaponBase()", even if it is 0...
	//m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0;

	//easyPrintLine("TEH ee %.2f", UTIL_WeaponTimeBase());

	//m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5;
	//m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5;
	//m_flNextSecondaryAttack = gpGlobals->time + 0.5;
	


	//MODDD - uh.  why'd we remove that again?
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0;
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5;

	m_fireState = FIRE_OFF;

	DestroyEffect();

}



class CEgonAmmo : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/w_chainammo.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/w_chainammo.mdl");
		precacheAmmoPickupSound();
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		if (pOther->GiveAmmo( AMMO_URANIUMBOX_GIVE, "uranium", URANIUM_MAX_CARRY ) != -1)
		{
			//MODDD - filtered.
			playAmmoPickupSound();

			//MODDD
			//UPDATE: apparently, the Gauss's ammo is the universal, and the egon's ammo is just the
			//mp5 chain ammo (seems really unfitting, I think it was a placeholder).
			//Still, leaving the expected sound.
			if(pOther->IsPlayer()){
				CBasePlayer* pPlayer = (CBasePlayer*)pOther;
				pPlayer->SetSuitUpdate("!HEV_EGONPOWER", FALSE, SUIT_NEXT_IN_20MIN);
			}


			return TRUE;
		}
		return FALSE;
	}
};
LINK_ENTITY_TO_CLASS( ammo_egonclip, CEgonAmmo );
