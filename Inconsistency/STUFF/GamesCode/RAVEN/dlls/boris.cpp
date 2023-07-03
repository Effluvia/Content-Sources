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
#define BORIS_SHOTGUN_AMMO			8
#define BORIS_M249_AMMO				50
#define	BORIS_MAX_BEAMS				8

#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"monsters.h"
#include	"talkmonster.h"
#include	"schedule.h"
#include	"defaultai.h"
#include	"scripted.h"
#include	"weapons.h"
#include	"soundent.h"

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define		BORIS_AE_SHOOT			( 3 )
#define		BORIS_AE_RELOAD			( 5 )
#define		BORIS_AE_ZAP_POWERUP	( 7 )
#define		BORIS_AE_ZAP_SHOOT		( 8 )
#define		BORIS_AE_ZAP_DONE		( 9 )

class CBoris : public CTalkMonster
{
public:
	void Spawn( void );
	void Precache( void );
	void SetYawSpeed( void );
	int  ISoundMask( void );
	void Shotgun( void );
	void AlertSound( void );
	int  Classify ( void );
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	
	void RunTask( Task_t *pTask );
	void StartTask( Task_t *pTask );
	virtual int	ObjectCaps( void ) { return CTalkMonster :: ObjectCaps() | FCAP_IMPULSE_USE; }
	int TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType);
	BOOL CheckRangeAttack1 ( float flDot, float flDist );
	BOOL CheckRangeAttack2 ( float flDot, float flDist );

	void DeclineFollowing( void );

	// Override these to set behavior
	Schedule_t *GetScheduleOfType ( int Type );
	Schedule_t *GetSchedule ( void );
	MONSTERSTATE GetIdealState ( void );

	void DeathSound( void );
	void PainSound( void );
	
	void TalkInit( void );

	void ClearBeams( );
	void ArmBeam( int side );
	void WackBeam( int side, CBaseEntity *pEntity );
	void ZapBeam( int side );
	void BeamGlow( void );

	void CheckAmmo ( void );

	void TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType);
	void Killed( entvars_t *pevAttacker, int iGib );
	
	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	float	m_painTime;
	float	m_checkAttackTime;
	float	m_flPlayerDamage;
	float	m_flNextAttack;

	CBeam *m_pBeam[BORIS_MAX_BEAMS];
	int		m_iBeams;

	BOOL	m_lastAttackCheck;

	int		m_cAmmoLoaded;
	int		m_cClipSize;
	int		m_iShotgunShell;

	CUSTOM_SCHEDULES;
};

LINK_ENTITY_TO_CLASS( monster_boris, CBoris );

TYPEDESCRIPTION	CBoris::m_SaveData[] = 
{
	DEFINE_FIELD( CBoris, m_painTime, FIELD_TIME ),
	DEFINE_FIELD( CBoris, m_checkAttackTime, FIELD_TIME ),
	DEFINE_FIELD( CBoris, m_lastAttackCheck, FIELD_BOOLEAN ),
	DEFINE_FIELD( CBoris, m_flPlayerDamage, FIELD_FLOAT ),
	DEFINE_FIELD( CBoris, m_cAmmoLoaded, FIELD_INTEGER ),
	DEFINE_FIELD( CBoris, m_cClipSize, FIELD_INTEGER ),
	DEFINE_ARRAY( CBoris, m_pBeam, FIELD_CLASSPTR, BORIS_MAX_BEAMS ),
	DEFINE_FIELD( CBoris, m_iBeams, FIELD_INTEGER ),
	DEFINE_FIELD( CBoris, m_flNextAttack, FIELD_TIME )
};

IMPLEMENT_SAVERESTORE( CBoris, CTalkMonster );

//=========================================================
// AI Schedules Specific to this monster
//=========================================================
Task_t	tlBorisFollow[] =
{
	{ TASK_MOVE_TO_TARGET_RANGE,(float)128		},	// Move within 128 of target ent (client)
	{ TASK_SET_SCHEDULE,		(float)SCHED_TARGET_FACE },
};

