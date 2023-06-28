/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/
/*

===== turret.cpp ========================================================

*/

//MODDD - NOTE.  Comment from as-is codebase.  I see no OFFSET stuff so I guess that already 
// happened.
// TODO: 
//		Take advantage of new monster fields like m_hEnemy and get rid of that OFFSET() stuff
//		Revisit enemy validation stuff, maybe it's not necessary with the newest monster code
//


// MODDD TODO - make the classes here available in a turrets.h file so that the statics can be reset on map changes like the ServerActivate method happening? Before precaches occur?
//CMiniTurret::gibModelRef;

// Move some things from basemonster.h to monsters.h, if not everything? Or rename monsters.cpp / monsters.h to basemonster.cpp and keep it all in basemonster.h ?

// Make the Mr. Friendly reference in player.cpp less error prone!!!

#include "extdll.h"
#include "turret.h"
#include "util.h"
#include "cbase.h"
#include "basemonster.h"
#include "weapons.h"
#include "effects.h"
#include "gib.h"
#include "util_debugdraw.h"

//MODDD
EASY_CVAR_EXTERN_DEBUGONLY(sparksTurretDeathMulti)
EASY_CVAR_EXTERN_DEBUGONLY(turretCanGib)
EASY_CVAR_EXTERN_DEBUGONLY(miniturretCanGib)
EASY_CVAR_EXTERN_DEBUGONLY(sentryCanGib)
EASY_CVAR_EXTERN_DEBUGONLY(turretBleedsOil)
EASY_CVAR_EXTERN_DEBUGONLY(turretDamageDecal)
EASY_CVAR_EXTERN_DEBUGONLY(turretGibDecal)
EASY_CVAR_EXTERN_DEBUGONLY(crazyMonsterPrintouts)
EASY_CVAR_EXTERN(sv_turret_postdeath)


extern DLL_GLOBAL float g_rawDamageCumula;




#define TURRET_SHOTS	2
//MODDD - more turret range, was 100 * 12.
#define TURRET_RANGE	(130 * 12)
#define TURRET_SPREAD	Vector( 0, 0, 0 )
#define TURRET_TURNRATE	30		//angles per 0.1 second


#define TURRET_MAXWAIT	15		// seconds turret will stay active w/o a target
#define TURRET_MAXSPIN	5		// seconds turret barrel will spin w/o a target
// NOTICE!!!  Sentries have a huge default idle-until-sleep-time (100,000 or so), if keyvalue "maxsleep" 
// is not set by the map.  In fact why not have its own constants.
#define SENTRY_MAXWAIT 1E6;
#define SENTRY_MAXSPIN 1E6;


#define TURRET_MACHINE_VOLUME	0.5

#define TURRET_GLOW_SPRITE "sprites/flare3.spr"

#define HITGROUP_TURRET_ARMOR 10





static GibInfo_t* getTurretGibInfo(float cvarRef){
	
	if(cvarRef == 3){
		return &aryGibInfo[GIB_EXTRAMETAL_1_ID];
	}
	if(cvarRef == 4){
		return &aryGibInfo[GIB_EXTRAMETAL_2_ID];
	}
	if(cvarRef == 5){
		return &aryGibInfo[GIB_EXTRAMETAL_3_ID];
	}
	if(cvarRef == 6){
		return &aryGibInfo[GIB_EXTRAMETAL_4_ID];
	}
	if(cvarRef == 7){
		return &aryGibInfo[GIB_EXTRAMETAL_5_ID];
	}
	if(cvarRef == 8){
		return &aryGibInfo[GIB_EXTRAMETAL_6_ID];
	}

	//fall back: the DUMMY to signify that we tried.
	return &aryGibInfo[GIB_DUMMY_ID];
}



static int TurretBloodColorBlackFilter(){
	// Whether it bleeds in whatever way, just say the color is BLACK regardless.
	// Other logic will handle blocking other types of effects.
	return BLOOD_COLOR_BLACK;
}








//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//CBaseTurret

TYPEDESCRIPTION	CBaseTurret::m_SaveData[] = 
{
	DEFINE_FIELD( CBaseTurret, m_flMaxSpin, FIELD_FLOAT ),
	DEFINE_FIELD( CBaseTurret, m_iSpin, FIELD_INTEGER ),

	DEFINE_FIELD( CBaseTurret, m_pEyeGlow, FIELD_CLASSPTR ),
	DEFINE_FIELD( CBaseTurret, m_eyeBrightness, FIELD_INTEGER ),
	DEFINE_FIELD( CBaseTurret, m_iDeployHeight, FIELD_INTEGER ),
	DEFINE_FIELD( CBaseTurret, m_iRetractHeight, FIELD_INTEGER ),
	DEFINE_FIELD( CBaseTurret, m_iMinPitch, FIELD_INTEGER ),

	DEFINE_FIELD( CBaseTurret, m_iBaseTurnRate, FIELD_INTEGER ),
	DEFINE_FIELD( CBaseTurret, m_fTurnRate, FIELD_FLOAT ),
	DEFINE_FIELD( CBaseTurret, m_iOrientation, FIELD_INTEGER ),
	DEFINE_FIELD( CBaseTurret, m_iOn, FIELD_INTEGER ),
	DEFINE_FIELD( CBaseTurret, m_fBeserk, FIELD_INTEGER ),
	DEFINE_FIELD( CBaseTurret, m_iAutoStart, FIELD_INTEGER ),


	DEFINE_FIELD( CBaseTurret, m_vecLastSight, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( CBaseTurret, m_flLastSight, FIELD_TIME ),
	DEFINE_FIELD( CBaseTurret, m_flMaxWait, FIELD_FLOAT ),
	DEFINE_FIELD( CBaseTurret, m_iSearchSpeed, FIELD_INTEGER ),

	DEFINE_FIELD( CBaseTurret, m_flStartYaw, FIELD_FLOAT ),
	DEFINE_FIELD( CBaseTurret, m_vecCurAngles, FIELD_VECTOR ),
	DEFINE_FIELD( CBaseTurret, m_vecGoalAngles, FIELD_VECTOR ),

	DEFINE_FIELD( CBaseTurret, m_flPingTime, FIELD_TIME ),
	DEFINE_FIELD( CBaseTurret, m_flSpinUpTime, FIELD_TIME ),
};

IMPLEMENT_SAVERESTORE( CBaseTurret, CBaseMonster );


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//CTurret

GibInfo_t* CTurret::gibModelRef = NULL;

TYPEDESCRIPTION	CTurret::m_SaveData[] = 
{
	DEFINE_FIELD( CTurret, m_iStartSpin, FIELD_INTEGER ),
};

IMPLEMENT_SAVERESTORE( CTurret, CBaseTurret );


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//CMiniTurret


GibInfo_t* CMiniTurret::gibModelRef = NULL;


#if REMOVE_ORIGINAL_NAMES != 1
	LINK_ENTITY_TO_CLASS( monster_turret, CTurret );
	LINK_ENTITY_TO_CLASS( monster_miniturret, CMiniTurret );
#endif

#if EXTRA_NAMES > 0
	LINK_ENTITY_TO_CLASS( turret, CTurret );
	LINK_ENTITY_TO_CLASS( miniturret, CMiniTurret );
	
#endif





void CBaseTurret::KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "maxsleep"))
	{
		m_flMaxWait = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "orientation"))
	{
		m_iOrientation = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;

	}
	else if (FStrEq(pkvd->szKeyName, "searchspeed"))
	{
		m_iSearchSpeed = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;

	}
	else if (FStrEq(pkvd->szKeyName, "turnrate"))
	{
		m_iBaseTurnRate = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "style") ||
			 FStrEq(pkvd->szKeyName, "height") ||
			 FStrEq(pkvd->szKeyName, "value1") ||
			 FStrEq(pkvd->szKeyName, "value2") ||
			 FStrEq(pkvd->szKeyName, "value3"))
		pkvd->fHandled = TRUE;
	else
		CBaseMonster::KeyValue( pkvd );
}

CBaseTurret::CBaseTurret(){
	postDeathEndTime = -1;
	nextDeathExplosionTime = -1;
	postDeathBeserkDir = -1;
	postDeathEyeBrightnessDir = -1;
}


/*
//more fitting name would be "Get" BloodColor.
//...nevermind, don't need to override this.
void CBaseTurret::BloodColor(){

}
*/



void CBaseTurret::StartReanimation(void){
	int i;

	// ???????
	//m_IdealMonsterState = MONSTERSTATE_ALERT;// Assume monster will be alert, having come back from the dead and all.
	//m_MonsterState = MONSTERSTATE_ALERT; //!!!

	//this->m_Activity = ACT_RESET;


	pev->deadflag = DEAD_NO;

	//before spawn or init script may interfere.
	int oldSeq = pev->sequence;
	//no recollection of that.
	m_hEnemy = NULL;

	//And clear the list of old enemies.
	//or m_intOldEnemyNextIndex - 1 ?
	for(i = 0; i < MAX_OLD_ENEMIES; i++){
		m_hOldEnemy[i] = NULL;
	}

	// don't call spawn yet, do that after revive.
	//Spawn();


	//Most of MonsterInit's script here just to be safe... Nah, assume Spawn calls it if it makes sense to.
	// The area mentioned ranged from
	//     pev->effects		= 0;
	// to
	//     SetEyePosition();

	StartReanimationPost(oldSeq);
}//END OF StartReanimation


// cloned, a lot about the turret is differnet or stripped-down from most monster AI.
// No schedule system for starters.
void CBaseTurret::StartReanimationPost(int preReviveSequence){

	// override the Initialize think!
	SetThink(&CBaseTurret::ReviveThink);
	pev->nextthink = gpGlobals->time + 0.1;

	m_afMemory = MEMORY_CLEAR;

	pev->sequence = -1; //force reset.
	SetSequenceByIndex(preReviveSequence, -1, FALSE);



	//pev->sequence		= 0;
	//pev->frame			= 0;
	//pev->health = 1;
	//pev->movetype = MOVETYPE_FLY;
	//pev->solid = SOLID_SLIDEBOX;
	//pev->fuser1 = 8;

	// oookay then
	if(FClassnameIs(pev, "monster_sentry")){
		pev->origin.z += 1;
	}

	//pev->flags &= ~FL_NO
	//pev->effects &= ~EF_NODRAW;


	// I need more help!... nevermind.
	//m_iOn = TRUE;

	//ChangeSchedule(slWaitForReviveSequence);
}//END OF StartReanimationPost


