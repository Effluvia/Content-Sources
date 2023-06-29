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
#include "c_weapon_870.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//
//-----------------------------------------------------
//

class CHudAmmo870 : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHudAmmo870, vgui::Panel );
public:
	CHudAmmo870( const char *pElementName );
	void Init( void );
	void Reset();
	void SetLabelText(const wchar_t *text);

	virtual void Paint();
	void PaintLabel( void );
	void PaintNumbers(vgui::HFont font, int xpos, int ypos, int value);

	virtual void OnThink();
	void UpdateAmmoDisplay();
	void SetAmmo(int ammo, bool playAnimation);
	void SetAmmo2(int ammo2, bool playAnimation);

	CPanelAnimationVar( vgui::HFont, m_hTextFont, "TextFont", "Default" );
	CPanelAnimationVar( vgui::HFont, m_hSmallNumberFont, "SmallNumberFont", "HudNumbersSmall" );

	CPanelAnimationVar( float, m_flBlur, "Blur", "0" );
	CPanelAnimationVar( Color, m_AmmoColor, "AmmoColor", "FgColor" );
	CPanelAnimationVar( Color, m_Ammo2Color, "Ammo2Color", "FgColor" );

	CPanelAnimationVarAliasType( float, text_xpos, "text_xpos", "8", "proportional_float" );
	CPanelAnimationVarAliasType( float, text_ypos, "text_ypos", "20", "proportional_float" );

	CPanelAnimationVarAliasType( float, digit_xpos, "digit_xpos", "50", "proportional_float" );
	CPanelAnimationVarAliasType( float, digit_ypos, "digit_ypos", "2", "proportional_float" );
	CPanelAnimationVarAliasType( float, digit2_xpos, "digit2_xpos", "98", "proportional_float" );
	CPanelAnimationVarAliasType( float, digit2_ypos, "digit2_ypos", "16", "proportional_float" );

	CPanelAnimationVarAliasType( float, ammo_xpos, "ammo_xpos", "16", "proportional_float" );
	CPanelAnimationVarAliasType( float, ammo_ypos, "ammo_ypos", "8", "proportional_float" );
	CPanelAnimationVarAliasType( float, ammo_spacing, "ammo_spacing", "16", "proportional_float" );

	CHandle< C_Weapon870 > m_hCurrentActiveWeapon;
	CHudTexture *m_iconPrimaryAmmo;
	CHudTexture *m_iconSecondaryAmmo;
	int m_iAmmo;
	int m_iAmmo2;
	wchar_t m_LabelText[32];
};

DECLARE_HUDELEMENT( CHudAmmo870 );

//-----------------------------------------------------------------------------
CHudAmmo870::CHudAmmo870( const char *pElementName ) :
	CHudElement( pElementName ), BaseClass(NULL, "HudAmmo870")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	SetHiddenBits( HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT | HIDEHUD_WEAPONSELECTION );
}

//-----------------------------------------------------------------------------
void CHudAmmo870::Init( void )
{
	m_iconPrimaryAmmo = NULL;
	m_iconSecondaryAmmo = NULL;

	m_iAmmo		= -1;
	m_iAmmo2	= -1;

	wchar_t *tempString = g_pVGuiLocalize->Find("#Valve_Hud_AMMO");
	if (tempString)
	{
		SetLabelText(tempString);
	}
	else
	{
		SetLabelText(L"AMMO");
	}
}

//-----------------------------------------------------------------------------
void CHudAmmo870::Reset()
{
	CHudElement::Reset();

	m_hCurrentActiveWeapon = NULL;
	m_iAmmo = 0;
	m_iAmmo2 = 0;

	UpdateAmmoDisplay();
}

//-----------------------------------------------------------------------------
void CHudAmmo870::OnThink()
{
	UpdateAmmoDisplay();
}

//-----------------------------------------------------------------------------
void CHudAmmo870::SetLabelText(const wchar_t *text)
{
	wcsncpy(m_LabelText, text, sizeof(m_LabelText) / sizeof(wchar_t));
	m_LabelText[(sizeof(m_LabelText) / sizeof(wchar_t)) - 1] = 0;
}