Schedule_t	slBorisFollow[] =
{
	{
		tlBorisFollow,
		ARRAYSIZE ( tlBorisFollow ),
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_HEAR_SOUND |
		bits_COND_PROVOKED,
		bits_SOUND_DANGER,
		"Follow"
	},
};

Task_t	tlBorisFaceTarget[] =
{
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_FACE_TARGET,			(float)0		},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_SET_SCHEDULE,		(float)SCHED_TARGET_CHASE },
};

Schedule_t	slBorisFaceTarget[] =
{
	{
		tlBorisFaceTarget,
		ARRAYSIZE ( tlBorisFaceTarget ),
		bits_COND_CLIENT_PUSH	|
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_HEAR_SOUND |
		bits_COND_PROVOKED,
		bits_SOUND_DANGER,
		"FaceTarget"
	},
};


Task_t	tlBorisIdleStand[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_WAIT,				(float)2		}, // repick IDLESTAND every two seconds.
	{ TASK_TLK_HEADRESET,		(float)0		}, // reset head position
};

Schedule_t	slBorisIdleStand[] =
{
	{ 
		tlBorisIdleStand,
		ARRAYSIZE ( tlBorisIdleStand ), 
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_HEAR_SOUND	|
		bits_COND_SMELL			|
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

DEFINE_CUSTOM_SCHEDULES( CBoris )
{
	slBorisFollow,
	slBorisFaceTarget,
	slBorisIdleStand,
};


IMPLEMENT_CUSTOM_SCHEDULES( CBoris, CTalkMonster );

void CBoris :: StartTask( Task_t *pTask )
{
	ClearBeams( );

	CTalkMonster::StartTask( pTask );	
}

void CBoris :: RunTask( Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	default:
		CTalkMonster::RunTask( pTask );
		break;
	}
}

//=========================================================
// ISoundMask - returns a bit mask indicating which types
// of sounds this monster regards. 
//=========================================================
int CBoris :: ISoundMask ( void) 
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
int	CBoris :: Classify ( void )
{
	return	CLASS_PLAYER_ALLY;
}

//=========================================================
// CheckAmmo - overridden for the grunt because he actually
// uses ammo! (base class doesn't)
//=========================================================
void CBoris :: CheckAmmo ( void )
{
	if ( m_cAmmoLoaded <= 0 )
	{
		SetConditions(bits_COND_NO_AMMO_LOADED);
	}
}

//=========================================================
// ALertSound - boris says "Freeze!"
//=========================================================
void CBoris :: AlertSound( void )
{
	if ( m_hEnemy != NULL )
	{
		if ( FOkToSpeak() )
		{
			PlaySentence( "BO_ATTACK", RANDOM_FLOAT(2.8, 3.2), VOL_NORM, ATTN_IDLE );
		}
	}

}
//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CBoris :: SetYawSpeed ( void )
{
	int ys = 0;
	switch ( m_Activity )
	{
	case ACT_IDLE:		
		ys = 70;
		break;
	case ACT_WALK:
		ys = 70;
		break;
	case ACT_RUN:
		ys = 90;
		break;
	default:
		ys = 70;
		break;
	}

	pev->yaw_speed = ys;
}


//=========================================================
// CheckRangeAttack1
//=========================================================
BOOL CBoris :: CheckRangeAttack1 ( float flDot, float flDist )
{
	if ( flDist <= 1024 && flDot >= 0.5 )
	{
		if ( gpGlobals->time > m_checkAttackTime )
		{
			TraceResult tr;
			
			Vector shootOrigin = pev->origin + Vector( 0, 0, 55 );
			CBaseEntity *pEnemy = m_hEnemy;
			Vector shootTarget = ( (pEnemy->BodyTarget( shootOrigin ) - pEnemy->pev->origin) + m_vecEnemyLKP );
			UTIL_TraceLine( shootOrigin, shootTarget, dont_ignore_monsters, ENT(pev), &tr );
			m_checkAttackTime = gpGlobals->time + 1;
			if ( tr.flFraction == 1.0 || (tr.pHit != NULL && CBaseEntity::Instance(tr.pHit) == pEnemy) )
				m_lastAttackCheck = TRUE;
			else
				m_lastAttackCheck = FALSE;
			m_checkAttackTime = gpGlobals->time + 1.5;
		}
		return m_lastAttackCheck;
	}
	return FALSE;
}

//=========================================================
// CheckRangeAttack2 - normal beam attack 
//=========================================================
BOOL CBoris :: CheckRangeAttack2 ( float flDot, float flDist )
{
	if (m_flNextAttack > gpGlobals->time)
	{
		return FALSE;
	}

	return CTalkMonster::CheckRangeAttack2( flDot, flDist );
}

//=========================================================
// Shotgun - shoots one round from the pistol at
// the enemy boris is facing.
//=========================================================
void CBoris :: Shotgun ( void )
{
	Vector vecShootOrigin = GetGunPosition();
	Vector vecShootDir = ShootAtEnemy( vecShootOrigin );

	UTIL_MakeVectors ( pev->angles );

	Vector	vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT(40,90) + gpGlobals->v_up * RANDOM_FLOAT(75,200) + gpGlobals->v_forward * RANDOM_FLOAT(-40, 40);
	EjectBrass ( vecShootOrigin - vecShootDir * 24, vecShellVelocity, pev->angles.y, m_iShotgunShell, TE_BOUNCE_SHOTSHELL); 
	FireBullets(gSkillData.hgruntShotgunPellets, vecShootOrigin, vecShootDir, VECTOR_CONE_15DEGREES, 2048, BULLET_PLAYER_BUCKSHOT, 0 ); // shoot +-7.5 degrees

	pev->effects |= EF_MUZZLEFLASH;
	
	EMIT_SOUND( ENT(pev), CHAN_WEAPON, "weapons/sbarrel1.wav", VOL_NORM, ATTN_NORM );

	m_cAmmoLoaded--;// take away a bullet!

	Vector angDir = UTIL_VecToAngles( vecShootDir );
	SetBlending( 0, angDir.x );
}
		
//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//
// Returns number of events handled, 0 if none.
//=========================================================
void CBoris :: HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch( pEvent->event )
	{
	case BORIS_AE_SHOOT:
		{
			Shotgun();
		}
		break;

	case BORIS_AE_RELOAD:
		{
			EMIT_SOUND( ENT(pev), CHAN_WEAPON, "hgrunt/gr_reload1.wav", 1, ATTN_NORM );
			m_cAmmoLoaded = m_cClipSize;
			ClearConditions(bits_COND_NO_AMMO_LOADED);
		}
		break;

	case BORIS_AE_ZAP_POWERUP:
	{
		UTIL_MakeAimVectors( pev->angles );

		if (m_iBeams == 0)
		{
			Vector vecSrc = pev->origin + gpGlobals->v_forward * 2;
			MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, vecSrc );
				WRITE_BYTE(TE_DLIGHT);
				WRITE_COORD(vecSrc.x);	// X
				WRITE_COORD(vecSrc.y);	// Y
				WRITE_COORD(vecSrc.z);	// Z
				WRITE_BYTE( 12 );		// radius * 0.1
				WRITE_BYTE( 255 );		// r
				WRITE_BYTE( 180 );		// g
				WRITE_BYTE( 96 );		// b
				WRITE_BYTE( 20 / pev->framerate );		// time * 10
				WRITE_BYTE( 0 );		// decay * 0.1
			MESSAGE_END( );

		}

		ArmBeam( -1 );
		ArmBeam( 1 );
		BeamGlow( );

		EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "debris/zap4.wav", 1, ATTN_NORM, 0, 100 + m_iBeams * 10 );
		pev->skin = m_iBeams / 2;
	}
	break;

	case BORIS_AE_ZAP_SHOOT:
	{
		ClearBeams( );
		ClearMultiDamage();

		UTIL_MakeAimVectors( pev->angles );

		ZapBeam( -1 );
		ZapBeam( 1 );

		EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "hassault/hw_shoot1.wav", 1, ATTN_NORM, 0, RANDOM_LONG( 130, 160 ) );
		ApplyMultiDamage(pev, pev);

		m_flNextAttack = gpGlobals->time + RANDOM_FLOAT( 0.5, 4.0 );
	}
	break;

	case BORIS_AE_ZAP_DONE:
	{
		ClearBeams( );
	}
	break;

	default:
		CTalkMonster::HandleAnimEvent( pEvent );
	}
}

