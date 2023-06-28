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
//
// Misc utility code
//

// Some things not necessarily specific to client or serverside moved to const.h and util_shared.h.

//MODDD - WHY NOT?
#ifndef UTIL_H
#define UTIL_H

#include "util_shared.h"  //includes util_printout.h
#include "util_entity.h"
#include "client_message.h" //for access to gmsg IDs and the LinkUserMessages method.
#include "activity.h"
#include "enginecallback.h"
//MODDD - needed to have "CGlobalState" available to prototypes here.
#include "saverestore.h"

//MODDD - referred to in here, yes.
EASY_CVAR_EXTERN_DEBUGONLY(soundAttenuationStuka)
EASY_CVAR_EXTERN_DEBUGONLY(soundVolumeStuka)


// Keep in synch with the array if more entries are added!
#define aryGibInfo_MAX_SIZE 10


//
// Constants that were used only by QC (maybe not used at all now)
//
// Un-comment only as needed
//
#define LANGUAGE_ENGLISH				0
#define LANGUAGE_GERMAN					1
#define LANGUAGE_FRENCH					2
#define LANGUAGE_BRITISH				3

#define AMBIENT_SOUND_STATIC			0	// medium radius attenuation
#define AMBIENT_SOUND_EVERYWHERE		1
#define AMBIENT_SOUND_SMALLRADIUS		2
#define AMBIENT_SOUND_MEDIUMRADIUS		4
#define AMBIENT_SOUND_LARGERADIUS		8
#define AMBIENT_SOUND_START_SILENT		16
#define AMBIENT_SOUND_NOT_LOOPING		32

#define SPEAKER_START_SILENT			1	// wait for trigger 'on' to start announcements

//MODDD - sound flags moved to util_shared.h.

#define LFO_SQUARE			1
#define LFO_TRIANGLE		2
#define LFO_RANDOM			3

// func_rotating
#define SF_BRUSH_ROTATE_Y_AXIS		0
#define SF_BRUSH_ROTATE_INSTANT		1
#define SF_BRUSH_ROTATE_BACKWARDS	2
#define SF_BRUSH_ROTATE_Z_AXIS		4
#define SF_BRUSH_ROTATE_X_AXIS		8
#define SF_PENDULUM_AUTO_RETURN		16
#define SF_PENDULUM_PASSABLE		32


//MODDD - NEW
//momentary_rot_button
#define SF_MOMENTARY_ROTATING_REQUIREMASTERTRIGGER		32

#define SF_BRUSH_ROTATE_SMALLRADIUS	128
#define SF_BRUSH_ROTATE_MEDIUMRADIUS 256
#define SF_BRUSH_ROTATE_LARGERADIUS 512

#define PUSH_BLOCK_ONLY_X	1
#define PUSH_BLOCK_ONLY_Y	2

//MODDD - "VEC_" macros moved to util_shared.h.

#define SVC_TEMPENTITY		23
#define SVC_INTERMISSION	30
#define SVC_CDTRACK			32
#define SVC_WEAPONANIM		35
#define SVC_ROOMTYPE		37
#define SVC_DIRECTOR		51



// triggers
#define SF_TRIGGER_ALLOWMONSTERS	1// monsters allowed to fire this trigger
#define SF_TRIGGER_NOCLIENTS		2// players not allowed to fire this trigger
#define SF_TRIGGER_PUSHABLES		4// only pushables can fire this trigger

// func breakable
#define SF_BREAK_TRIGGER_ONLY	1// may only be broken by trigger
#define SF_BREAK_TOUCH			2// can be 'crashed through' by running player (plate glass)
#define SF_BREAK_PRESSURE		4// can be broken by a player standing on it
#define SF_BREAK_CROWBAR		256// instant break if hit with crowbar

// func_pushable (it's also func_breakable, so don't collide with those flags)
#define SF_PUSH_BREAKABLE		128

#define SF_LIGHT_START_OFF		1

#define SPAWNFLAG_NOMESSAGE	1
#define SPAWNFLAG_NOTOUCH	1
#define SPAWNFLAG_DROIDONLY	4

#define SPAWNFLAG_USEONLY	1		// can't be touched, must be used (buttons)

#define TELE_PLAYER_ONLY	1
#define TELE_SILENT			2

#define SF_TRIG_PUSH_ONCE	1


// Sound Utilities

// sentence groups
//#define CBSENTENCENAME_MAX 16
//MODDD - upped!
#define CBSENTENCENAME_MAX 45

//MODDD - testing, ignoring that limit notice (there is no sound.h, ust hoping that isn't pre-compiled somehow, headers usually aren't...)
//SHIT SHIT SHIT.  Nope.  What is with the devs and having these un-alterable limits...

#define CVOXFILESENTENCEMAX 1536		// max number of sentences in game. NOTE: this must match
											// CVOXFILESENTENCEMAX in engine\sound.h!!!
//#define CVOXFILESENTENCEMAX		4000

#define cchMapNameMost 32

// Dot products for view cone checking
#define VIEW_FIELD_FULL		(float)-1.0 // +-180 degrees
#define VIEW_FIELD_WIDE		(float)-0.7 // +-135 degrees 0.1 // +-85 degrees, used for full FOV checks 
#define VIEW_FIELD_NARROW	(float)0.7 // +-45 degrees, more narrow check used to set up ranged attacks
#define VIEW_FIELD_ULTRA_NARROW	(float)0.9 // +-25 degrees, more narrow check used to set up ranged attacks


// All monsters need this data
#define DONT_BLEED			-1
#define BLOOD_COLOR_RED		(BYTE)70 //(BYTE)247
#define BLOOD_COLOR_YELLOW	(BYTE)195
// NEW!  Distinct green blood.  Most accurate choice pending but this looks ok
#define BLOOD_COLOR_GREEN	(BYTE)54
#define BLOOD_COLOR_BLACK	(BYTE)0 //black like oil
#define COLOR_SQUIDSPIT	(BYTE)22  // another similar choice: 84 ?



#define GIB_DUMMY_ID 0
#define GIB_HUMAN_ID 1
//MODDD - separate alien colors YELLOW and GREEN now supported, gib-wise too.  Read from alien blood color.
#define GIB_ALIEN_YELLOW_ID 2
#define GIB_ALIEN_GREEN_ID 3
#define GIB_GERMAN_ID 4

#define GIB_EXTRAMETAL_1_ID 5
#define GIB_EXTRAMETAL_2_ID 6
#define GIB_EXTRAMETAL_3_ID 7
#define GIB_EXTRAMETAL_4_ID 8
#define GIB_EXTRAMETAL_5_ID 9
#define GIB_EXTRAMETAL_6_ID GIB_GERMAN_ID



// For UTIL_SetGroupTrace and checks against g_groupop in dlls/client.cpp.   what?
#define GROUP_OP_AND	0
#define GROUP_OP_NAND	1




#define UTIL_EntitiesInPVS(pent)			(*g_engfuncs.pfnEntitiesInPVS)(pent)


// Keeps clutter down a bit, when writing key-value pairs
#define WRITEKEY_INT(pf, szKeyName, iKeyValue) ENGINE_FPRINTF(pf, "\"%s\" \"%d\"\n", szKeyName, iKeyValue)
#define WRITEKEY_FLOAT(pf, szKeyName, flKeyValue)								\
		ENGINE_FPRINTF(pf, "\"%s\" \"%f\"\n", szKeyName, flKeyValue)
