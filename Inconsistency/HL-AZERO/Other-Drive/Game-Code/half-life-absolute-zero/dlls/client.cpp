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
// Robin, 4-22-98: Moved set_suicide_frame() here from player.cpp to allow us to 
//				   have one without a hardcoded player.mdl in tf_client.cpp

/*

===== client.cpp ========================================================

  client/server game specific stuff

*/


// TODO.  How about node type filtering (_AIR, _GROUND, _WATER) for 'FindNearestNode' debug calls?
// Let them be specified by another parameter, as typed "air", "ground", or "water". if text,
// or for those as bits if a number (7 is 111 in binary, all three choices)



#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "saverestore.h"
#include "player.h"
#include "spectator.h"
#include "client.h"
#include "soundent.h"
#include "gamerules.h"
#include "game.h"
#include "customentity.h"
#include "weapons.h"
#include "weaponinfo.h"
#include "usercmd.h"
#include "netadr.h"
//MODDD
#include "scripted.h"
#include "squadmonster.h"
#include "nodes.h"
#include "util_debugdraw.h"
#include "rpg.h"
//MODDD - moved from below to here for neatness.
#include "voice_gamemgr.h"
#include "entity_state.h"
#include "trains.h"
#include "talkmonster.h"
#include "util_version.h"


#include "cvar_custom_info.h"
#include "cvar_custom_list.h"


//MODDD
extern float cheat_barnacleEatsEverything;
extern float globalPSEUDO_cameraMode;
extern BOOL globalPSEUDO_queueClientSendoff;
extern int global_useSentenceSave;
extern BOOL queueYMG_stopSend;
extern cvar_t* global_test_cvar_ref;


EASY_CVAR_EXTERN(pausecorrection2)


extern CGraph WorldGraph;







//MODDD - cheat CVAR storage.
/*
cvar_t* clientCheat_infiniteclip = 0;
cvar_t* clientCheat_infiniteammo = 0;
cvar_t* clientCheat_minimumfiredelay = 0;

//cvar_t* cvar_minimumfiredelaymem = 0;
*/

//extern cvar_t* cvar_sv_cheats;
extern float g_flWeaponCheat;
//old voice_gamemgr.h include location.
extern CVoiceGameMgr g_VoiceGameMgr;

extern BOOL queueSkillUpdate;
extern float cvar_skill_mem;



extern DLL_GLOBAL ULONG g_ulModelIndexPlayer;
extern DLL_GLOBAL BOOL g_fGameOver;
extern DLL_GLOBAL int g_iSkillLevel;
extern DLL_GLOBAL ULONG g_ulFrameCount;

extern BOOL g_firstPlayerEntered;
extern BOOL g_queueCVarHiddenSave;

extern CBaseMonster* g_routeTempMonster;
extern CBaseEntity* g_routeTempMonster_GoalEnt;


extern void CopyToBodyQue(entvars_t* pev);


//No.  For once, something here in client.cpp is to be externed elsewhere instead.
//extern void resetModCVars(CBasePlayer* arg_plyRef, BOOL isEmergency);


EASY_CVAR_EXTERN_MASS




static int g_serveractive = 0;

CBaseEntity* crashableEntityRef = NULL;
float crashableEntityReferTime = 0;
int crashableEntityReferMode = 0;

float globalPSEUDO_minimumfiredelaymem = -1;
////////////////////////////////////////////////////////////////////////////////////////////////////////

//MODDD TODO - this should just get copied to methods that need it, may only just be one. Or just put it in each one and make it static? eh.
char queuedSound[127];
BOOL playQueued = FALSE;
float playQueuedTime = -1;
int playQueuedPitch = 100;
// And what player is using the soundtest?
EHANDLE playedSoundPlayer;


char changeLevelQueuedName[64];
char changeLevelQueuedSpot[64];
float changeLevelQueuedTime = -1;




// sp, for "singleplayer".  Linked to the previous time 'PlayerPreThink' was called.
float sp_previousFrameTime;
// Only referred to for detecting single-player pauses, not multiplayer.
BOOL sp_playerCanPreThink = FALSE;
BOOL sp_playerCanPostThink = FALSE;

BOOL g_alreadyShownGameloadedMessage = FALSE;



// For the whole server.  Linked to the previous time 'StartFrame' was called.
float g_previousFrameTime;







//test for what crashes.
class CrashTest{
	public:
	void someMethod(){
		int x = 6;
	}
};//END OF CrashTest class




/*
typedef struct testStruct_s{
	int eckz;
	const char* someStr;
} testStruct_t;

testStruct_t aryStruct[] = { {5, "text"}, {7, "abc"}, {-16, "xyz"} };
*/


// (RELEASE ONLY.  No need for recording info about hidden CVars when nothing is hidden)
#ifndef _DEBUG

// Small class for recording info about each var that needs hidden access granted (from not having
// a CVar in release mode).
// An array of this type is made and checked against here in client.cpp for checking to see if what
// the user typed in is a hidden CVar.  Then its value is returned (typed a name only) or the CVar's
// value is set to the 2nd parameter.  Any broadcasting or serverside cache setting (global_...) is done.
typedef struct HiddenCVarInfo_s{
	const char* name;
	const char* nameLowercase;
	// What index is this CVar in clientside's cache?
	int dataID;
	// optional (can be NULL) for the CLIENTONLY_DEBUGONLY type.
	// Pointer to the linked 'global_' version that holds this var's value serverside.
	float* serverCacheVar;
	// Describes some other details in how to send this to the client (broadcasted?  clientonly?).
	int type;
} HiddenCVarInfo_t;

//typedef HiddenCVarInfo_t;

// IMPORTANT!  Keep HIDDEN_CVAR_INFO_LENGTH in check with the number of entries in it
// (cvar_custom_list.h) change.
//#define HIDDEN_CVAR_INFO_LENGTH 446

// Wait, how about this?
//#define HIDDEN_CVAR_INFO_LENGTH		(sizeof(aryHiddenCVarInfo)/sizeof(aryHiddenCVarInfo[0]))
// TEST.  How about this?
#define HIDDEN_CVAR_INFO_LENGTH		(sizeof(aryHiddenCVarInfo)/sizeof(HiddenCVarInfo_t))


HiddenCVarInfo_t aryHiddenCVarInfo[] = {
	// example of how an entry might look (some of these are CVar constants that derive to numbers)
	//{"someCVar", "somecvar", someCVar_ID, 0, &global_someCVar }
	EASY_CVAR_HIDDEN_LIST
};

#endif//_!DEBUG





// TEST:  What of this will compile in VS6?
// (tested: this section uncommented out by removing the
//  asterisk section starting below)
///////////////////////////////////////////////////////////
/*
namespace SomeEnumNamespace{
	enum SomeEnum{
		ABC,
		DEF,
		GHI
	};
}
namespace SomeUnnamedEnumNamespace{
	enum{
		JKL,
		MNO,
		PQR
	};
}


void TEST_ENUM_NAME(void){
	// No other combos are possible.  Any reference to anything in the namespace
	// must start with the namespace, and using the namespace alone isn't
	// specific enough to tell "SomeEnum" was wanted for the type.
	// But for getting an enum choice of SomeEnum,  referring to it without
	// involving 'SomeEnum' works fine.
	// The point is to get around the odd restriction of VS6 of disallowing
	// references to named enums.  Without the namespace above, lines like
	//     someVal = SomeEnum::ABC;
	// will lead to compile errors, while lines like
	//     someVal = ABC;
	// work fine.  What.  Why.

	// This way doesn't work in VS6.
	//SomeEnumNamespace::SomeEnum myEnum1 = SomeEnumNamespace::SomeEnum::ABC;
	
	// Both of these work!
	SomeEnumNamespace::SomeEnum myEnum2 = SomeEnumNamespace::ABC;
	// only way for the unnamed one.
	int myEnum3 = SomeUnnamedEnumNamespace::JKL;
}
*/
///////////////////////////////////////////////////////////






void debugNodeMode(void){
	EASY_CVAR_SET_DEBUGONLY(drawNodeAll, 230);
	//EASY_CVAR_SET_DEBUGONLY(drawNodeSpecial, -1);
	EASY_CVAR_SET_DEBUGONLY(drawNodeConnections, 2);
	//EASY_CVAR_SET_DEBUGONLY(drawNodeAlternateTime, 0);
	g_queueCVarHiddenSave = TRUE;
}
void debugNodeModeWeak(void){
	EASY_CVAR_SET_DEBUGONLY(drawNodeAll, 500);
	//EASY_CVAR_SET_DEBUGONLY(drawNodeSpecial, -1);
	EASY_CVAR_SET_DEBUGONLY(drawNodeConnections, 1);
	//EASY_CVAR_SET_DEBUGONLY(drawNodeAlternateTime, 0);
	g_queueCVarHiddenSave = TRUE;
}
void debugNodeModeOff(void){
	EASY_CVAR_SET_DEBUGONLY(drawNodeAll, -1);
	EASY_CVAR_SET_DEBUGONLY(drawNodeSpecial, -1);
	EASY_CVAR_SET_DEBUGONLY(drawNodeConnections, 0);
	EASY_CVAR_SET_DEBUGONLY(drawNodeAlternateTime, 0);
	g_queueCVarHiddenSave = TRUE;
}


BOOL attemptParseStringToInt(edict_t* pEntity, int* toStoreResult, const char* toRead, const char* errorMsg_badInput, const char* errorMsg_noInput){
	
	if(toRead != NULL && !isStringEmpty(toRead)){
		//try parsing it.
		try{
			int numbAttempt = tryStringToInt(toRead);
			*toStoreResult = numbAttempt;
			return TRUE;
		}catch(int){
			easyForcePrintLineClient(pEntity, errorMsg_badInput);
			return FALSE;
		}
	}else{
		easyForcePrintLineClient(pEntity, errorMsg_noInput);
		return FALSE;
	}//END OF arg check

	return FALSE;  //how could this be reached??
}//END OF attemptParseStringToInt




CPathTrack* getPathTrackWithID(edict_t* pEntity, int argID){
	//try to find it.
	CBaseEntity* pEntityScan = NULL;
	while( (pEntityScan = UTIL_FindEntityByClassname(pEntityScan, "path_track")) != NULL){
		CPathTrack* pathTrackRef = static_cast<CPathTrack*>(pEntityScan);
		if(pathTrackRef != NULL && pathTrackRef->PathTrackID == argID){
			//found it!
			return pathTrackRef;
			break;
		}

	}//END OF while loop

	//printout alongside this.
	easyForcePrintLineClient(pEntity, "path_track of ID %d not found.", argID);
	return NULL;
}//END OF getPathTrackWithID


CFuncTrackChange* getTrackChangeWithID(edict_t* pEntity, int argID){
	//try to find it.
	CBaseEntity* pEntityScan = NULL;
	while( (pEntityScan = UTIL_FindEntityByClassname(pEntityScan, "func_trackchange")) != NULL){
		CFuncTrackChange* trackChangeRef = static_cast<CFuncTrackChange*>(pEntityScan);
		if(trackChangeRef != NULL && trackChangeRef->FuncTrackChangeID == argID){
			//found it!
			return trackChangeRef;
			break;
		}

	}//END OF while loop

	//printout alongside this.
	easyForcePrintLineClient(pEntity, "func_trackchange of ID %d not found.", argID);
	return NULL;

}//END OF getTrackChangeWithID




CBaseEntity* getEntityWithID(int argSeekID){
	edict_t* pEdict = g_engfuncs.pfnPEntityOfEntIndex(argSeekID);
	if(pEdict != NULL && !pEdict->free){
		CBaseEntity* theInst = CBaseEntity::Instance(pEdict);
		//BOOL hasMonsterClip = ((theInst->pev->flags & FL_MONSTERCLIP) != 0);
		return theInst;
	}else{
		// nothing or invalid.
		return NULL;
	}
}


CBaseMonster* getMonsterWithID(int argSeekID){
	//find monster ID of argSeekID.
	edict_t		*pEdict = g_engfuncs.pfnPEntityOfEntIndex( 1 );
	CBaseEntity *pEntity;
	if ( !pEdict ){
		return NULL;
	}

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

		if(tempMonster->monsterID == argSeekID){
			// go!
			return tempMonster;
			//break;  redundant, "return" already interrupts.
		}

	}//END OF through all entities.


	//did not find it?
	return NULL;
}//getMonsterWithID


void reviveAllMonsters(edict_t* theCaller) {
	edict_t* pEdict = g_engfuncs.pfnPEntityOfEntIndex(1);
	CBaseEntity* pTempEntity;
	if (!pEdict)
		return;
	for (int i = 1; i < gpGlobals->maxEntities; i++, pEdict++)
	{
		if (pEdict->free)	// Not in use
			continue;
		if (!(pEdict->v.flags & (FL_CLIENT | FL_MONSTER)))	// Not a client/monster ?
			continue;

		pTempEntity = CBaseEntity::Instance(pEdict);
		if (!pTempEntity)
			continue;

		CBaseMonster* tempMonster = pTempEntity->MyMonsterPointer();
		if (tempMonster == NULL || FClassnameIs(tempMonster->pev, "player")) {
			continue;  //not players or non-monsters.
		}

		// try to revive.
		if (tempMonster->pev->deadflag >= DEAD_DEAD) {
			//ok

			easyForcePrintStarter();
			easyForcePrint("*REVIVED ");
			printBasicEntityInfo(theCaller, pTempEntity);
			easyForcePrintLine();  // new line

			tempMonster->StartReanimation();
		}
	}//END OF through all entities.

}//reviveAllMonsters


void removeAllMonsters(edict_t* theCaller) {
	edict_t* pEdict = g_engfuncs.pfnPEntityOfEntIndex(1);
	CBaseEntity* pTempEntity;
	if (!pEdict)
		return;
	for (int i = 1; i < gpGlobals->maxEntities; i++, pEdict++)
	{
		if (pEdict->free)	// Not in use
			continue;


		if (!(pEdict->v.flags & (FL_CLIENT | FL_MONSTER)))	// Not a client/monster ?
			continue;

		pTempEntity = CBaseEntity::Instance(pEdict);
		if (!pTempEntity)
			continue;

		CBaseMonster* tempMonster = pTempEntity->MyMonsterPointer();
		if (tempMonster == NULL || FClassnameIs(tempMonster->pev, "player")) {
			continue;  //not players or non-monsters.
		}

		if(tempMonster->monsterID >= 9000){
			// this is a cine, unaffected.
			continue;
		}

		easyForcePrintStarter();
		easyForcePrint("*REMOVED ");
		printBasicEntityInfo(theCaller, pTempEntity);
		easyForcePrintLine();  // new line

		//made it here? Remove it.
		//::UTIL_Remove(tempMonster);
		//tempMonster->onDelete();   automatically called by SUB_REMOVE, don't manually call this.
		tempMonster->SetThink(&CBaseEntity::SUB_Remove);
		tempMonster->pev->nextthink = gpGlobals->time;
	}//END OF list through all entities.
}

void removeAllMonstersExcept(edict_t* theCaller, int excludeID) {
	edict_t* pEdict = g_engfuncs.pfnPEntityOfEntIndex(1);
	CBaseEntity* pTempEntity;
	if (!pEdict)
		return;
	for (int i = 1; i < gpGlobals->maxEntities; i++, pEdict++)
	{
		if (pEdict->free)	// Not in use
			continue;

		//TEST WHY NO REMOV
		if (FClassnameIs(pEdict, "monster_barnacle")) {
			int x = 66; //?
		}

		// drop this requirement!  CSittingScientist lacks FL_MONSTER even though it is a CBaseMonster.
		// No idea if that was intentional.  But the "MyMonsterPointer" being non-NULL below is fine,
		// it is for them too
		// ...nevermind, restored.  CSittingScientist was the only weird one and several others
		// that should not be removed (scripted_sequence) were being included by removing this check.
		if (!(pEdict->v.flags & (FL_CLIENT | FL_MONSTER)))	// Not a client/monster ?
			continue;

		pTempEntity = CBaseEntity::Instance(pEdict);
		if (!pTempEntity)
			continue;

		CBaseMonster* tempMonster = pTempEntity->MyMonsterPointer();
		if (tempMonster == NULL || FClassnameIs(tempMonster->pev, "player")) {
			continue;  //not players or non-monsters.
		}

		if (tempMonster->monsterID == excludeID) {
			continue;  // remove all BUT this one!  Skip.
		}

		if(tempMonster->monsterID >= 9000){
			// this is a cine, unaffected.
			continue;
		}

		easyForcePrintStarter();
		easyForcePrint("*REMOVED ");
		printBasicEntityInfo(theCaller, pTempEntity);
		easyForcePrintLine();  // new line

		//made it here? Remove it.
		//::UTIL_Remove(tempMonster);
		//tempMonster->onDelete();   automatically called by SUB_REMOVE, don't manually call this.
		tempMonster->SetThink(&CBaseEntity::SUB_Remove);
		tempMonster->pev->nextthink = gpGlobals->time;
	}//END OF list through all entities.
}

void removeAllEntities(edict_t* theCaller) {
	edict_t* pEdict = g_engfuncs.pfnPEntityOfEntIndex(1);
	CBaseEntity* pTempEntity;
	if (!pEdict)
		return;
	for (int i = 1; i < gpGlobals->maxEntities; i++, pEdict++)
	{
		if (pEdict->free)	// Not in use
			continue;

		pTempEntity = CBaseEntity::Instance(pEdict);
		if (!pTempEntity)
			continue;

		if (FClassnameIs(pTempEntity->pev, "worldspawn") || FClassnameIs(pTempEntity->pev, "player")) {
			continue;  //not the map (???) or players.
		}
		
		easyForcePrintStarter();
		easyForcePrint("*REMOVED ");
		printBasicEntityInfo(theCaller, pTempEntity);
		easyForcePrintLine();  // new line

		//made it here? Remove it.
		////::UTIL_Remove(tempMonster);
		// do it this way below instead.
		//pTempEntity->onDelete();
		//pTempEntity->SetThink(&CBaseEntity::SUB_Remove);
		//pTempEntity->pev->nextthink = gpGlobals->time;

	}//END OF list through all entities.

}


void listAllEntities(edict_t* theCaller) {
	edict_t* pEdict = g_engfuncs.pfnPEntityOfEntIndex(1);
	CBaseEntity* pTempEntity;
	if (!pEdict)
		return;

	// Wait.  "pEdict++" really grabs the next entity just fine?  No need for say
	//     pEdict = g_engfuncs.pfnPEntityOfEntIndex(i)
	// each time?     huh.  imagine that.
	for (int i = 1; i < gpGlobals->maxEntities; i++, pEdict++)
	{
		if (pEdict->free)	// Not in use
			continue;

		pTempEntity = CBaseEntity::Instance(pEdict);
		if (!pTempEntity)
			continue;

		// exclude the world and the players, not helpful
		// no, be complete here.
		//if (FClassnameIs(pTempEntity->pev, "worldspawn") || FClassnameIs(pTempEntity->pev, "player")) {
		//	continue;
		//}

		// NOTICE!  Printing a huge volume of information this way just leads to buffer overflows and drops the player.
		// It's a debug feature anyway so just use the server print.
		easyForcePrintStarter();
		// ID
		easyForcePrint("%03d:", ENTINDEX(pEdict));
		//easyForcePrint("*Info: ");   no need
		printBasicEntityInfo(theCaller, pTempEntity);
		easyForcePrintLine();  // new line

	}//END OF list through all entities.

}

void listAllMonsters(edict_t* theCaller) {
	edict_t* pEdict = g_engfuncs.pfnPEntityOfEntIndex(1);
	CBaseEntity* pTempEntity;
	if (!pEdict)
		return;
	for (int i = 1; i < gpGlobals->maxEntities; i++, pEdict++)
	{
		if (pEdict->free)	// Not in use
			continue;

		//TEST WHY NO REMOV
		if (FClassnameIs(pEdict, "monster_barnacle")) {
			int x = 66; //?
		}

		// Nevermind this restriction, some monsters still lack FL_MONSTER.
		// Let the MyMonsterPointer being non-NULL be a better sign.
		//if (!(pEdict->v.flags & (FL_CLIENT | FL_MONSTER)))	// Not a client/monster ?
		//	continue;

		pTempEntity = CBaseEntity::Instance(pEdict);
		if (!pTempEntity)
			continue;

		CBaseMonster* tempMonster = pTempEntity->MyMonsterPointer();
		if (tempMonster == NULL || FClassnameIs(tempMonster->pev, "player")) {
			continue;  //not players or non-monsters.
		}

		easyForcePrintStarter();
		// ID
		easyForcePrint("%03d:", ENTINDEX(pEdict));
		//easyForcePrint("*Info: ");   no need
		printBasicEntityInfo(theCaller, pTempEntity);
		easyForcePrintLine();  // new line

	}//END OF list through all entities.
}




// NOTICE that "pEntity" and "arg_target" may not necessarily be the same.
// They are if we're getting the same local player's coords.  Otherwise, no.
void interpretAsCoords(edict_t* pEntity, CBaseEntity* arg_target, const char* arg_targetName){

	easyForcePrintLineClient(pEntity, "%s\'s Origin: (%.2f, %.2f, %.2f)\n%s\'s Angles: (%.2f, %.2f, %.2f)",
				arg_targetName,
				arg_target->pev->origin.x, arg_target->pev->origin.y, arg_target->pev->origin.z,
				arg_targetName,
				arg_target->pev->angles.x, arg_target->pev->angles.y, arg_target->pev->angles.z);

	/*
	easyForcePrintLineClient(pEntity, "Your coords:\nMy Origin: (%.2f, %.2f, %.2f)\nMy Angles: (%.2f, %.2f, %.2f)",
				tempplayer->pev->origin.x, tempplayer->pev->origin.y, tempplayer->pev->origin.z,
				tempplayer->pev->angles.x, tempplayer->pev->angles.y, tempplayer->pev->angles.z);

			easyForcePrintLineClient(pEntity, "%s\'s coords:\nOrigin: (%.2f, %.2f, %.2f)\nAngles: (%.2f, %.2f, %.2f)",
				STRING(pEntityForward->pev->classname),
				pEntityForward->pev->origin.x, pEntityForward->pev->origin.y, pEntityForward->pev->origin.z,
				pEntityForward->pev->angles.x, pEntityForward->pev->angles.y, pEntityForward->pev->angles.z);
	*/
}

// same case as above.
void interpretAsHealth(edict_t* pEntity, CBaseEntity* arg_target, const char* arg_arg1Ref, const char* arg_targetName ){

	if(isStringEmpty(arg_arg1Ref)){
		
		//if no argument was provided, we're fetching the stats of this entity.
		easyForcePrintLineClient(pEntity, "%s\'s health:%g maxHealth:%g deadflag:%d IsAlive:%d", arg_targetName, arg_target->pev->health, arg_target->pev->max_health, arg_target->pev->deadflag, arg_target->IsAlive() );
	}else{
		//set the entity's current health to the provided argument, if it is a whole number.
		
		if(checkMatchIgnoreCase(arg_arg1Ref, "full") ){
			//"full" is a keyword for the arg that means, set to max health.
			arg_target->pev->health = arg_target->pev->max_health;
		}else if(checkMatchIgnoreCase(arg_arg1Ref, "dead") || checkMatchIgnoreCase(arg_arg1Ref, "kill")  ){
			//keyword for dead.
			arg_target->pev->health = 0;
		}else{
			try{
				float numbAttempt = tryStringToFloat(arg_arg1Ref);

				if (pEntity == arg_target->edict() && arg_target->IsPlayer()) {
					// If I am setting my own health, and I'm a player, and the health is set above 0, I can revive.
					CBasePlayer* tempplayer = static_cast<CBasePlayer*>(arg_target);
					if (numbAttempt > 0) {
						tempplayer->reviveIfDead();
					}
				}

				arg_target->pev->health = numbAttempt;
				easyForcePrintLineClient(pEntity, "%s\'s health set to %g.", arg_targetName, numbAttempt);
			}catch(int){
				easyForcePrintLineClient(pEntity, "Problem reading number.  (arg must be whole number)");
			}
		}
	}

	/*
	CBaseEntity* pEntity = FindEntityForward( tempplayer );
	if ( pEntity )
	{
		if(isStringEmpty(arg1ref)){
			//if no argument was provided, we're fetching the stats of this entity.
			easyForcePrintLineClient(pEntity, "%s\nhealth: %d\nmaxHealth: %d", STRING(pEntity->pev->classname), pev->health, pev->max_health );
		}else{
			//set the entity's current health to the provided argument, if it is a whole number.
			try{
				int numbAttempt = tryStringToInt(arg1ref);
				pEntity->pev->health = numbAttempt;
				easyForcePrintLineClient(pEntity, "setHealth successful.");
			}catch(int err){
				easyForcePrintLineClient(pEntity, "Problem reading number.  (arg must be whole number)");
			}
		}
	}else{
		easyForcePrintLineClient(pEntity, "Could not find something to set / get health of!");
	}
	*/
}






// same case as above.
void interpretAsBattery(edict_t* pEntity, CBaseEntity* arg_target, const char* arg_arg1Ref, const char* arg_targetName) {

	if (isStringEmpty(arg_arg1Ref)) {
		//if no argument was provided, we're fetching the stats of this entity.
		easyForcePrintLineClient(pEntity, "%s\'s battery:%g, maxBattery:%g", arg_targetName, arg_target->pev->armorvalue, (float)MAX_NORMAL_BATTERY);
	}
	else {
		//set the entity's current battery to the provided argument, if it is a whole number.

		if (checkMatchIgnoreCase(arg_arg1Ref, "full")) {
			//"full" is a keyword for the arg that means, set to max battery.
			arg_target->pev->armorvalue = MAX_NORMAL_BATTERY;
		}
		else if (checkMatchIgnoreCase(arg_arg1Ref, "empty") || checkMatchIgnoreCase(arg_arg1Ref, "none")) {
			// 0
			arg_target->pev->armorvalue = 0;
		}
		else {
			try {
				float numbAttempt = tryStringToFloat(arg_arg1Ref);
				arg_target->pev->armorvalue = numbAttempt;
				easyForcePrintLineClient(pEntity, "%s\'s battery set to %g.", arg_targetName, numbAttempt);
			}
			catch (int) {
				easyForcePrintLineClient(pEntity, "Problem reading number.  (arg must be whole number)");
			}
		}
	}
}



//MODDD - new.
void resetPlayerRPGRockets(CBasePlayer* playerRef){
	if(playerRef){
		//...why is this here?!
		//playerRef->pev->flags &= ~FL_GODMODE;

		for(int i = 0; i < MAX_ITEM_TYPES; i++){
			CBasePlayerItem *pPlayerItem = playerRef->m_rgpPlayerItems[ i ];

			while ( pPlayerItem )
			{
				CBasePlayerWeapon* temp = (CBasePlayerWeapon*) pPlayerItem->GetWeaponPtr();
				
				if(temp != NULL && temp->m_iId == WEAPON_RPG){
					CRpg* temp2 = (CRpg *)temp;
					temp2->m_cActiveRockets = 0;
					return;
				}

				pPlayerItem = pPlayerItem->m_pNext;
			}//END OF while
		}//END OF for(...)
	}//END OF if(playerRef)
}//END OF resetPlayerRPGRockets



//The result of "fixcvars" or "unfuckup".
//NOTE: defined here instead of in combat.cpp (usual place for things to be extern'd out
//globally) because client.cpp is better for working with type "CBasePlayer" (more convenient)
void resetModCVars(CBasePlayer* arg_plyRef, BOOL isEmergency){
	
	entvars_t* pev = (arg_plyRef!=NULL)?arg_plyRef->pev:NULL;

	edict_t* pEntity = arg_plyRef->edict();


	BOOL minimumFireDelayWasOn = FALSE;
	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelaycustom) == 1){
		minimumFireDelayWasOn = TRUE;
	}
	
	globalPSEUDO_minimumfiredelaymem = 0;

	if(arg_plyRef != NULL){
		resetPlayerRPGRockets( arg_plyRef );
	}

	//if the barnacle cheat is on, turn it of.
	if(cheat_barnacleEatsEverything == 1){
		cheat_barnacleEatsEverything = 2;
		easyForcePrintLineBroadcast("CHEAT OFF: Barnacles returned to normal.");
	}
	
	EASY_CVAR_SET(cl_explosion, 0);
	EASY_CVAR_SET(pissedNPCs, 0);


	UTIL_ServerMassCVarReset(pev);


	if(!isEmergency){
		easyForcePrintLineClient(pEntity, "*** Remember to restart if a precache error has happend during this launch of the game***");
	}
	/*
	if(EASY_CVAR_GET(precacheAll) == 1){
		EASY_CVAR_GET(precacheAll, 0);
		easyForcePrintLineClient(pEntity, "***ALSO: precacheAll turned off.  Turn it back on if desired***");
	}
	*/

	if(arg_plyRef!=NULL && ( (arg_plyRef->pev->flags & FL_NOTARGET) || arg_plyRef->m_fNoPlayerSound == TRUE) ) {
		arg_plyRef->turnOffSneaky();

		if(EASY_CVAR_GET_DEBUGONLY(autoSneaky) != 0){
			EASY_CVAR_SET_DEBUGONLY(autoSneaky, 0);
			easyForcePrintLineClient(pEntity, "*** AI cheats (notarget / nosound) detected and turned off.  autoSneaky reset to 0.***");
		}else{
			easyForcePrintLineClient(pEntity, "*** AI cheats (notarget / nosound) detected and turned off.***");
		}
	}

	if(EASY_CVAR_GET_DEBUGONLY(itemBatteryPrerequisite) == 1){
		//if(!isEmergency){
			easyForcePrintLineClient(pEntity, "*** \"itemBatteryPrerequisite\" is on.  Remember to have at least one battery charge before expecting longjump, injectibles (besides adrenaline), and a few other things to work.***");
		//}
	}

	float temppppp = EASY_CVAR_GET(soundSentenceSave);

	easyForcePrintLineClient(pEntity, "*** soundSentenceSave is %d.  Read up about it if unsure of if this is appropriate (1 advised)***", (int)temppppp );

	float temppppp2 = EASY_CVAR_GET(precacheAll);

	if(temppppp == 1 && temppppp2 != 1){
		easyForcePrintLineClient(pEntity, "*** ~soundSentenceSave is most effective when paired with \"precacheAll = 1\".***");
	}
	if(temppppp != 1 && temppppp2 == 1){
		easyForcePrintLineClient(pEntity, "*** ~precacheAll is most effective when paired with \"soundSentenceSave = 1\".***");
	}
	easyForcePrintLineClient(pEntity, "*** use \'fixprecache\' to turn both of these on.***");

		
	if(minimumFireDelayWasOn){
		easyForcePrintLineClient(pEntity, "*** Minimum fire turned off, corresponding custom fire rate unaffected.  To control the fire rate of all weapons with \"minimum fire\", adjust \"cheat_minimumfiredelaycustom\"***");
	}

	if(isEmergency){
		easyForcePrintLineClient(pEntity, "!!! IMPORTANT !!!");
		easyForcePrintLineClient(pEntity, "CVar \"emergencyFix\" was activated last time.  Fix successfully applied.");
	}

	//reset kills the emergency flag.
	//global_emergencyFix = 0;
	//nope nope nope... do it properly. At least I hope that gets the same point across.
	EASY_CVAR_SET_DEBUGONLY(emergencyFix, 0);

	

	//if applicable..
	saveHiddenCVars();

}//END OF resetModCVars



void partyStart(CBasePlayer* playerRef){

	EASY_CVAR_SET_DEBUGONLY(myStrobe, 1);
	EASY_CVAR_SET_DEBUGONLY(peopleStrobe, 1);
	EASY_CVAR_SET_DEBUGONLY(forceWorldLightOff, 1);
	EASY_CVAR_SET_DEBUGONLY(wildHeads, 1);

	// Nope, do it the right way.  Hope this gets the same point across.
	//global_forceWorldLightOff = 1;
	EASY_CVAR_SET_DEBUGONLY(forceWorldLightOff, 1);

	g_queueCVarHiddenSave = TRUE;

	turnWorldLightsOff();

	if ( playerRef){
		edict_t* tempEd = ENT(playerRef->pev);

		// pick some music.
		char* eh[] = {"Half-Life01", "Half-Life02", "Half-Life08", "Half-Life12", "Half-Life16", "Half-Life17"  };
		int choice = RANDOM_LONG(0, 5);
		
		char* cmdStringSend = UTIL_VarArgs("mp3 play media/%s.mp3", eh[choice]);
		//CLIENT_COMMAND(tempEd, cmdStringSend);

		submitJukeboxRequest(tempEd, cmdStringSend);
	}
}

void partyOff(CBasePlayer* playerRef){

	entvars_t* pev = playerRef->pev;
	edict_t* pEntity = playerRef->edict();

	EASY_CVAR_SET_DEBUGONLY(myStrobe, 0)
	EASY_CVAR_SET_DEBUGONLY(peopleStrobe, 0)
	EASY_CVAR_SET_DEBUGONLY(forceWorldLightOff, 0)
	EASY_CVAR_SET_DEBUGONLY(wildHeads, 0)
		
		
	EASY_CVAR_SET_DEBUGONLY(myStrobe, 0)
	EASY_CVAR_SET_DEBUGONLY(peopleStrobe, 0)
	EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(fogTest, 0)
	EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(imAllFuckedUp, 0)
	EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(thatWasntGrass, 0)
	EASY_CVAR_SET_DEBUGONLY(thatWasntPunch, 0)


	//Hope this gets the same point across.
	//global_forceWorldLightOff = 0;
	EASY_CVAR_SET_DEBUGONLY(forceWorldLightOff, 0)

	g_queueCVarHiddenSave = TRUE;

	turnWorldLightsOn();
		
	if ( playerRef){
		edict_t* tempEd = ENT(playerRef->pev);
		submitJukeboxOff(tempEd);
	}

}//partyOff



/*
 * used by kill command and disconnect command
 * ROBIN: Moved here from player.cpp, to allow multiple player models
 */
void set_suicide_frame(entvars_t* pev)
{       
	if (!FStrEq(STRING(pev->model), "models/player.mdl"))
		return; // allready gibbed

//	pev->frame		= $deatha11;
	pev->solid		= SOLID_NOT;
	pev->movetype	= MOVETYPE_TOSS;
	pev->deadflag	= DEAD_DEAD;
	pev->nextthink	= -1;
}


/*
===========
ClientConnect

called when a player connects to a server
============
*/
BOOL ClientConnect( edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[ 128 ]  )
{	
	return g_pGameRules->ClientConnected( pEntity, pszName, pszAddress, szRejectReason );

// a client connecting during an intermission can cause problems
//	if (intermission_running)
//		ExitIntermission ();

}


/*
===========
ClientDisconnect

called when a player disconnects from a server

GLOBALS ASSUMED SET:  g_fGameOver
============
*/
void ClientDisconnect( edict_t *pEntity )
{
	if (g_fGameOver)
		return;

	char text[256];
	sprintf( text, "- %s has left the game\n", STRING(pEntity->v.netname) );
	MESSAGE_BEGIN( MSG_ALL, gmsgSayText, NULL );
		WRITE_BYTE( ENTINDEX(pEntity) );
		WRITE_STRING( text );
	MESSAGE_END();

	CSound *pSound;
	pSound = CSoundEnt::SoundPointerForIndex( CSoundEnt::ClientSoundIndex( pEntity ) );
	{
		// since this client isn't around to think anymore, reset their sound. 
		if ( pSound )
		{
			pSound->Reset();
		}
	}

// since the edict doesn't get deleted, fix it so it doesn't interfere.
	pEntity->v.takedamage = DAMAGE_NO;// don't attract autoaim
	pEntity->v.solid = SOLID_NOT;// nonsolid
	UTIL_SetOrigin ( &pEntity->v, pEntity->v.origin );
	
	g_pGameRules->ClientDisconnected( pEntity );
	

	//MODDD - what.  Why not delete the player entity then?  Before this player number (2nd, 3rd, etc.), that edict
	// space was NULL anyway.
	//MODDD - TODO.  If necessary, the private data freeing (FREE_PRIVATE) could be moved to the end
	// of some frame-thiink logic to happen at the end of the next frame, instead of right at the disconnect event.
	// I have no clue if other things in the same frame don't expect a player's private data to have changed.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	//pEntity->v...
	//entvars_t* tempEntVars = VARS(pEntity);
	//tempEntVars->nextthink = gpGlobals->time;
	// oh.  we need CBaseEntity.  dammit.
	//pEntity->SetThink(&CBaseEntity::SUB_Remove);

	
	//CBaseEntity* playerEnt = CBaseEntity::Instance(pEntity);
	//playerEnt->pev->nextthink = gpGlobals->time;
	//playerEnt->SetThink(&CBaseEntity::SUB_Remove);

	// UNWISE, UTIL_Remove looks to have no effect (maybe the think or FL_KILLME flags are never acted on.)
	// And REMOVE_ENTITY has weird side-effects on another player joining, doesn't create a new edict the player
	// just exists in the server without being linked to some edict.  Which is... pretty dang spooky.
	// Some things continue to work, some things don't (ignored by any AI logic, ignores line traces,
	// some touches, etc.)
	//UTIL_Remove(playerEnt);
	//REMOVE_ENTITY(pEntity);

	// SO.  How about we make the player data NULL instead to look like an edict meant to hold a player, but
	// none has connected to fill the slot yet?
	//    pEntity->pvPrivateData
	if (pEntity->pvPrivateData) {
		FREE_PRIVATE(pEntity);
		pEntity->pvPrivateData = NULL;  //freed!  Don't trick into thinking this is still valid memory.
		UTIL_Remove(CBaseEntity::Instance(pEntity));
	}

}

//MODDD - respawn method moved to player.h/.cpp.
/*
============
ClientKill

Player entered the suicide command

GLOBALS ASSUMED SET:  g_ulModelIndexPlayer
============
*/
void ClientKill( edict_t *pEntity )
{
	entvars_t *pev = &pEntity->v;

	CBasePlayer *pl = (CBasePlayer*) CBasePlayer::Instance( pev );

	//MODDD - just for safety.
	if (pl == NULL) {
		return;  // ?????????????
	}

	if ( pl->m_fNextSuicideTime > gpGlobals->time )
		return;  // prevent suiciding too ofter

	pl->m_fNextSuicideTime = gpGlobals->time + 1;  // don't let them suicide for 5 seconds after suiciding

	// have the player kill themself
	pev->health = 0;
	pl->Killed( pev, pev, GIB_NEVER );

//	pev->modelindex = g_ulModelIndexPlayer;
//	pev->frags -= 2;		// extra penalty
//	respawn( pev );
}

