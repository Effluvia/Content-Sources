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


#pragma once

#include "extdll.h"
#include "satchel.h"
#include "util.h"
#include "cbase.h"
#include "basemonster.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "gamerules.h"


EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelay)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelaycustom)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteclip)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteammo)



// "ChargeReady" values, get through accessor/setters named like that, are as follows:
// 0: ready to toss a satchel, left or right-click
// 1: satchel(s) deployed, left click to detonate ones deployed or right click to toss another (if there is ammo)
// 2: recently detonated satchels, button-press animation played.  Little delay before Charge goes to 0 IF there is ammo.



//MODDD - why wasn't this serverside-only anyway??
#ifndef CLIENT_DLL
//=========================================================
// DeactivateSatchels - removes all satchels owned by
// the provided player. Should only be used upon death.
//
// Made this global on purpose.
//=========================================================
void DeactivateSatchels(CBasePlayer* pOwner)
{
	edict_t* pFind;

	pFind = FIND_ENTITY_BY_CLASSNAME(NULL, "monster_satchel");

	while (!FNullEnt(pFind))
	{
		CBaseEntity* pEnt = CBaseEntity::Instance(pFind);
		CSatchelCharge* pSatchel = (CSatchelCharge*)pEnt;

		if (pSatchel)
		{
			if (pSatchel->pev->owner == pOwner->edict())
			{
				pSatchel->Deactivate();
			}
		}

		pFind = FIND_ENTITY_BY_CLASSNAME(pFind, "monster_satchel");
	}
}
#endif //CLIENT_DLL check




//MODDD - why wasn't this class always marked serverside-only??
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef CLIENT_DLL

LINK_ENTITY_TO_CLASS( monster_satchel, CSatchelCharge );

//=========================================================
// Deactivate - do whatever it is we do to an orphaned 
// satchel when we don't want it in the world anymore.
//=========================================================
void CSatchelCharge::Deactivate( void )
{
	pev->solid = SOLID_NOT;
	UTIL_Remove( this );
}


void CSatchelCharge::Spawn( void )
{
	Precache( );
	// motor
	pev->movetype = MOVETYPE_BOUNCE;
	pev->solid = SOLID_BBOX;

	SET_MODEL(ENT(pev), "models/w_satchel.mdl");
	//UTIL_SetSize(pev, Vector( -16, -16, -4), Vector(16, 16, 32));	// Old box -- size of headcrab monsters/players get blocked by this
	UTIL_SetSize(pev, Vector( -4, -4, -4), Vector(4, 4, 4));	// Uses point-sized, and can be stepped over
	UTIL_SetOrigin( pev, pev->origin );

	SetTouch( &CSatchelCharge::SatchelSlide );
	SetUse( &CGrenade::DetonateUse );
	SetThink( &CSatchelCharge::SatchelThink );
	pev->nextthink = gpGlobals->time + 0.1;

	pev->gravity = 0.5;
	pev->friction = 0.8;

	pev->dmg = gSkillData.plrDmgSatchel;
	// ResetSequenceInfo( );
	pev->sequence = 1;
}


void CSatchelCharge::SatchelSlide( CBaseEntity *pOther )
{
	entvars_t	*pevOther = pOther->pev;

	// don't hit the guy that launched this grenade
	if ( pOther->edict() == pev->owner )
		return;


	float flInterval = DetermineInterval();


	// pev->avelocity = Vector (300, 300, 300);
	pev->gravity = 1;// normal gravity now

	// HACKHACK - On ground isn't always set, so look for ground underneath
	TraceResult tr;
	UTIL_TraceLine( pev->origin, pev->origin - Vector(0,0,10), ignore_monsters, edict(), &tr );

	if ( tr.flFraction < 1.0 )
	{
		// add a bit of static friction
		pev->velocity = pev->velocity * 0.95;
		pev->avelocity = pev->avelocity * 0.9;
		// play sliding sound, volume based on velocity
	}

	//MODDD - TODO.  Like the grenade, check to see if there is ground shortly underneath despite FL_ONGROUND
	// not being set.  That menas it is likely this is going over an incline and should slow down a lot faster
	// instead.


	if ( !(pev->flags & FL_ONGROUND) && pev->velocity.Length2D() > 10 )
	{
		BounceSound();
	}


	//MODDD - workings of StudioFrameAdvance changed.
	// Default calls to StudioFrameAdvance are still ok, but ones that happen late in think methods should be replaced
	// with a DetermineInterval step further above, and give its flInterval to StudioFrameAdvance in the same place it was.
	//StudioFrameAdvance( );
	StudioFrameAdvance(flInterval);
}


