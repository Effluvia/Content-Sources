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

#include "extdll.h"
#include "shotgun.h"
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
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST(playerWeaponSpreadMode)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(viewModelPrintouts)




#define SHOTGUN_RELOAD_CUTIN_TIME 0.06


// NOTE - all cases of pev->iuser1 replaced with m_fInAttack.
// Never change pev->iuser# values in weapons, don't really understand what odd hardcoded behavior or bugs
// that can cause.

#define SHOTGUN_BIT1 1
#define SHOTGUN_BIT2 2
#define SHOTGUN_BIT3 4
#define SHOTGUN_BIT4 8
#define SHOTGUN_BIT5 16
#define SHOTGUN_BIT6 32
#define SHOTGUN_BIT7 64
// is 128 valid?  No clue
#define SHOTGUN_BIT8 128


DLL_GLOBAL const Vector VECTOR_CONE_DM_SHOTGUN = Vector(0.08716, 0.04362, 0.00);// 10 degrees by 5 degrees
DLL_GLOBAL const Vector VECTOR_CONE_DM_DOUBLESHOTGUN = Vector(0.17365, 0.04362, 0.00); // 20 degrees by 5 degrees


// oops.
//BOOL makeNoise = FALSE;



LINK_ENTITY_TO_CLASS( weapon_shotgun, CShotgun );



CShotgun::CShotgun(){

	//NEW VAR.  If non-zero, we are cheating.  Better for syncing.
	m_chargeReady = 0;

	queueReload = FALSE;

}


// Save/restore for serverside only!
#ifndef CLIENT_DLL
TYPEDESCRIPTION	CShotgun::m_SaveData[] =
{

	// is m_fireState even used here?
	DEFINE_FIELD(CShotgun, m_fInAttack, FIELD_INTEGER),

	DEFINE_FIELD(CShotgun, m_flNextReload, FIELD_TIME),
	DEFINE_FIELD(CShotgun, m_fInSpecialReload, FIELD_INTEGER),
	// DEFINE_FIELD( CShotgun, m_iShell, FIELD_INTEGER ),
	DEFINE_FIELD(CShotgun, m_flPumpTime, FIELD_TIME),

};
IMPLEMENT_SAVERESTORE(CShotgun, CBasePlayerWeapon);
#endif


void CShotgun::Spawn( )
{
	Precache( );
	m_iId = WEAPON_SHOTGUN;
	SET_MODEL(ENT(pev), "models/w_shotgun.mdl");

	m_iDefaultAmmo = SHOTGUN_DEFAULT_GIVE;

	// start with it
	m_fInAttack |= (SHOTGUN_BIT5);

	FallInit();// get ready to fall
}


void CShotgun::Precache( void )
{
	PRECACHE_MODEL("models/v_shotgun.mdl");
	PRECACHE_MODEL("models/w_shotgun.mdl");
	PRECACHE_MODEL("models/p_shotgun.mdl");

	m_iShell = PRECACHE_MODEL ("models/shotgunshell.mdl");// shotgun shell

	PRECACHE_SOUND("items/9mmclip1.wav");              

	PRECACHE_SOUND ("weapons/dbarrel1.wav");//shotgun
	PRECACHE_SOUND ("weapons/sbarrel1.wav");//shotgun

	PRECACHE_SOUND ("weapons/reload1.wav");	// shotgun reload
	PRECACHE_SOUND ("weapons/reload3.wav");	// shotgun reload

//	PRECACHE_SOUND ("weapons/sshell1.wav");	// shotgun reload - played on client
//	PRECACHE_SOUND ("weapons/sshell3.wav");	// shotgun reload - played on client
	
	PRECACHE_SOUND ("weapons/357_cock1.wav"); // gun empty sound
	PRECACHE_SOUND ("weapons/scock1.wav");	// cock gun

	precacheGunPickupSound();

	m_usSingleFire = PRECACHE_EVENT( 1, "events/shotgun1.sc" );
	m_usDoubleFire = PRECACHE_EVENT( 1, "events/shotgun2.sc" );
}


//MODDD
void CShotgun::customAttachToPlayer(CBasePlayer *pPlayer ){
	m_pPlayer->SetSuitUpdate("!HEV_SHOTGUN", FALSE, SUIT_NEXT_IN_30MIN);
}

