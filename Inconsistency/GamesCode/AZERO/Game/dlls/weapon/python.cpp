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
#include "python.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"
#include "basemonster.h"
#include "player.h"
#include "gamerules.h"


EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelay)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelaycustom)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteclip)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteammo)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(revolverLaserScope)



// eh?
#ifdef CLIENT_DLL
extern vec3_t ev_punchangle;
#endif



LINK_ENTITY_TO_CLASS( weapon_python, CPython );
LINK_ENTITY_TO_CLASS( weapon_357, CPython );





CPython::CPython(){
	m_fInAttack = 0;

	//the laser spot isn't blocked (is blocked after firing).
	m_fireState = 1;

}






//MODDD - copied from Weapons.cpp.  Good for telling whether to draw the laser spot or not ("not" implies the user just fired the weapon
//and is aiming away from the crosshair onscreen).
BOOL CanAttackPython( float attack_time, float curtime, BOOL isPredicted )
{	
#if defined( CLIENT_WEAPONS )
	if ( !isPredicted )
#else
	if ( 1 )
#endif
	{
		return ( attack_time <= curtime ) ? TRUE : FALSE;
	}
	else
	{
		return ( attack_time <= 0.0 ) ? TRUE : FALSE;
	}
}






int CPython::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "357";
	p->iMaxAmmo1 = _357_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = PYTHON_MAX_CLIP;
	p->iFlags = 0;
	p->iSlot = 1;
	p->iPosition = 1;
	p->iId = m_iId = WEAPON_PYTHON;
	p->iWeight = PYTHON_WEIGHT;

	return 1;
}

int CPython::AddToPlayer( CBasePlayer *pPlayer )
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


//MODDD
void CPython::customAttachToPlayer(CBasePlayer *pPlayer ){
	m_pPlayer->SetSuitUpdate("!HEV_44PISTOL", FALSE, SUIT_NEXT_IN_30MIN);
}

void CPython::Spawn( )
{
	m_fSpotActive = FALSE;


	pev->classname = MAKE_STRING("weapon_357"); // hack to allow for old names
	Precache( );
	m_iId = WEAPON_PYTHON;
	SET_MODEL(ENT(pev), "models/w_357.mdl");

	m_iDefaultAmmo = PYTHON_DEFAULT_GIVE;

	FallInit();// get ready to fall down.
}


void CPython::Precache( void )
{
	PRECACHE_MODEL("models/v_357.mdl");
	PRECACHE_MODEL("models/w_357.mdl");
	PRECACHE_MODEL("models/p_357.mdl");

	PRECACHE_MODEL("models/w_357ammobox.mdl");

	//MODDD
	m_iShell = PRECACHE_MODEL ("models/shell.mdl");// brass shell



	PRECACHE_SOUND("items/9mmclip1.wav");              

	PRECACHE_SOUND ("weapons/357_reload1.wav");
	PRECACHE_SOUND ("weapons/357_cock1.wav");
	PRECACHE_SOUND ("weapons/357_shot1.wav");
	PRECACHE_SOUND ("weapons/357_shot2.wav");

	precacheGunPickupSound();

	m_usFirePython = PRECACHE_EVENT( 1, "events/python.sc" );
}

BOOL CPython::Deploy( )
{
	m_fSpotActive = FALSE;
	updateModel();

	//assume block is off.
	m_fireState = 1;

	//MODDD - replaced.
	/*
	if(IsMultiplayer())
	{
		// enable laser sight geometry.
		pev->body = 1;
	}
	else
	{
		pev->body = 0;
	}
	*/

	return DefaultDeploy( "models/v_357.mdl", "models/p_357.mdl", PYTHON_DRAW, "python", UseDecrement(), pev->body, (16.0/30.0) );
}


void CPython::Holster( int skiplocal /* = 0 */ )
{
	//MODDD - added
	m_fSpotActive = FALSE;

	//turn the block off (enable spot).
	m_fireState = 1;
	
	spotDeleteCheck();


	m_fInReload = FALSE;// cancel any reload in progress.

	if (m_pPlayer->pev->fov != 0)
	{
		m_fInZoom = FALSE;
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 0;  // 0 means reset to default fov
	}


	//m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.0;
	//m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
	//SendWeaponAnim( PYTHON_HOLSTER, 1, m_fInAttack );

	
	DefaultHolster(PYTHON_HOLSTER, skiplocal, m_fInAttack, (16.0f/30.0f));
}


void CPython::SecondaryAttack( void )
{

//MODDD - why was the python zoom even ever dependent on single/multiplayer to begin with?
// Really, just don't use the zoom if you don't want it.  Why...
/*
	if(!IsMultiplayer())
	{
		return;
	}
*/

	// Moved to ItemPostFrameThink

}




