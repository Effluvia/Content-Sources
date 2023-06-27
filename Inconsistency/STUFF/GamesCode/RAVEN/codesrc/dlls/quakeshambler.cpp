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
// Beam offsets in model space
//=========================================================
Vector g_vLeftHandOrigin( 19.6, -2.7, 80.4 );
Vector g_vRightHandOrigin( -1.5, -37.2, 70.1 );
Vector g_vHeadCenter( 9.27, 14.37, 50.33 );

#define VEC_SHAMBLER_HULL_MIN	Vector( -32, -32, 0 )
#define VEC_SHAMBLER_HULL_MAX	Vector( 32, 32, 89 )

//=========================================================
// monster-specific tasks
//=========================================================
enum 
{
	TASK_SHAMBLER_SEARCH = LAST_COMMON_TASK + 1,
	TASK_SHAMBLER_WAIT_SEARCH
};

//=========================================================
// monster-specific schedules
//=========================================================
enum 
{
	SCHED_SHAMBLER_SEARCH = LAST_COMMON_SCHEDULE + 1,
	SCHED_SHAMBLER_SEARCH_FAIL
};

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define		SHAMBLER_AE_SMASH	( 3 )
#define		SHAMBLER_AE_SWIPE	( 4 )
#define		SHAMBLER_AE_MAGIC	( 5 )
#define		SHAMBLER_AE_GRUNT	( 6 )
#define		SHAMBLER_AE_BEAMS	( 7 )

class CQ1Shambler : public CQuakeMonster
{
public:
	void Spawn( void );
	void Precache( void );

	int  ISoundMask( void );
	void HandBeams( void );
	void HeadBeam( void );
	void KillBeam( void );

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

	BOOL FindSearchDestination( void );

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
	float m_nextSearchTime;
	float m_nextAttack;

	CBeam *m_pBeam;

	CUSTOM_SCHEDULES;
};

LINK_ENTITY_TO_CLASS( monster_q1shambler, CQ1Shambler );

TYPEDESCRIPTION	CQ1Shambler::m_SaveData[] = 
{
	DEFINE_FIELD( CQ1Shambler, m_painTime, FIELD_TIME ),
	DEFINE_FIELD( CQ1Shambler, m_nextAttack, FIELD_TIME ),
	DEFINE_FIELD( CQ1Shambler, m_nextSearchTime, FIELD_TIME ),
	DEFINE_FIELD( CQ1Shambler, m_pBeam, FIELD_CLASSPTR )
};

IMPLEMENT_SAVERESTORE( CQ1Shambler, CQuakeMonster );

//=========================================================
// InvestigateSound - sends a monster to the location of the
// sound that was just heard, to check things out. 
//=========================================================
Task_t tlShamblerSearch[] =
{
	{ TASK_SET_FAIL_SCHEDULE,		(float)SCHED_SHAMBLER_SEARCH_FAIL	},
	{ TASK_STOP_MOVING,				(float)0							},
	{ TASK_SHAMBLER_SEARCH,			(float)0							},
	{ TASK_FACE_IDEAL,				(float)0							},
	{ TASK_WALK_PATH,				(float)0							},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0							},
	{ TASK_PLAY_SEQUENCE,			(float)ACT_IDLE						},
	{ TASK_WAIT,					(float)10							},
	{ TASK_SHAMBLER_WAIT_SEARCH,	(float)0							}
};

Schedule_t	slShamblerSearch[] =
{
	{ 
		tlShamblerSearch,
		ARRAYSIZE ( tlShamblerSearch ), 
		bits_COND_NEW_ENEMY			|
		bits_COND_SEE_FEAR			|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"Shambler Search"
	},
};

//=========================================================
// InvestigateSound - sends a monster to the location of the
// sound that was just heard, to check things out. 
//=========================================================
Task_t tlShamblerSearchFail[] =
{
	{ TASK_STOP_MOVING,				(float)0							},
	{ TASK_SHAMBLER_WAIT_SEARCH,		(float)0							}
};

Schedule_t	slShamblerSearchFail[] =
{
	{ 
		tlShamblerSearchFail,
		ARRAYSIZE ( tlShamblerSearchFail ), 
		bits_COND_NEW_ENEMY			|
		bits_COND_SEE_FEAR			|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"Shambler Search Fail"
	},
};

DEFINE_CUSTOM_SCHEDULES( CQ1Shambler )
{
	slShamblerSearch,
	slShamblerSearchFail
};

IMPLEMENT_CUSTOM_SCHEDULES( CQ1Shambler, CQuakeMonster );

