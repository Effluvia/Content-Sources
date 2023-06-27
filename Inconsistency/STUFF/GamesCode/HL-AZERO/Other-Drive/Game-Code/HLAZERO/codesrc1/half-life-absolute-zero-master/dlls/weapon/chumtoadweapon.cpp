
#include "chumtoadweapon.h"
#include "chumtoad.h"
#include "util_debugdraw.h"


EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelay)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelaycustom)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteclip)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteammo)
EASY_CVAR_EXTERN_DEBUGONLY(playerChumtoadThrowDrawDebug)
EASY_CVAR_EXTERN_DEBUGONLY(chumtoadInheritsPlayerVelocity)

	
int CChumToadWeapon::numberOfEyeSkins = -1;



LINK_ENTITY_TO_CLASS( weapon_chumtoad, CChumToadWeapon );

/*
#if REMOVE_ORIGINAL_NAMES != 1
	LINK_ENTITY_TO_CLASS( weapon_chumtoad, CChumToadWeapon );
#endif

#if EXTRA_NAMES > 0
	//LINK_ENTITY_TO_CLASS( snarker, CChumToadWeapon );
	//no extras.
#endif
	*/




CChumToadWeapon::CChumToadWeapon(void){
	
	antiGravityPositionY = -1;
	
	//Holy crap, even "pev" stuff needs to be initialized by something. Not doing this here then.
	//pev->fuser1 = -1;
	waitingForChumtoadThrow = FALSE;


	//m_flReleaseThrow = -1;
	m_chargeReady &= ~32;
	
}



#ifndef CLIENT_DLL
TYPEDESCRIPTION	CChumToadWeapon::m_SaveData[] =
{
	DEFINE_FIELD( CChumToadWeapon, m_chargeReady, FIELD_INTEGER ),
};
IMPLEMENT_SAVERESTORE(CChumToadWeapon, CBasePlayerWeapon);
#endif








BOOL CChumToadWeapon::usesSoundSentenceSave(void){
	return FALSE;
}



//MODDD
void CChumToadWeapon::customAttachToPlayer(CBasePlayer *pPlayer ){
        //MODDD TODO ?
	//m_pPlayer->SetSuitUpdate("!HEV_SQUEEK", FALSE, SUIT_NEXT_IN_30SEC);
	SetThink(NULL);
    
}



// IMPORTANT!  Just dummy this out for clientside, it is meaningless there.
// Spawn call is just for calling the precache method, nothing more.
#ifndef CLIENT_DLL

// Returns whether the check passed (replacing this entity) or failed (not replacing it).
CBaseEntity* CChumToadWeapon::pickupWalkerReplaceCheck(void) {

	const char* pickupNameTest = "monster_chumtoad"; //GetPickupWalkerName();

	// No need to make it lowercase, classnames already can be trusted to be.
	//char tempClassname[127];
	//strcpy(&tempClassname[0], this->getClassname());
	//lowercase(tempClassname);

	if (!isStringEmpty(pickupNameTest) &&
		!(pev->spawnflags & SF_PICKUP_NOREPLACE) &&
		!stringEndsWith(this->getClassname(), "_noreplace"))
	{
		// lacking the NOREPLACE flag? replace with my spawnnable.

		//char pickupWalkerName[128];
		//sprintf(pickupWalkerName, "monster_%spickupwalker", baseclassname);

		//CBaseEntity::Create(pickupWalkerName, pev->origin, pev->angles);
		CBaseEntity* generated = CBaseEntity::Create(pickupNameTest, pev->origin, pev->angles);

		// By default, all entities made with "Create" get the SF_NORESPAWN flag.
		// This pickupwalker replaces this entity, so it should get that same flag only if the weapon it replaces had it too.
		if (pev->spawnflags & SF_NORESPAWN) {
			generated->pev->spawnflags |= SF_NORESPAWN;
		}
		else {
			generated->pev->spawnflags &= ~SF_NORESPAWN;
			// also, fade the monster on death.  Don't want to spam the game.
			generated->pev->spawnflags |= SF_MONSTER_FADECORPSE;
		}
		
		// And tell the generated pickupwalker that its current coords are the ones to use for respawning, regardless
		// of where it wanders off too.  (may or may not actually be respawnable)
		CBaseMonster* tempWalk = generated->GetMonsterPointer();
		if (tempWalk != NULL) {
			tempWalk->respawn_origin = pev->origin;
			tempWalk->respawn_angles = pev->angles;
		}


		UTIL_Remove(this);

		//easyForcePrintLine("pickupWalkerReplaceCheck TRUE");
		return generated;
	}
	//easyForcePrintLine("pickupWalkerReplaceCheck FALSE");
	return NULL;
}
const char* CChumToadWeapon::GetPickupWalkerName(void) {
	//MODDD - turn into monster_chumtoad instead, not using the pickup walker variant for chumtoads anymore.
	// Although the pickupwalker-replacement system isn't quite enough, monster_chumtoad itself is not
	// a subclass of CPickupWalker.   Need our own custom replacement method for that anyway.
	// IN SHORT, no behavior implied from this, drop it (empty string).
	// Although just have a name anyway, helps 'give' commands recognize this as something that wants 
	// to be replaced for turning that off (replacing something that had no collision, weapon_chumtoad, with
	// something that does, monster_chumtoad, that's going to the player origin, is just awkward).
	//return "monster_chumtoadpickupwalker";
	return "ass";
}
#else

