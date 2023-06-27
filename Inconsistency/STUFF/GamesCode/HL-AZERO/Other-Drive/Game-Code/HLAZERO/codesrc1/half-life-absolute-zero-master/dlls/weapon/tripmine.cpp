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
#include "tripmine.h"
#include "util.h"
#include "cbase.h"
#include "basemonster.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "effects.h"
#include "gamerules.h"


EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(tripmineAnimWaitsForFinish)

EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelay)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelaycustom)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteclip)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteammo)


//MODDD - for the viewmodel and worldmodel, same v_tripmine.mdl model used.
#define BODYGROUP_HANDS 0
#define HANDS_VISIBLE 0
#define HANDS_INVISIBLE 1

#define BODYGROUP_MINE 1
#define MINE_VIEWMODEL 0
#define MINE_WORLD 1




#ifndef CLIENT_DLL


LINK_ENTITY_TO_CLASS( monster_tripmine, CTripmineGrenade );

TYPEDESCRIPTION	CTripmineGrenade::m_SaveData[] = 
{
	DEFINE_FIELD( CTripmineGrenade, m_flPowerUp, FIELD_TIME ),
	DEFINE_FIELD( CTripmineGrenade, m_vecDir, FIELD_VECTOR ),
	DEFINE_FIELD( CTripmineGrenade, m_vecEnd, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( CTripmineGrenade, m_flBeamLength, FIELD_FLOAT ),
	DEFINE_FIELD( CTripmineGrenade, m_hOwner, FIELD_EHANDLE ),
	DEFINE_FIELD( CTripmineGrenade, m_pBeam, FIELD_CLASSPTR ),
	DEFINE_FIELD( CTripmineGrenade, m_posOwner, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( CTripmineGrenade, m_angleOwner, FIELD_VECTOR ),
	DEFINE_FIELD( CTripmineGrenade, m_pRealOwner, FIELD_EDICT ),
};

IMPLEMENT_SAVERESTORE(CTripmineGrenade,CGrenade);





void CTripmineGrenade::Spawn( void )
{

	//MODDD - this is just a precaution to keep "m_pBeam" from being a garbage value.
	m_pBeam = NULL;

	Precache( );
	// motor
	pev->movetype = MOVETYPE_FLY;
	pev->solid = SOLID_NOT;

	//MODDD NOTE - This is not the selectable player viewmodel weapon, this is the physical in-world explosive, 
	//             and we're using model v_tripmine? the view model? wha?
	SET_MODEL(ENT(pev), "models/v_tripmine.mdl");

	pev->frame = 0;

	pev->body = 0;
	SetBodygroup(BODYGROUP_HANDS, HANDS_INVISIBLE);
	SetBodygroup(BODYGROUP_MINE, MINE_WORLD);
	// same net effect of = 3



	pev->sequence = TRIPMINE_WORLD;
	ResetSequenceInfo( );
	pev->framerate = 0;
	
	UTIL_SetSize(pev, Vector( -8, -8, -8), Vector(8, 8, 8));
	UTIL_SetOrigin( pev, pev->origin );


	//MODDD - new check. Only player-placed tripmines get a delay and make a noise on startup.
	if(pev->spawnflags & SF_MONSTER_DYNAMIC){

		if (pev->spawnflags & 1)
		{
			// power up quickly
			m_flPowerUp = gpGlobals->time + 1.0;
		}
		else
		{
			// power up in 2.5 seconds
			m_flPowerUp = gpGlobals->time + 2.5;
		}

		if (pev->owner != NULL)
		{
			// play deploy sound
			EMIT_SOUND( ENT(pev), CHAN_VOICE, "weapons/mine_deploy.wav", 1.0, ATTN_NORM );
			EMIT_SOUND( ENT(pev), CHAN_BODY, "weapons/mine_charge.wav", 0.2, ATTN_NORM ); // chargeup

			m_pRealOwner = pev->owner;// see CTripmineGrenade for why.
		}
		SetThink( &CTripmineGrenade::PowerupThink );
		pev->nextthink = gpGlobals->time + 0.2;

	}else{
		//placed by the map instead? No delay

		
		SetThink( &CTripmineGrenade::PowerupThink );
		pev->nextthink = gpGlobals->time + 0.2;
		m_flPowerUp = gpGlobals->time;

		//It may sound like a good idea to call ActivateMine here, but for reasons beyond my mortal understanding,
		//this causes some tripmines to explode at spawn. Perhaps they are soon slightly adjusted and activating
		//the mine this early causes issues? At least one think cycle should sort this out to still appear instant.

		//ActivateMine(FALSE);
	}

	pev->takedamage = DAMAGE_YES;
	pev->dmg = gSkillData.plrDmgTripmine;
	pev->health = 1; // don't let die normally

	UTIL_MakeAimVectors( pev->angles );

	m_vecDir = gpGlobals->v_forward;
	m_vecEnd = pev->origin + m_vecDir * 2048;
}


void CTripmineGrenade::Precache( void )
{
	PRECACHE_MODEL("models/v_tripmine.mdl");
	PRECACHE_SOUND("weapons/mine_deploy.wav");
	PRECACHE_SOUND("weapons/mine_activate.wav");
	PRECACHE_SOUND("weapons/mine_charge.wav");

	precacheGunPickupSound();
	precacheAmmoPickupSound(); //doubles as ammo.
}


//MODDD NOTE - this is unused. When would this have happened? To signal that mines that used to be deactivated say at map startup
//             have now been toggled on?
void CTripmineGrenade::WarningThink( void  )
{
	// play warning sound
	// EMIT_SOUND( ENT(pev), CHAN_VOICE, "buttons/Blip2.wav", 1.0, ATTN_NORM );

	// set to power up
	SetThink( &CTripmineGrenade::PowerupThink );
	pev->nextthink = gpGlobals->time + 1.0;
}


void CTripmineGrenade::PowerupThink( void  )
{
	TraceResult tr;

	if (m_hOwner == NULL)
	{
		// find an owner
		edict_t *oldowner = pev->owner;
		pev->owner = NULL;
		UTIL_TraceLine( pev->origin + m_vecDir * 8, pev->origin - m_vecDir * 32, dont_ignore_monsters, ENT( pev ), &tr );
		if (tr.fStartSolid || (oldowner && tr.pHit == oldowner))
		{
			pev->owner = oldowner;
			m_flPowerUp += 0.1;
			pev->nextthink = gpGlobals->time + 0.1;
			return;
		}
		if (tr.flFraction < 1.0)
		{
			pev->owner = tr.pHit;
			m_hOwner = CBaseEntity::Instance( pev->owner );
			m_posOwner = m_hOwner->pev->origin;
			m_angleOwner = m_hOwner->pev->angles;
		}
		else
		{
			UTIL_StopSound( ENT(pev), CHAN_VOICE, "weapons/mine_deploy.wav" );
			UTIL_StopSound( ENT(pev), CHAN_BODY, "weapons/mine_charge.wav" );
			SetThink( &CBaseEntity::SUB_Remove );
			pev->nextthink = gpGlobals->time + 0.1;
			ALERT( at_console, "WARNING:Tripmine at %.0f, %.0f, %.0f removed\n", pev->origin.x, pev->origin.y, pev->origin.z );
			KillBeam();
			return;
		}
	}
	else if (m_posOwner != m_hOwner->pev->origin || m_angleOwner != m_hOwner->pev->angles)
	{
		// disable
		UTIL_StopSound( ENT(pev), CHAN_VOICE, "weapons/mine_deploy.wav" );
		UTIL_StopSound( ENT(pev), CHAN_BODY, "weapons/mine_charge.wav" );
		CBaseEntity *pMine = Create( "weapon_tripmine", pev->origin + m_vecDir * 24, pev->angles );
		pMine->pev->spawnflags |= SF_NORESPAWN;

		SetThink( &CBaseEntity::SUB_Remove );
		KillBeam();
		pev->nextthink = gpGlobals->time + 0.1;
		return;
	}
	// ALERT( at_console, "%d %.0f %.0f %0.f\n", pev->owner, m_pOwner->pev->origin.x, m_pOwner->pev->origin.y, m_pOwner->pev->origin.z );
 


	if (gpGlobals->time > m_flPowerUp)
	{
		//NOTICE - this changes the think method which doesn't check m_flPowerUp anymore,
		//         so this won't lead to calling this place over and over again.
		//         Also, playing the sound here since this implies the player set the mine recently. As opposed to map-spawned mines
		//         which have been there for who knows how long (in the world).
		BOOL playStartupSound = (pev->spawnflags & SF_MONSTER_DYNAMIC)!=0;
		//if this spawnflag is on, play the startupsound. Otherwise assume placed by the map.

		ActivateMine(playStartupSound);
	}
	pev->nextthink = gpGlobals->time + 0.1;
}


void CTripmineGrenade::KillBeam( void )
{
	if ( m_pBeam )
	{
		UTIL_Remove( m_pBeam );
		m_pBeam = NULL;
	}
}


float CTripmineGrenade::massInfluence(void){
	return 0.20f;
}//END OF massInfluence
int CTripmineGrenade::GetProjectileType(void){
	return PROJECTILE_DEPLOYABLE;
}




void CTripmineGrenade::Activate( void ){
	CGrenade::Activate();

	if(m_pBeam){
	//m_pBeam->SetColor( 0, 214, 198 );
	m_pBeam->SetColor( 218, 0, 0 );
	}
}

//Not to be confused with "Activate", the built-in method for entities on finishing loading a map and all other entities.
void CTripmineGrenade::ActivateMine(BOOL argPlayActivateSound){

	// make solid
	pev->solid = SOLID_BBOX;
	UTIL_SetOrigin( pev, pev->origin );

	MakeBeam( );

	if(argPlayActivateSound){
		// play enabled sound
		EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "weapons/mine_activate.wav", 0.5, ATTN_NORM, 1.0, 75 );
	}
}//END OF ActivateMine



void CTripmineGrenade::MakeBeam( void )
{
	TraceResult tr;

	// ALERT( at_console, "serverflags %f\n", gpGlobals->serverflags );

	UTIL_TraceLine( pev->origin, m_vecEnd, dont_ignore_monsters, ENT( pev ), &tr );

	m_flBeamLength = tr.flFraction;

	// set to follow laser spot
	SetThink( &CTripmineGrenade::BeamBreakThink );
	pev->nextthink = gpGlobals->time + 0.1;

	Vector vecTmpEnd = pev->origin + m_vecDir * 2048 * m_flBeamLength;

	m_pBeam = CBeam::BeamCreate( g_pModelNameLaser, 10 );
	m_pBeam->PointEntInit( vecTmpEnd, entindex() );

	//MODDD - tripmine laser color altered.
	//m_pBeam->SetColor( 0, 214, 198 );
	m_pBeam->SetColor( 218, 0, 0 );

	m_pBeam->SetScrollRate( 255 );
	m_pBeam->SetBrightness( 64 );
}


void CTripmineGrenade::BeamBreakThink( void  )
{
	BOOL bBlowup = 0;

	TraceResult tr;

	// HACKHACK Set simple box using this really nice global!
	gpGlobals->trace_flags = FTRACE_SIMPLEBOX;
	UTIL_TraceLine( pev->origin, m_vecEnd, dont_ignore_monsters, ENT( pev ), &tr );

	// ALERT( at_console, "%f : %f\n", tr.flFraction, m_flBeamLength );

	// respawn detect. 
	if ( !m_pBeam )
	{
		MakeBeam( );
		if (tr.pHit) {
			m_hOwner = CBaseEntity::Instance(tr.pHit);	// reset owner too
		}
	}

	if (fabs( m_flBeamLength - tr.flFraction ) > 0.001)
	{
		bBlowup = 1;
	}
	else
	{
		if (m_hOwner == NULL)
			bBlowup = 1;
		else if (m_posOwner != m_hOwner->pev->origin)
			bBlowup = 1;
		else if (m_angleOwner != m_hOwner->pev->angles)
			bBlowup = 1;
	}

	if (bBlowup)
	{
		// a bit of a hack, but all CGrenade code passes pev->owner along to make sure the proper player gets credit for the kill
		// so we have to restore pev->owner from pRealOwner, because an entity's tracelines don't strike it's pev->owner which meant
		// that a player couldn't trigger his own tripmine. Now that the mine is exploding, it's safe the restore the owner so the 
		// CGrenade code knows who the explosive really belongs to.
		pev->owner = m_pRealOwner;
		pev->health = 0;
		
		//MODDD - is this ok to send myself, the tripmine, as the inflictor, 1st arg? probably unimportant.
		Killed( pev, VARS( pev->owner ), GIB_NORMAL );
		return;
	}

	pev->nextthink = gpGlobals->time + 0.1;
}



GENERATE_TRACEATTACK_IMPLEMENTATION(CTripmineGrenade)
{
	GENERATE_TRACEATTACK_PARENT_CALL(CGrenade);
}

GENERATE_TAKEDAMAGE_IMPLEMENTATION(CTripmineGrenade)
{
	if (gpGlobals->time < m_flPowerUp && flDamage < pev->health)
	{
		// disable
		// Create( "weapon_tripmine", pev->origin + m_vecDir * 24, pev->angles );
		SetThink( &CBaseEntity::SUB_Remove );
		pev->nextthink = gpGlobals->time + 0.1;
		KillBeam();
		return FALSE;
	}
	return GENERATE_TAKEDAMAGE_PARENT_CALL(CGrenade);
}

GENERATE_KILLED_IMPLEMENTATION(CTripmineGrenade)
{
	pev->takedamage = DAMAGE_NO;
	
	if ( pevAttacker && ( pevAttacker->flags & FL_CLIENT ) )
	{
		// some client has destroyed this mine, he'll get credit for any kills
		pev->owner = ENT( pevAttacker );
	}

	SetThink( &CTripmineGrenade::DelayDeathThink );
	pev->nextthink = gpGlobals->time + RANDOM_FLOAT( 0.1, 0.3 );

	EMIT_SOUND( ENT(pev), CHAN_BODY, "common/null.wav", 0.5, ATTN_NORM ); // shut off chargeup
}


void CTripmineGrenade::DelayDeathThink( void )
{
	KillBeam();
	TraceResult tr;
	UTIL_TraceLine ( pev->origin + m_vecDir * 8, pev->origin - m_vecDir * 64,  dont_ignore_monsters, ENT(pev), & tr);

	//return;
	Explode( &tr, DMG_BLAST );
}
#endif
//MODDDD - NOTE: all the above was server-side only.  Easy to miss this...


LINK_ENTITY_TO_CLASS( weapon_tripmine, CTripmine );

CTripmine::CTripmine(){
	holdingSecondaryTarget0 = 0.0f;
	holdingSecondaryTarget1 = 0.37f;
	holdingSecondaryTarget2 = 0.54f;

	m_flReleaseThrow = -1;
	m_flStartThrow = -1;

	//coordinated with whether to put a full delay for placing tripmines to show all of the place anim or not (retail default).
	m_fireState = 0;

	//NOTE: this used to be a custom var named "weaponRetired", but it does not seem to sync too well b/w the server & client.
	m_fInAttack = 0;

}


//MODDD
void CTripmine::customAttachToPlayer(CBasePlayer *pPlayer ){
	m_pPlayer->SetSuitUpdate("!HEV_TRIPMINE", FALSE, SUIT_NEXT_IN_30MIN, 4.3f);
}

void CTripmine::Spawn( )
{
	Precache( );
	m_iId = WEAPON_TRIPMINE;
	SET_MODEL(ENT(pev), "models/v_tripmine.mdl");
	pev->frame = 0;

	pev->body = 0;
	SetBodygroup(BODYGROUP_HANDS, HANDS_INVISIBLE);
	SetBodygroup(BODYGROUP_MINE, MINE_WORLD);
	// same net effect of = 3

	pev->sequence = TRIPMINE_GROUND;
	// ResetSequenceInfo( );
	pev->framerate = 0;

	FallInit();// get ready to fall down

	m_iClip = -1;
	m_iDefaultAmmo = TRIPMINE_DEFAULT_GIVE;

	if ( !IsMultiplayer() )
	{
		UTIL_SetSize(pev, Vector(-16, -16, 0), Vector(16, 16, 28) ); 
	}

	//MODDD - new.
	m_fInAttack = FALSE;

}

void CTripmine::Precache( void )
{
	PRECACHE_MODEL ("models/v_tripmine.mdl");
	PRECACHE_MODEL ("models/p_tripmine.mdl");
	UTIL_PrecacheOther( "monster_tripmine" );

	m_usTripFire = PRECACHE_EVENT( 1, "events/tripfire.sc" );
}

int CTripmine::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "Trip Mine";
	p->iMaxAmmo1 = TRIPMINE_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 4;
	p->iPosition = 2;
	p->iId = m_iId = WEAPON_TRIPMINE;
	p->iWeight = TRIPMINE_WEIGHT;
	p->iFlags = ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE;

	return 1;
}

