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
#include "eiface.h"
#include "util.h"
#include "game.h"
#include "util_debugdraw.h"
#include "util_version.h"

#include "cvar_custom_info.h"
#include "cvar_custom_list.h"


EASY_CVAR_CREATE_SERVER_SETUP_MASS


EASY_CVAR_EXTERN_DEBUGONLY(hiddenMemPrintout)
EASY_CVAR_EXTERN_DEBUGONLY(emergencyFix)
EASY_CVAR_EXTERN(soundSentenceSave)

EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST(sv_germancensorship)
EASY_CVAR_EXTERN_DEBUGONLY(allowGermanModels)
EASY_CVAR_EXTERN_DEBUGONLY(forceWorldLightOff)


//MODDD - extern
extern BOOL globalPSEUDO_iCanHazMemez;
extern float globalPSEUDO_cl_bullsquidspit;
extern float globalPSEUDO_cl_hornettrail;
extern float globalPSEUDO_cl_hornetspiral;
extern float globalPSEUDO_germanCensorshipMem;
extern float globalPSEUDO_allowGermanModelsMem;
extern float forceWorldLightOffMem;

//extern cvar_t* cvar_sv_cheats;
extern BOOL queueSkillUpdate;

extern void resetModCVars(CBasePlayer* arg_plyRef, BOOL isEmergency);



//MODDD - NEW!  Version-related CVars, used to be named "protoVersionS" and "protoDateS".
//...replaced by a console command now, disregard this.
//cvar_t	sv_mod_version	= {"sv_mod_version", "0", FCVAR_SERVER | FCVAR_UNLOGGED | FCVAR_SPONLY };
//cvar_t	sv_mod_date	= {"sv_mod_date", "0", FCVAR_SERVER | FCVAR_UNLOGGED | FCVAR_SPONLY };

cvar_t* global_test_cvar_ref = NULL;

cvar_t	test_cvar	= {"test_cvar", "6", 0 };
//FCVAR_SERVER | FCVAR_SPONLY

//MODDD - CVAR TEST
void test_cvar_create(){
	// note - CVAR_CREATE and CVAR_REGISTER are the exact same call.
	CVAR_CREATE(&test_cvar);

	//ALSO, serverside CVars implicitly get the FCVAR_EXTDLL flag.
	// clientside CVars (registered in cl_dlls/hud.cpp typically) get the FCVAR_CLIENTDLL flag.
	// I know, go figure.  But it's good for this to be spelled out somewhere.
	// AHEM.  *COUGH*.   *COUGH*.

	// GENERAL RULE OF THUMB WITH CVARS:
	// I think getting the '->value' of a pointer to a CVar from any earlier point is OK.
	// Setting the value, like  "...->value = 36.2f",  is not.   Use engine calls to do this.
	// Any desynch between what ->value, engine calls for reading CVars, or the ingame console
	// (not intended to work for serverside CVars for connected players not running the server)
	// would be bad.  Although "->value"'s being unable to be trusted wouldn't be that terrible.

	

	// This test has been done.  Looks like the pointer returned from CVAR_GET_POINTER
	// is not the same as the one supplied to CVAR_CREATE, for a request of the same name.
	// I'm guessing CVAR_CREATE generates a copy from the supplied cvar_t and then returns
	// some part of memory where it's updated in real-time from changes to the CVar.
	// ...then what's the point of supplying the cvar_t struct anyway instead of just a bunch
	// of separate parameters like clientside does?  WHO KNOWS.
	/*
	cvar_t* test_cvar_tempRef = CVAR_GET_POINTER("test_cvar");
	
	if(test_cvar_tempRef == &test_cvar){
		easyForcePrintLine("test_cvar re-get test: IT MATCHES!");
	}else{
		easyForcePrintLine("test_cvar re-get test: Nope, well what the fuck.");
	}
	*/

	//global_test_cvar_ref = CVAR_GET_POINTER("test_cvar");

}//END OF test_cvar_create
	
	


cvar_t	displaysoundlist = {"displaysoundlist","0"};

// multiplayer server rules
cvar_t	fragsleft	= {"mp_fragsleft","0", FCVAR_SERVER | FCVAR_UNLOGGED };	  // Don't spam console/log files/users with this changing
cvar_t	timeleft	= {"mp_timeleft","0" , FCVAR_SERVER | FCVAR_UNLOGGED };	  // "      "

// multiplayer server rules
cvar_t	teamplay	= {"mp_teamplay","0", FCVAR_SERVER };
cvar_t	fraglimit	= {"mp_fraglimit","0", FCVAR_SERVER };
cvar_t	timelimit	= {"mp_timelimit","0", FCVAR_SERVER };
cvar_t	friendlyfire= {"mp_friendlyfire","0", FCVAR_SERVER };
cvar_t	falldamage	= {"mp_falldamage","0", FCVAR_SERVER };
cvar_t	weaponstay	= {"mp_weaponstay","0", FCVAR_SERVER };
cvar_t	forcerespawn= {"mp_forcerespawn","1", FCVAR_SERVER };
cvar_t	flashlight	= {"mp_flashlight","0", FCVAR_SERVER };
cvar_t	aimcrosshair= {"mp_autocrosshair","1", FCVAR_SERVER };
cvar_t	decalfrequency = {"decalfrequency","30", FCVAR_SERVER };
cvar_t	teamlist = {"mp_teamlist","hgrunt;scientist", FCVAR_SERVER };
cvar_t	teamoverride = {"mp_teamoverride","1" };
cvar_t	defaultteam = {"mp_defaultteam","0" };
cvar_t	allowmonsters={"mp_allowmonsters","0", FCVAR_SERVER };

cvar_t  mp_chattime = {"mp_chattime","10", FCVAR_SERVER };



//TEST
cvar_t  pregame_server_cvar = { "pregame_server_cvar", "0", FCVAR_SERVER | FCVAR_ARCHIVE };


// Engine Cvars
cvar_t 	*g_psv_gravity = NULL;
cvar_t	*g_psv_aim = NULL;
cvar_t	*g_footsteps = NULL;

//CVARS FOR SKILL LEVEL SETTINGS
// Agrunt
cvar_t	sk_agrunt_health1 = {"sk_agrunt_health1","0"};
cvar_t	sk_agrunt_health2 = {"sk_agrunt_health2","0"};
cvar_t	sk_agrunt_health3 = {"sk_agrunt_health3","0"};

cvar_t	sk_agrunt_dmg_punch1 = {"sk_agrunt_dmg_punch1","0"};
cvar_t	sk_agrunt_dmg_punch2 = {"sk_agrunt_dmg_punch2","0"};
cvar_t	sk_agrunt_dmg_punch3 = {"sk_agrunt_dmg_punch3","0"};

// Apache
cvar_t	sk_apache_health1	= {"sk_apache_health1","0"};
cvar_t	sk_apache_health2	= {"sk_apache_health2","0"};
cvar_t	sk_apache_health3	= {"sk_apache_health3","0"};

// MODDD - new. Barnacle
cvar_t	sk_barnacle_health1	= {"sk_barnacle_health1","25"};
cvar_t	sk_barnacle_health2	= {"sk_barnacle_health2","25"};
cvar_t	sk_barnacle_health3	= {"sk_barnacle_health3","25"};

// Barney
cvar_t	sk_barney_health1	= {"sk_barney_health1","0"};
cvar_t	sk_barney_health2	= {"sk_barney_health2","0"};
cvar_t	sk_barney_health3	= {"sk_barney_health3","0"};

// Bullsquid
cvar_t	sk_bullsquid_health1 = {"sk_bullsquid_health1","0"};
cvar_t	sk_bullsquid_health2 = {"sk_bullsquid_health2","0"};
cvar_t	sk_bullsquid_health3 = {"sk_bullsquid_health3","0"};

cvar_t	sk_bullsquid_dmg_bite1 = {"sk_bullsquid_dmg_bite1","0"};
cvar_t	sk_bullsquid_dmg_bite2 = {"sk_bullsquid_dmg_bite2","0"};
cvar_t	sk_bullsquid_dmg_bite3 = {"sk_bullsquid_dmg_bite3","0"};

cvar_t	sk_bullsquid_dmg_whip1 = {"sk_bullsquid_dmg_whip1","0"};
cvar_t	sk_bullsquid_dmg_whip2 = {"sk_bullsquid_dmg_whip2","0"};
cvar_t	sk_bullsquid_dmg_whip3 = {"sk_bullsquid_dmg_whip3","0"};

cvar_t	sk_bullsquid_dmg_spit1 = {"sk_bullsquid_dmg_spit1","0"};
cvar_t	sk_bullsquid_dmg_spit2 = {"sk_bullsquid_dmg_spit2","0"};
cvar_t	sk_bullsquid_dmg_spit3 = {"sk_bullsquid_dmg_spit3","0"};


// Big Momma
cvar_t	sk_bigmomma_health_factor1 = {"sk_bigmomma_health_factor1","1.0"};
cvar_t	sk_bigmomma_health_factor2 = {"sk_bigmomma_health_factor2","1.0"};
cvar_t	sk_bigmomma_health_factor3 = {"sk_bigmomma_health_factor3","1.0"};

cvar_t	sk_bigmomma_dmg_slash1 = {"sk_bigmomma_dmg_slash1","50"};
cvar_t	sk_bigmomma_dmg_slash2 = {"sk_bigmomma_dmg_slash2","50"};
cvar_t	sk_bigmomma_dmg_slash3 = {"sk_bigmomma_dmg_slash3","50"};

cvar_t	sk_bigmomma_dmg_blast1 = {"sk_bigmomma_dmg_blast1","100"};
cvar_t	sk_bigmomma_dmg_blast2 = {"sk_bigmomma_dmg_blast2","100"};
cvar_t	sk_bigmomma_dmg_blast3 = {"sk_bigmomma_dmg_blast3","100"};

cvar_t	sk_bigmomma_radius_blast1 = {"sk_bigmomma_radius_blast1","250"};
cvar_t	sk_bigmomma_radius_blast2 = {"sk_bigmomma_radius_blast2","250"};
cvar_t	sk_bigmomma_radius_blast3 = {"sk_bigmomma_radius_blast3","250"};

// Gargantua
cvar_t	sk_gargantua_health1 = {"sk_gargantua_health1","0"};
cvar_t	sk_gargantua_health2 = {"sk_gargantua_health2","0"};
cvar_t	sk_gargantua_health3 = {"sk_gargantua_health3","0"};

cvar_t	sk_gargantua_dmg_slash1	= {"sk_gargantua_dmg_slash1","0"};
cvar_t	sk_gargantua_dmg_slash2	= {"sk_gargantua_dmg_slash2","0"};
cvar_t	sk_gargantua_dmg_slash3	= {"sk_gargantua_dmg_slash3","0"};

cvar_t	sk_gargantua_dmg_fire1 = {"sk_gargantua_dmg_fire1","0"};
cvar_t	sk_gargantua_dmg_fire2 = {"sk_gargantua_dmg_fire2","0"};
cvar_t	sk_gargantua_dmg_fire3 = {"sk_gargantua_dmg_fire3","0"};

