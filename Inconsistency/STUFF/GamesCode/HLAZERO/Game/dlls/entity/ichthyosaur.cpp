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


#pragma once

//=========================================================
// icthyosaur - evin, satan fish monster
//=========================================================


//TODO - in primaryAttack shcedule, sometimes gets hung up in
// TASK_FACE_ENEMY.  Maybe facing its ideal dart position
// is a better metric in this case?
// ...has this already been fixed a while ago? I forget




#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "basemonster.h"
#include "schedule.h"
#include "flyingmonster.h"
#include "nodes.h"
#include "soundent.h"
#include "util_model.h"
#include "effects.h"
#include "weapons.h"
#include "defaultai.h"  //why not?


extern CGraph WorldGraph;


#define SEARCH_RETRY 16

#define ICHTHYOSAUR_SPEED 150

//MODDD - looks like the eye idea was ditched, this is not on the model.
#define EYE_MAD		0
#define EYE_BASE	1
#define EYE_CLOSED	2
#define EYE_BACK	3
#define EYE_LOOK	4


#define ICHTHYOSAUR_AE_SHAKE_RIGHT 1
#define ICHTHYOSAUR_AE_SHAKE_LEFT 2


#define EMIT_ICKY_SOUND( chan, array ) \
	UTIL_PlaySound( ENT(pev), chan , array [ RANDOM_LONG(0,ARRAYSIZE( array )-1) ], 1.0, 0.6, 0, RANDOM_LONG(95,105) ); 

#define PROBE_LENGTH 150




//=========================================================
// Monster's Anim Events Go Here
//=========================================================

// UNDONE: Save/restore here
class CIchthyosaur : public CFlyingMonster
{
public:
	CIchthyosaur();

	void  Spawn( void );
	void  Precache( void );
	void  SetYawSpeed( void );
	int   Classify( void );
	void  HandleAnimEvent( MonsterEvent_t *pEvent );
	CUSTOM_SCHEDULES;

	int Save( CSave &save ); 
	int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];

	Schedule_t *GetSchedule( void );
	Schedule_t *GetScheduleOfType ( int Type );
	
	GENERATE_TRACEATTACK_PROTOTYPE
	GENERATE_TAKEDAMAGE_PROTOTYPE

	GENERATE_KILLED_PROTOTYPE
	
	void BecomeDead( void );

	void EXPORT CombatUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void EXPORT BiteTouch( CBaseEntity *pOther );

	void  StartTask( Task_t *pTask );
	void  RunTask( Task_t *pTask );

	BOOL  CheckMeleeAttack1 ( float flDot, float flDist );
	BOOL  CheckRangeAttack1 ( float flDot, float flDist );

	float ChangeYaw( int speed );
	Activity GetStoppedActivity( void );

	void  Move( float flInterval );
	void  MoveExecute( CBaseEntity *pTargetEnt, const Vector &vecDir, float flInterval );
	void  MonsterThink( void );
	void  Stop( void );
	void  Swim( void );
	Vector DoProbe(const Vector &Probe);

	float VectorToPitch( const Vector &vec);
	float FlPitchDiff( void );
	float ChangePitch( int speed );

	Vector m_SaveVelocity;
	float m_idealDist;

	float m_flBlink;

	//MODDD - yes
	float chaseEnemyAttackSoundCooldown;

	float m_flEnemyTouched;
	BOOL  m_bOnAttack;

	float m_flMaxSpeed;
	float m_flMinSpeed;
	float m_flMaxDist;


	//MODDD - not commented out? debug feature to see what direction its facing.
	//CBeam *m_pBeam;

	float m_flNextAlert;

	static const char *pIdleSounds[];
	static const char *pAlertSounds[];
	static const char *pAttackSounds[];
	static const char *pBiteSounds[];
	static const char *pDieSounds[];
	static const char *pPainSounds[];

	void IdleSound( void );
	void AlertSound( void );
	void AttackSound( void );
	void BiteSound( void );
	void DeathSound( void );
	void PainSound( void );
};



#if REMOVE_ORIGINAL_NAMES != 1
	LINK_ENTITY_TO_CLASS( monster_ichthyosaur, CIchthyosaur );
#endif

#if EXTRA_NAMES > 0
	LINK_ENTITY_TO_CLASS( ichthyosaur, CIchthyosaur );
	LINK_ENTITY_TO_CLASS( evilfish, CIchthyosaur );
	
	//no extras.
#endif


