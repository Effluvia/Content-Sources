
#ifndef UTIL_SHARED_H
#define UTIL_SHARED_H


// SHORT VERSION:
// Most stuff in here is available for client/server files to see in C++, but not
// C lang files (pm_shared, etc.).  For showing up in both, use common/const.h.

// Similar preprocessor terms, macros, constants, whatever you call them, have been
// moved/merged here to reduce the chance of conflicts or inconsistencies on making
// changes in the future.  After all, changing how DLLEXPORT works wouldn't be fun
// if it were defined in twelve places.

// A lot of util_shared.h's #define's are moved here from dlls/extdll.h.
// Things that aren't specific to the server or clientside, like defining min & max
// methods, should be moved here.
// extdll.h, as "extdll" is an old term for server-related, should retain things 
// that were usually meant for serverside.
// It's included clientside mostly just to get some things to compile client & server-
// side (player weapons), but it feels like bad organization to have side-neutral
// utility methods or types like "typdef int BOOL".  Hence, util_shared.

// And unsure why but doing #define's that create methods already defined in system
// libraries like max, min, fabs BEFORE including the system library causes a bunch
// of errors in those library files.
// My guess is this makes a conflict with whatever way the libraries were meant to
// handle their own min/max and defining it here first throws that off.
// Best way is to do sensitive #define's like this after including the libary
// (includes for there in external_lib_include.h), possibly with "ifndef" checks to
// not override the system ones if defined.

// ALSO, a few files are #include'd further down after a lot of macro/constant stuff.

// Should "#pragma once" be used a lot more often?

/////////////////////////////////////////////////////////////////////////////////////






// Want these to go lots of places.
#include "external_lib_include.h"
#include "const.h"
#include "progdefs.h"
#include "vector.h"
#include "util_preprocessor.h"
#include "util_printout.h"

//#include "activity.h"
//#include "enginecallback.h"


// yay, don't need that.
/*
#ifdef CLIENT_DLL?
//#include "cvardef.h"
#else
#include "cdll_dll.h"
#endif
*/

EASY_CVAR_EXTERN_DEBUGONLY(hiddenMemPrintout)



//MODDDMIRROR - new macro (preprocessor constant in this case) to tell how many
// elements gHUD.Mirrors and when some mirror-related util.cpp methods are
// counting the number of "env_mirror" entities on the map.
// Originally 32.
#define MIRROR_MAX 32


// Hey, could always use a bucket of prime numbers, right?
// See the "Primes" array for those.
#define NUMBER_OF_PRIMES 177



// defaults for clientinfo messages
#define DEFAULT_VIEWHEIGHT 28



//...I have no clue why this is the only different one.
// It's only used by dlls/h_export.cpp.  Yes, this is proven.
// Renamed from DLL_CALL_CONV, or DLL Calling Convention, since "__stdcall"
// seems to describe the calling convention for the compiler to keep in mind
// when compiling the method.  There is more than enough reason to believe
// the name "DLLEXPORT" is better suited to the '__declspec(dllexport)' above.



// From dlls/util.h, may as well be shared since SND_CHANGE_PITCH is needed in cl_dlls/ev_hldm.h.
#define SND_SPAWNING		(1<<8)		// duplicated in protocol.h we're spawing, used in some cases for ambients 
#define SND_STOP			(1<<5)		// duplicated in protocol.h stop sound
#define SND_CHANGE_VOL		(1<<6)		// duplicated in protocol.h change sound vol
#define SND_CHANGE_PITCH	(1<<7)		// duplicated in protocol.h change sound pitch


//MODDD - turned into global const's for conveniently getting .x, .y, .z out of if needed.
// The 'DLL_GLOBAL' is meaningless but eh, g_vecZero had it
// Old example:
//#define VECTOR_CONE_1DEGREES	Vector(x, y, z)

