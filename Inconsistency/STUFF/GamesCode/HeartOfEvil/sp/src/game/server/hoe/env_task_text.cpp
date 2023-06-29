#include "cbase.h"
#include "baseentity.h"
#include "entityoutput.h"
#include "recipientfilter.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CEnvTaskText : public CPointEntity
{
public:
	DECLARE_CLASS( CEnvTaskText, CPointEntity );

	void	Spawn( void );
	void	Precache( void );

private:
	void InputShowTaskText( inputdata_t &inputdata );
	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS( env_tasktext, CEnvTaskText );

BEGIN_DATADESC( CEnvTaskText )

	DEFINE_INPUTFUNC( FIELD_STRING, "ShowTaskText", InputShowTaskText ),

END_DATADESC()



//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEnvTaskText::Spawn( void )
{
	Precache();

	SetSolid( SOLID_NONE );
	SetMoveType( MOVETYPE_NONE );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEnvTaskText::Precache( void )
{
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CEnvTaskText::InputShowTaskText( inputdata_t &inputdata )
{
	CBaseEntity *pPlayer = NULL;

	if ( inputdata.pActivator && inputdata.pActivator->IsPlayer() )
	{
		pPlayer = inputdata.pActivator;
	}
	else
	{
		pPlayer = UTIL_GetLocalPlayer();
	}

	if ( pPlayer )
	{
		if ( !pPlayer || !pPlayer->IsNetClient() )
			return;

		CSingleUserRecipientFilter user( (CBasePlayer *)pPlayer );
		user.MakeReliable();
		UserMessageBegin( user, "HudTaskText" );
			WRITE_STRING( STRING(inputdata.value.StringID()) );
		MessageEnd();
	}
}

