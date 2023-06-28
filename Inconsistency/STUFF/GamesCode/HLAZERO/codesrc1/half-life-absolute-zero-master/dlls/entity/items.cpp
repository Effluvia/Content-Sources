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

===== items.cpp ========================================================

  functions governing the selection/use of weapons for players
*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"
#include "player.h"
#include "skill.h"
#include "items.h"
#include "gamerules.h"

extern int gmsgItemPickup;

//MODDD - OnAntidotePickup
extern int gmsgAntidoteP;
extern int gmsgAdrenalineP;
extern int gmsgRadiationP;
extern int gmsgUpdateAirTankAirTime;



//MODDD
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST(wpn_glocksilencer)
//EASY_CVAR_EXTERN_DEBUGONLY(canTakeLongJump)



class CWorldItem : public CBaseEntity
{
public:
	void KeyValue(KeyValueData *pkvd ); 
	void Spawn( void );
	int	m_iType;
};

LINK_ENTITY_TO_CLASS(world_items, CWorldItem);

void CWorldItem::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "type"))
	{
		m_iType = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue( pkvd );
}

void CWorldItem::Spawn( void )
{
	CBaseEntity *pEntity = NULL;

	switch (m_iType) 
	{
	case 44: // ITEM_BATTERY:
		pEntity = CBaseEntity::Create( "item_battery", pev->origin, pev->angles );
		break;
	case 42: // ITEM_ANTIDOTE:
		pEntity = CBaseEntity::Create( "item_antidote", pev->origin, pev->angles );
		break;
	case 43: // ITEM_SECURITY:
		pEntity = CBaseEntity::Create( "item_security", pev->origin, pev->angles );
		break;
	case 45: // ITEM_SUIT:
		pEntity = CBaseEntity::Create( "item_suit", pev->origin, pev->angles );
		break;
	}

	if (!pEntity)
	{
		ALERT( at_console, "unable to create world_item %d\n", m_iType );
	}
	else
	{
		pEntity->pev->target = pev->target;
		pEntity->pev->targetname = pev->targetname;
		pEntity->pev->spawnflags = pev->spawnflags;
	}

	REMOVE_ENTITY(edict());
}


void CItem::Spawn( void )
{
	pev->movetype = MOVETYPE_TOSS;
	pev->solid = SOLID_TRIGGER;
	UTIL_SetOrigin( pev, pev->origin );
	UTIL_SetSize(pev, Vector(-16, -16, 0), Vector(16, 16, 16));
	SetTouch(&CItem::ItemTouch);


	if (DROP_TO_FLOOR(ENT(pev)) == 0)
	{
		//MODDD - NOTICE.  Assuming spawning at a position not too close to the floor = falling through the world is a bad assumption.
		// Yes.  DROP_TO_FLOOR just returns 0 if an object is too far above the ground, not necessarily because no floor exists.
		// And come to think of it, why not a "IsInWorld" check?  Wouldn't that make sense?

		// Way it should work is, if indeed not in the world, yea, say that.
		if (!IsInWorld()) {
			ALERT(at_error, "Item %s fell out of level at (%f,%f,%f)", STRING(pev->classname), pev->origin.x, pev->origin.y, pev->origin.z);
			UTIL_Remove(this);
		}
		else {
			// in the world?
			if (spawnedDynamically) {
				// acceptable to keep this, have a nice day.  give commands don't care whether something can hit the ground.
			}
			else {
				// not spawned dynamically?  This is the doing of the map.
				// Better to let the level designer know something was up then.
				ALERT(at_error, "Item %s at (%f,%f,%f) too far to detect the floor", STRING(pev->classname), pev->origin.x, pev->origin.y, pev->origin.z);
				UTIL_Remove(this);
			}
		}

		return;
	}
}

extern int gEvilImpulse101;

void CItem::ItemTouch( CBaseEntity *pOther )
{
	// if it's not a player, ignore
	if ( !pOther->IsPlayer() )
	{
		return;
	}

	CBasePlayer *pPlayer = (CBasePlayer *)pOther;

	// ok, a player is touching this item, but can he have it?
	if ( !g_pGameRules->CanHaveItem( pPlayer, this ) )
	{
		// no? Ignore the touch.
		return;
	}

	if (MyTouch( pPlayer ))
	{
		SUB_UseTargets( pOther, USE_TOGGLE, 0 );
		SetTouch( NULL );
		
		// player grabbed the item. 
		g_pGameRules->PlayerGotItem( pPlayer, this );
		if ( g_pGameRules->ItemShouldRespawn( this ) == GR_ITEM_RESPAWN_YES )
		{
			Respawn(); 
		}
		else
		{
			UTIL_Remove( this );
		}
	}
	else if (gEvilImpulse101)
	{
		UTIL_Remove( this );
	}
}

