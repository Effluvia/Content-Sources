#include "cbase.h"
#include "hoe_human.h"
#include "hoe_corpse.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define HGRUNT_HEALTH 60 // 50, 60, 80
#define HGRUNT_MODEL "models/namGrunt/namGrunt.mdl"

//=====================
// BodyGroups
//=====================

enum 
{
	HGRUNT_BODYGROUP_HEAD = 0,
	HGRUNT_BODYGROUP_TORSO,
	HGRUNT_BODYGROUP_ARMS,
	HGRUNT_BODYGROUP_LEGS,
	HGRUNT_BODYGROUP_WEAPON,
};

enum
{
	HGRUNT_BODY_HEAD_HELMET = 0,
	HGRUNT_BODY_HEAD_BLACK,
	HGRUNT_BODY_HEAD_BOONIE,
	HGRUNT_BODY_HEAD_CAP,
	HGRUNT_BODY_HEAD_ANGRY,
	HGRUNT_BODY_HEAD_HELMET_EVIL,
	HGRUNT_BODY_HEAD_HELMET2_EVIL,
	HGRUNT_BODY_HEAD_BOONIE_EVIL,
	HGRUNT_BODY_HEAD_CAP_EVIL,
};

enum
{
	HGRUNT_SKIN_WHITE = 0,
	HGRUNT_SKIN_BLACK,
};

#define HGRUNT_NUM_HEADS 9

class CNPC_HGrunt : public CHOEHuman
{
public:
	DECLARE_CLASS( CNPC_HGrunt, CHOEHuman );
//	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
	DEFINE_CUSTOM_AI;

	CNPC_HGrunt()
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

	int GetHeadGroupNum( void ) { return HGRUNT_BODYGROUP_HEAD; }
	int GetNumHeads( void ) { return HGRUNT_NUM_HEADS; }
	const char *GetHeadModelName( void ) { return "models/namgrunt/head_off.mdl"; }

	virtual const char *GetHelmetModelName( void );
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
			"weapon_870",
			"weapon_m16",
			"weapon_m79",
			NULL
		};
		return szClasses;
	};

	const char *GetCCImageNameCStr( void )
	{
		return UTIL_VarArgs( "cc_grunt%d", m_nHeadNum + 1 );
	}

	int ObjectCaps( void );
	void UseFunc( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
};

LINK_ENTITY_TO_CLASS( npc_human_grunt, CNPC_HGrunt );

//IMPLEMENT_SERVERCLASS_ST(CNPC_HGrunt, DT_NPC_HGrunt)
//END_SEND_TABLE()