BOOL CTripmine::Deploy( )
{
	//MODDD - new.
	m_fInAttack = FALSE;

	//pev->body = 0;
	return DefaultDeploy( "models/v_tripmine.mdl", "models/p_tripmine.mdl", TRIPMINE_DRAW, "trip", 0, 0, (16.0/30.0), -1 );
}


void CTripmine::Holster( int skiplocal /* = 0 */ )
{
	//MODDD - new.
	m_fInAttack = FALSE;

	//m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

	if (PlayerPrimaryAmmoCount() <= 0)
	{
		// out of mines
		m_pPlayer->pev->weapons &= ~(1<<WEAPON_TRIPMINE);
		SetThink( &CBasePlayerItem::DestroyItem );
		pev->nextthink = gpGlobals->time + 0.1;
	}else{
		//MODDD - now happens only when not planning on destroying the weapon (no ammo, no point in this)
		//SendWeaponAnim( TRIPMINE_HOLSTER );
		DefaultHolster(TRIPMINE_HOLSTER, skiplocal, 0, (16.0/30.0f));
	}

	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "common/null.wav", 1.0, ATTN_NORM);
}


void CTripmine::PrimaryAttack( void )
{
	if (PlayerPrimaryAmmoCount() <= 0) {
		return;
	}

	UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );
	Vector vecSrc	 = m_pPlayer->GetGunPosition( );
	Vector vecAiming = gpGlobals->v_forward;

	TraceResult tr;

	UTIL_TraceLine( vecSrc, vecSrc + vecAiming * 128, dont_ignore_monsters, ENT( m_pPlayer->pev ), &tr );

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usTripFire, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, 0, 0, 0, 0 );


	//MODDD - these traces don't even work clientside (dummies are given that have flFraction forced to 1.0),
	//        why bother compiling for the client anyway?