TYPEDESCRIPTION	CIchthyosaur::m_SaveData[] = 
{
	DEFINE_FIELD( CIchthyosaur, m_SaveVelocity, FIELD_VECTOR ),
	DEFINE_FIELD( CIchthyosaur, m_idealDist, FIELD_FLOAT ),
	DEFINE_FIELD( CIchthyosaur, m_flBlink, FIELD_FLOAT ),
	DEFINE_FIELD( CIchthyosaur, m_flEnemyTouched, FIELD_FLOAT ),
	DEFINE_FIELD( CIchthyosaur, m_bOnAttack, FIELD_BOOLEAN ),
	DEFINE_FIELD( CIchthyosaur, m_flMaxSpeed, FIELD_FLOAT ),
	DEFINE_FIELD( CIchthyosaur, m_flMinSpeed, FIELD_FLOAT ),
	DEFINE_FIELD( CIchthyosaur, m_flMaxDist, FIELD_FLOAT ),
	DEFINE_FIELD( CIchthyosaur, m_flNextAlert, FIELD_TIME ),
};

IMPLEMENT_SAVERESTORE( CIchthyosaur, CFlyingMonster );


const char *CIchthyosaur::pIdleSounds[] = 
{
	"ichy/ichy_idle1.wav",
	"ichy/ichy_idle2.wav",
	"ichy/ichy_idle3.wav",
	"ichy/ichy_idle4.wav",
};

const char *CIchthyosaur::pAlertSounds[] = 
{
	"ichy/ichy_alert2.wav",
	"ichy/ichy_alert3.wav",
};

const char *CIchthyosaur::pAttackSounds[] = 
{
	"ichy/ichy_attack1.wav",
	"ichy/ichy_attack2.wav",
};

const char *CIchthyosaur::pBiteSounds[] = 
{
	"ichy/ichy_bite1.wav",
	"ichy/ichy_bite2.wav",
};

const char *CIchthyosaur::pPainSounds[] = 
{
	"ichy/ichy_pain2.wav",
	"ichy/ichy_pain3.wav",
	"ichy/ichy_pain5.wav",
};

const char *CIchthyosaur::pDieSounds[] = 
{
	"ichy/ichy_die2.wav",
	"ichy/ichy_die4.wav",
};

void CIchthyosaur::IdleSound( void )	
{ 
	EMIT_ICKY_SOUND( CHAN_VOICE, pIdleSounds ); 
}

void CIchthyosaur::AlertSound( void ) 
{ 
	EMIT_ICKY_SOUND( CHAN_VOICE, pAlertSounds ); 
}

void CIchthyosaur::AttackSound( void ) 
{ 
	EMIT_ICKY_SOUND( CHAN_VOICE, pAttackSounds );
}

void CIchthyosaur::BiteSound( void ) 
{ 
	EMIT_ICKY_SOUND( CHAN_WEAPON, pBiteSounds );
}

void CIchthyosaur::DeathSound( void ) 
{ 
	EMIT_ICKY_SOUND( CHAN_VOICE, pDieSounds ); 
}

void CIchthyosaur::PainSound( void )	
{ 
	EMIT_ICKY_SOUND( CHAN_VOICE, pPainSounds ); 
}

//=========================================================
// monster-specific tasks and states
//=========================================================
enum 
{
	TASK_ICHTHYOSAUR_CIRCLE_ENEMY = LAST_COMMON_TASK + 1,
	TASK_ICHTHYOSAUR_SWIM,
	//TASK_ICHTHYOSAUR_FLOAT,   //MODDD - changed to "TASK_WATER_DEAD_FLOAT', made more general. moved to schedule.h
};

//=========================================================
// AI Schedules Specific to this monster
//=========================================================

static Task_t	tlSwimAround[] =
{
	{ TASK_SET_ACTIVITY,			(float)ACT_WALK },
	{ TASK_ICHTHYOSAUR_SWIM,		0.0 },
};

static Schedule_t	slSwimAround[] =
{
	{ 
		tlSwimAround,
		ARRAYSIZE(tlSwimAround), 
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_SEE_ENEMY		|
		bits_COND_NEW_ENEMY		|
		bits_COND_HEAR_SOUND,
		bits_SOUND_PLAYER |
		bits_SOUND_COMBAT,
		"SwimAround"
	},
};

static Task_t	tlSwimAgitated[] =
{
	{ TASK_STOP_MOVING,				(float) 0 },
	{ TASK_SET_ACTIVITY,			(float)ACT_RUN },
	{ TASK_WAIT,					(float)2.0 },
};

static Schedule_t	slSwimAgitated[] =
{
	{ 
		tlSwimAgitated,
		ARRAYSIZE(tlSwimAgitated), 
		0, 
		0, 
		"SwimAgitated"
	},
};


static Task_t	tlCircleEnemy[] =
{
	{ TASK_SET_ACTIVITY,			(float)ACT_WALK },
	{ TASK_ICHTHYOSAUR_CIRCLE_ENEMY, 0.0 },
};

