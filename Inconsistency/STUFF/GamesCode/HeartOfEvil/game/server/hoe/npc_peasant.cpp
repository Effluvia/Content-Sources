#include "cbase.h"
#include "hoe_human.h"
#include "schedule_hacks.h"
#include "hoe_corpse.h"
#define PEASANT_BYARMS
#ifdef PEASANT_BYARMS
#include "vphysics/constraints.h"
#include "physics_prop_ragdoll.h"
#include "physics_saverestore.h"
#include "rope.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define PEASANT_HEALTH 60 // 50, 60, 80
#define PEASANT_MODEL_STRING "models/peasant/%s%d.mdl"
#define PEASANT_FEAR_TIME 15

//=====================
// BodyGroups
//=====================

enum 
{
	PEASANT_BODYGROUP_HEAD = 0,
	PEASANT_BODYGROUP_BODY,
};

#define PEASANT_BODY_RANDOM			-1
#define PEASANT_BODY_RANDOM_MALE	-2
#define PEASANT_BODY_RANDOM_FEMALE	-3

#define PEASANT_NUM_HEADS 3

#define TLK_PEASANT_STARTLE "TLK_PEASANT_STARTLE"
#define TLK_PEASANT_FEAR "TLK_PEASANT_FEAR"
#define TLK_PEASANT_SCREAM "TLK_PEASANT_SCREAM"

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#ifdef PEASANT_BYARMS

// FIXME: merge with CCorpseRopes
class CPeasantRopes : public CLogicalEntity
{
	DECLARE_CLASS( CPeasantRopes, CLogicalEntity )
public:
	DECLARE_DATADESC()

	void Init( CBaseAnimating *pPeasant, string_t nameRope[2] );
	CBaseEntity *InitRope( CBaseAnimating *pPeasant, string_t iszEntName, const char *szAttach );
	void CreateShackle( CBaseAnimating *pPeasant, int nShackle, const char *szAttach );

	void TransferToRagdoll( CBaseEntity *pRagdollEnt );
	void TransferRopeToRagdoll( CBaseAnimating *pAnimating, int nRope, const char *szAttach, IPhysicsConstraintGroup *pGroup );

	void UpdateOnRemove( void )
	{
		UTIL_Remove( m_hRope[0] );
		UTIL_Remove( m_hRope[1] );

		UTIL_Remove( m_hShackle[0] ); // not really needed since parented to ragdoll
		UTIL_Remove( m_hShackle[1] ); // not really needed since parented to ragdoll

		if ( m_pConstraint[0] )
			physenv->DestroyConstraint( m_pConstraint[0] );
		if ( m_pConstraint[1] )
			physenv->DestroyConstraint( m_pConstraint[1] );

		BaseClass::UpdateOnRemove();
	}

	static CPeasantRopes *CreateRopes( CBaseAnimating *pPeasant, string_t nameRope[2] )
	{
		CBaseEntity *pEnt = CBaseEntity::Create( "peasant_ropes", pPeasant->GetAbsOrigin(), vec3_angle, pPeasant );
		if ( pEnt == NULL )
			return NULL;

		CPeasantRopes *pRopes = assert_cast<CPeasantRopes *>(pEnt);
		pRopes->Init( pPeasant, nameRope );

		return pRopes;
	}

	EHANDLE m_hRope[2]; // keyframe_rope
	EHANDLE m_hShackle[2]; // prop_dynamic
	IPhysicsConstraint *m_pConstraint[2]; // Length constraint
};

LINK_ENTITY_TO_CLASS( peasant_ropes, CPeasantRopes );

BEGIN_DATADESC( CPeasantRopes )
	DEFINE_FIELD( m_hRope[0], FIELD_EHANDLE ),
	DEFINE_FIELD( m_hRope[1], FIELD_EHANDLE ),
	DEFINE_FIELD( m_hShackle[0], FIELD_EHANDLE ),
	DEFINE_FIELD( m_hShackle[1], FIELD_EHANDLE ),
	DEFINE_PHYSPTR( m_pConstraint[0] ), // Can't use DEFINE_PHYSPTR_ARRAY with NULL elements
	DEFINE_PHYSPTR( m_pConstraint[1] ), // Can't use DEFINE_PHYSPTR_ARRAY with NULL elements
END_DATADESC()

//-----------------------------------------------------------------------------
void CPeasantRopes::Init( CBaseAnimating *pPeasant, string_t nameRope[2] )
{
	SetParent( pPeasant );

	CreateShackle( pPeasant, 0, "shackle_left" );
	CreateShackle( pPeasant, 1, "shackle_right" );

	m_hRope[0] = InitRope( pPeasant, nameRope[0], "rope_lefthand" );
	m_hRope[1] = InitRope( pPeasant, nameRope[1], "rope_righthand" );
}

