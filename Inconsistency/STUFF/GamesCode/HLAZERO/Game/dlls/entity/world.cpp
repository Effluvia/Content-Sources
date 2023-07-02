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

===== world.cpp ========================================================

  precaches and defs for entities and other data that must always be available.

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "nodes.h"
#include "soundent.h"
#include "client.h"
#include "decals.h"
#include "skill.h"
#include "effects.h"
#include "player.h"
#include "weapons.h"
#include "gamerules.h"
#include "gamerules_teamplay.h"


EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST(sv_germancensorship)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST(playerBulletHitEffectForceServer)
EASY_CVAR_EXTERN_DEBUGONLY(forceWorldLightOff)


#define SF_WORLD_DARK		0x0001		// Fade from black at startup
#define SF_WORLD_TITLE		0x0002		// Display game title at startup
#define SF_WORLD_FORCETEAM	0x0004		// Force teams


extern CGraph WorldGraph;
extern CSoundEnt *pSoundEnt;

extern CBaseEntity* g_pLastSpawn;
extern DLL_GLOBAL int gDisplayTitle;
extern DLL_GLOBAL BOOL g_fGameOver;
extern DLL_DECALLIST gDecals[];


extern BOOL g_queueCVarHiddenSave;
extern float forceWorldLightOffMem;
extern BOOL g_f_playerDeadTruce;


//MODDD -
extern void turnWorldLightsOn();
extern void turnWorldLightsOff();
extern void resetModCVars(CBasePlayer* arg_plyRef, BOOL isEmergency);


DLL_GLOBAL edict_t* g_pBodyQueueHead;
CGlobalState gGlobalState;
float g_flWeaponCheat;



//MODDD - comment below is likely out of date. "util.h" does not have any such decal list. "declas.h" does however.
//
// This must match the list in util.h
//
//MODDD - moved to decals.h. Why outside of decals.h to begin with?


/*
==============================================================================

BODY QUE

==============================================================================
*/

#define SF_DECAL_NOTINDEATHMATCH		2048

