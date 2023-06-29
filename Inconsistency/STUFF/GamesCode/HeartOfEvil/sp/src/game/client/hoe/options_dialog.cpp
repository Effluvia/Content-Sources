#include "cbase.h"
using namespace vgui;
#include <vgui/IVGui.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/CheckButton.h>
#include <vgui_controls/Slider.h>
#include "options_dialog.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//ConVar cl_showmypanel("cl_showmypanel", "0", FCVAR_CLIENTDLL, "Sets the state of myPanel <state>");

//CMyPanel class: Tutorial example class
class CMyPanel : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE(CMyPanel, vgui::Frame); 

	CMyPanel(vgui::VPANEL parent);
	~CMyPanel() {};

	void ShowPanel(bool bShow);
	void SyncPanelWithOptions( void );
	void SyncCheckButtonWithConVar( const char *name, bool &value );
	void SyncOptionWithCheckButton( const char *name, bool &value );
	void SyncConVarWithOption( const char *name, const bool value );
	void SyncConVarWithOption( const char *name, const float value );

	virtual void OnTick();
	virtual void OnCommand(const char* pcCommand);

	// Frame method
	virtual void OnFinishedClose( void );

	bool m_bFirstTime;

	MESSAGE_FUNC_PARAMS( OnSliderMoved, "SliderMoved", data );
	Slider *m_pSlider;

	// Options
	bool hoe_ironsights_auto;
	bool hud_quickinfo;
	bool crosshair;
	float hoe_color_correction_weight;
};

// Constuctor: Initializes the Panel
CMyPanel::CMyPanel(vgui::VPANEL parent)
: BaseClass(NULL, "MyPanel")
{
	SetParent( parent );

//	SetTitle( "HOE Options", true );

	SetScheme("SourceScheme");

	SetKeyBoardInputEnabled( true );
	SetMouseInputEnabled( true );
	
	SetProportional( false );
	SetTitleBarVisible( true );
	SetMinimizeButtonVisible( false );
	SetMaximizeButtonVisible( false );
	SetCloseButtonVisible( true );
	SetSizeable( false );
	SetMoveable( true );
	SetVisible( false );

	// Can't create this in VGUI2 build mode
	m_pSlider = new Slider( this, "grayscale" );
	m_pSlider->SetRange( 0, 255 );
	m_pSlider->AddActionSignalTarget( this );

	LoadControlSettings("resource/UI/HOEOptions.res");
	InvalidateLayout();

//	vgui::ivgui()->AddTickSignal( GetVPanel(), 100 );

	m_bFirstTime = true;
}

//-----------------------------------------------------------------------------
void CMyPanel::OnTick()
{
//	BaseClass::OnTick();
//	SetVisible(cl_showmypanel.GetBool()); //CL_SHOWMYPANEL / 1 BY DEFAULT
}

//-----------------------------------------------------------------------------
void CMyPanel::OnCommand( const char *cmd )
{
	if ( !Q_stricmp( cmd, "ok") )
	{
		DevMsg( "ok\n" );
		SyncConVarWithOption( "hoe_ironsights_auto", hoe_ironsights_auto );
		SyncConVarWithOption( "hud_quickinfo", hud_quickinfo );
		SyncConVarWithOption( "crosshair", crosshair );
		SyncConVarWithOption( "hoe_color_correction_weight", hoe_color_correction_weight );
		ShowPanel( false );
	}
	else if ( !Q_stricmp( cmd, "cancel") )
	{
		DevMsg( "cancel\n" );
		ShowPanel( false );
	}
	else if ( !Q_stricmp( cmd, "apply") )
	{
		DevMsg( "apply\n" );
		SyncConVarWithOption( "hoe_ironsights_auto", hoe_ironsights_auto );
		SyncConVarWithOption( "hud_quickinfo", hud_quickinfo );
		SyncConVarWithOption( "crosshair", crosshair );
		SyncConVarWithOption( "hoe_color_correction_weight", hoe_color_correction_weight );
	}
	else if ( !Q_stricmp( cmd, "hoe_ironsights_auto") )
	{
		DevMsg( "hoe_ironsights_auto\n" );
		SyncOptionWithCheckButton( cmd, hoe_ironsights_auto );
	}
	else if ( !Q_stricmp( cmd, "hud_quickinfo") )
	{
		DevMsg( "hud_quickinfo\n" );
		SyncOptionWithCheckButton( cmd, hud_quickinfo );
	}
	else if ( !Q_stricmp( cmd, "crosshair") )
	{
		DevMsg( "crosshair\n" );
		SyncOptionWithCheckButton( cmd, crosshair );
	}
	else
	{
		// This will pass on the "close" command to the frame (i.e., when the close button is clicked)
		BaseClass::OnCommand( cmd );
	}
}

