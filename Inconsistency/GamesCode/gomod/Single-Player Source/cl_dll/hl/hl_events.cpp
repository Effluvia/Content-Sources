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
#include "../hud.h"
#include "../cl_util.h"
#include "event_api.h"

extern "C"
{
// HLDM
void EV_FireGlock1( struct event_args_s *args  );
void EV_FireGlock2( struct event_args_s *args  );
void EV_FireShotGunSingle( struct event_args_s *args  );
void EV_FireShotGunDouble( struct event_args_s *args  );
void EV_FireMP5( struct event_args_s *args  );
void EV_FireMP52( struct event_args_s *args  );
void EV_FirePython( struct event_args_s *args  );
void EV_FireGauss( struct event_args_s *args  );
void EV_SpinGauss( struct event_args_s *args  );
void EV_Crowbar( struct event_args_s *args );
void EV_FireCrossbow( struct event_args_s *args );
void EV_FireCrossbow2( struct event_args_s *args );
void EV_FireRpg( struct event_args_s *args );
void EV_EgonFire( struct event_args_s *args );
void EV_EgonStop( struct event_args_s *args );
void EV_HornetGunFire( struct event_args_s *args );
void EV_TripmineFire( struct event_args_s *args );
void EV_SnarkFire( struct event_args_s *args );
void EV_RemoveTool( struct event_args_s *args );//REMOVETOOL
void EV_Punch( struct event_args_s *args );//PUNCH
void EV_FireHKMP5( struct event_args_s *args ); //CS MP5 WEAPON
void EV_m3Fire( struct event_args_s *args ); //CS M3 WEAPON
void EV_FireP228( struct event_args_s *args ); //CS P228 WEAPON
void EV_FireGLOCK18( struct event_args_s *args ); //CS GLOCK18 WEAPON
void EV_FireMAC10( struct event_args_s *args ); //CS MAC10 WEAPON
void EV_FireP90( struct event_args_s *args ); //CS P90 WEAPON
void EV_FirePHYSGUN( struct event_args_s *args ); //PHYSGUN WEAPON
void EV_FireAK47( struct event_args_s *args ); //CS AK47 WEAPON
//void EV_Duplicator( struct event_args_s *args );//DUPLICATOR
//void EV_Duplicator2( struct event_args_s *args );//DUPLICATOR
void EV_FireM4A1( struct event_args_s *args ); //CS M4A1 WEAPON
void EV_FireM4A1_UNSIL( struct event_args_s *args ); //CS M4A1 WEAPON
void EV_FireUSP( struct event_args_s *args ); //CS USP WEAPON
void EV_FireUSP_UNSIL( struct event_args_s *args ); //CS USP WEAPON
void EV_FireFIVESEVEN( struct event_args_s *args ); //CS FIVESEVEN WEAPON
void EV_xm1014Fire( struct event_args_s *args ); //CS XM1014 WEAPON
void EV_FireUMP( struct event_args_s *args ); //CS UMP45 WEAPON
void EV_FireDEAGLE( struct event_args_s *args ); //CS DEAGLE WEAPON
void EV_FirePoser( struct event_args_s *args ); //POSERTOOL WEAPON
void EV_FireTMP( struct event_args_s *args ); //CS TMP WEAPON

void EV_TrainPitchAdjust( struct event_args_s *args );
}

