#include "cbase.h"
#include "ai_basenpc.h"
#include "hoe_corpse.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define SCIENTIST_MODEL "models/scientist.mdl"
#define SCIENTIST_NUM_HEADS 4

class CDeadScientist : public CHOECorpse
{
public:
	DECLARE_CLASS( CDeadScientist, CHOECorpse );

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
			{ NULL, false }
		};

		SetBloodColor( BLOOD_COLOR_RED );
		InitCorpse( SCIENTIST_MODEL, poses, AllocPooledString( "npc_scientist" ) );

		BaseClass::Spawn();
	}

	void SelectModelGroups( void )
	{
		int headNum;

		if ( m_nBody == -1 )
		{
			headNum = random->RandomInt( 0, SCIENTIST_NUM_HEADS - 1 ); 
		}
		else 
		{
			headNum = m_nBody;
		}

		// Luther is black, make his hands black
		if ( headNum == 2 )
			m_nSkin = 1;
		else
			m_nSkin = 0;

		m_nBody = 0;
	}
};

LINK_ENTITY_TO_CLASS( npc_scientist_dead, CDeadScientist );
