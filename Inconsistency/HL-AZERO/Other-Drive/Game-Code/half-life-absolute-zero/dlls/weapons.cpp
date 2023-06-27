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

===== weapons.cpp ========================================================

  functions governing the selection/use of weapons for players

*/

// MODDD - HUGE NOTE HERE!
// The inner machinations of the drive behind Half-Life's devepers remain an enigma.
// Anyway, looks like all specific player weapons (mp5.cpp, crossbow.cpp, etc.) were included server AND clientside.
// weapons.h is also shared.
// HOWEVER.  weapons.cpp is only included serverside, NOT clientside.
// A lot of its methods seem to be implemented separately in cl_dlls/hl/hl_weapons.cpp, alongside some other things.

// The point of this is, don't do 'ifdef CLIENT_DLL' checks in here, they are pointless.  They either always pass
// or never pass, because there's only one reading of weapons.cpp:  serverside.
// Make separate copies for clientside elsewhere over in cl_dlls/hl/hl_weapons.cpp, probably.
// Dummy out anything needed by weapons clientside (like FOV requests) at your own peril!

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "basemonster.h"
#include "weapons.h"
#include "nodes.h"
#include "soundent.h"
#include "decals.h"
#include "gamerules.h"
#include "pickupwalker.h"


EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteammo)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteclip)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(firstPersonIdleDelayMin)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(firstPersonIdleDelayMax)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(viewModelPrintouts)
//EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(viewModelSyncFixPrintouts)
//EASY_CVAR_EXTERN(cl_holster)
//EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST(wpn_glocksilencer)


DLL_GLOBAL short g_sModelIndexLaser;// holds the index for the laser beam
DLL_GLOBAL const char *g_pModelNameLaser = "sprites/laserbeam.spr";
DLL_GLOBAL short g_sModelIndexLaserDot;// holds the index for the laser beam dot
DLL_GLOBAL short g_sModelIndexFireball;// holds the index for the fireball
DLL_GLOBAL short g_sModelIndexSmoke;// holds the index for the smoke cloud
DLL_GLOBAL short g_sModelIndexWExplosion;// holds the index for the underwater explosion
DLL_GLOBAL short g_sModelIndexBloodDrop;// holds the sprite index for the initial blood
DLL_GLOBAL short g_sModelIndexBloodSpray;// holds the sprite index for splattered blood


extern int gmsgCurWeapon;
MULTIDAMAGE gMultiDamage;
extern CGraph WorldGraph;
extern int gEvilImpulse101;
//MODDD - moved g_sModelIndexBubbles to util_shared.cpp.


// LAZY WAY.  Global flag turned on if coming from a call adding a new weapon.
// Only new weapons can put ammo into the clip of the player weapon directly, reload required otherwise
// (retail behavior is to put ammo into the clip from pickups if there is 0 ammo in the clip already).
BOOL g_isNewWeapon = FALSE;



// oops, not used here. see player.cpp.
// Not even global serverside (for the player now)
//extern int g_framesSinceRestore;





//MODDD - CBasePlayerItem::ItemInfoArray and CBasePlayerItem::AmmoInfoArray implementations moved
// to util_shared.cpp.
// MaxAmmoCarry also moved to util_shared.cpp.

	
/*
==============================================================================

MULTI-DAMAGE

Collects multiple small damages into a single damage

==============================================================================
*/

//
// ClearMultiDamage - resets the global multi damage accumulator
//
void ClearMultiDamage(void)
{
	gMultiDamage.pEntity = NULL;
	gMultiDamage.amount	= 0;
	gMultiDamage.type = 0;
	gMultiDamage.typeMod = 0;
}


//
// ApplyMultiDamage - inflicts contents of global multi damage register on gMultiDamage.pEntity
//
// GLOBALS USED:
//		gMultiDamage

void ApplyMultiDamage(entvars_t *pevInflictor, entvars_t *pevAttacker )
{
	Vector		vecSpot1;//where blood comes from
	Vector		vecDir;//direction blood should go
	TraceResult	tr;
	
	if ( !gMultiDamage.pEntity )
		return;

	//easyForcePrintLine("ApplyMultiDamage?? inf:%s att:%s victim(pent):%s dmgtotal:%.2f", pevInflictor!=NULL?STRING(pevInflictor->classname):"NULL", pevAttacker!=NULL?STRING(pevAttacker->classname):"NULL", gMultiDamage.pEntity->getClassname(), gMultiDamage.amount);

	gMultiDamage.pEntity->TakeDamage(pevInflictor, pevAttacker, gMultiDamage.amount, gMultiDamage.type, gMultiDamage.typeMod );
}


// GLOBALS USED:
//		gMultiDamage
//MODDD - 2nd bitmask added.
void AddMultiDamage( entvars_t *pevInflictor, CBaseEntity *pEntity, float flDamage, int bitsDamageType)
{
	AddMultiDamage(pevInflictor, pEntity, flDamage, bitsDamageType, 0);
}
void AddMultiDamage( entvars_t *pevInflictor, CBaseEntity *pEntity, float flDamage, int bitsDamageType, int bitsDamageTypeMod)
{
	if ( !pEntity )
		return;
	
	//easyForcePrintLine("ADDMultiDamge?? inf:%s victim:%s dmg:%.2f", STRING(pevInflictor->classname), pEntity->getClassname(), flDamage);

	if ( pEntity != gMultiDamage.pEntity )
	{
		//easyForcePrintLine("ADDMultiDamge?? WARNING: OUT OF SYNCH. Victim does not match gMultiDamage.pEntity:%s", gMultiDamage.pEntity!=NULL?STRING(gMultiDamage.pEntity->pev->classname):"NULL" );
		//easyForcePrintLine("CALLING MULTIDAMAGEAPPLY: dmg:%.2f", gMultiDamage.amount);
		ApplyMultiDamage(pevInflictor,pevInflictor); // UNDONE: wrong attacker!
		gMultiDamage.pEntity = pEntity;
		gMultiDamage.amount = 0;
	}

	//MODDD - the below used to occur above the "pEntity" check above.
	gMultiDamage.type |= bitsDamageType;
	gMultiDamage.typeMod |= bitsDamageTypeMod;
	////////////////////////////////////////////////////////////////////
	
	//easyForcePrintLine("gMultiDamage: before damage addition dmg:%.2f adding:%.2f", gMultiDamage.amount, flDamage); 
	gMultiDamage.amount += flDamage;
	//easyForcePrintLine("gMultiDamage: after damage addition dmg:%.2f", gMultiDamage.amount);
}

//MODDD - several methods that seem more fitting for util.h moved there:
// SpawnBlood
// DamageDecal
// DecalGunshot
// EjectBrass
// ExplodeModel


// MODDD - moved to util_shared.cpp.
//int giAmmoIndex = 0;
// MODDD - AddAmmoNameToAmmoRegistry moved to util_shared.cpp for access
// between client and serverside.  May as well be able to do both in both places.

 
TYPEDESCRIPTION	CBasePlayerItem::m_SaveData[] = 
{
	DEFINE_FIELD( CBasePlayerItem, m_pPlayer, FIELD_CLASSPTR ),
	DEFINE_FIELD( CBasePlayerItem, m_pNext, FIELD_CLASSPTR ),
	//DEFINE_FIELD( CBasePlayerItem, m_fKnown, FIELD_INTEGER ),Reset to zero on load
	DEFINE_FIELD( CBasePlayerItem, m_iId, FIELD_INTEGER ),
	// DEFINE_FIELD( CBasePlayerItem, m_iIdPrimary, FIELD_INTEGER ),
	// DEFINE_FIELD( CBasePlayerItem, m_iIdSecondary, FIELD_INTEGER ),
};
IMPLEMENT_SAVERESTORE( CBasePlayerItem, CBaseAnimating );


TYPEDESCRIPTION	CBasePlayerWeapon::m_SaveData[] = 
{
#if defined( CLIENT_WEAPONS )
	DEFINE_FIELD( CBasePlayerWeapon, m_flNextPrimaryAttack, FIELD_FLOAT ),
	DEFINE_FIELD( CBasePlayerWeapon, m_flNextSecondaryAttack, FIELD_FLOAT ),
	DEFINE_FIELD( CBasePlayerWeapon, m_flTimeWeaponIdle, FIELD_FLOAT ),
#else	// CLIENT_WEAPONS
	DEFINE_FIELD( CBasePlayerWeapon, m_flNextPrimaryAttack, FIELD_TIME ),
	DEFINE_FIELD( CBasePlayerWeapon, m_flNextSecondaryAttack, FIELD_TIME ),
	DEFINE_FIELD( CBasePlayerWeapon, m_flTimeWeaponIdle, FIELD_TIME ),
#endif	// CLIENT_WEAPONS
	//MODDD - phased out
	//DEFINE_FIELD( CBasePlayerWeapon, m_iPrimaryAmmoType, FIELD_INTEGER ),
	//DEFINE_FIELD( CBasePlayerWeapon, m_iSecondaryAmmoType, FIELD_INTEGER ),
	DEFINE_FIELD( CBasePlayerWeapon, m_iClip, FIELD_INTEGER ),
	DEFINE_FIELD( CBasePlayerWeapon, m_iDefaultAmmo, FIELD_INTEGER ),
//	DEFINE_FIELD( CBasePlayerWeapon, m_iClientClip, FIELD_INTEGER )	 , reset to zero on load so hud gets updated correctly
//  DEFINE_FIELD( CBasePlayerWeapon, m_iClientWeaponState, FIELD_INTEGER ), reset to zero on load so hud gets updated correctly
};

//IMPLEMENT_SAVERESTORE( CBasePlayerWeapon, CBasePlayerItem );

int CBasePlayerWeapon::Save( CSave &save )
{
	if ( !CBasePlayerItem::Save(save) )
		return 0;
	return save.WriteFields( "CBasePlayerWeapon", this, m_SaveData, ARRAYSIZE(m_SaveData) );
}
int CBasePlayerWeapon::Restore( CRestore &restore )
{
	if ( !CBasePlayerItem::Restore(restore) )
		return 0;
	
	int result = restore.ReadFields( "CBasePlayerWeapon", this, m_SaveData, ARRAYSIZE(m_SaveData) );

	// not working out so great.
	/*
	//weapons want to do this.
	if(m_pPlayer != NULL){
		m_pPlayer->forceNoWeaponLoop = TRUE;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + randomIdleAnimationDelay();
	}
	*/

	return result;
}