CBaseEntity* CItem::Respawn( void )
{
	SetTouch( NULL );
	pev->effects |= EF_NODRAW;

	UTIL_SetOrigin( pev, g_pGameRules->VecItemRespawnSpot( this ) );// blip to whereever you should respawn.

	SetThink ( &CItem::Materialize );
	pev->nextthink = g_pGameRules->FlItemRespawnTime( this ); 
	return this;
}

void CItem::Materialize( void )
{
	if ( pev->effects & EF_NODRAW )
	{
		// changing from invisible state to visible.
		UTIL_PlaySound( ENT(pev), CHAN_WEAPON, "items/suitchargeok1.wav", 1, ATTN_NORM, 0, 150, FALSE );
		pev->effects &= ~EF_NODRAW;
		pev->effects |= EF_MUZZLEFLASH;
	}

	SetTouch( &CItem::ItemTouch );
}

#define SF_SUIT_SHORTLOGON		0x0001

class CItemSuit : public CItem
{
	void Spawn( void )
	{ 
		Precache( );
		setModel("models/w_suit.mdl");
		CItem::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/w_suit.mdl");
	}
	BOOL MyTouch( CBasePlayer *pPlayer )
	{
		if ( pPlayer->pev->weapons & (1<<WEAPON_SUIT) )
			return FALSE;

		//MODDD - moved above the FVOX calls so that 'lacking the suit' for those few moments doesn't
		// block getting the FVOX messgaes.
		pPlayer->pev->weapons |= (1 << WEAPON_SUIT);

		//MODDDD - use the new "player->SetSuitUpdate" system!
		if (pev->spawnflags & SF_SUIT_SHORTLOGON) {
			//EMIT_SOUND_SUIT(pPlayer->edict(), "!HEV_A0");		// short version of suit logon,
			pPlayer->SetSuitUpdate("!HEV_A0", FALSE, SUIT_NEXT_IN_30MIN);
		}else {
			//EMIT_SOUND_SUIT(pPlayer->edict(), "!HEV_AAx");	// long version of suit logon
			pPlayer->SetSuitUpdate("!HEV_AAx", FALSE, SUIT_NEXT_IN_30MIN);
		}

		return TRUE;
	}
};

LINK_ENTITY_TO_CLASS(item_suit, CItemSuit);



