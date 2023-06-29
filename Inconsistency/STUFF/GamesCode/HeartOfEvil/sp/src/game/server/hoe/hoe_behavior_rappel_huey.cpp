#include "cbase.h"
#include "ai_motor.h"
#include "hoe_behavior_rappel_huey.h"
#include "beam_shared.h"
#include "rope.h"
#include "eventqueue.h"
#include "world.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define INVALID_ATTACHMENT 0

Activity ACT_RAPPEL_JUMP0;
Activity ACT_RAPPEL_JUMP2;

BEGIN_DATADESC( CHueyRappelBehavior )
	DEFINE_FIELD( m_bWaitingToRappel, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bOnGround, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_hLine, FIELD_EHANDLE ),
	DEFINE_FIELD( m_vecHueyRopeAnchor, FIELD_POSITION_VECTOR ),
END_DATADESC();

//=========================================================
//=========================================================
class CHueyRopeAnchor : public CPointEntity
{
	DECLARE_CLASS( CHueyRopeAnchor, CPointEntity );

public:
	void Spawn( void );
	void FallThink( void );
	void RemoveThink( void );
	EHANDLE m_hRope;

	DECLARE_DATADESC();
};

BEGIN_DATADESC( CHueyRopeAnchor )
	DEFINE_FIELD( m_hRope, FIELD_EHANDLE ),

	DEFINE_THINKFUNC( FallThink ),
	DEFINE_THINKFUNC( RemoveThink ),
END_DATADESC();

LINK_ENTITY_TO_CLASS( huey_rope_anchor, CHueyRopeAnchor );

//---------------------------------------------------------
//---------------------------------------------------------
#define RAPPEL_ROPE_WIDTH 1
void CHueyRopeAnchor::Spawn()
{
	BaseClass::Spawn();

	// Decent enough default in case something happens to our owner!
	float flDist = 384;

	if( GetOwnerEntity() )
	{
		flDist = fabs( GetOwnerEntity()->GetAbsOrigin().z - GetAbsOrigin().z );
	}

	m_hRope = CRopeKeyframe::CreateWithSecondPointDetached( this, -1, flDist, RAPPEL_ROPE_WIDTH, "cable/cable.vmt", 5, true );

	ASSERT( m_hRope != NULL );

	SetThink( &CHueyRopeAnchor::FallThink );
	SetNextThink( gpGlobals->curtime + 0.2 );
}

//---------------------------------------------------------
//---------------------------------------------------------
void CHueyRopeAnchor::FallThink()
{
	SetMoveType( MOVETYPE_FLYGRAVITY );

	Vector vecVelocity = GetAbsVelocity();

	vecVelocity.x = random->RandomFloat( -30.0f, 30.0f );
	vecVelocity.y = random->RandomFloat( -30.0f, 30.0f );

	SetAbsVelocity( vecVelocity );

	SetThink( &CHueyRopeAnchor::RemoveThink );
	SetNextThink( gpGlobals->curtime + 3.0 );
}

//---------------------------------------------------------
//---------------------------------------------------------
void CHueyRopeAnchor::RemoveThink()
{
	UTIL_Remove( m_hRope );	
	SetThink( &CHueyRopeAnchor::SUB_Remove );
	SetNextThink( gpGlobals->curtime );
}

#ifdef RAPPEL_PHYSICS

//=========================================================
//=========================================================
class CHueyRappelPhysicsTip : public CBaseAnimating
{
	DECLARE_CLASS( CHueyRappelPhysicsTip, CBaseAnimating );

public:
	DECLARE_DATADESC();

