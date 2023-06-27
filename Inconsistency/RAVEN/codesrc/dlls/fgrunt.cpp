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

//=========================================================
// monster template
//=========================================================

#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"monsters.h"
#include	"animation.h"
#include	"rcallymonster.h"
#include	"schedule.h"
#include	"defaultai.h"
#include	"scripted.h"
#include	"weapons.h"
#include	"soundent.h"
#include	"customentity.h"
#include	"plane.h"
#include	"pm_materials.h"

//=========================================================
//
//=========================================================
#define	FGRUNT_CLIP_SIZE				32 // how many bullets in a clip? - NOTE: 3 round burst sound, so keep as 3 * x!
#define FGRUNT_LIMP_HEALTH				20

#define FGRUNT_MEDIC_WAIT				5 // Wait ten seconds before calling for medic again.

#define FGRUNT_TYPE_NORMAL			0
#define FGRUNT_TYPE_MEDIC			1
#define FGRUNT_TYPE_ENGINEER		2

#define FGRUNT_9MMAR				( 1 << 0)
#define FGRUNT_HANDGRENADE			( 1 << 1)
#define FGRUNT_GRENADELAUNCHER		( 1 << 2)
#define FGRUNT_SHOTGUN				( 1 << 3)
#define FGRUNT_M249					( 1 << 4)

// Weapon group
#define MN_GUN_GROUP				3
#define MN_GUN_MP5					0
#define MN_GUN_SHOTGUN				1
#define MN_GUN_SAW					2
#define MN_GUN_TORCH				3
#define MN_GUN_NEEDLE				4
#define MN_GUN_NONE					5

//=========================================================
// monster-specific conditions
//=========================================================
#define bits_COND_FGRUNT_NOFIRE	( bits_COND_SPECIAL1 )

//=========================================================
// monster-specific tasks
//=========================================================
enum 
{
	TASK_FGRUNT_FACE_TOSS_DIR = LAST_RCALLYMONSTER_TASK + 1,
	TASK_FGRUNT_CHECK_FIRE,
	TASK_FGRUNT_STOP_FOLLOWING,
};

//=========================================================
// monster heads
//=========================================================
// Torso group
#define MN_TORSO_GROUP				2
#define MN_TORSO_NORMAL				0
#define MN_TORSO_SAW				1
#define MN_TORSO_SHOTGUN			2

// Head group
#define MN_HEAD_GROUP				1
#define MN_HEAD_MASK				0
#define MN_HEAD_CMDR_WHITE			1
#define MN_HEAD_SHOTGUN				2
#define MN_HEAD_SAW_WHT				3
#define MN_HEAD_SAW_BLK				4
#define MN_HEAD_MP					5
#define MN_HEAD_CMDR_OLD			6
#define MN_HEAD_CMDR_BLACK			7

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define		FGRUNT_AE_RELOAD		( 2 )
#define		FGRUNT_AE_KICK			( 3 )
#define		FGRUNT_AE_BURST1		( 4 )
#define		FGRUNT_AE_BURST2		( 5 ) 
#define		FGRUNT_AE_BURST3		( 6 ) 
#define		FGRUNT_AE_GREN_TOSS		( 7 )
#define		FGRUNT_AE_GREN_LAUNCH	( 8 )
#define		FGRUNT_AE_GREN_DROP		( 9 )
#define		FGRUNT_AE_CAUGHT_ENEMY	( 10) // grunt established sight with an enemy (player only) that had previously eluded the squad.
#define		FGRUNT_AE_DROP_GUN		( 11) // grunt (probably dead) is dropping his mp5.

//=========================================================
// monster-specific schedule types
//=========================================================
enum
{
	SCHED_FGRUNT_SUPPRESS = LAST_RCALLYMONSTER_SCHEDULE + 1,
	SCHED_FGRUNT_ESTABLISH_LINE_OF_FIRE,// move to a location to set up an attack against the enemy. (usually when a friendly is in the way).
	SCHED_FGRUNT_COVER_AND_RELOAD,
	SCHED_FGRUNT_SWEEP,
	SCHED_FGRUNT_FOUND_ENEMY,
	SCHED_FGRUNT_REPEL,
	SCHED_FGRUNT_REPEL_ATTACK,
	SCHED_FGRUNT_REPEL_LAND,
	SCHED_FGRUNT_WAIT_FACE_ENEMY,
	SCHED_FGRUNT_TAKECOVER_FAILED,// special schedule type that forces analysis of conditions and picks the best possible schedule to recover from this type of failure.
	SCHED_FGRUNT_ELOF_FAIL
};

class CFGrunt : public CRCAllyMonster
{
public:
	void Spawn( void );
	void Precache( void );
	void SetYawSpeed( void );
	int  ISoundMask( void );
	int  Classify ( void );
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	void CheckAmmo ( void );
	void SetActivity ( Activity NewActivity );
	void RunTask( Task_t *pTask );
	void StartTask( Task_t *pTask );
	void KeyValue( KeyValueData *pkvd );
	virtual int	ObjectCaps( void ) { return CRCAllyMonster :: ObjectCaps() | FCAP_IMPULSE_USE; }
	BOOL FCanCheckAttacks ( void );
	BOOL CheckRangeAttack1 ( float flDot, float flDist );
	BOOL CheckRangeAttack2 ( float flDot, float flDist );
	BOOL CheckMeleeAttack1 ( float flDot, float flDist );
	void DeclineFollowing( void );
	void PrescheduleThink ( void );
	Vector GetGunPosition( void );
	void Shoot ( void );
	void Shotgun ( void );
	void M249 ( void );
	// Override these to set behavior
	CBaseEntity	*Kick( void );
	Schedule_t *GetScheduleOfType ( int Type );
	Schedule_t *GetSchedule ( void );
	MONSTERSTATE GetIdealState ( void );

	BOOL ShouldSeekShootingSpot( void );

	void DeathSound( void );
	void PainSound( void );
	void GibMonster( void );
	void TalkInit( void );

	BOOL NoFriendlyFire( void );
	BOOL FOkToSpeak( void );
	void JustSpoke( void );

	void TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType);
	int TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType );
	void Killed( entvars_t *pevAttacker, int iGib );
	int IRelationship ( CBaseEntity *pTarget );

	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	// UNDONE: What is this for?  It isn't used?
	float	m_flPlayerDamage;// how much pain has the player inflicted on me?

	// checking the feasibility of a grenade toss is kind of costly, so we do it every couple of seconds,
	// not every server frame.
	float m_flNextGrenadeCheck;
	float m_flNextPainTime;
	float m_flLastEnemySightTime;

	Vector	m_vecTossVelocity;

	BOOL	m_fThrowGrenade;
	BOOL	m_fStanding;
	BOOL	m_fFirstEncounter;// only put on the handsign show in the squad's first encounter.
	int		m_cClipSize;

	int		m_iSentence;
	int		m_iHead;
	static const char *pGruntSentences[];

	int		m_iBrassShell;
	int		m_iShotgunShell;
	int		m_iSawShell;
	int		m_iSawLink;

	CUSTOM_SCHEDULES;
};

LINK_ENTITY_TO_CLASS( monster_human_grunt_ally, CFGrunt );

TYPEDESCRIPTION	CFGrunt::m_SaveData[] = 
{
	DEFINE_FIELD( CFGrunt, m_flNextGrenadeCheck, FIELD_TIME ),
	DEFINE_FIELD( CFGrunt, m_flNextPainTime, FIELD_TIME ),
	DEFINE_FIELD( CFGrunt, m_vecTossVelocity, FIELD_VECTOR ),
	DEFINE_FIELD( CFGrunt, m_fThrowGrenade, FIELD_BOOLEAN ),
	DEFINE_FIELD( CFGrunt, m_fStanding, FIELD_BOOLEAN ),
	DEFINE_FIELD( CFGrunt, m_fFirstEncounter, FIELD_BOOLEAN ),
	DEFINE_FIELD( CFGrunt, m_cClipSize, FIELD_INTEGER ),
	DEFINE_FIELD( CFGrunt, m_iHead, FIELD_INTEGER ),
};

IMPLEMENT_SAVERESTORE( CFGrunt, CRCAllyMonster );
const char *CFGrunt::pGruntSentences[] = 
{
	"FG_GREN", // grenade scared grunt
	"FG_ALERT", // sees player
	"FG_MONSTER", // sees monster
	"FG_COVER", // running to cover
	"FG_THROW", // about to throw grenade
	"FG_CHARGE",  // running out to get the enemy
	"FG_TAUNT", // say rude things
};

enum
{
	FGRUNT_SENT_NONE = -1,
	FGRUNT_SENT_GREN = 0,
	FGRUNT_SENT_ALERT,
	FGRUNT_SENT_MONSTER,
	FGRUNT_SENT_COVER,
	FGRUNT_SENT_THROW,
	FGRUNT_SENT_CHARGE,
	FGRUNT_SENT_TAUNT,
} FGRUNT_SENTENCE_TYPES;