class CItemBattery : public CItem
{
	void Spawn( void )
	{ 
		Precache( );
		setModel("models/w_battery.mdl");
		CItem::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/w_battery.mdl");
		PRECACHE_SOUND( "items/gunpickup2.wav" );
	}
	BOOL MyTouch( CBasePlayer *pPlayer )
	{
		if ( pPlayer->pev->deadflag != DEAD_NO )
		{
			return FALSE;
		}

		if ((pPlayer->pev->armorvalue < MAX_NORMAL_BATTERY) &&
			(pPlayer->pev->weapons & (1<<WEAPON_SUIT)))
		{
			int pct;
			//MODDD - removed.
			//char szcharge[64];

			pPlayer->pev->armorvalue += gSkillData.batteryCapacity;
			pPlayer->pev->armorvalue = min(pPlayer->pev->armorvalue, MAX_NORMAL_BATTERY);

			UTIL_PlaySound( pPlayer->edict(), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM, FALSE );

			MESSAGE_BEGIN( MSG_ONE, gmsgItemPickup, NULL, pPlayer->pev );
				WRITE_STRING( STRING(pev->classname) );
			MESSAGE_END();


			//MODDD - removed.
			/*
			pct = (int)( (float)(pPlayer->pev->armorvalue * 100.0) * (1.0/MAX_NORMAL_BATTERY) + 0.5);
			pct = (pct / 5);
			if (pct > 0)
				pct--;
		
			sprintf( szcharge,"!HEV_%1dP", pct );
			*/
			

			//eh, probably not that important.   REMOVED.
			/*
			if(pPlayer->suitCanPlay(szcharge, FALSE) || pPlayer->suitCanPlay("!HEV_BATTERY", FALSE) ){
				pPlayer->forceRepeatBlock("!HEV_BTY_DING", FALSE, SUIT_REPEAT_OK);
				pPlayer->forceRepeatBlock(szcharge, FALSE, SUIT_REPEAT_OK);
			}
			*/
			
			pPlayer->SetSuitUpdate("!HEV_BTY_DING", FALSE, SUIT_NEXT_IN_30SEC, 0.6);
			
			//MODDD - play the generic "battery get" sound the first time and rarely after.
			pPlayer->SetSuitUpdate("!HEV_BATTERY", FALSE, SUIT_NEXT_IN_30MIN);
			
			// Suit reports new power level
			// For some reason this wasn't working in release build -- round it.
			

			//MODDD - no, going to do this by logic instead of canned 15-percent increments.
			////EMIT_SOUND_SUIT(ENT(pev), szcharge);
			//pPlayer->SetSuitUpdate(szcharge, FALSE, SUIT_NEXT_IN_30SEC);

			//put a delay?   rely on "pPlayer->suitCanPlay("HEV_...", FALSE)" if so.
			//////pPlayer->SetSuitUpdate("!HEV_BNOTICE", FALSE, SUIT_REPEAT_OK, 0.7f);
			

			// NOTICE - a sentence of 5000 is a special code to do the flexible power readout sentence.
			pPlayer->SetSuitUpdateNumber(5000, SUIT_REPEAT_OK, -1, TRUE);

			/*
			if(pPlayer->pev->armorvalue == 0){
				pPlayer->SetSuitUpdate("!HEV_NOPOWER", FALSE, SUIT_REPEAT_OK);		 
			}else{
				//pPlayer->SetSuitUpdateNumber(pPlayer->pev->armorvalue, SUIT_REPEAT_OK);
				//5000 means, update at message-play time.
				pPlayer->SetSuitUpdateNumber(5000, SUIT_REPEAT_OK);

			}
			*/

			//??
			//////pPlayer->SetSuitUpdate("!HEV_BPERCENT", FALSE, SUIT_REPEAT_OK, 0.8f);		 
			
			return TRUE;		
		}
		return FALSE;
	}
};

LINK_ENTITY_TO_CLASS(item_battery, CItemBattery);






class CItemAntidote : public CItem
{
	void Spawn( void )
	{ 
		Precache( );
		setModel("models/w_antidote.mdl");
		CItem::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/w_antidote.mdl");
	}

	//MODDD - quite a few additions to make it function better as an item.
	BOOL MyTouch( CBasePlayer *pPlayer )
	{
		//player must be alive.
		if ( pPlayer->pev->deadflag != DEAD_NO || !(pPlayer->pev->weapons & (1<<WEAPON_SUIT))    )
		{
			return FALSE;
		}
		

		//Max of 5 antidotes (or any type of power canister).
		if(pPlayer->m_rgItems[ITEM_ANTIDOTE] < ITEM_ANTIDOTE_MAX){

			UTIL_PlaySound( pPlayer->edict(), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM, FALSE );

			pPlayer->SetSuitUpdate("!HEV_ANT_PICKUP", FALSE, SUIT_NEXT_IN_1MIN);
		
			pPlayer->m_rgItems[ITEM_ANTIDOTE] += 1;

			//ASSERT( gmsgAntidoteP > 0 );
		

			if(gmsgAntidoteP > 0){
			MESSAGE_BEGIN( MSG_ONE, gmsgAntidoteP, NULL, pPlayer->pev );
			WRITE_SHORT( pPlayer->m_rgItems[ITEM_ANTIDOTE]);
			MESSAGE_END();
			}
		
			MESSAGE_BEGIN( MSG_ONE, gmsgItemPickup, NULL, pPlayer->pev );
			WRITE_STRING( STRING(pev->classname) );
			MESSAGE_END();


			return TRUE;
		}//END OF if(pPlayer->m_rgItems[ITEM_ANTIDOTE] < 5)

		//I'm guessing returning "TRUE" means delete the item now (it has been applied to the player, can't be again),
		//and "FALSE" means whatever condition wasn't applied, so leave it untouched (like at 100 health upon touching
		//a medkit, or for antidotes, already having 5).
		return FALSE;
	}
};