CBaseEntity* CChumToadWeapon::pickupWalkerReplaceCheck(void) {
	return FALSE;
}
const char* CChumToadWeapon::GetPickupWalkerName(void) {
	//MODDD - turn into monster_chumtoad instead, not using the pickup walker variant for chumtoads anymore.
	// Although the pickupwalker-replacement system isn't quite enough, monster_chumtoad itself is not
	// a subclass of CPickupWalker.   Need our own custom replacement method for that anyway.
	// IN SHORT, no behavior implied from this, drop it (empty string).
	//return "monster_chumtoadpickupwalker";
	return "\0";
}

#endif





void CChumToadWeapon::Spawn( )
{
	// TODO!  Replace with a normal chumtoad instead, not the pickupwalker?

	//pev->classname = MAKE_STRING("weapon_chumtoad");

	const char* whut = NULL;

	if (pev->classname != 0) {
		whut = STRING(whut);
	}



	if(pickupWalkerReplaceCheck()){
		return;
	}

	Precache( );
	m_iId = WEAPON_CHUMTOAD;
	
    //MODDD - yes, this is the ammo pickup-able. Really using just the same chum toad "spawnable" (thrown) model?
	setModel("models/chumtoad.mdl");

	FallInit();//get ready to fall down.

	m_iClip = -1;
	m_iDefaultAmmo = CHUMTOAD_DEFAULT_GIVE;
	
	//MODDD - for now, manually set size.
	//UTIL_SetSize( pev, Vector( -10, -32, 0 ), Vector( 32, 32, 64 ) );
	UTIL_SetSize(pev, Vector(-16, -16, 0), Vector(16, 16, 24));

	/*
	pev->sequence = 1;
	pev->animtime = gpGlobals->time;
	pev->framerate = 1.0;
	*/


}


/*
void CChumToadWeapon::AttachToPlayer ( CBasePlayer *pPlayer )
{
	CBasePlayerItem::AttachToPlayer(pPlayer);
	SetThink(NULL);
}
*/


void CChumToadWeapon::FallInit( void )
{
	
	pev->movetype = MOVETYPE_TOSS;
	pev->solid = SOLID_BBOX;

	UTIL_SetOrigin( pev, pev->origin );
	UTIL_SetSize(pev, Vector( 0, 0, 0), Vector(0, 0, 0) );//pointsize until it lands on the ground.
	
	SetTouch( &CBasePlayerItem::DefaultTouch );

	//MODDD - use the alternate method to do something a little extra on touching the ground: rise up and rotate.
	//SetThink( &CBasePlayerItem::FallThink );
	SetThink(&CChumToadWeapon::FallThinkCustom);

	pev->nextthink = gpGlobals->time + 0.1;

}//END OF FallInit(...)



void CChumToadWeapon::FallThinkCustom ( void )
{
	pev->nextthink = gpGlobals->time + 0.1;

	if ( pev->flags & FL_ONGROUND )
	{
		// clatter if we have an owner (i.e., dropped by someone)
		// don't clatter if the gun is waiting to respawn (if it's waiting, it is invisible!)
		if ( !FNullEnt( pev->owner ) )
		{
			//int pitch = 95 + RANDOM_LONG(0,29);
			//UTIL_PlaySound(ENT(pev), CHAN_VOICE, "items/weapondrop1.wav", 1, ATTN_NORM, 0, pitch);
			//A toad is definitely not making that noise when it hits the ground.
		}

		// lie flat
		pev->angles.x = 0;
		pev->angles.z = 0;

		Materialize(); 
		
		this->pev->gravity = 0;
		this->pev->origin.z += 26;
		pev->movetype = MOVETYPE_FLY;

		antiGravityPositionY = pev->origin.z;

		//MODDD - new. Do the rotate effect.
		SetThink(&CChumToadWeapon::ItemRotate);
		pev->nextthink = gpGlobals->time + 0.02;

	}
}//END OF FallThinkCustom(...)




void CChumToadWeapon::ItemRotate ( void )
{
	//easyForcePrintLine("hahaha");
	this->pev->angles.y += 1.7;

	//this->pev->angles.x += sin(gpGlobals->time * 3.6) * 0.9;
	this->pev->angles.x = 0 + sin(gpGlobals->time * 3.6) * 14;
	
	//this->pev->origin.z += sin(gpGlobals->time * 3.6) * 0.31;
	this->pev->origin.z = antiGravityPositionY + sin(gpGlobals->time * 3.6) * 2.6;


	//gpGlobals->delt
	pev->nextthink = gpGlobals->time + 0.02;
}