#define WRITEKEY_STRING(pf, szKeyName, szKeyValue)								\
		ENGINE_FPRINTF(pf, "\"%s\" \"%s\"\n", szKeyName, szKeyValue)
#define WRITEKEY_VECTOR(pf, szKeyName, flX, flY, flZ)							\
		ENGINE_FPRINTF(pf, "\"%s\" \"%f %f %f\"\n", szKeyName, flX, flY, flZ)

// MODDD - several macros and macro methods (SetBits, ClearBits, FBitSet, FILE_GLOBAL,
// DLL_GLOBAL, CONSTANT, M_PI) moved to util_shared.h.






#define PLAYBACK_EVENT( flags, who, index ) PLAYBACK_EVENT_FULL( flags, who, index, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, 0, 0, 0, 0 );
#define PLAYBACK_EVENT_DELAY( flags, who, index, delay ) PLAYBACK_EVENT_FULL( flags, who, index, delay, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, 0, 0, 0, 0 );

#define EMIT_SOUND_ARRAY_FILTERED( chan, arg_array ) \
	UTIL_PlaySound( ENT(pev), chan , arg_array [ RANDOM_LONG(0,ARRAYSIZE( arg_array )-1) ], 1.0, ATTN_NORM, 0, RANDOM_LONG(95,105) ); 

#define EMIT_SOUND_ARRAY_STUKA_FILTERED( chan, arg_array ) \
	UTIL_PlaySound( ENT(pev), chan , arg_array [ RANDOM_LONG(0,ARRAYSIZE( arg_array )-1) ], EASY_CVAR_GET_DEBUGONLY(soundVolumeStuka), EASY_CVAR_GET_DEBUGONLY(soundAttenuationStuka), 0, RANDOM_LONG(m_voicePitch - 5,m_voicePitch + 5) ); 

//OLD WAY:
	//UTIL_PlaySound( ENT(pev), chan , arg_array [ RANDOM_LONG(0,ARRAYSIZE( arg_array )-1) ], 1.0, ATTN_NORM, 0, RANDOM_LONG(95,105) ); 

//#define RANDOM_SOUND_ARRAY( arg_array ) (arg_array) [ RANDOM_LONG(0,ARRAYSIZE( (arg_array) )-1) ]



#define PRECACHE_SOUND_ARRAY( a ) \
	{ for (int iLOCAL = 0; iLOCAL < ARRAYSIZE( a ); iLOCAL++ ) PRECACHE_SOUND((char *) a [iLOCAL]); }
#define PRECACHE_SOUND_ARRAY_SKIPSAVE( a ) \
	{ for (int iLOCAL = 0; iLOCAL < ARRAYSIZE( a ); iLOCAL++ ) PRECACHE_SOUND((char *) a [iLOCAL], TRUE); }
//#define PRECACHE_SOUND_ARRAY UTIL_PRECACHESOUND_ARRAY



// No need to worry, this "method?" is almost completely unused.  Looks to be only used in xen.cpp.
// It skips any soundSentenceSave checks.
#define EMIT_SOUND_ARRAY_DYN( chan, arg_array ) \
	EMIT_SOUND_DYN( ENT(pev), chan , arg_array [ RANDOM_LONG(0,ARRAYSIZE( arg_array )-1) ], 1.0, ATTN_NORM, 0, RANDOM_LONG(95,105) ); 

// This is used a lot more - much more flexible.
// This doesn't play the sound, it just grabs one sound path string from the array. It's up to the caller to give it the usual volume, attenuation, flags, pitch.
#define RANDOM_SOUND_ARRAY( arg_array ) (arg_array) [ RANDOM_LONG(0,ARRAYSIZE( (arg_array) )-1) ]





// forward class/typedef declarations.
class CBaseEntity;
class CBasePlayerItem;
class CBasePlayer;
class CWorld;
typedef struct GibInfo_s GibInfo_t;




//MODDD - NOTICE: left for reasons of compatability paranoia.  If only the script depends on this, it can be canned though.
//                See globals.cpp for more info.
extern DLL_GLOBAL int g_Language;

extern int giPrecacheGrunt;
//Why wasn't this externed everywhere before? I'm asking you, 'past me'. I'm not insane, I swear.
extern int global_useSentenceSave;

extern float previousFrameTime;
extern BOOL g_gamePaused;
extern BOOL g_gameLoaded;
extern BOOL g_mapLoaded;
extern BOOL g_mapLoadedEver;

extern int g_groupmask;
extern int g_groupop;

extern int gcallsentences;


extern unsigned short g_sTrailEngineChoice;
extern unsigned short g_sImitation7;
extern unsigned short g_sTrail;
extern unsigned short g_sTrailRA;
extern unsigned short g_sCustomBalls;
extern unsigned short g_sCustomBallsPowerup;
extern unsigned short g_quakeExplosionEffect;
extern unsigned short g_decalGunshotCustomEvent;
extern unsigned short g_sFreakyLight;
extern unsigned short g_sFriendlyVomit;
extern unsigned short g_sFloaterExplode;

//MODDD - important point before.
//extern globalvars_t *gpGlobals;


// extern arrays.
extern GibInfo_t aryGibInfo[aryGibInfo_MAX_SIZE];
extern const char* TOGGLE_STATE_STR[];
extern char gszallsentencenames[CVOXFILESENTENCEMAX][CBSENTENCENAME_MAX];




class UTIL_GroupTrace
{
public:
	UTIL_GroupTrace( int groupmask, int op );
	~UTIL_GroupTrace( void );

private:
	int m_oldgroupmask, m_oldgroupop;
};

//MODDD - new.
typedef struct GibInfo_s{
	const char* modelPath;
	int bodyMin;
	int bodyMax;
	int bloodColor;

} GibInfo_t;



typedef struct hudtextparms_s
{
	float	x;
	float	y;
	int		effect;
	byte	r1, g1, b1, a1;
	byte	r2, g2, b2, a2;
	float	fadeinTime;
	float	fadeoutTime;
	float	holdTime;
	float	fxTime;
	int		channel;
} hudtextparms_t;



// For printout convenience.    Probably should do that with tasks, as often as those numbers  come up.
//static const char* pStateNames[] = { "None", "Idle", "Combat", "Alert", "Hunt", "Prone", "Script", "PlayDead", "Dead" };
static const char* pStateNames[] = { "None", "Idle", "Combat", "Alert", "Prone", "Script", "Dead" };

typedef enum 
{
	MONSTERSTATE_NONE = 0,
	MONSTERSTATE_IDLE,
	MONSTERSTATE_COMBAT,
	MONSTERSTATE_ALERT,
	//MODDD - uh-oh.  you got cut.
	//MONSTERSTATE_HUNT,
	MONSTERSTATE_PRONE,
	MONSTERSTATE_SCRIPT,
	//MODDD - same for you.  The chumtoad works without this by the way.
	//MONSTERSTATE_PLAYDEAD,
	MONSTERSTATE_DEAD

} MONSTERSTATE;


// Things that toggle (buttons/triggers/doors) need this
typedef enum
{
	TS_AT_TOP,
	TS_AT_BOTTOM,
	TS_GOING_UP,
	TS_GOING_DOWN
} TOGGLE_STATE;



///////////////////////////////////////////////////////////////////////////
//MODDD - thanks to Spirit of Half-Life 1.9!
///////////////////////////////////////////////////////////////////////////