int CShotgun::AddToPlayer( CBasePlayer *pPlayer )
{
	if ( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev );
			WRITE_BYTE( m_iId );
		MESSAGE_END();
		return TRUE;
	}
	return FALSE;
}


int CShotgun::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "buckshot";
	p->iMaxAmmo1 = BUCKSHOT_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = SHOTGUN_MAX_CLIP;
	p->iSlot = 2;
	p->iPosition = 1;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_SHOTGUN;
	p->iWeight = SHOTGUN_WEIGHT;

	return 1;
}




//MODDD - new.
void CShotgun::Holster( int skiplocal /* = 0 */ )
{
	m_flReleaseThrow = -1;  //interrupt cock sound delay

	this->m_fireState &= ~128;  //nope.

	//Ha!  Got you.
	m_fInSpecialReload = 0;

	//CBasePlayerWeapon::Holster(skiplocal);
	
	DefaultHolster(SHOTGUN_HOLSTER, skiplocal, 0, (11.0f/30.0f));
}



#ifndef CLIENT_DLL
EASY_CVAR_EXTERN(soundSentenceSave)
#endif

BOOL CShotgun::Deploy( )
{
	this->m_fireState &= ~128;  //nope.

	m_flReleaseThrow = -1;

	//MODDD TODO: CHANGE SOUNDS!!!
	//if ( flRndSound <= 0.5 )


	//Just try to play at all only if using soundSentenceSave. This stupid meme isn't worth a "could not precache" notice or "missing file" problem.
#ifndef CLIENT_DLL
	if(EASY_CVAR_GET(soundSentenceSave) == 1){
		if(!globalflag_muteDeploySound){
			UTIL_PlaySound(ENT(m_pPlayer->pev), CHAN_WEAPON, "meme/i_got_a_shotgun.wav", 1, ATTN_NORM, 0, 100, TRUE);
		}
	}
#endif
	

	//MODDD - offermore specific deploy anim time after skipping other params.
	return DefaultDeploy( "models/v_shotgun.mdl", "models/p_shotgun.mdl", SHOTGUN_DRAW, "shotgun", 0, 0, 13.0/24.0 );
} 





void CShotgun::ItemPreFrame(){
	CBasePlayerWeapon::ItemPreFrame();

	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteclip) == 1){
		//cheating.
		m_chargeReady |= SHOTGUN_BIT1;
	}else{
		if(m_chargeReady & SHOTGUN_BIT1){
			m_chargeReady &= ~SHOTGUN_BIT1;
		}
	}

	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteammo) == 1){
		//cheating.
		m_chargeReady |= SHOTGUN_BIT2;
	}else{
		if(m_chargeReady & SHOTGUN_BIT2){
			m_chargeReady &= ~SHOTGUN_BIT2;
		}
	}

	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelay) == 1){
		//cheating.
		m_chargeReady |= SHOTGUN_BIT3;
	}else{
		if(m_chargeReady & SHOTGUN_BIT3){
			m_chargeReady &= ~SHOTGUN_BIT3;
		}
	}

}


