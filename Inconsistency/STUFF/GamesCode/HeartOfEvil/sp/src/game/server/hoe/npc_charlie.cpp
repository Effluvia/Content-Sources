#include "cbase.h"
#include "hoe_human.h"
#include "hoe_corpse.h"
#include "npcevent.h"
#include "props.h"
#include "schedule_hacks.h"
#include "ai_behavior_follow.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define CHARLIE_HEALTH 60 // 50, 60, 80
#define CHARLIE_MODEL "models/charlie/charlie.mdl"

//=====================
// BodyGroups
//=====================

enum 
{
	CHARLIE_BODYGROUP_HEAD = 0,
	CHARLIE_BODYGROUP_BODY,
//	CHARLIE_BODYGROUP_WEAPON,
};

enum
{
	CHARLIE_BODY_NVA_BROWN = 0,
	CHARLIE_BODY_NVA_GREEN,
	CHARLIE_BODY_VC,
	CHARLIE_BODY_VC2,
};

#define CHARLIE_NUM_HEADS 4

class CNPC_Charlie : public CHOEHuman
{
public:
	DECLARE_CLASS( CNPC_Charlie, CHOEHuman );
//	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
	DEFINE_CUSTOM_AI;

	CNPC_Charlie()
	{
		m_nBody = -1; /* random */
	}

	void Spawn( void );
	void Precache( void );
	void SelectModel();
	
	Class_T Classify( void );
	bool ClassifyPlayerAlly( void ) { return false; }
	bool ClassifyPlayerAllyVital( void ) { return false; }

	void SelectModelGroups( void );
	virtual bool CreateBehaviors( void );

	int GetHeadGroupNum( void ) { return CHARLIE_BODYGROUP_HEAD; }
	int GetNumHeads( void ) { return CHARLIE_NUM_HEADS; }
	const char *GetHeadModelName( void ) { return "models/charlie/charliehead_off.mdl"; }

	virtual const char *GetHelmetModelName( void );
	virtual bool DropHelmet( void );

	virtual SpeechManagerID_t GetSpeechManagerID( void ) { return SPEECH_MANAGER_CHARLIE; }
	const char *GetSentenceGroup( void ) const { return "CH"; }
	const char **GetFriendClasses( void ) const {
		static const char *szClasses[] = {
			"npc_charlie",
			NULL
		};
		return szClasses;
	};
	const char **GetWeaponClasses( void ) const {
		static const char *szClasses[] = {
			"weapon_ak47",
			"weapon_rpg7",
			NULL
		};
		return szClasses;
	};

	const char *GetCCImageNameCStr( void )
	{
		return UTIL_VarArgs( "cc_charlie%d", m_nHeadNum + 1);
	}

	int TranslateSchedule( int scheduleType );
	void HandleAnimEvent( animevent_t *pEvent );

	void Event_Killed( const CTakeDamageInfo &info );
	void DropPropagandaCard( void );

	CAI_FollowBehavior m_FollowBehavior; // added for alamo1 snipers

	static Animevent AE_CHARLIE_DROP_CARD;

	enum {
		SCHED_CHARLIE_VICTORY_DANCE = BaseClass::NEXT_SCHEDULE,
	};
};

LINK_ENTITY_TO_CLASS( npc_charlie, CNPC_Charlie );

//IMPLEMENT_SERVERCLASS_ST(CNPC_Charlie, DT_NPC_Charlie)
//END_SEND_TABLE()

BEGIN_DATADESC( CNPC_Charlie )
END_DATADESC()

