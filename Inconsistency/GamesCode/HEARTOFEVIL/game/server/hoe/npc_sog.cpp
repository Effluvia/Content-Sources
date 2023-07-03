#include "cbase.h"
#include "hoe_human.h"
#include "hoe_corpse.h"
#include "schedule_hacks.h"

#define SOG_LASER
#ifdef SOG_LASER
#include "beam_shared.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define SOG_HEALTH 80 // 60, 80, 100
#define SOG_MODEL "models/sog/sog.mdl"

//=====================
// BodyGroups
//=====================

enum 
{
	SOG_BODYGROUP_HEAD = 0,
	SOG_BODYGROUP_TORSO,
//	SOG_BODYGROUP_WEAPON,
};

#define SOG_NUM_HEADS 5

// Make sure this doesn't conflict with base class
#define bits_MEMORY_SOG_INVISIBLE bits_MEMORY_CUSTOM2

#define SOG_MIN_AMBUSH_RANGE 384
#define SOG_DIST_TOO_FAR 2048.0

// When this is set to zero a SOG that spawns hidden doesn't render when it becomes visible for the
// first time.
// I think this is because kRenderNone is set when alpha == zero and the client doesn't recognize
// properly that the rendermode changes when the SOG later becomes visible.
#define SOG_INVISIBLE_ALPHA 1

#ifdef SOG_LASER
// If the last time I fired at someone was between 0 and this many seconds, draw
// a bead on them much faster. (use subsequent paint time)
#define SNIPER_FASTER_ATTACK_PERIOD		3.0f

// How long to aim at someone before shooting them.
#define SNIPER_DEFAULT_PAINT_ENEMY_TIME			1.0f
// ...plus this
#define	SNIPER_DEFAULT_PAINT_NPC_TIME_NOISE		0.75f

#define SNIPER_SUBSEQUENT_PAINT_TIME	( ( IsXbox() ) ? 1.0f : 0.4f )
#endif // SOG_LASER

class CNPC_SOG : public CHOEHuman
{
public:
	DECLARE_CLASS( CNPC_SOG, CHOEHuman );
//	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
	DEFINE_CUSTOM_AI;

	void Spawn( void );
	void Precache( void );
	void SelectModel();
	void SelectModelGroups( void );

	void Activate( void );

	Class_T Classify( void );
	bool ClassifyPlayerAlly( void ) { return false; }
	bool ClassifyPlayerAllyVital( void ) { return false; }

	int GetHeadGroupNum( void ) { return SOG_BODYGROUP_HEAD; }
	int GetNumHeads( void ) { return SOG_NUM_HEADS; }
	const char *GetHeadModelName( void ) { return "models/sog/head_off.mdl"; }

	virtual const char *GetHelmetModelName( void ) { return "models/sog/helmet_beret.mdl"; }
	virtual bool DropHelmet( void );

	virtual SpeechManagerID_t GetSpeechManagerID( void ) { return SPEECH_MANAGER_GRUNT; }
	const char *GetSentenceGroup( void ) const { return "HG"; }
	const char **GetFriendClasses( void ) const {
		static const char *szClasses[] = {
			"npc_sog",
			NULL
		};
		return szClasses;
	};
	const char **GetWeaponClasses( void ) const {
		static const char *szClasses[] = {
			"weapon_m21",
			NULL
		};
		return szClasses;
	};

	const char *GetCCImageNameCStr( void )
	{
		return UTIL_VarArgs( "cc_sog%d", m_nHeadNum + 1 );
	}

	void Touch( CBaseEntity *pOther );

	void GatherEnemyConditions( CBaseEntity *pEnemy );
	int SelectScheduleAlert( void );
	int SelectScheduleCombat( void );
	int SelectScheduleIdle( void );
	int SelectScheduleIdleOrAlert( void );
	int TranslateSchedule( int scheduleType );
	void StartTask( const Task_t *pTask );
	void RunTask( const Task_t *pTask );

	bool CanFlinch( void );

	void Event_Killed( const CTakeDamageInfo &info );
	void UpdateOnRemove( void );

	void SetAlpha( byte alpha );
	virtual bool CanBeSeenBy( CAI_BaseNPC *pNPC ); // allows entities to be 'invisible' to NPC senses.

	bool m_bStartHidden;

#ifdef SOG_LASER
	void PrescheduleThink( void );
	void OnScheduleChange( void );

	void AimGun( void );

	Vector GetBulletOrigin( void );
	Vector GetLaserDirection( void );
	float GetBulletSpeed( void ) { return 6000; }

	bool IsFastSniper() { return false; }

	Vector DesiredBodyTarget( CBaseEntity *pTarget );
	Vector LeadTarget( CBaseEntity *pTarget );

	void LaserOn( const Vector &vecTarget, const Vector &vecDeviance );
	void LaserOff( void );
	float GetPositionParameter( float flTime, bool fLinear );
	void GetPaintAim( const Vector &vecStart, const Vector &vecGoal, float flParameter, Vector *pProgress );
	void PaintTarget( const Vector &vecTarget, float flPaintTime );

	/// This is the variable from which m_flPaintTime gets set.
	/// How long to aim at someone before shooting them.
	float m_flKeyfieldPaintTime;

	/// A random number from 0 to this is added to m_flKeyfieldPaintTime
	/// to yield m_flPaintTime's initial delay.
	float m_flKeyfieldPaintTimeNoise;

	// This keeps track of the last spot the laser painted. For 
	// continuous sweeping that changes direction.
	Vector m_vecPaintCursor;
	float  m_flPaintTime;

	Vector m_vecPaintStart; // used to track where a sweep starts for the purpose of interpolating.

	CBeam *m_pBeam;

	float						m_flTimeLastAttackedPlayer;

	float						m_flTimeLastShotMissed;
	bool						m_bShootZombiesInChest;		///< if true, do not try to shoot zombies in the headcrab

