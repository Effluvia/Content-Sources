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
#ifndef WEAPONS_H
#define WEAPONS_H

#include "cbase.h"  //why not?!
#include "animating.h"
#include "basemonster.h"
#include "effects.h"
#include "util_shared.h"

//MODDD - to get the gHUD instance for getting an accurate FOV reading clientside, player data is garbage there.
#ifdef CLIENT_DLL
#include "hud.h"
#endif


//MODDD - changed so that there is a max of 7 (two new power canisters that were not present ingame).
#define MAX_ITEMS				7	// hard coded item types


//MODDD - NOTE.   ugh.  Should've changed this ages ago but now it would mess with people's save files.
// No sense in ITEM_HEALTHKIT or ITEM_BATTERY.  Never used.
// ITEM_SECURITY has some collection logic in items.cpp but nothing ever depends on it being there.

// constant items
#define ITEM_HEALTHKIT		1
#define ITEM_ANTIDOTE		2
#define ITEM_SECURITY		3
#define ITEM_BATTERY		4
//MODDD - other power canisters
#define ITEM_ADRENALINE		5
#define ITEM_RADIATION		6


// inventory max's for power canisters.
// Should clientside (cl_dlls/ammo.cpp) involve these too?
#define ITEM_ANTIDOTE_MAX 5
#define ITEM_ADRENALINE_MAX 5
#define ITEM_RADIATION_MAX 5



#define WEAPON_NONE				0
#define WEAPON_CROWBAR			1
#define WEAPON_GLOCK			2
#define WEAPON_PYTHON			3
#define WEAPON_MP5				4
#define WEAPON_CHAINGUN			5  //WOA HOLD ON NOW!!! CHAINGUN? WOO.    definitely cut or never went forward with it though.
#define WEAPON_CROSSBOW			6
#define WEAPON_SHOTGUN			7
#define WEAPON_RPG				8
#define WEAPON_GAUSS			9
#define WEAPON_EGON				10
#define WEAPON_HORNETGUN		11
#define WEAPON_HANDGRENADE		12
#define WEAPON_TRIPMINE			13
#define WEAPON_SATCHEL			14
#define WEAPON_SNARK			15
//MODDD - new
#define WEAPON_CHUMTOAD			16


#define WEAPON_ALLWEAPONS		(~(1<<WEAPON_SUIT))

#define WEAPON_SUIT				31	// ?????

#define MAX_WEAPONS			32

//MODDD - Stored in const.h instead.  Coordination with two separate constants not necessary, refer to the same one either place.
//MODDD - coordinate with Player.cpp's long jump charge!
//#define PLAYER_LONGJUMPCHARGE_MAX 100
//#define PLAYER_LONGJUMP_PICKUPADD 25

#define MAX_NORMAL_BATTERY	100


// weapon weight factors (for auto-switching)   (-1 = noswitch)
#define CROWBAR_WEIGHT		0
#define GLOCK_WEIGHT		10
#define PYTHON_WEIGHT		15
#define MP5_WEIGHT			15
#define SHOTGUN_WEIGHT		15
#define CROSSBOW_WEIGHT		10
#define RPG_WEIGHT			20
#define GAUSS_WEIGHT		20
#define EGON_WEIGHT			20
#define HORNETGUN_WEIGHT	10
#define HANDGRENADE_WEIGHT	5
#define SNARK_WEIGHT		5
#define SATCHEL_WEIGHT		-10
#define TRIPMINE_WEIGHT		-10
//MODDD
#define CHUMTOAD_WEIGHT		5 



// weapon clip/carry ammo capacities

//MODDD - CLEVER BASTARD.  Just redirect these to our
// new skill CVar constants.
/**
#define URANIUM_MAX_CARRY		100
#define _9MM_MAX_CARRY			120    // was 150
#define _357_MAX_CARRY			36
#define BUCKSHOT_MAX_CARRY		125
#define BOLT_MAX_CARRY			50
#define ROCKET_MAX_CARRY		5
#define HANDGRENADE_MAX_CARRY	20
#define SATCHEL_MAX_CARRY		5
#define TRIPMINE_MAX_CARRY		5
#define SNARK_MAX_CARRY			15
#define HORNET_MAX_CARRY		8
#define M203_GRENADE_MAX_CARRY	12
#define CHUMTOAD_MAX_CARRY		5
*/