/*
======================
Game_HookEvents

Associate script file name with callback functions.  Callback's must be extern "C" so
 the engine doesn't get confused about name mangling stuff.  Note that the format is
 always the same.  Of course, a clever mod team could actually embed parameters, behavior
 into the actual .sc files and create a .sc file parser and hook their functionality through
 that.. i.e., a scripting system.

That was what we were going to do, but we ran out of time...oh well.
======================
*/
void Game_HookEvents( void )
{
	gEngfuncs.pfnHookEvent( "events/glock1.sc",					EV_FireGlock1 );
	gEngfuncs.pfnHookEvent( "events/glock2.sc",					EV_FireGlock2 );
	gEngfuncs.pfnHookEvent( "events/shotgun1.sc",				EV_FireShotGunSingle );
	gEngfuncs.pfnHookEvent( "events/shotgun2.sc",				EV_FireShotGunDouble );
	gEngfuncs.pfnHookEvent( "events/mp5.sc",					EV_FireMP5 );
	gEngfuncs.pfnHookEvent( "events/mp52.sc",					EV_FireMP52 );
	gEngfuncs.pfnHookEvent( "events/python.sc",					EV_FirePython );
	gEngfuncs.pfnHookEvent( "events/gauss.sc",					EV_FireGauss );
	gEngfuncs.pfnHookEvent( "events/gaussspin.sc",				EV_SpinGauss );
	gEngfuncs.pfnHookEvent( "events/train.sc",					EV_TrainPitchAdjust );
	gEngfuncs.pfnHookEvent( "events/crowbar.sc",				EV_Crowbar );
	gEngfuncs.pfnHookEvent( "events/crossbow1.sc",				EV_FireCrossbow );
	gEngfuncs.pfnHookEvent( "events/crossbow2.sc",				EV_FireCrossbow2 );
	gEngfuncs.pfnHookEvent( "events/rpg.sc",					EV_FireRpg );
	gEngfuncs.pfnHookEvent( "events/egon_fire.sc",				EV_EgonFire );
	gEngfuncs.pfnHookEvent( "events/egon_stop.sc",				EV_EgonStop );
	gEngfuncs.pfnHookEvent( "events/firehornet.sc",				EV_HornetGunFire );
	gEngfuncs.pfnHookEvent( "events/tripfire.sc",				EV_TripmineFire );
	gEngfuncs.pfnHookEvent( "events/snarkfire.sc",				EV_SnarkFire );
	gEngfuncs.pfnHookEvent( "events/removetool.sc",				EV_RemoveTool);//REMOVETOOL
	gEngfuncs.pfnHookEvent( "events/punch.sc",					EV_Punch);//PUNCHES
	gEngfuncs.pfnHookEvent( "events/hkmp5.sc",					EV_FireHKMP5 );//CS MP5 WEAPON
	gEngfuncs.pfnHookEvent( "events/m3Fire.sc",					EV_m3Fire );//CS MP5 WEAPON
	gEngfuncs.pfnHookEvent( "events/p228.sc",					EV_FireP228 );//CS P228 WEAPON
	gEngfuncs.pfnHookEvent( "events/glock18.sc",				EV_FireGLOCK18 );//CS GLOCK18 WEAPON
	gEngfuncs.pfnHookEvent( "events/mac10.sc",					EV_FireMAC10 );//CS MAC10 WEAPON
	gEngfuncs.pfnHookEvent( "events/p90.sc",					EV_FireP90 );//CS P90 WEAPON
	gEngfuncs.pfnHookEvent( "events/physgun.sc",				EV_FirePHYSGUN );//PHYSGUN WEAPON
	gEngfuncs.pfnHookEvent( "events/ak47.sc",					EV_FireAK47 );//AK47 WEAPON
//	gEngfuncs.pfnHookEvent( "events/duplicator.sc",				EV_Duplicator);//DUPLICATOR
//	gEngfuncs.pfnHookEvent( "events/duplicator2.sc",			EV_Duplicator2);//DUPLICATOR
	gEngfuncs.pfnHookEvent( "events/m4a1.sc",					EV_FireM4A1 );//CS M4A1 WEAPON
	gEngfuncs.pfnHookEvent( "events/m4a1_unsil.sc",				EV_FireM4A1_UNSIL );//CS M4A1 WEAPON
	gEngfuncs.pfnHookEvent( "events/usp.sc",					EV_FireUSP );//CS USP WEAPON
	gEngfuncs.pfnHookEvent( "events/usp_unsil.sc",				EV_FireUSP_UNSIL );//CS USP WEAPON
	gEngfuncs.pfnHookEvent( "events/fiveseven.sc",				EV_FireFIVESEVEN );//CS FIVESEVEN WEAPON
	gEngfuncs.pfnHookEvent( "events/xm1014Fire.sc",				EV_xm1014Fire);//CS XM1014 WEAPON
	gEngfuncs.pfnHookEvent( "events/ump45.sc",					EV_FireUMP);//CS UMP WEAPON
	gEngfuncs.pfnHookEvent( "events/deagleFire.sc",				EV_FireDEAGLE);//CS DEAGLE WEAPON
	gEngfuncs.pfnHookEvent( "events/posertool.sc",				EV_FirePoser);//POSERTOOL WEAPON
	gEngfuncs.pfnHookEvent( "events/tmp.sc",					EV_FireTMP );//CS TMP WEAPON
}
