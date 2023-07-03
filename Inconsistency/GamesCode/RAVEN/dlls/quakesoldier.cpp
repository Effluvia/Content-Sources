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

//=========================================================
// monster-specific definitions
//=========================================================
#define SOLDIER_CLIP_SIZE	8

//=========================================================
// monster-specific tasks
//=========================================================
enum 
{
	TASK_SOLDIER_SEARCH = LAST_COMMON_TASK + 1,
	TASK_SOLDIER_WAIT_SEARCH
};

//=========================================================
// monster-specific schedules
//=========================================================
enum 
{
	SCHED_SOLDIER_SEARCH = LAST_COMMON_SCHEDULE + 1,
	SCHED_SOLDIER_SEARCH_FAIL,
	SCHED_SOLDIER_COVER_AND_RELOAD,
	SCHED_SOLDIER_TAKE_COVER_FROM_ENEMY,
	SCHED_SOLDIER_AMBUSH_WAIT
};

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define		SOLDIER_AE_SHOOT	( 4 )
#define		SOLDIER_AE_RELOAD	( 5 )

class CQ1Soldier : public CQuakeMonster
{
public:
	void Spawn( void );
	void Precache( void );

	int  ISoundMask( void );
	void Shoot( void );

	int  Classify ( void );
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	void SetYawSpeed( void );

	void RunTask( Task_t *pTask );
	void StartTask( Task_t *pTask );

	void CheckAmmo ( void );

	int TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType);
	BOOL CheckRangeAttack1 ( float flDot, float flDist );

	// Override these to set behavior
	Schedule_t *GetScheduleOfType ( int Type );
	Schedule_t *GetSchedule ( void );
	MONSTERSTATE GetIdealState ( void );

	void IdleSound( void );
	void AlertSound( void );
	void DeathSound( void );
	void PainSound( void );
	
	BOOL FindSearchDestination( void );

	void TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType);
	void Killed( entvars_t *pevAttacker, int iGib );

	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

public:
	float m_painTime;
	float m_nextSearchTime;
	BOOL m_droppedGun;

	CUSTOM_SCHEDULES;
};

LINK_ENTITY_TO_CLASS( monster_q1soldier, CQ1Soldier );

TYPEDESCRIPTION	CQ1Soldier::m_SaveData[] = 
{
	DEFINE_FIELD( CQ1Soldier, m_painTime, FIELD_TIME ),
	DEFINE_FIELD( CQ1Soldier, m_nextSearchTime, FIELD_TIME ),
	DEFINE_FIELD( CQ1Soldier, m_droppedGun, FIELD_BOOLEAN ),
};

IMPLEMENT_SAVERESTORE( CQ1Soldier, CQuakeMonster );

//=========================================================
// InvestigateSound - sends a monster to the location of the
// sound that was just heard, to check things out. 
//=========================================================
Task_t tlSoldierSearch[] =
{
	{ TASK_SET_FAIL_SCHEDULE,		(float)SCHED_SOLDIER_SEARCH_FAIL	},
	{ TASK_STOP_MOVING,				(float)0							},
	{ TASK_SOLDIER_SEARCH,			(float)0							},
	{ TASK_FACE_IDEAL,				(float)0							},
	{ TASK_WALK_PATH,				(float)0							},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0							},
	{ TASK_PLAY_SEQUENCE,			(float)ACT_IDLE						},
	{ TASK_WAIT,					(float)10							},
	{ TASK_SOLDIER_WAIT_SEARCH,		(float)0							}
};

Schedule_t	slSoldierSearch[] =
{
	{ 
		tlSoldierSearch,
		ARRAYSIZE ( tlSoldierSearch ), 
		bits_COND_NEW_ENEMY			|
		bits_COND_SEE_FEAR			|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"Soldier Search"
	},
};

//=========================================================
// InvestigateSound - sends a monster to the location of the
// sound that was just heard, to check things out. 
//=========================================================
Task_t tlSoldierSearchFail[] =
{
	{ TASK_STOP_MOVING,				(float)0							},
	{ TASK_SOLDIER_WAIT_SEARCH,		(float)0							}
};

