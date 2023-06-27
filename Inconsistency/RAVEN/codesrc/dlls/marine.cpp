//========= Copyright © 2009-2010, Reckoning Team, All rights reserved. =============//
//																					 //
// Purpose:																			 //
//																					 //
// $NoKeywords: $																	 //
//===================================================================================//

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
#define	MARINE_CLIP_SIZE				32 // how many bullets in a clip? - NOTE: 3 round burst sound, so keep as 3 * x!
#define MARINE_LIMP_HEALTH				20

#define MARINE_MEDIC_WAIT				5 // Wait ten seconds before calling for medic again.

#define MARINE_TYPE_NORMAL			0
#define MARINE_TYPE_MEDIC			1
#define MARINE_TYPE_ENGINEER		2

#define MARINE_SIG552				( 1 << 0)
#define MARINE_HANDGRENADE			( 1 << 1)
#define MARINE_GRENADELAUNCHER		( 1 << 2)
#define MARINE_SHOTGUN				( 1 << 3)
#define MARINE_M249					( 1 << 4)

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
#define bits_COND_MARINE_NOFIRE	( bits_COND_SPECIAL1 )
//=========================================================
// monster-specific tasks
//=========================================================
enum 
{
	TASK_MARINE_FACE_TOSS_DIR = LAST_RCALLYMONSTER_TASK + 1,
	TASK_MARINE_CHECK_FIRE,
	TASK_MARINE_SAY_HEAL,
	TASK_MARINE_HEAL,
	TASK_MARINE_FIND_MEDIC,
	TASK_MARINE_STOP_FOLLOWING,
};
//=========================================================
// monster heads
//=========================================================

// Head group
#define MN_HEAD_GROUP				2
#define MN_HEAD_MASK				0
#define MN_HEAD_SHOTGUN				1
#define MN_HEAD_SAW					2
#define MN_HEAD_SAW_BLACK			3
#define MN_HEAD_MAJOR				4
#define MN_HEAD_BERET_BLACK			5
#define MN_HEAD_MEDIC				6
#define MN_HEAD_TORCH				7

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define		MARINE_AE_RELOAD		( 2 )
#define		MARINE_AE_KICK			( 3 )
#define		MARINE_AE_BURST1		( 4 )
#define		MARINE_AE_BURST2		( 5 ) 
#define		MARINE_AE_BURST3		( 6 ) 
#define		MARINE_AE_GREN_TOSS		( 7 )
#define		MARINE_AE_GREN_LAUNCH	( 8 )
#define		MARINE_AE_GREN_DROP		( 9 )
#define		MARINE_AE_CAUGHT_ENEMY	( 10) // grunt established sight with an enemy (player only) that had previously eluded the squad.
#define		MARINE_AE_DROP_GUN		( 11) // grunt (probably dead) is dropping his mp5.
#define		MARINE_AE_HIDEGUN		( 15)
#define		MARINE_AE_SHOWNEEDLE	( 16)
#define		MARINE_AE_HIDENEEDLE	( 17)
#define		MARINE_AE_SHOWGUN		( 18)
#define		MARINE_AE_SHOWTORCH		( 19)
#define		MARINE_AE_HIDETORCH		( 20)
#define		MARINE_AE_ONGAS			( 21)
#define		MARINE_AE_OFFGAS		( 22)

//=========================================================
// monster-specific schedule types
//=========================================================
enum
{
	SCHED_MARINE_SUPPRESS = LAST_RCALLYMONSTER_SCHEDULE + 1,
	SCHED_MARINE_ESTABLISH_LINE_OF_FIRE,// move to a location to set up an attack against the enemy. (usually when a friendly is in the way).
	SCHED_MARINE_COVER_AND_RELOAD,
	SCHED_MARINE_SWEEP,
	SCHED_MARINE_FOUND_ENEMY,
	SCHED_MARINE_REPEL,
	SCHED_MARINE_REPEL_ATTACK,
	SCHED_MARINE_REPEL_LAND,
	SCHED_MARINE_WAIT_FACE_ENEMY,
	SCHED_MARINE_TAKECOVER_FAILED,// special schedule type that forces analysis of conditions and picks the best possible schedule to recover from this type of failure.
	SCHED_MARINE_ELOF_FAIL,
	SCHED_MARINE_FIND_MEDIC,
	SCHED_MARINE_HEAL_CHASE,
	SCHED_MARINE_HEAL,
};
class CMarine : public CRCAllyMonster
{
public:
	void Spawn( void );
	void Precache( void );
	void SetYawSpeed( void );
	int  ISoundMask( void );
	int  Classify ( void );
	void StepSound( float volume, int material_type );
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
	Vector GetGunPosition( int stance = STANCE_ACTUAL );
	void Shoot ( void );
	void Shotgun ( void );
	void M249 ( void );
	// Override these to set behavior
	CBaseEntity	*Kick( void );
	Schedule_t *GetScheduleOfType ( int Type );
	Schedule_t *GetSchedule ( void );
	MONSTERSTATE GetIdealState ( void );

	virtual BOOL CanAnswer( void );
	virtual BOOL Enl_CanInfest( void ) { return TRUE; }
	BOOL ShouldSeekShootingSpot( void );

	void DeathSound( void );
	void PainSound( void );
	void GibMonster( void );
	void TalkInit( void );

	BOOL NoFriendlyFire( void );
	BOOL FOkToSpeak( void );
	void JustSpoke( void );

	void MakeGas( void );
	void UpdateGas( void );
	void KillGas( void );

	BOOL IsMedic( void );
	BOOL CanHeal( CBaseEntity *pHealTarget ); // Can we heal the player, or the injured grunt?
	void Heal( void );// Lets apply the healing.

	virtual BOOL CanCrouch( void ) { return TRUE; }

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
	float m_flMarineWaitTime;
	float m_flHealAnount;
	float m_healTime;

	int		m_flLinkToggle;// how much pain has the player inflicted on me?

	Vector	m_vecTossVelocity;

	CBeam *m_pBeam;

	BOOL	m_fThrowGrenade;
	BOOL	m_fStanding;
	BOOL	m_fFirstEncounter;// only put on the handsign show in the squad's first encounter.
	BOOL	m_fDepleteLine;
	BOOL	m_fOneHanded;
	BOOL	m_bImmortal;
	BOOL	m_bNeedleOut;
	int		m_cClipSize;

	int		m_iSentence;
	int		m_iHead;
	int		m_iType;
	static const char *pGruntSentences[];

	CUSTOM_SCHEDULES;
};

LINK_ENTITY_TO_CLASS( monster_marine, CMarine );
LINK_ENTITY_TO_CLASS( monster_marine_medic, CMarine );
LINK_ENTITY_TO_CLASS( monster_marine_engineer, CMarine );

TYPEDESCRIPTION	CMarine::m_SaveData[] = 
{
	DEFINE_FIELD( CMarine, m_pBeam, FIELD_CLASSPTR ),
	DEFINE_FIELD( CMarine, m_flNextGrenadeCheck, FIELD_TIME ),
	DEFINE_FIELD( CMarine, m_flNextPainTime, FIELD_TIME ),
	DEFINE_FIELD( CMarine, m_flMarineWaitTime, FIELD_TIME ),
	DEFINE_FIELD( CMarine, m_vecTossVelocity, FIELD_VECTOR ),
	DEFINE_FIELD( CMarine, m_fThrowGrenade, FIELD_BOOLEAN ),
	DEFINE_FIELD( CMarine, m_fStanding, FIELD_BOOLEAN ),
	DEFINE_FIELD( CMarine, m_fFirstEncounter, FIELD_BOOLEAN ),
	DEFINE_FIELD( CMarine, m_iType, FIELD_INTEGER ),
	DEFINE_FIELD( CMarine, m_cClipSize, FIELD_INTEGER ),
	DEFINE_FIELD( CMarine, m_iHead, FIELD_INTEGER ),
	DEFINE_FIELD( CMarine, m_fDepleteLine, FIELD_BOOLEAN ),
	DEFINE_FIELD( CMarine, m_fOneHanded, FIELD_BOOLEAN ),
	DEFINE_FIELD( CMarine, m_flHealAnount, FIELD_FLOAT ),
	DEFINE_FIELD( CMarine, m_healTime, FIELD_TIME ),
	DEFINE_FIELD( CMarine, m_bImmortal, FIELD_BOOLEAN ),
	DEFINE_FIELD( CMarine, m_bNeedleOut, FIELD_BOOLEAN ),
};

IMPLEMENT_SAVERESTORE( CMarine, CRCAllyMonster );
const char *CMarine::pGruntSentences[] = 
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
	MARINE_SENT_NONE = -1,
	MARINE_SENT_GREN = 0,
	MARINE_SENT_ALERT,
	MARINE_SENT_MONSTER,
	MARINE_SENT_COVER,
	MARINE_SENT_THROW,
	MARINE_SENT_CHARGE,
	MARINE_SENT_TAUNT,
} MARINE_SENTENCE_TYPES;