//=========================================================
// KeyValue
//
// !!! netname entvar field is used in squadmonster for groupname!!!
//=========================================================
void CFGrunt :: KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "body"))
	{
		m_iHead = atoi( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else
	{
		CBaseMonster::KeyValue( pkvd );
	}
}
//=========================================================
// NoFriendlyFire - checks for possibility of friendly fire
//
// Builds a large box in front of the grunt and checks to see 
// if any squad members are in that box. 
//=========================================================
BOOL CFGrunt :: NoFriendlyFire( void )
{
	CPlane	backPlane;
	CPlane  leftPlane;
	CPlane	rightPlane;

	Vector	vecLeftSide;
	Vector	vecRightSide;
	Vector	v_left;

	//!!!BUGBUG - to fix this, the planes must be aligned to where the monster will be firing its gun, not the direction it is facing!!!

	if ( m_hEnemy != NULL )
	{
		UTIL_MakeVectors ((m_hEnemy->BodyTarget(GetGunPosition())-GetGunPosition()));
	}
	else
	{
		// if there's no enemy, pretend there's a friendly in the way, so the grunt won't shoot.
		return FALSE;
	}
	
	vecLeftSide = pev->origin - ( gpGlobals->v_right * ( pev->size.x * 1.5 ) );
	vecRightSide = pev->origin + ( gpGlobals->v_right * ( pev->size.x * 1.5 ) );
	v_left = gpGlobals->v_right * -1;

	leftPlane.InitializePlane ( gpGlobals->v_right, vecLeftSide );
	rightPlane.InitializePlane ( v_left, vecRightSide );
	backPlane.InitializePlane ( gpGlobals->v_forward, pev->origin );

	if ( !m_afMemory & bits_MEMORY_PROVOKED )
	{
		CBaseEntity *pPlayer = FindNearestFriend(TRUE);
		if(pPlayer)
		{
			edict_t	*pentPlayer = pPlayer->edict();
			Vector center = ((pentPlayer->v.mins+pentPlayer->v.maxs)/2) + pentPlayer->v.origin;

			if (backPlane.PointInFront  ( center ) &&
				leftPlane.PointInFront  ( center ) && 
				rightPlane.PointInFront ( center ) )
			{
				// the player is in the check volume! Don't shoot!
				return FALSE;
			}
		}
	}

	int i;
	CBaseEntity *pFriend = NULL;
	for ( i = 0; i < TLK_CFRIENDS; i++ )
	{
		while (pFriend = EnumFriends( pFriend, i, TRUE ))
		{
			CBaseMonster *pMonster = pFriend->MyMonsterPointer();
			if ( pMonster->IsAlive() && pMonster != this )
			{
				Vector center = ((pMonster->pev->mins+pMonster->pev->maxs)/2)+pMonster->pev->origin;

				if ( backPlane.PointInFront  ( center ) &&
					 leftPlane.PointInFront  ( center ) && 
					 rightPlane.PointInFront ( center ) )
				{
					// this guy is in the check volume! Don't shoot!
					return FALSE;
				}
			}
		}
	}

	return TRUE;
}

void CFGrunt::Killed( entvars_t *pevAttacker, int iGib )
{
	SetUse( NULL );

	CRCAllyMonster::Killed( pevAttacker, iGib );
}

//=========================================================
// ShouldSeekShootingSpot - See if we want to shoot or grab
//=========================================================
BOOL CFGrunt::ShouldSeekShootingSpot( void )
{
	return (pev->weapons != 0);
}

//=========================================================
// someone else is talking - don't speak
//=========================================================
BOOL CFGrunt :: FOkToSpeak( void )
{
// if someone else is talking, don't speak
	if (gpGlobals->time <= CRCAllyMonster::g_talkWaitTime)
		return FALSE;

	// if in the grip of a barnacle, don't speak
	if ( m_MonsterState == MONSTERSTATE_PRONE || m_IdealMonsterState == MONSTERSTATE_PRONE )
	{
		return FALSE;
	}

	// if not alive, certainly don't speak
	if ( pev->deadflag != DEAD_NO )
	{
		return FALSE;
	}

	if ( pev->spawnflags & SF_MONSTER_GAG )
	{
		if ( m_MonsterState != MONSTERSTATE_COMBAT )
		{
			// no talking outside of combat if gagged.
			return FALSE;
		}
	}
	
	return TRUE;
}
//=========================================================
//=========================================================
void CFGrunt :: JustSpoke( void )
{
	CRCAllyMonster::g_talkWaitTime = gpGlobals->time + RANDOM_FLOAT(1.5, 2.0);
	m_iSentence = FGRUNT_SENT_NONE;
}
//=========================================================
// IRelationship - overridden because Male Assassins are 
// Human Grunt's nemesis.
//=========================================================
int CFGrunt::IRelationship ( CBaseEntity *pTarget )
{
	if ( FClassnameIs( pTarget->pev, "monster_male_assassin" ) )
	{
		return R_NM;
	}

	return CRCAllyMonster::IRelationship( pTarget );
}

//=========================================================
// AI Schedules Specific to this monster
//=========================================================
Task_t	tlFGruntGruntFollow[] =
{
	{ TASK_MOVE_TO_TARGET_RANGE,(float)128		},	// Move within 128 of target ent (client)
	{ TASK_SET_SCHEDULE,		(float)SCHED_TARGET_FACE },
};

Schedule_t	slFGruntGruntFollow[] =
{
	{
		tlFGruntGruntFollow,
		ARRAYSIZE ( tlFGruntGruntFollow ),
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_HEAR_SOUND |
		bits_COND_PROVOKED,
		bits_SOUND_DANGER,
		"Follow"
	},
};
Task_t	tlFGruntGruntFaceTarget[] =
{
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_FACE_TARGET,			(float)0		},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_SET_SCHEDULE,		(float)SCHED_TARGET_CHASE },
};

Schedule_t	slFGruntGruntFaceTarget[] =
{
	{
		tlFGruntGruntFaceTarget,
		ARRAYSIZE ( tlFGruntGruntFaceTarget ),
		bits_COND_CLIENT_PUSH	|
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_CLIENT_PUSH |
		bits_COND_HEAR_SOUND |
		bits_COND_PROVOKED,
		bits_SOUND_DANGER,
		"FaceTarget"
	},
};


Task_t	tlFGruntGruntIdleStand[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_WAIT,				(float)2		}, // repick IDLESTAND every two seconds.
	{ TASK_TLK_HEADRESET,		(float)0		}, // reset head position
};

Schedule_t	slFGruntGruntIdleStand[] =
{
	{ 
		tlFGruntGruntIdleStand,
		ARRAYSIZE ( tlFGruntGruntIdleStand ), 
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_HEAR_SOUND	|
		bits_COND_SMELL			|
		bits_COND_CLIENT_PUSH	|
		bits_COND_PROVOKED,

		bits_SOUND_COMBAT		|// sound flags - change these, and you'll break the talking code.
		//bits_SOUND_PLAYER		|
		//bits_SOUND_WORLD		|
		
		bits_SOUND_DANGER		|
		bits_SOUND_MEAT			|// scents
		bits_SOUND_CARCASS		|
		bits_SOUND_GARBAGE,
		"IdleStand"
	},
};
//=========================================================
// FGruntFail
//=========================================================
Task_t	tlFGruntGruntFail[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_WAIT,				(float)0.5		},
	{ TASK_WAIT_PVS,			(float)0		},
};

Schedule_t	slFGruntGruntFail[] =
{
	{
		tlFGruntGruntFail,
		ARRAYSIZE ( tlFGruntGruntFail ),
		bits_COND_CAN_RANGE_ATTACK1 |
		bits_COND_CAN_RANGE_ATTACK2 |
		bits_COND_CAN_MELEE_ATTACK1 |
		bits_COND_CAN_MELEE_ATTACK2,
		0,
		"FGrunt Fail"
	},
};

//=========================================================
// FGrunt Combat Fail
//=========================================================
Task_t	tlFGruntGruntCombatFail[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_WAIT_FACE_ENEMY,		(float)0.5		},
	{ TASK_WAIT_PVS,			(float)0		},
};

Schedule_t	slFGruntGruntCombatFail[] =
{
	{
		tlFGruntGruntCombatFail,
		ARRAYSIZE ( tlFGruntGruntCombatFail ),
		bits_COND_CAN_RANGE_ATTACK1	|
		bits_COND_CAN_RANGE_ATTACK2,
		0,
		"FGrunt Combat Fail"
	},
};

//=========================================================
// Victory dance!
//=========================================================
Task_t	tlFGruntGruntVictoryDance[] =
{
	{ TASK_SET_FAIL_SCHEDULE,				(float)SCHED_FAIL			},
	{ TASK_STOP_MOVING,						(float)0					},
	{ TASK_FACE_ENEMY,						(float)0					},
	{ TASK_WAIT,							(float)1.5					},
	{ TASK_GET_PATH_TO_ENEMY_CORPSE,		(float)0					},
	{ TASK_WALK_PATH,						(float)0					},
	{ TASK_WAIT_FOR_MOVEMENT,				(float)0					},
	{ TASK_FACE_ENEMY,						(float)0					},
	{ TASK_PLAY_SEQUENCE,					(float)ACT_VICTORY_DANCE	},
};

Schedule_t	slFGruntGruntVictoryDance[] =
{
	{ 
		tlFGruntGruntVictoryDance,
		ARRAYSIZE ( tlFGruntGruntVictoryDance ), 
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE,
		0,
		"FGruntVictoryDance"
	},
};

//=========================================================
// Establish line of fire - move to a position that allows
// the grunt to attack.
//=========================================================
Task_t tlFGruntGruntEstablishLineOfFire[] = 
{
	{ TASK_SET_FAIL_SCHEDULE,	(float)SCHED_FGRUNT_ELOF_FAIL	},
	{ TASK_GET_PATH_TO_ENEMY,	(float)0						},
	{ TASK_RUN_PATH,			(float)0						},
	{ TASK_WAIT_FOR_MOVEMENT,	(float)0						},
};

Schedule_t slFGruntGruntEstablishLineOfFire[] =
{
	{ 
		tlFGruntGruntEstablishLineOfFire,
		ARRAYSIZE ( tlFGruntGruntEstablishLineOfFire ),
		bits_COND_NEW_ENEMY			|
		bits_COND_ENEMY_DEAD		|
		bits_COND_CAN_RANGE_ATTACK1	|
		bits_COND_CAN_MELEE_ATTACK1	|
		bits_COND_CAN_RANGE_ATTACK2	|
		bits_COND_CAN_MELEE_ATTACK2	|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"FGruntEstablishLineOfFire"
	},
};

//=========================================================
// FGruntFoundEnemy - FGrunt established sight with an enemy
// that was hiding from the squad.
//=========================================================
Task_t	tlFGruntGruntFoundEnemy[] =
{
	{ TASK_STOP_MOVING,				0							},
	{ TASK_FACE_ENEMY,				(float)0					},
	{ TASK_PLAY_SEQUENCE_FACE_ENEMY,(float)ACT_SIGNAL1			},
};

Schedule_t	slFGruntGruntFoundEnemy[] =
{
	{ 
		tlFGruntGruntFoundEnemy,
		ARRAYSIZE ( tlFGruntGruntFoundEnemy ), 
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"FGruntFoundEnemy"
	},
};

//=========================================================
// GruntCombatFace Schedule
//=========================================================
Task_t	tlFGruntGruntCombatFace1[] =
{
	{ TASK_STOP_MOVING,				0							},
	{ TASK_SET_ACTIVITY,			(float)ACT_IDLE				},
	{ TASK_FACE_ENEMY,				(float)0					},
	{ TASK_WAIT,					(float)1.5					},
	{ TASK_SET_SCHEDULE,			(float)SCHED_FGRUNT_SWEEP	},
};

Schedule_t	slFGruntGruntCombatFace[] =
{
	{ 
		tlFGruntGruntCombatFace1,
		ARRAYSIZE ( tlFGruntGruntCombatFace1 ), 
		bits_COND_NEW_ENEMY				|
		bits_COND_ENEMY_DEAD			|
		bits_COND_CAN_RANGE_ATTACK1		|
		bits_COND_CAN_RANGE_ATTACK2,
		0,
		"Combat Face"
	},
};

//=========================================================
// Suppressing fire - don't stop shooting until the clip is
// empty or FGrunt gets hurt.
//=========================================================
Task_t	tlFGruntGruntSignalSuppress[] =
{
	{ TASK_STOP_MOVING,						0						},
	{ TASK_FACE_IDEAL,						(float)0				},
	{ TASK_PLAY_SEQUENCE_FACE_ENEMY,		(float)ACT_SIGNAL2		},
	{ TASK_FACE_ENEMY,						(float)0				},
	{ TASK_FGRUNT_CHECK_FIRE,			(float)0				},
	{ TASK_RANGE_ATTACK1,					(float)0				},
	{ TASK_FACE_ENEMY,						(float)0				},
	{ TASK_FGRUNT_CHECK_FIRE,			(float)0				},
	{ TASK_RANGE_ATTACK1,					(float)0				},
	{ TASK_FACE_ENEMY,						(float)0				},
	{ TASK_FGRUNT_CHECK_FIRE,			(float)0				},
	{ TASK_RANGE_ATTACK1,					(float)0				},
	{ TASK_FACE_ENEMY,						(float)0				},
	{ TASK_FGRUNT_CHECK_FIRE,			(float)0				},
	{ TASK_RANGE_ATTACK1,					(float)0				},
	{ TASK_FACE_ENEMY,						(float)0				},
	{ TASK_FGRUNT_CHECK_FIRE,			(float)0				},
	{ TASK_RANGE_ATTACK1,					(float)0				},
};

Schedule_t	slFGruntGruntSignalSuppress[] =
{
	{ 
		tlFGruntGruntSignalSuppress,
		ARRAYSIZE ( tlFGruntGruntSignalSuppress ), 
		bits_COND_ENEMY_DEAD		|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_HEAR_SOUND		|
		bits_COND_FGRUNT_NOFIRE		|
		bits_COND_NO_AMMO_LOADED,

		bits_SOUND_DANGER,
		"SignalSuppress"
	},
};