//=========================================================
// FindSearchDestination
//
//=========================================================
BOOL CQ1Shambler :: FindSearchDestination( void )
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
void CQ1Shambler :: StartTask( Task_t *pTask )
{
	KillBeam();
	
	switch(pTask->iTask)
	{
	case TASK_SHAMBLER_SEARCH:
		{
			if(FindSearchDestination())
				TaskComplete();
			else
				TaskFail();
		}
		break;
	case TASK_SHAMBLER_WAIT_SEARCH:
		{
			m_nextSearchTime = gpGlobals->time + RANDOM_FLOAT(10, 30);
			TaskComplete();
		}
		break;
	default:
		CQuakeMonster::StartTask( pTask );
		break;
	}
}

//=========================================================
// Kick
//
//=========================================================
CBaseEntity *CQ1Shambler :: Kick( void )
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
void CQ1Shambler :: RunTask( Task_t *pTask )
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
int CQ1Shambler :: ISoundMask ( void) 
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
int	CQ1Shambler :: Classify ( void )
{
	return	CLASS_HUMAN_MILITARY;
}

//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CQ1Shambler :: SetYawSpeed ( void )
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
BOOL CQ1Shambler :: CheckRangeAttack1 ( float flDot, float flDist )
{
	if( m_nextAttack > gpGlobals->time )
		return FALSE;

	if ( flDist <= 1024 && flDot >= 0.5 )
	{
		TraceResult tr;
			
		Vector shootOrigin = pev->origin + Vector( 0, 0, 86 );
		CBaseEntity *pEnemy = m_hEnemy;
		Vector shootTarget = ( (pEnemy->BodyTarget( shootOrigin ) - pEnemy->pev->origin) + m_vecEnemyLKP );
		UTIL_TraceLine( shootOrigin, shootTarget, dont_ignore_monsters, ENT(pev), &tr );

		if ( tr.flFraction == 1.0 || (tr.pHit != NULL && CBaseEntity::Instance(tr.pHit) == pEnemy) )
		{
			m_nextAttack = gpGlobals->time + RANDOM_FLOAT(0.5, 1);
			return TRUE;
		}
		else
			return FALSE;
	}

	return FALSE;
}

//=========================================================
// CheckMeleeAttack1
//=========================================================
BOOL CQ1Shambler :: CheckMeleeAttack1 ( float flDot, float flDist )
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

	if ( flDist <= 96 && flDot >= 0.7	&& 
		 pEnemy->Classify() != CLASS_ALIEN_BIOWEAPON &&
		 pEnemy->Classify() != CLASS_PLAYER_BIOWEAPON )
	{
		return TRUE;
	}
	return FALSE;
}

//=========================================================
// KillBeam
//
//=========================================================
void CQ1Shambler :: KillBeam ( void )
{
	if (m_pBeam)
	{
		UTIL_Remove( m_pBeam );
		m_pBeam = NULL;
	}
}

//=========================================================
// HandBeams
//
//=========================================================
void CQ1Shambler :: HandBeams ( void )
{
	// Kill previous beam
	KillBeam();

	float aliasScale = CVAR_GET_FLOAT("r_aliasscale");
	float aliasOffset = CVAR_GET_FLOAT("r_aliasoffset");

	float angleMatrix[3][4];
	UTIL_AngleMatrix(pev->angles, angleMatrix);

	Vector vecLeftHand;
	UTIL_VectorTransform(g_vLeftHandOrigin, angleMatrix, vecLeftHand);
	vecLeftHand[2] += aliasOffset;
	UTIL_VectorScale(vecLeftHand, aliasScale, vecLeftHand);
	UTIL_VectorAdd(vecLeftHand, pev->origin, vecLeftHand);

	Vector vecRightHand;
	UTIL_VectorTransform(g_vRightHandOrigin, angleMatrix, vecRightHand);
	vecRightHand[2] += aliasOffset;
	UTIL_VectorScale(vecRightHand, aliasScale, vecRightHand);
	UTIL_VectorAdd(vecRightHand, pev->origin, vecRightHand);

	// Create a single beam
	m_pBeam = CBeam::BeamCreate( "sprites/lgtning.spr", 60 );
	if (!m_pBeam)
		return;

	m_pBeam->SetStartPos(vecLeftHand);
	m_pBeam->SetEndPos(vecRightHand);
	m_pBeam->SetColor( 96, 96, 250 );
	m_pBeam->SetBrightness( 255 );
	m_pBeam->SetNoise( 20 );

	// Create a dynlight
	Vector vecSrc = GetGunPosition();
	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, vecSrc );
		WRITE_BYTE(TE_DLIGHT);
		WRITE_COORD(vecSrc.x);	// X
		WRITE_COORD(vecSrc.y);	// Y
		WRITE_COORD(vecSrc.z);	// Z
		WRITE_BYTE( 30 );		// radius * 0.1
		WRITE_BYTE( 96 );		// r
		WRITE_BYTE( 96 );		// g
		WRITE_BYTE( 255 );		// b
		WRITE_BYTE( 4 );		// time * 10
		WRITE_BYTE( 1 );		// decay * 0.1
	MESSAGE_END( );

	UTIL_EmitAmbientSound( ENT(pev), pev->origin, "shambler/sattck1.wav", 0.5, ATTN_NORM, 0, RANDOM_LONG( 100, 110 ) );
}
	
