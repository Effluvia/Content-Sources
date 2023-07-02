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
#include "crossbow.h"
#include "util.h"
#include "cbase.h"
#include "basemonster.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "gamerules.h"

#ifndef CLIENT_DLL
// the CCrossbowBolt is for serverside only!
#include "crossbowbolt.h"
#endif


EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelay)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelaycustom)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteclip)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteammo)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(playerCrossbowMode)
EASY_CVAR_EXTERN_DEBUGONLY(crossbowInheritsPlayerVelocity)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(crossbowReloadSoundDelay)



// delay between shots boosted, was 0.75
// (this is not reload time, delay between firing clip).
#define CROSSBOW_LOADBOLT_TIME 1.50



// MODDD - NOTICE: class CCrossbowBolt has been moved to its own file pair,
// crossbowbolt.h/.cpp.  As stated above, it is serverside only.

LINK_ENTITY_TO_CLASS( weapon_crossbow, CCrossbow );

//MODDD - new.
CCrossbow::CCrossbow(){

	crossbowReloadSoundTimer = -1;

}//END OF constructor




/*
#ifndef CLIENT_DLL
	TYPEDESCRIPTION	CCrossbow::m_SaveData[] =
	{
		DEFINE_FIELD(CCrossbow, testcrossbowvar, FIELD_INTEGER),
	};
	IMPLEMENT_SAVERESTORE(CCrossbow, CBasePlayerWeapon);
#endif
*/




void CCrossbow::Spawn( )
{
	Precache( );
	m_iId = WEAPON_CROSSBOW;
	SET_MODEL(ENT(pev), "models/w_crossbow.mdl");

	m_iDefaultAmmo = CROSSBOW_DEFAULT_GIVE;


	//this does not look to be working at all.
	//pev->scale = 0.01;


	FallInit();// get ready to fall down.
}

int CCrossbow::AddToPlayer( CBasePlayer *pPlayer )
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
void CCrossbow::customAttachToPlayer(CBasePlayer *pPlayer ){
	m_pPlayer->SetSuitUpdate("!HEV_XBOW", FALSE, SUIT_NEXT_IN_30MIN, 4.1f);
}

void CCrossbow::Precache( void )
{
	PRECACHE_MODEL("models/w_crossbow.mdl");
	PRECACHE_MODEL("models/v_crossbow.mdl");
	PRECACHE_MODEL("models/p_crossbow.mdl");

	PRECACHE_SOUND("weapons/xbow_fire1.wav");
	PRECACHE_SOUND("weapons/xbow_reload1.wav");

	precacheGunPickupSound();

	UTIL_PrecacheOther( "crossbow_bolt" );

	m_usCrossbow = PRECACHE_EVENT( 1, "events/crossbow1.sc" );
	m_usCrossbow2 = PRECACHE_EVENT( 1, "events/crossbow2.sc" );
}


int CCrossbow::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "bolts";
	p->iMaxAmmo1 = BOLT_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = CROSSBOW_MAX_CLIP;
	p->iSlot = 2;
	p->iPosition = 2;
	p->iId = WEAPON_CROSSBOW;
	p->iFlags = 0;
	p->iWeight = CROSSBOW_WEIGHT;
	return 1;
}


BOOL CCrossbow::Deploy( )
{
	
	//MODDD - canceled?
	crossbowReloadSoundTimer = -1;

	//MODDD - idletime.
	if (m_iClip)
		return DefaultDeploy( "models/v_crossbow.mdl", "models/p_crossbow.mdl", CROSSBOW_DRAW1, "bow", 0, 0, (16.0/30.0), -1 );
	return DefaultDeploy( "models/v_crossbow.mdl", "models/p_crossbow.mdl", CROSSBOW_DRAW2, "bow", 0, 0, (16.0/30.0), -1 );
}

