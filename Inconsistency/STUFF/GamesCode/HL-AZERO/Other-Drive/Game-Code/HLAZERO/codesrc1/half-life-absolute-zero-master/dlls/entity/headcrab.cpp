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
//=========================================================
// headcrab.cpp - tiny, jumpy alien parasite
//=========================================================



// TODO - try to pathfind (TRACE_MONSTER_HULL) and see if a jump from Here to THere looks promising
// before picking the leap on SCHED_RANGE_ATTACK1 or whatever.
// If it isn't, add a delay like 'recentPreLeapTraceFail = gpGlobals->time + 3'
// and during that time CheckRangeAttack2 returns 'false', to try other follow methods instead.
// Watch out if it has false-positives though, it would look dumb to fail when it should have passed.
// Done, I think.


#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "basemonster.h"
#include "schedule.h"
#include "game.h"

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define HC_AE_JUMPATTACK	( 2 )



/*
// is it really worth filling this out?
enum headcrab_sequence {  //key: frames, FPS
	SEQ_HEADCRAB_...
};
*/




// none needed yet
/*
enum
{
	SCHED_HEADCRAB_??? = LAST_COMMON_SCHEDULE + 1,
	SCHED_HEADCRAB_???
};
*/

enum
{
	TASK_HEADCRAB_WAIT_FOR_LEAP = LAST_COMMON_TASK + 1,
	//TASK_HEADCRAB_...
};




class CHeadCrab : public CBaseMonster
{
public:

	static const char *pIdleSounds[];
	static const char *pAlertSounds[];
	static const char *pPainSounds[];
	static const char *pAttackSounds[];
	static const char *pDeathSounds[];
	static const char *pBiteSounds[];

	float leapStartTime;
	BOOL leapEarlyFail;
	float nextLeapAllowedTime;
	float recentPreLeapTraceFail;

	//MODDD
	CHeadCrab(void);

	void Spawn( void );
	void Precache( void );
	void RunTask ( Task_t *pTask );
	void StartTask ( Task_t *pTask );
	void SetYawSpeed ( void );
	void EXPORT LeapTouch ( CBaseEntity *pOther );
	Vector Center( void );
	Vector BodyTarget( const Vector &posSrc );
	//MODDD
	Vector BodyTargetMod( const Vector &posSrc );
	void PainSound( void );
	void DeathSound( void );
	void IdleSound( void );
	void AlertSound( void );
	void PrescheduleThink( void );
	int  Classify ( void );
	void HandleAnimEvent( MonsterEvent_t *pEvent );

	BOOL CheckMeleeAttack1(float flDot, float flDist);
	BOOL CheckMeleeAttack2(float flDot, float flDist);
	BOOL CheckRangeAttack1 ( float flDot, float flDist );
	BOOL CheckRangeAttack2 ( float flDot, float flDist );

	void StartLeap(void);

	//MODDD - IMPORTANT. Make these virtual if child-classes (CBabyHeadcrab) need to override these.
	GENERATE_TRACEATTACK_PROTOTYPE
	GENERATE_TAKEDAMAGE_PROTOTYPE


	virtual float GetDamageAmount( void ) { return gSkillData.headcrabDmgBite; }
	virtual int GetVoicePitch( void ) { return 100; }
	virtual float GetSoundVolue( void ) { return 1.0; }
	Schedule_t* GetScheduleOfType ( int Type );


	//MODD - NEW.  advanced anim stuff.
	BOOL getMonsterBlockIdleAutoUpdate(void);
	BOOL forceIdleFrameReset(void);
	BOOL canPredictActRepeat(void);
	BOOL usesAdvancedAnimSystem(void);

	void SetActivity(Activity NewActivity);

	int tryActivitySubstitute(int activity);
	int LookupActivityHard(int activity);

	void HandleEventQueueEvent(int arg_eventID);
	///////////////////////////////////////////////

	BOOL SeeThroughWaterLine(void);


	CUSTOM_SCHEDULES;

};


#if REMOVE_ORIGINAL_NAMES != 1
	LINK_ENTITY_TO_CLASS( monster_headcrab, CHeadCrab );
#endif

#if EXTRA_NAMES > 0
	LINK_ENTITY_TO_CLASS( headcrab, CHeadCrab );
	
	//no extras.

#endif


	


	