	int m_iScopeAttachment;
#endif // SOG_LASER

	//-----------------------------------------------------------------------------
	// Conditions.
	//-----------------------------------------------------------------------------
	enum 
	{
		COND_SOG_CAN_AMBUSH = BaseClass::NEXT_CONDITION,
		COND_SOG_TOUCHED_ENEMY,
		NEXT_CONDITION
	};

	//-----------------------------------------------------------------------------
	// Tasks
	//-----------------------------------------------------------------------------
	enum
	{
		TASK_SOG_FADE_IN = BaseClass::NEXT_TASK,
		TASK_SOG_FADE_OUT,
#ifdef SOG_LASER
		TASK_SOG_PAINT_ENEMY,
#endif
		NEXT_TASK,
	};

	//-----------------------------------------------------------------------------
	// Schedules
	//-----------------------------------------------------------------------------
	enum
	{
		SCHED_SOG_ESTABLISH_AMBUSH_POSITION = BaseClass::NEXT_SCHEDULE,
		SCHED_SOG_FADE_IN,
		SCHED_SOG_FADE_IN_COWER,
		SCHED_SOG_FADE_IN_ESTABLISH_AMBUSH_POSITION,
		SCHED_SOG_FADE_IN_GRENADE_AND_RUN,
		SCHED_SOG_FADE_IN_HIT_AND_RUN,
		SCHED_SOG_FADE_IN_SCRIPT,
		SCHED_SOG_FADE_IN_TAKE_COVER_FROM_BEST_SOUND,
		SCHED_SOG_FADE_IN_TAKE_COVER_FROM_ENEMY,
		SCHED_SOG_FADE_IN_TAKE_COVER_FROM_ORIGIN,
		SCHED_SOG_FADE_OUT,
		SCHED_SOG_FREEZE,
		NEXT_SCHEDULE,
	};
};

LINK_ENTITY_TO_CLASS( npc_sog, CNPC_SOG );

//IMPLEMENT_SERVERCLASS_ST(CNPC_SOG, DT_NPC_HGrunt)
//END_SEND_TABLE()

