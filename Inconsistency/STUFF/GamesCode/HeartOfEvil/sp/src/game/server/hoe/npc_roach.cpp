#include "cbase.h"
#include "ai_basenpc.h"
#include "npcevent.h"
#include "ai_motor.h"
#include "ai_navigator.h"
#include "ai_senses.h"
#include "hoe_doors.h"
#include "ai_moveprobe.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define		ROACH_IDLE				0
#define		ROACH_BORED				1
#define		ROACH_SCARED_BY_ENT		2
#define		ROACH_SCARED_BY_LIGHT	3
#define		ROACH_SMELL_FOOD		4
#define		ROACH_EAT				5

enum RoachSize_t
{
	ROACH_SIZE_SMALL,
	ROACH_SIZE_MEDIUM,
	ROACH_SIZE_DEFAULT,
};

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
class CNPC_Roach : public CAI_BaseNPC
{
public:
	DECLARE_CLASS( CNPC_Roach, CAI_BaseNPC );
	DECLARE_DATADESC();

	CNPC_Roach() { m_nSize = ROACH_SIZE_DEFAULT; }

	void Spawn( void );
	void Precache( void );

	void OnRestore( void );

	float MaxYawSpeed( void );

	void RoachThink( void );
	void PickNewDest( int iCondition );
	void Look( int iDistance );
	void Move( float flInterval );

	Class_T Classify( void ) { return CLASS_ROACH; }
	bool ClassifyPlayerAlly( void ) { return false; }
	bool ClassifyPlayerAllyVital( void ) { return false; }

	bool ShouldCollide( int collisionGroup, int contentsMask ) const
	{
		return BaseClass::ShouldCollide( collisionGroup, contentsMask );
	}
	void Touch( CBaseEntity *pOther );

	void Event_Killed( const CTakeDamageInfo &info );
	int GetSoundInterests( void );

	void Eat( float flFullDuration );
	bool ShouldEat( void );

//	float StepHeight( void ) const { return 2.0; }

	RoachSize_t m_nSize;

	float   m_flLastLightLevel;
	float   m_flNextSmellTime;

	float	m_flPauseTime;
	bool	m_bMovedLastThink;

	// UNDONE: These don't necessarily need to be save/restored, but if we add more data, it may
	bool    m_fLightHacked;
	int     m_iMode;

	float m_flHungryTime;
};

LINK_ENTITY_TO_CLASS( monster_cockroach, CNPC_Roach );
LINK_ENTITY_TO_CLASS( npc_cockroach, CNPC_Roach );

BEGIN_DATADESC( CNPC_Roach )
	DEFINE_KEYFIELD( m_nSize, FIELD_INTEGER, "RoachSize" ),
	DEFINE_THINKFUNC( RoachThink ),
	DEFINE_FIELD( m_flNextSmellTime, FIELD_TIME ),
	DEFINE_FIELD( m_flPauseTime, FIELD_TIME ),
	DEFINE_FIELD( m_bMovedLastThink, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_fLightHacked, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_iMode, FIELD_INTEGER ),
	DEFINE_FIELD( m_flHungryTime, FIELD_TIME ),
END_DATADESC()


//-----------------------------------------------------------------------------
void CNPC_Roach::Spawn()
{
	Precache();

	switch ( m_nSize )
	{
	case ROACH_SIZE_SMALL:
		SetModel( "models/roach_small.mdl" );
		break;
	case ROACH_SIZE_MEDIUM:
		SetModel( "models/roach_medium.mdl" );
		break;
	default:
		SetModel( "models/roach.mdl" );
		break;
	}

	// If this is only 2 tall these guys navigate through solids
	UTIL_SetSize( this, Vector( -2, -2, 0 ), Vector( 2, 2, 4 ) );

	SetSolid( SOLID_BBOX );
	AddSolidFlags( FSOLID_NOT_STANDABLE );

	SetMoveType( MOVETYPE_STEP );
	CapabilitiesAdd( bits_CAP_MOVE_GROUND );

	SetBloodColor( BLOOD_COLOR_YELLOW );
	m_iHealth = 1;
	m_flFieldOfView = 0.5;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_NPCState = NPC_STATE_IDLE;

	NPCInit();
	SetActivity ( ACT_IDLE );

	SetViewOffset ( Vector ( 0, 0, 1 ) );// position of the eyes relative to monster's origin.

#if 1 // FIXME: how much light is on the roach?
	m_fLightHacked          = true;
	m_flLastLightLevel      = 0;
#else
	m_fLightHacked          = FALSE;
	m_flLastLightLevel      = -1;
#endif
	m_iMode                 = ROACH_IDLE;
	m_flNextSmellTime       = gpGlobals->curtime;

#if 1
	AddEffects( EF_NOSHADOW );
	SetCollisionGroup( COLLISION_GROUP_DEBRIS );
	AddSolidFlags( FSOLID_TRIGGER );
#endif

	SetThink( &CNPC_Roach::RoachThink );
	SetNextThink( gpGlobals->curtime + 0.1 );
}