//LRC- the values used for the new "global states" mechanism.
typedef enum
{
	STATE_OFF = 0,	// disabled, inactive, invisible, closed, or stateless. Or non-alert monster.
	STATE_TURN_ON,  // door opening, env_fade fading in, etc.
	STATE_ON,		// enabled, active, visisble, or open. Or alert monster.
	STATE_TURN_OFF, // door closing, monster dying (?).
	STATE_IN_USE,	// player is in control (train/tank/barney/scientist).
					// In_Use isn't very useful, I'll probably remove it.
} STATE;

extern char* GetStringForState(STATE state);

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////



//
// How did I ever live without ASSERT?
//
#ifdef	DEBUG
void DBG_AssertFunction(BOOL fExpr, const char* szExpr, const char* szFile, int szLine, const char* szMessage);
#define ASSERT(f)		DBG_AssertFunction(f, #f, __FILE__, __LINE__, NULL)
#define ASSERTSZ(f, sz)	DBG_AssertFunction(f, #f, __FILE__, __LINE__, sz)
#else	// !DEBUG
#define ASSERT(f)
#define ASSERTSZ(f, sz)
#endif	// !DEBUG



inline edict_t *FIND_ENTITY_BY_CLASSNAME(edict_t* entStart, const char* pszName) 
{
	return FIND_ENTITY_BY_STRING(entStart, "classname", pszName);
}	

inline edict_t *FIND_ENTITY_BY_TARGETNAME(edict_t* entStart, const char* pszName) 
{
	return FIND_ENTITY_BY_STRING(entStart, "targetname", pszName);
}	

// for doing a reverse lookup. Say you have a door, and want to find its button.
inline edict_t *FIND_ENTITY_BY_TARGET(edict_t* entStart, const char* pszName) 
{
	return FIND_ENTITY_BY_STRING(entStart, "target", pszName);
}	

//!!!!!!!!!!!!!!!!!!!!!
// entity-related typedefs and constant methods like ENT have been moved to util_entity.h.

inline BOOL FStringNull(int iString) { return iString == iStringNull; }

const char* TOGGLE_STATE_STR_Safe(int argIndex);


inline BOOL FClassnameIs(edict_t* pent, const char* szClassname)
	{ return FStrEq(STRING(VARS(pent)->classname), szClassname); }
inline BOOL FClassnameIs(entvars_t* pev, const char* szClassname)
	{ return FStrEq(STRING(pev->classname), szClassname); }


// Misc. Prototypes
extern void UTIL_SetSize(entvars_t* pev, const Vector &vecMin, const Vector &vecMax);
//MODDD - new
extern void UTIL_SetSizeAlt( entvars_t* pev, const Vector &vecMin, const Vector &vecMax);


extern float UTIL_VecToYaw(const Vector &vec);
//MODDD - new
extern float UTIL_VecToYawRadians( const Vector &vecAng );
extern Vector UTIL_VecGetForward2D( const Vector &vecAng );
extern Vector UTIL_VecGetForward( const Vector &vecAng );
extern Vector UTIL_velocityToAngles( const Vector &vecVel);
extern Vector UTIL_YawToVec			(const float &yaw);

extern Vector UTIL_VecToAngles	(const Vector &vec);
extern float UTIL_AngleMod		(float a);
extern float UTIL_AngleDiff		( float destAngle, float srcAngle );

extern CBaseEntity *UTIL_FindEntityInSphere(CBaseEntity *pStartEntity, const Vector &vecCenter, float flRadius);
extern CBaseEntity *UTIL_FindEntityByString(CBaseEntity *pStartEntity, const char *szKeyword, const char *szValue );
extern CBaseEntity *UTIL_FindEntityByClassname(CBaseEntity *pStartEntity, const char *szName );
extern CBaseEntity *UTIL_FindEntityByTargetname(CBaseEntity *pStartEntity, const char *szName );
extern CBaseEntity *UTIL_FindEntityGeneric(const char *szName, Vector &vecSrc, float flRadius );

// returns a CBaseEntity pointer to a player by index.  Only returns if the player is spawned and connected
// otherwise returns NULL
// Index is 1 based
extern CBaseEntity *UTIL_PlayerByIndex( int playerIndex );

extern void UTIL_MakeVectors(const Vector &vecAngles);

// Pass in an array of pointers and an array size, it fills the array and returns the number inserted
extern int UTIL_MonstersInSphere( CBaseEntity **pList, int listMax, const Vector &center, float radius );
extern int UTIL_EntitiesInBox( CBaseEntity **pList, int listMax, const Vector &mins, const Vector &maxs, int flagMask );

//MODD - new version
extern int UTIL_NonDeadEntitiesInBox( CBaseEntity **pList, int listMax, const Vector &mins, const Vector &maxs, int flagMask );

//no longer necessary.
//extern int		UTIL_EntitiesInBoxAlsoBarnacles( CBaseEntity **pList, int listMax, const Vector &mins, const Vector &maxs, int flagMask );

inline void UTIL_MakeVectorsPrivate( const Vector &vecAngles, float *p_vForward, float *p_vRight, float *p_vUp )
{
	g_engfuncs.pfnAngleVectors( vecAngles, p_vForward, p_vRight, p_vUp );
}

//MODDD - new, why wasn't there a private "aim" vectors version too?
//Just like "UTIL_MakeAimVectors", this just inverts the "x" angle before sending it to the same engine method (pfnAngleVectors instead in this case).
//or... does MakeAimVectorsPrivate alone already imply what "Aim" does for the gpGlobals vectors version?
//TESTED.  Yes, MakeAimVectorsPrivate is 1-to-1 the same as MakeAimVectors (global vector equivalent).  Better to verify than ride on a wrong assumption.
inline void UTIL_MakeAimVectorsPrivate( const Vector &vecAngles, float *p_vForward, float *p_vRight, float *p_vUp )
{
	float rgflVec[3];
	vecAngles.CopyToArray(rgflVec);
	rgflVec[0] = -rgflVec[0];
	//MAKE_VECTORS(rgflVec);
	g_engfuncs.pfnAngleVectors(rgflVec, p_vForward, p_vRight, p_vUp );
}

//MODDD NOTE - a UTIL_MakeInvVectorsPrivate (based off of UTIL_MakeInvVectors) could also be made if needed.  Bring that SWAP define over here if so.


extern void UTIL_MakeAimVectors		( const Vector &vecAngles ); // like MakeVectors, but assumes pitch isn't inverted
extern void UTIL_MakeInvVectors		( const Vector &vec, globalvars_t *pgv );

extern void UTIL_SetOrigin			( entvars_t* pev, const Vector &vecOrigin );
//MODDD - Spirit of HL had this...  why doesn't it have both though anyways?  (Missing the "entvars_t*" version that comes with Half-Life as-is, like above)
extern void UTIL_SetOrigin		( CBaseEntity* pEntity, const Vector &vecOrigin );



extern void		UTIL_ParticleEffect		( const Vector &vecOrigin, const Vector &vecDirection, ULONG ulColor, ULONG ulCount );
extern void		UTIL_ScreenShake		( const Vector &center, float amplitude, float frequency, float duration, float radius );
extern void		UTIL_ScreenShakeAll		( const Vector &center, float amplitude, float frequency, float duration );
extern void		UTIL_ShowMessage		( const char *pString, CBaseEntity *pPlayer );
extern void		UTIL_ShowMessageAll		( const char *pString );
extern void		UTIL_ScreenFadeAll		( const Vector &color, float fadeTime, float holdTime, int alpha, int flags );
extern void		UTIL_ScreenFade			( CBaseEntity *pEntity, const Vector &color, float fadeTime, float fadeHold, int alpha, int flags );