//=========================================================
// KeyValue
//
// !!! netname entvar field is used in squadmonster for groupname!!!
//=========================================================
void CMarine :: KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "head"))
	{
		m_iHead = atoi( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "immortal"))
	{
		m_bImmortal = atoi( pkvd->szValue );
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
BOOL CMarine :: NoFriendlyFire( void )
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

void CMarine::Killed( entvars_t *pevAttacker, int iGib )
{
	SetUse( NULL );

	CRCAllyMonster::Killed( pevAttacker, iGib );
}

//=========================================================
// ShouldSeekShootingSpot - See if we want to shoot or grab
//=========================================================
BOOL CMarine::ShouldSeekShootingSpot( void )
{
	return (pev->weapons != 0);
}

//=========================================================
// someone else is talking - don't speak
//=========================================================
BOOL CMarine :: FOkToSpeak( void )
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
void CMarine :: JustSpoke( void )
{
	CRCAllyMonster::g_talkWaitTime = gpGlobals->time + RANDOM_FLOAT(1.5, 2.0);
	m_iSentence = MARINE_SENT_NONE;
}
//=========================================================
// IRelationship - overridden because Male Assassins are 
// Human Grunt's nemesis.
//=========================================================
int CMarine::IRelationship ( CBaseEntity *pTarget )
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
Task_t	tlMarineFollow[] =
{
	{ TASK_MOVE_TO_TARGET_RANGE,(float)128		},	// Move within 128 of target ent (client)
	{ TASK_SET_SCHEDULE,		(float)SCHED_TARGET_FACE },
};

Schedule_t	slMarineFollow[] =
{
	{
		tlMarineFollow,
		ARRAYSIZE ( tlMarineFollow ),
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_HEAR_SOUND |
		bits_COND_PROVOKED,
		bits_SOUND_DANGER,
		"Follow"
	},
};
Task_t	tlMarineFaceTarget[] =
{
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_FACE_TARGET,			(float)0		},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_SET_SCHEDULE,		(float)SCHED_TARGET_CHASE },
};

Schedule_t	slMarineFaceTarget[] =
{
	{
		tlMarineFaceTarget,
		ARRAYSIZE ( tlMarineFaceTarget ),
		bits_COND_CLIENT_PUSH	|
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_BLOCKING_PATH |
		bits_COND_HEAR_SOUND |
		bits_COND_PROVOKED,
		bits_SOUND_DANGER,
		"FaceTarget"
	},
};


Task_t	tlMarineIdleStand[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_WAIT,				(float)2		}, // repick IDLESTAND every two seconds.
	{ TASK_TLK_HEADRESET,		(float)0		}, // reset head position
};

Schedule_t	slMarineIdleStand[] =
{
	{ 
		tlMarineIdleStand,
		ARRAYSIZE ( tlMarineIdleStand ), 
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_HEAR_SOUND	|
		bits_COND_SMELL			|
		bits_COND_FOLLOW_TARGET_TOOFAR |
		bits_COND_BLOCKING_PATH |
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
Task_t	tlMarineFail[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_WAIT,				(float)0.5		},
	{ TASK_WAIT_PVS,			(float)0		},
};

Schedule_t	slMarineFail[] =
{
	{
		tlMarineFail,
		ARRAYSIZE ( tlMarineFail ),
		bits_COND_CAN_RANGE_ATTACK1 |
		bits_COND_CAN_RANGE_ATTACK2 |
		bits_COND_CAN_MELEE_ATTACK1 |
		bits_COND_CAN_MELEE_ATTACK2,
		0,
		"Marine Fail"
	},
};

//=========================================================
// FGrunt Combat Fail
//=========================================================
Task_t	tlMarineCombatFail[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_WAIT_FACE_ENEMY,		(float)0.5		},
	{ TASK_WAIT_PVS,			(float)0		},
};

Schedule_t	slMarineCombatFail[] =
{
	{
		tlMarineCombatFail,
		ARRAYSIZE ( tlMarineCombatFail ),
		bits_COND_CAN_RANGE_ATTACK1	|
		bits_COND_CAN_RANGE_ATTACK2,
		0,
		"Marine Combat Fail"
	},
};

//=========================================================
// Victory dance!
//=========================================================
Task_t	tlMarineVictoryDance[] =
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

Schedule_t	slMarineVictoryDance[] =
{
	{ 
		tlMarineVictoryDance,
		ARRAYSIZE ( tlMarineVictoryDance ), 
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE,
		0,
		"MarineVictoryDance"
	},
};

//=========================================================
// Establish line of fire - move to a position that allows
// the grunt to attack.
//=========================================================
Task_t tlMarineEstablishLineOfFire[] = 
{
	{ TASK_SET_FAIL_SCHEDULE,	(float)SCHED_MARINE_ELOF_FAIL	},
	{ TASK_GET_PATH_TO_ENEMY,	(float)0						},
	{ TASK_RUN_PATH,			(float)0						},
	{ TASK_WAIT_FOR_MOVEMENT,	(float)0						},
};

Schedule_t slMarineEstablishLineOfFire[] =
{
	{ 
		tlMarineEstablishLineOfFire,
		ARRAYSIZE ( tlMarineEstablishLineOfFire ),
		bits_COND_NEW_ENEMY			|
		bits_COND_ENEMY_DEAD		|
		bits_COND_CAN_RANGE_ATTACK1	|
		bits_COND_CAN_MELEE_ATTACK1	|
		bits_COND_CAN_RANGE_ATTACK2	|
		bits_COND_CAN_MELEE_ATTACK2	|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"MarineEstablishLineOfFire"
	},
};

//=========================================================
// FGruntFoundEnemy - FGrunt established sight with an enemy
// that was hiding from the squad.
//=========================================================
Task_t	tlMarineFoundEnemy[] =
{
	{ TASK_STOP_MOVING,				0							},
	{ TASK_FACE_ENEMY,				(float)0					},
	{ TASK_PLAY_SEQUENCE_FACE_ENEMY,(float)ACT_SIGNAL1			},
};

Schedule_t	slMarineFoundEnemy[] =
{
	{ 
		tlMarineFoundEnemy,
		ARRAYSIZE ( tlMarineFoundEnemy ), 
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"MarineFoundEnemy"
	},
};

//=========================================================
// GruntCombatFace Schedule
//=========================================================
Task_t	tlMarineCombatFace1[] =
{
	{ TASK_STOP_MOVING,				0							},
	{ TASK_SET_ACTIVITY,			(float)ACT_IDLE				},
	{ TASK_FACE_ENEMY,				(float)0					},
	{ TASK_WAIT,					(float)1.5					},
	{ TASK_SET_SCHEDULE,			(float)SCHED_MARINE_SWEEP	},
};

