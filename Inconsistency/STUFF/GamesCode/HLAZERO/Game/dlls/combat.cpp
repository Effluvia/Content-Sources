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

===== combat.cpp ========================================================

  functions dealing with damage infliction & death

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "basemonster.h"
#include "soundent.h"
#include "decals.h"
#include "util_model.h"
#include "weapons.h"
#include "func_break.h"
//MODDD - okay?
#include "squadmonster.h"
#include "game.h"
//MODDD - necessary anymore?
#include "player.h"
#include "gib.h"  //new file

//MODDD - needed for a spawnflag. OR scripted spawnflags could be moved to cbase.h to be available everywhere if needed, they are uniquely named anyways to avoid conflicts.
//        (in name, not necessarily in the bit occupied)
#include "scripted.h"
#include "util_debugdraw.h"

EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST(sv_germancensorship)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(muteRicochetSound)
EASY_CVAR_EXTERN_DEBUGONLY(bulletholeAlertRange)
EASY_CVAR_EXTERN_DEBUGONLY(nothingHurts)
EASY_CVAR_EXTERN_DEBUGONLY(timedDamageAffectsMonsters)
EASY_CVAR_EXTERN_DEBUGONLY(bulletHoleAlertPrintout)
EASY_CVAR_EXTERN_DEBUGONLY(bulletholeAlertStukaOnly)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST(playerBulletHitEffectForceServer)
EASY_CVAR_EXTERN_DEBUGONLY(baseEntityDamagePushNormalMulti)
EASY_CVAR_EXTERN_DEBUGONLY(baseEntityDamagePushVerticalBoost)
EASY_CVAR_EXTERN_DEBUGONLY(baseEntityDamagePushVerticalMulti)
EASY_CVAR_EXTERN_DEBUGONLY(baseEntityDamagePushVerticalMinimum)
EASY_CVAR_EXTERN_DEBUGONLY(RadiusDamageDrawDebug)
EASY_CVAR_EXTERN_DEBUGONLY(AlienRadiationImmunity)
EASY_CVAR_EXTERN_DEBUGONLY(germanRobotGibs)
EASY_CVAR_EXTERN_DEBUGONLY(germanRobotBleedsOil)
EASY_CVAR_EXTERN_DEBUGONLY(germanRobotDamageDecal)
EASY_CVAR_EXTERN_DEBUGONLY(germanRobotGibsDecal)
EASY_CVAR_EXTERN_DEBUGONLY(monsterFadeOutRate)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST(playerWeaponTracerMode)
EASY_CVAR_EXTERN_DEBUGONLY(monsterWeaponTracerMode)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(decalTracerExclusivity)
EASY_CVAR_EXTERN_DEBUGONLY(cheat_iwantguts)
EASY_CVAR_EXTERN_DEBUGONLY(playerUseDrawDebug)


extern float globalPSEUDO_canApplyGermanCensorship;
extern BOOL globalPSEUDO_germanModel_hgibFound;


extern DLL_GLOBAL Vector g_vecAttackDir;
extern DLL_GLOBAL int g_iSkillLevel;

//MODDD - yea...
extern DLL_GLOBAL int g_bitsDamageType;
extern DLL_GLOBAL int g_bitsDamageTypeMod;
extern DLL_GLOBAL BOOL g_tossKilledCall;
extern DLL_GLOBAL float g_rawDamageCumula;

//extern entvars_t *g_pevLastInflictor;





BOOL CBaseMonster::HasHumanGibs( void )
{
	int myClass = Classify();

	//MODDD NOTICE - this is before the "IsOrganic()" check was made.
	// This will still catch robot replacement models, but that is fine, they are handled in GibMonster under a HasHumanGibs check.
	// Other odd cases (apache) have dummied GibMonster methods so this won't matter.
	if (

		myClass == CLASS_HUMAN_MILITARY ||
		myClass == CLASS_PLAYER_ALLY ||
		myClass == CLASS_HUMAN_PASSIVE ||
		myClass == CLASS_PLAYER
	)
	{

		return TRUE;
	}

	return FALSE;
}


BOOL CBaseMonster::HasAlienGibs( void )
{
	int myClass = Classify();

	if (
		myClass == CLASS_ALIEN_MILITARY ||
		myClass == CLASS_ALIEN_MONSTER ||
		myClass == CLASS_ALIEN_PASSIVE ||
		myClass == CLASS_INSECT ||
		myClass == CLASS_ALIEN_PREDATOR ||
		myClass == CLASS_ALIEN_PREY
	)
	{
		return TRUE;
	}

	return FALSE;
}


void CBaseMonster::FadeMonster( void )
{
	StopAnimation();
	pev->velocity = g_vecZero;
	pev->movetype = MOVETYPE_NONE;
	pev->avelocity = g_vecZero;
	pev->animtime = gpGlobals->time;
	pev->effects |= EF_NOINTERP;
	SUB_StartFadeOut();
}



BOOL CBaseMonster::DetermineGibHeadBlock(void){
	//by default, there isn't a case to block head generation. This does not guarantee a head gib is generated for all monsters.
	//Only enforces a block (restriction) on spawning a head if this returns TRUE. Such as a headless hgrunt corpse gibbing: no head to give.
	return FALSE;
}




//MODDD - merging CallGibMonster and GibMonster into one GibMonster method. No need for these to be separate.


//Same as "CallGibMonster", now GibMonster, but always just instantly deletes and does some things out of courtesy... if this even matters.
void CBaseMonster::cleanDelete(){
	
	//???
	BOOL fade = FALSE;

	pev->takedamage = DAMAGE_NO;
	pev->solid = SOLID_NOT;// do something with the body. while monster blows up
	
	pev->effects = EF_NODRAW; // make the model invisible
	if ( ShouldFadeOnDeath() && !fade )
		UTIL_Remove(this);
}



//=========================================================
// GibMonster - create some gore and get rid of a monster's
// model.
//=========================================================
//MODDD - new description.
// This is some basic script for monsters to gib. It assumes any monsters that fall under the HasHumanGibs() check will spawn blood red gibs,
// classes fall under the HasAlienGibs() check will spawn yellow gibs.
// This also checks for robot-replaced monsters in german censorship (still fall under the HasHumanGibs() check) and lets them spawn
// metal, gear, etc. gibs if allowed instead.
// In a particular monster, override GibMonster to give it its own specific gibbing behavior.
// parameter: BOOL fGibSpawnsDecal
GENERATE_GIBMONSTER_IMPLEMENTATION(CBaseMonster)
{

	BOOL gibbed = GibMonsterGib(fGibSpawnsDecal);

	GibMonsterSound(gibbed);
	GibMonsterEnd(gibbed);

}//END OF GibMonster


//parameter: BOOL fGibSpawnsDecal
GENERATE_GIBMONSTERGIB_IMPLEMENTATION(CBaseMonster){
	TraceResult	tr;
	BOOL gibbed = FALSE;
	
	//MODDD NOTE - Mainly, GibMonster means we plan on gibbing the monster, but filter out a few more possibilities, like being disallowed by violence_Xgibs per
	//             this monster's type, or german censorship not allowing gibs for humans or even robot replacements (germanRobotGibs of 0).



	//MODDD - for monsters who involve the last-hit-hitbox for how to gib a monster.
	// Precision for this makes no sense, so do whatever the default would be.  This is RED blood for zombies (most of the creature is the scientist).
	m_LastHitGroup = HITGROUP_GENERIC;

	// only humans throw skulls !!!UNDONE - eventually monsters will have their own sets of gibs
	if ( HasHumanGibs() )
	{
		
		//Old check... mostly.
		//if ( EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(sv_germancensorship) == 1 || CVAR_GET_FLOAT("violence_hgibs") != 0 )	// Only the player will ever get here   ...Why was this comment here as-is? only the player? what?
		
		
		//MODDD - Little intervention here when spawning human gibs. Check to see if german censorship is on.

		if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(sv_germancensorship)==0){
			//turned off? usual behavior, only require violence_hgibs to be off.
			if(CVAR_GET_FLOAT("violence_hgibs") != 0){
				BOOL spawnHeadBlock = this->DetermineGibHeadBlock();
				if(!spawnHeadBlock){
					//HeadGib assumes human, only option.
					CGib::SpawnHeadGib( pev, fGibSpawnsDecal );
				}
				CGib::SpawnRandomGibs( pev, 4, GIB_HUMAN_ID, fGibSpawnsDecal );	// throw some human gibs.
				gibbed = TRUE;  //MODDD - only count as gibbed if we managed to spawn gibs. Any blocking from germancensorship makes the model fade out instead of disappear instantly.
			}
		}else{
			//with german censorship on, a call for human gibs gets replaced with robot gibs only if we're allowed to use german models (separate CVar), robot gibs are allowed and the robot gib model was found.
			//Notice that german censorship ignores the "violence_hgibs", and would already disallow spawning gibs.
			if( CanUseGermanModel() && EASY_CVAR_GET_DEBUGONLY(germanRobotGibs)==1 && globalPSEUDO_germanModel_hgibFound==1 )
			{
				//robots do not have a head gib.
				//if(!spawnHeadBlock){
				//	CGib::SpawnHeadGib( pev, gibsSpawnDecal );
				//}

				if(EASY_CVAR_GET_DEBUGONLY(germanRobotGibsDecal)==0){
					//this disallows robot gib decals made on hitting the ground.
					fGibSpawnsDecal = FALSE;
				}

				CGib::SpawnRandomGibs( pev, 6, GIB_GERMAN_ID, fGibSpawnsDecal );	// throw some robot gibs.
				gibbed = TRUE;
			}
		}

	}
	else if ( HasAlienGibs() )
	{
		//NOTE: using real alien gibs.  If "agibs" are banned, then don't show.
		//if ( EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(sv_germancensorship) == 1 || CVAR_GET_FLOAT("violence_agibs") != 0 )	// Should never get here, but someone might call it directly

		//MODDD - no involvement from germancensorship, keep to using violence_agibs.
		if(CVAR_GET_FLOAT("violence_agibs") != 0)
		{
			Vector temp = this->pev->absmax - this->pev->absmin;
			

			int gibSpawnCount;
			int myBlood = BloodColor();
			int gibChoiceID;

			if(temp.x * temp.y * temp.z < 5000000){
				gibSpawnCount = 4;
			}else{
				gibSpawnCount = 20;
				// This check is... interesting, given that the gargantua already spawns gibs in its own file in retail (end of the transform-death-effect; no fall/corpse).
			}

			// Now need a little more info.   Yellow or green-blooded alien gibs?
			if (myBlood == BLOOD_COLOR_YELLOW) {
				gibChoiceID = GIB_ALIEN_YELLOW_ID;
			}
			else if (myBlood == BLOOD_COLOR_GREEN) {
				gibChoiceID = GIB_ALIEN_GREEN_ID;
			}
			else if (myBlood == BLOOD_COLOR_RED) {
				// the zombie?  Spawn a mixture of both gibs then.  And end early.
				

				// MODDD - don't know how german censorship will handle zombies, not doing anything for the human-gib portion for now
				if (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(sv_germancensorship) == 0) {
					if (CVAR_GET_FLOAT("violence_hgibs") != 0) {
						CGib::SpawnRandomGibs(pev, 3, GIB_HUMAN_ID, fGibSpawnsDecal);	// Throw human gibs
					}
				}

				CGib::SpawnRandomGibs(pev, 3, GIB_ALIEN_GREEN_ID, fGibSpawnsDecal);	// Throw alien gibs
				return TRUE;
			}
			else {
				// ???  unknown color for aliens
				gibChoiceID = GIB_DUMMY_ID;
			}

			CGib::SpawnRandomGibs(pev, gibSpawnCount, gibChoiceID, fGibSpawnsDecal);	// Throw alien gibs

			gibbed = TRUE;
			
		}
	}else{
		//Neither human nor alien? Should this assume some other form of mechanical? Turrets have their own gibbing script now and robot replacements still register as "HasHumanGibs()" to be replaced in there.

		//Default behavior is to just fade out, probably fine.

	}

	return gibbed;
}//END OF GibMonsterGib



//parameter: BOOL fGibbed
GENERATE_GIBMONSTERSOUND_IMPLEMENTATION(CBaseMonster){

	//MODDD - only play this sound if organic. This excludes german robot replacements.
	if( isOrganic() ){

		//Only organic things that gibbed can play this sound.
		if(fGibbed){
			//MODDD - use soundsentencesave, always available in sentence form.
			//Also, german censorship will block this sound for humans regardless of robot models.
			//UTIL_PlaySound(ENT(pev), CHAN_WEAPON, "common/bodysplat.wav", 1, ATTN_NORM, TRUE);
			UTIL_playOrganicGibSound(pev);
		}
	}else{
		//Any metallic things exploding or fading out play this sound instead. Includes german robot replacements gibbing.
		UTIL_playMetalGibSound(pev);
	}

}//END OF GibMonsterSound



//parameter: BOOL fGibbed
GENERATE_GIBMONSTEREND_IMPLEMENTATION(CBaseMonster){

	pev->takedamage = DAMAGE_NO;
	pev->solid = SOLID_NOT;// do something with the body. while monster blows up

	if ( fGibbed ){
		//MODDD - new. Stop the voice channel when deleted.
		UTIL_PlaySound(ENT(pev), CHAN_VOICE, "common/null.wav", 1, ATTN_NORM, 0, 100, FALSE);

		pev->effects = EF_NODRAW; // make the model invisible.
		
		if ( !IsPlayer() )
		{
			//GibMonster(DetermineGibHeadBlock(), gibsSpawnDecals);  //you are here.
			// don't remove players!
			SetThink ( &CBaseEntity::SUB_Remove );
			pev->nextthink = gpGlobals->time;
		}else{

			//???
		}
	}
	else
	{
		//NOTICE - the Player overrides this method to not try and delete itself
		//at the end of fade, not that changing the think method would work here anyways
		//to even get to that point.
		FadeMonster();
	}

	pev->deadflag = DEAD_DEAD;
	FCheckAITrigger();

	// don't let the status bar glitch for players.with <0 health.
	//MODDD NOTE - should this check only be for players then..?
	if (pev->health < -99)
	{
		pev->health = 0;
	}
	
	//If we were supposed to fade on death but for some reason didn't (gibbed), do this to be safe.
	if ( ShouldFadeOnDeath() && fGibbed )  //AKA, not fade.
		UTIL_Remove(this);



}//END OF GibMonsterEnd




//MODDD - Reorganized this a bit to remove some redundancies.  Should have equivalent behavior.

//=========================================================
// GetDeathActivity - determines the best type of death
// anim to play.
//=========================================================
Activity CBaseMonster::GetDeathActivity ( void )
{
	Activity	deathActivity;
	//BOOL		fTriedDirection;
	BOOL		fCanTryDirection = FALSE; //MODDD - new.
	float	flDot;
	TraceResult	tr;
	Vector		vecSrc;
	int violentDeathPriorityValue;

	//MODDD - how soon should I check for the violentDeath compared to other ACT's and return early if violent death is allowed?
	violentDeathPriorityValue = violentDeathPriority();

	if(!violentDeathAllowed()){
		//then forget it.
		violentDeathPriorityValue = -1;
	}

	if ( pev->deadflag != DEAD_NO )
	{
		// don't run this while dying.
		return m_IdealActivity;
	}


	if(violentDeathPriorityValue == 1 && violentDeathDamageRequirement() && violentDeathClear()){
		return ACT_DIEVIOLENT;
	}


	vecSrc = Center();

	//fTriedDirection = FALSE;
	deathActivity = ACT_DIESIMPLE;// in case we can't find any special deaths to do.



	switch ( m_LastHitGroup )
	{
		// try to pick a region-specific death.
	case HITGROUP_HEAD:
		if(LookupActivity(ACT_DIE_HEADSHOT) != ACTIVITY_NOT_AVAILABLE){
			//do it!
			deathActivity = ACT_DIE_HEADSHOT;
		}else{
			//try that instead
			fCanTryDirection = TRUE;
		}
		break;

	case HITGROUP_STOMACH:
		if(LookupActivity(ACT_DIE_GUTSHOT) != ACTIVITY_NOT_AVAILABLE){
			//do it!
			deathActivity = ACT_DIE_GUTSHOT;
		}else{
			//try that instead
			fCanTryDirection = TRUE;
		}
		break;

	case HITGROUP_GENERIC:
		// try to pick a death based on attack direction
		fCanTryDirection = TRUE;
		break;

	default:
		// try to pick a death based on attack direction
		fCanTryDirection = TRUE;
		break;
	}




	if(fCanTryDirection){

		
		if(violentDeathPriorityValue == 2 && violentDeathDamageRequirement() && violentDeathClear()){
			return ACT_DIEVIOLENT;
		}

		UTIL_MakeVectors ( pev->angles );
		flDot = DotProduct ( gpGlobals->v_forward, g_vecAttackDir * -1 );
		//Haven't found a good death activity above (has a sequence available)? Try the directions.
		if ( flDot > 0.3 )
		{
			if(LookupActivity(ACT_DIEFORWARD) != ACTIVITY_NOT_AVAILABLE){
				//One more check.
				// make sure there's room to fall forward
				UTIL_TraceHull ( vecSrc, vecSrc + gpGlobals->v_forward * 64, dont_ignore_monsters, head_hull, edict(), &tr );
				// Nothing in the way? it's good.
				if ( tr.flFraction == 1.0 )
				{
					deathActivity = ACT_DIEFORWARD;
				}
			}//END OF ACT_DIEFORWARD check
		}
		else if ( flDot <= -0.3 )
		{

			//MODDD INJECTION - is ACT_DIEVIOLENT possible?
			// It must be allowed by this particular monster to be used, regardless of having a sequence mapped by the model.
			// This way we can promise some attention was paid to whether it needs a trace check or not, which varries per monster.
			// There is no standard length of trace that will satisfy all violent death animations.  violentDeathClear will handle that.
			
			if(violentDeathPriorityValue == 3 && violentDeathDamageRequirement() && violentDeathClear()){
				return ACT_DIEVIOLENT;
			}
			
			/*
			if(violentDeathAllowed() && violentDeathDamageRequirement() && violentDeathClear()){
				//Apparently this is OK.
				//But if there are ever multiple violent death anims, we need to use this as a signal to pick
				//a more specific one later possibly, like what one passed its own distance check.
				deathActivity = ACT_DIEVIOLENT;
			}else
			*/
			{
				//give DIEBACKWARD a chance as usual.
				if(LookupActivity(ACT_DIEBACKWARD) != ACTIVITY_NOT_AVAILABLE){
					//One more check.
					// make sure there's room to fall backward
					UTIL_TraceHull ( vecSrc, vecSrc - gpGlobals->v_forward * 64, dont_ignore_monsters, head_hull, edict(), &tr );
					// Nothing in the way? it's good.
					if ( tr.flFraction == 1.0 )
					{
						deathActivity = ACT_DIEBACKWARD;
					}
				}//END OF ACT_DIEBACKWARD check
			}



		}//END OF flDot checks
	}//END OF fCanTryDirection

	return deathActivity;
}//END OF GetDeathActivity