Task_t	tlHCRangeAttack1[] =
{
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_FACE_IDEAL,			(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE	},
	{ TASK_FACE_IDEAL,			(float)0		},
	//MODDD - removed, already an enforced cooldown between jumps
	//{ TASK_WAIT_RANDOM,			(float)0.5		},
};

Schedule_t	slHCRangeAttack1[] =
{
	{ 
		tlHCRangeAttack1,
		ARRAYSIZE ( tlHCRangeAttack1 ), 
		bits_COND_ENEMY_OCCLUDED	|
		bits_COND_NO_AMMO_LOADED,
		0,
		"HCRangeAttack1"
	},
};

Task_t	tlHCRangeAttack1Fast[] =
{
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_FACE_IDEAL,			(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE	},
};

Schedule_t	slHCRangeAttack1Fast[] =
{
	{ 
		tlHCRangeAttack1Fast,
		ARRAYSIZE ( tlHCRangeAttack1Fast ), 
		bits_COND_ENEMY_OCCLUDED	|
		bits_COND_NO_AMMO_LOADED,
		0,
		"HCRAFast"
	},
};




//MODDD - NEW.  Pick this schedule if it is possible to leap but
// the leap-cooldown says not to.  This finishes as soon as it is possible to,
// or the range-attack condition expires for whatever reason.  Or the enemy is
// occluded which should cause that anyway.
Task_t	tlHCWaitForAttack[] =
{
	{ TASK_STOP_MOVING, (float)0 },
	{ TASK_HEADCRAB_WAIT_FOR_LEAP, (float)0 },
};

Schedule_t	slHCWaitForAttack[] =
{
	{
		tlHCWaitForAttack,
		ARRAYSIZE(tlHCWaitForAttack),
		bits_COND_ENEMY_OCCLUDED | bits_COND_NEW_ENEMY,
		0,
		"HCWaitForAttk"
	},
};











DEFINE_CUSTOM_SCHEDULES( CHeadCrab )
{
	slHCRangeAttack1,
	slHCRangeAttack1Fast,
	slHCWaitForAttack
};

IMPLEMENT_CUSTOM_SCHEDULES( CHeadCrab, CBaseMonster );

const char *CHeadCrab::pIdleSounds[] = 
{
	"headcrab/hc_idle1.wav",
	"headcrab/hc_idle2.wav",
	"headcrab/hc_idle3.wav",
};
const char *CHeadCrab::pAlertSounds[] = 
{
	"headcrab/hc_alert1.wav",
};
const char *CHeadCrab::pPainSounds[] = 
{
	"headcrab/hc_pain1.wav",
	"headcrab/hc_pain2.wav",
	"headcrab/hc_pain3.wav",
};
const char *CHeadCrab::pAttackSounds[] = 
{
	"headcrab/hc_attack1.wav",
	"headcrab/hc_attack2.wav",
	"headcrab/hc_attack3.wav",
};

const char *CHeadCrab::pDeathSounds[] = 
{
	"headcrab/hc_die1.wav",
	"headcrab/hc_die2.wav",
};

const char *CHeadCrab::pBiteSounds[] = 
{
	"headcrab/hc_headbite.wav",
};




CHeadCrab::CHeadCrab(void) {
	leapStartTime = -1;
	leapEarlyFail = FALSE;
	nextLeapAllowedTime = -1;
	recentPreLeapTraceFail = -1;

}


//=========================================================
// Classify - indicates this monster's place in the 
// relationship table.
//=========================================================
int CHeadCrab::Classify ( void )
{
	return	CLASS_ALIEN_PREY;
}

//=========================================================
// Center - returns the real center of the headcrab.  The 
// bounding box is much larger than the actual creature so 
// this is needed for targeting
//=========================================================
Vector CHeadCrab::Center ( void )
{
	return Vector( pev->origin.x, pev->origin.y, pev->origin.z + 6 );
}


Vector CHeadCrab::BodyTarget( const Vector &posSrc ) 
{ 
	return Center( );
}

//MODDD
Vector CHeadCrab::BodyTargetMod( const Vector &posSrc ) 
{ 
	return Center( );
}