cvar_t	sk_gargantua_dmg_stomp1	= {"sk_gargantua_dmg_stomp1","0"};
cvar_t	sk_gargantua_dmg_stomp2	= {"sk_gargantua_dmg_stomp2","0"};
cvar_t	sk_gargantua_dmg_stomp3	= {"sk_gargantua_dmg_stomp3","0"};


// Hassassin
cvar_t	sk_hassassin_health1 = {"sk_hassassin_health1","0"};
cvar_t	sk_hassassin_health2 = {"sk_hassassin_health2","0"};
cvar_t	sk_hassassin_health3 = {"sk_hassassin_health3","0"};

//MODDD - crossbow CVars for the hassassin
cvar_t	sk_hassassin_xbow_cl1 = {"sk_hassassin_xbow_cl1","0"};
cvar_t	sk_hassassin_xbow_cl2 = {"sk_hassassin_xbow_cl2","0"};
cvar_t	sk_hassassin_xbow_cl3 = {"sk_hassassin_xbow_cl3","0"};
cvar_t	sk_hassassin_xbow_mo1 = {"sk_hassassin_xbow_mo1","0"};
cvar_t	sk_hassassin_xbow_mo2 = {"sk_hassassin_xbow_mo2","0"};
cvar_t	sk_hassassin_xbow_mo3 = {"sk_hassassin_xbow_mo3","0"};



// Headcrab
cvar_t	sk_headcrab_health1 = {"sk_headcrab_health1","0"};
cvar_t	sk_headcrab_health2 = {"sk_headcrab_health2","0"};
cvar_t	sk_headcrab_health3 = {"sk_headcrab_health3","0"};

cvar_t	sk_headcrab_dmg_bite1 = {"sk_headcrab_dmg_bite1","0"};
cvar_t	sk_headcrab_dmg_bite2 = {"sk_headcrab_dmg_bite2","0"};
cvar_t	sk_headcrab_dmg_bite3 = {"sk_headcrab_dmg_bite3","0"};


// Hgrunt 
cvar_t	sk_hgrunt_health1 = {"sk_hgrunt_health1","0"};
cvar_t	sk_hgrunt_health2 = {"sk_hgrunt_health2","0"};
cvar_t	sk_hgrunt_health3 = {"sk_hgrunt_health3","0"};

cvar_t	sk_hgrunt_kick1 = {"sk_hgrunt_kick1","0"};
cvar_t	sk_hgrunt_kick2 = {"sk_hgrunt_kick2","0"};
cvar_t	sk_hgrunt_kick3 = {"sk_hgrunt_kick3","0"};

cvar_t	sk_hgrunt_pellets1 = {"sk_hgrunt_pellets1","0"};
cvar_t	sk_hgrunt_pellets2 = {"sk_hgrunt_pellets2","0"};
cvar_t	sk_hgrunt_pellets3 = {"sk_hgrunt_pellets3","0"};

cvar_t	sk_hgrunt_gspeed1 = {"sk_hgrunt_gspeed1","0"};
cvar_t	sk_hgrunt_gspeed2 = {"sk_hgrunt_gspeed2","0"};
cvar_t	sk_hgrunt_gspeed3 = {"sk_hgrunt_gspeed3","0"};

//MODD - hassault
cvar_t	sk_hassault_health1 = {"sk_hassault_health1","0"};
cvar_t	sk_hassault_health2 = {"sk_hassault_health2","0"};
cvar_t	sk_hassault_health3 = {"sk_hassault_health3","0"};

cvar_t	sk_hassault_dmg_melee1 = {"sk_hassault_dmg_melee1","0"};
cvar_t	sk_hassault_dmg_melee2 = {"sk_hassault_dmg_melee2","0"};
cvar_t	sk_hassault_dmg_melee3 = {"sk_hassault_dmg_melee3","0"};



// Houndeye
cvar_t	sk_houndeye_health1 = {"sk_houndeye_health1","0"};
cvar_t	sk_houndeye_health2 = {"sk_houndeye_health2","0"};
cvar_t	sk_houndeye_health3 = {"sk_houndeye_health3","0"};

cvar_t	sk_houndeye_dmg_blast1 = {"sk_houndeye_dmg_blast1","0"};
cvar_t	sk_houndeye_dmg_blast2 = {"sk_houndeye_dmg_blast2","0"};
cvar_t	sk_houndeye_dmg_blast3 = {"sk_houndeye_dmg_blast3","0"};


// ISlave
cvar_t	sk_islave_health1 = {"sk_islave_health1","0"};
cvar_t	sk_islave_health2 = {"sk_islave_health2","0"};
cvar_t	sk_islave_health3 = {"sk_islave_health3","0"};

cvar_t	sk_islave_dmg_claw1 = {"sk_islave_dmg_claw1","0"};
cvar_t	sk_islave_dmg_claw2 = {"sk_islave_dmg_claw2","0"};
cvar_t	sk_islave_dmg_claw3 = {"sk_islave_dmg_claw3","0"};

cvar_t	sk_islave_dmg_clawrake1	= {"sk_islave_dmg_clawrake1","0"};
cvar_t	sk_islave_dmg_clawrake2	= {"sk_islave_dmg_clawrake2","0"};
cvar_t	sk_islave_dmg_clawrake3	= {"sk_islave_dmg_clawrake3","0"};
	
cvar_t	sk_islave_dmg_zap1 = {"sk_islave_dmg_zap1","0"};
cvar_t	sk_islave_dmg_zap2 = {"sk_islave_dmg_zap2","0"};
cvar_t	sk_islave_dmg_zap3 = {"sk_islave_dmg_zap3","0"};


// Icthyosaur
cvar_t	sk_ichthyosaur_health1	= {"sk_ichthyosaur_health1","0"};
cvar_t	sk_ichthyosaur_health2	= {"sk_ichthyosaur_health2","0"};
cvar_t	sk_ichthyosaur_health3	= {"sk_ichthyosaur_health3","0"};

cvar_t	sk_ichthyosaur_shake1	= {"sk_ichthyosaur_shake1","0"};
cvar_t	sk_ichthyosaur_shake2	= {"sk_ichthyosaur_shake2","0"};
cvar_t	sk_ichthyosaur_shake3	= {"sk_ichthyosaur_shake3","0"};


// Leech
cvar_t	sk_leech_health1 = {"sk_leech_health1","0"};
cvar_t	sk_leech_health2 = {"sk_leech_health2","0"};
cvar_t	sk_leech_health3 = {"sk_leech_health3","0"};

cvar_t	sk_leech_dmg_bite1 = {"sk_leech_dmg_bite1","0"};
cvar_t	sk_leech_dmg_bite2 = {"sk_leech_dmg_bite2","0"};
cvar_t	sk_leech_dmg_bite3 = {"sk_leech_dmg_bite3","0"};


//MODDD - new
// StukaBat
cvar_t	sk_stukabat_health1 = {"sk_stukabat_health1","0"};
cvar_t	sk_stukabat_health2 = {"sk_stukabat_health2","0"};
cvar_t	sk_stukabat_health3 = {"sk_stukabat_health3","0"};
cvar_t	sk_stukabat_dmgclaw1 = {"sk_stukabat_dmgclaw1","0"};
cvar_t	sk_stukabat_dmgclaw2 = {"sk_stukabat_dmgclaw2","0"};
cvar_t	sk_stukabat_dmgclaw3 = {"sk_stukabat_dmgclaw3","0"};

/*
cvar_t	sk_stukabat_dmgexplosion1 = {"sk_stukabat_dmgexplosion1","0"};
cvar_t	sk_stukabat_dmgexplosion2 = {"sk_stukabat_dmgexplosion2","0"};
cvar_t	sk_stukabat_dmgexplosion3 = {"sk_stukabat_dmgexplosion3","0"};
*/

//MODDD - new
// Panther-Eye
cvar_t	sk_panthereye_health1 = {"sk_panthereye_health1","0"};
cvar_t	sk_panthereye_health2 = {"sk_panthereye_health2","0"};
cvar_t	sk_panthereye_health3 = {"sk_panthereye_health3","0"};
cvar_t	sk_panthereye_dmgclaw1 = {"sk_panthereye_dmgclaw1","0"};
cvar_t	sk_panthereye_dmgclaw2 = {"sk_panthereye_dmgclaw2","0"};
cvar_t	sk_panthereye_dmgclaw3 = {"sk_panthereye_dmgclaw3","0"};






// Controller
cvar_t	sk_controller_health1 = {"sk_controller_health1","0"};
cvar_t	sk_controller_health2 = {"sk_controller_health2","0"};
cvar_t	sk_controller_health3 = {"sk_controller_health3","0"};

cvar_t	sk_controller_dmgzap1 = {"sk_controller_dmgzap1","0"};
cvar_t	sk_controller_dmgzap2 = {"sk_controller_dmgzap2","0"};
cvar_t	sk_controller_dmgzap3 = {"sk_controller_dmgzap3","0"};

cvar_t	sk_controller_speedball1 = {"sk_controller_speedball1","0"};
cvar_t	sk_controller_speedball2 = {"sk_controller_speedball2","0"};
cvar_t	sk_controller_speedball3 = {"sk_controller_speedball3","0"};

cvar_t	sk_controller_dmgball1 = {"sk_controller_dmgball1","0"};
cvar_t	sk_controller_dmgball2 = {"sk_controller_dmgball2","0"};
cvar_t	sk_controller_dmgball3 = {"sk_controller_dmgball3","0"};

// Nihilanth
cvar_t	sk_nihilanth_health1 = {"sk_nihilanth_health1","0"};
cvar_t	sk_nihilanth_health2 = {"sk_nihilanth_health2","0"};
cvar_t	sk_nihilanth_health3 = {"sk_nihilanth_health3","0"};

cvar_t	sk_nihilanth_zap1 = {"sk_nihilanth_zap1","0"};
cvar_t	sk_nihilanth_zap2 = {"sk_nihilanth_zap2","0"};
cvar_t	sk_nihilanth_zap3 = {"sk_nihilanth_zap3","0"};

// Scientist
cvar_t	sk_scientist_health1 = {"sk_scientist_health1","0"};
cvar_t	sk_scientist_health2 = {"sk_scientist_health2","0"};
cvar_t	sk_scientist_health3 = {"sk_scientist_health3","0"};

//MODDD - new!  scientistDmgPunch
cvar_t  sk_scientist_dmg_punch1 = {"sk_scientist_dmg_punch1", "0"};
cvar_t  sk_scientist_dmg_punch2 = {"sk_scientist_dmg_punch2", "0"};
cvar_t  sk_scientist_dmg_punch3 = {"sk_scientist_dmg_punch3", "0"};