/*
===========
ClientPutInServer

called each time a player is spawned
============
*/
void ClientPutInServer( edict_t *pEntity )
{
	CBasePlayer *pPlayer;

	entvars_t *pev = &pEntity->v;

	pPlayer = GetClassPtr((CBasePlayer *)pev);
	pPlayer->SetCustomDecalFrames(-1); // Assume none;

	// Allocate a CBasePlayer for pev, and call spawn
	pPlayer->Spawn();

	// Reset interpolation during first frame
	pPlayer->pev->effects |= EF_NOINTERP;


	// This is the 'first appearance' of this player in this game.
	pPlayer->OnFirstAppearance();
	
	easyPrintLine("***Player entered the game (ClientPutInServer)");
}


//// HOST_SAY
// String comes in as
// say blah blah blah
// or as
// blah blah blah
//
void Host_Say( edict_t *pEntity, int teamonly )
{
	CBasePlayer *pClient;
	int j;
	char *p;
	char text[128];
	char szTemp[256];
	const char *cpSay = "say";
	const char *cpSayTeam = "say_team";
	const char *pcmd = CMD_ARGV(0);

	// We can get a raw string now, without the "say " prepended
	if ( CMD_ARGC() == 0 )
		return;

	entvars_t *pev = &pEntity->v;
	CBasePlayer* player = GetClassPtr((CBasePlayer *)pev);

	//Not yet.
	if ( player->m_flNextChatTime > gpGlobals->time )
		 return;

	if ( !stricmp( pcmd, cpSay) || !stricmp( pcmd, cpSayTeam ) )
	{
		if ( CMD_ARGC() >= 2 )
		{
			p = (char *)CMD_ARGS();
		}
		else
		{
			// say with a blank message, nothing to do
			return;
		}
	}
	else  // Raw text, need to prepend argv[0]
	{
		if ( CMD_ARGC() >= 2 )
		{
			sprintf( szTemp, "%s %s", ( char * )pcmd, (char *)CMD_ARGS() );
		}
		else
		{
			// Just a one word command, use the first word...sigh
			sprintf( szTemp, "%s", ( char * )pcmd );
		}
		p = szTemp;
	}

// remove quotes if present
	if (*p == '"')
	{
		p++;
		p[strlen(p)-1] = 0;
	}

// make sure the text has content
	char *pc;
	for ( pc = p; pc != NULL && *pc != 0; pc++ )
	{
		if ( isprint( *pc ) && !isspace( *pc ) )
		{
			pc = NULL;	// we've found an alphanumeric character,  so text is valid
			break;
		}
	}
	if ( pc != NULL )
		return;  // no character found, so say nothing

// turn on color set 2  (color on,  no sound)
	if ( teamonly )
		sprintf( text, "%c(TEAM) %s: ", 2, STRING( pEntity->v.netname ) );
	else
		sprintf( text, "%c%s: ", 2, STRING( pEntity->v.netname ) );

	j = sizeof(text) - 2 - strlen(text);  // -2 for /n and null terminator
	if ( (int)strlen(p) > j )
		p[j] = 0;

	strcat( text, p );
	strcat( text, "\n" );


	player->m_flNextChatTime = gpGlobals->time + CHAT_INTERVAL;

	// loop through all players
	// Start with the first player.
	// This may return the world in single player if the client types something between levels or during spawn
	// so check it, or it will infinite loop

	pClient = NULL;
	while ( ((pClient = (CBasePlayer*)UTIL_FindEntityByClassname( pClient, "player" )) != NULL) && (!FNullEnt(pClient->edict())) ) 
	{
		if ( !pClient->pev )
			continue;
		
		if ( pClient->edict() == pEntity )
			continue;

		if ( !(pClient->IsNetClient()) )	// Not a client ? (should never be true)
			continue;

		// can the receiver hear the sender? or has he muted him?
		if ( g_VoiceGameMgr.PlayerHasBlockedPlayer( pClient, player ) )
			continue;

		if ( teamonly && g_pGameRules->PlayerRelationship(pClient, CBaseEntity::Instance(pEntity)) != GR_TEAMMATE )
			continue;

		MESSAGE_BEGIN( MSG_ONE, gmsgSayText, NULL, pClient->pev );
			WRITE_BYTE( ENTINDEX(pEntity) );
			WRITE_STRING( text );
		MESSAGE_END();
	}

	// print to the sending client
	MESSAGE_BEGIN( MSG_ONE, gmsgSayText, NULL, &pEntity->v );
		WRITE_BYTE( ENTINDEX(pEntity) );
		WRITE_STRING( text );
	MESSAGE_END();

	// echo to server console
	g_engfuncs.pfnServerPrint( text );

	char * temp;
	if ( teamonly )
		temp = "say_team";
	else
		temp = "say";
	
	// team match?
	if ( g_teamplay )
	{
		UTIL_LogPrintf( "\"%s<%i><%s><%s>\" %s \"%s\"\n", 
			STRING( pEntity->v.netname ), 
			GETPLAYERUSERID( pEntity ),
			GETPLAYERAUTHID( pEntity ),
			g_engfuncs.pfnInfoKeyValue( g_engfuncs.pfnGetInfoKeyBuffer( pEntity ), "model" ),
			temp,
			p );
	}
	else
	{
		UTIL_LogPrintf( "\"%s<%i><%s><%i>\" %s \"%s\"\n", 
			STRING( pEntity->v.netname ), 
			GETPLAYERUSERID( pEntity ),
			GETPLAYERAUTHID( pEntity ),
			GETPLAYERUSERID( pEntity ),
			temp,
			p );
	}
}//END OF Host_Say




/*
void* newImitation( size_t stAllocateBlock, entvars_t *pev )
{
	return (void *)ALLOC_PRIVATE(ENT(pev), stAllocateBlock);
}
*/



