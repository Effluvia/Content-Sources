
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// preliminary.
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

#ifdef CLIENT_DLL
	// Yes, extdll.h clientside.  Hey, hl_weapons.cpp did it first.
	#include "extdll.h"
	#include "const.h"
	#include "progdefs.h"
	#include "util.h"
	#include "cbase.h"
	#include "util_shared.h"
	#include "util_printout.h"
	#include "cl_dll.h"
	//#include "hud_iface.h"
	#include "r_efx.h"
	#include "ev_hldm.h"
	#include <string.h>
	//#include "basemonster.h"

// from ev_hldm.cpp
	#include "r_efx.h"
	#include "event_api.h"
	#include "event_args.h"

#else
	//SERVER
	#include "extdll.h"
	#include "const.h"
	#include "progdefs.h"
	#include "cbase.h"
	#include "util.h"
	#include "util_shared.h"
	#include "util_printout.h"
	#include "enginecallback.h"
	#include "gamerules.h"

#endif

//SHARED STUFF BELOW
#include "weapons.h"
#include "util_version.h"

#include "cvar_custom_list.h"

//MODDD - includes for file-related things moved to external_lib_include.h
EASY_CVAR_EXTERN_CLIENTONLY(cl_hornetspiral)


// Although PRECACHE_MODEL is valid between client/serverside, it is dummied clientside under the assumption it's unnecessary there.
// However, if it is meant to work for both, it needs to be re-defined for clientside.
// To avoid contradictions with lots of other places that expect PRECACHE_MODEL to still be dummied out clientside, made a new one.
// PRECACHE_MODEL_SHARED, same as PRECACHE_MODEL serverside but clientside uses its proper method.
#ifdef CLIENT_DLL
	// Client?
#define PRECACHE_MODEL_SHARED (*gEngfuncs.pEventAPI->EV_FindModelIndex)
#else
	// Server? same as PRECACHE_MODEL
#define PRECACHE_MODEL_SHARED (*g_engfuncs.pfnPrecacheModel)
#endif



#ifdef CLIENT_DLL
	//from cl_dlls/ev_hldm.cpp, needed for clientside UTIL_Sparks to call.
	extern cl_enginefunc_t gEngfuncs;
#else
	//SERVER
	extern float globalPSEUDO_cl_hornetspiral;
#endif



#ifdef CLIENT_DLL
		//huh. global2PSEUDO_gamePath
#define GET_GAME_PATH_VAR globalPSEUDO_gamePath
#define DEFAULT_GET_GAME_DIR(receiver)\
	const char* tempGameDirRef = gEngfuncs.pfnGetGameDirectory();\
	strcpy(gameName, tempGameDirRef);
#else
#define GET_GAME_PATH_VAR globalPSEUDO_gamePath
#define DEFAULT_GET_GAME_DIR(receiver) GET_GAME_DIR(receiver);
#endif


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// Content.
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

#ifdef CLIENT_DLL
	char globalbuffer_cl_mod_version[128];
	char globalbuffer_cl_mod_date[128];

	char globalbuffer_sv_mod_version_cache[128];
	char globalbuffer_sv_mod_date_cache[128];

	char globalbuffer_cl_mod_display[128];
	char globalbuffer_sv_mod_display[128];
#else
	char globalbuffer_sv_mod_version[128];
	char globalbuffer_sv_mod_date[128];
#endif


extern char GET_GAME_PATH_VAR[];

EASY_CVAR_EXTERN_MASS


// Now for client only (serverside uses a player instance var)
#ifdef CLIENT_DLL
int g_framesSinceRestore = 0;
#endif





DLL_GLOBAL const Vector g_vecZero = Vector(0, 0, 0);


DLL_GLOBAL const Vector VECTOR_CONE_1DEGREES = Vector(0.00873, 0.00873, 0.00873);
DLL_GLOBAL const Vector VECTOR_CONE_2DEGREES = Vector(0.01745, 0.01745, 0.01745);
DLL_GLOBAL const Vector VECTOR_CONE_3DEGREES = Vector(0.02618, 0.02618, 0.02618);
DLL_GLOBAL const Vector VECTOR_CONE_4DEGREES = Vector(0.03490, 0.03490, 0.03490);
DLL_GLOBAL const Vector VECTOR_CONE_5DEGREES = Vector(0.04362, 0.04362, 0.04362);
DLL_GLOBAL const Vector VECTOR_CONE_6DEGREES = Vector(0.05234, 0.05234, 0.05234);
DLL_GLOBAL const Vector VECTOR_CONE_7DEGREES = Vector(0.06105, 0.06105, 0.06105);
DLL_GLOBAL const Vector VECTOR_CONE_8DEGREES = Vector(0.06976, 0.06976, 0.06976);
DLL_GLOBAL const Vector VECTOR_CONE_9DEGREES = Vector(0.07846, 0.07846, 0.07846);
DLL_GLOBAL const Vector VECTOR_CONE_10DEGREES = Vector(0.08716, 0.08716, 0.08716);
DLL_GLOBAL const Vector VECTOR_CONE_15DEGREES = Vector(0.13053, 0.13053, 0.13053);
DLL_GLOBAL const Vector VECTOR_CONE_20DEGREES = Vector(0.17365, 0.17365, 0.17365);



// Used to be only in dlls/weapons.cpp (serverside only file).
// This means the server and client has its own "giAmmoIndex", for use through AddAmmoNameToAmmoRegistry
// calls.  It is good practice to set giAmmoIndex to 0 before potentially making a lot of calls to that
// method, however indirectly (such as before precaching weapons).
int giAmmoIndex;


//MODDD - named ammo indeces so that they don't have to be re-found every single time TabulateAmmo is called
// Globals and set once at startup, since where what ammotype is in the array of ammo types never changes.
int AmmoIndex_9mm;
int AmmoIndex_357;
int AmmoIndex_ARgrenades;
int AmmoIndex_bolts;
int AmmoIndex_buckshot;
int AmmoIndex_rockets;
int AmmoIndex_uranium;
int AmmoIndex_Hornets;
// and some new ones.  Not all ammo types used to have these.
int AmmoIndex_HandGrenade;
int AmmoIndex_SatchelCharge;
int AmmoIndex_Snarks;
int AmmoIndex_TripMine;
int AmmoIndex_ChumToads;



// Also from weapons.cpp, since we can refer to these in both client/server now.
// These are implementations of CBasePlayerItem's two static arrays.
ItemInfo CBasePlayerItem::ItemInfoArray[MAX_WEAPONS];

//MODDD - NEW. More of a complement to ItemInfoArray, but not meant to be set by a weapon's GetItemInfo call.
// Instead, set by AddAmmoNameToAmmoRegistry methods.  They decide ammo type numbers for a given weapon,
// so they may as well store that for future reference per weapon too.
AmmoTypeCache CBasePlayerItem::AmmoTypeCacheArray[MAX_WEAPONS];

AmmoInfo CBasePlayerItem::AmmoInfoArray[MAX_AMMO_TYPES];




// flag to disable extra deploy sounds from weapons spawned by cheats, when set to TRUE before giving
// items and back to FALSE when done. Shared so that the client won't complain that this is missing,
// weapon script is shared. Yes this doesn't get transmitted to the client, but the client doesn't
// seem to do Deploy() calls anyways or doesn't play sounds called through there. In any case, no sync
// is needed as the server (player.cpp) sets this flag before doing some give's, and only the server-
// side deploys are  affected as needed which go on to play the sound for the client of course. It works.
BOOL globalflag_muteDeploySound = FALSE;


DLL_GLOBAL short g_sModelIndexBubbles;// holds the index for the bubbles model


globalvars_t *gpGlobals;


// originally in nodes.cpp
// and wat.  Why is the last value 0.
int Primes[NUMBER_OF_PRIMES] =
{ 1, 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67,
71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151,
157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233, 239,
241, 251, 257, 263, 269, 271, 277, 281, 283, 293, 307, 311, 313, 317, 331, 337,
347, 349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 409, 419, 421, 431, 433,
439, 443, 449, 457, 461, 463, 467, 479, 487, 491, 499, 503, 509, 521, 523, 541,
547, 557, 563, 569, 571, 577, 587, 593, 599, 601, 607, 613, 617, 619, 631, 641,
643, 647, 653, 659, 661, 673, 677, 683, 691, 701, 709, 719, 727, 733, 739, 743,
751, 757, 761, 769, 773, 787, 797, 809, 811, 821, 823, 827, 829, 839, 853, 857,
859, 863, 877, 881, 883, 887, 907, 911, 919, 929, 937, 941, 947, 953, 967, 971,
977, 983, 991, 997, 1009, 1013, 1019, 1021, 1031, 1033, 1039, 0 };





// this never needs to be extern'd ?
// glSeed and seed_table used to be copied between cl_dlls/com_weapons.cpp and dlls/util.cpp.
// (cl_dlls/com_weapons.cpp has since been merged into cl_dlls/hl/hl_weapons.cpp)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static unsigned int glSeed = 0;