//=========================================================
// GetSmallFlinchActivity - determines the best type of flinch
// anim to play.
//=========================================================
Activity CBaseMonster::GetSmallFlinchActivity ( void )
{
	Activity	flinchActivity;
	BOOL		fTriedDirection;
	float	flDot;

	fTriedDirection = FALSE;
	UTIL_MakeVectors ( pev->angles );
	flDot = DotProduct ( gpGlobals->v_forward, g_vecAttackDir * -1 );
	
	switch ( m_LastHitGroup )
	{
		// pick a region-specific flinch
	case HITGROUP_HEAD:
		flinchActivity = ACT_FLINCH_HEAD;
		break;
	case HITGROUP_STOMACH:
		flinchActivity = ACT_FLINCH_STOMACH;
		break;
	case HITGROUP_LEFTARM:
		flinchActivity = ACT_FLINCH_LEFTARM;
		break;
	case HITGROUP_RIGHTARM:
		flinchActivity = ACT_FLINCH_RIGHTARM;
		break;
	case HITGROUP_LEFTLEG:
		flinchActivity = ACT_FLINCH_LEFTLEG;
		break;
	case HITGROUP_RIGHTLEG:
		flinchActivity = ACT_FLINCH_RIGHTLEG;
		break;
	case HITGROUP_GENERIC:
	default:
		// just get a generic flinch.
		flinchActivity = ACT_SMALL_FLINCH;
		break;
	}


	// do we have a sequence for the ideal activity?
	if ( LookupActivity ( flinchActivity ) == ACTIVITY_NOT_AVAILABLE )
	{
		flinchActivity = ACT_SMALL_FLINCH;
	}

	return flinchActivity;
}//END OF GetSmallFlinchActivity


Activity CBaseMonster::GetBigFlinchActivity(void){
	
	Activity flinchActivity;

	flinchActivity = ACT_BIG_FLINCH;

	// do we have a sequence for this big flinch?
	if ( LookupActivity ( flinchActivity ) == ACTIVITY_NOT_AVAILABLE )
	{
		flinchActivity = ACT_SMALL_FLINCH;
	}

	return flinchActivity;
}//END OF GetBigFlinchActivity





// Is there any conflict with this if archers allow knockback?

//MODDD - NOTE
// For intensity of the most recent attack, check pev->health to see how far into the negatives it is.
// Maybe throw the corpse further if it's higher.
void CBaseMonster::BecomeDead(void)
{
	pev->takedamage = DAMAGE_YES;// don't let autoaim aim at corpses.

	// make the corpse fly away from the attack vector
	//MODDD - note that with any 'fling from previous damage' logic commented out below in the as-is script,
	// this just meant plummet if there's no ground below me.  Probably just to stop being MOVETYPE_STEP or FLY.
	pev->movetype = MOVETYPE_TOSS;


	//MODDD - restored, was found commented-out as-is.   (monsterKilledToss == 1 restores it as found here)
	// Seemed to be a thing in the alphas anyway, corpses to be flung a bit.


	// eh.  BecomeDead isn't supposed to be able to be called twice,
	// but can't hurt to check this anyway for unusual cases.
	BOOL killedMemory = HasMemory(bits_MEMORY_KILLED);

	// only the first time Killed is called, thank you!
	// pev->deadflag == DEAD_NO  might be ok too, but eh
	if (!killedMemory && g_tossKilledCall) {

		if (EASY_CVAR_GET(sv_explosionknockback) > 0.2 && (g_bitsDamageType & DMG_BLAST)) {
			int breako;
			// If I likely took good knockback from some explosion already, deny adding further knockback.
		}
		else
		if (!(g_bitsDamageType & DMG_CRUSH) && !(g_bitsDamageTypeMod & (DMG_PROJECTILE | DMG_MAP_BLOCKED | DMG_MAP_TRIGGER))) {

			if (EASY_CVAR_GET(monsterKilledToss) == 1) {
				pev->flags &= ~FL_ONGROUND;
				pev->origin.z += 2;
				pev->velocity = g_vecAttackDir * -1;
				pev->velocity = pev->velocity * RANDOM_FLOAT(300, 400);
				// adding a line.  Even here.
				pev->groundentity = NULL;
			}
			else if (EASY_CVAR_GET(monsterKilledToss) == 2) {
				//float tossAmount = (100 + -pev->health * 6);

				float overkillAmount;

				// say a headshot did 120 damage.  Health at the time was 2.
				// raw damage was 40.
				// Corpse pev->health is -118.
				// We'll only go as far back as g_rawDamageCumula.  so -40 then.

				float forceAmountMulti;

				Vector tempEnBoundDelta = (this->pev->absmax - this->pev->absmin);
				float tempEnSize = tempEnBoundDelta.x * tempEnBoundDelta.y * tempEnBoundDelta.z;

				if (tempEnSize < 16000) {  //headcrab size: 13824
					// I go flyin'!
					forceAmountMulti = 1.2;
				}
				else if (tempEnSize <= 800000) {  //size of agrunt: about 348160
					forceAmountMulti = 1 + -0.99 * ((tempEnSize - 16000) / (800000 - 16000));
				}
				else {
					// OH GOD ITS HUGE.
					forceAmountMulti = 0.01;
				}


				if (g_rawDamageCumula >= -pev->health) {
					// ok, just use neg health
					overkillAmount = -pev->health;
				}
				else {
					// Too deep? Force to g_rawDamageCumula.
					overkillAmount = g_rawDamageCumula;
				}


				Vector attackDirRev_2D = Vector(-g_vecAttackDir.x, -g_vecAttackDir.y, 0).Normalize();

				float tossAmount = (220 + overkillAmount * 8.5) * forceAmountMulti;

				pev->flags &= ~FL_ONGROUND;
				pev->origin.z += 2;
				// variation between tossAmount itself and 12% more.  (base behavior was 33% more)
				pev->velocity = attackDirRev_2D * RANDOM_FLOAT(tossAmount, tossAmount * 1.12);
				// And a tiny bit more z for more overkill.  Why not.
				//pev->velocity.z += tossAmount * 0.035;
				pev->velocity.z += tossAmount * 0.07;

				pev->groundentity = NULL;
			}//END OF monsterKilledToss check

		}// damage type checks

	}// g_tossKilledCall check

	// assume not so next time without it being set again
	g_tossKilledCall = FALSE;


	//MODDD - moved to the bottom, as to not interfere with the negative health to use for
	// determining extent of overkill.
	//-----------
	// give the corpse half of the monster's original maximum health. 
	pev->health = pev->max_health / 2;
	pev->max_health = 5; // max_health now becomes a counter for how many blood decals the corpse can place.


}


BOOL CBaseMonster::ShouldGibMonster( int iGib )
{

	if(EASY_CVAR_GET_DEBUGONLY(cheat_iwantguts) >= 1){
		//VIOLENCE!  BLOOD AND GUTS!!!
		return TRUE;
	}

	//easyForcePrintLine("ShouldGibMonster: My iGib is %d. Curhealth:%.2f : req:%d", iGib, pev->health, GIB_HEALTH_VALUE);

	if ( ( iGib == GIB_NORMAL && pev->health < GIB_HEALTH_VALUE ) || ( iGib == GIB_ALWAYS || iGib == GIB_ALWAYS_NODECAL ) )
		return TRUE;
	
	return FALSE;
}



//Ever wondered how the map knows a monster was killed for events / logic, such as unlocking doors or spawning other enemies in predetermined locations?
//It's "FCheckAITrigger". It is called right before gibbing and every frame of logic in monsterstate.cpp, so that a death anim would trigger the condition too.

/*
============
Killed
============
*/
//void CBaseMonster::Killed( entvars_t *pevAttacker, int iGib )
GENERATE_KILLED_IMPLEMENTATION(CBaseMonster)
{
	//m_pfnThink
		//UTIL_Remove(this);

	easyPrintLine("Killed: %s:%d. deadflag:%d. MY iGib WAS %d", getClassname(), monsterID, pev->deadflag, iGib);


	/*
	//Determined in GibMonster itself intead now.
	BOOL gibsSpawnDecals = !(iGib == GIB_ALWAYS_NODECAL);

	if(CanUseGermanModel() && EASY_CVAR_GET_DEBUGONLY(germanRobotGibsDecal)==0){
		//If german censorship is on and the oil decal for robot gibs is disabled (assuming robot gibs are enabled), the spawned gibs will not make decals.
		gibsSpawnDecals = FALSE;
	}
	*/

	unsigned int cCount = 0;
	BOOL fDone = FALSE;


	if (HasMemory(bits_MEMORY_KILLED))
	{
		if (ShouldGibMonster(iGib)) {
			//GibMonster(DetermineGibHeadBlock(), gibsSpawnDecals);
			GENERATE_GIBMONSTER_CALL;
		}
		return;
	}

	//MODDD - Not yet, freeman!  Maybe "BecomeDead" wants to have an easy way of knowing if this is the first
	// time it's been called before.
	// ...oh wait, this was pointless.  If had KILLED already, that point would've never been reached.
	// BecomeDead can't be called twice, even though 'Killed' can.
	// Well.  Can't hurt at least.
	//Remember( bits_MEMORY_KILLED );

	//MODDD NOTE - beats me why we don't just set the pev->deadflag to DEAD_DYING
	// right here. Although picking TASK_DIE soon calls "deathAnimationStart"
	// which does that.
	// Several of other simplier KILLED methods do the DEAD_DYING set then and there.


	// clear the deceased's sound channels.(may have been firing or reloading when killed)
	UTIL_PlaySound(ENT(pev), CHAN_WEAPON, "common/null.wav", 1, ATTN_NORM, 0, 100, FALSE);

	//MODDD - for voice maybe?  Unless that would stop death-cries or something.


	m_IdealMonsterState = MONSTERSTATE_DEAD;
	// Make sure this condition is fired too (TakeDamage breaks out before this happens on death)
	SetConditions(bits_COND_LIGHT_DAMAGE);

	if (monsterID == 0 || monsterID == 1) {
		int x = 45;
	}

	// tell owner ( if any ) that we're dead.This is mostly for MonsterMaker functionality.
	CBaseEntity* pOwner = CBaseEntity::Instance(pev->owner);
	if (pOwner)
	{
		easyPrintLine("DO I, %s, HAVE A OWNER? %s", getClassname(), pOwner->getClassname());
		pOwner->DeathNotice(pev);
	}


	// Before any mods to health, record what it was for monitoring overkill.
	//////killedHealth = pev->health;
	// Just make it the amount of damage dealth by the last attack.
	// may feel a little better this way.


	if (ShouldGibMonster(iGib))
	{
		//MODDD - this would've happened earlier before, may as well here to still at all at this point.
		Remember(bits_MEMORY_KILLED);

		//GibMonster(DetermineGibHeadBlock(), gibsSpawnDecals);
		GENERATE_GIBMONSTER_CALL;
		return;
	}
	else if (pev->flags & FL_MONSTER)
	{

		//WARNING - bad assumption! Leave what to do to the touch method up to the monster in question.
		//Flyers may want to detect when they touch the ground to stop a falling cycler.
		//SetTouch( NULL );
		OnKilledSetTouch();
		BecomeDead();
	}

	// don't let the status bar glitch for players.with <0 health.
	// MODDD NOTE - exactly what negative value of health is ever okay???
	// Why < -99 and not just < 0?  Testing that out.
	//if (pev->health < -99)
	if (pev->health < 0)
	{
		pev->health = 0;
	}

	//MODDD - just to make sure.
	m_bitsDamageType = 0;
	m_bitsDamageTypeMod = 0;


	//pev->enemy = ENT( pevAttacker );//why? (sjb)

	m_IdealMonsterState = MONSTERSTATE_DEAD;


	//MODDD - well golly gee, I do believe you are now dead
	Remember(bits_MEMORY_KILLED);

}//END OF Killed



//
// fade out - slowly fades a entity out, then removes it.
//
// DON'T USE ME FOR GIBS AND STUFF IN MULTIPLAYER! 
// SET A FUTURE THINK AND A RENDERMODE!!
//MODDD NOTE - luckily this doesn't really fade out the player, 
//but only because the player immediately sets the Think on its own to PlayerDeathThink
//after calling GibMonster. This overrides the SUB_FadeOut Set Think call below.
//But the player should still not call this method anyways.
void CBaseEntity::SUB_StartFadeOut ( void )
{
	if (pev->rendermode == kRenderNormal)
	{
		pev->renderamt = 255;
		pev->rendermode = kRenderTransTexture;
	}

	pev->solid = SOLID_NOT;
	pev->avelocity = g_vecZero;

	pev->nextthink = gpGlobals->time + 0.1;
	SetThink ( &CBaseEntity::SUB_FadeOut );
}

void CBaseEntity::SUB_FadeOut ( void  )
{
	//Multiply the fade by 0.1 because that's how long this think method is expected to delay before getting called again.
	//That effectively means with each second, the monster's opacity (renderamt) drops by an amount of "monsterFadeOutRate".
	
	const float fadeMoveAmount = EASY_CVAR_GET_DEBUGONLY(monsterFadeOutRate)*0.1;

	if ( pev->renderamt > fadeMoveAmount )
	{
		pev->renderamt -= fadeMoveAmount;
		pev->nextthink = gpGlobals->time + 0.1;
	}
	else 
	{
		pev->renderamt = 0;
		pev->nextthink = gpGlobals->time + 0.2;
		SetThink ( &CBaseEntity::SUB_Remove );
	}
}



//=========================================================
// CheckTraceHullAttack - expects a length to trace, amount 
// of damage to do, and damage type. Returns a pointer to
// the damaged entity in case the monster wishes to do
// other stuff to the victim (punchangle, etc)
//
// Used for many contact-range melee attacks. Bites, claws, etc.
//=========================================================
//MODDD - note that the old form, missing the 2nd mask, now  redirects to the new form and just sends "0"
//for the 2nd mask.  Safe to assume no damages meant there if nothing is supplied for it.
CBaseEntity* CBaseMonster::CheckTraceHullAttack( float flDist, int iDamage, int iDmgType ){
	return CheckTraceHullAttack(g_vecZero, flDist, iDamage, iDmgType, 0, NULL);
}
CBaseEntity* CBaseMonster::CheckTraceHullAttack( float flDist, int iDamage, int iDmgType, TraceResult* out_traceResult ){
	return CheckTraceHullAttack(g_vecZero, flDist, iDamage, iDmgType, 0, out_traceResult);
}
CBaseEntity* CBaseMonster::CheckTraceHullAttack( float flDist, int iDamage, int iDmgType, int iDmgTypeMod ){
	return CheckTraceHullAttack(g_vecZero, flDist, iDamage, iDmgType, iDmgTypeMod, NULL);
}
CBaseEntity* CBaseMonster::CheckTraceHullAttack( float flDist, int iDamage, int iDmgType, int iDmgTypeMod, TraceResult* out_traceResult ){
	return CheckTraceHullAttack(g_vecZero, flDist, iDamage, iDmgType, iDmgTypeMod, out_traceResult);
}
CBaseEntity* CBaseMonster::CheckTraceHullAttack( const Vector vecStartOffset, float flDist, int iDamage, int iDmgType ){
	return CheckTraceHullAttack(vecStartOffset, flDist, iDamage, iDmgType, 0, NULL);
}
CBaseEntity* CBaseMonster::CheckTraceHullAttack( const Vector vecStartOffset, float flDist, int iDamage, int iDmgType, TraceResult* out_traceResult ){
	return CheckTraceHullAttack(vecStartOffset, flDist, iDamage, iDmgType, 0, out_traceResult);
}
CBaseEntity* CBaseMonster::CheckTraceHullAttack( const Vector vecStartOffset, float flDist, int iDamage, int iDmgType, int iDmgTypeMod ){
	return CheckTraceHullAttack(vecStartOffset, flDist, iDamage, iDmgType, iDmgTypeMod, NULL);
}




