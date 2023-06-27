/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/

#ifndef BASEMONSTER_H
#define BASEMONSTER_H


#include "cbase.h"
#include "basetoggle.h"
#include "skill.h"

// monsters.h merged with this file.

// whether unspecified BuildNearestRoute calls start at a random node to look for cover from or use the current start node.
// See notes at the start of BuildNearestRoute for more comments on that.
#define DEFAULT_randomNodeSearchStart TRUE


#define DEFAULT_FORGET_SMALL_FLINCH_TIME 12
#define DEFAULT_FORGET_BIG_FLINCH_TIME 6


//MODDD - new constant for convenience, all other CoverRadius methods should be some multiple of this.
// Default was 784,  boosted to 1000.   Same for some others (Scientist, Panthereye) using 1200 boosted then, since they used to use 1.5 times this instead.
// (now 1.8 times).
#define DEFAULT_COVER_SEEK_DISTANCE 1000


//MODDD - moved here from the since deleted h_ai.cpp.  Not sure why it wasn't always here with the others.
// Never referred to anywhere?  What's the point.
//DLL_GLOBAL BOOL g_fDrawLines = FALSE;


//MODDD
// These were in h_ai.cpp too?  Not some .h file included in some places?
// Not even referred to anywhere.  GOODBYE.
//#define NUM_LATERAL_CHECKS 13  // how many checks are made on each side of a monster looking for lateral cover
//#define NUM_LATERAL_LOS_CHECKS 6  // how many checks are made on each side of a monster looking for lateral cover



#define RANDOMWANDER_TRIES 4


//MODDD - new. How fast this creature floats to the top. Divided by two each time it reaches the top and floats down until it is
//        slow enough to be deemed stationary and kill the think method like most monsters do.
//        This avoids annoying water wade sounds while it rapidly moves between water levels.
// IMPORTANT - If the water level could ever be changed, the think method shouldn't be killed. There can be checks to see if the
//             water level changes after being stationary (float / sink again at full speed), or a timing check to restore the
//             floatSinkSpeed to its full (initial) value if too much time passes without switching.
//             Without this a diffferent waterlevel would leave this creatue stuck where it stopped thinking.
#define WATER_DEAD_SINKSPEED_INITIAL 8








// CHECKLOCALMOVE result types 
#define LOCALMOVE_INVALID					0 // move is not possible
#define LOCALMOVE_INVALID_DONT_TRIANGULATE	1 // move is not possible, don't try to triangulate
#define LOCALMOVE_VALID						2 // move is possible

// Hit Group standards
#define HITGROUP_GENERIC	0
#define HITGROUP_HEAD		1
#define HITGROUP_CHEST		2
#define HITGROUP_STOMACH	3
#define HITGROUP_LEFTARM	4	
#define HITGROUP_RIGHTARM	5
#define HITGROUP_LEFTLEG	6
#define HITGROUP_RIGHTLEG	7


// Monster Spawnflags
#define SF_MONSTER_WAIT_TILL_SEEN		1// spawnflag that makes monsters wait until player can see them before attacking.
#define SF_MONSTER_GAG					2 // no idle noises from this monster
#define SF_MONSTER_HITMONSTERCLIP		4


//FLAG OVERVIEW:
// ALSO, a comment from the as-is script suggests flags at or above 256 (2^8) are 'taken by the engine'.
//     spawn flags 256 and above are already taken by the engine
// ...do with that what you will.  Spawnflag 2^10 looked mostly unused from my testing at least.
// I don't even know if that warning applies to completely new entities placed by the mapper or what flags
// could even get implicitly turned on by the map program or however that works, don't know.
// There is still keyvalue stuff of course, somehow I keep forgetting about that.

// Anyway, a list of flags for seeing what's available for giving to custom entities 
// (like letting stukabats have a preference for starting hanging.  Pick a spawnflag bit that's already for
//  something else in the class heirarchy and you get behavior you don't want also turned on/off by that)
//2^0 = 1:	NO, waitTillSeen
//2^1 = 2:	NO, gag
//2^2 = 4:  NO, HITMONSTERCLIP.
//2^3 = 8:	OPEN!
//2^4 = 16:	NO, PRISONER
//2^5 = 32: NO, squadleader (for non-squad monsters, perhaps open, but also used by turrets)
//2^6 = 64: CAREFUL, BEST NOT
//2^7 = 128: NO
//2^8 = 256: NO, PREDISASTER.  Although what non-talker would use this anyway.
//2^9 = 512: NO, FADECORPSE
//2^10 = 1024: OPEN!
//2^11 = 2048: possibly no.
//beyond?  test to be certain...   some readings, if in the "int" range of 2^0 - 2^31 (inclusive), are still cut high-wards (that is, just plain not sent / transferred to here for seeing)
//No, that's only for network-sent things.

//in short: 2^3 = 8 and 2^10 = 1024 are the best spots for custom flags.



//MODDD - "8" is now for a possible unique entity-only flag.
#define SF_MONSTER_APACHE_CINBOUNDS		1024
//whether the stuka has a strong preference for the ground (off) or the ceiling (on), snapping to its preferred type if close enough.  Will hover in place if not close enough to either.
#define SF_MONSTER_STUKA_ONGROUND		8
//whether a scientist (only, yet) spawns with the dirty skin (alpha models) or not.  off = no, on = yes.
#define SF_MONSTER_TALKMONSTER_BLOODY	8
//causes an HGrunt to not allow itself to be replaced with an HAssault (commando version) at map / level start.
//By default, one hgrunt in a squad is selected to be replaced with an HAssault if no other HAssault is found at mapstart.
#define SF_HGRUNT_DISALLOW_PROMOTION 8
//Used to tell a spawned weapon (snark, chumtoad) not to replace itself with the walkable version. This is also to stop the version spawned by a walkable from wanting to spawn.
#define SF_PICKUP_NOREPLACE 8
//This monster was spawned by a player using a throwable weapon, such as snarks or chumtoads.
#define SF_MONSTER_THROWN 1024
//For tripmines, same bit is unused there.
#define SF_MONSTER_DYNAMIC 1024




// ALSO: it appears this is consistently set for entities spawned by the player (or anything spawned outside of map-start, maybe?). 
// Marking, not sure why it wasn't before.
#define SF_MONSTER_DYNAMICSPAWN			(1<<30)
// Notice! (1<<30) is shared by an existing flag in cbase.h:
//#define SF_NORESPAWN	( 1 << 30 )// !!!set this bit on guns and stuff that should never respawn.
// ...this might be intentional, both are related to spawning.  Maybe it's the same thing.


//NOTE: the spot "32" (2^5) can be occupied by squadmonsters to mean "squadleader" (see "SF_SQUADMONSTER_LEADER" of "squadmonster.cpp").
///////////////////////////////////////////////////////////////////////////////////////


#define SF_MONSTER_PRISONER				16 // monster won't attack anyone, no one will attacke him.
//										32
//										64
#define SF_MONSTER_WAIT_FOR_SCRIPT		128 //spawnflag that makes monsters wait to check for attacking until the script is done or they've been attacked
#define SF_MONSTER_PREDISASTER			256	//this is a predisaster scientist or barney. Influences how they speak.
#define SF_MONSTER_FADECORPSE			512 // Fade out corpse after death

//MODDD - NOTE. This flag does the opposite of what it says. It's presence STOPS a monster from snapping to the ground at creation.
// Lacking the flag snaps a monster to the ground.    What?
//A more accurate name would be "SF_MONSTER_DONT_SNAP_TO_GROUND". By default they do (snap to ground).
//Also this is (1<<31).
#define SF_MONSTER_FALL_TO_GROUND		0x80000000    
                                                      

// specialty spawnflags
#define SF_MONSTER_TURRET_AUTOACTIVATE	32
#define SF_MONSTER_TURRET_STARTINACTIVE	64
#define SF_MONSTER_WAIT_UNTIL_PROVOKED	64 // don't attack the player unless provoked