void CChumToadWeapon::Precache( void )
{
	
	PRECACHE_MODEL("models/v_chub.mdl");
	PRECACHE_MODEL("models/chumtoad.mdl");

	////////////////////////////////////////////////////////////////////////////////////////////////
	//nevermind this, see "precacheAll" in Util.cpp for more info.
	//NOTE ABOUT WEAPONS NOT USING THIS!!! Player sounds are unconditionally always precached and do not use the sound-sentence save feature. W_Precache() should have these too then!
	//global_useSentenceSave = TRUE;

	PRECACHE_MODEL("models/p_chumtoad.mdl");
	/*
	//MODDD TODO: model / sound precaches. ALSO ADD TO Util.cpp's PRECACHEALL METHOD!
	PRECACHE_MODEL("models/p_chumtoad.mdl");
	PRECACHE_SOUND("squeek/sqk_hunt2.wav");
	PRECACHE_SOUND("squeek/sqk_hunt3.wav");
	*/
	//PRECACHE_SOUND("chumtoad/cht_throw1.wav");
	//PRECACHE_SOUND("chumtoad/cht_throw2.wav");
	PRECACHE_SOUND("chumtoad/chub_draw.wav");
	
	PRECACHE_SOUND("chumtoad/cht_croak_short.wav");
	PRECACHE_SOUND("chumtoad/cht_croak_medium.wav");
	PRECACHE_SOUND("chumtoad/cht_croak_long.wav");
	
	//just in case.
	precacheGunPickupSound();
	precacheAmmoPickupSound();
	//global_useSentenceSave = FALSE;
	////////////////////////////////////////////////////////////////////////////////////////////////

	UTIL_PrecacheOther("monster_chumtoad");

	m_usChumToadFire = PRECACHE_EVENT ( 1, "events/chumtoadfire.sc" );
}


//MODDD - for first-person player models, this won't work like you think it does.
void CChumToadWeapon::setModel(void){
	CChumToadWeapon::setModel(NULL);
}
void CChumToadWeapon::setModel(const char* m){
	CBasePlayerWeapon::setModel(m);
	
	/*
	//MODDD - IMPORTANT NOTE.
	// doing blinks here doesn't do much.  See hl_weapons.cpp where "numberOfEyeSkins" is used.
	// The counts are hardcoded there, unsure if "getNumberOfSkins" over there would work.

	if(numberOfEyeSkins == -1){
		//never loaded numberOfSkins? Do so.
		numberOfEyeSkins = getNumberOfSkins();
		if(numberOfEyeSkins == 0){
			easyPrintLine( "WARNING: Chumtoad (first person model) skin count is 0, error! Check v_chub.mdl for multiple skins. If it has them, please report this.  Forcing default of 3...");
			numberOfEyeSkins = 3;
		}else if(numberOfEyeSkins != 3){
			easyPrintLine( "WARNING: Chumtoad (first person model) skin count is %d, not 3. If v_chub.mdl does have 3 skins, please report this.", numberOfEyeSkins);
			if(numberOfEyeSkins < 1) numberOfEyeSkins = 1; //safety.
		}
	}
	*/

}//END OF setModel

int CChumToadWeapon::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "Chum Toads";
	p->iMaxAmmo1 = CHUMTOAD_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;

#if SPLIT_ALIEN_WEAPONS_INTO_NEW_SLOT != 1
	p->iSlot = 4;
	p->iPosition = 4;
#else
	// to the new slot you go!
	p->iSlot = 5;
	p->iPosition = 2;
#endif

	p->iId = m_iId = WEAPON_CHUMTOAD;
	p->iWeight = CHUMTOAD_WEIGHT;
	p->iFlags = ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE;

	return 1;
}