void CShotgun::ItemPostFrame( void )
{
	
	if(this->m_flNextPrimaryAttack <= UTIL_WeaponTimeBase() ){
		//HACK - turn off reverse now.
		//this->m_fireState &= ~128;  //nope.
	}


	if ( m_flPumpTime && m_flPumpTime < gpGlobals->time )
	{
		// play pumping sound
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/scock1.wav", 1, ATTN_NORM, 0, 95 + RANDOM_LONG(0,0x1f));
		m_flPumpTime = 0;

		if(m_iClip == 1){
			m_fInAttack |= SHOTGUN_BIT5; //signify we've put one bullet in.
		}else if(m_iClip > 1){
			m_fInAttack |= SHOTGUN_BIT5; //signify we've put one bullet in.
			m_fInAttack |= SHOTGUN_BIT6; //two bullets availabe at the time of pump, two bullets ready for double-fire.
		}
	}



	const BOOL holdingPrimary = m_pPlayer->pev->button & IN_ATTACK;
	const BOOL holdingSecondary = m_pPlayer->pev->button & IN_ATTACK2;






	CBasePlayerWeapon::ItemPostFrame();

	if (((!holdingPrimary && !holdingSecondary) || m_fInSpecialReload > 0)) {

		//MODDD - moved from weaponIdle for more control.
		if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
		{
			if (reloadSemi()) {
				//if handled, nothing to do.
			}
			else
			{
				//MODDD - times changed for new (restored?) model.
				int iAnim;
				float flRand = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 0, 1);
				if (flRand <= 0.8)
				{
					iAnim = SHOTGUN_IDLE_DEEP;
					//was 60 / 20...
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (58.0 / 12.0) + randomIdleAnimationDelay();// * RANDOM_LONG(2, 5);
				}
				else if (flRand <= 0.95)
				{
					iAnim = SHOTGUN_IDLE;  //sm_idle
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (20.0 / 9.0) + randomIdleAnimationDelay();
				}
				else
				{
					iAnim = SHOTGUN_IDLE4;  //idle2
					//m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (60.0/20.0);
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (58.0 / 15.0) + randomIdleAnimationDelay();
				}


				SendWeaponAnimServerOnly(iAnim);
			}
		}
	}//END OF not holdin



	
	if(m_fInAttack & SHOTGUN_BIT7){
		if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(viewModelPrintouts)==1)easyForcePrintLine("I SAW THE WHOLE THING");
		m_fInAttack &= ~SHOTGUN_BIT7;
		//SendWeaponAnimServerOnlyReverse( SHOTGUN_START_RELOAD );
		//CBasePlayerWeapon::SendWeaponAnimServerOnlyReverse(SHOTGUN_START_RELOAD, 1, 0);

		SendWeaponAnimServerOnlyReverse( (int)SHOTGUN_START_RELOAD, 0 );

		//m_fireState |= 128;
	}
}



void CShotgun::ItemPostFrameThink(void) {

	//NOTICE - m_flReleaseThrow comes from loaded games as 0, check for '> 0' to avoid playing this on restored games with the shotgun out
	if (m_flReleaseThrow > 0 && gpGlobals->time > m_flReleaseThrow) {
		m_flReleaseThrow = -1;
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/scock1.wav", 1, ATTN_NORM, 0, 95 + RANDOM_LONG(0, 0x1f));
	}

	if (PlayerPrimaryAmmoCount() > 0 && m_iClip < SHOTGUN_MAX_CLIP) {
		// Close enough since wanting to?
		if (m_flNextPrimaryAttack - SHOTGUN_RELOAD_CUTIN_TIME <= UTIL_WeaponTimeBase()) {

			if (queueReload) {
				// yay!
			//	reloadLogic();
			//	queueReload = FALSE;
			}
		}
		else {

		}
	}//END OF checks



	CBasePlayerWeapon::ItemPostFrameThink();
}






/*
#ifdef CLIENT_DLL
int cl_totalFires = 0;
#else
int sv_totalFires = 0;
#endif
*/




void CShotgun::PrimaryAttack()
{
	FireShotgun(TRUE);
}




void CShotgun::SecondaryAttack( void )
{
	FireShotgun(FALSE);
}