void CCrossbow::Holster( int skiplocal /* = 0 */ )
{
	int holsterAnimToSend;
	m_fInReload = FALSE;// cancel any reload in progress.

	//MODDD - canceled?
	crossbowReloadSoundTimer = -1;

	if (m_pPlayer->pev->fov != 0)
	{
		m_fInZoom = FALSE;
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 0;  // 0 means reset to default fov
	}

	/*
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	if (m_iClip)
		SendWeaponAnim( CROSSBOW_HOLSTER1 );
	else
		SendWeaponAnim( CROSSBOW_HOLSTER2 );
	*/


	//As of yet, both anim times are (16.0 / 30.0) seconds long,  frames/framerate.
	if(m_iClip){
		holsterAnimToSend = CROSSBOW_HOLSTER1;
	}else{
		holsterAnimToSend = CROSSBOW_HOLSTER2;
	}
	
	DefaultHolster(holsterAnimToSend, skiplocal, 0, (16.0f/30.0f));

}


void CCrossbow::PrimaryAttack( void )
{
	//MODDD - only give this chance if this new CVar is off.
	

	if(m_fInZoom){
		//If zoomed in, do a check for which mode to use.
		if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(playerCrossbowMode)!=2 && (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(playerCrossbowMode)==1 || !IsMultiplayer()) )
		{
			//single player? no difference, fall to the usual FireBolt below.

		}else{
			//Multiplayer? Do this instead.
			FireSniperBolt();
			return;
		}
	}

	FireBolt();
}



// this function only gets called in multiplayer
void CCrossbow::FireSniperBolt(){

	if (m_iClip == 0)
	{
		//MODDD - why play an empty sound?  This isn't a gun.
		//PlayEmptySound( );
		return;
	}

	TraceResult tr;

	m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;

	//MODDD
	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteclip) == 0){
		m_iClip --;
	}

	//MODDD
	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelay) == 0){

		if (m_iClip > 0) {
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + CROSSBOW_LOADBOLT_TIME;
		}
		else {
			// reloading next time?  Have a shorter delay.
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.6;
		}
	}else{
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelaycustom);
	}

	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	// HEY, COMMENT TIME!
	// So this playback method is interesting. m_usCrossbow2 puts the arrow right at the destination in an
	// instant, since sniper arrows work with a line-trace instead of a traveling arrow projectile
	// (just like bullets).
	// And it even works for other players: they see the bolt at the same place still.
	// So generating our own instant projectile at the destination serverside is redundant.
	// But still need to do the trace here to deal the damage.  Whoops.
	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usCrossbow2, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, m_iClip, PlayerPrimaryAmmoCount(), 0, 0 );

	// player "shoot" animation
	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
	
	Vector anglesAim = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;
	UTIL_MakeVectors( anglesAim );
	Vector vecSrc = m_pPlayer->GetGunPosition( ) - gpGlobals->v_up * 2;
	Vector vecDir = gpGlobals->v_forward;

	UTIL_TraceLine(vecSrc, vecSrc + vecDir * 8192, dont_ignore_monsters, m_pPlayer->edict(), &tr);