#ifndef CL_DLL
	//MODDD
	if (UTIL_PointContents(tr.vecEndPos) == CONTENTS_SKY) {
		// If we hit the sky, HALT!  Don't put a tripmine on me.
		// ...wait, how'd you reach this?  CHEATERRRRRRrrrrrrrrrrrr
		
	}else if (tr.flFraction < 1.0)
	{
		//success.  play animation.
		m_flReleaseThrow = 0;
		m_flStartThrow = gpGlobals->time + holdingSecondaryTarget1;
		SendWeaponAnimBypass( TRIPMINE_ARM2 );
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (11.0 / 30.0) + randomIdleAnimationDelay();

		CBaseEntity *pEntity = CBaseEntity::Instance( tr.pHit );
		if ( pEntity && !(pEntity->pev->flags & FL_CONVEYOR) )
		{
			Vector angles = UTIL_VecToAngles( tr.vecPlaneNormal );

			CBaseEntity *pEnt = CBaseEntity::Create( "monster_tripmine", tr.vecEndPos + tr.vecPlaneNormal * 8, angles, SF_MONSTER_DYNAMIC, m_pPlayer->edict() );

			//MODDD - cheat check.			
			if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteclip) == 0 && EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteammo) == 0){
				ChangePlayerPrimaryAmmoCount(-1);
			}

			// player "shoot" animation
			m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
			
			if (PlayerPrimaryAmmoCount() <= 0 )
			{
				// no more mines! 
				//MODDD - rather, do this after the "place" (arm2) animation is done.
				//RetireWeapon();
				return;
			}
		}
		else
		{
			// ALERT( at_console, "no deploy\n" );
		}
	}
	else
	{

	}