static Schedule_t	slCircleEnemy[] =
{
	{ 
		tlCircleEnemy,
		ARRAYSIZE(tlCircleEnemy), 
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_CAN_MELEE_ATTACK1 |
		bits_COND_CAN_RANGE_ATTACK1,
		0,
		"CircleEnemy"
	},
};


//MODDD - "slTwitchDie" changed to "slDieWaterFloat". And moved to defaultai.cpp
//        Setting a monster's own animation for floating is still necessary for the start of TASK_WATER_DEAD_FLOAT though.



DEFINE_CUSTOM_SCHEDULES(CIchthyosaur)
{
    slSwimAround,
	slSwimAgitated,
	slCircleEnemy,
	//slTwitchDie,
};
IMPLEMENT_CUSTOM_SCHEDULES(CIchthyosaur, CFlyingMonster);

//=========================================================
// Classify - indicates this monster's place in the 
// relationship table.
//=========================================================
int CIchthyosaur::Classify ( void )
{
	return	CLASS_ALIEN_MONSTER;
}


//=========================================================
// CheckMeleeAttack1
//=========================================================
BOOL CIchthyosaur::CheckMeleeAttack1 ( float flDot, float flDist )
{
	//MODDD - added a distance requirement that can satisfy it too.
	//Also, "flDot" is for the correctness between what angle I am facing now and what angle I want to face (like towards the enemy).


 	if ( flDot >= 0.7 && (m_flEnemyTouched > gpGlobals->time - 0.2) || (flDist < 280)  )
	{
		return TRUE;
	}
	return FALSE;
}

void CIchthyosaur::BiteTouch( CBaseEntity *pOther )
{
	// bite if we hit who we want to eat
	if ( pOther == m_hEnemy ) 
	{
		m_flEnemyTouched = gpGlobals->time;
		m_bOnAttack = TRUE;
	}


	/*
	//MODDD TODO
	if(m_pSchedule == slTwitchDie && getTaskNumber() == TASK_ICHTHYOSAUR_FLOAT){
		//if we can determine that the thing is above and blocking, stop here?
		//perhaps check to see if the top of this monster is above water at the time of collision? that may be good enough.
		TaskComplete();
	}
	*/

}

void CIchthyosaur::CombatUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( !ShouldToggle( useType, m_bOnAttack ) )
		return;

	if (m_bOnAttack)
	{
		m_bOnAttack = 0;
	}
	else
	{
		m_bOnAttack = 1;
	}
}

//=========================================================
// CheckRangeAttack1  - swim in for a chomp
//
//=========================================================
BOOL CIchthyosaur::CheckRangeAttack1 ( float flDot, float flDist )
{
	if ( flDot > -0.7 && (m_bOnAttack || ( flDist <= 192 && m_idealDist <= 192)))
	{
		return TRUE;
	}

	return FALSE;
}

//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CIchthyosaur::SetYawSpeed ( void )
{
	pev->yaw_speed = 100;
}






GENERATE_TRACEATTACK_IMPLEMENTATION(CIchthyosaur)
{
	GENERATE_TRACEATTACK_PARENT_CALL(CFlyingMonster);
}

GENERATE_TAKEDAMAGE_IMPLEMENTATION(CIchthyosaur)
{
	return GENERATE_TAKEDAMAGE_PARENT_CALL(CFlyingMonster);
}




//=========================================================
// Killed - overrides CFlyingMonster.
//
GENERATE_KILLED_IMPLEMENTATION(CIchthyosaur)
{
	//Is calling direct parent CFlyingMonster instead of CBaseMonster ok?
	GENERATE_KILLED_PARENT_CALL(CFlyingMonster);
	pev->velocity = Vector( 0, 0, 0 );

	//MODDD NOTE
	//Forcing the movetype to MOVETYPE_STEP in CFlyingMonster's Killed above can be bad for the floating logic.
	//Or flyers in general ever really, MOVETYPE_TOSS falls just fine. no idea why
	pev->movetype = MOVETYPE_FLY;


}

void CIchthyosaur::BecomeDead( void )
{
	pev->takedamage = DAMAGE_YES;// don't let autoaim aim at corpses.

	// give the corpse half of the monster's original maximum health. 
	pev->health = pev->max_health / 2;
	pev->max_health = 5; // max_health now becomes a counter for how many blood decals the corpse can place.
}