extern void		UTIL_TraceLine			(const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, edict_t *pentIgnore, TraceResult *ptr);
extern void		UTIL_TraceLine			(const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, IGNORE_GLASS ignoreGlass, edict_t *pentIgnore, TraceResult *ptr);

extern void		UTIL_TraceHull			(const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, int hullNumber, edict_t *pentIgnore, TraceResult *ptr);
extern TraceResult	UTIL_GetGlobalTrace		(void);
extern void		UTIL_TraceModel			(const Vector &vecStart, const Vector &vecEnd, int hullNumber, edict_t *pentModel, TraceResult *ptr);

//MODDD
extern void UTIL_fromToBlood(CBaseEntity* arg_entSrc, CBaseEntity* arg_entDest, const float& arg_fltDamage);
extern void UTIL_fromToBlood(CBaseEntity* arg_entSrc, CBaseEntity* arg_entDest, const float& arg_fltDamage, const float &arg_fltdistanceHint);
extern void UTIL_fromToBlood(CBaseEntity* arg_entSrc, CBaseEntity* arg_entDest, const float& arg_fltDamage, const float &arg_fltdistanceHint, Vector* arg_suggestedTraceHullVecEndPos, Vector* arg_suggestedTraceHullStart, Vector* arg_suggestedTraceHullEnd);

extern Vector	UTIL_GetAimVector		(edict_t* pent, float flSpeed);

extern int		UTIL_IsMasterTriggered	(string_t sMaster, CBaseEntity *pActivator);
extern void		UTIL_BloodStream( const Vector &origin, const Vector &direction, int color, int amount );
extern void		UTIL_BloodDrips( const Vector &origin, const Vector &direction, int color, int amount );
extern Vector	UTIL_RandomBloodVector( void );
extern Vector	UTIL_RandomBloodVectorHigh(void);
extern BOOL		UTIL_ShouldShowBlood( int bloodColor );
extern void		UTIL_BloodDecalTrace( TraceResult *pTrace, int bloodColor );
extern void		UTIL_DecalTrace( TraceResult *pTrace, int decalNumber );
extern void		UTIL_PlayerDecalTrace( TraceResult *pTrace, int playernum, int decalNumber, BOOL bIsCustom );
extern void		UTIL_GunshotDecalTrace( TraceResult *pTrace, int decalNumber );

//MODDD
extern void		UTIL_GunshotDecalTraceForceDefault( TraceResult *pTrace, int decalNumber );


//MODDD - new
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern Vector	UTIL_GetProjectileVelocityExtra(const Vector& playerVelocity, float velocityMode);





extern void		UTIL_Explosion(int msg_dest, const Vector &location, short sprite, float size, int framerate, int flag);
extern void		UTIL_Explosion(int msg_dest, const float* pMsgOrigin, const Vector& location, short sprite, float size, int framerate, int flag);
extern void		UTIL_Explosion(int msg_dest, const float* pMsgOrigin, edict_t* ed, const Vector& location, short sprite, float size, int framerate, int flag);

extern void		UTIL_Explosion(int msg_dest, const Vector &location, float offsetx, float offsety, float offsetz, short sprite, float size, int framerate, int flag);
extern void		UTIL_Explosion(int msg_dest, const float* pMsgOrigin, const Vector& location, float offsetx, float offsety, float offsetz, short sprite, float size, int framerate, int flag);
extern void		UTIL_Explosion(int msg_dest, const float* pMsgOrigin, edict_t* ed, const Vector& location, float offsetx, float offsety, float offsetz, short sprite, float size, int framerate, int flag);

extern void		UTIL_Explosion(int msg_dest, const Vector &location, short sprite, float size, int framerate, int flag, const Vector& altLocation);
extern void		UTIL_Explosion(int msg_dest, const float* pMsgOrigin, const Vector& location, short sprite, float size, int framerate, int flag, const Vector& altLocation);
extern void		UTIL_Explosion(int msg_dest, const float* pMsgOrigin, edict_t* ed, const Vector& location, short sprite, float size, int framerate, int flag, const Vector& altLocation);




extern void		UTIL_Explosion(int msg_dest, entvars_t* pev, const Vector &location, short sprite, float size, int framerate, int flag);
extern void		UTIL_Explosion(int msg_dest, const float* pMsgOrigin, entvars_t* pev, const Vector& location, short sprite, float size, int framerate, int flag);
extern void		UTIL_Explosion(int msg_dest, const float* pMsgOrigin, edict_t* ed, entvars_t* pev, const Vector& location, short sprite, float size, int framerate, int flag);

extern void		UTIL_Explosion(int msg_dest, entvars_t* pev, const Vector &location, float offsetx, float offsety, float offsetz, short sprite, float size, int framerate, int flag);
extern void		UTIL_Explosion(int msg_dest, const float* pMsgOrigin, entvars_t* pev, const Vector& location, float offsetx, float offsety, float offsetz, short sprite, float size, int framerate, int flag);
extern void		UTIL_Explosion(int msg_dest, const float* pMsgOrigin, edict_t* ed, entvars_t* pev, const Vector& location, float offsetx, float offsety, float offsetz, short sprite, float size, int framerate, int flag);

extern void		UTIL_Explosion(int msg_dest, entvars_t* pev, const Vector &location, float offsetx, float offsety, float offsetz, short sprite, float size, int framerate, int flag, float shrapMod);
extern void		UTIL_Explosion(int msg_dest, const float* pMsgOrigin, entvars_t* pev, const Vector& location, float offsetx, float offsety, float offsetz, short sprite, float size, int framerate, int flag, float shrapMod);
extern void		UTIL_Explosion(int msg_dest, const float* pMsgOrigin, edict_t* ed, entvars_t* pev, const Vector& location, float offsetx, float offsety, float offsetz, short sprite, float size, int framerate, int flag, float shrapMod);

extern void		UTIL_Explosion(int msg_dest, entvars_t* pev, const Vector &location, short sprite, float size, int framerate, int flag, const Vector& altLocation);
extern void		UTIL_Explosion(int msg_dest, const float* pMsgOrigin, entvars_t* pev, const Vector& location, short sprite, float size, int framerate, int flag, const Vector& altLocation);
extern void		UTIL_Explosion(int msg_dest, const float* pMsgOrigin, edict_t* ed, entvars_t* pev, const Vector& location, short sprite, float size, int framerate, int flag, const Vector& altLocation);

extern void		UTIL_Explosion(int msg_dest, entvars_t* pev, const Vector &location, short sprite, float size, int framerate, int flag, const Vector& altLocation, float shrapMode);
extern void		UTIL_Explosion(int msg_dest, const float* pMsgOrigin, entvars_t* pev, const Vector& location, short sprite, float size, int framerate, int flag, const Vector& altLocation, float shrapMode);
extern void		UTIL_Explosion(int msg_dest, const float* pMsgOrigin, edict_t* ed, entvars_t* pev, const Vector& location, short sprite, float size, int framerate, int flag, const Vector& altLocation, float shrapMode);