Schedule_t	slSoldierSearchFail[] =
{
	{ 
		tlSoldierSearchFail,
		ARRAYSIZE ( tlSoldierSearchFail ), 
		bits_COND_NEW_ENEMY			|
		bits_COND_SEE_FEAR			|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"Soldier Search Fail"
	},
};

//=========================================================
// Grunt reload schedule
//=========================================================
Task_t	tlSoldierHideReload[] =
{
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_SET_FAIL_SCHEDULE,		(float)SCHED_RELOAD			},
	{ TASK_FIND_COVER_FROM_ENEMY,	(float)0					},
	{ TASK_RUN_PATH,				(float)0					},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0					},
	{ TASK_FACE_ENEMY,				(float)0					},
	{ TASK_PLAY_SEQUENCE,			(float)ACT_RELOAD			},
};

Schedule_t slSoldierHideReload[] = 
{
	{
		tlSoldierHideReload,
		ARRAYSIZE ( tlSoldierHideReload ),
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_HEAR_SOUND,

		bits_SOUND_DANGER,
		"Soldier Hide Reload"
	}
};

//=========================================================
// Take cover from enemy! Soldiers will try to ambush the enemy
//
//=========================================================
Task_t	tlSoldierTakeCoverFromEnemy[] =
{
	{ TASK_STOP_MOVING,				(float)0							},
	{ TASK_WAIT,					(float)0.2							},
	{ TASK_FIND_COVER_FROM_ENEMY,	(float)0							},
	{ TASK_RUN_PATH,				(float)0							},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0							},
	{ TASK_REMEMBER,				(float)bits_MEMORY_INCOVER			},
	{ TASK_FACE_ENEMY,				(float)0							},
	{ TASK_SET_SCHEDULE,			(float)SCHED_SOLDIER_AMBUSH_WAIT	}
};

Schedule_t	slSoldierTakeCoverFromEnemy[] =
{
	{ 
		tlSoldierTakeCoverFromEnemy,
		ARRAYSIZE ( tlSoldierTakeCoverFromEnemy ), 
		bits_COND_NEW_ENEMY,
		0,
		"Soldier Take Cover from Enemy"
	},
};

//=========================================================
// Wait to ambush the enemy
//
//=========================================================
Task_t	tlSoldierAmbushWait[] =
{
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_SET_ACTIVITY,			(float)ACT_IDLE				},
	{ TASK_WAIT_INDEFINITE,			(float)0					}
};

Schedule_t	slSoldierAmbushWait[] =
{
	{ 
		tlSoldierAmbushWait,
		ARRAYSIZE ( tlSoldierAmbushWait ), 
		bits_COND_NEW_ENEMY			|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_CAN_RANGE_ATTACK1 |
		bits_COND_PROVOKED,
		0,
		"Soldier Ambush Wait"
	},
};

DEFINE_CUSTOM_SCHEDULES( CQ1Soldier )
{
	slSoldierSearch,
	slSoldierSearchFail,
	slSoldierHideReload,
	slSoldierTakeCoverFromEnemy,
	slSoldierAmbushWait
};

IMPLEMENT_CUSTOM_SCHEDULES( CQ1Soldier, CQuakeMonster );