void CBasePlayerItem::SetObjectCollisionBox( void )
{
	pev->absmin = pev->origin + Vector(-24, -24, 0);
	pev->absmax = pev->origin + Vector(24, 24, 16); 
}


//=========================================================
// Sets up movetype, size, solidtype for a new weapon. 
//=========================================================
void CBasePlayerItem::FallInit( void )
{
	pev->movetype = MOVETYPE_TOSS;
	pev->solid = SOLID_BBOX;

	UTIL_SetOrigin( pev, pev->origin );
	UTIL_SetSize(pev, Vector( 0, 0, 0), Vector(0, 0, 0) );//pointsize until it lands on the ground.
	
	SetTouch( &CBasePlayerItem::DefaultTouch );
	SetThink( &CBasePlayerItem::FallThink );

	pev->nextthink = gpGlobals->time + 0.1;
}

//=========================================================
// FallThink - Items that have just spawned run this think
// to catch them when they hit the ground. Once we're sure
// that the object is grounded, we change its solid type
// to trigger and set it in a large box that helps the
// player get it.
//=========================================================
void CBasePlayerItem::FallThink ( void )
{
	pev->nextthink = gpGlobals->time + 0.1;

	if ( pev->flags & FL_ONGROUND )
	{
		// clatter if we have an owner (i.e., dropped by someone)
		// don't clatter if the gun is waiting to respawn (if it's waiting, it is invisible!)
		if ( !FNullEnt( pev->owner ) )
		{
			int pitch = 95 + RANDOM_LONG(0,29);
			EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "items/weapondrop1.wav", 1, ATTN_NORM, 0, pitch);	
		}

		// lie flat
		pev->angles.x = 0;
		pev->angles.z = 0;

		Materialize(); 
	}
}

//=========================================================
// Materialize - make a CBasePlayerItem visible and tangible
//=========================================================
void CBasePlayerItem::Materialize( void )
{
	if ( pev->effects & EF_NODRAW )
	{
		// changing from invisible state to visible.
		UTIL_PlaySound( ENT(pev), CHAN_WEAPON, "items/suitchargeok1.wav", 1, ATTN_NORM, 0, 150, FALSE );
		pev->effects &= ~EF_NODRAW;
		pev->effects |= EF_MUZZLEFLASH;
	}

	pev->solid = SOLID_TRIGGER;

	UTIL_SetOrigin( pev, pev->origin );// link into world.
	SetTouch (&CBasePlayerItem::DefaultTouch);
	SetThink (NULL);

}

//=========================================================
// AttemptToMaterialize - the item is trying to rematerialize,
// should it do so now or wait longer?
//=========================================================
void CBasePlayerItem::AttemptToMaterialize( void )
{
	float time = g_pGameRules->FlWeaponTryRespawn( this );

	if ( time == 0 )
	{
		Materialize();
		return;
	}

	pev->nextthink = gpGlobals->time + time;
}

//=========================================================
// CheckRespawn - a player is taking this weapon, should 
// it respawn?
//=========================================================
void CBasePlayerItem::CheckRespawn ( void )
{
	switch ( g_pGameRules->WeaponShouldRespawn( this ) )
	{
	case GR_WEAPON_RESPAWN_YES:
		Respawn();
		break;
	case GR_WEAPON_RESPAWN_NO:
		return;
		break;
	}
}

//=========================================================
// Respawn- this item is already in the world, but it is
// invisible and intangible. Make it visible and tangible.
//=========================================================
CBaseEntity* CBasePlayerItem::Respawn( void )
{
	// make a copy of this weapon that is invisible and inaccessible to players (no touch function). The weapon spawn/respawn code
	// will decide when to make the weapon visible and touchable.
	CBaseEntity *pNewWeapon = CBaseEntity::Create( (char *)STRING( pev->classname ), g_pGameRules->VecWeaponRespawnSpot( this ), pev->angles, pev->owner );

	if ( pNewWeapon )
	{
		//MODDD - remove the automatic "SF_NORESPAWN" given on Create calls.  Clearly this came from a Respawn call and
		// thus should be respawnnable itself.
		pNewWeapon->pev->spawnflags &= ~SF_NORESPAWN;
		

		pNewWeapon->pev->effects |= EF_NODRAW;// invisible for now
		pNewWeapon->SetTouch( NULL );// no touch
		pNewWeapon->SetThink( &CBasePlayerItem::AttemptToMaterialize );

		DROP_TO_FLOOR ( ENT(pev) );

		// not a typo! We want to know when the weapon the player just picked up should respawn! This new entity we created is the replacement,
		// but when it should respawn is based on conditions belonging to the weapon that was taken.
		pNewWeapon->pev->nextthink = g_pGameRules->FlWeaponRespawnTime( this );
	}
	else
	{
		ALERT ( at_console, "Respawn failed to create %s!\n", STRING( pev->classname ) );
	}

	return pNewWeapon;
}



void CBasePlayerItem::DefaultTouchRemoveThink( CBaseEntity *pOther){
	DefaultTouch(pOther);
	SetThink ( NULL );
}//END OF DefaultTouchRemoveThink(...)

void CBasePlayerItem::DefaultTouch( CBaseEntity *pOther )
{
	//easyForcePrintLine("OH NO YOU RRR SONNY");

	// if it's not a player, ignore
	if ( !pOther->IsPlayer() )
		return;

	CBasePlayer *pPlayer = (CBasePlayer *)pOther;

	// can I have this?
	//MODDD - possible exception to "canHavePlayerItem".  If this is a glock with a silencer, and the player has a glock without a silencer, the player can still pick up to receive the silencer.
	if ( !g_pGameRules->CanHavePlayerItem( pPlayer, this ) && !(weaponCanHaveExtraCheck(pPlayer) ) )
	{
		if ( gEvilImpulse101 )
		{
			UTIL_Remove( this );
		}
		return;
	}else{
		//easyForcePrintLine("MOVE ON SONNY");
	}
	
	//easyForcePrintLine("WELLA??? %d", (this->m_pfnThink == &CBasePlayerWeapon::FallThink) );
	//easyForcePrintLine("WELLB??? %d", (this->m_pfnThink == NULL) );
	//easyForcePrintLine("WELLC??? %d", (this->m_pfnThink == &CBaseEntity::SUB_Remove) );

	//NOTICE - This check, "AddPlayerItem" will fail (false) if the player already has the weapon and just adds its ammo to their existing one.
	if (pOther->AddPlayerItem( this ))
	{
		//easyForcePrintLine("OKAY THERE SONNY");
		AttachToPlayer( pPlayer );

		playGunPickupSound(pPlayer->pev);
	}
	
	//easyForcePrintLine("DELLA??? %d", (this->m_pfnThink == &CBasePlayerWeapon::FallThink) );
	//easyForcePrintLine("DELLB??? %d", (this->m_pfnThink == NULL) );
	//easyForcePrintLine("DELLC??? %d", (this->m_pfnThink == &CBaseEntity::SUB_Remove) );

	SUB_UseTargets( pOther, USE_TOGGLE, 0 ); // UNDONE: when should this happen?
}



void CBasePlayerWeapon::setchargeReady(int arg){
	// Replace the first few bits with 'arg', don't reach or exceed 32 for safety.
	// Should only need 0 to 2 anyway.
	// This adds back in bits 64 and/or 128, if present already.
	m_chargeReady = arg | (m_chargeReady & (32 | 64 | 128));
}
int CBasePlayerWeapon::getchargeReady(void){
	// Get without the extra bits.
	return m_chargeReady & ~(32 | 64 | 128);
}
void CBasePlayerWeapon::forceBlockLooping(void){
	m_chargeReady |= 64;
}
void CBasePlayerWeapon::stopBlockLooping(void){
	m_chargeReady &= ~64;
}





CBasePlayerWeapon::CBasePlayerWeapon(void){
	//Starts as 0, not a garbage value, for ALL weapons.
	m_chargeReady = 0;
	
	//bitmask.
	//1 = primary held and ready to fire.
	//2 = secondary held and ready to fire.
	buttonFiltered = 0;

	bothFireButtonsMode = 2;

}

//MODDD
float CBasePlayerWeapon::randomIdleAnimationDelay(void){
	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(firstPersonIdleDelayMin) > 0 && EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(firstPersonIdleDelayMax) > 0){
		//let's go.
		//float rand = RANDOM_FLOAT(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(firstPersonIdleDelayMin), EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(firstPersonIdleDelayMax));
		float rand = UTIL_SharedRandomFloat(m_pPlayer->random_seed, EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(firstPersonIdleDelayMin), EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(firstPersonIdleDelayMax) );
		//easyPrintLine("OK server server %.2f", rand);
		return rand;
	}else{
		return 0;
	}
}//END OF randomIdleAnimationDelay





//MODDD - stub.
void CBasePlayerWeapon::ItemPreFrame( void ){
	//CBasePlayerItem::ItemPreFrame();  necessary?   "ItemPostFrame" does not seem to need to call its parent class's method.
	//empty.
}