class CDecal : public CBaseEntity
{
public:
	void Spawn( void );
	void KeyValue( KeyValueData *pkvd );
	void EXPORT StaticDecal( void );
	void EXPORT TriggerDecal( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
};

LINK_ENTITY_TO_CLASS( infodecal, CDecal );

// UNDONE:  These won't get sent to joining players in multi-player
void CDecal::Spawn( void )
{
	if ( pev->skin < 0 || (gpGlobals->deathmatch && FBitSet( pev->spawnflags, SF_DECAL_NOTINDEATHMATCH )) )
	{
		REMOVE_ENTITY(ENT(pev));
		return;
	}

	if ( FStringNull ( pev->targetname ) )
	{
		SetThink( &CDecal::StaticDecal );
		// if there's no targetname, the decal will spray itself on as soon as the world is done spawning.
		pev->nextthink = gpGlobals->time;
	}
	else
	{
		// if there IS a targetname, the decal sprays itself on when it is triggered.
		SetThink ( &CBaseEntity::SUB_DoNothing );
		SetUse(&CDecal::TriggerDecal);
	}
}

void CDecal::TriggerDecal ( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	// this is set up as a USE function for infodecals that have targetnames, so that the
	// decal doesn't get applied until it is fired. (usually by a scripted sequence)
	TraceResult trace;
	int		entityIndex;

	UTIL_TraceLine( pev->origin - Vector(5,5,5), pev->origin + Vector(5,5,5),  ignore_monsters, ENT(pev), &trace );

	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE( TE_BSPDECAL );
		WRITE_COORD( pev->origin.x );
		WRITE_COORD( pev->origin.y );
		WRITE_COORD( pev->origin.z );
		WRITE_SHORT( (int)pev->skin );
		entityIndex = (short)ENTINDEX(trace.pHit);
		WRITE_SHORT( entityIndex );
		if ( entityIndex )
			WRITE_SHORT( (int)VARS(trace.pHit)->modelindex );
	MESSAGE_END();

	SetThink( &CBaseEntity::SUB_Remove );
	pev->nextthink = gpGlobals->time + 0.1;
}


void CDecal::StaticDecal( void )
{
	TraceResult trace;
	int entityIndex;
	int modelIndex;

	UTIL_TraceLine( pev->origin - Vector(5,5,5), pev->origin + Vector(5,5,5),  ignore_monsters, ENT(pev), &trace );

	entityIndex = (short)ENTINDEX(trace.pHit);
	if ( entityIndex )
		modelIndex = (int)VARS(trace.pHit)->modelindex;
	else
		modelIndex = 0;

//DOOMMARINE23 If we're in censorship mode, replace all dynamic blood placed by the level designer with censored versions.
//WELCOME TO IF/ELSE HELL
		if (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(sv_germancensorship))
		{	
			if ((int)pev->skin >= 230 && (int)pev->skin <= 237)	//Dynamic Human Blood {blood1 through {blood 8
				{
					(int)pev->skin = 98 + RANDOM_LONG(0,1); // Human blood turns into oil (98 and 99)
				}
			
			else if ((int)pev->skin >= 244 && (int)pev-> skin <= 245) //{BIGBLOOD1 and {BIGBLOOD2
				{
					(int)pev->skin = 98 + RANDOM_LONG(0,1); //Big Human blood turns into oil (98 and 99) TODO: Unique Big Oil Decals!
				}

			else if ((int)pev->skin >= 224 && (int)pev-> skin <= 229) //{BLOODHAND1 through {BLOODHAND6
				{
					(int)pev->skin = 98 + RANDOM_LONG(0,1); //Bloody Hands iturns into oil (98 and 99) TODO: Unique Oily Hand Decals!
				}

			else if ((int)pev->skin >= 43 && (int)pev-> skin <= 48)	//Dynamic Yellow Blood {yblood1 through {yblood6 turns into PLACEHOLDER YORE DEAD FREEMAN
				{
					(int)pev->skin = 143;
				}

			else
				{
					(int)pev->skin = (int)pev->skin; //We're not any kind of blood, just display as intended
				} 
		}

	g_engfuncs.pfnStaticDecal( pev->origin, (int)pev->skin, entityIndex, modelIndex );

	SUB_Remove();
}


void CDecal::KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "texture"))
	{
		pev->skin = DECAL_INDEX( pkvd->szValue );
		
		// Found
		if ( pev->skin >= 0 )
			return;
		ALERT( at_console, "Can't find decal %s\n", pkvd->szValue );
	}
	else
		CBaseEntity::KeyValue( pkvd );
}


// Body queue class here.... It's really just CBaseEntity
class CCorpse : public CBaseEntity
{
	virtual int ObjectCaps( void ) { return FCAP_DONT_SAVE; }	
};

LINK_ENTITY_TO_CLASS( bodyque, CCorpse );

static void InitBodyQue(void)
{
	string_t	istrClassname = MAKE_STRING("bodyque");

	g_pBodyQueueHead = CREATE_NAMED_ENTITY( istrClassname );
	entvars_t *pev = VARS(g_pBodyQueueHead);
	
	// Reserve 3 more slots for dead bodies
	for ( int i = 0; i < 3; i++ )
	{
		pev->owner = CREATE_NAMED_ENTITY( istrClassname );
		pev = VARS(pev->owner);
	}
	
	pev->owner = g_pBodyQueueHead;
}