//MODDD - new! chumtoad health.
cvar_t	sk_chumtoad_health1 = {"sk_chumtoad_health1","0"};
cvar_t	sk_chumtoad_health2 = {"sk_chumtoad_health2","0"};
cvar_t	sk_chumtoad_health3 = {"sk_chumtoad_health3","0"};

// Snark
cvar_t	sk_snark_health1 = {"sk_snark_health1","0"};
cvar_t	sk_snark_health2 = {"sk_snark_health2","0"};
cvar_t	sk_snark_health3 = {"sk_snark_health3","0"};

cvar_t	sk_snark_dmg_bite1 = {"sk_snark_dmg_bite1","0"};
cvar_t	sk_snark_dmg_bite2 = {"sk_snark_dmg_bite2","0"};
cvar_t	sk_snark_dmg_bite3 = {"sk_snark_dmg_bite3","0"};

cvar_t	sk_snark_dmg_pop1 = {"sk_snark_dmg_pop1","0"};
cvar_t	sk_snark_dmg_pop2 = {"sk_snark_dmg_pop2","0"};
cvar_t	sk_snark_dmg_pop3 = {"sk_snark_dmg_pop3","0"};



// Zombie
cvar_t	sk_zombie_health1 = {"sk_zombie_health1","0"};
cvar_t	sk_zombie_health2 = {"sk_zombie_health2","0"};
cvar_t	sk_zombie_health3 = {"sk_zombie_health3","0"};

//MODDD - bullet resistance mod.
cvar_t	sk_zombie_bulletresistance1 = {"sk_zombie_bulletresistance1","0"};
cvar_t	sk_zombie_bulletresistance2 = {"sk_zombie_bulletresistance2","0"};
cvar_t	sk_zombie_bulletresistance3 = {"sk_zombie_bulletresistance3","0"};
//MODDD - also
cvar_t	sk_zombie_bulletpushback1 = {"sk_zombie_bulletpushback1","0"};
cvar_t	sk_zombie_bulletpushback2 = {"sk_zombie_bulletpushback2","0"};
cvar_t	sk_zombie_bulletpushback3 = {"sk_zombie_bulletpushback3","0"};






//MODDD - several new NPC's
//Archer
cvar_t	sk_archer_health1 = {"sk_archer_health1","0"};
cvar_t	sk_archer_health2 = {"sk_archer_health2","0"};
cvar_t	sk_archer_health3 = {"sk_archer_health3","0"};

//(here comes dat) Boid (oh shit waddup)
cvar_t	sk_boid_health1 = {"sk_boid_health1","0"};
cvar_t	sk_boid_health2 = {"sk_boid_health2","0"};
cvar_t	sk_boid_health3 = {"sk_boid_health3","0"};

//Floater
cvar_t	sk_floater_health1 = {"sk_floater_health1","0"};
cvar_t	sk_floater_health2 = {"sk_floater_health2","0"};
cvar_t	sk_floater_health3 = {"sk_floater_health3","0"};

//Friendly
cvar_t	sk_friendly_health1 = {"sk_friendly_health1","0"};
cvar_t	sk_friendly_health2 = {"sk_friendly_health2","0"};
cvar_t	sk_friendly_health3 = {"sk_friendly_health3","0"};

//Kingpin
cvar_t	sk_kingpin_health1 = {"sk_kingpin_health1","0"};
cvar_t	sk_kingpin_health2 = {"sk_kingpin_health2","0"};
cvar_t	sk_kingpin_health3 = {"sk_kingpin_health3","0"};









cvar_t	sk_zombie_dmg_one_slash1 = {"sk_zombie_dmg_one_slash1","0"};
cvar_t	sk_zombie_dmg_one_slash2 = {"sk_zombie_dmg_one_slash2","0"};
cvar_t	sk_zombie_dmg_one_slash3 = {"sk_zombie_dmg_one_slash3","0"};

cvar_t	sk_zombie_dmg_both_slash1 = {"sk_zombie_dmg_both_slash1","0"};
cvar_t	sk_zombie_dmg_both_slash2 = {"sk_zombie_dmg_both_slash2","0"};
cvar_t	sk_zombie_dmg_both_slash3 = {"sk_zombie_dmg_both_slash3","0"};


//Turret
cvar_t	sk_turret_health1 = {"sk_turret_health1","0"};
cvar_t	sk_turret_health2 = {"sk_turret_health2","0"};
cvar_t	sk_turret_health3 = {"sk_turret_health3","0"};


// MiniTurret
cvar_t	sk_miniturret_health1 = {"sk_miniturret_health1","0"};
cvar_t	sk_miniturret_health2 = {"sk_miniturret_health2","0"};
cvar_t	sk_miniturret_health3 = {"sk_miniturret_health3","0"};


// Sentry Turret
cvar_t	sk_sentry_health1 = {"sk_sentry_health1","0"};
cvar_t	sk_sentry_health2 = {"sk_sentry_health2","0"};
cvar_t	sk_sentry_health3 = {"sk_sentry_health3","0"};


// PLAYER WEAPONS

// Crowbar whack
cvar_t	sk_plr_crowbar1 = {"sk_plr_crowbar1","0"};
cvar_t	sk_plr_crowbar2 = {"sk_plr_crowbar2","0"};
cvar_t	sk_plr_crowbar3 = {"sk_plr_crowbar3","0"};

// Glock Round
cvar_t	sk_plr_9mm_bullet1 = {"sk_plr_9mm_bullet1","0"};
cvar_t	sk_plr_9mm_bullet2 = {"sk_plr_9mm_bullet2","0"};
cvar_t	sk_plr_9mm_bullet3 = {"sk_plr_9mm_bullet3","0"};

// 357 Round
cvar_t	sk_plr_357_bullet1 = {"sk_plr_357_bullet1","0"};
cvar_t	sk_plr_357_bullet2 = {"sk_plr_357_bullet2","0"};
cvar_t	sk_plr_357_bullet3 = {"sk_plr_357_bullet3","0"};

// MP5 Round
cvar_t	sk_plr_9mmAR_bullet1 = {"sk_plr_9mmAR_bullet1","0"};
cvar_t	sk_plr_9mmAR_bullet2 = {"sk_plr_9mmAR_bullet2","0"};
cvar_t	sk_plr_9mmAR_bullet3 = {"sk_plr_9mmAR_bullet3","0"};


// M203 grenade
cvar_t	sk_plr_9mmAR_grenade1 = {"sk_plr_9mmAR_grenade1","0"};
cvar_t	sk_plr_9mmAR_grenade2 = {"sk_plr_9mmAR_grenade2","0"};
cvar_t	sk_plr_9mmAR_grenade3 = {"sk_plr_9mmAR_grenade3","0"};


// Shotgun buckshot
cvar_t	sk_plr_buckshot1 = {"sk_plr_buckshot1","0"};
cvar_t	sk_plr_buckshot2 = {"sk_plr_buckshot2","0"};
cvar_t	sk_plr_buckshot3 = {"sk_plr_buckshot3","0"};


// Crossbow
cvar_t	sk_plr_xbow_bolt_client1 = {"sk_plr_xbow_bolt_client1","0"};
cvar_t	sk_plr_xbow_bolt_client2 = {"sk_plr_xbow_bolt_client2","0"};
cvar_t	sk_plr_xbow_bolt_client3 = {"sk_plr_xbow_bolt_client3","0"};

cvar_t	sk_plr_xbow_bolt_monster1 = {"sk_plr_xbow_bolt_monster1","0"};
cvar_t	sk_plr_xbow_bolt_monster2 = {"sk_plr_xbow_bolt_monster2","0"};
cvar_t	sk_plr_xbow_bolt_monster3 = {"sk_plr_xbow_bolt_monster3","0"};


// RPG
cvar_t	sk_plr_rpg1 = {"sk_plr_rpg1","0"};
cvar_t	sk_plr_rpg2 = {"sk_plr_rpg2","0"};
cvar_t	sk_plr_rpg3 = {"sk_plr_rpg3","0"};


// Zero Point Generator
cvar_t	sk_plr_gauss1 = {"sk_plr_gauss1","0"};
cvar_t	sk_plr_gauss2 = {"sk_plr_gauss2","0"};
cvar_t	sk_plr_gauss3 = {"sk_plr_gauss3","0"};


// Tau Cannon
cvar_t	sk_plr_egon_narrow1 = {"sk_plr_egon_narrow1","0"};
cvar_t	sk_plr_egon_narrow2 = {"sk_plr_egon_narrow2","0"};
cvar_t	sk_plr_egon_narrow3 = {"sk_plr_egon_narrow3","0"};

cvar_t	sk_plr_egon_wide1 = {"sk_plr_egon_wide1","0"};
cvar_t	sk_plr_egon_wide2 = {"sk_plr_egon_wide2","0"};
cvar_t	sk_plr_egon_wide3 = {"sk_plr_egon_wide3","0"};


// Hand Grendade
cvar_t	sk_plr_hand_grenade1 = {"sk_plr_hand_grenade1","0"};
cvar_t	sk_plr_hand_grenade2 = {"sk_plr_hand_grenade2","0"};
cvar_t	sk_plr_hand_grenade3 = {"sk_plr_hand_grenade3","0"};


// Satchel Charge
cvar_t	sk_plr_satchel1	= {"sk_plr_satchel1","0"};
cvar_t	sk_plr_satchel2	= {"sk_plr_satchel2","0"};
cvar_t	sk_plr_satchel3	= {"sk_plr_satchel3","0"};


// Tripmine
cvar_t	sk_plr_tripmine1 = {"sk_plr_tripmine1","0"};
cvar_t	sk_plr_tripmine2 = {"sk_plr_tripmine2","0"};
cvar_t	sk_plr_tripmine3 = {"sk_plr_tripmine3","0"};


// WORLD WEAPONS
cvar_t	sk_12mm_bullet1 = {"sk_12mm_bullet1","0"};
cvar_t	sk_12mm_bullet2 = {"sk_12mm_bullet2","0"};
cvar_t	sk_12mm_bullet3 = {"sk_12mm_bullet3","0"};

cvar_t	sk_9mmAR_bullet1 = {"sk_9mmAR_bullet1","0"};
cvar_t	sk_9mmAR_bullet2 = {"sk_9mmAR_bullet2","0"};
cvar_t	sk_9mmAR_bullet3 = {"sk_9mmAR_bullet3","0"};

cvar_t	sk_9mm_bullet1 = {"sk_9mm_bullet1","0"};
cvar_t	sk_9mm_bullet2 = {"sk_9mm_bullet2","0"};
cvar_t	sk_9mm_bullet3 = {"sk_9mm_bullet3","0"};


// HORNET
cvar_t	sk_hornet_dmg1 = {"sk_hornet_dmg1","0"};
cvar_t	sk_hornet_dmg2 = {"sk_hornet_dmg2","0"};
cvar_t	sk_hornet_dmg3 = {"sk_hornet_dmg3","0"};

//MODDD - hornet health.
cvar_t	sk_hornet_health1 = {"sk_hornet_health1","1"};
cvar_t	sk_hornet_health2 = {"sk_hornet_health2","1"};
cvar_t	sk_hornet_health3 = {"sk_hornet_health3","1"};