//-----------------------------------------------------------------------------
void CMyPanel::ShowPanel( bool bShow )
{
	if ( BaseClass::IsVisible() == bShow )
		return;

	if ( bShow )
	{
		SyncPanelWithOptions();

		if ( m_bFirstTime )
		{
			MoveToCenterOfScreen();
			m_bFirstTime = false;
		}

		SetKeyBoardInputEnabled( true );
		SetMouseInputEnabled( true );

		Activate();
	}
	else
	{
		Close(); // start fading out

		SetKeyBoardInputEnabled( false );
		SetMouseInputEnabled( false );
	}
}

//-----------------------------------------------------------------------------
void CMyPanel::OnFinishedClose( void )
{
	BaseClass::OnFinishedClose();
}

//-----------------------------------------------------------------------------
void CMyPanel::SyncPanelWithOptions( void )
{
	SyncCheckButtonWithConVar( "hoe_ironsights_auto", hoe_ironsights_auto );
	SyncCheckButtonWithConVar( "hud_quickinfo", hud_quickinfo );
	SyncCheckButtonWithConVar( "crosshair", crosshair );

	ConVarRef convar( "hoe_color_correction_weight" );
	if ( convar.IsValid() )
	{
		hoe_color_correction_weight = convar.GetFloat();
		m_pSlider->SetValue( convar.GetFloat() * 255 );
	}
}

//-----------------------------------------------------------------------------
void CMyPanel::SyncCheckButtonWithConVar( const char *name, bool &value )
{
	CheckButton *pButton = dynamic_cast<vgui::CheckButton *>( FindChildByName( name ) );
	if ( !pButton )
		return;

	ConVarRef convar( name );
	if ( convar.IsValid() )
		pButton->SetSelected( value = convar.GetBool() );
}

//-----------------------------------------------------------------------------
void CMyPanel::SyncOptionWithCheckButton( const char *name, bool &value )
{
	CheckButton *pButton = dynamic_cast<vgui::CheckButton *>( FindChildByName( name ) );
	if ( !pButton )
		return;

	value = pButton->IsSelected();
}

//-----------------------------------------------------------------------------
void CMyPanel::SyncConVarWithOption( const char *name, const bool value )
{
	ConVarRef convar( name );
	if ( convar.IsValid() )
		convar.SetValue( value );
}

//-----------------------------------------------------------------------------
void CMyPanel::SyncConVarWithOption( const char *name, const float value )
{
	ConVarRef convar( name );
	if ( convar.IsValid() )
		convar.SetValue( value );
}

//-----------------------------------------------------------------------------
void CMyPanel::OnSliderMoved( KeyValues *data )
{
	hoe_color_correction_weight = m_pSlider->GetValue() / 255.0f;
}

//Class: CMyPanelInterface Class. Used for construction.
class CMyPanelInterface : public IMyPanel
{
private:
	CMyPanel *MyPanel;
public:
	CMyPanelInterface()
	{
		MyPanel = NULL;
	}
	void Create(vgui::VPANEL parent)
	{
		MyPanel = new CMyPanel(parent);
	}
	void Destroy()
	{
		if (MyPanel)
		{
			MyPanel->SetParent( (vgui::Panel *)NULL);
			delete MyPanel;
		}
	}
	void ShowPanel( bool bShow )
	{
		MyPanel->ShowPanel( bShow );
	}
};

static CMyPanelInterface g_MyPanel;
IMyPanel* mypanel = (IMyPanel*)&g_MyPanel;

CON_COMMAND(ToggleMyPanel, "Toggles myPanel on or off")
{
//	cl_showmypanel.SetValue(!cl_showmypanel.GetBool());
	mypanel->ShowPanel( true );
};