unsigned int seed_table[256] =
{
	28985, 27138, 26457, 9451, 17764, 10909, 28790, 8716, 6361, 4853, 17798, 21977, 19643, 20662, 10834, 20103,
	27067, 28634, 18623, 25849, 8576, 26234, 23887, 18228, 32587, 4836, 3306, 1811, 3035, 24559, 18399, 315,
	26766, 907, 24102, 12370, 9674, 2972, 10472, 16492, 22683, 11529, 27968, 30406, 13213, 2319, 23620, 16823,
	10013, 23772, 21567, 1251, 19579, 20313, 18241, 30130, 8402, 20807, 27354, 7169, 21211, 17293, 5410, 19223,
	10255, 22480, 27388, 9946, 15628, 24389, 17308, 2370, 9530, 31683, 25927, 23567, 11694, 26397, 32602, 15031,
	18255, 17582, 1422, 28835, 23607, 12597, 20602, 10138, 5212, 1252, 10074, 23166, 19823, 31667, 5902, 24630,
	18948, 14330, 14950, 8939, 23540, 21311, 22428, 22391, 3583, 29004, 30498, 18714, 4278, 2437, 22430, 3439,
	28313, 23161, 25396, 13471, 19324, 15287, 2563, 18901, 13103, 16867, 9714, 14322, 15197, 26889, 19372, 26241,
	31925, 14640, 11497, 8941, 10056, 6451, 28656, 10737, 13874, 17356, 8281, 25937, 1661, 4850, 7448, 12744,
	21826, 5477, 10167, 16705, 26897, 8839, 30947, 27978, 27283, 24685, 32298, 3525, 12398, 28726, 9475, 10208,
	617, 13467, 22287, 2376, 6097, 26312, 2974, 9114, 21787, 28010, 4725, 15387, 3274, 10762, 31695, 17320,
	18324, 12441, 16801, 27376, 22464, 7500, 5666, 18144, 15314, 31914, 31627, 6495, 5226, 31203, 2331, 4668,
	12650, 18275, 351, 7268, 31319, 30119, 7600, 2905, 13826, 11343, 13053, 15583, 30055, 31093, 5067, 761,
	9685, 11070, 21369, 27155, 3663, 26542, 20169, 12161, 15411, 30401, 7580, 31784, 8985, 29367, 20989, 14203,
	29694, 21167, 10337, 1706, 28578, 887, 3373, 19477, 14382, 675, 7033, 15111, 26138, 12252, 30996, 21409,
	25678, 18555, 13256, 23316, 22407, 16727, 991, 9236, 5373, 29402, 6117, 15241, 27715, 19291, 19888, 19847
};


// When any hidden CVars are altered, don't save immediately, turn this flag on.
// At the end of the frame or an update cycle (runs every second), all hidden CVars will be saved to
// the usual file.  This way, editing multiple CVars while the window is open won't save multiple times,
// only once very soon after.
BOOL g_queueCVarHiddenSave = FALSE;





/*
=====================
UTIL_WeaponTimeBase

Always 0.0 on client, even if not predicting weapons ( won't get called
 in that case )
=====================
*/
float UTIL_WeaponTimeBase(void)
{
#if CLIENT_DLL
	// CLIENT
	return 0.0;
#else
	// SERVER
	#if defined( CLIENT_WEAPONS )
		return 0.0;
	#else
		return gpGlobals->time;
	#endif
#endif
}

// These were never called outside of their own .cpp files.  Helper methods.
unsigned int U_Random(void)
{
	glSeed *= 69069;
	glSeed += seed_table[glSeed & 0xff];

	return (++glSeed & 0x0fffffff);
}

void U_Srand(unsigned int seed)
{
	glSeed = seed_table[seed & 0xff];
}

/*
=====================
UTIL_SharedRandomLong
=====================
*/
int UTIL_SharedRandomLong(unsigned int seed, int low, int high)
{
	unsigned int range;

	U_Srand((int)seed + low + high);

	range = high - low + 1;
	if (!(range - 1))
	{
		return low;
	}
	else
	{
		int offset;
		int rnum;

		rnum = U_Random();

		offset = rnum % range;

		return (low + offset);
	}
}

