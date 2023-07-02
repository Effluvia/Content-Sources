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
==========================
This file contains "stubs" of class member implementations so that we can predict certain
 weapons client side.  From time to time you might find that you need to implement part of the
 these functions.  If so, cut it from here, paste it in hl_weapons.cpp or somewhere else and
 add in the functionality you need.
==========================
*/
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "nodes.h"
#include "soundent.h"
#include "skill.h"

//MODDD - new
#include "chumtoadweapon.h"



// Globals used by game logic
//MODDD - g_vecZero moved to util_shared.h/.cpp.

int gmsgWeapPickup = 0;
enginefuncs_t g_engfuncs;

//MODDD - used to be an important point here.
//globalvars_t  *gpGlobals;


//MODDD - removed here, since this (and CBasePlayerItem::AmmoInfoArray) are in util_shared.cpp now instead.
//ItemInfo CBasePlayerItem::ItemInfoArray[MAX_WEAPONS];


//MODDD - is that okay?  Just putting these new method-stubs here since the EMIT_SOUND_DYN was found here as-is.

void UTIL_PlaySound(entvars_t* entity, int channel, const char *pszName, float volume, float attenuation ){}
void UTIL_PlaySound(entvars_t* entity, int channel, const char *pszName, float volume, float attenuation, BOOL useSoundSentenceSave ){}
void UTIL_PlaySound(edict_t* entity, int channel, const char *pszName, float volume, float attenuation ){}
void UTIL_PlaySound(edict_t* entity, int channel, const char *pszName, float volume, float attenuation, BOOL useSoundSentenceSave  ){}
void UTIL_PlaySound(entvars_t* entity, int channel, const char *pszName, float volume, float attenuation, int flags, int pitch ){}
void UTIL_PlaySound(entvars_t* entity, int channel, const char *pszName, float volume, float attenuation, int flags, int pitch, BOOL useSoundSentenceSave ){}
void UTIL_PlaySound(edict_t* entity, int channel, const char *pszName, float volume, float attenuation, int flags, int pitch ){}
void UTIL_PlaySound(edict_t* entity, int channel, const char *pszName, float volume, float attenuation, int flags, int pitch, BOOL useSoundSentenceSave ){}


void EMIT_SOUND_DYN(edict_t *entity, int channel, const char *sample, float volume, float attenuation, int flags, int pitch) { }




//MODDD - also here dummied just in case.
void UTIL_EmitAmbientSound( entvars_t *entity, const Vector &vecOrigin, const char *samp, float vol, float attenuation ){}
void UTIL_EmitAmbientSound( entvars_t *entity, const Vector &vecOrigin, const char *samp, float vol, float attenuation, BOOL useSoundSentenceSave ){}
void UTIL_EmitAmbientSound( entvars_t *entity, const Vector &vecOrigin, const char *samp, float vol, float attenuation, int fFlags, int pitch ){}
void UTIL_EmitAmbientSound( entvars_t *entity, const Vector &vecOrigin, const char *samp, float vol, float attenuation, int fFlags, int pitch, BOOL useSoundSentenceSave ){}
void UTIL_EmitAmbientSound( edict_t *entity, const Vector &vecOrigin, const char *samp, float vol, float attenuation, int fFlags, int pitch ){}
void UTIL_EmitAmbientSound( edict_t *entity, const Vector &vecOrigin, const char *samp, float vol, float attenuation, int fFlags, int pitch, BOOL useSoundSentenceSave ){}
void EMIT_AMBIENT_SOUND( edict_t *entity, const Vector &vecOrigin, const char *samp, float vol, float attenuation, int fFlags, int pitch ){}







// CBaseEntity Stubs
int CBaseEntity::TakeHealth( float flHealth, int bitsDamageType ) { return 1; }

void CBaseEntity::ReportGeneric(){}

BOOL CBaseEntity::blocksImpact(void){return FALSE;}
float CBaseEntity::massInfluence(void){return 0;}
int CBaseEntity::GetProjectileType(void){return 0;}

Vector CBaseEntity::GetVelocityLogical(void){return Vector(); }   //blank vector?  most unorthodox.
void CBaseEntity::SetVelocityLogical(const Vector& arg_newVelocity){}

void CBaseEntity::OnDeflected(CBaseEntity* arg_entDeflector){}


//Yes, player has this too.
BOOL CBasePlayer::blocksImpact(void){return FALSE;}
float CGrenade::massInfluence(void){return 0;}
int CGrenade::GetProjectileType(void){return 0;}

//MODDD
BOOL CBaseEntity::isMovetypeFlying(void){return FALSE;}
BOOL CBaseEntity::isSizeGiant(void){return FALSE;}
BOOL CBaseEntity::isOrganic(void){return FALSE;}
int CBaseEntity::getHullIndexForNodes(void){return 0;}
int CBaseEntity::getHullIndexForGroundNodes(void) {return 0;}
int CBaseEntity::getNodeTypeAllowed(void) {return 0;}

//MODDD - moved from CBaseAnimating, serverside territority, to CBaseEntity, shared, so
// these have to be dummied now.
int CBaseEntity::getNumberOfBodyParts(void){return 0;}
int CBaseEntity::getNumberOfSkins(void){return 0;}

void CBaseEntity::onDelete(void){}


void CBasePlayer::OnFirstAppearance(void){}

void CBaseEntity::Spawn(void){}
//MODDD
BOOL CBaseEntity::usesSoundSentenceSave(void){return FALSE;}
CBaseEntity::CBaseEntity(void){}

//defined now.
void CBaseMonster::Activate(void){}
void CBaseMonster::Spawn(void){}

CBaseEntity* CBaseMonster::getNearestDeadBody(const Vector& arg_searchOrigin, const float arg_maxDist){return NULL;}
BOOL CBaseMonster::noncombat_Look_ignores_PVS_check(void){return FALSE;}
BOOL CBaseMonster::bypassAllowMonstersSpawnCheck(void) { return FALSE; }

BOOL CBaseMonster::violentDeathAllowed(void){return FALSE;}
BOOL CBaseMonster::violentDeathDamageRequirement(void){return FALSE;}
BOOL CBaseMonster::violentDeathClear(void){return FALSE;}
int CBaseMonster::violentDeathPriority(void){return 0;}

BOOL CBaseMonster::violentDeathClear_BackwardsCheck(float argDistance){return FALSE;}

BOOL CBaseMonster::canPredictActRepeat(void){return FALSE;}

CBaseEntity* CBaseMonster::HumanKick(void){return NULL;}
CBaseEntity* CBaseMonster::HumanKick(float argCheckDistance){return NULL;}


void CBaseMonster::precacheStandardMeleeAttackMissSounds(void){}
void CBaseMonster::precacheStandardMeleeAttackHitSounds(void){}
void CBaseMonster::playStandardMeleeAttackMissSound(void){}
void CBaseMonster::playStandardMeleeAttackHitSound(void){}
void CBaseMonster::playStandardMeleeAttackMetalHitSound(void){}
void CBaseMonster::playStandardMeleeAttackHitSound(CBaseEntity* hitEnt, const char** normalHitSounds, int normalHitSoundsSize, float vol, float attn, int pitchMin, int pitchMax){}

void CBaseMonster::determineStandardMeleeAttackHitSound(CBaseEntity* hitEnt){}






BOOL CBaseMonster::traceResultObstructionValidForAttack(const TraceResult& arg_tr){return FALSE;}

float CBaseMonster::getDistTooFar(void){return 0.0f;}
float CBaseMonster::getDistLook(void){return 0.0f;}



BOOL CBaseMonster::getGermanModelRequirement(void){return FALSE;}
const char* CBaseMonster::getGermanModel(void){return NULL;}
const char* CBaseMonster::getNormalModel(void){return NULL;}


void CBaseEntity::DefaultSpawnNotice(void){return;}
void CBaseEntity::ForceSpawnFlag(int arg_spawnFlag){return;}
BOOL CBaseEntity::isTalkMonster(void){return FALSE;}
BOOL CBaseEntity::isProvokable(void){return FALSE;}
BOOL CBaseEntity::isProvoked(void){return FALSE;}


BOOL CBaseEntity::isBreakableOrChild(void){return FALSE;}
BOOL CBaseEntity::isDestructibleInanimate(void){ return FALSE;}