//=========================================================
// FindSearchDestination
//
//=========================================================
BOOL CQ1Soldier :: FindSearchDestination( void )
{
	int i;
	TraceResult tr;

	if ( !WorldGraph.m_fGraphPresent )
	{
		ALERT ( at_aiconsole, "FindSearchDestination: graph not ready!\n" );
		return FALSE;
	}

	if ( WorldGraph.m_iLastActiveIdleSearch >= WorldGraph.m_cNodes )
		WorldGraph.m_iLastActiveIdleSearch = 0;

	int nodeNumber = -1;
	for ( i = 0; i < WorldGraph.m_cNodes ; i++ )
	{
		nodeNumber = (i + WorldGraph.m_iLastActiveIdleSearch) % WorldGraph.m_cNodes;
		CNode &node = WorldGraph.Node( nodeNumber );

		float flDist = (pev->origin - node.m_vecOrigin).Length();
		if(flDist > 768)
		{
			WorldGraph.m_iLastActiveIdleSearch = nodeNumber + 1; // next monster that searches for hint nodes will start where we left off.
			break;
		}
	}

	CNode &node = WorldGraph.Node( nodeNumber );

	// Set the next active search to be a far away node
	for ( i = 0; i < WorldGraph.m_cNodes ; i++ )
	{
		if(i == nodeNumber)
			continue;

		CNode &node_next = WorldGraph.Node( i );
		float flDist = (node_next.m_vecOrigin - node.m_vecOrigin).Length();
		if(flDist > 768)
		{
			WorldGraph.m_iLastActiveIdleSearch = i;
			break;
		}
	}
	if(nodeNumber == -1)
		return FALSE;

	if(MoveToLocation(m_movementActivity, 2, WorldGraph.Node( nodeNumber ).m_vecOrigin))
		return TRUE;

	return FALSE;
}

//=========================================================
// StartTask
//
//=========================================================
void CQ1Soldier :: StartTask( Task_t *pTask )
{
	switch(pTask->iTask)
	{
	case TASK_SOLDIER_SEARCH:
		{
			if(FindSearchDestination())
				TaskComplete();
			else
				TaskFail();
		}
		break;
	case TASK_SOLDIER_WAIT_SEARCH:
		{
			m_nextSearchTime = gpGlobals->time + RANDOM_FLOAT(10, 30);
			TaskComplete();
		}
		break;
	case TASK_RELOAD:
		m_IdealActivity = ACT_RELOAD;
		break;
	default:
		CQuakeMonster::StartTask( pTask );
		break;
	}
}

//=========================================================
// RunTask
//
//=========================================================
void CQ1Soldier :: RunTask( Task_t *pTask )
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
int CQ1Soldier :: ISoundMask ( void) 
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
int	CQ1Soldier :: Classify ( void )
{
	return	CLASS_HUMAN_MILITARY;
}

//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CQ1Soldier :: SetYawSpeed ( void )
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
// CheckRangeAttack1
//=========================================================
BOOL CQ1Soldier :: CheckRangeAttack1 ( float flDot, float flDist )
{
	if ( flDist <= 1024 && flDot >= 0.5 )
	{
		TraceResult tr;
			
		Vector shootOrigin = pev->origin + Vector( 0, 0, 55 );
		CBaseEntity *pEnemy = m_hEnemy;
		Vector shootTarget = ( (pEnemy->BodyTarget( shootOrigin ) - pEnemy->pev->origin) + m_vecEnemyLKP );
		UTIL_TraceLine( shootOrigin, shootTarget, dont_ignore_monsters, ENT(pev), &tr );

		if ( tr.flFraction == 1.0 || (tr.pHit != NULL && CBaseEntity::Instance(tr.pHit) == pEnemy) )
			return TRUE;
		else
			return FALSE;
	}

	return FALSE;
}

