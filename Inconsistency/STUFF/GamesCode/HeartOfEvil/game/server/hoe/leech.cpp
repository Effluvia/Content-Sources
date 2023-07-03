#include "cbase.h"
#include "ai_basenpc.h"
#include "npcevent.h"
#include "ai_motor.h"
#include "ai_senses.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Movement constants
#define		LEECH_ACCELERATE		10
#define		LEECH_CHECK_DIST		45
#define		LEECH_SWIM_SPEED		50
#define		LEECH_SWIM_ACCEL		80
#define		LEECH_SWIM_DECEL		10
#define		LEECH_TURN_RATE			90
#define		LEECH_SIZEX				10
#define		LEECH_FRAMETIME			0.1

#define LEECH_HEALTH 2
#define LEECH_DMG_BITE 2

#define AE_LEECH_ATTACK         1
#define AE_LEECH_FLOP           2

class CNPC_Leech : public CAI_BaseNPC
{
public:
	DECLARE_CLASS( CNPC_Leech, CAI_BaseNPC );
	DECLARE_DATADESC();
//	DEFINE_CUSTOM_AI;

	void Spawn( void );
	void Precache( void );
	Class_T Classify( void ) { return CLASS_LEECH; }

	void AlertSound( void );
	void AttackSound( void );

	void Activate( void );
	void RecalculateWaterLevel( void );
	void SwitchLeechState( void );
	Disposition_t IRelationType( CBaseEntity *pTarget );
	void Touch( CBaseEntity *pOther );
	void TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr );
	void HandleAnimEvent( animevent_t *pEvent );
	float ObstacleDistance( CBaseEntity *pTarget );
	void DeadThink( void );
	void UpdateMotion( void );
	void SwimThink( void );
	void Event_Killed( const CTakeDamageInfo &info );
	void RunTask( const Task_t *pTask );
	int DrawDebugTextOverlays(void);



	float	m_flTurning;// is this boid turning?
	bool	m_fPathBlocked;// TRUE if there is an obstacle ahead
	float	m_flAccelerate;
	float	m_obstacle;
	float	m_top;
	float	m_bottom;
	float	m_height;
	float	m_waterTime;
	float	m_sideTime;		// Timer to randomly check clearance on sides
	float	m_zTime;
	float	m_stateTime;
	float	m_attackSoundTime;

	float	m_flSpeed; // FIXME: was pev->speed in HL1
	Vector	m_vecOldOrigin;
};

LINK_ENTITY_TO_CLASS( monster_leech, CNPC_Leech );
LINK_ENTITY_TO_CLASS( npc_leech, CNPC_Leech );

BEGIN_DATADESC( CNPC_Leech ) 
	DEFINE_FIELD( m_flTurning, FIELD_FLOAT ),
	DEFINE_FIELD( m_fPathBlocked, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flAccelerate, FIELD_FLOAT ),
	DEFINE_FIELD( m_obstacle, FIELD_FLOAT ),
	DEFINE_FIELD( m_top, FIELD_FLOAT ),
	DEFINE_FIELD( m_bottom, FIELD_FLOAT ),
	DEFINE_FIELD( m_height, FIELD_FLOAT ),
	DEFINE_FIELD( m_waterTime, FIELD_TIME ),
	DEFINE_FIELD( m_sideTime, FIELD_TIME ),
	DEFINE_FIELD( m_zTime, FIELD_TIME ),
	DEFINE_FIELD( m_stateTime, FIELD_TIME ),
	DEFINE_FIELD( m_attackSoundTime, FIELD_TIME ),

	DEFINE_THINKFUNC( DeadThink ),
	DEFINE_THINKFUNC( SwimThink ),
END_DATADESC()

//-----------------------------------------------------------------------------
void CNPC_Leech::Spawn( void )
{
	BaseClass::Spawn();

	Precache();
	SetModel( STRING( GetModelName() ) );

	SetHullType( HULL_TINY_CENTERED );
	SetHullSizeSmall();
	UTIL_SetSize( this, Vector( -1, -1, 0 ), Vector( 1, 1, 2 ) );

	SetSolid( SOLID_BBOX );
	AddSolidFlags( FSOLID_NOT_STANDABLE ); // you can't stand on this
	SetCollisionGroup( COLLISION_GROUP_DEBRIS );

	SetMoveType( MOVETYPE_FLY );
	AddFlag( FL_SWIM );

	SetBloodColor( DONT_BLEED );
	m_NPCState = NPC_STATE_IDLE;

	m_flFieldOfView = -0.5;
	SetViewOffset( vec3_origin );		// Position of the eyes relative to NPC's origin.

	m_flDistTooFar = 750;
	SetDistLook( 750 );

	m_iHealth = LEECH_HEALTH;

	NPCInit();
	SetActivity( ACT_SWIM );

	m_stateTime = gpGlobals->curtime + random->RandomFloat( 1.0, 5.0 );
	SetThink( &CNPC_Leech::SwimThink );
}