const char* CBaseEntity::getClassname(void){return NULL;}




//This is a new method from UTIL.cpp, serverside, that is referred to in some weapons.
Vector UTIL_GetProjectileVelocityExtra(Vector& playerVelocity, float velocityMode){ return Vector(0,0,0); }



//void CTalkMonster::playPissed(){}


//For an egon test (from the old source code).
void UTIL_ScreenShake( const Vector &center, float amplitude, float frequency, float duration, float radius ){}


CBaseEntity *CBaseEntity::GetNextTarget( void ) { return NULL; }
int CBaseEntity::Save( CSave &save ) { return 1; }
int CBaseEntity::Restore( CRestore &restore ) { return 1; }
void CBaseEntity::PostRestore(){}

void CBaseEntity::playMetallicHitSound(int arg_channel, float arg_volume){}

BOOL CBaseEntity::IsWorld(){return FALSE;}
BOOL CBaseEntity::IsWorldAffiliated(){return FALSE;}
BOOL CBaseEntity::IsWorldOrAffiliated(){return FALSE;}

BOOL CBaseEntity::IsBreakable(){return FALSE;}

//MODDD - new
BOOL CBaseEntity::isForceHated(CBaseEntity *pBy){return FALSE;}
int CBaseEntity::forcedRelationshipWith(CBaseEntity *pWith){return 0;}

void CBaseEntity::SetObjectCollisionBox( void ) { }
void CBaseEntity::setModel(void){}
void CBaseEntity::setModel(const char* m){}

int CBaseEntity::Intersects( CBaseEntity *pOther ) { return 0; }
void CBaseEntity::MakeDormant( void ) { }
int CBaseEntity::IsDormant( void ) { return 0; }
BOOL CBaseEntity::IsInWorld( void ) { return TRUE; }
int CBaseEntity::ShouldToggle( USE_TYPE useType, BOOL currentState ) { return 0; }
int CBaseEntity::DamageDecal( int bitsDamageType ) { return -1; }
int CBaseEntity::DamageDecal( int bitsDamageType, int bitsDamageTypeMod ) { return -1; }

edict_t* CBaseEntity::overyLongComplicatedProcessForCreatingAnEntity(const char* entityName){ return NULL; }
CBaseEntity* CBaseEntity::CreateManual( const char *szName, const Vector &vecOrigin, const Vector &vecAngles, edict_t *pentOwner ) { return NULL; }
CBaseEntity* CBaseEntity::Create( const char *szName, const Vector &vecOrigin, const Vector &vecAngles, edict_t *pentOwner ) { return NULL; }
CBaseEntity* CBaseEntity::Create( const char *szName, const Vector &vecOrigin, const Vector &vecAngles, int setSpawnflags, edict_t *pentOwner ) { return NULL; }


void CBaseEntity::SUB_Remove( void ) { }

//MODDDMIRROR
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBaseEntity::SetNextThink( float delay, BOOL correctSpeed ) { }//LRC
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CBaseEntity::playAmmoPickupSound(){}
void CBaseEntity::playAmmoPickupSound(entvars_t* sentPev){}
void CBaseEntity::playGunPickupSound(){}
void CBaseEntity::playGunPickupSound(entvars_t* sentPev){}

void CBaseEntity::precacheAmmoPickupSound(){}
void CBaseEntity::precacheGunPickupSound(){}

void CBaseEntity::SpawnBlood(const Vector& vecSpot, float flDamage) {}
void CBaseEntity::SpawnBloodSlash(float flDamage, const Vector& vecDrawLoc, const Vector& vecTraceLine) {}
void CBaseEntity::SpawnBloodSlash(float flDamage, const Vector& vecDrawLoc, const Vector& vecTraceLine, const BOOL& extraBlood) {}


void UTIL_PRECACHESOUND(char* path){}


GENERATE_TRACEATTACK_IMPLEMENTATION_DUMMY_CLIENT(CBaseEntity)
void CBaseEntity::TraceAttack_Traceless(entvars_t* pevAttacker, float flDamage, Vector vecDir, int bitsDamageType, int bitsDamageTypeMod) {}
GENERATE_TAKEDAMAGE_IMPLEMENTATION_DUMMY_CLIENT(CBaseEntity)
void CBaseEntity::Knockback(const int knockbackAmount, const Vector& knockbackDir) {}
BOOL CBaseEntity::ChangeHealthFiltered(entvars_t* pevAttacker, float flDamage) { return FALSE; }
float CBaseMonster::hitgroupDamage(float flDamage, int bitsDamageType, int bitsDamageTypeMod, int iHitgroup){return FALSE;}


// CBaseDelay Stubs
void CBaseDelay::KeyValue( struct KeyValueData_s * ) { }
int CBaseDelay::Restore( class CRestore & ) { return 1; }
int CBaseDelay::Save( class CSave & ) { return 1; }
void CBaseDelay::PostRestore(void){}

// CBaseAnimating Stubs
int CBaseAnimating::Restore( class CRestore & ) { return 1; }
int CBaseAnimating::Save( class CSave & ) { return 1; }

// DEBUG Stubs
//The dummied version will be in util_entity.cpp, serverside, instead for consistency.
//edict_t *DBG_EntOfVars( const entvars_t *pev ) { return NULL; }
void DBG_AssertFunction(BOOL fExpr,	const char*	szExpr,	const char*	szFile,	int szLine,	const char*	szMessage) { }

// UTIL_* Stubs
void UTIL_PrecacheOther( const char *szClassname ) { }
void UTIL_BloodDrips( const Vector &origin, const Vector &direction, int color, int amount ) { }
void UTIL_DecalTrace( TraceResult *pTrace, int decalNumber ) { }
void UTIL_GunshotDecalTrace( TraceResult *pTrace, int decalNumber ) { }
void UTIL_MakeVectors( const Vector &vecAngles ) { }
BOOL UTIL_IsValidEntity( edict_t *pent ) { return TRUE; }
void UTIL_SetOrigin( entvars_t *, const Vector &org ) { }
BOOL UTIL_GetNextBestWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pCurrentWeapon ) { return TRUE; }
void UTIL_LogPrintf(char *,...) { }
void ClientPrintAll( int,char const *,char const *,char const *,char const *,char const *) { }
void ClientPrint( entvars_t *pClient, int msg_dest, const char *msg_name, const char *param1, const char *param2, const char *param3, const char *param4 ) { }

// CBaseToggle Stubs
int CBaseToggle::Restore( class CRestore & ) { return 1; }
int CBaseToggle::Save( class CSave & ) { return 1; }
void CBaseToggle::PostRestore() { }

void CBaseToggle::KeyValue( struct KeyValueData_s * ) { }

//MODDD - required niw that they are virtual
void CBaseToggle::LinearMove( Vector	vecDest, float flSpeed ){}
void CBaseToggle::AngularMove( Vector vecDestAngle, float flSpeed ){}





// CGrenade Stubs
void CGrenade::BounceSound( void ) { }

GENERATE_TRACEATTACK_IMPLEMENTATION_DUMMY_CLIENT(CGrenade)
GENERATE_TAKEDAMAGE_IMPLEMENTATION_DUMMY_CLIENT(CGrenade)
	
//MODDD - changed, parameters were ignored and thus ignored..
//void CGrenade::Explode( Vector, Vector ) { }
void CGrenade::Explode(void) { }
void CGrenade::Explode(TraceResult *pTrace, int bitsDamageType ){}
void CGrenade::Explode(TraceResult *pTrace, int bitsDamageType, int bitsDamageTypeMod ){}
void CGrenade::Explode(TraceResult* pTrace, float rawDamage, float flRange, int bitsDamageType, int bitsDamageTypeMod, float shrapMod) {}


GENERATE_KILLED_IMPLEMENTATION_DUMMY_CLIENT(CGrenade)
//void CGrenade::Killed( entvars_t *, int ) { }


CGrenade::CGrenade(void) {}
void CGrenade::Spawn( void ) { }
CGrenade * CGrenade::ShootTimed( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float flDamage, float flDetonateTime){ return 0; }
CGrenade* CGrenade::ShootTimedDropped(entvars_t* pevOwner, Vector vecStart, Vector vecVelocity, float flDamage, float flDetonateTime) { return 0; }
CGrenade *CGrenade::ShootContact( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float flDamage){ return 0; }
void CGrenade::DetonateUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value ){ }