//=========================================================
// Shoot
//
//=========================================================
void CQ1Soldier :: Shoot ( void )
{
	if (m_hEnemy == NULL)
		return;

	Vector vecShootOrigin = GetGunPosition();
	Vector vecShootDir = ShootAtEnemy( vecShootOrigin );

	UTIL_MakeVectors ( pev->angles );

	Vector	vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT(40,90) + gpGlobals->v_up * RANDOM_FLOAT(75,200) + gpGlobals->v_forward * RANDOM_FLOAT(-40, 40);
	FireBullets(gSkillData.soldierShotgunPellets, vecShootOrigin, vecShootDir, VECTOR_CONE_15DEGREES, 2048, BULLET_PLAYER_BUCKSHOT, 0 ); // shoot +-7.5 degrees // TODO

	CSoundEnt::InsertSound ( bits_SOUND_COMBAT, pev->origin, 384, 0.3 );

	EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "soldier/sattck1.wav", 1, ATTN_NORM, 0, PITCH_NORM);

	Vector vecSrc = GetGunPosition();
	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, vecSrc );
		WRITE_BYTE(TE_DLIGHT);
		WRITE_COORD(vecSrc.x);	// X
		WRITE_COORD(vecSrc.y);	// Y
		WRITE_COORD(vecSrc.z);	// Z
		WRITE_BYTE( 12 );		// radius * 0.1
		WRITE_BYTE( 255 );		// r
		WRITE_BYTE( 192 );		// g
		WRITE_BYTE( 60 );		// b
		WRITE_BYTE( 2 );		// time * 10
		WRITE_BYTE( 1 );		// decay * 0.1
	MESSAGE_END( );

	// UNDONE: Reload?
	m_cAmmoLoaded--;// take away a bullet!
}
		
//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//
// Returns number of events handled, 0 if none.
//=========================================================
void CQ1Soldier :: HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch( pEvent->event )
	{
	case SOLDIER_AE_SHOOT:
		Shoot();
		break;
	case SOLDIER_AE_RELOAD:
		m_cAmmoLoaded = SOLDIER_CLIP_SIZE;
		ClearConditions(bits_COND_NO_AMMO_LOADED);
		break;
	default:
		CQuakeMonster::HandleAnimEvent( pEvent );
	}
}

//=========================================================
// Spawn
//=========================================================
void CQ1Soldier :: Spawn()
{
	Precache( );

	SET_MODEL(ENT(pev), "models/soldier.mdl");
	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);
	SetAliasData();

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_RED;
	pev->health			= gSkillData.soldierHealth;
	pev->view_ofs		= Vector ( 0, 0, 50 );// position of the eyes relative to monster's origin.
	m_flFieldOfView		= VIEW_FIELD_WIDE; // NOTE: we need a wide field of view so npc will notice player and say hello
	m_MonsterState		= MONSTERSTATE_NONE;

	m_afCapability		= bits_CAP_HEAR | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP | bits_CAP_RANGE_ATTACK1;

	m_cAmmoLoaded		= SOLDIER_CLIP_SIZE;

	MonsterInit();
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CQ1Soldier :: Precache()
{
	PRECACHE_MODEL("models/soldier.mdl");
	
	PRECACHE_SOUND("soldier/death1.wav");
	PRECACHE_SOUND("soldier/guncock.wav");
	PRECACHE_SOUND("soldier/idle.wav");
	PRECACHE_SOUND("soldier/pain1.wav");
	PRECACHE_SOUND("soldier/pain2.wav");
	PRECACHE_SOUND("soldier/sattck1.wav");
	PRECACHE_SOUND("soldier/sight1.wav");

	// every new barney must call this, otherwise
	// when a level is loaded, nobody will talk (time is reset to 0)
	CQuakeMonster::Precache();
}	

//=========================================================
// CheckAmmo - overridden for the grunt because he actually
// uses ammo! (base class doesn't)
//=========================================================
void CQ1Soldier :: CheckAmmo ( void )
{
	if ( m_cAmmoLoaded <= 0 )
	{
		SetConditions(bits_COND_NO_AMMO_LOADED);
	}
}

//=========================================================
// PainSound
//=========================================================
int CQ1Soldier :: TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	return CQuakeMonster::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
}

//=========================================================
// AlertSound
//=========================================================
void CQ1Soldier :: AlertSound ( void )
{
	EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "soldier/sight1.wav", 1, ATTN_NORM, 0, PITCH_NORM);
}

//=========================================================
// PainSound
//=========================================================
void CQ1Soldier :: PainSound ( void )
{
	if (gpGlobals->time < m_painTime)
		return;
	
	m_painTime = gpGlobals->time + RANDOM_FLOAT(0.5, 0.75);

	switch (RANDOM_LONG(0,1))
	{
		case 0: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "soldier/pain1.wav", 1, ATTN_NORM, 0, PITCH_NORM); break;
		case 1: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "soldier/pain2.wav", 1, ATTN_NORM, 0, PITCH_NORM); break;
	}
}