//=========================================================
// HeadBeam
//
//=========================================================
void CQ1Shambler :: HeadBeam ( void )
{
	// Kill previous beam
	KillBeam();
	
	TraceResult tr;
	CBaseEntity *pEntity;

	Vector vecSrc = pev->origin + gpGlobals->v_up * 36;
	Vector vecAim = ShootAtEnemy( vecSrc );

	float deflection = 0.01;
	vecAim = vecAim + gpGlobals->v_right * RANDOM_FLOAT( 0, deflection ) + gpGlobals->v_up * RANDOM_FLOAT( -deflection, deflection );
	UTIL_TraceLine ( vecSrc, vecSrc + vecAim * 1024, dont_ignore_monsters, ENT( pev ), &tr);

	pEntity = CBaseEntity::Instance(tr.pHit);
	if (pEntity != NULL && pEntity->pev->takedamage)
		pEntity->TraceAttack( pev, gSkillData.shamblerDmgZap, vecAim, &tr, DMG_SHOCK ); // TODO

	ApplyMultiDamage(pev, pev);

	// Create beams
	float aliasScale = CVAR_GET_FLOAT("r_aliasscale");
	float aliasOffset = CVAR_GET_FLOAT("r_aliasoffset");

	float angleMatrix[3][4];
	UTIL_AngleMatrix(pev->angles, angleMatrix);

	Vector vecHeadCenter;
	UTIL_VectorTransform(g_vHeadCenter, angleMatrix, vecHeadCenter);
	vecHeadCenter[2] += aliasOffset;
	UTIL_VectorScale(vecHeadCenter, aliasScale, vecHeadCenter);
	UTIL_VectorAdd(vecHeadCenter, pev->origin, vecHeadCenter);

	m_pBeam = CBeam::BeamCreate( "sprites/lgtning.spr", 180 );
	if (!m_pBeam)
		return;

	m_pBeam->SetStartPos(vecHeadCenter);
	m_pBeam->SetEndPos(tr.vecEndPos);
	m_pBeam->SetColor( 96, 96, 250 );
	m_pBeam->SetBrightness( 255 );
	m_pBeam->SetNoise( 50 );

	UTIL_EmitAmbientSound( ENT(pev), tr.vecEndPos, "shambler/sboom.wav", 0.5, ATTN_NORM, 0, RANDOM_LONG( 100, 110 ) );

	vecSrc = GetGunPosition();
	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, vecSrc );
		WRITE_BYTE(TE_DLIGHT);
		WRITE_COORD(vecSrc.x);	// X
		WRITE_COORD(vecSrc.y);	// Y
		WRITE_COORD(vecSrc.z);	// Z
		WRITE_BYTE( 40 );		// radius * 0.1
		WRITE_BYTE( 96 );		// r
		WRITE_BYTE( 96 );		// g
		WRITE_BYTE( 250 );		// b
		WRITE_BYTE( 4 );		// time * 10
		WRITE_BYTE( 1 );		// decay * 0.1
	MESSAGE_END( );

	m_nextAttack = gpGlobals->time + RANDOM_FLOAT(1, 3);
}

//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//
// Returns number of events handled, 0 if none.
//=========================================================
void CQ1Shambler :: HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch( pEvent->event )
	{
	case SHAMBLER_AE_BEAMS:
		HandBeams();
		break;

	case SHAMBLER_AE_GRUNT:
		{
			switch (RANDOM_LONG(0,1))
			{
				case 0: EMIT_SOUND_DYN( ENT(pev), CHAN_BODY, "shambler/melee1.wav", 1, ATTN_NORM, 0, PITCH_NORM); break;
				case 1: EMIT_SOUND_DYN( ENT(pev), CHAN_BODY, "shambler/melee2.wav", 1, ATTN_NORM, 0, PITCH_NORM); break;
			}
		}
		break;

	case SHAMBLER_AE_SWIPE:
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
				pHurt->TakeDamage( pev, pev, gSkillData.shamblerDmgSwipe, DMG_CLUB );
			}
		}
		break;

	case SHAMBLER_AE_SMASH:
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
				pHurt->TakeDamage( pev, pev, gSkillData.shamblerDmgSmash, DMG_CLUB );
			}
		}
		break;

	case SHAMBLER_AE_MAGIC:
		HeadBeam();
		break;

	default:
		CQuakeMonster::HandleAnimEvent( pEvent );
	}
}