//-----------------------------------------------------------------------------
CBaseEntity *CPeasantRopes::InitRope( CBaseAnimating *pPeasant, string_t iszEntName, const char *szAttach )
{
	if ( iszEntName == NULL_STRING || szAttach == NULL )
		return NULL;

	int nAttach = pPeasant->LookupAttachment( szAttach );
	if ( nAttach == 0 )
	{
		DevWarning( "CPeasantRopes::InitRope: bogus attachment '%s'\n", szAttach );
		return NULL;
	}
	Vector vAttach;
	pPeasant->GetAttachment( nAttach, vAttach );

	// If the name of the top rope entity is "auto" then create a point entity directly above
	// the attachment point by tracing up to the world.  Otherwise use the named entity.
	CRopeKeyframe *pRope = NULL;
	CBaseEntity *pEnt = NULL;
	if ( Q_stricmp( STRING(iszEntName), "auto" ) )
	{
		pEnt = gEntList.FindEntityByName( NULL, STRING(iszEntName) );
		if ( pEnt == NULL )
		{
			DevWarning( "CPeasantRopes::InitRope: bogus entity '%s'\n", iszEntName );
			return NULL;
		}
		pRope = CRopeKeyframe::Create( pEnt, pPeasant, 0, nAttach, 2, "cable/cablerope.vmt", 2 );
		pRope->SetAbsOrigin( pEnt->GetAbsOrigin() );
	}
	else
	{
		trace_t tr;
		UTIL_TraceLine( vAttach, vAttach + Vector(0,0,2048), CONTENTS_SOLID, NULL, COLLISION_GROUP_NONE, &tr );
		if ( !tr.DidHitWorld() )
		{
			DevWarning( "CPeasantRopes::InitRope: can't find solid surface for rope '%s'\n", szAttach );
			return NULL;
		}
		pRope = CRopeKeyframe::CreateWithStartPosition( tr.endpos, pPeasant, nAttach, 2, "cable/cablerope.vmt", 5 );
		pEnt = pRope;
	}

	if ( pRope == NULL )
	{
		DevWarning( "CPeasantRopes::InitRope: failed to create keyframe_rope '%s (%s)'\n", GetDebugName(), szAttach );
		return NULL;
	}

	pRope->SetupHangDistance( 0.0f );
	pRope->EnableWind( false );
	pRope->m_RopeLength = (pEnt->GetAbsOrigin() - vAttach).Length();
	pRope->m_TextureScale.Set( 1.0f );

	return pRope;
}

//-----------------------------------------------------------------------------
void CPeasantRopes::CreateShackle( CBaseAnimating *pPeasant, int nShackle, const char *szAttach )
{
	Assert( nShackle >= 0 && nShackle < ARRAYSIZE(m_hShackle) );

	CBaseAnimating *pAnimating = (CBaseAnimating*)CreateEntityByName( "prop_dynamic" );
	if ( pAnimating )
	{
		pAnimating->SetModel( "models/peasant/shackle.mdl" );
//		pAnimating->SetName( AllocPooledString("Barney_HolsterdColt") );
		int iAttachment = pPeasant->LookupAttachment( szAttach );
		pAnimating->SetParent( pPeasant, iAttachment );
		pAnimating->SetOwnerEntity( pPeasant );
		pAnimating->SetSolid( SOLID_NONE );
		pAnimating->SetLocalOrigin( Vector( 0, 0, 0 ) );
		pAnimating->SetLocalAngles( QAngle( 0, 0, 0 ) );

		pAnimating->AddSpawnFlags( SF_DYNAMICPROP_NO_VPHYSICS );
		pAnimating->AddEffects( EF_PARENT_ANIMATES );

		DispatchSpawn( pAnimating );

		m_hShackle[nShackle] = pAnimating;
	}
}

//-----------------------------------------------------------------------------
void CPeasantRopes::TransferToRagdoll( CBaseEntity *pRagdollEnt )
{
	ragdoll_t *pRagdoll = Ragdoll_GetRagdoll( pRagdollEnt );
	if ( pRagdoll == NULL )
		return;

	SetParent( pRagdollEnt ); // for auto-delete with parent

	TransferRopeToRagdoll( pRagdollEnt->GetBaseAnimating(), 0, "rope_lefthand", pRagdoll->pGroup );
	TransferRopeToRagdoll( pRagdollEnt->GetBaseAnimating(), 1, "rope_righthand", pRagdoll->pGroup );
}

//-----------------------------------------------------------------------------
void CPeasantRopes::TransferRopeToRagdoll( CBaseAnimating *pAnimating, int nRope, const char *szAttach, IPhysicsConstraintGroup *pGroup )
{
	Assert( pAnimating != NULL );
	Assert( nRope >= 0 && nRope < 2 );
	CBaseEntity *pRopeEnt = m_hRope[nRope];
	if ( pRopeEnt == NULL )
		return;

	int nAttach = pAnimating->LookupAttachment( szAttach );
	Vector vAttach;
	QAngle angles;
	pAnimating->GetAttachment( nAttach, vAttach, angles );

	// Transfer the rope
	CRopeKeyframe *pRope = assert_cast<CRopeKeyframe *>( pRopeEnt );
	pRope->SetEndPoint( pAnimating, nAttach );

	// Create a length constraint
	IPhysicsObject *pPhysicsObject = pAnimating->VPhysicsGetObject();
	if ( pPhysicsObject )
	{
		IPhysicsObject *list[VPHYSICS_MAX_OBJECT_LIST_COUNT];
		int listCount = pAnimating->VPhysicsGetObjectList( list, ARRAYSIZE(list) );
		int iPhysicsBone = pAnimating->GetPhysicsBone( pAnimating->GetAttachmentBone( nAttach ) );
		if ( iPhysicsBone < listCount )
		{
			pPhysicsObject = list[iPhysicsBone];
		}

		constraint_lengthparams_t length;
		length.Defaults();
		length.InitWorldspace( g_PhysWorldObject, pPhysicsObject, pRope->GetAbsOrigin(), vAttach );
		m_pConstraint[nRope] = physenv->CreateLengthConstraint( g_PhysWorldObject, pPhysicsObject, pGroup, length );
	}

	// Transfer the shackles
	if ( m_hShackle[nRope] != NULL )
	{
		const char *szAttach[2] = { "shackle_left", "shackle_right" };
		CBaseAnimating *pShackle = m_hShackle[nRope].Get()->GetBaseAnimating();
		if ( pShackle )
		{
			pShackle->SetParent( pAnimating, pAnimating->LookupAttachment( szAttach[nRope] ) );
			pShackle->SetLocalOrigin( Vector( 0, 0, 0 ) );
			pShackle->SetLocalAngles( QAngle( 0, 0, 0 ) );
		}
	}
}