//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CIchthyosaur::HandleAnimEvent( MonsterEvent_t *pEvent )
{
	int bDidAttack = FALSE;
	switch( pEvent->event )
	{
	case ICHTHYOSAUR_AE_SHAKE_RIGHT:
	case ICHTHYOSAUR_AE_SHAKE_LEFT:
		{
			if (m_hEnemy != NULL && FVisible( m_hEnemy ))
			{
				CBaseEntity *pHurt = m_hEnemy;
				float dirCorrectness;

				//MODDD - need to know what this is for debugging?
				//ALSO using MOD distance instead.
				float distanceToEnemyBody = (m_hEnemy->BodyTargetMod( pev->origin ) - pev->origin).Length();

				if (m_flEnemyTouched < gpGlobals->time - 0.2 && distanceToEnemyBody > 94) //(32+16+32)) that was 80.
					break;


				//MODDD - using MOD version of ShootAtEnemy.
				Vector vecShootDir = ShootAtEnemyMod( pev->origin );
				UTIL_MakeAimVectors ( pev->angles );


				dirCorrectness = DotProduct( vecShootDir, gpGlobals->v_forward );

				//MODDD - little more flexible angle. This is a big jaw.
				//if (dirCorrectness > 0.707)
				if (dirCorrectness > 0.6)
				{
					m_bOnAttack = TRUE;

					if(!pHurt->blocksImpact()){
						pHurt->pev->punchangle.z = -18;
						pHurt->pev->punchangle.x = 5;
						pHurt->pev->velocity = pHurt->pev->velocity - gpGlobals->v_right * 300;
						if (pHurt->IsPlayer())
						{
							//MODDD NOTE - hold on WHAT.
							// Why is this mechanic for modifying actual angle, not just punch angle,
							pHurt->pev->angles.x += RANDOM_FLOAT( -35, 35 );
							pHurt->pev->angles.y += RANDOM_FLOAT( -90, 90 );
							pHurt->pev->angles.z = 0;
							pHurt->pev->fixangle = TRUE;
						}
					}

					pHurt->TakeDamage( pev, pev, gSkillData.ichthyosaurDmgShake, DMG_SLASH );
					//MODDD
					UTIL_fromToBlood(this, pHurt, gSkillData.ichthyosaurDmgShake, 128);


				}
			}
			BiteSound();

			bDidAttack = TRUE;
		}
		break;
	default:
		CFlyingMonster::HandleAnimEvent( pEvent );
		break;
	}

	if (bDidAttack)
	{
		Vector vecSrc = pev->origin + gpGlobals->v_forward * 32;
		UTIL_Bubbles( vecSrc - Vector( 8, 8, 8 ), vecSrc + Vector( 8, 8, 8 ), 16 );
	}
}


CIchthyosaur::CIchthyosaur(){

	chaseEnemyAttackSoundCooldown = 0;

}

//=========================================================
// Spawn
//=========================================================
void CIchthyosaur::Spawn()
{
	Precache( );

	SET_MODEL(ENT(pev), "models/icky.mdl");
	UTIL_SetSize( pev, Vector( -32, -32, -32 ), Vector( 32, 32, 32 ) );

	pev->classname = MAKE_STRING("monster_ichthyosaur");

	pev->solid			= SOLID_BBOX;
	pev->movetype		= MOVETYPE_FLY;
	m_bloodColor		= BLOOD_COLOR_GREEN;
	pev->health			= gSkillData.ichthyosaurHealth;
	pev->view_ofs		= Vector ( 0, 0, 16 );
	m_flFieldOfView		= VIEW_FIELD_WIDE;
	m_MonsterState		= MONSTERSTATE_NONE;
	SetBits(pev->flags, FL_SWIM);
	SetFlyingSpeed( ICHTHYOSAUR_SPEED );
	SetFlyingMomentum( 2.5 );	// Set momentum constant

	m_afCapability		= bits_CAP_RANGE_ATTACK1 | bits_CAP_SWIM;

	MonsterInit();

	SetTouch( &CIchthyosaur::BiteTouch );
	SetUse( &CIchthyosaur::CombatUse );

	m_idealDist = 384;
	m_flMinSpeed = 80;
	m_flMaxSpeed = 300;
	m_flMaxDist = 384;

	Vector Forward;
	UTIL_MakeVectorsPrivate(pev->angles, Forward, 0, 0);
	pev->velocity = m_flightSpeed * Forward.Normalize();
	m_SaveVelocity = pev->velocity;

}

extern int global_useSentenceSave;
//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CIchthyosaur::Precache()
{
	PRECACHE_MODEL("models/icky.mdl");
	
	global_useSentenceSave = TRUE;
	PRECACHE_SOUND_ARRAY( pIdleSounds );
	PRECACHE_SOUND_ARRAY( pAlertSounds );
	PRECACHE_SOUND_ARRAY( pAttackSounds );
	PRECACHE_SOUND_ARRAY( pBiteSounds );
	PRECACHE_SOUND_ARRAY( pDieSounds );
	PRECACHE_SOUND_ARRAY( pPainSounds );
	global_useSentenceSave = FALSE;
}