//MODDD - we're doing the player's hornet damage.  Leaving to default at 7 for now.
cvar_t	sk_plr_hornet1 = {"sk_plr_hornet1","7"};
cvar_t	sk_plr_hornet2 = {"sk_plr_hornet2","7"};
cvar_t	sk_plr_hornet3 = {"sk_plr_hornet3","7"};


// HEALTH/CHARGE
cvar_t	sk_suitcharger1	= { "sk_suitcharger1","0" };
cvar_t	sk_suitcharger2	= { "sk_suitcharger2","0" };		
cvar_t	sk_suitcharger3	= { "sk_suitcharger3","0" };		

cvar_t	sk_battery1	= { "sk_battery1","0" };			
cvar_t	sk_battery2	= { "sk_battery2","0" };			
cvar_t	sk_battery3	= { "sk_battery3","0" };			

cvar_t	sk_healthcharger1	= { "sk_healthcharger1","0" };		
cvar_t	sk_healthcharger2	= { "sk_healthcharger2","0" };		
cvar_t	sk_healthcharger3	= { "sk_healthcharger3","0" };		

cvar_t	sk_healthkit1	= { "sk_healthkit1","0" };		
cvar_t	sk_healthkit2	= { "sk_healthkit2","0" };		
cvar_t	sk_healthkit3	= { "sk_healthkit3","0" };		

cvar_t	sk_scientist_heal1	= { "sk_scientist_heal1","0" };	
cvar_t	sk_scientist_heal2	= { "sk_scientist_heal2","0" };	
cvar_t	sk_scientist_heal3	= { "sk_scientist_heal3","0" };	


// monster damage adjusters
cvar_t	sk_monster_head1	= { "sk_monster_head1","2" };
cvar_t	sk_monster_head2	= { "sk_monster_head2","2" };
cvar_t	sk_monster_head3	= { "sk_monster_head3","2" };

cvar_t	sk_monster_chest1	= { "sk_monster_chest1","1" };
cvar_t	sk_monster_chest2	= { "sk_monster_chest2","1" };
cvar_t	sk_monster_chest3	= { "sk_monster_chest3","1" };

cvar_t	sk_monster_stomach1	= { "sk_monster_stomach1","1" };
cvar_t	sk_monster_stomach2	= { "sk_monster_stomach2","1" };
cvar_t	sk_monster_stomach3	= { "sk_monster_stomach3","1" };

cvar_t	sk_monster_arm1	= { "sk_monster_arm1","1" };
cvar_t	sk_monster_arm2	= { "sk_monster_arm2","1" };
cvar_t	sk_monster_arm3	= { "sk_monster_arm3","1" };

cvar_t	sk_monster_leg1	= { "sk_monster_leg1","1" };
cvar_t	sk_monster_leg2	= { "sk_monster_leg2","1" };
cvar_t	sk_monster_leg3	= { "sk_monster_leg3","1" };

// player damage adjusters
cvar_t	sk_player_head1	= { "sk_player_head1","2" };
cvar_t	sk_player_head2	= { "sk_player_head2","2" };
cvar_t	sk_player_head3	= { "sk_player_head3","2" };

cvar_t	sk_player_chest1	= { "sk_player_chest1","1" };
cvar_t	sk_player_chest2	= { "sk_player_chest2","1" };
cvar_t	sk_player_chest3	= { "sk_player_chest3","1" };

cvar_t	sk_player_stomach1	= { "sk_player_stomach1","1" };
cvar_t	sk_player_stomach2	= { "sk_player_stomach2","1" };
cvar_t	sk_player_stomach3	= { "sk_player_stomach3","1" };

cvar_t	sk_player_arm1	= { "sk_player_arm1","1" };
cvar_t	sk_player_arm2	= { "sk_player_arm2","1" };
cvar_t	sk_player_arm3	= { "sk_player_arm3","1" };

cvar_t	sk_player_leg1	= { "sk_player_leg1","1" };
cvar_t	sk_player_leg2	= { "sk_player_leg2","1" };
cvar_t	sk_player_leg3	= { "sk_player_leg3","1" };


//MODDD - NEW BELOW!
///////////////////////////////////////////////
cvar_t	player_ammomax_9mm = { "player_ammomax_9mm", "1" };
cvar_t	player_ammomax_mp5_grenade = { "player_ammomax_mp5_grenade", "1" };
cvar_t	player_ammomax_revolver = { "player_ammomax_revolver", "1" };
cvar_t	player_ammomax_shotgun = { "player_ammomax_shotgun", "1" };
cvar_t	player_ammomax_crossbow = { "player_ammomax_crossbow", "1" };
cvar_t	player_ammomax_rpg = { "player_ammomax_rpg", "1" };
cvar_t	player_ammomax_uranium = { "player_ammomax_uranium", "1" };
cvar_t	player_ammomax_handgrenade = { "player_ammomax_handgrenade", "1" };
cvar_t	player_ammomax_satchel = { "player_ammomax_satchel", "1" };
cvar_t	player_ammomax_tripmine = { "player_ammomax_tripmine", "1" };
cvar_t	player_ammomax_hornet = { "player_ammomax_hornet", "1" };
cvar_t	player_ammomax_snark = { "player_ammomax_snark", "1" };
cvar_t	player_ammomax_chumtoad = { "player_ammomax_chumtoad", "1" };


cvar_t	player_revive_health1 = {"sk_player_revive_health1", "1"};
cvar_t	player_revive_health2 = {"sk_player_revive_health2", "1"};
cvar_t	player_revive_health3 = {"sk_player_revive_health3", "1"};


cvar_t	scientist_can_heal1 = { "sk_scientist_can_heal1", "1" };
cvar_t	scientist_can_heal2 = { "sk_scientist_can_heal2", "1" };
cvar_t	scientist_can_heal3 = { "sk_scientist_can_heal3", "1" };

cvar_t	npc_drop_weapon1 = { "sk_npc_drop_weapon1", "1" };
cvar_t	npc_drop_weapon2 = { "sk_npc_drop_weapon2", "1" };
cvar_t	npc_drop_weapon3 = { "sk_npc_drop_weapon3", "1" };


cvar_t  tdmg_buddha1 = { "tdmg_buddha1", "1" };
cvar_t  tdmg_buddha2 = { "tdmg_buddha2", "1" };
cvar_t  tdmg_buddha3 = { "tdmg_buddha3", "1" };
cvar_t  tdmg_playerbuddha1 = { "tdmg_playerbuddha1", "1" };
cvar_t  tdmg_playerbuddha2 = { "tdmg_playerbuddha2", "1" };
cvar_t  tdmg_playerbuddha3 = { "tdmg_playerbuddha3", "1" };


cvar_t	tdmg_paralyze_duration1 = { "tdmg_paralyze_duration1", "1" };
cvar_t	tdmg_paralyze_duration2 = { "tdmg_paralyze_duration2", "1" };
cvar_t	tdmg_paralyze_duration3 = { "tdmg_paralyze_duration3", "1" };

cvar_t	tdmg_nervegas_duration1 = { "tdmg_nervegas_duration1", "1" };
cvar_t	tdmg_nervegas_duration2 = { "tdmg_nervegas_duration2", "1" };
cvar_t	tdmg_nervegas_duration3 = { "tdmg_nervegas_duration3", "1" };
cvar_t	tdmg_nervegas_damage1 = { "tdmg_nervegas_damage1", "1" };
cvar_t	tdmg_nervegas_damage2 = { "tdmg_nervegas_damage2", "1" };
cvar_t	tdmg_nervegas_damage3 = { "tdmg_nervegas_damage3", "1" };

cvar_t	tdmg_poison_duration1 = { "tdmg_poison_duration1", "1" };
cvar_t	tdmg_poison_duration2 = { "tdmg_poison_duration2", "1" };
cvar_t	tdmg_poison_duration3 = { "tdmg_poison_duration3", "1" };
cvar_t	tdmg_poison_damage1 = { "tdmg_poison_damage1", "1" };
cvar_t	tdmg_poison_damage2 = { "tdmg_poison_damage2", "1" };
cvar_t	tdmg_poison_damage3 = { "tdmg_poison_damage3", "1" };

cvar_t	tdmg_radiation_duration1 = { "tdmg_radiation_duration1", "1" };
cvar_t	tdmg_radiation_duration2 = { "tdmg_radiation_duration2", "1" };
cvar_t	tdmg_radiation_duration3 = { "tdmg_radiation_duration3", "1" };
cvar_t	tdmg_radiation_damage1 = { "tdmg_radiation_damage1", "1" };
cvar_t	tdmg_radiation_damage2 = { "tdmg_radiation_damage2", "1" };
cvar_t	tdmg_radiation_damage3 = { "tdmg_radiation_damage3", "1" };

cvar_t	tdmg_acid_duration1 = { "tdmg_acid_duration1", "1" };
cvar_t	tdmg_acid_duration2 = { "tdmg_acid_duration2", "1" };
cvar_t	tdmg_acid_duration3 = { "tdmg_acid_duration3", "1" };
cvar_t	tdmg_acid_damage1 = { "tdmg_acid_damage1", "1" };
cvar_t	tdmg_acid_damage2 = { "tdmg_acid_damage2", "1" };
cvar_t	tdmg_acid_damage3 = { "tdmg_acid_damage3", "1" };

cvar_t	tdmg_slowburn_duration1 = { "tdmg_slowburn_duration1", "1" };
cvar_t	tdmg_slowburn_duration2 = { "tdmg_slowburn_duration2", "1" };
cvar_t	tdmg_slowburn_duration3 = { "tdmg_slowburn_duration3", "1" };
cvar_t	tdmg_slowburn_damage1 = { "tdmg_slowburn_damage1", "1" };
cvar_t	tdmg_slowburn_damage2 = { "tdmg_slowburn_damage2", "1" };
cvar_t	tdmg_slowburn_damage3 = { "tdmg_slowburn_damage3", "1" };

cvar_t	tdmg_slowfreeze_duration1 = { "tdmg_slowfreeze_duration1", "1" };
cvar_t	tdmg_slowfreeze_duration2 = { "tdmg_slowfreeze_duration2", "1" };
cvar_t	tdmg_slowfreeze_duration3 = { "tdmg_slowfreeze_duration3", "1" };
cvar_t	tdmg_slowfreeze_damage1 = { "tdmg_slowfreeze_damage1", "1" };
cvar_t	tdmg_slowfreeze_damage2 = { "tdmg_slowfreeze_damage2", "1" };
cvar_t	tdmg_slowfreeze_damage3 = { "tdmg_slowfreeze_damage3", "1" };

