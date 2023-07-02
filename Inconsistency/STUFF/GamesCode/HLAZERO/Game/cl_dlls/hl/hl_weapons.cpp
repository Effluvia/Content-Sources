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

#include "hl_weapons.h"

#include "extdll.h"
#include "util.h"
#include "cbase.h"
//MODDDD - interestingly enough...
#include "animating.h"
#include "basetoggle.h"
#include "basebutton.h"
#include "doors.h"  //is that okay?
#include "basemonster.h"
#include "weapons.h"
//MODDD
#include "dlls/weapon/chumtoadweapon.h"
#include "dlls/weapon/crossbow.h"
#include "dlls/weapon/crowbar.h"
#include "dlls/weapon/egon.h"
#include "dlls/weapon/gauss.h"
#include "dlls/weapon/glock.h"
#include "dlls/weapon/handgrenade.h"
#include "dlls/weapon/hornetgun.h"
#include "dlls/weapon/mp5.h"
#include "dlls/weapon/python.h"
#include "dlls/weapon/rpg.h"
#include "dlls/weapon/satchel.h"
#include "dlls/weapon/shotgun.h"
#include "dlls/weapon/squeak.h"
#include "dlls/weapon/tripmine.h"

#include "nodes.h"
#include "player.h"
#include "usercmd.h"
#include "entity_state.h"
#include "demo_api.h"
#include "pm_defs.h"
#include "event_api.h"
#include "r_efx.h"

#include "../hud_iface.h"
#include "../demo.h"



// INCLUDES FROM com_weapons.cpp.
//////////////////////////////////////////////////////
#include "hud.h"
#include "cl_util.h"

//#include "const.h"
//#include "entity_state.h"
//#include "r_efx.h"

//#include "hl/hl_weapons.h"
/////////////////////////////////////////////////////


// VARS FROM com_weapons.cpp
/////////////////////////////////////////////////////
// g_runfuncs is true if this is the first time we've "predicated" a particular movement/firing
//  command.  If it is 1, then we should play events/sounds etc., otherwise, we just will be
//  updating state info, but not firing events
int g_runfuncs = 0;

// During our weapon prediction processing, we'll need to reference some data that is part of
//  the final state passed into the postthink functionality.  We'll set this pointer and then
//  reset it to NULL as appropriate
struct local_state_s *g_finalstate = NULL;

/////////////////////////////////////////////////////



EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(viewModelPrintouts)
//EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(viewModelSyncFixPrintouts)
//EASY_CVAR_EXTERN(cl_holster)
//EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST(wpn_glocksilencer)
EASY_CVAR_EXTERN(pausecorrection1)


BOOL g_cl_HUD_Frame_ran = FALSE;
BOOL g_cl_HUD_UpdateClientData_ran = FALSE;
BOOL g_HUD_Redraw_ran = FALSE;

// go ahead and allow the very first time?
// After this, VidInit in cdll_int.cpp has to set this (map change).
BOOL g_cl_queueSharedPrecache = TRUE;
BOOL g_cl_firstSendoffSinceMapLoad = TRUE;
float g_cl_mapStartTime = -1;




// NOTICE!  Anything mentioning m_flAmmoStartCharge throughout here and client.cpp can be replaced, that var is no longer used.
// In the least, all references to it can be dummied out, fuser3 is unused now then.
// Strangely, clientside (would be in this file) is missing the frame-count-down logic (reduce by some number of milliseconds),
// even though dlls/player.cpp does it (serverside)?   odd.
// It does get relayed here, but still, no counting down between server-sendoff frames?  Other 0-based timers like
// the timeWeaponIdle and nextprimary/secondary attack ones do that.



// Implementations for methods usually found in dlls/weapons.cpp and dlls/player.cpp moved to their own files clientside:
// cl_dlls/hl/cl_weapons.cpp and cl_dlls/hl/cl_player.cpp.
// There are a few CBaseEntity (cbase.cpp) methods here, but not worth making a new file for.   Moved to the top for easy
// access.  Everything else here should be a class-less method.

// This class still has mainly classless methods that are a complement to ones in dlls/client.cpp.  Data is sent from
// the server to here to be unpacked for clientside animations / other temporary logic to keep in-synch.




// BEWARE - gpGlobals is said to be a 'dummy' object clientside!  gpGlobals->time seems to still work though,
// but a lot of times are based on 0 and count down towards that instead for whatever reason clientside.
// gpGlobals->time works because it's set by every HUD_PostRunCmd call.

// !!!!!!!!!!
// Also, CHECK!  What is g_iUser1?  Is that the check for menu/VGUI/console being open while ingame that might block
// clientside weapon think?  Not huge but worth knowing!

// ignored now, consider phasing out.
extern int flag_apply_m_flTimeWeaponIdle;
extern float stored_m_flTimeWeaponIdle;
extern int flag_apply_m_fJustThrown;
extern int stored_m_fJustThrown;

//BOOL g_recentDuckVal = FALSE;
//int recentDuckTime = 0;



//MODDD - important point before
//extern globalvars_t *gpGlobals;

extern int g_iUser1;

// Pool of client side entities/entvars_t
static entvars_t ev[ 32 ];
static int num_ents = 0;



// The entity we'll use to represent the local client
//MODDD - named changed from 'player' to 'localPlayer', losing 'static' to be extern'able from other places.
//static CBasePlayer player;
CBasePlayer localPlayer;

// Local version of game .dll global variables ( time, etc. )
// (gpGlobals->time uses this as a dummy for most fields, but some like gpGlobals->time are still updated clientside
// to keep shared weapon script (glock.cpp, etc.) from breaking)
static globalvars_t Globals; 

static CBasePlayerWeapon* g_pWpns[ 32 ];

float g_flApplyVel = 0.0;
BOOL g_irunninggausspred = FALSE;

vec3_t previousorigin;

// HLDM Weapon placeholder entities.
CGlock g_Glock;
CCrowbar g_Crowbar;
CPython g_Python;
CMP5 g_Mp5;
CCrossbow g_Crossbow;
CShotgun g_Shotgun;
CRpg g_Rpg;
CGauss g_Gauss;
CEgon g_Egon;
CHgun g_HGun;
CHandGrenade g_HandGren;
CSatchel g_Satchel;
CTripmine g_Tripmine;
CSqueak g_Snark;
//MODDD - new
CChumToadWeapon g_ChumToadWeapon;


//clientside only variable.
BOOL reloadBlocker = FALSE;

// no need?
//float flNextAttackChangeMem = 0;


// TEST.
BOOL blockUntilModelChange = FALSE;
int oldModel = -1;
int queuedBlockedModelAnim = -1;

float forgetBlockUntilModelChangeTime = -1;
float resistTime = -1;

float seqPlayDelay = -1;
int seqPlay = 0;
BOOL queuecall_lastinv = FALSE;
int g_currentanim = -1;



float sp_ClientPreviousTime = -1;
int g_cl_frameCount = 0;

extern int g_framesSinceRestore;


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/*
=====================
CBaseEntity::Killed

If weapons code "kills" an entity, just set its effects to EF_NODRAW
=====================
*/
GENERATE_KILLED_IMPLEMENTATION(CBaseEntity)
{
	//easyPrintLine("MESSAGE2");
	pev->effects |= EF_NODRAW;
}



//MODDD - overrides for methods clientside moved to their own "cl_" versions now.