// MoveToOrigin stuff
#define MOVE_START_TURN_DIST	64 // when this far away from moveGoal, start turning to face next goal
#define MOVE_STUCK_DIST			32 // if a monster can't step this far, it is stuck.


// MoveToOrigin stuff
#define MOVE_NORMAL				0// normal move in the direction monster is facing
#define MOVE_STRAFE				1// moves in direction specified, no matter which way monster is facing





//NOTICE - "monster to monster" relationship constnats moved to cbase.h because base entities have some relationship-related methods.

// these bits represent the monster's memory
#define MEMORY_CLEAR					0
#define bits_MEMORY_PROVOKED			( 1 << 0 )// right now only used for houndeyes.
#define bits_MEMORY_INCOVER				( 1 << 1 )// monster knows it is in a covered position.
#define bits_MEMORY_SUSPICIOUS			( 1 << 2 )// Ally is suspicious of the player, and will move to provoked more easily
#define bits_MEMORY_PATH_FINISHED		( 1 << 3 )// Finished monster path (just used by big momma for now)
#define bits_MEMORY_ON_PATH				( 1 << 4 )// Moving on a path
#define bits_MEMORY_MOVE_FAILED			( 1 << 5 )// Movement has already failed
#define bits_MEMORY_FLINCHED			( 1 << 6 )// Has already flinched
#define bits_MEMORY_KILLED				( 1 << 7 )// HACKHACK -- remember that I've already called my Killed()

//#define bits_MEMORY_BIG_FLINCHED			( 1 << 8 )// MODDD - new. Has already big flinched
//...nevermind, also cut.  Just check "forgetBigFlinchTime".  Laziness.

//#define bits_MEMORY_COVER_RECENTFAIL	( 1 << 8) //MODDD - cut for now. New memory flag to signify that a recent request for cover has failed and not to try that again.

#define bits_MEMORY_CUSTOM4				( 1 << 28 )	// Monster-specific memory
#define bits_MEMORY_CUSTOM3				( 1 << 29 )	// Monster-specific memory
#define bits_MEMORY_CUSTOM2				( 1 << 30 )	// Monster-specific memory
#define bits_MEMORY_CUSTOM1				( 1 << 31 )	// Monster-specific memory




// NOTE: tweak these values based on gameplay feedback:
//(MODDD - that comment is not mine.)


//MODD - old?
/*
#define PARALYZE_DURATION	2		// number of 2 second intervals to take damage
#define PARALYZE_DAMAGE		1.0		// damage to take each 2 second interval

#define NERVEGAS_DURATION	2
#define NERVEGAS_DAMAGE		5.0

#define POISON_DURATION		5
#define POISON_DAMAGE		2.0

#define RADIATION_DURATION	2
#define RADIATION_DAMAGE	1.0

#define ACID_DURATION		2
#define ACID_DAMAGE			5.0

#define SLOWBURN_DURATION	2
#define SLOWBURN_DAMAGE		1.0

#define SLOWFREEZE_DURATION	2
#define SLOWFREEZE_DAMAGE	1.0
*/


#define PARALYZE_DAMAGE		gSkillData.tdmg_paralyze_damage

#define NERVEGAS_DAMAGE		gSkillData.tdmg_nervegas_damage

#define POISON_DAMAGE		gSkillData.tdmg_poison_damage

#define RADIATION_DAMAGE	gSkillData.tdmg_radiation_damage

#define ACID_DAMAGE			gSkillData.tdmg_acid_damage

#define SLOWBURN_DAMAGE		gSkillData.tdmg_slowburn_damage

#define SLOWFREEZE_DAMAGE	gSkillData.tdmg_slowfreeze_damage

#define BLEEDING_DAMAGE		gSkillData.tdmg_bleeding_damage



//MODDD - prototypes for h_ai.cpp methods (FBoxVisible, VecCheckToss, VecCheckThrow) moved to util.h.
// Implementations too (util.cpp) since that file got deleted.
extern DLL_GLOBAL Vector g_vecAttackDir;
extern DLL_GLOBAL CONSTANT float g_flMeleeRange;
extern DLL_GLOBAL CONSTANT float g_flMediumRange;
extern DLL_GLOBAL CONSTANT float g_flLongRange;




extern void UTIL_MoveToOrigin(edict_t* pent, const Vector& vecGoal, float flDist, int iMoveType);




// trigger conditions for scripted AI
// these MUST match the CHOICES interface in halflife.fgd for the base monster
enum 
{
	AITRIGGER_NONE = 0,
	AITRIGGER_SEEPLAYER_ANGRY_AT_PLAYER,
	AITRIGGER_TAKEDAMAGE,
	AITRIGGER_HALFHEALTH,
	AITRIGGER_DEATH,
	AITRIGGER_SQUADMEMBERDIE,
	AITRIGGER_SQUADLEADERDIE,
	AITRIGGER_HEARWORLD,
	AITRIGGER_HEARPLAYER,
	AITRIGGER_HEARCOMBAT,
	AITRIGGER_SEEPLAYER_UNCONDITIONAL,
	AITRIGGER_SEEPLAYER_NOT_IN_COMBAT,
};
/*
		0 : "No Trigger"
		1 : "See Player"
		2 : "Take Damage"
		3 : "50% Health Remaining"
		4 : "Death"
		5 : "Squad Member Dead"
		6 : "Squad Leader Dead"
		7 : "Hear World"
		8 : "Hear Player"
		9 : "Hear Combat"
*/


//MODDD - moved outside the class, unsure why it was inside to begin with
typedef enum
{
	SCRIPT_PLAYING = 0,		// Playing the sequence
	SCRIPT_WAIT,				// Waiting on everyone in the script to be ready
	SCRIPT_CLEANUP,					// Cancelling the script / cleaning up
	SCRIPT_WALK_TO_MARK,
	SCRIPT_RUN_TO_MARK,
} SCRIPTSTATE;



// an array of waypoints makes up the monster's route. 
struct WayPoint_t
{
	Vector vecLocation;
	int iType;
	//MODDD - NEW.  Record the map node I came from, if coming from a node-route.
	// (-1 otherwise)
	int iMapNodeIndex;
};


//
// generic Monster
//
class CBaseMonster : public CBaseToggle
{
public:
	int m_afConditions;
	//MODDD - for being set by methods that often occur after ai has been run this frame (like touch methods, or even during schedules).
	// Set at the start of the next frame and cleared.
	int m_afConditionsNextFrame;

	//MODDD - new conditions bitmask.  Sweet mother.
	int m_afConditionsMod;
	int m_afConditionsModNextFrame;



	//MODDD - moved outside iRelationship.
	static int iEnemy[14][14];
	static int iEnemyTruce[14][14];

	//MODDD - new, for all monsters. Because the zombie ones are reused so much.
	static const char *pStandardAttackHitSounds[];
	static const char *pStandardAttackMissSounds[];


	static Schedule_t* m_scheduleList[];



	// these fields have been added in the process of reworking the state machine. (sjb)
	EHANDLE				m_hEnemy;		 // the entity that the monster is fighting.
	EHANDLE				m_hTargetEnt;	 // the entity that the monster is trying to reach

	EHANDLE				m_hOldEnemy[MAX_OLD_ENEMIES];
	Vector				m_vecOldEnemy[MAX_OLD_ENEMIES];
	// NEW.  Have the zoffsets too.  And VIEWOFS's.  Although should those just be stored as floats?
	float				m_flOldEnemy_zOffset[MAX_OLD_ENEMIES];
	Vector				m_vecOldEnemy_ViewOFS[MAX_OLD_ENEMIES];

	// It shall use the stack!
	// Use this variable to record the most recent addition to take instead.
	int m_intOldEnemyNextIndex;



