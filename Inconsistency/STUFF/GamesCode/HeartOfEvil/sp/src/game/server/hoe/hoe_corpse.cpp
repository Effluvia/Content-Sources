#include "cbase.h"
#include "ai_basenpc.h"
#include "hoe_corpse.h"
#include "datacache/imdlcache.h"
#include "physics_prop_ragdoll.h"
#ifdef CORPSE_ROPES
#include "vphysics/collision_set.h"
#include "physics_saverestore.h"
#include "phys_controller.h"
#include "rope.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef CORPSE_ROPES

#define MAX_CORPSE_ROPES 2

// This is a set of ropes/constraints attached to a ragdoll corpse
#define PERPETUAL_MOTIONxxx
#if 1 /*def PERPETUAL_MOTION*/
class CCorpseRopes : public CLogicalEntity, public IVPhysicsWatcher
#else
class CCorpseRopes : public CLogicalEntity
#endif
{
public:
	DECLARE_CLASS( CCorpseRopes, CLogicalEntity )
	DECLARE_DATADESC()

	CCorpseRopes();
	~CCorpseRopes();

	void Init( CBaseAnimating *pCorpseEnt, string_t iszEntName[MAX_CORPSE_ROPES], const char *szAttach[MAX_CORPSE_ROPES] );
	void InitRope( int nRope, IPhysicsConstraintGroup *pGroup, CBaseAnimating *pCorpseEnt, string_t iszEntName, const char *szAttach );

#define CORPSE_MOTION_THINK_CONTEXT "CorpseMotionThinkContext"
	void MotionThink( void );
	void NotifyVPhysicsStateChanged( IPhysicsObject *pPhysics, CBaseEntity *pEntity, bool bAwake );
	Vector m_vSpawnPos;
	EHANDLE m_hCorpseEnt;

	void InputShove( inputdata_t &inputdata );
#ifdef PERPETUAL_MOTION
	int m_iSwingDir;
	Vector m_vSpawnDir;
	void MotionThink( void )
	{
		CBaseEntity *pEnt = GetMoveParent();
		if ( pEnt == NULL )
			return;
		Vector velocity = pEnt->GetAbsVelocity();
		velocity.NormalizeInPlace();
		if ( DotProduct( velocity, m_vSpawnDir ) > 0 )
			m_iSwingDir = 1;
		else
			m_iSwingDir = -1;
		SetNextThink( gpGlobals->curtime + 0.1 );
	}
	void NotifyVPhysicsStateChanged( IPhysicsObject *pPhysics, CBaseEntity *pEntity, bool bAwake )
	{
		if ( bAwake )
			return;

		ragdoll_t *ragdoll = Ragdoll_GetRagdoll( pEntity );
		if ( !ragdoll || ragdoll->listCount < 1 || pPhysics != ragdoll->list[ 0 ].pObject )
			return;

		for ( int iRagdoll = 0; iRagdoll < ragdoll->listCount; ++iRagdoll )
		{
			IPhysicsObject *pPhysicsObject = ragdoll->list[ iRagdoll ].pObject;
			if ( pPhysicsObject != NULL )
			{
				pPhysicsObject->EnableMotion( true );
				pPhysicsObject->Wake();
			}
		}

		DevMsg( 2, "CCorpseRopes::NotifyVPhysicsStateChanged waking ragdoll\n" );

//		pEntity->SetAbsVelocity( forward * 5 );
		IPhysicsObject *pPhysicsObject = ragdoll->list[ 0 ].pObject;
		if ( pPhysicsObject != NULL )
		{
			pPhysicsObject->ApplyForceCenter( m_vSpawnDir * 500 * m_iSwingDir );
		}
	}
#endif // PERPETUAL_MOTION

	CHandle<CRopeKeyframe> m_hRope[MAX_CORPSE_ROPES]; // Visible textured rope
	IPhysicsConstraint *m_pConstraint[MAX_CORPSE_ROPES]; // Length constraint
	IPhysicsConstraint *m_pFeetConstraint; // Hack: for hanging_byfeet, constrain the feet together
	IPhysicsConstraintGroup *m_pConstraintGroup;
	EHANDLE m_hKeepForward; // For single rope, try to face in the spawn direction
//	EHANDLE m_hPointPush; // Entity to nudge the corpse so it doesn't look bolted in place
};

LINK_ENTITY_TO_CLASS( hoe_corpse_ropes, CCorpseRopes );

BEGIN_DATADESC( CCorpseRopes )
	DEFINE_AUTO_ARRAY( m_hRope, FIELD_EHANDLE ),
	DEFINE_FIELD( m_hKeepForward, FIELD_EHANDLE ),
//	DEFINE_FIELD( m_hPointPush, FIELD_EHANDLE ),
	DEFINE_PHYSPTR( m_pConstraint[0] ), // Can't use DEFINE_PHYSPTR_ARRAY with NULL elements
	DEFINE_PHYSPTR( m_pConstraint[1] ), // Can't use DEFINE_PHYSPTR_ARRAY with NULL elements
	DEFINE_PHYSPTR( m_pFeetConstraint ),
