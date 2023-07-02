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

// MODDD NOTICE - this file, too, is included both client/serverside.



//MODDD - WHY NOT???
#ifndef CBASE_H
#define CBASE_H


/*

Class Hierachy

CBaseEntity
	CBaseDelay
		CBaseToggle
			CBaseItem
			CBaseMonster
				CBaseCycler
				CBasePlayer
				CBaseGroup
*/




// what.  dude, this is cbase.h.
//#include "cbase.h"
// Better believe we'll need this though.
#include "util_entity.h"

// Also, look below for an overload of FNullEnt that used to be in player.cpp.
// Moved below CBaseEntity's class definition so it can be used mostly anywhere else now too.






// These are caps bits to indicate what an object's capabilities (currently used for save/restore and level transitions)
#define FCAP_CUSTOMSAVE				0x00000001
#define FCAP_ACROSS_TRANSITION		0x00000002		// should transfer between transitions
#define FCAP_MUST_SPAWN				0x00000004		// Spawn after restore
#define FCAP_DONT_SAVE				0x80000000		// Don't save this
#define FCAP_IMPULSE_USE			0x00000008		// can be used by the player
#define FCAP_CONTINUOUS_USE			0x00000010		// can be used by the player
#define FCAP_ONOFF_USE				0x00000020		// can be used by the player
#define FCAP_DIRECTIONAL_USE		0x00000040		// Player sends +/- 1 when using (currently only tracktrains)
#define FCAP_MASTER					0x00000080		// Can be used to "master" other entities (like multisource)

// UNDONE: This will ignore transition volumes (trigger_transition), but not the PVS!!!
#define FCAP_FORCE_TRANSITION		0x00000080		// ALWAYS goes across transitions



// what... what is this.  Nowhere refered to this constant
//#define BAD_WEAPON 0x00007FFF




//MODDD - bizarre...? Shouldn't basemonster.h or .cpp include schedule.h and monsterevent.h instead?
#include "saverestore.h"
#include "schedule.h"

#include "monsterevent.h"

#include "util.h"
#include "vector.h"





// spawnflag.
#define SF_NORESPAWN	( 1 << 30 )// !!!set this bit on guns and stuff that should never respawn.





//MODDD - moved from nodes.h, accessible in more places.
//MODDD - new.  Don't treat like the usual NODE HULL types below. 
//              NODE_POINT_DEFAULT says we're leaving up what NODE HULL this is to retail behavior, the checks in 
//              NODE_POINT_HULL is to say, pretend like our entity has no size. Just allow if there is a node connection at all.
//              For debugging route problems to see if the issue is caused by a hull being too big even if a path was found.
//  ALTHOUGH, these are incompatible with the static table!  DEFAULT_HULL is more of a signal that never gets used in the end, but
//  POINT_HULL can be.   Unknown why it didn't start at POINT_HULL to begin with.
//  Careful about changing that though, it would break the existing static route tables in existing maps.  Not a great idea without
//  some agreement, and nothing seems to use NODE_POINT_HULL anyway (just a debug feature).
#define NODE_DEFAULT_HULL		-2
#define NODE_POINT_HULL			-1

#define NODE_SMALL_HULL			0
#define NODE_HUMAN_HULL			1
#define NODE_LARGE_HULL			2
#define NODE_FLY_HULL			3

#define MAX_NODE_HULLS			4






//MODDD - new set of constants.  Sometimes we need to tell different types of projectiles apart from one another.
//safe default for all entities.  Override "GetProjectileType" to change per entity class.
#define PROJECTILE_NONE 0
//Not really intended to be thrown or go flying to a target.  Not quite the remote charges (satchel), but includes tripmines.
#define PROJECTILE_DEPLOYABLE 1
//Just the crossbow's bolt.
#define PROJECTILE_BOLT 2
//mp5 grenades, hand grenades. Moderate weighted non-propelled grenandes.
#define PROJECTILE_GRENADE 3
//projectile with a strong sense of direction, propelled, following or not.
#define PROJECTILE_ROCKET 4
//balls of electricity from controllers, archers, kingpins.
#define PROJECTILE_ENERGYBALL 5
//organic projectiles that are usually big green and toxic like bullsquid spit.
#define PROJECTILE_ORGANIC_DUMB 6
//living things that act like projectiles but can't do damage (chumtoads)
#define PROJECTILE_ORGANIC_HARMLESS 7
//living things that act like damaging projectiles on impact, with at least minimal AI to separate it from ORGANIC_DUMB.
#define PROJECTILE_ORGANIC_HOSTILE 8


// For CLASSIFY
#define CLASS_NONE				0
#define CLASS_MACHINE			1
#define CLASS_PLAYER			2
#define CLASS_HUMAN_PASSIVE		3
#define CLASS_HUMAN_MILITARY	4
#define CLASS_ALIEN_MILITARY	5
#define CLASS_ALIEN_PASSIVE		6
#define CLASS_ALIEN_MONSTER		7
#define CLASS_ALIEN_PREY		8
#define CLASS_ALIEN_PREDATOR	9
#define CLASS_INSECT			10
#define CLASS_PLAYER_ALLY		11
#define CLASS_PLAYER_BIOWEAPON	12 // hornets and snarks.launched by players
#define CLASS_ALIEN_BIOWEAPON	13 // hornets and snarks.launched by the alien menace
#define CLASS_BARNACLE			99 // special because no one pays attention to it, and it eats a wide cross-section of creatures.





// (MOVED FROM basemonster.h, originally from monsters.h for the entity methods that involve relationships)
// monster to monster relationship types

//MODDD - NEW TYPE: 
#define R_DEFAULT	-5  // A signal to allow the default relationship as intended in the table.