//=========================================================
// Spawn
//=========================================================
void CBoris :: Spawn()
{
	Precache( );

	SET_MODEL(ENT(pev), "models/boris.mdl");
	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_RED;
	pev->health			= gSkillData.borisHealth;
	pev->view_ofs		= Vector ( 0, 0, 50 );// position of the eyes relative to monster's origin.
	m_flFieldOfView		= VIEW_FIELD_WIDE; // NOTE: we need a wide field of view so npc will notice player and say hello
	m_MonsterState		= MONSTERSTATE_NONE;

	pev->body			= 1; // gun in holster

	m_afCapability		= bits_CAP_HEAR | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP;
	m_cClipSize			= BORIS_SHOTGUN_AMMO;
	m_cAmmoLoaded		= m_cClipSize;

	MonsterInit();
	SetUse( &CTalkMonster::FollowerUse );
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CBoris :: Precache()
{
	PRECACHE_MODEL("models/boris.mdl");

	PRECACHE_SOUND("boris/bo_pain1.wav");
	PRECACHE_SOUND("boris/bo_pain2.wav");
	PRECACHE_SOUND("boris/bo_pain3.wav");

	PRECACHE_SOUND("boris/bo_die1.wav");
	PRECACHE_SOUND("boris/bo_die2.wav");
	PRECACHE_SOUND("boris/bo_die3.wav");
	
	PRECACHE_MODEL("sprites/lgtning.spr");
	PRECACHE_SOUND("debris/zap1.wav");
	PRECACHE_SOUND("debris/zap4.wav");
	PRECACHE_SOUND("weapons/electro4.wav");
	PRECACHE_SOUND("hassault/hw_shoot1.wav");

	m_iShotgunShell = PRECACHE_MODEL ("models/shotgunshell.mdl");

	// every new boris must call this, otherwise
	// when a level is loaded, nobody will talk (time is reset to 0)
	TalkInit();
	CTalkMonster::Precache();
}	

// Init talk data
void CBoris :: TalkInit()
{
	CTalkMonster::TalkInit();

	// scientists speach group names (group names are in sentences.txt)

	m_szGrp[TLK_ANSWER]  =	"BO_ANSWER";
	m_szGrp[TLK_QUESTION] =	"BO_QUESTION";
	m_szGrp[TLK_IDLE] =		"BO_IDLE";
	m_szGrp[TLK_STARE] =		"BO_STARE";
	m_szGrp[TLK_USE] =		"BO_OK";
	m_szGrp[TLK_UNUSE] =	"BO_WAIT";
	m_szGrp[TLK_STOP] =		"BO_STOP";

	m_szGrp[TLK_NOSHOOT] =	"BO_SCARED";
	m_szGrp[TLK_HELLO] =	"BO_HELLO";

	m_szGrp[TLK_PLHURT1] =	"!BO_CUREA";
	m_szGrp[TLK_PLHURT2] =	"!BO_CUREB"; 
	m_szGrp[TLK_PLHURT3] =	"!BO_CUREC";

	m_szGrp[TLK_PHELLO] =	NULL;	//"BO_PHELLO";		// UNDONE
	m_szGrp[TLK_PIDLE] =	NULL;	//"BO_PIDLE";			// UNDONE
	m_szGrp[TLK_PQUESTION] = "BO_PQUEST";		// UNDONE

	m_szGrp[TLK_SMELL] =	"BO_SMELL";
	
	m_szGrp[TLK_WOUND] =	"BO_WOUND";
	m_szGrp[TLK_MORTAL] =	"BO_MORTAL";

	// get voice for head - just one boris voice for now
	m_voicePitch = 100;
}


static BOOL IsFacing( entvars_t *pevTest, const Vector &reference )
{
	Vector vecDir = (reference - pevTest->origin);
	vecDir.z = 0;
	vecDir = vecDir.Normalize();
	Vector forward, angle;
	angle = pevTest->v_angle;
	angle.x = 0;
	UTIL_MakeVectorsPrivate( angle, forward, NULL, NULL );
	// He's facing me, he meant it
	if ( DotProduct( forward, vecDir ) > 0.96 )	// +/- 15 degrees or so
	{
		return TRUE;
	}
	return FALSE;
}


int CBoris :: TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	// make sure friends talk about it if player hurts talkmonsters...
	int ret = CTalkMonster::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
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
				PlaySentence( "BO_MAD", 4, VOL_NORM, ATTN_NORM );

				Remember( bits_MEMORY_PROVOKED );
				StopFollowing( TRUE );
			}
			else
			{
				// Hey, be careful with that
				PlaySentence( "BO_SHOT", 4, VOL_NORM, ATTN_NORM );
				Remember( bits_MEMORY_SUSPICIOUS );
			}
		}
		else if ( !(m_hEnemy->IsPlayer()) && pev->deadflag == DEAD_NO )
		{
			PlaySentence( "BO_SHOT", 4, VOL_NORM, ATTN_NORM );
		}
	}

	return ret;
}

	
//=========================================================
// PainSound
//=========================================================
void CBoris :: PainSound ( void )
{
	if (gpGlobals->time < m_painTime)
		return;
	
	m_painTime = gpGlobals->time + RANDOM_FLOAT(0.5, 0.75);

	switch (RANDOM_LONG(0,2))
	{
	case 0: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "boris/bo_pain1.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
	case 1: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "boris/bo_pain2.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
	case 2: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "boris/bo_pain3.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
	}
}