//MODDD - version of the same method below that can expect the 2nd damge mask too.
//TODO - send the entity (monster) calling this method as a paramter and use it to see if the thing hit matches its "m_hEnemy" (if this calling entity has one).
//       If it does not match the enemy, try separate checks in case something else (crate, friend, etc.) was accidentally hit?
//       ...right now it just does the re-checks if the world was hit.  BUT can choose to return the world anyways if no other entity is hit by rechecks.
CBaseEntity* CBaseMonster::CheckTraceHullAttack( const Vector vecStartOffset, float flDist, int iDamage, int iDmgType, int iDmgTypeMod, TraceResult* out_traceResult )
{
	TraceResult tr;

	if (IsPlayer())
		UTIL_MakeVectors( pev->angles );
	else
		UTIL_MakeAimVectors( pev->angles );

	Vector vecStart = pev->origin + vecStartOffset;
	vecStart.z += pev->size.z * 0.5;
	Vector vecEnd = vecStart + (gpGlobals->v_forward * flDist );
	//Vector vecEnd = pev->origin + (gpGlobals->v_forward * flDist );

	edict_t* thingHit = NULL;
	UTIL_TraceHull( vecStart, vecEnd, dont_ignore_monsters, head_hull, ENT(pev), &tr );

	//DebugLine_Setup(0, vecStart, vecEnd, tr.flFraction);

	//by default.
	thingHit = tr.pHit;


	if(tr.pHit == NULL){
		//misssed. it is possible we are swinging at something below us. Do a re-check for slightly lower.
		TraceResult trB;
		
		Vector vecStartB = pev->origin + vecStartOffset + Vector(0, 0, pev->size.z*0.4);
		Vector vecEndB = vecStartB + (gpGlobals->v_forward * flDist*1.05 ) + Vector(0, 0, -flDist*1.05/3.6);
		UTIL_TraceHull( vecStartB, vecEndB, dont_ignore_monsters, head_hull, ENT(pev), &trB );
		
		//if(trB.pHit!=NULL)easyForcePrintLine("ALT A? %s", STRING(trB.pHit->v.classname));
		//else easyForcePrintLine("ALT A? null");

		if(trB.pHit != NULL && strcmp(STRING(trB.pHit ->v.classname), "worldspawn") != 0){
			//hit something, and it isn't worldpawn? Use that instead!
			//easyForcePrintLine("ALT A!");
			thingHit = trB.pHit;
		}else{
			//hit sound anyways?
			//TEXTURETYPE_PlaySound(&tr, vecStart, vecEnd, 0);
		}

		/*
		CBaseEntity* pPlayerEntityScan = NULL;
		while( (pPlayerEntityScan = UTIL_FindEntityByClassname(pPlayerEntityScan, "player")) != NULL){
			CBasePlayer* tempPlayer = static_cast<CBasePlayer*>(pPlayerEntityScan);
			tempPlayer->debugVect1Draw = TRUE;
			tempPlayer->debugVect1Start = vecStartB;
			tempPlayer->debugVect1End = vecEndB + (vecEndB - vecStartB)*trB.flFraction;
			tempPlayer->debugVect1Success = (trB.flFraction < 1.0);
		}
		*/
	}else if(tr.pHit != NULL && strcmp(STRING(tr.pHit->v.classname), "worldspawn") == 0){
		// hit the world? is it possible we're swinging at something above us from an incline? Do a re-check for slightly higher.
		TraceResult trB;
		
		Vector vecStartB = pev->origin + vecStartOffset + Vector(0, 0, pev->size.z*0.6);
		Vector vecEndB = vecStartB + (gpGlobals->v_forward * flDist*1.05 ) + Vector(0, 0, flDist*1.05/2.7);
		UTIL_TraceHull( vecStartB, vecEndB, dont_ignore_monsters, head_hull, ENT(pev), &trB );
		
		//easyForcePrintLine("ALT B?");
		if(trB.pHit != NULL && strcmp(STRING(trB.pHit ->v.classname), "worldspawn") != 0){
			//hit something, and it isn't worldpawn? Use that instead!
			//easyForcePrintLine("ALT B!");
			thingHit = trB.pHit;
		}else{
			//hit sound anyways?
			//TEXTURETYPE_PlaySound(&tr, vecStartB, vecEndB, 0);
		}
	}else{
		//original hit  wasn't worldspawn? ok.
		
	}

	/*
	if( tr.pHit){
		easyForcePrintLine("DID WE DO IT?? %s %d", STRING(tr.pHit->v.classname), iDamage);
	}else{
		easyForcePrintLine("DID WE DO IT?? none %d", iDamage);
	}
	*/

	if ( thingHit ){
		CBaseEntity *pEntity = CBaseEntity::Instance( thingHit );
		
		if(pEntity != NULL){
			Vector targetEnd = tr.vecEndPos;

			if(m_hEnemy != NULL && pEntity->edict() != m_hEnemy->edict() ){
				// Hit something, but it wasn't my intended target?  Do I hate it anyway?
				//CBaseMonster* monTest = m_hEnemy->GetMonsterPointer();
				//if(monTest != NULL){
					int theRel = IRelationship(pEntity);
					if(theRel > R_NO || theRel == R_FR){
						// I hate whatever got hit anyway?  Fall through to normal behavior then.

					}else{
						// Thing I hit is a friendly or to be ignored?  Don't waste time attacking level geometry if the enemy
						// is still within a straight line trace and close to the point hit.
						// (Count as a hit to the enemy instead, if the enemyis close enough to the point on something unintended hit)
						Vector t_vecStart = EyePosition();
						Vector t_vecEnd = m_hEnemy->BodyTargetMod(t_vecStart);
						Vector pointDelta = t_vecEnd - t_vecStart;
						TraceResult t_tr;

						UTIL_TraceLine(t_vecStart, t_vecEnd, dont_ignore_monsters, ENT(pev), &t_tr);

						BOOL somethingIHateHit = FALSE;
						CBaseEntity* t_hitEnt;

						if(t_tr.flFraction < 1.0 && t_tr.pHit != NULL){
							// Is the thing hit something I dislike + beyond?
							if(t_tr.pHit == m_hEnemy.Get()){
								// good to go!
								somethingIHateHit = TRUE;
								t_hitEnt = m_hEnemy;
							}else{
								// Is it something that I at least want to hurt?
								CBaseEntity* t_hitTest = CBaseEntity::Instance(t_tr.pHit);
								if(t_hitTest != NULL){
									int t_theRel = IRelationship(t_hitTest);
									if(t_theRel > R_NO || t_theRel == R_FR){
										// ok.
										somethingIHateHit = TRUE;
										t_hitEnt = t_hitTest;
									}
								}
							}
						}//trace check

						if(somethingIHateHit){
							// ok, how's the distance?
							//t_vecEnd = VecBModelOrigin( t_hitEnt->pev ) ???

							// This essentially moves the origin of the target to the corner nearest the player to test to see 
							// if it's "hull" is in the view cone
							Vector clampedHitRelative = UTIL_ClampVectorToBoxNonNormalized( pointDelta, t_hitEnt->pev->size * 0.5 );

							Vector closestPointOnBox = t_vecStart + clampedHitRelative;
							if (EASY_CVAR_GET_DEBUGONLY(playerUseDrawDebug) == 2) {
								DebugLine_SetupPoint(closestPointOnBox, 0, 0, 255);
							}

							// Use the closest point on the enemy hitbox to me for determining distance instead,
							// precision this close up can be important
							Vector newPointDelta = (closestPointOnBox - targetEnd);
							Vector newPointDelta_ALT = (t_vecEnd - targetEnd);
							
							float theDist = DistanceFromDelta(newPointDelta);
							float theDist2D = Distance2DFromDelta(newPointDelta);
							float theDist_ALT = DistanceFromDelta(newPointDelta_ALT);
							float theDist2D_ALT = Distance2DFromDelta(newPointDelta_ALT);

							//DebugLine_SetupPoint(4, closestPointOnBox, 255, 0, 255);
							//DebugLine_SetupPoint(4, t_vecStart, 0, 255, 0);

							//if(theDist < pev->size.x * 0.78 && theDist2D < pev->size.z * 0.65){
							if(theDist < flDist * 0.52 && theDist2D < flDist * 0.44){
								// close enough to the intended enemy?  Hit em' instead, save other vars for the blood call
								// coming soon
								pEntity = t_hitEnt;
								vecStart = t_vecStart;
								vecEnd = t_vecEnd;
								targetEnd = t_tr.vecEndPos;
								tr = t_tr;
							}
						}//somethingIHateHit
						
					}//relationship of thing hit check
				//}//monTest != NULL Check
			}// hitent - m_hEnemy mismatch check


			if (iDamage > 0 ){
				
				pEntity->TraceAttack_Traceless(pev, (float)iDamage, (vecEnd - vecStart).Normalize(), iDmgType, iDmgTypeMod);

				pEntity->TakeDamage( pev, pev, iDamage, iDmgType, iDmgTypeMod );
				//MODDD - draw blood.
				UTIL_fromToBlood(this, pEntity, (float)iDamage, flDist, &targetEnd, &vecStart, &vecEnd);

				/*
				for(int i = 0; i < 10; i++){
					UTIL_BloodStream(tr.pHit->, UTIL_RandomBloodVector(), BloodColor(), RANDOM_LONG(50, 100));
				}
				*/
			}
		}

		if(out_traceResult != NULL){
			// copy our lineTrace to the destination if provided.
			*out_traceResult = tr;
		}

		return pEntity;
	}

	// don't copy anything?
	return NULL;
}




//=========================================================
// FInViewCone - returns true is the passed ent is in
// the caller's forward view cone. The dot product is performed
// in 2d, making the view cone infinitely tall. 
//=========================================================
BOOL CBaseMonster::FInViewCone ( CBaseEntity *pEntity )
{
	//return FALSE;

	Vector2D	vec2LOS;
	float flDot;

	UTIL_MakeVectors ( pev->angles );
	
	vec2LOS = ( pEntity->pev->origin - pev->origin ).Make2D();
	vec2LOS = vec2LOS.Normalize();

	flDot = DotProduct (vec2LOS , gpGlobals->v_forward.Make2D() );

	if ( flDot > m_flFieldOfView )
	{
		//if(FClassnameIs(this->pev, "monster_human_assault")&&FClassnameIs(pEntity->pev, "player"))easyForcePrintLine("FInViewCone: %s%d : %s dot:%.2f fov:%.2f pass:%d", this->getClassname(), monsterID, pEntity->getClassname(), flDot, m_flFieldOfView, 1);
		return TRUE;
	}
	else
	{
		//if(FClassnameIs(this->pev, "monster_human_assault")&&FClassnameIs(pEntity->pev, "player"))easyForcePrintLine("FInViewCone: %s%d : %s dot:%.2f fov:%.2f pass:%d", this->getClassname(), monsterID, pEntity->getClassname(), flDot, m_flFieldOfView, 0);
		return FALSE;
	}
}