	float			m_flFieldOfView;// width of monster's field of view ( dot product )
	float			m_flWaitFinished;// if we're told to wait, this is the time that the wait will be over.
	float			m_flMoveWaitFinished;

	Activity			m_Activity;// what the monster is doing (animation)
	Activity			m_IdealActivity;// monster should switch to this activity

	int				m_LastHitGroup; // the last body region that took damage

	MONSTERSTATE		m_MonsterState;// monster's current state
	MONSTERSTATE		m_IdealMonsterState;// monster should change to this state

	int				m_iTaskStatus;
	Schedule_t* m_pSchedule;
	int				m_iScheduleIndex;

	WayPoint_t		m_Route[ROUTE_SIZE];	// Positions of movement
	int				m_movementGoal;			// Goal that defines route
	int				m_iRouteIndex;			// index into m_Route[]
	//MODDD - and why wasn't the number of nodes in the current route ever recorded to a var?
	// Idea is, there might not be a guarantee that the current route has a marked 'goal' node (although it should),
	// and there may be garbage memory left over from earlier routes larger than the current one (parts of m_Route not
	// zero'd out that are leftover from earlier runs).  Running into those would fool us into thinking this route has
	// a goal node, when it's really from an unrelated earlier route.
	// Record the length of the current route instead (given at the time the route is built/determined or ever given/loses
	// nodes), which is likely to be under ROUTE_SIZE. 
	// If, while looking for the goal node, the m_iRouteLength is exceeded, we'll assume the last note in the route we
	// were actually given (m_iRouteLength-1) is the 'goal' node.
	int m_iRouteLength;

	float			m_moveWaitTime;			// How long I should wait for something to move

	Vector				m_vecMoveGoal; // kept around for node graph moves, so we know our ultimate goal
	Activity			m_movementActivity;	// When moving, set this activity

	int				m_iAudibleList; // first index of a linked list of sounds that the monster can hear.
	int				m_afSoundTypes;

	Vector				m_vecLastPosition;// monster sometimes wants to return to where it started after an operation.

	int				m_iHintNode; // this is the hint node that the monster is moving towards or performing active idle on.

	int				m_afMemory;

	int				m_iMaxHealth;// keeps track of monster's maximum health value (for re-healing, etc)

	Vector			m_vecEnemyLKP;// last known position of enemy. (enemy's origin)

	//MODDD - NEW.  Good to keep in mind for pathfinding, easier to get the bottom of the entity's bounds fast.
	// (it will be the pev->mins.z; add it to the LKP to get the location of the bottom bound, consistently
	// gets the bottom bound of players too)
	float m_flEnemyLKP_zOffset;
	Vector m_vecEnemyLKP_ViewOFS;  //record they eyepos too...

	// NEW.  Has m_vecEnemyLKP ever set before?  If not, don't try to use it (defalts to origin of the map, an odd place
	// to look at for no apparent reason).
	BOOL m_fEnemyLKP_EverSet;

	int				m_cAmmoLoaded;		// how much ammo is in the weapon (used to trigger reload anim sequences)

	int				m_afCapability;// tells us what a monster can/can't do.

	//MODDD NOTE - interestingly enough, CBaseMonster methods barely, if ever, involve 
	// m_flNextAttack at all.  It's dormant in most monsters.  Odd.
	// In fact, REMOVED.  Classes that ever needed this (mostly, if not only CBasePlayer)
	// should just include it themselves.
	// Even the ISlave defined m_flNextAttack itself.
	//float			m_flNextAttack;		// cannot attack again until this time

	int				m_bitsDamageType;	// what types of damage has monster (player) taken
	//MODDD - complementary.
	int				m_bitsDamageTypeMod;

	float			m_tbdPrev;				// Time-based damage timer
	BYTE			m_rgbTimeBasedDamage[itbd_COUNT];
	BOOL			m_rgbTimeBasedFirstFrame[itbd_COUNT];

	//MODDD - var is actually used for all monsters now.  And why was it int-type anyway?
	// NOTICE - only use for determining how much damage the killing blow did against this monster,
	// regardless of health at the time.  An attack that did 50 damage, even with 49 health left,
	// will still result in the violent death activity.
	float			m_lastDamageAmount;// how much damage did monster last take

	int				m_bloodColor;		// color of blood particless

	int				m_failSchedule;				// Schedule type to choose if current schedule fails

	float			m_flHungryTime;// set this is a future time to stop the monster from eating for a while. 

	float			m_flDistTooFar;	// if enemy farther away than this, bits_COND_ENEMY_TOOFAR set in CheckEnemy
	float			m_flDistLook;	// distance monster sees (Default 2048)

	int				m_iTriggerCondition;// for scripted AI, this is the condition that will cause the activation of the monster's TriggerTarget
	string_t			m_iszTriggerTarget;// name of target that should be fired. 

	Vector				m_HackedGunPos;	// HACK until we can query end of gun

// Scripted sequence Info
	SCRIPTSTATE m_scriptState;		// internal cinematic state
	CCineMonster* m_pCine;




	float nextDirectRouteAttemptTime;

	//new
	BOOL disableEnemyAutoNode;
	BOOL waitForMovementTimed_Start;

	BOOL investigatingAltLKP;

	Vector m_vecEnemyLKP_Real;
	float m_flEnemyLKP_Real_zOffset;
	Vector m_vecEnemyLKP_Real_ViewOFS;
	BOOL m_fEnemyLKP_Real_EverSet;

	// Should these be saved?
	Vector m_vecEnemyLKP_prev;
	float m_flEnemyLKP_prev_zOffset;
	Vector m_vecEnemyLKP_prev_ViewOFS;
	BOOL m_fEnemyLKP_prev_EverSet;



	BOOL canSetAnim;
	BOOL m_fNewScheduleThisFrame;

	BOOL canDrawDebugSurface;

	BOOL signalActivityUpdate;

	static int monsterIDLatest;
	int monsterID;



	//MODDD - new
	BOOL hardSetFailSchedule;
	BOOL scheduleSurvivesStateChange;

	//MODDD - canned.
	//BOOL usingGermanModel;

	//MODDD - new. float / sink speed to get even at the surface.
	//Used by a schedule possibly.
	int oldWaterLevel;
	float floatSinkSpeed;
	float floatEndTimer;


	//MODDD - okay
	Vector debugVector1;
	Vector debugVector2;
	Vector debugVector3;
	Vector debugVector4;
	BOOL debugVectorsSet;
	BOOL debugFailColor;
	BOOL debugVectorPrePass;
	int debugVectorMode;

	BOOL forceNoDrop;

	BOOL fApplyTempVelocity;
	Vector velocityApplyTemp;

	BOOL drawPathConstant;
	BOOL drawFieldOfVisionConstant;

	BOOL dummyAI;


	//MODDD - new, pertaining to "takeDamage".
	BOOL blockDamage;
	BOOL buddhaMode;
	BOOL blockTimedDamage;


	MONSTERSTATE queuedMonsterState;

	float forgetSmallFlinchTime;
	float forgetBigFlinchTime;


	BOOL strictNodeTolerance;
	float goalDistTolerance;
	BOOL recentTimedTriggerDamage;
	float recentMoveExecuteFailureCooldown;

	Vector respawn_origin;
	Vector respawn_angles;




	//MODDD - new
	CBaseMonster(void);

	virtual BOOL usesSoundSentenceSave(void);


	//void smartResize();
	virtual BOOL getHasPathFindingMod();
	virtual BOOL getHasPathFindingModA();
	//this is okay to be virtual, yes?
	virtual BOOL NoFriendlyFireImp(const Vector& startVec, const Vector& endVec);
	virtual BOOL forceIdleFrameReset(void);

	virtual void onNewRouteNode(void);
		