//
// make a body que entry for the given ent so the ent can be respawned elsewhere
//
// GLOBALS ASSUMED SET:  g_eoBodyQueueHead
//
void CopyToBodyQue(entvars_t *pev) 
{
	if (pev->effects & EF_NODRAW)
		return;

	entvars_t *pevHead	= VARS(g_pBodyQueueHead);

	pevHead->angles		= pev->angles;
	pevHead->model		= pev->model;
	pevHead->modelindex	= pev->modelindex;
	pevHead->frame		= pev->frame;
	pevHead->colormap	= pev->colormap;
	pevHead->movetype	= MOVETYPE_TOSS;
	pevHead->velocity	= pev->velocity;
	pevHead->flags		= 0;
	pevHead->deadflag	= pev->deadflag;
	pevHead->renderfx	= kRenderFxDeadPlayer;
	pevHead->renderamt	= ENTINDEX( ENT( pev ) );

	pevHead->effects    = pev->effects | EF_NOINTERP;
	//pevHead->goalstarttime = pev->goalstarttime;
	//pevHead->goalframe	= pev->goalframe;
	//pevHead->goalendtime = pev->goalendtime ;
	
	pevHead->sequence = pev->sequence;
	pevHead->animtime = pev->animtime;

	UTIL_SetOrigin(pevHead, pev->origin);
	UTIL_SetSize(pevHead, pev->mins, pev->maxs);
	g_pBodyQueueHead = pevHead->owner;
}


CGlobalState::CGlobalState( void )
{
	Reset();

	//MODDD - need to involve the real constants like CFuncTrackChange::FuncTrachChangeIDLatest?  Probably not.
	m_i_monsterIDLatest = 0;
	m_i_FuncTrackChangeIDLatest = 0;
	m_i_PathTrackIDLatest = 0;
	m_i_scriptedIDLatest = 0;
}

void CGlobalState::Reset( void )
{
	m_pList = NULL; 
	m_listCount = 0;
}

globalentity_t *CGlobalState::Find( string_t globalname )
{
	if ( !globalname )
		return NULL;

	globalentity_t *pTest;
	const char *pEntityName = STRING(globalname);

	
	pTest = m_pList;
	while ( pTest )
	{
		if ( FStrEq( pEntityName, pTest->name ) )
			break;
	
		pTest = pTest->pNext;
	}

	return pTest;
}


// This is available all the time now on impulse 104, remove later
//#ifdef _DEBUG
void CGlobalState::DumpGlobals( void )
{
	static char *estates[] = { "Off", "On", "Dead" };
	globalentity_t *pTest;

	ALERT( at_console, "-- Globals --\n" );
	pTest = m_pList;
	while ( pTest )
	{
		ALERT( at_console, "%s: %s (%s)\n", pTest->name, pTest->levelName, estates[pTest->state] );
		pTest = pTest->pNext;
	}
}
//#endif


void CGlobalState::EntityAdd( string_t globalname, string_t mapName, GLOBALESTATE state )
{
	ASSERT( !Find(globalname) );

	globalentity_t *pNewEntity = (globalentity_t *)calloc( sizeof( globalentity_t ), 1 );
	ASSERT( pNewEntity != NULL );
	pNewEntity->pNext = m_pList;
	m_pList = pNewEntity;
	strcpy( pNewEntity->name, STRING( globalname ) );
	strcpy( pNewEntity->levelName, STRING(mapName) );
	pNewEntity->state = state;
	m_listCount++;
}


void CGlobalState::EntitySetState( string_t globalname, GLOBALESTATE state )
{
	globalentity_t *pEnt = Find( globalname );

	if ( pEnt )
		pEnt->state = state;
}


const globalentity_t *CGlobalState::EntityFromTable( string_t globalname )
{
	globalentity_t *pEnt = Find( globalname );

	return pEnt;
}


GLOBALESTATE CGlobalState::EntityGetState( string_t globalname )
{
	globalentity_t *pEnt = Find( globalname );
	if ( pEnt )
		return pEnt->state;

	return GLOBAL_OFF;
}


// Global Savedata for Delay
TYPEDESCRIPTION	CGlobalState::m_SaveData[] = 
{
	DEFINE_FIELD( CGlobalState, m_listCount, FIELD_INTEGER ),

	//MODDD - saving these for unpacking.
	DEFINE_FIELD( CGlobalState, m_i_monsterIDLatest, FIELD_INTEGER ),
	DEFINE_FIELD( CGlobalState, m_i_FuncTrackChangeIDLatest, FIELD_INTEGER ),
	DEFINE_FIELD( CGlobalState, m_i_PathTrackIDLatest, FIELD_INTEGER ),
	DEFINE_FIELD( CGlobalState, m_i_scriptedIDLatest, FIELD_INTEGER ),
};

