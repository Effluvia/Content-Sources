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


//MODDD TODO
//- the spawned snark model (w_squeak.mdl) has a "fidget" animation that is never used.  ?
// And a jump one.  	WSQUEAK_FIDGET,  WSQUEAK_JUMP.  Although not sure where to use those.
// Anim's that fudge the origin of the monster like that are awkward enough to deal with in death anims,
// let alone alive things.




#pragma once

#include "extdll.h"
#include "squeak.h"
#include "util.h"
#include "cbase.h"
#include "basemonster.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "soundent.h"
#include "gamerules.h"

EASY_CVAR_EXTERN_DEBUGONLY(snarkInheritsPlayerVelocity)

EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelay)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelaycustom)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteclip)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteammo)

#define SQUEEK_DETONATE_DELAY 15.0



#ifndef CLIENT_DLL


float CSqueakGrenade::m_flNextBounceSoundTime = 0;


#if REMOVE_ORIGINAL_NAMES != 1
	LINK_ENTITY_TO_CLASS( monster_snark, CSqueakGrenade );
#endif

#if EXTRA_NAMES > 0
	LINK_ENTITY_TO_CLASS( snark, CSqueakGrenade );
	
	//no extras.
#endif


TYPEDESCRIPTION	CSqueakGrenade::m_SaveData[] = 
{
	DEFINE_FIELD( CSqueakGrenade, m_flDie, FIELD_TIME ),
	// ok, vecTarget is a direction, not a position (which would have needed FIELD_POSITION_VECTOR instead.
	DEFINE_FIELD( CSqueakGrenade, m_vecTarget, FIELD_VECTOR ),
	DEFINE_FIELD( CSqueakGrenade, m_flNextHunt, FIELD_TIME ),
	DEFINE_FIELD( CSqueakGrenade, m_flNextHit, FIELD_TIME ),
	DEFINE_FIELD( CSqueakGrenade, m_posPrev, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( CSqueakGrenade, m_hOwner, FIELD_EHANDLE ),
	
};

//MODDD - class change. was CGrenade.
//IMPLEMENT_SAVERESTORE( CSqueakGrenade, CGrenade );
IMPLEMENT_SAVERESTORE(CSqueakGrenade, CBaseMonster);


int CSqueakGrenade::Classify ( void )
{
	if (m_iMyClass != 0)
		return m_iMyClass; // protect against recursion

	if (m_hEnemy != NULL)
	{
		m_iMyClass = CLASS_INSECT; // no one cares about it
		switch( m_hEnemy->Classify( ) )
		{
			case CLASS_PLAYER:
			case CLASS_HUMAN_PASSIVE:
			case CLASS_HUMAN_MILITARY:
			//MODDD - why no CLASS_PLAYER_ALLY?
			case CLASS_PLAYER_ALLY:
			{
				m_iMyClass = 0;
				return CLASS_ALIEN_MILITARY; // barney's get mad, grunts get mad at it
			}
		}
		m_iMyClass = 0;
	}

	return CLASS_ALIEN_BIOWEAPON;
}

CSqueakGrenade::CSqueakGrenade(){

}

// nevermind, probably already precached in whole by the player, no assistance needed.  Whatever, being explicit doesn't hurt.
BOOL CSqueakGrenade::usesSoundSentenceSave(void){
	return FALSE;
}



void CSqueakGrenade::Spawn( void )
{
	Precache( );
	// motor

	pev->classname = MAKE_STRING("monster_snark");

	pev->movetype = MOVETYPE_BOUNCE;
	pev->solid = SOLID_BBOX;

	setModel( "models/w_squeak.mdl");
	UTIL_SetSize(pev, Vector( -4, -4, 0), Vector(4, 4, 8));
	UTIL_SetOrigin( pev, pev->origin );

	SetTouch( &CSqueakGrenade::SuperBounceTouch );
	SetThink( &CSqueakGrenade::HuntThink );
	pev->nextthink = gpGlobals->time + 0.1;
	m_flNextHunt = gpGlobals->time + 1E6;

	
 //easyForcePrintLine("WHAT AM I??? %.2f + %.2f = %.2f", gpGlobals->time, 1E6, gpGlobals->time + 1E6);

	pev->flags |= FL_MONSTER;
	pev->takedamage		= DAMAGE_AIM;
	pev->health			= gSkillData.snarkHealth;
	pev->gravity		= 0.5;
	pev->friction		= 0.5;

	pev->dmg = gSkillData.snarkDmgPop;

	m_flDie = gpGlobals->time + SQUEEK_DETONATE_DELAY;

	m_flFieldOfView = 0; // 180 degrees

	if ( pev->owner )
		m_hOwner = Instance( pev->owner );

	m_flNextBounceSoundTime = gpGlobals->time;// reset each time a snark is spawned.

	pev->sequence = WSQUEAK_RUN;
	ResetSequenceInfo( );
	
}

extern int global_useSentenceSave;
void CSqueakGrenade::Precache( void )
{
	PRECACHE_MODEL("models/w_squeak.mdl");
	//nevermind this, see "precacheAll" in util.cpp for more info.
	//global_useSentenceSave = TRUE;
	PRECACHE_SOUND("squeek/sqk_blast1.wav");
	PRECACHE_SOUND("common/bodysplat.wav", TRUE);
	PRECACHE_SOUND("squeek/sqk_die1.wav");
	PRECACHE_SOUND("squeek/sqk_hunt1.wav");
	PRECACHE_SOUND("squeek/sqk_hunt2.wav");
	PRECACHE_SOUND("squeek/sqk_hunt3.wav");
	PRECACHE_SOUND("squeek/sqk_deploy1.wav");
	//global_useSentenceSave = FALSE;
}



void CSqueakGrenade::setModel(void){
	CSqueakGrenade::setModel(NULL);
}
void CSqueakGrenade::setModel(const char* m){
	//MODDD - class change.  Was CGrenade.
	CBaseMonster::setModel(m);

	//no blinking expected here, too tiny & jumpy for that to be worthwhile.

}//END OF setModel


GENERATE_KILLED_IMPLEMENTATION(CSqueakGrenade){
	pev->model = iStringNull;// make invisible
	SetThink( &CBaseEntity::SUB_Remove );
	SetTouch( NULL );
	pev->nextthink = gpGlobals->time + 0.1;

	// since squeak grenades never leave a body behind, clear out their takedamage now.
	// Squeaks do a bit of radius damage when they pop, and that radius damage will
	// continue to call this function unless we acknowledge the Squeak's death now. (sjb)
	pev->takedamage = DAMAGE_NO;

	// play squeek blast
	UTIL_PlaySound(ENT(pev), CHAN_ITEM, "squeek/sqk_blast1.wav", 1, 0.5, 0, PITCH_NORM);	

	CSoundEnt::InsertSound ( bits_SOUND_COMBAT, pev->origin, SMALL_EXPLOSION_VOLUME, 3.0 );



	//MODDD - does this still look ok with alpha blood instead of retail?  Can fancy it up a bit in that case if needed
	// ...yep.
	//UTIL_SpawnBlood(pev->origin, g_vecZero, BloodColor(), 80);
	if (UTIL_ShouldShowBlood(BloodColor())) {
		if (EASY_CVAR_GET(sv_bloodparticlemode) == 1 || EASY_CVAR_GET(sv_bloodparticlemode) == 2) {
			// alpha effect
			UTIL_BloodStream(pev->origin + Vector(RANDOM_FLOAT(-4.5,4.5), RANDOM_FLOAT(-4.5, 4.5), RANDOM_FLOAT(7, 14)), UTIL_RandomBloodVectorHigh(), BloodColor(), RANDOM_FLOAT(16, 22));
			UTIL_BloodStream(pev->origin + Vector(RANDOM_FLOAT(-4.5, 4.5), RANDOM_FLOAT(-4.5, 4.5), RANDOM_FLOAT(7, 14)), UTIL_RandomBloodVectorHigh(), BloodColor(), RANDOM_FLOAT(16, 22));
			UTIL_BloodStream(pev->origin + Vector(RANDOM_FLOAT(-4.5,4.5), RANDOM_FLOAT(-4.5, 4.5), RANDOM_FLOAT(7, 14)), UTIL_RandomBloodVectorHigh(), BloodColor(), RANDOM_FLOAT(16, 22));
		}
		if (EASY_CVAR_GET(sv_bloodparticlemode) == 0 || EASY_CVAR_GET(sv_bloodparticlemode) == 2) {
			// retail effect
			UTIL_BloodDrips(pev->origin, g_vecZero, BloodColor(), 80);
		}
	}//END OF should show blood check
	

	if (m_hOwner != NULL)
		RadiusDamageAutoRadius ( pev, m_hOwner->pev, pev->dmg, CLASS_NONE, DMG_BLAST );
	else
		RadiusDamageAutoRadius ( pev, pev, pev->dmg, CLASS_NONE, DMG_BLAST );

	// reset owner so death message happens
	if (m_hOwner != NULL)
		pev->owner = m_hOwner->edict();

		
	//is calling through the direct parent CBaseGrenade instead of CBaseMonster okay?
	//YES. Because we don't want this monster to blow up like a grenade does when "Killed". CBaseMonster lacks the unwanted behavior
	//Grenade has (giant explosion here for some reason), even if it is less direct of a parent.
	GENERATE_KILLED_PARENT_CALL(CBaseMonster);
}

GENERATE_GIBMONSTER_IMPLEMENTATION(CSqueakGrenade){
	UTIL_PlaySound(ENT(pev), CHAN_VOICE, "common/bodysplat.wav", 0.75, ATTN_NORM, 0, 200, FALSE);		
}

	
BOOL CSqueakGrenade::isOrganic(void){
	//an exception. Most grenades are not organic.
	return TRUE;
}

float CSqueakGrenade::massInfluence(void){
	return 0.20f;
}//END OF massInfluence
int CSqueakGrenade::GetProjectileType(void){
	return PROJECTILE_ORGANIC_HOSTILE;
}



Vector CSqueakGrenade::GetVelocityLogical(void){
	return pev->velocity;
}

void CSqueakGrenade::SetVelocityLogical(const Vector& arg_newVelocity){
	pev->velocity = arg_newVelocity;
}

void CSqueakGrenade::OnDeflected(CBaseEntity* arg_entDeflector){
	m_hOwner = arg_entDeflector;
}




void CSqueakGrenade::HuntThink( void )
{
	// ALERT( at_console, "think\n" );

	if (!IsInWorld())
	{
		SetTouch( NULL );
		UTIL_Remove( this );
		return;
	}
	
	StudioFrameAdvance_SIMPLE( );
	pev->nextthink = gpGlobals->time + 0.1;

	// explode when ready
	if (gpGlobals->time >= m_flDie)
	{
		g_vecAttackDir = pev->velocity.Normalize( );
		pev->health = -1;
		Killed( pev, 0 );
		return;
	}

	// float
	if (pev->waterlevel != 0)
	{
		if (pev->movetype == MOVETYPE_BOUNCE)
		{
			pev->movetype = MOVETYPE_FLY;
		}
		pev->velocity = pev->velocity * 0.9;
		pev->velocity.z += 8.0;
	}
	//MODDD - WOA THERE.  A mistake from the as-is dev's?  Now that's a treat.
	//Effectively this always sets pev->movetype TO MOVETYPE_FLOAT since it's a single equals (assignment, not a condition or check).
	//And since the result,  pev->movetype is not 0, it passes.
	//In other wors, the is the same as just having only "else".  The movetype is changed 100% of the time this else is reached at all.
	//...but it's also the only other choice and setting to the same MOVETYPE_BOUNCE when it's already that doesn't change anything.
	//else if (pev->movetype = MOVETYPE_FLY)
	else
	{
		pev->movetype = MOVETYPE_BOUNCE;
	}

	// return if not time to hunt
	if (m_flNextHunt > gpGlobals->time)
		return;

	m_flNextHunt = gpGlobals->time + 2.0;
	
	CBaseEntity *pOther = NULL;
	Vector vecDir;
	TraceResult tr;

	Vector vecFlat = pev->velocity;
	vecFlat.z = 0;
	vecFlat = vecFlat.Normalize( );

	UTIL_MakeVectors( pev->angles );

	//MODDD - use the better check.
	//if (m_hEnemy == NULL || !m_hEnemy->IsAlive())
	if(m_hEnemy == NULL || !m_hEnemy->IsAlive_FromAI(this))
	{
		// find target, bounce a bit towards it.
		Look( 512 );
		m_hEnemy = BestVisibleEnemy( );
	}

	// squeek if it's about time blow up
	if ((m_flDie - gpGlobals->time <= 0.5) && (m_flDie - gpGlobals->time >= 0.3))
	{
		UTIL_PlaySound(ENT(pev), CHAN_VOICE, "squeek/sqk_die1.wav", 1, ATTN_NORM, 0, 100 + RANDOM_LONG(0,0x3F));
		CSoundEnt::InsertSound ( bits_SOUND_COMBAT, pev->origin, 256, 0.25 );
	}

	// higher pitch as squeeker gets closer to detonation time
	float flpitch = 155.0 - 60.0 * ((m_flDie - gpGlobals->time) / SQUEEK_DETONATE_DELAY);
	if (flpitch < 80)
		flpitch = 80;

	if (m_hEnemy != NULL)
	{
		if (FVisible( m_hEnemy ))
		{
			vecDir = m_hEnemy->EyePosition() - pev->origin;
			m_vecTarget = vecDir.Normalize( );
		}

		float flVel = pev->velocity.Length();
		float flAdj = 50.0 / (flVel + 10.0);

		if (flAdj > 1.2)
			flAdj = 1.2;
		
		// ALERT( at_console, "think : enemy\n");

		// ALERT( at_console, "%.0f %.2f %.2f %.2f\n", flVel, m_vecTarget.x, m_vecTarget.y, m_vecTarget.z );

		pev->velocity = pev->velocity * flAdj + m_vecTarget * 300;
	}

	if (pev->flags & FL_ONGROUND)
	{
		pev->avelocity = Vector( 0, 0, 0 );
	}
	else
	{
		if (pev->avelocity == Vector( 0, 0, 0))
		{
			pev->avelocity.x = RANDOM_FLOAT( -100, 100 );
			pev->avelocity.z = RANDOM_FLOAT( -100, 100 );
		}
	}

	if ((pev->origin - m_posPrev).Length() < 1.0)
	{
		pev->velocity.x = RANDOM_FLOAT( -100, 100 );
		pev->velocity.y = RANDOM_FLOAT( -100, 100 );
	}
	m_posPrev = pev->origin;

	//IMPORTANT.  This forces the snark to look in the direction it is moving.
	pev->angles = UTIL_VecToAngles( pev->velocity );
	pev->angles.z = 0;
	pev->angles.x = 0;
}


void CSqueakGrenade::SuperBounceTouch( CBaseEntity *pOther )
{
	float flpitch;

	TraceResult tr = UTIL_GetGlobalTrace( );

	// don't hit the guy that launched this grenade
	if ( pev->owner && pOther->edict() == pev->owner )
		return;

	// at least until we've bounced once
	pev->owner = NULL;

	pev->angles.x = 0;
	pev->angles.z = 0;

	// avoid bouncing too much
	if (m_flNextHit > gpGlobals->time)
		return;

	// higher pitch as squeeker gets closer to detonation time
	flpitch = 155.0 - 60.0 * ((m_flDie - gpGlobals->time) / SQUEEK_DETONATE_DELAY);

	if ( pOther->pev->takedamage && m_flNextAttack < gpGlobals->time )
	{
		// attack!

		// make sure it's me who has touched them
		if (tr.pHit == pOther->edict())
		{
			// and it's not another squeakgrenade
			if (tr.pHit->v.modelindex != pev->modelindex)
			{
				// ALERT( at_console, "hit enemy\n");
				ClearMultiDamage( );
				pOther->TraceAttack(pev, gSkillData.snarkDmgBite, gpGlobals->v_forward, &tr, DMG_SLASH ); 
				if (m_hOwner != NULL)
					ApplyMultiDamage( pev, m_hOwner->pev );
				else
					ApplyMultiDamage( pev, pev );

				pev->dmg += gSkillData.snarkDmgPop; // add more explosion damage
				// m_flDie += 2.0; // add more life

				// make bite sound
				UTIL_PlaySound(ENT(pev), CHAN_WEAPON, "squeek/sqk_deploy1.wav", 1.0, ATTN_NORM, 0, (int)flpitch);
				m_flNextAttack = gpGlobals->time + 0.5;
			}
		}
		else
		{
			// ALERT( at_console, "been hit\n");
		}
	}

	m_flNextHit = gpGlobals->time + 0.1;
	m_flNextHunt = gpGlobals->time;

	if ( IsMultiplayer() )
	{
		// in multiplayer, we limit how often snarks can make their bounce sounds to prevent overflows.
		if ( gpGlobals->time < m_flNextBounceSoundTime )
		{
			// too soon!
			return;
		}
	}

	if (!(pev->flags & FL_ONGROUND))
	{
		// play bounce sound
		float flRndSound = RANDOM_FLOAT ( 0 , 1 );

		if ( flRndSound <= 0.33 )
			UTIL_PlaySound(ENT(pev), CHAN_VOICE, "squeek/sqk_hunt1.wav", 1, ATTN_NORM, 0, (int)flpitch);		
		else if (flRndSound <= 0.66)
			UTIL_PlaySound(ENT(pev), CHAN_VOICE, "squeek/sqk_hunt2.wav", 1, ATTN_NORM, 0, (int)flpitch);
		else 
			UTIL_PlaySound(ENT(pev), CHAN_VOICE, "squeek/sqk_hunt3.wav", 1, ATTN_NORM, 0, (int)flpitch);
		CSoundEnt::InsertSound ( bits_SOUND_COMBAT, pev->origin, 256, 0.25 );
	}
	else
	{
		// skittering sound
		CSoundEnt::InsertSound ( bits_SOUND_COMBAT, pev->origin, 100, 0.1 );
	}

	m_flNextBounceSoundTime = gpGlobals->time + 0.5;// half second.
}
#endif





LINK_ENTITY_TO_CLASS( weapon_snark, CSqueak );


#ifndef CLIENT_DLL
TYPEDESCRIPTION	CSqueak::m_SaveData[] =
{
	DEFINE_FIELD( CSqueak, m_chargeReady, FIELD_INTEGER ),
};
IMPLEMENT_SAVERESTORE(CSqueak, CBasePlayerWeapon);
#endif





//CSQueak, the player weapon
int CSqueak::numberOfEyeSkins = -1;


CSqueak::CSqueak(){

	m_flReleaseThrow = -1;
	m_chargeReady &= ~32;

}


//MODDD
void CSqueak::customAttachToPlayer(CBasePlayer *pPlayer ){
	m_pPlayer->SetSuitUpdate("!HEV_SQUEEK", FALSE, SUIT_NEXT_IN_30SEC);
}



const char* CSqueak::GetPickupWalkerName(void){
	return "monster_snarkpickupwalker";
}

void CSqueak::Spawn( )
{
	//pev->classname = MAKE_STRING("weapon_snark");

	if(pickupWalkerReplaceCheck()){
		return;
	}

	Precache( );
	m_iId = WEAPON_SNARK;
	setModel("models/w_sqknest.mdl");

	//NOTICE - if we want to fall then replace, best make a clone of the FallInit method that goes for a "FallInitReplace" think method for replacing itself on hitting the ground.
	//for now, it will be instant if a replacement is decided. The replacement monster snaps to the ground anyways.
	FallInit();//get ready to fall down.

	m_iClip = -1;
	m_iDefaultAmmo = SNARK_DEFAULT_GIVE;
		
	pev->sequence = 1;
	pev->animtime = gpGlobals->time;
	pev->framerate = 1.0;
}


//Player weapon, no soundsentencesave.
void CSqueak::Precache( void )
{
	PRECACHE_MODEL("models/w_sqknest.mdl");
	PRECACHE_MODEL("models/v_squeak.mdl");
	PRECACHE_MODEL("models/p_squeak.mdl");
	PRECACHE_SOUND("squeek/sqk_hunt2.wav");
	PRECACHE_SOUND("squeek/sqk_hunt3.wav");

	//just in case.
	precacheGunPickupSound();
	precacheAmmoPickupSound();

	UTIL_PrecacheOther("monster_snark");

	m_usSnarkFire = PRECACHE_EVENT ( 1, "events/snarkfire.sc" );

}

//MODDD - for first-person player models, this won't work like you think it does.
void CSqueak::setModel(void){
	CSqueak::setModel(NULL);
}
void CSqueak::setModel(const char* m){
	CBasePlayerWeapon::setModel(m);

	/*
	//MODDD - IMPORTANT NOTE.
	// doing blinks here doesn't do much.  See hl_weapons.cpp where "numberOfEyeSkins" is used.
	// The counts are hardcoded there, unsure if "getNumberOfSkins" over there would work.
	
	if(numberOfEyeSkins == -1){
		//never loaded numberOfSkins? Do so.
		numberOfEyeSkins = getNumberOfSkins();
		//EASY_CVAR_PRINTIF_PRE(houndeyePrintout, easyPrintLine( "HOUND: SKINCOUNTPOST1: %d %d", numberOfSkins, getNumberOfSkins( ) );
		//EASY_CVAR_PRINTIF_PRE(houndeyePrintout, easyPrintLine( "HOUND: BODYCOUNTPOST1: %d", this->getNumberOfBodyParts( ) );

		if(numberOfEyeSkins == 0){
			easyPrintLine( "WARNING: Squeak skin count is 0, error! Check v_squeak.mdl for multiple skins. If it has them, please report this.  Forcing default of 3...");
			numberOfEyeSkins = 3;
		}else if(numberOfEyeSkins != 3){
			easyPrintLine( "WARNING: Squeak skin count is %d, not 3. If v_squeak.mdl does have 3 skins, please report this.", numberOfEyeSkins);
			if(numberOfEyeSkins < 1) numberOfEyeSkins = 1; //safety.
		}
	}
	*/

}//END OF setModel


int CSqueak::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "Snarks";
	p->iMaxAmmo1 = SNARK_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;


#if SPLIT_ALIEN_WEAPONS_INTO_NEW_SLOT != 1
	p->iSlot = 4;
	p->iPosition = 3;
#else
	// to the new slot you go!
	p->iSlot = 5;
	p->iPosition = 1;
#endif

	p->iId = m_iId = WEAPON_SNARK;
	p->iWeight = SNARK_WEIGHT;
	p->iFlags = ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE;

	return 1;
}



