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
// monster template
//=========================================================
// UNDONE: Holster weapon?

#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"monsters.h"
#include	"quakemonster.h"
#include	"schedule.h"
#include	"defaultai.h"
#include	"scripted.h"
#include	"weapons.h"
#include	"soundent.h"
#include	"nodes.h"
#include	"effects.h"
//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define		DOG_AE_LEAP	( 3 )
#define		DOG_AE_BITE	( 4 )

class CQ1Dog : public CQuakeMonster
{
public:
	void Spawn( void );
	void Precache( void );

	int  ISoundMask( void );

	int  Classify ( void );
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	void SetYawSpeed( void );

	void RunTask( Task_t *pTask );
	void StartTask( Task_t *pTask );

	int TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType);
	BOOL CheckRangeAttack1 ( float flDot, float flDist );
	BOOL CheckMeleeAttack1 ( float flDot, float flDist );

	// Override these to set behavior
	Schedule_t *GetScheduleOfType ( int Type );
	Schedule_t *GetSchedule ( void );
	MONSTERSTATE GetIdealState ( void );

	CBaseEntity	*Kick( void );

	void IdleSound( void );
	void AlertSound( void );
	void DeathSound( void );
	void PainSound( void );

	void TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType);
	void Killed( entvars_t *pevAttacker, int iGib );

	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

public:
	float m_painTime;
};

LINK_ENTITY_TO_CLASS( monster_q1dog, CQ1Dog );

TYPEDESCRIPTION	CQ1Dog::m_SaveData[] = 
{
	DEFINE_FIELD( CQ1Dog, m_painTime, FIELD_TIME )
};

IMPLEMENT_SAVERESTORE( CQ1Dog, CQuakeMonster );

//=========================================================
// StartTask
//
//=========================================================
void CQ1Dog :: StartTask( Task_t *pTask )
{
	switch(pTask->iTask)
	{
	default:
		CQuakeMonster::StartTask( pTask );
		break;
	}
}

//=========================================================
// Kick
//
//=========================================================
CBaseEntity *CQ1Dog :: Kick( void )
{
	TraceResult tr;

	UTIL_MakeVectors( pev->angles );
	Vector vecStart = pev->origin;
	vecStart.z += pev->size.z * 0.5;
	Vector vecEnd = vecStart + (gpGlobals->v_forward * 96);

	UTIL_TraceHull( vecStart, vecEnd, dont_ignore_monsters, head_hull, ENT(pev), &tr );
	
	if ( tr.pHit )
	{
		CBaseEntity *pEntity = CBaseEntity::Instance( tr.pHit );
		return pEntity;
	}

	return NULL;
}

//=========================================================
// RunTask
//
//=========================================================
void CQ1Dog :: RunTask( Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_RANGE_ATTACK1:
		CQuakeMonster::RunTask( pTask );
		break;
	default:
		CQuakeMonster::RunTask( pTask );
		break;
	}
}

//=========================================================
// ISoundMask - returns a bit mask indicating which types
// of sounds this monster regards. 
//=========================================================
int CQ1Dog :: ISoundMask ( void) 
{
	return	bits_SOUND_WORLD	|
			bits_SOUND_COMBAT	|
			bits_SOUND_CARCASS	|
			bits_SOUND_MEAT		|
			bits_SOUND_GARBAGE	|
			bits_SOUND_DANGER	|
			bits_SOUND_PLAYER;
}

//=========================================================
// Classify - indicates this monster's place in the 
// relationship table.
//=========================================================
int	CQ1Dog :: Classify ( void )
{
	return	CLASS_HUMAN_MILITARY;
}

//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CQ1Dog :: SetYawSpeed ( void )
{
	int ys;

	switch ( m_Activity )
	{
	case ACT_IDLE:	
		ys = 150;		
		break;
	case ACT_RUN:	
		ys = 150;	
		break;
	case ACT_WALK:	
		ys = 180;		
		break;
	case ACT_RANGE_ATTACK1:	
		ys = 120;	
		break;
	case ACT_RANGE_ATTACK2:	
		ys = 120;	
		break;
	case ACT_MELEE_ATTACK1:	
		ys = 120;	
		break;
	case ACT_MELEE_ATTACK2:	
		ys = 120;	
		break;
	case ACT_TURN_LEFT:
	case ACT_TURN_RIGHT:	
		ys = 180;
		break;
	case ACT_GLIDE:
	case ACT_FLY:
		ys = 30;
		break;
	default:
		ys = 90;
		break;
	}

	pev->yaw_speed = ys;
}

//=========================================================
// CheckMeleeAttack1
//=========================================================
BOOL CQ1Dog :: CheckMeleeAttack1 ( float flDot, float flDist )
{
	CBaseMonster *pEnemy;

	if ( m_hEnemy != NULL )
	{
		pEnemy = m_hEnemy->MyMonsterPointer();

		if ( !pEnemy )
		{
			return FALSE;
		}
	}

	if ( flDist <= 64 && flDot >= 0.7	&& 
		 pEnemy->Classify() != CLASS_ALIEN_BIOWEAPON &&
		 pEnemy->Classify() != CLASS_PLAYER_BIOWEAPON )
	{
		return TRUE;
	}
	return FALSE;
}

//=========================================================
// CheckRangeAttack1
//=========================================================
BOOL CQ1Dog :: CheckRangeAttack1 ( float flDot, float flDist )
{
	//if( m_nextAttack > gpGlobals->time )
	//	return FALSE;

	return FALSE;
}

