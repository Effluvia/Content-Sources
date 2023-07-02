
//NOTICE - this file is not included by the client. Assume serverside.


#include "extdll.h"
#include "crossbowbolt.h"
#include "util.h"
#include "cbase.h"
#include "basemonster.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "gamerules.h"
#include "soundent.h"
#include "func_break.h"

EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(playerCrossbowMode)
EASY_CVAR_EXTERN_DEBUGONLY(hassassinCrossbowMode)
EASY_CVAR_EXTERN_DEBUGONLY(crossbowInheritsPlayerVelocity)
EASY_CVAR_EXTERN_DEBUGONLY(crossbowBoltDirectionAffectedByWater)
EASY_CVAR_EXTERN_DEBUGONLY(sparksPlayerCrossbowMulti)
//EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(crossbowReloadSoundDelay)



LINK_ENTITY_TO_CLASS( crossbow_bolt, CCrossbowBolt );


CCrossbowBolt *CCrossbowBolt::BoltCreate(const Vector& arg_velocity, float arg_speed ){
	return BoltCreate(arg_velocity, arg_speed, TRUE, FALSE);
}
CCrossbowBolt *CCrossbowBolt::BoltCreate(const Vector& arg_velocity, float arg_speed, BOOL useTracer, BOOL arg_noDamage ){
	// Create a new entity with CCrossbowBolt private data
	CCrossbowBolt *pBolt = GetClassPtr( (CCrossbowBolt *)NULL );
	pBolt->pev->classname = MAKE_STRING("bolt");


	pBolt->m_velocity = arg_velocity;
	pBolt->m_speed = arg_speed;

	pBolt->pev->velocity = arg_velocity;
	pBolt->pev->speed = arg_speed;

	pBolt->Spawn(useTracer, arg_noDamage);


	return pBolt;
}





CCrossbowBolt::CCrossbowBolt(void){
	recentVelocity = Vector(0,0,0);
	noDamage = FALSE;
	realNextThink = 0;
}//END OF CCrossbowBolt constructor



TYPEDESCRIPTION	CCrossbowBolt::m_SaveData[] = 
{
	DEFINE_FIELD( CCrossbowBolt, recentVelocity, FIELD_VECTOR ),
	DEFINE_FIELD( CCrossbowBolt, m_velocity, FIELD_VECTOR ),
	DEFINE_FIELD( CCrossbowBolt, m_speed, FIELD_FLOAT ),
	DEFINE_FIELD( CCrossbowBolt, noDamage, FIELD_BOOLEAN ),
	DEFINE_FIELD( CCrossbowBolt, hitSomething, FIELD_BOOLEAN ),
	
};
// Commented out, need custom save/restore's below.  Maybe?
//IMPLEMENT_SAVERESTORE( CCrossbowBolt, CBaseEntity );


//Typically want to do things after the iWriteFieldsResult or iReadFieldsResult. For writing it may not matter,
//but reading may require parent class vars to already be loaded if they are depended on.
//Also beware of depending on this same class's vars during loading, depending on vars not yet loaded is bad.
int CCrossbowBolt::Save( CSave &save )
{
	if ( !CBaseEntity::Save(save) )
		return 0;
	int iWriteFieldsResult = save.WriteFields( "CCrossbowBolt", this, m_SaveData, ARRAYSIZE(m_SaveData) );

	return iWriteFieldsResult;
}
int CCrossbowBolt::Restore( CRestore &restore )
{
	if ( !CBaseEntity::Restore(restore) )
		return 0;
	int iReadFieldsResult = restore.ReadFields( "CCrossbowBolt", this, m_SaveData, ARRAYSIZE(m_SaveData) );

	return iReadFieldsResult;
}



void CCrossbowBolt::Spawn(){
	Spawn(TRUE, FALSE);
}