extern DLL_GLOBAL const Vector VECTOR_CONE_1DEGREES;
extern DLL_GLOBAL const Vector VECTOR_CONE_2DEGREES;
extern DLL_GLOBAL const Vector VECTOR_CONE_3DEGREES;
extern DLL_GLOBAL const Vector VECTOR_CONE_4DEGREES;
extern DLL_GLOBAL const Vector VECTOR_CONE_5DEGREES;
extern DLL_GLOBAL const Vector VECTOR_CONE_6DEGREES;
extern DLL_GLOBAL const Vector VECTOR_CONE_7DEGREES;
extern DLL_GLOBAL const Vector VECTOR_CONE_8DEGREES;
extern DLL_GLOBAL const Vector VECTOR_CONE_9DEGREES;
extern DLL_GLOBAL const Vector VECTOR_CONE_10DEGREES;
extern DLL_GLOBAL const Vector VECTOR_CONE_15DEGREES;
extern DLL_GLOBAL const Vector VECTOR_CONE_20DEGREES;

// used by the player's auto-aim I think.  May as well be constants for everywhere.
// wait, no.
// NO.   See player.h for AUTOAIM_2DEGREES, those are what I was thinking of.
// These 'DOT_#DEGREE' ones are never used
/*
#define DOT_1DEGREE   0.9998476951564
#define DOT_2DEGREE   0.9993908270191
#define DOT_3DEGREE   0.9986295347546
#define DOT_4DEGREE   0.9975640502598
#define DOT_5DEGREE   0.9961946980917
#define DOT_6DEGREE   0.9945218953683
#define DOT_7DEGREE   0.9925461516413
#define DOT_8DEGREE   0.9902680687416
#define DOT_9DEGREE   0.9876883405951
#define DOT_10DEGREE  0.9848077530122
#define DOT_15DEGREE  0.9659258262891
#define DOT_20DEGREE  0.9396926207859
#define DOT_25DEGREE  0.9063077870367
*/



#ifndef CLIENT_DLL
//!!! Bunch of stuff mostly from util.h (serverside).
// Keeping it only clientside as this just doesn't exist serverside.
// ---
// Keeps clutter down a bit, when using a float as a bit-vector
#define SetBits(flBitVector, bits)		((flBitVector) = (int)(flBitVector) | (bits))
#define ClearBits(flBitVector, bits)	((flBitVector) = (int)(flBitVector) & ~(bits))
#define FBitSet(flBitVector, bit)		((int)(flBitVector) & (bit))
#endif





#define DMG_GENERIC			0			// generic damage was done
#define DMG_CRUSH			(1 << 0)	// crushed by falling or moving object
#define DMG_BULLET			(1 << 1)	// shot
#define DMG_SLASH			(1 << 2)	// cut, clawed, stabbed
#define DMG_BURN			(1 << 3)	// heat burned
#define DMG_FREEZE			(1 << 4)	// frozen
#define DMG_FALL			(1 << 5)	// fell too far
#define DMG_BLAST			(1 << 6)	// explosive blast damage
#define DMG_CLUB			(1 << 7)	// crowbar, punch, headbutt
#define DMG_SHOCK			(1 << 8)	// electric shock
#define DMG_SONIC			(1 << 9)	// sound pulse shockwave
#define DMG_ENERGYBEAM		(1 << 10)	// laser or other high energy beam 

//MODDD - note: Why is  1 << 11   missing?

#define DMG_NEVERGIB		(1 << 12)	// with this bit OR'd in, no damage type will be able to gib victims upon death
#define DMG_ALWAYSGIB		(1 << 13)	// with this bit OR'd in, any damage type can be made to gib victims upon death.
#define DMG_DROWN			(1 << 14)	// Drowning

//...what?  Nothing uses this constant.
//#define DMG_FIRSTTIMEBASED  DMG_DROWN

