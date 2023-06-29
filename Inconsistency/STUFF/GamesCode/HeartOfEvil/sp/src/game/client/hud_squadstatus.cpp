//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include "c_basehlplayer.h"
#include "vgui_controls/Panel.h"
#include "vgui_controls/AnimationController.h"
#include "vgui/ISurface.h"
#include <vgui/ILocalize.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar player_squad_transient_commands( "player_squad_transient_commands", "1", FCVAR_REPLICATED );

//-----------------------------------------------------------------------------
// Purpose: Shows the sprint power bar
//-----------------------------------------------------------------------------
class CHudSquadStatus : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHudSquadStatus, vgui::Panel );

public:
	CHudSquadStatus( const char *pElementName );
	virtual void Init( void );
	virtual void Reset( void );
	virtual void OnThink( void );
	bool ShouldDraw();

	void MsgFunc_SquadMemberDied(bf_read &msg);

protected:
	virtual void Paint();

private:
	CPanelAnimationVar( vgui::HFont, m_hTextFont, "TextFont", "Default" );
	CPanelAnimationVarAliasType( float, text_xpos, "text_xpos", "8", "proportional_float" );
	CPanelAnimationVarAliasType( float, text_ypos, "text_ypos", "20", "proportional_float" );

	CPanelAnimationVar( vgui::HFont, m_hIconFont, "IconFont", "HudNumbers" );
	CPanelAnimationVarAliasType( float, m_flIconInsetX, "IconInsetX", "8", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flIconInsetY, "IconInsetY", "8", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flIconGap, "IconGap", "20", "proportional_float" );
#ifdef HOE_DLL
	CPanelAnimationVarAliasType( float, m_flIconGapY, "IconGapY", "34", "proportional_float" );
#endif

	CPanelAnimationVar( Color, m_SquadIconColor, "SquadIconColor", "255 220 0 160" );
	CPanelAnimationVar( Color, m_LastMemberColor, "LastMemberColor", "255 220 0 0" );
	CPanelAnimationVar( Color, m_SquadTextColor, "SquadTextColor", "255 220 0 160" );
	
	int m_iSquadMembers;
	int m_iSquadMedics;
	bool m_bSquadMembersFollowing;
	bool m_bSquadMemberAdded;
	bool m_bSquadMemberJustDied;
};	


DECLARE_HUDELEMENT( CHudSquadStatus );
DECLARE_HUD_MESSAGE( CHudSquadStatus, SquadMemberDied );

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudSquadStatus::CHudSquadStatus( const char *pElementName ) : CHudElement( pElementName ), BaseClass( NULL, "HudSquadStatus" )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	SetHiddenBits( HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudSquadStatus::Init( void )
{
	HOOK_HUD_MESSAGE( CHudSquadStatus, SquadMemberDied );
	m_iSquadMembers = 0;
	m_iSquadMedics = 0;
	m_bSquadMemberAdded = false;
	m_bSquadMembersFollowing = true;
	m_bSquadMemberJustDied = false;
	SetAlpha( 0 );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudSquadStatus::Reset( void )
{
	Init();
}

//-----------------------------------------------------------------------------
// Purpose: Save CPU cycles by letting the HUD system early cull
// costly traversal.  Called per frame, return true if thinking and 
// painting need to occur.
//-----------------------------------------------------------------------------
bool CHudSquadStatus::ShouldDraw( void )
{
	bool bNeedsDraw = false;

	C_BaseHLPlayer *pPlayer = (C_BaseHLPlayer *)C_BasePlayer::GetLocalPlayer();
	if ( !pPlayer )
		return false;

#ifdef HOE_DLL
	bNeedsDraw = ( pPlayer->m_HL2Local.m_SquadMembers.Count() > 0 );
#else // HOE_DLL
	bNeedsDraw = ( pPlayer->m_HL2Local.m_iSquadMemberCount > 0 ||
					( pPlayer->m_HL2Local.m_iSquadMemberCount != m_iSquadMembers ) || 
					( pPlayer->m_HL2Local.m_fSquadInFollowMode != m_bSquadMembersFollowing ) ||
					( m_iSquadMembers > 0 ) ||
					( m_LastMemberColor[3] > 0 ) );
#endif // HOE_DLL
		
	return ( bNeedsDraw && CHudElement::ShouldDraw() );
}

//-----------------------------------------------------------------------------
// Purpose: updates hud icons
//-----------------------------------------------------------------------------
void CHudSquadStatus::OnThink( void )
{
	C_BaseHLPlayer *pPlayer = (C_BaseHLPlayer *)C_BasePlayer::GetLocalPlayer();
	if ( !pPlayer )
		return;

#ifdef HOE_DLL
	int squadMembers = pPlayer->m_HL2Local.m_SquadMembers.Count();
	bool following = squadMembers ? (pPlayer->m_HL2Local.m_SquadMembers[0].flags & C_PlayerSquadInfo::PSI_FOLLOW) != 0 : m_bSquadMembersFollowing;
#else // HOE_DLL
	int squadMembers = pPlayer->m_HL2Local.m_iSquadMemberCount;
	bool following = pPlayer->m_HL2Local.m_fSquadInFollowMode;
	m_iSquadMedics = pPlayer->m_HL2Local.m_iSquadMedicCount;
#endif // HOE_DLL

	// Only update if we've changed vars
	if ( squadMembers == m_iSquadMembers && following == m_bSquadMembersFollowing )
		return;

	// update status display
	if ( squadMembers > 0)
	{
		// we have squad members, show the display
		g_pClientMode->GetViewportAnimationController()->RunAnimationCommand( this, "alpha", 255.0f, 0.0f, 0.4f, AnimationController::INTERPOLATOR_LINEAR);
	}
	else
	{
		// no squad members, hide the display
		g_pClientMode->GetViewportAnimationController()->RunAnimationCommand( this, "alpha", 0.0f, 0.0f, 0.4f, AnimationController::INTERPOLATOR_LINEAR);
	}

	if ( squadMembers > m_iSquadMembers )
	{
		// someone is added
		// reset the last icon color and animate
		m_LastMemberColor = m_SquadIconColor;
		m_LastMemberColor[3] = 0;
		m_bSquadMemberAdded = true;
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "SquadMemberAdded" ); 
	}
	else if ( squadMembers < m_iSquadMembers )
	{
		// someone has left
		// reset the last icon color and animate
		m_LastMemberColor = m_SquadIconColor;
		m_bSquadMemberAdded = false;
		if (m_bSquadMemberJustDied)
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "SquadMemberDied" ); 
			m_bSquadMemberJustDied = false;
		}
		else
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "SquadMemberLeft" ); 
		}
	}