void CSatchelCharge::SatchelThink( void )
{
	StudioFrameAdvance_SIMPLE( );
	pev->nextthink = gpGlobals->time + 0.1;

	if (!IsInWorld())
	{
		UTIL_Remove( this );
		return;
	}

	if (pev->waterlevel == 3)
	{
		pev->movetype = MOVETYPE_FLY;
		pev->velocity = pev->velocity * 0.8;
		pev->avelocity = pev->avelocity * 0.9;
		pev->velocity.z += 8;
	}
	else if (pev->waterlevel == 0)
	{
		pev->movetype = MOVETYPE_BOUNCE;
	}
	else
	{
		pev->velocity.z -= 8;
	}	
}

void CSatchelCharge::Precache( void )
{
	PRECACHE_MODEL("models/grenade.mdl");
	PRECACHE_SOUND("weapons/g_bounce1.wav");
	PRECACHE_SOUND("weapons/g_bounce2.wav");
	PRECACHE_SOUND("weapons/g_bounce3.wav");
}

void CSatchelCharge::BounceSound( void )
{
	//MODDD - don't spam bounce sounds!
	if (gpGlobals->time >= nextBounceSoundAllowed) {
		// okay.
	}
	else {
		// oh.
		return;
	}
	nextBounceSoundAllowed = gpGlobals->time + 0.22;

	switch ( RANDOM_LONG( 0, 2 ) )
	{
	case 0:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/g_bounce1.wav", 1, ATTN_NORM);	break;
	case 1:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/g_bounce2.wav", 1, ATTN_NORM);	break;
	case 2:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/g_bounce3.wav", 1, ATTN_NORM);	break;
	}
}


LINK_ENTITY_TO_CLASS( weapon_satchel, CSatchel );



float CSatchelCharge::massInfluence(void){
	return 0.18f;
}
int CSatchelCharge::GetProjectileType(void){
	//do we need a custom type for remote charges like this?
	return PROJECTILE_GRENADE;
}

#endif //CLIENT_DLL check
///////////////////////////////////////////////////////////////////////////////////////////////////




CSatchel::CSatchel() {

	//NOTE: this used to be a custom var named "weaponRetired", but it does not seem to sync too well b/w the server & client.
	//Now reusing an existing sync'd var.
	//daPoopah = FALSE;

	//alreadySentSatchelOutOfAmmoNotice = FALSE;
	sentOutOfAmmoHolster = FALSE;

}


// Save/restore for serverside only!
#ifndef CLIENT_DLL
TYPEDESCRIPTION	CSatchel::m_SaveData[] =
{
	DEFINE_FIELD(CSatchel, m_chargeReady, FIELD_INTEGER),
};
IMPLEMENT_SAVERESTORE(CSatchel, CBasePlayerWeapon);
#endif


//=========================================================
// CALLED THROUGH the newly-touched weapon's instance. The existing player weapon is pOriginal
//=========================================================
int CSatchel::AddDuplicate( CBasePlayerItem *pOriginal )
{
	CSatchel *pSatchel;

	if ( IsMultiplayer() )
	{
		pSatchel = (CSatchel *)pOriginal;

		if ( pSatchel->getchargeReady() != 0 )
		{
			// player has some satchels deployed. Refuse to add more.
			return FALSE;
		}
	}

	return CBasePlayerWeapon::AddDuplicate ( pOriginal );
}

