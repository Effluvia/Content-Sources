#include "cbase.h"
#include "hoe_human_medic.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define ARMY_MEDIC_HEALTH 60 // 50, 60, 80
#define ARMY_MEDIC_MODEL "models/armymedic/armymedic.mdl"

//=====================
// BodyGroups
//=====================

enum 
{
	ARMY_MEDIC_BODYGROUP_HEAD = 0,
	ARMY_MEDIC_BODYGROUP_BODY,
//	ARMY_MEDIC_BODYGROUP_WEAPON,
	ARMY_MEDIC_BODYGROUP_SYRINGE,
};

enum
{
	ARMY_MEDIC_BODY_HEAD_BLACK = 0,
	ARMY_MEDIC_BODY_HEAD_WHITE,
};

enum
{
	ARMY_MEDIC_SKIN_WHITE = 0,
	ARMY_MEDIC_SKIN_BLACK,
};

#define ARMY_MEDIC_NUM_HEADS 2

class CNPC_ArmyMedic : public CHOEHumanMedic
{
public:
	DECLARE_CLASS( CNPC_ArmyMedic, CHOEHumanMedic );
//	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
	DEFINE_CUSTOM_AI;

	CNPC_ArmyMedic()
	{
		m_nBody = -1; /* random */
	}

	void Spawn( void );
	void Precache( void );
	void SelectModel();
	
	Class_T Classify( void );
	bool ClassifyPlayerAlly( void ) { return true; }
	bool ClassifyPlayerAllyVital( void ) { return false; }

	void SelectModelGroups( void );

	int GetHeadGroupNum( void ) { return ARMY_MEDIC_BODYGROUP_HEAD; }
	int GetNumHeads( void ) { return ARMY_MEDIC_NUM_HEADS; }
	const char *GetHeadModelName( void ) { return "models/armymedic/head_off.mdl"; }

	virtual const char *GetHelmetModelName( void ) { return "models/armymedic/helmet.mdl"; }
	virtual bool DropHelmet( void );

	virtual SpeechManagerID_t GetSpeechManagerID( void ) { return SPEECH_MANAGER_ALLY; }
	const char *GetSentenceGroup( void ) const { return "MF"; }
	const char **GetFriendClasses( void ) const {
		static const char *szClasses[] = {
			"npc_barney",
			"npc_mikeforce",
			"npc_mikeforce_medic",
//			"npc_peasant",
			"player",
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

LINK_ENTITY_TO_CLASS( npc_mikeforce_medic, CNPC_ArmyMedic );

//IMPLEMENT_SERVERCLASS_ST(CNPC_ArmyMedic, DT_NPC_ArmyMedic)
//END_SEND_TABLE()

BEGIN_DATADESC( CNPC_ArmyMedic )
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_ArmyMedic::Spawn( void )
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

	m_iHealth = ARMY_MEDIC_HEALTH;

	BaseClass::Spawn();

	CapabilitiesAdd( bits_CAP_MOVE_SHOOT );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_ArmyMedic::SelectModel()
{
	SetModelName( AllocPooledString( ARMY_MEDIC_MODEL ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_ArmyMedic::SelectModelGroups( void )
{
	if ( m_nBody == -1 )
	{
		m_nHeadNum = random->RandomInt( 0, ARMY_MEDIC_NUM_HEADS - 1 ); 
	}
	else 
	{
		m_nHeadNum = m_nBody;
	}

	if ( m_nHeadNum == ARMY_MEDIC_BODY_HEAD_BLACK )
	{
		m_nSkin = ARMY_MEDIC_SKIN_BLACK;
	}
	else
	{
		m_nSkin = ARMY_MEDIC_SKIN_WHITE;
	}

	m_nBody = 0;
	SetBodygroup( ARMY_MEDIC_BODYGROUP_HEAD, m_nHeadNum );
}

//-----------------------------------------------------------------------------
bool CNPC_ArmyMedic::DropHelmet( void )
{
	SetBodygroup( HMEDIC_BODYGROUP_SYRINGE, 2 );
	SetBodygroup( ARMY_MEDIC_BODYGROUP_HEAD, m_nHeadNum + 3 );
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Precaches all resources this monster needs.
//-----------------------------------------------------------------------------
void CNPC_ArmyMedic::Precache( void )
{
	PrecacheScriptSound( "NPC_MikeForce.FootstepLeft" );
	PrecacheScriptSound( "NPC_MikeForce.FootstepRight" );
	PrecacheScriptSound( "NPC_MikeForce.RunFootstepLeft" );
	PrecacheScriptSound( "NPC_MikeForce.RunFootstepRight" );

	PrecacheModel( "models/armymedic/helmet.mdl" );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Indicates this monster's place in the relationship table.
// Output : 
//-----------------------------------------------------------------------------
Class_T	CNPC_ArmyMedic::Classify( void )
{
	return CLASS_MIKEFORCE_MEDIC;
}

AI_BEGIN_CUSTOM_NPC( npc_mikeforce_medic, CNPC_ArmyMedic )
AI_END_CUSTOM_NPC()