void CShotgun::FireShotgun(BOOL isPrimary) {

	// don't fire underwater
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound();
		SetAttackDelays(UTIL_WeaponTimeBase() + 0.4);
		return;
	}


	if (reloadBlockFireCheck(isPrimary)) {
		queueReload = FALSE;  // clearly don't want to do it that way at least.  Might not be necessary to even do this here
		return;
	}


	BOOL enoughClip;
	if (isPrimary) {
		enoughClip = (m_iClip > 0);
	}
	else {
		enoughClip = (m_iClip > 1);
	}

	if (!enoughClip)
	{
		reloadLogic();
		if (m_iClip == 0) {
			PlayEmptySound();
		}
		return;
	}

	// why was SHOTGUN_BIT5 here ?!
	m_fInAttack &= ~(SHOTGUN_BIT6);

	/*
#ifdef CLIENT_DLL
	cl_totalFires++;
#else
	sv_totalFires++;
#endif
	*/

	//MODDD - little different behavior between primary/secondary.
	// And even louder (for alerting AI) with double
	if (isPrimary) {
		m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME + 200;
		m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;
	}
	else {
		m_pPlayer->m_iWeaponVolume = LOUDEST_GUN_VOLUME;  //louder
		m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH + 128;  //brighter
	}




	//MODDD - cheat check.
	if (!(m_chargeReady & SHOTGUN_BIT1)) {
		if (isPrimary) {
			m_iClip -= 1;
		}
		else {
			m_iClip -= 2;
		}
	}


	if (m_iClip == 0) {
		// next reload will need the pump.
		m_fInAttack &= ~(SHOTGUN_BIT5);
	}



	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif


	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

	//MODDD - did we really used to only do the 3rd person player model fire anim
	// for secondary fire?   Why???
	// player "shoot" animation
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);


	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecAiming = m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

	
	//MODDD - NOTE!  Don't bother recording vecDir from the shotgun.
	// FireBulletsPlayer only returns the direction of the most recently fired bullet, so multi-bullet firings like the shotgun just return the direction
	// of the last fired pellet.  Not very useful.
	// I think even Valve gave up on trying (or never did) to network shotgun blasts, tracers from serverside show they don't match up with the
	// client/ev_hldm-generated bullet decals.
	//Vector vecDir;

	if (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(playerWeaponSpreadMode) != 2 && (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(playerWeaponSpreadMode) == 1 || !IsMultiplayer()))
	{
		// (used t ostart with   vecDir = ...)
		// regular old, untouched spread for singleplayer
		if (isPrimary) {
			m_pPlayer->FireBulletsPlayer(6, vecSrc, vecAiming, VECTOR_CONE_10DEGREES, 2048, BULLET_PLAYER_BUCKSHOT, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed);
		}else {
			m_pPlayer->FireBulletsPlayer(12, vecSrc, vecAiming, VECTOR_CONE_10DEGREES, 2048, BULLET_PLAYER_BUCKSHOT, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed);
		}
	}
	else
	{
		// tuned for deathmatch
		if (isPrimary) {
			m_pPlayer->FireBulletsPlayer(4, vecSrc, vecAiming, VECTOR_CONE_DM_SHOTGUN, 2048, BULLET_PLAYER_BUCKSHOT, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed);
		}else {
			m_pPlayer->FireBulletsPlayer(8, vecSrc, vecAiming, VECTOR_CONE_DM_DOUBLESHOTGUN, 2048, BULLET_PLAYER_BUCKSHOT, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed);
		}
	}

	if (isPrimary) {
		if (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(viewModelPrintouts) == 1)easyForcePrintLine("SHOTGUN: PRIMARY FIRE AT %.2f", gpGlobals->time);
	}
	else {
		if (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(viewModelPrintouts) == 1)easyForcePrintLine("SHOTGUN: SECONDARY FIRE AT %.2f", gpGlobals->time);
	}


	// is this necessary here?
	m_fireState &= ~128;

	

	if (isPrimary) {
		//MODDD
		// used to send vecDir.x, vecDir.y for the 5th, 6th parameters from the end.  They're not useful for reasons described above,
		// and even ev_hldm.cpp's shotgun events ignore those parameters.  So that was just a lazy paste-over.
		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usSingleFire, 0.0, (float*)&g_vecZero, (float*)&g_vecZero, 0, 0, 0, 0, 0, 0);
		// this is single fire.
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (20.0 / 20.0) + randomIdleAnimationDelay();
	}
	else {
		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usDoubleFire, 0.0, (float*)&g_vecZero, (float*)&g_vecZero, 0, 0, 0, 0, 0, 0);
		//MODDD - secondary fire idle delay.
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (21.0 / 13.0) + randomIdleAnimationDelay();
	}


	if (!m_iClip && PlayerPrimaryAmmoCount() <= 0) {
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	}


	if (isPrimary) {
		//MODDD - WRONG.   if we were out of ammo, execution wouldn't have reached this point to begin with!
		// this just causes the last visible reload pump of the shotgun (before auto-reloading) to make no noise.
		//if (m_iClip != 0){
		m_flPumpTime = gpGlobals->time + 0.5;
		//}
	}
	else {

		//MODDD - WRONG.   if we were out of ammo, execution wouldn't have even reached this point to begin with!
		// this just causes the last visible reload pump of the shotgun (before auto-reloading) to make no noise.
		//if (m_iClip != 0){
		m_flPumpTime = gpGlobals->time + 0.85;

		if (m_iClip == 1) {
			// wait why do this??!
			//m_fInAttack |= SHOTGUN_BIT5; //signify we've put one bullet in.
		}
		else {
			m_fInAttack |= SHOTGUN_BIT6; //two bullets availabe at the time of pump, two bullets ready for double-fire.
		}
		//}

	}



	//MODDD
	if (!(m_chargeReady & SHOTGUN_BIT3)) {

		//MODDD - little tighter now, people want responsiveness.
		if (isPrimary) {
			//MODDD - now enough for the full anim, or almost. Real anim time is 1.00 seconds
			SetAttackDelays(UTIL_WeaponTimeBase() + 0.90 - 0.08);

			//SetAttackDelays(UTIL_WeaponTimeBase() + 0.75);
		}
		else {
			//MODDD - enough for full anim. Or almost. Real time is ~1.615 seconds.
			SetAttackDelays(UTIL_WeaponTimeBase() + 1.5 - 0.09);  //nah default is good here.

			//SetAttackDelays(UTIL_WeaponTimeBase() + 1.5);
		}

	}
	else {
		SetAttackDelays(UTIL_WeaponTimeBase() + EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelaycustom));
	}



	// is this cheat check necessary anymore?
	if (m_chargeReady & SHOTGUN_BIT2) {
		if (pszAmmo1() && PlayerPrimaryAmmoCount() < 8)
		{
			SetPlayerPrimaryAmmoCount(8);
		}
	}
	if (m_chargeReady & SHOTGUN_BIT1) {
		if (m_iClip <= 8)
		{
			m_iClip = 8;
		}
	}


	if (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(viewModelPrintouts) == 1)easyForcePrintLine("OK, next attack relative time:%.2f", m_flNextPrimaryAttack);

	/*
	if (m_iClip != 0)
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5.0;
	else
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.75;
	*/

	m_fInSpecialReload = 0;
}




