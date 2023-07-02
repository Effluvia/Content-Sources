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

===== generic grenade.cpp ========================================================

*/
// WARNING!  Like weapons.cpp, this too is not a shared file.  Beware!

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "basemonster.h"
#include "weapons.h"
#include "nodes.h"
#include "soundent.h"
#include "decals.h"
#include "util_debugdraw.h"

//===================grenade

EASY_CVAR_EXTERN(cl_explosion)
EASY_CVAR_EXTERN_DEBUGONLY(explosionDebrisSoundVolume)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_touchNeverExplodes)
EASY_CVAR_EXTERN_DEBUGONLY(handGrenadesUseOldBounceSound)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(trailTypeTest)


// Grenades flagged with this will be triggered when the owner calls detonateSatchelCharges
#define SF_DETONATE		0x0001

//MODDD
#define ROCKET_TRAIL 2

extern unsigned short g_sTrail;
extern unsigned short g_sTrailRA;
extern unsigned short g_quakeExplosionEffect;
extern unsigned short model_explosivestuff;


LINK_ENTITY_TO_CLASS(grenade, CGrenade);




CGrenade::CGrenade(void) {
	dropped = FALSE;
	firstGroundContactYet = FALSE;
	nextBounceSoundAllowed = -1;
}




GENERATE_TRACEATTACK_IMPLEMENTATION(CGrenade)
{
	//MODDD - class update, was CBaseMonster.
	GENERATE_TRACEATTACK_PARENT_CALL(CBaseAnimating);
}

GENERATE_TAKEDAMAGE_IMPLEMENTATION(CGrenade)
{
	//MODDD - class update, was CBaseMonster.
	return GENERATE_TAKEDAMAGE_PARENT_CALL(CBaseAnimating);
}


//
// Grenade Explode
//
//MODDD - method changed, parameters removed.  These aren't even used at all.
/*
void CGrenade::Explode( Vector vecSrc, Vector vecAim )
{
	TraceResult tr;
	UTIL_TraceLine ( pev->origin, pev->origin + Vector ( 0, 0, -32 ),  ignore_monsters, ENT(pev), & tr);

	Explode( &tr, DMG_BLAST );
}
*/

// Assuming we have a pev->dmg to go off of.
void CGrenade::Explode(void)
{
	TraceResult tr;
	UTIL_TraceLine(pev->origin, pev->origin + Vector(0, 0, -32), ignore_monsters, ENT(pev), &tr);

	//CGrenade::  ???
	CGrenade::Explode(&tr, pev->dmg, pev->dmg * 2.5, DMG_BLAST, 0, 1);
}

void CGrenade::Explode(TraceResult* pTrace, int bitsDamageType) {
	CGrenade::Explode(pTrace, pev->dmg, pev->dmg * 2.5, bitsDamageType, 0, 1);
}
void CGrenade::Explode(TraceResult* pTrace, int bitsDamageType, int bitsDamageTypeMod) {
	CGrenade::Explode(pTrace, pev->dmg, pev->dmg * 2.5, bitsDamageType, bitsDamageTypeMod, 1);
}

// Handle the grenade-instance-specific details and call "StaticExplode" for the aspects that don't
// depend on being a CGrenade.
void CGrenade::Explode(TraceResult* pTrace, float rawDamage, float flRange, int bitsDamageType, int bitsDamageTypeMod, float shrapMod)
{
	pev->model = iStringNull;//invisible
	pev->effects |= EF_NODRAW;
	pev->solid = SOLID_NOT;// intangible
	pev->takedamage = DAMAGE_NO;

	entvars_t* pevOwner = NULL;
	if (pev->owner) {
		pevOwner = VARS(pev->owner);
	}
	pev->owner = NULL; // can't traceline attack owner if this is set




	if (UTIL_PointContents(pev->origin) == CONTENTS_SKY) {
		// If we hit the sky, HALT!  No explosion, no sticking out, no sparks.
		// Just end.
	}
	else {
		// The Smoke method soon leads to this entity's deletion. The "StaticExplode" call above already makes it invisible.
		SetThink(&CGrenade::Smoke);
		StaticExplode(pev->origin, rawDamage, flRange, this, pevOwner, pTrace, bitsDamageType, bitsDamageTypeMod, shrapMod);
	}

	pev->velocity = g_vecZero;
	pev->nextthink = gpGlobals->time + 0.3;
}





