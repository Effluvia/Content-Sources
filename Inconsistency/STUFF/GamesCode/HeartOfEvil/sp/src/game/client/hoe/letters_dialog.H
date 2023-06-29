
#include <vgui_controls/Frame.h>
#include <vgui_controls/Button.h>

#include <game/client/iviewport.h>

#include <vgui/KeyCode.h>
#include <UtlVector.h>

namespace vgui
{
	class RichText;
}

#define PANEL_LETTERS "letters"

//-----------------------------------------------------------------------------
// Purpose: Displays the team menu
//-----------------------------------------------------------------------------
class CLettersDialog : public vgui::Frame, public IViewPortPanel
{
public:
	DECLARE_CLASS_SIMPLE( CLettersDialog, vgui::Frame );

	CLettersDialog(IViewPort *pViewPort);
	virtual ~CLettersDialog();

	virtual const char *GetName( void ) { return PANEL_LETTERS; }
	virtual void SetData(KeyValues *data) {};
	virtual void Reset() {};
	virtual void Update() {};
	virtual bool NeedsUpdate( void ) { return false; }
	virtual bool HasInputElements( void ) { return true; }
	virtual void ShowPanel( bool bShow );
	virtual void PerformLayout( void );

	virtual void Close( void );

	// both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
	vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel(); }
  	virtual bool IsVisible() { return BaseClass::IsVisible(); }
	virtual void SetParent( vgui::VPANEL parent ) { BaseClass::SetParent( parent ); }
	
	// VGUI2 overrides
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void OnKeyCodePressed(vgui::KeyCode code);

	// helper functions
	virtual void SetListOfLetters();
	virtual void LoadLetter( const char *letterName );

	// ListPanel calls this when an item is selected
	MESSAGE_FUNC_PARAMS( OnItemSelected, "ItemSelected", data );

	IViewPort *m_pViewPort;
	vgui::ListPanel *m_pListOfLetters;
	vgui::RichText *m_pLetterText;
	ButtonCode_t m_iTabKey;
	bool m_bFirstTime;

	char m_szLetterName[ MAX_PATH ];
};