//	DEFINE_PHYSPTR( m_pConstraintGroup ), // this is the ragdoll's constraintgroup, don't save it twice
#if 1
	DEFINE_THINKFUNC( MotionThink ),
	DEFINE_FIELD( m_hCorpseEnt, FIELD_EHANDLE ),
#endif
	DEFINE_INPUTFUNC( FIELD_FLOAT, "Shove", InputShove ),
END_DATADESC()

//-----------------------------------------------------------------------------
CCorpseRopes::CCorpseRopes()
{
	for ( int i = 0; i < MAX_CORPSE_ROPES; i++ )
	{
		m_hRope[i] = NULL;
		m_pConstraint[i] = NULL;
	}
	m_pFeetConstraint = NULL;
	m_pConstraintGroup = NULL;
}

//-----------------------------------------------------------------------------
CCorpseRopes::~CCorpseRopes()
{
	for ( int i = 0; i < MAX_CORPSE_ROPES; i++ )
	{
		UTIL_Remove( m_hRope[i] ); // may be NULL
		if ( m_pConstraint[i] )
		{
			m_pConstraint[i]->Deactivate();
			physenv->DestroyConstraint( m_pConstraint[i] );
		}
	}
//	if ( m_pConstraintGroup )
//		physenv->DestroyConstraintGroup( m_pConstraintGroup ); // NO! this belongs to the ragdoll
	UTIL_Remove( m_hKeepForward ); // may be NULL
}

//-----------------------------------------------------------------------------
void CCorpseRopes::Init( CBaseAnimating *pCorpseEnt, string_t iszEntName[MAX_CORPSE_ROPES], const char *szAttach[MAX_CORPSE_ROPES] )
{
	m_hCorpseEnt = pCorpseEnt;
//	WatchVPhysicsStateChanges( this, pCorpseEnt );
	m_vSpawnPos = pCorpseEnt->GetAbsOrigin();
#ifdef PERPETUAL_MOTION
	SetParent( pCorpseEnt );
	SetThink( &CCorpseRopes::MotionThink );
	SetNextThink( gpGlobals->curtime + 0.1 );
	WatchVPhysicsStateChanges( this, pCorpseEnt );
	m_iSwingDir = 1;
	GetVectors( &m_vSpawnDir, NULL, NULL );
#endif
	ragdoll_t *pRagdoll = Ragdoll_GetRagdoll( pCorpseEnt );
	if ( pRagdoll )
	{
		m_pConstraintGroup = pRagdoll->pGroup;
	}
	else
	{
		// Create our constraint group
		constraint_groupparams_t group;
		group.Defaults();
		m_pConstraintGroup = physenv->CreateConstraintGroup( group );
	}

	for ( int i = 0; i < MAX_CORPSE_ROPES; i++ )
	{
		InitRope( i, m_pConstraintGroup, pCorpseEnt, iszEntName[i], szAttach[i] );
	}

	// Mega Hack: for hanging_byfeet, constrain the feet together.
	// This requires a model where the legs don't collide with each other.
	if ( Q_stristr(szAttach[0], "feet" ) != NULL )
	{
#if 0
		IPhysicsObject *list[VPHYSICS_MAX_OBJECT_LIST_COUNT];
		int listCount = pCorpseEnt->VPhysicsGetObjectList( list, ARRAYSIZE(list) );
		list[8]->EnableMotion( false );
		list[11]->EnableMotion( false );
#else
		int nAttach1 = pCorpseEnt->LookupAttachment( "left_foot" );
		int nAttach2 = pCorpseEnt->LookupAttachment( "right_foot" );

		IPhysicsObject *list[VPHYSICS_MAX_OBJECT_LIST_COUNT];
		int listCount = pCorpseEnt->VPhysicsGetObjectList( list, ARRAYSIZE(list) );
		int iPhysicsBone1 = pCorpseEnt->GetPhysicsBone( pCorpseEnt->GetAttachmentBone( nAttach1 ) );
		int iPhysicsBone2 = pCorpseEnt->GetPhysicsBone( pCorpseEnt->GetAttachmentBone( nAttach2 ) );
		if ( iPhysicsBone1 < listCount && iPhysicsBone2 < listCount )
		{
			IPhysicsObject *pPhysicsObject1 = list[iPhysicsBone1];
			IPhysicsObject *pPhysicsObject2 = list[iPhysicsBone2];

#if 1
			constraint_fixedparams_t fixed;
			fixed.Defaults();
			fixed.InitWithCurrentObjectState( pPhysicsObject1, pPhysicsObject2 );
			m_pFeetConstraint = physenv->CreateFixedConstraint( pPhysicsObject1, pPhysicsObject2, m_pConstraintGroup, fixed );

#if 0
			// Hack #2: Stop legs colliding with each other.
			// Changinge the collisions affects ALL ragdolls using a particular model.
			// FIXME: these should go into the "collisionrules" keyvalues in the .qc file.
			int modelIndex = pCorpseEnt->GetModelIndex();
			IPhysicsCollisionSet *pSet = physics->FindCollisionSet( modelIndex );
			if ( pSet )
			{
				int i;
				for ( i = 0; i < listCount; i++ )
				{
					for ( int j = i+1; j < listCount; j++ )
					{
						pSet->DisableCollisions( i, j );
					}
				}
#if 0
				for ( i = 0; i < listCount; i++ )
				{
					list[i]->RecheckCollisionFilter();
					list[i]->RecheckContactPoints();
				}
				pCorpseEnt->VPhysicsGetObject()->RecheckCollisionFilter();
				pCorpseEnt->VPhysicsGetObject()->RecheckContactPoints();
#endif
				pCorpseEnt->CollisionRulesChanged();
			}
#endif
#else
			Vector vAttach1, vAttach2;
			pCorpseEnt->GetAttachment( nAttach1, vAttach1 );
			pCorpseEnt->GetAttachment( nAttach2, vAttach2 );

			constraint_lengthparams_t length;
			length.Defaults();
			length.InitWorldspace( pPhysicsObject1, pPhysicsObject2, vAttach1, vAttach2 );
			m_pFeetConstraint = physenv->CreateLengthConstraint( pPhysicsObject1, pPhysicsObject2, m_pConstraintGroup, length );
#endif
		}
#endif
	}

	m_pConstraintGroup->Activate();

	// If only one rope, apply torque to keep the ragdoll facing in the right direction
	if ( m_hRope[0] && !m_hRope[1] )
	{
extern CBaseEntity *CreateKeepForward( const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pOwner, float flAngularLimit, bool bActive );
		m_hKeepForward = CreateKeepForward( GetAbsOrigin(), GetAbsAngles(), pCorpseEnt, 5, true );
	}
#if 0
	// Create a physics thruster to shove the ragdoll via its Activate input
	if ( pCorpseEnt->GetEntityName() != NULL_STRING )
	{
		Vector forward;
		AngleVectors( GetAbsAngles(), &forward );
		VectorYawRotate( forward, 45, forward );
		QAngle angles;
		VectorAngles( forward, angles );
		m_hPointPush = CBaseEntity::Create( "phys_thruster", GetAbsOrigin(), angles, pCorpseEnt );
		char szName[256];
		Q_snprintf( szName, 256, "%s_%s", STRING(pCorpseEnt->GetEntityName()), "thruster" );
		m_hPointPush->SetName( AllocPooledString( szName ) );
		m_hPointPush->AddSpawnFlags( 0x1 ); // start inactive
		m_hPointPush->AddSpawnFlags( 0x2 ); // thrust center of mass
		m_hPointPush->KeyValue( "attach1", STRING(pCorpseEnt->GetEntityName()) );
		m_hPointPush->KeyValue( "force", "1500" );
//		m_hPointPush->KeyValue( "forcetime", "1" );
		m_hPointPush->Activate();
	}
#endif
}