// Explode like a grenade without a dozen paramters.  Does not remove the "pDamageDealer" entity,
// nor is it exempt from damage.  Just ignored in some effect-related logic.
// This also does not touch the owner of the entity, send along and set to NULL yourself if needed.
// If an owner is not provided, it will be implied to be the same as pDamageDealer itself.
// This is used to determine who dealt the damage.  Otherwise, provide it as VARS(thisEnt->pev->owner)
// to use the linked 'owner' entity.
void SimpleStaticExplode(Vector rawExplodeOrigin, float rawDamage, CBaseEntity* pDamageDealer) {
	SimpleStaticExplode(rawExplodeOrigin, rawDamage, rawDamage * 2.5, pDamageDealer);
}
void SimpleStaticExplode(Vector rawExplodeOrigin, float rawDamage, CBaseEntity* pDamageDealer, entvars_t* entOwner) {
	SimpleStaticExplode(rawExplodeOrigin, rawDamage, rawDamage * 2.5, pDamageDealer, entOwner);
}
void SimpleStaticExplode(Vector rawExplodeOrigin, float rawDamage, float flRange, CBaseEntity* pDamageDealer) {
	entvars_t* entOwner = NULL;
	if (pDamageDealer != NULL) {
		entOwner = pDamageDealer->pev;
	}
	// oh.. it turns out sending entOwner as ourselves is pointless.
	// RadiusDamage already knows to make the one blamed the same as the entity sent (pDamageDealer) in such a case.
	SimpleStaticExplode(rawExplodeOrigin, rawDamage, flRange, pDamageDealer, entOwner);
}

void SimpleStaticExplode(Vector rawExplodeOrigin, float rawDamage, float flRange, CBaseEntity* pDamageDealer, entvars_t* entOwner) {
	edict_t* edThingy = NULL;
	if (pDamageDealer != NULL) {
		edThingy = pDamageDealer->edict();
	}
	TraceResult tr;
	UTIL_TraceLine(rawExplodeOrigin, rawExplodeOrigin + Vector(0, 0, -32), ignore_monsters, edThingy, &tr);
	StaticExplode(rawExplodeOrigin, rawDamage, flRange, pDamageDealer, entOwner, &tr, DMG_BLAST, 0, 1);
}








// variants without range.
void StaticExplode(Vector rawExplodeOrigin, float rawDamage, CBaseEntity * pDamageDealer, entvars_t * entOwner, TraceResult * pTrace, int bitsDamageType) {
	StaticExplode(rawExplodeOrigin, rawDamage, rawDamage * 2.5, pDamageDealer, entOwner, pTrace, bitsDamageType, 0, 1);
}
void StaticExplode(Vector rawExplodeOrigin, float rawDamage, CBaseEntity* pDamageDealer, entvars_t* entOwner, TraceResult* pTrace, int bitsDamageType, int bitsDamageTypeMod) {
	StaticExplode(rawExplodeOrigin, rawDamage, rawDamage * 2.5, pDamageDealer, entOwner, pTrace, bitsDamageType, bitsDamageTypeMod, 1);
}
void StaticExplode(Vector rawExplodeOrigin, float rawDamage, CBaseEntity* pDamageDealer, entvars_t* entOwner, TraceResult* pTrace, int bitsDamageType, int bitsDamageTypeMod, float shrapMod){
	StaticExplode(rawExplodeOrigin, rawDamage, rawDamage * 2.5, pDamageDealer, entOwner, pTrace, bitsDamageType, bitsDamageTypeMod, shrapMod);
}

// variants with range
void StaticExplode(Vector rawExplodeOrigin, float rawDamage, float flRange, CBaseEntity* pDamageDealer, entvars_t* entOwner, TraceResult* pTrace, int bitsDamageType) {
	StaticExplode(rawExplodeOrigin, rawDamage, flRange, pDamageDealer, entOwner, pTrace, bitsDamageType, 0, 1);
}
void StaticExplode(Vector rawExplodeOrigin, float rawDamage, float flRange, CBaseEntity* pDamageDealer, entvars_t* entOwner, TraceResult* pTrace, int bitsDamageType, int bitsDamageTypeMod) {
	StaticExplode(rawExplodeOrigin, rawDamage, flRange, pDamageDealer, entOwner, pTrace, bitsDamageType, bitsDamageTypeMod, 1);
}