#ifndef CLIENT_DLL
	//MODDD - no NULL check? Are you daft man?
	if(tr.pHit != NULL){
		CBaseEntity* tempEnt = CBaseEntity::Instance(tr.pHit);
		Vector arrowVelocity;
		
		//MODDD - can stick to other map-related things so long as they don't move.
		//if ( FClassnameIs( pOther->pev, "worldspawn" ) )
		//if(pOther->IsWorldOrAffiliated() && pOther->pev->movetype == MOVETYPE_NONE)
		//...why do some static map things use MOVETYPE_PUSH? foget this.
		//if(tempEnt != NULL && tempEnt->IsWorld()){
		//	//make a crossbow bolt stick out of this.
		//	CCrossbowBolt *pBolt = CCrossbowBolt::BoltCreate();
		//	pBolt->pev->origin = tr.vecEndPos;
		//	pBolt->pev->angles = anglesAim;
		//	pBolt->pev->owner = m_pPlayer->edict();
		//	pBolt->m_velocity = Vector(0,0,0);
		//}

		if ( tr.pHit->v.takedamage )
		{
			ClearMultiDamage( );
			CBaseEntity::Instance(tr.pHit)->TraceAttack(m_pPlayer->pev, 120, vecDir, &tr, DMG_BULLET | DMG_NEVERGIB ); 
			ApplyMultiDamage( pev, m_pPlayer->pev );

			attemptSendBulletSound(tr.vecEndPos, m_pPlayer->pev);
		}
		
		// HACKY - set the velocity to let the arrow determine its own position as the Touch is called below.
		// pBolt->pev->velocity = vecDir;
		arrowVelocity = vecDir;
				
		// Tell this bolt not to deal damage because we already dealt the damage before even creating this bolt.
		// Nope!  Redundant with above now, already generate a crossbowbolt through the EV call.
		//CCrossbowBolt *pBolt = CCrossbowBolt::BoltCreate(arrowVelocity, 0, FALSE, TRUE);
		//pBolt->pev->origin = tr.vecEndPos;

		////no need to set pBolt->hitSomething, "BoltTouch" already does this.
		//pBolt->BoltTouch(tempEnt);
		

	}//END OF pHit null check
#endif

}//END OF FireSniperBolt

void CCrossbow::FireBolt()
{
	TraceResult tr;

	if (m_iClip == 0)
	{
		//MODDD - why play an empty sound?  This isn't a gun.
		//PlayEmptySound( );
		return;
	}

	m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;

	//MODDD
	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteclip) == 0){
		m_iClip --;
	}

	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif



	//!!!
	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usCrossbow, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, m_iClip, PlayerPrimaryAmmoCount(), 0, 0 );

	//MODDD - sync the idle delay with the lengths of the animations planned (as seen in ev_hldm for the crossbow's event)
	if(m_iClip ){
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (111.0 / 60.0) + randomIdleAnimationDelay() + randomIdleAnimationDelay();
	}else if(PlayerPrimaryAmmoCount() > 0){
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (16.0 / 30.0) + randomIdleAnimationDelay() + randomIdleAnimationDelay();
	}

	//MODDD
	//m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5.0;


	// player "shoot" animation
	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

	Vector anglesAim = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;
	UTIL_MakeVectors( anglesAim );
	
	anglesAim.x		= -anglesAim.x;
	Vector vecSrc	 = m_pPlayer->GetGunPosition( ) - gpGlobals->v_up * 2;
	Vector vecDir	 = gpGlobals->v_forward;


#ifndef CLIENT_DLL

	Vector arrowVelocity;
	float arrowSpeed;
	//MODDD - just decide this ahead of spawning the arrow, and give it to the arrow.
	if (m_pPlayer->pev->waterlevel == 3)
	{
		//MODDd - SEE BELOW.
		//pBolt->pev->velocity = vecDir * BOLT_WATER_VELOCITY;
		arrowVelocity = vecDir * BOLT_WATER_VELOCITY + UTIL_GetProjectileVelocityExtra(m_pPlayer->pev->velocity, EASY_CVAR_GET_DEBUGONLY(crossbowInheritsPlayerVelocity) );

		arrowSpeed = BOLT_WATER_VELOCITY;
	}
	else
	{
		//MODDD - allow to be affected by "affectedByPlayerVelocity"
		//pBolt->pev->velocity = vecDir * BOLT_AIR_VELOCITY;
		arrowVelocity = vecDir * BOLT_AIR_VELOCITY + UTIL_GetProjectileVelocityExtra(m_pPlayer->pev->velocity, EASY_CVAR_GET_DEBUGONLY(crossbowInheritsPlayerVelocity) );

		arrowSpeed = BOLT_AIR_VELOCITY;
	}


	CCrossbowBolt *pBolt = CCrossbowBolt::BoltCreate(arrowVelocity, arrowSpeed);
	pBolt->pev->origin = vecSrc;
	pBolt->pev->angles = anglesAim;
	pBolt->pev->owner = m_pPlayer->edict();

	//MODDD NOTE - a slight angular Z movement (or roll)? Did they really expect people to notice a slight roll on a speeding cylindrical object?
	pBolt->pev->avelocity.z = 10;
	