#endif
	
	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelay) == 0){
		// see "ItemPreFrame" for assignment.  "m_fireState" relies on the "tripmineAnimWaitsForFinish" cvar.
		// CHANGED: times were 0.3 and 0.9.
		if(m_fireState == 0){
			SetAttackDelays(UTIL_WeaponTimeBase() + 0.3);
		}else{
			//this delay allows the place and deploy anims to finish.
			SetAttackDelays(UTIL_WeaponTimeBase() + 0.9);
		}
	}else{
		SetAttackDelays(UTIL_WeaponTimeBase() + EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelaycustom));
	}

	//m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
}



void CTripmine::SecondaryAttack(void)
{

	if (EASY_CVAR_GET(cl_viewmodel_fidget) == 2) {
		//float flRand;
		int iAnim;

		//flRand = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 0, 1);
		//if (flRand <= 0.5)
		//{
			iAnim = TRIPMINE_FIDGET;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 100.0 / 30.0;
		//}
		
		//SetAttackDelays(m_flTimeWeaponIdle);
		m_flNextSecondaryAttack = m_flTimeWeaponIdle;
		m_flTimeWeaponIdle += randomIdleAnimationDelay();
		SendWeaponAnim(iAnim);
	}//CVar check

}





//MODDD - new.
void CTripmine::ItemPreFrame( void ){

	CBasePlayerWeapon::ItemPreFrame();
}