//=========================================================
//=========================================================
int CSatchel::AddToPlayer( CBasePlayer *pPlayer )
{
	int bResult = CBasePlayerItem::AddToPlayer( pPlayer );

	
	// just in case it was?
	//alreadySentSatchelOutOfAmmoNotice = FALSE;
	sentOutOfAmmoHolster = FALSE;

	pPlayer->pev->weapons |= (1<<m_iId);
	this->setchargeReady(0);// this satchel charge weapon now forgets that any satchels are deployed by it.

	if ( bResult )
	{
		return AddWeapon( );
	}
	return FALSE;
}


//MODDD
void CSatchel::customAttachToPlayer(CBasePlayer *pPlayer ){
	m_pPlayer->SetSuitUpdate("!HEV_SATCHEL", FALSE, SUIT_NEXT_IN_30MIN);
}

void CSatchel::Spawn( )
{
	Precache( );
	m_iId = WEAPON_SATCHEL;
	SET_MODEL(ENT(pev), "models/w_satchel.mdl");

	m_iClip = -1;
	m_iDefaultAmmo = SATCHEL_DEFAULT_GIVE;
		
	FallInit();// get ready to fall down.

	//daPoopah = FALSE;
}


void CSatchel::Precache( void )
{
	PRECACHE_MODEL("models/v_satchel.mdl");
	PRECACHE_MODEL("models/v_satchel_radio.mdl");
	PRECACHE_MODEL("models/w_satchel.mdl");
	PRECACHE_MODEL("models/p_satchel.mdl");
	PRECACHE_MODEL("models/p_satchel_radio.mdl");

	precacheGunPickupSound();
	precacheAmmoPickupSound(); //doubles as ammo.

	UTIL_PrecacheOther( "monster_satchel" );
}


int CSatchel::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "Satchel Charge";
	p->iMaxAmmo1 = SATCHEL_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 4;
	p->iPosition = 1;
	p->iFlags = ITEM_FLAG_SELECTONEMPTY | ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE;
	p->iId = m_iId = WEAPON_SATCHEL;
	p->iWeight = SATCHEL_WEIGHT;

	return 1;
}

//=========================================================
//=========================================================
BOOL CSatchel::IsUseable( void )
{
	if ( PlayerPrimaryAmmoCount() > 0 ) 
	{
		// player is carrying some satchels
		return TRUE;
	}

	if ( getchargeReady() != 0 )
	{
		// player isn't carrying any satchels, but has some out
		return TRUE;
	}

	return FALSE;
}

BOOL CSatchel::CanDeploy( void )
{
	if ( PlayerPrimaryAmmoCount() > 0 ) 
	{
		// player is carrying some satchels
		return TRUE;
	}

	if ( getchargeReady() != 0 )
	{
		// player isn't carrying any satchels, but has some out
		return TRUE;
	}

	return FALSE;
}

BOOL CSatchel::Deploy( )
{
	// YO
	//daPoopah = FALSE;

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.0;
	
	//no, done below.
	//m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );

	//easyPrintLine("DEPLOYYYYY %d", m_chargeReady);

	int chargeReadyVal = getchargeReady();

	float extraIdleTime;

	/*
#ifndef CLIENT_DLL
	if (getchargeReady() == 2) {
		// keep it at 0, need to redeploy soon.
		extraIdleTime = 0;
	}
	else {
		extraIdleTime = randomIdleAnimationDelay();
	}
#else
	// don't bother clientside?
	extraIdleTime = 0;
#endif
	*/

	extraIdleTime = randomIdleAnimationDelay();

	// Nevermind, just do this.
	if (chargeReadyVal != 1) {
		// Not 1?  Either placing the first satchel or waiting to re-deploy a satchel after firing recently (so skip to doing so).
		//if (getchargeReady() != 1 && PlayerPrimaryAmmoCount() <= 0)
		if (PlayerPrimaryAmmoCount() >= 0) {
			//ReDeploySatchel();
			setchargeReady(0);
			return DefaultDeploy("models/v_satchel.mdl", "models/p_satchel.mdl", SATCHEL_DRAW, "trip", 0, 0, (61.0 / 30.0) + extraIdleTime, (24.0 / 30.0));
		}
		else {
			// ?????   Is it safe to do this here?
			m_pPlayer->pev->weapons &= ~(1 << WEAPON_SATCHEL);
			SetThink(&CBasePlayerItem::DestroyItem);
			pev->nextthink = gpGlobals->time + 0.1;
			return FALSE;
		}
	}
	else {
		// matches 1?  Get the radio out for the charges currently out there.
		return DefaultDeploy("models/v_satchel_radio.mdl", "models/p_satchel_radio.mdl", SATCHEL_RADIO_DRAW, "hive", 0, 0, (19.0 / 30.0) + extraIdleTime, (12.0 / 30.0));
	}

	return TRUE;
}