BOOL CSqueak::Deploy( )
{

	m_flReleaseThrow = -1;
	m_chargeReady &= ~32;

	// play hunt sound
	float flRndSound = RANDOM_FLOAT ( 0 , 1 );

	if(!globalflag_muteDeploySound){
		if ( flRndSound <= 0.5 ){
			UTIL_PlaySound(ENT(pev), CHAN_VOICE, "squeek/sqk_hunt2.wav", 1, ATTN_NORM, 0, 100);
		}else{ 
			UTIL_PlaySound(ENT(pev), CHAN_VOICE, "squeek/sqk_hunt3.wav", 1, ATTN_NORM, 0, 100);
		}
		m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;
	}


	return DefaultDeploy( "models/v_squeak.mdl", "models/p_squeak.mdl", SQUEAK_UP, "squeak", 0, 0, (51.0 / 30.0), (0.6) );
}


void CSqueak::Holster( int skiplocal /* = 0 */ )
{
	//m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	

	m_flReleaseThrow = -1;
	m_chargeReady &= ~32;


	if (PlayerPrimaryAmmoCount() <= 0)
	{
		m_pPlayer->pev->weapons &= ~(1<<WEAPON_SNARK);
		SetThink( &CBasePlayerItem::DestroyItem );
		pev->nextthink = gpGlobals->time + 0.1;
		
	}else{
		//SendWeaponAnim( SQUEAK_DOWN );
		DefaultHolster(SQUEAK_DOWN, skiplocal, 0, (21.0f/16.0f));
	}
	
	
	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "common/null.wav", 1.0, ATTN_NORM);
}

