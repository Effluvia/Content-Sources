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
#include "mp5.h"
#include "util.h"
#include "cbase.h"
#include "basemonster.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "soundent.h"
#include "gamerules.h"


LINK_ENTITY_TO_CLASS( weapon_mp5, CMP5 );
LINK_ENTITY_TO_CLASS( weapon_9mmAR, CMP5 );



EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelay)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelaycustom)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteclip)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteammo)

EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST(playerWeaponSpreadMode)

// this CVar is serverside only. The client should never use it.
EASY_CVAR_EXTERN_DEBUGONLY(mp5GrenadeInheritsPlayerVelocity)



//MODDD
void CMP5::customAttachToPlayer(CBasePlayer* pPlayer){
	m_pPlayer->SetSuitUpdate("!HEV_ASSAULT", FALSE, SUIT_NEXT_IN_30MIN, 6.9f);
}

void CMP5::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_9mmAR"); // hack to allow for old names
	Precache( );
	SET_MODEL(ENT(pev), "models/w_9mmAR.mdl");
	m_iId = WEAPON_MP5;

	m_iDefaultAmmo = MP5_DEFAULT_GIVE;

	FallInit();// get ready to fall down.
}


void CMP5::Precache( void )
{
	PRECACHE_MODEL("models/v_9mmAR.mdl");
	PRECACHE_MODEL("models/w_9mmAR.mdl");
	PRECACHE_MODEL("models/p_9mmAR.mdl");

	m_iShell = PRECACHE_MODEL ("models/shell.mdl");// brass shellTE_MODEL

	PRECACHE_MODEL("models/grenade.mdl");	// grenade

	PRECACHE_MODEL("models/w_9mmARclip.mdl");


	PRECACHE_SOUND("items/9mmclip1.wav");

	PRECACHE_SOUND("items/clipinsert1.wav");
	PRECACHE_SOUND("items/cliprelease1.wav");

	PRECACHE_SOUND ("weapons/hks1.wav");// H to the K
	PRECACHE_SOUND ("weapons/hks2.wav");// H to the K
	PRECACHE_SOUND ("weapons/hks3.wav");// H to the K

	PRECACHE_SOUND( "weapons/glauncher.wav" );
	PRECACHE_SOUND( "weapons/glauncher2.wav" );

	PRECACHE_SOUND ("weapons/357_cock1.wav");

	precacheGunPickupSound();

	m_usMP5 = PRECACHE_EVENT( 1, "events/mp5.sc" );
	m_usMP52 = PRECACHE_EVENT( 1, "events/mp52.sc" );
}

int CMP5::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "9mm";
	p->iMaxAmmo1 = _9MM_MAX_CARRY;
	p->pszAmmo2 = "ARgrenades";
	p->iMaxAmmo2 = M203_GRENADE_MAX_CARRY;
	p->iMaxClip = MP5_MAX_CLIP;
	p->iSlot = 2;
	p->iPosition = 0;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_MP5;
	p->iWeight = MP5_WEIGHT;

	return 1;
}

int CMP5::AddToPlayer( CBasePlayer *pPlayer )
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

BOOL CMP5::Deploy( )
{
	return DefaultDeploy( "models/v_9mmAR.mdl", "models/p_9mmAR.mdl", MP5_DEPLOY, "mp5", 0, 0, (12.0 / 12.0), -1 );
}

void CMP5::Holster( int skiplocal /* = 0 */ ){
	DefaultHolster(MP5_HOLSTER, skiplocal, 0, (12.0f/12.0f));
}