extern void		UTIL_Explosion(int msg_dest, const Vector &location, float offsetx, float offsety, float offsetz, short sprite, float size, int framerate, int flag, const Vector& altLocation);
extern void		UTIL_Explosion(int msg_dest, const float* pMsgOrigin, const Vector& location, float offsetx, float offsety, float offsetz, short sprite, float size, int framerate, int flag, const Vector& altLocation);
extern void		UTIL_Explosion(int msg_dest, const float* pMsgOrigin, edict_t* ed, const Vector& location, float offsetx, float offsety, float offsetz, short sprite, float size, int framerate, int flag, const Vector& altLocation);

extern void		UTIL_Explosion(int msg_dest, entvars_t* pev, const Vector &location, float offsetx, float offsety, float offsetz, short sprite, float size, int framerate, int flag, const Vector& altLocation);
extern void		UTIL_Explosion(int msg_dest, const float* pMsgOrigin, entvars_t* pev, const Vector& location, float offsetx, float offsety, float offsetz, short sprite, float size, int framerate, int flag, const Vector& altLocation);
extern void		UTIL_Explosion(int msg_dest, const float* pMsgOrigin, edict_t* ed, entvars_t* pev, const Vector& location, float offsetx, float offsety, float offsetz, short sprite, float size, int framerate, int flag, const Vector& altLocation);

extern void		UTIL_Explosion(int msg_dest, entvars_t* pev, const Vector &location, float offsetx, float offsety, float offsetz, short sprite, float size, int framerate, int flag, const Vector& altLocation, float shrapMod);
extern void		UTIL_Explosion(int msg_dest, const float* pMsgOrigin, entvars_t* pev, const Vector& location, float offsetx, float offsety, float offsetz, short sprite, float size, int framerate, int flag, const Vector& altLocation, float shrapMod);
extern void		UTIL_Explosion(int msg_dest, const float* pMsgOrigin, edict_t* ed, entvars_t* pev, const Vector& location, float offsetx, float offsety, float offsetz, short sprite, float size, int framerate, int flag, const Vector& altLocation, float shrapMod);





extern void		UTIL_QuakeExplosion(int msg_dest, entvars_t* pev, const Vector& location, float offsetx, float offsety, float offsetz, float shrapMod);
extern void		UTIL_QuakeExplosion(int msg_dest, const float* pMsgOrigin, entvars_t* pev, const Vector& location, float offsetx, float offsety, float offsetz, float shrapMod);
extern void		UTIL_QuakeExplosion(int msg_dest, const float* pMsgOrigin, edict_t* ed, entvars_t* pev, const Vector& location, float offsetx, float offsety, float offsetz, float shrapMod);

extern void		UTIL_SpriteOrQuakeExplosion(int msg_dest, entvars_t* pev, const Vector& location, float offsetx, float offsety, float offsetz, short sprite, float size, int brightness, const Vector& altLocation, float shrapMod);
extern void		UTIL_SpriteOrQuakeExplosion(int msg_dest, const float* pMsgOrigin, entvars_t* pev, const Vector& location, float offsetx, float offsety, float offsetz, short sprite, float size, int brightness, const Vector& altLocation, float shrapMod);
extern void		UTIL_SpriteOrQuakeExplosion(int msg_dest, const float* pMsgOrigin, edict_t* ed, entvars_t* pev, const Vector& location, float offsetx, float offsety, float offsetz, short sprite, float size, int brightness, const Vector& altLocation, float shrapMod);




extern void		UTIL_Smoke(int msg_dest, const Vector& location, short sprite, float scale, int framerate);
extern void		UTIL_Smoke(int msg_dest, const float* pMsgOrigin, const Vector& location, short sprite, float scale, int framerate);
extern void		UTIL_Smoke(int msg_dest, const float* pMsgOrigin, edict_t* ed, const Vector& location, short sprite, float scale, int framerate);

extern void		UTIL_Smoke(int msg_dest, const Vector& location, float offsetx, float offsety, float offsetz, short sprite, float scale, int framerate);
extern void		UTIL_Smoke(int msg_dest, const float* pMsgOrigin, const Vector& location, float offsetx, float offsety, float offsetz, short sprite, float scale, int framerate);
extern void		UTIL_Smoke(int msg_dest, const float* pMsgOrigin, edict_t* ed, const Vector& location, float offsetx, float offsety, float offsetz, short sprite, float scale, int framerate);


extern void		UTIL_ExplosionSmoke(int msg_dest, const Vector& location, short sprite, float scale, int framerate);
extern void		UTIL_ExplosionSmoke(int msg_dest, const float* pMsgOrigin, const Vector& location, short sprite, float scale, int framerate);
extern void		UTIL_ExplosionSmoke(int msg_dest, const float* pMsgOrigin, edict_t* ed, const Vector& location, short sprite, float scale, int framerate);

extern void		UTIL_ExplosionSmoke(int msg_dest, const Vector& location, float offsetx, float offsety, float offsetz, short sprite, float scale, int framerate);
extern void		UTIL_ExplosionSmoke(int msg_dest, const float* pMsgOrigin, const Vector& location, float offsetx, float offsety, float offsetz, short sprite, float scale, int framerate);
extern void		UTIL_ExplosionSmoke(int msg_dest, const float* pMsgOrigin, edict_t* ed, const Vector& location, float offsetx, float offsety, float offsetz, short sprite, float scale, int framerate);


extern BOOL UTIL_getExplosionsHaveSparks(void);

// NOTE: use EMIT_SOUND_DYN to set the pitch of a sound. Pitch of 100
// is no pitch shift.  Pitch > 100 up to 255 is a higher pitch, pitch < 100
// down to 1 is a lower pitch.   150 to 70 is the realistic range.
// EMIT_SOUND_DYN with pitch != 100 should be used sparingly, as it's not quite as
// fast as EMIT_SOUND (the pitchshift mixer is not native coded).

extern void UTIL_PlaySound(entvars_t* entity, int channel, const char* pszName, float volume, float attenuation );
extern void UTIL_PlaySound(entvars_t* entity, int channel, const char* pszName, float volume, float attenuation, BOOL useSoundSentenceSave );
extern void UTIL_PlaySound(edict_t* entity, int channel, const char* pszName, float volume, float attenuation );
extern void UTIL_PlaySound(edict_t* entity, int channel, const char* pszName, float volume, float attenuation, BOOL useSoundSentenceSave  );
extern void UTIL_PlaySound(entvars_t* entity, int channel, const char* pszName, float volume, float attenuation, int flags, int pitch );
extern void UTIL_PlaySound(entvars_t* entity, int channel, const char* pszName, float volume, float attenuation, int flags, int pitch, BOOL useSoundSentenceSave );
extern void UTIL_PlaySound(edict_t* entity, int channel, const char* pszName, float volume, float attenuation, int flags, int pitch );
extern void UTIL_PlaySound(edict_t* entity, int channel, const char* pszName, float volume, float attenuation, int flags, int pitch, BOOL useSoundSentenceSave );

extern void EMIT_SOUND_DYN(edict_t* entity, int channel, const char* pszName, float volume, float attenuation, int flags, int pitch);


//WARNING - bypasses soundsentencesave filter. Careful. UTIL_PlaySound can take the same params now anyway.
inline void EMIT_SOUND(edict_t* entity, int channel, const char* pszName, float volume, float attenuation){
	EMIT_SOUND_DYN(entity, channel, pszName, volume, attenuation, 0, PITCH_NORM);
}