// time-based damage
//NOTE:  I believe this is just a way of saying, 1's for all timed damage (included), 0's for all else.
//       Any continual time damage is indicated by the 2nd damage bit (bitsDamageTypeMod) having DMG_TIMEDEFFECT or DMG_TIMEDEFFECTIGNORE.
//       This just shows the timed damages of the first bitmask as initial strikes.
//       Any continual damage still uses the 2nd bitmask for the aforementioned choices regardless.

//#define DMG_TIMEBASED		(~(0x00003fff))
// Going with the version from clientside files to support TF damage types... I guess?
//#define DMG_TIMEBASED		(~(0xff003fff))
// NEVERMIND, if they're phased out in both places now it doesn't make sense to care about those bits.
// Also notice the ~ (bitmask 'not').  So it would be 0xffffc000
// That means, bits 14 to 31 from the right (calling the right-most bit #0) are part of DMG_TIMEBASED.
// Although Half-Life (excluding that weird client-only TFC thing?) doesn't do anything with bits 24 to 31.
// They may as well be excluded.  This leaves a proper bitmask of: 0x00ffc000
#define DMG_TIMEBASED		(0x00ffc000)
// Now that's great and all but HOW ABOUT WE LIST OUT THE BITS LIKE A NORMAL PERSON.
// Beats me why they didn't, but too late now.



#define DMG_PARALYZE		(1 << 15) // slows affected creature down
#define DMG_NERVEGAS		(1 << 16) // nerve toxins, very bad
#define DMG_POISON			(1 << 17) // blood poisioning
#define DMG_RADIATION		(1 << 18) // radiation exposure
#define DMG_DROWNRECOVER	(1 << 19) // drowning recovery
#define DMG_ACID			(1 << 20) // toxic chemicals or acid burns
#define DMG_SLOWBURN		(1 << 21) // in an oven
#define DMG_SLOWFREEZE		(1 << 22) // in a subzero freezer
#define DMG_MORTAR			(1 << 23) // Hit by air raid (done to distinguish grenade from mortar)



//TF ADDITIONS
// Don't even know why this was ever anywhere, but ok.  From clientside files (health.h & eventscripts.h), never
// present serverside.  Commented out!
/*
#define DMG_IGNITE			(1 << 24) // Players hit by this begin to burn
#define DMG_RADIUS_MAX		(1 << 25) // Radius damage with this flag doesn't decrease over distance
#define DMG_RADIUS_QUAKE	(1 << 26) // Radius damage is done like Quake. 1/2 damage at 1/2 radius.
#define DMG_IGNOREARMOR		(1 << 27) // Damage ignores target's armor
#define DMG_AIMED			(1 << 28) // Does Hit location damage
#define DMG_WALLPIERCING	(1 << 29) // Blast Damages ents through walls

#define DMG_CALTROP				(1<<30)
#define DMG_HALLUC				(1<<31)

// TF Healing Additions for TakeHealth
#define DMG_IGNORE_MAXHEALTH	DMG_IGNITE
// TF Redefines since we never use the originals
#define DMG_NAIL				DMG_SLASH
#define DMG_NOT_SELF			DMG_FREEZE

#define DMG_TRANQ				DMG_MORTAR
#define DMG_CONCUSS				DMG_SONIC
*/




//MODDD - new.  Careful not to overflow this integer, these powers of 2 are getting high!
// Would have started with "24", but 24 - 31 may be placeholders for Team Fortress damage types, 
// according to health.h.
// NOTICE: beyond "31" in 1 << 31  is not valid.  Using a 2nd bitmask when referring to these values...

//#define DMG_TIMEDEFFECT			(1 << 32) // timed damage that must be differentiated from "generic".
//#define DMG_BLEEDING			(1 << 33)   // bleeding, usually inflicted from strong melee attacks.  Medkits cure it.

#define DMG_TIMEDEFFECT			(1 << 0) // timed damage that must be differentiated from "generic".  This is non-initial strike damage.
// Do not confuse with the mask "DMG_TIMEBASED", which is unique and not inclusive of any other damage types.