void StaticExplode(Vector rawExplodeOrigin, float rawDamage, float flRange, CBaseEntity* pDamageDealer, entvars_t* entOwner, TraceResult* pTrace, int bitsDamageType, int bitsDamageTypeMod, float shrapMod){
	float flRndSound;// sound randomizer
	//MODDD - new
	Vector explosionEffectStart = rawExplodeOrigin;
	//also retail's origin.
	Vector explosionOrigin = rawExplodeOrigin; //by default.
	//origin to do the explosion logic for, not necessiarly where the effect is spawned.
	Vector explosionLogicOrigin = rawExplodeOrigin;  //same.

	/*
	edict_t* ownerMem = NULL;
	if (pev->owner != NULL) {
		//before we do this trace, we must drop the owner.  Restore it afterwards in case that matters
		//for some other behavior.
		//During a trace, an entity ignored (this->edict()) also indicates to ignore its pev->owner if it has one.
		//This implied behavior will tear every hair out of your head.
		//const char* ownerClassname = STRING(pev->owner->v.classname);

		ownerMem = pev->owner;
		pev->owner = NULL;
	}
	*/

	entvars_t* pevThingy = NULL;
	edict_t* edictThingy = NULL;
	
	if (pDamageDealer != NULL) {
		pevThingy = pDamageDealer->pev;
		edictThingy = pDamageDealer->edict();
	}

	// Pull out of the wall a bit
	if (pTrace->flFraction != 1.0)
	{
		TraceResult trToEffectOrigin;

		//MODDD - let's not change our own origin, just record this.
		//pev->origin = ...
		explosionOrigin = pTrace->vecEndPos + (pTrace->vecPlaneNormal * (rawDamage - 24) * 0.6);;

		//MODDD - used for placing the quake explosion effect, if it is called for instead.
		explosionEffectStart = pTrace->vecEndPos + (pTrace->vecPlaneNormal * 5);

		//MODDD -Check. Is there a straight line, unobstructed, from the surface to the explosionEffectStart?
		Vector vecCheckStart;
		//vecCheckStart = pTrace->vecEndPos + (pTrace->vecPlaneNormal * 0.1f);
		vecCheckStart = pTrace->vecEndPos;
		//and start just a little off from the pTrace->vecEndPos as to not collide with that surface itself. Just safety.
		//...looks like we don't need to even do that.


		UTIL_TraceLine(vecCheckStart, explosionOrigin, dont_ignore_monsters, edictThingy, &trToEffectOrigin);

		if (trToEffectOrigin.fStartSolid || trToEffectOrigin.fAllSolid || trToEffectOrigin.flFraction < 1.0f) {
			//if there was any problem making it over, the logic origin should be vecCheckStart.
			float distanceThatMadeItA = (trToEffectOrigin.vecEndPos - vecCheckStart).Length();
			float distanceThatMadeItB = (explosionOrigin - vecCheckStart).Length() * trToEffectOrigin.flFraction;
			explosionLogicOrigin = vecCheckStart;
		}
		else {
			//no problems? the retail behavior, same offset from the surface for RadiusDamage logic to start, is fine.
			explosionLogicOrigin = explosionOrigin;
		}
		//::DebugLine_Setup(7, vecCheckStart, explosionOrigin, trToEffectOrigin.flFraction);

	}//END OF surface hit check (pTrace)

	//is this change from pev->origin to explosionOrigin ok?
	int iContents = UTIL_PointContents(explosionOrigin);
	short spriteChosen;
	if (iContents != CONTENTS_WATER){
		spriteChosen = g_sModelIndexFireball;
	}
	else{
		spriteChosen = g_sModelIndexWExplosion;
	}

	//MODDD - condensed that a bit. Also sending the "explosionOrigin in place of our rawExplodeOrigin.
	UTIL_Explosion(MSG_PAS, rawExplodeOrigin, NULL, pevThingy, explosionOrigin, spriteChosen, (rawDamage - 50) * 0.60, 15, TE_EXPLFLAG_NONE, explosionEffectStart, shrapMod);

	CSoundEnt::InsertSound(bits_SOUND_COMBAT, explosionOrigin, NORMAL_EXPLOSION_VOLUME, 3.0);

	//MODDD - sending explosionOrigin instead of defaulting to pev->origin.
	//RadiusDamageAutoRadius ( explosionOrigin, pev, pevOwner, pev->dmg, CLASS_NONE, bitsDamageType, bitsDamageTypeMod );

	//MODDD - and why were we still using explosionOrigin for the phyiscal damage spot all this time anyhow?
	//...no, use the new explosionLogicOrigin instead.  If there is space out from the surface hit by the explosion, this records the same origin as the retail effect.
	//But if something is in the way of even that, like a player firing a grenade while solidly against a crate, we don't want the player to block their own explosion
	//from the crate because they thesmelves were in the way to block the explosion origin that got pushed behind.  That's... okay for the visible effect but not
	//the explosion logic origin for doing radial damage.  It can't be blocked like that.
	//RadiusDamage( explosionOrigin, pev, pevOwner, pev->dmg, pev->dmg * 2.5, CLASS_NONE, bitsDamageType, bitsDamageTypeMod );
	RadiusDamage(explosionLogicOrigin, pevThingy, entOwner, rawDamage, flRange, CLASS_NONE, bitsDamageType, bitsDamageTypeMod);


	if (RANDOM_FLOAT(0, 1) < 0.5){
		UTIL_DecalTrace(pTrace, DECAL_SCORCH1);
	}
	else{
		UTIL_DecalTrace(pTrace, DECAL_SCORCH2);
	}

	flRndSound = RANDOM_FLOAT(0, 1);


	//randDebrisSound = 1;
	//easyPrintLine("DEBRIS SOUND: %d", randDebrisSound);

	if (EASY_CVAR_GET_DEBUGONLY(explosionDebrisSoundVolume) > 0) {
		int randDebrisSound = RANDOM_LONG(0, 2);

		float debrisVolumeChoice = UTIL_clamp(EASY_CVAR_GET_DEBUGONLY(explosionDebrisSoundVolume), 0, 1);

		switch (randDebrisSound)
		{
			//NOTE: volume, the argument after the string-path, used to be 0.55.  Now 0.78.
		case 0:	UTIL_PlaySound(edictThingy, CHAN_VOICE, "weapons/debris1.wav", debrisVolumeChoice, ATTN_NORM, 0, 84, FALSE);	break;
		case 1:	UTIL_PlaySound(edictThingy, CHAN_VOICE, "weapons/debris2.wav", debrisVolumeChoice, ATTN_NORM, 0, 84, FALSE);	break;
		case 2:	UTIL_PlaySound(edictThingy, CHAN_VOICE, "weapons/debris3.wav", debrisVolumeChoice, ATTN_NORM, 0, 84, FALSE);	break;
		}
	}


	//MODDD - only generate sparks if allowed.
	if (UTIL_getExplosionsHaveSparks()) {
		if (iContents != CONTENTS_WATER)
		{
			int sparkCount = RANDOM_LONG(0, 3);
			for (int i = 0; i < sparkCount; i++)
				CBaseEntity::Create("spark_shower", explosionOrigin, pTrace->vecPlaneNormal, NULL);
		}
	}
}//END OF StaticExplode


