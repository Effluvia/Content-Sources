#include "cbase.h"
#include "ai_basenpc.h"
#include "animation.h"
#include "beam_shared.h"
#include "fmtstr.h"
#include "npcevent.h"
#include "props.h"
#include "SoundEmitterSystem/isoundemittersystembase.h"
#include "Sprite.h"
#include "weapon_physcannon.h"
#include "ai_tacticalservices.h"
#define VORT_TELE_STUFF
#ifdef VORT_TELE_STUFF
#include "ai_memory.h"
#include "hoe_human.h"
#endif
#include "hoe_deathsound.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define VORTIGAUNT_MAX_BEAMS				8
#define	VORTIGAUNT_ZAP_GLOWGROW_TIME		0.5			// How long does glow last
#define	VORTIGAUNT_GLOWFADE_TIME			0.5			// How long does it fade


class CNPC_Vortigaunt : public CAI_BaseNPC
{
public:
	DECLARE_CLASS( CNPC_Vortigaunt, CAI_BaseNPC );
	DECLARE_DATADESC();
	DEFINE_CUSTOM_AI;

	void Spawn( void );
	void Precache( void );

	Class_T Classify( void );
	bool ClassifyPlayerAlly( void ) { return false; }
	bool ClassifyPlayerAllyVital( void ) { return false; }

	void HandleAnimEvent( animevent_t *pEvent );

	void Event_Killed( const CTakeDamageInfo &info );
	void UpdateOnRemove( void );

	bool ShouldGib( const CTakeDamageInfo &info );
	bool HasHumanGibs( void ) { return false; }
	bool HasAlienGibs( void ) { return true; }

	virtual float InnateRange1MinRange( void ) { return 0.0f; }
	virtual float InnateRange1MaxRange( void ) { return 1500.0*12; }
	bool InnateWeaponLOSCondition( const Vector &ownerPos, const Vector &targetPos, bool bSetConditions );

	int RangeAttack1Conditions( float flDot, float flDist );
	int MeleeAttack1Conditions( float flDot, float flDist );
	void Claw( void  );

	void AlertSound( void );
	void IdleSound( void );
	void PainSound( const CTakeDamageInfo &info );
	void DeathSound( const CTakeDamageInfo &info );
	void AttackHitSound( void );
	void AttackMissSound( void );

	int GetSoundInterests( void );

	int SelectSchedule( void );
	int TranslateSchedule( int scheduleType );
	void BuildScheduleTestBits( void );
	void StartTask( const Task_t *pTask );
	void RunTask( const Task_t *pTask );

	void TraceAttack( const CTakeDamageInfo &inputInfo, const Vector &vecDir, trace_t *ptr );

#ifdef VORT_TELE_STUFF
	void EnemiesForgetAboutMe( void );
#endif

	int m_iLeftHandAttachment;
	int m_iRightHandAttachment;
	bool m_bStopLoopingSounds;

	// ------------
	// Beams
	// ------------
	void ClearBeams( void );
	void BeamGlow( void );
	void ArmBeam( int side );
	void ZapBeam( int side );

	CHandle<CBeam>	m_pBeam[VORTIGAUNT_MAX_BEAMS];
	int				m_iBeams;
	int				m_nLightningSprite;

	// ---------------
	//  Glow
	// ----------------
	void			StartHandGlow( void );
	void			EndHandGlow( void );
	void			ClearHandGlow( void );
	float			m_fGlowAge;
	float			m_fGlowScale;
	float			m_fGlowChangeTime;
	bool			m_bGlowTurningOn;
	int				m_nCurGlowIndex;
#ifdef VORT_TELE_STUFF
	Vector			m_vecTeleport;
	int				m_iTeleportSprite;
	float			m_flTimeLastTeleport;
#endif

	CHandle<CSprite>	m_pLeftHandGlow;
	CHandle<CSprite>	m_pRightHandGlow;

	//-----------------------------------------------------------------------------
	// Custom schedules.
	//-----------------------------------------------------------------------------
	enum
	{
		SCHED_VORTIGAUNT_RANGE_ATTACK = BaseClass::NEXT_SCHEDULE,
#ifdef VORT_TELE_STUFF
		SCHED_VORTIGAUNT_TELEPORT_HIDE,
		SCHED_VORTIGAUNT_TELEPORT_ATTACK,
#endif
		NEXT_SCHEDULE,
	};

#ifdef VORT_TELE_STUFF
	//-----------------------------------------------------------------------------
	// Tasks
	//-----------------------------------------------------------------------------
	enum
	{
		TASK_VORT_FIND_TELEPORT_HIDE = BaseClass::NEXT_TASK,
		TASK_VORT_FIND_TELEPORT_ATTACK,
		TASK_VORT_TELEPORT,
		NEXT_TASK,
	};
#endif
};

