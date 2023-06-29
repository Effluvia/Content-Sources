#include "cbase.h"
#include "ai_basenpc.h"
#include "hoe_corpse.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define THEM_MODEL "models/them.mdl"
#define THEM_MODEL_BYFEET "models/them/them_byfeet.mdl"

class CDeadThem : public CHOECorpse
{
public:
	DECLARE_CLASS( CDeadThem, CHOECorpse );

	void Spawn( void )
	{
		CorpsePose poses[] = {
			{ "lying_on_back", false },
			{ "lying_on_side", false },
			{ "lying_on_stomach", false },
			{ "hanging_byfeet",	true, "rope_feet", NULL },
			{ "hanging_byarms", true, "rope_lefthand", "rope_righthand" },
			{ "hanging_byneck", true, "rope_neck", NULL },
			{ "deadsitting", false },
			{ NULL, false }
		};

		SetCCImageName( AllocPooledString( "cc_them" ) );
		SetBloodColor( BLOOD_COLOR_YELLOW );
		InitCorpse( (m_pose == 3) ? THEM_MODEL_BYFEET : THEM_MODEL, poses );

		BaseClass::Spawn();
	}

	void SelectModelGroups( void )
	{
//		m_nBody = 0;
	}

	IResponseSystem *GetResponseSystem()
	{
		extern IResponseSystem *g_pResponseSystem;
		return g_pResponseSystem;
	}
};

LINK_ENTITY_TO_CLASS( npc_them_dead, CDeadThem );