void CGrenade::Smoke( void )
{

	if (UTIL_PointContents ( pev->origin ) == CONTENTS_WATER)
	{
		UTIL_Bubbles( pev->origin - Vector( 64, 64, 64 ), pev->origin + Vector( 64, 64, 64 ), 100 );
	}
	else
	{
		UTIL_ExplosionSmoke(MSG_PVS, pev->origin, NULL, pev->origin, 0, 0, 0, g_sModelIndexSmoke,  (pev->dmg - 50) * 0.80, 12);
	}
	UTIL_Remove( this );
}

GENERATE_KILLED_IMPLEMENTATION(CGrenade)
{
	Detonate( );
}

BOOL CGrenade::isOrganic(void){
	// typically not.
	return FALSE;
}

// I do not.  Player weapons don't.
BOOL CGrenade::usesSoundSentenceSave(void){
	return FALSE;
}


// Timed grenade, this think is called when time runs out.
void CGrenade::DetonateUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	SetThink( &CGrenade::Detonate );
	pev->nextthink = gpGlobals->time;
}

void CGrenade::PreDetonate( void )
{
	CSoundEnt::InsertSound ( bits_SOUND_DANGER, pev->origin, 400, 0.3 );

	SetThink( &CGrenade::Detonate );
	pev->nextthink = gpGlobals->time + 1;
}


void CGrenade::Detonate( void )
{
	TraceResult tr;
	Vector		vecSpot;// trace starts here!

	vecSpot = pev->origin + Vector ( 0 , 0 , 8 );
	UTIL_TraceLine ( vecSpot, vecSpot + Vector ( 0, 0, -40 ),  ignore_monsters, ENT(pev), & tr);

	Explode( &tr, DMG_BLAST );
}