//=========================================================
// GetSchedule
//=========================================================
Schedule_t* CIchthyosaur::GetSchedule()
{
	// ALERT( at_console, "GetSchedule( )\n" );
	switch(m_MonsterState)
	{
	case MONSTERSTATE_IDLE:
		m_flightSpeed = 80;
		return GetScheduleOfType( SCHED_IDLE_WALK );

	case MONSTERSTATE_ALERT:
		m_flightSpeed = 150;
		return GetScheduleOfType( SCHED_IDLE_WALK );

	case MONSTERSTATE_COMBAT:
		m_flMaxSpeed = 400;
		// eat them
		if ( HasConditions( bits_COND_CAN_MELEE_ATTACK1 ) )
		{
			return GetScheduleOfType( SCHED_MELEE_ATTACK1 );
		}
		// chase them down and eat them
		if ( HasConditions( bits_COND_CAN_RANGE_ATTACK1 ) )
		{
			return GetScheduleOfType( SCHED_CHASE_ENEMY );
		}
		if ( HasConditions( bits_COND_HEAVY_DAMAGE ) )
		{
			m_bOnAttack = TRUE;
		}
		if ( pev->health < pev->max_health - 20 )
		{
			m_bOnAttack = TRUE;
		}

		return GetScheduleOfType( SCHED_STANDOFF );
	}

	return CFlyingMonster::GetSchedule();
}


//=========================================================
//=========================================================
Schedule_t* CIchthyosaur::GetScheduleOfType ( int Type ) 
{
	// ALERT( at_console, "GetScheduleOfType( %d ) %d\n", Type, m_bOnAttack );
	switch	( Type )
	{
	case SCHED_IDLE_WALK:
		return slSwimAround;
	case SCHED_STANDOFF:
		return slCircleEnemy;
	case SCHED_FAIL:
		return slSwimAgitated;
	case SCHED_DIE:
		//MODDD - renamed
		//return slTwitchDie;
		return slDieWaterFloat;
	case SCHED_CHASE_ENEMY:
		//MODDD - this is very spammy. SCHED_CHASE_ENEMY is interrupted by being able to do a Range Attack (bits_COND_CAN_RANGE_ATTACK1),
		//        which also leads to calling this schedule anyways.
		// Gave this a cooldown before playing another attack sound.
		if(gpGlobals->time >= chaseEnemyAttackSoundCooldown){
			chaseEnemyAttackSoundCooldown = gpGlobals->time + RANDOM_FLOAT(4.6, 9.2);
			AttackSound( );
		}

	}

	return CBaseMonster::GetScheduleOfType( Type );
}



//=========================================================
// Start task - selects the correct activity and performs
// any necessary calculations to start the next task on the
// schedule.
//=========================================================
void CIchthyosaur::StartTask(Task_t *pTask)
{
	switch (pTask->iTask)
	{
	case TASK_ICHTHYOSAUR_CIRCLE_ENEMY:
		break;
	case TASK_ICHTHYOSAUR_SWIM:
		break;
	case TASK_SMALL_FLINCH:
		if (m_idealDist > 128)
		{
			m_flMaxDist = 512;
			m_idealDist = 512;
		}
		else
		{
			m_bOnAttack = TRUE;
		}
		CFlyingMonster::StartTask(pTask);
		break;
	//MODDD - renamed from TASK_ICHTHYOSAUR_FLOAT
	case TASK_WATER_DEAD_FLOAT:
		pev->skin = EYE_BASE;
		SetSequenceByName( "bellyup" );
		CFlyingMonster::StartTask(pTask);
	break;
	default:
		CFlyingMonster::StartTask(pTask);
		break;
	}
}