float CSqueak::randomIdleAnimationDelay(){
	return 0; //no static delays for me.
}











void CSqueak::PrimaryAttack()
{
	//MODDD - animation has a delay before the visible snark is thrown, show it.

	// time of snark throw:  31/30   (time in the animation the snark is out of the hand)
	
	if (PlayerPrimaryAmmoCount() <= 0){
		// forget it
		return;
	}



	if (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelay) == 0) {
		// nothing unusual
	}
	else {
		// RAPID FIRE MODE
#ifndef CLIENT_DLL
		Throw();
#endif
		SendWeaponAnimBypass(SQUEAK_UP);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (52.0 / 30.0) + randomIdleAnimationDelay();

		SetAttackDelays(UTIL_WeaponTimeBase() + EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelaycustom) + 0.023f);

		m_flReleaseThrow = -1;
		m_chargeReady &= ~32;
		return;
	}



//#ifndef CLIENT_DLL
	//if (m_flReleaseThrow == -1) {
	// Nevermind this check, throwing already sets m_flNextPrimaryAttack which blocks interrupting a throw
		// start throw only
		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		TraceResult tr;
		
		// HACK HACK:  Ugly hacks to handle change in origin based on new physics code for players
		// Move origin up if crouched and start trace a bit outside of body ( 20 units instead of 16 )
		////////////////////////////////////////
		// MODDD -   ...  okay but why?  The center is shifted a bit lower during a duck which seems appropriate.
		//           Commenting out the duck check, maybe this is from some older state of physics/engine development
		//           when this used to make sense.
		Vector trace_origin;
		trace_origin = m_pPlayer->pev->origin;
		/*
		if ( m_pPlayer->pev->flags & FL_DUCKING )
		{
			trace_origin = trace_origin - ( VEC_HULL_MIN - VEC_DUCK_HULL_MIN );
		}
		*/
		
		int flags;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif
		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usSnarkFire, 0.0, (float*)&g_vecZero, (float*)&g_vecZero, 0.0, 0.0, 0, 0, 0, 0);


		//MODDD - why even bother doing this clientside, UTIL_TraceLine is dummied clientside, always
		// returns 'nothing in the way' no matter the start/endpoint.
		// even though ev_hldm's isn't, and can detect monsters...?
		//     er.   Why was this UTIL_TraceLine dummied clientside again.