BOOL CGrenade::isOrganic(){return FALSE;}
BOOL CGrenade::usesSoundSentenceSave(){return FALSE;}
void CGrenade::groundContact(void) {}




void UTIL_Remove( CBaseEntity *pEntity ){ }
struct skilldata_t  gSkillData;
void UTIL_SetSize( entvars_t *pev, const Vector &vecMin, const Vector &vecMax ){ }
CBaseEntity *UTIL_FindEntityInSphere( CBaseEntity *pStartEntity, const Vector &vecCenter, float flRadius ){ return 0;}

Vector UTIL_VecToAngles( const Vector &vec ){ return 0; }
CSprite *CSprite::SpriteCreate( const char *pSpriteName, const Vector &origin, BOOL animate ) { return 0; }
void CBeam::PointEntInit( const Vector &start, int endIndex ) { }
CBeam *CBeam::BeamCreate( const char *pSpriteName, int width ) { return NULL; }
void CSprite::Expand( float scaleSpeed, float fadeSpeed ) { }



CBaseEntity* CBaseMonster::CheckTraceHullAttack( float flDist, int iDamage, int iDmgType ) { return NULL; }
CBaseEntity* CBaseMonster::CheckTraceHullAttack( float flDist, int iDamage, int iDmgType, int iDmgTypeMod ) { return NULL; }
CBaseEntity* CBaseMonster::CheckTraceHullAttack( const Vector vecStartOffset, float flDist, int iDamage, int iDmgType ) { return NULL; }
CBaseEntity* CBaseMonster::CheckTraceHullAttack( const Vector vecStartOffset, float flDist, int iDamage, int iDmgType, int iDmgTypeMod ) { return NULL; }


void CBaseMonster::Eat ( float flFullDuration ) { }
BOOL CBaseMonster::FShouldEat ( void ) { return TRUE; }
void CBaseMonster::BarnacleVictimBitten ( entvars_t *pevBarnacle ) { }
void CBaseMonster::BarnacleVictimReleased ( void ) { }
void CBaseMonster::Listen ( void ) { }
float CBaseMonster::FLSoundVolume ( CSound *pSound ) { return 0.0; }
BOOL CBaseMonster::FValidateHintType ( short sHint ) { return FALSE; }
void CBaseMonster::Look ( float flDistance ) { }
int CBaseMonster::ISoundMask ( void ) { return 0; }
CSound* CBaseMonster::PBestSound ( void ) { return NULL; }
CSound* CBaseMonster::PBestScent ( void ) { return NULL; } 



float DetermineInterval(void) {return 0;}
float DetermineInterval(float flInterval) { return 0; }
float DetermineInterval_SAFE(void) { return 0; }
float DetermineInterval_SAFE(float flInterval) { return 0; }
//MODDD - method edited.  and new versions.
//float CBaseAnimating::StudioFrameAdvance ( float flInterval ) { return 0.0; }
float CBaseAnimating::StudioFrameAdvance_SIMPLE(void) { return 0.0; }
float CBaseAnimating::StudioFrameAdvance_SIMPLE(float flInterval) { return 0.0; }
void CBaseAnimating::StudioFrameAdvance(float flInterval) { }



void CBaseMonster::CallMonsterThink(void) {}
void CBaseMonster::MonsterThink ( void ) { }

void CBaseMonster::MonsterUse ( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value ) { }
int CBaseMonster::IgnoreConditions ( void ) { return 0; }
void CBaseMonster::RouteClear ( void ) { }
void CBaseMonster::RouteNew ( void ) { }
BOOL CBaseMonster::FRouteClear ( void ) { return FALSE; }


BOOL CBaseMonster::FRefreshRouteStrict ( void ) { return 0; }
BOOL CBaseMonster::FRefreshRoute ( void ) { return 0; }
BOOL CBaseMonster::FRefreshRouteCheap ( void ) { return 0; }
BOOL CBaseMonster::FRefreshRouteChaseEnemySmart ( void ) { return 0; }
BOOL CBaseMonster::FRefreshRouteChaseEnemySmartSafe ( void ) { return 0; }


BOOL CBaseMonster::MoveToEnemy( Activity movementAct, float waitTime ) { return FALSE; }
BOOL CBaseMonster::MoveToLocation( Activity movementAct, float waitTime, const Vector &goal ) { return FALSE; }
BOOL CBaseMonster::MoveToLocationStrict( Activity movementAct, float waitTime, const Vector &goal ) { return FALSE; }
//BOOL CBaseMonster::MoveToLocation( Activity movementAct, float waitTime, const Vector &goal, float argGoalDistTolerance ) { return FALSE; }
BOOL CBaseMonster::MoveToLocationCheap( Activity movementAct, float waitTime, const Vector &goal ) { return FALSE; }
BOOL CBaseMonster::MoveToTarget( Activity movementAct, float waitTime ) { return FALSE; }
BOOL CBaseMonster::MoveToTargetStrict( Activity movementAct, float waitTime ) { return FALSE; }
BOOL CBaseMonster::MoveToNode( Activity movementAct, float waitTime, const Vector &goal ) { return FALSE; }
int ShouldSimplify( int routeType ) { return TRUE; }
void CBaseMonster::RouteSimplify( CBaseEntity *pTargetEnt ) { }
void CBaseMonster::DrawRoute( entvars_t *pev, WayPoint_t *m_Route, int m_iRouteLength, int m_iRouteIndex, int r, int g, int b ){}
void CBaseMonster::DrawMyRoute(int r, int g, int b){}
BOOL CBaseMonster::FBecomeProne ( void ) { return TRUE; }
BOOL CBaseMonster::CheckRangeAttack1 ( float flDot, float flDist ) { return FALSE; }
BOOL CBaseMonster::CheckRangeAttack2 ( float flDot, float flDist ) { return FALSE; }
BOOL CBaseMonster::CheckMeleeAttack1 ( float flDot, float flDist ) { return FALSE; }
BOOL CBaseMonster::CheckMeleeAttack2 ( float flDot, float flDist ) { return FALSE; }

//MODDD - new
void CBaseMonster::ScheduleChange(){}
Schedule_t* CBaseMonster::GetStumpedWaitSchedule(){return NULL;}

void CBaseMonster::CheckAttacks ( CBaseEntity *pTarget, float flDist ) { }
BOOL CBaseMonster::FCanCheckAttacks ( void ) { return FALSE; }
int CBaseMonster::CheckEnemy ( CBaseEntity *pEnemy ) { return 0; }
void CBaseMonster::PushEnemy( CBaseEntity *pEnemy, Vector &vecLastKnownPos ) { }
BOOL CBaseMonster::PopEnemy( ) { return FALSE; }
void CBaseMonster::DrawFieldOfVision(void){}

void CBaseMonster::SetActivity ( Activity NewActivity ) { }
void CBaseMonster::RefreshActivity(void){}

//MODDD - new
BOOL CBaseMonster::allowedToSetActivity(void){ return FALSE;}
BOOL CBaseMonster::tryGetTaskID(void){ return FALSE;}
const char* CBaseMonster::tryGetScheduleName(void){ return NULL;}

void CBaseMonster::setModel(void){}
void CBaseMonster::setModel(const char* m){} //empty.
BOOL CBaseMonster::getMonsterBlockIdleAutoUpdate(){return FALSE;}
void CBaseMonster::testMethod(void){}

BOOL CBaseMonster::interestedInBait(int arg_classID){return 0;}
SCHEDULE_TYPE CBaseMonster::getHeardBaitSoundSchedule(CSound* pSound){return (SCHEDULE_TYPE)0;}
SCHEDULE_TYPE CBaseMonster::_getHeardBaitSoundSchedule(CSound* pSound){return (SCHEDULE_TYPE)0;}
SCHEDULE_TYPE CBaseMonster::getHeardBaitSoundSchedule(){return (SCHEDULE_TYPE)0;}

BOOL CBaseMonster::hasSeeEnemyFix(){return FALSE;}
BOOL CBaseMonster::getForceAllowNewEnemy(CBaseEntity* pOther){return FALSE;}

BOOL CBaseMonster::needsMovementBoundFix(void){return FALSE;}
void CBaseMonster::cheapKilled(void){}
void CBaseMonster::cheapKilledFlyer(void){}
void CBaseMonster::OnKilledSetTouch(void){}

