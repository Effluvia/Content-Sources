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
//=========================================================
// GameRules.cpp
//=========================================================

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "gamerules.h"
#include "gamerules_teamplay.h"
#include "skill.h"
#include "game.h"

extern DLL_GLOBAL BOOL g_fGameOver;
extern int gmsgDeathMsg;	// client dll messages
extern int gmsgMOTD;

// util.cpp
extern float cvar_skill_mem;
extern BOOL queueSkillUpdate;

extern edict_t *EntSelectSpawnPoint( CBaseEntity *pPlayer );

DLL_GLOBAL CGameRules* g_pGameRules = NULL;
int g_teamplay = 0;

//=========================================================
//=========================================================
BOOL CGameRules::CanHaveAmmo( CBasePlayer *pPlayer, const char *pszAmmoName, int iMaxCarry )
{
	int iAmmoIndex;

	if ( pszAmmoName )
	{
		//MODDD - GetAmmoIndex is no longer a member of CBasePlayer.
		iAmmoIndex = GetAmmoIndex( pszAmmoName );
		//easyForcePrintLine("WELL??? |%s| : %d", pszAmmoName, iAmmoIndex);

		if ( iAmmoIndex > -1 )
		{
			if ( pPlayer->AmmoInventory( iAmmoIndex ) < iMaxCarry )
			{
				// player has room for more of this type of ammo
				return TRUE;
			}
		}
	}

	return FALSE;
}


//MODDD - new version that supports ammo type Id given directly instead.
BOOL CGameRules::CanHaveAmmo(CBasePlayer* pPlayer, int iAmmoTypeId, int iMaxCarry)
{
	int iAmmoIndex;

	if(IS_AMMOTYPE_VALID(iAmmoTypeId))
	{
		//MODDD - GetAmmoIndex is no longer a member of CBasePlayer.
		iAmmoIndex = iAmmoTypeId;
		//easyForcePrintLine("WELL??? |%s| : %d", pszAmmoName, iAmmoIndex);

		if (iAmmoIndex > -1)
		{
			if (pPlayer->AmmoInventory(iAmmoIndex) < iMaxCarry)
			{
				// player has room for more of this type of ammo
				return TRUE;
			}
		}
	}

	return FALSE;
}




//=========================================================
//=========================================================
edict_t *CGameRules::GetPlayerSpawnSpot( CBasePlayer *pPlayer )
{
	edict_t *pentSpawnSpot = EntSelectSpawnPoint( pPlayer );

	pPlayer->pev->origin = VARS(pentSpawnSpot)->origin + Vector(0,0,1);
	pPlayer->pev->v_angle  = g_vecZero;
	pPlayer->pev->velocity = g_vecZero;
	pPlayer->pev->angles = VARS(pentSpawnSpot)->angles;
	pPlayer->pev->punchangle = g_vecZero;
	pPlayer->pev->fixangle = TRUE;
	
	return pentSpawnSpot;
}

//=========================================================
//=========================================================
BOOL CGameRules::CanHavePlayerItem( CBasePlayer *pPlayer, CBasePlayerItem *pWeapon )
{
	// only living players can have items
	if ( pPlayer->pev->deadflag != DEAD_NO )
		return FALSE;

	if ( pWeapon->pszAmmo1() )
	{
		if ( !CanHaveAmmo( pPlayer, pWeapon->pszAmmo1(), pWeapon->iMaxAmmo1() ) )
		{
			// we can't carry anymore ammo for this gun. We can only 
			// have the gun if we aren't already carrying one of this type
			if ( pPlayer->HasPlayerItem( pWeapon ) )
			{
				return FALSE;
			}
		}
	}
	else
	{
		// weapon doesn't use ammo, don't take another if you already have it.
		if ( pPlayer->HasPlayerItem( pWeapon ) )
		{
			return FALSE;
		}
	}

	// note: will fall through to here if GetItemInfo doesn't fill the struct!
	return TRUE;
}

//MODDD - new. Given an item of these stats, could we take it (version without the item object)?
BOOL CGameRules::CanHavePlayerItem( CBasePlayer *pPlayer, const char* arg_classname, const char* arg_ammoName, int arg_itemSlot, int arg_maxAmmo )
{
	//arg_classname: pWeapon->pev->classname
	//arg_ammoName: pWeapon->pszAmmo1()
	//arg_itemSlot: pWeapon->iItemSlot()
	//arg_maxAmmo: pWeapon->iMaxAmmo1()

	// only living players can have items
	if ( pPlayer->pev->deadflag != DEAD_NO )
		return FALSE;

	if ( arg_ammoName )
	{
		if ( !CanHaveAmmo( pPlayer, arg_ammoName, arg_maxAmmo ) )
		{
			// we can't carry anymore ammo for this gun. We can only 
			// have the gun if we aren't already carrying one of this type
			if ( pPlayer->HasPlayerItem( arg_itemSlot, arg_classname ) )
			{
				return FALSE;
			}
		}
	}
	else
	{
		// weapon doesn't use ammo, don't take another if you already have it.
		if ( pPlayer->HasPlayerItem( arg_itemSlot, arg_classname ) )
		{
			return FALSE;
		}
	}

	// note: will fall through to here if GetItemInfo doesn't fill the struct!
	return TRUE;
}