Task_t	tlFGruntGruntSuppress[] =
{
	{ TASK_STOP_MOVING,			0							},
	{ TASK_FACE_ENEMY,			(float)0					},
	{ TASK_FGRUNT_CHECK_FIRE,	(float)0					},
	{ TASK_RANGE_ATTACK1,		(float)0					},
	{ TASK_FACE_ENEMY,			(float)0					},
	{ TASK_FGRUNT_CHECK_FIRE,	(float)0					},
	{ TASK_RANGE_ATTACK1,		(float)0					},
	{ TASK_FACE_ENEMY,			(float)0					},
	{ TASK_FGRUNT_CHECK_FIRE,	(float)0					},
	{ TASK_RANGE_ATTACK1,		(float)0					},
	{ TASK_FACE_ENEMY,			(float)0					},
	{ TASK_FGRUNT_CHECK_FIRE,	(float)0					},
	{ TASK_RANGE_ATTACK1,		(float)0					},
	{ TASK_FACE_ENEMY,			(float)0					},
	{ TASK_FGRUNT_CHECK_FIRE,	(float)0					},
	{ TASK_RANGE_ATTACK1,		(float)0					},
};

Schedule_t	slFGruntGruntSuppress[] =
{
	{ 
		tlFGruntGruntSuppress,
		ARRAYSIZE ( tlFGruntGruntSuppress ), 
		bits_COND_ENEMY_DEAD		|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_HEAR_SOUND		|
		bits_COND_FGRUNT_NOFIRE		|
		bits_COND_NO_AMMO_LOADED,

		bits_SOUND_DANGER,
		"Suppress"
	},
};


//=========================================================
// FGrunt wait in cover - we don't allow danger or the ability
// to attack to break a grunt's run to cover schedule, but
// when a grunt is in cover, we do want them to attack if they can.
//=========================================================
Task_t	tlFGruntGruntWaitInCover[] =
{
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_SET_ACTIVITY,			(float)ACT_IDLE				},
	{ TASK_WAIT_FACE_ENEMY,			(float)1					},
};

Schedule_t	slFGruntGruntWaitInCover[] =
{
	{ 
		tlFGruntGruntWaitInCover,
		ARRAYSIZE ( tlFGruntGruntWaitInCover ), 
		bits_COND_NEW_ENEMY			|
		bits_COND_HEAR_SOUND		|
		bits_COND_CAN_RANGE_ATTACK1	|
		bits_COND_CAN_RANGE_ATTACK2	|
		bits_COND_CAN_MELEE_ATTACK1	|
		bits_COND_CAN_MELEE_ATTACK2,

		bits_SOUND_DANGER,
		"FGruntWaitInCover"
	},
};

//=========================================================
// run to cover.
// !!!BUGBUG - set a decent fail schedule here.
//=========================================================
Task_t	tlFGruntGruntTakeCover1[] =
{
	{ TASK_STOP_MOVING,				(float)0							},
	{ TASK_SET_FAIL_SCHEDULE,		(float)SCHED_FGRUNT_TAKECOVER_FAILED	},
	{ TASK_WAIT,					(float)0.2							},
	{ TASK_FIND_COVER_FROM_ENEMY,	(float)0							},
	{ TASK_RUN_PATH,				(float)0							},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0							},
	{ TASK_REMEMBER,				(float)bits_MEMORY_INCOVER			},
	{ TASK_SET_SCHEDULE,			(float)SCHED_FGRUNT_WAIT_FACE_ENEMY	},
};

Schedule_t	slFGruntGruntTakeCover[] =
{
	{ 
		tlFGruntGruntTakeCover1,
		ARRAYSIZE ( tlFGruntGruntTakeCover1 ), 
		0,
		0,
		"TakeCover"
	},
};

//=========================================================
// drop grenade then run to cover.
//=========================================================
Task_t	tlFGruntGruntGrenadeCover1[] =
{
	{ TASK_STOP_MOVING,						(float)0							},
	{ TASK_FIND_COVER_FROM_ENEMY,			(float)99							},
	{ TASK_FIND_FAR_NODE_COVER_FROM_ENEMY,	(float)384							},
	{ TASK_PLAY_SEQUENCE,					(float)ACT_SPECIAL_ATTACK1			},
	{ TASK_CLEAR_MOVE_WAIT,					(float)0							},
	{ TASK_RUN_PATH,						(float)0							},
	{ TASK_WAIT_FOR_MOVEMENT,				(float)0							},
	{ TASK_SET_SCHEDULE,					(float)SCHED_FGRUNT_WAIT_FACE_ENEMY	},
};

Schedule_t	slFGruntGruntGrenadeCover[] =
{
	{ 
		tlFGruntGruntGrenadeCover1,
		ARRAYSIZE ( tlFGruntGruntGrenadeCover1 ), 
		0,
		0,
		"GrenadeCover"
	},
};


//=========================================================
// drop grenade then run to cover.
//=========================================================
Task_t	tlFGruntGruntTossGrenadeCover1[] =
{
	{ TASK_FACE_ENEMY,						(float)0							},
	{ TASK_RANGE_ATTACK2, 					(float)0							},
	{ TASK_SET_SCHEDULE,					(float)SCHED_TAKE_COVER_FROM_ENEMY	},
};

Schedule_t	slFGruntGruntTossGrenadeCover[] =
{
	{ 
		tlFGruntGruntTossGrenadeCover1,
		ARRAYSIZE ( tlFGruntGruntTossGrenadeCover1 ), 
		0,
		0,
		"TossGrenadeCover"
	},
};

//=========================================================
// hide from the loudest sound source (to run from grenade)
//=========================================================
Task_t	tlFGruntGruntTakeCoverFromBestSound[] =
{
	{ TASK_SET_FAIL_SCHEDULE,			(float)SCHED_COWER			},// duck and cover if cannot move from explosion
	{ TASK_STOP_MOVING,					(float)0					},
	{ TASK_FIND_COVER_FROM_BEST_SOUND,	(float)0					},
	{ TASK_RUN_PATH,					(float)0					},
	{ TASK_WAIT_FOR_MOVEMENT,			(float)0					},
	{ TASK_REMEMBER,					(float)bits_MEMORY_INCOVER	},
	{ TASK_TURN_LEFT,					(float)179					},
};

Schedule_t	slFGruntGruntTakeCoverFromBestSound[] =
{
	{ 
		tlFGruntGruntTakeCoverFromBestSound,
		ARRAYSIZE ( tlFGruntGruntTakeCoverFromBestSound ), 
		0,
		0,
		"FGruntTakeCoverFromBestSound"
	},
};

//=========================================================
// Grunt reload schedule
//=========================================================
Task_t	tlFGruntGruntHideReload[] =
{
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_SET_FAIL_SCHEDULE,		(float)SCHED_RELOAD			},
	{ TASK_FIND_COVER_FROM_ENEMY,	(float)0					},
	{ TASK_RUN_PATH,				(float)0					},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0					},
	{ TASK_REMEMBER,				(float)bits_MEMORY_INCOVER	},
	{ TASK_FACE_ENEMY,				(float)0					},
	{ TASK_PLAY_SEQUENCE,			(float)ACT_RELOAD			},
};

Schedule_t slFGruntGruntHideReload[] = 
{
	{
		tlFGruntGruntHideReload,
		ARRAYSIZE ( tlFGruntGruntHideReload ),
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_HEAR_SOUND,

		bits_SOUND_DANGER,
		"FGruntHideReload"
	}
};

//=========================================================
// Do a turning sweep of the area
//=========================================================
Task_t	tlFGruntGruntSweep[] =
{
	{ TASK_TURN_LEFT,			(float)179	},
	{ TASK_WAIT,				(float)0.1	},
	{ TASK_TURN_LEFT,			(float)179	},
	{ TASK_WAIT,				(float)0.1	},
};

Schedule_t	slFGruntGruntSweep[] =
{
	{ 
		tlFGruntGruntSweep,
		ARRAYSIZE ( tlFGruntGruntSweep ), 
		
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_CAN_RANGE_ATTACK1	|
		bits_COND_CAN_RANGE_ATTACK2	|
		bits_COND_HEAR_SOUND,

		bits_SOUND_WORLD		|// sound flags
		bits_SOUND_DANGER		|
		bits_SOUND_PLAYER,

		"FGrunt Sweep"
	},
};

//=========================================================
// primary range attack. Overriden because base class stops attacking when the enemy is occluded.
// grunt's grenade toss requires the enemy be occluded.
//=========================================================
Task_t	tlFGruntGruntRangeAttack1A[] =
{
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_FGRUNT_CHECK_FIRE,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_FGRUNT_CHECK_FIRE,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_FGRUNT_CHECK_FIRE,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_FGRUNT_CHECK_FIRE,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
};

Schedule_t	slFGruntGruntRangeAttack1A[] =
{
	{ 
		tlFGruntGruntRangeAttack1A,
		ARRAYSIZE ( tlFGruntGruntRangeAttack1A ), 
		bits_COND_NEW_ENEMY			|
		bits_COND_ENEMY_DEAD		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_ENEMY_OCCLUDED	|
		bits_COND_HEAR_SOUND		|
		bits_COND_FGRUNT_NOFIRE		|
		bits_COND_NO_AMMO_LOADED,
		
		bits_SOUND_DANGER,
		"Range Attack1A"
	},
};


//=========================================================
// primary range attack. Overriden because base class stops attacking when the enemy is occluded.
// grunt's grenade toss requires the enemy be occluded.
//=========================================================
Task_t	tlFGruntGruntRangeAttack1B[] =
{
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_FGRUNT_CHECK_FIRE,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_FGRUNT_CHECK_FIRE,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_FGRUNT_CHECK_FIRE,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_FGRUNT_CHECK_FIRE,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
};

Schedule_t	slFGruntGruntRangeAttack1B[] =
{
	{ 
		tlFGruntGruntRangeAttack1B,
		ARRAYSIZE ( tlFGruntGruntRangeAttack1B ), 
		bits_COND_NEW_ENEMY			|
		bits_COND_ENEMY_DEAD		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_ENEMY_OCCLUDED	|
		bits_COND_NO_AMMO_LOADED	|
		bits_COND_FGRUNT_NOFIRE		|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"Range Attack1B"
	},
};

//=========================================================
// secondary range attack. Overriden because base class stops attacking when the enemy is occluded.
// grunt's grenade toss requires the enemy be occluded.
//=========================================================
Task_t	tlFGruntGruntRangeAttack2[] =
{
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_FGRUNT_FACE_TOSS_DIR,		(float)0					},
	{ TASK_PLAY_SEQUENCE,			(float)ACT_RANGE_ATTACK2	},
	{ TASK_SET_SCHEDULE,			(float)SCHED_FGRUNT_WAIT_FACE_ENEMY	},// don't run immediately after throwing grenade.
};

Schedule_t	slFGruntGruntRangeAttack2[] =
{
	{ 
		tlFGruntGruntRangeAttack2,
		ARRAYSIZE ( tlFGruntGruntRangeAttack2 ), 
		0,
		0,
		"RangeAttack2"
	},
};


//=========================================================
// repel 
//=========================================================
Task_t	tlFGruntGruntRepel[] =
{
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_FACE_IDEAL,			(float)0		},
	{ TASK_PLAY_SEQUENCE,		(float)ACT_GLIDE 	},
};

Schedule_t	slFGruntGruntRepel[] =
{
	{ 
		tlFGruntGruntRepel,
		ARRAYSIZE ( tlFGruntGruntRepel ), 
		bits_COND_SEE_ENEMY			|
		bits_COND_NEW_ENEMY			|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER			|
		bits_SOUND_COMBAT			|
		bits_SOUND_PLAYER, 
		"Repel"
	},
};


//=========================================================
// repel 
//=========================================================
Task_t	tlFGruntGruntRepelAttack[] =
{
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_PLAY_SEQUENCE,		(float)ACT_FLY 	},
};

Schedule_t	slFGruntGruntRepelAttack[] =
{
	{ 
		tlFGruntGruntRepelAttack,
		ARRAYSIZE ( tlFGruntGruntRepelAttack ), 
		bits_COND_ENEMY_OCCLUDED,
		0,
		"Repel Attack"
	},
};