void CBaseMonster::KilledFinishTouch( CBaseEntity *pOther ){}

int CBaseMonster::getLoopingDeathSequence(void){return 0;}
Schedule_t* CBaseMonster::flyerDeathSchedule(void){return NULL;}
BOOL CBaseMonster::getMovementCanAutoTurn(void){return FALSE;}


void CBaseMonster::updateEnemyLKP(void){}
void CBaseMonster::setEnemyLKP(const Vector& argNewVector, float zOffset){};
void CBaseMonster::setEnemyLKP(const Vector& argNewVector, float zOffset, const Vector& extraAddIn){};
void CBaseMonster::setEnemyLKP(CBaseEntity* theEnt){}
void CBaseMonster::setEnemyLKP(CBaseEntity* theEnt, const Vector& extraAddIn){}
void CBaseMonster::setEnemyLKP(entvars_t* theEntPEV){}
void CBaseMonster::setEnemyLKP(entvars_t* theEntPEV, const Vector& extraAddIn){}
void CBaseMonster::setEnemyLKP_Investigate(const Vector& argNewVector){}






void CBaseMonster::setPhysicalHitboxForDeath(){return;}
void CBaseMonster::DeathAnimationStart(){return;}
void CBaseMonster::DeathAnimationEnd(){return;}
void CBaseMonster::onDeathAnimationEnd(){return;}

int CBaseMonster::LookupActivityFiltered(int NewAcitivty){return 0;}
int CBaseMonster::LookupActivity(int NewActivity){return 0;}
int CBaseMonster::LookupActivityHeaviest(int NewActivity){return 0;}

void CBaseMonster::OnTakeDamageSetConditions(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType, int bitsDamageTypeMod){}


void CBaseMonster::setAnimationSmart(const char* arg_animName){return;}
void CBaseMonster::setAnimationSmart(const char* arg_animName, float arg_frameRate){return;}
void CBaseMonster::setAnimationSmart(int arg_animIndex, float arg_frameRate){return;}
void CBaseMonster::setAnimationSmartAndStop(const char* arg_animName){return;}
void CBaseMonster::setAnimationSmartAndStop(const char* arg_animName, float arg_frameRate){return;}
void CBaseMonster::setAnimationSmartAndStop(int arg_animIndex, float arg_frameRate){return;}




void CBaseMonster::setAnimation(char* animationName){return;}
void CBaseMonster::setAnimation(char* animationName, BOOL forceException){return;}
void CBaseMonster::setAnimation(char* animationName, BOOL forceException, BOOL forceLoopsProperty){return;}
void CBaseMonster::setAnimation(char* animationName, BOOL forceException, BOOL forceLoopsProperty, int extraLogic){return;}

GENERATE_DEADTAKEDAMAGE_IMPLEMENTATION_DUMMY_CLIENT(CBaseMonster)

void CBaseMonster::onNewRouteNode(void){}

//naw.
//void CSquadMonster::SetActivity ( Activity NewActivity ) { }
//void CSquadMonster::SetActivity ( Activity NewActivity, BOOL forceReset  ) { }



void CBaseMonster::SetSequenceByIndex(int iSequence){}
void CBaseMonster::SetSequenceByName(char* szSequence){}
void CBaseMonster::SetSequenceByIndex(int iSequence, float flFramerateMulti){}
void CBaseMonster::SetSequenceByName(char* szSequence, float flFramerateMulti){}
void CBaseMonster::SetSequenceByIndex(int iSequence, BOOL safeReset){}
void CBaseMonster::SetSequenceByName(char* szSequence, BOOL safeReset){}
void CBaseMonster::SetSequenceByIndex(int iSequence, float flFramerateMulti, BOOL safeReset){}
void CBaseMonster::SetSequenceByName(char* szSequence, float flFramerateMulti, BOOL safeReset){}

void CBaseMonster::SetSequenceByIndexForceLoops(int iSequence, BOOL forceLoops){}
void CBaseMonster::SetSequenceByNameForceLoops(char* szSequence, BOOL forceLoops){}
void CBaseMonster::SetSequenceByIndexForceLoops(int iSequence, float flFramerateMulti, BOOL forceLoops){}
void CBaseMonster::SetSequenceByNameForceLoops(char* szSequence, float flFramerateMulti, BOOL forceLoops){}
void CBaseMonster::SetSequenceByIndexForceLoops(int iSequence, BOOL safeReset, BOOL forceLoops){}
void CBaseMonster::SetSequenceByNameForceLoops(char* szSequence, BOOL safeReset, BOOL forceLoops){}
void CBaseMonster::SetSequenceByIndexForceLoops(int iSequence, float flFramerateMulti, BOOL safeReset, BOOL forceLoops){}
void CBaseMonster::SetSequenceByNameForceLoops(char* szSequence, float flFramerateMulti, BOOL safeReset, BOOL forceLoops){}




//MODDD - new 
int CBaseMonster::CheckLocalMove ( const Vector &vecStart, const Vector &vecEnd, CBaseEntity *pTarget, BOOL doZCheck, float *pflDist ) { return 0; }
int CBaseMonster::CheckLocalMoveHull ( const Vector &vecStart, const Vector &vecEnd, CBaseEntity *pTarget, float *pflDist ) { return 0; }
BOOL CBaseMonster::ZCheck(const Vector& vecPosition, const Vector& vecEnd){return FALSE;}
BOOL CBaseMonster::ZCheck_Ground(const Vector& vecPosition, const Vector& vecEnd){return FALSE;}
BOOL CBaseMonster::ZCheck_Flyer(const Vector& vecPosition, const Vector& vecEnd){return FALSE;}



float CBaseMonster::OpenDoorAndWait( entvars_t *pevDoor ) { return 0.0; }
void CBaseMonster::AdvanceRoute ( float distance, float flInterval ) { }
int CBaseMonster::RouteClassify( int iMoveFlag ) { return 0; }
int CBaseMonster::MovementGoalToMoveFlag(int iMoveGoal){return 0;}

BOOL CBaseMonster::attemptRampFix(const Vector& vecGoal, int iMoveFlag, CBaseEntity* pTarget) {return FALSE;}

BOOL CBaseMonster::BuildRoute ( const Vector &vecGoal, int iMoveFlag, CBaseEntity *pTarget ) { return FALSE; }


BOOL CBaseMonster::NoFriendlyFireImp(const Vector& startVec, const Vector& endVec){return FALSE;}

BOOL CBaseMonster::getHasPathFindingModA(){return FALSE;}
BOOL CBaseMonster::getHasPathFindingMod(){return FALSE;}

//MODDD
void CBaseMonster::heardBulletHit(entvars_t* pevShooter) { }
void CBaseMonster::wanderAway(const Vector& toWalkAwayFrom){}




void CBaseMonster::InsertWaypoint ( Vector vecLocation, int afMoveFlags ) { }
BOOL CBaseMonster::FTriangulate ( const Vector &vecStart , const Vector &vecEnd, float flDist, CBaseEntity *pTargetEnt, Vector *pApex ) { return FALSE; }
BOOL CBaseMonster::FPutFailedGoalAlongSurface(Vector vecFailedGoal, float flLookZOffset, float flAlongSurfaceDist, CBaseEntity* pTargetEnt, Vector* out_vecNewTarget) {return FALSE;}

BOOL CBaseMonster::usesSegmentedMove(void) {return FALSE;}
int CBaseMonster::MovePRE(float flInterval, float& flWaypointDist, float& flCheckDist, float& flDist, Vector& vecDir, CBaseEntity*& pTargetEnt) { return FALSE; }
void CBaseMonster::Move ( float flInterval ) { }
BOOL CBaseMonster::ShouldAdvanceRoute( float flWaypointDist, float flInterval ) { return FALSE; }
BOOL CBaseMonster::CheckPreMove(void) {return FALSE;}
void CBaseMonster::MoveExecute( CBaseEntity *pTargetEnt, const Vector &vecDir, float flInterval ) { }
void CBaseMonster::MonsterInit ( void ) { }
void CBaseMonster::MonsterInitThink ( void ) { }
void CBaseMonster::StartMonster ( void ) { }
void CBaseMonster::TaskComplete(void) {}
void CBaseMonster::MovementComplete( void ) { }