#ifndef CLIENT_DLL
		// find place to toss monster
		UTIL_TraceLine(trace_origin + gpGlobals->v_forward * 20, trace_origin + gpGlobals->v_forward * 64, dont_ignore_monsters, NULL, &tr);

		//
		if (tr.fAllSolid == 0 && tr.fStartSolid == 0 && tr.flFraction > 0.25)
		{
			// success?  Begin actual snark-generation delay
			m_flReleaseThrow = gpGlobals->time + (31.0f / 30.0f);



			// player "shoot" animation
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);

			m_chargeReady |= 32;

			//MODDD 
			if (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelay) == 0) {
				SetAttackDelays(UTIL_WeaponTimeBase() + (31.0f / 30.0f) + 0.8f);
			}
			else {
				SetAttackDelays(UTIL_WeaponTimeBase() + EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelaycustom) + 0.015f);
			}
			//NOTE: this ends up being the delay before doing the re-draw animation
			// (can still fire before then, unaffected by the time of the "throw" animation that hides the hands)
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (44.0 / 30.0) + -0.1f;

		}// END OF trace success (nothing in the way) check
#endif

	//}// END OF m_flReleaseThrow check... maybe
//#endif

}//PrimaryAttack


void CSqueak::SecondaryAttack( void )
{
	//MODDD - fidget on demand?

	if (EASY_CVAR_GET(cl_viewmodel_fidget) == 2) {
		float flRand;
		int iAnim;
		
		flRand = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 0, 1);
		if (flRand <= 0.5)
		{
			iAnim = SQUEAK_FIDGETFIT;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 70.0 / 16.0;
		}
		else
		{
			iAnim = SQUEAK_FIDGETNIP;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 80.0 / 16.0;
		}
		//SetAttackDelays(m_flTimeWeaponIdle);
		m_flNextSecondaryAttack = m_flTimeWeaponIdle;
		m_flTimeWeaponIdle += randomIdleAnimationDelay();
		SendWeaponAnim(iAnim);
	}//CVar check
	
	
}//SecondaryAttack