//=========================================================
// DeathSound 
//=========================================================
void CBoris :: DeathSound ( void )
{
	switch (RANDOM_LONG(0,2))
	{
		case 0: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "boris/bo_die1.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
		case 1: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "boris/bo_die2.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
		case 2: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "boris/bo_die3.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
	}
}


void CBoris::TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType)
{
	CTalkMonster::TraceAttack( pevAttacker, flDamage, vecDir, ptr, bitsDamageType );
}


void CBoris::Killed( entvars_t *pevAttacker, int iGib )
{
	SetUse( NULL );	
	CTalkMonster::Killed( pevAttacker, iGib );
}

//=========================================================
// AI Schedules Specific to this monster
//=========================================================

Schedule_t* CBoris :: GetScheduleOfType ( int Type )
{
	Schedule_t *psched;

	switch( Type )
	{
	// Hook these to make a looping schedule
	case SCHED_TARGET_FACE:
		{
			psched = CTalkMonster::GetScheduleOfType(Type);

			if (psched == slIdleStand)
				return slBorisFaceTarget;	// override this for different target face behavior
			else
				return psched;
		}
		break;

	case SCHED_TARGET_CHASE:
		{
			return slBorisFollow;
		}
		break;

	case SCHED_IDLE_STAND:
		{
			psched = CTalkMonster::GetScheduleOfType(Type);

			if (psched == slIdleStand)
			{
				// just look straight ahead.
				return slBorisIdleStand;
			}
			else
				return psched;	
		}
		break;
	}

	return CTalkMonster::GetScheduleOfType( Type );
}