//=========================================================
// FInViewCone - returns true is the passed vector is in
// the caller's forward view cone. The dot product is performed
// in 2d, making the view cone infinitely tall. 
//=========================================================
BOOL CBaseMonster::FInViewCone ( Vector *pOrigin )
{


	Vector2D	vec2LOS;
	float	flDot;

	UTIL_MakeVectors ( pev->angles );
	
	vec2LOS = ( *pOrigin - pev->origin ).Make2D();
	vec2LOS = vec2LOS.Normalize();

	flDot = DotProduct (vec2LOS , gpGlobals->v_forward.Make2D() );

	if ( flDot > m_flFieldOfView )
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

//=========================================================
// FVisible - returns true if a line can be traced from
// the caller's eyes to the target
//=========================================================
BOOL CBaseEntity::FVisible ( CBaseEntity *pEntity )
{
	TraceResult tr;
	Vector		vecLookerOrigin;
	Vector		vecTargetOrigin;
	
	if (FBitSet( pEntity->pev->flags, FL_NOTARGET ))
		return FALSE;

	// don't look through water
	//MODDD - some monsters that want to surface to do a ranged attack in the water, like archers,
	//        may need the ability to see through water.
	//This if-then is the addition.
	if(SeeThroughWaterLine() == FALSE){
		if ((pev->waterlevel != 3 && pEntity->pev->waterlevel == 3) 
			|| (pev->waterlevel == 3 && pEntity->pev->waterlevel == 0))
			return FALSE;
	}

	//MODDD - hold on. If we have a method just for getting the Eye Position of a given monster in case it has some special way of doing it,
	//        why don't we use that method?  The FVisible for a target (vecOrigin) below even does that!
	//vecLookerOrigin = pev->origin + pev->view_ofs;//look through the caller's 'eyes'
	vecLookerOrigin = EyePosition();

	vecTargetOrigin = pEntity->EyePosition();

	UTIL_TraceLine(vecLookerOrigin, vecTargetOrigin, ignore_monsters, ignore_glass, ENT(pev)/*pentIgnore*/, &tr);
	
	if (tr.flFraction != 1.0)
	{
		return FALSE;// Line of sight is not established
	}
	else
	{
		return TRUE;// line of sight is valid.
	}
}//END OF FVisible

//=========================================================
// FVisible - returns true if a line can be traced from
// the caller's eyes to the target vector
//=========================================================
BOOL CBaseEntity::FVisible ( const Vector &vecTargetOrigin )
{
	TraceResult tr;
	Vector		vecLookerOrigin;
	
	vecLookerOrigin = EyePosition();//look through the caller's 'eyes'

	//MODDD - also why was a similar waterlevel check absent?  Yes, we don't have a target entity provided so there is no "pev->waterlevel" of that one to
	//        take, but we can just as well check the contents of vecTargetOrigin.  Water creatures would be royally fucked without the ability to do that.
	////////////////////////////////////////////////////////////////////////////////////////
	if(SeeThroughWaterLine() == FALSE){
		//Is the target point in the water?
		//The target point being CONTENTS_SOLID is also possible, but we're going to assume whoever is calling this way knows what they're doing.
		int targetPointContents = UTIL_PointContents(vecTargetOrigin);

		if ((pev->waterlevel != 3 && targetPointContents == CONTENTS_WATER) 
			|| (pev->waterlevel == 3 && targetPointContents != CONTENTS_WATER ))
			return FALSE;
	}
	////////////////////////////////////////////////////////////////////////////////////////


	UTIL_TraceLine(vecLookerOrigin, vecTargetOrigin, ignore_monsters, ignore_glass, ENT(pev)/*pentIgnore*/, &tr);
	
	if (tr.flFraction != 1.0)
	{
		return FALSE;// Line of sight is not established
	}
	else
	{
		return TRUE;// line of sight is valid.
	}
}//END OF FVisible



//MODDD - now FVisible versions that can take a point to compare to another point or entity as well.
//MODDD MAJOR NOTE - If we want to check the waterlevel of the origin itself, which may or may not be the same as
//                   the waterlevel at the entity's eye point, we need to check that outside of this call before calling it
//                   (along with the "SeeThroughWaterLine()" exception).
//                   ...nah, just use the pev->origin of the Looker anyways (pev->waterlevel's for that point), this won't be static.
BOOL CBaseEntity::FVisible (const Vector& vecLookerOrigin, CBaseEntity *pEntity )
{
	TraceResult tr;
	Vector		vecTargetOrigin;
	
	if (FBitSet( pEntity->pev->flags, FL_NOTARGET ))
		return FALSE;

	// don't look through water
	//MODDD - some monsters that want to surface to do a ranged attack in the water, like archers,
	//        may need the ability to see through water.
	//This if-then is the addition.
	if(SeeThroughWaterLine() == FALSE){
		int lookerPointContents = UTIL_PointContents(vecLookerOrigin);

		if ((lookerPointContents != CONTENTS_WATER && pEntity->pev->waterlevel == 3) 
			|| (lookerPointContents == CONTENTS_WATER && pEntity->pev->waterlevel == 0))
			return FALSE;
	}

	vecTargetOrigin = pEntity->EyePosition();

	UTIL_TraceLine(vecLookerOrigin, vecTargetOrigin, ignore_monsters, ignore_glass, ENT(pev)/*pentIgnore*/, &tr);
	
	if (tr.flFraction != 1.0)
	{
		return FALSE;// Line of sight is not established
	}
	else
	{
		return TRUE;// line of sight is valid.
	}
}//END OF FVisible

BOOL CBaseEntity::FVisible (const Vector& vecLookerOrigin, const Vector &vecTargetOrigin )
{
	TraceResult tr;
	
	if(SeeThroughWaterLine() == FALSE){
		//Is the target point in the water?
		//The target point being CONTENTS_SOLID is also possible, but we're going to assume whoever is calling this way knows what they're doing.
		int lookerPointContents = UTIL_PointContents(vecLookerOrigin);
		int targetPointContents = UTIL_PointContents(vecTargetOrigin);

		if ((lookerPointContents != CONTENTS_WATER && targetPointContents == CONTENTS_WATER) 
			|| (lookerPointContents == CONTENTS_WATER && targetPointContents != CONTENTS_WATER ))
			return FALSE;
	}
	

	UTIL_TraceLine(vecLookerOrigin, vecTargetOrigin, ignore_monsters, ignore_glass, ENT(pev)/*pentIgnore*/, &tr);
	
	if (tr.flFraction != 1.0)
	{
		return FALSE;// Line of sight is not established
	}
	else
	{
		return TRUE;// line of sight is valid.
	}
}//END OF FVisible





/*
================
TraceAttack
================
*/
//CBaseEntity's "TraceAttack" method doesn't seem to be used very often, if at all.  All NPCs are of "CBaseMonster" which never calls its parent class (CBaseEntity)'s "TraceAttack".
GENERATE_TRACEATTACK_IMPLEMENTATION(CBaseEntity)
{
	if ( pev->takedamage )
	{
		AddMultiDamage( pevAttacker, this, flDamage, bitsDamageType, bitsDamageTypeMod );

		//MODDD - old way.
		//SpawnBlood(vecOrigin, blood, flDamage);// a little surface blood.
		if(useBloodEffect && CanMakeBloodParticles()){
			Vector vecBloodOrigin = ptr->vecEndPos - vecDir * 4;
			SpawnBlood(vecBloodOrigin, flDamage);// a little surface blood.

			TraceBleed( flDamage, vecDir, ptr, bitsDamageType, bitsDamageTypeMod );
		}

		// NOTICE - useBulletHitSound will be checked for in whatever called TraceAttack so that other places
		// can play custom hit sounds (like FireBullets).  That is, TraceAttack determines whether the caller can play its hit sound.
		// Things that ricochet want to deny this hit sound, it's needless extra noise.

		//if(useBulletHitSound && (pevAttacker != NULL && (pevAttacker->renderfx & (ISPLAYER | ISNPC))) ){
		
		//UTIL_playFleshHitSound(pev);

		//By default, if a monster wants to play a bullet hit sound on me, allow it.
		//if(useBulletHitSound)*useBulletHitSound=TRUE;
		
	}
}


//MODDD - NEW METHOD.  Not everything needs to override this, just things that need to interpret
// damage meant for traceattack without calling traceattack the usual way, in their own way like zombies.
// Calling parent methods is necessary like for TraceAttack and TakeDamage.
void CBaseEntity::TraceAttack_Traceless(entvars_t* pevAttacker, float flDamage, Vector vecDir, int bitsDamageType, int bitsDamageTypeMod) {

	g_rawDamageCumula += flDamage;


}//TraceAttack_Traceless


//MODDD - CBaseEntity's TakeDamage and Killed moved to here.
// inflict damage on this entity.  bitsDamageType indicates type of damage inflicted, ie: DMG_CRUSH
GENERATE_TAKEDAMAGE_IMPLEMENTATION(CBaseEntity)
{
	Vector vecTemp;

	if (!pev->takedamage)
		return 0;

	// UNDONE: some entity types may be immune or resistant to some bitsDamageType
	
	// if Attacker == Inflictor, the attack was a melee or other instant-hit attack.
	// (that is, no actual entity projectile was involved in the attack so use the shooter's origin). 
	if ( pevAttacker == pevInflictor )	
	{
		vecTemp = pevInflictor->origin - ( VecBModelOrigin(pev) );
	}
	else
	// an actual missile was involved.
	{
		//MODDD - NOTE. ...oh.  no difference.  okay guys.
		vecTemp = pevInflictor->origin - ( VecBModelOrigin(pev) );
	}

// this global is still used for glass and other non-monster killables, along with decals.
	g_vecAttackDir = vecTemp.Normalize();
	
// save damage based on the target's armor level

// figure momentum add (don't let hurt brushes or other triggers move player)


	//the "(pev->movetype == MOVETYPE_TOSS && pev->flags & FL_ONGROUND)" condition was added in case this monster is a tossable that is against the ground. If so it can also be pushed.
	
	if(EASY_CVAR_GET_DEBUGONLY(baseEntityDamagePushNormalMulti) != 0 || EASY_CVAR_GET_DEBUGONLY(baseEntityDamagePushVerticalBoost) != 0 || EASY_CVAR_GET_DEBUGONLY(baseEntityDamagePushVerticalMulti) ){
		if ((!FNullEnt(pevInflictor)) && (pev->movetype == MOVETYPE_WALK || pev->movetype == MOVETYPE_STEP || (pev->movetype == MOVETYPE_TOSS && pev->flags & FL_ONGROUND)) && (pevAttacker->solid != SOLID_TRIGGER) )
		{
			Vector vecDir = pev->origin - (pevInflictor->absmin + pevInflictor->absmax) * 0.5;
			vecDir = vecDir.Normalize();

			//MODDD - note. This can cause hit entities to glitch out like mad in their line of movement when the velocity pushes them against the ground.
			//        Would forcing them to lift into the air a bit help?

			
			//float flForce = flDamage * ((32 * 32 * 72.0) / (pev->size.x * pev->size.y * pev->size.z)) * 5;
			float sizeFactor = ((32 * 32 * 72.0) / (pev->size.x * pev->size.y * pev->size.z));
			float flForce = flDamage * sizeFactor * EASY_CVAR_GET_DEBUGONLY(baseEntityDamagePushNormalMulti);
			

			
			//if (flForce > 1000.0) 
			//	flForce = 1000.0;
			//pev->velocity = pev->velocity + vecDir * flForce;


			Vector newVelocity = pev->velocity + vecDir * flForce + Vector(0, 0, EASY_CVAR_GET_DEBUGONLY(baseEntityDamagePushVerticalBoost) + EASY_CVAR_GET_DEBUGONLY(baseEntityDamagePushVerticalMulti)*sizeFactor   );

			
			if(newVelocity.z < EASY_CVAR_GET_DEBUGONLY(baseEntityDamagePushVerticalMinimum)){
				//A push less than this in the Z coord is not allowed.
				newVelocity.z = EASY_CVAR_GET_DEBUGONLY(baseEntityDamagePushVerticalMinimum);
			}
			
			//vector / oldlength = normal
			//normal * newlength = right

			//(vector / oldlength) * newlength = right ...
			
			//oldlength = sqrt(vector.x^2 + vector,y^2 + vector.z^2)
			//oldlength = sqrt(dot prod(vector*vector)) ...

			//vector / (sqrt(dotprod(vector*vector)) * newlength = right.

			//1600 * (1000 / 1600)

			if(pev->flags & FL_ONGROUND){
				::UTIL_SetOrigin(pev, Vector(pev->origin.x, pev->origin.y, pev->origin.z + 0.3));
				pev->flags &= ~FL_ONGROUND;  //is this ok?
				
				//NOTICE - to have any effective pushing beyond a little weird-looking glitchiness, we need this MOVETYPE_TOSS.
				//Other movetypes like STEP can instantly lock to the ground.
				//If it were formal for entities meant to be pushed to revert movetype to MOVETYPE_STEP on touching the ground since going airborne (however slight),
				//this would be more effective. Unknown if just having a MOVETYPE_TOSS permanently otherwise is even an issue come to think of it?
				//pev->movetype = MOVETYPE_TOSS;
			}

			//maxed at 1000.
			if (newVelocity.Length() > 1000.0) 
				newVelocity = newVelocity * (1000.0 / newVelocity.Length());

			pev->velocity = newVelocity; //and apply.
		}
	}

// do the damage
	pev->health -= flDamage;
	if (pev->health <= 0)
	{
		Killed( pevAttacker, GIB_NORMAL );
		return 0;
	}

	return 1;
}//TakeDamage




//NEW METHOD.  How far should a monster be pushed?
// Larger monsters are affected less by the same force.
//NOTE: zombie does its own knockback from several other damage sources separately.
void CBaseEntity::Knockback(const int knockbackAmount, const Vector& knockbackDir) {

	float forceAmountMulti;
	Vector tempEnBoundDelta = (this->pev->absmax - this->pev->absmin);
	float tempEnSize = tempEnBoundDelta.x * tempEnBoundDelta.y * tempEnBoundDelta.z;

	if (tempEnSize < 16000) {  //headcrab size: 13824
		// I go flyin'!
		forceAmountMulti = 1.2;
	}
	else if (tempEnSize <= 800000) {  //size of agrunt: about 348160
		forceAmountMulti = 1 + -0.99 * ((tempEnSize - 16000) / (800000 - 16000));
	}
	else {
		// OH GOD ITS HUGE.
		forceAmountMulti = 0.01;
	}


	Vector attackDirRev_2D = Vector(knockbackDir.x, knockbackDir.y, 0).Normalize();

	float tossAmount = (220 + knockbackAmount * 8.5) * forceAmountMulti;

	pev->flags &= ~FL_ONGROUND;
	pev->origin.z += 2;
	// variation between tossAmount itself and 12% more.  (base behavior was 33% more)
	pev->velocity = attackDirRev_2D * RANDOM_FLOAT(tossAmount, tossAmount * 1.12);
	// And a tiny bit more z for more overkill.  Why not.
	//pev->velocity.z += tossAmount * 0.035;
	pev->velocity.z += tossAmount * 0.07;

	pev->groundentity = NULL;

}//Knockback







//MODDD - new
BOOL CBaseEntity::ChangeHealthFiltered(entvars_t* pevAttacker, float flDamage) {
	BOOL canDoDmg = FALSE;

	if (EASY_CVAR_GET_DEBUGONLY(nothingHurts) == 0) {
		canDoDmg = TRUE;
	}
	else if (EASY_CVAR_GET_DEBUGONLY(nothingHurts) == 1) {
		canDoDmg = FALSE;
	}else if (EASY_CVAR_GET_DEBUGONLY(nothingHurts) == 2) {
		// only the player can deal damage.
		CBaseEntity* attacka;
		if (pevAttacker != NULL) {
			attacka = CBaseEntity::Instance(pevAttacker);
		}else {
			attacka = NULL;
		}
		
		if (attacka != NULL && attacka->IsPlayer()) {
			canDoDmg = TRUE;
		}
	}//END Of "nothingHurts" CVar check.

	if (canDoDmg) {
		// ordinary take-damage.
		pev->health -= flDamage;
		return TRUE;
	}else {
		g_rawDamageCumula = 0;
		return FALSE;
	}
}//ChangeHealthFiltered




//MODDD - new.  Similar to CBaseEntity's version, but a few more variables to take advantage of for monsters receiving damage.
BOOL CBaseMonster::ChangeHealthFiltered(entvars_t* pevAttacker, float flDamage) {
	BOOL canDoDmg = FALSE;

	if (EASY_CVAR_GET_DEBUGONLY(nothingHurts) == 0) {
		canDoDmg = TRUE;
	}
	else if (EASY_CVAR_GET_DEBUGONLY(nothingHurts) == 1) {
		canDoDmg = FALSE;
	}
	else if (EASY_CVAR_GET_DEBUGONLY(nothingHurts) == 2) {
		// only the player can deal damage.
		CBaseEntity* attacka;
		if (pevAttacker != NULL) {
			attacka = CBaseEntity::Instance(pevAttacker);
		}
		else {
			attacka = NULL;
		}

		if (attacka != NULL && attacka->IsPlayer()) {
			canDoDmg = TRUE;
		}
	}//END Of "nothingHurts" CVar check.

	if (canDoDmg) {
		if (this->blockDamage == TRUE) {
			// don't take damage.
			g_rawDamageCumula = 0;  //whoopsie
			return FALSE;
		}
		else if (this->buddhaMode == TRUE) {
			// allow the loss of health, but have a minimum of 1 hit point.
			pev->health = max(pev->health - flDamage, 1);
			return TRUE;
		}
		else {
			// ordinary take-damage.
			pev->health -= flDamage;
			return TRUE;
		}
	}
	else {
		g_rawDamageCumula = 0;
		return FALSE;
	}

}//ChangeHealthFiltered




// give health
int CBaseEntity::TakeHealth( float flHealth, int bitsDamageType )
{
	if (!pev->takedamage)
		return 0;

// heal
	if ( pev->health >= pev->max_health )
		return 0;

	pev->health += flHealth;

	if (pev->health > pev->max_health)
		pev->health = pev->max_health;

	return 1;
}




//void CBaseEntity::Killed( entvars_t *pevAttacker, int iGib )
GENERATE_KILLED_IMPLEMENTATION(CBaseEntity)
{
	pev->takedamage = DAMAGE_NO;
	pev->deadflag = DEAD_DEAD;

	UTIL_Remove( this );
}


// Meant to be called by combat-related methods (if not only TraceAttack) to determine direction, if wanted (retail).
void CBaseEntity::SpawnBlood(const Vector& vecSpot, float flDamage) {
	int theBlood = BloodColor();

	if (!UTIL_ShouldShowBlood(theBlood)) {
		return;
	}

	// NOTICE: pev->health alone isn't enough of an accurate sign as to whether this shot will kill the monster
	// (increased blood effect).
	// gMultiDamage could be holding damage being stored up for the TakeDamage call coming up, so let that
	// be subtracted from health to see whether any other accumulated damage also leads to this being the killing
	// blow.  Keep in mind this allows other shots in the same frame (like one shotgun blast) to all be killing shots
	// with stupid amounts of blood each, which might be ok.  If not, some flag in gMultiDamage like 'deadBloodSpawned'
	// that gets reset in 'ClearMultiDamage' would be fine.
	float healthEstimate = pev->health - gMultiDamage.amount;

	if (EASY_CVAR_GET(sv_bloodparticlemode) == 1 || EASY_CVAR_GET(sv_bloodparticlemode) == 2) {
		// NOTE - ignores bloodDir and uses random blood vectors in each place
		int drawBloodVersionValue = 0;
		int drawTripleBloodValue = 1;

		// any damage over a limit does not contribute to extra blood.
		float extraBloodFactor = (min(flDamage, 30) / 30);


		if (pev->deadflag == DEAD_NO) {

			if (healthEstimate <= flDamage) {
				// killing blow?
				UTIL_BloodStream(vecSpot, UTIL_RandomBloodVector(), theBlood, (int)RANDOM_LONG(74, 95) + (int)(30 * extraBloodFactor));
				UTIL_BloodStream(vecSpot, UTIL_RandomBloodVector(), theBlood, (int)RANDOM_LONG(74, 95) + (int)(30 * extraBloodFactor));
				UTIL_BloodStream(vecSpot, UTIL_RandomBloodVector(), theBlood, (int)RANDOM_LONG(74, 95) + (int)(30 * extraBloodFactor));
			}
			else {
				// normal
				UTIL_BloodStream(vecSpot, UTIL_RandomBloodVector(), theBlood, RANDOM_LONG(5, 8) + (int)(RANDOM_FLOAT(4, 8) * extraBloodFactor));
				UTIL_BloodStream(vecSpot, UTIL_RandomBloodVector(), theBlood, RANDOM_LONG(5, 8) + (int)(RANDOM_FLOAT(4, 8) * extraBloodFactor));
				UTIL_BloodStream(vecSpot, UTIL_RandomBloodVector(), theBlood, RANDOM_LONG(5, 8) + (int)(RANDOM_FLOAT(4, 8) * extraBloodFactor));
			}
		}
		else if (healthEstimate == DEAD_DYING) {
			// dying
			UTIL_BloodStream(vecSpot, UTIL_RandomBloodVector(), theBlood, RANDOM_LONG(15, 21) + (int)(18 * extraBloodFactor));
			UTIL_BloodStream(vecSpot, UTIL_RandomBloodVector(), theBlood, RANDOM_LONG(15, 21) + (int)(18 * extraBloodFactor));
			UTIL_BloodStream(vecSpot, UTIL_RandomBloodVector(), theBlood, RANDOM_LONG(15, 21) + (int)(18 * extraBloodFactor));
		}
		else {
			// DEAD.  Similar, but spray it further up to be more noticeable.
			UTIL_BloodStream(vecSpot, UTIL_RandomBloodVectorHigh(), theBlood, RANDOM_LONG(12, 18) + (int)(15 * extraBloodFactor));
			UTIL_BloodStream(vecSpot, UTIL_RandomBloodVectorHigh(), theBlood, RANDOM_LONG(12, 18) + (int)(15 * extraBloodFactor));
			UTIL_BloodStream(vecSpot, UTIL_RandomBloodVectorHigh(), theBlood, RANDOM_LONG(12, 18) + (int)(15 * extraBloodFactor));
		}

	}//END OF alpha check
	if (EASY_CVAR_GET(sv_bloodparticlemode) == 0 || EASY_CVAR_GET(sv_bloodparticlemode) == 2) {
		// RETAIL CALL.
		// Is it still a good idea to assume g_vecAttackDir is always set as it was (hopefully?) in retail?
		UTIL_BloodDrips(vecSpot, g_vecAttackDir, theBlood, (int)flDamage);
	}
	
}//SpawnBlood



void CBaseEntity::SpawnBloodSlash(float flDamage, const Vector& vecDrawLoc, const Vector& vecTraceLine) {
	SpawnBloodSlash(flDamage, vecDrawLoc, vecTraceLine, FALSE);
}

// takes a TraceResult ONLY so that info like the surface it hit 
void CBaseEntity::SpawnBloodSlash(float flDamage, const Vector& vecDrawLoc, const Vector& vecTraceLine, const BOOL& extraBlood) {
	int theBlood = BloodColor();

	if (!CanMakeBloodParticles() || !UTIL_ShouldShowBlood(theBlood)) {
		return;
	}

	Vector vecDirUp = Vector(0, 0, 1);
	Vector vecCross = CrossProduct(vecTraceLine, vecDirUp);

	//draw blood across "vecCross":

	for (int i = 0; i <= 4; i++) {
		Vector vecThisDrawLoc = vecDrawLoc + vecCross * (i - 2) * 3.5;
		float bloodAmount;

		if (!extraBlood) {
			bloodAmount = RANDOM_LONG(4, 7);
		}else {
			bloodAmount = RANDOM_LONG(22, 40);
		}

		//UTIL_BloodStream(vecThisDrawLoc, UTIL_RandomBloodVector(), theBlood, bloodAmount);
		UTIL_SpawnBlood(vecThisDrawLoc, theBlood, bloodAmount);
	}
}//SpawnBloodSlash



//=========================================================
// TraceAttack
//=========================================================

GENERATE_TRACEATTACK_IMPLEMENTATION(CBaseMonster)
{
	if ( pev->takedamage )
	{

		if ( !((bitsDamageType & DMG_BLAST) || (bitsDamageTypeMod & DMG_HITBOX_EQUAL) )) {
			// Let logic in other places deal with this in the same frame
			m_LastHitGroup = ptr->iHitgroup;
		}
		else {
			// has DMG_BLAST or HITBOX_EQUAL?  No precision, force the hitgroup to GENERIC.
			m_LastHitGroup = HITGROUP_GENERIC;
		}

		// And, let monsters change up damage values per place if desired
		flDamage = hitgroupDamage(flDamage, bitsDamageType, bitsDamageTypeMod, ptr->iHitgroup);

		//MODDD - surrounded by parameter.  The gargantua has customized bleeding.

		
		//MODDD - similar case below, robot requires this CVar to emit black oil blood.
		if( !(this->CanUseGermanModel() && EASY_CVAR_GET_DEBUGONLY(germanRobotBleedsOil)==0 ) && useBloodEffect && CanMakeBloodParticles()){
			Vector vecBloodOrigin = ptr->vecEndPos - vecDir * 4;
			SpawnBlood(vecBloodOrigin, flDamage);// a little surface blood.
		}

		// NOTICE - useBulletHitSound will be checked for in whatever called TraceAttack so that other places
		// can play custom hit sounds (like FireBullets).  That is, TraceAttack determines whether the caller can play its hit sound.
		// Things that ricochet want to deny this hit sound, it's needless extra noise.
		/*
		if(useBulletHitSound && (pevAttacker != NULL && (pevAttacker->renderfx & (ISPLAYER | ISNPC))) ){
			//UTIL_playFleshHitSound(pev);
		}
		*/

		//MODDD!!!
		// SpawnBlood(ptr->vecEndPos, BloodColor(), flDamage);// a little surface blood.
		
		// NOTE: "SpawnBlood" does the same thing as "UTIL_BloodStream".  Should've seen that sooner.
		// Apparently, "TraceBleed" draws the blood texture on a nearby surface (floor, wall)...

		// Can TraceBleed all the time with germancensorship off.
		// With german censorship on, check germanRobotDamageDecal before drawing robot blood (oil). 
		// If this monster has a german model replacement but this CVar is off, block the TraceBleed request.
		// Note that "CanUseGermanModel" is always false when GermanCensorship is turned off.
		// If TraceBleed is called with a monster with red blood (no german robot model provided), this will get denied anyways.
		if( !(this->CanUseGermanModel() && EASY_CVAR_GET_DEBUGONLY(germanRobotDamageDecal)==0 ) && useBloodEffect){
			TraceBleed( flDamage, vecDir, ptr, bitsDamageType, bitsDamageTypeMod );
		}

		AddMultiDamage( pevAttacker, this, flDamage, bitsDamageType, bitsDamageTypeMod );
	}//END OF pev->takedamage check

}//END OF CBaseMonster's Traceattack implementation



/*
============
TakeDamage

The damage is coming from inflictor, but get mad at attacker
This should be the only function that ever reduces health.
bitsDamageType indicates the type of damage sustained, ie: DMG_SHOCK

Time-based damage: only occurs while the monster is within the trigger_hurt.
When a monster is poisoned via an arrow etc it takes all the poison damage at once.

GLOBALS ASSUMED SET:  g_iSkillLevel
============
*/

//MODDD - new TakeDamage interception.
//MODDD - TODO. extra idea. Perhaps with a deadflag of DEAD_DYING,
//        DeadTakeDamage should be called too? Just an idea.
// AND NOTICE - g_rawDamageCumula must be set to 0 anywhere the method ends.  Before any 'return' too.
// That way next time damage is dealt, it starts fresh.
GENERATE_TAKEDAMAGE_IMPLEMENTATION(CBaseMonster){
	//TEMP
	//bitsDamageType |= DMG_POISON;

	float flDamageTake;
	Vector vecDir;

	if (!pev->takedamage) {
		g_rawDamageCumula = 0;  //whoopsie
		return 0;
	}


	////virtual BOOL	IsAlive( void ) { return (pev->deadflag != DEAD_DEAD); } inv:  == DEAD_DEAD
	//virtual BOOL	IsAlive( void ) { return (pev->deadflag == DEAD_NO); }       inv:  != DEAD_NO

	//if(!IsAlive())
	// Retail's way here. Only count as dead.
	if(pev->deadflag == DEAD_DEAD)
	{
		/*
		if ( m_MonsterState == MONSTERSTATE_SCRIPT && pev->spawnflag & SF_SCRIPT_NOINTERRUPT )
		{
			//Require the deadflag to be all the way dead, not dying.
			if(pev->deadflag == DEAD_DEAD){
				//okay to proceed, would have in retail.
			}else{
				//not okay. Don't damage scripted entities in a DYING state.
				SetConditions( bits_COND_LIGHT_DAMAGE );
				return 0;
			}
		}
		*/

		// timed damages NOT allowed for corpses.
		bitsDamageType &= ~ DMG_TIMEBASED;
		bitsDamageTypeMod &= ~ DMG_TIMEBASEDMOD;
		int deadTakeDmgRes = DeadTakeDamage( pevInflictor, pevAttacker, flDamage, bitsDamageType, bitsDamageTypeMod );
		g_rawDamageCumula = 0;
		return deadTakeDmgRes;
	}


	if (pev->deadflag == DEAD_NO && m_MonsterState != MONSTERSTATE_SCRIPT && m_MonsterState != MONSTERSTATE_PRONE) {
		OnTakeDamageSetConditions(pevInflictor, pevAttacker, flDamage, bitsDamageType, bitsDamageTypeMod);
	}

	//MODDD - old location of pain sound

	//!!!LATER - make armor consideration here!
	flDamageTake = flDamage;

	//MODDD - little edit though.  Talkmonsters and all other non-player damages take only a portion of timed damage.
	// Their health is often lower than the player's 100 (especially Talkers), so this makes these forms of damage less
	// game-breaking.
	if (bitsDamageTypeMod & (DMG_TIMEDEFFECT | DMG_TIMEDEFFECTIGNORE)) {
		if (this->isTalkMonster()) {
			flDamageTake = flDamageTake * 0.15;
		}
		else if (!IsPlayer()) {
			// non-player monster?
			flDamageTake = flDamageTake * 0.60;
		}
	}



	if(pev->deadflag == DEAD_NO){

		if(!blockTimedDamage){
			int myClassify = Classify();

			if (
				// because apaches are HUMAN_MILITARY, not MACHINE, but taking poison is zany.
				myClassify == CLASS_MACHINE || (!isOrganic()) ||
				(
					EASY_CVAR_GET_DEBUGONLY(AlienRadiationImmunity) == 1 &&
					(
						myClassify == CLASS_ALIEN_MILITARY ||
						myClassify == CLASS_ALIEN_PASSIVE ||
						myClassify == CLASS_ALIEN_MONSTER ||
						myClassify == CLASS_ALIEN_PREY ||
						myClassify == CLASS_ALIEN_PREDATOR ||
						myClassify == CLASS_PLAYER_BIOWEAPON ||
						myClassify == CLASS_ALIEN_BIOWEAPON ||
						myClassify == CLASS_BARNACLE
						)
					)
				)
			{
				// no radiation for you
				bitsDamageType &= ~DMG_RADIATION;
			}

			if (myClassify == CLASS_MACHINE || (!isOrganic())) {
				// no poison for you
				bitsDamageType &= ~DMG_POISON;
				bitsDamageTypeMod &= ~DMG_POISONHALF;
			}


			if (bitsDamageTypeMod & DMG_POISONHALF) {
				bitsDamageType |= DMG_POISON;  // let it be so
			}

			if (bitsDamageTypeMod & DMG_MAP_TRIGGER) {
				// Recently inflicted trigger damage, eh?  Did this same source of damage happen to give any timed damage?
				// If so, this defies buddha mode for this same turn.  This makes sure that standing in a trigger dealing
				// 0 direct damage but timed damage doesn't let you exploit the buddha timed damage mode to never die
				// while standing in a pool of radiation.
				if ((bitsDamageType & DMG_TIMEBASED) || (bitsDamageTypeMod & DMG_TIMEBASEDMOD)) {
					recentTimedTriggerDamage = TRUE;
				}
			}

			applyNewTimedDamage(bitsDamageType, bitsDamageTypeMod);

			// set damage type sustained
			m_bitsDamageType |= bitsDamageType;
			m_bitsDamageTypeMod |= bitsDamageTypeMod;

		}else{
			m_bitsDamageType |= (bitsDamageType & ~DMG_TIMEBASED);
			m_bitsDamageTypeMod |= (bitsDamageTypeMod & ~DMG_TIMEBASEDMOD);
		}

		//MODDD NOTE - can include PainSound in the same m_MonsterState restrictions above if wanted.
		// ...not sure what this comment was about.  But yea, painsound now below the m_bitsDamageType setting, check for that in PainSound if wanted.
		// ALTHOUGH, why play a painsound on things that, ya know, don't deal any damage?
		// Still happen on attacks that start timed damage though.
		//if (pev->deadflag == DEAD_NO)
		if(flDamage > 0 || (bitsDamageType & DMG_TIMEBASED) || (bitsDamageTypeMod & DMG_TIMEBASEDMOD)  ){
			// no pain sound during death animation.
			PainSound();// "Ouch!"
		}

		//MODDD - SAFETY.  Reset the damage bits, no reason to keep them around when applyNewTimedDamage
		// handled them and put them in other arrays if needed.
		// Should anything at a later point need to read these, put this reset later or those other places
		// earlier.  If any weird bugs happen consider these resets, retail kept the bits damages set to
		// whatever recent damage was inflicted.
		// ...nevermind, don't do this reset.  Messes with player hud showing the damages I bet, but untested.
		//m_bitsDamageType = 0;
		//m_bitsDamageTypeMod = 0;

	}else{
		// dead?  can't take any timed damages.
		m_bitsDamageType = 0;
		m_bitsDamageTypeMod = 0;
	}


	// grab the vector of the incoming attack. ( pretend that the inflictor is a little lower than it is, so the body will tend to fly upward a bit).
	//MODDD - important note!  Only the player is moved by "vecDir", otherwise that variable gets ignored
	vecDir = Vector( 0, 0, 0 );
	if (!FNullEnt( pevInflictor ))
	{
		CBaseEntity *pInflictor = CBaseEntity::Instance( pevInflictor );
		if (pInflictor)
		{
			// And lower offset for the center!  Was 10.  (that vect's subtracted)
			vecDir = ( pInflictor->Center() - Vector ( 0, 0, 15 ) - Center() ).Normalize();
			vecDir = g_vecAttackDir = vecDir.Normalize();
		}
	}



	// add to the damage total for clients, which will be sent as a single
	// message at the end of the frame
	// todo: remove after combining shotgun blasts?
	//MODDD - above are as-is comments from the codebase.
	// Why would this have to be removed after 'combinging shotgun blasts', whatever that means?  Whether that's already been done or not?
	// There is collecting the damage of each hitting shell through traceattacks and adding the damage up to give at the end
	// of a frame, not sure if it means that (which also already happens as of retail)
	if ( IsPlayer() )
	{
		if (pevInflictor) {
			pev->dmg_inflictor = ENT(pevInflictor);
		}

		pev->dmg_take += flDamageTake;

		// check for godmode or invincibility
		if ( pev->flags & FL_GODMODE )
		{
			g_rawDamageCumula = 0;  //whoopsie
			return 0;
		}
	}
	
	//if ( !FNullEnt(pevInflictor) && (pevAttacker->solid != SOLID_TRIGGER) )
	// if this is a player, move him around!
	//MODDD NOTE - only the player can have MOVETYPE_WALK.
	// the SOLID_TRIGGER check is on the attacker, not the victim.  So trigger-inflicted damage doesn't move the player
	// around I assume, although that is its own damage type now (DMG_MAP_TRIGGER) for the 2nd damagetype bitmask.
	// Let's just put this in a proper IsPlayer check to be safe.
	// Although CBasePlayer has its own TakeDamage method.  Odd, odd.
	if (IsPlayer()) {
		if ((!FNullEnt(pevInflictor)) && (pev->movetype == MOVETYPE_WALK) && (!pevAttacker || pevAttacker->solid != SOLID_TRIGGER))
		{
			pev->velocity = pev->velocity + vecDir * -DamageForce(flDamage);
		}
	}
	else {
		// Anything else?  Can be knocked back by explosive damage, if a CVar permits.
		if (EASY_CVAR_GET(sv_explosionknockback) > 0) {
			// ALSO, don't affect gravity-less movetypes, they might behave oddly with this.
			if (
				pev->deadflag < DEAD_DEAD &&
				//(pev->movetype != MOVETYPE_NONE && pev->movetype != MOVETYPE_FLY && pev->movetype != MOVETYPE_FLYMISSILE)
				// ...nevermind, only affect MOVETYPE_STEP to be safe.  Players (MOVETYPE_WALK) are already affected separately,
				// and exempt from reaching this point anyway (this is in the 'else', of a 'IsPlayer' check).
				// Now up to the monster!  Overridable.
				//(pev->movetype == MOVETYPE_STEP)
				(AffectedByKnockback())
			){
				if (bitsDamageType & DMG_BLAST) {
					Knockback(g_rawDamageCumula * 0.28 * EASY_CVAR_GET(sv_explosionknockback), -g_vecAttackDir);
				}
			}
		}
	}

	//easyPrintLine("CBaseMonster::name:%s:%d TOOK DAMAGE. health:%.2f Damage:%.2f Blast:%d Gib::N:%d A:%d", getClassname(), monsterID, pev->health, flDamage, (bitsDamageType & DMG_BLAST), (bitsDamageType & DMG_NEVERGIB), (bitsDamageType & DMG_ALWAYSGIB) );
	//easyForcePrintLine("TakeDamage. %s:%d health:%.2f gib damge bits: %d %d", this->getClassname(), monsterID, pev->health, (bitsDamageType&DMG_NEVERGIB), (bitsDamageType&DMG_ALWAYSGIB) );
	

	BOOL healthCheckBlock = FALSE;
	if (!ChangeHealthFiltered(pevAttacker, flDamageTake)) {
		// call to block the rest of the method if this returns FALSE. Sets g_rawDamageCumula already if that happens.
		// WAIT no.   Still want the AI to properly react for debugging, just ignore the pev->health == 0 check
		//return 0;
		healthCheckBlock = TRUE;
	}
	
	// HACKHACK Don't kill monsters in a script.  Let them break their scripts first
	if ( m_MonsterState == MONSTERSTATE_SCRIPT )
	{
		if(  (!m_pCine || m_pCine->CanInterrupt())  ) {

			if (monsterID == 3) {
				int x = 45;
			}

			SetConditions( bits_COND_LIGHT_DAMAGE );

			//MODDD
			// BEWARE!  Setting _DAMAGE conditions without setting m_vecEnemyLKP is a no-no!
			//setEnemyLKP()  ???

			//////////////////////////////////////////////////////////////////////////////////////
			// shorter verison of script further below
			if (pevInflictor)
			{
				if (m_hEnemy == NULL || pevInflictor == m_hEnemy->pev || !HasConditions(bits_COND_SEE_ENEMY)){
					setEnemyLKP_Investigate(pevInflictor->origin);
				}
			}else{
				setEnemyLKP_Investigate(pev->origin + ( g_vecAttackDir * 64 ));
			}
			MakeIdealYaw( m_vecEnemyLKP );
			//////////////////////////////////////////////////////////////////////////////////////
			
		}
		g_rawDamageCumula = 0;  //whoopsie
		return 0;
	}


	if(!healthCheckBlock){
		if (pev->health <= 0 )
		{
			//MODDD - removing this. We can send the inflictor to killed now.
			//g_pevLastInflictor = pevInflictor;
		
			// using g_rawDamageCumula instead of flDamageTake.
			m_lastDamageAmount = g_rawDamageCumula;

			attemptResetTimedDamage(TRUE);

			float MYHEALTH = pev->health;
		

			//MODDD - Freshly dead?  Anticipate a BecomeDead call soon.
			g_tossKilledCall = TRUE;
			g_bitsDamageType = bitsDamageType;
			g_bitsDamageTypeMod = bitsDamageTypeMod;

			if ( bitsDamageType & DMG_ALWAYSGIB )
			{
				Killed( pevInflictor, pevAttacker, GIB_ALWAYS );
			}
			else if ( bitsDamageType & DMG_NEVERGIB )
			{
				Killed( pevInflictor, pevAttacker, GIB_NEVER );
			}
			else
			{
				Killed( pevInflictor, pevAttacker, GIB_NORMAL );
			}

			// Just in case BecomeDead didn't flick this off.
			g_tossKilledCall = FALSE;


			//g_pevLastInflictor = NULL;

			g_rawDamageCumula = 0;  //ok
			return 0;
		}
		//MODDD - else, if not killed by this strike:
		else{

		
			// moved to above
			//if(!blockTimedDamage){
			//	applyNewTimedDamage(bitsDamageType, bitsDamageTypeMod);
			//}
		
		}//END OF pev->health 0 check
	}//!healthCheckBlock


	// react to the damage (get mad)
	if ( (pev->flags & FL_MONSTER) && !FNullEnt(pevAttacker) )
	{
		if ( pevAttacker->flags & (FL_MONSTER | FL_CLIENT) )
		{// only if the attack was a monster or client!
			
			if( !(bitsDamageTypeMod & (DMG_TIMEDEFFECT|DMG_TIMEDEFFECTIGNORE)) ){
				//MODDD - if this is continual damage (e.g. poison, radiation), don't allow the LKP to be updated! timed damage tells us nothing and shouldn't disturb anything.
				BOOL updatedEnemyLKP = FALSE;  //turn on if we do.

				// enemy's last known position is somewhere down the vector that the attack came from.
				if (pevInflictor)
				{
					//MODDD NOTE - "bits_COND_SEE_ENEMY" appears to still be on for monsters that have a straight line to their enemy but aren't necessarily turned to them.
					//             So the condition being off is kindof an odd choice?
					if (m_hEnemy == NULL || pevInflictor == m_hEnemy->pev || !HasConditions(bits_COND_SEE_ENEMY))
					{
						setEnemyLKP_Investigate(pevInflictor->origin);
						updatedEnemyLKP = TRUE;
					}else{
						//MODDD NOTE -don't update the LKP at all? This will happen if...
						//    (m_hEnemy != NULL && pev->inflictor != m_hEnemy->pev && HasConditions(bits_COND_SEE_ENEMY))
						//...so while looking straight at an enemy or being able to and taking damage from another source,
						//the LKP is unaffected. that is fine.
					}
				}
				else
				{
					setEnemyLKP_Investigate(pev->origin + ( g_vecAttackDir * 64 ));
					updatedEnemyLKP = TRUE;
				}

				if (monsterID == 0 || monsterID == 1) {
					int x = 45;
				}



				//MODDD - If we didn't change the LKP, why look in its direction?
				//...undone, perhaps we should look at the enemy if they damaged us? whatever, err on defalt behavior.
				//if(updatedEnemyLKP){
					MakeIdealYaw( m_vecEnemyLKP );

					//MODDD - also let the enemy know to not get stumped on investigating the LKP.
					// It makes no sense to investigate the LKP set by getting hit by something and give up on
					// going outside of cover to find the enemy. 
					//unstumpable = TRUE;
				//}

			}//END OF timed damage check


			// add pain to the conditions 
			// !!!HACKHACK - fudged for now. Do we want to have a virtual function to determine what is light and 
			// heavy damage per monster class?
			// MODDD - I hear your cries, original devs. So shall it be done!
			// Default behavior for the base monster in OnTakeDamageSetConditions, plus not triggering schedule-interrupting conditions
			// ("Took Damage Recently" or something) for timed damage, which just looks annoying. Why react to predictable damage?
			//***
			// OnTakeDamageSetConditions call moved further above to before PainSound, for letting the reaction change to LIGHT, HEAVY, or 
			// neither damage conditions being set (happens on timed damage in most cases)
		}
	}

	g_rawDamageCumula = 0;  //and done
	return 1;
}//END OF TakeDamage

//MODDD - NEW.  Overridable part of TRACEATTACK that most monsters call.
// Let monsters override to change how much damage different hitgroups do,
// preferrably still involving the skill CVars (extra multipliers are fine).
float CBaseMonster::hitgroupDamage(float flDamage, int bitsDamageType, int bitsDamageTypeMod, int iHitgroup) {
	// ALSO, only allow changes if lacking DMG_HITBOX_EQUAL.  Explosions or inprecise attacks should use
	// this damage type.  After all, a distant explosion doing headshot damage makes no sense.

	g_rawDamageCumula += flDamage;  //does not care about influence from hitboxes.

	if (!(bitsDamageTypeMod & DMG_HITBOX_EQUAL)) {
		switch (iHitgroup)
		{
		case HITGROUP_GENERIC:
			break;
		case HITGROUP_HEAD:
			return flDamage * gSkillData.monHead;
		case HITGROUP_CHEST:
			return flDamage * gSkillData.monChest;
		case HITGROUP_STOMACH:
			return flDamage * gSkillData.monStomach;
		case HITGROUP_LEFTARM:
		case HITGROUP_RIGHTARM:
			return flDamage * gSkillData.monArm;
		case HITGROUP_LEFTLEG:
		case HITGROUP_RIGHTLEG:
			return flDamage * gSkillData.monLeg;
		default:
			return flDamage;
		}
	}//END OF bitsDamageTypeMod check
	return flDamage;
}//hitgroupDamage



//MODDD - may incorporate the bitsDamageTypeMod (extra damage bitmask) in the future. For now, just the plain one works.
// take health
int CBaseMonster::TakeHealth (float flHealth, int bitsDamageType)
{
	if (!pev->takedamage)
		return 0;

	// clear out any damage types we healed.
	// UNDONE: generic health should not heal any
	// UNDONE: time-based damage

	m_bitsDamageType &= ~(bitsDamageType & ~DMG_TIMEBASED);
	
	return CBaseEntity::TakeHealth(flHealth, bitsDamageType);
}




/*
void CBaseMonster::PreThink(void)
{
	CheckTimeBasedDamage();
}
*/
//MonsterThink...?  already used though (but the checkTimeBasedDamage call could be inserted into there instead).  "Think" here is fine I believe.
// Let's not do this though.
/*
void CBaseMonster::Think(void)
{
	CBaseEntity::Think();
	//easyPrintLine("heeeee");

	
	////MODDD: REVERTMOD
	//uh.  this is bad it seems.
	//
	//if(EASY_CVAR_GET_DEBUGONLY(timedDamageAffectsMonsters) == 1){
	//	CheckTimeBasedDamage();
	//}
	//

}
*/


// TODO - let monster size influence the amount tossed for dead-tossing?
// gargantuas should be barely phased ever, see scientist for some common size threshholds.
// There's that mass overridable or whatever but who wants to do that.
// And look at damage before factoring in what's done from hitgroups (no extra push for headshot),
// think zombie does that.

//=========================================================
// DeadTakeDamage - takedamage function called when a monster's
// corpse is damaged.
//=========================================================
//int CBaseMonster::DeadTakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType, int bitsDamageTypeMod )
GENERATE_DEADTAKEDAMAGE_IMPLEMENTATION(CBaseMonster)
{
	Vector vecDir;

	// oopsie, debug test
	//DeathSound();

	//MODDD - wait, what's with the comment below?
	// Nothing is ever done with "vecDir", although this does set g_vecAttackDir.  weird.
	// ...oh.  It's a leftover paste from CBaseMonster's TAKEDAMAGE, although only the player uses vecDir.
	// And since the player never gets checked in DEADTAKEDAMAGE here... yea.

	// grab the vector of the incoming attack. ( pretend that the inflictor is a little lower than it is, so the body will tend to fly upward a bit).
	vecDir = Vector( 0, 0, 0 );
	if (!FNullEnt( pevInflictor ))
	{
		CBaseEntity *pInflictor = CBaseEntity::Instance( pevInflictor );
		if (pInflictor)
		{
			vecDir = ( pInflictor->Center() - Vector ( 0, 0, 10 ) - Center() ).Normalize();
			vecDir = g_vecAttackDir = vecDir.Normalize();
		}
	}


	//MODDD - TODO: intriguing... see later.
#if 0// turn this back on when the bounding box issues are resolved.

	pev->flags &= ~FL_ONGROUND;
	pev->origin.z += 1;
	
	// let the damage scoot the corpse around a bit.
	if ( !FNullEnt(pevInflictor) && (pevAttacker->solid != SOLID_TRIGGER) )
	{
		pev->velocity = pev->velocity + vecDir * -DamageForce( flDamage );
	}

#endif

	// kill the corpse if enough damage was done to destroy the corpse and the damage is of a type that is allowed to destroy the corpse.
	//MODDD - don't allow gibbing if DMG_NEVERGIB is in there!  Likely deliberate.
	if (!(bitsDamageType & DMG_NEVERGIB) && bitsDamageType & DMG_GIB_CORPSE )
	{
		//MODDD NOTE - the same "Killed" as running out of health the first time while alive?
		//             That's... an akward design choice. Why not a separate "CorpseKilled" or something?
		//             Other places may need to be aware they may be getting their "Killed" called from a dead monster probably wanting to gib.
		if ( pev->health <= flDamage )
		{
			pev->health = -50;
			Killed( pevInflictor, pevAttacker, GIB_ALWAYS );
			return 0;
		}
		// Accumulate corpse gibbing damage, so you can gib with multiple hits
		// MODDD - penality reduced, was 0.1.
		pev->health -= flDamage * 0.18;
	}
	
	return 1;
}

float CBaseMonster::DamageForce( float damage )
{ 
	float force = damage * ((32 * 32 * 72.0) / (pev->size.x * pev->size.y * pev->size.z)) * 5;
	
	if ( force > 1000.0) 
	{
		force = 1000.0;
	}

	return force;
}


/*
//...Not anymore. Any calls to RadiusDamage, even from RadiusDamageAutoRadius, end up getting redirected to RadiusDamageTest if the RadiusDamageDrawDebug CVar is set.
void CBaseMonster::RadiusDamageAutoRadiusTest(entvars_t* pevInflictor, entvars_t*	pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType )
{
	::RadiusDamageTest( pev->origin, pevInflictor, pevAttacker, flDamage, flDamage * 2.5, iClassIgnore, bitsDamageType, 0 );
}
*/
void RadiusDamageTest( Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, float flRadius, int iClassIgnore, int bitsDamageType, int bitsDamageTypeMod )
{

	CBaseEntity *pEntity = NULL;
	TraceResult	tr;
	float	flAdjustedDamage, falloff;
	Vector		vecSpot;

	if ( flRadius )
		falloff = flDamage / flRadius;
	else
		falloff = 1.0;

	int bInWater = (UTIL_PointContents ( vecSrc ) == CONTENTS_WATER);

	vecSrc.z += 1;// in case grenade is lying on the ground

	if ( !pevAttacker )
		pevAttacker = pevInflictor;
	

	DebugLine_ClearAll();


	easyForcePrintLine("RadiusDamageTest: Searching... intended damage:%.2f radius:%.2f", flDamage, flRadius);
	// iterate on all entities in the vicinity.
	while ((pEntity = UTIL_FindEntityInSphere( pEntity, vecSrc, flRadius )) != NULL)
	{
		//easyForcePrintLine("RadiusDamage: Scanning... %s", pEntity->getClassname());
		if ( pEntity->pev->takedamage != DAMAGE_NO )
		{
			easyForcePrintLine("RadiusDamageTest: Entity in range: %s", pEntity->getClassname());
			//continue;

			// UNDONE: this should check a damage mask, not an ignore
			if ( iClassIgnore != CLASS_NONE && pEntity->Classify() == iClassIgnore )
			{// houndeyes don't hurt other houndeyes with their attack
				continue;
			}

			// blast's don't tavel into or out of water
			if (bInWater && pEntity->pev->waterlevel == 0)
				continue;
			if (!bInWater && pEntity->pev->waterlevel == 3)
				continue;


			easyForcePrintLine("RadiusDamageTest: Calling BodyTarget.");

			vecSpot = pEntity->BodyTarget( vecSrc );
			
			easyForcePrintLine("RadiusDamageTest: Source:(%.2f,%.2f,%.2f) Bodyspot:(%.2f,%.2f,%.2f)", vecSrc.x, vecSrc.y, vecSrc.z, vecSpot.x, vecSpot.y, vecSpot.z);


			UTIL_TraceLine ( vecSrc, vecSpot, dont_ignore_monsters, ENT(pevInflictor), &tr );

			if(pEntity->IsPlayer()){
				DebugLine_Setup(0, vecSrc, vecSpot, tr.flFraction);

			}


			easyForcePrintLine("RadiusDamageTest: Trace test...");

			if ( tr.flFraction == 1.0 || tr.pHit == pEntity->edict() )
			{// the explosion can 'see' this entity, so hurt them!
				
				/*
				if (tr.fStartSolid)
				{
					// if we're stuck inside them, fixup the position and distance
					tr.vecEndPos = vecSrc;
					tr.flFraction = 0.0;
				}
				*/


				easyForcePrintLine("IS IT SOLID?! ss:%d as:%d io:%d", tr.fStartSolid, tr.fAllSolid, tr.fInOpen);

				easyForcePrintLine("RadiusDamageTest: Trace hit successful (hit the target or nothing blocking). Thing hit: %s fract:%.2f", (tr.pHit!=NULL)?STRING(tr.pHit->v.classname):"NULL", tr.flFraction );
				
				// decrease damage for an ent that's farther from the bomb.
				flAdjustedDamage = ( vecSrc - tr.vecEndPos ).Length() * falloff;
				flAdjustedDamage = flDamage - flAdjustedDamage;
			

				
				if ( flAdjustedDamage < 0 )
				{
					flAdjustedDamage = 0;

					easyForcePrintLine("RadiusDamageTest: damage was 0, blocked.");

					//MODDD - waitasec. If we're going to do "0" damage, why bother with any damage effects at all?  Skip acting on this entity.
					continue;
				}

				easyForcePrintLine("RadiusDamageTest: adjDamage:%.2f.", flAdjustedDamage );
				
				// ALERT( at_console, "hit %s\n", STRING( pEntity->pev->classname ) );
				//MODDD - both ways of taking damage send bitsDamageTypeMod.
				if (tr.flFraction != 1.0)
				{
					easyForcePrintLine("RadiusDamageTest: that same trace was a direct hit! Trace attack.");
					ClearMultiDamage( );
					//TODO - why do you put player blood at the blast center sometimes when the player & a baddie are hit at the same time???


					
					if(pEntity->IsPlayer()){

						/*
						CBasePlayer* thisNameSucks = static_cast<CBasePlayer*>(pEntity);
						thisNameSucks->debugVect1Draw = FALSE;
						thisNameSucks->debugVect2Draw = FALSE;
						thisNameSucks->debugVect3Draw = FALSE;
						thisNameSucks->debugVect4Draw = FALSE;
						thisNameSucks->debugVect5Draw = FALSE;

						//
		
						thisNameSucks->debugVect1Start = vecSrc;
						thisNameSucks->debugVect1End = vecSrc + (vecSpot-vecSrc)*tr.flFraction; //(distToClosest+1)*tr.flFraction
						thisNameSucks->debugVect1Draw = TRUE;
						thisNameSucks->debugVect1Success = TRUE;
						
						thisNameSucks->debugVect2Start = tr.vecEndPos + Vector(0, 0, -7);
						thisNameSucks->debugVect2End = tr.vecEndPos + Vector(0, 0, 7);
						thisNameSucks->debugVect2Draw = TRUE;
						thisNameSucks->debugVect2Success = FALSE;

						*/
					}
		

					//tr.vecEndPos

					pEntity->TraceAttack( pevInflictor, flAdjustedDamage, (tr.vecEndPos - vecSrc).Normalize( ), &tr, bitsDamageType, bitsDamageTypeMod );
					ApplyMultiDamage( pevInflictor, pevAttacker );
				}
				else
				{
					easyForcePrintLine("RadiusDamageTest: that same trace was not a direct hit. Forcing a takeDamage call.");
					pEntity->TakeDamage ( pevInflictor, pevAttacker, flAdjustedDamage, bitsDamageType, bitsDamageTypeMod );
				}
			}else{
				easyForcePrintLine("RadiusDamageTest: Trace hit blocked, something in the way? No hit.");
			}
		}
	}
}



//
// RadiusDamage - this entity is exploding, or otherwise needs to inflict damage upon entities within a certain range.
// 
// only damage ents that can clearly be seen by the explosion!


//From CBaseMonster to assume the position of the monster called on (pev->origin).
void CBaseMonster::RadiusDamageAutoRadius(entvars_t* pevInflictor, entvars_t*	pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType )
{
	::RadiusDamage( pev->origin, pevInflictor, pevAttacker, flDamage, flDamage * 2.5, iClassIgnore, bitsDamageType, 0 );
}
void CBaseMonster::RadiusDamageAutoRadius(entvars_t* pevInflictor, entvars_t*	pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType, int bitsDamageTypeMod )
{
	::RadiusDamage( pev->origin, pevInflictor, pevAttacker, flDamage, flDamage * 2.5, iClassIgnore, bitsDamageType, bitsDamageTypeMod );
}

//MODDD - these versions now don't belong to CBaseMonster. Why did they? They don't take anything from the monster they are called on.
void RadiusDamageAutoRadius( Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType )
{
	::RadiusDamage( vecSrc, pevInflictor, pevAttacker, flDamage, flDamage * 2.5, iClassIgnore, bitsDamageType, 0);
}
void RadiusDamageAutoRadius( Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType, int bitsDamageTypeMod )
{
	::RadiusDamage( vecSrc, pevInflictor, pevAttacker, flDamage, flDamage * 2.5, iClassIgnore, bitsDamageType, bitsDamageTypeMod );
}
//MODDD - monster versions restored too.
void CBaseMonster::RadiusDamageAutoRadius( Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType )
{
	::RadiusDamage( vecSrc, pevInflictor, pevAttacker, flDamage, flDamage * 2.5, iClassIgnore, bitsDamageType, 0);
}
void CBaseMonster::RadiusDamageAutoRadius( Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType, int bitsDamageTypeMod )
{
	::RadiusDamage( vecSrc, pevInflictor, pevAttacker, flDamage, flDamage * 2.5, iClassIgnore, bitsDamageType, bitsDamageTypeMod );
}

//MODDD - version that supports bitsDamageTypeMod below. The old one now just passes along a blank (none) bitsDamageTypeMOD value. Healthy default.
void RadiusDamage( Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, float flRadius, int iClassIgnore, int bitsDamageType )
{
	RadiusDamage(vecSrc, pevInflictor, pevAttacker, flDamage, flRadius, iClassIgnore, bitsDamageType, 0);
}
void RadiusDamage( Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, float flRadius, int iClassIgnore, int bitsDamageType, int bitsDamageTypeMod )
{

	if (EASY_CVAR_GET_DEBUGONLY(RadiusDamageDrawDebug) == 1) {
		// Draw lines for existing damage calls instead
		DebugLine_ClearAll();
	}

	if(EASY_CVAR_GET_DEBUGONLY(RadiusDamageDrawDebug) == 2){
		// pipe it to here instead
		RadiusDamageTest(vecSrc, pevInflictor, pevAttacker, flDamage, flRadius, iClassIgnore, bitsDamageType, bitsDamageTypeMod);
		return;
	}

	CBaseEntity *pEntity = NULL;
	TraceResult tr;
	float flAdjustedDamage, falloff;
	Vector vecSpot;

	if ( flRadius )
		falloff = flDamage / flRadius;
	else
		falloff = 1.0;

	int bInWater = (UTIL_PointContents ( vecSrc ) == CONTENTS_WATER);

	vecSrc.z += 1;// in case grenade is lying on the ground

	if ( !pevAttacker )
		pevAttacker = pevInflictor;


	// iterate on all entities in the vicinity.
	while ((pEntity = UTIL_FindEntityInSphere( pEntity, vecSrc, flRadius )) != NULL)
	{
		//easyForcePrintLine("RadiusDamage: Scanning... %s", pEntity->getClassname());

		if ( pEntity->pev->takedamage != DAMAGE_NO )
		{
			// UNDONE: this should check a damage mask, not an ignore
			if ( iClassIgnore != CLASS_NONE && pEntity->Classify() == iClassIgnore )
			{// houndeyes don't hurt other houndeyes with their attack
				continue;
			}

			// blast's don't tavel into or out of water
			if (bInWater && pEntity->pev->waterlevel == 0)
				continue;
			if (!bInWater && pEntity->pev->waterlevel == 3)
				continue;

			vecSpot = pEntity->BodyTarget( vecSrc );
			
			UTIL_TraceLine ( vecSrc, vecSpot, dont_ignore_monsters, ENT(pevInflictor), &tr );

			const char* inflictorClassname = 0;
			const char* pHitClassname = 0;
			const char* pEntityClassname = 0;
			
			if (pevInflictor != NULL) {
				inflictorClassname = STRING(pevInflictor->classname);
			}
			if (tr.pHit != NULL) {
				pHitClassname = STRING(tr.pHit->v.classname);;
			}
			if (pEntity != NULL) {
				pEntityClassname = pEntity->getClassname();
			}

			/*
			if(::FClassnameIs(pEntity->pev, "func_pushable")){
				const char* myClassname = pEntity->getClassname();
				Vector thisOrigin = pEntity->pev->origin;
				const char* hitClassname = "_blank_";
				const char* pevInflictorClassname = "_blank_";
				const char* pevAttackerClassname = "_blank_";

				if(tr.pHit != NULL){
					hitClassname = STRING(tr.pHit->v.classname);
				}

				if(pevInflictor != NULL){
					pevInflictorClassname = STRING(pevInflictor->classname);
				}
				if(pevAttacker != NULL){
					pevAttackerClassname = STRING(pevAttacker->classname);
				}


				int x = 666;
				::DebugLine_Setup(6, vecSrc, vecSpot, tr.flFraction);
			}
			*/


			if (EASY_CVAR_GET_DEBUGONLY(RadiusDamageDrawDebug) == 1) {
				DebugLine_Setup(vecSrc, vecSpot, tr.flFraction);
			}

			//Look uh.. if the player or whatever wants to get stupidly close to the point the spawned grenade is in the middle of clipping
			//through the player or the object as it's fired, screw it. It's already good 99% of the time now. The check I got's good enough
			//in ggrenade's Explode (trToEffectOrigin).


			if ( tr.flFraction == 1.0 || tr.pHit == pEntity->edict() )
			{// the explosion can 'see' this entity, so hurt them!
				
				
				/*
				//MODDD - no? This is what makes it look like a collision happened at the source of the explosion with the player,
				//  but the player's blood spawns at the source of the explosion instead of where the player was hit.
				//  It's ok to start solid, the rest of the logic works fine. This might be an out-of-date fix from an earlier point of development
				//  in the as-is game.
				if (tr.fStartSolid)
				{
					// if we're stuck inside them, fixup the position and distance
					tr.vecEndPos = vecSrc;
					tr.flFraction = 0.0;
				}
				*/
				
				// decrease damage for an ent that's farther from the bomb.
				flAdjustedDamage = ( vecSrc - tr.vecEndPos ).Length() * falloff;
				flAdjustedDamage = flDamage - flAdjustedDamage;
			
				if ( flAdjustedDamage < 0 )
				{
					flAdjustedDamage = 0;

					//MODDD - waitasec. If we're going to do "0" damage, why bother with any damage effects at all?  Skip acting on this entity.
					continue;
				}
				
				// ALERT( at_console, "hit %s\n", STRING( pEntity->pev->classname ) );

				// Wait...  why do we care if the entity was directly hit by this trace at this point?
				// Seems the line can go right through the entity and still have a fraction of 1.0.
				// Even though the entity isn't the ignore one (pevInflictor, the grenade/rocket/exploding
				// thing), and the end point for the trace (vecSpot) was the entity in radius's BodyTarget.
				// Just... what.
				// If nothing in the way just call it a hit all the same, close enough to the explosion
				// and unobstructed to even reach this point (blocked by any other object forbids reaching
				// this point)
				// True, but I think the point is to count damage without a trace, even though we have one that didn't hit anything for whatever reason,
				// even given the bodytarget of the entity.  The point of the hit was to deal damage, even if no point of the monster was hit in some
				// freak accident.  But it's best not to call TraceAttack with a trace that didn't hit anything.
				// Using another equivalent, "TraceAttack_Traceless" for things that expect the same cumulative damage from TraceAttack anyway.
				// Hitgroup can't factor into damage that way, but then again if damage can be dealth without a hit (missing the bodygroup point),
				// did hitgroup really matter to begin with?  BLAST damage or HITBOX_EQUAL marks damage to ignore bitbox-damages anyway.

				if (tr.flFraction != 1.0)
				{
					ClearMultiDamage( );
					// TODO - why do you put player blood at the blast center sometimes when the player & a baddie are hit at the same time???
					// Fixed now... I think.
					// And, marked not to do enhanced damage for hitting a particular sub hitbox (headshots?).  That can't be consistent.
					pEntity->TraceAttack( pevInflictor, flAdjustedDamage, (tr.vecEndPos - vecSrc).Normalize( ), &tr, bitsDamageType, bitsDamageTypeMod | DMG_HITBOX_EQUAL );
					ApplyMultiDamage( pevInflictor, pevAttacker );
				}
				else
				{
					// MODDD - !!!  A  TakeDamage call without a TraceAttack prior, eh?  That is high treason missy!
					// Call this alternate method then, sometimes cumulative damage needs to be added in an entity's own way.
					pEntity->TraceAttack_Traceless(pevInflictor, flAdjustedDamage, (tr.vecEndPos - vecSrc).Normalize(), bitsDamageType, bitsDamageTypeMod | DMG_HITBOX_EQUAL);
					pEntity->TakeDamage ( pevInflictor, pevAttacker, flAdjustedDamage, bitsDamageType, bitsDamageTypeMod | DMG_HITBOX_EQUAL);
				}
			}
		}
	}

}//RadiusDamage




//MODDD - new method to summarize tracer generating script and returning whether to block drawing a decal, based on whether a tracer was sent this time.
//        Mirrors EV_HLDM_CheckTracer from clientside cl_dlls/ev_hldm.cpp
//        This script used to be sitting in FireBullets (for monsters other than the player) to fire bullets serverside, the only way they can.
//        Now that the player can fire bullets serverside with some choices of CVar "playerWeaponTracerMode", it makes even more sense to make a method
//        out of this too.
BOOL CBaseEntity::CheckTracer(const Vector& vecSrc, const Vector& vecEnd, const Vector& vecDirForward, const Vector& vecDirRight, int iBulletType, int iTracerFreq, int* p_tracerCount){

	BOOL disableBulletHitDecal = FALSE;

	
	//if (iTracerFreq != 0 && (tracerCount++ % iTracerFreq) == 0)
	if (iTracerFreq != 0 && ((*p_tracerCount)++ % iTracerFreq) == 0)
	{
		Vector vecTracerSrc;

		if ( IsPlayer() )
		{// adjust tracer position for player
			//MODDD NOTE - funny enough this was in retail, back when this block of script (this method) was ONLY in FireBullets, and not FireBulletsPlayer (the former of which the player does not call).
			//             And it works here fine... assuming the player-only offset in sending a tracer turns out to be good?
			//    And changed to use parameters vecDirRight and vecDirForward instead in case how those are obtained ever changes elsewhere.
			//vecTracerSrc = vecSrc + Vector ( 0 , 0 , -4 ) + gpGlobals->v_right * 2 + gpGlobals->v_forward * 16;
			vecTracerSrc = vecSrc + Vector ( 0 , 0 , -4 ) + vecDirRight * 2 + vecDirForward * 16;
		}
		else
		{
			vecTracerSrc = vecSrc;
		}
			
		//MODDD NOTE - the comment below here was from the as-is SDK.  Uhhhh... what?
		//             This poorly named "tracer" variable, being 1, will block some weapons from drawing a decal.  (NOTE: it has been renamed to "disableBulletHitDecal")
		//             So it... forces weapons, that fire tracers at all but not every single time (iTracerFreq is NOT 1... and wasn't 0 to even make it this far),
		//             to not render decals on the same frames they make tracer effects.
		//             ...okaaaaaay. Why such a riddle to unravel. No shitty comments please, not that I'm perfect by any means.
		//             Why would these be mutually exclusive of one another ever?  Defaulting this behavior off (always make decals regardless of 
		//             a non-0, non-1 tracer frequency setting).
		//             Lastly, if the only type of bullet that requires this "tracer" var (now disableBulletHitDecal) to be on being BULLET_MONSTER_12MM is confusing well...
		//             I got nothing.
		//             And even zanier.  See EV_HLDM_CheckTracer of ev_hldm.cpp.  It turns below into a method of its own... kinda like serverside should do.  In fact... done.
		//             (this is that method)
		//             Anyways, it specifically stops BULLET_PLAYER_MP5 from leaving a bullet decal if a tracer was made that particular time.  So for monsters
		//             only 12MM is blocked, which certainly isn't what HGrunts even use (they use MP5s).  I just don't get it.

		if ( iTracerFreq != 1 )		// guns that always trace also always decal
			disableBulletHitDecal = TRUE;
		switch( iBulletType )
		{
		case BULLET_MONSTER_MP5:
		case BULLET_MONSTER_9MM:
		case BULLET_MONSTER_12MM:
		default:
			//MODDD - so uh, that covers anything? particularly the "default:" part?

			MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, vecTracerSrc );
				WRITE_BYTE( TE_TRACER );
				WRITE_COORD( vecTracerSrc.x );
				WRITE_COORD( vecTracerSrc.y );
				WRITE_COORD( vecTracerSrc.z );
				WRITE_COORD( vecEnd.x );  //WRITE_COORD( tr.vecEndPos.x );
				WRITE_COORD( vecEnd.y );  //WRITE_COORD( tr.vecEndPos.y );
				WRITE_COORD( vecEnd.z );  //WRITE_COORD( tr.vecEndPos.z );
			MESSAGE_END();
			break;
		}
	}

	return disableBulletHitDecal;

}//END OF CheckTracer