//#define R_FO	-4  // food. I attract the other monster, and may get eaten at melee range.
//#define R_BA	-3	// bait. I attract the other monster, but don't get attacked by it.


#define R_CA    -3 // (CAUTIOUS). Not an enemy (yet?), but looks kind of scary. Don't attack or piss off, but if cowardly (scientist), point and gawk, maybe run away. If not (Barney), point your gun at it and stare.
                   //This status should occur on enemies that are marked provokable and not yet provoked, unless Barnies are supposed to be a liablity and stupidly prvoke it regardless of your wishes.
                   //This will NOT be default AI - factions HUMAN_MILITARY and anything ALIEN don't care about avoiding conflict.

#define R_AL	-2 // (ALLY) pals. Good alternative to R_NO when applicable.
#define R_FR	-1// (FEAR)will run
#define R_NO	0// (NO RELATIONSHIP) disregard
#define R_DL	1// (DISLIKE) will attack
#define R_HT	2// (HATE)will attack this character instead of any visible DISLIKEd characters
#define R_NM	3// (NEMESIS)  A monster Will ALWAYS attack its nemsis, no matter what

//MODDD - new level. Bait. To be used in a class (Chumtoad) only.
#define R_BA    4//



// Neither of these constants was used (both in weapons.cpp, TRACER_FREQ cloned here in cbase.h or whatever reason)
//#define TRACER_FREQ		4			// Tracers fire every 4 bullets
//#define NOT_USED 255





// Ugly technique to override base member functions
// Normally it's illegal to cast a pointer to a member function of a derived class to a pointer to a 
// member function of a base class.  static_cast is a sleezy way around that problem.

#ifdef _DEBUG

#define SetThink( a ) ThinkSet( static_cast <void (CBaseEntity::*)(void)> (a), #a )
#define SetTouch( a ) TouchSet( static_cast <void (CBaseEntity::*)(CBaseEntity *)> (a), #a )
#define SetUse( a ) UseSet( static_cast <void (CBaseEntity::*)(	CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )> (a), #a )
#define SetBlocked( a ) BlockedSet( static_cast <void (CBaseEntity::*)(CBaseEntity *)> (a), #a )

#else

#define SetThink( a ) m_pfnThink = static_cast <void (CBaseEntity::*)(void)> (a)
#define SetTouch( a ) m_pfnTouch = static_cast <void (CBaseEntity::*)(CBaseEntity *)> (a)
#define SetUse( a ) m_pfnUse = static_cast <void (CBaseEntity::*)( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )> (a)
#define SetBlocked( a ) m_pfnBlocked = static_cast <void (CBaseEntity::*)(CBaseEntity *)> (a)

#endif



// people gib if their health is <= this at the time of death
#define GIB_HEALTH_VALUE	-30



//MODDD - was 8, how about 16?
#define ROUTE_SIZE			16 // how many waypoints a monster can store at one time

//MODDD - was 10.  And why was MAX_PATH_SIZE over ROUTE_SIZE's as-is value then? 
// Just... be the same as the max # of nodes a monster can take for a route.
#define MAX_PATH_SIZE ROUTE_SIZE // max number of nodes available for a path.




#define MAX_OLD_ENEMIES		4 // how many old enemies to remember

#define bits_CAP_DUCK			( 1 << 0 )// crouch
#define bits_CAP_JUMP			( 1 << 1 )// jump/leap
#define bits_CAP_STRAFE			( 1 << 2 )// strafe ( walk/run sideways)
#define bits_CAP_SQUAD			( 1 << 3 )// can form squads
#define bits_CAP_SWIM			( 1 << 4 )// proficiently navigate in water
#define bits_CAP_CLIMB			( 1 << 5 )// climb ladders/ropes
#define bits_CAP_USE			( 1 << 6 )// open doors/push buttons/pull levers
#define bits_CAP_HEAR			( 1 << 7 )// can hear forced sounds
#define bits_CAP_AUTO_DOORS		( 1 << 8 )// can trigger auto doors
#define bits_CAP_OPEN_DOORS		( 1 << 9 )// can open manual doors
#define bits_CAP_TURN_HEAD		( 1 << 10)// can turn head, always bone controller 0

#define bits_CAP_RANGE_ATTACK1	( 1 << 11)// can do a range attack 1
#define bits_CAP_RANGE_ATTACK2	( 1 << 12)// can do a range attack 2
#define bits_CAP_MELEE_ATTACK1	( 1 << 13)// can do a melee attack 1
#define bits_CAP_MELEE_ATTACK2	( 1 << 14)// can do a melee attack 2

#define bits_CAP_FLY			( 1 << 15)// can fly, move all around

#define bits_CAP_DOORS_GROUP    (bits_CAP_USE | bits_CAP_AUTO_DOORS | bits_CAP_OPEN_DOORS)

// used by suit voice to indicate damage sustained and repaired type to player

// instant damage

//MODDD - damage types (DMG_...) and itbd_'s moved to util_shared.h.  Merged with some other redundant stuff in some
// clientside files.

// when calling KILLED(), a value that governs gib behavior is expected to be 
// one of these three values
#define GIB_NORMAL			0// gib if entity was overkilled
#define GIB_NEVER			1// never gib, no matter how much death damage is done ( freezing, etc )
#define GIB_ALWAYS			2// always gib ( Houndeye Shock, Barnacle Bite )
#define GIB_ALWAYS_NODECAL	3// MODDD - new. Always gib, but no decals (blood splatter on the ground) for spawned gibs.





class CBaseEntity;
class CBaseMonster;
class CBasePlayerItem;
class CSquadMonster;




class CCineMonster;
// Note that even though CSound is serverside, still need it clientside as all the standard serverside entity header files (cbase.h, basemonster.h)
// are included to keep references to entity-types in shared script (specific weapon .cpp files and other needed headers) satisfied, even though
// their implementations are either never seen or dummied in hl/hl_baseentity.cpp.  This applies to other types too.
class CSound;