#ifdef HOE_DLL
	if ( ( squadMembers != m_iSquadMembers ) && ( squadMembers > 4 || m_iSquadMembers > 4 ) )
	{
#if 1
		char sequence[32];
		Q_snprintf( sequence, sizeof(sequence), "SquadMembers%d", max(squadMembers,4) );
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( sequence );
#else
		int dx = 0;
		if ( m_iSquadMembers > 4 )
			dx -= m_flIconGap * (m_iSquadMembers - 4);
		if ( squadMembers > 4 )
			dx += m_flIconGap * (squadMembers - 4);

		int x, y;
		GetPos( x, y );
		SetPos( x - dx, y );

		int wide = GetWide();
		SetWide( wide + dx );
#endif
	}
#endif

	if ( following != m_bSquadMembersFollowing )
	{
		if ( following )
		{
			// flash the squad area to indicate they are following
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "SquadMembersFollowing" );
		}
		else
		{
			// flash the crosshair to indicate the targeted order is in effect
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "SquadMembersStationed" );
		}
	}

	m_iSquadMembers = squadMembers;
	m_bSquadMembersFollowing = following;
}

//-----------------------------------------------------------------------------
// Purpose: Notification of squad member being killed
//-----------------------------------------------------------------------------
void CHudSquadStatus::MsgFunc_SquadMemberDied(bf_read &msg)
{
	m_bSquadMemberJustDied = true;
}