// should only be called by the server.
void CSqueak::Throw(void) {

	if (PlayerPrimaryAmmoCount() <= 0) {
		// forget it
		return;
	}


	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	TraceResult tr;

	Vector trace_origin;
	trace_origin = m_pPlayer->pev->origin;
	/*
	if (m_pPlayer->pev->flags & FL_DUCKING)
	{
		trace_origin = trace_origin - (VEC_HULL_MIN - VEC_DUCK_HULL_MIN);
	}
	*/

		
#ifndef CLIENT_DLL
	// find place to toss monster
	UTIL_TraceLine(trace_origin + gpGlobals->v_forward * 20, trace_origin + gpGlobals->v_forward * 64, dont_ignore_monsters, NULL, &tr);

	//
	if (tr.fAllSolid == 0 && tr.fStartSolid == 0 && tr.flFraction > 0.25)
	{
			
		// hence cometh thy creature.  or some shit.
#ifndef CLIENT_DLL
		//Doesn't make a difference to send the SF_MONSTER_THROWN spawn flag here since snarks don't have the default monster behavior of snapping
		//to the ground at spawn, but it doesn't hurt.
		CBaseEntity* pSqueak = CBaseEntity::Create("monster_snark", tr.vecEndPos, m_pPlayer->pev->v_angle, SF_MONSTER_THROWN, m_pPlayer->edict());

		//MODDD - now the generated snark inherits a factor of the player's velocity instead of all of it, such as 12%. Set by CVar.
		pSqueak->pev->velocity = gpGlobals->v_forward * 200 + UTIL_GetProjectileVelocityExtra(m_pPlayer->pev->velocity, EASY_CVAR_GET_DEBUGONLY(snarkInheritsPlayerVelocity));
#endif

		// play hunt sound
		float flRndSound = RANDOM_FLOAT(0, 1);

		if (flRndSound <= 0.5)
			UTIL_PlaySound(ENT(pev), CHAN_VOICE, "squeek/sqk_hunt2.wav", 1, ATTN_NORM, 0, 105);
		else
			UTIL_PlaySound(ENT(pev), CHAN_VOICE, "squeek/sqk_hunt3.wav", 1, ATTN_NORM, 0, 105);

		//MODDD - little more radius to alert things on a throw, it is a 'squeak' after all.
		// Normal gun volume is 600.
		m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME + 200;

		//MODDD - cheat check
		if (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteclip) == 0 && EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteammo) == 0) {
			ChangePlayerPrimaryAmmoCount(-1);
		}

	}// END OF trace success (nothing in the way) check
	else {
		// But if that failed, don't finish the throw animation.  Looks a little odd to do that.
		// Just force a re-deploy this instant.
		SendWeaponAnimBypass(SQUEAK_UP);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (52.0 / 30.0) + randomIdleAnimationDelay();

		m_flReleaseThrow = -1;
		m_chargeReady &= ~32;
	}