void CBaseTurret::ReviveThink(void){

	StudioFrameAdvance_SIMPLE( );

	if(m_fSequenceFinished){

		pev->frame = 0;
		pev->framerate = 1;
		m_flFramerateSuggestion = 1;

		Spawn();

		/*
		if (m_iAutoStart)
		{
			m_flLastSight = gpGlobals->time + m_flMaxWait;
			SetThink(&CBaseTurret::AutoSearchThink);
			pev->nextthink = gpGlobals->time + .1;
		}
		else
			SetThink(&CBaseEntity::SUB_DoNothing);
		*/
		return;
	}

	SetThink(&CBaseTurret::ReviveThink);
	pev->nextthink = gpGlobals->time + 0.1;
}



void CBaseTurret::Spawn()
{ 
	//NOTICE - only precache my own things. Other spawn methods call their own precaches already.
	// This will still end up getting called twice per turret since each turret's precache method calls the parent CBaseTurret::Precache,
	// and each turret calls their parent CBaseTurret::Spawn which calls CBaseTruret::Precache right here.
	// But it's common for precache methods to have redundant calls, like every other thing of the same class calling Precache at startup.
	// No problem here.
	CBaseTurret::Precache( );

	pev->nextthink		= gpGlobals->time + 1;
	pev->movetype		= MOVETYPE_FLY;
	pev->sequence		= 0;
	pev->frame			= 0;
	pev->solid			= SOLID_SLIDEBOX;
	pev->takedamage		= DAMAGE_AIM;

	m_bloodColor = TurretBloodColorBlackFilter();
	
	SetBits (pev->flags, FL_MONSTER);
	SetUse( &CBaseTurret::TurretUse );

	if (( pev->spawnflags & SF_MONSTER_TURRET_AUTOACTIVATE ) 
		 && !( pev->spawnflags & SF_MONSTER_TURRET_STARTINACTIVE ))
	{
		m_iAutoStart = TRUE;
	}

	ResetSequenceInfo( );
	SetBoneController( 0, 0 );
	SetBoneController( 1, 0 );
	m_flFieldOfView = VIEW_FIELD_FULL;
	// m_flSightRange = TURRET_RANGE;

	//MODDD - usually called by "monsterInit".
	pev->renderfx |= ISMETALNPC;
	
	pev->max_health		= pev->health;

	// No need for other turrets to do this, all others call this parent spawn method and reach here.
	if (monsterID == -1) {
		//MODDD - must do manually since init is skipped.
		monsterID = monsterIDLatest;
		monsterIDLatest++;
	}

}



extern int global_useSentenceSave;
void CBaseTurret::Precache( )
{
	global_useSentenceSave = TRUE;
	PRECACHE_SOUND ("turret/tu_fire1.wav");
	PRECACHE_SOUND ("turret/tu_ping.wav");
	PRECACHE_SOUND ("turret/tu_active2.wav");
	PRECACHE_SOUND ("turret/tu_die.wav");
	PRECACHE_SOUND ("turret/tu_die2.wav");
	PRECACHE_SOUND ("turret/tu_die3.wav");
	// PRECACHE_SOUND ("turret/tu_retract.wav"); // just use deploy sound to save memory
	PRECACHE_SOUND ("turret/tu_deploy.wav");
	PRECACHE_SOUND ("turret/tu_spinup.wav");
	PRECACHE_SOUND ("turret/tu_spindown.wav");
	PRECACHE_SOUND ("turret/tu_search.wav");
	PRECACHE_SOUND ("turret/tu_alert.wav");
	global_useSentenceSave = FALSE;
}


float CTurret::getGibCVar(){
	return EASY_CVAR_GET_DEBUGONLY(turretCanGib);
}

void CTurret::Spawn()
{ 
	// TEST.  Default is floor (0), used when spawned by commands otherwise.
	//m_iOrientation = 1;

	Precache( );
	SET_MODEL(ENT(pev), "models/turret.mdl");
	pev->health			= gSkillData.turretHealth;
	m_HackedGunPos		= Vector( 0, 0, 12.75 );
	m_flMaxSpin =		TURRET_MAXSPIN;
	pev->view_ofs.z = 12.75;

	CBaseTurret::Spawn( );

	pev->classname = MAKE_STRING("monster_turret");

	//MODDD - bit more there?  Odd for projectiles to go through the top portion
	//m_iRetractHeight = 16;
	//m_iDeployHeight = 32;
	m_iRetractHeight = 18;
	m_iDeployHeight = 46;

	m_iMinPitch	= -15;
	UTIL_SetSize(pev, Vector(-32, -32, -m_iRetractHeight), Vector(32, 32, m_iRetractHeight));
	
	SetThink(&CBaseTurret::Initialize);	

	m_pEyeGlow = CSprite::SpriteCreate( TURRET_GLOW_SPRITE, pev->origin, FALSE );
	m_pEyeGlow->SetTransparency( kRenderGlow, 255, 0, 0, 0, kRenderFxNoDissipation );
	m_pEyeGlow->SetAttachment( edict(), 2 );
	m_eyeBrightness = 0;

	pev->nextthink = gpGlobals->time + 0.3;
}

void CTurret::Precache()
{
	CBaseTurret::Precache( );
	PRECACHE_MODEL ("models/turret.mdl");	
	PRECACHE_MODEL (TURRET_GLOW_SPRITE);

	if(gibModelRef == NULL)gibModelRef = getTurretGibInfo(getGibCVar());
	if(!isStringEmpty(gibModelRef->modelPath)){
		PRECACHE_MODEL((char*)gibModelRef->modelPath);
	}

}


float CMiniTurret::getGibCVar(){
	return EASY_CVAR_GET_DEBUGONLY(miniturretCanGib);
}

void CMiniTurret::Spawn()
{ 
	Precache( );
	SET_MODEL(ENT(pev), "models/miniturret.mdl");
	pev->health			= gSkillData.miniturretHealth;
	m_HackedGunPos		= Vector( 0, 0, 12.75 );
	m_flMaxSpin = 0;
	pev->view_ofs.z = 12.75;

	CBaseTurret::Spawn( );

	pev->classname = MAKE_STRING("monster_miniturret");

	m_iRetractHeight = 16;
	m_iDeployHeight = 32;
	m_iMinPitch	= -15;
	UTIL_SetSize(pev, Vector(-16, -16, -m_iRetractHeight), Vector(16, 16, m_iRetractHeight));


	// NOTE - miniturret doesn't get an eye (m_pEyeGlow).  See CTurret where it's set up, if
	// this should get an eye too.   Maybe smaller.


	SetThink(&CBaseTurret::Initialize);
	pev->nextthink = gpGlobals->time + 0.3;
}


void CMiniTurret::Precache()
{
	CBaseTurret::Precache( );
	PRECACHE_MODEL ("models/miniturret.mdl");	
	global_useSentenceSave = TRUE;
	PRECACHE_SOUND("weapons/hks1.wav", TRUE); //player sounds.
	PRECACHE_SOUND("weapons/hks2.wav", TRUE);
	PRECACHE_SOUND("weapons/hks3.wav", TRUE);
	global_useSentenceSave = FALSE;

	if(gibModelRef == NULL)gibModelRef = getTurretGibInfo(getGibCVar());
	if(!isStringEmpty(gibModelRef->modelPath)){
		PRECACHE_MODEL((char*)gibModelRef->modelPath);
	}

}

void CBaseTurret::Initialize(void)
{
	m_iOn = 0;
	m_fBeserk = 0;
	m_iSpin = 0;

	SetBoneController( 0, 0 );
	SetBoneController( 1, 0 );

	if (m_iBaseTurnRate == 0) m_iBaseTurnRate = TURRET_TURNRATE;
	if (m_flMaxWait == 0) m_flMaxWait = TURRET_MAXWAIT;
	m_flStartYaw = pev->angles.y;
	if (m_iOrientation == 1)
	{
		pev->idealpitch = 180;
		pev->angles.x = 180;
		pev->view_ofs.z = -pev->view_ofs.z;
		pev->effects |= EF_INVLIGHT;
		pev->angles.y = pev->angles.y + 180;
		if (pev->angles.y > 360)
			pev->angles.y = pev->angles.y - 360;
	}

	m_vecGoalAngles.x = 0;

	//MODDD - NEW. Implement per turret as needed.
	PostInit();

	if (m_iAutoStart)
	{
		m_flLastSight = gpGlobals->time + m_flMaxWait;
		SetThink(&CBaseTurret::AutoSearchThink);
		pev->nextthink = gpGlobals->time + .1;
	}
	else
		SetThink(&CBaseEntity::SUB_DoNothing);
}

// implement per turret as needed, called by Initialize above.
void CBaseTurret::PostInit(void) {
	// ...
}

void CBaseTurret::TurretUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( !ShouldToggle( useType, m_iOn ) )
		return;

	if (m_iOn)
	{
		m_hEnemy = NULL;
		pev->nextthink = gpGlobals->time + 0.1;
		m_iAutoStart = FALSE;// switching off a turret disables autostart
		//!!!! this should spin down first!!BUGBUG
		SetThink(&CBaseTurret::Retire);
	}
	else 
	{
		pev->nextthink = gpGlobals->time + 0.1; // turn on delay

		// if the turret is flagged as an autoactivate turret, re-enable it's ability open self.
		if ( pev->spawnflags & SF_MONSTER_TURRET_AUTOACTIVATE )
		{
			m_iAutoStart = TRUE;
		}
		
		SetThink(&CBaseTurret::Deploy);
	}
}


void CBaseTurret::Ping( void )
{
	// make the pinging noise every second while searching
	if (m_flPingTime == 0)
		m_flPingTime = gpGlobals->time + 1;
	else if (m_flPingTime <= gpGlobals->time)
	{
		m_flPingTime = gpGlobals->time + 1;
		// MODDD - play off-pitch pings in beserk mode.
		if (m_fBeserk == 0) {
			UTIL_PlaySound(ENT(pev), CHAN_ITEM, "turret/tu_ping.wav", 1, ATTN_NORM);
		}else{
			// off-pitch!
			UTIL_PlaySound(ENT(pev), CHAN_STATIC, "turret/tu_ping.wav", 1, ATTN_NORM - 0.2, 0, RANDOM_LONG(103, 105));
		}
		EyeOn( );
	}
	else if (m_eyeBrightness > 0)
	{
		EyeOff( );
	}
}



