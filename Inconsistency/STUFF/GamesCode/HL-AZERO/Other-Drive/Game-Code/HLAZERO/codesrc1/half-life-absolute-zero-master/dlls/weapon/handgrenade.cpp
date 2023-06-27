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
#include "handgrenade.h"
#include "util.h"
#include "cbase.h"
#include "basemonster.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"

//MODDD - extern
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelay)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelaycustom)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteclip)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteammo)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(handGrenadePickupYieldsOne);

LINK_ENTITY_TO_CLASS( weapon_handgrenade, CHandGrenade );


//NOTICE: this file does not include the projectile.  See "ShootTimed" above called by "PrimaryAttack".
//        Looks like the projectile is still a generic GGrenade object with a hand grenade model just like MP5
//        grenades.

CHandGrenade::CHandGrenade(void){
	//NEW VAR.  If -500, we are cheating.  Better for syncing.
	m_fireState = 0;

	// Used to be a var called "replayDeploy".  If on, at the next idle-call, it will first play the deploy (AKA draw) anim first to bring the grenade into view.
	//NOTE: is a bitmask.  
	// 1 (2 to the 0th) is for "replayDeploy", a watered-down version.
	// 2 (2 to the 1st) is for "weaponRetired", which re-does the "DefaultDeploy" on call.
	m_fInAttack = 0;
}


#ifndef CLIENT_DLL
TYPEDESCRIPTION	CHandGrenade::m_SaveData[] =
{
	DEFINE_FIELD( CHandGrenade, m_chargeReady, FIELD_INTEGER ),
	DEFINE_FIELD( CHandGrenade, m_fInAttack, FIELD_INTEGER ),
};
IMPLEMENT_SAVERESTORE(CHandGrenade, CBasePlayerWeapon);
#endif




//





//MODDD
void CHandGrenade::customAttachToPlayer(CBasePlayer *pPlayer ){
	m_pPlayer->SetSuitUpdate("!HEV_GRENADE", FALSE, SUIT_NEXT_IN_30MIN);
}

void CHandGrenade::Spawn( )
{
	Precache( );
	m_iId = WEAPON_HANDGRENADE;
	SET_MODEL(ENT(pev), "models/w_grenade.mdl");


	//MODDD - Aha!  I spot a mistake.
	// 'pev->dmg' is never effective for the equippable player weapons themselves, only the spawned
	// projectiles. CHandGrenade is the weapon, not the projectile.
	// There is no custom Hand Grenade projectile, we only use a generic CGrenade.
	/*
#ifndef CLIENT_DLL
	pev->dmg = gSkillData.plrDmgHandGrenade;
#endif
	*/
	


	m_iClip = -1;

	//if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(handGrenadePickupYieldsOne) == 1)
	// Make "m_iDefaultAmmo" non-zero so that it works like other pickups.  Ones with a defaultAmmo of 0
	// go a different route (ExtractClipAmmo).
	// Handle what to give in the grenade's own AddPrimaryAmmo (CVar makes it 1 or retail default, 5).
	//m_iDefaultAmmo = HANDGRENADE_DEFAULT_GIVE;
	m_iDefaultAmmo = 1;
	
	FallInit();// get ready to fall down.
}

// overridden.
BOOL CHandGrenade::ExtractAmmo( CBasePlayerWeapon *pWeapon ){
	// Nevermind, not sure what I was thinking.
	// If 'm_iDefaultAmmo' is 0, just detect that in AddPrimaryAmmo as it's called and don't do anything.
	return CBasePlayerWeapon::ExtractAmmo(pWeapon);
}




BOOL CHandGrenade::AddPrimaryAmmo(int iCount, char* szName, int iMaxClip, int iMaxCarry) {
	return CHandGrenade::AddPrimaryAmmo(iCount, szName, iMaxClip, iMaxCarry, 0);
}