void CIchthyosaur::RunTask ( Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_ICHTHYOSAUR_CIRCLE_ENEMY:
		if (m_hEnemy == NULL)
		{
			TaskComplete( );
		}
		else if (FVisible( m_hEnemy ))
		{
			Vector vecFrom = m_hEnemy->EyePosition( );

			Vector vecDelta = (pev->origin - vecFrom).Normalize( );
			Vector vecSwim = CrossProduct( vecDelta, Vector( 0, 0, 1 ) ).Normalize( );
			
			if (DotProduct( vecSwim, m_SaveVelocity ) < 0)
				vecSwim = vecSwim * -1.0;

			Vector vecPos = vecFrom + vecDelta * m_idealDist + vecSwim * 32;

			// ALERT( at_console, "vecPos %.0f %.0f %.0f\n", vecPos.x, vecPos.y, vecPos.z );

			TraceResult tr;
		
			UTIL_TraceHull( vecFrom, vecPos, ignore_monsters, large_hull, m_hEnemy->edict(), &tr );

			if (tr.flFraction > 0.5)
				vecPos = tr.vecEndPos;

			m_SaveVelocity = m_SaveVelocity * 0.8 + 0.2 * (vecPos - pev->origin).Normalize() * m_flightSpeed;

			// ALERT( at_console, "m_SaveVelocity %.2f %.2f %.2f\n", m_SaveVelocity.x, m_SaveVelocity.y, m_SaveVelocity.z );

			if (HasConditions( bits_COND_ENEMY_FACING_ME ) && m_hEnemy->FVisible( this ))
			{
				m_flNextAlert -= 0.1;

				if (m_idealDist < m_flMaxDist)
				{
					m_idealDist += 4;
				}
				if (m_flightSpeed > m_flMinSpeed)
				{
					m_flightSpeed -= 2;
				}
				else if (m_flightSpeed < m_flMinSpeed)
				{
					m_flightSpeed += 2;
				}
				if (m_flMinSpeed < m_flMaxSpeed)
				{
					m_flMinSpeed += 0.5;
				}
			}
			else 
			{
				m_flNextAlert += 0.1;

				if (m_idealDist > 128)
				{
					m_idealDist -= 4;
				}
				if (m_flightSpeed < m_flMaxSpeed)
				{
					m_flightSpeed += 4;
				}
			}
			// ALERT( at_console, "%.0f\n", m_idealDist );
		}
		else
		{
			m_flNextAlert = gpGlobals->time + 0.2;
		}

		if (m_flNextAlert < gpGlobals->time)
		{
			// ALERT( at_console, "AlertSound()\n");
			AlertSound( );
			m_flNextAlert = gpGlobals->time + RANDOM_FLOAT( 3, 5 );
		}

		break;
	case TASK_ICHTHYOSAUR_SWIM:
		if (m_fSequenceFinished)
		{
			TaskComplete( );
		}
		break;
		/*
	case TASK_DIE:
		//NOTICE - this overridden behavior is no longer necessary.
		//The base "slDieWaterFloat" schedule calls TASK_DIE_SIMPLE which waits for the sequence to finish and set the deadflag just like here. 
		if ( m_fSequenceFinished )
		{
			pev->deadflag = DEAD_DEAD;

			TaskComplete( );
		}
		*/
	break;
	//MODDD - TASK_ICHTHYOSAUR_FLOAT renamed TASK_WATER_FLOAT_DEAD and made more general (schedule.cpp)

	default: 
		CFlyingMonster::RunTask ( pTask );
		break;
	}
}



float CIchthyosaur::VectorToPitch( const Vector &vec )
{
	float pitch;
	if (vec.z == 0 && vec.x == 0)
		pitch = 0;
	else
	{
		//MODDD - why was this all casted to an int anyway?  pitch is a float.
		pitch = (atan2(vec.z, (float)sqrt(vec.x*vec.x+vec.y*vec.y)) * 180.0f / M_PI);
		if (pitch < 0)
			pitch += 360;
	}
	return pitch;
}

//=========================================================
void CIchthyosaur::Move(float flInterval)
{
	CFlyingMonster::Move( flInterval );
}

float CIchthyosaur::FlPitchDiff( void )
{
	float flPitchDiff;
	float flCurrentPitch;

	flCurrentPitch = UTIL_AngleMod( pev->angles.z );

	if ( flCurrentPitch == pev->idealpitch )
	{
		return 0;
	}

	flPitchDiff = pev->idealpitch - flCurrentPitch;

	if ( pev->idealpitch > flCurrentPitch )
	{
		if (flPitchDiff >= 180)
			flPitchDiff = flPitchDiff - 360;
	}
	else 
	{
		if (flPitchDiff <= -180)
			flPitchDiff = flPitchDiff + 360;
	}
	return flPitchDiff;
}

float CIchthyosaur::ChangePitch( int speed )
{
	if ( pev->movetype == MOVETYPE_FLY )
	{
		float diff = FlPitchDiff();
		float target = 0;
		if ( m_IdealActivity != GetStoppedActivity() )
		{
			if (diff < -20)
				target = 45;
			else if (diff > 20)
				target = -45;
		}
		pev->angles.x = UTIL_Approach(target, pev->angles.x, 220.0 * 0.1 );
	}
	return 0;
}

float CIchthyosaur::ChangeYaw( int speed )
{
	if ( pev->movetype == MOVETYPE_FLY )
	{
		float diff = FlYawDiff();
		float target = 0;

		if ( m_IdealActivity != GetStoppedActivity() )
		{
			if ( diff < -20 )
				target = 20;
			else if ( diff > 20 )
				target = -20;
		}
		pev->angles.z = UTIL_Approach( target, pev->angles.z, 220.0 * 0.1 );
	}
	return CFlyingMonster::ChangeYaw( speed );
}


Activity CIchthyosaur::GetStoppedActivity( void )
{ 
	if ( pev->movetype != MOVETYPE_FLY )		// UNDONE: Ground idle here, IDLE may be something else
		return ACT_IDLE;
	return ACT_WALK;
}