/*
================
FireBullets

Go to the trouble of combining multiple pellets into a single damage call.

This version is used by Monsters.
================
*/
//MODDD - borrowing blindly from gpGlobals->v_right and gpGlobals->v_up?  If every single place in the universe that calls our method
//        sets them so we have no chance of relying on garbage from old angles putting their vectors in there...
void CBaseEntity::FireBullets(ULONG cShots, Vector vecSrc, Vector vecDirShooting, Vector vecSpread, float flDistance, int iBulletType, int iTracerFreq, int iDamage, entvars_t *pevAttacker )
{
	static int tracerCount;
	BOOL disableBulletHitDecal;
	TraceResult tr;
	Vector vecRight = gpGlobals->v_right;
	Vector vecUp = gpGlobals->v_up;

	if (pevAttacker == NULL) {
		pevAttacker = pev;  // the default attacker is ourselves
	}

	ClearMultiDamage();
	gMultiDamage.type = DMG_BULLET | DMG_NEVERGIB;
	for (ULONG iShot = 1; iShot <= cShots; iShot++)
	{
		// get circular gaussian spread
		float x, y, z;
		do {
			x = RANDOM_FLOAT(-0.5,0.5) + RANDOM_FLOAT(-0.5,0.5);
			y = RANDOM_FLOAT(-0.5,0.5) + RANDOM_FLOAT(-0.5,0.5);
			z = x*x+y*y;
		} while (z > 1);

		Vector vecDir = vecDirShooting +
						x * vecSpread.x * vecRight +
						y * vecSpread.y * vecUp;
		Vector vecEnd;

		vecEnd = vecSrc + vecDir * flDistance;
		UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(pev)/*pentIgnore*/, &tr);


		//MODDD - tracers added for the player, serverside, depending on playerWeaponTracerMode choice.
		switch( (int)EASY_CVAR_GET_DEBUGONLY(monsterWeaponTracerMode)  ){
		case 0:
			//nothing.
			iTracerFreq = 0;
		break;
		case 1:
			//whatever it was as provided this time (retail)
			
		break;
		case 2:
			//always
			iTracerFreq = 1;
		break;
		}//END OF switch

		//MODDD - squeaky clean, moved to a method.  Like how cl_dlls/ev_hldm.cpp did it.
		disableBulletHitDecal = CheckTracer(vecSrc, tr.vecEndPos, gpGlobals->v_forward, gpGlobals->v_right, iBulletType, iTracerFreq, &tracerCount);



		// do damage, paint decals
		if (tr.flFraction != 1.0)
		{
			CBaseEntity *pEntity = CBaseEntity::Instance(tr.pHit);
			if(pEntity != NULL){
				BOOL useBulletHitSound = TRUE;
				BOOL doDefaultBulletHitEffectCheck = FALSE;
				// NOTICE - iDamage is specified for a custom amount of damage.  Otherwise rely on iBulletType to fill that in for us.
				// This will also force gibbing on over 16 damage (which a lot of damage sources that happen to be over 16 don't do).
				if ( iDamage )
				{
					pEntity->TraceAttack(pevAttacker, iDamage, vecDir, &tr, DMG_BULLET | ((iDamage > 16) ? DMG_ALWAYSGIB : DMG_NEVERGIB), 0, TRUE, &useBulletHitSound);
					
					//TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
					//DecalGunshot( &tr, iBulletType );
					doDefaultBulletHitEffectCheck = TRUE;
				} 
				//MODDD - changing how this works. Instead of always playing the textureHit sound, we will let the hit entity determine whether it wants to play
				//        the texture sound / generate a hit decal. For instance, shots that hit helmets and ricochet shouldn't play flesh hit sounds at the same
				//        time like the texture system wants to. That means, bullets merely ALLOW the bullethit sound to be played, but it doesn't have to be.
				//        Acutally, little issue with this approach. TEXTURETYPE_PlaySound needs a fair amount of information (trace result, evSrc, vecEnd, and
				//        our iBulletType). We don't need another TraceAttack overload just for this
				//        So, little compromise. Instead, traceAttack gets to set the "useBulletHitsound" variable which this method will send by reference.
				//        If the hit entity allows the bullet hit sound to play, then we can call TEXTURETYPE_PlaySound and DecalGunshot from here as usual.
				//        This also means, no need for checking to see if the bitsDamage bitmask includes DMG_BULLET. "useBulletHitSound" being provided at all
				//        and not null alone tells us we're seeing if it is ok to play the sound (typically bullets checking).
				else switch(iBulletType)
				{
				default:
				case BULLET_MONSTER_9MM:
					pEntity->TraceAttack(pevAttacker, gSkillData.monDmg9MM, vecDir, &tr, DMG_BULLET, 0, TRUE, &useBulletHitSound);
					//TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
					//DecalGunshot( &tr, iBulletType );
					doDefaultBulletHitEffectCheck = TRUE;
				break;
				case BULLET_MONSTER_MP5:
					pEntity->TraceAttack(pevAttacker, gSkillData.monDmgMP5, vecDir, &tr, DMG_BULLET, 0, TRUE, &useBulletHitSound);
					//TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
					//DecalGunshot( &tr, iBulletType );
					doDefaultBulletHitEffectCheck = TRUE;
				break;
				case BULLET_MONSTER_12MM:		
					pEntity->TraceAttack(pevAttacker, gSkillData.monDmg12MM, vecDir, &tr, DMG_BULLET, 0, TRUE, &useBulletHitSound);

					if ( EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(decalTracerExclusivity) != 1 || !disableBulletHitDecal )
					{
						//TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
						//DecalGunshot( &tr, iBulletType );
						doDefaultBulletHitEffectCheck = TRUE;  //so tracers don't even get a normal shot at this? Isn't that a bit weird? Maybe CVar this.
					}
				break;
				case BULLET_NONE: // FIX 
					pEntity->TraceAttack(pevAttacker, 50, vecDir, &tr, DMG_CLUB, 0, TRUE, &useBulletHitSound);

					// BULLET_NONE? When would this happen? And different logic for doing decals? why? Not even DMG_BULLET above?
					// Just leaving this logic as it is.

					if(useBulletHitSound == TRUE){
						TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
						// only decal glass
						if ( !FNullEnt(tr.pHit) && VARS(tr.pHit)->rendermode != 0)
						{
							UTIL_DecalTrace( &tr, DECAL_GLASSBREAK1 + RANDOM_LONG(0,2) );
						}
					}
				break;
				}

				if(doDefaultBulletHitEffectCheck && useBulletHitSound){
					TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
					DecalGunshot( &tr, iBulletType );
				}


				//MODDD
				if (pEntity && pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MACHINE )
				{
					//easyPrintLine("WHAT IS THE THING I HIT %s", STRING(pEntity->pev->classname) );

				}else{

					//if ( FNullEnt(tr.pHit))
					{

						if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(muteRicochetSound) < 1){
							MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, tr.vecEndPos );
								WRITE_BYTE( TE_GUNSHOT );
								WRITE_COORD( tr.vecEndPos.x );
								WRITE_COORD( tr.vecEndPos.y );
								WRITE_COORD( tr.vecEndPos.z );
							MESSAGE_END();
						}
					}
				}
			}//END OF pEntity NULL check
		}
		// make bullet trails
		UTIL_BubbleTrail( vecSrc, tr.vecEndPos, (flDistance * tr.flFraction) / 64.0 );
	}
	ApplyMultiDamage(pev, pevAttacker);
}