BOOL CHandGrenade::AddPrimaryAmmo( int iCount, char *szName, int iMaxClip, int iMaxCarry, int forcePickupSound)
{
	// No behavior if m_iDefaultAmmo is 0, even calling the parent method.
	// I forget what the point of this check even was, but something like this used to happen here.  (My script, not as-is).
	// WWWWWRRRRRRRROOOOOOONNNNNNNNNNNGGGGG.  Bad.
	// Checking iCount is plenty.  Only force amount given on brand new pickups, not existing drops.
	// Forcing a count coming as 0 just adds more grenades than there actually were in the dropped weaponbox
	// (11 or 15 grenades from a weaponbox that had 10)
	if (iCount > 0) {
		//This is the real intervention.  The grenade gives only "one" grenade if the CVar is being used ( = 1), and 5 otherwise (retail value).
		if (m_pPlayer && EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(handGrenadePickupYieldsOne) == 1) {
			iCount = 1;
		}
		else {
			iCount = HANDGRENADE_DEFAULT_GIVE;
		}
		// and default behavior
		return CBasePlayerWeapon::AddPrimaryAmmo(iCount, szName, iMaxClip, iMaxCarry, forcePickupSound);
	}
	else {
		// finish?... don't tamper with anything, just go straight to default and elegantly add nothing I guess
		//m_iDefaultAmmo = 0;
		//return FALSE;

		return CBasePlayerWeapon::AddPrimaryAmmo(iCount, szName, iMaxClip, iMaxCarry, forcePickupSound);
	}
}



void CHandGrenade::Precache( void )
{
	PRECACHE_MODEL("models/w_grenade.mdl");
	PRECACHE_MODEL("models/v_grenade.mdl");
	PRECACHE_MODEL("models/p_grenade.mdl");

	precacheGunPickupSound();
	precacheAmmoPickupSound(); //since this same entity doubles as ammo.

}

int CHandGrenade::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "Hand Grenade";
	p->iMaxAmmo1 = HANDGRENADE_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 4;
	p->iPosition = 0;
	p->iId = m_iId = WEAPON_HANDGRENADE;
	p->iWeight = HANDGRENADE_WEIGHT;
	p->iFlags = ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE;

	return 1;
}


BOOL CHandGrenade::Deploy( )
{
	m_flReleaseThrow = -1;
	return DefaultDeploy( "models/v_grenade.mdl", "models/p_grenade.mdl", HANDGRENADE_DRAW, "crowbar" );
}

BOOL CHandGrenade::CanHolster( void )
{
	// can only holster hand grenades when not primed!
	return ( m_flStartThrow == 0 );
}

void CHandGrenade::ItemPreFrame(void){
	CBasePlayerWeapon::ItemPreFrame();

	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelay) == 1){
		//cheating.
		m_fireState = -500;
	}else{
		if(m_fireState == -500){
			m_fireState = 0;
		}
	}
}


void CHandGrenade::ItemPostFrame(void) {



	CBasePlayerWeapon::ItemPostFrame();
}

void CHandGrenade::ItemPostFrameThink(void) {



	CBasePlayerWeapon::ItemPostFrameThink();
}






void CHandGrenade::Holster( int skiplocal /* = 0 */ )
{
	//m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

	m_fInAttack = 0; //remove both bits.

	//MODDD NEW - safe?
	m_flStartThrow = 0;

	if (PlayerPrimaryAmmoCount() > 0)
	{
		//SendWeaponAnim( HANDGRENADE_HOLSTER );
		DefaultHolster(HANDGRENADE_HOLSTER, skiplocal, 0, (16.0f/30.0f));
	}
	else
	{
		// no more grenades!
		m_pPlayer->pev->weapons &= ~(1<<WEAPON_HANDGRENADE);
		SetThink( &CBasePlayerItem::DestroyItem );
		pev->nextthink = gpGlobals->time + 0.1;
	}

	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "common/null.wav", 1.0, ATTN_NORM);
}//END OF Holster


void CHandGrenade::PrimaryAttack()
{
	EitherAttack();
}//END OF PrimaryAttack