BOOL CChumToadWeapon::Deploy( )
{

	m_chargeReady &= ~32;

	if(!globalflag_muteDeploySound){
		// play hunt sound
		float flRndSound = RANDOM_FLOAT ( 0 , 1 );
		//MODDD TODO: CHANGE SOUNDS!!!    Roger roger.
		/*
		if ( flRndSound <= 0.5 ){
			UTIL_PlaySound(ENT(m_pPlayer->pev), CHAN_WEAPON, "chumtoad/cht_throw1.wav", 1, ATTN_NORM, 0, 100);
		}else{
			UTIL_PlaySound(ENT(m_pPlayer->pev), CHAN_WEAPON, "chumtoad/cht_throw2.wav", 1, ATTN_NORM, 0, 100);
		}
		*/
		UTIL_PlaySound(ENT(m_pPlayer->pev), CHAN_WEAPON, "chumtoad/chub_draw.wav", 1, ATTN_NORM, 0, 100);
		m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;
	}
	

	pev->fuser1 = -1;
	waitingForChumtoadThrow = FALSE;

	//MODDD - the 2nd argument, "szWeaponModel", is typically something like "models/p_NAME.mdl". Unsure what to do here.
	//Animation extension (4th argument) must be something that exists at the end of player animations (for the third person player model) beginning with 
	   //ref_aim_
	   //ref_shoot_
	   //crouch_aim_
	   //crouch_shoot_
	//ex: "crowbar" is one ending, producing ref_aim_crowbar, ref_shoot_crowbar, etc.  Can't make it up, has to exist by that name in player.mdl.
	//Reusing "squeak" player animations sounds ok.

	//Check the anim extension. Does the player have a "chum" third person anim to show up for other players if this matters?
	//return DefaultDeploy( "models/v_chub.mdl", "models/chumtoad.mdl", CHUMTOADWEAPON_UP, "chub", 0, 0, (36.0/36.0), (0.6) );
	return DefaultDeploy( "models/v_chub.mdl", "models/p_chumtoad.mdl", CHUMTOADWEAPON_UP, "squeak", 0, 0, (36.0/36.0), (0.6) );
}


void CChumToadWeapon::Holster( int skiplocal /* = 0 */ )
{
	//m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	
	pev->fuser1 = -1;
	waitingForChumtoadThrow = FALSE;

	m_chargeReady &= ~32;

	if ( PlayerPrimaryAmmoCount() <= 0)
	{
		m_pPlayer->pev->weapons &= ~(1<<WEAPON_CHUMTOAD);
		SetThink( &CBasePlayerItem::DestroyItem );
		pev->nextthink = gpGlobals->time + 0.1;
	}else{
		DefaultHolster(CHUMTOADWEAPON_DOWN, skiplocal, 0, (21.0f/21.0f));
		//??? what?
	}

	//SendWeaponAnim( CHUMTOADWEAPON_DOWN );
	
	UTIL_PlaySound(ENT(m_pPlayer->pev), CHAN_WEAPON, "common/null.wav", 1.0, ATTN_NORM, 0, 100, FALSE);
}