/*
================
FireBulletsPlayer

Go to the trouble of combining multiple pellets into a single damage call.

This version is used by Players, uses the random seed generator to sync client and server side shots.

//MODDD - extra note. Notice that the BULLET_PLAYER... cases below don't do the TEXTURETYPE_PLAYSOUND and DecalGunshot calls. These are both handled by the client instead
//        (see the EV_HLDM_PlayTextureSound method of cl_dlls/ev_hldm.cpp).
//        But this can only work for hits on map geometry. Hits on entities, even func_breakable, do not give their texture types for sounds (glass ,etc.).
//        Playing from the server seems to be enough for them (Notice the breakable plays a sound of their own in TakeDamage).
//        Looks like the main idea is, map geometry needs something else to play sounds for them (which is why monsters firing bullets, through the plain FireBullets above, 
//        have to call the aforementionde TEXTURETYPE_PLAYSOUND and DecalGunshot methods: to guarantee a sound plays. Even though this is redundant with the sound hit entities
//        such as, say, func_breakable already make on hit. That may or may not be terrible, but could be correctable. Check if the hit thing is the map, and, if so, play
//        a hit sound because we know the map won't on its own and the client can't (that is only for player-bullet hits). So the monster must in that case. Otherwise trust
//        the hit entity... not the map... handles playing sounds for taking hits correctly.
//        Looks like most things do expect the attacker/shooter to handle the sound part at least. Func_breakables are more of an exception to play a sound no matter
//        where they are hit. It's fine to make the player play a texturehitsound when not hitting the map (worldspawn) still.

================
*/
Vector CBaseEntity::FireBulletsPlayer ( ULONG cShots, Vector vecSrc, Vector vecDirShooting, Vector vecSpread, float flDistance, int iBulletType, int iTracerFreq, int iDamage, entvars_t *pevAttacker, int shared_rand )
{
	static int tracerCount;
	BOOL disableBulletHitDecal;
	TraceResult tr;
	Vector vecRight = gpGlobals->v_right;
	Vector vecUp = gpGlobals->v_up;
	float x, y, z;

	if ( pevAttacker == NULL ){
		pevAttacker = pev;  // the default attacker is ourselves
	}

	ClearMultiDamage();
	gMultiDamage.type = DMG_BULLET | DMG_NEVERGIB;

	for ( ULONG iShot = 1; iShot <= cShots; iShot++ )
	{
		// Use player's random seed.
		// get circular gaussian spread
		x = UTIL_SharedRandomFloat( shared_rand + iShot, -0.5, 0.5 ) + UTIL_SharedRandomFloat( shared_rand + ( 1 + iShot ) , -0.5, 0.5 );
		y = UTIL_SharedRandomFloat( shared_rand + ( 2 + iShot ), -0.5, 0.5 ) + UTIL_SharedRandomFloat( shared_rand + ( 3 + iShot ), -0.5, 0.5 );
		z = x * x + y * y;

		Vector vecDir = vecDirShooting +
						x * vecSpread.x * vecRight +
						y * vecSpread.y * vecUp;
		Vector vecEnd;

		vecEnd = vecSrc + vecDir * flDistance;
		UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(pev)/*pentIgnore*/, &tr);
		

		//MODDD - tracers added for the player, serverside, depending on playerWeaponTracerMode choice.
		switch( (int)EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(playerWeaponTracerMode)  ){
		case 0:
			// nothing.
			iTracerFreq = 0;
		break;
		case 1:
			// clientside only (retail).  So nothing here.
			iTracerFreq = 0;
		break;
		case 2:
			// serverside, per whatever weapons said to do for tracers.  Leave "iTracerFreq" as it was sent.

		break;
		case 3:
			// clientside and serverside as-is.  Let it proceed.

		break;
		case 4:
			// clientside, all weapons, all shots. Not here.
			iTracerFreq = 0;
		break;
		case 5:
			// serverside, all weapons, all shots. Force it.
			iTracerFreq = 1;
		break;
		case 6:
			// clientside and serverside, all weapons, all shots. Go.
			iTracerFreq = 1;
		break;
		default:
			// unrecognized setting?  Default to nothing like retail did.
			iTracerFreq = 0;
		break;
		}//END OF switch

		//MODDD - the idea was always here for FireBullets (monsters) above, but for the player, this is new.  Player's "disableBulletHitDecal" goes unused though.
		disableBulletHitDecal = CheckTracer(vecSrc, tr.vecEndPos, gpGlobals->v_forward, gpGlobals->v_right, iBulletType, iTracerFreq, &tracerCount);

		/*
		//easyPrintLine("IS IT NULL????? %d", (CBaseEntity::Instance(tr.pHit)  == NULL) );
		if(CBaseEntity::Instance(tr.pHit)  != NULL){
			CBaseEntity *pEntity = CBaseEntity::Instance(tr.pHit);
			//easyPrintLine("NAME:::%s", STRING(pEntity->pev->classname) );
		}
		*/

		//easyPrintLine("flFraction?", tr.flFraction); 
		// do damage, paint decals
		if (tr.flFraction != 1.0)
		{
			CBaseEntity *pEntity = CBaseEntity::Instance(tr.pHit);
			if(pEntity != NULL){
				BOOL useBulletHitSound = TRUE; //by default.
				// set to TRUE if a case relies on a common default for this.
				// Leave FALSE if the case handles this itself.
				BOOL doDefaultBulletHitEffectCheck = FALSE;

				//tr.pHit->v.origin
				
				// This is an "AI Sound", or not a real one audible to the player, but one that checks for monsters nearby (distance) and alerts them if they are in hearing range.
				attemptSendBulletSound(tr.vecEndPos, pevAttacker);


				//easyPrintLine("FireBulletsPlayer: iDamage: %d PLAYER BULLET TYPE?! %d THING HIT: %s", iDamage, iBulletType, pEntity->getClassname());

				// NOTICE - iDamage is specified for a custom amount of damage.  Otherwise rely on iBulletType to fill that in for us.
				// This will also force gibbing on over 16 damage (which a lot of damage sources that happen to be over 16 don't do).
				if ( iDamage )
				{
					//MODDD NOTE
					// Why does this area, completely unused (the player never uses "iDamage" in FirePlayerBullets, relies in iBulletType to get a default damage value and pick from a below case),
					// have the TEXTURETYPE_PlaySound and DecalGunshot calls that the NPC's FireBullets method has? The world may never know.
					pEntity->TraceAttack(pevAttacker, iDamage, vecDir, &tr, DMG_BULLET | ((iDamage > 16) ? DMG_ALWAYSGIB : DMG_NEVERGIB), 0, TRUE, &useBulletHitSound);
					//TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
					//DecalGunshot( &tr, iBulletType );
					
					doDefaultBulletHitEffectCheck = TRUE;
				} 
				else switch(iBulletType)
				{
				default:
				case BULLET_PLAYER_9MM:	
					pEntity->TraceAttack(pevAttacker, gSkillData.plrDmg9MM, vecDir, &tr, DMG_BULLET, 0, TRUE, &useBulletHitSound);
					//If what we hit was an entity, we need to play the sound from the server. Clientside's texture sound player won't catch this.
					doDefaultBulletHitEffectCheck = TRUE;
				break;
				case BULLET_PLAYER_MP5:	
					pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgMP5, vecDir, &tr, DMG_BULLET, 0, TRUE, &useBulletHitSound);
					doDefaultBulletHitEffectCheck = TRUE;
				break;
				case BULLET_PLAYER_BUCKSHOT:	
					 // make distance based!
					pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgBuckshot, vecDir, &tr, DMG_BULLET, 0, TRUE, &useBulletHitSound);
					doDefaultBulletHitEffectCheck = TRUE;
				break;
				case BULLET_PLAYER_357:		
					pEntity->TraceAttack(pevAttacker, gSkillData.plrDmg357, vecDir, &tr, DMG_BULLET, 0, TRUE, &useBulletHitSound);
					doDefaultBulletHitEffectCheck = TRUE;
				break;
				case BULLET_NONE: // FIX

					pEntity->TraceAttack(pevAttacker, 50, vecDir, &tr, DMG_CLUB, 0, TRUE, &useBulletHitSound);
					
					//if( !FClassnameIs(pEntity->pev, "worldspawn") && useBulletHitSound){
					if(useBulletHitSound == TRUE){
						TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
						// only decal glass
						if ( !FNullEnt(tr.pHit) && VARS(tr.pHit)->rendermode != 0)
						{
							UTIL_DecalTrace( &tr, DECAL_GLASSBREAK1 + RANDOM_LONG(0,2) );
						}
					}

					break;
				}//END OF switch


				// !!! I think these have been taken care of since!
				//MODDD TODO - remove the "worldspawn" check and have worldspawn itself (CWorld, world.cpp) override TraceAttack to disallow if it is the player making the
				//             request? That sounds neat.
				//MODDD TODO - switch! Same for the above worldspawn check too.
				// Also, if the "playerBulletHitEffectForceServer" CVar is set to 1, the client won't make hitsound / decal effects in ev_hldm.cpp. Instead, it will happen here serverside
				// to be broadcast to all clients like all other effects (by NPCs, etc.).
				
				//if(doDefaultBulletHitEffectCheck && (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(playerBulletHitEffectForceServer)==1 || !FClassnameIs(pEntity->pev, "worldspawn")) && useBulletHitSound){
				//if(doDefaultBulletHitEffectCheck && !FClassnameIs(pEntity->pev, "worldspawn") && useBulletHitSound){
				///////////////////////////////////////////////////////////////////////////////////////

				// The "useBulletHitSound" can be turned off by a TraceAttack method, presumably because it decided to handle the effect itself
				// and handling it here would be redundant.
				// This is common for things that do a ricochet effect on detecting a hit on armor or a helmet (hgrunts, agrunts).  They turn it off.
				// Note that, unless TEXTURETYPE_PlaySound detects a machine or the world (CLASS_NONE.. however crude that is) was hit,
				// it's going to force a flesh sound.  Just keeping that in tune with retail to play nicely.
				if(doDefaultBulletHitEffectCheck && useBulletHitSound){
					TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
					DecalGunshot( &tr, iBulletType );
				}

				/*
				if(iDamage == 0 &&
					iBulletType != BULLET_NONE &&
					( !FNullEnt(tr.pHit)) &&
					!FClassnameIs(pEntity->pev, "worldspawn"))
				{
					//Play a sound since the client won't.
					TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
					if(VARS(tr.pHit)->rendermode != 0)){
						DecalGunshot( &tr, iBulletType );
					}
				}
				*/


				//MODDD
				if (pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MACHINE )
				{
					//easyPrintLine("WHAT IS THE THING I HIT %s", STRING(pEntity->pev->classname) );

				}else{
					
					//if ( FNullEnt(tr.pHit))
					{
						if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(muteRicochetSound) < 1){
							MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, tr.vecEndPos );
								WRITE_BYTE( TE_GUNSHOT );
								WRITE_COORD( tr.vecEndPos.x );
								WRITE_COORD( tr.vecEndPos.y );
								WRITE_COORD( tr.vecEndPos.z );
							MESSAGE_END();
						}
					}
				}
			}//END OF pEntity NULL check
			
		}//END OF if (tr.flFraction != 1.0)

		//easyPrintLine("NULL?? %d", FNullEnt(tr.pHit) );
		//COME BACK
			

		// make bullet trails
		UTIL_BubbleTrail( vecSrc, tr.vecEndPos, (flDistance * tr.flFraction) / 64.0 );
	}
	ApplyMultiDamage(pev, pevAttacker);

	return Vector( x * vecSpread.x, y * vecSpread.y, 0.0 );
}