//
// Contact grenade, explode when it touches something
// 
void CGrenade::ExplodeTouch( CBaseEntity *pOther )
{

	/*
	// WHO IS OWNER
	CBaseEntity* myOwner = NULL;
	if(pev->owner != NULL){
		myOwner = CBaseEntity::Instance(pev->owner);
	}
	const char* daOwnerClassname;
	if(myOwner != NULL){
		daOwnerClassname = myOwner->getClassname();
	}else{
		daOwnerClassname = "NULL?!";
	}
	const char* daTouchedClassname = pOther->getClassname();
	*/



	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_touchNeverExplodes) != 1){
		TraceResult tr;
		Vector		vecSpot;// trace starts here!

		pev->enemy = pOther->edict();

		//MODDD - just put the explosion exactly at the impact point for now (or maybe just less of an extreme later)
		//REVERTED, not the issue.
		vecSpot = pev->origin - pev->velocity.Normalize() * 32;
		UTIL_TraceLine( vecSpot, vecSpot + pev->velocity.Normalize() * 64, ignore_monsters, ENT(pev), &tr );
		
		//attempted improvement anyhow.
		//vecSpot = pev->origin + pev->velocity.Normalize() * -10;
		//Vector vecSpotEnd = pev->origin + pev->velocity.Normalize() * 10;
		//UTIL_TraceLine( vecSpot, vecSpotEnd, ignore_monsters, ENT(pev), &tr );
		

		Explode( &tr, DMG_BLAST );
		//easyPrintLine("cheat_touchNeverExplodes what??");
	}//END OF CVar check...
	
}


void CGrenade::DangerSoundThink( void )
{
	if (!IsInWorld())
	{
		UTIL_Remove( this );
		return;
	}

	CSoundEnt::InsertSound ( bits_SOUND_DANGER, pev->origin + pev->velocity * 0.5, pev->velocity.Length( ), 0.2 );
	pev->nextthink = gpGlobals->time + 0.2;

	if (pev->waterlevel != 0)
	{
		pev->velocity = pev->velocity * 0.5;
	}
}


//MODDD - NEW.  Method to be called when a 'touch' method detects the ground.
void CGrenade::groundContact(void) {

	//MODDD - grenades placed on the ground should not do the sequence change, they're
	// already upright.
	if (!dropped) {
		// Also, only happen the first time since touching the ground.
		// It's most important not to do the angle shift multiple times.

		if (!firstGroundContactYet) {
			// no weirdness from that.  both might be redundant though.
			pev->effects |= EF_NOINTERP;
			pev->renderfx |= STOPINTR;

			// needs to be specified now that it animates properly.
			//   imagine that.
			pev->frame = 0;
			pev->framerate = 0;

			// and hit those angles.
			// No pitch, and need to rotate a ways
			//pev->angles = g_vecZero;

			// What was the sequence I was thrown with?  Longways or sideways?
			if (pev->sequence >= 3 && pev->sequence <= 5) {
				pev->sequence = 1;   //laying on the ground.
				// standard throw anim?  Land long-ways
				SetAngleX(0);
				//SetAngleX(RANDOM_FLOAT(-150, 150));
				ChangeAngleY(-90);
				SetAngleZ(0);
				//SetAngleZ(RANDOM_FLOAT(-150, 150));
			}
			else if(pev->sequence == 1 || pev->sequence == 2){
				pev->sequence = 1;   //laying on the ground.
				// toss throw anim?  Land sideways.
				SetAngleX(0);
				//ChangeAngleY(0);
				SetAngleZ(0);
			}

			firstGroundContactYet = TRUE;
		}
	}//dropped check
}