#define DMG_BLEEDING			(1 << 1)   //  bleeding, usually inflicted from strong melee attacks.  Medkits cure it.
#define DMG_TIMEDEFFECTIGNORE	(1 << 2)   // same as TIMEDEFFECT, but made to ignore armor (regardless of the cvar).
#define DMG_BARNACLEBITE		(1 << 3)   // not timed.  Just sent by the barnacle's execution bite to NPCs to mark not to ignore (if they would).
#define DMG_GAUSS				(1 << 4)   // coming from the player's gauss weapon. Some things (apache) are now immune to it.  ...or not? that got canceled.
#define DMG_PROJECTILE          (1 << 5)   // so far, only for crossbow bolt direct hits.   
#define DMG_HITBOX_EQUAL		(1 << 6)   // signal that this type of damage can't be increased by hitting particular hitboxes.
										   // It is still completely up to any given monster's TraceAttack / TakeDamage to implement this
										   // (check for the presence of DMG_HITBOX_EQUAL in bitsDamageTypeMod and deny enhancing damage
										   //  per headshots, etc. accordingly).
										   // For instance, for lightning attacks, different amounts of damage for body, leg, arm, or headshots don't make sense.
										   // It's possible damage from NPC's just shouldn't even do this kind of damage anyways.
#define DMG_MAP_TRIGGER			(1 << 7)   // From map triggers (trigger_hurt most likely).  On doing high damage in a single attack,
										   // the intent is clear that the player is not meant to revive in a zone they can't reocver from.
										   // Such as falling into a deep pit with no way out, often has a high-damage hurt trigger there.
#define DMG_MAP_BLOCKED			(1 << 8)   // From being in the way of map geometry like heavy closing doors or wheels.
										   // As it is difficult to tell whether the player would be stuck if revived from dying to this,
										   // it is fine to play it safe and forbid reviving.  Often coincides with DMG_CRUSH, but it is not
										   // the case that map-only entities use this (tentacles and xen trees deal DMG_CRUSH).
										   // OR, check to see if any attempts to deal DMG_MAP_BLOCKED damage were dealt some short time after
										   // the player stops moving, or never stops moving for a decent while taking DMG_MAP_BLOCKED 
										   // (so not simply moving from being on a moving platforms).
										   // If so, it is safe to assume the player is stuck in something moving and should not be revived.
#define DMG_POISONHALF			(1 << 9)   // For crossbow bolts to use instead of DMG_POISON.  Has half the duration of poison (not a separate skill entry)
										   // Note that this itself isn't a separate timed damage, or else having DMG_POISON and DMG_HALFPOISON counting down
										   // at the same time would be possible (two poison indicator icons)... weird.


//Which types of damage in the new mask are secondary?
//When other types are added, add them like   (DMG_BLEEDING | NEW | ALSONEW | ...)
#define DMG_TIMEBASEDMOD		(DMG_BLEEDING)



// these are the damage types that are allowed to gib corpses
#define DMG_GIB_CORPSE		( DMG_CRUSH | DMG_FALL | DMG_BLAST | DMG_SONIC | DMG_CLUB )

// these are the damage types that have client hud art
//MODDD - needed edit for bleeding damage!

#define DMG_SHOWNHUD		(DMG_POISON | DMG_ACID | DMG_FREEZE | DMG_SLOWFREEZE | DMG_DROWN | DMG_BURN | DMG_SLOWBURN | DMG_NERVEGAS | DMG_RADIATION | DMG_SHOCK)

//NOTICE: "DMG_TIMEDEFFECT" is just for non-initial strike damage, and is not what triggers the signs.  It is too inspecific.
//It is not a mask, but a way to tell apart timed damage from generic damage when needed (the "ignore armor" cvar).
#define DMG_SHOWNHUDMOD		(DMG_BLEEDING);