//=========================================================
// GetSchedule - Decides which type of schedule best suits
// the monster's current state and conditions. Then calls
// monster's member function to get a pointer to a schedule
// of the proper type.
//=========================================================
Schedule_t *CBoris :: GetSchedule ( void )
{
	ClearBeams( );

	if ( HasConditions( bits_COND_HEAR_SOUND ) )
	{
		CSound *pSound;
		pSound = PBestSound();

		ASSERT( pSound != NULL );
		if ( pSound && (pSound->m_iType & bits_SOUND_DANGER) )
			return GetScheduleOfType( SCHED_TAKE_COVER_FROM_BEST_SOUND );
	}

	if ( HasConditions( bits_COND_ENEMY_DEAD ) && FOkToSpeak() )
	{
		PlaySentence( "BO_KILL", 4, VOL_NORM, ATTN_NORM );
	}

	switch( m_MonsterState )
	{
	case MONSTERSTATE_COMBAT:
		{
// dead enemy
			if ( HasConditions( bits_COND_ENEMY_DEAD ) )
			{
				// call bo_se class, all code to handle dead enemies is centralized there.
				return CBaseMonster :: GetSchedule();
			}

			if ( HasConditions( bits_COND_CAN_RANGE_ATTACK2) )
			{
				return GetScheduleOfType ( SCHED_RANGE_ATTACK2 );
			}

			// no ammo
			if ( HasConditions ( bits_COND_NO_AMMO_LOADED ) )
			{
				return GetScheduleOfType ( SCHED_RELOAD );
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
		{
			if ( HasConditions(bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE))
			{
				// flinch if hurt
				return GetScheduleOfType( SCHED_SMALL_FLINCH );
			}

			if ( m_hEnemy == NULL && IsFollowing() )
			{
				if ( !m_hTargetEnt->IsAlive() )
				{
					// UNDONE: Comment about the recently dead player here?
					StopFollowing( FALSE );
					break;
				}
				else
				{
					if ( HasConditions( bits_COND_CLIENT_PUSH ) )
					{
						return GetScheduleOfType( SCHED_MOVE_AWAY_FOLLOW );
					}
					return GetScheduleOfType( SCHED_TARGET_FACE );
				}
			}

			if ( HasConditions( bits_COND_CLIENT_PUSH ) )
			{
				return GetScheduleOfType( SCHED_MOVE_AWAY );
			}

			// try to say something about smells
			TrySmellTalk();
		}
		break;
	}
	
	return CTalkMonster::GetSchedule();
}

MONSTERSTATE CBoris :: GetIdealState ( void )
{
	return CTalkMonster::GetIdealState();
}



void CBoris::DeclineFollowing( void )
{
	PlaySentence( "BO_POK", 2, VOL_NORM, ATTN_NORM );
}

//=========================================================
// ArmBeam - small beam from arm to nearby geometry
//=========================================================
void CBoris :: ArmBeam( int side )
{
	TraceResult tr;
	float flDist = 1.0;
	
	if (m_iBeams >= BORIS_MAX_BEAMS)
		return;

	UTIL_MakeAimVectors( pev->angles );
	Vector vecSrc = pev->origin + gpGlobals->v_up * 36 + gpGlobals->v_right * side * 16 + gpGlobals->v_forward * 32;

	for (int i = 0; i < 3; i++)
	{
		Vector vecAim = gpGlobals->v_right * side * RANDOM_FLOAT( 0, 1 ) + gpGlobals->v_up * RANDOM_FLOAT( -1, 1 );
		TraceResult tr1;
		UTIL_TraceLine ( vecSrc, vecSrc + vecAim * 512, dont_ignore_monsters, ENT( pev ), &tr1);
		if (flDist > tr1.flFraction)
		{
			tr = tr1;
			flDist = tr.flFraction;
		}
	}

	// Couldn't find anything close enough
	if ( flDist == 1.0 )
		return;

	DecalGunshot( &tr, BULLET_PLAYER_CROWBAR );

	m_pBeam[m_iBeams] = CBeam::BeamCreate( "sprites/lgtning.spr", 30 );
	if (!m_pBeam[m_iBeams])
		return;

	m_pBeam[m_iBeams]->PointEntInit( tr.vecEndPos, entindex( ) );
	m_pBeam[m_iBeams]->SetEndAttachment( side < 0 ? 3 : 2 );
	m_pBeam[m_iBeams]->SetColor( 96, 96, 250 );
	m_pBeam[m_iBeams]->SetBrightness( 64 );
	m_pBeam[m_iBeams]->SetNoise( 80 );
	m_iBeams++;
}


//=========================================================
// BeamGlow - brighten all beams
//=========================================================
void CBoris :: BeamGlow( )
{
	int b = m_iBeams * 32;
	if (b > 255)
		b = 255;

	for (int i = 0; i < m_iBeams; i++)
	{
		if (m_pBeam[i]->GetBrightness() != 255) 
		{
			m_pBeam[i]->SetBrightness( b );
		}
	}
}


//=========================================================
// WackBeam - regenerate dead colleagues
//=========================================================
void CBoris :: WackBeam( int side, CBaseEntity *pEntity )
{
	Vector vecDest;
	float flDist = 1.0;
	
	if (m_iBeams >= BORIS_MAX_BEAMS)
		return;

	if (pEntity == NULL)
		return;

	m_pBeam[m_iBeams] = CBeam::BeamCreate( "sprites/lgtning.spr", 30 );
	if (!m_pBeam[m_iBeams])
		return;

	m_pBeam[m_iBeams]->PointEntInit( pEntity->Center(), entindex( ) );
	m_pBeam[m_iBeams]->SetEndAttachment( side < 0 ? 3 : 2 );
	m_pBeam[m_iBeams]->SetColor( 96, 96, 250 );
	m_pBeam[m_iBeams]->SetBrightness( 255 );
	m_pBeam[m_iBeams]->SetNoise( 80 );
	m_iBeams++;
}

//=========================================================
// ZapBeam - heavy damage directly forward
//=========================================================
void CBoris :: ZapBeam( int side )
{
	Vector vecSrc, vecAim;
	TraceResult tr;
	CBaseEntity *pEntity;

	if (m_iBeams >= BORIS_MAX_BEAMS)
		return;

	vecSrc = pev->origin + gpGlobals->v_up * 36;
	vecAim = ShootAtEnemy( vecSrc );
	float deflection = 0.01;
	vecAim = vecAim + side * gpGlobals->v_right * RANDOM_FLOAT( 0, deflection ) + gpGlobals->v_up * RANDOM_FLOAT( -deflection, deflection );
	UTIL_TraceLine ( vecSrc, vecSrc + vecAim * 1024, dont_ignore_monsters, ENT( pev ), &tr);

	m_pBeam[m_iBeams] = CBeam::BeamCreate( "sprites/lgtning.spr", 50 );
	if (!m_pBeam[m_iBeams])
		return;

	m_pBeam[m_iBeams]->PointEntInit( tr.vecEndPos, entindex( ) );
	m_pBeam[m_iBeams]->SetEndAttachment( side < 0 ? 2 : 1 );
	m_pBeam[m_iBeams]->SetColor( 96, 96, 250 );
	m_pBeam[m_iBeams]->SetBrightness( 255 );
	m_pBeam[m_iBeams]->SetNoise( 60 );
	m_iBeams++;

	pEntity = CBaseEntity::Instance(tr.pHit);
	if (pEntity != NULL && pEntity->pev->takedamage)
		pEntity->TraceAttack( pev, gSkillData.borisDmgZap, vecAim, &tr, DMG_SHOCK );

	UTIL_EmitAmbientSound( ENT(pev), tr.vecEndPos, "weapons/electro4.wav", 0.5, ATTN_NORM, 0, RANDOM_LONG( 140, 160 ) );
}


//=========================================================
// ClearBeams - remove all beams
//=========================================================
void CBoris :: ClearBeams( )
{
	for (int i = 0; i < BORIS_MAX_BEAMS; i++)
	{
		if (m_pBeam[i])
		{
			UTIL_Remove( m_pBeam[i] );
			m_pBeam[i] = NULL;
		}
	}
	m_iBeams = 0;
	pev->skin = 0;

	STOP_SOUND( ENT(pev), CHAN_WEAPON, "debris/zap4.wav" );
}