#endif // PEASANT_BYARMS

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class CNPC_Peasant : public CHOEHuman
{
public:
	DECLARE_CLASS( CNPC_Peasant, CHOEHuman );
//	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
	DEFINE_CUSTOM_AI;

	CNPC_Peasant()
	{
		m_nBody = PEASANT_BODY_RANDOM;
	}

	void Spawn( void );
	void Precache( void );
	void SelectModel( void );
	
	void UpdateOnRemove( void );

	Class_T Classify( void );
	bool ClassifyPlayerAlly( void ) { return false; }
	bool ClassifyPlayerAllyVital( void ) { return false; }

	int ObjectCaps( void ) 
	{ 
		// Set a flag indicating this NPC is usable by the player
		int caps = UsableNPCObjectCaps( BaseClass::ObjectCaps() );
		return caps; 
	}

	void SelectModelGroups( void );

	int GetHeadGroupNum( void ) { return PEASANT_BODYGROUP_HEAD; }
	int GetNumHeads( void ) { return PEASANT_NUM_HEADS; }
	const char *GetHeadModelName( void )
	{
		return IsMale() ? "models/peasant/head_male_off.mdl" : "models/peasant/head_female_off.mdl";
	}

	virtual const char *GetHelmetModelName( void )
	{
		return IsMale() ? "models/peasant/hat_male.mdl" : "models/peasant/hat_female.mdl";
	}
	virtual bool DropHelmet( void );

	virtual SpeechManagerID_t GetSpeechManagerID( void ) { return SPEECH_MANAGER_ALLY; }
	const char *GetSentenceGroup( void ) const { return "PS"; }
	const char **GetFriendClasses( void ) const {
		static const char *szClasses[] = {
			"npc_mikeforce",
			"npc_mikeforce_medic",
			"npc_barney",
			"npc_peasant",
			"player",
			NULL
		};
		return szClasses;
	};
	const char **GetWeaponClasses( void ) const {
		static const char *szClasses[] = {
			NULL
		};
		return szClasses;
	};

	const char *GetCCImageNameCStr( void )
	{
		return UTIL_VarArgs( "cc_peasant_%c%d", IsMale() ? 'm' : 'f', m_nHeadNum + 1 );
	}

	void UseFunc( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	Disposition_t IRelationType( CBaseEntity *pTarget );
	int OnTakeDamage_Alive( const CTakeDamageInfo &info );

	virtual bool IdleSpeech( void );
	bool IsMale( void ) { return Q_stristr( STRING( GetModelName() ), "Female" ) == NULL; }
	bool IsScared( void ) { return m_flFearTime > gpGlobals->curtime; }

	void GatherEnemyConditions( CBaseEntity *pEnemy );

	int SelectSchedule( void );
	int TranslateSchedule( int scheduleType );
	Activity NPC_TranslateActivity( Activity eNewActivity );
	void StartTask( const Task_t *pTask );
	void RunTask( const Task_t *pTask );
	void HandleAnimEvent( animevent_t *pEvent );

	void Touch( CBaseEntity *pOther );

	void ModifyOrAppendCriteria( AI_CriteriaSet& set );

#ifdef PEASANT_BYARMS
	void HangingThink( void );
	bool BecomeRagdollOnClient( const Vector &force );

	bool m_bHanging;
	string_t m_nameRope[2];
	int m_nInitialBody;
	CHandle<CPeasantRopes> m_hRopes;
//	Vector m_vSpawnPos;
#endif

	float m_flFearTime;

	//-----------------------------------------------------------------------------
	// Conditions.
	//-----------------------------------------------------------------------------
	enum 
	{
		COND_ENEMY_TOO_CLOSE = BaseClass::NEXT_CONDITION,
		NEXT_CONDITION
	};

	//-----------------------------------------------------------------------------
	// Custom schedules.
	//-----------------------------------------------------------------------------
	enum
	{
		SCHED_PEASANT_STARTLE = BaseClass::NEXT_SCHEDULE,
		SCHED_PEASANT_PANIC,
		SCHED_PEASANT_COWER,
		NEXT_SCHEDULE,
	};

	//-----------------------------------------------------------------------------
	// Tasks
	//-----------------------------------------------------------------------------
	enum
	{
		 TASK_PEASANT_SOUND_SCREAM = BaseClass::NEXT_TASK,
		 TASK_PEASANT_SOUND_FEAR,
		 TASK_PEASANT_SOUND_STARTLE,
		 NEXT_TASK,
	};

	//-----------------------------------------------------------------------------
	// Activities
	//-----------------------------------------------------------------------------
	static Activity ACT_FEAR_DISPLAY;
	static Activity ACT_EXCITED;
};

LINK_ENTITY_TO_CLASS( npc_peasant, CNPC_Peasant );

//IMPLEMENT_SERVERCLASS_ST(CNPC_Peasant, DT_NPC_Charlie)
//END_SEND_TABLE()

BEGIN_DATADESC( CNPC_Peasant )
	DEFINE_FIELD(m_flFearTime, FIELD_TIME),
	DEFINE_USEFUNC( UseFunc ),
#ifdef PEASANT_BYARMS
	DEFINE_KEYFIELD( m_bHanging, FIELD_BOOLEAN, "Hanging" ),
	DEFINE_KEYFIELD( m_nameRope[0], FIELD_STRING, "rope1" ),
	DEFINE_KEYFIELD( m_nameRope[1], FIELD_STRING, "rope2" ),
	DEFINE_FIELD( m_hRopes, FIELD_EHANDLE ),
	DEFINE_THINKFUNC( HangingThink ),
#endif
END_DATADESC()

//-----------------------------------------------------------------------------
// Activities
//-----------------------------------------------------------------------------
Activity CNPC_Peasant::ACT_FEAR_DISPLAY;
Activity CNPC_Peasant::ACT_EXCITED;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Peasant::Spawn( void )
{
	m_spawnEquipment = NULL_STRING;

	m_iHealth = PEASANT_HEALTH;

	BaseClass::Spawn();

	CapabilitiesRemove(
		bits_CAP_USE_WEAPONS |
		bits_CAP_AIM_GUN |
		bits_CAP_SQUAD | bits_CAP_NO_HIT_SQUADMATES |
		bits_CAP_USE_SHOT_REGULATOR );

	m_flFearTime = gpGlobals->curtime;

	SetUse( &CNPC_Peasant::UseFunc );

#ifdef PEASANT_BYARMS
	if ( m_bHanging )
	{
		m_hRopes = CPeasantRopes::CreateRopes( this, m_nameRope );

//		m_vSpawnPos = GetAbsOrigin();

		SetMoveType( MOVETYPE_NONE );
		ResetSequence( LookupSequence( "hanging_idle" ) );
		SetThink( &CNPC_Peasant::HangingThink );
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Peasant::SelectModel()
{
	CFmtStr fmt;
	const char *gender = 0;
	int body = 0;

	// nama1 has totally bogus "body" values for peasant
	if ( m_nBody >= 6 )
		m_nBody = m_nBody % 6;

	// I had to separate the models because of 32-texture limit
	if ( m_nBody == PEASANT_BODY_RANDOM )
	{
		if ( random->RandomInt(0, 1) )
			m_nBody = PEASANT_BODY_RANDOM_MALE;
		else
			m_nBody = PEASANT_BODY_RANDOM_FEMALE;
	}
	if ( m_nBody == PEASANT_BODY_RANDOM_MALE )
	{
		gender = "Male";
		body = random->RandomInt(1, 3);
		m_nHeadNum = random->RandomInt(0, 2);
	}
	if ( m_nBody == PEASANT_BODY_RANDOM_FEMALE )
	{
		gender = "Female";
		body = random->RandomInt(1, 3);
		m_nHeadNum = random->RandomInt(0, 2);
	}
	if ( m_nBody >= 0 && m_nBody <= 3 )
	{
		gender = "Male";
		body = m_nBody + 1;
		m_nHeadNum = random->RandomInt(0, 2);
	}
	if ( m_nBody >= 3 && m_nBody < 6 )
	{
		gender = "Female";
		body = m_nBody - 3 + 1;
		m_nHeadNum = random->RandomInt(0, 2);
	}

	Assert(gender);

#ifdef PEASANT_BYARMS
	m_nInitialBody = m_nBody;
#endif

	m_nBody = 0;
	SetModelName( AllocPooledString( fmt.sprintf( PEASANT_MODEL_STRING, gender, body ) ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Peasant::SelectModelGroups( void )
{
	SetBodygroup( PEASANT_BODYGROUP_HEAD, m_nHeadNum );
}

//-----------------------------------------------------------------------------
bool CNPC_Peasant::DropHelmet( void )
{
	if ( IsMale() && m_nHeadNum != 0 )
		return false;
	if ( !IsMale() && m_nHeadNum == 2 )
		return false;

	SetBodygroup( PEASANT_BODYGROUP_HEAD, m_nHeadNum + 4 );
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Precaches all resources this monster needs.
//-----------------------------------------------------------------------------
void CNPC_Peasant::Precache( void )
{
	PrecacheScriptSound( "NPC_Barney.FootstepLeft" );
	PrecacheScriptSound( "NPC_Barney.FootstepRight" );
	PrecacheScriptSound( "NPC_Barney.RunFootstepLeft" );
	PrecacheScriptSound( "NPC_Barney.RunFootstepRight" );

#ifdef PEASANT_BYARMS
	PrecacheModel( "models/peasant/shackle.mdl" );
#endif

	PrecacheModel( "models/peasant/hat_male.mdl" );
	PrecacheModel( "models/peasant/hat_female.mdl" );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Indicates this monster's place in the relationship table.
// Output : 
//-----------------------------------------------------------------------------
Class_T	CNPC_Peasant::Classify( void )
{
	return CLASS_PEASANT; 
}

//-----------------------------------------------------------------------------
void CNPC_Peasant::UpdateOnRemove( void )
{
#ifdef PEASANT_BYARMS
	UTIL_Remove( m_hRopes );
#endif
	BaseClass::UpdateOnRemove();
}

//-----------------------------------------------------------------------------
void CNPC_Peasant::UseFunc( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	// Can't +USE NPCs running scripts
	if ( IsInAScript() )
		return;

#ifdef PEASANT_BYARMS
	if ( m_bHanging )
		return;
#endif

	if ( !IsMoving() &&
		pCaller && pCaller->IsPlayer() &&
		!HasMemory( bits_MEMORY_PROVOKED ) &&
		OkToShout() )
	{
#ifdef HOE_HUMAN_RR
		SpeechSelection_t selection;
		if ( SelectSpeechResponse( TLK_USE, NULL, selection ) )
		{
			SetTalkTarget( pCaller );
			DispatchSpeechSelection( selection );
			if ( GetSpeechManager() )
				GetSpeechManager()->ExtendSpeechCategoryTimer( SPEECH_CATEGORY_IDLE, 30 );
		}
#else
		if ( IsMale() )
			Speak( "MALE", 0, pCaller );
		else
			Speak( "FEMALE", 0, pCaller );
#endif
	}
}

//-----------------------------------------------------------------------------
Disposition_t CNPC_Peasant::IRelationType( CBaseEntity *pTarget )
{
	if ( pTarget->IsPlayer() && HasMemory( bits_MEMORY_PROVOKED ) )
		return D_FR;

	return BaseClass::IRelationType( pTarget );
}

//-----------------------------------------------------------------------------
int CNPC_Peasant::OnTakeDamage_Alive( const CTakeDamageInfo &info )
{
	if ( GetState() != NPC_STATE_PRONE &&
		 info.GetAttacker() &&
		 info.GetAttacker()->IsPlayer() &&
		 IRelationType( info.GetAttacker() ) != D_FR )
	{
		Remember( bits_MEMORY_PROVOKED );

		// Alert other peasants of the trecherous player. Other NPCs don't much care.
		CBaseEntity *pEntity = NULL;
		while ( ( pEntity = gEntList.FindEntityByClassnameWithin( pEntity, "npc_peasant", GetAbsOrigin(), 768.0f ) ) != 0 )
		{
			if ( pEntity == this )
				continue;

			if ( !pEntity->IsAlive() || pEntity->m_lifeState == LIFE_DYING )
				continue;

			if ( pEntity->MyNPCPointer()->GetState() == NPC_STATE_PRONE )
				continue;

			if ( pEntity->MyNPCPointer()->IsInAScript() )
				continue;

			if ( !FVisible( pEntity) && !pEntity->MyNPCPointer()->FVisible( this ) )
				continue;

			pEntity->MyNPCPointer()->Remember( bits_MEMORY_PROVOKED );
		}
	}

	return BaseClass::OnTakeDamage_Alive( info );
}

//-----------------------------------------------------------------------------
bool CNPC_Peasant::IdleSpeech( void )
{
	CHumanSpeechManager *pSpeechMgr = GetSpeechManager();
	if ( pSpeechMgr && !pSpeechMgr->GetSpeechCategoryTimer( SPEECH_CATEGORY_IDLE ).Expired() )
		return false;

	if ( IsScared() ||
#ifdef HOE_HUMAN_RR
//		SpokeConcept( TLK_IDLE ) ||
#else
		SpokeConcept( HUMAN_SC_IDLE ) ||
#endif
//		random->RandomInt(0, 4) ||
		!OkToSpeak() )
		return false;

	bool bProvokedByPlayer = HasMemory( bits_MEMORY_PROVOKED );
	CBaseEntity *pFriend = FindNearestSpeechTarget( !bProvokedByPlayer );

	if ( pFriend && pFriend->IsPlayer() )
	{
#ifdef HOE_HUMAN_RR
		SpeechSelection_t selection;
		if ( SelectSpeechResponse( TLK_IDLE, NULL, selection ) )
		{
			SetTalkTarget( pFriend );
			DispatchSpeechSelection( selection );
			if ( pSpeechMgr )
				pSpeechMgr->ExtendSpeechCategoryTimer( SPEECH_CATEGORY_IDLE, 30 );
			return true;
		}
#else
		if ( IsMale() )
			Speak( "MALE", HUMAN_SC_IDLE, pFriend );
		else
			Speak( "FEMALE", HUMAN_SC_IDLE, pFriend );
#endif
	}
	return false;
}

//---------------------------------------------------------
void CNPC_Peasant::GatherEnemyConditions( CBaseEntity *pEnemy )
{
	BaseClass::GatherEnemyConditions( pEnemy );

	ClearCondition( COND_ENEMY_TOO_CLOSE );
	float distSqEnemy = ( pEnemy->GetAbsOrigin() - GetAbsOrigin() ).LengthSqr();
	if ( distSqEnemy <= 128.0f )
		SetCondition( COND_ENEMY_TOO_CLOSE );
}

//-----------------------------------------------------------------------------
int CNPC_Peasant::SelectSchedule( void )
{
	// Let the base class handle barnacle interactions
	if ( GetState() == NPC_STATE_PRONE )
		return BaseClass::SelectSchedule();

	if ( HasCondition( COND_HEAR_DANGER ) || HasCondition( COND_HEAR_COMBAT ) )
	{
		m_flFearTime = gpGlobals->curtime + PEASANT_FEAR_TIME;
	}

	if ( ( HasCondition( COND_PLAYER_PUSHING ) ||
		   HasCondition( COND_HEAR_MOVE_AWAY ) ) && !IsInAScript() )
	{
		return SCHED_MOVE_AWAY;
	}

	switch ( GetState() )
	{
	case NPC_STATE_COMBAT:
		{
			if ( HasCondition( COND_SEE_ENEMY ) || HasCondition( COND_NEW_ENEMY ) )
			{
				m_flFearTime = gpGlobals->curtime + PEASANT_FEAR_TIME;
				return BaseClass::SelectSchedule();
			}
			else if ( !IsScared() )
			{
				SetIdealState( NPC_STATE_ALERT );
			}
		}
		/* break; */
	case NPC_STATE_ALERT:
	case NPC_STATE_IDLE:
		{
			if ( HasCondition( COND_ENEMY_DEAD ) )
			{
				m_flFearTime = gpGlobals->curtime;
			}
			if ( !HasCondition( COND_HEAR_DANGER ) &&
				 HasCondition( COND_HEAR_COMBAT ) )
			{
				return SCHED_PEASANT_STARTLE;
			}
		}
		break;
	}
	return BaseClass::SelectSchedule();
}

//-----------------------------------------------------------------------------
int CNPC_Peasant::TranslateSchedule( int scheduleType )
{
	switch( scheduleType )
	{
	case SCHED_COWER:
		return SCHED_PEASANT_COWER;
		break;
	case SCHED_STANDOFF:
		return SCHED_PEASANT_COWER;
		break;
	case SCHED_FAIL:
		return SCHED_PEASANT_PANIC;
		break;
	default:
		return BaseClass::TranslateSchedule( scheduleType );
		break;
	}
}

//-----------------------------------------------------------------------------
Activity CNPC_Peasant::NPC_TranslateActivity( Activity eNewActivity )
{
	if ( eNewActivity == ACT_WALK )
	{
		if ( IsScared() )
		{
			return ACT_WALK_SCARED;
		}
	}
	if ( eNewActivity == ACT_RUN )
	{
		if ( IsScared() )
		{
			return ACT_RUN_SCARED;
		}
	}

	return BaseClass::NPC_TranslateActivity( eNewActivity );
}

//-----------------------------------------------------------------------------
void CNPC_Peasant::StartTask( const Task_t *pTask )
{
	switch( pTask->iTask )
	{
	case TASK_HUMAN_CROUCH: // peasant "crouch" is actually cower in fear
		{
			TaskComplete();
		}
		break;

	case TASK_HUMAN_UNCROUCH:
		{
			TaskComplete();
		}
		break;

	case TASK_SOUND_WAKE:
	case TASK_HUMAN_SOUND_GRENADE:
	case TASK_HUMAN_SOUND_HELP:
	case TASK_HUMAN_SOUND_MEDIC:
	case TASK_HUMAN_SOUND_COVER:
	case TASK_HUMAN_SOUND_FOUND_ENEMY:
//	case TASK_HUMAN_SOUND_STOP_SHOOTING:
	case TASK_PEASANT_SOUND_SCREAM:
		if ( pTask->flTaskData == 0 || random->RandomFloat(0, 1) <= pTask->flTaskData )
		{
			if ( OkToShout() )
				Speak( TLK_PEASANT_SCREAM );
		}
		TaskComplete();
		break;

	case TASK_PEASANT_SOUND_FEAR:
		if ( pTask->flTaskData == 0 || random->RandomFloat(0, 1) <= pTask->flTaskData )
		{
			if ( OkToShout() )
				Speak( TLK_PEASANT_FEAR );
		}
		TaskComplete();
		break;

	case TASK_PEASANT_SOUND_STARTLE:
		if ( pTask->flTaskData == 0 || random->RandomFloat(0, 1) <= pTask->flTaskData )
		{
			if ( OkToShout() )
				Speak( TLK_PEASANT_STARTLE );
		}
		TaskComplete();
		break;

	default:
		BaseClass::StartTask( pTask );
		break;
	}
}

//-----------------------------------------------------------------------------
void CNPC_Peasant::RunTask( const Task_t *pTask )
{
	switch( pTask->iTask )
	{
	case TASK_PLAY_SEQUENCE_FACE_ENEMY:
		// This is to fix SCHED_PEASANT_STARTLE not facing combat noise or attack direction
		// in idle/alert state.
		if ( GetEnemy() == NULL )
		{
			GetMotor()->UpdateYaw();
		}
		BaseClass::RunTask( pTask );
		break;
	default:
		BaseClass::RunTask( pTask );
		break;
	}
}

//-----------------------------------------------------------------------------
void CNPC_Peasant::HandleAnimEvent( animevent_t *pEvent )
{
	BaseClass::HandleAnimEvent( pEvent );
}

//------------------------------------------------------------------------------
void CNPC_Peasant::Touch( CBaseEntity *pOther )
{
	BaseClass::Touch( pOther );

	if ( !IsInAScript() )
	{
		// Did the player touch me?
		if ( pOther->IsPlayer() || ( pOther->VPhysicsGetObject() && (pOther->VPhysicsGetObject()->GetGameFlags() & FVPHYSICS_PLAYER_HELD ) ) )
		{
			TestPlayerPushing( ( pOther->IsPlayer() ) ? pOther : AI_GetSinglePlayer() );
		}
	}
}

//-----------------------------------------------------------------------------
void CNPC_Peasant::ModifyOrAppendCriteria( AI_CriteriaSet& set )
{
	BaseClass::ModifyOrAppendCriteria( set );

	set.AppendCriteria( "gender", IsMale() ? "male" : "female" );
}

#ifdef PEASANT_BYARMS

//-----------------------------------------------------------------------------
void CNPC_Peasant::HangingThink( void )
{
	if ( !UTIL_FindClientInPVS( edict() ) )
	{
		ClearCondition( COND_IN_PVS );
		SetNextThink( gpGlobals->curtime + random->RandomFloat( 1.0f, 1.5f ) );
		return;
	}

	// Setting this condition so attachment points are properly updated.
	// See CBaseAnimating::CanSkipAnimation.
	SetCondition( COND_IN_PVS );

	StudioFrameAdvance();
	DispatchAnimEvents(this);

	SetNextThink( gpGlobals->curtime + 0.1f );
}

//-----------------------------------------------------------------------------
bool CNPC_Peasant::BecomeRagdollOnClient( const Vector &force )
{
#if 1
	if ( m_bHanging && m_hRopes && CanBecomeRagdoll() )
	{
		CBaseEntity *pRagdollEnt = CreateRagdollCorpse( force, false, false );
		if ( pRagdollEnt == NULL )
			return false;

		m_hRopes->TransferToRagdoll( pRagdollEnt );
		m_hRopes = NULL;

		// Get rid of our old body
		// RemoveDeffered lets the client copy the damage decals to the ragdoll
		// from this NPC before the NPC is deleted.
		RemoveDeferred();/*UTIL_Remove(this);*/ 

		return true;
	}

	return BaseClass::BecomeRagdollOnClient( force );
#else
	if ( !CanBecomeRagdoll() ) 
		return false;

	if ( !m_bHanging )
		return BaseClass::BecomeRagdollOnClient( force );

	CBaseEntity *pEnt = CBaseEntity::CreateNoSpawn( "npc_peasant_dead", GetAbsOrigin(), GetAbsAngles() );
	if ( pEnt == NULL )
		return BaseClass::BecomeRagdollOnClient( force );

	// FIXME: should sync animation cycle with ourself
	pEnt->KeyValue( "pose", 7 ); // hanging_idle
	pEnt->KeyValue( "body", m_nInitialBody );
	pEnt->KeyValue( "head", m_nHeadNum );
	pEnt->KeyValue( "rope1", STRING(m_nameRope1) );
	pEnt->KeyValue( "rope2", STRING(m_nameRope2) );

	DispatchSpawn( pEnt );

	// Apply force to ragdoll (see RagdollCreate)
	Vector nudgeForce = force;
	Vector forcePosition = WorldSpaceCenter();
	IPhysicsObject *list[VPHYSICS_MAX_OBJECT_LIST_COUNT];
	int listCount = pEnt->VPhysicsGetObjectList( list, ARRAYSIZE(list) );

	if ( listCount > 0 && list[0] )
	{
		list[0]->ApplyForceCenter( nudgeForce );
		list[0]->GetPosition( &forcePosition, NULL );
	}

	float totalMass = 0;
	for ( int i = 0; i < listCount; i++ )
	{
		if ( list[i] )
			totalMass += list[i]->GetMass();
	}
	for ( int i = 1; i < listCount; i++ )
	{
		if ( list[i] )
		{
			float scale = list[i]->GetMass() / totalMass;
			list[i]->ApplyForceOffset( scale * nudgeForce, forcePosition );
		}
	}

	UTIL_Remove( m_hRope1 );
	UTIL_Remove( m_hRope2 );
	RemoveDeferred();

	return true;
#endif
}

#endif // PEASANT_BYARMS

HOE_BEGIN_CUSTOM_NPC( npc_peasant, CNPC_Peasant )

	DECLARE_CONDITION( COND_ENEMY_TOO_CLOSE )

	DECLARE_TASK( TASK_PEASANT_SOUND_SCREAM )
	DECLARE_TASK( TASK_PEASANT_SOUND_FEAR )
	DECLARE_TASK( TASK_PEASANT_SOUND_STARTLE )

	DECLARE_ACTIVITY( ACT_FEAR_DISPLAY )
	DECLARE_ACTIVITY( ACT_EXCITED )

	DECLARE_SCHEDULE( SCHED_PEASANT_STARTLE )
	DECLARE_SCHEDULE( SCHED_PEASANT_PANIC )
	DECLARE_SCHEDULE( SCHED_PEASANT_COWER )

	LOAD_SCHEDULES_FILE( npc_peasant )
#if 0
	DEFINE_SCHEDULE
	(
		SCHED_PEASANT_STARTLE,

		"	Tasks"
		"		TASK_STOP_MOVING					0"
		"		TASK_PEASANT_SOUND_STARTLE			0"
		"		TASK_PLAY_SEQUENCE_FACE_ENEMY		ACTIVITY:ACT_FEAR_DISPLAY"
		"		TASK_SET_ACTIVITY					ACTIVITY:ACT_IDLE"
		"		TASK_WAIT							2.0"
		"		TASK_WAIT_RANDOM					1.0"
		""
		"	Interrupts"
		"		COND_NEW_ENEMY"
		"		COND_SEE_ENEMY"
		"		COND_HEAR_DANGER"
	)

	DEFINE_SCHEDULE
	(
		SCHED_PEASANT_PANIC,

		"	Tasks"
		"		TASK_STOP_MOVING					0"
		"		TASK_FACE_ENEMY						0"
		"		TASK_PEASANT_SOUND_SCREAM			0.3"
		"		TASK_PLAY_SEQUENCE_FACE_ENEMY		ACTIVITY:ACT_EXCITED"
		"		TASK_SET_ACTIVITY					ACTIVITY:ACT_IDLE"
		""
		"	Interrupts"
	)

	DEFINE_SCHEDULE
	(
		SCHED_PEASANT_COWER,

		"	Tasks"
		"		TASK_STOP_MOVING					0"
		"		TASK_PEASANT_SOUND_FEAR				0.3"
		"		TASK_SET_ACTIVITY					ACTIVITY:ACT_CROUCHIDLE"
		"		TASK_WAIT							2.0"
		"		TASK_WAIT_RANDOM					10.0"
		""
		"	Interrupts"
		"		COND_NEW_ENEMY"
	)
#endif

HOE_END_CUSTOM_NPC()

//-----------------------------------------------------------------------------

class CDeadPeasant : public CHOECorpse
{
public:
	DECLARE_CLASS( CDeadPeasant, CHOECorpse )
#ifdef PEASANT_BYARMS
	DECLARE_DATADESC()
#endif

	int m_nHeadNum;

#ifdef PEASANT_BYARMS
	CDeadPeasant()
	{
		m_nHeadNum = -1;
	}
#endif

	void Spawn( void )
	{
		CorpsePose poses[] = {
			{ "lying_on_back", false },
			{ "lying_on_stomach", false },
			{ "dead_sitting", false },
			{ "dead_hang", true }, // no such sequence???
			{ "dead_table1", false },
			{ "dead_table2", false },
			{ "dead_table3", false },
#ifdef PEASANT_BYARMS
			{ "hanging_idle", true, "rope_lefthand", "rope_righthand" },
#endif
			{ NULL, false }
		};

		SelectModel();

		SetBloodColor( BLOOD_COLOR_RED );
		InitCorpse( STRING( GetModelName() ), poses, AllocPooledString( "npc_peasant" ) );

		BaseClass::Spawn();
	}

	void SelectModel( void )
	{
		CFmtStr fmt;
		const char *gender = 0;
		int body = 0;

		// nama1 has totally bogus "body" values for peasant
		if ( m_nBody >= 6 )
			m_nBody = m_nBody % 6;

		// I had to separate the models because of 32-texture limit
		if ( m_nBody == PEASANT_BODY_RANDOM )
		{
			if ( random->RandomInt(0, 1) )
				m_nBody = PEASANT_BODY_RANDOM_MALE;
			else
				m_nBody = PEASANT_BODY_RANDOM_FEMALE;
		}
		if ( m_nBody == PEASANT_BODY_RANDOM_MALE )
		{
			gender = "Male";
			body = random->RandomInt(1, 3);
		}
		if ( m_nBody == PEASANT_BODY_RANDOM_FEMALE )
		{
			gender = "Female";
			body = random->RandomInt(1, 3);
		}
		if ( m_nBody >= 0 && m_nBody <= 3 )
		{
			gender = "Male";
			body = m_nBody + 1;
		}
		if ( m_nBody >= 3 && m_nBody < 6 )
		{
			gender = "Female";
			body = m_nBody - 3 + 1;
		}

		if ( m_nHeadNum > 2 )
			m_nHeadNum = -1;
		if ( m_nHeadNum < 0 )
			m_nHeadNum = random->RandomInt(0, 2);

		Assert(gender);

		m_nBody = 0;
		SetModelName( AllocPooledString( fmt.sprintf( PEASANT_MODEL_STRING, gender, body ) ) );
	}

	void SelectModelGroups( void )
	{
		SetBodygroup( PEASANT_BODYGROUP_HEAD, m_nHeadNum );
	}
};

LINK_ENTITY_TO_CLASS( npc_peasant_dead, CDeadPeasant );

#ifdef PEASANT_BYARMS
BEGIN_DATADESC( CDeadPeasant )
	DEFINE_KEYFIELD( m_nHeadNum, FIELD_INTEGER, "head" ),
END_DATADESC()
#endif