//MODDD - new bitmasks for "curables".  That is, conditions that can be cured by some item.
//The significance of this list is playing on hard difficulty with the "timedDamageEndlessOnHard" 
//CVar on.  This way, only CURABLES (by some item) last forever, and non-curables don't (that
//would be far too cruel).
//Perhaps "DMG_ACID" should appear ingame and be cured by the antidote or something? unsure.
//UNUSED, this mechanism wasn't needed.  Or wasn't particularly helpful; would have led to 
//taking the "long" way.
#define DMG_CURABLE			(DMG_NERVEGAS | DMG_POISON | DMG_RADIATION)
#define DMG_CURABLEMOD		(DMG_BLEEDING | DMG_POISON)

//MODDD - any damages
#define DMG_ARMORBLOCKEXCEPTION		(0) //empty.
// doesn't involve "bleeding".  That is the initial strike.  It will leave "DMG_TIMEDEFFECTIGNORE", so it works.
#define DMG_ARMORBLOCKEXCEPTIONMOD	(DMG_TIMEDEFFECTIGNORE)




//MODDD - NOTE - Pay little attention to these.  They're just arbitrary, for helping an iterator method
// see which damage type is which.  For instance, "itbd_Poison" and "DMG_POISON" further below have no link,
// but "itbd_Poison" is useless outside of Player.cpp (and monsters.cpp since they take timeddamage too now)
// while "DMG_POISON" is referred to both by attackers and player.cpp.
#define itbd_Paralyze		0		
#define itbd_NerveGas		1
#define itbd_Poison			2
#define itbd_Radiation		3
#define itbd_DrownRecover	4
#define itbd_Acid			5
#define itbd_SlowBurn		6
#define itbd_SlowFreeze		7

// more flexible way of recording the first itbd that belongs to m_bitsDamageTypeMod.
#define itbd_BITMASK2_FIRST	8

//MODDD - addition.
#define itbd_Bleeding		8

//MODDD - record the count of all itbd values.   also renamed from CDMG_TIMEBASED to itbd_MAX.
#define itbd_COUNT			9






/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// extern things. CONTENT HERE.
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

extern DLL_GLOBAL const Vector g_vecZero;

extern int giAmmoIndex;


extern int AmmoIndex_9mm;
extern int AmmoIndex_357;
extern int AmmoIndex_ARgrenades;
extern int AmmoIndex_bolts;
extern int AmmoIndex_buckshot;
extern int AmmoIndex_rockets;
extern int AmmoIndex_uranium;
extern int AmmoIndex_Hornets;
extern int AmmoIndex_HandGrenade;
extern int AmmoIndex_SatchelCharge;
extern int AmmoIndex_Snarks;
extern int AmmoIndex_TripMine;
extern int AmmoIndex_ChumToads;


extern BOOL globalflag_muteDeploySound;


extern DLL_GLOBAL short g_sModelIndexBubbles;// holds the index for the bubbles model


extern globalvars_t *gpGlobals;

extern int Primes[NUMBER_OF_PRIMES];


#ifdef CLIENT_DLL
	extern char globalbuffer_cl_mod_version[128];
	extern char globalbuffer_cl_mod_date[128];

	extern char globalbuffer_sv_mod_version_cache[128];
	extern char globalbuffer_sv_mod_date_cache[128];

	extern char globalbuffer_cl_mod_display[128];
	extern char globalbuffer_sv_mod_display[128];
#else
	extern char globalbuffer_sv_mod_version[128];
	extern char globalbuffer_sv_mod_date[128];
#endif



//GERMAN GIBS
//~as seen here:
//m_iGermanGibModelIndex =PRECACHE_MODEL ("models/germanygibs.mdl");
//pGib->Spawn( "models/germangibs.mdl" );
//~there is a slight variation in name: germanygibs vs. germangibs.  If this is just a mistake from Valve, we'll just go with either.
//UPDATE: We're sticking to the name "g_hgibs.mdl" to be more consistent with other naming.
#define GERMAN_GIB_PATH "models/g_hgibs.mdl"