Schedule_t	slMarineCombatFace[] =
{
	{ 
		tlMarineCombatFace1,
		ARRAYSIZE ( tlMarineCombatFace1 ), 
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
Task_t	tlMarineSignalSuppress[] =
{
	{ TASK_STOP_MOVING,						0						},
	{ TASK_FACE_IDEAL,						(float)0				},
	{ TASK_PLAY_SEQUENCE_FACE_ENEMY,		(float)ACT_SIGNAL2		},
	{ TASK_FACE_ENEMY,						(float)0				},
	{ TASK_MARINE_CHECK_FIRE,			(float)0				},
	{ TASK_RANGE_ATTACK1,					(float)0				},
	{ TASK_FACE_ENEMY,						(float)0				},
	{ TASK_MARINE_CHECK_FIRE,			(float)0				},
	{ TASK_RANGE_ATTACK1,					(float)0				},
	{ TASK_FACE_ENEMY,						(float)0				},
	{ TASK_MARINE_CHECK_FIRE,			(float)0				},
	{ TASK_RANGE_ATTACK1,					(float)0				},
	{ TASK_FACE_ENEMY,						(float)0				},
	{ TASK_MARINE_CHECK_FIRE,			(float)0				},
	{ TASK_RANGE_ATTACK1,					(float)0				},
	{ TASK_FACE_ENEMY,						(float)0				},
	{ TASK_MARINE_CHECK_FIRE,			(float)0				},
	{ TASK_RANGE_ATTACK1,					(float)0				},
};

Schedule_t	slMarineSignalSuppress[] =
{
	{ 
		tlMarineSignalSuppress,
		ARRAYSIZE ( tlMarineSignalSuppress ), 
		bits_COND_ENEMY_DEAD		|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_HEAR_SOUND		|
		bits_COND_MARINE_NOFIRE		|
		bits_COND_NO_AMMO_LOADED,

		bits_SOUND_DANGER,
		"SignalSuppress"
	},
};

Task_t	tlMarineSuppress[] =
{
	{ TASK_STOP_MOVING,			0							},
	{ TASK_FACE_ENEMY,			(float)0					},
	{ TASK_MARINE_CHECK_FIRE,	(float)0					},
	{ TASK_RANGE_ATTACK1,		(float)0					},
	{ TASK_FACE_ENEMY,			(float)0					},
	{ TASK_MARINE_CHECK_FIRE,	(float)0					},
	{ TASK_RANGE_ATTACK1,		(float)0					},
	{ TASK_FACE_ENEMY,			(float)0					},
	{ TASK_MARINE_CHECK_FIRE,	(float)0					},
	{ TASK_RANGE_ATTACK1,		(float)0					},
	{ TASK_FACE_ENEMY,			(float)0					},
	{ TASK_MARINE_CHECK_FIRE,	(float)0					},
	{ TASK_RANGE_ATTACK1,		(float)0					},
	{ TASK_FACE_ENEMY,			(float)0					},
	{ TASK_MARINE_CHECK_FIRE,	(float)0					},
	{ TASK_RANGE_ATTACK1,		(float)0					},
};

Schedule_t	slMarineSuppress[] =
{
	{ 
		tlMarineSuppress,
		ARRAYSIZE ( tlMarineSuppress ), 
		bits_COND_ENEMY_DEAD		|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_HEAR_SOUND		|
		bits_COND_MARINE_NOFIRE		|
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
Task_t	tlMarineWaitInCover[] =
{
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_SET_ACTIVITY,			(float)ACT_IDLE				},
	{ TASK_WAIT_FACE_ENEMY,			(float)1					},
};

Schedule_t	slMarineWaitInCover[] =
{
	{ 
		tlMarineWaitInCover,
		ARRAYSIZE ( tlMarineWaitInCover ), 
		bits_COND_NEW_ENEMY			|
		bits_COND_HEAR_SOUND		|
		bits_COND_CAN_RANGE_ATTACK1	|
		bits_COND_CAN_RANGE_ATTACK2	|
		bits_COND_CAN_MELEE_ATTACK1	|
		bits_COND_CAN_MELEE_ATTACK2,

		bits_SOUND_DANGER,
		"MarineWaitInCover"
	},
};

//=========================================================
// run to cover.
// !!!BUGBUG - set a decent fail schedule here.
//=========================================================
Task_t	tlMarineTakeCover1[] =
{
	{ TASK_STOP_MOVING,				(float)0							},
	{ TASK_SET_FAIL_SCHEDULE,		(float)SCHED_MARINE_TAKECOVER_FAILED	},
	{ TASK_WAIT,					(float)0.2							},
	{ TASK_FIND_COVER_FROM_ENEMY,	(float)0							},
	{ TASK_RUN_PATH,				(float)0							},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0							},
	{ TASK_REMEMBER,				(float)bits_MEMORY_INCOVER			},
	{ TASK_SET_SCHEDULE,			(float)SCHED_MARINE_WAIT_FACE_ENEMY	},
};

Schedule_t	slMarineTakeCover[] =
{
	{ 
		tlMarineTakeCover1,
		ARRAYSIZE ( tlMarineTakeCover1 ), 
		0,
		0,
		"TakeCover"
	},
};

//=========================================================
// drop grenade then run to cover.
//=========================================================
Task_t	tlMarineGrenadeCover1[] =
{
	{ TASK_STOP_MOVING,						(float)0							},
	{ TASK_FIND_COVER_FROM_ENEMY,			(float)99							},
	{ TASK_FIND_FAR_NODE_COVER_FROM_ENEMY,	(float)384							},
	{ TASK_PLAY_SEQUENCE,					(float)ACT_SPECIAL_ATTACK1			},
	{ TASK_CLEAR_MOVE_WAIT,					(float)0							},
	{ TASK_RUN_PATH,						(float)0							},
	{ TASK_WAIT_FOR_MOVEMENT,				(float)0							},
	{ TASK_SET_SCHEDULE,					(float)SCHED_MARINE_WAIT_FACE_ENEMY	},
};

Schedule_t	slMarineGrenadeCover[] =
{
	{ 
		tlMarineGrenadeCover1,
		ARRAYSIZE ( tlMarineGrenadeCover1 ), 
		0,
		0,
		"GrenadeCover"
	},
};


//=========================================================
// drop grenade then run to cover.
//=========================================================
Task_t	tlMarineTossGrenadeCover1[] =
{
	{ TASK_FACE_ENEMY,						(float)0							},
	{ TASK_RANGE_ATTACK2, 					(float)0							},
	{ TASK_SET_SCHEDULE,					(float)SCHED_TAKE_COVER_FROM_ENEMY	},
};

Schedule_t	slMarineTossGrenadeCover[] =
{
	{ 
		tlMarineTossGrenadeCover1,
		ARRAYSIZE ( tlMarineTossGrenadeCover1 ), 
		0,
		0,
		"TossGrenadeCover"
	},
};

//=========================================================
// hide from the loudest sound source (to run from grenade)
//=========================================================
Task_t	tlMarineTakeCoverFromBestSound[] =
{
	{ TASK_SET_FAIL_SCHEDULE,			(float)SCHED_COWER			},// duck and cover if cannot move from explosion
	{ TASK_STOP_MOVING,					(float)0					},
	{ TASK_FIND_COVER_FROM_BEST_SOUND,	(float)0					},
	{ TASK_RUN_PATH,					(float)0					},
	{ TASK_WAIT_FOR_MOVEMENT,			(float)0					},
	{ TASK_REMEMBER,					(float)bits_MEMORY_INCOVER	},
	{ TASK_TURN_LEFT,					(float)179					},
};

Schedule_t	slMarineTakeCoverFromBestSound[] =
{
	{ 
		tlMarineTakeCoverFromBestSound,
		ARRAYSIZE ( tlMarineTakeCoverFromBestSound ), 
		0,
		0,
		"MarineTakeCoverFromBestSound"
	},
};

//=========================================================
// Grunt reload schedule
//=========================================================
Task_t	tlMarineHideReload[] =
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

Schedule_t slMarineHideReload[] = 
{
	{
		tlMarineHideReload,
		ARRAYSIZE ( tlMarineHideReload ),
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_HEAR_SOUND,

		bits_SOUND_DANGER,
		"MarineHideReload"
	}
};

//=========================================================
// Do a turning sweep of the area
//=========================================================
Task_t	tlMarineSweep[] =
{
	{ TASK_TURN_LEFT,			(float)179	},
	{ TASK_WAIT,				(float)0.1	},
	{ TASK_TURN_LEFT,			(float)179	},
	{ TASK_WAIT,				(float)0.1	},
};

Schedule_t	slMarineSweep[] =
{
	{ 
		tlMarineSweep,
		ARRAYSIZE ( tlMarineSweep ), 
		
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_CAN_RANGE_ATTACK1	|
		bits_COND_CAN_RANGE_ATTACK2	|
		bits_COND_HEAR_SOUND,

		bits_SOUND_WORLD		|// sound flags
		bits_SOUND_DANGER		|
		bits_SOUND_PLAYER,

		"Marine Sweep"
	},
};

//=========================================================
// primary range attack. Overriden because base class stops attacking when the enemy is occluded.
// grunt's grenade toss requires the enemy be occluded.
//=========================================================
Task_t	tlMarineRangeAttack1A[] =
{
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_MARINE_CHECK_FIRE,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_MARINE_CHECK_FIRE,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_MARINE_CHECK_FIRE,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_MARINE_CHECK_FIRE,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
};

Schedule_t	slMarineRangeAttack1A[] =
{
	{ 
		tlMarineRangeAttack1A,
		ARRAYSIZE ( tlMarineRangeAttack1A ), 
		bits_COND_NEW_ENEMY			|
		bits_COND_ENEMY_DEAD		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_ENEMY_OCCLUDED	|
		bits_COND_HEAR_SOUND		|
		bits_COND_MARINE_NOFIRE		|
		bits_COND_NO_AMMO_LOADED,
		
		bits_SOUND_DANGER,
		"Range Attack1A"
	},
};


//=========================================================
// primary range attack. Overriden because base class stops attacking when the enemy is occluded.
// grunt's grenade toss requires the enemy be occluded.
//=========================================================
Task_t	tlMarineRangeAttack1B[] =
{
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_MARINE_CHECK_FIRE,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_MARINE_CHECK_FIRE,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_MARINE_CHECK_FIRE,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_MARINE_CHECK_FIRE,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
};

Schedule_t	slMarineRangeAttack1B[] =
{
	{ 
		tlMarineRangeAttack1B,
		ARRAYSIZE ( tlMarineRangeAttack1B ), 
		bits_COND_NEW_ENEMY			|
		bits_COND_ENEMY_DEAD		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_ENEMY_OCCLUDED	|
		bits_COND_NO_AMMO_LOADED	|
		bits_COND_MARINE_NOFIRE		|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"Range Attack1B"
	},
};

//=========================================================
// secondary range attack. Overriden because base class stops attacking when the enemy is occluded.
// grunt's grenade toss requires the enemy be occluded.
//=========================================================
Task_t	tlMarineRangeAttack2[] =
{
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_MARINE_FACE_TOSS_DIR,		(float)0					},
	{ TASK_PLAY_SEQUENCE,			(float)ACT_RANGE_ATTACK2	},
	{ TASK_SET_SCHEDULE,			(float)SCHED_MARINE_WAIT_FACE_ENEMY	},// don't run immediately after throwing grenade.
};

Schedule_t	slMarineRangeAttack2[] =
{
	{ 
		tlMarineRangeAttack2,
		ARRAYSIZE ( tlMarineRangeAttack2 ), 
		0,
		0,
		"RangeAttack2"
	},
};


//=========================================================
// repel 
//=========================================================
Task_t	tlMarineRepel[] =
{
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_FACE_IDEAL,			(float)0		},
	{ TASK_PLAY_SEQUENCE,		(float)ACT_GLIDE 	},
};

Schedule_t	slMarineRepel[] =
{
	{ 
		tlMarineRepel,
		ARRAYSIZE ( tlMarineRepel ), 
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
Task_t	tlMarineRepelAttack[] =
{
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_PLAY_SEQUENCE,		(float)ACT_FLY 	},
};

Schedule_t	slMarineRepelAttack[] =
{
	{ 
		tlMarineRepelAttack,
		ARRAYSIZE ( tlMarineRepelAttack ), 
		bits_COND_ENEMY_OCCLUDED,
		0,
		"Repel Attack"
	},
};

//=========================================================
// repel land
//=========================================================
Task_t	tlMarineRepelLand[] =
{
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_PLAY_SEQUENCE,		(float)ACT_LAND	},
	{ TASK_GET_PATH_TO_LASTPOSITION,(float)0				},
	{ TASK_RUN_PATH,				(float)0				},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0				},
	{ TASK_CLEAR_LASTPOSITION,		(float)0				},
};