#ifndef CLIENT_DLL
#define URANIUM_MAX_CARRY		gSkillData.player_ammomax_uranium
#define _9MM_MAX_CARRY			gSkillData.player_ammomax_9mm
#define _357_MAX_CARRY			gSkillData.player_ammomax_revolver
#define BUCKSHOT_MAX_CARRY		gSkillData.player_ammomax_shotgun
#define BOLT_MAX_CARRY			gSkillData.player_ammomax_crossbow
#define ROCKET_MAX_CARRY		gSkillData.player_ammomax_rpg
#define HANDGRENADE_MAX_CARRY	gSkillData.player_ammomax_handgrenade
#define SATCHEL_MAX_CARRY		gSkillData.player_ammomax_satchel
#define TRIPMINE_MAX_CARRY		gSkillData.player_ammomax_tripmine
#define SNARK_MAX_CARRY			gSkillData.player_ammomax_snark
#define HORNET_MAX_CARRY		gSkillData.player_ammomax_hornet
#define M203_GRENADE_MAX_CARRY	gSkillData.player_ammomax_mp5_grenade
#define CHUMTOAD_MAX_CARRY		gSkillData.player_ammomax_chumtoad
#else
// methods dealing with these are dummied clientide anyway.
// Using 254 instead of 0 in case that helps with some dumb "is 0" checks clientside,
// but logic isn't strong there anyway.  If it were to be smarter, the gsSkillData's
// would need to be sent to be broadcasted from server to clientside for all players
// at startup.      ........no.
// And using 254 instead of 255 in case 255 is some equivalent for '-1' in interpreting
// a signed type as unsigned somewhere.
#define URANIUM_MAX_CARRY		254
#define _9MM_MAX_CARRY			254
#define _357_MAX_CARRY			254
#define BUCKSHOT_MAX_CARRY		254
#define BOLT_MAX_CARRY			254
#define ROCKET_MAX_CARRY		254
#define HANDGRENADE_MAX_CARRY	254
#define SATCHEL_MAX_CARRY		254
#define TRIPMINE_MAX_CARRY		254
#define SNARK_MAX_CARRY			254
#define HORNET_MAX_CARRY		254
#define M203_GRENADE_MAX_CARRY	254
#define CHUMTOAD_MAX_CARRY		254
#endif



// the maximum amount of ammo each weapon's clip can hold
#define WEAPON_NOCLIP			-1

#define GLOCK_MAX_CLIP			13
#define PYTHON_MAX_CLIP			6
#define MP5_MAX_CLIP			30
#define MP5_DEFAULT_AMMO		30
#define SHOTGUN_MAX_CLIP		8
#define CROSSBOW_MAX_CLIP		5
#define RPG_MAX_CLIP			1
#define GAUSS_MAX_CLIP			WEAPON_NOCLIP
#define EGON_MAX_CLIP			WEAPON_NOCLIP
#define HORNETGUN_MAX_CLIP		WEAPON_NOCLIP
#define HANDGRENADE_MAX_CLIP	WEAPON_NOCLIP
#define SATCHEL_MAX_CLIP		WEAPON_NOCLIP
#define TRIPMINE_MAX_CLIP		WEAPON_NOCLIP
#define SNARK_MAX_CLIP			WEAPON_NOCLIP


// the default amount of ammo that comes with each gun when it spawns
//MODDD - NOTE: is "12", but we add one in the case of first getting the weapon for the first time or reloading when not empty.  IF the CVar "glockOldReloadLogic" is on.
#define GLOCK_DEFAULT_GIVE			13

#define PYTHON_DEFAULT_GIVE			6
#define MP5_DEFAULT_GIVE			30
#define MP5_DEFAULT_AMMO			30
#define MP5_M203_DEFAULT_GIVE		0
#define SHOTGUN_DEFAULT_GIVE		12
#define CROSSBOW_DEFAULT_GIVE		5
#define RPG_DEFAULT_GIVE			1
#define GAUSS_DEFAULT_GIVE			20
#define EGON_DEFAULT_GIVE			20
#define HANDGRENADE_DEFAULT_GIVE	5
#define SATCHEL_DEFAULT_GIVE		1
#define TRIPMINE_DEFAULT_GIVE		1
#define SNARK_DEFAULT_GIVE			5
#define HIVEHAND_DEFAULT_GIVE		8
//MODDD
#define CHUMTOAD_DEFAULT_GIVE		1

// The amount of ammo given to a player by an ammo item.
#define AMMO_URANIUMBOX_GIVE	20
#define AMMO_GLOCKCLIP_GIVE		GLOCK_MAX_CLIP
#define AMMO_357BOX_GIVE		PYTHON_MAX_CLIP
#define AMMO_MP5CLIP_GIVE		MP5_MAX_CLIP
#define AMMO_CHAINBOX_GIVE		200
#define AMMO_M203BOX_GIVE		2
#define AMMO_BUCKSHOTBOX_GIVE	12
#define AMMO_CROSSBOWCLIP_GIVE	CROSSBOW_MAX_CLIP
#define AMMO_RPGCLIP_GIVE		RPG_MAX_CLIP
#define AMMO_URANIUMBOX_GIVE	20
#define AMMO_SNARKBOX_GIVE		5

