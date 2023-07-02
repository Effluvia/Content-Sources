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
#ifndef PLAYER_H
#define PLAYER_H


#include "pm_materials.h"
#include "weapons.h"
#include "friendly.h"


// can remember the skin of up to 10 chumtoads, the ammo the game ships with anyway
#define CHUMTOAD_SKIN_MEM_MAX 10

//MODDD - new const
//Player always has long jump.  Forces "m_flongjump" to true whenever possible, generally from loading a game or spawning.
#define PLAYER_ALWAYSHASLONGJUMP 1

//Does the long jump use the delay feature (must crouch for so long before doing the long jump), = 1, or store it to a charge that is taken from, 
//like a storage battery, = 0?
#define LONGJUMPUSESDELAY 0

//How much "charge" does each long jump use?  Implies "LONGJUMPUSESDELAY" is 0.
#define LONGJUMP_CHARGEUSE 25


//NO AGAIN!  Now stored in const.h for access anywhere (client / server).
//No.  This is now stored in weapons.h, which Player's files have access to.
//Edit "PLAYER_LONGJUMPCHARGE_MAX" for the same effect.  Note that "delay" uses this as the time to spend charging, so 100 seconds is unrealistic
//for that (make it 1.2 when "LONGJUMPUSESDELAY" is 1).
//#define PLAYER_LONGJUMPCHARGE_MAX 1.2
//#define PLAYER_LONGJUMPCHARGE_MAX 100

//What is the delay allowed between successive longjumnps?  Used onyl when "LONGJUMPUSESDELAY" is 0, or when they're using the stored battery.
#define PLAYER_LONGJUMP_DELAY 4



//
// Player PHYSICS FLAGS bits
//
#define PFLAG_ONLADDER		( 1<<0 )
#define PFLAG_ONSWING		( 1<<0 )
#define PFLAG_ONTRAIN		( 1<<1 )
#define PFLAG_ONBARNACLE	( 1<<2 )
#define PFLAG_DUCKING		( 1<<3 )		// In the process of ducking, but totally squatted yet
#define PFLAG_USING			( 1<<4 )		// Using a continuous entity
#define PFLAG_OBSERVER		( 1<<5 )		// player is locked in stationary cam mode. Spectators can move, observers can't.


#define AUTOAIM_2DEGREES  0.0348994967025
#define AUTOAIM_5DEGREES  0.08715574274766
#define AUTOAIM_8DEGREES  0.1391731009601
#define AUTOAIM_10DEGREES 0.1736481776669


//
// generic player
//
//-----------------------------------------------------
//This is Half-Life player entity
//-----------------------------------------------------

//MODDD - changed, was 4.
#define CSUITPLAYLIST	6		// max of # suit sentences queued up at any time

#define SUIT_GROUP			TRUE
#define SUIT_SENTENCE		FALSE

#define SUIT_REPEAT_OK		0
#define SUIT_NEXT_IN_30SEC	30
#define SUIT_NEXT_IN_1MIN	60
#define SUIT_NEXT_IN_5MIN	300
#define SUIT_NEXT_IN_10MIN	600
//MODDD - new.
#define SUIT_NEXT_IN_20MIN	1200

#define SUIT_NEXT_IN_30MIN	1800
#define SUIT_NEXT_IN_1HOUR	3600

#define CSUITNOREPEAT		32

//MODDD - MOVED HERE, was in player.cpp deeper in the file.
#define SUITUPDATETIME	3.5
#define SUITFIRSTUPDATETIME 0.1


#define SOUND_FLASHLIGHT_ON		"items/flashlight1.wav"
#define SOUND_FLASHLIGHT_OFF	"items/flashlight1.wav"

#define TEAM_NAME_LENGTH	16

#define MAX_ID_RANGE 2048
#define SBAR_STRING_SIZE 128

#define CHAT_INTERVAL 1.0f



typedef enum
{
	PLAYER_IDLE,
	PLAYER_WALK,
	PLAYER_JUMP,
	PLAYER_SUPERJUMP,
	PLAYER_DIE,
	PLAYER_ATTACK1,
} PLAYER_ANIM;