void CMP5::PrimaryAttack()
{


	/*
	if(m_iClip > 2){
		m_iClip = 2; //DEBUG. run out fast.
	}

	easyForcePrintLine("MP5 CALLED??! %.2f", gpGlobals->time);
	*/
	//MODDD TODO - Holding down doesn't re-click every say, 0.15 seconds? No idea, check back later. or check weapons.cpp, do printouts, etc.mysterious, check back later.

	// don't fire underwater
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound( );
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15;
		return;
	}

	if (m_iClip <= 0)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15;
		return;
	}

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;


	//MODDD
	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteclip) == 0){
		m_iClip --;
	}


	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

	// player "shoot" animation
	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

	Vector vecSrc	 = m_pPlayer->GetGunPosition( );
	Vector vecAiming = m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );
	Vector vecDir;

	//MODDD NOTE - for whatever reason the outcomes were swapped. That is, the condition was "NOT in multiplayer", yet passing picked the "optimized muliplayer" one and vice versa.
	//Changed.
	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(playerWeaponSpreadMode)!=2 && (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(playerWeaponSpreadMode)==1 || !IsMultiplayer()) )
	{
		// single player spread
		vecDir = m_pPlayer->FireBulletsPlayer( 1, vecSrc, vecAiming, VECTOR_CONE_3DEGREES, 8192, BULLET_PLAYER_MP5, 2, 0, m_pPlayer->pev, m_pPlayer->random_seed );
	}
	else
	{
		// optimized multiplayer. Widened to make it easier to hit a moving player
		vecDir = m_pPlayer->FireBulletsPlayer( 1, vecSrc, vecAiming, VECTOR_CONE_6DEGREES, 8192, BULLET_PLAYER_MP5, 2, 0, m_pPlayer->pev, m_pPlayer->random_seed );
	}

  int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif


	//MODDD - pre-determine the 
	int fireAnimChoice = UTIL_SharedRandomLong(m_pPlayer->random_seed, 0, 2);


	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usMP5, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, fireAnimChoice, 0, 0, 0 );



	//MODDD - how long is the picked anim?
	//uhhhh, oops.  They all take 7/10'ths of a second.  Well, that was needless.
	switch(fireAnimChoice){
	case 0:
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (7.0/10.0) + randomIdleAnimationDelay();
	break;
	case 1:
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (7.0/10.0) + randomIdleAnimationDelay();
	break;
	case 2:
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (7.0/10.0) + randomIdleAnimationDelay();
	break;
	}


	
	if (!m_iClip && PlayerPrimaryAmmoCount() <= 0) {
		// HEV suit - indicate out of ammo condition
		// NOTE that this noticing the player is out of ammo after firing, being out of ammo at the moment
		// without ammo ends far earlier and plays the 'click' noise.
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	}


	//MODDD -Uhhhhhh...  I don't know what the 'if primAttack < weaponTimeBase' thing is, looks like something
	// similar to what was in the hornetgun.
	// Although notice a crucial difference: this starts with NextPrimaryAttack set to UTIL_WeaponTimeBase() + a constant.
	// hornet did NextPrimaryAttack set to NextPrimaryAttack + a constant.
	// In the mp5's case here, that condition could never even work.  But it could for the hornetgun (and only caused issues).
	// so BYE.
	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelay) == 0){
		/*
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1;
		if (m_flNextPrimaryAttack < UTIL_WeaponTimeBase()) {
		    m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1;
		}
		*/

		// ...also, set secondaryAttack delay too
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.4;
	}else{
		SetAttackDelays(UTIL_WeaponTimeBase() + EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelaycustom));
	}


	//m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
}



void CMP5::SecondaryAttack( void )
{
	// don't fire underwater
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound( );
		m_flNextPrimaryAttack = 0.15;
		return;
	}

	if (PlayerSecondaryAmmoCount() == 0)
	{
		PlayEmptySound( );
		return;
	}

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	m_pPlayer->m_iExtraSoundTypes = bits_SOUND_DANGER;
	m_pPlayer->m_flStopExtraSoundTime = UTIL_WeaponTimeBase() + 0.2;
		
	//MODDD - cheat check.
	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteammo) == 0 && EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteclip) == 0){
		ChangePlayerSecondaryAmmoCount(-1);
	}

	// player "shoot" animation
	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

 	UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );


	//MODDD - if(m_pPlayer->playerProjectilesAffectedByVelocity... TODO???
	// we don't add in player velocity anymore.

	//if( == 1){



	//Script that generates an entity should be serverside only.
#ifndef CLIENT_DLL
	Vector extraStart = UTIL_GetProjectileVelocityExtra(m_pPlayer->pev->velocity, EASY_CVAR_GET_DEBUGONLY(mp5GrenadeInheritsPlayerVelocity) );

	//MODDD - supply the damage to deal in this call.
	CGrenade::ShootContact( m_pPlayer->pev, 
							m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_forward * 16, 
							gpGlobals->v_forward * 800 + extraStart,
							gSkillData.plrDmgM203Grenade);
	