// WAIT.  So EyeOn sets the brightness to exactly 255 (max).
// EyeOff, reduces the brightness by 30 only.
// MMMMMMmmmmmmmmmaaaaaaaakkkkkkkkeeeeeeeesssssss sssssseeeeeeennnnnnnnssssssseeeeeee ttttttttooooooo mmmmmmmmmmeeeeeeeeeeeeee
void CBaseTurret::EyeOn( )
{
	if (m_pEyeGlow)
	{
		if (m_eyeBrightness != 255)
		{
			m_eyeBrightness = 255;
		}
		m_pEyeGlow->SetBrightness( m_eyeBrightness );
	}
}


void CBaseTurret::EyeOff( )
{
	if (m_pEyeGlow)
	{
		if (m_eyeBrightness > 0)
		{
			//MODDD - brightness reduction each frame changed, was 30
			m_eyeBrightness = max( 0, m_eyeBrightness - 36 );
			m_pEyeGlow->SetBrightness( m_eyeBrightness );
		}
	}
}


void CBaseTurret::onDelete(void) {
	if (m_pEyeGlow) {
		// remove it.
		UTIL_Remove(m_pEyeGlow);
		m_pEyeGlow = NULL;
	}
}

void CBaseTurret::BeserkAimLogic(void) {

	//MODDD - random aim cycles will happen more often
	float randoVal = RANDOM_FLOAT(0, 1);
	//if (randoVal < 0.60)
	{

		//if (pev->deadflag != DEAD_DEAD) {
			m_vecGoalAngles.y = RANDOM_FLOAT(0, 360);
		//}
		//else {
		//	m_vecGoalAngles.y = m_vecGoalAngles.y + RANDOM_FLOAT(30, 42);
		//}

		//m_vecGoalAngles.y += RANDOM_FLOAT(-30, 30);
		//m_vecGoalAngles.y += 20;

		/*
		if (m_vecGoalAngles.y >= 0) {
			if (m_vecGoalAngles.y >= 180) {
				m_vecGoalAngles.y += -360;
			}
		}
		else {
			if (m_vecGoalAngles.y <= -180) {
				m_vecGoalAngles.y += 360;
			}
		}
		*/

		int pitchLow;
		int pitchHigh;

		//MODDD - range changed, aiming straight down is not very exciting, remove
		// extremely low random choices (range used to be 0 to 90 in the random)
		if (m_iOrientation == 0) {
			// floor
			pitchHigh = 48;
			pitchLow = 0;
		}
		else {
			// ceiling
			// 25, 85?
			pitchHigh = 86;
			pitchLow = 28;
		}
		m_vecGoalAngles.x = RANDOM_FLOAT(pitchLow, pitchHigh) - 90 * m_iOrientation;

		//MODDD - takes longer to be destroyed from this
		if (randoVal < 0.082) {
			TakeDamage(pev, pev, 1, DMG_GENERIC); // don't beserk forever
		}

		// Why???
		//return;
	}

}//END OF BeserkAimLogic


void CBaseTurret::ActiveThink(void)
{
	int fAttack = 0;
	Vector vecDeltaToEnemy;
	Vector vecDirToEnemy;

	pev->nextthink = gpGlobals->time + 0.1;
	StudioFrameAdvance_SIMPLE( );

	if ((!m_iOn) || (m_hEnemy == NULL))
	{
		m_hEnemy = NULL;
		m_flLastSight = gpGlobals->time + m_flMaxWait;
		SetThink(&CBaseTurret::SearchThink);
		return;
	}
	
	// if it's dead, look for something new
	// MODDD - don't count chumtoads playing dead!
	if ( !m_hEnemy->IsAlive_FromAI(this) )
	{
		if (!m_flLastSight)
		{
			m_flLastSight = gpGlobals->time + 0.5; // continue-shooting timeout
		}
		else
		{
			if (gpGlobals->time > m_flLastSight)
			{	
				m_hEnemy = NULL;
				m_flLastSight = gpGlobals->time + m_flMaxWait;
				SetThink(&CBaseTurret::SearchThink);
				return;
			}
		}
	}

	Vector vecMid = pev->origin + pev->view_ofs;


	Vector vecMidEnemy = m_hEnemy->BodyTargetMod( vecMid );
	//Vector vecMidEnemyRough = m_hEnemy->BodyTarget(vecMid);
	// no, use this to be filled instead.
	Vector vecMidEnemyRough;

	// Look for our current enemy
	//MODDD - NOTE!!!    OK SO.   FBoxVisible can modify the third parameter, vecMidEnemy.
	// It's existing value is not used by the method, so it would make more sense to send
	// it a blank vector to be filled instead.

	//int fEnemyVisible = FBoxVisible(pev, m_hEnemy->pev, vecMidEnemy );
	int fEnemyVisible = FBoxVisible(pev, m_hEnemy->pev, vecMidEnemyRough);

	vecDeltaToEnemy = vecMidEnemy - vecMid;	// calculate dir and dist to enemy
	vecDirToEnemy = vecDeltaToEnemy.Normalize();
	float flDistToEnemy = vecDeltaToEnemy.Length();

	Vector vec = UTIL_VecToAngles(vecDirToEnemy );

	//DebugLine_Setup(0, vecMid, pTesto, 255, 0, 255);

	// Current enmey is not visible.
	if (!fEnemyVisible || (flDistToEnemy > TURRET_RANGE))
	{
		if (!m_flLastSight)
			m_flLastSight = gpGlobals->time + 0.5;
		else
		{
			// Should we look for a new target?
			if (gpGlobals->time > m_flLastSight)
			{
				m_hEnemy = NULL;
				m_flLastSight = gpGlobals->time + m_flMaxWait;
				SetThink(&CBaseTurret::SearchThink);
				return;
			}
		}
		fEnemyVisible = 0;
	}
	else
	{
		m_vecLastSight = vecMidEnemy;
	}

	UTIL_MakeAimVectors(m_vecCurAngles);	

	/*
	ALERT( at_console, "%.0f %.0f : %.2f %.2f %.2f\n", 
		m_vecCurAngles.x, m_vecCurAngles.y,
		gpGlobals->v_forward.x, gpGlobals->v_forward.y, gpGlobals->v_forward.z );
	*/
	
	Vector vecLOS = vecDirToEnemy; //vecMid - m_vecLastSight;
	vecLOS = vecLOS.Normalize();

	float theDotProd = DotProduct(vecLOS, gpGlobals->v_forward);

	// Is the Gun looking at the target
	if (theDotProd <= 0.866) // 30 degree slop
		fAttack = FALSE;
	else
		fAttack = TRUE;


	//easyForcePrintLine("AH YEA %.2f", theDotProd);

	// fire the gun
	if (m_iSpin) {

		if (fAttack) {
			Vector vecSrc, vecAng;
			GetAttachment(0, vecSrc, vecAng);
			SetTurretAnim(TURRET_ANIM_FIRE);

			//MODDD - NEW.  At a high enough dot product, have even better aim.
			if (theDotProd > 0.993) {
				Shoot(vecSrc, (vecMidEnemyRough - vecSrc).Normalize() );
			}
			else {
				// default way (would miss small targets like chumtoads, even standing still)
				Shoot(vecSrc, gpGlobals->v_forward);
			}


			//Shoot(vecSrc, ((vecMidEnemy - vecMid).Normalize()));

			//DebugLine_Setup(0, vecSrc, vecMidEnemy, 0, 255, 0);
			//DebugLine_Setup(1, vecSrc, vecSrc + gpGlobals->v_forward * 800, 255, 0, 0);
			//DebugLine_Setup(1, vecSrc, vecSrc + ((vecMidEnemy - vecSrc).Normalize()) * 800, 255, 0, 0);
		}
		else if (m_fBeserk == 1) {
			Vector vecSrc, vecAng;
			GetAttachment(0, vecSrc, vecAng);
			SetTurretAnim(TURRET_ANIM_FIRE);
			Shoot(vecSrc, gpGlobals->v_forward);
		}
		else {
			// retail fallback still then.
			SetTurretAnim(TURRET_ANIM_SPIN);
		}
	}else{
		SetTurretAnim(TURRET_ANIM_SPIN);
	}


	// NOTICE - only things that call CBaseTurret TakeDamage logic (like CMiniTurret, the ceiling ones)
	// can trigger m_fBeserk.  Sentries skip that and never do.
	//move the gun
	if (m_fBeserk == 1)
	{
		BeserkAimLogic();

		// NOTICE - beserk logic might kill this turret.
		// SpinUpCall in the Turret class (not CBaseTurret...  yeah, that's not confusing)
		// will set the think to ActiveThink, ignoring the think set from TakeDamage.
		if(pev->deadflag != DEAD_NO){
			return;
		}
	} 
	else if (fEnemyVisible)
	{
		if (vec.y > 360)
			vec.y -= 360;

		if (vec.y < 0)
			vec.y += 360;

		//ALERT(at_console, "[%.2f]", vec.x);
		
		if (vec.x < -180)
			vec.x += 360;

		if (vec.x > 180)
			vec.x -= 360;

		// now all numbers should be in [1...360]
		// pin to turret limitations to [-90...15]

		if (m_iOrientation == 0)
		{
			if (vec.x > 90)
				vec.x = 90;
			else if (vec.x < m_iMinPitch)
				vec.x = m_iMinPitch;
		}
		else
		{
			if (vec.x < -90)
				vec.x = -90;
			else if (vec.x > -m_iMinPitch)
				vec.x = -m_iMinPitch;
		}

		// ALERT(at_console, "->[%.2f]\n", vec.x);

		m_vecGoalAngles.y = vec.y;
		m_vecGoalAngles.x = vec.x;

	}


	//MODDD - NEW.  Turn the eye brightness up if we have a target, but not to full in an instant.
	
	if(m_pEyeGlow && m_eyeBrightness < 255){
		m_eyeBrightness += 40;
		if (m_eyeBrightness > 255) {
			m_eyeBrightness = 255;
		}
		m_pEyeGlow->SetBrightness(m_eyeBrightness);
	}


	SpinUpCall();
	MoveTurret();
}


void CTurret::Shoot(Vector &vecSrc, Vector &vecDirToEnemy)
{
	FireBullets( 1, vecSrc, vecDirToEnemy, TURRET_SPREAD, TURRET_RANGE, BULLET_MONSTER_12MM, 1 );
	UTIL_PlaySound(ENT(pev), CHAN_WEAPON, "turret/tu_fire1.wav", 1, 0.6);
	pev->effects = pev->effects | EF_MUZZLEFLASH;
}