/*
===========
ClientCommand
called each time a player uses a "cmd" command
============
*/
// !!!START OF THIS SILLY SILLY METHOD!!!
// Use CMD_ARGV and CMD_ARGC to get pointers the character string command.
// Looks like CMD_ARGC gets the count of arguments supplied, that can be handy.
void ClientCommand( edict_t *pEntity )
{
	const char *pcmd = CMD_ARGV(0);
	const char *pstr;

	char pcmdRefined[128];
	strncpy( &pcmdRefined[0], &pcmd[0], 127 );
	pcmdRefined[127] = '\0';
	lowercase(&pcmdRefined[0]);

	const char* pcmdRefinedRef = pcmdRefined;
	
	//MODDD - update "g_flWeaponCheat" to what sv_cheats is.
	
	//if(cvar_sv_cheats != 0){
	//	if(cvar_sv_cheats->value == 1){
	//		g_flWeaponCheat = 1;
	//	}else{
	//		g_flWeaponCheat = 0;
	//	}
	//}

	
	////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Check for this command before the null-player check.  In case of bad transitions, the 'getmap'
	// command will still work.
	if (FStrEq(pcmdRefinedRef, "_mapname")) {
		easyForcePrintLineClient(pEntity, "Map: %s", STRING(gpGlobals->mapname));
		return;

		// "spawn" is an engine command, can't use it
	}else if (FStrEq(pcmdRefinedRef, "fixnullplayer") || FStrEq(pcmdRefinedRef, "respawn") || FStrEq(pcmdRefinedRef, "forcerespawn")) {
		// More of an experiment in how entities work.  Taking a bad transition to another map that puts the player through walls
		// in a dead-looking state can be fixed by calling for this in console, then 'revive' in console, then 'noclip'.
		// The map can be entered then.
		// Spawning for all ent's might be a little delayed though, unsure how this works.
		// Proceed at your own risk after salvaging a bad map traversal like this!  Bizarre things can get broken, like turrets firing
		// crazy fast (a2a2 -> a2a2c).  Or maybe not?   No clue how often or consistently issues happen.


		// comparison?
		//CBasePlayer* pPlayer = (CBasePlayer *)pEntity->pvPrivateData;
		//CBasePlayer* tempplayer = GetClassPtr((CBasePlayer*) pev);


		if ( !pEntity->pvPrivateData ){
			//CBasePlayer* testo = CreateEntity<CBasePlayer>();
			
			// HACKY: Assuming the problem is due to a bad startspot (given, but when attempted to search for something
			// of this name, it finds nothing). Setting to null forces using the map's default spawn spot (as though
			// doing 'map MAPNAME').  Although this isn't necessary anymore, the default loc is used instead now if this
			// doesn't exist.
			//gpGlobals->startspot = NULL;

			/*
			// None of these work???   Oooookay C++
			//CBasePlayer* ye = (CBasePlayer*)ALLOC_PRIVATE(pEntity, sizeof(CBasePlayer));
			//CBasePlayer* ye = (CBasePlayer*)ALLOC_PRIVATE((&pEntity->v)->pContainingEntity, sizeof(CBasePlayer));
			//CBasePlayer* ye = (CBasePlayer*)newImitation((size_t)sizeof(CBasePlayer), &(pEntity->v) );

			CBasePlayer* ye = new(&pEntity->v) CBasePlayer;
			ye->pev = &pEntity->v;

			//int whut = sizeof(CBasePlayer);

			// because the default memory's '0' for 'deadflag' (DEAD_NO) can throw some things off.
			ye->pev->deadflag = DEAD_DEAD;
			ye->Spawn(TRUE);
			*/

			// WAIT!  Just do this
			ClientPutInServer(pEntity);
			CBasePlayer* ye = (CBasePlayer*)pEntity->pvPrivateData;




			int ecks = ye->BloodColor();

			easyForcePrintLine("Do they match? A %d", (ye == pEntity->pvPrivateData));

			CBasePlayer* test2 = (CBasePlayer*)(pEntity->pvPrivateData);
			easyForcePrintLine("Do they match? B %d", (ye == test2));


			//g_engfuncs.pfnPvAllocEntPrivateData(pEntity, 16);
			//pEntity->pvPrivateData = 


			//FREE_PRIVATE(pEntity);
			//pEntity = (void*)ALLOC_PRIVATE(ENT(pev), stAllocateBlock);
		}else{
			// Private data present?  ok, work normally then.
			// like a revive but strips everything, places at the designated spawnpoint per gamemode (single/multiplayer).
			// Like going to the map by 'map X', but doesn't reset anything else.
			// Mimicking what the end of post-killed-think for the player (the very bottom with the 'respawn' call without the map restart on singleplayer)
			
			CBasePlayer* playerRef =  (CBasePlayer*)(pEntity->pvPrivateData);
			//daPlayah->RemoveAllItems(TRUE);
			playerRef->pev->button = 0;
			playerRef->m_iRespawnFrames = 0;
			playerRef->RemoveAllItems(TRUE);
			// no need.  Just call Spawn() instead.
			//respawn(playerRef->pev, !(m_afPhysicsFlags & PFLAG_OBSERVER) );// don't copy a corpse if we're in deathcam.
			playerRef->Spawn();
			playerRef->pev->nextthink = -1;

		}//pvPrivateData check


		return;
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////////



	// Is the client spawned yet?
	if ( !pEntity->pvPrivateData ){
		return;
	}

	entvars_t* pev = &pEntity->v;

	//MODDD - just go ahead and do this now, almost every client call needs this anyway.
	CBasePlayer* tempplayer = GetClassPtr((CBasePlayer*) pev);

	//MODDD
	// If the minimumfiredelay mem's  value does NOT match minimumfiredelay, it must have been changed.
	// Force an RPG re-update to be safe.
	if(globalPSEUDO_minimumfiredelaymem != EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelay) ){
		globalPSEUDO_minimumfiredelaymem = EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelay);
		
		//MODDD - section copied from 'Host_Say' further down, turn into a more general utility if possible/needed.
		// Loop through all players and call "resetPlayerRPGRockets" on each.
		// New scope for neatness.
		{
			CBasePlayer* pItrClient = NULL;
			while (((pItrClient = (CBasePlayer*)UTIL_FindEntityByClassname(pItrClient, "player")) != NULL) && (!FNullEnt(pItrClient->edict())))
			{
				if (!pItrClient->pev)
					continue;
				//if (pItrClient->edict() == pEntity)
				//	continue;
				if (!(pItrClient->IsNetClient()))	// Not a client ? (should never be true)
					continue;

				//resetPlayerRPGRockets(GetClassPtr((CBasePlayer*)pev));
				resetPlayerRPGRockets(pItrClient);

				//MESSAGE_BEGIN(MSG_ONE, gmsgSayText, NULL, pItrClient->pev);
				//  WRITE_BYTE(ENTINDEX(pEntity));
				//  WRITE_STRING(text);
				//MESSAGE_END();
			}
		}
	}//END OF globalPSEUDO_minimumfiredelaymem check




	//if the "else" statement of this long line of possible commands is caught, it means we can check the hidden-vars for a match instead.
	//(assume "TRUE" until then; reaching the last "else" changes it to FALSE)
	BOOL caughtByFirst = TRUE;

	if ( FStrEq(pcmdRefinedRef, "say" ) )
	{
		Host_Say( pEntity, 0 );
	}
	else if ( FStrEq(pcmdRefinedRef, "say_team" ) )
	{
		Host_Say( pEntity, 1 );
	}
	else if ( FStrEq(pcmdRefinedRef, "fullupdate" ) )
	{
		GetClassPtr((CBasePlayer *)pev)->ForceClientDllUpdate(); 
	}
	else if ( FStrEq(pcmdRefinedRef, "give" ) )
	{
		if ( g_flWeaponCheat != 0.0)
		{
			GetClassPtr((CBasePlayer *)pev)->GiveNamedItem( CMD_ARGV(1), attemptInterpretSpawnFlag(CMD_ARGV(2)) );
		}
	}
	//MODDD - inserted new command: givedist.  Change the "givedoffset" var to change what distance items / weapons / npcs are spawned
	//from you.  Although, at large distances (above 100 usually), looking down too much will make it spawn underground and fall through
	//the world.
	//MODDD TODO - all givedist commands should spawn something facing whoever spawned it?  or maybe the default angles of 0,0,0 are fine?
	else if ( FStrEq(pcmdRefinedRef, "givedist" ) )
	{
		if ( tempplayer && g_flWeaponCheat != 0.0)
		{
			UTIL_MakeVectors(pev->v_angle + pev->punchangle);

			//For CBasePlayer, "pev->origin + pev->view_ofs" is the same as "GetGunPosition()"

			Vector vecDest = pev->origin + pev->view_ofs + gpGlobals->v_forward * EASY_CVAR_GET_DEBUGONLY(offsetgivedistance);

			/*
			//TEST - make a breakable.  Tested in c3a2.
			//...ugh.
			CBaseEntity* pushableTest = CBaseEntity::CreateManual("func_breakable", vecDest, Vector(0, 0, 0), NULL);
			
			//pushableTest->pev->model = ALLOC_STRING("*27");
			pushableTest->pev->model = ALLOC_STRING("models/agrunt.mdl");

			pushableTest->Spawn();

			return;
			*/

			tempplayer->GiveNamedItem( CMD_ARGV(1), attemptInterpretSpawnFlag(CMD_ARGV(2)), vecDest);
		}
	}
	//MODDD - other command added.  Spawns item around where you are looking (experimental)
	//Also, "ent_create" does this since this seems to be the intention.
	else if( FStrEq(pcmdRefinedRef, "givelook" ) || FStrEq(pcmdRefinedRef, "ent_create" ) ){
		
		if ( tempplayer && g_flWeaponCheat != 0.0){
			edict_t *pentIgnore;
			TraceResult tr;

			pentIgnore = tempplayer->edict();
			UTIL_MakeVectors(pev->v_angle + pev->punchangle);

			//a tiny bit in front for safety.
			Vector vecSrc = pev->origin + pev->view_ofs + gpGlobals->v_forward * 5;
			Vector vecDest = pev->origin + pev->view_ofs + gpGlobals->v_forward * 2048;

			//nah, precision for while ducking not necessary.
			/*
			Vector playerEyePos = Vector(tempplayer->body;
			if(pev->flags & FL_DUCKING){

			}
			*/

			UTIL_TraceLine( vecSrc, vecDest, dont_ignore_monsters, pentIgnore, &tr );
			//tr.vecEndPos();

			if (tr.fAllSolid){
				ALERT ( at_console, "Could not determine spawn location.\n" );
				return;
			}

			/*
			if(tr.pHit != NULL){
				easyForcePrintLineClient(pEntity, "HIT SOMETHING? %s", STRING(tr.pHit->v.classname) );
			}else{
				easyForcePrintLineClient(pEntity, "DID NOT HIT SOMETHING?");
			}
			*/

			/*
			easyForcePrintLineClient(pEntity, "YEEEE %.2f", tr.flFraction);
			UTIL_printLineVector("VECTOA", tr.vecPlaneNormal);
			*/

			//tempplayer->debugDrawVectRecentGive1 = vecSrc;
			//tempplayer->debugDrawVectRecentGive2 = tr.vecEndPos;

			tempplayer->GiveNamedItem( CMD_ARGV(1),  attemptInterpretSpawnFlag(CMD_ARGV(2)),
									 tr.vecEndPos.x,
									 tr.vecEndPos.y,
									 tr.vecEndPos.z + EASY_CVAR_GET_DEBUGONLY(offsetgivelookvertical),
									 TRUE, &tr);
		}
	}
	else if ( FStrEq(pcmdRefinedRef, "everything") || FStrEq(pcmdRefinedRef, "gimmie") || FStrEq(pcmdRefinedRef, "gimmi") || FStrEq(pcmdRefinedRef, "gimmy") || FStrEq(pcmdRefinedRef, "fuckmyshitup") ){

		if ( g_flWeaponCheat != 0.0)
		{
			int i;

			CBasePlayer* playerRef = GetClassPtr((CBasePlayer *)pev);
			if(playerRef){
				playerRef->reviveIfDead();
				playerRef->setHealth(100);
				playerRef->setArmorBattery(100);
				playerRef->grantAllItems();
				playerRef->giveMaxAmmo();
				
				playerRef->attemptResetTimedDamage(TRUE);
				//playerRef->pev->flags |= FL_GODMODE;
				//playerRef->pev->flags |= MOVETYPE_NOCLIP;


				playerRef->m_rgItems[ITEM_ANTIDOTE] = ITEM_ANTIDOTE_MAX;
				playerRef->m_rgItems[ITEM_ADRENALINE] = ITEM_ADRENALINE_MAX;
				playerRef->m_rgItems[ITEM_RADIATION] = ITEM_RADIATION_MAX;
				playerRef->airTankAirTime = PLAYER_AIRTANK_TIME_MAX;

			}
		}else {
			easyForcePrintLineClient(pEntity, "You need sv_cheats on for that!");
		}



		// Just a test to see how many entities can be spawned before the game pukes.  Looks like the magic number is 900.
		// Any index that tries to go over 899 crashes.
		// Any indexes ever deleted before that point try to be used first.
		// Note that going past 200 to 300 will already give 'too many entities in visible packet' or whatever that was errors.
		/*
		// (keep in mind: 3 entities were giving rendering printouts before this began)
		SZp#753 : SV: 898
		SV : SZd#753
		SZp#754 : SV: 899
		SV : SZd#754
		SZp#755 : hl.exe has triggered a breakpoint.
		*/
		/*
		for (int i = 0; i < 50; i++) {
			easyForcePrint("SZp#%d : ", i);
			CBaseEntity* entRef = CBaseEntity::Create("monster_zombie", Vector(105.33, 598.49, 80), Vector(0, 0, 0), NULL);

			if (entRef != NULL) {
				// ENTINDEX( this->edict() )
				int yea = entRef->entindex();
				easyForcePrintLine("%d", yea);
			}
			else {
				easyForcePrintLine("none?");
			}

			//UTIL_Remove(entRef);
			easyForcePrintLine("SZd#%d", i);
		}
		*/


	}else if ( FStrEq(pcmdRefinedRef, "notimeddamage") || FStrEq(pcmdRefinedRef, "notdmg") || FStrEq(pcmdRefinedRef, "resettimeddamage") || FStrEq(pcmdRefinedRef, "resettdmg") || FStrEq(pcmdRefinedRef, "cleartimeddamage") || FStrEq(pcmdRefinedRef, "cleartdmg")){
		// only reset timed damages.
		if ( g_flWeaponCheat != 0.0)
		{
			CBasePlayer* playerRef = GetClassPtr((CBasePlayer *)pev);
			if(playerRef){
				playerRef->attemptResetTimedDamage(TRUE);
			}
		}else {
			easyForcePrintLineClient(pEntity, "You need sv_cheats on for that!");
		}
	}else if ( FStrEq(pcmdRefinedRef, "supergun") || FStrEq(pcmdRefinedRef, "neverrunout") ){
		if ( g_flWeaponCheat != 0.0)
		{
		EASY_CVAR_SET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteclip, 1 );
		EASY_CVAR_SET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteammo, 1 );
		g_queueCVarHiddenSave = TRUE;
		}

	}else if ( FStrEq(pcmdRefinedRef, "allcheats") || FStrEq(pcmdRefinedRef, "allcheat") || FStrEq(pcmdRefinedRef, "cheats") || FStrEq(pcmdRefinedRef, "cheat") || FStrEq(pcmdRefinedRef, "cheatersalwayswin") || FStrEq(pcmdRefinedRef, "fuckeverything") || FStrEq(pcmdRefinedRef, "iamshivagodofdeath")  ){
		if ( g_flWeaponCheat != 0.0)
		{
		EASY_CVAR_SET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteclip, 1 );
		EASY_CVAR_SET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteammo, 1 );
		EASY_CVAR_SET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelay, 1 );
		g_queueCVarHiddenSave = TRUE;

		globalPSEUDO_minimumfiredelaymem = 1;

		CBasePlayer* playerRef = GetClassPtr((CBasePlayer *)pev);
			if(playerRef){
				playerRef->reviveIfDead();
				playerRef->setHealth(100);
				playerRef->setArmorBattery(100);
				playerRef->grantAllItems();
				playerRef->giveMaxAmmo();

				playerRef->attemptResetTimedDamage(TRUE);
				//playerRef->pev->flags |= FL_GODMODE;
				//playerRef->pev->flags |= MOVETYPE_NOCLIP;
			}
		}
	}else if ( FStrEq(pcmdRefinedRef, "infiniteclip") ){
		if ( g_flWeaponCheat != 0.0)
		{
		EASY_CVAR_SET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteclip, EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteclip)==0?1:0 );
		g_queueCVarHiddenSave = TRUE;
		}

	}else if ( FStrEq(pcmdRefinedRef, "infiniteammo") ){
		if ( g_flWeaponCheat != 0.0)
		{
			EASY_CVAR_SET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteammo, EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteammo)==0?1:0 );
			g_queueCVarHiddenSave = TRUE;
		}
	}else if (  FStrEq(pcmdRefinedRef, "minimumfiredelay") || FStrEq(pcmdRefinedRef, "minimumdelay") || FStrEq(pcmdRefinedRef, "firedelay") || FStrEq(pcmdRefinedRef, "dakkadakkadakka")|| FStrEq(pcmdRefinedRef, "dakka") ||FStrEq(pcmdRefinedRef, "dakadakadaka") || FStrEq(pcmdRefinedRef, "daka") || FStrEq(pcmdRefinedRef, "needsmoredaka") || FStrEq(pcmdRefinedRef, "needsmoardaka") || FStrEq(pcmdRefinedRef, "needsmoredakka") || FStrEq(pcmdRefinedRef, "needsmoardakka") || FStrEq(pcmdRefinedRef, "notenoughdaka") || FStrEq(pcmdRefinedRef, "notenoughdakka") || FStrEq(pcmdRefinedRef, "notenuffdaka") || FStrEq(pcmdRefinedRef, "notenuffdakka") || FStrEq(pcmdRefinedRef, "notenufdaka") || FStrEq(pcmdRefinedRef, "notenufdakka")   || FStrEq(pcmdRefinedRef, "neverenoughdaka") || FStrEq(pcmdRefinedRef, "neverenoughdakka") || FStrEq(pcmdRefinedRef, "neverenuffdaka") || FStrEq(pcmdRefinedRef, "neverenuffdakka") || FStrEq(pcmdRefinedRef, "neverenufdaka") || FStrEq(pcmdRefinedRef, "neverenufdakka")   || FStrEq(pcmdRefinedRef, "nevaenoughdaka") || FStrEq(pcmdRefinedRef, "nevaenoughdakka") || FStrEq(pcmdRefinedRef, "nevaenuffdaka") || FStrEq(pcmdRefinedRef, "nevaenuffdakka") || FStrEq(pcmdRefinedRef, "nevaenufdaka") || FStrEq(pcmdRefinedRef, "nevaenufdakka")          ){

		if ( g_flWeaponCheat != 0.0)
		{
			EASY_CVAR_SET_DEBUGONLY(cheat_minimumfiredelay, EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelay)==0?1:0 );
			globalPSEUDO_minimumfiredelaymem = EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelay);
			g_queueCVarHiddenSave = TRUE;
			resetPlayerRPGRockets( GetClassPtr((CBasePlayer *)pev) );
		}

	}else if (  FStrEq(pcmdRefinedRef, "superman")){

		//TODO: lock these cheats by sv_cheats (or g_flWeaponCheat ? )
		EASY_CVAR_SET_DEBUGONLY(normalSpeedMulti, 3);
		EASY_CVAR_SET_DEBUGONLY(noclipSpeedMulti, 5);
		EASY_CVAR_SET_DEBUGONLY(jumpForceMulti, 3);
		EASY_CVAR_SET_DEBUGONLY(ladderSpeedMulti, DEFAULT_ladderSpeedMulti*3);
		g_queueCVarHiddenSave = TRUE;

	}else if( FStrEq(pcmdRefinedRef, "normalman")){

		EASY_CVAR_SET_DEBUGONLY(normalSpeedMulti, 1);
		EASY_CVAR_SET_DEBUGONLY(noclipSpeedMulti, 2.5);
		EASY_CVAR_SET_DEBUGONLY(jumpForceMulti, 1);
		EASY_CVAR_SET_DEBUGONLY(ladderSpeedMulti, DEFAULT_ladderSpeedMulti);
		g_queueCVarHiddenSave = TRUE;

	}else if ( FStrEq(pcmdRefinedRef, "disablecheats") || FStrEq(pcmdRefinedRef, "disablecheat") || FStrEq(pcmdRefinedRef, "nocheating") || FStrEq(pcmdRefinedRef, "nocheats") || FStrEq(pcmdRefinedRef, "nocheat") || FStrEq(pcmdRefinedRef, "winnersdontdodrugs") ){

		CBasePlayer* playerRef = GetClassPtr((CBasePlayer *)pev);
		
		EASY_CVAR_SET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteclip, 0 );
		EASY_CVAR_SET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteammo, 0 );
		EASY_CVAR_SET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelay, 0 );
		globalPSEUDO_minimumfiredelaymem = 0;
		EASY_CVAR_SET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_nogaussrecoil, 0);
		resetPlayerRPGRockets( playerRef );
		g_queueCVarHiddenSave = TRUE;
		
		/*
		EASY_CVAR_SET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(gaussRecoilSendsUpInSP, 0);
		EASY_CVAR_SET(sv_longjump_chargemode, 0);
		EASY_CVAR_SET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_touchNeverExplodes, 0);
		*/
		

		//if the barnacle cheat is on, turn it of.
		if(cheat_barnacleEatsEverything == 1){
			cheat_barnacleEatsEverything = 2;
			easyForcePrintLineBroadcast("CHEAT OFF: Barnacles returned to normal.");
		}

	}else if ( FStrEq(pcmdRefinedRef, "thealmightybarnacle") ){
		if ( g_flWeaponCheat != 0.0)
		{
			if(cheat_barnacleEatsEverything == 0){
				cheat_barnacleEatsEverything = 1;
			}else if(cheat_barnacleEatsEverything == 1){
				cheat_barnacleEatsEverything = 2;
			}else if(cheat_barnacleEatsEverything == 2){
				cheat_barnacleEatsEverything = 1;
			}
			
			if(cheat_barnacleEatsEverything == 1){
				easyForcePrintLineBroadcast("CHEAT ON: Barnacles can eat any monster (NPC)!");
			}else if(cheat_barnacleEatsEverything == 2){
				easyForcePrintLineBroadcast("CHEAT OFF: Barnacles returned to normal.");
			}
		}
	}else if( FStrEq(pcmdRefinedRef, "fixrpg") ){
		//This command is also included in anything toggling / changing "cheat_minimumfiredelay".
		//It resets an important var (m_cActiveRockets) that, when non-zero, makes the player unable to reload
		//in follow (dot) fire mode.
		resetPlayerRPGRockets( GetClassPtr((CBasePlayer *)pev) );
	}else if( FStrEq(pcmdRefinedRef, "fixcvar") || FStrEq(pcmdRefinedRef, "resetcvar") || FStrEq(pcmdRefinedRef, "fixcvars") || FStrEq(pcmdRefinedRef, "reestcvars") || FStrEq(pcmdRefinedRef, "unfuckup") || FStrEq(pcmdRefinedRef, "unfuckupcvar") || FStrEq(pcmdRefinedRef, "unfuckupcvars") || FStrEq(pcmdRefinedRef, "unfuck") || FStrEq(pcmdRefinedRef, "unfuckcvar") || FStrEq(pcmdRefinedRef, "unfuckcvars") ){
		//reset the most annoying CVars.
		CBasePlayer* playerRef = GetClassPtr((CBasePlayer *)pev);
		resetModCVars(playerRef, FALSE);

	}else if ( FStrEq(pcmdRefinedRef, "fixstuka") || FStrEq(pcmdRefinedRef, "stukafix")  ){

		EASY_CVAR_SET_DEBUGONLY(STUcheckDistH, 6);
		EASY_CVAR_SET_DEBUGONLY(STUcheckDistV, 8);
		EASY_CVAR_SET_DEBUGONLY(STUcheckDistD, 10);
		EASY_CVAR_SET_DEBUGONLY(STUextraTriangH, 1.2f);
		EASY_CVAR_SET_DEBUGONLY(STUextraTriangV, 1.3f);
		EASY_CVAR_SET_DEBUGONLY(STUrepelMulti, 1.0f);
		EASY_CVAR_SET_DEBUGONLY(STUSpeedMulti, 1.0f);
		EASY_CVAR_SET_DEBUGONLY(drawDebugPathfinding2, 0);
		EASY_CVAR_SET_DEBUGONLY(STUExplodeTest, 0);
		EASY_CVAR_SET_DEBUGONLY(STUYawSpeedMulti, 0.88);
		EASY_CVAR_SET_DEBUGONLY(STUDetection, 1);
		g_queueCVarHiddenSave = TRUE;
		
	}else if ( FStrEq(pcmdRefinedRef, "partyfix") || FStrEq(pcmdRefinedRef, "fixparty") || FStrEq(pcmdRefinedRef, "resetparty") || FStrEq(pcmdRefinedRef, "partyreset") ){

		EASY_CVAR_SET_DEBUGONLY(raveEffectSpawnInterval, 0.22)
		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(strobeDurationMin, 0.08)
		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(strobeDurationMax, 0.14)
		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(strobeRadiusMin, 280)
		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(strobeRadiusMax, 410)
		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(strobeSpawnDistHori, 180)
		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(strobeSpawnDistVertMin, 25)
		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(strobeSpawnDistVertMax, 75)
		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(strobeMultiColor, 1)
		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(raveLaserEnabled, 1)
		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(raveLaserSpawnFreq, 2.3)
		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(raveLaserLength, 180)
		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(raveLaserSpawnDistHoriMin, 50)
		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(raveLaserSpawnDistHoriMax, 110)
		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(raveLaserSpawnDistVertMin, -5)
		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(raveLaserSpawnDistVertMax, 45)
		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(raveLaserBrightnessMin, 1.0)
		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(raveLaserBrightnessMax, 1.0)
		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(raveLaserDurationMin, 0.06)
		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(raveLaserDurationMax, 0.12)
		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(raveLaserThicknessMin, 0.8)
		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(raveLaserThicknessMax, 2.6)
		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(raveLaserNoiseMin, 0)
		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(raveLaserNoiseMax, 0)
		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(raveLaserFrameRateMin, 10)
		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(raveLaserFrameRateMax, 10)
		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(raveLaserMultiColor, 1)
		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(thatWasntGrass, 0)
		EASY_CVAR_SET_DEBUGONLY(thatWasntPunch, 0)
		g_queueCVarHiddenSave = TRUE;

	}else if( FStrEq(pcmdRefinedRef, "resetSparks") || FStrEq(pcmdRefinedRef, "tooManySparks")   ){
	
		EASY_CVAR_SET_DEBUGONLY(sparksAllMulti, 1);
		EASY_CVAR_SET_DEBUGONLY(sparksEnvMulti, 0.5f);
		EASY_CVAR_SET_DEBUGONLY(sparksButtonMulti, 1);
		EASY_CVAR_SET_DEBUGONLY(sparksPlayerCrossbowMulti, 1);
		EASY_CVAR_SET_DEBUGONLY(sparksComputerHitMulti, 1);
		EASY_CVAR_SET_DEBUGONLY(sparksTurretDeathMulti, 1);
		EASY_CVAR_SET_DEBUGONLY(sparksOspreyHitMulti, 1);
		EASY_CVAR_SET_DEBUGONLY(sparksExplosionMulti, 1);
		EASY_CVAR_SET_DEBUGONLY(sparksBeamMulti, 1);
		EASY_CVAR_SET_DEBUGONLY(sparksAIFailMulti, 1);
		//CVAR_SET_FLOAT("shrapRand", 190);
		//CVAR_SET_FLOAT("shrapRandHeightExtra", 140);
		//CVAR_SET_FLOAT("shrapMode", 1);
		//CVAR_SET_FLOAT("explosionShrapnelMulti", 1);
		g_queueCVarHiddenSave = TRUE;

	}else if( FStrEq(pcmdRefinedRef, "partyon") || FStrEq(pcmdRefinedRef, "party") || FStrEq(pcmdRefinedRef, "partymode") || FStrEq(pcmdRefinedRef, "whospikedthepunch") || FStrEq(pcmdRefinedRef, "clubon") ||FStrEq(pcmdRefinedRef, "rave") || FStrEq(pcmdRefinedRef, "technorave") || FStrEq(pcmdRefinedRef, "techno") || FStrEq(pcmdRefinedRef, "club") || FStrEq(pcmdRefinedRef, "clubbing") || FStrEq(pcmdRefinedRef, "breaktime") || FStrEq( pcmdRefinedRef, "ecstasy") || FStrEq(pcmdRefinedRef, "partytime") || FStrEq(pcmdRefinedRef, "indaclub") || FStrEq(pcmdRefinedRef, "intheclub") || FStrEq(pcmdRefinedRef, "whodruggedmyshit") || FStrEq(pcmdRefinedRef, "drugs") || FStrEq(pcmdRefinedRef, "thattastedfunny") || FStrEq(pcmdRefinedRef, "thattastesfunny") || FStrEq(pcmdRefinedRef, "tastesfunny") || FStrEq(pcmdRefinedRef, "krunk") || FStrEq(pcmdRefinedRef, "getfucked") || FStrEq(pcmdRefinedRef, "fuckedup") || FStrEq(pcmdRefinedRef, "getfuckedup") || FStrEq(pcmdRefinedRef, "letsgetwasted") || FStrEq(pcmdRefinedRef, "getwasted") ){
		partyStart(tempplayer);

	}else if(  FStrEq(pcmdRefinedRef, "partyoff") || FStrEq(pcmdRefinedRef, "cluboff") ||FStrEq(pcmdRefinedRef, "icantseeshit") || FStrEq(pcmdRefinedRef, "kidsthesedays") || FStrEq(pcmdRefinedRef, "nomoreclubbing") || FStrEq(pcmdRefinedRef, "fucktheclub") || FStrEq(pcmdRefinedRef, "fuckdaclub") || FStrEq(pcmdRefinedRef, "breaktimeisover") || FStrEq(pcmdRefinedRef, "breaktimesover") || FStrEq(pcmdRefinedRef, "breaktimesup") || FStrEq(pcmdRefinedRef, "backtowork") || FStrEq(pcmdRefinedRef, "getbacktowork") || FStrEq(pcmdRefinedRef, "drugfree") || FStrEq(pcmdRefinedRef, "outtheclub") || FStrEq(pcmdRefinedRef, "outoftheclub") || FStrEq(pcmdRefinedRef, "dapopo") || FStrEq(pcmdRefinedRef, "popo") || FStrEq(pcmdRefinedRef, "police") || FStrEq(pcmdRefinedRef, "callthepolice") || FStrEq(pcmdRefinedRef, "thispartysucks") ){
		partyOff(tempplayer);

	}else if(  FStrEq(pcmdRefinedRef, "hyperravebutnotext") || FStrEq(pcmdRefinedRef, "hyperavebutnotext") || FStrEq(pcmdRefinedRef, "hardcorebutnotext") || FStrEq(pcmdRefinedRef, "hyperravenotext") || FStrEq(pcmdRefinedRef, "hyperavenotext") || FStrEq(pcmdRefinedRef, "hardcorenotext")   ){
		partyStart(tempplayer);

		EASY_CVAR_SET_DEBUGONLY(raveEffectSpawnInterval, 0.1)
		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(strobeDurationMin, 0.06)
		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(strobeDurationMax, 0.08)
		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(raveLaserSpawnFreq, 2.3)
		//only difference: no crazy text.
		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(imAllFuckedUp, 0)
		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(thatWasntGrass, 1)
		EASY_CVAR_SET_DEBUGONLY(thatWasntPunch, 1)
		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(fogTest, 2)
		g_queueCVarHiddenSave = TRUE;

	}else if(  FStrEq(pcmdRefinedRef, "hyperrave") || FStrEq(pcmdRefinedRef, "hyperave") || FStrEq(pcmdRefinedRef, "hardcore")   ){
		partyStart(tempplayer);

		EASY_CVAR_SET_DEBUGONLY(raveEffectSpawnInterval, 0.1)
		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(strobeDurationMin, 0.06)
		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(strobeDurationMax, 0.08)
		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(raveLaserSpawnFreq, 2.3)
		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(imAllFuckedUp, 1)
		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(thatWasntGrass, 1)
		EASY_CVAR_SET_DEBUGONLY(thatWasntPunch, 1)
		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(fogTest, 2)
		g_queueCVarHiddenSave = TRUE;

	}else if(  FStrEq(pcmdRefinedRef, "fixCamera") || FStrEq(pcmdRefinedRef, "camerafix") || FStrEq(pcmdRefinedRef, "cameraReset") || FStrEq(pcmdRefinedRef, "cameraReset")    ){
	
		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(myCameraSucks, 0)
		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(cameraPosFixedX, -1)
		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(cameraPosFixedY, -1)
		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(cameraPosFixedZ, -1)
		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(cameraPosOffX, 0)
		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(cameraPosOffY, 0)
		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(cameraPosOffZ, 260)
		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(cameraRotFixedX, 90)
		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(cameraRotFixedY, -1)
		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(cameraRotFixedZ, -1)
		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(cameraRotOffX, 0)
		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(cameraRotOffY, 0)
		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(cameraRotOffZ, 0)
		g_queueCVarHiddenSave = TRUE;

	}else if ( FStrEq(pcmdRefinedRef, "fixprecache" ) || FStrEq(pcmdRefinedRef, "fixcache" ) || FStrEq(pcmdRefinedRef, "cachefix"   ) || FStrEq(pcmdRefinedRef, "precachefix")  ){

		EASY_CVAR_SET(precacheAll, 1);
		EASY_CVAR_SET(soundSentenceSave, 1);

		easyForcePrintLineClient(pEntity, "***precacheAll & soundSentenceSave activated.  Remember to restart if in-game, or close / re-launch if there has been any precache-related crash this session.***");

	}else if ( FStrEq(pcmdRefinedRef, "drop" ) )
	{
		// player is dropping an item. 
		GetClassPtr((CBasePlayer *)pev)->DropPlayerItem((char *)CMD_ARGV(1));
	}else if ( FStrEq(pcmdRefinedRef, "playbatterysound" ) ){
		
		int iszItem = ALLOC_STRING( CMD_ARGV(1) );
		const char* pszName = STRING(iszItem);

		int i = 0;
		int pszNameLength = 0;
		while(i < 99){
			if(pszName[i] == '\0'){
				pszNameLength =  i;
				break;
			}
			i++;
		}
		int result = 0;

		if(sscanf(pszName, "%d", &result) == EOF){
			easyForcePrintLineClient(pEntity, "That isn\'t a number, dillweed!");
		}else{

			if ( tempplayer)
			{
				//tempplayer->SetSuitUpdateNumber(tempplayer->pev->armorvalue, 0);
				
				/*
				if(result != 0){
					tempplayer->SetSuitUpdate("!HEV_BNOTICE", FALSE, SUIT_REPEAT_OK, 0.7f);
					tempplayer->SetSuitUpdateNumber(result, 0, -1, -1);
					tempplayer->SetSuitUpdate("!HEV_BPERCENT", FALSE, SUIT_REPEAT_OK, 0.8f);		 
				}else{
					tempplayer->SetSuitUpdateNumber(result, 0, -1, -1);
				}
				*/
				tempplayer->SetSuitUpdateNumber(result, 0, -1, FALSE);
			}
		}
		int istr = MAKE_STRING(pszName);
		easyForcePrintLineClient(pEntity, "help me %d", result);

		//delete[] stuff;

	}else if ( FStrEq(pcmdRefinedRef, "playtest2" ) ){
		int iszItem = ALLOC_STRING( CMD_ARGV(1) );
		const char* pszName = STRING(iszItem);

		int i = 0;
		int pszNameLength = 0;
		while(i < 99){
			if(pszName[i] == '\0'){
				pszNameLength =  i;
				break;
			}
			i++;
		}
		int result = 0;

		if(sscanf(pszName, "%d", &result) == EOF){
			easyForcePrintLineClient(pEntity, "That isn\'t a number, dillweed!");
		}else{
			
			if ( tempplayer){
				// why
				//edict_t* tempEd = ENT(tempplayer->pev);
				edict_t* tempEd = pEntity;

				switch(result){
				case 0:  //ENT(pev)???
					UTIL_PlaySound(tempEd, CHAN_ITEM, "!playtestEnd", 1, ATTN_NORM, 0, 100, FALSE);
				break;
				case 1:
					UTIL_PlaySound(tempEd, CHAN_ITEM, "weapons/reload2.wav", 1, ATTN_NORM, 0, 100, FALSE);
				break;
				case 2:
					UTIL_PlaySound(tempEd, CHAN_ITEM, "weapons/reload3.wav", 1, ATTN_NORM, 0, 100, FALSE);
				break;
				case 3:
					UTIL_PlaySound(tempEd, CHAN_ITEM, "weapons/357_reload1.wav", 1, ATTN_NORM, 0, 100, FALSE);
				break;
				case 4:
					UTIL_PlaySound(tempEd, CHAN_ITEM, "weapons/guncock1.wav", 1, ATTN_NORM, 0, 100, FALSE);
				break;
				case 5:
					UTIL_PlaySound(tempEd, CHAN_ITEM, "items/gunpickup1.wav", 1, ATTN_NORM, 0, 100, FALSE);
				break;
				case 6:
					UTIL_PlaySound(tempEd, CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM, 0, 100, FALSE);
				break;
				case 7:
					UTIL_PlaySound(tempEd, CHAN_ITEM, "items/gunpickup3.wav", 1, ATTN_NORM, 0, 100, FALSE);
				break;
				case 8:
					UTIL_PlaySound(tempEd, CHAN_ITEM, "items/gunpickup4.wav", 1, ATTN_NORM, 0, 100, FALSE);
				break;
				case 9:
					UTIL_PlaySound(tempEd, CHAN_ITEM, "vox/acknowledge.wav", 1, ATTN_NORM, 0, 100, FALSE);
				break;
				case 10:
					UTIL_PlaySound(tempEd, CHAN_ITEM, "weapons/xbow_hitbod1.wav", 1, ATTN_NORM, 0, 100, FALSE);
				break;
				case 11:
					UTIL_PlaySound(tempEd, CHAN_ITEM, "weapons/xbow_hitbod2.wav", 1, ATTN_NORM, 0, 100, FALSE);
				break;
				default:
					easyForcePrintLineClient(pEntity, "sound %d not found.", result);
				break;
				}
			}
		}

	}else if ( FStrEq(pcmdRefinedRef, "soundtest" ) || FStrEq(pcmdRefinedRef, "playsound")){

		if ( tempplayer){
			edict_t* tempEd = pEntity;
			int iszItem = ALLOC_STRING( CMD_ARGV(1) );
			const char* argPitch = CMD_ARGV(2);
			const char* pszName = STRING(iszItem);

			copyString(pszName, queuedSound, 127 );
			playQueuedPitch = 100;  //default pitch if not provided

			if(argPitch != NULL && !isStringEmpty(argPitch)){
				try{
					int numbAttempt = tryStringToInt(argPitch);
					//success? apply.
					playQueuedPitch = numbAttempt;
					if(playQueuedPitch > 255){
						easyForcePrintLineClient(pEntity, "***Pitch can not exceed 255.");
						playQueuedPitch = 255;
					}else if(playQueuedPitch < 0){
						easyForcePrintLineClient(pEntity, "***Pitch must be at least 0.");
						playQueuedPitch = 0;
					}
				}catch(int){
					easyForcePrintLineClient(pEntity, "***WARNING. 2nd arg, pitch, not understood. Must be a whole number.");
				}
			}

			//a slight delay.  A sound playing instantly may cut off unexpectedly.
			playQueuedTime = gpGlobals->time + 0.1f;
			playedSoundPlayer.Set(pEntity);
			playQueued = TRUE;
		}
		
	}else if ( FStrEq(pcmdRefinedRef, "sentencetest" ) || FStrEq(pcmdRefinedRef, "playsentence")){
			
		if ( tempplayer){
			edict_t* tempEd = pEntity;
			int iszItem = ALLOC_STRING( CMD_ARGV(1) );
			const char* argPitch = CMD_ARGV(2);
			const char* pszName = STRING(iszItem);
			
			if(pszName[0] != '!'){
				queuedSound[0] = '!';
				copyString(pszName, &queuedSound[1], 127 - 1);
				//offset.  Start at queuedSound[1], end at one less than usual size.
			}else{
				//eh, just overwrite that "!".
				copyString(pszName, queuedSound, 127 );
			}

			playQueuedPitch = 100;  //default pitch if not provided

			if(argPitch != NULL && !isStringEmpty(argPitch)){
				try{
					int numbAttempt = tryStringToInt(argPitch);
					//success? apply.
					playQueuedPitch = numbAttempt;
					if(playQueuedPitch > 255){
						easyForcePrintLineClient(pEntity, "***Pitch can not exceed 255.");
						playQueuedPitch = 255;
					}else if(playQueuedPitch < 0){
						easyForcePrintLineClient(pEntity, "***Pitch must be at least 0.");
						playQueuedPitch = 0;
					}
				}catch(int){
					easyForcePrintLineClient(pEntity, "***WARNING. 2nd arg, pitch, not understood. Must be a whole number.");
				}
			}

			//a slight delay.  A sound playing instantly may cut off unexpectedly.
			playQueuedTime = gpGlobals->time + 0.1f;
			playedSoundPlayer.Set(pEntity);
			playQueued = TRUE;
		}
		
	}else if ( FStrEq(pcmdRefinedRef, "stopsoundtest" ) ){   // 'stopsound' is already occupied, funny enough
		if ( tempplayer){
			edict_t* tempEd = pEntity;
			int iszItem = ALLOC_STRING( CMD_ARGV(1) );
			const char* pszName = STRING(iszItem);

			if(pszName[0] == '\0'){
				UTIL_PlaySound( tempEd, CHAN_VOICE, "common/null.wav", 1.0, ATTN_IDLE, 0, 100, FALSE);
				UTIL_PlaySound( tempEd, CHAN_ITEM, "common/null.wav", 1.0, ATTN_IDLE, 0, 100, FALSE);
				UTIL_PlaySound( tempEd, CHAN_STREAM, "common/null.wav", 1.0, ATTN_IDLE, 0, 100, FALSE);
			}else{
				//same as UTIL_StopSound, maybe?
				EMIT_SOUND_DYN(tempEd, CHAN_STREAM, pszName, 0, 0, SND_STOP, PITCH_NORM);
			}

			//also turn off MP3 music.
			submitJukeboxOff(tempEd);	
		}

	}else if ( FStrEq(pcmdRefinedRef, "stopsentencetest" ) || FStrEq(pcmdRefinedRef, "stopsentence")){
		if ( tempplayer){
			edict_t* tempEd = pEntity;
			int iszItem = ALLOC_STRING( CMD_ARGV(1) );
			const char* pszName = STRING(iszItem);

			if(pszName[0] == '\0'){
				UTIL_PlaySound( tempEd, CHAN_VOICE, "common/null.wav", 1.0, ATTN_IDLE, 0, 100, FALSE);
				UTIL_PlaySound( tempEd, CHAN_ITEM, "common/null.wav", 1.0, ATTN_IDLE, 0, 100, FALSE);
				UTIL_PlaySound( tempEd, CHAN_STREAM, "common/null.wav", 1.0, ATTN_IDLE, 0, 100, FALSE);
			}else{
				char tempsound[127];
				if(pszName[0] != '!'){
					tempsound[0] = '!';
					copyString(pszName, &tempsound[1], 127 - 1);
				}else{
					//eh, just overwrite that "!".
					copyString(pszName, tempsound, 127 );
				}
				EMIT_SOUND_DYN(tempEd, CHAN_STREAM, tempsound, 0, 0, SND_STOP, PITCH_NORM);
			}
				
			// also turn off MP3 music.
			submitJukeboxOff(tempEd);
		}//END OF if(tempplayer)

	}else if(FStrEq(pcmdRefinedRef, "origin") || FStrEq(pcmdRefinedRef, "coords")){
		if(tempplayer){
			CBaseEntity* pEntityForward = FindEntityForward( tempplayer);
			if(pEntityForward == NULL){
				//assume we meant the player.
				interpretAsCoords(pEntity, tempplayer, "Client");
			}else{
				interpretAsCoords(pEntity, pEntityForward, STRING(pEntityForward->pev->classname) );
			}
		}
		
	}else if( FStrEq(pcmdRefinedRef, "myorigin") || FStrEq(pcmdRefinedRef, "getmyorigin") || FStrEq(pcmdRefinedRef, "playerorigin") || FStrEq(pcmdRefinedRef, "mycoords") || FStrEq(pcmdRefinedRef, "getmycoords") || FStrEq(pcmdRefinedRef, "playercoords")){
		
		if(tempplayer){
			interpretAsCoords(pEntity, tempplayer, "Client");
		}

	}else if(FStrEq(pcmdRefinedRef, "entorigin") || FStrEq(pcmdRefinedRef, "entityorigin") || FStrEq(pcmdRefinedRef, "monsterorigin") || FStrEq(pcmdRefinedRef, "entcoords") || FStrEq(pcmdRefinedRef, "entitycoords") || FStrEq(pcmdRefinedRef, "monstercoords") ){
		
		CBaseEntity* pEntityForward = FindEntityForward( tempplayer );
		if(pEntityForward){
			interpretAsCoords(pEntity, pEntityForward, STRING(pEntityForward->pev->classname) );
		}

	}else if( FStrEq(pcmdRefinedRef, "jukebox")){
		char tempCommandBuffer[128];
		//this OK?
		//char* cmdStringSend;
		//edict_t* tempEd = ENT(pev);

		//edict_t* tempEd = ENT(tempplayer->pev);
		edict_t* tempEd = pEntity; // we already started with this, use it dangit

		//SERVER_COMMAND
		const char* arg1ref = CMD_ARGV(1);
		
		char arg1refFiltered[128];
		strncpy( &arg1refFiltered[0], &arg1ref[0], 127 );
		arg1refFiltered[127] = '\0';
		lowercase(&arg1refFiltered[0]);


		if(FStrEq(arg1refFiltered, "stop") || FStrEq(arg1refFiltered, "off" ) ){
			//don't try to play.  This is a code to stop any MP3 music playing now.
			submitJukeboxOff(pev);
			return;
		}


		if(isStringEmpty(arg1ref)){
			//no arguments?  Then we play a random HL.
			
			int choice = RANDOM_LONG(1, 17);
			/*
			if ( !pEntity->pvPrivateData )
				return;

			entvars_t *pev = &pEntity->v;
			*/

			//let's be careful with itoa or atoi...?
			//itoa(

			//Unwise to do it this way - any printouts between now and later will cause cmdStringSend to store that printout's contents instead.
			//The string buffer used in UTIL_VarArgs is shared throughout all calls to it. It is assumed it won't get reused until another printout
			//is made.
			//Not as bad here, but for future use keep this in mind.
			//sprintf does fine here.
			//cmdStringSend = UTIL_VarArgs("mp3 play media/Half-Life%02d.mp3", 0, choice);
			
			sprintf(&tempCommandBuffer[0], "mp3 play media/Half-Life%02d.mp3", choice);
			easyForcePrintLineClient(pEntity, "RESULT: %s", tempCommandBuffer);

		}else{

			//UNFORTUNATELY, this whole ignore case for playing MP3 files is pointless.
			//The M
			if(stringEndsWithIgnoreCase(arg1ref, ".mp3")){
				sprintf(&tempCommandBuffer[0], "mp3 play media/%s", arg1ref);
			}else{
				//doesn't end with .mp3?  add it there.
				sprintf(&tempCommandBuffer[0], "mp3 play media/%s.mp3", arg1ref);
			}

		}
		//CLIENT_COMMAND(pEntity, cmdStringSend);
		//CLIENT_COMMAND(tempEd, "mp3 play media/Half-Life11.mp3");
		//CLIENT_COMMAND(tempEd, cmdStringSend);
		submitJukeboxRequest(pev, tempCommandBuffer);


	}else if( FStrEq(pcmdRefinedRef, "lazy") || FStrEq(pcmdRefinedRef, "lazyass") || FStrEq(pcmdRefinedRef, "wakeup") || FStrEq(pcmdRefinedRef, "wakethefuckup") || FStrEq(pcmdRefinedRef, "rise") || FStrEq(pcmdRefinedRef, "revive") || FStrEq(pcmdRefinedRef, "risefromthegrave") || FStrEq(pcmdRefinedRef, "getthefuckup") || FStrEq(pcmdRefinedRef, "suprisemotherfucker") || FStrEq(pcmdRefinedRef, "suprisemothafucka") || FStrEq(pcmdRefinedRef, "suprisemotherfucka") || FStrEq(pcmdRefinedRef, "suprisemothafucker") || FStrEq(pcmdRefinedRef, "getup") || FStrEq(pcmdRefinedRef, "younodie") || FStrEq(pcmdRefinedRef, "myparentsarenthome")   ){
		
		//First a check. Is the player dead? If so this is a self-revive.
		if(tempplayer->pev->deadflag != DEAD_NO){
			if(g_flWeaponCheat != 0.0){
				//can't do this without cheats.
				
				tempplayer->stopSelfSounds();

				tempplayer->Spawn(TRUE);
			}else{
				easyForcePrintLineClient(pEntity, "Cheaters stay dead!");
			}
		}else{
			//Time to play necromancer. Perhaps.
			if(g_flWeaponCheat == 0.0){
				easyForcePrintLineClient(pEntity, "Necromancy is not your strongpoint.");
				return;
			}
			CBaseEntity* forwardEnt = FindEntityForward(tempplayer);
			if(forwardEnt != NULL){
				CBaseMonster* tempMon;
				tempMon = forwardEnt->GetMonsterPointer();
				if(tempMon == NULL){
					easyForcePrintLineClient(pEntity, "ERROR: Thing in crosshairs is not a monster: %s.", forwardEnt->getClassname());
					return;
				}
				if(tempMon->pev->deadflag < DEAD_DEAD){
					//dying or NO? Deny.
					easyForcePrintLineClient(pEntity, "That isn\'t dead yet, genius.");
					return;
				}
				tempMon->StartReanimation();
			}else{
				easyForcePrintLineClient(pEntity, "ERROR: Could not find an entity / monster in crosshairs.");
			}
		}//END OF player dead check else.

	}else if (FStrEq(pcmdRefinedRef, "reviveall") || FStrEq(pcmdRefinedRef, "necromancy") || FStrEq(pcmdRefinedRef, "thriller") || FStrEq(pcmdRefinedRef, "thisisthriller") || FStrEq(pcmdRefinedRef, "cuzthisisthriller") || FStrEq(pcmdRefinedRef, "causethisisthriller") ) {

		reviveAllMonsters(pEntity);

	}else if( FStrEq(pcmdRefinedRef, "ent_remove") || FStrEq(pcmdRefinedRef, "remove") || FStrEq(pcmdRefinedRef, "removeent") || FStrEq(pcmdRefinedRef, "removeentity") || FStrEq(pcmdRefinedRef, "entremove") || FStrEq(pcmdRefinedRef, "entityremove") || FStrEq(pcmdRefinedRef, "kaplow") || FStrEq(pcmdRefinedRef, "ihateyou") || FStrEq(pcmdRefinedRef, "begone") || FStrEq(pcmdRefinedRef, "begonepeasant") || FStrEq(pcmdRefinedRef, "bye") || FStrEq(pcmdRefinedRef, "byebye")){
		if(g_flWeaponCheat == 0.0){
			easyForcePrintLineClient(pEntity, "Can\'t remove things without cheats.");
			return;
		}
		CBaseEntity* pEntityToRemove;
		
		// NOTE - can supply arg now.
		if (CMD_ARGC() > 1) {
			// get an entity with that ID to remove
			int theID;
			int tempNumb;
			try {
				tempNumb = tryStringToInt(CMD_ARGV(1));
				theID = tempNumb;
				// Nope, get the entity of this ID instead (these ID's come from the engines)
				pEntityToRemove = getEntityWithID(tempNumb);
				if (pEntityToRemove != NULL) {
					// ok
				}else {
					// oh.
					easyForcePrintLineClient(pEntity, "No entity of ID %d found.", theID);
				}
			}catch (int) {
				easyForcePrintClient(pEntity, "ERROR: ID given invalid. Must be integer.");
			}
		}else {
			// just look for something in front of me.
			pEntityToRemove = FindEntityForward(tempplayer);
			if (pEntityToRemove != NULL) {
				// ok.
			}else {
				// oh.
				easyForcePrintClient(pEntity, "Could not find something in front to remove!");
			}
		}
		if ( pEntityToRemove ){
			//SUB_Remove calls 'onDelete' already
			pEntityToRemove->pev->nextthink = gpGlobals->time;
			pEntityToRemove->SetThink(&CBaseEntity::SUB_Remove);
		}else{
			// error message already handled.
		}
	}else if( FStrEq(pcmdRefinedRef, "monster_remove") || FStrEq(pcmdRefinedRef, "removemonster") || FStrEq(pcmdRefinedRef, "removemon") || FStrEq(pcmdRefinedRef, "mon_remove") || FStrEq(pcmdRefinedRef, "monremove")){
		if(g_flWeaponCheat == 0.0){
			easyForcePrintLineClient(pEntity, "Can\'t remove things without cheats.");
			return;
		}
		CBaseEntity* pEntityToRemove;
		
		// NOTE - can supply arg now.
		if (CMD_ARGC() > 1) {
			// get an entity with that ID to remove
			int theID;
			int tempNumb;
			try {
				tempNumb = tryStringToInt(CMD_ARGV(1));
				theID = tempNumb;
				// monster of ID?
				pEntityToRemove = getMonsterWithID(tempNumb);
				if (pEntityToRemove != NULL) {
					// ok
				}else {
					// oh.
					easyForcePrintLineClient(pEntity, "No entity of ID %d found.", theID);
				}
			}catch (int) {
				easyForcePrintClient(pEntity, "ERROR: ID given invalid. Must be integer.");
			}
		}else {
			// just look for something in front of me.
			pEntityToRemove = FindEntityForward(tempplayer);
			if (pEntityToRemove != NULL) {
				// ok.
			}else {
				// oh.
				easyForcePrintClient(pEntity, "Could not find something in front to remove!");
			}
		}
		if ( pEntityToRemove ){
			//SUB_Remove calls 'onDelete' already
			pEntityToRemove->pev->nextthink = gpGlobals->time;
			pEntityToRemove->SetThink(&CBaseEntity::SUB_Remove);
		}else{
			// error message already handled.
		}
	}else if( FStrEq(pcmdRefinedRef, "entdrawbounds") || FStrEq(pcmdRefinedRef, "ent_drawbounds") || FStrEq(pcmdRefinedRef, "drawentbounds") ){
		// not to be confused with "showbounds", that one's mad old
		if(g_flWeaponCheat == 0.0){
			easyForcePrintLineClient(pEntity, "no drawin");
			return;
		}
		CBaseEntity* pEntityToDraw;

		// NOTE - can supply arg now.
		if (CMD_ARGC() > 1) {
			// get an entity with that ID to draw
			int theID;
			int tempNumb;
			try {
				tempNumb = tryStringToInt(CMD_ARGV(1));
				theID = tempNumb;
				// monster of ID?
				pEntityToDraw = getEntityWithID(tempNumb);
				if (pEntityToDraw != NULL) {
					// ok
				}else {
					// oh.
					easyForcePrintLineClient(pEntity, "No entity of ID %d found.", theID);
				}
			}catch (int) {
				easyForcePrintClient(pEntity, "ERROR: ID given invalid. Must be integer.");
			}
		}else {
			// just look for something in front of me.
			pEntityToDraw = FindEntityForward(tempplayer);
			if (pEntityToDraw != NULL) {
				// ok.
			}else {
				// oh.
				easyForcePrintClient(pEntity, "Could not find something in front to draw!");
			}
		}
		if ( pEntityToDraw ){
			UTIL_drawBoxFrame(pEntityToDraw->pev->absmin, pEntityToDraw->pev->absmax, 12, 800, 255, 255, 255);
		}else{
			// error message already handled.
		}
	}else if( FStrEq(pcmdRefinedRef, "health") || FStrEq(pcmdRefinedRef, "gethealth") || FStrEq(pcmdRefinedRef, "sethealth")  ){
		const char* arg1ref = CMD_ARGV(1);
		if(g_flWeaponCheat == 0.0){
			easyForcePrintLineClient(pEntity, "No health trickery for you, cheater!");
			return;
		}

		//ambiguous as to whether this is what is in the crosshairs or the player itself.  Try to figure it out:
		CBaseEntity* forwardEnt = FindEntityForward(tempplayer);

		if(forwardEnt == NULL){
			//assume this is for the player.
			interpretAsHealth(pEntity, tempplayer, arg1ref, "Client");

		}else{
			interpretAsHealth(pEntity, forwardEnt, arg1ref, STRING(forwardEnt->pev->classname) );
			//easyForcePrintLineClient(pEntity, "MODEL: %s", STRING(forwardEnt->pev->model));
		}
	}else if( FStrEq(pcmdRefinedRef, "setmyhealth") || FStrEq(pcmdRefinedRef, "setplayerhealth") || FStrEq(pcmdRefinedRef, "getmyhealth") || FStrEq(pcmdRefinedRef, "getplayerhealth") || FStrEq(pcmdRefinedRef, "myhealth") || FStrEq(pcmdRefinedRef, "playerhealth")  ){
		const char* arg1ref = CMD_ARGV(1);
		//tryStringToInt

		if(g_flWeaponCheat == 0.0){
			easyForcePrintLineClient(pEntity, "No health trickery for you, cheater!");
			return;
		}

		interpretAsHealth(pEntity, (CBaseEntity*)tempplayer, arg1ref, "Client" );

	}else if( FStrEq(pcmdRefinedRef, "setenthealth") || FStrEq(pcmdRefinedRef, "setmonsterhealth") ||  FStrEq(pcmdRefinedRef, "setentityhealth") || FStrEq(pcmdRefinedRef, "getenthealth") || FStrEq(pcmdRefinedRef, "getmonsterhealth") ||  FStrEq(pcmdRefinedRef, "getentityhealth") || FStrEq(pcmdRefinedRef, "monsterhealth") || FStrEq(pcmdRefinedRef, "enthealth") || FStrEq(pcmdRefinedRef, "entityhealth")  ){
		const char* arg1ref = CMD_ARGV(1);
		
		if(g_flWeaponCheat == 0.0){
			easyForcePrintLineClient(pEntity, "No health trickery for you, cheater!");
			return;
		}

		CBaseEntity* forwardEnt = FindEntityForward(tempplayer);

		if(forwardEnt != NULL){
			interpretAsHealth(pEntity, forwardEnt, arg1ref, STRING(forwardEnt->pev->classname) );
		}else{
			easyForcePrintLineClient(pEntity, "ERROR: Could not find an entity / monster in crosshairs.");
		}
	}else if( FStrEq(pcmdRefinedRef, "battery") || FStrEq(pcmdRefinedRef, "getbattery") || FStrEq(pcmdRefinedRef, "setbattery") || FStrEq(pcmdRefinedRef, "armor") || FStrEq(pcmdRefinedRef, "getarmor") || FStrEq(pcmdRefinedRef, "setarmor")){
		const char* arg1ref = CMD_ARGV(1);
		if(g_flWeaponCheat == 0.0){
			easyForcePrintLineClient(pEntity, "No battery trickery for you, cheater!");
			return;
		}
		//ambiguous as to whether this is what is in the crosshairs or the player itself.  Try to figure it out:
		CBaseEntity* forwardEnt = FindEntityForward(tempplayer);

		if (forwardEnt == NULL) {
			//assume this is for the player.
			interpretAsBattery(pEntity, tempplayer, arg1ref, "Client");
		}
		else {
			interpretAsBattery(pEntity, forwardEnt, arg1ref, STRING(forwardEnt->pev->classname));
		}

	}else if( FStrEq(pcmdRefinedRef, "setmybattery") || FStrEq(pcmdRefinedRef, "setplayerbattery") || FStrEq(pcmdRefinedRef, "getmybattery") || FStrEq(pcmdRefinedRef, "getplayerbattery") || FStrEq(pcmdRefinedRef, "mybattery") || FStrEq(pcmdRefinedRef, "playerbattery")    ||    FStrEq(pcmdRefinedRef, "setmyarmor") || FStrEq(pcmdRefinedRef, "setplayerarmor") || FStrEq(pcmdRefinedRef, "getmyarmor") || FStrEq(pcmdRefinedRef, "getplayerarmor") || FStrEq(pcmdRefinedRef, "myarmor") || FStrEq(pcmdRefinedRef, "playerarmor")){
		const char* arg1ref = CMD_ARGV(1);
		//tryStringToInt
		if(g_flWeaponCheat == 0.0){
			easyForcePrintLineClient(pEntity, "No battery trickery for you, cheater!");
			return;
		}

		interpretAsBattery(pEntity, (CBaseEntity*)tempplayer, arg1ref, "Client" );

	}else if( FStrEq(pcmdRefinedRef, "setentbattery") || FStrEq(pcmdRefinedRef, "setmonsterbattery") ||  FStrEq(pcmdRefinedRef, "setentitybattery") || FStrEq(pcmdRefinedRef, "getentbattery") || FStrEq(pcmdRefinedRef, "getmonsterbattery") ||  FStrEq(pcmdRefinedRef, "getentitybattery") || FStrEq(pcmdRefinedRef, "monsterbattery") || FStrEq(pcmdRefinedRef, "entbattery") || FStrEq(pcmdRefinedRef, "entitybattery")    ||    FStrEq(pcmdRefinedRef, "setentarmor") || FStrEq(pcmdRefinedRef, "setmonsterarmor") || FStrEq(pcmdRefinedRef, "setentityarmor") || FStrEq(pcmdRefinedRef, "getentarmor") || FStrEq(pcmdRefinedRef, "getmonsterarmor") || FStrEq(pcmdRefinedRef, "getentityarmor") || FStrEq(pcmdRefinedRef, "monsterarmor") || FStrEq(pcmdRefinedRef, "entarmor") || FStrEq(pcmdRefinedRef, "entityarmor")){
		const char* arg1ref = CMD_ARGV(1);
		if(g_flWeaponCheat == 0.0){
			easyForcePrintLineClient(pEntity, "No battery trickery for you, cheater!");
			return;
		}

		CBaseEntity* forwardEnt = FindEntityForward(tempplayer);

		if(forwardEnt != NULL){
			interpretAsBattery(pEntity, forwardEnt, arg1ref, STRING(forwardEnt->pev->classname) );
		}else{
			easyForcePrintLineClient(pEntity, "ERROR: Could not find an entity / monster in crosshairs.");
		}
	}else if( FStrEq(pcmdRefinedRef, "id") || FStrEq(pcmdRefinedRef, "getid") || FStrEq(pcmdRefinedRef, "monsterid") || FStrEq(pcmdRefinedRef, "getmonsterid")){
		const char* arg1ref = CMD_ARGV(1);

		
		if(g_flWeaponCheat == 0.0){
			easyForcePrintLineClient(pEntity, "No id trickery for you, cheater!");
			return;
		}

		//ambiguous as to whether this is what is in the crosshairs or the player itself.  Try to figure it out:
		CBaseEntity* forwardEnt = FindEntityForward(tempplayer);

		if(forwardEnt == NULL){
			easyForcePrintLineClient(pEntity, "ERROR: Could not find a monster in crosshairs.");
		}else{
			CBaseMonster* tempMonster = forwardEnt->GetMonsterPointer();
			if(tempMonster == NULL){
				easyForcePrintLineClient(pEntity, "ERROR: Entity in crosshairs, \"%s\", is not a monster or subclass.", STRING(forwardEnt->pev->classname) );
			}else{
				easyForcePrintLineClient(pEntity, "ID:%d Class:%s", tempMonster->monsterID, STRING(tempMonster->pev->classname) );
			}
		}
	
	}else if( FStrEq(pcmdRefinedRef, "getsize") || FStrEq(pcmdRefinedRef, "sizes") ){
		
		const char* arg1ref = CMD_ARGV(1);

		//ambiguous as to whether this is what is in the crosshairs or the player itself.  Try to figure it out:
		CBaseEntity* forwardEnt = FindEntityForward(tempplayer);

		if(forwardEnt == NULL){
			easyForcePrintLineClient(pEntity, "ERROR: Could not find an entity in crosshairs.");
		}else{
			
			easyForcePrintLineClient(pEntity, "------Sizedata of %s------", forwardEnt->getClassname());

			easyForcePrintLineClient(pEntity, "mins: (%.2f %.2f %.2f) maxs: (%.2f %.2f %.2f)", 
				forwardEnt->pev->mins.x,
				forwardEnt->pev->mins.y,
				forwardEnt->pev->mins.z,
				forwardEnt->pev->maxs.x,
				forwardEnt->pev->maxs.y,
				forwardEnt->pev->maxs.z
				);
			Vector temp = (forwardEnt->pev->maxs - forwardEnt->pev->mins);
			easyForcePrintLineClient(pEntity, "delta: (%.2f %.2f %.2f) size: %.2f", temp.x, temp.y, temp.z, (temp.x*temp.y*temp.z));

			easyForcePrintLineClient(pEntity, "absmin: (%.2f %.2f %.2f) absmax: (%.2f %.2f %.2f)", 
				forwardEnt->pev->absmin.x,
				forwardEnt->pev->absmin.y,
				forwardEnt->pev->absmin.z,
				forwardEnt->pev->absmax.x,
				forwardEnt->pev->absmax.y,
				forwardEnt->pev->absmax.z
				);
			temp = (forwardEnt->pev->absmax - forwardEnt->pev->absmin);
			easyForcePrintLineClient(pEntity, "delta: (%.2f %.2f %.2f) size: %.2f", temp.x, temp.y, temp.z, (temp.x*temp.y*temp.z));

			easyForcePrintLineClient(pEntity, "-------------------------"  );

		}

	
	}else if( FStrEq(pcmdRefinedRef, "drawpathtrack") || FStrEq(pcmdRefinedRef, "pathtrackdraw")  || FStrEq(pcmdRefinedRef, "drawtrackpath") || FStrEq(pcmdRefinedRef, "trackpathdraw")  ){
		
		const char* arg1ref = CMD_ARGV(1);
		int trackIDToFind = -1;
		CPathTrack* pathTrackTarget;
		BOOL parseSuccess;

		if(g_flWeaponCheat == 0.0){easyForcePrintLineClient(pEntity, "No.");return;}

		DebugLine_ClearAll();

		parseSuccess = attemptParseStringToInt(pEntity, &trackIDToFind, arg1ref, "ERROR! Input number ID could not be understood.", "ERROR! Must provide path ID to draw.");
		if(!parseSuccess)return;

		pathTrackTarget = getPathTrackWithID(pEntity, trackIDToFind);

		if(pathTrackTarget != NULL){
			::DebugLine_SetupPathTrack(pathTrackTarget);

			//Calling this because paths don't usually get a chance to ever show this information. They are invisible and might not even collide with a player-crosshair traceline anyways.
			pathTrackTarget->ReportGeneric();  
		}//END OF pathTrackTarget check

	
	}else if( FStrEq(pcmdRefinedRef, "teleportpathtrack") || FStrEq(pcmdRefinedRef, "pathtrackteleport") || FStrEq(pcmdRefinedRef, "teleporttrackpath") || FStrEq(pcmdRefinedRef, "trackpathteleport")   ){
		const char* arg1ref = CMD_ARGV(1);
		int trackIDToFind = -1;
		CPathTrack* pathTrackTarget;
		BOOL parseSuccess;

		if(g_flWeaponCheat == 0.0){easyForcePrintLineClient(pEntity, "No.");return;}

		parseSuccess = attemptParseStringToInt(pEntity, &trackIDToFind, arg1ref, "ERROR! Input number ID could not be understood.", "ERROR! Must provide path ID to move to.");
		if(!parseSuccess)return;

		pathTrackTarget = getPathTrackWithID(pEntity, trackIDToFind);

		if(pathTrackTarget != NULL){
			//go there!
			tempplayer->pev->origin = pathTrackTarget->pev->origin;
			easyForcePrintLineClient(pEntity, "Teleport successful.");
		}

	
	}else if( FStrEq(pcmdRefinedRef, "enablepathtrack") || FStrEq(pcmdRefinedRef, "pathtrackenable") || FStrEq(pcmdRefinedRef, "enabletrackpath") || FStrEq(pcmdRefinedRef, "trackpathenable") ){
		const char* arg1ref = CMD_ARGV(1);
		int trackIDToFind = -1;
		CPathTrack* pathTrackTarget;
		BOOL parseSuccess;

		if(g_flWeaponCheat == 0.0){easyForcePrintLineClient(pEntity, "No.");return;}

		parseSuccess = attemptParseStringToInt(pEntity, &trackIDToFind, arg1ref, "ERROR! Input number ID could not be understood.", "ERROR! Must provide path ID to enable.");
		if(!parseSuccess)return;

		pathTrackTarget = getPathTrackWithID(pEntity, trackIDToFind);

		if(pathTrackTarget != NULL){
			if(pathTrackTarget->pev->spawnflags & SF_PATH_DISABLED){
				pathTrackTarget->pev->spawnflags &= ~SF_PATH_DISABLED;
				easyForcePrintLineClient(pEntity, "SUCCESS - path_track:%d enabled.", trackIDToFind);
			}else{
				easyForcePrintLineClient(pEntity, "-path_track:%d already enabled.", trackIDToFind);
			}
		}//END OF pathTrackTarget check

	
	}else if( FStrEq(pcmdRefinedRef, "disablepathtrack") || FStrEq(pcmdRefinedRef, "pathtrackdisable") || FStrEq(pcmdRefinedRef, "disabletrackpath") || FStrEq(pcmdRefinedRef, "trackpathdisable") ){
		
		const char* arg1ref = CMD_ARGV(1);
		int trackIDToFind = -1;
		CPathTrack* pathTrackTarget;
		BOOL parseSuccess;

		if(g_flWeaponCheat == 0.0){easyForcePrintLineClient(pEntity, "No.");return;}

		parseSuccess = attemptParseStringToInt(pEntity, &trackIDToFind, arg1ref, "ERROR! Input number ID could not be understood.", "ERROR! Must provide path ID to disable.");
		if(!parseSuccess)return;

		pathTrackTarget = getPathTrackWithID(pEntity, trackIDToFind);

		if(pathTrackTarget != NULL){
			if( !(pathTrackTarget->pev->spawnflags & SF_PATH_DISABLED) ){
				pathTrackTarget->pev->spawnflags |= SF_PATH_DISABLED;
				easyForcePrintLineClient(pEntity, "SUCCESS - path_track:%d disabled.", trackIDToFind);
			}else{
				easyForcePrintLineClient(pEntity, "-path_track:%d already disabled.", trackIDToFind);
			}
		}//END OF pathTrackTarget check

	
	}else if( FStrEq(pcmdRefinedRef, "drawtrackchange") || FStrEq(pcmdRefinedRef, "trackchangedraw")){
	
		const char* arg1ref = CMD_ARGV(1);
		int trackIDToFind = -1;
		CFuncTrackChange* trackChangeTarget;
		BOOL parseSuccess;

		if(g_flWeaponCheat == 0.0){easyForcePrintLineClient(pEntity, "No.");return;}

		DebugLine_ClearAll();

		parseSuccess = attemptParseStringToInt(pEntity, &trackIDToFind, arg1ref, "ERROR! Input number ID could not be understood.", "ERROR! Must provide trackchange ID to draw.");
		if(!parseSuccess)return;

		trackChangeTarget = getTrackChangeWithID(pEntity, trackIDToFind);

		if(trackChangeTarget != NULL){
			
			DebugLine_Setup(5, trackChangeTarget->pev->origin + Vector(0, 0, -80), trackChangeTarget->pev->origin + Vector(0, 0, 80), 32, 89, 89);

			if(trackChangeTarget->m_trackTop != NULL){
				::DebugLine_SetupPathTrack(trackChangeTarget->m_trackTop);
			}
			if(trackChangeTarget->m_trackBottom != NULL){
				::DebugLine_SetupPathTrack(trackChangeTarget->m_trackBottom);
			}

			//just a good idea.
			trackChangeTarget->ReportGeneric();  

		}//END OF trackChangeTarget check

	
	}else if( FStrEq(pcmdRefinedRef, "teleporttrackchange") || FStrEq(pcmdRefinedRef, "trackchangeteleport")){
	
		const char* arg1ref = CMD_ARGV(1);
		int trackIDToFind = -1;
		CFuncTrackChange* trackChangeTarget;
		BOOL parseSuccess;

		if(g_flWeaponCheat == 0.0){easyForcePrintLineClient(pEntity, "No.");return;}

		parseSuccess = attemptParseStringToInt(pEntity, &trackIDToFind, arg1ref, "ERROR! Input number ID could not be understood.", "ERROR! Must provide trackchange ID to move to.");
		if(!parseSuccess)return;

		trackChangeTarget = getTrackChangeWithID(pEntity, trackIDToFind);

		if(trackChangeTarget != NULL){
			//go there!
			tempplayer->pev->origin = trackChangeTarget->pev->origin;
			easyForcePrintLineClient(pEntity, "Teleport successful.");
		}

	
	}else if( FStrEq(pcmdRefinedRef, "report")  ){
		const char* arg1ref = CMD_ARGV(1);
		CBaseEntity* forwardEnt;
		
		if(g_flWeaponCheat == 0.0){
			easyForcePrintLineClient(pEntity, "No reports for you, sorry.");
			return;
		}

		forwardEnt = FindEntityForward(tempplayer);

		if(forwardEnt != NULL){
			forwardEnt->ReportGeneric();
		}else{
			easyForcePrintLineClient(pEntity, "ERROR: Could not find an entity in crosshairs.");
		}

	
	}else if( FStrEq(pcmdRefinedRef, "reportai") || FStrEq(pcmdRefinedRef, "reportstate") || FStrEq(pcmdRefinedRef, "getai") || FStrEq(pcmdRefinedRef, "getstate") || FStrEq(pcmdRefinedRef, "printai") || FStrEq(pcmdRefinedRef, "printState")  ){
		const char* arg1ref = CMD_ARGV(1);
		CBaseEntity* forwardEnt;
		
		if(g_flWeaponCheat == 0.0){
			easyForcePrintLineClient(pEntity, "No ai trickery for you, cheater!");
			return;
		}


		forwardEnt = FindEntityForward(tempplayer);

		int monsterIDToUse = -1;

		if(arg1ref != NULL && !isStringEmpty(arg1ref)){
			//try parsing it.
			try{
				int numbAttempt = tryStringToInt(arg1ref);
				monsterIDToUse = numbAttempt;
			}catch(int){
				easyForcePrintLineClient(pEntity, "ERROR! Input number ID could not be understood.");
				return;
			}

			if(monsterIDToUse != -1){
				//try to find it.
				forwardEnt = ::getMonsterWithID(monsterIDToUse);
				if(forwardEnt == NULL){
					//still didn't find it?
					easyForcePrintLineClient(pEntity, "ERROR: Monster of ID %d not found.", monsterIDToUse);
					return;
				}
			}
		}//END OF arg check

		if(forwardEnt == NULL){
			//try looking nearby?


		}

		//easyForcePrintLineClient(pEntity, "ARE YOU DAFT MAN?! 1");
		if(forwardEnt != NULL){
			//easyForcePrintLineClient(pEntity, "ARE YOU DAFT MAN?! 2A");
			CBaseMonster* testMon = forwardEnt->GetMonsterPointer();
			if(testMon == NULL){
				easyForcePrintLineClient(pEntity, "ERROR: thing in crosshairs is not a monster.  It is \"%s\".", STRING(forwardEnt->pev->classname) );
				easyForcePrintLineClient(pEntity, "Have some other stats. nextthink:%.2f. ltime:%.2f Current time:%.2f", forwardEnt->pev->nextthink, forwardEnt->pev->ltime, gpGlobals->time);
			}else{
				//got it!
				//easyForcePrintLineClient(pEntity, "ARE YOU DAFT MAN?! 3");
				testMon->ReportAIState();

				//testMon->pev->waterlevel = 1;  //does foricng it work?
				//..no.

				//easyForcePrintLineClient(pEntity, "ARE YOU DAFT MAN?! 4");
			}
		}else{
			//easyForcePrintLineClient(pEntity, "ARE YOU DAFT MAN?! 2B");
			easyForcePrintLineClient(pEntity, "ERROR: Could not find an entity / monster in crosshairs.");
		}

	}else if( FStrEq(pcmdRefinedRef, "squadinfo") || FStrEq(pcmdRefinedRef, "squad") || FStrEq(pcmdRefinedRef, "reportsquad") || FStrEq(pcmdRefinedRef, "squadreport") || FStrEq(pcmdRefinedRef, "squadstate")  ){
		CBaseEntity* forwardEnt = FindEntityForward(tempplayer);
	
		if(g_flWeaponCheat == 0.0){
			easyForcePrintLineClient(pEntity, "No squad trickery for you, cheater!");
			return;
		}

		if(forwardEnt == NULL){
			easyForcePrintLineClient(pEntity, "SquadInfo failed: nothing found?  Aim with crosshairs.");
			return;
		}

		CSquadMonster* squadTest = forwardEnt->MySquadMonsterPointer();

		if(squadTest != NULL){
			const char* netnameSafe;

			if(FStringNull(squadTest->pev->netname) || ((netnameSafe = STRING(squadTest->pev->netname)) == NULL) ){
				netnameSafe = "__NULL__";
			}
			//otherwise, trust that "netnameSafe" is ok.

			easyForcePrintLineClient(pEntity, "***SQUADINFO FOR %s:%d deadflag:%d hasSquadCapaBit:%d netname:%s***", squadTest->getClassname(), squadTest->monsterID, squadTest->pev->deadflag, (squadTest->m_afCapability & bits_CAP_SQUAD)!=0, netnameSafe  );

			if(squadTest->InSquad() == FALSE){
				easyForcePrintLineClient(pEntity, "...not in squad.");
				return;
			}

			CSquadMonster* leader = squadTest->MySquadLeader();
			if(leader == NULL){
				easyForcePrintLineClient(pEntity, "Error 24.  no leader...?");
			}

			if(squadTest->IsLeader()){
				easyForcePrintLineClient(pEntity, "Leader: -ME!- %s:%d deadflag:%d", leader->getClassname(), leader->monsterID, leader->pev->deadflag);
			}else{
				easyForcePrintLineClient(pEntity, "Leader: %s:%d deadflag:%d", leader->getClassname(), leader->monsterID, leader->pev->deadflag);
			}

			for(int i = 0; i < 4; i++){
				CSquadMonster* thisMember = leader->MySquadMember(i);
				if(thisMember == NULL){
					easyForcePrintLineClient(pEntity, "Slot %d: EMPTY", i);
				}else{
					if(thisMember == squadTest){
						easyForcePrintLineClient(pEntity, "Slot %d: -Me!- %s:%d deadflag:%d", i, thisMember->getClassname(), thisMember->monsterID, thisMember->pev->deadflag);
					}else{
						easyForcePrintLineClient(pEntity, "Slot %d: %s:%d deadflag:%d", i, thisMember->getClassname(), thisMember->monsterID, thisMember->pev->deadflag);
					}
				}

			}
			easyForcePrintLineClient(pEntity, "***END OF SQUADINFO***");
		}else{
			easyForcePrintLineClient(pEntity, "SquadInfo failed: not a SquadMonster!  Thing picked: %s", forwardEnt->getClassname() );
		}
	}else if(FStrEq(pcmdRefinedRef, "reportnetname")){
		const char* arg1ref = CMD_ARGV(1);

		//ambiguous as to whether this is what is in the crosshairs or the player itself.  Try to figure it out:
		CBaseEntity* forwardEnt = FindEntityForward(tempplayer);

		if(forwardEnt == NULL){
			easyForcePrintLineClient(pEntity, "ERROR: Could not find a monster in crosshairs.");
		}else{
			CBaseMonster* tempMonster = forwardEnt->GetMonsterPointer();
			if(tempMonster == NULL){
				easyForcePrintLineClient(pEntity, "ERROR: Entity in crosshairs, \"%s\", is not a monster or subclass.", STRING(forwardEnt->pev->classname) );
			}else{
				tempMonster->reportNetName();
			}
		}
		
	}else if( FStrEq(pcmdRefinedRef, "selfquery") || FStrEq(pcmdRefinedRef, "queryself") || FStrEq(pcmdRefinedRef, "reportself") ){
		easyForcePrintLineClient(pEntity, "PLAYER STATS:");
		tempplayer->ReportAIState();
	
	}else if( FStrEq(pcmdRefinedRef, "gib") || FStrEq(pcmdRefinedRef, "gibbed") || FStrEq(pcmdRefinedRef, "gibme") || FStrEq(pcmdRefinedRef, "diehard") || FStrEq(pcmdRefinedRef, "diehard") || FStrEq(pcmdRefinedRef, "explode") || FStrEq(pcmdRefinedRef, "asplode") || FStrEq(pcmdRefinedRef, "yourheadasplode") || FStrEq(pcmdRefinedRef, "myheadasplode") ||  FStrEq(pcmdRefinedRef, "headasplode") || FStrEq(pcmdRefinedRef, "yourheadexplode") || FStrEq(pcmdRefinedRef, "myheadexplode") ||  FStrEq(pcmdRefinedRef, "headexplode") ){
		if(g_flWeaponCheat == 0.0){
			easyForcePrintLineClient(pEntity, "No exploding.");
			return;
		}

		//BOOL godModeMem = tempplayer->pev->flags & FL_GODMODE;
		//if(godModeMem){
		//	tempplayer->pev->flags &= ~FL_GODMODE;
		//}

		//"DMG_FALL" in there so we ignore battery.
		//tempplayer->TakeDamage(VARS(eoNullEntity), VARS(eoNullEntity), 255, DMG_FALL|DMG_ALWAYSGIB, 0);

		// straight-shot the health change and Killed() call to skip possible interference from godmode and "NothingHurts".
		tempplayer->pev->health = -200;
		tempplayer->Killed(tempplayer->pev, tempplayer->pev, GIB_ALWAYS);

		//if(godModeMem){
		//	tempplayer->pev->flags |= FL_GODMODE;
		//}
	}else if( FStrEq(pcmdRefinedRef, "dividebyzero") || FStrEq(pcmdRefinedRef, "crash") || FStrEq(pcmdRefinedRef, "enditalready") || FStrEq(pcmdRefinedRef, "thissucks") || FStrEq(pcmdRefinedRef, "iwantmydesktop") || FStrEq(pcmdRefinedRef, "iwannagohome")   ){
		
		if(g_flWeaponCheat == 0.0){
			easyForcePrintLineClient(pEntity, "Oh no you don\'t.");
			return;
		}

		const char* arg1ref = CMD_ARGV(1);

		if(arg1ref != NULL && !isStringEmpty(arg1ref)){
			
			try{
				int crashType = tryStringToInt(arg1ref);

				switch(crashType){
				case 0:{
					//Classic: divide by 0.
					int i = 1337;
					int zero = 0;

					int crashTime = i / zero;
				break;}
				case 1:{
					//dereferencing a NULL pointer.
					CrashTest* somePointer = NULL;
					CrashTest someThing = (*somePointer);

				break;}
				case 2:{
					//trying to access methods of a NULL pointer (of object type, has methods).
					CrashTest* somePointer = NULL;
					somePointer->someMethod();

				break;}
				case 3:{
					//dereferencing a pointer to deleted memory.
					CrashTest* somePointer = NULL;
					somePointer = new CrashTest();
					delete somePointer;

					CrashTest someThing = (*somePointer);

				break;}
				case 4:{
					//trying to access methods of a pointer to deleted memory (of object type, has methods)
					CrashTest* somePointer = NULL;
					somePointer = new CrashTest();
					delete somePointer;
					
					somePointer->someMethod();
					
				break;}
				case 5:{
					crashableEntityRef = NULL;

					crashableEntityReferTime = gpGlobals->time + 1.0f;
					crashableEntityReferMode = 5;

					easyForcePrintLineClient(pEntity, "You are doomed in T minus 1 seconds.");

				break;}
				case 6:{
					crashableEntityRef = NULL;

					crashableEntityReferTime = gpGlobals->time + 1.0f;
					crashableEntityReferMode = 6;
					
					easyForcePrintLineClient(pEntity, "You are doomed in T minus 1 seconds.");
				break;}
				case 7:{
					crashableEntityRef = CBaseEntity::CreateManual("monster_chumtoad", Vector(0,0,0), Vector(0,0,0), NULL);
					UTIL_Remove(crashableEntityRef);

					crashableEntityReferTime = gpGlobals->time + 1.0f;
					crashableEntityReferMode = 5;
					
					easyForcePrintLineClient(pEntity, "You are doomed in T minus 1 seconds.");
				break;}
				case 8:{
					crashableEntityRef = CBaseEntity::CreateManual("monster_chumtoad", Vector(0,0,0), Vector(0,0,0), NULL);
					UTIL_Remove(crashableEntityRef);

					crashableEntityReferTime = gpGlobals->time + 1.0f;
					crashableEntityReferMode = 6;
					
					easyForcePrintLineClient(pEntity, "You are doomed in T minus 1 seconds.");
				break;}
				default:{
					easyForcePrintLineClient(pEntity, "***Crash mode unrecognized.  Try a value from 0 to 8 as of the time of writing.");
				break;}

				}//END OF switch

			}catch(int){
				easyForcePrintLineClient(pEntity, "***I don\'t know how to crash like that.  Yes, really.  <invalid mode>");
			}

		}else{
			easyForcePrintLineClient(pEntity, "Tell me how to crash! enter a number after that command.");
		}



	}else if( FStrEq(pcmdRefinedRef, "tpnode") || FStrEq(pcmdRefinedRef, "teleporttonode") || FStrEq(pcmdRefinedRef, "nodeteleport") || FStrEq(pcmdRefinedRef, "nodetp")){
	
		if(g_flWeaponCheat == 0.0){
			easyForcePrintLineClient(pEntity, "Teleportation on a whim is not normal, mortal.");
			return;
		}

		const char* arg1ref = CMD_ARGV(1);
		
		try{
			int numbAttempt = tryStringToInt(arg1ref);
			
			if(numbAttempt >= 0 && numbAttempt < WorldGraph.m_cNodes){
				tempplayer->pev->origin = -tempplayer->pev->mins + WorldGraph.m_pNodes[numbAttempt].m_vecOriginPeek;
			}else{
				easyForcePrintLineClient(pEntity, "ERROR: node index out of bounds!");
			}

		}catch(int){
			easyForcePrintLineClient(pEntity, "Problem reading number.  (arg must be whole number)");
		}

	}else if( FStrEq(pcmdRefinedRef, "tpmonster") || FStrEq(pcmdRefinedRef, "teleporttomonster") || FStrEq(pcmdRefinedRef, "monsterteleport") || FStrEq(pcmdRefinedRef, "monstertp" ) ){
	
		if(g_flWeaponCheat == 0.0){
			easyForcePrintLineClient(pEntity, "Teleportation on a whim is not normal, mortal.");
			return;
		}
		
		const char* arg1ref = CMD_ARGV(1);
		
		try{
			int numbAttempt = tryStringToInt(arg1ref);
			
			CBaseMonster* attemptedMonsterFetch = getMonsterWithID(numbAttempt);

			if(attemptedMonsterFetch != 0){
				easyForcePrintLineClient(pEntity, "Found it.");
				tempplayer->pev->origin = attemptedMonsterFetch->pev->origin;
			}else{
				easyForcePrintLineClient(pEntity, "The map says we\'re fucked!");
			}

		}catch(int){
			easyForcePrintLineClient(pEntity, "Problem reading number.  (arg must be whole number)");
		}

	}else if( FStrEq(pcmdRefinedRef, "nodecon") || FStrEq(pcmdRefinedRef, "nodeconnections") || FStrEq(pcmdRefinedRef, "shownodecon") || FStrEq(pcmdRefinedRef, "shownodeconnections") || FStrEq(pcmdRefinedRef, "nodeneighbors")){
	
		if(g_flWeaponCheat == 0.0){
			easyForcePrintLineClient(pEntity, "Hm. No.");
			return;
		}

			const char* arg1ref = CMD_ARGV(1);
		
			try{
				int numbAttempt = tryStringToInt(arg1ref);
				WorldGraph.ShowNodeConnections(numbAttempt);
			}catch(int){
				easyForcePrintLineClient(pEntity, "Problem reading number.  (arg must be whole number)");
			}

	}else if( FStrEq(pcmdRefinedRef, "getnode") || FStrEq(pcmdRefinedRef, "getclosestnode") || FStrEq(pcmdRefinedRef, "getnearnode") || FStrEq(pcmdRefinedRef, "getnearestnode") || FStrEq(pcmdRefinedRef, "node") || FStrEq(pcmdRefinedRef, "closestnode") || FStrEq(pcmdRefinedRef, "nearnode") || FStrEq(pcmdRefinedRef, "nearestnode")   ){
			
		BOOL special = FALSE;
		BOOL ai = FALSE;
		BOOL target = FALSE;

		if(g_flWeaponCheat == 0.0){
			easyForcePrintLineClient(pEntity, "Hm. No.");
			return;
		}

		for(int i = 0; i < 3; i++){
			const char* thisArg = CMD_ARGV(i+1);

			if(!isStringEmpty(thisArg)){
				char thisArgRefined[128];
				strncpy( &thisArgRefined[0], &thisArg[0], 127 );
				thisArgRefined[127] = '\0';
				lowercase(&thisArgRefined[0]);
				const char* thisArgRefindeRef = thisArgRefined;

				if(FStrEq(thisArgRefindeRef, "special")){
					special = TRUE;
				}else if(FStrEq(thisArgRefindeRef, "ai")){
					ai = TRUE;
				}else if(FStrEq(thisArgRefindeRef, "target")){
					target = TRUE;
				}else{
					easyForcePrintLineClient(pEntity, "WARNING: arg \"%s\" unknown",  thisArg);
				}
			}
		}

		
		/*
		const char* arg1ref = CMD_ARGV(1);
		const char* arg2ref = CMD_ARGV(2);
		
		if( stringEndsWith(pcmdRefinedRef, "special") || (!isStringEmpty(arg1ref) && FStrEq(arg1ref, "special"))  ){
			//save that!
			special = TRUE;
		}
		if( stringEndsWith(pcmdRefinedRef, "ai") || (!isStringEmpty(arg1ref) && FStrEq(arg1ref, "ai"))  ){
			//save that!
			special = TRUE;
		}

		if( (stringEndsWith(pcmdRefinedRef, "specialai") || (!isStringEmpty(arg1ref) && FStrEq(arg1ref, "specialai"))  )
			|| (stringEndsWith(pcmdRefinedRef, "aispecial") || (!isStringEmpty(arg1ref) && FStrEq(arg1ref, "aispecial")))
			|| (FStrEq(arg2ref, "ai") && FStrEq(arg2ref, "special"))
			|| (FStrEq(arg2ref, "special") && FStrEq(arg2ref, "ai"))){
			//save that!
			special = TRUE;
			ai = TRUE;
		}
		*/
		
		Vector vecSearchStart;
		if(!target){
			//player's position.
			vecSearchStart = tempplayer->pev->origin;
		}else{
			//get where the player is looking...
			TraceResult tr;

			UTIL_MakeVectors(pev->v_angle + pev->punchangle);

			//a tiny bit in front for safety.
			Vector vecSrc = pev->origin + pev->view_ofs + gpGlobals->v_forward * 5;
			Vector vecDest = pev->origin + pev->view_ofs + gpGlobals->v_forward * 2048;

			//nah, precision for while ducking not necessary.
			/*
			Vector playerEyePos = Vector(tempplayer->body;
			if(pev->flags & FL_DUCKING){

			}
			*/

			UTIL_TraceLine( vecSrc, vecDest, dont_ignore_monsters, tempplayer->edict(), &tr );
			//tr.vecEndPos();

			if (tr.fAllSolid){
				easyForcePrintLine ( "ERROR: solid trace line?" );
				return;
			}
			
			vecSearchStart = tr.vecEndPos;
		}

		if(ai == TRUE){
			int nodeTest = WorldGraph.FindNearestNode(vecSearchStart, tempplayer);

				
			if(nodeTest == -1){
				easyForcePrintLineClient(pEntity, "***ERROR: no nearby acceptable nodes.***");
			}else{
				if(target){
					easyForcePrintLineClient(pEntity, "*AI: Target is near node #%d", nodeTest);
				}else{
					easyForcePrintLineClient(pEntity, "*AI: Client is near node #%d", nodeTest);
				}
				
				if(special){
					EASY_CVAR_SET_DEBUGONLY(drawNodeSpecial, nodeTest);
					EASY_CVAR_SET_DEBUGONLY(drawNodeAlternateTime, 0);
					g_queueCVarHiddenSave = TRUE;
				}
			}
		}else{
			//the ol' fashioned way.
			int closestNodeID = -1;
			float closestNodeDist = 99999999;
			for(int i = 0; i < WorldGraph.m_cNodes; i++){
				float thisDist = (WorldGraph.m_pNodes[i].m_vecOrigin - vecSearchStart).Length();
				if(thisDist < closestNodeDist){
					closestNodeID = i;
					closestNodeDist = thisDist;
				}
			}//END OF for(int i...)
			if(closestNodeID == -1){
				easyForcePrintLineClient(pEntity, "***ERROR: closest to no node at all.  (?)");
			}else{
				if(target){
					easyForcePrintLineClient(pEntity, "Target is closest to node #%d", closestNodeID);
				}else{
					easyForcePrintLineClient(pEntity, "Client is closest to node #%d", closestNodeID);
				}
				if(special){
					EASY_CVAR_SET_DEBUGONLY(drawNodeSpecial, closestNodeID);
					EASY_CVAR_SET_DEBUGONLY(drawNodeAlternateTime, 0);
					g_queueCVarHiddenSave = TRUE;
				}
			}
		}

	}else if( FStrEq(pcmdRefinedRef, "nodebugnode") || FStrEq(pcmdRefinedRef, "debugnodeoff") || FStrEq(pcmdRefinedRef, "debugnodesoff") || FStrEq(pcmdRefinedRef, "debugnodemodeoff")  || FStrEq(pcmdRefinedRef, "nonodedebug") || FStrEq(pcmdRefinedRef, "nodedebugoff") || FStrEq(pcmdRefinedRef, "nodesdebugoff") || FStrEq(pcmdRefinedRef, "nodedebugmodeoff")   ){
		debugNodeModeOff();
	}else if( stringStartsWith(pcmdRefinedRef, "debugnode") || stringStartsWith(pcmdRefinedRef, "debugnodes") || stringStartsWith(pcmdRefinedRef, "debugnodemode") || stringStartsWith(pcmdRefinedRef, "nodedebug") || stringStartsWith(pcmdRefinedRef, "nodesdebug") || stringStartsWith(pcmdRefinedRef, "nodedebugmode")   ){
		
		
		if(g_flWeaponCheat == 0.0){
			easyForcePrintLineClient(pEntity, "Hm. No.");
			return;
		}

		const char* arg1ref = CMD_ARGV(1);
		if( stringEndsWith(pcmdRefinedRef, "weak") || stringEndsWith(pcmdRefinedRef, "soft") || stringEndsWith(pcmdRefinedRef, "lite") || (!isStringEmpty(arg1ref) && (FStrEq(arg1ref, "weak")) || (FStrEq(arg1ref, "soft")) || (FStrEq(arg1ref, "lite")) )  ){
			debugNodeModeWeak();
		}else{
			debugNodeMode();
		}


	}else if( FStrEq(pcmdRefinedRef, "printoutisolatednodes") || FStrEq(pcmdRefinedRef, "printisolatednodes") || FStrEq(pcmdRefinedRef, "printoutdeadnodes") || FStrEq(pcmdRefinedRef, "printdeadnodes")  || FStrEq(pcmdRefinedRef, "printoutorphannodes") || FStrEq(pcmdRefinedRef, "printorphannodes")    ){
		
		if(g_flWeaponCheat == 0.0){
			easyForcePrintLineClient(pEntity, "Hm. No.");
			return;
		}

		BOOL printedOutYet = FALSE;
		for(int i = 0; i < WorldGraph.m_cNodes; i++){
			if(WorldGraph.m_pNodes[i].m_cNumLinks < 1){
				if(!printedOutYet){
					easyForcePrintLineClient(pEntity, "***Isolated Nodes***");
					printedOutYet =  TRUE;
				}
				easyForcePrintLineClient(pEntity, "-Node #%d", i);
			}
		}

		if(printedOutYet){
			easyForcePrintLineClient(pEntity, "*********");
		}else{
			easyForcePrintLineClient(pEntity, "*No isolated nodes found.");
		}

	}else if( stringStartsWith(pcmdRefinedRef, "printnode") || stringStartsWith(pcmdRefinedRef, "printoutnode") ){

		
		if(g_flWeaponCheat == 0.0){
			easyForcePrintLineClient(pEntity, "Hm. No.");
			return;
		}

		BOOL printBlockingEnt = FALSE;
		if(stringEndsWith(pcmdRefinedRef, "2")){
			printBlockingEnt = TRUE;
		}

		easyForcePrintLineClient(pEntity, "***NODE CONNECTION MAP***");
		for(int i = 0; i < WorldGraph.m_cNodes; i++){
			easyForcePrintClient(pEntity, "#%d: ", i);


			CNode& thisNode = WorldGraph.m_pNodes[i];

			if(thisNode.m_cNumLinks < 1){
				easyForcePrintClient(pEntity, "none (ISOLATED)");
			}else{
				BOOL firstDest = TRUE;
				for(int i2 = 0; i2 < thisNode.m_cNumLinks; i2++){
					//CNode& pLinkNode = WorldGraph.Node( WorldGraph.NodeLink( i, i2).m_iDestNode );

					
					int thisDest =  WorldGraph.NodeLink( i, i2).m_iDestNode;
					if(!firstDest){
						easyForcePrintClient(pEntity, ", %d", thisDest);
					}else{
						firstDest = FALSE;
						easyForcePrintClient(pEntity, "%d", thisDest);
					}
					
					if(printBlockingEnt){
						entvars_t* pevLinkEnt = WorldGraph.NodeLink( i, i2).m_pLinkEnt;
						easyForcePrintClient(pEntity, " blk: %s", pevLinkEnt!=NULL?STRING(pevLinkEnt->classname):"X" );
					}

				}//END OF for(int i2 = 0...)
			}

			//easyForcePrintClient(pEntity, " m_pLinkEnt: %s", thisNode.m_pLin

			easyForcePrintClient(pEntity, "\n");
		}//END OF for(int i = 0...)
		easyForcePrintLineClient(pEntity, "*************************");



	}else if( FStrEq(pcmdRefinedRef, "forgetenemyandtarget") || FStrEq(pcmdRefinedRef, "forgettargetandenemy")){
		
		if(g_flWeaponCheat == 0.0){
			easyForcePrintLineClient(pEntity, "You may not tamper with artificial intelligence!");
			return;
		}
		
		CBaseEntity* forwardEnt = FindEntityForward(tempplayer);

		if(forwardEnt != NULL){

			CBaseMonster* testMon = forwardEnt->GetMonsterPointer();
			if(testMon != NULL){
				testMon->m_hEnemy = NULL;
				testMon->m_hTargetEnt = NULL;
				easyForcePrintLineClient(pEntity, "Worked cleared enemy & target, on %s:%d", testMon->getClassname(), testMon->monsterID);
			}

		}
	
	}else if( FStrEq(pcmdRefinedRef, "forgettarget")){
		
		if(g_flWeaponCheat == 0.0){
			easyForcePrintLineClient(pEntity, "You may not tamper with artificial intelligence!");
			return;
		}

		CBaseEntity* forwardEnt = FindEntityForward(tempplayer);

		if(forwardEnt != NULL){

			CBaseMonster* testMon = forwardEnt->GetMonsterPointer();
			if(testMon != NULL){
				testMon->m_hTargetEnt = NULL;
				easyForcePrintLineClient(pEntity, "Worked cleared enemy & target, on %s:%d", testMon->getClassname(), testMon->monsterID);
			}

		}
	
	}else if( FStrEq(pcmdRefinedRef, "forgetenemy")){
		
		if(g_flWeaponCheat == 0.0){
			easyForcePrintLineClient(pEntity, "You may not tamper with artificial intelligence!");
			return;
		}

		CBaseEntity* forwardEnt = FindEntityForward(tempplayer);

		if(forwardEnt != NULL){

			CBaseMonster* testMon = forwardEnt->GetMonsterPointer();
			if(testMon != NULL){
				testMon->m_hEnemy = NULL;
				easyForcePrintLineClient(pEntity, "Worked cleared enemy & target, on %s:%d", testMon->getClassname(), testMon->monsterID);
			}

		}
	
	}else if(FStrEq(pcmdRefinedRef, "removefirefly")){

		CBaseEntity *pEntityTemp = NULL;
		
		//TEST!
		while ((pEntityTemp = UTIL_FindEntityInSphere( pEntityTemp, tempplayer->pev->origin, 1024 )) != NULL)
		{
			//CBarnacle* tempBarnacle = (CBarnacle*)pEntityTemp;
			//easyForcePrintLineClient(pEntity, "FOUND ONE? %d ", pEntityTemp->pev->deadflag == DEAD_NO);
			pEntityTemp->pev->effects &= ~(EF_BRIGHTFIELD);
		}
	
	}else if(FStrEq(pcmdRefinedRef, "showboundsall")){
		
		if(g_flWeaponCheat == 0.0){
			easyForcePrintLineClient(pEntity, "Hm. No.");
			return;
		}

		CBaseEntity *pEntityTemp = NULL;

		while ((pEntityTemp = UTIL_FindEntityInSphere( pEntityTemp, tempplayer->pev->origin, 1024 )) != NULL)
		{
			//lol.
			//pEntityTemp->IsTriggered();
			//If this entity is an NPC, draw its bounds.
			if(pEntityTemp->pev->renderfx & ISNPC){
				UTIL_drawBox(pEntityTemp->pev->origin + pEntityTemp->pev->mins, pEntityTemp->pev->origin + pEntityTemp->pev->maxs);
				UTIL_drawBox(pEntityTemp->pev->absmin, pEntityTemp->pev->absmax);
			}
		}//END OF while(things in area)

	}else if(FStrEq(pcmdRefinedRef, "showbounds")){
		
		
		if(g_flWeaponCheat == 0.0){
			easyForcePrintLineClient(pEntity, "Hm. No.");
			return;
		}

		edict_t		*pentIgnore;
		TraceResult tr;
		//isn't this already "pPlayer" that we have provided here now?
		//CBasePlayer* playerRef = GetClassPtr((CBasePlayer *)pev);
		CBasePlayer* playerRef = tempplayer;
		pentIgnore = playerRef ->edict();

		Vector vecOrigSrc = pev->origin;
		Vector vecDest = vecOrigSrc + gpGlobals->v_forward * 2048;


		//nah, precision for while ducking not necessary.
		/*
		Vector playerEyePos = Vector(tempplayer->body;
		if(pev->flags & FL_DUCKING){

		}
		*/

		UTIL_TraceLine ( pev->origin + pev->view_ofs, pev->origin + pev->view_ofs + gpGlobals->v_forward * 128, ignore_monsters, ENT(pev), &tr);
		UTIL_TraceLine( playerRef->GetGunPosition(), vecDest, dont_ignore_monsters, pentIgnore, &tr );
		//tr.vecEndPos();

		/*
		if (tr.fAllSolid){
			ALERT ( at_console, "Could not determine spawn location.\n" );
			return;
		}
		tempplayer->GiveNamedItem( CMD_ARGV(1), tr.vecEndPos.x,
									tr.vecEndPos.y,
									tr.vecEndPos.z + giveLookVertCVAR->value,
									TRUE);
		*/

		CBaseEntity *hitEntity = CBaseEntity::Instance(tr.pHit);
		if(hitEntity && hitEntity->pev->renderfx & ISNPC){
			//MODDD - FOR DEBUGGING !!!
			UTIL_drawBox(hitEntity->pev->origin + hitEntity->pev->mins, hitEntity->pev->origin + hitEntity->pev->maxs);
			UTIL_printLineVector("mins", hitEntity->pev->mins);
			UTIL_printLineVector("maxs", hitEntity->pev->maxs);
			UTIL_printLineVector("absmin", hitEntity->pev->absmin);
			UTIL_printLineVector("absmax", hitEntity->pev->absmax);

			//UTIL_drawBox(hitEntity->pev->absmin, hitEntity->pev->absmax);
			
			/*
			UTIL_drawBox(hitEntity->pev->origin, hitEntity->pev->origin + hitEntity->pev->maxs);
			UTIL_drawBox(hitEntity->pev->origin, hitEntity->pev->origin + hitEntity->pev->mins);
			*/
			//UTIL_drawLine( hitEntity->pev->origin, hitEntity->pev->origin + Vector(30, 30, 30);
			/*
			if(hitEntity->pev->renderfx & ISNPC){
				CBaseMonster* attemptMonster = static_cast<CBaseMonster*>(hitEntity);
				
				//easyForcePrintLineClient(pEntity, "YES YOU FAY!!! %s %d", STRING(hitEntity->pev->classname), attemptMonster->getHasPathFindingMod() );
				attemptMonster->canDrawDebugSurface = TRUE;
			}
			*/
		}else{
			if(!hitEntity){
				easyForcePrintLineClient(pEntity, "~NPC not found.  Target NPC with crosshair and try again.");
			}else{
				easyForcePrintLineClient(pEntity, "~invalid thing targeted.  name: %s", STRING(hitEntity->pev->classname) );
			}
		}

	}else if( ::stringStartsWith(pcmdRefinedRef, "hitbox") || ::stringStartsWith(pcmdRefinedRef, "showhitbox") ){
		if(g_flWeaponCheat == 0.0){
			easyForcePrintLineClient(pEntity, "Hm. No.");
			return;
		}

		edict_t		*pentIgnore;
		TraceResult tr;
		CBasePlayer* playerRef = tempplayer;
		pentIgnore = playerRef ->edict();

		Vector vecOrigSrc = pev->origin;
		Vector vecDest = vecOrigSrc + gpGlobals->v_forward * 2048;

		UTIL_TraceLine ( pev->origin + pev->view_ofs, pev->origin + pev->view_ofs + gpGlobals->v_forward * 128, ignore_monsters, ENT(pev), & tr);
		UTIL_TraceLine( playerRef->GetGunPosition(), vecDest, dont_ignore_monsters, pentIgnore, &tr );
		
		CBaseEntity *hitEntity = CBaseEntity::Instance(tr.pHit);
		CBaseMonster* tempMonsterPointer = NULL;
		if(hitEntity && ((tempMonsterPointer = hitEntity->GetMonsterPointer()) != NULL) && hitEntity->pev->renderfx & ISNPC){
			//MODDD - FOR DEBUGGING !!!

			easyForcePrintLineClient(pEntity, "FOUND!");

			//check. Was a number supplied?
			const char* arg1ref = CMD_ARGV(1);
			BOOL argSupplied = FALSE;
			int numbAttempt;
			if(!isStringEmpty(arg1ref)){
				argSupplied=TRUE;
				numbAttempt = tryStringToInt(arg1ref);
			}


			if(::stringEndsWith(pcmdRefinedRef, "bone")){
				if(argSupplied){
					tempMonsterPointer->showHitboxInfoOfBone(numbAttempt);
				}else{
					easyForcePrintLineClient(pEntity, "ERROR: bone number required.");
				}
			}else if(::stringEndsWith(pcmdRefinedRef, "group")){
				if(argSupplied){
					tempMonsterPointer->showHitboxInfoOfGroup(numbAttempt);
				}else{
					easyForcePrintLineClient(pEntity, "ERROR: group number required.");
				}
			}else if(argSupplied){
				//assume it is for ID (the "nth" bone, like first, 2nd, 3rd, etc.)
				tempMonsterPointer->showHitboxInfoNumber(numbAttempt);
			}else{
				//no arg supplied? proceed.
				tempMonsterPointer->showHitboxInfoAll();
			}

		}else{
			if(!hitEntity){
				easyForcePrintLineClient(pEntity, "~NPC not found.  Target NPC with crosshair and try again.");
			}else{
				easyForcePrintLineClient(pEntity, "~invalid thing targeted.  name: %s", STRING(hitEntity->pev->classname) );
			}
		}

	}else if(FStrEq(pcmdRefinedRef, "god2")){
		if(g_flWeaponCheat != 0.0){
			tempplayer->reviveIfDead();
			if(tempplayer->pev->flags & FL_GODMODE){
				tempplayer->pev->flags &= ~FL_GODMODE;
				easyForcePrintLineClient(pEntity, "godmode OFF");
			}else{
				tempplayer->pev->flags |= FL_GODMODE;
				easyForcePrintLineClient(pEntity, "godmode ON");
			}
		}else{
			easyForcePrintLineClient(pEntity, "No \'God\' you cheater!");
		}

	}else if(FStrEq(pcmdRefinedRef, "noclip2")){
		if(g_flWeaponCheat != 0.0){
			if(tempplayer->pev->deadflag == DEAD_NO){
				if(tempplayer->pev->movetype == MOVETYPE_NOCLIP){
					tempplayer->pev->movetype = MOVETYPE_BOUNCE;
					easyForcePrintLineClient(pEntity, "noclip OFF");
				}else{
					tempplayer->pev->movetype = MOVETYPE_NOCLIP;
					easyForcePrintLineClient(pEntity, "noclip ON");
				}
			}else{
				easyForcePrintLineClient(pEntity, "\'Noclip\' is not for the dead, try \'revive\' first.");
			}
		}else{
			easyForcePrintLineClient(pEntity, "No \'Noclip\' you cheater!");
		}

	}else if(FStrEq(pcmdRefinedRef, "cameraper_f")){
		globalPSEUDO_cameraMode = 0;

	}else if(FStrEq(pcmdRefinedRef, "cameraper_t")){
		globalPSEUDO_cameraMode = 1;
	}else if( FStrEq(pcmdRefinedRef, "crazyprintoutoff") ){
		if(g_flWeaponCheat == 0.0){
			easyForcePrintLineClient(pEntity, "Hm. No.");
			return;
		}

		CBaseEntity *pSearchEntity = NULL;
		while ((pSearchEntity = UTIL_FindEntityInSphere(pSearchEntity, tempplayer->pev->origin, 5000 )) != NULL)
		{
			//CBaseMonster *pMon = pSearchEntity->MyMonsterPointer( );
			//if(pMon != NULL){
			//if(pSearchEntity != NULL){
			pSearchEntity->crazyPrintout = FALSE;
			//}
		}
	
	}else if(  FStrEq(pcmdRefinedRef, "crazyprintout") || FStrEq(pcmdRefinedRef, "crazyprintouton") ){
		
		if(g_flWeaponCheat == 0.0){
			easyForcePrintLineClient(pEntity, "Hm. No.");
			return;
		}

		const char* arg1ref = CMD_ARGV(1);

		CBaseEntity* forwardEnt = FindEntityForward(tempplayer);

		if(forwardEnt != NULL){
			CBaseMonster* testMon = forwardEnt->GetMonsterPointer();
			if(testMon == NULL){
				easyForcePrintLineClient(pEntity, "ERROR: thing in crosshairs is not a monster.  It is \"%s\".", STRING(forwardEnt->pev->classname) );
			}else{
				//got it!
				testMon->crazyPrintout = !testMon->crazyPrintout;
				
				easyForcePrintLineClient(pEntity, "WORKED!  crazyPrintout is %s", testMon->crazyPrintout == 1?"ON":"OFF");

			}
		}else{
			easyForcePrintLineClient(pEntity, "ERROR: Could not find an entity / monster in crosshairs.");
		}

	}else if(FStrEq(pcmdRefinedRef, "forcefailtask") || FStrEq(pcmdRefinedRef, "forcetaskfail") || FStrEq(pcmdRefinedRef, "failtask") || FStrEq(pcmdRefinedRef, "taskfail") ){
		
		if(g_flWeaponCheat == 0.0){
			easyForcePrintLineClient(pEntity, "You may not tamper with artificial intelligence!");
			return;
		}

		const char* arg1ref = CMD_ARGV(1);

		CBaseEntity* forwardEnt = FindEntityForward(tempplayer);

		if(forwardEnt != NULL){
			CBaseMonster* testMon = forwardEnt->GetMonsterPointer();
			if(testMon == NULL){
				easyForcePrintLineClient(pEntity, "ERROR: thing in crosshairs is not a monster.  It is \"%s\".", STRING(forwardEnt->pev->classname) );
			}else{
				//got it!
				testMon->TaskFail();
				easyForcePrintLineClient(pEntity, "That monster, %s:%d, just failed...", testMon->getClassname(), testMon->monsterID);

			}
		}else{
			easyForcePrintLineClient(pEntity, "ERROR: Could not find an entity / monster in crosshairs.");
		}


	}else if(FStrEq(pcmdRefinedRef, "forcetaskcomplete") || FStrEq(pcmdRefinedRef, "forcecompletetask") || FStrEq(pcmdRefinedRef, "taskcomplete") || FStrEq(pcmdRefinedRef, "completetask") ){
		if(g_flWeaponCheat == 0.0){
			easyForcePrintLineClient(pEntity, "You may not tamper with artificial intelligence!");
			return;
		}
		
		const char* arg1ref = CMD_ARGV(1);

		CBaseEntity* forwardEnt = FindEntityForward(tempplayer);

		if(forwardEnt != NULL){
			CBaseMonster* testMon = forwardEnt->GetMonsterPointer();
			if(testMon == NULL){
				easyForcePrintLineClient(pEntity, "ERROR: thing in crosshairs is not a monster.  It is \"%s\".", STRING(forwardEnt->pev->classname) );
			}else{
				//got it!
				testMon->TaskComplete();
				easyForcePrintLineClient(pEntity, "That monster, %s:%d, just completed the current task.", testMon->getClassname(), testMon->monsterID);

			}
		}else{
			easyForcePrintLineClient(pEntity, "ERROR: Could not find an entity / monster in crosshairs.");
		}


	}else if(  FStrEq(pcmdRefinedRef, "checklocalmove") || FStrEq(pcmdRefinedRef, "testchecklocalmove") ){
		
		if(g_flWeaponCheat == 0.0){
			easyForcePrintLineClient(pEntity, "Hm. No.");
			return;
		}

		const char* arg1ref = CMD_ARGV(1);

		if(arg1ref == NULL || isStringEmpty(arg1ref)){
			easyForcePrintLineClient(pEntity, "No arg!  Need a distance to go for the check...");
			return;
		}

		float dist = (float)atof(arg1ref);
		//destination for storing something?
		float distReg = 0;

		UTIL_MakeVectors(tempplayer->pev->v_angle + tempplayer->pev->punchangle);
		
		Vector vecStart = tempplayer->pev->origin + Vector(0, 0, 10);
		Vector vecEnd = vecStart + gpGlobals->v_forward * dist;
		BOOL success = (tempplayer->CheckLocalMove(vecStart, vecEnd, NULL, TRUE, &distReg) == LOCALMOVE_VALID);
		//UTIL_TraceLine(tempplayer->pev->origin + tempplayer->pev->view_ofs + gpGlobals->v_forward * 5,pMe->pev->origin + pMe->pev->view_ofs + gpGlobals->v_forward * 2048,dont_ignore_monsters, pMe->edict(), &tr );

		::DebugLine_ClearAll();

		float fullLength = dist;

		if(success){
			easyForcePrintLineClient(pEntity, "SUCCESS!  CLEAR!");
			::DebugLine_Setup(0, vecStart, vecEnd, (1.0f));
		}else{
			easyForcePrintLineClient(pEntity, "Stopped this far: %.2f", distReg);
			::DebugLine_Setup(0, vecStart, vecEnd, (distReg / fullLength));
		}

	}else if( FStrEq(pcmdRefinedRef, "testnoderouteformonster") ){
		// Take the ID of the monster to test on, and the nodes in the route (each should have a straight-line path between then).
		// Returns whether the route is deemed valid for this monster to take, or, if not, why it failed.

		if(CMD_ARGC() <= 1){
			// only itself?  Print help instead
			easyForcePrintLine("***No parameters given, printing out help info***");
			easyForcePrintLine("Method takes the monster ID to use followed by a list of nodes in the route to test.  Must be at least one.");
		}else if(CMD_ARGC() == 2){
			easyForcePrintLine("***Not enough parameters, needs at least one more for the route to test.***");
		}else if((CMD_ARGC() > 34)){
			easyForcePrintLine("***Too many parameters, at most 32 nodes may be described.***");
		}else{
			// Go.
			float routeLength = 0;
			float linkLength = 0;
			int i;
			int theMonsterID;
			int nodeParameterCount = CMD_ARGC() - 2;
			int nodeList[32];
			BOOL parseSuccess;
			BOOL movePass;
			BOOL routeSuccess = TRUE;  // assume it is until proven otherwise
			// (the method ending early by 'return' never reaches a success message so that is the same as failing)

			parseSuccess = attemptParseStringToInt(pEntity, &theMonsterID, CMD_ARGV(1), "ERROR!  Could not read MonsterID.", "???");
			if(!parseSuccess)return;

			for(i = 0; i < nodeParameterCount; i++){
				int thisNodeNumber;
				// UTIL_VarArgs?  nah, not worth generating an error message that won't be seen.  
				// sprintf?  Hm.
				parseSuccess = attemptParseStringToInt(pEntity, &thisNodeNumber, CMD_ARGV(i + 2), "ERROR!  Could not read a node, bad characters?", "???");
				if(!parseSuccess){
					easyForcePrintLine("***Failed to read \"%s\".***", CMD_ARGV(i + 2));
					return;
				}

				// little validation. That's a valid node, right?
				if(thisNodeNumber >= 0 && thisNodeNumber < WorldGraph.m_cNodes){
					// proceed
					nodeList[i] = thisNodeNumber;
				}else{
					easyForcePrintLine("ERROR! Node of invalid index, negative or exceeding this map's node count not allowed.  Node given: %d   Map max: %d", thisNodeNumber, WorldGraph.m_cNodes);
					return;
				}

			}//for loop through parameters
			
			CBaseMonster* theMon = getMonsterWithID(theMonsterID);
			if(theMon == NULL){
				easyForcePrintLine("ERROR!  Monster of ID '%d' not found.", theMonsterID);
				return;
			}

			easyForcePrintLine("***Route read successfully, testing***");
			CNode* theNode = &WorldGraph.Node(nodeList[0]);

			// Now with the nodes, check to see that the monster can reach the first node.
			// And don't add pev->view_ofs to anything!  Ground-to-ground will work fine, snaps the start at startup anyway and
			// the point it ends up at (simulated movement against the ground to point B) is compared to the end-point snapped to
			// the ground, if 'doCheckZ' is on.
			// WAIT!  end point isn't snapped to the ground, and that can be a good thing.  If the dest point is to the enemy origin (player)
			// and the player is standing over the edge of something like a box, do we really want to be fooled into thinking hitting the
			// point under the player is ok just because the player's origin (centered floor-wise, at least) could trace lower to the ground from
			// that height?
			movePass = (theMon->CheckLocalMove(theMon->pev->origin, theNode->m_vecOrigin, NULL, TRUE, &linkLength ) == LOCALMOVE_VALID);
			routeLength += linkLength;

			// is that fair
			float linkLengthPotential = Distance(theMon->pev->origin, theNode->m_vecOrigin);

			if(!movePass){
				easyForcePrintLine("Route failed!  Monster could not reach first node from its current point."); // Link length reached:%.2f  Potential:%.2f", linkLength, linkLengthPotential);
				return;
			}

			// Now, test between every two nodes in the route from start to end
			// (if only one node was given, this was only a test from where the monster is to that one node)
			for(i = 0; i < nodeParameterCount-1; i++){
				CNode* firstNode = &WorldGraph.Node(nodeList[i]);
				CNode* secondNode = &WorldGraph.Node(nodeList[i+1]);
				
				movePass = (theMon->CheckLocalMove(firstNode->m_vecOrigin, secondNode->m_vecOrigin, NULL, TRUE, &linkLength ) == LOCALMOVE_VALID);
				routeLength += linkLength;

				if(!movePass){
					easyForcePrintLine("Route failed!  Monster could not from node #%d (%d) to #%d (%d).");//  Link length reached:%.2f  Potential:%.2f", i, nodeList[i], i+1, nodeList[i+1], linkLength, linkLengthPotential);
					routeSuccess = FALSE;
					break;
				}

			}

			if(routeSuccess){
				easyForcePrintLine("***Route success!  All nodes reached.  Route length: %.2f***", routeLength);
			}else{
				// oh
				easyForcePrintLine("***Incomplete route length: %.2f***", routeLength);
			}

		}//arg count check
	}//end of command
	else if( FStrEq(pcmdRefinedRef, "testnoderoute2formonster") ){

		if(CMD_ARGC() <= 1){
			// only itself?  Print help instead
			easyForcePrintLine("***No parameters given, printing out help info***");
			easyForcePrintLine("Method takes the monster ID to use followed by two parameters: start and end node.  Route is determined by WorldGraph.FindShortestPath.");

		}else if(CMD_ARGC() < 4){
			easyForcePrintLine("***Not enough parameters, need the ID of the monster to use, start node, and end node***");
		}else if(CMD_ARGC() > 4){
			easyForcePrintLine("***Too many parameters, only start/end nodes allowed, none inbetween.***");
		}else{
			int i;
			int theMonsterID;
			int nodeParameterCount = 2;
			int nodeList[2];
			BOOL parseSuccess;
			BOOL movePass;
			BOOL routeSuccess = TRUE;  // assume it is until proven otherwise
			// (the method ending early by 'return' never reaches a success message so that is the same as failing)

			parseSuccess = attemptParseStringToInt(pEntity, &theMonsterID, CMD_ARGV(1), "ERROR!  Could not read MonsterID.", "???");
			if(!parseSuccess)return;

			for(i = 0; i < nodeParameterCount; i++){
				int thisNodeNumber;
				parseSuccess = attemptParseStringToInt(pEntity, &thisNodeNumber, CMD_ARGV(i + 2), "ERROR!  Could not read a node, bad characters?", "???");
				if(!parseSuccess){
					easyForcePrintLine("***Failed to read \"%s\".***", CMD_ARGV(i + 2));
					return;
				}

				if(thisNodeNumber >= 0 && thisNodeNumber < WorldGraph.m_cNodes){
					nodeList[i] = thisNodeNumber;
				}else{
					easyForcePrintLine("ERROR! Node of invalid index, negative or exceeding this map's node count not allowed.  Node given: %d   Map max: %d", thisNodeNumber, WorldGraph.m_cNodes);
					return;
				}
			}//for loop through parameters


			CBaseMonster* theMon = getMonsterWithID(theMonsterID);
			if(theMon == NULL){
				easyForcePrintLine("ERROR!  Monster of ID '%d' not found.", theMonsterID);
				return;
			}
			easyForcePrintLine("***Route read successfully, testing***");


			int iNodeHull;

			if (EASY_CVAR_GET_DEBUGONLY(pathfindForcePointHull) != 1) {
				// normal way.  Get the Hull from this monster trying to pathfind.
				// Can be used to tell if some paths between nodes are invalid from this monster's size.
				iNodeHull = WorldGraph.HullIndex( theMon ); // make this a monster virtual function
			}else{
				// force 0 size.
				iNodeHull = NODE_POINT_HULL;
			}


			int iPath[ MAX_PATH_SIZE ];

			g_routeTempMonster = theMon;
			// Assume the movegoal is set if it will be relevant?
			g_routeTempMonster_GoalEnt = theMon->GetGoalEntity();

			int iResult = WorldGraph.FindShortestPath ( iPath, nodeList[0], nodeList[1], iNodeHull, theMon->m_afCapability );
			g_routeTempMonster = NULL;

			if(iResult){
				easyForcePrintLine("***Route success!  Route length: %d nodes***", iResult);

				for(i = 0; i < iResult; i++){
					easyForcePrintLine("#%d: %d", i, iPath[i]);
				}

			}else{
				// oh
				easyForcePrintLine("***Could not form route.***");
			}

		}//arg count check
	}//end of command


	/*
	else if( FStrEq(pcmdRefinedRef, "debugcine1")){
		CBaseEntity *pEntity = NULL;
		int searchRadius = 1024;

		while ((pEntity = UTIL_FindEntityInSphere( pEntity, pev->origin, searchRadius )) != NULL)
		{

			if(FClassnameIs(pEntity->pev, "scripted_sequence") ){
				CBaseMonster* monTest = NULL;
				if( (monTest = pEntity->GetMonsterPointer()) != NULL){
					//easyForcePrintLineClient(pEntity, "HEY I EXIST::::%d", monTest->monsterID );
					monTest->ReportAIState();
				}
			}

		}
		return;

		tempplayer->pev->flags &= ~FL_GODMODE;
		tempplayer->TakeDamage(VARS(eoNullEntity), VARS(eoNullEntity), 99999, DMG_ALWAYSGIB, 0);
		return;


		while ((pEntity = UTIL_FindEntityInSphere( pEntity, pev->origin, searchRadius )) != NULL)
		{
			CSquadMonster *pRecruit = pEntity->MySquadMonsterPointer( );



			BOOL canPrint = FALSE;

			int iMyClass = 0;

			if(pRecruit){
				iMyClass = pRecruit->Classify();
			}

			if(pRecruit && pRecruit->IsAlive() && !pRecruit->m_pCine && pRecruit->Classify() == CLASS_HUMAN_MILITARY &&
				   FStringNull( pRecruit->pev->netname ) ){

				canPrint = TRUE;
				easyForcePrintLineClient(pEntity, "debugger   i: %d class: %s insq: %d count: %d leadernotnull: %d ", "-1", STRING(pRecruit->pev->classname), pRecruit->MySquadMonsterPointer()->InSquad(), pRecruit->SquadCount(), (pRecruit->MySquadLeader() != NULL) );

			}

		}
	}
	else if( FStrEq(pcmdRefinedRef, "debugcine2")){
		const char* arg1ref = CMD_ARGV(1);

		CBaseEntity* forwardEnt = FindEntityForward(tempplayer);

		if(forwardEnt != NULL){
			CBaseMonster* testMon = forwardEnt->GetMonsterPointer();
			if(testMon == NULL){
				easyForcePrintLineClient(pEntity, "ERROR: thing in crosshairs is not a monster.  It is \"%s\".", STRING(forwardEnt->pev->classname) );
			}else{
				//got it!
				if(testMon->m_pCine!=NULL){
					tempplayer->pev->origin = testMon->m_pCine->pev->origin;
				}
			}
		}else{
			easyForcePrintLineClient(pEntity, "ERROR: Could not find an entity / monster in crosshairs.");
		}
	
	}else if( FStrEq(pcmdRefinedRef, "debugcine3")){
		const char* arg1ref = CMD_ARGV(1);

		edict_t		*pEdicttt;
		CBaseEntity *pEntityyy;


		pEdicttt = g_engfuncs.pfnPEntityOfEntIndex( 1 );
		pEntityyy;
		if ( pEdicttt ){
			for ( int i = 1; i < gpGlobals->maxEntities; i++, pEdicttt++ ){
				if ( pEdicttt->free )	// Not in use
				continue;
		
				pEntityyy = CBaseEntity::Instance(pEdicttt);
				if ( !pEntityyy )
					continue;

				CBaseMonster* tempmon = NULL;
				if(  (tempmon = pEntityyy->GetMonsterPointer()) != NULL && tempmon->m_pCine != NULL){
					UTIL_SetOrigin(tempmon->m_pCine->pev, tempplayer->pev->origin + Vector(0, 0, 6));
				}

			}//END OF loop
		}
	}
	*/
	
	else if(  FStrEq(pcmdRefinedRef, "tracehull") || FStrEq(pcmdRefinedRef, "hulltrace") ){
		// Takes 1 parameter, give it a hull choice to use.  Implies 1 if none is given.

		//typedef enum { point_hull=0, human_hull=1, large_hull=2, head_hull=3 };
		//HULL TRACE STYLE.
		//0: point_hull: (0, 0, 0), (0, 0, 0) ?
		//human_hull: VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX  ...that is, (-16, -16, 0), (16, 16, 72)
		//large_hull: ( -32, -32, 0 ), ( 32, 32, 64 ) );
		//head_hull:  (-16, -16, 0), (16, 16, 24)   (height unverified)
		
		TraceResult tr;

		//Vector itsAngles = (Vector(-pev->v_angle.x, pev->v_angle.y, pev->v_angle.z) + pev->punchangle);
		//UTIL_MakeVectors(itsAngles);

		UTIL_MakeVectors(tempplayer->pev->v_angle + tempplayer->pev->punchangle);

		const Vector tempForward = gpGlobals->v_forward;
		Vector vecStart = tempplayer->pev->origin + tempplayer->pev->view_ofs + tempForward * -10;
		Vector vecEnd = tempplayer->pev->origin + tempplayer->pev->view_ofs + tempForward * 200;

		const char* arg1ref = CMD_ARGV(1);
		
		if(arg1ref == NULL || isStringEmpty(arg1ref)){
			easyForcePrintLineClient(pEntity, "***Can specify a hull-type, implying 1.  Choices are integers 0 through 3.");
			//return;
			arg1ref = "1";
		}
		int argHullType = atoi(arg1ref);

		debugPoint1 = vecStart;
		debugPoint1Given = TRUE;
		debugPoint2 = vecEnd;
		debugPoint2Given = TRUE;

		//UTIL_SetSize(tempplayer->pev, Vector(-32, -32, 0), Vector(32, 32, 64) );
		
		// could use ENT(tempplayer->pev) instead of templayer->edict(), but... why?

		UTIL_TraceHull( vecStart, vecEnd, dont_ignore_monsters, argHullType, tempplayer->edict(), &tr);
		//UTIL_MakeVectors(tempplayer->pev->v_angle + tempplayer->pev->punchangle);

		easyForcePrintStarter();
		//easyForcePrint("Trace info: ");
		printBasicTraceInfo(pEntity, tr);
		easyForcePrintLine();

		//debugPoint3 = pev->origin + pev->view_ofs + gpGlobals->v_forward * (5) + (gpGlobals->v_forward * (200 - 5) * (1  - tr.flFraction) );
		debugPoint3Given = TRUE;
		debugPoint3 = vecStart * (1 + -tr.flFraction) + vecEnd * tr.flFraction;
		//debugPoint3 = pev->origin + pev->view_ofs + tempForward * 180;

	}else if( FStrEq(pcmdRefinedRef, "tracemonsterhull") || FStrEq(pcmdRefinedRef, "monsterhulltrace") ){
		// Takes no parameters, implies hull choice from the monster given.
		// I have no idea if it uses the exact size of the monste, or just approximates it to one of the
		// hull choices (0, 1, 2, 3) from tracehull automatically from that.  If so this isn't really any
		// more precise or helpful.
		// Might borrow hull choice from some part of 'pev' vars too, if that is ever set somewhere in there.
		
		if (CMD_ARGC() > 1) {
			easyForcePrintLineClient(pEntity, "***tracemonsterhull does not take a hull-type, any parmeter(s) ignored.");
		}
		
		TraceResult tr;
		
		UTIL_MakeVectors(tempplayer->pev->v_angle + tempplayer->pev->punchangle);

		const Vector tempForward = gpGlobals->v_forward;
		Vector vecStart = tempplayer->pev->origin + tempplayer->pev->view_ofs + tempForward * -10;
		Vector vecEnd = tempplayer->pev->origin + tempplayer->pev->view_ofs + tempForward * 200;

		debugPoint1 = vecStart;
		debugPoint1Given = TRUE;
		debugPoint2 = vecEnd;
		debugPoint2Given = TRUE;


		TRACE_MONSTER_HULL(tempplayer->edict(), vecStart, vecEnd, dont_ignore_monsters, tempplayer->edict(), &tr);

		easyForcePrintStarter();
		//easyForcePrint("Trace info: ");
		printBasicTraceInfo(pEntity, tr);
		easyForcePrintLine();

		//debugPoint3 = pev->origin + pev->view_ofs + gpGlobals->v_forward * (5) + (gpGlobals->v_forward * (200 - 5) * (1  - tr.flFraction) );
		debugPoint3Given = TRUE;
		debugPoint3 = vecStart * (1 + -tr.flFraction) + vecEnd * tr.flFraction;
		//debugPoint3 = pev->origin + pev->view_ofs + tempForward * 180;

	}else if( FStrEq(pcmdRefinedRef, "getorigin") ){
		const char* arg1ref = CMD_ARGV(1);
		if(g_flWeaponCheat == 0.0){
			easyForcePrintLineClient(pEntity, "No origin trickery for you, cheater!");
			return;
		}

		//ambiguous as to whether this is what is in the crosshairs or the player itself.  Try to figure it out:
		CBaseEntity* forwardEnt = FindEntityForward(tempplayer);

		if(forwardEnt == NULL){
			//No.
			easyForcePrintLineClient(pEntity, "NO entity found.");
			return;
		}else{
			easyForcePrintLineClient(pEntity, "Entity found:%s. Origin:(%.2f,%.2f,%.2f)", forwardEnt->getClassname(),
				forwardEnt->pev->origin.x,
				forwardEnt->pev->origin.y,
				forwardEnt->pev->origin.z
			);
		}

	}else if( FStrEq(pcmdRefinedRef, "setorigin") ){
		
		if(g_flWeaponCheat == 0.0){
			easyForcePrintLineClient(pEntity, "No origin trickery for you, cheater!");
			return;
		}
		//ambiguous as to whether this is what is in the crosshairs or the player itself.  Try to figure it out:
		if(CMD_ARGC() < 4){
			easyForcePrintLineClient(pEntity, "Need 3 parameters.");
			return;
		}else if(CMD_ARGC() > 4){
			easyForcePrintLineClient(pEntity, "WARNING: Parameters after the 3rd ignored.");
			return;
		}
		const char* arg1ref = CMD_ARGV(1);
		const char* arg2ref = CMD_ARGV(2);
		const char* arg3ref = CMD_ARGV(3);

		CBaseEntity* forwardEnt = FindEntityForward(tempplayer);

		if(forwardEnt == NULL){
			//no.
			easyForcePrintLineClient(pEntity, "NO entity found.");
			return;
		}else{
			
			try{
				float xVal = tryStringToFloat(arg1ref);
				float yVal = tryStringToFloat(arg2ref);
				float zVal = tryStringToFloat(arg3ref);

				//forwardEnt->pev->origin = Vector(xVal, yVal, zVal);
				UTIL_SetOrigin (forwardEnt->pev, Vector(xVal, yVal, zVal) );// take him off ground so engine doesn't instantly reset onground 
				//forwardEnt->pev->flags |= FL_ONGROUND
				forwardEnt->pev->flags &= ~FL_ONGROUND;

				//easyForcePrintLineClient(pEntity, "MODEL: %s", STRING(forwardEnt->pev->model));

			}catch(int){
				easyForcePrintLineClient(pEntity, "ERROR - could not parse inputs. No commas or non numeric characters allowed.");
			}

		}//END OF forwardEnt check

	}else if( FStrEq(pcmdRefinedRef, "getangles") ){
		const char* arg1ref = CMD_ARGV(1);
		if(g_flWeaponCheat == 0.0){
			easyForcePrintLineClient(pEntity, "No angles trickery for you, cheater!");
			return;
		}

		//ambiguous as to whether this is what is in the crosshairs or the player itself.  Try to figure it out:
		CBaseEntity* forwardEnt = FindEntityForward(tempplayer);

		if(forwardEnt == NULL){
			//No.
			easyForcePrintLineClient(pEntity, "NO entity found.");
			return;
		}else{
			easyForcePrintLineClient(pEntity, "Entity found:%s. Angles:(%.2f,%.2f,%.2f)", forwardEnt->getClassname(),
				forwardEnt->pev->angles.x,
				forwardEnt->pev->angles.y,
				forwardEnt->pev->angles.z
			);
		}

	}else if( FStrEq(pcmdRefinedRef, "setangles") ){
		
		if(g_flWeaponCheat == 0.0){
			easyForcePrintLineClient(pEntity, "No angles trickery for you, cheater!");
			return;
		}
		// ambiguous as to whether this is what is in the crosshairs or the player itself.  Try to figure it out:
		if(CMD_ARGC() < 4){
			easyForcePrintLineClient(pEntity, "Need 3 parameters.");
			return;
		}else if(CMD_ARGC() > 4){
			easyForcePrintLineClient(pEntity, "WARNING: Parameters after the 3rd ignored.");
			return;
		}
		const char* arg1ref = CMD_ARGV(1);
		const char* arg2ref = CMD_ARGV(2);
		const char* arg3ref = CMD_ARGV(3);


		CBaseEntity* forwardEnt = FindEntityForward(tempplayer);

		if(forwardEnt == NULL){
			//no.
			easyForcePrintLineClient(pEntity, "NO entity found.");
			return;
		}else{
			
			try{
				float xVal = tryStringToFloat(arg1ref);
				float yVal = tryStringToFloat(arg2ref);
				float zVal = tryStringToFloat(arg3ref);

				pev->angles.x = xVal;
				pev->angles.y = yVal;
				pev->angles.z = zVal;

			}catch(int){
				easyForcePrintLineClient(pEntity, "ERROR - could not parse inputs. No commas or non numeric characters allowed.");
			}

		}//END OF forwardEnt check

	}else if( FStrEq(pcmdRefinedRef, "setmyorigin") ){
		const char* arg1ref = CMD_ARGV(1);
		const char* arg2ref = CMD_ARGV(2);
		const char* arg3ref = CMD_ARGV(3);

		if(g_flWeaponCheat == 0.0){
			easyForcePrintLineClient(pEntity, "No origin trickery for you, cheater!");
			return;
		}
		//ambiguous as to whether this is what is in the crosshairs or the player itself.  Try to figure it out:
		
		if(
			arg1ref == NULL || arg2ref == NULL || arg3ref == NULL ||
			isStringEmpty(arg1ref) || isStringEmpty(arg2ref) || isStringEmpty(arg3ref)
		){
			easyForcePrintLineClient(pEntity, "Need 3 parameters.");
			return;
		}
		
		try{
			float xVal = tryStringToFloat(arg1ref);
			float yVal = tryStringToFloat(arg2ref);
			float zVal = tryStringToFloat(arg3ref);

			//forwardEnt->pev->origin = Vector(xVal, yVal, zVal);
			UTIL_SetOrigin (tempplayer->pev, Vector(xVal, yVal, zVal) );// take him off ground so engine doesn't instantly reset onground 
			//forwardEnt->pev->flags |= FL_ONGROUND
			tempplayer->pev->flags &= ~FL_ONGROUND;

			//easyForcePrintLineClient(pEntity, "MODEL: %s", STRING(forwardEnt->pev->model));
		
		}catch(int){
			easyForcePrintLineClient(pEntity, "ERROR - could not parse inputs. No commas or non numeric characters allowed.");
		}

	
	}else if( FStrEq(pcmdRefinedRef, "drawpath") || FStrEq(pcmdRefinedRef, "pathdraw") || FStrEq(pcmdRefinedRef, "drawnpcpath")  ){
		const char* arg1ref = CMD_ARGV(1);
		if(g_flWeaponCheat == 0.0){
			easyForcePrintLineClient(pEntity, "Hey, that\'s a debugging feature mister!");
			return;
		}
		CBaseEntity* forwardEnt = NULL;
		const char* argTest = CMD_ARGV(1);
		if(!isStringEmpty(argTest)){
			//get the monster with this ID.
			try{
				int numbAttempt = tryStringToInt(arg1ref);
				forwardEnt = getMonsterWithID(numbAttempt);
			}catch(int){
				easyForcePrintLineClient(pEntity, "Problem reading number.  (arg must be whole number)");
				return;
			}
		}else{
			//no argument provided? Try to see if there is a monster in front of the player.
			forwardEnt = FindEntityForward(tempplayer);
		}
		if(forwardEnt == NULL){
			//can't do this.
			easyForcePrintLineClient(pEntity, "***No entity found.***");
		}else{
			CBaseMonster* tempMon;
			if(  (tempMon = forwardEnt->GetMonsterPointer() ) != NULL){
				if(tempMon->drawPathConstant){
					easyForcePrintLineClient(pEntity, "***Stopped drawing path for %s ID:%d.***", tempMon->getClassname(), tempMon->monsterID);
					tempMon->drawPathConstant = FALSE;
				}else{
					easyForcePrintLineClient(pEntity, "***Drawing path for %s ID:%d.***", tempMon->getClassname(), tempMon->monsterID);
					tempMon->drawPathConstant = TRUE;
				}
			}else{
				easyForcePrintLineClient(pEntity, "***Entity \"%s\" is not a monster / NPC.***", forwardEnt->getClassname() );
			}
		}
	}else if( FStrEq(pcmdRefinedRef, "dummy") || FStrEq(pcmdRefinedRef, "dummyai") || FStrEq(pcmdRefinedRef, "nothinkforyou")  ){
		// Does not dummy all AI, only for the directed monster
		const char* arg1ref = CMD_ARGV(1);
		if(g_flWeaponCheat == 0.0){
			easyForcePrintLineClient(pEntity, "Hey, that\'s a debugging feature mister!");
			return;
		}
		CBaseEntity* forwardEnt = NULL;
		const char* argTest = CMD_ARGV(1);
		if(!isStringEmpty(argTest)){
			//get the monster with this ID.
			try{
				int numbAttempt = tryStringToInt(arg1ref);
				forwardEnt = getMonsterWithID(numbAttempt);
			}catch(int){
				easyForcePrintLineClient(pEntity, "Problem reading number.  (arg must be whole number)");
				return;
			}
		}else{
			//no argument provided? Try to see if there is a monster in front of the player.
			forwardEnt = FindEntityForward(tempplayer);
		}
		if(forwardEnt == NULL){
			//can't do this.
			easyForcePrintLineClient(pEntity, "***No entity found.***");
		}else{
			CBaseMonster* tempMon;
			if(  (tempMon = forwardEnt->GetMonsterPointer() ) != NULL){
				if(tempMon->dummyAI){
					easyForcePrintLineClient(pEntity, "***Resuming AI for %s ID:%d.***", tempMon->getClassname(), tempMon->monsterID);
					tempMon->dummyAI = FALSE;
				}else{
					easyForcePrintLineClient(pEntity, "***Stopping AI for %s ID:%d.***", tempMon->getClassname(), tempMon->monsterID);
					tempMon->dummyAI = TRUE;
				}
			}else{
				easyForcePrintLineClient(pEntity, "***Entity \"%s\" is not a monster / NPC.***", forwardEnt->getClassname() );
			}
		}
	}else if( FStrEq(pcmdRefinedRef, "stopdrawpath") || FStrEq(pcmdRefinedRef, "pathdrawstop") || FStrEq(pcmdRefinedRef, "stopdrawnpcpath") || FStrEq(pcmdRefinedRef, "drawpathstop") || FStrEq(pcmdRefinedRef, "drawnpcpathstop") ){
		if(g_flWeaponCheat == 0.0){
			easyForcePrintLineClient(pEntity, "Hm. No.");
			return;
		}
		CBaseEntity *pEntityTemp = NULL;

		while ((pEntityTemp = UTIL_FindEntityInSphere( pEntityTemp, tempplayer->pev->origin, 1024*6 )) != NULL)
		{
			CBaseMonster* tempMon = NULL;
			//If this entity is an NPC, draw its bounds.
			if( (tempMon = pEntityTemp->GetMonsterPointer()) != NULL  ){
				tempMon->drawPathConstant = FALSE;
			}
		}//END OF while(things in area)

	}else if( FStrEq(pcmdRefinedRef, "alldrawpath") || FStrEq(pcmdRefinedRef, "pathdrawall") || FStrEq(pcmdRefinedRef, "alldrawnpcpath") || FStrEq(pcmdRefinedRef, "drawpathall") || FStrEq(pcmdRefinedRef, "drawnpcpathall") ){
		if(g_flWeaponCheat == 0.0){
			easyForcePrintLineClient(pEntity, "Hm. No.");
			return;
		}
		CBaseEntity *pEntityTemp = NULL;

		while ((pEntityTemp = UTIL_FindEntityInSphere( pEntityTemp, tempplayer->pev->origin, 1024*6 )) != NULL)
		{
			CBaseMonster* tempMon = NULL;
			//If this entity is an NPC, draw its bounds.
			if( (tempMon = pEntityTemp->GetMonsterPointer()) != NULL  ){
				tempMon->drawPathConstant = TRUE;
			}
		}//END OF while(things in area)

	}else if( FStrEq(pcmdRefinedRef, "testpointcontents") || FStrEq(pcmdRefinedRef, "pointcontents") ){
		
		if(CMD_ARGC() == 1){
			easyForcePrintLineClient(pEntity, "Need the X Y Z (no commas, only spaces) coords of a point to test.");
		}else if(CMD_ARGC() != 4){
			easyForcePrintLineClient(pEntity, "Invalid number of params, 3 expected.");
		}else{
			try{
				float the_x = tryStringToFloat(CMD_ARGV(1));
				float the_y = tryStringToFloat(CMD_ARGV(2));
				float the_z = tryStringToFloat(CMD_ARGV(3));
				Vector theVec = Vector(the_x, the_y, the_z);
				int theContents = UTIL_PointContents(theVec);
				const char* contentsTerm;
				switch(theContents){
				case CONTENTS_EMPTY:contentsTerm = "EMPTY";break;
				case CONTENTS_SOLID:contentsTerm = "SOLID";break;
				case CONTENTS_WATER:contentsTerm = "WATER";break;
				case CONTENTS_SLIME:contentsTerm = "SLIME";break;
				case CONTENTS_LAVA:contentsTerm = "LAVA";break;
				case CONTENTS_SKY:contentsTerm = "SKY";break;
				// Do any of these others have a chance of showing up?  No idea
				case CONTENTS_ORIGIN:contentsTerm = "ORIGIN";break;
				case CONTENTS_CLIP:contentsTerm = "CLIP";break;
				case CONTENTS_CURRENT_0:contentsTerm = "CURRENT_0";break;
				case CONTENTS_CURRENT_90:contentsTerm = "CURRENT_90";break;
				case CONTENTS_CURRENT_180:contentsTerm = "CURRENT_180";break;
				case CONTENTS_CURRENT_270:contentsTerm = "CURRENT_270";break;
				case CONTENTS_CURRENT_UP:contentsTerm = "CURRENT_UP";break;
				case CONTENTS_CURRENT_DOWN:contentsTerm = "CURRENT_DOWN";break;
				case CONTENTS_TRANSLUCENT:contentsTerm = "TRANSLUCENT";break;
				case CONTENTS_LADDER:contentsTerm = "LADDER";break;
				case CONTENTS_FLYFIELD:contentsTerm = "FLYFIELD";break;
				case CONTENTS_GRAVITY_FLYFIELD:contentsTerm = "GRAVITY_FLYFIELD";break;
				case CONTENTS_FOG:contentsTerm = "FOG";break;
				default:contentsTerm = "???";break;

				}//theContents
				easyForcePrintLineClient(pEntity, "Point is %d (%s)", theContents, contentsTerm);
			}catch(int){
				easyForcePrintLineClient(pEntity, "Problem reading coords?");
				return;
			}
		}	
	}else if( FStrEq(pcmdRefinedRef, "drawfov") || FStrEq(pcmdRefinedRef, "fovdraw") || FStrEq(pcmdRefinedRef, "drawnpcfov")  ){
		const char* arg1ref = CMD_ARGV(1);

		if(g_flWeaponCheat == 0.0){
			easyForcePrintLineClient(pEntity, "Hey, that\'s a debugging feature mister!");
			return;
		}

		CBaseEntity* forwardEnt = NULL;
		const char* argTest = CMD_ARGV(1);

		if(!isStringEmpty(argTest)){
			//get the monster with this ID.
			try{
				int numbAttempt = tryStringToInt(arg1ref);
				forwardEnt = getMonsterWithID(numbAttempt);
			}catch(int){
				easyForcePrintLineClient(pEntity, "Problem reading number.  (arg must be whole number)");
				return;
			}
		}else{
			//no argument provided? Try to see if there is a monster in front of the player.
			forwardEnt = FindEntityForward(tempplayer);
		}

		if(forwardEnt == NULL){
			//can't do this.
			easyForcePrintLineClient(pEntity, "***No entity found.***");
		}else{
			CBaseMonster* tempMon;
			if(  (tempMon = forwardEnt->GetMonsterPointer() ) != NULL){
				if(tempMon->drawFieldOfVisionConstant){
					easyForcePrintLineClient(pEntity, "***Stopped drawing FOV for %s ID:%d.***", tempMon->getClassname(), tempMon->monsterID);
					tempMon->drawFieldOfVisionConstant = FALSE;
				}else{
					easyForcePrintLineClient(pEntity, "***Drawing FOV for %s ID:%d.***", tempMon->getClassname(), tempMon->monsterID);
					tempMon->drawFieldOfVisionConstant = TRUE;
				}
			}else{
				easyForcePrintLineClient(pEntity, "***Entity \"%s\" is not a monster / NPC.***", forwardEnt->getClassname() );
			}
		}
	
	}else if( FStrEq(pcmdRefinedRef, "stopdrawfov") || FStrEq(pcmdRefinedRef, "fovdrawstop") || FStrEq(pcmdRefinedRef, "stopdrawnpcfov") || FStrEq(pcmdRefinedRef, "drawfovstop") || FStrEq(pcmdRefinedRef, "drawnpcfovstop") ){
		if(g_flWeaponCheat == 0.0){
			easyForcePrintLineClient(pEntity, "Hm. No.");
			return;
		}
		CBaseEntity *pEntityTemp = NULL;

		while ((pEntityTemp = UTIL_FindEntityInSphere( pEntityTemp, tempplayer->pev->origin, 1024*6 )) != NULL)
		{
			CBaseMonster* tempMon = NULL;
			//If this entity is an NPC, draw its bounds.
			if( (tempMon = pEntityTemp->GetMonsterPointer()) != NULL  ){
				tempMon->drawFieldOfVisionConstant = FALSE;
			}
		}//END OF while(things in area)

	}else if( FStrEq(pcmdRefinedRef, "alldrawfov") || FStrEq(pcmdRefinedRef, "fovdrawall") || FStrEq(pcmdRefinedRef, "alldrawnpcfov") || FStrEq(pcmdRefinedRef, "drawfovall") || FStrEq(pcmdRefinedRef, "drawnpcfovall") ){
		if(g_flWeaponCheat == 0.0){
			easyForcePrintLineClient(pEntity, "Hm. No.");
			return;
		}
		CBaseEntity *pEntityTemp = NULL;

		while ((pEntityTemp = UTIL_FindEntityInSphere( pEntityTemp, tempplayer->pev->origin, 1024*6 )) != NULL)
		{
			CBaseMonster* tempMon = NULL;
			//If this entity is an NPC, draw its bounds.
			if( (tempMon = pEntityTemp->GetMonsterPointer()) != NULL  ){
				tempMon->drawFieldOfVisionConstant = TRUE;
			}
		}//END OF while(things in area)

	}else if( FStrEq(pcmdRefinedRef, "hidemyshit") || FStrEq(pcmdRefinedRef, "hideall") || FStrEq(pcmdRefinedRef, "hidefp") ){

		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(drawHUD, -1);
		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(drawViewModel, 0);
		g_queueCVarHiddenSave = TRUE;
		
	}else if( FStrEq(pcmdRefinedRef, "showmyshit") || FStrEq(pcmdRefinedRef, "showall") || FStrEq(pcmdRefinedRef, "showfp") ){
		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(drawHUD, 1);
		EASY_CVAR_SET_CLIENTONLY_DEBUGONLY_FORCEBROADCAST(drawViewModel, 1);
		g_queueCVarHiddenSave = TRUE;
		
	}else if( FStrEq(pcmdRefinedRef, "setsequence") || FStrEq(pcmdRefinedRef, "sequence") || FStrEq(pcmdRefinedRef, "setanimation") ){
		CBaseEntity* forwardEnt = NULL;
		const char* arg1ref = CMD_ARGV(1);
		int seqNumb;
		
		if(!isStringEmpty(arg1ref)){
			//get the monster with this ID.
			try{
				int numbAttempt = tryStringToInt(arg1ref);
				//forwardEnt = getMonsterWithID(numbAttempt);
				seqNumb = numbAttempt;
			}catch(int){
				easyForcePrintLineClient(pEntity, "Problem reading number.  (arg must be whole number)");
				return;
			}
		}else{
			easyForcePrintLineClient(pEntity, "Must provide sequence number (number from 0 to max number of sequences in model.");
			return;
		}
		
		{
			//no argument provided? Try to see if there is a monster in front of the player.
			forwardEnt = FindEntityForward(tempplayer);
		}

		if(forwardEnt == NULL){
			//can't do this.
			easyForcePrintLineClient(pEntity, "***No entity found.***");
		}else{
			
			CBaseMonster* tempMon;
			if(  (tempMon = forwardEnt->GetMonsterPointer() ) != NULL){
				tempMon->SetSequenceByIndex(seqNumb, 1);
			}else{
				easyForcePrintLineClient(pEntity, "***Entity \"%s\" is not a monster / NPC.***", forwardEnt->getClassname() );
			}
		}

	}else if( FStrEq(pcmdRefinedRef, "apacherocket") || FStrEq(pcmdRefinedRef, "bigrocket") || FStrEq(pcmdRefinedRef, "boom") ){

		if(g_flWeaponCheat == 0.0){
			easyForcePrintLineClient(pEntity, "Mere mortals cannot summon aircraft-grade rockets!");
			return;
		}

		static float side = 1.0;
		static int count;

		Vector itsAngles = (Vector(-tempplayer->pev->v_angle.x, tempplayer->pev->v_angle.y, tempplayer->pev->v_angle.z) + tempplayer->pev->punchangle);
			
		UTIL_MakeVectors(itsAngles);
		Vector vecSrc = tempplayer->pev->origin + tempplayer->pev->view_ofs + (gpGlobals->v_forward * 8);

		/*
		switch( m_iRockets % 5)
		{
		case 0:	vecSrc = vecSrc + gpGlobals->v_right * 10; break;
		case 1: vecSrc = vecSrc - gpGlobals->v_right * 10; break;
		case 2: vecSrc = vecSrc + gpGlobals->v_up * 10; break;
		case 3: vecSrc = vecSrc - gpGlobals->v_up * 10; break;
		case 4: break;
		}
		*/

		/*
		MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, vecSrc );
			WRITE_BYTE( TE_SMOKE );
			WRITE_COORD( vecSrc.x );
			WRITE_COORD( vecSrc.y );
			WRITE_COORD( vecSrc.z );
			WRITE_SHORT( g_sModelIndexSmoke );
			WRITE_BYTE( 20 ); // scale * 10
			WRITE_BYTE( 12 ); // framerate
		MESSAGE_END();
		*/

		CBaseEntity *pRocket = CBaseEntity::Create( "hvr_rocket", vecSrc, itsAngles, tempplayer->edict() );
		if (pRocket){
			pRocket->pev->angles = itsAngles;
			//pRocket->pev->velocity = pev->velocity + gpGlobals->v_forward * 100;
			pRocket->pev->velocity = gpGlobals->v_forward * 165;
		}

	}else if( FStrEq(pcmdRefinedRef, "moveent")){
		
		const char* arg1ref = CMD_ARGV(1);
		//const char* arg2ref = CMD_ARGV(1);

		
		if(arg1ref == NULL || isStringEmpty(arg1ref)){ //|| arg2ref == NULL || isStringEmpty(arg2ref)){
			easyForcePrintLineClient(pEntity, "Insufficient args!  Provide (ID)");
			return;
		}


		if ( tempplayer && g_flWeaponCheat != 0.0)
		{
			edict_t		*pentIgnore;
			TraceResult tr;

			pentIgnore = tempplayer->edict();
			UTIL_MakeVectors(tempplayer->pev->v_angle + tempplayer->pev->punchangle);

			//a tiny bit in front for safety.
			Vector vecSrc = tempplayer->pev->origin + tempplayer->pev->view_ofs + gpGlobals->v_forward * 5;
			Vector vecDest = tempplayer->pev->origin + tempplayer->pev->view_ofs + gpGlobals->v_forward * 2048;

			//nah, precision for while ducking not necessary.
			/*
			Vector playerEyePos = Vector(tempplayer->body;
			if(pev->flags & FL_DUCKING){

			}
			*/

			int searchID = atoi(arg1ref);

			UTIL_TraceLine( vecSrc, vecDest, dont_ignore_monsters, pentIgnore, &tr );
			//tr.vecEndPos();

			if (tr.fAllSolid){
				ALERT ( at_console, "Could not determine spawn location.\n" );
				return;
			}

			/*
			tempplayer->GiveNamedItem( CMD_ARGV(1),  CMD_ARGV(2),
									 tr.vecEndPos.x,
									 tr.vecEndPos.y,
									 tr.vecEndPos.z + EASY_CVAR_GET_DEBUGONLY(offsetgivelookvertical),
									 TRUE, &tr);
			*/

			CBaseMonster* destMon = getMonsterWithID(searchID);
			
			if(destMon == NULL){
				easyForcePrintLineClient(pEntity, "ERROR: could not find monster of id %d", searchID);
			}else{

				//MOVE!
				//destMon->pev->origin = tempplayer->pev->origin;
				UTIL_SetOrigin(destMon->pev, tempplayer->pev->origin);
				/*
				if(destMon->m_pCine != NULL){
					destMon->m_pCine->pev->origin = tempplayer->pev->origin;
				}
				*/
			}
		}//END OF tempPlayer and weapon cheat checks

	}else if(FStrEq(pcmdRefinedRef, "myasshungers")){
		CBaseEntity* forwardEnt = FindEntityForward(tempplayer);

		if(forwardEnt != NULL && FClassnameIs(forwardEnt->pev, "monster_scientist")){

			CTalkMonster* sciRef = static_cast<CTalkMonster*>(forwardEnt);
			sciRef->initiateAss();
		}

	}else if(FStrEq(pcmdRefinedRef, "isthereanyairnodes") || FStrEq(pcmdRefinedRef, "arethereanyairnodes") || FStrEq(pcmdRefinedRef, "airnodes") ){

		if(map_anyAirNodes == TRUE){
			easyForcePrintLineClient(pEntity, "~yes, air nodes present.");
		}else{
			easyForcePrintLineClient(pEntity, "~no air nodes.");
		}
		
	}else if( FStrEq(pcmdRefinedRef, "healcheck") || FStrEq(pcmdRefinedRef, "healers") || FStrEq(pcmdRefinedRef, "wallhealthdoor") || FStrEq(pcmdRefinedRef, "wallhealthdoorcheck") || FStrEq(pcmdRefinedRef, "heallist") || FStrEq(pcmdRefinedRef, "wallhealthdoorlist")  ){
		// I have no idea how horribly out of date this call might be a this point,
		// CHealthDoor, or func_door_health, is now its own entity (used to be a hijacked 'func_door_rotating' that 
		// took healing behavior on seeing the right spawnflag, but it got hard to separate out from garbage/unused
		// spawnflags on old maps).

		// The FORCE_ROTDOOR_TO_HEALTHDOOR constant is a thing, but this is just some debug call


		int i = 0;
		//FL_CLIENT|FL_MONSTER
		BOOL anyYet = FALSE;

		easyForcePrintLine("***HEALER LIST.  Bits 13 to 30 were used to detect healdoors from func_door_rotating, this is no longer done but this list printout will still include func_door_rotating\'s that have any of these spawnflags.***");
			

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////
		edict_t* pEdict = g_engfuncs.pfnPEntityOfEntIndex(1);
		CBaseEntity* pTempEntity;
		if (!pEdict)
			return;  // ???

		// Wait.  "pEdict++" really grabs the next entity just fine?  No need for say
		//     pEdict = g_engfuncs.pfnPEntityOfEntIndex(i)
		// each time?     huh.  imagine that.
		for (i = 1; i < gpGlobals->maxEntities; i++, pEdict++)
		{
			if (pEdict->free)	// Not in use
				continue;

			pTempEntity = CBaseEntity::Instance(pEdict);
			if (!pTempEntity)
				continue;


			int flagFound = -1;

			//that is SF_DOOR_HEAL.

			//(1<<10)   (1<<11) | (1<<12) |
			if (FClassnameIs(pTempEntity->pev, "func_door_health")) {
				easyForcePrint("Oh look, a func_door_health.  Well that was easy.");
				anyYet = TRUE;

			}else if(
				FClassnameIs(pTempEntity->pev, "func_door_rotating") &&
				(pTempEntity->pev->spawnflags & ( (1<<10) | (1<<11) | (1<<12) | (1<<13) | (1<<14) | (1<<15) | (1<<16) | (1<<17) | (1<<18) | (1<<19) | (1<<20) | (1<<21) | (1<<22) | (1<<23) | (1<<24) | (1<<25) | (1<<26) | (1<<27) | (1<<28) | (1<<29) | (1<<30) ) )
			)
			{
				anyYet = TRUE;

				easyForcePrint("HELLO func_door_rotating YOU HAVE STRANGE SPAWNFLAGS #%d classname:%s netname:%s target:%s targetname:%s origin:(%.2f, %.2f, %.2f) spawnflags:",
					i,
					pTempEntity->pev->classname!=NULL?STRING(pTempEntity->pev->classname):"_",
					pTempEntity->pev->netname!=NULL?STRING(pTempEntity->pev->netname):"_",
					pTempEntity->pev->target!=NULL?STRING(pTempEntity->pev->target):"_",
					pTempEntity->pev->targetname!=NULL?STRING(pTempEntity->pev->targetname):"_",
					pTempEntity->pev->origin.x, pTempEntity->pev->origin.y, pTempEntity->pev->origin.z
				);
				printIntAsBinary((unsigned int)pTempEntity->pev->spawnflags, 32u);
				easyForcePrintLine();
			}
		}//END
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////

		if(!anyYet){
			easyForcePrintLine("*No wallDoorHealth\'s or like-flagged things found!");
		}


	}else if(FStrEq(pcmdRefinedRef, "forcenodeupdate") || FStrEq(pcmdRefinedRef, "rebuildgraph") || FStrEq(pcmdRefinedRef, "rebuildnodegraph") || FStrEq(pcmdRefinedRef, "buildgraph") || FStrEq(pcmdRefinedRef, "buildnodegraph") || FStrEq(pcmdRefinedRef, "buildnodes") || FStrEq(pcmdRefinedRef, "rebuildnodes")){
		int i;
		if(g_flWeaponCheat == 0.0){
			easyForcePrintLineClient(pEntity, "Enable cheats first. \"scheduleNodeUpdate\" does not require cheats.");
			return;
		}

		if(WorldGraph.m_cNodes > 0){
			//Need to keep track of created nodes for calling their ->spawn's.
			int nodeCount = WorldGraph.m_cNodes;
			CNodeEnt** aryNodeEnt = new CNodeEnt*[nodeCount];

			
			//Build node ent's from the existing nodes on the map to rebuild the nodes and connections.
			for( i = 0; i < nodeCount; i++){
				const char* spawnName;
				if(WorldGraph.m_pNodes[i].m_afNodeInfo & bits_NODE_AIR){
					spawnName = "info_node_air";
				}else{
					spawnName = "info_node";
				}

				//"CreateManual" instead of "Create" skips the DispatchSpawn call.
				//Not that it may even be necessary at all for node entities.  Calling ->spawn straight should do it.
				CNodeEnt* thisNode = static_cast<CNodeEnt*>(CBaseEntity::CreateManual(
					spawnName,
					WorldGraph.m_pNodes[ i ].m_vecOrigin,
					Vector(0, WorldGraph.m_pNodes[ i ].m_flHintYaw, 0)
				));

				aryNodeEnt[i] = thisNode;
			
				thisNode->m_sHintType = WorldGraph.m_pNodes[ i ].m_sHintType;
				thisNode->m_sHintActivity = WorldGraph.m_pNodes[ i ].m_sHintActivity;
			}//END OF for

			// Clear the graph to signify it needs rebuilding.
			WorldGraph.InitGraph();
			WorldGraph.AllocNodes();
			
			// Now call each created node's Spawn.
			for(i = 0; i < nodeCount; i++){
				aryNodeEnt[i]->Spawn();
			}
			
			// Any air nodes? need to let the world know.
			// ...oh this has to be difficult.
			
			CWorld* theWorld = getWorld();
			if(theWorld != NULL){
				theWorld->getCustomMapSettingsFromGlobal();
			}
			
			delete[] aryNodeEnt;

			// It may be tempting to call WorldGraph.BuildNodeGraph, but don't.
			// This is called by the TestHull generated by the first spawned node noticing that the graph has been blanked.
			// The TestHull waits a frame (assumes all other nodes have spawned in the previous frame), and calls
			// BuildNodeGraph itself.  But prove with a breakpoint if you want.

			easyForcePrintLineClient(pEntity, "Nodes rebuilding...");
		}else{
			easyForcePrintLineClient(pEntity, "ERROR: no nodes present or building in progress.");
		}

	}else if( FStrEq(pcmdRefinedRef, "clearnodecache") || FStrEq(pcmdRefinedRef, "resetnodecache")){
		// Weaker call than rebuilding all nodes.  Force the node cache to reset in case of changing pathfinding-related CVars related
		// to shortest-path like pathfindIgnoreIsolatedNodes.

		memset(WorldGraph.m_Cache, 0, sizeof(WorldGraph.m_Cache));
		easyForcePrintLineClient(pEntity, "***Node cache cleared.");

	}else if ( FStrEq(pcmdRefinedRef, "fov" ) )
	{
		if ( g_flWeaponCheat && CMD_ARGC() > 1)
		{
			GetClassPtr((CBasePlayer *)pev)->m_iFOV = atoi( CMD_ARGV(1) );
		}
		else
		{
			CLIENT_PRINTF( pEntity, print_console, UTIL_VarArgs( "\"fov\" is \"%d\"\n", (int)GetClassPtr((CBasePlayer *)pev)->m_iFOV ) );
		}
	}
	else if ( FStrEq(pcmdRefinedRef, "use" ) )
	{
		GetClassPtr((CBasePlayer *)pev)->SelectItem((char *)CMD_ARGV(1));
	}
	else if (((pstr = strstr(pcmdRefinedRef, "weapon_")) != NULL)  && (pstr == pcmdRefinedRef))
	{
		//easyForcePrintLineClient(pEntity, "HOW THE %s", pcmdRefinedRef);
		GetClassPtr((CBasePlayer *)pev)->SelectItem(pcmd);
	}
	else if (FStrEq(pcmdRefinedRef, "_lastinv" ))
	{
		GetClassPtr((CBasePlayer *)pev)->SelectLastItem();
	}
	else if ( FStrEq( pcmdRefinedRef, "spectate" ) && (pev->flags & FL_PROXY) )	// added for proxy support
	{
		CBasePlayer * pPlayer = GetClassPtr((CBasePlayer *)pev);

		edict_t *pentSpawnSpot = g_pGameRules->GetPlayerSpawnSpot( pPlayer );
		pPlayer->StartObserver( pev->origin, VARS(pentSpawnSpot)->angles);
	}
	//NOTE: sending the raw ,non-forced-lowercase string.  Hard to say what this "gameRules" wants it to do.
	else if ( g_pGameRules->ClientCommand( GetClassPtr((CBasePlayer *)pev), pcmd ) )
	{

		//easyForcePrintLineClient(pEntity, "AWWWWW SNAP SON????");

		// MenuSelect returns true only if the command is properly handled,  so don't print a warning
	}
	else
	{
		caughtByFirst = FALSE;
		//script moveed....
	}

	if(caughtByFirst){
		return;   //skip the hidden CVar stuff below, no need for extra interpretation.
	}

	caughtByFirst = TRUE;


	// Thanks for the else-if chain limit, guys...

	if (FStrEq(pcmdRefinedRef, "schedulenodeupdate")) {

		//the next map loaded will enforce regenerating the node graph regardless of coming from a file already or not.
		scheduleNodeUpdate = TRUE;
		easyForcePrintLineClient(pEntity, "Scheduling node update. Start a map to rebuild nodes / skip node file.");

	}
	else if (FStrEq(pcmdRefinedRef, "blockalltriggers") || FStrEq(pcmdRefinedRef, "unblockalltriggers")) {

		if (g_flWeaponCheat == 0.0) {
			easyForcePrintLineClient(pEntity, "Let\'s not toy with integral map features.  Without cheats that is.");
			return;
		}


		const char* arg1ref = CMD_ARGV(1);
		int argValue;
		float argValueAsFloat;

		//what's our word choice?
		int defaultValue = 0;
		if (FStrEq(pcmdRefinedRef, "blockalltriggers")) {
			defaultValue = 1;
		}

		if (!isStringEmpty(arg1ref)) {
			//get the monster with this ID.
			try {
				int numbAttempt = tryStringToInt(arg1ref);
				//forwardEnt = getMonsterWithID(numbAttempt);
				argValue = numbAttempt;
				if (!(argValue == 0 || argValue == 1)) {
					throw 1;
				}
			}
			catch (int) {
				easyForcePrintLineClient(pEntity, "Problem reading number.  (arg must be 0 or 1)");
				return;
			}

			//proceed to interpret argValue.
			if (defaultValue == 0) {
				//the word "unblock" was used? it means the opposite instead.
				argValue = (argValue == 1) ? argValue = 0 : argValue = 1;
			}

		}
		else {
			//assume it is what was typed.
			argValue = defaultValue;
		}

		argValueAsFloat = (float)argValue;

		EASY_CVAR_SET_DEBUGONLY(blockAutosaveTrigger, argValueAsFloat);
		EASY_CVAR_SET_DEBUGONLY(blockChangeLevelTrigger, argValueAsFloat);
		EASY_CVAR_SET_DEBUGONLY(blockMultiTrigger, argValueAsFloat);
		EASY_CVAR_SET_DEBUGONLY(blockTeleportTrigger, argValueAsFloat);
		EASY_CVAR_SET_DEBUGONLY(blockHurtTrigger, argValueAsFloat);
		EASY_CVAR_SET_DEBUGONLY(blockMusicTrigger, argValueAsFloat);
		g_queueCVarHiddenSave = TRUE;

		if (argValue == 0) {
			easyForcePrintLineClient(pEntity, "All triggers unblocked.");
		}
		else {
			easyForcePrintLineClient(pEntity, "All triggers blocked.");
		}


	}
	else if (FStrEq(pcmdRefinedRef, "testangles") || FStrEq(pcmdRefinedRef, "angletest")) {
		easyForcePrintLineClient(pEntity, "***MAke sure mode printouts are enabled.");

		//test angles, like pev->angles.
		//angles are
		//x: pitch (orientation aimed up/down from facing straight horizontal across, think of from a side-view, 0 is straight horizontal, positive is tilted to look up, negative is tilted to look down),
		//y: yaw (orientation looking in a direction against the floor, think of from a top-down view like a circle, with the typical north, east, south, west directions to go in).
		//z: roll (think of it as adjacent to pitch. look at the model from the front instead, and turn it in a swivelling way (sideways).)
		Vector angleTest = Vector(36, 90, 0);

		Vector vecForward1;
		Vector vecRight1;
		Vector vecUp1;

		Vector vecForward2;
		Vector vecRight2;
		Vector vecUp2;

		easyForcePrintClient(pEntity, "Private Plain Vectors:");
		UTIL_MakeVectorsPrivate(angleTest, vecForward1, vecRight1, vecUp1);
		UTIL_printVectorClient(pEntity, vecForward1);
		UTIL_printVectorClient(pEntity, vecRight1);
		UTIL_printVectorClient(pEntity, vecUp1);
		easyForcePrintLineClient(pEntity);


		easyForcePrintClient(pEntity, "Private Aim Vectors:");
		UTIL_MakeAimVectorsPrivate(angleTest, vecForward2, vecRight2, vecUp2);
		UTIL_printVectorClient(pEntity, vecForward2);
		UTIL_printVectorClient(pEntity, vecRight2);
		UTIL_printVectorClient(pEntity, vecUp2);
		easyForcePrintLineClient(pEntity);


		easyForcePrintClient(pEntity, "Global Plain Vectors:");
		UTIL_MakeVectors(angleTest);
		UTIL_printVectorClient(pEntity, gpGlobals->v_forward);
		UTIL_printVectorClient(pEntity, gpGlobals->v_right);
		UTIL_printVectorClient(pEntity, gpGlobals->v_up);
		easyForcePrintLineClient(pEntity);

		easyForcePrintClient(pEntity, "Global Aim Vectors:");
		UTIL_MakeAimVectors(angleTest);
		UTIL_printVectorClient(pEntity, gpGlobals->v_forward);
		UTIL_printVectorClient(pEntity, gpGlobals->v_right);
		UTIL_printVectorClient(pEntity, gpGlobals->v_up);
		easyForcePrintLineClient(pEntity);



	}
	else if (FStrEq(pcmdRefinedRef, "debug1")) {
		DebugCall1(tempplayer);
	}
	else if (FStrEq(pcmdRefinedRef, "debug2")) {
		DebugCall2(tempplayer);
	}
	else if (FStrEq(pcmdRefinedRef, "debug3")) {
		DebugCall3(tempplayer);
	}
	else if (FStrEq(pcmdRefinedRef, "getnormalvector") || FStrEq(pcmdRefinedRef, "getnormal") || FStrEq(pcmdRefinedRef, "normalvector") || FStrEq(pcmdRefinedRef, "normal")) {
		if (tempplayer)
		{
			if (g_flWeaponCheat != 0.0) {
				edict_t* pentIgnore;
				TraceResult tr;

				pentIgnore = tempplayer->edict();
				UTIL_MakeVectors(tempplayer->pev->v_angle + tempplayer->pev->punchangle);

				//a tiny bit in front for safety.
				Vector vecSrc = tempplayer->pev->origin + tempplayer->pev->view_ofs + gpGlobals->v_forward * 5;
				Vector vecDest = tempplayer->pev->origin + tempplayer->pev->view_ofs + gpGlobals->v_forward * 2048;

				//nah, precision for while ducking not necessary.
				/*
				Vector playerEyePos = Vector(tempplayer->body;
				if(pev->flags & FL_DUCKING){

				}
				*/


				UTIL_TraceLine(vecSrc, vecDest, dont_ignore_monsters, pentIgnore, &tr);
				//tr.vecEndPos();

				if (!tr.fAllSolid) {
					if (tr.flFraction < 1.0) {
						CBaseEntity* pEntityHit = CBaseEntity::Instance(tr.pHit);

						easyForcePrintLineClient(pEntity, "Name:%s HitLoc:(%.2f, %.2f, %.2f) Normal:(%.2f, %.2f, %.2f)",
							(pEntityHit != NULL ? pEntityHit->getClassnameShort() : "NULL"),
							tr.vecEndPos.x,
							tr.vecEndPos.y,
							tr.vecEndPos.z,
							tr.vecPlaneNormal.x,
							tr.vecPlaneNormal.y,
							tr.vecPlaneNormal.z
						);
					}
					else {
						easyForcePrintLineClient(pEntity, "WARNING: trace failed; flFraction is 1.0 (did not hit anything).");
					}

				}//END OF line trace valid check
				else {
					easyForcePrintLineClient(pEntity, "WARNING: trace failed; AllSolid.");
				}

			}
			else {
				easyForcePrintLineClient(pEntity, "Enable cheats to use getNormalVector.");
			}
		}//END OF player and cheat check

	}
	else if (FStrEq(pcmdRefinedRef, "chillout") || FStrEq(pcmdRefinedRef, "chill") || FStrEq(pcmdRefinedRef, "relax") || FStrEq(pcmdRefinedRef, "relaxbuddy") || FStrEq(pcmdRefinedRef, "smokeweed") || FStrEq(pcmdRefinedRef, "fageddaboutit") || FStrEq(pcmdRefinedRef, "forgetaboutit") || FStrEq(pcmdRefinedRef, "thesearenothtedroidsyouarelookingfor") || FStrEq(pcmdRefinedRef, "jedimindtrick")) {

		if (g_flWeaponCheat) {
			CBaseEntity* pEntityTemp = NULL;

			while ((pEntityTemp = UTIL_FindEntityInSphere(pEntityTemp, tempplayer->pev->origin, 1024 * 2)) != NULL)
			{
				CBaseMonster* monsterTest = pEntityTemp->GetMonsterPointer();
				if (monsterTest != NULL) {
					monsterTest->ForgetEnemy();
				}
			}//END OF while(things in area)
		}
		else {
			easyForcePrintLineClient(pEntity, "Captain Retrospect says: You shouldn\'t have pissed them off.\n");
		}
	}
	else if (FStrEq(pcmdRefinedRef, "removeallmonsters") ) {

		if (g_flWeaponCheat) {
			if (CMD_ARGC() <= 1) {
				// good, they meant it.
				removeAllMonsters(pEntity);
			}
			else {
				easyForcePrintLineClient(pEntity, "WARNING: removeallmonsters does not accept any parameters.  Do you mean removeallmonstersexcept?");
			}
		}else {
			easyForcePrintLineClient(pEntity, "Nope.");
		}
	}
	else if (FStrEq(pcmdRefinedRef, "removeallmonstersexcept")) {
		if (g_flWeaponCheat) {
			if (CMD_ARGC() > 1) {
				int tempNumb;
				int theID = -1;
				
				try {
					tempNumb = tryStringToInt(CMD_ARGV(1));
					theID = tempNumb;
					removeAllMonstersExcept(pEntity, theID);
				}
				catch (int) {
					easyForcePrintClient(pEntity, "ID must be an integer.");
				}

			}
			else {
				easyForcePrintClient(pEntity, "Must specify an ID not to be removed!  Support for multiple exception ID\'s can be added if necessary.");
			}
		}
		else {
			easyForcePrintClient(pEntity, "Nope.");
		}
	}
	else if (FStrEq(pcmdRefinedRef, "removeallentities")) {
		easyForcePrintLineClient(pEntity, "Sorry, too dangerous with or without cheats. Bye.");
		return;

		if (g_flWeaponCheat) {
			removeAllEntities(pEntity);
		}
		else {
			easyForcePrintLineClient(pEntity, "Nope.");
		}
	}
	else if (FStrEq(pcmdRefinedRef, "listallentities")) {
		if (g_flWeaponCheat == 0.0) {
			easyForcePrintLineClient(pEntity, "Woa hey.  You need cheats.");
			return;
		}
		listAllEntities(pEntity);
	}
	else if (FStrEq(pcmdRefinedRef, "listallmonsters")) {
		if (g_flWeaponCheat == 0.0) {
			easyForcePrintLineClient(pEntity, "Woa hey.  You need cheats.");
			return;
		}
		listAllMonsters(pEntity);
	}
	else if (FStrEq(pcmdRefinedRef, "basicentityinfo")) {
		// Is this redundant with listallentities above?  Although this shows a little more
		if (g_flWeaponCheat == 0.0) {
			easyForcePrintLineClient(pEntity, "Woa hey.  You need cheats.");
			return;
		}
		int i;
		int highextUsedEntityIndex = -1;
		easyForcePrintLine("---------------------------------------------------------");

		// first count from maxentities down.  What is the greatest entity index that's ever used?
		// No need to print out 800 lines of NULL entity info.
		for (i = gpGlobals->maxEntities-1; i >= 0; i--) {
			edict_t* pEdi = g_engfuncs.pfnPEntityOfEntIndex(i);
			if (pEdi != NULL) {
				// ok.
				highextUsedEntityIndex = i;
				break;
			}
		}

		// go one beeyond, if allowed.
		if (highextUsedEntityIndex < gpGlobals->maxEntities - 1) {
			highextUsedEntityIndex++;
		}


		for (i = 0;  i <= highextUsedEntityIndex; i++) {
			edict_t* pEdi = g_engfuncs.pfnPEntityOfEntIndex(i);
			easyForcePrintStarter();
			if (pEdi != NULL) {
				easyForcePrint("%d Null? NO free?%d entindex:%i eoffset:%i", i, pEdi->free, ENTINDEX(pEdi), OFFSET(pEdi) );

				if (pEdi->v.netname != NULL) {
					easyForcePrint(" netname:%s", STRING(pEdi->v.netname));
				}
				if (pEdi->v.target != NULL) {
					easyForcePrint(" target:%s", STRING(pEdi->v.target));
				}
				if (pEdi->v.targetname != NULL) {
					easyForcePrint(" targetname:%s", STRING(pEdi->v.targetname));
				}
				if (pEdi->v.owner != NULL) {
					easyForcePrint(" owner:%s", STRING(pEdi->v.owner->v.classname));
				}
				

				CBaseEntity* tempEnt = CBaseEntity::Instance(pEdi);
				if (tempEnt != NULL) {
					easyForcePrint(" class:%s deadflag:%d", tempEnt->getClassname(), tempEnt->pev->deadflag);
				}
				// tempEnt->entindex(), or ENTINDEX(pEdi)
				// tempEnt->eoffset(), or OFFSET(pEdi)
			}
			else {
				easyForcePrint("%d Null? YES", i);
			}
			easyForcePrintLine();
		}
		if (highextUsedEntityIndex < gpGlobals->maxEntities - 1) {
			// some entities skipped?  make note of that.
			easyForcePrintLine("...");
		}
		easyForcePrintLine("---------------------------------------------------------");

	}
	
	/*
	else if (FStrEq(pcmdRefinedRef, "basicentityinfooffset")) {
		
		//NOTICE!!! Not a wise thing to even try.
		// Looks like offset is related to how much memory the pentity takes up and still starting
		// from the first entity... or wherever edicts are stored in memory probably.
		// Anyway, offsets are in increments of 804, but not really a point when getting by index (1, 2, 3)
		// works perfectly fine.
		
		easyForcePrintLine("---------------------------------------------------------");
		for (int i = 0; i < gpGlobals->maxEntities; i++) {
			edict_t* pEdi = g_engfuncs.pfnPEntityOfEntOffset(i);
			easyForcePrintStarter();
			if (pEdi != NULL) {
				easyForcePrint("%d Null? NO free?%d entindex:%i eoffset:%i ", i, pEdi->free, ENTINDEX(pEdi), OFFSET(pEdi));
				// garbage, for... some... reason.   No freakin clue.
				//CBaseEntity* tempEnt = CBaseEntity::Instance(pEdi);
				//if (tempEnt != NULL) {
				//	...
				//}
			}
			else {
				easyForcePrint("%d Null? YES", i);
			}
			easyForcePrintLine();
		}
		easyForcePrintLine("---------------------------------------------------------");
		
	}
	*/
	else if (FStrEq(pcmdRefinedRef, "testalt")) {
		
		const char* arg1ref = CMD_ARGV(1);

		/*
		if(FStrEq(arg1ref, "0") == TRUE) {
			MESSAGE_BEGIN(MSG_ONE, gmsgCliTest, NULL, pev);
			MESSAGE_END();
		}
		else if (FStrEq(arg1ref, "1") == TRUE) {

			MESSAGE_BEGIN(MSG_ALL, SVC_CDTRACK);
			WRITE_BYTE(3);
			WRITE_BYTE(3);
			MESSAGE_END();
		}
		else if (FStrEq(arg1ref, "2") == TRUE) {

			MESSAGE_BEGIN(MSG_ALL, SVC_INTERMISSION);
			MESSAGE_END();
		}
		else {
			easyForcePrintClient(pEntity, "????????");
		}
		*/

		/*
		if (CMD_ARGC() < 3) {
			easyForcePrintClient(pEntity, "AW SHIT");
		}else {
			int int1 = tryStringToFloat(CMD_ARGV(1));
			int int2 = tryStringToFloat(CMD_ARGV(2));

			MESSAGE_BEGIN(MSG_ALL, SVC_CDTRACK);
			WRITE_BYTE(int1);
			WRITE_BYTE(int2);
			MESSAGE_END();
		}
		*/

		MESSAGE_BEGIN(MSG_ALL, SVC_CDTRACK);
		WRITE_BYTE(2);
		WRITE_BYTE(0);
		MESSAGE_END();

	}else if (FStrEq(pcmdRefinedRef, "_mod_version_server")) {
		char aryChr[128];
		char aryChrD[128];
		writeVersionInfo(aryChr, 128);
		writeDateInfo(aryChrD, 128);

		method_precacheAll();

		easyForcePrintLineClient(pEntity, "AZ az.dll  Version: %s  Date: %s", aryChr, aryChrD);
	}
	else if (FStrEq(pcmdRefinedRef, "_cl_fvox")) {
		if (CMD_ARGC() <= 1) {
			// doesn't make sense to do this if so.  And yes, "1" because that's the lowest possible for just the console
			// command term itself.  First argument starts at CMD_ARGV(1).
			return;
		}
		const char* arg1ref = CMD_ARGV(1);
		CBasePlayer* tempRef = GetClassPtr((CBasePlayer*)pev);

		BOOL setSilent = FALSE;
		if (CMD_ARGC() == 3) {
			// Also to make the player stop making noise.
			const char* arg2ref = CMD_ARGV(2);
			if (FStrEq(arg2ref, "1")) {
				//tempRef->fvoxEnabledMem = tempRef->fvoxEnabled;
				setSilent = TRUE;
			}
		}
		if (FStrEq(arg1ref, "0")) {
			tempRef->set_fvoxEnabled(FALSE, setSilent);
		}
		else {
			tempRef->set_fvoxEnabled(TRUE, setSilent);
		}
	}
	else if (FStrEq(pcmdRefinedRef, "_cl_holster")) {
		if (CMD_ARGC() <= 1) {
			// doesn't make sense to do this if so.  And yes, "1" because that's the lowest possible for just the console
			// command term itself.  First argument starts at CMD_ARGV(1).
			return;
		}
		const char* arg1ref = CMD_ARGV(1);
		CBasePlayer* tempRef = GetClassPtr((CBasePlayer*)pev);

		if (FStrEq(arg1ref, "0")) {
			tempRef->fHolsterAnimsEnabled = FALSE;
		}else {
			tempRef->fHolsterAnimsEnabled = TRUE;
		}
	}
	else if (FStrEq(pcmdRefinedRef, "_cl_breakholster")) {
		if (CMD_ARGC() <= 1) {
			// doesn't make sense to do this if so.  And yes, "1" because that's the lowest possible for just the console
			// command term itself.  First argument starts at CMD_ARGV(1).
			return;
		}
		const char* arg1ref = CMD_ARGV(1);
		CBasePlayer* tempRef = GetClassPtr((CBasePlayer*)pev);

		if (FStrEq(arg1ref, "0")) {
			tempRef->fBreakHolster = FALSE;
		}else {
			tempRef->fBreakHolster = TRUE;
		}
	}
	else if (FStrEq(pcmdRefinedRef, "_cl_ladder")) {
		if (CMD_ARGC() <= 1) {
			return;
		}
		const char* arg1ref = CMD_ARGV(1);
		CBasePlayer* tempRef = GetClassPtr((CBasePlayer*)pev);
		
		tempRef->set_cl_ladder_choice(tryStringToFloat(arg1ref));
	}
	else if (FStrEq(pcmdRefinedRef, "_default_fov")) {
		if (CMD_ARGC() <= 1) {
			return;
		}
		const char* arg1ref = CMD_ARGV(1);
		CBasePlayer* tempRef = GetClassPtr((CBasePlayer*)pev);
		
		tempRef->default_fov = tryStringToFloat(arg1ref);
	}
	else if (FStrEq(pcmdRefinedRef, "_auto_adjust_fov")) {
		if (CMD_ARGC() <= 1) {
			return;
		}
		const char* arg1ref = CMD_ARGV(1);
		CBasePlayer* tempRef = GetClassPtr((CBasePlayer*)pev);
		
		tempRef->auto_adjust_fov = tryStringToFloat(arg1ref);
	}
	else if (FStrEq(pcmdRefinedRef, "_auto_determined_fov")) {
		if (CMD_ARGC() <= 1) {
			return;
		}
		const char* arg1ref = CMD_ARGV(1);
		CBasePlayer* tempRef = GetClassPtr((CBasePlayer*)pev);
		
		tempRef->auto_determined_fov = tryStringToFloat(arg1ref);
	}
	else if (FStrEq(pcmdRefinedRef, "cmdclient") || (FStrEq(pcmdRefinedRef, "clientcmd")) ) {
		char arychr_buffer[64];
		// CMD_ARGS() sends all individual CMD_ARGV(#) put together for convenience.
		if (CMD_ARGC() > 1) {
			//strcpy(&arychr_buffer[0], CMD_ARGS());
			sprintf(&arychr_buffer[0], "%s%s", CMD_ARGS(), "\n");
			submitJukeboxRequest(pev, arychr_buffer);
		}else {
			//not over 1?  You provided no more than just the command text!
			easyForcePrintLineClient(pEntity, "***Can not send a null string!  Give more.***");
		}

		//sprintf(arychr_buffer, "cd play %3d\n", iTrack);
		//CLIENT_COMMAND(pClient, arychr_buffer);
		//char* cmdStringSend = UTIL_VarArgs("mp3 play media/%s.mp3", eh[choice]);
		//CLIENT_COMMAND(tempEd, cmdStringSend);
		//submitJukeboxRequest(pev, "anus");

	}
	else if (FStrEq(pcmdRefinedRef, "cmdserver") || (FStrEq(pcmdRefinedRef, "servercmd")) ) {
		if (g_flWeaponCheat == 0.0) {
			easyForcePrintLineClient(pEntity, "Woa hey.  You need cheats for that.");
			return;
		}
		char arychr_buffer[64];
		// CMD_ARGS() sends all individual CMD_ARGV(#) put together for convenience.
		if (CMD_ARGC() > 1) {
			//char* leaveOff = strcpy(&arychr_buffer[0], CMD_ARGS());
			//strcpy(leaveOff, "\n");

			sprintf(&arychr_buffer[0], "%s%s", CMD_ARGS(), "\n");


			//submitJukeboxRequest(pev, arychr_buffer);
			g_engfuncs.pfnServerCommand(arychr_buffer);
		}
		else {
			//not over 1?  You provided no more than just the command text!
			easyForcePrintLineClient(pEntity, "***Can not send a null string!  Give more.***");

			//SERVER_COMMAND("reload\n");
		}

	}
	else if (FStrEq(pcmdRefinedRef, "jolt") || FStrEq(pcmdRefinedRef, "fling") || FStrEq(pcmdRefinedRef, "boost") || (FStrEq(pcmdRefinedRef, "push"))) {
		const char* arg1ref = CMD_ARGV(1);
		if (g_flWeaponCheat == 0.0) {
			easyForcePrintLineClient(pEntity, "No summoning inexplicable forces without cheats.");
			return;
		}
		
		float forceForward;

		if (arg1ref == NULL) {
			forceForward = 800;  //default
		}
		else {
			try{
				float tempFloat = tryStringToFloat(arg1ref);
				forceForward = tempFloat;
			}
			catch (int) {
				forceForward = 800;
				easyForcePrintLineClient(pEntity, "Aw man!  Your number sucked.");
			}
		}

		// push forward!
		UTIL_MakeVectors(pev->v_angle);
		tempplayer->pev->origin = tempplayer->pev->origin + Vector(0, 0, 12);
		tempplayer->pev->velocity = gpGlobals->v_forward * forceForward;
		
	}
	else if (FStrEq(pcmdRefinedRef, "changelevel3") ) {
		if (g_flWeaponCheat == 0.0) {
			easyForcePrintLineClient(pEntity, "Woa hey.  You need cheats.");
			return;
		}
		char arychr_buffer[64];
		// CMD_ARGS() sends all individual CMD_ARGV(#) put together for convenience.
		if (CMD_ARGC() < 3) {
			easyForcePrintLineClient(pEntity, "***changelevel3 needs two params: map name and spot name (immitates a transition call)***");
			//easyForcePrintLineClient(pEntity, "***One parameter is supported (map name), as seen in multiplayer gamerules usage, but it doesn't appear to do anything.***");
			
		}
		else {
			easyForcePrintLineClient(pEntity, "***Standby for changelevel (close console!)***");
			char* tempstr1 = (char*)CMD_ARGV(1);
			char* tempstr2 = (char*)CMD_ARGV(2);

			// send the parameters over to temp buffers to be played soon (when the console's closed, game unpaused)
			strcpy(&changeLevelQueuedName[0], tempstr1);

			if (tempstr2 != NULL) {
				strcpy(&changeLevelQueuedSpot[0], tempstr2);
			}else {
				// whut.  Force to the empty string.
				changeLevelQueuedSpot[0] = '\0';
			}

			// play it next time we're unpaused, likely out of console
			//changeLevelQueuedTime = gpGlobals->time + 0.1f
			changeLevelQueuedTime = gpGlobals->time + 0.5f;

			//CHANGE_LEVEL(tempstr1, tempstr2);
		}
	}
	else if (FStrEq(pcmdRefinedRef, "motd_show")) {
		// the command "motd" is hardcoded, strangely enough, and doesn't show the MOTD again like startup does (connected to server).
		// so how about this

		if (IsMultiplayer()) {
			CHalfLifeMultiplay* temp_multiPlayRules = static_cast<CHalfLifeMultiplay*>(g_pGameRules);
			if (temp_multiPlayRules) {
				temp_multiPlayRules->SendMOTDToClient( ENT(pev) );
			}
		}
	}
	else if (FStrEq(pcmdRefinedRef, "printweapons")) {
		easyForcePrintClient(tempplayer->edict(), "***Printing your weapons***");
		tempplayer->printOutWeapons();
	}
	else if (FStrEq(pcmdRefinedRef, "removeallitems") || FStrEq(pcmdRefinedRef, "removeitems")) {
		if (g_flWeaponCheat == 0.0) {
			easyForcePrintLineClient(pEntity, "Need cheats for that... weirdly.");
			return;
		}

		tempplayer->RemoveAllItems(TRUE);
	}else if ( FStrEq(pcmdRefinedRef, "removeallweapons") || (FStrEq(pcmdRefinedRef, "removeweapons") ) || FStrEq(pcmdRefinedRef, "strip") || FStrEq(pcmdRefinedRef, "stripme")) {
		if (g_flWeaponCheat == 0.0) {
			easyForcePrintLineClient(pEntity, "Need cheats for that... weirdly.");
			return;
		}

		// not the suit this time.
		tempplayer->RemoveAllItems(FALSE);
	}
	else if (FStrEq(pcmdRefinedRef, "removeallammo") || (FStrEq(pcmdRefinedRef, "removeammo")) ) {
		if (g_flWeaponCheat == 0.0) {
			easyForcePrintLineClient(pEntity, "Need cheats for that... weirdly.");
			return;
		}

		// only remove all ammo's.

		tempplayer->RemoveAllAmmo();

		
	}
	else if (FStrEq(pcmdRefinedRef, "removesuit") ) {
		if (g_flWeaponCheat == 0.0) {
			easyForcePrintLineClient(pEntity, "Need cheats for that... weirdly.");
			return;
		}

		tempplayer->pev->weapons &= ~(1 << WEAPON_SUIT);
		//tempplayer->UpdateClientData();  //and let clientside know
	}
	else if (FStrEq(pcmdRefinedRef, "removepowercanisters") || FStrEq(pcmdRefinedRef, "removesyringes") || FStrEq(pcmdRefinedRef, "removesidebar")) {
		if (g_flWeaponCheat == 0.0) {
			easyForcePrintLineClient(pEntity, "Need cheats for that... weirdly.");
			return;
		}
		int i;
		for (i = 0; i < MAX_ITEMS; i++) {
			tempplayer->m_rgItems[i] = 0;
		}
		// may as well
		tempplayer->airTankAirTime = 0;
		tempplayer->longJumpCharge = 0;

	}







	




	// NEW things HERE MAYBE

	
	else if (FStrEq(pcmdRefinedRef, "tcs_init_link")) {
		global_test_cvar_ref = CVAR_GET_POINTER("test_cvar");
		easyForcePrintLineClient(pEntity, "***server cvar ref established, probably.  Found? %d", (global_test_cvar_ref != NULL));

		easyForcePrintLineClient(pEntity, "1 test %d %.2f %s end", 45, 45.5, "str");
		easyPrintLineClient(pEntity,      "2 test? %d %.2f %s end", 45, 45.5, "str");
	}
	else if (FStrEq(pcmdRefinedRef, "tcs_reset")) {
		
		global_test_cvar_ref = CVAR_GET_POINTER("test_cvar");
		if (global_test_cvar_ref != NULL) {
			global_test_cvar_ref->value = 6;
		}
		CVAR_SET_FLOAT("test_cvar", 6);
		easyForcePrintLineClient(pEntity, "***server cvar reset.");
		
	}
	else if (FStrEq(pcmdRefinedRef, "tcs_set_direct")) {
		CVAR_SET_FLOAT("test_cvar", 13.0f);
		easyForcePrintLineClient(pEntity, "***Set to 13 success, probably.");
	}
	else if (FStrEq(pcmdRefinedRef, "tcs_set_struct")) {
		//cvar_t* tempRef = CVAR_GET_POINTER("test_cvar");
		cvar_t* tempRef = global_test_cvar_ref;
		if (tempRef != NULL) {
			tempRef->value = 13.0f;
			easyForcePrintLineClient(pEntity, "***Set to 13 success, probably.");
		}
		else {
			easyForcePrintLineClient(pEntity, "***ERROR: test_cvar struct call did not work.");
		}
	}
	else if (FStrEq(pcmdRefinedRef, "tcs_get_direct")) {
		float tempVal = CVAR_GET_FLOAT("test_cvar");
		easyForcePrintLineClient(pEntity, "***Value: %.2g", tempVal);
	}
	else if (FStrEq(pcmdRefinedRef, "tcs_get_struct")) {
		//cvar_t* tempRef = CVAR_GET_POINTER("test_cvar");
		cvar_t* tempRef = global_test_cvar_ref;
		if (tempRef != NULL) {
			char binaryBuffer[33];
			convertIntToBinary(binaryBuffer, tempRef->flags, 32);
			easyForcePrintLineClient(pEntity, "***Value: %.2g, flags: %s", tempRef->value, binaryBuffer);
		}
		else {
			easyForcePrintLineClient(pEntity, "***ERROR: test_cvar struct call did not work.");
		}
	}
	else if (FStrEq(pcmdRefinedRef, "test1")) {
		float theRes = EASY_CVAR_GET(pregame_server_cvar);
		easyForcePrintLine("pregame_server_cvar is %.2f", theRes);
	}
	else if (FStrEq(pcmdRefinedRef, "test2")) {
		EASY_CVAR_SET(pregame_server_cvar, 24);
		easyForcePrintLine("pregame_server_cvar set?");
	}else if( FStrEq(pcmdRefinedRef, "garbage")){

		// 139?
		edict_t* pEdi = g_engfuncs.pfnPEntityOfEntIndex(79);

		if(pEdi != NULL && !pEdi->free){
			// ok
			CBaseEntity* ddd = CBaseEntity::Instance(pEdi);
			BOOL hasMonsterClip = ((ddd->pev->flags & FL_MONSTERCLIP) != 0);

			int x = 45;
		}

		// Because 'spawn' and 'respawn' are already engine commands?
	}

	// new stuff?









	
	else {
		caughtByFirst = FALSE;
	}


	if (caughtByFirst) {
		return;   //skip the hidden CVar stuff below, no need for extra interpretation.
	}



	
	//EASY_CVAR_HIDDEN_LIST