//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CHeadCrab::SetYawSpeed ( void )
{
	//MODDD - overal sped up a bit, especially at higher difficulties.
	float turnSpeedMulti;
	if (g_iSkillLevel == SKILL_MEDIUM) {
		turnSpeedMulti = 1.8;
	}
	else if (g_iSkillLevel == SKILL_HARD) {
		turnSpeedMulti = 2.3;
	}
	else {
		// easy?
		turnSpeedMulti = 1.4;
	}

	int ys;

	switch ( m_Activity )
	{
	case ACT_IDLE:			
		ys = 30;
		break;
	case ACT_RUN:			
	case ACT_WALK:			
		ys = 20;
		break;
	case ACT_TURN_LEFT:
	case ACT_TURN_RIGHT:
		ys = 60;
		break;
	case ACT_RANGE_ATTACK1:	
		ys = 30;
		break;
	default:
		ys = 30;
		break;
	}

	pev->yaw_speed = ys * turnSpeedMulti;
}



//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CHeadCrab::HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch( pEvent->event )
	{
		case HC_AE_JUMPATTACK:
		{
			//MODDD - begin with angles forced to ideal, safety.
			SetAngleY(pev->ideal_yaw);

			StartLeap();
			// TEST

			if (g_iSkillLevel == SKILL_HARD) {
				nextLeapAllowedTime = gpGlobals->time + RANDOM_FLOAT(0.8, 1.3);
			}
			else if (g_iSkillLevel == SKILL_MEDIUM) {
				nextLeapAllowedTime = gpGlobals->time + RANDOM_FLOAT(1.1, 1.9);
			}
			else {
				// easy?
				nextLeapAllowedTime = gpGlobals->time + RANDOM_FLOAT(1.5, 2.5);
			}

			//nextLeapAllowedTime = gpGlobals->time + 5.0f;
			
			//MODDD - oh look, the variable we nor CBaseMonster ever uses.  Goodbye.
			//m_flNextAttack = gpGlobals->time + 2;
		}
		break;

		default:
			CBaseMonster::HandleAnimEvent( pEvent );
			break;
	}
}

//=========================================================
// Spawn
//=========================================================
void CHeadCrab::Spawn()
{
	Precache( );

	SET_MODEL(ENT(pev), "models/headcrab.mdl");
	UTIL_SetSize(pev, Vector(-12, -12, 0), Vector(12, 12, 24));

	pev->classname = MAKE_STRING("monster_headcrab");

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_GREEN;
	pev->effects		= 0;
	pev->health			= gSkillData.headcrabHealth;
	pev->view_ofs		= Vector ( 0, 0, 20 );// position of the eyes relative to monster's origin.
	pev->yaw_speed		= 5;//!!! should we put this in the monster's changeanim function since turn rates may vary with state/anim?
	m_flFieldOfView		= 0.5;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState		= MONSTERSTATE_NONE;

	MonsterInit();


	//pev->renderfx |= DONOTDRAWSHADOW;

}

extern int global_useSentenceSave;

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CHeadCrab::Precache()
{
	global_useSentenceSave = TRUE;
	PRECACHE_SOUND_ARRAY(pIdleSounds);
	PRECACHE_SOUND_ARRAY(pAlertSounds);
	PRECACHE_SOUND_ARRAY(pPainSounds);
	PRECACHE_SOUND_ARRAY(pAttackSounds);
	PRECACHE_SOUND_ARRAY(pDeathSounds);
	PRECACHE_SOUND_ARRAY(pBiteSounds);

	PRECACHE_MODEL("models/headcrab.mdl");

	global_useSentenceSave = FALSE;
}	


//=========================================================
// RunTask 
//=========================================================
void CHeadCrab::RunTask ( Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_RANGE_ATTACK1:
	case TASK_RANGE_ATTACK2:
		{
			if ( m_fSequenceFinished )
			{
				TaskComplete();
				SetTouch( NULL );
				m_IdealActivity = ACT_IDLE;
			}
			break;
		}
	//MODDD - NEW.
	case TASK_HEADCRAB_WAIT_FOR_LEAP:
	{

		if (gpGlobals->time >= nextLeapAllowedTime) {
			// move on!
			TaskComplete(); 
			break;
		}
		else {

			if (!HasConditions(bits_COND_CAN_RANGE_ATTACK1)) {
				// no longer able to range-attack?  Stop, pick any other schedule.
				TaskComplete();
				break;
			}

		}

		// In the meantime, try facing ideal angles.  Likely have some by this point, or so
		// assumes the orginary attack method anyway.

		if (!FacingIdeal()) {
			// back to turning, or keep at it
			SetTurnActivity();
			ChangeYaw(pev->yaw_speed);
		}
		else {
			if (m_IdealActivity == ACT_TURN_LEFT || m_IdealActivity == ACT_TURN_RIGHT) {
				// stop turning then.
				m_IdealActivity = ACT_IDLE;
			}
		}

		//if (FacingIdeal())
		//{
		//	TaskComplete();
		//}
	

		break;
	}
	default:
		{
			CBaseMonster::RunTask(pTask);
		}
	}
}