	virtual void setPhysicalHitboxForDeath(void);
	virtual void DeathAnimationStart(void);
	virtual void DeathAnimationEnd(void);
	virtual void onDeathAnimationEnd(void);

	virtual int LookupActivityFiltered(int NewAcitivty);
	virtual int LookupActivity(int NewActivity);
	virtual int LookupActivityHeaviest(int NewActivity);

	virtual void OnTakeDamageSetConditions(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType, int bitsDamageTypeMod);


	virtual void setAnimationSmart(const char* arg_animName);
	virtual void setAnimationSmart(const char* arg_animName, float arg_frameRate);
	virtual void setAnimationSmart(int arg_animIndex, float arg_frameRate);
	virtual void setAnimationSmartAndStop(const char* arg_animName);
	virtual void setAnimationSmartAndStop(const char* arg_animName, float arg_frameRate);
	virtual void setAnimationSmartAndStop(int arg_animIndex, float arg_frameRate);
		
	virtual BOOL usesAdvancedAnimSystem(void);

	void setAnimation(char* animationName);
	void setAnimation(char* animationName, BOOL forceException);
	void setAnimation(char* animationName, BOOL forceException, BOOL forceLoopsProperty);
	void setAnimation(char* animationName, BOOL forceException, BOOL forceLoopsProperty, int extraLogic);

	virtual BOOL isSizeGiant(void);
	virtual BOOL isOrganic(void);
	virtual BOOL isOrganicLogic(void);

	virtual float getBarnaclePulledTopOffset(void);

	virtual float getBarnacleForwardOffset(void);
	virtual float getBarnacleAnimationFactor(void);



	//MODDD - new var
	virtual BOOL hasSeeEnemyFix(void);
	virtual BOOL getForceAllowNewEnemy(CBaseEntity* pOther);

	virtual BOOL needsMovementBoundFix(void);
	virtual void cheapKilled(void);
	virtual void cheapKilledFlyer(void);
	virtual void OnKilledSetTouch(void);

	EXPORT void KilledFinishTouch( CBaseEntity *pOther );

	virtual int getLoopingDeathSequence(void);
	Schedule_t* flyerDeathSchedule(void);
	virtual BOOL getMovementCanAutoTurn(void);

	void updateEnemyLKP(void);
	void setEnemyLKP(const Vector& argNewVector, float zOffset);
	void setEnemyLKP(const Vector& argNewVector, float zOffset, const Vector& extraAddIn);
	virtual void setEnemyLKP(CBaseEntity* theEnt);
	virtual void setEnemyLKP(CBaseEntity* theEnt, const Vector& extraAddIn);
	void setEnemyLKP(entvars_t* theEntPEV);
	void setEnemyLKP(entvars_t* theEntPEV, const Vector& extraAddIn);
	void setEnemyLKP_Investigate(const Vector& argNewVector);

	
	virtual CBaseEntity* getNearestDeadBody(const Vector& arg_searchOrigin, const float arg_maxDist);

	virtual BOOL noncombat_Look_ignores_PVS_check(void);
	virtual BOOL bypassAllowMonstersSpawnCheck(void);

	virtual BOOL violentDeathAllowed(void);
	virtual BOOL violentDeathDamageRequirement(void);
	virtual BOOL violentDeathClear(void);
	virtual int violentDeathPriority(void);

	//not meant to be implemented in other places but in case it happens, I trust whoever else knows what they're doing.
	virtual BOOL violentDeathClear_BackwardsCheck(float argDistance);

	void lookAtEnemyLKP(void);
	void predictActRepeat(int arg_bits_cond);

	virtual BOOL canPredictActRepeat(void);

	//could be overridden but probably won't be.  Give it a customizable distance to check if that's the problem.
	virtual CBaseEntity* HumanKick(void);
	virtual CBaseEntity* HumanKick(float argCheckDistance);

	//Never used by the base monster class. Must be called by a given monster, and can be implemented there to be more specific or custom if necessary.
	virtual void precacheStandardMeleeAttackMissSounds(void);
	virtual void precacheStandardMeleeAttackHitSounds(void);
	virtual void playStandardMeleeAttackMissSound(void);
	virtual void playStandardMeleeAttackHitSound(void);
	virtual void playStandardMeleeAttackMetalHitSound(void);
	void playStandardMeleeAttackHitSound(CBaseEntity* hitEnt, const char** normalHitSounds, int normalHitSoundsSize, float vol, float attn, int pitchMin, int pitchMax);

	void determineStandardMeleeAttackHitSound(CBaseEntity* hitEnt);



	//don't override this.  it's a general utility that pops up in a lot of places.  Just use custom code if any part of it needs changes.
	BOOL traceResultObstructionValidForAttack(const TraceResult& arg_tr);

	virtual float getDistTooFar(void);
	virtual float getDistLook(void);

	virtual BOOL getGermanModelRequirement(void);
	virtual const char* getGermanModel(void);
	virtual const char* getNormalModel(void);
	virtual void Activate(void);
	virtual void Spawn(void);


	//MODDD
	virtual BOOL skipSpawnStuckCheck(void){return FALSE;}


	virtual float TimedDamageBuddhaFilter(float dmgIntent);
	virtual void TimedDamagePostBuddhaCheck(void);
	int convert_itbd_to_damage(int i);
	virtual void removeTimedDamage(int arg_type, int* m_bitsDamageTypeRef);
	virtual void removeTimedDamageImmediate(int arg_type, int* m_bitsDamageTypeRef, BYTE bDuration);
	virtual BYTE parse_itbd_duration(int i);
	virtual void parse_itbd(int i);
	virtual void timedDamage_nonFirstFrame(int i, int* m_bitsDamageTypeRef);
	virtual void CheckTimeBasedDamage(void);
	//void PreThink(void);
	//virtual void Think(void);

	void setTimedDamageDuration(int i, int* m_bitsDamageTypeRef);
	void attemptResetTimedDamage(BOOL forceReset);
	void applyNewTimedDamage(int arg_bitsDamageType, int arg_bitsDamageTypeMod);
	

	static TYPEDESCRIPTION m_SaveData[];
	virtual int	Save( CSave &save ); 
	virtual int	Restore( CRestore &restore );


	//MODDD - new
	void PostRestore(void);
	

	void KeyValue( KeyValueData *pkvd );

// monster use function
	void EXPORT MonsterUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	// Never implemented, as-is?  Whoops.
	//void EXPORT CorpseUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

// overrideable Monster member functions
	
	virtual int BloodColor( void ) { return m_bloodColor; }

	virtual CBaseMonster *MyMonsterPointer( void ) { return this; }
	virtual void Look ( float flDistance );// basic sight function for monsters
	virtual void RunAI ( void );// core ai function!	
	void Listen ( void );


	//MODDD - changed, made more strict. Any problems with this or scripted ent's? Check Nihilanth.
	//virtual BOOL	IsAlive( void ) { return (pev->deadflag != DEAD_DEAD); }
	virtual BOOL IsAlive( void ) { return (pev->deadflag == DEAD_NO); }

	//MODD - NEW.  Might want some things (chumtoads) to have the ability to fool other monsters into thinking they are dead and be ignored from 'getEnemy' searches.
	// Also counts as dead a little ways into the DEAD_DYING animation.
	// Why does this have a check for 'pev->health > 0' ?    Removed!  I don't see the point.
	//virtual BOOL IsAlive_FromAI( CBaseMonster* whoWantsToKnow ) { return (pev->deadflag == DEAD_NO || (pev->deadflag == DEAD_DYING && pev->frame < (255*0.3) ) ) && pev->health > 0; }
	virtual BOOL IsAlive_FromAI( CBaseMonster* whoWantsToKnow ) { return (pev->deadflag == DEAD_NO || (pev->deadflag == DEAD_DYING && pev->frame < (255*0.3) ) ); }