// MODDD - 'bullet types' enum moved to util_shared.

// ITEM_FLAG_... and WEAPON_IS_ONTARGET macros moved to const.h to show up everywhere



// new choice.  Rockets and shotguns ain't exactly subtle.
#define LOUDEST_GUN_VOLUME		2200

// changed from 1000 to 1400.  They are loud after all.
#define LOUD_GUN_VOLUME			1500
// changed from 600 to 900.  C'mon, glocks aren't quiet
#define NORMAL_GUN_VOLUME		900
// changed from 200 to 300
#define QUIET_GUN_VOLUME		300

#define BRIGHT_GUN_FLASH		512
#define NORMAL_GUN_FLASH		256
#define DIM_GUN_FLASH			128

#define BIG_EXPLOSION_VOLUME	2048
#define NORMAL_EXPLOSION_VOLUME	1024
#define SMALL_EXPLOSION_VOLUME	512

#define WEAPON_ACTIVITY_VOLUME	64


// VECTOR_CONE_... macros moved to util_shared.h



class CBasePlayer;

// no no no, can't forward declare a struct without a name.
// that is one like
//     typedef struct { ... } Y.
// If it were named it would've been
//     typedef struct X { ... } Y
// (X being the name)
// allllrighty then.




extern DLL_GLOBAL short g_sModelIndexLaser;// holds the index for the laser beam
extern DLL_GLOBAL const char* g_pModelNameLaser;

extern DLL_GLOBAL short g_sModelIndexLaserDot;// holds the index for the laser beam dot
extern DLL_GLOBAL short g_sModelIndexFireball;// holds the index for the fireball
extern DLL_GLOBAL short g_sModelIndexSmoke;// holds the index for the smoke cloud
extern DLL_GLOBAL short g_sModelIndexWExplosion;// holds the index for the underwater explosion
//extern DLL_GLOBAL short g_sModelIndexBubbles;// holds the index for the bubbles model
extern DLL_GLOBAL short g_sModelIndexBloodDrop;// holds the sprite index for blood drops
extern DLL_GLOBAL short g_sModelIndexBloodSpray;// holds the sprite index for blood spray (bigger)

extern int gmsgWeapPickup;



#ifdef CLIENT_DLL
extern int g_framesSinceRestore;
#endif



#ifdef CLIENT_DLL
//MODDD - also externed in cl_util.h now.
// nope, just use "IsMultiplayer" from util_shared.h instead
//extern bool bIsMultiplayer(void);

//MODDD - NOTE.  Defined in cl_dlls/hl/hl_weapons.cpp
extern void LoadVModel(char* szViewModel, CBasePlayer* m_pPlayer);
#endif


extern void ClearMultiDamage(void);
extern void ApplyMultiDamage(entvars_t* pevInflictor, entvars_t* pevAttacker);
extern void AddMultiDamage(entvars_t* pevInflictor, CBaseEntity* pEntity, float flDamage, int bitsDamageType);
extern void AddMultiDamage(entvars_t* pevInflictor, CBaseEntity* pEntity, float flDamage, int bitsDamageType, int bitsDamageTypeMod);



//MODDD - NEW.  Methods in ggrenade.cpp for allowing a grenade's explode to be used without a grenade.
// Includes scorch effect (if possible), explosion sprite effect, and radiusDamage.


extern void SimpleStaticExplode(Vector rawExplodeOrigin, float rawDamage, CBaseEntity* pDamageDealer);
extern void SimpleStaticExplode(Vector rawExplodeOrigin, float rawDamage, CBaseEntity* pDamageDealer, entvars_t* entOwner);
extern void SimpleStaticExplode(Vector rawExplodeOrigin, float rawDamage, float flRange, CBaseEntity* pDamageDealer);
extern void SimpleStaticExplode(Vector rawExplodeOrigin, float rawDamage, float flRange, CBaseEntity* pDamageDealer, entvars_t* entOwner);