void CMiniTurret::Shoot(Vector &vecSrc, Vector &vecDirToEnemy)
{
	FireBullets( 1, vecSrc, vecDirToEnemy, TURRET_SPREAD, TURRET_RANGE, BULLET_MONSTER_9MM, 1 );

	switch(RANDOM_LONG(0,2))
	{
	case 0: UTIL_PlaySound(ENT(pev), CHAN_WEAPON, "weapons/hks1.wav", 1, ATTN_NORM, 0, 100, FALSE); break;
	case 1: UTIL_PlaySound(ENT(pev), CHAN_WEAPON, "weapons/hks2.wav", 1, ATTN_NORM, 0, 100, FALSE); break;
	case 2: UTIL_PlaySound(ENT(pev), CHAN_WEAPON, "weapons/hks3.wav", 1, ATTN_NORM, 0, 100, FALSE); break;
	}
	pev->effects = pev->effects | EF_MUZZLEFLASH;
}


void CBaseTurret::Deploy(void)
{
	pev->nextthink = gpGlobals->time + 0.1;
	StudioFrameAdvance_SIMPLE( );

	if (pev->sequence != TURRET_ANIM_DEPLOY)
	{
		m_iOn = 1;
		SetTurretAnim(TURRET_ANIM_DEPLOY);
		UTIL_PlaySound(ENT(pev), CHAN_BODY, "turret/tu_deploy.wav", TURRET_MACHINE_VOLUME, ATTN_NORM);
		SUB_UseTargets( this, USE_ON, 0 );
	}

	if (m_fSequenceFinished)
	{
		pev->maxs.z = m_iDeployHeight;
		pev->mins.z = -m_iDeployHeight;
		UTIL_SetSize(pev, pev->mins, pev->maxs);

		m_vecCurAngles.x = 0;

		if (m_iOrientation == 1)
		{
			m_vecCurAngles.y = UTIL_AngleMod( pev->angles.y + 180 );
		}
		else
		{
			m_vecCurAngles.y = UTIL_AngleMod( pev->angles.y );
		}

		SetTurretAnim(TURRET_ANIM_SPIN);
		pev->framerate = 0;
		SetThink(&CBaseTurret::SearchThink);
	}

	m_flLastSight = gpGlobals->time + m_flMaxWait;
}

void CBaseTurret::Retire(void)
{
	// make the turret level
	m_vecGoalAngles.x = 0;
	m_vecGoalAngles.y = m_flStartYaw;

	pev->nextthink = gpGlobals->time + 0.1;

	StudioFrameAdvance_SIMPLE( );

	EyeOff( );

	if (!MoveTurret())
	{
		if (m_iSpin)
		{
			SpinDownCall();
		}
		else if (pev->sequence != TURRET_ANIM_RETIRE)
		{
			SetTurretAnim(TURRET_ANIM_RETIRE);
			UTIL_PlaySound(ENT(pev), CHAN_BODY, "turret/tu_deploy.wav", TURRET_MACHINE_VOLUME, ATTN_NORM, 0, 120);
			SUB_UseTargets( this, USE_OFF, 0 );
		}
		else if (m_fSequenceFinished) 
		{	
			m_iOn = 0;
			m_flLastSight = 0;
			SetTurretAnim(TURRET_ANIM_NONE);
			pev->maxs.z = m_iRetractHeight;
			pev->mins.z = -m_iRetractHeight;
			UTIL_SetSize(pev, pev->mins, pev->maxs);
			if (m_iAutoStart)
			{
				SetThink(&CBaseTurret::AutoSearchThink);		
				pev->nextthink = gpGlobals->time + .1;
			}
			else
				SetThink(&CBaseEntity::SUB_DoNothing);
		}
	}
	else
	{
		SetTurretAnim(TURRET_ANIM_SPIN);
	}
}


void CTurret::SpinUpCall(void)
{
	StudioFrameAdvance_SIMPLE( );
	pev->nextthink = gpGlobals->time + 0.1;

	// Are we already spun up? If not start the two stage process.
	if (!m_iSpin)
	{
		SetTurretAnim( TURRET_ANIM_SPIN );
		// for the first pass, spin up the the barrel
		if (!m_iStartSpin)
		{
			pev->nextthink = gpGlobals->time + 1.0; // spinup delay
			UTIL_PlaySound(ENT(pev), CHAN_BODY, "turret/tu_spinup.wav", TURRET_MACHINE_VOLUME, ATTN_NORM);
			m_iStartSpin = 1;
			pev->framerate = 0.1;
		}
		// after the barrel is spun up, turn on the hum
		else if (pev->framerate >= 1.0)
		{
			pev->nextthink = gpGlobals->time + 0.1; // retarget delay
			UTIL_PlaySound(ENT(pev), CHAN_STATIC, "turret/tu_active2.wav", TURRET_MACHINE_VOLUME, ATTN_NORM);
			SetThink(&CBaseTurret::ActiveThink);
			m_iStartSpin = 0;
			m_iSpin = 1;
		} 
		else
		{
			pev->framerate += 0.075;
		}
	}

	if (m_iSpin)
	{
		SetThink(&CBaseTurret::ActiveThink);
	}
}


void CTurret::SpinDownCall(void)
{
	if (m_iSpin)
	{
		SetTurretAnim( TURRET_ANIM_SPIN );
		if (pev->framerate == 1.0)
		{
			UTIL_PlaySound(ENT(pev), CHAN_STATIC, "turret/tu_active2.wav", 0, 0, SND_STOP, 100);
			UTIL_PlaySound(ENT(pev), CHAN_ITEM, "turret/tu_spindown.wav", TURRET_MACHINE_VOLUME, ATTN_NORM);
		}
		pev->framerate -= 0.02;
		if (pev->framerate <= 0)
		{
			pev->framerate = 0;
			m_iSpin = 0;
		}
	}
}


void CBaseTurret::SetTurretAnim(TURRET_ANIM anim)
{
	if (pev->sequence != anim)
	{
		switch(anim)
		{
		case TURRET_ANIM_FIRE:
		case TURRET_ANIM_SPIN:
			if (pev->sequence != TURRET_ANIM_FIRE && pev->sequence != TURRET_ANIM_SPIN)
			{
				pev->frame = 0;
			}
			break;
		default:
			pev->frame = 0;
			break;
		}

		pev->sequence = anim;
		ResetSequenceInfo( );

		switch(anim)
		{
		case TURRET_ANIM_RETIRE:
			pev->frame			= 255;
			pev->framerate		= -1.0;
			break;
		case TURRET_ANIM_DIE:
			pev->framerate		= 1.0;
			break;
		}
		//ALERT(at_console, "Turret anim #%d\n", anim);
	}
}

//
// This search function will sit with the turret deployed and look for a new target. 
// After a set amount of time, the barrel will spin down. After m_flMaxWait, the turret will
// retact.
//
void CBaseTurret::SearchThink(void)
{
	// ensure rethink
	SetTurretAnim(TURRET_ANIM_SPIN);
	StudioFrameAdvance_SIMPLE( );
	pev->nextthink = gpGlobals->time + 0.1;

	if (m_flSpinUpTime == 0 && m_flMaxSpin)
		m_flSpinUpTime = gpGlobals->time + m_flMaxSpin;

	Ping( );

	// If we have a target and we're still healthy
	if (m_hEnemy != NULL)
	{
		//MODDD - don't count chumtoads playing dead!
		if (!m_hEnemy->IsAlive_FromAI(this))
			m_hEnemy = NULL;// Dead enemy forces a search for new one
	}


	// Acquire Target
	if (m_hEnemy == NULL)
	{
		Look(TURRET_RANGE);
		m_hEnemy = BestVisibleEnemy();
	}

	// If we've found a target, spin up the barrel and start to attack
	if (m_hEnemy != NULL)
	{
		m_flLastSight = 0;
		m_flSpinUpTime = 0;
		SetThink(&CBaseTurret::ActiveThink);
	}
	else
	{
		// Are we out of time, do we need to retract?
 		if (gpGlobals->time > m_flLastSight)
		{
			//Before we retrace, make sure that we are spun down.
			m_flLastSight = 0;
			m_flSpinUpTime = 0;
			SetThink(&CBaseTurret::Retire);
		}
		// should we stop the spin?
		else if ((m_flSpinUpTime) && (gpGlobals->time > m_flSpinUpTime))
		{
			SpinDownCall();
		}
		
		// generic hunt for new victims
		m_vecGoalAngles.y = (m_vecGoalAngles.y + 0.1 * m_fTurnRate);
		if (m_vecGoalAngles.y >= 360)
			m_vecGoalAngles.y -= 360;
		MoveTurret();
	}
}

// 
// This think function will deploy the turret when something comes into range. This is for
// automatically activated turrets.
//
void CBaseTurret::AutoSearchThink(void)
{
	// ensure rethink
	StudioFrameAdvance_SIMPLE( );
	pev->nextthink = gpGlobals->time + 0.3;

	// If we have a target and we're still healthy

	if (m_hEnemy != NULL)
	{
		//MODDD - don't count chumtoads plaing dead!
		if (!m_hEnemy->IsAlive_FromAI(this))
			m_hEnemy = NULL;// Dead enemy forces a search for new one
	}

	// Acquire Target

	if (m_hEnemy == NULL)
	{
		Look( TURRET_RANGE );
		m_hEnemy = BestVisibleEnemy();
	}

	if (m_hEnemy != NULL)
	{
		SetThink(&CBaseTurret::Deploy);
		UTIL_PlaySound(ENT(pev), CHAN_BODY, "turret/tu_alert.wav", TURRET_MACHINE_VOLUME, ATTN_NORM);
	}
}







// New starter for turret death.  Happens instantly instead of relying on a think-cycle to see the deadflag
// as DEADFLAG_NO, less chance of getting interrupted by a further think-set that happens before the flag
// even got set to DEADFLAG_DEAD.  Example:
//    TakeDamage(...)     <leaves health at or under 0>
//      TurretDeathCheck(...)    <sees turret is out of health, deadflag DEAD_NO, sets think to TurretDeathThink but does nothing to deadflag>
//    if(deadflag == DEAD_NO)    <still passes>
//      SetThink(Turret::ActiveThink)     <resumes ActiveThink without any deadflag set)
// This is avoided from calling DeathStart instantly, which can assume this is the first death frame, with the new post death
// sequence or not.
void CBaseTurret::DeathStart(void) {

	//if (pev->deadflag != DEAD_DEAD)
	//{
	pev->deadflag = DEAD_DEAD;


	if (m_iSpin && EASY_CVAR_GET(sv_turret_postdeath) == 2) {
		// if spinning at death, go haywire.
		postDeathEndTime = gpGlobals->time + RANDOM_FLOAT(6.5, 8.4);
		nextDeathExplosionTime = gpGlobals->time + RANDOM_FLOAT(0.70, 1.50);

		// And start doing random pings during that time.
		m_flPingTime = gpGlobals->time + RANDOM_FLOAT(0.3, 0.4);
	}
	else {
		// mundane death
		postDeathEndTime = -1;
		DeathEnd();
	}

	//MODDD - death sound moved to below too

	UTIL_PlaySound(ENT(pev), CHAN_STATIC, "turret/tu_active2.wav", 0, 0, SND_STOP, 100);

	//MODDD - several lines moved to the postDeathEndTimeReached check below

	//}//END OF deadflag != DEAD_DEAD check

}//DeathStart