// These are player-specific but still need to be included here to reach the HUD logic clientside.
////////////////////////////////////////////////////////////////////////////
// How many seconds worth of air does the air tank have?  Coordinate with the GUI (clientside, ammo.cpp)
#define PLAYER_AIRTANK_TIME_MAX 120
// How much longjump charge can the player hold at one time?
#define PLAYER_LONGJUMPCHARGE_MAX 100
// How much charge does one 'longjumpcharge' grant?
#define PLAYER_LONGJUMP_PICKUPADD 25
////////////////////////////////////////////////////////////////////////////

#define DEFAULT_SPARK_BALLS 6


class CBaseEntity;
class CBasePlayerWeapon;



// Originally in cl_dlls/hl/ev_hldm.h and dlls/weapons.h.
// bullet types
typedef	enum
{
	BULLET_NONE = 0,
	BULLET_PLAYER_9MM, // glock
	BULLET_PLAYER_MP5, // mp5
	BULLET_PLAYER_357, // python
	BULLET_PLAYER_BUCKSHOT, // shotgun
	BULLET_PLAYER_CROWBAR, // crowbar swipe

	BULLET_MONSTER_9MM,
	BULLET_MONSTER_MP5,
	BULLET_MONSTER_12MM,
} Bullet;


// And why were these only in dlls/util.h??
// ev_hldm.cpp had a few places that require a 'hull' choice and had to use a literal number
// instead.  UGH!!
typedef enum { ignore_monsters = 1, dont_ignore_monsters = 0, missile = 2 } IGNORE_MONSTERS;
typedef enum { ignore_glass = 1, dont_ignore_glass = 0 } IGNORE_GLASS;
//MODDD - added the name "HULL_TYPE" to this.  why not.
typedef enum { point_hull = 0, human_hull = 1, large_hull = 2, head_hull = 3 } HULL_TYPE;




// METHODS
////////////////////////////////////////////////////////////////////////////////////////


extern float UTIL_WeaponTimeBase(void);
extern int UTIL_SharedRandomLong(unsigned int seed, int low, int high);
extern float UTIL_SharedRandomFloat(unsigned int seed, float low, float high);

extern BOOL CanAttack(float attack_time, float curtime, BOOL isPredicted);


extern int GetAmmoIndex(const char* psz);
extern int MaxAmmoCarry(const char* psz);
//MODDD NEW - prototype so that other places can call this method too.
extern void AddAmmoNameToAmmoRegistry_Primary(int iId, const char* szAmmoname, int iAmmoMax);
extern void AddAmmoNameToAmmoRegistry_Secondary(int iId, const char* szAmmoname, int iAmmoMax);
extern void RegisterWeapon(CBasePlayerWeapon* pWeapon, CBasePlayerWeapon* pAryWeaponStore[]);
extern void PostWeaponRegistry(void);





//MODDD - from util.h. Checking for equal strings is really not server-exclusive.
inline BOOL FStrEq(const char*sz1, const char*sz2)
	{ return (strcmp(sz1, sz2) == 0); }


//MODDD - have this.  Although clientside shouldn't touch entities too much.
extern const char* FClassname(CBaseEntity* derp);


extern BOOL stringStartsWith(const char* source, const char* startswith);


extern BOOL checkMatch(const char* src1, char* src2);
extern BOOL checkMatch(const char* src1, char* src2, int size);
extern BOOL checkMatchIgnoreCase(const char* src1, char* src2);
extern BOOL checkMatchIgnoreCase(const char* src1, char* src2, int size);


extern const char* tryIntToString(int arg_src);
extern int tryStringToInt(const char* arg_src);
extern float tryStringToFloat(const char* arg_src);
extern const char* tryFloatToString(float arg_src);
extern void tryFloatToStringBuffer(char* dest, float arg_src);