extern void StaticExplode(Vector rawExplodeOrigin, float rawDamage, CBaseEntity* pDamageDealer, entvars_t* entOwner, TraceResult* pTrace, int bitsDamageType);
extern void StaticExplode(Vector rawExplodeOrigin, float rawDamage, CBaseEntity* pDamageDealer, entvars_t* entOwner, TraceResult* pTrace, int bitsDamageType, int bitsDamageTypeMod);
extern void StaticExplode(Vector rawExplodeOrigin, float rawDamage, CBaseEntity* pDamageDealer, entvars_t* entOwner, TraceResult* pTrace, int bitsDamageType, int bitsDamageTypeMod, float shrapMod);
extern void StaticExplode(Vector rawExplodeOrigin, float rawDamage, float flRange, CBaseEntity* pDamageDealer, entvars_t* entOwner, TraceResult* pTrace, int bitsDamageType);
extern void StaticExplode(Vector rawExplodeOrigin, float rawDamage, float flRange, CBaseEntity* pDamageDealer, entvars_t* entOwner, TraceResult* pTrace, int bitsDamageType, int bitsDamageTypeMod);
extern void StaticExplode(Vector rawExplodeOrigin, float rawDamage, float flRange, CBaseEntity* pDamageDealer, entvars_t* entOwner, TraceResult* pTrace, int bitsDamageType, int bitsDamageTypeMod, float shrapMod);






typedef struct
{
	int	iSlot;
	int	iPosition;
	const char* pszAmmo1;	// ammo 1 type
	int	iMaxAmmo1;		// max ammo 1
	const char* pszAmmo2;	// ammo 2 type
	int	iMaxAmmo2;		// max ammo 2
	const char* pszName;
	int	iMaxClip;
	int	iId;
	int	iFlags;
	int	iWeight;// this value used to determine this weapon's importance in autoselection.
} ItemInfo;

typedef struct
{
	const char* pszName;
	int iId;
	//MODDD - and capacity per ammo type wasn't stored here, beeecaaaauuuuuussssseee?
	int iAmmoMax;
} AmmoInfo;

//MODDD - NEW.  See AmmoTypeCacheArray.
typedef struct {
	int iPrimaryAmmoType;
	int iSecondaryAmmoType;
} AmmoTypeCache;


typedef struct
{
	CBaseEntity* pEntity;
	float		amount;
	int			type;
	//MODDD - 2nd damge bitmask added.
	int			typeMod;
} MULTIDAMAGE;

extern MULTIDAMAGE gMultiDamage;



// Contact Grenade / Timed grenade / Satchel Charge
// MODDD - parent class changed from CBaseMonster to CBaseAnimating.
// That was.  A lot of completely unused logic just to get 'm_flNextAttack', which
// CBaseMonster itself completely ignored.  And that got replaced by
// m_flBounceDamageCooldown.
// Keep in mind, this won't be saved as m_flNextAttack was in CBaseMonster, but
// do we really care?  m_flBounceDamageCooldown is at most a forced delay for stopping
// the grenade from doing high-speed contact damge too quickly in a short amount of time.
class CGrenade : public CBaseAnimating
{
public:
	//MODDD - whoops, unused as-is.  Odd place for it as there is a satchel.cpp.
	//typedef enum { SATCHEL_DETONATE = 0, SATCHEL_RELEASE } SATCHELCODE;

	BOOL dropped;
	BOOL firstGroundContactYet;

	BOOL m_fRegisteredSound;// whether or not this grenade has issued its DANGER sound to the world sound list yet.

	//MODDD - NEW.  Since we're no longer inheriting from CBaseMonster, we lack m_flNextAttack.
	// Although it was just being used as a simple time variable without any behavior just from
	// being part of CBaseMonster anyway, may as well be replaced with a more fitting name here.
	float m_flBounceDamageCooldown;
	// and don't spam bounce sounds.
	float nextBounceSoundAllowed;

public:


	CGrenade(void);

	//MODDD - added for compatibility.
	// not necessary.
	//void Activate( void );

	void Spawn( void );

	//MODDD - supports damage to deal in the call.
	static CGrenade *ShootTimed( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float flDamage, float flDetonateTime );
	static CGrenade* ShootTimedDropped(entvars_t* pevOwner, Vector vecStart, Vector vecVelocity, float flDamage, float flDetonateTime);
	static CGrenade *ShootContact( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float flDamage );


	//MODDD - unused methods, removed.
	//static CGrenade *ShootSatchelCharge( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity );
	//static void UseSatchelCharges( entvars_t *pevOwner, SATCHELCODE code );

	//MODDD - parameters are ignored, removed.
	//void Explode( Vector vecSrc, Vector vecAim );


	//MODDD - why do we even need these?
	GENERATE_TRACEATTACK_PROTOTYPE_VIRTUAL
	GENERATE_TAKEDAMAGE_PROTOTYPE_VIRTUAL


	void Explode(void);
	void Explode( TraceResult *pTrace, int bitsDamageType );
	void Explode( TraceResult *pTrace, int bitsDamageType, int bitsDamageTypeMod );
	void Explode(TraceResult* pTrace, float rawDamage, float flRange, int bitsDamageType, int bitsDamageTypeMod, float shrapMod);