#ifndef _DEBUG
	for(int i = 0; i < HIDDEN_CVAR_INFO_LENGTH; i++){

		//easyForcePrintLine("What's my i? %d", i);

		// no need for this check, but beware of it a cause of crashing.
		// With proper setup, no need to check every time.
		//if(aryHiddenCVarInfo[i].nameLowercase == 0){
		//
		//}

		if( FStrEq(pcmdRefinedRef, aryHiddenCVarInfo[i].nameLowercase) ){
			int theType = aryHiddenCVarInfo[i].type;
			int argCount = CMD_ARGC();
			if(argCount > 2){easyForcePrintLineClient(pEntity, "WARNING: only one term after a CVar allowed, further terms ignored.");}
			if(argCount == 1){
				// only one?  Print out its value
				if(theType == 0){
					//EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY
					easyForcePrintLineClient(pEntity, "\"%s\" is %g", aryHiddenCVarInfo[i].name, *aryHiddenCVarInfo[i].serverCacheVar);
				}else if(theType == 1){
					//EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY
					easyForcePrintLineClient(pEntity, "\"%s\" is %g", aryHiddenCVarInfo[i].name, *aryHiddenCVarInfo[i].serverCacheVar);
				}else if(theType == 2){
					//EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY
					EASY_CVAR_PRINT_CLIENTONLY(aryHiddenCVarInfo[i].dataID)
				}else{
					// ?????????
				}
			}else{
				// any higher than 1 (probably 2)?  Set the CVar's value to the 2nd one.
				const char* arg1ref = CMD_ARGV(1);
				float tempF = 0;

				if(theType == 0){
					//EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY
					try{
						tempF = tryStringToFloat(arg1ref);

						// CLONED HERE.  Can't fudge what's used for 'global_' in that.
						//EASY_CVAR_SET_DEBUGONLY(aryHiddenCVarInfo[i].name, tempF)
						if(IS_DEDICATED_SERVER()){
							CVAR_SET_FLOAT(aryHiddenCVarInfo[i].name, tempF);
						}else{
							*aryHiddenCVarInfo[i].serverCacheVar = tempF;
						}

						g_queueCVarHiddenSave = TRUE;
					}catch(int){
						easyForcePrintLineClient(pEntity, "ERROR: Bad input. No effect.");
					}
				}else if(theType == 1){
					//EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY
					try{
						tempF = tryStringToFloat(arg1ref);

						// CLONED HERE.  Can't fudge what's used for 'global_' in that.
						//EASY_CVAR_SET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(aryHiddenCVarInfo[i].name, tempF)
						if(IS_DEDICATED_SERVER()){
							CVAR_SET_FLOAT(aryHiddenCVarInfo[i].name, tempF);
						}
						*aryHiddenCVarInfo[i].serverCacheVar = tempF;

						// SAME
						//CALL_EASY_CVAR_SYNCH_SERVER_TO_CLIENT_BROADCAST_DEBUGONLY(aryHiddenCVarInfo[i].name)
						MESSAGE_BEGIN(MSG_ALL, gmsgUpdateClientCVarNoSave, NULL);
							WRITE_SHORT(aryHiddenCVarInfo[i].dataID); 
							WRITE_SHORT(*aryHiddenCVarInfo[i].serverCacheVar * 100);
						MESSAGE_END();

						g_queueCVarHiddenSave = TRUE;
					}catch(int){
						easyForcePrintLineClient(pEntity, "ERROR: Bad input. No effect.");
					}
				}else if(theType == 2){
					//EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY
					try{
						tempF = tryStringToFloat(arg1ref);
						CUSTOM_CLIENT_CALL(aryHiddenCVarInfo[i].dataID, tempF)
					}catch(int){
						easyForcePrintLineClient(pEntity, "ERROR: Bad input. No effect.");
					}

					// Nothing to save here, server can't store anything about a CLIENTONLY var.
				}else{
					// ?????????
				}
			}//argCount check

			// stop going through the loop.  Or this whole message-parse method, really
			//break;
			return;
		}//typed in match check
	}//loop through hidden CVar info