cvar_t	tdmg_bleeding_duration1 = { "tdmg_bleeding_duration1", "1" };
cvar_t	tdmg_bleeding_duration2 = { "tdmg_bleeding_duration2", "1" };
cvar_t	tdmg_bleeding_duration3 = { "tdmg_bleeding_duration3", "1" };
cvar_t	tdmg_bleeding_damage1 = { "tdmg_bleeding_damage1", "1" };
cvar_t	tdmg_bleeding_damage2 = { "tdmg_bleeding_damage2", "1" };
cvar_t	tdmg_bleeding_damage3 = { "tdmg_bleeding_damage3", "1" };







// END Cvars 



//cvar_t tempVar = {"testvarrr", "24", 0};


extern int gmsgUpdateClientCVar;
extern int gmsgUpdateClientCVarNoSave;



/*
#define TEST_METHOD(onething, otherthing)\
	tempVar.string = otherthing;
	*/


// Register your console variables here
// This gets called one time when the game is initialied
void GameDLLInit( void )
{
	easyForcePrintLine("!!!!!! GameDLLInit");
	
	// well gee, I think so
	g_gameLoaded = TRUE;

	InitShared();
	initOldDebugStuff();
	


	//MODDD - new
	//if(sv_cheatsRefClient == 0){
	//	cvar_sv_cheats = CVAR_GET_POINTER( "sv_cheats" );
	//}

/*
	//get the game's version from  version.h...
	
	char aryChr[128];
	char aryChrD[128];
	writeVersionInfo(aryChr, 128);
	writeDateInfo(aryChrD, 128);

	//CVAR_SET_STRING("sl_mod_version", aryChr);
	//CVAR_SET_STRING("sl_mod_date", aryChrD);
	
	
	//...replaced by a console command now, disregard this.
	sv_mod_version->value_string = aryChr;
	sv_mod_date->value_string = aryChrD;
	CVAR_CREATE(&sv_mod_version);
	CVAR_CREATE(&sv_mod_date);
*/

	
	
	//MODDD - CRITICAL. Moved from player.cpp
	easyForcePrintLine("LINKING USER MESSAGES...");
	// Make sure any necessary user messages have been registered
	LinkUserMessages();




	if (IS_DEDICATED_SERVER()) {
		// just a test
		CVAR_REGISTER(&pregame_server_cvar);

		EASY_CVAR_CREATE_SERVER_MASS
	}








	determineHiddenMemPath();

	globalPSEUDO_iCanHazMemez = checkSubDirectoryExistence("sound\\memez");
	if (EASY_CVAR_GET_DEBUGONLY(hiddenMemPrintout) == 1) {
		easyForcePrintLine("MEMEZ FOUND??: %d", (globalPSEUDO_iCanHazMemez == 1));
	}

	loadHiddenCVars();



	// NOTE!  Do this to avoid changing up other CVars just from noticing a discrepency in cl_bullsquidspit choice
	// since changing the game.  Maybe someone wanted other specifics controlled by cl_bullsquidspit to stay
	// the way they are.
	globalPSEUDO_cl_bullsquidspit = EASY_CVAR_GET(cl_bullsquidspit);
	globalPSEUDO_cl_hornettrail = EASY_CVAR_GET(cl_hornettrail);
	globalPSEUDO_cl_hornetspiral = EASY_CVAR_GET(cl_hornetspiral);
	
	globalPSEUDO_germanCensorshipMem = EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(sv_germancensorship);
	globalPSEUDO_allowGermanModelsMem = EASY_CVAR_GET_DEBUGONLY(allowGermanModels);
	forceWorldLightOffMem = EASY_CVAR_GET_DEBUGONLY(forceWorldLightOff);


	// Once to go as early as possible
	// WARNING!!! This early is unacceptable, there's some spam about "PF_MessageEnd_I:  Unknown User Msg 119"
	// seen on 'developer 2', if updateCVarRefs() is done this early.  Seems to be from the 
	// gmsgUpdateClientCVar messages (although any message sent this early is problematic).
	updateCVarRefs(TRUE);
	
	
	
	
	


	//MODDD - Deemed best place for initializing CVars (first update, necessary for spawn-script so that it may see CVars in their intended form).

	//TODO - make me a EASY_CVAR define.
	// why are we setting this to itself again?  Probably unnecessary later.
	if (EASY_CVAR_GET(soundSentenceSave) == -1) {

		//global_soundSentenceSave = (int)EASY_CVAR_GET(soundSentenceSave);
		EASY_CVAR_SET(soundSentenceSave, EASY_CVAR_GET(soundSentenceSave));

	}
	//MODDD -.
	if (EASY_CVAR_GET_DEBUGONLY(emergencyFix) == 1) {
		//MODDD - TODO - is it a good idea to EASY_CVAR_SET_DEBUGONLY this??
		//global_emergencyFix = 0;
		//"gets the point across"
		EASY_CVAR_SET_DEBUGONLY(emergencyFix, 0);
		resetModCVars(NULL, TRUE);
	}






	// Register cvars here:

	g_psv_gravity = CVAR_GET_POINTER( "sv_gravity" );
	g_psv_aim = CVAR_GET_POINTER( "sv_aim" );
	g_footsteps = CVAR_GET_POINTER( "mp_footsteps" );

	//MODDD - new CVars here... these have been moved though.
	//CVAR_REGISTER(&giveDistOffset);
	//CVAR_REGISTER(&giveLookVert);
	//and here.

	
	test_cvar_create();
	


	
	//is that ok to blank it?


	///tempVar.name = "testvarrr";
	//tempVar.string = "18";
	//CVAR_REGISTER(&tempVar);



	//////////////////////////////////////////////////////////


	CVAR_REGISTER (&displaysoundlist);

	CVAR_REGISTER (&teamplay);
	CVAR_REGISTER (&fraglimit);
	CVAR_REGISTER (&timelimit);

	CVAR_REGISTER (&fragsleft);
	CVAR_REGISTER (&timeleft);

	CVAR_REGISTER (&friendlyfire);
	CVAR_REGISTER (&falldamage);
	CVAR_REGISTER (&weaponstay);
	CVAR_REGISTER (&forcerespawn);
	CVAR_REGISTER (&flashlight);
	CVAR_REGISTER (&aimcrosshair);
	CVAR_REGISTER (&decalfrequency);
	CVAR_REGISTER (&teamlist);
	CVAR_REGISTER (&teamoverride);
	CVAR_REGISTER (&defaultteam);
	CVAR_REGISTER (&allowmonsters);

	CVAR_REGISTER (&mp_chattime);



// REGISTER CVARS FOR SKILL LEVEL STUFF
	// Agrunt
	CVAR_REGISTER ( &sk_agrunt_health1 );// {"sk_agrunt_health1","0"};
	CVAR_REGISTER ( &sk_agrunt_health2 );// {"sk_agrunt_health2","0"};
	CVAR_REGISTER ( &sk_agrunt_health3 );// {"sk_agrunt_health3","0"};

	CVAR_REGISTER ( &sk_agrunt_dmg_punch1 );// {"sk_agrunt_dmg_punch1","0"};
	CVAR_REGISTER ( &sk_agrunt_dmg_punch2 );// {"sk_agrunt_dmg_punch2","0"};
	CVAR_REGISTER ( &sk_agrunt_dmg_punch3 );// {"sk_agrunt_dmg_punch3","0"};

	// Apache
	CVAR_REGISTER ( &sk_apache_health1 );// {"sk_apache_health1","0"};
	CVAR_REGISTER ( &sk_apache_health2 );// {"sk_apache_health2","0"};
	CVAR_REGISTER ( &sk_apache_health3 );// {"sk_apache_health3","0"};

	// MODDD - Barnacle
	CVAR_REGISTER ( &sk_barnacle_health1 );
	CVAR_REGISTER ( &sk_barnacle_health2 );
	CVAR_REGISTER ( &sk_barnacle_health3 );


	// Barney
	CVAR_REGISTER ( &sk_barney_health1 );// {"sk_barney_health1","0"};
	CVAR_REGISTER ( &sk_barney_health2 );// {"sk_barney_health2","0"};
	CVAR_REGISTER ( &sk_barney_health3 );// {"sk_barney_health3","0"};

	// Bullsquid
	CVAR_REGISTER ( &sk_bullsquid_health1 );// {"sk_bullsquid_health1","0"};
	CVAR_REGISTER ( &sk_bullsquid_health2 );// {"sk_bullsquid_health2","0"};
	CVAR_REGISTER ( &sk_bullsquid_health3 );// {"sk_bullsquid_health3","0"};

	CVAR_REGISTER ( &sk_bullsquid_dmg_bite1 );// {"sk_bullsquid_dmg_bite1","0"};
	CVAR_REGISTER ( &sk_bullsquid_dmg_bite2 );// {"sk_bullsquid_dmg_bite2","0"};
	CVAR_REGISTER ( &sk_bullsquid_dmg_bite3 );// {"sk_bullsquid_dmg_bite3","0"};

	CVAR_REGISTER ( &sk_bullsquid_dmg_whip1 );// {"sk_bullsquid_dmg_whip1","0"};
	CVAR_REGISTER ( &sk_bullsquid_dmg_whip2 );// {"sk_bullsquid_dmg_whip2","0"};
	CVAR_REGISTER ( &sk_bullsquid_dmg_whip3 );// {"sk_bullsquid_dmg_whip3","0"};

	CVAR_REGISTER ( &sk_bullsquid_dmg_spit1 );// {"sk_bullsquid_dmg_spit1","0"};
	CVAR_REGISTER ( &sk_bullsquid_dmg_spit2 );// {"sk_bullsquid_dmg_spit2","0"};
	CVAR_REGISTER ( &sk_bullsquid_dmg_spit3 );// {"sk_bullsquid_dmg_spit3","0"};


	CVAR_REGISTER ( &sk_bigmomma_health_factor1 );// {"sk_bigmomma_health_factor1","1.0"};
	CVAR_REGISTER ( &sk_bigmomma_health_factor2 );// {"sk_bigmomma_health_factor2","1.0"};
	CVAR_REGISTER ( &sk_bigmomma_health_factor3 );// {"sk_bigmomma_health_factor3","1.0"};

	CVAR_REGISTER ( &sk_bigmomma_dmg_slash1 );// {"sk_bigmomma_dmg_slash1","50"};
	CVAR_REGISTER ( &sk_bigmomma_dmg_slash2 );// {"sk_bigmomma_dmg_slash2","50"};
	CVAR_REGISTER ( &sk_bigmomma_dmg_slash3 );// {"sk_bigmomma_dmg_slash3","50"};

	CVAR_REGISTER ( &sk_bigmomma_dmg_blast1 );// {"sk_bigmomma_dmg_blast1","100"};
	CVAR_REGISTER ( &sk_bigmomma_dmg_blast2 );// {"sk_bigmomma_dmg_blast2","100"};
	CVAR_REGISTER ( &sk_bigmomma_dmg_blast3 );// {"sk_bigmomma_dmg_blast3","100"};

	CVAR_REGISTER ( &sk_bigmomma_radius_blast1 );// {"sk_bigmomma_radius_blast1","250"};
	CVAR_REGISTER ( &sk_bigmomma_radius_blast2 );// {"sk_bigmomma_radius_blast2","250"};
	CVAR_REGISTER ( &sk_bigmomma_radius_blast3 );// {"sk_bigmomma_radius_blast3","250"};

	// Gargantua
	CVAR_REGISTER ( &sk_gargantua_health1 );// {"sk_gargantua_health1","0"};
	CVAR_REGISTER ( &sk_gargantua_health2 );// {"sk_gargantua_health2","0"};
	CVAR_REGISTER ( &sk_gargantua_health3 );// {"sk_gargantua_health3","0"};

	CVAR_REGISTER ( &sk_gargantua_dmg_slash1 );// {"sk_gargantua_dmg_slash1","0"};
	CVAR_REGISTER ( &sk_gargantua_dmg_slash2 );// {"sk_gargantua_dmg_slash2","0"};
	CVAR_REGISTER ( &sk_gargantua_dmg_slash3 );// {"sk_gargantua_dmg_slash3","0"};

	CVAR_REGISTER ( &sk_gargantua_dmg_fire1 );// {"sk_gargantua_dmg_fire1","0"};
	CVAR_REGISTER ( &sk_gargantua_dmg_fire2 );// {"sk_gargantua_dmg_fire2","0"};
	CVAR_REGISTER ( &sk_gargantua_dmg_fire3 );// {"sk_gargantua_dmg_fire3","0"};

	CVAR_REGISTER ( &sk_gargantua_dmg_stomp1 );// {"sk_gargantua_dmg_stomp1","0"};
	CVAR_REGISTER ( &sk_gargantua_dmg_stomp2 );// {"sk_gargantua_dmg_stomp2","0"};
	CVAR_REGISTER ( &sk_gargantua_dmg_stomp3	);// {"sk_gargantua_dmg_stomp3","0"};


	// Hassassin
	CVAR_REGISTER ( &sk_hassassin_health1 );// {"sk_hassassin_health1","0"};
	CVAR_REGISTER ( &sk_hassassin_health2 );// {"sk_hassassin_health2","0"};
	CVAR_REGISTER ( &sk_hassassin_health3 );// {"sk_hassassin_health3","0"};
	//MODDD - crossbow for the hassassin
	CVAR_REGISTER ( &sk_hassassin_xbow_cl1 );
	CVAR_REGISTER ( &sk_hassassin_xbow_cl2 );
	CVAR_REGISTER ( &sk_hassassin_xbow_cl3 );
	CVAR_REGISTER ( &sk_hassassin_xbow_mo1 );
	CVAR_REGISTER ( &sk_hassassin_xbow_mo2 );
	CVAR_REGISTER ( &sk_hassassin_xbow_mo3 );








	// Headcrab
	CVAR_REGISTER ( &sk_headcrab_health1 );// {"sk_headcrab_health1","0"};
	CVAR_REGISTER ( &sk_headcrab_health2 );// {"sk_headcrab_health2","0"};
	CVAR_REGISTER ( &sk_headcrab_health3 );// {"sk_headcrab_health3","0"};

	CVAR_REGISTER ( &sk_headcrab_dmg_bite1 );// {"sk_headcrab_dmg_bite1","0"};
	CVAR_REGISTER ( &sk_headcrab_dmg_bite2 );// {"sk_headcrab_dmg_bite2","0"};
	CVAR_REGISTER ( &sk_headcrab_dmg_bite3 );// {"sk_headcrab_dmg_bite3","0"};


	// Hgrunt 
	CVAR_REGISTER ( &sk_hgrunt_health1 );// {"sk_hgrunt_health1","0"};
	CVAR_REGISTER ( &sk_hgrunt_health2 );// {"sk_hgrunt_health2","0"};
	CVAR_REGISTER ( &sk_hgrunt_health3 );// {"sk_hgrunt_health3","0"};

	CVAR_REGISTER ( &sk_hgrunt_kick1 );// {"sk_hgrunt_kick1","0"};
	CVAR_REGISTER ( &sk_hgrunt_kick2 );// {"sk_hgrunt_kick2","0"};
	CVAR_REGISTER ( &sk_hgrunt_kick3 );// {"sk_hgrunt_kick3","0"};

	CVAR_REGISTER ( &sk_hgrunt_pellets1 );
	CVAR_REGISTER ( &sk_hgrunt_pellets2 );
	CVAR_REGISTER ( &sk_hgrunt_pellets3 );

	CVAR_REGISTER ( &sk_hgrunt_gspeed1 );
	CVAR_REGISTER ( &sk_hgrunt_gspeed2 );
	CVAR_REGISTER ( &sk_hgrunt_gspeed3 );

	//MODDD - hassault
	CVAR_REGISTER ( &sk_hassault_health1 );
	CVAR_REGISTER ( &sk_hassault_health2 );
	CVAR_REGISTER ( &sk_hassault_health3 );

	CVAR_REGISTER ( &sk_hassault_dmg_melee1 );
	CVAR_REGISTER ( &sk_hassault_dmg_melee2 );
	CVAR_REGISTER ( &sk_hassault_dmg_melee3 );



	// Houndeye
	CVAR_REGISTER ( &sk_houndeye_health1 );// {"sk_houndeye_health1","0"};
	CVAR_REGISTER ( &sk_houndeye_health2 );// {"sk_houndeye_health2","0"};
	CVAR_REGISTER ( &sk_houndeye_health3 );// {"sk_houndeye_health3","0"};

	CVAR_REGISTER ( &sk_houndeye_dmg_blast1 );// {"sk_houndeye_dmg_blast1","0"};
	CVAR_REGISTER ( &sk_houndeye_dmg_blast2 );// {"sk_houndeye_dmg_blast2","0"};
	CVAR_REGISTER ( &sk_houndeye_dmg_blast3 );// {"sk_houndeye_dmg_blast3","0"};


	// ISlave
	CVAR_REGISTER ( &sk_islave_health1 );// {"sk_islave_health1","0"};
	CVAR_REGISTER ( &sk_islave_health2 );// {"sk_islave_health2","0"};
	CVAR_REGISTER ( &sk_islave_health3 );// {"sk_islave_health3","0"};

	CVAR_REGISTER ( &sk_islave_dmg_claw1 );// {"sk_islave_dmg_claw1","0"};
	CVAR_REGISTER ( &sk_islave_dmg_claw2 );// {"sk_islave_dmg_claw2","0"};
	CVAR_REGISTER ( &sk_islave_dmg_claw3 );// {"sk_islave_dmg_claw3","0"};

	CVAR_REGISTER ( &sk_islave_dmg_clawrake1	);// {"sk_islave_dmg_clawrake1","0"};
	CVAR_REGISTER ( &sk_islave_dmg_clawrake2	);// {"sk_islave_dmg_clawrake2","0"};
	CVAR_REGISTER ( &sk_islave_dmg_clawrake3	);// {"sk_islave_dmg_clawrake3","0"};
		
	CVAR_REGISTER ( &sk_islave_dmg_zap1 );// {"sk_islave_dmg_zap1","0"};
	CVAR_REGISTER ( &sk_islave_dmg_zap2 );// {"sk_islave_dmg_zap2","0"};
	CVAR_REGISTER ( &sk_islave_dmg_zap3 );// {"sk_islave_dmg_zap3","0"};


	// Icthyosaur
	CVAR_REGISTER ( &sk_ichthyosaur_health1	);// {"sk_ichthyosaur_health1","0"};
	CVAR_REGISTER ( &sk_ichthyosaur_health2	);// {"sk_ichthyosaur_health2","0"};
	CVAR_REGISTER ( &sk_ichthyosaur_health3	);// {"sk_ichthyosaur_health3","0"};

	CVAR_REGISTER ( &sk_ichthyosaur_shake1	);// {"sk_ichthyosaur_health3","0"};
	CVAR_REGISTER ( &sk_ichthyosaur_shake2	);// {"sk_ichthyosaur_health3","0"};
	CVAR_REGISTER ( &sk_ichthyosaur_shake3	);// {"sk_ichthyosaur_health3","0"};



	// Leech
	CVAR_REGISTER ( &sk_leech_health1 );// {"sk_leech_health1","0"};
	CVAR_REGISTER ( &sk_leech_health2 );// {"sk_leech_health2","0"};
	CVAR_REGISTER ( &sk_leech_health3 );// {"sk_leech_health3","0"};

	CVAR_REGISTER ( &sk_leech_dmg_bite1 );// {"sk_leech_dmg_bite1","0"};
	CVAR_REGISTER ( &sk_leech_dmg_bite2 );// {"sk_leech_dmg_bite2","0"};
	CVAR_REGISTER ( &sk_leech_dmg_bite3 );// {"sk_leech_dmg_bite3","0"};


	//MODDD - new
	// StukaBat
	CVAR_REGISTER ( &sk_stukabat_health1 );
	CVAR_REGISTER ( &sk_stukabat_health2 );
	CVAR_REGISTER ( &sk_stukabat_health3 );
	CVAR_REGISTER ( &sk_stukabat_dmgclaw1 );
	CVAR_REGISTER ( &sk_stukabat_dmgclaw2 );
	CVAR_REGISTER ( &sk_stukabat_dmgclaw3 );

	/*
	//MODDD -disabled, re-enable if needed!
	CVAR_REGISTER ( &sk_stukabat_dmgexplosion1 );
	CVAR_REGISTER ( &sk_stukabat_dmgexplosion2 );
	CVAR_REGISTER ( &sk_stukabat_dmgexplosion3 );
	*/
	
	CVAR_REGISTER ( &sk_panthereye_health1 );
	CVAR_REGISTER ( &sk_panthereye_health2 );
	CVAR_REGISTER ( &sk_panthereye_health3 );
	CVAR_REGISTER ( &sk_panthereye_dmgclaw1 );
	CVAR_REGISTER ( &sk_panthereye_dmgclaw2 );
	CVAR_REGISTER ( &sk_panthereye_dmgclaw3 );


	

	// Controller
	CVAR_REGISTER ( &sk_controller_health1 );
	CVAR_REGISTER ( &sk_controller_health2 );
	CVAR_REGISTER ( &sk_controller_health3 );

	CVAR_REGISTER ( &sk_controller_dmgzap1 );
	CVAR_REGISTER ( &sk_controller_dmgzap2 );
	CVAR_REGISTER ( &sk_controller_dmgzap3 );

	CVAR_REGISTER ( &sk_controller_speedball1 );
	CVAR_REGISTER ( &sk_controller_speedball2 );
	CVAR_REGISTER ( &sk_controller_speedball3 );

	CVAR_REGISTER ( &sk_controller_dmgball1 );
	CVAR_REGISTER ( &sk_controller_dmgball2 );
	CVAR_REGISTER ( &sk_controller_dmgball3 );

	// Nihilanth
	CVAR_REGISTER ( &sk_nihilanth_health1 );// {"sk_nihilanth_health1","0"};
	CVAR_REGISTER ( &sk_nihilanth_health2 );// {"sk_nihilanth_health2","0"};
	CVAR_REGISTER ( &sk_nihilanth_health3 );// {"sk_nihilanth_health3","0"};

	CVAR_REGISTER ( &sk_nihilanth_zap1 );
	CVAR_REGISTER ( &sk_nihilanth_zap2 );
	CVAR_REGISTER ( &sk_nihilanth_zap3 );

	// Scientist
	CVAR_REGISTER ( &sk_scientist_health1 );// {"sk_scientist_health1","0"};
	CVAR_REGISTER ( &sk_scientist_health2 );// {"sk_scientist_health2","0"};
	CVAR_REGISTER ( &sk_scientist_health3 );// {"sk_scientist_health3","0"};

	//MODDD - new!
	CVAR_REGISTER ( &sk_scientist_dmg_punch1 );
	CVAR_REGISTER ( &sk_scientist_dmg_punch2 );
	CVAR_REGISTER ( &sk_scientist_dmg_punch3 );
	
	//MODDD - new!
	CVAR_REGISTER ( &sk_chumtoad_health1 );// {"sk_snark_health1","0"};
	CVAR_REGISTER ( &sk_chumtoad_health2 );// {"sk_snark_health2","0"};
	CVAR_REGISTER ( &sk_chumtoad_health3 );// {"sk_snark_health3","0"};

	// Snark
	CVAR_REGISTER ( &sk_snark_health1 );// {"sk_snark_health1","0"};
	CVAR_REGISTER ( &sk_snark_health2 );// {"sk_snark_health2","0"};
	CVAR_REGISTER ( &sk_snark_health3 );// {"sk_snark_health3","0"};

	CVAR_REGISTER ( &sk_snark_dmg_bite1 );// {"sk_snark_dmg_bite1","0"};
	CVAR_REGISTER ( &sk_snark_dmg_bite2 );// {"sk_snark_dmg_bite2","0"};
	CVAR_REGISTER ( &sk_snark_dmg_bite3 );// {"sk_snark_dmg_bite3","0"};

	CVAR_REGISTER ( &sk_snark_dmg_pop1 );// {"sk_snark_dmg_pop1","0"};
	CVAR_REGISTER ( &sk_snark_dmg_pop2 );// {"sk_snark_dmg_pop2","0"};
	CVAR_REGISTER ( &sk_snark_dmg_pop3 );// {"sk_snark_dmg_pop3","0"};



	// Zombie
	CVAR_REGISTER ( &sk_zombie_health1 );// {"sk_zombie_health1","0"};
	CVAR_REGISTER ( &sk_zombie_health2 );// {"sk_zombie_health3","0"};
	CVAR_REGISTER ( &sk_zombie_health3 );// {"sk_zombie_health3","0"};


	CVAR_REGISTER ( &sk_zombie_dmg_one_slash1 );// {"sk_zombie_dmg_one_slash1","0"};
	CVAR_REGISTER ( &sk_zombie_dmg_one_slash2 );// {"sk_zombie_dmg_one_slash2","0"};
	CVAR_REGISTER ( &sk_zombie_dmg_one_slash3 );// {"sk_zombie_dmg_one_slash3","0"};

	CVAR_REGISTER ( &sk_zombie_dmg_both_slash1 );// {"sk_zombie_dmg_both_slash1","0"};
	CVAR_REGISTER ( &sk_zombie_dmg_both_slash2 );// {"sk_zombie_dmg_both_slash2","0"};
	CVAR_REGISTER ( &sk_zombie_dmg_both_slash3 );// {"sk_zombie_dmg_both_slash3","0"};

	//MODDD - zombie bullet resistance
	CVAR_REGISTER(&sk_zombie_bulletresistance1);
	CVAR_REGISTER(&sk_zombie_bulletresistance2);
	CVAR_REGISTER(&sk_zombie_bulletresistance3);
	//MODDD - also
	CVAR_REGISTER(&sk_zombie_bulletpushback1);
	CVAR_REGISTER(&sk_zombie_bulletpushback2);
	CVAR_REGISTER(&sk_zombie_bulletpushback3);





	
	//MODDD - several new NPC's
	//Archer
	CVAR_REGISTER(&sk_archer_health1);
	CVAR_REGISTER(&sk_archer_health2);
	CVAR_REGISTER(&sk_archer_health3);

	//(here comes dat) Boid (oh shit waddup)
	CVAR_REGISTER(&sk_boid_health1);
	CVAR_REGISTER(&sk_boid_health2);
	CVAR_REGISTER(&sk_boid_health3);

	//Floater
	CVAR_REGISTER(&sk_floater_health1);
	CVAR_REGISTER(&sk_floater_health2);
	CVAR_REGISTER(&sk_floater_health3);

	//Friendly
	CVAR_REGISTER(&sk_friendly_health1);
	CVAR_REGISTER(&sk_friendly_health2);
	CVAR_REGISTER(&sk_friendly_health3);

	//Kingpin
	CVAR_REGISTER(&sk_kingpin_health1);
	CVAR_REGISTER(&sk_kingpin_health2);
	CVAR_REGISTER(&sk_kingpin_health3);






	//Turret
	CVAR_REGISTER ( &sk_turret_health1 );// {"sk_turret_health1","0"};
	CVAR_REGISTER ( &sk_turret_health2 );// {"sk_turret_health2","0"};
	CVAR_REGISTER ( &sk_turret_health3 );// {"sk_turret_health3","0"};


	// MiniTurret
	CVAR_REGISTER ( &sk_miniturret_health1 );// {"sk_miniturret_health1","0"};
	CVAR_REGISTER ( &sk_miniturret_health2 );// {"sk_miniturret_health2","0"};
	CVAR_REGISTER ( &sk_miniturret_health3 );// {"sk_miniturret_health3","0"};


	// Sentry Turret
	CVAR_REGISTER ( &sk_sentry_health1 );// {"sk_sentry_health1","0"};
	CVAR_REGISTER ( &sk_sentry_health2 );// {"sk_sentry_health2","0"};
	CVAR_REGISTER ( &sk_sentry_health3 );// {"sk_sentry_health3","0"};


	// PLAYER WEAPONS

	// Crowbar whack
	CVAR_REGISTER ( &sk_plr_crowbar1 );// {"sk_plr_crowbar1","0"};
	CVAR_REGISTER ( &sk_plr_crowbar2 );// {"sk_plr_crowbar2","0"};
	CVAR_REGISTER ( &sk_plr_crowbar3 );// {"sk_plr_crowbar3","0"};

	// Glock Round
	CVAR_REGISTER ( &sk_plr_9mm_bullet1 );// {"sk_plr_9mm_bullet1","0"};
	CVAR_REGISTER ( &sk_plr_9mm_bullet2 );// {"sk_plr_9mm_bullet2","0"};
	CVAR_REGISTER ( &sk_plr_9mm_bullet3 );// {"sk_plr_9mm_bullet3","0"};

	// 357 Round
	CVAR_REGISTER ( &sk_plr_357_bullet1 );// {"sk_plr_357_bullet1","0"};
	CVAR_REGISTER ( &sk_plr_357_bullet2 );// {"sk_plr_357_bullet2","0"};
	CVAR_REGISTER ( &sk_plr_357_bullet3 );// {"sk_plr_357_bullet3","0"};

	// MP5 Round
	CVAR_REGISTER ( &sk_plr_9mmAR_bullet1 );// {"sk_plr_9mmAR_bullet1","0"};
	CVAR_REGISTER ( &sk_plr_9mmAR_bullet2 );// {"sk_plr_9mmAR_bullet2","0"};
	CVAR_REGISTER ( &sk_plr_9mmAR_bullet3 );// {"sk_plr_9mmAR_bullet3","0"};


	// M203 grenade
	CVAR_REGISTER ( &sk_plr_9mmAR_grenade1 );// {"sk_plr_9mmAR_grenade1","0"};
	CVAR_REGISTER ( &sk_plr_9mmAR_grenade2 );// {"sk_plr_9mmAR_grenade2","0"};
	CVAR_REGISTER ( &sk_plr_9mmAR_grenade3 );// {"sk_plr_9mmAR_grenade3","0"};


	// Shotgun buckshot
	CVAR_REGISTER ( &sk_plr_buckshot1 );// {"sk_plr_buckshot1","0"};
	CVAR_REGISTER ( &sk_plr_buckshot2 );// {"sk_plr_buckshot2","0"};
	CVAR_REGISTER ( &sk_plr_buckshot3 );// {"sk_plr_buckshot3","0"};


	// Crossbow
	CVAR_REGISTER ( &sk_plr_xbow_bolt_monster1 );// {"sk_plr_xbow_bolt1","0"};
	CVAR_REGISTER ( &sk_plr_xbow_bolt_monster2 );// {"sk_plr_xbow_bolt2","0"};
	CVAR_REGISTER ( &sk_plr_xbow_bolt_monster3 );// {"sk_plr_xbow_bolt3","0"};

	CVAR_REGISTER ( &sk_plr_xbow_bolt_client1 );// {"sk_plr_xbow_bolt1","0"};
	CVAR_REGISTER ( &sk_plr_xbow_bolt_client2 );// {"sk_plr_xbow_bolt2","0"};
	CVAR_REGISTER ( &sk_plr_xbow_bolt_client3 );// {"sk_plr_xbow_bolt3","0"};


	// RPG
	CVAR_REGISTER ( &sk_plr_rpg1 );// {"sk_plr_rpg1","0"};
	CVAR_REGISTER ( &sk_plr_rpg2 );// {"sk_plr_rpg2","0"};
	CVAR_REGISTER ( &sk_plr_rpg3 );// {"sk_plr_rpg3","0"};


	// Gauss Gun
	CVAR_REGISTER ( &sk_plr_gauss1 );// {"sk_plr_gauss1","0"};
	CVAR_REGISTER ( &sk_plr_gauss2 );// {"sk_plr_gauss2","0"};
	CVAR_REGISTER ( &sk_plr_gauss3 );// {"sk_plr_gauss3","0"};


	// Egon Gun
	CVAR_REGISTER ( &sk_plr_egon_narrow1 );// {"sk_plr_egon_narrow1","0"};
	CVAR_REGISTER ( &sk_plr_egon_narrow2 );// {"sk_plr_egon_narrow2","0"};
	CVAR_REGISTER ( &sk_plr_egon_narrow3 );// {"sk_plr_egon_narrow3","0"};

	CVAR_REGISTER ( &sk_plr_egon_wide1 );// {"sk_plr_egon_wide1","0"};
	CVAR_REGISTER ( &sk_plr_egon_wide2 );// {"sk_plr_egon_wide2","0"};
	CVAR_REGISTER ( &sk_plr_egon_wide3 );// {"sk_plr_egon_wide3","0"};


	// Hand Grendade
	CVAR_REGISTER ( &sk_plr_hand_grenade1 );// {"sk_plr_hand_grenade1","0"};
	CVAR_REGISTER ( &sk_plr_hand_grenade2 );// {"sk_plr_hand_grenade2","0"};
	CVAR_REGISTER ( &sk_plr_hand_grenade3 );// {"sk_plr_hand_grenade3","0"};


	// Satchel Charge
	CVAR_REGISTER ( &sk_plr_satchel1 );// {"sk_plr_satchel1","0"};
	CVAR_REGISTER ( &sk_plr_satchel2 );// {"sk_plr_satchel2","0"};
	CVAR_REGISTER ( &sk_plr_satchel3 );// {"sk_plr_satchel3","0"};


	// Tripmine
	CVAR_REGISTER ( &sk_plr_tripmine1 );// {"sk_plr_tripmine1","0"};
	CVAR_REGISTER ( &sk_plr_tripmine2 );// {"sk_plr_tripmine2","0"};
	CVAR_REGISTER ( &sk_plr_tripmine3 );// {"sk_plr_tripmine3","0"};


	// WORLD WEAPONS
	CVAR_REGISTER ( &sk_12mm_bullet1 );// {"sk_12mm_bullet1","0"};
	CVAR_REGISTER ( &sk_12mm_bullet2 );// {"sk_12mm_bullet2","0"};
	CVAR_REGISTER ( &sk_12mm_bullet3 );// {"sk_12mm_bullet3","0"};

	CVAR_REGISTER ( &sk_9mmAR_bullet1 );// {"sk_9mm_bullet1","0"};
	CVAR_REGISTER ( &sk_9mmAR_bullet2 );// {"sk_9mm_bullet1","0"};
	CVAR_REGISTER ( &sk_9mmAR_bullet3 );// {"sk_9mm_bullet1","0"};

	CVAR_REGISTER ( &sk_9mm_bullet1 );// {"sk_9mm_bullet1","0"};
	CVAR_REGISTER ( &sk_9mm_bullet2 );// {"sk_9mm_bullet2","0"};
	CVAR_REGISTER ( &sk_9mm_bullet3 );// {"sk_9mm_bullet3","0"};


	// HORNET
	CVAR_REGISTER ( &sk_hornet_dmg1 );// {"sk_hornet_dmg1","0"};
	CVAR_REGISTER ( &sk_hornet_dmg2 );// {"sk_hornet_dmg2","0"};
	CVAR_REGISTER ( &sk_hornet_dmg3 );// {"sk_hornet_dmg3","0"};

	//MODDD - hornet health
	CVAR_REGISTER ( &sk_hornet_health1 );
	CVAR_REGISTER ( &sk_hornet_health2 );
	CVAR_REGISTER ( &sk_hornet_health3 );
	
	//MODDD
	CVAR_REGISTER ( &sk_plr_hornet1 );
	CVAR_REGISTER ( &sk_plr_hornet2 );
	CVAR_REGISTER ( &sk_plr_hornet3 );


	// HEALTH/SUIT CHARGE DISTRIBUTION
	CVAR_REGISTER ( &sk_suitcharger1 );
	CVAR_REGISTER ( &sk_suitcharger2 );
	CVAR_REGISTER ( &sk_suitcharger3 );

	CVAR_REGISTER ( &sk_battery1 );
	CVAR_REGISTER ( &sk_battery2 );
	CVAR_REGISTER ( &sk_battery3 );

	CVAR_REGISTER ( &sk_healthcharger1 );
	CVAR_REGISTER ( &sk_healthcharger2 );
	CVAR_REGISTER ( &sk_healthcharger3 );

	CVAR_REGISTER ( &sk_healthkit1 );
	CVAR_REGISTER ( &sk_healthkit2 );
	CVAR_REGISTER ( &sk_healthkit3 );

	CVAR_REGISTER ( &sk_scientist_heal1 );
	CVAR_REGISTER ( &sk_scientist_heal2 );
	CVAR_REGISTER ( &sk_scientist_heal3 );

// monster damage adjusters
	CVAR_REGISTER ( &sk_monster_head1 );
	CVAR_REGISTER ( &sk_monster_head2 );
	CVAR_REGISTER ( &sk_monster_head3 );

	CVAR_REGISTER ( &sk_monster_chest1 );
	CVAR_REGISTER ( &sk_monster_chest2 );
	CVAR_REGISTER ( &sk_monster_chest3 );

	CVAR_REGISTER ( &sk_monster_stomach1 );
	CVAR_REGISTER ( &sk_monster_stomach2 );
	CVAR_REGISTER ( &sk_monster_stomach3 );

	CVAR_REGISTER ( &sk_monster_arm1 );
	CVAR_REGISTER ( &sk_monster_arm2 );
	CVAR_REGISTER ( &sk_monster_arm3 );

	CVAR_REGISTER ( &sk_monster_leg1 );
	CVAR_REGISTER ( &sk_monster_leg2 );
	CVAR_REGISTER ( &sk_monster_leg3 );

// player damage adjusters
	CVAR_REGISTER ( &sk_player_head1 );
	CVAR_REGISTER ( &sk_player_head2 );
	CVAR_REGISTER ( &sk_player_head3 );

	CVAR_REGISTER ( &sk_player_chest1 );
	CVAR_REGISTER ( &sk_player_chest2 );
	CVAR_REGISTER ( &sk_player_chest3 );

	CVAR_REGISTER ( &sk_player_stomach1 );
	CVAR_REGISTER ( &sk_player_stomach2 );
	CVAR_REGISTER ( &sk_player_stomach3 );

	CVAR_REGISTER ( &sk_player_arm1 );
	CVAR_REGISTER ( &sk_player_arm2 );
	CVAR_REGISTER ( &sk_player_arm3 );

	CVAR_REGISTER ( &sk_player_leg1 );
	CVAR_REGISTER ( &sk_player_leg2 );
	CVAR_REGISTER ( &sk_player_leg3 );


	//MODDD - NEW BELOW!
	///////////////////////////////////////////////
	CVAR_REGISTER(&player_ammomax_9mm);
	CVAR_REGISTER(&player_ammomax_mp5_grenade);
	CVAR_REGISTER(&player_ammomax_revolver);
	CVAR_REGISTER(&player_ammomax_shotgun);
	CVAR_REGISTER(&player_ammomax_crossbow);
	CVAR_REGISTER(&player_ammomax_rpg);
	CVAR_REGISTER(&player_ammomax_uranium);
	CVAR_REGISTER(&player_ammomax_handgrenade);
	CVAR_REGISTER(&player_ammomax_satchel);
	CVAR_REGISTER(&player_ammomax_tripmine);
	CVAR_REGISTER(&player_ammomax_hornet);
	CVAR_REGISTER(&player_ammomax_snark);
	CVAR_REGISTER(&player_ammomax_chumtoad);


	CVAR_REGISTER(&player_revive_health1);
	CVAR_REGISTER(&player_revive_health2);
	CVAR_REGISTER(&player_revive_health3);

	CVAR_REGISTER(&scientist_can_heal1);
	CVAR_REGISTER(&scientist_can_heal2);
	CVAR_REGISTER(&scientist_can_heal3);

	CVAR_REGISTER(&npc_drop_weapon1);
	CVAR_REGISTER(&npc_drop_weapon2);
	CVAR_REGISTER(&npc_drop_weapon3);

	
	CVAR_REGISTER(&tdmg_buddha1);
	CVAR_REGISTER(&tdmg_buddha2);
	CVAR_REGISTER(&tdmg_buddha3);
	CVAR_REGISTER(&tdmg_playerbuddha1);
	CVAR_REGISTER(&tdmg_playerbuddha2);
	CVAR_REGISTER(&tdmg_playerbuddha3);

	CVAR_REGISTER(&tdmg_paralyze_duration1);
	CVAR_REGISTER(&tdmg_paralyze_duration2);
	CVAR_REGISTER(&tdmg_paralyze_duration3);

	CVAR_REGISTER(&tdmg_nervegas_duration1);
	CVAR_REGISTER(&tdmg_nervegas_duration2);
	CVAR_REGISTER(&tdmg_nervegas_duration3);
	CVAR_REGISTER(&tdmg_nervegas_damage1);
	CVAR_REGISTER(&tdmg_nervegas_damage2);
	CVAR_REGISTER(&tdmg_nervegas_damage3);

	CVAR_REGISTER(&tdmg_poison_duration1);
	CVAR_REGISTER(&tdmg_poison_duration2);
	CVAR_REGISTER(&tdmg_poison_duration3);
	CVAR_REGISTER(&tdmg_poison_damage1);
	CVAR_REGISTER(&tdmg_poison_damage2);
	CVAR_REGISTER(&tdmg_poison_damage3);

	CVAR_REGISTER(&tdmg_radiation_duration1);
	CVAR_REGISTER(&tdmg_radiation_duration2);
	CVAR_REGISTER(&tdmg_radiation_duration3);
	CVAR_REGISTER(&tdmg_radiation_damage1);
	CVAR_REGISTER(&tdmg_radiation_damage2);
	CVAR_REGISTER(&tdmg_radiation_damage3);

	CVAR_REGISTER(&tdmg_acid_duration1);
	CVAR_REGISTER(&tdmg_acid_duration2);
	CVAR_REGISTER(&tdmg_acid_duration3);
	CVAR_REGISTER(&tdmg_acid_damage1);
	CVAR_REGISTER(&tdmg_acid_damage2);
	CVAR_REGISTER(&tdmg_acid_damage3);

	CVAR_REGISTER(&tdmg_slowburn_duration1);
	CVAR_REGISTER(&tdmg_slowburn_duration2);
	CVAR_REGISTER(&tdmg_slowburn_duration3);
	CVAR_REGISTER(&tdmg_slowburn_damage1);
	CVAR_REGISTER(&tdmg_slowburn_damage2);
	CVAR_REGISTER(&tdmg_slowburn_damage3);

	CVAR_REGISTER(&tdmg_slowfreeze_duration1);
	CVAR_REGISTER(&tdmg_slowfreeze_duration2);
	CVAR_REGISTER(&tdmg_slowfreeze_duration3);
	CVAR_REGISTER(&tdmg_slowfreeze_damage1);
	CVAR_REGISTER(&tdmg_slowfreeze_damage2);
	CVAR_REGISTER(&tdmg_slowfreeze_damage3);

	CVAR_REGISTER(&tdmg_bleeding_duration1);
	CVAR_REGISTER(&tdmg_bleeding_duration2);
	CVAR_REGISTER(&tdmg_bleeding_duration3);
	CVAR_REGISTER(&tdmg_bleeding_damage1);
	CVAR_REGISTER(&tdmg_bleeding_damage2);
	CVAR_REGISTER(&tdmg_bleeding_damage3);


// END REGISTER CVARS FOR SKILL LEVEL STUFF

	// go ahead and apply what's loaded from skill.cfg as soon as possible
	// Wait.  No need, any gamerules constructor already calls 'RefreshSkillData'.
	//queueSkillUpdate = TRUE;

	SERVER_COMMAND( "exec skill.cfg\n" );


}