void CGrenade::BounceTouch( CBaseEntity *pOther )
{
	// don't hit the guy that launched this grenade
	if ( pOther->edict() == pev->owner )
		return;

	// only do damage if we're moving fairly fast
	if (m_flBounceDamageCooldown < gpGlobals->time && pev->velocity.Length() > 100)
	{
		entvars_t *pevOwner = VARS( pev->owner );
		if (pevOwner)
		{
			TraceResult tr = UTIL_GetGlobalTrace( );
			ClearMultiDamage( );
			pOther->TraceAttack(pevOwner, 1, gpGlobals->v_forward, &tr, DMG_CLUB ); 
			ApplyMultiDamage( pev, pevOwner);
		}
		m_flBounceDamageCooldown = gpGlobals->time + 1.0; // debounce
	}

	Vector vecTestVelocity;
	// pev->avelocity = Vector (300, 300, 300);

	// this is my heuristic for modulating the grenade velocity because grenades dropped purely vertical
	// or thrown very far tend to slow down too quickly for me to always catch just by testing velocity. 
	// trimming the Z velocity a bit seems to help quite a bit.
	vecTestVelocity = pev->velocity; 
	vecTestVelocity.z *= 0.45;

	if ( !m_fRegisteredSound && vecTestVelocity.Length() <= 60 )
	{
		//ALERT( at_console, "Grenade Registered!: %f\n", vecTestVelocity.Length() );

		// grenade is moving very slow. It's probably very close to where it will ultimately stop moving. 
		// go ahead and emit the danger sound.
		
		// register a radius louder than the explosion, so we make sure everyone gets out of the way
		CSoundEnt::InsertSound ( bits_SOUND_DANGER, pev->origin, pev->dmg / 0.4, 0.3 );
		m_fRegisteredSound = TRUE;
	}



	//BOOL isThatNull = (pev->groundentity == NULL);

	if (pev->flags & FL_ONGROUND)
	{
		// add a bit of static friction
		pev->velocity = pev->velocity * 0.8;

		groundContact();
	}
	else
	{
		//if (pOther->pev->solid == SOLID_BSP) {
			//MODDD - hit some part of the map?  NOW anything.
			// Sliding on an incline strangely doesn't count as 'FL_ONGROUND' until it's moving very slowly along it.
			// You know what fine, trace-line.  Because this engine sure can't send us the point of contact now can it.
		
			TraceResult tr;
			//Vector vecStart = Center();
			// we have 0 size, just be the origin.
			Vector vecStart = pev->origin;
			UTIL_TraceLine(vecStart, vecStart + Vector(0, 0, -4), dont_ignore_monsters, ENT(pev), &tr);

			if (tr.fStartSolid || tr.fAllSolid || tr.flFraction < 1.0) {
				// this will also be ground contact.

				// add a bit of static friction (more so, likely going up a ramp)
				pev->velocity = pev->velocity * 0.55;

				groundContact();
			}

		//}

		// play bounce sound
		BounceSound();
	}


	//MODDD - ??? what was this even trying to do.
	/*
	pev->framerate = pev->velocity.Length() / 200.0;
	if (pev->framerate > 1.0)
		pev->framerate = 1;
	else if (pev->framerate < 0.5)
		pev->framerate = 0;
	*/

}


void CGrenade::SlideTouch( CBaseEntity *pOther )
{
	// don't hit the guy that launched this grenade
	if ( pOther->edict() == pev->owner )
		return;

	// pev->avelocity = Vector (300, 300, 300);

	if (pev->flags & FL_ONGROUND)
	{
		// add a bit of static friction
		pev->velocity = pev->velocity * 0.95;

		if (pev->velocity.x != 0 || pev->velocity.y != 0)
		{
			// maintain sliding sound
		}
	}
	else
	{
		BounceSound();
	}
}

void CGrenade::BounceSound( void )
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

	//MODDD - refer to CVar.
	if(EASY_CVAR_GET_DEBUGONLY(handGrenadesUseOldBounceSound) != 1){
		switch ( RANDOM_LONG( 0, 2 ) )
		{
		case 0:	UTIL_PlaySound(ENT(pev), CHAN_VOICE, "weapons/grenade_hit1.wav", 0.25, ATTN_NORM, 0, 100, FALSE);	break;
		case 1:	UTIL_PlaySound(ENT(pev), CHAN_VOICE, "weapons/grenade_hit2.wav", 0.25, ATTN_NORM, 0, 100, FALSE);	break;
		case 2:	UTIL_PlaySound(ENT(pev), CHAN_VOICE, "weapons/grenade_hit3.wav", 0.25, ATTN_NORM, 0, 100, FALSE);	break;
		}
	}else{
		//Using the old sound if the CVar is 1.  copied from the sachel.
		switch ( RANDOM_LONG( 0, 2 ) )
		{
		case 0:	UTIL_PlaySound(ENT(pev), CHAN_VOICE, "weapons/g_bounce1.wav", 1, ATTN_NORM, 0, 100, FALSE);	break;
		case 1:	UTIL_PlaySound(ENT(pev), CHAN_VOICE, "weapons/g_bounce2.wav", 1, ATTN_NORM, 0, 100, FALSE);	break;
		case 2:	UTIL_PlaySound(ENT(pev), CHAN_VOICE, "weapons/g_bounce3.wav", 1, ATTN_NORM, 0, 100, FALSE);	break;
		}
	}

}

void CGrenade::TumbleThink( void )
{
	if (!IsInWorld())
	{
		UTIL_Remove( this );
		return;
	}

	//int mySeq = pev->sequence;
	//float myFram = pev->frame;
	//float myFramrat = pev->framerate;


	StudioFrameAdvance_SIMPLE( );
	pev->nextthink = gpGlobals->time + 0.1;

	if (pev->dmgtime - 1 < gpGlobals->time)
	{
		CSoundEnt::InsertSound ( bits_SOUND_DANGER, pev->origin + pev->velocity * (pev->dmgtime - gpGlobals->time), 400, 0.1 );
	}

	if (pev->dmgtime <= gpGlobals->time)
	{
		SetThink( &CGrenade::Detonate );
	}
	if (pev->waterlevel != 0)
	{
		pev->velocity = pev->velocity * 0.5;

		//MODDD - only not on touching the ground yet
		if (!firstGroundContactYet) {
			pev->framerate = 0.2;
		}
	}
}

