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

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//
//-----------------------------------------------------
//

class CHudUseable: public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHudUseable, vgui::Panel );
public:
	CHudUseable( const char *pElementName );
	void Init( void );
	bool ShouldDraw( void );

	virtual void Paint();
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

	void MsgFunc_HudUseableText(bf_read &msg);

	void Reset( void );
	void MessageAdd( const char *pName );
	void UpdateSize( void );

	CPanelAnimationVar( Color, m_TextColor, "TextColor", "FgColor" );
	CPanelAnimationVar( vgui::HFont, m_hTextFont, "TextFont", "Default" );
	CPanelAnimationVarAliasType( float, bottom_ypos, "bottom", "480", "proportional_float" );

private:
	client_textmessage_t		*m_pMessage;
	bool						m_bHasMessage;
	vgui::Label					*m_pLabel;
};

#if 0
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void DispatchHudText( const char *pszText )
{
	if ( pszText == NULL )
	{
		(GET_HUDELEMENT( CHudUseable ))->Reset();
	}
	else
	{
		(GET_HUDELEMENT( CHudUseable ))->MessageAdd( pszText );
	}
}
#endif

//
//-----------------------------------------------------
//

DECLARE_HUDELEMENT( CHudUseable );
DECLARE_HUD_MESSAGE( CHudUseable, HudUseableText );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CHudUseable::CHudUseable( const char *pElementName ) :
	CHudElement( pElementName ), BaseClass( NULL, "HudUseable" )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );
	SetScheme( "ClientScheme" );
	SetCursor( NULL );
	SetVisible( false );

	m_pLabel = vgui::SETUP_PANEL(new vgui::Label( this, "Label", "" ));
	m_pLabel->SetPaintBackgroundEnabled( false );
//	m_pLabel->SetFgColor( m_TextColor );
//	m_pLabel->SetFont( m_hTextFont );
	m_pLabel->SetContentAlignment( vgui::Label::a_center );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudUseable::Init(void)
{
	HOOK_HUD_MESSAGE( CHudUseable, HudUseableText );

	Reset();
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudUseable::Reset( void )
{
	SetVisible( false );
 	m_pMessage = NULL;
	m_bHasMessage = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CHudUseable::ShouldDraw( void )
{
	return ( CHudElement::ShouldDraw() && m_bHasMessage );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudUseable::Paint()
{
	BaseClass::Paint();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudUseable::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_pLabel->SetFgColor( m_TextColor );
	m_pLabel->SetFont( m_hTextFont );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudUseable::MessageAdd( const char *pName )
{
	if ( pName == NULL || pName[0] == '\0' )
	{
		m_pMessage = NULL;
		m_bHasMessage = false;
		SetVisible( false );
		return;
	}

	if ( pName[0] == '#' )
	{
		m_pMessage = TextMessageGet( pName+1 );
	}
	else
	{
		m_pMessage = TextMessageGet( pName );
	}

	if ( m_pMessage )
	{
		DevMsg(1, "CHudUseable::MessageAdd added '%s'\n", pName);
		m_pLabel->SetText( m_pMessage->pMessage );
		UpdateSize();
		SetVisible( true );
		m_bHasMessage = true;
	}
	else
	{
		DevWarning("CHudUseable::MessageAdd NULL returned by TextMessageGet('%s')\n", pName);
		SetVisible( false );
		m_bHasMessage = false;
	}
}

//-----------------------------------------------------------------------------
void CHudUseable::UpdateSize( void )
{
	int w, h;
	w = ScreenWidth();
	h = ScreenHeight();

	int wide, tall;
	m_pLabel->GetContentSize( wide, tall );
	wide += 16, tall += 16;

	m_pLabel->SetSize( wide, tall );

	SetSize( wide, tall );
	SetPos( ( w - wide ) / 2, bottom_ypos - tall );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudUseable::MsgFunc_HudUseableText( bf_read &msg )
{
	char szString[2048];
	msg.ReadString( szString, sizeof(szString) );

	MessageAdd( szString );
}