	virtual BOOL ShouldFadeOnDeath( void );

// Basic Monster AI functions
	virtual float ChangeYaw ( int speed );
	float VecToYaw( Vector vecDir );
	float FlYawDiff ( void ); 

	float DamageForce( float damage );

	

	// Used to call the virtual "MonsterThink" which monsters may implement
	// for custom behavior.  Advisable to call the parent CBaseMonster
	// MonsterThink though.
	void EXPORT	CallMonsterThink( void );
	
// stuff written for new state machine
//MODDD NOTE - ...The meaning of the above comment shall forever be lost to time.
	virtual void MonsterThink( void );

	virtual void heardBulletHit(entvars_t* pevShooter);
	virtual void wanderAway(const Vector& toWalkAwayFrom);
	

	virtual int IRelationship ( CBaseEntity *pTarget );

	//MODDD - new. Sometimes a monster may want to know what a certain class would think of another target, instead of its own class.
	//Nah, canned.
	//static int IRelationshipOfClass(int argClassifyValue, CBaseEntity* pTarget);


	virtual void MonsterInit ( void );
	virtual void MonsterInitDead( void );	// Call after animation/pose is set up
	virtual void BecomeDead( void );
	void EXPORT CorpseFallThink( void );

	void EXPORT MonsterInitThink ( void );
	virtual void StartMonster ( void );
	virtual CBaseEntity* BestVisibleEnemy ( void );// finds best visible enemy for attack
	virtual BOOL FInViewCone ( CBaseEntity *pEntity );// see if pEntity is in monster's view cone
	virtual BOOL FInViewCone ( Vector *pOrigin );// see if given location is in monster's view cone
	virtual void HandleAnimEvent( MonsterEvent_t *pEvent );


	virtual int CheckLocalMove ( const Vector &vecStart, const Vector &vecEnd, CBaseEntity *pTarget, BOOL doZCheck, float *pflDist );// check validity of a straight move through space
	virtual int CheckLocalMoveHull ( const Vector &vecStart, const Vector &vecEnd, CBaseEntity *pTarget, float *pflDist );// check validity of a straight move through space

	virtual BOOL ZCheck(const Vector& vecPosition, const Vector& vecEnd);
	virtual BOOL ZCheck_Ground(const Vector& vecPosition, const Vector& vecEnd);
	virtual BOOL ZCheck_Flyer(const Vector& vecPosition, const Vector& vecEnd);

	
	virtual BOOL usesSegmentedMove(void);
	virtual int MovePRE(float flInterval, float& flWaypointDist, float& flCheckDist, float& flDist, Vector& vecDir, CBaseEntity*& pTargetEnt );
	virtual void Move( float flInterval = 0.1 );

	virtual void MoveExecute( CBaseEntity *pTargetEnt, const Vector &vecDir, float flInterval );
	virtual BOOL ShouldAdvanceRoute( float flWaypointDist, float flInterval );
	virtual BOOL CheckPreMove(void);


	virtual Activity GetStoppedActivity( void ) { return ACT_IDLE; }
	virtual void Stop( void ) { m_IdealActivity = GetStoppedActivity(); }

	// This will stop animation until you call ResetSequenceInfo() at some point in the future
	inline void StopAnimation( void ) { pev->framerate = 0; }

	// these functions will survey conditions and set appropriate conditions bits for attack types.
	virtual BOOL CheckRangeAttack1( float flDot, float flDist );
	virtual BOOL CheckRangeAttack2( float flDot, float flDist );
	virtual BOOL CheckMeleeAttack1( float flDot, float flDist );
	virtual BOOL CheckMeleeAttack2( float flDot, float flDist );



	BOOL FHaveSchedule( void );
	BOOL FScheduleValid ( void );
	void ClearSchedule( void );
	BOOL FScheduleDone ( void );
	void ChangeSchedule ( Schedule_t *pNewSchedule );
	void NextScheduledTask ( void );
	Schedule_t *ScheduleInList( const char *pName, Schedule_t **pList, int listCount );

	virtual Schedule_t *ScheduleFromName( const char *pName );

	void MaintainSchedule ( void );
	virtual void StartTask ( Task_t *pTask );
	virtual void RunTask ( Task_t *pTask );
	virtual Schedule_t *GetScheduleOfType( int Type );
	virtual Schedule_t *GetSchedule( void );
	
	virtual void ScheduleChange( void ); //MODDD - a little default behavior now, see defaultai.cpp.
	virtual Schedule_t* GetStumpedWaitSchedule(void);
	// virtual int CanPlaySequence( void ) { return ((m_pCine == NULL) && (m_MonsterState == MONSTERSTATE_NONE || m_MonsterState == MONSTERSTATE_IDLE || m_IdealMonsterState == MONSTERSTATE_IDLE)); }
	virtual int CanPlaySentence(BOOL fDisregardState);
	virtual int CanPlaySequence( BOOL fDisregardState, int interruptLevel );
	virtual void PlaySentence( const char *pszSentence, float duration, float volume, float attenuation );
	virtual void PlayScriptedSentence( const char *pszSentence, float duration, float volume, float attenuation, BOOL bConcurrent, CBaseEntity *pListener );

	virtual void SentenceStop( void );


	Task_t *GetTask ( void );
	virtual MONSTERSTATE GetIdealState ( void );
	

	//MODDD 
	int getTaskNumber(void);
	const char* getScheduleName(void);

	virtual void SetActivity ( Activity NewActivity );
	virtual void RefreshActivity(void);
	//MODDD
	//virtual void SetActivity ( Activity NewActivity, BOOL forceReset );
	

	virtual BOOL allowedToSetActivity(void);
	BOOL tryGetTaskID(void);
	const char* tryGetScheduleName(void);

	//MODDD - edited.
	void SetSequenceByIndex(int iSequence);
	void SetSequenceByName(char* szSequence);
	void SetSequenceByIndex(int iSequence, float flFramerateMulti);
	void SetSequenceByName(char* szSequence, float flFramerateMulti);
	void SetSequenceByIndex(int iSequence, BOOL safeReset);
	void SetSequenceByName(char* szSequence, BOOL safeReset);
	void SetSequenceByIndex(int iSequence, float flFramerateMulti, BOOL safeReset);
	void SetSequenceByName(char* szSequence, float flFramerateMulti, BOOL safeReset);

	void SetSequenceByIndexForceLoops(int iSequence, BOOL forceLoops);
	void SetSequenceByNameForceLoops(char* szSequence, BOOL forceLoops);
	void SetSequenceByIndexForceLoops(int iSequence, float flFramerateMulti, BOOL forceLoops);
	void SetSequenceByNameForceLoops(char* szSequence, float flFramerateMulti, BOOL forceLoops);
	void SetSequenceByIndexForceLoops(int iSequence, BOOL safeReset, BOOL forceLoops);
	void SetSequenceByNameForceLoops(char* szSequence, BOOL safeReset, BOOL forceLoops);
	void SetSequenceByIndexForceLoops(int iSequence, float flFramerateMulti, BOOL safeReset, BOOL forceLoops);
	void SetSequenceByNameForceLoops(char* szSequence, float flFramerateMulti, BOOL safeReset, BOOL forceLoops);


	void SetState ( MONSTERSTATE State );
	
	//MODDD - new.
	void reportNetName(void);

	virtual void ReportAIState( void );

	void CheckAttacks ( CBaseEntity *pTarget, float flDist );
	virtual int CheckEnemy ( CBaseEntity *pEnemy );

	//MODDD - helper
	void refreshStack(void);

	void PushEnemy( CBaseEntity *pEnemy, Vector &vecLastKnownPos );
	BOOL PopEnemy( void );
	void DrawFieldOfVision(void);