void CSatchel::Holster( int skiplocal /* = 0 */ )
{
	//daPoopah = FALSE;
	//m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	

	//MODDD - If the remote isn't out or recently detonated already, put it away then!
	
	if (PlayerPrimaryAmmoCount() <= 0 && getchargeReady() != 1)
	{
		setchargeReady(0);
		//RetireWeapon();
		//daPoopah = TRUE;
		//return;

		// Out of satchels, nothing deployed out there to be detonated.


		/*

#ifndef CLIENT_DLL
		if (m_pPlayer->fHolsterAnimsEnabled) {
			m_pPlayer->pev->weapons &= ~(1 << WEAPON_SATCHEL);
			SetThink(&CBasePlayerItem::DestroyItem);
			pev->nextthink = gpGlobals->time + 0.8;
		}
		else
#endif
		{
			m_pPlayer->pev->weapons &= ~(1 << WEAPON_SATCHEL);
			SetThink(&CBasePlayerItem::DestroyItem);
			pev->nextthink = gpGlobals->time + 0.1;
			return;
		}
		*/


		m_pPlayer->pev->weapons &= ~(1 << WEAPON_SATCHEL);
		SetThink(&CBasePlayerItem::DestroyItem);
		pev->nextthink = gpGlobals->time + 0.1;
		return;
	}



	{
		int holsterAnimToSend;
		if ( getchargeReady() )
		{
			holsterAnimToSend = SATCHEL_RADIO_HOLSTER;
		}
		else
		{
			holsterAnimToSend = SATCHEL_DROP;
		}
	
		DefaultHolster(holsterAnimToSend, skiplocal, 0, (16.0f/30.0f));
	}

	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "common/null.wav", 1.0, ATTN_NORM);
}


void CSatchel::PrimaryAttack()
{
	switch (getchargeReady() )
	{
	case 0:
		{
		Throw( );
		}
		break;
	case 1:
		{

		if (!(m_pPlayer->m_afButtonPressed & IN_ATTACK)) {
			//MODDD
			// Only allow detonation on a solid click, not holding it down.  Sure want this to be intentional.
			break;
		}

		SendWeaponAnim( SATCHEL_RADIO_FIRE );

		edict_t *pPlayer = m_pPlayer->edict( );

		CBaseEntity *pSatchel = NULL;


		while ((pSatchel = UTIL_FindEntityInSphere( pSatchel, m_pPlayer->pev->origin, 4096 )) != NULL)
		{
			if (FClassnameIs( pSatchel->pev, "monster_satchel"))
			{
				if (pSatchel->pev->owner == pPlayer)
				{
					pSatchel->Use( m_pPlayer, m_pPlayer, USE_ON, 0 );
					// wait.  what's the point of this call if below already sets it regardless of finding any
					// satchels.  Nevermind.
					//setchargeReady(2);
				}
			}
		}

		// NOTE - 'setchargeReady(2)' can get overridden by the PlayerPrimaryAmmoCount() check below being 0, and that is fine.
		setchargeReady(2);
		SetAttackDelays(UTIL_WeaponTimeBase() + 0.5);

		//MODDD - match anim time
		//m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (31.0/30.0);
		break;
		}

	case 2:
		// we're reloading, don't allow fire
		{
		}
		break;
	}


	//MODDD - check.  See if this should be removed soon.
	// That is, not with charges out already (chargeReady == 1) nor ammo in reserve.
	// Note that a Throw() call with 0 ammo doesn't do anything.
	if (getchargeReady() != 1 && PlayerPrimaryAmmoCount() <= 0)
	{
		setchargeReady(0);
		// not yet!  Wait for animation to end
		//RetireWeapon();
	}

}