Schedule_t	slMarineRepelLand[] =
{
	{ 
		tlMarineRepelLand,
		ARRAYSIZE ( tlMarineRepelLand ), 
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

//=========================================================
// Find medic. Grunt stops moving and calls the nearest medic,
// if none is around, we don't do much. I don't think I have much
// to put in here, other than to make the grunt stop moving, and
// run the medic calling task, I guess.
//=========================================================
Task_t	tlMarineFindMedic[] =
{
	{ TASK_STOP_MOVING,					(float)0	},
	{ TASK_MARINE_FIND_MEDIC,		(float)0	},
};

Schedule_t	slMarineFindMedic[] =
{
	{ 
		tlMarineFindMedic,
		ARRAYSIZE ( tlMarineFindMedic ), 
		bits_COND_NEW_ENEMY			|
		bits_COND_SEE_FEAR			|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_HEAR_SOUND		|
		bits_COND_PROVOKED,
		bits_SOUND_DANGER,

		"Marine Find Medic"
	},
};

//=========================================================
// heal - heal the player or grunt
// 
//=========================================================
Task_t	tlMarineHeal[] =
{
	{ TASK_MOVE_TO_TARGET_RANGE,			(float)50		},	// Move within 60 of target ent (client)
	{ TASK_SET_FAIL_SCHEDULE,				(float)SCHED_TARGET_CHASE },	// If you fail, catch up with that guy! (change this to put syringe away and then chase)
	{ TASK_FACE_IDEAL,						(float)0		},
	{ TASK_MARINE_SAY_HEAL,					(float)0		},
	{ TASK_PLAY_SEQUENCE_FACE_TARGET,		(float)ACT_ARM	},			// Whip out the needle
	{ TASK_MARINE_HEAL,						(float)0		},	// Put it in the player
	{ TASK_PLAY_SEQUENCE_FACE_TARGET,		(float)ACT_DISARM	},			// Put away the needle
	{ TASK_MARINE_STOP_FOLLOWING,			(float)0		},	// Stop following if it's not a player
};

Schedule_t	slMarineHeal[] =
{
	{
		tlMarineHeal,
		ARRAYSIZE ( tlMarineHeal ),
		0, 
		0,
		"Heal"
	},
};

//=========================================================
// Victory dance!
//=========================================================
Task_t	tlMarineHealChase[] =
{
	{ TASK_SET_FAIL_SCHEDULE,				(float)SCHED_FAIL			},
	{ TASK_STOP_MOVING,						(float)0					},
	{ TASK_FACE_TARGET,						(float)0					},
	{ TASK_GET_PATH_TO_TARGET,				(float)0					},
	{ TASK_MOVE_TO_TARGET_RANGE,			(float)96					},	// Move within 128 of target ent (client)
	{ TASK_WAIT_FOR_MOVEMENT,				(float)0					},
	{ TASK_FACE_TARGET,						(float)0					},
	{ TASK_SET_SCHEDULE,					(float)SCHED_MARINE_HEAL		},
};

Schedule_t	slMarineHealChase[] =
{
	{ 
		tlMarineHealChase,
		ARRAYSIZE ( tlMarineHealChase ), 
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE,
		0,
		"Marine Heal Chase"
	},
};

DEFINE_CUSTOM_SCHEDULES( CMarine )
{
	slMarineFollow,
	slMarineFaceTarget,
	slMarineIdleStand,
	slMarineFail,
	slMarineCombatFail,
	slMarineVictoryDance,
	slMarineEstablishLineOfFire,
	slMarineFoundEnemy,
	slMarineCombatFace,
	slMarineSignalSuppress,
	slMarineSuppress,
	slMarineWaitInCover,
	slMarineTakeCover,
	slMarineGrenadeCover,
	slMarineTossGrenadeCover,
	slMarineTakeCoverFromBestSound,
	slMarineHideReload,
	slMarineSweep,
	slMarineRangeAttack1A,
	slMarineRangeAttack1B,
	slMarineRangeAttack2,
	slMarineRepel,
	slMarineRepelAttack,
	slMarineRepelLand,
	slMarineFindMedic,
	slMarineHeal,
	slMarineHealChase,
};


IMPLEMENT_CUSTOM_SCHEDULES( CMarine, CRCAllyMonster );

void CMarine :: StartTask( Task_t *pTask )
{
	m_iTaskStatus = TASKSTATUS_RUNNING;

	switch ( pTask->iTask )
	{
	case TASK_MARINE_SAY_HEAL:
		PlaySentence( "MG_HEAL", 2, VOL_NORM, ATTN_IDLE );
		TaskComplete();
		break;

	case TASK_MARINE_CHECK_FIRE:
		if ( !NoFriendlyFire() )
		{
			SetConditions( bits_COND_MARINE_NOFIRE );
		}
		TaskComplete();
		break;

	case TASK_MARINE_FIND_MEDIC:
			// First try looking for a medic in my squad
			if ( InSquad() )
			{
				CRCAllyMonster *pSquadLeader = (CRCAllyMonster *)MySquadLeader( );
				if ( pSquadLeader )
				{
					for (int i = 0; i < MAXRC_SQUAD_MEMBERS; i++)
					{
						CRCAllyMonster *pMember = pSquadLeader->MySquadMember(i);
						if ( pMember && pMember != this )
						{
							CRCAllyMonster *pMedic = pMember->MyTalkSquadMonsterPointer();
							if ( pMedic && pMedic->pev->deadflag == DEAD_NO && pMedic->IsMedic() )
							{
								if ( !pMedic->IsFollowing() && pMedic->CanHeal(this) ) 
								{
									ALERT( at_console, "I've found my medic!\n" );
									EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "fgrunt/medic.wav", 1, ATTN_NORM, 0, GetVoicePitch());
									pMedic->GruntHealerCall( this );
									TaskComplete();
									break;
								}
							}
						}
					}
				}
			}
			// If not, search bsp.
			if ( !TaskIsComplete() ) 
			{
				CBaseEntity *pFriend = NULL;
				int i;

				// for each friend in this bsp...
				for ( i = 0; i < TLK_CFRIENDS; i++ )
				{
					while (pFriend = EnumFriends( pFriend, i, TRUE ))
					{
						CRCAllyMonster *pMedic = pFriend->MyTalkSquadMonsterPointer();
						if ( pMedic && pMedic->pev->deadflag == DEAD_NO && pMedic->IsMedic())
						{
							if ( !pMedic->IsFollowing() && pMedic->CanHeal(this) ) 
							{
								EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "fgrunt/medic.wav", 1, ATTN_NORM, 0, GetVoicePitch());
								pMedic->GruntHealerCall( this );
								TaskComplete();
								break;
							}
						}
					}
				}
			}
			if ( !TaskIsComplete() ) 
			{
				TaskFail();
			}
			m_flMarineWaitTime = MARINE_MEDIC_WAIT + gpGlobals->time; // Call again in ten seconds anyway.
		break;

	case TASK_MARINE_HEAL:
		m_IdealActivity = ACT_MELEE_ATTACK2;
		Heal();
		break;

	case TASK_MARINE_STOP_FOLLOWING:
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

	case TASK_MARINE_FACE_TOSS_DIR:
		break;

	default: 
		CRCAllyMonster :: StartTask( pTask );
		break;
	}
}