//Is it OK to throw a chumtoad towards where the player is facing now?
//  Check before throwing and right before spawning to make sure this hasn't changed in the new location, 
//  since the player might have moved since starting the throw while valid.
//ALSO, if the 
BOOL CChumToadWeapon::checkThrowValid(Vector trace_origin, float* minFractionStore){

	UTIL_MakeVectors( m_pPlayer->pev->v_angle );
	TraceResult tr;
	
	int flags;
	#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
	#else
		flags = 0;
	#endif




	// find place to toss monster
	//UTIL_TraceLine( trace_origin + gpGlobals->v_forward * 20, trace_origin + gpGlobals->v_forward * 100, dont_ignore_monsters, NULL, &tr );

	
	//typedef enum { point_hull=0, human_hull=1, large_hull=2, head_hull=3 };

	//SIGH. Even for things that let this pass, the chumtoad can get stuck on walls. Wow.
	//UTIL_TraceHull( trace_origin + gpGlobals->v_forward * 30, trace_origin + gpGlobals->v_forward * 100, dont_ignore_monsters, head_hull, m_pPlayer->edict(), &tr );

	
	TraceResult trLeft;
	TraceResult trCenter;
	TraceResult trRight;
	TraceResult trUp;
	TraceResult trDown;
	
	const float checkDistStart = 15;
	const float checkAdj = 12;
	const float checkDistEnd = 80;


	Vector vecStartLeft = trace_origin + gpGlobals->v_forward * checkDistStart + -gpGlobals->v_right * checkAdj;
	Vector vecStartCenter = trace_origin + gpGlobals->v_forward * checkDistStart;
	Vector vecStartRight = trace_origin + gpGlobals->v_forward * checkDistStart + gpGlobals->v_right * checkAdj;
	Vector vecEndLeft = trace_origin + gpGlobals->v_forward * checkDistEnd + -gpGlobals->v_right * checkAdj;
	Vector vecEndCenter = trace_origin + gpGlobals->v_forward * checkDistEnd;
	Vector vecEndRight = trace_origin + gpGlobals->v_forward * checkDistEnd + gpGlobals->v_right * checkAdj;
	
	Vector vecStartUp = trace_origin + gpGlobals->v_forward * checkDistStart + gpGlobals->v_up * checkAdj;
	Vector vecStartDown = trace_origin + gpGlobals->v_forward * checkDistStart + -gpGlobals->v_up * checkAdj;
	Vector vecEndUp = trace_origin + gpGlobals->v_forward * checkDistEnd + gpGlobals->v_up * checkAdj;
	Vector vecEndDown = trace_origin + gpGlobals->v_forward * checkDistEnd + -gpGlobals->v_up * checkAdj;

	UTIL_TraceLine( vecStartLeft, vecEndLeft, dont_ignore_monsters, this->m_pPlayer->edict(), &trLeft );
	UTIL_TraceLine( vecStartCenter, vecEndCenter, dont_ignore_monsters, this->m_pPlayer->edict(), &trCenter );
	UTIL_TraceLine( vecStartRight, vecEndRight, dont_ignore_monsters, this->m_pPlayer->edict(), &trRight );
	UTIL_TraceLine( vecStartUp, vecEndUp, dont_ignore_monsters, this->m_pPlayer->edict(), &trUp );
	UTIL_TraceLine( vecStartDown, vecEndDown, dont_ignore_monsters, this->m_pPlayer->edict(), &trDown );
	
	float minFraction;
	trLeft.flFraction<trRight.flFraction?minFraction=trLeft.flFraction:minFraction=trRight.flFraction;
	minFraction<trCenter.flFraction?minFraction=minFraction:minFraction=trCenter.flFraction;
	minFraction<trUp.flFraction?minFraction=minFraction:minFraction=trUp.flFraction;
	minFraction<trDown.flFraction?minFraction=minFraction:minFraction=trDown.flFraction;
	BOOL tracesSolid;
	BOOL tracesStartSolid;
	tracesSolid = (trLeft.fAllSolid != 0 || trCenter.fAllSolid != 0 || trRight.fAllSolid != 0 || trUp.fAllSolid != 0 || trDown.fAllSolid != 0);
	tracesStartSolid = (trLeft.fStartSolid != 0 || trCenter.fStartSolid != 0 || trRight.fStartSolid != 0 || trUp.fStartSolid != 0 || trDown.fStartSolid != 0);
	



		
#ifndef CLIENT_DLL
	if(EASY_CVAR_GET_DEBUGONLY(playerChumtoadThrowDrawDebug)){

		easyForcePrintLine("flFraction: %.2f %.2f %.2f %.2f %.2f :::%.2f", trLeft.flFraction, trCenter.flFraction, trRight.flFraction, trUp.flFraction, trDown.flFraction, minFraction);
		easyForcePrintLine("fAllSolid: %d %d %d %d %d :::%d", trLeft.fAllSolid, trCenter.fAllSolid, trRight.fAllSolid, trUp.fAllSolid, trDown.fAllSolid, tracesSolid);
		easyForcePrintLine("fStartSolid:  %d %d %d %d %d :::%d", trLeft.fStartSolid, trCenter.fStartSolid, trRight.fStartSolid, trUp.fStartSolid, trDown.fStartSolid, tracesStartSolid);
		easyForcePrintLine("FINAL STATS: minfract:%.2f tracesSolid:%d tracesStartSolid:%d", minFraction, tracesSolid, tracesStartSolid);
	

		if(tracesSolid){
			easyForcePrintLine("DANGER: TRACES ALLSOLID!");
		}if(tracesStartSolid){
			easyForcePrintLine("DANGER: TRACES fStartSolid!");
		}

		if(minFraction < 1.0){
			easyForcePrintLine("DANGER: incomplete fraction somewhere, look.");
		}

		//MODDD - IMPORTANT LESSON! A fraction (flFraction) can still be 1.0 and have collided with something as close as possible! Unknown why it reports 1.0 then instead of 0 (for 0% of the way).
		//Checking for "pHit being null" should work though.

		/*
		m_pPlayer->debugVect1Success = (trLeft.flFraction >= 1.0 && !trLeft.fStartSolid && !trLeft.fAllSolid);
		m_pPlayer->debugVect2Success = (trCenter.flFraction >= 1.0 && !trCenter.fStartSolid && !trCenter.fAllSolid);
		m_pPlayer->debugVect3Success = (trRight.flFraction >= 1.0 && !trRight.fStartSolid && !trRight.fAllSolid);
		m_pPlayer->debugVect4Success = (trUp.flFraction >= 1.0 && !trUp.fStartSolid && !trUp.fAllSolid);
		m_pPlayer->debugVect5Success = (trDown.flFraction >= 1.0 && !trDown.fStartSolid && !trDown.fAllSolid);
		*/

	
		if(trLeft.pHit != NULL){easyForcePrintLine("trLeft HIT %s", STRING(trLeft.pHit->v.classname));}
		if(trCenter.pHit != NULL){easyForcePrintLine("trCenter HIT %s", STRING(trCenter.pHit->v.classname));}
		if(trRight.pHit != NULL){easyForcePrintLine("trRight HIT %s", STRING(trRight.pHit->v.classname));}
		if(trUp.pHit != NULL){easyForcePrintLine("trUp HIT %s", STRING(trUp.pHit->v.classname));}
		if(trDown.pHit != NULL){easyForcePrintLine("trDown HIT %s", STRING(trDown.pHit->v.classname));}
	
		DebugLine_ClearAll();

		//HACKER SACKS.
		if(trLeft.fStartSolid && !trLeft.fAllSolid)trLeft.flFraction = 0;
		if(trCenter.fStartSolid && !trCenter.fAllSolid)trCenter.flFraction = 0;
		if(trRight.fStartSolid && !trRight.fAllSolid)trRight.flFraction = 0;
		if(trUp.fStartSolid && !trUp.fAllSolid)trUp.flFraction = 0;
		if(trDown.fStartSolid && !trDown.fAllSolid)trDown.flFraction = 0;

		DebugLine_Setup(0, vecStartLeft, vecEndLeft, trLeft.flFraction);
		DebugLine_Setup(1, vecStartCenter, vecEndCenter, trCenter.flFraction);
		DebugLine_Setup(2, vecStartRight, vecEndRight, trRight.flFraction);
		DebugLine_Setup(3, vecStartUp, vecEndUp, trUp.flFraction);
		DebugLine_Setup(4, vecStartDown, vecEndDown, trDown.flFraction);

		//wall: 4, 7     SOLID_BSP, MOVETYPE_PUSH
		//grunt: 3, 4    SOLID_SLIDEBOX, MOVETYPE_STEP
	
		/*
		easyForcePrintLine("WHAT DO YOU THINK? %.2f %d", trLeft.flFraction, trLeft.pHit!=NULL);

		if(trLeft.pHit != NULL){
			easyForcePrintLine("eee? %d %d", trLeft.pHit->v.solid, trLeft.pHit->v.movetype);
			::UTIL_printLineVector("siz?", trLeft.pHit->v.size);

			trLeft.pHit->v.solid = SOLID_BSP;
			trLeft.pHit->v.movetype = MOVETYPE_PUSH;

		}
		*/

		easyForcePrintLine("--------------------------------------------------------------");


			/*
		#ifndef CLIENT_DLL
			UTIL_drawLine(vecStartLeft, vecEndLeft);
			UTIL_drawLine(vecStartCenter, vecEndCenter);
			UTIL_drawLine(vecStartRight, vecEndRight);
			UTIL_drawLine(vecStartUp, vecEndUp);
			UTIL_drawLine(vecStartDown, vecEndDown);
		#endif
			*/

	}//END OF DEBUG DRAW CHECK
#endif



		/*
	#ifndef CLIENT_DLL
		Vector livingSizeMins = VEC_HUMAN_HULL_MIN + Vector(-45, -45, -8);
		Vector livingSizeMaxs = VEC_HUMAN_HULL_MAX + Vector(45, 45, 30);
		//UTIL_EntitiesInBox
		CBaseEntity *ent = NULL;
		CBaseEntity* theList[32];
		int theListSoftMax = UTIL_EntitiesInBox( theList, 32, pev->origin + livingSizeMins, pev->origin + livingSizeMaxs, 0 );
		for(int i = 0; i < theListSoftMax; i++){
			ent = theList[i];
			if ( !UTIL_IsDeadEntity(ent) && ent != this && ent->MyMonsterPointer() != NULL ){
				//easyPrintLine("WHAT THE heck IS YOUR darn ID %d", ent->MyMonsterPointer()->monsterID);
				//if(UTIL_IsAliveEntity(ent) && IRelationship(ent) <= R_NO ){
					//maybe send some "scramble position" advisory to get off of me, if friendly?
					//ent->needToMove = TRUE;
				//}
				//return FALSE;
			}
			easyForcePrintLine("FOUND %d: %s", i, ent->getClassname() );
		}
	#endif
		*/


	//MODDD NOTE - watch the required and thorw distance above (gpGlobals->forward * #) and flFraction! If it is too small, chumtoads can be thrown "through" walls, like map walls, and clip through. Looks weird and the toad leaves this universe.
	

	// ... || EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(testVar) == 2
	if ( (tracesSolid == FALSE && tracesStartSolid == FALSE && minFraction >= 1.0))
	//if ( tr.fAllSolid == 0 && tr.fStartSolid == 0 && tr.flFraction >= 1.0)
	{
		if(minFractionStore != NULL){ *minFractionStore = minFraction; }  //on success, the caller wants to know the minimum fraction seen, if a place to put it is provided.
		return TRUE;
	}

	return FALSE;
}//END OF checkThrowValid