//MODDD - new, moved outside of basemonster.h so it must show up implemented (dummied) here.
void CBaseMonster::TaskFail(void){}

int CBaseMonster::TaskIsRunning( void ) { return 0; }


int CBaseMonster::IRelationship ( CBaseEntity *pTarget ) { return 0; }

//Canned.
//int CBaseMonster::IRelationshipOfClass (int argClassValue, CBaseEntity* pTarget ){return 0;}

BOOL CBaseMonster::FindCover ( Vector vecThreat, Vector vecViewOffset, float flMinDist, float flMaxDist ) { return FALSE; }
BOOL CBaseMonster::FindRandom ( Activity movementAct, Vector vecThreat, Vector vecViewOffset, float flMinDist, float flMaxDist ) { return FALSE; }
BOOL CBaseMonster::SCHEDULE_attemptFindCoverFromEnemy(Task_t* pTask){return FALSE;}
BOOL CBaseMonster::FValidateCover(const Vector& vecCoverLocation){return FALSE;}
BOOL CBaseMonster::BuildNearestRoute ( Vector vecThreat, Vector vecViewOffset, float flMinDist, float flMaxDist, BOOL randomNodeSearchStart ) { return FALSE; }
BOOL CBaseMonster::BuildNearestRoute( Vector vecThreat, Vector vecViewOffset, float flMinDist, float flMaxDist, BOOL randomNodeSearchStart, int iMoveFlag, CBaseEntity* pTarget ){return FALSE;}

void CBaseMonster::SetTurnActivity(void){}
void CBaseMonster::SetTurnActivityForceAct(void){}

CBaseEntity *CBaseMonster::BestVisibleEnemy ( void ) { return NULL; }
BOOL CBaseMonster::FInViewCone ( CBaseEntity *pEntity ) { return FALSE; }
BOOL CBaseMonster::FInViewCone ( Vector *pOrigin ) { return FALSE; }

BOOL CBaseEntity::FVisible (CBaseEntity *pEntity ) { return FALSE; }
BOOL CBaseEntity::FVisible (const Vector &vecTargetOrigin ) { return FALSE; }
BOOL CBaseEntity::FVisible (const Vector& vecLookerOrigin, CBaseEntity *pEntity ) { return FALSE; }
BOOL CBaseEntity::FVisible (const Vector& vecLookerOrigin, const Vector &vecTargetOrigin ) { return FALSE; }

//MODDD - new
BOOL CBaseEntity::SeeThroughWaterLine(void){return FALSE;}
void CBaseEntity::SetGravity(float newGravityVal) { }

void CBaseMonster::MakeIdealYaw( Vector vecTarget ) { }
float CBaseMonster::FlYawDiff ( void ) { return 0.0; }
float CBaseMonster::ChangeYaw ( int yawSpeed ) { return 0; }
float CBaseMonster::VecToYaw ( Vector vecDir ) { return 0.0; }

//MODDD - constructor
CBaseAnimating::CBaseAnimating(void){}
void CBaseAnimating::resetFrame(void){}
int CBaseAnimating::LookupActivityHard ( int activity ){return 0;}
int CBaseAnimating::tryActivitySubstitute(int activity){return 0;}

void CBaseAnimating::queuedAnimClear(void){return;}
void CBaseAnimating::queuedAnimPush(char* arg_animName, float arg_frameStart, float arg_frameCutoff, float arg_framerate){return;}
void CBaseAnimating::applyAnimationQueueSuggestion(void){return;}
void CBaseAnimating::checkEndOfAnimation(void){return;}

void CBaseAnimating::animEventQueuePush(float arg_timeFromStart, float arg_eventType){return;}
void CBaseAnimating::resetEventQueue(void){return;}






int CBaseAnimating::LookupActivity ( int activity ) { return 0; }
int CBaseAnimating::LookupActivityHeaviest ( int activity ) { return 0; }
void CBaseMonster::SetEyePosition ( void ) { }
int CBaseAnimating::LookupSequence ( const char *label ) { return 0; }
void CBaseAnimating::ResetSequenceInfo ( ) { }
BOOL CBaseAnimating::GetSequenceFlags( ) { return FALSE; }
void CBaseAnimating::DispatchAnimEvents ( float flInterval ) { }
void CBaseMonster::HandleAnimEvent( MonsterEvent_t *pEvent ) { }

//MODDD - new.
void CBaseAnimating::onAnimationLoop(void){}

BOOL CBaseAnimating::onResetBlend0(void){return FALSE;}
BOOL CBaseAnimating::onResetBlend1(void){return FALSE;}
BOOL CBaseAnimating::onResetBlend2(void){return FALSE;}

BOOL CBaseAnimating::canResetBlend0(void){return FALSE;}
BOOL CBaseAnimating::canResetBlend1(void){return FALSE;}
BOOL CBaseAnimating::canResetBlend2(void){return FALSE;}


//MODDD - ok?
//Belongs to CBaseAnimating here.  CBaseMonster has no broad cases for it (custom).

//WAIT.  "Already has a body?"  Uhhhhh, how??  Other animating stuff has body's in here just fine.

//void CBaseAnimating::HandleEventQueueEvent( int arg_eventID) { }

//OH.  Probably having the real simple  {return;}    method body in cbase.h like HandleAnimEvent does in there too,
//meaning "CBaseAnimating" doesn't have to stub either in here.   CBaseMonster has to as it does not have the method body in the .h file.

//void CBaseMonster::HandleEventQueueEvent( int arg_eventID) { }


float CBaseAnimating::SetBoneController ( int iController, float flValue ) { return 0.0; }
void CBaseAnimating::InitBoneControllers ( void ) { }
float CBaseAnimating::SetBlending ( int iBlender, float flValue ) { return 0; }
void CBaseAnimating::GetBonePosition ( int iBone, Vector &origin, Vector &angles ) { }
void CBaseAnimating::GetAttachment ( int iAttachment, Vector &origin, Vector &angles ) { }
int CBaseAnimating::FindTransition( int iEndingSequence, int iGoalSequence, int *piDir ) { return -1; }
void CBaseAnimating::GetAutomovement( Vector &origin, Vector &angles, float flInterval ) { }
void CBaseAnimating::SetBodygroup( int iGroup, int iValue ) { }
int CBaseAnimating::GetBodygroup( int iGroup ) { return 0; }
Vector CBaseMonster::GetGunPosition( void ) { return g_vecZero; }
Vector CBaseMonster::GetGunPositionAI(void){return g_vecZero;}
void CBaseMonster::lookAtEnemy_pitch(void){}




void CBaseEntity::FireBullets(ULONG cShots, Vector vecSrc, Vector vecDirShooting, Vector vecSpread, float flDistance, int iBulletType, int iTracerFreq, int iDamage, entvars_t *pevAttacker ) { }
void CBaseEntity::TraceBleed( float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType ) { }
void CBaseEntity::TraceBleed( float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType, int bitsDamageTypeMod ) { }
void CBaseMonster::MakeDamageBloodDecal ( int cCount, float flNoise, TraceResult *ptr, const Vector &vecDir ) { }
BOOL CBaseMonster::FGetNodeRoute(Vector vecStart, Vector vecDest) {return FALSE;}
BOOL CBaseMonster::FGetNodeRoute(Vector vecStart, Vector vecDest, BOOL asIfSnappedToGround) {return FALSE;}
BOOL CBaseMonster::FGetNodeRoute_Final(Vector vecStart, Vector vecDest, BOOL asIfSnappedToGround, int iSrcNode, int iDestNode, int iNodeTypeInfo) {return FALSE;}
BOOL CBaseMonster::FVerifyRoute(Vector vecStart, Vector vecDest, int iPath[], int iPathSize) {return FALSE;}

int CBaseMonster::FindHintNode ( void ) { return NO_NODE; }
void CBaseMonster::ReportAIState( void ) { }
void CBaseMonster::KeyValue( KeyValueData *pkvd ) { }
BOOL CBaseMonster::FCheckAITrigger ( void ) { return FALSE; }
BOOL CBaseMonster::NoFriendlyFire(void) { return FALSE; }


int CBaseMonster::CanPlaySentence(BOOL fDisregardState){return FALSE;}
int CBaseMonster::CanPlaySequence( BOOL fDisregardMonsterState, int interruptLevel ) { return FALSE; }

