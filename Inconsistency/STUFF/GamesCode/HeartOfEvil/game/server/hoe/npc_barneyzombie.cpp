#include "cbase.h"
#include "hoe_base_zombie.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define BZOMBIE_HEALTH 600 // 400, 600, 800
#define BZOMBIE_MODEL "models/barneyzombie/barneyzombie.mdl"

//=====================
// BodyGroups
//=====================

enum 
{
	BZOMBIE_BODYGROUP_HEAD = 0,
	BZOMBIE_BODYGROUP_TORSO,
};

#define BZOMBIE_NUM_HEADS 1

class CNPC_BarneyZombie : public CHOEBaseZombie
{
public:
	DECLARE_CLASS( CNPC_BarneyZombie, CHOEBaseZombie );
//	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
	DEFINE_CUSTOM_AI;

	void Spawn( void );
	void Precache( void );
	void SelectModel();
	void SelectModelGroups( void );

	Class_T Classify( void );
	bool ClassifyPlayerAlly( void ) { return true; }
	bool ClassifyPlayerAllyVital( void ) { return true; }

	bool CheckFollowPlayer( void ) { return true; }

	const char *GetSentenceGroup( void ) const { return "SZ"; }

	int GetHeadGroupNum( void ) { return BZOMBIE_BODYGROUP_HEAD; }
	int GetNumHeads( void ) { return BZOMBIE_NUM_HEADS; }
	const char *GetHeadModelName( void ) { return "models/barneyzombiehead.mdl"; }

	int ObjectCaps( void ) ;

	void InputGiveHead( inputdata_t &inputdata );
};

LINK_ENTITY_TO_CLASS( npc_barneyzombie, CNPC_BarneyZombie );

//IMPLEMENT_SERVERCLASS_ST(CNPC_BarneyZombie, DT_NPC_HGrunt)
//END_SEND_TABLE()

BEGIN_DATADESC( CNPC_BarneyZombie )
	DEFINE_INPUTFUNC( FIELD_VOID, "GiveHead", InputGiveHead ),
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_BarneyZombie::Spawn( void )
{
	m_spawnEquipment = AllocPooledString( "weapon_colt1911A1" );

	m_iHealth = BZOMBIE_HEALTH;

	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_BarneyZombie::SelectModel()
{
	SetModelName( AllocPooledString( BZOMBIE_MODEL ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_BarneyZombie::SelectModelGroups( void )
{
	m_nBody = 0;
	m_nHeadNum = 1;
	SetBodygroup( BZOMBIE_BODYGROUP_HEAD, m_nHeadNum ); // hack -- start with no head
	SetBodygroup( BZOMBIE_BODYGROUP_TORSO, 0 );
}

//-----------------------------------------------------------------------------
// Purpose: Precaches all resources this monster needs.
//-----------------------------------------------------------------------------
void CNPC_BarneyZombie::Precache( void )
{
	PrecacheScriptSound( "NPC_Spx.FootstepLeft" );
	PrecacheScriptSound( "NPC_Spx.FootstepRight" );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Indicates this monster's place in the relationship table.
// Output : 
//-----------------------------------------------------------------------------
Class_T	CNPC_BarneyZombie::Classify( void )
{
	return CLASS_BARNEY_ZOMBIE;
}

//-----------------------------------------------------------------------------
int CNPC_BarneyZombie::ObjectCaps( void ) 
{
	// Set a flag indicating this NPC is usable by the player
	int caps = UsableNPCObjectCaps( BaseClass::ObjectCaps() );
	if ( !HasAHead() )
		caps &= ~FCAP_IMPULSE_USE;
	return caps; 
}

//-----------------------------------------------------------------------------
void CNPC_BarneyZombie::InputGiveHead( inputdata_t &inputdata )
{
	m_nHeadNum = 0;
	SetBodygroup( BZOMBIE_BODYGROUP_HEAD, m_nHeadNum );
//	m_FollowBehavior.SetFollowTarget( UTIL_GetLocalPlayer() );
}

AI_BEGIN_CUSTOM_NPC( npc_barneyzombie, CNPC_BarneyZombie )
AI_END_CUSTOM_NPC()