// Spawns a chumtoad in front of the player with some velocity to go forwards.
void CChumToadWeapon::ThrowChumtoad(Vector vecSpawnPoint){

	Vector toadSpawnPoint;
	toadSpawnPoint = vecSpawnPoint;

#ifndef CLIENT_DLL
	//only send YAW. angle.y.
	
	//MODDD TODO - throw sound (creature noise)?
	UTIL_PlaySound(ENT(pev), CHAN_VOICE, "chumtoad/cht_croak_short.wav", 1, ATTN_NORM, 0, RANDOM_LONG(98, 106));

	// Little range boost, but not much.  Point is to be stealthy with this.
	m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME + 80;

	CBaseEntity *pChumToad = CBaseEntity::Create( "monster_chumtoad", toadSpawnPoint, Vector(0, m_pPlayer->pev->v_angle.y, 0), SF_MONSTER_THROWN, m_pPlayer->edict() );
	
	if (pChumToad != NULL) {
		pChumToad->pev->velocity = gpGlobals->v_forward * 200 + UTIL_GetProjectileVelocityExtra(m_pPlayer->pev->velocity, EASY_CVAR_GET_DEBUGONLY(chumtoadInheritsPlayerVelocity));
	}
#endif
	//MODDD - cheat check
	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteclip) == 0 && EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteammo) == 0){
		ChangePlayerPrimaryAmmoCount(-1);
	}

}//END OF ThrowChumtoad()