//-----------------------------------------------------------------------------
// Purpose: draws the power bar
//-----------------------------------------------------------------------------
#ifdef HOE_DLL // new CPlayerSquadInfo version
void CHudSquadStatus::Paint()
{
	C_BaseHLPlayer *pPlayer = (C_BaseHLPlayer *)C_BasePlayer::GetLocalPlayer();
	if ( !pPlayer )
		return;

	CUtlVector<C_PlayerSquadInfo>& members = pPlayer->m_HL2Local.m_SquadMembers;

	surface()->DrawSetTextFont( m_hIconFont );
	int xpos = m_flIconInsetX, ypos = m_flIconInsetY;
	for (int i = 0; i < members.Count(); i++)
	{
		Color clr = m_SquadIconColor;

		if ( members[i].flags & C_PlayerSquadInfo::PSI_JOIN )
		{
			float flFrac = members[i].frac / 100.0f;
			clr[3] *= flFrac;
		}
		else if ( members[i].flags & C_PlayerSquadInfo::PSI_LEAVE )
		{
			float flFrac = members[i].frac / 100.0f;
			clr[3] *= 1.0 - flFrac;
		}
		else if ( members[i].flags & C_PlayerSquadInfo::PSI_DIED )
		{
			float flFrac = members[i].frac / 100.0f;
			clr.SetColor( 255, 0, 0, 255 );
			clr[3] *= 1.0 - flFrac;
		}

		if ( members[i].flags & C_PlayerSquadInfo::PSI_USEABLE )
		{
			clr[0] = 0;
			clr[1] = 255;
			clr[2] = 0;
		}

		if ( members[i].health > 0 )
		{
			// Draw health
			int w = surface()->GetCharacterWidth( m_hIconFont, 'M' );
			int x = xpos + w + 3; // left edge of bar
			int ascent = surface()->GetFontAscent( m_hIconFont, 'M' );
			int y1 = ypos + ascent * 0.33f; // top edge of bar
			int h = ypos + ascent - y1; // height of total bar
			int h2 = clamp( h * (members[i].health / 100.0f), 1, h ); // height of health remaining

			surface()->DrawSetColor( Color( 128, 128, 128, clr[3] ) );
			surface()->DrawFilledRect( x, y1, x + 3, y1 + h - h2 );
			surface()->DrawSetColor( clr );
			surface()->DrawFilledRect( x, y1 + h - h2, x + 3, ypos + ascent );
		}

		surface()->DrawSetTextColor( clr );
		surface()->DrawSetTextPos(xpos, ypos);

		if ( members[i].flags & C_PlayerSquadInfo::PSI_MEDIC )
		{
			surface()->DrawUnicodeChar( 'M' );
		}
		else
		{
			surface()->DrawUnicodeChar( 'C' );
		}
		xpos += m_flIconGap;
#ifdef HOE_DLL
		if ( i == 7 )
		{
			xpos = m_flIconInsetX;
			ypos += m_flIconGapY;
		}
#endif
	}

	// draw our squad status
	wchar_t *text = NULL;
	if (m_bSquadMembersFollowing)
	{
		text = g_pVGuiLocalize->Find("#Valve_Hud_SQUAD_FOLLOWING");

		if (!text)
		{
			text = L"SQUAD FOLLOWING";
		}
	}
	else
	{
#ifdef HOE_DLL
#else
		if ( !player_squad_transient_commands.GetBool() )
#endif
		{
			text = g_pVGuiLocalize->Find("#Valve_Hud_SQUAD_STATIONED");

			if (!text)
			{
				text = L"SQUAD STATIONED";
			}
		}
	}

	if (text)
	{
		surface()->DrawSetTextFont(m_hTextFont);
		surface()->DrawSetTextColor(m_SquadTextColor);
#ifdef HOE_DLL
		int wide, tall;
		surface()->GetTextSize( m_hTextFont, text, wide, tall );
		surface()->DrawSetTextPos( ( GetWide() - wide ) / 2, GetTall() - text_ypos );
#else
		surface()->DrawSetTextPos(text_xpos, text_ypos);
#endif
		surface()->DrawPrintText(text, wcslen(text));
	}
}
#else // HOE_DLL
void CHudSquadStatus::Paint()
{
	C_BaseHLPlayer *pPlayer = (C_BaseHLPlayer *)C_BasePlayer::GetLocalPlayer();
	if ( !pPlayer )
		return;

	// draw the suit power bar
	surface()->DrawSetTextColor( m_SquadIconColor );
	surface()->DrawSetTextFont( m_hIconFont );
	int xpos = m_flIconInsetX, ypos = m_flIconInsetY;
	for (int i = 0; i < m_iSquadMembers; i++)
	{
		if (m_bSquadMemberAdded && i == m_iSquadMembers - 1)
		{
			// draw the last added squad member specially
			surface()->DrawSetTextColor( m_LastMemberColor );
		}

		surface()->DrawSetTextPos(xpos, ypos);

		if (i < m_iSquadMedics)
		{
			surface()->DrawUnicodeChar('M');
		}
		else
		{
			surface()->DrawUnicodeChar('C');
		}
		xpos += m_flIconGap;
#ifdef HOE_DLL
		if ( i == 7 )
		{
			xpos = m_flIconInsetX;
			ypos += m_flIconGapY;
		}
#endif
	}
	if (!m_bSquadMemberAdded && m_LastMemberColor[3])
	{
		// draw the last one in the special color
		surface()->DrawSetTextColor( m_LastMemberColor );
		surface()->DrawSetTextPos(xpos, ypos);
		surface()->DrawUnicodeChar('C');
	}

	// draw our squad status
	wchar_t *text = NULL;
	if (m_bSquadMembersFollowing)
	{
		text = g_pVGuiLocalize->Find("#Valve_Hud_SQUAD_FOLLOWING");

		if (!text)
		{
			text = L"SQUAD FOLLOWING";
		}
	}
	else
	{
#ifdef HOE_DLL
#else
		if ( !player_squad_transient_commands.GetBool() )
#endif
		{
			text = g_pVGuiLocalize->Find("#Valve_Hud_SQUAD_STATIONED");

			if (!text)
			{
				text = L"SQUAD STATIONED";
			}
		}
	}

	if (text)
	{
		surface()->DrawSetTextFont(m_hTextFont);
		surface()->DrawSetTextColor(m_SquadTextColor);
#ifdef HOE_DLL
		int wide, tall;
		surface()->GetTextSize( m_hTextFont, text, wide, tall );
		surface()->DrawSetTextPos( ( GetWide() - wide ) / 2, GetTall() - text_ypos );
#else
		surface()->DrawSetTextPos(text_xpos, text_ypos);
#endif
		surface()->DrawPrintText(text, wcslen(text));
	}
}
#endif // HOE_DLL