void CCrossbowBolt::Spawn(BOOL useTracer, BOOL arg_noDamage)
{
	hitSomething = FALSE;

	noDamage = arg_noDamage;

	Precache( );
	pev->movetype = MOVETYPE_FLY;

	pev->solid = SOLID_BBOX;
	// wow, SOLID_TRIGGER is an awful idea.  Really inconsistent hit-detection, shame.
	//pev->solid = SOLID_TRIGGER;

	//MODDD - making these "Parabolic bolts" is easy!  Just change pev->movetype to MOVETYPE_TOSS !

	pev->gravity = 0.5;

	SET_MODEL(ENT(pev), "models/crossbow_bolt.mdl");
	

	//MODDD - this section has been added.
	///////////////////////////////////////////////////////
	if(useTracer){

		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);

		WRITE_BYTE(TE_BEAMFOLLOW);
		WRITE_SHORT(entindex());	// entity
		WRITE_SHORT(m_iTrail);	// model
		WRITE_BYTE(10); // life
		WRITE_BYTE(1.5);  // width
		WRITE_BYTE(224);   // r, g, b
		WRITE_BYTE(224);   // r, g, b
		WRITE_BYTE(255);   // r, g, b
		WRITE_BYTE(100);	// brightness
	
		MESSAGE_END();
	}//END OF useTracer check
	///////////////////////////////////////////////////////


	UTIL_SetOrigin( pev, pev->origin );
	UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));

	SetTouch( &CCrossbowBolt::BoltTouch );
	SetThink( &CCrossbowBolt::BoltThink );
	

	//MODDD NOTE - why is this " + 0.2" instead of " + 0.1" like most think methods and even BoltThink's own think refresh?
	//             The world may never know.
	//pev->nextthink = gpGlobals->time + 0.2;
	realNextThink = gpGlobals->time + 0.2f;
	pev->nextthink = gpGlobals->time;
	
}


void CCrossbowBolt::Precache( )
{
	PRECACHE_MODEL ("models/crossbow_bolt.mdl");
	PRECACHE_SOUND("weapons/xbow_hitbod1.wav");
	PRECACHE_SOUND("weapons/xbow_hitbod2.wav");
	
	//MODDD - do we even ever play this? Not even in clientside events (cl_dlls/ev_hldm.cpp)? 
	//        That's ok, this sounds terrible anyways. At least at the same default volume as everything else which is blisteringly loud in any sound player for some reason.
	PRECACHE_SOUND("weapons/xbow_fly1.wav");

	PRECACHE_SOUND("weapons/xbow_hit1.wav");
	PRECACHE_SOUND("fvox/beep.wav");

	//MODDD - used to be this:
	//m_iTrail = PRECACHE_MODEL("sprites/streak.spr");
	m_iTrail = PRECACHE_MODEL("sprites/smoke.spr");
}


int CCrossbowBolt::Classify ( void )
{
	return	CLASS_NONE;
}