enum sbar_data
{
	SBAR_ID_TARGETNAME = 1,
	SBAR_ID_TARGETHEALTH,
	SBAR_ID_TARGETARMOR,
	SBAR_END,
};



extern BOOL gInitHUD;


// MODDD - global method, moved from client.cpp
extern void respawn(entvars_t *pev, BOOL fCopyCorpse);





class CBasePlayer : public CBaseMonster
{
public:

	//MODDD - moved from CBaseEntity.  Stores ammo counts as named vars.
	// Might not even be necessary, notice the lack of one for snark/squeak ammo, a retail weapon.
	int ammo_9mm;
	int ammo_357;
	int ammo_bolts;
	int ammo_buckshot;
	int ammo_rockets;
	int ammo_uranium;
	int ammo_hornets;
	int ammo_argrens;


	BOOL queueFirstAppearanceMessageSend;
	float superDuperDelay;
	
	float m_fCustomHolsterWaitTime;

	BOOL m_bHolstering;
	CBasePlayerItem* m_pQueuedActiveItem;

	//the player will be unkillable up to this time (implied to be set & since revive).
	float reviveSafetyTime;


	BOOL iWasFrozenToday;


	float friendlyCheckTime;
	
	EHANDLE closestFriendlyMemEHANDLE;
	CFriendly* closestFriendlyMem;
	float horrorPlayTimePreDelay;
	float horrorPlayTime;

	
	int				random_seed;    // See that is shared between client & server for shared weapons code

	//MODDD - REMOVED.  Never referred to anywhere else, clearly this got hastily cut or never really developed anyfurther.
	//int				m_iPlayerSound;// the index of the sound list slot reserved for this player

	int				m_iTargetVolume;// ideal sound volume. 
	int				m_iWeaponVolume;// how loud the player's weapon is right now.
	int				m_iExtraSoundTypes;// additional classification for this weapon's sound
	int				m_iWeaponFlash;// brightness of the weapon flash
	float			m_flStopExtraSoundTime;
	
	float			m_flFlashLightTime;	// Time until next battery draw/Recharge
	int				m_iFlashBattery;		// Flashlight Battery Draw

	int				m_afButtonLast;
	int				m_afButtonPressed;
	int				m_afButtonReleased;
	
	edict_t			*m_pentSndLast;			// last sound entity to modify player room type
	float			m_flSndRoomtype;		// last roomtype set by sound entity
	float			m_flSndRange;			// dist from player to sound entity

	float			m_flFallVelocity;
	
	//MODDD NOTE - holds one-use items (adrenaline, antidote, radiation)
	int				m_rgItems[MAX_ITEMS];

	int				m_fKnownItem;		// True when a new item needs to be added
	int				m_fNewAmmo;			// True when a new item has been added

	unsigned int	m_afPhysicsFlags;	// physics flags - set when 'normal' physics should be revisited or overriden
	float			m_fNextSuicideTime; // the time after which the player can next use the suicide command


// these are time-sensitive things that we keep track of
	float			m_flTimeStepSound;	// when the last stepping sound was made
	float			m_flTimeWeaponIdle; // when to play another weapon idle animation.
	float			m_flSwimTime;		// how long player has been underwater
	float			m_flDuckTime;		// how long we've been ducking
	float			m_flWallJumpTime;	// how long until next walljump

	float			m_flSuitUpdate;					// when to play next suit update
	int				m_rgSuitPlayList[CSUITPLAYLIST];// next sentencenum to play for suit update
	
	//MODDD - also store the time for each suit sound to play.  Some may exceed the default 3.5 seconds
	//(causing the next in line to spill over).
	float			m_rgSuitPlayListDuration[CSUITPLAYLIST];// next sentencenum to play for suit update
	void (CBasePlayer::*m_rgSuitPlayListEvent[CSUITPLAYLIST])();
	float m_rgSuitPlayListEventDelay[CSUITPLAYLIST];
	float m_rgSuitPlayListFVoxCutoff[CSUITPLAYLIST];

	float currentSuitSoundEventTime;
	float currentSuitSoundFVoxCutoff;
	int sentenceFVoxCutoffStop;

	void (CBasePlayer::*currentSuitSoundEvent)();
	