void CShotgun::Reload( void ){
	//MODDD - majority of contents moved to 'reloadLogic', see that method.
	// This is now the only entry point into reload logic from outside the class for neatness.

	// MODDD - little more flexibility though for ease of use
	if (m_flNextPrimaryAttack - SHOTGUN_RELOAD_CUTIN_TIME > UTIL_WeaponTimeBase()) {

		// NOTICE!  Only count the intent if the player actually held down reload, this 'Reload' method can be called
		// by other shotgun-related logic too!  This check is good enough though.
		// And no need for that check, only entry point from input now.  Yay.
		//if (m_pPlayer->pev->button & IN_RELOAD) {
			// And let a reload start when possible then.
			queueReload = TRUE;
		//}
		return;
	}

	reloadLogic();
}//Reload



BOOL CShotgun::reloadBlockFireCheck(BOOL isPrimary){

	
	if(TRUE){
		if (m_fInSpecialReload > 0){

			// HOWEVER, one other check.  Don't count if the click comes while the shotgun isn't
			// even loading ammo yet (and empty clip).
			// NEVERMIND, better way below now (isPrimary alongside clip being at least 1).
			/*
			if (m_iClip == 0 && m_fInSpecialReload < 2) {
				// too early!
				//return 0;
			}
			else {
				// proceed
			}
			*/

			if (( (isPrimary && m_iClip > 0) || m_iClip >= 2)) {
				if (m_iClip != 0) {
					m_fInAttack |= SHOTGUN_BIT4;  //queue a pump next time.
				}
				else {
					m_fInAttack |= SHOTGUN_BIT8;  //queue a pump after reloading once
				}

				if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase()) {
					reloadSemi();
				}
				//this->m_fireState
			}

			return TRUE;
		}else if(m_fInSpecialReload == 1){
			//initial delay? can't interrupt. being > 0 already catches this though. oh well.
			return TRUE;
		}
	}else{
		//// NOPE NOPE NOPE
		//if (m_fInSpecialReload == 1){ //&& m_flTimeWeaponIdle > UTIL_WeaponTimeBase() ){
		//	//initial reload delay? block it.
		//	return TRUE;
		//}
	}

	return FALSE; //not stopped earlier? Not blocked.

}