	BOOL FGetNodeRoute(Vector vecStart, Vector vecDest );
	BOOL FGetNodeRoute(Vector vecStart, Vector vecDest, BOOL asIfSnappedToGround);
	BOOL FGetNodeRoute_Final(Vector vecStart, Vector vecDest, BOOL asIfSnappedToGround, int iSrcNode, int iDestNode, int iNodeTypeInfo);
	BOOL FVerifyRoute(Vector vecStart, Vector vecDest, int iPath[], int iPathSize);
	
	//MODDD - not inline as of now.  I need my breakpoints.
	//inline
	void TaskComplete(void);
	void MovementComplete( void );
	

	//For now not inline, harder to debug inlines (use break points).  Also moved to basemonster.cpp, easier to test with breakpoints, disabling + recompiling, etc.
	//inline
	void TaskFail( void );


	inline void TaskBegin( void ) { m_iTaskStatus = TASKSTATUS_RUNNING; }
	int TaskIsRunning( void );
	inline int TaskIsComplete( void ) { return (m_iTaskStatus == TASKSTATUS_COMPLETE); }


	//MODDD - no longer inline, safety, for now at least..
	// Want to know if checking for 'node.iType == 0' like 'FRouteClear' checks for would lead to any
	// different behavior.
	int MovementIsComplete( void ) {
		//return (m_movementGoal == MOVEGOAL_NONE);
		//return FRouteClear();

		/*
		if ( m_Route[ m_iRouteIndex ].iType == 0 || m_movementGoal == MOVEGOAL_NONE ){
			if(m_Route[ m_iRouteIndex ].iType == 0 && m_movementGoal != MOVEGOAL_NONE){
				// let me know!
				easyForcePrintLine("!!! %s:%d NOTICE!  current node type is 0, yet the movegoal is not NONE (is %d)", getClassname(), monsterID, m_movementGoal);
			}
			return TRUE;
		}
		*/

		// NOTICE - several ways of clearing the movegoal don't advance m_iRouteIndex, so it may very well be
		// at one less than the route length with MOVEGOAL_NONE.
		// Even so, don't make assumptions based off of m_iRouteIndex.  Being one less than m_iRouteLength could still
		// mean it's in progress.  Probably don't even need to count it here at all.
		if ( m_iRouteIndex >= m_iRouteLength || m_movementGoal == MOVEGOAL_NONE ){
			if(m_iRouteIndex >= m_iRouteLength && m_movementGoal != MOVEGOAL_NONE){
				// let me know!
				easyPrintLine("!!! ROUTE DEBUG %s:%d NOTICE!  Place in route exceeded RouteLength, yet the movegoal is not NONE (is %d)", getClassname(), monsterID, m_movementGoal);
			}
			return TRUE;
		}
		
		return FALSE;
	}

	int IScheduleFlags ( void );
	//MODDD - made virtual.
	virtual BOOL FRefreshRouteStrict( void );
	virtual BOOL FRefreshRoute(void);
	virtual BOOL FRefreshRouteCheap( void );
	virtual BOOL FRefreshRouteChaseEnemySmart(void);
	virtual BOOL FRefreshRouteChaseEnemySmartSafe(void);
	

	BOOL FRouteClear ( void );

	void RouteSimplify( CBaseEntity *pTargetEnt );

	//MODDD - easier to access.
	static void DrawRoute( entvars_t *pev, WayPoint_t *m_Route, int m_iRouteLength, int m_iRouteIndex, int r, int g, int b );
	// And now a non-static version that uses member variables whenever possible.  Imagine that.
	void DrawMyRoute(int r, int g, int b );

	void AdvanceRoute ( float distance, float flInterval );
	virtual BOOL FTriangulate ( const Vector &vecStart , const Vector &vecEnd, float flDist, CBaseEntity *pTargetEnt, Vector *pApex );
	BOOL FPutFailedGoalAlongSurface(Vector vecFailedGoal, float flLookZOffset, float flAlongSurfaceDist, CBaseEntity* pTargetEnt,  Vector* out_vecNewTarget);

	//MODDD - made virtual.
	virtual void MakeIdealYaw( Vector vecTarget );

	virtual void SetYawSpeed ( void ) { return; }// allows different yaw_speeds for each activity
	

	virtual BOOL attemptRampFix(const Vector &vecGoal, int iMoveFlag, CBaseEntity *pTarget);

	//MODDD - made virtual like BuildNearestRoute was. If something overrides this we should probably use its version.
	virtual BOOL BuildRoute ( const Vector &vecGoal, int iMoveFlag, CBaseEntity *pTarget );

	//MODDD - now accepts an optional moveflag and optional target entity just like BuildRoute does. To whatever route it makes with BuildRoute,
	//        it should be able to pass that information along just as if BuildRoute were called plainly.
	//        But calling BuildNearestRoute without those things is possible too for pure retail behavior.
	virtual BOOL BuildNearestRoute ( Vector vecThreat, Vector vecViewOffset, float flMinDist, float flMaxDist, BOOL randomNodeSearchStart );
	virtual BOOL BuildNearestRoute ( Vector vecThreat, Vector vecViewOffset, float flMinDist, float flMaxDist, BOOL randomNodeSearchStart, int iMoveFlag, CBaseEntity* pTarget);



	int RouteClassify( int iMoveFlag );
	int MovementGoalToMoveFlag(int iMoveGoal);
	void InsertWaypoint ( Vector vecLocation, int afMoveFlags );
	
	BOOL FindLateralCover ( const Vector &vecThreat, const Vector &vecViewOffset );
	virtual BOOL FindCover ( Vector vecThreat, Vector vecViewOffset, float flMinDist, float flMaxDist );
	virtual BOOL FindRandom ( Activity movementAct, Vector vecThreat, Vector vecViewOffset, float flMinDist, float flMaxDist );
	virtual BOOL SCHEDULE_attemptFindCoverFromEnemy(Task_t* pTask);
	virtual BOOL FValidateCover(const Vector& vecCoverLocation);
	virtual float CoverRadius( void ) { return DEFAULT_COVER_SEEK_DISTANCE; } // Default cover radius

	virtual BOOL FCanCheckAttacks ( void );
	virtual void CheckAmmo( void ) { return; }
	virtual int IgnoreConditions ( void );
	


	// NOTE - SetConditions and ClearConditions assume changes apply to the normal and NextFrame conditions bitmasks.
	// HasConditions only involves the normal bitmask.
	// Verify that this 'NextFrame' system is even necessary anymore, one easy point of failure was pushing friendly NPC's
	// post-disaster not working (kept getting the PUSH condition forgotten before the next frame to affect the schedule).
	// 'NextFrame' stuff fixed that, or at least did at the time.

	inline void SetConditions( int iConditions ) {
		m_afConditions |= iConditions;
		m_afConditionsNextFrame |= iConditions;
	}
	inline void SetNextFrameConditions(int iConditions) {
		m_afConditions |= iConditions;
		m_afConditionsNextFrame |= iConditions;
	}
	inline void ClearConditions( int iConditions ) {
		m_afConditions &= ~iConditions;
		m_afConditionsNextFrame &= ~iConditions;
		// include m_afConditionsNextFrame, yes or no ?
	}
	inline void ClearConditions_ThisFrame(int iConditions) {
		m_afConditions &= ~iConditions;
	}
	inline void ClearConditions_NextFrame(int iConditions) {
		m_afConditionsNextFrame &= ~iConditions;
	}

	inline BOOL HasConditions( int iConditions ) { if (m_afConditions & iConditions ) return TRUE; return FALSE; }
	inline BOOL HasAllConditions( int iConditions ) { if ( (m_afConditions & iConditions) == iConditions ) return TRUE; return FALSE; }

	inline BOOL HasConditions_NextFrame(int iConditions) { if (m_afConditionsNextFrame & iConditions) return TRUE; return FALSE; }
	inline BOOL HasAllConditions_NextFrame(int iConditions) { if ((m_afConditionsNextFrame & iConditions) == iConditions) return TRUE; return FALSE; }