// Start the TURRET_ANIM_DIE sequence now.  When that finishes playing (detected in TurretDeathThink think),
// the turret will really be done (stop thinking).
void CBaseTurret::DeathEnd(void) {

	// don't allow the turret turn speedups after this point of death,
	// they mess up coming back to a resting state
	// (special value: -1 to slow down turret speed)
	m_fBeserk = -1;

	if (m_iOrientation == 0)
		m_vecGoalAngles.x = -15;
	else
		m_vecGoalAngles.x = -90;

	SetTurretAnim(TURRET_ANIM_DIE);
	
	//MODDD - what?  why?
	//EyeOn();


	float flRndSound = RANDOM_FLOAT(0, 1);

	if (flRndSound <= 0.33)
		UTIL_PlaySound(ENT(pev), CHAN_BODY, "turret/tu_die.wav", 1.0, ATTN_NORM);
	else if (flRndSound <= 0.66)
		UTIL_PlaySound(ENT(pev), CHAN_BODY, "turret/tu_die2.wav", 1.0, ATTN_NORM);
	else
		UTIL_PlaySound(ENT(pev), CHAN_BODY, "turret/tu_die3.wav", 1.0, ATTN_NORM);


	// just in case.
	UTIL_PlaySound(ENT(pev), CHAN_STATIC, "turret/tu_active2.wav", 0, 0, SND_STOP, 100);

	Vector tempVec;
	tempVec.x = RANDOM_FLOAT(pev->absmin.x, pev->absmax.x);
	tempVec.y = RANDOM_FLOAT(pev->absmin.y, pev->absmax.y);
	tempVec.z = pev->origin.z - m_iOrientation * 64;
	// lots of smoke
	UTIL_Smoke(MSG_BROADCAST, NULL, NULL, tempVec, 0, 0, 0, g_sModelIndexSmoke, 25, 10 - m_iOrientation * 5);


	pev->takedamage = DAMAGE_YES;// don't let autoaim aim at corpses.

}//END OF DeathEnd


//MODDD - NOTE.  This gets called continuously after losing all health until the
// turret finally stops moving and the anim is finished.
// In fact this becomes the new think method, that's why.
// The pev->deadflag != DEAD_DEAD check sees if this is the first call though.
void CBaseTurret::TurretDeathThink( void )
{
	BOOL iActive = FALSE;
	BOOL postDeathEndTimeReached = FALSE;

	//MODDD - changed, was 2 and 5.
	const float lotsaSmokeTimeRandoMax = 3.5;
	const float smokeTimeRandoMax = 7;

	StudioFrameAdvance_SIMPLE( );

	// think faster!  More dramatic.
	pev->nextthink = gpGlobals->time + 0.052;


	//MODDD - some script moved to DeathStart

	if (gpGlobals->time >= postDeathEndTime) {
		postDeathEndTimeReached = TRUE;
	}


	if (postDeathEndTime != -1) {

		if (postDeathEndTimeReached) {
			postDeathEndTime = -1;
			DeathEnd();
		}
		else {

			if (m_pEyeGlow){
				float brightnessChange;
				if (m_eyeBrightness < 140) {
					brightnessChange = 48;
				}else {
					brightnessChange = 27;
				}
				// make the light go back and forth.
				if (postDeathEyeBrightnessDir < 0) {
					m_eyeBrightness -= brightnessChange;
					if (m_eyeBrightness < 0) {
						m_eyeBrightness = 0;
						postDeathEyeBrightnessDir = 1;
					}
				}else {
					m_eyeBrightness += brightnessChange;
					if (m_eyeBrightness > 255) {
						m_eyeBrightness = 255;
						postDeathEyeBrightnessDir = -1;
					}
				}
				m_pEyeGlow->SetBrightness(m_eyeBrightness);
			}//m_pEyeGlow check



			if (gpGlobals->time >= m_flPingTime) {
				m_flPingTime = gpGlobals->time + RANDOM_FLOAT(0.32, 0.38);
				// these pings go further.
				UTIL_PlaySound(ENT(pev), CHAN_STATIC, "turret/tu_ping.wav", 1, ATTN_NORM - 0.3, 0, RANDOM_LONG(103, 105));
			}

			// And start doing random pings during that time.

			Vector tempVec;
			tempVec.x = RANDOM_FLOAT(pev->absmin.x, pev->absmax.x);
			tempVec.y = RANDOM_FLOAT(pev->absmin.y, pev->absmax.y);
			tempVec.z = pev->origin.z - m_iOrientation * 64;

			if (gpGlobals->time >= nextDeathExplosionTime && nextDeathExplosionTime != -2) {

				Vector expVec;
				expVec.x = RANDOM_FLOAT(pev->origin.x + pev->mins.x * 1.5, pev->origin.x + pev->maxs.x * 1.5);
				expVec.y = RANDOM_FLOAT(pev->origin.y + pev->mins.y * 1.5, pev->origin.y + pev->maxs.y * 1.5);
				expVec.z = -m_iOrientation * 26 + RANDOM_FLOAT(pev->origin.z + pev->mins.z * 0.7, pev->origin.z + pev->maxs.z * 0.7);

				UTIL_Explosion(MSG_PVS, expVec, NULL, pev, expVec, 0, 0, 0, g_sModelIndexFireball, RANDOM_LONG(17, 28), 18, TE_EXPLFLAG_NONE, 0.4);
				//easyForcePrintLine("1 explosion, yes? %.2f", gpGlobals->time);
				// further than so man so many seconds away?  go ahead, do it again
				if (nextDeathExplosionTime < postDeathEndTime - 1.2) {
					nextDeathExplosionTime = gpGlobals->time + RANDOM_FLOAT(0.85, 1.15);
				}
				else {
					nextDeathExplosionTime = -2;
				}
			}

			// GO MAD FOR A BIT
			m_fBeserk = 1;

			Vector vecSrc, vecAng;
			GetAttachment(0, vecSrc, vecAng);
			SetTurretAnim(TURRET_ANIM_FIRE);

			UTIL_MakeAimVectors(m_vecCurAngles);

			Shoot(vecSrc, gpGlobals->v_forward);
			//BeserkAimLogic();

			//SpinUpCall();
			//MoveTurret();
		}
	}//postDeathEndTime != -1 check


	//MODDD- why was this here?  That makes the EyeOn call at initial killed above pointless?
	//EyeOff( );

	if (pev->dmgtime + RANDOM_FLOAT( 0, lotsaSmokeTimeRandoMax) > gpGlobals->time)
	{
		Vector tempVec;
		tempVec.x = RANDOM_FLOAT( pev->absmin.x, pev->absmax.x );
		tempVec.y = RANDOM_FLOAT( pev->absmin.y, pev->absmax.y );
		tempVec.z = pev->origin.z - m_iOrientation * 64;
		// lots of smoke
		UTIL_Smoke(MSG_BROADCAST, NULL, NULL, tempVec, 0, 0, 0, g_sModelIndexSmoke, 25, 10 - m_iOrientation * 5);
	}
	
	if (pev->dmgtime + RANDOM_FLOAT( 0, smokeTimeRandoMax ) > gpGlobals->time)
	{
		Vector vecSrc = Vector( RANDOM_FLOAT( pev->absmin.x, pev->absmax.x ), RANDOM_FLOAT( pev->absmin.y, pev->absmax.y ), 0 );
		if (m_iOrientation == 0)
			vecSrc = vecSrc + Vector( 0, 0, RANDOM_FLOAT( pev->origin.z, pev->absmax.z ) );
		else
			vecSrc = vecSrc + Vector( 0, 0, RANDOM_FLOAT( pev->absmin.z, pev->origin.z ) );

		//UTIL_Sparks( vecSrc );
		UTIL_Sparks( vecSrc, DEFAULT_SPARK_BALLS, EASY_CVAR_GET_DEBUGONLY(sparksTurretDeathMulti) );

	}

	//MODDD - moved outside the if-then below.  Conditions before that failing (timeReached, sequenceFinished)
	// would cause MoveTurret to never get called when it may have been the intention to.
	BOOL turretDoneMoving;
	
	if (!m_fSequenceFinished) {
		turretDoneMoving = !MoveTurret();
	}
	else {
		// force a stop if done animating.
		turretDoneMoving = TRUE;
	}

	//MODDD - postDeathEndTimeReached added.


	if (postDeathEndTimeReached) {
		int i;
		// This needs to be called every frame to reduce the brightness by 30 each time.
		// EyeOff does not instantly turn it off or lead to that happening alone.
		// GO.  FIGURE.
		EyeOff();

		if(EASY_CVAR_GET(sv_turret_postdeath) == 1 || EASY_CVAR_GET(sv_turret_postdeath) == 2){
			// sparks?
			for (i = 0; i < 2; i++) {
				Vector vecSrc = Vector(RANDOM_FLOAT(pev->origin.x + pev->mins.x * 0.9, pev->origin.x + pev->maxs.x * 0.9), RANDOM_FLOAT(pev->origin.y + pev->mins.y * 0.9, pev->origin.y + pev->maxs.y * 0.9), 0);
				if (m_iOrientation == 0)
					vecSrc.z = RANDOM_FLOAT(pev->origin.z, pev->absmax.z) + RANDOM_FLOAT(18, 26);
				else
					vecSrc.z = RANDOM_FLOAT(pev->absmin.z, pev->origin.z) + -RANDOM_FLOAT(18, 26);

				// 2 balls.   haha.
				UTIL_Sparks(vecSrc, 2, EASY_CVAR_GET_DEBUGONLY(sparksTurretDeathMulti) * 0.5);
			}
		}

	}

	//MODDD - condition removed, what was the point of this?    && pev->dmgtime + 5 < gpGlobals->time
	if (postDeathEndTimeReached && m_fSequenceFinished && turretDoneMoving)
	{
		int i;

		if(EASY_CVAR_GET(sv_turret_postdeath) == 1 || EASY_CVAR_GET(sv_turret_postdeath) == 2){
			Vector expVec;
			expVec.x = RANDOM_FLOAT(pev->origin.x + pev->mins.x * 0.9, pev->origin.x + pev->maxs.x * 0.9);
			expVec.y = RANDOM_FLOAT(pev->origin.y + pev->mins.y * 0.9, pev->origin.y + pev->maxs.y * 0.9);
			expVec.z = -m_iOrientation * 30 + RANDOM_FLOAT(pev->origin.z + pev->mins.z * 0.5, pev->origin.z + pev->maxs.z * 0.5);

		
			UTIL_Explosion(MSG_PVS, expVec, NULL, pev, expVec, 0, 0, 0, g_sModelIndexFireball, RANDOM_LONG(32, 37), 14, TE_EXPLFLAG_NONE, 0.7);
		}

		//EyeOff();

		pev->framerate = 0;
		SetThink( NULL );

		// remove eye.
		if (m_pEyeGlow) {
			// remove it.
			UTIL_Remove(m_pEyeGlow);
			m_pEyeGlow = NULL;
		}
	}


}// TurretDeathThink