	virtual void Spawn( void );
	virtual void Precache( void );
	virtual void UpdateOnRemove( );
	virtual void VPhysicsUpdate( IPhysicsObject *pPhysics );

//	virtual int	UpdateTransmitState( void );
	void						CreateSpring( CBaseEntity *pRoot, int iAttachment, CBaseEntity *pNPC );
//	static CBarnacleTongueTip	*CreateTongueTip( CNPC_Barnacle *pBarnacle, CBaseAnimating *pTongueRoot, const Vector &vecOrigin, const QAngle &vecAngles );
//	static CBarnacleTongueTip	*CreateTongueRoot( const Vector &vecOrigin, const QAngle &vecAngles );

	IPhysicsSpring			*m_pSpring;
	EHANDLE					m_hNPC;

//	CHandle<CNPC_Barnacle>	m_hBarnacle;
};

BEGIN_DATADESC( CHueyRappelPhysicsTip )
//	DEFINE_FIELD( m_hBarnacle, FIELD_EHANDLE ),
//	DEFINE_PHYSPTR( m_pSpring ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( huey_rappel_physics_tip, CHueyRappelPhysicsTip );

//-----------------------------------------------------------------------------
void CHueyRappelPhysicsTip::Spawn( void )
{
	Precache();
	SetModel( "models/props_junk/rock001a.mdl" );
//	AddEffects( EF_NODRAW );

	// We don't want this to be solid, because we don't want it to collide with the barnacle.
	SetSolid( SOLID_VPHYSICS );
	AddSolidFlags( FSOLID_NOT_SOLID );

	BaseClass::Spawn();

	VPhysicsInitNormal( GetSolid(), GetSolidFlags(), false );

//	m_pSpring = NULL;
}