	int				m_iSuitPlayNext;				// next sentence slot for queue storage;
	int				m_rgiSuitNoRepeat[CSUITNOREPEAT];		// suit sentence no repeat list
	float			m_rgflSuitNoRepeatTime[CSUITNOREPEAT];	// how long to wait before allowing repeat
	
	//MODDD - var removed.  Identical copy in CBaseMonster that went completely unused too.
	// This was only ever used in the TakeDamage method, no point being a member var here
	// anyway
	//int				m_lastDamageAmount;		// Last damage taken

	float			m_flgeigerRange;		// range to nearest radiation source
	float			m_flgeigerDelay;		// delay per update of range msg to client
	int				m_igeigerRangePrev;
	int				m_iStepLeft;			// alternate left/right foot stepping sound
	char			m_szTextureName[CBTEXTURENAMEMAX];	// current texture name we're standing on
	char			m_chTextureType;		// current texture type

	int				m_idrowndmg;			// track drowning damage taken
	int				m_idrownrestored;		// track drowning damage restored

	int				m_bitsHUDDamage;		// Damage bits for the current fame. These get sent to 
												// the hude via the DAMAGE message
	//MODDD - complementary
	int				m_bitsModHUDDamage;
	//MODDD - yes.
	int m_bitsDamageTypeForceShow;
	int m_bitsDamageTypeModForceShow;


	BOOL			m_fInitHUD;				// True when deferred HUD restart msg needs to be sent
	BOOL			m_fGameHUDInitialized;
	int				m_iTrain;				// Train control position
	BOOL			m_fWeapon;				// Set this to FALSE to force a reset of the current weapon HUD info

	EHANDLE			m_pTank;				// the tank which the player is currently controlling,  NULL if no tank
	float			m_fDeadTime;			// the time at which the player died  (used in PlayerDeathThink())

	BOOL			m_fNoPlayerSound;	// a debugging feature. Player makes no sound if this is true. 
	BOOL			m_fLongJump; // does this player have the longjump module?
	//MODDD
	BOOL			m_fLongJumpMemory;

	// ??????????
	//float	m_tSneaking;

	int		m_iUpdateTime;		// stores the number of frame ticks before sending HUD update messages
	//MODDD - now floats.
	float		m_iClientHealth;	// the health currently known by the client.  If this changes, send a new
	float		m_iClientBattery;	// the Battery currently known by the client.  If this changes, send a new

	//MODDD - added for the power canisters / syringes.
	int m_iClientAntidote;
	int m_iClientAdrenaline;
	int m_iClientRadiation;

	int		m_iHideHUD;		// the players hud weapon info is to be hidden
	int		m_iClientHideHUD;
	int		m_iFOV;			// field of view
	int		m_iClientFOV;	// client's known FOV

	// usable player items 
	//MODDD - NOTE.  Holds permanent pickups like CBasePlayerItem/Weapon instances.
	CBasePlayerItem	*m_rgpPlayerItems[MAX_ITEM_TYPES];
	CBasePlayerItem *m_pActiveItem;
	CBasePlayerItem *m_pClientActiveItem;  // client version of the active item
	CBasePlayerItem *m_pLastItem;

#ifdef CLIENT_DLL
	// And this version is for clientside only, ignores serverside issues for holstering to work better
	float m_flNextAttackCLIENTHISTORY;
	CBasePlayerItem* m_pActiveItemCLIENTHISTORY;
	int m_rgAmmoCLIENTHISTORY[MAX_AMMO_TYPES];
#endif

	// shared ammo slots
	int m_rgAmmo[MAX_AMMO_TYPES];
	int m_rgAmmoLast[MAX_AMMO_TYPES];

	Vector			m_vecAutoAim;
	BOOL			m_fOnTarget;
	int				m_iDeaths;
	float			m_iRespawnFrames;	// used in PlayerDeathThink() to make sure players can always respawn

	int m_lastx;
	int m_lasty;  // These are the previous update's crosshair angles, DON"T SAVE/RESTORE

	int m_nCustomSprayFrames;// Custom clan logo frames for this player
	float m_flNextDecalTime;// next time this player can spray a decal