//=========================================================
// Spawn
//=========================================================
void CQ1Shambler :: Spawn()
{
	Precache( );

	SET_MODEL(ENT(pev), "models/shambler.mdl");
	UTIL_SetSize(pev, VEC_SHAMBLER_HULL_MIN, VEC_SHAMBLER_HULL_MAX);
	SetAliasData();

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_RED;
	pev->health			= gSkillData.shamblerHealth;
	pev->view_ofs		= Vector ( 0, 0, 85 );// position of the eyes relative to monster's origin.
	m_flFieldOfView		= VIEW_FIELD_WIDE; // NOTE: we need a wide field of view so npc will notice player and say hello
	m_MonsterState		= MONSTERSTATE_NONE;

	m_afCapability		= bits_CAP_HEAR | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP | bits_CAP_RANGE_ATTACK1;

	MonsterInit();
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CQ1Shambler :: Precache()
{
	PRECACHE_MODEL("models/shambler.mdl");
	
	PRECACHE_SOUND("shambler/melee1.wav");
	PRECACHE_SOUND("shambler/melee2.wav");
	PRECACHE_SOUND("shambler/sattck1.wav");
	PRECACHE_SOUND("shambler/sboom.wav");
	PRECACHE_SOUND("shambler/shurt2.wav");
	PRECACHE_SOUND("shambler/sidle.wav");
	PRECACHE_SOUND("shambler/smack.wav");
	PRECACHE_SOUND("shambler/ssight.wav");
	PRECACHE_SOUND("shambler/sdeath.wav");

	PRECACHE_MODEL("sprites/lgtning.spr");

	// every new barney must call this, otherwise
	// when a level is loaded, nobody will talk (time is reset to 0)
	CQuakeMonster::Precache();
}	

//=========================================================
// PainSound
//=========================================================
int CQ1Shambler :: TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	return CQuakeMonster::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
}

//=========================================================
// AlertSound
//=========================================================
void CQ1Shambler :: AlertSound ( void )
{
	EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "shambler/ssight.wav", 1, ATTN_NORM, 0, PITCH_NORM);
}

//=========================================================
// PainSound
//=========================================================
void CQ1Shambler :: PainSound ( void )
{
	if (gpGlobals->time < m_painTime)
		return;
	
	m_painTime = gpGlobals->time + RANDOM_FLOAT(0.5, 0.75);

	EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "shambler/shurt2.wav", 1, ATTN_NORM, 0, PITCH_NORM);
}

//=========================================================
// DeathSound 
//=========================================================
void CQ1Shambler :: DeathSound ( void )
{
	EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "shambler/sdeath.wav", 1, ATTN_NORM, 0, PITCH_NORM);
}

//=========================================================
// IdleSound 
//=========================================================
void CQ1Shambler :: IdleSound ( void )
{
	EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "shambler/sidle.wav", 1, ATTN_NORM, 0, PITCH_NORM);
}

//=========================================================
// TraceAttack 
//=========================================================
void CQ1Shambler::TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType)
{
	CQuakeMonster::TraceAttack( pevAttacker, flDamage, vecDir, ptr, bitsDamageType );
}

//=========================================================
// Killed 
//=========================================================
void CQ1Shambler::Killed( entvars_t *pevAttacker, int iGib )
{
	KillBeam();
	CQuakeMonster::Killed( pevAttacker, iGib );
}

//=========================================================
// AI Schedules Specific to this monster
//=========================================================
Schedule_t* CQ1Shambler :: GetScheduleOfType ( int Type )
{
	switch( Type )
	{
	case SCHED_SHAMBLER_SEARCH:
		{
			return &slShamblerSearch[0];
		}
		break;
	case SCHED_SHAMBLER_SEARCH_FAIL:
		{
			return &slShamblerSearchFail[0];
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
Schedule_t *CQ1Shambler :: GetSchedule ( void )
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
		if ( m_nextSearchTime < gpGlobals->time && RANDOM_LONG(0, 1))
		{
			return GetScheduleOfType( SCHED_SHAMBLER_SEARCH );
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

MONSTERSTATE CQ1Shambler :: GetIdealState ( void )
{
	return CQuakeMonster::GetIdealState();
}