GENERATE_TRACEATTACK_IMPLEMENTATION(CBaseTurret)
{
	if ( ptr->iHitgroup == HITGROUP_TURRET_ARMOR )
	{
		// hit armor
		if ( pev->dmgtime != gpGlobals->time || (RANDOM_LONG(0,10) < 1) )
		{
			UTIL_Ricochet( ptr->vecEndPos, RANDOM_FLOAT( 1, 2) );
			pev->dmgtime = gpGlobals->time;
		}

		flDamage = 0.1;// don't hurt the monster much, but allow bits_COND_LIGHT_DAMAGE to be generated
	}

	if ( !pev->takedamage )
		return;

	//MODDD NEW - can draw blood.
	if(useBloodEffect && CanMakeBloodParticles() ){
		//MODDD!!!!!!
		Vector vecBloodOrigin = ptr->vecEndPos - vecDir * 4;
		SpawnBlood(vecBloodOrigin, flDamage);// a little surface blood.   ...  oil.  yeah.
	}

	AddMultiDamage( pevAttacker, this, flDamage, bitsDamageType, bitsDamageTypeMod );

	if(EASY_CVAR_GET_DEBUGONLY(turretDamageDecal) != 0){
		//MODDD - also new, from base monster blood drawing for oil.
		TraceBleed( flDamage, vecDir, ptr, bitsDamageType, bitsDamageTypeMod );
	}

}//END OF TraceAttack



//This returns a boolean: whether to interrupt the (presumably) TakeDamage calling method. It should act on this and block script below to behave like the original
//when told to block. Returning TRUE (1) means pass, returning FALSE (0) means block.
BOOL CBaseTurret::TurretDeathCheck(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType, int bitsDamageTypeMod, void (CBaseTurret::*eventMethod)() ){


	/*
	if(!IsAlive())
	{
		//timed damages NOT allowed for corpses.
		bitsDamageType &= ~ DMG_TIMEBASED;
		bitsDamageTypeMod &= ~ DMG_TIMEBASEDMOD;

		return DeadTakeDamage( pevInflictor, pevAttacker, flDamage, bitsDamageType, bitsDamageTypeMod );
	}
	*/

	/*
		if ( bitsDamageType & DMG_ALWAYSGIB )
		{
			Killed( pevAttacker, GIB_ALWAYS );
		}
		else if ( bitsDamageType & DMG_NEVERGIB )
		{
			Killed( pevAttacker, GIB_NEVER );
		}
		else
		{
			Killed( pevAttacker, GIB_NORMAL );
		}
		*/


	if (pev->health <= 0)
	{

		/*
		DeadTakeDamage

		if	( ShouldGibMonster( iGib ) )
		{
			CallGibMonster(gibsSpawnDecals);
			return;
		}
		else if ( pev->flags & FL_MONSTER )
		{
			SetTouch( NULL );
			BecomeDead();
		}
		*/

		if(pev->deadflag == DEAD_NO){

			if(this->getGibCVar() <= 0){
				//retail behavior: invincible, collisionless corpse.
				//
				pev->health = pev->max_health / 2;
				pev->max_health = 5;
				

				// no, leave this up to the first frame of the TurretDeathThink think method.
				// If dead without even spinning, it skips to the normal death anim and
				// handles this.
				//if (postDeathEndTime != -1 && gpGlobals->time >= postDeathEndTime) {
				//	pev->takedamage = DAMAGE_YES;// don't let autoaim aim at corpses.
				//}

				//pev->health = 0;
				//pev->takedamage = DAMAGE_NO;
				//just refuse any calls in TAKE_DAMAGE calls form now on.

			}else{
				//anything else? Gibbable corpse.
				//pev->health = 60;

				int gibFlag = 0;
				if ( bitsDamageType & DMG_ALWAYSGIB )
				{
					//Killed( pevAttacker, GIB_ALWAYS );
					gibFlag = GIB_ALWAYS;
				}
				else if ( bitsDamageType & DMG_NEVERGIB )
				{
					gibFlag = GIB_NEVER;
				}
				else
				{
					gibFlag = GIB_NORMAL;
				}



				if	( ShouldGibMonster( gibFlag ) )
				{
					GibMonster(EASY_CVAR_GET_DEBUGONLY(turretGibDecal)!=0);
					return 0;
				}
				else if ( pev->flags & FL_MONSTER )
				{
					SetTouch( NULL );
					
					
					// MODDD - NOTICE!  If turrets were to be affected by the new corpse-toss (monsterKilledToss), it would
					// go here.  But eh.  Turrets are associated with being stationary anyway.
					// Sentries are on stands and could support it at least, but eh.  would need to rip some pieces out of
					// basemonster's BecomeDead and go here.  Heck just make that its own method and call it from here.
					// oh.
					// MODDD - TODO TODO TODO.    maybe.

					//BecomeDead();

					//if (postDeathEndTime != -1 && gpGlobals->time >= postDeathEndTime) {
					//	pev->takedamage = DAMAGE_YES;// don't let autoaim aim at corpses.
					//}

					// give the corpse half of the monster's original maximum health. 
					pev->health = pev->max_health / 2;
					pev->max_health = 5; // max_health now becomes a counter for how many blood decals the corpse can place.

					
				}
			}

			pev->dmgtime = gpGlobals->time;

			//MODDD - line commented out.  What was wrong with having a child class of CBaseMonster,
			// have the FL_MONSTER flag?  Just caused 'removeAllMonsters' not to work with these when dead, BAH.
			// Clearly this thing's still targetable by other AI whether it has FL_MONSTER or not so,... dunno.
			//ClearBits (pev->flags, FL_MONSTER); // why are they set in the first place???

			//MODDD - now first, was below "SetUse". Is that okay?
			SUB_UseTargets( this, USE_ON, 0 ); // wake up others

			SetUse(NULL);


			DeathStart();
			// Then, think continuously with the TurretDeathThink or SentryDeathThink method (whichever is sent)
			SetThink(eventMethod);

			//SetThink(&CBaseTurret::TurretDeathThink);

			pev->nextthink = gpGlobals->time + 0.1;
			

		}else{

			if (gpGlobals->time < postDeathEndTime) {
				// stop the post-death beserk then.
				postDeathEndTime = -1;
				DeathEnd();
			}
			if (pev->deadflag != DEAD_NO && this->getGibCVar() <= 0) {
				// don't proceed, corpse is still invulnerable, that counts gibbing.
				return 0;
			}

			// kill the corpse if enough damage was done to destroy the corpse and the damage is of a type that is allowed to destroy the corpse.
			if ( bitsDamageType & DMG_GIB_CORPSE )
			{
				if ( pev->health <= flDamage )
				{
					pev->health = -50;
					//Killed( pevAttacker, GIB_ALWAYS );
					GibMonster(EASY_CVAR_GET_DEBUGONLY(turretGibDecal)!=0 );
					return 0;
				}
				// Accumulate corpse gibbing damage, so you can gib with multiple hits
				pev->health -= flDamage * 0.1;
			}
		}

		return 0;
	}

	if(pev->deadflag != DEAD_NO){
		return 0; //force a block, nothing after me could be important.
	}

	return 1; //by default, don't block.
}



// take damage. bitsDamageType indicates type of damage sustained, ie: DMG_BULLET
GENERATE_TAKEDAMAGE_IMPLEMENTATION(CBaseTurret)
{
	if ( !pev->takedamage )
		return 0;



	// allow the post-death beserk time to be interrupted by taking enough damage to get to 0 health again
	if (pev->deadflag == DEAD_DEAD && gpGlobals->time < postDeathEndTime) {
		
	}
	else
	if(pev->deadflag != DEAD_NO && this->getGibCVar() <= 0){
		// if dead and the gib CVar is 0 (retail), this corpse is invlunerable.

		return 0;
	}

	if(pev->deadflag == DEAD_NO){
		if (!m_iOn) {
			flDamage /= 10.0;
		}
	}



	if(bitsDamageType & DMG_ENERGYBEAM){
		flDamage *= 0.65;
	}else if(bitsDamageTypeMod & DMG_PROJECTILE){
		flDamage *= 0.7;
	}else if(bitsDamageType & DMG_BLAST){
		flDamage *= 0.8;
	}else if(bitsDamageType & DMG_BULLET || bitsDamageTypeMod & DMG_GAUSS){
		flDamage *= 0.9;
	}

	//MODDD - bounded pev->health reduction with 'nothing hurts' CVar check
	//pev->health -= flDamage;

	///////////////////////////////////////////////////////////////////////////////////
	if (!ChangeHealthFiltered(pevAttacker, flDamage)) {
		// call to block the rest of the method if this returns FALSE. Sets g_rawDamageCumula already if that happens.
		return 0;
	}
	///////////////////////////////////////////////////////////////////////////////////

	//SetThink(&CBaseTurret::TurretDeathThink);
	if(!TurretDeathCheck(pevInflictor, pevAttacker, flDamage, bitsDamageType, bitsDamageTypeMod, static_cast <void (CBaseTurret::*)(void)>(&CBaseTurret::TurretDeathThink) ) )return 0; //this can block.


	if(pev->deadflag == DEAD_NO){
		//MODDD - condition changed, go beserk at a percentage of health
		//if (pev->health <= 10)
		//if(pev->health < pev->max_health*0.38)
		if (pev->health <= 15)
		{
			//MODDD
			// what?  The random check 'RANDOM_LONG(0, 0x7FFF) > 800' that used to be here was completly ignored.
			// Assuming that was the point, keeping it out.
			if (m_iOn)
			{
				m_fBeserk = 1;
				SetThink(&CBaseTurret::SearchThink);
			}
		}
	}
	
	return 1;
}