void CSatchel::SecondaryAttack( void )
{
	int theChargeReady = getchargeReady();



	if (theChargeReady == 0) {

		// NEVERMIND THAT.  no one would like this.
		// People want to throw the charge, not look at a funny figet.
		/*
		if (EASY_CVAR_GET(cl_viewmodel_fidget) == 2) {
			if (PlayerPrimaryAmmoCount() > 0) {
				//float flRand;
				int iAnim;

				//flRand = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 0, 1);
				//if (flRand <= 0.5)
				//{
				iAnim = SATCHEL_FIDGET1;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 61.0 / 30.0;
				//}

				//SetAttackDelays(m_flTimeWeaponIdle);
				m_flNextSecondaryAttack = m_flTimeWeaponIdle;
				m_flTimeWeaponIdle += randomIdleAnimationDelay();
				SendWeaponAnim(iAnim);
			}
		}
		else
		*/
		{
			// not 2?  go ahead and throw here too.
			Throw();
		}//CVar check


	}else if (theChargeReady == 1)
	{
		Throw( );
		// why nextattack delays here?  Throw handles that
	}

}


void CSatchel::Throw( void )
{
	if (PlayerPrimaryAmmoCount() > 0)
	{
		Vector vecSrc = m_pPlayer->pev->origin;

		Vector vecThrow = gpGlobals->v_forward * 274 + m_pPlayer->pev->velocity;

#ifndef CLIENT_DLL
		CBaseEntity *pSatchel = Create( "monster_satchel", vecSrc, Vector( 0, 0, 0), m_pPlayer->edict() );
		pSatchel->pev->velocity = vecThrow;
		pSatchel->pev->avelocity.y = 400;

		m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_satchel_radio.mdl");
		m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_satchel_radio.mdl");
#else
		LoadVModel ( "models/v_satchel_radio.mdl", m_pPlayer );
#endif

		SendWeaponAnim( SATCHEL_RADIO_DRAW );
		

		//MODDD - addition.
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (19.0/30.0) + randomIdleAnimationDelay();


		// player "shoot" animation
		m_pPlayer->SetAnimation( PLAYER_ATTACK1 );


		// safety?  Make sure a notice to stop blocking deployment of the weapon can be sent this way.
		// TEST, no.
		//alreadySentSatchelOutOfAmmoNotice = TRUE;
		//sentOutOfAmmoHolster = FALSE;
		setchargeReady(1);
		
		
		//MODDD - cheat check.			
		if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteclip) == 0 && EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteammo) == 0){
			ChangePlayerPrimaryAmmoCount(-1);
		}

		//MODDD
		//NOTE: Primary fire isn't affected here since this may be the first charge (holding any longer would make it blow up
		// in the player's face)... no longer the case, just check for solid key-presses instead.
		// also, times changed a bit.  Used to be 1.0 and 0.5.
		if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelay) == 0){
			SetAttackDelays(UTIL_WeaponTimeBase() + 0.5);
		}else{
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelaycustom);
			//they stick together sometimes, so they get an extra delay.
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelaycustom) + 0.03f;
		}

	}//END OF ammo check
}



//BOOL ForceNoSelectOnEmptyRecentSendoff = FALSE;