#endif
	
	
	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT( flags, m_pPlayer->edict(), m_usMP52 );
	
	//MODDD - m_flTimeWeaponIdle
	//only one anim:
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (34.0 / 30.0) + randomIdleAnimationDelay();

	//MODDD - slightly more delay between mp5 grenade firing
	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelay) == 0){
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.9;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1 + 0.18;
	}else{
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelaycustom);
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelaycustom);
	}


	//m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5;// idle pretty soon after shooting.

	if (PlayerSecondaryAmmoCount() <= 0) {
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	}
}

void CMP5::Reload( void )
{
	if ( m_pPlayer->ammo_9mm <= 0 )
		return;

	//NEW DELAY, old was 1.5.
	DefaultReload( MP5_MAX_CLIP, MP5_RELOAD, (47.0/20.0) );
}


void CMP5::WeaponIdle( void )
{
	ResetEmptySound( );

	m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	int iAnim;
	//MODDD NOTE - "m_flTimeWeaponIdle" changes per "iAnim" choice are additions.
	switch ( RANDOM_LONG( 0, 1 ) )
	{
	case 0:	
		iAnim = MP5_LONGIDLE;
		m_flTimeWeaponIdle = (40.0/8.0) + randomIdleAnimationDelay();
		break;
	
	default:
	case 1:
		iAnim = MP5_IDLE1;
		m_flTimeWeaponIdle = (110.0/35.0) + randomIdleAnimationDelay();
		break;
	}

	SendWeaponAnim( iAnim );

}



class CMP5AmmoClip : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/w_9mmARclip.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/w_9mmARclip.mdl");
		precacheAmmoPickupSound();
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		int bResult = (pOther->GiveAmmo( AMMO_MP5CLIP_GIVE, "9mm", _9MM_MAX_CARRY) != -1);
		if (bResult)
		{
			
			//MODDD - filtered.
			playAmmoPickupSound();

			//MODDD
			if(pOther->IsPlayer()){
				CBasePlayer* pPlayer = (CBasePlayer*)pOther;
				pPlayer->SetSuitUpdate("!HEV_9MM", FALSE, SUIT_NEXT_IN_20MIN);
			}

		}
		return bResult;
	}
};
LINK_ENTITY_TO_CLASS( ammo_mp5clip, CMP5AmmoClip );
LINK_ENTITY_TO_CLASS( ammo_9mmAR, CMP5AmmoClip );



class CMP5Chainammo : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/w_chainammo.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/w_chainammo.mdl");
		precacheAmmoPickupSound();
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		int bResult = (pOther->GiveAmmo( AMMO_CHAINBOX_GIVE, "9mm", _9MM_MAX_CARRY) != -1);
		if (bResult)
		{
			playAmmoPickupSound();

			//MODDD
			if(pOther->IsPlayer()){
				CBasePlayer* pPlayer = (CBasePlayer*)pOther;
				pPlayer->SetSuitUpdate("!HEV_9MM", FALSE, SUIT_NEXT_IN_20MIN);
			}

		}
		return bResult;
	}
};
LINK_ENTITY_TO_CLASS( ammo_9mmbox, CMP5Chainammo );


class CMP5AmmoGrenade : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/w_ARgrenade.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/w_ARgrenade.mdl");
		precacheAmmoPickupSound();
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		int bResult = (pOther->GiveAmmo( AMMO_M203BOX_GIVE, "ARgrenades", M203_GRENADE_MAX_CARRY ) != -1);

		if (bResult)
		{
			playAmmoPickupSound();

			//MODDD
			if(pOther->IsPlayer()){
				CBasePlayer* pPlayer = (CBasePlayer*)pOther;
				pPlayer->SetSuitUpdate("!HEV_AGRENADE", FALSE, SUIT_NEXT_IN_20MIN);
			}
		}
		return bResult;
	}
};
LINK_ENTITY_TO_CLASS( ammo_mp5grenades, CMP5AmmoGrenade );
LINK_ENTITY_TO_CLASS( ammo_ARgrenades, CMP5AmmoGrenade );