//=========================================================
// repel land
//=========================================================
Task_t	tlFGruntGruntRepelLand[] =
{
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_PLAY_SEQUENCE,		(float)ACT_LAND	},
	{ TASK_GET_PATH_TO_LASTPOSITION,(float)0				},
	{ TASK_RUN_PATH,				(float)0				},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0				},
	{ TASK_CLEAR_LASTPOSITION,		(float)0				},
};

Schedule_t	slFGruntGruntRepelLand[] =
{
	{ 
		tlFGruntGruntRepelLand,
		ARRAYSIZE ( tlFGruntGruntRepelLand ), 
		bits_COND_SEE_ENEMY			|
		bits_COND_NEW_ENEMY			|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER			|
		bits_SOUND_COMBAT			|
		bits_SOUND_PLAYER, 
		"Repel Land"
	},
};

DEFINE_CUSTOM_SCHEDULES( CFGrunt )
{
	slFGruntGruntFollow,
	slFGruntGruntFaceTarget,
	slFGruntGruntIdleStand,
	slFGruntGruntFail,
	slFGruntGruntCombatFail,
	slFGruntGruntVictoryDance,
	slFGruntGruntEstablishLineOfFire,
	slFGruntGruntFoundEnemy,
	slFGruntGruntCombatFace,
	slFGruntGruntSignalSuppress,
	slFGruntGruntSuppress,
	slFGruntGruntWaitInCover,
	slFGruntGruntTakeCover,
	slFGruntGruntGrenadeCover,
	slFGruntGruntTossGrenadeCover,
	slFGruntGruntTakeCoverFromBestSound,
	slFGruntGruntHideReload,
	slFGruntGruntSweep,
	slFGruntGruntRangeAttack1A,
	slFGruntGruntRangeAttack1B,
	slFGruntGruntRangeAttack2,
	slFGruntGruntRepel,
	slFGruntGruntRepelAttack,
	slFGruntGruntRepelLand
};


IMPLEMENT_CUSTOM_SCHEDULES( CFGrunt, CRCAllyMonster );

void CFGrunt :: StartTask( Task_t *pTask )
{
	m_iTaskStatus = TASKSTATUS_RUNNING;

	switch ( pTask->iTask )
	{

	case TASK_FGRUNT_CHECK_FIRE:
		if ( !NoFriendlyFire() )
		{
			SetConditions( bits_COND_FGRUNT_NOFIRE );
		}
		TaskComplete();
		break;

	case TASK_FGRUNT_STOP_FOLLOWING:
		if(!m_hTargetEnt->IsPlayer())
			StopFollowing(TRUE);

		TaskComplete();
		break;

	case TASK_WALK_PATH:
	case TASK_RUN_PATH:
		// grunt no longer assumes he is covered if he moves
		Forget( bits_MEMORY_INCOVER );
		CRCAllyMonster ::StartTask( pTask );
		break;

	case TASK_RELOAD:
		m_IdealActivity = ACT_RELOAD;
		break;

	case TASK_FGRUNT_FACE_TOSS_DIR:
		break;

	default: 
		CRCAllyMonster :: StartTask( pTask );
		break;
	}
}

void CFGrunt :: RunTask( Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_FGRUNT_FACE_TOSS_DIR:
		{
			// project a point along the toss vector and turn to face that point.
			MakeIdealYaw( pev->origin + m_vecTossVelocity * 64 );
			ChangeYaw( pev->yaw_speed );

			if ( FacingIdeal() )
			{
				m_iTaskStatus = TASKSTATUS_COMPLETE;
			}
			break;
		}
	default:
		{
			CRCAllyMonster :: RunTask( pTask );
			break;
		}
	}
}
//=========================================================
// GibMonster - make gun fly through the air.
//=========================================================
void CFGrunt :: GibMonster ( void )
{
	Vector	vecGunPos;
	Vector	vecGunAngles;

	if ( GetBodygroup( 3 ) != 3 )
	{// throw a gun if the grunt has one
		GetAttachment( 0, vecGunPos, vecGunAngles );
		
		CBaseEntity *pGun;
		if (FBitSet( pev->weapons, FGRUNT_SHOTGUN ))
		{
			pGun = DropItem( "weapon_shotgun", vecGunPos, pev->angles );
		}
		else if (FBitSet( pev->weapons, FGRUNT_9MMAR ))
		{
			pGun = DropItem( "weapon_9mmar", vecGunPos, pev->angles );
		}
		else if (FBitSet( pev->weapons, FGRUNT_M249 ))
		{
			pGun = DropItem( "weapon_m249", vecGunPos, pev->angles );
		}

		if ( pGun )
		{
			pGun->pev->velocity = Vector (RANDOM_FLOAT(-100,100), RANDOM_FLOAT(-100,100), RANDOM_FLOAT(200,300));
			pGun->pev->avelocity = Vector ( 0, RANDOM_FLOAT( 200, 400 ), 0 );
		}
	
		if (FBitSet( pev->weapons, FGRUNT_GRENADELAUNCHER ))
		{
			pGun = DropItem( "ammo_ARgrenades", vecGunPos, pev->angles );
			if ( pGun )
			{
				pGun->pev->velocity = Vector (RANDOM_FLOAT(-100,100), RANDOM_FLOAT(-100,100), RANDOM_FLOAT(200,300));
				pGun->pev->avelocity = Vector ( 0, RANDOM_FLOAT( 200, 400 ), 0 );
			}
		}
	}

	CBaseMonster :: GibMonster();
}
//=========================================================
// ISoundMask - returns a bit mask indicating which types
// of sounds this monster regards. 
//=========================================================
int CFGrunt :: ISoundMask ( void) 
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
// CheckAmmo - overridden for the grunt because he actually
// uses ammo! (base class doesn't)
//=========================================================
void CFGrunt :: CheckAmmo ( void )
{
	if ( m_cAmmoLoaded <= 0 )
	{
		SetConditions(bits_COND_NO_AMMO_LOADED);
	}
}
//=========================================================
// Classify - indicates this monster's place in the 
// relationship table.
//=========================================================
int	CFGrunt :: Classify ( void )
{
	return	CLASS_PLAYER_ALLY;
}
//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CFGrunt :: SetYawSpeed ( void )
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
// PrescheduleThink - this function runs after conditions
// are collected and before scheduling code is run.
//=========================================================
void CFGrunt :: PrescheduleThink ( void )
{
	if ( InSquad() && m_hEnemy != NULL )
	{
		if ( HasConditions ( bits_COND_SEE_ENEMY ) )
		{
			// update the squad's last enemy sighting time.
			((CRCAllyMonster *)MySquadLeader())->m_flLastEnemySightTime = gpGlobals->time;
		}
		else
		{
			if ( gpGlobals->time - ((CRCAllyMonster *)MySquadLeader())->m_flLastEnemySightTime > 5 )
			{
				// been a while since we've seen the enemy
				((CRCAllyMonster *)MySquadLeader())->m_fEnemyEluded = TRUE;
			}
		}
	}
	CRCAllyMonster :: PrescheduleThink();
}