//=========================================================
// LeapTouch - this is the headcrab's touch function when it
// is in the air
//=========================================================
void CHeadCrab::LeapTouch ( CBaseEntity *pOther )
{
	if ( !pOther->pev->takedamage )
	{
		//MODDD - NEW.  On a jump failing very early, increase the delay moreso next time.
		if (!leapEarlyFail && ((gpGlobals->time - leapStartTime) < 0.3) ) {
			leapEarlyFail = TRUE;
			nextLeapAllowedTime = gpGlobals->time + RANDOM_FLOAT(2.6, 3.6);
		}
		return;
	}

	//MODDD - added a "isHated" check that may force a monster to be hated anyways.
	//if ( pOther->Classify() == Classify() )
	if( !pOther->isForceHated(this) && pOther->Classify() == Classify())
	{
		return;
	}

	// Don't hit if back on ground
	if ( !FBitSet( pev->flags, FL_ONGROUND ) )
	{
		UTIL_PlaySound( edict(), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pBiteSounds), GetSoundVolue(), ATTN_IDLE, 0, GetVoicePitch() );
		

		//MODDD - send TraceAttack_Traceless too for things building cumulative damage to see it anyway.
		// the direction sent isn't paid attention too in that case, debating whether to even have it available there at all.
		pOther->TraceAttack_Traceless(pev, GetDamageAmount(), g_vecZero, DMG_SLASH, 0);

		pOther->TakeDamage( pev, pev, GetDamageAmount(), DMG_SLASH, 0 );
		UTIL_fromToBlood(this, pOther, GetDamageAmount());

		//CheckTraceHullAttack(256, 60, DMG_SLASH);
	}

	SetTouch( NULL );
}

//=========================================================
// PrescheduleThink
//=========================================================
void CHeadCrab::PrescheduleThink ( void )
{
	// make the crab coo a little bit in combat state
	if ( m_MonsterState == MONSTERSTATE_COMBAT && RANDOM_FLOAT( 0, 5 ) < 0.1 )
	{
		IdleSound();
	}

}



void CHeadCrab::StartTask ( Task_t *pTask )
{

	switch ( pTask->iTask )
	{
	//MODDD - NEW.  Copy/paste of TASK_FACE_IDEAL, but able to be interrupted by the leap-cooldown expiring (can attack),
	// or no longer being able to attack since it was called for.
	case TASK_HEADCRAB_WAIT_FOR_LEAP:
	{
		if (FacingIdeal())
		{
			TaskComplete();
			return;
		}

		SetTurnActivity();
		break;
	}

	case TASK_RANGE_ATTACK1:
		{
			UTIL_PlaySound( edict(), CHAN_WEAPON, pAttackSounds[0], GetSoundVolue(), ATTN_IDLE, 0, GetVoicePitch() );
			m_IdealActivity = ACT_RANGE_ATTACK1;
			SetTouch ( &CHeadCrab::LeapTouch );
			break;
		}
	default:
		{
			CBaseMonster::StartTask( pTask );
		}
	}
}



BOOL CHeadCrab::CheckMeleeAttack1(float flDot, float flDist) {
	return FALSE;
}
BOOL CHeadCrab::CheckMeleeAttack2(float flDot, float flDist) {
	return FALSE;
}


//=========================================================
// CheckRangeAttack1
//=========================================================
BOOL CHeadCrab::CheckRangeAttack1 ( float flDot, float flDist )
{

	if(gpGlobals->time >= recentPreLeapTraceFail){
		// acceptable
	}else {
		// nope.
		return FALSE;
	}

	//MODDD - distance now depends on difficulty, retail was 256.
	// And with faster turn-rates, flDot requirement tightened below (was 0.65)
	float jumpAllowRange;
	if (g_iSkillLevel == SKILL_HARD) {
		jumpAllowRange = 300;
	}
	else if (g_iSkillLevel == SKILL_MEDIUM) {
		jumpAllowRange = 340;
	}
	else {
		jumpAllowRange = 390;
	}



	if ( FBitSet( pev->flags, FL_ONGROUND ) && flDist <= jumpAllowRange && flDot >= 0.72 )
	{

		return TRUE;
	}
	return FALSE;
}