LINK_ENTITY_TO_CLASS( npc_vortigaunt, CNPC_Vortigaunt );

//-----------------------------------------------------------------------------
// Animation events.
//-----------------------------------------------------------------------------
#define AE_ISLAVE_CLAW 1
#define AE_ISLAVE_CLAWRAKE 2
#define AE_ISLAVE_ZAP_POWERUP 3
#define AE_ISLAVE_ZAP_SHOOT 4
#define AE_ISLAVE_ZAP_DONE 5

//-----------------------------------------------------------------------------
// Interactions
//-----------------------------------------------------------------------------
// TEMP from HL2
int	g_interactionVortigauntStomp		= 0;
int	g_interactionVortigauntStompFail	= 0;
int	g_interactionVortigauntStompHit		= 0;
int	g_interactionVortigauntKick			= 0;
int	g_interactionVortigauntClaw			= 0;

BEGIN_DATADESC( CNPC_Vortigaunt )
	DEFINE_ARRAY( m_pBeam,					FIELD_EHANDLE, VORTIGAUNT_MAX_BEAMS ),
	DEFINE_FIELD( m_iBeams,					FIELD_INTEGER),
	DEFINE_FIELD( m_fGlowAge,				FIELD_FLOAT),
	DEFINE_FIELD( m_fGlowScale,				FIELD_FLOAT),
	DEFINE_FIELD( m_fGlowChangeTime,		FIELD_FLOAT),
	DEFINE_FIELD( m_bGlowTurningOn,			FIELD_BOOLEAN),
	DEFINE_FIELD( m_nCurGlowIndex,			FIELD_INTEGER),
	DEFINE_FIELD( m_pLeftHandGlow,			FIELD_EHANDLE),
	DEFINE_FIELD( m_pRightHandGlow,			FIELD_EHANDLE),
	DEFINE_FIELD( m_iLeftHandAttachment,	FIELD_INTEGER ),
	DEFINE_FIELD( m_iRightHandAttachment,	FIELD_INTEGER ),