LINK_ENTITY_TO_CLASS(item_antidote, CItemAntidote);





//MODDD - new classes, for the other pickup injectibles.
class CItemAdrenaline : public CItem
{
	void Spawn( void )
	{ 
		Precache( );
		setModel("models/w_adrenaline.mdl");
		CItem::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/w_adrenaline.mdl");
	}

	BOOL MyTouch( CBasePlayer *pPlayer )
	{


		//player must be alive.
		if ( pPlayer->pev->deadflag != DEAD_NO || !(pPlayer->pev->weapons & (1<<WEAPON_SUIT))    )
		{
			return FALSE;
		}

		//Max of 5 adrenalines (or any type of power canister).
		if(pPlayer->m_rgItems[ITEM_ADRENALINE] < ITEM_ADRENALINE_MAX){

			UTIL_PlaySound( pPlayer->edict(), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM, FALSE );

			pPlayer->SetSuitUpdate("!HEV_ADR_PICKUP", FALSE, SUIT_NEXT_IN_1MIN);
		
			pPlayer->m_rgItems[ITEM_ADRENALINE] += 1;

			//ASSERT( gmsgAdrenalineP > 0 );
		

			if(gmsgAdrenalineP > 0){
				MESSAGE_BEGIN( MSG_ONE, gmsgAdrenalineP, NULL, pPlayer->pev );
				WRITE_SHORT( pPlayer->m_rgItems[ITEM_ADRENALINE]);
				MESSAGE_END();
			}
		
			MESSAGE_BEGIN( MSG_ONE, gmsgItemPickup, NULL, pPlayer->pev );
			WRITE_STRING( STRING(pev->classname) );
			MESSAGE_END();


			return TRUE;
		}//END OF if(pPlayer->m_rgItems[ITEM_ADRENALINE] < 5)

		//I'm guessing returning "TRUE" means delete the item now (it has been applied to the player, can't be again),
		//and "FALSE" means whatever condition wasn't applied, so leave it untouched (like at 100 health upon touching
		//a medkit, or for antidotes, already having 5).
		return FALSE;
	}
};

LINK_ENTITY_TO_CLASS(item_adrenaline, CItemAdrenaline);







//MODDD - new classes, for the other pickup injectibles.
class CItemRadiation : public CItem
{
	void Spawn( void )
	{ 
		Precache( );
		setModel("models/w_rad.mdl");
		CItem::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/w_rad.mdl");
	}

	BOOL MyTouch( CBasePlayer *pPlayer )
	{


		//player must be alive.
		if ( pPlayer->pev->deadflag != DEAD_NO || !(pPlayer->pev->weapons & (1<<WEAPON_SUIT))    )
		{
			return FALSE;
		}

		//Max of 5 antidotes (or any type of power canister).
		if(pPlayer->m_rgItems[ITEM_RADIATION] < ITEM_RADIATION_MAX){

			UTIL_PlaySound( pPlayer->edict(), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM, FALSE );

			pPlayer->SetSuitUpdate("!HEV_RAD_PICKUP", FALSE, SUIT_NEXT_IN_1MIN);
		
			pPlayer->m_rgItems[ITEM_RADIATION] += 1;

			//ASSERT( gmsgRadiationP > 0 );
		

			if(gmsgRadiationP > 0){
			MESSAGE_BEGIN( MSG_ONE, gmsgRadiationP, NULL, pPlayer->pev );
			WRITE_SHORT( pPlayer->m_rgItems[ITEM_RADIATION]);
			MESSAGE_END();
			}
		
			MESSAGE_BEGIN( MSG_ONE, gmsgItemPickup, NULL, pPlayer->pev );
			WRITE_STRING( STRING(pev->classname) );
			MESSAGE_END();


			return TRUE;
		}//END OF if(pPlayer->m_rgItems[ITEM_RADIATION] < 5)

		//I'm guessing returning "TRUE" means delete the item now (it has been applied to the player, can't be again),
		//and "FALSE" means whatever condition wasn't applied, so leave it untouched (like at 100 health upon touching
		//a medkit, or for antidotes, already having 5).
		return FALSE;
	}
};

LINK_ENTITY_TO_CLASS(item_radiation, CItemRadiation);