	void EXPORT Smoke( void );

	void EXPORT BounceTouch( CBaseEntity *pOther );
	void EXPORT SlideTouch( CBaseEntity *pOther );
	void EXPORT ExplodeTouch( CBaseEntity *pOther );
	void EXPORT DangerSoundThink( void );
	void EXPORT PreDetonate( void );
	void EXPORT Detonate( void );
	void EXPORT DetonateUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void EXPORT TumbleThink( void );

	virtual void BounceSound( void );
	virtual int BloodColor( void ) { return DONT_BLEED; }

	virtual float massInfluence(void);
	virtual int GetProjectileType(void);
	
	GENERATE_KILLED_PROTOTYPE_VIRTUAL
	//virtual void Killed( entvars_t *pevAttacker, int iGib );

	virtual BOOL isOrganic(void);
	virtual BOOL usesSoundSentenceSave(void);

	virtual void groundContact(void);


};




// Items that the player has in their inventory that they can use
class CBasePlayerItem : public CBaseAnimating
{
public:
	virtual void SetObjectCollisionBox( void );

	virtual int	Save( CSave &save );
	virtual int	Restore( CRestore &restore );
	
	static	TYPEDESCRIPTION m_SaveData[];

	virtual BOOL AddToPlayer( CBasePlayer *pPlayer );	// return TRUE if the item you want the item added to the player inventory
	
	
	virtual BOOL AddDuplicate( CBasePlayerItem *pItem ) { return FALSE; }	// return TRUE if you want your duplicate removed from world
	//MODDD NOTE - not sure if this was intentional or not, but this method returns how much ammo the given item gave to the player.  As seen in weapons.cpp's implementation, if any ammo is given, remove this item.  Being "0" (no ammo given) is the same as returning FALSE, or saying not to remove the item (did not touch).
	
	//MODDD - the glock has an exception for being picked up anyways: giving the player the silencer.
	virtual BOOL weaponCanHaveExtraCheck( CBasePlayer* pPlayer){return FALSE; }
	virtual BOOL weaponPlayPickupSoundException( CBasePlayer* pPlayer){return FALSE; }


	void EXPORT DestroyItem( void );

	//MODDD - new
	void EXPORT DefaultTouchRemoveThink( CBaseEntity *pOther );	// default weapon touch

	void EXPORT DefaultTouch( CBaseEntity *pOther );	// default weapon touch


	void EXPORT FallThink ( void );// when an item is first spawned, this think is run to determine when the object has hit the ground.
	void EXPORT Materialize( void );// make a weapon visible and tangible
	void EXPORT AttemptToMaterialize( void );  // the weapon desires to become visible and tangible, if the game rules allow for it
	CBaseEntity* Respawn ( void );// copy a weapon
	
	//MODDD - why not virtual before???
	virtual void FallInit( void );

	void CheckRespawn( void );
	virtual int GetItemInfo(ItemInfo *p) { return 0; }	// returns 0 if struct not filled out
	virtual BOOL CanDeploy( void ) { return TRUE; }
	virtual BOOL Deploy( )								// returns is deploy was successful
		 { return TRUE; }

	virtual BOOL CanHolster( void ) { return TRUE; }// can this weapon be put away right now?
	virtual void Holster( int skiplocal = 0 );
	virtual void UpdateItemInfo( void ) { return; }

	virtual void ItemPreFrame( void )	{ return; }		// called each frame by the player PreThink
	virtual void ItemPostFrame( void ) { return; }		// called each frame by the player PostThink
	//MODDD - it appears the above two are not called when the player's "m_flNextAttack" var is on (delay before being able to fire again).
	
	//So, new methods that work regardless of that:
	virtual void ItemPreFrameThink( void )	{ return; }		// called each frame by the player PreThink, even when "m_flNextAttack" is on.
	virtual void ItemPostFrameThink( void ) { return; }		// called each frame by the player PostThink, even when "m_flNextAttack" is on.
	

	virtual void Drop( void );
	virtual void Kill( void );
	virtual void AttachToPlayer ( CBasePlayer *pPlayer );

	virtual void customAttachToPlayer( CBasePlayer *pPlayer) { }  //can default to doing nothing.

	virtual int PrimaryAmmoIndex() { return -1; }
	virtual int SecondaryAmmoIndex() { return -1; }

	virtual int UpdateClientData( CBasePlayer *pPlayer ) { return 0; }

	virtual CBasePlayerItem *GetWeaponPtr( void ) { return NULL; }