#endif //!_DEBUG






	//NOTICE - so much as reaching here says that the hidden check didn't find anything. If it did, it would've
	//terminated this method early and never reached here.

	// tell the user they entered an unknown command
	char command[128];

	// check the length of the command (prevents crash)
	// max total length is 192 ...and we're adding a string below ("Unknown command: %s\n")
	strncpy( command, pcmd, 127 );
	command[127] = '\0';

	// tell the user they entered an unknown command
	ClientPrint( &pEntity->v, HUD_PRINTCONSOLE, UTIL_VarArgs( "Unknown command: %s\n", command ) );


	//Proven true by finding the player-entered string to be a reference to a hidden CVar to stop further checks.
	//BOOL hiddenCVarHandled = FALSE;


	/*
	if(FALSE){
	}
	//MODDD - insertion
	EASY_CVAR_HIDDEN_LIST
	//not here. we may still have another possible link below.


	if(hiddenCVarHandled)return;  //stop, we're done.
	
	if(FALSE){
	}
	EASY_CVAR_HIDDEN_LIST_2
	else{
		// tell the user they entered an unknown command
		char command[128];

		// check the length of the command (prevents crash)
		// max total length is 192 ...and we're adding a string below ("Unknown command: %s\n")
		strncpy( command, pcmd, 127 );
		command[127] = '\0';

		// tell the user they entered an unknown command
		ClientPrint( &pEntity->v, HUD_PRINTCONSOLE, UTIL_VarArgs( "Unknown command: %s\n", command ) );
	}
	*/


}//END OF... something big.
//!!!END OF THIS SILLY SILLY METHOD!!!