// Global Savedata for Delay
TYPEDESCRIPTION	gGlobalEntitySaveData[] = 
{
	DEFINE_ARRAY( globalentity_t, name, FIELD_CHARACTER, 64 ),
	DEFINE_ARRAY( globalentity_t, levelName, FIELD_CHARACTER, 32 ),
	DEFINE_FIELD( globalentity_t, state, FIELD_INTEGER ),
};


int CGlobalState::Save( CSave &save )
{
	int i;
	globalentity_t *pEntity;

	
	//MODDD - new. Needs to happen before saving so that the instance vars are updated in time to reach the written data.
	SaveDynamicIDs(this);


	if ( !save.WriteFields( "GLOBAL", this, m_SaveData, ARRAYSIZE(m_SaveData) ) )
		return 0;
	
	pEntity = m_pList;
	for ( i = 0; i < m_listCount && pEntity; i++ )
	{
		if ( !save.WriteFields( "GENT", pEntity, gGlobalEntitySaveData, ARRAYSIZE(gGlobalEntitySaveData) ) )
			return 0;

		pEntity = pEntity->pNext;
	}


	

	return 1;
}

int CGlobalState::Restore( CRestore &restore )
{
	int i, listCount;
	globalentity_t tmpEntity;


	ClearStates();
	if ( !restore.ReadFields( "GLOBAL", this, m_SaveData, ARRAYSIZE(m_SaveData) ) )
		return 0;

	
	//MODDD - new. Needs to happen after loading so that they are available for reading from instance data.
	RestoreDynamicIDs(this);



	
	listCount = m_listCount;	// Get new list count
	m_listCount = 0;				// Clear loaded data

	for ( i = 0; i < listCount; i++ )
	{
		if ( !restore.ReadFields( "GENT", &tmpEntity, gGlobalEntitySaveData, ARRAYSIZE(gGlobalEntitySaveData) ) )
			return 0;
		EntityAdd( MAKE_STRING(tmpEntity.name), MAKE_STRING(tmpEntity.levelName), tmpEntity.state );
	}

	

	return 1;
}

void CGlobalState::EntityUpdate( string_t globalname, string_t mapname )
{
	globalentity_t *pEnt = Find( globalname );

	if ( pEnt )
		strcpy( pEnt->levelName, STRING(mapname) );
}


void CGlobalState::ClearStates( void )
{
	globalentity_t *pFree = m_pList;
	while ( pFree )
	{
		globalentity_t *pNext = pFree->pNext;
		free( pFree );
		pFree = pNext;
	}
	Reset();
}


void SaveGlobalState( SAVERESTOREDATA *pSaveData )
{
	CSave saveHelper( pSaveData );
	gGlobalState.Save( saveHelper );
}


void RestoreGlobalState( SAVERESTOREDATA *pSaveData )
{
	CRestore restoreHelper( pSaveData );
	gGlobalState.Restore( restoreHelper );
}


// BEWARE! This does not get called on going between map transitions.
void ResetGlobalState( void )
{
	gGlobalState.ClearStates();
	gInitHUD = TRUE;	// Init the HUD on a new game / load game

	//MODDD - call this event.
	OnMapLoadPreStart();

}

// moved CWorld class definition to cbase.h
//=======================
// CWorld
//
// This spawns first when each level begins.
//=======================

LINK_ENTITY_TO_CLASS( worldspawn, CWorld );



CWorld::CWorld(void){

	//If never specified, the defaults remain as such.  They are to be applied on restoring.
	m_fl_node_linktest_height = 8;
	m_fl_node_hulltest_height = 8;
	m_f_node_hulltest_heightswap = FALSE;

	m_f_map_anyAirNodes = FALSE;

	skyboxEverSet = FALSE;

	m_f_playerDeadTruce = FALSE;

}//END OF CWorld constructor