//-----------------------------------------------------------------------------
void CCorpseRopes::InitRope( int nRope, IPhysicsConstraintGroup *pGroup, CBaseAnimating *pCorpseEnt, string_t iszEntName, const char *szAttach )
{
	if ( iszEntName == NULL_STRING || szAttach == NULL )
		return;

	int nAttach = pCorpseEnt->LookupAttachment( szAttach );
	if ( nAttach == 0 )
	{
		DevWarning( "CCorpseRopes::InitRope: bogus attachment '%s'\n", szAttach );
		return;
	}
	Vector vAttach;
	pCorpseEnt->GetAttachment( nAttach, vAttach );

	// If the name of the top rope entity is "auto" then create a point entity directly above
	// the attachment point by tracing up to the world.  Otherwise use the named entity.
	CRopeKeyframe *pRope = NULL;
	CBaseEntity *pEnt = NULL;
	if ( Q_stricmp( STRING(iszEntName), "auto" ) )
	{
		pEnt = gEntList.FindEntityByName( NULL, STRING(iszEntName) );
		if ( pEnt == NULL )
		{
			DevWarning( "CCorpseRopes::InitRope: bogus rope entity '%s'\n", iszEntName );
			return;
		}
		pRope = CRopeKeyframe::Create( pEnt, pCorpseEnt, 0, nAttach, 2, "cable/cablerope.vmt", 5 );
	}
	else
	{
		trace_t tr;
		UTIL_TraceLine( vAttach, vAttach + Vector(0,0,2048), CONTENTS_SOLID, NULL, COLLISION_GROUP_NONE, &tr );
		if ( !tr.DidHitWorld() )
		{
			DevWarning( "CCorpseRopes::InitRope: can't find solid surface for rope '%s (%s)'\n", pCorpseEnt->GetDebugName(), szAttach );
			return;
		}
#if 1
		pRope = CRopeKeyframe::CreateWithStartPosition( tr.endpos, pCorpseEnt, nAttach, 2, "cable/cablerope.vmt", 5 );
		pEnt = pRope;
#else
		pEnt = CBaseEntity::Create( "info_target", tr.endpos, vec3_angle );
		if ( pEnt == NULL )
		{
			DevWarning( "CCorpseRopes::InitRope: failed to create auto entity '%s (%s)'\n", pCorpseEnt->GetDebugName(), szAttach );
			return;
		}
#endif
	}

	if ( pRope == NULL )
	{
		DevWarning( "CCorpseRopes::InitRope: failed to create keyframe_rope '%s (%s)'\n", pCorpseEnt->GetDebugName(), szAttach );
		return;
	}
	pRope->SetupHangDistance( 0.0f );
	pRope->EnableWind( false );
	pRope->m_RopeLength = (pEnt->GetAbsOrigin() - vAttach).Length();
	pRope->m_TextureScale.Set( 1.0f );
	m_hRope[nRope] = pRope;

	IPhysicsObject *pPhysicsObject = pCorpseEnt->VPhysicsGetObject();
	if ( pPhysicsObject )
	{
		IPhysicsObject *list[VPHYSICS_MAX_OBJECT_LIST_COUNT];
		int listCount = pCorpseEnt->VPhysicsGetObjectList( list, ARRAYSIZE(list) );
		int iPhysicsBone = pCorpseEnt->GetPhysicsBone( pCorpseEnt->GetAttachmentBone( nAttach ) );
		if ( iPhysicsBone < listCount )
		{
			pPhysicsObject = list[iPhysicsBone];
		}

#if 1
		constraint_lengthparams_t length;
		length.Defaults();
		length.InitWorldspace( g_PhysWorldObject, pPhysicsObject, pEnt->GetAbsOrigin(), vAttach );
		m_pConstraint[nRope] = physenv->CreateLengthConstraint( g_PhysWorldObject, pPhysicsObject, pGroup, length );
#else
		constraint_ballsocketparams_t ballsocket;
		ballsocket.Defaults();
		ballsocket.InitWithCurrentObjectState( g_PhysWorldObject, pPhysicsObject, pEnt->GetAbsOrigin() );

//		g_PhysWorldObject->WorldToLocal( &ballsocket.constraintPosition[0], pEnt->GetAbsOrigin() );
//		pPhysicsObject->WorldToLocal( &ballsocket.constraintPosition[1], vAttach );

		physenv->CreateBallsocketConstraint( g_PhysWorldObject, pPhysicsObject, pGroup, ballsocket );
#endif
	}
}