#ifdef VORT_TELE_STUFF
	DEFINE_FIELD( m_vecTeleport,			FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( m_flTimeLastTeleport,		FIELD_TIME ),
#endif
END_DATADESC()

ConVar	sk_vortigaunt_health( "sk_vortigaunt_health","0");
ConVar	sk_vortigaunt_dmg_claw( "sk_vortigaunt_dmg_claw","0");
ConVar	sk_vortigaunt_dmg_rake( "sk_vortigaunt_dmg_rake","0");
ConVar	sk_vortigaunt_dmg_zap( "sk_vortigaunt_dmg_zap","0");

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Vortigaunt::Spawn( void )
{
	BaseClass::Spawn();

	Precache();
	SetModel( STRING( GetModelName() ) );

#if 0
	CStudioHdr *pStudioHdr = GetModelPtr();
	if ( pStudioHdr && pStudioHdr->SequencesAvailable() )
	{
		for ( int nSeq = 0; nSeq < pStudioHdr->GetNumSeq(); nSeq++ )
		{
			mstudioseqdesc_t &seqdesc = pStudioHdr->pSeqdesc( nSeq );
			DevMsg("islave.mdl: seq %s act %s\n", seqdesc.pszLabel(), seqdesc.pszActivityName());
			animevent_t event;
			int index = 0;
			while ( ( index = GetAnimationEvent( pStudioHdr, nSeq, &event, 0.0f, 1.0f, index ) ) != 0 )
			{
				DevMsg( "islave.mdl: animevent %s %d\n", "<anim name>", event.event );
			}
		}
	}
#endif

	SetHullType(HULL_WIDE_HUMAN);
	SetHullSizeNormal();

	SetSolid( SOLID_BBOX );
	SetMoveType( MOVETYPE_STEP );

	CapabilitiesClear();
	CapabilitiesAdd(
		bits_CAP_MOVE_GROUND |
		bits_CAP_DOORS_GROUP |
		bits_CAP_INNATE_RANGE_ATTACK1 |
		bits_CAP_INNATE_MELEE_ATTACK1 );

	SetBloodColor( BLOOD_COLOR_GREEN );
	m_NPCState = NPC_STATE_NONE;

	m_flFieldOfView = -1.0;
	SetViewOffset( Vector ( 0, 0, 64 ) ); // Position of the eyes relative to NPC's origin.

	m_iHealth = sk_vortigaunt_health.GetInt();

	m_nCurGlowIndex			= 0;
	m_pLeftHandGlow			= NULL;
	m_pRightHandGlow		= NULL;

	m_bStopLoopingSounds	= false;

	m_iLeftHandAttachment = 1;
	m_iRightHandAttachment = 0;

	NPCInit();
}

//-----------------------------------------------------------------------------
// Purpose: Precaches all resources this monster needs.
//-----------------------------------------------------------------------------
void CNPC_Vortigaunt::Precache( void )
{
	SetModelName( AllocPooledString( "models/islave.mdl" ) );
	PrecacheModel( STRING( GetModelName() ) );

	/*m_nLightningSprite = */PrecacheModel("sprites/lgtning.vmt");
	PrecacheModel("sprites/greenglow1.vmt");

	PrecacheScriptSound("NPC_Vortigaunt.Pain");
	PrecacheScriptSound("NPC_Vortigaunt.Die");
	PrecacheScriptSound("NPC_Vortigaunt.AttackHit");
	PrecacheScriptSound("NPC_Vortigaunt.AttackMiss");
	PrecacheScriptSound("NPC_Vortigaunt.ZapPowerup");
	PrecacheScriptSound("NPC_Vortigaunt.ZapShoot");
	PrecacheScriptSound("NPC_Vortigaunt.WTF");

#ifdef VORT_TELE_STUFF
	PrecacheScriptSound("NPC_Vortigaunt.Teleport");
	m_iTeleportSprite = PrecacheModel( "sprites/fexplo1.vmt" );
#endif

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Indicates this monster's place in the relationship table.
// Output : 
//-----------------------------------------------------------------------------
Class_T	CNPC_Vortigaunt::Classify( void )
{
	return CLASS_VORTIGAUNT; 
}

//-----------------------------------------------------------------------------
void CNPC_Vortigaunt::UpdateOnRemove( void )
{
	ClearBeams();
	ClearHandGlow();

	// Chain at end to mimic destructor unwind order
	BaseClass::UpdateOnRemove();
}

//------------------------------------------------------------------------------
void CNPC_Vortigaunt::Event_Killed( const CTakeDamageInfo &info )
{
	ClearBeams();
	ClearHandGlow();

	BaseClass::Event_Killed( info );
}

//-----------------------------------------------------------------------------
bool CNPC_Vortigaunt::ShouldGib( const CTakeDamageInfo &info )
{
	if ( info.GetDamageType() & (DMG_NEVERGIB|DMG_DISSOLVE) )
		return false;

	if ( info.GetDamageType() & (DMG_ALWAYSGIB/*|DMG_BLAST*/) )
		return true;

	return false;
}

//-----------------------------------------------------------------------------
void CNPC_Vortigaunt::HandleAnimEvent( animevent_t *pEvent )
{
//	DevMsg("CNPC_Vortigaunt::HandleAnimEvent %d\n", pEvent->event);

	if ( pEvent->event == AE_ISLAVE_CLAW )
	{
		Claw();
		return;
	}

	if ( pEvent->event == AE_ISLAVE_CLAWRAKE )
	{
		Claw();
		return;
	}

	if (pEvent->event == AE_ISLAVE_ZAP_POWERUP)
	{
#if 1
		m_flPlaybackRate = 2.0;
#else
		// speed up attack when on hard
		if ( g_iSkillLevel == SKILL_HARD )
			 m_flPlaybackRate = 1.5;
#endif

		ArmBeam( -1 );
		ArmBeam( 1 );
		BeamGlow();

		// Make hands glow if not already glowing
		if ( m_fGlowAge == 0 )
		{
			StartHandGlow();
		}

		CPASAttenuationFilter filter( this );
		CSoundParameters params;
		if ( GetParametersForSound( "NPC_Vortigaunt.ZapPowerup", params, NULL ) )
		{
#if 1
			EmitSound_t ep;
			ep.m_nChannel = params.channel;
			ep.m_pSoundName = "NPC_Vortigaunt.ZapPowerup";
			ep.m_flVolume = params.volume;
			ep.m_nPitch = 100 + m_iBeams * 15;
			ep.m_SoundLevel = params.soundlevel;
			ep.m_nFlags = m_bStopLoopingSounds ? SND_CHANGE_PITCH : 0; // this event happens 4 times
#else // with this method the raw WAV generates no closed caption
			EmitSound_t ep( params );
			ep.m_nPitch = 100 + m_iBeams * 10;
#endif
			EmitSound( filter, entindex(), ep );
			m_bStopLoopingSounds = true;
		}
		CSoundEnt::InsertSound( SOUND_COMBAT, WorldSpaceCenter(), SOUNDENT_VOLUME_PISTOL, 0.2, this, SOUNDENT_CHANNEL_REPEATING );

		//m_nSkin = m_iBeams / 2;
		return;
	}

	if (pEvent->event == AE_ISLAVE_ZAP_SHOOT)
	{
		ClearBeams();

		ClearMultiDamage();

		ZapBeam( -1 );
		ZapBeam( 1 );
		EndHandGlow();

		EmitSound( "NPC_Vortigaunt.ZapShoot" );
		m_bStopLoopingSounds = true;
		ApplyMultiDamage();

		m_flNextAttack = gpGlobals->curtime + random->RandomFloat( 0.5, 4.0 );
		return;
	}

	if (pEvent->event == AE_ISLAVE_ZAP_DONE)
	{
		ClearBeams();
		return;
	}

	switch( pEvent->event )
	{
	case 1:
	default:
		BaseClass::HandleAnimEvent( pEvent );
		break;
	}
}

//------------------------------------------------------------------------------
bool CNPC_Vortigaunt::InnateWeaponLOSCondition( const Vector &ownerPos, const Vector &targetPos, bool bSetConditions )
{
	CBaseEntity *pTargetEnt = GetEnemy();

	Vector forward, right, up;
	AngleVectors( GetLocalAngles(), &forward, &right, &up );

	Vector vecSrc = ownerPos + up * 36;
	Vector vecAim = targetPos - vecSrc;
	vecAim.NormalizeInPlace();

	trace_t tr;
	AI_TraceLine ( vecSrc, vecSrc + vecAim * 1024, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr );

	if ( ( pTargetEnt && tr.m_pEnt == pTargetEnt ) || tr.fraction == 1.0 )
	{
		return true;
	}

	if ( bSetConditions )
	{
		SetCondition( COND_WEAPON_SIGHT_OCCLUDED );
		SetEnemyOccluder( tr.m_pEnt );
	}
	return false;
}

//------------------------------------------------------------------------------
int CNPC_Vortigaunt::RangeAttack1Conditions( float flDot, float flDist )
{
	if (GetEnemy() == NULL)
	{
		return( COND_NONE );
	}

	if ( gpGlobals->curtime < m_flNextAttack )
	{
		return( COND_NONE );
	}

	// dvs: Allow up-close range attacks for episodic as the vort's melee
	// attack is rather ineffective.
#ifndef HL2_EPISODIC
	if ( flDist <= 70 )
	{
		return( COND_TOO_CLOSE_TO_ATTACK );
	}
#else
	if ( flDist < 32.0f )
		return COND_TOO_CLOSE_TO_ATTACK;
#endif // HL2_EPISODIC
	if ( flDist > InnateRange1MaxRange() )
	{
		return( COND_TOO_FAR_TO_ATTACK );
	}
	else if ( flDot < 0.65 )
	{
		return( COND_NOT_FACING_ATTACK );
	}

	return( COND_CAN_RANGE_ATTACK1 );
}

//-----------------------------------------------------------------------------
int CNPC_Vortigaunt::MeleeAttack1Conditions ( float flDot, float flDist )
{
	if (flDist > 70)
	{
		return COND_TOO_FAR_TO_ATTACK;
	}
	else if (flDot < 0.7)
	{
		// If I'm not facing attack clear TOOFAR which may have been set by range attack
		// Otherwise we may try to back away even when melee attack is valid
		ClearCondition(COND_TOO_FAR_TO_ATTACK);
		return COND_NOT_FACING_ATTACK;
	}

	return COND_CAN_MELEE_ATTACK1;
}

//------------------------------------------------------------------------------
void CNPC_Vortigaunt::Claw( void )
{
	CBaseEntity *pHurt = CheckTraceHullAttack( 40, Vector(-10,-10,-10), Vector(10,10,10), sk_vortigaunt_dmg_claw.GetFloat(), DMG_SLASH );
	if ( pHurt )
	{
		pHurt->ViewPunch( QAngle(5,0,-18) );

		// Play a random attack hit sound
		EmitSound( "NPC_Vortigaunt.AttackHit" );
	}
}

//-----------------------------------------------------------------------------
void CNPC_Vortigaunt::AlertSound( void )
{
	PlaySentence( "SLV_ALERT", 0.0, 0.85, SNDLVL_NORM );
}

//-----------------------------------------------------------------------------
void CNPC_Vortigaunt::IdleSound( void )
{
	if ( random->RandomInt( 0, 2 ) == 0 )
	{
		PlaySentence( "SLV_IDLE", 0.0, 0.85, SNDLVL_NORM );
	}
}

//-----------------------------------------------------------------------------
void CNPC_Vortigaunt::PainSound( const CTakeDamageInfo &info )
{
	EmitSound( "NPC_Vortigaunt.Pain" );
}

//-----------------------------------------------------------------------------
void CNPC_Vortigaunt::DeathSound( const CTakeDamageInfo &info )
{
	CNPCDeathSound *pEnt = (CNPCDeathSound *) CBaseEntity::Create( "npc_death_sound", GetAbsOrigin(), GetAbsAngles(), NULL );
	if ( pEnt )
	{
		EmitSound( "AI_BaseNPC.SentenceStop" );
		Q_strcpy( pEnt->m_szSoundName.GetForModify(), "NPC_Vortigaunt.Die" );
		m_hDeathSound = pEnt;
	}
	else
	{
		Assert( 0 );
		EmitSound( "NPC_Vortigaunt.Die" );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Play a random attack sound.
//-----------------------------------------------------------------------------
void CNPC_Vortigaunt::AttackHitSound( void )
{
	EmitSound( "NPC_Vortigaunt.AttackHit" );
}

//-----------------------------------------------------------------------------
// Purpose: Play a random attack sound.
//-----------------------------------------------------------------------------
void CNPC_Vortigaunt::AttackMissSound( void )
{
	EmitSound( "NPC_Vortigaunt.AttackMiss" );
}

//=========================================================
// GetSoundInterests - returns a bit mask indicating which types
// of sounds this monster regards. 
//=========================================================
int CNPC_Vortigaunt::GetSoundInterests( void ) 
{
	return BaseClass::GetSoundInterests();
}

//---------------------------------------------------------
int CNPC_Vortigaunt::SelectSchedule( void )
{
	ClearBeams();
	EndHandGlow();

#ifdef VORT_TELE_STUFF
	if ( m_flTimeLastTeleport < gpGlobals->curtime - 0.75 &&
		GetState() == NPC_STATE_COMBAT &&
		 /*HasCondition( COND_SEE_ENEMY ) &&*/
		 !IsMoving() )
	{
		// Teleport away after attacking
		if ( GetLastAttackTime() > gpGlobals->curtime - 2.0 )
			return SCHED_VORTIGAUNT_TELEPORT_HIDE;

		switch ( random->RandomInt(0,3) )
		{
		case 1:
			return SCHED_VORTIGAUNT_TELEPORT_HIDE;
		case 2:
			return SCHED_VORTIGAUNT_TELEPORT_ATTACK;
		}
	}
#endif

	return BaseClass::SelectSchedule();
}

//---------------------------------------------------------
int CNPC_Vortigaunt::TranslateSchedule( int scheduleType )
{

	switch( scheduleType )
	{
	case SCHED_RANGE_ATTACK1:
		return SCHED_VORTIGAUNT_RANGE_ATTACK;
		break;
	case SCHED_FAIL_ESTABLISH_LINE_OF_FIRE:
		{
			// Fail schedule doesn't go through SelectSchedule()
			// So we have to clear beams and glow here
			ClearBeams();
			EndHandGlow();

#ifdef VORT_TELE_STUFF
			return SCHED_VORTIGAUNT_TELEPORT_ATTACK;
#else
			return SCHED_COMBAT_FACE;
#endif
			break;
		}
	case SCHED_FAIL_TAKE_COVER:
		{
			// Fail schedule doesn't go through SelectSchedule()
			// So we have to clear beams and glow here
			ClearBeams();
			EndHandGlow();

			return SCHED_RUN_RANDOM;
			break;
		}
#ifdef VORT_TELE_STUFF
	case SCHED_CHASE_ENEMY_FAILED:
		{
			if ( GetEnemy() )
				return SCHED_VORTIGAUNT_TELEPORT_ATTACK;
		}
		break;
#endif
	}
	return BaseClass::TranslateSchedule( scheduleType );
}

//-----------------------------------------------------------------------------
// Purpose: Allows for modification of the interrupt mask for the current schedule.
//			In the most cases the base implementation should be called first.
//-----------------------------------------------------------------------------
void CNPC_Vortigaunt::BuildScheduleTestBits( void )
{
	// Ignore damage if we were recently damaged or we're attacking.
	if ( GetActivity() == ACT_MELEE_ATTACK1 )
	{
		ClearCustomInterruptCondition( COND_LIGHT_DAMAGE );
		ClearCustomInterruptCondition( COND_HEAVY_DAMAGE );
	}
#ifndef HL2_EPISODIC
	else if ( m_flNextFlinch >= gpGlobals->curtime )
	{
		ClearCustomInterruptCondition( COND_LIGHT_DAMAGE );
		ClearCustomInterruptCondition( COND_HEAVY_DAMAGE );
	}
#endif // !HL2_EPISODIC

	BaseClass::BuildScheduleTestBits();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Vortigaunt::StartTask( const Task_t *pTask )
{
	switch( pTask->iTask )
	{
#ifdef VORT_TELE_STUFF
	case TASK_VORT_FIND_TELEPORT_HIDE:
		if ( !FindCoverPosInRadius( GetEnemy(), GetAbsOrigin(), 768, &m_vecTeleport ) )
		{
			TaskFail( FAIL_NO_COVER );
		}
		else
		{
			TaskComplete();
		}
		break;
	case TASK_VORT_FIND_TELEPORT_ATTACK:
		{
			CBaseEntity *pEnemy = GetEnemy();
			if ( !pEnemy )
			{
				TaskFail( FAIL_NO_ENEMY );
				break;
			}

			Vector vecEnemyOrigin = pEnemy->GetAbsOrigin();
			Vector vecEnemyEyePos = pEnemy->EyePosition();
			QAngle vecEnemyAngles = pEnemy->GetAbsAngles();

			Vector vecFacing;
			AngleVectors( vecEnemyAngles, &vecFacing );

			// A: Find position behind enemy with LOS to enemy
			FlankType_t eFlankType = FLANKTYPE_ARC;
			Vector vecFlankRefPos = vecEnemyOrigin + vecFacing * 100;
			float flFlankParam = 90;

			if ( !GetTacticalServices()->FindLosFromThreatPos( vecEnemyOrigin, vecEnemyEyePos, 70, 1000, 1.0, eFlankType, vecFlankRefPos, flFlankParam, &m_vecTeleport ) )
			{
				// B: Find position with LOS to enemy
				if ( !GetTacticalServices()->FindLosFromThreatPos( vecEnemyOrigin, vecEnemyEyePos, 70, 1000, 1.0, &m_vecTeleport ) )
				{
					TaskFail( FAIL_NO_SHOOT );
					break;
				}
			}
			TaskComplete();
			break;
		}
	case TASK_VORT_TELEPORT:
		{
			CSprite *pSprite = CSprite::SpriteCreate( "sprites/fexplo1.vmt",
				CollisionProp()->WorldSpaceCenter(), true );
			pSprite->SetTransparency( kRenderTransAdd, 255, 255, 255, 128, kRenderFxNone );
			pSprite->SetScale( 1.0 );
			pSprite->AnimateAndDie( 30.0 );

			EmitSound( "NPC_Vortigaunt.Teleport" );
			CSoundEnt::InsertSound( SOUND_COMBAT, WorldSpaceCenter(), SOUNDENT_VOLUME_PISTOL, 0.2, this, SOUNDENT_CHANNEL_WEAPON );

			// Face the enemy
			QAngle angles = GetAbsAngles();
			if ( GetEnemy() )
				angles.y = UTIL_VecToYaw( GetEnemy()->GetAbsOrigin() - m_vecTeleport );
			Teleport( &m_vecTeleport, &angles, NULL );
			GetMotor()->SetIdealYaw( angles.y );

			m_flTimeLastTeleport = gpGlobals->curtime;

#ifdef VORT_TELE_STUFF
			EnemiesForgetAboutMe();
#endif

			TaskComplete();
			break;
		}
#endif
#if 0
	case TASK_ANNOUNCE_ATTACK:
		{
			EmitSound("NPC_Spx.AnnounceAttack"); // could I use responserules?
			BaseClass::StartTask( pTask );
		}
		break;
#endif
	default:
		BaseClass::StartTask(pTask);
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Vortigaunt::RunTask( const Task_t *pTask )
{
	switch (pTask->iTask)
	{
	case TASK_RANGE_ATTACK1:
		{
			if (GetEnemy() != NULL)
			{
				if ( GetEnemy()->IsPlayer() )
				{
					m_flPlaybackRate = 1.5;
				}
				if (!GetEnemy()->IsAlive())
				{
					if( IsActivityFinished() )
					{
						TaskComplete();
						break;
					}
				}
				// This is along attack sequence so if the enemy
				// becomes occluded bail
				if (HasCondition( COND_ENEMY_OCCLUDED ))
				{
					TaskComplete();
					break;
				}

				CSoundEnt::InsertSound( SOUND_COMBAT, WorldSpaceCenter(), SOUNDENT_VOLUME_PISTOL, 0.2, this, SOUNDENT_CHANNEL_REPEATING );

			}
			BaseClass::RunTask( pTask );
			break;
		}
	default:
		BaseClass::RunTask(pTask);
		break;
	}
}

//=========================================================
// BeamGlow - brighten all beams
//=========================================================
void CNPC_Vortigaunt::BeamGlow( void )
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
// ArmBeam - small beam from arm to nearby geometry
//=========================================================
void CNPC_Vortigaunt::ArmBeam( int side )
{
	trace_t tr;
	float flDist = 1.0;
	
	if (m_iBeams >= VORTIGAUNT_MAX_BEAMS)
		return;

	Vector forward, right, up;
	AngleVectors( GetLocalAngles(), &forward, &right, &up );
	Vector vecSrc = GetLocalOrigin() + up * 36 + right * side * 16 + forward * 32;

	for (int i = 0; i < 3; i++)
	{
		Vector vecAim = forward * random->RandomFloat( -1, 1 ) + right * side * random->RandomFloat( 0, 1 ) + up * random->RandomFloat( -1, 1 );
		trace_t tr1;
		AI_TraceLine ( vecSrc, vecSrc + vecAim * 512, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr1);
		if (flDist > tr1.fraction)
		{
			tr = tr1;
			flDist = tr.fraction;
		}
	}

	// Couldn't find anything close enough
	if ( flDist == 1.0 )
		return;

	UTIL_ImpactTrace( &tr, DMG_CLUB );

	m_pBeam[m_iBeams] = CBeam::BeamCreate( "sprites/lgtning.vmt", 3.0 );
	if (!m_pBeam[m_iBeams])
		return;

	m_pBeam[m_iBeams]->PointEntInit( tr.endpos, this );
	m_pBeam[m_iBeams]->SetEndAttachment( side < 0 ? m_iLeftHandAttachment : m_iRightHandAttachment );
	m_pBeam[m_iBeams]->SetColor( 96, 128, 16 );
	m_pBeam[m_iBeams]->SetBrightness( 64 );
	m_pBeam[m_iBeams]->SetNoise( 12.5 );
	m_iBeams++;
}

//=========================================================
// ZapBeam - heavy damage directly forward
//=========================================================
void CNPC_Vortigaunt::ZapBeam( int side )
{
	Vector vecSrc, vecAim;
	trace_t tr;
	CBaseEntity *pEntity;

	if (m_iBeams >= VORTIGAUNT_MAX_BEAMS)
		return;

	Vector forward, right, up;
	AngleVectors( GetLocalAngles(), &forward, &right, &up );

	vecSrc = GetLocalOrigin() + up * 36;
	vecAim = GetShootEnemyDir( vecSrc );
	float deflection = 0.01;
	vecAim = vecAim + side * right * random->RandomFloat( 0, deflection ) + up * random->RandomFloat( -deflection, deflection );
	AI_TraceLine ( vecSrc, vecSrc + vecAim * 1024, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr);

	m_pBeam[m_iBeams] = CBeam::BeamCreate( "sprites/lgtning.vmt", 5.0 );
	if (!m_pBeam[m_iBeams])
		return;

	m_pBeam[m_iBeams]->PointEntInit( tr.endpos, this );
	m_pBeam[m_iBeams]->SetEndAttachment( side < 0 ? m_iLeftHandAttachment : m_iRightHandAttachment );
	m_pBeam[m_iBeams]->SetColor( 180, 255, 96 );
	m_pBeam[m_iBeams]->SetBrightness( 255 );
	m_pBeam[m_iBeams]->SetNoise( 3 );
	m_iBeams++;

	pEntity = tr.m_pEnt;
	if (pEntity != NULL && m_takedamage)
	{
		CTakeDamageInfo dmgInfo( this, this, sk_vortigaunt_dmg_zap.GetFloat(), DMG_SHOCK );
		dmgInfo.SetDamagePosition( tr.endpos );
		VectorNormalize( vecAim );// not a unit vec yet
		// hit like a 5kg object flying 400 ft/s
		dmgInfo.SetDamageForce( 5 * 400 * 12 * vecAim );
		pEntity->DispatchTraceAttack( dmgInfo, vecAim, &tr );
	}
}

//------------------------------------------------------------------------------
// Purpose : Put glowing sprites on hands
// Input   :
// Output  :
//------------------------------------------------------------------------------
void CNPC_Vortigaunt::StartHandGlow( void )
{
	m_bGlowTurningOn	= true;
	m_fGlowAge			= 0;

	m_fGlowChangeTime = VORTIGAUNT_ZAP_GLOWGROW_TIME;
	m_fGlowScale = 0.8f;

	if ( m_pLeftHandGlow == NULL )
	{
		m_pLeftHandGlow = CSprite::SpriteCreate( "sprites/greenglow1.vmt", GetLocalOrigin(), FALSE );
		m_pLeftHandGlow->SetAttachment( this, m_iLeftHandAttachment );
		m_pLeftHandGlow->SetTransparency( kRenderGlow, 255, 200, 200, 0, kRenderFxNoDissipation );
		m_pLeftHandGlow->SetBrightness( 0 );
		m_pLeftHandGlow->SetScale( 0 );
	}
	
	if ( m_pRightHandGlow == NULL )
	{	
		m_pRightHandGlow = CSprite::SpriteCreate( "sprites/greenglow1.vmt", GetLocalOrigin(), FALSE );
		m_pRightHandGlow->SetAttachment( this, m_iRightHandAttachment );

		m_pRightHandGlow->SetTransparency( kRenderGlow, 255, 200, 200, 0, kRenderFxNoDissipation );
		m_pRightHandGlow->SetBrightness( 0 );
		m_pRightHandGlow->SetScale( 0 );
	}
}

//------------------------------------------------------------------------------
// Purpose : Fade glow from hands
// Input   :
// Output  :
//------------------------------------------------------------------------------
void CNPC_Vortigaunt::EndHandGlow( void )
{
	m_bGlowTurningOn	= false;
	m_fGlowChangeTime = VORTIGAUNT_GLOWFADE_TIME;

	if ( m_pLeftHandGlow )
	{
		m_pLeftHandGlow->SetScale( 0, 0.5f );
		m_pLeftHandGlow->FadeAndDie( 0.5f );
	}

	if ( m_pRightHandGlow )
	{
		m_pRightHandGlow->SetScale( 0, 0.5f );
		m_pRightHandGlow->FadeAndDie( 0.5f );
	}
}

//------------------------------------------------------------------------------
void CNPC_Vortigaunt::ClearHandGlow( )
{
	if (m_pLeftHandGlow)
	{
		UTIL_Remove( m_pLeftHandGlow );
		m_pLeftHandGlow = NULL;
	}
	if (m_pRightHandGlow)
	{
		UTIL_Remove( m_pRightHandGlow );
		m_pRightHandGlow = NULL;
	}
}

//=========================================================
// ClearBeams - remove all beams
//=========================================================
void CNPC_Vortigaunt::ClearBeams( void )
{
	for (int i = 0; i < VORTIGAUNT_MAX_BEAMS; i++)
	{
		if (m_pBeam[i])
		{
			UTIL_Remove( m_pBeam[i] );
			m_pBeam[i] = NULL;
		}
	}

	m_iBeams = 0;
//	m_nSkin = 0;

	if ( m_bStopLoopingSounds )
	{
		StopSound( "NPC_Vortigaunt.StartShootLoop" );
		StopSound( "NPC_Vortigaunt.ZapPowerup" );
		StopSound( "NPC_Vortigaunt.ZapShoot" );
		m_bStopLoopingSounds = false;
	}
}

//------------------------------------------------------------------------------
void CNPC_Vortigaunt::TraceAttack( const CTakeDamageInfo &info1, const Vector &vecDir, trace_t *ptr )
{
	CTakeDamageInfo info = info1;

	if ( (info.GetDamageType() & DMG_SHOCK) && FClassnameIs( info.GetAttacker(), GetClassname() ) )
	{
		// mask off damage from other vorts for now
		info.SetDamage( 0.01 );
	}

	BaseClass::TraceAttack( info, vecDir, ptr, 0 );
}

#ifdef VORT_TELE_STUFF
//------------------------------------------------------------------------------
void CNPC_Vortigaunt::EnemiesForgetAboutMe( void )
{
	ResetVisibilityCache( this ); // this should happen for any NPC that teleports

	CAI_BaseNPC **ppAIs = g_AI_Manager.AccessAIs();
	for ( int i = 0; i < g_AI_Manager.NumAIs(); i++ )
	{
		CAI_BaseNPC *pNPC = ppAIs[i];

		if ( pNPC == this )
			continue;

		if ( !pNPC->IsAlive() )
			continue;

		if ( pNPC->GetEnemy() == this )
		{
			CHOEHuman *pHuman = dynamic_cast<CHOEHuman *>( pNPC );
			if ( pHuman )
				pHuman->EnemyTeleported( this );
		}

//		if ( pNPC->GetEnemies() == NULL )
//			continue;

//		ppAIs[i]->GetEnemies()->ClearMemory( this );
	}
}
#endif // VORT_TELE_STUFF

AI_BEGIN_CUSTOM_NPC( npc_vortigaunt, CNPC_Vortigaunt )

	//=========================================================
	// > SCHED_VORTIGAUNT_RANGE_ATTACK
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_VORTIGAUNT_RANGE_ATTACK,

		"	Tasks"
		"		TASK_STOP_MOVING				0"
		"		TASK_FACE_IDEAL					0"
		"		TASK_RANGE_ATTACK1				0"
		"		TASK_WAIT						0.2" // Wait a sec before killing beams
		""
		"	Interrupts"
	);

#ifdef VORT_TELE_STUFF
	DECLARE_TASK( TASK_VORT_FIND_TELEPORT_ATTACK )
	DECLARE_TASK( TASK_VORT_FIND_TELEPORT_HIDE )
	DECLARE_TASK( TASK_VORT_TELEPORT )

	DEFINE_SCHEDULE
	(
		SCHED_VORTIGAUNT_TELEPORT_HIDE,

		"	Tasks"
		"		TASK_STOP_MOVING				0"
		"		TASK_VORT_FIND_TELEPORT_HIDE	0"
		"		TASK_VORT_TELEPORT	            0"
		""
		"	Interrupts"
	);

	DEFINE_SCHEDULE
	(
		SCHED_VORTIGAUNT_TELEPORT_ATTACK,

		"	Tasks"
		"		TASK_STOP_MOVING				0"
		"		TASK_VORT_FIND_TELEPORT_ATTACK	0"
		"		TASK_VORT_TELEPORT	            0"
		""
		"	Interrupts"
	);
#endif

AI_END_CUSTOM_NPC()