void CMarine :: RunTask( Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_MARINE_HEAL:
		if ( m_fSequenceFinished )
		{
			TaskComplete();
		}
		else
		{
			if ( TargetDistance() > 90 )
				TaskComplete();
			pev->ideal_yaw = UTIL_VecToYaw( m_hTargetEnt->pev->origin - pev->origin );
			ChangeYaw( pev->yaw_speed );
		}
	break;
	case TASK_MARINE_FACE_TOSS_DIR:
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
void CMarine :: GibMonster ( void )
{
	Vector	vecGunPos;
	Vector	vecGunAngles;

	if ( GetBodygroup( 3 ) != 3 )
	{// throw a gun if the grunt has one
		GetAttachment( 0, vecGunPos, vecGunAngles );
		
		CBaseEntity *pGun;
		if (FBitSet( pev->weapons, MARINE_SHOTGUN ))
		{
			pGun = DropItem( "weapon_shotgun", vecGunPos, vecGunAngles );
		}
		else if (FBitSet( pev->weapons, MARINE_SIG552 ))
		{
			pGun = DropItem( "weapon_sig552", vecGunPos, vecGunAngles );
		}
		else
		{
			pGun = DropItem( "weapon_m249", vecGunPos, vecGunAngles );
		}
		if ( pGun )
		{
			pGun->pev->velocity = Vector (RANDOM_FLOAT(-100,100), RANDOM_FLOAT(-100,100), RANDOM_FLOAT(200,300));
			pGun->pev->avelocity = Vector ( 0, RANDOM_FLOAT( 200, 400 ), 0 );
		}
	
		if (FBitSet( pev->weapons, MARINE_GRENADELAUNCHER ))
		{
			pGun = DropItem( "ammo_ARgrenades", vecGunPos, vecGunAngles );
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
int CMarine :: ISoundMask ( void) 
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
void CMarine :: CheckAmmo ( void )
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
int	CMarine :: Classify ( void )
{
	return	CLASS_PLAYER_ALLY;
}
//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CMarine :: SetYawSpeed ( void )
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
void CMarine :: PrescheduleThink ( void )
{
	if (m_pBeam)
	{
		UpdateGas();
	}

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
BOOL CMarine :: FCanCheckAttacks ( void )
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
BOOL CMarine :: CheckMeleeAttack1 ( float flDot, float flDist )
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
BOOL CMarine :: CheckRangeAttack1 ( float flDot, float flDist )
{
	if(!FBitSet(pev->weapons, MARINE_SIG552 ) && !FBitSet(pev->weapons, MARINE_SHOTGUN) && !FBitSet(pev->weapons, MARINE_M249))
		return FALSE;

	if ( !HasConditions( bits_COND_ENEMY_OCCLUDED ) && flDist <= 2048 && flDot >= 0.5 && !m_fOneHanded )
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
BOOL CMarine :: CheckRangeAttack2 ( float flDot, float flDist )
{
	if(!pev->weapons)
		return false;

	if (!FBitSet(pev->weapons, (MARINE_HANDGRENADE | MARINE_GRENADELAUNCHER)) )
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

	if (FBitSet( pev->weapons, MARINE_HANDGRENADE))
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
			vecTarget = vecTarget + ((vecTarget - pev->origin).Length() / gSkillData.marineGrenadeSpeed) * m_hEnemy->pev->velocity;
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

		
	if (FBitSet( pev->weapons, MARINE_HANDGRENADE))
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
		Vector vecToss = VecCheckThrow( pev, GetGunPosition(), vecTarget, gSkillData.marineGrenadeSpeed, 0.5 );

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
CBaseEntity *CMarine :: Kick( void )
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

Vector CMarine :: GetGunPosition( int stance )
{
	if ( (m_fStanding || stance == STANCE_STANDING) && stance != STANCE_CROUCHING )
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
void CMarine :: Shoot ( void )
{
	if (m_hEnemy == NULL )
	{
		return;
	}

	Vector vecShootOrigin = GetGunPosition();
	Vector vecShootDir = ShootAtEnemy( vecShootOrigin );

	UTIL_MakeVectors ( pev->angles );

	EMIT_SOUND( ENT(pev), CHAN_WEAPON, "weapons/sig552_fire1.wav", 1, 0.5 );

	Vector vecShellOrigin = vecShootOrigin + gpGlobals->v_forward * 16;
	Vector vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT(40,90) + gpGlobals->v_up * RANDOM_FLOAT(75,200) + gpGlobals->v_forward * RANDOM_FLOAT(-40, 40);
	PLAYBACK_EVENT_FULL( 0, this->edict(), m_usEjectBrass, 0.0, vecShellOrigin, vecShellVelocity, pev->angles.y, 0, 1, TE_BOUNCE_SHELL, 0, 0 );
	FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_4DEGREES, 2048, BULLET_MONSTER_SIG552 ); // shoot +-5 degrees

	pev->effects |= EF_MUZZLEFLASH;

	m_cAmmoLoaded--;// take away a bullet!

	Vector angDir = UTIL_VecToAngles( vecShootDir );
	SetBlending( 0, angDir.x );
}

//=========================================================
// Shoot
//=========================================================
void CMarine :: Shotgun ( void )
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
	PLAYBACK_EVENT_FULL( 0, this->edict(), m_usEjectBrass, 0.0, vecShellOrigin, vecShellVelocity, pev->angles.y, 0, 4/*body*/, TE_BOUNCE_SHOTSHELL, 0, 0 );
	FireBullets(gSkillData.marineShotgunPellets, vecShootOrigin, vecShootDir, VECTOR_CONE_9DEGREES, 2048, BULLET_MONSTER_12MM, 0 ); // shoot +-7.5 degrees

	pev->effects |= EF_MUZZLEFLASH;

	m_cAmmoLoaded--;// take away a bullet!

	Vector angDir = UTIL_VecToAngles( vecShootDir );
	SetBlending( 0, angDir.x );
}
//=========================================================
// Shoot
//=========================================================
void CMarine :: M249 ( void )
{
	if (m_hEnemy == NULL )
	{
		return;
	}

	EMIT_SOUND( ENT(pev), CHAN_WEAPON, "weapons/m249_fire1.wav", 1, 0.5 );

	Vector vecShootOrigin = GetGunPosition();
	Vector vecShootDir = ShootAtEnemy( vecShootOrigin );

	UTIL_MakeVectors ( pev->angles );

	Vector vecShellOrigin = vecShootOrigin + gpGlobals->v_forward * 16;
	Vector vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT(40,90) + gpGlobals->v_up * RANDOM_FLOAT(75,200) + gpGlobals->v_forward * RANDOM_FLOAT(-40, 40);
		
	m_flLinkToggle = !m_flLinkToggle;
	PLAYBACK_EVENT_FULL( 0, this->edict(), m_usEjectBrass, 0.0, vecShellOrigin, vecShellVelocity, pev->angles.y, 0, m_flLinkToggle == 1 ? 5 : 1 , TE_BOUNCE_SHELL, 0, 0 );
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
void CMarine :: HandleAnimEvent( MonsterEvent_t *pEvent )
{
	Vector	vecShootDir;
	Vector	vecShootOrigin;

	switch( pEvent->event )
	{
		case MARINE_AE_DROP_GUN:
			{
			Vector	vecGunPos;
			Vector	vecGunAngles;

			GetAttachment( 0, vecGunPos, vecGunAngles );

			// switch to body group with no gun.
			SetBodygroup( MN_GUN_GROUP, MN_GUN_NONE );

			// now spawn a gun.
			if (FBitSet( pev->weapons, MARINE_SHOTGUN ))
			{
				 DropItem( "weapon_shotgun", vecGunPos, vecGunAngles );
			}
			else if (FBitSet( pev->weapons, MARINE_SIG552 ))
			{
				 DropItem( "weapon_sig552", vecGunPos, vecGunAngles );
			}
			else
			{
				 DropItem( "weapon_m249", vecGunPos, vecGunAngles );
			}
			if (FBitSet( pev->weapons, MARINE_GRENADELAUNCHER ))
			{
				DropItem( "ammo_ARgrenades", BodyTarget( pev->origin ), vecGunAngles );
			}

			}
			break;

		case MARINE_AE_SHOWGUN:
			if (!pev->weapons)
			{
				pev->weapons = MARINE_SIG552;
			}

			if ( pev->weapons & MARINE_SIG552 )
			{
				SetBodygroup( MN_GUN_GROUP, MN_GUN_MP5 );
			}
			else if (pev->weapons & MARINE_SHOTGUN )
			{
				SetBodygroup( MN_GUN_GROUP, MN_GUN_SHOTGUN );
			}
			else
			{
				SetBodygroup( MN_GUN_GROUP, MN_GUN_SAW );
			}
			m_fOneHanded = FALSE;
			break;

		case MARINE_AE_HIDEGUN:
			SetBodygroup( MN_GUN_GROUP, MN_GUN_NONE );
			m_fOneHanded = TRUE;
			m_bNeedleOut = TRUE;
			break;

		case MARINE_AE_HIDENEEDLE:
			SetBodygroup( MN_GUN_GROUP, MN_GUN_NONE );
			m_bNeedleOut = FALSE;
			break;

		case MARINE_AE_SHOWNEEDLE:
			SetBodygroup( MN_GUN_GROUP, MN_GUN_NEEDLE );
			break;

		case MARINE_AE_SHOWTORCH:
			SetBodygroup(MN_GUN_GROUP, MN_GUN_TORCH);
			break;

		case MARINE_AE_HIDETORCH:
			SetBodygroup(MN_GUN_GROUP, MN_GUN_NONE);
			break;

		case MARINE_AE_ONGAS:
			MakeGas ();
			UpdateGas ();
			break;

		case MARINE_AE_OFFGAS:
			KillGas ();
			break;

		case MARINE_AE_RELOAD:
			m_cAmmoLoaded = m_cClipSize;
			ClearConditions(bits_COND_NO_AMMO_LOADED);
			break;

		case MARINE_AE_GREN_TOSS:
		{
			UTIL_MakeVectors( pev->angles );
			// CGrenade::ShootTimed( pev, pev->origin + gpGlobals->v_forward * 34 + Vector (0, 0, 32), m_vecTossVelocity, 3.5 );
			CGrenade::ShootTimed( pev, GetGunPosition(), m_vecTossVelocity, 3.5 );

			m_fThrowGrenade = FALSE;
			m_flNextGrenadeCheck = gpGlobals->time + 6;// wait six seconds before even looking again to see if a grenade can be thrown.
			// !!!LATER - when in a group, only try to throw grenade if ordered.
		}
		break;

		case MARINE_AE_GREN_LAUNCH:
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

		case MARINE_AE_GREN_DROP:
		{
			UTIL_MakeVectors( pev->angles );
			CGrenade::ShootTimed( pev, pev->origin + gpGlobals->v_forward * 17 - gpGlobals->v_right * 27 + gpGlobals->v_up * 6, g_vecZero, 3 );
		}
		break;

		case MARINE_AE_BURST1:
		{
			if ( FBitSet( pev->weapons, MARINE_SIG552 ))
			{
				Shoot();
			}
			else if ( FBitSet( pev->weapons, MARINE_SHOTGUN ))
			{
				Shotgun( );

				EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/spas12_fire_single.wav", 1, 0.5 );
			}
			else
			{
				M249( );
			}
		
			CSoundEnt::InsertSound ( bits_SOUND_COMBAT, pev->origin, 384, 0.3 );
		}
		break;

		case MARINE_AE_BURST2:
		case MARINE_AE_BURST3:
			if ( FBitSet( pev->weapons, MARINE_SIG552 ))
				Shoot();
			else
				M249();
			break;

		case MARINE_AE_KICK:
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
				pHurt->TakeDamage( pev, pev, gSkillData.marineDmgKick, DMG_CLUB );
			}
		}
		break;

		case MARINE_AE_CAUGHT_ENEMY:
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
void CMarine :: Spawn()
{
	Precache( );

	SET_MODEL(ENT(pev), "models/marine.mdl");
	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_RED;
	pev->health			= gSkillData.marineHealth;
	pev->view_ofs		= Vector ( 0, 0, 50 );// position of the eyes relative to monster's origin.
	m_flFieldOfView		= VIEW_FIELD_WIDE; // NOTE: we need a wide field of view so npc will notice player and say hello
	m_MonsterState		= MONSTERSTATE_NONE;
	m_flNextGrenadeCheck = gpGlobals->time + 1;
	m_flNextPainTime	= gpGlobals->time;

	m_afCapability		= bits_CAP_HEAR | bits_CAP_SQUAD | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP | bits_CAP_GRABBABLE | bits_CAP_INFESTABLE;

	m_fEnemyEluded		= FALSE;
	m_fFirstEncounter	= TRUE;// this is true when the grunt spawns, because he hasn't encountered an enemy yet.

	m_flHealAnount		= gSkillData.marineHeal;

	m_HackedGunPos = Vector ( 0, 0, 55 );

	if(FClassnameIs(this->edict(), "monster_marine_medic"))
		m_iType = MARINE_TYPE_MEDIC;
	else if(FClassnameIs(this->edict(), "monster_marine_engineer"))
		m_iType = MARINE_TYPE_ENGINEER;
	else
		m_iType = MARINE_TYPE_NORMAL;

	if ( m_iHead == -1 )
		m_iHead = 0;

	if (FBitSet( pev->weapons, MARINE_SHOTGUN ))
	{
		SetBodygroup( MN_GUN_GROUP, MN_GUN_SHOTGUN );
		m_cClipSize		= 8;
	}
	if (FBitSet( pev->weapons, MARINE_SIG552 ))
	{
		SetBodygroup( MN_GUN_GROUP, MN_GUN_MP5 );
		m_cClipSize	= MARINE_CLIP_SIZE;
	}
	if (FBitSet( pev->weapons, MARINE_M249 ))
	{
		SetBodygroup( MN_GUN_GROUP, MN_GUN_SAW );
		m_cClipSize	= MARINE_CLIP_SIZE;
	}
	if (pev->weapons == 0)
	{
		SetBodygroup( MN_GUN_GROUP, MN_GUN_NONE );
		m_cClipSize	= MARINE_CLIP_SIZE;
		m_fOneHanded = TRUE;
	}
	
	if(m_iType == MARINE_TYPE_MEDIC)
	{
		SetBodygroup(MN_HEAD_GROUP, MN_HEAD_MEDIC);
	}
	else if(m_iType == MARINE_TYPE_ENGINEER)
	{
		SetBodygroup(MN_HEAD_GROUP, MN_HEAD_TORCH);
	}
	else
	{
		if ( m_iHead == 0 )
		{
			SetBodygroup( MN_HEAD_GROUP, MN_HEAD_MASK );
		}
		if ( m_iHead == 1 )
		{
			SetBodygroup( MN_HEAD_GROUP, MN_HEAD_MASK );
		}
		if ( m_iHead == 2 )
		{
			SetBodygroup( MN_HEAD_GROUP, MN_HEAD_SHOTGUN );
		}
		if ( m_iHead == 3 )
		{
			SetBodygroup( MN_HEAD_GROUP, MN_HEAD_SAW );
		}
		if ( m_iHead == 4 )
		{
			SetBodygroup( MN_HEAD_GROUP, MN_HEAD_SAW_BLACK );
		}
		if ( m_iHead == 5 )
		{
			SetBodygroup( MN_HEAD_GROUP, MN_HEAD_SAW );
		}
		if ( m_iHead == 6 )
		{
			SetBodygroup( MN_HEAD_GROUP, MN_HEAD_MAJOR );
		}
		if ( m_iHead == 7 )
		{
			SetBodygroup( MN_HEAD_GROUP, MN_HEAD_BERET_BLACK );
		}
	}

	m_cAmmoLoaded		= m_cClipSize;

	MonsterInit();
	SetUse( &CRCAllyMonster::FollowerUse );
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CMarine :: Precache()
{
	m_usEjectBrass = PRECACHE_EVENT( 1, "events/ejectbullet.sc" );

	PRECACHE_MODEL("models/marine.mdl");

	PRECACHE_SOUND("weapons/m249_fire1.wav" );
	PRECACHE_SOUND("weapons/sig552_fire1.wav" );

	PRECACHE_SOUND("fgrunt/gr_pain1.wav");
	PRECACHE_SOUND("fgrunt/gr_pain2.wav");
	PRECACHE_SOUND("fgrunt/gr_pain3.wav");
	PRECACHE_SOUND("fgrunt/gr_pain4.wav");
	PRECACHE_SOUND("fgrunt/gr_pain5.wav");
	PRECACHE_SOUND("fgrunt/gr_pain6.wav");

	PRECACHE_SOUND("fgrunt/death1.wav");
	PRECACHE_SOUND("fgrunt/death2.wav");
	PRECACHE_SOUND("fgrunt/death3.wav");
	PRECACHE_SOUND("fgrunt/death4.wav");
	PRECACHE_SOUND("fgrunt/death5.wav");
	PRECACHE_SOUND("fgrunt/death6.wav");
	
	PRECACHE_SOUND("fgrunt/medic.wav");

	PRECACHE_SOUND("weapons/spas12_fire_single.wav");
	PRECACHE_SOUND("common/hit_miss.wav");// because we use the basemonster SWIPE animation event

	TalkInit();
	CRCAllyMonster::Precache();
}	

// Init talk data
void CMarine :: TalkInit()
{
	// scientists speach group names (group names are in sentences.txt)

	m_szGrp[TLK_ANSWER]  =	"FG_ANSWER";
	m_szGrp[TLK_QUESTION] =	"FG_QUESTION";
	m_szGrp[TLK_IDLE] =		"FG_IDLE";
	m_szGrp[TLK_STARE] =		"FG_STARE";
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

	if (m_iType == MARINE_TYPE_MEDIC)
	{
		m_voicePitch = 105;
	}
	else if (m_iType == MARINE_TYPE_ENGINEER)
	{
		m_voicePitch = 93;
	}
	else
	{
		if ( m_iHead == 0 )m_voicePitch = 100;
		if ( m_iHead == 1 )m_voicePitch = 100;
		if ( m_iHead == 2 )m_voicePitch = 90;
		if ( m_iHead == 3 )m_voicePitch = 100;
		if ( m_iHead == 4 )m_voicePitch = 90;
		if ( m_iHead == 5 )m_voicePitch = 100;
		if ( m_iHead == 6 )m_voicePitch = 100;
		if ( m_iHead == 7 )m_voicePitch = 90;
	}
}
	
//=========================================================
// PainSound
//=========================================================
void CMarine :: PainSound ( void )
{
	if ( gpGlobals->time > m_flNextPainTime )
	{
		switch ( RANDOM_LONG(0,5) )
		{
			case 0: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "fgrunt/gr_pain1.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
			case 1: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "fgrunt/gr_pain2.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
			case 2: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "fgrunt/gr_pain3.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
			case 3: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "fgrunt/gr_pain4.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
			case 4: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "fgrunt/gr_pain5.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
			case 5: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "fgrunt/gr_pain6.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
		}
		m_flNextPainTime = gpGlobals->time + 1;
	}
}

//=========================================================
// DeathSound 
//=========================================================
void CMarine :: DeathSound ( void )
{
	switch (RANDOM_LONG(0,5))
	{
	case 0: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "fgrunt/death1.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
	case 1: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "fgrunt/death2.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
	case 2: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "fgrunt/death3.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
	case 3: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "fgrunt/death4.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
	case 4: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "fgrunt/death5.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
	case 5: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "fgrunt/death6.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
	}
}
//=========================================================
// TraceAttack - make sure we're not taking it in the helmet
//=========================================================
void CMarine :: TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType)
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
			UTIL_StudioDecal(ptr->vecPlaneNormal, ptr->vecEndPos, "shot_metal", entindex());
			UTIL_Ricochet( ptr->vecEndPos, ptr->vecPlaneNormal );
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
int CMarine :: TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
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

	if( m_bImmortal )
	{
		flDamage = flDamage*0.1;
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
			if ( (m_afMemory & bits_MEMORY_SUSPICIOUS) || IsFacing( pevAttacker, pev->origin ) )
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

Schedule_t* CMarine :: GetScheduleOfType ( int Type )
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
				return slMarineFaceTarget;	// override this for different target face behavior
			else
				return psched;
		}
		break;
	case SCHED_TARGET_CHASE:
		{
			return slMarineFollow;
		}
		break;
	case SCHED_IDLE_STAND:
		{
			psched = CRCAllyMonster::GetScheduleOfType(Type);

			if (psched == slIdleStand)
			{
				// just look straight ahead.
				return slMarineIdleStand;
			}
			else
				return psched;	
		}
		break;
	case SCHED_TAKE_COVER_FROM_ENEMY:
		{
			return &slMarineTakeCover[ 0 ];
		}
		break;
	case SCHED_TAKE_COVER_FROM_BEST_SOUND:
		{
			return &slMarineTakeCoverFromBestSound[ 0 ];
		}
		break;
	case SCHED_MARINE_FIND_MEDIC:
		{
			return &slMarineFindMedic[ 0 ];
		}
		break;
	case SCHED_MARINE_TAKECOVER_FAILED:
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
	case SCHED_MARINE_ELOF_FAIL:
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
	case SCHED_MARINE_ESTABLISH_LINE_OF_FIRE:
		{
			return &slMarineEstablishLineOfFire[ 0 ];
		}
		break;
	case SCHED_RANGE_ATTACK1:
		{
			// randomly stand or crouch
			if (m_fStanding)
				return &slMarineRangeAttack1B[ 0 ];
			else
				return &slMarineRangeAttack1A[ 0 ];
		}
		break;
	case SCHED_RANGE_ATTACK2:
		{
			return &slMarineRangeAttack2[ 0 ];
		}
		break;
	case SCHED_COMBAT_FACE:
		{
			return &slMarineCombatFace[ 0 ];
		}
		break;
	case SCHED_MARINE_WAIT_FACE_ENEMY:
		{
			return &slMarineWaitInCover[ 0 ];
		}
	case SCHED_MARINE_SWEEP:
		{
			return &slMarineSweep[ 0 ];
		}
		break;
	case SCHED_MARINE_COVER_AND_RELOAD:
		{
			return &slMarineHideReload[ 0 ];
		}
		break;
	case SCHED_MARINE_FOUND_ENEMY:
		{
			return &slMarineFoundEnemy[ 0 ];
		}
		break;
	case SCHED_VICTORY_DANCE:
		{
			if ( InSquad() )
			{
				if ( !IsLeader() )
				{
					return &slMarineFail[ 0 ];
				}
			}
			if ( IsFollowing() )
			{
				return &slMarineFail[ 0 ];
			}

			return &slMarineVictoryDance[ 0 ];
		}
		break;
	case SCHED_MARINE_SUPPRESS:
		{
			if ( m_fFirstEncounter )
			{
				m_fFirstEncounter = FALSE;// after first encounter, leader won't issue handsigns anymore when he has a new enemy
				return &slMarineSignalSuppress[ 0 ];
			}
			else
			{
				return &slMarineSuppress[ 0 ];
			}
		}
		break;
	case SCHED_FAIL:
		{
			if ( m_hEnemy != NULL )
			{
				// grunt has an enemy, so pick a different default fail schedule most likely to help recover.
				return &slMarineCombatFail[ 0 ];
			}

			return &slMarineFail[ 0 ];
		}
		break;
	case SCHED_MARINE_REPEL:
		{
			if (pev->velocity.z > -128)
				pev->velocity.z -= 32;
			return &slMarineRepel[ 0 ];
		}
		break;
	case SCHED_MARINE_REPEL_ATTACK:
		{
			if (pev->velocity.z > -128)
				pev->velocity.z -= 32;
			return &slMarineRepelAttack[ 0 ];
		}
		break;
	case SCHED_MARINE_REPEL_LAND:
		{
			return &slMarineRepelLand[ 0 ];
		}
		break;
	case SCHED_MARINE_HEAL_CHASE:
		{
			return &slMarineHealChase[ 0 ];
		}
		break;
	case SCHED_MARINE_HEAL:
		{
			return &slMarineHeal[ 0 ];
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
void CMarine :: SetActivity ( Activity NewActivity )
{
	int	iSequence = ACTIVITY_NOT_AVAILABLE;
	int	iGaitSequence = ACTIVITY_NOT_AVAILABLE;
	void *pmodel = GET_MODEL_PTR( ENT(pev) );

	switch ( NewActivity)
	{
	case ACT_RANGE_ATTACK1:
		// grunt is either shooting standing or shooting crouched
		if (FBitSet( pev->weapons, MARINE_SIG552))
		{
			if ( m_fStanding )
			{
				// get aimable sequence
				iSequence = LookupSequence( "standing_sig552" );
			}
			else
			{
				// get crouching shoot
				iSequence = LookupSequence( "crouching_sig552" );
			}
		}
		else if (FBitSet( pev->weapons, MARINE_SHOTGUN))
		{
			// get aimable sequence
			iSequence = LookupSequence( "standing_shotgun" );
		}
		else
		{
			if ( m_fStanding )
			{
				// get aimable sequence
				iSequence = LookupSequence( "standing_m249" );
			}
			else
			{
				// get crouching shoot
				iSequence = LookupSequence( "crouching_m249" );
			}
		}
		break;
	case ACT_RELOAD:
		// grunt is either shooting standing or shooting crouched
		if (FBitSet( pev->weapons, MARINE_SIG552))
		{
			iSequence = LookupSequence( "reload_sig552" );
		}
		else if (FBitSet( pev->weapons, MARINE_SHOTGUN))
		{
			iSequence = LookupSequence( "reload_shotgun" );
		}
		else
		{
			iSequence = LookupSequence( "reload_m249" );
		}
		break;
	case ACT_RANGE_ATTACK2:
		// get toss anim
		iSequence = LookupSequence( "throwgrenade" );
		break;
	case ACT_RUN:
		if(m_fOneHanded)
		{
			iSequence = LookupSequence("run_nogun");
		}
		else
		{
			if ( pev->health <= MARINE_LIMP_HEALTH )
			{
				// limp!
				iSequence = LookupActivity ( ACT_RUN_HURT );
			}
			else
			{
				iSequence = LookupActivity ( NewActivity );
			}
		}
		break;
	case ACT_WALK:
		if(m_fOneHanded)
		{
			iSequence = LookupSequence("walk_nogun");
		}
		else
		{
			if ( pev->health <= MARINE_LIMP_HEALTH )
			{
				// limp!
				iSequence = LookupActivity ( ACT_WALK_HURT );
			}
			else
			{
				iSequence = LookupActivity ( NewActivity );
			}
		}
		break;
	case ACT_IDLE:
		if(m_fOneHanded)
		{
			iSequence = LookupSequence("idle_nogun");
		}
		else
		{
			if ( m_MonsterState == MONSTERSTATE_COMBAT )
			{
				NewActivity = ACT_IDLE_ANGRY;
			}
			iSequence = LookupActivity ( NewActivity );
		}
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
Schedule_t *CMarine :: GetSchedule ( void )
{
	if(m_pGrabber)
	{
		CBaseMonster::GetSchedule();
	}\

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
			return GetScheduleOfType ( SCHED_MARINE_REPEL_LAND );
		}
		else
		{
			// repel down a rope, 
			if ( m_MonsterState == MONSTERSTATE_COMBAT )
				return GetScheduleOfType ( SCHED_MARINE_REPEL_ATTACK );
			else
				return GetScheduleOfType ( SCHED_MARINE_REPEL );
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

			// Behavior for following the player
			if(IsMedic())
			{
				if ( IsFollowing() && m_hTargetEnt )
				{
					if ( !m_hTargetEnt->IsAlive() )
					{
						// UNDONE: Comment about the recently dead player here?
						StopFollowing( FALSE );
						break;
					}
					
					if(!m_hTargetEnt->IsPlayer())
					{
						return GetScheduleOfType( SCHED_MARINE_HEAL_CHASE );
					}
					else
					{
						// If I'm already close enough to my target
						if ( TargetDistance() <= 128 )
						{
							if ( CanHeal(m_hTargetEnt) )	// Heal opportunistically
								return GetScheduleOfType(SCHED_MARINE_HEAL);
						}
					}
				}
			}

			if ( pev->health < pev->max_health && ( m_flMarineWaitTime < gpGlobals->time ))
			{
				// Find a medic 
				return GetScheduleOfType( SCHED_MARINE_FIND_MEDIC );
			}

// new enemy
			if ( HasConditions(bits_COND_NEW_ENEMY) )
			{
				if ( InSquad() )
				{
					((CRCAllyMonster *)MySquadLeader())->m_fEnemyEluded = FALSE;
					if ( FClassnameIs(m_hEnemy->pev, "monster_tenlightened") && !IsFacing(m_hEnemy->pev, pev->origin))
					{
						if(HasConditions(bits_COND_SEE_ENEMY))
						{
							return GetScheduleOfType ( SCHED_TAKE_COVER_FROM_ENEMY );
						}
						else
						{
							return GetScheduleOfType ( SCHED_STANDOFF );
						}
					}
					else if(!FBitSet(pev->weapons, MARINE_SIG552 ) && !FBitSet(pev->weapons, MARINE_SHOTGUN) && !FBitSet(pev->weapons, MARINE_M249))
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
								return GetScheduleOfType ( SCHED_MARINE_SUPPRESS );
							}
							else
							{
								return GetScheduleOfType ( SCHED_MARINE_ESTABLISH_LINE_OF_FIRE );
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
								return GetScheduleOfType ( SCHED_MARINE_SUPPRESS );
							}
							else
							{
								return GetScheduleOfType ( SCHED_MARINE_ESTABLISH_LINE_OF_FIRE );
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
				return GetScheduleOfType ( SCHED_MARINE_COVER_AND_RELOAD );
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
						m_iSentence = MARINE_SENT_COVER;
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
			if ( FClassnameIs(m_hEnemy->pev, "monster_tenlightened") && !IsFacing(m_hEnemy->pev, pev->origin))
			{
				if(HasConditions(bits_COND_SEE_ENEMY))
				{
					return GetScheduleOfType ( SCHED_TAKE_COVER_FROM_ENEMY );
				}
				else
				{
					return GetScheduleOfType ( SCHED_STANDOFF );
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
						return GetScheduleOfType ( SCHED_MARINE_FOUND_ENEMY );
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
				if ( FClassnameIs(m_hEnemy->pev, "monster_tenlightened") && !IsFacing(m_hEnemy->pev, pev->origin))
				{
					return GetScheduleOfType ( SCHED_STANDOFF );
				}
				else if ( HasConditions( bits_COND_CAN_RANGE_ATTACK2 ) && OccupySlot( bits_SLOTS_FGRUNT_GRENADE ) )
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
					return GetScheduleOfType( SCHED_MARINE_ESTABLISH_LINE_OF_FIRE );
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
				return GetScheduleOfType ( SCHED_MARINE_ESTABLISH_LINE_OF_FIRE );
			}
		}
		break;
	case MONSTERSTATE_ALERT:	
	case MONSTERSTATE_IDLE:
		if ( HasConditions ( bits_COND_NO_AMMO_LOADED ) )
		{
			return GetScheduleOfType ( SCHED_RELOAD );
		}
		if ( pev->health < pev->max_health && ( m_flMarineWaitTime < gpGlobals->time ))
		{
			// Find a medic 
			return GetScheduleOfType( SCHED_MARINE_FIND_MEDIC ); // Unresolved
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

			if(!m_hTargetEnt->IsPlayer())
			{
				return GetScheduleOfType( SCHED_MARINE_HEAL_CHASE );
			}
			else if(!HasConditions(bits_COND_BLOCKING_PATH))
			{
				// If I'm already close enough to my target
				if ( TargetDistance() <= 128 )
				{
					if (CanHeal(m_hTargetEnt))	// Heal opportunistically
						return GetScheduleOfType( SCHED_MARINE_HEAL );
					//if (HasConditions( bits_COND_CLIENT_PUSH ))	// Player wants me to move
						//return GetScheduleOfType( SCHED_MOVE_AWAY_FOLLOW );
				}
				return GetScheduleOfType( SCHED_TARGET_FACE );	// Just face and follow.
			}
		}

		//if ( HasConditions( bits_COND_CLIENT_PUSH ) )
		//{
		//	return GetScheduleOfType( SCHED_MOVE_AWAY );
		//}

		// try to say something about smells
		TrySmellTalk();
		break;
	}
	
	return CRCAllyMonster :: GetSchedule();
}
MONSTERSTATE CMarine :: GetIdealState ( void )
{
	return CRCAllyMonster::GetIdealState();
}
void CMarine::DeclineFollowing( void )
{
	if(!strcmp("major", STRING(pev->targetname)))
		PlaySentence( "DT01_SARGEB", 2, VOL_NORM, ATTN_NORM );
	else
		PlaySentence( "FG_STOP", 2, VOL_NORM, ATTN_NORM );
}
BOOL CMarine::IsMedic( void )
{ 
	return m_iType == MARINE_TYPE_MEDIC ? TRUE : FALSE;
}

BOOL CMarine::CanHeal( CBaseEntity *pHealTarget )
{ 
	if(m_MonsterState == MONSTERSTATE_SCRIPT)
		return FALSE;

	if(m_iType != MARINE_TYPE_MEDIC)
		return FALSE;

	ALERT(at_console, "Heal amount is %f\n", m_flHealAnount );
	if ( m_flHealAnount <= 0 )
	{
		if ( !m_fDepleteLine )
		{
			PlaySentence( "MG_NOTHEAL", 2, VOL_NORM, ATTN_IDLE );
			m_fDepleteLine = TRUE;
		}
		return FALSE;
	}

	if ( (m_healTime > gpGlobals->time) || (pHealTarget == NULL) || pHealTarget->pev->health == pHealTarget->pev->max_health )
	{
		return FALSE;
	}
	return TRUE;
}
void CMarine::Heal( void )
{
	if ( !CanHeal(m_hTargetEnt) )
		return;

	Vector target = m_hTargetEnt->pev->origin - pev->origin;
	float flHealAmount = m_hTargetEnt->pev->max_health - m_hTargetEnt->pev->health;

	if(flHealAmount > m_flHealAnount)
		flHealAmount = m_flHealAnount;

	m_flHealAnount -= flHealAmount;

	if ( target.Length() > 100 )
		return;

	m_hTargetEnt->pev->starttime = gpGlobals->time;
	m_hTargetEnt->TakeHealth( flHealAmount, DMG_GENERIC );
}

//=========================================================
// AUTOGENE 
//=========================================================
extern int gmsgCreateDLight;
void CMarine::UpdateGas( void )
{
	TraceResult tr;

	if ( !m_pBeam )
		return;

	Vector vStart;
	Vector vEnd;
	Vector vAng;

	GetAttachment(4, vStart, vAng);
	GetAttachment(3, vEnd, vAng);

	Vector vecDir = (vEnd - vStart).Normalize();
	vEnd = vStart + vecDir*64;

	UTIL_TraceLine( vStart, vEnd, dont_ignore_monsters, edict(), &tr );

	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_ELIGHT );
		WRITE_SHORT( entindex( ) + 0x2000 );		// entity, attachment
		WRITE_COORD( vStart.x );		// origin
		WRITE_COORD( vStart.y );
		WRITE_COORD( vStart.z );
		WRITE_COORD( 32 );	// radius
		WRITE_BYTE( 24 );	// R
		WRITE_BYTE( 121 );	// G
		WRITE_BYTE( 239 );	// B
		WRITE_BYTE( 2 );	// life * 10
		WRITE_COORD( 0 ); // decay
	MESSAGE_END();

	if ( tr.flFraction != 1.0 )
	{
		UTIL_Sparks(tr.vecEndPos);
		UTIL_CustomDecal ( &tr, "shot" );

		MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, tr.vecEndPos );
			WRITE_BYTE( TE_STREAK_SPLASH );
			WRITE_COORD( tr.vecEndPos.x );		// origin
			WRITE_COORD( tr.vecEndPos.y );
			WRITE_COORD( tr.vecEndPos.z );
			WRITE_COORD( tr.vecPlaneNormal.x );	// direction
			WRITE_COORD( tr.vecPlaneNormal.y );
			WRITE_COORD( tr.vecPlaneNormal.z );
			WRITE_BYTE( 10 );	// Streak color 6
			WRITE_SHORT( 60 );	// count
			WRITE_SHORT( 25 );
			WRITE_SHORT( 50 );	// Random velocity modifier
		MESSAGE_END();
		MESSAGE_BEGIN( MSG_ALL, gmsgCreateDLight, NULL );
			WRITE_COORD( vStart.x );		// origin
			WRITE_COORD( vStart.y );
			WRITE_COORD( vStart.z );
			WRITE_BYTE( RANDOM_LONG(2, 8) );	// radius
			WRITE_BYTE( 255 );	// R
			WRITE_BYTE( 32 );	// G
			WRITE_BYTE( 5 );	// B
			WRITE_BYTE( 1 );	// life * 10
			WRITE_BYTE( 8 ); // decay
		MESSAGE_END();
	}
}

void CMarine::MakeGas( void )
{
	Vector		posGun, angleGun;
	TraceResult tr;
	Vector vecEndPos;

	UTIL_MakeVectors( pev->angles );
	m_pBeam = CBeam::BeamCreate( g_pModelNameLaser, 7 );

	if ( m_pBeam )
	{
		GetAttachment( 4, posGun, angleGun );
		UTIL_Sparks( posGun );

		m_pBeam->EntsInit( entindex(), entindex() );
		m_pBeam->SetColor( 24, 121, 239 );
		m_pBeam->SetBrightness( 190 );
		m_pBeam->SetScrollRate( 20 );
		m_pBeam->SetStartAttachment( 4 );
		m_pBeam->SetEndAttachment( 3 );
		m_pBeam->DoSparks( tr.vecEndPos, posGun );
		m_pBeam->SetFlags( BEAM_FSHADEIN );
		m_pBeam->pev->spawnflags = SF_BEAM_SPARKSTART | SF_BEAM_TEMPORARY;
	}
	return;
}

void CMarine :: KillGas( void )
{
	if ( m_pBeam )
	{
		UTIL_Remove( m_pBeam );
		m_pBeam = NULL;
	}
	return;
}

BOOL CMarine :: CanAnswer( void )
{
	return m_bNeedleOut != TRUE;
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

LINK_ENTITY_TO_CLASS( monster_marine_dead, CDeadFGrunt );

//=========================================================
// ********** DeadBGrunt SPAWN **********
//=========================================================
void CDeadFGrunt :: Spawn( )
{
	PRECACHE_MODEL("models/marine.mdl");
	SET_MODEL(ENT(pev), "models/marine.mdl");

	pev->effects		= 0;
	pev->yaw_speed		= 8;
	pev->sequence		= 0;
	m_bloodColor		= BLOOD_COLOR_RED;

	pev->sequence = LookupSequence( m_szPoses[m_iPose] );
	if (pev->sequence == -1)
	{
		ALERT ( at_console, "Dead marine with bad pose\n" );
	}
	// Corpses have less health
	pev->health			= 8;

	SetBodygroup( MN_GUN_GROUP, MN_GUN_NONE );

	if ( m_iHead == 0 )
		SetBodygroup( MN_HEAD_GROUP, MN_HEAD_MASK );
	if ( m_iHead == 1 )
		SetBodygroup( MN_HEAD_GROUP, MN_HEAD_MASK );
	if ( m_iHead == 2 )
		SetBodygroup( MN_HEAD_GROUP, MN_HEAD_SHOTGUN );
	if ( m_iHead == 3 )
		SetBodygroup( MN_HEAD_GROUP, MN_HEAD_SAW );
	if ( m_iHead == 4 )
		SetBodygroup( MN_HEAD_GROUP, MN_HEAD_SAW_BLACK );
	if ( m_iHead == 5 )
		SetBodygroup( MN_HEAD_GROUP, MN_HEAD_MASK );
	if ( m_iHead == 6 )
		SetBodygroup( MN_HEAD_GROUP, MN_HEAD_MAJOR );
	if ( m_iHead == 7 )
		SetBodygroup( MN_HEAD_GROUP, MN_HEAD_BERET_BLACK );

	MonsterInitDead();
}

void CMarine :: StepSound( float volume, int material_type )
{
	switch(material_type)
	{
	case CHAR_TEX_CONCRETE:
		switch (RANDOM_LONG(0,3))
		{
		case 0: EMIT_SOUND_DYN( edict(), CHAN_BODY, "player/pl_step1.wav", volume, ATTN_NORM, 0, PITCH_NORM); break;
		case 1: EMIT_SOUND_DYN( edict(), CHAN_BODY, "player/pl_step2.wav", volume, ATTN_NORM, 0, PITCH_NORM); break;
		case 2: EMIT_SOUND_DYN( edict(), CHAN_BODY, "player/pl_step3.wav", volume, ATTN_NORM, 0, PITCH_NORM); break;
		case 3: EMIT_SOUND_DYN( edict(), CHAN_BODY, "player/pl_step4.wav", volume, ATTN_NORM, 0, PITCH_NORM); break;
		}
		break;
	default:
		CBaseMonster::StepSound(volume, material_type);
		break;
	}
}