void CBasePlayerWeapon::ItemPostFrame( void )
{
	//easyPrintLine("DFFFF %.2f", m_pPlayer->glockSilencerOnVar);
	//easyPrintLine("DFFFF %d", m_pPlayer->pev->button & EXTRA1);
	//easyPrintLine("DFFFF %d", pev->body);
	//easyPrintLine("DFFFF %d", m_fInAttack);

	// HOLSTER - SWAPPO DISABLE.
	/*
	if(m_pPlayer->m_bHolstering){  //m_pPlayer->m_flNextAttack <= 0.0){
		// done holstering? complete the switch to the picked weapon. Deploy that one.
		m_pPlayer->m_bHolstering = FALSE;
		m_chargeReady &= ~128;
		m_pPlayer->m_pQueuedActiveItem->m_chargeReady &= ~128;
		m_pPlayer->setActiveItem(m_pPlayer->m_pQueuedActiveItem);

		m_pPlayer->m_pQueuedActiveItem = NULL;
		m_pPlayer->m_fCustomHolsterWaitTime = -1;
		return;
	}
	*/


	BOOL secondaryHeld = ((m_pPlayer->pev->button & IN_ATTACK2) && CanAttack( m_flNextSecondaryAttack, gpGlobals->time, UseDecrement() ));
	BOOL primaryHeld = ((m_pPlayer->pev->button & IN_ATTACK) && CanAttack( m_flNextPrimaryAttack, gpGlobals->time, UseDecrement() ));


	if(primaryHeld && secondaryHeld){
		// This little printout has overstayed its welcome.  Byebye!
		//easyForcePrintLine("YOU HOLD BOTH? YOU DISGUST ME");
		//m_flTimeWeaponIdle = -1;  ???
		WeaponIdle();
		return;   //block!
	}




	/*
	if(primaryHeld){
		buttonFiltered |= 1;
	}else{
		buttonFiltered &= ~(1);
	}
	if(secondaryHeld){
		buttonFiltered |= 2;
	}else{
		buttonFiltered &= ~(2);
	}
	*/

	/*
	m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = 999;
	m_iClip = 999;

	if(pszAmmo2()){
		m_pPlayer->m_rgAmmo[SecondaryAmmoIndex()] = 999;

	}
	*/



	//NOTICE: reverted to old system for now...
	/*
	BOOL canDoNormal = TRUE;

	#if defined( CLIENT_WEAPONS )
		if ( m_pPlayer->m_flNextAttack > 0 )
	#else
		if ( gpGlobals->time < m_flNextAttack )
	#endif
		{
			canDoNormal = FALSE;
		}
		*/
	

	//easyForcePrintLine("I AM VERY VERY oh my");

	//NOTICE: had to mod this method, as it seems to sync better.
	if ((m_fInReload) && ( m_pPlayer->m_flNextAttack <= UTIL_WeaponTimeBase() ) )
	{
		int myPrimaryAmmoType = getPrimaryAmmoType();
		if (IS_AMMOTYPE_VALID(myPrimaryAmmoType)) {
			// complete the reload. 
			int j = min(iMaxClip() - m_iClip, m_pPlayer->m_rgAmmo[myPrimaryAmmoType]);

			// Add them to the clip
			m_iClip += j;

			//MODDD - cheat intervention, if available.  With this cheat, reloading does not consume total ammo.
			if (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteammo) == 0) {
				m_pPlayer->m_rgAmmo[myPrimaryAmmoType] -= j;
			}
			else {
				if (m_pPlayer->m_rgAmmo[myPrimaryAmmoType] == 0) {
					m_pPlayer->m_rgAmmo[myPrimaryAmmoType] = 1;
				}

			}

			m_pPlayer->TabulateAmmo();

			//MODDD - new event!
			this->OnReloadApply();
		}// END OF Valid ammotype check

		m_fInReload = FALSE;
	}
	


	//BOOL canDoNormalPressBehavior = TRUE;
	BOOL canCallBoth = TRUE;
	BOOL canCallPrimaryNot = TRUE;
	BOOL canCallPrimary = TRUE;
	BOOL canCallSecondaryNot = TRUE;
	BOOL canCallSecondary = TRUE;
	BOOL canCallNeither = TRUE;
	/*
	if(secondaryHeld && primaryHeld){

		//0 = can not press both at the same time (nothing happens, not even NeitherHeld).
		//1 = can not press both at the same time (NeitherHeld() is called only).
		//2 = usual behavior: only "secondaryPressed" is called, "primaryNotPressed" forced.
		//3 = same as 2, but "primaryNotPressed" is not called.
		//4 = "bothPressed" called only.
		//5 = "bothPressed" called as well as the two "not"'s.
		
		switch(bothFireButtonsMode){
		case 0:
			//nothing can register, not even "NeitherHeld();":
			canCallBoth = FALSE;
			canCallPrimaryNot = FALSE;
			canCallPrimary = FALSE;
			canCallSecondaryNot = FALSE;
			canCallSecondary = FALSE;
			canCallNeither = FALSE;
		break;
		case 1:
			//nothing can register, EXCEPT for "NeitherHeld();"
			canCallBoth = FALSE;
			canCallPrimaryNot = FALSE;
			canCallPrimary = FALSE;
			canCallSecondaryNot = FALSE;
			canCallSecondary = FALSE;
			canCallNeither = FALSE;
			NeitherHeld();
		break;
		case 2:
			primaryHeld = FALSE;

			canCallBoth = FALSE;
			canCallPrimaryNot = TRUE;
			canCallPrimary = FALSE;
			canCallSecondaryNot = FALSE;
			canCallSecondary = TRUE;
			canCallNeither = FALSE;

		break;
		case 3:
			//primaryHeld = FALSE;

			canCallBoth = FALSE;
			canCallPrimaryNot = FALSE;
			canCallPrimary = FALSE;
			canCallSecondaryNot = FALSE;
			canCallSecondary = TRUE;
			canCallNeither = FALSE;

		break;
		case 4:
			//nothing can register, EXCEPT for "NeitherHeld();"
			canCallBoth = TRUE;
			canCallPrimaryNot = FALSE;
			canCallPrimary = FALSE;
			canCallSecondaryNot = FALSE;
			canCallSecondary = FALSE;
			canCallNeither = FALSE;
		break;

		}//END OF switch(...)

	}//END OF (both held)
	*/

	if(canCallNeither && !secondaryHeld && !primaryHeld){
		NeitherHeld();
	}

	if(canCallBoth && secondaryHeld && primaryHeld){
		BothHeld();
		//canCallBoth = TRUE;
		canCallPrimaryNot = FALSE;
		canCallPrimary = FALSE;
		canCallSecondaryNot = FALSE;
		canCallSecondary = FALSE;
		canCallNeither = FALSE;

		primaryHeld = FALSE;
	}

	
	//easyPrintLine("NOT HELDD!!! %d %d %.2f", canCallPrimaryNot, primaryHeld, gpGlobals->time);
	if(canCallPrimaryNot && !primaryHeld){
		PrimaryNotHeld();
	}
	if(canCallSecondaryNot && !secondaryHeld){
		SecondaryNotHeld();
	}


	//if ((m_pPlayer->pev->button & IN_ATTACK2) && CanAttack( m_flNextSecondaryAttack, gpGlobals->time, UseDecrement() ) )
	if(canCallSecondary && secondaryHeld)
	{
		//MODD - cheat intervention.  This check is only done if the infinite ammo & clip cheats are off (secondary weaps don't use clips).
		if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteammo) == 0 && EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteclip) == 0){
			if ( pszAmmo2() && PlayerSecondaryAmmoCount() <= 0 )
			{
				m_fFireOnEmpty = TRUE;
			}
		}else{
			if ( pszAmmo2() && PlayerSecondaryAmmoCount() < 1 )
			{
				SetPlayerSecondaryAmmoCount(1);
			}
		}

		m_pPlayer->TabulateAmmo();
		SecondaryAttack();

		//MODDD NOTE - ?????
		// Why remove IN_ATTACK2 from the bitmask?  Seems to have no effect,
		// holding right-click with the mp5 still fires AR grenades continuously.
		// Nowhere else removes IN_... flags manually.  weeeeeeiiiiiirrrrrrddd.
		// ***This seems to screw with m_pPlayer->m_afButtonPressed working right (records a button
		// press being this very frame), so disabling this one!  Don't know what they were thinking here.
		// Think about it.  Every frame just resets pev->button to IN_ATTACK2 as it is held down,
		// so this doesn't stop continual fire. It just tricks the game into thinking every single frame
		// is a completley new press.  Which.... makes m_afButtonPressed and Released useless.
		// IN_ATTACK (primary) lacked this line as seen below, and did not have this issue. Case closed.
		//m_pPlayer->pev->button &= ~IN_ATTACK2;
	}
	else if (canCallPrimary && primaryHeld )
	{
		//MODDD - cheat intervention.
		if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteclip) == 0){
			if ( (m_iClip == 0 && pszAmmo1()) || (iMaxClip() == -1 && PrimaryAmmoIndex()!=-1 && PlayerPrimaryAmmoCount() <= 0 ) )
			{
				m_fFireOnEmpty = TRUE;
			}
		}else{
			/*
			if ( (m_iClip == 0 && pszAmmo1()) || (iMaxClip() == -1 && PlayerPrimaryAmmoCount() <= 0 ) )
			{
				SetPlayerPrimaryAmmoCount(1);
			}*/
			
			//-1 means, this weap does not use the "clip" but goes straight to using primary ammo.
			if(m_iClip < 1 && m_iClip != -1 && (iMaxClip() > 0) ){
				m_iClip = 1;
			}
		}

		// Doubt this is necessary anymore, but verify.  Improvements for ammo, boundary checking in lots of places.
		// Verified!
		/*
		if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteammo) == 1){
			//MODDD - this was a nasty bug.
			//(crash by going through the glass-door near the end of the laser room, where you get the 1st crowbar to beat it with & walk through WHILE cheat_infiniteammo is on, solved by this)
			
			if(PrimaryAmmoIndex() != -1 && PlayerPrimaryAmmoCount() < 1){
				SetPlayerPrimaryAmmoCount(1);
			}
		}
		*/
		
		m_pPlayer->TabulateAmmo();
		if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(viewModelPrintouts)==1)easyForcePrintLine("Postframe PrimaryAttack!");
		PrimaryAttack();

	}
	else if ( m_pPlayer->pev->button & IN_RELOAD && iMaxClip() != WEAPON_NOCLIP && !m_fInReload ) 
	{
		// reload when reload is pressed, or if no buttons are down and weapon is empty.
		Reload();
	}
	else if ( !(m_pPlayer->pev->button & (IN_ATTACK|IN_ATTACK2) ) )
	{
		// no fire buttons down

		m_fFireOnEmpty = FALSE;

		if ( !IsUseable() && m_flNextPrimaryAttack < ( UseDecrement() ? 0.0 : gpGlobals->time ) ) 
		{
			// weapon isn't useable, switch.
			if ( !(iFlags() & ITEM_FLAG_NOAUTOSWITCHEMPTY) && g_pGameRules->GetNextBestWeapon( m_pPlayer, this ) )
			{
				easyPrintLineClient(m_pPlayer->edict(), "OH DEAR IT TURNS OUT YOUR WEAPON IS UNUSABLE. LET ME TRY TO PICK A BETTER ONE FOR YOU!");
				m_flNextPrimaryAttack = ( UseDecrement() ? 0.0 : gpGlobals->time ) + 0.3;
				return;
			}
		}
		else
		{

			// weapon is useable. Reload if empty and weapon has waited as long as it has to after firing
			if ( m_iClip == 0 && !(iFlags() & ITEM_FLAG_NOAUTORELOAD) && m_flNextPrimaryAttack < ( UseDecrement() ? 0.0 : gpGlobals->time ) )
			{
				//MODDD - cheat intervention.
				if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteclip) != 1){
					Reload();
				}else{
					if(m_iClip < 1 && m_iClip != -1 && (iMaxClip() > 0) ){
						m_iClip = 1;
					}
				}
				return;
			}
		}

		WeaponIdle( );
		return;
	}
	//MODDD - note: ISSUE.  This method, "itemPostFrame", happens when m_flNextAttack isn't passed yet.  So, this "idle" block is never reached, even if it is okay to.

	// catch all
	if ( ShouldWeaponIdle() )
	{
		WeaponIdle();
	}

}//ItemPostFrame