//-----------------------------------------------------------------------------
void CNPC_Roach::Precache()
{
	PrecacheModel("models/roach.mdl");
	PrecacheModel("models/roach_small.mdl");
	PrecacheModel("models/roach_medium.mdl");

	PrecacheScriptSound("NPC_Roach.Die");
	PrecacheScriptSound("NPC_Roach.Smash");
	PrecacheScriptSound("NPC_Roach.Walk");

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
void CNPC_Roach::OnRestore( void )
{
	// our nav goal is stomped because we have no schedule
	m_iMode = ROACH_IDLE;
	SetIdealActivity( ACT_IDLE );

	BaseClass::OnRestore();
}

//-----------------------------------------------------------------------------
float CNPC_Roach::MaxYawSpeed( void )
{
	return 120.0f;
}

//-----------------------------------------------------------------------------
void CNPC_Roach::Eat( float flFullDuration )
{
	m_flHungryTime = gpGlobals->curtime + flFullDuration;
}

//-----------------------------------------------------------------------------
bool CNPC_Roach::ShouldEat( void )
{
	if ( m_flHungryTime > gpGlobals->curtime )
	{
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
void CNPC_Roach::RoachThink( void  )
{
	if ( !UTIL_FindClientInPVS( edict() ) )
		SetNextThink( gpGlobals->curtime + random->RandomFloat( 1.0f , 1.5f ) );
	else
		SetNextThink( gpGlobals->curtime + 0.1f );// keep monster thinking

	float flInterval = gpGlobals->curtime - GetLastThink();

	StudioFrameAdvance( ); // animate

	if ( !m_fLightHacked )
	{
		// if light value hasn't been collection for the first time yet, 
		// suspend the creature for a second so the world finishes spawning, then we'll collect the light level.
		SetNextThink( gpGlobals->curtime + 1 );
		m_fLightHacked = TRUE;
		return;
	}
	else if ( m_flLastLightLevel < 0 )
	{
		// collect light level for the first time, now that all of the lightmaps in the roach's area have been calculated.
		m_flLastLightLevel = 0;
	}

	if ( m_flPauseTime > gpGlobals->curtime )
		return;

	switch ( m_iMode )
	{
	case ROACH_IDLE:
	case ROACH_EAT:
		{
			// if not moving, sample environment to see if anything scary is around. Do a radius search 'look' at random.
			if ( random->RandomInt( 0, 3 ) == 1 )
			{
				Look( 150 );

				if ( HasCondition( COND_SEE_FEAR ) )
				{
					// if see something scary
					//ALERT ( at_aiconsole, "Scared\n" );
					Eat( 30 +  ( random->RandomInt( 0, 14 ) ) );// roach will ignore food for 30 to 45 seconds
					PickNewDest( ROACH_SCARED_BY_ENT );
					SetActivity ( ACT_WALK );
				}
				else if ( random->RandomInt( 0,149 ) == 1 )
				{
					// if roach doesn't see anything, there's still a chance that it will move. (boredom)
					//ALERT ( at_aiconsole, "Bored\n" );
					PickNewDest( ROACH_BORED );
					SetActivity ( ACT_WALK );

					if ( m_iMode == ROACH_EAT )
					{
						// roach will ignore food for 30 to 45 seconds if it got bored while eating. 
						Eat( 30 +  ( random->RandomInt(0,14) ) );
					}
				}
			}

			// don't do this stuff if eating!
			if ( m_iMode == ROACH_IDLE )
			{
				if ( ShouldEat() )
				{
					 GetSenses()->Listen();
				}

				if ( 0 > m_flLastLightLevel )
				{
					// someone turned on lights!
					//ALERT ( at_console, "Lights!\n" );
					PickNewDest( ROACH_SCARED_BY_LIGHT );
					SetActivity ( ACT_WALK );
				}
				else if ( HasCondition( COND_SMELL ) )
				{
#if 1
					CSound *pSound = GetBestScent();
#else
					CSound *pSound = GetLoudestSoundOfType( ALL_SOUNDS );
#endif
					// roach smells food and is just standing around. Go to food unless food isn't on same z-plane.
					if ( pSound && abs( pSound->GetSoundOrigin().z - GetAbsOrigin().z ) <= 3 )
					{
						PickNewDest( ROACH_SMELL_FOOD );
						SetActivity ( ACT_WALK );
					}
				}
			}

			break;
		}
	case ROACH_SCARED_BY_LIGHT:
		{
			// if roach was scared by light, then stop if we're over a spot at least as dark as where we started!
			if ( 0 <= m_flLastLightLevel )
			{
				SetActivity ( ACT_IDLE );
				m_flLastLightLevel = 0;// make this our new light level.
			}
			break;
		}
	}

	MaintainActivity();

	if ( GetActivity() != ACT_IDLE && GetNavigator()->IsGoalActive() )
	{
		Move( flInterval );
	}
}

void CNPC_Roach::PickNewDest( int iCondition )
{
	Vector  vecNewDir;
	Vector  vecDest;
//	float   flDist;

	m_iMode = iCondition;

    GetNavigator()->ClearGoal();

	if ( m_iMode == ROACH_SMELL_FOOD )
	{
		// find the food and go there.
#if 1
		CSound *pSound = GetBestScent();
		if ( pSound )
		{
			AI_NavGoal_t goal( GOALTYPE_LOCATION, pSound->GetSoundOrigin(), ACT_WALK );
			GetNavigator()->SetGoal( goal );
			return;
		}
#else
		CSound *pSound = GetLoudestSoundOfType( ALL_SOUNDS );

		if ( pSound )
		{
			GetNavigator()->SetRandomGoal( 3 - random->RandomInt( 0,5 ) );
			return;
		}
#endif
	}

#if 1
	GetNavigator()->SetWanderGoal( 256, 512 );
#else
	do 
	{
		// picks a random spot, requiring that it be at least 128 units away
		// else, the roach will pick a spot too close to itself and run in 
		// circles. this is a hack but buys me time to work on the real monsters.
#if 1
		vecNewDir.x = random->RandomFloat( -1, 1 );
		vecNewDir.y = random->RandomFloat( -1, 1 );
#else
		vecNewDir.x = random->RandomInt( -1, 1 );
		vecNewDir.y = random->RandomInt( -1, 1 );
#endif
		flDist          = 256 + ( random->RandomInt(0,255) );
		vecDest = GetAbsOrigin() + vecNewDir * flDist;

	} while ( ( vecDest - GetAbsOrigin() ).Length2D() < 128 );

	Vector vecLocation;

	vecLocation.x = vecDest.x;
	vecLocation.y = vecDest.y;
	vecLocation.z = GetAbsOrigin().z;
	
	AI_NavGoal_t goal( GOALTYPE_LOCATION, vecLocation, ACT_WALK );

	GetNavigator()->SetGoal( goal );
#endif
	if ( random->RandomInt( 0, 9 ) == 1 )
	{
		// every once in a while, a roach will play a skitter sound when they decide to run
		EmitSound( "NPC_Roach.Walk" );
	}
}

//=========================================================
// Look - overriden for the roach, which can virtually see 
// 360 degrees.
//=========================================================
void CNPC_Roach::Look( int iDistance )
{
	CBaseEntity     *pSightEnt = NULL;// the current visible entity that we're dealing with

	// DON'T let visibility information from last frame sit around!
	ClearCondition( COND_SEE_HATE | COND_SEE_DISLIKE |  COND_SEE_ENEMY | COND_SEE_FEAR );

	// don't let monsters outside of the player's PVS act up, or most of the interesting
	// things will happen before the player gets there!
	if ( !UTIL_FindClientInPVS( edict() ) )
	{
		return;
	}
	
	// Does sphere also limit itself to PVS?
	// Examine all entities within a reasonable radius
	// !!!PERFORMANCE - let's trivially reject the ent list before radius searching!

	for ( CEntitySphereQuery sphere( GetAbsOrigin(), iDistance ); (pSightEnt = sphere.GetCurrentEntity()) != NULL; sphere.NextEntity() )
	{
		// only consider ents that can be damaged. !!!temporarily only considering other monsters and clients
		if (  pSightEnt->IsPlayer() || pSightEnt->IsNPC() )
		{
			if ( /*FVisible( pSightEnt ) &&*/ !FBitSet( pSightEnt->GetFlags(), FL_NOTARGET ) && pSightEnt->m_iHealth > 0 )
			{
#if 1 // FIXME: CLASS_INSECT relationships
				if ( dynamic_cast<CNPC_Roach *>(pSightEnt) == NULL )
					SetCondition( COND_SEE_FEAR );
#else
				// don't add the Enemy's relationship to the conditions. We only want to worry about conditions when
				// we see monsters other than the Enemy.
				switch ( IRelationType ( pSightEnt ) )
				{
				case    D_FR:           
					SetCondition( COND_SEE_FEAR );
					break;
				case    D_NU:
					break;
				default:
//					Msg ( "%s can't asses %s\n", STRING( pev->classname ), STRING( pSightEnt->pev->classname ) );
					break;
				}
#endif
			}
		}
	}
}

//=========================================================
// roach's move function
//=========================================================
void CNPC_Roach::Move( float flInterval ) 
{
	float           flWaypointDist;
	Vector          vecApex;

#if 1
	float flDist = (GetAbsOrigin() - GetNavigator()->GetCurWaypointPos()).Length2D();
	if ( flDist < 2 )
	{
		if ( GetNavigator()->CurWaypointIsGoal() )
		{
			PickNewDest( ROACH_IDLE );
		}
		else
		{
			GetNavigator()->AdvancePath();
		}
	}
#endif

#if 1
//	bool bWalked = false;

	GetMotor()->SetIdealYawAndUpdate( GetNavigator()->GetCurWaypointPos() - GetAbsOrigin() );

	// Don't move till we face the ideal yaw
	if ( !FacingIdeal() )
		return;
#else
	// local move to waypoint.
	flWaypointDist = ( GetNavigator()->GetGoalPos() - GetAbsOrigin() ).Length2D();

	float idealYaw;
	idealYaw = UTIL_VecToYaw( GetNavigator()->GetGoalPos() );
	GetMotor()->SetIdealYaw( idealYaw );
#endif

#if 1
	m_flGroundSpeed = random->RandomFloat( 75.0f, 125.0f );
	Vector vecWalk = GetNavigator()->GetCurWaypointPos() - GetAbsOrigin();
	flDist = vecWalk.Length();
	vecWalk.NormalizeInPlace();
	Vector newPos;
	if ( flDist > m_flGroundSpeed * flInterval )
		newPos = GetAbsOrigin() + vecWalk * m_flGroundSpeed * flInterval;
	else
		newPos = GetAbsOrigin() + vecWalk * flDist;

#if 0 // jun 26 2009
	AIMoveTrace_t directTrace;
	bool bTraceClear = GetMoveProbe()->MoveLimit( NAV_GROUND, GetLocalOrigin(), newPos, 
		MASK_NPCSOLID, NULL/*pMoveGoal->pMoveTarget*/, 
		100.0, 
		AIMLF_2D, 
		&directTrace );
	newPos = directTrace.vEndPosition;
	if ( directTrace.flTotalDist <= 0 || GetMotor()->MoveGroundStep( newPos ) != AIM_SUCCESS )
#else
	if ( GetMotor()->MoveGroundStep( newPos ) != AIM_SUCCESS )
#endif
	{
		if ( m_bMovedLastThink )
		{
			m_flPauseTime = gpGlobals->curtime + random->RandomFloat(1.0,2.5);
			SetActivity ( ACT_IDLE );
		}
		m_bMovedLastThink = false;

		// stuck, so just pick a new spot to run off to
		PickNewDest( ROACH_IDLE );
		return;
	}
	m_bMovedLastThink = true;
	flWaypointDist = ( GetNavigator()->GetGoalPos() - GetAbsOrigin() ).Length2D();
#else
	if ( random->RandomInt( 0,7 ) == 1 )
	{
		// randomly check for blocked path.(more random load balancing)
#if 1
		m_flGroundSpeed = 175.0;
		Vector vecWalk = GetNavigator()->GetGoalPos() - GetAbsOrigin();
		vecWalk.NormalizeInPlace();
#if 1
		Vector newPos = GetAbsOrigin() + vecWalk * m_flGroundSpeed * flInterval;
		if ( GetMotor()->MoveGroundStep( newPos ) != AIM_SUCCESS )
#else
		if ( !WalkMove( vecWalk * m_flGroundSpeed * flInterval, MASK_NPCSOLID ) )
#endif
#else
		if ( !WalkMove( GetNavigator()->GetGoalPos() - GetAbsOrigin(), MASK_NPCSOLID ) )
#endif
		{
			// stuck, so just pick a new spot to run off to
			PickNewDest( m_iMode );

#if 1
			GetMotor()->SetIdealYawAndUpdate( GetNavigator()->GetGoalPos() - GetAbsOrigin() );

			// Don't move till we face the ideal yaw
			if ( !FacingIdeal() )
				return;
		}
		else
		{
			bWalked = true;
#endif
		}
	}

#if 1
	if ( !bWalked )
	{
		m_flGroundSpeed = 175.0;
		Vector vecWalk = GetNavigator()->GetGoalPos() - GetAbsOrigin();
		vecWalk.NormalizeInPlace();
#if 1
		Vector newPos = GetAbsOrigin() + vecWalk * m_flGroundSpeed * flInterval;
		GetMotor()->MoveGroundStep( newPos );
#else
		WalkMove( vecWalk * m_flGroundSpeed * flInterval, MASK_NPCSOLID );
#endif
	}

	// local move to waypoint.
	flWaypointDist = ( GetNavigator()->GetGoalPos() - GetAbsOrigin() ).Length2D();
#else
	WalkMove( (GetNavigator()->GetGoalPos() - GetAbsOrigin()) * flInterval, MASK_NPCSOLID );
#endif
#endif

	// if the waypoint is closer than step size, then stop after next step (ok for roach to overshoot)
	if ( flWaypointDist <= m_flGroundSpeed * flInterval )
	{
		// take truncated step and stop

		SetActivity ( ACT_IDLE );
		m_flLastLightLevel = 0;// this is roach's new comfortable light level

		if ( m_iMode == ROACH_SMELL_FOOD )
		{
			m_iMode = ROACH_EAT;
		}
		else
		{
			m_iMode = ROACH_IDLE;
		}
//		m_flPauseTime = gpGlobals->curtime + random->RandomFloat(3.0,6.0);
	}

	if ( random->RandomInt( 0,149 ) == 1 && m_iMode != ROACH_SCARED_BY_LIGHT && m_iMode != ROACH_SMELL_FOOD )
	{
		// random skitter while moving as long as not on a b-line to get out of light or going to food
		PickNewDest( ROACH_IDLE );
	}
}

void CNPC_Roach::Touch( CBaseEntity *pOther )
{
	IDoor *pDoor = dynamic_cast< IDoor *>(pOther);
	if ( pDoor != NULL )
	{
		 IDoorAccessor *pDoorAccessor = pDoor->GetDoorAccessor();
		 if ( !pDoorAccessor->IsDoorOpening() && !pDoorAccessor->IsDoorClosing() )
			 return;
	}
	else if ( pOther->GetAbsVelocity() == vec3_origin || !pOther->IsPlayer() )
	{
		return;
	}

	Vector          vecSpot;
	trace_t         tr;

	vecSpot = GetAbsOrigin() + Vector ( 0 , 0 , 8 );//move up a bit, and trace down.
	//UTIL_TraceLine ( vecSpot, vecSpot + Vector ( 0, 0, -24 ),  ignore_monsters, ENT(pev), & tr);

	UTIL_TraceLine ( vecSpot, vecSpot + Vector ( 0, 0, -24 ), MASK_ALL, this, COLLISION_GROUP_NONE, &tr);

	// This isn't really blood.  So you don't have to screen it out based on violence levels (UTIL_ShouldShowBlood())
	UTIL_DecalTrace( &tr, "YellowBlood" );

	// DMG_GENERIC because we don't want any physics force generated
	TakeDamage( CTakeDamageInfo( pOther, pOther, m_iHealth, DMG_GENERIC ) );
}

void CNPC_Roach::Event_Killed( const CTakeDamageInfo &info )
{
	RemoveSolidFlags( FSOLID_NOT_SOLID );
		
	//random sound
	if ( random->RandomInt( 0,4 ) == 1 )
	{
		EmitSound( "NPC_Roach.Die" );
	}
	else
	{
		EmitSound( "NPC_Roach.Smash" );
	}
	
	CSoundEnt::InsertSound ( SOUND_WORLD, GetAbsOrigin(), 128, 1 );

	UTIL_Remove( this );
}

int CNPC_Roach::GetSoundInterests ( void) 
{
	return  SOUND_CARCASS    |
			SOUND_MEAT;
}