typedef enum { USE_OFF = 0, USE_ON = 1, USE_SET = 2, USE_TOGGLE = 3 } USE_TYPE;

typedef void (CBaseEntity::*BASEPTR)(void);
typedef void (CBaseEntity::*ENTITYFUNCPTR)(CBaseEntity *pOther );
typedef void (CBaseEntity::*USEPTR)( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );



//MODDD - moved defines for 'EXPORT' to const.h.

extern "C" EXPORT int GetEntityAPI( DLL_FUNCTIONS *pFunctionTable, int interfaceVersion );
extern "C" EXPORT int GetEntityAPI2( DLL_FUNCTIONS *pFunctionTable, int *interfaceVersion );


extern int DispatchCreated(edict_t* pent);
extern int DispatchSpawn( edict_t *pent );
extern void DispatchKeyValue( edict_t *pentKeyvalue, KeyValueData *pkvd );
extern void DispatchTouch( edict_t *pentTouched, edict_t *pentOther );
extern void DispatchUse( edict_t *pentUsed, edict_t *pentOther );
extern void DispatchThink( edict_t *pent );
extern void DispatchBlocked( edict_t *pentBlocked, edict_t *pentOther );
extern void DispatchSave( edict_t *pent, SAVERESTOREDATA *pSaveData );
extern int  DispatchRestore( edict_t *pent, SAVERESTOREDATA *pSaveData, int globalEntity );
extern void DispatchObjectCollsionBox( edict_t *pent );
extern void SaveWriteFields( SAVERESTOREDATA *pSaveData, const char *pname, void *pBaseData, TYPEDESCRIPTION *pFields, int fieldCount );
extern void SaveReadFields( SAVERESTOREDATA *pSaveData, const char *pname, void *pBaseData, TYPEDESCRIPTION *pFields, int fieldCount );
extern void SaveGlobalState( SAVERESTOREDATA *pSaveData );
extern void RestoreGlobalState( SAVERESTOREDATA *pSaveData );
extern void ResetGlobalState( void );

