#include "cbase.h"
#include "hoe_human_medic.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define HGRUNT_MEDIC_HEALTH 60 // 50, 60, 80
#define HGRUNT_MODEL "models/gruntmedic/gruntmedic.mdl"

//=====================
// BodyGroups
//=====================

enum 
{
	HGRUNT_MEDIC_BODYGROUP_HEAD = 0,
	HGRUNT_MEDIC_BODYGROUP_BODY,
//	HGRUNT_MEDIC_BODYGROUP_WEAPON,
	HGRUNT_MEDIC_BODYGROUP_SYRINGE,
};

enum
{
	HGRUNT_MEDIC_BODY_HEAD_BLACK = 0,
	HGRUNT_MEDIC_BODY_HEAD_WHITE,
};

enum
{
	HGRUNT_MEDIC_SKIN_WHITE = 0,
	HGRUNT_MEDIC_SKIN_BLACK,
};

#define HGRUNT_MEDIC_NUM_HEADS 2

class CNPC_HGruntMedic : public CHOEHumanMedic
{
public:
	DECLARE_CLASS( CNPC_HGruntMedic, CHOEHumanMedic );
//	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
	DEFINE_CUSTOM_AI;

	CNPC_HGruntMedic()
	{
		m_nBody = -1; /* random */
	}

	void Spawn( void );
	void Precache( void );
	void SelectModel();
	
	Class_T Classify( void );
	bool ClassifyPlayerAlly( void ) { return HasSpawnFlags( SF_HUMAN_FRIENDLY ); }
	bool ClassifyPlayerAllyVital( void ) { return false; }

	void SelectModelGroups( void );

	int GetHeadGroupNum( void ) { return HGRUNT_MEDIC_BODYGROUP_HEAD; }
	int GetNumHeads( void ) { return HGRUNT_MEDIC_NUM_HEADS; }
	const char *GetHeadModelName( void ) { return "models/armymedic/head_off.mdl"; }

	virtual const char *GetHelmetModelName( void ) { return "models/armymedic/helmet.mdl"; }
	virtual bool DropHelmet( void );

	virtual SpeechManagerID_t GetSpeechManagerID( void ) { return HasSpawnFlags( SF_HUMAN_FRIENDLY ) ? SPEECH_MANAGER_ALLY : SPEECH_MANAGER_GRUNT; }
	const char *GetSentenceGroup( void ) const { return "HG"; }
	const char **GetFriendClasses( void ) const {
		static const char *szClasses[] = {
			"npc_grunt_medic",
			"npc_human_grunt",
			NULL
		};
		return szClasses;
	};
	const char **GetWeaponClasses( void ) const {
		static const char *szClasses[] = {
			"weapon_colt1911A1",
			NULL
		};
		return szClasses;
	};

	const char *GetCCImageNameCStr( void )
	{
		return UTIL_VarArgs( "cc_medic%d", m_nHeadNum + 1 );
	}
};

LINK_ENTITY_TO_CLASS( npc_grunt_medic, CNPC_HGruntMedic );

//IMPLEMENT_SERVERCLASS_ST(CNPC_HGruntMedic, DT_NPC_HGruntMedic)
//END_SEND_TABLE()

BEGIN_DATADESC( CNPC_HGruntMedic )
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_HGruntMedic::Spawn( void )
{
	// HL1 monstermaker would call ApplyDefaultSettings, but npc_maker should
	// be using a template NPC which lets us specify a weapon.
	// If spawned by an npc_maker with no weapon specified then give a random weapon
	if ( m_spawnEquipment == NULL_STRING || !Q_strcmp(STRING(m_spawnEquipment), "random") )
	{
		m_spawnEquipment = AllocPooledString( "weapon_colt1911A1" );
	}
	else if ( !Q_strcmp(STRING(m_spawnEquipment), "none") )
	{
		m_spawnEquipment = NULL_STRING;
	}

	m_iHealth = HGRUNT_MEDIC_HEALTH;

	BaseClass::Spawn();

	CapabilitiesAdd( bits_CAP_MOVE_SHOOT );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_HGruntMedic::SelectModel()
{
	SetModelName( AllocPooledString( HGRUNT_MODEL ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_HGruntMedic::SelectModelGroups( void )
{
	if ( m_nBody == -1 )
	{
		m_nHeadNum = random->RandomInt( 0, HGRUNT_MEDIC_NUM_HEADS - 1 ); 
	}
	else 
	{
		m_nHeadNum = m_nBody;
	}

	if ( m_nHeadNum == HGRUNT_MEDIC_BODY_HEAD_BLACK )
	{
		m_nSkin = HGRUNT_MEDIC_SKIN_BLACK;
	}
	else
	{
		m_nSkin = HGRUNT_MEDIC_SKIN_WHITE;
	}

	m_nBody = 0;
	SetBodygroup( HGRUNT_MEDIC_BODYGROUP_HEAD, m_nHeadNum );
}


//-----------------------------------------------------------------------------
bool CNPC_HGruntMedic::DropHelmet( void )
{
	SetBodygroup( HMEDIC_BODYGROUP_SYRINGE, 2 );
	SetBodygroup( HGRUNT_MEDIC_BODYGROUP_HEAD, m_nHeadNum + 3 );
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Precaches all resources this monster needs.
//-----------------------------------------------------------------------------
void CNPC_HGruntMedic::Precache( void )
{
	PrecacheScriptSound( "NPC_HGrunt.FootstepLeft" );
	PrecacheScriptSound( "NPC_HGrunt.FootstepRight" );
	PrecacheScriptSound( "NPC_HGrunt.RunFootstepLeft" );
	PrecacheScriptSound( "NPC_HGrunt.RunFootstepRight" );

	PrecacheModel( "models/armymedic/helmet.mdl" );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Indicates this monster's place in the relationship table.
// Output : 
//-----------------------------------------------------------------------------
Class_T	CNPC_HGruntMedic::Classify( void )
{
	if ( HasSpawnFlags( SF_HUMAN_FRIENDLY ) )
		return CLASS_GRUNT_MEDIC_FRIEND;
	
	return CLASS_GRUNT_MEDIC; 
}

AI_BEGIN_CUSTOM_NPC( npc_grunt_medic, CNPC_HGruntMedic )
AI_END_CUSTOM_NPC()