//-----------------------------------------------------------------------------
void CCorpseRopes::MotionThink( void )
{
	CBaseEntity *pCorpse = m_hCorpseEnt;
	if ( pCorpse == NULL )
		return;

	edict_t *pentPlayer = UTIL_FindClientInPVS( edict() );
	CBasePlayer *pPlayer = NULL;
	bool bLOS = false, bFOV = false;
	if ( pentPlayer )
	{
		pPlayer = (CBasePlayer *)CBaseEntity::Instance( pentPlayer );
		bLOS = pPlayer->FVisible( pCorpse, MASK_VISIBLE );

		// This is copied from FInViewCone but with fov=0
		Vector los = ( pCorpse->WorldSpaceCenter() - pPlayer->EyePosition() );
		los.z = 0;
		VectorNormalize( los );
		Vector facingDir = pPlayer->EyeDirection2D( );
		float flDot = DotProduct( los, facingDir );
		if ( flDot > 0 )
			bFOV = true;

	}
	if ( !pPlayer || ( bLOS && bFOV ) )
	{
		SetNextThink( gpGlobals->curtime + random->RandomFloat( 1.0f, 1.5f ), CORPSE_MOTION_THINK_CONTEXT );
		return;
	}

	// Don't jump too much if the player could turn and see the ragdoll
	float scale = bLOS ? 0.5 : 1.0;

	if ( (m_hCorpseEnt->GetAbsOrigin() - m_vSpawnPos).Length2D() <= 16 )
	{
		Vector forward, right;
		GetVectors( &forward, &right, NULL );
		Vector origin = m_hCorpseEnt->GetAbsOrigin() - forward * 4 * scale + right * 4 * scale;
		Vector velocity = forward * 2 - right * 4; velocity *= scale;
		pCorpse->Teleport( &origin, NULL, &velocity );

		IPhysicsObject *list[VPHYSICS_MAX_OBJECT_LIST_COUNT];
		int listCount = pCorpse->VPhysicsGetObjectList( list, ARRAYSIZE(list) );
		for ( int i = 0 ; i < listCount; i++ )
		{
			if ( list[i] )
			{
				list[i]->EnableMotion( true );
				list[i]->Wake();
//				list[i]->SetVelocityInstantaneous( &velocity, NULL );
			}
		}
		return;
	}

//	SetNextThink( gpGlobals->curtime + 0.1, CORPSE_MOTION_THINK_CONTEXT );
}

//-----------------------------------------------------------------------------
void CCorpseRopes::NotifyVPhysicsStateChanged( IPhysicsObject *pPhysics, CBaseEntity *pEntity, bool bAwake )
{
	if ( bAwake )
	{
		SetContextThink( NULL, TICK_NEVER_THINK, CORPSE_MOTION_THINK_CONTEXT );
		return;
	}

	ragdoll_t *pRagdoll = Ragdoll_GetRagdoll( pEntity );
	if ( pRagdoll && RagdollIsAsleep( *pRagdoll ) )
	{
		SetContextThink( &CCorpseRopes::MotionThink, gpGlobals->curtime, CORPSE_MOTION_THINK_CONTEXT );
	}
}