/*
//MODDD - added.
// not necessary.
void CGrenade::Activate( void ){
	
	CBaseMonster::Activate();
	//Just a bridge for the heirarchy.  Other grenades may implement "Activate".
}
*/


// A lot of these defaults are never used, like "grenade.mdl".   w_grenade is used for hand grenades!
// TODO - strip em' out???    Or move to whatever relies on them.
// ShootContact's grenade seems to keep it for the mp5 grenades though.   In fact, MOVED
void CGrenade::Spawn( void )
{
	pev->movetype = MOVETYPE_BOUNCE;
	pev->classname = MAKE_STRING( "grenade" );
	
	pev->solid = SOLID_BBOX;

	UTIL_SetSize(pev, Vector( 0, 0, 0), Vector(0, 0, 0));

	// DEFAULT, some outside source should set this if necessary.
	pev->dmg = 100;
	m_fRegisteredSound = FALSE;
}




//MODDD - damage specified per call.
CGrenade *CGrenade::ShootContact( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float flDamage)
{
	//MODDD NOTE - call "Spawn" on something NULL.. this works.   W H A T
	//             oh, GetClassPtr calls CREATE_ENTITY (engine method) to put an entity in the game. So it just isn't linked to some class besides CBaseEntity.
	CGrenade *pGrenade = GetClassPtr( (CGrenade *)NULL );
	pGrenade->Spawn();

	//MODDD - moved here, as this is the only other grenade spawn call.  Other is the hand grenade that uses w_grenade.mdl.
	SET_MODEL(ENT(pGrenade->pev), "models/grenade.mdl");

	/*
	// WHO IS OWNER
	CBaseEntity* myOwner = NULL;
	if(pevOwner != NULL){
		myOwner = CBaseEntity::Instance(pevOwner);
	}
	const char* daOwnerClassname;
	if(myOwner != NULL){
		daOwnerClassname = myOwner->getClassname();
	}else{
		daOwnerClassname = "NULL?!";
	}
	/////////////////////////////////////////////////////////
	*/


	// contact grenades arc lower
	pGrenade->pev->gravity = 0.5;// lower gravity since grenade is aerodynamic and engine doesn't know it.
	UTIL_SetOrigin( pGrenade->pev, vecStart );
	pGrenade->pev->velocity = vecVelocity;
	pGrenade->pev->angles = UTIL_VecToAngles (pGrenade->pev->velocity);
	pGrenade->pev->owner = ENT(pevOwner);
	
	// make monsters afaid of it while in the air
	pGrenade->SetThink( &CGrenade::DangerSoundThink );
	pGrenade->pev->nextthink = gpGlobals->time;
	
	// Tumble in air
	pGrenade->pev->avelocity.x = RANDOM_FLOAT ( -100, -500 );
	
	// Explode on contact
	pGrenade->SetTouch( &CGrenade::ExplodeTouch );

	pGrenade->pev->dmg = flDamage;


	//MODDD - ?
	//For a reference.
	//PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), fUseAutoAim ? m_usFireGlock1 : m_usFireGlock2, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, m_fInAttack, 0, ( m_iClip == 0 ) ? 1 : 0, 0 );
	
	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(trailTypeTest) > -1){
		//This was just for a test.  Enable (along with some other things in place), and this should make mp5 grenades fly with a trail of grey dots.
		PLAYBACK_EVENT_FULL (FEV_GLOBAL, pGrenade->edict(), g_sTrail, 0.0, 
		(float *)&pGrenade->pev->origin, (float *)&pGrenade->pev->angles, 0.7, 0.0, pGrenade->entindex(), (int)EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(trailTypeTest), 0, 0);
	}else if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(trailTypeTest) == -2){
		PLAYBACK_EVENT_FULL (FEV_GLOBAL, pGrenade->edict(), g_sTrailRA, 0.0, 
		(float *)&pGrenade->pev->origin, (float *)&pGrenade->pev->angles, 0.7, 0.0, pGrenade->entindex(), 0, 0, 0);
	
	}

	return pGrenade;
}