	static ItemInfo ItemInfoArray[MAX_WEAPONS];
	//MODDD - NEW.
	static AmmoTypeCache AmmoTypeCacheArray[MAX_WEAPONS];

	static AmmoInfo AmmoInfoArray[MAX_AMMO_TYPES];

	CBasePlayer	*m_pPlayer;
	CBasePlayerItem *m_pNext;
	int	m_iId;												// WEAPON_???

	virtual int iItemSlot( void ) { return 0; }			// return 0 to MAX_ITEMS_SLOTS, used in hud

	int		iItemPosition( void ) { return ItemInfoArray[ m_iId ].iPosition; }
	const char	*pszAmmo1( void )	{ return ItemInfoArray[ m_iId ].pszAmmo1; }
	int		iMaxAmmo1( void )	{ return ItemInfoArray[ m_iId ].iMaxAmmo1; }
	const char	*pszAmmo2( void )	{ return ItemInfoArray[ m_iId ].pszAmmo2; }
	int		iMaxAmmo2( void )	{ return ItemInfoArray[ m_iId ].iMaxAmmo2; }
	const char	*pszName( void )	{ return ItemInfoArray[ m_iId ].pszName; }
	int		iMaxClip( void )	{ return ItemInfoArray[ m_iId ].iMaxClip; }
	int		iWeight( void )		{ return ItemInfoArray[ m_iId ].iWeight; }
	int		iFlags( void )		{ return ItemInfoArray[ m_iId ].iFlags; }

	//MODDD - NEW!  Convenient access for the new AmmoTypeCacheArray.
	// It may be possible to phase out CBasePlayerWeapon's m_iPrimaryAmmoType and m_iSecondaryAmmoType vars.
	// These do not involve those variables.
	int getPrimaryAmmoType(void) { return AmmoTypeCacheArray[m_iId].iPrimaryAmmoType; }
	int getSecondaryAmmoType(void) { return AmmoTypeCacheArray[m_iId].iSecondaryAmmoType; }
	// And static versions, for use with an ID only
	static int getPrimaryAmmoType(int arg_iId) { return AmmoTypeCacheArray[arg_iId].iPrimaryAmmoType; }
	static int getSecondaryAmmoType(int arg_iId) { return AmmoTypeCacheArray[arg_iId].iSecondaryAmmoType; }

	// int	m_iIdPrimary;										// Unique Id for primary ammo
	// int	m_iIdSecondary;										// Unique Id for secondary ammo
};


// inventory items that 
class CBasePlayerWeapon : public CBasePlayerItem
{
public:
	virtual int	Save( CSave &save );
	virtual int	Restore( CRestore &restore );

	static TYPEDESCRIPTION m_SaveData[];


	//MODDD - constructor.
	CBasePlayerWeapon();

	//MODDD - new.
	void setchargeReady(int arg);
	int getchargeReady(void);
	void forceBlockLooping(void);
	void stopBlockLooping(void);

	BOOL isBasePlayerWeapon(void){return TRUE;}


	//MODDD - new.  Get the time to add to an idle animation's delay (beyond the bare minimum to finish the current anim of course)
	// Made virtual. For the lazy ass in you, go overwrite this to add 0 seconds for no static delays for things not meant to be lifeless like snarks or chumtoads.
	virtual float randomIdleAnimationDelay(void);


	// generic weapon versions of CBasePlayerItem calls
	virtual BOOL AddToPlayer( CBasePlayer *pPlayer );
	virtual BOOL AddDuplicate( CBasePlayerItem *pItem );

	virtual BOOL ExtractAmmo( CBasePlayerWeapon *pWeapon ); //{ return TRUE; }			// Return TRUE if you can add ammo to yourself when picked up
	virtual BOOL ExtractClipAmmo( CBasePlayerWeapon *pWeapon );// { return TRUE; }			// Return TRUE if you can add ammo to yourself when picked up

	//MODDD -  impossible to return FALSE but still expected to do something (ExtractAmmo)?  Works though.
	virtual BOOL AddWeapon( void ) {
		ExtractAmmo( this );
		return TRUE;
	}	// Return TRUE if you want to add yourself to the player

	// generic "shared" ammo handlers
	//MODDD - "AddPrimaryAmmo" has been made "virtual" so that overriden methods in child classes get priority.
	virtual BOOL AddPrimaryAmmo( int iCount, char *szName, int iMaxClip, int iMaxCarry );
	virtual BOOL AddPrimaryAmmo( int iCount, char *szName, int iMaxClip, int iMaxCarry, int forcePickupSound );
	
	BOOL AddSecondaryAmmo( int iCount, char *szName, int iMaxCarry );