void CTripmine::ItemPostFrame(void) {

	CBasePlayerWeapon::ItemPostFrame();
}

//MODDD - new.
void CTripmine::ItemPostFrameThink( void ){


	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(tripmineAnimWaitsForFinish) == 1){
		m_fireState = 1;
	}else{
		m_fireState = 0;
	}

	if(gpGlobals->time > m_flStartThrow){

		if(m_flReleaseThrow == -1){

		}else if(m_flReleaseThrow == 0){
			if (PlayerPrimaryAmmoCount() <= 0 ){
				//If out of ammo, let the idle method catch this and retire the weapon.
				m_flReleaseThrow = -1;
				m_flStartThrow = -1;
				m_flTimeWeaponIdle = 0;  //call for idle animation, nothing else to do.
			}else{
				m_flReleaseThrow = 1;
				m_flStartThrow = gpGlobals->time + holdingSecondaryTarget2;
				SendWeaponAnimBypass( TRIPMINE_DRAW );
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (11.0/30.0) + randomIdleAnimationDelay();

				//m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + randomIdleAnimationDelay() + 10;
				//m_pPlayer->forceNoWeaponLoop = TRUE;
			}
			
		}else if(m_flReleaseThrow == 1){
			m_flReleaseThrow = -1;
			m_flStartThrow = -1;
			//m_flTimeWeaponIdle = 0;  //call for idle animation, nothing else to do.
		}
	}


	CBasePlayerWeapon::ItemPostFrameThink();
}