//WARNING - this never gets called it seems!
void CWorld::Activate(void){
	CBaseEntity::Activate();
}


void CWorld::Spawn( void )
{


	applyLoadedCustomMapSettingsToGlobal();

	
	//screw this, no good.
	//SetThink(&CWorld::WorldThink );
	//pev->nextthink = gpGlobals->time + 0.1;

	/////////////////////////////////////////////////////

	//this update is not particular to a player.
	//updateCVarRefs();
	//NOTE: best do the early update in client.cpp @ "ServerActivate" instead.  Seems to occur earlier than this.
	//UPDATE: "GameDLLInit" method of game.cpp deemed the earliest point best for initial CVar update.


	/////////////////////////////////////////////////////
	


	//MODDD
	///////////////////////////////////////////////////////////////////////////////////////////////
	// Those rancid devs hardcoded the engine to do "DispatchKeyword" calls before
	// absolutely anything else.  Those dire bastards.
	// Instead going to assume all the keywords are done by the time of the "Spawn" call.
	// As this is in "Spawn", this does not apply to loading a saved game.
	// Skybox is remembered just fine between games.
	if (!skyboxEverSet) {
		// Point is, if the skybox wasn't set by some keyword, force it to 'desert' here.
		CVAR_SET_STRING("sv_skyname", "desert");
	}

	// until proven othewise.    ...for next time of course.
	skyboxEverSet = FALSE;
	///////////////////////////////////////////////////////////////////////////////////////////////


	g_fGameOver = FALSE;
	Precache( );
	//MODDD - removed.
	//g_flWeaponCheat = CVAR_GET_FLOAT( "sv_cheats" );  // Is the impulse 101 command allowed?
	//handled in the client instead now.
}



//MODDD NOTE - what are the "room_type" and "waterroom_type" CVars for? are they dummied out?
// same for "v_dark" ?
// Nope, v_dark is used, but only applies on loading a map, not restored games.  Even before 
// spawnflags were cleared after map startup (For loaded games, it was set to 1 but still had
// no effect.  Odd.).