/*
========================
ClientUserInfoChanged

called after the player changes
userinfo - gives dll a chance to modify it before
it gets sent into the rest of the engine.
========================
*/
void ClientUserInfoChanged( edict_t *pEntity, char *infobuffer )
{
	// Is the client spawned yet?
	//MODDD - null check on pEntity too
	if (!pEntity || !pEntity->pvPrivateData )
		return;

	// msg everyone if someone changes their name,  and it isn't the first time (changing no name to current name)
	if ( pEntity->v.netname && STRING(pEntity->v.netname)[0] != 0 && !FStrEq( STRING(pEntity->v.netname), g_engfuncs.pfnInfoKeyValue( infobuffer, "name" )) )
	{
		char sName[256];
		char *pName = g_engfuncs.pfnInfoKeyValue( infobuffer, "name" );
		strncpy( sName, pName, sizeof(sName) - 1 );
		sName[ sizeof(sName) - 1 ] = '\0';

		// First parse the name and remove any %'s
		for ( char *pApersand = sName; pApersand != NULL && *pApersand != 0; pApersand++ )
		{
			// Replace it with a space
			if ( *pApersand == '%' )
				*pApersand = ' ';
		}

		// Set the name
		g_engfuncs.pfnSetClientKeyValue( ENTINDEX(pEntity), infobuffer, "name", sName );

		char text[256];
		sprintf( text, "* %s changed name to %s\n", STRING(pEntity->v.netname), g_engfuncs.pfnInfoKeyValue( infobuffer, "name" ) );

		//MODDD - put it in console too, laziness.
		easyForcePrintLine("ClientUserInfoChanged, place 1: %s", text);

		MESSAGE_BEGIN( MSG_ALL, gmsgSayText, NULL );
			WRITE_BYTE( ENTINDEX(pEntity) );
			WRITE_STRING( text );
		MESSAGE_END();

		// team match?
		if ( g_teamplay )
		{
			//MODDD - put it in console too, I have no idea where UTIL_LogPrintf is putting this.
			easyForcePrintLine("ClientUserInfoChanged, place 2: \"%s<%i><%s><%s>\" changed name to \"%s\"",
				STRING(pEntity->v.netname),
				GETPLAYERUSERID(pEntity),
				GETPLAYERAUTHID(pEntity),
				g_engfuncs.pfnInfoKeyValue(infobuffer, "model"),
				g_engfuncs.pfnInfoKeyValue(infobuffer, "name"));

			UTIL_LogPrintf( "\"%s<%i><%s><%s>\" changed name to \"%s\"\n", 
				STRING( pEntity->v.netname ), 
				GETPLAYERUSERID( pEntity ), 
				GETPLAYERAUTHID( pEntity ),
				g_engfuncs.pfnInfoKeyValue( infobuffer, "model" ), 
				g_engfuncs.pfnInfoKeyValue( infobuffer, "name" ) );
		}
		else
		{
			//MODDD - put it in console too, I have no idea where UTIL_LogPrintf is putting this.
			easyForcePrintLine("ClientUserInfoChanged, place 3: \"%s<%i><%s><%i>\" changed name to \"%s\"",
				STRING(pEntity->v.netname),
				GETPLAYERUSERID(pEntity),
				GETPLAYERAUTHID(pEntity),
				GETPLAYERUSERID(pEntity),
				g_engfuncs.pfnInfoKeyValue(infobuffer, "name"));

			UTIL_LogPrintf( "\"%s<%i><%s><%i>\" changed name to \"%s\"\n", 
				STRING( pEntity->v.netname ), 
				GETPLAYERUSERID( pEntity ), 
				GETPLAYERAUTHID( pEntity ),
				GETPLAYERUSERID( pEntity ), 
				g_engfuncs.pfnInfoKeyValue( infobuffer, "name" ) );
		}
	}

	//MODDD - NULL-CHECK PARANOIA.  Just being safe.
	if (g_pGameRules != NULL) {
		g_pGameRules->ClientUserInfoChanged(GetClassPtr((CBasePlayer*)&pEntity->v), infobuffer);
	}
}