	char m_szTeamName[TEAM_NAME_LENGTH];

	//MODDD - must be defined here, since this got removed from CBaseMonster.
	// Didn't make sense to be there, CBaseMonster did absolutely nothing with it and
	// only few subclasses (like CBasePlayer) ever did.
	float m_flNextAttack;


	//VARIABLES
	BOOL antidoteQueued;
	BOOL radiationQueued;
	//BOOL adrenalineQueued;    no need, recoveryIndex is enough

	float rawDamageSustained;


	float m_flStartCharge;

	float m_flAmmoStartCharge;
	float m_flPlayAftershock;
	float m_flNextAmmoBurn;// while charging, when to absorb another unit of player's ammo?

	int m_izSBarState[SBAR_END];
	float m_flNextSBarUpdateTime;
	float m_flStatusBarDisappearDelay;
	char m_SbarString0[SBAR_STRING_SIZE];
	char m_SbarString1[SBAR_STRING_SIZE];

	float m_flNextChatTime;

	float default_fov;
	float auto_adjust_fov;
	float auto_determined_fov;

	//Measuring the amount of time the player can breathe underwater with an air tank.
	BOOL airTankAirTimeNeedsUpdate;
	float airTankAirTime;
	float airTankAirTimeMem;
	float oldWaterMoveTime;
	float longJumpCharge;
	float longJumpChargeMem;
	float longJumpDelay;
	BOOL longJump_waitForRelease;
	BOOL longJumpChargeNeedsUpdate;
	float oldThinkTime;
	float lastDuckVelocityLength;

	//MODD
	float nextMadEffect;
	int deadflagmem;

	BOOL recentlyGrantedGlockSilencer;


	BOOL recentlyGibbed;

	int recoveryIndex;
	int recoveryDelay;
	int recoveryDelayMin;


	BOOL airTankWaitingStart;

	BOOL scheduleRemoveAllItems;
	BOOL scheduleRemoveAllItemsIncludeSuit;

	//MODDD - created to differentiate between "m_fLongJump" (always on now) and having ever picked
	//up the long jump item itself, if needed (mostly to satisfy the hazard course).
	BOOL hasLongJumpItem;

	int hasGlockSilencer;
	int hasGlockSilencerMem;


	int obligedCustomSentence;



	//MODDD - why even have these clientside at all if it's going to be ignored by having the actual
	// clientside CVars available?  Just something to trick me into thinking compiled script works as expected there.
#ifndef CLIENT_DLL
	BOOL fvoxEnabled;
	BOOL fHolsterAnimsEnabled;
	BOOL fBreakHolster;
	float cl_ladder_choice;

	// instance of player at serverside, global at clientside.
	int m_framesSinceRestore;
#endif


	float recentlySaidBattery;

	//void Think(void);
	//void MonsterThink(void);


	//MODDD - is this the first time in a while the player has been close to radation (false)?
	BOOL foundRadiation;

	BOOL getBatteryValueRealTime;
	int batterySayPhase;

	BOOL batteryInitiative;

	BOOL alreadyDroppedItemsAtDeath;
	BOOL sentCarcassScent;

	//MODDD
	int drowning;  // a BOOL, but the client doesn't know what bools are.  Probably wouldn't hurt anyways.
	int drowningMem;

	float playerBrightLightMem;
	float cameraModeMem;

	float mirrorsDoNotReflectPlayerMem;

	cvar_t* crossbowInheritsPlayerVelocity;
	float crossbowInheritsPlayerVelocityMem;

	cvar_t* fastHornetsInheritsPlayerVelocity;
	float fastHornetsInheritsPlayerVelocityMem;

	//cvar_t* autoSneaky;
	float autoSneakyMem;

	int framesUntilPushStops;

	float pushSpeedMulti;
	float pushSpeedMultiMem;

	float noclipSpeedMultiMem;
	float normalSpeedMultiMem;
	float jumpForceMultiMem;
	float ladderCycleMultiMem;
	float ladderSpeedMultiMem;
	float sv_player_midair_fixMem;
	float sv_player_midair_accelMem;

	int clearWeaponFlag;