void CChumToadWeapon::PrimaryAttack()
{
#ifndef CLIENT_DLL
	if (PlayerPrimaryAmmoCount() > 0)
	{
		Vector trace_origin;
		// HACK HACK:  Ugly hacks to handle change in origin based on new physics code for players
		// Move origin up if crouched and start trace a bit outside of body ( 20 units instead of 16 )
		trace_origin = m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs;
		if ( m_pPlayer->pev->flags & FL_DUCKING )
		{
			trace_origin = trace_origin - ( VEC_HULL_MIN - VEC_DUCK_HULL_MIN );
		}

		float minFraction;
		if(checkThrowValid(trace_origin, &minFraction)){
			//Come to think of it... what's the point of playing this event clientside? We can't guarantee it is in sync for things like say,
			//a failed chumtoad throw because tracechecks with map geometry to determine a successful area for throwing or a block isn't possible clientside. Only server.
			//Just send the anim the old-fashioned way.
			//PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usChumToadFire, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, 0, 0, 0, 0 );

			forceBlockLooping();
			//bypass??
			SendWeaponAnimBypass( CHUMTOADWEAPON_THROW | 0 );
			// player "shoot" animation
			m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

			//MODDD - old throw location, now throwing with a little delay...

			m_chargeReady |= 32;
			//sendJustThrown( ENT(m_pPlayer->pev), m_fJustThrown);

			//MODDD 
			if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelay) == 0){
				SetAttackDelays(UTIL_WeaponTimeBase() + CHUMTOAD_THROW_DELAY + 1.4f);
				//chumtoadThrowReverseDelay = m_flNextPrimaryAttack - CHUMTOAD_THROW_DELAY;  //time to throw the chumtoad, counting backwards.
				pev->fuser1 = UTIL_WeaponTimeBase() + CHUMTOAD_THROW_DELAY;
			}else{
				//extra delay so that the things don't just collide with each other and... start defying gravity I guess.
				SetAttackDelays(UTIL_WeaponTimeBase() + EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelaycustom) + 0.03f);
				//chumtoadThrowReverseDelay = m_flNextPrimaryAttack - 0.1f;
				pev->fuser1 = UTIL_WeaponTimeBase() + 0.03f;
			}

			//NOTE: this ends up being the delay before doing the re-draw animation (can still fire before then, unaffected by the time of the "throw" animation that hides the hands)
			//To be clear, the "(# / #)" part is still just animation frames divided by animation framerate.
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (16.0/ 24.0) + 0.1f;



			//Keep the client in sync, send this.   .... why?
			//sendTimeWeaponIdleUpdate( ENT(m_pPlayer->pev), m_flTimeWeaponIdle);

			//server-only, but if it updates for the client, no problem. The only effect, a spawned chumtoad, is server-only anyways.
			waitingForChumtoadThrow = TRUE;
			//...not working for some reason.
			//pev->fuser1 = 2;
		}
	}

#endif
	//m_flTimeWeaponIdle = 666;
}//END OF PrimaryAttack(...)



void CChumToadWeapon::SecondaryAttack( void )
{

	if (EASY_CVAR_GET(cl_viewmodel_fidget) == 2) {
		float flRand;
		int iAnim;

		flRand = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 0, 1);
		if (flRand <= 0.5)
		{
			iAnim = CHUMTOADWEAPON_FIDGETLICK;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 30.0 / 16.0;
		}
		else
		{
			iAnim = CHUMTOADWEAPON_FIDGETCROAK;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 50.0 / 16.0;
		}
		//SetAttackDelays(m_flTimeWeaponIdle);
		m_flNextSecondaryAttack = m_flTimeWeaponIdle;
		m_flTimeWeaponIdle += randomIdleAnimationDelay();
		SendWeaponAnim(iAnim);
	}//CVar check

}



float CChumToadWeapon::randomIdleAnimationDelay(){
	return 0; //no static delays for me.
}