void CPython::PrimaryAttack()
{
	//force the spot inactive at fire.  Re-show it (if still zoomed-in) when the gun can fire again.
	m_fSpotActive = FALSE;
	spotDeleteCheck();

	//this is what blocks "m_fSpotActive" and checks for turning it again if still zoomed.
	m_fireState = 0;
	

	// don't fire underwater
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound( );
		//MODDD - why so often anyway?
		//m_flNextPrimaryAttack = 0.15;
		m_flNextPrimaryAttack = 0.6;
		return;
	}

	if (m_iClip <= 0)
	{
		if (!m_fFireOnEmpty) {
			Reload();
		}
		else
		{
			EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/357_cock1.wav", 0.8, ATTN_NORM);
			//m_flNextPrimaryAttack = 0.15;
			m_flNextPrimaryAttack = 0.6;
		}

		return;
	}

	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteclip) == 0){
		m_iClip --;
	}

	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

	// player "shoot" animation
	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );


	UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );

	Vector vecSrc	 = m_pPlayer->GetGunPosition( );
	Vector vecAiming = m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );



	/*
	if(m_pPlayer->pev->v_angle.x != 0 && m_pPlayer->pev->v_angle.y != 0) {
		easyPrintLine("VEC AIMING????!!!! %.2f %.2f %.2f", m_pPlayer->pev->v_angle.x, m_pPlayer->pev->v_angle.y, m_pPlayer->pev->v_angle.z);
		easyPrintLine("VEC PUNCH????!!!! %.2f %.2f %.2f", m_pPlayer->pev->punchangle.x, m_pPlayer->pev->punchangle.y, m_pPlayer->pev->punchangle.z);
	}
	*/

	Vector vecDir;
	vecDir = m_pPlayer->FireBulletsPlayer( 1, vecSrc, vecAiming, VECTOR_CONE_1DEGREES, 8192, BULLET_PLAYER_357, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );

    int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usFirePython, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, m_fInAttack, 0, 0, 0 );
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (31.0 / 30.0) + randomIdleAnimationDelay();

	if (!m_iClip && PlayerPrimaryAmmoCount() <= 0)
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

	//MODDD.
	// Wait, no UTIL_WeaponTimeBase? ...  ?        what?                     what?
	// also, fire-rate changed.  Was 0.75, upped to 0.9.
	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelay) == 0){
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.9;
	}else{
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelaycustom);
		//m_pPlayer->pev->punchangle = Vector(0, 0, 0);   not necessary, I think.
	}

	//m_flTimeWeaponIdle = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
}


void CPython::Reload( void )
{
	if ( m_pPlayer->ammo_357 <= 0 )
		return;

	if ( m_pPlayer->pev->fov != 0 )
	{
		m_fInZoom = FALSE;
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 0;  // 0 means reset to default fov
	}

	//MODDD - this var has been replaced with "m_fInAttack", an existing var that syncs better with the client.
	//int bUseScope = FALSE;

//MODDD - if the "revolverLaserScopeMem" var is on, always show the scope.
//...nah, just update in preframe.

//MODDD - suspend the pointer.
#ifndef CLIENT_DLL
	if ( m_pSpot && m_fSpotActive )
	{
		m_pSpot->Suspend( PYTHON_RELOAD + 0.1f );
	}
#endif
	//frames / FPS = time

	//1.5 = frames / 37
	//55.5 = frames


	//MODDD - added.

	//MODDD - use "m_fInAttack" instead.  Syncs better.
	//if (DefaultReload( 6, PYTHON_RELOAD, 2.0, bUseScope ))
	//MODDD - also, delay changed to the time of the animation instead.


	if (DefaultReload( 6, PYTHON_RELOAD, (110.0 / 37.0), m_fInAttack ))
	{
	#ifndef CLIENT_DLL
		//m_flSoundDelay = 1.5;
		//m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.5;
		m_flSoundDelay =  gpGlobals->time + 1.5;
		//easyPrintLine("DELAYYYYY %.2f::%.2f", m_flSoundDelay, UTIL_WeaponTimeBase());
	#endif
	}

}