//=========================================================
// CheckRangeAttack2
//=========================================================
BOOL CHeadCrab::CheckRangeAttack2 ( float flDot, float flDist )
{
	return FALSE;
	// BUGBUG: Why is this code here?  There is no ACT_RANGE_ATTACK2 animation.  I've disabled it for now.
#if 0
	if ( FBitSet( pev->flags, FL_ONGROUND ) && flDist > 64 && flDist <= 256 && flDot >= 0.5 )
	{
		return TRUE;
	}
	return FALSE;
#endif
}




void CHeadCrab::StartLeap(void) {
	ClearBits(pev->flags, FL_ONGROUND);

	//MODDD - isn't this supposed to be here too?
	pev->groundentity = NULL;
	// Also setting these.
	leapStartTime = gpGlobals->time;
	leapEarlyFail = FALSE;
	////////////////////////////////////

	UTIL_SetOrigin(pev, pev->origin + Vector(0, 0, 1));// take him off ground so engine doesn't instantly reset onground 
	UTIL_MakeVectors(pev->angles);

	Vector vecJumpDir;
	if (m_hEnemy != NULL)
	{
		float gravity = g_psv_gravity->value;
		if (gravity <= 1)
			gravity = 1;

		// How fast does the headcrab need to travel to reach that height given gravity?
		float height = (m_hEnemy->pev->origin.z + m_hEnemy->pev->view_ofs.z - pev->origin.z);
		if (height < 16)
			height = 16;
		float speed = sqrt(2 * gravity * height);
		float time = speed / gravity;

		// Scale the sideways velocity to get there at the right time
		vecJumpDir = (m_hEnemy->pev->origin + m_hEnemy->pev->view_ofs - pev->origin);
		vecJumpDir = vecJumpDir * (1.0 / time);

		// Speed to offset gravity at the desired height
		vecJumpDir.z = speed;

		// Don't jump too far/fast
		float distance = vecJumpDir.Length();

		if (distance > 650)
		{
			vecJumpDir = vecJumpDir * (650.0 / distance);
		}
	}
	else
	{
		// jump hop, don't care where
		vecJumpDir = Vector(gpGlobals->v_forward.x, gpGlobals->v_forward.y, gpGlobals->v_up.z) * 350;
	}

	int iSound = RANDOM_LONG(0, 2);
	if (iSound != 0)
		UTIL_PlaySound(edict(), CHAN_VOICE, pAttackSounds[iSound], GetSoundVolue(), ATTN_IDLE, 0, GetVoicePitch());

	pev->velocity = vecJumpDir;

}



GENERATE_TRACEATTACK_IMPLEMENTATION(CHeadCrab)
{
	GENERATE_TRACEATTACK_PARENT_CALL(CBaseMonster);
}
GENERATE_TAKEDAMAGE_IMPLEMENTATION(CHeadCrab)
{
	// Don't take any acid damage -- BigMomma's mortar is acid

	//MODDD - TODO: create a better way of immunity to DMG_ACID.
	//...nevermind, this should be okay.
	if ( bitsDamageType & DMG_ACID )
		flDamage = 0;

	//MODDD - ADDITION - this way, we don't trigger the timed damage for Acid either.
	bitsDamageType &= (~DMG_ACID);

	/*
	if(pevInflictor && FClassnameIs(pevInflictor, "monster_bigmomma")){

	}
	*/

	return GENERATE_TAKEDAMAGE_PARENT_CALL(CBaseMonster);
}

//=========================================================
// IdleSound
//=========================================================
#define CRAB_ATTN_IDLE (float)1.5
void CHeadCrab::IdleSound ( void )
{
	UTIL_PlaySound( edict(), CHAN_VOICE, RANDOM_SOUND_ARRAY(pIdleSounds), GetSoundVolue(), ATTN_IDLE, 0, GetVoicePitch() );
}

//=========================================================
// AlertSound 
//=========================================================
void CHeadCrab::AlertSound ( void )
{
	UTIL_PlaySound( edict(), CHAN_VOICE, RANDOM_SOUND_ARRAY(pAlertSounds), GetSoundVolue(), ATTN_IDLE, 0, GetVoicePitch() );
}