//=========================================================
// FCanCheckAttacks - this is overridden for human grunts
// because they can throw/shoot grenades when they can't see their
// target and the base class doesn't check attacks if the monster
// cannot see its enemy.
//
// !!!BUGBUG - this gets called before a 3-round burst is fired
// which means that a friendly can still be hit with up to 2 rounds. 
// ALSO, grenades will not be tossed if there is a friendly in front,
// this is a bad bug. Friendly machine gun fire avoidance
// will unecessarily prevent the throwing of a grenade as well.
//=========================================================
BOOL CFGrunt :: FCanCheckAttacks ( void )
{
	if ( !HasConditions( bits_COND_ENEMY_TOOFAR ) )
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


//=========================================================
// CheckMeleeAttack1
//=========================================================
BOOL CFGrunt :: CheckMeleeAttack1 ( float flDot, float flDist )
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
// CheckRangeAttack1 - overridden for HGrunt, cause 
// FCanCheckAttacks() doesn't disqualify all attacks based
// on whether or not the enemy is occluded because unlike
// the base class, the HGrunt can attack when the enemy is
// occluded (throw grenade over wall, etc). We must 
// disqualify the machine gun attack if the enemy is occluded.
//=========================================================
BOOL CFGrunt :: CheckRangeAttack1 ( float flDot, float flDist )
{
	if(!FBitSet(pev->weapons, FGRUNT_9MMAR ) && !FBitSet(pev->weapons, FGRUNT_SHOTGUN) && !FBitSet(pev->weapons, FGRUNT_M249))
		return FALSE;

	if ( !HasConditions( bits_COND_ENEMY_OCCLUDED ) && flDist <= 2048 && flDot >= 0.5 )
	{
		if ( !m_hEnemy->IsPlayer() && flDist <= 64 )
		{
			// kick nonclients, but don't shoot at them.
			return FALSE;
		}

		// See if we should stand or crouch
		TraceResult	tr;
		Vector vecCrouch = pev->origin + Vector( 0, 0, 48 );
		Vector vecStanding = pev->origin + Vector( 0, 0, 60 );

		// verify that a bullet fired from the gun will hit the enemy before the world.
		UTIL_TraceLine( vecCrouch, m_hEnemy->BodyTarget(vecCrouch), ignore_monsters, ignore_glass, ENT(pev), &tr);

		// We can crouch
		if ( tr.flFraction == 1.0 )
		{
			m_fStanding = FALSE;

			if(NoFriendlyFire())
				return TRUE;
		}

		// verify that a bullet fired from the gun will hit the enemy before the world.
		UTIL_TraceLine( vecStanding, m_hEnemy->BodyTarget(vecStanding), ignore_monsters, ignore_glass, ENT(pev), &tr);

		// Standing
		if ( tr.flFraction == 1.0 )
		{
			m_fStanding = TRUE;

			if(NoFriendlyFire())
				return TRUE;
		}
	}

	return FALSE;
}

//=========================================================
// CheckRangeAttack2 - this checks the Grunt's grenade
// attack. 
//=========================================================
BOOL CFGrunt :: CheckRangeAttack2 ( float flDot, float flDist )
{
	if(!pev->weapons)
		return false;

	if (!FBitSet(pev->weapons, (FGRUNT_HANDGRENADE | FGRUNT_GRENADELAUNCHER)) )
	{
		return FALSE;
	}
	
	// if the grunt isn't moving, it's ok to check.
	if ( m_flGroundSpeed != 0 )
	{
		m_fThrowGrenade = FALSE;
		return m_fThrowGrenade;
	}

	// assume things haven't changed too much since last time
	if (gpGlobals->time < m_flNextGrenadeCheck )
	{
		return m_fThrowGrenade;
	}

	if ( !FBitSet ( m_hEnemy->pev->flags, FL_ONGROUND ) && m_hEnemy->pev->waterlevel == 0 && m_vecEnemyLKP.z > pev->absmax.z  )
	{
		//!!!BUGBUG - we should make this check movetype and make sure it isn't FLY? Players who jump a lot are unlikely to 
		// be grenaded.
		// don't throw grenades at anything that isn't on the ground!
		m_fThrowGrenade = FALSE;
		return m_fThrowGrenade;
	}
	
	Vector vecTarget;

	if (FBitSet( pev->weapons, FGRUNT_HANDGRENADE))
	{
		// find feet
		if (RANDOM_LONG(0,1))
		{
			// magically know where they are
			vecTarget = Vector( m_hEnemy->pev->origin.x, m_hEnemy->pev->origin.y, m_hEnemy->pev->absmin.z );
		}
		else
		{
			// toss it to where you last saw them
			vecTarget = m_vecEnemyLKP;
		}
		// vecTarget = m_vecEnemyLKP + (m_hEnemy->BodyTarget( pev->origin ) - m_hEnemy->pev->origin);
		// estimate position
		// vecTarget = vecTarget + m_hEnemy->pev->velocity * 2;
	}
	else
	{
		// find target
		// vecTarget = m_hEnemy->BodyTarget( pev->origin );
		vecTarget = m_vecEnemyLKP + (m_hEnemy->BodyTarget( pev->origin ) - m_hEnemy->pev->origin);
		// estimate position
		if (HasConditions( bits_COND_SEE_ENEMY))
			vecTarget = vecTarget + ((vecTarget - pev->origin).Length() / gSkillData.fgruntGrenadeSpeed) * m_hEnemy->pev->velocity;
	}

	// are any of my squad members near the intended grenade impact area?
	if ( InSquad() )
	{
		if (SquadMemberInRange( vecTarget, 256 ))
		{
			// crap, I might blow my own guy up. Don't throw a grenade and don't check again for a while.
			m_flNextGrenadeCheck = gpGlobals->time + 1; // one full second.
			m_fThrowGrenade = FALSE;
			return m_fThrowGrenade;	//AJH need this or it is overridden later.
		}
	}
	
	if ( ( vecTarget - pev->origin ).Length2D() <= 256 )
	{
		// crap, I don't want to blow myself up
		m_flNextGrenadeCheck = gpGlobals->time + 1; // one full second.
		m_fThrowGrenade = FALSE;
		return m_fThrowGrenade;
	}

		
	if (FBitSet( pev->weapons, FGRUNT_HANDGRENADE))
	{
		Vector vecToss = VecCheckToss( pev, GetGunPosition(), vecTarget, 0.5 );

		if ( vecToss != g_vecZero )
		{
			m_vecTossVelocity = vecToss;

			// throw a hand grenade
			m_fThrowGrenade = TRUE;
			// don't check again for a while.
			m_flNextGrenadeCheck = gpGlobals->time; // 1/3 second.
		}
		else
		{
			// don't throw
			m_fThrowGrenade = FALSE;
			// don't check again for a while.
			m_flNextGrenadeCheck = gpGlobals->time + 1; // one full second.
		}
	}
	else
	{
		Vector vecToss = VecCheckThrow( pev, GetGunPosition(), vecTarget, gSkillData.fgruntGrenadeSpeed, 0.5 );

		if ( vecToss != g_vecZero )
		{
			m_vecTossVelocity = vecToss;

			// throw a hand grenade
			m_fThrowGrenade = TRUE;
			// don't check again for a while.
			m_flNextGrenadeCheck = gpGlobals->time + 0.3; // 1/3 second.
		}
		else
		{
			// don't throw
			m_fThrowGrenade = FALSE;
			// don't check again for a while.
			m_flNextGrenadeCheck = gpGlobals->time + 1; // one full second.
		}
	}

	return m_fThrowGrenade;
}
//=========================================================
//=========================================================
CBaseEntity *CFGrunt :: Kick( void )
{
	TraceResult tr;

	UTIL_MakeVectors( pev->angles );
	Vector vecStart = pev->origin;
	vecStart.z += pev->size.z * 0.5;
	Vector vecEnd = vecStart + (gpGlobals->v_forward * 70);

	UTIL_TraceHull( vecStart, vecEnd, dont_ignore_monsters, head_hull, ENT(pev), &tr );
	
	if ( tr.pHit )
	{
		CBaseEntity *pEntity = CBaseEntity::Instance( tr.pHit );
		return pEntity;
	}

	return NULL;
}

//=========================================================
// GetGunPosition	return the end of the barrel
//=========================================================
Vector CFGrunt :: GetGunPosition( void )
{
	if ( m_fStanding )
	{
		return pev->origin + Vector( 0, 0, 60 );
	}
	else
	{
		return pev->origin + Vector( 0, 0, 48 );
	}
}

//=========================================================
// Shoot
//=========================================================
void CFGrunt :: Shoot ( void )
{
	if (m_hEnemy == NULL )
	{
		return;
	}

	Vector vecShootOrigin = GetGunPosition();
	Vector vecShootDir = ShootAtEnemy( vecShootOrigin );

	UTIL_MakeVectors ( pev->angles );

	switch(RANDOM_LONG(0, 2))
	{
		case 0: EMIT_SOUND( ENT(pev), CHAN_WEAPON, "weapons/hks1.wav", 1, 0.5 ); break;
		case 1: EMIT_SOUND( ENT(pev), CHAN_WEAPON, "weapons/hks2.wav", 1, 0.5 ); break;
		case 2: EMIT_SOUND( ENT(pev), CHAN_WEAPON, "weapons/hks3.wav", 1, 0.5 ); break;
	}

	Vector vecShellOrigin = vecShootOrigin + gpGlobals->v_forward * 16;
	Vector vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT(40,90) + gpGlobals->v_up * RANDOM_FLOAT(75,200) + gpGlobals->v_forward * RANDOM_FLOAT(-40, 40);
	EjectBrass ( vecShootOrigin - vecShootDir * 24, vecShellVelocity, pev->angles.y, m_iBrassShell, TE_BOUNCE_SHELL); 
	FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_4DEGREES, 2048, BULLET_MONSTER_MP5 ); // shoot +-5 degrees

	pev->effects |= EF_MUZZLEFLASH;

	m_cAmmoLoaded--;// take away a bullet!

	Vector angDir = UTIL_VecToAngles( vecShootDir );
	SetBlending( 0, angDir.x );
}

//=========================================================
// Shoot
//=========================================================
void CFGrunt :: Shotgun ( void )
{
	if (m_hEnemy == NULL)
	{
		return;
	}

	Vector vecShootOrigin = GetGunPosition();
	Vector vecShootDir = ShootAtEnemy( vecShootOrigin );

	UTIL_MakeVectors ( pev->angles );

	Vector vecShellOrigin = vecShootOrigin + gpGlobals->v_forward * 16;
	Vector vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT(40,90) + gpGlobals->v_up * RANDOM_FLOAT(75,200) + gpGlobals->v_forward * RANDOM_FLOAT(-40, 40);
	EjectBrass ( vecShootOrigin - vecShootDir * 24, vecShellVelocity, pev->angles.y, m_iShotgunShell, TE_BOUNCE_SHOTSHELL);
	FireBullets(gSkillData.fgruntShotgunPellets, vecShootOrigin, vecShootDir, VECTOR_CONE_9DEGREES, 2048, BULLET_MONSTER_12MM, 0 ); // shoot +-7.5 degrees

	pev->effects |= EF_MUZZLEFLASH;

	m_cAmmoLoaded--;// take away a bullet!

	Vector angDir = UTIL_VecToAngles( vecShootDir );
	SetBlending( 0, angDir.x );
}
//=========================================================
// Shoot
//=========================================================
void CFGrunt :: M249 ( void )
{
	if (m_hEnemy == NULL )
	{
		return;
	}

	switch(RANDOM_LONG(0, 2))
	{
		case 0: EMIT_SOUND( ENT(pev), CHAN_WEAPON, "weapons/saw_shoot1.wav", 1, ATTN_NORM ); break;
		case 1: EMIT_SOUND( ENT(pev), CHAN_WEAPON, "weapons/saw_shoot2.wav", 1, ATTN_NORM ); break;
		case 2: EMIT_SOUND( ENT(pev), CHAN_WEAPON, "weapons/saw_shoot3.wav", 1, ATTN_NORM ); break;
	}

	Vector vecShootOrigin = GetGunPosition();
	Vector vecShootDir = ShootAtEnemy( vecShootOrigin );

	UTIL_MakeVectors ( pev->angles );

	Vector vecShellOrigin = vecShootOrigin + gpGlobals->v_forward * 16;
	Vector vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT(40,90) + gpGlobals->v_up * RANDOM_FLOAT(75,200) + gpGlobals->v_forward * RANDOM_FLOAT(-40, 40);
			
	static bool linkToggle = false;
	linkToggle = !linkToggle;
	EjectBrass ( vecShootOrigin - vecShootDir * 24, vecShellVelocity, pev->angles.y, linkToggle ? m_iSawShell : m_iSawLink, TE_BOUNCE_SHELL); 
	FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_3DEGREES, 2048, BULLET_MONSTER_556 ); // shoot +-5 degrees

	pev->effects |= EF_MUZZLEFLASH;
	m_cAmmoLoaded--;// take away a bullet!

	Vector angDir = UTIL_VecToAngles( vecShootDir );
	SetBlending( 0, angDir.x );
}
//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CFGrunt :: HandleAnimEvent( MonsterEvent_t *pEvent )
{
	Vector	vecShootDir;
	Vector	vecShootOrigin;

	switch( pEvent->event )
	{
		case FGRUNT_AE_DROP_GUN:
			{
			Vector	vecGunPos;
			Vector	vecGunAngles;

			GetAttachment( 0, vecGunPos, vecGunAngles );

			// switch to body group with no gun.
			SetBodygroup( MN_GUN_GROUP, MN_GUN_NONE );

			// now spawn a gun.
			if (FBitSet( pev->weapons, FGRUNT_SHOTGUN ))
			{
				 DropItem( "weapon_shotgun", vecGunPos, pev->angles );
			}
			else if (FBitSet( pev->weapons, FGRUNT_9MMAR ))
			{
				 DropItem( "weapon_sig552", vecGunPos, pev->angles );
			}
			else
			{
				 DropItem( "weapon_m249", vecGunPos, pev->angles );
			}
			if (FBitSet( pev->weapons, FGRUNT_GRENADELAUNCHER ))
			{
				DropItem( "ammo_ARgrenades", BodyTarget( pev->origin ), pev->angles );
			}

			}
			break;

		case FGRUNT_AE_RELOAD:
			m_cAmmoLoaded = m_cClipSize;
			ClearConditions(bits_COND_NO_AMMO_LOADED);
			break;

		case FGRUNT_AE_GREN_TOSS:
		{
			UTIL_MakeVectors( pev->angles );
			// CGrenade::ShootTimed( pev, pev->origin + gpGlobals->v_forward * 34 + Vector (0, 0, 32), m_vecTossVelocity, 3.5 );
			CGrenade::ShootTimed( pev, GetGunPosition(), m_vecTossVelocity, 3.5 );

			m_fThrowGrenade = FALSE;
			m_flNextGrenadeCheck = gpGlobals->time + 6;// wait six seconds before even looking again to see if a grenade can be thrown.
			// !!!LATER - when in a group, only try to throw grenade if ordered.
		}
		break;

		case FGRUNT_AE_GREN_LAUNCH:
		{
			EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/glauncher.wav", 0.8, 0.5);
			CGrenade::ShootContact( pev, GetGunPosition(), m_vecTossVelocity );
			m_fThrowGrenade = FALSE;
			if (g_iSkillLevel == SKILL_EASY)
				m_flNextGrenadeCheck = gpGlobals->time + RANDOM_FLOAT( 2, 5 );// wait a random amount of time before shooting again
			else
				m_flNextGrenadeCheck = gpGlobals->time + 6;// wait six seconds before even looking again to see if a grenade can be thrown.
		}
		break;

		case FGRUNT_AE_GREN_DROP:
		{
			UTIL_MakeVectors( pev->angles );
			CGrenade::ShootTimed( pev, pev->origin + gpGlobals->v_forward * 17 - gpGlobals->v_right * 27 + gpGlobals->v_up * 6, g_vecZero, 3 );
		}
		break;

		case FGRUNT_AE_BURST1:
		{
			if ( FBitSet( pev->weapons, FGRUNT_9MMAR ))
			{
				Shoot();
			}
			else if ( FBitSet( pev->weapons, FGRUNT_SHOTGUN ))
			{
				Shotgun( );

				EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/sbarrel1.wav", 1, 0.5 );
			}
			else
			{
				M249( );
			}
		
			CSoundEnt::InsertSound ( bits_SOUND_COMBAT, pev->origin, 384, 0.3 );
		}
		break;

		case FGRUNT_AE_BURST2:
		case FGRUNT_AE_BURST3:
			if ( FBitSet( pev->weapons, FGRUNT_9MMAR ))
				Shoot();
			else
				M249();
			break;

		case FGRUNT_AE_KICK:
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
				}
				pHurt->TakeDamage( pev, pev, gSkillData.fgruntDmgKick, DMG_CLUB );
			}
		}
		break;

		case FGRUNT_AE_CAUGHT_ENEMY:
		{
			if ( FOkToSpeak() )
			{
				SENTENCEG_PlayRndSz(ENT(pev), "FG_ALERT", VOL_NORM, ATTN_NORM, 0, m_voicePitch);
				JustSpoke();
			}

		}

		default:
			CRCAllyMonster::HandleAnimEvent( pEvent );
			break;
	}
}