void CIchthyosaur::MoveExecute( CBaseEntity *pTargetEnt, const Vector &vecDir, float flInterval )
{
	m_SaveVelocity = vecDir * m_flightSpeed;
}


void CIchthyosaur::MonsterThink ( void )
{
	CFlyingMonster::MonsterThink( );

	if (pev->deadflag == DEAD_NO)
	{
		if (m_MonsterState != MONSTERSTATE_SCRIPT)
		{
			Swim( );

			// blink the eye
			if (m_flBlink < gpGlobals->time)
			{
				pev->skin = EYE_CLOSED;
				if (m_flBlink + 0.2 < gpGlobals->time)
				{
					m_flBlink = gpGlobals->time + RANDOM_FLOAT( 3, 4 );
					if (m_bOnAttack)
						pev->skin = EYE_MAD;
					else
						pev->skin = EYE_BASE;
				}
			}
		}
	}
}

void CIchthyosaur::Stop( void ) 
{
	if (!m_bOnAttack)
		m_flightSpeed = 80.0;
}

void CIchthyosaur::Swim( )
{
	//pev->velocity = Vector(10, 0, 30);
	//return;

	int retValue = 0;

	Vector start = pev->origin;

	Vector Angles;
	Vector Forward, Right, Up;

	if (FBitSet( pev->flags, FL_ONGROUND))
	{
		pev->angles.x = 0;
		pev->angles.y += RANDOM_FLOAT( -45, 45 );
		ClearBits( pev->flags, FL_ONGROUND );

		Angles = Vector( -pev->angles.x, pev->angles.y, pev->angles.z );
		UTIL_MakeVectorsPrivate(Angles, Forward, Right, Up);

		pev->velocity = Forward * 200 + Up * 200;

		return;
	}

	if (m_bOnAttack && m_flightSpeed < m_flMaxSpeed)
	{
		m_flightSpeed += 40;
	}
	if (m_flightSpeed < 180)
	{
		if (m_IdealActivity == ACT_RUN)
			SetActivity( ACT_WALK );
		if (m_IdealActivity == ACT_WALK)
			pev->framerate = m_flightSpeed / 150.0;
		// ALERT( at_console, "walk %.2f\n", pev->framerate );
	}
	else
	{
		if (m_IdealActivity == ACT_WALK)
			SetActivity( ACT_RUN );
		if (m_IdealActivity == ACT_RUN)
			pev->framerate = m_flightSpeed / 150.0;
		// ALERT( at_console, "run  %.2f\n", pev->framerate );
	}

	/*
	if (!m_pBeam)
	{
		m_pBeam = CBeam::BeamCreate( "sprites/laserbeam.spr", 80 );
		m_pBeam->PointEntInit( pev->origin + m_SaveVelocity, entindex( ) );
		m_pBeam->SetEndAttachment( 1 );
		m_pBeam->SetColor( 255, 180, 96 );
		m_pBeam->SetBrightness( 192 );
	}
	*/


	Angles = UTIL_VecToAngles( m_SaveVelocity );
	Angles.x = -Angles.x;
	UTIL_MakeVectorsPrivate(Angles, Forward, Right, Up);

	Vector f, u, l, r, d;
	f = DoProbe(start + PROBE_LENGTH   * Forward);
	r = DoProbe(start + PROBE_LENGTH/3 * Forward+Right);
	l = DoProbe(start + PROBE_LENGTH/3 * Forward-Right);
	u = DoProbe(start + PROBE_LENGTH/3 * Forward+Up);
	d = DoProbe(start + PROBE_LENGTH/3 * Forward-Up);

	Vector SteeringVector = f+r+l+u+d;
	m_SaveVelocity = (m_SaveVelocity + SteeringVector/2).Normalize();

	Angles = Vector( -pev->angles.x, pev->angles.y, pev->angles.z );
	UTIL_MakeVectorsPrivate(Angles, Forward, Right, Up);
	// ALERT( at_console, "%f : %f\n", Angles.x, Forward.z );



	float flDot = DotProduct( Forward, m_SaveVelocity );
	if (flDot > 0.5)
		pev->velocity = m_SaveVelocity = m_SaveVelocity * m_flightSpeed;
	else if (flDot > 0)
		pev->velocity = m_SaveVelocity = m_SaveVelocity * m_flightSpeed * (flDot + 0.5);
	else
		pev->velocity = m_SaveVelocity = m_SaveVelocity * 80;

	// ALERT( at_console, "%.0f %.0f\n", m_flightSpeed, pev->velocity.Length() );


	// ALERT( at_console, "Steer %f %f %f\n", SteeringVector.x, SteeringVector.y, SteeringVector.z );

	/*
	m_pBeam->SetStartPos( pev->origin + pev->velocity );
	m_pBeam->RelinkBeam( );
	*/

	// ALERT( at_console, "speed %f\n", m_flightSpeed );
	
	Angles = UTIL_VecToAngles( m_SaveVelocity );

	// Smooth Pitch
	//
	if (Angles.x > 180)
		Angles.x = Angles.x - 360;
	pev->angles.x = UTIL_Approach(Angles.x, pev->angles.x, 50 * 0.1 );
	if (pev->angles.x < -80) pev->angles.x = -80;
	if (pev->angles.x >  80) pev->angles.x =  80;

	// Smooth Yaw and generate Roll
	//
	float turn = 360;
	// ALERT( at_console, "Y %.0f %.0f\n", Angles.y, pev->angles.y );

	if (fabs(Angles.y - pev->angles.y) < fabs(turn))
	{
		turn = Angles.y - pev->angles.y;
	}
	if (fabs(Angles.y - pev->angles.y + 360) < fabs(turn))
	{
		turn = Angles.y - pev->angles.y + 360;
	}
	if (fabs(Angles.y - pev->angles.y - 360) < fabs(turn))
	{
		turn = Angles.y - pev->angles.y - 360;
	}

	float speed = m_flightSpeed * 0.1;

	// ALERT( at_console, "speed %.0f %f\n", turn, speed );
	if (fabs(turn) > speed)
	{
		if (turn < 0.0)
		{
			turn = -speed;
		}
		else
		{
			turn = speed;
		}
	}
	pev->angles.y += turn;
	pev->angles.z -= turn;
	pev->angles.y = fmod((pev->angles.y + 360.0), 360.0);

	static float yaw_adj;

	yaw_adj = yaw_adj * 0.8 + turn;

	// ALERT( at_console, "yaw %f : %f\n", turn, yaw_adj );

	SetBoneController( 0, -yaw_adj / 4.0 );

	// Roll Smoothing
	//
	turn = 360;
	if (fabs(Angles.z - pev->angles.z) < fabs(turn))
	{
		turn = Angles.z - pev->angles.z;
	}
	if (fabs(Angles.z - pev->angles.z + 360) < fabs(turn))
	{
		turn = Angles.z - pev->angles.z + 360;
	}
	if (fabs(Angles.z - pev->angles.z - 360) < fabs(turn))
	{
		turn = Angles.z - pev->angles.z - 360;
	}
	speed = m_flightSpeed/2 * 0.1;
	if (fabs(turn) < speed)
	{
		pev->angles.z += turn;
	}
	else
	{
		if (turn < 0.0)
		{
			pev->angles.z -= speed;
		}
		else
		{
			pev->angles.z += speed;
		}
	}
	if (pev->angles.z < -20) pev->angles.z = -20;
	if (pev->angles.z >  20) pev->angles.z =  20;

	UTIL_MakeVectorsPrivate( Vector( -Angles.x, Angles.y, Angles.z ), Forward, Right, Up);

	// UTIL_MoveToOrigin ( ENT(pev), pev->origin + Forward * speed, speed, MOVE_STRAFE );
}