BOOL CBaseMonster::FindLateralCover ( const Vector &vecThreat, const Vector &vecViewOffset ) { return FALSE; }
Vector CBaseMonster::ShootAtEnemy( const Vector &shootOrigin ) { return g_vecZero; }
Vector CBaseMonster::ShootAtEnemyEyes( const Vector &shootOrigin ) { return g_vecZero; }
Vector CBaseMonster::ShootAtEnemyMod( const Vector &shootOrigin ) { return g_vecZero; }
BOOL CBaseMonster::FacingIdeal( void ) { return FALSE; }
BOOL CBaseMonster::FacingIdeal(float argDegreeTolerance){return FALSE;}
BOOL CBaseMonster::FCanActiveIdle ( void ) { return FALSE; }
void CBaseMonster::PlaySentence( const char *pszSentence, float duration, float volume, float attenuation ) { }
void CBaseMonster::PlayScriptedSentence( const char *pszSentence, float duration, float volume, float attenuation, BOOL bConcurrent, CBaseEntity *pListener ) { }
void CBaseMonster::SentenceStop( void ) { }
void CBaseMonster::CorpseFallThink( void ) { }
void CBaseMonster::MonsterInitDead( void ) { }
BOOL CBaseMonster::BBoxFlat ( void ) { return TRUE; }
BOOL CBaseMonster::GetEnemy ( void ) { return FALSE; }
//MODDD
BOOL CBaseMonster::GetEnemy (BOOL arg_forceWork ){return FALSE;}


GENERATE_TRACEATTACK_IMPLEMENTATION_DUMMY_CLIENT(CBaseMonster)
GENERATE_TAKEDAMAGE_IMPLEMENTATION_DUMMY_CLIENT(CBaseMonster)
BOOL CBaseMonster::ChangeHealthFiltered(entvars_t* pevAttacker, float flDamage) { return FALSE; }


Task_t* CBaseMonster::GetTask(void){return NULL;}
int CBaseMonster::getTaskNumber(void){return -1;}
const char* CBaseMonster::getScheduleName(void){return NULL;}
BOOL CBaseMonster::forceIdleFrameReset(void){return FALSE;}


BOOL CBaseMonster::usesAdvancedAnimSystem(void){return FALSE;}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////









///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////




void CBaseMonster::OnCineCleanup(CCineMonster* pOldCine){ }

CBaseEntity* CBaseMonster::DropItem ( char *pszItemName, const Vector &vecPos, const Vector &vecAng ) { return NULL; }
BOOL CBaseMonster::ShouldFadeOnDeath( void ) { return FALSE; }



void RadiusDamageTest( Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, float flRadius, int iClassIgnore, int bitsDamageType, int bitsDamageTypeMod ){}



void CBaseMonster::RadiusDamageAutoRadius(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType ){}
void CBaseMonster::RadiusDamageAutoRadius(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType, int bitsDamageTypeMod ){}
void CBaseMonster::RadiusDamageAutoRadius(Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType ){}
void CBaseMonster::RadiusDamageAutoRadius(Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType, int bitsDamageTypeMod  ){}
void RadiusDamageAutoRadius(Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType );
void RadiusDamageAutoRadius(Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType, int bitsDamageTypeMod  );
void RadiusDamage( Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, float flRadius, int iClassIgnore, int bitsDamageType ){}
void RadiusDamage( Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, float flRadius, int iClassIgnore, int bitsDamageType, int bitsDamageTypeMod ){}



void CBaseMonster::FadeMonster( void ) { }



BOOL CBaseMonster::DetermineGibHeadBlock(void){return FALSE;}

//MODDD - using this preprocessor method.
GENERATE_GIBMONSTER_IMPLEMENTATION_DUMMY_CLIENT(CBaseMonster)
GENERATE_GIBMONSTERGIB_IMPLEMENTATION_DUMMY_CLIENT(CBaseMonster)
GENERATE_GIBMONSTERSOUND_IMPLEMENTATION_DUMMY_CLIENT(CBaseMonster)
GENERATE_GIBMONSTEREND_IMPLEMENTATION_DUMMY_CLIENT(CBaseMonster)

BOOL CBaseMonster::HasHumanGibs( void ) { return FALSE; }
BOOL CBaseMonster::HasAlienGibs( void ) { return FALSE; }
Activity CBaseMonster::GetDeathActivity ( void ) { return ACT_DIE_HEADSHOT; }
MONSTERSTATE CBaseMonster::GetIdealState ( void ) { return MONSTERSTATE_ALERT; }
Schedule_t* CBaseMonster::GetScheduleOfType ( int Type ) { return NULL; }
Schedule_t *CBaseMonster::GetSchedule ( void ) { return NULL; }
void CBaseMonster::RunTask ( Task_t *pTask ) { }
void CBaseMonster::StartTask ( Task_t *pTask ) { }
Schedule_t *CBaseMonster::ScheduleFromName( const char *pName ) { return NULL;}
void CBaseMonster::BecomeDead( void ) {}
void CBaseMonster::RunAI ( void ) {}


//NOTICE - player's Killed clientside implementation is in hl_weapons.

//NOTE - CBaseEntity's killed implementation is in hl_weapons instead, given a slight bit of behavior for clientside.
GENERATE_KILLED_IMPLEMENTATION_DUMMY_CLIENT(CBaseMonster)
//void CBaseMonster::Killed( entvars_t *pevAttacker, int iGib ) {}


void CBasePlayer::StartReanimation(void) {}
void CBasePlayer::StartReanimationPost(int preReviveSequence) {}

void CBasePlayer::reviveIfDead(void) {}
void CBasePlayer::startRevive(void) {}
void CBasePlayer::declareRevivelessDead(void) {}


int CBaseMonster::TakeHealth (float flHealth, int bitsDamageType) { return 0; }

CBaseMonster::CBaseMonster(void){}
BOOL CBaseMonster::usesSoundSentenceSave(void){return FALSE;}




float CBaseMonster::TimedDamageBuddhaFilter(float dmgIntent) { return 0; }
void CBaseMonster::TimedDamagePostBuddhaCheck(void) {}
int CBaseMonster::convert_itbd_to_damage(int i){ return 0;}
void CBaseMonster::removeTimedDamage(int arg_type, int* m_bitsDamageTypeRef) {}
void CBaseMonster::removeTimedDamageImmediate(int arg_type, int* m_bitsDamageTypeRef, BYTE bDuration) {}
BYTE CBaseMonster::parse_itbd_duration(int i) { return 0; }
void CBaseMonster::parse_itbd(int i) {}
void CBaseMonster::timedDamage_nonFirstFrame(int i, int* m_bitsDamageTypeRef) {}
void CBaseMonster::CheckTimeBasedDamage(void){}

void CBaseMonster::setTimedDamageDuration(int i, int* m_bitsDamageTypeRef) {}
void CBaseMonster::attemptResetTimedDamage(BOOL forceReset) {}
void CBaseMonster::applyNewTimedDamage(int arg_bitsDamageType, int arg_bitsDamageTypeMod) {}


BOOL CBaseMonster::isSizeGiant(void){return FALSE;}
BOOL CBaseMonster::isOrganic(){return FALSE;}
BOOL CBaseMonster::isOrganicLogic(){return FALSE;}

BOOL CBaseEntity::getIsBarnacleVictimException(void){return FALSE;}
float CBaseMonster::getBarnaclePulledTopOffset(void){return 0;}
float CBaseMonster::getBarnacleForwardOffset(void){return 0;}
//NOTICE: "1", 100%, is the default.
float CBaseMonster::getBarnacleAnimationFactor(void){return 0;}


void CBaseMonster::ForgetEnemy(void) {}

void CBaseMonster::removeFromPoweredUpCommandList(CBaseMonster* argToRemove){}
void CBaseMonster::forceNewEnemy(CBaseEntity* argIssuing, CBaseEntity* argNewEnemy, BOOL argPassive){}

void CBaseMonster::setPoweredUpOff(void){}
void CBaseMonster::setPoweredUpOn(CBaseMonster* argPoweredUpCauseEnt, float argHowLong ){}
void CBaseMonster::forgetForcedEnemy(CBaseMonster* argIssuing, BOOL argPassive){}