void CTripmine::WeaponIdle( void )
{
	//Do not idle if doing another explicit animation.
	if(m_flReleaseThrow == -1){

	}else{
		return;
	}

	/*

	if ( PlayerPrimaryAmmoCount() > 0 ){
		//VERIFY.  Is this even reached anymore?
		if(m_fInAttack == TRUE){
			m_fInAttack = FALSE;


			DefaultDeploy( "models/v_tripmine.mdl", "models/p_tripmine.mdl", TRIPMINE_DRAW, "trip", 0, 0, (16.0/30.0), -1 );
			//m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + randomIdleAnimationDelay() + 10;
			//m_pPlayer->forceNoWeaponLoop = TRUE;
			return;
		}
		//SendWeaponAnim( TRIPMINE_DRAW );
		//m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (11.0/30.0);
	}
	*/

	
	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;


	if (m_pPlayer->pev->viewmodel == iStringNull) {
		if (PlayerPrimaryAmmoCount() > 0) {

			globalflag_muteDeploySound = TRUE;
			Deploy();
			globalflag_muteDeploySound = FALSE;

			return;
		}
	}

	
	//no ammo? retire.
	if (PlayerPrimaryAmmoCount() <= 0 ){
		RetireWeapon();
		m_fInAttack = TRUE;
		return;
	}

	int iAnim;
	float flRand = 0;
	
	if (EASY_CVAR_GET(cl_viewmodel_fidget) == 1) {
		flRand = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 0, 1);
	}
	else {
		// never play fidget this way.
		flRand = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 0, 0.75);
	}

	if (flRand <= 0.25)
	{
		iAnim = TRIPMINE_IDLE1;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 90.0 / 30.0 + randomIdleAnimationDelay();
	}
	else if (flRand <= 0.75)
	{
		iAnim = TRIPMINE_IDLE2;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 60.0 / 30.0 + randomIdleAnimationDelay();
	}
	else
	{
		iAnim = TRIPMINE_FIDGET;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 100.0 / 30.0 + randomIdleAnimationDelay();
	}

	SendWeaponAnim( iAnim );
}