Vector CIchthyosaur::DoProbe(const Vector &Probe)
{
	Vector WallNormal = Vector(0,0,-1); // WATER normal is Straight Down for fish.
	float frac;
	BOOL bBumpedSomething = ProbeZ(pev->origin, Probe, &frac);

	TraceResult tr;

	//(edict_t *pEdict, const float *v1, const float *v2, int fNoMonsters, edict_t *pentToSkip, TraceResult *ptr);
	TRACE_MONSTER_HULL(edict(), pev->origin, Probe, dont_ignore_monsters, edict(), &tr);
	if ( tr.fAllSolid || tr.flFraction < 0.99 )
	{
		if (tr.flFraction < 0.0) tr.flFraction = 0.0;
		if (tr.flFraction > 1.0) tr.flFraction = 1.0;
		if (tr.flFraction < frac)
		{
			frac = tr.flFraction;
			bBumpedSomething = TRUE;
			WallNormal = tr.vecPlaneNormal;
		}
	}

	if (bBumpedSomething && (m_hEnemy == NULL || tr.pHit != m_hEnemy->edict()))
	{
		Vector ProbeDir = Probe - pev->origin;

		Vector NormalToProbeAndWallNormal = CrossProduct(ProbeDir, WallNormal);
		Vector SteeringVector = CrossProduct( NormalToProbeAndWallNormal, ProbeDir);

		float SteeringForce = m_flightSpeed * (1-frac) * (DotProduct(WallNormal.Normalize(), m_SaveVelocity.Normalize()));
		if (SteeringForce < 0.0)
		{
			SteeringForce = -SteeringForce;
		}
		SteeringVector = SteeringForce * SteeringVector.Normalize();
		
		return SteeringVector;
	}
	return Vector(0, 0, 0);
}