void CPython::ItemPostFrameThink(){
	//MODDD - SecondaryAttack script moved here.
	if ((m_pPlayer->m_afButtonPressed & IN_ATTACK2)) {
		const int zoomedFOV = (int)roundf(getPlayerBaseFOV() * 0.4444f);

		if (m_pPlayer->pev->fov != 0)
		{
			if (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(revolverLaserScope) == 1) {
				m_fSpotActive = FALSE;
			}
			m_fInZoom = FALSE;
			m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 0;  // 0 means reset to default fov

		}
		//MODDD - used to be 40.
		else if (m_pPlayer->pev->fov != zoomedFOV)
		{
			//"m_fireState == 1" means the block isn't on.
			if (m_fireState == 1 && EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(revolverLaserScope) == 1) {
				m_fSpotActive = TRUE;
			}
			else {
				m_fSpotActive = FALSE;
			}
			m_fInZoom = TRUE;
			m_pPlayer->pev->fov = m_pPlayer->m_iFOV = zoomedFOV;


			//if the spot is on and the python can attack, force the idle anim
			if (m_fSpotActive && CanAttackPython(m_flNextPrimaryAttack, gpGlobals->time, UseDecrement())) {
				//m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + randomIdleAnimationDelay();
				//MOVED TO BELOW.  anytime on zoom, refresh the idle anim delay to be safe.
			}

			//scramble the idle, to stop an animation that may make the python appear not to be looking forward.
		}

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + randomIdleAnimationDelay();

		spotDeleteCheck();

		//MODDD
		//if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelay) == 0){
		// NOPE NOT EVEN YOU.
		//	m_flNextSecondaryAttack = 0.5;
		//}else{
		//	m_flNextSecondaryAttack = EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelaycustom);
		//}

	}//END OF IN_ATTACK2 check



	#ifndef CLIENT_DLL
	
	//MODDD - TODO.  wait, why is this done here in python.cpp, and serverside only?
	// Why not ev_hldm.cpp like other weapons (mp5) do?  Might be a reason.
	// ...I think I see, this is to drop the shells as tempents while retail
	// never did this, I think?  Verify (reload python, look down)
	if(m_flSoundDelay != 0 && m_flSoundDelay <= gpGlobals->time)
	{
		//easyPrintLine("EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE %.2f :::%.2f", m_flSoundDelay, gpGlobals->time);
		for (int i = 0; i < 6; i++)
		{
			EjectBrass ( m_pPlayer->pev->origin, 
						Vector( RANDOM_FLOAT( -10.0, 10.0 ), RANDOM_FLOAT( -10.0, 10.0 ), (float)0.0 ), 
						m_pPlayer->pev->angles.y, m_iShell, TE_BOUNCE_SHELL); 

		}

		EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/357_reload1.wav", RANDOM_FLOAT(0.8, 0.9), ATTN_NORM);
		
		m_flSoundDelay = 0;
	}
	#endif


	// PARANOIA:  doesn't happen anyway it seems but did for the RPG rocket.
	// If holstering definitely don't restore the lasersight.
	if (!m_pPlayer->m_bHolstering) {
		UpdateSpot();
	}

	CBasePlayerWeapon::ItemPostFrameThink();
}//END OF ItemPostFrameThink()



//MODDD - added
void CPython::ItemPostFrame(){

	//MODDD - TODO. would it be better to put frame script in ItemPostFrame  here instead?


	CBasePlayerWeapon::ItemPostFrame();
}




//MODDD - added.
// ALSO beware, ItemPreFrame is server-only.   ItemPostFrame is called by both client & server.
void CPython::ItemPreFrame(){


	updateModel();

	//easyPrintLine("PYTHON FIRESTATE %d \n", m_fireState );

	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(revolverLaserScope) == 0){
		if(m_fSpotActive){
			m_fSpotActive = FALSE;
			spotDeleteCheck();
		}
	}else{
		/*
		if(m_fSpotActive && ((m_pPlayer->pev->fov == 0) || m_fireState == 0) ){
			m_fSpotActive = FALSE;
			spotDeleteCheck();
		}else if(!m_fSpotActive && m_pPlayer->pev->fov != 0){
			m_fSpotActive = TRUE;
		}
		*/
		

		if(CanAttackPython( m_flNextPrimaryAttack, gpGlobals->time, UseDecrement() )){
			//block turned off.
			m_fireState = 1;
		}

		/*
		if( m_fireState == 0 && m_fSpotActive ) {
			m_fSpotActive = FALSE;
			spotDeleteCheck();
		}else 
		*/
		if(m_fireState == 1 && !m_fSpotActive && m_pPlayer->pev->fov != 0){
			//assume zoomed in, can have the laser on, but don't
			m_fSpotActive = TRUE;
		}

	}
	
	CBasePlayerWeapon::ItemPreFrame();
}


//MODDD - added.
void CPython::updateModel(){
	//MODDD - the python used to have two body values. Now the scope is always attached and part of the only default body value of 0. No need for this.
	//The revolverLaserScope CVar is still checked for enabling / disabling the laser pointer that shows up on zooming in.
	/*
	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(revolverLaserScope) == 0){
		// scope not allowed?  Retail behavior is still to be toggled on for
		// multiplayer only (maybe?).
		m_fInAttack = IsMultiplayer();    // 0 or 1.
	}else{
		m_fInAttack = 1;
	}
	*/

	m_fInAttack = 0;

	if(pev->body != m_fInAttack){
		// body choice, goes with presence of laser-sight, no/yes
		pev->body = m_fInAttack;
	}
}