//-----------------------------------------------------------------------------
void CHudAmmo870::UpdateAmmoDisplay()
{
	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
	C_Weapon870 *p870 = NULL;

	if ( player )
	{
		if ( !player->GetVehicle() || player->UsingStandardWeaponsInVehicle() )
		{
			C_BaseCombatWeapon *wpn = player->GetActiveWeapon();
			if ( wpn )
				p870 = dynamic_cast<C_Weapon870 *>(wpn);
		}
	}

	if ( !p870 )
	{
		m_hCurrentActiveWeapon = NULL;
		SetPaintEnabled(false);
		SetPaintBackgroundEnabled(false);
		return;
	}

	SetPaintEnabled(true);
	SetPaintBackgroundEnabled(true);

	if ( m_hCurrentActiveWeapon == p870 )
	{
		SetAmmo( player->GetAmmoCount( p870->GetPrimaryAmmoType() ), true );
		SetAmmo2( player->GetAmmoCount( p870->GetSecondaryAmmoType() ), true );
	}
	else
	{
		SetAmmo( player->GetAmmoCount( p870->GetPrimaryAmmoType() ), false );
		SetAmmo2( player->GetAmmoCount( p870->GetSecondaryAmmoType() ), false );
		m_hCurrentActiveWeapon = p870;

		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("870WeaponChanged");
	}

#if 1
	m_iconPrimaryAmmo = gHUD.GetIcon( "ammo870" );
	m_iconSecondaryAmmo = gHUD.GetIcon( "ammo870_2" );
#else
	m_iconPrimaryAmmo = gWR.GetAmmoIconFromWeapon( p870->GetPrimaryAmmoType() );
	m_iconSecondaryAmmo = gWR.GetAmmoIconFromWeapon( p870->GetSecondaryAmmoType() );
#endif
}

//-----------------------------------------------------------------------------
void CHudAmmo870::SetAmmo( int ammo, bool playAnimation )
{
	if (ammo != m_iAmmo)
	{
		if (ammo == 0)
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("870AmmoEmpty");
		}
		else if (ammo < m_iAmmo)
		{
			// ammo has decreased
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("870AmmoDecreased");
		}
		else
		{
			// ammunition has increased
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("870AmmoIncreased");
		}

		m_iAmmo = ammo;
	}
}

//-----------------------------------------------------------------------------
void CHudAmmo870::SetAmmo2( int ammo, bool playAnimation )
{
	if (ammo != m_iAmmo2)
	{
		if (ammo == 0)
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("870AmmoSecondaryEmpty");
		}
		else if (ammo < m_iAmmo2)
		{
			// ammo has decreased
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("870AmmoSecondaryDecreased");
		}
		else
		{
			// ammunition has increased
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("870AmmoSecondaryIncreased");
		}

		m_iAmmo2 = ammo;
	}
}

//-----------------------------------------------------------------------------
void CHudAmmo870::Paint( void )
{
	BaseClass::Paint();

	if ( m_hCurrentActiveWeapon && m_iconPrimaryAmmo && m_iconSecondaryAmmo )
	{
		int wide, tall;
		GetSize( wide, tall );

		int count = 0;
		for ( int i = 0; i < MAX_CLIP; i++ )
		{
			if ( m_hCurrentActiveWeapon->m_AmmoQueue[i] != 0 )
				count++;
		}
		if ( count )
		{
			int iconWidth = ammo_spacing;
			int x = ammo_xpos;
			int y = ammo_ypos;

			// Draw in reverse order, same as order shots are fired
			for ( int i = MAX_CLIP - 1; i >= 0; i-- )
			{
				if ( m_hCurrentActiveWeapon->m_AmmoQueue[i] == 1 )
					m_iconPrimaryAmmo->DrawSelf( x, y, GetFgColor() );
				else if ( m_hCurrentActiveWeapon->m_AmmoQueue[i] == 2 )
					m_iconSecondaryAmmo->DrawSelf( x, y, GetFgColor() );
				else 
					continue;
				x += iconWidth;
			}
		}

		PaintLabel();

		surface()->DrawSetTextColor( m_AmmoColor );
		PaintNumbers( m_hSmallNumberFont, digit_xpos, digit_ypos, m_iAmmo );

		surface()->DrawSetTextColor( m_Ammo2Color );
		PaintNumbers( m_hSmallNumberFont, digit2_xpos, digit2_ypos, m_iAmmo2 );

		int x, y;
		x = digit_xpos - m_iconPrimaryAmmo->Width() / 2, y = ammo_ypos;
		m_iconPrimaryAmmo->DrawSelf( x, y, m_AmmoColor );

		x = digit2_xpos - m_iconSecondaryAmmo->Width() / 2, y = ammo_ypos;
		m_iconSecondaryAmmo->DrawSelf( x, y, m_Ammo2Color );
	}
}

//-----------------------------------------------------------------------------
void CHudAmmo870::PaintLabel( void )
{
	surface()->DrawSetTextFont(m_hTextFont);
	surface()->DrawSetTextColor(GetFgColor());
	surface()->DrawSetTextPos(text_xpos, text_ypos);
	surface()->DrawUnicodeString( m_LabelText );
}

//-----------------------------------------------------------------------------
void CHudAmmo870::PaintNumbers(HFont font, int xpos, int ypos, int value)
{
	surface()->DrawSetTextFont(font);
	wchar_t unicode[6];
	swprintf(unicode, L"%d", value);

	int wide, tall;
	surface()->GetTextSize( font, unicode, wide, tall );
	xpos -= wide / 2;

	surface()->DrawSetTextPos(xpos, ypos);
	surface()->DrawUnicodeString( unicode );
}