void CHandGrenade::SecondaryAttack()
{
	EitherAttack();
}

// Regardless of what key is used, pull the pin all the same and start holding down.
// What is released influences the thrown grenade path (typical throw or toss)
void CHandGrenade::EitherAttack() {

	// wait.  No ammo check?      really?
	if (PlayerPrimaryAmmoCount() <= 0) {
		return;
	}



	m_fInAttack &= (~1);


	//if the player is not cheating:
	if (m_fireState != -500) {
		//MODDD
		// this method is called anytime primary fire is held down.
		// So check to see if the grenade should explode in the user's hands (held down too long)
		// ...nope!  Pineapple grenades don't explode in your hands.  But still do instantly on being
		// thrown after being held for too long. That wasn't an oversight this time, go figure.
		/*
		if (m_flStartThrow && gpGlobals->time >= m_flStartThrow + 3) {
			// Held down to long, explode at player origin!

#ifndef CLIENT_DLL
			//UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
			SimpleStaticExplode(m_pPlayer->GetGunPosition(), gSkillData.plrDmgHandGrenade, m_pPlayer);
#endif

			//m_fInAttack &= (~1);
			m_flReleaseThrow = 0;
			m_flStartThrow = 0;
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5;
			m_flTimeWeaponIdle = m_flNextSecondaryAttack = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5;// ensure that the animation can finish playing
			return;
		}
		*/

		//easyForcePrintLine("ARE YOU hello %.2f, %d", m_flStartThrow, PlayerPrimaryAmmoCount());

		//"!m_flStartThrow"? Just say ==0 for fuck's sake.
		if (m_flStartThrow == 0 && PlayerPrimaryAmmoCount() > 0)
		{
			m_flStartThrow = gpGlobals->time;
			m_flReleaseThrow = 0;

			SendWeaponAnim(HANDGRENADE_PINPULL);
			//MODDD NOTE: does this really warrant a " + randomIdleAnimationDelay()" ?
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5;
		}

		// The grenade-shoot will happen in WeaponIdle (detected release for a certain area to be reached there)

	}
	else {
		// CHEATING.

		SetAttackDelays(UTIL_WeaponTimeBase() + EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelaycustom));
		
		//m_flTimeWeaponIdle = m_flNextSecondaryAttack = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5;// ensure that the animation can finish playing

		float flVel = 220 + 6 * 140;
		Vector angThrow = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;
		if (angThrow.x < 0) {
			angThrow.x = -10 + angThrow.x * ((90 - 10) / 90.0);
		}else {
			angThrow.x = -10 + angThrow.x * ((90 + 10) / 90.0);
		}

		//MODDD NOOOOOOOOOOOOOOOOOOOOOOOO
		//if ( flVel > 500 )
		//	flVel = 500;

		UTIL_MakeVectors(angThrow);
		Vector vecSrc = m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_forward * 16;
		Vector vecThrow = gpGlobals->v_forward * flVel + m_pPlayer->pev->velocity;

		CGrenade::ShootTimed(m_pPlayer->pev, vecSrc, vecThrow, gSkillData.plrDmgHandGrenade, 0.3f);
	}//END OF cheat check

}//eitherAttack