#endif

	if (!m_iClip && PlayerPrimaryAmmoCount() <= 0)
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

	//MODDD
	// also removed the NextSecondaryAttack sets.  What's the point of affecting when we can zoom again?
	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelay) == 0){

		if (m_iClip > 0) {
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + CROSSBOW_LOADBOLT_TIME;
		}
		else {
			// reloading next time?  Have a shorter delay.
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.6;
		}
		//m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + CROSSBOW_LOADBOLT_TIME;
	}else{
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelaycustom);
		//m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelaycustom);
	}

	
	/*
	if (m_iClip != 0)
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5.0;
	else
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.75;
	*/

}


void CCrossbow::SecondaryAttack()
{

	// This is what checks for a key being pressed for the first frame since a previous release (or first time ever).
	// This does not count continually holding it down after.
	// odd that retail never took advantage of this.
	if(!(m_pPlayer->m_afButtonPressed & IN_ATTACK2)){
		//MODDD
		// If we're not the first frame, don't do it!  Stops holding down the key from continuously
		// zooming in and out.  This would happen stupidly rapidly now that the 1 second forced
		// delay has been lifted.  Be responsive dammit!
		return;
	}
	
	// Clientside must get the FOV value from the gHUD instance instead, the player info is only useful
	// serverside.
	// Strangely, even in retail "m_pPlayer->pev->fov" and "m_pPlayer->m_iFOV" were changed on zoom for both
	// client and serverside, even though it would've been useless clientside. 
	// On clientside, only HUD-related logic handles FOV.  No idea.
	// Still making sure the thing checked is correct (zoomedFOV), since retail did compare against a constant
	// (which would be correct for client/serverside, so this should be too)
	// Our new "getPlayerBaseFOV", in CBaseWeapon, gets the intended default/automatic FOV from client/server
	// correctly.
	const int zoomedFOV = (int)roundf(getPlayerBaseFOV() * 0.2222f);

	if ( m_pPlayer->pev->fov != 0 )
	{
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 0; // 0 means reset to default fov
		m_fInZoom = 0;
	}
	//MODDD - used to be only 20.
	else if ( m_pPlayer->pev->fov != zoomedFOV)
	{
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = zoomedFOV;
		m_fInZoom = 1;
	}
	
	pev->nextthink = UTIL_WeaponTimeBase() + 0.1;

	//if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelay) == 0){
	//MODDD - NOPE! Not even you now.  Outclassed.
	//	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0;
	//}else{
	//	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelaycustom);
	//}

}



//MODDD - new.
void CCrossbow::ItemPostFrameThink(){

	#ifndef CLIENT_DLL
		
	/*
#ifdef CLIENT_DLL
	easyForcePrintLine("CL: I WILL EAT YOU %.2f %.2f %d", crossbowReloadSoundTimer, UTIL_WeaponTimeBase(), (UTIL_WeaponTimeBase() >= crossbowReloadSoundTimer) );
#else
	easyForcePrintLine("SV: I WILL EAT YOU %.2f %.2f %d", crossbowReloadSoundTimer, UTIL_WeaponTimeBase(), (UTIL_WeaponTimeBase() >= crossbowReloadSoundTimer) );
#endif
	*/

	if(crossbowReloadSoundTimer != -1 && gpGlobals->time >= crossbowReloadSoundTimer){
		crossbowReloadSoundTimer = -1;

		//play it!
		easyForcePrintLine("xbow_reload1w.wav PLAYED");
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/xbow_reload1.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 93 + RANDOM_LONG(0,0xF));

	}//END OF crossbowReloadSoundTimer check

	#endif
	
	CBasePlayerWeapon::ItemPostFrameThink();

}//END OF ItemPostFrameThink()




//MODDD - new.
void CCrossbow::ItemPostFrame(){

	

	CBasePlayerWeapon::ItemPostFrame();
}//END OF ItemPostFrame()