//=========================================================
// DeathSound 
//=========================================================
void CQ1Soldier :: DeathSound ( void )
{
	EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "soldier/death1.wav", 1, ATTN_NORM, 0, PITCH_NORM);
}

//=========================================================
// IdleSound 
//=========================================================
void CQ1Soldier :: IdleSound ( void )
{
	EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "soldier/idle.wav", 1, ATTN_NORM, 0, PITCH_NORM);
}

//=========================================================
// TraceAttack 
//=========================================================
void CQ1Soldier::TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType)
{
	CQuakeMonster::TraceAttack( pevAttacker, flDamage, vecDir, ptr, bitsDamageType );
}

//=========================================================
// Killed 
//=========================================================
void CQ1Soldier::Killed( entvars_t *pevAttacker, int iGib )
{
	if(!m_droppedGun)
	{
		// drop the gun!
		CBaseEntity *pGun = DropItem( "weapon_shotgun", GetGunPosition(), pev->angles );
		m_droppedGun = TRUE;
	}

	CQuakeMonster::Killed( pevAttacker, iGib );
}

//=========================================================
// AI Schedules Specific to this monster
//=========================================================
Schedule_t* CQ1Soldier :: GetScheduleOfType ( int Type )
{
	switch( Type )
	{
	case SCHED_SOLDIER_SEARCH:
		{
			return &slSoldierSearch[0];
		}
		break;
	case SCHED_SOLDIER_SEARCH_FAIL:
		{
			return &slSoldierSearchFail[0];
		}
		break;
	case SCHED_SOLDIER_COVER_AND_RELOAD:
		{
			return &slSoldierHideReload[0];
		}
		break;
	case SCHED_SOLDIER_TAKE_COVER_FROM_ENEMY:
		{
			return &slSoldierTakeCoverFromEnemy[0];
		}
		break;
	case SCHED_SOLDIER_AMBUSH_WAIT:
		{
			return &slSoldierAmbushWait[0];
		}
		break;
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
Schedule_t *CQ1Soldier :: GetSchedule ( void )
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
			if ( HasConditions( bits_COND_LIGHT_DAMAGE) )
			{
				if(RANDOM_LONG(0, 2) == 2)
					return GetScheduleOfType( SCHED_SOLDIER_TAKE_COVER_FROM_ENEMY );
				else
					return GetScheduleOfType( SCHED_SMALL_FLINCH );
			}

			if ( HasConditions( bits_COND_HEAVY_DAMAGE ) )
				return GetScheduleOfType( SCHED_SOLDIER_TAKE_COVER_FROM_ENEMY );

			if ( HasConditions ( bits_COND_NO_AMMO_LOADED ) )
			{
				//!!!KELLY - this individual just realized he's out of bullet ammo. 
				// He's going to try to find cover to run to and reload, but rarely, if 
				// none is available, he'll drop and reload in the open here. 
				return GetScheduleOfType ( SCHED_SOLDIER_COVER_AND_RELOAD );
			}
		}
		break;

	case MONSTERSTATE_ALERT:	
	case MONSTERSTATE_IDLE:
		if ( HasConditions ( bits_COND_NO_AMMO_LOADED ) )
		{
			return GetScheduleOfType ( SCHED_RELOAD );
		}
		if ( m_nextSearchTime < gpGlobals->time && RANDOM_LONG(0, 1))
		{
			return GetScheduleOfType( SCHED_SOLDIER_SEARCH );
		}
		if ( HasConditions(bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE))
		{
			// flinch if hurt
			return GetScheduleOfType( SCHED_SMALL_FLINCH );
		}
		break;
	}
	
	return CQuakeMonster::GetSchedule();
}

MONSTERSTATE CQ1Soldier :: GetIdealState ( void )
{
	return CQuakeMonster::GetIdealState();
}