//MODDD - filtered version, so that "STOP" can apply to the sentence-trick too.
//        And yes it works, it goes through _FILTERED like the rest of the soundsentencesave system.
//        After all even a "STOP" is just an order through the same sound playing call.
inline void UTIL_StopSound(edict_t* entity, int channel, const char* pszName){
	UTIL_PlaySound(entity, channel, pszName, 0, 0, SND_STOP, PITCH_NORM);
}
//And me too for specifying whether to use the soundSentenceSave in the call instead of leaving it up to context (the setting on the entity)
inline void UTIL_StopSound(edict_t* entity, int channel, const char* pszName, BOOL useSoundSentenceSave){
	UTIL_PlaySound(entity, channel, pszName, 0, 0, SND_STOP, PITCH_NORM, useSoundSentenceSave);
}



extern void EMIT_SOUND_SUIT(edict_t* entity, const char* pszName);
extern void STOP_SOUND_SUIT(edict_t* entity, const char* pszName);
extern void EMIT_GROUPID_SUIT(edict_t* entity, int isentenceg);
extern void EMIT_GROUPNAME_SUIT(edict_t* entity, const char* groupname);


//MODDD - several new versions added here too.
extern void UTIL_EmitAmbientSound( entvars_t *entity, const Vector &vecOrigin, const char *samp, float vol, float attenuation );
extern void UTIL_EmitAmbientSound( entvars_t *entity, const Vector &vecOrigin, const char *samp, float vol, float attenuation, BOOL useSoundSentenceSave );
extern void UTIL_EmitAmbientSound( entvars_t *entity, const Vector &vecOrigin, const char *samp, float vol, float attenuation, int fFlags, int pitch );
extern void UTIL_EmitAmbientSound( entvars_t *entity, const Vector &vecOrigin, const char *samp, float vol, float attenuation, int fFlags, int pitch, BOOL useSoundSentenceSave );
extern void UTIL_EmitAmbientSound( edict_t *entity, const Vector &vecOrigin, const char *samp, float vol, float attenuation, int fFlags, int pitch );
extern void UTIL_EmitAmbientSound( edict_t *entity, const Vector &vecOrigin, const char *samp, float vol, float attenuation, int fFlags, int pitch, BOOL useSoundSentenceSave );
extern void EMIT_AMBIENT_SOUND( edict_t *entity, const Vector &vecOrigin, const char *samp, float vol, float attenuation, int fFlags, int pitch );

extern void UTIL_PRECACHESOUND(char* path);
extern void UTIL_PRECACHESOUND(char* path, BOOL dontSkipSave);

//any other places these need externing at?
extern void UTIL_PRECACHESOUND_ARRAY(const char* a[], int aSize);
extern void UTIL_PRECACHESOUND_ARRAY(const char* a[], int aSize, BOOL dontSkipSave);


extern void UTIL_TE_ShowLine(const Vector& vec1, const Vector& vec2);
extern void UTIL_TE_ShowLine(float x1, float y1, float z1, float x2, float y2, float z2);

extern void UTIL_drawRect(const Vector& vec1, const Vector& vec2);
extern void UTIL_drawRect(float x1, float y1, float z1, float x2, float y2, float z2);

extern void UTIL_drawBox(const Vector& vec1, const Vector& vec2);
extern void UTIL_drawBox(float x1, float y1, float z1, float x2, float y2, float z2);


extern void UTIL_drawLineFrame(const Vector& vec1, const Vector& vec2, int r, int g, int b);
extern void UTIL_drawLineFrame(const Vector& vec1, const Vector& vec2, int width, int r, int g, int b);
extern void UTIL_drawLineFrame(float x1, float y1, float z1, float x2, float y2, float z2, int width, int r, int g, int b);
extern void UTIL_drawLineFrame(float x1, float y1, float z1, float x2, float y2, float z2, int width, int life, int r, int g, int b);

extern void UTIL_drawPointFrame(const Vector& vecPoint, int width, int r, int g, int b);
extern void UTIL_drawPointFrame(float point_x, float point_y, float point_z, int width, int r, int g, int b);


extern void UTIL_drawRectFrame(const Vector& vec1, const Vector& vec2, int width, int r, int g, int b);
extern void UTIL_drawRectFrame(float x1, float y1, float z1, float x2, float y2, float z2, int width, int r, int g, int b);

extern void UTIL_drawBoxFrame(const Vector& vec1, const Vector& vec2, int width, int r, int g, int b);
extern void UTIL_drawBoxFrame(const Vector& vec1, const Vector& vec2, int width, int life, int r, int g, int b);

extern void UTIL_drawBoxFrame(float x1, float y1, float z1, float x2, float y2, float z2, int width, int r, int g, int b);
extern void UTIL_drawBoxFrame(float x1, float y1, float z1, float x2, float y2, float z2, int width, int life, int r, int g, int b);

extern void UTIL_drawLineFrameBoxAround(const Vector& vec1, int width, int boxSize, int r, int g, int b);
extern void UTIL_drawLineFrameBoxAround(float x1, float y1, float z1, int width, int boxSize, int r, int g, int b);

extern void UTIL_drawLineFrameBoxAround2(const Vector& vec1, int width, int boxSize, int r, int g, int b);
extern void UTIL_drawLineFrameBoxAround2(float x1, float y1, float z1, int width, int boxSize, int r, int g, int b);

extern void UTIL_drawLineFrameBoxAround3(const Vector& vec1, int width, int boxSize, int r, int g, int b);
extern void UTIL_drawLineFrameBoxAround3(float x1, float y1, float z1, int width, int boxSize, int r, int g, int b);



extern void UTIL_TE_BeamPoints(const Vector& vec1, const Vector& vec2, int frameStart, int frameRate, int life, int width, int noise, int r, int g, int b, int brightness, int speed);
extern void UTIL_TE_BeamPoints(float x1, float y1, float z1, float x2, float y2, float z2, int frameStart, int frameRate, int life, int width, int noise, int r, int g, int b, int brightness, int speed);

extern void UTIL_TE_BeamPoints_Rect(const Vector& vec1, const Vector& vec2, int frameStart, int frameRate, int life, int width, int noise, int r, int g, int b, int brightness, int speed);
extern void UTIL_TE_BeamPoints_Rect(float x1, float y1, float z1, float x2, float y2, float z2, int frameStart, int frameRate, int life, int width, int noise, int r, int g, int b, int brightness, int speed);

extern void UTIL_TE_BeamPoints_Box(const Vector& vec1, const Vector& vec2, int frameStart, int frameRate, int life, int width, int noise, int r, int g, int b, int brightness, int speed);
extern void UTIL_TE_BeamPoints_Box(float x1, float y1, float z1, float x2, float y2, float z2, int frameStart, int frameRate, int life, int width, int noise, int r, int g, int b, int brightness, int speed);



extern Vector UTIL_rotateShift(const Vector& src, const Vector& forward );
extern Vector UTIL_rotateShift(const Vector& src, const float forwardX, const float forwardY, const float forwardZ );
extern Vector UTIL_rotateShift(const float srcX, const float srcY, const float srcZ, const Vector& forward);
extern Vector UTIL_rotateShift(const float srcX, const float srcY, const float srcZ, const float forwardX, const float forwardY, const float forwardZ );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern void UTIL_Ricochet( const Vector &position, float scale );
extern void UTIL_RicochetTracer(const Vector& position, const Vector& vecDir);

