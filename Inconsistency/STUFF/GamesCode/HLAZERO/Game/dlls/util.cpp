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

===== util.cpp ========================================================

  Utility code.  Really not optional after all.

*/

// ?
//#include <time.h>

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "saverestore.h"
#include "shake.h"
#include "decals.h"
#include "player.h"
#include "weapons.h"
#include "gamerules.h"
#include "game.h"

#include "turret.h" //to call the turret class's static model references for a possible re-precache at a map/level change.
#include "barnacle.h" //to reset its static standard gib var.
#include "squidspit.h" //precache its model in a way that stores its index to a static integer. Like retail did.
#include "talkmonster.h"
#include "barney.h"

#include "trains.h"
#include "util_debugdraw.h"
#include "nodes.h"
#include "lights.h"
#include "scripted.h"

#include "cvar_custom_info.h"
#include "cvar_custom_list.h"


//EASY_CVAR_EXTERN(flashLightSpawnInterval)
EASY_CVAR_EXTERN(flashLightDurationMin)
EASY_CVAR_EXTERN(flashLightDurationMax)
EASY_CVAR_EXTERN(flashLightRadiusMin)
EASY_CVAR_EXTERN(flashLightRadiusMax)
EASY_CVAR_EXTERN(flashLightSpawnDistHori)
EASY_CVAR_EXTERN(flashLightSpawnDistVertMin)
EASY_CVAR_EXTERN(flashLightSpawnDistVertMax)
EASY_CVAR_EXTERN(flashLightMultiColor)

EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST(sv_germancensorship)

EASY_CVAR_EXTERN_DEBUGONLY(sparksEnvMulti)
EASY_CVAR_EXTERN_DEBUGONLY(shrapRand)
EASY_CVAR_EXTERN_DEBUGONLY(shrapRandHeightExtra)
EASY_CVAR_EXTERN_DEBUGONLY(shrapMode)
EASY_CVAR_EXTERN_DEBUGONLY(explosionShrapnelMulti)
EASY_CVAR_EXTERN(cl_explosion)
EASY_CVAR_EXTERN_DEBUGONLY(sparksExplosionMulti)

EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(muteRicochetSound)
EASY_CVAR_EXTERN_DEBUGONLY(fleshhitmakessound)
EASY_CVAR_EXTERN_DEBUGONLY(quakeExplosionSound)

EASY_CVAR_EXTERN_DEBUGONLY(meleeDrawBloodModeA)
EASY_CVAR_EXTERN_DEBUGONLY(meleeDrawBloodModeB)
EASY_CVAR_EXTERN_DEBUGONLY(meleeDrawBloodModeBFix)
EASY_CVAR_EXTERN_DEBUGONLY(meleeDrawBloodModeAOffset)
EASY_CVAR_EXTERN_DEBUGONLY(meleeDrawBloodModeBOffset)

EASY_CVAR_EXTERN_DEBUGONLY(sparksAllMulti)

EASY_CVAR_EXTERN_DEBUGONLY(hgruntPrintout)
EASY_CVAR_EXTERN_DEBUGONLY(panthereyePrintout)
EASY_CVAR_EXTERN_DEBUGONLY(squadmonsterPrintout)
EASY_CVAR_EXTERN_DEBUGONLY(hassaultPrintout)
EASY_CVAR_EXTERN_DEBUGONLY(gargantuaPrintout)
EASY_CVAR_EXTERN_DEBUGONLY(barnaclePrintout)
EASY_CVAR_EXTERN_DEBUGONLY(houndeyePrintout)

EASY_CVAR_EXTERN_DEBUGONLY(canDropInSinglePlayer)
EASY_CVAR_EXTERN_DEBUGONLY(useAlphaSparks)
EASY_CVAR_EXTERN(cl_bullsquidspit)
EASY_CVAR_EXTERN(cl_hornetspiral)
EASY_CVAR_EXTERN_DEBUGONLY(mutePlayerPainSounds)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST(playerBulletHitEffectForceServer)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST(playerWeaponSpreadMode)
EASY_CVAR_EXTERN_DEBUGONLY(sentryCanGib)
EASY_CVAR_EXTERN_DEBUGONLY(miniturretCanGib)
EASY_CVAR_EXTERN_DEBUGONLY(turretCanGib)
EASY_CVAR_EXTERN(soundSentenceSave)
EASY_CVAR_EXTERN(pissedNPCs)
EASY_CVAR_EXTERN(sv_explosion_shake)
EASY_CVAR_EXTERN_DEBUGONLY(sv_explosion_shake_range)
EASY_CVAR_EXTERN_DEBUGONLY(sv_explosion_shake_amp)
EASY_CVAR_EXTERN_DEBUGONLY(sv_explosion_shake_freq)
EASY_CVAR_EXTERN_DEBUGONLY(sv_explosion_shake_duration)





//extern int giPrecacheGrunt;
//extern DLL_GLOBAL short g_sModelIndexBubbles;// holds the index for the bubbles model
extern DLL_GLOBAL int g_iSkillLevel;
extern DLL_GLOBAL short g_sModelIndexLaser;// holds the index for the laser beam

extern BOOL g_queueCVarHiddenSave;


#define HUMAN_GIB_COUNT			6
#define ALIEN_GIB_COUNT			4
#define GERMAN_GIB_COUNT		11


#define ENTVARS_COUNT		(sizeof(gEntvarsDescription)/sizeof(gEntvarsDescription[0]))

#define SWAP(a,b,temp)	((temp)=(a),(a)=(b),(b)=(temp))


EASY_CVAR_DECLARATION_SERVER_MASS

// handled on save/restore in world.cpp
BOOL g_f_playerDeadTruce = FALSE;

//MODDD - event ID's.
unsigned short g_sTrailEngineChoice;
unsigned short g_sImitation7;
unsigned short g_sTrail;
unsigned short g_sTrailRA;
unsigned short g_sCustomBalls;
unsigned short g_sCustomBallsPowerup;
unsigned short g_quakeExplosionEffect;
unsigned short g_decalGunshotCustomEvent;
unsigned short g_sFreakyLight;
unsigned short g_sFriendlyVomit;
unsigned short g_sFloaterExplode;

unsigned short model_explosivestuff;

short g_sGaussBallSprite = 0;
short g_sBallVomitSprite = 0;
short g_sBallForceFieldSprite = 0;

int global_useSentenceSave = 0;   //This is for script only, used to make mass-recognizing lots of sound precache calls --> sentances easier.


//MODDD - keep track of "sv_cheats".
cvar_t* cvar_sv_cheats = 0;

float forceWorldLightOffMem = -1;
float cvar_skill_mem = -1;

float barnacleCanGibMem = -1;


//MODDD - hidden cheat.  Not a CVar, so that it can't be seen by console's autocomplete
float cheat_barnacleEatsEverything = 0;


//VITAL!!!
BOOL globalPSEUDO_queueClientSendoff = FALSE;

float globalPSEUDO_cameraMode = -1;

float globalPSEUDO_canApplyGermanCensorship = 0;
float globalPSEUDO_allowGermanModelsMem = -1;
float globalPSEUDO_germanCensorshipMem = -1;

float globalPSEUDO_cl_bullsquidspit = -1;
float globalPSEUDO_cl_hornetspiral = -1;
float globalPSEUDO_cl_hornettrail = -1;

BOOL globalPSEUDO_germanModel_hgibFound = FALSE;
BOOL globalPSEUDO_germanModel_scientistFound = FALSE;
BOOL globalPSEUDO_germanModel_barneyFound = FALSE;
BOOL globalPSEUDO_germanModel_hgruntFound = FALSE;
BOOL globalPSEUDO_germanModel_hassaultFound = FALSE;


BOOL g_gamePaused = FALSE;
BOOL g_gameLoaded = FALSE;
BOOL g_mapLoaded = FALSE;
BOOL g_mapLoadedEver = FALSE;
BOOL g_firstPlayerEntered = FALSE;
float g_nextCVarUpdate = -1;


//cvar_t* cvar_skill = NULL;
//HEY, already have something like this: g_iSkillLevel!!!
//float global_skill = -1;

BOOL queueSkillUpdate = FALSE;

BOOL queueYMG_stopSend = FALSE;




//MODDD - moved from player.
int giPrecacheGrunt = 0;

int g_groupmask = 0;
int g_groupop = 0;




// Array of gib models with some helpful info.
// This does not include gibs only used by func_breakable. 
// Looks like that file is able to automatically pick up on the available number of body values and use those.
// AND remember, keep 'aryGibInfo_MAX_SIZE' in synch if more entries are added.
GibInfo_t aryGibInfo[aryGibInfo_MAX_SIZE] = {
	{"", 0, 0, 0}, //the dummy
	{"models/hgibs.mdl", 1, HUMAN_GIB_COUNT-1, BLOOD_COLOR_RED},
	// Distinct yellow & green colors now!
	{"models/agibs.mdl", 0, ALIEN_GIB_COUNT-1, BLOOD_COLOR_YELLOW},
	{"models/agibs.mdl", 0, ALIEN_GIB_COUNT-1, BLOOD_COLOR_GREEN},
	{GERMAN_GIB_PATH, 0, GERMAN_GIB_COUNT-1, BLOOD_COLOR_BLACK},
	
	{"models/metalplategibs.mdl", 0, 12, BLOOD_COLOR_BLACK},
	{"models/metalplategibs_dark.mdl", 0, 12, BLOOD_COLOR_BLACK},
	{"models/metalplategibs_green.mdl", 0, 12, BLOOD_COLOR_BLACK},
	{"models/metalgibs.mdl", 1, 5, BLOOD_COLOR_BLACK},
	{"models/shrapnel.mdl", 0, 2, BLOOD_COLOR_BLACK},
};



// grunt stuff
extern int g_fGruntQuestion;
extern float g_hgrunt_allyDeadRecentlyExpireTime;
extern float g_hgrunt_allyDeadRecentlyAgainTime;

extern float g_scientist_PredisasterSuitMentionAllowedTime;
extern float g_scientist_HeadcrabMentionAllowedTime;
extern float g_scientist_sayGetItOffCooldown;
extern int g_TalkMonster_PlayerDead_DialogueMod;
// barney
extern float g_sayBulletHitCooldown;

// Reset global var times to avoid taking a long time to be reached on unusual transition or game-load combos
// (that is, going to a map that has a far higher gpGlobals->time saved will require that to be reached before
// these vars expire).
void resetGlobalVars(void) {


	g_fGruntQuestion = 0;
	g_hgrunt_allyDeadRecentlyExpireTime = -1;
	g_hgrunt_allyDeadRecentlyAgainTime = -1;

	g_nextCVarUpdate = -1;

	g_scientist_PredisasterSuitMentionAllowedTime = -1;
	g_scientist_HeadcrabMentionAllowedTime = -1;
	g_scientist_sayGetItOffCooldown = -1;

	g_TalkMonster_PlayerDead_DialogueMod = 0;
	
	g_sayBulletHitCooldown = -1;


	CTalkMonster::g_talkWaitTime = 0;
	CBarney::g_barneyAlertTalkWaitTime = 0;

	// Reset these gibmodel references to re-get them when these turrets are precached again.
	CTurret::gibModelRef = NULL;
	CMiniTurret::gibModelRef = NULL;
	CSentry::gibModelRef = NULL;

	CBarnacle::s_iStandardGibID = -1;

	CSquidSpit::iSquidSpitSprite = -1;
	
}//resetGlobalVars






/*
//MODDD - Nothing used this?   Whoops.  Might be a good idea to use UTIL_MakeAimVectors instead
// of UTIL_MakeVectors anyway
void UTIL_ParametricRocket( entvars_t *pev, Vector vecOrigin, Vector vecAngles, edict_t *owner )
{	
	pev->startpos = vecOrigin;
	// Trace out line to end pos
	TraceResult tr;
	UTIL_MakeVectors( vecAngles );
	UTIL_TraceLine( pev->startpos, pev->startpos + gpGlobals->v_forward * 8192, ignore_monsters, owner, &tr);
	pev->endpos = tr.vecEndPos;

	// Now compute how long it will take based on current velocity
	Vector vecTravel = pev->endpos - pev->startpos;
	float travelTime = 0.0;
	if ( pev->velocity.Length() > 0 )
	{
		travelTime = vecTravel.Length() / pev->velocity.Length();
	}
	pev->starttime = gpGlobals->time;
	pev->impacttime = gpGlobals->time + travelTime;
}
*/

// Normal overrides
void UTIL_SetGroupTrace( int groupmask, int op )
{
	g_groupmask		= groupmask;
	g_groupop		= op;

	ENGINE_SETGROUPMASK( g_groupmask, g_groupop );
}

void UTIL_UnsetGroupTrace( void )
{
	g_groupmask		= 0;
	g_groupop		= 0;

	ENGINE_SETGROUPMASK( 0, 0 );
}

// Smart version, it'll clean itself up when it pops off stack
UTIL_GroupTrace::UTIL_GroupTrace( int groupmask, int op )
{
	m_oldgroupmask	= g_groupmask;
	m_oldgroupop	= g_groupop;

	g_groupmask		= groupmask;
	g_groupop		= op;

	ENGINE_SETGROUPMASK( g_groupmask, g_groupop );
}

UTIL_GroupTrace::~UTIL_GroupTrace( void )
{
	g_groupmask		=	m_oldgroupmask;
	g_groupop		=	m_oldgroupop;

	ENGINE_SETGROUPMASK( g_groupmask, g_groupop );
}

TYPEDESCRIPTION	gEntvarsDescription[] = 
{
	DEFINE_ENTITY_FIELD( classname, FIELD_STRING ),
	DEFINE_ENTITY_GLOBAL_FIELD( globalname, FIELD_STRING ),
	
	DEFINE_ENTITY_FIELD( origin, FIELD_POSITION_VECTOR ),
	DEFINE_ENTITY_FIELD( oldorigin, FIELD_POSITION_VECTOR ),
	DEFINE_ENTITY_FIELD( velocity, FIELD_VECTOR ),
	DEFINE_ENTITY_FIELD( basevelocity, FIELD_VECTOR ),
	DEFINE_ENTITY_FIELD( movedir, FIELD_VECTOR ),

	DEFINE_ENTITY_FIELD( angles, FIELD_VECTOR ),
	DEFINE_ENTITY_FIELD( avelocity, FIELD_VECTOR ),
	DEFINE_ENTITY_FIELD( punchangle, FIELD_VECTOR ),
	DEFINE_ENTITY_FIELD( v_angle, FIELD_VECTOR ),
	DEFINE_ENTITY_FIELD( fixangle, FIELD_FLOAT ),
	DEFINE_ENTITY_FIELD( idealpitch, FIELD_FLOAT ),
	DEFINE_ENTITY_FIELD( pitch_speed, FIELD_FLOAT ),
	DEFINE_ENTITY_FIELD( ideal_yaw, FIELD_FLOAT ),
	DEFINE_ENTITY_FIELD( yaw_speed, FIELD_FLOAT ),

	DEFINE_ENTITY_FIELD( modelindex, FIELD_INTEGER ),
	DEFINE_ENTITY_GLOBAL_FIELD( model, FIELD_MODELNAME ),

	DEFINE_ENTITY_FIELD( viewmodel, FIELD_MODELNAME ),
	DEFINE_ENTITY_FIELD( weaponmodel, FIELD_MODELNAME ),

	DEFINE_ENTITY_FIELD( absmin, FIELD_POSITION_VECTOR ),
	DEFINE_ENTITY_FIELD( absmax, FIELD_POSITION_VECTOR ),
	DEFINE_ENTITY_GLOBAL_FIELD( mins, FIELD_VECTOR ),
	DEFINE_ENTITY_GLOBAL_FIELD( maxs, FIELD_VECTOR ),
	DEFINE_ENTITY_GLOBAL_FIELD( size, FIELD_VECTOR ),

	DEFINE_ENTITY_FIELD( ltime, FIELD_TIME ),
	DEFINE_ENTITY_FIELD( nextthink, FIELD_TIME ),

	DEFINE_ENTITY_FIELD( solid, FIELD_INTEGER ),
	DEFINE_ENTITY_FIELD( movetype, FIELD_INTEGER ),

	DEFINE_ENTITY_FIELD( skin, FIELD_INTEGER ),
	DEFINE_ENTITY_FIELD( body, FIELD_INTEGER ),
	DEFINE_ENTITY_FIELD( effects, FIELD_INTEGER ),

	DEFINE_ENTITY_FIELD( gravity, FIELD_FLOAT ),
	DEFINE_ENTITY_FIELD( friction, FIELD_FLOAT ),
	DEFINE_ENTITY_FIELD( light_level, FIELD_FLOAT ),

	DEFINE_ENTITY_FIELD( frame, FIELD_FLOAT ),
	DEFINE_ENTITY_FIELD( scale, FIELD_FLOAT ),
	DEFINE_ENTITY_FIELD( sequence, FIELD_INTEGER ),
	DEFINE_ENTITY_FIELD( animtime, FIELD_TIME ),
	DEFINE_ENTITY_FIELD( framerate, FIELD_FLOAT ),
	DEFINE_ENTITY_FIELD( controller, FIELD_INTEGER ),
	DEFINE_ENTITY_FIELD( blending, FIELD_INTEGER ),

	DEFINE_ENTITY_FIELD( rendermode, FIELD_INTEGER ),
	DEFINE_ENTITY_FIELD( renderamt, FIELD_FLOAT ),
	DEFINE_ENTITY_FIELD( rendercolor, FIELD_VECTOR ),
	DEFINE_ENTITY_FIELD( renderfx, FIELD_INTEGER ),

	DEFINE_ENTITY_FIELD( health, FIELD_FLOAT ),
	DEFINE_ENTITY_FIELD( frags, FIELD_FLOAT ),
	DEFINE_ENTITY_FIELD( weapons, FIELD_INTEGER ),
	DEFINE_ENTITY_FIELD( takedamage, FIELD_FLOAT ),

	DEFINE_ENTITY_FIELD( deadflag, FIELD_FLOAT ),
	DEFINE_ENTITY_FIELD( view_ofs, FIELD_VECTOR ),
	DEFINE_ENTITY_FIELD( button, FIELD_INTEGER ),
	DEFINE_ENTITY_FIELD( impulse, FIELD_INTEGER ),

	DEFINE_ENTITY_FIELD( chain, FIELD_EDICT ),
	DEFINE_ENTITY_FIELD( dmg_inflictor, FIELD_EDICT ),
	DEFINE_ENTITY_FIELD( enemy, FIELD_EDICT ),
	DEFINE_ENTITY_FIELD( aiment, FIELD_EDICT ),
	DEFINE_ENTITY_FIELD( owner, FIELD_EDICT ),
	DEFINE_ENTITY_FIELD( groundentity, FIELD_EDICT ),

	DEFINE_ENTITY_FIELD( spawnflags, FIELD_INTEGER ),
	DEFINE_ENTITY_FIELD( flags, FIELD_FLOAT ),

	DEFINE_ENTITY_FIELD( colormap, FIELD_INTEGER ),
	DEFINE_ENTITY_FIELD( team, FIELD_INTEGER ),

	DEFINE_ENTITY_FIELD( max_health, FIELD_FLOAT ),
	DEFINE_ENTITY_FIELD( teleport_time, FIELD_TIME ),
	//MODDD - NOTE.    wat.  armortype is never referred to in the code base.  Oh well.
	DEFINE_ENTITY_FIELD( armortype, FIELD_FLOAT ),
	DEFINE_ENTITY_FIELD( armorvalue, FIELD_FLOAT ),
	DEFINE_ENTITY_FIELD( waterlevel, FIELD_INTEGER ),
	DEFINE_ENTITY_FIELD( watertype, FIELD_INTEGER ),

	// Having these fields be local to the individual levels makes it easier to test those levels individually.
	DEFINE_ENTITY_GLOBAL_FIELD( target, FIELD_STRING ),
	DEFINE_ENTITY_GLOBAL_FIELD( targetname, FIELD_STRING ),
	DEFINE_ENTITY_FIELD( netname, FIELD_STRING ),
	DEFINE_ENTITY_FIELD( message, FIELD_STRING ),

	DEFINE_ENTITY_FIELD( dmg_take, FIELD_FLOAT ),
	DEFINE_ENTITY_FIELD( dmg_save, FIELD_FLOAT ),
	DEFINE_ENTITY_FIELD( dmg, FIELD_FLOAT ),
	DEFINE_ENTITY_FIELD( dmgtime, FIELD_TIME ),

	DEFINE_ENTITY_FIELD( noise, FIELD_SOUNDNAME ),
	DEFINE_ENTITY_FIELD( noise1, FIELD_SOUNDNAME ),
	DEFINE_ENTITY_FIELD( noise2, FIELD_SOUNDNAME ),
	DEFINE_ENTITY_FIELD( noise3, FIELD_SOUNDNAME ),
	DEFINE_ENTITY_FIELD( speed, FIELD_FLOAT ),
	DEFINE_ENTITY_FIELD( air_finished, FIELD_TIME ),
	DEFINE_ENTITY_FIELD( pain_finished, FIELD_TIME ),
	DEFINE_ENTITY_FIELD( radsuit_finished, FIELD_TIME ),
};


#ifdef	DEBUG
	void
DBG_AssertFunction(
	BOOL		fExpr,
	const char*	szExpr,
	const char*	szFile,
	int		szLine,
	const char*	szMessage)
	{
	if (fExpr)
		return;
	char szOut[512];
	if (szMessage != NULL)
		sprintf(szOut, "ASSERT FAILED:\n %s \n(%s@%d)\n%s", szExpr, szFile, szLine, szMessage);
	else
		sprintf(szOut, "ASSERT FAILED:\n %s \n(%s@%d)", szExpr, szFile, szLine);
	ALERT(at_console, szOut);
	}
#endif	// DEBUG

BOOL UTIL_GetNextBestWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pCurrentWeapon )
{
	return g_pGameRules->GetNextBestWeapon( pPlayer, pCurrentWeapon );
}

// ripped this out of the engine
float UTIL_AngleMod(float a)
{
	if (a < 0)
	{
		a = a + 360 * ((int)(a / 360) + 1);
	}
	else if (a >= 360)
	{
		a = a - 360 * ((int)(a / 360));
	}
	// a = (360.0/65536) * ((int)(a*(65536/360.0)) & 65535);
	return a;
}

float UTIL_AngleDiff( float destAngle, float srcAngle )
{
	float delta;

	delta = destAngle - srcAngle;
	if ( destAngle > srcAngle )
	{
		if ( delta >= 180 )
			delta -= 360;
	}
	else
	{
		if ( delta <= -180 )
			delta += 360;
	}
	return delta;
}

Vector UTIL_VecToAngles( const Vector &vec )
{
	float rgflVecOut[3];
	VEC_TO_ANGLES(vec, rgflVecOut);
	return Vector(rgflVecOut);
}
	
//	float UTIL_MoveToOrigin( edict_t *pent, const Vector vecGoal, float flDist, int iMoveType )
void UTIL_MoveToOrigin( edict_t *pent, const Vector &vecGoal, float flDist, int iMoveType )
{
	float rgfl[3];
	vecGoal.CopyToArray(rgfl);
//		return MOVE_TO_ORIGIN ( pent, rgfl, flDist, iMoveType ); 
	MOVE_TO_ORIGIN ( pent, rgfl, flDist, iMoveType ); 
}




//MODDD - this is a version of "UTIL_EntitiesInBox" that can also find barnacles.
//They are ignored by the tag search because they lack any.  So, this just does an extra check fo "monster_barnacle" particularly.
//UPDATE - barnacles are ignored because they lacked FL_MONSTER, which has been given back.
//This is now no longer necessary. It just added a check like so
//    if ( flagMask && !( FStrEq(STRING(pEdict->v.classname), "monster_barnacle") ) && !(pEdict->v.flags & flagMask) )
/*
int UTIL_EntitiesInBoxAlsoBarnacles( CBaseEntity **pList, int listMax, const Vector &mins, const Vector &maxs, int flagMask )
{
	//....
}
*/

int UTIL_EntitiesInBox( CBaseEntity **pList, int listMax, const Vector &mins, const Vector &maxs, int flagMask )
{
	edict_t* pEdict = g_engfuncs.pfnPEntityOfEntIndex( 1 );
	CBaseEntity* pEntity;
	int count;

	count = 0;

	if ( !pEdict )
		return count;

	for ( int i = 1; i < gpGlobals->maxEntities; i++, pEdict++ )
	{
		if ( pEdict->free )	// Not in use
			continue;
		
		if ( flagMask && !(pEdict->v.flags & flagMask) )	// Does it meet the criteria?
			continue;

		if ( mins.x > pEdict->v.absmax.x ||
			 mins.y > pEdict->v.absmax.y ||
			 mins.z > pEdict->v.absmax.z ||
			 maxs.x < pEdict->v.absmin.x ||
			 maxs.y < pEdict->v.absmin.y ||
			 maxs.z < pEdict->v.absmin.z )
			 continue;

		pEntity = CBaseEntity::Instance(pEdict);
		if ( !pEntity )
			continue;

		pList[ count ] = pEntity;
		count++;

		if ( count >= listMax )
			return count;
	}

	return count;
}



int UTIL_NonDeadEntitiesInBox( CBaseEntity **pList, int listMax, const Vector &mins, const Vector &maxs, int flagMask )
{
	edict_t* pEdict = g_engfuncs.pfnPEntityOfEntIndex( 1 );
	CBaseEntity* pEntity;
	int count;

	count = 0;

	if ( !pEdict )
		return count;

	for ( int i = 1; i < gpGlobals->maxEntities; i++, pEdict++ )
	{
		if ( pEdict->free )	// Not in use
			continue;
		
		if ( flagMask && !(pEdict->v.flags & flagMask) )	// Does it meet the criteria?
			continue;

		if ( mins.x > pEdict->v.absmax.x ||
			 mins.y > pEdict->v.absmax.y ||
			 mins.z > pEdict->v.absmax.z ||
			 maxs.x < pEdict->v.absmin.x ||
			 maxs.y < pEdict->v.absmin.y ||
			 maxs.z < pEdict->v.absmin.z )
			 continue;

		pEntity = CBaseEntity::Instance(pEdict);
		if ( !pEntity )
			continue;

		//MODDD - we will not accept "DEAD_DEAD" entities in this search.  Anything in dying anims / not dying is ok.
		if(pEntity->pev->deadflag == DEAD_DEAD){
			continue;
		}

		pList[ count ] = pEntity;
		count++;

		if ( count >= listMax )
			return count;
	}

	return count;
}



int UTIL_MonstersInSphere( CBaseEntity **pList, int listMax, const Vector &center, float radius )
{
	edict_t		*pEdict = g_engfuncs.pfnPEntityOfEntIndex( 1 );
	CBaseEntity *pEntity;
	int		count;
	float distance;
	float delta;

	count = 0;
	float radiusSquared = radius * radius;

	if ( !pEdict )
		return count;

	for ( int i = 1; i < gpGlobals->maxEntities; i++, pEdict++ )
	{
		if ( pEdict->free )	// Not in use
			continue;
		
		if ( !(pEdict->v.flags & (FL_CLIENT|FL_MONSTER)) )	// Not a client/monster ?
			continue;

		// Use origin for X & Y since they are centered for all monsters
		// Now X
		delta = center.x - pEdict->v.origin.x;//(pEdict->v.absmin.x + pEdict->v.absmax.x)*0.5;
		delta *= delta;

		if ( delta > radiusSquared )
			continue;
		distance = delta;
		
		// Now Y
		delta = center.y - pEdict->v.origin.y;//(pEdict->v.absmin.y + pEdict->v.absmax.y)*0.5;
		delta *= delta;

		distance += delta;
		if ( distance > radiusSquared )
			continue;

		// Now Z
		delta = center.z - (pEdict->v.absmin.z + pEdict->v.absmax.z)*0.5;
		delta *= delta;

		distance += delta;
		if ( distance > radiusSquared )
			continue;

		pEntity = CBaseEntity::Instance(pEdict);
		if ( !pEntity )
			continue;

		pList[ count ] = pEntity;
		count++;

		if ( count >= listMax )
			return count;
	}


	return count;
}


CBaseEntity *UTIL_FindEntityInSphere( CBaseEntity *pStartEntity, const Vector &vecCenter, float flRadius )
{
	edict_t	*pentEntity;

	if (pStartEntity)
		pentEntity = pStartEntity->edict();
	else
		pentEntity = NULL;

	pentEntity = FIND_ENTITY_IN_SPHERE( pentEntity, vecCenter, flRadius);

	if (!FNullEnt(pentEntity))
		return CBaseEntity::Instance(pentEntity);
	return NULL;
}


CBaseEntity *UTIL_FindEntityByString( CBaseEntity *pStartEntity, const char *szKeyword, const char *szValue )
{
	edict_t	*pentEntity;

	if (pStartEntity)
		pentEntity = pStartEntity->edict();
	else
		pentEntity = NULL;

	pentEntity = FIND_ENTITY_BY_STRING( pentEntity, szKeyword, szValue );

	if (!FNullEnt(pentEntity))
		return CBaseEntity::Instance(pentEntity);
	return NULL;
}

CBaseEntity *UTIL_FindEntityByClassname( CBaseEntity *pStartEntity, const char *szName )
{
	return UTIL_FindEntityByString( pStartEntity, "classname", szName );
}

CBaseEntity *UTIL_FindEntityByTargetname( CBaseEntity *pStartEntity, const char *szName )
{
	return UTIL_FindEntityByString( pStartEntity, "targetname", szName );
}


CBaseEntity *UTIL_FindEntityGeneric( const char *szWhatever, Vector &vecSrc, float flRadius )
{
	CBaseEntity *pEntity = NULL;

	pEntity = UTIL_FindEntityByTargetname( NULL, szWhatever );
	if (pEntity)
		return pEntity;

	CBaseEntity *pSearch = NULL;
	float flMaxDist2 = flRadius * flRadius;
	while ((pSearch = UTIL_FindEntityByClassname( pSearch, szWhatever )) != NULL)
	{
		float flDist2 = (pSearch->pev->origin - vecSrc).Length();
		flDist2 = flDist2 * flDist2;
		if (flMaxDist2 > flDist2)
		{
			pEntity = pSearch;
			flMaxDist2 = flDist2;
		}
	}
	return pEntity;
}


// returns a CBaseEntity pointer to a player by index.  Only returns if the player is spawned and connected
// otherwise returns NULL
// Index is 1 based
CBaseEntity	*UTIL_PlayerByIndex( int playerIndex )
{
	CBaseEntity *pPlayer = NULL;

	if ( playerIndex > 0 && playerIndex <= gpGlobals->maxClients )
	{
		edict_t *pPlayerEdict = INDEXENT( playerIndex );
		if ( pPlayerEdict && !pPlayerEdict->free )
		{
			pPlayer = CBaseEntity::Instance( pPlayerEdict );
		}
	}
	
	return pPlayer;
}


//MODDD - NOTE.  Kinda backwards, but UTIL_MakeVectors inverts the pitch.
// UTIL_MakeAimVectors, doesn't.  So that's better for nearly anything unless there's some strong
// reason to think the pitch needs to be treated as inverted (mostly dealing with player v_angles).
// Confusing that MakeVectors sends the angle-vector plainly, yet MakeAimVectors inverts the pitch
// before sending it?  That's because the engine call (MAKE_VECTORS) automatically inverts the pitch.
// So to get a result as though the pitch weren't inverted, you have to invert the pitch to make it
// a double negative (as MakeAimVectors does).  Ain't that grand.


/*
// LINE DEMO, put somewhere and see the lines, green ones are good calls.
// Only use UTIL_MakeVectors on the pev->v_angle of players (v_angle is useless for anything else).
// Any math converting between vectors and angles, or the player's pev->angles, must use UTIL_MakeAimVectors
// instead (the engine's automatic pitch invert from MakeVectors is bad).
// Monster pev->angles can use either since the pitch is 0.

UTIL_MakeVectors( arg_entDest->pev->v_angle );
DebugLine_Setup(0, arg_entDest->Center(), arg_entDest->Center() + gpGlobals->v_forward * 800, 0, 255, 0);
UTIL_MakeAimVectors( arg_entDest->pev->v_angle );
DebugLine_Setup(1, arg_entDest->Center(), arg_entDest->Center() + gpGlobals->v_forward * 800, 255, 0, 0);

UTIL_MakeVectors( arg_entDest->pev->angles );
DebugLine_Setup(2, arg_entDest->Center(), arg_entDest->Center() + gpGlobals->v_forward * 800, 255, 0, 255);
UTIL_MakeAimVectors( arg_entDest->pev->angles );
DebugLine_Setup(3, arg_entDest->Center(), arg_entDest->Center() + gpGlobals->v_forward * 800, 0, 255, 0);
*/

// Another test, proof that v_angles + viewpunch gets the way the player is looking while kickback is still decaying.
/*
UTIL_MakeVectors(pev->v_angle + pev->punchangle);
DebugLine_Setup(pev->origin + pev->view_ofs, pev->origin + pev->view_ofs + gpGlobals->v_forward * 500, 0, 255, 0);

UTIL_MakeVectors(pev->v_angle);
DebugLine_Setup(pev->origin + pev->view_ofs, pev->origin + pev->view_ofs + gpGlobals->v_forward * 500, 255, 0, 0);
*/


void UTIL_MakeVectors( const Vector &vecAngles )
{
	MAKE_VECTORS( vecAngles );
}


void UTIL_MakeAimVectors( const Vector &vecAngles )
{
	float rgflVec[3];
	vecAngles.CopyToArray(rgflVec);
	rgflVec[0] = -rgflVec[0];
	MAKE_VECTORS(rgflVec);
}


void UTIL_MakeInvVectors( const Vector &vec, globalvars_t *pgv )
{
	MAKE_VECTORS(vec);

	float tmp;
	pgv->v_right = pgv->v_right * -1;

	SWAP(pgv->v_forward.y, pgv->v_right.x, tmp);
	SWAP(pgv->v_forward.z, pgv->v_up.x, tmp);
	SWAP(pgv->v_right.z, pgv->v_up.y, tmp);
}


static unsigned short FixedUnsigned16( float value, float scale )
{
	int output;

	output = value * scale;
	if ( output < 0 )
		output = 0;
	if ( output > 0xFFFF )
		output = 0xFFFF;

	return (unsigned short)output;
}

static short FixedSigned16( float value, float scale )
{
	int output;

	output = value * scale;

	if ( output > 32767 )
		output = 32767;

	if ( output < -32768 )
		output = -32768;

	return (short)output;
}

// Shake the screen of all clients within radius
// radius == 0, shake all clients
// UNDONE: Allow caller to shake clients not ONGROUND?
// UNDONE: Fix falloff model (disabled)?
// UNDONE: Affect user controls?
void UTIL_ScreenShake( const Vector &center, float amplitude, float frequency, float duration, float radius )
{
	int		i;
	float	localAmplitude;
	ScreenShake	shake;

	shake.duration = FixedUnsigned16( duration, 1<<12 );		// 4.12 fixed
	shake.frequency = FixedUnsigned16( frequency, 1<<8 );	// 8.8 fixed

	for ( i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CBaseEntity *pPlayer = UTIL_PlayerByIndex( i );

		if ( !pPlayer || !(pPlayer->pev->flags & FL_ONGROUND) )	// Don't shake if not onground
			continue;

		//MODDD - another possible condition. The player is blocking any impact.
		if(pPlayer->blocksImpact()){
			continue;
		}


		localAmplitude = 0;

		if ( radius <= 0 )
			localAmplitude = amplitude;
		else
		{
			Vector delta = center - pPlayer->pev->origin;
			float distance = delta.Length();
	
			// Had to get rid of this falloff - it didn't work well
			if ( distance < radius )
				localAmplitude = amplitude;//radius - distance;
		}
		if ( localAmplitude )
		{
			shake.amplitude = FixedUnsigned16( localAmplitude, 1<<12 );		// 4.12 fixed
			
			MESSAGE_BEGIN( MSG_ONE, gmsgShake, NULL, pPlayer->edict() );		// use the magic #1 for "one client"
				
				WRITE_SHORT( shake.amplitude );				// shake amount
				WRITE_SHORT( shake.duration );				// shake lasts this long
				WRITE_SHORT( shake.frequency );				// shake noise frequency

			MESSAGE_END();
		}
	}
}


void UTIL_ScreenShakeAll( const Vector &center, float amplitude, float frequency, float duration )
{
	UTIL_ScreenShake( center, amplitude, frequency, duration, 0 );
}


void UTIL_ScreenFadeBuild( ScreenFade &fade, const Vector &color, float fadeTime, float fadeHold, int alpha, int flags )
{
	fade.duration = FixedUnsigned16( fadeTime, 1<<12 );		// 4.12 fixed
	fade.holdTime = FixedUnsigned16( fadeHold, 1<<12 );		// 4.12 fixed
	fade.r = (int)color.x;
	fade.g = (int)color.y;
	fade.b = (int)color.z;
	fade.a = alpha;
	fade.fadeFlags = flags;
}


void UTIL_ScreenFadeWrite( const ScreenFade &fade, CBaseEntity *pEntity )
{
	if ( !pEntity || !pEntity->IsNetClient() )
		return;

	MESSAGE_BEGIN( MSG_ONE, gmsgFade, NULL, pEntity->edict() );		// use the magic #1 for "one client"
		
		WRITE_SHORT( fade.duration );		// fade lasts this long
		WRITE_SHORT( fade.holdTime );		// fade lasts this long
		WRITE_SHORT( fade.fadeFlags );		// fade type (in / out)
		WRITE_BYTE( fade.r );				// fade red
		WRITE_BYTE( fade.g );				// fade green
		WRITE_BYTE( fade.b );				// fade blue
		WRITE_BYTE( fade.a );				// fade blue

	MESSAGE_END();
}


void UTIL_ScreenFadeAll( const Vector &color, float fadeTime, float fadeHold, int alpha, int flags )
{
	int		i;
	ScreenFade	fade;


	UTIL_ScreenFadeBuild( fade, color, fadeTime, fadeHold, alpha, flags );

	for ( i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CBaseEntity *pPlayer = UTIL_PlayerByIndex( i );
	
		UTIL_ScreenFadeWrite( fade, pPlayer );
	}
}


void UTIL_ScreenFade( CBaseEntity *pEntity, const Vector &color, float fadeTime, float fadeHold, int alpha, int flags )
{
	ScreenFade	fade;

	UTIL_ScreenFadeBuild( fade, color, fadeTime, fadeHold, alpha, flags );
	UTIL_ScreenFadeWrite( fade, pEntity );
}


void UTIL_HudMessage( CBaseEntity *pEntity, const hudtextparms_t &textparms, const char *pMessage )
{
	if ( !pEntity || !pEntity->IsNetClient() )
		return;

	MESSAGE_BEGIN( MSG_ONE, SVC_TEMPENTITY, NULL, pEntity->edict() );
		WRITE_BYTE( TE_TEXTMESSAGE );
		WRITE_BYTE( textparms.channel & 0xFF );

		WRITE_SHORT( FixedSigned16( textparms.x, 1<<13 ) );
		WRITE_SHORT( FixedSigned16( textparms.y, 1<<13 ) );
		WRITE_BYTE( textparms.effect );

		WRITE_BYTE( textparms.r1 );
		WRITE_BYTE( textparms.g1 );
		WRITE_BYTE( textparms.b1 );
		WRITE_BYTE( textparms.a1 );

		WRITE_BYTE( textparms.r2 );
		WRITE_BYTE( textparms.g2 );
		WRITE_BYTE( textparms.b2 );
		WRITE_BYTE( textparms.a2 );

		WRITE_SHORT( FixedUnsigned16( textparms.fadeinTime, 1<<8 ) );
		WRITE_SHORT( FixedUnsigned16( textparms.fadeoutTime, 1<<8 ) );
		WRITE_SHORT( FixedUnsigned16( textparms.holdTime, 1<<8 ) );

		if ( textparms.effect == 2 )
			WRITE_SHORT( FixedUnsigned16( textparms.fxTime, 1<<8 ) );
		
		if ( strlen( pMessage ) < 512 )
		{
			WRITE_STRING( pMessage );
		}
		else
		{
			char tmp[512];
			strncpy( tmp, pMessage, 511 );
			tmp[511] = 0;
			WRITE_STRING( tmp );
		}
	MESSAGE_END();
}

void UTIL_HudMessageAll( const hudtextparms_t &textparms, const char *pMessage )
{
	int		i;

	for ( i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CBaseEntity *pPlayer = UTIL_PlayerByIndex( i );
		if ( pPlayer )
			UTIL_HudMessage( pPlayer, textparms, pMessage );
	}
}


//MODDD
// UTIL_ClientPrint methods moved to util_printout.h/.cpp for serverside
// And now UTIL_SayText ones too.



char *UTIL_dtos1( int d )
{
	static char buf[8];
	sprintf( buf, "%d", d );
	return buf;
}

char *UTIL_dtos2( int d )
{
	static char buf[8];
	sprintf( buf, "%d", d );
	return buf;
}

char *UTIL_dtos3( int d )
{
	static char buf[8];
	sprintf( buf, "%d", d );
	return buf;
}

char *UTIL_dtos4( int d )
{
	static char buf[8];
	sprintf( buf, "%d", d );
	return buf;
}

void UTIL_ShowMessage( const char *pString, CBaseEntity *pEntity )
{
	if ( !pEntity || !pEntity->IsNetClient() )
		return;

	MESSAGE_BEGIN( MSG_ONE, gmsgHudText, NULL, pEntity->edict() );
	WRITE_STRING( pString );
	MESSAGE_END();
}


void UTIL_ShowMessageAll( const char *pString )
{
	int	i;

	// loop through all players

	for ( i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CBaseEntity *pPlayer = UTIL_PlayerByIndex( i );
		if ( pPlayer )
			UTIL_ShowMessage( pString, pPlayer );
	}
}


//void	(*pfnTraceLine)				(const float *v1, const float *v2, int fNoMonsters, edict_t *pentToSkip, TraceResult *ptr);

// Overloaded to add IGNORE_GLASS
void UTIL_TraceLine( const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, IGNORE_GLASS ignoreGlass, edict_t *pentIgnore, TraceResult *ptr )
{
	TRACE_LINE( vecStart, vecEnd, (igmon == ignore_monsters ? TRUE : FALSE) | (ignoreGlass?0x100:0), pentIgnore, ptr );
}

//If I understand right, not giving a "ignoreGlass" parameter implies, don't ignore glass.
void UTIL_TraceLine( const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, edict_t *pentIgnore, TraceResult *ptr )
{
	TRACE_LINE( vecStart, vecEnd, (igmon == ignore_monsters ? TRUE : FALSE), pentIgnore, ptr );
}

void UTIL_TraceHull( const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, int hullNumber, edict_t *pentIgnore, TraceResult *ptr )
{
	TRACE_HULL( vecStart, vecEnd, (igmon == ignore_monsters ? TRUE : FALSE), hullNumber, pentIgnore, ptr );
}



// *** some other rarely-used, if ever,  TRACE_ methods are described in docs.txt too

//MODDDD NOTE - Does this work? And why does it want a hullNumber like TraceHull then? It takes a model by "edict_t *pentModel".
// Perhaps that is just a poorly named "pentIgnore", to be ignored by this trace? No clue.
void UTIL_TraceModel( const Vector &vecStart, const Vector &vecEnd, int hullNumber, edict_t *pentModel, TraceResult *ptr )
{
	g_engfuncs.pfnTraceModel( vecStart, vecEnd, hullNumber, pentModel, ptr );
}

TraceResult UTIL_GetGlobalTrace( )
{
	TraceResult tr;

	tr.fAllSolid		= gpGlobals->trace_allsolid;
	tr.fStartSolid		= gpGlobals->trace_startsolid;
	tr.fInOpen			= gpGlobals->trace_inopen;
	tr.fInWater			= gpGlobals->trace_inwater;
	tr.flFraction		= gpGlobals->trace_fraction;
	tr.flPlaneDist		= gpGlobals->trace_plane_dist;
	tr.pHit			= gpGlobals->trace_ent;
	tr.vecEndPos		= gpGlobals->trace_endpos;
	tr.vecPlaneNormal	= gpGlobals->trace_plane_normal;
	tr.iHitgroup		= gpGlobals->trace_hitgroup;
	return tr;
}

	



void UTIL_fromToBlood(CBaseEntity* arg_entSrc, CBaseEntity* arg_entDest, const float& arg_fltDamage){
	//default distance of 1024.
	UTIL_fromToBlood(arg_entSrc, arg_entDest, arg_fltDamage, 70, NULL, NULL, NULL);
}


void UTIL_fromToBlood(CBaseEntity* arg_entSrc, CBaseEntity* arg_entDest, const float& arg_fltDamage, const float &arg_fltdistanceHint){
	UTIL_fromToBlood(arg_entSrc, arg_entDest, arg_fltDamage, arg_fltdistanceHint, NULL, NULL, NULL);

}


//MODDD - get a straight line from some start pos related to "arg_entSrc" to an end pos, likely related ti "arg_entDest".
// At some place in the line, draw the blood, depending on "meleeDrawBloodMode".
void UTIL_fromToBlood(CBaseEntity* arg_entSrc, CBaseEntity* arg_entDest, const float& arg_fltDamage, const float &arg_fltdistanceHint, Vector* arg_suggestedTraceHullVecEndPos, Vector* arg_suggestedTraceHullStart, Vector* arg_suggestedTraceHullEnd){

	BOOL extraBlood = FALSE;

	//arg_fltdistanceHint

/*

meleeDrawBloodModeA - Mode variable, for drawing blood when "checkTraceHullAttack" is used in a melee attack meant to draw blood.
0: don't draw any blood.
1: use CheckTraceHullAttack's usual "vecEndPos" to draw blood.  Usually very close to the melee attacker itself.
2: use hit of a normal Trace-Line using CheckTraceHull's same start & end points (start = center of attacker's model, end = some distance in the direction the attacker is facing FROM THE CENTER)
3: use hit of a normal Trace-Line using CheckTraceHull's same start (start = center of attacker's model, end = some distance in the direction the attacker is facing FROM THE EYES)
4: use hit of a normal Trace-Line using CheckTraceHull's same start (start = center of attacker's model, end = victim's eye position)
5: use hit of a normal Trace-Line using CheckTraceHull's same start (start = center of attacker's model, end = victim's body target). (Body target is usually the victim model's center)
6: use hit of a normal Trace-Line using (start = eye position of the attacker, end = some distance in the direction the attacker is facing FROM THE CENTER)
7: use hit of a normal Trace-Line using (start = eye position of the attacker, end = some distance in the direction the attacker is facing FROM THE EYES)
8: use hit of a normal Trace-Line using (start = eye position of the attacker, end = eye position of the victim)
9: use hit of a normal Trace-Line using (start = eye position of the attacker, end = victim's "body target")


meleeDrawBloodModeB - Mode variable, for drawing blood when "checkTraceHullAttack" is not used in a melee attack meant to draw blood.
~(Mode 1 is very similar to A's 1, just immitates CheckTraceHullAttack.   Modes 0 & 2 - 6 are identical.)
0: don't draw any blood.
1: Call CheckTraceHullAttack anyway and use the usual "vecEndPos" to draw blood.  Usually very close to the melee attacker itself.
2: use hit of a normal Trace-Line using CheckTraceHull's same start & end points (start = center of attacker's model, end = some distance in the direction the attacker is facing FROM THE CENTER)
3: use hit of a normal Trace-Line using CheckTraceHull's same start (start = center of attacker's model, end = some distance in the direction the attacker is facing FROM THE EYES)
4: use hit of a normal Trace-Line using CheckTraceHull's same start (start = center of attacker's model, end = victim's eye position)
5: use hit of a normal Trace-Line using CheckTraceHull's same start (start = center of attacker's model, end = victim's body target)
6: use hit of a normal Trace-Line using (start = eye position of the attacker, end = some distance in the direction the attacker is facing FROM THE CENTER)
7: use hit of a normal Trace-Line using (start = eye position of the attacker, end = some distance in the direction the attacker is facing FROM THE EYES)
8: use hit of a normal Trace-Line using (start = eye position of the attacker, end = eye position of the victim)
9: use hit of a normal Trace-Line using (start = eye position of the attacker, end = victim's "body target")

*/

	//NOTE: "arg_suggestedTraceHullPosition" missing means 

	if(arg_entSrc==NULL || arg_entDest==NULL){
		easyPrintLine("UTIL_fromToBlood FAILED: source or destination entity is NULL!");
		return;
	}

	// Only point of this is to draw blood, so may as well block on censorship.
	if( !arg_entDest->CanMakeBloodParticles() || !UTIL_ShouldShowBlood(arg_entDest->BloodColor()) ){
		return;
	}



	if(FClassnameIs(arg_entSrc->pev, "monster_headcrab")){
		extraBlood = TRUE;
	}

	

	BOOL checkTraceHullAttackUsed = (arg_suggestedTraceHullStart != NULL && arg_suggestedTraceHullEnd != NULL);

	TraceResult tr;
	Vector vecStart;
	Vector vecEnd;
	Vector vecFromToDir;
	
	Vector vecStart2;
	Vector vecEnd2;


	Vector vecCenterOfSrc = arg_entSrc->pev->origin;
	vecCenterOfSrc.z += arg_entSrc->pev->size.z * 0.5;



	// Wrong!  Any pev->angles may as well use UTIL_MakeAimVectors.
	// Makes no difference for monster pev->angles (0 pitch), but the player needs MakeAimVectors for this.
	// Does the player even ever call tis as a 'entSrc'?  oh.  ah well.
	//if (arg_entSrc->IsPlayer()){
	//	UTIL_MakeVectors( arg_entSrc->pev->angles );
	//}else{
	//	UTIL_MakeAimVectors( arg_entSrc->pev->angles );
	//}
	UTIL_MakeAimVectors(arg_entSrc->pev->angles);


	
	if(checkTraceHullAttackUsed){
		//check for meleeDrawBloodModeA
		switch((int)EASY_CVAR_GET_DEBUGONLY(meleeDrawBloodModeA) ){
			case 0:
				//no draw.
				return;
			break;
			case 1:
				//No line trace, just use checkTraceHull's end pos.


				//recrod these anyways.
				vecStart = *arg_suggestedTraceHullStart;
				vecEnd = *arg_suggestedTraceHullEnd;


				//arg_entDest->DrawAlphaBlood(arg_fltDamage, *arg_suggestedTraceHullVecEndPos);
				vecFromToDir = (vecEnd - vecStart).Normalize();
				arg_entDest->SpawnBloodSlash(arg_fltDamage, *arg_suggestedTraceHullVecEndPos - (vecFromToDir * EASY_CVAR_GET_DEBUGONLY(meleeDrawBloodModeAOffset) ), vecFromToDir, extraBlood );


				if(arg_entDest->IsPlayer()){
					//easyPrintLine("BUT WHATT  %s", STRING(arg_entDest->pev->classname) );
					//CBasePlayer* playa = static_cast<CBasePlayer*>(arg_entDest);
					debugDrawVect = *arg_suggestedTraceHullVecEndPos - (vecFromToDir * EASY_CVAR_GET_DEBUGONLY(meleeDrawBloodModeAOffset) );
					debugDrawVect2 = *arg_suggestedTraceHullStart;
					debugDrawVect3 = *arg_suggestedTraceHullEnd;
					debugDrawVect4 = Vector(0,0,0);
					debugDrawVect5 = Vector(0,0,0);
				}


				return;
			break;
			case 2:
				//use provided args from checkTraceHull for a line trace.
				vecStart = *arg_suggestedTraceHullStart;
				vecEnd = *arg_suggestedTraceHullEnd;

				vecStart2 = vecCenterOfSrc;
				vecEnd2 = vecCenterOfSrc + (gpGlobals->v_forward * arg_fltdistanceHint );

					if(arg_entDest->IsPlayer()){
						//easyPrintLine("BUT WHATT  %s", STRING(arg_entDest->pev->classname) );

						//CBasePlayer* playa = static_cast<CBasePlayer*>(arg_entDest);
						//playa->debugDrawVect = tr.vecEndPos;
						debugDrawVect2 = vecStart;
						debugDrawVect3 = vecEnd;
						debugDrawVect4 = vecStart2;
						debugDrawVect5 = vecEnd2;
					}

			break;


			case 3:
				vecStart = vecCenterOfSrc;
				//note that "EyePosition" is absolute, factors in pev->origin.  No need to add anything else.
				vecEnd = arg_entSrc->EyePosition() + (gpGlobals->v_forward * arg_fltdistanceHint );
			break;
			case 4:
				vecStart = vecCenterOfSrc;
				//note that "EyePosition" is absolute, factors in pev->origin.  No need to add anything else.
				vecEnd = arg_entDest->EyePosition();
			break;
			case 5:
				vecStart = vecCenterOfSrc;
				vecEnd = arg_entDest->BodyTargetMod(vecStart);
			break;
			case 6:
				vecStart = arg_entSrc->EyePosition();
				vecEnd = vecCenterOfSrc + (gpGlobals->v_forward * arg_fltdistanceHint );
			break;
			case 7:
				vecStart = arg_entSrc->EyePosition();
				vecEnd = arg_entSrc->EyePosition() + (gpGlobals->v_forward * arg_fltdistanceHint );
			break;
			case 8:
				vecStart = arg_entSrc->EyePosition();
				vecEnd = arg_entDest->EyePosition();
			break;
			case 9:
				vecStart = arg_entSrc->EyePosition();
				vecEnd = arg_entDest->BodyTargetMod(vecStart);
			break;
			default:
				//POTATO.
				return;
			break;
		}//END OF switch

	}else{
		//check for meleeDrawBloodModeB
		switch((int)EASY_CVAR_GET_DEBUGONLY(meleeDrawBloodModeB)){
			case 0:
				//no draw.
				return;
			break;
			case 1:
				//No checkTraceHull?  Let's do it.  Use its endpos.
				

				if(EASY_CVAR_GET_DEBUGONLY(meleeDrawBloodModeBFix) != 1){
					vecStart = vecCenterOfSrc;
				}else{
					vecStart = vecCenterOfSrc - gpGlobals->v_forward * ((arg_entSrc->pev->size.x / 2) + 1);
					//easyPrintLine("MELEEDRAWBLOODFIX SIZECALC: %.2f", ((arg_entSrc->pev->size.x / 2) + 1));
				}

				vecEnd = vecStart + (gpGlobals->v_forward * arg_fltdistanceHint );
				UTIL_TraceHull( vecStart, vecEnd, dont_ignore_monsters, head_hull, ENT(arg_entSrc->pev), &tr );
				
				if ( tr.pHit ){
					
					vecFromToDir = (vecEnd - vecStart).Normalize();
					arg_entDest->SpawnBloodSlash(arg_fltDamage, tr.vecEndPos - (vecFromToDir * EASY_CVAR_GET_DEBUGONLY(meleeDrawBloodModeBOffset)), vecFromToDir, extraBlood );

					if(arg_entDest->IsPlayer()){
						//easyPrintLine("BUT WHAT %.2f  %s", tr.flFraction, STRING(tr.pHit->v.classname) );
						//CBasePlayer* playa = static_cast<CBasePlayer*>(arg_entDest);
						debugDrawVect = tr.vecEndPos - (vecFromToDir * EASY_CVAR_GET_DEBUGONLY(meleeDrawBloodModeBOffset) );
						debugDrawVect2 = vecStart;
						debugDrawVect3 = vecEnd;
						debugDrawVect4 = Vector(0,0,0);
						debugDrawVect5 = Vector(0,0,0);
					}
				}

				
				return;
			break;
			case 2:
				//No provided args to use.
				vecStart = vecCenterOfSrc;
				vecEnd = vecCenterOfSrc + (gpGlobals->v_forward * arg_fltdistanceHint );
			break;


			case 3:
				vecStart = vecCenterOfSrc;
				//note that "EyePosition" is absolute, factors in pev->origin.  No need to add anything else.
				vecEnd = arg_entSrc->EyePosition() + (gpGlobals->v_forward * arg_fltdistanceHint );
			break;
			case 4:
				vecStart = vecCenterOfSrc;
				//note that "EyePosition" is absolute, factors in pev->origin.  No need to add anything else.
				vecEnd = arg_entDest->EyePosition();
			break;
			case 5:
				vecStart = vecCenterOfSrc;
				vecEnd = arg_entDest->BodyTargetMod(vecStart);
			break;
			case 6:
				vecStart = arg_entSrc->EyePosition();
				vecEnd = vecCenterOfSrc + (gpGlobals->v_forward * arg_fltdistanceHint );
			break;
			case 7:
				vecStart = arg_entSrc->EyePosition();
				vecEnd = arg_entSrc->EyePosition() + (gpGlobals->v_forward * arg_fltdistanceHint );
			break;
			case 8:
				vecStart = arg_entSrc->EyePosition();
				vecEnd = arg_entDest->EyePosition();
			break;
			case 9:
				vecStart = arg_entSrc->EyePosition();
				vecEnd = arg_entDest->BodyTargetMod(vecStart);
			break;
			default:
				//POTATO.
				return;
			break;
		}//END OF switch

	}//END OF else OF if( used checkHullTrace )

	/*
	vecStart = arg_entSrc->pev->origin;
	vecStart.z += arg_entSrc->pev->size.z * 0.5;

	vecEnd = vecStart + (gpGlobals->v_forward * arg_fltdistanceHint );
	*/

	
	UTIL_TraceLine(vecStart, vecEnd, dont_ignore_monsters, ENT(arg_entSrc->pev)/*pentIgnore*/, &tr);
	if(tr.flFraction != 1.0){
		//arg_entDest->DrawAlphaBlood(arg_fltDamage, &tr);

		//arg_entDest->DrawAlphaBlood(arg_fltDamage, &tr);
		//arg_entDest->DrawAlphaBloodSlash(arg_fltDamage, tr.vecEndPos, (vecEnd - vecStart).Normalize() );

		vecFromToDir = (vecEnd - vecStart).Normalize();
		//easyPrintLine("TRACE HULL ATTACK USED?: %d", checkTraceHullAttackUsed);
		if(checkTraceHullAttackUsed){
			arg_entDest->SpawnBloodSlash(arg_fltDamage, tr.vecEndPos - (vecFromToDir * EASY_CVAR_GET_DEBUGONLY(meleeDrawBloodModeAOffset) ), vecFromToDir, extraBlood );
		}else{
			arg_entDest->SpawnBloodSlash(arg_fltDamage, tr.vecEndPos - (vecFromToDir * EASY_CVAR_GET_DEBUGONLY(meleeDrawBloodModeBOffset) ), vecFromToDir, extraBlood );
		}

	}

	if(arg_entDest->IsPlayer()){
		//CBasePlayer* playa = static_cast<CBasePlayer*>(arg_entDest);
		
		if(checkTraceHullAttackUsed){
			debugDrawVect = tr.vecEndPos - (vecFromToDir * EASY_CVAR_GET_DEBUGONLY(meleeDrawBloodModeAOffset));
		}else{
			debugDrawVect = tr.vecEndPos - (vecFromToDir * EASY_CVAR_GET_DEBUGONLY(meleeDrawBloodModeBOffset));
		}
		
		debugDrawVect2 = vecStart;
		debugDrawVect3 = vecEnd;
		debugDrawVect4 = vecStart2;
		debugDrawVect5 = vecEnd2;
	}
}//END OF UTIL_fromToBlood



//open-source version, maybe?
void UTIL_SetSizeAlt( entvars_t* pev, const Vector &vecMin, const Vector &vecMax){
	int i = 0;

	//((float *)pev->absmin)[i] = ((float *)pev->origin)[i] - vecMin;
	//((float *)pev->absmax)[i] = ((float *)pev->origin)[i] + vecMax;

	pev->mins = vecMin;
	pev->maxs = vecMax;

	pev->absmin = pev->origin + vecMin;
	pev->absmax = pev->origin + vecMax;
	
	pev->absmin.x -= 1;
	pev->absmin.y -= 1;
	pev->absmin.z -= 1;
	pev->absmax.x += 1;
	pev->absmax.y += 1;
	pev->absmax.z += 1;
}


void UTIL_SetSize( entvars_t *pev, const Vector &vecMin, const Vector &vecMax )
{

	/*
	// what
	if(pev != NULL){
		const char* daClassname = STRING(pev->classname);
		CBaseEntity* theInst = CBaseEntity::Instance(pev);
		CBaseMonster* monTest = theInst->GetMonsterPointer();
		if(monTest != NULL){
			if(monTest->monsterID != -1){
				int x = 45;
			}
		}
	}
	*/

	SET_SIZE( ENT(pev), vecMin, vecMax );
}

float UTIL_VecToYaw( const Vector &vec )
{
	return VEC_TO_YAW(vec);
}

//same as above, but comes already in radians.
float UTIL_VecToYawRadians( const Vector &vecAng )
{
	return vecAng.y * (M_PI / 180.0f);
}

//shortcut:  just give a ".angles" and this will return the forward-vector for it.
Vector UTIL_VecGetForward2D( const Vector &vecAng )
{
	float yawInRads = vecAng.y * (M_PI / 180.0f);
	//return a vector made of this yaw.
	return Vector(cos(yawInRads), sin(yawInRads), 0);
}

//NOTICE: player's up&down angle is inverted (negative).  Handle that, somehow?  Maybe just before sending off the view angle here, if you do that?
Vector UTIL_VecGetForward( const Vector &vecAng )
{
	const float yawInRads = vecAng.y * (M_PI / 180.0f);
	const float upAndDown = sin( vecAng.x * (M_PI / 180.0f) ) ;
	//return a vector made of this yaw.

	//we need a vector that involves the "upAndDown" angle, but still has a total length of "1".
	//So, any further in the up&down direction will lessen the extent of the 2d directions:

	if(upAndDown == 0){
		return Vector(cos(yawInRads), sin(yawInRads), 0 );

	}else if(upAndDown == 1){
		return Vector(0, 0, upAndDown );

	}else{
		const float new2dLength = 1 - pow(upAndDown, 2.0f);
		const float tempMultiplicand = sqrt(new2dLength);
		return Vector(cos(yawInRads)*tempMultiplicand, sin(yawInRads)*tempMultiplicand, upAndDown );
	}
}

Vector UTIL_velocityToAngles( const Vector &vecVel){
	// nothing happens to angles.z.

	// just want "direction" information about the velocity only.
	Vector vecVelDir = vecVel.Normalize();
	Vector vecVelDir2D = Vector(vecVelDir.x, vecVelDir.y, 0).Normalize();

	// angle X component is pitch: how far looking up or down?   Based on Z component of velocity.
	float xComp = asin(vecVelDir.z) * (180.0f / M_PI);  //get as degrees... yes, really.
	
	// our angle's Y component is made of the velocity's X and Y...
	float yComp = 0;

	if(vecVelDir2D.x == 0){
		// we are at the top or bottom.  Which?
		if(vecVelDir2D.y < 0){
			// bottom.
			yComp = 270;
		}else{
			// top.
			yComp = 90;
		}
	}else if(vecVelDir2D.y == 0){
		// to the leftmost or rightmost, which?
		if(vecVelDir2D.x < 0){
			// left.
			yComp = 180;
		}else{
			// right.
			yComp = 0;
		}
	}else{

		// But wait there is atan2.  Aaaaahhhhh fuck.

		// neither flattened X or Y is 0?  ok.
		if(vecVelDir2D.x >= 0){
			// right-half of the graph (quadrants I and IV).  OK.
			yComp = atan( vecVelDir2D.y / vecVelDir2D.x ) * (180.0f / M_PI);  //to degrees... stop cringing.
		}else{
			// right-half of the graph (quadrants II and III).  CAREFUL!  add 180 degrees.
			yComp = atan( vecVelDir2D.y / vecVelDir2D.x ) * (180.0f / M_PI) + 180;  //to degrees... stop cringing.
		}
	}

	if(yComp < 0){
		yComp += 360;
	}
	if(yComp >= 360){
		yComp -= 360;
	}

	//easyForcePrintLine("VELDIR BE %.2f %.2f %.2f YOUR ANGLES BE %.2f %.2f %.2f", vecVelDir.x, vecVelDir.y, vecVelDir.z, xComp, yComp, 0);
	return Vector(xComp, yComp, 0);
}//END OF UTIL_velocityToAngles


//MODDD - new
Vector UTIL_YawToVec( const float &yaw )
{
	//yaw is from 0 to 359.999, I think?

	//Keep in mind, trigonometry is for arclettes (whatever the  hell they're called), not degrees.
	//~RADIANS, you fool!

	float yawInRads = yaw * (M_PI / 180.0f);

	float x = cos(yawInRads);
	float y = sin(yawInRads);
	float z = 0;
	return Vector(x, y, z);
	
}


void UTIL_SetOrigin( entvars_t *pev, const Vector &vecOrigin )
{
	SET_ORIGIN(ENT(pev), vecOrigin );
}

//MODDD - Spirit of HL had this...  why doesn't it have both though anyways?  (Missing the "entvars_t*" version that comes with Half-Life as-is, like above)
void UTIL_SetOrigin( CBaseEntity *pEntity, const Vector &vecOrigin )
{
	SET_ORIGIN(ENT(pEntity->pev), vecOrigin );
}


void UTIL_ParticleEffect( const Vector &vecOrigin, const Vector &vecDirection, ULONG ulColor, ULONG ulCount )
{
	PARTICLE_EFFECT( vecOrigin, vecDirection, (float)ulColor, (float)ulCount );
}


float UTIL_Approach( float target, float value, float speed )
{
	float delta = target - value;

	if ( delta > speed )
		value += speed;
	else if ( delta < -speed )
		value -= speed;
	else 
		value = target;

	return value;
}


float UTIL_ApproachAngle( float target, float value, float speed )
{
	target = UTIL_AngleMod( target );
	value = UTIL_AngleMod( target );
	
	float delta = target - value;

	// Speed is assumed to be positive
	if ( speed < 0 )
		speed = -speed;

	if ( delta < -180 )
		delta += 360;
	else if ( delta > 180 )
		delta -= 360;

	if ( delta > speed )
		value += speed;
	else if ( delta < -speed )
		value -= speed;
	else 
		value = target;

	return value;
}


float UTIL_AngleDistance( float next, float cur )
{
	float delta = next - cur;

	if ( delta < -180 )
		delta += 360;
	else if ( delta > 180 )
		delta -= 360;

	return delta;
}


float UTIL_SplineFraction( float value, float scale )
{
	value = scale * value;
	float valueSquared = value * value;

	// Nice little ease-in, ease-out spline-like curve
	return 3 * valueSquared - 2 * valueSquared * value;
}

//UTIL_VAR MOVED TO util_printout
	
Vector UTIL_GetAimVector( edict_t *pent, float flSpeed )
{
	Vector tmp;
	GET_AIM_VECTOR(pent, flSpeed, tmp);
	return tmp;
}

int UTIL_IsMasterTriggered(string_t sMaster, CBaseEntity *pActivator)
{
	if (sMaster)
	{
		edict_t *pentTarget = FIND_ENTITY_BY_TARGETNAME(NULL, STRING(sMaster));
	
		if ( !FNullEnt(pentTarget) )
		{
			CBaseEntity *pMaster = CBaseEntity::Instance(pentTarget);
			if ( pMaster && (pMaster->ObjectCaps() & FCAP_MASTER) )
				return pMaster->IsTriggered( pActivator );
		}

		ALERT(at_console, "Master was null or not a master!\n");
	}

	// if this isn't a master entity, just say yes.
	return 1;
}

BOOL UTIL_ShouldShowBlood( int color )
{
	if(color == DONT_BLEED){
		// don't try, clearly a signal not to.
		return FALSE;
	}else if (color == BLOOD_COLOR_RED){
		// hblood CVar allows it and germancensorship is off?  Proceed.
		if (CVAR_GET_FLOAT("violence_hblood") != 0 && EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(sv_germancensorship) != 1) {
			return TRUE;
		}
	}
	else if(color == BLOOD_COLOR_BLACK){
		// oil from robots? never censored.
		return TRUE;
	}
	else if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(sv_germancensorship) != 1)
	{
		// even alien blood is restricted by sv_germancensorship, but not gibs? Keep it this way?
		if (CVAR_GET_FLOAT("violence_ablood") != 0) {
			return TRUE;
		}
	}
	return FALSE;
}

// UTIL_PointContents moved to util_shared.h/.cpp


void UTIL_BloodStream( const Vector &origin, const Vector &direction, int color, int amount )
{
	if (amount == 0) {
		return;
	}


// as described in const.h:
//  coord coord coord (start position)
//  coord coord coord (spray vector)
//  byte (color)
//  byte (speed)
// ...weird that the last coord is described as 'speed' there and 'amount' here, suppose it defines both.
// Shame, would be nice if the effect moved faster without needing to be more particles.
	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, origin );
		WRITE_BYTE( TE_BLOODSTREAM );
		WRITE_COORD( origin.x );
		WRITE_COORD( origin.y );
		WRITE_COORD( origin.z );
		WRITE_COORD( direction.x );
		WRITE_COORD( direction.y );
		WRITE_COORD( direction.z );
		WRITE_BYTE( color );
		WRITE_BYTE( min( amount, 255 ) );
	MESSAGE_END();
}

// And the retail blood effect doesn't do anything with direction.  o-kay then.
void UTIL_BloodDrips( const Vector &origin, const Vector &direction, int color, int amount )
{
	//MODDD - several checks removed, done in the caller instead
	if (amount == 0) {
		return;
	}

	if ( IsMultiplayer() )
	{
		// scale up blood effect in multiplayer for better visibility
		amount *= 2;
	}

	if ( amount > 255 )
		amount = 255;

	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, origin );
		WRITE_BYTE( TE_BLOODSPRITE );
		WRITE_COORD( origin.x);								// pos
		WRITE_COORD( origin.y);
		WRITE_COORD( origin.z);
		WRITE_SHORT( g_sModelIndexBloodSpray );				// initial sprite model
		WRITE_SHORT( g_sModelIndexBloodDrop );				// droplet sprite models
		WRITE_BYTE( color );								// color index into host_basepal
		WRITE_BYTE( min( max( 3, amount / 10 ), 16 ) );		// size
	MESSAGE_END();
}

Vector UTIL_RandomBloodVector( void )
{
	Vector direction;
	direction.x = RANDOM_FLOAT ( -1.0, 1.0 );
	direction.y = RANDOM_FLOAT ( -1.0, 1.0 );
	direction.z = RANDOM_FLOAT ( 0.3, 0.6 );
	return direction;
}
// high, as in upwards moreso.  Maybe?  I really don't get how TE_BLOODSTREAM interprets this, if it does at all.
Vector UTIL_RandomBloodVectorHigh(void)
{
	Vector direction;
	direction.x = RANDOM_FLOAT(-0.33, 0.33);
	direction.y = RANDOM_FLOAT(-0.33, 0.33);
	direction.z = RANDOM_FLOAT(0.6, 1.0);
	return direction;
}


void UTIL_BloodDecalTrace( TraceResult *pTrace, int bloodColor )
{
	if ( UTIL_ShouldShowBlood( bloodColor ) )
	{
		if ( bloodColor == BLOOD_COLOR_RED ){
			UTIL_DecalTrace( pTrace, DECAL_BLOOD1 + RANDOM_LONG(0,5) );
		}else if ( bloodColor == BLOOD_COLOR_BLACK ){
			UTIL_DecalTrace( pTrace, DECAL_OIL1 + RANDOM_LONG(0,1) );
		}else{

			// EXPERIMENTAL
			if (bloodColor == BLOOD_COLOR_GREEN) {
				UTIL_DecalTrace(pTrace, DECAL_GBLOOD1 + RANDOM_LONG(0, 7));
			}
			else {
				//MODDD - if separate decals between BLOOD_COLOR_YELLOW and BLOOD_COLOR_GREEN were found,
				// they would go here as checks for those.
				UTIL_DecalTrace(pTrace, DECAL_YBLOOD1 + RANDOM_LONG(0, 5));
			}

		}
	}
}


void UTIL_DecalTrace( TraceResult *pTrace, int decalNumber )
{
	short entityIndex;
	int index;
	int message;

	if ( decalNumber < 0 )
		return;

	index = gDecals[ decalNumber ].index;

	if ( index < 0 )
		return;

	if (pTrace->flFraction == 1.0)
		return;

	// Only decal BSP models
	if ( pTrace->pHit )
	{
		CBaseEntity *pEntity = CBaseEntity::Instance( pTrace->pHit );
		if ( pEntity && !pEntity->IsBSPModel() )
			return;
		entityIndex = ENTINDEX( pTrace->pHit );
	}
	else 
		entityIndex = 0;

	message = TE_DECAL;
	if ( entityIndex != 0 )
	{
		if ( index > 255 )
		{
			message = TE_DECALHIGH;
			index -= 256;
		}
	}
	else
	{
		message = TE_WORLDDECAL;
		if ( index > 255 )
		{
			message = TE_WORLDDECALHIGH;
			index -= 256;
		}
	}
	
	//???????????????????????????????????????????????????? change index for wacky results. like 134.
	
	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( message );
		WRITE_COORD( pTrace->vecEndPos.x );
		WRITE_COORD( pTrace->vecEndPos.y );
		WRITE_COORD( pTrace->vecEndPos.z );
		WRITE_BYTE( index );
		if (entityIndex) {
			WRITE_SHORT(entityIndex);
		}
	MESSAGE_END();
	
}

/*
==============
UTIL_PlayerDecalTrace

A player is trying to apply his custom decal for the spray can.
Tell connected clients to display it, or use the default spray can decal
if the custom can't be loaded.
==============
*/
void UTIL_PlayerDecalTrace( TraceResult *pTrace, int playernum, int decalNumber, BOOL bIsCustom )
{
	int index;
	
	if (!bIsCustom)
	{
		if ( decalNumber < 0 )
			return;

		index = gDecals[ decalNumber ].index;
		if ( index < 0 )
			return;
	}
	else
		index = decalNumber;

	if (pTrace->flFraction == 1.0)
		return;

	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_PLAYERDECAL );
		WRITE_BYTE ( playernum );
		WRITE_COORD( pTrace->vecEndPos.x );
		WRITE_COORD( pTrace->vecEndPos.y );
		WRITE_COORD( pTrace->vecEndPos.z );
		WRITE_SHORT( (short)ENTINDEX(pTrace->pHit) );
		WRITE_BYTE( index );
	MESSAGE_END();
}

void UTIL_GunshotDecalTrace( TraceResult *pTrace, int decalNumber )
{
	if ( decalNumber < 0 )
		return;

	int index = gDecals[ decalNumber ].index;
	if ( index < 0 )
		return;

	if (pTrace->flFraction == 1.0)
		return;


	//MODDD - NOTE - disabled for now, this is the sparks in the gunshot I think.
	/*
	MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, pTrace->vecEndPos );
		WRITE_BYTE( TE_GUNSHOTDECAL );
		WRITE_COORD( pTrace->vecEndPos.x );
		WRITE_COORD( pTrace->vecEndPos.y );
		WRITE_COORD( pTrace->vecEndPos.z );
		WRITE_SHORT( (short)ENTINDEX(pTrace->pHit) );
		WRITE_BYTE( index );
	MESSAGE_END();
	*/


	//easyForcePrintLine("UTIL_GunshotDecalTrace?");

	UTIL_DecalTrace(pTrace, decalNumber);

	//PLAYBACK_EVENT_FULL (FEV_GLOBAL, NULL, g_decalGunshotCustomEvent, 0.0, 
	//(float *)&pTrace->vecEndPos, (float *)&Vector(0,0,0), 0.0, 0.0, 0, 0, FALSE, FALSE);
	

	//NOTE - this method is called when an NPC misses a monster and hits the world.  Something happens client-side in ev_hldm for the player.
	//easyPrintLine("UTIL.cpp - REPORT!!! BULLET DECAL?");
	//???
	/*
	MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, pTrace->vecEndPos );
		WRITE_BYTE( TE_GUNSHOT );
		WRITE_COORD( pTrace->vecEndPos.x );
		WRITE_COORD( pTrace->vecEndPos.y );
		WRITE_COORD( pTrace->vecEndPos.z );
	MESSAGE_END();
	*/

	//
}


void UTIL_GunshotDecalTraceForceDefault(TraceResult *pTrace, int decalNumber){
	//easyPrintLine("UTIL.cpp - FORCE BULLET DECAL?");

}


Vector UTIL_GetProjectileVelocityExtra(const Vector& playerVelocity, float velocityMode)  {
	//easyPrintLine("UTIL_GetProjectileVelocityExtra %.2f", velocityMode);
	if(velocityMode == 0){
		return Vector(0, 0, 0);
	}else if(velocityMode == 1){
		return playerVelocity;
	}else{
		return playerVelocity * velocityMode;
	}
}


// MODDD - UTIL_Sparks moved to util_shared.cpp.


void interpretSoundAsSentence(char* interpretationFINAL, const char* pszName){
	char interpretation1[46];
	int finalOutputOffset = 1;
	BOOL reading = TRUE;
	int readIndex = 0;
	int writeIndex = 0;
	int currentLength = 0;
	BOOL success = FALSE;
	BOOL foundSlash = FALSE;
	char* writeTo = interpretation1;

	interpretationFINAL[0] = '!';

	int endOutputIndex = -1;

	while(reading){
		char thisChar = pszName[readIndex];

		switch(thisChar){
		case '/':
			if(!foundSlash){
				foundSlash = TRUE;

				//4
				//easyPrintLine("SECT 1 : %d", currentLength);
				if(currentLength <= 15){
					strncpy( &interpretationFINAL[finalOutputOffset], &writeTo[0], currentLength );
					interpretationFINAL[finalOutputOffset + currentLength] = '_';
					interpretationFINAL[finalOutputOffset + currentLength+1] = '\0';

					//writeTo = interpretation2;
					writeIndex = 0;
					finalOutputOffset = finalOutputOffset + currentLength + 1;
					//easyPrintLine("??? %s %s %d %d", interpretation1, interpretationFINAL, finalOutputOffset, currentLength);
					currentLength = 0;
			
				}else{
					easyForcePrintLine("ERROR CODE 32589321");
					break;
					// Nevermind below here... apparently.

					strncpy( &interpretationFINAL[finalOutputOffset], &writeTo[0], 2 );
					strncpy( &interpretationFINAL[finalOutputOffset+2], &writeTo[currentLength-2], 2 );

					/*
					interpretationFINAL[finalOutputOffset + 0] = writeTo[0];
					interpretationFINAL[finalOutputOffset + 1] = writeTo[1];
					interpretationFINAL[finalOutputOffset + 2] = writeTo[currentLength-2];
					interpretationFINAL[finalOutputOffset + 3] = writeTo[currentLength-1];
					*/
					interpretationFINAL[finalOutputOffset + 4] = '_';

					//writeTo = interpretation2;
					writeIndex = 0;
					currentLength = 0;
					finalOutputOffset = finalOutputOffset + 5;
				}

			}else{
				easyForcePrintLine("!!!ERROR: SECOND SLASH IN SOUND SAMPLE: %s", pszName);
				break;
			}

		break;
		case '\0':
		case '.':
			success = TRUE;
			if(writeIndex < 20){
				writeTo[writeIndex] = '\0';
			}

			endOutputIndex = readIndex - 1;
			reading = FALSE;

		break;
		case '_':
			//skip.
			writeTo[writeIndex] = thisChar;
			writeIndex++;
			currentLength += 1;
			//...nah, go ahead and include, we have space now.

		break;
		default:
			writeTo[writeIndex] = thisChar;
			writeIndex++;
			currentLength += 1;

		break;
		}//END OF switch(thisChar)

		readIndex += 1;

	}//END OF while(reading)


	if(!success){
		easyForcePrintLine("ERROR CODE: GENERIC45");
	}

	if(currentLength <= 30){
		//easyPrintLine("wat??? %d", finalOutputOffset);
		strncpy( &interpretationFINAL[finalOutputOffset], &writeTo[0], currentLength );
		interpretationFINAL[finalOutputOffset + currentLength] = '\0';
	}else{
		easyForcePrintLine("ERROR CODE 5474572");
		return;

		//easyPrintLine("wat??! %d %d %d", finalOutputOffset + currentLength, finalOutputOffset, currentLength);
		strncpy( &interpretationFINAL[finalOutputOffset], &writeTo[0], 6 );
		strncpy( &interpretationFINAL[finalOutputOffset+6], &writeTo[currentLength-5], 5 );
	
		//easyPrintLine("LAST? %d", finalOutputOffset + 11);
		interpretationFINAL[finalOutputOffset + 11] = '\0';
	}

	//easyPrintLine("OFFSET? %d srcSTART? %d FILENAMELENGTH? %d", offset, srcStart, fileNameLength);
	//easyPrintLine("STRING1: %s", interpretation1);
	////easyPrintLine("STRING2: %s", interpretation2);
	//easyPrintLine("STRINGF: %s", interpretationFINAL);
}


void UTIL_PlaySound(entvars_t* entity, int channel, const char* pszName, float volume, float attenuation ){
	UTIL_PlaySound(ENT(entity), channel, pszName, volume, attenuation, 0, 100, ((CBaseEntity*)(CBaseEntity::Instance(entity)))->usesSoundSentenceSave());
}
void UTIL_PlaySound(entvars_t* entity, int channel, const char* pszName, float volume, float attenuation, BOOL useSoundSentenceSave ){
	UTIL_PlaySound(ENT(entity), channel, pszName, volume, attenuation, 0, 100, useSoundSentenceSave);
}
void UTIL_PlaySound(edict_t* entity, int channel, const char* pszName, float volume, float attenuation ){
	UTIL_PlaySound(entity, channel, pszName, volume, attenuation, 0, 100, ((CBaseEntity*)(CBaseEntity::Instance(entity)))->usesSoundSentenceSave());
}
void UTIL_PlaySound(edict_t* entity, int channel, const char* pszName, float volume, float attenuation, BOOL useSoundSentenceSave ){
	UTIL_PlaySound(entity, channel, pszName, volume, attenuation, 0, 100, useSoundSentenceSave);
}
void UTIL_PlaySound(entvars_t* entity, int channel, const char* pszName, float volume, float attenuation, int flags, int pitch ){
	UTIL_PlaySound(ENT(entity), channel, pszName, volume, attenuation, flags, pitch, ((CBaseEntity*)(CBaseEntity::Instance(entity)))->usesSoundSentenceSave());
}
void UTIL_PlaySound(entvars_t* entity, int channel, const char* pszName, float volume, float attenuation, int flags, int pitch, BOOL useSoundSentenceSave ){
	UTIL_PlaySound(ENT(entity), channel, pszName, volume, attenuation, flags, pitch, useSoundSentenceSave);
}
void UTIL_PlaySound(edict_t* entity, int channel, const char* pszName, float volume, float attenuation, int flags, int pitch){
	UTIL_PlaySound(entity, channel, pszName, volume, attenuation, flags, pitch, ((CBaseEntity*)(CBaseEntity::Instance(entity)))->usesSoundSentenceSave());
}


void UTIL_PlaySound(edict_t* entity, int channel, const char* pszName, float volume, float attenuation, int flags, int pitch, BOOL useSoundSentenceSave ){
	if ( !pszName )
		return;
/*
	if(stringStartsWith(pszName, "player/") ){
		useSoundSentenceSave = TRUE;
	}
*/

	//easyPrintLine("PLAYING SOUND %s", pszName);

	//easyPrintLine("ATTEMPT FILTERED SOUND: %s %d", pszName, useSoundSentenceSave);
	if(EASY_CVAR_GET(soundSentenceSave) == 1 && (useSoundSentenceSave)){

		char interpretationFINAL[50];
		if(pszName[0] != '!'){
			interpretSoundAsSentence(interpretationFINAL, pszName);
		}else{
			// already a sentence, send as is.
			strcpy(&interpretationFINAL[0], pszName);
		}
		//easyForcePrintLine("UTIL_PlaySound - used sentence: |||%s|||", interpretationFINAL);

		/*
		if(pMonster != NULL && pMonster->pev != NULL){
		easyPrintLine("WHAT NAME %s", STRING(pMonster->pev->classname));
		}else{
		easyPrintLine("BOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO");
		}
		*/

		EMIT_SOUND_DYN(entity, channel, interpretationFINAL, volume, attenuation, flags, pitch);
	}else{
		// no sentence-save interpretation, send it as it came.
		EMIT_SOUND_DYN(entity, channel, pszName, volume, attenuation, flags, pitch);
	}


//else
	//	SENTENCEG_PlayRndSz( ENT( pev ), pszSentence, volume, attenuation, 0, GetVoicePitch() );

}//END OF UTIL_PlaySound



void EMIT_SOUND_DYN(edict_t* entity, int channel, const char* pszName, float volume, float attenuation, int flags, int pitch){
	//easyPrintLine("sound was: %s  %d", pszName, flags);
	/*
	if(flags != 0){
		easyPrintLine("******************************************************");
		easyPrintLine("******************************************************");
		easyPrintLine("******************************************************");
		easyPrintLine("******************************************************");
		easyPrintLine("***IMPORTANT!  SOUND WITH FLAGS SENT!  REPORT THIS!***");
		easyPrintLine("******************************************************");
		easyPrintLine("******************************************************");
		easyPrintLine("******************************************************");
		easyPrintLine("******************************************************");
		easyPrintLine("sound was: %s  %d", pszName, flags);
		if(entity){
			easyPrintLine("entity that gave this: %s", STRING(entity->v.classname));
		}else{
			easyPrintLine("entity that gave this: NULL!!!");
		}

		//CBaseMonster* attempt = GetClassPtr((CBaseMonster *)entity);

	}
	*/
	
	//easyPrintLine("EMITTED SOUND SON %s %s", STRING(CBaseEntity::Instance(entity)->pev->classname), pszName  );
	

	//if(!FClassnameIs(entity, "env_spark")){
	//	int x = 45;
	//}

	if (pszName && *pszName == '!')
	{
		char name[32];
		if (SENTENCEG_Lookup(pszName, name) >= 0){
			EMIT_SOUND_DYN2(entity, channel, name, volume, attenuation, flags, pitch);
			//easyPrintLine("SENTENCE PLAYED SENDOFF: %s :::%s", pszName, name);
		}else{
			ALERT( at_aiconsole, "Unable to find %s in sentences.txt\n", pszName );
		}
	}
	else {
		EMIT_SOUND_DYN2(entity, channel, pszName, volume, attenuation, flags, pitch);
	}


	//if(0){
	//	g_engfuncs.pfnEmitSound(NULL, 0, "", 0, 0, 0, 0);
	//}

}//END OF EMIT_SOUND_DYN




//MODDD - suit sound methods. They specify some flags for EMIT_SOUND_DYN, the raw engine call for playing sounds.
//        They don't need to use the soundsentencesave system - it is implied that the user knows they are sentences and treats them as such.
//        No sound file references are to be used with these is all, and exclamation marks in front for single sentences. Sentence Groups
//        don't get them.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// play a specific sentence over the HEV suit speaker - just pass player entity, and !sentencename
// MODDD - lower the attenuation to not play for other players!
void EMIT_SOUND_SUIT(edict_t* entity, const char* pszName)
{
	float fvol;
	int pitch = PITCH_NORM;

	// Bizarre, setting the "HEV suit volume" slider in the Audio settings
	// sets suitvolume to 2 (over 1), crashing the game just like in retail.
	// Valid audio range is 0 to 1.
	fvol = UTIL_clamp(CVAR_GET_FLOAT("suitvolume")/2, 0, 1);

	if (RANDOM_LONG(0,1))
		pitch = RANDOM_LONG(0,6) + 98;

	//MODDD - changed min volume from 0.05 to 0.02.
	if (fvol > 0.02) {
		//EMIT_SOUND_DYN(entity, CHAN_STATIC, pszName, fvol, ATTN_NORM, 0, pitch);
		// higher attenuation to expire faster with distance
		EMIT_SOUND_DYN(entity, CHAN_STATIC, pszName, fvol, 4.0f, 0, pitch);
	}
}

//MODDD - new.  Similar to EMIT_SOUND_SUIT, but have the SND_STOP to stop the sound / sentence.
void STOP_SOUND_SUIT(edict_t* entity, const char* pszName)
{
	const float fvol = 1;
	int pitch = PITCH_NORM;
	
	///fvol = clamp(CVAR_GET_FLOAT("suitvolume")/2, 0, 1);

	//if (RANDOM_LONG(0,1))
	//	pitch = RANDOM_LONG(0,6) + 98;

	//if (fvol > 0.02)
		EMIT_SOUND_DYN(entity, CHAN_STATIC, pszName, fvol, ATTN_NORM, SND_STOP, pitch);
}

// play a sentence, randomly selected from the passed in group id, over the HEV suit speaker
void EMIT_GROUPID_SUIT(edict_t* entity, int isentenceg)
{
	float fvol;
	int pitch = PITCH_NORM;
	
	fvol = UTIL_clamp(CVAR_GET_FLOAT("suitvolume")/2, 0, 1);
	if (RANDOM_LONG(0,1))
		pitch = RANDOM_LONG(0,6) + 98;

	if (fvol > 0.02)
		SENTENCEG_PlayRndI(entity, isentenceg, fvol, ATTN_NORM, 0, pitch);
}

// play a sentence, randomly selected from the passed in groupname
void EMIT_GROUPNAME_SUIT(edict_t* entity, const char* groupname)
{
	float fvol;
	int pitch = PITCH_NORM;
	
	fvol = UTIL_clamp(CVAR_GET_FLOAT("suitvolume")/2, 0, 1);
	if (RANDOM_LONG(0,1))
		pitch = RANDOM_LONG(0,6) + 98;

	if (fvol > 0.02)
		SENTENCEG_PlayRndSz(entity, groupname, fvol, ATTN_NORM, 0, pitch);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




//MODDD NOTE
// UTIL_PlaySound tells whether to do the sound-sentence-save effect (use the equivalent sentence in sentences.txt
// instead of playing normally to save on precaches).
// After that, EMIT_SOUND_DYN acts as a parser to tell whether what it has is a single-sentence call or not
// (the engine then figures out whether it is a single sound file reference or a sentence-group reference... who came up with this?)
// Then, EMIT_SOUDN_DYN2 is the direct call to the engine.
// The AmbientSound calls now work in the same way: Call UTIL_EmitAmbientSound to let whether to use the soundsentencesave
// system be implied or forced yes/no, that calls EMIT_AMBIENT_SOUND which does a little more processing, then 
// that calls EMIT_AMBIENT_SOUND2 (engine).

//entvars_t??
void UTIL_EmitAmbientSound( entvars_t* entity, const Vector &vecOrigin, const char *samp, float vol, float attenuation ){
	UTIL_EmitAmbientSound(ENT(entity), vecOrigin, samp, vol, attenuation, 0, 100, ((CBaseEntity*)(CBaseEntity::Instance(entity)))->usesSoundSentenceSave());
}
void UTIL_EmitAmbientSound( entvars_t* entity, const Vector &vecOrigin, const char *samp, float vol, float attenuation, BOOL useSoundSentenceSave ){
	UTIL_EmitAmbientSound(ENT(entity), vecOrigin, samp, vol, attenuation, 0, 100, useSoundSentenceSave);
}
void UTIL_EmitAmbientSound( entvars_t* entity, const Vector &vecOrigin, const char *samp, float vol, float attenuation, int fFlags, int pitch ){
	UTIL_EmitAmbientSound(ENT(entity), vecOrigin, samp, vol, attenuation, fFlags, pitch, ((CBaseEntity*)(CBaseEntity::Instance(entity)))->usesSoundSentenceSave());
}
void UTIL_EmitAmbientSound( entvars_t* entity, const Vector &vecOrigin, const char *samp, float vol, float attenuation, int fFlags, int pitch, BOOL useSoundSentenceSave ){
	UTIL_EmitAmbientSound(ENT(entity), vecOrigin, samp, vol, attenuation, fFlags, pitch, useSoundSentenceSave);
}
void UTIL_EmitAmbientSound( edict_t* entity, const Vector &vecOrigin, const char *samp, float vol, float attenuation, int fFlags, int pitch ){
	UTIL_EmitAmbientSound(entity, vecOrigin, samp, vol, attenuation, fFlags, pitch, ((CBaseEntity*)(CBaseEntity::Instance(entity)))->usesSoundSentenceSave());
}
void UTIL_EmitAmbientSound( edict_t* entity, const Vector &vecOrigin, const char *samp, float vol, float attenuation, int fFlags, int pitch, BOOL useSoundSentenceSave )
{
	if (!samp) {
		return;
	}

	//CBaseEntity* pEntity = (CBaseEntity*)(CBaseEntity::Instance(entity));

	//easyPrintLine("UTIL_EmitAmbientSound ATTEMPT FILTERED SOUND: %s %d %d", pszName, pMonster->usesSoundSentenceSave(), useSoundSentenceSave);
	if(EASY_CVAR_GET(soundSentenceSave) == 1 && (useSoundSentenceSave)){

		char interpretationFINAL[50];
		if(samp[0] != '!'){
			interpretSoundAsSentence(interpretationFINAL, samp);
		}else{
			//already a sentence, send as is.
			strcpy(&interpretationFINAL[0], samp);
		}
		//easyPrintLine("UTIL_EmitAmbientSound - used sentence: |||%s|||", interpretationFINAL);

		EMIT_AMBIENT_SOUND( entity, vecOrigin, interpretationFINAL, vol, attenuation, fFlags, pitch);
	}else{
		EMIT_AMBIENT_SOUND( entity, vecOrigin, samp, vol, attenuation, fFlags, pitch);
	}

}//END OF UTIL_EmitAmbientSound



void EMIT_AMBIENT_SOUND( edict_t *entity, const Vector &vecOrigin, const char *samp, float vol, float attenuation, int fFlags, int pitch){
	float rgfl[3];
	vecOrigin.CopyToArray(rgfl);

	if (samp && *samp == '!'){
		char name[32];
		if (SENTENCEG_Lookup(samp, name) >= 0) {
			EMIT_AMBIENT_SOUND2(entity, rgfl, name, vol, attenuation, fFlags, pitch);
		}
	}
	else {
		EMIT_AMBIENT_SOUND2(entity, rgfl, samp, vol, attenuation, fFlags, pitch);
	}
}//END OF UTIL_EmitAmbientSound




void UTIL_PRECACHESOUND(char* path){
	//scientist/sci_dragoff.wav
	//easyPrintLine("PRECACHE: %s", path);

	//easyForcePrintLine("UTIL_PRECACHESOUND call 1  %s : %.1f %d", path, global_useSentenceSave, !global_useSentenceSave);

	if(!global_useSentenceSave){
		// if the "useSentenceSave" variable isn't turned on (to help make affecting mass precaches more managable), don't let this be skipped.
		UTIL_PRECACHESOUND(path, TRUE);
	}else{
		// Otherwise, skip away, provided the CVar "soundSentenceSave" still allows it.
		UTIL_PRECACHESOUND(path, FALSE);
	}
}

void UTIL_PRECACHESOUND(char* path, BOOL dontSkipSave){

	//easyPrintLine("LETS PRECACHE... %s, %d", path, skipSave);

	// To be clear, this just means... if the soundSentenceSave CVar is off, we have to precache this sound unconditionally.
	// Otherwise, ordinarilly skip the precache, but if "dontSkipSave" is on, we still have to precache the sound. Some things defy the soundSentenceSave and play without sentences anyways.
	//easyForcePrintLine("UTIL_PRECACHESOUND call 2  %.1f %d", EASY_CVAR_GET(soundSentenceSave), dontSkipSave);
	if(EASY_CVAR_GET(soundSentenceSave) == 0 || dontSkipSave){
		PRECACHE_SOUND_REAL(path);
	}
}


void UTIL_PRECACHESOUND_ARRAY(const char* a[], int aSize){
	if(!global_useSentenceSave){
		UTIL_PRECACHESOUND_ARRAY(a, aSize, TRUE);
	}else{
		UTIL_PRECACHESOUND_ARRAY(a, aSize, FALSE);
	}
}
void UTIL_PRECACHESOUND_ARRAY(const char* a[], int aSize, BOOL dontSkipSave){
	for (int i = 0; i < aSize; i++ ) PRECACHE_SOUND( (char *) a[i], dontSkipSave); 
}



void UTIL_Explosion(int msg_dest, const Vector &location, short sprite, float size, int framerate, int flag){
	UTIL_Explosion(msg_dest, NULL, NULL, NULL, location, 0.0f, 0.0f, 0.0f, sprite, size, framerate, flag, location, 1);
}
void UTIL_Explosion(int msg_dest, const float* pMsgOrigin, const Vector& location, short sprite, float size, int framerate, int flag) {
	UTIL_Explosion(msg_dest, pMsgOrigin, NULL, NULL, location, 0.0f, 0.0f, 0.0f, sprite, size, framerate, flag, location, 1);
}
void UTIL_Explosion(int msg_dest, const float* pMsgOrigin, edict_t* ed, const Vector& location, short sprite, float size, int framerate, int flag) {
	UTIL_Explosion(msg_dest, pMsgOrigin, ed, NULL, location, 0.0f, 0.0f, 0.0f, sprite, size, framerate, flag, location, 1);
}

void UTIL_Explosion(int msg_dest, const Vector &location, float offsetx, float offsety, float offsetz, short sprite, float size, int framerate, int flag){
	UTIL_Explosion(msg_dest, NULL, NULL, NULL, location, offsetx, offsety, offsetz, sprite, size, framerate, flag, location, 1);
}
void UTIL_Explosion(int msg_dest, const float* pMsgOrigin, const Vector& location, float offsetx, float offsety, float offsetz, short sprite, float size, int framerate, int flag) {
	UTIL_Explosion(msg_dest, pMsgOrigin, NULL, NULL, location, offsetx, offsety, offsetz, sprite, size, framerate, flag, location, 1);
}
void UTIL_Explosion(int msg_dest, const float* pMsgOrigin, edict_t* ed, const Vector& location, float offsetx, float offsety, float offsetz, short sprite, float size, int framerate, int flag) {
	UTIL_Explosion(msg_dest, pMsgOrigin, ed, NULL, location, offsetx, offsety, offsetz, sprite, size, framerate, flag, location, 1);
}

void UTIL_Explosion(int msg_dest, const Vector &location, short sprite, float size, int framerate, int flag, const Vector& altLocation){
	UTIL_Explosion(msg_dest, NULL, NULL, NULL, location, 0.0f, 0.0f, 0.0f, sprite, size, framerate, flag, altLocation, 1);
}
void UTIL_Explosion(int msg_dest, const float* pMsgOrigin, const Vector& location, short sprite, float size, int framerate, int flag, const Vector& altLocation) {
	UTIL_Explosion(msg_dest, pMsgOrigin, NULL, NULL, location, 0.0f, 0.0f, 0.0f, sprite, size, framerate, flag, altLocation, 1);
}
void UTIL_Explosion(int msg_dest, const float* pMsgOrigin, edict_t* ed, const Vector& location, short sprite, float size, int framerate, int flag, const Vector& altLocation) {
	UTIL_Explosion(msg_dest, pMsgOrigin, ed, NULL, location, 0.0f, 0.0f, 0.0f, sprite, size, framerate, flag, altLocation, 1);
}

void UTIL_Explosion(int msg_dest, entvars_t* pev, const Vector &location, short sprite, float size, int framerate, int flag){
	UTIL_Explosion(msg_dest, NULL, NULL, pev, location, 0.0f, 0.0f, 0.0f, sprite, size, framerate, flag, location, 1);
}
void UTIL_Explosion(int msg_dest, const float* pMsgOrigin, entvars_t* pev, const Vector& location, short sprite, float size, int framerate, int flag) {
	UTIL_Explosion(msg_dest, pMsgOrigin, NULL, pev, location, 0.0f, 0.0f, 0.0f, sprite, size, framerate, flag, location, 1);
}
void UTIL_Explosion(int msg_dest, const float* pMsgOrigin, edict_t* ed, entvars_t* pev, const Vector& location, short sprite, float size, int framerate, int flag) {
	UTIL_Explosion(msg_dest, pMsgOrigin, ed, pev, location, 0.0f, 0.0f, 0.0f, sprite, size, framerate, flag, location, 1);
}


void UTIL_Explosion(int msg_dest, entvars_t* pev, const Vector &location, float offsetx, float offsety, float offsetz, short sprite, float size, int framerate, int flag){
	UTIL_Explosion(msg_dest, NULL, NULL, pev, location, offsetx, offsety, offsetz, sprite, size, framerate, flag, location, 1);
}
void UTIL_Explosion(int msg_dest, const float* pMsgOrigin, entvars_t* pev, const Vector& location, float offsetx, float offsety, float offsetz, short sprite, float size, int framerate, int flag) {
	UTIL_Explosion(msg_dest, pMsgOrigin, NULL, pev, location, offsetx, offsety, offsetz, sprite, size, framerate, flag, location, 1);
}
void UTIL_Explosion(int msg_dest, const float* pMsgOrigin, edict_t* ed, entvars_t* pev, const Vector& location, float offsetx, float offsety, float offsetz, short sprite, float size, int framerate, int flag) {
	UTIL_Explosion(msg_dest, pMsgOrigin, ed, pev, location, offsetx, offsety, offsetz, sprite, size, framerate, flag, location, 1);
}


void UTIL_Explosion(int msg_dest, entvars_t* pev, const Vector &location, float offsetx, float offsety, float offsetz, short sprite, float size, int framerate, int flag, float shrapMod){
	UTIL_Explosion(msg_dest, NULL, NULL, pev, location, offsetx, offsety, offsetz, sprite, size, framerate, flag, location, shrapMod);
}
void UTIL_Explosion(int msg_dest, const float* pMsgOrigin, entvars_t* pev, const Vector& location, float offsetx, float offsety, float offsetz, short sprite, float size, int framerate, int flag, float shrapMod) {
	UTIL_Explosion(msg_dest, pMsgOrigin, NULL, pev, location, offsetx, offsety, offsetz, sprite, size, framerate, flag, location, shrapMod);
}
void UTIL_Explosion(int msg_dest, const float* pMsgOrigin, edict_t* ed, entvars_t* pev, const Vector& location, float offsetx, float offsety, float offsetz, short sprite, float size, int framerate, int flag, float shrapMod) {
	UTIL_Explosion(msg_dest, pMsgOrigin, ed, pev, location, offsetx, offsety, offsetz, sprite, size, framerate, flag, location, shrapMod);
}




void UTIL_Explosion(int msg_dest, entvars_t* pev, const Vector &location, short sprite, float size, int framerate, int flag, const Vector& altLocation){
	UTIL_Explosion(msg_dest, NULL, NULL, pev, location, 0.0f, 0.0f, 0.0f, sprite, size, framerate, flag, altLocation, 1);
}
void UTIL_Explosion(int msg_dest, const float* pMsgOrigin, entvars_t* pev, const Vector& location, short sprite, float size, int framerate, int flag, const Vector& altLocation) {
	UTIL_Explosion(msg_dest, pMsgOrigin, NULL, pev, location, 0.0f, 0.0f, 0.0f, sprite, size, framerate, flag, altLocation, 1);
}
void UTIL_Explosion(int msg_dest, const float* pMsgOrigin, edict_t* ed, entvars_t* pev, const Vector& location, short sprite, float size, int framerate, int flag, const Vector& altLocation) {
	UTIL_Explosion(msg_dest, pMsgOrigin, ed, pev, location, 0.0f, 0.0f, 0.0f, sprite, size, framerate, flag, altLocation, 1);
}


void UTIL_Explosion(int msg_dest, entvars_t* pev, const Vector &location, short sprite, float size, int framerate, int flag, const Vector& altLocation, float shrapMod){
	UTIL_Explosion(msg_dest, NULL, NULL, pev, location, 0.0f, 0.0f, 0.0f, sprite, size, framerate, flag, altLocation, shrapMod);
}
void UTIL_Explosion(int msg_dest, const float* pMsgOrigin, entvars_t* pev, const Vector& location, short sprite, float size, int framerate, int flag, const Vector& altLocation, float shrapMod) {
	UTIL_Explosion(msg_dest, pMsgOrigin, NULL, pev, location, 0.0f, 0.0f, 0.0f, sprite, size, framerate, flag, altLocation, shrapMod);
}
void UTIL_Explosion(int msg_dest, const float* pMsgOrigin, edict_t* ed, entvars_t* pev, const Vector& location, short sprite, float size, int framerate, int flag, const Vector& altLocation, float shrapMod) {
	UTIL_Explosion(msg_dest, pMsgOrigin, ed, pev, location, 0.0f, 0.0f, 0.0f, sprite, size, framerate, flag, altLocation, shrapMod);
}



void UTIL_Explosion(int msg_dest, const Vector &location, float offsetx, float offsety, float offsetz, short sprite, float size, int framerate, int flag, const Vector& altLocation){
	UTIL_Explosion(msg_dest, NULL, NULL, NULL, location, offsetx, offsety, offsetz, sprite, size, framerate, flag, altLocation, 1);
}
void UTIL_Explosion(int msg_dest, const float* pMsgOrigin, const Vector& location, float offsetx, float offsety, float offsetz, short sprite, float size, int framerate, int flag, const Vector& altLocation) {
	UTIL_Explosion(msg_dest, pMsgOrigin, NULL, NULL, location, offsetx, offsety, offsetz, sprite, size, framerate, flag, altLocation, 1);
}
void UTIL_Explosion(int msg_dest, const float* pMsgOrigin, edict_t* ed, const Vector& location, float offsetx, float offsety, float offsetz, short sprite, float size, int framerate, int flag, const Vector& altLocation) {
	UTIL_Explosion(msg_dest, pMsgOrigin, ed, NULL, location, offsetx, offsety, offsetz, sprite, size, framerate, flag, altLocation, 1);
}


void UTIL_Explosion(int msg_dest, entvars_t* pev, const Vector& location, float offsetx, float offsety, float offsetz, short sprite, float size, int framerate, int flag, const Vector& altLocation) {
	UTIL_Explosion(msg_dest, NULL, NULL, pev, location, offsetx, offsety, offsetz, sprite, size, framerate, flag, altLocation, 1);
}
void UTIL_Explosion(int msg_dest, const float* pMsgOrigin, entvars_t* pev, const Vector& location, float offsetx, float offsety, float offsetz, short sprite, float size, int framerate, int flag, const Vector& altLocation) {
	UTIL_Explosion(msg_dest, pMsgOrigin, NULL, pev, location, offsetx, offsety, offsetz, sprite, size, framerate, flag, altLocation, 1);
}
void UTIL_Explosion(int msg_dest, const float* pMsgOrigin, edict_t* ed, entvars_t* pev, const Vector &location, float offsetx, float offsety, float offsetz, short sprite, float size, int framerate, int flag, const Vector& altLocation){
	UTIL_Explosion(msg_dest, pMsgOrigin, ed, pev, location, offsetx, offsety, offsetz, sprite, size, framerate, flag, altLocation, 1);
}



void UTIL_Explosion(int msg_dest, entvars_t* pev, const Vector& location, float offsetx, float offsety, float offsetz, short sprite, float size, int framerate, int flag, const Vector& altLocation, float shrapMod) {
	UTIL_Explosion(msg_dest, NULL, NULL, pev, location, offsetx, offsety, offsetz, sprite, size, framerate, flag, altLocation, shrapMod);
}
void UTIL_Explosion(int msg_dest, const float* pMsgOrigin, entvars_t* pev, const Vector& location, float offsetx, float offsety, float offsetz, short sprite, float size, int framerate, int flag, const Vector& altLocation, float shrapMod) {
	UTIL_Explosion(msg_dest, pMsgOrigin, NULL, pev, location, offsetx, offsety, offsetz, sprite, size, framerate, flag, altLocation, shrapMod);
}

// 
void UTIL_Explosion(int msg_dest, const float* pMsgOrigin, edict_t * ed, entvars_t* pev, const Vector &location, float offsetx, float offsety, float offsetz, short sprite, float size, int framerate, int flag, const Vector& altLocation, float shrapMod){
	//easyPrintLine("ENTVARS_COUNT? : %d", ENTVARS_COUNT);


	/*
	MESSAGE_BEGIN(msg_dest, SVC_TEMPENTITY, location, ed);
		WRITE_BYTE(TE_EXPLOSION);		// This makes a dynamic light and the explosion sprites/sound
		WRITE_COORD(location.x + offsetx);	// Send to PAS because of the sound
		WRITE_COORD(location.y + offsety);
		WRITE_COORD(location.z + offsetz);
		WRITE_SHORT(sprite);
		WRITE_BYTE((int)size); // scale * 10
		WRITE_BYTE(framerate); // framerate
		WRITE_BYTE(flag);
	MESSAGE_END();
	*/

	// How about a shake?
	// size may come in as '30' in a typical explosion (see ggrenade.cpp's UTIL_Explosion call)
	// basing moreso on the garg shake's stats.  higher amp, less freq.  Shorter duration though.

	if(EASY_CVAR_GET(sv_explosion_shake) == 1){
		// size * 12?
		UTIL_ScreenShake( pev->origin, 11.0 * EASY_CVAR_GET_DEBUGONLY(sv_explosion_shake_amp), 90.0 * EASY_CVAR_GET_DEBUGONLY(sv_explosion_shake_freq), 0.65 * EASY_CVAR_GET_DEBUGONLY(sv_explosion_shake_duration), size * 24 * EASY_CVAR_GET_DEBUGONLY(sv_explosion_shake_range) );
	}
	// retail egon shake:
	//UTIL_ScreenShake( tr.vecEndPos, 5.0, 150.0, 0.75, 250.0 );

	
	if(EASY_CVAR_GET(cl_explosion) == 0){
		MESSAGE_BEGIN(msg_dest, SVC_TEMPENTITY, pMsgOrigin, ed );
			WRITE_BYTE( TE_EXPLOSION );		// This makes a dynamic light and the explosion sprites/sound
			WRITE_COORD( location.x + offsetx);	// Send to PAS because of the sound
			WRITE_COORD( location.y + offsety);
			WRITE_COORD( location.z + offsetz);
			WRITE_SHORT( sprite );
			// NOTICE - any calls from retail to make explosions already factor in the 10-times amount, don't do that here.
			WRITE_BYTE( (int)size  ); // scale * 10
			WRITE_BYTE( framerate  ); // framerate
			WRITE_BYTE( flag );
		MESSAGE_END();
	}else{
		UTIL_QuakeExplosion(msg_dest, pMsgOrigin, ed, pev, altLocation, offsetx, offsety, offsetz, shrapMod);
	}//END OF cl_explosion check
	

}//END OF UTIL_Explosion



void UTIL_QuakeExplosion(int msg_dest, entvars_t* pev, const Vector& location, float offsetx, float offsety, float offsetz, float shrapMod) {
	UTIL_QuakeExplosion(msg_dest, NULL, NULL, pev, location, offsetx, offsety, offsetz, shrapMod);
}
void UTIL_QuakeExplosion(int msg_dest, const float* pMsgOrigin, entvars_t* pev, const Vector& location, float offsetx, float offsety, float offsetz, float shrapMod) {
	UTIL_QuakeExplosion(msg_dest, pMsgOrigin, NULL, pev, location, offsetx, offsety, offsetz, shrapMod);
}

// do not call directly outside of utility files!
// UTIL_Explosion and some others call this if cl_explosion says it should be.
// And so far we're ignoring the 'offset' coords, but they're stil given in case 
// that changes.
void UTIL_QuakeExplosion(int msg_dest, const float* pMsgOrigin, edict_t* ed, entvars_t* pev, const Vector& location, float offsetx, float offsety, float offsetz, float shrapMod) {
	// never called serverside..?

	//MODDD - TEST!!!
	//PLAYBACK_EVENT_FULL (FEV_GLOBAL, NULL, g_quakeExplosionEffect, 0.0, 
	//(float *)&altLocation, (float *)&Vector(0,0,0), 0.0, 0.0, 0, 0, FALSE, FALSE);

	PLAYBACK_EVENT_FULL(FEV_GLOBAL, NULL, g_quakeExplosionEffect, 0.0,
		(float*)&location, (float*)&Vector(0, 0, 0), EASY_CVAR_GET_DEBUGONLY(shrapRand), EASY_CVAR_GET_DEBUGONLY(shrapRandHeightExtra), (int)EASY_CVAR_GET_DEBUGONLY(shrapMode), (int)(EASY_CVAR_GET_DEBUGONLY(explosionShrapnelMulti) * 14 * shrapMod), FALSE, FALSE);
	
	//MODDD TODO - these play the sound at the exploding entity's location, not necesarily at the sent origin.
	//Should we use the AMBIENT sound player to play the sound at a specific origin? Would that be ok, no side effects?
	//Example: This section was found in TEXTURETYPE_PlaySound, serverside:
	//    UTIL_EmitAmbientSound(ENT(0), ptr->vecEndPos, rgsz[RANDOM_LONG(0,cnt-1)], fvol, fattn, 0, 96 + RANDOM_LONG(0,0xf));
	//    //EMIT_SOUND_DYN( ENT(m_pPlayer->pev), CHAN_WEAPON, rgsz[RANDOM_LONG(0,cnt-1)], fvol, ATTN_NORM, 0, 96 + RANDOM_LONG(0,0xf));
	//...2nd line commented out, or rough equivalent of the first Ambient version to give a more specific location.

	// ALSO, this is often playing on top of a relatively quiet sound, 'weapon/debris#.wav', as 
	// seen in ggrenade.cpp, look for explosionDebrisSoundVolume.
	if (EASY_CVAR_GET_DEBUGONLY(quakeExplosionSound) == 1) {
		if (pev != NULL) {
			//UTIL_PlaySound(ENT(pev), CHAN_WEAPON, "!old_explode0", 0.7, ATTN_NORM);

			//MODDD - NOTE: used to use "CHAN_WEAPON".  changed to "CHAN_ITEM" so that it gets priority over mass sounds coming from "CHAN_VOICE" (tested with lots of headcrab deaths).
			switch (RANDOM_LONG(0, 2)) {
			case 0:
				UTIL_PlaySound(ENT(pev), CHAN_ITEM, "old/explode0.wav", 1.00, ATTN_NORM);
				break;
			case 1:
				UTIL_PlaySound(ENT(pev), CHAN_ITEM, "old/explode1.wav", 1.00, ATTN_NORM);
				break;
			case 2:
				UTIL_PlaySound(ENT(pev), CHAN_ITEM, "old/explode2.wav", 1.00, ATTN_NORM);
				break;
			}

		}
	}//END OF if(EASY_CVAR_GET_DEBUGONLY(quakeExplosionSound) == 1)
	else if (EASY_CVAR_GET_DEBUGONLY(quakeExplosionSound) == 2) {
		if (pev != NULL) {
			//UTIL_PlaySound(ENT(pev), CHAN_WEAPON, "!old_explode0", 0.7, ATTN_NORM);

			//MODDD - NOTE: used to use "CHAN_WEAPON".  changed to "CHAN_ITEM" so that it gets priority over mass sounds coming from "CHAN_VOICE" (tested with lots of headcrab deaths).
			switch (RANDOM_LONG(0, 2)) {
			case 0:
				UTIL_PlaySound(ENT(pev), CHAN_ITEM, "weapons/explode3.wav", 1.00, ATTN_NORM);
				break;
			case 1:
				UTIL_PlaySound(ENT(pev), CHAN_ITEM, "weapons/explode4.wav", 1.00, ATTN_NORM);
				break;
			case 2:
				UTIL_PlaySound(ENT(pev), CHAN_ITEM, "weapons/explode5.wav", 1.00, ATTN_NORM);
				break;
			}

		}

	}//END OF quake explosion sound checks.

}//END OF UTIL_QuakeExplosion




void UTIL_SpriteOrQuakeExplosion(int msg_dest, entvars_t* pev, const Vector& location, float offsetx, float offsety, float offsetz, short sprite, float size, int brightness, const Vector& altLocation, float shrapMod) {
	UTIL_SpriteOrQuakeExplosion(msg_dest, NULL, NULL, pev, location, offsetx, offsety, offsetz, sprite, size, brightness, altLocation, shrapMod);
}
void UTIL_SpriteOrQuakeExplosion(int msg_dest, const float* pMsgOrigin, entvars_t* pev, const Vector& location, float offsetx, float offsety, float offsetz, short sprite, float size, int brightness, const Vector& altLocation, float shrapMod) {
	UTIL_SpriteOrQuakeExplosion(msg_dest, pMsgOrigin, NULL, pev, location, offsetx, offsety, offsetz, sprite, size, brightness, altLocation, shrapMod);
}
// Decision based off cl_explosion:
// 0: generate a sprite effect (like a more custom explosion graphic), or...
// 1: use the same quake explosion as most other places in that case.
void UTIL_SpriteOrQuakeExplosion(int msg_dest, const float* pMsgOrigin, edict_t* ed, entvars_t* pev, const Vector& location, float offsetx, float offsety, float offsetz, short sprite, float size, int brightness, const Vector& altLocation, float shrapMod) {

	if (EASY_CVAR_GET(cl_explosion) == 0) {
		MESSAGE_BEGIN(msg_dest, SVC_TEMPENTITY, pMsgOrigin, ed);
			WRITE_BYTE(TE_SPRITE);
			WRITE_COORD(location.x + offsetx);
			WRITE_COORD(location.y + offsety);
			WRITE_COORD(location.z + offsetz);
			WRITE_SHORT(sprite);
			WRITE_BYTE(size); // scale * 10
			WRITE_BYTE(brightness); // brightness
		MESSAGE_END();
	}else {
		//UTIL_Explosion(pev, location, offsetx, offsety, offsetz, sprite, size, NO_FRAMERATE, NO_FLAG, ONLY_ALT_LOC, shrapMod);

		// Copy that works for cl_explosion of 0.
		UTIL_QuakeExplosion(msg_dest, pMsgOrigin, ed, pev, altLocation, offsetx, offsety, offsetz, shrapMod);
	}

}// UTIL_SpriteOrQuakeExplosion




// Runs regardless of cl_explosion.
void UTIL_Smoke(int msg_dest, const Vector& location, short sprite, float scale, int framerate){
	UTIL_Smoke(msg_dest, NULL, NULL, location, 0, 0, 0, sprite, scale, framerate);
}
void UTIL_Smoke(int msg_dest, const float* pMsgOrigin, const Vector& location, short sprite, float scale, int framerate) {
	UTIL_Smoke(msg_dest, pMsgOrigin, NULL, location, 0, 0, 0, sprite, scale, framerate);
}
void UTIL_Smoke(int msg_dest, const float* pMsgOrigin, edict_t* ed, const Vector& location, short sprite, float scale, int framerate) {
	UTIL_Smoke(msg_dest, pMsgOrigin, ed, location, 0, 0, 0, sprite, scale, framerate);
}

void UTIL_Smoke(int msg_dest, const Vector& location, float offsetx, float offsety, float offsetz, short sprite, float scale, int framerate) {
	UTIL_Smoke(msg_dest, NULL, NULL, location, offsetx, offsety, offsetz, sprite, scale, framerate);
}
void UTIL_Smoke(int msg_dest, const float* pMsgOrigin, const Vector& location, float offsetx, float offsety, float offsetz, short sprite, float scale, int framerate) {
	UTIL_Smoke(msg_dest, pMsgOrigin, NULL, location, offsetx, offsety, offsetz, sprite, scale, framerate);
}
void UTIL_Smoke(int msg_dest, const float* pMsgOrigin, edict_t* ed, const Vector& location, float offsetx, float offsety, float offsetz, short sprite, float scale, int framerate){
	
	//return;  //TEST

	MESSAGE_BEGIN(msg_dest, SVC_TEMPENTITY, pMsgOrigin, ed);
		WRITE_BYTE( TE_SMOKE );
		WRITE_COORD( location.x + offsetx);
		WRITE_COORD( location.y + offsety);
		WRITE_COORD( location.z + offsetz);
		WRITE_SHORT( sprite );
		WRITE_BYTE( (int)scale ); // scale * 10
		WRITE_BYTE( framerate  ); // framerate
	MESSAGE_END();
}


// Same as UTIL_Smoke, but requires cl_explosion to be 0 or 2 to work
// (quake explosions don't involve the smoke effect by default).
void UTIL_ExplosionSmoke(int msg_dest, const Vector& location, short sprite, float scale, int framerate) {
	UTIL_ExplosionSmoke(msg_dest, NULL, NULL, location, 0, 0, 0, sprite, scale, framerate);
}
void UTIL_ExplosionSmoke(int msg_dest, const float* pMsgOrigin, const Vector& location, short sprite, float scale, int framerate) {
	UTIL_ExplosionSmoke(msg_dest, pMsgOrigin, NULL, location, 0, 0, 0, sprite, scale, framerate);
}
void UTIL_ExplosionSmoke(int msg_dest, const float* pMsgOrigin, edict_t* ed, const Vector& location, short sprite, float scale, int framerate) {
	UTIL_ExplosionSmoke(msg_dest, pMsgOrigin, ed, location, 0, 0, 0, sprite, scale, framerate);
}


void UTIL_ExplosionSmoke(int msg_dest, const Vector& location, float offsetx, float offsety, float offsetz, short sprite, float scale, int framerate) {
	UTIL_ExplosionSmoke(msg_dest, NULL, NULL, location, offsetx, offsety, offsetz, sprite, scale, framerate);
}
void UTIL_ExplosionSmoke(int msg_dest, const float* pMsgOrigin, const Vector & location, float offsetx, float offsety, float offsetz, short sprite, float scale, int framerate) {
	UTIL_ExplosionSmoke(msg_dest, pMsgOrigin, NULL, location, offsetx, offsety, offsetz, sprite, scale, framerate);
}
void UTIL_ExplosionSmoke(int msg_dest, const float* pMsgOrigin, edict_t* ed, const Vector& location, float offsetx, float offsety, float offsetz, short sprite, float scale, int framerate) {
	if (EASY_CVAR_GET(cl_explosion) != 1) {
		MESSAGE_BEGIN(msg_dest, SVC_TEMPENTITY, pMsgOrigin, ed);
			WRITE_BYTE(TE_SMOKE);
			WRITE_COORD(location.x + offsetx);
			WRITE_COORD(location.y + offsety);
			WRITE_COORD(location.z + offsetz);
			WRITE_SHORT(sprite);
			WRITE_BYTE((int)scale); // scale * 10
			WRITE_BYTE(framerate); // framerate
		MESSAGE_END();
	}
}



//MODDD - new
BOOL UTIL_getExplosionsHaveSparks(void){
	//Deprecated way.
	/*
	float explosionsHaveSparksVar = EASY_CVAR_GET(explosionsHaveSparks);
	//only alloy explosions if not using the quake explosion, and the "explosionsHaveSparks" var is 1 or 2.
	return  (EASY_CVAR_GET(cl_explosion) != 1 && (explosionsHaveSparksVar == 1 || explosionsHaveSparksVar == 2) ) ;
	*/
	//return  (EASY_CVAR_GET(cl_explosion) != 1 && (EASY_CVAR_GET_DEBUGONLY(explosionShrapnelMulti) > 0) ) ;

	//For an explosion to satisfy the condition of generating sparks, the "cl_explosion" var must be off (retail explosions only) and the sparks multiple (of 6) must be above 0.
	return  (EASY_CVAR_GET(cl_explosion) != 1 && EASY_CVAR_GET_DEBUGONLY(sparksExplosionMulti) > 0 && EASY_CVAR_GET_DEBUGONLY(sparksAllMulti) > 0) ;
}




//Note that this is most commonly used for the armor ricochet effect (hgrunt helmet non-fatal hits, agrunt plated armor).
void UTIL_Ricochet( const Vector& position, float scale )
{
	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(muteRicochetSound) < 2){
		MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, position );
			WRITE_BYTE( TE_ARMOR_RICOCHET );
			WRITE_COORD( position.x );
			WRITE_COORD( position.y );
			WRITE_COORD( position.z );
			WRITE_BYTE( (int)(scale*10) );
		MESSAGE_END();
	}
}

//MODDD - NEW.  From a snippet of AGrunt's script, may as well be re-usable.
// Also try sending ptr->vecPlaneNormal instead of -vecDir in calls to here like retail did.
// (that is 'negative' vecDir because here no longer makes the direction negative).
void UTIL_RicochetTracer(const Vector& position, const Vector& vecDir) {
	Vector vecTracerDir = vecDir;

	//MODDD - tightened a bit, was from -0.3 to 0.3
	vecTracerDir.x += RANDOM_FLOAT(-0.18, 0.18);
	vecTracerDir.y += RANDOM_FLOAT(-0.18, 0.18);
	vecTracerDir.z += RANDOM_FLOAT(-0.18, 0.18);

	// now normalize it after that fudging.
	vecTracerDir = vecTracerDir.Normalize();
	
	//MODDD - RETAIL BUG FIXED, TE_TRACER expects the end point exactly as the 2nd arg,
	// not direction or point relative to the start!
	// And starting pushed back a little in the opposite direction to boost the odds of seeing 
	// the ricochet effect if it happens while the player is close to the thing hit.
	// Oddly might not work though.
	Vector vecStart = position - (vecTracerDir * 80);
	Vector vecEnd = position + vecTracerDir * 650;   // was 512

	//DebugLine_Setup(0, vecStart, vecEnd, 255, 0, 255);

	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, position);
	WRITE_BYTE(TE_TRACER);
	WRITE_COORD(vecStart.x);
	WRITE_COORD(vecStart.y);
	WRITE_COORD(vecStart.z);
	
	WRITE_COORD(vecEnd.x);
	WRITE_COORD(vecEnd.y);
	WRITE_COORD(vecEnd.z);
	MESSAGE_END();
}



BOOL UTIL_TeamsMatch( const char *pTeamName1, const char *pTeamName2 )
{
	// Everyone matches unless it's teamplay
	if ( !g_pGameRules->IsTeamplay() )
		return TRUE;

	// Both on a team?
	if ( *pTeamName1 != 0 && *pTeamName2 != 0 )
	{
		if ( !stricmp( pTeamName1, pTeamName2 ) )	// Same Team?
			return TRUE;
	}

	return FALSE;
}


//MODDD - UTIL_WaterLevel, UTIL_Bubbles, UTIL_BubbleTrail moved to util_shared.h/cpp

void UTIL_Remove( CBaseEntity *pEntity )
{
	if ( !pEntity )
		return;

	pEntity->UpdateOnRemove();
	pEntity->pev->flags |= FL_KILLME;
	pEntity->pev->targetname = 0;
}


//MODDD
//also requires this entity to be finished with the death animation (DEAD_DEAD)
BOOL UTIL_IsDeadEntity( CBaseEntity* ent){

	if(!ent){
		return FALSE;
	}
	if(!ent->pev){
		return FALSE;
	}

	if(ent->pev->deadflag != DEAD_DEAD){
		//not good enough to say dead.
		return FALSE;
	}

	edict_t* pentAttempt = ENT(ent->pev);
	//note that the usual UTIL_IsValidEntity that accepts the edict_t (see below) already has a null check.
	/*
	if(pentAttempt == NULL){
		return FALSE;
	}
	*/
	//a valid, "dead" entity.
	return UTIL_IsValidEntity(pentAttempt);

}


//MODDD - same as IsValidEntity for Entity, but also has a pev->deadflag check.
//This is a literal check, not for the AI to be tricked or have a slightly delayed reaction to for things with the DEAD_DYING deadflag (death anim in progress).
//flag DEAD_NO only.
BOOL UTIL_IsAliveEntity( CBaseEntity* ent){

	if(!ent){
		return FALSE;
	}
	if(!ent->pev){
		return FALSE;
	}

	if(ent->pev->deadflag != DEAD_NO){
		//must be dead.
		return FALSE;
	}

	edict_t* pentAttempt = ENT(ent->pev);
	//note that the usual UTIL_IsValidEntity that accepts the edict_t (see below) already has a null check.
	/*
	if(pentAttempt == NULL){
		return FALSE;
	}
	*/
	return UTIL_IsValidEntity(pentAttempt);
}


//MODDD - slight extension.
BOOL UTIL_IsValidEntity( CBaseEntity* ent){

	if(!ent){
		return FALSE;
	}
	edict_t* pentAttempt = ENT(ent->pev);
	//note that the usual UTIL_IsValidEntity that accepts the edict_t (see below) already has a null check.
	/*
	if(pentAttempt == NULL){
		return FALSE;
	}
	*/
	return UTIL_IsValidEntity(pentAttempt);
}

BOOL UTIL_IsValidEntity( edict_t *pent )
{
	if ( !pent || pent->free || (pent->v.flags & FL_KILLME) )
		return FALSE;
	return TRUE;
}


void UTIL_PrecacheOther( const char *szClassname )
{
	edict_t	*pent;

	pent = CREATE_NAMED_ENTITY( MAKE_STRING( szClassname ) );
	if ( FNullEnt( pent ) )
	{
		ALERT ( at_console, "NULL Ent in UTIL_PrecacheOther\n" );
		return;
	}
	
	CBaseEntity *pEntity = CBaseEntity::Instance (VARS( pent ));
	if (pEntity)
		pEntity->Precache( );
	REMOVE_ENTITY(pent);
}

//=========================================================
// UTIL_LogPrintf - Prints a logged message to console.
// Preceded by LOG: ( timestamp ) < message >
//=========================================================
void UTIL_LogPrintf( char *fmt, ... )
{
	va_list			argptr;
	static char arychr_buffer[1024];
	
	va_start ( argptr, fmt );
	vsprintf ( arychr_buffer, fmt, argptr );
	va_end   ( argptr );

	// Print to server console
	ALERT( at_logged, "%s", arychr_buffer );
}

//=========================================================
// UTIL_DotPoints - returns the dot product of a line from
// src to check and vecdir.
//=========================================================
float UTIL_DotPoints ( const Vector &vecSrc, const Vector &vecCheck, const Vector &vecDir )
{
	Vector2D	vec2LOS;

	vec2LOS = ( vecCheck - vecSrc ).Make2D();
	vec2LOS = vec2LOS.Normalize();

	return DotProduct (vec2LOS , ( vecDir.Make2D() ) );
}


//=========================================================
// UTIL_StripToken - for redundant keynames
//=========================================================
void UTIL_StripToken( const char *pKey, char *pDest )
{
	int i = 0;

	while ( pKey[i] && pKey[i] != '#' )
	{
		pDest[i] = pKey[i];
		i++;
	}
	pDest[i] = 0;
}


//MODDD - SetMovedir and VecBModelOrigin moved here from subs.cpp and bmodels.cpp.
/*
QuakeEd only writes a single float for angles (bad idea), so up and down are
just constant angles.
*/
void SetMovedir( entvars_t *pev )
{
	if (pev->angles == Vector(0, -1, 0))
	{
		pev->movedir = Vector(0, 0, 1);
	}
	else if (pev->angles == Vector(0, -2, 0))
	{
		pev->movedir = Vector(0, 0, -1);
	}
	else
	{
		UTIL_MakeVectors(pev->angles);
		pev->movedir = gpGlobals->v_forward;
	}
	
	pev->angles = g_vecZero;
}

Vector VecBModelOrigin( entvars_t* pevBModel )
{
	return pevBModel->absmin + ( pevBModel->size * 0.5 );
}



// --------------------------------------------------------------
//
// CSave
//
// --------------------------------------------------------------
static int gSizes[FIELD_TYPECOUNT] = 
{
	sizeof(float),		// FIELD_FLOAT
	sizeof(int),		// FIELD_STRING
	sizeof(int),		// FIELD_ENTITY
	sizeof(int),		// FIELD_CLASSPTR
	sizeof(int),		// FIELD_EHANDLE
	sizeof(int),		// FIELD_entvars_t
	sizeof(int),		// FIELD_EDICT
	sizeof(float)*3,	// FIELD_VECTOR
	sizeof(float)*3,	// FIELD_POSITION_VECTOR
	sizeof(int *),		// FIELD_POINTER
	sizeof(int),		// FIELD_INTEGER
	sizeof(int *),		// FIELD_FUNCTION
	sizeof(int),		// FIELD_BOOLEAN
	sizeof(short),		// FIELD_SHORT
	sizeof(char),		// FIELD_CHARACTER
	sizeof(float),		// FIELD_TIME
	sizeof(int),		// FIELD_MODELNAME
	sizeof(int),		// FIELD_SOUNDNAME
};


// Base class includes common SAVERESTOREDATA pointer, and manages the entity table
CSaveRestoreBuffer::CSaveRestoreBuffer( void )
{
	m_pdata = NULL;
}

CSaveRestoreBuffer::CSaveRestoreBuffer( SAVERESTOREDATA *pdata )
{
	m_pdata = pdata;
}

CSaveRestoreBuffer::~CSaveRestoreBuffer( void )
{
}

int CSaveRestoreBuffer::EntityIndex( CBaseEntity *pEntity )
{
	if ( pEntity == NULL )
		return -1;
	return EntityIndex( pEntity->pev );
}

int CSaveRestoreBuffer::EntityIndex( entvars_t *pevLookup )
{
	if ( pevLookup == NULL )
		return -1;
	return EntityIndex( ENT( pevLookup ) );
}

int CSaveRestoreBuffer::EntityIndex( EOFFSET eoLookup )
{
	return EntityIndex( ENT( eoLookup ) );
}


int CSaveRestoreBuffer::EntityIndex( edict_t *pentLookup )
{
	if ( !m_pdata || pentLookup == NULL )
		return -1;

	int i;
	ENTITYTABLE *pTable;

	for ( i = 0; i < m_pdata->tableCount; i++ )
	{
		pTable = m_pdata->pTable + i;
		if ( pTable->pent == pentLookup )
			return i;
	}
	return -1;
}

edict_t *CSaveRestoreBuffer::EntityFromIndex( int entityIndex )
{
	if ( !m_pdata || entityIndex < 0 )
		return NULL;

	int i;
	ENTITYTABLE *pTable;

	for ( i = 0; i < m_pdata->tableCount; i++ )
	{
		pTable = m_pdata->pTable + i;
		if ( pTable->id == entityIndex )
			return pTable->pent;
	}
	return NULL;
}

int CSaveRestoreBuffer::EntityFlagsSet( int entityIndex, int flags )
{
	if ( !m_pdata || entityIndex < 0 )
		return 0;
	if ( entityIndex > m_pdata->tableCount )
		return 0;

	m_pdata->pTable[ entityIndex ].flags |= flags;

	return m_pdata->pTable[ entityIndex ].flags;
}


void CSaveRestoreBuffer::BufferRewind( int size )
{
	if ( !m_pdata )
		return;

	if ( m_pdata->size < size )
		size = m_pdata->size;

	m_pdata->pCurrentData -= size;
	m_pdata->size -= size;
}

#ifndef _WIN32
extern "C" {
unsigned _rotr ( unsigned val, int shift)
{
        register unsigned lobit;        /* non-zero means lo bit set */
        register unsigned num = val;    /* number to rotate */

        shift &= 0x1f;                  /* modulo 32 -- this will also make
                                           negative shifts work */

        while (shift--) {
                lobit = num & 1;        /* get high bit */
                num >>= 1;              /* shift right one bit */
                if (lobit)
                        num |= 0x80000000;  /* set hi bit if lo bit was set */
        }

        return num;
}
}
#endif

unsigned int CSaveRestoreBuffer::HashString( const char *pszToken )
{
	unsigned int hash = 0;

	while ( *pszToken )
		hash = _rotr( hash, 4 ) ^ *pszToken++;

	return hash;
}

unsigned short CSaveRestoreBuffer::TokenHash( const char *pszToken )
{
	unsigned short	hash = (unsigned short)(HashString( pszToken ) % (unsigned)m_pdata->tokenCount );
	
#if _DEBUG
	static int tokensparsed = 0;
	tokensparsed++;
	if ( !m_pdata->tokenCount || !m_pdata->pTokens )
		ALERT( at_error, "No token table array in TokenHash()!" );
#endif

	for ( int i=0; i<m_pdata->tokenCount; i++ )
	{
#if _DEBUG
		static qboolean beentheredonethat = FALSE;
		if ( i > 50 && !beentheredonethat )
		{
			beentheredonethat = TRUE;
			ALERT( at_error, "CSaveRestoreBuffer::TokenHash() is getting too full!" );
		}
#endif

		int index = hash + i;
		if ( index >= m_pdata->tokenCount )
			index -= m_pdata->tokenCount;

		if ( !m_pdata->pTokens[index] || strcmp( pszToken, m_pdata->pTokens[index] ) == 0 )
		{
			m_pdata->pTokens[index] = (char *)pszToken;
			return index;
		}
	}
		
	// Token hash table full!!! 
	// [Consider doing overflow table(s) after the main table & limiting linear hash table search]
	ALERT( at_error, "CSaveRestoreBuffer::TokenHash() is COMPLETELY FULL!" );
	return 0;
}

void CSave::WriteData( const char *pname, int size, const char *pdata )
{
	BufferField( pname, size, pdata );
}


void CSave::WriteShort( const char *pname, const short *data, int count )
{
	BufferField( pname, sizeof(short) * count, (const char *)data );
}


void CSave::WriteInt( const char *pname, const int *data, int count )
{
	BufferField( pname, sizeof(int) * count, (const char *)data );
}


void CSave::WriteFloat( const char *pname, const float *data, int count )
{
	BufferField( pname, sizeof(float) * count, (const char *)data );
}


void CSave::WriteTime( const char *pname, const float *data, int count )
{
	int i;
	//MODDD - what devs?   Why 'Vector tmp;'?
	//And 'Vector input;' was completely unused too.  Whoops.

	BufferHeader( pname, sizeof(float) * count );
	for ( i = 0; i < count; i++ )
	{
		float tmp = data[0];

		// Always encode time as a delta from the current time so it can be re-based if loaded in a new level
		// Times of 0 are never written to the file, so they will be restored as 0, not a relative time
		if ( m_pdata )
			tmp -= m_pdata->time;

		BufferData( (const char *)&tmp, sizeof(float) );
		data ++;
	}
}


void CSave::WriteString( const char *pname, const char *pdata )
{
#ifdef TOKENIZE
	short	token = (short)TokenHash( pdata );
	WriteShort( pname, &token, 1 );
#else
	BufferField( pname, strlen(pdata) + 1, pdata );
#endif
}


void CSave::WriteString( const char *pname, const int *stringId, int count )
{
	int i, size;

#ifdef TOKENIZE
	short	token = (short)TokenHash( STRING( *stringId ) );
	WriteShort( pname, &token, 1 );
#else
#if 0
	if ( count != 1 )
		ALERT( at_error, "No string arrays!\n" );
	WriteString( pname, (char *)STRING(*stringId) );
#endif

	size = 0;
	for ( i = 0; i < count; i++ )
		size += strlen( STRING( stringId[i] ) ) + 1;

	BufferHeader( pname, size );
	for ( i = 0; i < count; i++ )
	{
		const char *pString = STRING(stringId[i]);
		BufferData( pString, strlen(pString)+1 );
	}
#endif
}


void CSave::WriteVector( const char *pname, const Vector &value )
{
	WriteVector( pname, &value.x, 1 );
}


void CSave::WriteVector( const char *pname, const float *value, int count )
{
	BufferHeader( pname, sizeof(float) * 3 * count );
	BufferData( (const char *)value, sizeof(float) * 3 * count );
}



void CSave::WritePositionVector( const char *pname, const Vector &value )
{

	if ( m_pdata && m_pdata->fUseLandmark )
	{
		Vector tmp = value - m_pdata->vecLandmarkOffset;
		WriteVector( pname, tmp );
	}

	WriteVector( pname, value );
}


void CSave::WritePositionVector( const char *pname, const float *value, int count )
{
	int i;
	//MODDD - what devs?   Why 'Vector tmp;'?
	//And 'Vector input;' was completely unused too.  Whoops.

	BufferHeader( pname, sizeof(float) * 3 * count );
	for ( i = 0; i < count; i++ )
	{
		Vector tmp( value[0], value[1], value[2] );

		if ( m_pdata && m_pdata->fUseLandmark )
			tmp = tmp - m_pdata->vecLandmarkOffset;

		BufferData( (const char *)&tmp.x, sizeof(float) * 3 );
		value += 3;
	}
}


void CSave::WriteFunction( const char *pname, const int *data, int count )
{
	const char *functionName;

	functionName = NAME_FOR_FUNCTION( *data );
	if ( functionName )
		BufferField( pname, strlen(functionName) + 1, functionName );
	else
		ALERT( at_error, "Invalid function pointer in entity!" );
}


void EntvarsKeyvalue( entvars_t *pev, KeyValueData *pkvd )
{
	int i;
	TYPEDESCRIPTION		*pField;

	for ( i = 0; i < ENTVARS_COUNT; i++ )
	{
		pField = &gEntvarsDescription[i];

		if ( !stricmp( pField->fieldName, pkvd->szKeyName ) )
		{
			switch( pField->fieldType )
			{
			case FIELD_MODELNAME:
			case FIELD_SOUNDNAME:
			case FIELD_STRING:
				(*(int *)((char *)pev + pField->fieldOffset)) = ALLOC_STRING( pkvd->szValue );
				break;

			case FIELD_TIME:
			case FIELD_FLOAT:
				(*(float *)((char *)pev + pField->fieldOffset)) = atof( pkvd->szValue );
				break;

			case FIELD_INTEGER:
				(*(int *)((char *)pev + pField->fieldOffset)) = atoi( pkvd->szValue );
				break;

			case FIELD_POSITION_VECTOR:
			case FIELD_VECTOR:
				UTIL_StringToVector( (float *)((char *)pev + pField->fieldOffset), pkvd->szValue );
				break;

			default:
			case FIELD_EVARS:
			case FIELD_CLASSPTR:
			case FIELD_EDICT:
			case FIELD_ENTITY:
			case FIELD_POINTER:
				ALERT( at_error, "Bad field in entity!!\n" );
				break;
			}
			pkvd->fHandled = TRUE;
			return;
		}
	}
}



int CSave::WriteEntVars( const char *pname, entvars_t *pev )
{
	return WriteFields( pname, pev, gEntvarsDescription, ENTVARS_COUNT );
}



int CSave::WriteFields( const char *pname, void *pBaseData, TYPEDESCRIPTION *pFields, int fieldCount )
{
	int			i, j, actualCount, emptyCount;
	TYPEDESCRIPTION	*pTest;
	int			entityArray[MAX_ENTITYARRAY];

	// Precalculate the number of empty fields
	emptyCount = 0;
	for ( i = 0; i < fieldCount; i++ )
	{
		void *pOutputData;
		pOutputData = ((char *)pBaseData + pFields[i].fieldOffset );
		if ( DataEmpty( (const char *)pOutputData, pFields[i].fieldSize * gSizes[pFields[i].fieldType] ) )
			emptyCount++;
	}

	// Empty fields will not be written, write out the actual number of fields to be written
	actualCount = fieldCount - emptyCount;
	WriteInt( pname, &actualCount, 1 );

	for ( i = 0; i < fieldCount; i++ )
	{
		void *pOutputData;
		pTest = &pFields[ i ];
		pOutputData = ((char *)pBaseData + pTest->fieldOffset );

		// UNDONE: Must we do this twice?
		if ( DataEmpty( (const char *)pOutputData, pTest->fieldSize * gSizes[pTest->fieldType] ) )
			continue;

		switch( pTest->fieldType )
		{
		case FIELD_FLOAT:
			WriteFloat( pTest->fieldName, (float *)pOutputData, pTest->fieldSize );
		break;
		case FIELD_TIME:
			WriteTime( pTest->fieldName, (float *)pOutputData, pTest->fieldSize );
		break;
		case FIELD_MODELNAME:
		case FIELD_SOUNDNAME:
		case FIELD_STRING:
			WriteString( pTest->fieldName, (int *)pOutputData, pTest->fieldSize );
		break;
		case FIELD_CLASSPTR:
		case FIELD_EVARS:
		case FIELD_EDICT:
		case FIELD_ENTITY:
		case FIELD_EHANDLE:
			if ( pTest->fieldSize > MAX_ENTITYARRAY )
				ALERT( at_error, "Can't save more than %d entities in an array!!!\n", MAX_ENTITYARRAY );
			for ( j = 0; j < pTest->fieldSize; j++ )
			{
				switch( pTest->fieldType )
				{
					case FIELD_EVARS:
						entityArray[j] = EntityIndex( ((entvars_t **)pOutputData)[j] );
					break;
					case FIELD_CLASSPTR:
						entityArray[j] = EntityIndex( ((CBaseEntity **)pOutputData)[j] );
					break;
					case FIELD_EDICT:
						entityArray[j] = EntityIndex( ((edict_t **)pOutputData)[j] );
					break;
					case FIELD_ENTITY:
						entityArray[j] = EntityIndex( ((EOFFSET *)pOutputData)[j] );
					break;
					case FIELD_EHANDLE:
						entityArray[j] = EntityIndex( (CBaseEntity *)(((EHANDLE *)pOutputData)[j]) );
					break;
				}
			}
			WriteInt( pTest->fieldName, entityArray, pTest->fieldSize );
		break;
		case FIELD_POSITION_VECTOR:
			WritePositionVector( pTest->fieldName, (float *)pOutputData, pTest->fieldSize );
		break;
		case FIELD_VECTOR:
			WriteVector( pTest->fieldName, (float *)pOutputData, pTest->fieldSize );
		break;

		case FIELD_BOOLEAN:
		case FIELD_INTEGER:
			WriteInt( pTest->fieldName, (int *)pOutputData, pTest->fieldSize );
		break;

		case FIELD_SHORT:
			WriteData( pTest->fieldName, 2 * pTest->fieldSize, ((char *)pOutputData) );
		break;

		case FIELD_CHARACTER:
			WriteData( pTest->fieldName, pTest->fieldSize, ((char *)pOutputData) );
		break;

		// For now, just write the address out, we're not going to change memory while doing this yet!
		case FIELD_POINTER:
			WriteInt( pTest->fieldName, (int *)(char *)pOutputData, pTest->fieldSize );
		break;

		case FIELD_FUNCTION:
			WriteFunction( pTest->fieldName, (int *)(char *)pOutputData, pTest->fieldSize );
		break;
		default:
			ALERT( at_error, "Bad field type\n" );
		}
	}

	return 1;
}


void CSave::BufferString( char *pdata, int len )
{
	char c = 0;

	BufferData( pdata, len );		// Write the string
	BufferData( &c, 1 );			// Write a null terminator
}


int CSave::DataEmpty( const char *pdata, int size )
{
	for ( int i = 0; i < size; i++ )
	{
		if ( pdata[i] )
			return 0;
	}
	return 1;
}


void CSave::BufferField( const char *pname, int size, const char *pdata )
{
	BufferHeader( pname, size );
	BufferData( pdata, size );
}


void CSave::BufferHeader( const char *pname, int size )
{
	short	hashvalue = TokenHash( pname );
	if ( size > 1<<(sizeof(short)*8) )
		ALERT( at_error, "CSave::BufferHeader() size parameter exceeds 'short'!" );
	BufferData( (const char *)&size, sizeof(short) );
	BufferData( (const char *)&hashvalue, sizeof(short) );
}


void CSave::BufferData( const char *pdata, int size )
{
	if ( !m_pdata )
		return;

	if ( m_pdata->size + size > m_pdata->bufferSize )
	{
		ALERT( at_error, "Save/Restore overflow!" );
		m_pdata->size = m_pdata->bufferSize;
		return;
	}

	memcpy( m_pdata->pCurrentData, pdata, size );
	m_pdata->pCurrentData += size;
	m_pdata->size += size;
}


// --------------------------------------------------------------
//
// CRestore
//
// --------------------------------------------------------------

int CRestore::ReadField( void *pBaseData, TYPEDESCRIPTION *pFields, int fieldCount, int startField, int size, char *pName, void *pData )
{
	int i, j, stringCount, fieldNumber, entityIndex;
	TYPEDESCRIPTION *pTest;
	float time, timeData;
	Vector	position;
	edict_t	*pent;
	char	*pString;

	time = 0;
	position = Vector(0,0,0);

	if ( m_pdata )
	{
		time = m_pdata->time;
		if ( m_pdata->fUseLandmark )
			position = m_pdata->vecLandmarkOffset;
	}

	for ( i = 0; i < fieldCount; i++ )
	{
		fieldNumber = (i+startField)%fieldCount;
		pTest = &pFields[ fieldNumber ];
		if ( !stricmp( pTest->fieldName, pName ) )
		{
			if ( !m_global || !(pTest->flags & FTYPEDESC_GLOBAL) )
			{
				for ( j = 0; j < pTest->fieldSize; j++ )
				{
					void *pOutputData = ((char *)pBaseData + pTest->fieldOffset + (j*gSizes[pTest->fieldType]) );
					void *pInputData = (char *)pData + j * gSizes[pTest->fieldType];

					switch( pTest->fieldType )
					{
					case FIELD_TIME:
						timeData = *(float *)pInputData;
						// Re-base time variables
						timeData += time;
						*((float *)pOutputData) = timeData;
					break;
					case FIELD_FLOAT:
						*((float *)pOutputData) = *(float *)pInputData;
					break;
					case FIELD_MODELNAME:
					case FIELD_SOUNDNAME:
					case FIELD_STRING:
						// Skip over j strings
						pString = (char *)pData;
						for ( stringCount = 0; stringCount < j; stringCount++ )
						{
							while (*pString)
								pString++;
							pString++;
						}
						pInputData = pString;
						if ( strlen( (char *)pInputData ) == 0 )
							*((int *)pOutputData) = 0;
						else
						{
							int string;

							string = ALLOC_STRING( (char *)pInputData );
							
							*((int *)pOutputData) = string;

							if ( !FStringNull( string ) && m_precache )
							{
								if ( pTest->fieldType == FIELD_MODELNAME )
									PRECACHE_MODEL( (char *)STRING( string ) );
								else if ( pTest->fieldType == FIELD_SOUNDNAME )
									PRECACHE_SOUND( (char *)STRING( string ) );
							}
						}
					break;
					case FIELD_EVARS:
						entityIndex = *( int *)pInputData;
						pent = EntityFromIndex( entityIndex );
						if ( pent )
							*((entvars_t **)pOutputData) = VARS(pent);
						else
							*((entvars_t **)pOutputData) = NULL;
					break;
					case FIELD_CLASSPTR:
						entityIndex = *( int *)pInputData;
						pent = EntityFromIndex( entityIndex );
						if ( pent )
							*((CBaseEntity **)pOutputData) = CBaseEntity::Instance(pent);
						else
							*((CBaseEntity **)pOutputData) = NULL;
					break;
					case FIELD_EDICT:
						entityIndex = *( int *)pInputData;
						pent = EntityFromIndex( entityIndex );
						*((edict_t **)pOutputData) = pent;
					break;
					case FIELD_EHANDLE:
						// Input and Output sizes are different!
						pOutputData = (char *)pOutputData + j*(sizeof(EHANDLE) - gSizes[pTest->fieldType]);
						entityIndex = *( int *)pInputData;
						pent = EntityFromIndex( entityIndex );
						if ( pent )
							*((EHANDLE *)pOutputData) = CBaseEntity::Instance(pent);
						else
							*((EHANDLE *)pOutputData) = NULL;
					break;
					case FIELD_ENTITY:
						entityIndex = *( int *)pInputData;
						pent = EntityFromIndex( entityIndex );
						if ( pent )
							*((EOFFSET *)pOutputData) = OFFSET(pent);
						else
							*((EOFFSET *)pOutputData) = 0;
					break;
					case FIELD_VECTOR:
						((float *)pOutputData)[0] = ((float *)pInputData)[0];
						((float *)pOutputData)[1] = ((float *)pInputData)[1];
						((float *)pOutputData)[2] = ((float *)pInputData)[2];
					break;
					case FIELD_POSITION_VECTOR:
						((float *)pOutputData)[0] = ((float *)pInputData)[0] + position.x;
						((float *)pOutputData)[1] = ((float *)pInputData)[1] + position.y;
						((float *)pOutputData)[2] = ((float *)pInputData)[2] + position.z;
					break;

					case FIELD_BOOLEAN:
					case FIELD_INTEGER:
						*((int *)pOutputData) = *( int *)pInputData;
					break;

					case FIELD_SHORT:
						*((short *)pOutputData) = *( short *)pInputData;
					break;

					case FIELD_CHARACTER:
						*((char *)pOutputData) = *( char *)pInputData;
					break;

					case FIELD_POINTER:
						*((int *)pOutputData) = *( int *)pInputData;
					break;
					case FIELD_FUNCTION:
						if ( strlen( (char *)pInputData ) == 0 )
							*((int *)pOutputData) = 0;
						else
							*((int *)pOutputData) = FUNCTION_FROM_NAME( (char *)pInputData );
					break;

					default:
						ALERT( at_error, "Bad field type\n" );
					}
				}
			}
#if 0
			else
			{
				ALERT( at_console, "Skipping global field %s\n", pName );
			}
#endif
			return fieldNumber;
		}
	}

	return -1;
}


int CRestore::ReadEntVars( const char *pname, entvars_t *pev )
{
	return ReadFields( pname, pev, gEntvarsDescription, ENTVARS_COUNT );
}


int CRestore::ReadFields( const char *pname, void *pBaseData, TYPEDESCRIPTION *pFields, int fieldCount )
{
	unsigned short	i, token;
	int	lastField, fileCount;
	HEADER	header;

	i = ReadShort();
	ASSERT( i == sizeof(int) );			// First entry should be an int

	token = ReadShort();

	// Check the struct name
	if ( token != TokenHash(pname) )			// Field Set marker
	{
//		ALERT( at_error, "Expected %s found %s!\n", pname, BufferPointer() );
		BufferRewind( 2*sizeof(short) );
		return 0;
	}

	// Skip over the struct name
	fileCount = ReadInt();						// Read field count

	lastField = 0;								// Make searches faster, most data is read/written in the same order

	// Clear out base data
	for ( i = 0; i < fieldCount; i++ )
	{
		// Don't clear global fields
		if ( !m_global || !(pFields[i].flags & FTYPEDESC_GLOBAL) )
			memset( ((char *)pBaseData + pFields[i].fieldOffset), 0, pFields[i].fieldSize * gSizes[pFields[i].fieldType] );
	}

	for ( i = 0; i < fileCount; i++ )
	{
		BufferReadHeader( &header );
		lastField = ReadField( pBaseData, pFields, fieldCount, lastField, header.size, m_pdata->pTokens[header.token], header.pData );
		lastField++;
	}
	
	return 1;
}


void CRestore::BufferReadHeader( HEADER *pheader )
{
	ASSERT( pheader!=NULL );
	pheader->size = ReadShort();				// Read field size
	pheader->token = ReadShort();				// Read field name token
	pheader->pData = BufferPointer();			// Field Data is next
	BufferSkipBytes( pheader->size );			// Advance to next field
}


short	CRestore::ReadShort( void )
{
	short tmp = 0;

	BufferReadBytes( (char *)&tmp, sizeof(short) );

	return tmp;
}

int CRestore::ReadInt( void )
{
	int tmp = 0;

	BufferReadBytes( (char *)&tmp, sizeof(int) );

	return tmp;
}

int CRestore::ReadNamedInt( const char *pName )
{
	HEADER header;

	BufferReadHeader( &header );
	return ((int *)header.pData)[0];
}

char *CRestore::ReadNamedString( const char *pName )
{
	HEADER header;

	BufferReadHeader( &header );
#ifdef TOKENIZE
	return (char *)(m_pdata->pTokens[*(short *)header.pData]);
#else
	return (char *)header.pData;
#endif
}


char *CRestore::BufferPointer( void )
{
	if ( !m_pdata )
		return NULL;

	return m_pdata->pCurrentData;
}

void CRestore::BufferReadBytes( char *pOutput, int size )
{
	ASSERT( m_pdata !=NULL );

	if ( !m_pdata || Empty() )
		return;

	if ( (m_pdata->size + size) > m_pdata->bufferSize )
	{
		ALERT( at_error, "Restore overflow!" );
		m_pdata->size = m_pdata->bufferSize;
		return;
	}

	if ( pOutput )
		memcpy( pOutput, m_pdata->pCurrentData, size );
	m_pdata->pCurrentData += size;
	m_pdata->size += size;
}


void CRestore::BufferSkipBytes( int bytes )
{
	BufferReadBytes( NULL, bytes );
}

int CRestore::BufferSkipZString( void )
{
	char *pszSearch;
	int  len;

	if ( !m_pdata )
		return 0;

	int maxLen = m_pdata->bufferSize - m_pdata->size;

	len = 0;
	pszSearch = m_pdata->pCurrentData;
	while ( *pszSearch++ && len < maxLen )
		len++;

	len++;

	BufferSkipBytes( len );

	return len;
}

//MODDD - changed paramter name "string" to "par_string"
int CRestore::BufferCheckZString( const char *par_string )
{
	if ( !m_pdata )
		return 0;

	int maxLen = m_pdata->bufferSize - m_pdata->size;
	int len = strlen( par_string );
	if ( len <= maxLen )
	{
		if ( !strncmp( par_string, m_pdata->pCurrentData, len ) )
			return 1;
	}
	return 0;
}




//MODDDMIRROR
// ...nothing even ever calls these, what.
///////////////////////////////////////////////////////////////////////////////////////////////
Vector UTIL_GetMirrorOrigin(CBaseEntity *pMirror, Vector pos)
{
	Vector result = pos;

	switch ((int)pMirror->pev->frags)
	{
	case 0:
		result[0] = pMirror->pev->origin[0]*2 - pos[0];
		break;
	case 1:
		result[1] = pMirror->pev->origin[1]*2 - pos[1];
		break;
	case 2:
	default:
		result[2] = pMirror->pev->origin[2]*2 - pos[2];
		break;
	}
	return result;
}

Vector UTIL_GetMirrorAngles (CBaseEntity *pMirror, Vector angles )
{
	Vector result = angles;
	switch ((int)pMirror->pev->frags)
	{
	case 0:
		result[0] = -result[0]; 
		break;
	case 1:
		result[1] = -result[1]; 
		break;
	case 2:
	default:
		result[2] = -result[2]; 
		break;
	}
	return result;
}

Vector UTIL_MirrorVector( Vector angles )
{
	Vector result = angles;
	edict_t *pFind; 
    int numMirrors = 0;
	
	pFind = FIND_ENTITY_BY_CLASSNAME( NULL, "env_mirror" );
          
	while ( !FNullEnt( pFind ) )
	{
		CBaseEntity *pMirror = CBaseEntity::Instance( pFind );
        
		if(numMirrors > MIRROR_MAX){
			easyForcePrintLine("WARNING! Max numer of env_mirror entities \"%d\" exceeded!", MIRROR_MAX);
			break;
		}
		if ( pMirror )
		{
			numMirrors++;
			if(!pMirror->pev->impulse) continue;
			result = UTIL_GetMirrorAngles(pMirror, angles);
		}
		pFind = FIND_ENTITY_BY_CLASSNAME( pFind, "env_mirror" );
	}
	return result;
}

Vector UTIL_MirrorPos ( Vector endpos )
{
	Vector mirpos(0, 0, 0);
	edict_t *pFind; 
    int numMirrors = 0;
	
	pFind = FIND_ENTITY_BY_CLASSNAME( NULL, "env_mirror" );

	while ( !FNullEnt( pFind ) )
	{
		CBaseEntity *pMirror = CBaseEntity::Instance( pFind );
                    
        if(numMirrors > MIRROR_MAX){
			easyForcePrintLine("WARNING! Max numer of env_mirror entities \"%d\" exceeded!", MIRROR_MAX);
			break;
		}
		if ( pMirror )
		{
			numMirrors++;
			if(!pMirror->pev->impulse) continue;
			Vector delta;
			float dist;
		
			delta = pMirror->Center() - endpos;
			dist = delta.Length();
			if(pMirror->pev->waterlevel < dist) continue;
			mirpos = UTIL_GetMirrorOrigin(pMirror, endpos);
		}
		pFind = FIND_ENTITY_BY_CLASSNAME( pFind, "env_mirror" );
	}
	return mirpos;
}
///////////////////////////////////////////////////////////////////////////////////////////////



void UTIL_TE_ShowLine(const Vector& vec1, const Vector& vec2){
	UTIL_TE_ShowLine(vec1.x, vec1.y, vec1.z, vec2.x, vec2.y, vec2.z);
}


void UTIL_TE_ShowLine(float x1, float y1, float z1, float x2, float y2, float z2){
	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_SHOWLINE);

		WRITE_COORD( x1 );
		WRITE_COORD( y1 );
		WRITE_COORD( z1 );

		WRITE_COORD( x2 );
		WRITE_COORD( y2 );
		WRITE_COORD( z2 );
	MESSAGE_END();
}



void UTIL_drawRect(const Vector& vec1, const Vector& vec2){
	UTIL_drawRect(vec1.x, vec1.y, vec1.z, vec2.x, vec2.y, vec2.z);
}

void UTIL_drawRect(float x1, float y1, float z1, float x2, float y2, float z2){
	UTIL_TE_ShowLine(x1, y1, z1, x2, y2, z1);
	UTIL_TE_ShowLine(x2, y2, z1, x2, y2, z2);
	UTIL_TE_ShowLine(x2, y2, z2, x1, y1, z2);
	UTIL_TE_ShowLine(x1, y1, z2, x1, y1, z1);
	
}




void UTIL_drawBox(const Vector& vec1, const Vector& vec2){
	UTIL_drawBox(vec1.x, vec1.y, vec1.z, vec2.x, vec2.y, vec2.z);
}


void UTIL_drawBox(float x1, float y1, float z1, float x2, float y2, float z2){

	UTIL_TE_ShowLine(x1, y1, z1, x1, y2, z1);
	UTIL_TE_ShowLine(x1, y1, z1, x2, y1, z1);
	UTIL_TE_ShowLine(x1, y1, z1, x1, y1, z2);
	
	UTIL_TE_ShowLine(x2, y1, z1, x2, y2, z1);
	UTIL_TE_ShowLine(x2, y1, z1, x2, y1, z2);
	
	UTIL_TE_ShowLine(x1, y2, z1, x2, y2, z1);
	UTIL_TE_ShowLine(x1, y2, z1, x1, y2, z2);

	UTIL_TE_ShowLine(x2, y2, z1, x2, y2, z2);

	UTIL_TE_ShowLine(x1, y1, z2, x1, y2, z2);
	UTIL_TE_ShowLine(x1, y1, z2, x2, y1, z2);
	
	UTIL_TE_ShowLine(x2, y1, z2, x2, y2, z2);
	
	UTIL_TE_ShowLine(x1, y2, z2, x2, y2, z2);

}//END OF UTIL_drawBox




void UTIL_drawLineFrame(const Vector& vec1, const Vector& vec2, int r, int g, int b){
	UTIL_drawLineFrame( vec1.x, vec1.y, vec1.z, vec2.x, vec2.y, vec2.z, DEBUG_LINE_WIDTH, r, g, b);
}

void UTIL_drawLineFrame(const Vector& vec1, const Vector& vec2, int width, int r, int g, int b){
	UTIL_drawLineFrame( vec1.x, vec1.y, vec1.z, vec2.x, vec2.y, vec2.z, width, r, g, b);
	//UTIL_drawLineFrame( roundToNearest(vec1.x), roundToNearest(vec1.y), roundToNearest(vec1.z), roundToNearest(vec2.x), roundToNearest(vec2.y), roundToNearest(vec2.z), BLABLABLA2);
}


void UTIL_drawLineFrame(float x1, float y1, float z1, float x2, float y2, float z2, int width, int r, int g, int b){
	// Fill in defaults. No one wants to specify all that other stuff.
	// And a speed of 0 is ok.
	const int frameStart = 0;
	const int frameRate = 0;
	const int life = 1;
	const int speed = 0;
	const int noise = 0;
	UTIL_TE_BeamPoints(x1, y1, z1, x2, y2, z2, frameStart, frameRate, life, width, noise, r, g, b, 255, speed);
}//END OF UTIL_drawLineFrame


void UTIL_drawLineFrame(float x1, float y1, float z1, float x2, float y2, float z2, int width, int life, int r, int g, int b){
	// Fill in defaults. No one wants to specify all that other stuff.
	// And a speed of 0 is ok.
	const int frameStart = 0;
	const int frameRate = 0;
	//const int life = 1;
	const int speed = 0;
	const int noise = 0;
	UTIL_TE_BeamPoints(x1, y1, z1, x2, y2, z2, frameStart, frameRate, life, width, noise, r, g, b, 255, speed);
}//END OF UTIL_drawLineFrame



//Draw a vertical line going through this point, with the point as the center of the line. This makes it easier to see the point than drawing at one X, Y, Z coordinate (nearly impossible to see sometimes)
void UTIL_drawPointFrame(const Vector& vecPoint, int width, int r, int g, int b){
	UTIL_drawPointFrame(vecPoint.x, vecPoint.y, vecPoint.z, width, r, g, b);
}//END OF UTIL_drawPointFrame

void UTIL_drawPointFrame(float point_x, float point_y, float point_z, int width, int r, int g, int b){
	//UTIL_drawLineFrame(vecPoint + Vector(0, 0, -30), vecPoint + Vector(0,0, 30), width, r, g, b);
	UTIL_drawLineFrame(point_x, point_y, point_z + -30, point_x, point_y, point_z + 30, width, r, g, b);
}//END OF UTIL_drawPointFrame





void UTIL_drawRectFrame(const Vector& vec1, const Vector& vec2, int width, int r, int g, int b){
	UTIL_drawRectFrame(vec1.x, vec1.y, vec1.z, vec2.x, vec2.y, vec2.z, width, r, g, b);
}

void UTIL_drawRectFrame(float x1, float y1, float z1, float x2, float y2, float z2, int width, int r, int g, int b){
	UTIL_drawLineFrame(x1, y1, z1, x2, y2, z1, width, r, g, b);
	UTIL_drawLineFrame(x2, y2, z1, x2, y2, z2, width, r, g, b);
	UTIL_drawLineFrame(x2, y2, z2, x1, y1, z2, width, r, g, b);
	UTIL_drawLineFrame(x1, y1, z2, x1, y1, z1, width, r, g, b);
	
}

void UTIL_drawBoxFrame(const Vector& vec1, const Vector& vec2, int width, int r, int g, int b){
	UTIL_drawBoxFrame(vec1.x, vec1.y, vec1.z, vec2.x, vec2.y, vec2.z, width, r, g, b);
}
void UTIL_drawBoxFrame(const Vector& vec1, const Vector& vec2, int width, int life, int r, int g, int b){
	UTIL_drawBoxFrame(vec1.x, vec1.y, vec1.z, vec2.x, vec2.y, vec2.z, width, life, r, g, b);
}
	

void UTIL_drawBoxFrame(float x1, float y1, float z1, float x2, float y2, float z2, int width, int r, int g, int b){

	UTIL_drawLineFrame(x1, y1, z1, x1, y2, z1, width, r, g, b);
	UTIL_drawLineFrame(x1, y1, z1, x2, y1, z1, width, r, g, b);
	UTIL_drawLineFrame(x1, y1, z1, x1, y1, z2, width, r, g, b);
	UTIL_drawLineFrame(x2, y1, z1, x2, y2, z1, width, r, g, b);
	UTIL_drawLineFrame(x2, y1, z1, x2, y1, z2, width, r, g, b);
	UTIL_drawLineFrame(x1, y2, z1, x2, y2, z1, width, r, g, b);
	UTIL_drawLineFrame(x1, y2, z1, x1, y2, z2, width, r, g, b);
	UTIL_drawLineFrame(x2, y2, z1, x2, y2, z2, width, r, g, b);
	UTIL_drawLineFrame(x1, y1, z2, x1, y2, z2, width, r, g, b);
	UTIL_drawLineFrame(x1, y1, z2, x2, y1, z2, width, r, g, b);
	UTIL_drawLineFrame(x2, y1, z2, x2, y2, z2, width, r, g, b);
	UTIL_drawLineFrame(x1, y2, z2, x2, y2, z2, width, r, g, b);

}//END OF UTIL_drawBoxFrame


void UTIL_drawBoxFrame(float x1, float y1, float z1, float x2, float y2, float z2, int width, int life, int r, int g, int b){

	UTIL_drawLineFrame(x1, y1, z1, x1, y2, z1, width, life, r, g, b);
	UTIL_drawLineFrame(x1, y1, z1, x2, y1, z1, width, life,r, g, b);
	UTIL_drawLineFrame(x1, y1, z1, x1, y1, z2, width, life,r, g, b);
	UTIL_drawLineFrame(x2, y1, z1, x2, y2, z1, width, life,r, g, b);
	UTIL_drawLineFrame(x2, y1, z1, x2, y1, z2, width, life,r, g, b);
	UTIL_drawLineFrame(x1, y2, z1, x2, y2, z1, width, life,r, g, b);
	UTIL_drawLineFrame(x1, y2, z1, x1, y2, z2, width, life,r, g, b);
	UTIL_drawLineFrame(x2, y2, z1, x2, y2, z2, width, life,r, g, b);
	UTIL_drawLineFrame(x1, y1, z2, x1, y2, z2, width, life,r, g, b);
	UTIL_drawLineFrame(x1, y1, z2, x2, y1, z2, width, life,r, g, b);
	UTIL_drawLineFrame(x2, y1, z2, x2, y2, z2, width, life,r, g, b);
	UTIL_drawLineFrame(x1, y2, z2, x2, y2, z2, width, life,r, g, b);

}//END OF UTIL_drawBoxFrame


//draw a box centered around a point for emphasis, and a 3D cross through the point itself.

void UTIL_drawLineFrameBoxAround(const Vector& vec1, int width, int boxSize, int r, int g, int b){
	UTIL_drawLineFrameBoxAround(vec1.x, vec1.y, vec1.z, width, boxSize, r, g, b);
}

//draw a box centered around a point for emphasis, and a 3D cross through the point itself.

void UTIL_drawLineFrameBoxAround(float x1, float y1, float z1, int width, int boxSize, int r, int g, int b){
	int halfwidth = boxSize/2;
	UTIL_drawBoxFrame(x1-halfwidth, y1-halfwidth, z1-halfwidth, x1+halfwidth, y1+halfwidth, z1+halfwidth, width, r, g, b);

	/*
	UTIL_drawLineFrame(x1-halfwidth, y1, z1, x1+halfwidth, y1, z1, width, r, g, b);
	UTIL_drawLineFrame(x1, y1-halfwidth, z1, x1, y1+halfwidth, z1, width, r, g, b);
	UTIL_drawLineFrame(x1, y1, z1-halfwidth, x1, y1, z1+halfwidth, width, r, g, b);
	*/
}




//draw a box centered around a point for emphasis, and a 3D cross through the point itself.
//MORE DETAIL

void UTIL_drawLineFrameBoxAround2(const Vector& vec1, int width, int boxSize, int r, int g, int b){
	UTIL_drawLineFrameBoxAround(vec1.x, vec1.y, vec1.z, width, boxSize, r, g, b);
}

//draw a box centered around a point for emphasis, and a 3D cross through the point itself.

void UTIL_drawLineFrameBoxAround2(float x1, float y1, float z1, int width, int boxSize, int r, int g, int b){
	int halfwidth = boxSize/2;
	UTIL_drawBoxFrame(x1-halfwidth, y1-halfwidth, z1-halfwidth, x1+halfwidth, y1+halfwidth, z1+halfwidth, width, r, g, b);

	UTIL_drawLineFrame(x1-halfwidth, y1, z1, x1+halfwidth, y1, z1, width, r, g, b);
	UTIL_drawLineFrame(x1, y1-halfwidth, z1, x1, y1+halfwidth, z1, width, r, g, b);
	UTIL_drawLineFrame(x1, y1, z1-halfwidth, x1, y1, z1+halfwidth, width, r, g, b);


	UTIL_drawLineFrame(x1-halfwidth, y1-halfwidth, z1, x1+halfwidth, y1-halfwidth, z1, width, r, g, b);
	UTIL_drawLineFrame(x1-halfwidth, y1+halfwidth, z1, x1+halfwidth, y1+halfwidth, z1, width, r, g, b);

	UTIL_drawLineFrame(x1-halfwidth, y1-halfwidth, z1, x1-halfwidth, y1+halfwidth, z1, width, r, g, b);
	UTIL_drawLineFrame(x1+halfwidth, y1-halfwidth, z1, x1+halfwidth, y1+halfwidth, z1, width, r, g, b);
}



//draw a box centered around a point for emphasis, and a 3D cross through the point itself.
//EVEN MORE DETAIL

void UTIL_drawLineFrameBoxAround3(const Vector& vec1, int width, int boxSize, int r, int g, int b){
	UTIL_drawLineFrameBoxAround(vec1.x, vec1.y, vec1.z, width, boxSize, r, g, b);
}

//draw a box centered around a point for emphasis, and a 3D cross through the point itself.

void UTIL_drawLineFrameBoxAround3(float x1, float y1, float z1, int width, int boxSize, int r, int g, int b){
	int halfwidth = boxSize/2;
	UTIL_drawBoxFrame(x1-halfwidth, y1-halfwidth, z1-halfwidth, x1+halfwidth, y1+halfwidth, z1+halfwidth, width, r, g, b);

	UTIL_drawLineFrame(x1-halfwidth, y1, z1, x1+halfwidth, y1, z1, width, r, g, b);
	UTIL_drawLineFrame(x1, y1-halfwidth, z1, x1, y1+halfwidth, z1, width, r, g, b);
	UTIL_drawLineFrame(x1, y1, z1-halfwidth, x1, y1, z1+halfwidth, width, r, g, b);


	
	UTIL_drawLineFrame(x1-halfwidth, y1-halfwidth, z1, x1+halfwidth, y1-halfwidth, z1, width, r, g, b);
	UTIL_drawLineFrame(x1-halfwidth, y1+halfwidth, z1, x1+halfwidth, y1+halfwidth, z1, width, r, g, b);

	UTIL_drawLineFrame(x1-halfwidth, y1-halfwidth, z1, x1-halfwidth, y1+halfwidth, z1, width, r, g, b);
	UTIL_drawLineFrame(x1+halfwidth, y1-halfwidth, z1, x1+halfwidth, y1+halfwidth, z1, width, r, g, b);

	UTIL_drawLineFrame(x1-halfwidth, y1, z1+halfwidth, x1+halfwidth, y1, z1+halfwidth, width, r, g, b);
	UTIL_drawLineFrame(x1-halfwidth, y1, z1-halfwidth, x1+halfwidth, y1, z1-halfwidth, width, r, g, b);
	UTIL_drawLineFrame(x1, y1-halfwidth, z1+halfwidth, x1, y1+halfwidth, z1+halfwidth, width, r, g, b);
	UTIL_drawLineFrame(x1, y1-halfwidth, z1-halfwidth, x1, y1+halfwidth, z1-halfwidth, width, r, g, b);



	UTIL_drawLineFrame(x1-halfwidth, y1, z1-halfwidth, x1-halfwidth, y1, z1+halfwidth, width, r, g, b);
	UTIL_drawLineFrame(x1+halfwidth, y1, z1-halfwidth, x1+halfwidth, y1, z1+halfwidth, width, r, g, b);
	
	UTIL_drawLineFrame(x1, y1-halfwidth, z1-halfwidth, x1, y1-halfwidth, z1+halfwidth, width, r, g, b);
	UTIL_drawLineFrame(x1, y1+halfwidth, z1-halfwidth, x1, y1+halfwidth, z1+halfwidth, width, r, g, b);
	
}




//MODDD - TODO, major.  For debug tools at least.
// Any coord below -4096 or at/above 4096 is not sent properly by WriteCoord (DebugLine's use those).
// -4096 exactly and 4095.9999 are ok though.
// Even overflow logic is weird, you would think 4096 exactly plus that borderline-okay amount (4095.9999) would
// put the coord at the exact same place but, nope.  It messes up the other coords too, goes kinda vaguely right
// sometimes and completely off the others, even with the other two coords perfectly valid.
// Seems to be more of a world-limit thing than data-limit thing, any checks for being in the world tend to involve
// +- 4096.  Still weird to not tolerate it anyway.
// It also appears it is a bound issue, and not a difference one.  Even a line from -4096 to 4095.9999 should be ok,
// but that is difficult to test exactly (lines with both points stuck in the map refuse to draw it seems).
// Although a line over 4096 in length could be drawn, so that should be all that matters.

// SO.   Check.  If any coord is over 4096, take the higest one and adjust the size so that it reaches 4095 at most
// (no one cares about precision up to but not including 4096 like 4095.9999, why risk rounding being weird and going
// up anyway).


void UTIL_TE_BeamPoints(const Vector& vec1, const Vector& vec2, int frameStart, int frameRate, int life, int width, int noise, int r, int g, int b, int brightness, int speed ){
	UTIL_TE_BeamPoints(vec1.x, vec1.y, vec1.z, vec2.x, vec2.y, vec2.z, frameStart, frameRate, life, width, noise, r, g, b, brightness, speed);
}
void UTIL_TE_BeamPoints(float x1, float y1, float z1, float x2, float y2, float z2, int frameStart, int frameRate, int life, int width, int noise, int r, int g, int b, int brightness, int speed){

	/*
	x1 = UTIL_clamp(x1, -4095, 4095);
	y1 = UTIL_clamp(y1, -4095, 4095);
	z1 = UTIL_clamp(z1, -4095, 4095);
	x2 = UTIL_clamp(x2, -4095, 4095);
	y2 = UTIL_clamp(y2, -4095, 4095);
	z2 = UTIL_clamp(z2, -4095, 4095);
	*/


	/*
	int* offendingCoord1 = NULL;
	int* offendingCoord2 = NULL;

	if (x1 < -4095) {

	}
	*/


	if (
		x1 < -4095 || x1 > 4095 ||
		y1 < -4095 || y1 > 4095 ||
		z1 < -4095 || z1 > 4095 ||
		x2 < -4095 || x2 > 4095 ||
		y2 < -4095 || y2 > 4095 ||
		z2 < -4095 || z2 > 4095
	){
		//LAZY. just cut the end point in half, but maintain the direction from vec1.

		// This is also assuming the 2nd point is the problem one, should work out as the most common issue would be
		// some weapon with a ridiculous end-point (like gauss) easily going out of map bounds, but the first point still 
		// being in-bounds (player origin).
		const Vector vec1 = Vector(x1, y1, z1);
		const Vector vec2 = Vector(x2, y2, z2);
		const Vector vecDelta = (vec2 - vec1);
		const Vector vecDir = vecDelta.Normalize();
		float daLength = vecDelta.Length();

		const Vector vecFinal2 = vec2 - vecDir * (daLength / 2);

		// and apply.
		x2 = vecFinal2.x;
		y2 = vecFinal2.y;
		z2 = vecFinal2.z;
	}




	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_BEAMPOINTS);
		WRITE_COORD( x1 );
		WRITE_COORD( y1 );
		WRITE_COORD( z1 );
		WRITE_COORD( x2 );
		WRITE_COORD( y2 );
		WRITE_COORD( z2 );

		WRITE_SHORT( g_sModelIndexLaser );
		WRITE_BYTE( frameStart ); // frame start
		WRITE_BYTE( frameRate ); // framerate
		WRITE_BYTE( life ); // life
		WRITE_BYTE( width );  // width
		WRITE_BYTE( noise );   // noise
		WRITE_BYTE( r );   // r, g, b
		WRITE_BYTE( g );   // r, g, b
		WRITE_BYTE( b );   // r, g, b
		WRITE_BYTE( brightness );	// brightness
		WRITE_BYTE( speed );		// speed
	MESSAGE_END();
}



void UTIL_TE_BeamPoints_Rect(const Vector& vec1, const Vector& vec2, int frameStart, int frameRate, int life, int width, int noise, int r, int g, int b, int brightness, int speed){
	UTIL_TE_BeamPoints_Rect(vec1.x, vec1.y, vec1.z, vec2.x, vec2.y, vec2.z, frameStart, frameRate, life, width, noise, r, g, b, brightness, speed);
}

void UTIL_TE_BeamPoints_Rect(float x1, float y1, float z1, float x2, float y2, float z2, int frameStart, int frameRate, int life, int width, int noise, int r, int g, int b, int brightness, int speed){
	UTIL_TE_BeamPoints(x1, y1, z1, x2, y2, z1, frameStart, frameRate, life, width, noise, r, g, b, brightness, speed);
	UTIL_TE_BeamPoints(x2, y2, z1, x2, y2, z2, frameStart, frameRate, life, width, noise, r, g, b, brightness, speed);
	UTIL_TE_BeamPoints(x2, y2, z2, x1, y1, z2, frameStart, frameRate, life, width, noise, r, g, b, brightness, speed);
	UTIL_TE_BeamPoints(x1, y1, z2, x1, y1, z1, frameStart, frameRate, life, width, noise, r, g, b, brightness, speed);
	
}



void UTIL_TE_BeamPoints_Box(const Vector& vec1, const Vector& vec2, int frameStart, int frameRate, int life, int width, int noise, int r, int g, int b, int brightness, int speed){
	UTIL_TE_BeamPoints_Box(vec1.x, vec1.y, vec1.z, vec2.x, vec2.y, vec2.z, frameStart, frameRate, life, width, noise, r, g, b, brightness, speed);
}


void UTIL_TE_BeamPoints_Box(float x1, float y1, float z1, float x2, float y2, float z2, int frameStart, int frameRate, int life, int width, int noise, int r, int g, int b, int brightness, int speed){

	UTIL_TE_BeamPoints(x1, y1, z1, x1, y2, z1, frameStart, frameRate, life, width, noise, r, g, b, brightness, speed);
	UTIL_TE_BeamPoints(x1, y1, z1, x2, y1, z1, frameStart, frameRate, life, width, noise, r, g, b, brightness, speed);
	UTIL_TE_BeamPoints(x1, y1, z1, x1, y1, z2, frameStart, frameRate, life, width, noise, r, g, b, brightness, speed);
	
	UTIL_TE_BeamPoints(x2, y1, z1, x2, y2, z1, frameStart, frameRate, life, width, noise, r, g, b, brightness, speed);
	UTIL_TE_BeamPoints(x2, y1, z1, x2, y1, z2, frameStart, frameRate, life, width, noise, r, g, b, brightness, speed);
	
	UTIL_TE_BeamPoints(x1, y2, z1, x2, y2, z1, frameStart, frameRate, life, width, noise, r, g, b, brightness, speed);
	UTIL_TE_BeamPoints(x1, y2, z1, x1, y2, z2, frameStart, frameRate, life, width, noise, r, g, b, brightness, speed);

	UTIL_TE_BeamPoints(x2, y2, z1, x2, y2, z2, frameStart, frameRate, life, width, noise, r, g, b, brightness, speed);

	UTIL_TE_BeamPoints(x1, y1, z2, x1, y2, z2, frameStart, frameRate, life, width, noise, r, g, b, brightness, speed);
	UTIL_TE_BeamPoints(x1, y1, z2, x2, y1, z2, frameStart, frameRate, life, width, noise, r, g, b, brightness, speed);
	
	UTIL_TE_BeamPoints(x2, y1, z2, x2, y2, z2, frameStart, frameRate, life, width, noise, r, g, b, brightness, speed);
	
	UTIL_TE_BeamPoints(x1, y2, z2, x2, y2, z2, frameStart, frameRate, life, width, noise, r, g, b, brightness, speed);

}//END OF UTIL_TE_BeamPoints_Box






Vector UTIL_rotateShift(const Vector& src, const Vector& forward ){
	return UTIL_rotateShift(src.x, src.y, src.z, forward.x, forward.y, forward.z);
}

Vector UTIL_rotateShift(const Vector& src, const float forwardX, const float forwardY, const float forwardZ ){
	return UTIL_rotateShift(src.x, src.y, src.z, forwardX, forwardY, forwardZ);
}

Vector UTIL_rotateShift(const float srcX, const float srcY, const float srcZ, const Vector& forward){
	return UTIL_rotateShift(srcX, srcY, srcZ, forward.x, forward.y, forward.z);
}

Vector UTIL_rotateShift(const float srcX, const float srcY, const float srcZ, const float forwardX, const float forwardY, const float forwardZ ){

	Vector toReturn;

	float ang = atan( (srcY/srcX) );
	//float ang2 = atan( (float)(forward.y / forward.x) );
	//float angDelta = ang2 - ang;

	float magSrc = sqrt( pow(srcX, (float)2.0) + pow(srcY, (float)2.0)  ) ;


	float angDest = forwardY + ang;
	float finalX = cos(angDest) * magSrc;
	float finalY = sin(angDest) * magSrc;
	float finalZ = 0;

	toReturn.x = finalX;
	toReturn.y = finalY;
	toReturn.z = finalZ;

	return toReturn;
}



Vector UTIL_Intersect( Vector vecSrc, Vector vecDst, Vector vecMove, float flSpeed )
{
	Vector vecTo = vecDst - vecSrc;

	float a = DotProduct( vecMove, vecMove ) - flSpeed * flSpeed;
	float b = 0 * DotProduct(vecTo, vecMove); // why does this work?
	float c = DotProduct( vecTo, vecTo );

	float t;
	if (a == 0)
	{
		t = c / (flSpeed * flSpeed);
	}
	else
	{
		t = b * b - 4 * a * c;
		t = sqrt( t ) / (2.0 * a);
		float t1 = -b +t;
		float t2 = -b -t;

		if (t1 < 0 || t2 < t1)
			t = t2;
		else
			t = t1;
	}

	// ALERT( at_console, "Intersect %f\n", t );

	if (t < 0.1)
		t = 0.1;
	if (t > 10.0)
		t = 10.0;

	Vector vecHit = vecTo + vecMove * t;
	return vecHit.Normalize( ) * flSpeed;
}




Vector UTIL_projectionComponent(const Vector& u, const Vector& n){

	//const Vector& vecNorm = tr.vecPlaneNormal;
	//const Vector& vecU = vecSuggestedDir;

	//Vector vecResult = vecU - ( (vecU.x * vecNorm.x + vecU.y * vecNorm.y + vecU.z * vecNorm.z) / (pow(vecNorm.x, 2) + pow(vecNorm.y, 2) + pow(vecNorm.z, 2) ) * vecNorm  );

	return u - ( ( (u.x * n.x + u.y * n.y + u.z * n.z ) / ( pow(n.x, 2) + pow(n.y, 2) + pow(n.z, 2) ) ) * n   );

}

Vector UTIL_projectionComponentPreserveMag(const Vector& u, const Vector& n){

	//const Vector& vecNorm = tr.vecPlaneNormal;
	//const Vector& vecU = vecSuggestedDir;

	//Vector vecResult = vecU - ( (vecU.x * vecNorm.x + vecU.y * vecNorm.y + vecU.z * vecNorm.z) / (pow(vecNorm.x, 2) + pow(vecNorm.y, 2) + pow(vecNorm.z, 2) ) * vecNorm  );

	Vector tempVect = (u - ( ( (u.x * n.x + u.y * n.y + u.z * n.z ) / ( pow(n.x, 2) + pow(n.y, 2) + pow(n.z, 2) ) ) * n   ) );
	float sizeMult = (u.Length() ) / tempVect.Length();

	return tempVect * sizeMult;

}


void UTIL_generateFreakyLight( const Vector& arg_origin){

	PLAYBACK_EVENT_FULL (FEV_GLOBAL, NULL, g_sFreakyLight, 0.0, 
	(float *)&arg_origin, (float *)&Vector(0,0,0), 0.0, 0.0, 0, 0, FALSE, FALSE);
	
}



//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//PRINT QUEUE RELATED, mostly.
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////


Vector UTIL_getFloor(const Vector &vecStart, const float& distDown, IGNORE_MONSTERS igmon, edict_t *pentIgnore ){

	TraceResult tr;
	
	UTIL_TraceLine(vecStart, Vector(vecStart.x, vecStart.y, vecStart.z - distDown), igmon, pentIgnore, &tr);

	/*
				CBaseEntity* pPlayerEntityScan = NULL;
				while( (pPlayerEntityScan = UTIL_FindEntityByClassname(pPlayerEntityScan, "player")) != NULL){
					CBasePlayer* tempPlayer = static_cast<CBasePlayer*>(pPlayerEntityScan);
					tempPlayer->debugVect1Draw = TRUE;
					tempPlayer->debugVect1Start = vecStart;
					tempPlayer->debugVect1End = Vector(vecStart.x, vecStart.y, vecStart.z - distDown);
					tempPlayer->debugVect1Success = (tr.flFraction < 1.0);
					
				}
*/


	if(tr.flFraction >= 1.0){
		//did not hit...  just return the start?
		//return vecStart;
		//...signal this was an error.
		return Vector(-999999,-999999,-999999);
	}else{
		//position of collision, or the ground in this case.
		return tr.vecEndPos;
	}

}

BOOL isErrorVector(const Vector& vec){
	if(vec.x == -999999 && vec.y == -999999 && vec.z == -999999){
		//what are the odds? not that great.  error bector.
		return TRUE;
	}else{
		return FALSE;
	}
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
// PRINT QUEUE HELP end.
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////




//find any scientists in a wide area and update their models.
void globalUpdateModel(){

	edict_t* pEdict = g_engfuncs.pfnPEntityOfEntIndex(1);
	CBaseEntity* pEntity;
	if (!pEdict)
		return;
	for (int i = 1; i < gpGlobals->maxEntities; i++, pEdict++)
	{
		if (pEdict->free)	// Not in use
			continue;
		if (!(pEdict->v.flags & (FL_CLIENT | FL_MONSTER)))	// Not a client/monster ?
			continue;

		pEntity = CBaseEntity::Instance(pEdict);
		if (!pEntity)
			continue;

		CBaseMonster* tempMonster = pEntity->MyMonsterPointer();
		if (tempMonster == NULL || FClassnameIs(tempMonster->pev, "player")) {
			continue;  //not players or non-monsters.
		}

		// ok!
		tempMonster->setModel();

	}//END OF through all entities.

}

void turnWorldLightsOn(){
	// 0 normal
	LIGHT_STYLE(0, "m");
	
	// 1 FLICKER (first variety)
	LIGHT_STYLE(1, "mmnmmommommnonmmonqnmmo");
	
	// 2 SLOW STRONG PULSE
	LIGHT_STYLE(2, "abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcba");
	
	// 3 CANDLE (first variety)
	LIGHT_STYLE(3, "mmmmmaaaaammmmmaaaaaabcdefgabcdefg");
	
	// 4 FAST STROBE
	LIGHT_STYLE(4, "mamamamamama");
	
	// 5 GENTLE PULSE 1
	LIGHT_STYLE(5,"jklmnopqrstuvwxyzyxwvutsrqponmlkj");
	
	// 6 FLICKER (second variety)
	LIGHT_STYLE(6, "nmonqnmomnmomomno");
	
	// 7 CANDLE (second variety)
	LIGHT_STYLE(7, "mmmaaaabcdefgmmmmaaaammmaamm");
	
	// 8 CANDLE (third variety)
	LIGHT_STYLE(8, "mmmaaammmaaammmabcdefaaaammmmabcdefmmmaaaa");
	
	// 9 SLOW STROBE (fourth variety)
	LIGHT_STYLE(9, "aaaaaaaazzzzzzzz");
	
	// 10 FLUORESCENT FLICKER
	LIGHT_STYLE(10, "mmamammmmammamamaaamammma");

	// 11 SLOW PULSE NOT FADE TO BLACK
	LIGHT_STYLE(11, "abcdefghijklmnopqrrqponmlkjihgfedcba");
	
	// 12 UNDERWATER LIGHT MUTATION
	// this light only distorts the lightmap - no contribution
	// is made to the brightness of affected surfaces
	LIGHT_STYLE(12, "mmnnmmnnnmmnn");
	
	// styles 32-62 are assigned by the light program for switchable lights

	// 63 testing
	LIGHT_STYLE(63, "a");

	//"hope that gets the point across"
	//global_forceWorldLightOff = 0;
	EASY_CVAR_SET_DEBUGONLY(forceWorldLightOff, 0);
	g_queueCVarHiddenSave = TRUE;

	/*
	LIGHT_STYLE(0, "jklmnopqrstuvwxyzyxwvutsrqponmlkj");
	LIGHT_STYLE(1, "jklmnopqrstuvwxyzyxwvutsrqponmlkj");
	LIGHT_STYLE(2, "jklmnopqrstuvwxyzyxwvutsrqponmlkj");
	LIGHT_STYLE(3, "jklmnopqrstuvwxyzyxwvutsrqponmlkj");
	LIGHT_STYLE(4, "jklmnopqrstuvwxyzyxwvutsrqponmlkj");
	LIGHT_STYLE(5, "jklmnopqrstuvwxyzyxwvutsrqponmlkj");
	LIGHT_STYLE(6, "jklmnopqrstuvwxyzyxwvutsrqponmlkj");
	LIGHT_STYLE(7, "jklmnopqrstuvwxyzyxwvutsrqponmlkj");
	LIGHT_STYLE(8, "jklmnopqrstuvwxyzyxwvutsrqponmlkj");
	LIGHT_STYLE(9, "jklmnopqrstuvwxyzyxwvutsrqponmlkj");
	LIGHT_STYLE(10, "jklmnopqrstuvwxyzyxwvutsrqponmlkj");
	LIGHT_STYLE(11, "jklmnopqrstuvwxyzyxwvutsrqponmlkj");
	LIGHT_STYLE(12, "jklmnopqrstuvwxyzyxwvutsrqponmlkj");
	LIGHT_STYLE(63, "jklmnopqrstuvwxyzyxwvutsrqponmlkj");
	*/

}


void turnWorldLightsOff(){
	// 0 normal
	LIGHT_STYLE(0, "a");
	
	// 1 FLICKER (first variety)
	LIGHT_STYLE(1, "a");
	
	// 2 SLOW STRONG PULSE
	LIGHT_STYLE(2, "a");
	
	// 3 CANDLE (first variety)
	LIGHT_STYLE(3, "a");
	
	// 4 FAST STROBE
	LIGHT_STYLE(4, "a");
	
	// 5 GENTLE PULSE 1
	LIGHT_STYLE(5,"a");
	
	// 6 FLICKER (second variety)
	LIGHT_STYLE(6, "a");
	
	// 7 CANDLE (second variety)
	LIGHT_STYLE(7, "a");
	
	// 8 CANDLE (third variety)
	LIGHT_STYLE(8, "a");
	
	// 9 SLOW STROBE (fourth variety)
	LIGHT_STYLE(9, "a");
	
	// 10 FLUORESCENT FLICKER
	LIGHT_STYLE(10, "a");

	// 11 SLOW PULSE NOT FADE TO BLACK
	LIGHT_STYLE(11, "a");
	
	// 12 UNDERWATER LIGHT MUTATION
	// this light only distorts the lightmap - no contribution
	// is made to the brightness of affected surfaces
	LIGHT_STYLE(12, "a");
	
	// styles 32-62 are assigned by the light program for switchable lights

	// 63 testing
	LIGHT_STYLE(63, "a");


	//NEW. Also turn the lights off of any light entities.
	
	edict_t		*pEdict = g_engfuncs.pfnPEntityOfEntIndex( 1 );
	CBaseEntity* pEntity;
	
	if(pEdict){
		for ( int i = 1; i < gpGlobals->maxEntities; i++, pEdict++ ){
			if ( pEdict->free )	// Not in use
				continue;
		
			//if ( flagMask && !(pEdict->v.flags & flagMask) )	// Does it meet the criteria?
			//	continue;

			pEntity = CBaseEntity::Instance(pEdict);
			if ( !pEntity )
				continue;

			const char* entClassname = pEntity->getClassname();
			if(
				FStrEq(entClassname, "light") ||
				FStrEq(entClassname, "light_spot") ||
				FStrEq(entClassname, "light_environment"))
			{
				//yay lightz
				CLight* lightRef = static_cast<CLight*>(pEntity);
				lightRef->TurnOff();
			}
			

			if(FStrEq(entClassname, "env_glow")){
				//pEntity->pev->nextthink = 0;
				//::UTIL_Remove(pEntity);
				pEntity->pev->effects |= EF_NODRAW;
			}


		}//END OF for loop through all entities.
	}//END OF initial entity edict check




	//"hope that gets the point across"
	//global_forceWorldLightOff = 1;
	EASY_CVAR_SET_DEBUGONLY(forceWorldLightOff, 1);
	g_queueCVarHiddenSave = TRUE;

}



//const Vector* arg_suggestedOrigin
void updateCVarRefs(BOOL isEarly){

	// Skill update check moved to client.cpp.  Should not be constrained by the
	// minimum frame / startup delay.  Probbaly?

	if(EASY_CVAR_GET_DEBUGONLY(forceWorldLightOff) != forceWorldLightOffMem){
		//easyPrintLine("WHAT THE light %.2f %.2f", cvar_forceWorldLightOff->value, global_forceWorldLightOff);
		forceWorldLightOffMem = EASY_CVAR_GET_DEBUGONLY(forceWorldLightOff);

		if(EASY_CVAR_GET_DEBUGONLY(forceWorldLightOff) == 1){
			turnWorldLightsOff();
		}else{
			//turn on is instant and not updated.
			turnWorldLightsOn();
		}
	}

	/*
	if(global_forceWorldLightOff == 1){
		turnWorldLightsOff();
	}else{
		//turnWorldLightsOn();
	}
	*/




	if (EASY_CVAR_GET_DEBUGONLY(barnacleCanGib) != barnacleCanGibMem) {
		barnacleCanGibMem = EASY_CVAR_GET_DEBUGONLY(barnacleCanGib);
		//MODDD - section
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//If this CVar is changed, update all existing dead barnacles.  This makes barnacle corpses that were once unkillable killable, or vice versa.
		CBaseEntity* pEntityTemp = NULL;
		while ((pEntityTemp = UTIL_FindEntityByClassname(pEntityTemp, "monster_barnacle")) != NULL) {
			//CBarnacle* tempBarnacle = (CBarnacle*)pEntityTemp;
			//easyForcePrintLine("FOUND ONE? %d ", pEntityTemp->pev->deadflag == DEAD_NO);
			if (pEntityTemp->pev->deadflag == DEAD_NO)
			{

			}
			else {

				if (EASY_CVAR_GET_DEBUGONLY(barnacleCanGib) == 0) {
					pEntityTemp->pev->takedamage = DAMAGE_NO;
					pEntityTemp->pev->solid = SOLID_NOT;
				}
				else {
					pEntityTemp->pev->takedamage = DAMAGE_AIM;
					pEntityTemp->pev->solid = SOLID_SLIDEBOX;
				}
			}
		}
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	}//END OF barnacleCanGib CVar check


	if(globalPSEUDO_germanCensorshipMem != EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(sv_germancensorship) || globalPSEUDO_allowGermanModelsMem != EASY_CVAR_GET_DEBUGONLY(allowGermanModels)){
		//easyForcePrintLine("ARE YOU amazin %.2f %.2f %.2f", globalPSEUDO_canApplyGermanCensorship, EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(sv_germancensorship), EASY_CVAR_GET_DEBUGONLY(allowGermanModels));

		if(globalPSEUDO_canApplyGermanCensorship != 1 && (globalPSEUDO_germanCensorshipMem != -1 && EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(sv_germancensorship) == 1) && EASY_CVAR_GET_DEBUGONLY(allowGermanModels) == 1){
		//if(globalPSEUDO_canApplyGermanCensorship != 1 && EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(sv_germancensorship) == 1 && EASY_CVAR_GET_DEBUGONLY(allowGermanModels) == 1){
			//note: "EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(sv_germancensorship)" being -1 means this is the first time it has been set.  Trust this is not the doing of the user.
			easyForcePrintLine("***Change map, restart, or load to update models.  Must have german models.");
			//  ...CVars are \"sv_germancensorship\" & \"allowGermanModels\"***"
		}

		globalPSEUDO_germanCensorshipMem = EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(sv_germancensorship);
		globalPSEUDO_allowGermanModelsMem = EASY_CVAR_GET_DEBUGONLY(allowGermanModels);

		if(globalPSEUDO_canApplyGermanCensorship != 1){
			//can't update.
		}else{
			globalUpdateModel();
		}
		// wait.  Why is sv_germanCensorship hidden again?
		g_queueCVarHiddenSave = TRUE;
	}//END OF german CVars update check


	if(EASY_CVAR_GET(cl_bullsquidspit) != globalPSEUDO_cl_bullsquidspit){
		globalPSEUDO_cl_bullsquidspit = EASY_CVAR_GET(cl_bullsquidspit);

		//commit some changes...
		if(EASY_CVAR_GET(cl_bullsquidspit) == 0){
			EASY_CVAR_SET_DEBUGONLY(bullsquidSpitUseAlphaModel, 0);
			EASY_CVAR_SET_DEBUGONLY(bullsquidSpitUseAlphaEffect, 0);
		}else if(EASY_CVAR_GET(cl_bullsquidspit) == 1){
			EASY_CVAR_SET_DEBUGONLY(bullsquidSpitUseAlphaModel, 1);
			EASY_CVAR_SET_DEBUGONLY(bullsquidSpitUseAlphaEffect, 1);
		}
		g_queueCVarHiddenSave = TRUE;
	}

	if(EASY_CVAR_GET(cl_hornetspiral) != globalPSEUDO_cl_hornetspiral){
		//easyForcePrintLine("hornetcvartest: what2 ", EASY_CVAR_GET(cl_hornetspiral), globalPSEUDO_cl_hornetspiral);
		globalPSEUDO_cl_hornetspiral = EASY_CVAR_GET(cl_hornetspiral);
		
		//commit some changes...
		if(EASY_CVAR_GET(cl_hornetspiral) == 0){
			EASY_CVAR_SET_DEBUGONLY(hornetSpiral, 0);
			EASY_CVAR_SET_DEBUGONLY(hornetSpeedMulti, 1);
			EASY_CVAR_SET_DEBUGONLY(hornetSpeedDartMulti, 2);
			EASY_CVAR_SET_DEBUGONLY(agruntHornetRandomness, 0.1);
		}else if(EASY_CVAR_GET(cl_hornetspiral) == 1){
			EASY_CVAR_SET_DEBUGONLY(hornetSpiral, 1);
			EASY_CVAR_SET_DEBUGONLY(hornetSpeedMulti, 0.6);
			EASY_CVAR_SET_DEBUGONLY(hornetSpeedDartMulti, 2.3);
			EASY_CVAR_SET_DEBUGONLY(agruntHornetRandomness, 0.02);
		}
		g_queueCVarHiddenSave = TRUE;
	}
	
	if(EASY_CVAR_GET(cl_hornettrail) != globalPSEUDO_cl_hornettrail){
		globalPSEUDO_cl_hornettrail = EASY_CVAR_GET(cl_hornettrail);
		
		if(EASY_CVAR_GET(cl_hornettrail) == 0){
			EASY_CVAR_SET_DEBUGONLY(hornetTrail, 0);
			EASY_CVAR_SET_DEBUGONLY(hornetZoomPuff, 1);
		}else if(EASY_CVAR_GET(cl_hornettrail) == 1){
			EASY_CVAR_SET_DEBUGONLY(hornetTrail, 1);
			EASY_CVAR_SET_DEBUGONLY(hornetZoomPuff, 0);
		}
		g_queueCVarHiddenSave = TRUE;
	}
	


	//if(pev != NULL){
		//these are to be sent to the player.
		//~Initial time calls with null pev (start of game).  This is ok.
	
	if (!isEarly) {
		if (gpGlobals->time >= g_nextCVarUpdate) {
			g_nextCVarUpdate = gpGlobals->time + 1;
			EASY_CVAR_UPDATE_SERVER_MASS

			// if in release mode and a dedicated server, using the 
#ifndef _DEBUG
			if (IS_DEDICATED_SERVER()) {
				EASY_CVAR_UPDATE_SERVER_DEDICATED_MASS
			}
#endif
			if(g_queueCVarHiddenSave == TRUE){
				saveHiddenCVars();
			}
			
		}//g_nextCVarUpdate check
	}//isEarly check

		/*
		int tempo = EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(wpn_glocksilencer);
		int otherTempo = CVAR_GET_FLOAT("wpn_glocksilencer");
		//int yaytemp = CVAR_GET_FLOAT(#CVarName)

		if (tempo != 3) {
			int xxx = 4;
		}


		if (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(wpn_glocksilencer) != global_wpn_glocksilencer) {
			global_wpn_glocksilencer = EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(wpn_glocksilencer);
			MESSAGE_BEGIN( MSG_ALL, gmsgUpdateClientCVar, NULL);
				WRITE_SHORT(wpn_glocksilencer_ID);
				WRITE_SHORT( global_wpn_glocksilencer *100);
			MESSAGE_END();
		}
		*/
	//}


		   //EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST(wpn_glocksilencer, wpn_glocksilencer_ID)


	globalPSEUDO_queueClientSendoff = FALSE;

}//END OF updateCVarRefs



BOOL verifyModelExists(char* path){
	if(checkSubFileExistence(path) || checkValveSubFileExistence(path) ){
		//no issue.
		//easyForcePrintLine("I, %s, EXIST!", path);
		PRECACHE_MODEL(path);
		return TRUE;
	}else{
		//easyForcePrintLine("SHIT. %s sucks", path);
		return FALSE;
	}
}


int attemptInterpretSpawnFlag(const char* pszSpawnFlags){

	/*
	if(pszSpawnFlags == NULL){
		//didn't try, nothing.  Done.
		return -1;
	}
	*/

	int toReturn = -1;

	if(isStringEmpty(pszSpawnFlags)){
		//that is, if the first character is the terminating '\0' (length is 0), or the char pointer is null,
		//don't print the error and just give up here.
		return toReturn;
	}

	try{
		toReturn = tryStringToInt(pszSpawnFlags);

		if(toReturn < 0){
			easyPrintLine("GIVE ERROR: Spawn flag cannot be negative!");
			toReturn = -1;  //return to default "-1"
		}

	}catch(int){
		easyPrintLine("GIVE ERROR: Spawn flag invalid; ignored.");
	}

	return toReturn;

}//END OF attemptInterpretSpawnFlag


// This is called by human monsters looking for the RED blood color instead of a direct call for RED blood.
// This lets german censorship give black blood (oil) in anticipation of the robot model replacement.
int UTIL_BloodColorRedFilter(BOOL robotReplacementModelExists){
	if(getGermanModelsAllowed() && robotReplacementModelExists){
		//If german models are allowed and this monster's german model has been found, use the oil blood.
		return BLOOD_COLOR_BLACK;
	}else{
		//Even if german censorship is on but the model substitution isn't allowed or this particular robot model wasn't found,
		//it will catch the RED blood and block any calls to draw it.
		return BLOOD_COLOR_RED;
	}
}//END OF UTIL_BloodColorRedFilter



//OTHER MAJOR PRECAHCE METHODS:
//World.cpp's W_Precache
//ClientPrecache.cpp's ClientPrecache



//extern void W_Precache(void);




// Precaches the weapon and queues the weapon info for sending to clients
void UTIL_PrecacheOtherWeapon( const char *szClassname )
{
	edict_t	*pent;

	pent = CREATE_NAMED_ENTITY( MAKE_STRING( szClassname ) );
	if ( FNullEnt( pent ) )
	{
		ALERT ( at_console, "NULL Ent in UTIL_PrecacheOtherWeapon\n" );
		return;
	}
	
	CBaseEntity *pEntity = CBaseEntity::Instance (VARS( pent ));

	if (pEntity)
	{
		pEntity->Precache();
		RegisterWeapon( (CBasePlayerWeapon*)pEntity, NULL);
	}

	REMOVE_ENTITY(pent);
}//END OF UTIL_PrecacheOtherWeapon


// called by worldspawn
void W_Precache(void)
{


	//just to be clear. Should be this way by default.
	global_useSentenceSave = FALSE;

	
	// MODDD - place for script similar between client and serverside.
	PrecacheShared();
	// And since ammo/weapon registry is about to get determined:
	ClearWeaponInfoCache();
	
	// custom items...
	
	//NOTICE - these are already precached by the weapon_satchel, which is called here for precaching all the assets of.
	//  PRECACHE_SOUND("weapons/g_bounce1.wav"); ...

	// common world objects
	UTIL_PrecacheOther( "item_suit" );
	UTIL_PrecacheOther( "item_battery" );

	UTIL_PrecacheOther( "item_antidote" );

	//MODDD - added Adrenaline & Radiation
	UTIL_PrecacheOther( "item_adrenaline" );
	UTIL_PrecacheOther( "item_radiation" );


	UTIL_PrecacheOther( "item_security" );
	UTIL_PrecacheOther( "item_longjump" );
	//UTIL_PrecacheOther( "item_longjumpcharge" );



	//MODDD - marker. Weapon precache starts here.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// shotgun
	UTIL_PrecacheOtherWeapon( "weapon_shotgun" );
	UTIL_PrecacheOther( "ammo_buckshot" );

	// crowbar
	UTIL_PrecacheOtherWeapon( "weapon_crowbar" );

	// glock
	UTIL_PrecacheOtherWeapon( "weapon_9mmhandgun" );
	UTIL_PrecacheOther( "ammo_9mmclip" );

	// mp5
	UTIL_PrecacheOtherWeapon( "weapon_9mmAR" );
	UTIL_PrecacheOther( "ammo_9mmAR" );
	UTIL_PrecacheOther( "ammo_ARgrenades" );

	// python
	UTIL_PrecacheOtherWeapon( "weapon_357" );
	UTIL_PrecacheOther( "ammo_357" );
	
	// gauss
	UTIL_PrecacheOtherWeapon( "weapon_gauss" );
	UTIL_PrecacheOther( "ammo_gaussclip" );

	// rpg
	UTIL_PrecacheOtherWeapon( "weapon_rpg" );
	UTIL_PrecacheOther( "ammo_rpgclip" );

	// crossbow
	UTIL_PrecacheOtherWeapon( "weapon_crossbow" );
	UTIL_PrecacheOther( "ammo_crossbow" );

	// egon
	UTIL_PrecacheOtherWeapon( "weapon_egon" );

	// tripmine
	UTIL_PrecacheOtherWeapon( "weapon_tripmine" );

	//MODDD NOTE - Wondering about weapons/mine_charge ?  It's covered by weapon_tripmine. So it is guaranteed to be available for the gargantua too.


	// satchel charge
	UTIL_PrecacheOtherWeapon( "weapon_satchel" );

	// hand grenade
	UTIL_PrecacheOtherWeapon("weapon_handgrenade");

	// squeak grenade
	UTIL_PrecacheOtherWeapon( "weapon_snark" );
	//MODDD IMPORTANT - make sure to call "UTIL_PrecacheOtherWeapon" on guns! This registrers their ammo too.
	//MODDD - chumtoad
	UTIL_PrecacheOtherWeapon( "weapon_chumtoad" );

	// hornetgun
	UTIL_PrecacheOtherWeapon( "weapon_hornetgun" );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//MODDD - with all weapons precached, all ammo types are available.  Call this.
	PostWeaponRegistry();




	if ( g_pGameRules->IsDeathmatch() )
	{
		UTIL_PrecacheOther( "weaponbox" );// container for dropped deathmatch weapons
	}

	g_sModelIndexFireball = PRECACHE_MODEL ("sprites/zerogxplode.spr");// fireball
	g_sModelIndexWExplosion = PRECACHE_MODEL ("sprites/WXplo1.spr");// underwater fireball
	g_sModelIndexSmoke = PRECACHE_MODEL ("sprites/steam1.spr");// smoke
	//MODDD - g_sModelIndexBubbles precache moved to util_shared's "PrecacheShared".
	//g_sModelIndexBubbles = PRECACHE_MODEL ("sprites/bubble.spr");//bubbles
	g_sModelIndexBloodSpray = PRECACHE_MODEL ("sprites/bloodspray.spr"); // initial blood
	g_sModelIndexBloodDrop = PRECACHE_MODEL ("sprites/blood.spr"); // splattered blood 

	g_sModelIndexLaser = PRECACHE_MODEL( (char *)g_pModelNameLaser );
	g_sModelIndexLaserDot = PRECACHE_MODEL("sprites/laserdot.spr");


	// used by explosions
	PRECACHE_MODEL ("models/grenade.mdl");
	PRECACHE_MODEL ("sprites/explode1.spr");

	PRECACHE_SOUND ("weapons/debris1.wav");// explosion aftermaths
	PRECACHE_SOUND ("weapons/debris2.wav");// explosion aftermaths
	PRECACHE_SOUND ("weapons/debris3.wav");// explosion aftermaths

	PRECACHE_SOUND ("weapons/grenade_hit1.wav");//grenade
	PRECACHE_SOUND ("weapons/grenade_hit2.wav");//grenade
	PRECACHE_SOUND ("weapons/grenade_hit3.wav");//grenade

	PRECACHE_SOUND ("weapons/bullet_hit1.wav");	// hit by bullet
	PRECACHE_SOUND ("weapons/bullet_hit2.wav");	// hit by bullet
	
	PRECACHE_SOUND ("items/weapondrop1.wav");// weapon falls to the ground
	
}//END OF W_Precache


void ClientPrecache( void ){

	BOOL precacheAllVar = (EASY_CVAR_GET(precacheAll) >= 1);

	//just to be clear. Should be this way by default.
	global_useSentenceSave = FALSE;



	//All things here must default to precache regardless of the soundsentencesave (because global_useSentenceSave is forced to FALSE beforehand above).
	//use "FALSE" as a 2nd argument in any PREACHE_SOUND call to force this not to precache instead (can only play from the soundsentencesave system).

	//Things that may get spammed a lot like debris sounds, foot steps, or must be played from C (pm_shared.c) should remain precached regardless and
	//not have sentence equivalents in sentences.txt. That just wastes sentence space for forever unused sentences.

	//Don't use a blend of sentence equivalents and real precached sound calls for the exact same sound file - it is the worst of both worlds.
	//One entry in the precache list, and one sentence in the sentence list. Keep it all to one or the other.

	//Or in short:

	/*
	
	                    precached?   has sentence?
	NON-SENTENCESAVE:     YES            NO
	    SENTENCESAVE:      NO           YES
	*/


	PRECACHE_SOUND("player/sprayer.wav");		// spray paint sound for PreAlpha
	PRECACHE_SOUND("plats/train_use1.wav");		// use a train
	


	//player/plyrjmp8.wav? probably not.

	//MODDD - console often complains about this not being pre-cached.
	PRECACHE_SOUND("player/pl_jump1.wav");
	PRECACHE_SOUND("player/pl_jump2.wav");

	//MODDD - other precache-'s.  NOT NEEDED ANYMORE.
	//////////////////////////////////////////////////////////////////////////////////////
	/*
	PRECACHE_SOUND("fvox/one.wav");
	PRECACHE_SOUND("fvox/two.wav");

	PRECACHE_SOUND("fvox/three.wav");
	PRECACHE_SOUND("fvox/four.wav");
	PRECACHE_SOUND("fvox/five.wav");
	PRECACHE_SOUND("fvox/six.wav");
	PRECACHE_SOUND("fvox/seven.wav");
	PRECACHE_SOUND("fvox/eight.wav");
	PRECACHE_SOUND("fvox/nine.wav");
	PRECACHE_SOUND("fvox/ten.wav");
	PRECACHE_SOUND("fvox/eleven.wav");
	PRECACHE_SOUND("fvox/twelve.wav");
	PRECACHE_SOUND("fvox/thirteen.wav");
	PRECACHE_SOUND("fvox/fourteen.wav");
	PRECACHE_SOUND("fvox/fifteen.wav");
	PRECACHE_SOUND("fvox/sixteen.wav");
	PRECACHE_SOUND("fvox/seventeen.wav");
	PRECACHE_SOUND("fvox/eighteen.wav");
	PRECACHE_SOUND("fvox/nineteen.wav");

	PRECACHE_SOUND("fvox/twenty.wav");
	PRECACHE_SOUND("fvox/thirty.wav");
	
	PRECACHE_SOUND("fvox/fourty.wav");
	PRECACHE_SOUND("fvox/fifty.wav");
	PRECACHE_SOUND("fvox/sixty.wav");
	PRECACHE_SOUND("fvox/seventy.wav");
	PRECACHE_SOUND("fvox/eighty.wav");
	PRECACHE_SOUND("fvox/ninety.wav");
	PRECACHE_SOUND("fvox/onehundred.wav");
	*/

	/////////////////////////////////////////////////////////////////////////////////////////
	
	//MODDDREMOVE
	PRECACHE_SOUND("player/pl_jumpland2.wav");
	
	//PRECACHE_SOUND("player/pl_fallpain2.wav");  //CURRENTLY UNUSED.	Check pm_shared.c for its only commented out mention.
	PRECACHE_SOUND("player/pl_fallpain3.wav");		
	
	PRECACHE_SOUND("player/pl_step1.wav");		// walk on concrete
	PRECACHE_SOUND("player/pl_step2.wav");
	PRECACHE_SOUND("player/pl_step3.wav");
	PRECACHE_SOUND("player/pl_step4.wav");

	PRECACHE_SOUND("common/npc_step1.wav");		// NPC walk on concrete
	PRECACHE_SOUND("common/npc_step2.wav");
	PRECACHE_SOUND("common/npc_step3.wav");
	PRECACHE_SOUND("common/npc_step4.wav");

	PRECACHE_SOUND("player/pl_metal1.wav");		// walk on metal
	PRECACHE_SOUND("player/pl_metal2.wav");
	PRECACHE_SOUND("player/pl_metal3.wav");
	PRECACHE_SOUND("player/pl_metal4.wav");

	PRECACHE_SOUND("player/pl_dirt1.wav");		// walk on dirt
	PRECACHE_SOUND("player/pl_dirt2.wav");
	PRECACHE_SOUND("player/pl_dirt3.wav");
	PRECACHE_SOUND("player/pl_dirt4.wav");

	PRECACHE_SOUND("player/pl_duct1.wav");		// walk in duct
	PRECACHE_SOUND("player/pl_duct2.wav");
	PRECACHE_SOUND("player/pl_duct3.wav");
	PRECACHE_SOUND("player/pl_duct4.wav");

	PRECACHE_SOUND("player/pl_grate1.wav");		// walk on grate
	PRECACHE_SOUND("player/pl_grate2.wav");
	PRECACHE_SOUND("player/pl_grate3.wav");
	PRECACHE_SOUND("player/pl_grate4.wav");

	PRECACHE_SOUND("player/pl_slosh1.wav");		// walk in shallow water
	PRECACHE_SOUND("player/pl_slosh2.wav");
	PRECACHE_SOUND("player/pl_slosh3.wav");
	PRECACHE_SOUND("player/pl_slosh4.wav");

	PRECACHE_SOUND("player/pl_tile1.wav");		// walk on tile
	PRECACHE_SOUND("player/pl_tile2.wav");
	PRECACHE_SOUND("player/pl_tile3.wav");
	PRECACHE_SOUND("player/pl_tile4.wav");
	PRECACHE_SOUND("player/pl_tile5.wav");

	PRECACHE_SOUND("player/pl_swim1.wav");		// breathe bubbles
	PRECACHE_SOUND("player/pl_swim2.wav");
	PRECACHE_SOUND("player/pl_swim3.wav");
	PRECACHE_SOUND("player/pl_swim4.wav");

	PRECACHE_SOUND("player/pl_ladder1.wav");	// climb ladder rung
	PRECACHE_SOUND("player/pl_ladder2.wav");
	PRECACHE_SOUND("player/pl_ladder3.wav");
	PRECACHE_SOUND("player/pl_ladder4.wav");

	PRECACHE_SOUND("player/pl_wade1.wav");		// wade in water
	PRECACHE_SOUND("player/pl_wade2.wav");
	PRECACHE_SOUND("player/pl_wade3.wav");
	PRECACHE_SOUND("player/pl_wade4.wav");

	PRECACHE_SOUND("debris/wood1.wav");			// hit wood texture
	PRECACHE_SOUND("debris/wood2.wav");
	PRECACHE_SOUND("debris/wood3.wav");

	PRECACHE_SOUND("buttons/spark5.wav");		// hit computer texture
	PRECACHE_SOUND("buttons/spark6.wav");
	PRECACHE_SOUND("debris/glass1.wav");
	PRECACHE_SOUND("debris/glass2.wav");
	PRECACHE_SOUND("debris/glass3.wav");

	// breakables involve some of these, a utility method too, just precache them always now and play without soundsentencesave.
	PRECACHE_SOUND("debris/bustflesh1.wav");
	PRECACHE_SOUND("debris/bustflesh2.wav");
	PRECACHE_SOUND("debris/bustmetal1.wav");
	PRECACHE_SOUND("debris/bustmetal2.wav");
	PRECACHE_SOUND("debris/metal2.wav");
	PRECACHE_SOUND("debris/metal4.wav");
	PRECACHE_SOUND("debris/metal5.wav");
	// for the garg fall.  But really just be forced at this point.
	PRECACHE_SOUND("debris/metal6.wav");

	//MODDD - NEW.  Used by various metal-hit sounds now.  Not the breakable though but that could be changed.
	PRECACHE_SOUND("debris/metal1.wav");
	PRECACHE_SOUND("debris/metal3.wav");

	PRECACHE_SOUND( SOUND_FLASHLIGHT_ON );
	PRECACHE_SOUND( SOUND_FLASHLIGHT_OFF );

// player gib sounds
	PRECACHE_SOUND("common/bodysplat.wav");		




	// Just hard precache these (TRUE), not sure how I feel about playing sentences through pm_shared (even though it looks like it works).
	// It just might be telling if retail never did that.
// player pain sounds
	PRECACHE_SOUND("player/pl_pain2.wav");
	PRECACHE_SOUND("player/pl_pain4.wav");
	PRECACHE_SOUND("player/pl_pain5.wav");
	PRECACHE_SOUND("player/pl_pain6.wav");
	PRECACHE_SOUND("player/pl_pain7.wav");

	//MODDD - now precached.
	PRECACHE_SOUND("player/h2odeath.wav");

	PRECACHE_MODEL("models/player.mdl");

	// hud sounds
	// sounds only played client-side. Those don't need any sort of precaching or sentences for soundsentencesave.
	//PRECACHE_SOUND("common/wpn_hudoff.wav", FALSE);
	//PRECACHE_SOUND("common/wpn_hudon.wav", FALSE);
	//PRECACHE_SOUND("common/wpn_moveselect.wav", FALSE);
	// These are used by the player for the "use" button too (player.cpp; server calls).
	PRECACHE_SOUND("common/wpn_select.wav");
	PRECACHE_SOUND("common/wpn_denyselect.wav");
	

	// geiger sounds
	// Not using soundsentencesave for those either.
	PRECACHE_SOUND("player/geiger6.wav");
	PRECACHE_SOUND("player/geiger5.wav");
	PRECACHE_SOUND("player/geiger4.wav");
	PRECACHE_SOUND("player/geiger3.wav");
	PRECACHE_SOUND("player/geiger2.wav");
	PRECACHE_SOUND("player/geiger1.wav");
	

	//MODD - NEW!
	//NOTICE: adding   ,FALSE);   to a sound precache-entry, as of the current setup, allows
	//it to have this precache line ignored so that sentences.txt can pick it up instead (saving precious precache's)
	PRECACHE_SOUND("weapons/glockSilencerOn.wav");
	PRECACHE_SOUND("weapons/glockSilencerOff.wav");
	
	// gauss fire sounds.
	PRECACHE_SOUND("weapons/electro4.wav");
	PRECACHE_SOUND("weapons/electro5.wav");
	PRECACHE_SOUND("weapons/electro6.wav");


	// NOTICE - being FALSE here actually means, these are part of the soundsentencesave system.  So
	// these are exceptions that get sentences in sentences.txt anyway.
	PRECACHE_SOUND("items/smallmedkit1.wav", FALSE);
	PRECACHE_SOUND("items/medshot4.wav", FALSE);
	PRECACHE_SOUND("items/medshotno1.wav", FALSE);
	PRECACHE_SOUND("items/medcharge4.wav", FALSE);
	PRECACHE_SOUND("items/suitcharge1.wav", FALSE);
	PRECACHE_SOUND("items/suitchargeno1.wav", FALSE);
	PRECACHE_SOUND("items/airtank1.wav", FALSE);
	
	PRECACHE_SOUND( "items/suitchargeok1.wav" );//!!! temporary sound for respawning weapons.
	
	//PRECACHE_SOUND("weapons/reload1.wav");  //only possibly in cbase.cpp (for precacheAmmoPickupSound)
	//PRECACHE_SOUND("weapons/reload2.wav");
	//PRECACHE_SOUND("weapons/reload3.wav");
	PRECACHE_SOUND("common/null.wav");

	// So far used for picking up power canisters (items.cpp) instead of the more randomized gunPickupSound.
	//MODDD TODO - Is that on purpose? Should they be more randomized?
	PRECACHE_SOUND( "items/gunpickup2.wav" );

	PRECACHE_SOUND("items/9mmclip1.wav");
	PRECACHE_SOUND("items/clipinsert1.wav");
	PRECACHE_SOUND("items/cliprelease1.wav");
	PRECACHE_SOUND("weapons/hks1.wav");// H to the K
	PRECACHE_SOUND("weapons/hks2.wav");// H to the K
	PRECACHE_SOUND("weapons/hks3.wav");// H to the K
	PRECACHE_SOUND("weapons/glauncher.wav" );
	PRECACHE_SOUND("weapons/glauncher2.wav" );
	PRECACHE_SOUND("weapons/357_cock1.wav");
	
	PRECACHE_SOUND("weapons/sbarrel1.wav");
	PRECACHE_SOUND("weapons/rocket1.wav");
	PRECACHE_SOUND("weapons/cbar_miss1.wav");

	// NOTE - no need to precache pl_gun sounds here, already precached by the glock.
	// They won't use the soundsentencesave system.
	
	//explosion sounds don't use the soundsentencesave.
	PRECACHE_SOUND("old/explode0.wav");
	PRECACHE_SOUND("old/explode1.wav");
	PRECACHE_SOUND("old/explode2.wav");
	PRECACHE_SOUND("weapons/explode3.wav");
	PRECACHE_SOUND("weapons/explode4.wav");
	PRECACHE_SOUND("weapons/explode5.wav");


	//eh?
	global_useSentenceSave = FALSE;



	PRECACHE_MODEL("models/w_silencer.mdl");


	//if "precacheAllVar" is on, don't precache the grunt here. Already done by precacheAll.
	if (giPrecacheGrunt && !precacheAllVar)
		UTIL_PrecacheOther("monster_human_grunt");

	//MODDD - LINK EVENTS TO FILES, SERVERSIDE
	g_sTrailEngineChoice = PRECACHE_EVENT(1, "events/trail_enginechoice.sc");
	g_sImitation7 = PRECACHE_EVENT(1, "events/imitation7.sc");
	g_sTrail = PRECACHE_EVENT( 1, "events/trail.sc" );
	g_sTrailRA = PRECACHE_EVENT(1, "events/trailra.sc");
	g_sCustomBalls = PRECACHE_EVENT( 1, "events/customballs.sc");
	g_sCustomBallsPowerup = PRECACHE_EVENT( 1, "events/customballsp.sc");
	g_quakeExplosionEffect = PRECACHE_EVENT( 1, "events/quakeexpfx.sc" );
	g_decalGunshotCustomEvent = PRECACHE_EVENT( 1, "events/gunshotnew.sc" );

	g_sFreakyLight = PRECACHE_EVENT( 1, "events/freakylight.sc" );
	g_sFriendlyVomit = PRECACHE_EVENT( 1, "events/friendlyvomit.sc");
	
	g_sFloaterExplode = PRECACHE_EVENT( 1, "events/floaterexplode.sc");
	

	model_explosivestuff = PRECACHE_MODEL( "models/shrapnel.mdl" );

}//END OF ClientPrecache()


//Some extra things called for back in World.cpp.
void ExtraWorldPrecache(){
	
	// sounds used from C physics code
	PRECACHE_SOUND("common/null.wav");				// clears sound channels

	
	PRECACHE_MODEL( "models/hgibs.mdl" );
	PRECACHE_MODEL( "models/agibs.mdl" );

	PRECACHE_SOUND ("weapons/ric1.wav");
	PRECACHE_SOUND ("weapons/ric2.wav");
	PRECACHE_SOUND ("weapons/ric3.wav");
	PRECACHE_SOUND ("weapons/ric4.wav");
	PRECACHE_SOUND ("weapons/ric5.wav");
}//END OF ExtraWorldPrecache()





// NOTE: postprocessor commands seem to accept just one condition at a time, don't try "&&", other conditions will be ignored.
// However, "#if" already incorporates a part of "#ifdef": if the constant has not been defined, "0" is seen instead.
// But, a constant can still be defined as 0 and pass #ifdef.


// ALSO, this method is still called regardless of any CVArs like "precacheAll" or "soundSentenceSave".
// Those CVars are still checked for whether to precache some things on any level start regarldess of monsters spawned at
// startup or skip some sounds in case the "soundSentenceSave" is on.
// And renamed with "method_" to bedifferentiated from the CVar of the same name floating around
// as a macro now.
void method_precacheAll(void){
	//easyPrintLine("LANGUAGE: %d", g_Language);

#if secret_ymg == 1
	PRECACHE_SOUND("meme/ymg.wav", TRUE);
#endif


	easyPrintLine("method_precacheAll:::%.1f %.1f", EASY_CVAR_GET(precacheAll), EASY_CVAR_GET(soundSentenceSave));

	PRECACHE_SOUND("items/clipinsert1.wav");
	PRECACHE_SOUND("items/cliprelease1.wav");

	
	//Never hurts to be safe.  Any precache calls throughout this method are deliberate, don't try to skip with the soundsentencesave system.
	global_useSentenceSave = FALSE;

	//The things in here were unconditionally precached
	W_Precache();
	ClientPrecache();
	ExtraWorldPrecache();

	
	//A call to the "soundSentenceSave" CVar has been moved to game.cpp.
	
	BOOL soundSentenceSaveVar = (EASY_CVAR_GET(soundSentenceSave) >= 1);
	BOOL precacheAllVar = (EASY_CVAR_GET(precacheAll) >= 1);


	g_sGaussBallSprite = PRECACHE_MODEL ("sprites/hotglow.spr");
	g_sBallVomitSprite = PRECACHE_MODEL ("sprites/hotglow_green.spr");
	g_sBallForceFieldSprite = PRECACHE_MODEL ("sprites/hotglow_ff.spr");
	

	//reset.
	globalPSEUDO_canApplyGermanCensorship = 0;

	globalPSEUDO_germanModel_hgibFound = FALSE;
	globalPSEUDO_germanModel_scientistFound = FALSE;
	globalPSEUDO_germanModel_barneyFound = FALSE;
	globalPSEUDO_germanModel_hgruntFound = FALSE;
	globalPSEUDO_germanModel_hassaultFound = FALSE;


	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(sv_germancensorship) == -1){
		//global_germanCensorship = EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(sv_germancensorship);

		//"gets the point across"
		//global_allowGermanModels = EASY_CVAR_GET_DEBUGONLY(allowGermanModels);
		// wait.  whut.  set it to 'itself'... why.
		// Perhaps to guarantee it is the same as the PSEUDO version as to not trigger
		// some change the user didn't call for.
		EASY_CVAR_SET_DEBUGONLY(allowGermanModels, EASY_CVAR_GET_DEBUGONLY(allowGermanModels))
		g_queueCVarHiddenSave = TRUE;
	}

	//!!!!tryLoadGermanGibs  !!!!!

	//easyForcePrintLine("ARE YOU flaming %.2f %.2f", EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(sv_germancensorship), EASY_CVAR_GET_DEBUGONLY(allowGermanModels));
	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(sv_germancensorship) == 1 && EASY_CVAR_GET_DEBUGONLY(allowGermanModels) == 1){
		
		if( !(globalPSEUDO_germanModel_hgibFound = verifyModelExists(GERMAN_GIB_PATH)) ){
			easyForcePrintLine("***NOTICE: model \"%s\" missing.  Gibs will not spawn.***", GERMAN_GIB_PATH);
		}
		if( !(globalPSEUDO_germanModel_scientistFound = verifyModelExists("models/g_scientist.mdl")) ){
			easyForcePrintLine("***NOTICE: model \"%s\" missing.  Scientist model maintained.***", "models/g_scientist.mdl");
		}
		if( !(globalPSEUDO_germanModel_barneyFound = verifyModelExists("models/g_barney.mdl")) ){
			easyForcePrintLine("***NOTICE: model \"%s\" missing.  Barney model maintained.***", "models/g_barney.mdl");
		}
		if( !(globalPSEUDO_germanModel_hgruntFound = verifyModelExists("models/g_hgrunt.mdl")) ){
			easyForcePrintLine("***NOTICE: model \"%s\" missing.  HGrunt model maintained.***", "models/g_hgrunt.mdl");
		}
		if( !(globalPSEUDO_germanModel_hassaultFound = verifyModelExists("models/g_hassault.mdl")) ){
			easyForcePrintLine("***NOTICE: model \"%s\" missing.  HAssault model maintained.***", "models/g_hassault.mdl");
		}


		//PRECACHE_MODEL( GERMAN_GIB_PATH );
		//PRECACHE_MODEL("models/g_scientist.mdl");
		//?

		//valid now.  Can use german models.
		globalPSEUDO_canApplyGermanCensorship = 1;
	}else{
		
		
	}
	

	if(EASY_CVAR_GET_DEBUGONLY(sentryCanGib) == 3 || EASY_CVAR_GET_DEBUGONLY(miniturretCanGib) == 3 || EASY_CVAR_GET_DEBUGONLY(turretCanGib) == 3){
		PRECACHE_MODEL((char*)aryGibInfo[GIB_EXTRAMETAL_1_ID].modelPath);
	}
	if(EASY_CVAR_GET_DEBUGONLY(sentryCanGib) == 4 || EASY_CVAR_GET_DEBUGONLY(miniturretCanGib) == 4 || EASY_CVAR_GET_DEBUGONLY(turretCanGib) == 4){
		PRECACHE_MODEL((char*)aryGibInfo[GIB_EXTRAMETAL_2_ID].modelPath);
	}
	if(EASY_CVAR_GET_DEBUGONLY(sentryCanGib) == 5 || EASY_CVAR_GET_DEBUGONLY(miniturretCanGib) == 5 || EASY_CVAR_GET_DEBUGONLY(turretCanGib) == 5){
		PRECACHE_MODEL((char*)aryGibInfo[GIB_EXTRAMETAL_3_ID].modelPath);
	}
	if(EASY_CVAR_GET_DEBUGONLY(sentryCanGib) == 6 || EASY_CVAR_GET_DEBUGONLY(miniturretCanGib) == 6 || EASY_CVAR_GET_DEBUGONLY(turretCanGib) == 6){
		PRECACHE_MODEL((char*)aryGibInfo[GIB_EXTRAMETAL_4_ID].modelPath);
	}
	if(EASY_CVAR_GET_DEBUGONLY(sentryCanGib) == 7 || EASY_CVAR_GET_DEBUGONLY(miniturretCanGib) == 7 || EASY_CVAR_GET_DEBUGONLY(turretCanGib) == 7){
		PRECACHE_MODEL((char*)aryGibInfo[GIB_EXTRAMETAL_5_ID].modelPath);
	}
	if(EASY_CVAR_GET_DEBUGONLY(sentryCanGib) == 8 || EASY_CVAR_GET_DEBUGONLY(miniturretCanGib) == 8 || EASY_CVAR_GET_DEBUGONLY(turretCanGib) == 8){
		PRECACHE_MODEL((char*)aryGibInfo[GIB_EXTRAMETAL_6_ID].modelPath);
	}
		

	globalPSEUDO_allowGermanModelsMem = EASY_CVAR_GET_DEBUGONLY(allowGermanModels);
	//globalPSEUDO_germanCensorshipMem = EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(sv_germancensorship);
	


	//MODDD - precache unconditionally now. Never call these through sentences.
	PRECACHE_SOUND("common/bodydrop3.wav", TRUE);
	PRECACHE_SOUND("common/bodydrop4.wav", TRUE);


	if(precacheAllVar){
		//counts for this.
		giPrecacheGrunt = 1;

		//NOTE: although this sound occurs in gargantua.cpp (and is referred to for the sound-sentence-save trick to work),
		//      it can still be called by a section in monsters.cpp that receives sound-calls from the map.  Seems to happen on any map with a gargantua spawned?
		PRECACHE_SOUND("garg/gar_attack1.wav", TRUE);

		
		//Only do this if precacheAllVar is on and soundSentenceSave is off.
		//precacheAllVar being off suggests each entity will precache this stuff as it occurs in the map for the first time.
		//soundSentenceSaveVar being on suggests the soundSentenceSave system is avoiding hte need for precacheing these things to begin with.
		// (Sounds require sentence.txt equivalents to skip precaches and be played there instead of normally, the file's name follows a format for easy automatic 
		//  translation between plain file name and the sentence name).
		if(!soundSentenceSaveVar){
			//NOTICE - bodysplat sound already precached in ClientPrecache to be guaranteed to get precached, it may be played normally even with the soundsentencesave on.


			PRECACHE_SOUND("weapons/mortarhit.wav", TRUE);

			/*
			//NOTE: don't bother precaching snarks.  They are likely already precached by the player (precaches for all weapons already from retail to never crash on spawning / getting weapons not on map)
			PRECACHE_MODEL("models/w_squeak.mdl");
			PRECACHE_SOUND("squeek/sqk_blast1.wav");
			PRECACHE_SOUND("common/bodysplat.wav");
			PRECACHE_SOUND("squeek/sqk_die1.wav");
			PRECACHE_SOUND("squeek/sqk_hunt1.wav");
			PRECACHE_SOUND("squeek/sqk_hunt2.wav");
			PRECACHE_SOUND("squeek/sqk_hunt3.wav");
			PRECACHE_SOUND("squeek/sqk_deploy1.wav");
			*/

			//NOTICE - not worth making the precache standard attack miss / hit sound methods static.  All their sounds are already included in here.
			//CBaseMonster::precacheStandardMeleeAttackMissSounds();
			//CBaseMonster::precacheStandardMeleeAttackHitSounds();

			PRECACHE_SOUND("zombie/claw_miss1.wav");
			PRECACHE_SOUND("zombie/claw_miss2.wav");
			PRECACHE_SOUND("zombie/claw_strike1.wav");
			PRECACHE_SOUND("zombie/claw_strike2.wav");
			PRECACHE_SOUND("zombie/claw_strike3.wav");

			PRECACHE_SOUND("garg/gar_flameoff1.wav");
			PRECACHE_SOUND("garg/gar_flameon1.wav");
			PRECACHE_SOUND("garg/gar_flamerun1.wav");
			PRECACHE_SOUND("garg/gar_step1.wav");
			PRECACHE_SOUND("garg/gar_step2.wav");
			PRECACHE_SOUND("garg/gar_idle1.wav");
			PRECACHE_SOUND("garg/gar_idle2.wav");
			PRECACHE_SOUND("garg/gar_idle3.wav");
			PRECACHE_SOUND("garg/gar_idle4.wav");
			PRECACHE_SOUND("garg/gar_idle5.wav");
			PRECACHE_SOUND("garg/gar_attack1.wav");
			PRECACHE_SOUND("garg/gar_attack2.wav");
			PRECACHE_SOUND("garg/gar_attack3.wav");
			PRECACHE_SOUND("garg/gar_alert1.wav");
			PRECACHE_SOUND("garg/gar_alert2.wav");
			PRECACHE_SOUND("garg/gar_alert3.wav");
			PRECACHE_SOUND("garg/gar_pain1.wav");
			PRECACHE_SOUND("garg/gar_pain2.wav");
			PRECACHE_SOUND("garg/gar_pain3.wav");
			PRECACHE_SOUND("garg/gar_stomp1.wav");
			PRECACHE_SOUND("garg/gar_breathe1.wav");
			PRECACHE_SOUND("garg/gar_breathe2.wav");
			PRECACHE_SOUND("garg/gar_breathe3.wav");

			PRECACHE_SOUND("garg/gar_die1.wav");
			PRECACHE_SOUND("garg/gar_die2.wav");


			/*
			gargantua ricochet sound. Unused, forget about this.
			#if 0
			PRECACHE_SOUND("weapons/ric1.wav");
			PRECACHE_SOUND("weapons/ric2.wav");
			PRECACHE_SOUND("weapons/ric3.wav");
			PRECACHE_SOUND("weapons/ric4.wav");
			PRECACHE_SOUND("weapons/ric5.wav");
			#else
			PRECACHE_SOUND("debris/metal4.wav");
			PRECACHE_SOUND("debris/metal6.wav");
			#endif
			*/
			
			//see debris/zap# 1 - 8 for some more sounds like weapons/electro#.

			PRECACHE_SOUND("ichy/ichy_idle1.wav");
			PRECACHE_SOUND("ichy/ichy_idle2.wav");
			PRECACHE_SOUND("ichy/ichy_idle3.wav");
			PRECACHE_SOUND("ichy/ichy_idle4.wav");
			PRECACHE_SOUND("ichy/ichy_alert2.wav");
			PRECACHE_SOUND("ichy/ichy_alert3.wav");
			PRECACHE_SOUND("ichy/ichy_attack1.wav");
			PRECACHE_SOUND("ichy/ichy_attack2.wav");
			PRECACHE_SOUND("ichy/ichy_bite1.wav");
			PRECACHE_SOUND("ichy/ichy_bite2.wav");
			PRECACHE_SOUND("ichy/ichy_pain2.wav");
			PRECACHE_SOUND("ichy/ichy_pain3.wav");
			PRECACHE_SOUND("ichy/ichy_pain5.wav");
			PRECACHE_SOUND("ichy/ichy_die2.wav");
			PRECACHE_SOUND("ichy/ichy_die4.wav");
			PRECACHE_SOUND("aslave/slv_pain1.wav");
			PRECACHE_SOUND("aslave/slv_pain2.wav");
			PRECACHE_SOUND("aslave/slv_die1.wav");
			PRECACHE_SOUND("aslave/slv_die2.wav");
			PRECACHE_SOUND("debris/zap1.wav");
			PRECACHE_SOUND("debris/zap4.wav");
			//PRECACHE_SOUND("weapons/electro4.wav");    //no, now done regardless of soundsentencesave above. Client precaches it, so just play it straight.
			PRECACHE_SOUND("hassault/hw_shoot1.wav");
			PRECACHE_SOUND("zombie/zo_pain2.wav");
			PRECACHE_SOUND("headcrab/hc_headbite.wav");
			PRECACHE_SOUND("roach/rch_die.wav");
			PRECACHE_SOUND("roach/rch_walk.wav");
			PRECACHE_SOUND("roach/rch_smash.wav");
			PRECACHE_SOUND("scientist/sci_pain1.wav");
			PRECACHE_SOUND("scientist/sci_pain2.wav");
			PRECACHE_SOUND("scientist/sci_pain3.wav");
			PRECACHE_SOUND("scientist/sci_pain4.wav");
			PRECACHE_SOUND("scientist/sci_pain5.wav");
			
			PRECACHE_SOUND("scientist/sci_die1.wav");
			PRECACHE_SOUND("scientist/sci_die2.wav");
			PRECACHE_SOUND("scientist/sci_die3.wav");
			PRECACHE_SOUND("scientist/sci_die4.wav");

			PRECACHE_SOUND("apache/ap_rotor4.wav");
	
			PRECACHE_SOUND("apache/ap_rotor1.wav");
			PRECACHE_SOUND("apache/ap_rotor2.wav");
			PRECACHE_SOUND("apache/ap_rotor3.wav");
			PRECACHE_SOUND("apache/ap_whine1.wav");
	
			PRECACHE_SOUND("turret/tu_fire1.wav");

	
			PRECACHE_SOUND("barnacle/bcl_alert2.wav");
			PRECACHE_SOUND("barnacle/bcl_bite3.wav");
			PRECACHE_SOUND("barnacle/bcl_chew1.wav");
			PRECACHE_SOUND("barnacle/bcl_chew2.wav");
			PRECACHE_SOUND("barnacle/bcl_chew3.wav");
			PRECACHE_SOUND("barnacle/bcl_die1.wav");
			PRECACHE_SOUND("barnacle/bcl_die3.wav");
			PRECACHE_SOUND("turret/tu_ping.wav");
			PRECACHE_SOUND("turret/tu_active2.wav");
			PRECACHE_SOUND("turret/tu_die.wav");
			PRECACHE_SOUND("turret/tu_die2.wav");
			PRECACHE_SOUND("turret/tu_die3.wav");
			PRECACHE_SOUND("turret/tu_deploy.wav");
			PRECACHE_SOUND("turret/tu_spinup.wav");
			PRECACHE_SOUND("turret/tu_spindown.wav");
			PRECACHE_SOUND("turret/tu_search.wav");
			PRECACHE_SOUND("turret/tu_alert.wav");


			PRECACHE_SOUND("zombie/zo_attack1.wav");
			PRECACHE_SOUND("zombie/zo_attack2.wav");
			PRECACHE_SOUND("zombie/zo_idle1.wav");
			PRECACHE_SOUND("zombie/zo_idle2.wav");
			PRECACHE_SOUND("zombie/zo_idle3.wav");
			PRECACHE_SOUND("zombie/zo_idle4.wav");
			PRECACHE_SOUND("zombie/zo_alert10.wav");
			PRECACHE_SOUND("zombie/zo_alert20.wav");
			PRECACHE_SOUND("zombie/zo_alert30.wav");
			PRECACHE_SOUND("zombie/zo_pain1.wav");
			PRECACHE_SOUND("controller/con_attack1.wav");
			PRECACHE_SOUND("controller/con_attack2.wav");
			PRECACHE_SOUND("controller/con_attack3.wav");
			PRECACHE_SOUND("controller/con_idle1.wav");
			PRECACHE_SOUND("controller/con_idle2.wav");
			PRECACHE_SOUND("controller/con_idle3.wav");
			PRECACHE_SOUND("controller/con_idle4.wav");
			PRECACHE_SOUND("controller/con_idle5.wav");
			PRECACHE_SOUND("controller/con_alert1.wav");
			PRECACHE_SOUND("controller/con_alert2.wav");
			PRECACHE_SOUND("controller/con_alert3.wav");
			PRECACHE_SOUND("controller/con_pain1.wav");
			PRECACHE_SOUND("controller/con_pain2.wav");
			PRECACHE_SOUND("controller/con_pain3.wav");
			PRECACHE_SOUND("controller/con_die1.wav");
			PRECACHE_SOUND("controller/con_die2.wav");
			PRECACHE_SOUND("x/x_teleattack1.wav");
			PRECACHE_SOUND("X/x_attack1.wav");
			PRECACHE_SOUND("X/x_attack2.wav");
			PRECACHE_SOUND("X/x_attack3.wav");
			PRECACHE_SOUND("X/x_ballattack1.wav");
			PRECACHE_SOUND("X/x_shoot1.wav");
			PRECACHE_SOUND("X/x_recharge1.wav");
			PRECACHE_SOUND("X/x_recharge2.wav");
			PRECACHE_SOUND("X/x_recharge3.wav");
			PRECACHE_SOUND("X/x_laugh1.wav");
			PRECACHE_SOUND("X/x_laugh2.wav");
			PRECACHE_SOUND("X/x_pain1.wav");
			PRECACHE_SOUND("X/x_pain2.wav");
			PRECACHE_SOUND("X/x_die1.wav");
			PRECACHE_SOUND("debris/beamstart7.wav");

			//PRECACHE_SOUND("ambience/flies.wav");
			//PRECACHE_SOUND("ambience/squirm2.wav");
			PRECACHE_SOUND("tentacle/te_alert1.wav");
			PRECACHE_SOUND("tentacle/te_alert2.wav");
			PRECACHE_SOUND("tentacle/te_flies1.wav");
			PRECACHE_SOUND("tentacle/te_squirm2.wav");
			PRECACHE_SOUND("tentacle/te_move1.wav");
			PRECACHE_SOUND("tentacle/te_move2.wav");
			PRECACHE_SOUND("tentacle/te_roar1.wav");
			PRECACHE_SOUND("tentacle/te_roar2.wav");
			PRECACHE_SOUND("tentacle/te_search1.wav");
			PRECACHE_SOUND("tentacle/te_search2.wav");
			PRECACHE_SOUND("tentacle/te_sing1.wav");
			PRECACHE_SOUND("tentacle/te_sing2.wav");
			PRECACHE_SOUND("tentacle/te_strike1.wav");
			PRECACHE_SOUND("tentacle/te_strike2.wav");
			PRECACHE_SOUND("tentacle/te_swing1.wav");
			PRECACHE_SOUND("tentacle/te_swing2.wav");

			PRECACHE_SOUND("leech/leech_bite1.wav");
			PRECACHE_SOUND("leech/leech_bite2.wav");
			PRECACHE_SOUND("leech/leech_bite3.wav");
			PRECACHE_SOUND("leech/leech_alert1.wav");
			PRECACHE_SOUND("leech/leech_alert2.wav");


			PRECACHE_SOUND("houndeye/he_alert1.wav");
			PRECACHE_SOUND("houndeye/he_alert2.wav");
			PRECACHE_SOUND("houndeye/he_alert3.wav");
			PRECACHE_SOUND("houndeye/he_die1.wav");
			PRECACHE_SOUND("houndeye/he_die2.wav");
			PRECACHE_SOUND("houndeye/he_die3.wav");
			PRECACHE_SOUND("houndeye/he_idle1.wav");
			PRECACHE_SOUND("houndeye/he_idle2.wav");
			PRECACHE_SOUND("houndeye/he_idle3.wav");
			PRECACHE_SOUND("houndeye/he_hunt1.wav");
			PRECACHE_SOUND("houndeye/he_hunt2.wav");
			PRECACHE_SOUND("houndeye/he_hunt3.wav");
			PRECACHE_SOUND("houndeye/he_pain1.wav");
			PRECACHE_SOUND("houndeye/he_pain3.wav");
			PRECACHE_SOUND("houndeye/he_pain4.wav");
			PRECACHE_SOUND("houndeye/he_pain5.wav");
			PRECACHE_SOUND("houndeye/he_attack1.wav");
			PRECACHE_SOUND("houndeye/he_attack3.wav");
			PRECACHE_SOUND("houndeye/he_blast1.wav");
			PRECACHE_SOUND("houndeye/he_blast2.wav");
			PRECACHE_SOUND("houndeye/he_blast3.wav");
			//MODDD - this is an addition.
			PRECACHE_SOUND("houndeye/he_idle4.wav");
			PRECACHE_SOUND("houndeye/he_hunt4.wav");
			PRECACHE_SOUND("houndeye/he_pain2.wav");
			PRECACHE_SOUND("houndeye/he_attack2.wav");


			PRECACHE_SOUND("debris/beamstart1.wav");
			PRECACHE_SOUND("gonarch/gon_childdie1.wav");
			PRECACHE_SOUND("gonarch/gon_childdie2.wav");
			PRECACHE_SOUND("gonarch/gon_childdie3.wav");
			PRECACHE_SOUND("gonarch/gon_sack1.wav");
			PRECACHE_SOUND("gonarch/gon_sack2.wav");
			PRECACHE_SOUND("gonarch/gon_sack3.wav");
			PRECACHE_SOUND("gonarch/gon_die1.wav");
			PRECACHE_SOUND("gonarch/gon_attack1.wav");
			PRECACHE_SOUND("gonarch/gon_attack2.wav");
			PRECACHE_SOUND("gonarch/gon_attack3.wav");
			PRECACHE_SOUND("gonarch/gon_birth1.wav");
			PRECACHE_SOUND("gonarch/gon_birth2.wav");
			PRECACHE_SOUND("gonarch/gon_birth3.wav");
			PRECACHE_SOUND("gonarch/gon_alert1.wav");
			PRECACHE_SOUND("gonarch/gon_alert2.wav");
			PRECACHE_SOUND("gonarch/gon_alert3.wav");
			PRECACHE_SOUND("gonarch/gon_pain2.wav");
			PRECACHE_SOUND("gonarch/gon_pain4.wav");
			PRECACHE_SOUND("gonarch/gon_pain5.wav");
			PRECACHE_SOUND("gonarch/gon_step1.wav");
			PRECACHE_SOUND("gonarch/gon_step2.wav");
			PRECACHE_SOUND("gonarch/gon_step3.wav");
	
			PRECACHE_SOUND("bullchicken/bc_acid1.wav");
			PRECACHE_SOUND("bullchicken/bc_spithit1.wav");
			PRECACHE_SOUND("bullchicken/bc_spithit2.wav");

			// Unused bullsquid sounds, now for the floater's spit effect to use
			PRECACHE_SOUND("bullchicken/bc_attack1.wav");
			PRECACHE_SOUND("bullchicken/bc_acid2.wav");
			PRECACHE_SOUND("bullchicken/bc_spithit3.wav");

			PRECACHE_SOUND("bullchicken/bc_attack2.wav");
			PRECACHE_SOUND("bullchicken/bc_attack3.wav");
			PRECACHE_SOUND("bullchicken/bc_die1.wav");
			PRECACHE_SOUND("bullchicken/bc_die2.wav");
			PRECACHE_SOUND("bullchicken/bc_die3.wav");
			PRECACHE_SOUND("bullchicken/bc_idle1.wav");
			PRECACHE_SOUND("bullchicken/bc_idle2.wav");
			PRECACHE_SOUND("bullchicken/bc_idle3.wav");
			PRECACHE_SOUND("bullchicken/bc_idle4.wav");
			PRECACHE_SOUND("bullchicken/bc_idle5.wav");
			PRECACHE_SOUND("bullchicken/bc_pain1.wav");
			PRECACHE_SOUND("bullchicken/bc_pain2.wav");
			PRECACHE_SOUND("bullchicken/bc_pain3.wav");
			PRECACHE_SOUND("bullchicken/bc_pain4.wav");
			PRECACHE_SOUND("bullchicken/bc_attackgrowl.wav");
			PRECACHE_SOUND("bullchicken/bc_attackgrowl2.wav");
			PRECACHE_SOUND("bullchicken/bc_attackgrowl3.wav");
			PRECACHE_SOUND("bullchicken/bc_bite2.wav");
			PRECACHE_SOUND("bullchicken/bc_bite3.wav");



			PRECACHE_SOUND("barney/ba_attack1.wav");
			PRECACHE_SOUND("barney/ba_attack2.wav");
			PRECACHE_SOUND("barney/ba_pain1.wav");
			PRECACHE_SOUND("barney/ba_pain2.wav");
			PRECACHE_SOUND("barney/ba_pain3.wav");
			PRECACHE_SOUND("barney/ba_die1.wav");
			PRECACHE_SOUND("barney/ba_die2.wav");
			PRECACHE_SOUND("barney/ba_die3.wav");
			PRECACHE_SOUND("headcrab/hc_idle1.wav");
			PRECACHE_SOUND("headcrab/hc_idle2.wav");
			PRECACHE_SOUND("headcrab/hc_idle3.wav");
			PRECACHE_SOUND("headcrab/hc_alert1.wav");
			PRECACHE_SOUND("headcrab/hc_pain1.wav");
			PRECACHE_SOUND("headcrab/hc_pain2.wav");
			PRECACHE_SOUND("headcrab/hc_pain3.wav");
			PRECACHE_SOUND("headcrab/hc_attack1.wav");
			PRECACHE_SOUND("headcrab/hc_attack2.wav");
			PRECACHE_SOUND("headcrab/hc_attack3.wav");
			PRECACHE_SOUND("headcrab/hc_die1.wav");
			PRECACHE_SOUND("headcrab/hc_die2.wav");
			PRECACHE_SOUND("agrunt/ag_attack1.wav");
			PRECACHE_SOUND("agrunt/ag_attack2.wav");
			PRECACHE_SOUND("agrunt/ag_attack3.wav");
			PRECACHE_SOUND("agrunt/ag_die1.wav");
			PRECACHE_SOUND("agrunt/ag_die4.wav");
			PRECACHE_SOUND("agrunt/ag_die5.wav");
			PRECACHE_SOUND("agrunt/ag_pain1.wav");
			PRECACHE_SOUND("agrunt/ag_pain2.wav");
			PRECACHE_SOUND("agrunt/ag_pain3.wav");
			PRECACHE_SOUND("agrunt/ag_pain4.wav");
			PRECACHE_SOUND("agrunt/ag_pain5.wav");
			PRECACHE_SOUND("agrunt/ag_idle1.wav");
			PRECACHE_SOUND("agrunt/ag_idle2.wav");
			PRECACHE_SOUND("agrunt/ag_idle3.wav");
			PRECACHE_SOUND("agrunt/ag_idle4.wav");
			PRECACHE_SOUND("agrunt/ag_alert1.wav");
			PRECACHE_SOUND("agrunt/ag_alert3.wav");
			PRECACHE_SOUND("agrunt/ag_alert4.wav");
			PRECACHE_SOUND("agrunt/ag_alert5.wav");
			PRECACHE_SOUND("agrunt/ag_fire1.wav");
			PRECACHE_SOUND("agrunt/ag_fire2.wav");
			PRECACHE_SOUND("agrunt/ag_fire3.wav");

			//MODDD - new
			PRECACHE_SOUND("agrunt/ag_powerup.wav");

			PRECACHE_SOUND("hornet/ag_buzz1.wav");
			PRECACHE_SOUND("hornet/ag_buzz2.wav");
			PRECACHE_SOUND("hornet/ag_buzz3.wav");
			PRECACHE_SOUND("hornet/ag_hornethit1.wav");
			PRECACHE_SOUND("hornet/ag_hornethit2.wav");
			PRECACHE_SOUND("hornet/ag_hornethit3.wav");
			PRECACHE_SOUND("hgrunt/gr_mgun1.wav");
			PRECACHE_SOUND("hgrunt/gr_mgun2.wav");
			PRECACHE_SOUND("hgrunt/gr_die1.wav");
			PRECACHE_SOUND("hgrunt/gr_die2.wav");
			PRECACHE_SOUND("hgrunt/gr_die3.wav");
			PRECACHE_SOUND("hgrunt/gr_pain1.wav");
			PRECACHE_SOUND("hgrunt/gr_pain2.wav");
			PRECACHE_SOUND("hgrunt/gr_pain3.wav");
			PRECACHE_SOUND("hgrunt/gr_pain4.wav");
			PRECACHE_SOUND("hgrunt/gr_pain5.wav");
			PRECACHE_SOUND("hgrunt/gr_reload1.wav");


			PRECACHE_SOUND("scientist/scream25.wav");
			
			PRECACHE_SOUND("hassault/hw_spin.wav");	
			PRECACHE_SOUND("hassault/hw_spinup.wav");	
			PRECACHE_SOUND("hassault/hw_spindown.wav");	
			PRECACHE_SOUND("hassault/hw_shoot1.wav");	
			PRECACHE_SOUND("hassault/hw_shoot2.wav");
			PRECACHE_SOUND("hassault/hw_shoot3.wav");
			PRECACHE_SOUND("hassault/hw_gun4.wav");

			PRECACHE_SOUND("hgrunt/gr_die1.wav");
			PRECACHE_SOUND("hgrunt/gr_die2.wav");
			PRECACHE_SOUND("hgrunt/gr_die3.wav");
	

			//redundant with existing plain sentences, no need to do this.
			//PRECACHE_SOUND("hgrunt/gr_idle1.wav");
			//PRECACHE_SOUND("hgrunt/gr_idle2.wav");
			//PRECACHE_SOUND("hgrunt/gr_idle3.wav");

			PRECACHE_SOUND("hassault/hw_alert.wav");




			//MODDD TODO MAJOR - include chumtoad sounds if not yet handled, and handle sounds for...

			//friendly
			PRECACHE_SOUND("friendly/friendly_horror.wav");
			PRECACHE_SOUND("friendly/friendly_horror_end.wav");
			PRECACHE_SOUND("friendly/friendly_horror_start.wav");
			
			
			//"weapons/electro4.wav",    already done.
			//PRECACHE_SOUND("weapons/electro5.wav");  ditto.
			//PRECACHE_SOUND("weapons/electro6.wav");
			PRECACHE_SOUND("scientist/scream01.wav");
			PRECACHE_SOUND("scientist/scream02.wav");
			//chew sounds done above.
			PRECACHE_SOUND("barnacle/bcl_tongue1.wav");

			//snapbug

			//archer

			//kingpin
	
			PRECACHE_SOUND("ambience/zapmachine.wav");
			PRECACHE_SOUND("ambience/particle_suck1.wav");
			PRECACHE_SOUND("debris/zap3.wav");
			PRECACHE_SOUND("debris/zap6.wav");
			PRECACHE_SOUND("debris/zap8.wav");
			PRECACHE_SOUND("ambience/alien_chatter.wav");
			PRECACHE_SOUND("x/x_shoot1.wav");
			PRECACHE_SOUND("debris/beamstart4.wav");
			PRECACHE_SOUND("debris/beamstart6.wav");
			PRECACHE_SOUND("debris/beamstart8.wav");
			PRECACHE_SOUND("debris/beamstart10.wav");
			PRECACHE_SOUND("debris/beamstart11.wav");
			PRECACHE_SOUND("debris/beamstart15.wav");
			
			//floater

			//flyer?



			/*
	
			PRECACHE_SOUND("buttons/lever1.wav");
			PRECACHE_SOUND("buttons/lever2.wav");
			PRECACHE_SOUND("buttons/lever3.wav");
			PRECACHE_SOUND("buttons/lever4.wav");
			PRECACHE_SOUND("buttons/lever5.wav");

			PRECACHE_SOUND("debris/bustmetal1.wav");
			PRECACHE_SOUND("debris/bustmetal2.wav");
	
			PRECACHE_SOUND("debris/bustconcrete1.wav");
			PRECACHE_SOUND("debris/bustmetal2.wav");
			PRECACHE_SOUND("debris/bustglass1.wav");
			PRECACHE_SOUND("debris/bustglass2.wav");
			PRECACHE_SOUND("debris/bustglass3.wav");
			PRECACHE_SOUND("doors/doorstop1.wav");
			PRECACHE_SOUND("doors/doorstop2.wav");
			PRECACHE_SOUND("doors/doorstop3.wav");
			PRECACHE_SOUND("doors/doorstop4.wav");
			PRECACHE_SOUND("doors/doorstop5.wav");
			PRECACHE_SOUND("doors/doorstop6.wav");
			PRECACHE_SOUND("doors/doorstop7.wav");
			PRECACHE_SOUND("doors/doorstop8.wav");

			PRECACHE_SOUND("doors/doormove1.wav");
			PRECACHE_SOUND("doors/doormove2.wav");
			PRECACHE_SOUND("doors/doormove3.wav");
			PRECACHE_SOUND("doors/doormove4.wav");
			PRECACHE_SOUND("doors/doormove5.wav");
			PRECACHE_SOUND("doors/doormove6.wav");
			PRECACHE_SOUND("doors/doormove7.wav");
			PRECACHE_SOUND("doors/doormove8.wav");
			PRECACHE_SOUND("doors/doormove9.wav");
			PRECACHE_SOUND("doors/doormove10.wav");

			PRECACHE_SOUND("ambience/drips.wav");
			PRECACHE_SOUND("ambience/labmoan.wav");
			PRECACHE_SOUND("ambience/mechwhine.wav");
			PRECACHE_SOUND("ambience/wind1.wav");
			PRECACHE_SOUND("ambience/wind2.wav");
	
			PRECACHE_SOUND("holo/tr_holo_nicejob.wav");
			PRECACHE_SOUND("debris/flesh6.wav");

			*/

		}//END OF if(!soundSentenceSaveVar)



	//can we do these too?
	//NOTICE: unnecessary.  world.cpp already handles the gibs (also, don't forget agibs.mdl).
	//PRECACHE_MODEL ("models/germanygibs.mdl");
	//PRECACHE_MODEL ("models/hgibs.mdl");


	// is that necessary?
	PRECACHE_MODEL( "models/can.mdl" );
	PRECACHE_MODEL("sprites/laserdot.spr");

	PRECACHE_MODEL("models/stukabat.mdl");

	PRECACHE_MODEL("models/garg.mdl");
	PRECACHE_MODEL("sprites/gargeye1.spr");
	PRECACHE_MODEL("sprites/xbeam3.spr");
	PRECACHE_MODEL("models/metalplategibs.mdl");
	PRECACHE_MODEL("models/gman.mdl");
	PRECACHE_MODEL("models/icky.mdl");
	
	PRECACHE_MODEL("models/islave.mdl");
	PRECACHE_MODEL("sprites/lgtning.spr");
	
	PRECACHE_MODEL("models/roach.mdl");
	PRECACHE_MODEL("models/scientist.mdl");
	PRECACHE_MODEL("models/barney.mdl");

	PRECACHE_MODEL("models/boid.mdl");
	PRECACHE_MODEL("models/osprey.mdl");
	PRECACHE_MODEL("models/HVR.mdl");

	PRECACHE_MODEL("sprites/rope.spr");
	PRECACHE_MODEL("sprites/fexplo.spr");
	PRECACHE_MODEL("models/osprey_tailgibs.mdl");
	PRECACHE_MODEL("models/osprey_bodygibs.mdl");
	PRECACHE_MODEL("models/osprey_enginegibs.mdl");
	PRECACHE_MODEL("models/apache.mdl");
	PRECACHE_MODEL("sprites/white.spr");
	PRECACHE_MODEL("models/metalplategibs_green.mdl");
	PRECACHE_MODEL("sprites/smoke.spr");
	PRECACHE_MODEL("models/barnacle.mdl");

	PRECACHE_MODEL("sprites/flare3.spr");
	PRECACHE_MODEL("models/turret.mdl");
	PRECACHE_MODEL("models/miniturret.mdl");
	PRECACHE_MODEL("models/zombie.mdl");
	PRECACHE_MODEL("models/sentry.mdl");
	PRECACHE_MODEL("models/controller.mdl");
	
	PRECACHE_MODEL("sprites/xspark4.spr");
	PRECACHE_MODEL("sprites/xspark1.spr");
	PRECACHE_MODEL("models/nihilanth.mdl");
	PRECACHE_MODEL("sprites/flare6.spr");
	PRECACHE_MODEL("sprites/nhth1.spr");
	PRECACHE_MODEL("sprites/exit1.spr");
	PRECACHE_MODEL("sprites/tele1.spr");
	PRECACHE_MODEL("sprites/animglow01.spr");
	PRECACHE_MODEL("sprites/muzzleflash3.spr");

	PRECACHE_MODEL("models/tentacle2.mdl");
	PRECACHE_MODEL("models/leech.mdl");

	PRECACHE_MODEL("models/bigrat.mdl");
	PRECACHE_MODEL("models/houndeye.mdl");

	PRECACHE_MODEL("sprites/shockwave.spr");
	PRECACHE_MODEL("models/hassassin.mdl");

	PRECACHE_MODEL("models/big_mom.mdl");
	PRECACHE_MODEL("sprites/mommaspit.spr");//spitprojectile.
	PRECACHE_MODEL("sprites/mommaspout.spr");//clientsidespittle.
	PRECACHE_MODEL("sprites/mommablob.spr");

	PRECACHE_MODEL("models/bullsquid.mdl");

	// Call this to set a static model reference (int, by load order?) correctly.
	CSquidSpit::precacheStatic();

	PRECACHE_MODEL("models/baby_headcrab.mdl");
	PRECACHE_MODEL("models/headcrab.mdl");
	
	PRECACHE_MODEL("models/agrunt.mdl");
	PRECACHE_MODEL("sprites/muz4.spr");
	PRECACHE_MODEL("models/hornet.mdl");
	PRECACHE_MODEL("sprites/muz1.spr");
	PRECACHE_MODEL("sprites/laserbeam.spr");
	PRECACHE_MODEL("models/hgrunt.mdl");
	
	PRECACHE_MODEL("sprites/poison.spr");

	PRECACHE_MODEL("models/shell.mdl");//brassshell
	PRECACHE_MODEL("models/shotgunshell.mdl");
	PRECACHE_MODEL("models/floater.mdl");
	PRECACHE_MODEL("models/w_medkit.mdl");
	PRECACHE_MODEL ("models/w_battery.mdl");

	PRECACHE_MODEL ("models/w_antidote.mdl");
	PRECACHE_MODEL ("models/w_suit.mdl");

	PRECACHE_MODEL ("models/w_adrenaline.mdl");
	PRECACHE_MODEL ("models/w_rad.mdl");

	PRECACHE_MODEL ("models/w_security.mdl");
	PRECACHE_MODEL ("models/w_longjump.mdl");
	
	PRECACHE_MODEL("models/w_oxygen.mdl");

	//Some MP5 stuff to precacahe, just to be safe.  Probably unnecessary besides the w_chainammo.mdl, though?
	PRECACHE_MODEL ("models/w_9mmARclip.mdl");
	PRECACHE_MODEL ("models/w_chainammo.mdl");
	PRECACHE_MODEL ("models/w_ARgrenade.mdl");
	PRECACHE_MODEL ("models/w_rpgammo.mdl");

	//unnecessary since each player weapon's precache script is called? Whatever.
	PRECACHE_MODEL("models/v_9mmAR.mdl");
	PRECACHE_MODEL("models/w_9mmAR.mdl");
	PRECACHE_MODEL("models/p_9mmAR.mdl");

	//MODDD - chumtoad!
	PRECACHE_MODEL("models/chumtoad.mdl");
	PRECACHE_MODEL("models/v_chub.mdl");

	PRECACHE_MODEL ("models/w_357ammobox.mdl");
	PRECACHE_MODEL ("models/w_9mmclip.mdl");
	PRECACHE_MODEL ("models/w_shotbox.mdl");
	PRECACHE_MODEL ("models/w_crossbow_clip.mdl");
	PRECACHE_MODEL ("models/w_gaussammo.mdl");

	PRECACHE_MODEL ("models/shell.mdl");// brass shellTE_MODEL

	PRECACHE_MODEL("models/grenade.mdl");	// grenade
    

	PRECACHE_EVENT( 1, "events/mp5.sc" );
	PRECACHE_EVENT( 1, "events/mp52.sc" );

	
	PRECACHE_MODEL("models/hassault.mdl");
	PRECACHE_MODEL ("models/shell.mdl");// brass shell

	
	//Odd that no places seem to precache this?  Does multiplayer even precache it?
	//Anyhow, this is necessary to "drop" in single-player (the new CVar, "canDropInSinglePlayer" can allow this)
	//UPDATE: search for the following line in weapons.cpp:
	/*
	UTIL_PrecacheOther( "weaponbox" );// container for dropped deathmatch weapons
	*/
	PRECACHE_MODEL("models/w_weaponbox.mdl");
	
	PRECACHE_MODEL("models/panthereye.mdl");


	//MODDD - keep up.
	// snapbug
	PRECACHE_MODEL("models/friendly.mdl");
	PRECACHE_MODEL("models/snapbug.mdl");
	PRECACHE_MODEL("models/archer.mdl");
	PRECACHE_MODEL("models/kingpin.mdl");
	PRECACHE_MODEL("models/floater.mdl");
	PRECACHE_MODEL("models/aflock.mdl");
	


	}//END OF if(precacheAllVar)
	else if(EASY_CVAR_GET_DEBUGONLY(canDropInSinglePlayer) == 1){
		// If the player can drop in SinglePlayer, precache this in anticipation of that.
		// If "precacheAll" was already run, it already covered this (hence this happening in "else")
		PRECACHE_MODEL("models/w_weaponbox.mdl");
	}

}//END OF method_precacheAll



//MODDD NOTE - The "arg_tolerance" of IsFacing and IsFacingAway show how much of a difference between where
//             the target entity (pevTest) is looking now, angle-wise, and the point he should be looking at for the check,
//             is allowed.
//             Tolerance can not be negative, as a tolerance of 0 is already impossibly strict (exactly 1.0 or -1.0 respectively
//             between IsFacing / IsFacingAway).
//             A tolerance of 1.0 supports looking 180 degrees around, or +- 90.
//             Use a tolerance of 2.0 to allow looking anywhere (360 degrees), +- 180.
//             IN SHORT, the range of tolerance is 0 to 2.

//DEFAULT DEGREES: about +/- 15?
BOOL UTIL_IsFacing( entvars_t *pevTest, const Vector &vecLookAtTest )
{
	return UTIL_IsFacing(pevTest, vecLookAtTest, 0.04);  // 1 - 0.04 = 0.96, +/- 15 degrees or so
}
//is "pevTest" looking at "vecLookAtTest"?
BOOL UTIL_IsFacing( entvars_t *pevTest, const Vector &vecLookAtTest, const float& arg_tolerance )
{
	if(pevTest == NULL){
		//doesn't exist. uh, not facing you?
		return FALSE;
	}

	Vector vecDir = (vecLookAtTest - pevTest->origin);
	vecDir.z = 0;
	vecDir = vecDir.Normalize();
	Vector forward, angle;
	//HM.  perhaps for non-player, v_angle should be  angles ?

	CBaseEntity* entTest = CBaseEntity::Instance(pevTest);
	if(entTest != NULL && entTest->IsPlayer() == TRUE){
		angle = pevTest->v_angle;
	}else{
		angle = pevTest->angles;
	}

	angle.x = 0;
	UTIL_MakeVectorsPrivate( angle, forward, NULL, NULL );

	//UTIL_drawLineFrame(pevTest->origin, pevTest->origin + (vecDir*66), 12, 255, 0, 0);
	//UTIL_drawLineFrame(pevTest->origin, pevTest->origin + (forward*66), 12, 0, 255, 0);
	//easyPrintLine("DOTTTT %.2f", DotProduct( forward, vecDir ));
	
	float theProd = DotProduct(forward, vecDir);
	// He's facing me, he meant it
	//0.707 is +- 45 degrees.
	if ( theProd >= (1 - arg_tolerance) )
	{
		return TRUE;
	}
	return FALSE;
}



BOOL UTIL_IsFacingAway( entvars_t *pevTest, const Vector &vecLookAtTest )
{
	return UTIL_IsFacingAway(pevTest, vecLookAtTest, 0.04);  // -1 + 0.04 = -0.96, +/- 15 degrees or so
}

//is "pevTest" looking away from "vecLookAtTest"?
BOOL UTIL_IsFacingAway( entvars_t *pevTest, const Vector &vecLookAtTest, const float& arg_tolerance )
{
	Vector vecDir = (vecLookAtTest - pevTest->origin);
	vecDir.z = 0;
	vecDir = vecDir.Normalize();
	Vector forward, angle;

	CBaseEntity* entTest = CBaseEntity::Instance(pevTest);
	if(entTest != NULL && entTest->IsPlayer() == TRUE){
		angle = pevTest->v_angle;
	}else{
		angle = pevTest->angles;
	}

	angle.x = 0;
	UTIL_MakeVectorsPrivate( angle, forward, NULL, NULL );
	
	easyPrintLine("HOW FAR FACIN AWAY %.2f TOL: %.2f", DotProduct( forward, vecDir ), (-1 + arg_tolerance) );

	if ( DotProduct( forward, vecDir ) <= (-1 + arg_tolerance) )
	{
		return TRUE;
	}
	return FALSE;
}


//MODDD - utility random invert.  Good tool for a random that is so far from 0, but can be positive or negative from it.
//Such as, say I want a random number in range  |30, 80|  or  ( [-80, -30] or [30, 80] ).  This makes sure the absolute value isn't under 30 or over 80.
//Below even does the absolute range all-in-one (randomAbsoluteValue)

float randomInvert(const float& arg_flt){
	if(g_engfuncs.pfnRandomLong(0, 1) == 0){
		return arg_flt;
	}else{
		return -arg_flt;
	}
}

float randomAbsoluteValue(const float& arg_fltMin, const float& arg_fltMax){

	float fltRange = arg_fltMax - arg_fltMin;

	//just get from 0 to "range", both ways.
	//that is, -range to +range  (or, -range to 0 and 0 to +range)

	float fltReturn = g_engfuncs.pfnRandomFloat(-fltRange, fltRange);

	//add back the minimum (or, subtract?)

	if(fltReturn < 0){
		fltReturn -= arg_fltMin;
	}else if(fltReturn > 0){
		fltReturn += arg_fltMin;
	}else{
		//exactly 0?  strange, but not impossible (I think)
		fltReturn += randomInvert(arg_fltMin);
	}
	return fltReturn;
}

int randomValueInt(const int& arg_min, const int& arg_max){
	//nothing too special.
	return g_engfuncs.pfnRandomLong(arg_min, arg_max);
}

float randomValue(const float& arg_fltMin, const float& arg_fltMax){
	//nothing too special.
	return g_engfuncs.pfnRandomFloat(arg_fltMin, arg_fltMax);
}


void UTIL_deriveColorFromMonsterHealth(const float& curHealth, const float& maxHealth, int& r, int& g, int& b){
	//nothing too special.
	
	const int brightness = 250;

	//At what point is the GUI going from green to yellow? Measure of health (0 - 100)
	const float yellowFract = 0.5;

	const float max = maxHealth;
	const float cur = curHealth;
	const float fract = cur / max;
	const float yellowMark = yellowFract * max;

	//easyPrintLine("PPOPY: %.2f %.2f %.2f %.2f", curHealth, maxHealth, fract, yellowMark);	
	
	if(fract >= yellowFract){
		//r = (int) (( ( -m_iHealth +yellowMark+100  ) /((float)yellowMark)) *175 );
		r = (int) (( ( -cur + max ) /((float) (max-yellowMark) )) *brightness );
		g = brightness;
		b = 5;
	}else{
		r = brightness;
		g = (int) (( ( cur  ) /((float)yellowMark)) *brightness );
		b = 5;
	}

}


void attemptSendBulletSound(const Vector& bulletHitLoc, entvars_t* pevShooter){

	if(EASY_CVAR_GET_DEBUGONLY(bulletholeAlertRange) > 0 ){
		CBaseEntity* pEntityScan = NULL;
		//easyPrintLine("HOOOOO %s %s %d", STRING(pev->classname), STRING(pevAttacker->classname), (this->pev == pevAttacker));
				
		//why above the range?  In case something has good enough hearing (above 1) to pick this sound up anyways.  Up to a multiple of 1.5 supported.
		float extraCheck = 1.5;

		while ( (pEntityScan = UTIL_FindEntityInSphere( pEntityScan, bulletHitLoc, EASY_CVAR_GET_DEBUGONLY(bulletholeAlertRange)*extraCheck ) ) != NULL)
		{

			//MySquadMonsterPointer ????
			CBaseMonster* testMon = pEntityScan->MyMonsterPointer();
			//CSquadMonster* testMon2 = pEntityScan->MySquadMonsterPointer();
					
			//"testMon->pev->renderfx & ISNPC" is a cheap way of making sure this is an NPC.  Anything that beings with "monsterInit" will have this render flag.
			//Also, "m_MonsterState == MONSTERSTATE_SCRIPT" would mean this is a cinematic monster.  Ignore.
			if(testMon != NULL && testMon->pev != pevShooter && (testMon->pev->renderfx & ISNPC) && (testMon->m_MonsterState != MONSTERSTATE_SCRIPT && testMon->m_pCine == NULL)  && (EASY_CVAR_GET_DEBUGONLY(bulletholeAlertStukaOnly) != 1 || FClassnameIs(testMon->pev, "monster_stukabat") )  ){

				float hearing = testMon->HearingSensitivity();
				float trueDist = (testMon->pev->origin - bulletHitLoc).Length();
				float adjustedDist = 0;
				if(hearing != 0){
					adjustedDist = trueDist / hearing;
				}

				
				if(adjustedDist <= EASY_CVAR_GET_DEBUGONLY(bulletholeAlertRange)){
					if(EASY_CVAR_GET_DEBUGONLY(bulletHoleAlertPrintout) == 1){
						easyPrintLine("I HEARD BULLETHIT!: %s truedist: %.2f adjusteddist: %.2f HS: %.2f", STRING(testMon->pev->classname), trueDist, adjustedDist, hearing);
					}
					testMon->heardBulletHit(pevShooter);
							
				}else{
					if(EASY_CVAR_GET_DEBUGONLY(bulletHoleAlertPrintout) == 1){
						easyPrintLine("Hearing sensitivity not good enough!: %s truedist: %.2f adjusteddist: %.2f HS: %.2f", STRING(testMon->pev->classname), trueDist, adjustedDist, hearing);
					}
				}

			}//END OF if(testMon != NULL)

		}//END OF while(another entity so far from the bullet hole to be triggered)
	}//END OF EASY_CVAR_GET_DEBUGONLY(bulletholeAlertRange) above 0 check
}



//...better fitted to a "player".  Was good for the most part before, but not perfect.
CBaseEntity *FindEntityForward( CBasePlayer* pMe )
{
	TraceResult tr;
	//UTIL_MakeVectors(pev->v_angle + pev->punchangle);
	//UTIL_MakeVectors( pev->v_angle + pev->punchangle );
	//UTIL_MakeVectors(pMe->pev->v_angle + pMe->pev->punchangle);


	const float SEARCH_DIST = 2048; //2048;

	UTIL_MakeVectors(pMe->pev->v_angle + pMe->pev->punchangle);

	Vector vecStart = pMe->pev->origin + pMe->pev->view_ofs + gpGlobals->v_forward * 5;
	Vector vecEnd = vecStart + gpGlobals->v_forward * SEARCH_DIST;
	UTIL_TraceLine(vecStart, vecEnd, dont_ignore_monsters, pMe->edict(), &tr );
	

	/*
	pMe->debugDrawVect2 = pMe->pev->origin + pMe->pev->view_ofs + gpGlobals->v_forward * 5;
	pMe->debugDrawVect3 = pMe->pev->origin + pMe->pev->view_ofs + gpGlobals->v_forward * 360;

	
	Vector angleToUse;
	
	angleToUse.x = fmod(pMe->pev->v_angle.x, 360.0f);
	angleToUse.y = fmod(pMe->pev->v_angle.y, 360.0f);
	angleToUse.z = fmod(pMe->pev->v_angle.z, 360.0f);
	
	
	UTIL_printLineVector("derp", pMe->pev->v_angle);
	UTIL_printLineVector("hmmm", pMe->pev->v_angle - pMe->pev->punchangle);
	UTIL_printLineVector("OWWW", pMe->pev->punchangle);
	UTIL_printLineVector("gah", angleToUse);
	UTIL_printLineVector("ANGLE", pMe->pev->angles);
	
	UTIL_MakeVectors(angleToUse);
	
	pMe->debugDrawVectRecentGive1 = pMe->pev->origin + pMe->pev->view_ofs + gpGlobals->v_forward * 5;
	pMe->debugDrawVectRecentGive2 = pMe->pev->origin + pMe->pev->view_ofs + gpGlobals->v_forward * 360;
	*/

	
	if(EASY_CVAR_GET_DEBUGONLY(drawDebugPathfinding2) == 1){::DebugLine_ClearAll();}
	
	//float fullLength = 5 + 2048;
	BOOL success = FALSE;
	float fracto = 1.0;


	if ( tr.flFraction != 1.0 && !FNullEnt( tr.pHit) )
	{
		CBaseEntity *pHit = CBaseEntity::Instance( tr.pHit );

		fracto = tr.flFraction;
		if(EASY_CVAR_GET_DEBUGONLY(drawDebugPathfinding2) == 1){::DebugLine_Setup(0, vecStart, vecEnd, fracto);}

		return pHit;
	}else if(tr.flFraction != 1.0){
		//just a check:  we did hit something tangible at least, right?  Even the map?

		fracto = tr.flFraction;

		//Try in a small sphere (area nearby)?
		CBaseEntity* pEntityTemp = NULL;
		CBaseEntity* bestYet = NULL;
		float bestDistYet = 99999999;
		while ((pEntityTemp = UTIL_FindEntityInSphere( pEntityTemp, tr.vecEndPos, 20 )) != NULL)
		{
			CBaseMonster* monsterTest = NULL;
			if( (monsterTest = pEntityTemp->GetMonsterPointer()) != NULL){
				if(monsterTest->pev != pMe->pev){
					float tempDist = 0;
					if( (tempDist = (monsterTest->pev->origin - tr.vecEndPos).Length()) < bestDistYet ){
						bestYet = monsterTest;
						bestDistYet = tempDist;
					}
				}
			}
		}//END OF while(...)
		//Find something?
		if(bestYet != NULL){
			if(EASY_CVAR_GET_DEBUGONLY(drawDebugPathfinding2) == 1){::DebugLine_Setup(0, vecStart, vecEnd, fracto);}
			return bestYet;
		}
	}else{
		//HEY. Super fail.
		int x = 5423;
	}

	/*
	if(success){
		::DebugLine_Setup(0, vecStart, vecEnd, (1.0f));
	}else{
		::DebugLine_Setup(0, vecStart, vecEnd, fracto);
	}
	*/

	if(EASY_CVAR_GET_DEBUGONLY(drawDebugPathfinding2) == 1){::DebugLine_Setup(0, vecStart, vecEnd, fracto);}
	return NULL;
}


//if -1 or 0, return as-is.  If anything else, take as "arg_delay - gpGlobals->time", looking like a count-down.
//~-1 or 0 means already happened or not in use.  These are key values.  I stick to -1.
float timeDelayFilter(float arg_delay){
	if(arg_delay == 0 || arg_delay == -1){
		//as-is.
		return arg_delay;
	}else{
		return arg_delay - gpGlobals->time;
	}
}

Vector getRotatedVectorAboutZAxis(const Vector& arg_vec, const float& arg_deg){
	
	Vector vecReturn;

	float arc = (M_PI * arg_deg / 180.0f);

	Vector rotMatrix[3];
	rotMatrix[0].x = cos(arc);
	rotMatrix[0].y = -sin(arc);
	rotMatrix[0].z = 0;
	rotMatrix[1].x = sin(arc);
	rotMatrix[1].y = cos(arc);
	rotMatrix[1].z = 0;
	rotMatrix[2].x = 0;
	rotMatrix[2].y = 0;
	rotMatrix[2].z = 1;

	vecReturn.x = DotProduct(arg_vec, rotMatrix[0]);
	vecReturn.y = DotProduct(arg_vec, rotMatrix[1]);
	vecReturn.z = DotProduct(arg_vec, rotMatrix[2]);

	return vecReturn;
}


void UTIL_ServerMassCVarReset(entvars_t* pev){

	edict_t* pEntity = ENT(pev);

	EASY_CVAR_RESET_MASS
	EASY_CVAR_RESET_MASS_CLIENT_SIGNAL
}


void PRINTQUEUE_STUKA_SEND(PrintQueue& toPrint, const char* src, ...){
	if(toPrint.latestPlace < 5){
		va_list argptr;
		va_start(argptr, src);
		const char* tempResult = UTIL_VarArgsVA(src, argptr );
		//strncpyTerminate(&contents[latestPlace][0], &UTIL_VarArgsVA(src, argptr )[0], lengthOfString(src, PRINTQUEUE_STRINGSIZE) );
		strncpyTerminate(&toPrint.contents[toPrint.latestPlace][0], tempResult, lengthOfString(tempResult, PRINTQUEUE_STRINGSIZE) );
		va_end(argptr);
		toPrint.latestPlace++;
	}else{
		//PROBLEM! TOO MANY IN QUEUE!
	}
}

BOOL getGermanModelsAllowed(void){
	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(sv_germancensorship) == 1 && EASY_CVAR_GET_DEBUGONLY(allowGermanModels) == 1 && globalPSEUDO_canApplyGermanCensorship == 1){
		return TRUE;
	}else{
		return FALSE;
	}
}


///////////////////////////////////////////////////////////////////////////
//MODDD - Spirit of Half-Life 1.9
///////////////////////////////////////////////////////////////////////////

char* GetStringForState( STATE state )
{
	switch(state)
	{
	case STATE_ON: return "ON";
	case STATE_OFF: return "OFF";
	case STATE_TURN_ON: return "TURN ON";
	case STATE_TURN_OFF: return "TURN OFF";
	case STATE_IN_USE: return "IN USE";
	default:
		return "STATE_UNKNOWN!?";
	}
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////


Vector projectionOntoPlane(Vector arg_vectOnto, Vector arg_planeNormal){
	
	return arg_vectOnto - ( ( ::DotProduct(arg_vectOnto, arg_planeNormal) / ::pow(arg_planeNormal.Length(), 2) ) * arg_planeNormal );
}//END OF projectionOntoPlane


	
CBaseEntity* UTIL_CreateNamedEntity(const char* arg_entityName){
	edict_t* pent = CREATE_NAMED_ENTITY( MAKE_STRING( arg_entityName) );
	return CBaseEntity::Instance( pent );
}

BOOL entityHidden(CBaseEntity* test){
	if(test==NULL)return FALSE;
	return (test->pev->flags & FL_NOTARGET);
}
BOOL entityHidden(edict_t* test){
	if(test==NULL)return FALSE;
	return (test->v.flags & FL_NOTARGET);
}


// MODD - trying another instead of CHAN_WEAPON, this should be harder to interrupt.
// No idea if other channels (CHAN_STATIC) just support more than 1 sound fine though.
// If that has some issue CHAN_BODY may be ok too.
void UTIL_playOrganicGibSound(entvars_t* pevSoundSource){
	// CHAN_BODY ???
	switch(RANDOM_LONG(0, 2)){
	case 0:
		UTIL_PlaySound(ENT(pevSoundSource), CHAN_STATIC, "common/bodysplat.wav", 1, ATTN_NORM - 0.12, FALSE);
	break;
	case 1:
		UTIL_PlaySound(ENT(pevSoundSource), CHAN_STATIC, "debris/bustflesh1.wav", 1, ATTN_NORM - 0.35, FALSE);
	break;
	case 2:
		UTIL_PlaySound(ENT(pevSoundSource), CHAN_STATIC, "debris/bustflesh2.wav", 1, ATTN_NORM - 0.35, FALSE);
	break;
	}//END OF switch
}//END OF Util_playOrganicGibsound


void UTIL_playMetalGibSound(entvars_t* pevSoundSource){
	switch(RANDOM_LONG(0, 4)){
	case 0:
		UTIL_PlaySound(ENT(pevSoundSource), CHAN_STATIC, "debris/bustmetal1.wav", 1, ATTN_NORM - 0.17, FALSE);
	break;
	case 1:
		UTIL_PlaySound(ENT(pevSoundSource), CHAN_STATIC, "debris/bustmetal2.wav", 1, ATTN_NORM - 0.17, FALSE);
	break;
	case 2:
		UTIL_PlaySound(ENT(pevSoundSource), CHAN_STATIC, "debris/metal2.wav", 1, ATTN_NORM - 0.17, FALSE);
	break;
	case 3:
		UTIL_PlaySound(ENT(pevSoundSource), CHAN_STATIC, "debris/metal4.wav", 1, ATTN_NORM - 0.17, FALSE);
	break;
	case 4:
		UTIL_PlaySound(ENT(pevSoundSource), CHAN_STATIC, "debris/metal5.wav", 1, ATTN_NORM - 0.17, FALSE);
	break;
	}//switch
}//UTIL_playMetalGibSound


// To be exchanged for zombie hit-sounds whenever something metallic is hit instead.
void UTIL_playMeleeMetalHitSound(entvars_t* pevSoundSource){
	
	//UTIL_PlaySound( ENT(pev), CHAN_WEAPON, pAttackHitSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackHitSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );

	const int randomSound = RANDOM_LONG(0, 5);
	const float metalVol = 0.57;
	const float metalAttn = ATTN_NORM - 0.02;
	switch (randomSound) {
	case 0:UTIL_PlaySound(ENT(pevSoundSource), CHAN_WEAPON, "debris/metal1.wav", metalVol - 0.15, metalAttn, 0, 92 + RANDOM_LONG(0, 5), FALSE); break;
	case 1:UTIL_PlaySound(ENT(pevSoundSource), CHAN_WEAPON, "debris/metal2.wav", metalVol, metalAttn, 0, 94 + RANDOM_LONG(0, 5), FALSE); break;
	case 2:UTIL_PlaySound(ENT(pevSoundSource), CHAN_WEAPON, "debris/metal3.wav", metalVol, metalAttn, 0, 94 + RANDOM_LONG(0, 5), FALSE); break;
	case 3:UTIL_PlaySound(ENT(pevSoundSource), CHAN_WEAPON, "debris/metal4.wav", metalVol - 0.1, metalAttn, 0, 90 + RANDOM_LONG(0, 5), FALSE); break;
	case 4:UTIL_PlaySound(ENT(pevSoundSource), CHAN_WEAPON, "debris/metal5.wav", metalVol - 0.08, metalAttn, 0, 92 + RANDOM_LONG(0, 5), FALSE); break;
	case 5:UTIL_PlaySound(ENT(pevSoundSource), CHAN_WEAPON, "debris/metal6.wav", metalVol, metalAttn, 0, 94 + RANDOM_LONG(0, 5), FALSE); break;
	}//switch

}//UTIL_playMeleeMetalHitSound


// Plays sounds ambiend like the rest of texturehit (sound.cpp).
// BEWARE !  Keep in synch with the cl_dlls/ev_hldm.cpp equivalent for clientside in its texturehit method
void UTIL_playMetalTextureHitSound(entvars_t* pevSoundSource, const Vector& vecOrigin){

	const int randomSound = RANDOM_LONG(0, 5);
	const float metalVol = 0.57;
	const float metalAttn = ATTN_NORM - 0.02;
	switch (randomSound) {
	case 0:EMIT_AMBIENT_SOUND(ENT(pevSoundSource), vecOrigin, "debris/metal1.wav", metalVol - 0.15, metalAttn, 0, 92 + RANDOM_LONG(0, 5)); break;
	case 1:EMIT_AMBIENT_SOUND(ENT(pevSoundSource), vecOrigin, "debris/metal2.wav", metalVol, metalAttn, 0, 94 + RANDOM_LONG(0, 5)); break;
	case 2:EMIT_AMBIENT_SOUND(ENT(pevSoundSource), vecOrigin, "debris/metal3.wav", metalVol, metalAttn, 0, 94 + RANDOM_LONG(0, 5)); break;
	case 3:EMIT_AMBIENT_SOUND(ENT(pevSoundSource), vecOrigin, "debris/metal4.wav", metalVol - 0.1, metalAttn, 0, 90 + RANDOM_LONG(0, 5)); break;
	case 4:EMIT_AMBIENT_SOUND(ENT(pevSoundSource), vecOrigin, "debris/metal5.wav", metalVol - 0.08, metalAttn, 0, 92 + RANDOM_LONG(0, 5)); break;
	case 5:EMIT_AMBIENT_SOUND(ENT(pevSoundSource), vecOrigin, "debris/metal6.wav", metalVol, metalAttn, 0, 94 + RANDOM_LONG(0, 5)); break;
	}//switch

}//UTIL_playMetalTextureHitSound




//Right before a CHANGE_LEVEL call to move to another map from the current one. Not loading a new map on the player's demand.
void OnBeforeChangeLevelTransition(){
	// For all intents and purposes, this is like loading a game. Don't reset the counters.
	g_mapLoaded = TRUE;

	
	//just to be safe.
	DebugLine_ClearAll();
}


//MODDD - another new event, even earlier than OnMapLoadStart.  Called by Engine-called method "ResetGlobalState".
// Possibly as early as it gets, even earlier than a Map instance being made. The Map (CWorld constructor being called) happens after this.
// More importantly, not only is this called before ANY spawn/precache methods, but also before even any KeyValue reads, otherwise thought
// to be the earliest possible until this was found.
// BUT BEWARE - this does not get called on going between transitions!
void OnMapLoadPreStart(){
	

}


//MODDD - new event, called by CWorld's precache method (first thing precached since starting a map or calling changelevel, transition or not).
// !!! IMOPORTANT !!!    For the clientside equivalent, see cl_dlls/cdll_int.cpp:  HUD_VidInit.
// That's the source of a lot of precache calls throughout HUD classes that lets them work (not only once at game bootup).
void OnMapLoadStart(){
	easyForcePrintLine("!!!!!! OnMapLoadStart !!!");

	resetGlobalVars();

	// Reset this in case the game ended between calls, the player calling precache or actually spawning (new game) will
	// soon cause this to be TRUE again.
	g_firstPlayerEntered = FALSE;

	DebugLine_ClearAll();

	if(!g_mapLoaded){
		// If we loaded a game or came from a transition, these values have already been loaded from save data or are cumulative with
		// soon-to-be new entities for this map, possibly. Don't overwrite them with 0's.
		ResetDynamicStaticIDs();
	}

	// Next time, will force these off in case there isn't a loaded game.
	// NOTICE!  Don't rely on g_mapLoaded in any other gamelogic. It is only for detecting whether
	// this map was going between a transition or not very early on, it seems.
	// This variable was made a long while ago.
	g_mapLoaded = FALSE;
	
	// This var gets toggled the first time any map is loaded though.  Some things need to know this.
	g_mapLoadedEver = TRUE;
	
}//END OF OnMapLoadStart



void OnMapLoadEnd(){
	//Going around changing the map? Need this to reset.
	DebugLine_drawTime = -1;
	queueYMG_stopSend = TRUE;
	
	
	CWorld* theWorld = getWorld();
	if(theWorld != NULL){
		//In case any node ent's spawned with an AIR type, let the worldspawn entity know.
		theWorld->getCustomMapSettingsFromGlobal();
	}
	
}//END OF OnMapLoadEnd


const char* TOGGLE_STATE_STR[] = {"TS_AT_TOP", "TS_AT_BOTTOM", "TS_GOING_UP", "TS_GOING_DOWN"  };

const char* TOGGLE_STATE_STR_Safe(int argIndex){
	if(argIndex >= 0 && argIndex <= 3){
		return TOGGLE_STATE_STR[argIndex];
	}else{
		return "BAD_VALUE";
	}
}



// Only the physical IDs themselves need to be reset in the case of starting a new map as opposed to loading one from a save file.
void ResetDynamicStaticIDs(){
	CBaseMonster::monsterIDLatest = 0;
	CFuncTrackChange::FuncTrackChangeIDLatest = 0;
	CPathTrack::PathTrackIDLatest = 0;
	CCineMonster::scriptedIDLatest = 0;
}

void SaveDynamicIDs(CGlobalState* argGS){
	argGS->m_i_monsterIDLatest = CBaseMonster::monsterIDLatest;
	argGS->m_i_FuncTrackChangeIDLatest = CFuncTrackChange::FuncTrackChangeIDLatest;
	argGS->m_i_PathTrackIDLatest = CPathTrack::PathTrackIDLatest;
	argGS->m_i_scriptedIDLatest = CCineMonster::scriptedIDLatest;

}

void RestoreDynamicIDs(CGlobalState* argGS){

	//MODDD - is that okay?
	CBaseMonster::monsterIDLatest = argGS->m_i_monsterIDLatest;
	CFuncTrackChange::FuncTrackChangeIDLatest = argGS->m_i_FuncTrackChangeIDLatest;
	CPathTrack::PathTrackIDLatest = argGS->m_i_PathTrackIDLatest;
	CCineMonster::scriptedIDLatest = argGS->m_i_scriptedIDLatest;
	
	// Make sure these are not overwritten by World's precache calling OnMapLoadStart.
	g_mapLoaded = TRUE;

}//END OF RestoreGlobalState


BOOL GermanModelOrganicLogic(){
	// Do german models still have the behavior of their organic originals, like being attractive to scavenger enemies
	// when dead?
	// Can make this a CVar later.
	return TRUE;
}


CWorld* getWorld(void){
	// The world should be entity #0.  Standard.
	// VARS(ENT(0))
	// VARS(INDEXENT(0))
	// CBaseEntity::Instance( INDEXENT(0) )
	CBaseEntity* worldEntTest = CBaseEntity::Instance( ENT(0) );
	if(worldEntTest != NULL && FClassnameIs(worldEntTest->pev, "worldspawn")){
		return static_cast<CWorld*>(worldEntTest);
	}else{
		return NULL;     //whut
	}
}





//=========================================================
// FBoxVisible - a more accurate ( and slower ) version
// of FVisible. 
//
// !!!UNDONE - make this CBaseMonster?
//=========================================================
BOOL FBoxVisible ( entvars_t *pevLooker, entvars_t *pevTarget, Vector &vecTargetOrigin, float flSize )
{
	// don't look through water
	if (
		(pevLooker->waterlevel != 3 && pevTarget->waterlevel == 3)
		|| (pevLooker->waterlevel == 3 && pevTarget->waterlevel == 0)
	) {
		return FALSE;
	}

	TraceResult tr;
	Vector	vecLookerOrigin = pevLooker->origin + pevLooker->view_ofs;//look through the monster's 'eyes'
	for (int i = 0; i < 5; i++)
	{
		Vector vecTarget = pevTarget->origin;
		vecTarget.x += RANDOM_FLOAT( pevTarget->mins.x + flSize, pevTarget->maxs.x - flSize);
		vecTarget.y += RANDOM_FLOAT( pevTarget->mins.y + flSize, pevTarget->maxs.y - flSize);
		vecTarget.z += RANDOM_FLOAT( pevTarget->mins.z + flSize, pevTarget->maxs.z - flSize);

		UTIL_TraceLine(vecLookerOrigin, vecTarget, ignore_monsters, ignore_glass, ENT(pevLooker)/*pentIgnore*/, &tr);
		
		if (tr.flFraction == 1.0)
		{
			vecTargetOrigin = vecTarget;
			return TRUE;// line of sight is valid.
		}
	}
	return FALSE;// Line of sight is not established
}



//MODDD - clone that doesn't take vecTargetOrigin.
BOOL FBoxVisible(entvars_t* pevLooker, entvars_t* pevTarget, float flSize)
{
	// don't look through water
	if (
		(pevLooker->waterlevel != 3 && pevTarget->waterlevel == 3)
		|| (pevLooker->waterlevel == 3 && pevTarget->waterlevel == 0)
		) {
		return FALSE;
	}

	TraceResult tr;
	Vector	vecLookerOrigin = pevLooker->origin + pevLooker->view_ofs;//look through the monster's 'eyes'
	for (int i = 0; i < 5; i++)
	{
		Vector vecTarget = pevTarget->origin;
		vecTarget.x += RANDOM_FLOAT(pevTarget->mins.x + flSize, pevTarget->maxs.x - flSize);
		vecTarget.y += RANDOM_FLOAT(pevTarget->mins.y + flSize, pevTarget->maxs.y - flSize);
		vecTarget.z += RANDOM_FLOAT(pevTarget->mins.z + flSize, pevTarget->maxs.z - flSize);

		UTIL_TraceLine(vecLookerOrigin, vecTarget, ignore_monsters, ignore_glass, ENT(pevLooker)/*pentIgnore*/, &tr);

		if (tr.flFraction == 1.0)
		{
			return TRUE;// line of sight is valid.
		}
	}
	return FALSE;// Line of sight is not established
}










//
// VecCheckToss - returns the velocity at which an object should be lobbed from vecspot1 to land near vecspot2.
// returns g_vecZero if toss is not feasible.
// 
Vector VecCheckToss ( entvars_t *pev, const Vector &vecSpot1, Vector vecSpot2, float flGravityAdj )
{
	TraceResult tr;
	Vector		vecMidPoint;// halfway point between Spot1 and Spot2
	Vector		vecApex;// highest point 
	Vector		vecScale;
	Vector		vecGrenadeVel;
	Vector		vecTemp;
	float		flGravity = g_psv_gravity->value * flGravityAdj;

	if (vecSpot2.z - vecSpot1.z > 500)
	{
		// to high, fail
		return g_vecZero;
	}

	UTIL_MakeVectors (pev->angles);

	// toss a little bit to the left or right, not right down on the enemy's bean (head). 
	vecSpot2 = vecSpot2 + gpGlobals->v_right * ( RANDOM_FLOAT(-8,8) + RANDOM_FLOAT(-16,16) );
	vecSpot2 = vecSpot2 + gpGlobals->v_forward * ( RANDOM_FLOAT(-8,8) + RANDOM_FLOAT(-16,16) );
	
	// calculate the midpoint and apex of the 'triangle'
	// UNDONE: normalize any Z position differences between spot1 and spot2 so that triangle is always RIGHT

	// How much time does it take to get there?

	// get a rough idea of how high it can be thrown
	vecMidPoint = vecSpot1 + (vecSpot2 - vecSpot1) * 0.5;
	UTIL_TraceLine(vecMidPoint, vecMidPoint + Vector(0,0,500), ignore_monsters, ENT(pev), &tr);
	vecMidPoint = tr.vecEndPos;
	// (subtract 15 so the grenade doesn't hit the ceiling)
	vecMidPoint.z -= 15;

	if (vecMidPoint.z < vecSpot1.z || vecMidPoint.z < vecSpot2.z)
	{
		// to not enough space, fail
		return g_vecZero;
	}

	// How high should the grenade travel to reach the apex
	float distance1 = (vecMidPoint.z - vecSpot1.z);
	float distance2 = (vecMidPoint.z - vecSpot2.z);

	// How long will it take for the grenade to travel this distance
	float time1 = sqrt( distance1 / (0.5 * flGravity) );
	float time2 = sqrt( distance2 / (0.5 * flGravity) );

	if (time1 < 0.1)
	{
		// too close
		return g_vecZero;
	}

	// how hard to throw sideways to get there in time.
	vecGrenadeVel = (vecSpot2 - vecSpot1) / (time1 + time2);
	// how hard upwards to reach the apex at the right time.
	vecGrenadeVel.z = flGravity * time1;

	// find the apex
	vecApex  = vecSpot1 + vecGrenadeVel * time1;
	vecApex.z = vecMidPoint.z;

	UTIL_TraceLine(vecSpot1, vecApex, dont_ignore_monsters, ENT(pev), &tr);
	if (tr.flFraction != 1.0)
	{
		// fail!
		return g_vecZero;
	}

	// UNDONE: either ignore monsters or change it to not care if we hit our enemy
	UTIL_TraceLine(vecSpot2, vecApex, ignore_monsters, ENT(pev), &tr); 
	if (tr.flFraction != 1.0)
	{
		// fail!
		return g_vecZero;
	}
	
	return vecGrenadeVel;
}


//
// VecCheckThrow - returns the velocity vector at which an object should be thrown from vecspot1 to hit vecspot2.
// returns g_vecZero if throw is not feasible.
// 
Vector VecCheckThrow ( entvars_t *pev, const Vector &vecSpot1, Vector vecSpot2, float flSpeed, float flGravityAdj )
{
	float flGravity = g_psv_gravity->value * flGravityAdj;

	Vector vecGrenadeVel = (vecSpot2 - vecSpot1);

	// throw at a constant time
	float time = vecGrenadeVel.Length( ) / flSpeed;
	vecGrenadeVel = vecGrenadeVel * (1.0 / time);

	// adjust upward toss to compensate for gravity loss
	vecGrenadeVel.z += flGravity * time * 0.5;

	Vector vecApex = vecSpot1 + (vecSpot2 - vecSpot1) * 0.5;
	vecApex.z += 0.5 * flGravity * (time * 0.5) * (time * 0.5);
	
	TraceResult tr;
	UTIL_TraceLine(vecSpot1, vecApex, dont_ignore_monsters, ENT(pev), &tr);
	if (tr.flFraction != 1.0)
	{
		// fail!
		return g_vecZero;
	}

	UTIL_TraceLine(vecSpot2, vecApex, ignore_monsters, ENT(pev), &tr);
	if (tr.flFraction != 1.0)
	{
		// fail!
		return g_vecZero;
	}

	return vecGrenadeVel;
}



// For a given entity, print at least its name.  If a monster, give its MonsterID.  If it has a netname, give that too.
// And this is NOT a 'printline' call, the caller can print something before and after this as needed
// (most likely a newline character).
// Caller accepted, but unused, sicne the server printout is used instead (to avoid buffer overflows).

//TODO - support printing out info about the linked m_pcine entity, if applicable and requested?


void printBasicEntityInfo(CBaseEntity* entRef) {
	printBasicEntityInfo(NULL, entRef);
}
void printBasicEntityInfo(edict_t* theCaller, CBaseEntity* entRef) {

	if (entRef == NULL) {
		// ?????  No more information to give
		easyForcePrint("NULLENT");
		return;
	}

	CBaseMonster* monsterTest = entRef->GetMonsterPointer();
	if (monsterTest != NULL) {
		easyForcePrint("%s:%d", entRef->getClassname(), monsterTest->monsterID);
	}
	else {
		easyForcePrint("%s", entRef->getClassname());
	}

	// FStringNull ?
	if (entRef->pev->netname != NULL) {
		const char* stuff = STRING(entRef->pev->netname);
		if (stuff != NULL && stuff[0] != '\0') {
			//ok, print it too.
			easyForcePrint(" netname:%s", stuff);
		}
	}
	if (entRef->pev->target != NULL) {
		const char* stuff = STRING(entRef->pev->target);
		if (stuff != NULL && stuff[0] != '\0') {
			//ok, print it too.
			easyForcePrint(" target:%s", stuff);
		}
	}
	if (entRef->pev->targetname != NULL) {
		const char* stuff = STRING(entRef->pev->targetname);
		if (stuff != NULL && stuff[0] != '\0') {
			//ok, print it too.
			easyForcePrint(" targetname:%s", stuff);
		}
	}
	if (entRef->pev->globalname != NULL) {
		const char* stuff = STRING(entRef->pev->globalname);
		if (stuff != NULL && stuff[0] != '\0') {
			//ok, print it too.
			easyForcePrint(" globalname:%s", stuff);
		}
	}

}//printBasicEntityInfo



// or from the server.
void printBasicTraceInfo(const TraceResult& tr) {
	easyForcePrint("fract:%.2f startSol:%d allSol:%d inOpen:%d inWat:%d ", tr.flFraction, tr.fStartSolid, tr.fAllSolid, tr.fInOpen, tr.fInWater);
	if (tr.pHit != NULL) {
		easyForcePrint("Hit:%s hitgr:%d", STRING(tr.pHit->v.classname), tr.iHitgroup);
	}
	else {
		easyForcePrint("pHit:none");
	}
	//if(tr.fAllSolid){ tr.flFraction = 0; easyForcePrintLineClient(pEntity, "SOLID"); }

}//printBasicTraceInfo

// This does print out to the client though, the debug methods that use this don't spam this.
void printBasicTraceInfo(edict_t* theCaller, const TraceResult& tr) {
	easyForcePrintClient(theCaller, "fract:%.2f startSol:%d allSol:%d inOpen:%d inWat:%d ", tr.flFraction, tr.fStartSolid, tr.fAllSolid, tr.fInOpen, tr.fInWater);
	if (tr.pHit != NULL) {
		easyForcePrintClient(theCaller, "Hit:%s hitgr:%d", STRING(tr.pHit->v.classname), tr.iHitgroup);
	}
	else {
		easyForcePrintClient(theCaller, "pHit:none");
	}
	//if(tr.fAllSolid){ tr.flFraction = 0; easyForcePrintLineClient(pEntity, "SOLID"); }

}//printBasicTraceInfo







//***MOVED FROM A FEW OTHER PLACES***
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/*
================
SpawnBlood
================
*/
// Used to call only BloodStream (retail blood effect).
// Now looks at sv_bloodparticlemode to pick from BloodStream or BloodDrips.
void UTIL_SpawnBlood(const Vector& vecSpot, int bloodColor, int amount) {
	UTIL_SpawnBlood(vecSpot, UTIL_RandomBloodVector(), bloodColor, amount);
}
void UTIL_SpawnBlood(const Vector& vecSpot, const Vector& bloodDir, int bloodColor, int amount)
{
	if (!UTIL_ShouldShowBlood(bloodColor)) {
		return;
	}
	if (EASY_CVAR_GET(sv_bloodparticlemode) == 1 || EASY_CVAR_GET(sv_bloodparticlemode) == 2) {
		UTIL_BloodStream(vecSpot, bloodDir, bloodColor, amount);
	}
	if (EASY_CVAR_GET(sv_bloodparticlemode) == 0 || EASY_CVAR_GET(sv_bloodparticlemode) == 2) {
		// RETAIL CALL.
		// Is 'g_vecZero' safe to assume or should this be able to be specified?  I don't
		// think most places calling here can give any input on blood direction though
		// WAIT, how about UTIL_RandomBloodVector() ?
		UTIL_BloodDrips(vecSpot, bloodDir, bloodColor, amount);
	}
}//UTIL_SpawnBlood


int DamageDecal( CBaseEntity *pEntity, int bitsDamageType )
{
	return ::DamageDecal(pEntity, bitsDamageType, 0);
}
int DamageDecal( CBaseEntity *pEntity, int bitsDamageType, int bitsDamageTypeMod )
{
	if (!pEntity) {
		return (DECAL_GUNSHOT1 + RANDOM_LONG(0, 4));
	}
	
	return pEntity->DamageDecal( bitsDamageType, bitsDamageTypeMod );
}

void DecalGunshot( TraceResult *pTrace, int iBulletType )
{
	// Is the entity valid
	if (!UTIL_IsValidEntity(pTrace->pHit)) {
		return;
	}

	if ( VARS(pTrace->pHit)->solid == SOLID_BSP || VARS(pTrace->pHit)->movetype == MOVETYPE_PUSHSTEP )
	{
		CBaseEntity *pEntity = NULL;
		// Decal the wall with a gunshot
		if ( !FNullEnt(pTrace->pHit) )
			pEntity = CBaseEntity::Instance(pTrace->pHit);

		switch( iBulletType )
		{
		case BULLET_PLAYER_9MM:
		case BULLET_MONSTER_9MM:
		case BULLET_PLAYER_MP5:
		case BULLET_MONSTER_MP5:
		case BULLET_PLAYER_BUCKSHOT:
		case BULLET_PLAYER_357:
		default:
			// smoke and decal
			UTIL_GunshotDecalTrace( pTrace, DamageDecal( pEntity, DMG_BULLET ) );
			break;
		case BULLET_MONSTER_12MM:
			// smoke and decal
			UTIL_GunshotDecalTrace( pTrace, DamageDecal( pEntity, DMG_BULLET ) );
			break;
		case BULLET_PLAYER_CROWBAR:
			// wall decal
			UTIL_DecalTrace( pTrace, DamageDecal( pEntity, DMG_CLUB ) );
			break;
		}

		//MODDD - what was this supposed to do again
		//UTIL_GunshotDecalTraceForceDefault( pTrace, DamageDecal( pEntity, DMG_BULLET ) );

	}
}

//MODDD - repurposed 'DecalGunshot' above to replace bullethit decals (DECAL_GUNSHOT1 through 4)
// with scorch marks instead.  If the entity hit chooses its own decal's that are different, those
// will still be used instead.   Unknown if scorch marks should always replace even that.
// If unknown what iBulletType to use, just use DMG_CLUB, 0.
// islave did that through calling DecalGunshot with BULLET_PLAYER_CROWBAR for bullettype.
void DecalSafeSmallScorchMark(TraceResult *pTrace, int bitsDamageType, int bitsDamageTypeMod){
	if (!UTIL_IsValidEntity(pTrace->pHit)) {
		return;
	}

	if ( VARS(pTrace->pHit)->solid == SOLID_BSP || VARS(pTrace->pHit)->movetype == MOVETYPE_PUSHSTEP )
	{
		CBaseEntity *pEntity = NULL;
		// Decal the wall with a gunshot
		if ( !FNullEnt(pTrace->pHit) )
			pEntity = CBaseEntity::Instance(pTrace->pHit);
		
		//UTIL_DecalTrace( pTrace, DamageDecal( pEntity, DMG_CLUB ) );
		int decalFromEnt = DamageDecal(pEntity, bitsDamageType, bitsDamageTypeMod);

		if(decalFromEnt >= DECAL_GUNSHOT1 && decalFromEnt <= DECAL_GUNSHOT5){
			// ok, replace it.
			// CHANGE: use scorch marks 1 and 2 instead, 3 is larger.
			//int newDecal = DECAL_SMALLSCORCH1 + RANDOM_LONG(0, 2);
			int newDecal = DECAL_SMALLSCORCH1 + RANDOM_LONG(0, 1);
			UTIL_DecalTrace( pTrace, newDecal );
		}else{
			// nope, use that.
			UTIL_DecalTrace( pTrace, decalFromEnt );
		}

		//MODDD - ???
		//UTIL_GunshotDecalTraceForceDefault( pTrace, DamageDecal( pEntity, DMG_BULLET ) );
	}
}

// like above, but takes one custom decal to paint instead.
void DecalSafeDecal(TraceResult *pTrace, int decalChoice, int bitsDamageType, int bitsDamageTypeMod){
	if (!UTIL_IsValidEntity(pTrace->pHit)) {
		return;
	}
	if ( VARS(pTrace->pHit)->solid == SOLID_BSP || VARS(pTrace->pHit)->movetype == MOVETYPE_PUSHSTEP )
	{
		CBaseEntity *pEntity = NULL;
		if ( !FNullEnt(pTrace->pHit) )
			pEntity = CBaseEntity::Instance(pTrace->pHit);

		int decalFromEnt = DamageDecal(pEntity, bitsDamageType, bitsDamageTypeMod);

		if(decalFromEnt >= DECAL_GUNSHOT1 && decalFromEnt <= DECAL_GUNSHOT5){
			int newDecal = decalChoice;
			UTIL_DecalTrace( pTrace, newDecal );
		}else{
			// nope, use that.
			UTIL_DecalTrace( pTrace, decalFromEnt );
		}
	}
}







//
// EjectBrass - tosses a brass shell from passed origin at passed velocity
//
void EjectBrass ( const Vector &vecOrigin, const Vector &vecVelocity, float rotation, int model, int soundtype )
{
	// FIX: when the player shoots, their gun isn't in the same position as it is on the model other players see.

	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, vecOrigin );
		WRITE_BYTE( TE_MODEL);
		WRITE_COORD( vecOrigin.x);
		WRITE_COORD( vecOrigin.y);
		WRITE_COORD( vecOrigin.z);
		WRITE_COORD( vecVelocity.x);
		WRITE_COORD( vecVelocity.y);
		WRITE_COORD( vecVelocity.z);
		WRITE_ANGLE( rotation );
		WRITE_SHORT( model );
		WRITE_BYTE ( soundtype);
		WRITE_BYTE ( 25 );// 2.5 seconds
	MESSAGE_END();
}


//MODDD - found this way, not part of compiles.
/*
// UNDONE: This is no longer used?
void ExplodeModel( const Vector &vecOrigin, float speed, int model, int count )
{
	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, vecOrigin );
		WRITE_BYTE ( TE_EXPLODEMODEL );
		WRITE_COORD( vecOrigin.x );
		WRITE_COORD( vecOrigin.y );
		WRITE_COORD( vecOrigin.z );
		WRITE_COORD( speed );
		WRITE_SHORT( model );
		WRITE_SHORT( count );
		WRITE_BYTE ( 15 );// 1.5 seconds
	MESSAGE_END();
}
*/

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

//bleh.



void UTIL_SetDeadPlayerTruce(BOOL arg_playerDeadTruce){

	if(g_f_playerDeadTruce != arg_playerDeadTruce){
		g_f_playerDeadTruce = arg_playerDeadTruce;

		if(arg_playerDeadTruce == TRUE){
			// If this is starting the truce, go through every single entity.
			// Any with an enemy that the monster should currently no longer be hostile toward
			// (hgrunt vs. barney, vice versa) will forget the enemy and do whatever else.

			int i;
			int i2;
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

				if(monsterTest->m_hEnemy != NULL){
					int daRel = monsterTest->IRelationship(monsterTest->m_hEnemy);
					if(daRel == R_NO || daRel == R_AL){
						// relationship is now NO or AL with my 'enemy'?  Cancel that then
						monsterTest->ForgetEnemy();
					}
				}
			}//END OF through all entities.

		}//arg_playerDeadTruce == TRUE check
	}//arg/g match check
}//UTIL_SetDeadPlayerTruce