BEGIN_DATADESC( CNPC_HGrunt )
	DEFINE_USEFUNC( UseFunc ),
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_HGrunt::Spawn( void )
{
	// HL1 monstermaker would call ApplyDefaultSettings, but npc_maker should
	// be using a template NPC which lets us specify a weapon.
	// If spawned by an npc_maker with no weapon specified then give a random weapon
	if ( m_spawnEquipment == NULL_STRING || !Q_strcmp(STRING(m_spawnEquipment), "random") )
	{
		switch ( random->RandomInt( 0, 5 ) )
		{
		case 0:
		case 1:
		case 2:
			m_spawnEquipment = AllocPooledString( "weapon_m16" );
			break;
		case 3:
		case 4:
			m_spawnEquipment = AllocPooledString( "weapon_870" );
			break;
		case 5:
			m_spawnEquipment = AllocPooledString( "weapon_m79" );
			break;
		}
	}
	else if ( !Q_strcmp(STRING(m_spawnEquipment), "none") )
	{
		m_spawnEquipment = NULL_STRING;
	}

	m_iHealth = HGRUNT_HEALTH;

	BaseClass::Spawn();

	CapabilitiesAdd( bits_CAP_MOVE_SHOOT );

	SetUse( &CNPC_HGrunt::UseFunc );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_HGrunt::SelectModel()
{
	SetModelName( AllocPooledString( HGRUNT_MODEL ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_HGrunt::SelectModelGroups( void )
{
	if ( m_nBody == -1 )
	{
		if ( HasSpawnFlags( SF_HUMAN_FRIENDLY ) )
		{
			m_nHeadNum = random->RandomInt( 0, 4 );// If I am friendly, don't use the evil heads
		}
		else
		{
			m_nHeadNum = random->RandomInt( 0, HGRUNT_NUM_HEADS - 1 ); 
		}
	}
	else 
	{
		m_nHeadNum = m_nBody;
	}

	if ( m_nHeadNum == HGRUNT_BODY_HEAD_BLACK )
	{
		m_nSkin = HGRUNT_SKIN_BLACK;
	}
	else
	{
		m_nSkin = HGRUNT_SKIN_WHITE;
	}

	m_nBody = 0;
	SetBodygroup( HGRUNT_BODYGROUP_HEAD, m_nHeadNum );

	switch ( m_nHeadNum )
	{
	case HGRUNT_BODY_HEAD_HELMET:
	case HGRUNT_BODY_HEAD_BLACK: 
	case HGRUNT_BODY_HEAD_HELMET_EVIL:
	case HGRUNT_BODY_HEAD_HELMET2_EVIL:
		SetBodygroup( HGRUNT_BODYGROUP_TORSO, 0 );
		break;
	
	case HGRUNT_BODY_HEAD_CAP:
	case HGRUNT_BODY_HEAD_CAP_EVIL:
		SetBodygroup( HGRUNT_BODYGROUP_TORSO, 1 );
		break;
	
	case HGRUNT_BODY_HEAD_BOONIE:
	case HGRUNT_BODY_HEAD_BOONIE_EVIL:
	case HGRUNT_BODY_HEAD_ANGRY:
		SetBodygroup( HGRUNT_BODYGROUP_TORSO, 2 );
		break;
	}
}

//-----------------------------------------------------------------------------
const char *CNPC_HGrunt::GetHelmetModelName( void )
{
	switch ( m_nHeadNum )
	{
	case HGRUNT_BODY_HEAD_HELMET:
	case HGRUNT_BODY_HEAD_BLACK: 
	case HGRUNT_BODY_HEAD_HELMET_EVIL:
	case HGRUNT_BODY_HEAD_HELMET2_EVIL:
		return "models/namGrunt/helmet_helmet.mdl";
		break;
	
	case HGRUNT_BODY_HEAD_CAP:
	case HGRUNT_BODY_HEAD_CAP_EVIL:
		return "models/namGrunt/helmet_cap.mdl";
		break;
	
	case HGRUNT_BODY_HEAD_BOONIE:
	case HGRUNT_BODY_HEAD_BOONIE_EVIL:
	case HGRUNT_BODY_HEAD_ANGRY:
		return "models/namGrunt/helmet_boonie.mdl";
		break;
	}

	return NULL;
}

//-----------------------------------------------------------------------------
bool CNPC_HGrunt::DropHelmet( void )
{
	SetBodygroup( HGRUNT_BODYGROUP_HEAD, m_nHeadNum + 10 );
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Precaches all resources this monster needs.
//-----------------------------------------------------------------------------
void CNPC_HGrunt::Precache( void )
{
	PrecacheScriptSound( "NPC_HGrunt.FootstepLeft" );
	PrecacheScriptSound( "NPC_HGrunt.FootstepRight" );
	PrecacheScriptSound( "NPC_HGrunt.RunFootstepLeft" );
	PrecacheScriptSound( "NPC_HGrunt.RunFootstepRight" );

	PrecacheModel( "models/namGrunt/helmet_helmet.mdl" );
	PrecacheModel( "models/namGrunt/helmet_cap.mdl" );
	PrecacheModel( "models/namGrunt/helmet_boonie.mdl" );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Indicates this monster's place in the relationship table.
// Output : 
//-----------------------------------------------------------------------------
Class_T	CNPC_HGrunt::Classify( void )
{
	if ( HasSpawnFlags( SF_HUMAN_FRIENDLY ) )
		return CLASS_HUMAN_GRUNT_FRIEND;
	
	return CLASS_HUMAN_GRUNT; 
}

//-----------------------------------------------------------------------------
int CNPC_HGrunt::ObjectCaps( void ) 
{ 
	int caps = BaseClass::ObjectCaps();

	if ( HasSpawnFlags( SF_HUMAN_FRIENDLY ) )
	{
		// Set a flag indicating this NPC is usable by the player
		caps = UsableNPCObjectCaps( caps );
	}
	return caps; 
}

//-----------------------------------------------------------------------------
void CNPC_HGrunt::UseFunc( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	// Can't +USE NPCs running scripts
	if ( IsInAScript() )
		return;

	if ( !HasSpawnFlags( SF_HUMAN_FRIENDLY ) )
		return;

	if ( !IsMoving() &&
		pCaller && pCaller->IsPlayer() &&
		!HasMemory( bits_MEMORY_PROVOKED ) &&
		OkToShout() )
	{
		SpeechSelection_t selection;
		if ( SelectSpeechResponse( TLK_USE, NULL, selection ) )
		{
			SetTalkTarget( pCaller );
			DispatchSpeechSelection( selection );
			if ( GetSpeechManager() )
				GetSpeechManager()->ExtendSpeechCategoryTimer( SPEECH_CATEGORY_IDLE, 30 );
		}
	}
}

AI_BEGIN_CUSTOM_NPC( npc_human_grunt, CNPC_HGrunt )
AI_END_CUSTOM_NPC()

//-----------------------------------------------------------------------------

#define HGRUNT_MODEL_BYARMS "models/namGrunt/namGrunt_byarms.mdl"
#define HGRUNT_MODEL_BYFEET "models/namGrunt/namGrunt_byfeet.mdl"
#define HGRUNT_MODEL_HEADLESS "models/namGrunt/namGrunt_headless.mdl"

#define SF_CORPSE_HEADLESS (1 << 18)

class CDeadHGrunt : public CHOECorpse
{
public:
	DECLARE_CLASS( CDeadHGrunt, CHOECorpse );

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
		const char *szModel = HGRUNT_MODEL_BYARMS;

		// The "by feet" ragdoll disables collisions between the legs.
		if ( m_pose == 3 )
			szModel = HGRUNT_MODEL_BYFEET;

		// The original game had a few "headless" corpses where the head was jammed into a wall.
		// That's no good for ragdoll corpses.  This ragdoll has no head physics and only a neck
		// in HGRUNT_BODYGROUP_HEAD.
		if ( HasSpawnFlags( SF_CORPSE_HEADLESS ) )
			szModel = HGRUNT_MODEL_HEADLESS;

		SetBloodColor( BLOOD_COLOR_RED );
		InitCorpse( szModel, poses, AllocPooledString( "npc_human_grunt" ) );

		BaseClass::Spawn();
	}

	void SelectModelGroups( void )
	{
		int headNum;

		if ( m_nBody == -1 )
		{
			headNum = random->RandomInt( 0, HGRUNT_NUM_HEADS - 1 ); 
		}
		else 
		{
			headNum = m_nBody;
		}

		if ( headNum == HGRUNT_BODY_HEAD_BLACK )
		{
			m_nSkin = HGRUNT_SKIN_BLACK;
		}
		else
		{
			m_nSkin = HGRUNT_SKIN_WHITE;
		}

		m_nBody = 0;

		switch ( headNum )
		{
		case HGRUNT_BODY_HEAD_HELMET:
		case HGRUNT_BODY_HEAD_BLACK: 
		case HGRUNT_BODY_HEAD_HELMET_EVIL:
		case HGRUNT_BODY_HEAD_HELMET2_EVIL:
			SetBodygroup( HGRUNT_BODYGROUP_TORSO, 0 );
			break;
		
		case HGRUNT_BODY_HEAD_CAP:
		case HGRUNT_BODY_HEAD_CAP_EVIL:
			SetBodygroup( HGRUNT_BODYGROUP_TORSO, 1 );
			break;
		
		case HGRUNT_BODY_HEAD_BOONIE:
		case HGRUNT_BODY_HEAD_BOONIE_EVIL:
		case HGRUNT_BODY_HEAD_ANGRY:
			SetBodygroup( HGRUNT_BODYGROUP_TORSO, 2 );
			break;
		}

		if ( HasSpawnFlags( SF_CORPSE_HEADLESS ) == false )
		{
			// Remove the helmet when hanging upside down or 25% of the time
			if ( m_pose == 3 /*|| !random->RandomInt(0, 3)*/ )
				headNum += 10;

			SetBodygroup( HGRUNT_BODYGROUP_HEAD, headNum );
		}
	}
};

LINK_ENTITY_TO_CLASS( npc_hgrunt_dead, CDeadHGrunt );