	virtual void UpdateItemInfo( void ) {}	// updates HUD state

	virtual BOOL PlayEmptySound( void );
	virtual void ResetEmptySound( void );

	virtual void SendWeaponAnim( int iAnim, int skiplocal = 1, int body = 0 );  // skiplocal is 1 if client is predicting weapon animations
	virtual void SendWeaponAnimReverse( int iAnim, int skiplocal = 1, int body = 0 ); 
	//MODDD - new version that works more like a direct client call... does server and client.
	virtual void SendWeaponAnimBypass(int iAnim, int body = 0);
	virtual void SendWeaponAnimBypassReverse(int iAnim, int body = 0);
	//THIS is a client-only call. Careful.
	virtual void SendWeaponAnimClientOnly(int iAnim, int body = 0);
	virtual void SendWeaponAnimClientOnlyReverse(int iAnim, int body = 0);

	virtual void SendWeaponAnimServerOnly(int iAnim, int body = 0);
	virtual void SendWeaponAnimServerOnlyReverse(int iAnim, int body = 0);

	virtual void SendWeaponAnimMessageFromServer(int iAnim, int body);


	virtual BOOL CanDeploy( void );
	virtual BOOL IsUseable( void );

	//MODDD - see implementation.
	BOOL Deploy();

	//MODDD - new parameter (optional): "delayAnimTime".  Really a custom time for starting weapon idle animations (trying to play an idle anim during a deploy will ignore the idle call, causing the deploy anim, on finishing, to just be stalled while it is IMPLIED the idle anim is playing (the delay set), but it is not (the gun is just static).
	//BOOL DefaultDeploy( char *szViewModel, char *szWeaponModel, int iAnim, char *szAnimExt, int skiplocal = 0, int body = 0 );
	BOOL DefaultDeploy( char *szViewModel, char *szWeaponModel, int iAnim, char *szAnimExt, int skiplocal = 0, int body = 0, float deployAnimTime = 1.0, float fireDelayTime = 0.5 );
	
	void DefaultHolster(int iAnim, int skiplocal = 0, int body = 0, float holsterAnimTime = 1.0);

	int DefaultReload( int iClipSize, int iAnim, float fDelay, int body = 0 );

	//MODDD - added so that "hl_weapons.cpp" in clientside may override it.  But it will have to be called over there to do anything.
	virtual void ItemPreFrame( void );

	virtual void ItemPostFrame( void );	// called each frame by the player PostThink
	// called by CBasePlayerWeapons ItemPostFrame()

	//MODDD - new.
	virtual void ItemPostFrameThink(void);

	
	//MODDD
	virtual void customAttachToPlayer(CBasePlayer *pPlayer ) {}


	
	virtual void PrimaryAttack( void ) { return; }				// do "+ATTACK"
	virtual void SecondaryAttack( void ) { return; }			// do "+ATTACK2"
	virtual void Reload( void ) { return; }						// do "+RELOAD"
	virtual void WeaponIdle( void ) { return; }					// called when no buttons pressed
	virtual int UpdateClientData( CBasePlayer *pPlayer );		// sends hud info to client dll, if things have changed
	virtual void RetireWeapon( void );
	virtual BOOL ShouldWeaponIdle( void ) {return FALSE; }
	virtual void Holster( int skiplocal = 0 );
	virtual BOOL UseDecrement( void ) { return FALSE; }

	//MODDD - new
	virtual void PrimaryNotHeld( void ) { return; }
	virtual void SecondaryNotHeld( void ) { return; }
	virtual void NeitherHeld( void ) { return; }
	virtual void BothHeld( void ) { return; }
	
	int PrimaryAmmoIndex(void);
	int SecondaryAmmoIndex(void);
	
	///MODDD - beats me why these didn't exist up until this point.
	int PlayerPrimaryAmmoCount(void);
	int PlayerSecondaryAmmoCount(void);
	void ChangePlayerPrimaryAmmoCount(int changeBy);
	void ChangePlayerSecondaryAmmoCount(int changeBy);
	void SetPlayerPrimaryAmmoCount(int changeBy);
	void SetPlayerSecondaryAmmoCount(int changeBy);


	void PrintState( void );
	
	//MODDD - new
	virtual const char* GetPickupWalkerName(void);
	virtual CBaseEntity* pickupWalkerReplaceCheck();
	//MODDD - new event, called alongside a reload changing the ammo counts.
	virtual void OnReloadApply(void);
	virtual void OnAddPrimaryAmmoAsNewWeapon(void);