//-----------------------------------------------------------------------------
void CCorpseRopes::InputShove( inputdata_t &inputdata )
{
	if ( m_hCorpseEnt && m_hCorpseEnt->VPhysicsGetObject() )
	{
		Vector forward, right;
		GetVectors( &forward, &right, NULL );
		Vector force((forward + right).NormalizeInPlace() * inputdata.value.Float());
		IPhysicsObject *list[VPHYSICS_MAX_OBJECT_LIST_COUNT];
		int listCount = m_hCorpseEnt->VPhysicsGetObjectList( list, ARRAYSIZE(list) );
		for ( int i = 0 ; i < listCount; i++ )
		{
			if ( list[i] )
			{
				list[i]->EnableMotion( true );
				list[i]->Wake();
//				list[i]->SetVelocityInstantaneous( &vec3_origin, NULL );
				list[i]->ApplyForceCenter( force * list[i]->GetMass() );
			}
		}
	}
}

#endif // CORPSE_ROPES

//-----------------------------------------------------------------------------
// Spawnflags
//-----------------------------------------------------------------------------
#define SF_CORPSE_RAGDOLL (1 << 16)
#define SF_CORPSE_SLEEP (1 << 17)

ConVar hoe_corpse_ragdoll( "hoe_corpse_ragdoll", "1", FCVAR_NONE, "If true, Hammer-placed corpses are prop_ragdolls." );
ConVar hoe_corpse_sleep( "hoe_corpse_sleep", "0", FCVAR_NONE, "If true, Hammer-placed prop_ragdoll corpses spawn asleep." );

BEGIN_DATADESC( CHOECorpse )
	DEFINE_KEYFIELD( m_pose, FIELD_INTEGER, "pose" ),
	DEFINE_THINKFUNC( AnimThink ),
#ifdef CORPSE_ROPES
	DEFINE_KEYFIELD( m_nameRope1, FIELD_STRING, "rope1" ),
	DEFINE_KEYFIELD( m_nameRope2, FIELD_STRING, "rope2" ),
#endif
END_DATADESC()

//-----------------------------------------------------------------------------
CHOECorpse::CHOECorpse()
{
	m_pose = 0; // default to lying_on_back, map name2 didn't specify pose for 3 mikeforce corpses
	m_poses = NULL;
}

//-----------------------------------------------------------------------------
void CHOECorpse::Spawn( void )
{
	Assert( m_poses != NULL );

	int i;
	for ( i = 0; m_poses[i].sequence; i++ ) /**/;
	Assert( m_pose >= 0 && m_pose < i );

#if 1
	if ( ( hoe_corpse_ragdoll.GetBool() || HasSpawnFlags( SF_CORPSE_RAGDOLL ) )
		/*&& !m_poses[m_pose].hanging*/ )
	{
		InitRagdollCorpse();
		return;
	}
#endif

	BaseClass::Spawn();

	Precache();
	SetModel( STRING( GetModelName() ) );
	SelectModelGroups();

	ResetSequence( LookupSequence( m_poses[m_pose].sequence ) );

	m_iHealth = 8;

	NPCInitDead();

	if ( m_poses[m_pose].hanging )
	{
		SetMoveType( MOVETYPE_NONE );
		m_flPlaybackRate = 1.0;

		if ( SequenceLoops() )
			SetThink( &CHOECorpse::AnimThink );
		else
			SetThink( NULL ); // don't fall to ground
	}

	// Create a marker for carcass smell and seeing dead friends
	if ( m_iszLivingClass != NULL_STRING )
	{
		bool bSmell = !m_poses[m_pose].hanging;
		CHOECorpseMarker *pMarker = assert_cast<CHOECorpseMarker *>( CBaseEntity::Create( "hoe_corpse_marker", GetAbsOrigin(), GetAbsAngles(), this ) );
		pMarker->InitMarker( this, m_iszLivingClass, bSmell );
	}

#ifdef CORPSE_ROPES
	if ( m_nameRope1 != NULL_STRING || m_nameRope2 != NULL_STRING )
	{
		string_t szEntName[2] = { m_nameRope1, m_nameRope2 };
		const char *szAttach[2] = { m_poses[m_pose].rope1, m_poses[m_pose].rope2 };
		for ( int i = 0; i < 2; i++ )
		{
			InitRope( szEntName[i], szAttach[i] );
		}
	}
#endif
}