//MODDD - added.
void CPython::spotDeleteCheck(){
#ifndef CLIENT_DLL
	if (m_pSpot && !m_fSpotActive){
		//remove the spot.
		m_pSpot->Killed( NULL, GIB_NEVER );
		m_pSpot = NULL;
	}

#endif
}


void CPython::WeaponIdle( void )
{
	ResetEmptySound( );

	m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

	// ALERT( at_console, "%.2f\n", gpGlobals->time - m_flSoundDelay );
	//if (m_flSoundDelay != 0 && m_flSoundDelay <= UTIL_WeaponTimeBase() )
	//NOTE: "m_flSoundDelay" bit moved over to postFrame.

	//frames / FPS = time

	//time = 1.5
	//FPS = 37

	//frames / 37 = 1.5

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;


	int iAnim;
	
	//MODDD - why was this a random between 10 and 15?  it will always be above 1 (not the intended range 
	//from the looks of the choices below).  The old source got this right.
	//float flRand = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
	float flRand = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 0, 1 );


	//MODDD - if not zoomed in, no effect on idle animations.  Do any as usual.
	if ( m_pPlayer->pev->fov == 0 )
	{
		if (flRand <= 0.5)
		{
			iAnim = PYTHON_IDLE1;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (70.0/30.0) + randomIdleAnimationDelay();
		}
		else if (flRand <= 0.7)
		{
			iAnim = PYTHON_IDLE2;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (70.0/30.0) + randomIdleAnimationDelay();
		}
		else if (flRand <= 0.9)
		{
			iAnim = PYTHON_IDLE3;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (88.0/30.0) + randomIdleAnimationDelay();
		}
		else
		{
			iAnim = PYTHON_FIDGET;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (170.0/30.0) + randomIdleAnimationDelay();
		}
	
	}else{

		//Narrow it down.  Only animations that don't show the python moving around too much allowed while zoomed.


		if (flRand <= 0.5)
		{
			iAnim = PYTHON_IDLE1;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (70.0/30.0) + randomIdleAnimationDelay();
		}else if (flRand <= 1)
		{
			iAnim = PYTHON_IDLE3;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (88.0/30.0) + randomIdleAnimationDelay();
		}

	}
	
	/*
	int bUseScope = IsMultiplayer();
	*/
	
	//SendWeaponAnim( iAnim, UseDecrement() ? 1 : 0, bUseScope );
	SendWeaponAnim( iAnim, UseDecrement() ? 1 : 0, m_fInAttack );
}



void CPython::UpdateSpot( void )
{
	//easyPrintLine("SAY WHAT %.2f, %.2f, %.2f", m_pPlayer->pev->punchangle.x, m_pPlayer->pev->punchangle.y, m_pPlayer->pev->punchangle.z);

#ifndef CLIENT_DLL
	if (m_fSpotActive)
	{
		if (!m_pSpot)
		{
			m_pSpot = CLaserSpot::CreateSpot();
		}

		//MODDD TODO - for whatever reason this earlier line that made the laser pointer include the punch angle was commented out.  Restored.
		//    ...nevermind.  Restore it IF the laser sight is visible during the delay before firing again, but maybe only if there's at
		//    least one bullet left before having to reload.
		//UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );
		UTIL_MakeVectors( m_pPlayer->pev->v_angle );

		//ev_punchangle

		Vector vecSrc = m_pPlayer->GetGunPosition( );;
		Vector vecAiming = gpGlobals->v_forward;

		TraceResult tr;
		UTIL_TraceLine ( vecSrc, vecSrc + vecAiming * 8192, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr );
		
		UTIL_SetOrigin( m_pSpot->pev, tr.vecEndPos );

		//MODDD - new
		if (UTIL_PointContents(m_pSpot->pev->origin) == CONTENTS_SKY) {
			// If we hit the sky, go invisible
			m_pSpot->pev->effects |= EF_NODRAW;
		}else{
			m_pSpot->pev->effects &= ~EF_NODRAW;
		}
	}
#endif

}







class CPythonAmmo : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/w_357ammobox.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/w_357ammobox.mdl");
		precacheAmmoPickupSound();
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		if (pOther->GiveAmmo( AMMO_357BOX_GIVE, "357", _357_MAX_CARRY ) != -1)
		{
			playAmmoPickupSound();

			//MODDD
			if(pOther->IsPlayer()){
				CBasePlayer* pPlayer = (CBasePlayer*)pOther;
				pPlayer->SetSuitUpdate("!HEV_44AMMO", FALSE, SUIT_NEXT_IN_20MIN);
			}

			return TRUE;
		}
		return FALSE;
	}
};
LINK_ENTITY_TO_CLASS( ammo_357, CPythonAmmo );