	//MODDD - new general event for easy compatability with a commonly wanted feature (getting the player's
	// FOV choice given a possible influencing CVar).
	// Wanted to make this inline with 'ifdef's for CLIENT_DLL and not, but the player has to be defined at this point.
	// So see implementations in cl_dlls/hl/hl_baseentity.cpp and dlls/weapons.cpp.
	float getPlayerBaseFOV(void);
	//MODDD - get 'framesSinceRestore' between client and serverside.
	int getFramesSinceRestore(void);



	virtual CBasePlayerItem *GetWeaponPtr( void ) { return (CBasePlayerItem *)this; }

	//MODDD - NEW. Convenience method, set both attack delays.
	inline void SetAttackDelays(float targetTime) {
		m_flNextPrimaryAttack = targetTime;
		m_flNextSecondaryAttack = targetTime;
	}

	//MODDD - new var.  Like "pev->button" from the player, but accounts for whether firing with the weapon is allowed at the moment (can't be done in weapons themselves, only in weapons.cpp)
	// Are this and bothFireButtonsMode still used/necessary?
	int buttonFiltered;


	int bothFireButtonsMode;
	//0 = can not press both at the same time (nothing happens, not even NeitherHeld).
	//1 = can not press both at the same time (NeitherHeld() is called only).
	//2 = usual behavior: only "secondaryPressed" is called, "primaryNotPressed" forced.
	//3 = same as 2, but "primaryNotPressed" is not called.
	//4 = "bothPressed" called only.
	//5 = "bothPressed" called as well as the two "not"'s.




	int m_iPlayEmptySound;
	int m_fFireOnEmpty;		// True when the gun is empty and the player is still holding down the
							// attack key(s)

	float m_flPumpTime;
	int m_fInSpecialReload;									// Are we in the middle of a reload for the shotguns
	float m_flNextPrimaryAttack;								// soonest time ItemPostFrame will call PrimaryAttack
	float m_flNextSecondaryAttack;							// soonest time ItemPostFrame will call SecondaryAttack
	float m_flTimeWeaponIdle;									// soonest time ItemPostFrame will call WeaponIdle
	
	//MODDD - PHASED OUT, use getPrimary/SecondaryAmmoType methods to get
	// ammo-type numbers from the cache instead
	//int m_iPrimaryAmmoType;									// "primary" ammo index into players m_rgAmmo[]
	//int m_iSecondaryAmmoType;								// "secondary" ammo index into players m_rgAmmo[]
	int m_iClip;											// number of shots left in the primary weapon clip, -1 it not used
	int m_iClientClip;										// the last version of m_iClip sent to hud dll
	int m_iClientWeaponState;								// the last version of the weapon state sent to hud dll (is current weapon, is on target)
	int m_fInReload;										// Are we in the middle of a reload;

	int m_iDefaultAmmo;// how much ammo you get when you pick up this weapon as placed by a level designer.
	
};


class CBasePlayerAmmo : public CBaseEntity
{
public:


	virtual void Spawn( void );
	void EXPORT DefaultTouch( CBaseEntity *pOther ); // default weapon touch
	virtual BOOL AddAmmo( CBaseEntity *pOther ) { return TRUE; };

	CBaseEntity* Respawn( void );
	void EXPORT Materialize( void );
};



//MODDD - A few prototypes defined in weapons.cpp (SpawnBlood, DamageDecal, DecalGunshot) moved to util.cpp.



//=========================================================
// CWeaponBox - a single entity that can store weapons
// and ammo. 
//=========================================================
class CWeaponBox : public CBaseEntity
{
	void Precache( void );
	void Spawn( void );
	void Touch( CBaseEntity *pOther );
	void KeyValue( KeyValueData *pkvd );
	BOOL IsEmpty( void );
	//MODDD - uses const char* now
	int GiveAmmo( int iCount, const char* szName, int iMax, int *pIndex = NULL );
	void SetObjectCollisionBox( void );

public:
	void EXPORT Kill ( void );
	int	Save( CSave &save );
	int	Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];
	
	BOOL HasWeapon( CBasePlayerItem *pCheckItem );
	BOOL PackWeapon( CBasePlayerItem *pWeapon );
	BOOL PackAmmo( int iszName, int iCount );
	
	CBasePlayerItem	*m_rgpPlayerItems[MAX_ITEM_TYPES];// one slot for each 

	int m_rgiszAmmo[MAX_AMMO_TYPES];// ammo names
	int m_rgAmmo[MAX_AMMO_TYPES];// ammo quantities

	int m_cAmmoTypes;// how many ammo types packed into this box (if packed by a level designer)
};


// MODDD - all weapon and weapon-projectile classes that used to be here moved
// to their own .h files (weapon-projectiles often but not always in the same .h files)


#endif // WEAPONS_H