extern void FireTargets( const char *targetName, CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

//MODDD - temporary.
extern void FireTargetsTest( const char *targetName, CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );








//
// EHANDLE. Safe way to point to CBaseEntities who may die between frames
//
class EHANDLE
{
private:
	edict_t *m_pent;
	int	m_serialnumber;
public:
	edict_t *Get( void );
	edict_t *Set( edict_t *pent );

	//MODDD NOTE - this type used to be int, it is now BOOL to more
	// clearly convey the point.
	// See the implementation for this auto-cast, "Get() != NULL".
	// This isn't some ID, it is just a yes/no answer, a BOOL.
	// Often invokedy by EHANDLE's sent to FNullEnt.
	operator BOOL ();

	operator CBaseEntity *();

	CBaseEntity * operator = (CBaseEntity *pEntity);
	CBaseEntity * operator ->();

	CBaseEntity* GetEntity();
};


//
// Base Entity.  All entity types derive from this
//
class CBaseEntity 
{
public:

	// Constructor.  Set engine to use C/C++ callback functions
	// pointers to engine data
	entvars_t* pev;		// Don't need to save/restore this pointer, the engine resets it

	// path corners
	CBaseEntity* m_pGoalEnt;// path corner we are heading towards
	CBaseEntity* m_pLink;// used for temporary link-list operations. 

	//MODDD - 'ammo_' variables moved to CBasePlayer.
	// Why were they available for even NPCs that never use them...

	//Special stuff for grenades and satchels.
	float m_flStartThrow;
	float m_flReleaseThrow;

	//WARNING - unreliable.  Do not use.
	//float fuser4;
	/*
	float fuser5;
	float fuser6;
	float fuser7;
	float fuser8;
	*/

	int m_chargeReady;
	int m_fInAttack;
	int m_fireState;


	BOOL alreadySaved;
	int wasAttached;

	//MODDD - new instance var.
	BOOL usingCustomSequence;
	BOOL doNotResetSequence;

	// New var to send printouts related to animation.
	// Turn on for an entity by looking at it ingame and typing "crazyprintout"
	BOOL crazyPrintout;



	//MODDD
	BOOL spawnedDynamically;
	// Might want to know if the player sent along spawnflags in a 'give' command.
	BOOL flagForced;

	Activity timeOfDeath_activity;
	int timeOfDeath_sequence;

	float waitForScriptedTime;
	
	// For being grabbed by a gargantua.  Revert the old pev->mins/maxs right before saving, then back to the ones to be used
	// while grabbed (0'd).
	BOOL isGrabbed;
	Vector m_vecOldBoundsMins;
	Vector m_vecOldBoundsMaxs;
	float m_fOldGravity;





	static CBaseEntity *Instance( edict_t *pent )
	{ 
		if ( !pent )
			pent = ENT(0);
		CBaseEntity *pEnt = (CBaseEntity *)GET_PRIVATE(pent); 
		return pEnt; 
	}

	static CBaseEntity *Instance( entvars_t *pev ) { return Instance( ENT( pev ) ); }
	static CBaseEntity *Instance( int eoffset) { return Instance( ENT( eoffset) ); }

	static edict_t* overyLongComplicatedProcessForCreatingAnEntity(const char* entityName);
	static CBaseEntity* CreateManual(const char* szName, const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner = NULL);
	static CBaseEntity* Create(const char* szName, const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner = NULL);
	static CBaseEntity* Create(const char* szName, const Vector& vecOrigin, const Vector& vecAngles, int setSpawnFlags, edict_t* pentOwner = NULL);




	virtual void ReportGeneric(void);

	//MODDD - NEW!!! compatability.
	Vector GetAbsVelocity(void);

	Vector GetAbsOrigin(void);
	void SetAbsOrigin(const Vector& arg_newOrigin);

	Vector GetAbsAngles(void);
	void SetAbsAngles(const Vector& arg_newOrigin);

	void ChangeAngleX(const float changeBy);
	void ChangeAngleY(const float changeBy);
	void ChangeAngleZ(const float changeBy);
	void SetAngleX(const float newVal);
	void SetAngleY(const float newVal);
	void SetAngleZ(const float newVal);
	
	
	virtual BOOL isBasePlayerWeapon(void){return FALSE;}

	virtual BOOL blocksImpact(void);
	virtual float massInfluence(void);
	virtual int GetProjectileType(void);

	virtual Vector GetVelocityLogical(void);
	virtual void SetVelocityLogical(const Vector& arg_newVelocity);

	virtual void OnDeflected(CBaseEntity* arg_entDeflector);



	//MODDDMIRROR - snippets needed for the mirror from Spirit of HL 1.9.
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	//LRC - decent mechanisms for setting think times!
	// this should have been done a long time ago, but MoveWith finally forced me.
	virtual void SetNextThink( float delay ) { SetNextThink(delay, FALSE); }
	virtual void SetNextThink( float delay, BOOL correctSpeed );

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	

	virtual BOOL isMovetypeFlying(void);

	//MODDD - new. 
	virtual BOOL isSizeGiant(void);
	virtual BOOL isOrganic(void);
	virtual int getHullIndexForNodes(void);
	virtual int getHullIndexForGroundNodes(void);
	virtual int getNodeTypeAllowed(void);

	int getNumberOfBodyParts(void);
	int getNumberOfSkins(void);


	//MODDD
	virtual void onDelete(void);

	//MODDD
	const char* getClassname(void);
	const char* getClassnameShort(void);
	
	// initialization functions
	//MODDD - changed for a test.
	//virtual void Spawn( void ) { return; }
	virtual void Spawn( void );
	virtual void Precache( void ) { return; }

	//MODDD - new
	CBaseEntity(void);
	//static void method_precacheAll();


	//Even though the relationship may disallow it, I can be caught by the barnacle anyways (if true).
	virtual BOOL getIsBarnacleVictimException(void);
	virtual BOOL isBreakableOrChild(void);
	virtual BOOL isDestructibleInanimate(void);
	virtual BOOL isTalkMonster(void);
	virtual BOOL isProvokable(void);
	virtual BOOL isProvoked(void);

	//also MODDD
	virtual void DefaultSpawnNotice(void);
	virtual void ForceSpawnFlag(int arg_spawnFlag);
	
	//MODDD
	virtual BOOL usesSoundSentenceSave(void);

	
	//MODDD
	virtual void playAmmoPickupSound();
	virtual void playAmmoPickupSound(entvars_t* sentPev);
	virtual void playGunPickupSound();
	virtual void playGunPickupSound(entvars_t* sentPev);
	virtual void precacheAmmoPickupSound();
	virtual void precacheGunPickupSound();


	//MODDD - method to create a blood effect.  Moved to CBaseEntity for involving more details about the entity
	// spawning the blood without the need for 3 to 6 parameters.
	virtual void SpawnBlood(const Vector& vecSpot, float flDamage);
	virtual void SpawnBloodSlash(float flDamage, const Vector& vecDrawLoc, const Vector& vecTraceLine);
	virtual void SpawnBloodSlash(float flDamage, const Vector& vecDrawLoc, const Vector& vecTraceLine, const BOOL& extraBlood);



	virtual void KeyValue( KeyValueData* pkvd) { pkvd->fHandled = FALSE; }
	virtual int	Save( CSave &save );
	virtual int	Restore( CRestore &restore );

	//MODDD - so this isn't virtual?
	// Here's how the call to some child class's "Restore" goes:
	//     CSomeCreature::Restore -> CBaseMonster::Restore
	//       CBaseMonster::Restore -> CBaseEntity::Restore
	//         CBaseEntity::Restore -> CBaseEntity::PostRestore
	//         CBaseMonster::PostRestore
	//       CSomeCreature::PostRestore
	// ...it works exactly as it does when PostRestore is non-virtual.  But if it were
	// virtual, each PostRestore call would go to the deepest version possible
	// (CSomeCreature's PostRestore, which,)
	// even if it goes up the heirarchy of PostRestore's, would still mean each of 
	// those travels happens 3 times in this case.
	void PostRestore(void);


	virtual void playMetallicHitSound(int arg_channel, float arg_volume);

	//Made because this check is done a lot for traces. Need to know if a wall, floor, any part of the map, etc. was hit
	virtual BOOL IsWorld(void);
	virtual BOOL IsWorldAffiliated(void);
	BOOL IsWorldOrAffiliated(void);  //NOT virtual. Never needs to be special.
	virtual BOOL IsBreakable(void);

	virtual int	ObjectCaps( void ) { return FCAP_ACROSS_TRANSITION; }
	virtual void Activate( void ) {}
	
	// Setup the object->object collision box (pev->mins / pev->maxs is the object->world collision box)
	virtual void SetObjectCollisionBox( void );


	//Method to call without a string. Can be used to check the German model system for monsters (substitute a different one if censorship is on)
	virtual void setModel(void);
	//Method called to set this entity's model. Can be overridden by classes that need to set something that comes from the model, such as number of skins for eye-blinking.
	virtual void setModel(const char* m);



	// A simpler version of forcedRelationshipWith that should be considered at the same time.
	// Just a quick check to see whether the current should be unconditionably hated no matter what.
	// It isn't taken to heart too much, as it would conflict with  forcedRelationshipWith.
	// This method is not involved in IRelationship.
	virtual BOOL isForceHated(CBaseEntity *pBy);


	// This method allows child classes to change how other monsters look at this particular instance (or the class in general towards other's by class, period).
	// Such as, make a Bullsquid ignore Headcrabs (a Headcrab could override forcedRelationshipWith to say R_NO if pWith's class is monster_bullsquid).
	// Or a  more realistic example: The chumtoad can force all other monsters to hate it, besides fellow toads (class check) without needing to check their class and
	//   then send a class it happens to hate to be hated (really dodgy and not possibly anyways in IRelationship, which is caller-sided)
	virtual int forcedRelationshipWith(CBaseEntity *pWith);

	
#ifndef CLIENT_DLL
	// This method lets script in GetSchedule related to picking up on bait sounds or not get handled in one Monster method instead of all over the place.
	//also, server only. AI only.
	virtual SCHEDULE_TYPE getHeardBaitSoundSchedule(CSound* pSound);
	// This underscored version skips the monsterstate (IDLE / ALERT) check. It should only be called by "getHeardBaitSoundSchedule" without a provided Sound, which had
	// to find a sound itself and already did the state check.
	virtual SCHEDULE_TYPE _getHeardBaitSoundSchedule(CSound* pSound);
	virtual SCHEDULE_TYPE getHeardBaitSoundSchedule();
#endif;



// Classify - returns the type of group (i.e, "houndeye", or "human military" so that monsters with different classnames
// still realize that they are teammates. (overridden for monsters that form groups)
	virtual int Classify ( void ) { return CLASS_NONE; }
	virtual void DeathNotice ( entvars_t *pevChild ) {}// monster maker children use this to tell the monster maker that they have died.


	static TYPEDESCRIPTION m_SaveData[];


	//MODDD - new args possible.
	GENERATE_TRACEATTACK_PROTOTYPE_VIRTUAL
	virtual void TraceAttack_Traceless(entvars_t* pevAttacker, float flDamage, Vector vecDir, int bitsDamageType, int bitsDamageTypeMod);
	GENERATE_TAKEDAMAGE_PROTOTYPE_VIRTUAL

	// NEW
	virtual void Knockback(const int knockbackAmount, const Vector& knockbackDir);
	// NEW, common TakeDamage utility method
	virtual BOOL ChangeHealthFiltered(entvars_t* pevAttacker, float flDamage);


	

	virtual int	TakeHealth( float flHealth, int bitsDamageType );

	GENERATE_KILLED_PROTOTYPE_VIRTUAL
	//virtual void Killed( entvars_t *pevAttacker, int iGib );

	virtual int	BloodColor( void ) { return DONT_BLEED; }

	// MODDD - NEW.  Extra filter on top of BloodColor, in case it has a blood color for other purposes.
	// Think of BloodColor as more of a statistic.  Some things may have a blood color but still not
	// bleed in certain ways (turrets still have the black (oil) blood color, but don't bleed particles
	// unless CVar 'turretBleedsOil' is on).  Also not for censorship, see UTIL_ShouldShowBlood and several
	// spots in combat.cpp for that.
	virtual BOOL CanMakeBloodParticles(void) {return FALSE;}

	//MODDD - TraceBleed supports the extra damage bitmask.
	virtual void TraceBleed( float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType );
	virtual void TraceBleed( float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType, int bitsDamageTypeMod );
	virtual BOOL IsTriggered( CBaseEntity *pActivator ) {return TRUE;}
	virtual CBaseMonster *MyMonsterPointer( void ) { return NULL;}
	virtual CSquadMonster *MySquadMonsterPointer( void ) { return NULL;}
	virtual	int	GetToggleState( void ) { return TS_AT_TOP; }
	virtual void AddPoints( int score, BOOL bAllowNegativeScore ) {}
	virtual void AddPointsToTeam( int score, BOOL bAllowNegativeScore ) {}
	virtual BOOL AddPlayerItem( CBasePlayerItem *pItem ) { return 0; }
	virtual BOOL RemovePlayerItem( CBasePlayerItem *pItem ) { return 0; }
	
	//MODDD - 'const char*' now
	virtual int	GiveAmmo( int iAmount, const char* szName, int iMax ) { return -1; }
	virtual float GetDelay( void ) { return 0; }
	virtual int	IsMoving( void ) { return pev->velocity != g_vecZero; }
	virtual void OverrideReset( void ) {}

	//supports the extra damage bitmask for some reason.
	virtual int	DamageDecal( int bitsDamageType );
	virtual int	DamageDecal( int bitsDamageType, int bitsDamageTypeMod );

	// This is ONLY used by the node graph to test movement through a door
	virtual void SetToggleState( int state ) {}
	
	//MODDD - NOTE.     what.   Only overridden by the player and nothing checks
	// for anything related to there either.
	//virtual void    StartSneaking( void ) {}
	//virtual void    StopSneaking( void ) {}
	
	virtual BOOL	OnControls( entvars_t *pev ) { return FALSE; }
	virtual BOOL    IsSneaking( void ) { return FALSE; }


	//MODDD - why does IsAlive check for above 0 health?  The deadflag is always properly adjusted if it goes under 0.
	//virtual BOOL	IsAlive( void ) { return (pev->deadflag == DEAD_NO) && pev->health > 0; }
	virtual BOOL IsAlive(void){return (pev->deadflag == DEAD_NO); }
	
	//MODD - NEW.  Might want some things (chumtoads) to have the ability to fool other monsters into thinking they are dead and be ignored from 'getEnemy' searches.
	// Also counts as dead a little ways into the DEAD_DYING animation.
	virtual BOOL IsAlive_FromAI( CBaseMonster* whoWantsToKnow ) { return (pev->deadflag == DEAD_NO || (pev->deadflag == DEAD_DYING && pev->frame < (255*0.3) ) ); }


	virtual BOOL	IsBSPModel( void ) { return pev->solid == SOLID_BSP || pev->movetype == MOVETYPE_PUSHSTEP; }
	virtual BOOL	ReflectGauss( void ) { return ( IsBSPModel() && !pev->takedamage ); }
	virtual BOOL	HasTarget( string_t targetname ) { return FStrEq(STRING(targetname), STRING(pev->targetname) ); }
	virtual BOOL    IsInWorld( void );
	virtual	BOOL	IsPlayer( void ) { return FALSE; }
	virtual BOOL	IsNetClient( void ) { return FALSE; }
	virtual const char *TeamID( void ) { return ""; }


//	virtual void SetActivator( CBaseEntity *pActivator ) {}
	virtual CBaseEntity *GetNextTarget( void );
	
	// fundamental callbacks
	void (CBaseEntity ::*m_pfnThink)(void);
	void (CBaseEntity ::*m_pfnTouch)( CBaseEntity *pOther );
	void (CBaseEntity ::*m_pfnUse)( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void (CBaseEntity ::*m_pfnBlocked)( CBaseEntity *pOther );

	virtual void Think( void ) { if (m_pfnThink) (this->*m_pfnThink)(); }
	


	virtual void Touch( CBaseEntity *pOther ) { if (m_pfnTouch) (this->*m_pfnTouch)( pOther ); }
	virtual void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value ) 
	{ 
		//easyPrintLine("WELL IS IT????? %d %s", m_pfnUse == NULL, pCaller!=NULL?pCaller->getClassname():"blankcaller?");
		if (m_pfnUse) 
			(this->*m_pfnUse)( pActivator, pCaller, useType, value );
	}
	virtual void Blocked( CBaseEntity *pOther ) { if (m_pfnBlocked) (this->*m_pfnBlocked)( pOther ); }

	// allow engine to allocate instance data
    void *operator new( size_t stAllocateBlock, entvars_t *pev )
	{
		return (void *)ALLOC_PRIVATE(ENT(pev), stAllocateBlock);
	}

	// don't use this.
#if _MSC_VER >= 1200 // only build this code if MSVC++ 6.0 or higher
	void operator delete(void *pMem, entvars_t *pev)
	{
		pev->flags |= FL_KILLME;
	}
#endif

	void UpdateOnRemove( void );

	// common member functions
	void EXPORT SUB_Remove( void );
	void EXPORT SUB_DoNothing( void );
	void EXPORT SUB_StartFadeOut ( void );
	void EXPORT SUB_FadeOut ( void );
	void EXPORT SUB_CallUseToggle( void ) { this->Use( this, this, USE_TOGGLE, 0 ); }
	int ShouldToggle( USE_TYPE useType, BOOL currentState );


	//MODDD - new.
	BOOL CheckTracer(const Vector& vecSrc, const Vector& vecEnd, const Vector& vecDirForward, const Vector& vecDirRight, int iBulletType, int iTracerFreq, int* p_tracerCount);

	void FireBullets( ULONG	cShots, Vector  vecSrc, Vector	vecDirShooting,	Vector	vecSpread, float flDistance, int iBulletType, int iTracerFreq = 4, int iDamage = 0, entvars_t *pevAttacker = NULL  );
	Vector FireBulletsPlayer( ULONG	cShots, Vector  vecSrc, Vector	vecDirShooting,	Vector	vecSpread, float flDistance, int iBulletType, int iTracerFreq = 4, int iDamage = 0, entvars_t *pevAttacker = NULL, int shared_rand = 0 );

	virtual CBaseEntity *Respawn( void ) { return NULL; }

	//MODDD - NOTICE!  Identical version here and in CBaseDelay, despite not being virtual.
	// This looks ok as only things within classes call it on themselves
	// (always calls the version that makes the most sense).
	// If there were floaty "CBaseEntity"'s of unknown depth (Delay? Animating? Monster? etc.), 
	// it would be better for this to be virtual to call the deepest version possible.
	// LEAVE IT FOR NOW
	void SUB_UseTargets( CBaseEntity *pActivator, USE_TYPE useType, float value );

	// Do the bounding boxes of these two intersect?
	int Intersects( CBaseEntity *pOther );
	void MakeDormant( void );
	int IsDormant( void );
	BOOL IsLockedByMaster( void ) { return FALSE; }


	//MODDD - no idea why this version didn't exist before.  From SELF.
	//...Oh.  "MyMonsterPointer", nevermind.  This can stay as an alias for MyMonsterPointer.
	CBaseMonster *GetMonsterPointer(void)
	{
		//CBaseEntity *pEntity = Instance( pev );
		//if ( pEntity )
		//	return pEntity->MyMonsterPointer();
		//return NULL;
		return this->MyMonsterPointer();
	}
	//MODDD - and why weren't these variants static anyway?  Nothing about the instance called is involved, may as well be
	static CBaseMonster *GetMonsterPointer( entvars_t *pevMonster ) 
	{
		CBaseEntity *pEntity = Instance( pevMonster );
		if ( pEntity )
			return pEntity->MyMonsterPointer();
		return NULL;
	}
	static CBaseMonster *GetMonsterPointer( edict_t *pentMonster ) 
	{
		CBaseEntity *pEntity = Instance( pentMonster );
		if ( pEntity )
			return pEntity->MyMonsterPointer();
		return NULL;
	}


	// Ugly code to lookup all functions to make sure they are exported when set.
#ifdef _DEBUG
	void FunctionCheck( void *pFunction, char *name ) 
	{
		if (pFunction && !NAME_FOR_FUNCTION((unsigned long)(pFunction)) )
			ALERT( at_error, "No EXPORT: %s:%s (%08lx)\n", STRING(pev->classname), name, (unsigned long)pFunction );
	}

	BASEPTR	ThinkSet( BASEPTR func, char *name ) 
	{
		m_pfnThink = func; 
		FunctionCheck( (void *)*((int *)((char *)this + ( offsetof(CBaseEntity,m_pfnThink)))), name ); 
		return func;
	}
	ENTITYFUNCPTR TouchSet( ENTITYFUNCPTR func, char *name ) 
	{
		m_pfnTouch = func; 
		FunctionCheck( (void *)*((int *)((char *)this + ( offsetof(CBaseEntity,m_pfnTouch)))), name ); 
		return func;
	}
	USEPTR	UseSet( USEPTR func, char *name ) 
	{
		//easyPrintLine("UseSet NAME: %s", name);
		m_pfnUse = func; 
		FunctionCheck( (void *)*((int *)((char *)this + ( offsetof(CBaseEntity,m_pfnUse)))), name ); 
		return func;
	}
	ENTITYFUNCPTR	BlockedSet( ENTITYFUNCPTR func, char *name ) 
	{
		m_pfnBlocked = func; 
		FunctionCheck( (void *)*((int *)((char *)this + ( offsetof(CBaseEntity,m_pfnBlocked)))), name ); 
		return func;
	}

#endif


	// virtual functions used by a few classes
	
	// used by monsters that are created by the MonsterMaker
	virtual	void UpdateOwner( void ) { return; }

	virtual BOOL FBecomeProne( void ) {return FALSE;}
	edict_t *edict() { return ENT( pev ); }
	EOFFSET eoffset( ) { return OFFSET( pev ); }
	int entindex( ) { return ENTINDEX( edict() ); }

	virtual Vector Center( ) { return (pev->absmax + pev->absmin) * 0.5; } // center point of entity
	virtual Vector EyePosition( ) { return pev->origin + pev->view_ofs; }			// position of eyes

	//MODDD - new. Don't add the position in this one.
	//Anything overriding EyePosition should override EyeOffset too.
	virtual Vector EyeOffset( ) { return pev->view_ofs; }			// position of eyes

	virtual Vector EarPosition( ) { return pev->origin + pev->view_ofs; }			// position of ears
	virtual Vector BodyTarget( const Vector &posSrc ) { return Center( ); }		// position to shoot at
	//MODDD - This method was created to mimick BodyTarget (a clone of it most of the time across the rest
	//        of in-game objects that customize it), but for CBasePlayer, it doesn't try to "randomize" the
	//        result (shift vertically, for some reason...).
	virtual Vector BodyTargetMod( const Vector &posSrc ) { return Center( ); }		// position to shoot at

	virtual int Illumination( ) { return GETENTITYILLUM( ENT( pev ) ); }

	//MODDD NOTE - despite these being virtual, they are never implemented elsewhere.  May be for the best.
	//             Adding features to these by other smaller implementable (virtual) methods, like the new
	//             SeeThroughWaterLine, may be best.
	virtual	BOOL FVisible (CBaseEntity *pEntity );
	virtual	BOOL FVisible (const Vector &vecTargetOrigin );
	virtual BOOL FVisible (const Vector& vecLookerOrigin, CBaseEntity *pEntity );
	virtual BOOL FVisible (const Vector& vecLookerOrigin, const Vector &vecTargetOrigin );

	virtual BOOL SeeThroughWaterLine(void);
	virtual void SetGravity(float newGravityVal);

};



///////////////////////////////////////////////////////////////////////////////////////////////////////////
//MODDD - that overload from player.cpp
inline BOOL FNullEnt(CBaseEntity* ent) { return (ent == NULL) || FNullEnt(ent->edict()); }

//MODDD - that new overload to resolve ambiguity.
// This is what happened when FNullEnt was given a EHANDLE before the CBaseEntity*-taking
// and this direct EHANDLE-taking overload were made available:  the BOOL interpetation was 
// used, auto-casting the EHANDLE as a BOOL to fit that FNullEnt overload.
// And that calls for "Get() != NULL", so may as well cut out the middleman (cast)
// and skip to that.  Also the check in the EHANDLE one was for "<some int == 0>", 
// since being NULL returns TRUE and otherwise FALSE (invert the '!= NULL' part).
inline BOOL FNullEnt(EHANDLE someHandle) { return (someHandle.Get() == NULL); }
///////////////////////////////////////////////////////////////////////////////////////////////////////////




class CPointEntity : public CBaseEntity
{
public:
	//MODDD
	CPointEntity();
	void Spawn( void );
	virtual int ObjectCaps( void ) { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
private:
};


typedef struct locksounds			// sounds that doors and buttons make when locked/unlocked
{
	string_t	sLockedSound;		// sound a door makes when it's locked
	string_t	sLockedSentence;	// sentence group played when door is locked
	string_t	sUnlockedSound;		// sound a door makes when it's unlocked
	string_t	sUnlockedSentence;	// sentence group played when door is unlocked

	int	iLockedSentence;		// which sentence in sentence group to play next
	int	iUnlockedSentence;		// which sentence in sentence group to play next

	float flwaitSound;			// time delay between playing consecutive 'locked/unlocked' sounds
	float flwaitSentence;			// time delay between playing consecutive sentences
	BYTE	bEOFLocked;				// true if hit end of list of locked sentences
	BYTE	bEOFUnlocked;			// true if hit end of list of unlocked sentences
} locksound_t;

void PlayLockSounds(entvars_t *pev, locksound_t *pls, int flocked, int fbutton);

//
// MultiSouce
//

#define MAX_MULTI_TARGETS	16 // maximum number of targets a single multi_manager entity may be assigned.
#define MS_MAX_TARGETS 32

class CMultiSource : public CPointEntity
{
public:
	void Spawn( );
	void KeyValue( KeyValueData *pkvd );

	virtual BOOL IsWorldAffiliated(void);

	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	int ObjectCaps( void ) { return (CPointEntity::ObjectCaps() | FCAP_MASTER); }
	BOOL IsTriggered( CBaseEntity *pActivator );
	void EXPORT Register( void );
	virtual int	Save( CSave &save );
	virtual int	Restore( CRestore &restore );

	static	TYPEDESCRIPTION m_SaveData[];

	EHANDLE m_rgEntities[MS_MAX_TARGETS];
	int m_rgTriggered[MS_MAX_TARGETS];

	int m_iTotal;
	string_t m_globalstate;
};


//
// generic Delay entity.
//
class CBaseDelay : public CBaseEntity
{
public:
	float	m_flDelay;
	int		m_iszKillTarget;

	virtual void KeyValue( KeyValueData* pkvd);
	virtual int	Save( CSave &save );
	virtual int	Restore( CRestore &restore );

	//MODDD - new
	void PostRestore(void);
	
	static	TYPEDESCRIPTION m_SaveData[];
	// common member functions
	void SUB_UseTargets( CBaseEntity *pActivator, USE_TYPE useType, float value );
	void EXPORT DelayThink( void );
};




//MODDD - CBaseAnimating class used to be here, moved to its own file animating.h.

//MODDD - CBaseToggle used to be here, moved to its own file basetoggle.h. Implementations still in subs.cpp or wherever else they were.



//MODDD - removed. CBaseMonster's file, basemonster.h, can now be included anywhere else without any assumptions about what's been included by the caller at that point.
//#include "basemonster.h"


//MODDD - CGib moved to its own file, gib.h



//MODDD - CBaseButton used to be here, moved to its own file basebutton.h. Implementations still in buttons.cpp or wherever they were.

//
// Weapons 
//


//
// Converts a entvars_t * to a class pointer
// It will allocate the class and entity if necessary
//
template <class T> T * GetClassPtr( T *a )
{
	entvars_t *pev = (entvars_t *)a;

	// allocate entity if necessary
	if (pev == NULL)
		pev = VARS(CREATE_ENTITY());

	// get the private data
	// same as: 
	//     a = (T*)pev->pContainingEntity->pvPrivateData;
	a = (T *)GET_PRIVATE(ENT(pev));

	if (a == NULL) 
	{
		// allocate private data 
		a = new(pev) T;
		a->pev = pev;
	}
	return a;
}

//MODDD - clone of GetClassPtr that doesn't take any parameters and just makes the new entity.
template <class T> T * CreateEntity(void)
{
	entvars_t* pev;
	T* a;

	// allocate entity
	// 'CREATE_ENTITY()' creates the edict_t*.  
	//pev = VARS(CREATE_ENTITY());
	pev = &(CREATE_ENTITY())->v;

	// get the private data
	//a = (T*)GET_PRIVATE(ENT(pev));

	//if (a == NULL) 
	//{
		// allocate private data 
		// NOTE - same as calling  ALLOC_PRIVATE(ENT(pev), sizeof(T))  .  See the new constructor
		// of CBaseEntity, 'operator new'.
		// Nope!  For reasons science cannot explain, replacing 'new' still misses some needed behavior.
		// Don't do that.  (can get random crashes on accessing some things, mostly methods)
		a = new(pev) T;
		//a = (T*)ALLOC_PRIVATE(pev->pContainingEntity, sizeof(T));

		a->pev = pev;
	//}
	return a;
}








/*
bit_PUSHBRUSH_DATA | bit_TOGGLE_DATA
bit_MONSTER_DATA
bit_DELAY_DATA
bit_TOGGLE_DATA | bit_DELAY_DATA | bit_MONSTER_DATA
bit_PLAYER_DATA | bit_MONSTER_DATA
bit_MONSTER_DATA | CYCLER_DATA
bit_LIGHT_DATA
path_corner_data
bit_MONSTER_DATA | wildcard_data
bit_MONSTER_DATA | bit_GROUP_DATA
boid_flock_data
boid_data
CYCLER_DATA
bit_ITEM_DATA
bit_ITEM_DATA | func_hud_data
bit_TOGGLE_DATA | bit_ITEM_DATA
EOFFSET
env_sound_data
env_sound_data
push_trigger_data
*/

//MODDD - SelAmmo moved to game_shared/util/util_entity.h


// this moved here from world.cpp, to allow classes to be derived from it
//=======================
// CWorld
//
// This spawns first when each level begins.
//=======================
class CWorld : public CBaseEntity
{
public:
	//Must reset these to defaults in case of changing the map.
	float m_fl_node_linktest_height;
	float m_fl_node_hulltest_height;
	BOOL m_f_node_hulltest_heightswap;
	BOOL m_f_map_anyAirNodes;
	BOOL m_f_playerDeadTruce;

	BOOL skyboxEverSet;



	//MODDD - constructor?
	CWorld(void);

	void Activate(void);
	void Spawn( void );
	void Precache( void );
	void KeyValue( KeyValueData *pkvd );


	GENERATE_TRACEATTACK_PROTOTYPE_VIRTUAL
	GENERATE_TAKEDAMAGE_PROTOTYPE_VIRTUAL

	
	//////////////////////////////////////////////////////////////////////////////////
	static TYPEDESCRIPTION m_SaveData[];
	virtual int Save( CSave &save ); 
	virtual int Restore( CRestore &restore );
	//////////////////////////////////////////////////////////////////////////////////
	//void EXPORT WorldThink( void );

	void applyLoadedCustomMapSettingsToGlobal(void);
	void getCustomMapSettingsFromGlobal(void);



	BOOL IsWorld(void);
	BOOL IsWorldAffiliated(void);


};



#endif //END OF #ifndef CBASE_H