//-----------------------------------------------------------------------------
void CHueyRappelPhysicsTip::Precache( void )
{
	PrecacheModel( "models/props_junk/rock001a.mdl" );
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
void CHueyRappelPhysicsTip::UpdateOnRemove( void )
{
	if ( m_pSpring )
	{
		physenv->DestroySpring( m_pSpring );
		m_pSpring = NULL;
	}
	BaseClass::UpdateOnRemove();
}

//-----------------------------------------------------------------------------
void CHueyRappelPhysicsTip::VPhysicsUpdate( IPhysicsObject *pPhysics )
{
	BaseClass::VPhysicsUpdate( pPhysics );

	if ( m_hNPC )
	{
		// MOVETYPE_FLY towards the point
		Vector velocity = GetAbsOrigin() - m_hNPC->GetAbsOrigin(), move;
		float interval = ( gpGlobals->frametime > 0.0f ) ? TICK_INTERVAL : 0.0f;
		if ( interval )
		{
			VectorScale( velocity, 1 / interval, move );
			m_hNPC->SetAbsVelocity( move );
		}
	}
}

ConVar hoe_rappel_tip_damping_speed( "hoe_rappel_tip_damping_speed", "0.01" );
ConVar hoe_rappel_damping( "hoe_rappel_damping", "0.01" );
ConVar hoe_rappel_relative_damping( "hoe_rappel_relative_damping", "0.01" );

//-----------------------------------------------------------------------------
void CHueyRappelPhysicsTip::CreateSpring( CBaseEntity *pRootEnt, int iAttachment, CBaseEntity *pNPC )
{
	IPhysicsObject *pRootPhysObject = pRootEnt->VPhysicsGetObject();
	Assert( pRootPhysObject );

	IPhysicsObject *pTipPhysObject = VPhysicsGetObject();
	Assert( pTipPhysObject );
	pTipPhysObject->SetMass( 100 );
	float dampingSpeed = hoe_rappel_tip_damping_speed.GetFloat();;
	float dampingRotation = 10;
	pTipPhysObject->SetDamping( &dampingSpeed, &dampingRotation );
//	pTipPhysObject->EnableMotion( true );
//	pTipPhysObject->EnableGravity( true );
	pTipPhysObject->SetCallbackFlags( pTipPhysObject->GetCallbackFlags() & (~CALLBACK_DO_FLUID_SIMULATION) );

	springparams_t spring;
	spring.constant = 2e5f;
	spring.damping = hoe_rappel_damping.GetFloat();
	spring.relativeDamping = hoe_rappel_relative_damping.GetFloat();
	((CBaseAnimating *)pRootEnt)->GetAttachment( iAttachment, spring.startPosition );
	spring.endPosition = GetAbsOrigin();
	spring.naturalLength = (spring.endPosition - spring.startPosition).Length();
	spring.useLocalPositions = false;
	m_pSpring = physenv->CreateSpring( pRootPhysObject, pTipPhysObject, &spring );

	m_hNPC = pNPC;
}

#endif // RAPPEL_PHYSICS

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CHueyRappelBehavior::CHueyRappelBehavior()
{
	m_hLine = NULL;
	m_bWaitingToRappel = false;
	m_bOnGround = true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CHueyRappelBehavior::KeyValue( const char *szKeyName, const char *szValue )
{
	if ( FStrEq( szKeyName, "waitinghueyrappel" ) )
	{
		m_bWaitingToRappel = ( atoi(szValue) != 0);
		m_bOnGround = !m_bWaitingToRappel;
		return true;
	}

	return BaseClass::KeyValue( szKeyName, szValue );
}

void CHueyRappelBehavior::Precache()
{
	CBaseEntity::PrecacheModel( "cable/cable.vmt" );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#define RAPPEL_MAX_SPEED	196	// FIXME: HL1 was random 128-196
#define RAPPEL_MIN_SPEED	128 // Go no slower than this.
#define RAPPEL_DECEL_DIST	(20.0f * 12.0f)	// Start slowing down when you're this close to the ground.
void CHueyRappelBehavior::SetDescentSpeed()
{
	// Trace to the floor and see how close we're getting. Slow down if we're close.
	// STOP if there's an NPC under us.
	trace_t tr;
	AI_TraceLine( GetOuter()->GetAbsOrigin(), GetOuter()->GetAbsOrigin() - Vector( 0, 0, 8192 ), MASK_SHOT, GetOuter(), COLLISION_GROUP_NONE, &tr );

	float flDist = fabs( GetOuter()->GetAbsOrigin().z - tr.endpos.z );

	float speed = RAPPEL_MAX_SPEED;

	// namb0/name3 drops from an extreme height, fall faster
	if ( flDist > 768.0f )
		speed *= 1.5;

	if ( flDist <= RAPPEL_DECEL_DIST )
	{
		float factor;
		factor = flDist / RAPPEL_DECEL_DIST;

		speed = max( RAPPEL_MIN_SPEED, speed * factor );
	}

#ifdef RAPPEL_PHYSICS
	if ( m_hTipEnt )
	{
		CHueyRappelPhysicsTip *pTip = (CHueyRappelPhysicsTip *)m_hTipEnt.Get();
		if ( pTip->m_pSpring )
		{
			Vector p1, p2;
			pTip->m_pSpring->GetEndpoints( &p1, &p2 );
			float length = (p2 - p1).Length() + speed / 2 * 0.05;
			pTip->m_pSpring->SetSpringLength( length );

			{
				Vector p1, p2;
				pTip->m_pSpring->GetEndpoints( &p1, &p2 );
				NDebugOverlay::Line( p1, p2, 0, 255, 0, true, 1 );
			}
		}
	}
#else
	Vector vecNewVelocity = vec3_origin;
	vecNewVelocity.z = -speed;
	GetOuter()->SetAbsVelocity( vecNewVelocity );
#endif
}


void CHueyRappelBehavior::CleanupOnDeath( CBaseEntity *pCulprit, bool bFireDeathOutput )
{
	BaseClass::CleanupOnDeath( pCulprit, bFireDeathOutput );

	//This will remove the beam and create a rope if the NPC dies while rappeling down.
	if ( m_hLine )
	{
		 CAI_BaseNPC *pNPC = GetOuter();

		 if ( pNPC )
		 {
			 CutZipline();
		 }
	}
#ifdef RAPPEL_PHYSICS
	if ( m_hTipEnt )
	{
		UTIL_Remove( m_hTipEnt );
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pTask - 
//-----------------------------------------------------------------------------
void CHueyRappelBehavior::StartTask( const Task_t *pTask )
{
	switch( pTask->iTask )
	{
	case TASK_MOVE_AWAY_PATH:
		GetOuter()->GetMotor()->SetIdealYaw( UTIL_AngleMod( GetOuter()->GetLocalAngles().y - 180.0f ) );
		BaseClass::StartTask( pTask );
		break;

	case TASK_RANGE_ATTACK1:
		BaseClass::StartTask( pTask );
		break;

	case TASK_HUEY_RAPPEL_DEPLOY:
		GetOuter()->SetIdealActivity( m_iDeployActivity );
		break;

	case TASK_HUEY_RAPPEL:
		{
			CreateZipline();
			SetDescentSpeed();
		}
		break;

	case TASK_HUEY_HIT_GROUND:
		m_bOnGround = true;
#ifdef RAPPEL_PHYSICS
		GetOuter()->SetMoveType( MOVETYPE_STEP );
#endif
		if( GetOuter()->GetGroundEntity() != NULL && GetOuter()->GetGroundEntity()->IsNPC() && GetOuter()->GetGroundEntity()->m_iClassname == GetOuter()->m_iClassname )
		{
			// Although I tried to get NPC's out from under me, I landed on one. Kill it, so long as it's the same type of character as me.
			variant_t val;
			val.SetFloat( 0 );
			g_EventQueue.AddEvent( GetOuter()->GetGroundEntity(), "sethealth", val, 0, GetOuter(), GetOuter() );
		}

		TaskComplete();
		break;

	default:
		BaseClass::StartTask( pTask );
		break;
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pTask - 
//-----------------------------------------------------------------------------
void CHueyRappelBehavior::RunTask( const Task_t *pTask )
{
	switch( pTask->iTask )
	{
	case TASK_FALL_TO_GROUND:
		{
			BaseClass::RunTask( pTask );
			if ( GetOuter()->GetFlags() & FL_ONGROUND )
			{
				GetOuter()->GetShotRegulator()->EnableShooting();
				m_flFallVelocity -= PLAYER_MAX_SAFE_FALL_SPEED;
				float flFallDamage = m_flFallVelocity * DAMAGE_FOR_FALL_SPEED;
				if ( flFallDamage > 0 )
				{
					DevMsg( "CHueyRappelBehavior TASK_FALL_TO_GROUND %.2f damage\n", flFallDamage );
					GetOuter()->TakeDamage( CTakeDamageInfo( GetWorldEntity(), GetWorldEntity(), flFallDamage, DMG_FALL ) );
				}
			}
			else
			{
				m_flFallVelocity = -GetOuter()->GetAbsVelocity().z;
			}
		}
		break;

	case TASK_HUEY_RAPPEL_DEPLOY:
		GetOuter()->AutoMovement( );
		if ( GetOuter()->IsActivityFinished() )
		{
			TaskComplete();
		}
		break;

	case TASK_HUEY_RAPPEL:
		{
			// If we don't do this, the beam won't show up sometimes. Ideally, all beams would update their
			// bboxes correctly, but we're close to shipping and we can't change that now.
			if ( m_hLine )
			{
				m_hLine->RelinkBeam();
			}

			if( GetEnemy() )
			{
				// Face the enemy if there's one.
				Vector vecEnemyLKP = GetEnemyLKP();
				GetOuter()->GetMotor()->SetIdealYawToTargetAndUpdate( vecEnemyLKP );
			}

			SetDescentSpeed();
			if ( GetOuter()->GetFlags() & FL_ONGROUND )
			{
				CBaseEntity *pGroundEnt = GetOuter()->GetGroundEntity();

				if ( pGroundEnt && pGroundEnt->IsPlayer() )
				{
					// try to shove the player in the opposite direction as they are facing (so they'll see me)
					Vector vecForward;
					pGroundEnt->GetVectors( &vecForward, NULL, NULL );
					pGroundEnt->SetAbsVelocity( vecForward * -500 );
					break;
				}

				GetOuter()->m_OnRappelTouchdown.FireOutput( GetOuter(), GetOuter(), 0 );
				GetOuter()->RemoveFlag( FL_FLY );
				
				CutZipline();

#ifdef RAPPEL_PHYSICS
				if ( m_hTipEnt )
					UTIL_Remove( m_hTipEnt );
#endif

				TaskComplete();
			}
			else
			{
				// Stay under the attachment point in case the chopper moves around
				if ( m_hHuey != NULL && m_hHuey->GetBaseAnimating() != NULL &&
					m_iAttachment != INVALID_ATTACHMENT )
				{
					Vector vecAttach;
					if ( m_hHuey->GetBaseAnimating()->GetAttachment( m_iAttachment, vecAttach ) )
					{
#if 1
						vecAttach.z = GetAbsOrigin().z;
						UTIL_SetOrigin( GetOuter(), vecAttach );
#else
						Vector velocity = vecAttach - GetAbsOrigin();
						velocity.z = 0;
						float interval = 0.1;
						Vector move;
						VectorScale( velocity, 1 / interval, move );
						GetOuter()->ApplyAbsVelocityImpulse( move );
#endif
					}
				}
			}
		}
		break;

	default:
		BaseClass::RunTask( pTask );
		break;
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CHueyRappelBehavior::CanSelectSchedule()
{
	if ( !GetOuter()->IsInterruptable() )
		return false;

	if ( m_bWaitingToRappel )
		return true;

	if ( m_bOnGround )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
bool CHueyRappelBehavior::ShouldAlwaysThink()
{
	// The NPC shouldn't stop rappelling if the player leaves PVS
	return !m_bWaitingToRappel && !m_bOnGround;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHueyRappelBehavior::GatherConditions()
{
	BaseClass::GatherConditions();

	// It doesn't seem possible to get this condition while enemy==NULL...
	if ( HasCondition( COND_CAN_RANGE_ATTACK1 ) && GetEnemy() != NULL )
	{
		// Shoot at the enemy so long as I'm six feet or more above them.
		if( (GetAbsOrigin().z - GetEnemy()->GetAbsOrigin().z >= 36.0f) &&
			GetOuter()->GetShotRegulator()->ShouldShoot() )
		{
			Activity activity = GetOuter()->TranslateActivity( ACT_GESTURE_RANGE_ATTACK1 );
			Assert( activity != ACT_INVALID );
			GetOuter()->AddGesture( activity );
			// FIXME: this seems a bit wacked
			GetOuter()->Weapon_SetActivity( GetOuter()->Weapon_TranslateActivity( ACT_RANGE_ATTACK1 ), 0 );

			GetOuter()->OnRangeAttack1();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : int
//-----------------------------------------------------------------------------
int CHueyRappelBehavior::SelectSchedule()
{
	if ( HasCondition( COND_HUEY_BEGIN_RAPPEL ) )
	{
		m_bWaitingToRappel = false;
		return SCHED_HUEY_RAPPEL_DEPLOY;
	}

	if ( HasCondition( COND_HUEY_ABORT_RAPPEL ) )
	{
		m_bWaitingToRappel = false;
		m_bOnGround = true;

		CAI_BaseNPC *pOuter = GetOuter();
		m_flFallVelocity = -pOuter->GetAbsVelocity().z;

		// No shooting while falling
		pOuter->GetShotRegulator()->DisableShooting();

		trace_t tr;
		UTIL_TraceHull( pOuter->GetAbsOrigin(), pOuter->GetAbsOrigin() - Vector(0,0,200), pOuter->GetHullMins(), pOuter->GetHullMaxs(),
			MASK_NPCSOLID, pOuter, COLLISION_GROUP_NONE, &tr );
		if ( tr.fraction == 1.0 )
			return SCHED_HUEY_RAPPEL_FALL_HIGH;
		return SCHED_HUEY_RAPPEL_FALL_LOW;
	}

	if ( m_bWaitingToRappel )
	{
		return SCHED_HUEY_RAPPEL_WAIT;
	}
	else
	{
		return SCHED_HUEY_RAPPEL;
	}
	
	return BaseClass::SelectSchedule();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHueyRappelBehavior::BeginRappel( CBaseEntity *pHuey, int iAttachment, Activity iDeployActivity )
{
	// Send the message to begin rappeling!
	SetCondition( COND_HUEY_BEGIN_RAPPEL );
//	m_bOnGround = false;
//	GetOuter()->AddFlag( FL_FLY );

	m_vecHueyRopeAnchor = GetOuter()->GetAbsOrigin();

	m_hHuey = pHuey;
	m_iAttachment = iAttachment;
	m_iDeployActivity = iDeployActivity;

#ifdef RAPPEL_PHYSICS
	GetOuter()->SetMoveType( MOVETYPE_FLY );
	CHueyRappelPhysicsTip *pTipEnt = (CHueyRappelPhysicsTip *) CBaseEntity::Create( "huey_rappel_physics_tip", GetOuter()->GetAbsOrigin(), GetOuter()->GetAbsAngles() );
	Assert( pTipEnt );
	pTipEnt->CreateSpring( pHuey, iAttachment, GetOuter() );
	m_hTipEnt = pTipEnt;
#endif

	trace_t tr;
	UTIL_TraceEntity( GetOuter(), GetAbsOrigin(), GetAbsOrigin()-Vector(0,0,4096), MASK_SHOT, GetOuter(), COLLISION_GROUP_NONE, &tr );

	if ( tr.m_pEnt != NULL && tr.m_pEnt->IsNPC() )
	{
		Vector forward;
		GetOuter()->GetVectors( &forward, NULL, NULL );

		CSoundEnt::InsertSound( SOUND_DANGER, tr.m_pEnt->EarPosition() - forward * 12.0f, 32.0f, 0.2f, GetOuter() );
	}
}

//-----------------------------------------------------------------------------
void CHueyRappelBehavior::AbortRappel( void )
{
	if ( m_bOnGround )
		return;

	// The huey was destroyed or shoved violently (ie hit by rpg)
	SetCondition( COND_HUEY_ABORT_RAPPEL );

	CutZipline();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHueyRappelBehavior::CutZipline()
{
	if( m_hLine )
	{
		UTIL_Remove( m_hLine );
	}

	CBaseEntity *pAnchor = CreateEntityByName( "huey_rope_anchor" );
	pAnchor->SetOwnerEntity( GetOuter() ); // Boy, this is a hack!!
	pAnchor->SetAbsOrigin( m_vecHueyRopeAnchor );
	pAnchor->Spawn();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHueyRappelBehavior::CreateZipline()
{
	if ( !m_hLine && m_hHuey != NULL )
	{
		int attachment = GetOuter()->LookupAttachment( "lefthand" );

		if ( attachment != INVALID_ATTACHMENT )
		{
			CBeam *pBeam;
			pBeam = CBeam::BeamCreate( "cable/cable.vmt", 1 );
			pBeam->SetColor( 150, 150, 150 );
			pBeam->SetWidth( 0.3 );
			pBeam->SetEndWidth( 0.3 );

			CAI_BaseNPC *pNPC = GetOuter();
			Assert( m_hHuey != NULL );
			pBeam->EntsInit( m_hHuey, pNPC );

			pBeam->SetStartAttachment( m_iAttachment );
			pBeam->SetEndAttachment( attachment );

			m_hLine.Set( pBeam );
		}
	}
}


AI_BEGIN_CUSTOM_SCHEDULE_PROVIDER( CHueyRappelBehavior )

	DECLARE_ACTIVITY( ACT_RAPPEL_JUMP0 )
	DECLARE_ACTIVITY( ACT_RAPPEL_JUMP2 )

	DECLARE_TASK( TASK_HUEY_RAPPEL_DEPLOY )
	DECLARE_TASK( TASK_HUEY_RAPPEL )
	DECLARE_TASK( TASK_HUEY_HIT_GROUND )

	DECLARE_CONDITION( COND_HUEY_BEGIN_RAPPEL )
	DECLARE_CONDITION( COND_HUEY_ABORT_RAPPEL )

	//===============================================
	//===============================================
	DEFINE_SCHEDULE
	(
		SCHED_HUEY_RAPPEL_WAIT,

		"	Tasks"
		"		TASK_SET_ACTIVITY				ACTIVITY:ACT_RAPPEL_LOOP"
		"		TASK_WAIT_INDEFINITE			0"
		""
		"	Interrupts"
		"		COND_HUEY_BEGIN_RAPPEL"
	);

	//===============================================
	//===============================================
	DEFINE_SCHEDULE
	(
		SCHED_HUEY_RAPPEL_DEPLOY,

		"	Tasks"
		"		TASK_HUEY_RAPPEL_DEPLOY			0"
		""
		"	Interrupts"
		"		COND_HUEY_ABORT_RAPPEL" // Chopper died or was violently shoved
	);

	//===============================================
	//===============================================
	DEFINE_SCHEDULE
	(
		SCHED_HUEY_RAPPEL,

		"	Tasks"
		"		TASK_SET_ACTIVITY		ACTIVITY:ACT_RAPPEL_LOOP"
		"		TASK_HUEY_RAPPEL		0"
		"		TASK_SET_SCHEDULE		SCHEDULE:SCHED_HUEY_CLEAR_RAPPEL_POINT"
		""
		"	Interrupts"
		""
		"		COND_NEW_ENEMY"	// Only so the enemy selection code will pick an enemy!
		"		COND_HUEY_ABORT_RAPPEL" // Chopper died or was violently shoved
	);

	//===============================================
	//===============================================
	DEFINE_SCHEDULE
	(
		SCHED_HUEY_CLEAR_RAPPEL_POINT,

		"	Tasks"
		"		TASK_HUEY_HIT_GROUND	0"
		"		TASK_MOVE_AWAY_PATH		128"	// Clear this spot for other rappellers
		"		TASK_RUN_PATH			0"
		"		TASK_WAIT_FOR_MOVEMENT	0"
		""
		"	Interrupts"
		""
	);

	//===============================================
	//===============================================
	DEFINE_SCHEDULE
	(
		SCHED_HUEY_RAPPEL_FALL_HIGH,

		"	Tasks"
		"		TASK_PLAY_SEQUENCE				ACTIVITY:ACT_BARNACLE_RELEASED"
		"		TASK_FALL_TO_GROUND				0"
		"		TASK_PLAY_SEQUENCE				ACTIVITY:ACT_BARNACLE_HIT_GROUND"
		"		TASK_SUGGEST_STATE				STATE:IDLE"
		""
		"	Interrupts"
	);

	//===============================================
	//===============================================
	DEFINE_SCHEDULE
	(
		SCHED_HUEY_RAPPEL_FALL_LOW,

		"	Tasks"
		"		TASK_PLAY_SEQUENCE				ACTIVITY:ACT_BARNACLE_RELEASED"
		"		TASK_FALL_TO_GROUND				0"
		"		TASK_SUGGEST_STATE				STATE:IDLE"
		""
		"	Interrupts"
	);

AI_END_CUSTOM_SCHEDULE_PROVIDER()