	inline BOOL HasConditionsEither(int iConditions) { if ((m_afConditions & iConditions) || (m_afConditionsNextFrame & iConditions) ) return TRUE; return FALSE; }
	inline BOOL HasAllConditionsEither(int iConditions) { if ((m_afConditions & iConditions) == iConditions || (m_afConditionsNextFrame & iConditions) == iConditions) return TRUE; return FALSE; }


	// For easier breakpoints.
	inline void ClearAllConditions(void) {
		m_afConditions = 0;
		m_afConditionsNextFrame = 0;
	}
	inline void ClearAllConditions_ThisFrame(void) {
		m_afConditions = 0;
	}
	inline void ClearAllConditions_NextFrame(void) {
		m_afConditionsNextFrame = 0;
	}

	inline void ClearAllConditionsExcept(int iConditions) {
		m_afConditions &= iConditions;
		m_afConditionsNextFrame &= iConditions;
	}
	inline void ClearAllConditionsExcept_ThisFrame(int iConditions) {
		m_afConditions &= iConditions;
	}
	inline void ClearAllConditionsExcept_NextFrame(int iConditions) {
		m_afConditionsNextFrame &= iConditions;
	}





	//MODDD - cloned for the new conditions bitmask.

	
	inline void SetConditionsMod( int iConditions ) {
		m_afConditionsMod |= iConditions;
		m_afConditionsModNextFrame |= iConditions;
	}
	inline void SetNextFrameConditionsMod(int iConditions) {
		m_afConditionsMod |= iConditions;
		m_afConditionsModNextFrame |= iConditions;
	}
	inline void ClearConditionsMod( int iConditions ) {
		m_afConditionsMod &= ~iConditions;
		m_afConditionsModNextFrame &= ~iConditions;
		// include m_afConditionsModNextFrame, yes or no ?
	}
	inline void ClearConditionsMod_ThisFrame(int iConditions) {
		m_afConditionsMod &= ~iConditions;
	}
	inline void ClearConditionsMod_NextFrame(int iConditions) {
		m_afConditionsModNextFrame &= ~iConditions;
	}

	inline BOOL HasConditionsMod( int iConditions ) { if (m_afConditionsMod & iConditions ) return TRUE; return FALSE; }
	inline BOOL HasAllConditionsMod( int iConditions ) { if ( (m_afConditionsMod & iConditions) == iConditions ) return TRUE; return FALSE; }

	inline BOOL HasConditionsMod_NextFrame(int iConditions) { if (m_afConditionsModNextFrame & iConditions) return TRUE; return FALSE; }
	inline BOOL HasAllConditionsMod_NextFrame(int iConditions) { if ((m_afConditionsModNextFrame & iConditions) == iConditions) return TRUE; return FALSE; }

	inline BOOL HasConditionsModEither(int iConditions) { if ((m_afConditionsMod & iConditions) || (m_afConditionsModNextFrame & iConditions) ) return TRUE; return FALSE; }
	inline BOOL HasAllConditionsModEither(int iConditions) { if ((m_afConditionsMod & iConditions) == iConditions || (m_afConditionsModNextFrame & iConditions) == iConditions) return TRUE; return FALSE; }


	// For easier breakpoints.
	inline void ClearAllConditionsMod(void) {
		m_afConditionsMod = 0;
		m_afConditionsModNextFrame = 0;
	}
	inline void ClearAllConditionsMod_ThisFrame(void) {
		m_afConditionsMod = 0;
	}
	inline void ClearAllConditionsMod_NextFrame(void) {
		m_afConditionsModNextFrame = 0;
	}

	inline void ClearAllConditionsModExcept(int iConditions) {
		m_afConditionsMod &= iConditions;
		m_afConditionsModNextFrame &= iConditions;
	}
	inline void ClearAllConditionsModExcept_ThisFrame(int iConditions) {
		m_afConditionsMod &= iConditions;
	}
	inline void ClearAllConditionsModExcept_NextFrame(int iConditions) {
		m_afConditionsModNextFrame &= iConditions;
	}







	virtual BOOL FValidateHintType( short sHint );
	int FindHintNode ( void );
	virtual BOOL FCanActiveIdle ( void );
	virtual void SetTurnActivity ( void );
	virtual void SetTurnActivityForceAct(void);
	float FLSoundVolume ( CSound *pSound );

	BOOL MoveToNode( Activity movementAct, float waitTime, const Vector &goal );
	BOOL MoveToTarget( Activity movementAct, float waitTime );
	BOOL MoveToTargetStrict( Activity movementAct, float waitTime );
	BOOL MoveToLocation( Activity movementAct, float waitTime, const Vector &goal );
	BOOL MoveToLocationStrict( Activity movementAct, float waitTime, const Vector &goal );
	//BOOL MoveToLocation( Activity movementAct, float waitTime, const Vector &goal, float argGoalDistTolerance );
	BOOL MoveToLocationCheap( Activity movementAct, float waitTime, const Vector &goal );
	BOOL MoveToEnemy( Activity movementAct, float waitTime );

	// Returns the time when the door will be open
	float OpenDoorAndWait( entvars_t *pevDoor );

	virtual void testMethod(void);

	//MODDD - 
	virtual BOOL interestedInBait(int arg_classID);
	virtual SCHEDULE_TYPE getHeardBaitSoundSchedule(CSound* pSound);
	virtual SCHEDULE_TYPE _getHeardBaitSoundSchedule(CSound* pSound);
	virtual SCHEDULE_TYPE getHeardBaitSoundSchedule();


	virtual int ISoundMask( void );
	virtual CSound* PBestSound ( void );
	virtual CSound* PBestScent ( void );
	virtual float HearingSensitivity( void ) { return 1.0; }

	BOOL FBecomeProne ( void );
	virtual void BarnacleVictimBitten( entvars_t *pevBarnacle );
	virtual void BarnacleVictimReleased( void );

	//MODDD - virtual. This can be hardcoded now.
	virtual void SetEyePosition ( void );

	BOOL FShouldEat( void );// see if a monster is 'hungry'
	void Eat ( float flFullDuration );// make the monster 'full' for a while.

	//MODDD - new version that can expect the 2nd bitmask.
	CBaseEntity *CheckTraceHullAttack( float flDist, int iDamage, int iDmgType );
	CBaseEntity *CheckTraceHullAttack( float flDist, int iDamage, int iDmgType, TraceResult* out_tr);
	CBaseEntity *CheckTraceHullAttack( float flDist, int iDamage, int iDmgType, int iDmgTypeMod );
	CBaseEntity *CheckTraceHullAttack( float flDist, int iDamage, int iDmgType, int iDmgTypeMod, TraceResult* out_tr);
	CBaseEntity *CheckTraceHullAttack( const Vector vecStartOffset, float flDist, int iDamage, int iDmgType );
	CBaseEntity *CheckTraceHullAttack( const Vector vecStartOffset, float flDist, int iDamage, int iDmgType, TraceResult* out_tr );
	CBaseEntity *CheckTraceHullAttack( const Vector vecStartOffset, float flDist, int iDamage, int iDmgType, int iDmgTypeMod );
	CBaseEntity *CheckTraceHullAttack( const Vector vecStartOffset, float flDist, int iDamage, int iDmgType, int iDmgTypeMod, TraceResult* out_tr );
	BOOL FacingIdeal( void );
	BOOL FacingIdeal(float argDegreeTolerance);

	BOOL FCheckAITrigger( void );// checks and, if necessary, fires the monster's trigger target. 

	//MODDD - why not virtual ?!
	// And no base method in basemonster.cpp?  Are you freakin' kidding me here.
	virtual BOOL NoFriendlyFire( void );

	BOOL BBoxFlat( void );

	// PrescheduleThink 
	virtual void PrescheduleThink( void ) { return; }

	BOOL GetEnemy ( void );
	//MODDD - new version
	BOOL GetEnemy (BOOL arg_forceWork );