//MODDD - new.
void CBasePlayerWeapon::ItemPostFrameThink() {
	/*
	//MODDD - should be a good place to check for deplying the next weapon.
	if(m_pActiveItem && m_bHolstering && gpGlobals->time >= m_flNextAttack){
		//done holstering? complete the switch to the picked weapon. Deploy that one.

		m_bHolstering = FALSE;
		setActiveItem(m_pQueuedActiveItem);
		m_pQueuedActiveItem = FALSE;
	}
	*/

	// HOLSTER - SWAPPO DISABLE.
	//MODDD - should be a good place to check for deplying the next weapon.
	if (m_pPlayer->m_bHolstering && gpGlobals->time >= m_pPlayer->m_fCustomHolsterWaitTime) {  //m_pPlayer->m_flNextAttack <= 0.0){
		// done holstering? complete the switch to the picked weapon. Deploy that one.
		m_pPlayer->m_bHolstering = FALSE;
		m_chargeReady &= ~128;

		if (m_pPlayer->m_pQueuedActiveItem != NULL) {
			m_pPlayer->m_pQueuedActiveItem->m_chargeReady &= ~128;
			m_pPlayer->setActiveItem(m_pPlayer->m_pQueuedActiveItem);

			m_pPlayer->m_pQueuedActiveItem = NULL;
		}

		m_pPlayer->m_fCustomHolsterWaitTime = -1;
	}


	CBasePlayerItem::ItemPostFrameThink();
}//ItemPostFrameThink








void CBasePlayerItem::DestroyItem( void )
{
	if ( m_pPlayer )
	{
		// if attached to a player, remove. 
		m_pPlayer->RemovePlayerItem( this );
	}

	Kill( );
}

BOOL CBasePlayerItem::AddToPlayer( CBasePlayer *pPlayer )
{
	m_pPlayer = pPlayer;

	return TRUE;
}

void CBasePlayerItem::Drop( void )
{
	SetTouch( NULL );
	SetThink(&CBaseEntity::SUB_Remove);
	pev->nextthink = gpGlobals->time + .1;
}

void CBasePlayerItem::Kill( void )
{
	SetTouch( NULL );
	SetThink(&CBaseEntity::SUB_Remove);
	pev->nextthink = gpGlobals->time + .1;
}


//MODDD NOTICE - clientside's Holster was found completely dummied out even in the as-is script. Is it ok to avoid
//               copying the additions from here to there then?
void CBasePlayerItem::Holster( int skiplocal /* = 0 */ )
{ 
	m_pPlayer->pev->viewmodel = 0; 
	m_pPlayer->pev->weaponmodel = 0;

	//MODDD - for safety set these things too now, even for base items.
	m_pPlayer->m_flNextAttack = 0;
	//Don't want to risk setting idle animations while holstering, set this to an impossible value. ?
	m_pPlayer->m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5;


}


//IMPORTANT!!! This method is only called when adding to the player for the very first time (not just giving a copy of the weapon's ammo).
void CBasePlayerItem::AttachToPlayer ( CBasePlayer *pPlayer )
{
	//easyForcePrintLine("OH NO YOU ATTACHED SONNY");

	pev->movetype = MOVETYPE_FOLLOW;
	pev->solid = SOLID_NOT;
	pev->aiment = pPlayer->edict();
	pev->effects = EF_NODRAW; // ??
	pev->modelindex = 0;// server won't send down to clients if modelindex == 0
	pev->model = iStringNull;
	pev->owner = pPlayer->edict();
	pev->nextthink = gpGlobals->time + .1;
	SetTouch( NULL );
	


	customAttachToPlayer( pPlayer );

}

//MODDD - BOOL edit
// CALLED THROUGH the newly-touched weapon instance. The existing player weapon is pOriginal
BOOL CBasePlayerWeapon::AddDuplicate( CBasePlayerItem *pOriginal )
{
	g_isNewWeapon = FALSE;

	//easyPrintLine("WHO AIM EYE?! %d : %s", m_iDefaultAmmo, STRING(pOriginal->pev->classname) );
	if ( m_iDefaultAmmo )
	{
		BOOL test = ExtractAmmo( (CBasePlayerWeapon *)pOriginal );
		//easyPrintLine("DOOP3 %d", test);
		return test;
	}
	else
	{
		// a dead player dropped this.
		return ExtractClipAmmo( (CBasePlayerWeapon *)pOriginal );
	}
}


//MODDD - BOOL edit.
// Also, this is only for completely new weapons.  Existing ones ripped for ammo don't use this, they use AddDuplicate.
BOOL CBasePlayerWeapon::AddToPlayer( CBasePlayer *pPlayer )
{
	g_isNewWeapon = TRUE;

	BOOL bResult = CBasePlayerItem::AddToPlayer( pPlayer );

	//MODDD NOTE - is it safe to make this mask change (adjustment) before we've even confirmed that Adding this weapon to the player is allowed?
	pPlayer->pev->weapons |= (1<<m_iId);

	//MODDD - in case ammo types were to ever start at 0 intead, watch out for places like this.
	// Re-gets the ammo index on finding 0.  Also have IS_AMMOTYPE_VALID but eh, unnecessary here.
	// CHANGE: No longer possible, these are cached at startup.  If it is 0 here we just don't have an ammo type, re-gets are senseless.
	/*
	if ( m_iPrimaryAmmoType == 0 )
	{
		//MODDD - method no longer belongs to player, no point.
		// And cached anyway.  Do we even need m_iPrimary/SecondaryAmmoType anymore?
		//m_iPrimaryAmmoType = GetAmmoIndex( pszAmmo1() );
		//m_iSecondaryAmmoType = GetAmmoIndex( pszAmmo2() );
		m_iPrimaryAmmoType = getPrimaryAmmoType();
		m_iSecondaryAmmoType = getSecondaryAmmoType();
	}
	*/


	if (bResult) {
		// 'AddWeapon()' includes an ExtractAmmo() call.  Always.  Tricky tricky tricky...
		BOOL theResult = AddWeapon();

		g_isNewWeapon = FALSE;   // safety.

		return theResult;
	}
	return FALSE;
}

int CBasePlayerWeapon::UpdateClientData( CBasePlayer *pPlayer )
{
	BOOL bSend = FALSE;
	int state = 0;
	if ( pPlayer->m_pActiveItem == this )
	{
		if ( pPlayer->m_fOnTarget )
			state = WEAPON_IS_ONTARGET;
		else
			state = 1;
	}

	// Forcing send of all data!
	if ( !pPlayer->m_fWeapon )
	{
		bSend = TRUE;
	}
	
	// This is the current or last weapon, so the state will need to be updated
	if ( this == pPlayer->m_pActiveItem ||
		 this == pPlayer->m_pClientActiveItem )
	{
		if ( pPlayer->m_pActiveItem != pPlayer->m_pClientActiveItem )
		{
			bSend = TRUE;
		}
	}

	// If the ammo, state, or fov has changed, update the weapon
	if ( m_iClip != m_iClientClip || 
		 state != m_iClientWeaponState || 
		 pPlayer->m_iFOV != pPlayer->m_iClientFOV )
	{
		bSend = TRUE;
	}

	if ( bSend )
	{
		//MODDD - wait. Why were m_iId & m_iClip written as BYTE when MsgFunc_CurWeapon reads them as CHAR?
		// Making them written as CHAR here.  just, what.
		MESSAGE_BEGIN( MSG_ONE, gmsgCurWeapon, NULL, pPlayer->pev );
			WRITE_BYTE( state );
			WRITE_CHAR( m_iId );
			WRITE_CHAR( m_iClip );
		MESSAGE_END();

		m_iClientClip = m_iClip;
		m_iClientWeaponState = state;
		pPlayer->m_fWeapon = TRUE;
	}

	if ( m_pNext )
		m_pNext->UpdateClientData( pPlayer );

	return 1;
}




void CBasePlayerWeapon::SendWeaponAnim( int iAnim, int skiplocal, int body )
{
	//MODDD - that does it.  Screw this.
	SendWeaponAnimBypass(iAnim, body);
	return;


	if ( UseDecrement() )
		skiplocal = 1;
	else
		skiplocal = 0;

	//So much as this action can trigger a sendoff apparently.
	m_pPlayer->pev->weaponanim = iAnim;

#if defined( CLIENT_WEAPONS )
	if ( skiplocal && ENGINE_CANSKIP( m_pPlayer->edict() ) ){
		if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(viewModelPrintouts)==1)easyForcePrintLine("SendWeaponAnim: %d BLOCKED BY ClientSkip.", iAnim);
		// WARNING!!! Restored clientskip, seems we may have expected this to stop things at an earlier point.
		return;
	}
