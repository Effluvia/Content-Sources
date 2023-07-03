#include "cbase.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "itextmessage.h"
#include "iclientmode.h"
#include "vgui_controls/AnimationController.h"
#include "vgui_basepanel.h"
#include "vgui_controls/Controls.h"
#include "vgui_controls/Panel.h"
#include "vgui_controls/TextImage.h"
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

class CHudTask: public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHudTask, vgui::Panel );
public:
	CHudTask( const char *pElementName );
	void Init( void );
	bool ShouldDraw( void );

	virtual void Paint();
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

	void MsgFunc_HudTaskText(bf_read &msg);

	void Reset( void );
	void MessageAdd( const char *pName );
	void SetTaskText( const char *text );

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
		(GET_HUDELEMENT( CHudTask ))->Reset();
	}
	else
	{
		(GET_HUDELEMENT( CHudTask ))->MessageAdd( pszText );
	}
}
#endif

//
//-----------------------------------------------------
//

DECLARE_HUDELEMENT( CHudTask );
DECLARE_HUD_MESSAGE( CHudTask, HudTaskText );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CHudTask::CHudTask( const char *pElementName ) :
	CHudElement( pElementName ), BaseClass( NULL, "HudTask" )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );
	SetScheme( "ClientScheme" );
	SetCursor( NULL );
	SetVisible( false );
	SetAlpha( 0 );

	m_pLabel = vgui::SETUP_PANEL(new vgui::Label( this, "Label", "" ));
	m_pLabel->SetPaintBackgroundEnabled( false );
//	m_pLabel->SetFgColor( m_TextColor );
//	m_pLabel->SetFont( m_hTextFont );
	m_pLabel->SetContentAlignment( vgui::Label::a_center );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudTask::Init(void)
{
	HOOK_HUD_MESSAGE( CHudTask, HudTaskText );

	Reset();
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudTask::Reset( void )
{
	SetVisible( false );
	SetAlpha( 0 );
 	m_pMessage = NULL;
	m_bHasMessage = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CHudTask::ShouldDraw( void )
{
	return ( ( GetAlpha() > 0 ) && CHudElement::ShouldDraw() && m_bHasMessage );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudTask::Paint()
{
	BaseClass::Paint();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudTask::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_pLabel->SetFgColor( m_TextColor );
	m_pLabel->SetFont( m_hTextFont );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudTask::MessageAdd( const char *pName )
{
	if ( pName == NULL || pName[0] == '\0' )
	{
		m_pMessage = NULL;
		m_bHasMessage = false;
//		SetVisible( false );

		// See scripts/HudAnimations.txt
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "HudTaskHide" ); 
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
		DevMsg(1, "CHudTask::MessageAdd added '%s'\n", pName);
		SetTaskText( m_pMessage->pMessage );
		SetVisible( true );
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "HudTaskShow" ); 
		m_bHasMessage = true;
	}
	else
	{
		DevWarning("CHudTask::MessageAdd NULL returned by TextMessageGet('%s')\n", pName);
//		SetVisible( false );
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "HudTaskHide" ); 
		m_bHasMessage = false;
	}
}

//-----------------------------------------------------------------------------
void CHudTask::SetTaskText( const char *text )
{
	int w, h;
	w = ScreenWidth();
	h = ScreenHeight();

	m_pLabel->SetText( text );
//	m_pLabel->InvalidateLayout( true ); // needed or GetContentSize returns bogus values

	int wide, tall;
	m_pLabel->GetTextImage()->GetContentSize( wide, tall );
	wide += 16, tall += 16;
	m_pLabel->SetSize( wide, tall );

	SetPos( ( w - wide ) / 2, max(bottom_ypos - tall, 0) );
	SetSize( wide, tall );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudTask::MsgFunc_HudTaskText( bf_read &msg )
{
	char szString[2048];
	msg.ReadString( szString, sizeof(szString) );

	MessageAdd( szString );
}