	void MakeDamageBloodDecal ( int cCount, float flNoise, TraceResult *ptr, const Vector &vecDir );
	
	
	//MODDD CRITICAL
	// NEVER BEEN VIRTUAL.  OH DEAR.
	// LETS OPEN UP PANDAROA's BOX, SHALL WE???
	GENERATE_TRACEATTACK_PROTOTYPE_VIRTUAL
	GENERATE_TAKEDAMAGE_PROTOTYPE_VIRTUAL

	virtual BOOL ChangeHealthFiltered(entvars_t* pevAttacker, float flDamage);

	//MODDD - NEW. Overridable part of TRACEATTACK.
	virtual float hitgroupDamage(float flDamage, int bitsDamageType, int bitsDamageTypeMod, int iHitgroup);



	// combat functions
	//MODDD - no implementation, as-is?   whoops.
	//float UpdateTarget ( entvars_t *pevTarget );

	virtual Activity GetDeathActivity ( void );
	Activity GetSmallFlinchActivity( void );

	//MODDD - new
	Activity GetBigFlinchActivity(void);

	GENERATE_KILLED_PROTOTYPE_VIRTUAL
	//virtual void Killed( entvars_t *pevAttacker, int iGib );
	
	virtual BOOL DetermineGibHeadBlock(void);

	GENERATE_GIBMONSTER_PROTOTYPE_VIRTUAL
	GENERATE_GIBMONSTERGIB_PROTOTYPE_VIRTUAL
	GENERATE_GIBMONSTERSOUND_PROTOTYPE_VIRTUAL
	GENERATE_GIBMONSTEREND_PROTOTYPE_VIRTUAL

	
	BOOL		 ShouldGibMonster( int iGib );
	
	//MODDD - removed. Merged with GibMonster
	//void	 CallGibMonster( void );
	//void	 CallGibMonster( BOOL gibsSpawnDecals );


	//MODDD
	void cleanDelete(void);


	virtual BOOL	HasHumanGibs( void );
	virtual BOOL	HasAlienGibs( void );
	virtual void FadeMonster( void );	// Called instead of GibMonster() when gibs are disabled
	                                        //MODDD - little out of date comment above. GibMonster is called in anticipation of possibly gibbing a monster.
	                                        //        GibMonster will call FadeMonster if the monster is to be deleted without spawning any gibs.

	Vector ShootAtEnemy( const Vector &shootOrigin );
	Vector ShootAtEnemyEyes( const Vector &shootOrigin );
	Vector ShootAtEnemyMod( const Vector &shootOrigin );
	virtual Vector BodyTarget( const Vector &posSrc ) { return Center( ) * 0.75 + EyePosition() * 0.25; }		// position to shoot at
	//MODDD
	virtual Vector BodyTargetMod( const Vector &posSrc ) { return Center( ) * 0.75 + EyePosition() * 0.25; }		// position to shoot at

	virtual	Vector GetGunPosition( void );
	virtual Vector GetGunPositionAI(void);

	virtual void lookAtEnemy_pitch(void);

	virtual int TakeHealth( float flHealth, int bitsDamageType );
	

	//MODDD
	virtual void setModel(void);
	virtual void setModel(const char* m);
	virtual BOOL getMonsterBlockIdleAutoUpdate(void);



	//MODDD - DAMMIT WHY (were) YOU NOT VIRTUAL.
	GENERATE_DEADTAKEDAMAGE_PROTOTYPE_VIRTUAL

	//~see implementation in combat.cpp



	//MODDD - versions that support 2nd damage bitmask, "bitsDamageTypeMod".
	void RadiusDamageAutoRadius(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType );
	void RadiusDamageAutoRadius(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType, int bitsDamageTypeMod );
	//MODDD - 2nd version in basemonster.h moved to weapons.h to be treated as more of a universal utility.
	//...restoring these versions for compatability with the way things were and there are places that don't include "weapons.h". Identical and call to the global version as before.
	void RadiusDamageAutoRadius(Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType );
	void RadiusDamageAutoRadius(Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType, int bitsDamageTypeMod  );
	
	
	virtual int	IsMoving( void ) { return m_movementGoal != MOVEGOAL_NONE; }

	void RouteClear( void );
	void RouteNew( void );
	
	virtual void DeathSound ( void ) { return; }
	virtual void AlertSound ( void ) { return; }
	virtual void IdleSound ( void ) { return; }
	virtual void PainSound ( void ) { return; }
	
	// don't have to be implemented in hl_baseentity.cpp?  ok, interesting.   ...oh.  because they're stubs, { }.   DERP.
	virtual void StopFollowing( BOOL clearSchedule ) {}
	//MODDD - new version.
	virtual void StopFollowing(BOOL clearSchedule, BOOL playUnuseSentence) {}

	inline void Remember( int iMemory ) { m_afMemory |= iMemory; }
	inline void Forget( int iMemory ) { m_afMemory &= ~iMemory; }
	inline BOOL HasMemory( int iMemory ) { if ( m_afMemory & iMemory ) return TRUE; return FALSE; }
	inline BOOL HasAllMemories( int iMemory ) { if ( (m_afMemory & iMemory) == iMemory ) return TRUE; return FALSE; }

	BOOL ExitScriptedSequence( );
	BOOL CineCleanup( );
		
	//MODDD - version that takes a significant piece for customizing per monster.
	//        Called by CineCleanup above (which isn't implementable).
	virtual void OnCineCleanup(CCineMonster* pOldCine);


	CBaseEntity* DropItem ( char *pszItemName, const Vector &vecPos, const Vector &vecAng );// drop an item.

	virtual void ForgetEnemy(void);

	//MODDD - new.
	virtual void removeFromPoweredUpCommandList(CBaseMonster* argToRemove);
	virtual void forceNewEnemy(CBaseEntity* argIssuing, CBaseEntity* argNewEnemy, BOOL argPassive);

	virtual void setPoweredUpOff(void);
	virtual void setPoweredUpOn(CBaseMonster* argPoweredUpCauseEnt, float argHowLong );
	virtual void forgetForcedEnemy(CBaseMonster* argIssuing, BOOL argPassive);

	virtual void StartReanimation(void);
	virtual void StartReanimationPost(int preReviveSequence);

	virtual float MoveYawDegreeTolerance(void);
	int BloodColorRedFilter(void);
	int CanUseGermanModel(void);

	WayPoint_t* GetGoalNode(void);
	CBaseEntity* GetGoalEntity(void);

	virtual void ReportGeneric(void);

	virtual void onEnemyDead(CBaseEntity* pRecentEnemy);

	virtual BOOL predictRangeAttackEnd(void);

	void CheckRespawn(void);
	CBaseEntity* Respawn(void);
	void EXPORT AttemptToMaterialize(void);
	void EXPORT Materialize(void);

	virtual float ScriptEventSoundAttn(void);
	virtual float ScriptEventSoundVoiceAttn(void);
	virtual BOOL CanMakeBloodParticles(void);
	virtual BOOL AffectedByKnockback(void);

};




/*
//MODDD - never inherit from only, inherit from CBaseMonster or some child class too, and inherit from CResawpanble as a 2nd choice.
// Could be any entity though, but CBaseMonster's respawn methods, well, expect a CBaseMonster of some kind for the MonsterInit call.
// OOOoooookay.  Fuck this.  Don't do this.   It bad.
class CRespawnable {
public:
	Vector respawn_origin;
	Vector respawn_angles;

	// NOPE, this doesn't work, monsters that inherit from CBaseMonster (or some subclass) and CRespawnable still use the
	// CBaseMonster one (does return FALSE).  No idea how that works.
	// Just copy this to things inheriting from CRespawnable.
	//BOOL isRespawnable(void) { return TRUE; }
	
};
*/




#endif // BASEMONSTER_H