#endif

	// Moved below the CLIENTSKIP above.
	//m_pPlayer->pev->weaponanim = iAnim;


	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(viewModelPrintouts)==1)easyForcePrintLine("SendWeaponAnim: %d", iAnim);

	this->m_fireState &= ~128;

	//MODDD - turned into a method for breakpoint convenience.
	SendWeaponAnimMessageFromServer(iAnim, body);
	
}


void CBasePlayerWeapon::SendWeaponAnimReverse( int iAnim, int skiplocal, int body )
{
	//MODDD - that does it.  Screw this.
	SendWeaponAnimBypassReverse(iAnim, body);
	return;


	if ( UseDecrement() )
		skiplocal = 1;
	else
		skiplocal = 0;

	iAnim |= 128;
	this->m_fireState |= 128;


	//So much as this action can trigger a sendoff apparently.
	m_pPlayer->pev->weaponanim = iAnim;

#if defined( CLIENT_WEAPONS )
	if ( skiplocal && ENGINE_CANSKIP( m_pPlayer->edict() ) ){
		if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(viewModelPrintouts)==1)easyForcePrintLine("SendWeaponAnimReverse: %d BLOCKED BY ClientSkip.", iAnim);
		return;
	}
#endif

	// Moved below the CLIENTSKIP above.
	//m_pPlayer->pev->weaponanim = iAnim;

	
	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(viewModelPrintouts)==1)easyForcePrintLine("SendWeaponAnimReverse: %d", iAnim);
	
	SendWeaponAnimMessageFromServer(iAnim, body);
}





//MODDD
void CBasePlayerWeapon::SendWeaponAnimBypass( int iAnim, int body )
{
	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(viewModelPrintouts)==1)easyForcePrintLine("SendWeaponAnimBypass: %d", iAnim);
	this->m_fireState &= ~128;
	
	m_pPlayer->pev->weaponanim = iAnim;
	SendWeaponAnimMessageFromServer(iAnim, body);
}

//MODDD
void CBasePlayerWeapon::SendWeaponAnimBypassReverse( int iAnim, int body )
{
	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(viewModelPrintouts)==1)easyForcePrintLine("SendWeaponAnimBypassReverse: %d", iAnim);
	this->m_fireState |= 128;

	iAnim |= 128;
	
	m_pPlayer->pev->weaponanim = iAnim;
	SendWeaponAnimMessageFromServer(iAnim, body);

}




void CBasePlayerWeapon::SendWeaponAnimClientOnly( int iAnim, int body )
{
	//Server? No you don't.
}

//MODDD
void CBasePlayerWeapon::SendWeaponAnimClientOnlyReverse( int iAnim, int body )
{
	//Server? no.
}


//MODDD
void CBasePlayerWeapon::SendWeaponAnimServerOnly( int iAnim, int body )
{
	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(viewModelPrintouts)==1)easyForcePrintLine("SendWeaponAnimServerOnly: %d raw:%d", iAnim, iAnim);
	this->m_fireState &= ~128;
	
	m_pPlayer->pev->weaponanim = iAnim;
	SendWeaponAnimMessageFromServer(iAnim, body);
	
}

//MODDD
void CBasePlayerWeapon::SendWeaponAnimServerOnlyReverse( int iAnim, int body )
{
	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(viewModelPrintouts)==1)easyForcePrintLine("SendWeaponAnimServerOnlyReverse: %d raw: %d", iAnim, iAnim | 128);
	this->m_fireState |= 128;
	
	iAnim |= 128;

	m_pPlayer->pev->weaponanim = iAnim;
	SendWeaponAnimMessageFromServer(iAnim, body);
	
}

void CBasePlayerWeapon::SendWeaponAnimMessageFromServer(int iAnim, int body) {
	MESSAGE_BEGIN(MSG_ONE, SVC_WEAPONANIM, NULL, m_pPlayer->pev);
		WRITE_BYTE(iAnim);						// sequence number
		//WRITE_BYTE( pev->body );					// weaponmodel bodygroup.
		//...wait. We provided a "body", didn't we? Why send the pev->body the weapon has instead of that???
		WRITE_BYTE(body);					// weaponmodel bodygroup.
	MESSAGE_END();

	pev->body = body;  // is this a good idea?
}







//MODDD - new version.


BOOL CBasePlayerWeapon::AddPrimaryAmmo( int iCount, char *szName, int iMaxClip, int iMaxCarry ){
	return AddPrimaryAmmo(iCount, szName, iMaxClip, iMaxCarry, 0);
}

//MODDD - called for either extra weapons OR ammo itself adding the ammunition
BOOL CBasePlayerWeapon::AddPrimaryAmmo( int iCount, char *szName, int iMaxClip, int iMaxCarry, int forcePickupSound )
{
	int iIdAmmo;

	//easyPrintLine("PLAYER NULL 3??? %d", m_pPlayer == NULL);
	int myPrimaryAmmoType = getPrimaryAmmoType();

	if (g_isNewWeapon) {
		// let the weapon know
		OnAddPrimaryAmmoAsNewWeapon();
	}


	//MODDD - sending myPrimaryAmmoType instead of szName throughout, consider changing the whole method sometime
	if (iMaxClip < 1)
	{
		//MODDD - ...  why here?  Leave this up to the weapon in spawn.
		// Reload logic doesn't play well with this when it's still needed regardless (rpg clipless logic hack).
		//m_iClip = -1;

		iIdAmmo = m_pPlayer->GiveAmmoID( iCount, myPrimaryAmmoType, iMaxCarry );
	}
	//MODDD - nope!  Ammo to an existing weapon always goes straight to the pool.
	// Reload you lazy bastard.
	else if (m_iClip == 0 && g_isNewWeapon == TRUE)
	{
		int i;
		i = min( m_iClip + iCount, iMaxClip ) - m_iClip;
		m_iClip += i;
		iIdAmmo = m_pPlayer->GiveAmmoID( iCount - i, myPrimaryAmmoType, iMaxCarry );
	}
	else
	{
		iIdAmmo = m_pPlayer->GiveAmmoID( iCount, myPrimaryAmmoType, iMaxCarry );
	}
	
	// m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = iMaxCarry; // hack for testing

	//MODDD - play the pickup sound if it's forced!
	if(forcePickupSound == TRUE ){
		playAmmoPickupSound();
	}else if(forcePickupSound == 2){
		this->playGunPickupSound(m_pPlayer->pev);
	}


	// NOTE - couldn't this just be played by the 'forcePickupSound' check and nothing else?
	// What's the point of the rest of this?
	/*
	if (iIdAmmo > 0 )//|| weaponPlayPickupSoundException(m_pPlayer))
	{
		//MODDD - now wait just a moment.
		// Shouldn't a weapon know its AmmoType numbers well before receiving ammo like this?
		// Why is it being set by the GiveAmmo calls above.
		// I...     I don't.  I can't.  (changed since, does not re-find the AmmoType numbers, uses cached version from startup)
		// In fact, phased out.  Byebye!
		//m_iPrimaryAmmoType = iIdAmmo;

		if (m_pPlayer->HasPlayerItem( this ) )
		{
			// play the "got ammo" sound only if we gave some ammo to a player that already had this gun.
			// if the player is just getting this gun for the first time, DefaultTouch will play the "picked up gun" sound for us.
			
			//don't play here if we have already.
			if(forcePickupSound == FALSE){
				playAmmoPickupSound();
			}

			//EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);

		}
	}
	*/
	//MODDD - done.  See if this causes any problems, mainly playing the sound when it shouldn't.
	if (forcePickupSound == FALSE) {
		playAmmoPickupSound();
	}


	return iIdAmmo > 0 ? TRUE : FALSE;
}


BOOL CBasePlayerWeapon::AddSecondaryAmmo( int iCount, char *szName, int iMax )
{
	int iIdAmmo;

	int mySecondaryAmmoType = getSecondaryAmmoType();

	//MODDD - sending mySecondaryAmmoType instead of szName throughout, consider changing the whole method sometime
	iIdAmmo = m_pPlayer->GiveAmmoID( iCount, mySecondaryAmmoType, iMax );

	//m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = iMax; // hack for testing

	if (iIdAmmo > 0)
	{
		//MODDD - phased out, byebye
		//m_iSecondaryAmmoType = iIdAmmo;

		playAmmoPickupSound();
		//EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
	}
	return iIdAmmo > 0 ? TRUE : FALSE;
}

//=========================================================
// IsUseable - this function determines whether or not a 
// weapon is useable by the player in its current state. 
// (does it have ammo loaded? do I have any ammo for the 
// weapon?, etc)
//=========================================================
BOOL CBasePlayerWeapon::IsUseable( void )
{
	if ( m_iClip <= 0 )
	{
		if ( PlayerPrimaryAmmoCount() <= 0 && iMaxAmmo1() != -1 )			
		{
			// clip is empty (or nonexistant) and the player has no more ammo of this type. 
			return FALSE;
		}
	}

	return TRUE;
}

BOOL CBasePlayerWeapon::CanDeploy( void )
{
	BOOL bHasAmmo = 0;

	//MODDD - TODO. Replace with a reference to a better method for ya ma.
	if ( !pszAmmo1() )
	{
		// this weapon doesn't use ammo, can always deploy.
		return TRUE;
	}


	//MODDD - if this weapon doesn't require ammo, also allow.
	if (iFlags() & ITEM_FLAG_SELECTONEMPTY) {
		// also allow unconditionally.
		return TRUE;
	}



	if ( pszAmmo1() )
	{
		int ammoIndex = getPrimaryAmmoType();
		if (IS_AMMOTYPE_VALID(ammoIndex)) {
			bHasAmmo |= (m_pPlayer->m_rgAmmo[ammoIndex] != 0);
		}
		else {
			// oh.
		}
	}
	if ( pszAmmo2() )
	{
		int ammoIndex = getSecondaryAmmoType();
		if (IS_AMMOTYPE_VALID(ammoIndex)) {
			bHasAmmo |= (m_pPlayer->m_rgAmmo[ammoIndex] != 0);
		}
		else {
			// oh.
		}
	}
	if (m_iClip > 0)
	{
		bHasAmmo |= 1;
	}
	if (!bHasAmmo)
	{
		return FALSE;
	}

	return TRUE;
}