extern float UTIL_Approach( float target, float value, float speed );
extern float UTIL_ApproachAngle( float target, float value, float speed );
extern float UTIL_AngleDistance( float next, float cur );

extern Vector UTIL_Intersect( Vector vecSrc, Vector vecDst, Vector vecMove, float flSpeed );

extern Vector UTIL_projectionComponent(const Vector& u, const Vector& n);
extern Vector UTIL_projectionComponentPreserveMag(const Vector& u, const Vector& n);

////////////////////////////////////////////////////////////////////
extern void UTIL_Remove( CBaseEntity *pEntity );

//MODDD
extern BOOL UTIL_IsDeadEntity( CBaseEntity* ent);
//MODDD
extern BOOL UTIL_IsAliveEntity( CBaseEntity* ent);

//MODDD
extern BOOL UTIL_IsValidEntity( CBaseEntity* ent);

extern BOOL UTIL_IsValidEntity( edict_t *pent );
extern BOOL UTIL_TeamsMatch( const char *pTeamName1, const char *pTeamName2 );

// Use for ease-in, ease-out style interpolation (accel/decel)
extern float UTIL_SplineFraction( float value, float scale );

//MODDD - few methods moved to util_shared.h

// allows precacheing of other entities
extern void UTIL_PrecacheOther( const char *szClassname );


extern BOOL UTIL_GetNextBestWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pCurrentWeapon );


// prints as transparent 'title' to the HUD
extern void		UTIL_HudMessageAll( const hudtextparms_t &textparms, const char *pMessage );
extern void		UTIL_HudMessage( CBaseEntity *pEntity, const hudtextparms_t &textparms, const char *pMessage );

// for handy use with ClientPrint params
extern char *UTIL_dtos1( int d );
extern char *UTIL_dtos2( int d );
extern char *UTIL_dtos3( int d );
extern char *UTIL_dtos4( int d );

// Writes message to console with timestamp and FragLog header.
extern void UTIL_LogPrintf( char *fmt, ... );

// Sorta like FInViewCone, but for nonmonsters. 
extern float UTIL_DotPoints ( const Vector &vecSrc, const Vector &vecCheck, const Vector &vecDir );

extern void UTIL_StripToken( const char *pKey, char *pDest );// for redundant keynames

// Misc functions
extern void SetMovedir(entvars_t* pev);
extern Vector VecBModelOrigin( entvars_t* pevBModel );
extern int BuildChangeList( LEVELLIST *pLevelList, int maxList );


int USENTENCEG_Pick(int isentenceg, char *szfound);
int USENTENCEG_PickSequential(int isentenceg, char *szfound, int ipick, int freset);
void USENTENCEG_InitLRU(unsigned char *plru, int count);

void SENTENCEG_Init();
void SENTENCEG_Stop(edict_t *entity, int isentenceg, int ipick);
int SENTENCEG_PlayRndI(edict_t *entity, int isentenceg, float volume, float attenuation, int flags, int pitch);
int SENTENCEG_PlayRndSz(edict_t *entity, const char *szrootname, float volume, float attenuation, int flags, int pitch);
//MODDD - new
int SENTENCEG_PlayRndSz_Ambient(edict_t *entity, Vector vOrigin, const char *szgroupname, float volume, float attenuation, int flags, int pitch);

int SENTENCEG_PlaySequentialSz(edict_t *entity, const char *szrootname, float volume, float attenuation, int flags, int pitch, int ipick, int freset);
int SENTENCEG_GetIndex(const char *szrootname);
int SENTENCEG_Lookup(const char* pszName, char *sentencenum);


//MODDD - why did we have no option for playing a singular sentence on any particular entity like a utility, not just TalkMonster's???
// And don't send over an exclamation mark, we already add one here.
void SENTENCEG_PlaySingular(entvars_t* entity, int arg_channel, const char *pszSentence, float volume, float attenuation );
void SENTENCEG_PlaySingular(edict_t* entity, int arg_channel, const char *pszSentence, float volume, float attenuation );
void SENTENCEG_PlaySingular(entvars_t* entity, int arg_channel, const char *pszSentence, float volume, float attenuation, int flag );
void SENTENCEG_PlaySingular(edict_t* entity, int arg_channel, const char *pszSentence, float volume, float attenuation, int flag );
void SENTENCEG_PlaySingular(entvars_t* entity, int arg_channel, const char *pszSentence, float volume, float attenuation, int flag, int pitch );
void SENTENCEG_PlaySingular(edict_t* entity, int arg_channel, const char *pszSentence, float volume, float attenuation, int flag, int pitch );


void TEXTURETYPE_Init();
char TEXTURETYPE_Find(char *name);
float TEXTURETYPE_PlaySound(TraceResult *ptr,  Vector vecSrc, Vector vecEnd, int iBulletType);



void UTIL_SetGroupTrace( int groupmask, int op );
void UTIL_UnsetGroupTrace( void );


//MODDDMIRROR
Vector UTIL_MirrorVector( Vector angles );
Vector UTIL_MirrorPos ( Vector endpos );


extern void UTIL_generateFreakyLight( const Vector& arg_origin);


//PrintQueue related:

extern Vector UTIL_getFloor(const Vector &vecStart, const float& distDown, IGNORE_MONSTERS igmon, edict_t *pentIgnore );
extern BOOL isErrorVector(const Vector& vec);


extern void method_precacheAll(void);

extern BOOL UTIL_IsFacing( entvars_t *pevTest, const Vector &vecLookAtTest );
extern BOOL UTIL_IsFacing( entvars_t *pevTest, const Vector &vecLookAtTest, const float& arg_tolerance );
extern BOOL UTIL_IsFacingAway( entvars_t *pevTest, const Vector &vecLookAtTest );
extern BOOL UTIL_IsFacingAway( entvars_t *pevTest, const Vector &vecLookAtTest, const float& arg_tolerance );


extern float randomInvert(const float& arg_flt);
extern float randomAbsoluteValue(const float& arg_fltMin, const float& arg_fltMax);
extern int randomValueInt(const int& arg_min, const int& arg_max);
extern float randomValue(const float& arg_fltMin, const float& arg_fltMax);

extern void UTIL_deriveColorFromMonsterHealth(const float& curHealth, const float& maxHealth, int& r, int& g, int& b);

extern void attemptSendBulletSound(const Vector& bulletHitLoc, entvars_t* pevShooter);

//extern CBaseEntity *FindEntityForwardOLDVERSION( CBaseEntity *pMe );
extern CBaseEntity *FindEntityForward( CBasePlayer *pMe );


extern float timeDelayFilter(float arg_delay);
extern Vector getRotatedVectorAboutZAxis(const Vector& arg_vec, const float& arg_deg);

extern void UTIL_ServerMassCVarReset(entvars_t* pev);

extern BOOL getGermanModelsAllowed(void);
extern BOOL verifyModelExists(char* path);

extern int attemptInterpretSpawnFlag(const char* pszSpawnFlags);
extern int UTIL_BloodColorRedFilter(BOOL robotReplacementModelExists);

Vector projectionOntoPlane(Vector arg_vectOnto, Vector arg_planeNormal);


extern CBaseEntity* UTIL_CreateNamedEntity(const char* arg_entityName);


extern BOOL entityHidden(CBaseEntity* test);
extern BOOL entityHidden(edict_t* test);