//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//
// Returns number of events handled, 0 if none.
//=========================================================
void CQ1Dog :: HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch( pEvent->event )
	{
	case DOG_AE_LEAP:
		// NADA
		break;

	case DOG_AE_BITE:
		{
			CBaseEntity *pHurt = Kick();

			if ( pHurt )
			{
				// SOUND HERE!
				if(!pHurt->IsBSPModel())
				{
					UTIL_MakeVectors( pev->angles );
					pHurt->pev->punchangle.x = 15;
					pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_forward * 100 + gpGlobals->v_up * 50;

					EMIT_SOUND_DYN( ENT(pHurt->pev), CHAN_STATIC, "shambler/smack.wav", 1, ATTN_NORM, 0, PITCH_NORM);
				}
				pHurt->TakeDamage( pev, pev, gSkillData.dogDmgBite, DMG_CLUB );
			}
		}
		break;

	default:
		CQuakeMonster::HandleAnimEvent( pEvent );
	}
}

//=========================================================
// Spawn
//=========================================================
void CQ1Dog :: Spawn()
{
	Precache( );

	SET_MODEL(ENT(pev), "models/dog.mdl");
	UTIL_SetSize(pev, Vector ( -16, -16, 0 ), Vector ( 16, 16, 36 ) );

	SetAliasData();

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_RED;
	pev->health			= gSkillData.dogHealth;
	pev->view_ofs		= Vector ( 0, 0, 85 );// position of the eyes relative to monster's origin.
	m_flFieldOfView		= VIEW_FIELD_WIDE; // NOTE: we need a wide field of view so npc will notice player and say hello
	m_MonsterState		= MONSTERSTATE_NONE;

	m_afCapability		= bits_CAP_HEAR | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP;

	MonsterInit();
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CQ1Dog :: Precache()
{
	PRECACHE_MODEL("models/dog.mdl");
	
	PRECACHE_SOUND("shambler/smack.wav");
	PRECACHE_SOUND("dog/dattack1.wav");
	PRECACHE_SOUND("dog/ddeath.wav");
	PRECACHE_SOUND("dog/didle.wav");
	PRECACHE_SOUND("dog/dpain1.wav");

	// every new barney must call this, otherwise
	// when a level is loaded, nobody will talk (time is reset to 0)
	CQuakeMonster::Precache();
}	

//=========================================================
// PainSound
//=========================================================
int CQ1Dog :: TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	return CQuakeMonster::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
}

//=========================================================
// AlertSound
//=========================================================
void CQ1Dog :: AlertSound ( void )
{
	EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "dog/dattack1.wav", 1, ATTN_NORM, 0, PITCH_NORM);
}

//=========================================================
// PainSound
//=========================================================
void CQ1Dog :: PainSound ( void )
{
	if (gpGlobals->time < m_painTime)
		return;
	
	m_painTime = gpGlobals->time + RANDOM_FLOAT(0.5, 0.75);

	EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "dog/dpain1.wav", 1, ATTN_NORM, 0, PITCH_NORM);
}

//=========================================================
// DeathSound 
//=========================================================
void CQ1Dog :: DeathSound ( void )
{
	EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "dog/ddeath.wav", 1, ATTN_NORM, 0, PITCH_NORM);
}

//=========================================================
// IdleSound 
//=========================================================
void CQ1Dog :: IdleSound ( void )
{
	EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "dog/didle.wav", 1, ATTN_NORM, 0, PITCH_NORM);
}

//=========================================================
// TraceAttack 
//=========================================================
void CQ1Dog::TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType)
{
	CQuakeMonster::TraceAttack( pevAttacker, flDamage, vecDir, ptr, bitsDamageType );
}

//=========================================================
// Killed 
//=========================================================
void CQ1Dog::Killed( entvars_t *pevAttacker, int iGib )
{
	CQuakeMonster::Killed( pevAttacker, iGib );
}

//=========================================================
// AI Schedules Specific to this monster
//=========================================================
Schedule_t* CQ1Dog :: GetScheduleOfType ( int Type )
{
	switch( Type )
	{
	default:
		return CQuakeMonster::GetScheduleOfType( Type );
		break;
	}
}

//=========================================================
// GetSchedule - Decides which type of schedule best suits
// the monster's current state and conditions. Then calls
// monster's member function to get a pointer to a schedule
// of the proper type.
//=========================================================
Schedule_t *CQ1Dog :: GetSchedule ( void )
{
	if ( HasConditions( bits_COND_HEAR_SOUND ) )
	{
		CSound *pSound;
		pSound = PBestSound();

		ASSERT( pSound != NULL );
		if ( pSound && (pSound->m_iType & bits_SOUND_DANGER) )
			return GetScheduleOfType( SCHED_TAKE_COVER_FROM_BEST_SOUND );
	}

	switch( m_MonsterState )
	{
	case MONSTERSTATE_COMBAT:
		{
// dead enemy
			if ( HasConditions( bits_COND_ENEMY_DEAD ) )
			{
				// call base class, all code to handle dead enemies is centralized there.
				return CBaseMonster :: GetSchedule();
			}

			// always act surprized with a new enemy
			if ( HasConditions( bits_COND_NEW_ENEMY ) && HasConditions( bits_COND_LIGHT_DAMAGE) )
				return GetScheduleOfType( SCHED_SMALL_FLINCH );

			if ( HasConditions( bits_COND_HEAVY_DAMAGE ) )
				return GetScheduleOfType( SCHED_TAKE_COVER_FROM_ENEMY );
		}
		break;

	case MONSTERSTATE_ALERT:	
	case MONSTERSTATE_IDLE:
		if ( HasConditions(bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE))
		{
			// flinch if hurt
			return GetScheduleOfType( SCHED_SMALL_FLINCH );
		}
		break;
	}
	
	return CQuakeMonster::GetSchedule();
}

MONSTERSTATE CQ1Dog :: GetIdealState ( void )
{
	return CQuakeMonster::GetIdealState();
}