/*
=====================
UTIL_SharedRandomFloat
=====================
*/
float UTIL_SharedRandomFloat(unsigned int seed, float low, float high)
{
	unsigned int range;

	U_Srand((int)seed + *(int*)&low + *(int*)&high);

	U_Random();
	U_Random();

	range = high - low;
	if (!range)
	{
		return low;
	}
	else
	{
		int tensixrand;
		float offset;

		tensixrand = U_Random() & 65535;

		offset = (float)tensixrand / 65536.0;

		return (low + offset * range);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// global method copied from weapons.cpp, now supported clientside too
BOOL CanAttack(float attack_time, float curtime, BOOL isPredicted)
{

#ifdef CLIENT_DLL
	// clientside
	return (attack_time <= 0) ? TRUE : FALSE;

#else
	// serverside
#if defined( CLIENT_WEAPONS )
	if (!isPredicted)
#else
	if (1)
#endif
	{
		return (attack_time <= curtime) ? TRUE : FALSE;
	}
	else
	{
		return (attack_time <= 0.0) ? TRUE : FALSE;
	}

	//END OF normal behavior w/o the cheat.
	//}else{
	//	//with cheats, can always attack.
	//	return TRUE;
	//}
#endif
}//CanAttack




//MODDD - w-...what?  Why was this a CBasePlayer method at all?  Just be a global shared utility then.
// It depends on nothing about any spawned or blank player data.
int GetAmmoIndex(const char* psz)
{
	int i;

	if (!psz)
		return -1;


	for (i = 1; i < MAX_AMMO_TYPES; i++)
	{

		if (!CBasePlayerItem::AmmoInfoArray[i].pszName)
			continue;

		if (stricmp(psz, CBasePlayerItem::AmmoInfoArray[i].pszName) == 0)
			return i;
	}

	return -1;
}


//...oh.  This was only ever referred to in weapons.cpp.  Well, whoops. Doesn't hurt to be here (shared) I suppose.
// Although note that a weapon's 'm_iId' can be used to access ItemInfoArray[] if there is some wepaon in mind.
// Come to think of it it seems odd that max ammo counts are stored in weapons (ItemInfoArray) instead of Ammo
// (AmmoInfoArray).   So the glock and mp5 just get copies of the same maximum when it should've been attached to
// the same ammo entry.     ah well.
// In finding a maximum for 9mm ammo, it likely awkwardly finds the glock first in the ItemInfoArray list and 
// reports its maxAmmo from being one weapon with the correct 'psz' string.  YECH.
// Also, takes the name as a 'const char*' instead of an int meant to be converted to a string.
//=========================================================
// MaxAmmoCarry - pass in a name and this function will tell
// you the maximum amount of that type of ammunition that a 
// player can carry.
//=========================================================
int MaxAmmoCarry(const char* psz)
{
	for (int i = 0; i < MAX_WEAPONS; i++)
	{
		if (CBasePlayerItem::ItemInfoArray[i].pszAmmo1 && !strcmp(psz, CBasePlayerItem::ItemInfoArray[i].pszAmmo1)) {
			return CBasePlayerItem::ItemInfoArray[i].iMaxAmmo1;
		}
		if (CBasePlayerItem::ItemInfoArray[i].pszAmmo2 && !strcmp(psz, CBasePlayerItem::ItemInfoArray[i].pszAmmo2)) {
			return CBasePlayerItem::ItemInfoArray[i].iMaxAmmo2;
		}
	}

	ALERT(at_console, "MaxAmmoCarry() doesn't recognize '%s'!\n", psz);
	return -1;
}


/*
// ... in fact, alternate version that takes a weapon ID instead.
// SCRAPPED, only the weaponbox uses MaxAmmoCarry.  Not worth supporting when nothing else would use this.
// Picked-up weapons for ammo and ammo pickups already use the item's ItemInfoArray[m_iId].iMaxAmmo# values,
// see weapon.h "iMaxAmmo#()" methods that do this (most common, if not only way used).
int MaxAmmoCarry_ItemID(int iID)
{
	// I... guess an ID of 0 is valid technically but odd.
	// Nah disallowed.  Represents WEAPON_NONE, weapons.h
	if (iID > 0 && iID < MAX_WEAPONS) {
		// ok
	}
	else {
		// oh.
		ALERT(at_console, "MaxAmmoCarry() iID out of bounds: '%d'!\n", iID);
		return -1;
	}

	const ItemInfo& theItem = CBasePlayerItem::ItemInfoArray[iID];

	// then to choose what maxAmmo to return.  Different variants for returning each would be good then.
	return 
	theItem.iMaxAmmo1;
	theItem.iMaxAmmo2;
}
*/




// Precaches the ammo and queues the ammo info for sending to clients
//MODDD - Accepts weapon taking the ammos for saving to CBasePlayerItem::AmmoTypeCacheArray[#].
// Also, variants created for primary/secondary ammo to know which one to use in AmmoTypeCacheArray[#].
// And now accepts max-ammo for that type of ammo too, although nothing refers to type through
// AmmoInfoArray yet.
void AddAmmoNameToAmmoRegistry_Primary(int iId, const char* szAmmoname, int iAmmoMax)
{
	// make sure it's not already in the registry
	for (int i = 0; i < MAX_AMMO_TYPES; i++)
	{
		if (!CBasePlayerItem::AmmoInfoArray[i].pszName) {
			continue;
		}

		if (stricmp(CBasePlayerItem::AmmoInfoArray[i].pszName, szAmmoname) == 0) {
			//MODDD - still, save this number as the ammo type for this weapon.
			CBasePlayerItem::AmmoTypeCacheArray[iId].iPrimaryAmmoType = i;
			return; // ammo already in registry, just quit
		}
	}

	//MODDD - Note that incrementing this early means index #0 is forever unused.
	// Although perpaps this was intentional.  If 0 is a valid space and the data sent over
	// is unsigned, it's more awkward to check for some high number just because -1 isn't an option.
	// Just leave it.
	// Another thing to consider.  Save-restore expects these to be the same since last time as it
	// always happens at game startup, so be prepared for lots of crying.
	giAmmoIndex++;

	ASSERT(giAmmoIndex < MAX_AMMO_TYPES);
	if (giAmmoIndex >= MAX_AMMO_TYPES) {
		easyForcePrintLine("ERROR!!! Too many ammo types, overflowed into #0!");
		giAmmoIndex = 0;
	}

	CBasePlayerItem::AmmoInfoArray[giAmmoIndex].pszName = szAmmoname;
	CBasePlayerItem::AmmoInfoArray[giAmmoIndex].iId = giAmmoIndex;   // yes, this info is redundant
	CBasePlayerItem::AmmoInfoArray[giAmmoIndex].iAmmoMax = iAmmoMax;

	//MODDD - and save as the ammo type for this weapon.
	CBasePlayerItem::AmmoTypeCacheArray[iId].iPrimaryAmmoType = giAmmoIndex;

	//giAmmoIndex++;
}//END OF AddAmmoNameToAmmoRegistry_Primary

void AddAmmoNameToAmmoRegistry_Secondary(int iId, const char* szAmmoname, int iAmmoMax)
{
	// make sure it's not already in the registry
	for (int i = 0; i < MAX_AMMO_TYPES; i++)
	{
		if (!CBasePlayerItem::AmmoInfoArray[i].pszName) {
			continue;
		}

		if (stricmp(CBasePlayerItem::AmmoInfoArray[i].pszName, szAmmoname) == 0) {
			CBasePlayerItem::AmmoTypeCacheArray[iId].iSecondaryAmmoType = i;
			return; // ammo already in registry, just quit
		}
	}

	giAmmoIndex++;

	ASSERT(giAmmoIndex < MAX_AMMO_TYPES);
	if (giAmmoIndex >= MAX_AMMO_TYPES) {
		easyForcePrintLine("ERROR!!! Too many ammo types, overflowed into #0!");
		giAmmoIndex = 0;
	}

	CBasePlayerItem::AmmoInfoArray[giAmmoIndex].pszName = szAmmoname;
	CBasePlayerItem::AmmoInfoArray[giAmmoIndex].iId = giAmmoIndex;   // yes, this info is redundant
	CBasePlayerItem::AmmoInfoArray[giAmmoIndex].iAmmoMax = iAmmoMax;

	//MODDD
	CBasePlayerItem::AmmoTypeCacheArray[iId].iSecondaryAmmoType = giAmmoIndex;

	//giAmmoIndex++;
}//END OF AddAmmoNameToAmmoRegistry_Secondary





// Call this on each weapon in the game clientside and serverside to get some basic info about each
// weapon cached, especially the ammo-types.
void RegisterWeapon(CBasePlayerWeapon* pWeapon, CBasePlayerWeapon* pAryWeaponStore[]) {
	ItemInfo tempInfo;
	//easyForcePrintLine("HELP HUD_PrepEntity!!! %s %d %s", pEntity->getClassname(), tempInfo.iId, tempInfo.pszAmmo1);

	//MODDD - unsure why lots of places do this to their ItemInfo instances, but may as well here too.
	memset(&tempInfo, 0, sizeof tempInfo);

	//MODDD - added a bit to fill CBasePlayerItem::ItemInfoArray with the
	// "ItemInfo" instance ("tempInfo" here), similar to UTIL_PrecacheOtherWeapon 
	// of dlls/util.cpp
	if (pWeapon->GetItemInfo(&tempInfo)) {

		//TAGGG - CRITICAL CRITICAL CRITICAL.
		// This is what sets something in
		// ItemInfoArray.  Any other mentions of "GetItemInfo" are often forgotten,
		// so changes to some stat on a whim (like glock max ammo on seeing a change
		// in CVar "glockOldReloadLogic") can change ItemInfoArray right there.
		// Also, this is a copy-by-value.  Important since this same "II" instance is 
		// re-used for getting all the weapon info.
		CBasePlayerItem::ItemInfoArray[tempInfo.iId] = tempInfo;
		
		// This line used to be here, but wasn't bounded by the new "if(...)" check above.
		// Seems fitting, otherwise even 'tempInfo.iId' is garbage data anyway.
		// ...since this has been shared since, this only runs if the given 'pAryWeaponStore'
		// (presumably g_pWpns) is not NULL.  Clientside gives it, serverside does not.
		//g_pWpns[tempInfo.iId] = tempWeaponRef;
		if (pAryWeaponStore != NULL){
			pAryWeaponStore[tempInfo.iId] = pWeapon;
		}

		//MODDD - and if there isn't an ammo-type, tell the ammo type cache this.
		if (tempInfo.pszAmmo1 && *tempInfo.pszAmmo1){
			AddAmmoNameToAmmoRegistry_Primary(tempInfo.iId, tempInfo.pszAmmo1, tempInfo.iMaxAmmo1);
		}else {
			CBasePlayerItem::AmmoTypeCacheArray[tempInfo.iId].iPrimaryAmmoType = -1;
		}

		if (tempInfo.pszAmmo2 && *tempInfo.pszAmmo2){
			AddAmmoNameToAmmoRegistry_Secondary(tempInfo.iId, tempInfo.pszAmmo2, tempInfo.iMaxAmmo2);
		}else {
			CBasePlayerItem::AmmoTypeCacheArray[tempInfo.iId].iSecondaryAmmoType = -1;
		}

		// this memset is unnecessary, the method ends after this.
		//memset(&tempInfo, 0, sizeof tempInfo);
	}//END OF GetItemInfo pass check

}//registerWeapon


//MODDD - After 'registerWeapon' has been called for all weapons, all possible ammo-types have also been registered.
// Use them to fill the cached ammo indeces.
void PostWeaponRegistry(void) {

	// No references to snark/squeak ammo in there, eh?  Interesting.  Seems these really aren't that necessary.
	// Don't see what these do that a weapon's own PrimaryAmmoIndex() method can't do.
	// Could replace calls for ammo-type by string name throughout other files though.

	AmmoIndex_9mm = GetAmmoIndex("9mm");
	AmmoIndex_357 = GetAmmoIndex("357");
	AmmoIndex_ARgrenades = GetAmmoIndex("ARgrenades");
	AmmoIndex_bolts = GetAmmoIndex("bolts");
	AmmoIndex_buckshot = GetAmmoIndex("buckshot");
	AmmoIndex_rockets = GetAmmoIndex("rockets");
	AmmoIndex_uranium = GetAmmoIndex("uranium");
	AmmoIndex_Hornets = GetAmmoIndex("Hornets");
	// NEW. Here they are if ever needed anyway.
	AmmoIndex_HandGrenade = GetAmmoIndex("Hand Grenade");
	AmmoIndex_SatchelCharge = GetAmmoIndex("Satchel Charge");
	AmmoIndex_Snarks = GetAmmoIndex("Snarks");
	AmmoIndex_TripMine = GetAmmoIndex("Trip Mine");
	AmmoIndex_ChumToads = GetAmmoIndex("Chum Toads");

}//PostWeaponRegistry



	
//MODDD
const char* FClassname(CBaseEntity* derp){
	if(derp != NULL){
		return STRING(derp->pev->classname);
	}else{
		return "NULL";
	}
}


BOOL stringStartsWith(const char* source, const char* startswith){

	for(int i = 0; i < 127; i++){
		if(startswith[i] == '\0'){
			//if we made it this far (end of "startswith"), something went right.
			return TRUE;
		}
		if(source[i] == '\0'){
			//ending prematurely, source is less than startsWith and followed up to this point.
			return FALSE;
		}

		if(source[i] != startswith[i]){
			return FALSE;
		}
	}
	return TRUE;
}


BOOL checkMatch(const char* src1, char* src2){
	return checkMatch(src1, src2, 127);

}

BOOL checkMatch(const char* src1, char* src2, int size){
	BOOL matching = TRUE;
	int i;
	for(i = 0; i < size; i++){
		if(src1[i] != src2[i]){
			//no match? we're done.
			matching = FALSE;
			break;
		}
		if(src1[i] == '\0'){
			//since they are equal and either is the termination, finish gracefully.
			break;
		}
	}
	return matching;
}


BOOL checkMatchIgnoreCase(const char* src1, char* src2){
	return checkMatchIgnoreCase(src1, src2, 127);

}

BOOL checkMatchIgnoreCase(const char* src1, char* src2, int size){
	BOOL matching = TRUE;
	int i;
	for(i = 0; i < size; i++){

		if(src1[i] == src2[i]){


		}else if(src1[i] >= 'A' && src1[i] <= 'Z'){
			//try again with the lowercase char:
			char lowerChar = ((char)  (((int)(src1[i])) + 32) );
			
			if(lowerChar == src2[i]){
				//okay.
			}else{
				matching = FALSE;
				break;
			}
		}else{
			matching = FALSE;
			break;
		}

		if(src1[i] == '\0'){
			//since they are equal and either is the termination, finish gracefully.
			break;
		}
	}

	return matching;
}



const char* tryIntToString(int arg_src){

	//WARNING - they should provide a buffer instead.
	static char chrReturn[13];
	itoa(arg_src, chrReturn, 10);
	return chrReturn;
}


int tryStringToInt(const char* arg_src){
	int i = 0;

	if(arg_src == NULL){
		//can't do anything.
		throw 13;
	}

	while(i < 128){

		if(arg_src[i] == '\0'){

			if(i > 0){
				char tempChar[128];
				strncpy( &tempChar[0], &arg_src[0], i );
				tempChar[i] = '\0';
				int result = atoi(tempChar);

				return result;

			}else{
				//the very first character is the termination '\0' ?  Error.
				throw 13;
			}
		}

		if(arg_src[i] >= '0' && arg_src[i] <= '9'){
			//pass.
		}else if(i == 0 && arg_src[i] == '-'){
			//allow the minus sign, if it is the first character.
		}else{
			//invalid character!
			throw 13;
		}

		i++;

	}//END OF while(...)

	//loop above ended (did not return or throw)?  Too long!
	throw 13;

}


float tryStringToFloat(const char* arg_src){
	int i = 0;

	if(arg_src == NULL){
		//can't do anything.
		throw 13;
	}


	int decimalCount = 0;

	while(i < 128){

		if(arg_src[i] == '\0'){

			if(i > 0){
				char tempChar[128];
				strncpy( &tempChar[0], &arg_src[0], i );
				tempChar[i] = '\0';
				float result = (float)atof(tempChar);

				return result;

			}else{
				//the very first character is the termination '\0' ?  Error.
				throw 13;
			}
		}

		if(arg_src[i] >= '0' && arg_src[i] <= '9'){
			//pass.

		}else if(arg_src[i] == '.'){

			if(decimalCount > 0){
				//FAIL!
				throw 13;
			}
			decimalCount++;

		}else if(i == 0 && arg_src[i] == '-'){
			//allow the minus sign, if it is the first character.
		}else{
			//invalid character!
			throw 13;
		}

		i++;

	}//END OF while(...)

	//loop above ended (did not return or throw)?  Too long!
	throw 13;
}


const char* tryFloatToString(float arg_src){
	
	//WARNING - they should provide a buffer instead.
	static char chrReturn[13] ;
	//itoa(arg_src, chrReturn, 10);

	sprintf(chrReturn, "%f", 3.0f);

	return chrReturn;
}

void tryFloatToStringBuffer(char* dest, float arg_src){
	
	//itoa(arg_src, chrReturn, 10);
	sprintf(dest, "%f", arg_src);
}



//thanks, 
//http://stackoverflow.com/questions/11656532/returning-an-array-using-c


void lowercase(char* src){
	// assume length of 128.
	// Can end anytime the terminating character is found.
	for(int i = 0; i < 127; i++){
		// in other words, this character of the sent text is between "A" and "Z" (capital).
		if(src[i] >= 65 && src[i] <= 90){
			// adding 32 to the character makes it lowercased, since the range of lowercased chars is 97 'a' to 122 'z'.
			src[i] = (char)((int)src[i] + 32);
			//src[i] = src[i];
			//src[i] = src[i];
		}else{
			// not a letter.  Just leave it.
			src[i] = src[i];
		}
		if(src[i] == '\0'){
			//end.
			break;
			//return &src[0];
		}
	}
	src[127] = '\0';
	//return &src[0];
}


void lowercase(char* src, int size){
	BOOL broke = FALSE;
	// Yes, check all at size-1 because the very last spot [size-1] itself 
	// should be a terminating character if all the size is used.
	// Ending early at finding the terminating character first is possible
	// of course.
	for(int i = 0; i < size-1; i++){
		if(src[i] >= 65 && src[i] <= 90){
			src[i] = (char)((int)src[i] + 32);
		}else{
			src[i] = src[i];
		}
		if(src[i] == '\0'){
			broke = TRUE;
			break;
		}
	}
	if(!broke){
		src[size-1] = '\0';
	}
}



//Thanks!
//https://stackoverflow.com/questions/554204/where-is-round-in-c
float roundToNearest(float num) {
    return (num > 0.0f) ? floor(num + 0.5f) : ceil(num - 0.5f);
}


//NOTE: non-inclusive end index.
void UTIL_substring(char* dest, const char* src, int startIndex, int endIndex){

	int i = 0;

	if(endIndex == -1){
		while(true){
			const char& thisChar = src[i + startIndex];
			if(thisChar == '\0'){
				//done.
				break;
			}else{
				dest[i] = thisChar;
				i++;
			}
		}
	}else{
		while(i+startIndex < endIndex){
			dest[i] = src[i + startIndex];
			i++;
		}

	}
	//finally, wherever "i" left off.
	dest[i] = '\0';
}


int UTIL_findCharFirstPos(const char* search, char toFind){
	int i = 0;
	while(TRUE){
		if(search[i] == '\0'){
			//give up.
			return -1;
		}else if(search[i] == toFind){
			return i;
		}
		i++;
	}//END OF while(TRUE)
	//???
	return -1;
}

void UTIL_appendTo(char* dest, const char* add, int appendStartLoc){
	int lengthOfToAdd = lengthOfString(add);
	strncpy( &dest[appendStartLoc], add, lengthOfToAdd );

}

void appendTo(char* dest, const char* add, int* refIndex){
	int lengthOfToAdd = lengthOfString(add);
	strncpy( &dest[*refIndex], add, lengthOfToAdd );

	//probably implied.  NOPE?!
	//dest[*refIndex + lengthOfToAdd] = '\0';

	*refIndex += (lengthOfToAdd);
}

void appendToAndTerminate(char* dest, const char* add, int* refIndex){
	int lengthOfToAdd = lengthOfString(add);
	strncpy( &dest[*refIndex], add, lengthOfToAdd );

	//probably implied.  NOPE?!
	dest[*refIndex + lengthOfToAdd] = '\0';

	*refIndex += (lengthOfToAdd);
}


void appendTo(char* dest, const char* add, int* refIndex, char endCharacter){
	int lengthOfToAdd = lengthOfString(add);
	strncpy( &dest[*refIndex], add, lengthOfToAdd );

	dest[*refIndex + lengthOfToAdd] = endCharacter;

	*refIndex += (lengthOfToAdd + 1);
}

void appendToAndTerminate(char* dest, const char* add, int* refIndex, char endCharacter){
	int lengthOfToAdd = lengthOfString(add);
	strncpy( &dest[*refIndex], add, lengthOfToAdd );

	dest[*refIndex + lengthOfToAdd] = endCharacter;
	dest[*refIndex + lengthOfToAdd + 1] = '\0';

	*refIndex += (lengthOfToAdd + 1);
}

void appendTo(char* dest, const int numb, int* refIndex){
	
	char numbChar[4];
	itoa(numb, numbChar, 10);
	appendTo(dest, numbChar, refIndex);

}

void appendToAndTerminate(char* dest, const int numb, int* refIndex){
	
	char numbChar[4];
	itoa(numb, numbChar, 10);
	appendToAndTerminate(dest, numbChar, refIndex);

}


void appendTo(char* dest, const int numb, int* refIndex, char endCharacter){
	
	char numbChar[4];
	itoa(numb, numbChar, 10);
	appendTo(dest, numbChar, refIndex, endCharacter);

}

void appendToAndTerminate(char* dest, const int numb, int* refIndex, char endCharacter){
	
	char numbChar[4];
	itoa(numb, numbChar, 10);
	appendToAndTerminate(dest, numbChar, refIndex, endCharacter);

}



void appendTo(char* dest, const char add, int* refIndex){
	dest[*refIndex] = add;

	*refIndex += (1);
}

void appendToAndTerminate(char* dest, const char add, int* refIndex){
	dest[*refIndex] = add;
	dest[*refIndex + 1] = '\0';

	*refIndex += (1);
}


void strncpyTerminate(char* dest, const char* send, int arg_length){
	strncpy(dest, send, arg_length);
	dest[arg_length] = '\0';
}








void copyString(const char* src, char* dest){
	//no size given?  "127" is the default.
	copyString(src, dest, 127);
}



void copyString(const char* src, char* dest, int size){

	// strncpy takes a size, but copies 0's all the way to the end.
	// Really unnecessary.  A bounded strcpy that stops at the max if the source exceeds the given size without doing anything
	// extra if the source is under the size (runs into a '\0' char, end there) would be nice, but oh well.
	strcpy(dest, src);

	/*
	//MODDD - why when strcpy does it just fine?
	BOOL queueBreak = FALSE;
	int i;
	for(i = 0; i < size; i++){
		
		if(src[i] == '\0'){
			//we will be done.
			queueBreak = TRUE;
		}
		dest[i] = src[i];

		if(queueBreak){
			break;
		}
	}

	//all the way to the end, never "terminated"?
	if(queueBreak == FALSE){
		dest[size-1] = '\0';
		//last char will have to clip and say '\0' to terminate.
	}
	*/

}


void UTIL_appendToEnd(char* dest, const char* add){

	int lengthOfDest = lengthOfString(dest);
	int lengthOfToAdd = lengthOfString(add);

	//aaaabb
	//0123456
	//|a| = 4
	//|b| = 2

	//easyForcePrintLine("LENGTH TO ADD: %s :::%d", add, lengthOfToAdd);
	strncpy( &dest[lengthOfDest], add, lengthOfToAdd );
	
	//OK, c++... I had more faith in you than that.
	dest[lengthOfDest + lengthOfToAdd] = '\0';
}


int lengthOfString(const char* src){
	int i = 0;

	while(i < 500){
		if(src[i] == '\0'){
			//this is length, done.
			return i;
		}else{
			//something that isn't the end? continue.
		}

		i++;
	}
	//ERROR CODE.  Either never ended or too long.
	return -1;
}


int lengthOfString(const char* src, int storeSize){
	//note that "storeSize" is the (optional) size of the "src" array we're printing to.
	//If we try to surpass this, cut off the string at the cap index of
	//cap = storeSize - 2;
	//why?  because "storeSize - 1", the last legal index for characters, needs to be '\0', the null terminating character.
	//this leaves storeSize - 2 as the last legal index for normal (non-terminating?) characters.

	int i = 0;

	while(i <= storeSize - 2){
		if(src[i] == '\0'){
			//this is length, done.
			return i;
		}else{
			//something that isn't the end? continue.
		}

		i++;
	}
	
	//hm, error code?  hard to say.  Cutoff can happen though.
	//not editing!
	//src[i] = '\0'; 

	return i;
}


BOOL isStringEmpty(const char* arg_src){
	if(arg_src == NULL || arg_src[0] == '\0'){
		//empty!
		return TRUE;
	}else{
		return FALSE;
	}
}


BOOL stringEndsWith(const char* arg_src, const char* arg_endsWith){
	int i = 0;
	int lastPos;
	
	int srcLength = lengthOfString(arg_src);
	int endsWithLength = lengthOfString(arg_endsWith);

	if(srcLength < endsWithLength){
		//can't possibly end with something longer than itself (arg_src shorter than what it ends with)
		return FALSE;
	}

	//start place to compare arg_src to.
	int srcIndex = srcLength - endsWithLength;

	for(i = 0; i < endsWithLength; i++){
		if(arg_src[srcIndex + i] == arg_endsWith[i]){
			//pass...
		}else{
			//No match!
			return FALSE;
		}
	}
	//made it past the loop?  Then we're done.
	return TRUE;
}

BOOL stringEndsWithIgnoreCase(const char* arg_src, const char* arg_endsWith){
	int i = 0;
	int lastPos;
	
	int srcLength = lengthOfString(arg_src);
	int endsWithLength = lengthOfString(arg_endsWith);

	if(srcLength < endsWithLength){
		//can't possibly end with something longer than itself (arg_src shorter than what it ends with)
		return FALSE;
	}

	//derpyherp
	//herp

	//start place to compare arg_src to.
	int srcIndex = srcLength - endsWithLength;

	for( i = 0; i < endsWithLength; i++){



		if(arg_src[srcIndex + i] == arg_endsWith[i]){
			//pass...
			//capital check.  Characters may be implicitly cast to integers (as values on the ASCII table)
			//~anything between corresponding #'s for 'A' and 'Z' is capital, of course.
		}else if(arg_src[srcIndex + i] >= 'A' && arg_src[srcIndex + i] <= 'Z'){
			
			//let's try making the source character lowercase, compare again:

			//add 32 to a capitol ASCII value (range is 65 = A, 90 = Z) to change to range 97 = a, 122 = z.
			char lowerChar = ((char)  (((int)(arg_src[srcIndex + i])) + 32) );
			//easyPrintLine("lowercase check? %d %d %d", arg_src[srcIndex + i], lowerChar, arg_endsWith[i]);
			BOOL passed = (lowerChar == arg_endsWith[i]);

			if(passed){
				//continue.
			}else{
				//even lowercase'd, we still failed.
				//easyPrintLine("awnaw!!: %d %d", arg_src[srcIndex + i], arg_endsWith[i]);
				//easyPrintLine("craaap!!: %d %d", lowerChar, arg_endsWith[i]);
			
				return FALSE;
			}

		}else{
			//No match!
			//easyPrintLine("nomatch: %d %d", arg_src[srcIndex + i], arg_endsWith[i]);
			return FALSE;
		}
	}
	//made it past the loop?  Then we're done.
	return TRUE;
}








//check the existence of this subpath under the game's directory.
//That is, arg_subdir (let's say X ) in here:
//  C:/.../<half life folder>/<mod name>/X
BOOL checkSubFileExistence(const char* arg_subdir){
	if(GET_GAME_PATH_VAR[0] == '\0'){
		return FALSE;
	}

	char tempCheck[MAX_PATH];
	tempCheck[0] = '\0';

	copyString(GET_GAME_PATH_VAR, tempCheck);

	/*
#ifdef CLIENT_DLL
	easyForcePrintLine("CLIENT: checkSubFileExistence call. Gamepath:|%s| subdir:|%s|", GET_GAME_PATH_VAR, arg_subdir);
#else
	easyForcePrintLine("SERVER: checkSubFileExistence call. Gamepath:|%s| subdir:|%s|", GET_GAME_PATH_VAR, arg_subdir);
#endif
	*/

	/*
	for(int i = 0; i < 500; i++){
		easyForcePrintLine("%d: %i:%c, %i:%c", i, globalPSEUDO_gamePath[i], globalPSEUDO_gamePath[i], tempCheck[i], tempCheck[i]);

		BOOL doBreak = FALSE;
		if(globalPSEUDO_gamePath[i] == '\0'){
			easyForcePrintLine("GP FAIL");
			doBreak = TRUE;
		}
		if(tempCheck[i] == '\0'){
			easyForcePrintLine("TC FAIL");
			doBreak = TRUE;
		}
		if(doBreak)break;
	}
	*/



	UTIL_appendToEnd(tempCheck, arg_subdir);

	/*
	for(int i = 0; i < 500; i++){
		easyForcePrintLine("-%d: %i:%c", i, arg_subdir[i], arg_subdir[i]);
		if(arg_subdir[i] == '\0'){
			break;
		}
	}

	BOOL gpBrokenYet = FALSE;

	for(int i = 0; i < 500; i++){

		
		if(globalPSEUDO_gamePath[i] == '\0'){
			gpBrokenYet = TRUE;
		}

		
		if(gpBrokenYet == FALSE){
			easyForcePrintLine("%d: %i:%c, %i:%c", i, globalPSEUDO_gamePath[i], globalPSEUDO_gamePath[i], tempCheck[i], tempCheck[i]);
		}else{
			easyForcePrintLine("%d: %i:%c, %i:%c", i, 'e', 'e', tempCheck[i], tempCheck[i]);
		}
		BOOL doBreak = FALSE;
		if(tempCheck[i] == '\0'){
			easyForcePrintLine("TC FAIL");
			doBreak = TRUE;
		}
		if(doBreak)break;
	}
	*/


	/*
#ifdef CLIENT_DLL
	easyForcePrintLine("CLIENT: checkSubFileExistence result: |%s|", tempCheck);
#else
	easyForcePrintLine("SERVER: checkSubFileExistence result: |%s|", tempCheck);
#endif
	*/


	//std::string lineRaw;
	FILE* myFile = fopen(tempCheck, "r" );

	//easyForcePrintLine("OOOOOOOOO %s", tempCheck);

	if (myFile)
	{
		fclose(myFile);
		//"memez" exists?  assume it has the files.
		return TRUE;
	}else{
		//myfile.close();   //???

		//no memes.
		return FALSE;
	}
}





BOOL globalPSEUDO_iCanHazMemez = FALSE;

char globalPSEUDO_halflifePath[512];
char globalPSEUDO_gamePath[512];
char globalPSEUDO_valveGamePath[512];
char globalPSEUDO_hiddenMemPath[512];



void determineHiddenMemPath(void){

	globalPSEUDO_halflifePath[0] = '\0';
	globalPSEUDO_gamePath[0] = '\0';
	globalPSEUDO_valveGamePath[0] = '\0';
	globalPSEUDO_hiddenMemPath[0] = '\0';

	//WCHAR;
	//LPSTR buffer;
	//char buffer[MAX_PATH];

	//Seems to work fine for getting an absolute path to the hl.exe file, but this is unnecessary.
	/*
    GetModuleFileName(NULL, globalPSEUDO_halflifePath, MAX_PATH) ;
	
	//easyForcePrintLine("Executable Path: %s", globalPSEUDO_halflifePath);

	//we expect this to be a path straight to the EXE, so cut off at the last slash.

	int recentSlashPos = -1;
	int i = 0;
	while(i < MAX_PATH){
		if(globalPSEUDO_halflifePath[i] == '\0'){
			break;
		}else if(globalPSEUDO_halflifePath[i] == '\\' || globalPSEUDO_halflifePath[i] == '/'){
			recentSlashPos = i;
		}
		i++;
	}
	
	//the "hl.exe" in here is unhelpful, cut it (substring, cut off from the last slash onwards)
	//UTIL_substring(globalPSEUDO_halflifePath, 0, recentSlashPos + 1);
	globalPSEUDO_halflifePath[recentSlashPos + 1] = '\0';  //termination... same effect.

	//what we have is "globalPSEUDO_halflifePath" = "C:\...\<half life folder>\"
	*/

	char gameName[100];
	gameName[0] = '\0';
	DEFAULT_GET_GAME_DIR(gameName);
	UTIL_appendToEnd(gameName, "\\");
	// "<modname>\" or "valve\" for vanilla
	
	char valveGameName[100] = "valve\\\0";


	copyString(globalPSEUDO_halflifePath, globalPSEUDO_gamePath);
	UTIL_appendToEnd(globalPSEUDO_gamePath, gameName);
	copyString(globalPSEUDO_halflifePath, globalPSEUDO_valveGamePath);
	UTIL_appendToEnd(globalPSEUDO_valveGamePath, valveGameName);

	copyString(globalPSEUDO_gamePath, globalPSEUDO_hiddenMemPath);

#ifdef CLIENT_DLL
	UTIL_appendToEnd(globalPSEUDO_hiddenMemPath, "absZeroMemClient.txt"); //recentSlashPos + 1);
#else
	UTIL_appendToEnd(globalPSEUDO_hiddenMemPath, "absZeroMem.txt"); //recentSlashPos + 1);
#endif
	
		
	/*
#ifdef CLIENT_DLL
	easyForcePrintLine("CLIENT: helpme.txt SEARCH?" );
#else
	easyForcePrintLine("SERVER: helpme.txt SEARCH?" );
#endif
	*/

	
	if(checkSubFileExistence("helpme.txt")){
		EASY_CVAR_SET_DEBUGONLY(hiddenMemPrintout, 1);
		
#ifdef CLIENT_DLL
			easyForcePrintLine("CLIENT: helpme.txt found - hiddenMemPrintout enabled!" );
#else
			easyForcePrintLine("SERVER: helpme.txt found - hiddenMemPrintout enabled!" );
#endif
			
	}else{
		//???
	}

	if(EASY_CVAR_GET_DEBUGONLY(hiddenMemPrintout) == 1){
		easyForcePrintLine("HALF LIFE PATH: %s", globalPSEUDO_halflifePath);
		easyForcePrintLine("GAME PATH: %s", globalPSEUDO_gamePath);
		easyForcePrintLine("HIDDEN MEM PATH: %s", globalPSEUDO_hiddenMemPath);
	}
	
}//END OF determineHiddenPath



BOOL checkValveSubFileExistence(const char* arg_subdir){
	if(globalPSEUDO_valveGamePath[0] == '\0'){
		return FALSE;
	}
	char tempCheck[MAX_PATH];
	tempCheck[0] = '\0';

	copyString(globalPSEUDO_valveGamePath, tempCheck);
	
	UTIL_appendToEnd(tempCheck, arg_subdir);

	//std::string lineRaw;
	FILE* myFile = fopen(tempCheck, "r" );

	//easyForcePrintLine("OOOOOOOOO %s", tempCheck);

	if (myFile)
	{
		fclose(myFile);
		return TRUE;
	}else{
		return FALSE;
	}
}



//check the existence of this subpath under the game's directory.
//That is, arg_subdir (let's say X ) in here:
//  C:/.../<half life folder>/<mod name>/X
BOOL checkSubDirectoryExistence(const char* arg_subdir){
	if(globalPSEUDO_gamePath[0] == '\0'){
		return FALSE;
	}

	char tempCheck[MAX_PATH];
	tempCheck[0] = '\0';

	copyString(globalPSEUDO_gamePath, tempCheck);


	/*
	for(int i = 0; i < 500; i++){
		easyForcePrintLine("%d: %i:%c, %i:%c", i, globalPSEUDO_gamePath[i], globalPSEUDO_gamePath[i], tempCheck[i], tempCheck[i]);

		BOOL doBreak = FALSE;
		if(globalPSEUDO_gamePath[i] == '\0'){
			easyForcePrintLine("GP FAIL");
			doBreak = TRUE;
		}
		if(tempCheck[i] == '\0'){
			easyForcePrintLine("TC FAIL");
			doBreak = TRUE;
		}
		if(doBreak)break;
	}
	*/

	UTIL_appendToEnd(tempCheck, arg_subdir);

	/*
	for(int i = 0; i < 500; i++){
		easyForcePrintLine("-%d: %i:%c", i, arg_subdir[i], arg_subdir[i]);
		if(arg_subdir[i] == '\0'){
			break;
		}
	}

	BOOL gpBrokenYet = FALSE;

	for(int i = 0; i < 500; i++){

		
		if(globalPSEUDO_gamePath[i] == '\0'){
			gpBrokenYet = TRUE;
		}

		
		if(gpBrokenYet == FALSE){
			easyForcePrintLine("%d: %i:%c, %i:%c", i, globalPSEUDO_gamePath[i], globalPSEUDO_gamePath[i], tempCheck[i], tempCheck[i]);
		}else{
			easyForcePrintLine("%d: %i:%c, %i:%c", i, 'e', 'e', tempCheck[i], tempCheck[i]);
		}
		BOOL doBreak = FALSE;
		if(tempCheck[i] == '\0'){
			easyForcePrintLine("TC FAIL");
			doBreak = TRUE;
		}
		if(doBreak)break;
	}
	*/


	//NOTE: see the "Thanks, ..." stack overflow link near the top of util.cpp for the source of this approach.  Nifty.
	if(stat( tempCheck, &info ) != 0)
        return FALSE;
    else if(info.st_mode & S_IFDIR)
        return TRUE;
    else
        return FALSE;


}





#ifndef _DEBUG

void processLoadHiddenCVarLine(const char* aryChrLineBuffer){

	if(EASY_CVAR_GET_DEBUGONLY(hiddenMemPrintout) == 1){
		easyForcePrintLine("RAW INPUT LINE: %s", aryChrLineBuffer);
	}

	if(isStringEmpty(aryChrLineBuffer) || lengthOfString(aryChrLineBuffer) < 2){
		//no / not enough content.  Garbage.
		return;
	}
			
	//find the space.
	int spacePos = UTIL_findCharFirstPos(aryChrLineBuffer, ' ');

	if(spacePos == -1){
		if(EASY_CVAR_GET_DEBUGONLY(hiddenMemPrintout) == 1){
			easyForcePrintLine("ISSUE; NO SPACE");
		}
		//try another line?
		return;
	}

	//atof();

	char identifier[128];
	char valueRaw[128];

	UTIL_substring(identifier, aryChrLineBuffer, 0, spacePos);
	lowercase(identifier);
	UTIL_substring(valueRaw, aryChrLineBuffer, spacePos + 1, -1);

	float value = 0;

	try{
		value = tryStringToFloat(valueRaw);
	}catch(int){
		if(EASY_CVAR_GET_DEBUGONLY(hiddenMemPrintout) == 1){
			easyForcePrintLine("ISSUE; CANT FIND NUMBER");
		}
		easyForcePrintLine("Error loading %s : \"%s\" could not be converted to a decimal.", identifier, valueRaw);
		//Must be able to extract a number to really do anything
		return;
	}


	//easyForcePrintLine("??????? %d : %s", strcmp(identifier, "glockuselastbulletanim" ), identifier);
			
	//easyForcePrintLine("are you serial    %s", line.c_str() );
			
	if(EASY_CVAR_GET_DEBUGONLY(hiddenMemPrintout) == 1){
		easyForcePrintLine("IDENTITY VALUE PAIR: %s %.2f", identifier, value);
	}
	
	
	EASY_CVAR_HIDDEN_LOAD_MASS;

	//if we reach here, it means the loaded name wasn't matched to a CVar we know of.
	easyForcePrintLine("WARNING: absMem entry \"%s\" not linked to a known CVar. Value discarded.", identifier);

}

#endif



void loadHiddenCVars(void){
	
#ifndef _DEBUG

	g_queueCVarHiddenSave = FALSE;  // ???  paranoia?
	if(globalPSEUDO_hiddenMemPath[0] == '\0'){
		return;
	}

	char aryChrLineBuffer[256];
	int intLineBufferPos = 0;
	
	//http://www.cplusplus.com/reference/cstdio/fscanf/
	FILE* myFile = fopen( globalPSEUDO_hiddenMemPath, "r");
	if (myFile)
	{
		char c;
		while(TRUE){
			c = fgetc(myFile);
			//next character.

			if(c == '\r'){
				//skip, unnecessary.
			}else if(c == '\n' || c == EOF){
				//End of file or new line? In any case count as a line break just in case the very last line didn't end in a newline character.
				
				aryChrLineBuffer[intLineBufferPos] = '\0'; //this makes some string-processing methods happier.
				processLoadHiddenCVarLine(&aryChrLineBuffer[0]);

				intLineBufferPos = 0;

				//If EOF, stop reading altogether.
				if(c == EOF){
					break;
				}
			}else{
				//any other character? Add to the buffer for this line.
				if(intLineBufferPos < 255){
					aryChrLineBuffer[intLineBufferPos] = c;
					intLineBufferPos++;
				}
			}
		}//END OF while reading file.

		easyPrintLine("***Hidden CVars Loaded***");
		//fprintf(fp, "%s", string);
		fclose(myFile);
	}


#endif
	



	/*
#ifndef _DEBUG

	if(globalPSEUDO_hiddenMemPath[0] == '\0'){
		return;
	}

	std::string lineRaw;
	ifstream myfile (globalPSEUDO_hiddenMemPath);
	if (myfile.is_open())
	{
		//easyForcePrintLine("IMA LOAD THIS");
		while ( getline (myfile,lineRaw) )
		{
			const char* thisLine = lineRaw.c_str();
			processLoadHiddenCVarLine(thisLine);
		}
		myfile.close();
	}
	else 
	{
		
	}
#endif
	*/


#ifndef CLIENT_DLL
	//easyForcePrintLine("hornetcvartest: what1 ", CVAR_GET_FLOAT("cl_hornetspiral"), globalPSEUDO_cl_hornetspiral);

	//hack. Force the PSEUDO value to match the current loaded (possibly) value of cl_hornetspiral so that the game doesn't automatically change more specific settings
	//about hornet movement the user may have specified. Note that cl_hornetspiral isn't hidden (normal CVar even in Release), so you have to get it from the CVar
	//system directly and not the stored variables (global_...)
	globalPSEUDO_cl_hornetspiral = EASY_CVAR_GET(cl_hornetspiral);
	
	//No need to set this to itself now...?
	//"settin"
	//global_cl_hornetspiral = CVAR_GET_FLOAT("cl_hornetspiral");
	EASY_CVAR_SET(cl_hornetspiral, EASY_CVAR_GET(cl_hornetspiral));
	

#endif

}//loadHiddenCVars


void saveHiddenCVars(void){
	
#ifndef _DEBUG
	g_queueCVarHiddenSave = FALSE;  // satisifed
	if(globalPSEUDO_hiddenMemPath[0] == '\0'){
		return;
	}
	FILE* myFile = fopen( globalPSEUDO_hiddenMemPath, "w");  //"a+t");
	if (myFile)
	{
		
		EASY_CVAR_HIDDEN_SAVE_MASS;

		fclose(myFile);
		if(EASY_CVAR_GET_DEBUGONLY(hiddenMemPrintout) == 1){
			easyForcePrintLine("File generation okay?"); 
		}

		easyPrintLine("***Hidden CVars Saved***");
	}else{
		if(EASY_CVAR_GET_DEBUGONLY(hiddenMemPrintout) == 1){
			easyForcePrintLine("File generation failed?"); 
		}
	}
#endif

	/*
#ifndef _DEBUG
	if(globalPSEUDO_hiddenMemPath[0] == '\0'){
		return;
	}
	//easyForcePrintLine("Unable to open file, generating...");
	ofstream myfile (globalPSEUDO_hiddenMemPath);
	if (myfile.is_open())
	{
		EASY_CVAR_HIDDENSAVE_MASS;
		myfile.close();
		if(global_hiddenMemPrintout == 1){
			easyForcePrintLine("File generation okay...?"); 
		}
	}else{
		if(global_hiddenMemPrintout == 1){
			easyForcePrintLine("File generation failed?"); 
		}
	}
#endif
	*/
}//saveHiddenCVars


//Provided character buffer (character array) must be binaryDigits+1 large to account for all digits and the terminating '/0', or 0-valued null character.
//Assuming an unsigned 32 bit int is provided.
void convertIntToBinary(char* buffer, unsigned int arg, unsigned int binaryDigits){

	//char aryChrReturn[9];
	//aryChrReturn[8] = '\0';

	buffer[0] = '\0';  //terminating character at the start just in case of error.

	unsigned int maxNumberPossible;

	if(binaryDigits > 32){
		easyForcePrintLine("ERROR: Too many biniary digits! Max for int is 32");
		return;
	}else if(binaryDigits == 32){
		//do-able, but need a different method.  (1 << 32) would overflow, leaving only "1". Just invert all bits of solid 0 for straight 1's.
		maxNumberPossible = ~0;
	}else if(binaryDigits < 0){
		easyForcePrintLine("ERROR: negative number of binaryDigits not allowed!");
		return;
	}else if(binaryDigits == 0){
		easyForcePrintLine("ERROR: binaryDigits cannot be 0. Nothing to do.");
		return;
	}else{
		//between 1 and 31 inclusive? this is fine. 2 to the binaryDigits power, minus 1 to remove the left-most single 1 bit and make all the ones right-ward into 1's.
		//That produces the largest number possibly stored by our number of bits all being 1's.
		//This is because all bits from 2^0 to 2^(binaryDigits - 1) are included,  2^(binaryDigits) itself is excluded.
		maxNumberPossible = (1 << binaryDigits) - 1;
	}

	/*
	unsigned int maxNumberPossible2 = (1 << binaryDigits);
	unsigned int maxNumberPossible3 = ~0;
	unsigned int maxNumberPossible = (1 << binaryDigits) - 1;
	*/

	if (arg > maxNumberPossible) {
		easyForcePrintLine("convertToBinary: ERROR. Overflow. Input number of %u is above the maximum allowed for %u digits!", arg, binaryDigits);
		return;
	}

	buffer[binaryDigits] = '\0';  //terminating character.

	for (unsigned int i = 0; i < binaryDigits; i++) {
		
		if (  (arg & (1 << i)) != 0 ) {
			//this digit is 1.
			buffer[binaryDigits-1 - i] = '1';
		}
		else {
			buffer[binaryDigits-1 - i] = '0';
		}
	}
	
	//return string(aryChrReturn);
}//END OF convertIntToBinary




//MODDD - from dlls/util.cpp, but also copied to hud_spectator (as-is).
// Despite a comment that says otherwise, it was identical to this method,
// just with a few less comments & commented-out section.
void UTIL_StringToVector( float *pVector, const char *pString )
{
	char *pstr, *pfront, tempString[128];
	int j;

	strcpy( tempString, pString );
	pstr = pfront = tempString;

	for ( j = 0; j < 3; j++ )			// lifted from pr_edict.c
	{
		pVector[j] = atof( pfront );

		while ( *pstr && *pstr != ' ' )
			pstr++;
		if (!*pstr)
			break;
		pstr++;
		pfront = pstr;
	}
	if (j < 2)
	{
		/*
		ALERT( at_error, "Bad field in entity!! %s:%s == \"%s\"\n",
			pkvd->szClassName, pkvd->szKeyName, pkvd->szValue );
		*/
		for (j = j+1;j < 3; j++)
			pVector[j] = 0;
	}
}


void UTIL_StringToIntArray( int *pVector, int count, const char *pString )
{
	char *pstr, *pfront, tempString[128];
	int j;

	strcpy( tempString, pString );
	pstr = pfront = tempString;

	for ( j = 0; j < count; j++ )			// lifted from pr_edict.c
	{
		pVector[j] = atoi( pfront );

		while ( *pstr && *pstr != ' ' )
			pstr++;
		if (!*pstr)
			break;
		pstr++;
		pfront = pstr;
	}

	for ( j++; j < count; j++ )
	{
		pVector[j] = 0;
	}
}


float UTIL_clamp(float argTest, float argMin, float argMax){
	if(argTest < argMin) return argMin;
	else if(argTest > argMax) return argMax;
	else return argTest;
}

Vector UTIL_ClampVectorToBox( const Vector &input, const Vector &clampSize )
{
	Vector temp = UTIL_ClampVectorToBoxNonNormalized(input, clampSize);

	return temp.Normalize();
}

Vector UTIL_ClampVectorToBoxNonNormalized( const Vector &input, const Vector &clampSize )
{
	Vector sourceVector = input;

	if ( sourceVector.x > clampSize.x )
		sourceVector.x -= clampSize.x;
	else if ( sourceVector.x < -clampSize.x )
		sourceVector.x += clampSize.x;
	else
		sourceVector.x = 0;

	if ( sourceVector.y > clampSize.y )
		sourceVector.y -= clampSize.y;
	else if ( sourceVector.y < -clampSize.y )
		sourceVector.y += clampSize.y;
	else
		sourceVector.y = 0;
	
	if ( sourceVector.z > clampSize.z )
		sourceVector.z -= clampSize.z;
	else if ( sourceVector.z < -clampSize.z )
		sourceVector.z += clampSize.z;
	else
		sourceVector.z = 0;

	return sourceVector;
}














BOOL IsMultiplayer(void){
#ifdef CLIENT_DLL
	// bIsMultiplayer()
	//return (gEngfuncs.GetMaxClients() == 1) ? FALSE : TRUE;
	return (gEngfuncs.GetMaxClients() > 1);
#else
	return g_pGameRules->IsMultiplayer();
#endif
}//END OF IsMultiplayer


void UTIL_Sparks(const Vector& position){
	//This starting method from the SDK in particular should no longer be called, having been replaced
	//by UTIL_Sparks (all calls to UTIL_Sparks in this project refer to UTIL_Sparks instead).

	//If this is somehow called again, please say so.
	//easyPrintLine("!!!!!!!!! SPARK CREATION UNSOURCED 2!!!!!!!!!");
	UTIL_Sparks(position, DEFAULT_SPARK_BALLS, EASY_CVAR_GET_DEBUGONLY(sparksEnvMulti));

}//END OF Util_Sparks(...)

void UTIL_Sparks(const Vector& position, int arg_ballsToSpawn, float arg_extraSparkMulti){

	if (EASY_CVAR_GET_DEBUGONLY(useAlphaSparks) == 0) {
		//use retail then.

#ifdef CLIENT_DLL
		//Clientside? Call for the effect directly.
		//float aryfl_pos[3];
		//aryfl_pos[0] = position.x;
		//aryfl_pos[1] = position.y;
		//aryfl_pos[2] = position.z;
		gEngfuncs.pEfxAPI->R_SparkShower((float*)&position);
#else
		//Serverside?  Tell it to call for that.
		MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, position);
		WRITE_BYTE(TE_SPARKS);
		WRITE_COORD(position.x);
		WRITE_COORD(position.y);
		WRITE_COORD(position.z);
		MESSAGE_END();
#endif
		return;
	}//END OF if

	//PLAYBACK_EVENT_FULL (FEV_GLOBAL, pGib->edict(), g_sTrail, 0.0, 
	//	(float *)&pGib->pev->origin, (float *)&pGib->pev->angles, 0.7, 0.0, pGib->entindex(), ROCKET_TRAIL, 0, 0);


	int ballsToSpawn;

	float multToUse = arg_extraSparkMulti * EASY_CVAR_GET_DEBUGONLY(sparksAllMulti);

	/*
	float multToUse = arg_extraSparkMulti;
	if(arg_extraSparkMulti == -1){
		//fall back to global.
		easyPrintLine("!!!!!!!!! SPARK CREATION UNSOURCED 1!!!!!!!!!");
		multToUse = EASY_CVAR_GET(sparkBallAmmountMulti);
	}
	*/

	if (multToUse != 1) {
		//multiplying by 1 is useless, so don't if it is.
		ballsToSpawn = (int)((float)arg_ballsToSpawn * multToUse);
	}
	else {
		ballsToSpawn = arg_ballsToSpawn;
	}



#ifdef CLIENT_DLL
	//Clientside?  Can call EV_ShowBalls directly.
	event_args_t tempArgSendoff;
	memset(&tempArgSendoff, 0, sizeof tempArgSendoff);
	// is this the same as setting to  {}, like 
	//     something = {}
	// ?

	tempArgSendoff.origin[0] = position.x;
	tempArgSendoff.origin[1] = position.y;
	tempArgSendoff.origin[2] = position.z;
	tempArgSendoff.iparam1 = ballsToSpawn;

	EV_ShowBalls(&tempArgSendoff);
#else
	//Serverside?  Tell the client to call "EV_ShowBalls" with the parameters.
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	PLAYBACK_EVENT_FULL(FEV_GLOBAL, NULL, g_sCustomBalls, 0.0, (float*)&position, (float*)&Vector(0, 0, 0), 0.0, 0.0, ballsToSpawn, 0, FALSE, FALSE);
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//( int flags, const edict_t *pInvoker, unsigned short eventindex, float delay, float *origin, float *angles, float fparam1, float fparam2, int iparam1, int iparam2, int bparam1, int bparam2 );
#endif


	/*
	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, position );
		WRITE_BYTE( TE_SPRAY );
		WRITE_COORD( position.x );
		WRITE_COORD( position.y );
		WRITE_COORD( position.z );

		WRITE_COORD( 0 );
		WRITE_COORD( 0 );
		WRITE_COORD( 0 );

		WRITE_BYTE(g_sGaussBallSprite);

		WRITE_BYTE(8);
		WRITE_BYTE(10);
		WRITE_BYTE(1);
		WRITE_BYTE(1);


	MESSAGE_END();
	*/

	// coord, coord, coord (position)
	// coord, coord, coord (direction)
	// short (modelindex)
	// byte (count)
	// byte (speed)
	// byte (noise)
	// byte (rendermode)



	//gEngfuncs.pEfxAPI->R_Sprite_Trail( TE_SPRITETRAIL, tr.endpos, fwd, m_iBalls, 8, 0.6, gEngfuncs.pfnRandomFloat( 10, 20 ) / 100.0, 100,
	//							255, 200 );


	//int type, float * start, float * end, int modelIndex, int count, float life, float size, float amplitude, int renderamt, float speed 

	//#define TE_SPRITETRAIL		15		// line of moving glow sprites with gravity, fadeout, and collisions
// coord, coord, coord (start) 
// coord, coord, coord (end) 
// short (sprite index)
// byte (count)
// byte (life in 0.1's) 
// byte (scale in 0.1's) 
// byte (velocity along vector in 10's)
// byte (randomness of velocity in 10's)

	//client: gEngfuncs.pfnRandomFloat( 10, 20 );
	//server: g_engfuncs.pfnRandomFloat( 10, 20 );
	//*not all commands are this squeakly clean from one to the other.  (make that uh, virtually none that are useful).


	/*
	byte rando = (byte)((g_engfuncs.pfnRandomFloat( 10, 20 ) / 100) * 10) ;

	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, position );
		WRITE_BYTE( TE_SPRITETRAIL );
		WRITE_COORD( position.x );
		WRITE_COORD( position.y );
		WRITE_COORD( position.z );

		WRITE_COORD( position.x );
		WRITE_COORD( position.y );
		WRITE_COORD( position.z );

		WRITE_SHORT(g_sGaussBallSprite);
		WRITE_BYTE(8);
		WRITE_BYTE( rando );

		WRITE_BYTE(10);
		WRITE_BYTE(10);

	MESSAGE_END();

	*/

}//END OF UTIL_Sparks





//MODDD - yet more shared now!
//TODO - a version that takes 'float*' might be nice for clientside?
// May not be as necessary now, support for the Vector side is more standardized now
// (Vector class supported everywhere C++)
int UTIL_PointContents(const Vector& vec)
{
#ifdef CLIENT_DLL
	return gEngfuncs.PM_PointContents((float*)&vec, NULL);
#else
	// serverside.
	return POINT_CONTENTS(vec);
#endif
}


float UTIL_WaterLevel(const Vector& position, float minz, float maxz)
{
	BOOL alreadyInWater = (UTIL_PointContents(position) == CONTENTS_WATER);
	int pointContentsTemp;

	if (alreadyInWater) {
		minz = position.z; //go no lower.
	}
	else {
		//go no higher.
		maxz = position.z;
	}

	Vector midUp = position;
	midUp.z = minz;

	pointContentsTemp = UTIL_PointContents(midUp);

	//MODDD - don't allow CONTENTS_SOLID to end early. It could just be at or past (beneath) the floor of this body of water.
	if (pointContentsTemp != CONTENTS_WATER && pointContentsTemp != CONTENTS_SOLID)
		return minz;

	midUp.z = maxz;
	if (UTIL_PointContents(midUp) == CONTENTS_WATER)
		return maxz;

	float diff = maxz - minz;
	while (diff > 1.0)
	{
		midUp.z = minz + diff / 2.0;

		pointContentsTemp = UTIL_PointContents(midUp);
		if (pointContentsTemp == CONTENTS_WATER)
		{
			minz = midUp.z;
		}
		else if (pointContentsTemp != CONTENTS_SOLID)
		{
			maxz = midUp.z;
		}
		else
		{
			//MODDD - NEW. If equal to solid, it depends on what direction we want to go based on "alreadyInWater".
			if (alreadyInWater) {
				//lean on being below this.
				maxz = midUp.z;
			}
			else {
				//lean on being above this.
				minz = midUp.z;
			}

		}
		diff = maxz - minz;
	}

	return midUp.z;
}


void UTIL_Bubbles(Vector mins, Vector maxs, int count)
{
	Vector mid = (mins + maxs) * 0.5;

	float flHeight = UTIL_WaterLevel(mid, mid.z, mid.z + 1024);
	flHeight = flHeight - mins.z;


// Now for the call.
#ifdef CLIENT_DLL
	gEngfuncs.pEfxAPI->R_Bubbles(mins, maxs, flHeight, g_sModelIndexBubbles, count, 8);
	// does this make sense?
	//void	(*R_Bubbles)					(float* mins, float* maxs, float height, int modelIndex, int count, float speed);
	//void	(*R_BubbleTrail)				(float* start, float* end, float height, int modelIndex, int count, float speed);
#else
	MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, mid);
	WRITE_BYTE(TE_BUBBLES);
	WRITE_COORD(mins.x);	// mins
	WRITE_COORD(mins.y);
	WRITE_COORD(mins.z);
	WRITE_COORD(maxs.x);	// maxz
	WRITE_COORD(maxs.y);
	WRITE_COORD(maxs.z);
	WRITE_COORD(flHeight);			// height
	WRITE_SHORT(g_sModelIndexBubbles);
	WRITE_BYTE(count); // count
	WRITE_COORD(8); // speed
	MESSAGE_END();
#endif
}// UTIL_Bubbles