void CCrossbowBolt::BoltTouch( CBaseEntity *pOther )
{
	BOOL goingToExplode = FALSE;
	int iBitsDamage = 0;

	//MODDD - is that safe?
	recentVelocity = pev->velocity;

	// why yes, I believe I did.
	hitSomething = TRUE;


	float crossbowMode = -1;

	float damageToDeal = 0;

	damageToDeal = gSkillData.plrDmgCrossbowClient;
	damageToDeal = gSkillData.plrDmgCrossbowMonster;

	//if( pev->owner != NULL){
	const char* ownerClassName = STRING(pev->owner->v.classname);
	if( !strcmp(ownerClassName, "player")){
		crossbowMode = EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(playerCrossbowMode);
		if (pOther->IsPlayer()) {
			// Player attacking a player.
			damageToDeal = gSkillData.plrDmgCrossbowClient;
		}
		else {
			// Player attacking anything else.
			damageToDeal = gSkillData.plrDmgCrossbowMonster;
		}
			
	}else if(!strcmp(ownerClassName, "monster_human_assassin")){
		crossbowMode = EASY_CVAR_GET_DEBUGONLY(hassassinCrossbowMode);
		if (pOther->IsPlayer()) {
			// hassassin attacking a player.
			damageToDeal = gSkillData.hassassinDmgCrossbowClient;
		}
		else {
			// hassassin attacking anything else.
			damageToDeal = gSkillData.hassassinDmgCrossbowMonster;
		}

	}
	//}
	
	//MODDD - filter w/ new CVar.

	// mode 2 forces multiplayer, mode 1 forces single player, 0 (or anything else) leaves it
	// up to the game.

	// if the mode is 2, or the mode is 0 AND multiplayer is on, we will explode.
	if (crossbowMode == 2 || (crossbowMode == 0 && IsMultiplayer())) {
		//plain bolts explode in multiplayer mode.
		goingToExplode = TRUE;
	}
	else{
		// singleplayer mode? nothing special.
		// Things with a crossbow not tracked by either CVar also won't explode this way ever.
		// Because exploding crossbows out of nowhere in the future would be unpleasant.
	}
	

	
	SetTouch( NULL );
	SetThink( NULL );

	entvars_t* pevOwner;
	pevOwner = VARS(pev->owner);

	TraceResult tr = UTIL_GetGlobalTrace();

	if (pOther->pev->takedamage)
	{
		// Force pev->solid to SOLID_NOT.  Maybe that will stop a bolt from screwing with the physics of
			// tossed corpses.
		pev->solid = SOLID_NOT;

		if (!noDamage) {

			// UNDONE: this needs to call TraceAttack instead
			// ...what was that comment supposed to mean.  That does happen too.
			ClearMultiDamage();

			if (goingToExplode) {
				// explosions gib. Look like it.
				// ...eh, nevermind.  These are small explosions. Gib just if we deserve it as usual
				// (overkill factor).  If you change your mind, add back DMG_ALWAYSGIB.
				// Also, no poison for explosive arrows. C'mon, that's just ridiculous.
				// And this is the direct hit, not the explosion.  Don't count just the impact
				// as BLAST damage, whoops.  I don't think BULLET hurts though.
				//gMultiDamage.type = DMG_BLAST | DMG_ALWAYSGIB;
				gMultiDamage.type = DMG_BULLET | DMG_NEVERGIB;
				gMultiDamage.typeMod = DMG_PROJECTILE;
			}
			else {
				// May do lots of damage, but no gibbing. Does not make sense for ordinary
				// piercing arrows.  Also does poison.
				// MODDD TODO - make crossbow poison weaker than most?  It's already pretty devastating
				// even without the poison.  Would need a new damage type (for the 2nd dmg bitmask
				// probably).  BLEGH.
				gMultiDamage.type = DMG_BULLET | DMG_NEVERGIB; //| DMG_POISON;
				gMultiDamage.typeMod = DMG_PROJECTILE | DMG_POISONHALF;
			}


			pOther->TraceAttack(pevOwner, damageToDeal, pev->velocity.Normalize(), &tr, gMultiDamage.type, gMultiDamage.typeMod);


			//ClearMultiDamage();
			//gMultiDamage.type = DMG_BULLET | DMG_NEVERGIB;

			//easyForcePrintLine("OW! 3 %s health:%.2f", pOther->getClassname(), pOther->pev->health);
			//easyForcePrintLine("!!!CROSSBOWBOLT: ApplyMultiDamage CALL!!!");
			ApplyMultiDamage(pev, pevOwner);

		}//END OF noDamage


		//easyForcePrintLine("OW! 4 %s health:%.2f", pOther->getClassname(), pOther->pev->health);

		BOOL extraPasss = FALSE;
		if (FClassnameIs(pOther->pev, "func_breakable")) {
			CBreakable *tempBreak = static_cast<CBreakable*>(pOther);
			if (tempBreak->m_Material == matWood || tempBreak->m_Material == matFlesh || tempBreak->m_Material == matLastMaterial) {
				extraPasss = TRUE;
			}
		}


		pev->velocity = Vector(0, 0, 0);
		//MODDD - if it is organic.
		if( (pOther->Classify() != CLASS_NONE && pOther->isOrganic() == TRUE) || extraPasss){
			// May seem like a place for hitEffect, but that has sparks, don't want sparks for this.

			// play body "thwack" sound
			switch (RANDOM_LONG(0, 1))
			{
			case 0:
				UTIL_PlaySound(ENT(pev), CHAN_BODY, "weapons/xbow_hitbod1.wav", 1, ATTN_NORM, 0, 100, FALSE); break;
			case 1:
				UTIL_PlaySound(ENT(pev), CHAN_BODY, "weapons/xbow_hitbod2.wav", 1, ATTN_NORM, 0, 100, FALSE); break;
			}
		}
		else {
			// not organic?  Slightly modified
			EMIT_SOUND_DYN(ENT(pev), CHAN_BODY, "weapons/xbow_hit1.wav", RANDOM_FLOAT(0.98, 1.0), ATTN_NORM - 0.1, 0, 107 + RANDOM_LONG(0, 4));
			hitEffect(tr);
		}//END OF pOther organic check

		
		// Does this crossbowbolt need to  be removed this instant?
		// If it collided with an entity (must have to reach this point) and isn't going to explode (deleted soon later),
		// go ahead.
		if(!goingToExplode){
			// wait, the Killed method for base entities does so little, meangingless to set flags for something about to be
			// deleted, so why not just skip it and do UTIL_REMOVE?  If exploding that happens anyway.
			
			//Killed( pev, pev, GIB_NEVER );
			UTIL_Remove(this);
		}else{

		}
	}
	else
	{
		// Didn't hit a typical ent?  Probably going to stick out of a surface and stay for a little while.

		SetThink( &CBaseEntity::SUB_Remove );
		// this will get changed below if the bolt is allowed to stick in what it hit.
		// (as in, not be instant to look like it sticks to it for some seconds).
		pev->nextthink = gpGlobals->time;
		
		if (UTIL_PointContents(pev->origin) == CONTENTS_SKY) {
			// If we hit the sky, HALT!  No explosion, no sticking out, no sparks.
			// Just end.
			return;
		}


		//MODDD - can stick to other map-related things so long as they don't move.
		//if ( FClassnameIs( pOther->pev, "worldspawn" ) )
		//if(pOther->IsWorldOrAffiliated() && pOther->pev->movetype == MOVETYPE_NONE)
		//...why do some static map things use MOVETYPE_PUSH? foget this.
		if(pOther->IsWorld())
		{
			// if what we hit is static architecture, can stay around for a while.
			Vector vecDir = pev->velocity.Normalize( );
			UTIL_SetOrigin( pev, pev->origin - vecDir * 12 );
			pev->angles = UTIL_VecToAngles( vecDir );
			pev->solid = SOLID_NOT;
			pev->movetype = MOVETYPE_FLY;
			pev->velocity = Vector( 0, 0, 0 );
			pev->avelocity.z = 0;
			// why wasn't this RANDOM_FLOAT?  coords are decimals
			pev->angles.z = RANDOM_FLOAT(0,360);
			pev->nextthink = gpGlobals->time + 10.0;
		}

		EMIT_SOUND_DYN(ENT(pev), CHAN_BODY, "weapons/xbow_hit1.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 98 + RANDOM_LONG(0, 7));
		hitEffect(tr);
	}

	attemptSendBulletSound(pev->origin, pevOwner);

	// It's your time to shine!  Explode and remove self.
	if(goingToExplode){
		pev->solid = SOLID_NOT;  //safety?
		SetThink( &CCrossbowBolt::ExplodeThink );
		pev->nextthink = gpGlobals->time + 0.1;
	}
	
	//easyForcePrintLine("OW! 5 %s health:%.2f", pOther->getClassname(), pOther->pev->health);
}


// Makes bubbles if it is underwater.
void CCrossbowBolt::BoltThink( void )
{
	//think every single frame of game logic, but only run the retail logic every 0.1 seconds for the same behavior there.
	pev->nextthink = gpGlobals->time;
		
	if(EASY_CVAR_GET_DEBUGONLY(crossbowBoltDirectionAffectedByWater) != 1){
		//forcing velocity to the one that fired me's intention every frame can preserve direction underwater.
		pev->velocity = m_velocity;
	}

	if(gpGlobals->time >= realNextThink ){
		//one typical think cycle.
		realNextThink = gpGlobals->time + 0.1;	

		if (pev->waterlevel == 0)
			return;

		UTIL_BubbleTrail( pev->origin - pev->velocity * 0.1, pev->origin, 8);

	}//END OF realNextThink check

}//END OF BoltThink


void CCrossbowBolt::ExplodeThink( void )
{
	int iContents = UTIL_PointContents ( pev->origin );
	int iScale;
	
	//NEW?
	int shrapMod = 1;  //safe?
	Vector vecSpot;

	pev->dmg = 40;
	iScale = 10;

	short spriteChosen;
	if (iContents != CONTENTS_WATER)
	{
		spriteChosen = g_sModelIndexFireball;
	}
	else
	{
		spriteChosen = g_sModelIndexWExplosion;
	}

	//MODDD - mimicking how ggrenade.cpp's Explode method, called by some touch method (ExplodeTouch) does it with its trace it sends.
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	TraceResult pTrace;
	vecSpot = pev->origin - recentVelocity.Normalize() * 32;
	UTIL_TraceLine( vecSpot, vecSpot + recentVelocity.Normalize() * 64, ignore_monsters, ENT(pev), &pTrace );
	
	//MODDD - new
	Vector explosionEffectStartRetail = pev->origin;
	Vector explosionEffectStartQuake = pev->origin;

	Vector explosionOrigin = pev->origin; //by default.

	// Pull out of the wall a bit
	if ( pTrace.flFraction != 1.0 )
	{
		//MODDD - let's not change our own origin, just record this.
		//pev->origin = ...
		//used to double as both the explosionOrigin and explosionEffectStartRetail ?
		explosionOrigin = pTrace.vecEndPos + (pTrace.vecPlaneNormal * (pev->dmg - 24) * 0.6);
		explosionEffectStartRetail = pTrace.vecEndPos + (pTrace.vecPlaneNormal * (pev->dmg*1.1 - 24) * 0.6);

		//MODDD
		explosionEffectStartQuake = pTrace.vecEndPos + (pTrace.vecPlaneNormal * 5);
		
	}

	//...
	CSoundEnt::InsertSound ( bits_SOUND_COMBAT, explosionOrigin, NORMAL_EXPLOSION_VOLUME / 2.0f, 3.0 );
	entvars_t *pevOwner;
	if ( pev->owner )
		pevOwner = VARS( pev->owner );
	else
		pevOwner = NULL;
	//...


	UTIL_Explosion(MSG_PVS, pev->origin, NULL, pev, explosionEffectStartRetail, spriteChosen, iScale, 15, TE_EXPLFLAG_NONE, explosionEffectStartQuake, shrapMod );
	//MODDD - sending explosionOrigin instead of defaulting to pev->origin.
	//RadiusDamageAutoRadius ( explosionOrigin, pev, pevOwner, pev->dmg, CLASS_NONE, bitsDamageType, bitsDamageTypeMod );
	RadiusDamage( explosionOrigin, pev, pevOwner, pev->dmg, 128, CLASS_NONE, DMG_BLAST | DMG_ALWAYSGIB );
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	//MODDD - sent through filter.
	UTIL_Explosion(pev, pev->origin, spriteChosen, iScale, 15, TE_EXPLFLAG_NONE);
	
	entvars_t *pevOwner;

	if ( pev->owner )
		pevOwner = VARS( pev->owner );
	else
		pevOwner = NULL;

	pev->owner = NULL; // can't traceline attack owner if this is set

	//easyForcePrintLine("!!!CROSSBOWBOLT: RADIUS DAMAGE CALL!!!");
	::RadiusDamage( pev->origin, pev, pevOwner, pev->dmg, 128, CLASS_NONE, DMG_BLAST | DMG_ALWAYSGIB );
	*/


	UTIL_Remove(this);
}



float CCrossbowBolt::massInfluence(void){
	return 0.02f;
}//END OF massInfluence

int CCrossbowBolt::GetProjectileType(void){
	return PROJECTILE_BOLT;
}


Vector CCrossbowBolt::GetVelocityLogical(void){
	if(!hitSomething){
		//moving, probably?
		return m_velocity;
	}else{
		//not moving.
		return Vector(0, 0, 0);
	}
}
//Likewise, if something else wants to change our velocity, and we pay more attention to something other than pev->velocty,
//we need to apply the change to that instead.  Or both, leaving that up to the thing implementing this.
void CCrossbowBolt::SetVelocityLogical(const Vector& arg_newVelocity){
	pev->velocity = arg_newVelocity;
	m_velocity = arg_newVelocity;
}


// Sparks not in water, bubbles underwater.
// Any oddities when hit near the waterlevel?  EHHhhhhh.   Dunno.
void CCrossbowBolt::hitEffect(const TraceResult& tr){

	if (UTIL_PointContents(pev->origin) != CONTENTS_WATER){
		UTIL_Sparks(pev->origin, DEFAULT_SPARK_BALLS, EASY_CVAR_GET_DEBUGONLY(sparksPlayerCrossbowMulti));
	}else{
		// why not generate some bubbles.
		UTIL_MakeAimVectors(pev->angles);
		Vector temp1 = tr.vecEndPos + tr.vecPlaneNormal * 2 + -gpGlobals->v_forward * 6 + Vector(-1.7, -1.7, -1.7);
		Vector temp2 = tr.vecEndPos + tr.vecPlaneNormal * 2 + -gpGlobals->v_forward * 6 + Vector(1.7, 1.7, 1.7);
		UTIL_Bubbles(temp1, temp2, 8);
	}

}//hitEffect