//=========================================================
// Spawn
//=========================================================
void CFGrunt :: Spawn()
{
	Precache( );

	SET_MODEL(ENT(pev), "models/hgrunt_opfor.mdl");
	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_RED;
	pev->health			= gSkillData.fgruntHealth;
	pev->view_ofs		= Vector ( 0, 0, 50 );// position of the eyes relative to monster's origin.
	m_flFieldOfView		= VIEW_FIELD_WIDE; // NOTE: we need a wide field of view so npc will notice player and say hello
	m_MonsterState		= MONSTERSTATE_NONE;
	m_flNextGrenadeCheck = gpGlobals->time + 1;
	m_flNextPainTime	= gpGlobals->time;

	m_afCapability		= bits_CAP_HEAR | bits_CAP_SQUAD | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP;

	m_fEnemyEluded		= FALSE;
	m_fFirstEncounter	= TRUE;// this is true when the grunt spawns, because he hasn't encountered an enemy yet.

	m_HackedGunPos = Vector ( 0, 0, 55 );
	m_iHead = pev->body;

	if ( m_iHead == -1 )
		m_iHead = 0;

	if (FBitSet( pev->weapons, FGRUNT_SHOTGUN ))
	{
		SetBodygroup( MN_GUN_GROUP, MN_GUN_SHOTGUN );
		SetBodygroup( MN_TORSO_GROUP, MN_TORSO_SHOTGUN );
		m_cClipSize		= 8;
	}
	if (FBitSet( pev->weapons, FGRUNT_9MMAR ))
	{
		SetBodygroup( MN_GUN_GROUP, MN_GUN_MP5 );
		SetBodygroup( MN_TORSO_GROUP, MN_TORSO_NORMAL );
		m_cClipSize	= FGRUNT_CLIP_SIZE;
	}
	if (FBitSet( pev->weapons, FGRUNT_M249 ))
	{
		SetBodygroup( MN_GUN_GROUP, MN_GUN_SAW );
		SetBodygroup( MN_TORSO_GROUP, MN_TORSO_SAW );
		m_cClipSize	= FGRUNT_CLIP_SIZE;
	}
	if (pev->weapons == 0)
	{
		SetBodygroup( MN_GUN_GROUP, MN_GUN_NONE );
		SetBodygroup( MN_TORSO_GROUP, MN_TORSO_NORMAL );
		m_cClipSize	= FGRUNT_CLIP_SIZE;
	}

	if ( m_iHead == 0 )
	{
		SetBodygroup( MN_HEAD_GROUP, MN_HEAD_MASK );
	}
	if ( m_iHead == 1 )
	{
		SetBodygroup( MN_HEAD_GROUP, MN_HEAD_CMDR_BLACK );
	}
	if ( m_iHead == 2 )
	{
		SetBodygroup( MN_HEAD_GROUP, MN_HEAD_SHOTGUN );
	}
	if ( m_iHead == 3 )
	{
		SetBodygroup( MN_HEAD_GROUP, MN_HEAD_SAW_WHT );
	}
	if ( m_iHead == 4 )
	{
		SetBodygroup( MN_HEAD_GROUP, MN_HEAD_SAW_BLK );
	}
	if ( m_iHead == 5 )
	{
		SetBodygroup( MN_HEAD_GROUP, MN_HEAD_MP );
	}
	if ( m_iHead == 6 )
	{
		SetBodygroup( MN_HEAD_GROUP, MN_HEAD_CMDR_OLD );
	}
	if ( m_iHead == 7 )
	{
		SetBodygroup( MN_HEAD_GROUP, MN_HEAD_CMDR_BLACK );
	}

	m_cAmmoLoaded		= m_cClipSize;

	MonsterInit();
	SetUse( &CRCAllyMonster::FollowerUse );
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CFGrunt :: Precache()
{
	PRECACHE_MODEL("models/hgrunt_opfor.mdl");

	PRECACHE_SOUND("fgrunt/fg_pain1.wav");
	PRECACHE_SOUND("fgrunt/fg_pain2.wav");
	PRECACHE_SOUND("fgrunt/fg_pain3.wav");
	PRECACHE_SOUND("fgrunt/fg_pain4.wav");
	PRECACHE_SOUND("fgrunt/fg_pain5.wav");
	PRECACHE_SOUND("fgrunt/fg_pain6.wav");

	PRECACHE_SOUND("fgrunt/fg_death1.wav");
	PRECACHE_SOUND("fgrunt/fg_death2.wav");
	PRECACHE_SOUND("fgrunt/fg_death3.wav");
	PRECACHE_SOUND("fgrunt/fg_death4.wav");
	PRECACHE_SOUND("fgrunt/fg_death5.wav");
	PRECACHE_SOUND("fgrunt/fg_death6.wav");

	PRECACHE_SOUND("weapons/sbarrel1.wav");

	PRECACHE_SOUND("weapons/hks1.wav");
	PRECACHE_SOUND("weapons/hks2.wav");
	PRECACHE_SOUND("weapons/hks3.wav");

	PRECACHE_SOUND ("weapons/saw_shoot1.wav");
	PRECACHE_SOUND ("weapons/saw_shoot2.wav");
	PRECACHE_SOUND ("weapons/saw_shoot3.wav");

	PRECACHE_SOUND("common/hit_miss.wav");// because we use the basemonster SWIPE animation event

	m_iBrassShell = PRECACHE_MODEL ("models/shell.mdl");// brass shell
	m_iShotgunShell = PRECACHE_MODEL ("models/shotgunshell.mdl");

	m_iSawShell = PRECACHE_MODEL ("models/saw_shell.mdl");// brass shell
	m_iSawLink = PRECACHE_MODEL ("models/saw_link.mdl");

	TalkInit();
	CRCAllyMonster::Precache();
}	

// Init talk data
void CFGrunt :: TalkInit()
{
	// scientists speach group names (group names are in sentences.txt)

	m_szGrp[TLK_ANSWER]  =	"FG_ANSWER";
	m_szGrp[TLK_QUESTION] =	"FG_QUESTION";
	m_szGrp[TLK_IDLE] =		"FG_IDLE";
	m_szGrp[TLK_STARE] =	"FG_STARE";
	m_szGrp[TLK_USE] =		"FG_OK";
	m_szGrp[TLK_UNUSE] =	"FG_WAIT";
	m_szGrp[TLK_STOP] =		"FG_STOP";

	m_szGrp[TLK_NOSHOOT] =	"FG_SCARED";
	m_szGrp[TLK_HELLO] =	"FG_HELLO";

	m_szGrp[TLK_PLHURT1] =	"FG_CURE";
	m_szGrp[TLK_PLHURT2] =	"FG_CURE"; 
	m_szGrp[TLK_PLHURT3] =	"FG_CURE";

	m_szGrp[TLK_SMELL] =	"FG_SMELL";
	
	m_szGrp[TLK_WOUND] =	"FG_WOUND";
	m_szGrp[TLK_MORTAL] =	"FG_MORTAL";

	CRCAllyMonster::TalkInit();

	if ( m_iHead == 0 )m_voicePitch = 100;
	if ( m_iHead == 1 )m_voicePitch = 100;
	if ( m_iHead == 2 )m_voicePitch = 90;
	if ( m_iHead == 3 )m_voicePitch = 100;
	if ( m_iHead == 4 )m_voicePitch = 90;
	if ( m_iHead == 5 )m_voicePitch = 100;
	if ( m_iHead == 6 )m_voicePitch = 100;
	if ( m_iHead == 7 )m_voicePitch = 90;
}
	
//=========================================================
// PainSound
//=========================================================
void CFGrunt :: PainSound ( void )
{
	if ( gpGlobals->time > m_flNextPainTime )
	{
		switch ( RANDOM_LONG(0,5) )
		{
			case 0: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "fgrunt/fg_pain1.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
			case 1: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "fgrunt/fg_pain2.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
			case 2: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "fgrunt/fg_pain3.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
			case 3: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "fgrunt/fg_pain4.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
			case 4: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "fgrunt/fg_pain5.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
			case 5: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "fgrunt/fg_pain6.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
		}
		m_flNextPainTime = gpGlobals->time + 1;
	}
}

//=========================================================
// DeathSound 
//=========================================================
void CFGrunt :: DeathSound ( void )
{
	switch (RANDOM_LONG(0,5))
	{
	case 0: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "fgrunt/fg_death1.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
	case 1: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "fgrunt/fg_death2.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
	case 2: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "fgrunt/fg_death3.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
	case 3: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "fgrunt/fg_death4.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
	case 4: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "fgrunt/fg_death5.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
	case 5: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "fgrunt/fg_death6.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
	}
}
//=========================================================
// TraceAttack - make sure we're not taking it in the helmet
//=========================================================
void CFGrunt :: TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType)
{
	if(pevAttacker)
	{
		if(IRelationship(CBaseEntity::Instance(pevAttacker)) == R_AL || IRelationship(CBaseEntity::Instance(pevAttacker)) == R_NO)
		{
			if(ENT(pevAttacker) != this->edict())
			{
				flDamage = 0;
				return;
			}
		}
	}

	// check for helmet shot
	if (ptr->iHitgroup == 11)
	{
		// make sure we're wearing one
		if (bitsDamageType & (DMG_BULLET | DMG_SLASH | DMG_BLAST | DMG_CLUB))
		{
			UTIL_Ricochet( ptr->vecEndPos, 1 );
			flDamage = 0;
		}

		// it's head shot anyways
		ptr->iHitgroup = HITGROUP_HEAD;
	}
	CRCAllyMonster::TraceAttack( pevAttacker, flDamage, vecDir, ptr, bitsDamageType );
}
//=========================================================
// TakeDamage - overridden for the grunt because the grunt
// needs to forget that he is in cover if he's hurt. (Obviously
// not in a safe place anymore).
//=========================================================
int CFGrunt :: TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
	if(pevAttacker)
	{
		if(IRelationship(CBaseEntity::Instance(pevAttacker)) == R_AL || IRelationship(CBaseEntity::Instance(pevAttacker)) == R_NO)
		{
			if(ENT(pevAttacker) != this->edict())
			{
				return 0;
			}
		}
	}

	Forget( bits_MEMORY_INCOVER );

	// make sure friends talk about it if player hurts talkmonsters...
	int ret = CRCAllyMonster::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
	if ( !IsAlive() || pev->deadflag == DEAD_DYING )
		return ret;

	if ( m_MonsterState != MONSTERSTATE_PRONE && (pevAttacker->flags & FL_CLIENT) )
	{
		m_flPlayerDamage += flDamage;

		// This is a heurstic to determine if the player intended to harm me
		// If I have an enemy, we can't establish intent (may just be crossfire)
		if ( m_hEnemy == NULL )
		{
			// If the player was facing directly at me, or I'm already suspicious, get mad
			if ( (m_afMemory & bits_MEMORY_SUSPICIOUS) )
			{
				// Alright, now I'm pissed!
				PlaySentence( "FG_MAD", 4, VOL_NORM, ATTN_NORM );

				Remember( bits_MEMORY_PROVOKED );
				StopFollowing( TRUE );
			}
			else
			{
				// Hey, be careful with that
				PlaySentence( "FG_SHOT", 4, VOL_NORM, ATTN_NORM );
				Remember( bits_MEMORY_SUSPICIOUS );
			}
		}
		else if ( !(m_hEnemy->IsPlayer()) && pev->deadflag == DEAD_NO )
		{
			PlaySentence( "FG_SHOT", 4, VOL_NORM, ATTN_NORM );
		}
	}

	return CRCAllyMonster :: TakeDamage ( pevInflictor, pevAttacker, flDamage, bitsDamageType );
}