void UTIL_BubbleTrail(Vector from, Vector to, int count)
{
	float flHeight = UTIL_WaterLevel(from, from.z, from.z + 256);
	flHeight = flHeight - from.z;

	if (flHeight < 8)
	{
		flHeight = UTIL_WaterLevel(to, to.z, to.z + 256);
		flHeight = flHeight - to.z;
		if (flHeight < 8)
			return;

		// UNDONE: do a ploink sound
		flHeight = flHeight + to.z - from.z;
	}

	if (count > 255)
		count = 255;


// Now for the call.
#ifdef CLIENT_DLL
	gEngfuncs.pEfxAPI->R_BubbleTrail(from, to, flHeight, g_sModelIndexBubbles, count, 8);
	// does this make sense?
	//void	(*R_Bubbles)					(float* mins, float* maxs, float height, int modelIndex, int count, float speed);
	//void	(*R_BubbleTrail)				(float* start, float* end, float height, int modelIndex, int count, float speed);
#else
	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_BUBBLETRAIL);
	WRITE_COORD(from.x);	// mins
	WRITE_COORD(from.y);
	WRITE_COORD(from.z);
	WRITE_COORD(to.x);	// maxz
	WRITE_COORD(to.y);
	WRITE_COORD(to.z);
	WRITE_COORD(flHeight);			// height
	WRITE_SHORT(g_sModelIndexBubbles);
	WRITE_BYTE(count); // count
	WRITE_COORD(8); // speed
	MESSAGE_END();
