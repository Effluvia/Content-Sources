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
// skill.h - skill level concerns
//=========================================================

//MODDD - WHY NOT???
#ifndef SKILL_H
#define SKILL_H

#define SKILL_EASY		1
#define SKILL_MEDIUM	2
#define SKILL_HARD		3


extern DLL_GLOBAL int g_iSkillLevel;

float GetSkillCvar(char* pName);
float GetSkillCvarSingular(char* pName);




struct skilldata_t
{

	//MODDD - removed.  What does this offer that g_iSkillLevel below  doesn't?
	// Any mentions of this replaced with that.
	//int iSkillLevel; // game skill level

// Monster Health & Damage
	float agruntHealth;
	float agruntDmgPunch;

	float apacheHealth;
	
	float barnacleHealth;

	float barneyHealth;

	float bigmommaHealthFactor;		// Multiply each node's health by this
	float bigmommaDmgSlash;			// melee attack damage
	float bigmommaDmgBlast;			// mortar attack damage
	float bigmommaRadiusBlast;		// mortar attack radius

	float bullsquidHealth;
	float bullsquidDmgBite;
	float bullsquidDmgWhip;
	float bullsquidDmgSpit;

	float gargantuaHealth;
	float gargantuaDmgSlash;
	float gargantuaDmgFire;
	float gargantuaDmgStomp;

	float hassassinHealth;
	//MODDD - NEW.  Customizable damages like the player.
	float hassassinDmgCrossbowClient;
	float hassassinDmgCrossbowMonster;

	float headcrabHealth;
	float headcrabDmgBite;

	float hgruntHealth;
	float hgruntDmgKick;
	float hgruntShotgunPellets;
	float hgruntGrenadeSpeed;

	//MODDD - new.
	float hassaultHealth;
	float hassaultDmgMelee;
	

	float houndeyeHealth;
	float houndeyeDmgBlast;

	float slaveHealth;
	float slaveDmgClaw;
	float slaveDmgClawrake;
	float slaveDmgZap;

	float ichthyosaurHealth;
	float ichthyosaurDmgShake;

	float leechHealth;
	float leechDmgBite;

	//MODDD - new.
	float stukaBatHealth;
	float stukaBatDmgClaw;
	float stukaBatDmgExplosion;
	
	//MODDD - new.
	float panthereyeHealth;
	float panthereyeDmgClaw;


	

	float controllerHealth;
	float controllerDmgZap;
	float controllerSpeedBall;
	float controllerDmgBall;

	float nihilanthHealth;
	float nihilanthZap;

	float scientistHealth;
	//MODDD - NEW!
	float scientistDmgPunch;
	
	//MODDD - new!
	float chumtoadHealth;



	//MODDD TODO MAJOR - new NPCs.
	//snapbug
	//archer
	//flyer?





	float snarkHealth;
	float snarkDmgBite;
	float snarkDmgPop;

	float zombieHealth;
	float zombieDmgOneSlash;
	float zombieDmgBothSlash;
	//MODDD - extra zombie feature
	float zombieBulletResistance;
	float zombieBulletPushback;


	
	//MODDD - several new NPC's
	//Archer
	float archerHealth;

	//(here comes dat) Boid (oh shit waddup)
	float boidHealth;

	//Floater
	float floaterHealth;

	//Friendly
	float friendlyHealth;

	//Kingpin
	float kingpinHealth;






	float turretHealth;
	float miniturretHealth;
	float sentryHealth;


// Player Weapons
	float plrDmgCrowbar;
	float plrDmg9MM;
	float plrDmg357;
	float plrDmgMP5;
	float plrDmgM203Grenade;
	float plrDmgBuckshot;
	float plrDmgCrossbowClient;
	float plrDmgCrossbowMonster;
	float plrDmgRPG;
	float plrDmgGauss;
	float plrDmgEgonNarrow;
	float plrDmgEgonWide;
	float plrDmgHornet;
	float plrDmgHandGrenade;
	float plrDmgSatchel;
	float plrDmgTripmine;
	
// weapons shared by monsters
	float monDmg9MM;
	float monDmgMP5;
	float monDmg12MM;
	float monDmgHornet;
	//MODDD
	float monHealthHornet;

// health/suit charge
	float suitchargerCapacity;
	float batteryCapacity;
	float healthchargerCapacity;
	float healthkitCapacity;
	float scientistHeal;

// monster damage adj
	float monHead;
	float monChest;
	float monStomach;
	float monLeg;
	float monArm;

// player damage adj
	float plrHead;
	float plrChest;
	float plrStomach;
	float plrLeg;
	float plrArm;


	//MODDD - NEW BELOW!
	//////////////////////////////////////////////////

	// glock/mp5
	float player_ammomax_9mm;
	float player_ammomax_mp5_grenade;
	float player_ammomax_revolver;
	float player_ammomax_shotgun;
	float player_ammomax_crossbow;
	float player_ammomax_rpg;
	// gauss/egon
	float player_ammomax_uranium;
	float player_ammomax_handgrenade;
	float player_ammomax_satchel;
	float player_ammomax_tripmine;
	float player_ammomax_hornet;
	float player_ammomax_snark;
	float player_ammomax_chumtoad;



	float player_revive_health;
	float scientist_can_heal;
	float npc_drop_weapon;


	float tdmg_buddha;
	float tdmg_playerbuddha;


	float tdmg_paralyze_duration;

	float tdmg_nervegas_duration;
	float tdmg_nervegas_damage;

	float tdmg_poison_duration;
	float tdmg_poison_damage;

	float tdmg_radiation_duration;
	float tdmg_radiation_damage;

	float tdmg_acid_duration;
	float tdmg_acid_damage;

	float tdmg_slowburn_duration;
	float tdmg_slowburn_damage;

	float tdmg_slowfreeze_duration;
	float tdmg_slowfreeze_damage;

	float tdmg_bleeding_duration;
	float tdmg_bleeding_damage;



};



extern DLL_GLOBAL skilldata_t gSkillData;

#endif //END OF #ifdef SKILL_H