//MODDD - should a weapon NOT have any deploy methods, this default will also undo the player silencer render effect.  Just for safety.
// ...or not?
BOOL CBasePlayerWeapon::Deploy(){
	


	//m_pPlayer->pev->renderfx &= ~128;
	return CBasePlayerItem::Deploy();
}


BOOL CBasePlayerWeapon::DefaultDeploy( char *szViewModel, char *szWeaponModel, int iAnim, char *szAnimExt, int skiplocal /* = 0 */, int body, float deployAnimTime, float fireDelayTime )
{
	if (!CanDeploy()) {
		return FALSE;
	}

	// safety
	m_fInReload = FALSE;

	m_chargeReady &= ~128;

	m_pPlayer->TabulateAmmo();
	m_pPlayer->pev->viewmodel = MAKE_STRING(szViewModel);
	m_pPlayer->pev->weaponmodel = MAKE_STRING(szWeaponModel);
	strcpy( m_pPlayer->m_szAnimExtention, szAnimExt );


	//MODDD - force!
	// ...no, rather not fight the engine.  It works in mysterious ways.
	//SendWeaponAnim( iAnim, skiplocal, body );
	// WRONG!  Bypass already ignores skiplocal.  Send the body here!
	//SendWeaponAnimBypass( iAnim, skiplocal );
	SendWeaponAnimBypass(iAnim, body);

	if(fireDelayTime == -1){
		// use deployAnimTime then
		fireDelayTime = deployAnimTime;
	}


	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + fireDelayTime; //0.5;
	//MODDD - why not primary/secondary ones too??  Always weirdness if m_flNextAttack is set without those
	SetAttackDelays(m_pPlayer->m_flNextAttack);

	//m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + deployAnimTime; //used to be "... + 1.0", now depends on optional parameter (defaults to "1.0");
	
	// MODDD - "+ randomIdleAnimationDelay()" removed.  Leave that up to the weapons themselves instead.
	// Some like the satchel need to return to WeaponIdle to see if a satchel needs to be re-deployed
	// (creates an akward time of holding the remote where left and right-click do nothing after switching out from recently detonating with ammo left).
	// REVERTED.  Satchel's handled now.
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + deployAnimTime + randomIdleAnimationDelay(); //used to be "... + 1.0", now depends on optional parameter (defaults to "1.0");

	
	forceBlockLooping();


	//MODDD - remove the silencer block (no muzzle flash)
	//m_pPlayer->pev->renderfx &= ~128;
	
	// NEVERMIND, canned.
	/*
	easyPrintLine("WHATTT %d", (int)CVAR_GET_FLOAT("deployingWeaponPlaysOtherSound") );
	if(CVAR_GET_FLOAT("deployingWeaponPlaysSound") == 1){
		EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/gunpickup4.wav", 1, ATTN_NORM);
	}
	*/
	return TRUE;
}//END OF DefaultDeploy


void CBasePlayerWeapon::DefaultHolster( int iAnim, int skiplocal /* = 0 */, int body, float holsterAnimTime )
{

	// Like base Holster which may not get called, set reload to False.
	m_fInReload = FALSE;


	//HACK - make this a little longer to stop the client from thinking it is done the moment this ends.
	//       The notice to deploy the next weapon may come a little late.
	// try with and without + 1 maybe?
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + holsterAnimTime + 1;

	// Let this handle the time it takes to change anims instead, not transmitted to the client by default.
	m_pPlayer->m_fCustomHolsterWaitTime = gpGlobals->time + holsterAnimTime;

	// Don't want to risk setting idle animations while holstering, set this to an impossible value.
	m_pPlayer->m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + holsterAnimTime + 5;


	m_chargeReady |= 128;

	//MODDD - doing Bypass instead of normal.
	SendWeaponAnimBypass( iAnim );


}//END OF DefaultHolster






BOOL CBasePlayerWeapon::DefaultReload( int iClipSize, int iAnim, float fDelay, int body )
{
	int myPrimaryAmmoType = getPrimaryAmmoType();
	if (IS_AMMOTYPE_VALID(myPrimaryAmmoType)) {
		// ok.
	}
	else {
		// oh.
		return FALSE;
	}


	//MODDD - cheat intervention.  Can only fail to reload if the infiniteAmmo cheat is off.
	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteammo) != 1){
		if (m_pPlayer->m_rgAmmo[myPrimaryAmmoType] <= 0)
			return FALSE;

		int j = min(iClipSize - m_iClip, m_pPlayer->m_rgAmmo[myPrimaryAmmoType]);

		//MODDD - is this edit ok?  To help with things that mess with the max-count like the glock with old reload logic.
		//if (j == 0)
		if(j <= 0)
			return FALSE;
	}else{
		if (m_pPlayer->m_rgAmmo[myPrimaryAmmoType] <= 0)
			m_pPlayer->m_rgAmmo[myPrimaryAmmoType] = 1;

		int j = min(iClipSize - m_iClip, m_pPlayer->m_rgAmmo[myPrimaryAmmoType]);
		if (j <= 0)
			m_pPlayer->m_rgAmmo[myPrimaryAmmoType] = 1;

		//return TRUE;
	}

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + fDelay;

	//!!UNDONE -- reload sound goes here !!!
	//SendWeaponAnim( iAnim, UseDecrement() ? 1 : 0 );

	//MODDD - reload animation must happen at all costs!  Force that sucker!
	//...this may no logner be necessary, can check.
	//For that matter, even above, why didn't we send the "body"? It was effectively ignored then, at least from this DefaultReload call.
	this->SendWeaponAnimBypass(iAnim, body);
	m_fInReload = TRUE;


	//MODDD - eh, why not.
	forceBlockLooping();

	//MODDD - this will now always be at the end of this reload anim instead.
	//m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + fDelay + this->randomIdleAnimationDelay();
	
	return TRUE;
}