//=========================================================
// AI Schedules Specific to this monster
//=========================================================

Schedule_t* CFGrunt :: GetScheduleOfType ( int Type )
{
	Schedule_t *psched;

	switch( Type )
	{
	// Hook these to make a looping schedule
	case SCHED_TARGET_FACE:
		{
			// call base class default so that barney will talk
			// when 'used' 
			psched = CRCAllyMonster::GetScheduleOfType(Type);

			if (psched == slIdleStand)
				return slFGruntGruntFaceTarget;	// override this for different target face behavior
			else
				return psched;
		}
		break;
	case SCHED_TARGET_CHASE:
		{
			return slFGruntGruntFollow;
		}
		break;
	case SCHED_IDLE_STAND:
		{
			psched = CRCAllyMonster::GetScheduleOfType(Type);

			if (psched == slIdleStand)
			{
				// just look straight ahead.
				return slFGruntGruntIdleStand;
			}
			else
				return psched;	
		}
		break;
	case SCHED_TAKE_COVER_FROM_ENEMY:
		{
			return &slFGruntGruntTakeCover[ 0 ];
		}
		break;
	case SCHED_TAKE_COVER_FROM_BEST_SOUND:
		{
			return &slFGruntGruntTakeCoverFromBestSound[ 0 ];
		}
		break;
	case SCHED_FGRUNT_TAKECOVER_FAILED:
		{
			if ( HasConditions( bits_COND_CAN_RANGE_ATTACK1 ) && OccupySlot( bits_SLOTS_FGRUNT_ENGAGE ) )
			{
				return GetScheduleOfType( SCHED_RANGE_ATTACK1 );
			}
			else
			{
				return GetScheduleOfType ( SCHED_FAIL );
			}
		}
		break;
	case SCHED_FGRUNT_ELOF_FAIL:
		{
			if(HasConditions( bits_COND_CAN_RANGE_ATTACK1 ))
			{
				return GetScheduleOfType( SCHED_RANGE_ATTACK1 );
			}
			else
			{
				return GetScheduleOfType ( SCHED_FAIL );
			}
		}
		break;
	case SCHED_FGRUNT_ESTABLISH_LINE_OF_FIRE:
		{
			return &slFGruntGruntEstablishLineOfFire[ 0 ];
		}
		break;
	case SCHED_RANGE_ATTACK1:
		{
			// randomly stand or crouch
			if (m_fStanding)
				return &slFGruntGruntRangeAttack1B[ 0 ];
			else
				return &slFGruntGruntRangeAttack1A[ 0 ];
		}
		break;
	case SCHED_RANGE_ATTACK2:
		{
			return &slFGruntGruntRangeAttack2[ 0 ];
		}
		break;
	case SCHED_COMBAT_FACE:
		{
			return &slFGruntGruntCombatFace[ 0 ];
		}
		break;
	case SCHED_FGRUNT_WAIT_FACE_ENEMY:
		{
			return &slFGruntGruntWaitInCover[ 0 ];
		}
	case SCHED_FGRUNT_SWEEP:
		{
			return &slFGruntGruntSweep[ 0 ];
		}
		break;
	case SCHED_FGRUNT_COVER_AND_RELOAD:
		{
			return &slFGruntGruntHideReload[ 0 ];
		}
		break;
	case SCHED_FGRUNT_FOUND_ENEMY:
		{
			return &slFGruntGruntFoundEnemy[ 0 ];
		}
		break;
	case SCHED_VICTORY_DANCE:
		{
			if ( InSquad() )
			{
				if ( !IsLeader() )
				{
					return &slFGruntGruntFail[ 0 ];
				}
			}
			if ( IsFollowing() )
			{
				return &slFGruntGruntFail[ 0 ];
			}

			return &slFGruntGruntVictoryDance[ 0 ];
		}
		break;
	case SCHED_FGRUNT_SUPPRESS:
		{
			if ( m_fFirstEncounter )
			{
				m_fFirstEncounter = FALSE;// after first encounter, leader won't issue handsigns anymore when he has a new enemy
				return &slFGruntGruntSignalSuppress[ 0 ];
			}
			else
			{
				return &slFGruntGruntSuppress[ 0 ];
			}
		}
		break;
	case SCHED_FAIL:
		{
			if ( m_hEnemy != NULL )
			{
				// grunt has an enemy, so pick a different default fail schedule most likely to help recover.
				return &slFGruntGruntCombatFail[ 0 ];
			}

			return &slFGruntGruntFail[ 0 ];
		}
		break;
	case SCHED_FGRUNT_REPEL:
		{
			if (pev->velocity.z > -128)
				pev->velocity.z -= 32;
			return &slFGruntGruntRepel[ 0 ];
		}
		break;
	case SCHED_FGRUNT_REPEL_ATTACK:
		{
			if (pev->velocity.z > -128)
				pev->velocity.z -= 32;
			return &slFGruntGruntRepelAttack[ 0 ];
		}
		break;
	case SCHED_FGRUNT_REPEL_LAND:
		{
			return &slFGruntGruntRepelLand[ 0 ];
		}
		break;
	default:
		{
			return CRCAllyMonster :: GetScheduleOfType ( Type );
		}
	}
}
//=========================================================
// SetActivity 
//=========================================================
void CFGrunt :: SetActivity ( Activity NewActivity )
{
	int	iSequence = ACTIVITY_NOT_AVAILABLE;
	int	iGaitSequence = ACTIVITY_NOT_AVAILABLE;
	void *pmodel = GET_MODEL_PTR( ENT(pev) );

	switch ( NewActivity)
	{
	case ACT_RANGE_ATTACK1:
		// grunt is either shooting standing or shooting crouched
		if (FBitSet( pev->weapons, FGRUNT_9MMAR))
		{
			if ( m_fStanding )
			{
				// get aimable sequence
				iSequence = LookupSequence( "standing_mp5" );
			}
			else
			{
				// get crouching shoot
				iSequence = LookupSequence( "crouching_mp5" );
			}
		}
		else if (FBitSet( pev->weapons, FGRUNT_SHOTGUN))
		{
			// get aimable sequence
			iSequence = LookupSequence( "standing_shotgun" );
		}
		else
		{
			if ( m_fStanding )
			{
				// get aimable sequence
				iSequence = LookupSequence( "standing_saw" );
			}
			else
			{
				// get crouching shoot
				iSequence = LookupSequence( "crouching_saw" );
			}
		}
		break;
	case ACT_RELOAD:
		// grunt is either shooting standing or shooting crouched
		if (FBitSet( pev->weapons, FGRUNT_9MMAR))
		{
			iSequence = LookupSequence( "reload_mp5" );
		}
		else if (FBitSet( pev->weapons, FGRUNT_SHOTGUN))
		{
			iSequence = LookupSequence( "reload_shotgun" );
		}
		else
		{
			iSequence = LookupSequence( "reload_saw" );
		}
		break;
	case ACT_RANGE_ATTACK2:
		// get toss anim
		iSequence = LookupSequence( "throwgrenade" );
		break;
	case ACT_RUN:
		if ( pev->health <= FGRUNT_LIMP_HEALTH )
		{
			// limp!
			iSequence = LookupActivity ( ACT_RUN_HURT );
		}
		else
		{
			iSequence = LookupActivity ( NewActivity );
		}
		break;
	case ACT_WALK:
		if ( pev->health <= FGRUNT_LIMP_HEALTH )
		{
			// limp!
			iSequence = LookupActivity ( ACT_WALK_HURT );
		}
		else
		{
			iSequence = LookupActivity ( NewActivity );
		}
		break;
	case ACT_IDLE:
		if ( m_MonsterState == MONSTERSTATE_COMBAT )
		{
			NewActivity = ACT_IDLE_ANGRY;
		}
		iSequence = LookupActivity ( NewActivity );
		break;
	default:
		iSequence = LookupActivity ( NewActivity );
		break;
	}

	m_Activity = NewActivity; // Go ahead and set this so it doesn't keep trying when the anim is not present

	// Set to the desired anim, or default anim if the desired is not present
	if ( iSequence > ACTIVITY_NOT_AVAILABLE )
	{
		if ( pev->sequence != iSequence || !m_fSequenceLoops )
		{
			pev->frame = 0;
		}

		pev->sequence		= iSequence;	// Set to the reset anim (if it's there)
		ResetSequenceInfo( );
		SetYawSpeed();
	}
	else
	{
		// Not available try to get default anim
		ALERT ( at_console, "%s has no sequence for act:%d\n", STRING(pev->classname), NewActivity );
		pev->sequence		= 0;	// Set to the reset anim (if it's there)
	}
}
//=========================================================
// GetSchedule - Decides which type of schedule best suits
// the monster's current state and conditions. Then calls
// monster's member function to get a pointer to a schedule
// of the proper type.
//=========================================================
Schedule_t *CFGrunt :: GetSchedule ( void )
{
	// grunts place HIGH priority on running away from danger sounds.
	if ( HasConditions(bits_COND_HEAR_SOUND) )
	{
		CSound *pSound;
		pSound = PBestSound();

		ASSERT( pSound != NULL );
		if ( pSound)
		{
			if (pSound->m_iType & bits_SOUND_DANGER)
			{
				// dangerous sound nearby!
				
				//!!!KELLY - currently, this is the grunt's signal that a grenade has landed nearby,
				// and the grunt should find cover from the blast
				// good place for "SHIT!" or some other colorful verbal indicator of dismay.
				// It's not safe to play a verbal order here "Scatter", etc cause 
				// this may only affect a single individual in a squad. 
				
				if (FOkToSpeak())
				{
					SENTENCEG_PlayRndSz( ENT(pev), "FG_GREN", VOL_NORM, ATTN_NORM, 0, m_voicePitch);
					JustSpoke();
				}
				return GetScheduleOfType( SCHED_TAKE_COVER_FROM_BEST_SOUND );
			}
		}
	}
	// flying? If PRONE, barnacle has me. IF not, it's assumed I am rapelling. 
	if ( pev->movetype == MOVETYPE_FLY && m_MonsterState != MONSTERSTATE_PRONE )
	{
		if (pev->flags & FL_ONGROUND)
		{
			// just landed
			pev->movetype = MOVETYPE_STEP;
			return GetScheduleOfType ( SCHED_FGRUNT_REPEL_LAND );
		}
		else
		{
			// repel down a rope, 
			if ( m_MonsterState == MONSTERSTATE_COMBAT )
				return GetScheduleOfType ( SCHED_FGRUNT_REPEL_ATTACK );
			else
				return GetScheduleOfType ( SCHED_FGRUNT_REPEL );
		}
	}
	if ( HasConditions( bits_COND_ENEMY_DEAD ) && FOkToSpeak() )
	{
		PlaySentence( "FG_KILL", 4, VOL_NORM, ATTN_NORM );
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
// new enemy
			if ( HasConditions(bits_COND_NEW_ENEMY) )
			{
				if ( InSquad() )
				{
					((CRCAllyMonster *)MySquadLeader())->m_fEnemyEluded = FALSE;
					if(!FBitSet(pev->weapons, FGRUNT_9MMAR ) && !FBitSet(pev->weapons, FGRUNT_SHOTGUN) && !FBitSet(pev->weapons, FGRUNT_M249))
					{
						if ( HasConditions ( bits_COND_CAN_MELEE_ATTACK1 ) )
						{
							return GetScheduleOfType ( SCHED_MELEE_ATTACK1 );
						}
						else
						{
							// hide!
							return GetScheduleOfType( SCHED_TAKE_COVER_FROM_ENEMY );
						}
					}
					else
					{
						if ( !IsLeader() )
						{
							if ( HasConditions ( bits_COND_CAN_RANGE_ATTACK1 ) )
							{
								return GetScheduleOfType ( SCHED_FGRUNT_SUPPRESS );
							}
							else
							{
								return GetScheduleOfType ( SCHED_FGRUNT_ESTABLISH_LINE_OF_FIRE );
							}
						}
						else 
						{
							//!!!KELLY - the leader of a squad of grunts has just seen the player or a 
							// monster and has made it the squad's enemy. You
							// can check pev->flags for FL_CLIENT to determine whether this is the player
							// or a monster. He's going to immediately start
							// firing, though. If you'd like, we can make an alternate "first sight" 
							// schedule where the leader plays a handsign anim
							// that gives us enough time to hear a short sentence or spoken command
							// before he starts pluggin away.
							if (FOkToSpeak())// && RANDOM_LONG(0,1))
							{
								if ((m_hEnemy != NULL) &&
										(m_hEnemy->Classify() != CLASS_PLAYER_ALLY) && 
										(m_hEnemy->Classify() != CLASS_HUMAN_MILITARY) && 
										(m_hEnemy->Classify() != CLASS_HUMAN_PASSIVE) && 
										(m_hEnemy->Classify() != CLASS_MACHINE))
									// monster
									SENTENCEG_PlayRndSz( ENT(pev), "FG_ALERT", VOL_NORM, ATTN_NORM, 0, m_voicePitch);
								else
									// player
									SENTENCEG_PlayRndSz( ENT(pev), "FG_ATTACK", VOL_NORM, ATTN_NORM, 0, m_voicePitch);

								JustSpoke();
							}
						
							if ( HasConditions ( bits_COND_CAN_RANGE_ATTACK1 ) )
							{
								return GetScheduleOfType ( SCHED_FGRUNT_SUPPRESS );
							}
							else
							{
								return GetScheduleOfType ( SCHED_FGRUNT_ESTABLISH_LINE_OF_FIRE );
							}
						}
					}
				}
			}
// no ammo
			else if ( HasConditions ( bits_COND_NO_AMMO_LOADED ) )
			{
				//!!!KELLY - this individual just realized he's out of bullet ammo. 
				// He's going to try to find cover to run to and reload, but rarely, if 
				// none is available, he'll drop and reload in the open here. 
				return GetScheduleOfType ( SCHED_FGRUNT_COVER_AND_RELOAD );
			}
			
// damaged just a little
			else if ( HasConditions( bits_COND_LIGHT_DAMAGE ) )
			{
				// if hurt:
				// 90% chance of taking cover
				// 10% chance of flinch.

				int iPercent = RANDOM_LONG(0,99);

				if ( iPercent <= 90 && m_hEnemy != NULL )
				{
					if (FOkToSpeak()) // && RANDOM_LONG(0,1))
					{
						//SENTENCEG_PlayRndSz( ENT(pev), "HG_COVER", VOL_NORM, ATTN_NORM, 0, m_voicePitch);
						m_iSentence = FGRUNT_SENT_COVER;
						//JustSpoke();
					}
					// only try to take cover if we actually have an enemy!

					return GetScheduleOfType( SCHED_TAKE_COVER_FROM_ENEMY );
				}
				else
				{
					return GetScheduleOfType( SCHED_SMALL_FLINCH );
				}
			}
// can kick
			else if ( HasConditions ( bits_COND_CAN_MELEE_ATTACK1 ) )
			{
				return GetScheduleOfType ( SCHED_MELEE_ATTACK1 );
			}
// can grenade launch

			else if ( HasConditions ( bits_COND_CAN_RANGE_ATTACK2 ) && OccupySlot( bits_SLOTS_FGRUNT_GRENADE ) )
			{
				// shoot a grenade if you can
				return GetScheduleOfType( SCHED_RANGE_ATTACK2 );
			}
// can shoot
			else if ( HasConditions ( bits_COND_CAN_RANGE_ATTACK1 ) )
			{
				if ( InSquad() )
				{
					// if the enemy has eluded the squad and a squad member has just located the enemy
					// and the enemy does not see the squad member, issue a call to the squad to waste a 
					// little time and give the player a chance to turn.
					if ( ((CRCAllyMonster *)MySquadLeader())->m_fEnemyEluded && !HasConditions ( bits_COND_ENEMY_FACING_ME ) )
					{
						((CRCAllyMonster *)MySquadLeader())->m_fEnemyEluded = FALSE;
						return GetScheduleOfType ( SCHED_FGRUNT_FOUND_ENEMY );
					}
				}

				if (OccupySlot ( bits_SLOTS_FGRUNT_ENGAGE ))
				{
					// try to take an available ENGAGE slot
					return GetScheduleOfType( SCHED_RANGE_ATTACK1 );
				}
				else if ( HasConditions ( bits_COND_CAN_RANGE_ATTACK2 ) && OccupySlot( bits_SLOTS_FGRUNT_GRENADE ) )
				{
					// throw a grenade if can and no engage slots are available
					return GetScheduleOfType( SCHED_RANGE_ATTACK2 );
				}
				else
				{
					// hide!
					return GetScheduleOfType( SCHED_TAKE_COVER_FROM_ENEMY );
				}
			}
// can't see enemy
			else if ( HasConditions( bits_COND_ENEMY_OCCLUDED ) )
			{
				if ( HasConditions( bits_COND_CAN_RANGE_ATTACK2 ) && OccupySlot( bits_SLOTS_FGRUNT_GRENADE ) )
				{
					//!!!KELLY - this grunt is about to throw or fire a grenade at the player. Great place for "fire in the hole"  "frag out" etc
					if (FOkToSpeak())
					{
						SENTENCEG_PlayRndSz( ENT(pev), "FG_THROW", VOL_NORM, ATTN_NORM, 0, m_voicePitch);
						JustSpoke();
					}
					return GetScheduleOfType( SCHED_RANGE_ATTACK2 );
				}
				else if ( OccupySlot( bits_SLOTS_FGRUNT_ENGAGE ) )
				{
					return GetScheduleOfType( SCHED_FGRUNT_ESTABLISH_LINE_OF_FIRE );
				}
				else
				{
					//!!!KELLY - grunt is going to stay put for a couple seconds to see if
					// the enemy wanders back out into the open, or approaches the
					// grunt's covered position. Good place for a taunt, I guess?
					if (FOkToSpeak() && RANDOM_LONG(0,1))
					{
						SENTENCEG_PlayRndSz( ENT(pev), "FG_TAUNT", VOL_NORM, ATTN_NORM, 0, m_voicePitch);
						JustSpoke();
					}
					return GetScheduleOfType( SCHED_STANDOFF );
				}
			}
			
			if ( HasConditions( bits_COND_SEE_ENEMY ) && !HasConditions ( bits_COND_CAN_RANGE_ATTACK1 ) )
			{
				return GetScheduleOfType ( SCHED_FGRUNT_ESTABLISH_LINE_OF_FIRE );
			}
		}
		break;
	case MONSTERSTATE_ALERT:	
	case MONSTERSTATE_IDLE:
		if ( HasConditions ( bits_COND_NO_AMMO_LOADED ) )
		{
			return GetScheduleOfType ( SCHED_RELOAD );
		}
		// Behavior for following the player
		if ( IsFollowing() && m_hTargetEnt )
		{
			if ( !m_hTargetEnt->IsAlive() )
			{
				// UNDONE: Comment about the recently dead player here?
				StopFollowing( FALSE );
				break;
			}

			// If I'm already close enough to my target
			if ( TargetDistance() <= 128 )
			{
				if (HasConditions( bits_COND_CLIENT_PUSH ))	// Player wants me to move
					return GetScheduleOfType( SCHED_MOVE_AWAY_FOLLOW );
			}
			return GetScheduleOfType( SCHED_TARGET_FACE );	// Just face and follow.
		}

		if ( HasConditions( bits_COND_CLIENT_PUSH ) )
		{
			return GetScheduleOfType( SCHED_MOVE_AWAY );
		}

		// try to say something about smells
		TrySmellTalk();
		break;
	}
	
	return CRCAllyMonster :: GetSchedule();
}
MONSTERSTATE CFGrunt :: GetIdealState ( void )
{
	return CRCAllyMonster::GetIdealState();
}
void CFGrunt::DeclineFollowing( void )
{
	PlaySentence( "FG_STOP", 2, VOL_NORM, ATTN_NORM );
}

