#include "cbase.h"
#include "basehlcombatweapon.h"
#include "player.h"
#include "gamerules.h"
#include "npcevent.h"
#include "engine/IEngineSound.h"
#include "items.h"
#include "in_buttons.h"
#include "soundent.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CWeaponLetter: public CBaseHLCombatWeapon
{
	DECLARE_CLASS( CWeaponLetter, CBaseHLCombatWeapon );
public:
	DECLARE_SERVERCLASS();

	void	Precache( void );
	void	Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
	void	PrimaryAttack( void );
	void	SecondaryAttack( void );

	bool	Deploy( void );
	bool	Holster( CBaseCombatWeapon *pSwitchingTo = NULL );

	void GetControlPanelInfo( int nPanelIndex, const char *&pPanelName );
	void GetControlPanelClassName( int nPanelIndex, const char *&pPanelName );

private:

//	DECLARE_ACTTABLE();
	DECLARE_DATADESC();
};


BEGIN_DATADESC( CWeaponLetter )
END_DATADESC()

//acttable_t CWeaponLetter::m_acttable[] = 
//{
//};

//IMPLEMENT_ACTTABLE(CWeaponLetter);

IMPLEMENT_SERVERCLASS_ST(CWeaponLetter, DT_WeaponLetter)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_letter, CWeaponLetter );
PRECACHE_WEAPON_REGISTER(weapon_letter);

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponLetter::Precache( void )
{
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponLetter::Deploy( void )
{
	return BaseClass::Deploy();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponLetter::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	return BaseClass::Holster( pSwitchingTo );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEvent - 
//			*pOperator - 
//-----------------------------------------------------------------------------
void CWeaponLetter::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	switch( pEvent->event )
	{
	case 1:
	default:
		BaseClass::Operator_HandleAnimEvent( pEvent, pOperator );
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponLetter::PrimaryAttack( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponLetter::SecondaryAttack( void )
{
}

//-----------------------------------------------------------------------------
void CWeaponLetter::GetControlPanelInfo( int nPanelIndex, const char *&pPanelName )
{
	pPanelName = "letter_screen"; // see scripts/screens/letter_screen.res, scripts/vgui_screens.txt
}

//-----------------------------------------------------------------------------
void CWeaponLetter::GetControlPanelClassName( int nPanelIndex, const char *&pPanelName )
{
	pPanelName = "vgui_screen";
}
