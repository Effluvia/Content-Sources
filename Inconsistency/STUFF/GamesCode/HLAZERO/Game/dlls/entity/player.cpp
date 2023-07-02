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

===== player.cpp ========================================================

  functions dealing with the player

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "trains.h"
#include "nodes.h"
#include "weapons.h"
#include "soundent.h"
#include "basemonster.h"
#include "shake.h"
#include "decals.h"
#include "gamerules.h"
#include "game.h"
#include "hltv.h"
#include "gib.h"
#include "player_extra.h"
#include "util_debugdraw.h"
#include "satchel.h"
#include "talkmonster.h"
// only included to see what some default AI schedules are such as "slSmallFlinsh" for another
// monster.
#include "defaultai.h"

#include "cvar_custom_info.h"
#include "cvar_custom_list.h"


EASY_CVAR_EXTERN_MASS


EASY_CVAR_EXTERN_DEBUGONLY(myStrobe)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(raveEffectSpawnInterval)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST(sv_germancensorship)
EASY_CVAR_EXTERN_DEBUGONLY(mutePlayerPainSounds)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(geigerChannel)
EASY_CVAR_EXTERN_DEBUGONLY(drawDebugBloodTrace)
EASY_CVAR_EXTERN_DEBUGONLY(autoSneaky)
EASY_CVAR_EXTERN(sv_longjump_chargemode)
EASY_CVAR_EXTERN_DEBUGONLY(endlessFlashlightBattery)
EASY_CVAR_EXTERN_DEBUGONLY(normalSpeedMulti)
EASY_CVAR_EXTERN_DEBUGONLY(noclipSpeedMulti)
EASY_CVAR_EXTERN_DEBUGONLY(jumpForceMulti)
EASY_CVAR_EXTERN_DEBUGONLY(timedDamageEndlessOnHard)
EASY_CVAR_EXTERN_DEBUGONLY(drawNodeAll)
EASY_CVAR_EXTERN_DEBUGONLY(drawNodeSpecial)
EASY_CVAR_EXTERN_DEBUGONLY(drawNodeConnections)
EASY_CVAR_EXTERN_DEBUGONLY(drawNodeAlternateTime)
EASY_CVAR_EXTERN_DEBUGONLY(nodeSearchStartVerticalOffset)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(timedDamageDeathRemoveMode)
extern float globalPSEUDO_cameraMode;
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(mirrorsDoNotReflectPlayer)
EASY_CVAR_EXTERN_DEBUGONLY(barnacleCanGib)
EASY_CVAR_EXTERN_DEBUGONLY(canDropInSinglePlayer)
EASY_CVAR_EXTERN_DEBUGONLY(timedDamageIgnoresArmor)
EASY_CVAR_EXTERN_DEBUGONLY(itemBatteryPrerequisite)
EASY_CVAR_EXTERN_DEBUGONLY(timedDamageDisableViewPunch)
EASY_CVAR_EXTERN_DEBUGONLY(batteryDrainsAtDeath)
EASY_CVAR_EXTERN_DEBUGONLY(batteryDrainsAtAdrenalineMode)
EASY_CVAR_EXTERN_DEBUGONLY(printOutCommonTimables)
EASY_CVAR_EXTERN_DEBUGONLY(playerBrightLight)
EASY_CVAR_EXTERN_DEBUGONLY(disablePainPunchAutomatic)
EASY_CVAR_EXTERN_DEBUGONLY(timedDamageReviveRemoveMode)
EASY_CVAR_EXTERN_DEBUGONLY(playerExtraPainSoundsMode)

//MASS!  Try to remove those that are not used in here..
///////////////////////////////////////////////////////////////////////////////////
EASY_CVAR_EXTERN_DEBUGONLY(weaponPickupPlaysAnyReloadSounds)
EASY_CVAR_EXTERN_DEBUGONLY(playerReviveInvincibilityTime)
EASY_CVAR_EXTERN_DEBUGONLY(playerReviveBuddhaMode)
EASY_CVAR_EXTERN_DEBUGONLY(playerReviveTimeBlocksTimedDamage)
EASY_CVAR_EXTERN_DEBUGONLY(drawDebugCine)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST(wpn_glocksilencer)
EASY_CVAR_EXTERN_DEBUGONLY(nothingHurts)
EASY_CVAR_EXTERN_DEBUGONLY(RadiusDamageDrawDebug)
EASY_CVAR_EXTERN_DEBUGONLY(customLogoSprayMode)
EASY_CVAR_EXTERN_DEBUGONLY(ladderCycleMulti)
EASY_CVAR_EXTERN_DEBUGONLY(ladderSpeedMulti)
EASY_CVAR_EXTERN_DEBUGONLY(friendlyPianoFollowVolume)
EASY_CVAR_EXTERN_DEBUGONLY(playerUseDrawDebug)
EASY_CVAR_EXTERN_DEBUGONLY(playerFadeOutRate)
//EASY_CVAR_EXTERN(cl_holster)
//EASY_CVAR_EXTERN(cl_ladder)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hideDamage)
EASY_CVAR_EXTERN_DEBUGONLY(minimumRespawnDelay)
EASY_CVAR_EXTERN_DEBUGONLY(monsterToPlayerHitgroupSpecial)
EASY_CVAR_EXTERN(precacheAll)
EASY_CVAR_EXTERN(blastExtraArmorDamageMode)
EASY_CVAR_EXTERN(sv_player_midair_fix)
EASY_CVAR_EXTERN(sv_player_midair_accel)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(viewModelPrintouts)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteclip)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteammo)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelay)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelaycustom)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_nogaussrecoil)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(gaussRecoilSendsUpInSP)
EASY_CVAR_EXTERN(playerDeadTruce)
EASY_CVAR_EXTERN(playerDeadTalkerBehavior)

extern int g_TalkMonster_PlayerDead_DialogueMod;


#define TRAIN_NEW		0xc0
#define TRAIN_OFF		0x00
#define TRAIN_NEUTRAL	0x01
#define TRAIN_SLOW		0x02
#define TRAIN_MEDIUM	0x03
#define TRAIN_FAST		0x04 
#define TRAIN_BACK		0x05

#define TRAIN_ACTIVE	0x80 

#define FLASH_DRAIN_TIME	 1.2 //100 units/3 minutes
#define FLASH_CHARGE_TIME	 0.2 // 100 units/20 seconds  (seconds per unit)



// Unused constants from the as-is codebase?  Whoops!
// Looks unrelated to the ladder, maybe climbing up something more complex than a ladder was planned at some point.
// Not to be confused with the MAX_CLIMB_SPEED (now _ALPHA and _RETAIL) constants in const.h.
/*
#define CLIMB_SHAKE_FREQUENCY	22	// how many frames in between screen shakes when climbing

#define CLIMB_SPEED_DEC			15	// climbing deceleration rate
#define CLIMB_PUNCH_X			-7  // how far to 'punch' client X axis when climbing
#define CLIMB_PUNCH_Z			7	// how far to 'punch' client Z axis when climbing
*/


#define ARMOR_RATIO 0.2	// Armor Takes 80% of the damage
#define ARMOR_BONUS 0.5	// Each Point of Armor is work 1/x points of health

//MODDD NOTE - if there were a need for this to be drawn on the HUD, it would be
// best to move this to util_shared.h.
#define PLAYER_AIRTIME 12		// lung full of air lasts this many seconds

//#define PLAYER_USE_SEARCH_RADIUS	(float)64
#define PLAYER_USE_SEARCH_RADIUS 72

#define GEIGERDELAY 0.25



//extern cvar_t* cvar_sv_cheats;
//MODDD
extern unsigned short g_sFreakyLight;
// nope, for the player serverside
//extern int g_framesSinceRestore;


///////////////////////////////////////////////////////////////////////////////////

// #define DUCKFIX

extern DLL_GLOBAL ULONG g_ulModelIndexPlayer;
extern DLL_GLOBAL BOOL g_fGameOver;
extern DLL_GLOBAL int g_iSkillLevel;
extern DLL_GLOBAL int gDisplayTitle;

extern void CopyToBodyQue(entvars_t* pev);
extern edict_t *EntSelectSpawnPoint( CBaseEntity *pPlayer );

// the world node graph
extern CGraph WorldGraph;
extern DLL_GLOBAL ULONG g_ulFrameCount;
extern BOOL g_firstPlayerEntered;
extern float g_flWeaponCheat;
extern BOOL g_f_playerDeadTruce;



int gEvilImpulse101;
//MODDD - and just what was this doing above EntSelectSpawnPoint halfway down the file?
// Also, FNullEnt moved to util_entity.h.  Because why would only the player want this.
DLL_GLOBAL CBaseEntity* g_pLastSpawn;
BOOL gInitHUD = TRUE;
BOOL g_giveWithoutTargetLocation = FALSE;




LINK_ENTITY_TO_CLASS( player, CBasePlayer );


//MODDD - some notes from throughout the file from the as-is codebase moved here for convenience.
// See unused ideas at least briefly mentioned at some point I guess.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* Time based Damage works as follows:
	1) There are several types of timebased damage:

		#define DMG_PARALYZE		(1 << 14)	// slows affected creature down
		#define DMG_NERVEGAS		(1 << 15)	// nerve toxins, very bad
		#define DMG_POISON			(1 << 16)	// blood poisioning
		#define DMG_RADIATION		(1 << 17)	// radiation exposure
		#define DMG_DROWNRECOVER	(1 << 18)	// drown recovery
		#define DMG_ACID			(1 << 19)	// toxic chemicals or acid burns
		#define DMG_SLOWBURN		(1 << 20)	// in an oven
		#define DMG_SLOWFREEZE		(1 << 21)	// in a subzero freezer

	2) A new hit inflicting tbd restarts the tbd counter - each monster has an 8bit counter,
		per damage type. The counter is decremented every second, so the maximum time
		an effect will last is 255/60 = 4.25 minutes.  Of course, staying within the radius
		of a damaging effect like fire, nervegas, radiation will continually reset the counter to max.

	3) Every second that a tbd counter is running, the player takes damage.  The damage
		is determined by the type of tdb.
			Paralyze		- 1/2 movement rate, 30 second duration.
			Nervegas		- 5 points per second, 16 second duration = 80 points max dose.
			Poison			- 2 points per second, 25 second duration = 50 points max dose.
			Radiation		- 1 point per second, 50 second duration = 50 points max dose.
			Drown			- 5 points per second, 2 second duration.
			Acid/Chemical	- 5 points per second, 10 second duration = 50 points max.
			Burn			- 10 points per second, 2 second duration.
			Freeze			- 3 points per second, 10 second duration = 30 points max.

	4) Certain actions or countermeasures counteract the damaging effects of tbds:

		Armor/Heater/Cooler - Chemical(acid),burn, freeze all do damage to armor power, then to body
							- recharged by suit recharger
		Air In Lungs		- drowning damage is done to air in lungs first, then to body
							- recharged by poking head out of water
							- 10 seconds if swiming fast
		Air In SCUBA		- drowning damage is done to air in tanks first, then to body
							- 2 minutes in tanks. Need new tank once empty.
		Radiation Syringe	- Each syringe full provides protection vs one radiation dosage
		Antitoxin Syringe	- Each syringe full provides protection vs one poisoning (nervegas or poison).
		Health kit			- Immediate stop to acid/chemical, fire or freeze damage.
		Radiation Shower	- Immediate stop to radiation damage, acid/chemical or fire damage.

*/

// If player is taking time based damage, continue doing damage to player -
// this simulates the effect of being poisoned, gassed, dosed with radiation etc -
// anything that continues to do damage even after the initial contact stops.
// Update all time based damage counters, and shut off any that are done.

// The m_bitsDamageType bit MUST be set if any damage is to be taken.
// This routine will detect the initial on value of the m_bitsDamageType
// and init the appropriate counter.  Only processes damage every second.





/*
THE POWER SUIT

The Suit provides 3 main functions: Protection, Notification and Augmentation.
Some functions are automatic, some require power.
The player gets the suit shortly after getting off the train in C1A0 and it stays
with him for the entire game.

Protection

	Heat/Cold
		When the player enters a hot/cold area, the heating/cooling indicator on the suit
		will come on and the battery will drain while the player stays in the area.
		After the battery is dead, the player starts to take damage.
		This feature is built into the suit and is automatically engaged.
	Radiation Syringe
		This will cause the player to be immune from the effects of radiation for N seconds. Single use item.
	Anti-Toxin Syringe
		This will cure the player from being poisoned. Single use item.
	Health
		Small (1st aid kits, food, etc.)
		Large (boxes on walls)
	Armor
		The armor works using energy to create a protective field that deflects a
		percentage of damage projectile and explosive attacks. After the armor has been deployed,
		it will attempt to recharge itself to full capacity with the energy reserves from the battery.
		It takes the armor N seconds to fully charge.

Notification (via the HUD)

x	Health
x	Ammo
x	Automatic Health Care
		Notifies the player when automatic healing has been engaged.
x	Geiger counter
		Classic Geiger counter sound and status bar at top of HUD
		alerts player to dangerous levels of radiation. This is not visible when radiation levels are normal.
x	Poison
	Armor
		Displays the current level of armor.

Augmentation

	Reanimation (w/adrenaline)
		Causes the player to come back to life after he has been dead for 3 seconds.
		Will not work if player was gibbed. Single use.
	Long Jump
		Used by hitting the ??? key(s). Caused the player to further than normal.
	SCUBA
		Used automatically after picked up and after player enters the water.
		Works for N seconds. Single use.

Things powered by the battery

	Armor
		Uses N watts for every M units of damage.
	Heat/Cool
		Uses N watts for every second in hot/cold area.
	Long Jump
		Uses N watts for every jump.
	Alien Cloak
		Uses N watts for each use. Each use lasts M seconds.
	Alien Shield
		Augments armor. Reduces Armor drain by one half

*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////



// Global Savedata for player
TYPEDESCRIPTION	CBasePlayer::m_playerSaveData[] = 
{
	//MODDDREMOVE - don't see anything here different?
	DEFINE_FIELD(CBasePlayer, m_flFlashLightTime, FIELD_TIME),
	DEFINE_FIELD(CBasePlayer, m_iFlashBattery, FIELD_INTEGER),

	DEFINE_FIELD(CBasePlayer, m_afButtonLast, FIELD_INTEGER),
	DEFINE_FIELD(CBasePlayer, m_afButtonPressed, FIELD_INTEGER),
	DEFINE_FIELD(CBasePlayer, m_afButtonReleased, FIELD_INTEGER),

	DEFINE_ARRAY(CBasePlayer, m_rgItems, FIELD_INTEGER, MAX_ITEMS),
	DEFINE_FIELD(CBasePlayer, m_afPhysicsFlags, FIELD_INTEGER),

	DEFINE_FIELD(CBasePlayer, m_flTimeStepSound, FIELD_TIME),
	DEFINE_FIELD(CBasePlayer, m_flTimeWeaponIdle, FIELD_TIME),
	DEFINE_FIELD(CBasePlayer, m_flSwimTime, FIELD_TIME),
	DEFINE_FIELD(CBasePlayer, m_flDuckTime, FIELD_TIME),
	DEFINE_FIELD(CBasePlayer, m_flWallJumpTime, FIELD_TIME),

	DEFINE_FIELD(CBasePlayer, m_flSuitUpdate, FIELD_TIME),
	DEFINE_ARRAY(CBasePlayer, m_rgSuitPlayList, FIELD_INTEGER, CSUITPLAYLIST),

	DEFINE_ARRAY(CBasePlayer, m_rgSuitPlayListDuration, FIELD_FLOAT, CSUITPLAYLIST),

	DEFINE_FIELD(CBasePlayer, m_iSuitPlayNext, FIELD_INTEGER),
	DEFINE_ARRAY(CBasePlayer, m_rgiSuitNoRepeat, FIELD_INTEGER, CSUITNOREPEAT),
	DEFINE_ARRAY(CBasePlayer, m_rgflSuitNoRepeatTime, FIELD_TIME, CSUITNOREPEAT),


	//MODDD - no longer only for the player.  The var was not even used outside the 
	// TakeDamage method in as-is anyway.
	//DEFINE_FIELD(CBasePlayer, m_lastDamageAmount, FIELD_INTEGER),

	DEFINE_ARRAY(CBasePlayer, m_rgpPlayerItems, FIELD_CLASSPTR, MAX_ITEM_TYPES),
	DEFINE_FIELD(CBasePlayer, m_pActiveItem, FIELD_CLASSPTR),
	DEFINE_FIELD(CBasePlayer, m_pLastItem, FIELD_CLASSPTR),

	DEFINE_ARRAY(CBasePlayer, m_rgAmmo, FIELD_INTEGER, MAX_AMMO_TYPES),
	DEFINE_FIELD(CBasePlayer, m_idrowndmg, FIELD_INTEGER),
	DEFINE_FIELD(CBasePlayer, m_idrownrestored, FIELD_INTEGER),
	//           why
	//DEFINE_FIELD(CBasePlayer, m_tSneaking, FIELD_TIME),

	DEFINE_FIELD(CBasePlayer, m_iTrain, FIELD_INTEGER),
	DEFINE_FIELD(CBasePlayer, m_bitsHUDDamage, FIELD_INTEGER),
	DEFINE_FIELD(CBasePlayer, m_flFallVelocity, FIELD_FLOAT),
	DEFINE_FIELD(CBasePlayer, m_iTargetVolume, FIELD_INTEGER),
	DEFINE_FIELD(CBasePlayer, m_iWeaponVolume, FIELD_INTEGER),
	DEFINE_FIELD(CBasePlayer, m_iExtraSoundTypes, FIELD_INTEGER),
	DEFINE_FIELD(CBasePlayer, m_iWeaponFlash, FIELD_INTEGER),
	DEFINE_FIELD(CBasePlayer, m_fLongJump, FIELD_BOOLEAN),
	DEFINE_FIELD(CBasePlayer, m_fInitHUD, FIELD_BOOLEAN),
	

	DEFINE_FIELD(CBasePlayer, m_pTank, FIELD_EHANDLE),
	DEFINE_FIELD(CBasePlayer, m_iHideHUD, FIELD_INTEGER),
	DEFINE_FIELD(CBasePlayer, m_iFOV, FIELD_INTEGER),

	//MODDD - NEW VAR
	DEFINE_FIELD(CBasePlayer, airTankAirTime, FIELD_FLOAT),
	DEFINE_FIELD(CBasePlayer, longJumpCharge, FIELD_FLOAT),
	DEFINE_FIELD(CBasePlayer, m_fLongJumpMemory, FIELD_BOOLEAN),
	DEFINE_FIELD(CBasePlayer, longJumpDelay, FIELD_FLOAT),
	DEFINE_FIELD(CBasePlayer, longJump_waitForRelease, FIELD_BOOLEAN),
	DEFINE_FIELD(CBasePlayer, lastDuckVelocityLength, FIELD_FLOAT),
	//DEFINE_FIELD(CBasePlayer, lastDuckVelocityLength, FIELD_FLOAT),
	
	//DEFINE_FIELD(CBasePlayer, glockSilencerOnVar, FIELD_FLOAT),
	//DEFINE_FIELD(CBasePlayer, egonAltFireOnVar, FIELD_FLOAT),
	
	DEFINE_FIELD(CBasePlayer, recoveryIndex, FIELD_INTEGER),
	DEFINE_FIELD(CBasePlayer, recoveryDelay, FIELD_TIME),
	DEFINE_FIELD(CBasePlayer, recoveryDelayMin, FIELD_TIME),
	
	DEFINE_FIELD(CBasePlayer, recentlyGibbed, FIELD_BOOLEAN),
	DEFINE_FIELD(CBasePlayer, airTankWaitingStart, FIELD_BOOLEAN),
	DEFINE_FIELD(CBasePlayer, hasLongJumpItem, FIELD_BOOLEAN),

	DEFINE_FIELD(CBasePlayer, foundRadiation, FIELD_BOOLEAN),
	
	DEFINE_FIELD( CBasePlayer, m_fNoPlayerSound, FIELD_INTEGER ), // NOTE: added back in.  No issue.
	
	//MODDD - m_flNextAttack saved in CBasePlayer now.  Removed from CBaseMonster due to being
	// unused there and by most (if not all) subclasses.
	DEFINE_FIELD( CBasePlayer, m_flNextAttack, FIELD_TIME ),

	DEFINE_FIELD( CBasePlayer, hasGlockSilencer, FIELD_INTEGER ),

	DEFINE_FIELD(CBasePlayer, recentMajorTriggerDamage, FIELD_BOOLEAN),
	DEFINE_FIELD(CBasePlayer, lastBlockDamageAttemptReceived, FIELD_TIME),
	DEFINE_FIELD(CBasePlayer, recentRevivedTime, FIELD_TIME),

	DEFINE_FIELD(CBasePlayer, alreadySentSatchelOutOfAmmoNotice, FIELD_BOOLEAN),
	// ?
	DEFINE_FIELD(CBasePlayer, m_flNextAmmoBurn, FIELD_FLOAT),
	DEFINE_FIELD(CBasePlayer, deadStage, FIELD_INTEGER ),
	DEFINE_FIELD(CBasePlayer, nextDeadStageTime, FIELD_TIME),
	

	DEFINE_ARRAY(CBasePlayer, recentDeadPlayerFollowers, FIELD_EHANDLE, 5),
	DEFINE_FIELD(CBasePlayer, recentDeadPlayerFollowersCount, FIELD_INTEGER ),
	
	//DEFINE_FIELD( CBasePlayer, m_fDeadTime, FIELD_FLOAT ), // only used in multiplayer games
	//DEFINE_FIELD( CBasePlayer, m_fGameHUDInitialized, FIELD_INTEGER ), // only used in multiplayer games
	//DEFINE_FIELD( CBasePlayer, m_flStopExtraSoundTime, FIELD_TIME ),
	//DEFINE_FIELD( CBasePlayer, m_fKnownItem, FIELD_INTEGER ), // reset to zero on load
	//DEFINE_FIELD( CBasePlayer, m_iPlayerSound, FIELD_INTEGER ),	// Don't restore, set in Precache()
	//DEFINE_FIELD( CBasePlayer, m_pentSndLast, FIELD_EDICT ),	// Don't restore, client needs reset
	//DEFINE_FIELD( CBasePlayer, m_flSndRoomtype, FIELD_FLOAT ),	// Don't restore, client needs reset
	//DEFINE_FIELD( CBasePlayer, m_flSndRange, FIELD_FLOAT ),	// Don't restore, client needs reset
	//DEFINE_FIELD( CBasePlayer, m_fNewAmmo, FIELD_INTEGER ), // Don't restore, client needs reset
	//DEFINE_FIELD( CBasePlayer, m_flgeigerRange, FIELD_FLOAT ),	// Don't restore, reset in Precache()
	//DEFINE_FIELD( CBasePlayer, m_flgeigerDelay, FIELD_FLOAT ),	// Don't restore, reset in Precache()
	//DEFINE_FIELD( CBasePlayer, m_igeigerRangePrev, FIELD_FLOAT ),	// Don't restore, reset in Precache()
	//DEFINE_FIELD( CBasePlayer, m_iStepLeft, FIELD_INTEGER ), // Don't need to restore
	//DEFINE_ARRAY( CBasePlayer, m_szTextureName, FIELD_CHARACTER, CBTEXTURENAMEMAX ), // Don't need to restore
	//DEFINE_FIELD( CBasePlayer, m_chTextureType, FIELD_CHARACTER ), // Don't need to restore
	//DEFINE_FIELD( CBasePlayer, m_fNoPlayerSound, FIELD_BOOLEAN ), // Don't need to restore, debug
	//DEFINE_FIELD( CBasePlayer, m_iUpdateTime, FIELD_INTEGER ), // Don't need to restore

	// !!! If put back in for whatever reason, these are FLOATs now!
	//DEFINE_FIELD( CBasePlayer, m_iClientHealth, FIELD_INTEGER ), // Don't restore, client needs reset
	//DEFINE_FIELD( CBasePlayer, m_iClientBattery, FIELD_INTEGER ), // Don't restore, client needs reset

	//DEFINE_FIELD( CBasePlayer, m_iClientHideHUD, FIELD_INTEGER ), // Don't restore, client needs reset
	//DEFINE_FIELD( CBasePlayer, m_fWeapon, FIELD_BOOLEAN ),  // Don't restore, client needs reset
	//DEFINE_FIELD( CBasePlayer, m_nCustomSprayFrames, FIELD_INTEGER ), // Don't restore, depends on server message after spawning and only matters in multiplayer
	//DEFINE_FIELD( CBasePlayer, m_vecAutoAim, FIELD_VECTOR ), // Don't save/restore - this is recomputed
	//DEFINE_ARRAY( CBasePlayer, m_rgAmmoLast, FIELD_INTEGER, MAX_AMMO_TYPES ), // Don't need to restore
	//DEFINE_FIELD( CBasePlayer, m_fOnTarget, FIELD_BOOLEAN ), // Don't need to restore
	//DEFINE_FIELD( CBasePlayer, m_nCustomSprayFrames, FIELD_INTEGER ), // Don't need to restore
};




//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// GLOBAL METHODS, sit at the top.

// LinkUserMessages and message ID's moved to client.cpp.
// global methods VecVelocityForDamage, ThrowGib, and ThrowHead removed, unused or otherwise dead code.


int TrainSpeed(int iSpeed, int iMax)
{
	float fSpeed, fMax;
	int iRet = 0;

	fMax = (float)iMax;
	fSpeed = iSpeed;

	fSpeed = fSpeed/fMax;

	if (iSpeed < 0)
		iRet = TRAIN_BACK;
	else if (iSpeed == 0)
		iRet = TRAIN_NEUTRAL;
	else if (fSpeed < 0.33)
		iRet = TRAIN_SLOW;
	else if (fSpeed < 0.66)
		iRet = TRAIN_MEDIUM;
	else
		iRet = TRAIN_FAST;

	return iRet;
}



// checks if the spot is clear of players
BOOL IsSpawnPointValid( CBaseEntity *pPlayer, CBaseEntity *pSpot )
{
	CBaseEntity *ent = NULL;

	if ( !pSpot->IsTriggered( pPlayer ) )
	{
		return FALSE;
	}

	while ( (ent = UTIL_FindEntityInSphere( ent, pSpot->pev->origin, 128 )) != NULL )
	{
		// if ent is a client, don't spawn on 'em
		if ( ent->IsPlayer() && ent != pPlayer )
			return FALSE;
	}

	return TRUE;
}


//MODDD - moved from client.cpp.
// called by ClientKill and DeadThink
void respawn(entvars_t* pev, BOOL fCopyCorpse)
{
	if (gpGlobals->coop || gpGlobals->deathmatch)
	{
		if ( fCopyCorpse )
		{
			// make a copy of the dead body for appearances sake
			CopyToBodyQue(pev);
		}

		// respawn player
		GetClassPtr( (CBasePlayer *)pev)->Spawn( );
	}
	else
	{       // restart the entire server
		SERVER_COMMAND("reload\n");
	}
}


/*
============
EntSelectSpawnPoint

Returns the entity to spawn at

USES AND SETS GLOBAL g_pLastSpawn
============
*/
edict_t *EntSelectSpawnPoint( CBaseEntity *pPlayer )
{
	CBaseEntity *pSpot;
	edict_t		*player;

	player = pPlayer->edict();

// choose a info_player_deathmatch point
	if (g_pGameRules->IsCoOp())
	{
		pSpot = UTIL_FindEntityByClassname( g_pLastSpawn, "info_player_coop");
		if ( !FNullEnt(pSpot) )
			goto ReturnSpot;
		pSpot = UTIL_FindEntityByClassname( g_pLastSpawn, "info_player_start");
		if ( !FNullEnt(pSpot) ) 
			goto ReturnSpot;
	}
	else if ( g_pGameRules->IsDeathmatch() )
	{
		pSpot = g_pLastSpawn;
		// Randomize the start spot
		for ( int i = RANDOM_LONG(1,5); i > 0; i-- )
			pSpot = UTIL_FindEntityByClassname( pSpot, "info_player_deathmatch" );
		if ( FNullEnt( pSpot ) )  // skip over the null point
			pSpot = UTIL_FindEntityByClassname( pSpot, "info_player_deathmatch" );

		CBaseEntity *pFirstSpot = pSpot;

		do 
		{
			if ( pSpot )
			{
				// check if pSpot is valid
				if ( IsSpawnPointValid( pPlayer, pSpot ) )
				{
					if ( pSpot->pev->origin == Vector( 0, 0, 0 ) )
					{
						pSpot = UTIL_FindEntityByClassname( pSpot, "info_player_deathmatch" );
						continue;
					}

					// if so, go to pSpot
					goto ReturnSpot;
				}
			}
			// increment pSpot
			pSpot = UTIL_FindEntityByClassname( pSpot, "info_player_deathmatch" );
		} while ( pSpot != pFirstSpot ); // loop if we're not back to the start

		// we haven't found a place to spawn yet,  so kill any guy at the first spawn point and spawn there
		if ( !FNullEnt( pSpot ) )
		{
			CBaseEntity *ent = NULL;
			while ( (ent = UTIL_FindEntityInSphere( ent, pSpot->pev->origin, 128 )) != NULL )
			{
				// if ent is a client, kill em (unless they are ourselves)
				if ( ent->IsPlayer() && !(ent->edict() == player) )
					ent->TakeDamage( VARS(INDEXENT(0)), VARS(INDEXENT(0)), 300, DMG_GENERIC );
			}
			goto ReturnSpot;
		}
	}

	// If startspot is set, (re)spawn there.
	//MODDD - order changed a bit.  If the tried 'gpGlobals->startspot' does not exist,
	// make a printout about it and just use info_player_start.

	if ( !FStringNull( gpGlobals->startspot ) && strlen(STRING(gpGlobals->startspot))){

		pSpot = UTIL_FindEntityByTargetname( NULL, STRING(gpGlobals->startspot) );
		if ( !FNullEnt(pSpot) ){
			goto ReturnSpot;
		}else{
			easyForcePrintLine("WARNING!  Player Spawn: Startspot '%s' not found, falling back to default info_player_start", STRING(gpGlobals->startspot) );
		}
	}

	
	pSpot = UTIL_FindEntityByClassname(NULL, "info_player_start");
	if ( !FNullEnt(pSpot) )
		goto ReturnSpot;
	

ReturnSpot:
	if ( FNullEnt( pSpot ) )
	{
		ALERT(at_error, "PutClientInServer: no info_player_start on level");
		return INDEXENT(0);
	}

	g_pLastSpawn = pSpot;
	return pSpot->edict();
}






// This is a glorious hack to find free space when you've crouched into some solid space
// Our crouching collisions do not work correctly for some reason and this is easier
// than fixing the problem :(
void FixPlayerCrouchStuck( edict_t *pPlayer )
{
	TraceResult trace;

	// Move up as many as 18 pixels if the player is stuck.
	for ( int i = 0; i < 18; i++ )
	{
		UTIL_TraceHull( pPlayer->v.origin, pPlayer->v.origin, dont_ignore_monsters, head_hull, pPlayer, &trace );
		if ( trace.fStartSolid )
			pPlayer->v.origin.z ++;
		else
			break;
	}
}


/*
================
CheckPowerups

Check for turning off powerups

GLOBALS ASSUMED SET:  g_ulModelIndexPlayer
================
*/
static void CheckPowerups(entvars_t *pev)
{
	if (pev->health <= 0)
		return;

	pev->modelindex = g_ulModelIndexPlayer;    // don't use eyes
}






//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////







//MODDD - new
inline void CBasePlayer::resetLongJumpCharge(){

	if(m_fLongJump){
		//Can render an empty spring icon.
		longJumpCharge = 0;
	}else{
		//don't even render to GUI.
		longJumpCharge = -1;
	}

}


//NOTICE - this is the default "PainSound" method that any Monster has.  It is called by the base monster class's TakeDamage method.
//Below, PainChance is custom and new for the player, and can only be called by the Player in here.
void CBasePlayer::PainSound( void )
{
	float flRndSound;//sound randomizer

	//disallow making noise if this CVar is on.
	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hideDamage) >= 1 || EASY_CVAR_GET_DEBUGONLY(mutePlayerPainSounds) == 1 || EASY_CVAR_GET_DEBUGONLY(playerExtraPainSoundsMode) == 2){
		//playerExtraPainSoundsMode of 2 suggets that we don't want to use the default PainSound method at all.
		return;
	}

	flRndSound = RANDOM_FLOAT ( 0 , 1 ); 
	
	if (flRndSound <= 0.33) {
		UTIL_PlaySound(ENT(pev), CHAN_VOICE, "player/pl_pain5.wav", 1, ATTN_NORM, FALSE);
	}else if (flRndSound <= 0.66) {
		UTIL_PlaySound(ENT(pev), CHAN_VOICE, "player/pl_pain6.wav", 1, ATTN_NORM, FALSE);
	}else {
		UTIL_PlaySound(ENT(pev), CHAN_VOICE, "player/pl_pain7.wav", 1, ATTN_NORM, FALSE);
	}
}//END OF PainSound


//A chance of 3 / 5 to play pain, 2 / 5 nothing.  Spotted across the script, condensed here for ease of (re)use.
void CBasePlayer::PainChance( void )
{

	//disallow making noise if this CVar is on.
	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hideDamage) >= 1 || EASY_CVAR_GET_DEBUGONLY(mutePlayerPainSounds) == 1){
		return;
	}

	//NOTICE that #4 and #5 are possible (which, as of writing, do nothing at all: no sound).
	switch (RANDOM_LONG(1,5)) 
	{
	case 1: UTIL_PlaySound(ENT(pev), CHAN_VOICE, "player/pl_pain5.wav", 1, ATTN_NORM, FALSE);break;
	case 2: UTIL_PlaySound(ENT(pev), CHAN_VOICE, "player/pl_pain6.wav", 1, ATTN_NORM, FALSE);break;
	case 3: UTIL_PlaySound(ENT(pev), CHAN_VOICE, "player/pl_pain7.wav", 1, ATTN_NORM, FALSE);break;
	}

}//END OF PainChance



//MODDD - DeathSound method that takes no 
void CBasePlayer::DeathSound( void ){
	//assume a revive is not planned.
	DeathSound(FALSE);
}


void CBasePlayer::DeathSound( BOOL plannedRevive )
{
	// water death sounds
	if (pev->waterlevel == 3)
	{
		UTIL_PlaySound(ENT(pev), CHAN_VOICE, "player/h2odeath.wav", 1, ATTN_NONE, FALSE);
		return;
	}

	// temporarily using pain sounds for death sounds
	//NOTICE that #4 and #5 are possible (which, as of writing, do nothing at all: no sound).
	//MODDD - redirected...
	PainChance();

	//MODDD
	//Try something more specific later?
	
	// play one of the suit death alarms
	if(plannedRevive == FALSE){
		//play the usual beeps and flatline.  "Cause of Death" FVOX (player suit voice), if 
		//implemented, will probably be played when not planning a revive too.
		m_flSuitUpdate = gpGlobals->time;  //say the next line now!

		
		//if (RANDOM_FLOAT(0, 1) <= 0.4) {
		//	// general failure notice
		////SUIT_NEXT_IN_1MIN * 2
		//	SetSuitUpdate("!HEV_E3", FALSE, SUIT_REPEAT_OK, 4.2f);
		//}
		//else {
		//     SENTENCEG_Stop


		// MODDD - NOTICE!!!  Not supporting remembering the most recently played sentence just to be used twice in the player class.
		// Would need to have a copy of the sentence recently played at all times to know what to stop, which just isn't worth it.
		// This rarely ever comes up.  So, we're good.

			EMIT_GROUPNAME_SUIT(ENT(pev), "HEV_DEAD");
			//strcpy(recentlyPlayedSound, "HEV_DEAD");
		//}
		
	}else{

		/*
		float fvol = CVAR_GET_FLOAT("suitvolume");
		if (fvol > 0.05){

			//SENTENCEG_PlayRndSz(ENT(pev), "HEV_DEAD_ADR", fvol, ATTN_NORM, 0, PITCH_NORM);

			//example?
			//SENTENCEG_PlayRndSz( ENT(pev), "HG_ALERT", HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
			//WRONG.
			//EMIT_SOUND_DYN(ENT(pev), CHAN_STATIC, "HEV_DEAD_ADR", fvol, ATTN_NORM, 0, PITCH_NORM);
		}
		//EMIT_SOUND_SUIT(ENT(pev), "HEV_DEAD_ADR");
		//for now, don't play the flatline.
		//SetSuitUpdate("!HEV_DEAD_ADR", FALSE, SUIT_NEXT_IN_30SEC);
		*/
		m_flSuitUpdate = gpGlobals->time;  //say the next line now!
		EMIT_GROUPNAME_SUIT(ENT(pev), "HEV_DEADALT");
		//strcpy(recentlyPlayedSound, "HEV_DEADALT");
	}
}



// PARANOIA:  Dummy these methods, player revive logic does not involve this!
void CBasePlayer::StartReanimation() {
}
void CBasePlayer::StartReanimationPost(int preReviveSequence) {
}

// Convenient for cheat methods to revive the player if a cheat is used while the player is dead.
// Obviously they wanted to continue playing.
void CBasePlayer::reviveIfDead() {
	if (pev->deadflag != DEAD_NO) {
		// revive the player
		stopSelfSounds();
		Spawn(TRUE);
	}
}


void CBasePlayer::startRevive(void) {
	// can recover.
	//if (!adrenalineQueued) {
		recoveryIndex = 1;
	//	adrenalineQueued = TRUE;
		SetSuitUpdateEventFVoxCutoff("!HEV_ADR_USE", FALSE, SUIT_REPEAT_OK, SUITUPDATETIME, TRUE, 0.41 - 0.07, &CBasePlayer::consumeAdrenaline, 0.41 - 0.07 + 0.55);
	//}
	//if(revived){
		// If revived, send the signal to reset falling velocity.
	g_engfuncs.pfnSetPhysicsKeyValue(edict(), "res", "1");
	//}

}

// For having planned a revive, but deciding against it (fell too far on hitting the ground, stuck in geometry)
void CBasePlayer::declareRevivelessDead(void) {
	recoveryIndex = 3;
	if (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(timedDamageDeathRemoveMode) > 0) {
		attemptResetTimedDamage(TRUE);
	}
	if (EASY_CVAR_GET_DEBUGONLY(batteryDrainsAtDeath) == 1) {
		SetAndUpdateBattery(0);
	}
	
	if(!IsMultiplayer()){
		if(EASY_CVAR_GET(playerDeadTruce) >= 1){
			// IDEA: if this CVar is on (maybe only once there' a line of sight between anything and the player),
			// do the truce between hmilitary and playerally.
			UTIL_SetDeadPlayerTruce(TRUE);
		}
	}
}



// override takehealth
// bitsDamageType indicates type of damage healed. 
int CBasePlayer::TakeHealth( float flHealth, int bitsDamageType )
{
	return CBaseMonster::TakeHealth (flHealth, bitsDamageType);

}

Vector CBasePlayer::GetGunPosition( )
{
//	UTIL_MakeVectors(pev->v_angle);
//	m_HackedGunPos = pev->view_ofs;
	Vector origin;
	origin = pev->origin + pev->view_ofs;
	return origin;
}
Vector CBasePlayer::GetGunPositionAI(){
	return GetGunPosition();
}



//=========================================================
// TraceAttack
//=========================================================
//MODDD - uses the maximum args to guarantee this version ends up getting called.
GENERATE_TRACEATTACK_IMPLEMENTATION(CBasePlayer)
{
	//Just a print-out that I needed.
	/*
	easyPrint("Yes.......%d\n", 0);
	int eye = 0;
	for (eye = 1; eye < MAX_AMMO_TYPES; eye++)
	{
		easyPrint("HELP %s\n", CBasePlayerItem::AmmoInfoArray[eye].pszName);

	}
	*/
	
	//easyForcePrintLine("AddMultiDamage CALL FROM TRACEATTACK. Attacker:%s Victim:%s hitgrp:%d Dmg:%.2f", pevAttacker!=NULL?STRING(pevAttacker->classname):"NULL", this->getClassname(), ptr->iHitgroup, flDamage);


	if ( pev->takedamage )
	{
		BOOL isAttackerPlayer = FALSE;

		if(pevAttacker != NULL){
			CBaseEntity* entTest = CBaseEntity::Instance(pevAttacker);

			if (entTest == NULL) {
				//What????
				easyForcePrintLine("who is this living contradiction??? %s", STRING(pevAttacker->classname));
				int xxx = 46;

			}else{
				if (entTest->IsPlayer()) {
					isAttackerPlayer = TRUE;
				}
			}
		}else{
			//all we can assume. leave FALSE.
		}
		
		m_LastHitGroup = ptr->iHitgroup;

		//MODDD - don't allow damage edits based on hitbox if this type of damage forbids it.
		//It doesn't make sense for explosions or lightning to different damage based on where it hits.
		//Or for enemies that don't even try to target hitboxes, it makes this mostly luck or looking at them
		//from a bad (unlucky) angle to put your head in the way first.
		//Kinda opens the question as to whether all NPC-based damage should have these damage enhancements off.
		//Maybe turn them off all the time for single player? who knows.
		//Also, some throw around these damage type for bitsDamageType for reductions to helmets, but not always all:
		//(bitsDamageType & (DMG_BULLET | DMG_SLASH | DMG_BLAST | DMG_CLUB))
		//...a lot of the times, damage types based on some natural force (in lack of a better term) like burn, shock, etc.
		// may also be ok to assume for blocking hitbox checks.

		//ALSO - new CVar, "monsterToPlayerHitgroupSpecial".
		//0: never special. Always 100% of intended damage.
		//1: on. Monsters can do enhanced or reduced damage depending on the hitgroup hit, such as triple damage for headshots.
		//2: semi. Monsters can potentially do reduced damage if any area reduces damage instead (as of retail skill.cfg settings, likely unchanged, nowhere), but never enhanced damage such as from headshots.
		//   In other words, damage modifiers are possible from what hitgroup was touched, but it can't go above 100% of intended damage like triple.
		if( (isAttackerPlayer || EASY_CVAR_GET_DEBUGONLY(monsterToPlayerHitgroupSpecial) > 0) && !(bitsDamageTypeMod & DMG_HITBOX_EQUAL) ){
			float damageModifier = 1.0f;
			
			switch ( ptr->iHitgroup )
			{
			case HITGROUP_GENERIC:
				break;
			case HITGROUP_HEAD:
				//flDamage *= gSkillData.plrHead;
				damageModifier = gSkillData.plrHead;
				break;
			case HITGROUP_CHEST:
				damageModifier = gSkillData.plrChest;
				break;
			case HITGROUP_STOMACH:
				damageModifier = gSkillData.plrStomach;
				break;
			case HITGROUP_LEFTARM:
			case HITGROUP_RIGHTARM:
				damageModifier = gSkillData.plrArm;
				break;
			case HITGROUP_LEFTLEG:
			case HITGROUP_RIGHTLEG:
				damageModifier = gSkillData.plrLeg;
				break;
			default:
				break;
			}

			if(!isAttackerPlayer && EASY_CVAR_GET_DEBUGONLY(monsterToPlayerHitgroupSpecial) == 2){
				// Don't let the modifier picked exceed 100% for player-induced damage.
				if(damageModifier > 1.0f){
					damageModifier = 1.0f;
				}
			}

			if(damageModifier != 1.0f){
				// apply.
				flDamage *= damageModifier;
			}

		}//END OF DMG_HITBOX_EQUAL damage type check



		//MODDD
		//SpawnBlood(ptr->vecEndPos, BloodColor(), flDamage);// a little surface blood.


		if(EASY_CVAR_GET_DEBUGONLY(RadiusDamageDrawDebug) == 1)DebugLine_Setup(1, ptr->vecEndPos + Vector(0, 0, -20), ptr->vecEndPos + Vector(0, 0, 20), 0, 0, 255);
		
		//NEVERMIND THIS REDUCTION. Other places don't disable bleeding per this Var so it's a lost cause. Just leave this, not too jarring or distracting.
		//if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hideDamage) <= 0){
			SpawnBlood(ptr->vecEndPos, flDamage);

			//already seems to play?  Verify!
			/*
			if(useBulletHitSound && (pevAttacker != NULL && (pevAttacker->renderfx & (ISPLAYER | ISNPC))) ){
				UTIL_playFleshHitSound(pev);
			}
			*/

			TraceBleed( flDamage, vecDir, ptr, bitsDamageType, bitsDamageTypeMod );
		//}//END OF hideDamage check

		//easyForcePrintLine("AddMultiDamage CALL FROM TRACEATTACK. Attacker:%s Victim:%s hitgrp:%d Dmg:%.2f", pevAttacker!=NULL?STRING(pevAttacker->classname):"NULL", this->getClassname(), ptr->iHitgroup, flDamage);
		
		AddMultiDamage( pevAttacker, this, flDamage, bitsDamageType, bitsDamageTypeMod );
	}//END OF if pev->takedamage

}//END OF TraceAttack


/*
	Take some damage.  
	NOTE: each call to TakeDamage with bitsDamageType set to a time-based damage
	type will cause the damage time countdown to be reset.  Thus the ongoing effects of poison, radiation
	etc are implemented with subsequent calls to TakeDamage using DMG_GENERIC.
*/
//MODDD - this one NOW accepts the "unsigned int".  It may be used to better convey a greater power of 2.
//Otherwise, capacity is wasted on negatives (unused).
GENERATE_TAKEDAMAGE_IMPLEMENTATION(CBasePlayer)
{
	//easyPrintLine("PLAYER TAKEDAMAGE FLAGS: %d, %d", bitsDamageType, bitsDamageTypeMod);
	
	if(reviveSafetyTime != -1){
		// revive safety time will not protect against MAP_BLOCKED damage.
		if (!(bitsDamageTypeMod & DMG_MAP_BLOCKED)) {
		
			if(reviveSafetyTime >= gpGlobals->time){
				if(EASY_CVAR_GET_DEBUGONLY(playerReviveBuddhaMode) == 1){
					buddhaMode = TRUE;
				}else{
					blockDamage = TRUE;
				}
				if(EASY_CVAR_GET_DEBUGONLY(playerReviveTimeBlocksTimedDamage) == 1){
					blockTimedDamage = TRUE;
				}
			}else{
				reviveSafetyTime = -1;
				buddhaMode = FALSE;
				blockDamage = FALSE;
				blockTimedDamage = FALSE;
			}
		}

	}//END OF reviveSafetyTime

	//MODDD - don't make these noises if waiting for a revive.
	//If dead, don't play.
	//Also, was this intended ONLY for fall damage?  If so, just make that the requirement.
	
	// NOTICE - this is only for whether to make the pain sound or not.  Just stop now if there is nothing to make pain at
	// (no damage, and no timed damages started)
	if(IsAlive() &&  (flDamage > 0 || (bitsDamageType & DMG_TIMEBASED) || (bitsDamageTypeMod & DMG_TIMEBASEDMOD) )   ){
		//must be alive to play ANY paint sound effects at all, regardless of setting.
		//Note that a fatal fall will play the sound effect on its own, and it will not need to be played here.
		BOOL pass = FALSE;

		//no chance at passing if hit sounds are muted.
		if(EASY_CVAR_GET_DEBUGONLY(mutePlayerPainSounds) != 1){

			if(EASY_CVAR_GET_DEBUGONLY(playerExtraPainSoundsMode) == 0){
				//no extra pain sounds.
				
			}/*else if(EASY_CVAR_GET_DEBUGONLY(playerExtraPainSoundsMode) == 1){
				if(bitsDamageType & (DMG_FALL) ){
					pass = TRUE;
				}
				
			}*/
			else if(EASY_CVAR_GET_DEBUGONLY(playerExtraPainSoundsMode) == 1 || EASY_CVAR_GET_DEBUGONLY(playerExtraPainSoundsMode) == 2 ){
				pass = TRUE;
			}
			
		}//END OF if(EASY_CVAR_GET_DEBUGONLY(mutePlayerPainSounds) != 1))

		if(pass){
			//AMD889 ADDED THIS, is it the old landing stuff?
			//MODDD - redirected:
			//PainSound();  pain sounds?
			PainChance();
		}

		//} //END OF if(bitsDamageType & (DMG_FALL)
	}//END OF if(IsAlive)
	



	if ((bitsDamageTypeMod & (DMG_MAP_TRIGGER)) && flDamage >= 5) {
		recentMajorTriggerDamage = 7;
	}
	else {
		recentMajorTriggerDamage &= ~2;
		// How about this:  recentMajorTriggerDamage will be a bitmask.
		// Bit #1 (1) means damage during this very frame.  So don't reset it on taking damage another
		// time that isn't trigger damage, reset it at the end of this frame.
		// Bit #2 (2) however, will reset on taking damage from any other source that isn't a trigger.
		// Bit #3 (4) means damage since the last timed-damage-dealing cycle.  It is reset after that.
	}


	if ((bitsDamageTypeMod & (DMG_MAP_BLOCKED)) && flDamage >= 60) {
		// too much damage from a crush? no chance.
		recentMajorTriggerDamage = 7;
	}else {
		lastBlockDamageAttemptReceived = FALSE;
		
	}

	//MODDD - involving the damage requirement now
	if ( (bitsDamageTypeMod & DMG_MAP_BLOCKED) && flDamage > 1) {
		// mark it!
		lastBlockDamageAttemptReceived = gpGlobals->time;
	}




	// have suit diagnose the problem - ie: report damage type
	int bitsDamage = bitsDamageType;
	int bitsDamageMod = bitsDamageTypeMod;
	int ffound = TRUE;
	int fmajor;
	int fcritical;
	int fTookDamage;
	int ftrivial;
	float flRatio;
	float flBonus;
	float flHealthPrev = pev->health;

	flBonus = ARMOR_BONUS;
	flRatio = ARMOR_RATIO;


	//MODDD - only double blast damage on armor, if the CVar allows.  0 is retail behavior (during multiplayer only)
	if (
		(EASY_CVAR_GET(blastExtraArmorDamageMode) == 0 && IsMultiplayer()) ||
		EASY_CVAR_GET(blastExtraArmorDamageMode) == 2
	) {
		if ((bitsDamageType & DMG_BLAST) )
		{
			// blasts damage armor more.
			flBonus *= 2;
		}
	}


	// Already dead
	if (!IsAlive()) {
		recentMajorTriggerDamage &= ~1;
		return 0;
	}
	// go take the damage first

	
	//MODDD - "CBaseEntity::Instance" of a NULL pev, funny enough, crashes.    Yup..
	if(pevAttacker != NULL){
		CBaseEntity *pAttacker = CBaseEntity::Instance(pevAttacker);
	
		//MODDD - is the pAttacker NULL check that forbids returning early okay here?
		if ( pAttacker != NULL && !g_pGameRules->FPlayerCanTakeDamage( this, pAttacker ) )
		{
			// Refuse the damage
			recentMajorTriggerDamage &= ~1;
			return 0;
		}
	}

	// keep track of amount of damage last sustained
	float rawDamageThisFrame = flDamage;

	// Armor. 
	//MODDD - new if-then includes the possibility of marked "DMG_TIMEBASED" (the whole mask being used, not just one piece) ignoring damage or not.
	//if (pev->armorvalue && !(bitsDamageType & (DMG_FALL | DMG_DROWN)) )// armor doesn't protect against fall or drown damage!
	
	//easyPrintLine("B4 DMG : %.2f", flDamage);
	
	//flDamage = 2;   //force all to 2?
	
	//easyPrintLine("DAMAGE PRE DETAILS %d %.8f", fTookDamage, flDamage);


	// How much damage was sustained since the last sendoff?  Like pev->dmg_take, but this gets only raw damage before doing armor damage reductions.
	rawDamageSustained += flDamage;


	//!!!  not that this matters.
	// Athe moment bitmask 'DMG_ARMORBLOCKEXCEPTION' is empty, so any AND operation (&) with it must produce 0.
	// Which is ok, look below.  That makes  "FALSE || ...",   which just makes this operation have no effect on the if-condition.
	//Warning	C6313	Incorrect operator:  zero - valued flag cannot be tested with bitwise - and .Use an equality test to check for zero - valued flags.
	// ALSO, adding DMG_MAP_TRIGGER to armor exceptions.

	// Did I used to have armor before the damage-absorb logic that might run below?
	BOOL hadArmor = (pev->armorvalue > 0);
	float flArmorDamage = 0;


	if (pev->armorvalue && !(bitsDamageType & (DMG_FALL | DMG_DROWN)) && 
		!(bitsDamage & DMG_ARMORBLOCKEXCEPTION || bitsDamageMod & DMG_ARMORBLOCKEXCEPTIONMOD || bitsDamageMod & DMG_MAP_TRIGGER) &&
		(  !(bitsDamageMod & DMG_TIMEDEFFECTIGNORE)   &&   (EASY_CVAR_GET_DEBUGONLY(timedDamageIgnoresArmor) == 0 || !(bitsDamageMod & (DMG_TIMEDEFFECT) ) ))
		
	)  // armor doesn't protect against fall or drown damage!  ... or "DMG_TIMEDEFFECTIGNORE".
	{
		float flNewHealthDamage = flDamage * flRatio;
		
		flArmorDamage = (flDamage - flNewHealthDamage) * flBonus;


		// if(flDamage * (1 - (flRatio) ) * flBonus >= pev->armorvalue) ...

		//easyPrintLine("ARMOR RED: %.2f", flArmorDamage);

		// Does this use more armor than we have?
		if (flArmorDamage > pev->armorvalue)
		{
			// recalculate with what armor we have, use it up.
			flArmorDamage = pev->armorvalue;
			flNewHealthDamage = flDamage - (flArmorDamage * (1 / flBonus));
		}
		else {
			// no change needed.
		}


		if (flArmorDamage >= pev->armorvalue) {

			if (pev->armorvalue > 0) {
				// if the armor value is not already 0 (going from above 0 to 0), we can play a line
				SetSuitUpdate("!HEV_E1", FALSE, SUIT_NEXT_IN_1MIN*2, 4.2f);
			}

			// force to 0 for safety.
			pev->armorvalue = 0;
		}else {
			pev->armorvalue -= flArmorDamage;
		}

		flDamage = flNewHealthDamage;
	}

	if (EASY_CVAR_GET_DEBUGONLY(nothingHurts) > 0) {
		//nothing hurts, the player does not take damage with this cheat on.
		flDamage = 0;
	}

	// this cast to INT is critical!!! If a player ends up with 0.5 health, the engine will get that
	// as an int (zero) and think the player is dead! (this will incite a clientside screentilt, etc)
	// MODDD - CHANGE.  No need to force this to an int anymore!  Decimal player pev->health values are now fine.
	// In client.cpp and here before health msg_func sendoff's, the value sent is forced  to 1 IF it is between
	// 0 and 1.  That's all that really matters.
	// Problem is when rounded-down, it's 0 (as the client receives it), yet over here it's treated as "above 0"
	// from keeping the decimal.  So long as the client receives a forced 1 in such a case, it works fine.
	//flDamage = (int)flDamage;

	fTookDamage = GENERATE_TAKEDAMAGE_PARENT_CALL(CBaseMonster);
	

	//easyPrintLine("AR DMG : %.2f", flDamage);

	// reset damage time countdown for each type of time based damage player just sustained


	// No need! BaseMonster script handles this fine now.
	/*
	if(!blockTimedDamage)
	{
		applyNewTimedDamage(bitsDamageType, bitsDamageTypeMod);
	}
	*/

	// tell director about it
	MESSAGE_BEGIN( MSG_SPEC, SVC_DIRECTOR );
		WRITE_BYTE ( 9 );	// command length in bytes
		WRITE_BYTE ( DRC_CMD_EVENT );	// take damage event
		WRITE_SHORT( ENTINDEX(this->edict()) );	// index number of primary entity
		WRITE_SHORT( ENTINDEX(ENT(pevInflictor)) );	// index number of secondary entity
		WRITE_LONG( 5 );   // eventflags (priority and flags)
	MESSAGE_END();


	// how bad is it, doc?

	ftrivial = (pev->health > 75 || rawDamageThisFrame < 5);
	fmajor = (rawDamageThisFrame > 25);
	fcritical = (pev->health < 30);

	// handle all bits set in this damage message,
	// let the suit give player the diagnosis

	// UNDONE: add sounds for types of damage sustained (ie: burn, shock, slash )
	// UNDONE: still need to record damage and heal messages for the following types
	// MODDD - I READ YA LOUND N' CLEAR, ASSHOLE!

		// DMG_BURN	
		// DMG_FREEZE
		// DMG_BLAST
		// DMG_SHOCK

	//This seems redundant with the base monster class... ?  It does this now.
	/*
	m_bitsDamageType |= bitsDamage; // Save this so we can report it to the client
	m_bitsDamageTypeMod |= bitsDamageMod;
	*/

	m_bitsHUDDamage = -1;  // make sure the damage bits get resent
	m_bitsModHUDDamage = -1;



	recentMajorTriggerDamage &= ~1;

	//MODDD - wait a second.  If this player was killed by the recent hit (TakeDamage call), shouldn't the rest
	// of this method be skipped?
	if (!UTIL_IsAliveEntity(this)) {
		return fTookDamage;
	}



	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hideDamage) >= 1){
		//If so, skip the rest of this method.  It's only about making fvox chatter on taking damage.
		return fTookDamage;
	}

	//MODDD - NOTE
	// freeze damage is checked here instead, since "fTookDamage" isn't called for freeze damage.
	// It is sent from the map, indifferent to SLOWFREEZE's damage (seems to be for the unused
	// instigator of continual freeze damage, even though ordinary freeze damage doesn't seem
	// to be timed).  The map uses its own damage (0.1), which affects armor but not health
	// (b/c damage done to health is truncated first)... that truncation may no longer be the case.
	if (bitsDamage & DMG_FREEZE){
		SetSuitUpdateAndForceBlock("!HEV_FREEZE", FALSE, SUIT_NEXT_IN_30SEC);	
		bitsDamage &= ~DMG_FREEZE;
		ffound = TRUE;
	}

	//while (fTookDamage && (!ftrivial || (bitsDamage & DMG_TIMEBASED)) && ffound && bitsDamage)
	//including "bitsDamageMod".  It has its own mask for its timed damages.
	

	//if "EASY_CVAR_GET_DEBUGONLY(playerReviveTimeBlocksTimedDamage)" is 2, we're blocking notifications about new timed damage as well.
	BOOL blockTimedDamageUpdates = (blockTimedDamage && EASY_CVAR_GET_DEBUGONLY(playerReviveTimeBlocksTimedDamage) == 2);

	//...These are just statuses & removals of non-timed damage for handling them.  No issue to still just report these.  (in regards to revive-invincibility-post-delay)
	while (fTookDamage && (!ftrivial || (bitsDamage & (DMG_BURN | DMG_SHOCK) || bitsDamage & DMG_TIMEBASED) || (bitsDamageMod & DMG_TIMEBASEDMOD)  ) && ffound && bitsDamage)
	{
		ffound = FALSE;

		//MODDD - play messages for these too
		if (bitsDamage & DMG_BURN){
			if(!blockTimedDamageUpdates){
				SetSuitUpdateAndForceBlock("!HEV_FIRE", FALSE, SUIT_NEXT_IN_30SEC);	
			}
			bitsDamage &= ~DMG_BURN;
			ffound = TRUE;
		}
		
		if (bitsDamage & DMG_CLUB)
		{
			if(!blockTimedDamageUpdates){
				if (!fmajor)
					SetSuitUpdate("!HEV_DMG4", FALSE, SUIT_NEXT_IN_30SEC);	// minor fracture
			}
			bitsDamage &= ~DMG_CLUB;
			ffound = TRUE;
		}

		if (bitsDamage & (DMG_FALL | DMG_CRUSH))
		{
			if(!blockTimedDamageUpdates){
				if (fmajor)
					SetSuitUpdate("!HEV_DMG5", FALSE, SUIT_NEXT_IN_30SEC);	// major fracture
				else
					SetSuitUpdate("!HEV_DMG4", FALSE, SUIT_NEXT_IN_30SEC);	// minor fracture
			}


			//MODDD - unsure if this should be for onky DMG_FALL or also for DMG_CRUSH
			if (bitsDamage & (DMG_FALL))
			{
				if(!blockTimedDamageUpdates){
					if (fmajor){
						//MODDD - changed to a custom sentence.
						//SetSuitUpdate("!HEV_HEAL1", FALSE, SUIT_NEXT_IN_30SEC, 6.8f);	// major fracture
						SetSuitUpdate("!HEV_FALLDMG", FALSE, SUIT_NEXT_IN_30SEC, 9.3f);	// major fracture
					}
				}
			}


			bitsDamage &= ~(DMG_FALL | DMG_CRUSH);
			ffound = TRUE;
		}
		
		if (bitsDamage & DMG_BULLET)
		{
			if(!blockTimedDamageUpdates){
				if (rawDamageThisFrame > 5)
					SetSuitUpdate("!HEV_DMG6", FALSE, SUIT_NEXT_IN_30SEC, 3.9f);	// blood loss detected
				else{
					//MODDDREMOVE - ?
					SetSuitUpdate("!HEV_DMG0", FALSE, SUIT_NEXT_IN_30SEC, 3.9f);	// minor laceration
				}
			}

			bitsDamage &= ~DMG_BULLET;
			ffound = TRUE;
		}

		if (bitsDamage & DMG_SLASH)
		{
			if(!blockTimedDamageUpdates){
				if (fmajor)
					SetSuitUpdate("!HEV_DMG1", FALSE, SUIT_NEXT_IN_30SEC, 3.9f);	// major laceration
				else
					SetSuitUpdate("!HEV_DMG0", FALSE, SUIT_NEXT_IN_30SEC, 3.9f);	// minor laceration
			}

			bitsDamage &= ~DMG_SLASH;
			ffound = TRUE;
		}
		
		if (bitsDamage & DMG_SONIC)
		{
			if(!blockTimedDamageUpdates){
				if (fmajor)
					SetSuitUpdate("!HEV_DMG2", FALSE, SUIT_NEXT_IN_1MIN, 3.9f);	// internal bleeding
			}
			bitsDamage &= ~DMG_SONIC;
			ffound = TRUE;
		}

		if ( (bitsDamage & (DMG_POISON | DMG_PARALYZE)) || (bitsDamageMod & (DMG_POISONHALF) ) )
		{
			if(!blockTimedDamageUpdates){
				SetSuitUpdate("!HEV_DMG3", FALSE, SUIT_NEXT_IN_1MIN, 4.6f);	// blood toxins detected
			}

			bitsDamage &= ~(DMG_POISON | DMG_PARALYZE);
			bitsDamageMod &= ~(DMG_POISONHALF);
			ffound = TRUE;
		}

		if (bitsDamage & DMG_ACID)
		{
			if(!blockTimedDamageUpdates){
				SetSuitUpdate("!HEV_DET1", FALSE, SUIT_NEXT_IN_1MIN, 4.3f);	// hazardous chemicals detected
			}
			bitsDamage &= ~DMG_ACID;
			ffound = TRUE;
		}

		if (bitsDamage & DMG_NERVEGAS)
		{
			if(!blockTimedDamageUpdates){
				SetSuitUpdate("!HEV_DET0", FALSE, SUIT_NEXT_IN_1MIN, 4.2f);	// biohazard detected
			}
			bitsDamage &= ~DMG_NERVEGAS;
			ffound = TRUE;
		}

		if (bitsDamage & DMG_RADIATION)
		{
			if(!blockTimedDamageUpdates){
				SetSuitUpdate("!HEV_DET2", FALSE, SUIT_NEXT_IN_1MIN);	// radiation detected
			}

			bitsDamage &= ~DMG_RADIATION;
			ffound = TRUE;
		}
		if (bitsDamage & DMG_SHOCK)
		{
			if(!blockTimedDamageUpdates){
				//MODDD - added.
				SetSuitUpdate("!HEV_SHOCK", FALSE, SUIT_NEXT_IN_30SEC);	
			}

			bitsDamage &= ~DMG_SHOCK;
			ffound = TRUE;
		}
		//MODDD - new.
		if (bitsDamageMod & DMG_BLEEDING)
		{
			if (gSkillData.tdmg_bleeding_duration >= 0) {
				if (!blockTimedDamageUpdates) {
					//SetSuitUpdate("!???", FALSE, SUIT_NEXT_IN_1MIN);
				}
			}
			bitsDamageMod &= ~DMG_BLEEDING;
			ffound = TRUE;
		}
	}

	//MODDD - if "timedDamageDisableViewPunch" is on, timed damage will not throw the view off for a moment.
	//... not this "!(bitsDamageType & DMG_TIMEBASED || bitsDamageTypeMod & DMG_TIMEBASEDMOD)", because that is for starting timed damage.
	//That is okay anyways.  The continued effects (DMG_TIMEDEFFECT) are what this CVar is concerned with.
	if(EASY_CVAR_GET_DEBUGONLY(disablePainPunchAutomatic) != 1){
		if(EASY_CVAR_GET_DEBUGONLY(timedDamageDisableViewPunch) == 0 || !(bitsDamageTypeMod & (DMG_TIMEDEFFECT | DMG_TIMEDEFFECTIGNORE) )   ){
			pev->punchangle.x = -2;
		}
	}

	//Don't print out these plain damage notices on taking damage during the post-revive-invincibility-delay.  We kinda already know...
	if(!(blockDamage || buddhaMode)){

		if (fTookDamage && !ftrivial && fmajor && flHealthPrev >= 75) 
		{
			// first time we take major damage...
			// turn automedic on if not on
			SetSuitUpdate("!HEV_MED1", FALSE, SUIT_NEXT_IN_30MIN, 5.2f);	// automedic on

			// give morphine shot if not given recently
			SetSuitUpdate("!HEV_HEAL7", FALSE, SUIT_NEXT_IN_30MIN, 4.5f);	// morphine shot
		}
		
		if (fTookDamage && !ftrivial && fcritical && flHealthPrev < 75)
		{

			// already took major damage, now it's critical...
			if (pev->health < 6)
				SetSuitUpdate("!HEV_HLTH3", FALSE, SUIT_NEXT_IN_10MIN, 4.2f);	// near death
			else if (pev->health < 20)
				SetSuitUpdate("!HEV_HLTH2", FALSE, SUIT_NEXT_IN_10MIN, 4.5f);	// health critical
	
			// give critical health warnings
			if (!RANDOM_LONG(0,3) && flHealthPrev < 50)
				SetSuitUpdate("!HEV_DMG7", FALSE, SUIT_NEXT_IN_5MIN, 4.5f); //seek medical attention
		}


		// if we're taking time based damage, warn about its continuing effects
		//MODDD - include new damage mask.
		//Wait... isn't that odd?   normally, timed damage is just sent as "generic" (besides the inital strike, in order to avoid
		//a feedback loop of permanent duration resetting).  That type has been changed to "DMG_TIMEDEFFECT" for differentiating for
		//the "timed damage ignores armor" cvar.   TEST WITHOUT THE "DMG_TIMEBASED" IN THERE BELOW.
		//if (fTookDamage && (bitsDamageType & DMG_TIMEBASED) && flHealthPrev < 75)
		//NOTICE:::Problem solved.  Apparently,  "DMG_GENERIC" WAS under the mask of "DMG_TIMEBASED".  So, it counted.
		//Must count "DMG_TIMEDEFFECT" under the new mask to match this.

		//easyPrintLine("RANDO STATUS %d %d %d    %d ", bitsDamageTypeMod, DMG_TIMEBASEDMOD, bitsDamageTypeMod & DMG_TIMEBASEDMOD,    bitsDamageType & DMG_TIMEBASED  );
	
		if(EASY_CVAR_GET_DEBUGONLY(printOutCommonTimables) == 1){
			easyPrintLine("RAD TIME: %d  BLEED TIME %d  POISON TIME %d ", m_rgbTimeBasedDamage[itbd_Radiation], m_rgbTimeBasedDamage[itbd_Bleeding], m_rgbTimeBasedDamage[itbd_Poison]);
		}


		//if (fTookDamage && (bitsDamageType & DMG_TIMEBASED || bitsDamageTypeMod & DMG_TIMEBASEDMOD || bitsDamageTypeMod ) && flHealthPrev < 75)
		if (fTookDamage && (bitsDamageTypeMod & (DMG_TIMEDEFFECT | DMG_TIMEDEFFECTIGNORE) ) && flHealthPrev < 75)
		{
			if (flHealthPrev < 50)
			{
				if (!RANDOM_LONG(0,3)){
					SetSuitUpdate("!HEV_DMG7", FALSE, SUIT_NEXT_IN_5MIN); //seek medical attention
				}
			}
			else{
				SetSuitUpdate("!HEV_HLTH1", FALSE, SUIT_NEXT_IN_10MIN);	// health dropping
			}
		}
	}//END OF (if NOT during post-invincibility-delay)


	// Made it this far?  No queued messages?  Can play this (generic, hev_damage)
	// Not for taking timed damage though.
	// !((bitsDamage & DMG_TIMEBASED) || (bitsDamageMod & DMG_TIMEBASEDMOD)) 
	if ( !(bitsDamage & DMG_TIMEDEFFECT) ) {
		// If all m_rgSuitPlayList members are 0, that might be ok too?  unsure if that is the case after all queued
		// suit sounds are over.    This should work though,
		if (m_flSuitUpdate == 0) {

			// Lost armor, or a recent attack did a lot of damage to armor?  Say something.
			if ((hadArmor && pev->armorvalue <= 0) || (flArmorDamage >= 30) ) {
				SetSuitUpdate("!HEV_E6", FALSE, 180);
			}

			if (m_flSuitUpdate == 0) {
				// Still available?  Generic damage message.
				SetSuitUpdate("!HEV_E4", FALSE, 150);
			}
		}
		else if (hadArmor && pev->armorvalue <= 0) {
			// had armor but lost it?  Definitely queue this.
			SetSuitUpdate("!HEV_E6", FALSE, 180);
		}
	}//END OF DMG_TIMEBASED and MOD.
	

	return fTookDamage;
}//END OF takeDamage


GENERATE_DEADTAKEDAMAGE_IMPLEMENTATION(CBasePlayer) {

	return GENERATE_DEADTAKEDAMAGE_PARENT_CALL(CBaseMonster);
}


//Parameters: integer named fGibSpawnsDecal
GENERATE_GIBMONSTER_IMPLEMENTATION(CBasePlayer){
	
	recentlyGibbed = TRUE;

	GENERATE_GIBMONSTER_PARENT_CALL(CBaseMonster);
}



//=========================================================
// PackDeadPlayerItems - call this when a player dies to
// pack up the appropriate weapons and ammo items, and to
// destroy anything that shouldn't be packed.
//
// This is pretty brute force :(
//=========================================================
void CBasePlayer::PackDeadPlayerItems( void )
{
	int iWeaponRules;
	int iAmmoRules;
	int i;
	CBasePlayerWeapon *rgpPackWeapons[ 20 ];// 20 hardcoded for now. How to determine exactly how many weapons we have?
	int iPackAmmo[ MAX_AMMO_TYPES + 1];
	int iPW = 0;// index into packweapons array
	int iPA = 0;// index into packammo array

	memset(rgpPackWeapons, NULL, sizeof(rgpPackWeapons) );
	memset(iPackAmmo, -1, sizeof(iPackAmmo) );

	// get the game rules 
	iWeaponRules = g_pGameRules->DeadPlayerWeapons( this );
 	iAmmoRules = g_pGameRules->DeadPlayerAmmo( this );

	
	if(!alreadyDroppedItemsAtDeath){
		//when scheduled to drop items, they stay on the player until respawn is called.  This script will
		//not know any better and will continue dropping duplicates of the same things the player was holding
		//at the time of death.
		//return;
		alreadyDroppedItemsAtDeath = TRUE;
	}else{
		return;
	}

	if ( iWeaponRules == GR_PLR_DROP_GUN_NO && iAmmoRules == GR_PLR_DROP_AMMO_NO )
	{
		// nothing to pack. Remove the weapons and return. Don't call create on the box!
		//MODDD - schedule for respawn instead of clearing items here.
		// 
		// And don't check for recoveryIndex just yet, that's not reliable at this point.
		// How about, at revive time, the player decides whether to do wipe weapons or not
		// (coming from a revive or not).  Dropping weapons will remove inventory-versions of the
		// weapons as they are dropped.  That way, anything not dropped will stay at revive.
		// Total respawns (no adrenaline involved) call for losing everything unconditionally though.
		//if (recoveryIndex == 3) {
			scheduleRemoveAllItems = TRUE;
			scheduleRemoveAllItemsIncludeSuit = TRUE;
			//RemoveAllItems( TRUE );
		//}
		return;
	}

// go through all of the weapons and make a list of the ones to pack
	for ( i = 0 ; i < MAX_ITEM_TYPES ; i++ )
	{
		if ( m_rgpPlayerItems[ i ] )
		{
			// there's a weapon here. Should I pack it?
			CBasePlayerItem *pPlayerItem = m_rgpPlayerItems[ i ];

			while ( pPlayerItem )
			{
				switch( iWeaponRules )
				{
				case GR_PLR_DROP_GUN_ACTIVE:
					if ( m_pActiveItem && pPlayerItem == m_pActiveItem )
					{
						// this is the active item. Pack it.
						rgpPackWeapons[ iPW++ ] = (CBasePlayerWeapon *)pPlayerItem;
					}
					break;

				case GR_PLR_DROP_GUN_ALL:
					rgpPackWeapons[ iPW++ ] = (CBasePlayerWeapon *)pPlayerItem;
					break;

				default:
					break;
				}

				pPlayerItem = pPlayerItem->m_pNext;
			}
		}
	}

// now go through ammo and make a list of which types to pack.
	if ( iAmmoRules != GR_PLR_DROP_AMMO_NO )
	{
		for ( i = 0 ; i < MAX_AMMO_TYPES ; i++ )
		{
			if ( m_rgAmmo[ i ] > 0 )
			{
				// player has some ammo of this type.
				switch ( iAmmoRules )
				{
				case GR_PLR_DROP_AMMO_ALL:
					iPackAmmo[ iPA++ ] = i;
					break;

				case GR_PLR_DROP_AMMO_ACTIVE:
					if ( m_pActiveItem && i == m_pActiveItem->PrimaryAmmoIndex() ) 
					{
						// this is the primary ammo type for the active weapon
						iPackAmmo[ iPA++ ] = i;
					}
					else if ( m_pActiveItem && i == m_pActiveItem->SecondaryAmmoIndex() ) 
					{
						// this is the secondary ammo type for the active weapon
						iPackAmmo[ iPA++ ] = i;
					}
					break;

				default:
					break;
				}
			}
		}
	}

// create a box to pack the stuff into.
	CWeaponBox *pWeaponBox = (CWeaponBox *)CBaseEntity::Create( "weaponbox", pev->origin, pev->angles, edict() );

	pWeaponBox->pev->angles.x = 0;// don't let weaponbox tilt.
	pWeaponBox->pev->angles.z = 0;

	pWeaponBox->SetThink( &CWeaponBox::Kill );
	pWeaponBox->pev->nextthink = gpGlobals->time + 120;

// back these two lists up to their first elements
	iPA = 0;
	iPW = 0;

// pack the ammo
	while ( iPackAmmo[ iPA ] != -1 )
	{
		pWeaponBox->PackAmmo( MAKE_STRING( CBasePlayerItem::AmmoInfoArray[ iPackAmmo[ iPA ] ].pszName ), m_rgAmmo[ iPackAmmo[ iPA ] ] );

		//MODDD - ditto, see below.
		m_rgAmmo[iPackAmmo[iPA]] = 0;

		iPA++;
	}

// now pack all of the items in the lists
	while ( rgpPackWeapons[ iPW ] )
	{
		// weapon unhooked from the player. Pack it into der box.
		pWeaponBox->PackWeapon( rgpPackWeapons[ iPW ] );

		//MODDD - remove this item from pev->weapons individually, in case we don't get to setting pev->weapons to 0.
		// Respawning through revives (adrenaline) does not wipe pev->weapons.
		// So this must be done as to not leave the HUD thinking we still have weapons we really don't.
		pev->weapons &= ~(1 << rgpPackWeapons[iPW]->m_iId);// take item off hud

		iPW++;
	}

	pWeaponBox->pev->velocity = pev->velocity * 1.2;// weaponbox has player's velocity, then some.


	scheduleRemoveAllItems = TRUE;
	scheduleRemoveAllItemsIncludeSuit = TRUE;
	//RemoveAllItems( TRUE );// now strip off everything that wasn't handled by the code above.
}


//MODDD - NEW.  No need to call from RemoveAllItems, doing that removes the need for several checks in here.
void CBasePlayer::RemoveAllAmmo(void) {
	int i;
	for (i = 0; i < MAX_AMMO_TYPES; i++) {
		m_rgAmmo[i] = 0;
	}
	TabulateAmmo();  //safety?

		
	// Go through all weapons.  Any exhaustible weapons (grenades, snarks, etc.) need to be deleted
	// on running out of ammo.
	// Satchel has an exception: do not destroy if there are still charges out.  It must be selectable
	// to get to the remote and set them off.
	for (i = 0; i < MAX_ITEM_TYPES; i++)
	{
		if (m_rgpPlayerItems[i])
		{
			CBasePlayerItem* pPlayerItem = m_rgpPlayerItems[i];

			while (pPlayerItem)
			{
				CBasePlayerWeapon* gun;

				gun = (CBasePlayerWeapon*)pPlayerItem->GetWeaponPtr();

				if (gun) {

					if (FClassnameIs(gun->pev, "weapon_satchel")) {
						CSatchel* tempSatch = static_cast<CSatchel*>(gun);
						// hacky!  Bundle this setting into CheckOutOfAmmo if this is ever used again for any other weap

						alreadySentSatchelOutOfAmmoNotice = FALSE;
						tempSatch->CheckOutOfAmmo();

						// Allow this to be unselectable (no charges out while out of ammo)?  Delete.
						if (alreadySentSatchelOutOfAmmoNotice) {
							//if (m_pActiveItem == gun)m_pActiveItem = NULL;
							//gun->Drop();
							pev->weapons &= ~(1 << gun->m_iId);// take item off hud
							gun->DestroyItem();
							//this->RemovePlayerItem(gun);
						}

					}
					else {
						if (gun->iFlags() & ITEM_FLAG_EXHAUSTIBLE) {
							// No need to blank m_pActiveItem, RemovePlayerItem (called by DestroyItem of a player item,
							// actually)  already blanks the current m_pActiveItem, sets its viewmodel to NULL, and 
							// handles other cleanup.
							//if (m_pActiveItem == gun)m_pActiveItem = NULL;
							//gun->Drop();
							pev->weapons &= ~(1 << gun->m_iId);// take item off hud
							gun->DestroyItem();
							//this->RemovePlayerItem(gun);
						}
					}


				}

				if (ITEM_FLAG_EXHAUSTIBLE) {

				}

				pPlayerItem = pPlayerItem->m_pNext;
			}//weapons in slot
		}// this slot has a weapon?
	}//loop thru slots



	/*
	// If doing the 'pick a better weapon' thing, GetNextBestWeapon would have
	// to support a NULL weapon a that 2nd parameter (the current weapon to compare
	// against, I think).

	if(m_pActiveItem == NULL){
		BOOL getNextSuccess = g_pGameRules->GetNextBestWeapon(this, pWeapon);

		// m_pActiveItem == NULL
		if (!getNextSuccess || pWeapon == m_pActiveItem) {
			//send a signal to clear the currently equipped weapon.
			MESSAGE_BEGIN(MSG_ONE, gmsgClearWeapon, NULL, pev);
			MESSAGE_END();
		}
	}
	*/


}//RemoveAllAmmo

void CBasePlayer::RemoveAllItems( BOOL removeSuit )
{
	int i;
	
	// Safe to reset since these settings were valid only for one call.
	scheduleRemoveAllItems = FALSE;
	scheduleRemoveAllItemsIncludeSuit = FALSE;

	if (m_pActiveItem)
	{
		ResetAutoaim( );
		m_pActiveItem->Holster( );
		m_pActiveItem = NULL;
	}

	//MODDD - and power canisters.
	for (i = 0; i < MAX_ITEMS; i++) {
		m_rgItems[i] = 0;
	}
	// why not.
	airTankAirTime = 0;
	longJumpCharge = 0;


	//MODDD - reset holstering too.
	m_pQueuedActiveItem = NULL;
	m_bHolstering = FALSE;



	m_pLastItem = NULL;

	CBasePlayerItem *pPendingItem;
	for (i = 0; i < MAX_ITEM_TYPES; i++)
	{
		m_pActiveItem = m_rgpPlayerItems[i];
		while (m_pActiveItem)
		{
			pPendingItem = m_pActiveItem->m_pNext; 
			m_pActiveItem->Drop( );
			m_pActiveItem = pPendingItem;
		}
		m_rgpPlayerItems[i] = NULL;
	}
	m_pActiveItem = NULL;

	pev->viewmodel		= 0;
	pev->weaponmodel	= 0;
	

	//MODDD - this will be done at respawn instead of at here.,
	if ( removeSuit ){
		pev->weapons = 0;
	}else{
		pev->weapons &= ~WEAPON_ALLWEAPONS;
	}

	for ( i = 0; i < MAX_AMMO_TYPES;i++)
		m_rgAmmo[i] = 0;


	//MODDD - probably so.
	this->hasGlockSilencer = FALSE;


	UpdateClientData();
	// send Selected Weapon Message to our client

	//MODDD NOTICE - this does not clear the equipped weapon, the client event notices there isn't even a weapon ID 0
	//               and ignore the request, leaving it set to the existing weapon with its clip remaining.
	//               Try gmsgClearWeapon instead.
	/*
	MESSAGE_BEGIN( MSG_ONE, gmsgCurWeapon, NULL, pev );
		WRITE_BYTE(0);
		WRITE_BYTE(0);
		WRITE_BYTE(0);
	MESSAGE_END();
	*/
	
	MESSAGE_BEGIN( MSG_ONE, gmsgClearWeapon, NULL, pev );
	MESSAGE_END();

}//END OF RemoveAllItems






//MODDD - override this for behavior to start a fadeout with germancensorship on
//since gibbing fails and leads to this intead.
//Again, note that the player has its own PlayerDeathThink method that must be
//set instead of some Fadeout think, so this PlayerDeathThink
//must handle fadeout on its own.
void CBasePlayer::FadeMonster(){

	//StopAnimation();
	
	//Don't interrupt the physics, the player still falls.
	

	pev->animtime = gpGlobals->time;
	pev->effects |= EF_NOINTERP;
}

/*
* GLOBALS ASSUMED SET:  g_ulModelIndexPlayer
*
* ENTITY_METHOD(PlayerDie)
*/
//entvars_t *g_pevLastInflictor;  // Set in combat.cpp.  Used to pass the damage inflictor for death messages.
								// Better solution:  Add as parameter to all Killed() functions.
//MODDD - the above comment is a little outdated. "g_pevLastInflictor" is never referred too, but used
//        to hold the last pevInflictor sent along with TakeDamage, as seen in Combat.cpp.
//        Now, the pevAttacker is sent to Killed methods only instead. Assuming only knowing the attacker
//        is necessary, but this could be edited to also receive the pevInflictor that takeDamage does too
//        more neatly.
GENERATE_KILLED_IMPLEMENTATION(CBasePlayer)
{
	// for now, don't let NPC's talk.  Gets set back to 0 on a revive
	g_TalkMonster_PlayerDead_DialogueMod = 1;
	// Any talkers following me at this moment?  Record some of them (if over 5 somehow, but otherwise all) for sifting through later.
	RecordFollowers();

	if(!IsMultiplayer()){
		BOOL canStartTruce = FALSE;

		if(EASY_CVAR_GET(playerDeadTruce) == 2){
			// start the truce only if any human faction sees the player at the time of death
			int i;
			edict_t* pEdict;
			CBaseEntity* pTempEntity;
			pEdict = g_engfuncs.pfnPEntityOfEntIndex(1);
			if (!pEdict)return;
			for (i = 1; i < gpGlobals->maxEntities; i++, pEdict++){
				if (pEdict->free)	// Not in use
					continue;
				if (!(pEdict->v.flags & (FL_MONSTER)))	// Not a monster ?
					continue;
				pTempEntity = CBaseEntity::Instance(pEdict);
				if (!pTempEntity)
					continue;
				if(!pTempEntity->IsAlive()){
					continue;
				}
				CBaseMonster* monsterTest = pTempEntity->GetMonsterPointer();
				if(monsterTest==NULL){
					continue;
				}
				int daClass = monsterTest->Classify();
				if(daClass == CLASS_HUMAN_MILITARY || daClass == CLASS_MACHINE || daClass == CLASS_PLAYER_ALLY || daClass == CLASS_HUMAN_PASSIVE){
					// go ahead
				}else{
					// nope!
					continue;
				}

				if(monsterTest->FVisible(this->pev->origin) && monsterTest->FInViewCone(this)){
					// can see me?  Start the truce.
					canStartTruce = TRUE;
					break;
				}

			}//END OF through all entities.

		}else if(EASY_CVAR_GET(playerDeadTruce) == 3){
			// Always start the truce now
			canStartTruce = TRUE;
		}

		if(canStartTruce){
			// IDEA: if this CVar is on (maybe only once there' a line of sight between anything and the player),
			// do the truce between hmilitary and playerally.
			// This is guaranteed on a reviveless death (figured out later), but for now, allow it only if within eyesight
			// of any hgrunts
			UTIL_SetDeadPlayerTruce(TRUE);
		}
	}// IsMultiplayer check


	//gee, I don't think you're doing this right now.
	m_bHolstering = FALSE;
	m_pQueuedActiveItem = NULL;
	m_fCustomHolsterWaitTime = -1;


	friendlyCheckTime = -1;

	//do a friendly check, all friendlies. Any that has this player as its enemy will stop playing the horror sound.
	CBaseEntity *pEntity = NULL;
	while((pEntity = UTIL_FindEntityByClassname( pEntity, "monster_friendly" )) != NULL){
		CFriendly* tempMrFriendly = static_cast<CFriendly*>(pEntity);

		//targeting this player?
		if(tempMrFriendly->m_hEnemy!=NULL && tempMrFriendly && tempMrFriendly->m_hEnemy == this){
			tempMrFriendly->horrorSelected = FALSE;
			tempMrFriendly->stopHorrorSound();
		}
	}//END OF while(friendly check)



	// Holster weapon immediately, to allow it to cleanup
	if ( m_pActiveItem )
		m_pActiveItem->Holster( );

	//MODDD - the pevInflictor is finally provided!
	//g_pGameRules->PlayerKilled( this, pevAttacker, g_pevLastInflictor );
	g_pGameRules->PlayerKilled( this, pevAttacker, pevInflictor );

	if ( m_pTank != NULL )
	{
		m_pTank->Use( this, this, USE_OFF, 0 );
		m_pTank = NULL;
	}

	CSound *pSound;
	// this client isn't going to be thinking for a while, so reset the sound until they respawn
	pSound = CSoundEnt::SoundPointerForIndex( CSoundEnt::ClientSoundIndex( edict() ) );
	{
		if ( pSound )
		{
			pSound->Reset();
		}
	}

	SetAnimation( PLAYER_DIE );
	
	m_iRespawnFrames = 0;

	pev->modelindex = g_ulModelIndexPlayer;    // don't use eyes


	//MODDD - movetype change logic was here


	stopSelfSounds();



	// send "health" update message to zero
	m_iClientHealth = 0;
	MESSAGE_BEGIN( MSG_ONE, gmsgHealth, NULL, pev );
		WRITE_BYTE( (int)m_iClientHealth );
	MESSAGE_END();


	//MODDD - no more holstering if in progress.
	this->m_bHolstering = FALSE;
	this->m_chargeReady &= ~128;

	
	// Tell Ammo Hud that the player is dead
	//MODDD - last two things are read as CHARS, so why not write as CHARs then
	MESSAGE_BEGIN( MSG_ONE, gmsgCurWeapon, NULL, pev );
		WRITE_BYTE(0);
		WRITE_CHAR(0XFF);
		WRITE_CHAR(0xFF);
	MESSAGE_END();
	
	//MODDD - this is more accurate?
	
	//MESSAGE_BEGIN( MSG_ONE, gmsgClearWeapon, NULL, pev );
	//MESSAGE_END();
	

	//MODDD - new way of telling the HUD that the player is dead.  The above is
	//a bit indirect of a way that is incompatible with the current approach.
	deadflagmem = pev->deadflag;
	MESSAGE_BEGIN( MSG_ONE, gmsgUpdatePlayerAlive, NULL, pev );
		WRITE_SHORT( IsAlive() );
	MESSAGE_END();


	// reset FOV
	pev->fov = m_iFOV = m_iClientFOV = 0;

	MESSAGE_BEGIN( MSG_ONE, gmsgSetFOV, NULL, pev );
		WRITE_BYTE(0);
	MESSAGE_END();


	
	BOOL gibbedThisFrame = FALSE; //only good this call.

	// UNDONE: Put this in, but add FFADE_PERMANENT and make fade time 8.8 instead of 4.12
	// UTIL_ScreenFade( edict(), Vector(128,0,0), 6, 15, 255, FFADE_OUT | FFADE_MODULATE );

	//MODDD - pev->health requirement changed from -40.  Seems a little hard to reach typically.
	if ( ( pev->health < -35 && iGib != GIB_NEVER ) || iGib == GIB_ALWAYS )
	{
		//pev->solid			= SOLID_NOT;   //but GibMonster already does this.
		//GibMonster();	// This clears pev->model
		GENERATE_GIBMONSTER_CALL;
		
		//let's leave the EF_NODRAW up to GibMonster.
		//pev->effects |= EF_NODRAW;

		gibbedThisFrame = TRUE;

		//return;   //GOT YOUUUU
	}
	//MODDD - added.
	else{
		//recentlyGibbed = FALSE;   Keep this on until Spawn() gets called again

	}



	//MODDD - as-is movetype-changing logic moved here to respond to being gibbed or not.
	// If gibbed, don't fall.  Why bother?  What is 'it' that's falling anyway?
	if (!recentlyGibbed) {
		pev->deadflag = DEAD_DYING;
		pev->movetype = MOVETYPE_TOSS;
		ClearBits(pev->flags, FL_ONGROUND);
		if (pev->velocity.z < 10)
			pev->velocity.z += RANDOM_FLOAT(0, 300);
	}
	else {
		pev->deadflag = DEAD_DEAD;
		pev->movetype = MOVETYPE_NONE;
		ClearBits(pev->flags, FL_ONGROUND);
		pev->velocity = g_vecZero;
	}



	// Does it look like we have a good shot at reviving with adrenaline?
	if (recentlyGibbed) {
		declareRevivelessDead();

		// clear out the suit message cache so we don't keep chattering
		// already done earlier!
		//SetSuitUpdate(NULL, FALSE, 0);

		// the gib call should've cleared the currently playing suit sound, so this one can play
		// uninterrupted.

		m_flSuitUpdate = gpGlobals->time;  //say the next line now!
		// "critical failure"
		SetSuitUpdate("!HEV_E2", FALSE, SUIT_REPEAT_OK, 4.2f);
	}
	else if (recentMajorTriggerDamage & 1) {
		// gonna die.
		declareRevivelessDead();

		m_flSuitUpdate = gpGlobals->time;  //say the next line now!
		SetSuitUpdate("!HEV_E3", FALSE, SUIT_REPEAT_OK, 4.2f);
	}
	else if (gpGlobals->time <= recentRevivedTime + 10) {
		// If it's been too soon since the previous revive, report failure.

		declareRevivelessDead();
		//m_flSuitUpdate = gpGlobals->time;  //say the next line now!
		//SetSuitUpdate("!HEV_E3", FALSE, SUIT_REPEAT_OK, 4.2f);
		DeathSound(FALSE);
	}
	else if (playerHasSuit() && m_rgItems[ITEM_ADRENALINE] > 0) {
		// For now, if the player has adrenaline and hasn't been gibbed, tell "DeathSound" this.
		if(EASY_CVAR_GET_DEBUGONLY(batteryDrainsAtAdrenalineMode) == 1){
			SetAndUpdateBattery(0);
		}
		if(EASY_CVAR_GET_DEBUGONLY(timedDamageReviveRemoveMode) == 1){
			attemptResetTimedDamage(TRUE);
		}

		//if (gpGlobals->time > lastBlockDamageAttemptReceived + 1.5) {
		//	// good chance we can revive.
			DeathSound(TRUE);
		//}
		//else {
		//	// not high hopes.
		//	DeathSound(FALSE);
		//}

		// get started!
		recoveryIndex = 0;

		recoveryDelay = gpGlobals->time + 6;
		recoveryDelayMin = gpGlobals->time + 0.4f;

	}else{
		// gonna die.
		declareRevivelessDead();
		DeathSound(FALSE);
	}
	

	//MODDD - This is supposed to be skipped, as per original script (was here, and is skipped if the player was gibbed by a now defunct "return").
	if(gibbedThisFrame){
		pev->angles.x = 0;
		pev->angles.z = 0;

		SetThink(&CBasePlayer::PlayerDeathThink);
		pev->nextthink = gpGlobals->time + 0.1;
	}

	//MODDD - set this.  Does not get seen if reviving by adrenaline.
	minimumRespawnDelay = gpGlobals->time + EASY_CVAR_GET_DEBUGONLY(minimumRespawnDelay);
	
}


void CBasePlayer::onDelete(void) {
	// shouldn't we force any suit sounds to stop playing in such a case too?
	EMIT_SOUND(ENT(pev), CHAN_STATIC, "common/null.wav", 1, ATTN_NORM);
	EMIT_SOUND(ENT(pev), CHAN_VOICE, "common/null.wav", 1, ATTN_NORM);
	

	// wait.  player deleted.
	// what.
}


// commonly used script to silence any sounds coming from the player, should work usually.
// Includes playing the most recently played FVOX line with the "SND_STOP" flag.  Unsure why
// sometimes that is necessary and playing "null.wav" in all the usual channels isn't enough.
void CBasePlayer::stopSelfSounds(void) {
	// clear out the suit message cache so we don't keep chattering
	SetSuitUpdate(NULL, FALSE, 0);

	// In fact, kill all sounds coming from the player / FVOX.
	//////////////////////////////////////////////////////////////////////////////////////
	// I think the base GibMonster already handles CHAN_VOICE.  Although we might not be calling gibmonster.
	// Can't hurt.

	//EMIT_SOUND(ENT(pev), CHAN_VOICE, "common/null.wav", 1, ATTN_NORM);
	//// shouldn't we force any suit sounds to stop playing in such a case too?
	//EMIT_SOUND_DYN(ENT(pev), CHAN_STATIC, "common/null.wav", 1, ATTN_NORM, 0, 100);

	EMIT_SOUND(ENT(pev), CHAN_VOICE, "common/null.wav", 1.0, ATTN_IDLE);
	EMIT_SOUND(ENT(pev), CHAN_ITEM, "common/null.wav", 1.0, ATTN_IDLE);
	EMIT_SOUND(ENT(pev), CHAN_STREAM, "common/null.wav", 1.0, ATTN_IDLE);

	// seems this usually works.
	if (recentlyPlayedSound[0] != '\0') {
		//EMIT_SOUND_DYN(ENT(pev), CHAN_STATIC, recentlyPlayedSound, 1, ATTN_NORM, SND_STOP, 100);
		STOP_SOUND_SUIT(ENT(pev), recentlyPlayedSound);
		recentlyPlayedSound[0] = '\0';  //cleared.
	}

	//UTIL_StopSound(tempplayer->edict(), CHAN_STATIC, "");
	//////////////////////////////////////////////////////////////////////////////////////

}//END OF stopSelfSounds


// position to shoot at
Vector CBasePlayer::BodyTarget( const Vector &posSrc ){
	return Center( ) + pev->view_ofs * RANDOM_FLOAT( 0.5, 1.1 );
};		

Vector CBasePlayer::BodyTargetMod( const Vector &posSrc ) {
	/*
	Vector org = Center( ) + pev->view_ofs;
	if ( pev->flags & FL_DUCKING )
	{
	org = org + ( VEC_HULL_MIN - VEC_DUCK_HULL_MIN );
	}
	*/

	if ( !(pev->flags & FL_DUCKING) ){
		return Center() + pev->view_ofs;
	}else{
		/*
		if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(testVar) == 1){
		return pev->origin + Vector(0, 0, VEC_DUCK_HULL_MAX.z);
		}else{
		//old way
		return Center() + pev->view_ofs;
		}
		*/
		return pev->origin + Vector(0, 0, VEC_DUCK_HULL_MAX.z - 6);
	}
};// position to shoot at









// Set the activity based on an event or current state
void CBasePlayer::SetAnimation( PLAYER_ANIM playerAnim )
{
	int animDesired;
	float speed;
	char szAnim[64];

	speed = pev->velocity.Length2D();

	if (pev->flags & FL_FROZEN)
	{
		speed = 0;
		playerAnim = PLAYER_IDLE;
	}

	switch (playerAnim) 
	{
	case PLAYER_JUMP:
		m_IdealActivity = ACT_HOP;
		break;
	
	case PLAYER_SUPERJUMP:
		m_IdealActivity = ACT_LEAP;
		break;
	
	case PLAYER_DIE:
		m_IdealActivity = ACT_DIESIMPLE;
		m_IdealActivity = GetDeathActivity( );
		break;

	case PLAYER_ATTACK1:	
		switch( m_Activity )
		{
		case ACT_HOVER:
		case ACT_SWIM:
		case ACT_HOP:
		case ACT_LEAP:
		case ACT_DIESIMPLE:
			m_IdealActivity = m_Activity;
			break;
		default:
			m_IdealActivity = ACT_RANGE_ATTACK1;
			break;
		}
		break;
	case PLAYER_IDLE:
	case PLAYER_WALK:
		if ( !FBitSet( pev->flags, FL_ONGROUND ) && (m_Activity == ACT_HOP || m_Activity == ACT_LEAP) )	// Still jumping
		{
			m_IdealActivity = m_Activity;
		}
		else if ( pev->waterlevel > 1 )
		{
			if ( speed == 0 )
				m_IdealActivity = ACT_HOVER;
			else
				m_IdealActivity = ACT_SWIM;
		}
		else
		{
			m_IdealActivity = ACT_WALK;
		}
		break;
	}

	switch (m_IdealActivity)
	{
	case ACT_HOVER:
	case ACT_LEAP:
	case ACT_SWIM:
	case ACT_HOP:
	case ACT_DIESIMPLE:
	default:
		if ( m_Activity == m_IdealActivity)
			return;
		m_Activity = m_IdealActivity;

		animDesired = LookupActivity( m_Activity );
		// Already using the desired animation?
		if (pev->sequence == animDesired)
			return;

		pev->gaitsequence = 0;
		pev->sequence		= animDesired;
		pev->frame			= 0;
		ResetSequenceInfo( );
		return;

	case ACT_RANGE_ATTACK1:
		if ( FBitSet( pev->flags, FL_DUCKING ) )	// crouching
			strcpy( szAnim, "crouch_shoot_" );
		else
			strcpy( szAnim, "ref_shoot_" );
		strcat( szAnim, m_szAnimExtention );
		animDesired = LookupSequence( szAnim );
		if (animDesired == -1)
			animDesired = 0;

		if ( pev->sequence != animDesired || !m_fSequenceLoops )
		{
			pev->frame = 0;
		}

		if (!m_fSequenceLoops)
		{
			pev->effects |= EF_NOINTERP;
		}

		m_Activity = m_IdealActivity;

		pev->sequence		= animDesired;
		ResetSequenceInfo( );
		break;

	case ACT_WALK:
		if (m_Activity != ACT_RANGE_ATTACK1 || m_fSequenceFinished)
		{
			if ( FBitSet( pev->flags, FL_DUCKING ) )	// crouching
				strcpy( szAnim, "crouch_aim_" );
			else
				strcpy( szAnim, "ref_aim_" );
			strcat( szAnim, m_szAnimExtention );
			animDesired = LookupSequence( szAnim );
			if (animDesired == -1)
				animDesired = 0;
			m_Activity = ACT_WALK;
		}
		else
		{
			animDesired = pev->sequence;
		}
	}

	if ( FBitSet( pev->flags, FL_DUCKING ) )
	{
		if ( speed == 0)
		{
			pev->gaitsequence	= LookupActivity( ACT_CROUCHIDLE );
			// pev->gaitsequence	= LookupActivity( ACT_CROUCH );
		}
		else
		{
			pev->gaitsequence	= LookupActivity( ACT_CROUCH );
		}
	}
	else if ( speed > 220 )
	{
		pev->gaitsequence	= LookupActivity( ACT_RUN );
	}
	else if (speed > 0)
	{
		pev->gaitsequence	= LookupActivity( ACT_WALK );
	}
	else
	{
		// pev->gaitsequence	= LookupActivity( ACT_WALK );
		pev->gaitsequence	= LookupSequence( "deep_idle" );
	}


	// Already using the desired animation?
	if (pev->sequence == animDesired)
		return;

	//ALERT( at_console, "Set animation to %d\n", animDesired );
	// Reset to first frame of desired animation
	pev->sequence		= animDesired;
	pev->frame			= 0;
	ResetSequenceInfo( );

	//NOTICE - hard to work wtih, doing this in hl_wpn_glock.cpp instead (communicate that the silencer is on to the third person model).
	//pev->iuser1 = 4000;
	//pev->effects = 4;
}

/*
===========
TabulateAmmo
This function is used to find and store 
all the ammo we have into the ammo vars.
============
*/


//MODDD - supports the cached ammotype indeces.
// These "ammo_" counts can probably be phased out at some point, several weapons don't even use them
// (gauss makes no mention of ammo_uranium, there isn't even a count for a retail weapon: snarks/squeak).
// Ammo counts through the array of ammo's on the player (m_rgAmmo) are kept in synch with clientside
// anyway, which is really what these are tied to.
// So these counts look to have no advantage over say, PlayerPrimaryAmmoCount() for a given weapon.
void CBasePlayer::TabulateAmmo()
{
	ammo_9mm = AmmoInventory( AmmoIndex_9mm );
	ammo_357 = AmmoInventory(AmmoIndex_357 );
	ammo_argrens = AmmoInventory(AmmoIndex_ARgrenades );
	ammo_bolts = AmmoInventory(AmmoIndex_bolts );
	ammo_buckshot = AmmoInventory(AmmoIndex_buckshot );
	ammo_rockets = AmmoInventory(AmmoIndex_rockets );
	ammo_uranium = AmmoInventory(AmmoIndex_uranium );
	ammo_hornets = AmmoInventory(AmmoIndex_Hornets );
}


void CBasePlayer::set_fvoxEnabled(BOOL argNew, BOOL setSilent) {
	fvoxEnabled = argNew;

	if (!setSilent) {
		// let the player know the FVOX value has changed, but not if it's the first
		// call joining a server.
		if (fvoxEnabled == 1) {
			//just turned it on.
			SetSuitUpdateFVoxException("!HEV_V0", FALSE, SUIT_REPEAT_OK);
		}
		else {
			//just turned it off.  Clear other queud messages.
			SetSuitUpdate(NULL, FALSE, 0);
			SetSuitUpdateFVoxException("!HEV_V1", FALSE, SUIT_REPEAT_OK);
		}
	}
}
void CBasePlayer::set_cl_ladder_choice(float argNew) {
	cl_ladder_choice = argNew;

	int filter = cl_ladder_choice;
	if (filter < 0) {
		filter = 0;
	}
	if (filter > 2) {
		filter = 2;
	}

	// compatiblestring is just going to be cl_ladder_choice converted to a string (required by the physics key as a value).
	char compatiblestring[2];
	sprintf(compatiblestring, "%d", filter);
	compatiblestring[1] = '\0';

	g_engfuncs.pfnSetPhysicsKeyValue(edict(), "plm", compatiblestring);
}





//MODDD - player constructor.
CBasePlayer::CBasePlayer(void){
	int i;


	// turning this into a method for how much is otherwise just duplicated at this point.
	// We want a lot of the exact same things for CBasePlayer creation and resetting between map transitions.
	_commonReset();

	//for (i = 0; i < CHUMTOAD_SKIN_MEM_MAX; i++) {
	//	chumToadSkinMem[i] = 0;
	//}


	// This will be changed soon after the player spawns
	fvoxEnabled = 0;
	// same.
	fHolsterAnimsEnabled = 0;
	fBreakHolster = 0;
	cl_ladder_choice = 0;



	m_pLastItem = NULL;
	m_pActiveItem = NULL;
	m_pQueuedActiveItem = NULL;

	queueFirstAppearanceMessageSend = FALSE;
	
	iWasFrozenToday = FALSE;
	m_fLongJumpMemory = FALSE;

	//oldWaterMoveTime = -1;
	
	m_bHolstering = FALSE;
	m_pQueuedActiveItem = NULL;
	m_fCustomHolsterWaitTime = -1;
	superDuperDelay = -2;
	friendlyCheckTime = -1;
	closestFriendlyMemEHANDLE = NULL;
	closestFriendlyMem = NULL;
	horrorPlayTimePreDelay = -1;
	horrorPlayTime = -1;


	//m_flStartCharge = -1;  //okay?

	reviveSafetyTime = -1;
	grabbedByBarnacle = FALSE;
	grabbedByBarnacleMem = FALSE;

	recentlyGrantedGlockSilencer = FALSE;
	recentlySaidBattery = -1;  //do not save, meant to relate to what was recently said in-game yet.

	nextMadEffect = -1;

	
	alreadyDroppedItemsAtDeath = FALSE;
	sentCarcassScent = FALSE;

	hasGlockSilencer = FALSE;

	antidoteQueued = FALSE;
	radiationQueued = FALSE;

	for(i = 0; i < CSUITPLAYLIST; i++){
		m_rgSuitPlayListEvent[i] = NULL;
		m_rgSuitPlayListEventDelay[i] = -1;
		m_rgSuitPlayListFVoxCutoff[i] = -1;
	}

	currentSuitSoundEventTime = -1;
	currentSuitSoundEvent = NULL;
	currentSuitSoundFVoxCutoff = -1;

	sentenceFVoxCutoffStop = -1;

	rawDamageSustained = 0;

	// lazy defaults, get from the client soon at startup.
	default_fov = 90;
	auto_adjust_fov = 0;
	auto_determined_fov = 90;

	recentlyPlayedSound[0] = '\0';

	alreadySentSatchelOutOfAmmoNotice = FALSE;
	deadStage = 0;
	nextDeadStageTime = -1;

	m_framesSinceRestore = 0;

}//END OF CBasePlayer constructor

/*
===========
WaterMove
============
*/
void CBasePlayer::WaterMove()
{
	int air;

	if (pev->movetype == MOVETYPE_NOCLIP)
		return;

	if (pev->health < 0)
		return;

	// waterlevel 0 - not in water
	// waterlevel 1 - feet in water
	// waterlevel 2 - waist in water
	// waterlevel 3 - head in water

	//MODDD - all new to keep track of how much time passed between this call and the last one.
	float timeDelta;

	if(oldWaterMoveTime != -1){
		timeDelta = gpGlobals->time - oldWaterMoveTime;
	}else{
		timeDelta = 0;
	}
	//timeDelta = gpGlobals->frametime;

	oldWaterMoveTime = gpGlobals->time;

	//easyPrint("timedelta: %.12f\n", timeDelta);
	//easyPrint("delta: %.12f\n", oldWaterMoveTime);


	//MODDD
	//If not in the water, OR is in the water, but has air left in the air tank... (also requires at least one battery charge, if that cvar is on)
	if (pev->waterlevel != 3 || (pev->waterlevel == 3 && airTankAirTime > 0 && (EASY_CVAR_GET_DEBUGONLY(itemBatteryPrerequisite) == 0 || pev->armorvalue > 0 ) ) )
	{
		//MODDD
		//If underwater, use some air (implied we have some from the above if-statement's second condition)
		if(pev->waterlevel == 3){
			//easyPrint(" AIR UPDATE %.3f\n", airTankAirTime);
			//easyPrint(" AIR UPDATE DELTA %.3f\n", timeDelta);

			airTankAirTime -= timeDelta;
			if(airTankAirTime < 0){
				airTankAirTime = 0;
			}else{

				//if using the air tank for the first time since entering the water (immediately),
				//play the air sound.

				//MODDD - if going 
				if(airTankWaitingStart == TRUE){
					//pPlayer->edict() ???
					UTIL_PlaySound( ENT(pev), CHAN_ITEM, "items/airtank1.wav", 1, ATTN_NORM, TRUE );
					airTankWaitingStart = FALSE;
				}
			}
			airTankAirTimeNeedsUpdate = TRUE;
		}else{
			//If not underwater, the air-trigger sound is ready to play.
			airTankWaitingStart = TRUE;
		}
		//This way, under
		
		// not underwater
		// play 'up for air' sound
		if (pev->air_finished < gpGlobals->time)
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/pl_wade1.wav", 1, ATTN_NORM);
		else if (pev->air_finished < gpGlobals->time + 9)
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/pl_wade2.wav", 1, ATTN_NORM);

		pev->air_finished = gpGlobals->time + PLAYER_AIRTIME;
		pev->dmg = 2;

		// if we took drowning damage, give it back slowly
		if (m_idrowndmg > m_idrownrestored)
		{
			// set drowning damage bit.  hack - dmg_drownrecover actually
			// makes the time based damage code 'give back' health over time.
			// make sure counter is cleared so we start count correctly.
			
			// NOTE: this actually causes the count to continue restarting
			// until all drowning damage is healed.

			m_bitsDamageType |= DMG_DROWNRECOVER;
			m_bitsDamageType &= ~DMG_DROWN;
			m_rgbTimeBasedDamage[itbd_DrownRecover] = 0;
		}
		drowning = FALSE;
	}
	else
	{	
		//No air and underwater.
		
		//MODDD - getting an airtank will re-trigger the air sound.
		airTankWaitingStart = TRUE;

		// fully under water
		// stop restoring damage while underwater
		m_bitsDamageType &= ~DMG_DROWNRECOVER;
		m_rgbTimeBasedDamage[itbd_DrownRecover] = 0;


		if (pev->air_finished < gpGlobals->time)		// drown!
		{
			//MODDD
			drowning = TRUE;

			if (pev->pain_finished < gpGlobals->time)
			{
				// take drowning damage
				pev->dmg += 1;
				if (pev->dmg > 5)
					pev->dmg = 5;
				TakeDamage(VARS(eoNullEntity), VARS(eoNullEntity), pev->dmg, DMG_DROWN);
				pev->pain_finished = gpGlobals->time + 1;
				
				// track drowning damage, give it back when
				// player finally takes a breath

				m_idrowndmg += pev->dmg;
			} 
		}
		else
		{
			m_bitsDamageType &= ~DMG_DROWN;
			//MODDD
			drowning = FALSE;
		}
	}


	if (!pev->waterlevel)
	{
		if (FBitSet(pev->flags, FL_INWATER))
		{       
			ClearBits(pev->flags, FL_INWATER);
		}
		return;
	}
	
	// make bubbles

	air = (int)(pev->air_finished - gpGlobals->time);
	if (!RANDOM_LONG(0,0x1f) && RANDOM_LONG(0,PLAYER_AIRTIME-1) >= air)
	{
		switch (RANDOM_LONG(0,3))
			{
			case 0:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_swim1.wav", 0.8, ATTN_NORM); break;
			case 1:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_swim2.wav", 0.8, ATTN_NORM); break;
			case 2:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_swim3.wav", 0.8, ATTN_NORM); break;
			case 3:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_swim4.wav", 0.8, ATTN_NORM); break;
		}
	}

	if (pev->watertype == CONTENTS_LAVA)		// do damage
	{
		if (pev->dmgtime < gpGlobals->time)
			TakeDamage(VARS(eoNullEntity), VARS(eoNullEntity), 10 * pev->waterlevel, DMG_BURN);
	}
	else if (pev->watertype == CONTENTS_SLIME)		// do damage
	{
		pev->dmgtime = gpGlobals->time + 1;
		TakeDamage(VARS(eoNullEntity), VARS(eoNullEntity), 4 * pev->waterlevel, DMG_ACID);
	}
	
	if (!FBitSet(pev->flags, FL_INWATER))
	{
		SetBits(pev->flags, FL_INWATER);
		pev->dmgtime = 0;
	}
}

// TRUE if the player is attached to a ladder
BOOL CBasePlayer::IsOnLadder( void )
{ 
	return ( pev->movetype == MOVETYPE_FLY );
}


//PlayerDeathThink is called in the middle of the Player's PreThink method if not gibbed.
//If gibbed, this method is set to the entire new Think (none of PreThink gets  called)
void CBasePlayer::PlayerDeathThink(void)
{
	float flForward;


	if (FBitSet(pev->flags, FL_ONGROUND))
	{
		flForward = pev->velocity.Length() - 20;
		if (flForward <= 0) {
			pev->velocity = g_vecZero;
		}
		else {
			pev->velocity = flForward * pev->velocity.Normalize();
		}
	}

	if ( HasWeapons() )
	{
		// we drop the guns here because weapons that have an area effect and can kill their user
		// will sometimes crash coming back from CBasePlayer::Killed() if they kill their owner because the
		// player class sometimes is freed. It's safer to manipulate the weapons once we know
		// we aren't calling into any of their code anymore through the player pointer.
		PackDeadPlayerItems();
	}

	//MODDD NOTE - This area blocks turning the DEAD_DEAD flag on until the player death anim has finished or taken too long.
	if (pev->modelindex && (!m_fSequenceFinished) && (pev->deadflag == DEAD_DYING))
	{
		StudioFrameAdvance_SIMPLE( );

		m_iRespawnFrames++;				// Note, these aren't necessarily real "frames", so behavior is dependent on # of client movement commands
		if ( m_iRespawnFrames < 120 )   // Animations should be no longer than this
			return;
	}
	
	
	// once we're done animating our death and we're on the ground, we want to set movetype to None so our dead body won't do collisions and stuff anymore
	// this prevents a bug where the dead body would go to a player's head if he walked over it while the dead player was clicking their button to respawn
	if (pev->movetype != MOVETYPE_NONE && FBitSet(pev->flags, FL_ONGROUND)) {
		pev->movetype = MOVETYPE_NONE;
	}


	if (pev->deadflag == DEAD_DYING) {
		pev->deadflag = DEAD_DEAD;
	}
	
	StopAnimation();

	pev->effects |= EF_NOINTERP;
	pev->framerate = 0.0;


	
	//recoveryIndex:
	//-1: not out of health or in the dead / incapacitated state.
	//    Changed from this immediately on the player's Killed call!
	// 0: in incapacitated state, planned to revive, waiting for delay start
	//    (when on ground, or after 6 seconds pass, just start anyways).
	//    Can be interrupted by being stuck in map geometry (like blocking a func_rotating).
	//    This skips to 3 (dead), forbids revive.
	//    Requires being on the ground or waiting for 6 seconds without map entanglement.
	// 1: done waiting, beings playing the antidote FVOX line which gives the antidote.
	// 2: antidote injected, waiting to respawn the player.
	// -----------------------------------------------------
	// 3: super dead; can NOT revive.  Respawn or let the player click-out to load a save.




	//MODDD - adrenaline script "injection" - no pun intended.
	

	if(recoveryIndex == 0){
		// by the way, "pev->model != 0" is a gib-check.  The model being null (== 0) means, gibbed.  No "body" to recover.
		// nah, just use "recentlyGibbed", new var.
		// search "iGib" for more details on gibbing here (and in basemonster.h).

		float fallSpeedToleranceMulti = 1;
		float fallDamageReduction = 1;
		if (jumpForceMultiMem > 1) {
			//a jump force multiple above 1 will increase the tolerance for falls.
			fallSpeedToleranceMulti = sqrt(jumpForceMultiMem);
			fallDamageReduction = jumpForceMultiMem;
		}
		// Hit the ground while waiting for a revive?  Did we hit the ground too hard?
		if ((pev->flags & FL_ONGROUND) && m_flFallVelocity > PLAYER_MAX_SAFE_FALL_SPEED * fallSpeedToleranceMulti) {
			// nope, ded then.   No need for authentic gibbing but just count this as non-revivable.
			//recentlyGibbed = TRUE;
			pev->movetype = MOVETYPE_NONE;
			ClearBits(pev->flags, FL_ONGROUND);
			pev->velocity = g_vecZero;
			// no need to set pev->deadflag to DEAD_DEAD, that would've been handled by the death animation finishing above.  I think.

			UTIL_PlaySound(ENT(pev), CHAN_ITEM, "common/bodysplat.wav", 1, ATTN_NORM, 0, 100, FALSE);

			m_flFallVelocity = 0;  //I think this is safe?  No need to handle this again
			m_flSuitUpdate = gpGlobals->time;
			SetSuitUpdate("!HEV_E3", FALSE, SUIT_REPEAT_OK, 4.2f);

			declareRevivelessDead();
		}// END OF fall velocity check


		if (recoveryIndex == 0) {
			// only bother with any of this, if not dead from fall impact.
			// And has a suit, not sitting in a insta-death trigger, and has adrenaline.
			if (playerHasSuit() && !(recentMajorTriggerDamage & 2) && this->m_rgItems[ITEM_ADRENALINE] > 0) {

				// note that, if the player is not on the ground BUT otherwise meets conditions to recover,
				// the respawn will still be stalled until the player hits the ground (where the timer starts and
				// the player revives).
				// Even if on the ground, a minimum time (recoveryDelayMin) must have passed to start, including midair time, usually small.

				if (pev->flags & FL_ONGROUND){
					if (gpGlobals->time >= recoveryDelayMin && gpGlobals->time > lastBlockDamageAttemptReceived + 1.5) {
						startRevive();
					}
				}

				if (recoveryIndex == 0) {
					// Only check for waiting too long to revive / interrupting from map-inflicted damage if
					// we're not already trying to revive.
					if(
						(gpGlobals->time >= recoveryDelay - 3 && !(gpGlobals->time > lastBlockDamageAttemptReceived + 1.5))
					) {
						// 3 seconds away from the max recovery delay, and still recent map-inflicted damage?
						// Give up.
						m_flSuitUpdate = gpGlobals->time;
						SetSuitUpdate("!HEV_E2", FALSE, SUIT_REPEAT_OK, 4.2f);

						declareRevivelessDead();
					}
					else if (gpGlobals->time >= recoveryDelay) {
						// Took too long?  Make a decision now.  Reviving as being blocked by map damage should've happened sooner if it would've canceled this.
						// ALTHOUGH, if still falling and too fast, forbid it. ||
						if ( !(m_flFallVelocity > PLAYER_MAX_SAFE_FALL_SPEED* fallSpeedToleranceMulti) ) {
							startRevive();
						}
						else {
							// falling too fast, call it dead.
							EMIT_GROUPNAME_SUIT(ENT(pev), "HEV_DEAD");
							declareRevivelessDead();
						}
						
					}
				}//END OF recoveryIndex == 0 check... again

			}
			else {
				// Some condition went bad during this time?
				// Can not recover.  Let ordinary respawning handle this situation.

				// WAIT, don't force this one instant, maybe this is coming after a 'successful' revive sound
				// that turned out to fail.  Playing at the same time as it is just... weird.
				//m_flSuitUpdate = gpGlobals->time;
				
				SetSuitUpdate("!HEV_E3", FALSE, SUIT_REPEAT_OK, 4.2f);
				declareRevivelessDead();
			}
		}//END OF recoveryIndex == 0 check... again.
	}//END OF huge-ass recoveryIndex == 0 check


	if(recoveryIndex == 2 && gpGlobals->time >= recoveryDelay){
		//recover!
		recoveryIndex = -1;
		recoveryDelay = -1;
		recoveryDelayMin = -1;
		GetClassPtr( (CBasePlayer *)pev)->Spawn( TRUE );

		//invincibility delay?
		if(EASY_CVAR_GET_DEBUGONLY(playerReviveInvincibilityTime) > 0){
			reviveSafetyTime = gpGlobals->time + EASY_CVAR_GET_DEBUGONLY(playerReviveInvincibilityTime);
		}
		return;
	}

	// ordinary respawn can not handle the situation if the player may revive.
	if(recoveryIndex != 3){
		return;
	}
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//...if execution wasn't stopped by the above check, the player is dead with no chance of revival.
	


	// NOTE - happens instantly for deadstage 0 since nextDeadStageTime stays at 0 (time has always surpassed it)
	if(EASY_CVAR_GET(playerDeadTalkerBehavior) != 0){
		if(deadStage < 9 && gpGlobals->time >= nextDeadStageTime){
			//MODDD - NEW.  Enforce taking away the solid-ness and let takedamage by DAMAGE_NO, why not.
			// This avoids difficulties with stukabats landing that multiplayer doesn't have to deal with
			// from the 'StartDeathCam' call further down.
			pev->solid = SOLID_NOT;
			pev->takedamage = DAMAGE_NO;

			HandleDeadStage();
		}//deadStage
	}




	//MODDD - if landed on the ground, send the same scent the AI does at death to alert scavengers (eaters) of this.
	if(FBitSet(pev->flags, FL_ONGROUND) && pev->velocity.Length() == 0 && !sentCarcassScent){
		sentCarcassScent = TRUE;
		CSoundEnt::InsertSound ( bits_SOUND_CARCASS, pev->origin, 384, 30 );

		/*
		// pev->model == 0; ?
		//MODDD - going to use this space to do a check for any nearby barnies/scientists to notice this
		// WAIT.  Neverind, better to have script for them sift through the linked list of visible entities
		// to see if any happen to include a dead player and act on that or not, so one running around a corner
		// from a distance can react.
		if(pev->modelindex != 0){

		}//pev->modelindex
		*/

	}//ONGROUND and !sentCarcassScent checks


	//Since this think method overrides fade logic in case of german censorship, can do it here.

	//Assume we're fading out:
	if(recentlyGibbed && !(pev->effects & EF_NODRAW) ){
		//If German Censorship is on, we've been gibbed, and we're still drawn, 
		//do a fade for this frame.

		if (pev->rendermode == kRenderNormal){
			pev->renderamt = 255;
			pev->rendermode = kRenderTransTexture;
		}else{
			
			//pev->renderamt = max(pev->renderamt - gpGlobals->frametime * EASY_CVAR_GET_DEBUGONLY(playerFadeOutRate) , 0) ;
			pev->renderamt -= gpGlobals->frametime * EASY_CVAR_GET_DEBUGONLY(playerFadeOutRate);

			if(pev->renderamt <= 0){
				//stop the transparency, just go straight to normally opaque but invisible with EF_NODRAW as expected.
				pev->rendermode = ::kRenderNormal;
				pev->renderamt = 0;
				pev->effects |= EF_NODRAW;
			}

		}

	}//END OF fade check
	else{
		//Should be invisible? can force it to be safe.
		//pev->effects |= EF_NODRAW;
	}




	BOOL fAnyButtonDown = (pev->button & ~IN_SCORE );
	
	// wait for all buttons released
	if (pev->deadflag == DEAD_DEAD)
	{
		if (fAnyButtonDown)
			return;

		if ( g_pGameRules->FPlayerCanRespawn( this ) )
		{
			m_fDeadTime = gpGlobals->time;
			pev->deadflag = DEAD_RESPAWNABLE;
		}
		
		return;
	}


// if the player has been dead for one second longer than allowed by forcerespawn, 
// forcerespawn isn't on. Send the player off to an intermission camera until they 
// choose to respawn.
	if ( IsMultiplayer() && ( gpGlobals->time > (m_fDeadTime + 6) ) && !(m_afPhysicsFlags & PFLAG_OBSERVER) )
	{
		// go to dead camera. 
		StartDeathCam();
	}

// wait for any button down,  or mp_forcerespawn is set and the respawn time is up
	if (!fAnyButtonDown 
		&& !( IsMultiplayer() && forcerespawn.value > 0 && (gpGlobals->time > (m_fDeadTime + 5))) )
		return;


	
	if(gpGlobals->time < minimumRespawnDelay){
		//don't pass this point yet.  Respawning way too soon is just annoying in debugging at times.
		return;
	}

	pev->button = 0;
	m_iRespawnFrames = 0;

	//ALERT(at_console, "Respawn\n");

	if(scheduleRemoveAllItems == TRUE){
		RemoveAllItems(scheduleRemoveAllItemsIncludeSuit);
	}

	respawn(pev, !(m_afPhysicsFlags & PFLAG_OBSERVER) );// don't copy a corpse if we're in deathcam.
	pev->nextthink = -1;
}



//=========================================================
// StartDeathCam - find an intermission spot and send the
// player off into observer mode
//=========================================================
void CBasePlayer::StartDeathCam( void )
{
	edict_t *pSpot, *pNewSpot;
	int iRand;

	if ( pev->view_ofs == g_vecZero )
	{
		// don't accept subsequent attempts to StartDeathCam()
		return;
	}

	pSpot = FIND_ENTITY_BY_CLASSNAME( NULL, "info_intermission");	

	if ( !FNullEnt( pSpot ) )
	{
		// at least one intermission spot in the world.
		iRand = RANDOM_LONG( 0, 3 );

		while ( iRand > 0 )
		{
			pNewSpot = FIND_ENTITY_BY_CLASSNAME( pSpot, "info_intermission");
			
			if ( pNewSpot )
			{
				pSpot = pNewSpot;
			}

			iRand--;
		}

		CopyToBodyQue( pev );
		StartObserver( pSpot->v.origin, pSpot->v.v_angle );
	}
	else
	{
		// no intermission spot. Push them up in the air, looking down at their corpse
		TraceResult tr;
		CopyToBodyQue( pev );
		UTIL_TraceLine( pev->origin, pev->origin + Vector( 0, 0, 128 ), ignore_monsters, edict(), &tr );
		StartObserver( tr.vecEndPos, UTIL_VecToAngles( tr.vecEndPos - pev->origin  ) );
		return;
	}
}

void CBasePlayer::StartObserver( Vector vecPosition, Vector vecViewAngle )
{
	m_afPhysicsFlags |= PFLAG_OBSERVER;

	pev->view_ofs = g_vecZero;
	pev->angles = pev->v_angle = vecViewAngle;
	pev->fixangle = TRUE;
	pev->solid = SOLID_NOT;
	pev->takedamage = DAMAGE_NO;
	pev->movetype = MOVETYPE_NONE;
	pev->modelindex = 0;
	UTIL_SetOrigin( pev, vecPosition );
}




// On reaching the delay for setting the next deadStage, this method runs.  Also sets the delay for the next deadstage bump.
// (also instantly for deadStage 0, or the moment it's decided that a revive won't happen).

// optional idea when player is killed:
//   hgrunt / hassault AI becomes friendly to talkmonsters, to allow them to come up and kneel.
//   Talkmonsters will not make normal conversation anymore

// SINGLEPLAYER ONLY.
// When the player is killed...
// - All normal conversation from talkmonsters (idlechat or conversations) is no longer allowed.  (switch that back if revived through cheats, cancel all this really)
// - Of any monster(s) following the player at the time, pick the closest one as a guaranteed kneel-er.  Otherwise, leave that choice open for later.  Don't act on this yet.
// - Any scientists nearby (500 radius) with a line of sight to the player, facing or not, and not already in a combat state will panic (scream more likely?) and run for cover.  Barnies with LOS may turn toward the player and make a startled/pain noise (if not preoccupied by being in a combat state).
// - Wait a minimum of 12 seconds.  Within the first 4 seconds, scientists/barnies behave the same as above if looking at the player while in non-combat states.
//   If any talkmonsters in a 700 radius are not in a combat state, wait for that.  When that happens, set a short delay (6 to 10 seconds).
// --- The delay gets reset if this changes while it is running.
// - Any talkmonster within a radius of 600 will come within a radius of 600 + line of sight.
//   Wait 4-6 seconds.
// - PICKING THE KNEELER.  If picked earlier from the closest follower, and that follower is still alive, that is one of the kneelers picked.  Pick the other (or both if no kneeler #1) from the closest NPC besides that (or top two closest NPCs if no #1).
// - Picked kneelers will pathfind for a distance of 90 from the player to do kneel anims.  May have some line (dialogue) coming later, use filler for now.
// 

// deadStage  (used if player deadflag != DEAD_NO)
// 0: (initial: should not be seen by monsters for very long, if at all; changed to 1 the first time PlayerDeathThink decides that a revive won't happen)
// 1: (killed under 1 second ago).  Scientists not panicing or in combat or barnies not in combat can face me starting now.
// 2: (killed under 4 seconds ago).  
// 3: Waiting for the full 12 second delay.  No changed behavior during this time.
// --- if any talkers are in a combat state, goto stage 4.  Otherwise goto stage 6.
// 4: Every 2 seconds, check to see if any talkers are in a combat state. If not, goto 5 with a longer delay (6.5 sec).
// 5: If yet still, no talkers in a combat state, goto stage 6.  Otherwise, revert to 4 (attempt to advance failed)
// 6: All talkmonsters within 700 units given the order to come look.  Wait 3 seconds.
// 7: Kneelers picked. instructed to come closer to do kneel animations + say line (filler as of now).  Wait 20 seconds.
// 8: Waiting for post-death delay.
// 9: Delay over, deadStage does not change anymore.
//    --- tell talkers with a line of sight with a 60% chance to pick a random nearby node to walk to?



void CBasePlayer::HandleDeadStage(void){
	int i;
	int i2;
	edict_t* pEdict;
	CBaseEntity* pTempEntity;
	switch(deadStage){
	case 0:{

		pEdict = g_engfuncs.pfnPEntityOfEntIndex(1);
		if (!pEdict)return;
		for (i = 1; i < gpGlobals->maxEntities; i++, pEdict++){
			if (pEdict->free)	// Not in use
				continue;
			if (!(pEdict->v.flags & (FL_MONSTER)))	// Not a monster ?
				continue;
			pTempEntity = CBaseEntity::Instance(pEdict);
			if (!pTempEntity)
				continue;
			if (!pTempEntity->isTalkMonster())
				continue;
			if(pTempEntity->IsAlive() && Distance(pTempEntity->pev->origin, pev->origin) < 1200){
				// ok, let it know.  Might or might not actually do anything with that at this instant.
				CTalkMonster* theTalker = static_cast<CTalkMonster*>(pTempEntity);
				if(theTalker->m_MonsterState == MONSTERSTATE_PRONE){
					// nope
					continue;
				}
				theTalker->OnPlayerDead(this);
			}

		}//END OF through all entities.

		deadStage++;
		nextDeadStageTime = gpGlobals->time + 0.6;
	}break;
	case 1:{
		// Make any talker not in a combat state (and scientist not panicing; stopped should be good enough) with line of sight to me face me.
		

		pEdict = g_engfuncs.pfnPEntityOfEntIndex(1);
		if (!pEdict)return;
		for (i = 1; i < gpGlobals->maxEntities; i++, pEdict++){
			if (pEdict->free)	// Not in use
				continue;
			if (!(pEdict->v.flags & (FL_MONSTER)))	// Not a monster ?
				continue;
			pTempEntity = CBaseEntity::Instance(pEdict);
			if (!pTempEntity)
				continue;
			if (!pTempEntity->isTalkMonster())
				continue;
			if(!pTempEntity->IsAlive()){
				continue;
			}

			CTalkMonster* theTalker = static_cast<CTalkMonster*>(pTempEntity);
			if(theTalker->m_MonsterState == MONSTERSTATE_PRONE){
				// nope
				continue;
			}

			// Not moving, not in a combat state (convenient to look at me)? 
			if(!theTalker->IsMoving() && theTalker->m_MonsterState != MONSTERSTATE_COMBAT && Distance(theTalker->pev->origin, this->pev->origin) < 700){
				// have a direct line of sight?  Not already facing enough?
				if(theTalker->FVisible(pev->origin) && !UTIL_IsFacing(theTalker->pev, pev->origin, 0.05) ){
					// Turn and face.
					theTalker->MakeIdealYaw(this->pev->origin);
					theTalker->ChangeSchedule(theTalker->GetScheduleOfType(SCHED_ALERT_FACE));
				}
			}

		}//END OF through all entities.

		deadStage++;
		nextDeadStageTime = gpGlobals->time + 1.4;
	}break;
	case 2:{
		// filler time.

		deadStage++;
		nextDeadStageTime = gpGlobals->time + 3;
	}break;
	case 3:{

		BOOL anythingInCombat = FALSE;
		// any talkmonsters in a combat state within a certain radius?

			pEdict = g_engfuncs.pfnPEntityOfEntIndex(1);
			if (!pEdict)return;
			for (i = 1; i < gpGlobals->maxEntities; i++, pEdict++){
				if (pEdict->free)	// Not in use
					continue;
				if (!(pEdict->v.flags & (FL_MONSTER)))	// Not a monster ?
					continue;
				pTempEntity = CBaseEntity::Instance(pEdict);
				if (!pTempEntity)
					continue;
				if (!pTempEntity->isTalkMonster())
					continue;
				if(!pTempEntity->IsAlive()){
					continue;
				}

				CTalkMonster* theTalker = static_cast<CTalkMonster*>(pTempEntity);
				if(theTalker->m_MonsterState == MONSTERSTATE_PRONE){
					// do not count still
					continue;
				}

				if(Distance(theTalker->pev->origin, this->pev->origin) < 700){
					if(theTalker->m_MonsterState == MONSTERSTATE_COMBAT){
						anythingInCombat = TRUE;
						break;
					}
				}//DISTANCE MY buddy
			}//END OF through all entities.


			if(anythingInCombat){
				// Do another check for anything in combat later.
				deadStage = 3;
				nextDeadStageTime = gpGlobals->time + 5;
			}else{
				// Skip this then
				deadStage = 6;
				HandleDeadStage();
			}

	}break;
	case 5:{

		/*
		if(any monsters in combat){
			// oops.  Revert.
			deadStage = 4;
			nextDeadStageTime = gpglobals->time + 2;
		}else{
			// Still safe?  Proceed
			deadStage = 6;
			HandleDeadStage();
		}
		*/
	}break;
	case 6:{
		// all talkers come out to look at the dead player origin, if their 'deadPlayerFocus' is set to this
		

		// any talkmonsters in a combat state within a certain radius?

		pEdict = g_engfuncs.pfnPEntityOfEntIndex(1);
		if (!pEdict)return;
		for (i = 1; i < gpGlobals->maxEntities; i++, pEdict++){
			if (pEdict->free)	// Not in use
				continue;
			if (!(pEdict->v.flags & (FL_MONSTER)))	// Not a monster ?
				continue;
			pTempEntity = CBaseEntity::Instance(pEdict);
			if (!pTempEntity)
				continue;
			if (!pTempEntity->isTalkMonster())
				continue;
			if(!pTempEntity->IsAlive()){
				continue;
			}

			CTalkMonster* theTalker = static_cast<CTalkMonster*>(pTempEntity);
			if(theTalker->m_MonsterState == MONSTERSTATE_PRONE){
				// do not count still
				continue;
			}

			if(/*theTalker->FVisible(pev->origin) &&*/ Distance(theTalker->pev->origin, this->pev->origin) < 1400){
				// COME TO ME friend

				theTalker->ChangeScheduleToApproachDeadPlayer(this->pev->origin);
			}//DISTANCE MY buddy
		}//END OF through all entities.




		deadStage++;
		nextDeadStageTime = gpGlobals->time + 3;
	}break;
	case 7:{
		// tell kneelers to come closer
		// IDEA:  Test the picked kneelers (whether 1 is from the nearest following or top 2 closest)
		// and see if they can route to the player.  Whatever can't, pick the next nearest instead.
		// (was there some safe-route test method for that?  See around MOVE_TO_ENEMY_RANGE)

		int ary_kneelers_count = 0;
		int ary_kneelers_offset = 0;
		CTalkMonster* ary_kneelers[2];

		// First, look through the list of nearest followers.  Whichever of those is closest now can be a
		// kneeler.
		float bestDistoYet = 1200;
		CTalkMonster* bestMofoYet = NULL;

		for(i = 0; i < recentDeadPlayerFollowersCount; i++){
			if(recentDeadPlayerFollowers[i] != NULL){
				float thisDisto = Distance(recentDeadPlayerFollowers[i]->pev->origin, pev->origin);
				if(thisDisto < bestDistoYet){
					bestDistoYet = thisDisto;
					bestMofoYet = static_cast<CTalkMonster*>(recentDeadPlayerFollowers[i].GetEntity());
				}
			}
		}

		if(bestMofoYet != NULL){
			// pick it as #0
			ary_kneelers[ary_kneelers_count] = bestMofoYet;
			ary_kneelers_count++;
			ary_kneelers_offset = 1;
		}

		// now. the two closest?


		bestDistoYet = 1200;
		//bestMofoYet = NULL;

		pEdict = g_engfuncs.pfnPEntityOfEntIndex(1);
		if (!pEdict)return;
		for (i = 1; i < gpGlobals->maxEntities; i++, pEdict++){
			if (pEdict->free)	// Not in use
				continue;
			if (!(pEdict->v.flags & (FL_MONSTER)))	// Not a monster ?
				continue;
			pTempEntity = CBaseEntity::Instance(pEdict);
			if (!pTempEntity)
				continue;
			if (!pTempEntity->isTalkMonster())
				continue;
			if(!pTempEntity->IsAlive()){
				continue;
			}

			CTalkMonster* theTalker = static_cast<CTalkMonster*>(pTempEntity);
			if(theTalker->m_MonsterState == MONSTERSTATE_PRONE){
				// do not count still
				continue;
			}

			BOOL skippo = FALSE;
			// if already in the list, don't involve
			for(i2 = 0; i2 < ary_kneelers_count; i2++){
				if(theTalker->edict() == ary_kneelers[i2]->edict()){
					skippo = TRUE;
					break;
				}
			}
			if (skippo)continue;


			float thisDisto = Distance(theTalker->pev->origin, pev->origin);
			// can also accept anything if the ary_kneelers_count is less than 2.  Must still be within 1200 in distance though
			if(thisDisto < bestDistoYet || (thisDisto < 1200 && ary_kneelers_count < 2) ){
				bestDistoYet = thisDisto;

				// Idea:  on finding a best distance, goto the current ary_kneelers_offset place
				// in ary_kneelers (ary_kneelers_offset is 1 if one was picked from being the nearest
				// follower, it is 0 if there were no followers to choose from).
				// Shift all positions above this up by one, and store the current ent into the current
				// ary_kneelers_offset place.  That means whatever had the previous best distance would
				// get the slot after this one (if there was more than 1 left from ary_kneelers_offset being 0).
				// effect:
				//[3] a       [3] b
				//[2] b       [2] c
				//[1] c       [1] d
				//[0] d  -->  [0] *

				//for(int i2 = ary_kneelers_offset; i2 < 2-1; i2++){
				for(i2 = 2-1; i2 > ary_kneelers_offset; i2--){
					ary_kneelers[i2] = ary_kneelers[i2-1];
				}
				ary_kneelers[ary_kneelers_offset] = theTalker;
				// no more than 2 allowed
				if(ary_kneelers_count < 2){
					ary_kneelers_count++;
				}
			}//bestdist check

		}//END OF through all entities.


		// go through ary_kneelers, make em' come up closer to kneel
		for (i2 = 0; i2 < ary_kneelers_count; i2++) {
			ary_kneelers[i2]->ChangeScheduleToApproachDeadPlayerKneel(this->pev->origin);
		}

		//ChangeScheduleToApproachDeadPlayerKneel

		deadStage++;
		nextDeadStageTime = gpGlobals->time + 24;
	}break;
	case 8:{
		// nothing special really, talkers can know this is the post-death period for any other dialogue about that (if it exists)

		// DEAD TODO:  60% of the time, each walker with a line of sight to the player nearby picks a random node nearby to walk to?
		// maybe not.

		// allow post death conversation now, if ever implemented.
		g_TalkMonster_PlayerDead_DialogueMod = 2;

		deadStage++;
	}break;
	}//switch on deadStage

}//HandleDeadStage




// 
// PlayerUse - handles USE keypress
//
void CBasePlayer::PlayerUse ( void ){
	// Was use pressed or released?
	if ( ! ((pev->button | m_afButtonPressed | m_afButtonReleased) & IN_USE) )
		return;
	
	// Hit Use on a train?
	if ( m_afButtonPressed & IN_USE )
	{
		if ( m_pTank != NULL )
		{
			// Stop controlling the tank
			// TODO: Send HUD Update
			m_pTank->Use( this, this, USE_OFF, 0 );
			m_pTank = NULL;
			return;
		}
		else
		{
			if ( m_afPhysicsFlags & PFLAG_ONTRAIN )
			{
				m_afPhysicsFlags &= ~PFLAG_ONTRAIN;
				m_iTrain = TRAIN_NEW|TRAIN_OFF;
				return;
			}
			else
			{	// Start controlling the train!
				CBaseEntity *pTrain = CBaseEntity::Instance( pev->groundentity );

				if ( pTrain && !(pev->button & IN_JUMP) && FBitSet(pev->flags, FL_ONGROUND) && (pTrain->ObjectCaps() & FCAP_DIRECTIONAL_USE) && pTrain->OnControls(pev) )
				{
					m_afPhysicsFlags |= PFLAG_ONTRAIN;
					m_iTrain = TrainSpeed(pTrain->pev->speed, pTrain->pev->impulse);
					m_iTrain |= TRAIN_NEW;
					//MODDD - soundsentencesave
					UTIL_PlaySound( ENT(pev), CHAN_ITEM, "plats/train_use1.wav", 0.8, ATTN_NORM, 0, 100, FALSE);
					return;
				}
			}
		}
	}

	//MODDD - Keep the closest object in mind too, of the dotproducts are not significantly different,
	// the distance may be a better indicator.

	CBaseEntity* pToUseOn = NULL;
	Vector ToUse_LOS;

	CBaseEntity* pObject = NULL;
	Vector vecLOS;

	CBaseEntity* pClosestLOS = NULL;
	Vector closestLOS;

	CBaseEntity* pClosestDistance = NULL;
	Vector closestDistance_LOS;
	float closestDistance_Dot;


	//MODDD - VIEW_FIELD_NARROW is 0.7  (+- 45 degrees).
	//float flMaxDot = VIEW_FIELD_NARROW;
	//float flMaxDot = (float)0.94;
	//float flMaxDot = (float)0.905;
	float flMaxDot = (float)0.78;

	// anything should satisfy this default
	float closestDistance = PLAYER_USE_SEARCH_RADIUS + 10;
	

	float flDot;
	float thisDistance;



	UTIL_MakeVectors ( pev->v_angle );// so we know which way we are facing
	Vector2D vecForward2D = gpGlobals->v_forward.Make2D();



	if (EASY_CVAR_GET_DEBUGONLY(playerUseDrawDebug) == 2) {
		DebugLine_ClearAll();
	}

	// We want the object we pick to the the one we are looking the most directly at.
	// After all, why would the user be looking it?
	// HOWEVER.  There are times this still tries to pick up something in the background when we meant for 
	// something up close.  If the dot products between the top two items are similar, use distance instead.

	//MODDD - slight adjustment. Go forwards a bit to search instead... UNDONE.
	while ((pObject = UTIL_FindEntityInSphere( pObject, pev->origin, PLAYER_USE_SEARCH_RADIUS )) != NULL)
	//while ((pObject = UTIL_FindEntityInSphere( pObject, pev->origin + gpGlobals->v_forward * 64, PLAYER_USE_SEARCH_RADIUS )) != NULL)
	{
		//MODDD - no check for self?  what?
		if(pObject == this){
			continue;
		}

		const char* theClassname = pObject->getClassname();


		//MODDD - distance check... UNDONE.
		if (pObject->ObjectCaps() & (FCAP_IMPULSE_USE | FCAP_CONTINUOUS_USE | FCAP_ONOFF_USE))
		//if (((pObject->pev->origin - this->pev->origin).Length() <= 64) && pObject->ObjectCaps() & (FCAP_IMPULSE_USE | FCAP_CONTINUOUS_USE | FCAP_ONOFF_USE))
		{
			// !!!PERFORMANCE- should this check be done on a per case basis AFTER we've determined that
			// this object is usable? This dot is being done for every object within PLAYER_USE_SEARCH_RADIUS
			// when player hits the use key. How many objects can be in that area, anyway? (sjb)
			vecLOS = (VecBModelOrigin( pObject->pev ) - (pev->origin + pev->view_ofs));
			
			// This essentially moves the origin of the target to the corner nearest the player to test to see 
			// if it's "hull" is in the view cone
			vecLOS = UTIL_ClampVectorToBoxNonNormalized( vecLOS, pObject->pev->size * 0.5 );

			Vector closestPointOnBox = pev->origin + pev->view_ofs + vecLOS;
			if (EASY_CVAR_GET_DEBUGONLY(playerUseDrawDebug) == 2) {
				DebugLine_SetupPoint(closestPointOnBox, 0, 0, 255);
			}
			
			Vector vecLOSNorm = vecLOS.Normalize();

			//MODDD - original dotproduct line:
			//flDot = DotProduct (vecLOSNorm , gpGlobals->v_forward);

			//MODDD - use the 2D dot product instead.
			// ...no, some part of the Z component still matters.
			// Could do something complicated with a 2D dot product check and a check with the
			// Z component, sounds simple but the issue is when looking straightup (0, 0, 1).
			// The 2D "0, 0" multiplied by anything is still 0, so that a 'good' 2D dotproduct
			// from that is impossible.  Perhaps giving the Z component more value as the 
			// floor-wise direction length gets closer to 0 would work.
			// Anyway, for now, just multiply the Z component by 0.5 of both vectors and
			// re-normalize.  Or maybe we don't need to renormalize?
			// Messes with the math behind a dotproduct if you don't normalize, but 
			// keep in mind, (0,0,1) is still a bit odd.  Half is (0,0,0.5), and normalizing
			// that gets the same (0,0,1) back (0 for X and Y components can't be influenced).
			// Anything close to looking up to begin with, like (0, 0.01, 0.99) (rough),
			// will just be looking further away from the top while the (0,0,1) stays the same.
			// But leaving as halved Z components, so (0,0,0.5) and (0,0.01,0.495).
			// Although it's still strange that the most the dot product between two
			// straight verticals, (0,0,1) with (0,0,1), = (0,0,0.5) with (0,0,0.5),
			// is 0*0 + 0*0 + 0.5*0.5 = 0.25.
			// It may seem like it makes sense to multiply this by 4 to make it 1 (so that similarity
			// still brings it close to 1), this just undoes the Z-halving of each, making that
			// pointless.

			Vector2D vecLOS2D = vecLOSNorm.Make2D();
			// CHECK.  Lines going straight up/down (0,0,+-1) will cause
			// the X and Y of the resulting 2D vector be 0 just fine.
			// Made a mistake, they were "NaN" because the divide lines below
			// were dividing by that "0" length.


			float vecLOS2D_prevLength = vecLOS2D.Length();
			float vecForward2D_prevLength = vecForward2D.Length();

			// Make each vector normalized.
			// This remembers their 2D lengths before normalizing this way.
			if (vecLOS2D_prevLength != 0) {
				vecLOS2D = vecLOS2D / vecLOS2D_prevLength;
			}
			if (vecForward2D_prevLength != 0) {
				vecForward2D = vecForward2D / vecForward2D_prevLength;
			}

			// How about the average of the 2D prevLengths?
			float weight2D = (vecLOS2D_prevLength + vecForward2D_prevLength) / 2;
			// There are two coords to multiply this by in 2D.
			// Multiplying both by the solid "weight2D" would throw off the balance,
			// this way, it all still adds up to the same number.
			//float weight2D_each = (1 - weight2D) / 2 + weight2D;
			//float weight2D_each = (1 + weight2D )/2;
			//float weight2D_each = (1 + pow(weight2D, 2) )/2;
			float weight2D_each = pow((1 + weight2D )/2, 2);
			// weightZ, is whatever didn't go to weight2D.
			float weightZ = 1 - weight2D;
			//float weightZ = 1 - pow(weight2D, 2);
			//float weightZ = pow(1 - weight2D, 2);

			// Now... commence.
			//x*x + y*y + z*z = 1

			// 1*1 + 1*1 + 1*1 = 3
			// 1*1 + 1*1 + 1*1*0.5 = 2.5
			// 1*1*0.5 + 1*1*0.5 +1*1 = 2
			// 1*1*0.75 + 1*1*0.75 + 1*1 = .2.5

			//1*1 + 1*1 + 1*0.3 = 2.3
			//1*1*0.3 + 1*1*0.3 + 1 = 1.6
			//1*1*0.65 + 1*1*0.65 + 1 = 2.3

			//1*1 + 1*1 + 1*0.8 = 2.8
			//1*1*0.8 + 1*1*0.8 + 1 = 2.6
			//1*1*0.9 + 1*1*0.9 + 1 = 2.8

			flDot = vecLOS2D.x * vecForward2D.x * weight2D_each +
				vecLOS2D.y * vecForward2D.y * weight2D_each +
				vecLOSNorm.z * gpGlobals->v_forward.z * weightZ;

			
			if (EASY_CVAR_GET_DEBUGONLY(playerUseDrawDebug) == 1) {
				// the normal way, for comparison.
				float flDotAlt = DotProduct (vecLOSNorm , gpGlobals->v_forward);

				// For a spectacular error, use the first version.
				//easyForcePrintLine("playeruse: DOTPROD TO OBJ? %s: %.2f", flDot, pObject->getClassname());
				CBaseMonster* monTest = pObject->GetMonsterPointer();
				if (monTest != NULL) {
					easyForcePrintLine("playeruse: DOTPROD TO OBJ? %s:%d: %.2f oldway: %.2f", monTest->getClassname(), monTest->monsterID, flDot, flDotAlt);
				}
				else {
					easyForcePrintLine("playeruse: DOTPROD TO OBJ? %s: %.2f oldway: %.2f", pObject->getClassname(), flDot, flDotAlt);
				}
			}


			//closestPointOnBox
			//thisDistance = Distance(pev->origin, pObject->pev->origin);
			thisDistance = Distance(pev->origin + pev->view_ofs, closestPointOnBox);

			// Must also be looking at it enough.
			if (flDot > 0.73 && thisDistance < closestDistance) {
				//ok
				pClosestDistance = pObject;
				closestDistance = thisDistance;
				// in case the distance-closer object gets picked, or the FOS rather for seeing if it should be.
				closestDistance_LOS = vecLOS;
				closestDistance_Dot = flDot;

				// TODO : debug printouts for the next best-distance object?
			}




			if (flDot > flMaxDot )
			{// only if the item is in front of the user
				pClosestLOS = pObject;
				closestLOS = vecLOS; //MODDD - new.
				flMaxDot = flDot;
//				ALERT( at_console, "%s : %f\n", STRING( pObject->pev->classname ), flDot );
			}
//			ALERT( at_console, "%s : %f\n", STRING( pObject->pev->classname ), flDot );
		}
	}//END OF loop through entities to check for using




	//OLD WAY, pClosestLOS was the only choice.
	//pToUseOn = pClosestLOS;
	//ToUse_LOS = closestLOS

	// Now to choose between pClosestLOS and pClosestDistance.
	// Or if either/both are NULL, it isn't much of a decision.



	if (pClosestDistance != NULL && pClosestLOS != NULL) {
		// the decision.

		if (pClosestDistance == pClosestLOS) {
			// They are equal?  It's all the same then.
			pToUseOn = pClosestLOS;
			ToUse_LOS = closestLOS;
		}
		else {
			// Different and non-null?

			// should never be negative, but I'm paranoid.
			//flMaxDot >= 0.96 ||
			if ( fabs(flMaxDot - closestDistance_Dot) >= 0.22) {
				// pick the most aimed at
				pToUseOn = pClosestLOS;
				ToUse_LOS = closestLOS;
			}
			else {
				// pick the closest.
				pToUseOn = pClosestDistance;
				ToUse_LOS = closestDistance_LOS;
			}
		}

	}
	else {
		if (pClosestLOS == NULL) {
			if (pClosestDistance == NULL) {
				// both null
			}
			else {
				pToUseOn = pClosestDistance;
				ToUse_LOS = closestDistance_LOS;
			}
		}else if (pClosestDistance == NULL) {
			if (pClosestLOS == NULL) {
				// both null
			}
			else {
				pToUseOn = pClosestLOS;
				ToUse_LOS = closestLOS;
			}
		}
	}



	//DEBUG.
	/*
	if(pToUseOn){
		easyForcePrintLine("PLAYER USE: %s", pToUseOn->getClassname());
	}else{
		easyForcePrintLine("PLAYER USE: nothing.");
	}
	*/

	BOOL flUseSuccess = FALSE;
	
	//easyForcePrintLine("aaaaaaaaAAAaaaa %d", pToUseOn==NULL);

	// Found an object
	if (pToUseOn )
	{
		if (EASY_CVAR_GET_DEBUGONLY(playerUseDrawDebug) == 1) {
			easyForcePrintLine("playeruse: WHAT IS OBJECT I PICK?? %s", pToUseOn->getClassname());
		}
		//!!!!!!!!!!
		//!!!UNDONE: traceline here to prevent USEing buttons through walls		
		edict_t		*pentIgnore;
		TraceResult tr;

		pentIgnore = this->edict();
		UTIL_MakeVectors(pev->v_angle);// + pev->punchangle);
		
		//Vector vecDirTowardsClosest = (pToUseOn->pev->origin - this->pev->origin).Normalize();
		Vector vecDirTowardsClosest = ToUse_LOS;

		//(vecDirTowardsClosest - this->pev->origin).Length(); 
		//float distToClosest = (pToUseOn->pev->origin - this->pev->origin).Length();
		float distToClosest = vecDirTowardsClosest.Length();

		//notice: NOT "gpGlobals->v_forward", which is just what direction the player happens to be facing. We want a straight line to the object we are trying to touch.
		Vector vecSrc = pev->origin + pev->view_ofs + vecDirTowardsClosest * 0;

		//why a little over 1 (100%)? To go a little further just to make sure we hit it.
		Vector vecDest = pev->origin + pev->view_ofs + vecDirTowardsClosest * 1.1;//(0+distToClosest+1); //(0+PLAYER_USE_SEARCH_RADIUS);

		//nah, precision for while ducking not necessary.
		

		if(pev->flags & FL_DUCKING){
			vecSrc.z -= 1;
			vecDest.z -= 1;
		}
		

		//easyForcePrintLine("?????????????????????????????????");

		// Do we have an unobstructed line to the thing we want to "use"?
		UTIL_TraceLine( vecSrc, vecDest, dont_ignore_monsters, pentIgnore, &tr );
		//tr.vecEndPos();

		/*
		//could still be ok? Skip this check.
		if (tr.fAllSolid){
			//trace failed this early? just stop.
			return;
		}
		*/
		
		//debugVect1Draw = TRUE;
		if(EASY_CVAR_GET_DEBUGONLY(playerUseDrawDebug) == 1){
			::DebugLine_ClearAll();
		}
		
		//debugVect1End

		if(EASY_CVAR_GET_DEBUGONLY(playerUseDrawDebug) == 1)DebugLine_Setup(0, vecSrc, vecSrc+vecDirTowardsClosest, tr.flFraction);


		//tr.flFraction is in case it goes too far by a bit.
		if(tr.pHit != NULL ){

			CBaseEntity* hitEntity = CBaseEntity::Instance(tr.pHit);

			float distToPointHit = (vecDirTowardsClosest.Length());

			//easyForcePrintLine("HIT SOMETHING? %s fract:%.2f distoff:%.2f", STRING(tr.pHit->v.classname), (tr.flFraction), distToPointHit*(1 - (tr.flFraction)) );

			//may hit worldspawn, which does not block. Not sure why it even counts as hit if so (flFraction stays 1)
			if( (hitEntity != NULL && hitEntity->pev == pToUseOn->pev) || distToPointHit<=5 || distToPointHit*(1 - (tr.flFraction)) <= 10 ){
				//the trace-hit entity matches the entity selected to "use" on? this is valid.
				flUseSuccess = TRUE;
				if (EASY_CVAR_GET_DEBUGONLY(playerUseDrawDebug) == 1) {
					easyForcePrintLine("playeruse: That was easy. %s", pToUseOn->getClassname());
				}
			}else{

				if( hitEntity != NULL && ::FClassnameIs(hitEntity->pev, "worldspawn")  ){
					//possible exception. See if this is the case.
					if(tr.flFraction>=1.0){
						if (EASY_CVAR_GET_DEBUGONLY(playerUseDrawDebug) == 1) {
							easyForcePrintLine("playeruse: weird flag A (success) %s", pToUseOn->getClassname());
						}
						//missed touching anything? just count it.
						flUseSuccess = TRUE;
					}else{
						//worldspawn but still an inadequate "fraction" of the distance reached towards the target?
						//Try from the view point of the player to the target instead.
						Vector vecSrc2 = pev->origin + pev->view_ofs;

						Vector vecDest2 = VecBModelOrigin(pToUseOn->pev);

						TraceResult tr2;
						UTIL_TraceLine( vecSrc2, vecDest2, dont_ignore_monsters, pentIgnore, &tr2 );
						
						float distToPointHit2 = (vecDest2 - (vecSrc2)).Length();
						
						if(EASY_CVAR_GET_DEBUGONLY(playerUseDrawDebug) == 1)DebugLine_Setup(0, vecSrc2, vecDest2, tr2.flFraction);


						//debugVect1End =  vecDest2;

						//easyForcePrintLine("HIT SOMETHING B? %s fract:%.2f distoff:%.2f", STRING(tr2.pHit->v.classname), (tr2.flFraction), distToPointHit2*(1 - (tr2.flFraction)) );
						
						if(tr2.pHit != NULL){
							CBaseEntity* hitEntity2 = CBaseEntity::Instance(tr2.pHit);
							if( (hitEntity2 != NULL && hitEntity2->pev == pToUseOn->pev) || distToPointHit2<=5 || distToPointHit2*(1 - (tr2.flFraction)) <= 10){
								//it's good!
								if (EASY_CVAR_GET_DEBUGONLY(playerUseDrawDebug) == 1) {
									easyForcePrintLine("playeruse: weird flag B (success) %s", pToUseOn->getClassname());
								}
								flUseSuccess = TRUE;
							}else{
								if (EASY_CVAR_GET_DEBUGONLY(playerUseDrawDebug) == 1) {
									easyForcePrintLine("playeruse: weird flag C (fail) %s", pToUseOn->getClassname());
								}
								flUseSuccess = FALSE;
							}
						}else{
							if (EASY_CVAR_GET_DEBUGONLY(playerUseDrawDebug) == 1) {
								easyForcePrintLine("playeruse: weird flag D (success) %s", pToUseOn->getClassname());
							}
							flUseSuccess = TRUE;
						}

					}//END OF inner center test
				}else{
					//mismatch? not ok.
					if (EASY_CVAR_GET_DEBUGONLY(playerUseDrawDebug) == 1) {
						easyForcePrintLine("playeruse: weird flag E (fail) %s", pToUseOn->getClassname());
					}
					flUseSuccess = FALSE;
				}
			}
			

		}else{
			if (EASY_CVAR_GET_DEBUGONLY(playerUseDrawDebug) == 1) {
				easyForcePrintLine("playeruse: weird flag F (success) %s", pToUseOn->getClassname());
			}
			//hit nothing? Just assume it worked.
			flUseSuccess = TRUE;
		}


		//if(EASY_CVAR_GET_DEBUGONLY(playerUseDrawDebug) == 1)aryDebugLines[0].setSuccess(flUseSuccess);

		if (EASY_CVAR_GET_DEBUGONLY(playerUseDrawDebug) == 1) {
			//if successful, force it all green.
			if (flUseSuccess) { DebugLine_ColorSuccess(0); };
		}

		/*
		tempplayer->GiveNamedItem( CMD_ARGV(1),  attemptInterpretSpawnFlag(CMD_ARGV(2)),
									tr.vecEndPos.x,
									tr.vecEndPos.y,
									tr.vecEndPos.z + EASY_CVAR_GET_DEBUGONLY(offsetgivelookvertical),
									TRUE, &tr);
		*/
	}else{
		//no object touched? stop.
		flUseSuccess = FALSE;

		if(EASY_CVAR_GET_DEBUGONLY(playerUseDrawDebug) == 1){
			::DebugLine_ClearAll();
		}
	}


	//MODDD DEBUG FEATURE - STOP FOR NOW.
	//return;


	if(flUseSuccess == TRUE){
		
		int caps = pToUseOn->ObjectCaps();

		if ( m_afButtonPressed & IN_USE ){
			UTIL_PlaySound( ENT(pev), CHAN_ITEM, "common/wpn_select.wav", 0.4, ATTN_NORM, 0, 100, FALSE);
		}

		if ( ( (pev->button & IN_USE) && (caps & FCAP_CONTINUOUS_USE) ) ||
			 ( (m_afButtonPressed & IN_USE) && (caps & (FCAP_IMPULSE_USE|FCAP_ONOFF_USE)) ) )
		{
			if ( caps & FCAP_CONTINUOUS_USE )
				m_afPhysicsFlags |= PFLAG_USING;

			pToUseOn->Use( this, this, USE_SET, 1 );
		}
		// UNDONE: Send different USE codes for ON/OFF.  Cache last ONOFF_USE object to send 'off' if you turn away
		else if ( (m_afButtonReleased & IN_USE) && (pToUseOn->ObjectCaps() & FCAP_ONOFF_USE) )	// BUGBUG This is an "off" use
		{
			pToUseOn->Use( this, this, USE_SET, 0 );
		}
	}
	else
	{

		//NOTE - is that a little hard to read? m_afButtonPressed is a bitmask of inputs. IN_USE is on when the player pressed the USE key this frame, I assume.
		//      So this just means, only play the "deny" sound if pressing the USE key.
		if ( m_afButtonPressed & IN_USE ){
			UTIL_PlaySound( ENT(pev), CHAN_ITEM, "common/wpn_denyselect.wav", 0.4, ATTN_NORM, 0, 100, FALSE);
		}
	}
}//END OF PlayerUse



void CBasePlayer::Jump()
{
	Vector		vecWallCheckDir;// direction we're tracing a line to find a wall when walljumping
	Vector		vecAdjustedVelocity;
	Vector		vecSpot;
	TraceResult	tr;
	
	if (FBitSet(pev->flags, FL_WATERJUMP))
		return;
	
	if (pev->waterlevel >= 2)
	{
		return;
	}

	// jump velocity is sqrt( height * gravity * 2)

	// If this isn't the first frame pressing the jump button, break out.
	if ( !FBitSet( m_afButtonPressed, IN_JUMP ) )
		return;         // don't pogo stick

	if ( !(pev->flags & FL_ONGROUND) || !pev->groundentity )
	{
		return;
	}

	//MODDD - use UTIL_MakeAimVectors on pev->angles
// many features in this function use v_forward, so makevectors now.
	UTIL_MakeAimVectors (pev->angles);

	// ClearBits(pev->flags, FL_ONGROUND);		// don't stairwalk
	
	SetAnimation( PLAYER_JUMP );

	//MODDD - require a slightly stricter condition:
	/*
	if ( m_fLongJump &&
		(pev->button & IN_DUCK) &&
		( pev->flDuckTime > 0 ) &&
		pev->velocity.Length() > 50 )
	{
		SetAnimation( PLAYER_SUPERJUMP );
	}
	*/

	
	/*if ( m_fLongJump &&
		(pev->button & IN_DUCK) &&
		( pev->flDuckTime >= 0 ) &&
		pev->velocity.Length() > 0 &&
		//!pev->button & IN_JUMP &&
		FBitSet ( pev->flags, FL_ONGROUND ) &&
		longJumpCharge == PLAYER_LONGJUMPCHARGE_MAX)
		*/

	//MODDD - CHECK POINT CHECKPOINT
	//MODDD - nah, let's just use the same condition that means we would've longjumped:

#if LONGJUMPUSESDELAY == 1
	if (m_fLongJump && longJumpCharge >= PLAYER_LONGJUMPCHARGE_MAX)
	{
		SetAnimation( PLAYER_SUPERJUMP );
	}
#else
	//easyPrint("yes anim? %.2f\n", (lastDuckVelocityLength)  );
	if (m_fLongJump && longJump_waitForRelease && lastDuckVelocityLength > 7 && (EASY_CVAR_GET_DEBUGONLY(itemBatteryPrerequisite) || pev->armorvalue > 0 ))
	{
		SetAnimation( PLAYER_SUPERJUMP );
	}

#endif

	/*
	if(longJumpCharge != 0){
		longJumpChargeNeedsUpdate = TRUE;
		longJumpCharge = 0;
	}
	*/


	// If you're standing on a conveyor, add it's velocity to yours (for momentum)
	entvars_t *pevGround = VARS(pev->groundentity);
	if ( pevGround && (pevGround->flags & FL_CONVEYOR) )
	{
		pev->velocity = pev->velocity + pev->basevelocity;
	}
}




void CBasePlayer::Duck( )
{
	if (pev->button & IN_DUCK) 
	{
		if ( m_IdealActivity != ACT_LEAP )
		{
			SetAnimation( PLAYER_WALK );
		}
	}
}

//
// ID's player as such.
//
int  CBasePlayer::Classify ( void )
{
	return CLASS_PLAYER;
}


void CBasePlayer::AddPoints( int score, BOOL bAllowNegativeScore )
{
	// Positive score always adds
	if ( score < 0 )
	{
		if ( !bAllowNegativeScore )
		{
			if ( pev->frags < 0 )		// Can't go more negative
				return;
			
			if ( -score > pev->frags )	// Will this go negative?
			{
				score = -pev->frags;		// Sum will be 0
			}
		}
	}

	pev->frags += score;

	MESSAGE_BEGIN( MSG_ALL, gmsgScoreInfo );
		WRITE_BYTE( ENTINDEX(edict()) );
		WRITE_SHORT( pev->frags );
		WRITE_SHORT( m_iDeaths );
		//WRITE_SHORT( 0 );  No need, playerclass longer expected.
		WRITE_SHORT( g_pGameRules->GetTeamIndex( m_szTeamName ) + 1 );
	MESSAGE_END();
}


void CBasePlayer::AddPointsToTeam( int score, BOOL bAllowNegativeScore )
{
	int index = entindex();

	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CBaseEntity *pPlayer = UTIL_PlayerByIndex( i );

		if ( pPlayer && i != index )
		{
			if ( g_pGameRules->PlayerRelationship( this, pPlayer ) == GR_TEAMMATE )
			{
				pPlayer->AddPoints( score, bAllowNegativeScore );
			}
		}
	}
}

//Player ID
void CBasePlayer::InitStatusBar()
{
	m_flStatusBarDisappearDelay = 0;
	m_SbarString1[0] = m_SbarString0[0] = 0; 
}

//MODDD - uh, found as-is.  WHAT IS THIS???!
// Oh, this is what puts the text showing the name of the player you're looking at in multiplayer.
// Shows health/ammo too, but only for allies in some team-based mode.  Seems rare to ever run into that though.
// TODO: maybe a CVar to force health/ammo on all the time just for the heck of it?   eh.
void CBasePlayer::UpdateStatusBar()
{
	int newSBarState[ SBAR_END ];
	char sbuf0[ SBAR_STRING_SIZE ];
	char sbuf1[ SBAR_STRING_SIZE ];

	memset( newSBarState, 0, sizeof(newSBarState) );
	strcpy( sbuf0, m_SbarString0 );
	strcpy( sbuf1, m_SbarString1 );

	// Find an ID Target
	TraceResult tr;
	UTIL_MakeVectors( pev->v_angle + pev->punchangle );
	Vector vecSrc = EyePosition();
	Vector vecEnd = vecSrc + (gpGlobals->v_forward * MAX_ID_RANGE);
	UTIL_TraceLine( vecSrc, vecEnd, dont_ignore_monsters, edict(), &tr);

	if (tr.flFraction != 1.0)
	{
		if ( !FNullEnt( tr.pHit ) )
		{
			CBaseEntity *pEntity = CBaseEntity::Instance( tr.pHit );

			if (pEntity != NULL && pEntity->Classify() == CLASS_PLAYER )
			//if (pEntity != NULL )
			{
				newSBarState[ SBAR_ID_TARGETNAME ] = ENTINDEX( pEntity->edict() );
				strcpy( sbuf1, "1 %p1\n2 Health: %i2%%\n3 Armor: %i3%%" );

				// allies and medics get to see the targets health
				if ( g_pGameRules->PlayerRelationship( this, pEntity ) == GR_TEAMMATE )
				{
					newSBarState[ SBAR_ID_TARGETHEALTH ] = 100 * (pEntity->pev->health / pEntity->pev->max_health);
					newSBarState[ SBAR_ID_TARGETARMOR ] = pEntity->pev->armorvalue; //No need to get it % based since 100 it's the max.
				}

				m_flStatusBarDisappearDelay = gpGlobals->time + 1.0;
			}
		}
		else if ( m_flStatusBarDisappearDelay > gpGlobals->time )
		{
			// hold the values for a short amount of time after viewing the object
			newSBarState[ SBAR_ID_TARGETNAME ] = m_izSBarState[ SBAR_ID_TARGETNAME ];
			newSBarState[ SBAR_ID_TARGETHEALTH ] = m_izSBarState[ SBAR_ID_TARGETHEALTH ];
			newSBarState[ SBAR_ID_TARGETARMOR ] = m_izSBarState[ SBAR_ID_TARGETARMOR ];
		}
	}

	BOOL bForceResend = FALSE;

	if ( strcmp( sbuf0, m_SbarString0 ) )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgStatusText, NULL, pev );
			WRITE_BYTE( 0 );
			WRITE_STRING( sbuf0 );
		MESSAGE_END();

		strcpy( m_SbarString0, sbuf0 );

		// make sure everything's resent
		bForceResend = TRUE;
	}

	if ( strcmp( sbuf1, m_SbarString1 ) )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgStatusText, NULL, pev );
			WRITE_BYTE( 1 );
			WRITE_STRING( sbuf1 );
		MESSAGE_END();

		strcpy( m_SbarString1, sbuf1 );

		// make sure everything's resent
		bForceResend = TRUE;
	}

	// Check values and send if they don't match
	for (int i = 1; i < SBAR_END; i++)
	{
		if ( newSBarState[i] != m_izSBarState[i] || bForceResend )
		{
			MESSAGE_BEGIN( MSG_ONE, gmsgStatusValue, NULL, pev );
				WRITE_BYTE( i );
				WRITE_SHORT( newSBarState[i] );
			MESSAGE_END();

			m_izSBarState[i] = newSBarState[i];
		}
	}
}



void CBasePlayer::PreThink(void)
{
	//if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(testVar) == -1)
	//	return;
	

	//MODDD - this is required for some things to modify velocity, such as touch methods forcing the player off something's back.
	if(fApplyTempVelocity){
		fApplyTempVelocity = FALSE;
		pev->velocity = velocityApplyTemp;
	}


	//MODDD - new crate pushing system.  See if the modifier needs to be reset from a lack of signal to say it's still being pushed, since
	//        we seem to have no explicit "release" event.  It can simply be when you aren't pushing for a few frames.
	if(framesUntilPushStops >= 0){
		framesUntilPushStops--;
		if(framesUntilPushStops <= 0){
			//Ran out of push frames? Remove the influence from pushSpeedMulti (1 = no change).
			//Any sort of pushing, use'ing or physically touching, keeps "framesUntilPushStops" forced above 0. So this is a good release mechanism.
			framesUntilPushStops = -1;
			pushSpeedMulti = 1;
		}
	}

	/*
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(testVar)
	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(testVar) == 2){
		EASY_CVAR_SET_DEBUGONLY(testVar, 1);
		UTIL_printLineVector("MIN", pev->mins);
		UTIL_printLineVector("MAX", pev->maxs);

		MIN: (-16.00, -16.00, -36.00)
		MAX: (16.00, 16.00, 36.00)
	}
	*/


	//horrorSelected


	if(friendlyCheckTime != -1 && friendlyCheckTime <= gpGlobals->time){
		
		CBaseEntity *pEntity = NULL;
		CFriendly* closestFriendly = NULL;
		float closestDistanceYet = 3000;  //must be this close to care.
		while((pEntity = UTIL_FindEntityByClassname( pEntity, "monster_friendly" )) != NULL){

			if(!pEntity)continue;

			CFriendly* tempMrFriendly = static_cast<CFriendly*>(pEntity);

			if(!tempMrFriendly || !tempMrFriendly->pev->deadflag == DEAD_NO)continue;

			//MODDD NOTICE: do we need to do something to a friendly that used to be on but will no longer be on after this? Keep track of having horrorSelected TRUE at first?
			//if(tempMrFriendly->horrorSelected == TRUE){
			//    ...
		    //}


			if(tempMrFriendly->m_hEnemy!=NULL && tempMrFriendly->m_hEnemy->IsPlayer() == FALSE){
				tempMrFriendly->horrorSelected = FALSE;  //If targeting a non-player, this must be off.
				tempMrFriendly->stopHorrorSound();
			}

			
			float thisDist = (pEntity->pev->origin - pev->origin).Length();
			//easyForcePrintLine("OKAYYY??? %d %.2f", tempMrFriendly->monsterID, thisDist);
			//closest Friendly and is targeting this player?

			if(tempMrFriendly->m_hEnemy == this){
				tempMrFriendly->horrorSelected = FALSE;
			}

			if( thisDist < closestDistanceYet && tempMrFriendly->m_hEnemy!=NULL && tempMrFriendly->m_hEnemy == this && (tempMrFriendly->m_pSchedule != ::slSmallFlinch  ) ){
				tempMrFriendly->horrorSelected = FALSE;  //off for this player unless it is the closest.
				//tempMrFriendly->stopHorrorSound(); careful, this may be the closest one. we are unsure. don't interrupt the sound yet.
				closestFriendly = tempMrFriendly;
				closestDistanceYet = thisDist;
			}


		}//END OF while(friendly check)

		if(closestFriendly != NULL){
			//selected!
			closestFriendly->horrorSelected = TRUE;
			closestFriendlyMem = closestFriendly;
			closestFriendlyMemEHANDLE = closestFriendly;
		}else{
			closestFriendlyMem = NULL;
			closestFriendlyMemEHANDLE = NULL;
		}


		friendlyCheckTime = gpGlobals->time + 0.8;
	}


	if(closestFriendlyMemEHANDLE!=NULL && (pev->deadflag == DEAD_NO) && EASY_CVAR_GET_DEBUGONLY(friendlyPianoFollowVolume) > 0 ){

		if(horrorPlayTimePreDelay != -1 && horrorPlayTimePreDelay < gpGlobals->time){
			horrorPlayTimePreDelay = -1;
		}

		if(horrorPlayTimePreDelay == -1){

			if(horrorPlayTime == -1){
				horrorPlayTime = 0;
			}

			if(horrorPlayTime != -1 && horrorPlayTime < gpGlobals->time){
				//CHECK: only play if I'm the closest to the player? plus a general distance check too?
				horrorPlayTime = gpGlobals->time + 0.558;
				
				float tempVol = ( ((-(this->pev->origin - closestFriendlyMemEHANDLE->pev->origin).Length()) / 3000) + EASY_CVAR_GET_DEBUGONLY(friendlyPianoFollowVolume)+0.1 );

				if(tempVol < 0){
					//don't play? no point.
				}else{
					if(tempVol > EASY_CVAR_GET_DEBUGONLY(friendlyPianoFollowVolume) ){tempVol = EASY_CVAR_GET_DEBUGONLY(friendlyPianoFollowVolume);}

					//easyForcePrintLine("WHERE ELSE SUCKAH %.2f", tempVol);

					// ATTN_STATIC ?
					//UTIL_PlaySound( edict(), CHAN_VOICE, "friendly/friendly_horror.wav", 1.0, 1.8, 0, 100, TRUE );
					// Now with higher attenuation!  Want to play just for the intended target.
					// Attenuation is the number after the volume (tempVol).  Old attn: 1.8
					UTIL_PlaySound( this->edict(), CHAN_STATIC, "friendly/friendly_horror.wav", tempVol, 4.0, 0, 100, TRUE );
				}

			}
		}

	}//END OF closestFriendly check


	
	int buttonsChanged = (m_afButtonLast ^ pev->button);	// These buttons have changed this frame
	
	// Debounced button codes for pressed/released
	// UNDONE: Do we need auto-repeat?
	// MODDD NOTE - oh look, a way of telling whether a key's been pressed for the first time in a frame.
	//  Not used by anything at all in retail for weapons (IN_ATTACK and 2) beeeeeecccccaaaaauuuuussssseee???
	//  That crossbow "full second delay before rezooming because we can't tell if the user made a fresh click
	//  this frame" sure seems like a fat load of bullshit now doesn't it.
	// Anyway, this is also done in cl_dlls/hl/hl_weapons.cpp (clientside), so these can be trusted there too.
	m_afButtonPressed =  buttonsChanged & pev->button;		// The changed ones still down are "pressed"
	m_afButtonReleased = buttonsChanged & (~pev->button);	// The ones not down are "released"

	g_pGameRules->PlayerThink( this );

	if ( g_fGameOver )
		return;         // intermission or finale

	UTIL_MakeVectors(pev->v_angle);             // is this still used?
	
	ItemPreFrame( );

	WaterMove();

	
	if ( g_pGameRules && g_pGameRules->FAllowFlashlight() )
		m_iHideHUD &= ~HIDEHUD_FLASHLIGHT;
	else
		m_iHideHUD |= HIDEHUD_FLASHLIGHT;



	//MODDD - moved here, so that any damages forced to be removed by "-1" skill CVars never make it to the HUD.
	CheckTimeBasedDamage();

	// JOHN: checks if new client data (for HUD and view control) needs to be sent to the client
	UpdateClientData();
	
	//return;
	CheckSuitUpdate();


	
	if(currentSuitSoundEventTime != -1 && currentSuitSoundEventTime <= gpGlobals->time){
		//If waiting on an event and it is time to play it, do just that.
		currentSuitSoundEventTime = -1;
		
		//Call the local method, "eventMethod", provided.  That is horrendous.
		(this->*(currentSuitSoundEvent))();

	}//END OF currentSuitSoundEventTime check


	if(!fvoxEnabled && currentSuitSoundFVoxCutoff != -1 && currentSuitSoundFVoxCutoff <= gpGlobals->time){
		// If waiting to cut off this sound...
		currentSuitSoundFVoxCutoff = -1;

		int isentence = sentenceFVoxCutoffStop;

		if(isentence < 10000){
			char sentence[CBSENTENCENAME_MAX+1];
			strcpy(sentence, "!");
			strcat(sentence, gszallsentencenames[isentence]);

			STOP_SOUND_SUIT(ENT(pev), sentence);

		}else{
			//not supported (number readings are all or nothing).
		}

	}//END OF currentSuitSoundEventTime check



	//MODDD - new m_flFallVelocity set location
	if (!FBitSet(pev->flags, FL_ONGROUND))
	{
		m_flFallVelocity = -pev->velocity.z;
	}

	if (pev->deadflag >= DEAD_DYING)
	{
		PlayerDeathThink();
		return;
	}

	// So the correct flags get sent to client asap.
	//
	if ( m_afPhysicsFlags & PFLAG_ONTRAIN )
		pev->flags |= FL_ONTRAIN;
	else 
		pev->flags &= ~FL_ONTRAIN;

	// Train speed control
	if ( m_afPhysicsFlags & PFLAG_ONTRAIN )
	{
		CBaseEntity *pTrain = CBaseEntity::Instance( pev->groundentity );
		float vel;
		
		if ( !pTrain )
		{
			TraceResult trainTrace;
			// Maybe this is on the other side of a level transition
			UTIL_TraceLine( pev->origin, pev->origin + Vector(0,0,-38), ignore_monsters, ENT(pev), &trainTrace );

			// HACKHACK - Just look for the func_tracktrain classname
			if ( trainTrace.flFraction != 1.0 && trainTrace.pHit )
			pTrain = CBaseEntity::Instance( trainTrace.pHit );


			if ( !pTrain || !(pTrain->ObjectCaps() & FCAP_DIRECTIONAL_USE) || !pTrain->OnControls(pev) )
			{
				//ALERT( at_error, "In train mode with no train!\n" );
				m_afPhysicsFlags &= ~PFLAG_ONTRAIN;
				m_iTrain = TRAIN_NEW|TRAIN_OFF;
				return;
			}
		}
		else if ( !FBitSet( pev->flags, FL_ONGROUND ) || FBitSet( pTrain->pev->spawnflags, SF_TRACKTRAIN_NOCONTROL ) || (pev->button & (IN_MOVELEFT|IN_MOVERIGHT) ) )
		{
			// Turn off the train if you jump, strafe, or the train controls go dead
			m_afPhysicsFlags &= ~PFLAG_ONTRAIN;
			m_iTrain = TRAIN_NEW|TRAIN_OFF;
			return;
		}

		pev->velocity = g_vecZero;
		vel = 0;
		if ( m_afButtonPressed & IN_FORWARD )
		{
			vel = 1;
			pTrain->Use( this, this, USE_SET, (float)vel );
		}
		else if ( m_afButtonPressed & IN_BACK )
		{
			vel = -1;
			pTrain->Use( this, this, USE_SET, (float)vel );
		}

		//MODDD - now also allows a pTrain->pev->iuser1 value of 1 to trigger a UI update.
		if (vel || pTrain->pev->iuser1 == 1)
		{
			m_iTrain = TrainSpeed(pTrain->pev->speed, pTrain->pev->impulse);
			m_iTrain |= TRAIN_ACTIVE|TRAIN_NEW;
			pTrain->pev->iuser1 = 0;
		}

	} else if (m_iTrain & TRAIN_ACTIVE)
		m_iTrain = TRAIN_NEW; // turn off train


	
	float timeDelta;

	if(oldThinkTime != -1){
		timeDelta = gpGlobals->time - oldThinkTime;
	}else{
		timeDelta = 0;
	}
	//timeDelta = gpGlobals->frametime;

	oldThinkTime = gpGlobals->time;

	
	if(m_fLongJump){
#if LONGJUMPUSESDELAY == 0
		// continual re-charge for infinigeLongJumpCharge choices of 2 and 3.
		if (longJumpCharge < PLAYER_LONGJUMPCHARGE_MAX) {
			if (EASY_CVAR_GET(sv_longjump_chargemode) == 2) {
				// every 2 minutes, one additional longjump
				longJumpCharge += timeDelta * (25.0f/120.0f);
			}
			else if (EASY_CVAR_GET(sv_longjump_chargemode) == 3) {
				// every 30 seconds, one additional longjump
				longJumpCharge += timeDelta * (25.0f/30.0f);
			}

			if (longJumpCharge > PLAYER_LONGJUMPCHARGE_MAX) {
				longJumpCharge = PLAYER_LONGJUMPCHARGE_MAX;
			}
		}//END OF longJumpCharge check
#endif

		if((EASY_CVAR_GET_DEBUGONLY(itemBatteryPrerequisite) == 0 || pev->armorvalue > 0 )){

#if LONGJUMPUSESDELAY == 1

		if ((pev->button & IN_DUCK) &&
			( pev->flDuckTime >= 0 ) &&
			//pev->velocity.Length() > 0 &&
			//!pev->button & IN_JUMP &&
			FBitSet ( pev->flags, FL_ONGROUND )

			)
		{
			longJumpChargeNeedsUpdate = TRUE;
		
			if(longJumpCharge != PLAYER_LONGJUMPCHARGE_MAX){
				longJumpCharge += timeDelta;
				//easyPrint("YAY timedelta: %.3f\n", timeDelta);
				if(longJumpCharge >= PLAYER_LONGJUMPCHARGE_MAX ){
					longJumpCharge = PLAYER_LONGJUMPCHARGE_MAX;
					g_engfuncs.pfnSetPhysicsKeyValue( edict(), "slj", "1" );	
				}

			}else{
				//easyPrint("EH timedelta: %.3f\n", timeDelta);
			}

			/*
			easyPrint("timedelta: %.3f\n", timeDelta);
			easyPrint("old think time: %.3f\n", oldThinkTime);
			easyPrint("THE JumP is %.3f\n", longJumpCharge);
			*/

		}else{
			if(longJumpCharge != 0){
				g_engfuncs.pfnSetPhysicsKeyValue( edict(), "slj", "0" );
				longJumpCharge = 0;
				longJumpChargeNeedsUpdate = TRUE;
			}

		}

#else
		//long jump uses limited charges + delay b/w successive use.

		if(longJumpDelay > 0){
			longJumpDelay -= timeDelta;
		}


		if ((longJumpDelay <= 0 && (EASY_CVAR_GET(sv_longjump_chargemode) == 1 || longJumpCharge >= LONGJUMP_CHARGEUSE) && pev->button & IN_DUCK) &&
			( pev->flDuckTime >= 0 ) &&
			//pev->velocity.Length() > 0 &&
			//!pev->button & IN_JUMP &&
			FBitSet ( pev->flags, FL_ONGROUND )
			
			)
		{
			
			//easyPrint("YAY timedelta: %.3f\n", timeDelta);
			g_engfuncs.pfnSetPhysicsKeyValue( edict(), "slj", "1" );
			longJump_waitForRelease = TRUE;

			//Store the player's velocity length (just "speed") to "lastDuckVelocityLength", since any other references
			//to current velocity length after this will just refer to the velocity of a jump.  As in, of course there was movement,
			//I care if the duck jump is considered stationary (no movement in any direction, no long jump).
			lastDuckVelocityLength = pev->velocity.Length();

			/*
			easyPrint("timedelta: %.3f\n", timeDelta);
			easyPrint("old think time: %.3f\n", oldThinkTime);
			easyPrint("THE JumP is %.3f\n", longJumpCharge);
			*/

		}else{
			
			if(longJump_waitForRelease){

				//The player must have just jumped, and the velocity's "length" (speed as a single number) must be over 7, a requirement
				//of pm_shared.c for doing the superjump.

				if(pev->oldbuttons & IN_JUMP && lastDuckVelocityLength > 7 ){
					//easyPrint("pev->velocity? %.3f\n", lastDuckVelocityLength);

					if(EASY_CVAR_GET(sv_longjump_chargemode) != 1){
						longJumpCharge -= LONGJUMP_CHARGEUSE;
					}

					longJumpDelay = PLAYER_LONGJUMP_DELAY;
					longJumpChargeNeedsUpdate = TRUE;
					
					// that is, if the charge has run out BUT "sv_longjump_chargemode" is 1, don't play this message (spammy)
					// And if regeneration is on, that's covered too.  Point of the message is to let the player know
					// long-jumping after this one isn't immediately possible (having 4 charge left with 25 required
					// may not be 0 charge left, but should clearly still play this message or else it virtually never will)
					if(longJumpCharge < LONGJUMP_CHARGEUSE && EASY_CVAR_GET(sv_longjump_chargemode) != 1){
						//play the out of ammo message.
						SetSuitUpdate("!HEV_LJDEPLETED", FALSE, 0);
					}
				}


				longJump_waitForRelease = FALSE;
				g_engfuncs.pfnSetPhysicsKeyValue( edict(), "slj", "0" );
				
			}
		}

#endif

		}//END OF if((EASY_CVAR_GET_DEBUGONLY(itemBatteryPrerequisite) == 0 || pev->armorvalue > 0 ))

	}//END OF if(m_fLongJump)


	if (pev->button & IN_JUMP)
	{
		// If on a ladder, jump off the ladder
		// else Jump
		Jump();
	}

	// If trying to duck, already ducked, or in the process of ducking
	if ((pev->button & IN_DUCK) || FBitSet(pev->flags, FL_DUCKING) || (m_afPhysicsFlags & PFLAG_DUCKING)) {
		Duck();
	}

	
	/*
	if ( m_fLongJump &&
		(pev->button & IN_DUCK) &&
		( pev->flDuckTime > 0 ) &&
		pev->velocity.Length() > 0 &&
		!pev->button & IN_JUMP &&
		FBitSet ( pev->flags, FL_ONGROUND )
		)

		*/
	//USED TO DO SOME LONG JUMP LOGIC HERE.  Now doing it before jump instead.
	
	//MODDD - old m_flFallVelocity set location


	// StudioFrameAdvance( );//!!!HACKHACK!!! Can't be hit by traceline when not animating?

	// Clear out ladder pointer
	m_hEnemy = NULL;

	if ( m_afPhysicsFlags & PFLAG_ONBARNACLE )
	{
		pev->velocity = g_vecZero;
	}

}//END OF PreThink.







float CBasePlayer::TimedDamageBuddhaFilter(float dmgIntent) {

	if (dmgIntent >= pev->health && gSkillData.tdmg_playerbuddha == 1 && !recentTimedTriggerDamage) {
		dmgIntent = pev->health - 1;
		if (dmgIntent < 0) dmgIntent = 0;  // no healing from this!
	}

	return dmgIntent;
}


// at the end of a frame, if a monster has 1 health and buddha mode, cancel the timed damage.
void CBasePlayer::TimedDamagePostBuddhaCheck(void) {
	
	if (pev->health <= 1 && gSkillData.tdmg_playerbuddha == 1 && !recentTimedTriggerDamage) {

		// show on the UI another frame anyway
		m_bitsDamageTypeForceShow |= m_bitsDamageType;
		m_bitsDamageTypeModForceShow |= m_bitsDamageTypeMod;

		// ok, don't allow another frame.
		attemptResetTimedDamage(TRUE);
	}

	recentTimedTriggerDamage = FALSE;
	//m_bitsDamageTypeMod &= ~DMG_MAP_TRIGGER;
	recentMajorTriggerDamage &= ~4;
}


void CBasePlayer::removeTimedDamageImmediate(int arg_type, int* m_bitsDamageTypeRef, BYTE bDuration) {
	// in addition to what any monster does (actually remove the timed damage)...
	CBaseMonster::removeTimedDamageImmediate(arg_type, m_bitsDamageTypeRef, bDuration);

	// Is the duration given 0?  A duration of 255 in this case (-1) means it doesn't
	// even make it to the HUD.
	if (bDuration == 0) {
		int damageBit = convert_itbd_to_damage(arg_type);
		// still let the damage indicator icon show up.
		if (arg_type < itbd_BITMASK2_FIRST) {
			m_bitsDamageTypeForceShow |= damageBit;
		}
		else {
			m_bitsDamageTypeModForceShow |= damageBit;
		}
	}
}


BYTE CBasePlayer::parse_itbd_duration(int i) {

	switch (i)
	{
	//NOTE - PLAYER ONLY.
	case itbd_DrownRecover:
		return 4;	// get up to 5*10 = 50 points back

	case itbd_Bleeding:
		return ((gSkillData.tdmg_bleeding_duration >= 0) ? ((BYTE)gSkillData.tdmg_bleeding_duration) : 255);
	default:
		// Unhandled?  Let the monster class handle it instead
		return CBaseMonster::parse_itbd_duration(i);
	}

}//parse_itbd_duration



//MODDD - CheckTimeBasedDamage moved to basemonster.h/.cpp.
// Instead, player.cpp will override certain parts of CheckTimeBasedDamage (below) to
// do extra things or some things differently in key areas.  Otherwise, most logic
// now applies to all monsters too.

void CBasePlayer::parse_itbd(int i) {
	// In short, the point is to work with types unsupported by the base monster (itbd_DrownRecover)
	// or override how existing ones work (bleeding).

	int damageType = 0;
	damageType = DMG_TIMEDEFFECT;

	switch (i){
	//NOTE - PLAYER ONLY.
	case itbd_DrownRecover: {
		// NOTE: this hack is actually used to RESTORE health
		// after the player has been drowning and finally takes a breath
		if (m_idrowndmg > m_idrownrestored)
		{
			int idif = min(m_idrowndmg - m_idrownrestored, 10);

			TakeHealth(idif, DMG_GENERIC);
			m_idrownrestored += idif;
		}
	}break;
	// Overwriting this one compltely to use UTIL_MakeVectors instead of UTIL_MakeAimVectors.
	// Yes, the player uses MakeVectors and monsters use MakeAimVectors.   Go.   Figure.
	// ONLY for pev->v_angle, which only the player uses!   MakeAimVectors is needed for player pev->angles,
	// but that's not being used here (Make_Vectors choices are interchangeable for monster pev->angles,
	// always 0 pitch).
	case itbd_Bleeding: {

		// this will always ignore the armor (hence DMG_TIMEDEFFECT).
		TakeDamage(pev, pev, TimedDamageBuddhaFilter(BLEEDING_DAMAGE), 0, damageType | DMG_TIMEDEFFECTIGNORE);

		UTIL_MakeVectors(pev->v_angle + pev->punchangle);
		//pev->origin + pev->view_ofs
		//BodyTargetMod(g_vecZero)
		// BLEEDING_DAMAGE
		UTIL_SpawnBlood(
			pev->origin + pev->view_ofs + gpGlobals->v_forward * RANDOM_FLOAT(9, 13) + gpGlobals->v_right * RANDOM_FLOAT(-5, 5) + gpGlobals->v_up * RANDOM_FLOAT(4, 7),
			BloodColor(),
			RANDOM_LONG(8, 15)
		);
	}break;
	default:
		// Unhandled?  Let the monster class handle it instead
		CBaseMonster::parse_itbd(i);
	break;
	}//switch(i)

}//END OF parse_itbd


void CBasePlayer::timedDamage_nonFirstFrame(int i, int* m_bitsDamageTypeRef) {
	// use up an antitoxin on poison or nervegas after a few seconds of damage
	// MODDD - instead of referring to constants like "NERVEGASDURATION", it is referring to the
	// variable "nervegasDuration", which is set according to difficulty.  Same for poison.
	if (
		((i == itbd_NerveGas) && (m_rgbTimeBasedDamage[i] < gSkillData.tdmg_nervegas_duration)) ||
		((i == itbd_Poison) && (m_rgbTimeBasedDamage[i] < gSkillData.tdmg_poison_duration))
	)
	{
		if (!antidoteQueued && m_rgItems[ITEM_ANTIDOTE] && (EASY_CVAR_GET_DEBUGONLY(itemBatteryPrerequisite) == 0 || pev->armorvalue > 0))
		{
			antidoteQueued = TRUE;
			//not yet!  Wait for the hissing sound.
			//m_rgbTimeBasedDamage[i] = 0;
			//m_rgItems[ITEM_ANTIDOTE]--;

			//MODDD - this used to refer to "HEV_HEAL4".  "HEV_HEAL5" refers to an antidote,
			//HEAL4, re-used below for the radiation item (power canister / syringe), refers to "anti-toxins".
			//SetSuitUpdateFVoxlessFriendlyEvent("!HEV_ANT_USE", FALSE, SUIT_REPEAT_OK, -1, -2, consumeAntidote);

			SetSuitUpdateEventFVoxCutoff("!HEV_ANT_USE", FALSE, SUIT_REPEAT_OK, SUITUPDATETIME, TRUE, 1.36, &CBasePlayer::consumeAntidote, 1.36 + 0.55);
		}
	}
	//MODDD - for the radiation instead.
	if (((i == itbd_Radiation) && (m_rgbTimeBasedDamage[i] < gSkillData.tdmg_radiation_duration)))
	{
		if (!radiationQueued && m_rgItems[ITEM_RADIATION] && (EASY_CVAR_GET_DEBUGONLY(itemBatteryPrerequisite) == 0 || pev->armorvalue > 0))
		{
			radiationQueued = TRUE;

			//m_rgbTimeBasedDamage[i] = 0;
			//m_rgItems[ITEM_RADIATION]--;
			//SetSuitUpdate("!HEV_RAD_USE", FALSE, SUIT_REPEAT_OK);

			SetSuitUpdateEventFVoxCutoff("!HEV_RAD_USE", FALSE, SUIT_REPEAT_OK, SUITUPDATETIME, TRUE, 1.28, &CBasePlayer::consumeRadiation, 1.28 + 0.55);
		}
	}

	// and do the usual logic
	CBaseMonster::timedDamage_nonFirstFrame(i, m_bitsDamageTypeRef);
}






// if in range of radiation source, ping geiger counter

int CBasePlayer::getGeigerChannel(void){
	return EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(geigerChannel);
}


void CBasePlayer::UpdateGeigerCounter( void )
{
	//only the server is calling this.
	
	//can run into a "total_channels == MAX_CHANNEL" printout from the engine if too many sounds are queued up while the game is paused.
	//This issue should be fixed now.


	//easyForcePrintLine("WHAT THE hekkin hek %.8f %.8f d:%.8f %d", gpGlobals->time, previousFrameTime, (gpGlobals->time - previousFrameTime), frameElapsed);
	//easyForcePrintLine("WELL???!!! %d", frameElapsed);

	BYTE range;

	// delay per update ie: don't flood net with these msgs
	//MODDD - the geiger sound should still attempt to play each frame when in range, like it did in the client.
	//if (gpGlobals->time < m_flgeigerDelay)
	//	return;
	if (gpGlobals->time >= m_flgeigerDelay){

	m_flgeigerDelay = gpGlobals->time + GEIGERDELAY;
		
	// send range to radition source to client

	range = (BYTE) (m_flgeigerRange / 4);


	if (range != m_igeigerRangePrev)
	{
		m_igeigerRangePrev = range;

		MESSAGE_BEGIN( MSG_ONE, gmsgGeigerRange, NULL, pev );
			WRITE_BYTE( range );
		MESSAGE_END();
	}


	// reset counter and semaphore
	if (!RANDOM_LONG(0,3))
		m_flgeigerRange = 1000;

	//MODDD - sound-related clientside geiger.cpp script moved here for better control.

	}//END OF if (gpGlobals->time < m_flgeigerDelay)


	int pct;
	float flvol;

	//MODDD NOTE - uh.  What does 'rg' even do???
	float rg[3];
	int i;


	//MODDD - around the time we start hearing geiger sounds, play "radiation detected".
	if(m_flgeigerRange < 800){
		if(!foundRadiation){
			SetSuitUpdate("!HEV_DET2", FALSE, SUIT_NEXT_IN_1MIN);	// radiation detected
		}else{
			forceRepeatBlock("!HEV_DET2", FALSE, SUIT_NEXT_IN_1MIN);
			//after leaving radiation, the use
		}
		foundRadiation = TRUE;
	}//END OF if(m_flgeigerRange < 800)
	else{
		//mark to let the suit's timer cool down.
		foundRadiation = FALSE;
	}


	if (m_flgeigerRange < 1000 && m_flgeigerRange > 0)
	{

		//MODDD - this has been moved to player.cpp for better control over channels.
		//(see the method "UpdateGeigerCounter").
		
		// peicewise linear is better than continuous formula for this
		if (m_flgeigerRange > 800)
		{
			pct = 0;			//Con_Printf ( "range > 800\n");
		}
		else if (m_flgeigerRange > 600)
		{
			pct = 2;
			flvol = 0.4;		//Con_Printf ( "range > 600\n");
			rg[0] = 1;
			rg[1] = 1;
			i = 2;
		}
		else if (m_flgeigerRange > 500)
		{
			pct = 4;
			flvol = 0.5;		//Con_Printf ( "range > 500\n");
			rg[0] = 1;
			rg[1] = 2;
			i = 2;
		}
		else if (m_flgeigerRange > 400)
		{
			pct = 8;
			flvol = 0.6;		//Con_Printf ( "range > 400\n");
			rg[0] = 1;
			rg[1] = 2;
			rg[2] = 3;
			i = 3;
		}
		else if (m_flgeigerRange > 300)
		{
			pct = 8;
			flvol = 0.7;		//Con_Printf ( "range > 300\n");
			rg[0] = 2;
			rg[1] = 3;
			rg[2] = 4;
			i = 3;
		}
		else if (m_flgeigerRange > 200)
		{
			pct = 28;
			flvol = 0.78;		//Con_Printf ( "range > 200\n");
			rg[0] = 2;
			rg[1] = 3;
			rg[2] = 4;
			i = 3;
		}
		else if (m_flgeigerRange > 150)
		{
			pct = 40;
			flvol = 0.80;		//Con_Printf ( "range > 150\n");
			rg[0] = 3;
			rg[1] = 4;
			rg[2] = 5;
			i = 3;
		}
		else if (m_flgeigerRange > 100)
		{
			pct = 60;
			flvol = 0.85;		//Con_Printf ( "range > 100\n");
			rg[0] = 3;
			rg[1] = 4;
			rg[2] = 5;
			i = 3;
		}
		else if (m_flgeigerRange > 75)
		{
			pct = 80;
			flvol = 0.9;		//Con_Printf ( "range > 75\n");
			//gflGeigerDelay = cl.time + GEIGERDELAY * 0.75;
			rg[0] = 4;
			rg[1] = 5;
			rg[2] = 6;
			i = 3;
		}
		else if (m_flgeigerRange > 50)
		{
			pct = 90;
			flvol = 0.95;		//Con_Printf ( "range > 50\n");
			rg[0] = 5;
			rg[1] = 6;
			i = 2;
		}
		else
		{
			pct = 95;
			flvol = 1.0;		//Con_Printf ( "range < 50\n");
			rg[0] = 5;
			rg[1] = 6;
			i = 2;
		}

		
		flvol = (flvol * ((rand() & 127)) / 255) + 0.25; // UTIL_RandomFloat(0.25, 0.5);

		if ((rand() & 127) < pct || (rand() & 127) < pct)
		{
			//S_StartDynamicSound (-1, 0, rgsfx[rand() % i], r_origin, flvol, 1.0, 0, 100);	
			char sz[256];
			
			int j = rand() & 1;
			if (i > 2)
				j += rand() & 1;


			sprintf(sz, "player/geiger%d.wav", j + 1);
			//EMIT_SOUND(ENT(pev), CHAN_ITEM, sz, flvol, 0.05f);

			//prrrr
			Vector headPos;

			headPos.x = pev->origin.x + pev->view_ofs.x;
			headPos.y = pev->origin.y + pev->view_ofs.y;
			headPos.z = pev->origin.z + pev->view_ofs.z;

			//ATTN_NORM is used for the suit?  I thought it would be "ATTN_STATIC".
			//Should the suit not use CHAN_STATIC if in multiplayer?  Other comments here warn against using "CHAN_STATIC" in multiplayer.
			
			//MODDD - soundsentencesave... CANCELED
			UTIL_PlaySound(ENT(pev), getGeigerChannel(), sz, flvol, ATTN_NORM, 0, 100, FALSE);

			//UTIL_EmitAmbientSound(ENT(pev), headPos, sz, flvol, ATTN_STATIC, 0, 100, FALSE);

			//sprintf(sz, "player/geiger%d.wav", j + 1);
			//PlaySound(sz, flvol);
		}
	}
	//easyForcePrintLine("DO I BUCKO %d", usesSoundSentenceSave() );
}//END OF UpdateGeigerCounter

/*
================
CheckSuitUpdate

Play suit update if it's time
================
*/
void CBasePlayer::CheckSuitUpdate()
{
	int i;
	int isentence = 0;
	int isearch = m_iSuitPlayNext;
	
	// Ignore suit updates if no suit
	if ( !(pev->weapons & (1<<WEAPON_SUIT)) )
		return;

	// if in range of radiation source, ping geiger counter
	UpdateGeigerCounter();


	//MODDD - NOTE: MULTIPLAYER FVOX BLOCKER
	// UNWISE TO STOP THIS CRUDELY!!!  Some FVOX messages carry delays for using injectables and need this logic to run to ever be seen and called,
	// even if the FVOX voice never plays.
	/*
	if ( IsMultiplayer() )
	{
		// don't bother updating HEV voice in multiplayer.
		return;
	}
	*/

	if ( gpGlobals->time >= m_flSuitUpdate && m_flSuitUpdate > 0)
	{
		//MODDD - this will force going to another custom clip if given.
		if(obligedCustomSentence == 0){

			// play a sentence off of the end of the queue
			for (i = 0; i < CSUITPLAYLIST; i++)
			{
				isentence = m_rgSuitPlayList[isearch];
				if (isentence != 0)
					break;
			
				if (++isearch == CSUITPLAYLIST)
					isearch = 0;
			}


			if(isentence >= 10000){
				// playing a real-time battery speech.
				batterySayPhase = 0;
			}else{
				// non-battery related messages will reset "recentlySaidBattery".
				recentlySaidBattery = -1;
			}

		}else{
			isentence = obligedCustomSentence;
			//reset, needs to be another "obligedCustomSentence" to happen again.
			obligedCustomSentence = 0;
		}

		
		//COMMENTED OUT, annoying fast.
		//easyPrintLine ("WHAT IS SENTENCE %d", isentence );
		if (isentence)
		{
			m_rgSuitPlayList[isearch] = 0;

			//Clearly "isearch" is the picked entry to play.  Get the other info needed and then clear it too.
			if(m_rgSuitPlayListEventDelay[isearch] != -1){
				currentSuitSoundEventTime = gpGlobals->time + m_rgSuitPlayListEventDelay[isearch];
				currentSuitSoundEvent = m_rgSuitPlayListEvent[isearch];
			}else{
				currentSuitSoundEventTime = -1;
				currentSuitSoundEvent = NULL;
			}

			if(!fvoxEnabled && m_rgSuitPlayListFVoxCutoff[isearch] != -1){
				currentSuitSoundFVoxCutoff = gpGlobals->time + m_rgSuitPlayListFVoxCutoff[isearch];
				sentenceFVoxCutoffStop = isentence;
			}else{
				currentSuitSoundFVoxCutoff = -1;
				sentenceFVoxCutoffStop = -1;
			}
			
			m_rgSuitPlayListEventDelay[isearch] = -1;
			m_rgSuitPlayListEvent[isearch] = NULL;


			if (isentence > 0)
			{
				// play sentence number
				//MODDD - if the sentence number is less than 10,000, it is an ordinary sentence.
				//Play as usual.
				if(isentence < 10000){
					char sentence[CBSENTENCENAME_MAX+1];
					strcpy(sentence, "!");
					strcat(sentence, gszallsentencenames[isentence]);
					EMIT_SOUND_SUIT(ENT(pev), sentence);
					//STOP_SOUND_SUIT

					strcpy(recentlyPlayedSound, "!");
					strcat(recentlyPlayedSound, gszallsentencenames[isentence]);


					m_flSuitUpdate = gpGlobals->time + m_rgSuitPlayListDuration[isearch];

				}else{
					//otherwise, this is something custom:
					int determiner = isentence - 10000;
					float timeToSay = 0.8f;

					
					if(getBatteryValueRealTime && batterySayPhase < 1){
						isentence = 10000 + pev->armorvalue;
						determiner = isentence - 10000;
					}


					if(batterySayPhase == 0){
						//use the current battery value.
						//isentence = 10000 + pev->armorvalue;

						//First, does a recent battery reading match what we will say now (like picking up 2
						//batteries and taking no damage between the time to play those messages)?
						//If so, don't play this number.
						if(recentlySaidBattery == determiner){
							m_flSuitUpdate = gpGlobals->time + timeToSay;
							batterySayPhase = -1;
							return;
						}

						recentlySaidBattery = determiner;  //do not repeat this immediately.

						if(determiner != 0){
							//say the first part (power is) and end later (percent).
							EMIT_SOUND_SUIT(ENT(pev), "!HEV_BNOTICE");
							timeToSay = 0.68f;

							obligedCustomSentence = isentence;
							determiner = 0;

							m_flSuitUpdate = gpGlobals->time + timeToSay;
							batterySayPhase++;
							return;	
						}

						/*
						if(batteryInitiative){
							
							
						}else{
							determiner = isentence - 10000;
						}
						*/
					}else if(batterySayPhase == 2){

						EMIT_SOUND_SUIT(ENT(pev), "!HEV_BPERCENT");
						timeToSay = 0.77f;
						m_flSuitUpdate = gpGlobals->time + timeToSay;
						batterySayPhase = -1;
						return;	
					}
					

					if(determiner >= 10 && timeToSay < 20){
						//extra time for the teens (and 11, 12):
						timeToSay = 1.12f;
					}
					if(determiner >= 20){
						timeToSay = 0.9f;
					}

					//EMIT_SOUND_SUIT(ENT(pev), sentence);


					switch(determiner){
						//NOTE: case 0 should be handled separately (HEV_NOPOWER).
					
					case 0:
						EMIT_SOUND_SUIT(ENT(pev), "!HEV_NOPOWER");
						timeToSay = 1.9f;
						batterySayPhase = -1;
						//do not follow up with "percent" if that was planned.
					break;
					case 1:
						//EMIT_SOUND_SUIT(ENT(pev), "fvox/one.wav");
						EMIT_SOUND_SUIT(ENT(pev), "!HEV_numb1");
						timeToSay = 0.77f;
					break;
					case 2:
						EMIT_SOUND_SUIT(ENT(pev), "!HEV_numb2");
						timeToSay = 0.75f;
					break;
					case 3:
						EMIT_SOUND_SUIT(ENT(pev), "!HEV_numb3");
						timeToSay = 0.83f;
					break;
					case 4:
						EMIT_SOUND_SUIT(ENT(pev), "!HEV_numb4");
						timeToSay = 0.85f;
					break;
					case 5:
						EMIT_SOUND_SUIT(ENT(pev), "!HEV_numb5");
						timeToSay = 0.99f;
					break;
					case 6:
						EMIT_SOUND_SUIT(ENT(pev), "!HEV_numb6");
						timeToSay = 0.90f;
					break;
					case 7:
						EMIT_SOUND_SUIT(ENT(pev), "!HEV_numb7");
						timeToSay = 0.83f;
					break;
					case 8:
						EMIT_SOUND_SUIT(ENT(pev), "!HEV_numb8");
						timeToSay = 0.75f;
					break;
					case 9:
						EMIT_SOUND_SUIT(ENT(pev), "!HEV_numb9");
						timeToSay = 0.86f;
					break;
					case 10:
						EMIT_SOUND_SUIT(ENT(pev), "!HEV_numb10");
						timeToSay = 0.74f;
					break;
					case 11:
						EMIT_SOUND_SUIT(ENT(pev), "!HEV_numb11");
						timeToSay = 0.88f;
					break;
					case 12:
						EMIT_SOUND_SUIT(ENT(pev), "!HEV_numb12");
						timeToSay = 0.88f;
					break;
					case 13:
						EMIT_SOUND_SUIT(ENT(pev), "!HEV_numb13");
						timeToSay = 1.13f;

					break;
					case 14:
						EMIT_SOUND_SUIT(ENT(pev), "!HEV_numb14");
						timeToSay = 1.10f;
					break;
					case 15:
						EMIT_SOUND_SUIT(ENT(pev), "!HEV_numb15");
						timeToSay = 1.10f;
					break;
					case 16:
						EMIT_SOUND_SUIT(ENT(pev), "!HEV_numb16");
						timeToSay = 1.11f;
					break;
					case 17:
						EMIT_SOUND_SUIT(ENT(pev), "!HEV_numb17");
						timeToSay = 1.16f;
					break;
					case 18:
						EMIT_SOUND_SUIT(ENT(pev), "!HEV_numb18");
						timeToSay = 0.84f;
					break;
					case 19:
						EMIT_SOUND_SUIT(ENT(pev), "!HEV_numb19");
						timeToSay = 0.85f;
					break;
					default:

						if(determiner >= 20 && determiner < 30){
							EMIT_SOUND_SUIT(ENT(pev), "!HEV_numb20");
							obligedCustomSentence = isentence - 20;
							timeToSay = 0.79f;
						}else if(determiner >= 30 && determiner < 40){
							EMIT_SOUND_SUIT(ENT(pev), "!HEV_numb30");
							obligedCustomSentence = isentence - 30;
							timeToSay = 0.85f;
						}else if(determiner >= 40 && determiner < 50){
							EMIT_SOUND_SUIT(ENT(pev), "!HEV_numb40");
							obligedCustomSentence = isentence - 40;
							timeToSay = 0.88f;
						}else if(determiner >= 50 && determiner < 60){
							EMIT_SOUND_SUIT(ENT(pev), "!HEV_numb50");
							obligedCustomSentence = isentence - 50;
							timeToSay = 0.89f;
						}else if(determiner >= 60 && determiner < 70){
							EMIT_SOUND_SUIT(ENT(pev), "!HEV_numb60");
							obligedCustomSentence = isentence - 60;
							timeToSay = 1.00f;
						}else if(determiner >= 70 && determiner < 80){
							EMIT_SOUND_SUIT(ENT(pev), "!HEV_numb70");
							obligedCustomSentence = isentence - 70;
							timeToSay = 0.98;
						}else if(determiner >= 80 && determiner < 90){
							EMIT_SOUND_SUIT(ENT(pev), "!HEV_numb80");
							obligedCustomSentence = isentence - 80;
							timeToSay = 0.76f;
						}else if(determiner >= 90 && determiner < 100){
							EMIT_SOUND_SUIT(ENT(pev), "!HEV_numb90");
							obligedCustomSentence = isentence - 90;
							timeToSay = 0.91f;
						}else if(determiner == 100){
							
							EMIT_SOUND_SUIT(ENT(pev), "!HEV_numb100");
							obligedCustomSentence = isentence - 100;
							timeToSay = 0.96f;
						}


						if(obligedCustomSentence == 10000){
							obligedCustomSentence = 0;
							//why?  because "obligedCustomSentence" being 10,000 means saying "zero".
							//You do not say "twenty zero" or "fourty zero" to denote a zero place in the one's
							//position when saying a word.  It's just "twenty" or "fourty", so forcing
							//"obligedCustomSentence" to 0 achieves this: don't queue anything after saying the
							//ten's place.

							if(determiner >= 20 && determiner <= 100){
								//add an extra pause after this if not followed by anything.
								timeToSay += 0.10f;
							}

						}
						
						
					break;

					}//END OF switch(determiner)

					if(batterySayPhase == 1 && obligedCustomSentence == 0 ){   //determiner > 0 && determiner != 5001){
						//if not 0, 

						obligedCustomSentence = 10000;
						batterySayPhase++;
						//queue saying "percent".

					}

					//NO, the suitPlayDuration will be ignored for these custom calls.
					//Standardize it or customize it above.
					//m_flSuitUpdate = gpGlobals->time + m_rgSuitPlayListDuration[isearch];
					m_flSuitUpdate = gpGlobals->time + timeToSay;
					
				}//END OF else OF if(isentence < 10000)
				//EMIT_SOUND_SUIT(ENT(pev), "scientist/scream25.wav");
				//EMIT_SOUND_DYN(ENT(pev), CHAN_STATIC, "scientist/scream25.wav", CVAR_GET_FLOAT("suitvolume"), ATTN_NORM, 0, 100);

			}
			else
			{
				// play sentence group
				EMIT_GROUPID_SUIT(ENT(pev), -isentence);

				m_flSuitUpdate = gpGlobals->time + m_rgSuitPlayListDuration[isearch];
			}



			//m_iSuitPlayNext
		//m_flSuitUpdate = gpGlobals->time + SUITUPDATETIME;
		//NOTICE: line moved above, per the "if-else" scopes for more control each time.
		//m_flSuitUpdate = gpGlobals->time + m_rgSuitPlayListDuration[isearch];
		}
		else {
			// queue is empty, don't check 
			m_flSuitUpdate = 0;
		}
	}
}


BOOL CBasePlayer::SetSuitUpdatePRE(){
	return SetSuitUpdatePRE(FALSE);
}//END OF SetSuitUpdatePRE(...)

//MODDD - new method to contain things commonly repeated throughout "SetSuitUpdate" method clones before the action.
// Returns a boolean (true/false) whether this request is allowed to happen or blocked.
BOOL CBasePlayer::SetSuitUpdatePRE(BOOL fvoxException ){
	
	// Ignore suit updates if no suit

	if ( !(pev->weapons & (1<<WEAPON_SUIT))  )
		return FALSE;

	//MODDD - NOTE: MULTIPLAYER FVOX BLOCKER
	// Check here disabled, merged into the "fvoxEnabled" check below.
	/*
	if ( IsMultiplayer() )
	{
		// due to static channel design, etc. We don't play HEV sounds in multiplayer right now.
		return FALSE;
	}
	*/

	//MODDD - also don't play if FVOX is no longer "enabled" and this is NOT the notification to turn it on / off (exceptions).
	//if(fvoxEnabled == 0 && !(name == "!HEV_V0" || name == "!HEV_V1")  ){
	// removed the multiplayer check!    IsMultiplayer() ||
	if( ( fvoxEnabled == 0 || globalflag_muteDeploySound==TRUE) && !fvoxException){
		return FALSE;
	}

	//made it here? sounds ok.
	return TRUE;
}//END OF SetSuitUpdatePRE(...)

//MODDD - assume this is not an exception to the "fvoxEnabled" setting (whether to play FVox sounds or not)
BOOL CBasePlayer::SetSuitUpdatePRE(char *name, int fgroup, int& isentence ){
	return SetSuitUpdatePRE(name, fgroup, isentence, FALSE);
}

//MODDD - new method to contain things commonly repeated throughout "SetSuitUpdate" method clones before the action.
// Returns a boolean (true/false) whether this request is allowed to happen or blocked.
BOOL CBasePlayer::SetSuitUpdatePRE(char *name, int fgroup, int& isentence, BOOL fvoxException ){
	
	int i;
	//int isentence;
	
	// Ignore suit updates if no suit

	if ( !(pev->weapons & (1<<WEAPON_SUIT))  )
		return FALSE;
	
	//MODDD - NOTE: MULTIPLAYER FVOX BLOCKER
	// Check here disabled, merged into the "fvoxEnabled" check below.
	/*
	if ( IsMultiplayer() )
	{
		// due to static channel design, etc. We don't play HEV sounds in multiplayer right now.
		return FALSE;
	}
	*/

	//MODDD - also don't play if FVOX is no longer "enabled" and this is NOT the notification to turn it on / off (exceptions).
	//if(fvoxEnabled == 0 && !(name == "!HEV_V0" || name == "!HEV_V1")  ){
	// removed the multiplayer check!    IsMultiplayer() ||
	if( (fvoxEnabled == 0 || globalflag_muteDeploySound == TRUE) && !fvoxException){
		return FALSE;
	}


	// if name == NULL, then clear out the queue

	if (!name)
	{
		m_flSuitUpdate = 0;  //MODDD - NEW.  that's wise, right?

		for (i = 0; i < CSUITPLAYLIST; i++)
			m_rgSuitPlayList[i] = 0;
		return FALSE;
	}
	// get sentence or group number
	if (!fgroup)
	{
		isentence = SENTENCEG_Lookup(name, NULL);
		if (isentence < 0)
			return FALSE;
	}
	else
		// mark group number as negative
		isentence = -SENTENCEG_GetIndex(name);

	//made it here? sounds ok.
	return TRUE;

}//END OF SetSuitUpdatePRE(...)




BOOL CBasePlayer::SetSuitUpdatePOST(int iempty, int isentence, float fNoRepeatTime, float playDuration, BOOL canPlay){

	return SetSuitUpdateEventPOST(iempty, isentence, fNoRepeatTime, playDuration, canPlay, -1, NULL);
	
}//END OF SetSuitUpdatePOST(...)


BOOL CBasePlayer::SetSuitUpdateEventPOST(int iempty, int isentence, float fNoRepeatTime, float playDuration, BOOL canPlay, float eventDelay, void (CBasePlayer::*eventMethod)() ){
	
	return SetSuitUpdateEventFVoxCutoffPOST(iempty, isentence, fNoRepeatTime, playDuration, canPlay, eventDelay, eventMethod, -1);
}

BOOL CBasePlayer::SetSuitUpdateEventFVoxCutoffPOST(int iempty, int isentence, float fNoRepeatTime, float playDuration, BOOL canPlay, float eventDelay, void (CBasePlayer::*eventMethod)(), float fvoxCutoff ){

	if (fNoRepeatTime)
	{
		if (iempty < 0)
			iempty = RANDOM_LONG(0, CSUITNOREPEAT-1); // pick random slot to take over
		m_rgiSuitNoRepeat[iempty] = isentence;
		m_rgflSuitNoRepeatTime[iempty] = fNoRepeatTime + gpGlobals->time;
	}

	// find empty spot in queue, or overwrite last spot
	
	
	//MODDD - ADDITION
	//============================================================================
	if(!canPlay){
		return FALSE;
	}
	//============================================================================


	m_rgSuitPlayList[m_iSuitPlayNext] = isentence;
	m_rgSuitPlayListDuration[m_iSuitPlayNext] = playDuration;

	//defaults.
	//m_rgSuitPlayListEventDelay[m_iSuitPlayNext] = -1;
	//m_rgSuitPlayListEvent[m_iSuitPlayNext] = NULL;
	m_rgSuitPlayListEventDelay[m_iSuitPlayNext] = eventDelay;
	m_rgSuitPlayListEvent[m_iSuitPlayNext] = eventMethod;

	m_rgSuitPlayListFVoxCutoff[m_iSuitPlayNext] = fvoxCutoff;
	
	m_iSuitPlayNext++;

	if (m_iSuitPlayNext == CSUITPLAYLIST)
		m_iSuitPlayNext = 0;

	if (m_flSuitUpdate <= gpGlobals->time)
	{
		if (m_flSuitUpdate == 0)
			// play queue is empty, don't delay too long before playback
			m_flSuitUpdate = gpGlobals->time + SUITFIRSTUPDATETIME;
		else{
			//MODDD - no, trust the update is fine from when it was last set, let it continue unabridged.
			//m_flSuitUpdate = gpGlobals->time + SUITUPDATETIME; 
		}
	}

	return TRUE;
}//END OF SetSuitUpdateEventFVoxCutoffPOST(...)





BOOL CBasePlayer::SetSuitUpdateNoRepeatSweep(int& iempty, int isentence){
	int i;

	//MODDD - ADDITION - innocent (TRUE) until proven guilty (FALSE).
	//============================================================================
	BOOLEAN canPlay = TRUE;
	//============================================================================
	
	// check norepeat list - this list lets us cancel
	// the playback of words or sentences that have already
	// been played within a certain time.

	for (i = 0; i < CSUITNOREPEAT; i++)
	{
		if (isentence == m_rgiSuitNoRepeat[i])
			{
			// this sentence or group is already in 
			// the norepeat list
				
				if (m_rgflSuitNoRepeatTime[i] < gpGlobals->time){
					// norepeat time has expired, clear it out
					m_rgiSuitNoRepeat[i] = 0;
					m_rgflSuitNoRepeatTime[i] = 0.0;
					iempty = i;
					break;
				}
				else{
					//Clear it out anyways!  Reset the timer.
					m_rgiSuitNoRepeat[i] = 0;
					m_rgflSuitNoRepeatTime[i] = 0.0;
					iempty = i;
					canPlay = FALSE;
					//return;
				}
			}
		// keep track of empty slot
		if (!m_rgiSuitNoRepeat[i])
			iempty = i;
	}

	return canPlay;
}//END OF SetSuitUpdateNoRepeatSweep(...)

BOOL CBasePlayer::SetSuitUpdateCheckNoRepeatApply(int& iempty, int isentence){
	int i;
	
	// check norepeat list - this list lets us cancel
	// the playback of words or sentences that have already
	// been played within a certain time.

	for (i = 0; i < CSUITNOREPEAT; i++)
	{
		if (isentence == m_rgiSuitNoRepeat[i])
		{
		// this sentence or group is already in 
		// the norepeat list

		if (m_rgflSuitNoRepeatTime[i] < gpGlobals->time)
			{
			// norepeat time has expired, clear it out
			m_rgiSuitNoRepeat[i] = 0;
			m_rgflSuitNoRepeatTime[i] = 0.0;
			iempty = i;
			break;
			}
		else
			{
			// don't play, still marked as norepeat
			return FALSE;
			}
		}
		// keep track of empty slot
		if (!m_rgiSuitNoRepeat[i])
			iempty = i;
	}
	return TRUE;  //I think?
}//END OF SetSuitUpdateNoRepeatSweep(...)

BOOL CBasePlayer::SetSuitUpdateCheckNoRepeat(int& iempty, int isentence){
	int i;
	// check norepeat list - this list lets us cancel
	// the playback of words or sentences that have already
	// been played within a certain time.

	for (i = 0; i < CSUITNOREPEAT; i++)
	{
		if (isentence == m_rgiSuitNoRepeat[i])
			{
			// this sentence or group is already in 
			// the norepeat list

				if (m_rgflSuitNoRepeatTime[i] < gpGlobals->time){
					// norepeat time has expired, clear it out
					//m_rgiSuitNoRepeat[i] = 0;
					//m_rgflSuitNoRepeatTime[i] = 0.0;
					//iempty = i;
					//nah, just RETURN TRUE below.
					break;
				}
				else{
					//Clear it out anyways!  Reset the timer.  ...no, this means we can't play.
					//m_rgiSuitNoRepeat[i] = 0;
					//m_rgflSuitNoRepeatTime[i] = 0.0;
					//iempty = i;
					return FALSE;
				}
			}

		// keep track of empty slot
		//NOTE - iempty won't be referred to.  Remove iempty as a parameter and remove this action?
		if (!m_rgiSuitNoRepeat[i])
			iempty = i;
	}

	return TRUE;
}//END OF SetSuitUpdateCheckNoRepeat(...)



void CBasePlayer::SetSuitUpdateNumber(int number, float fNoRepeatTime, int noRepeatID, BOOL arg_getBatteryValueRealTime)
{
	SetSuitUpdateNumber(number, fNoRepeatTime, noRepeatID, arg_getBatteryValueRealTime, FALSE);
}

//MODDD - new var, "playDuration".  Defaults to "SUITUPDATETIME", 3.5 seconds, 
//NOTE: for "noRepeatTime" to be useful, this would need an ID, such as, printing the number
//for what?  health, battery, etc.?
//void CBasePlayer::SetSuitUpdateNumber(int number, float fNoRepeatTime, int noRepeatID, BOOL arg_getBatteryValueRealTime)
void CBasePlayer::SetSuitUpdateNumber(int number, float fNoRepeatTime, int noRepeatID, BOOL arg_getBatteryValueRealTime, BOOL fvoxException)
{
	BOOL passPRE = SetSuitUpdatePRE(fvoxException);
	// wait... why were we missing this checka while back?  I don't get that.
	if(!passPRE)return;  
	
	//That is, get the battery's current value the moment the number reading speech is loaded (if TRUE).
	getBatteryValueRealTime = arg_getBatteryValueRealTime;
	
	//This just sets this sound up for playing, and doesn't add it to the conventional no-repeat list.
	SetSuitUpdatePOST(0, 10000 + number, 0, 0, TRUE);
	
}

// m_rgSuitPlayListDuration


//MODDD - new method.
//Like "SetSuitUpdate", but only sets a sound to be blocked from repeating WITHOUT queueing it or playing it at all.
//This way, if some event is needed that happens in quick succession but uses different clips (curing bleeding via
//the wall-health charge kit has a delay, so both clips will play one after the other), this should be called to
//make the others wait for a delay too.
//The point is to stop sounds with the same purpose ("bleeding has stopped", "feeling better", etc.) from playing soon
//after one plays simply because they are not the exact same sound from 10 seconds ago. Those with the same purpose
//should also be blocked to not be spammy in case of a lot of potential messages.

void CBasePlayer::forceRepeatBlock(char *name, int fgroup, float fNoRepeatTime){
	forceRepeatBlock(name, fgroup, fNoRepeatTime, FALSE);
}
void CBasePlayer::forceRepeatBlock(char *name, int fgroup, float fNoRepeatTime, BOOL fvoxException){
	
	int i;
	int iempty = -1;
	int isentence;
	
	BOOL passPRE = SetSuitUpdatePRE(name, fgroup, isentence, fvoxException);
	// Must pass the PRE check to move on with this request.
	if(!passPRE) return;

	//MODDD - compare with old copy of forceRepeatBlock.  Is this simplification okay?

	SetSuitUpdateNoRepeatSweep(iempty, isentence);
	
	// sentence is not in norepeat list, save if norepeat time was given

	BOOL passPOST = SetSuitUpdatePOST(iempty, isentence, fNoRepeatTime, 0, FALSE);

}


BOOL CBasePlayer::suitCanPlay(char *name, int fgroup){
	return suitCanPlay(name, fgroup, FALSE);
}

// can the player play this sound at the moment, or is it repeat-blocked?
BOOL CBasePlayer::suitCanPlay(char *name, int fgroup, BOOL fvoxException){
	int i;
	int isentence;
	int iempty = -1;
	
	BOOL passPRE = SetSuitUpdatePRE(name, fgroup, isentence, fvoxException);
	// Must pass the PRE check to move on with this request.
	if(!passPRE) return FALSE;


	BOOL noRepeatPass = SetSuitUpdateCheckNoRepeat(iempty, isentence);
	if(!noRepeatPass){
		return FALSE;
	}
	// sentence is not in norepeat list, save if norepeat time was given

	// No need for the "post" part that plays / updates the norepeat list. This just asked the question, and it was answered.
	return TRUE;
}


// add sentence to suit playlist queue. if fgroup is TRUE, then
// name is a sentence group (HEV_AA), otherwise name is a specific
// sentence name ie: !HEV_AA0.  If fNoRepeatTime is specified in
// seconds, then we won't repeat playback of this word or sentence
// for at least that number of seconds.


void CBasePlayer::SetSuitUpdateFVoxException(char *name, int fgroup, float fNoRepeatTime){
	SetSuitUpdate(name, fgroup, fNoRepeatTime, SUITUPDATETIME, TRUE);
}
void CBasePlayer::SetSuitUpdateFVoxException(char *name, int fgroup, float fNoRepeatTime, float playDuration){
	SetSuitUpdate(name, fgroup, fNoRepeatTime, playDuration, TRUE);
}

void CBasePlayer::SetSuitUpdate(char *name, int fgroup, float fNoRepeatTime){
	SetSuitUpdate(name, fgroup, fNoRepeatTime, SUITUPDATETIME, FALSE);
}
void CBasePlayer::SetSuitUpdate(char *name, int fgroup, float fNoRepeatTime, float playDuration){
	SetSuitUpdate(name, fgroup, fNoRepeatTime, playDuration, FALSE);
}

//MODDD - new var, "playDuration".  Defaults to "SUITUPDATETIME", 3.5 seconds, 
void CBasePlayer::SetSuitUpdate(char *name, int fgroup, float fNoRepeatTime, float playDuration, BOOL fvoxException )
{
	int i;
	int iempty = -1;
	int isentence;
	
	BOOL passPRE = SetSuitUpdatePRE(name, fgroup, isentence, fvoxException);
	if(!passPRE) return;

	BOOL noRepeatPass = SetSuitUpdateCheckNoRepeatApply(iempty, isentence);
	if(!noRepeatPass){
		return;
	}

	BOOL passPOST = SetSuitUpdatePOST(iempty, isentence, fNoRepeatTime, playDuration, TRUE);

}



void CBasePlayer::SetSuitUpdateAndForceBlock(char *name, int fgroup, float fNoRepeatTime){
	SetSuitUpdateAndForceBlock(name, fgroup, fNoRepeatTime, SUITUPDATETIME, FALSE);
}
void CBasePlayer::SetSuitUpdateAndForceBlock(char *name, int fgroup, float fNoRepeatTime, float playDuration){
	SetSuitUpdateAndForceBlock(name, fgroup, fNoRepeatTime, playDuration, FALSE);
}

//MODDD - This is just a clone of "SetSuitUpdate" that has the change from "ForceSuitUpdateReset" or whatever that was called.
void CBasePlayer::SetSuitUpdateAndForceBlock(char *name, int fgroup, float fNoRepeatTime, float playDuration, BOOL fvoxException){
	
	int i;
	int iempty = -1;
	int isentence;
	
	BOOL passPRE = SetSuitUpdatePRE(name, fgroup, isentence, fvoxException);
	//Must pass the PRE check to move on with this request.
	if(!passPRE) return;

	BOOL canPlay = SetSuitUpdateNoRepeatSweep(iempty, isentence);

	// sentence is not in norepeat list, save if norepeat time was given

	BOOL passPOST = SetSuitUpdatePOST(iempty, isentence, fNoRepeatTime, playDuration, canPlay);

}//END OF SetSuitUpdateAndForceBlock



//SetSuitUpdateFVoxlessFriendlyEvent("!HEV_ANT_USE", FALSE, SUIT_REPEAT_OK, -1, -2, consumeAntidote);

//fvoxCutoff

void CBasePlayer::SetSuitUpdateEvent(char *name, int fgroup, float fNoRepeatTime, float eventDelay, void (CBasePlayer::*eventMethod)() ){
	SetSuitUpdateEvent(name, fgroup, fNoRepeatTime, SUITUPDATETIME, eventDelay, eventMethod);
}
void CBasePlayer::SetSuitUpdateEvent(char *name, int fgroup, float fNoRepeatTime, float playDuration, float eventDelay, void (CBasePlayer::*eventMethod)() ){
	SetSuitUpdateEventFVoxCutoff(name, fgroup, fNoRepeatTime, playDuration, FALSE, eventDelay, eventMethod, -1);

}
void CBasePlayer::SetSuitUpdateEvent(char *name, int fgroup, float fNoRepeatTime, float playDuration, BOOL fvoxException, float eventDelay, void (CBasePlayer::*eventMethod)() ){
	SetSuitUpdateEventFVoxCutoff(name, fgroup, fNoRepeatTime, playDuration, fvoxException, eventDelay, eventMethod, -1);

}

void CBasePlayer::SetSuitUpdateEventFVoxCutoff(char *name, int fgroup, float fNoRepeatTime, float eventDelay, void (CBasePlayer::*eventMethod)(), float fvoxOffCutoff ){
	SetSuitUpdateEventFVoxCutoff(name, fgroup, fNoRepeatTime, SUITUPDATETIME, FALSE, eventDelay, eventMethod, fvoxOffCutoff);
}
void CBasePlayer::SetSuitUpdateEventFVoxCutoff(char *name, int fgroup, float fNoRepeatTime, float playDuration, float eventDelay, void (CBasePlayer::*eventMethod)(), float fvoxOffCutoff ){
	SetSuitUpdateEventFVoxCutoff(name, fgroup, fNoRepeatTime, playDuration, FALSE, eventDelay, eventMethod, fvoxOffCutoff);
}
void CBasePlayer::SetSuitUpdateEventFVoxCutoff(char *name, int fgroup, float fNoRepeatTime, float playDuration, BOOL fvoxException, float eventDelay, void (CBasePlayer::*eventMethod)(), float fvoxOffCutoff ){
	int i;
	int iempty = -1;
	int isentence;
	

	// NOTE - in place of what is now "fvoxException",  used to be "fvoxException||(fvoxOffCutoff!=-1)".
	// Removed the other part (-1 check), because I don't like implied exceptions like this.  Just say it's meant to be one in the first place/call, dangit.
	BOOL passPRE = SetSuitUpdatePRE(name, fgroup, isentence, fvoxException );
	//Must pass the PRE check to move on with this request.
	if(!passPRE) return;

	BOOL canPlay = SetSuitUpdateNoRepeatSweep(iempty, isentence);

	// sentence is not in norepeat list, save if norepeat time was given

	BOOL passPOST = SetSuitUpdateEventFVoxCutoffPOST(iempty, isentence, fNoRepeatTime, playDuration, canPlay, eventDelay, eventMethod, fvoxOffCutoff);


}//END OF SetSuitUpdateEventFVoxCutoff




//=========================================================
// UpdatePlayerSound - updates the position of the player's
// reserved sound slot in the sound list.
//=========================================================
void CBasePlayer::UpdatePlayerSound ( void )
{
	int iBodyVolume;
	int iVolume;
	CSound *pSound;

	pSound = CSoundEnt::SoundPointerForIndex( CSoundEnt::ClientSoundIndex( edict() ) );

	if ( !pSound )
	{
		ALERT ( at_console, "Client lost reserved sound!\n" );
		return;
	}

	pSound->m_iType = bits_SOUND_NONE;

	// now calculate the best target volume for the sound. If the player's weapon
	// is louder than his body/movement, use the weapon volume, else, use the body volume.
	
	if ( FBitSet ( pev->flags, FL_ONGROUND ) )
	{	
		iBodyVolume = pev->velocity.Length(); 

		// clamp the noise that can be made by the body, in case a push trigger,
		// weapon recoil, or anything shoves the player abnormally fast. 
		if ( iBodyVolume > 512 )
		{
			iBodyVolume = 512;
		}
	}
	else
	{
		iBodyVolume = 0;
	}

	if ( pev->button & IN_JUMP )
	{
		iBodyVolume += 100;
	}

// convert player move speed and actions into sound audible by monsters.
	if ( m_iWeaponVolume > iBodyVolume )
	{
		m_iTargetVolume = m_iWeaponVolume;

		// OR in the bits for COMBAT sound if the weapon is being louder than the player. 
		pSound->m_iType |= bits_SOUND_COMBAT;
	}
	else
	{
		m_iTargetVolume = iBodyVolume;
	}

	// decay weapon volume over time so bits_SOUND_COMBAT stays set for a while
	m_iWeaponVolume -= 250 * gpGlobals->frametime;
	if ( m_iWeaponVolume < 0 )
	{
		iVolume = 0;
	}


	// if target volume is greater than the player sound's current volume, we paste the new volume in 
	// immediately. If target is less than the current volume, current volume is not set immediately to the
	// lower volume, rather works itself towards target volume over time. This gives monsters a much better chance
	// to hear a sound, especially if they don't listen every frame.
	iVolume = pSound->m_iVolume;

	if ( m_iTargetVolume > iVolume )
	{
		iVolume = m_iTargetVolume;
	}
	else if ( iVolume > m_iTargetVolume )
	{
		iVolume -= 250 * gpGlobals->frametime;

		if ( iVolume < m_iTargetVolume )
		{
			iVolume = 0;
		}
	}

	if ( m_fNoPlayerSound )
	{
		// debugging flag, lets players move around and shoot without monsters hearing.
		iVolume = 0;
	}

	if ( gpGlobals->time > m_flStopExtraSoundTime )
	{
		// since the extra sound that a weapon emits only lasts for one client frame, we keep that sound around for a server frame or two 
		// after actual emission to make sure it gets heard.
		m_iExtraSoundTypes = 0;
	}

	if ( pSound )
	{
		pSound->m_vecOrigin = pev->origin;
		pSound->m_iType |= ( bits_SOUND_PLAYER | m_iExtraSoundTypes );
		pSound->m_iVolume = iVolume;
	}

	// keep track of virtual muzzle flash
	m_iWeaponFlash -= 256 * gpGlobals->frametime;
	if (m_iWeaponFlash < 0)
		m_iWeaponFlash = 0;

	//UTIL_MakeAimVectors ( pev->angles );
	//gpGlobals->v_forward.z = 0;

	// Below are a couple of useful little bits that make it easier to determine just how much noise the 
	// player is making. 
	// UTIL_ParticleEffect ( pev->origin + gpGlobals->v_forward * iVolume, g_vecZero, 255, 25 );
	//ALERT ( at_console, "%d/%d\n", iVolume, m_iTargetVolume );
}



void CBasePlayer::PostThink()
{
	//if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(testVar) == -1)return;
	//CBasePlayer* tempplayerTTT = this;
	//easyPrintLine("VIEW ANGLES?! %.2f %.2f %.2f", tempplayerTTT->pev->v_angle.x, tempplayerTTT->pev->v_angle.y, tempplayerTTT->pev->v_angle.z);
	//easyPrintLine("MY VIEW ANGLES: %.2f, %.2f, %.2f", pev->angles.x, pev->angles.y, pev->angles.z);


	if (!g_mapLoadedEver) {
		easyPrintLine("!!!NOTICE:  Player think called before any map loaded? frame:%lu", g_ulFrameCount);
	}

	if (g_mapLoadedEver && queueFirstAppearanceMessageSend) {
		queueFirstAppearanceMessageSend = FALSE;

		/*
		UTIL_StopSound( edict(), CHAN_AUTO, "hgrunt/gr_pain2.wav", TRUE);
		UTIL_StopSound( edict(), CHAN_WEAPON, "hgrunt/gr_pain2.wav", TRUE);
		UTIL_StopSound( edict(), CHAN_VOICE, "hgrunt/gr_pain2.wav", TRUE);
		UTIL_StopSound( edict(), CHAN_ITEM, "hgrunt/gr_pain2.wav", TRUE);
		UTIL_StopSound( edict(), CHAN_BODY, "hgrunt/gr_pain2.wav", TRUE);
		UTIL_StopSound( edict(), CHAN_STREAM, "hgrunt/gr_pain2.wav", TRUE);
		UTIL_StopSound( edict(), CHAN_STATIC, "hgrunt/gr_pain2.wav", TRUE);
		UTIL_EmitAmbientSound(edict(), pev->origin, "hgrunt/gr_pain2.wav", 0, 0, SND_STOP, PITCH_NORM, TRUE);
		*/

		easyPrintLineClient(this->edict(), "PLAYER: OnFirstAppearance, sent first message");
		
		// fvox, holster, ladder given to the server player from the client by a response from this call.
		MESSAGE_BEGIN(MSG_ONE, gmsgOnFirstAppearance, NULL, pev);
		MESSAGE_END();
		

		MESSAGE_BEGIN(MSG_ONE, gmsgServerDLL_Info, NULL, pev);
			WRITE_STRING(globalbuffer_sv_mod_version);
			WRITE_STRING(globalbuffer_sv_mod_date);
		MESSAGE_END();
		//MESSAGE_BEGIN(MSG_ALL, gmsgUpdateClientCVar, NULL);


		//!!! IMPORTANT  Any broadcast CVars should show up here!!
		// Let any newly connected player get a fresh copy to replace the likely unhelpful clientside defaults.

		//EASY_CVAR_SYNCH_SERVER_TO_CLIENT(wpn_glocksilencer, wpn_glocksilencer_ID, pev);
		
		EASY_CVAR_SYNCH_SERVER_MASS

	}//END OF queueFirstAppearanceMessageSend check

	
	if(EASY_CVAR_GET_DEBUGONLY(myStrobe) == 1){
		if(nextMadEffect <= gpGlobals->time){
			//send effect!
			UTIL_generateFreakyLight(pev->origin);

			nextMadEffect = gpGlobals->time + EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(raveEffectSpawnInterval);
		}
	}


	//MODDD - new m_flFallVelocity reset location
	


	if ( g_fGameOver || !IsAlive() ){

		//MODDD - HOLD ON.  Go ahead and check for cheat impulse commands before skipping the rest.
		// If the player is cheating, who cares about those restrictions?
		// And may as well reset pev->impulse, no sense being queued up for some revive or 
		// multiplayer respawn.
		CheatImpulseCommands(pev->impulse);
		pev->impulse = 0;

		goto pt_end;         // intermission or finale  (for gameover)
		// WAIT WHAT.  Really?   g_fGameOver is on during intermissions between levels?
		// Probaly not, likely means the time between changemaps where no player movement is allowed
		// (in multiplayer servers typically).
	}// gameover or IsAlive checks


	// Handle Tank controlling
	if ( m_pTank != NULL )
	{ // if they've moved too far from the gun,  or selected a weapon, unuse the gun
		if ( m_pTank->OnControls( pev ) && !pev->weaponmodel )
		{  
			m_pTank->Use( this, this, USE_SET, 2 );	// try fire the gun
		}
		else
		{  // they've moved off the platform
			m_pTank->Use( this, this, USE_OFF, 0 );
			m_pTank = NULL;
		}
	}

// do weapon stuff
	ItemPostFrame( );

// check to see if player landed hard enough to make a sound
// falling farther than half of the maximum safe distance, but not as far a max safe distance will
// play a bootscrape sound, and no damage will be inflicted. Fallling a distance shorter than half
// of maximum safe distance will make no sound. Falling farther than max safe distance will play a 
// fallpain sound, and damage will be inflicted based on how far the player fell

	if ( (FBitSet(pev->flags, FL_ONGROUND)) && (pev->health > 0) && m_flFallVelocity >= PLAYER_FALL_PUNCH_THRESHHOLD )
	{
		// ALERT ( at_console, "%f\n", m_flFallVelocity );

		float fallSpeedToleranceMulti = 1;
		float fallDamageReduction = 1;
		if(jumpForceMultiMem > 1){
			//a jump force multiple above 1 will increase the tolerance for falls.
			fallSpeedToleranceMulti = sqrt(jumpForceMultiMem);
			fallDamageReduction = jumpForceMultiMem;
		}


		if (pev->watertype == CONTENTS_WATER)
		{
			// Did he hit the world or a non-moving entity?
			// BUG - this happens all the time in water, especially when 
			// BUG - water has current force
			// if ( !pev->groundentity || VARS(pev->groundentity)->velocity.z == 0 )
				// EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_wade1.wav", 1, ATTN_NORM);
		}
		else if ( m_flFallVelocity > PLAYER_MAX_SAFE_FALL_SPEED * fallSpeedToleranceMulti )
		{// after this point, we start doing damage
			

			float flFallDamage = g_pGameRules->FlPlayerFallDamage( this ) / fallDamageReduction;

			//MODDD - see "float CHalfLifeRules::FlPlayerFallDamage( CBasePlayer *pPlayer )" for why this may still be a bit wonky.
			//Fall damage for super jumps is still odd because the received "Fall Damage" does not factor in "fallDamageReduction".
			//It just subtracts PLAYER_MAX_SAFE_FALL_SPEED from m_flFallVelocity.  Properly, it would subtract
			//(PLAYER_MAX_SAFE_FALL_SPEED * fallSpeedToleranceMulti) from m_flFallVelocity.;

			if ( flFallDamage > pev->health )
			{//splat
				// note: play on item channel because we play footstep landing on body channel
				UTIL_PlaySound(ENT(pev), CHAN_ITEM, "common/bodysplat.wav", 1, ATTN_NORM, 0, 100, FALSE);
			}

			if ( flFallDamage > 0 )
			{
				TakeDamage(VARS(eoNullEntity), VARS(eoNullEntity), flFallDamage, DMG_FALL ); 

				//MODDD NOTE: - this causes any "x" axis punchangle from pm_shared's "checkfalling" method
				//(applies punch on hitting the ground from a fall) to just become "0" in an instant, making it ineffective!
				//pev->punchangle.x = 0;
			}
		}

		if ( IsAlive() )
		{
			SetAnimation( PLAYER_WALK );
		}
    }




	//MODDD - AI-sound placement changed to after 'pt_end' instead.
	// If a dead player slams into the ground, something should still turn to hear it.



	// select the proper animation for the player character	
	if ( IsAlive() )
	{
		if (!pev->velocity.x && !pev->velocity.y)
			SetAnimation( PLAYER_IDLE );
		else if ((pev->velocity.x || pev->velocity.y) && (FBitSet(pev->flags, FL_ONGROUND)))
			SetAnimation( PLAYER_WALK );
		else if (pev->waterlevel > 1)
			SetAnimation( PLAYER_WALK );
	}

	StudioFrameAdvance_SIMPLE( );
	CheckPowerups(pev);

	UpdatePlayerSound();

	// Track button info so we can detect 'pressed' and 'released' buttons next frame
	m_afButtonLast = pev->button;



	// BLOCK REMOVED.  See pm_shared.c, 'special ladder movement check'. Turns out this can be done there
	// like it should have been.
	/*
	int filterediuser4 = pev->iuser4 & ~(FLAG_JUMPED | FLAG_RESET_RECEIVED);
	//NOTE: coordinate the right-hand-side value with "ladderCycle" inside of "pm_shared.c".
	
	//...
	*/



pt_end:


	//MODDD - AI-sound placement moved from above.
	// old m_flFallVelocity reset location
	if (FBitSet(pev->flags, FL_ONGROUND))
	{
		//MODDD - and why multiplayer only anyhow?  I know the sound's just for AI though, probably why.  Doesn't hurt to anyway though.
		// (this isn't the audible sound of course)
		// Do require being alive though.
		//if (m_flFallVelocity > 64 && !IsMultiplayer())
		if (m_flFallVelocity > 64 && pev->deadflag == DEAD_NO)
		{
			CSoundEnt::InsertSound(bits_SOUND_PLAYER, pev->origin, m_flFallVelocity, 0.2);
			// ALERT( at_console, "fall %f\n", m_flFallVelocity );
		}
		m_flFallVelocity = 0;
	}


#if defined( CLIENT_WEAPONS )
		// Decay timers on weapons
	// go through all of the weapons and make a list of the ones to pack
	for ( int i = 0 ; i < MAX_ITEM_TYPES ; i++ )
	{
		if ( m_rgpPlayerItems[ i ] )
		{
			CBasePlayerItem *pPlayerItem = m_rgpPlayerItems[ i ];

			while ( pPlayerItem )
			{
				CBasePlayerWeapon *gun;

				gun = (CBasePlayerWeapon *)pPlayerItem->GetWeaponPtr();
				
				if ( gun && gun->UseDecrement() )
				{
					gun->m_flNextPrimaryAttack		= max( gun->m_flNextPrimaryAttack - gpGlobals->frametime, -1.0 );
					gun->m_flNextSecondaryAttack	= max( gun->m_flNextSecondaryAttack - gpGlobals->frametime, -0.001 );

					if ( gun->m_flTimeWeaponIdle != 1000 )
					{
						gun->m_flTimeWeaponIdle		= max( gun->m_flTimeWeaponIdle - gpGlobals->frametime, -0.001 );
					}

					if ( gun->pev->fuser1 != 1000 )
					{
						gun->pev->fuser1	= max( gun->pev->fuser1 - gpGlobals->frametime, -0.001 );
					}

					// Only decrement if not flagged as NO_DECREMENT
//					if ( gun->m_flPumpTime != 1000 )
				//	{
				//		gun->m_flPumpTime	= max( gun->m_flPumpTime - gpGlobals->frametime, -0.001 );
				//	}
					
				}

				pPlayerItem = pPlayerItem->m_pNext;
			}
		}
	}

	m_flNextAttack -= gpGlobals->frametime;
	if ( m_flNextAttack < -0.001 )
		m_flNextAttack = -0.001;
	
	if ( m_flNextAmmoBurn != 1000 )
	{
		m_flNextAmmoBurn -= gpGlobals->frametime;
		
		if ( m_flNextAmmoBurn < -0.001 )
			m_flNextAmmoBurn = -0.001;
	}

	if ( m_flAmmoStartCharge != 1000 )
	{
		m_flAmmoStartCharge -= gpGlobals->frametime;
		
		if ( m_flAmmoStartCharge < -0.001 )
			m_flAmmoStartCharge = -0.001;
	}
	
#else
	return;
#endif

	//pev->waterlevel = 3;
}


//MODDD - public setter methods.
void CBasePlayer::setHealth(int newHealth){
	pev->health = newHealth;
}
void CBasePlayer::setArmorBattery(int newBattery){
	pev->armorvalue = newBattery;
}

void CBasePlayer::SetAndUpdateBattery(int argNewBattery) {
	pev->armorvalue = argNewBattery;

	if (pev->armorvalue != m_iClientBattery) {
		m_iClientBattery = pev->armorvalue;
		MESSAGE_BEGIN(MSG_ONE, gmsgBattery, NULL, pev);
			WRITE_SHORT((int)pev->armorvalue);
		MESSAGE_END();
	}
}

// Called by things that heal on pickup/use.
// If the player had BLEEDING damage, stop it and play one of these messages.
void CBasePlayer::attemptCureBleeding(void) {

	if (m_rgbTimeBasedDamage[itbd_Bleeding] > 0 || m_bitsDamageTypeMod & DMG_BLEEDING) {
		int choice = RANDOM_LONG(0, 1);
		if (choice == 0) {
			// hiss, wound_sterilized
			SetSuitUpdate("!HEV_HEAL6", FALSE, SUIT_NEXT_IN_30SEC);
		}
		else if (choice == 1) {
			// hiss, morphine_shot
			SetSuitUpdate("!HEV_HEAL7", FALSE, SUIT_NEXT_IN_30SEC);
		}
		//else if (choice == 2) {
		//	SetSuitUpdate("!HEV_HEAL1", FALSE, SUIT_NEXT_IN_30SEC);
		//}

		//forceRepeatBlock("!HEV_HEAL1", FALSE, SUIT_NEXT_IN_30SEC);
		forceRepeatBlock("!HEV_HEAL6", FALSE, SUIT_NEXT_IN_30SEC);
		forceRepeatBlock("!HEV_HEAL7", FALSE, SUIT_NEXT_IN_30SEC);

		// removeTimedDamage ...
		m_rgbTimeBasedDamage[itbd_Bleeding] = 0;
		m_rgbTimeBasedFirstFrame[itbd_Bleeding] = TRUE;
		// necessary? isn't for antidote / anti-toxin (radiation item), though.
		// Apply to the other bitmask, since this is new (old one was full).
		m_bitsDamageTypeMod &= ~DMG_BLEEDING;
	}

}//attemptCureBleeding




void CBasePlayer::grantAllItems(){
	/*
	pev->weapons |= WEAPON_ALLWEAPONS;
	*/
	//THIS SHORTCUT IS UNWISE!!!

	pev->weapons |= (1<<WEAPON_SUIT);

	//All weapons granted, all ammo accessible.
	if(m_fLongJump != TRUE){
		longJumpChargeNeedsUpdate = TRUE;
	}
	m_fLongJump = TRUE;


	//It is up to anything with special deploy sounds to deny playing extra sounds if this is set.
	globalflag_muteDeploySound = TRUE;

	GiveNamedItemIfLacking( "weapon_crowbar" );
	GiveNamedItemIfLacking( "weapon_9mmhandgun" ); //same as "weapon_glock"
	GiveNamedItemIfLacking( "weapon_9mmAR" ); //same as "weapon_mp5"
	GiveNamedItemIfLacking( "weapon_357" ); //same as "weapon_python"
		
	//crossbow moved to last

	GiveNamedItemIfLacking( "weapon_gauss" );
	GiveNamedItemIfLacking( "weapon_hornetgun" );
	GiveNamedItemIfLacking( "weapon_tripmine" );
	GiveNamedItemIfLacking( "weapon_rpg" );

	GiveNamedItemIfLacking( "weapon_egon" );

	GiveNamedItemIfLacking( "weapon_satchel" );
	GiveNamedItemIfLacking( "weapon_shotgun" );
	GiveNamedItemIfLacking( "weapon_handgrenade" );
	GiveNamedItemIfLacking( "weapon_snark" );

	// Don't give chumtoads through cheat commands in multiplayer.
	// Don't really serve a purpose there.
	// Beats me what someone is doing using cheats in multiplayer anyway.  Garrysmod 1998 anyone.
	if (!IsMultiplayer()) {
		GiveNamedItemIfLacking("weapon_chumtoad");
	}

	//Deploy this instead.
	GiveNamedItemIfLacking( "weapon_crossbow" );


	//ItemInfoArray[ m_iId ].pszAmmo1 = 4;

	//well that was needless.
	/*
	CBasePlayerItem* test = FindNamedPlayerItem("weapon_9mmhandgun");
	if(test != NULL){
		CBasePlayerWeapon* test2 = (CBasePlayerWeapon *)test->GetWeaponPtr();
		if(test2 != NULL){
			CGlock* test3 = (CGlock*)test2;
			if(test3 != NULL){
				//WE GOT IT!
				this->hasGlockSilencer = 1;
			}
		}
	}
	*/
	this->hasGlockSilencer = TRUE;
	
	globalflag_muteDeploySound = FALSE;


	// Eliminate HEV chatter from all these new weapons (since the new HEV messages 
	// play upon receiving a weapon now)
	// No longer necessary, if granted while globalflag_muteDeploySound is on, they also won't
	// add FVox messages.
	//SetSuitUpdate(NULL, FALSE, 0);
}

// often called alongside "grantAllItems" above.
void CBasePlayer::giveMaxAmmo(){
	GiveAmmo( 999, "9mm", _9MM_MAX_CARRY );
	GiveAmmo( 999, "357", _357_MAX_CARRY );
	GiveAmmo( 999, "ARgrenades", M203_GRENADE_MAX_CARRY );
	GiveAmmo( 999, "bolts", BOLT_MAX_CARRY );
	GiveAmmo( 999, "buckshot", BUCKSHOT_MAX_CARRY );
	GiveAmmo( 999, "rockets", ROCKET_MAX_CARRY );
	GiveAmmo( 999, "uranium", URANIUM_MAX_CARRY );
	GiveAmmo( 999, "Hornets", HORNET_MAX_CARRY );

	GiveAmmo( 999, "Hand Grenade", HANDGRENADE_MAX_CARRY );
	GiveAmmo( 999, "Satchel Charge", SATCHEL_MAX_CARRY );
	GiveAmmo( 999, "Snarks", SNARK_MAX_CARRY );
	GiveAmmo( 999, "Trip Mine", TRIPMINE_MAX_CARRY );

	if (!IsMultiplayer()) {
		GiveAmmo(999, "Chum Toads", CHUMTOAD_MAX_CARRY);
	}
	
#if LONGJUMPUSESDELAY == 0
	longJumpCharge = PLAYER_LONGJUMPCHARGE_MAX;
	longJumpChargeNeedsUpdate = TRUE;
#endif
}


BOOL CBasePlayer::playerHasSuit(){
	return (pev->weapons & (1<<WEAPON_SUIT));
}
BOOL CBasePlayer::playerHasLongJump(){
	return m_fLongJump;
}



//MODDD - holds many of the things that used to be in the CBasePlayer constructor.
// Now this can be called by that constructor, AND the "commonReset" method below to remove all those redundant variable resets.
// This itself, "_commonReset" with the underscore, shouldn't be called straight by anything else.
// The intention is the CBasePlayer constructor calls this only, and commonReset calls this in addition to some other things it 
// needs to do now that the game has been created (can trust pev->... calls work, unlike in the constructor)
// ...oh.  almost nothing in common.  well ok then.
void CBasePlayer::_commonReset(void){

	hasGlockSilencerMem = -1;
	
	//in case a game is loaded... just forget this.
	minimumRespawnDelay = -1;

	framesUntilPushStops = -1;


	m_bitsDamageTypeForceShow = 0;
	m_bitsDamageTypeModForceShow = 0;

	recentDeadPlayerFollowersCount = 0;
}//END OF _commonReset



// This holds a lot of commands common between just defaulting several attributes to say, "Please update me with the real value soon".
// Methods that use this heavily are
//  Spawn
//  Precache
//  Restore
//  ForceClientDllUpdate
void CBasePlayer::commonReset(void){
	_commonReset();

	recentMajorTriggerDamage = FALSE;
	lastBlockDamageAttemptReceived = -5;
	
	// no... this causes it to be changed on revive, not what we want.
	//recentRevivedTime = -1;


	
	iWasFrozenToday = -1;

	//This discrepency forces writing to the physics keys at least once.
	pushSpeedMultiMem = -1;
	pushSpeedMulti = 1;

	normalSpeedMultiMem = -1;
	noclipSpeedMultiMem = -1;
	jumpForceMultiMem = -1;
	ladderCycleMultiMem = -1;
	ladderSpeedMultiMem = -1;
	sv_player_midair_fixMem = -1;
	sv_player_midair_accelMem = -1;

	//not for a CVar.
	clearWeaponFlag = -1;


	autoSneakyMem = -2;  //because -1 is a valid value for triggering a check in this case.


	//alphaCrosshairMem = -1;

	cameraModeMem = -1;
	mirrorsDoNotReflectPlayerMem = -1;

	playerBrightLightMem = -1;

	
	drowning = FALSE;  //!!! FOR NOW.
	drowningMem = -1;

	batterySayPhase = -1;
	obligedCustomSentence = 0;  //reset.

	//NOTICE:::for now, obligedCustomSentence will be reset when loading or entering a new place.
	//It doesn't need to be in use for long, and most messages interrupted aren't that important.
	obligedCustomSentence = 0;

#if PLAYER_ALWAYSHASLONGJUMP == 1
		m_fLongJump = TRUE;
#endif
	
	//MODDD
	deadflagmem = -1;

	

	//re-acquire pointers.
	//the_default_fov = 0;


	//MODDD - added
	m_iClientAntidote = -1;
	m_iClientAdrenaline = -1;
	m_iClientRadiation = -1;


	//recoveryIndex = -1;  //nah, this is okay.
	//recoveryDelay = -1;
	//recoveryDelayMin = -1;

	//recentlyGibbed = FALSE;



	//canApplyDefaultFOV REMOVAL
	//canApplyDefaultFOVMem = 0;  //assume "no".  Will be contradicted if necessary.
	
	//MODDD
	oldWaterMoveTime = -1;
	oldThinkTime = -1;
	
	//No, don't always reset airTankTime!
	//airTankAirTime = 0;
	airTankAirTimeMem = -1;
	longJumpChargeMem = -1;
	//resetLongJumpCharge();
	airTankAirTimeNeedsUpdate = TRUE;
	



	//MODDD
	// only spawn does this below.  proceed?
	m_fLongJumpMemory = m_fLongJump;
	longJumpDelay = 0;
	longJump_waitForRelease = FALSE;
	longJumpChargeNeedsUpdate = TRUE;


	//MODDD - also new.  Just a check to see if the user does not have long jump, yet "longJumpCharge" is not negative 1 (don't draw to GUI).
	if(m_fLongJumpMemory != m_fLongJump || (!m_fLongJump && longJumpCharge != -1) ){
		//easyPrint("TEST2 %d\n", 0);
		m_fLongJumpMemory = m_fLongJump;
		// NOPE, probably remove more surrounding script later.
		// Don't reset the longjump charge for this reason, not
		// requiring any particular pickup anymore.
		//resetLongJumpCharge();
		longJumpChargeNeedsUpdate = TRUE;
	}
	

	//MODDD - this enables the long jump.  The "animation" elsewhere isn't the physical long jump.
	/*
	if ( m_fLongJump )
	{
		g_engfuncs.pfnSetPhysicsKeyValue( edict(), "slj", "1" );
	}
	else
	{
		g_engfuncs.pfnSetPhysicsKeyValue( edict(), "slj", "0" );
	}
	*/
	//Just disable for now, we re-enable in real time when we want to long jump.
	g_engfuncs.pfnSetPhysicsKeyValue( edict(), "slj", "0" );

	//A cheap way to reset fallingvelocity if hitting the ground before reviving with adrenaline.
	g_engfuncs.pfnSetPhysicsKeyValue( edict(), "res", "0" );


	//MODDD - resetting here should be okay.
	m_flgeigerRange = 1000;
	m_igeigerRangePrev = 1000;


	//MODDD - unsure.  But most client things are reset, I think this should be okay.
	m_bitsHUDDamage = -1;
	m_bitsModHUDDamage = -1;

	// Nevermind, no longer necessary.  Other ways to determine being a player at clientside.
	//pev->renderfx |= ISPLAYER;


}//END OF commonReset



void CBasePlayer::autoSneakyCheck(void){
	float autoSneakyValue = EASY_CVAR_GET_DEBUGONLY(autoSneaky);

	if(autoSneakyValue == 1){
		turnOnSneaky();
	}else if(autoSneakyValue == -1){
		turnOffSneaky();
	}
}

void CBasePlayer::turnOnSneaky(void){
	//g_engfuncs.pfnClientCmd("impulse 105");
	pev->flags |= FL_NOTARGET;
	m_fNoPlayerSound = TRUE;
}

void CBasePlayer::turnOffSneaky(void){
	pev->flags &= (~FL_NOTARGET);
	m_fNoPlayerSound = FALSE;
}


// MODDD - you better believe it.
// Called to signify the 'first appearance' of this player in a given game.
// Called by Client.cpp's "ClientPutInServer" (which also calls Spawn for the first time) for starting a new game
// or connecting to a server (gee imagine that),
// OR called by Restore calling precache (which calls onFirstAppearance) on loading a game.
// This only needs to run once on starting/joining a server/game to load the server's broadcasted CVars into
// the connected client's cache.
// Because lacking FCVAR_REPLICATED is a bitch.
void CBasePlayer::OnFirstAppearance(void) {
	//NOTICE - this is happening on coming from map transitions too (Restore call), but not sure what can be done about that.
	// Not that it's a big problem though.

	// Single or multiplayer, this works fine.
	g_firstPlayerEntered = TRUE;
	queueFirstAppearanceMessageSend = TRUE;

}//END OF OnFirstAppearance


// The hideDamage CVar makes the player unaffected by punches.
// It is still up to individual cases to check for this "blocksImpact" feature of any entity and know not to do the camera punch + movement force if it is on.
BOOL CBasePlayer::blocksImpact(void){
	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hideDamage) <= 0){
		return FALSE;
	}else{
		return TRUE;
	}
}//END OF blocksImpact




//MODDD - new.  Without args, assume we're not reviving from adrenaline.
void CBasePlayer::Spawn( void ){
	Spawn(FALSE);
}

void CBasePlayer::Spawn( BOOL revived ){
	
	/*
	//pev->friction		= 1.0;  //multiplayer requires this to be set to move. Why? dunno
	
	pev->takedamage		= DAMAGE_AIM;
	//pev->solid			= (int)EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(testVar);
	//pev->movetype		= MOVETYPE_STEP;  //FORCED!!!
	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_BOUNCE;
	pev->health = 100;
	//pev->classname		= MAKE_STRING("player");
	g_pGameRules->GetPlayerSpawnSpot( this );
	pev->classname		= MAKE_STRING("zzzwhat");
	SET_MODEL(ENT(pev), "models/player.mdl");
	g_ulModelIndexPlayer = pev->modelindex;
	return;
	*/

	//just in case.
	pev->rendermode = kRenderNormal;
	pev->renderamt = 0;  //yes, kRenderNormal needs this to be 0 to look normal (even opaque)

	
	m_bHolstering = FALSE;
	m_pQueuedActiveItem = NULL;
	m_fCustomHolsterWaitTime = -1;

	//pev->rendermode = kRenderTransTexture;


	friendlyCheckTime = 0; //can check.

	//flag set at death for some reason.  Undo in spawn.
	pev->effects &= ~EF_NOINTERP;

	//haven't died yet...
	alreadyDroppedItemsAtDeath = FALSE;
	sentCarcassScent = FALSE;

	//the air-tank air sound may play at contact with water.
	airTankWaitingStart = TRUE;

	//easyPrintLine("IS SPAWN CALLED?!");

	//PRECACHE_MODEL("models/player/gman/Gman.mdl");


	// DEAD TODO:  any talkmonsters with a 'deadPlayerFocus' of this player will
	// get 'TaskFail()' called to cancel whatever they're doing.  Drop the talk restriction.
	// while(each talkmonster talkmonster->TaskFail();
	g_TalkMonster_PlayerDead_DialogueMod = 0;
	deadStage = 0;
	nextDeadStageTime = -1;
	recentDeadPlayerFollowersCount = 0;

	UTIL_SetDeadPlayerTruce(FALSE);


	pev->classname = MAKE_STRING("player");
	if(!revived){

		//MODDD - why not reset these at a true spawn?
		m_bitsDamageType = 0;
		m_bitsDamageTypeMod = 0;
		m_bitsDamageTypeForceShow = 0;
		m_bitsDamageTypeModForceShow = 0;


		// safe to reset this then
		// Set to a very low negative number so that comparisons with gpGlobals->time shortly after loading
		// the map still work (killed 6 seconds after beginning a map forgets you have adrenaline because
		// 6 is indeed less than a usual time default (like -1) + 10.
		// .......   who keeps running into this stuff
		recentRevivedTime = -100;
		
		drowning = FALSE;

		//FVOX messages play when this is true.
		// ...and why set it here though?
		//fvoxEnabled = TRUE;

		
		foundRadiation = FALSE;

		//MODDD - start without longjump item until otherwise told (can still use the "long jump" ability
		//as long as "m_fLongJump" is on, and I believe it is just always on now).
		hasLongJumpItem = FALSE;

		pev->health			= 100;
		//always set battery to 0 at a true respawn.
		//It will be reset if needed before now.
		pev->armorvalue		= 0;

		//MODDD - how about this while we're at it?  Already does the above two sets.
		attemptResetTimedDamage(TRUE);

	}else{


		//MODDD - the player will not recover previous drowning-induced drown damage after an
		//        underwater "incapacitation".

		//should breathe-time be explicitly reset?  it may be already.
		//pev->air_finished = gpGlobals->time + PLAYER_AIRTIME;

		m_idrowndmg = 0;
		pev->health = gSkillData.player_revive_health;

		if(EASY_CVAR_GET_DEBUGONLY(batteryDrainsAtAdrenalineMode) == 3){
			pev->armorvalue		= 0;
		}
		if(EASY_CVAR_GET_DEBUGONLY(timedDamageReviveRemoveMode) == 3){
			attemptResetTimedDamage(TRUE);
		}
	}//END OF if(!revived)

	
	pev->takedamage		= DAMAGE_AIM;

	//hm, experiment?
	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_BOUNCE;

	//MODDD - unreliable with health mods above.
	//pev->max_health		= pev->health;
	pev->max_health = 100;


	// MODDD -  LETS   FUCKING   GOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO
	// Say you're resetting all the flags like a normal person.  Except for FL_PROXY which is to be preserved.
	// Keeping FL_GOD flags too, because endlessly revive/die/revive/die/revive/die'ing in a insta-kill pit is hilarious.
	//pev->flags		   &= FL_PROXY;	// keep proxy flag sey by engine
	pev->flags = pev->flags & (FL_PROXY | FL_GODMODE | FL_NOTARGET);



	pev->flags		   |= FL_CLIENT;
	pev->air_finished	= gpGlobals->time + 12;
	pev->dmg			= 2;				// initial water damage
	pev->effects		= 0;
	pev->deadflag		= DEAD_NO;
	pev->dmg_take		= 0;
	pev->dmg_save		= 0;
	pev->friction		= 1.0;
	pev->gravity		= 1.0;
	m_bitsHUDDamage		= -1;
	//MODDD
	m_bitsModHUDDamage	= -1;


	m_afPhysicsFlags	= 0;


#if PLAYER_ALWAYSHASLONGJUMP == 1
		m_fLongJump = TRUE;
#else
		//start without it?
		m_fLongJump = FALSE;
#endif



	recentlyGibbed = FALSE;


	if(!revived){
		//These are some vars that are best reset only at spawn.  On revive, just leave them the way they are.

		//glockSilencerOnVar = 0;
		//egonAltFireOnVar = 0;
		//At start, assume the silencer and egon-altfire is off.

		lastDuckVelocityLength = 0;

		airTankAirTime = 0;
		//start with no air tank.

		longJumpCharge = 0;
		//start with no charge.

		g_engfuncs.pfnSetPhysicsKeyValue( edict(), "slj", "0" );
		// I assure you this is still half-life
		//g_engfuncs.pfnSetPhysicsKeyValue( edict(), "hl", "1" );

		//new phyiscs var: player ladder movement.  This is related to a CVar that may be changed.
		g_engfuncs.pfnSetPhysicsKeyValue( edict(), "plm", "0" );

		g_engfuncs.pfnSetPhysicsKeyValue( edict(), "gmm", "1" );
	}//END OF if(!revived)



	// may as well turn these off, either way.  It would have been done by now if it was going to be done.
	// hm... apply it now.  Adrenaline revives, remember?
	// Wait.  No, on adrenaline revives we do want to keep weapons not dropped.
	// How about, if reviving, we don't follow this.
	// We'll trust on reviving that anything dropped was also removed from the player as to avoid duplicates.

	if (revived) {
		// in case we dropped anything, update the clientside weapon list.
		// wait no, this isn't helpful anymore?
		//m_fKnownItem = FALSE;
	}
	else {
		if (scheduleRemoveAllItems) {
			RemoveAllItems(scheduleRemoveAllItemsIncludeSuit);
		}
	}

	scheduleRemoveAllItemsIncludeSuit = FALSE;
	scheduleRemoveAllItems = FALSE;
	// We may still need to send an update for parts of the HUD that are now not synced right.
	//UpdateClientData();
	//ForceClientDllUpdate();
	// ???
	//
	//MESSAGE_BEGIN(MSG_ONE, gmsgClearWeapon, NULL, pev);
	//MESSAGE_END();


	recoveryIndex = -1;
	recoveryDelay = -1;
	recoveryDelayMin = -1;

	
	pev->fov = m_iFOV				= 0;// init field of view.
	m_iClientFOV		= -1; // make sure fov reset is sent

	m_flNextDecalTime	= 0;// let this player decal as soon as he spawns.

	m_flgeigerDelay = gpGlobals->time + 2.0;	// wait a few seconds until user-defined message registrations
												// are recieved by all clients
	
	m_flTimeStepSound	= 0;
	m_iStepLeft = 0;
	m_flFieldOfView		= 0.5;// some monsters use this to determine whether or not the player is looking at them.


	m_bloodColor	= BLOOD_COLOR_RED;

	m_flNextAttack	= UTIL_WeaponTimeBase();

	// what
	//StartSneaking();

	m_iFlashBattery = 99;
	m_flFlashLightTime = 1; // force first message

// dont let uninitialized value here hurt the player
	m_flFallVelocity = 0;

	g_pGameRules->SetDefaultPlayerTeam( this );


	if(!revived){
		//get the spawn spot.
		g_pGameRules->GetPlayerSpawnSpot( this );
		pev->punchangle = g_vecZero;  // that too?
	}else{
		//not spawning, reviving.  re-appear close to the same spot, probably adjust the view ang. to look forward.
		pev->origin = pev->origin + Vector(0,0,1);
		pev->v_angle  = g_vecZero;
		pev->velocity = g_vecZero;

		//pev->angles = VARS(pentSpawnSpot)->angles;
		pev->angles = Vector( 0,pev->angles.y,pev->angles.z);
		//that is, force the "x" rotation to 0, use the others as they were.
		//This makes the player face straight forward, in the sense of not up or down.

		pev->punchangle = g_vecZero;
		pev->fixangle = TRUE;
	}
	//otherwise, leave it as it is.

	//

	SET_MODEL(ENT(pev), "models/player.mdl");

	//SET_MODEL(ENT(pev), "models/player/gman/Gman.mdl");
	
    g_ulModelIndexPlayer = pev->modelindex;
	pev->sequence		= LookupActivity( ACT_IDLE );

	if ( FBitSet(pev->flags, FL_DUCKING) ) 
		UTIL_SetSize(pev, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX);
	else
		UTIL_SetSize(pev, VEC_HULL_MIN, VEC_HULL_MAX);

    pev->view_ofs = VEC_VIEW;
	Precache();
	m_HackedGunPos		= Vector( 0, 32, 0 );


	/*
	//MODDD - REMOVED.  This could never go anywhere, no other mentions of m_iPlayerSound.
	if ( m_iPlayerSound == SOUNDLIST_EMPTY )
	{
		ALERT ( at_console, "Couldn't alloc player sound slot!\n" );
	}
	*/


	if(!revived){
		m_pLastItem = NULL;
		m_fWeapon = FALSE;
	}

	m_fInitHUD = TRUE;
	m_iClientHideHUD = -1;  // force this to be recalculated
	m_pClientActiveItem = NULL;
	m_iClientBattery = -1;

	
	if(this->m_pActiveItem != NULL){
		/*
		CBasePlayerWeapon* testWeap = (CBasePlayerWeapon *)m_pActiveItem->GetWeaponPtr();
		if(testWeap != 0){
			
		}
		*/
	}

	
	//deploy the active weapon, since the HUD was cleared.
	if (m_pActiveItem)
	{
		m_pActiveItem->Deploy( );
		m_pActiveItem->UpdateItemInfo( );
	}


	//MODDD - only reset ammo on a true spawn, not a revive.
	if(!revived){
		// reset all ammo values to 0
		for ( int i = 0; i < MAX_AMMO_TYPES; i++ )
		{
			// so setting both to 0 doesn't get ammo out of synch until ammo numbers are changed at some point?
			// Suppose not.   (idea was to not set AmmoLost, so that the HUD sees something out of synch if that
			// is the case... not important if ammo numbers are force-updated somewhere else)
			m_rgAmmo[i] = 0;
			m_rgAmmoLast[i] = 0;  // client ammo values also have to be reset  (the death hud clear messages does on the client side)
		}
	}else{
		// I think this just forces all ammo valus to be updated clientside?
		for ( int i = 0; i < MAX_AMMO_TYPES; i++ )
		{
			m_rgAmmoLast[i] = 0;  // client ammo values also have to be reset  (the death hud clear messages does on the client side)
		}
	}//END OF else OF if(!revived)


	m_lastx = m_lasty = 0;
	
	m_flNextChatTime = gpGlobals->time;

	// commonReset DISABLE:  Do we need it here?
	//commonReset();

	//TabulateAmmo();
	//SendAmmoUpdate();

	//MODDD - only call "rules->PlayerSpawn" on non-revives.  Seems the only purpose of this call is to grant the player starting weapons at spawn
	// in multiplayer.  Weird to start with a new glock/crowbar after getting up from adrenaline.
	if (!revived) {
		g_pGameRules->PlayerSpawn(this);
	}

	//Force a client re-update at revive!

	// Nevermind.
	//pev->renderfx |= ISPLAYER;


}//END OF Spawn


//MODDD - added from inheritance heirarchy
void CBasePlayer::Activate(void){
	
	//MODDD - this solves the egon "click" on the first use since load issue... I think.
	TabulateAmmo();

	CBaseMonster::Activate();
}

void CBasePlayer::Precache( void )
{
	// in the event that the player JUST spawned, and the level node graph
	// was loaded, fix all of the node graph pointers before the game starts.
	
	// !!!BUGBUG - now that we have multiplayer, this needs to be moved!
	if ( WorldGraph.m_fGraphPresent && !WorldGraph.m_fGraphPointersSet )
	{
		if ( !WorldGraph.FSetGraphPointers() )
		{
			ALERT ( at_console, "**Graph pointers were not set!\n");
		}
		else
		{
			ALERT ( at_console, "**Graph Pointers Set!\n" );
		} 
	}

	// SOUNDS / MODELS ARE PRECACHED in ClientPrecache() (game specific)
	// because they need to precache before any clients have connected

	// init geiger counter vars during spawn and each time
	// we cross a level transition


	//m_flgeigerRange = 1000;
	//m_igeigerRangePrev = 1000;   //Moved to common reset.  Seems safe.

	//MODDD - NOTE:::only done on spawn now.  Damages should be remembered between maps.
	// Although this was probably harmless as timed damages are committed to other arrays right in TakeDamage?   huh.
	//m_bitsDamageType = 0;  //NOTE:::should this be done in common reset too?
	//m_bitsDamageTypeMod = 0;

	//m_bitsHUDDamage = -1;  done in commonreset.

	m_iClientBattery = -1;

	superDuperDelay = -2;
	commonReset();

	recentRevivedTime = -100;  //is a reset here fine then?
	// PENDING:  let commonReset be told whether it's for a player revive or not ('not' being a clean spawn, game restore or map transition)

	m_iTrain = TRAIN_NEW;

	
	//MODDD - CRITICAL.
	// Why are user messages linked in the player precache method?  Why not as early as possible in the game DLL init (game.cpp) like server-registered
	// CVars are?
	/*
	easyForcePrintLine("LINKING USER MESSAGES...");
	// Make sure any necessary user messages have been registered
	LinkUserMessages();
	*/

	m_iUpdateTime = 5;  // won't update for 1/2 a second

	if ( gInitHUD )
		m_fInitHUD = TRUE;


}


int CBasePlayer::Save( CSave &save )
{
	if ( !CBaseMonster::Save(save) )
		return 0;

	return save.WriteFields( "PLAYER", this, m_playerSaveData, ARRAYSIZE(m_playerSaveData) );
}

//
// Marks everything as new so the player will resend this to the hud.
//
void CBasePlayer::RenewItems(void)
{

}



int CBasePlayer::Restore( CRestore &restore )
{
	// Coming from a save file?
	// Even so much as the "easyForcePrintClient" deeper in this call
	// can cause this error:
	//     "SZ_GetSpace:  Tried to write to an uninitialized sizebuf_t: ???"
	// So that's <pretty> great.


	// TEST?
	//UTIL_StopSound( edict(), CHAN_WEAPON, "garg/gar_stomp1.wav", TRUE);
	//UTIL_StopSound( edict(), CHAN_WEAPON, "hgrunt/gr_pain2.wav", TRUE);

	

	easyPrintLine("***Player Restored");
	OnFirstAppearance();
	// safe?  On a fresh map start, 'Restore' gets skipped, which is fine, should rely on
	// the '0' the constructor set it to.  Or maybe all memory of all entities is set to 0 anyway.

	m_framesSinceRestore = 0;

	friendlyCheckTime = 0;  //can check again.
	
	//MODDD - Remove this var at some point, not using this anymore.
	// Causes weirdness at transitions, unsure if it's even necessary at a fresh spawn.
	//globalPSEUDO_forceFirstPersonIdleDelay = 1;


	////easyPrintLine("PLAYER RESTORE METHOD CALLED!");

	if ( !CBaseMonster::Restore(restore) )
		return 0;

	int status = restore.ReadFields( "PLAYER", this, m_playerSaveData, ARRAYSIZE(m_playerSaveData) );

	
	SAVERESTOREDATA *pSaveData = (SAVERESTOREDATA *)gpGlobals->pSaveData;
	// landmark isn't present.
	if ( !pSaveData->fUseLandmark )
	{
		ALERT( at_console, "No Landmark:%s\n", pSaveData->szLandmarkName );

		// default to normal spawn
		edict_t* pentSpawnSpot = EntSelectSpawnPoint( this );
		pev->origin = VARS(pentSpawnSpot)->origin + Vector(0,0,1);
		pev->angles = VARS(pentSpawnSpot)->angles;
	}
	pev->v_angle.z = 0;	// Clear out roll
	pev->angles = pev->v_angle;

	pev->fixangle = TRUE;           // turn this way immediately

// Copied from spawn() for now
	m_bloodColor	= BLOOD_COLOR_RED;

    g_ulModelIndexPlayer = pev->modelindex;

	if ( FBitSet(pev->flags, FL_DUCKING) ) 
	{
		// Use the crouch HACK
		//FixPlayerCrouchStuck( edict() );
		// Don't need to do this with new player prediction code.
		UTIL_SetSize(pev, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX);
	}
	else
	{
		UTIL_SetSize(pev, VEC_HULL_MIN, VEC_HULL_MAX);
	}

	// This is to let pm_shared know that this is half-life?  uh, ok
	// MODDD - DISABLED.  pm_shared never checks for any "hl" physics key so this is pointless
	//g_engfuncs.pfnSetPhysicsKeyValue( edict(), "hl", "1" );

	
	//autoSneakyCheck();
	superDuperDelay = -2;

	// commonReset DISABLE:  Do we need it here?
	//commonReset();

	RenewItems();

#if defined( CLIENT_WEAPONS )
	// HACK:	This variable is saved/restored in CBaseMonster as a time variable, but we're using it
	//			as just a counter.  Ideally, this needs its own variable that's saved as a plain float.
	//			Barring that, we clear it out here instead of using the incorrect restored time value.
	m_flNextAttack = UTIL_WeaponTimeBase();
#endif


	/*
	//MODDD - if we have a weapon out, make it work (animate / not) the same way as one recently deployed would.
	if(m_pActiveItem != NULL){
		forceNoWeaponLoop = TRUE;
		CBasePlayerWeapon* wpnTest = (CBasePlayerWeapon *) m_pActiveItem->GetWeaponPtr();
		//gun = (CBasePlayerWeapon *)pPlayerItem->GetWeaponPtr();
		if(wpnTest != NULL){
			wpnTest->m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + wpnTest->randomIdleAnimationDelay();
		}
	}
	*/

	//assume we have a weapon out.
	// + randomIdleAnimationDelay()


	// Need to keep this in synch with the physics key since loading?
	SetGravity(pev->gravity);


	if(!IsMultiplayer() && pev->deadflag != DEAD_NO){ //recoveryIndex == 3){
		// don't know how that could happen
		if(deadStage < 8){
			// not to the post-death yet.
			g_TalkMonster_PlayerDead_DialogueMod = 1;
		}else{
			// 8 technically isn't post-death but eh.  count for coming from a load game here anyway.
			g_TalkMonster_PlayerDead_DialogueMod = 2;
		}
	}

	return status;
}//Restore




//MODDD - ... wait.  This is never called either.  OOPS.
// Was it meant to be for the really unfinished hud_fastswitch maybe?  Same for the found-empty SelectPrevItem?
// Oh.  Actually look in clientside weapons_resource.cpp, a comment suggests fast-swapping to a weapon is only
// supposed to work if there is only one item in that bucket.  Oooooookay, seems limited then.
void CBasePlayer::SelectNextItem( int iItem )
{
	CBasePlayerItem *pItem;

	pItem = m_rgpPlayerItems[ iItem ];
	
	if (!pItem)
		return;

	if (pItem == m_pActiveItem)
	{
		// select the next one in the chain
		pItem = m_pActiveItem->m_pNext; 
		if (! pItem)
		{
			return;
		}

		CBasePlayerItem *pLast;
		pLast = pItem;
		while (pLast->m_pNext)
			pLast = pLast->m_pNext;

		// relink chain
		pLast->m_pNext = m_pActiveItem;
		m_pActiveItem->m_pNext = NULL;
		m_rgpPlayerItems[ iItem ] = pItem;
	}

	ResetAutoaim( );




	/*
	// FIX, this needs to queue them up and delay
	if (m_pActiveItem)
	{
		m_pActiveItem->Holster( );
	}
	
	m_pActiveItem = pItem;

	if (m_pActiveItem)
	{
		m_pActiveItem->Deploy( );
		m_pActiveItem->UpdateItemInfo( );
	}
	*/
	//MODDD - replaced by this.

	// ...wait.  No line like this?  why?
	//m_pLastItem = m_pActiveItem;

	setActiveItem_HolsterCheck(pItem);



	// Wait.. why did I even put this here?  SelectNextItem...  what?
	// It's not at startup or called per frame.  Good god what was I smoking
	//pev->renderfx |= ISPLAYER;
}


//MODDD - IMPORTANT. Keep any changes with this in check with hl_weapons.cpp's version for clientside.
void CBasePlayer::SelectItem(const char *pstr)
{
	if (!pstr)
		return;

	CBasePlayerItem *pItem = NULL;

	for (int i = 0; i < MAX_ITEM_TYPES; i++)
	{
		if (m_rgpPlayerItems[i])
		{
			pItem = m_rgpPlayerItems[i];
	
			while (pItem)
			{
				const char* thaNam = STRING(pItem->pev->classname);


				if (FClassnameIs(pItem->pev, pstr))
					break;
				pItem = pItem->m_pNext;
			}
		}

		if (pItem)
			break;
	}

	if (!pItem)
		return;

	
	if (pItem == m_pActiveItem 
		//MODDD - new extra condition. If it is the case that we're switching out weapons (holster anim playing;
		//        m_pQueuedActiveItem not null), and we happen to pick the original item, it is ok to do that switch.
		&& !( m_pQueuedActiveItem != NULL && pItem != m_pQueuedActiveItem)
		)
		return;
	
	ResetAutoaim( );

	setActiveItem_HolsterCheck(pItem);

}//END OF SelectItem


//MODDD - final step in selecting an item.  Doesn't do the holster-decision, this would be called by that
// or anything selecting a weapon that skips holstering.
void CBasePlayer::setActiveItem(CBasePlayerItem* argItem){

	m_pLastItem = m_pActiveItem;


	m_pActiveItem = argItem;

	if (m_pActiveItem)
	{
		m_pActiveItem->Deploy( );
		m_pActiveItem->UpdateItemInfo( );
	}
}



// if allowed by cl_holster, and sets the target weapon to deploy after that finishes.
// Or goes to deploy instantly like retail.
// a filter that calls setActiveItem straight away if holstering is disabled.
void CBasePlayer::setActiveItem_HolsterCheck(CBasePlayerItem* argItem) {
	// -1 for forceHolster means, 'leave it up to cl_holster'
	setActiveItem_HolsterCheck(argItem, -1);
}
void CBasePlayer::setActiveItem_HolsterCheck(CBasePlayerItem* argItem, int forceHolster) {

	// ******SCRIPT THIS REPLACES, exact or similar-intent repeated a few places
	/*
	// FIX, this needs to queue them up and delay
	if (m_pActiveItem)
		m_pActiveItem->Holster( );

	m_pLastItem = m_pActiveItem;
	m_pActiveItem = pItem;

	if (m_pActiveItem)
	{
		m_pActiveItem->Deploy( );
		m_pActiveItem->UpdateItemInfo( );
	}
	*/
	///////////////////////////////////////////////////////////////////////////


	if (argItem == m_pActiveItem) {
		// change to the same weapon as what's already equippped?     what?
		return;
	}


	// Use 'forceHolster' to determine whether this is left up to the cl_holster setting,
	// or force off or on only this call.
	BOOL willHolster;
	if (forceHolster == -1) {
		//default: Leave it to cl_holster
		willHolster = fHolsterAnimsEnabled;
	}
	else {
		// force this call.
		willHolster = (forceHolster == 1);
	}
	

	//MODDD - if the weapon isn't selectable, why bother?   BAIL.
	if (!argItem->CanDeploy())
	{
		return;
	}

	if (m_pActiveItem) {
		if (!m_bHolstering) {
			// don't holster the currently equipped weapon if already in the middle of holstering.
			m_bHolstering = TRUE;
			m_chargeReady |= 128;
			m_pActiveItem->Holster();
		}
		// TEST - is this a good idea?  If in the middle of holstering at the time, make the last-selected weapon
		// the one that holstering was going towards instead?  similar check in 'setActiveItem', if not only there?
		// if(m_pQueuedActiveItem != NULL){
		//     m_pLastItem = m_pQueuedActiveItem
		// }

		if (willHolster) {
			// using holster anim? Tell the currently equipped item to change to this weapon when that is over.
			m_pQueuedActiveItem = argItem;  //set this later instead, after the holster anim is done.
		}else {
			// Not using holster anims? Immediately change weapon.
			setActiveItem(argItem);
			m_bHolstering = FALSE;
		}
	}
	else {
		// just pick it now.
		setActiveItem(argItem);
	}
}//setActiveItem_HolsterCheck



// equips the previously equipped weapon.  "lastinv" in console;  q key by default
void CBasePlayer::SelectLastItem(void)
{
	if (!m_pLastItem)
	{
		return;
	}

	if ( m_pActiveItem && !m_pActiveItem->CanHolster() )
	{
		return;
	}

	ResetAutoaim( );



	/*
	// FIX, this needs to queue them up and delay
	if (m_pActiveItem)
		m_pActiveItem->Holster( );
	
	CBasePlayerItem *pTemp = m_pActiveItem;
	m_pActiveItem = m_pLastItem;
	m_pLastItem = pTemp;
	m_pActiveItem->Deploy( );
	m_pActiveItem->UpdateItemInfo( );
	*/

	//MODDD - replaced with this.

	// Record the current active item, setting it with LastItem forgets what it was otherwise.
	//CBasePlayerItem* pTemp = m_pActiveItem;
	setActiveItem_HolsterCheck(m_pLastItem, (fBreakHolster != TRUE) );
	// And the previous ActiveItem is now the 'Last' (recent) item in case of another switch-back.
	//m_pLastItem = pTemp;

}

//==============================================
// HasWeapons - do I have any weapons at all?
//==============================================
BOOL CBasePlayer::HasWeapons( void )
{
	int i;

	for ( i = 0 ; i < MAX_ITEM_TYPES ; i++ )
	{
		if ( m_rgpPlayerItems[ i ] )
		{
			return TRUE;
		}
	}

	return FALSE;
}

// whut.  Nothing calls this at least.
void CBasePlayer::SelectPrevItem( int iItem )
{
}


const char *CBasePlayer::TeamID( void )
{
	if ( pev == NULL )		// Not fully connected yet
		return "";

	// return their team name
	return m_szTeamName;
}


void CBasePlayer::GiveNamedItemIfLacking( const char *pszName ){
	if(!HasNamedPlayerItem(pszName)){
		GiveNamedItem(pszName);
	}
}

edict_t* CBasePlayer::GiveNamedItem( const char *pszName ){
	return GiveNamedItem(pszName, 0);
}

//send right to the player's origin like the retail "give" command does (which likely referred to this method now)
edict_t* CBasePlayer::GiveNamedItem( const char *pszName, int pszSpawnFlags  )
{
	char resultpre[128];
	strncpy( &resultpre[0], &pszName[0], 127 );
	resultpre[127] = '\0';
	lowercase(&resultpre[0]);

	//Case exceptions.  Some names DO use caps, unfortunately, so this does the correction to the guaranteed all-lowercased text to get the right entity text.
	
	//If we didn't precache everything, the air tank isn't either.
	if(EASY_CVAR_GET(precacheAll) == 0){
		
		if(FStrEq(resultpre, "item_airtank")){
			//just do the effect.  This will stop a possible precache error, since "give" implies we didn't need to see the model.


			if(airTankAirTime < PLAYER_AIRTANK_TIME_MAX){
			//pPlayer->SetSuitUpdate("!HEV_DET4", FALSE, SUIT_NEXT_IN_1MIN);
			airTankAirTime = PLAYER_AIRTANK_TIME_MAX;
		
			if(gmsgUpdateAirTankAirTime > 0){
				MESSAGE_BEGIN( MSG_ONE, gmsgUpdateAirTankAirTime, NULL, pev );
				WRITE_SHORT( airTankAirTime);
				MESSAGE_END();
			}
			MESSAGE_BEGIN( MSG_ONE, gmsgItemPickup, NULL, pev );
			WRITE_STRING( STRING(pev->classname) );
			MESSAGE_END();

			//MODDD QUESTION - does the precache sentence save system handle this, or is it guaranteed precached like other player sounds?
			UTIL_PlaySound( edict(), CHAN_ITEM, "items/airtank1.wav", 1, ATTN_NORM, TRUE );
			}
			return NULL;
		}

	}//END OF if(EASY_CVAR_GET(precacheAll) == 0)


	// Let this know that a spawned item should not be replaced by something that has physical bounds
	// (like chumtoads now).
	g_giveWithoutTargetLocation = TRUE;
	edict_t* thing = GiveNamedItem(pszName, pszSpawnFlags, pev->origin);
	g_giveWithoutTargetLocation = FALSE;

	// Because this is the default, sending to the origin, call "DispatchTouch" too.
	if(thing != NULL){
		DispatchTouch( thing, ENT( pev ) );
	}

	// spawn flag checking is already done in the above "GiveNamedItem".

	return thing;
}


edict_t* CBasePlayer::GiveNamedItem( const char *pszName, float xCoord, float yCoord, float zCoord ){
	return GiveNamedItem(pszName, NULL, Vector(xCoord, yCoord, zCoord), FALSE, NULL);
}

edict_t* CBasePlayer::GiveNamedItem( const char *pszName, float xCoord, float yCoord, float zCoord, BOOL factorSpawnSize ){
	return GiveNamedItem(pszName, NULL, Vector(xCoord, yCoord, zCoord), factorSpawnSize, NULL);
}
edict_t* CBasePlayer::GiveNamedItem( const char *pszName, const Vector& coord ){
	return GiveNamedItem(pszName, NULL, coord, FALSE, NULL);
}
edict_t* CBasePlayer::GiveNamedItem( const char *pszName, const Vector& coord, BOOL factorSpawnSize ){
	return GiveNamedItem(pszName, NULL, coord, factorSpawnSize, NULL);
}


edict_t* CBasePlayer::GiveNamedItem( const char *pszName, int pszSpawnFlags, float xCoord, float yCoord, float zCoord ){
	return GiveNamedItem(pszName, pszSpawnFlags, Vector(xCoord, yCoord, zCoord), FALSE, NULL);
}

edict_t* CBasePlayer::GiveNamedItem( const char *pszName, int pszSpawnFlags, float xCoord, float yCoord, float zCoord, BOOL factorSpawnSize){
	return GiveNamedItem(pszName, pszSpawnFlags, Vector(xCoord, yCoord, zCoord), factorSpawnSize, NULL);
}
edict_t* CBasePlayer::GiveNamedItem( const char *pszName, int pszSpawnFlags, const Vector& coord){
	return GiveNamedItem(pszName, pszSpawnFlags, coord, FALSE, NULL);
}


edict_t* CBasePlayer::GiveNamedItem( const char *pszName, int pszSpawnFlags, const Vector& coord, BOOL factorSpawnSize ){
	return GiveNamedItem(pszName, pszSpawnFlags, coord, factorSpawnSize, NULL);
}

edict_t* CBasePlayer::GiveNamedItem( const char *pszName, int pszSpawnFlags, float xCoord, float yCoord, float zCoord, BOOL factorSpawnSize, TraceResult* tr ){
	return GiveNamedItem(pszName, pszSpawnFlags, Vector(xCoord, yCoord, zCoord), factorSpawnSize, tr);
}

// NOTE - the 'TraceResult* tr' is now unused?  whoops.
edict_t* CBasePlayer::GiveNamedItem( const char *pszName, int pszSpawnFlags, const Vector& coord, BOOL factorSpawnSize, TraceResult* tr )
{
	edict_t	*pent;
	int istr;
	entvars_t *pentpev;
	
	int iszItem;
	char* pszNameFinal;  //will just point at the name used to (attempt to?) genreate the object.

	char resultpre[128];
	char result[128];
	strncpy( &resultpre[0], &pszName[0], 127 );
	resultpre[127] = '\0';
	lowercase(&resultpre[0]);

	//Case exceptions.  Some names DO use caps, unfortunately, so this does the correction to the guaranteed all-lowercased text to get the right entity text.
	if(FStrEq(resultpre, "weapon_9mmar")){
		strcpy(resultpre, "weapon_9mmAR\0");
		//strcpy
	}else if(FStrEq(resultpre, "ammo_argrenades")){
		strcpy(resultpre, "ammo_ARgrenades\0");
	}else if(FStrEq(resultpre, "ammo_argrenades")){
		strcpy(resultpre, "ammo_ARgrenades\0");
	}else if(FStrEq(resultpre, "ammo_9mmar")){
		strcpy(resultpre, "ammo_9mmAR\0");
	}

	pent = overyLongComplicatedProcessForCreatingAnEntity(&resultpre[0]);
	pszNameFinal = &resultpre[0];
	
	//MODDD - pre-check.
	if ( FNullEnt( pent ) ){
		// didn't work?  Try with or without "monster_" in front, whichever was missing.
		// Monsters are the most commonly wanted spawn after all.
		
		if(!stringStartsWith(resultpre, "monster_")){
			// lack it?  Try with it.
			// Put "monster_" in there first.
			strncpy(&result[0], "monster_", 8);
			// and the rest of the tried name after that.
			strncpy( &result[8], &resultpre[0], 127-8 );
			result[127] = '\0';

		}else{
			// have it already?  Try without it.
			// Just put the tried name in, 8 characters in (cuts off "monster_").
			strncpy( &result[0], &resultpre[8], 127-8 );
			result[127] = '\0';
		}
		//try putting "monster_" in front?
		
		// try once more.
		pent = overyLongComplicatedProcessForCreatingAnEntity(result);
		pszNameFinal = &result[0];
	}
	
	//int istr = MAKE_STRING( boot );
	//pent = CREATE_NAMED_ENTITY(istr);
	if ( FNullEnt( pent ) )
	{
		ALERT ( at_console, "NULL Ent in GiveNamedItem!\n" );
		return NULL;
	}

	CBaseEntity* temptest = Instance(pent);

	// If the "monster_barancle" was spawned, push it down a bit to be safe.  If any of it clips
	// through the ceiling, the tongue won't drop.
	float extraOffset = 0;
	//easyPrintLine("WHAT %s", pszNameFinal);
	if(FStrEq(pszNameFinal, "monster_barnacle") || FStrEq(pszNameFinal, "barnacle")  ){
		extraOffset = -2;
	}

	int forceDynamicIndex = 0;
	//-1 = force non-dynamic (as though by map)
	//0 = inactive.
	//1 = force dynamic.

	int addedSpawnFlags = 0;
	Vector coordMod(0,0,0);

	//TODO: to make name-lookups easier, add a virtual method to the monster class named 
	//"getPreferredClassname".  That would return "monster_stuka" for bats regardless of what was
	//used to spawn it.

	/*
	//Nope, no more special rules for you.
	if(FStrEq(pszNameFinal, "monster_stuka") || FStrEq(pszNameFinal, "stuka") || FStrEq(pszNameFinal, "monster_stukabat") || FStrEq(pszNameFinal, "monster_stuka")  ){
		//extraOffset = -2;
		
		if(tr != NULL){

			//STUKA DIMENSIONS:
			//UTIL_SetSize( pev, Vector( -8, -8, 0 ), Vector( 8, 8, 28 ));

			if(tr->vecPlaneNormal[2] < 0.3 && tr->vecPlaneNormal[2] > -0.3){
				//completely flat-ways surface selected for placement.
				coordMod.x += tr->vecPlaneNormal[0] * 5;
				coordMod.y += tr->vecPlaneNormal[1] * 5;
				coordMod.z += tr->vecPlaneNormal[2] * 5;
			}else if(tr->vecPlaneNormal[2] < -0.3){
				//placed on a ceiling, mark as such.
				//addedSpawnFlags |= SF_STUKABAT_CEILING;
				//no, the flag is "SF_MONSTER_STUKA_ONGROUND"...
				//Force this as a non-dynamic spawn and it will snap to  the ceiling without the flag.

				coordMod.z -= 28;

				forceDynamicIndex = -1;
			}

		}//END OF if(tr != NULL)

	}//END OF if(<stukabat spawned>)
	*/

	//easyPrintLine("DAHHH %s :::%.2f", STRING(pent->v.classname), extraOffset);

	pentpev = VARS( pent );
	
	if(factorSpawnSize){

		pentpev->origin = Vector(	coord.x + coordMod.x - (pentpev->size.x / 2),
										coord.y + coordMod.y - (pentpev->size.y / 2),
										coord.z + coordMod.z - pentpev->size.z + extraOffset
										);

	}else{
		pentpev->origin = Vector(	coord.x + coordMod.x,
										coord.y + coordMod.y,
										coord.z + coordMod.z + extraOffset
										);
	}

	//if( forceDynamicIndex != -1 && (isStringEmpty(pszSpawnFlags) || forceDynamicIndex == 1)  ){
	if( forceDynamicIndex != -1 && (pszSpawnFlags==-1 || forceDynamicIndex == 1)  ){
		//if we were given a null spawn flag, tell the spawn about this.
		if(temptest != NULL){
			temptest->DefaultSpawnNotice();
			
		}else{
			easyPrintLine("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
			easyPrintLine("GIVE-SPAWN ERROR PLEASE REPORT2");
			easyPrintLine("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
		}

	}else{
		//see if there are any spawn flags from the client (in this spawn call)
		//int spawnFlagValue = attemptInterpretSpawnFlag(pszSpawnFlags);
		int spawnFlagValue = pszSpawnFlags;
		if(spawnFlagValue >= 0){
			//pent->v.spawnflags = spawnFlagValue;
			if(temptest != NULL){
				temptest->ForceSpawnFlag(spawnFlagValue);
			
			}else{
				easyPrintLine("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
				easyPrintLine("GIVE-SPAWN ERROR PLEASE REPORT");
				easyPrintLine("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
			}
		}

	}
	//IMPORTANT! Anything spawned by me is also "non-respawnable", regardless of the suggested flag (if any).
	pent->v.spawnflags |= SF_NORESPAWN;

	//virtual BOOL isBasePlayerWeapon(void){return FALSE;};
	

	if (!g_giveWithoutTargetLocation) {
		if (temptest->isBasePlayerWeapon()) {
			CBasePlayerWeapon* tempWeap = static_cast<CBasePlayerWeapon*>(temptest);
			//If the thing we called for can spawn a pickupWalker, just skip to doing that instead.
			const char* pickupWalkerNameTest = tempWeap->GetPickupWalkerName();
			if (::isStringEmpty(pickupWalkerNameTest)) {
				// no walker? nothing unusual.
				// don't ignore this flag. though.
			}
			else if (!(temptest->pev->spawnflags & SF_PICKUP_NOREPLACE)) {
				// there is a walker! Just skip to spawning that instead.
				CBaseEntity* newWalker = tempWeap->pickupWalkerReplaceCheck();
				// Unless there is any other intervention like ending in "_noReplace".
				if (newWalker != NULL) {
					//send the walker instead.
					::UTIL_Remove(temptest);
					edict_t* myEd = ENT(newWalker->pev);

					// NOTE - no need to call DispatchSpawn!  The pickupWalkerReplaceCheck already does that.
					//DispatchSpawn(myEd);

					return myEd;
				}

				//...or call giveNamedItem, using the walker name instead?  Choices, choices!
			}
		}

		DispatchSpawn(pent);
	}
	else {


		if (temptest->isBasePlayerWeapon()) {
			CBasePlayerWeapon* tempWeap = static_cast<CBasePlayerWeapon*>(temptest);
			//If the thing we called for can spawn a pickupWalker, just skip to doing that instead.
			const char* pickupWalkerNameTest = tempWeap->GetPickupWalkerName();
			if (::isStringEmpty(pickupWalkerNameTest)) {
				// no walker? nothing unusual.
				// don't ignore this flag. though.
			}
			else {
				// ok
				temptest->pev->spawnflags |= SF_PICKUP_NOREPLACE;
			}
		}
		DispatchSpawn(pent);

	}//END OF g_giveWithoutTargetLocation check



	return pent;
}


//MODDD - "findEntityForward" has been moved to util.cpp.

BOOL CBasePlayer::FlashlightIsOn( void )
{
	return FBitSet(pev->effects, EF_DIMLIGHT);
}

void CBasePlayer::FlashlightTurnOn( void )
{
	if ( !g_pGameRules->FAllowFlashlight() )
	{
		return;
	}

	if ( (pev->weapons & (1<<WEAPON_SUIT)) )
	{
		//MODDD - channel changed from "CHAN_WEAPON" to "CHAN_STREAM".
		//MODDD - soundsentencesave
		UTIL_PlaySound( ENT(pev), CHAN_STREAM, SOUND_FLASHLIGHT_ON, 1.0, ATTN_NORM, 0, PITCH_NORM, FALSE );
		SetBits(pev->effects, EF_DIMLIGHT);
		MESSAGE_BEGIN( MSG_ONE, gmsgFlashlight, NULL, pev );
		WRITE_BYTE(1);
		WRITE_BYTE(m_iFlashBattery);
		MESSAGE_END();

		m_flFlashLightTime = FLASH_DRAIN_TIME + gpGlobals->time;

	}
}


void CBasePlayer::FlashlightTurnOff( void )
{
	//MODDD - channel changed from "CHAN_WEAPON" to "CHAN_STREAM".
	//MODDD - soundsentencesave
	UTIL_PlaySound( ENT(pev), CHAN_STREAM, SOUND_FLASHLIGHT_OFF, 1.0, ATTN_NORM, 0, PITCH_NORM, FALSE );
    ClearBits(pev->effects, EF_DIMLIGHT);
	MESSAGE_BEGIN( MSG_ONE, gmsgFlashlight, NULL, pev );
	WRITE_BYTE(0);
	WRITE_BYTE(m_iFlashBattery);
	MESSAGE_END();

	m_flFlashLightTime = FLASH_CHARGE_TIME + gpGlobals->time;

}

/*
===============
ForceClientDllUpdate

When recording a demo, we need to have the server tell us the entire client state
so that the client side .dll can behave correctly.
Reset stuff so that the state is transmitted.
===============
*/
void CBasePlayer::ForceClientDllUpdate( void )
{

	m_iClientHealth  = -1;
	m_iClientBattery = -1;

	
	m_iTrain |= TRAIN_NEW;  // Force new train message.
	m_fWeapon = FALSE;          // Force weapon send
	m_fKnownItem = FALSE;    // Force weaponinit messages.
	m_fInitHUD = TRUE;		// Force HUD gmsgResetHUD message
	

	commonReset();

	// Now force all the necessary messages
	//  to be sent.
	UpdateClientData();
}

/*
============
ImpulseCommands
============
*/

void CBasePlayer::ImpulseCommands( )
{
	TraceResult	tr;// UNDONE: kill me! This is temporary for PreAlpha CDs

	// Handle use events
	PlayerUse();
		
	int iImpulse = (int)pev->impulse;

	//MODDD - why wasn't there a 0 check here? Skip the checks below if we're still 0 from using a sent impulse already.
	if(iImpulse == 0){
		return;
	}


	switch (iImpulse)
	{
	case 99:
		{

		int iOn;

		if (!gmsgLogo)
		{
			iOn = 1;
			gmsgLogo = REG_USER_MSG("Logo", 1);
		} 
		else 
		{
			iOn = 0;
		}
		
		ASSERT( gmsgLogo > 0 );
		// send "health" update message
		MESSAGE_BEGIN( MSG_ONE, gmsgLogo, NULL, pev );
			WRITE_BYTE(iOn);
		MESSAGE_END();

		if(!iOn)
			gmsgLogo = 0;
		break;
		}
	case 100:
        // temporary flashlight for level designers
		//MODDD NOTE - and this is the same mechanism for turning the player flashlight on/off even in ordinary ingame use apparently.
        if ( FlashlightIsOn() )
		{
			FlashlightTurnOff();
		}
        else 
		{
			FlashlightTurnOn();
		}
		break;

	case 201:// paint decal
		
		
		if ( gpGlobals->time < m_flNextDecalTime )
		{
			// too early!
			break;
		}
		

		UTIL_MakeVectors(pev->v_angle);
		UTIL_TraceLine ( pev->origin + pev->view_ofs, pev->origin + pev->view_ofs + gpGlobals->v_forward * 128, ignore_monsters, ENT(pev), & tr);

		if ( tr.flFraction != 1.0 )
		{// line hit something, so paint a decal
			m_flNextDecalTime = gpGlobals->time + decalfrequency.value;
			CSprayCan *pCan = GetClassPtr((CSprayCan *)NULL);
			pCan->Spawn( pev );
		}

		break;

	default:
		// check all of the cheat impulse commands now
		CheatImpulseCommands( iImpulse );
		break;
	}
	
	pev->impulse = 0;
}


//=========================================================
//=========================================================
void CBasePlayer::CheatImpulseCommands( int iImpulse )
{
	/*
	// necessary...?  maybe not.   Nope, definitely not, only needs to be checked once 
	// in server logic (client.cpp).
	if(cvar_sv_cheats == 0){
		cvar_sv_cheats = CVAR_GET_POINTER( "sv_cheats" );
	}
	
	//MODDD - update "g_flWeaponCheat" to what sv_cheats is.
	if(cvar_sv_cheats != 0){
		if(cvar_sv_cheats->value == 1){
			g_flWeaponCheat = 1;
		}else{
			g_flWeaponCheat = 0;
		}
	}
	*/

	if ( g_flWeaponCheat == 0.0 )
	{
		return;
	}

	CBaseEntity *pEntity;

	switch ( iImpulse )
	{
	case 76:
		{
			if (!giPrecacheGrunt)
			{
				giPrecacheGrunt = 1;
				ALERT(at_console, "You must now restart to use Grunt-o-matic.\n");
			}
			else
			{
				UTIL_MakeVectors( Vector( 0, pev->v_angle.y, 0 ) );
				Create("monster_human_grunt", pev->origin + gpGlobals->v_forward * 128, pev->angles);
			}
			break;
		}

		
	case 101: {
		//int i;

		// EHHhhh why not.
		globalflag_muteDeploySound = TRUE;

		gEvilImpulse101 = TRUE;
		GiveNamedItem("item_suit");
		GiveNamedItem("item_battery");
		GiveNamedItem("weapon_crowbar");
		GiveNamedItem("weapon_9mmhandgun");
		GiveNamedItem("ammo_9mmclip");
		GiveNamedItem("weapon_shotgun");
		GiveNamedItem("ammo_buckshot");
		GiveNamedItem("weapon_9mmAR");
		GiveNamedItem("ammo_9mmAR");
		GiveNamedItem("ammo_ARgrenades");
		GiveNamedItem("weapon_handgrenade");
		GiveNamedItem("weapon_tripmine");
		GiveNamedItem("weapon_357");
		GiveNamedItem("ammo_357");
		GiveNamedItem("weapon_crossbow");
		GiveNamedItem("ammo_crossbow");
		GiveNamedItem("weapon_egon");
		GiveNamedItem("weapon_gauss");
		GiveNamedItem("ammo_gaussclip");
		GiveNamedItem("weapon_rpg");
		GiveNamedItem("ammo_rpgclip");
		GiveNamedItem("weapon_satchel");
		GiveNamedItem("weapon_snark");
		GiveNamedItem("weapon_hornetgun");

		if (!IsMultiplayer()) {
			GiveNamedItem("weapon_chumtoad");
		}

		GiveNamedItem("item_antidote");
		GiveNamedItem("item_adrenaline");
		GiveNamedItem("item_radiation");

		globalflag_muteDeploySound = FALSE;
			
			
		//MODDD - NEW.  Have some perks of 'everything'.
		setHealth(100);
		setArmorBattery(100);
		airTankAirTime = PLAYER_AIRTANK_TIME_MAX;
		
		//for (i = 0; i < MAX_ITEMS; i++) {
		///	m_rgItems[?] = ?;
		//}



		if (m_fLongJump != TRUE) {
			longJumpChargeNeedsUpdate = TRUE;
		}
		m_fLongJump = TRUE;
		longJumpCharge = PLAYER_LONGJUMPCHARGE_MAX;

		gEvilImpulse101 = FALSE;
	}
		break;

	case 102:
		// Gibbage!!!
		CGib::SpawnRandomGibs( pev, 1, GIB_HUMAN_ID );
		break;

	case 103:
		// What the hell are you doing?
		pEntity = FindEntityForward( this );
		if ( pEntity )
		{
			CBaseMonster *pMonster = pEntity->MyMonsterPointer();
			if ( pMonster )
				pMonster->ReportAIState();
		}
		break;

	case 104:
		// Dump all of the global state varaibles (and global entity names)
		gGlobalState.DumpGlobals();
		break;

	case 105:// player makes no sound for monsters to hear.
		{
			if ( m_fNoPlayerSound )
			{
				ALERT ( at_console, "Player is audible\n" );
				m_fNoPlayerSound = FALSE;
			}
			else
			{
				ALERT ( at_console, "Player is silent\n" );
				m_fNoPlayerSound = TRUE;
			}
			break;
		}

	case 106:
		// Give me the classname and targetname of this entity.
		pEntity = FindEntityForward( this );
		if ( pEntity )
		{
			ALERT ( at_console, "Classname: %s", STRING( pEntity->pev->classname ) );
			
			if ( !FStringNull ( pEntity->pev->targetname ) )
			{
				ALERT ( at_console, " - Targetname: %s\n", STRING( pEntity->pev->targetname ) );
			}
			else
			{
				ALERT ( at_console, " - TargetName: No Targetname\n" );
			}

			ALERT ( at_console, "Model: %s\n", STRING( pEntity->pev->model ) );
			if ( pEntity->pev->globalname )
				ALERT ( at_console, "Globalname: %s\n", STRING( pEntity->pev->globalname ) );
		}
		break;

	case 107:
		{
			TraceResult tr;

			edict_t		*pWorld = g_engfuncs.pfnPEntityOfEntIndex( 0 );

			Vector start = pev->origin + pev->view_ofs;
			Vector end = start + gpGlobals->v_forward * 1024;
			UTIL_TraceLine( start, end, ignore_monsters, edict(), &tr );
			if ( tr.pHit )
				pWorld = tr.pHit;
			const char *pTextureName = TRACE_TEXTURE( pWorld, start, end );
			if ( pTextureName )
				ALERT( at_console, "Texture: %s\n", pTextureName );
		}
		break;
	case 195:// show shortest paths for entire level to nearest node
		{
			Create("node_viewer_fly", pev->origin, pev->angles);
		}
		break;
	case 196:// show shortest paths for entire level to nearest node
		{
			Create("node_viewer_large", pev->origin, pev->angles);
		}
		break;
	case 197:// show shortest paths for entire level to nearest node
		{
			Create("node_viewer_human", pev->origin, pev->angles);
		}
		break;
	case 199:// show nearest node and all connections
		{
			ALERT ( at_console, "%d\n", WorldGraph.FindNearestNode ( pev->origin, bits_NODE_GROUP_REALM ) );
			WorldGraph.ShowNodeConnections ( WorldGraph.FindNearestNode ( pev->origin, bits_NODE_GROUP_REALM ) );
		}
		break;
	case 202:// Random blood splatter
		{
			TraceResult tr;
			UTIL_MakeVectors(pev->v_angle);
			// only a distance of 128.  why.
			UTIL_TraceLine(pev->origin + pev->view_ofs, pev->origin + pev->view_ofs + gpGlobals->v_forward * 600, ignore_monsters, ENT(pev), &tr);

			if (tr.flFraction != 1.0)
			{// line hit something, so paint a decal
				CBloodSplat* pBlood = GetClassPtr((CBloodSplat*)NULL);
				pBlood->Spawn(pev);
			}
		}
	break;
	case 203:// remove creature.
		pEntity = FindEntityForward( this );
		if ( pEntity )
		{
			if ( pEntity->pev->takedamage )
				pEntity->SetThink(&CBaseEntity::SUB_Remove);
		}
		break;
	}
}

//
// Add a weapon to the player (Item == Weapon == Selectable Object)
//
BOOL CBasePlayer::AddPlayerItem( CBasePlayerItem *pItem )
{
	CBasePlayerItem *pInsert;
	
	pInsert = m_rgpPlayerItems[pItem->iItemSlot()];

	while (pInsert)
	{
		if (FClassnameIs( pInsert->pev, STRING( pItem->pev->classname) ))
		{
			if (pItem->AddDuplicate( pInsert ))
			{
				g_pGameRules->PlayerGotWeapon ( this, pItem );
				pItem->CheckRespawn();

				// ugly hack to update clip w/o an update clip message
				pInsert->UpdateItemInfo( );
				if (m_pActiveItem)
					m_pActiveItem->UpdateItemInfo( );

				pItem->Kill( );
			}
			else if (gEvilImpulse101)
			{
				// FIXME: remove anyway for deathmatch testing
				pItem->Kill( );
			}
			//easyForcePrintLine("ALREADY GOT ME");
			return FALSE;
		}
		pInsert = pInsert->m_pNext;
	}


	if (pItem->AddToPlayer( this ))
	{
		g_pGameRules->PlayerGotWeapon ( this, pItem );
		pItem->CheckRespawn();

		pItem->m_pNext = m_rgpPlayerItems[pItem->iItemSlot()];
		m_rgpPlayerItems[pItem->iItemSlot()] = pItem;

		// should we switch to this item?
		//MODDD - don't if we're using cheats to achieve this, annoying to change weapons automatically.
		// If lacking any weapon, at least switch to the first granted one though.
		if(!globalflag_muteDeploySound || m_pActiveItem == NULL){
			if ( g_pGameRules->FShouldSwitchWeapon( this, pItem ) )
			{
				SwitchWeapon( pItem );
			}
		}


		return TRUE;
	}
	else if (gEvilImpulse101)
	{
		// FIXME: remove anyway for deathmatch testing
		pItem->Kill( );
	}
	return FALSE;
}//END OF AddPlayerItem



void CBasePlayer::printOutWeapons(void){
	//CBasePlayerWeapon whut;
	//whut.PrintState();

	for(int i = 0; i < 6; i++){
		int i2 = 0;
		CBasePlayerItem* thisItem = m_rgpPlayerItems[i];
		while(thisItem){

			easyForcePrintLine("slot:%d row:%d %s", i, i2, STRING(thisItem->pev->classname) );
			i2++;
			thisItem = thisItem->m_pNext;
		}//END OF while
	}//END OF for

}//END OF printOutWeapons



//TEST: see if, given an item with this iItemSlot and classname string, we are capable of adding it (not out of ammo).
BOOL CBasePlayer::CanAddPlayerItem( int arg_iItemSlot, const char* arg_classname, const char* arg_ammoname, int arg_iMaxAmmo)
{
	//printOutWeapons();

	CBasePlayerItem *pInsert;
	
	pInsert = m_rgpPlayerItems[arg_iItemSlot];

	while (pInsert)
	{

		//easyForcePrintLine("MATCH TEST?? %s", STRING(pInsert->pev->classname) ); 
		if (FClassnameIs( pInsert->pev, arg_classname ))
		{
			//if (pItem->AddDuplicate( pInsert ))
			if(g_pGameRules->CanHaveAmmo(this, arg_ammoname, arg_iMaxAmmo) )
			{
				//g_pGameRules->PlayerGotWeapon ( this, pItem );
				//pItem->CheckRespawn();

				// ugly hack to update clip w/o an update clip message
				/*
				pInsert->UpdateItemInfo( );
				if (m_pActiveItem)
					m_pActiveItem->UpdateItemInfo( );

				pItem->Kill( );
				*/
				return TRUE;  //we can add it!
			}
			else 
			{
				//The player already has this item, but this item rejects taking ammo? Denied.
				//return FALSE;
				//...oh, already falls to FALSE below. neat.
				//pItem->Kill( );
			}
			//easyForcePrintLine("ALREADY GOT ME SONNY");
			return FALSE;
		}
		pInsert = pInsert->m_pNext;
	}


	//For now, if the player doesn't already have it, just assume "yes".

	return TRUE;
}//END OF CanAddPlayerItem


int CBasePlayer::RemovePlayerItem( CBasePlayerItem *pItem )
{
	if (m_pActiveItem == pItem)
	{
		ResetAutoaim( );
		pItem->Holster( );
		pItem->pev->nextthink = 0;// crowbar may be trying to swing again, etc.
		pItem->SetThink( NULL );
		m_pActiveItem = NULL;
		pev->viewmodel = 0;
		pev->weaponmodel = 0;
	}
	else if ( m_pLastItem == pItem )
		m_pLastItem = NULL;

	CBasePlayerItem *pPrev = m_rgpPlayerItems[pItem->iItemSlot()];

	if (pPrev == pItem)
	{
		m_rgpPlayerItems[pItem->iItemSlot()] = pItem->m_pNext;
		return TRUE;
	}
	else
	{
		while (pPrev && pPrev->m_pNext != pItem)
		{
			pPrev = pPrev->m_pNext;
		}
		if (pPrev)
		{
			pPrev->m_pNext = pItem->m_pNext;
			return TRUE;
		}
	}
	return FALSE;
}


//
// Returns the unique ID for the ammo, or -1 if error
//
int CBasePlayer::GiveAmmo( int iCount, const char* szName, int iMax )
{
	if ( !szName )
	{
		// no ammo.
		return -1;
	}

	if ( !g_pGameRules->CanHaveAmmo( this, szName, iMax ) )
	{
		// game rules say I can't have any more of this ammo type.
		return -1;
	}

	//MODDD - Nothing about 'i' nor szName changes between now and WRITE_BYTE below?  Just send 'i' then!
	const int i = GetAmmoIndex( szName );

	if ( i < 0 || i >= MAX_AMMO_TYPES )
		return -1;

	int iAdd = min( iCount, iMax - m_rgAmmo[i] );
	if ( iAdd < 1 )
		return i;

	m_rgAmmo[ i ] += iAdd;


	if ( gmsgAmmoPickup )  // make sure the ammo messages have been linked first
	{
		// Send the message that ammo has been picked up
		MESSAGE_BEGIN( MSG_ONE, gmsgAmmoPickup, NULL, pev );
			WRITE_BYTE( i );		// ammo ID
			WRITE_BYTE( iAdd );		// amount
		MESSAGE_END();
	}

	TabulateAmmo();

	return i;
}



//MODDD - new version, given the ammo type ID instead of the name of the ammo.
int CBasePlayer::GiveAmmoID(int iCount, int iAmmoTypeId, int iMax)
{
	if(!IS_AMMOTYPE_VALID(iAmmoTypeId))
	{
		// no ammo.
		return -1;
	}

	if (!g_pGameRules->CanHaveAmmo(this, iAmmoTypeId, iMax))
	{
		// game rules say I can't have any more of this ammo type.
		return -1;
	}

	const int i = iAmmoTypeId;

	int iAdd = min(iCount, iMax - m_rgAmmo[i]);
	if (iAdd < 1)
		return i;

	m_rgAmmo[i] += iAdd;


	if (gmsgAmmoPickup)  // make sure the ammo messages have been linked first
	{
		// Send the message that ammo has been picked up
		MESSAGE_BEGIN(MSG_ONE, gmsgAmmoPickup, NULL, pev);
		WRITE_BYTE(i);		// ammo ID
		WRITE_BYTE(iAdd);		// amount
		MESSAGE_END();
	}

	TabulateAmmo();

	return i;
}





/*
============
ItemPreFrame

Called every frame by the player PreThink
============
*/
void CBasePlayer::ItemPreFrame()
{
	//if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(testVar) == -1)return;


	// Even though ItemPostFrame() turns this off faster, it turns it off too fast. At least one full frame must run
	// with the "res" physics flag left on to send to the client to be effective and block the jump-land sound.
	if(pev->iuser4 & FLAG_RESET_RECEIVED){
		pev->iuser4 &= ~FLAG_RESET_RECEIVED;
		g_engfuncs.pfnSetPhysicsKeyValue( edict(), "res", "0" );
		//turn this phyiscs flag off, we got the signal back.
		//easyForcePrintLine("HOLY thing thinger-thinger");
	}


	//MODDD - moved here.  May as well terminate here if NULL, so that the new "think" method only happens if at least there is an item to "think" for.
	if (!m_pActiveItem)
		return;



	/*
	//MODDD - just the first time, in case of load / starting out with an item
	// NO.
	if(globalPSEUDO_forceFirstPersonIdleDelay == 1){
		CBasePlayerWeapon* weapTest = (CBasePlayerWeapon*)m_pActiveItem->GetWeaponPtr();
		if(weapTest != NULL){
			//forceNoWeaponLoop = TRUE;
			weapTest->forceBlockLooping();
			weapTest->m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + weapTest->randomIdleAnimationDelay();
			globalPSEUDO_forceFirstPersonIdleDelay = 0;
		}
	}
	*/


	//MODDD - reference to new method.  Override this method in any item / weapon to get a "think"
	//frame method that does notdepend on "m_flNextAttack" not being used at the moment.
	m_pActiveItem->ItemPreFrameThink( );

#if defined( CLIENT_WEAPONS )
    if ( m_flNextAttack > 0 )
#else
    if ( gpGlobals->time < m_flNextAttack )
#endif
	{
		return;
	}

	m_pActiveItem->ItemPreFrame( );
}


/*
============
ItemPostFrame

Called every frame by the player PostThink
============
*/
void CBasePlayer::ItemPostFrame()
{
	static int fInSelect = FALSE;

	// check if the player is using a tank
	if ( m_pTank != NULL )
		return;

	
	BOOL canCallItemPostFrame = TRUE;

	//MODDD - moved here.  May as well terminate here if NULL, so that the new "think" method only happens if at least there is an item to "think" for.
	//(just use a bool for more control)
	if (!m_pActiveItem)
		canCallItemPostFrame = FALSE;

	//MODDD - reference to new method.  Override this method in any item / weapon to get a "think"
	//frame method that does notdepend on "m_flNextAttack" not being used at the moment.
	if(canCallItemPostFrame){
		m_pActiveItem->ItemPostFrameThink( );
	}
	
	//MODDD - new.  No need to keep counting after that
	if(m_framesSinceRestore < 50){
		m_framesSinceRestore++;
	}

	//MODDDD - now always done, BUT with a few edits to ensure the same logic.
	//MODDDD - no, reverted to normal for now...
	//m_pActiveItem->ItemPostFrame( );
	


	
	//MODDD - also moved here.  "ImpulseCommands", also responsible for the "use" key, should happen regardless of having a weapon or not.
	//MODDD - yet another move! Now above m_flNextAttack for faster flashlight action.
	ImpulseCommands();
		

#if defined( CLIENT_WEAPONS )
	//easyForcePrintLine("AAA I WILL be a decent fellow %.2f", m_flNextAttack);
    if ( m_flNextAttack > 0 )
#else
	//easyForcePrintLine("BBB I WILL be a decent fellow %.2f %.2f", gpGlobals->time, m_flNextAttack);
    if ( gpGlobals->time < m_flNextAttack )
#endif
	{
		return;
	}


	//MODDD - this is done (the if-then) instead so that "impulseCommands" can happen regardless of whether the weapon is null or not.
	if(canCallItemPostFrame){
		m_pActiveItem->ItemPostFrame( );
	}
	
}

int CBasePlayer::AmmoInventory( int iAmmoIndex )
{
	if (iAmmoIndex == -1)
	{
		return -1;
	}

	return m_rgAmmo[ iAmmoIndex ];
}



// Called from UpdateClientData
// makes sure the client has all the necessary ammo info,  if values have changed
void CBasePlayer::SendAmmoUpdate(void)
{
	for (int i=0; i < MAX_AMMO_TYPES;i++)
	{

		//2 is the glock ID.
		/*
		if(i == 2){
			easyPrintLine("GLOCK AMMO: ! %d, %d, %d", i,  m_rgAmmo[i], m_rgAmmoLast[i]);
		}
		*/

		if (m_rgAmmo[i] != m_rgAmmoLast[i])
		{
			m_rgAmmoLast[i] = m_rgAmmo[i];

			ASSERT( m_rgAmmo[i] >= 0 );
			ASSERT( m_rgAmmo[i] < 255 );

			// send "Ammo" update message
			MESSAGE_BEGIN( MSG_ONE, gmsgAmmoX, NULL, pev );
				WRITE_BYTE( i );
				WRITE_BYTE( max( min( m_rgAmmo[i], 254 ), 0 ) );  // clamp the value to one byte
			MESSAGE_END();
		}
	}
}


/*
=========================================================
	UpdateClientData

resends any changed player HUD info to the client.
Called every frame by PlayerPreThink
Also called at start of demo recording and playback by
ForceClientDllUpdate to ensure the demo gets messages
reflecting all of the HUD state info.
=========================================================
*/
void CBasePlayer::UpdateClientData( void )
{

	if(this->superDuperDelay == -2){
		superDuperDelay = gpGlobals->time + 1;
	}


	if (m_fInitHUD)
	{
		m_fInitHUD = FALSE;
		gInitHUD = FALSE;

		MESSAGE_BEGIN( MSG_ONE, gmsgResetHUD, NULL, pev );
			WRITE_BYTE( 0 );
		MESSAGE_END();

		
		if ( !m_fGameHUDInitialized )
		{
			MESSAGE_BEGIN( MSG_ONE, gmsgInitHUD, NULL, pev );
			MESSAGE_END();

			g_pGameRules->InitHUD( this );
			m_fGameHUDInitialized = TRUE;
			if ( IsMultiplayer() )
			{
				FireTargets( "game_playerjoin", this, this, USE_TOGGLE, 0 );
			}
		}

		FireTargets( "game_playerspawn", this, this, USE_TOGGLE, 0 );
		InitStatusBar();
	}

	if ( m_iHideHUD != m_iClientHideHUD )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgHideWeapon, NULL, pev );
			WRITE_BYTE( m_iHideHUD );
		MESSAGE_END();

		m_iClientHideHUD = m_iHideHUD;
	}

	//easyPrintLine("EEEEEEE %d", m_rgbTimeBasedDamage[itbd_Bleeding]);
	//easyPrintLine("EEDDD %u %u, %u, %u", 1 << 28, 1 << 29, 1 << 30, 1 << 31);


	/*
	if(the_default_fov == NULL){
		the_default_fov = CVAR_GET_POINTER( "default_fov" );
		//g_engfuncs.pfnSetPhysicsKeyValue( edict(), "slj", "0" );
	}
	*/
	//easyPrintLine("??????? %d", gpGlobals->maxEntities);
	


	if(superDuperDelay > -1 && superDuperDelay < gpGlobals->time){
		MESSAGE_BEGIN( MSG_ONE, gmsgAutoMus, NULL, pev );
		MESSAGE_END();

		//superDuperDelay = -1;
		superDuperDelay = gpGlobals->time + 60*4.5;
	}


	//SCOPE
	{
	BOOL tempB = ( (pev->flags & FL_FROZEN) != 0);
	if(iWasFrozenToday != tempB ){
		iWasFrozenToday = tempB;
		//easyPrintLine("I WAS FROZEN, TODAY!? %d", tempB);
		MESSAGE_BEGIN( MSG_ONE, gmsgUpdateFreezeStatus, NULL, pev );
			WRITE_BYTE( tempB );
		MESSAGE_END();
	}

	}//END OF SCOPE


	//NOTICE - this physics key, psm (push speed multiplier) is NOT controlled by CVar, hidden or not.
	//         Pushing a crate with varrying friction (weight, size, whatever to feel accurate) affects the player speed differently.
	if(pushSpeedMultiMem != pushSpeedMulti){
		pushSpeedMultiMem = pushSpeedMulti;
		if(pushSpeedMultiMem != 0){
			char buffer[13];
			tryFloatToStringBuffer(buffer, pushSpeedMulti);
			g_engfuncs.pfnSetPhysicsKeyValue( edict(), "psm", buffer );
		}else{
			g_engfuncs.pfnSetPhysicsKeyValue( edict(), "psm", "0" );
		}
	}

	
	if(noclipSpeedMultiMem != EASY_CVAR_GET_DEBUGONLY(noclipSpeedMulti)){
		noclipSpeedMultiMem = EASY_CVAR_GET_DEBUGONLY(noclipSpeedMulti);
		
		if(noclipSpeedMultiMem != 0){
			char buffer[13];
			tryFloatToStringBuffer(buffer, EASY_CVAR_GET_DEBUGONLY(noclipSpeedMulti) );
			g_engfuncs.pfnSetPhysicsKeyValue( edict(), "ncm", buffer );
		}else{
			g_engfuncs.pfnSetPhysicsKeyValue( edict(), "ncm", "0" );
		}
	}
	
	if(normalSpeedMultiMem != EASY_CVAR_GET_DEBUGONLY(normalSpeedMulti) ){
		normalSpeedMultiMem = EASY_CVAR_GET_DEBUGONLY(normalSpeedMulti);
		//keep this CVar in sync with pm_shared...
		if(normalSpeedMultiMem != 0){
			char buffer[13];
			tryFloatToStringBuffer(buffer, EASY_CVAR_GET_DEBUGONLY(normalSpeedMulti) );
			g_engfuncs.pfnSetPhysicsKeyValue( edict(), "nsm", buffer );
		}else{
			g_engfuncs.pfnSetPhysicsKeyValue( edict(), "nsm", "0" );
		}
	}
	if(jumpForceMultiMem != EASY_CVAR_GET_DEBUGONLY(jumpForceMulti) ){
		jumpForceMultiMem = EASY_CVAR_GET_DEBUGONLY(jumpForceMulti);
		//keep this CVar in sync with pm_shared...
		if(jumpForceMultiMem != 0){
			char buffer[13];
			tryFloatToStringBuffer(buffer, EASY_CVAR_GET_DEBUGONLY(jumpForceMulti) );
			g_engfuncs.pfnSetPhysicsKeyValue( edict(), "jfm", buffer );
		}else{
			g_engfuncs.pfnSetPhysicsKeyValue( edict(), "jfm", "0" );
		}
	}

	if(ladderCycleMultiMem != EASY_CVAR_GET_DEBUGONLY(ladderCycleMulti) ){
		ladderCycleMultiMem = EASY_CVAR_GET_DEBUGONLY(ladderCycleMulti);
		//keep this CVar in sync with pm_shared...
		if(ladderCycleMultiMem != 0){
			char buffer[13];
			tryFloatToStringBuffer(buffer, EASY_CVAR_GET_DEBUGONLY(ladderCycleMulti));
			g_engfuncs.pfnSetPhysicsKeyValue( edict(), "lcm", buffer );
		}else{
			g_engfuncs.pfnSetPhysicsKeyValue( edict(), "lcm", "0" );
		}
	}
	if(ladderSpeedMultiMem != EASY_CVAR_GET_DEBUGONLY(ladderSpeedMulti) ){
		ladderSpeedMultiMem = EASY_CVAR_GET_DEBUGONLY(ladderSpeedMulti);
		//keep this CVar in sync with pm_shared...
		if(ladderCycleMultiMem != 0){
			char buffer[13];
			tryFloatToStringBuffer(buffer, EASY_CVAR_GET_DEBUGONLY(ladderSpeedMulti));
			g_engfuncs.pfnSetPhysicsKeyValue( edict(), "lsm", buffer );
		}else{
			g_engfuncs.pfnSetPhysicsKeyValue( edict(), "lsm", "0" );
		}
	}
	if (sv_player_midair_fixMem != EASY_CVAR_GET(sv_player_midair_fix)) {
		sv_player_midair_fixMem = EASY_CVAR_GET(sv_player_midair_fix);
		// keep this CVar in sync with pm_shared
		if (sv_player_midair_fixMem == 2) {
			//char buffer[13];
			//tryFloatToStringBuffer(buffer, EASY_CVAR_GET(sv_player_midair_fix));
			g_engfuncs.pfnSetPhysicsKeyValue(edict(), "maf", "2");
		}else if(sv_player_midair_fixMem == 1){
			g_engfuncs.pfnSetPhysicsKeyValue(edict(), "maf", "1");
		}else {
			g_engfuncs.pfnSetPhysicsKeyValue(edict(), "maf", "0");
		}
	}
	if (sv_player_midair_accelMem != EASY_CVAR_GET(sv_player_midair_accel)) {
		sv_player_midair_accelMem = EASY_CVAR_GET(sv_player_midair_accel);
		// keep this CVar in sync with pm_shared
		if(sv_player_midair_accelMem == 1){
			g_engfuncs.pfnSetPhysicsKeyValue(edict(), "maa", "1");
		}else {
			g_engfuncs.pfnSetPhysicsKeyValue(edict(), "maa", "0");
		}
	}
	

	if(clearWeaponFlag == -1){
		MESSAGE_BEGIN( MSG_ONE, gmsgClearWeapon, NULL, pev );
			//WRITE_SHORT( (int)useAlphaCrosshair->value);
		MESSAGE_END();
		clearWeaponFlag = 1;
	}

	if(autoSneakyMem != EASY_CVAR_GET_DEBUGONLY(autoSneaky) ){
		autoSneakyMem = EASY_CVAR_GET_DEBUGONLY(autoSneaky) ;
		autoSneakyCheck();
	}


	//MODDDMIRROR.  Mostly disabled.  I think?
	if(cameraModeMem != globalPSEUDO_cameraMode || mirrorsDoNotReflectPlayerMem != EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(mirrorsDoNotReflectPlayer) ){
		cameraModeMem = globalPSEUDO_cameraMode;
		mirrorsDoNotReflectPlayerMem = EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(mirrorsDoNotReflectPlayer);

		BOOL allowPlayerMarker = FALSE;
		CBaseEntity *pEntityTemp = NULL;


		/*
		MESSAGE_BEGIN( MSG_ONE, gmsgUpdateCam, NULL, pev );
			//WRITE_SHORT( (int)pev->armorvalue);
		MESSAGE_END();
		*/

		
		/*
		if(cameraModeMem == 0 && mirrorsDoNotReflectPlayerMem != 1){
			allowPlayerMarker = TRUE;
		}else{
			allowPlayerMarker = FALSE;
		}
		//easyPrintLine("CHANGE STATUS: %d   %.2f %.2f ", allowPlayerMarker, cameraModeMem, mirrorsDoNotReflectPlayerMem);
		
		if(allowPlayerMarker){
			while ((pEntityTemp = UTIL_FindEntityByClassname( pEntityTemp, "player_marker" )) != NULL){
					//pEntityTemp->pev->effects &= ~128;  //can't draw in first person!
					//pEntityTemp->pev->renderfx &= ~NOREFLECT;
					//pEntityTemp->pev->renderfx |= ISPLAYER;

				pEntityTemp->pev->effects &= ~128;
			}
		}else{
			while ((pEntityTemp = UTIL_FindEntityByClassname( pEntityTemp, "player_marker" )) != NULL){
					//pEntityTemp->pev->effects &= ~128;  //can't draw in first person!
					//pEntityTemp->pev->renderfx |= NOREFLECT;
				pEntityTemp->pev->effects |= 128;
			}
		}
		*/

	}//END OF cameraModeMem check






	if(grabbedByBarnacleMem != grabbedByBarnacle){
		grabbedByBarnacleMem = grabbedByBarnacle;
		//update!  The client must know this (to apply a camera offset as to not clip the barnacle tentacle when looking up)
		MESSAGE_BEGIN( MSG_ONE, gmsgUpdBnclStat, NULL, pev );
			WRITE_BYTE( grabbedByBarnacle );
		MESSAGE_END();
	}


	if(drowningMem != drowning){
		//easyPrintLine("HELP %d %d" , drowningMem, drowning);
		drowningMem = drowning;

		MESSAGE_BEGIN( MSG_ONE, gmsgDrowning, NULL, pev );
			WRITE_BYTE( drowning );
		MESSAGE_END();
	}

	if(playerBrightLightMem != EASY_CVAR_GET_DEBUGONLY(playerBrightLight) ){
		playerBrightLightMem = EASY_CVAR_GET_DEBUGONLY(playerBrightLight);
		if(playerBrightLightMem != 1){
			pev->effects &= ~EF_BRIGHTLIGHT;
		}else{
			pev->effects |= EF_BRIGHTLIGHT;
		}

	}


	// HACKHACK -- send the message to display the game title
	if (gDisplayTitle)
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgShowGameTitle, NULL, pev );
		WRITE_BYTE( 0 );
		MESSAGE_END();
		gDisplayTitle = 0;
	}


	if (pev->health != m_iClientHealth)
	//if(properlyRoundedHealth != m_iClientHealth)
	{
		/*
		int iHealth = max( pev->health, 0 );  // make sure that no negative health values are sent

		// send "health" update message
		MESSAGE_BEGIN(MSG_ONE, gmsgHealth, NULL, pev);
			WRITE_BYTE(iHealth);
		MESSAGE_END();

		m_iClientHealth = pev->health;
		*/


		
		// NEW FILTER:  If health is between 0 and 1, just force it to 1.
		//if (pev->health > 0 && pev->health < 1) {
		///	pev->health = 1;
		//}

		int iHealth;

		if (pev->health > 0 && pev->health < 1) {
			// force it!
			iHealth = 1;
		}
		else {
			iHealth = max(pev->health, 0);  // make sure that no negative health values are sent
		}

		// send "health" update message
		MESSAGE_BEGIN( MSG_ONE, gmsgHealth, NULL, pev );
			WRITE_BYTE( iHealth );
		MESSAGE_END();

		//m_iClientHealth = pev->health;
		m_iClientHealth = iHealth;
		//healthMem = pev->health;
		
	}



	//MODDD 
	//Whenever the deadFlag is altered, tell the GUI about it.
	if(pev->deadflag != deadflagmem){
		deadflagmem = pev->deadflag;

		MESSAGE_BEGIN( MSG_ONE, gmsgUpdatePlayerAlive, NULL, pev );
			WRITE_SHORT( IsAlive() );
		MESSAGE_END();
	}

	


	//MODDD - the next four if-then's are new.
	//MODDD - TODO.  Why not have a "m_rgClientItems" array for detectnig changes
	// in any m_rgItems member, and then update m_rgClientItems[#] and send off the
	// new value?  Ammo reserve array does it that way I think.
	if (m_rgItems[ITEM_ANTIDOTE] != m_iClientAntidote)
	{
		m_iClientAntidote = m_rgItems[ITEM_ANTIDOTE];

		ASSERT( gmsgAntidoteP > 0 );
		MESSAGE_BEGIN( MSG_ONE, gmsgAntidoteP, NULL, pev );
		WRITE_SHORT( m_rgItems[ITEM_ANTIDOTE]);
		MESSAGE_END();
	}

	if (m_rgItems[ITEM_ADRENALINE] != m_iClientAdrenaline)
	{
		m_iClientAdrenaline = m_rgItems[ITEM_ADRENALINE];

		ASSERT( gmsgAdrenalineP > 0 );
		MESSAGE_BEGIN( MSG_ONE, gmsgAdrenalineP, NULL, pev );
		WRITE_SHORT( m_rgItems[ITEM_ADRENALINE]);
		MESSAGE_END();
	}

	if (m_rgItems[ITEM_RADIATION] != m_iClientRadiation)
	{
		m_iClientRadiation = m_rgItems[ITEM_RADIATION];

		ASSERT( gmsgRadiationP > 0 );
		MESSAGE_BEGIN( MSG_ONE, gmsgRadiationP, NULL, pev );
		WRITE_SHORT( m_rgItems[ITEM_RADIATION]);
		MESSAGE_END();
	}

	if(airTankAirTimeNeedsUpdate || airTankAirTime != airTankAirTimeMem){
		airTankAirTimeNeedsUpdate = FALSE;
		airTankAirTimeMem = airTankAirTime;

		//airTankAirTime = 30;
		//easyPrint("yes here is tank airtime %d\n", airTankAirTime);
		
		MESSAGE_BEGIN( MSG_ONE, gmsgUpdateAirTankAirTime, NULL, pev );
		WRITE_SHORT( (int)(airTankAirTime * 100));
		//WRITE_ANGLE
		MESSAGE_END();
	}

	if(m_fLongJumpMemory != m_fLongJump || (!m_fLongJump && longJumpCharge != -1) ){
		//easyPrint("TEST1 %d\n", 0);
		m_fLongJumpMemory = m_fLongJump;
		// See another note about commenting this out.
		//resetLongJumpCharge();
		longJumpChargeNeedsUpdate = TRUE;
	}

	if(longJumpChargeNeedsUpdate || longJumpChargeMem != longJumpCharge){
		longJumpChargeNeedsUpdate = FALSE;
		longJumpChargeMem = longJumpCharge;

		MESSAGE_BEGIN( MSG_ONE, gmsgUpdateLongJumpCharge, NULL, pev );
		
		WRITE_SHORT( (int)(longJumpCharge*100) );
		//WRITE_SHORT( (int)longJumpCharge);
		//WRITE_ANGLE
		MESSAGE_END();
	}

	if (pev->armorvalue != m_iClientBattery)
	{
		m_iClientBattery = pev->armorvalue;

		ASSERT( gmsgBattery > 0 );
		// send "health" update message
		MESSAGE_BEGIN( MSG_ONE, gmsgBattery, NULL, pev );
			WRITE_SHORT( (int)pev->armorvalue);
		MESSAGE_END();
	}

	int hasGlockSilencerTEST = ( !(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(wpn_glocksilencer)==0) && (hasGlockSilencer || EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(wpn_glocksilencer)==2 ));
	//easyForcePrintLine("disregardIN WHAT %d");
	if ( hasGlockSilencerTEST != hasGlockSilencerMem)
	{
		// let the client know we have the silencer for drawing purposes.
		// And some viewmodel logic.
		hasGlockSilencerMem = hasGlockSilencerTEST;
		MESSAGE_BEGIN( MSG_ONE, gmsgHasGlockSilencer, NULL, pev );
			WRITE_SHORT( hasGlockSilencerTEST);
		MESSAGE_END();
	}


	//MODDD
	//if (pev->dmg_take || pev->dmg_save || m_bitsHUDDamage != m_bitsDamageType)
	if (  pev->dmg_take || rawDamageSustained || pev->dmg_save || m_bitsHUDDamage != m_bitsDamageType || m_bitsModHUDDamage != m_bitsDamageTypeMod)
	{
		// Comes from inside me if not set
		Vector damageOrigin = pev->origin;
		// send "damage" message
		// causes screen to flash, and pain compass to show direction of damage
		edict_t *other = pev->dmg_inflictor;
		if ( other )
		{
			CBaseEntity *pEntity = CBaseEntity::Instance(other);
			if ( pEntity )
				damageOrigin = pEntity->Center();
		}

		int forbiddenBits = 0;
		int forbiddenBitsMod = 0;

		if (gSkillData.tdmg_bleeding_duration < 0) {
			forbiddenBitsMod |= DMG_BLEEDING;
		}


		// only send down damage type that have hud art
		//MODDD - the "ForceShow" bitmasks can force an icon to be shown this call, even if it does not do damage
		int visibleDamageBits = ((m_bitsDamageType | m_bitsDamageTypeForceShow) & ~forbiddenBits) & DMG_SHOWNHUD;
		//MODDD
		int visibleDamageBitsMod = ((m_bitsDamageTypeMod | m_bitsDamageTypeModForceShow) & ~forbiddenBitsMod) & DMG_SHOWNHUDMOD;

		MESSAGE_BEGIN( MSG_ONE, gmsgDamage, NULL, pev );
			WRITE_BYTE( pev->dmg_save );

			WRITE_BYTE( pev->dmg_take );
			//MODDD - also send the amount of damage taken before the reduction from armor battery.
			WRITE_BYTE( rawDamageSustained );
			
			WRITE_LONG( visibleDamageBits );
			//MODDD
			WRITE_LONG( visibleDamageBitsMod );
			WRITE_COORD( damageOrigin.x );
			WRITE_COORD( damageOrigin.y );
			WRITE_COORD( damageOrigin.z );
		MESSAGE_END();

		//MODDD reset these, only good one frame
		m_bitsDamageTypeForceShow = 0;
		m_bitsDamageTypeModForceShow = 0;

		pev->dmg_take = 0;
		pev->dmg_save = 0;
		rawDamageSustained = 0;
		m_bitsHUDDamage = m_bitsDamageType;
		//MODDD
		m_bitsModHUDDamage = m_bitsDamageTypeMod;
		
		// Clear off non-time-based damage indicators
		m_bitsDamageType &= DMG_TIMEBASED;
		//MODDD
		m_bitsModHUDDamage &= (DMG_TIMEBASEDMOD);
	}//END OF Crazy damage check


	// Update Flashlight
	if ((m_flFlashLightTime) && (m_flFlashLightTime <= gpGlobals->time))
	{

		//This only appleis if the endlessFlashlightBattery CVar is 0 (off).
		if(EASY_CVAR_GET_DEBUGONLY(endlessFlashlightBattery) == 0){
			if (FlashlightIsOn())
			{
				if (m_iFlashBattery)
				{
					m_flFlashLightTime = FLASH_DRAIN_TIME + gpGlobals->time;
					m_iFlashBattery--;
				
					if (!m_iFlashBattery) {
						FlashlightTurnOff();
					}
				}
			}
			else
			{
				if (m_iFlashBattery < 100)
				{
					m_flFlashLightTime = FLASH_CHARGE_TIME + gpGlobals->time;
					m_iFlashBattery++;
				}
				else {
					m_flFlashLightTime = 0;
				}
			}
		

			MESSAGE_BEGIN( MSG_ONE, gmsgFlashBattery, NULL, pev );
			WRITE_BYTE(m_iFlashBattery);
			MESSAGE_END();

		}//END OF if(endlessFlashlightBattery->value == 0)
	}


	if (m_iTrain & TRAIN_NEW)
	{
		ASSERT( gmsgTrain > 0 );
		// send "health" update message
		MESSAGE_BEGIN( MSG_ONE, gmsgTrain, NULL, pev );
			WRITE_BYTE(m_iTrain & 0xF);
		MESSAGE_END();

		m_iTrain &= ~TRAIN_NEW;
	}

	//
	// New Weapon?
	//
	if (!m_fKnownItem)
	{
		m_fKnownItem = TRUE;

	// WeaponInit Message
	// byte  = # of weapons
	//
	// for each weapon:
	// byte	name str length (not including null)
	// bytes... name
	// byte	Ammo Type
	// byte	Ammo2 Type
	// byte	bucket
	// byte	bucket pos
	// byte	flags	
	// ????		Icons
		
		// Send ALL the weapon info now
		int i;

		for (i = 0; i < MAX_WEAPONS; i++)
		{
			ItemInfo& II = CBasePlayerItem::ItemInfoArray[i];

			if ( !II.iId )
				continue;

			const char *pszName;
			if (!II.pszName) {
				pszName = "Empty";
			}
			else {
				pszName = II.pszName;
			}

			MESSAGE_BEGIN( MSG_ONE, gmsgWeaponList, NULL, pev );  
				WRITE_STRING(pszName);			// string	weapon name

				//MODDD - cached now
				//WRITE_BYTE(GetAmmoIndex(II.pszAmmo1));	// byte	Ammo Type
				WRITE_BYTE(CBasePlayerItem::getPrimaryAmmoType(II.iId));

				WRITE_BYTE(II.iMaxAmmo1);				// byte     Max Ammo 1

				//MODDD - cached now
				//WRITE_BYTE(GetAmmoIndex(II.pszAmmo2));	// byte	Ammo2 Type
				WRITE_BYTE(CBasePlayerItem::getSecondaryAmmoType(II.iId));

				WRITE_BYTE(II.iMaxAmmo2);				// byte     Max Ammo 2
				WRITE_BYTE(II.iSlot);					// byte	bucket
				WRITE_BYTE(II.iPosition);				// byte	bucket pos
				WRITE_BYTE(II.iId);						// byte	id (bit index into pev->weapons)
				WRITE_BYTE(II.iFlags);					// byte	Flags
			MESSAGE_END();
		}
	}

	//MODDD NOTE -   It may seem like ammo updates happen every single frame,
	// but only ammo pools changed since last send-off are updated.
	SendAmmoUpdate();

	// Update all the items
	for ( int i = 0; i < MAX_ITEM_TYPES; i++ )
	{
		if (m_rgpPlayerItems[i]) {  // each item updates it's successors
			m_rgpPlayerItems[i]->UpdateClientData(this);
		}
	}

	// Cache and client weapon change
	m_pClientActiveItem = m_pActiveItem;
	m_iClientFOV = m_iFOV;

	// Update Status Bar
	if ( m_flNextSBarUpdateTime < gpGlobals->time )
	{
		UpdateStatusBar();
		m_flNextSBarUpdateTime = gpGlobals->time + 0.2;
	}
}//UpdateClientData


//=========================================================
// FBecomeProne - Overridden for the player to set the proper
// physics flags when a barnacle grabs player.
//=========================================================
BOOL CBasePlayer::FBecomeProne ( void )
{
	//MODDD - how convenient!  The red carpet's rolled out all for me.
	this->grabbedByBarnacle = TRUE;

	m_afPhysicsFlags |= PFLAG_ONBARNACLE;


	//MODDD TODO - is this a good idea?
	//pev->effects |= EF_NOINTERP;

	return TRUE;
}

//=========================================================
// BarnacleVictimBitten - bad name for a function that is called
// by Barnacle victims when the barnacle pulls their head
// into its mouth. For the player, just die.
//=========================================================
void CBasePlayer::BarnacleVictimBitten ( entvars_t *pevBarnacle )
{
	TakeDamage ( pevBarnacle, pevBarnacle, pev->health + pev->armorvalue, DMG_SLASH | DMG_ALWAYSGIB );
}

//=========================================================
// BarnacleVictimReleased - overridden for player who has
// physics flags concerns. 
//=========================================================
void CBasePlayer::BarnacleVictimReleased ( void )
{
	//MODDD - same!
	this->grabbedByBarnacle = FALSE;

	m_afPhysicsFlags &= ~PFLAG_ONBARNACLE;

	
	//MODDD TODO - is this a good idea?
	//pev->effects &= ~EF_NOINTERP;

}


//=========================================================
// Illumination 
// return player light level plus virtual muzzle flash
//=========================================================
int CBasePlayer::Illumination( void )
{
	int iIllum = CBaseEntity::Illumination( );

	iIllum += m_iWeaponFlash;
	if (iIllum > 255)
		return 255;
	return iIllum;
}


void CBasePlayer::EnableControl(BOOL fControl)
{
	if (!fControl)
		pev->flags |= FL_FROZEN;
	else
		pev->flags &= ~FL_FROZEN;

}


//=========================================================
// Autoaim
// set crosshair position to point to enemey
//=========================================================
Vector CBasePlayer::GetAutoaimVector( float flDelta )
{
	//MODDD - NEW!  Also allow 'sv_aim' of 0 to stop this
	if (g_psv_aim->value == 0 || g_iSkillLevel == SKILL_HARD)
	{
		UTIL_MakeVectors( pev->v_angle + pev->punchangle );
		return gpGlobals->v_forward;
	}

	Vector vecSrc = GetGunPosition( );
	float flDist = 8192;

	// always use non-sticky autoaim
	// UNDONE: use sever variable to chose!
	if (1 || g_iSkillLevel == SKILL_MEDIUM)
	{
		m_vecAutoAim = Vector( 0, 0, 0 );
		// flDelta *= 0.5;
	}

	BOOL m_fOldTargeting = m_fOnTarget;
	Vector angles = AutoaimDeflection(vecSrc, flDist, flDelta );

	// update ontarget if changed
	if ( !g_pGameRules->AllowAutoTargetCrosshair() )
		m_fOnTarget = 0;
	else if (m_fOldTargeting != m_fOnTarget)
	{
		m_pActiveItem->UpdateItemInfo( );
	}

	if (angles.x > 180)
		angles.x -= 360;
	if (angles.x < -180)
		angles.x += 360;
	if (angles.y > 180)
		angles.y -= 360;
	if (angles.y < -180)
		angles.y += 360;

	if (angles.x > 25)
		angles.x = 25;
	if (angles.x < -25)
		angles.x = -25;
	if (angles.y > 12)
		angles.y = 12;
	if (angles.y < -12)
		angles.y = -12;


	// always use non-sticky autoaim
	// UNDONE: use sever variable to chose!
	if (0 || g_iSkillLevel == SKILL_EASY)
	{
		m_vecAutoAim = m_vecAutoAim * 0.67 + angles * 0.33;
	}
	else
	{
		m_vecAutoAim = angles * 0.9;
	}

	// m_vecAutoAim = m_vecAutoAim * 0.99;

	// Don't send across network if sv_aim is 0
	if ( g_psv_aim->value != 0 )
	{
		if ( m_vecAutoAim.x != m_lastx ||
			 m_vecAutoAim.y != m_lasty )
		{
			SET_CROSSHAIRANGLE( edict(), -m_vecAutoAim.x, m_vecAutoAim.y );
			
			m_lastx = m_vecAutoAim.x;
			m_lasty = m_vecAutoAim.y;
		}
	}

	// ALERT( at_console, "%f %f\n", angles.x, angles.y );

	UTIL_MakeVectors( pev->v_angle + pev->punchangle + m_vecAutoAim );
	return gpGlobals->v_forward;
}


Vector CBasePlayer::AutoaimDeflection( Vector &vecSrc, float flDist, float flDelta  )
{
	edict_t		*pEdict = g_engfuncs.pfnPEntityOfEntIndex( 1 );
	CBaseEntity	*pEntity;
	float	bestdot;
	Vector		bestdir;
	edict_t		*bestent;
	TraceResult tr;

	if ( g_psv_aim->value == 0 )
	{
		m_fOnTarget = FALSE;
		return g_vecZero;
	}

	UTIL_MakeVectors( pev->v_angle + pev->punchangle + m_vecAutoAim );

	// try all possible entities
	bestdir = gpGlobals->v_forward;
	bestdot = flDelta; // +- 10 degrees
	bestent = NULL;

	m_fOnTarget = FALSE;

	UTIL_TraceLine( vecSrc, vecSrc + bestdir * flDist, dont_ignore_monsters, edict(), &tr );


	if ( tr.pHit && tr.pHit->v.takedamage != DAMAGE_NO)
	{
		// don't look through water
		if (!((pev->waterlevel != 3 && tr.pHit->v.waterlevel == 3) 
			|| (pev->waterlevel == 3 && tr.pHit->v.waterlevel == 0)))
		{
			if (tr.pHit->v.takedamage == DAMAGE_AIM)
				m_fOnTarget = TRUE;

			return m_vecAutoAim;
		}
	}

	for ( int i = 1; i < gpGlobals->maxEntities; i++, pEdict++ )
	{
		Vector center;
		Vector dir;
		float dot;

		if ( pEdict->free )	// Not in use
			continue;
		
		if (pEdict->v.takedamage != DAMAGE_AIM)
			continue;
		if (pEdict == edict())
			continue;
//		if (pev->team > 0 && pEdict->v.team == pev->team)
//			continue;	// don't aim at teammate
		if ( !g_pGameRules->ShouldAutoAim( this, pEdict ) )
			continue;

		pEntity = Instance( pEdict );
		if (pEntity == NULL)
			continue;

		if (!pEntity->IsAlive())
			continue;

		// don't look through water
		if ((pev->waterlevel != 3 && pEntity->pev->waterlevel == 3) 
			|| (pev->waterlevel == 3 && pEntity->pev->waterlevel == 0))
			continue;

		center = pEntity->BodyTarget( vecSrc );

		dir = (center - vecSrc).Normalize( );

		// make sure it's in front of the player
		if (DotProduct (dir, gpGlobals->v_forward ) < 0)
			continue;

		dot = fabs( DotProduct (dir, gpGlobals->v_right ) ) 
			+ fabs( DotProduct (dir, gpGlobals->v_up ) ) * 0.5;

		// tweek for distance
		dot *= 1.0 + 0.2 * ((center - vecSrc).Length() / flDist);

		if (dot > bestdot)
			continue;	// to far to turn

		UTIL_TraceLine( vecSrc, center, dont_ignore_monsters, edict(), &tr );
		if (tr.flFraction != 1.0 && tr.pHit != pEdict)
		{
			// ALERT( at_console, "hit %s, can't see %s\n", STRING( tr.pHit->v.classname ), STRING( pEdict->v.classname ) );
			continue;
		}

		// don't shoot at friends
		if (IRelationship( pEntity ) < 0)
		{
			if ( !pEntity->IsPlayer() && !g_pGameRules->IsDeathmatch())
				// ALERT( at_console, "friend\n");
				continue;
		}

		// can shoot at this one
		bestdot = dot;
		bestent = pEdict;
		bestdir = dir;
	}

	if (bestent)
	{
		bestdir = UTIL_VecToAngles (bestdir);
		bestdir.x = -bestdir.x;
		bestdir = bestdir - pev->v_angle - pev->punchangle;

		if (bestent->v.takedamage == DAMAGE_AIM)
			m_fOnTarget = TRUE;

		return bestdir;
	}

	return Vector( 0, 0, 0 );
}


void CBasePlayer::ResetAutoaim( )
{
	if (m_vecAutoAim.x != 0 || m_vecAutoAim.y != 0)
	{
		m_vecAutoAim = Vector( 0, 0, 0 );
		SET_CROSSHAIRANGLE( edict(), 0, 0 );
	}
	m_fOnTarget = FALSE;
}

/*
=============
SetCustomDecalFrames

  UNDONE:  Determine real frame limit, 8 is a placeholder.
  Note:  -1 means no custom frames present.
=============
*/
void CBasePlayer::SetCustomDecalFrames( int nFrames )
{
	if (nFrames > 0 &&
		nFrames < 8)
		m_nCustomSprayFrames = nFrames;
	else
		m_nCustomSprayFrames = -1;
}

/*
=============
GetCustomDecalFrames

  Returns the # of custom frames this player's custom clan logo contains.
=============
*/
int CBasePlayer::GetCustomDecalFrames( void )
{
	return m_nCustomSprayFrames;
}



/*
//MODDD - clone of DropPlayerItem, only no resulting drop.
// Wait.  There's already a "RemovePlayerItem" that takes a reference to the object instead.
// And that's already availble in the scenario I have.   ...     oops.
// Just don't repeat that mistake.
void CBasePlayer::RemovePlayerItemClassname(char* pszItemName)
{
	<snip>
}
*/


//=========================================================
// DropPlayerItem - drop the named item, or if no name,
// the active item. 
//=========================================================
void CBasePlayer::DropPlayerItem ( char *pszItemName )
{
	//MODDD - new cvar.
	//if ( !IsMultiplayer() || (weaponstay.value > 0) )
	if ( EASY_CVAR_GET_DEBUGONLY(canDropInSinglePlayer) == 0 && (!IsMultiplayer() || (weaponstay.value > 0)) )
	{
		// no dropping in single player.
		return;
	}

	if ( !strlen( pszItemName ) )
	{
		// if this string has no length, the client didn't type a name!
		// assume player wants to drop the active item.
		// make the string null to make future operations in this function easier
		pszItemName = NULL;
	} 

	CBasePlayerItem *pWeapon;
	int i;

	for ( i = 0 ; i < MAX_ITEM_TYPES ; i++ )
	{
		pWeapon = m_rgpPlayerItems[ i ];

		while ( pWeapon )
		{
			if ( pszItemName )
			{
				// try to match by name. 
				if ( !strcmp( pszItemName, STRING( pWeapon->pev->classname ) ) )
				{
					// match! 
					break;
				}
			}
			else
			{
				// trying to drop active item
				if ( pWeapon == m_pActiveItem )
				{
					// active item!
					break;
				}
			}

			pWeapon = pWeapon->m_pNext; 
		}

		
		// if we land here with a valid pWeapon pointer, that's because we found the 
		// item we want to drop and hit a BREAK;  pWeapon is the item.
		if ( pWeapon )
		{
			BOOL getNextSuccess = g_pGameRules->GetNextBestWeapon( this, pWeapon );

			
			// m_pActiveItem == NULL
			if(!getNextSuccess || pWeapon==m_pActiveItem){
				//send a signal to clear the currently equipped weapon.
				MESSAGE_BEGIN( MSG_ONE, gmsgClearWeapon, NULL, pev );
				MESSAGE_END();
			}

			//MODDD - whoops!  Don't use MakeVectors on pev->angles.  That's only for player pev->v_angle!
			// use UTIL_MakeAimVectors here
			UTIL_MakeAimVectors ( pev->angles ); 

			pev->weapons &= ~(1<<pWeapon->m_iId);// take item off hud

			CWeaponBox *pWeaponBox = (CWeaponBox *)CBaseEntity::Create( "weaponbox", pev->origin + gpGlobals->v_forward * 10, pev->angles, edict() );
			pWeaponBox->pev->angles.x = 0;
			pWeaponBox->pev->angles.z = 0;
			pWeaponBox->PackWeapon( pWeapon );
			pWeaponBox->pev->velocity = gpGlobals->v_forward * 300 + gpGlobals->v_forward * 100;
			
			// drop half of the ammo for this weapon.
			int iAmmoIndex;

			//iAmmoIndex = GetAmmoIndex ( pWeapon->pszAmmo1() ); // ???
			iAmmoIndex = pWeapon->getPrimaryAmmoType();
			
			if ( iAmmoIndex != -1 )
			{
				// this weapon weapon uses ammo, so pack an appropriate amount.
				if ( pWeapon->iFlags() & ITEM_FLAG_EXHAUSTIBLE )
				{
					// pack up all the ammo, this weapon is its own ammo type
					pWeaponBox->PackAmmo( MAKE_STRING(pWeapon->pszAmmo1()), m_rgAmmo[ iAmmoIndex ] );
					m_rgAmmo[ iAmmoIndex ] = 0; 

				}
				else
				{
					// pack half of the ammo
					//MODDD - but don't loose one if there's an odd number.  or... even, however that works.
					pWeaponBox->PackAmmo( MAKE_STRING(pWeapon->pszAmmo1()), (int) ceil( m_rgAmmo[ iAmmoIndex ] / 2.0f ) );
					m_rgAmmo[ iAmmoIndex ] = (int)floor(m_rgAmmo[ iAmmoIndex ] / 2.0f);
				}
			}

			/*
			// nope, not a good idea.  Apparently.
			SendAmmoUpdate();

			//MODDD - update the GUI.  Why was this not here?
			MESSAGE_BEGIN( MSG_ONE, gmsgCurWeapon, NULL, pev );
				WRITE_BYTE(0);
				WRITE_BYTE(0);
				WRITE_BYTE(0);
			MESSAGE_END();
			*/

			return;// we're done, so stop searching with the FOR loop.
		}
	}
}


BOOL CBasePlayer::HasPlayerItem( CBasePlayerItem *pCheckItem )
{
	CBasePlayerItem *pItem = m_rgpPlayerItems[pCheckItem->iItemSlot()];

	while (pItem)
	{
		if (FClassnameIs( pItem->pev, STRING( pCheckItem->pev->classname) ))
		{
			return TRUE;
		}
		pItem = pItem->m_pNext;
	}

	return FALSE;
}

//MODDD - new version. given an item slot and classname instead of an item object.
BOOL CBasePlayer::HasPlayerItem( int arg_iItemSlot, const char* arg_className )
{
	CBasePlayerItem *pItem = m_rgpPlayerItems[arg_iItemSlot];

	while (pItem)
	{
		if (FClassnameIs( pItem->pev, arg_className ))
		{
			return TRUE;
		}
		pItem = pItem->m_pNext;
	}

	return FALSE;
}



//=========================================================
// HasNamedPlayerItem Does the player already have this item?
//=========================================================
BOOL CBasePlayer::HasNamedPlayerItem( const char *pszItemName )
{
	CBasePlayerItem *pItem;
	int i;
 
	for ( i = 0 ; i < MAX_ITEM_TYPES ; i++ )
	{
		pItem = m_rgpPlayerItems[ i ];
		
		while (pItem)
		{
			if ( !strcmp( pszItemName, STRING( pItem->pev->classname ) ) )
			{
				return TRUE;
			}
			pItem = pItem->m_pNext;
		}
	}

	return FALSE;
}


//MODDD - new.
CBasePlayerItem* CBasePlayer::FindNamedPlayerItem(const char *pszItemName){

	CBasePlayerItem *pItem;
	int i;
 
	for ( i = 0 ; i < MAX_ITEM_TYPES ; i++ )
	{
		pItem = m_rgpPlayerItems[ i ];
		
		while (pItem)
		{
			if ( !strcmp( pszItemName, STRING( pItem->pev->classname ) ) )
			{
				return pItem;
			}
			pItem = pItem->m_pNext;
		}
	}

	return FALSE;

	//gun = (CBasePlayerWeapon *)pPlayerItem->GetWeaponPtr();

}


//MODDD - as-is method, modified to work with holstering.
// This is called on the player weapon being forced to something else by event, most likely (if not only)
// picking up a new weapon that wants to deploy.
BOOL CBasePlayer::SwitchWeapon( CBasePlayerItem *pWeapon ) 
{
	// old contents of method, now supports holstering
	/*
	if ( !pWeapon->CanDeploy() )
	{
		return FALSE;
	}
	
	ResetAutoaim( );
	
	if (m_pActiveItem)
	{
		m_pActiveItem->Holster( );
	}

	m_pActiveItem = pWeapon;
	pWeapon->Deploy( );

	return TRUE;
	*/

	// Check: is the current weapon in the middle of a reload?
	if(m_pActiveItem != NULL){
		CBasePlayerWeapon* tempWeap =  (CBasePlayerWeapon*)m_pActiveItem->GetWeaponPtr();
		if(tempWeap->m_fInReload || tempWeap->m_fInSpecialReload){
			// deny changing weapons then!
			return FALSE;
		}
	}


	ResetAutoaim();
	
	// NOTICE - on being told to switch weapons from receiving one in the middle of a stream of
	// received weapons (multiplayer spawn, or cheat grants), disable holster anims.  Just akward
	// to do that for getting subsequent weapons after getting a new one already.
	setActiveItem_HolsterCheck(pWeapon, !globalflag_muteDeploySound && (fBreakHolster != TRUE) );
	return TRUE;
	
}



//MODDD - SOME NEW PLAYER METHODS
void CBasePlayer::consumeAntidote(){
	antidoteQueued = FALSE;

	if (m_rgItems[ITEM_ANTIDOTE] <= 0) {
		return;  //what
	}

	//m_rgbTimeBasedDamage[itbd_NerveGas] = 0;
	//m_rgbTimeBasedDamage[itbd_Poison] = 0;
	removeTimedDamage(itbd_NerveGas, &m_bitsDamageType);
	removeTimedDamage(itbd_Poison, &m_bitsDamageType);
	m_rgItems[ITEM_ANTIDOTE]--;

	MESSAGE_BEGIN( MSG_ONE, gmsgHUDItemFlash, NULL, pev );
		WRITE_BYTE( 0 );
	MESSAGE_END();


}//END OF consumeAntidote

void CBasePlayer::consumeRadiation(){
	radiationQueued = FALSE;

	if (m_rgItems[ITEM_RADIATION] <= 0) {
		return;  //what
	}

	removeTimedDamage(itbd_Radiation, &m_bitsDamageType);
	m_rgItems[ITEM_RADIATION]--;

	MESSAGE_BEGIN( MSG_ONE, gmsgHUDItemFlash, NULL, pev );
		WRITE_BYTE( 1 );
	MESSAGE_END();

}//END OF consumeRadiation

void CBasePlayer::consumeAdrenaline(){
	if (m_rgItems[ITEM_ADRENALINE] <= 0) {
		if (recoveryIndex == 1) {
			declareRevivelessDead();  // assumption?
			m_flSuitUpdate = gpGlobals->time;  //say the next line now!
			SetSuitUpdate("!HEV_E3", FALSE, SUIT_REPEAT_OK, 4.2f);
		}
		return;  //what
	}

	recentRevivedTime = gpGlobals->time;

	m_rgItems[ITEM_ADRENALINE]--;

	recoveryIndex = 2;
	recoveryDelay = gpGlobals->time + RANDOM_FLOAT(2.3f, 3.6f);

	//m_rgItems[ITEM_ADRENALINE] --;
	//SetSuitUpdate("!HEV_ADR_USE", FALSE, SUIT_REPEAT_OK);

	if(EASY_CVAR_GET_DEBUGONLY(batteryDrainsAtAdrenalineMode) == 2){
		SetAndUpdateBattery(0);
	}
	if (EASY_CVAR_GET_DEBUGONLY(timedDamageReviveRemoveMode) == 2) {
		attemptResetTimedDamage(TRUE);
	}
	
	MESSAGE_BEGIN( MSG_ONE, gmsgHUDItemFlash, NULL, pev );
		WRITE_BYTE( 2 );
	MESSAGE_END();

}//END OF consumeAdrenaline


// Sounds from the player do NOT attempt to use the soundsentencesave system.  They are
// precached always and don't expect equivalent sentences to exist (they shouldn't, it is
// wasteful to have a sentence entry and use a guaranteed precache when only either is needed)
BOOL CBasePlayer::usesSoundSentenceSave(void) {
	return FALSE;
}

void CBasePlayer::SetGravity(float newGravityVal){
	// do this anyway I guess?
	pev->gravity = newGravityVal;

	// don't allow things too extreme?
	float filter = UTIL_clamp(newGravityVal, -4, 4);

	// canned values: set em' fast
	if(filter == 0){
		g_engfuncs.pfnSetPhysicsKeyValue( edict(), "gmm", "0" );
	}else if(filter == 1.0f){
		g_engfuncs.pfnSetPhysicsKeyValue( edict(), "gmm", "1" );
	}else{
		// parse away
		char buffer[13];
		tryFloatToStringBuffer(buffer, filter );
		g_engfuncs.pfnSetPhysicsKeyValue( edict(), "gmm", buffer );
	}
	
}


// At the time of death, record any entities following the player
// (up to 5, don't really see any higher of a count being useful)
void CBasePlayer::RecordFollowers(void){
	edict_t		*pEdict = g_engfuncs.pfnPEntityOfEntIndex( 1 );
	CBaseEntity *pEntity;
	float closestDistanceYet = 2000;
	int dickIndex = 0;
	if ( !pEdict ){
		return;
	}

	// reset for safety
	recentDeadPlayerFollowersCount = 0;


	for ( int i = 1; i < gpGlobals->maxEntities; i++, pEdict++ )
	{
		if ( pEdict->free )	// Not in use
			continue;
		if ( !(pEdict->v.flags & (FL_CLIENT|FL_MONSTER)) )	// Not a client/monster ?
			continue;

		pEntity = CBaseEntity::Instance(pEdict);
		if ( !pEntity ){
			continue;
		}

		const char* theClassname = pEntity->getClassname();

		CBaseMonster* tempMonster = pEntity->MyMonsterPointer();
		if(tempMonster == NULL || FClassnameIs(tempMonster->pev, "player")){
			continue;  //not players or non-monsters.
		}

		if(!tempMonster->isTalkMonster()){
			// only TalkMonsters can be followers
			continue;
		}
		if(!tempMonster->IsAlive()){
			continue;
		}

		CTalkMonster* daTalkah = static_cast<CTalkMonster*>(tempMonster);
		// Is this talker following anything, and is it following me?
		if(daTalkah->m_MonsterState == MONSTERSTATE_PRONE){
			// nope
			continue;
		}

		if(daTalkah->IsFollowing() && daTalkah->m_hTargetEnt->edict() == this->edict()){
			// ok, record it
			
			
			if(recentDeadPlayerFollowersCount < 5){
				// record anything, pick the closest of these another time.
				recentDeadPlayerFollowers[recentDeadPlayerFollowersCount] = daTalkah;
				recentDeadPlayerFollowersCount++;
				// (but still record the current closest just in case)
				float theDista = Distance(daTalkah->pev->origin, this->pev->origin);
				if(theDista < closestDistanceYet){
					closestDistanceYet = theDista;
				}

			}else{
				float theDista = Distance(daTalkah->pev->origin, this->pev->origin);
				if(theDista < closestDistanceYet){
					// If this is better than the closest distance yet, go ahead
					// and overwrite some point on the array.
					closestDistanceYet = theDista;
					recentDeadPlayerFollowers[dickIndex] = daTalkah;
					dickIndex++;
					if(dickIndex >= 5){
						// start over at 0 if this gets too high
						dickIndex = 0;
					}
				}
			}
			
		}//daTalkah following check



	}//END OF through all entities.


}//RecordFollowers










/*
void CBasePlayer::Think(void)
{
	//NEVER CALLED
	CBaseMonster::Think();
	

	//easyPrintLine("PLAYER THINK CALLED?");
	//...strangely, this doesn't seem to get called.  Not an issue though, postThink is good enough.

}

void CBasePlayer::MonsterThink(void)
{
	//NEVER CALLED.  Player uses different think methods, which are linked differently.
	//The call "SetThink ( &CBaseMonster::CallMonsterThink );" from "StartMonster" in monsters.cpp, which is called (in a little time) from "MonsterInit" (also in monsters.cpp), is what sets "MonsterThink" to be the think method for here.
	CBaseMonster::MonsterThink();
	

	//easyPrintLine("PLAYER THINK CALLED?");
	//...strangely, this doesn't seem to get called.  Not an issue though, postThink is good enough.

}
*/