#endif

//#endif


}





void CSqueak::ItemPostFrame(void){
	
	//MODDD - nope.
	/*
	if ( ( pev->skin == 0 ) && RANDOM_LONG(0,127) == 0 )
	{// start blinking!

		//MODDD - no, HOUNDEYE_EYE_FRAMES - 1 (3) just gives an open eye again.  - 2 is good.
		pev->skin = max(numberOfEyeSkins - 1, 0);
		//pev->skin = 2;
	}
	else if ( pev->skin > 0 )
	{// already blinking
		pev->skin--;
	}
	*/

	CBasePlayerWeapon::ItemPostFrame();

}//END OF ItemPostFrame



void CSqueak::ItemPostFrameThink(void) {

	// Moved here to not require no keys to be held down (WeaponIdle does).
	if (m_chargeReady & 32 && m_flTimeWeaponIdle <= UTIL_WeaponTimeBase())
	{
		m_flReleaseThrow = -1;
		m_chargeReady &= ~32;

		if (PlayerPrimaryAmmoCount() <= 0)
		{
			RetireWeapon();
			return;
		}
		SendWeaponAnimBypass(SQUEAK_UP);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (52.0 / 30.0) + randomIdleAnimationDelay();
		return;
	}


#ifndef CLIENT_DLL
	// Queued up a throw?  Let's go then
	if (m_flReleaseThrow != -1 && gpGlobals->time >= m_flReleaseThrow) {
		m_flReleaseThrow = -1;
		Throw();
	}
#endif


	CBasePlayerWeapon::ItemPostFrameThink();
}//ItemPostFrameThink