void CWorld::Precache( void )
{
	//MODDD - old place for startup.
	
	easyForcePrintLine("---CWORLD::PRECACHE---");
	

	//uh, whut??
	//ALERT ( at_console, "MAP PRECACHE CALLED\n" );
	//SetThink(&CWorld::worldThink);

	OnMapLoadStart();

	g_pLastSpawn = NULL;
	
#if 1
	CVAR_SET_STRING("sv_gravity", "800"); // 67ft/sec
	CVAR_SET_STRING("sv_stepsize", "18");

	//MODDD - another default.  Why wasn't this always this way?? This puts the camera behind you in thirdperson instead of... akwardly off to looking at the player model from the side.
	CVAR_SET_STRING("cam_idealyaw", "0");

#else
	CVAR_SET_STRING("sv_gravity", "384"); // 32ft/sec
	CVAR_SET_STRING("sv_stepsize", "24");
#endif

	CVAR_SET_STRING("room_type", "0");// clear DSP

	// Set up game rules
	if (g_pGameRules)
	{
		delete g_pGameRules;
	}

	g_pGameRules = InstallGameRules( );

	//!!!UNDONE why is there so much Spawn code in the Precache function? I'll just keep it here 

	///!!!LATER - do we want a sound ent in deathmatch? (sjb)
	//pSoundEnt = CBaseEntity::Create( "soundent", g_vecZero, g_vecZero, edict() );
	pSoundEnt = GetClassPtr( ( CSoundEnt *)NULL );
	pSoundEnt->Spawn();

	if ( !pSoundEnt )
	{
		ALERT ( at_console, "**COULD NOT CREATE SOUNDENT**\n" );
	}

	InitBodyQue();
	
// init sentence group playback stuff from sentences.txt.
// ok to call this multiple times, calls after first are ignored.

	SENTENCEG_Init();

// init texture type array from materials.txt

	TEXTURETYPE_Init();


// the area based ambient sounds MUST be the first precache_sounds


	//INCREDIBLE PRECACHE CALL
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	
	//MODDD - W_Precache and ClientPrecache moved to util.cpp.
// player precaches     
	//W_Precache ();	// get weapon precaches
	//ClientPrecache();
	
	//MODDD - call this method so that ones usually non-native (not included) to a map can be spawned by the player ("give") without crashing the game.
	method_precacheAll();
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	//~Some other precache junk moved to PreacheAll.
	
	
	//MODDD - lightsetup moved, see "turnWorldLightsOn" of util.cpp.
	// Also, on startup, 'forceWorldLightOff' can influence whether the lights
	// start on or off.

	BOOL prevQueue = g_queueCVarHiddenSave;
	if(EASY_CVAR_GET_DEBUGONLY(forceWorldLightOff) != 1){
		turnWorldLightsOn();
	}else{
		turnWorldLightsOff();
	}
	forceWorldLightOffMem = EASY_CVAR_GET_DEBUGONLY(forceWorldLightOff);
	// This call won't save
	g_queueCVarHiddenSave = prevQueue;


	// WHYYY won't ARRAYSIZE(gDecals) work just because it was moved to decals.cpp?? Damn you C++...
	for ( int i = 0; i < DLL_DECALLIST_SIZE; i++ ){
		gDecals[i].index = DECAL_INDEX( gDecals[i].name );

		if(EASY_CVAR_GET_DEBUGONLY(hiddenMemPrintout) == 1){
			easyForcePrintLine("DECAL DERIVATION: %s %d", gDecals[i].name, gDecals[i].index);
		}
	}

	//Keep track of "scheduleNodeUpdate", because it's about to get reset by InitGraph below.
	BOOL scheduleNodeUpdate_tempmem = scheduleNodeUpdate;

// init the WorldGraph.
	WorldGraph.InitGraph();

// make sure the .NOD file is newer than the .BSP file.
	//MODDD - NEW.  If the user wants to force the map to rebuild this time, go ahead.
	//if ( !WorldGraph.CheckNODFile ( ( char * )STRING( gpGlobals->mapname ) ) )
	if ( scheduleNodeUpdate_tempmem || !WorldGraph.CheckNODFile ( ( char * )STRING( gpGlobals->mapname ) ) )
	{// NOD file is not present, or is older than the BSP file.
		WorldGraph.AllocNodes ();
	}
	else
	{// Load the node graph for this level
		if ( !WorldGraph.FLoadGraph ( (char *)STRING( gpGlobals->mapname ) ) )
		{// couldn't load, so alloc and prepare to build a graph.
			ALERT ( at_console, "*Error opening .NOD file\n" );
			WorldGraph.AllocNodes ();
		}
		else
		{
			ALERT ( at_console, "\n*Graph Loaded!\n" );

			//MODDD - if we found any air nodes, need to apply that to here.
			//...nevermind, we do this anyways at util.cpp's OnMapLoadEnd regardless of loading from a file or not.
			//getCustomMapSettingsFromGlobal();
		}
	}

	if (pev->speed > 0) {
		CVAR_SET_FLOAT("sv_zmax", pev->speed);
	}
	else {
		CVAR_SET_FLOAT("sv_zmax", 4096);
	}

	if ( pev->netname )
	{
		ALERT( at_aiconsole, "Chapter title: %s\n", STRING(pev->netname) );
		CBaseEntity *pEntity = CBaseEntity::Create( "env_message", g_vecZero, g_vecZero, NULL );
		if ( pEntity )
		{
			pEntity->SetThink( &CBaseEntity::SUB_CallUseToggle );
			pEntity->pev->message = pev->netname;
			pev->netname = 0;
			pEntity->pev->nextthink = gpGlobals->time + 0.3;
			pEntity->pev->spawnflags = SF_MESSAGE_ONCE;
		}
	}

	if (pev->spawnflags & SF_WORLD_DARK) {
		CVAR_SET_FLOAT("v_dark", 1.0);
	}
	else {
		CVAR_SET_FLOAT("v_dark", 0.0);
	}

	if (pev->spawnflags & SF_WORLD_TITLE) {
		gDisplayTitle = TRUE;		// display the game title if this key is set
	}
	else {
		gDisplayTitle = FALSE;
	}

	//MODDD - no need for games saved on this map to show the title again.
	pev->spawnflags &= ~(SF_WORLD_DARK | SF_WORLD_TITLE);


	if ( pev->spawnflags & SF_WORLD_FORCETEAM )
	{
		CVAR_SET_FLOAT( "mp_defaultteam", 1 );
	}
	else
	{
		CVAR_SET_FLOAT( "mp_defaultteam", 0 );
	}

}//END OF Precache



