#include "cbase.h"
#include "hoe_human.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define SARGE_HEALTH 10
#define SARGE_MODEL "models/sarge.mdl"
#define SARGE_NUM_HEADS 1

class CNPC_Sarge : public CHOEHuman
{
public:
	DECLARE_CLASS( CNPC_Sarge, CHOEHuman );

	CNPC_Sarge()
	{
	}

	void Spawn( void );
	void Precache( void );
	void SelectModel();
	
	Class_T Classify( void );
	bool ClassifyPlayerAlly( void ) { return false; }
	bool ClassifyPlayerAllyVital( void ) { return false; }

#if 0 // copy-paste error?
	int ObjectCaps( void ) 
	{ 
		// Set a flag indicating this NPC is usable by the player
		int caps = UsableNPCObjectCaps( BaseClass::ObjectCaps() );
		return caps; 
	}
#endif

	void SelectModelGroups( void );

	int GetHeadGroupNum( void ) { return 0; }
	int GetNumHeads( void ) { return SARGE_NUM_HEADS; }
	const char *GetHeadModelName( void ) { return "models/error.mdl"; }

	virtual SpeechManagerID_t GetSpeechManagerID( void ) { return SPEECH_MANAGER_ALLY; }
	const char *GetSentenceGroup( void ) const { return "SARGE_SENTENCE_GROUP"; }
	const char **GetFriendClasses( void ) const {
		static const char *szClasses[] = {
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
		return "cc_sarge";
	}
};

LINK_ENTITY_TO_CLASS( npc_sarge, CNPC_Sarge );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Sarge::Spawn( void )
{
	m_spawnEquipment = NULL_STRING;

	m_iHealth = SARGE_HEALTH;

	BaseClass::Spawn();

	CapabilitiesRemove(
		bits_CAP_USE_WEAPONS |
		bits_CAP_AIM_GUN |
		bits_CAP_SQUAD | bits_CAP_NO_HIT_SQUADMATES |
		bits_CAP_USE_SHOT_REGULATOR );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Sarge::SelectModel()
{
	SetModelName( AllocPooledString( SARGE_MODEL ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Sarge::SelectModelGroups( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: Precaches all resources this monster needs.
//-----------------------------------------------------------------------------
void CNPC_Sarge::Precache( void )
{
	PrecacheScriptSound( "NPC_Barney.FootstepLeft" );
	PrecacheScriptSound( "NPC_Barney.FootstepRight" );
	PrecacheScriptSound( "NPC_Barney.RunFootstepLeft" );
	PrecacheScriptSound( "NPC_Barney.RunFootstepRight" );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Indicates this monster's place in the relationship table.
// Output : 
//-----------------------------------------------------------------------------
Class_T	CNPC_Sarge::Classify( void )
{
	return CLASS_NONE; 
}