int CBaseTurret::MoveTurret(void)
{
	int state = 0;


	float finalTurnRate;

	if (m_fBeserk == 0) {
		finalTurnRate = m_fTurnRate;
	}
	else if(m_fBeserk == 1){
		// faster!
		if (pev->deadflag != DEAD_DEAD) {
			finalTurnRate = m_fTurnRate * 2.2;
		}
		else {
			// dead?  even crazier
			finalTurnRate = m_fTurnRate * 3.8;
		}
	}
	else {
		// final end animation?  go slower
		finalTurnRate = m_fTurnRate * 0.45;
	}

	if (finalTurnRate > 450) {
		// cap it!
		finalTurnRate = 450;
	}


	//MODDD - NEW.  When dead and in beserk mode, different behavior.
	if (pev->deadflag == DEAD_DEAD && m_fBeserk == 1) {
		
		// just move right
		//m_vecCurAngles.y += 9.5;
		m_vecCurAngles.y += RANDOM_FLOAT(0.8, 1) * 14.2;

		if (m_vecCurAngles.y < 0)
			m_vecCurAngles.y += 360;
		else if (m_vecCurAngles.y >= 360)
			m_vecCurAngles.y -= 360;

		//ALERT(at_console, "%.2f -> %.2f\n", m_vecCurAngles.y, y);
		state = 1;

		if (m_iOrientation == 0)
			SetBoneController(0, m_vecCurAngles.y - pev->angles.y);
		else
			SetBoneController(0, pev->angles.y - 180 - m_vecCurAngles.y);


		float highExtent;
		float lowExtent;
		const float slowPitchSpeed = 3.2;
		const float fastPitchSpeed = 7.5;

		if (m_iOrientation == 0) {
			// floor
			highExtent = 45;
			lowExtent = 1;
		}
		else {
			// ceiling
			highExtent = 84;
			lowExtent = 32;
		}

		if (postDeathBeserkDir > 0) {
			// ceiling: approaching sideways.
			// floor: approaching upwards.
			float daGoal = highExtent - 90 * m_iOrientation;
			float distToGoal = fabs(m_vecCurAngles.x - daGoal);
			//easyForcePrintLine("AAAA %.2f", distToGoal);
			// 7 * 2.2


			BOOL condition = distToGoal > (highExtent - lowExtent) - 19;
			if (m_iOrientation == 0) condition = !condition;

			if (condition) //|| distToGoal < 7
			{   
				// go faster
				m_vecCurAngles.x += RANDOM_FLOAT(0.7, 1) * fastPitchSpeed * postDeathBeserkDir;
			}
			else{
				m_vecCurAngles.x += RANDOM_FLOAT(0.7, 1) * slowPitchSpeed * postDeathBeserkDir;
			}

			if (m_vecCurAngles.x >= daGoal) {  //360
				// go the other way
				m_vecCurAngles.x = daGoal;
				postDeathBeserkDir = -1;
			}
		}
		else {
			// ceiling: straight down/up
			// floor: approaching sideways.   Go figure.
			float daGoal = lowExtent - 90 * m_iOrientation;
			float distToGoal = fabs(m_vecCurAngles.x - daGoal);
			//easyForcePrintLine("BBBB %.2f", distToGoal);

			BOOL condition = distToGoal < 19;
			if (m_iOrientation == 0) condition = !condition;

			if (condition)
			{
				// go faster
				m_vecCurAngles.x += RANDOM_FLOAT(0.7, 1) * fastPitchSpeed * postDeathBeserkDir;
			}
			else {
				m_vecCurAngles.x += RANDOM_FLOAT(0.7, 1) * slowPitchSpeed * postDeathBeserkDir;
			}


			if (m_vecCurAngles.x <= daGoal) {  //10
				// go the other way
				m_vecCurAngles.x = daGoal;
				postDeathBeserkDir = 1;
			}
		}

		if (m_iOrientation == 0)
			SetBoneController(1, -m_vecCurAngles.x);
		else
			SetBoneController(1, m_vecCurAngles.x);


	}
	else {
		// normal move behavior, get closer to m_vecGoalAngles.


		// any x movement?
		if (m_vecCurAngles.x != m_vecGoalAngles.x)
		{
			float flDir = m_vecGoalAngles.x > m_vecCurAngles.x ? 1 : -1;


			m_vecCurAngles.x += 0.1 * finalTurnRate * flDir;

			// if we started below the goal, and now we're past, peg to goal
			if (flDir == 1)
			{
				if (m_vecCurAngles.x > m_vecGoalAngles.x)
					m_vecCurAngles.x = m_vecGoalAngles.x;
			}
			else
			{
				if (m_vecCurAngles.x < m_vecGoalAngles.x)
					m_vecCurAngles.x = m_vecGoalAngles.x;
			}

			state = 1;

			if (m_iOrientation == 0)
				SetBoneController(1, -m_vecCurAngles.x);
			else
				SetBoneController(1, m_vecCurAngles.x);

		}


		if (m_vecCurAngles.y != m_vecGoalAngles.y)
		{
			float flDir = m_vecGoalAngles.y > m_vecCurAngles.y ? 1 : -1;
			float flDist = fabs(m_vecGoalAngles.y - m_vecCurAngles.y);


			if (flDist > 180)
			{
				flDist = 360 - flDist;
				flDir = -flDir;
			}
			if (flDist > 30)
			{
				if (m_fTurnRate < m_iBaseTurnRate * 10)
				{
					m_fTurnRate += m_iBaseTurnRate;
				}
			}
			else if (m_fTurnRate > 45)
			{
				m_fTurnRate -= m_iBaseTurnRate;
			}
			else
			{
				m_fTurnRate += m_iBaseTurnRate;
			}

			finalTurnRate = m_fTurnRate;  // let changes above apply now too.

			m_vecCurAngles.y += 0.1 * finalTurnRate * flDir;


			//MODDD - and there wasn't something like this here for y angles (was for x),
			// because?
			// oh wait, there is that flDist < 0.05 * turnRate thing below, maybe that works fine
			/*
			if (flDir == 1)
			{
				if (m_vecCurAngles.y > m_vecGoalAngles.y)
					m_vecCurAngles.y = m_vecGoalAngles.y;
			}
			else
			{
				if (m_vecCurAngles.y < m_vecGoalAngles.y)
					m_vecCurAngles.y = m_vecGoalAngles.y;
			}
			*/


			if (m_vecCurAngles.y < 0)
				m_vecCurAngles.y += 360;
			else if (m_vecCurAngles.y >= 360)
				m_vecCurAngles.y -= 360;


			if (flDist < (0.05 * m_iBaseTurnRate))
				m_vecCurAngles.y = m_vecGoalAngles.y;


			//ALERT(at_console, "%.2f -> %.2f\n", m_vecCurAngles.y, y);
			state = 1;


			if (m_iOrientation == 0)
				SetBoneController(0, m_vecCurAngles.y - pev->angles.y);
			else
				SetBoneController(0, pev->angles.y - 180 - m_vecCurAngles.y);

		}
	}

	if (!state)
		m_fTurnRate = m_iBaseTurnRate;

	//ALERT(at_console, "(%.2f, %.2f)->(%.2f, %.2f)\n", m_vecCurAngles.x, 
	//	m_vecCurAngles.y, m_vecGoalAngles.x, m_vecGoalAngles.y);
	return state;
}

//
// ID as a machine
//
int CBaseTurret::Classify ( void )
{
	if (m_iOn || m_iAutoStart)
		return	CLASS_MACHINE;
	return CLASS_NONE;
}
BOOL CBaseTurret::isOrganic(void){
	return FALSE;
}
BOOL CBaseTurret::isOrganicLogic(void){
	//doesn't hurt to be safe.  Never treat me as organic in any way.
	return FALSE;
}

GENERATE_GIBMONSTER_IMPLEMENTATION(CBaseTurret){
	TraceResult	tr;
	BOOL gibbed = FALSE;
	
	if(getGibCVar() == 0){
		//no behavior allowed.
		return;
	}

	//MODDD - perhaps some generic mechanical crashing sound instead?
	//UTIL_PlaySound(ENT(pev), CHAN_WEAPON, "common/bodysplat.wav", 1, ATTN_NORM, TRUE);
	//


	if(getGibInfoRef()!=NULL){
		easyForcePrintLine("gib model: %s", getGibInfoRef()->modelPath);
	}else{
		easyForcePrintLine("Im about to crash from a missing gibInfoRef yaaaaay");
	}

	if( isStringEmpty(getGibInfoRef()->modelPath)){
		//no gib model to work with. Don't bother.

		if(getGibCVar() == 1){
			//set to fade, leave gibbing off.
		}else{
			//delete instantly regardless.
			gibbed = TRUE;
		}

	}else{
		//make some gibs.
		gibbed = TRUE;
		//TODO - should nubmer of gibs spawned depend on the type of turret destroyed? sentry, turret, miniturret?
		CGib::SpawnRandomGibs( pev, 8, *getGibInfoRef(), fGibSpawnsDecal );	// Throw gibs
	}

	
	pev->takedamage = DAMAGE_NO;
	pev->solid = SOLID_NOT;
	pev->deadflag = DEAD_DEAD;
	FCheckAITrigger();
	
	//GibMonster(DetermineGibHeadBlock(), gibsSpawnDecals);

	// don't let the status bar glitch for players.with <0 health.
	if (pev->health < -99)
	{
		pev->health = 0;
	}
	
	//if ( ShouldFadeOnDeath() && !fade )
	//	UTIL_Remove(this);

	//of course.
	UTIL_playMetalGibSound(pev);

	if ( gibbed ){
		// don't remove players!
		SetThink ( &CBaseEntity::SUB_Remove );
		pev->nextthink = gpGlobals->time;
	}
	else{
		FadeMonster();
	}

}//END OF GibMonster


// Can I make blood particles?  Leaving this up to a CVar for turrets.
BOOL CBaseTurret::CanMakeBloodParticles(void){
	return EASY_CVAR_GET_DEBUGONLY(turretBleedsOil) != 0;
}












//////////////////////////////////////////////////////////////////////////////////////////////////////
//CSentry


GibInfo_t* CSentry::gibModelRef = NULL;

#if REMOVE_ORIGINAL_NAMES != 1
	LINK_ENTITY_TO_CLASS( monster_sentry, CSentry );