void CCrossbow::Reload( void )
{
	if ( m_pPlayer->ammo_bolts <= 0 )
		return;

	if ( m_pPlayer->pev->fov != 0 )
	{
		//MODDD - really... just set the FOV dammit.
		//SecondaryAttack();
		m_fInZoom = FALSE;
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 0;  // 0 means reset to default fov
	}

	if ( DefaultReload( 5, CROSSBOW_RELOAD, 4.5 ) )
	{
		#ifndef CLIENT_DLL
		//UTIL_WeaponTimeBase() ???

		if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(crossbowReloadSoundDelay) >= 0){
			crossbowReloadSoundTimer = gpGlobals->time + EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(crossbowReloadSoundDelay);
		}
		 
		//MODDD - moved the sound effect to ItemPostFrame for playing later.
		#endif
	}

}


void CCrossbow::WeaponIdle( void )
{
	/*
#ifdef CLIENT_DLL
	easyForcePrintLine("CL: WeaponIdle %.2f %.2f %d", m_flTimeWeaponIdle, UTIL_WeaponTimeBase(), (UTIL_WeaponTimeBase() >= m_flTimeWeaponIdle) );
#else
	easyForcePrintLine("SV: WeaponIdle %.2f %.2f %d", m_flTimeWeaponIdle, UTIL_WeaponTimeBase(), (UTIL_WeaponTimeBase() >= m_flTimeWeaponIdle) );
#endif
	*/

	m_pPlayer->GetAutoaimVector( AUTOAIM_2DEGREES );  // get the autoaim vector but ignore it;  used for autoaim crosshair in DM

	ResetEmptySound( );
	
	if ( m_flTimeWeaponIdle < UTIL_WeaponTimeBase() )
	{
		/*
#ifdef CLIENT_DLL
	easyForcePrintLine("CL: WOOHOO! %.2f %.2f %d", m_flTimeWeaponIdle, UTIL_WeaponTimeBase(), (UTIL_WeaponTimeBase() >= m_flTimeWeaponIdle) );
#else
	easyForcePrintLine("SV: WOOHOO! %.2f %.2f %d", m_flTimeWeaponIdle, UTIL_WeaponTimeBase(), (UTIL_WeaponTimeBase() >= m_flTimeWeaponIdle) );
#endif
	*/

		forceBlockLooping();

		float flRand = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 0, 1 );
		if (flRand <= 0.75)
		{
			if (m_iClip)
			{
				SendWeaponAnim( CROSSBOW_IDLE1 );
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 90.0 / 30.0 + randomIdleAnimationDelay();
			}
			else
			{
				SendWeaponAnim( CROSSBOW_IDLE2 );
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 30.0 / 30.0 + randomIdleAnimationDelay();
			}
			// + randomIdleAnimationDelay()
			//m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
		}
		else
		{
			if (m_iClip)
			{
				SendWeaponAnim( CROSSBOW_FIDGET1 );
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 80.0 / 30.0 + randomIdleAnimationDelay();
			}
			else
			{
				SendWeaponAnim( CROSSBOW_FIDGET2 );
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 30.0 / 30.0 + randomIdleAnimationDelay();
			}
		}
	}
}



class CCrossbowAmmo : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/w_crossbow_clip.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/w_crossbow_clip.mdl");
		precacheAmmoPickupSound();
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		if (pOther->GiveAmmo( AMMO_CROSSBOWCLIP_GIVE, "bolts", BOLT_MAX_CARRY ) != -1)
		{
			playAmmoPickupSound();

			//MODDD
			if(pOther->IsPlayer()){
				CBasePlayer* pPlayer = (CBasePlayer*)pOther;
				pPlayer->SetSuitUpdate("!HEV_BOLTS", FALSE, SUIT_NEXT_IN_20MIN, 4.3);
			}

			return TRUE;
		}
		return FALSE;
	}
};
LINK_ENTITY_TO_CLASS( ammo_crossbow, CCrossbowAmmo );