/*
=====================
CBaseEntity::FireBulletsPlayer

Only produces random numbers to match the server ones.
=====================
*/
Vector CBaseEntity::FireBulletsPlayer(ULONG cShots, Vector vecSrc, Vector vecDirShooting, Vector vecSpread, float flDistance, int iBulletType, int iTracerFreq, int iDamage, entvars_t* pevAttacker, int shared_rand)
{
	float x, y, z;

	for (ULONG iShot = 1; iShot <= cShots; iShot++)
	{
		if (pevAttacker == NULL)
		{
			// get circular gaussian spread
			do {
				x = RANDOM_FLOAT(-0.5, 0.5) + RANDOM_FLOAT(-0.5, 0.5);
				y = RANDOM_FLOAT(-0.5, 0.5) + RANDOM_FLOAT(-0.5, 0.5);
				z = x * x + y * y;
			} while (z > 1);
		}
		else
		{
			//Use player's random seed.
			// get circular gaussian spread
			x = UTIL_SharedRandomFloat(shared_rand + iShot, -0.5, 0.5) + UTIL_SharedRandomFloat(shared_rand + (1 + iShot), -0.5, 0.5);
			y = UTIL_SharedRandomFloat(shared_rand + (2 + iShot), -0.5, 0.5) + UTIL_SharedRandomFloat(shared_rand + (3 + iShot), -0.5, 0.5);
			z = x * x + y * y;
		}

	}

	return Vector(x * vecSpread.x, y * vecSpread.y, 0.0);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////












//MODDD
// Looks like this is done to give any calls to 'AlertMessage' something to do for clientside, since the
// normally serverside-only methods aren't available here.  Some are completely dummied, this still does something.
// Still, have the 'easyPrint' system so that's obsolete.
/*
======================
AlertMessage

Print debug messages to console
======================
*/
void AlertMessage( ALERT_TYPE atype, char *szFmt, ... )
{
	va_list argptr;
	static char	string[1024];
	
	va_start (argptr, szFmt);
	vsprintf (string, szFmt,argptr);
	va_end (argptr);

	gEngfuncs.Con_Printf( "cl:  " );
	gEngfuncs.Con_Printf( string );
}

//Returns if it's multiplayer.
//Mostly used by the client side weapons.
//MODDD - obsolete, "IsMultiplayer" used in util_shared.h replaces this and
// can be called in shared code too, still functional.
//bool bIsMultiplayer ( void )
//{
//	return (gEngfuncs.GetMaxClients() == 1) ? FALSE : TRUE;
//}

//Just loads a v_ model.
void LoadVModel ( char *szViewModel, CBasePlayer *m_pPlayer )
{
	gEngfuncs.CL_LoadModel( szViewModel, &m_pPlayer->pev->viewmodel );
}

/*
=====================
HUD_PrepEntity

Links the raw entity to an entvars_s holder.  If a player is passed in as the owner, then
we set up the m_pPlayer field.
=====================
*/
void HUD_PrepEntity( CBaseEntity *pEntity, CBasePlayer *pWeaponOwner )
{
	memset( &ev[ num_ents ], 0, sizeof( entvars_t ) );
	pEntity->pev = &ev[ num_ents++ ];

	pEntity->Precache();
	pEntity->Spawn();

	// Only proceed with all this, if pWeaponOwner isn't NULL.
	// I sure hope it isn't NULL.

	//MODDD - now, only a check for whether this entity is a Player or not.
	// If not a player, we trust it's a weapon and all this makes sense.
	//if ( pWeaponOwner )
	if (!pEntity->IsPlayer())
	{
		CBasePlayerWeapon* tempWeaponRef = (CBasePlayerWeapon*)pEntity;
		tempWeaponRef->m_pPlayer = pWeaponOwner;
		RegisterWeapon(tempWeaponRef, g_pWpns);

		// nope, given by RegisterWeapon now.  In ItemInfoArray so that it's available
		// for serverside weapons which don't use global static copies like clientside deos.
		//tempWeaponRef->m_iPrimaryAmmoType = GetAmmoIndex(szName);

	}//END OF pWeaponOwner check
}//END OF HUD_PrepEntity





/*
=====================
UTIL_TraceLine

Don't actually trace, but act like the trace didn't hit anything.
=====================
*/

//MODDD - why not this too?
void UTIL_TraceLine( const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, IGNORE_GLASS ignoreGlass, edict_t *pentIgnore, TraceResult *ptr )
{
	memset( ptr, 0, sizeof( *ptr ) );
	ptr->flFraction = 1.0;
}
void UTIL_TraceLine( const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, edict_t *pentIgnore, TraceResult *ptr )
{
	memset( ptr, 0, sizeof( *ptr ) );
	ptr->flFraction = 1.0;
}
//MODDD - why not this too?
TraceResult	UTIL_GetGlobalTrace(){
	//MODDD - mimicking how other dummied out versions handle it?
	TraceResult tr;
	// Clean the memory.  yea?
	memset(&tr, 0, sizeof(tr));
	//tr.fInOpen = 1;   // nevermind, not much (nothing in shared script) checks this.
	tr.flFraction		= 1.0;
	return tr;
}

void UTIL_TraceHull( const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, int hullNumber, edict_t *pentIgnore, TraceResult *ptr )
{
	memset( ptr, 0, sizeof( *ptr ) );
	ptr->flFraction = 1.0;
}

//MODDDD NOTE - never used. at all. Does this work?
void UTIL_TraceModel( const Vector &vecStart, const Vector &vecEnd, int hullNumber, edict_t *pentModel, TraceResult *ptr )
{
	memset( ptr, 0, sizeof( *ptr ) );
	ptr->flFraction = 1.0;
}


/*
=====================
UTIL_ParticleBox

For debugging, draw a box around a player made out of particles
=====================
*/
void UTIL_ParticleBox( CBasePlayer *pPlayer, float *mins, float *maxs, float life, unsigned char r, unsigned char g, unsigned char b )
{
	int i;
	vec3_t mmin, mmax;

	for ( i = 0; i < 3; i++ )
	{
		mmin[ i ] = pPlayer->pev->origin[ i ] + mins[ i ];
		mmax[ i ] = pPlayer->pev->origin[ i ] + maxs[ i ];
	}

	gEngfuncs.pEfxAPI->R_ParticleBox( (float *)&mmin, (float *)&mmax, 5.0, 0, 255, 0 );
}

/*
=====================
UTIL_ParticleBoxes

For debugging, draw boxes for other collidable players
=====================
*/
void UTIL_ParticleBoxes( void )
{
	int idx;
	physent_t *pe;
	cl_entity_t *pPlayer;
	vec3_t mins, maxs;
	
	gEngfuncs.pEventAPI->EV_SetUpPlayerPrediction( false, true );

	// Store off the old count
	gEngfuncs.pEventAPI->EV_PushPMStates();

	pPlayer = gEngfuncs.GetLocalPlayer();
	// Now add in all of the players.
	gEngfuncs.pEventAPI->EV_SetSolidPlayers ( pPlayer->index - 1 );	

	for ( idx = 1; idx < 100; idx++ )
	{
		pe = gEngfuncs.pEventAPI->EV_GetPhysent( idx );
		if ( !pe )
			break;

		if ( pe->info >= 1 && pe->info <= gEngfuncs.GetMaxClients() )
		{
			mins = pe->origin + pe->mins;
			maxs = pe->origin + pe->maxs;

			gEngfuncs.pEfxAPI->R_ParticleBox( (float *)&mins, (float *)&maxs, 0, 0, 255, 2.0 );
		}
	}

	gEngfuncs.pEventAPI->EV_PopPMStates();
}

/*
=====================
UTIL_ParticleLine

For debugging, draw a line made out of particles
=====================
*/
//MODDD - oh.  pPlayer isn't even used in here?
void UTIL_ParticleLine( CBasePlayer * pPlayer, float *start, float *end, float life, unsigned char r, unsigned char g, unsigned char b )
{
	gEngfuncs.pEfxAPI->R_ParticleLine( start, end, r, g, b, life );
}


/*
=====================
HUD_InitClientWeapons

Set up weapons, player and functions needed to run weapons code client-side.
=====================
*/


//#include "r_studioint.h"
//extern engine_studio_api_t IEngineStudio;

//Note - does nothing if already initialized from an earlier call.
void HUD_InitClientWeapons( void )
{
	static int initialized = 0;

	// WAIT!  Go ahead and precache here if the map has changed since (VidInit got called).
	// The rest of this method gets blocked on having already run once.  As-is did it that way for 
	// what used to be here only (weapon-related stuff), keeping that much that way.
	if(g_cl_queueSharedPrecache){
		g_cl_queueSharedPrecache = FALSE;
		// Also set the current time for rendering to know how long it's been since the map has been loaded.
		// There is also IEngineStudio.GetTimes( ... ), but it looks like this gets the exact same time, or 
		// close enough for it to work out anyway.  Unsure if would even perform any better in VidInit.
		g_cl_mapStartTime = gEngfuncs.GetClientTime();

		// Interesting, this actually fails.  IEngineStudio.GetTimes (pointer to a method) is NULL.
		// Which is odd because VidInit happens earlier than HUD_InitClientWeapons, where it isn't NULL.
		// Strange strange strange, not that it needs to be used here.
		/*
		int dummy1;
		double test3;
		double dummy2;
		IEngineStudio.GetTimes(&dummy1, &test3, &dummy2);
		*/
		
		PrecacheShared();
	}

	if (initialized) {
		return;  //right there.
	}

	initialized = 1;


	// Set up pointer ( dummy object )
	gpGlobals = &Globals;

	// Fill in current time ( probably not needed )
	gpGlobals->time = gEngfuncs.GetClientTime();

	//MODDD - is this a good idea?
	// ONLY APPLIES IF THE CLIENTSIDE PAUSE-CORRECTION IS TURNED ON ('pausecorrection1' cvar).
	// This blocks the think-calls in HUD_PostRunCmd very early on for the first frame,
	// since it seems to be called the same time as client initialization
	// (this exact given gpGlobals->time is spotted and so that frame is skipped).
	// Going to err on caution and not do that, left as -1 sp_ClientPreviousTime will
	// let the first frame run like it usually would.
	//if (sp_ClientPreviousTime == -1){sp_ClientPreviousTime = gpGlobals->time;}


	// Fake functions
	g_engfuncs.pfnPrecacheModel		= stub_PrecacheModel;
	g_engfuncs.pfnPrecacheSound		= stub_PrecacheSound;
	g_engfuncs.pfnPrecacheEvent		= stub_PrecacheEvent;
	g_engfuncs.pfnNameForFunction	= stub_NameForFunction;
	g_engfuncs.pfnSetModel			= stub_SetModel;
	g_engfuncs.pfnSetClientMaxspeed = HUD_SetMaxSpeed;

	// Handled locally
	g_engfuncs.pfnPlaybackEvent		= HUD_PlaybackEvent;
	g_engfuncs.pfnAlertMessage		= AlertMessage;

	// Pass through to engine
	g_engfuncs.pfnPrecacheEvent		= gEngfuncs.pfnPrecacheEvent;
	g_engfuncs.pfnRandomFloat		= gEngfuncs.pfnRandomFloat;
	g_engfuncs.pfnRandomLong		= gEngfuncs.pfnRandomLong;



	// MODDD - place for script similar between client and serverside.
	// NO.  Separated out into ClearWeaponInfoCache.
	// For... 'reasons'.  Most precaches done only once ever while the game is booted get glitchy
	// after changing the map but failing to precache again.  That's not what's weird.
	// Thing is, weapons don't need to precache ever again, they didn't in retail.
	// Don't question it, just move on.  PrecacheShared() will be called after VidInit() sets a flag
	// that lets only PrecacheShared() be called on checking HUD_InitClientWeapons again
	// (most is blocked by having ever initialized, even through maps).
	//PrecacheShared();
	ClearWeaponInfoCache();

	// Allocate a slot for the local player
	HUD_PrepEntity( &localPlayer	, NULL );

	// Allocate slot(s) for each weapon that we are going to be predicting
	//MODDD - order changed to reflect the serverside ordering as to not break people's save files,
	// but still keep the ammotype indices in cached arrays consistent between client and serverside.
	// Doesn't really matter if they are different, but it is to make future debugging less of a headache.
	// Maybe.  Doesn't hurt to match, certainly.
	/*
	HUD_PrepEntity( &g_Glock	, &localPlayer );
	HUD_PrepEntity( &g_Crowbar	, &localPlayer );
	HUD_PrepEntity( &g_Python	, &localPlayer );
	HUD_PrepEntity( &g_Mp5	, &localPlayer );
	HUD_PrepEntity( &g_Crossbow	, &localPlayer );
	HUD_PrepEntity( &g_Shotgun	, &localPlayer );
	HUD_PrepEntity( &g_Rpg	, &localPlayer );
	HUD_PrepEntity( &g_Gauss	, &localPlayer );
	HUD_PrepEntity( &g_Egon	, &localPlayer );
	HUD_PrepEntity( &g_HGun	, &localPlayer );
	HUD_PrepEntity( &g_HandGren	, &localPlayer );
	HUD_PrepEntity( &g_Satchel	, &localPlayer );
	HUD_PrepEntity( &g_Tripmine	, &localPlayer );
	HUD_PrepEntity( &g_Snark	, &localPlayer );
	// new
	HUD_PrepEntity( &g_ChumToadWeapon	, &localPlayer );
	*/

	HUD_PrepEntity(&g_Shotgun, &localPlayer);
	HUD_PrepEntity(&g_Crowbar, &localPlayer);
	HUD_PrepEntity(&g_Glock, &localPlayer);
	HUD_PrepEntity(&g_Mp5, &localPlayer);
	HUD_PrepEntity(&g_Python, &localPlayer);
	HUD_PrepEntity(&g_Gauss, &localPlayer);
	HUD_PrepEntity(&g_Rpg, &localPlayer);
	HUD_PrepEntity(&g_Crossbow, &localPlayer);
	HUD_PrepEntity(&g_Egon, &localPlayer);
	HUD_PrepEntity(&g_Tripmine, &localPlayer);
	HUD_PrepEntity(&g_Satchel, &localPlayer);
	HUD_PrepEntity(&g_HandGren, &localPlayer);
	HUD_PrepEntity(&g_Snark, &localPlayer);
	HUD_PrepEntity(&g_ChumToadWeapon, &localPlayer);
	HUD_PrepEntity(&g_HGun, &localPlayer);

	/*
	// summary of serverside precache statements:
	UTIL_PrecacheOtherWeapon("weapon_shotgun");
	UTIL_PrecacheOtherWeapon("weapon_crowbar");
	UTIL_PrecacheOtherWeapon("weapon_9mmhandgun");
	UTIL_PrecacheOtherWeapon("weapon_9mmAR");
	UTIL_PrecacheOtherWeapon("weapon_357");
	UTIL_PrecacheOtherWeapon("weapon_gauss");
	UTIL_PrecacheOtherWeapon("weapon_rpg");
	UTIL_PrecacheOtherWeapon("weapon_crossbow");
	UTIL_PrecacheOtherWeapon("weapon_egon");
	UTIL_PrecacheOtherWeapon("weapon_tripmine");
	UTIL_PrecacheOtherWeapon("weapon_satchel");
	UTIL_PrecacheOtherWeapon("weapon_handgrenade");
	UTIL_PrecacheOtherWeapon("weapon_snark");
	UTIL_PrecacheOtherWeapon("weapon_chumtoad");
	UTIL_PrecacheOtherWeapon("weapon_hornetgun");
	*/
	/*
	readout of ammo-types from serverside printout post weapon registry.
	0
	1  buckshot
	2   9mm
	3   ARgren
	4   357
	5   uranium
	6   rockets
	7   bolts
	8   tripm
	9   satch
	10  handgren
	11  snar
	12  chumtoads
	13   hornets
	*/


	//MODDD - and finally after all weapons have been registered, call this.
	PostWeaponRegistry();

}

/*
=====================
HUD_GetLastOrg

Retruns the last position that we stored for egon beam endpoint.
=====================
*/
void HUD_GetLastOrg( float *org )
{
	int i;
	
	// Return last origin
	for ( i = 0; i < 3; i++ )
	{
		org[i] = previousorigin[i];
	}
}

/*
=====================
HUD_SetLastOrg

Remember our exact predicted origin so we can draw the egon to the right position.
=====================
*/
void HUD_SetLastOrg( void )
{
	int i;
	
	// Offset final origin by view_offset
	for ( i = 0; i < 3; i++ )
	{
		previousorigin[i] = g_finalstate->playerstate.origin[i] + g_finalstate->client.view_ofs[ i ];
	}
}

/*
=====================
HUD_WeaponsPostThink

Run Weapon firing code on client
=====================
*/

void HUD_WeaponsPostThink(local_state_s* from, local_state_s* to, usercmd_t* cmd, double time, unsigned int random_seed)
{

	int i;
	int buttonsChanged;
	CBasePlayerWeapon* pWeapon = NULL;
	CBasePlayerWeapon* pCurrent;
	weapon_data_t nulldata, * pfrom, * pto;
	static int lasthealth;
	// MODDD - new.
	static float nextBlinkLogicFrame = -1;

	memset(&nulldata, 0, sizeof(nulldata));

	//MODDD - why can't this be done at the start of HUD_PostRunCMD, which calls this HUD_WeaponsPostThink method anyways?
	//HUD_InitClientWeapons();

	// Get current clock
	//MODDD - same. Why can't HUD_PostRunCMD do this instead?
	//gpGlobals->time = time;

	// Fill in data based on selected weapon
	// FIXME, make this a method in each weapon?  where you pass in an entity_state_t *?


	//MODDD - number of frames (skin values) for different stages of eye blinking. Just hardcoding since everything else player-model related is.
	//Default of 0, weapons with blinkable eyes should specify this.
	int numberOfEyeSkins = 0;
	//And in a 1 out of X chance every 0.1 seconds (one frame for NPC think logic), the blink will occur.
	int blinkChancePerFrame = 127;

	switch (from->client.m_iId)
	{
	case WEAPON_CROWBAR:
		pWeapon = &g_Crowbar;
		break;

	case WEAPON_GLOCK:
		pWeapon = &g_Glock;
		break;

	case WEAPON_PYTHON:
		pWeapon = &g_Python;
		break;

	case WEAPON_MP5:
		pWeapon = &g_Mp5;
		break;

	case WEAPON_CROSSBOW:
		pWeapon = &g_Crossbow;
		break;

	case WEAPON_SHOTGUN:
		pWeapon = &g_Shotgun;
		break;

	case WEAPON_RPG:
		pWeapon = &g_Rpg;
		break;

	case WEAPON_GAUSS:
		pWeapon = &g_Gauss;
		break;

	case WEAPON_EGON:
		pWeapon = &g_Egon;
		break;

	case WEAPON_HORNETGUN:
		pWeapon = &g_HGun;
		break;

	case WEAPON_HANDGRENADE:
		pWeapon = &g_HandGren;
		break;

	case WEAPON_SATCHEL:
		pWeapon = &g_Satchel;
		break;

	case WEAPON_TRIPMINE:
		pWeapon = &g_Tripmine;
		break;

	case WEAPON_SNARK:
		numberOfEyeSkins = 3;
		blinkChancePerFrame = 33;
		pWeapon = &g_Snark;
		break;
		//MODDD - new
	case WEAPON_CHUMTOAD:
		numberOfEyeSkins = 3;
		blinkChancePerFrame = 50;
		pWeapon = &g_ChumToadWeapon;
		break;

	}//END OF weapon ID link

	// Store pointer to our destination entity_state_t so we can get our origin, etc. from it
	//  for setting up events on the client
	g_finalstate = to;

	// If we are running events/etc. go ahead and see if we
	//  managed to die between last frame and this one
	// If so, run the appropriate player killed or spawn function
	if (g_runfuncs)
	{
		if (to->client.health <= 0 && lasthealth > 0)
		{
			localPlayer.Killed(NULL, 0);

		}
		else if (to->client.health > 0 && lasthealth <= 0)
		{
			localPlayer.Spawn();
		}

		lasthealth = to->client.health;
	}

	// We are not predicting the current weapon, just bow out here.
	if (!pWeapon)
		return;



	// Nope, no need.  The viewmodel can be accessed this same way in studiomodelrenderer.cpp so there is no point in this
	// renderflag telling what it is.
	//gEngfuncs.GetViewModel()->curstate.renderfx |= ISVIEWMODEL;

	//gEngfuncs.GetViewModel()->curstate.renderfx &= ~ANIMATEBACKWARDS;


	/*
	if(localPlayer.forceNoWeaponLoop == TRUE){
		gEngfuncs.GetViewModel()->curstate.renderfx |= FORCE_NOLOOP;
	}else{
		gEngfuncs.GetViewModel()->curstate.renderfx &= ~FORCE_NOLOOP;
	}
	*/


	//unnecessary, if it was here.  Not used in this setting.
	pWeapon->m_chargeReady &= ~32;

	//MODDD
	//"pWeapon" is the currently equipped weapon here... I think.
	if (pWeapon->m_chargeReady & 64) {
		pWeapon->m_chargeReady &= ~64;

		gEngfuncs.GetViewModel()->curstate.renderfx |= FORCE_NOLOOP;
	}
	else {
		//pWeapon->m_chargeReady &= ~64;  //unnecessary?
		gEngfuncs.GetViewModel()->curstate.renderfx &= ~FORCE_NOLOOP;
	}

	//easyPrintLine("LOOOOOOP %d", localPlayer.forceNoWeaponLoop);



	//MODDD - moved from below, see the old ItmePostFrame call location there.
	// Don't go firing anything if we have died.
	// Or if we don't have a weapon model deployed
	//NEW LOCATION...


	BOOL letsNotNameThingsLikeThat = FALSE;

	for (i = 0; i < 32; i++)
	{
		pCurrent = g_pWpns[i];

		if (!pCurrent)
		{
			continue;
		}

		pfrom = &from->weapondata[i];


		pCurrent->m_fInReload = pfrom->m_fInReload;
		pCurrent->m_fInSpecialReload = pfrom->m_fInSpecialReload;
		//		pCurrent->m_flPumpTime			= pfrom->m_flPumpTime;
		pCurrent->m_iClip = pfrom->m_iClip;

		if (pCurrent->m_iId == WEAPON_SHOTGUN) {
			//easyForcePrintLine("UPDATE A: %.2f -> %.2f", pCurrent->m_flNextPrimaryAttack, pfrom->m_flNextPrimaryAttack);
		}

		//If the new value would be less than 0 but our next primary attack is noticably above 0, this is too much chance in one frame. Deny the transfer this time.
		// Anything relying on this needs to be checked.  denying significant differences causes 
		// retail snark throws to fail, even though it's perfectly sound.
		// Although...  they shouldn't have set m_flNextPrimaryAttack as though a snark was thrown just because the serverside
		// traces are dummied (called in PrimaryAttack in snark.cpp;  the ev_hldm ones worked fine).
		// So.   eh.  whichever makes the most sense.
		// YES KEEP THIS.  I stared at this for 5+ hours to say... yes.  This gives better than retail behavior.
		// Without this check, some clientside weapon-fires happen twice, especially with nextAttack's that are uneven
		// (like + 0.82 instead of 0.75 on the singlefire shotgun).  Why?   BECAUSE THE GODS WERE NOT PLEASED WITH 0.82.
		



		if(g_cl_firstSendoffSinceMapLoad == FALSE){
			// normal

			if( !(pfrom->m_flNextPrimaryAttack <= 0 && pCurrent->m_flNextPrimaryAttack >= 0.1) ){
				pCurrent->m_flNextPrimaryAttack = pfrom->m_flNextPrimaryAttack;
			}else{
				int x = 45;
			}
			if( !(pfrom->m_flNextSecondaryAttack <= 0 && pCurrent->m_flNextSecondaryAttack >= 0.1) ){
				pCurrent->m_flNextSecondaryAttack = pfrom->m_flNextSecondaryAttack;
			}
			if( !(pfrom->m_flTimeWeaponIdle <= 0 && pCurrent->m_flTimeWeaponIdle >= 0.1) ){
				pCurrent->m_flTimeWeaponIdle = pfrom->m_flTimeWeaponIdle;
			}else{
				//	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(viewModelSyncFixPrintouts)==1) easyForcePrintLine("*****VIEWMODEL SYNCH FIX APPLIED.");
			}

		}else{

			// Force it, no prevention
			pCurrent->m_flNextPrimaryAttack = pfrom->m_flNextPrimaryAttack;
			pCurrent->m_flNextSecondaryAttack = pfrom->m_flNextSecondaryAttack;
			pCurrent->m_flTimeWeaponIdle = pfrom->m_flTimeWeaponIdle;

		}
		


		pCurrent->pev->fuser1 = pfrom->fuser1;
		pCurrent->m_flStartThrow = pfrom->fuser2;
		pCurrent->m_flReleaseThrow = pfrom->fuser3;



		//NOTICE - despite already existing, "fuser4" seems untouched by the DLL.  Do not rely on it!
		//pCurrent->fuser4				= pfrom->fuser4;
		/*
		pCurrent->fuser5				= pfrom->fuser5;
		pCurrent->fuser6				= pfrom->fuser6;
		pCurrent->fuser7				= pfrom->fuser7;
		pCurrent->fuser8				= pfrom->fuser8;
		*/

		pCurrent->m_chargeReady = pfrom->iuser1;
		pCurrent->m_fInAttack = pfrom->iuser2;
		pCurrent->m_fireState = pfrom->iuser3;


		if (pfrom->iuser1 & 128) {
			// HOLSTERING CHECK
			int x = 45;
		}


		//MODDD - disabled since now, ammotypes are cached client and serverside at startup.
		//pCurrent->m_iSecondaryAmmoType		= (int)from->client.vuser3[ 2 ];
		//pCurrent->m_iPrimaryAmmoType		= (int)from->client.vuser4[ 0 ];

		int myPrimaryAmmoType = pCurrent->getPrimaryAmmoType();
		int mySecondaryAmmoType = pCurrent->getSecondaryAmmoType();


		//MODDD - MAJOR BUG AS OF RETAIL.
		//   m_iPrimaryAmmoType and  m_iSecondaryAmmoType can be -1!  Setting an array at -1 here <interferes with> up unrelated memory.
		//   Like poor poor m_pLastItem, which becomes a pointer to invalid memory but is still tried to be used because it technically
		//   isn't 'NULL' (is really non-zero).
		//   CURSE YOU C/C++ AND YOUR LACK OF BOUNDARY CHECKING!
		//   ...yes I know, It's c++, but it's <definitely> called    \..~$----->>>>debug<<<<-----$~../   mode for a <heckin>' reason


		//MODDD - wait.  We only get ammo for the equippe weapon this way.
		// This is just pasting the ammo value of the currently equiped weapon to all ammo types.
		// Ehhhhhh..?   Adding this if-then.
		if (pCurrent == pWeapon) {
			if (IS_AMMOTYPE_VALID(myPrimaryAmmoType)) {
				localPlayer.m_rgAmmo[myPrimaryAmmoType] = (int)from->client.vuser4[1];
			}
			if (IS_AMMOTYPE_VALID(mySecondaryAmmoType)) {
				localPlayer.m_rgAmmo[mySecondaryAmmoType] = (int)from->client.vuser4[2];
			}
		}


		//This stuff used to be above. Isn't this area better?

		if (pWeapon->m_iId == pCurrent->m_iId) {

			if (pWeapon->m_iId == WEAPON_EGON && pfrom->m_iId == WEAPON_EGON && pCurrent->m_iId == WEAPON_EGON) {
				//easyPrintLine("SYNC TEST EGON: %.8f, %.8f, %0.2f", pCurrent->m_flStartThrow, pfrom->fuser2, gpGlobals->time);
				//easyPrintLine("SYNC TEST EGON 2: %df, %d, %0.2f", pCurrent->m_fInAttack, pfrom->iuser2, gpGlobals->time);
			}
			if ((pWeapon->m_iId == WEAPON_GLOCK && pfrom->m_iId == WEAPON_GLOCK || pCurrent->m_iId == WEAPON_GLOCK) ||
				(pWeapon->m_iId == WEAPON_SHOTGUN && pfrom->m_iId == WEAPON_SHOTGUN || pCurrent->m_iId == WEAPON_SHOTGUN)
				) {
				//easyPrintLine("SYNC TEST GLOCK: %d, %d, %.2f", pCurrent->m_fireState, pfrom->iuser3, gpGlobals->time);

				//pWeapon == &g_Glock

				//m_fireState


				/*
				if(pfrom->iuser3 & 128){
					letsNotNameThingsLikeThat = TRUE;
					//pfrom->iuser3 &= ~128;
					gEngfuncs.GetViewModel()->curstate.renderfx |= ANIMATEBACKWARDS;
				}
				*/
			}
		}
	}//END OF yet another weapons loop.



	// For random weapon events, use this seed to seed random # generator
	localPlayer.random_seed = random_seed;

	// Get old buttons from previous state.
	localPlayer.m_afButtonLast = from->playerstate.oldbuttons;

	// Which buttsons chave changed
	buttonsChanged = (localPlayer.m_afButtonLast ^ cmd->buttons);	// These buttons have changed this frame

	// Debounced button codes for pressed/released
	// The changed ones still down are "pressed"
	localPlayer.m_afButtonPressed = buttonsChanged & cmd->buttons;
	// The ones not down are "released"
	localPlayer.m_afButtonReleased = buttonsChanged & (~cmd->buttons);

	// Set player variables that weapons code might check/alter
	// TAGGG - CRITICAL CRITICAL CRITICAL!!!  Kinda important methinks.
	// Although this looks to be for receiving button info from the server to the client?
	// Seems a bit backwards, but all this prediction mumbo jumbo can be weird.
	// input.cpp's own "->button" transfer is probably more forward to understand.
	localPlayer.pev->button = cmd->buttons;

	localPlayer.pev->velocity = from->client.velocity;
	localPlayer.pev->flags = from->client.flags;

	localPlayer.pev->deadflag = from->client.deadflag;
	localPlayer.pev->waterlevel = from->client.waterlevel;
	localPlayer.pev->maxspeed = from->client.maxspeed;
	localPlayer.pev->fov = from->client.fov;

	//IS THAT OKAY???
	//localPlayer.pev->weaponanim = from->client.weaponanim;


	//MODDD - little insertion here...
	//LITTLE TIP ABOUT WEAPON ANIMS: send an anim with 128 as part of a bitmask and it will animate backwards!


	//localPlayer.pev->weaponanim



	/*
	gEngfuncs.GetViewModel()->curstate.renderfx |= ISVIEWMODEL;


	if(from->client.weaponanim & 128){

		if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(testVar)==1){
			easyPrintLine("BACKWARDS!!!! %d : %d", from->client.weaponanim & ~128, from->client.weaponanim);
		}

		from->client.weaponanim &= ~128;
		gEngfuncs.GetViewModel()->curstate.renderfx |= ANIMATEBACKWARDS;
	}else{
		if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(testVar)==1){
			easyPrintLine("NOOOOOT BACKWARDS!!!! %d", from->client.weaponanim);
		}
		gEngfuncs.GetViewModel()->curstate.renderfx &= ~ANIMATEBACKWARDS;
	}
	*/

	if (pWeapon == &g_Glock) {
		//DIE.  DIE HARD.

	}


	localPlayer.pev->weaponanim = from->client.weaponanim;

	localPlayer.pev->viewmodel = from->client.viewmodel;


	//mODDD - SIGNIFICANT!!!   Don't give m_flNextAttack from here if it was set clientside before this point in this frame!
	// Using a different idea, a copy of m_flNextAttack called m_flNextAttackCLIENTHISTORY instead, kept the same until the end
	// of this method.  That is to be used in cl_weapons.cpp instead.   (and cl_player.cpp if it showed up there)
	// Might not need anything like this regardless but it doesn't appear to hurt.
	//if (localPlayer.m_flNextAttack == flNextAttackChangeMem) {
	localPlayer.m_flNextAttack = from->client.m_flNextAttack;
	//}
	//flNextAttackChangeMem = localPlayer.m_flNextAttack;


	localPlayer.m_flNextAmmoBurn = from->client.fuser2;
	localPlayer.m_flAmmoStartCharge = from->client.fuser3;

	//Stores all our ammo info, so the client side weapons can use them.
	localPlayer.ammo_9mm = (int)from->client.vuser1[0];
	localPlayer.ammo_357 = (int)from->client.vuser1[1];
	localPlayer.ammo_argrens = (int)from->client.vuser1[2];
	localPlayer.ammo_bolts = (int)from->client.ammo_nails; //is an int anyways...
	localPlayer.ammo_buckshot = (int)from->client.ammo_shells;
	localPlayer.ammo_uranium = (int)from->client.ammo_cells;
	localPlayer.ammo_hornets = (int)from->client.vuser2[0];
	localPlayer.ammo_rockets = (int)from->client.ammo_rockets;


	// Point to current weapon object
	if (from->client.m_iId)
	{
		// OOoooooooo you motherfucker you
		localPlayer.m_pActiveItem = g_pWpns[from->client.m_iId];
	}

	//easyForcePrintLine("AW snao D %.2f", localPlayer.m_flNextAttack);


	if (localPlayer.m_pActiveItem->m_iId == WEAPON_RPG)
	{
		((CRpg*)localPlayer.m_pActiveItem)->m_fSpotActive = (int)from->client.vuser2[1];
		((CRpg*)localPlayer.m_pActiveItem)->m_cActiveRockets = (int)from->client.vuser2[2];
	}
	//MODDD - added.
	else if (localPlayer.m_pActiveItem->m_iId == WEAPON_PYTHON) {
		((CPython*)localPlayer.m_pActiveItem)->m_fSpotActive = (int)from->client.vuser2[1];
	}

	//MODDD


	/*
	//it's all in interpretation...  (only things trying to use this will ever get this right)
	if( !(pWeapon->m_fireState & 128)){

		gEngfuncs.GetViewModel()->curstate.renderfx |= ISVIEWMODEL;
		gEngfuncs.GetViewModel()->curstate.renderfx &= ~ANIMATEBACKWARDS;

	}else{

		gEngfuncs.GetViewModel()->curstate.renderfx |= ISVIEWMODEL;
		gEngfuncs.GetViewModel()->curstate.renderfx |= ANIMATEBACKWARDS;
	}
	*/


	//easyPrintLine("WHAT??? %d %d %d", gEngfuncs.GetViewModel()->curstate.renderfx,  (DONOTDRAWSHADOW | ISPLAYER), ISVIEWMODEL);



	/*
	// REMOVED, used better-synched vars since, but keep this sort of thing in mind if needed in the future.
	// And why was m_flTimeWeaponIdle a thing?  Isn't it synched already?  Wow I completely forget why I did that.
	if(flag_apply_m_flTimeWeaponIdle){
		easyForcePrintLine("flag_apply1. newidle: %.2f", stored_m_flTimeWeaponIdle);
		pWeapon->m_flTimeWeaponIdle = stored_m_flTimeWeaponIdle;
		flag_apply_m_flTimeWeaponIdle = FALSE;
	}
	if(flag_apply_m_fJustThrown){
		easyForcePrintLine("flag_apply2 . newthrown: %d", stored_m_fJustThrown);
		//can only work if our weapon is a snark or chumtoad.

		if(pWeapon->m_iId == WEAPON_SNARK){
			CSqueak* tempSqueak = static_cast<CSqueak*>(pWeapon);
			tempSqueak->m_fJustThrown = stored_m_fJustThrown;
		}else if(pWeapon->m_iId == WEAPON_CHUMTOAD){
			CChumToadWeapon* tempToad = static_cast<CChumToadWeapon*>(pWeapon);
			tempToad->m_fJustThrown = stored_m_fJustThrown;
		}
		flag_apply_m_fJustThrown = FALSE;
	}
	*/



	//MODDD - OLD LOCATION OF ItemPostFrame() CALLS.  Moved to back here, some issues with the new location.
	if ((localPlayer.pev->deadflag != (DEAD_DISCARDBODY + 1)) &&
		!CL_IsDead() && localPlayer.pev->viewmodel && !g_iUser1)
	{
		//MODDD - just like the player does, call this too.		
		pWeapon->ItemPostFrameThink();



		if (localPlayer.m_flNextAttack <= 0)
		{
			pWeapon->ItemPostFrame();
		}
		//MODDD - new.  No need to keep counting after that
		if(g_framesSinceRestore < 50){
			g_framesSinceRestore++;
		}
	}

	// Assume that we are not going to switch weapons
	to->client.m_iId = from->client.m_iId;

	// Now see if we issued a changeweapon command ( and we're not dead )
	if (cmd->weaponselect && (localPlayer.pev->deadflag != (DEAD_DISCARDBODY + 1)))
	{
		// Switched to a different weapon?
		if (from->weapondata[cmd->weaponselect].m_iId == cmd->weaponselect)
		{


			CBasePlayerWeapon* pNew = g_pWpns[cmd->weaponselect];

			//MODDD NOTICE - unlike the player's (player.cpp) own script that works similary to this in the SelectItem method, serverside only there,
			//               this does nothing at all if pNew turns out to be NULL. SelectItem will still set the current item to NULL at least,
			//               and set the LastWeapon to what it was.
			//               Is it okay this way? TODO - test this sometime.
			//               NEVERMIND - my understanding is a little off. The player's currently equipped weapon to holster there, is m_pActiveItem (and here)
			//               so it will be ok to change weaps even if it is null.  pNew is what to change to, which must not be null. this is ok, I think.

			if (pNew && (pNew != pWeapon)
				//MODDD - new condition to mirror the player's change. If queueing up a weapon during the holster anim,
				//        it can be the same as you started with.
				|| (localPlayer.m_pQueuedActiveItem != NULL && pNew != localPlayer.m_pQueuedActiveItem)
				)
			{

				//MODDD - now anticipates the holster animations better.
				/*
				// Put away old weapon
				if (localPlayer.m_pActiveItem)
					localPlayer.m_pActiveItem->Holster( );

				localPlayer.m_pLastItem = localPlayer.m_pActiveItem;
				localPlayer.m_pActiveItem = pNew;

				// Deploy new weapon
				if (localPlayer.m_pActiveItem)
				{
					localPlayer.m_pActiveItem->Deploy( );
				}

				// Update weapon id so we can predict things correctly.
				to->client.m_iId = cmd->weaponselect;
				*/


				//MODDD - NEW.
				localPlayer.setActiveItem_HolsterCheck(pNew);


				//MODDD - See if the requested weapon (pnew) become the one equipped (active).
				// That happens when not using holster (instantly applied instead of unholstering the current and queueing the one wanted)
				if (localPlayer.m_pActiveItemCLIENTHISTORY == pNew) {
					// Update weapon id so we can predict things correctly.
					to->client.m_iId = cmd->weaponselect;
				}
				/////////////////////////////////////////////////////////////////////////////////////////////

			}
		}
	}

	// Copy in results of prediction code
	to->client.viewmodel = localPlayer.pev->viewmodel;
	to->client.fov = localPlayer.pev->fov;
	to->client.weaponanim = localPlayer.pev->weaponanim;
	to->client.m_flNextAttack = localPlayer.m_flNextAttack;
	to->client.fuser2 = localPlayer.m_flNextAmmoBurn;
	to->client.fuser3 = localPlayer.m_flAmmoStartCharge;
	to->client.maxspeed = localPlayer.pev->maxspeed;

	//HL Weapons
	to->client.vuser1[0] = localPlayer.ammo_9mm;
	to->client.vuser1[1] = localPlayer.ammo_357;
	to->client.vuser1[2] = localPlayer.ammo_argrens;

	to->client.ammo_nails = localPlayer.ammo_bolts;
	to->client.ammo_shells = localPlayer.ammo_buckshot;
	to->client.ammo_cells = localPlayer.ammo_uranium;
	to->client.vuser2[0] = localPlayer.ammo_hornets;
	to->client.ammo_rockets = localPlayer.ammo_rockets;

	if (localPlayer.m_pActiveItem->m_iId == WEAPON_RPG)
	{
		from->client.vuser2[1] = ((CRpg*)localPlayer.m_pActiveItem)->m_fSpotActive;
		from->client.vuser2[2] = ((CRpg*)localPlayer.m_pActiveItem)->m_cActiveRockets;
	}
	//MODDD - addition
	else if (localPlayer.m_pActiveItem->m_iId == WEAPON_PYTHON) {
		from->client.vuser2[1] = ((CPython*)localPlayer.m_pActiveItem)->m_fSpotActive;
		//??
	}



	//Apparently, animations sent from serverside are picked up here.

	// Make sure that weapon animation matches what the game .dll is telling us
	//  over the wire ( fixes some animation glitches )


	//easyForcePrintLine("WHAT THE FUCK c:%d f:%d t:%d pw:%d pr:%d", HUD_GetWeaponAnim(), from->client.weaponanim, to->client.weaponanim, pWeapon->m_pPlayer->pev->weaponanim, pWeapon->pev->sequence);


	int tempthingy = to->client.weaponanim;
	if (to->client.weaponanim == ANIM_NO_UPDATE) {
		int xxx = 4;
	}
	if (to->client.weaponanim == 9) {
		int xxx = 4;
	}





	// OOOOOOOOOooooooooooooooooooooooookay.  So maybe don't do this, random crash ahoy.
	// Just check m_chargeReady straightaway.    Sheesh.
	/*
	if (pWeapon->m_chargeReady & 128) {
		localPlayer.m_bHolstering = TRUE;
	}
	else {
		localPlayer.m_bHolstering = FALSE;
	}
	*/



	int themodeltemp = gEngfuncs.GetViewModel()->curstate.modelindex;


	if (seqPlayDelay != -1 && gpGlobals->time >= seqPlayDelay) {
		seqPlayDelay = -1;
		//SendWeaponAnim(seqPlay, 1, pCurrent->pev->body);
		HUD_SendWeaponAnim(seqPlay, 2, 1);
	}





	if (forgetBlockUntilModelChangeTime != -1 && gpGlobals->time >= forgetBlockUntilModelChangeTime) {
		//aw no
		forgetBlockUntilModelChangeTime = -1;
		blockUntilModelChange = FALSE;
	}


	if (blockUntilModelChange) {
		// has the model changed?
		int theModel = gEngfuncs.GetViewModel()->curstate.modelindex;
		if (oldModel != -1 && theModel != oldModel) {
			// pass.
			blockUntilModelChange = FALSE;
			// !!! DANGEROUSLY HACKY reconsider
			if (queuedBlockedModelAnim != -1) {
				//to->client.weaponanim = queuedBlockedModelAnim;
				HUD_SendWeaponAnim(queuedBlockedModelAnim, 2, 1);
			}
			queuedBlockedModelAnim = -1;
			oldModel = -1;
			goto skipperLoc2;
		}
	}



	//MODDD - is this better?
	// ...actually no, bad idea.  It causes client-only issued animations like ones
	// given in ev_hldm to get overwritten by a repeat of the most previously issueed
	// server animation (to->client.weaponanim).
	//extern int g_currentanim;
	//g_currentanim = gEngfuncs.GetViewModel()->curstate.sequence;


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//MODDD - MAJOR MAJOR MAJOR.
	// is that a good idea?
	// Removing the ability of g_currentanim being set differently to cause changes in animations.


	// for a breakpoint only.
	{
		int clientWeaponAnim = HUD_GetWeaponAnim();
	}


	//BOOL tempo1 = g_cl_HUD_Frame_ran;
	//BOOL tempo2 = g_cl_HUD_UpdateClientData_ran;
	//BOOL tempo3 = g_HUD_Redraw_ran;


	if (g_cl_frameCount > 4) {
		if (pWeapon == &g_Crowbar) {
			if (to->client.weaponanim == CROWBAR_ATTACK1HIT || to->client.weaponanim == CROWBAR_ATTACK2HIT || to->client.weaponanim == CROWBAR_ATTACK3HIT) {
				// go ahead and allow below.
				int x = 45;
			}
			else {
				// oh.
				g_currentanim = to->client.weaponanim;
			}
		}
		else {
			g_currentanim = to->client.weaponanim;
		}
	}

	// The game is really running when HUD_Redraw has been called since joining a game / starting one / etc.
	// Yes, really.  That kind of check.     Anyway, this starts bumping frames up.
	if (g_HUD_Redraw_ran) {
		g_cl_frameCount++;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////


	//if(to->client.weaponanim  == EGON_IDLE1){
	//	g_currentanim = 666;
	//}

	//MODDD - added check for "254".  That's a special code for, "the serverside anim request got cleared".
	// This probably happened from becoming irrelevant and we don't want to make the client play an
	// irrelevant animation, like putting on/taking off the glock silencer twice.
	if ( g_runfuncs && (to->client.weaponanim != ANIM_NO_UPDATE && HUD_GetWeaponAnim() != to->client.weaponanim ) )
	{
		int body = 2;

		//Pop the model to body 0.
		if ( pWeapon == &g_Tripmine ){
			 body = 0;
		}


		if(to->client.weaponanim & 64){
			//this is only a cheap trick to get the animation to change. Remove it.
			easyForcePrintLine("flaggo 46");

			/*
			to->client.weaponanim &= ~64;

			from->client.weaponanim &= ~64;
			pWeapon->pev->weaponanim &= ~64;

			pWeapon->m_pPlayer->pev->weaponanim &= ~64;
			*/

		}


		//Show laser sight/scope combo
		//MODDD - how this is done is slightly different.
		/*
		if ( pWeapon == &g_Python && bIsMultiplayer() )
			 body = 1;
		*/
		if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(viewModelPrintouts)==1)easyForcePrintLine("Client: Received anim from server. Existing anim: %d New: %d", HUD_GetWeaponAnim(), to->client.weaponanim);




		if (seqPlayDelay != -1) {
			int x = 666;




		}else

			//if (!localPlayer.m_bHolstering) {
			// This is a sign of holsterng.  If doing so, don't accept further requests to change animation.
			// They are too early.
			//if( (!(pWeapon->m_chargeReady & 128) && !localPlayer.m_bHolstering) && !blockUntilModelChange ){
		if (!blockUntilModelChange) {
		//if(1){



			if (resistTime != -1) {
				if (gpGlobals->time >= resistTime) {
					resistTime = -1;
				}
				else {
					//return;
					goto skipperLoc;
				}
			}

			//MODDD
			if (pWeapon == &g_Glock) {
				// Force a fixed anim down to viewmodel
				HUD_SendWeaponAnim(to->client.weaponanim, g_Glock.m_fireState, 2);

			}
			else if (pWeapon == &g_Python) {
				HUD_SendWeaponAnim(to->client.weaponanim, g_Python.m_fInAttack, 2);

			}
			else if (pWeapon == &g_Shotgun) {

				HUD_SendWeaponAnim(to->client.weaponanim, body, 2);

			}
			else {
				// Force a fixed anim down to viewmodel
				HUD_SendWeaponAnim(to->client.weaponanim, body, 1);
			}
		skipperLoc:		int x = 4;

		}//END OF holstering check
		else {
			int x = 45;
		}

		//put above in "else".
		//// Force a fixed anim down to viewmodel
		//HUD_SendWeaponAnim( to->client.weaponanim, body, 1 );
	}
	else {
		// Forget this then!!
		// careful, don't be so eager.
		//blockUntilModelChange = FALSE;
	}

	skipperLoc2:



	for ( i = 0; i < 32; i++ )
	{
		pCurrent = g_pWpns[ i ];

		pto = &to->weapondata[ i ];

		if ( !pCurrent )
		{
			memset( pto, 0, sizeof( weapon_data_t ) );
			continue;
		}
	
		pto->m_fInReload				= pCurrent->m_fInReload;
		pto->m_fInSpecialReload			= pCurrent->m_fInSpecialReload;
//		pto->m_flPumpTime				= pCurrent->m_flPumpTime;
		pto->m_iClip					= pCurrent->m_iClip; 

		if(pCurrent->m_iId == WEAPON_SHOTGUN){
			//easyForcePrintLine("UPDATE B: %.2f -> %.2f", pto->m_flNextPrimaryAttack, pCurrent->m_flNextPrimaryAttack);
		}


		pto->m_flNextPrimaryAttack		= pCurrent->m_flNextPrimaryAttack;
		pto->m_flNextSecondaryAttack	= pCurrent->m_flNextSecondaryAttack;
		pto->m_flTimeWeaponIdle			= pCurrent->m_flTimeWeaponIdle;
		pto->fuser1						= pCurrent->pev->fuser1;
		pto->fuser2						= pCurrent->m_flStartThrow;
		pto->fuser3						= pCurrent->m_flReleaseThrow;

		
		//NOTE: fuser4 seems unreliable, do not use!
		//pto->fuser4						= pCurrent->fuser4;
		/*
		pto->fuser5						= pCurrent->fuser5;
		pto->fuser6						= pCurrent->fuser6;
		pto->fuser7						= pCurrent->fuser7;
		pto->fuser8						= pCurrent->fuser8;
		*/
		
		//if(pCurrent->m_iId == WEAPON_GLOCK && pto->m_iId == WEAPON_GLOCK){

		//}else{
		pto->iuser1						= pCurrent->m_chargeReady;
		//}
		pto->iuser2						= pCurrent->m_fInAttack;
		pto->iuser3						= pCurrent->m_fireState;

		// Decrement weapon counters, server does this at same time ( during post think, after doing everything else )
		pto->m_flNextReload				-= cmd->msec / 1000.0;
		pto->m_fNextAimBonus			-= cmd->msec / 1000.0;
		
		if(pCurrent->m_iId == WEAPON_SHOTGUN){
			//easyForcePrintLine("UPDATE C: %.2f -> %.2f", pto->m_flNextPrimaryAttack, pto->m_flNextPrimaryAttack - cmd->msec / 1000.0 );
		}

		pto->m_flNextPrimaryAttack		-= cmd->msec / 1000.0;
		pto->m_flNextSecondaryAttack	-= cmd->msec / 1000.0;
		pto->m_flTimeWeaponIdle			-= cmd->msec / 1000.0;
		pto->fuser1						-= cmd->msec / 1000.0;

		//MODDD - cached ammo types, disregard
		//to->client.vuser3[2]				= pCurrent->m_iSecondaryAmmoType;
		//to->client.vuser4[0]				= pCurrent->m_iPrimaryAmmoType;



		/*
		int myPrimaryAmmoType = pCurrent->getPrimaryAmmoType();
		int mySecondaryAmmoType = pCurrent->getSecondaryAmmoType();

		//MODDD - boundary checks
		//...cached now, nevermind this
		if (IS_AMMOTYPE_VALID(myPrimaryAmmoType)) {
			to->client.vuser4[1] = localPlayer.m_rgAmmo[myPrimaryAmmoType];
		}

		if (IS_AMMOTYPE_VALID(mySecondaryAmmoType)) {
			to->client.vuser4[2] = localPlayer.m_rgAmmo[mySecondaryAmmoType];
		}
		*/




/*		if ( pto->m_flPumpTime != -9999 )
		{
			pto->m_flPumpTime -= cmd->msec / 1000.0;
			if ( pto->m_flPumpTime < -0.001 )
				pto->m_flPumpTime = -0.001;
		}*/

		if ( pto->m_fNextAimBonus < -1.0 )
		{
			pto->m_fNextAimBonus = -1.0;
		}

		if ( pto->m_flNextPrimaryAttack < -1.0 )
		{
			if(pCurrent->m_iId == WEAPON_SHOTGUN){
				//easyForcePrintLine("HOW THE hey THIS HAPPEN WILLIS.");
			}
			pto->m_flNextPrimaryAttack = -1.0;
		}

		if ( pto->m_flNextSecondaryAttack < -0.001 )
		{
			pto->m_flNextSecondaryAttack = -0.001;
		}

		if ( pto->m_flTimeWeaponIdle < -0.001 )
		{
			pto->m_flTimeWeaponIdle = -0.001;
		}

		if ( pto->m_flNextReload < -0.001 )
		{
			pto->m_flNextReload = -0.001;
		}

		if ( pto->fuser1 < -0.001 )
		{
			pto->fuser1 = -0.001;
		}
	}//END OF that for loop. Wow don't hide from me like that.

	// m_flNextAttack is now part of the weapons, but is part of the player instead
	to->client.m_flNextAttack -= cmd->msec / 1000.0;
	if ( to->client.m_flNextAttack < -0.001 )
	{
		to->client.m_flNextAttack = -0.001;
	}

	to->client.fuser2 -= cmd->msec / 1000.0;
	if ( to->client.fuser2 < -0.001 )
	{
		to->client.fuser2 = -0.001;
	}
	
	to->client.fuser3 -= cmd->msec / 1000.0;
	if ( to->client.fuser3 < -0.001 )
	{
		to->client.fuser3 = -0.001;
	}

	// Store off the last position from the predicted state.
	HUD_SetLastOrg();

	// Wipe it so we can't use it after this frame
	g_finalstate = NULL;



	/*
	if(from->client.m_iId != WEAPON_GLOCK && gEngfuncs.GetViewModel()->curstate.iuser1 == -500){
		//if(from->client.m_iId != WEAPON_SHOTGUN){
		gEngfuncs.GetViewModel()->curstate.iuser1 = 0;
		//}
		//gEngfuncs.GetLocalPlayer()->curstate.iuser1 = 0;
	}
	*/

	/*
	gEngfuncs.GetLocalPlayer()->curstate.body = 50000;
	gEngfuncs.GetViewModel()->curstate.body = -500;
	localPlayer.pev->body = 50002;
	pWeapon->m_pPlayer->pev->body = -250;
	*/


	//by default, enable the muzzle flash in case it was tampered with.
	//gEngfuncs.GetViewModel()->curstate.renderfx &= ~NOMUZZLEFLASH;
	//hacky hacky.
	gEngfuncs.GetViewModel()->curstate.renderfx &= ~(1 << 1);


	//MODDD TODO - shouldn't the body always be reset to 0 just in case?
	//gEngfuncs.GetViewModel()->curstate.body = 0;

	if(from->client.m_iId == WEAPON_TRIPMINE){
		//...the tripmine still needs this to reset its body to 0 if it has been tampered with (like by the glock-silencer
		//setting the client model's body to "1".  Default is 0.
		gEngfuncs.GetViewModel()->curstate.body = 0;

	}else if(from->client.m_iId == WEAPON_GLOCK){
		//the glock's "body" (submodel) is coordinated with "m_fireState".

		//Check, is this reliable? Should the body be set in other places too to be safe?
		gEngfuncs.GetViewModel()->curstate.body = pWeapon->m_fireState;

		if(pWeapon->m_fireState == 0){
			//silencer off? muzzle flash.
		}else{
			//disable muzzle flash for silencer.
			gEngfuncs.GetViewModel()->curstate.renderfx |= NOMUZZLEFLASH;
			//will this work??
			//pWeapon->m_pPlayer->pev->renderfx |= NOMUZZLEFLASH;
			//localPlayer.pev->renderfx |= NOMUZZLEFLASH;
		}

		//gEngfuncs.GetViewModel()->curstate.iuser1 = -500;  //tells the event system to pay attention to the model (no muzzle flash if the silencer is on)

		//easyPrintLine("CLIENT FIRESTATE: %d", pWeapon->m_fireState);
		

		//localPlayer.pev->iuser1 = 50000;
		//gEngfuncs.GetLocalPlayer()->curstate.iuser1 = 500 + pWeapon->m_fireState;

	}else if(from->client.m_iId == WEAPON_PYTHON){
		gEngfuncs.GetViewModel()->curstate.body = pWeapon->m_fInAttack;

		
		//easyPrintLine("PRINTER START!!!!!!!!!!!!!!!!!!!!!!!!!!");
		//for(int i = 0; i < 32; i++){
		//	entvars_t& thisEnt = ev[i];
		//	easyPrintLine("PRINTER:::%s", thisEnt.classname);
		//}
		//easyPrintLine("PRINTER END!!!!!!!!!!!!!!!!!!!!!!!!!!");
		

	}else if(from->client.m_iId == WEAPON_EGON){
		
		//?
		//gEngfuncs.GetViewModel()->curstate.frame = 17;
	}

	/*
	gEngfuncs.GetViewModel()->baseline.body = pWeapon->m_fireState;
	gEngfuncs.GetViewModel()->curstate.body = pWeapon->m_fireState;
	*/
	//gEngfuncs.GetViewModel()->baseline.body = gEngfuncs.GetViewModel()->prevstate.body;


	if(numberOfEyeSkins == 0){
		//force a skin of just the default.
		gEngfuncs.GetViewModel()->curstate.skin = 0;

	}else{
		//do blink logic here. Once every 0.1 seconds to resemble the NPC logic.
		if(gpGlobals->time >= nextBlinkLogicFrame){
			nextBlinkLogicFrame = gpGlobals->time + 0.1;

			cl_entity_s* viewModelRef = gEngfuncs.GetViewModel();
			entity_state_t& viewModelState = viewModelRef->curstate;

			if( ( viewModelState.skin == 0 ) && RANDOM_LONG(0, blinkChancePerFrame) == 0)
			{
				viewModelState.skin = max(numberOfEyeSkins - 1, 0);
			}
			else if ( viewModelState.skin > 0 )
			{// already blinking
				viewModelState.skin--;
			}

		}//END OF time check (every 0.1 seconds)

	}//END OF skin count check
	


	// And keep this in synch in case of changes the client didn't replicate.
	localPlayer.m_pActiveItemCLIENTHISTORY = localPlayer.m_pActiveItem;
	localPlayer.m_flNextAttackCLIENTHISTORY = localPlayer.m_flNextAttack;
	//if (pWeapon != NULL) {
	//	pWeapon->m_flNextPrimaryAttackCLIENTHISTORY = pWeapon->m_flNextPrimaryAttack;
	//}
	
	for (i = 0; i < MAX_AMMO_TYPES; i++) {
		localPlayer.m_rgAmmoCLIENTHISTORY[i] = localPlayer.m_rgAmmo[i];
	}
	
}//END OF HUD_WeaponsPostThink






/*
=====================
HUD_PostRunCmd

Client calls this during prediction, after it has moved the player and updated any info changed into to->
time is the current client clock based on prediction
cmd is the command that caused the movement, etc
runfuncs is 1 if this is the first time we've predicted this command.  If so, sounds and effects should play, otherwise, they should
be ignored
=====================
*/
void DLLEXPORT HUD_PostRunCmd(struct local_state_s* from, struct local_state_s* to, struct usercmd_s* cmd, int runfuncs, double arg_time, unsigned int random_seed)
{
	//MODDD - new. Must keep track of the time between method calls to know when it is ok to do an eye-check.
	
	g_runfuncs = runfuncs;

	//MODDD - moved to here from HUD_WeaponsPostThink.
	HUD_InitClientWeapons();
	gpGlobals->time = arg_time;

	//MODDD - curious about other places, any point of using 'gEngfuncs.GetClientTime()' instead of relying on the time
	// supplied by these HUD_PostRunCmd calls (the 'time' parameter)?


	//MODDD - for now, these are unused.  Who knows if they'd ever be useful.
	// unfortunately to->client.bInDuck doesn't seem to work.  view.cpp has 'ent->curstate.usehull' at least.
	//g_recentDuckVal = to->client.bInDuck;
	//recentDuckTime = to->client.flDuckTime;

	//////////////////////////////////////////////////////////////////////////
	//MODDD - should we keep doing this??
	// ok, but ONLY for singleplayer!  In multiplayer it breaks viewmodel animations.  They often, randomly break.
	// (EV_ calls don't make it from server to client; nothing shows up at all, no view anims / clientside event if the glitch happens)
	// That's not a frustrating problem to run into at all!

	// NOTE - the difference between current time and previous becomes negative if paused
	// with high host_framerate values (somewhere above 0.02).  I have no clue.

	// ALSO, at high enough host_framerate choices (above 0.051), the time will go forwards a bit and repeatedly
	// snap back to its old place, so that some frames register as paused frames (delta <= 0) and other times don't,
	// it's weird.
	// INTERESTING.  This looks to be related to the client running multiple times when above 0.051 for each time
	// the server runs (from a breakpoint on the equipped weapon's WeaponIdle).
	// A fix would involve checking host_framerate to see what counts as that, but who's really
	// using high host_framerate's like that anyway.  Not worth risking mistaking unpaused frames for paused at 
	// high speeds, working as intended when unpaused is really the point there.
	BOOL g_cl_isPaused = (gpGlobals->time - sp_ClientPreviousTime) <= 0;
	sp_ClientPreviousTime = gpGlobals->time;


	//easyForcePrintLine("paused? %d delta? %.2f cur? %.2f", g_cl_isPaused, (gpGlobals->time - sp_ClientPreviousTime), gpGlobals->time);

	if (!IsMultiplayer()) {
		float pausecorrection_val = EASY_CVAR_GET(pausecorrection1);
		// CLIENT PAUSE CORRECTION FIX.  Block think logic if there isn't any time since the previous frame.
		if (pausecorrection_val != 0) {
			if(g_cl_isPaused){
				//If no time has passed since the last client-reported time, assume we're paused. Don't try any logic.
				//easyForcePrintLine("HUD_PostRunCmd: paused. time:%.2f", gpGlobals->time);
				return;
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////


	//localPlayer = gEngfuncs.GetLocalPlayer();

	//if(localPlayer){
	if (localPlayer.m_pActiveItem != 0) {
		localPlayer.m_pActiveItem->m_flStartThrow = 2;
	}
	// NOTE!  Pointless!   Funny enough 'm_pClientActiveItem' is only for the server to see whetehr the active
	// item changed by the end of a frame for doing a certain update.  May as well be dummied clientside!
	//if (localPlayer.m_pClientActiveItem != 0) {
	//	localPlayer.m_pClientActiveItem->m_flStartThrow = 2;
	//}
	//}




#if defined( CLIENT_WEAPONS )

	//MODDD - way to force some edits for testing.   Maybe.
	////////////////////////////////////////////////////////////////////////////////////////
	/*
	if(from->client.m_iId == WEAPON_GLOCK){
		gEngfuncs.GetViewModel()->curstate.body = pWeapon->m_fireState;
	}
	*/
	/*
	if(!cl_lw){
		easyPrintLine("FLAG4234");
	}else{
		if( !(cl_lw->value) ){
			easyPrintLine("FLAG4235");
		}
	}
	*/
	//gEngfuncs.GetViewModel()->curstate.body = 1;
	////////////////////////////////////////////////////////////////////////////////////////




	if (cl_lw && cl_lw->value)
	{
		HUD_WeaponsPostThink(from, to, cmd, arg_time, random_seed);

		g_cl_firstSendoffSinceMapLoad = FALSE;
	}
	else
#endif
	{
		to->client.fov = g_lastFOV;
	}

	// ???? what  is  this  doing
	if (g_irunninggausspred == TRUE)
	{
		Vector forward;
		gEngfuncs.pfnAngleVectors(v_angles, forward, NULL, NULL);
		to->client.velocity = to->client.velocity - forward * g_flApplyVel * 5;
		g_irunninggausspred = FALSE;
	}

	// All games can use FOV state
	g_lastFOV = to->client.fov;
}







////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FROM Com_Weapons.cpp
// Shared weapons common/shared functions



/*
====================
COM_Log

Log debug messages to file ( appends )
====================
*/
void COM_Log( char *pszFile, char *fmt, ...)
{
	va_list		argptr;
	char		string[1024];
	FILE *fp;
	char *pfilename;
	
	if ( !pszFile )
	{
		pfilename = "c:\\hllog.txt";
	}
	else
	{
		pfilename = pszFile;
	}

	va_start (argptr,fmt);
	vsprintf (string, fmt,argptr);
	va_end (argptr);

	fp = fopen( pfilename, "a+t");
	if (fp)
	{
		fprintf(fp, "%s", string);
		fclose(fp);
	}
}



//#include "crowbar.h"

/*
=====================
HUD_SendWeaponAnim

Change weapon model animation
=====================
*/
void HUD_SendWeaponAnim( int iAnim, int body, int force )
{
	if (iAnim == -1) {
		// not a sequence, ignore
		return;
	}


	//if (iAnim == CROWBAR_ATTACK1HIT || iAnim == CROWBAR_ATTACK2HIT || iAnim == CROWBAR_ATTACK3HIT) {
	//	int x = 45;
	//}

	//RESIST
	//resistTime = gpGlobals->time + 0.01;

	// Don't actually change it.
	if ( !g_runfuncs && !force && force != 2  )
		return;


	if(iAnim & 64){
		//just remove it.
		iAnim &= ~64;
	}


	// MOVED ABOVE  ... or not, maybe don't
	g_currentanim = iAnim;


	BOOL doReverse = FALSE;

	if(iAnim & 128){
		//we have this bit? Take it off, not really part of the anim.
		//iAnim &= ~128;

		//it is a signal to play this animation in reverse.
		doReverse = TRUE;
	}

	//MODDD - just to make sure there isn't a flicker.

	if(force == 2){
		if(gEngfuncs.GetViewModel()->curstate.body != body){
			gEngfuncs.GetViewModel()->curstate.body = body;
		}
	}

	//!!!

	// Tell animation system new info

	/*
	if(doReverse){
		gEngfuncs.GetViewModel()->curstate.renderfx |= ANIMATEBACKWARDS;
	}else{
		gEngfuncs.GetViewModel()->curstate.renderfx &= ~ANIMATEBACKWARDS;
	}
	*/

	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(viewModelPrintouts)==1)easyForcePrintLine("!!!!pfnWeaponAnim: a:%d rev?:%d", iAnim, doReverse);

	gEngfuncs.pfnWeaponAnim( iAnim, body);


	//MODDD - just to make sure there isn't a flicker.
	if(force == 2){
		if(gEngfuncs.GetViewModel()->curstate.body != body){
			gEngfuncs.GetViewModel()->curstate.body = body;
		}
	}

}

/*
=====================
HUD_GetWeaponAnim

Retrieve current predicted weapon animation
=====================
*/
int HUD_GetWeaponAnim( void )
{
	return g_currentanim;
}

/*
=====================
HUD_PlaySound

Play a sound, if we are seeing this command for the first time
=====================
*/
void HUD_PlaySound( char *sound, float volume )
{
	if ( !g_runfuncs || !g_finalstate )
		return;

	gEngfuncs.pfnPlaySoundByNameAtLocation( sound, volume, (float *)&g_finalstate->playerstate.origin );
}

/*
=====================
HUD_PlaybackEvent

Directly queue up an event on the client
=====================
*/
void HUD_PlaybackEvent( int flags, const edict_t *pInvoker, unsigned short eventindex, float delay,
	float *origin, float *angles, float fparam1, float fparam2, int iparam1, int iparam2, int bparam1, int bparam2 )
{
	vec3_t org;
	vec3_t ang;

	//MODDD - how about, if there is no final state, use the supplied origin and angles?   (or really v_origin and v_angles, the 2nd of which it always used anyway)?
	/*
	if ( !g_runfuncs || !g_finalstate )
	     return;

	// Weapon prediction events are assumed to occur at the player's origin
	org			= g_finalstate->playerstate.origin;
	ang			= v_angles;
	gEngfuncs.pfnPlaybackEvent( flags, pInvoker, eventindex, delay, (float *)&org, (float *)&ang, fparam1, fparam2, iparam1, iparam2, bparam1, bparam2 );
	*/

	// This can still stop, I suppose.
	if ( !g_runfuncs )
	     return;


	if(!g_finalstate){
		//MODDD - try this?
		org			= v_origin;
		ang			= v_angles;
		gEngfuncs.pfnPlaybackEvent( flags, pInvoker, eventindex, delay, (float *)&org, (float *)&ang, fparam1, fparam2, iparam1, iparam2, bparam1, bparam2 );
	}else{
		// Weapon prediction events are assumed to occur at the player's origin
		org			= g_finalstate->playerstate.origin;
		ang			= v_angles;
		gEngfuncs.pfnPlaybackEvent( flags, pInvoker, eventindex, delay, (float *)&org, (float *)&ang, fparam1, fparam2, iparam1, iparam2, bparam1, bparam2 );
	}

}

/*
=====================
HUD_SetMaxSpeed

=====================
*/
void HUD_SetMaxSpeed( const edict_t *ed, float speed )
{
}

//MODDD - several methods moved to util_shared.cpp, merged with copies seen in dlls/uti.cpp


/*
======================
stub_*

stub functions for such things as precaching.  So we don't have to modify weapons code that
 is compiled into both game and client .dlls.
======================
*/
int stub_PrecacheModel ( char* s ) { return 0; }
int stub_PrecacheSound ( char* s ) { return 0; }
unsigned short stub_PrecacheEvent ( int type, const char *s ) { return 0; }
const char *stub_NameForFunction ( unsigned long function ) { return "func"; }
void stub_SetModel ( edict_t *e, const char *m ) {}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////