#endif

#if EXTRA_NAMES > 0
	LINK_ENTITY_TO_CLASS( sentry, CSentry );
	
	//no extras.

#endif




void CSentry::Precache()
{
	CBaseTurret::Precache( );
	PRECACHE_MODEL ("models/sentry.mdl");

	if(gibModelRef == NULL)gibModelRef = getTurretGibInfo(getGibCVar());
	if(!isStringEmpty(gibModelRef->modelPath)){
		PRECACHE_MODEL((char*)gibModelRef->modelPath);
	}
}

float CSentry::getGibCVar(){
	return EASY_CVAR_GET_DEBUGONLY(sentryCanGib);
}


CSentry::CSentry(void) {
	nextTouchCooldown = -1;
}

void CSentry::Spawn()
{ 
	Precache( );
	SET_MODEL(ENT(pev), "models/sentry.mdl");

	pev->health			= gSkillData.sentryHealth;
	m_HackedGunPos		= Vector( 0, 0, 48 );
	pev->view_ofs.z		= 48;
	m_flMaxWait = SENTRY_MAXWAIT;
	m_flMaxSpin	= SENTRY_MAXSPIN;

	CBaseTurret::Spawn();
	//NOTE - this includes the monsterID giving call


	pev->movetype = MOVETYPE_FLY;
	pev->solid = SOLID_SLIDEBOX;
	pev->fuser1 = 8;

	pev->classname = MAKE_STRING("monster_sentry");
	
	m_iRetractHeight = 64;

	m_iDeployHeight = 64;
	m_iMinPitch	= -60;

	//MODDD - doing that after the 'DROP_TO_FLOOR' call soon.
	//UTIL_SetSize(pev, Vector(-16, -16, -m_iRetractHeight), Vector(16, 16, m_iRetractHeight));
	UTIL_SetSize(pev, Vector(-16, -16, 0), Vector(16, 16, m_iRetractHeight));
	


	//MODDD - NOT YET FREEMAN.
	// Something about touch being set this early has a rare chance of causing issues.
	// If the sentry has a "Touch" event, it could tall "TakeDamage" and set the Think method before
	// the intended one below has a chance to be called, so we get a Sentry that hasn't been setup
	// trying to do combat.  Has a gun rotate speed of 0.  Not a pretty sight.
	//SetTouch(&CSentry::SentryTouch);
	
	//MODDD - Not yet, Freeman!  Call "PostInit", like a very tiny "StartMonster".
	//SetThink(&CBaseTurret::Initialize);	
	//pev->nextthink = gpGlobals->time + 0.3; 

	SetThink(&CSentry::PreInit);
	pev->nextthink = gpGlobals->time + 0.1; 	
}
void CSentry::PreInit(void) {

	//MODDD - copied over from StartMonster in basemonster.cpp.

	
	// Snap to the ground, this turret does look like a small turret on stands after all.
	if (!FBitSet(pev->spawnflags, SF_MONSTER_FALL_TO_GROUND))
	{
		if (EASY_CVAR_GET_DEBUGONLY(crazyMonsterPrintouts))easyForcePrintLine("YOU amazing piece of work");

		//pev->solid = SOLID_SLIDEBOX;

		//int oldMoveType = pev->movetype;
		//pev->movetype = MOVETYPE_STEP;
		pev->origin.z += 1;
		DROP_TO_FLOOR(ENT(pev));
		//UTIL_SetOrigin(pev, pev->origin);
		//pev->movetype = oldMoveType;

		// and finally apply the size intended.  Unsure why we have non-zero min Z, but ok.
		UTIL_SetSize(pev, Vector(-16, -16, -m_iRetractHeight), Vector(16, 16, m_iRetractHeight));
	}
	else
	{
		pev->flags &= ~FL_ONGROUND;
	}
	

	// now go do the intended init.
	SetThink(&CBaseTurret::Initialize);	
	pev->nextthink = gpGlobals->time + 0.2; 
}
// Called by CBaseTurret::Initialize.
void CSentry::PostInit(void) {
	// OK FREEMAN JEEZ.
	SetTouch(&CSentry::SentryTouch);
}


void CSentry::Shoot(Vector &vecSrc, Vector &vecDirToEnemy)
{

	FireBullets( 1, vecSrc, vecDirToEnemy, TURRET_SPREAD, TURRET_RANGE, BULLET_MONSTER_MP5, 1 );
	
	switch(RANDOM_LONG(0,2))
	{
	case 0: UTIL_PlaySound(ENT(pev), CHAN_WEAPON, "weapons/hks1.wav", 1, ATTN_NORM, 0, 100, FALSE); break;
	case 1: UTIL_PlaySound(ENT(pev), CHAN_WEAPON, "weapons/hks2.wav", 1, ATTN_NORM, 0, 100, FALSE); break;
	case 2: UTIL_PlaySound(ENT(pev), CHAN_WEAPON, "weapons/hks3.wav", 1, ATTN_NORM, 0, 100, FALSE); break;
	}
	pev->effects = pev->effects | EF_MUZZLEFLASH;
}


GENERATE_TRACEATTACK_IMPLEMENTATION(CSentry)
{
	GENERATE_TRACEATTACK_PARENT_CALL(CBaseTurret);
}




GENERATE_TAKEDAMAGE_IMPLEMENTATION(CSentry)
{
	if ( !pev->takedamage )
		return 0;

	
	if(pev->deadflag != DEAD_NO && this->getGibCVar() <= 0){
		//if dead and the gib CVar is 0 (retail), this corpse is invlunerable.
		return 0;
	}

	if(pev->deadflag == DEAD_NO){
		if (!m_iOn)
		{
			SetThink( &CBaseTurret::Deploy );
			SetUse( NULL );
			pev->nextthink = gpGlobals->time + 0.1;
		}
	}



	if(bitsDamageType & DMG_ENERGYBEAM){
		flDamage *= 0.77;
	}else if(bitsDamageTypeMod & DMG_PROJECTILE){
		// less reduction for xbow bolts, much less armor in the way
		flDamage *= 0.90;
	}else if(bitsDamageType & DMG_BLAST){
		flDamage *= 0.87;
	}else if(bitsDamageType & DMG_BULLET || bitsDamageTypeMod & DMG_GAUSS){
		flDamage *= 0.92;
	}



	//MODDD - bounded pev->health reduction with 'nothing hurts' CVar check
	//pev->health -= flDamage;
	///////////////////////////////////////////////////////////////////////////////////
	if (!ChangeHealthFiltered(pevAttacker, flDamage)) {
		// call to block the rest of the method if this returns FALSE. Sets g_rawDamageCumula already if that happens.
		return 0;
	}
	///////////////////////////////////////////////////////////////////////////////////




	//void (CBaseTurret::*eventMethod)()
	if(!TurretDeathCheck(pevInflictor, pevAttacker, flDamage, bitsDamageType, bitsDamageTypeMod, static_cast <void (CBaseTurret::*)(void)>(&CSentry::SentryDeathThink) ) )return 0; //this can block.


	return 1;
}


void CSentry::SentryTouch( CBaseEntity *pOther )
{
	if (gpGlobals->time >= nextTouchCooldown) {
		if (pOther && (pOther->IsPlayer() || (pOther->pev->flags & FL_MONSTER)))
		{
			TakeDamage(pOther->pev, pOther->pev, 0, 0);
		}
		nextTouchCooldown = gpGlobals->time + 1;
	}
}



void CSentry::DeathStart(void) {

	//if (pev->deadflag != DEAD_DEAD)
	//{
		pev->deadflag = DEAD_DEAD;

		float flRndSound = RANDOM_FLOAT(0, 1);

		if (flRndSound <= 0.33)
			UTIL_PlaySound(ENT(pev), CHAN_BODY, "turret/tu_die.wav", 1.0, ATTN_NORM);
		else if (flRndSound <= 0.66)
			UTIL_PlaySound(ENT(pev), CHAN_BODY, "turret/tu_die2.wav", 1.0, ATTN_NORM);
		else
			UTIL_PlaySound(ENT(pev), CHAN_BODY, "turret/tu_die3.wav", 1.0, ATTN_NORM);

		UTIL_PlaySound(ENT(pev), CHAN_STATIC, "turret/tu_active2.wav", 0, 0, SND_STOP, 100);

	//}

	// String it together, no post-death sequence for you.
	DeathEnd();

}//DeathStart


void CSentry::DeathEnd(void) {

	//MODDD - Is this necessary?  Snaps the turret back to its default place.  Disabled the Y one at least (other is X; pitch).
	//SetBoneController(0, 0);
	SetBoneController(1, 0);

	SetTurretAnim(TURRET_ANIM_DIE);

	if (this->getGibCVar() <= 0) {
		pev->solid = SOLID_NOT;
	}
	else {
		//we are still kill-able.   ...wait why does this make me fall through the world? WTF?
		//setPhysicalHitboxForDeath();

		//is that okay??
		UTIL_SetSize(pev, Vector(pev->mins.x, pev->mins.y, pev->mins.z), Vector(pev->maxs.x, pev->maxs.y, pev->mins.z + 0.2));
	}

	pev->angles.y = UTIL_AngleMod(pev->angles.y + RANDOM_LONG(0, 2) * 120);

	// ???  why?
	//EyeOn();
}//DeathEnd



void CSentry::SentryDeathThink( void )
{
	BOOL iActive = FALSE;

	StudioFrameAdvance_SIMPLE( );
	pev->nextthink = gpGlobals->time + 0.1;

	//MODDD - some script moved to DeathStart

	EyeOff( );

	Vector vecSrc, vecAng;
	GetAttachment( 1, vecSrc, vecAng );

	if (pev->dmgtime + RANDOM_FLOAT( 0, 2 ) > gpGlobals->time)
	{
		// lots of smoke
		UTIL_Smoke(MSG_BROADCAST, NULL, NULL, vecSrc, RANDOM_FLOAT( -16, 16 ), RANDOM_FLOAT( -16, 16 ), -32, g_sModelIndexSmoke, 15, 8);
	}
	
	if (pev->dmgtime + RANDOM_FLOAT( 0, 8 ) > gpGlobals->time)
	{
		//UTIL_Sparks(...)
		UTIL_Sparks( vecSrc, DEFAULT_SPARK_BALLS, EASY_CVAR_GET_DEBUGONLY(sparksTurretDeathMulti) );
	}

	if (m_fSequenceFinished && pev->dmgtime + 5 < gpGlobals->time)
	{
		pev->framerate = 0;
		SetThink( NULL );
	}
}