Animevent CNPC_Charlie::AE_CHARLIE_DROP_CARD;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Charlie::Spawn( void )
{
	// If spawned by an npc_maker with no weapon specified then give a random weapon
	if ( m_spawnEquipment == NULL_STRING || !Q_strcmp(STRING(m_spawnEquipment), "random") )
	{
		// FIXME: RPG7 wasn't picked in HL1 version
		m_spawnEquipment = AllocPooledString( "weapon_ak47" );
	}
	else if ( !Q_strcmp(STRING(m_spawnEquipment), "none") )
	{
		m_spawnEquipment = NULL_STRING;
	}

	m_iHealth = CHARLIE_HEALTH;

	BaseClass::Spawn();

	CapabilitiesAdd( bits_CAP_MOVE_SHOOT );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Charlie::SelectModel()
{
	SetModelName( AllocPooledString( CHARLIE_MODEL ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Charlie::SelectModelGroups( void )
{
	if ( m_nBody == -1 )
	{
		m_nHeadNum = random->RandomInt( 0, CHARLIE_NUM_HEADS - 1 ); 
	}
	else 
	{
		m_nHeadNum = m_nBody;
	}

	m_nBody = 0;
	SetBodygroup( CHARLIE_BODYGROUP_HEAD, m_nHeadNum );
	SetBodygroup( CHARLIE_BODYGROUP_BODY, m_nHeadNum );
}

//-----------------------------------------------------------------------------
const char *CNPC_Charlie::GetHelmetModelName( void )
{
	switch ( m_nHeadNum )
	{
	case 0:
		return "models/charlie/helmet_Brown.mdl";
		break;
	case 1:
		return "models/charlie/helmet_Green.mdl";
		break;
	case 2:
		return "models/charlie/helmet_VC1.mdl";
		break;
	case 3:
		return "models/charlie/helmet_VC2.mdl";
		break;
	}

	return NULL;
}

//-----------------------------------------------------------------------------
bool CNPC_Charlie::DropHelmet( void )
{
	SetBodygroup( CHARLIE_BODYGROUP_HEAD, m_nHeadNum + 5 );
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Precaches all resources this monster needs.
//-----------------------------------------------------------------------------
void CNPC_Charlie::Precache( void )
{
	UTIL_PrecacheOther( "prop_physics" /*, "models/propagandacard.mdl"*/ );
	PrecacheModel( "models/propagandacard.mdl" );

	PrecacheScriptSound( "NPC_Charlie.FootstepLeft" );
	PrecacheScriptSound( "NPC_Charlie.FootstepRight" );
	PrecacheScriptSound( "NPC_Charlie.RunFootstepLeft" );
	PrecacheScriptSound( "NPC_Charlie.RunFootstepRight" );

	PrecacheModel( "models/charlie/helmet_Brown.mdl" );
	PrecacheModel( "models/charlie/helmet_Green.mdl" );
	PrecacheModel( "models/charlie/helmet_VC1.mdl" );
	PrecacheModel( "models/charlie/helmet_VC2.mdl" );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Indicates this monster's place in the relationship table.
// Output : 
//-----------------------------------------------------------------------------
Class_T	CNPC_Charlie::Classify( void )
{
	return CLASS_CHARLIE; 
}

//-----------------------------------------------------------------------------
bool CNPC_Charlie::CreateBehaviors( void )
{
	AddBehavior( &m_FollowBehavior );
	
	return BaseClass::CreateBehaviors();
}

//-----------------------------------------------------------------------------
int CNPC_Charlie::TranslateSchedule( int scheduleType )
{
	switch ( scheduleType )
	{
	case SCHED_VICTORY_DANCE:
		return SCHED_CHARLIE_VICTORY_DANCE;
	default:
		return BaseClass::TranslateSchedule( scheduleType );
	}
}

//-----------------------------------------------------------------------------
void CNPC_Charlie::HandleAnimEvent( animevent_t *pEvent )
{
	if ( pEvent->event == AE_CHARLIE_DROP_CARD )
	{
		DropPropagandaCard( );
		return;
	}

	return BaseClass::HandleAnimEvent( pEvent );
}

//-----------------------------------------------------------------------------
void CNPC_Charlie::DropPropagandaCard( void )
{
	Vector origin;
	QAngle angles;
	GetAttachment( LookupAttachment( "lefthand" ), origin, angles );

	Vector forward, right, up;
	AngleVectors( GetAbsAngles(), &forward, &right, &up );

//	Vector velocity = forward * 100 + up * 100;
//	origin += forward * 8;

	CPhysicsProp *pGib = assert_cast<CPhysicsProp*>(CreateEntityByName( "prop_physics" ));
	pGib->SetAbsOrigin( origin );
	pGib->SetAbsAngles( angles );
//	pGib->SetAbsVelocity( velocity );
	pGib->SetModel( "models/propagandacard.mdl" );
	pGib->Spawn();
	pGib->SetMoveType( MOVETYPE_VPHYSICS );
}

//-----------------------------------------------------------------------------
void CNPC_Charlie::Event_Killed( const CTakeDamageInfo &info )
{
	if ( random->RandomInt(0, 20) == 0 )
		DropPropagandaCard();

	BaseClass::Event_Killed( info );
}

HOE_BEGIN_CUSTOM_NPC( npc_charlie, CNPC_Charlie )
	DECLARE_ANIMEVENT( AE_CHARLIE_DROP_CARD )
	DECLARE_SCHEDULE( SCHED_CHARLIE_VICTORY_DANCE )
	LOAD_SCHEDULES_FILE( npc_charlie )
HOE_END_CUSTOM_NPC()

//-----------------------------------------------------------------------------

#define CHARLIE_MODEL_BYARMS "models/charlie/charlie_byarms.mdl"
#define CHARLIE_MODEL_BYFEET "models/charlie/charlie_byfeet.mdl"

class CDeadCharlie : public CHOECorpse
{
public:
	DECLARE_CLASS( CDeadCharlie, CHOECorpse );

	void Spawn( void )
	{
		CorpsePose poses[] = {
			{ "lying_on_back", false },
			{ "lying_on_side", false },
			{ "lying_on_stomach", false },
			{ "hanging_byfeet",	true, "rope_feet", NULL },
			{ "hanging_byarms", true, "lefthand", "rope_righthand" },
			{ "hanging_byneck", true, "rope_neck", NULL },
			{ "deadsitting", false },
			{ "deadseated", false },
			{ NULL, false }
		};

		// The "by arms" ragdoll allows the arms to rotate an extreme amount.
		const char *szModel = CHARLIE_MODEL_BYARMS;

		// The "by feet" ragdoll disables collisions between the legs.
		if ( m_pose == 3 )
			szModel = CHARLIE_MODEL_BYFEET;

		SetBloodColor( BLOOD_COLOR_RED );
		InitCorpse( szModel, poses, AllocPooledString( "npc_charlie" ) );

		BaseClass::Spawn();
	}

	void SelectModelGroups( void )
	{
		int nHeadNum;
		if ( m_nBody == -1 )
		{
			nHeadNum = random->RandomInt( 0, CHARLIE_NUM_HEADS - 1 ); 
		}
		else 
		{
			nHeadNum = m_nBody;
		}

		m_nBody = 0;
		SetBodygroup( CHARLIE_BODYGROUP_BODY, nHeadNum );

		// Remove the helmet when hanging upside down or 25% of the time
		if ( m_pose == 3 /*|| !random->RandomInt(0, 3)*/ )
			nHeadNum += 5;

		SetBodygroup( CHARLIE_BODYGROUP_HEAD, nHeadNum );
	}
};

LINK_ENTITY_TO_CLASS( npc_charlie_dead, CDeadCharlie );