//-----------------------------------------------------------------------------
void CNPC_Leech::Precache( void )
{
	SetModelName( AllocPooledString( "models/leech.mdl" ) );
	PrecacheModel( STRING( GetModelName() ) );

	PrecacheScriptSound("NPC_Leech.Alert");
	PrecacheScriptSound("NPC_Leech.Attack");

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
void CNPC_Leech::Activate( void )
{
	RecalculateWaterLevel();
	BaseClass::Activate();
}

//-----------------------------------------------------------------------------
void CNPC_Leech::RecalculateWaterLevel( void )
{
	// Calculate boundaries
	Vector vecTest = GetAbsOrigin() - Vector(0,0,400);

	trace_t tr;

	UTIL_TraceLine( GetAbsOrigin(), vecTest, MASK_NPCSOLID, this, COLLISION_GROUP_NONE, &tr);
	if ( tr.fraction != 1.0 )
		m_bottom = tr.endpos.z + 1;
	else
		m_bottom = vecTest.z;

	m_top = UTIL_WaterLevel( GetAbsOrigin(), GetAbsOrigin().z, GetAbsOrigin().z + 400 ) - 1;

	// Chop off 20% of the outside range
	float newBottom = m_bottom * 0.8 + m_top * 0.2;
	m_top = m_bottom * 0.2 + m_top * 0.8;
	m_bottom = newBottom;
	m_height = random->RandomFloat( m_bottom, m_top );
	m_waterTime = gpGlobals->curtime + random->RandomFloat( 5, 7 );
}

//-----------------------------------------------------------------------------
void CNPC_Leech::SwitchLeechState( void )
{
	m_stateTime = gpGlobals->curtime + random->RandomFloat( 3, 6 );
	if ( GetState() == NPC_STATE_COMBAT )
	{
		SetEnemy( NULL );
		SetState( NPC_STATE_IDLE );
		// We may be up against the player, so redo the side checks
		m_sideTime = 0;
	}
	else
	{
		GetSenses()->Look( GetSenses()->GetDistLook() );
		CBaseEntity *pEnemy = BestEnemy();
		if ( pEnemy && pEnemy->GetWaterLevel() != 0 /*&& pEnemy->pev->watertype != CONTENT_FOG*/ )
		{
			SetEnemy( pEnemy );
			SetState( NPC_STATE_COMBAT );
			m_stateTime = gpGlobals->curtime + random->RandomFloat( 18, 25 );
			AlertSound();
		}
	}
}

//-----------------------------------------------------------------------------
Disposition_t CNPC_Leech::IRelationType( CBaseEntity *pTarget )
{
	if ( pTarget->IsPlayer() )
		return D_HT;
	return BaseClass::IRelationType( pTarget );
}

//-----------------------------------------------------------------------------
void CNPC_Leech::Touch( CBaseEntity *pOther )
{
	if ( !pOther->IsPlayer() )
		return;

	if ( pOther == GetTouchTrace().m_pEnt )
	{
		if ( pOther->GetAbsVelocity() == vec3_origin )
			return;

		SetBaseVelocity( pOther->GetAbsVelocity() );
		AddFlag( FL_BASEVELOCITY );
	}
}

//-----------------------------------------------------------------------------
void CNPC_Leech::AttackSound( void )
{
	if ( gpGlobals->curtime > m_attackSoundTime )
	{
		EmitSound( "NPC_Leech.Attack" );
		m_attackSoundTime = gpGlobals->curtime + 0.5;
	}
}

//-----------------------------------------------------------------------------
void CNPC_Leech::AlertSound( void )
{
	EmitSound( "NPC_Leech.Alert" );
}

//-----------------------------------------------------------------------------
void CNPC_Leech::TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr )
{
	SetAbsVelocity( vec3_origin );

	// Nudge the leech away from the damage
	if ( info.GetInflictor() )
	{
		Vector velocity = vecDir;
		VectorNormalize( velocity );
		SetAbsVelocity( velocity * 25 );
	}

	BaseClass::TraceAttack( info, vecDir, ptr, 0 );
}