void CShotgun::reloadFinishPump(){
	
	// no need to do this again. Flag is only set if ending early.
	m_fInAttack &= (~SHOTGUN_BIT4);

	// reload debounce has timed out
	//MODDD - ALSO, BYPASS NOW

	BOOL usePumpAnim = FALSE;


	if (m_fInAttack & (SHOTGUN_BIT5)) {
		// no pump
	}
	else {
		//pump
		usePumpAnim = TRUE;
	}

	/*
	if( (m_fInAttack & (SHOTGUN_BIT5 | SHOTGUN_BIT6 )) == (SHOTGUN_BIT5 | SHOTGUN_BIT6) ){
		// Both pump bits? No possible need for a pump.

	}else if( m_fInAttack & (SHOTGUN_BIT5 ) ){
		// only for 1-bullet? See if we have more than 1 bullet to make a 2nd pump worthwhile (enable doublefire on a whim).
		if(m_iClip > 1){
			usePumpAnim = TRUE;
		}
	}else if( m_fInAttack & (SHOTGUN_BIT6) ){
		// only for 2-bullets? inclusive of the first, deny.

	}else{ //neither.
		// If we have neither pump bit, we certainly need to pump.
		usePumpAnim = TRUE;
	}
	*/
				

	if(usePumpAnim == TRUE){
		SendWeaponAnimServerOnly( SHOTGUN_PUMP );

		//MODDD - no delay needed for this? neat.      .......  oh.
		// play cocking sound, later.
		m_flReleaseThrow = gpGlobals->time + 0.32;

	
		if(m_iClip == 1){
			m_fInAttack |= SHOTGUN_BIT5; //signify we've put one bullet in.
		}else if(m_iClip > 1){
			m_fInAttack |= SHOTGUN_BIT5; //signify we've put one bullet in.
			m_fInAttack |= SHOTGUN_BIT6; //two bullets availabe at the time of pump, two bullets ready for double-fire.
		}


		if (m_fInSpecialReload == 3) {
			// Add them to the clip
			m_iClip += 1;
			ChangePlayerPrimaryAmmoCount(-1);
		}

		m_fInSpecialReload = 0;
				
		//m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (12.0 / 12.0) + randomIdleAnimationDelay();


		if(TRUE){
			//m_flPumpTime = gpGlobals->time + 0.5;

			//MODDD - reduced the fire-delay a bit, was 0.85
			SetAttackDelays(UTIL_WeaponTimeBase() + 0.65);
		}

	}else{
		//this->m_fireState |= 128;  //this means, play backwards.  Method call does this now.
		//SendWeaponAnimServerOnlyReverse( SHOTGUN_START_RELOAD );
		

		if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(viewModelPrintouts)==1)easyForcePrintLine("ILL hahaha");
		m_fInAttack |= SHOTGUN_BIT7;


		if (m_fInSpecialReload == 3) {
			// Add them to the clip
			m_iClip += 1;
			ChangePlayerPrimaryAmmoCount(-1);
		}

		m_fInSpecialReload = 0;
				
		//m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (11.0 / 13.0) + randomIdleAnimationDelay();


		if(TRUE){
			//m_flPumpTime = gpGlobals->time + 0.5;

			//MODDD - reduced the fire-delay a bit, was 0.75
			SetAttackDelays(UTIL_WeaponTimeBase() + 0.44);

			//MODDD NOTE - shouldn't primary and secondary be affected the same??
			// Why yes, yes they sohuld.

		}
	}
}



BOOL CShotgun::reloadSemi(){
	
	if (m_iClip == 0 && m_fInSpecialReload == 0 && PlayerPrimaryAmmoCount() > 0)
	{
		reloadLogic( );
	}
	else if (m_fInSpecialReload != 0)
	{
		if (m_iClip != 8 && PlayerPrimaryAmmoCount() > 0)
		{
			/*
			makeNoise = TRUE;
			if (m_iClip > 0 && (m_fInAttack & SHOTGUN_BIT4)) {
				makeNoise = FALSE;
			}
			*/

			if (m_iClip > 0 && (m_fInAttack & SHOTGUN_BIT4)) {
				// force the pump!
				reloadFinishPump();
			}

			reloadLogic( );
		}
		else
		{
			reloadFinishPump();
		}
	}else{
		return FALSE;
	}

	return TRUE;
}