// This method doesn't tell whether this is out of ammo, it does the check within itself and acts on that.
// This should be called by anything external that could affect whether this weapon is still a valid selection
// without jumping to equip ('removeammo' command).
void CSatchel::CheckOutOfAmmo() {
#ifndef CLIENT_DLL
	if(m_pPlayer != NULL){

		if ((PlayerPrimaryAmmoCount() <= 0 && getchargeReady() != 1)) {
			if (!m_pPlayer->alreadySentSatchelOutOfAmmoNotice) {
				// send it!  Let the weapon's place on the HUD's weapon-select know it can turn red, ordinarily it doesn't know to.
				m_pPlayer->alreadySentSatchelOutOfAmmoNotice = TRUE;

				MESSAGE_BEGIN(MSG_ONE, gmsgCurWeaponForceNoSelectOnEmpty, NULL, m_pPlayer->pev);
				WRITE_CHAR(m_iId);
				WRITE_BYTE(TRUE);
				MESSAGE_END();

				//ForceNoSelectOnEmptyRecentSendoff = TRUE;

			}
		}else if((PlayerPrimaryAmmoCount() > 0) ){
			// got some ammo back?  OK

			if(m_pPlayer->alreadySentSatchelOutOfAmmoNotice){
				m_pPlayer->alreadySentSatchelOutOfAmmoNotice = FALSE;

				MESSAGE_BEGIN(MSG_ONE, gmsgCurWeaponForceNoSelectOnEmpty, NULL, m_pPlayer->pev);
				WRITE_CHAR(m_iId);
				WRITE_BYTE(FALSE);
				MESSAGE_END();

				Deploy();

			}
			
		}


		/*
		if ((PlayerPrimaryAmmoCount() <= 0 && getchargeReady() != 1)) {
			if (!alreadySentSatchelOutOfAmmoNotice) {
				// send it!  Let the weapon's place on the HUD's weapon-select know it can turn red, ordinarily it doesn't know to.
				alreadySentSatchelOutOfAmmoNotice = TRUE;

				MESSAGE_BEGIN(MSG_ONE, gmsgCurWeaponForceNoSelectOnEmpty, NULL, m_pPlayer->pev);
				WRITE_CHAR(m_iId);
				WRITE_BYTE(TRUE);
				MESSAGE_END();

				//ForceNoSelectOnEmptyRecentSendoff = TRUE;

			}
		}else if(daPoopah == TRUE && (PlayerPrimaryAmmoCount() > 0) ){
			// got some ammo back?  OK
			daPoopah = FALSE;

			alreadySentSatchelOutOfAmmoNotice = FALSE;

			MESSAGE_BEGIN(MSG_ONE, gmsgCurWeaponForceNoSelectOnEmpty, NULL, m_pPlayer->pev);
			WRITE_CHAR(m_iId);
			WRITE_BYTE(FALSE);
			MESSAGE_END();

			Deploy();
		}
		*/

	}
	/*
	//  && ForceNoSelectOnEmptyRecentSendoff != FALSE
	else if (alreadySentSatchelOutOfAmmoNotice) {
		
		alreadySentSatchelOutOfAmmoNotice = FALSE;  // can re-send on running out of ammo again.
		sentOutOfAmmoHolster = FALSE;

		MESSAGE_BEGIN(MSG_ONE, gmsgCurWeaponForceNoSelectOnEmpty, NULL, m_pPlayer->pev);
		WRITE_CHAR(m_iId);
		WRITE_BYTE(FALSE);
		MESSAGE_END();

		//ForceNoSelectOnEmptyRecentSendoff = FALSE;

		// CHECK: is the viewmodel retired?  Only then deploy!
		if(daPoopah){
			Deploy(); // show the charge viewmodel again
			daPoopah = FALSE;
		}
		return;
	}else{
		// ???
		//alreadySentSatchelOutOfAmmoNotice = FALSE;
	}
	*/

#endif
}


void CSatchel::ItemPostFrameThink(void) {


#ifndef CLIENT_DLL
	
	CheckOutOfAmmo();

#endif


	CBasePlayerWeapon::ItemPostFrameThink();
}