void CBaseMonster::StartReanimation(void){}
void CBaseMonster::StartReanimationPost(int preReviveSequence){}
float CBaseMonster::MoveYawDegreeTolerance(){return 0;}
int CBaseMonster::BloodColorRedFilter(){return 0;}
int CBaseMonster::CanUseGermanModel(){return 0;}

WayPoint_t* CBaseMonster::GetGoalNode(void){return NULL;}
CBaseEntity* CBaseMonster::GetGoalEntity(void) {return NULL;}

void CBaseMonster::ReportGeneric(){}
void CBaseMonster::onEnemyDead(CBaseEntity* pRecentEnemy) {}
BOOL CBaseMonster::predictRangeAttackEnd(void) { return FALSE; }

void CBaseMonster::CheckRespawn(void) {}
CBaseEntity* CBaseMonster::Respawn(void) { return NULL; }
void CBaseMonster::AttemptToMaterialize(void) {}
void CBaseMonster::Materialize(void) {}

float CBaseMonster::ScriptEventSoundAttn(void) {return 0;}
float CBaseMonster::ScriptEventSoundVoiceAttn(void) { return 0; }
BOOL CBaseMonster::CanMakeBloodParticles(void) {return FALSE;}
BOOL CBaseMonster::AffectedByKnockback(void) {return FALSE;}


//removed.
//void CBaseMonster::PreThink(){}
//void CBaseEntity::broadThink(){}
//void CBaseMonster::broadThink(){}


/*
void CBasePlayer::Think(void){}
void CBasePlayer::MonsterThink(void){}
*/

void CBasePlayer::commonReset(void){}
void CBasePlayer::autoSneakyCheck(void){}
void CBasePlayer::turnOnSneaky(void){}
void CBasePlayer::turnOffSneaky(void){}

void CBasePlayer::PainSound( void ){}


//YEP.
BOOL CBasePlayer::HasNamedPlayerItem(const char *pszItemName){return FALSE;}


//CBaseEntity::CBaseEntity(){}
//CBaseMonster::CBaseMonster(){}
//CBasePlayer::CBasePlayer(){}


int CBaseMonster::Restore( class CRestore & ) { return 1; }
int CBaseMonster::Save( class CSave & ) { return 1; }
void CBaseMonster::PostRestore(){}

int TrainSpeed(int iSpeed, int iMax) { 	return 0; }
void CBasePlayer::DeathSound( void ) { }
int CBasePlayer::TakeHealth( float flHealth, int bitsDamageType ) { return 0; }

//MODDD
//NOTICE: implementations already present in hl_weapons.cpp.  Only one implementation allowed clientside.
//void CBasePlayer::Spawn(){}
//void CBasePlayer::Spawn(BOOL revived){}
//void CBasePlayer::Activate(){}



GENERATE_TRACEATTACK_IMPLEMENTATION_DUMMY_CLIENT(CBasePlayer)
GENERATE_TAKEDAMAGE_IMPLEMENTATION_DUMMY_CLIENT(CBasePlayer)

GENERATE_DEADTAKEDAMAGE_IMPLEMENTATION_DUMMY_CLIENT(CBasePlayer)
GENERATE_GIBMONSTER_IMPLEMENTATION_DUMMY_CLIENT(CBasePlayer)




void CBasePlayer::PackDeadPlayerItems( void ) { }
void CBasePlayer::RemoveAllAmmo(void) {}
void CBasePlayer::RemoveAllItems( BOOL removeSuit ) { }
void CBasePlayer::SetAndUpdateBattery(int argNewBattery) {}

void CBasePlayer::FadeMonster( void ) { }


Vector CBasePlayer::BodyTarget(const Vector& posSrc) { return g_vecZero; }
Vector CBasePlayer::BodyTargetMod(const Vector& posSrc) { return g_vecZero; }

void CBasePlayer::SetAnimation( PLAYER_ANIM playerAnim ) { }
void CBasePlayer::WaterMove() { }
BOOL CBasePlayer::IsOnLadder( void ) { return FALSE; }
void CBasePlayer::PlayerDeathThink(void) { }
void CBasePlayer::StartDeathCam( void ) { }
void CBasePlayer::StartObserver( Vector vecPosition, Vector vecViewAngle ) { }
void CBasePlayer::PlayerUse ( void ) { }
void CBasePlayer::Jump() { }
void CBasePlayer::Duck( ) { }
int  CBasePlayer::Classify ( void ) { return 0; }
void CBasePlayer::PreThink(void) { }


float CBasePlayer::TimedDamageBuddhaFilter(float dmgIntent) { return 0; }
void CBasePlayer::TimedDamagePostBuddhaCheck(void) {}
void CBasePlayer::removeTimedDamageImmediate(int arg_type, int* m_bitsDamageTypeRef, BYTE bDuration) {}
void CBasePlayer::parse_itbd(int i) {}
BYTE CBasePlayer::parse_itbd_duration(int i) { return 0; }
void CBasePlayer::timedDamage_nonFirstFrame(int i, int* m_bitsDamageTypeRef) {}


void CBasePlayer::UpdateGeigerCounter( void ) { }
void CBasePlayer::CheckSuitUpdate() { }
void CBasePlayer::SetSuitUpdate(char *name, int fgroup, float fNoRepeatTime) { }

//MODDD - stub here too.
void CBasePlayer::SetSuitUpdate(char *name, int fgroup, float fNoRepeatTime, float playDuration) { }
//void CBasePlayer::broadThink(){}

void CBasePlayer::UpdatePlayerSound ( void ) { }
void CBasePlayer::PostThink() { }
void CBasePlayer::Precache( void ) { }
int CBasePlayer::Save( CSave &save ) { return 0; }
void CBasePlayer::RenewItems(void) { }
int CBasePlayer::Restore( CRestore &restore ) { return 0; }
void CBasePlayer::SelectNextItem( int iItem ) { }
BOOL CBasePlayer::HasWeapons( void ) { return FALSE; }
void CBasePlayer::SelectPrevItem( int iItem ) { }
//CBaseEntity *FindEntityForwardOLDVERSION( CBaseEntity *pMe ) { return NULL; }
//MODDD
CBaseEntity *FindEntityForward( CBasePlayer *pMe ) { return NULL; }
BOOL CBasePlayer::FlashlightIsOn( void ) { return FALSE; }
void CBasePlayer::FlashlightTurnOn( void ) { }
void CBasePlayer::FlashlightTurnOff( void ) { }
void CBasePlayer::ForceClientDllUpdate( void ) { }
void CBasePlayer::ImpulseCommands( ) { }
void CBasePlayer::CheatImpulseCommands( int iImpulse ) { }

void CBasePlayer::SendAmmoUpdate(void) { }
void CBasePlayer::UpdateClientData( void ) { }
BOOL CBasePlayer::FBecomeProne ( void ) { return TRUE; }
void CBasePlayer::BarnacleVictimBitten ( entvars_t *pevBarnacle ) { }
void CBasePlayer::BarnacleVictimReleased ( void ) { }
int CBasePlayer::Illumination( void ) { return 0; }
void CBasePlayer::EnableControl(BOOL fControl) { }
Vector CBasePlayer::GetAutoaimVector( float flDelta ) { return g_vecZero; }
Vector CBasePlayer::AutoaimDeflection( Vector &vecSrc, float flDist, float flDelta  ) { return g_vecZero; }
void CBasePlayer::ResetAutoaim( ) { }
void CBasePlayer::SetCustomDecalFrames( int nFrames ) { }
int CBasePlayer::GetCustomDecalFrames( void ) { return -1; }
void CBasePlayer::DropPlayerItem ( char *pszItemName ) { }

void CBasePlayer::SetGravity(float newGravityVal) {}



void ClearMultiDamage(void) { }
void ApplyMultiDamage(entvars_t *pevInflictor, entvars_t *pevAttacker ) { }
void AddMultiDamage( entvars_t *pevInflictor, CBaseEntity *pEntity, float flDamage, int bitsDamageType) { }
void AddMultiDamage(entvars_t* pevInflictor, CBaseEntity* pEntity, float flDamage, int bitsDamageType, int bitsDamageTypeMod) { }