// NOTICE!  Draws the decal only, doesn't involve blood particles.  Often called alongside
// 'SpawnBlood' calls to do that
void CBaseEntity::TraceBleed( float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType )
{
	TraceBleed(flDamage, vecDir, ptr, bitsDamageType, 0);
}
void CBaseEntity::TraceBleed( float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType, int bitsDamageTypeMod )
{
	if (!UTIL_ShouldShowBlood(BloodColor())) {
		return;
	}
	
	if (flDamage == 0)
		return;

	// can involve "bitsDamageTypeMod" if necessary too.
	// Seems safe to imply any bleeding damage (DMG_BLEEDING for the bitsDamageTypeMod new bitmask) will come with DMG_SLASH so that blood is dealt.
	// Eh, letting bleeding damage allow the effect. why not.
	if (
		!(bitsDamageType & (DMG_CRUSH | DMG_BULLET | DMG_SLASH | DMG_BLAST | DMG_CLUB | DMG_MORTAR)) &&
		!( bitsDamageTypeMod & (DMG_BLEEDING) )
		)
		return;
	
	// make blood decal on the wall! 
	TraceResult Bloodtr;
	Vector vecTraceDir; 
	float flNoise;
	int cCount;
	int i;

/*
	if ( !IsAlive() )
	{
		// dealing with a dead monster. 
		if ( pev->max_health <= 0 )
		{
			// no blood decal for a monster that has already decalled its limit.
			return; 
		}
		else
		{
			pev->max_health--;
		}
	}
*/

	if (flDamage < 10)
	{
		flNoise = 0.1;
		cCount = 1;
	}
	else if (flDamage < 25)
	{
		flNoise = 0.2;
		cCount = 2;
	}
	else
	{
		flNoise = 0.3;
		cCount = 4;
	}

	for ( i = 0 ; i < cCount ; i++ )
	{
		vecTraceDir = vecDir * -1;// trace in the opposite direction the shot came from (the direction the shot is going)

		vecTraceDir.x += RANDOM_FLOAT( -flNoise, flNoise );
		vecTraceDir.y += RANDOM_FLOAT( -flNoise, flNoise );
		vecTraceDir.z += RANDOM_FLOAT( -flNoise, flNoise );

		//MODDD - TODO: vecTraceDir * -172?   Doesn't that make the "* -1" above redundant (the minus in -172)?  Why this double negative?
		UTIL_TraceLine( ptr->vecEndPos, ptr->vecEndPos + vecTraceDir * -172, ignore_monsters, ENT(pev), &Bloodtr);

		if ( Bloodtr.flFraction != 1.0 )
		{
			UTIL_BloodDecalTrace( &Bloodtr, BloodColor() );
		}
	}
}