void CChumToadWeapon::ItemPostFrame(){
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


#ifndef CLIENT_DLL
	//easyForcePrintLine("WHAT? npa:%.2f: ctrd:%.2f wfct:%d", m_flNextPrimaryAttack, pev->fuser1, waitingForChumtoadThrow);
	//if(m_flStartThrow != -1 && m_flStartThrow <= 0){
	//if(m_flNextPrimaryAttack > 0 && chumtoadThrowReverseDelay != -1 && m_flNextPrimaryAttack <= chumtoadThrowReverseDelay){
	if(pev->fuser1 <= 0 && waitingForChumtoadThrow){
		//time expired? Throw chumtoad.
		Vector trace_origin;
		// HACK HACK:  Ugly hacks to handle change in origin based on new physics code for players
		// Move origin up if crouched and start trace a bit outside of body ( 20 units instead of 16 )
		trace_origin = m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs;
		if ( m_pPlayer->pev->flags & FL_DUCKING )
		{
			trace_origin = trace_origin - ( VEC_HULL_MIN - VEC_DUCK_HULL_MIN );
		}
		float minFraction;

		if(this->checkThrowValid(trace_origin, &minFraction)){
			Vector toadSpawnPoint = trace_origin + gpGlobals->v_forward * 100 * (minFraction - 0.36);
			this->ThrowChumtoad(toadSpawnPoint);
		}else{
			//interrupt throw anim?
			m_flTimeWeaponIdle = 0;
			//sendTimeWeaponIdleUpdate( ENT(m_pPlayer->pev), m_flTimeWeaponIdle);
		}
		//m_flStartThrow = -1;
		//chumtoadThrowReverseDelay = -1;  //don't throw again this time.
		
		waitingForChumtoadThrow = FALSE;
		//pev->fuser1 = -1;
	}
	
#endif

}//END OF ItemPostFrame





void CChumToadWeapon::ItemPostFrameThink(void) {


	//if (m_fJustThrown)
	if ((m_chargeReady & 32) && m_flTimeWeaponIdle <= UTIL_WeaponTimeBase())
	{
		m_chargeReady &= ~32;


//#ifndef CLIENT_DLL
//		sendJustThrown(ENT(m_pPlayer->pev), m_fJustThrown);
//#endif
		if (PlayerPrimaryAmmoCount() <= 0)
		{
			RetireWeapon();
			return;
		}

		SendWeaponAnim(CHUMTOADWEAPON_UP);
		//MODDD - no static.  Start another anim right at the end of it.
		//m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (36 / 36.0) + randomIdleAnimationDelay();
		return;
	}

	CBasePlayerWeapon::ItemPostFrameThink();
}//ItemPostFrameThink






void CChumToadWeapon::WeaponIdle( void )
{
	//return;
	//forceBlockLooping();


	if(m_pPlayer->pev->viewmodel == iStringNull){
		if (PlayerPrimaryAmmoCount() > 0 ){
			// let's not play the deploy sound.
			// MODDD - TODO: make a parameter for Deploy perhaps?
			globalflag_muteDeploySound = TRUE;
	        Deploy();
			globalflag_muteDeploySound = FALSE;

			return;
		}
	}
	
	//is this redundant now??
	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;



	if (PlayerPrimaryAmmoCount() <= 0) {
		// no.  No idle delays, they just make ammo pickups take longer to take effect with the empty viewmodel still up.
		return;
	}


	//Now that there isn't a static delay for living throwables, the odds of a unique idle anim have been slightly reduced.
	//Old ones were:
	//      if  flRand <= 0.75
	//... else  flRand <= 0.875

	//MODDD TODO - animations here.
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
		iAnim = CHUMTOADWEAPON_IDLE1;
		//MODDD - don't double length, already matches anim time.
		//m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 30.0 / 16 * (2);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 30.5 / 16 + randomIdleAnimationDelay();
	}
	else if (flRand <= 0.82 + 0.09)
	{
		iAnim = CHUMTOADWEAPON_FIDGETLICK;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 30.0 / 16.0 + randomIdleAnimationDelay();
	}
	else
	{
		iAnim = CHUMTOADWEAPON_FIDGETCROAK;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 50.0 / 16.0 + randomIdleAnimationDelay();
	}

	SendWeaponAnim( iAnim );
	
	//CHUMTOADWEAPON_IDLE1 = 0, //31, 16
	//CHUMTOADWEAPON_FIDGETLICK, //31, 16
	//CHUMTOADWEAPON_FIDGETCROAK, //51, 16
}//END OF WeaponIdle(...)



CChumToadWeapon_NoReplace::CChumToadWeapon_NoReplace() {

}//END OF constructor

LINK_ENTITY_TO_CLASS(weapon_chumtoad_noreplace, CChumToadWeapon_NoReplace);


void CChumToadWeapon_NoReplace::Spawn(void) {
	CChumToadWeapon::Spawn();

	// after that call, may as well lose the "_noreplace" bit.
	pev->classname = MAKE_STRING("weapon_chumtoad");
}