void ServerDeactivate( void )
{
	// It's possible that the engine will call this function more times than is necessary
	//  Therefore, only run it one time for each call to ServerActivate 
	if ( g_serveractive != 1 )
	{
		return;
	}

	g_serveractive = 0;

	// Peform any shutdown operations here...
	//
}


void ServerActivate( edict_t *pEdictList, int edictCount, int clientMax )
{
	int i;
	CBaseEntity* pClass;

	globalPSEUDO_queueClientSendoff = TRUE;

	//MODDD
	//updateCVarRefs(NULL);
	//UPDATE: "GameDLLInit" method of game.cpp deemed the earliest point best for initial CVar update.

	// Every call to ServerActivate should be matched by a call to ServerDeactivate
	g_serveractive = 1;

	// Clients have not been initialized yet
	for ( i = 0; i < edictCount; i++ )
	{
		if ( pEdictList[i].free )
			continue;
		
		// Clients aren't necessarily initialized until ClientPutInServer()
		if ( i < clientMax || !pEdictList[i].pvPrivateData )
			continue;

		pClass = CBaseEntity::Instance( &pEdictList[i] );
		// Activate this entity if it's got a class & isn't dormant
		if ( pClass && !(pClass->pev->flags & FL_DORMANT) )
		{
			pClass->Activate();
		}
		else
		{
			ALERT( at_console, "Can't instance %s\n", STRING(pEdictList[i].v.classname) );
		}
	}

	// Link user messages here to make sure first client can get them...
	LinkUserMessages();

	//guaranteed to happen well after the last precache... or activate call reallly.
	OnMapLoadEnd();

	//..When does the game load gpGlobals->time from a save file exactly, from what it was at the time of save? No clue.
}//END OF ServerActivate



/*
================
PlayerPreThink

Called every frame before physics are run
================
*/
//Looks like "StartFrame", "PlayerPreThink", and "PlayerPostThink" are always called in the same order.
void PlayerPreThink( edict_t *pEntity )
{
	//typical non-paused delta:
	//d:0.01669312

	//MODDD NOTE:
	// HOO BOY is this hacky as Hell. But it works. Tell if the game is paused or not.
	// For reasons that remain unexplained by modern Science, the current time and previousFrameTime will vary by a very small
	// amount even while paused and nothing else happens. Some tiny ammount gets added / subtracted. No clue.
	// CHANGE, let StartFrame tell whether the game is paused instead maybe?    Nevermind, it isn't called when paused,
	// but PlayerPreThink still is.
	
	CBasePlayer* pPlayer = (CBasePlayer *)GET_PRIVATE(pEntity);

	if(pPlayer != NULL){
		//entvars_t* pev = &pEntity->v;

		//easyForcePrintLineClient(pEntity, "??A %d", playerCanThink1);

		if (!IsMultiplayer()) {
			BOOL stopMethod = FALSE;
			// NOTICE!  Counting these in PlayerPreThink is only useful when not in multiplayer.
			// Otherwise every player is calling this PlayerPreThink, not wise to rely on it from this method.
			float pausecorrection_val = EASY_CVAR_GET(pausecorrection2);

			// And no, this isn't blocking frames with a tiny amount of change, even completely paused there is a very tiny difference
			// at all times.  Weird.
			g_gamePaused = ((gpGlobals->time - sp_previousFrameTime) <= 0.005);

			//easyForcePrintLineClient(pEntity, "WHAT THE heck %.8f %.8f d:%.8f %d", gpGlobals->time, previousFrameTime, (gpGlobals->time - previousFrameTime), g_gamePaused);
			sp_previousFrameTime = gpGlobals->time;

			if (pausecorrection_val == 1 ) {
				// SERVER PAUSE CORRECTION FIX #1.  Block think logic if there isn't enough time since the previous frame.
				if (g_gamePaused) {
					sp_playerCanPreThink = FALSE;
					sp_playerCanPostThink = FALSE;
				}
				else {
					sp_playerCanPreThink = TRUE;
					sp_playerCanPostThink = TRUE;
				}
			}
			else if (pausecorrection_val != 2) {
				// 0? always allow
				sp_playerCanPreThink = TRUE;
				sp_playerCanPostThink = TRUE;
			}


			if (!sp_playerCanPreThink) {
				// If somewhere wants to know.
				g_gamePaused = TRUE;
				// This client message call has been cut. Serverside can issue the unpause order.
				//submitUnpauseRequest(&pEntity->v);
				if (EASY_CVAR_GET_DEBUGONLY(disablePauseSinglePlayer) == 1) {
					SERVER_COMMAND("unpause\n");
				}
				//easyForcePrintLineClient(pEntity, "!!! UNPAUSE REQUEST");
				//return;
				stopMethod = TRUE;
			}
			// must be turned back on by StartFrame.  Again, ignored in multiplayer.
		
			if (pausecorrection_val == 2 ) {
				// SERVER PAUSE CORRECTION FIX #2.  Block think logic next time if StartFrame hasn't run to re-allow another think frame.
				sp_playerCanPreThink = FALSE;
				//sp_playerCanPostThink = FALSE;
			}

			if (stopMethod) {
				// a late return.
				return;
			}
		}//END OF multiplayer check
		
		// to reach here, not paused?
		g_gamePaused = FALSE;

		//if (pPlayer)  //already checked above
			pPlayer->PreThink( );

	}// player null check

	
}//PlayerPreThink


/*
================
PlayerPostThink

Called every frame after physics are run
================
*/
void PlayerPostThink( edict_t *pEntity )
{
	//easyForcePrintLineClient(pEntity, "??B %d", playerCanThink2);
	
	CBasePlayer* pPlayer = (CBasePlayer *)GET_PRIVATE(pEntity);
	
	// Don't assume pPlayer isn't NULL.   Really.   Don't.
	if(pPlayer != NULL){
		entvars_t *pev = &pEntity->v;


		if (!IsMultiplayer()) {
			if (!sp_playerCanPostThink) {
				// Going to skip PlayerThink?  Process impulse commands anyway like
				// PlayerThink would have

				// Can all impulse commands be checked (flashlight, etc.)?
				if(!g_fGameOver && pPlayer->IsAlive()){
					pPlayer->ImpulseCommands();
				}else{
					// Something forbidding the normal way?  That's fine, skip to
					// checking for cheat ones.
					pPlayer->CheatImpulseCommands(pPlayer->pev->impulse);
					pPlayer->pev->impulse = 0;
				}

				return;
			}
			float pausecorrection_val = EASY_CVAR_GET(pausecorrection2);
			// must be turned back on by StartFrame.
			// Nevermind, the pre-think check is good enough.  I think.
			// NNNNNNNNN-no.   Nope, that's a horrible idea.

			if (pausecorrection_val == 2) {
				// SERVER PAUSE CORRECTION FIX #2.  Block think logic next time if StartFrame hasn't run to re-allow another think frame.
				//sp_playerCanPreThink = FALSE;
				sp_playerCanPostThink = FALSE;
			}
		}//not multiplayer check

	

		if(queueYMG_stopSend == TRUE){
			queueYMG_stopSend = FALSE;
			message_ymg_stop(ENT(pev));
		}


		if(crashableEntityReferTime > 0 && gpGlobals->time >= crashableEntityReferTime){
			switch(crashableEntityReferMode){
			case 5:{
				//dereferencing a NULL entity.
				const CBaseEntity& entRef = (*crashableEntityRef);
			break;}
			case 6:{
				//accessing methods of a NULL entity.
				crashableEntityRef->Spawn();
			break;}
			case 7:{
				//dereferencing a deleted entity.
				const CBaseEntity& entRef = (*crashableEntityRef);
			break;}
			case 8:{
				//accessing methods of a deleted entity.
				crashableEntityRef->Spawn();
			break;}

			}//END Of switch

			crashableEntityReferTime = -1;  //.... what?
		}//END OF crashable check


		//if (pPlayer)   //already checked now
		pPlayer->PostThink( );


	// ................................................. what.
	#if defined( CLIENT_WEAPONS )

		//iterate through the player's weapons.
		//for ( int i = 0 ; i < MAX_ITEM_TYPES ; i++ ){
		//	if ( pPlayer->m_rgpPlayerItems[ i ] ){
				//
				//CBasePlayerItem *pPlayerItem = pPlayer->m_rgpPlayerItems[ i ];

				//while ( pPlayerItem ){
				//	CBasePlayerWeapon *gun;
				//	gun = (CBasePlayerWeapon *)pPlayerItem->GetWeaponPtr();

				//	//Essentially, if this "item" exists and is a "weapon":
				//	if(gun){
			
				//		/*
				//		if(clientCheat_infiniteclip->value == 1){
				//			//all weapons must have at least "1" in the clip for convenience.
				//			if(gun->m_iClip <= 0){
				//				gun->m_iClip = 1;
				//			}
				//			//Secondary tends not to have clips, just assume they're meant too then.
				//			if(gun->m_iSecondaryAmmoType > 0 && pPlayer->m_rgAmmo[gun->m_iSecondaryAmmoType] == 0){
				//				pPlayer->m_rgAmmo[gun->m_iSecondaryAmmoType] = 1;
				//			}
				//		}
				//		*/
				//		/*
				//		if(clientCheat_infiniteclip->value == 1){
				//			//all weapons must have at least "1" in the total ammo-count for convenience.
				//			if(gun->m_iPrimaryAmmoType > 0 && pPlayer->m_rgAmmo[gun->m_iPrimaryAmmoType] == 0){
				//				pPlayer->m_rgAmmo[gun->m_iPrimaryAmmoType] = 1;
				//			}
				//			//Secondary too, if available.
				//			if(gun->m_iSecondaryAmmoType > 0 && pPlayer->m_rgAmmo[gun->m_iSecondaryAmmoType] == 0){
				//				pPlayer->m_rgAmmo[gun->m_iSecondaryAmmoType] = 1;
				//			}
				//		}*/
				//				

				//	}//END OF if(gun)
				//}//END OF while (this particular chained item exists)
				//pPlayerItem = pPlayerItem->m_pNext;
		//	}//END OF if( this player's particular item exists)
		//}//END OF for each item in the player's inventory

	#endif //CLIENT_WEAPONS


	}//player null check


}//END OF PlayerPostThink




// Completely unused (not just empty, never even called in the first place). Tried going to a new map, transition, loading a game... this never fires.
void ParmsNewLevel( void )
{

}


// This gets called while changing levels at transitions or loading a saved game, but not on starting a map by console's "map x" or "changelevel x".
// During a level transition, this gets called once before "OnMapLoadStart", and then twice after. Why?
// ~"OnBeforeChangeLevelTransition" is still called before either this or OnMapLoadStart.
// While loading a save game, this gets called after "OnMapLoadStart".
void ParmsChangeLevel( void )
{
	// retrieve the pointer to the save data
	SAVERESTOREDATA *pSaveData = (SAVERESTOREDATA *)gpGlobals->pSaveData;

	if ( pSaveData )
		pSaveData->connectionCount = BuildChangeList( pSaveData->levelList, MAX_LEVEL_CONNECTIONS );
}


//
// GLOBALS ASSUMED SET:  g_ulFrameCount
//


void StartFrame( void )
{
	// Keep g_flWeaponCheat in synch with sv_cheats.
	if(CVAR_GET_FLOAT("sv_cheats") == 1){
		g_flWeaponCheat = 1;
	}else{
		g_flWeaponCheat = 0;
	}
	

	//if(cvar_skill == NULL){
	//	cvar_skill = CVAR_GET_POINTER("skill");
	//}

	//if(cvar_skill != NULL && cvar_skill->value != cvar_skill_mem){
	if(CVAR_GET_FLOAT("skill") != cvar_skill_mem){
		// update as soon as we can.
		queueSkillUpdate = TRUE;
	}

	if(queueSkillUpdate == TRUE && g_pGameRules != NULL){
		g_pGameRules->RefreshSkillData();
	}





	//NOTE - this does not automatically play sounds though the soundSentenceSave system.
	//       Anything not starting with an exclamation mark or from "sentencetest", as opposed to "soundtest",
	//       is played raw as any other typical sound and needs to be precached first to be played.
	//OTHER NOTE - NEW!
	//       We're not playing just the soonest player that calls "PostThink", like before.
	//       We record what player called for playing a sound (playedSoundPlayer) and use that one.
	if (playQueued && playedSoundPlayer != NULL && playQueuedTime <= gpGlobals->time) {
		edict_t* tempEd = ENT(playedSoundPlayer->pev);


		//EMIT_SOUND_DYN( edict(), CHAN_VOICE, pszSentence, volume, attenuation, 0, GetVoicePitch());
		//easyForcePrintLineClient(pEntity, "??___?? %s", queuedSound);
		//or use temped there???
		//no, wait to start playing soon.

		//...why the HELL Does what channel is picked have a difference on how sounds work?  This is just weird.
		//-was CHAN_STREAM before.
		EMIT_SOUND_DYN(playedSoundPlayer->edict(), CHAN_VOICE, queuedSound, 1, ATTN_NORM, 0, playQueuedPitch);

		//called already.
		playQueued = FALSE;
	}

	
	
	//easyForcePrintLine("!!");
	// let the player think since there is a frame of logic this time around.
	// multiplayer ignores these.
	// Should 'pausecorrection2' choice influence whether these are reset to TRUE here or not?  Hard to say.

	float pausecorrection_val = EASY_CVAR_GET(pausecorrection2);

	if (pausecorrection_val == 2) {
		// StartFrame running resets them then.
		sp_playerCanPreThink = TRUE;
		sp_playerCanPostThink = TRUE;
	}




	// And sending gmsgUpdateClientCVar messages in the first frame causes "PF_MessageEnd_I:  Unknown User Msg 119" messages
	// beeeeecccccccccaaaaaaaaauuuuuuuusssssssseeeeeee?
	// as in, yes, even the first time "StartFrame" here is called, which runs every frame.  SPOOKY.
	// Skipping the first two frames just to be safe.


	// does the 'g_ulFrameCount > 2' requirement even make sense?
	// Maybe do neither of these in such a case it fails?  Maybe not, don't know.
	


	// NOTE!  Beware of loaded games!  They set gpGlobals->time!   I don't know if in the codebase or if that's up to the engine though.
	// Somewhere a "g_previousFrameTime = gpGlobals->time" right after a restore would be smart, if suddenly seeing "gpGlobals->time" at 800
	// compared to a prevTime of 0 is an issue.
	// Same for sp_previousFrameTime, maybe?

	BOOL tempPauseCheck = ((gpGlobals->time - g_previousFrameTime) <= 0.005);
	//gEngfuncs.GetMaxClients(
	g_previousFrameTime = gpGlobals->time;




	//if(g_ulFrameCount <= 20 || !g_mapLoadedEver || !g_alreadyShownGameloadedMessage) {
	//	easyPrintLine("??? mapLE:%d LoadedMess:%d playerCon:%d time:%.2f prevtime:%.2f : ispaus?%d   frame:%lu playercount:%d", g_mapLoadedEver, g_alreadyShownGameloadedMessage, g_firstPlayerEntered, gpGlobals->time, g_previousFrameTime, tempPauseCheck, g_ulFrameCount, gpGlobals->maxClients);
	//}
	//

	// g_ulFrameCount >= 1 && ?
	// g_mapLoadedEver ?
	
	// The 'at least 1st frame' requirement stops "PF_MessageEnd_I:  Unknown User Msg 119" messages on restoring games.
	// 'g_firstPlayerEntered"' is better than a flat frame check because time to load a brand new map can go between 1
	// and 20 frames (pre-steam HL is done on the 2nd, WON takes many more frames).
	// In any case, waiting until the first player has called 'spawn' looks to work.
	// TEST: are broadcast messages for broadcasted/synch'd/replicated (synonyms) Cvars valid in a dedicated server
	// with 0 players?  Just checking for the cause of some error messages like this 119 one, either being called too early
	// or having no clients to send a message to yet (I doubt the latter).
	// Starting a non-dedicated server (automatically join it after creation) takes about 30 frames in pre-steam.
	// In WON, about 40 frames.

	if(g_ulFrameCount >= 1 && g_firstPlayerEntered){
		if (!g_alreadyShownGameloadedMessage) {
			g_alreadyShownGameloadedMessage = TRUE;
			easyPrintLine("!!!Called for CVar updates with a map loaded for the first time, frame:%lu", g_ulFrameCount);
		}
		updateCVarRefs(FALSE);


		if (gpGlobals->time >= DebugLine_drawTime) {
			//::debugLine_setup(1, 
			//::debugLine_setupFract(0, 118, 759, 38, 118, 759, 600, fabs(sin(gpGlobals->time*0.8)) );
			DebugLine_RenderAll();
			//::UTIL_drawLineFrame(118.48, 759.76, 37.03, 500, 500, 500, 12, 255, 0, 0);
			DebugLine_drawTime = gpGlobals->time + 0.09;

			// draws the old debug stuff the old way for compatability.  Not from retail.
			// See lower portions of util_debugdraw.cpp
			drawOldDebugStuff();
		}//DebugLine_drawTime check


	}
	else {
		// This printout has no point if this is a dedicated server.  There will likely be good
		// periods of time before the first player ever joins.
		if(!IS_DEDICATED_SERVER()){
			easyPrintLine("!!!Called for CVar updates without a player present yet, frame: %lu mle:%d fpe:%d", g_ulFrameCount, g_mapLoadedEver, g_firstPlayerEntered);
		}
		
		updateCVarRefs(TRUE);
	}


	if ( g_pGameRules )
		g_pGameRules->Think();


	// NOTICE - StartFrame can't detect whether the game is paused in real time.
	// It is not called when the game is paused (console or menu up in singleplayer).
	// Not worth checking here.


	/*
	// EARLY TEST.
	// Causes a crash in release mode on creating a multiplayer server.
	// Errrrr.   okay?  Wait until the map has loaded and the first client connected and this is ok.
	// !!! Forgot about a '!pEntity->pvPrivateData' check, can that be involed??
	edict_t* thePlayerEd = g_engfuncs.pfnPEntityOfEntIndex(1);
	CBasePlayer* tempplayer;
	if (thePlayerEd != NULL) {
		if (FNullEnt(thePlayerEd) || thePlayerEd->free) {
			easyForcePrintLine("oh dear too early of access %d %d", FNullEnt(thePlayerEd), thePlayerEd->free);
		}
		else {
			tempplayer = GetClassPtr((CBasePlayer*)&thePlayerEd->v);
		}
	}
	else {
		// ??????
		tempplayer = NULL;
	}
	*/





	/*
	//MODDD - TEST.  Use some CVar to change my blood.  See what looks good.
	static float nextBloodGen = 0;

	if (gpGlobals->time > nextBloodGen) {
		// BLOOD TEST.
		Vector bloodSpawnOrigin = Vector(105.33, 598.49, 80);
		UTIL_SpawnBlood(bloodSpawnOrigin, (int)EASY_CVAR_GET_DEBUGONLY(hyperBarney), 1);
		nextBloodGen = gpGlobals->time + 0.08;
	}
	*/













	if (changeLevelQueuedTime != -1 && gpGlobals->time >= changeLevelQueuedTime) {
		// JUST DO IT
		changeLevelQueuedTime = -1;


		if ( !(changeLevelQueuedSpot == NULL || changeLevelQueuedSpot[0] == '\0') ) {
			char peepeebuffer[128];
			sprintf(&peepeebuffer[0], "changelevel2 %s %s\n", changeLevelQueuedName, changeLevelQueuedSpot);
			SERVER_COMMAND(peepeebuffer);
		}
		else {
			easyForcePrintLine("***Cant work with a NULL spot!  Try again***");
		}


		//CHANGE_LEVEL(changeLevelQueuedName, changeLevelQueuedSpot);
	}


	if ( g_fGameOver )
		return;

	gpGlobals->teamplay = teamplay.value;
	g_ulFrameCount++;
}