//=========================================================
//=========================================================
void CBaseMonster::MakeDamageBloodDecal ( int cCount, float flNoise, TraceResult *ptr, const Vector &vecDir )
{
	if (!UTIL_ShouldShowBlood(BloodColor())) {
		return;
	}

	// make blood decal on the wall! 
	TraceResult Bloodtr;
	Vector vecTraceDir; 
	int i;

	if ( !IsAlive() )
	{
		// dealing with a dead monster. 
		if ( pev->max_health <= 0 )
		{
			// no blood decal for a monster that has already decalled its limit.
			return; 
		}
		else
		{
			pev->max_health--;
		}
	}

	for ( i = 0 ; i < cCount ; i++ )
	{
		vecTraceDir = vecDir;

		vecTraceDir.x += RANDOM_FLOAT( -flNoise, flNoise );
		vecTraceDir.y += RANDOM_FLOAT( -flNoise, flNoise );
		vecTraceDir.z += RANDOM_FLOAT( -flNoise, flNoise );

		UTIL_TraceLine( ptr->vecEndPos, ptr->vecEndPos + vecTraceDir * 172, ignore_monsters, ENT(pev), &Bloodtr);

/*
		MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
			WRITE_BYTE( TE_SHOWLINE);
			WRITE_COORD( ptr->vecEndPos.x );
			WRITE_COORD( ptr->vecEndPos.y );
			WRITE_COORD( ptr->vecEndPos.z );
			
			WRITE_COORD( Bloodtr.vecEndPos.x );
			WRITE_COORD( Bloodtr.vecEndPos.y );
			WRITE_COORD( Bloodtr.vecEndPos.z );
		MESSAGE_END();
*/

		if ( Bloodtr.flFraction != 1.0 )
		{
			UTIL_BloodDecalTrace( &Bloodtr, BloodColor() );
		}
	}
}