BOOL CBasePlayerWeapon::PlayEmptySound( void )
{
	// wait.  return 0 regardless of... anything?  Hell's the point
	if (m_iPlayEmptySound)
	{
		UTIL_PlaySound(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/357_cock1.wav", 0.8, ATTN_NORM, 0, 100, FALSE);
		m_iPlayEmptySound = 0;
		return 0;
	}
	return 0;
}

void CBasePlayerWeapon::ResetEmptySound( void )
{
	m_iPlayEmptySound = 1;
}

//=========================================================
//=========================================================
int CBasePlayerWeapon::PrimaryAmmoIndex( void )
{
	//return m_iPrimaryAmmoType;
	return getPrimaryAmmoType();
}

//=========================================================
//=========================================================
int CBasePlayerWeapon::SecondaryAmmoIndex( void )
{
	//MODDD - why not always return m_iSecondaryAmmoType?  It will just remain -1 for things that never set it
	//return -1;
	//return m_iSecondaryAmmoType;
	return getSecondaryAmmoType();
}


int CBasePlayerWeapon::PlayerPrimaryAmmoCount() {
	int myPrimaryAmmoType = getPrimaryAmmoType();
	if (IS_AMMOTYPE_VALID(myPrimaryAmmoType)) {
		return m_pPlayer->m_rgAmmo[myPrimaryAmmoType];
	}else{
		return -1;  //what
	}
}
int CBasePlayerWeapon::PlayerSecondaryAmmoCount() {
	int mySecondaryAmmoType = getSecondaryAmmoType();
	if (IS_AMMOTYPE_VALID(mySecondaryAmmoType)) {
		return m_pPlayer->m_rgAmmo[mySecondaryAmmoType];
	}
	else {
		return -1;  //what
	}
}


// WARNING - these don't check to see whether removing ammo goes below 0 or
// adding ammo goes over the max allowed in reserve!
void CBasePlayerWeapon::ChangePlayerPrimaryAmmoCount(int changeBy) {
	int myPrimaryAmmoType = getPrimaryAmmoType();
	if (IS_AMMOTYPE_VALID(myPrimaryAmmoType)) {
		m_pPlayer->m_rgAmmo[myPrimaryAmmoType] += changeBy;
	}
	else {
		//what
	}
}
void CBasePlayerWeapon::ChangePlayerSecondaryAmmoCount(int changeBy) {
	int mySecondaryAmmoType = getSecondaryAmmoType();
	if (IS_AMMOTYPE_VALID(mySecondaryAmmoType)) {
		m_pPlayer->m_rgAmmo[mySecondaryAmmoType] += changeBy;
	}
	else {
		//what
	}
}
void CBasePlayerWeapon::SetPlayerPrimaryAmmoCount(int newVal) {
	int myPrimaryAmmoType = getPrimaryAmmoType();
	if (IS_AMMOTYPE_VALID(myPrimaryAmmoType)) {
		m_pPlayer->m_rgAmmo[myPrimaryAmmoType] = newVal;
	}
	else {
		//what
	}
}
void CBasePlayerWeapon::SetPlayerSecondaryAmmoCount(int newVal) {
	int mySecondaryAmmoType = getSecondaryAmmoType();
	if (IS_AMMOTYPE_VALID(mySecondaryAmmoType)) {
		m_pPlayer->m_rgAmmo[mySecondaryAmmoType] = newVal;
	}
	else {
		//what
	}
}


void CBasePlayerWeapon::Holster( int skiplocal /* = 0 */ )
{ 

	m_fInReload = FALSE; // cancel any reload in progress.
	m_pPlayer->pev->viewmodel = 0; 
	m_pPlayer->pev->weaponmodel = 0;

	//MODDD - also for safety, set these to instantly switch to the next weapon for default behavior
	//        (weapons that do not override holster... but most, if not all, should have holster animations to use here).
	//        Weapons should never call base Holster if implementing Holster. They should typically call "DefaultHolster" 
	//        to tell how to holster themselves (anim index and anim duration).
	m_pPlayer->m_flNextAttack = 0;

	m_pPlayer->m_fCustomHolsterWaitTime = gpGlobals->time;

	//Don't want to risk setting idle animations while holstering, set this to an impossible value. ?
	m_pPlayer->m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5;

}

void CBasePlayerAmmo::Spawn( void )
{
	pev->movetype = MOVETYPE_TOSS;
	pev->solid = SOLID_TRIGGER;
	UTIL_SetSize(pev, Vector(-16, -16, 0), Vector(16, 16, 16));
	UTIL_SetOrigin( pev, pev->origin );

	SetTouch( &CBasePlayerAmmo::DefaultTouch );
}

CBaseEntity* CBasePlayerAmmo::Respawn( void )
{
	pev->effects |= EF_NODRAW;
	SetTouch( NULL );

	UTIL_SetOrigin( pev, g_pGameRules->VecAmmoRespawnSpot( this ) );// move to wherever I'm supposed to repawn.

	SetThink( &CBasePlayerAmmo::Materialize );
	pev->nextthink = g_pGameRules->FlAmmoRespawnTime( this );

	return this;
}

void CBasePlayerAmmo::Materialize( void )
{
	if ( pev->effects & EF_NODRAW )
	{
		// changing from invisible state to visible.
		UTIL_PlaySound( ENT(pev), CHAN_WEAPON, "items/suitchargeok1.wav", 1, ATTN_NORM, 0, 150, FALSE );
		pev->effects &= ~EF_NODRAW;
		pev->effects |= EF_MUZZLEFLASH;
	}

	SetTouch( &CBasePlayerAmmo::DefaultTouch );
}

void CBasePlayerAmmo::DefaultTouch( CBaseEntity *pOther )
{
	if ( !pOther->IsPlayer() )
	{
		return;
	}

	if (AddAmmo( pOther ))
	{
		if ( g_pGameRules->AmmoShouldRespawn( this ) == GR_AMMO_RESPAWN_YES )
		{
			Respawn();
		}
		else
		{
			SetTouch( NULL );
			SetThink(&CBaseEntity::SUB_Remove);
			pev->nextthink = gpGlobals->time + .1;
		}
	}
	else if (gEvilImpulse101)
	{
		// evil impulse 101 hack, kill always
		SetTouch( NULL );
		SetThink(&CBaseEntity::SUB_Remove);
		pev->nextthink = gpGlobals->time + .1;
	}
}

//=========================================================
// called by the new item with the existing item as parameter
//
// if we call ExtractAmmo(), it's because the player is picking up this type of weapon for 
// the first time. If it is spawned by the world, m_iDefaultAmmo will have a default ammo amount in it.
// if  this is a weapon dropped by a dying player, has 0 m_iDefaultAmmo, which means only the ammo in 
// the weapon clip comes along. 
//=========================================================
//MODDD - NOTE - this method occurs for picked-up weapons (instead of ammo) ONLY!
// But is called regardless of being for a new pickup or ones the player already has just for ammo.
// ALSO, little edit.  AddPrimary & AddSecondary ammo actually return BOOL's, int's in the end but meant to convey
// just 0 or 1,  "no" or "yes".  Seems dishonest to call it 'int' here for no reason.
// Also changed how 'iReturn' works a little.  Either AddPrimaryAmmo or AddSecondaryAmmo call returning TRUE should
// let iReturn be TRUE.  Otherwise, calling AddPrimaryAmmo that returns TRUE and then AddSecondaryAmmo that returns FALSE
// forgets that PrimaryAmmo said TRUE.  Which I assume isn't what was wanted here.
// So iReturn should start as false and do '|=' on the Add calls.
BOOL CBasePlayerWeapon::ExtractAmmo( CBasePlayerWeapon *pWeapon )
{
	BOOL iReturn = FALSE;

	if ( pszAmmo1() != NULL )
	{
		// blindly call with m_iDefaultAmmo. It's either going to be a value or zero. If it is zero,
		// we only get the ammo in the weapon's clip, which is what we want. 
		iReturn |= pWeapon->AddPrimaryAmmo( m_iDefaultAmmo, (char *)pszAmmo1(), iMaxClip(), iMaxAmmo1() );
		m_iDefaultAmmo = 0;
	}

	if ( pszAmmo2() != NULL )
	{
		iReturn |= pWeapon->AddSecondaryAmmo( 0, (char *)pszAmmo2(), iMaxAmmo2() );
	}
	
	//easyPrintLine("DOOP2 %d", iReturn);
	return iReturn;
}

//=========================================================
// called by the new item's class with the existing item as parameter
//=========================================================
//MODDD - same BOOL edit as above.
BOOL CBasePlayerWeapon::ExtractClipAmmo( CBasePlayerWeapon *pWeapon )
{
	int iAmmo;

	if ( m_iClip == WEAPON_NOCLIP )
	{
		iAmmo = 0;// guns with no clips always come empty if they are second-hand
	}
	else
	{
		iAmmo = m_iClip;
	}
	
	//MODDD - GiveAmmo can return a number outside of 0 or 1 (FALSE/TRUE) though? odd.  Edit.
	// And beware, -1 is also technically not equal to 0 like a simple 'not 0' check would yield.
	//return pWeapon->m_pPlayer->GiveAmmo( iAmmo, (char *)pszAmmo1(), iMaxAmmo1() ); // , &m_iPrimaryAmmoType
	
	//return (pWeapon->m_pPlayer->GiveAmmo(iAmmo, (const char*)pszAmmo1(), iMaxAmmo1() ) > 0);
	// is this replacement ok???
	// This is hard to debug, doing this for now.
	int myPrimaryAmmotype = PrimaryAmmoIndex();
	if (IS_AMMOTYPE_VALID(myPrimaryAmmotype)) {
		// trust it
		easyPrintLine("!!! Report this!  SUCCESS minor");
		return (pWeapon->m_pPlayer->GiveAmmoID(iAmmo, myPrimaryAmmotype, iMaxAmmo1()) > 0);
	}
	else {
		// oh
		easyForcePrintLine("!!! Report this!  ExtractClipAmmo FAILURE minor, weaponname:%s ammoindex:%d ammoname:%s pickupamount:%d", pszName(), myPrimaryAmmotype, pszAmmo1(), iAmmo);
		return (pWeapon->m_pPlayer->GiveAmmo(iAmmo, (const char*)pszAmmo1(), iMaxAmmo1()) > 0);
	}

}
	
//=========================================================
// RetireWeapon - no more ammo for this gun, put it away.
//=========================================================
void CBasePlayerWeapon::RetireWeapon( void )
{
	// first, no viewmodel at all.
	m_pPlayer->pev->viewmodel = iStringNull;
	m_pPlayer->pev->weaponmodel = iStringNull;
	//m_pPlayer->pev->viewmodelindex = NULL;

	g_pGameRules->GetNextBestWeapon( m_pPlayer, this );
}

//*********************************************************
// weaponbox code:
//*********************************************************

LINK_ENTITY_TO_CLASS( weaponbox, CWeaponBox );

TYPEDESCRIPTION	CWeaponBox::m_SaveData[] = 
{
	DEFINE_ARRAY( CWeaponBox, m_rgAmmo, FIELD_INTEGER, MAX_AMMO_TYPES ),
	DEFINE_ARRAY( CWeaponBox, m_rgiszAmmo, FIELD_STRING, MAX_AMMO_TYPES ),
	DEFINE_ARRAY( CWeaponBox, m_rgpPlayerItems, FIELD_CLASSPTR, MAX_ITEM_TYPES ),
	DEFINE_FIELD( CWeaponBox, m_cAmmoTypes, FIELD_INTEGER ),
};

IMPLEMENT_SAVERESTORE( CWeaponBox, CBaseEntity );

//=========================================================
//
//=========================================================
void CWeaponBox::Precache( void )
{
	PRECACHE_MODEL("models/w_weaponbox.mdl");
}

//=========================================================
//=========================================================
void CWeaponBox::KeyValue( KeyValueData *pkvd )
{
	if ( m_cAmmoTypes < MAX_AMMO_TYPES )
	{
		PackAmmo( ALLOC_STRING(pkvd->szKeyName), atoi(pkvd->szValue) );
		m_cAmmoTypes++;// count this new ammo type.

		pkvd->fHandled = TRUE;
	}
	else
	{
		ALERT ( at_console, "WeaponBox too full! only %d ammotypes allowed\n", MAX_AMMO_TYPES );
	}
}

//=========================================================
// CWeaponBox - Spawn 
//=========================================================
void CWeaponBox::Spawn( void )
{
	Precache( );

	pev->movetype = MOVETYPE_TOSS;
	pev->solid = SOLID_TRIGGER;

	UTIL_SetSize( pev, g_vecZero, g_vecZero );

	SET_MODEL( ENT(pev), "models/w_weaponbox.mdl");
}

//=========================================================
// CWeaponBox - Kill - the think function that removes the
// box from the world.
//=========================================================
void CWeaponBox::Kill( void )
{
	CBasePlayerItem *pWeapon;
	int i;

	// destroy the weapons
	for ( i = 0 ; i < MAX_ITEM_TYPES ; i++ )
	{
		pWeapon = m_rgpPlayerItems[ i ];

		while ( pWeapon )
		{
			pWeapon->SetThink(&CBaseEntity::SUB_Remove);
			pWeapon->pev->nextthink = gpGlobals->time + 0.1;
			pWeapon = pWeapon->m_pNext;
		}
	}

	// remove the box
	UTIL_Remove( this );
}

//=========================================================
// CWeaponBox - Touch: try to add my contents to the toucher
// if the toucher is a player.
//=========================================================
void CWeaponBox::Touch( CBaseEntity *pOther )
{
	if ( !(pev->flags & FL_ONGROUND ) )
	{
		return;
	}

	if ( !pOther->IsPlayer() )
	{
		// only players may touch a weaponbox.
		return;
	}

	if ( !pOther->IsAlive() )
	{
		// no dead guys.
		return;
	}

	CBasePlayer *pPlayer = (CBasePlayer *)pOther;
	int i;

// dole out ammo
	for ( i = 0 ; i < MAX_AMMO_TYPES ; i++ )
	{
		if ( !FStringNull( m_rgiszAmmo[ i ] ) )
		{
			const char* thisAmmoString = (char*)STRING(m_rgiszAmmo[i]);
			// there's some ammo of this type. 
			pPlayer->GiveAmmo( m_rgAmmo[ i ], thisAmmoString, MaxAmmoCarry(thisAmmoString) );

			//ALERT ( at_console, "Gave %d rounds of %s\n", m_rgAmmo[i], STRING(m_rgiszAmmo[i]) );

			// now empty the ammo from the weaponbox since we just gave it to the player
			m_rgiszAmmo[ i ] = iStringNull;
			m_rgAmmo[ i ] = 0;
		}
	}

// go through my weapons and try to give the usable ones to the player. 
// it's important the the player be given ammo first, so the weapons code doesn't refuse 
// to deploy a better weapon that the player may pick up because he has no ammo for it.
	for ( i = 0 ; i < MAX_ITEM_TYPES ; i++ )
	{
		if ( m_rgpPlayerItems[ i ] )
		{
			CBasePlayerItem *pItem;

			// have at least one weapon in this slot
			while ( m_rgpPlayerItems[ i ] )
			{
				//ALERT ( at_console, "trying to give %s\n", STRING( m_rgpPlayerItems[ i ]->pev->classname ) );

				pItem = m_rgpPlayerItems[ i ];
				m_rgpPlayerItems[ i ] = m_rgpPlayerItems[ i ]->m_pNext;// unlink this weapon from the box

				if ( pPlayer->AddPlayerItem( pItem ) )
				{
					pItem->AttachToPlayer( pPlayer );
				}
			}
		}
	}


	//EMIT_SOUND( pOther->edict(), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM );
	playGunPickupSound(pPlayer->pev);


	SetTouch(NULL);
	UTIL_Remove(this);
}

//=========================================================
// CWeaponBox - PackWeapon: Add this weapon to the box
//=========================================================
BOOL CWeaponBox::PackWeapon( CBasePlayerItem *pWeapon )
{
	// is one of these weapons already packed in this box?
	if ( HasWeapon( pWeapon ) )
	{
		return FALSE;// box can only hold one of each weapon type
	}

	if ( pWeapon->m_pPlayer )
	{
		if ( !pWeapon->m_pPlayer->RemovePlayerItem( pWeapon ) )
		{
			// failed to unhook the weapon from the player!
			return FALSE;
		}
	}

	int iWeaponSlot = pWeapon->iItemSlot();
	
	if ( m_rgpPlayerItems[ iWeaponSlot ] )
	{
		// there's already one weapon in this slot, so link this into the slot's column
		pWeapon->m_pNext = m_rgpPlayerItems[ iWeaponSlot ];	
		m_rgpPlayerItems[ iWeaponSlot ] = pWeapon;
	}
	else
	{
		// first weapon we have for this slot
		m_rgpPlayerItems[ iWeaponSlot ] = pWeapon;
		pWeapon->m_pNext = NULL;	
	}

	pWeapon->pev->spawnflags |= SF_NORESPAWN;// never respawn
	pWeapon->pev->movetype = MOVETYPE_NONE;
	pWeapon->pev->solid = SOLID_NOT;
	pWeapon->pev->effects = EF_NODRAW;
	pWeapon->pev->modelindex = 0;
	pWeapon->pev->model = iStringNull;
	pWeapon->pev->owner = edict();
	pWeapon->SetThink( NULL );// crowbar may be trying to swing again, etc.
	pWeapon->SetTouch( NULL );
	pWeapon->m_pPlayer = NULL;

	//ALERT ( at_console, "packed %s\n", STRING(pWeapon->pev->classname) );

	return TRUE;
}

//=========================================================
// CWeaponBox - PackAmmo
//=========================================================
BOOL CWeaponBox::PackAmmo( int iszName, int iCount )
{
	int iMaxCarry;

	if ( FStringNull( iszName ) )
	{
		// error here
		ALERT ( at_console, "NULL String in PackAmmo!\n" );
		return FALSE;
	}

	const char* thisAmmoString = (char*)STRING(iszName);
	
	iMaxCarry = MaxAmmoCarry(thisAmmoString);

	if ( iMaxCarry != -1 && iCount > 0 )
	{
		//ALERT ( at_console, "Packed %d rounds of %s\n", iCount, STRING(iszName) );
		GiveAmmo( iCount, thisAmmoString, iMaxCarry );
		return TRUE;
	}

	return FALSE;
}

//=========================================================
// CWeaponBox - GiveAmmo
//=========================================================
int CWeaponBox::GiveAmmo( int iCount, const char* szName, int iMax, int *pIndex/* = NULL*/ )
{
	int i;

	for (i = 1; i < MAX_AMMO_TYPES && !FStringNull( m_rgiszAmmo[i] ); i++)
	{
		if (stricmp( szName, STRING( m_rgiszAmmo[i])) == 0)
		{
			if (pIndex)
				*pIndex = i;

			int iAdd = min( iCount, iMax - m_rgAmmo[i]);
			if (iCount == 0 || iAdd > 0)
			{
				m_rgAmmo[i] += iAdd;

				return i;
			}
			return -1;
		}
	}
	if (i < MAX_AMMO_TYPES)
	{
		if (pIndex)
			*pIndex = i;

		m_rgiszAmmo[i] = MAKE_STRING( szName );
		m_rgAmmo[i] = iCount;

		return i;
	}
	ALERT( at_console, "out of named ammo slots\n");
	return i;
}

//=========================================================
// CWeaponBox::HasWeapon - is a weapon of this type already
// packed in this box?
//=========================================================
BOOL CWeaponBox::HasWeapon( CBasePlayerItem *pCheckItem )
{
	CBasePlayerItem *pItem = m_rgpPlayerItems[pCheckItem->iItemSlot()];

	while (pItem)
	{
		if (FClassnameIs( pItem->pev, STRING( pCheckItem->pev->classname) ))
		{
			return TRUE;
		}
		pItem = pItem->m_pNext;
	}

	return FALSE;
}

//=========================================================
// CWeaponBox::IsEmpty - is there anything in this box?
//=========================================================
BOOL CWeaponBox::IsEmpty( void )
{
	int i;

	for ( i = 0 ; i < MAX_ITEM_TYPES ; i++ )
	{
		if ( m_rgpPlayerItems[ i ] )
		{
			return FALSE;
		}
	}

	for ( i = 0 ; i < MAX_AMMO_TYPES ; i++ )
	{
		if ( !FStringNull( m_rgiszAmmo[ i ] ) )
		{
			// still have a bit of this type of ammo
			return FALSE;
		}
	}

	return TRUE;
}

//=========================================================
//=========================================================
void CWeaponBox::SetObjectCollisionBox( void )
{
	pev->absmin = pev->origin + Vector(-16, -16, 0);
	pev->absmax = pev->origin + Vector(16, 16, 16); 
}


void CBasePlayerWeapon::PrintState( void )
{
	ALERT( at_console, "primary:  %f\n", m_flNextPrimaryAttack );
	ALERT( at_console, "idle   :  %f\n", m_flTimeWeaponIdle );

//	ALERT( at_console, "nextrl :  %f\n", m_flNextReload );
//	ALERT( at_console, "nextpum:  %f\n", m_flPumpTime );

//	ALERT( at_console, "m_frt  :  %f\n", m_fReloadTime );
	ALERT( at_console, "m_finre:  %i\n", m_fInReload );
//	ALERT( at_console, "m_finsr:  %i\n", m_fInSpecialReload );

	ALERT( at_console, "m_iclip:  %i\n", m_iClip );
}



// by default, doesn't use this.  Override and specify for things that do.
const char* CBasePlayerWeapon::GetPickupWalkerName(void){
	return "\0";
}

// Returns whether the check passed (replacing this entity) or failed (not replacing it).
CBaseEntity* CBasePlayerWeapon::pickupWalkerReplaceCheck(void){
	
	const char* pickupNameTest = GetPickupWalkerName();

	// No need to make it lowercase, classnames already can be trusted to be.
	//char tempClassname[127];
	//strcpy(&tempClassname[0], this->getClassname());
	//lowercase(tempClassname);

	if( !isStringEmpty(pickupNameTest) &&
		!(pev->spawnflags & SF_PICKUP_NOREPLACE) &&
		!stringEndsWith(this->getClassname(), "_noreplace")){
		// lacking the NOREPLACE flag? replace with my spawnnable.

		//char pickupWalkerName[128];
		//sprintf(pickupWalkerName, "monster_%spickupwalker", baseclassname);

		//CBaseEntity::Create(pickupWalkerName, pev->origin, pev->angles);
		CBaseEntity* generated = CBaseEntity::Create(pickupNameTest, pev->origin, pev->angles);

		// By default, all entities made with "Create" get the SF_NORESPAWN flag.
		// This pickupwalker replaces this entity, so it should get that same flag only if the weapon it replaces had it too.
		if (pev->spawnflags & SF_NORESPAWN) {
			generated->pev->spawnflags |= SF_NORESPAWN;
		}
		else {
			generated->pev->spawnflags &= ~SF_NORESPAWN;
		}

		// And tell the generated pickupwalker that its current coords are the ones to use for respawning, regardless
		// of where it wanders off too.
		CPickupWalker* tempWalk = static_cast<CPickupWalker*>(generated);
		tempWalk->respawn_origin = pev->origin;
		tempWalk->respawn_angles = pev->angles;


		UTIL_Remove( this );

		//easyForcePrintLine("pickupWalkerReplaceCheck TRUE");
		return generated;
	}
	//easyForcePrintLine("pickupWalkerReplaceCheck FALSE");
	return NULL;
}

// blank by default, re-define for weapons that need to do something the moment
// a reload animation finishes and ammo is taken from the ammo pool to the clip.
void CBasePlayerWeapon::OnReloadApply(void) {

}
void CBasePlayerWeapon::OnAddPrimaryAmmoAsNewWeapon(void) {

}


// REMEMBER, weapons.cpp is only included serverside!
float CBasePlayerWeapon::getPlayerBaseFOV(void){
	//FOV info is stored in the player.
	return m_pPlayer->getBaseFOV();
}

int CBasePlayerWeapon::getFramesSinceRestore(void){
	// instance var here
	return m_pPlayer->m_framesSinceRestore;
}




//MODDD - NOTE.  SaveData for weapons moved to their own files, surrounded by 'ifndef CLIENT_DLL',
// since this file (weapons.cpp) has always been serverside only, and specific weapons files like
// glock.cpp are shared (need to specify being for only clientside).