//=========================================================
// AlertSound 
//=========================================================
void CHeadCrab::PainSound ( void )
{
	UTIL_PlaySound( edict(), CHAN_VOICE, RANDOM_SOUND_ARRAY(pPainSounds), GetSoundVolue(), ATTN_IDLE, 0, GetVoicePitch() );
}

//=========================================================
// DeathSound 
//=========================================================
void CHeadCrab::DeathSound ( void )
{
	UTIL_PlaySound( edict(), CHAN_VOICE, RANDOM_SOUND_ARRAY(pDeathSounds), GetSoundVolue(), ATTN_IDLE, 0, GetVoicePitch() );
}

Schedule_t* CHeadCrab::GetScheduleOfType ( int Type )
{
	switch	( Type )
	{
		case SCHED_RANGE_ATTACK1:
		{
			//MODDD - don't go straight to a leap if it's been too soon.
			if (gpGlobals->time >= nextLeapAllowedTime) {
				
				// False positives?  Yay!  This can thoroughly burn in hell.
				/*
				//MODDD - one more check.
				// Is there a clear path from me to them for the leap?
				if (m_hEnemy) {

					BOOL passPreTrace = FALSE;
					Vector traceStart = Center();
					TraceResult tr;
					TRACE_MONSTER_HULL(edict(), traceStart, m_hEnemy->BodyTargetMod(traceStart), dont_ignore_monsters, edict(), &tr);

					if (tr.fAllSolid) {
						// what
					}else if (tr.flFraction >= 1.0) {
						// proceed.
						passPreTrace = TRUE;
					}
					else {
						// wha..?
						if (tr.pHit != NULL) {
							CBaseEntity* getIt = CBaseEntity::Instance(tr.pHit);
							if (getIt != NULL) {

								if (getIt->edict() == m_hEnemy.Get()) {
									// hey, worth a shot.
									passPreTrace = TRUE;
								}

								CBaseMonster* monTest = getIt->GetMonsterPointer();
								if (monTest != NULL) {
									if (monTest->Classify() > R_NO) {
										// ok, go ahead anyway then.
										passPreTrace = TRUE;
									}
								}
							}

						}
					}

					if (!passPreTrace) {
						// fail?  Let the rest know.
						recentPreLeapTraceFail = gpGlobals->time + 3;

						// Try getting another schedule now that range-attack is blocked
						ClearConditions(bits_COND_CAN_RANGE_ATTACK1);
						return GetSchedule();
					}
				}//END OF enemy check
				*/

				return &slHCRangeAttack1[0];
			}else {
				return &slHCWaitForAttack[0];
			}
		}
		break;
	}

	return CBaseMonster::GetScheduleOfType( Type );
}



//MODDD - NEW.  advanced anim stuff


BOOL CHeadCrab::getMonsterBlockIdleAutoUpdate(void) {
	return FALSE;
}
BOOL CHeadCrab::forceIdleFrameReset(void) {
	return FALSE;
}
// Whether this monster can re-pick the same animation before the next frame starts if it anticipates it getting picked.
// This is subtly retail behavior, but may not always play well.
BOOL CHeadCrab::canPredictActRepeat(void) {
	return TRUE;
}
BOOL CHeadCrab::usesAdvancedAnimSystem(void) {
	return TRUE;
}
void CHeadCrab::SetActivity(Activity NewActivity) {
	CBaseMonster::SetActivity(NewActivity);
}




int CHeadCrab::tryActivitySubstitute(int activity) {
	int i = 0;

	// no need for default, just falls back to the normal activity lookup.
	switch (activity) {
		case ACT_WALK: {


		break; }
		case ACT_RUN: {
			if (g_iSkillLevel == SKILL_HARD) {
				return LookupSequence("run2");
			}

		break; }
	}//END OF switch


	// not handled by above? Rely on the model's anim for this activity if there is one.
	return CBaseAnimating::LookupActivity(activity);
}//END OF tryActivitySubstitute

int CHeadCrab::LookupActivityHard(int activity) {
	int i = 0;
	m_flFramerateSuggestion = 1;
	pev->framerate = 1;
	// any animation events in progress?  Clear it.
	resetEventQueue();

	//no need for default, just falls back to the normal activity lookup.
	switch (activity) {
		case ACT_WALK: {


		break; }
		case ACT_RUN: {
			if (g_iSkillLevel == SKILL_HARD) {
				return LookupSequence("run2");
			}

		break; }
	}//END OF switch

	// not handled by above?  try the real deal.
	return CBaseAnimating::LookupActivity(activity);
}//END OF LookupActivityHard