	BOOL grabbedByBarnacle;
	BOOL grabbedByBarnacleMem;

	float minimumRespawnDelay;

	char recentlyPlayedSound[CBSENTENCENAME_MAX];

	int recentMajorTriggerDamage;
	float lastBlockDamageAttemptReceived;
	float recentRevivedTime;

	BOOL alreadySentSatchelOutOfAmmoNotice;

	int deadStage;
	float nextDeadStageTime;
	
	int recentDeadPlayerFollowersCount;
	EHANDLE recentDeadPlayerFollowers[5];

	
	//MODDD - TODO.  Keep in synch with the client.  That's gonna be fun.
	// How about a message to set each one at startup/spawn, and another
	// to set skins one at a time, given an index (what [#] to set) and the
	// skin itself.
	// Idea is, viewmodel shows the skin of the most recently picked up chumtoad,
	// and shows the next one picked up after that one is thrown.
	// Yes, we crazy.
	// Looking like this is getting scrapped though
	//int chumToadSkinMem[CHUMTOAD_SKIN_MEM_MAX];





	CBasePlayer(void);

	//MODDD - some setter methods.
	void setHealth(int newHealth);
	void setArmorBattery(int newBattery);
	void SetAndUpdateBattery(int argNewBattery);
	void attemptCureBleeding();


	void grantAllItems();
	void giveMaxAmmo();

	BOOL playerHasSuit();
	BOOL playerHasLongJump();

	BOOL blocksImpact(void);

	virtual void OnFirstAppearance(void);

	//MODDD - new.  Accept a new parameter (optional: assuming "false" if not given)
	virtual void Spawn(BOOL revived);
	virtual void Spawn( void );

	//MODDD - added from inheritance heirarchy
	//(IE: available, but not referred / used for the "player" itself before)
	virtual void Activate( void );


	//MODDD - this method was found as-is, but used to be named "Pain" but was never called by the player. This means it never had a chance to be played.
	//The proper name is "PainSound" to override the Monster-provided method for playing a pain sound. So any calls there for "PainSound" did nothing while
	//this was named "Pain" - no association.
	void PainSound( void ) override;

	//Separate method for a different way of doing a chance.
	void PainChance(void);

//	virtual void Think( void );
	virtual void Jump( void );
	virtual void Duck( void );
	virtual void PreThink( void );
	virtual void PostThink( void );
	virtual Vector GetGunPosition( void );
	virtual Vector GetGunPositionAI(void);
	virtual int TakeHealth( float flHealth, int bitsDamageType );

	//MODDD
	GENERATE_TRACEATTACK_PROTOTYPE_VIRTUAL
	GENERATE_TAKEDAMAGE_PROTOTYPE_VIRTUAL

	GENERATE_DEADTAKEDAMAGE_PROTOTYPE
	GENERATE_GIBMONSTER_PROTOTYPE
	
	void FadeMonster(void);

	
	GENERATE_KILLED_PROTOTYPE_VIRTUAL

	void onDelete(void);
	void stopSelfSounds(void);

	virtual Vector BodyTarget( const Vector &posSrc );		// position to shoot at
	virtual Vector BodyTargetMod(const Vector& posSrc);
	
	
	//MODDD NOTE - RRrrrrrreeeeeaaaaaally don't know what the point of these was.
	/*
	virtual void StartSneaking( void ) { m_tSneaking = gpGlobals->time - 1; }
	virtual void StopSneaking( void ) { m_tSneaking = gpGlobals->time + 30; }
	virtual BOOL IsSneaking( void ) { return m_tSneaking <= gpGlobals->time; }
	*/

	virtual BOOL IsAlive( void ) { return (pev->deadflag == DEAD_NO) && pev->health > 0; }
	virtual BOOL IsAlive_FromAI( CBaseMonster* whoWantsToKnow ) { return (pev->deadflag == DEAD_NO) && pev->health > 0; }

	virtual BOOL ShouldFadeOnDeath( void ) { return FALSE; }
	virtual	BOOL IsPlayer( void ) { return TRUE; }			// Spectators should return FALSE for this, they aren't "players" as far as game logic is concerned