//MODDD - has most of what used to be the 'Reload' method.
// Done so calls from outside the class can be separated (anywhere in-class that used to call Reload now calls 'reloadLogic' instead)
void CShotgun::reloadLogic(void) {


	if (PlayerPrimaryAmmoCount() <= 0 || m_iClip == SHOTGUN_MAX_CLIP)
		return;

	// don't reload until recoil is done
	// MODDD - little more flexibility though for ease of use
	if (m_flNextPrimaryAttack - SHOTGUN_RELOAD_CUTIN_TIME > UTIL_WeaponTimeBase()) {
		return;
	}

	// check to see if we're ready to reload
	if (m_fInSpecialReload == 0)
	{
		//MODDD - important!  was just  "SendWeaponAnimServerOnly".
		SendWeaponAnimServerOnly(SHOTGUN_START_RELOAD);
		//
		m_fInSpecialReload = 1;
		//m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.6;

		//MODDD - cut in a little more, was 0.75
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.58;


		//SetAttackDelays(UTIL_WeaponTimeBase() + 0.6); //why 1?

		return;
	}
	else if (m_fInSpecialReload == 1)
	{
		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		m_fInSpecialReload = 2;


	}
	else if (m_fInSpecialReload == 2)
	{
		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		//if (makeNoise == FALSE)return;

		// was waiting for gun to move to side
		m_fInSpecialReload = 3;

		if (RANDOM_LONG(0, 1))
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/reload1.wav", 1, ATTN_NORM, 0, 85 + RANDOM_LONG(0, 0x1f));
		else
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/reload3.wav", 1, ATTN_NORM, 0, 85 + RANDOM_LONG(0, 0x1f));

		// still server only?   maybe.  Is a singleplayer mod anyway.
		SendWeaponAnimServerOnly(SHOTGUN_RELOAD);


		// drop #8, pick up #4.  Queue a pump after this shell is in.
		if (m_fInAttack & SHOTGUN_BIT8) {
			m_fInAttack &= ~SHOTGUN_BIT8;
			m_fInAttack |= SHOTGUN_BIT4;
		}

		//MODDD - beware.  Altering the "m_flTimeWeaponIdle" to be different will affect "m_flNextReload" as well, since
		//        "m_flNextReload" is only checked if "m_flTimeWeaponIdle" is not applied (not waiting for the delay to pass).
		//m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (10.0 / 16.0);

		//MODDD - tightened as well.  These are delays between bullets being put into the shotgun
		m_flNextReload = UTIL_WeaponTimeBase() + 0.5 - 0.025;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5 - 0.025;

	}
	else   //m_fInSpecialReload == 3
	{
		//if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		//	return;
		// Add them to the clip
		m_iClip += 1;
		ChangePlayerPrimaryAmmoCount(-1);
		m_fInSpecialReload = 2;
	}

	queueReload = FALSE;  // clearly did it.

}//reloadLogic






void CShotgun::WeaponIdle( void )
{
	ResetEmptySound( );

	m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );



	//MODDD - now plays outside the idle ever so slightly so that holding down doesn't hide this sound.
	/*
	if ( m_flPumpTime && m_flPumpTime < gpGlobals->time )
	{
		// play pumping sound
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/scock1.wav", 1, ATTN_NORM, 0, 95 + RANDOM_LONG(0,0x1f));
		m_flPumpTime = 0;
	}
	*/


}



class CShotgunAmmo : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/w_shotbox.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/w_shotbox.mdl");
		precacheAmmoPickupSound();
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		if (pOther->GiveAmmo( AMMO_BUCKSHOTBOX_GIVE, "buckshot", BUCKSHOT_MAX_CARRY ) != -1)
		{
			playAmmoPickupSound();

			//MODDD
			if(pOther->IsPlayer()){
				CBasePlayer* pPlayer = (CBasePlayer*)pOther;
				pPlayer->SetSuitUpdate("!HEV_BUCKSHOT", FALSE, SUIT_NEXT_IN_20MIN);
			}

			return TRUE;
		}
		return FALSE;
	}
};
LINK_ENTITY_TO_CLASS( ammo_buckshot, CShotgunAmmo );