// Handles custom events sent from "LookupActivityHard", which sends events as timed delays along with picking an animation in script.
// So this handles script-provided events, not model ones.
void CHeadCrab::HandleEventQueueEvent(int arg_eventID) {

	switch (arg_eventID) {
	case 0: {


		break; }
	case 1: {


		break; }
	}//END OF switch

}//END OF HandleEventQueueEvent



 // If a headcrab lands in the water, go ahead and let it target something to jump out at.  Kinda useless stuck in there.
BOOL CHeadCrab::SeeThroughWaterLine(void){
	return TRUE;
}//END OF SeeThroughWaterLine







///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////





class CBabyCrab : public CHeadCrab
{
public:
	CBabyCrab();

	void Spawn( void );
	void Precache( void );
	void SetYawSpeed ( void );
	float GetDamageAmount( void ) { return gSkillData.headcrabDmgBite * 0.3; }
	BOOL CheckRangeAttack1 ( float flDot, float flDist );
	Schedule_t* GetScheduleOfType ( int Type );
	virtual int GetVoicePitch( void ) { return PITCH_NORM + RANDOM_LONG(40,50); }
	virtual float GetSoundVolue( void ) { return 0.8; }
};


#if REMOVE_ORIGINAL_NAMES != 1
	LINK_ENTITY_TO_CLASS( monster_babycrab, CBabyCrab );
#endif

#if EXTRA_NAMES > 0
	LINK_ENTITY_TO_CLASS( babycrab, CBabyCrab );
	
	#if EXTRA_NAMES == 2
		LINK_ENTITY_TO_CLASS( babyheadcrab, CBabyCrab );
		LINK_ENTITY_TO_CLASS( monster_babyheadcrab, CBabyCrab );
		LINK_ENTITY_TO_CLASS( headcrab_baby, CBabyCrab );
		LINK_ENTITY_TO_CLASS( monster_headcrab_baby, CBabyCrab );
	#endif

#endif

CBabyCrab::CBabyCrab(){

}

void CBabyCrab::Spawn( void )
{
	CHeadCrab::Spawn();
	SET_MODEL(ENT(pev), "models/baby_headcrab.mdl");

	pev->classname = MAKE_STRING("monster_babycrab");

	pev->rendermode = kRenderTransTexture;
	pev->renderamt = 192;
	UTIL_SetSize(pev, Vector(-12, -12, 0), Vector(12, 12, 24));
	
	pev->health	= gSkillData.headcrabHealth * 0.25;	// less health than full grown
}

void CBabyCrab::Precache( void )
{
	PRECACHE_MODEL( "models/baby_headcrab.mdl" );
	CHeadCrab::Precache();
}


void CBabyCrab::SetYawSpeed ( void )
{
	pev->yaw_speed = 120;
}


BOOL CBabyCrab::CheckRangeAttack1( float flDot, float flDist )
{
	if ( pev->flags & FL_ONGROUND )
	{
		if ( pev->groundentity && (pev->groundentity->v.flags & (FL_CLIENT|FL_MONSTER)) )
			return TRUE;

		// A little less accurate, but jump from closer
		if ( flDist <= 180 && flDot >= 0.55 )
			return TRUE;
	}

	return FALSE;
}


Schedule_t* CBabyCrab::GetScheduleOfType ( int Type )
{
	switch( Type )
	{
		case SCHED_FAIL:	// If you fail, try to jump!
			if (m_hEnemy != NULL) {
				//MODDD - also only from the allowed delay
				//return slHCRangeAttack1Fast;
				if (gpGlobals->time >= nextLeapAllowedTime) {
					return slHCRangeAttack1Fast;
				}
				else {
					return &slHCWaitForAttack[0];
				}
			}
		break;

		case SCHED_RANGE_ATTACK1:
		{
			//MODDD - don't go straight to a leap if it's been too soon.
			if (gpGlobals->time >= nextLeapAllowedTime) {
				return slHCRangeAttack1Fast;
			}
			else {
				return &slHCWaitForAttack[0];
			}
		}
		break;
	}

	return CHeadCrab::GetScheduleOfType( Type );
}