	virtual BOOL IsNetClient( void ) { return TRUE; }		// Bots should return FALSE for this, they can't receive NET messages
															// Spectators should return TRUE for this

	
	virtual const char *TeamID( void );

	virtual int	Save( CSave &save );
	virtual int	Restore( CRestore &restore );
	static TYPEDESCRIPTION m_playerSaveData[];



	void RenewItems(void);
	void PackDeadPlayerItems( void );
	void RemoveAllAmmo(void);
	void RemoveAllItems( BOOL removeSuit );
	BOOL SwitchWeapon( CBasePlayerItem *pWeapon );

	// JOHN:  sends custom messages if player HUD data has changed  (eg health, ammo)
	virtual void UpdateClientData( void );
	

	// Player is moved across the transition by other means
	virtual int	ObjectCaps( void ) { return CBaseMonster::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	virtual void Precache( void );
	BOOL		IsOnLadder( void );
	BOOL		FlashlightIsOn( void );
	void		FlashlightTurnOn( void );
	void		FlashlightTurnOff( void );
	
	void UpdatePlayerSound ( void );

	void DeathSound ( void );
	//MODDD - new argument possibility, see player.cpp for more info.
	void DeathSound ( BOOL plannedRevive);

	void StartReanimation(void);
	void StartReanimationPost(int preReviveSequence);

	void reviveIfDead(void);
	void startRevive(void);
	void declareRevivelessDead(void);


	int Classify ( void );
	void SetAnimation( PLAYER_ANIM playerAnim );
	void SetWeaponAnimType( const char *szExtention );
	char m_szAnimExtention[32];

	// custom player functions
	virtual void ImpulseCommands( void );
	void CheatImpulseCommands( int iImpulse );

	void StartDeathCam( void );
	void StartObserver( Vector vecPosition, Vector vecViewAngle );

	void AddPoints( int score, BOOL bAllowNegativeScore );
	void AddPointsToTeam( int score, BOOL bAllowNegativeScore );
	BOOL AddPlayerItem( CBasePlayerItem *pItem );

	//MODDD - new
	void printOutWeapons(void);
	BOOL CanAddPlayerItem( int arg_iItemSlot, const char* arg_classname, const char* arg_ammoname, int arg_iMaxAmmo);
	


	BOOL RemovePlayerItem( CBasePlayerItem *pItem );
	void DropPlayerItem ( char *pszItemName );
	BOOL HasPlayerItem( CBasePlayerItem *pCheckItem );
	//MODDD - new
    BOOL HasPlayerItem( int arg_iItemSlot, const char* arg_className );

	BOOL HasNamedPlayerItem( const char *pszItemName );
	//MODDD
	CBasePlayerItem* FindNamedPlayerItem(const char* pszItemName );

	BOOL HasWeapons( void );// do I have ANY weapons?
	void SelectPrevItem( int iItem );
	void SelectNextItem( int iItem );
	void SelectLastItem(void);
	void SelectItem(const char *pstr);

	void setActiveItem(CBasePlayerItem* argItem);
	void setActiveItem_HolsterCheck(CBasePlayerItem* argItem);
	void setActiveItem_HolsterCheck(CBasePlayerItem* argItem, int forceHolster);
	
	void ItemPreFrame( void );
	void ItemPostFrame( void );

	//MODDD - new.  Calls "GiveNamedItem" IF the player does not have the item named "pszName".
	void GiveNamedItemIfLacking( const char *pszName );

	//MODDD - new versions too.
	//default (no extra args, just the name) uses the name of the thing to spawn.  Assumes the player's origin (pev->origin) is the spawn point.
	//If given 3 coords, turn that into a vector and send.
	//With a vector, use that as the origin instead.
	//GiveNamedItem, return the created item's edict for possible use again.
	
	
	
	edict_t* GiveNamedItem( const char *pszName );
	edict_t* GiveNamedItem( const char *pszName, float xCoord, float yCoord, float zCoord );
	edict_t* GiveNamedItem( const char *pszName, float xCoord, float yCoord, float zCoord, BOOL factorSpawnSize );
	edict_t* GiveNamedItem( const char *pszName, const Vector& coord );
	edict_t* GiveNamedItem( const char *pszName, const Vector& coord, BOOL factorSpawnSize );
	
	edict_t* GiveNamedItem( const char *pszName, int pszSpawnFlags );
	edict_t* GiveNamedItem( const char *pszName, int pszSpawnFlags, float xCoord, float yCoord, float zCoord );
	edict_t* GiveNamedItem( const char *pszName, int pszSpawnFlags, float xCoord, float yCoord, float zCoord, BOOL factorSpawnSize );
	edict_t* GiveNamedItem( const char *pszName, int pszSpawnFlags, const Vector& coord );
	edict_t* GiveNamedItem( const char *pszName, int pszSpawnFlags, const Vector& coord, BOOL factorSpawnSize );
	edict_t* GiveNamedItem( const char *pszName, int pszSpawnFlags, float xCoord, float yCoord, float zCoord, BOOL factorSpawnSize, TraceResult* tr );
	edict_t* GiveNamedItem( const char *pszName, int pszSpawnFlags, const Vector& coord, BOOL factorSpawnSize, TraceResult* tr );
	


	
	
	void EnableControl(BOOL fControl);

	//MODDD - now 'const char*' instead of just 'char*', most places involving strings do that.
	int GiveAmmo( int iAmount, const char* szName, int iMax );
	int GiveAmmoID(int iCount, int iAmmoTypeId, int iMax);

	void SendAmmoUpdate(void);

	void WaterMove( void );
	void EXPORT PlayerDeathThink( void );

	void HandleDeadStage(void);

	void PlayerUse( void );

	void CheckSuitUpdate();

	
	BOOL SetSuitUpdatePRE();
	BOOL SetSuitUpdatePRE(BOOL fvoxException);
	BOOL SetSuitUpdatePRE(char *name, int fgroup, int& isentence);
	BOOL SetSuitUpdatePRE(char *name, int fgroup, int& isentence, BOOL fvoxException);

	BOOL SetSuitUpdatePOST(int iempty, int isentence, float fNoRepeatTime, float playDuration, BOOL canPlay);
	
	BOOL SetSuitUpdateEventPOST(int iempty, int isentence, float fNoRepeatTime, float playDuration, BOOL canPlay, float eventDelay, void (CBasePlayer::*eventMethod)()  );

	BOOL SetSuitUpdateEventFVoxCutoffPOST(int iempty, int isentence, float fNoRepeatTime, float playDuration, BOOL canPlay, float eventDelay, void (CBasePlayer::*eventMethod)(), float fvoxCutoff );


	BOOL SetSuitUpdateNoRepeatSweep(int& iempty, int isentence);
	BOOL SetSuitUpdateCheckNoRepeatApply(int& iempty, int isentence);
	BOOL SetSuitUpdateCheckNoRepeat(int& iempty, int isentence);
	
	void SetSuitUpdateNumber(int number, float fNoRepeatTime, int noRepeatID, BOOL arg_getBatteryValueRealTime);
	void SetSuitUpdateNumber(int number, float fNoRepeatTime, int noRepeatID, BOOL arg_getBatteryValueRealTime, BOOL fvoxException);


	

	void SetSuitUpdateFVoxException(char *name, int fgroup, float fNoRepeatTime);
	void SetSuitUpdateFVoxException(char *name, int fgroup, float fNoRepeatTime, float playDuration);
	void SetSuitUpdate(char *name, int fgroup, float fNoRepeatTime);
	void SetSuitUpdate(char *name, int fgroup, float fNoRepeatTime, float playDuration);
	void SetSuitUpdate(char *name, int fgroup, float fNoRepeatTime, float playDuration, BOOL fvoxException );


	void SetSuitUpdateEvent(char *name, int fgroup, float fNoRepeatTime, float eventDelay, void (CBasePlayer::*eventMethod)() );
	void SetSuitUpdateEvent(char *name, int fgroup, float fNoRepeatTime, float playDuration, float eventDelay, void (CBasePlayer::*eventMethod)() );
	void SetSuitUpdateEvent(char *name, int fgroup, float fNoRepeatTime, float playDuration, BOOL fvoxException, float eventDelay, void (CBasePlayer::*eventMethod)() );
	void SetSuitUpdateEventFVoxCutoff(char *name, int fgroup, float fNoRepeatTime, float eventDelay, void (CBasePlayer::*eventMethod)(), float fvoxOffCutoff );
	void SetSuitUpdateEventFVoxCutoff(char *name, int fgroup, float fNoRepeatTime, float playDuration, float eventDelay, void (CBasePlayer::*eventMethod)(), float fvoxOffCutoff );
	void SetSuitUpdateEventFVoxCutoff(char *name, int fgroup, float fNoRepeatTime, float playDuration, BOOL fvoxException, float eventDelay, void (CBasePlayer::*eventMethod)(), float fvoxOffCutoff );


	void SetSuitUpdateAndForceBlock(char *name, int fgroup, float fNoRepeatTime);
	void SetSuitUpdateAndForceBlock(char *name, int fgroup, float fNoRepeatTime, float playDuration);
	void SetSuitUpdateAndForceBlock(char *name, int fgroup, float fNoRepeatTime, float playDuration, BOOL fvoxException);

	void forceRepeatBlock(char *name, int fgroup, float fNoRepeatTime);
	void forceRepeatBlock(char *name, int fgroup, float fNoRepeatTime, BOOL fvoxException);
	
	BOOL suitCanPlay(char* name, int fgroup);
	BOOL suitCanPlay(char *name, int fgroup, BOOL fvoxException);

	
	void consumeAntidote(void);
	void consumeRadiation(void);
	void consumeAdrenaline(void);
	

	//MODDD
	int getGeigerChannel();
	void UpdateGeigerCounter( void );

	virtual float TimedDamageBuddhaFilter(float dmgIntent);
	virtual void TimedDamagePostBuddhaCheck(void);
	virtual void removeTimedDamageImmediate(int arg_type, int* m_bitsDamageTypeRef, BYTE bDuration);
	virtual BYTE parse_itbd_duration(int i);
	virtual void parse_itbd(int i);
	virtual void timedDamage_nonFirstFrame(int i, int* m_bitsDamageTypeRef);

	BOOL FBecomeProne ( void );
	void BarnacleVictimBitten ( entvars_t *pevBarnacle );
	void BarnacleVictimReleased ( void );

	int AmmoInventory( int iAmmoIndex );
	int Illumination( void );

	void ResetAutoaim( void );
	Vector GetAutoaimVector( float flDelta  );
	Vector AutoaimDeflection( Vector &vecSrc, float flDist, float flDelta  );

	void ForceClientDllUpdate( void );  // Forces all client .dll specific data to be resent to client.

	void DeathMessage( entvars_t *pevKiller );

	void SetCustomDecalFrames( int nFrames );
	int GetCustomDecalFrames( void );

	void TabulateAmmo( void );

	void set_fvoxEnabled(BOOL argNew, BOOL setSilent);
	void set_cl_ladder_choice(float argNew);


	void InitStatusBar( void );
	void UpdateStatusBar( void );

	//NEW METHODS for organization.
	void _commonReset(void);
	void commonReset(void);

	void resetLongJumpCharge();

	void autoSneakyCheck(void);
	void turnOnSneaky(void);
	void turnOffSneaky(void);

	BOOL usesSoundSentenceSave(void);

	void SetGravity(float newGravityVal);

	void RecordFollowers(void);

	//MODDD - NEW.  Also inline now, very simple method and consistent for client/serverside.
	// HOWEVER, this is only for easily compiling in both places.
	// Clientside should use gHUD.getPlayerBaseFOV intead, as these places are not updated clientside.
	// In CBaseWeapon and child classes (just about anything), just use CBaseWeapon's own "getPlayerBaseFOV"
	// instead. It redirects to the right place between client/serverside for you.
	// No, do it proper, compile only serverside then.  No trickery.
#ifndef CLIENT_DLL
	inline float getBaseFOV(void) {
		if (auto_adjust_fov == 0) {
			// don't use the auto one then.
			return default_fov;
		}
		else {
			// use the one related to screensize.
			return auto_determined_fov;
		}
	}//END OF getBaseFOV
#endif

};


#endif // PLAYER_H