/*
===============
GetGameDescription

Returns the descriptive name of this .dll.  E.g., Half-Life, or Team Fortress 2
===============
*/
// MODDD NOTE - hold up what's with the above description involving "Team Fotress 2"?
// It ain't 2007, it's 1998 Valve.  The fuck you guys smokin'
const char *GetGameDescription()
{
	if ( g_pGameRules ) // this function may be called before the world has spawned, and the game rules initialized
		return g_pGameRules->GetGameDescription();
	else
		return GAME_NORMAL_DESCRIPTION;
}

/*
================
Sys_Error

Engine is going to shut down, allows setting a breakpoint in game .dll to catch that occasion
================
*/
void Sys_Error( const char *error_string )
{
	// Default case, do nothing.  MOD AUTHORS:  Add code ( e.g., _asm { int 3 }; here to cause a breakpoint for debugging your game .dlls

	//MODDD
	// Oh, lookie there.  An error string.     Definitely not just now noticing this.    Nope.
	easyForcePrintLine("Sys_Error: %s", error_string);
	// This is ok?
	_asm { int 3 };

}

/*
================
PlayerCustomization

A new player customization has been registered on the server
UNDONE:  This only sets the # of frames of the spray can logo
animation right now.
================
*/
void PlayerCustomization( edict_t *pEntity, customization_t *pCust )
{
	entvars_t *pev = &pEntity->v;
	CBasePlayer *pPlayer = (CBasePlayer *)GET_PRIVATE(pEntity);

	if (!pPlayer)
	{
		ALERT(at_console, "PlayerCustomization:  Couldn't get player!\n");
		return;
	}

	if (!pCust)
	{
		ALERT(at_console, "PlayerCustomization:  NULL customization!\n");
		return;
	}

	switch (pCust->resource.type)
	{
	case t_decal:
		pPlayer->SetCustomDecalFrames(pCust->nUserData2); // Second int is max # of frames.
		break;
	case t_sound:
	case t_skin:
	case t_model:
		// Ignore for now.
		break;
	default:
		ALERT(at_console, "PlayerCustomization:  Unknown customization type!\n");
		break;
	}
}

/*
================
SpectatorConnect

A spectator has joined the game
================
*/
void SpectatorConnect( edict_t *pEntity )
{
	entvars_t *pev = &pEntity->v;
	CBaseSpectator *pPlayer = (CBaseSpectator *)GET_PRIVATE(pEntity);

	if (pPlayer)
		pPlayer->SpectatorConnect( );
}

/*
================
SpectatorConnect

A spectator has left the game
================
*/
void SpectatorDisconnect( edict_t *pEntity )
{
	entvars_t *pev = &pEntity->v;
	CBaseSpectator *pPlayer = (CBaseSpectator *)GET_PRIVATE(pEntity);

	if (pPlayer)
		pPlayer->SpectatorDisconnect( );
}

/*
================
SpectatorConnect

A spectator has sent a usercmd
================
*/
void SpectatorThink( edict_t *pEntity )
{
	entvars_t *pev = &pEntity->v;
	CBaseSpectator *pPlayer = (CBaseSpectator *)GET_PRIVATE(pEntity);

	if (pPlayer)
		pPlayer->SpectatorThink( );
}

////////////////////////////////////////////////////////
// PAS and PVS routines for client messaging
//

/*
================
SetupVisibility

A client can have a separate "view entity" indicating that his/her view should depend on the origin of that
view entity.  If that's the case, then pViewEntity will be non-NULL and will be used.  Otherwise, the current
entity's origin is used.  Either is offset by the view_ofs to get the eye position.

From the eye position, we set up the PAS and PVS to use for filtering network messages to the client.  At this point, we could
 override the actual PAS or PVS values, or use a different origin.

NOTE:  Do not cache the values of pas and pvs, as they depend on reusable memory in the engine, they are only good for this one frame
================
*/
void SetupVisibility( edict_t *pViewEntity, edict_t *pClient, unsigned char **pvs, unsigned char **pas )
{
	Vector org;
	edict_t *pView = pClient;

	// Find the client's PVS
	if ( pViewEntity )
	{
		pView = pViewEntity;
	}

	if ( pClient->v.flags & FL_PROXY )
	{
		*pvs = NULL;	// the spectator proxy sees
		*pas = NULL;	// and hears everything
		return;
	}

	org = pView->v.origin + pView->v.view_ofs;
	if ( pView->v.flags & FL_DUCKING )
	{
		org = org + ( VEC_HULL_MIN - VEC_DUCK_HULL_MIN );
	}

	*pvs = ENGINE_SET_PVS ( (float *)&org );
	*pas = ENGINE_SET_PAS ( (float *)&org );
}

//old entity_state.h include location

/*
AddToFullPack

Return 1 if the entity state has been filled in for the ent and the entity will be propagated to the client, 0 otherwise

state is the server maintained copy of the state info that is transmitted to the client
a MOD could alter values copied into state to send the "host" a different look for a particular entity update, etc.
e and ent are the entity that is being added to the update, if 1 is returned
host is the player's edict of the player whom we are sending the update to
player is 1 if the ent/e is a player and 0 otherwise
pSet is either the PAS or PVS that we previous set up.  We can use it to ask the engine to filter the entity against the PAS or PVS.
we could also use the pas/ pvs that we set in SetupVisibility, if we wanted to.  Caching the value is valid in that case, but still only for the current frame
*/
int AddToFullPack( struct entity_state_s *state, int e, edict_t *ent, edict_t *host, int hostflags, int player, unsigned char *pSet )
{
	int i;

	// don't send if flagged for NODRAW and it's not the host getting the message
	if ( ( ent->v.effects == EF_NODRAW ) &&
		 ( ent != host ) )
		return 0;

	// Ignore ents without valid / visible models
	if ( !ent->v.modelindex || !STRING( ent->v.model ) )
		return 0;

	// Don't send spectators to other players
	if ( ( ent->v.flags & FL_SPECTATOR ) && ( ent != host ) )
	{
		return 0;
	}

	// Ignore if not the host and not touching a PVS/PAS leaf
	// If pSet is NULL, then the test will always succeed and the entity will be added to the update
	if ( ent != host )
	{
		if ( !ENGINE_CHECK_VISIBILITY( (const struct edict_s *)ent, pSet ) )
		{
			return 0;
		}
	}


	// Don't send entity to local client if the client says it's predicting the entity itself.
	if ( ent->v.flags & FL_SKIPLOCALHOST )
	{
		if ( ( hostflags & 1 ) && ( ent->v.owner == host ) )
			return 0;
	}
	
	if ( host->v.groupinfo )
	{
		UTIL_SetGroupTrace( host->v.groupinfo, GROUP_OP_AND );

		// Should always be set, of course
		if ( ent->v.groupinfo )
		{
			if ( g_groupop == GROUP_OP_AND )
			{
				if ( !(ent->v.groupinfo & host->v.groupinfo ) )
					return 0;
			}
			else if ( g_groupop == GROUP_OP_NAND )
			{
				if ( ent->v.groupinfo & host->v.groupinfo )
					return 0;
			}
		}

		UTIL_UnsetGroupTrace();
	}

	memset( state, 0, sizeof( *state ) );

	// Assign index so we can track this entity from frame to frame and
	//  delta from it.
	state->number	  = e;
	state->entityType = ENTITY_NORMAL;
	
	// Flag custom entities.
	if ( ent->v.flags & FL_CUSTOMENTITY )
	{
		state->entityType = ENTITY_BEAM;
	}

	// 
	// Copy state data
	//

	// Round animtime to nearest millisecond
	state->animtime   = (int)(1000.0 * ent->v.animtime ) / 1000.0;

	memcpy( state->origin, ent->v.origin, 3 * sizeof( float ) );
	memcpy( state->angles, ent->v.angles, 3 * sizeof( float ) );
	memcpy( state->mins, ent->v.mins, 3 * sizeof( float ) );
	memcpy( state->maxs, ent->v.maxs, 3 * sizeof( float ) );

	memcpy( state->startpos, ent->v.startpos, 3 * sizeof( float ) );
	memcpy( state->endpos, ent->v.endpos, 3 * sizeof( float ) );

	state->impacttime = ent->v.impacttime;
	state->starttime = ent->v.starttime;

	state->modelindex = ent->v.modelindex;
		
	state->frame      = ent->v.frame;

	state->skin       = ent->v.skin;
	state->effects    = ent->v.effects;

	// This non-player entity is being moved by the game .dll and not the physics simulation system
	//  make sure that we interpolate it's position on the client if it moves
	//MODDD - yet no mention of EFLAG_SLERP in cl_dlls/StudioModelRenderer.cpp?  strange
	if ( !player &&
		 ent->v.animtime &&
		 ent->v.velocity[ 0 ] == 0 && 
		 ent->v.velocity[ 1 ] == 0 && 
		 ent->v.velocity[ 2 ] == 0 )
	{
		state->eflags |= EFLAG_SLERP;
	}

	state->scale	  = ent->v.scale;
	state->solid	  = ent->v.solid;
	state->colormap   = ent->v.colormap;

	state->movetype   = ent->v.movetype;
	state->sequence   = ent->v.sequence;
	state->framerate  = ent->v.framerate;
	state->body       = ent->v.body;

	for (i = 0; i < 4; i++)
	{
		state->controller[i] = ent->v.controller[i];
	}

	for (i = 0; i < 2; i++)
	{
		state->blending[i]   = ent->v.blending[i];
	}

	state->rendermode    = ent->v.rendermode;
	state->renderamt     = ent->v.renderamt; 
	state->renderfx      = ent->v.renderfx;
	state->rendercolor.r = ent->v.rendercolor.x;
	state->rendercolor.g = ent->v.rendercolor.y;
	state->rendercolor.b = ent->v.rendercolor.z;

	state->aiment = 0;
	if ( ent->v.aiment )
	{
		state->aiment = ENTINDEX( ent->v.aiment );
	}

	state->owner = 0;
	if ( ent->v.owner )
	{
		int owner = ENTINDEX( ent->v.owner );
		
		// Only care if owned by a player
		if ( owner >= 1 && owner <= gpGlobals->maxClients )
		{
			state->owner = owner;	
		}
	}

	//MODDD - this... also appears to do effectively nothing.  Fantastic!
	/*
	state->iuser1 = ent->v.iuser1;
	state->iuser1 = ent->v.iuser1;
	state->iuser2 = ent->v.iuser2;
	state->iuser3 = ent->v.iuser3;
	state->fuser4 = ent->v.fuser4;
	state->fuser2 = ent->v.fuser2;
	state->fuser3 = ent->v.fuser3;
	state->fuser4 = ent->v.fuser4;
	state->vuser1 = ent->v.vuser1;
	state->vuser2 = ent->v.vuser2;
	state->vuser3 = ent->v.vuser3;
	state->vuser4 = ent->v.vuser4;
	*/



	// HACK:  Somewhat...
	// Class is overridden for non-players to signify a breakable glass object ( sort of a class? )
	if ( !player )
	{
		state->playerclass  = ent->v.playerclass;
	}


	/*
	state->playerclass = 2;
	state->usehull = 3;
	//state->
	state->iuser1 = 2;
	state->iuser1 = 2;
	state->iuser2 = 2;
	state->iuser3 = 2;
	state->fuser4 = 2;
	state->fuser2 = 2;
	state->fuser3 = 2;
	state->fuser4 = 2;
	state->vuser1.x = 2;
	state->vuser2.x = 2;
	state->vuser3.x = 2;
	state->vuser4.x = 2;
	*/


	// Special stuff for players only
	if ( player )
	{
		memcpy( state->basevelocity, ent->v.basevelocity, 3 * sizeof( float ) );

		state->weaponmodel  = MODEL_INDEX( STRING( ent->v.weaponmodel ) );
		state->gaitsequence = ent->v.gaitsequence;
		state->spectator = ent->v.flags & FL_SPECTATOR;
		state->friction     = ent->v.friction;

		state->gravity      = ent->v.gravity;
//		state->team			= ent->v.team;
//		
		state->usehull      = ( ent->v.flags & FL_DUCKING ) ? 1 : 0;



		//MODDD - INTERVENTION.
		if (ent->v.health > 0 && ent->v.health < 1) {
			// force it!  That is, round up if between 0 and 1
			state->health = 1;
		}
		else {
			// nothing special.
			state->health = ent->v.health;
		}
		
		//state->health = ent->v.health;


		
	}//END OF player check

	return 1;
}

//MODDD - DEFAULT_VIEWHEIGT define merged with that in util_shared.h.

/*
===================
CreateBaseline

Creates baselines used for network encoding, especially for player data since players are not spawned until connect time.
===================
*/
void CreateBaseline( int player, int eindex, struct entity_state_s *baseline, struct edict_s *entity, int playermodelindex, vec3_t player_mins, vec3_t player_maxs )
{
	baseline->origin		= entity->v.origin;
	baseline->angles		= entity->v.angles;
	baseline->frame			= entity->v.frame;
	baseline->skin			= (short)entity->v.skin;

	// render information
	baseline->rendermode	= (byte)entity->v.rendermode;
	baseline->renderamt		= (byte)entity->v.renderamt;
	baseline->rendercolor.r	= (byte)entity->v.rendercolor.x;
	baseline->rendercolor.g	= (byte)entity->v.rendercolor.y;
	baseline->rendercolor.b	= (byte)entity->v.rendercolor.z;
	baseline->renderfx		= (byte)entity->v.renderfx;

	if ( player )
	{
		baseline->mins			= player_mins;
		baseline->maxs			= player_maxs;

		baseline->colormap		= eindex;
		baseline->modelindex	= playermodelindex;
		baseline->friction		= 1.0;
		baseline->movetype		= MOVETYPE_WALK;

		baseline->scale			= entity->v.scale;
		baseline->solid			= SOLID_SLIDEBOX;
		baseline->framerate		= 1.0;
		baseline->gravity		= 1.0;

	}
	else
	{
		baseline->mins			= entity->v.mins;
		baseline->maxs			= entity->v.maxs;

		baseline->colormap		= 0;
		baseline->modelindex	= entity->v.modelindex;//SV_ModelIndex(pr_strings + entity->v.model);
		baseline->movetype		= entity->v.movetype;

		baseline->scale			= entity->v.scale;
		baseline->solid			= entity->v.solid;
		baseline->framerate		= entity->v.framerate;
		baseline->gravity		= entity->v.gravity;
	}
}

typedef struct
{
	char name[32];
	int  field;
} entity_field_alias_t;

#define FIELD_ORIGIN0			0
#define FIELD_ORIGIN1			1
#define FIELD_ORIGIN2			2
#define FIELD_ANGLES0			3
#define FIELD_ANGLES1			4
#define FIELD_ANGLES2			5

static entity_field_alias_t entity_field_alias[]=
{
	{ "origin[0]",			0 },
	{ "origin[1]",			0 },
	{ "origin[2]",			0 },
	{ "angles[0]",			0 },
	{ "angles[1]",			0 },
	{ "angles[2]",			0 },
};

void Entity_FieldInit( struct delta_s *pFields )
{
	entity_field_alias[ FIELD_ORIGIN0 ].field		= DELTA_FINDFIELD( pFields, entity_field_alias[ FIELD_ORIGIN0 ].name );
	entity_field_alias[ FIELD_ORIGIN1 ].field		= DELTA_FINDFIELD( pFields, entity_field_alias[ FIELD_ORIGIN1 ].name );
	entity_field_alias[ FIELD_ORIGIN2 ].field		= DELTA_FINDFIELD( pFields, entity_field_alias[ FIELD_ORIGIN2 ].name );
	entity_field_alias[ FIELD_ANGLES0 ].field		= DELTA_FINDFIELD( pFields, entity_field_alias[ FIELD_ANGLES0 ].name );
	entity_field_alias[ FIELD_ANGLES1 ].field		= DELTA_FINDFIELD( pFields, entity_field_alias[ FIELD_ANGLES1 ].name );
	entity_field_alias[ FIELD_ANGLES2 ].field		= DELTA_FINDFIELD( pFields, entity_field_alias[ FIELD_ANGLES2 ].name );
}

/*
==================
Entity_Encode

Callback for sending entity_state_t info over network. 
FIXME:  Move to script
==================
*/
void Entity_Encode( struct delta_s *pFields, const unsigned char *from, const unsigned char *to )
{
	entity_state_t *f, *t;
	int localplayer = 0;
	static int initialized = 0;

	if ( !initialized )
	{
		Entity_FieldInit( pFields );
		initialized = 1;
	}

	f = (entity_state_t *)from;
	t = (entity_state_t *)to;

	// Never send origin to local player, it's sent with more resolution in clientdata_t structure
	localplayer =  ( t->number - 1 ) == ENGINE_CURRENT_PLAYER();
	if ( localplayer )
	{
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN0 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN1 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN2 ].field );
	}

	if ( ( t->impacttime != 0 ) && ( t->starttime != 0 ) )
	{
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN0 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN1 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN2 ].field );

		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ANGLES0 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ANGLES1 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ANGLES2 ].field );
	}

	if ( ( t->movetype == MOVETYPE_FOLLOW ) &&
		 ( t->aiment != 0 ) )
	{
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN0 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN1 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN2 ].field );
	}
	else if ( t->aiment != f->aiment )
	{
		DELTA_SETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN0 ].field );
		DELTA_SETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN1 ].field );
		DELTA_SETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN2 ].field );
	}
}

static entity_field_alias_t player_field_alias[]=
{
	{ "origin[0]",			0 },
	{ "origin[1]",			0 },
	{ "origin[2]",			0 },
};

void Player_FieldInit( struct delta_s *pFields )
{
	player_field_alias[ FIELD_ORIGIN0 ].field		= DELTA_FINDFIELD( pFields, player_field_alias[ FIELD_ORIGIN0 ].name );
	player_field_alias[ FIELD_ORIGIN1 ].field		= DELTA_FINDFIELD( pFields, player_field_alias[ FIELD_ORIGIN1 ].name );
	player_field_alias[ FIELD_ORIGIN2 ].field		= DELTA_FINDFIELD( pFields, player_field_alias[ FIELD_ORIGIN2 ].name );
}

/*
==================
Player_Encode

Callback for sending entity_state_t for players info over network. 
==================
*/
void Player_Encode( struct delta_s *pFields, const unsigned char *from, const unsigned char *to )
{
	entity_state_t *f, *t;
	int localplayer = 0;
	static int initialized = 0;

	if ( !initialized )
	{
		Player_FieldInit( pFields );
		initialized = 1;
	}

	f = (entity_state_t *)from;
	t = (entity_state_t *)to;

	// Never send origin to local player, it's sent with more resolution in clientdata_t structure
	localplayer =  ( t->number - 1 ) == ENGINE_CURRENT_PLAYER();
	if ( localplayer )
	{
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN0 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN1 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN2 ].field );
	}

	if ( ( t->movetype == MOVETYPE_FOLLOW ) &&
		 ( t->aiment != 0 ) )
	{
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN0 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN1 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN2 ].field );
	}
	else if ( t->aiment != f->aiment )
	{
		DELTA_SETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN0 ].field );
		DELTA_SETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN1 ].field );
		DELTA_SETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN2 ].field );
	}
}

#define CUSTOMFIELD_ORIGIN0			0
#define CUSTOMFIELD_ORIGIN1			1
#define CUSTOMFIELD_ORIGIN2			2
#define CUSTOMFIELD_ANGLES0			3
#define CUSTOMFIELD_ANGLES1			4
#define CUSTOMFIELD_ANGLES2			5
#define CUSTOMFIELD_SKIN			6
#define CUSTOMFIELD_SEQUENCE		7
#define CUSTOMFIELD_ANIMTIME		8

entity_field_alias_t custom_entity_field_alias[]=
{
	{ "origin[0]",			0 },
	{ "origin[1]",			0 },
	{ "origin[2]",			0 },
	{ "angles[0]",			0 },
	{ "angles[1]",			0 },
	{ "angles[2]",			0 },
	{ "skin",				0 },
	{ "sequence",			0 },
	{ "animtime",			0 },
};

void Custom_Entity_FieldInit( struct delta_s *pFields )
{
	custom_entity_field_alias[ CUSTOMFIELD_ORIGIN0 ].field	= DELTA_FINDFIELD( pFields, custom_entity_field_alias[ CUSTOMFIELD_ORIGIN0 ].name );
	custom_entity_field_alias[ CUSTOMFIELD_ORIGIN1 ].field	= DELTA_FINDFIELD( pFields, custom_entity_field_alias[ CUSTOMFIELD_ORIGIN1 ].name );
	custom_entity_field_alias[ CUSTOMFIELD_ORIGIN2 ].field	= DELTA_FINDFIELD( pFields, custom_entity_field_alias[ CUSTOMFIELD_ORIGIN2 ].name );
	custom_entity_field_alias[ CUSTOMFIELD_ANGLES0 ].field	= DELTA_FINDFIELD( pFields, custom_entity_field_alias[ CUSTOMFIELD_ANGLES0 ].name );
	custom_entity_field_alias[ CUSTOMFIELD_ANGLES1 ].field	= DELTA_FINDFIELD( pFields, custom_entity_field_alias[ CUSTOMFIELD_ANGLES1 ].name );
	custom_entity_field_alias[ CUSTOMFIELD_ANGLES2 ].field	= DELTA_FINDFIELD( pFields, custom_entity_field_alias[ CUSTOMFIELD_ANGLES2 ].name );
	custom_entity_field_alias[ CUSTOMFIELD_SKIN ].field	= DELTA_FINDFIELD( pFields, custom_entity_field_alias[ CUSTOMFIELD_SKIN ].name );
	custom_entity_field_alias[ CUSTOMFIELD_SEQUENCE ].field= DELTA_FINDFIELD( pFields, custom_entity_field_alias[ CUSTOMFIELD_SEQUENCE ].name );
	custom_entity_field_alias[ CUSTOMFIELD_ANIMTIME ].field= DELTA_FINDFIELD( pFields, custom_entity_field_alias[ CUSTOMFIELD_ANIMTIME ].name );
}

/*
==================
Custom_Encode

Callback for sending entity_state_t info ( for custom entities ) over network. 
FIXME:  Move to script
==================
*/
void Custom_Encode( struct delta_s *pFields, const unsigned char *from, const unsigned char *to )
{
	entity_state_t *f, *t;
	int beamType;
	static int initialized = 0;

	if ( !initialized )
	{
		Custom_Entity_FieldInit( pFields );
		initialized = 1;
	}

	f = (entity_state_t *)from;
	t = (entity_state_t *)to;

	beamType = t->rendermode & 0x0f;
		
	if ( beamType != BEAM_POINTS && beamType != BEAM_ENTPOINT )
	{
		DELTA_UNSETBYINDEX( pFields, custom_entity_field_alias[ CUSTOMFIELD_ORIGIN0 ].field );
		DELTA_UNSETBYINDEX( pFields, custom_entity_field_alias[ CUSTOMFIELD_ORIGIN1 ].field );
		DELTA_UNSETBYINDEX( pFields, custom_entity_field_alias[ CUSTOMFIELD_ORIGIN2 ].field );
	}

	if ( beamType != BEAM_POINTS )
	{
		DELTA_UNSETBYINDEX( pFields, custom_entity_field_alias[ CUSTOMFIELD_ANGLES0 ].field );
		DELTA_UNSETBYINDEX( pFields, custom_entity_field_alias[ CUSTOMFIELD_ANGLES1 ].field );
		DELTA_UNSETBYINDEX( pFields, custom_entity_field_alias[ CUSTOMFIELD_ANGLES2 ].field );
	}

	if ( beamType != BEAM_ENTS && beamType != BEAM_ENTPOINT )
	{
		DELTA_UNSETBYINDEX( pFields, custom_entity_field_alias[ CUSTOMFIELD_SKIN ].field );
		DELTA_UNSETBYINDEX( pFields, custom_entity_field_alias[ CUSTOMFIELD_SEQUENCE ].field );
	}

	// animtime is compared by rounding first
	// see if we really shouldn't actually send it
	if ( (int)f->animtime == (int)t->animtime )
	{
		DELTA_UNSETBYINDEX( pFields, custom_entity_field_alias[ CUSTOMFIELD_ANIMTIME ].field );
	}
}

/*
=================
RegisterEncoders

Allows game .dll to override network encoding of certain types of entities and tweak values, etc.
=================
*/
void RegisterEncoders( void )
{
	DELTA_ADDENCODER( "Entity_Encode", Entity_Encode );
	DELTA_ADDENCODER( "Custom_Encode", Custom_Encode );
	DELTA_ADDENCODER( "Player_Encode", Player_Encode );
}

int GetWeaponData( struct edict_s *player, struct weapon_data_s *pInfo)
{
#if defined( CLIENT_WEAPONS )
	int i;
	weapon_data_t *item;
	entvars_t *pev = &player->v;
	CBasePlayer *pl = ( CBasePlayer *) CBasePlayer::Instance( pev );
	CBasePlayerWeapon *gun;
	
	ItemInfo II;

	memset(pInfo, 0, 32 * sizeof( weapon_data_t ) );

	if ( !pl )
		return 1;

	// go through all of the weapons and make a list of the ones to pack
	for ( i = 0 ; i < MAX_ITEM_TYPES ; i++ )
	{
		if ( pl->m_rgpPlayerItems[ i ] )
		{
			// there's a weapon here. Should I pack it?
			CBasePlayerItem *pPlayerItem = pl->m_rgpPlayerItems[ i ];

			while ( pPlayerItem )
			{
				gun = (CBasePlayerWeapon *)pPlayerItem->GetWeaponPtr();
				if ( gun && gun->UseDecrement() )
				{
					// Get The ID.
					memset( &II, 0, sizeof( II ) );
					gun->GetItemInfo( &II );


					//MODDD - EXPERIMENT!
					// Comment out the above two lines and try this.
					// If this works out fine, let every weapon have a 'GetItemInfoID'
					// to return only its ID in the ItemInfoArray for convenience.
					// Or... do it anyway because that's kinda neat.
					// Just seems like there's no need to write-out to some ItemInfo instance
					// every single frame when what's already in CBasePlayerItem::ItemInfoArray
					// should work just fine.
					// HOWEVER.  Keep in mind weaposn that may fudge with specifics from time to time.
					// Glock can modify its max clip allowed on a whim.
					// Perhaps each ItemInfo, or even a separate array (one index per weapon)
					// can be used to tell if an ItemInfo block is invalid, and, if so, re-get it
					// through the usual 'GetItemInfo' thing above.
					/*
					ItemInfo temper;
					memset( &temper, 0, sizeof(temper) );
					gun->GetItemInfo(&temper);

					II = CBasePlayerItem::ItemInfoArray[temper.iId];
					*/


					if ( II.iId >= 0 && II.iId < 32 )
					{
						item = &pInfo[ II.iId ];
					 	
						item->m_iId						= II.iId;
						item->m_iClip					= gun->m_iClip;

						item->m_flTimeWeaponIdle		= max( gun->m_flTimeWeaponIdle, -0.001 );
						item->m_flNextPrimaryAttack		= max( gun->m_flNextPrimaryAttack, -0.001 );
						item->m_flNextSecondaryAttack	= max( gun->m_flNextSecondaryAttack, -0.001 );
						item->m_fInReload				= gun->m_fInReload;
						item->m_fInSpecialReload		= gun->m_fInSpecialReload;
						item->fuser1					= max( gun->pev->fuser1, -0.001 );
						item->fuser2					= gun->m_flStartThrow;
						item->fuser3					= gun->m_flReleaseThrow;
						
						
						//WARNING - unreliable.  Do not use.
						//item->fuser4					= gun->fuser4;
						/*
						item->fuser5					= gun->fuser5;
						item->fuser6					= gun->fuser6;
						item->fuser7					= gun->fuser7;
						item->fuser8					= gun->fuser8;
						*/

						item->iuser1					= gun->m_chargeReady;
						item->iuser2					= gun->m_fInAttack;
						item->iuser3					= gun->m_fireState;
						
						
											
//						item->m_flPumpTime				= max( gun->m_flPumpTime, -0.001 );
					}
				}
				pPlayerItem = pPlayerItem->m_pNext;
			}
		}
	}
#else
	memset(pInfo, 0, 32 * sizeof( weapon_data_t ) );
#endif
	return 1;
}

/*
=================
UpdateClientData

Data sent to current client only
engine sets cd to 0 before calling.
=================
*/
void UpdateClientData ( const struct edict_s *ent, int sendweapons, struct clientdata_s *cd )
{
	cd->flags			= ent->v.flags;


	//MODDD - INTERVENTION.
	if (ent->v.health > 0 && ent->v.health < 1) {
		// force it!  That is, round up if between 0 and 1
		cd->health = 1;
	}
	else {
		// nothing special.
		cd->health = ent->v.health;
	}
	
	//cd->health = ent->v.health;




	cd->viewmodel		= MODEL_INDEX( STRING( ent->v.viewmodel ) );

	cd->waterlevel		= ent->v.waterlevel;
	cd->watertype		= ent->v.watertype;
	cd->weapons			= ent->v.weapons;

	// Vectors
	cd->origin			= ent->v.origin;
	
	//MODDD - INTERVENTION.  Don't let bob logic think we're moving if we're dead.
	if (ent->v.deadflag == DEAD_NO) {
		cd->velocity = ent->v.velocity;
	}
	else {
		cd->velocity = g_vecZero;
	}



	cd->view_ofs		= ent->v.view_ofs;
	cd->punchangle		= ent->v.punchangle;

	cd->bInDuck			= ent->v.bInDuck;
	cd->flTimeStepSound = ent->v.flTimeStepSound;
	cd->flDuckTime		= ent->v.flDuckTime;
	cd->flSwimTime		= ent->v.flSwimTime;
	cd->waterjumptime	= ent->v.teleport_time;

	strcpy( cd->physinfo, ENGINE_GETPHYSINFO( ent ) );

	cd->maxspeed		= ent->v.maxspeed;
	cd->fov				= ent->v.fov;
	
	
	//MODDD - great emphasis! Can we send a special flag for one frame?
	//Should we send the flag by a separate client message
	cd->weaponanim		= ent->v.weaponanim;

	cd->pushmsec		= ent->v.pushmsec;

#if defined( CLIENT_WEAPONS )
	if ( sendweapons )
	{
		entvars_t *pev = (entvars_t *)&ent->v;
		CBasePlayer *pl = ( CBasePlayer *) CBasePlayer::Instance( pev );

		if ( pl )
		{
			cd->m_flNextAttack	= pl->m_flNextAttack;
			cd->fuser2			= pl->m_flNextAmmoBurn;
			cd->fuser3			= pl->m_flAmmoStartCharge;
			cd->vuser1.x		= pl->ammo_9mm;
			cd->vuser1.y		= pl->ammo_357;
			cd->vuser1.z		= pl->ammo_argrens;
			cd->ammo_nails		= pl->ammo_bolts;
			cd->ammo_shells		= pl->ammo_buckshot;
			cd->ammo_rockets	= pl->ammo_rockets;
			cd->ammo_cells		= pl->ammo_uranium;
			cd->vuser2.x		= pl->ammo_hornets;
			

			if ( pl->m_pActiveItem )
			{
				CBasePlayerWeapon *gun;
				gun = (CBasePlayerWeapon *)pl->m_pActiveItem->GetWeaponPtr();
				if ( gun && gun->UseDecrement() )
				{
					ItemInfo II;
					memset( &II, 0, sizeof( II ) );
					gun->GetItemInfo( &II );

					cd->m_iId = II.iId;



					//MODDD - m_iPrimary/Secondary Ammo Types are cached now between client and serverside, don't bother sending.
					/*
					cd->vuser3.z	= gun->m_iSecondaryAmmoType;
					cd->vuser4.x	= gun->m_iPrimaryAmmoType;
					*/

					int myPrimaryAmmoType = gun->getPrimaryAmmoType();
					int mySecondaryAmmoType = gun->getSecondaryAmmoType();

					//MODDD - don't bother accessing the array at bad memory, even though the value will be properly ignored now
					// clientside.
					if (IS_AMMOTYPE_VALID(myPrimaryAmmoType)) {
						cd->vuser4.y = pl->m_rgAmmo[myPrimaryAmmoType];
					}
					else {
						// to be ignored but keep it clean
						cd->vuser4.y = 0;
					}
					//MODDD - same.
					if (IS_AMMOTYPE_VALID(mySecondaryAmmoType)) {
						cd->vuser4.z = pl->m_rgAmmo[mySecondaryAmmoType];
					}
					else {
						// to be ignored but keep it clean
						cd->vuser4.z = 0;
					}


					
					if ( pl->m_pActiveItem->m_iId == WEAPON_RPG )
					{
						cd->vuser2.y = ( ( CRpg * )pl->m_pActiveItem)->m_fSpotActive;
						cd->vuser2.z = ( ( CRpg * )pl->m_pActiveItem)->m_cActiveRockets;
					}
				}
			}
		}
	}
#endif
}

/*
=================
CmdStart

We're about to run this usercmd for the specified player.  We can set up groupinfo and masking here, etc.
This is the time to examine the usercmd for anything extra.  This call happens even if think does not.
=================
*/
void CmdStart( const edict_t *player, const struct usercmd_s *cmd, unsigned int random_seed )
{
	entvars_t *pev = (entvars_t *)&player->v;
	CBasePlayer *pl = ( CBasePlayer *) CBasePlayer::Instance( pev );

	if( !pl )
		return;

	if ( pl->pev->groupinfo != 0 )
	{
		UTIL_SetGroupTrace( pl->pev->groupinfo, GROUP_OP_AND );
	}

	pl->random_seed = random_seed;
}

/*
=================
CmdEnd

Each cmdstart is exactly matched with a cmd end, clean up any group trace flags, etc. here
=================
*/
void CmdEnd ( const edict_t *player )
{
	entvars_t *pev = (entvars_t *)&player->v;
	CBasePlayer *pl = ( CBasePlayer *) CBasePlayer::Instance( pev );

	if( !pl )
		return;
	if ( pl->pev->groupinfo != 0 )
	{
		UTIL_UnsetGroupTrace();
	}
}

/*
================================
ConnectionlessPacket

 Return 1 if the packet is valid.  Set response_buffer_size if you want to send a response packet.  Incoming, it holds the max
  size of the response_buffer, so you must zero it out if you choose not to respond.
================================
*/
int ConnectionlessPacket( const struct netadr_s *net_from, const char *args, char *response_buffer, int *response_buffer_size )
{
	// Parse stuff from args
	int max_buffer_size = *response_buffer_size;

	// Zero it out since we aren't going to respond.
	// If we wanted to response, we'd write data into response_buffer
	*response_buffer_size = 0;

	// Since we don't listen for anything here, just respond that it's a bogus message
	// If we didn't reject the message, we'd return 1 for success instead.
	return 0;
}

/*
================================
GetHullBounds

  Engine calls this to enumerate player collision hulls, for prediction.  Return 0 if the hullnumber doesn't exist.
================================
*/
int GetHullBounds( int hullnumber, float *mins, float *maxs )
{
	int iret = 0;

	switch ( hullnumber )
	{
	case 0:				// Normal player
		mins = VEC_HULL_MIN;
		maxs = VEC_HULL_MAX;
		iret = 1;
		break;
	case 1:				// Crouched player
		mins = VEC_DUCK_HULL_MIN;
		maxs = VEC_DUCK_HULL_MAX;
		iret = 1;
		break;
	case 2:				// Point based hull
		mins = Vector( 0, 0, 0 );
		maxs = Vector( 0, 0, 0 );
		iret = 1;
		break;
	}

	return iret;
}

/*
================================
CreateInstancedBaselines

Create pseudo-baselines for items that aren't placed in the map at spawn time, but which are likely
to be created during play ( e.g., grenades, ammo packs, projectiles, corpses, etc. )
================================
*/
void CreateInstancedBaselines ( void )
{
	int iret = 0;
	entity_state_t state;

	memset( &state, 0, sizeof( state ) );

	// Create any additional baselines here for things like grendates, etc.
	// iret = ENGINE_INSTANCE_BASELINE( pc->pev->classname, &state );

	// Destroy objects.
	//UTIL_Remove( pc );
}

/*
================================
InconsistentFile

One of the ENGINE_FORCE_UNMODIFIED files failed the consistency check for the specified player
 Return 0 to allow the client to continue, 1 to force immediate disconnection ( with an optional disconnect message of up to 256 characters )
================================
*/
int InconsistentFile( const edict_t *player, const char *filename, char *disconnect_message )
{
	// Server doesn't care?
	if ( CVAR_GET_FLOAT( "mp_consistency" ) != 1 )
		return 0;

	// Default behavior is to kick the player
	sprintf( disconnect_message, "Server is enforcing file consistency for %s\n", filename );

	// Kick now with specified disconnect message.
	return 1;
}

/*
================================
AllowLagCompensation

 The game .dll should return 1 if lag compensation should be allowed ( could also just set
  the sv_unlag cvar.
 Most games right now should return 0, until client-side weapon prediction code is written
  and tested for them ( note you can predict weapons, but not do lag compensation, too, 
  if you want.
================================
*/
int AllowLagCompensation( void )
{
	return 1;
}