//=========================================================
// BGrunt Dead PROP
//
// Designer selects a pose in worldcraft, 0 through num_poses-1
// this value is added to what is selected as the 'first dead pose'
// among the monster's normal animations. All dead poses must
// appear sequentially in the model file. Be sure and set
// the m_iFirstPose properly!
//
//=========================================================

class CDeadFGrunt : public CBaseMonster
{
public:
	void Spawn( void );
	int	Classify ( void ) { return	CLASS_PLAYER_ALLY; }

	void KeyValue( KeyValueData *pkvd );

	int		m_iHead;
	int	m_iPose;// which sequence to display
	static char *m_szPoses[7];
};

char *CDeadFGrunt::m_szPoses[] = { "deadstomach", "deadside", "deadsitting", "dead_on_back", "dead_headcrabbed", "hgrunt_dead_stomach", "dead_canyon" };

void CDeadFGrunt::KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "pose"))
	{
		m_iPose = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "head"))
	{
		m_iHead = atoi( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else 
		CBaseMonster::KeyValue( pkvd );
}

LINK_ENTITY_TO_CLASS( monster_fgrunt_dead, CDeadFGrunt );

//=========================================================
// ********** DeadBGrunt SPAWN **********
//=========================================================
void CDeadFGrunt :: Spawn( )
{
	PRECACHE_MODEL("models/hgrunt_opfor.mdl");
	SET_MODEL(ENT(pev), "models/hgrunt_opfor.mdl");

	pev->effects		= 0;
	pev->yaw_speed		= 8;
	pev->sequence		= 0;
	m_bloodColor		= BLOOD_COLOR_RED;

	pev->sequence = LookupSequence( m_szPoses[m_iPose] );
	if (pev->sequence == -1)
	{
		ALERT ( at_console, "Dead fgrunt with bad pose\n" );
	}
	// Corpses have less health
	pev->health			= 8;

	SetBodygroup( MN_GUN_GROUP, MN_GUN_NONE );

	if ( m_iHead == 0 )
	{
		SetBodygroup( MN_HEAD_GROUP, MN_HEAD_MASK );
	}
	if ( m_iHead == 1 )
	{
		SetBodygroup( MN_HEAD_GROUP, MN_HEAD_CMDR_BLACK );
	}
	if ( m_iHead == 2 )
	{
		SetBodygroup( MN_HEAD_GROUP, MN_HEAD_SHOTGUN );
	}
	if ( m_iHead == 3 )
	{
		SetBodygroup( MN_HEAD_GROUP, MN_HEAD_SAW_WHT );
	}
	if ( m_iHead == 4 )
	{
		SetBodygroup( MN_HEAD_GROUP, MN_HEAD_SAW_BLK );
	}
	if ( m_iHead == 5 )
	{
		SetBodygroup( MN_HEAD_GROUP, MN_HEAD_MP );
	}
	if ( m_iHead == 6 )
	{
		SetBodygroup( MN_HEAD_GROUP, MN_HEAD_CMDR_OLD );
	}
	if ( m_iHead == 7 )
	{
		SetBodygroup( MN_HEAD_GROUP, MN_HEAD_CMDR_BLACK );
	}

	MonsterInitDead();
}
