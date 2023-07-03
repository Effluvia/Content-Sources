#include "cbase.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include "c_basehlplayer.h"
#ifdef HOE_THIRDPERSON
#include "input.h"
#endif

#include <vgui/IScheme.h>
#include <vgui_controls/Panel.h>
#include <vgui/ISurface.h>

// memdbgon must be the last include file in a .cpp file!
#include "tier0/memdbgon.h"

extern bool IsWeaponM21Zoomed( CBasePlayer *pPlayer );

/**
 * Simple HUD element for displaying a sniper scope on screen
 */
class CHudScope : public vgui::Panel, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CHudScope, vgui::Panel );

public:
	CHudScope( const char *pElementName );

//	void Init();
//	void MsgFunc_ShowScope( bf_read &msg );

protected:
	virtual void ApplySchemeSettings(vgui::IScheme *scheme);
	virtual bool ShouldDraw( void );
	virtual void Paint( void );

private:
//	bool			m_bShow;
    CHudTexture*	m_pScope;
//	CHudTexture*	m_pSight;
};


DECLARE_HUDELEMENT( CHudScope );
//DECLARE_HUD_MESSAGE( CHudScope, ShowScope );

using namespace vgui;


/**
 * Constructor - generic HUD element initialization stuff. Make sure our 2 member variables
 * are instantiated.
 */
CHudScope::CHudScope( const char *pElementName ) : CHudElement(pElementName), BaseClass(NULL, "HudScope")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

//	m_bShow = false;
	m_pScope = 0;
//	m_pSight = 0;

	// Scope will not show when the player is dead
	SetHiddenBits( HIDEHUD_PLAYERDEAD );
 }

#if 0
/**
 * Hook up our HUD message, and make sure we are not showing the scope
 */
void CHudScope::Init()
{
	HOOK_HUD_MESSAGE( CHudScope, ShowScope );

	m_bShow = false;
}
#endif

/**
 * Load  in the scope material here
 */
void CHudScope::ApplySchemeSettings( vgui::IScheme *scheme )
{
	BaseClass::ApplySchemeSettings(scheme);

	SetPaintBackgroundEnabled(false);
	SetPaintBorderEnabled(false);

	int screenWide, screenTall;
	GetHudSize(screenWide, screenTall);
	SetBounds(0, 0, screenWide, screenTall);

	if (!m_pScope)
	{
		m_pScope = gHUD.GetIcon("scope");
	}
//	if (!m_pSight)
//	{
//		m_pSight = gHUD.GetIcon("sight");
//	}
}

//-----------------------------------------------------------------------------
// Purpose: Save CPU cycles by letting the HUD system early cull
// costly traversal.  Called per frame, return true if thinking and 
// painting need to occur.
//-----------------------------------------------------------------------------
bool CHudScope::ShouldDraw( void )
{
	if ( /*!m_pSight ||*/ !m_pScope )
		return false;

	C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();

	if ( pPlayer == NULL )
		return false;

#ifdef HOE_THIRDPERSON
	if ( !::input->CAM_IsThirdPerson() )
		return false;
#endif // HOE_THIRDPERSON

	if ( !IsWeaponM21Zoomed( pPlayer ) )
		return false;

	return ( CHudElement::ShouldDraw() /*&& !engine->IsDrawingLoadingImage()*/ );
}

/**
 * Simple - if we want to show the scope, draw it. Otherwise don't.
 */
void CHudScope::Paint( void )
{
#if 0
    C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
    if (!pPlayer)
    {
        return;
    }

	if (m_bShow)
	{
#endif
		float scale = GetTall() / 600.0f; // Assumes 512x512 texture
		int w = m_pScope->Width() * scale;
		int h = m_pScope->Height() * scale;
		int x = (GetWide() - w) / 2;
		int y = (GetTall() - h) / 2;

		m_pScope->DrawSelf(x, y, w, h, Color(255,255,255,255));
//		m_pSight->DrawSelf(x, y, w, h, Color(255,255,255,128));

		surface()->DrawSetColor( 0, 0, 0, 255 );
		surface()->DrawFilledRect( 0, 0, x, GetTall() ); // left
		surface()->DrawFilledRect( x + w, 0, GetWide(), GetTall() ); // right
		surface()->DrawFilledRect( x, 0, x + w, y ); // top
		surface()->DrawFilledRect( x, y + h, x + w, GetTall() ); // bottom
#if 0
		int thick = 3 * scale;
		if ( thick & 1 )
			thick += 1;
		int inset = 8 * scale;
		surface()->DrawSetColor( 0, 0, 0, 128 );
		surface()->DrawFilledRect( GetWide() / 2 - thick / 2, y + inset, GetWide() / 2 + thick / 2, y + h - inset ); // vertical
		surface()->DrawFilledRect( x + inset, GetTall() / 2 - thick / 2, x + w - inset, GetTall() / 2 + thick / 2 ); // horizontal
#endif

		// Shouldn't need to check for NULL here since ShouldDraw() does that.
		C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
		if ( !pPlayer )
		{
			DevWarning( "CHudScope::Paint WITH NO PLAYER\n" );
			return;
		}
		C_BaseCombatWeapon *wpn = pPlayer->GetActiveWeapon();
		if ( !wpn )
		{
			DevWarning( "CHudScope::Paint WITH NO WEAPON\n" );
			return;
		}
		bool bCanShoot = wpn->m_flNextPrimaryAttack - 0.1 <= gpGlobals->curtime;
		if ( !bCanShoot )
		{
			int thick = 3 * scale;
			if ( thick & 1 )
				thick += 1;
			surface()->DrawSetColor( 255, 0, 0, 255 );
			surface()->DrawFilledRect( GetWide() / 2 - thick / 2, GetTall() / 2 - thick / 2, GetWide() / 2 + thick / 2, GetTall() / 2 + thick / 2 ); // vertical
		}
#if 0
		// Hide the crosshair
  		pPlayer->m_Local.m_iHideHUD |= HIDEHUD_CROSSHAIR;
	}
	else if ((pPlayer->m_Local.m_iHideHUD & HIDEHUD_CROSSHAIR) != 0)
	{
		pPlayer->m_Local.m_iHideHUD &= ~HIDEHUD_CROSSHAIR;
	}
#endif
}

#if 0
/**
 * Callback for our message - set the show variable to whatever
 * boolean value is received in the message
 */
void CHudScope::MsgFunc_ShowScope(bf_read &msg)
{
	m_bShow = msg.ReadByte();
}
#endif