class CItemSecurity : public CItem
{
	void Spawn( void )
	{ 
		Precache( );
		setModel("models/w_security.mdl");
		CItem::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/w_security.mdl");
		PRECACHE_SOUND( "items/gunpickup2.wav" );
	}
	BOOL MyTouch( CBasePlayer *pPlayer )
	{
		//MODDD - play the general item pickup sound
		UTIL_PlaySound( pPlayer->edict(), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM, FALSE );


		pPlayer->m_rgItems[ITEM_SECURITY] += 1;
		return TRUE;
	}
};

LINK_ENTITY_TO_CLASS(item_security, CItemSecurity);




/*
class CItemLongJump : public CItem
{
	void Spawn( void )
	{ 
		Precache( );
		setModel("models/w_longjump.mdl");
		CItem::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/w_longjump.mdl");
	}
	BOOL MyTouch( CBasePlayer *pPlayer )
	{
		//MODDD - the player probably always has "m_fLongJump" on now.  Use another var for measuring whether to
		//pick up the long jump a first time or not (just to satisfy the Hazard course).
		//Note that "canTakeLongJumpMem" being 2 means it can be picked up any time, even if picked up before (to restore all long jump charge).
		
		//if ( pPlayer->m_fLongJump )
		//if(pPlayer->m_fLongJump && !(EASY_CVAR_GET_DEBUGONLY(canTakeLongJump) == 2) && (EASY_CVAR_GET_DEBUGONLY(canTakeLongJump) == 0 || (EASY_CVAR_GET_DEBUGONLY(canTakeLongJump) == 1 && pPlayer->hasLongJumpItem) ) )
		float canTakeLongJumpVal = EASY_CVAR_GET_DEBUGONLY(canTakeLongJump);

		if (pPlayer->m_fLongJump) {
			// If the player has the longjump module (although having ever picked up "item_longjump"
			// is recorded by "hasLongJumpItem" instead now), check the canTakeLongJump CVar for
			// when to disallow picking up item_l
			if (canTakeLongJumpVal == 0) {
				// nope.  "m_fLongJump" being on is all it takes to be denied.
				return FALSE;
			}
			else if (canTakeLongJumpVal == 1) {
				// Can only get item_longjump once.
				if (pPlayer->hasLongJumpItem) {
					return FALSE;
				}
			}
			else if (canTakeLongJumpVal == 2 || canTakeLongJumpVal == 3) {
				//proceed.
			}
		}

		//can't pick up regardless if longJumpCharge is full.
		if(pPlayer->longJumpCharge >= PLAYER_LONGJUMPCHARGE_MAX){
			return FALSE;
		}
		

		if ( ( pPlayer->pev->weapons & (1<<WEAPON_SUIT) ) )
		{
			//MODDD - play this.
			UTIL_PlaySound( pPlayer->edict(), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM, FALSE );


			pPlayer->m_fLongJump = TRUE;// player now has longjump module
			
			//MODDD - also set "longJumpCharge" from -1 to 0 so that the GUI can draw the icon now.
			//NOTICE: not doing that update here.  The player should detect when there is a descrepency between
			//the "m_fLongJump" var and another memory one, in order to change the longJumpCharge from -1 to 0 and
			//then make the client (GUI) update.
			//pPlayer->longJumpCharge = 0;
			//pPlayer->longJumpChargeNeedsUpdate = TRUE;

			//Not from that alone anymore, "slj" is turned on when we also have enough charge to make a long jump, and the successive
			//delay has been satisfied.
			//g_engfuncs.pfnSetPhysicsKeyValue( pPlayer->edict(), "slj", "1" );

			//eh, I guess this should be okay.   The longjump starts with full charge.
			pPlayer->longJumpCharge = PLAYER_LONGJUMPCHARGE_MAX;
			pPlayer->hasLongJumpItem = TRUE;

			MESSAGE_BEGIN( MSG_ONE, gmsgItemPickup, NULL, pPlayer->pev );
				WRITE_STRING( STRING(pev->classname) );
			MESSAGE_END();


			//MODDD - we have a better way to make the call compatible with the new (improved?) queue system.
			//EMIT_SOUND_SUIT( pPlayer->edict(), "!HEV_A1" );	// Play the longjump sound UNDONE: Kelly? correct sound?
			//pPlayer->SetSuitUpdate("!HEV_LJC_PICKUP", FALSE, SUIT_NEXT_IN_1MIN*2);
			pPlayer->SetSuitUpdate("!HEV_A1", FALSE, SUIT_NEXT_IN_1MIN * 2);

			return TRUE;		
		}
		return FALSE;
	}
};

LINK_ENTITY_TO_CLASS( item_longjump, CItemLongJump );
*/