//MODDD - NEW, ggrenade-related.
void SimpleStaticExplode(Vector rawExplodeOrigin, float rawDamage, CBaseEntity* pDamageDealer) { }
void SimpleStaticExplode(Vector rawExplodeOrigin, float rawDamage, CBaseEntity* pDamageDealer, entvars_t* entOwner) { }
void SimpleStaticExplode(Vector rawExplodeOrigin, float rawDamage, float flRange, CBaseEntity* pDamageDealer) { }
void SimpleStaticExplode(Vector rawExplodeOrigin, float rawDamage, float flRange, CBaseEntity* pDamageDealer, entvars_t* entOwner) { }

void StaticExplode(Vector rawExplodeOrigin, float rawDamage, CBaseEntity* pDamageDealer, entvars_t* entOwner, TraceResult* pTrace, int bitsDamageType) {}
void StaticExplode(Vector rawExplodeOrigin, float rawDamage, CBaseEntity* pDamageDealer, entvars_t* entOwner, TraceResult* pTrace, int bitsDamageType, int bitsDamageTypeMod) {}
void StaticExplode(Vector rawExplodeOrigin, float rawDamage, CBaseEntity* pDamageDealer, entvars_t* entOwner, TraceResult* pTrace, int bitsDamageType, int bitsDamageTypeMod, float shrapMod) {}
void StaticExplode(Vector rawExplodeOrigin, float rawDamage, float flRange, CBaseEntity* pDamageDealer, entvars_t* entOwner, TraceResult* pTrace, int bitsDamageType) {}
void StaticExplode(Vector rawExplodeOrigin, float rawDamage, float flRange, CBaseEntity* pDamageDealer, entvars_t* entOwner, TraceResult* pTrace, int bitsDamageType, int bitsDamageTypeMod) {}
void StaticExplode(Vector rawExplodeOrigin, float rawDamage, float flRange, CBaseEntity* pDamageDealer, entvars_t* entOwner, TraceResult* pTrace, int bitsDamageType, int bitsDamageTypeMod, float shrapMod) {}


void UTIL_SpawnBlood(const Vector& vecSpot, int bloodColor, int amount) { }
void UTIL_SpawnBlood(const Vector& vecSpot, const Vector& bloodDir, int bloodColor, int amount) {}

int DamageDecal( CBaseEntity *pEntity, int bitsDamageType ) { return 0; }
int DamageDecal( CBaseEntity *pEntity, int bitsDamageType, int bitsDamageTypeMod ) { return 0; }
void DecalGunshot( TraceResult *pTrace, int iBulletType ) { }
void EjectBrass ( const Vector &vecOrigin, const Vector &vecVelocity, float rotation, int model, int soundtype ) { }

//MODDD - removed here.  Now included server and clientside, no need to dummy it out here.
//void AddAmmoNameToAmmoRegistry( const char *szAmmoname ) { }

int CBasePlayerItem::Restore( class CRestore & ) { return 1; }
int CBasePlayerItem::Save( class CSave & ) { return 1; }
int CBasePlayerWeapon::Restore( class CRestore & ) { return 1; }
int CBasePlayerWeapon::Save( class CSave & ) { return 1; }

//MODDD
//No, not necessary.  Already has a counterpart, clientside, in hl_weapons.cpp that is more faithful to its normal clientside counterpart (SendWeaponAnim).
//void CBasePlayerWeapon::SendWeaponAnimBypass(int iAnim, int body){}

void CBasePlayerItem::SetObjectCollisionBox( void ) { }
void CBasePlayerItem::FallInit( void ) { }
void CBasePlayerItem::FallThink ( void ) { }
void CBasePlayerItem::Materialize( void ) { }
void CBasePlayerItem::AttemptToMaterialize( void ) { }
void CBasePlayerItem::CheckRespawn ( void ) { }
CBaseEntity* CBasePlayerItem::Respawn( void ) { return NULL; }

//MODDD - new
void CBasePlayerItem::DefaultTouchRemoveThink( CBaseEntity *pOther) { }

void CBasePlayerItem::DefaultTouch( CBaseEntity *pOther ) { }
void CBasePlayerItem::DestroyItem( void ) { }
BOOL CBasePlayerItem::AddToPlayer( CBasePlayer *pPlayer ) { return TRUE; }
void CBasePlayerItem::Drop( void ) { }
void CBasePlayerItem::Kill( void ) { }
void CBasePlayerItem::Holster( int skiplocal ) { }
void CBasePlayerItem::AttachToPlayer ( CBasePlayer *pPlayer ) { }
BOOL CBasePlayerWeapon::AddDuplicate( CBasePlayerItem *pOriginal ) { return 0; }
BOOL CBasePlayerWeapon::AddToPlayer( CBasePlayer *pPlayer ) { return FALSE; }
int CBasePlayerWeapon::UpdateClientData( CBasePlayer *pPlayer ) { return 0; }
BOOL CBasePlayerWeapon::AddPrimaryAmmo( int iCount, char *szName, int iMaxClip, int iMaxCarry ) { return TRUE; }
//MODDD
BOOL CBasePlayerWeapon::AddPrimaryAmmo( int iCount, char *szName, int iMaxClip, int iMaxCarry, int forcePickupSound ) { return TRUE; }

BOOL CBasePlayerWeapon::AddSecondaryAmmo( int iCount, char *szName, int iMax ) { return TRUE; }
BOOL CBasePlayerWeapon::IsUseable( void ) { return TRUE; }
void CBasePlayerAmmo::Spawn( void ) { }
CBaseEntity* CBasePlayerAmmo::Respawn( void ) { return this; }
void CBasePlayerAmmo::Materialize( void ) { }
void CBasePlayerAmmo::DefaultTouch( CBaseEntity *pOther ) { }
BOOL CBasePlayerWeapon::ExtractAmmo( CBasePlayerWeapon *pWeapon ) { return 0; }
BOOL CBasePlayerWeapon::ExtractClipAmmo( CBasePlayerWeapon *pWeapon ) { return 0; }
void CBasePlayerWeapon::RetireWeapon( void ) { }


//MODDD - new
CBaseEntity* CBasePlayerWeapon::pickupWalkerReplaceCheck(void){return 0;}
const char* CBasePlayerWeapon::GetPickupWalkerName(void){return 0;}
//MODDD - new event, called alongside a reload changing the ammo counts.
void CBasePlayerWeapon::OnReloadApply(void) {}
void CBasePlayerWeapon::OnAddPrimaryAmmoAsNewWeapon(void) {}





//MODDD - edited.
//void CSoundEnt::InsertSound ( int iType, const Vector &vecOrigin, int iVolume, float flDuration ) {}
CSound* CSoundEnt::InsertSound ( int iType, const Vector &vecOrigin, int iVolume, float flDuration ) {return NULL;}
//MODDD - new
void CSound::setCanListenCheck(  BOOL (* arg_canListenHandle)(CBaseEntity* pOther) ){return;}



//MODDD - new
BOOL getGermanModelsAllowed(void){return FALSE;}
//NEW... weird.
//CRpgRocket::CRpgRocket(void){return;}
	



// util_debugdraw.h methods, dummied.
///////////////////////////////////////////////////////////////////////////////////////////////////////
class CPathTrack;

void DebugLine_ClearLine(int argID) {}

void DebugLine_ClearAll(void) {}
void DebugLine_RenderAll(void) {}

void DebugLine_Setup(Vector vecStart, Vector vecEnd, float fraction, int successR = 0, int successG = 255, int successB = 0, int failR = 255, int failG = 0, int failB = 0) {}
void DebugLine_Setup(int argID, Vector vecStart, Vector vecEnd, float fraction, int successR = 0, int successG = 255, int successB = 0, int failR = 255, int failG = 0, int failB = 0) {}

void DebugLine_Setup(Vector vecStart, Vector vecEnd, int r, int g, int b) {}
void DebugLine_Setup(int argID, Vector vecStart, Vector vecEnd, int r, int g, int b) {}

void DebugLine_SetupPoint(Vector vecPoint, int r, int g, int b) {}
void DebugLine_SetupPoint(int argID, Vector vecPoint, int r, int g, int b) {}

void DebugLine_SetupPathTrack(CPathTrack* argPathTrack) {}
void DebugLine_SetupPathTrack(int argID, CPathTrack* argPathTrack) {}

void DebugLine_Color(int argID, int r, int g, int b) {}
void DebugLine_ColorSuccess(int argID) {}
void DebugLine_ColorFail(int argID) {}
void DebugLine_ColorBool(int argID, BOOL argPass) {}
///////////////////////////////////////////////////////////////////////////////////////////////////////