//MODDD - wasn't a way to specify damage in the call?  Why?
CGrenade * CGrenade::ShootTimed( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float flDamage, float flDetonateTime )
{

	/*
	// OLD SCRIPT

	CGrenade* pGrenade = GetClassPtr((CGrenade*)NULL);
	pGrenade->Spawn();
	UTIL_SetOrigin(pGrenade->pev, vecStart);
	pGrenade->pev->velocity = vecVelocity;
	pGrenade->pev->angles = UTIL_VecToAngles(pGrenade->pev->velocity);
	pGrenade->pev->owner = ENT(pevOwner);

	pGrenade->SetTouch(&CGrenade::BounceTouch);	// Bounce if touched

	// Take one second off of the desired detonation time and set the think to PreDetonate. PreDetonate
	// will insert a DANGER sound into the world sound list and delay detonation for one second so that 
	// the grenade explodes after the exact amount of time specified in the call to ShootTimed(). 

	pGrenade->pev->dmgtime = gpGlobals->time + flDetonateTime;
	pGrenade->SetThink(&CGrenade::TumbleThink);
	pGrenade->pev->nextthink = gpGlobals->time + 0.1;
	if (flDetonateTime < 0.1)
	{
		pGrenade->pev->nextthink = gpGlobals->time;
		pGrenade->pev->velocity = Vector(0, 0, 0);
	}

	pGrenade->pev->sequence = RANDOM_LONG(3, 6);
	pGrenade->pev->framerate = 1.0;

	// Tumble through the air
	// pGrenade->pev->avelocity.x = -400;

	pGrenade->pev->gravity = 0.5;
	pGrenade->pev->friction = 0.8;

	SET_MODEL(ENT(pGrenade->pev), "models/w_grenade.mdl");
	pGrenade->pev->dmg = 100;

	return pGrenade;
	*/



	CGrenade *pGrenade = GetClassPtr( (CGrenade *)NULL );
	pGrenade->Spawn();


	//MODDD - CHANGE: Let's not set the sequence before setting the model, yes?

	SET_MODEL(ENT(pGrenade->pev), "models/w_grenade.mdl");



	UTIL_SetOrigin( pGrenade->pev, vecStart );
	pGrenade->pev->velocity = vecVelocity;
	pGrenade->pev->angles = UTIL_VecToAngles(pGrenade->pev->velocity);
	pGrenade->pev->owner = ENT(pevOwner);

	
	pGrenade->SetTouch( &CGrenade::BounceTouch );	// Bounce if touched
	
	// Take one second off of the desired detonation time and set the think to PreDetonate. PreDetonate
	// will insert a DANGER sound into the world sound list and delay detonation for one second so that 
	// the grenade explodes after the exact amount of time specified in the call to ShootTimed(). 

	pGrenade->pev->dmgtime = gpGlobals->time + flDetonateTime;
	pGrenade->SetThink( &CGrenade::TumbleThink );
	pGrenade->pev->nextthink = gpGlobals->time + 0.1;
	if (flDetonateTime < 0.1)
	{
		pGrenade->pev->nextthink = gpGlobals->time;
		pGrenade->pev->velocity = Vector( 0, 0, 0 );
	}



	//MODDD
	// Randomizes an animation for the grenade.
	// ALSO, old range was 3 to 6.  Now 3 to 5.  What is with #6 anyway, it's still jittering too weird even
	// when it plays correctly, it's just weeeiird.
	pGrenade->pev->sequence = RANDOM_LONG( 3, 5 );
	//pGrenade->pev->sequence = 5;

	// HeYyyyyyy!! Set this too you fool!
	pGrenade->pev->frame = 0;
	pGrenade->pev->framerate = 1.0;
	pGrenade->ResetSequenceInfo();


	// Tumble through the air
	// pGrenade->pev->avelocity.x = -400;

	//MODDD - important maybe?  Explicitly set to zero.
	pGrenade->pev->avelocity = g_vecZero;


	pGrenade->pev->gravity = 0.5;
	pGrenade->pev->friction = 0.8;


	pGrenade->pev->dmg = flDamage;

	return pGrenade;
}


// similar to ShootTimed but best for grenades that are placed on/near the ground instead.
CGrenade* CGrenade::ShootTimedDropped(entvars_t* pevOwner, Vector vecStart, Vector vecVelocity, float flDamage, float flDetonateTime) {
	CGrenade* result = CGrenade::ShootTimed(pevOwner, vecStart, vecVelocity, flDamage, flDetonateTime);

	result->pev->sequence = 0;
	result->pev->framerate = 0;
	result->dropped = TRUE;

	result->pev->angles = g_vecZero;

	return result;
}




//MODDD - methods "ShootSatchelCharge" and "UseSatchelCharges" removed.  Unused here.

float CGrenade::massInfluence(void){
	return 0.12f;
}
int CGrenade::GetProjectileType(void){
	return PROJECTILE_GRENADE;
}

//======================end grenade