void CSqueak::WeaponIdle( void )
{

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

	if (PlayerPrimaryAmmoCount() <= 0) {
		// no.  No idle delays, they just make ammo pickups take longer to take effect with the empty viewmodel still up
		return;
	}


	//Now that there isn't a static delay for living throwables, the odds of a unique idle anim have been slightly reduced.
	//Old ones were:
	//      if  flRand <= 0.75
	//... else  flRand <= 0.875

	int iAnim;
	float flRand;

	if (EASY_CVAR_GET(cl_viewmodel_fidget) == 1) {
		flRand = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 0, 1);
	}
	else {
		// never play others this way.
		flRand = 0;
	}

	if (flRand <= 0.82)
	{
		iAnim = SQUEAK_IDLE1;
		//MODDD - don't double length, already matches anim time.
		//m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 30.0 / 16 * (2);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 30.0 / 16 + randomIdleAnimationDelay();
	}
	else if (flRand <= 0.82 + 0.09)
	{
		iAnim = SQUEAK_FIDGETFIT;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 70.0 / 16.0 + randomIdleAnimationDelay();
	}
	else
	{
		iAnim = SQUEAK_FIDGETNIP;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 80.0 / 16.0 + randomIdleAnimationDelay();
	}
	SendWeaponAnim( iAnim );
}


CSqueak_NoReplace::CSqueak_NoReplace() {

}//END OF constructor

LINK_ENTITY_TO_CLASS(weapon_snark_noreplace, CSqueak_NoReplace);


void CSqueak_NoReplace::Spawn(void) {
	CSqueak::Spawn();

	//after that call, may as well lose the "_noreplace" bit.
	pev->classname = MAKE_STRING("weapon_snark");
}