//
// Just to ignore the "wad" field.
//
void CWorld::KeyValue( KeyValueData *pkvd )
{

	//node_linktest_height = 14;
	//node_hulltest_heightswap = TRUE;
	
	//MODDD - new to be set by the map potentially.
	if ( FStrEq(pkvd->szKeyName, "node_linktest_height") )
	{
		m_fl_node_linktest_height = (float)atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	if ( FStrEq(pkvd->szKeyName, "node_hulltest_height") )
	{
		m_fl_node_hulltest_height = (float)atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	if ( FStrEq(pkvd->szKeyName, "node_hulltest_heightswap") )
	{
		m_f_node_hulltest_heightswap = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	


	if ( FStrEq(pkvd->szKeyName, "skyname") )
	{
		// Sent over net now.
		CVAR_SET_STRING( "sv_skyname", pkvd->szValue );


		// MODDD - record whether the skybox was specified in load properties.
		// If not, enforce the default.  Stops say, crossfire.bsp from taking
		// the skybox of whatever map was previously loaded.
		// If any of our maps rely on using a previous map's skybox without setting
		// one itself, this feature should be removed.
		if (pkvd->szValue != NULL && strlen(pkvd->szValue) > 0) {
			// this counts.
			skyboxEverSet = TRUE;
		}

		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq(pkvd->szKeyName, "sounds") )
	{
		gpGlobals->cdAudioTrack = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq(pkvd->szKeyName, "WaveHeight") )
	{
		// Sent over net now.
		pev->scale = atof(pkvd->szValue) * (1.0/8.0);
		pkvd->fHandled = TRUE;
		CVAR_SET_FLOAT( "sv_wateramp", pev->scale );
	}
	else if ( FStrEq(pkvd->szKeyName, "MaxRange") )
	{
		pev->speed = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq(pkvd->szKeyName, "chaptertitle") )
	{
		pev->netname = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq(pkvd->szKeyName, "startdark") )
	{
		// UNDONE: This is a gross hack!!! The CVAR is NOT sent over the client/sever link
		// but it will work for single player
		int flag = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
		if ( flag )
			pev->spawnflags |= SF_WORLD_DARK;
	}
	else if ( FStrEq(pkvd->szKeyName, "newunit") )
	{
		// Single player only.  Clear save directory if set
		if ( atoi(pkvd->szValue) )
			CVAR_SET_FLOAT( "sv_newunit", 1 );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq(pkvd->szKeyName, "gametitle") )
	{
		if ( atoi(pkvd->szValue) )
			pev->spawnflags |= SF_WORLD_TITLE;

		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq(pkvd->szKeyName, "mapteams") )
	{
		pev->team = ALLOC_STRING( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq(pkvd->szKeyName, "defaultteam") )
	{
		if ( atoi(pkvd->szValue) )
		{
			pev->spawnflags |= SF_WORLD_FORCETEAM;
		}
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue( pkvd );
}


GENERATE_TRACEATTACK_IMPLEMENTATION(CWorld){

	GENERATE_TRACEATTACK_PARENT_CALL(CBaseEntity);

	// is the attacker a player?
	if(pevAttacker != NULL){
		CBaseEntity* tempEnt = CBaseEntity::Instance(pevAttacker);
		if(tempEnt!=NULL) {
			if(FClassnameIs(tempEnt->pev, "player") && EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(playerBulletHitEffectForceServer) == 0 ){
				// don't allow. Players already play sounds clientside.
				if(useBulletHitSound){
					*useBulletHitSound = FALSE;
				}
			}
		}
	}
}


GENERATE_TAKEDAMAGE_IMPLEMENTATION(CWorld){

	return GENERATE_TAKEDAMAGE_PARENT_CALL(CBaseEntity);
}



TYPEDESCRIPTION	CWorld::m_SaveData[] = 
{
	DEFINE_FIELD( CWorld, m_fl_node_linktest_height, FIELD_FLOAT ),
	DEFINE_FIELD( CWorld, m_fl_node_hulltest_height, FIELD_FLOAT ),
	DEFINE_FIELD( CWorld, m_f_node_hulltest_heightswap, FIELD_BOOLEAN ),
	DEFINE_FIELD( CWorld, m_f_map_anyAirNodes, FIELD_BOOLEAN ),
	DEFINE_FIELD( CWorld, m_f_playerDeadTruce, FIELD_BOOLEAN ),
	
};


//IMPLEMENT_SAVERESTORE( CWorld, CBaseEntity );
int CWorld::Save( CSave &save )
{
	if ( !CBaseEntity::Save(save) )
		return 0;

	// keep me in synch
	// (complement to 'applyLoadedCustomMapSettingsToGlobal' for saving pending if
	//  anything else like this is needed)
	m_f_playerDeadTruce = g_f_playerDeadTruce;

	int saveFieldResult = save.WriteFields( "CWorld", this, m_SaveData, ARRAYSIZE(CWorld::m_SaveData) );

	return saveFieldResult;
}
int CWorld::Restore( CRestore &restore )
{
	if ( !CBaseEntity::Restore(restore) )
		return 0;

	int readFieldsResult = restore.ReadFields( "CWorld", this, m_SaveData, ARRAYSIZE(CWorld::m_SaveData) );
	
	//Plug in what I loaded:
	applyLoadedCustomMapSettingsToGlobal();

	//no good.
	//SetThink(&CWorld::WorldThink );
	//pev->nextthink = gpGlobals->time + 0.1;

	return readFieldsResult;
}

/*
void CWorld::WorldThink(void){

	easyForcePrintLine("I AM THE SUPER DUPER MAN");

	::UTIL_drawLineFrame(0, 0, 0, 500, 500, 500, 12, 255, 0, 0);

	
	pev->nextthink = gpGlobals->time + 0.1;

	//nope, redundant then.
	//CBaseEntity::Think();
}//END OF WorldThink
*/


//MODDD - load new settings / variables / config / whatever saved per map so that it may take effect if necessary.
//For instance, nodes don't know how to access the map so these global variables can be modified on loading a new map or restoring an existing
//so that the values at the time of that map's creation per the user's save file are used.
void CWorld::applyLoadedCustomMapSettingsToGlobal(void){
	
	// Clearly these need to take effect.
	node_linktest_height = m_fl_node_linktest_height;
	node_hulltest_height = m_fl_node_hulltest_height;
	node_hulltest_heightswap = m_f_node_hulltest_heightswap;

	g_f_playerDeadTruce = m_f_playerDeadTruce;

}//END Of applyLoadedCustomMapSettingsToGlobal


//MODDD - for hte other way around.  Settings set by nodes.cpp should be sent to here instead.
void CWorld::getCustomMapSettingsFromGlobal(void){
	
	m_f_map_anyAirNodes = map_anyAirNodes;
}


BOOL CWorld::IsWorld(){
	return TRUE;  //Why yes, yes I am the world, thank you for asking.
}
BOOL CWorld::IsWorldAffiliated(){
	return FALSE; //Just saying "no" for things that only want the map and not say, ladders.
}