//-----------------------------------------------------------------------------
void CHOECorpse::Precache( void )
{
	PrecacheModel( STRING( GetModelName() ) );
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
void CHOECorpse::InitCorpse( const char *modelName, CorpsePose poses[], string_t iszLivingClass )
{
	SetModelName( AllocPooledString( modelName ) );
	m_poses = poses;
	m_iszLivingClass = iszLivingClass;
}

#ifdef CORPSE_ROPES
//-----------------------------------------------------------------------------
void CHOECorpse::InitRope( string_t iszEntName, const char *szAttach )
{
	if ( iszEntName == NULL_STRING || szAttach == NULL )
		return;

	int nAttach = LookupAttachment( szAttach );
	if ( nAttach == 0 )
	{
		DevWarning( "CHOECorpse::InitRope: bogus attachment '%s'\n", szAttach );
		return;
	}
	Vector vAttach;
	GetAttachment( nAttach, vAttach );

	// If the name of the top rope entity is "auto" then create a point entity directly above
	// the attachment point by tracing up to the world.  Otherwise use the named entity.
	CRopeKeyframe *pRope = NULL;
	CBaseEntity *pEnt = NULL;
	if ( Q_stricmp( STRING(iszEntName), "auto" ) )
	{
		pEnt = gEntList.FindEntityByName( NULL, STRING(iszEntName) );
		if ( pEnt == NULL )
		{
			DevWarning( "CHOECorpse::InitRope: bogus entity '%s'\n", iszEntName );
			return;
		}
		pRope = CRopeKeyframe::Create( pEnt, this, 0, nAttach, 2, "cable/cablerope.vmt", 2 );
	}
	else
	{
		trace_t tr;
		UTIL_TraceLine( vAttach, vAttach + Vector(0,0,2048), CONTENTS_SOLID, NULL, COLLISION_GROUP_NONE, &tr );
		if ( !tr.DidHitWorld() )
		{
			DevWarning( "CHOECorpse::InitRope: can't find solid surface for rope '%s'\n", szAttach );
			return;
		}
#if 1
		pRope = CRopeKeyframe::CreateWithStartPosition( tr.endpos, this, nAttach, 2, "cable/cablerope.vmt", 5 );
		pEnt = pRope;
#else
		pEnt = CBaseEntity::Create( "info_target", tr.endpos, vec3_angle );
		if ( pEnt == NULL )
		{
			DevWarning( "CHOECorpse::InitRope: failed to create auto entity '%s'\n", szAttach );
			return;
		}
#endif
	}

	if ( pRope == NULL )
	{
		DevWarning( "CCorpseRopes::InitRope: failed to create keyframe_rope '%s (%s)'\n", GetDebugName(), szAttach );
		return;
	}

	if ( pRope )
	{
		pRope->SetupHangDistance( 0.0f );
		pRope->EnableWind( false );
		pRope->m_RopeLength = (pEnt->GetAbsOrigin() - vAttach).Length();
		pRope->m_TextureScale.Set( 1.0f );
	}
}
#endif // CORPSE_ROPES

//-----------------------------------------------------------------------------
void CHOECorpse::AnimThink( void )
{
	if ( !UTIL_FindClientInPVS( edict() ) )
	{
		SetNextThink( gpGlobals->curtime + random->RandomFloat( 1.0f, 1.5f ) );
		return;
	}

	StudioFrameAdvance();
	DispatchAnimEvents(this);
	SetNextThink( gpGlobals->curtime + 0.1f );
}

//-----------------------------------------------------------------------------
// see monstermaker.cpp
static void DispatchActivate( CBaseEntity *pEntity )
{
	bool bAsyncAnims = mdlcache->SetAsyncLoad( MDLCACHE_ANIMBLOCK, false );
	pEntity->Activate();
	mdlcache->SetAsyncLoad( MDLCACHE_ANIMBLOCK, bAsyncAnims );
}
void CHOECorpse::InitRagdollCorpse( void )
{
	bool bSleep = hoe_corpse_sleep.GetBool() || HasSpawnFlags( SF_CORPSE_SLEEP );
	Vector origin = GetAbsOrigin();
	if ( !bSleep && !m_poses[m_pose].hanging )
		origin += Vector(0,0,5);
	CBaseEntity *pPropRagdoll = CBaseEntity::CreateNoSpawn( "prop_ragdoll", origin, GetAbsAngles() );
	if ( pPropRagdoll )
	{
		pPropRagdoll->KeyValue( "model", STRING(GetModelName()) );
		pPropRagdoll->KeyValue( "InitialPose", m_poses[m_pose].sequence );
#if 1
		int flags = bSleep ? 0x10000 : 0;
		if ( m_poses[m_pose].hanging == false )
			flags |= 0x4; // debris
		pPropRagdoll->AddSpawnFlags( flags );
#else
		if ( bSleep )
			pPropRagdoll->KeyValue( "spawnflags", "65540" ); // debris + asleep
		else
			pPropRagdoll->KeyValue( "spawnflags", "4" ); // debris
#endif
		DispatchSpawn( pPropRagdoll );
		DispatchActivate( pPropRagdoll );

		// Transfer our name to the ragdoll, and forget our name so other entities
		// don't latch on to us (like ambient_generic)
		pPropRagdoll->SetName( GetEntityName() );
		SetName( NULL_STRING );

		// Transfer bodygroups to the ragdoll
		Precache();
		SetModel( STRING( GetModelName() ) );
		SelectModelGroups();
		((CBaseAnimating *)pPropRagdoll)->m_nBody = m_nBody;

		// Hack: allow shot ragdolls to splatter blood
		((CRagdollProp *) pPropRagdoll)->SetBloodColor( BloodColor() );

		// Transfer CC image (so THEM can talk)
		pPropRagdoll->SetCCImageName( GetCCImageName() );

		// Keep the consistent with CHOEHuman::BecomeRagdollOnClient
		pPropRagdoll->AddEFlags( EFL_NO_ROTORWASH_PUSH );

		// Create a marker for carcass smell and seeing dead friends
		if ( /*!m_poses[m_pose].hanging &&*/ m_iszLivingClass != NULL_STRING )
		{
			bool bSmell = !m_poses[m_pose].hanging;
			CHOECorpseMarker *pMarker = assert_cast<CHOECorpseMarker *>( CBaseEntity::Create( "hoe_corpse_marker", GetAbsOrigin(), GetAbsAngles(), this ) );
			if ( pMarker )
				pMarker->InitMarker( pPropRagdoll, m_iszLivingClass, bSmell );
		}

#ifdef CORPSE_ROPES
		if ( m_nameRope1 != NULL_STRING || m_nameRope2 != NULL_STRING )
		{
			string_t szEntName[2] = { m_nameRope1, m_nameRope2 };
			const char *szAttach[2] = { m_poses[m_pose].rope1, m_poses[m_pose].rope2 };

			CBaseEntity *pRopes = CBaseEntity::Create( "hoe_corpse_ropes", GetAbsOrigin(), GetAbsAngles() );
			if ( pRopes )
			{
				if ( pPropRagdoll->GetEntityName() != NULL_STRING )
				{
					char szName[256];
					Q_snprintf( szName, 256, "%s_%s", STRING(pPropRagdoll->GetEntityName()), "ropes" );
					pRopes->SetName( AllocPooledString( szName ) );
				}
				((CCorpseRopes *)pRopes)->Init( (CBaseAnimating *)pPropRagdoll, szEntName, szAttach );
			}
#if 0
			// Create our constraint group
			constraint_groupparams_t group;
			group.Defaults();
			IPhysicsConstraintGroup *pConstraintGroup = physenv->CreateConstraintGroup( group );
//			m_hCraneMagnet->SetConstraintGroup( m_pConstraintGroup );

			InitRope( pConstraintGroup, (CBaseAnimating *)pPropRagdoll, m_nameRope1, m_poses[m_pose].rope1 );
			InitRope( pConstraintGroup, (CBaseAnimating *)pPropRagdoll, m_nameRope2, m_poses[m_pose].rope2 );

			pConstraintGroup->Activate();
#endif
		}
		else if ( m_poses[m_pose].hanging )
		{
			DevWarning( "hanging corpse with no ropes! (%s)\n", pPropRagdoll->GetDebugName() );
		}
#endif // CORPSE_ROPES

#ifdef CORPSE_BEHEAD
		// Keep this entity around to respond to machete head-chopping
		BaseClass::Spawn();
		Precache();
		SetModel( "models/error.mdl" );
		m_iHealth = 8;
		NPCInitDead();
		SetMoveType( MOVETYPE_NONE );
		SetBloodColor( DONT_BLEED );
		SetGravity( 0.0 );
		SetThink( NULL );
		m_takedamage = DAMAGE_NO;
		AddEffects( EF_NODRAW );

		((CRagdollProp *) pPropRagdoll)->SetDamageEntity( this );
#else
		RemoveDeferred();
#endif
	}
}

#ifdef CORPSE_BEHEAD
//---------------------------------------------------------
bool CHOECorpse::HandleInteraction( int interactionType, void *data, CBaseCombatCharacter* sourceEnt )
{
	extern int g_interactionMacheteHeadChop;
	if ( interactionType == g_interactionMacheteHeadChop )
	{
		if ( HasAHead() )
		{
			sourceEnt->EmitSound( "Weapon_Machete.HeadChop" );
			Behead();
		}
		return true;
	}

	return false;
}
#endif // CORPSE_BEHEAD

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

BEGIN_DATADESC( CHOECorpseMarker )
	DEFINE_FIELD( m_iszLivingClass, FIELD_STRING ),
	DEFINE_THINKFUNC( MarkerThink ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( hoe_corpse_marker, CHOECorpseMarker );

//-----------------------------------------------------------------------------
void CHOECorpseMarker::InitMarker( CBaseEntity *pParent, string_t iszLivingClass, bool bSmell )
{
	if ( pParent )
	{
		SetAbsOrigin( pParent->WorldSpaceCenter() );
	}
	SetParent( pParent );
	m_iszLivingClass = iszLivingClass;

//	CSoundEnt::InsertSound( SOUND_CARCASS, GetAbsOrigin(), 384, 100000, this,
//		SOUNDENT_CHANNEL_REPEATING );

	if ( bSmell )
	{
		SetThink( &CHOECorpseMarker::MarkerThink );
		SetNextThink( gpGlobals->curtime + 5.0f );
	}
}

//-----------------------------------------------------------------------------
void CHOECorpseMarker::MarkerThink( void )
{
	// These get auto-deleted if the parent is removed (with a warning about orphaned
	// entity).  I also saw this happen when going back to a previous map where the
	// ragdoll gets removed.
	if ( GetParent() == NULL )
	{
		DevMsg( "CHOECorpseMarker::MarkerThink Removing parentless corpse marker\n" );
		UTIL_Remove( this ); // in case the ragdoll is deleted
		return;
	}

	// I would like to update the smell position only if this entity moves
	// and avoid this whole think function.  Actually CSound::GetSoundOrigin()
	// could be changed to return the origin of the sound owner in this case.
	CSoundEnt::InsertSound( SOUND_CARCASS, GetAbsOrigin(), 384, 6, this,
		SOUNDENT_CHANNEL_REPEATING );

	SetNextThink( gpGlobals->curtime + 5.0f );
}


typedef void (*NPCBecameRagdollProc)( int entindex, void *ragdoll );
typedef void (*RagdollMovedProc)( void *ragdoll, const Vector &origin );

// THIS MUST MATCH hoe_clienttoserver.cpp on the client
struct ClientToServerProcs
{
	NPCBecameRagdollProc NPCBecameRagdoll;
	RagdollMovedProc RagdollMoved;
};

struct CTSRagdollInfo
{
	int entindex;
	void *clientRagdoll;
	EHANDLE bullseye;
};

void NPCBecameRagdollWrapper( int entindex, void *ragdoll );
void RagdollMovedWrapper( void *ragdoll, const Vector &origin );

class CClientToServer : public CPointEntity
{
public:
	DECLARE_CLASS( CClientToServer, CBaseEntity );
//	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	CClientToServer()
	{
		m_procs.NPCBecameRagdoll = NPCBecameRagdollWrapper;
		m_procs.RagdollMoved = RagdollMovedWrapper;

		gm_ClientToServer = this;
	}
	~CClientToServer()
	{
		gm_ClientToServer = NULL;
	}

	int UpdateTransmitState( void )
	{
		if ( m_bSent )
			return SetTransmitState( FL_EDICT_DONTSEND );
		else
		{
			m_bSent = true;
			return SetTransmitState( FL_EDICT_ALWAYS );
		}
	}

	void Activate( void )
	{
		BaseClass::Activate();
		Q_snprintf( m_szAddress.GetForModify(), 128, "%p", &m_procs );

		// FIXME: turn off once client exists 
		DispatchUpdateTransmitState();
	}

	void NPCBecameRagdoll( int entindex, void *ragdoll );
	void RagdollMoved( void *ragdoll, const Vector &origin );
	CBaseEntity *CClientToServer::GetBullseyeForVictim( int entindex );

	CNetworkString( m_szAddress, 128 );
	ClientToServerProcs m_procs;
	bool m_bSent;
	CUtlVector< CTSRagdollInfo > m_ragdolls;
	static CClientToServer *gm_ClientToServer;
};

IMPLEMENT_SERVERCLASS_ST(CClientToServer, DT_ClientToServer)
	SendPropString( SENDINFO( m_szAddress ) ),
END_SEND_TABLE();

LINK_ENTITY_TO_CLASS( hoe_clientoserver, CClientToServer );

CClientToServer *CClientToServer::gm_ClientToServer = NULL;

void CClientToServer::NPCBecameRagdoll( int entindex, void *ragdoll )
{
	for ( int i = 0; i < m_ragdolls.Count();  )
	{
		if ( m_ragdolls[i].bullseye == NULL )
		{
			m_ragdolls.Remove( i );
			continue;
		}
		if ( m_ragdolls[i].entindex == entindex )
		{
			m_ragdolls[i].clientRagdoll = ragdoll;
			break;
		}
		i++;
	}
}

void CClientToServer::RagdollMoved( void *ragdoll, const Vector &origin )
{
//	DevMsg( "RAGDOLL MOVED %p %.2f,%.2f,%.2f\n", ragdoll, origin.x, origin.y, origin.z );
	for ( int i = 0; i < m_ragdolls.Count(); )
	{
		if ( m_ragdolls[i].bullseye == NULL )
		{
			m_ragdolls.Remove( i );
			continue;
		}
		if ( m_ragdolls[i].clientRagdoll == ragdoll )
		{
			m_ragdolls[i].bullseye.Get()->SetAbsOrigin( origin );
			break;
		}
		i++;
	}
}

CBaseEntity *CClientToServer::GetBullseyeForVictim( int entindex )
{
	for ( int i = 0; i < m_ragdolls.Count();  )
	{
		if ( m_ragdolls[i].bullseye == NULL )
		{
			m_ragdolls.Remove( i );
			continue;
		}
		if ( m_ragdolls[i].entindex == entindex )
		{
			return m_ragdolls[i].bullseye;
		}
		i++;
	}
	return NULL;
}

void NPCBecameRagdollWrapper( int entindex, void *ragdoll )
{
	CClientToServer::gm_ClientToServer->NPCBecameRagdoll( entindex, ragdoll );
}

void RagdollMovedWrapper( void *ragdoll, const Vector &origin )
{
	CClientToServer::gm_ClientToServer->RagdollMoved( ragdoll, origin );
}

void RegisterBullseyeOnRagdoll( int entindex, CBaseEntity *pBullseye )
{
	CTSRagdollInfo info;
	info.entindex = entindex;
	info.bullseye = pBullseye;
	info.clientRagdoll = NULL;
	CClientToServer::gm_ClientToServer->m_ragdolls.AddToTail( info );
}

CBaseEntity *GetRagdollBullseyeForVictim( int entindex )
{
	return CClientToServer::gm_ClientToServer->GetBullseyeForVictim( entindex );
}

class CClientToServerInit : public CAutoGameSystem
{
public:
	CClientToServerInit( char const *name ) : CAutoGameSystem( name )
	{
	}

	virtual void LevelInitPostEntity() 
	{
		if ( CClientToServer::gm_ClientToServer == NULL )
		{
			CBaseEntity *pEnt = CreateEntityByName( "hoe_clientoserver" );
			DispatchSpawn( pEnt );
			pEnt->Activate();
		}
	}
	virtual void LevelShutdownPreEntity() 
	{
		
	}
};

CClientToServerInit g_ClientToServerInit( "CClientToServerInit" );