void CSatchel::WeaponIdle( void )
{
	int randomAnim;

	//if the weapon is retired but we pick up ammo, re-deploy it.
	/*
	if(daPoopah && PlayerPrimaryAmmoCount() > 0){
		daPoopah = FALSE;
		DefaultDeploy( "models/v_satchel.mdl", "models/p_satchel.mdl", SATCHEL_DRAW, "trip", 0, 0, (99.0/30.0), (24.0/30.0) );
		return;
	}
	*/

	//easyPrintLine("WHAT THE time %.2f, %.2f", m_flTimeWeaponIdle, UTIL_WeaponTimeBase());

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;


	//MODDD - is this okay?
	if (m_pPlayer->pev->viewmodel == iStringNull) {
		if (PlayerPrimaryAmmoCount() > 0) {

			globalflag_muteDeploySound = TRUE;
			Deploy();
			globalflag_muteDeploySound = FALSE;

			return;
		}
	}


	switch( getchargeReady() )
	{
	case 0:

		if (PlayerPrimaryAmmoCount() > 0) {

			if (EASY_CVAR_GET(cl_viewmodel_fidget) == 1) {
				randomAnim = RANDOM_LONG(0, 1);
			}
			else {
				// never play fidget this way.
				randomAnim = 0;
			}

			if (randomAnim == 0) {
				SendWeaponAnim(SATCHEL_IDLE1);
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 101.0 / 20.0 + randomIdleAnimationDelay();
			}
			else {
				SendWeaponAnim(SATCHEL_FIDGET1);
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 61.0 / 30.0 + randomIdleAnimationDelay();
			}


			// use tripmine animations
			strcpy(m_pPlayer->m_szAnimExtention, "trip");
		}
		else {
			// no ammo?  uh-oh.

			//if (m_pPlayer->pev->viewmodel != iStringNull) {
				// make it so then.

//#ifndef CLIENT_DLL
			if (!sentOutOfAmmoHolster) {
				//this->Holster();
				SendWeaponAnimBypass(SATCHEL_RADIO_HOLSTER);
				sentOutOfAmmoHolster = TRUE;
			}
//#endif
			//}

		}
		break;
	case 1:
		randomAnim = RANDOM_LONG(0, 1);
		
		//easyPrintLine("RANDOM stuff no %d", randomAnim);
		if(randomAnim == 0){
			SendWeaponAnim( SATCHEL_RADIO_IDLE1 );
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 81.0/30.0 + randomIdleAnimationDelay();
		}else{
			
			SendWeaponAnim( SATCHEL_RADIO_FIDGET1 );
			//m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 111.0/30.0 - 0.9f + randomIdleAnimationDelay();
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 111.0/40.0 + randomIdleAnimationDelay();
		}

		// use hivehand animations
		strcpy( m_pPlayer->m_szAnimExtention, "hive" );
		break;
	case 2:
		ReDeploySatchel();
		break;
	}// END OF switch ON getChargeReady()

	//MODDD - no, specific to the anim that was chosen's time.
	//m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );// how long till we do this again.
}


//MODDD - convenience method
void CSatchel::ReDeploySatchel(void) {

	/*
	// check here no longer necessary, done a bit earlier
	if (PlayerPrimaryAmmoCount() <= 0)
	{
		setchargeReady(0);
		RetireWeapon();
		daPoopah = TRUE;
		return;
	}
	*/

#ifndef CLIENT_DLL
	m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_satchel.mdl");
	m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_satchel.mdl");
#else
	LoadVModel("models/v_satchel.mdl", m_pPlayer);
#endif

	SendWeaponAnim(SATCHEL_DRAW);

	// use tripmine animations
	strcpy(m_pPlayer->m_szAnimExtention, "trip");

	SetAttackDelays(UTIL_WeaponTimeBase() + 0.5);
	setchargeReady(0);
	//MODDD - addition.
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 61.0 / 30.0 + randomIdleAnimationDelay();
}//ReDeploySatchel