extern void lowercase(char* src);
extern void lowercase(char* src, int size);

extern float roundToNearest(float num);


extern void UTIL_substring(char* dest, const char* src, int startIndex, int endIndex);
extern int UTIL_findCharFirstPos(const char* search, char toFind);
extern void UTIL_appendTo(char* dest, const char* add, int appendStartLoc);

extern void appendTo(char* dest, const char* add, int* refIndex);
extern void appendToAndTerminate(char* dest, const char* add, int* refIndex);
extern void appendTo(char* dest, const char* add, int* refIndex, char endCharacter);
extern void appendToAndTerminate(char* dest, const char* add, int* refIndex, char endCharacter);
extern void appendTo(char* dest, const int numb, int* refIndex);
extern void appendToAndTerminate(char* dest, const int numb, int* refIndex);
extern void appendTo(char* dest, const int numb, int* refIndex, char endCharacter);
extern void appendToAndTerminate(char* dest, const int numb, int* refIndex, char endCharacter);
extern void appendTo(char* dest, const char add, int* refIndex);
extern void appendToAndTerminate(char* dest, const char add, int* refIndex);
extern void strncpyTerminate(char* dest, const char* send, int arg_length);


extern void copyString(const char* src, char* dest);
extern void copyString(const char* src, char* dest, int size);

extern void UTIL_appendToEnd(char* dest, const char* add);


extern int lengthOfString(const char* src);
extern int lengthOfString(const char* src, int storeSize);
extern BOOL isStringEmpty(const char* arg_src);
extern BOOL stringEndsWith(const char* arg_src, const char* arg_endsWith);
extern BOOL stringEndsWithIgnoreCase(const char* arg_src, const char* arg_endsWith);


extern BOOL checkSubFileExistence(const char* arg_subdir);

extern void determineHiddenMemPath(void);
extern BOOL checkValveSubFileExistence(const char* arg_subdir);

extern BOOL checkSubDirectoryExistence(const char* arg_subdir);
extern void loadHiddenCVars(void);
extern void saveHiddenCVars(void);


extern void convertIntToBinary(char* buffer, unsigned int arg, unsigned int binaryDigits);



extern void UTIL_StringToVector( float *pVector, const char *pString );
extern void UTIL_StringToIntArray( int *pVector, int count, const char *pString );
//simple version for just one number.
extern float UTIL_clamp(float argTest, float argMin, float argMax);
//MODDD - version that skips normalization offered.
extern Vector UTIL_ClampVectorToBox( const Vector &input, const Vector &clampSize );
extern Vector UTIL_ClampVectorToBoxNonNormalized( const Vector &input, const Vector &clampSize );



// Common method for determing whether the current game is multiplayer or not.
// Behaves like 'bIsMultiplayer' (cl_dlls/hl/hl_weapons.cpp) if clientside, and
// behaves like 'IsMultiplayer' of gamerules for serverside.
extern BOOL IsMultiplayer(void);

// Yes, these can now be called from server or clientside.
// However, it is still almost always called serverside, as it used to be.
// Calling this from clientside just cuts out the middleman, always ended up there anyway.
// And changed up a bit, can be told how many glowballs to spawn.
extern void UTIL_Sparks(const Vector& position);
extern void UTIL_Sparks(const Vector& position, int arg_ballsToSpawn, float arg_extraSparkMulti);



// Search for water transition along a vertical line
extern int UTIL_PointContents(const Vector& vec);
extern float UTIL_WaterLevel(const Vector& position, float minz, float maxz);
extern void UTIL_Bubbles(Vector mins, Vector maxs, int count);
extern void UTIL_BubbleTrail(Vector from, Vector to, int count);

extern void InitShared(void);
extern void PrecacheShared(void);
extern void ClearWeaponInfoCache(void);

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

#endif //END OF UTIL_SHARED_H