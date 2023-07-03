#include "cbase.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "itextmessage.h"
#include "iclientmode.h"
#include "vgui_basepanel.h"
#include "vgui_controls/Controls.h"
#include "vgui_controls/Panel.h"
#include "vgui/ILocalize.h"
#include "vgui/IScheme.h"
#include "vgui/ISurface.h"
#include "client_textmessage.h"
#include <ctype.h>
#include <vgui_controls/AnimationController.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//
//-----------------------------------------------------
//

class CHudActiveWeapon: public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHudActiveWeapon, vgui::Panel );
public:
	CHudActiveWeapon( const char *pElementName );
	void Init( void );
	void Reset();

	virtual void Paint();

	virtual void OnThink();
	void UpdateAmmoDisplay();

	CHandle< C_BaseCombatWeapon > m_hCurrentActiveWeapon;
	CHudTexture *m_icon;
};

DECLARE_HUDELEMENT( CHudActiveWeapon );

//-----------------------------------------------------------------------------
CHudActiveWeapon::CHudActiveWeapon( const char *pElementName ) :
	CHudElement( pElementName ), BaseClass(NULL, "HudActiveWeapon")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	SetHiddenBits( HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT | HIDEHUD_WEAPONSELECTION );
}

//-----------------------------------------------------------------------------
void CHudActiveWeapon::Init( void )
{
	m_icon = NULL;
}

//-----------------------------------------------------------------------------
void CHudActiveWeapon::Reset()
{
	CHudElement::Reset();

	m_hCurrentActiveWeapon = NULL;
}

//-----------------------------------------------------------------------------
void CHudActiveWeapon::OnThink()
{
	UpdateAmmoDisplay();
}

//-----------------------------------------------------------------------------
void CHudActiveWeapon::UpdateAmmoDisplay()
{
	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
	C_BaseCombatWeapon *wpn = NULL;

	if ( player && player->ShouldDrawLocalPlayer() )
	{
		if ( !player->GetVehicle() || player->UsingStandardWeaponsInVehicle() )
		{
			wpn = player->GetActiveWeapon();
		}
	}

	if ( !wpn /*|| wpn->Clip1() < 0*/ )
	{
		m_hCurrentActiveWeapon = NULL;
		SetPaintEnabled(false);
		SetPaintBackgroundEnabled(false);
		return;
	}

	SetPaintEnabled(true);
	SetPaintBackgroundEnabled(true);

	if ( m_hCurrentActiveWeapon == wpn )
	{
	}
	else
	{
		m_hCurrentActiveWeapon = wpn;

//		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("870WeaponChanged");
	}

	const FileWeaponInfo_t &wpnInfo = wpn->GetWpnData();
	m_icon = wpnInfo.iconInactive;
}

//-----------------------------------------------------------------------------
void CHudActiveWeapon::Paint( void )
{
	BaseClass::Paint();

	if ( m_hCurrentActiveWeapon && m_icon )
	{
		int wide, tall;
		GetSize( wide, tall );

		int x, y;
//		GetPos( x, y );
		x = (wide - m_icon->Width()) / 2;
		y = (tall - m_icon->Height()) / 2;
		m_icon->DrawSelf( x, y, GetFgColor() );
	}
}