//-----------------------------------------------------------------------------
void CNPC_Leech::HandleAnimEvent( animevent_t *pEvent )
{
	if ( pEvent->event == AE_LEECH_ATTACK )
	{
		CBaseEntity *pEnemy = GetEnemy();
		if ( pEnemy != NULL )
		{
			Vector dir, face;

			AngleVectors( GetAbsAngles(), &face );
			face.z = 0;
			face.NormalizeInPlace();

			dir = pEnemy->GetAbsOrigin() - GetAbsOrigin();
			dir.z = 0;
			dir.NormalizeInPlace();
			
			if ( DotProduct(dir, face) > 0.9 )		// Only take damage if the leech is facing the prey
			{
				AttackSound();
				CTakeDamageInfo info( this, this, LEECH_DMG_BITE, DMG_SLASH );
				CalculateMeleeDamageForce( &info, dir, pEnemy->GetAbsOrigin() );
				pEnemy->TakeDamage( info );
			}
		}
		m_stateTime -= 2;
		return;
	}

	if ( pEvent->event == AE_LEECH_FLOP )
	{
		// Play flop sound
		// NO-OP in HL1 either
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

//
// ObstacleDistance - returns normalized distance to obstacle
//
//-----------------------------------------------------------------------------
float CNPC_Leech::ObstacleDistance( CBaseEntity *pTarget )
{
	trace_t		tr;
	Vector		vecTest;

	// use VELOCITY, not angles, not all boids point the direction they are flying
	//Vector vecDir = UTIL_VecToAngles( pev->velocity );
	Vector forward, right, up;
	AngleVectors( GetAbsAngles(), &forward, &right, &up );

	// check for obstacle ahead
	vecTest = GetAbsOrigin() + forward * LEECH_CHECK_DIST;
	UTIL_TraceLine( GetAbsOrigin(), vecTest, MASK_NPCSOLID, this, COLLISION_GROUP_NONE, &tr );

	if ( tr.startsolid )
	{
		m_flSpeed = -LEECH_SWIM_SPEED * 0.5;
//		ALERT( at_console, "Stuck from (%f %f %f) to (%f %f %f)\n", pev->oldorigin.x, pev->oldorigin.y, pev->oldorigin.z, pev->origin.x, pev->origin.y, pev->origin.z );
//		UTIL_SetOrigin( pev, pev->oldorigin );
	}

	if ( tr.fraction != 1.0 )
	{
//		NDebugOverlay::Line( GetAbsOrigin(), tr.endpos, 255,0,0, true, 0.1 );
		if ( (pTarget == NULL || tr.m_pEnt != pTarget) )
		{
			return tr.fraction;
		}
		else
		{
			if ( fabs(m_height - GetAbsOrigin().z) > 10 )
				return tr.fraction;
		}
	}

	if ( m_sideTime < gpGlobals->curtime )
	{
		// extra wide checks
		vecTest = GetAbsOrigin() + right * LEECH_SIZEX * 2 + forward * LEECH_CHECK_DIST;
		UTIL_TraceLine( GetAbsOrigin(), vecTest, MASK_NPCSOLID, this, COLLISION_GROUP_NONE, &tr );
		if ( tr.fraction != 1.0 )
			return tr.fraction;

		vecTest = GetAbsOrigin() - right * LEECH_SIZEX * 2 + forward * LEECH_CHECK_DIST;
		UTIL_TraceLine( GetAbsOrigin(), vecTest, MASK_NPCSOLID, this, COLLISION_GROUP_NONE, &tr );
		if ( tr.fraction != 1.0 )
			return tr.fraction;

		// Didn't hit either side, so stop testing for another 0.5 - 1 seconds
		m_sideTime = gpGlobals->curtime + random->RandomFloat(0.5,1);
	}
	return 1.0;
}

//-----------------------------------------------------------------------------
void CNPC_Leech::DeadThink( void )
{
	if ( IsActivityFinished() )
	{
		if ( GetActivity() == ACT_DIEFORWARD )
		{
			SetThink( NULL );
			StopAnimation();
			return;
		}
		else if ( GetFlags() & FL_ONGROUND )
		{
			AddSolidFlags( FSOLID_NOT_SOLID );
			SetActivity( ACT_DIEFORWARD );
		}
	}
	StudioFrameAdvance();
	SetNextThink( gpGlobals->curtime + 0.1 );

	// Apply damage velocity, but keep out of the walls
	Vector velocity = GetAbsVelocity();
	if ( velocity.x != 0 || velocity.y != 0 )
	{
		trace_t tr;

		// Look 0.5 seconds ahead
		UTIL_TraceLine( GetAbsOrigin(), GetAbsOrigin() + velocity * 0.5, MASK_NPCSOLID, this, COLLISION_GROUP_NONE, &tr);
		if ( tr.fraction != 1.0 )
		{
			velocity.x = 0;
			velocity.y = 0;
			SetAbsVelocity( velocity );
		}
	}
}

//-----------------------------------------------------------------------------
void CNPC_Leech::UpdateMotion( void )
{
	float flapspeed = (m_flSpeed - m_flAccelerate) / LEECH_ACCELERATE;
	m_flAccelerate = m_flAccelerate * 0.8 + m_flSpeed * 0.2;

	if (flapspeed < 0) 
		flapspeed = -flapspeed;
	flapspeed += 1.0;
	if (flapspeed < 0.5) 
		flapspeed = 0.5;
	if (flapspeed > 1.9) 
		flapspeed = 1.9;

	m_flPlaybackRate = flapspeed;

	QAngle avelocity = GetLocalAngularVelocity();
	if ( !m_fPathBlocked )
		avelocity.y = GetMotor()->GetIdealYaw();
	else
		avelocity.y = GetMotor()->GetIdealYaw() * m_obstacle;

	if ( avelocity.y > 150 )
		SetIdealActivity( ACT_TURN_LEFT );
	else if ( avelocity.y < -150 )
		SetIdealActivity( ACT_TURN_RIGHT );
	else
		SetIdealActivity( ACT_SWIM );

	// lean
	float targetPitch, delta;
	delta = m_height - GetAbsOrigin().z;

#if 1
	if ( delta < 0 )
		targetPitch = 10;
	else if ( delta > 0 )
		targetPitch = -10;
	else
		targetPitch = 0;
#else
	if ( delta < -10 )
		targetPitch = -30;
	else if ( delta > 10 )
		targetPitch = 30;
	else
		targetPitch = 0;
#endif

	QAngle angles = GetAbsAngles();
	angles.x = UTIL_Approach( targetPitch, angles.x, 60 * LEECH_FRAMETIME );
	SetAbsAngles( angles );

	// bank
	avelocity.z = - (angles.z + (avelocity.y * 0.25));

SetLocalAngularVelocity( avelocity );

	if ( GetState() == NPC_STATE_COMBAT && HasCondition( COND_CAN_MELEE_ATTACK1 ) )
		SetIdealActivity( ACT_MELEE_ATTACK1 );

	// Out of water check
	if ( !GetWaterLevel() /* || pev->watertype == CONTENT_FOG*/ )
	{
		SetMoveType( MOVETYPE_FLYGRAVITY );
		SetIdealActivity( ACT_HOP );
		SetAbsVelocity( vec3_origin );

		// Animation will intersect the floor if either of these is non-zero
		angles = GetAbsAngles();
		angles.z = 0;
		angles.x = 0;
		SetAbsAngles( angles );

		if ( m_flPlaybackRate < 1.0 )
			m_flPlaybackRate = 1.0;
	}
	else if ( GetMoveType() == MOVETYPE_FLYGRAVITY )
	{
		SetMoveType( MOVETYPE_FLY );
		RemoveFlag( FL_ONGROUND );
		RecalculateWaterLevel();
		m_waterTime = gpGlobals->curtime + 2;	// Recalc again soon, water may be rising
	}

	if ( GetActivity() != GetIdealActivity() )
	{
		SetActivity ( GetIdealActivity() );
	}

	StudioFrameAdvance();
	DispatchAnimEvents( this );
}

//-----------------------------------------------------------------------------
void CNPC_Leech::SwimThink( void )
{
	trace_t			tr;
	float			flLeftSide;
	float			flRightSide;
	float			targetSpeed;
	float			targetYaw = 0;
	CBaseEntity		*pTarget;

#if 0
	// FIXME: This returns NULL when it shouldn't, all the leeches gets
	// stuck on one wall...
	if ( !UTIL_FindClientInPVS( edict() ) )
	{
		SetNextThink( gpGlobals->curtime + random->RandomFloat(1,1.5) );
		SetAbsVelocity( vec3_origin );
		return;
	}
	else
#endif
		SetNextThink( gpGlobals->curtime + 0.1 );

	targetSpeed = LEECH_SWIM_SPEED;

	if ( m_waterTime < gpGlobals->curtime )
		RecalculateWaterLevel();

	if ( m_stateTime < gpGlobals->curtime )
		SwitchLeechState();

	ClearCondition( COND_CAN_MELEE_ATTACK1 );
	switch ( GetState() )
	{
	case NPC_STATE_COMBAT:
		pTarget = GetEnemy();
		if ( !pTarget )
			SwitchLeechState();
		else
		{
			// Chase the enemy's eyes
			m_height = pTarget->EyePosition().z - 5;
			// Clip to viable water area
			if ( m_height < m_bottom )
				m_height = m_bottom;
			else if ( m_height > m_top )
				m_height = m_top;
			Vector location = pTarget->EyePosition() - GetAbsOrigin();
			if ( location.Length() < 40 )
				SetCondition( COND_CAN_MELEE_ATTACK1 );
			// Turn towards target ent
			targetYaw = UTIL_VecToYaw( location );

			targetYaw = UTIL_AngleDiff( targetYaw, UTIL_AngleMod( GetAbsAngles().y ) );

			if ( targetYaw < (-LEECH_TURN_RATE*0.75) )
				targetYaw = (-LEECH_TURN_RATE*0.75);
			else if ( targetYaw > (LEECH_TURN_RATE*0.75) )
				targetYaw = (LEECH_TURN_RATE*0.75);
			else
				targetSpeed *= 2;
		}

		break;

	default:
		if ( m_zTime < gpGlobals->curtime )
		{
			float newHeight = random->RandomFloat( m_bottom, m_top );
			m_height = 0.5 * m_height + 0.5 * newHeight;
			m_zTime = gpGlobals->curtime + random->RandomFloat( 1, 4 );
		}
		if ( random->RandomInt( 0, 100 ) < 10 )
			targetYaw = random->RandomInt( -30, 30 );
		pTarget = NULL;
		// oldorigin test
		if ( (GetAbsOrigin() - m_vecOldOrigin).Length() < 1 )
		{
			// If leech didn't move, there must be something blocking it, so try to turn
			m_sideTime = 0;

#if 0
			for ( int i = 0; i < 360; i += 10 )
			{
				Vector vecTest = GetAbsOrigin() + (UTIL_YawToVector(i) * LEECH_CHECK_DIST);
				UTIL_TraceLine( GetAbsOrigin(), vecTest, MASK_NPCSOLID, this, COLLISION_GROUP_NONE, &tr);
				if ( tr.fraction == 1.0 )
				{
					targetYaw = i;
					DevMsg( "CNPC_Leech::SwimThink picking unblocked yaw %d\n", i );
					break;
				}
			}
#endif
		}

		break;
	}

	m_obstacle = ObstacleDistance( pTarget );
	m_vecOldOrigin = GetAbsOrigin();
	if ( m_obstacle < 0.1 )
		m_obstacle = 0.1;

	Vector forward, right;
	AngleVectors( GetAbsAngles(), &forward, &right, NULL );

	// is the way ahead clear?
	if ( m_obstacle == 1.0 )
	{
		// if the leech is turning, stop the trend.
		if ( m_flTurning != 0 )
		{
			m_flTurning = 0;
		}

		m_fPathBlocked = FALSE;
		m_flSpeed = UTIL_Approach( targetSpeed, m_flSpeed, LEECH_SWIM_ACCEL * LEECH_FRAMETIME );
		SetAbsVelocity( forward * m_flSpeed );
	}
	else
	{
		m_obstacle = 1.0 / m_obstacle;
		// IF we get this far in the function, the leader's path is blocked!
		m_fPathBlocked = TRUE;

		if ( m_flTurning == 0 )// something in the way and leech is not already turning to avoid
		{
			Vector vecTest;
			// measure clearance on left and right to pick the best dir to turn
			vecTest = GetAbsOrigin() + (right * LEECH_SIZEX) + (forward * LEECH_CHECK_DIST);
			UTIL_TraceLine( GetAbsOrigin(), vecTest, MASK_NPCSOLID, this, COLLISION_GROUP_NONE, &tr);
			flRightSide = tr.fraction;

			vecTest = GetAbsOrigin() + (right * -LEECH_SIZEX) + (forward * LEECH_CHECK_DIST);
			UTIL_TraceLine( GetAbsOrigin(), vecTest, MASK_NPCSOLID, this, COLLISION_GROUP_NONE, &tr);
			flLeftSide = tr.fraction;

			// turn left, right or random depending on clearance ratio
			float delta = (flRightSide - flLeftSide);
			if ( delta > 0.1 || (delta > -0.1 && random->RandomInt(0,100)<50) )
				m_flTurning = -LEECH_TURN_RATE;
			else
				m_flTurning = LEECH_TURN_RATE;
		}
		m_flSpeed = UTIL_Approach( -(LEECH_SWIM_SPEED*0.5), m_flSpeed, LEECH_SWIM_DECEL * LEECH_FRAMETIME * m_obstacle );
		SetAbsVelocity( forward * m_flSpeed );
	}
	GetMotor()->SetIdealYaw( m_flTurning + targetYaw );
	UpdateMotion();
}

//-----------------------------------------------------------------------------
void CNPC_Leech::Event_Killed( const CTakeDamageInfo &info )
{
	Vector		vecSplatDir;
	trace_t		tr;

	//ALERT(at_aiconsole, "Leech: killed\n");
	// tell owner ( if any ) that we're dead.This is mostly for MonsterMaker functionality.
	CBaseEntity *pOwner = GetOwnerEntity();
	if ( pOwner )
		pOwner->DeathNotice( this );

	// When we hit the ground, play the "death_end" activity
	if ( GetWaterLevel() /* && pev->watertype != CONTENT_FOG */ )
	{
		SetAbsAngles( QAngle(0,GetAbsAngles().y,0) );
		SetAbsOrigin( GetAbsOrigin() + Vector(0,0,1) );
		if ( random->RandomInt( 0, 99 ) < 70 )
			SetLocalAngularVelocity( QAngle(0,random->RandomInt( -720, 720 ),0) );
		else
			SetLocalAngularVelocity( vec3_angle );

		SetGravity( 0.02 );
		RemoveFlag( FL_ONGROUND );
		SetActivity( ACT_DIESIMPLE );
	}
	else
		SetActivity( ACT_DIEFORWARD );
	
	SetMoveType( MOVETYPE_FLYGRAVITY );
	m_takedamage = DAMAGE_NO;
	SetThink( &CNPC_Leech::DeadThink );
}

//-----------------------------------------------------------------------------
void CNPC_Leech::RunTask( const Task_t *pTask )
{
	switch (pTask->iTask)
	{
		case TASK_DIE:
		{
			// BaseNPC::Event_Killed
			//    BaseNPC::CleanupOnDeath
			// BaseNPC::SelectDeadSchedule
			//    BaseNPC::CleanupOnDeath
			if ( IsActivityFinished() )
			{
				SetCycle(1.0); // seems to get stuck at 0.999
				//m_iHealth = 0;
				AddSolidFlags( FSOLID_NOT_SOLID );
				//SetState( NPC_STATE_DEAD );
				BaseClass::RunTask(pTask);
			}
		}
		break;

		default:
		{
			BaseClass::RunTask( pTask );
		}
		break;
	}
}

//-----------------------------------------------------------------------------
int CNPC_Leech::DrawDebugTextOverlays(void)
{
	int text_offset = 0;

	text_offset = BaseClass::DrawDebugTextOverlays();

	if (m_debugOverlays & OVERLAY_TEXT_BIT)
	{
		CFmtStr fmt;
		EntityText(text_offset++, fmt.sprintf("water:    %.2f", (float)GetWaterLevel()), 0);
		EntityText(text_offset++, fmt.sprintf("top:      %.2f", (float)m_top), 0);
		EntityText(text_offset++, fmt.sprintf("bottom:   %.2f", (float)m_bottom), 0);
		EntityText(text_offset++, fmt.sprintf("height:   %.2f", (float)m_height), 0);
		EntityText(text_offset++, fmt.sprintf("origin.z: %.2f", (float)GetAbsOrigin().z), 0);
	}

	float x = GetAbsOrigin().x;
	float y = GetAbsOrigin().y;
//	float z = GetAbsOrigin().z;
	NDebugOverlay::VertArrow( GetAbsOrigin(), Vector( x, y, m_top ), 4.0f, 0, 255, 0, 0, true, 0.1f );
	NDebugOverlay::VertArrow( GetAbsOrigin(), Vector( x, y, m_bottom ), 4.0f, 0, 0, 255, 0, true, 0.1f );

	NDebugOverlay::Cross3D( Vector(x,y,m_height), Vector(-3,-3,-3), Vector(3,3,3), 255,0,0, true, 0.1f );

	return text_offset;
}