#endif

}// UTIL_BubbleTrail







void InitShared(void) {


	// at startup, fill these arrays for reading from anytime.
	// Can do from querying the version info from the server if necessary, although it's fine to trust client DLL's.
#ifdef CLIENT_DLL
	writeVersionInfo(globalbuffer_cl_mod_version, 128);
	writeDateInfo(globalbuffer_cl_mod_date, 128);
	// fill the display buffer with this info.
	sprintf(&globalbuffer_cl_mod_display[0], "CL: %s - %s", &globalbuffer_cl_mod_version[0], &globalbuffer_cl_mod_date[0]);

	// mark these as yet to be filled (conncet a server)
	globalbuffer_sv_mod_version_cache[0] = '\0';
	globalbuffer_sv_mod_date_cache[0] = '\0';
	globalbuffer_sv_mod_display[0] = '\0';
#else
	writeVersionInfo(globalbuffer_sv_mod_version, 128);
	writeDateInfo(globalbuffer_sv_mod_date, 128);
#endif





}//END OF InitShared



// Gets called every map load client and serverside for safety.
// Even clientside, as preaches there only done once at init can randomly get glitchy.
void PrecacheShared(void){
	g_sModelIndexBubbles = PRECACHE_MODEL_SHARED("sprites/bubble.spr");//bubbles


	// TEST.  See how other indexes change?  or don't?
	// Shouldn't ever be 0, most likely? I don't know if anything ever gets 0 intentionally.
	// Everything getting 0 one after the other though (for different files each time),
	// that's definitely bad.
	//int tester2 =PRECACHE_MODEL_SHARED("sprites/hotglow.spr");
	//int tester3 =PRECACHE_MODEL_SHARED("sprites/bubble.spr");

}//END OF PrecacheShared



 // NOTICE - gets called at every map-load for serverside, but only on loading the first map on clientside.
 // No idea why, been that way since retail, don't question how precache stuff works.
 // Its inner achinations are incomprehensible to meer mortals such as we.