void CHandGrenade::WeaponIdle( void )
{
	const BOOL holdingPrimary = m_pPlayer->pev->button & IN_ATTACK;
	const BOOL holdingSecondary = m_pPlayer->pev->button & IN_ATTACK2;


	if (holdingPrimary && holdingSecondary) {
		// don't allow holding both at the same time, ignore.
		return;
	}

	
	// does... does m_flReleaseThrow even have a point???

	if (m_flReleaseThrow == 0 && m_flStartThrow) {
		m_flReleaseThrow = UTIL_WeaponTimeBase();//gpGlobals->time;  //UTIL_WeaponTimeBase()????
	}

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase()) {
		return;
	}



	//MODDD - is this okay for grenades?
	if (m_pPlayer->pev->viewmodel == iStringNull) {
		if (PlayerPrimaryAmmoCount() > 0) {

			globalflag_muteDeploySound = TRUE;
			Deploy();
			globalflag_muteDeploySound = FALSE;

			return;
		}
	}




	// schedule to show the draw anim following throwing a grenade.
	if (PlayerPrimaryAmmoCount() > 0 ){
		if(m_fInAttack & 1){
			SendWeaponAnim( HANDGRENADE_DRAW );
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (16.0/30.0) + randomIdleAnimationDelay();
			m_fInAttack &= (~1);
			return;
		}
	}
	else {
		// No ammo?  No idletime.  Better response times for picking up ammo with the empty viewmodel on.
		// No need for that actually, so long as nothing else tries to set m_flTimeWeaponIdle past the
		// current time very far.
		//m_flTimeWeaponIdle = 0;
	}
	

	//if m_fireState == -500, we are cheating.  No release-fire needed; it is instant.
	if ( m_flStartThrow && m_fireState != -500)
	{

		float timeSinceThrow = gpGlobals->time - m_flStartThrow;

		float timeUntilBoom = 3 - timeSinceThrow;
		if (timeUntilBoom < 0) timeUntilBoom = 0;


		BOOL primaryThrow;

		if ((m_pPlayer->m_afButtonReleased & IN_ATTACK)) {
			primaryThrow = TRUE;
		}
		else if ((m_pPlayer->m_afButtonReleased & IN_ATTACK2)) {
			primaryThrow = FALSE;
		}
		else {
			// ???
			primaryThrow = TRUE;
		}



		if (primaryThrow) {
			
			Vector angThrow = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;

			if (angThrow.x < 0)
				angThrow.x = -10 + angThrow.x * ((90 - 10) / 90.0);
			else
				angThrow.x = -10 + angThrow.x * ((90 + 10) / 90.0);


			//MODDD - flVel now depends on the time held down.

			//short, medium, long?
			//NOTE: all anim times shortened (cut off from the end) due to there being time where the screen is just blank during the anim.
			if (timeSinceThrow < 1) {
				SendWeaponAnim(HANDGRENADE_THROW1);
				//m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (19.0/13.0);
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + ((5.0 + 3.25) / 13.0);
				m_fInAttack |= 1;
			}
			else if (timeSinceThrow < 2) {
				SendWeaponAnim(HANDGRENADE_THROW2);
				//m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (19.0/20.0);
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + ((7.0 + 5.0) / 20.0);
				m_fInAttack |= 1;
			}
			else { // timeSinceThrow < 3
				SendWeaponAnim(HANDGRENADE_THROW3);
				//m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (19.0/30.0);
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + ((13.0 + 6.5) / 30.0);
				m_fInAttack |= 1;
			}

			//float flVel = ( 90 - angThrow.x ) * 4;
			float flVel = 220 + timeSinceThrow * 140;

			easyPrintLine("HAND GRENADE VELOCITY + THROW ANG: %.3f, %.3f", flVel, ((float)((90 - angThrow.x) * 4)));

			UTIL_MakeVectors(angThrow);

			Vector vecSrc = m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_forward * 16;
			Vector vecThrow = gpGlobals->v_forward * flVel + m_pPlayer->pev->velocity;

			CGrenade::ShootTimed(m_pPlayer->pev, vecSrc, vecThrow, gSkillData.plrDmgHandGrenade, timeUntilBoom);
		}else {
			// Do the toss.
			// In short, no mod on the angle thrown (generally a little lower then),
			// and less impact from holding down on throw, slightly greater initial velocity.

			Vector angThrow = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;
			
			// no mod to angThrow.

			SendWeaponAnim(HANDGRENADE_THROW1);
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + ((5.0 + 3.25) / 13.0);
			m_fInAttack |= 1;

			float flVel = 240 + timeSinceThrow * 60;

			easyPrintLine("HAND GRENADE VELOCITY + THROW ANG: %.3f, %.3f", flVel, ((float)((90 - angThrow.x) * 4)));

			UTIL_MakeVectors(angThrow);

			Vector vecSrc = m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_forward * 16;
			Vector vecThrow = gpGlobals->v_forward * flVel + m_pPlayer->pev->velocity;

			CGrenade* grenRef = CGrenade::ShootTimed(m_pPlayer->pev, vecSrc, vecThrow, gSkillData.plrDmgHandGrenade, timeUntilBoom);
			//MODDD - rolled?  Randomize between sequeneces 1 and 2, use that instead.  That's the 'roll' sequences.
			// They're identical to each other though.  oh well.
			if (grenRef != NULL) {
				grenRef->pev->sequence = RANDOM_LONG(1, 2);
			}

		}//END OF primaryThrow check





		//MODDD - section removed.   Odd, while testing, I could not trigger "THROW3" at all.
		//time
		/*
		if ( flVel < 500 )
		{
			SendWeaponAnim( HANDGRENADE_THROW1 );
			easyPrintLine("HAND GRENADE THROW1");
		}
		else if ( flVel < 1000 )
		{
			SendWeaponAnim( HANDGRENADE_THROW2 );
			easyPrintLine("HAND GRENADE THROW2");
		}
		else
		{
			SendWeaponAnim( HANDGRENADE_THROW3 );
			easyPrintLine("HAND GRENADE THROW3");
		}
		*/

		/*
		//Randomize animation instead.
		//MODDD - new way of showing grenade toss animations: just randomize animations.
		//MODDD - section also removed.  Using hold time -> distance -> animation instead.
		float flRand = RANDOM_FLOAT(0,1);
		if(flRand <= 0.333){
			//EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/357_cock1.wav", 0.8, ATTN_NORM);
			SendWeaponAnim( HANDGRENADE_THROW1 );
		}else if(flRand <= 0.666){
			//EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/dbarrel1.wav", 0.8, ATTN_NORM);
			SendWeaponAnim( HANDGRENADE_THROW2 );
		}else{
			//EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/cbar_hit1.wav", 0.8, ATTN_NORM);
			SendWeaponAnim( HANDGRENADE_THROW3 );
		}
		*/

		// player "shoot" animation
		m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

		m_flReleaseThrow = 0;
		m_flStartThrow = 0;
		//m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5;
		SetAttackDelays(UTIL_WeaponTimeBase() + 0.8);// ensure that the animation can finish playing


		//MODDD - handle idle-delay above depending on the chosen anim's time.
		//m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5;

		//MODDD - cheat check.
		if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteclip) == 0 && EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteammo) == 0){
			ChangePlayerPrimaryAmmoCount(-1);
		}

		//MODDD - no point to this section anymore.  WeaponIdles and the NextAttacks are set above already.
		/*
		if ( PlayerPrimaryAmmoCount() <= 0 )
		{
			// just threw last grenade
			// set attack times in the future, and weapon idle in the future so we can see the whole throw
			// animation, weapon idle will automatically retire the weapon for us.
			m_flTimeWeaponIdle = m_flNextSecondaryAttack = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5;// ensure that the animation can finish playing
		}
		*/
		return;
	}

	//MODDD - never observed as m_flReleaseThrow never goes above 0?  Well, whoops.
	// SECTION REMOVED, see older versions for it.

	if (PlayerPrimaryAmmoCount() > 0)
	{
		int iAnim;
		float flRand = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 0, 1 );
		if (flRand <= 0.75)
		{
			iAnim = HANDGRENADE_IDLE;
			//m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );// how long till we do this again.
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (90.0 / 30.0) + randomIdleAnimationDelay();
		
		}
		else 
		{
			iAnim = HANDGRENADE_FIDGET;
			//m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 75.0 / 30.0;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (75.0 / 30.0) + randomIdleAnimationDelay();
		}
		
		SendWeaponAnim( iAnim );
	}
}