extern void UTIL_playOrganicGibSound(entvars_t* pevSoundSource);
extern void UTIL_playMetalGibSound(entvars_t* pevSoundSource);
extern void UTIL_playMeleeMetalHitSound(entvars_t* pevSoundSource);
extern void UTIL_playMetalTextureHitSound(entvars_t* pevSoundSource, const Vector& vecOrigin);

extern void updateCVarRefs(BOOL isEarly);

extern void turnWorldLightsOn();
extern void turnWorldLightsOff();

extern void OnBeforeChangeLevelTransition(void);
extern void OnMapLoadPreStart(void);
extern void OnMapLoadStart(void);
extern void OnMapLoadEnd(void);

extern void ResetDynamicStaticIDs(void);
extern void SaveDynamicIDs(CGlobalState* argGS);
extern void RestoreDynamicIDs(CGlobalState* argGS);

extern BOOL GermanModelOrganicLogic(void);

extern CWorld* getWorld(void);






//MODDD - moved prototypes from basemonster.h
/////////////////////////////////////////
//MODDD - this variation doesn't even have an implementation?
//BOOL FBoxVisible ( entvars_t *pevLooker, entvars_t *pevTarget );
BOOL FBoxVisible ( entvars_t *pevLooker, entvars_t *pevTarget, Vector &vecTargetOrigin, float flSize = 0.0 );
BOOL FBoxVisible(entvars_t* pevLooker, entvars_t* pevTarget, float flSize = 0.0);

Vector VecCheckToss ( entvars_t *pev, const Vector &vecSpot1, Vector vecSpot2, float flGravityAdj = 1.0 );
Vector VecCheckThrow ( entvars_t *pev, const Vector &vecSpot1, Vector vecSpot2, float flSpeed, float flGravityAdj = 1.0 );
/////////////////////////////////////////

extern void printBasicEntityInfo(CBaseEntity* entRef);
extern void printBasicEntityInfo(edict_t* theCaller, CBaseEntity* entRef);
extern void printBasicTraceInfo(const TraceResult& tr);
extern void printBasicTraceInfo(edict_t* theCaller, const TraceResult& tr);



//MODDD - moved from weapons.h
// changed a bit though.
extern void UTIL_SpawnBlood(const Vector& vecSpot, int bloodColor, int amount);
extern void UTIL_SpawnBlood(const Vector& vecSpot, const Vector& bloodDir, int bloodColor, int amount);


//MODDD - extra damage bitmask support.
extern int DamageDecal( CBaseEntity *pEntity, int bitsDamageType );
extern int DamageDecal( CBaseEntity *pEntity, int bitsDamageType, int bitsDamageTypeMod );

extern void DecalGunshot( TraceResult *pTrace, int iBulletType );

extern void DecalSafeSmallScorchMark(TraceResult* pTrace, int bitsDamageType, int bitsDamageTypeMod);
extern void DecalSafeDecal(TraceResult* pTrace, int decalChoice, int bitsDamageType, int bitsDamageTypeMod);


extern void EjectBrass (const Vector &vecOrigin, const Vector &vecVelocity, float rotation, int model, int soundtype );
//MODDD - the implementation got canned, so why wasn't this too?
//extern void ExplodeModel( const Vector &vecOrigin, float speed, int model, int count );


//MODDD - these versions moved from basemonster.h. They are inspecific
// to the monster called on and don't need to be for monsters only.
// Implementations in combat.cpp.
//NOTICE: any RadiusDamage methods that don't provide flRadius have been renamed to
// "RadiusDamageAutoRadius" to avoid some call ambiguity (the parameters you supply could
// go to unintended places if say, more than one overload accepts the same amount of numbers
// but gives them a different purpose... The compiler may make a bad decision)
extern void RadiusDamageTest( Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, float flRadius, int iClassIgnore, int bitsDamageType, int bitsDamageTypeMod );
extern void RadiusDamageAutoRadius(Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType );
extern void RadiusDamageAutoRadius(Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType, int bitsDamageTypeMod  );
//MODDD - added bitsDamageTypeMod versions.
extern void RadiusDamage( Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, float flRadius, int iClassIgnore, int bitsDamageType );
extern void RadiusDamage( Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, float flRadius, int iClassIgnore, int bitsDamageType, int bitsDamageTypeMod );


extern void UTIL_SetDeadPlayerTruce(BOOL arg_playerDeadTruce);








#ifndef CLIENT_DLL

#define PRINTQUEUE_NAMESTRINGSIZE 16
#define PRINTQUEUE_STRINGS 5
#define PRINTQUEUE_STRINGSIZE 30
//#define PRINTQUEUE_NUMBEROF 4     No, varries per setup.

// "+ 2" for boundary room (commans, stoppers, etc.)
#define PRINTQUEUE_TOTALEXPECTED PRINTQUEUE_STRINGS * (PRINTQUEUE_STRINGSIZE + 5)

//MODDD - keep it real yo!

class PrintQueue{
public:
	int latestPlace;
	char contents[PRINTQUEUE_STRINGS][PRINTQUEUE_STRINGSIZE];
	char displayName[PRINTQUEUE_NAMESTRINGSIZE];

	PrintQueue(const char* arg_name){
		//startest index to add.
		latestPlace = 0;
		int nameLength = lengthOfString(arg_name, PRINTQUEUE_NAMESTRINGSIZE);
		strncpyTerminate(&displayName[0], arg_name, nameLength);
	}

	inline void printOut(char *format, ...){
		va_list argptr;
		va_start(argptr, format);
		char* tempResult = UTIL_VarArgsVA(format, argptr );;
		va_end(argptr);
	}

	
	
	void sendToPrintQueue(const char* src, ...){
		if(latestPlace < 5){
			va_list argptr;
			va_start(argptr, src);
			const char* tempResult = UTIL_VarArgsVA(src, argptr );
			//strncpyTerminate(&contents[latestPlace][0], &UTIL_VarArgsVA(src, argptr )[0], lengthOfString(src, PRINTQUEUE_STRINGSIZE) );
			strncpyTerminate(&contents[latestPlace][0], tempResult, lengthOfString(tempResult, PRINTQUEUE_STRINGSIZE) );
			va_end(argptr);
			latestPlace++;
		}else{
			//PROBLEM! TOO MANY IN QUEUE!
		}
	}

	void receivePrintQueue(char* dest, int* positionOverhead){
		//next place to start writing characters to.
		int* recentIndex = positionOverhead;
		appendTo(dest, displayName, recentIndex, ':');
		if(latestPlace == 0){
			//just print <n>.
			appendToAndTerminate(dest, "<n>", recentIndex, '|');
		}else{
			for(int i = 0; i < latestPlace; i++){
				appendTo(dest, (i+1), recentIndex, ':');
				//appendTo(dest, '*', recentIndex);

				if(i == latestPlace - 1){
					appendTo(dest, &contents[i][0], recentIndex);
				}else{
					appendTo(dest, &contents[i][0], recentIndex, ',');
				}
			}//END OF for(...)
			appendTo(dest, '|', recentIndex);
		}//END OF else OF if(latestPlace == 0)
		//no, already occurring.
		//*positionOverhead += recentIndex;
	}//END OF sendPrintQueue
	void clearPrintQueue(){
		//reset.  Overwrite or ignore others.
		latestPlace = 0;
	}
};

//alternate way now.
extern void PRINTQUEUE_STUKA_SEND(PrintQueue& toPrint, const char* src, ...);

#endif //END OF CLIENT_DLL Check (lack of)






#endif //END OF #ifdef UTIL_H