//MODDD - new.  This assumes the player already has acquired "item_longjump" (otherwise, not pickup-able) and adds charge to the
//player's longjump module.
// NOTICE - idea of a separate "CItemLongJumpCharge" and "CItemLongJump" removed.
// There will only be "CItemLongJump" which works like the "Charge"  (grant 25 charge on pickup, always allowed to be picked up).
// In short, this has been changed from CItemLongJumpCharge to CItemLongJump, original CItemLongJump commented out above.
// Similar change made to the entity name further below.
class CItemLongJump : public CItem
{
	void Spawn( void )
	{ 
		Precache( );
		setModel("models/w_longjump.mdl");
		CItem::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/w_longjump.mdl");
	}
	BOOL MyTouch( CBasePlayer *pPlayer )
	{
		if ( ( pPlayer->pev->weapons & (1<<WEAPON_SUIT) && pPlayer->m_fLongJump && pPlayer->longJumpCharge < PLAYER_LONGJUMPCHARGE_MAX ) )
		{

			// CVAR NOW INEFFECTIVE.
			/*
			float canTakeLongJumpVal = EASY_CVAR_GET_DEBUGONLY(canTakeLongJump);
			// hm...
			if (canTakeLongJumpVal == 0) {
				// Take longjumpcharges whenever.  Picking up item_longjump is impossible.
				// Only m_fLongJump is required.
			}
			else if (canTakeLongJumpVal == 1 || canTakeLongJumpVal == 2) {
				// Must have come across item_longjump before.  Otherwise, no go.
				if (!pPlayer->hasLongJumpItem) {
					return FALSE;
				}
			}
			else {
				//Take longjumpcharges whenever.
			}
			*/


			pPlayer->longJumpCharge += PLAYER_LONGJUMP_PICKUPADD;
			pPlayer->longJumpCharge = min(pPlayer->longJumpCharge, PLAYER_LONGJUMPCHARGE_MAX);
			
			pPlayer->longJumpChargeNeedsUpdate = TRUE;
			

			MESSAGE_BEGIN( MSG_ONE, gmsgItemPickup, NULL, pPlayer->pev );
				WRITE_STRING( STRING(pev->classname) );
			MESSAGE_END();

			UTIL_PlaySound( pPlayer->edict(), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM, FALSE );

			pPlayer->SetSuitUpdate("!HEV_LJC_PICKUP", FALSE, SUIT_NEXT_IN_1MIN*2 );
			
			return TRUE;
		}
		return FALSE;
	}
};

//LINK_ENTITY_TO_CLASS( item_longjumpcharge, CItemLongJumpCharge );
LINK_ENTITY_TO_CLASS(item_longjump, CItemLongJump);






//MODDD - new.  Provides the glock silencer as a pickup.
//TODO: model for the silencer alone?  This is using the glock with the silencer, the silencer alone is not here as a model.
class CItemGlockSilencer : public CItem
{
	void Spawn( void )
	{ 
		Precache( );
		setModel("models/w_silencer.mdl");
		CItem::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/w_silencer.mdl");
	}
	BOOL MyTouch( CBasePlayer *pPlayer )
	{
		if ( ( pPlayer->pev->weapons & (1<<WEAPON_SUIT) && pPlayer->hasGlockSilencer == 0 ) )
		{
			
			//MODDD - play this.
			UTIL_PlaySound( pPlayer->edict(), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM, FALSE );

			/*
			MESSAGE_BEGIN( MSG_ONE, gmsgItemPickup, NULL, pPlayer->pev );
				WRITE_STRING( STRING(pev->classname) );
			MESSAGE_END();
			*/

			if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(wpn_glocksilencer) == 1){
				pPlayer->SetSuitUpdate("!HEV_SILENCER", FALSE, SUIT_NEXT_IN_30MIN);
			}
			pPlayer->hasGlockSilencer = 1;
			
			return TRUE;		
		}
		return FALSE;
	}
};

LINK_ENTITY_TO_CLASS( item_glocksilencer, CItemGlockSilencer );