void ClearWeaponInfoCache(void){
	// each side will handle calls to "RegisterWeapon" on its own to initalize weapon info.
	// Clientside does it through "HUD_PrepEntity" per weapon, Serverside does it through "UTIL_PrecacheOtherWeapon" per weapon.
	memset(CBasePlayerItem::ItemInfoArray, 0, sizeof(CBasePlayerItem::ItemInfoArray));
	memset(CBasePlayerItem::AmmoInfoArray, 0, sizeof(CBasePlayerItem::AmmoInfoArray));
	giAmmoIndex = 0;
}









/*
void	(*R_Explosion)				(float* pos, int model, float scale, float framerate, int flags);

void	(*R_ParticleExplosion)		(float* org);
void	(*R_ParticleExplosion2)		(float* org, int colorStart, int colorLength);


void	(*R_Spray)					(float* pos, float* dir, int modelIndex, int count, int speed, int spread, int rendermode);
void	(*R_Sprite_Explode)			(TEMPENTITY* pTemp, float scale, int flags);
void	(*R_Sprite_Smoke)				(TEMPENTITY* pTemp, float scale);
void	(*R_Sprite_Spray)				(float* pos, float* dir, int modelIndex, int count, int speed, int iRand);
void	(*R_Sprite_Trail)				(int type, float* start, float* end, int modelIndex, int count, float life, float size, float amplitude, int renderamt, float speed);



void	(*R_SparkEffect)				(float* pos, int count, int velocityMin, int velocityMax);
void	(*R_SparkShower)				(float* pos);
void	(*R_SparkStreaks)				(float* pos, int count, int velocityMin, int velocityMax);
*/







/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