BEGIN_DATADESC( CNPC_SOG )
	DEFINE_KEYFIELD( m_bStartHidden, FIELD_BOOLEAN, "StartHidden" ),
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_SOG::Spawn( void )
{
	// If spawned by an npc_maker with no weapon specified then give a random weapon
	if ( m_spawnEquipment == NULL_STRING || !Q_strcmp(STRING(m_spawnEquipment), "random") )
	{
		m_spawnEquipment = AllocPooledString( "weapon_m21" );
	}
	else if ( !Q_strcmp(STRING(m_spawnEquipment), "none") )
	{
		m_spawnEquipment = NULL_STRING;
	}

	m_iHealth = SOG_HEALTH;

	BaseClass::Spawn();

//	CapabilitiesAdd( bits_CAP_MOVE_SHOOT );

	m_flFieldOfView = VIEW_FIELD_WIDE;
	m_flDistTooFar = SOG_DIST_TOO_FAR;
	SetDistLook( SOG_DIST_TOO_FAR );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_SOG::SelectModel()
{
	SetModelName( AllocPooledString( SOG_MODEL ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_SOG::SelectModelGroups( void )
{
	if ( m_nBody == -1 )
	{
		m_nHeadNum = random->RandomInt( 0, SOG_NUM_HEADS - 1 ); 
	}
	else 
	{
		m_nHeadNum = m_nBody;
	}

	m_nBody = 0;
	SetBodygroup( SOG_BODYGROUP_HEAD, m_nHeadNum );
}

//-----------------------------------------------------------------------------
bool CNPC_SOG::DropHelmet( void )
{
	SetBodygroup( SOG_BODYGROUP_HEAD, m_nHeadNum + SOG_NUM_HEADS + 1 );
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Precaches all resources this monster needs.
//-----------------------------------------------------------------------------
void CNPC_SOG::Precache( void )
{
	PrecacheScriptSound( "NPC_SOG.FootstepLeft" );
	PrecacheScriptSound( "NPC_SOG.FootstepRight" );
	PrecacheScriptSound( "NPC_SOG.RunFootstepLeft" );
	PrecacheScriptSound( "NPC_SOG.RunFootstepRight" );

	PrecacheScriptSound( "NPC_SOG.FadeIn" );
	PrecacheScriptSound( "NPC_SOG.FadeOut" );

#ifdef SOG_LASER
	PrecacheModel("effects/bluelaser1.vmt");	
#endif

	PrecacheModel( "models/SOG/helmet_beret.mdl" );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
void CNPC_SOG::Activate( void )
{
	BaseClass::Activate();

	if ( m_bStartHidden )
	{
		SetAlpha( SOG_INVISIBLE_ALPHA );
		Remember( bits_MEMORY_SOG_INVISIBLE );
		SetSchedule( SCHED_SOG_FREEZE );

		m_bStartHidden = false;
	}

#ifdef SOG_LASER
	m_flKeyfieldPaintTime = SNIPER_DEFAULT_PAINT_ENEMY_TIME; 
	m_flKeyfieldPaintTimeNoise = SNIPER_DEFAULT_PAINT_NPC_TIME_NOISE;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Indicates this monster's place in the relationship table.
// Output : 
//-----------------------------------------------------------------------------
Class_T	CNPC_SOG::Classify( void )
{
	return CLASS_SOG;
}

//------------------------------------------------------------------------------
void CNPC_SOG::Touch( CBaseEntity *pOther )
{
	BaseClass::Touch( pOther );

	if ( IRelationType( pOther ) == D_HT )
		SetCondition( COND_SOG_TOUCHED_ENEMY );
}

//---------------------------------------------------------
void CNPC_SOG::GatherEnemyConditions( CBaseEntity *pEnemy )
{
	BaseClass::GatherEnemyConditions( pEnemy );

	ClearCondition( COND_SOG_CAN_AMBUSH );
	if ( HasMemory( bits_MEMORY_SOG_INVISIBLE ) )
	{
		float flDistEnemy = ( pEnemy->GetAbsOrigin() - GetAbsOrigin() ).Length();
		if ( HasCondition( COND_SEE_ENEMY ) && !HasCondition( COND_ENEMY_FACING_ME ) &&
			flDistEnemy > SOG_MIN_AMBUSH_RANGE )
		{
			SetCondition( COND_SOG_CAN_AMBUSH );
		}
	}
}

//-----------------------------------------------------------------------------
int CNPC_SOG::SelectScheduleCombat( void )
{
	if ( HasCondition( COND_ENEMY_DEAD ) )
		return COND_NONE;

	if ( HasMemory( bits_MEMORY_SOG_INVISIBLE ) )
	{
		// Important squad command to carry out
		int sched = SelectScheduleFromSquadCommand();
		if (sched != SCHED_NONE)
			return sched;

		if ( HasCondition( COND_LIGHT_DAMAGE ) || HasCondition( COND_HEAVY_DAMAGE ) )	// Damaged?  Run away
		{
			return SCHED_SOG_FADE_IN_TAKE_COVER_FROM_ENEMY;
		}
		else if ( HasCondition( COND_NO_PRIMARY_AMMO ) )	// No Ammo ? Fade in and deal with it then
		{
			return SCHED_SOG_FADE_IN;
		}
		else if ( HasCondition( COND_SOG_CAN_AMBUSH ) )	// Enemy is facing away from me and not too close?
		{
			if ( HasCondition( COND_CAN_RANGE_ATTACK1 ) &&
				!GetShotRegulator()->IsInRestInterval() &&
				!HasCondition( COND_WEAPON_PLAYER_IN_SPREAD ) &&
				!HasCondition( COND_WEAPON_BLOCKED_BY_FRIEND ) &&
				!HasCondition( COND_WEAPON_SIGHT_OCCLUDED ) &&
				OccupyStrategySlotRange( SQUAD_SLOT_HUMAN_ENGAGE1, SQUAD_SLOT_HUMAN_ENGAGE2 ) )
			{
				return SCHED_SOG_FADE_IN_HIT_AND_RUN;	// Try normal attack
			}
			else if ( HasCondition( COND_CAN_RANGE_ATTACK2 ) && OccupyStrategySlotRange( SQUAD_SLOT_HUMAN_GRENADE1, SQUAD_SLOT_HUMAN_GRENADE2 ) )
			{
				return SCHED_SOG_FADE_IN_GRENADE_AND_RUN; // Try grenade attack
			}
			else
			{
				return SCHED_SOG_FADE_IN;	// Just fade in and deal with it then
			}
		}
		else if ( HasCondition( COND_ENEMY_TOO_FAR ) )	// Enemy has gotten a long way away
		{
			return SCHED_SOG_FADE_IN_ESTABLISH_AMBUSH_POSITION; // Get closer but still in cover
		}
		else if ( HasCondition( COND_SOG_TOUCHED_ENEMY ) )	// Enemy touched me
		{
			ClearCondition( COND_SOG_TOUCHED_ENEMY );
			return SCHED_SOG_FADE_IN;	// Fade in and deal with it
		}
		else
		{
			return SCHED_SOG_FADE_OUT;
		}
	}
	// Visible
	else if ( HasCondition( COND_LIGHT_DAMAGE ) ||
		HasCondition( COND_HEAVY_DAMAGE ) ||
		HasCondition( COND_NO_PRIMARY_AMMO ) ||
		HasCondition( COND_ENEMY_DEAD ) ||
		HasCondition( COND_NEW_ENEMY ) )	// These conditions take precedence over enemy occluded
	{
		return BaseClass::SelectScheduleCombat();
	}

	else
	{
		if ( HasCondition( COND_ENEMY_OCCLUDED ) )
		{
			int sched = SelectScheduleFromSquadCommand();
			if (sched != SCHED_NONE)
				return sched;

			if ( HasCondition( COND_CAN_RANGE_ATTACK2 ) && OccupyStrategySlotRange( SQUAD_SLOT_HUMAN_GRENADE1, SQUAD_SLOT_HUMAN_GRENADE2 ) )
			{
				return SCHED_RANGE_ATTACK2;
			}
			else if ( HasCondition( COND_ENEMY_TOO_FAR ) )
			{
				return SCHED_SOG_ESTABLISH_AMBUSH_POSITION; // Get closer but still in cover
			}
			else
			{
				return SCHED_SOG_FADE_OUT;
			}
		}
	}

	return BaseClass::SelectScheduleCombat();
}


//-----------------------------------------------------------------------------
int CNPC_SOG::SelectScheduleIdle( void )
{
	return SelectScheduleIdleOrAlert();
}

//-----------------------------------------------------------------------------
int CNPC_SOG::SelectScheduleAlert( void )
{
	return SelectScheduleIdleOrAlert();
}

//-----------------------------------------------------------------------------
int CNPC_SOG::SelectScheduleIdleOrAlert( void )
{
	// Invisible
	if ( HasMemory( bits_MEMORY_SOG_INVISIBLE ) )
	{
		if ( HasCondition( COND_LIGHT_DAMAGE ) || HasCondition( COND_HEAVY_DAMAGE ) )	// Damaged?  Run away
		{
			return SCHED_SOG_FADE_IN_TAKE_COVER_FROM_ORIGIN;
		}
		else if ( HasCondition( COND_NO_PRIMARY_AMMO ) )	// No Ammo ? Fade in and deal with it then
		{
			return SCHED_SOG_FADE_IN;
		}
		else if ( HasCondition( COND_SOG_TOUCHED_ENEMY ) )	// Enemy touched me
		{
			ClearCondition( COND_SOG_TOUCHED_ENEMY );
			return SCHED_SOG_FADE_IN_TAKE_COVER_FROM_ORIGIN; // Fade in and run away
		}
		else
		{
			if ( HasCondition( COND_ENEMY_TOO_FAR ) ) // If I have no enemy then he can't be too far away
			{
				ClearCondition( COND_ENEMY_TOO_FAR );
			}

			return SCHED_SOG_FADE_OUT;
		}
	}
	else if ( HasCondition( COND_NO_PRIMARY_AMMO ) )
	{
		return SCHED_RELOAD;
	}
	else
	{
		return SCHED_SOG_FADE_OUT;
	}
}

//-----------------------------------------------------------------------------
int CNPC_SOG::TranslateSchedule( int scheduleType )
{
	switch( scheduleType )
	{
	case SCHED_FAIL:		// This means I will freeze if I can't find cover - even if I am in full view
	case SCHED_IDLE_STAND:
	case SCHED_ALERT_STAND:
	case SCHED_STANDOFF:
	case SCHED_SOG_FADE_OUT:
		return SCHED_SOG_FADE_OUT;
		break;

	case SCHED_RANGE_ATTACK1:
		return SCHED_SOG_FADE_IN_HIT_AND_RUN;
		break;

	case SCHED_RANGE_ATTACK2:
		return SCHED_SOG_FADE_IN_GRENADE_AND_RUN;
		break;

	case SCHED_SOG_ESTABLISH_AMBUSH_POSITION:
		if ( HasMemory( bits_MEMORY_SOG_INVISIBLE ) )
			return SCHED_SOG_FADE_IN_ESTABLISH_AMBUSH_POSITION;
		return scheduleType;
		break;

	case SCHED_TAKE_COVER_FROM_BEST_SOUND:
		if ( HasMemory( bits_MEMORY_SOG_INVISIBLE ) )
			return SCHED_SOG_FADE_IN_TAKE_COVER_FROM_BEST_SOUND;
		break;

	case SCHED_SOG_FREEZE:
	case SCHED_SOG_FADE_IN:
	case SCHED_SOG_FADE_IN_SCRIPT:
	case SCHED_SOG_FADE_IN_COWER:
	case SCHED_SOG_FADE_IN_HIT_AND_RUN:
	case SCHED_SOG_FADE_IN_GRENADE_AND_RUN:
	case SCHED_SOG_FADE_IN_TAKE_COVER_FROM_ENEMY:
	case SCHED_SOG_FADE_IN_TAKE_COVER_FROM_BEST_SOUND:
	case SCHED_SOG_FADE_IN_TAKE_COVER_FROM_ORIGIN:
	case SCHED_SOG_FADE_IN_ESTABLISH_AMBUSH_POSITION:
		return scheduleType;
		break;

	default:
		if ( HasMemory( bits_MEMORY_SOG_INVISIBLE ) )
		{
			DevMsg( "Invisible SOG trying to change to unknown schedule\n" );
			EmitSound( "NPC_SOG.FadeIn" );
			SetAlpha( 255 );
		}
		break;
	}

	return BaseClass::TranslateSchedule( scheduleType );
}

//-----------------------------------------------------------------------------
void CNPC_SOG::StartTask( const Task_t *pTask )
{
	switch( pTask->iTask )
	{
	case TASK_SOG_FADE_IN:
		if ( GetRenderColor().a == 255 )
			TaskComplete();
		else
			EmitSound( "NPC_SOG.FadeIn" );
		break;
	case TASK_SOG_FADE_OUT:
		if ( GetRenderColor().a <= SOG_INVISIBLE_ALPHA )
			TaskComplete();
		else
			EmitSound( "NPC_SOG.FadeOut" );
		break;
#ifdef SOG_LASER
	case TASK_SOG_PAINT_ENEMY:
		{
			// The closer the enemy is the more accurate our initial aim is and
			// the less time we take to aim
			float flDistToEnemy = (GetEnemyLKP() - GetAbsOrigin()).Length();
			float frac = flDistToEnemy / 1024.0f;
			frac = clamp( frac, 0.0f, 1.0f );
//			DevMsg( "sog sniper fraction %.2f\n", frac );

			if ( GetEnemy()->IsPlayer() )
			{
				float delay = 0;

				if ( gpGlobals->curtime - m_flTimeLastAttackedPlayer <= SNIPER_FASTER_ATTACK_PERIOD )
				{
					SetWait( SNIPER_SUBSEQUENT_PAINT_TIME + delay );
					m_flPaintTime = SNIPER_SUBSEQUENT_PAINT_TIME + delay;
				}
				else
				{
					SetWait( m_flKeyfieldPaintTime + delay );
					m_flPaintTime = m_flKeyfieldPaintTime + delay;
				}
			}
			else
			{
				m_flPaintTime = m_flKeyfieldPaintTimeNoise > 0									 ? 
					m_flKeyfieldPaintTime + random->RandomFloat( 0, m_flKeyfieldPaintTimeNoise ) :
					m_flKeyfieldPaintTime
				;

				if ( IsFastSniper() )
				{
					// Get the shot off a little faster.
					m_flPaintTime *= 0.75f;
				}

				SetWait( m_flPaintTime * frac );
			}

			// Try to start the laser where the player can't miss seeing it!
			Vector vecCursor;
			AngleVectors( GetEnemy()->GetLocalAngles(), &vecCursor );
			vecCursor = vecCursor * 300 * frac;
			vecCursor += GetEnemy()->EyePosition();				
			LaserOn( vecCursor, Vector( 16, 16, 16 ) );

			// Add in some TASK_FACE_ENEMY
			Vector vecEnemyLKP = vecCursor;
			if ( !FInAimCone( vecEnemyLKP ) )
			{
				GetMotor()->SetIdealYawToTarget( vecEnemyLKP );
				GetMotor()->SetIdealYaw( CalcReasonableFacing( true ) ); // CalcReasonableFacing() is based on previously set ideal yaw

				// Don't let the activity change from ACT_IDLE_ANGRY
//				SetTurnActivity(); 
			}

			AddLookTarget( GetEnemy(), 1.0, m_flPaintTime, 0.2 );
		}
		break;
#endif // SOG_LASER
	default:
		BaseClass::StartTask( pTask );
		break;
	}
}

//-----------------------------------------------------------------------------
void CNPC_SOG::RunTask( const Task_t *pTask )
{
	switch( pTask->iTask )
	{
	case TASK_SOG_FADE_IN:
		if ( GetRenderColor().a < 255 )
			SetAlpha( min( GetRenderColor().a + 50, 255 ) );
		else
			TaskComplete();
		break;
	case TASK_SOG_FADE_OUT:
		if ( GetRenderColor().a > SOG_INVISIBLE_ALPHA )
			SetAlpha( max( GetRenderColor().a - 10, SOG_INVISIBLE_ALPHA ) );
		else
			TaskComplete();
		break;
#ifdef SOG_LASER
	case TASK_SOG_PAINT_ENEMY:
		if ( IsWaitFinished() )
		{
			LaserOff();
			TaskComplete();
			break;
		}
		PaintTarget( LeadTarget( GetEnemy() ), m_flPaintTime );

		// Add in some TASK_FACE_ENEMY
		if ( m_pBeam )
		{
			Vector vecEnemyLKP = m_vecPaintCursor;
			if ( !FInAimCone( vecEnemyLKP ) )
			{
				GetMotor()->SetIdealYawToTarget( vecEnemyLKP );
				GetMotor()->SetIdealYaw( CalcReasonableFacing( true ) ); // CalcReasonableFacing() is based on previously set ideal yaw
			}
			GetMotor()->UpdateYaw();
		}
		break;
#endif // SOG_LASER
	default:
		BaseClass::RunTask( pTask );
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Turn off flinching under certain circumstances
//-----------------------------------------------------------------------------
bool CNPC_SOG::CanFlinch( void )
{
//	if ( IsActiveDynamicInteraction() )
//		return false;

	if ( IsPlayingGesture( ACT_GESTURE_RANGE_ATTACK1 ) )
		return false;

#ifdef SOG_LASER
	// No flinching while painting else the laser goes awry
	if ( m_pBeam != NULL )
		return false;
#endif // SOG_LASER

	return BaseClass::CanFlinch();
}

//-----------------------------------------------------------------------------
void CNPC_SOG::Event_Killed( const CTakeDamageInfo &info )
{
	if ( GetRenderColor().a < 255 )
	{
		SetAlpha( 255 );
		EmitSound( "NPC_SOG.FadeIn" );
	}

#ifdef SOG_LASER
	LaserOff();
#endif

	BaseClass::Event_Killed( info );
}

//---------------------------------------------------------
void CNPC_SOG::UpdateOnRemove( void )
{
#ifdef SOG_LASER
	LaserOff();
#endif
	BaseClass::UpdateOnRemove();
}

//-----------------------------------------------------------------------------
void CNPC_SOG::SetAlpha( byte alpha )
{
	byte oldAlpha = GetRenderColor().a;
	if ( oldAlpha == alpha )
		return;

	SetRenderColorA( alpha );

	if ( alpha == 255 )
		SetRenderMode( kRenderNormal );
	else if ( alpha > 0 )
		SetRenderMode( kRenderTransTexture );
	else
		SetRenderMode( kRenderNone );

	if ( alpha <= SOG_INVISIBLE_ALPHA && oldAlpha > SOG_INVISIBLE_ALPHA )
	{
		AddFlag( FL_NOTARGET );
		AddEffects( EF_NOSHADOW | EF_NORECEIVESHADOW );
	}
	if ( alpha > SOG_INVISIBLE_ALPHA && oldAlpha <= SOG_INVISIBLE_ALPHA )
	{
		RemoveFlag( FL_NOTARGET );
		RemoveEffects( EF_NOSHADOW | EF_NORECEIVESHADOW );
	}

	if ( GetActiveWeapon() != NULL )
	{
		GetActiveWeapon()->SetRenderColorA( alpha );
		GetActiveWeapon()->SetRenderMode( GetRenderMode() );
	}
}

//-----------------------------------------------------------------------------
bool CNPC_SOG::CanBeSeenBy( CAI_BaseNPC *pNPC )
{
	if ( GetRenderColor().a <= SOG_INVISIBLE_ALPHA )
		return false;

	return BaseClass::CanBeSeenBy( pNPC );
}

#ifdef SOG_LASER
//-----------------------------------------------------------------------------
void CNPC_SOG::PrescheduleThink( void )
{
	BaseClass::PrescheduleThink();

	// Think faster if the beam is on, this gives the beam higher resolution.
	if ( m_pBeam )
	{
		SetNextThink( gpGlobals->curtime + 0.03 );
	}
	else
	{
		SetNextThink( gpGlobals->curtime + 0.1f );
	}
}

//-----------------------------------------------------------------------------
void CNPC_SOG::OnScheduleChange( void )
{
	LaserOff();

	BaseClass::OnScheduleChange();
}

//-----------------------------------------------------------------------------
void CNPC_SOG::AimGun( void )
{
	if ( m_pBeam )
	{
		Vector vecShootOrigin = Weapon_ShootPosition();
		Vector vecShootDir = m_vecPaintCursor - vecShootOrigin;
		VectorNormalize( vecShootDir );
		SetAim( vecShootDir );
	}
	else if (GetEnemy())
	{
		Vector vecShootOrigin;

		vecShootOrigin = Weapon_ShootPosition();
		Vector vecShootDir = GetShootEnemyDir( vecShootOrigin, false );

		SetAim( vecShootDir );
	}
	else
	{
		RelaxAim( );
	}
}

//-----------------------------------------------------------------------------
Vector CNPC_SOG::GetBulletOrigin( void )
{
	CBaseCombatWeapon *pWeapon = GetActiveWeapon();
	if ( !pWeapon ) return WorldSpaceCenter();
	Vector origin;
	pWeapon->GetAttachment( "scope", origin );
	return origin;
}

//-----------------------------------------------------------------------------
Vector CNPC_SOG::GetLaserDirection( void )
{
	CBaseCombatWeapon *pWeapon = GetActiveWeapon();
	if ( !pWeapon ) return WorldSpaceCenter();
	Vector origin;
	QAngle angles;
	pWeapon->GetAttachment( "scope", origin, angles );
	Vector forward;
	AngleVectors( angles, &forward );
	return forward;
}

//---------------------------------------------------------
Vector CNPC_SOG::DesiredBodyTarget( CBaseEntity *pTarget )
{
	// By default, aim for the center
	Vector vecTarget = pTarget->WorldSpaceCenter();

	float flTimeSinceLastMiss = gpGlobals->curtime - m_flTimeLastShotMissed;

	if( pTarget->GetFlags() & FL_CLIENT )
	{
		if( !BaseClass::FVisible( vecTarget ) )
		{
			// go to the player's eyes if his center is concealed.
			// Bump up an inch so the player's not looking straight down a beam.
			vecTarget = pTarget->EyePosition() + Vector( 0, 0, 1 );
		}
	}
	else
	{
		if( pTarget->Classify() == CLASS_HEADCRAB )
		{
			// Headcrabs are tiny inside their boxes.
			vecTarget = pTarget->GetAbsOrigin();
			vecTarget.z += 4.0;
		}
		else if( !m_bShootZombiesInChest && pTarget->Classify() == CLASS_ZOMBIE )
		{
			if( flTimeSinceLastMiss > 0.0f && flTimeSinceLastMiss < 4.0f && hl2_episodic.GetBool() )
			{
				vecTarget = pTarget->BodyTarget( GetBulletOrigin(), false );
			}
			else
			{
				// Shoot zombies in the headcrab
				vecTarget = pTarget->HeadTarget( GetBulletOrigin() );
			}
		}
		else if( pTarget->Classify() == CLASS_ANTLION )
		{
			// Shoot about a few inches above the origin. This makes it easy to hit antlions
			// even if they are on their backs.
			vecTarget = pTarget->GetAbsOrigin();
			vecTarget.z += 18.0f;
		}
		else if( pTarget->Classify() == CLASS_EARTH_FAUNA )
		{
			// Shoot birds in the center
		}
		else
		{
			// Shoot NPCs in the chest
			vecTarget.z += 8.0f;
		}
	}

	return vecTarget;
}

//---------------------------------------------------------
//---------------------------------------------------------
Vector CNPC_SOG::LeadTarget( CBaseEntity *pTarget )
{
	float targetTime;
	float targetDist;
	//float adjustedShotDist;
	//float actualShotDist;
	Vector vecAdjustedShot;
	Vector vecTarget;
	trace_t tr;

	if( pTarget == NULL )
	{
		// no target
		return vec3_origin;
	}

	// Get target
	vecTarget = DesiredBodyTarget( pTarget );

	// Get bullet time to target
	targetDist = (vecTarget - GetBulletOrigin() ).Length();
	targetTime = targetDist / GetBulletSpeed();
	
	// project target's velocity over that time. 
	Vector vecVelocity = vec3_origin;

	if( pTarget->IsPlayer() || pTarget->Classify() == CLASS_MISSILE )
	{
		// This target is a client, who has an actual velocity.
		vecVelocity = pTarget->GetSmoothedVelocity();

		// Slow the vertical velocity down a lot, or the sniper will
		// lead a jumping player by firing several feet above his head.
		// THIS may affect the sniper hitting a player that's ascending/descending
		// ladders. If so, we'll have to check for the player's ladder flag.
		if( pTarget->GetFlags() & FL_CLIENT )
		{
			vecVelocity.z *= 0.25;
		}
	}
	else
	{
		if( pTarget->MyNPCPointer() && pTarget->MyNPCPointer()->GetNavType() == NAV_FLY )
		{
			// Take a flying monster's velocity directly.
			vecVelocity = pTarget->GetAbsVelocity();
		}
		else
		{
			// Have to build a velocity vector using the character's current groundspeed.
			CBaseAnimating *pAnimating;

			pAnimating = (CBaseAnimating *)pTarget;

			Assert( pAnimating != NULL );

			QAngle vecAngle;
			vecAngle.y = pAnimating->GetSequenceMoveYaw( pAnimating->GetSequence() );
			vecAngle.x = 0;
			vecAngle.z = 0;

			vecAngle.y += pTarget->GetLocalAngles().y;

			AngleVectors( vecAngle, &vecVelocity );

			vecVelocity = vecVelocity * pAnimating->m_flGroundSpeed;
		}
	}
#if 0
	if( m_iMisses > 0 && !FClassnameIs( pTarget, "npc_bullseye" ) )
	{
		// I'm supposed to miss this shot, so aim above the target's head.
		// BUT DON'T miss bullseyes, and don't count the shot.
		vecAdjustedShot = vecTarget; 
		vecAdjustedShot.z += 16;

		m_iMisses--;

		// NDebugOverlay::Cross3D(vecAdjustedShot,12.0f,255,0,0,false,1);

		return vecAdjustedShot;
	}
#endif
	vecAdjustedShot = vecTarget + ( vecVelocity * targetTime );

	// if the adjusted shot falls well short of the target, take the straight shot.
	// it's not very interesting for the bullet to hit something far away from the 
	// target. (for instance, if a sign or ledge or something is between the player
	// and the sniper, and the sniper would hit this object if he tries to lead the player)

	return vecAdjustedShot;
}

//-----------------------------------------------------------------------------
void CNPC_SOG::LaserOff( void )
{
	if ( m_pBeam )
	{
		UTIL_Remove( m_pBeam);
		m_pBeam = NULL;
	}

	SetNextThink( gpGlobals->curtime + 0.1f );
}

//-----------------------------------------------------------------------------
#define LASER_LEAD_DIST	64
void CNPC_SOG::LaserOn( const Vector &vecTarget, const Vector &vecDeviance )
{
	if (!m_pBeam)
	{
		m_pBeam = CBeam::BeamCreate( "effects/bluelaser1.vmt", 1.0f );
		m_pBeam->SetColor( 0, 100, 255 );
	}
	else
	{
		// Beam seems to be on.
		//return;
	}

	// Don't aim right at the guy right now.
	Vector vecInitialAim;

	if( vecDeviance == vec3_origin )
	{
		// Start the aim where it last left off!
		vecInitialAim = m_vecPaintCursor;
	}
	else
	{
		vecInitialAim = vecTarget;
	}

	vecInitialAim.x += random->RandomFloat( -vecDeviance.x, vecDeviance.x );
	vecInitialAim.y += random->RandomFloat( -vecDeviance.y, vecDeviance.y );
	vecInitialAim.z += random->RandomFloat( -vecDeviance.z, vecDeviance.z );
	
	// The beam is backwards, sortof. The endpoint is the sniper. This is
	// so that the beam can be tapered to very thin where it emits from the sniper.
	m_pBeam->PointsInit( vecInitialAim, GetBulletOrigin() );
	m_pBeam->SetBrightness( 255 );
	m_pBeam->SetNoise( 0 );
	m_pBeam->SetWidth( 1.0f );
	m_pBeam->SetEndWidth( 1.0f /*was 0*/ );
	m_pBeam->SetScrollRate( 0 );
	m_pBeam->SetFadeLength( 0 );
//	m_pBeam->SetHaloTexture( sHaloSprite );
//	m_pBeam->SetHaloScale( 4.0f );

	m_vecPaintStart = vecInitialAim;

	// Think faster whilst painting. Higher resolution on the 
	// beam movement.
	SetNextThink( gpGlobals->curtime + 0.02 );
}

//-----------------------------------------------------------------------------
// Crikey!
//-----------------------------------------------------------------------------
float CNPC_SOG::GetPositionParameter( float flTime, bool fLinear )
{
	float flElapsedTime;
	float flTimeParameter;

	flElapsedTime = flTime - (GetWaitFinishTime() - gpGlobals->curtime);

	flTimeParameter = ( flElapsedTime / flTime );

	if( fLinear )
	{
		return flTimeParameter;
	}
	else
	{
		return (1 + sin( (M_PI * flTimeParameter) - (M_PI / 2) ) ) / 2;
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_SOG::GetPaintAim( const Vector &vecStart, const Vector &vecGoal, float flParameter, Vector *pProgress )
{
#if 0
	Vector vecDelta;

	vecDelta = vecGoal - vecStart;

	float flDist = VectorNormalize( vecDelta );

	vecDelta = vecStart + vecDelta * (flDist * flParameter);

	vecDelta = (vecDelta - GetBulletOrigin() ).Normalize();

	*pProgress = vecDelta;
#else
	// Quaternions
	Vector vecIdealDir;
	QAngle vecIdealAngles;
	QAngle vecCurrentAngles;
	Vector vecCurrentDir;
	Vector vecBulletOrigin = GetBulletOrigin();

	// vecIdealDir is where the gun should be aimed when the painting
	// time is up. This can be approximate. This is only for drawing the
	// laser, not actually aiming the weapon. A large discrepancy will look
	// bad, though.
	vecIdealDir = vecGoal - vecBulletOrigin;
	VectorNormalize(vecIdealDir);

	// Now turn vecIdealDir into angles!
	VectorAngles( vecIdealDir, vecIdealAngles );

	// This is the vector of the beam's current aim.
	vecCurrentDir = vecStart - vecBulletOrigin;
	VectorNormalize(vecCurrentDir);

	// Turn this to angles, too.
	VectorAngles( vecCurrentDir, vecCurrentAngles );

	Quaternion idealQuat;
	Quaternion currentQuat;
	Quaternion aimQuat;

	AngleQuaternion( vecIdealAngles, idealQuat );
	AngleQuaternion( vecCurrentAngles, currentQuat );

	QuaternionSlerp( currentQuat, idealQuat, flParameter, aimQuat );

	QuaternionAngles( aimQuat, vecCurrentAngles );

	// Rebuild the current aim vector.
	AngleVectors( vecCurrentAngles, &vecCurrentDir );

	*pProgress = vecCurrentDir;
#endif
}

//-----------------------------------------------------------------------------
// Sweep the laser sight towards the point where the gun should be aimed
//-----------------------------------------------------------------------------
void CNPC_SOG::PaintTarget( const Vector &vecTarget, float flPaintTime )
{
	Vector vecCurrentDir;
	Vector vecStart;

	// vecStart is the barrel of the gun (or the laser sight)
	vecStart = GetBulletOrigin();

	float P;

	// keep painttime from hitting 0 exactly.
	flPaintTime = max( flPaintTime, 0.000001f );

	P = GetPositionParameter( flPaintTime, false );
#if 0
	// Vital allies are sharper about avoiding the sniper.
	if( P > 0.25f && GetEnemy() && GetEnemy()->IsNPC() && HasCondition(COND_SEE_ENEMY) && !m_bWarnedTargetEntity )
	{
		m_bWarnedTargetEntity = true;

		if( GetEnemy()->Classify() == CLASS_PLAYER_ALLY_VITAL && GetEnemy()->MyNPCPointer()->FVisible(this) )
		{
			CSoundEnt::InsertSound( SOUND_DANGER | SOUND_CONTEXT_REACT_TO_SOURCE, GetEnemy()->EarPosition(), 16, 1.0f, this );
		}
	}
#endif
	GetPaintAim( m_vecPaintStart, vecTarget, clamp(P,0.0f,1.0f), &vecCurrentDir );

#if 1
#define THRESHOLD 0.8f
	float flNoiseScale;
	
	if ( P >= THRESHOLD )
	{
		flNoiseScale = 1 - (1 / (1 - THRESHOLD)) * ( P - THRESHOLD );
	}
	else if ( P <= 1 - THRESHOLD )
	{
		flNoiseScale = P / (1 - THRESHOLD);
	}
	else
	{
		flNoiseScale = 1;
	}

	// mult by P
	vecCurrentDir.x += flNoiseScale * ( sin( 3 * M_PI * gpGlobals->curtime ) * 0.0006 );
	vecCurrentDir.y += flNoiseScale * ( sin( 2 * M_PI * gpGlobals->curtime + 0.5 * M_PI ) * 0.0006 );
	vecCurrentDir.z += flNoiseScale * ( sin( 1.5 * M_PI * gpGlobals->curtime + M_PI ) * 0.0006 );
#endif

	// Trace to where the beam should end
	trace_t tr;
	UTIL_TraceLine( vecStart, vecStart + vecCurrentDir * 8192, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );
	m_vecPaintCursor = tr.endpos;

	// Allow the rendered laser to be a few degrees off to make up for crummy aim in the animations
	QAngle angCurrent, angScope;
	VectorAngles( vecCurrentDir, angCurrent );
	vecCurrentDir = GetLaserDirection();
	VectorAngles( vecCurrentDir, angScope );

//	DevMsg( "sog sniper angle dx,dy,dz=%.2f,%.2f,%.2f\n", angScope.x-angCurrent.x, angScope.y-angCurrent.y, angScope.z-angCurrent.z );

	angScope.x = UTIL_ApproachAngle( angCurrent.x, angScope.x, 5 );
	angScope.y = UTIL_ApproachAngle( angCurrent.y, angScope.y, 5 );
	angScope.z = UTIL_ApproachAngle( angCurrent.z, angScope.z, 5 );

	AngleVectors( angScope, &vecCurrentDir );
	UTIL_TraceLine( vecStart, vecStart + vecCurrentDir * 8192, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );

	m_pBeam->SetStartPos( tr.endpos );
	m_pBeam->SetEndPos( vecStart );
	m_pBeam->RelinkBeam();

}
#endif // SOG_LASER

HOE_BEGIN_CUSTOM_NPC( npc_sog, CNPC_SOG )

	DECLARE_CONDITION( COND_SOG_CAN_AMBUSH )
	DECLARE_CONDITION( COND_SOG_TOUCHED_ENEMY )

	DECLARE_TASK( TASK_SOG_FADE_IN )
	DECLARE_TASK( TASK_SOG_FADE_OUT )
#ifdef SOG_LASER
	DECLARE_TASK( TASK_SOG_PAINT_ENEMY )
#endif

	DECLARE_SCHEDULE( SCHED_SOG_ESTABLISH_AMBUSH_POSITION )
	DECLARE_SCHEDULE( SCHED_SOG_FADE_IN )
	DECLARE_SCHEDULE( SCHED_SOG_FADE_IN_COWER )
	DECLARE_SCHEDULE( SCHED_SOG_FADE_IN_ESTABLISH_AMBUSH_POSITION )
	DECLARE_SCHEDULE( SCHED_SOG_FADE_IN_GRENADE_AND_RUN )
	DECLARE_SCHEDULE( SCHED_SOG_FADE_IN_HIT_AND_RUN )
	DECLARE_SCHEDULE( SCHED_SOG_FADE_IN_SCRIPT )
	DECLARE_SCHEDULE( SCHED_SOG_FADE_IN_TAKE_COVER_FROM_BEST_SOUND )
	DECLARE_SCHEDULE( SCHED_SOG_FADE_IN_TAKE_COVER_FROM_ENEMY )
	DECLARE_SCHEDULE( SCHED_SOG_FADE_IN_TAKE_COVER_FROM_ORIGIN )
	DECLARE_SCHEDULE( SCHED_SOG_FADE_OUT )
	DECLARE_SCHEDULE( SCHED_SOG_FREEZE )

	LOAD_SCHEDULES_FILE( npc_sog )

HOE_END_CUSTOM_NPC()

//-----------------------------------------------------------------------------

class CDeadSOG : public CHOECorpse
{
public:
	DECLARE_CLASS( CDeadSOG, CHOECorpse );

	void Spawn( void )
	{
		CorpsePose poses[] = {
			{ "lying_on_back", false },
			{ "lying_on_side", false },
			{ "lying_on_stomach", false },
			{ "hanging_byfeet",	true },
			{ "hanging_byarms", true },
			{ "hanging_byneck", true },
			{ "deadsitting", false },
			{ "deadseated", false },
			{ NULL, false }
		};

		SetBloodColor( BLOOD_COLOR_RED );
		InitCorpse( SOG_MODEL, poses );

		BaseClass::Spawn();
	}

	void SelectModelGroups( void )
	{
		int headNum;

		if ( m_nBody == -1 )
		{
			headNum = random->RandomInt( 0, SOG_NUM_HEADS - 1 ); 
		}
		else 
		{
			headNum = m_nBody;
		}

		m_nBody = 0;
		SetBodygroup( SOG_BODYGROUP_HEAD, headNum );
	}
};

LINK_ENTITY_TO_CLASS( npc_sog_dead, CDeadSOG );