//=========================================================
// load the SkillData struct with the proper values based on the skill level.
//=========================================================
void CGameRules::RefreshSkillData ( void )
{
	int iSkill;

	// set before the bound check below, don't want to keep re-calling RefreshSkillData
	// just because some bad skill value (9, etc.) was used.
	iSkill = (int)CVAR_GET_FLOAT("skill");

	// don't call this again repeatedly.
	cvar_skill_mem = iSkill;
	queueSkillUpdate = FALSE;

	// Check for a bad skill value, snap it to the range 1 to 3 (EASY, MED, HARD).
	if ( iSkill < SKILL_EASY )
	{
		iSkill = SKILL_EASY;
	}
	else if ( iSkill > SKILL_HARD )
	{
		iSkill = SKILL_HARD; 
	}

	//MODDD - and this wasn't below the iSkill bounds checking because...?
	g_iSkillLevel = iSkill;


	//MODDD - removed.
	//gSkillData.iSkillLevel = iSkill;

	ALERT ( at_console, "\nGAME SKILL LEVEL:%d\n",iSkill );

	//Agrunt		
	gSkillData.agruntHealth = GetSkillCvar( "sk_agrunt_health" );
	gSkillData.agruntDmgPunch = GetSkillCvar( "sk_agrunt_dmg_punch");

	// Apache 
	gSkillData.apacheHealth = GetSkillCvar( "sk_apache_health");

	// Barnacle
	gSkillData.barnacleHealth = GetSkillCvar( "sk_barnacle_health");

	// Barney
	gSkillData.barneyHealth = GetSkillCvar( "sk_barney_health");

	// Big Momma
	gSkillData.bigmommaHealthFactor = GetSkillCvar( "sk_bigmomma_health_factor" );
	gSkillData.bigmommaDmgSlash = GetSkillCvar( "sk_bigmomma_dmg_slash" );
	gSkillData.bigmommaDmgBlast = GetSkillCvar( "sk_bigmomma_dmg_blast" );
	gSkillData.bigmommaRadiusBlast = GetSkillCvar( "sk_bigmomma_radius_blast" );

	// Bullsquid
	gSkillData.bullsquidHealth = GetSkillCvar( "sk_bullsquid_health");
	gSkillData.bullsquidDmgBite = GetSkillCvar( "sk_bullsquid_dmg_bite");
	gSkillData.bullsquidDmgWhip = GetSkillCvar( "sk_bullsquid_dmg_whip");
	gSkillData.bullsquidDmgSpit = GetSkillCvar( "sk_bullsquid_dmg_spit");

	// Gargantua
	gSkillData.gargantuaHealth = GetSkillCvar( "sk_gargantua_health");
	gSkillData.gargantuaDmgSlash = GetSkillCvar( "sk_gargantua_dmg_slash");
	gSkillData.gargantuaDmgFire = GetSkillCvar( "sk_gargantua_dmg_fire");
	gSkillData.gargantuaDmgStomp = GetSkillCvar( "sk_gargantua_dmg_stomp");

	// Hassassin
	gSkillData.hassassinHealth = GetSkillCvar( "sk_hassassin_health");
	//MODDD - NEW.  Given crossbow skill values like the player's.
	gSkillData.hassassinDmgCrossbowClient = GetSkillCvar( "sk_hassassin_xbow_cl");
	gSkillData.hassassinDmgCrossbowMonster = GetSkillCvar( "sk_hassassin_xbow_mo");

	// Headcrab
	gSkillData.headcrabHealth = GetSkillCvar( "sk_headcrab_health");
	gSkillData.headcrabDmgBite = GetSkillCvar( "sk_headcrab_dmg_bite");

	// Hgrunt 
	gSkillData.hgruntHealth = GetSkillCvar( "sk_hgrunt_health");
	gSkillData.hgruntDmgKick = GetSkillCvar( "sk_hgrunt_kick");
	gSkillData.hgruntShotgunPellets = GetSkillCvar( "sk_hgrunt_pellets");
	gSkillData.hgruntGrenadeSpeed = GetSkillCvar( "sk_hgrunt_gspeed");

	//MODDD - NEW - HAssault
	gSkillData.hassaultHealth = GetSkillCvar( "sk_hassault_health");
	gSkillData.hassaultDmgMelee = GetSkillCvar( "sk_hassault_dmg_melee");
	
	// Houndeye
	gSkillData.houndeyeHealth = GetSkillCvar( "sk_houndeye_health");
	gSkillData.houndeyeDmgBlast = GetSkillCvar( "sk_houndeye_dmg_blast");

	// ISlave
	gSkillData.slaveHealth = GetSkillCvar( "sk_islave_health");
	gSkillData.slaveDmgClaw = GetSkillCvar( "sk_islave_dmg_claw");
	gSkillData.slaveDmgClawrake = GetSkillCvar( "sk_islave_dmg_clawrake");
	gSkillData.slaveDmgZap = GetSkillCvar( "sk_islave_dmg_zap");

	// Icthyosaur
	gSkillData.ichthyosaurHealth = GetSkillCvar( "sk_ichthyosaur_health");
	gSkillData.ichthyosaurDmgShake = GetSkillCvar( "sk_ichthyosaur_shake");

	// Leech
	gSkillData.leechHealth = GetSkillCvar( "sk_leech_health");

	gSkillData.leechDmgBite = GetSkillCvar( "sk_leech_dmg_bite");

	// MODDD - new.
	// Stukabat
	gSkillData.stukaBatHealth = GetSkillCvar( "sk_stukabat_health");
	gSkillData.stukaBatDmgClaw = GetSkillCvar( "sk_stukabat_dmgclaw");	
	/*
	//MODDD - NOPE.
	gSkillData.stukaBatDmgExplosion = GetSkillCvar( "sk_stukabat_dmgexplosion");
	*/

	//Panthereye
	gSkillData.panthereyeHealth = GetSkillCvar( "sk_panthereye_health");
	gSkillData.panthereyeDmgClaw = GetSkillCvar( "sk_panthereye_dmgclaw");

	
	// Controller
	gSkillData.controllerHealth = GetSkillCvar( "sk_controller_health");
	gSkillData.controllerDmgZap = GetSkillCvar( "sk_controller_dmgzap");
	gSkillData.controllerSpeedBall = GetSkillCvar( "sk_controller_speedball");
	gSkillData.controllerDmgBall = GetSkillCvar( "sk_controller_dmgball");

	// Nihilanth
	gSkillData.nihilanthHealth = GetSkillCvar( "sk_nihilanth_health");
	gSkillData.nihilanthZap = GetSkillCvar( "sk_nihilanth_zap");

	// Scientist
	gSkillData.scientistHealth = GetSkillCvar( "sk_scientist_health");
	gSkillData.scientistDmgPunch = GetSkillCvar( "sk_scientist_dmg_punch");

	
	//MODDD - new!
	gSkillData.chumtoadHealth = GetSkillCvar("sk_chumtoad_health");

	// Snark
	gSkillData.snarkHealth = GetSkillCvar( "sk_snark_health");
	gSkillData.snarkDmgBite = GetSkillCvar( "sk_snark_dmg_bite");
	gSkillData.snarkDmgPop = GetSkillCvar( "sk_snark_dmg_pop");

	// Zombie
	gSkillData.zombieHealth = GetSkillCvar( "sk_zombie_health");
	gSkillData.zombieDmgOneSlash = GetSkillCvar( "sk_zombie_dmg_one_slash");
	gSkillData.zombieDmgBothSlash = GetSkillCvar( "sk_zombie_dmg_both_slash");
	//MODDD - Zombie damage reistance.
	gSkillData.zombieBulletResistance = GetSkillCvar( "sk_zombie_bulletresistance");
	gSkillData.zombieBulletPushback = GetSkillCvar("sk_zombie_bulletpushback");



	//MODDD - several new NPC's
	//Archer
	gSkillData.archerHealth = GetSkillCvar("sk_archer_health");

	//(here comes dat) Boid (oh shit waddup)
	gSkillData.boidHealth = GetSkillCvar("sk_boid_health");

	//Floater
	gSkillData.floaterHealth = GetSkillCvar("sk_floater_health");

	//Friendly
	gSkillData.friendlyHealth = GetSkillCvar("sk_friendly_health");

	//Kingpin
	gSkillData.kingpinHealth = GetSkillCvar("sk_kingpin_health");







	//Turret
	gSkillData.turretHealth = GetSkillCvar( "sk_turret_health");

	// MiniTurret
	gSkillData.miniturretHealth = GetSkillCvar( "sk_miniturret_health");
	
	// Sentry Turret
	gSkillData.sentryHealth = GetSkillCvar( "sk_sentry_health");

// PLAYER WEAPONS

	// Crowbar whack
	gSkillData.plrDmgCrowbar = GetSkillCvar( "sk_plr_crowbar");

	// Glock Round
	gSkillData.plrDmg9MM = GetSkillCvar( "sk_plr_9mm_bullet");

	// 357 Round
	gSkillData.plrDmg357 = GetSkillCvar( "sk_plr_357_bullet");

	// MP5 Round
	gSkillData.plrDmgMP5 = GetSkillCvar( "sk_plr_9mmAR_bullet");

	// M203 grenade
	gSkillData.plrDmgM203Grenade = GetSkillCvar( "sk_plr_9mmAR_grenade");

	// Shotgun buckshot
	gSkillData.plrDmgBuckshot = GetSkillCvar( "sk_plr_buckshot");

	// Crossbow
	gSkillData.plrDmgCrossbowClient = GetSkillCvar( "sk_plr_xbow_bolt_client");
	gSkillData.plrDmgCrossbowMonster = GetSkillCvar( "sk_plr_xbow_bolt_monster");

	// RPG
	gSkillData.plrDmgRPG = GetSkillCvar( "sk_plr_rpg");

	// Gauss gun
	gSkillData.plrDmgGauss = GetSkillCvar( "sk_plr_gauss");

	// Egon Gun
	gSkillData.plrDmgEgonNarrow = GetSkillCvar( "sk_plr_egon_narrow");
	gSkillData.plrDmgEgonWide = GetSkillCvar( "sk_plr_egon_wide");

	// Hand Grendade
	gSkillData.plrDmgHandGrenade = GetSkillCvar( "sk_plr_hand_grenade");

	// Satchel Charge
	gSkillData.plrDmgSatchel = GetSkillCvar( "sk_plr_satchel");

	// Tripmine
	gSkillData.plrDmgTripmine = GetSkillCvar( "sk_plr_tripmine");

	// MONSTER WEAPONS
	gSkillData.monDmg12MM = GetSkillCvar( "sk_12mm_bullet");
	gSkillData.monDmgMP5 = GetSkillCvar ("sk_9mmAR_bullet" );
	gSkillData.monDmg9MM = GetSkillCvar( "sk_9mm_bullet");

	// MONSTER HORNET
	gSkillData.monDmgHornet = GetSkillCvar( "sk_hornet_dmg");

	//MODDD
	gSkillData.monHealthHornet = GetSkillCvar( "sk_hornet_health");

	// PLAYER HORNET
// Up to this point, player hornet damage and monster hornet damage were both using
// monDmgHornet to determine how much damage to do. In tuning the hivehand, we now need
// to separate player damage and monster hivehand damage. Since it's so late in the project, we've
// added plrDmgHornet to the SKILLDATA struct, but not to the engine CVar list, so it's inaccesible
// via SKILLS.CFG. Any player hivehand tuning must take place in the code. (sjb)
	//MODDD -    NO - I THINK NOT.  here you go.
	//gSkillData.plrDmgHornet = 7;
	gSkillData.plrDmgHornet = GetSkillCvar( "sk_plr_hornet");




	// HEALTH/CHARGE
	gSkillData.suitchargerCapacity = GetSkillCvar( "sk_suitcharger" );
	gSkillData.batteryCapacity = GetSkillCvar( "sk_battery" );
	gSkillData.healthchargerCapacity = GetSkillCvar ( "sk_healthcharger" );
	gSkillData.healthkitCapacity = GetSkillCvar ( "sk_healthkit" );
	gSkillData.scientistHeal = GetSkillCvar ( "sk_scientist_heal" );

	// monster damage adj
	gSkillData.monHead = GetSkillCvar( "sk_monster_head" );
	gSkillData.monChest = GetSkillCvar( "sk_monster_chest" );
	gSkillData.monStomach = GetSkillCvar( "sk_monster_stomach" );
	gSkillData.monLeg = GetSkillCvar( "sk_monster_leg" );
	gSkillData.monArm = GetSkillCvar( "sk_monster_arm" );

	// player damage adj
	gSkillData.plrHead = GetSkillCvar( "sk_player_head" );
	gSkillData.plrChest = GetSkillCvar( "sk_player_chest" );
	gSkillData.plrStomach = GetSkillCvar( "sk_player_stomach" );
	gSkillData.plrLeg = GetSkillCvar( "sk_player_leg" );
	gSkillData.plrArm = GetSkillCvar( "sk_player_arm" );


	//MODDD - NEW BELOW!
	///////////////////////////////////////////////
	gSkillData.player_ammomax_9mm = GetSkillCvarSingular("player_ammomax_9mm");
	gSkillData.player_ammomax_mp5_grenade = GetSkillCvarSingular("player_ammomax_mp5_grenade");
	gSkillData.player_ammomax_revolver = GetSkillCvarSingular("player_ammomax_revolver");
	gSkillData.player_ammomax_shotgun = GetSkillCvarSingular("player_ammomax_shotgun");
	gSkillData.player_ammomax_crossbow = GetSkillCvarSingular("player_ammomax_crossbow");
	gSkillData.player_ammomax_rpg = GetSkillCvarSingular("player_ammomax_rpg");
	gSkillData.player_ammomax_uranium = GetSkillCvarSingular("player_ammomax_uranium");
	gSkillData.player_ammomax_handgrenade = GetSkillCvarSingular("player_ammomax_handgrenade");
	gSkillData.player_ammomax_satchel = GetSkillCvarSingular("player_ammomax_satchel");
	gSkillData.player_ammomax_tripmine = GetSkillCvarSingular("player_ammomax_tripmine");
	gSkillData.player_ammomax_hornet = GetSkillCvarSingular("player_ammomax_hornet");
	gSkillData.player_ammomax_snark = GetSkillCvarSingular("player_ammomax_snark");
	gSkillData.player_ammomax_chumtoad = GetSkillCvarSingular("player_ammomax_chumtoad");


	gSkillData.player_revive_health = GetSkillCvar("sk_player_revive_health");
	gSkillData.scientist_can_heal = GetSkillCvar("sk_scientist_can_heal");
	gSkillData.npc_drop_weapon = GetSkillCvar("sk_npc_drop_weapon");
	
	gSkillData.tdmg_buddha = GetSkillCvar("tdmg_buddha");
	gSkillData.tdmg_playerbuddha = GetSkillCvar("tdmg_playerbuddha");

	gSkillData.tdmg_paralyze_duration = GetSkillCvar("tdmg_paralyze_duration");

	gSkillData.tdmg_nervegas_duration = GetSkillCvar("tdmg_nervegas_duration");
	gSkillData.tdmg_nervegas_damage = GetSkillCvar("tdmg_nervegas_damage");

	gSkillData.tdmg_poison_duration = GetSkillCvar("tdmg_poison_duration");
	gSkillData.tdmg_poison_damage = GetSkillCvar("tdmg_poison_damage");

	gSkillData.tdmg_radiation_duration = GetSkillCvar("tdmg_radiation_duration");
	gSkillData.tdmg_radiation_damage = GetSkillCvar("tdmg_radiation_damage");

	gSkillData.tdmg_acid_duration = GetSkillCvar("tdmg_acid_duration");
	gSkillData.tdmg_acid_damage = GetSkillCvar("tdmg_acid_damage");

	gSkillData.tdmg_slowburn_duration = GetSkillCvar("tdmg_slowburn_duration");
	gSkillData.tdmg_slowburn_damage = GetSkillCvar("tdmg_slowburn_damage");

	gSkillData.tdmg_slowfreeze_duration = GetSkillCvar("tdmg_slowfreeze_duration");
	gSkillData.tdmg_slowfreeze_damage = GetSkillCvar("tdmg_slowfreeze_damage");

	gSkillData.tdmg_bleeding_duration = GetSkillCvar("tdmg_bleeding_duration");
	gSkillData.tdmg_bleeding_damage = GetSkillCvar("tdmg_bleeding_damage");


}

//=========================================================
// instantiate the proper game rules object
//=========================================================

CGameRules *InstallGameRules( void )
{
	SERVER_COMMAND( "exec game.cfg\n" );
	
	//MODDD - name changed, conflicted with a windows API call in
	// VS6.
	GOLDSRC_SERVER_EXECUTE( );


	if ( !gpGlobals->deathmatch )
	{
		// generic half-life
		g_teamplay = 0;
		return new CHalfLifeRules;
	}
	else
	{
		if ( teamplay.value > 0 )
		{
			// teamplay

			g_teamplay = 1;
			return new CHalfLifeTeamplay;
		}
		if ((int)gpGlobals->deathmatch == 1)
		{
			// vanilla deathmatch
			g_teamplay = 0;
			return new CHalfLifeMultiplay;
		}
		else
		{
			// vanilla deathmatch??
			g_teamplay = 0;
			return new CHalfLifeMultiplay;
		}
	}
}



