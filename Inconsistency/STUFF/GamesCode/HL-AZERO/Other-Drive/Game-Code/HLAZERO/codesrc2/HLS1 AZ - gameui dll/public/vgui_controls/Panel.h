//========= Copyright � 1996-2003, Valve LLC, All rights reserved. ============
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================




// NOTICE!!! Consider restoring panel and panel.h from a backup if coming back to this point.
// Not that there's much to se anyway.






#ifndef PANEL_H
#define PANEL_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui/VGUI.h>
#include <vgui/Dar.h>
#include <vgui/MessageMap.h>
#include <vgui/IClientPanel.h>
#include <vgui/IScheme.h>
#include <vgui_controls/Controls.h>
#include <vgui_controls/PanelAnimationVar.h>
#include <Color.h>

// undefine windows function macros that overlap 
#ifdef PostMessage
#undef PostMessage
#endif

#ifdef SetCursor
#undef SetCursor
#endif

namespace vgui
{



//-----------------------------------------------------------------------------
// Purpose: Helper functions to construct vgui panels
//
//  SETUP_PANEL - will make a panel ready for use right now (i.e setup its colors, borders, fonts, etc)
//
template< class T >
inline T *SETUP_PANEL(T *panel)
{
	panel->MakeReadyForUse();
	return panel;
}

//
// CREATE_PANEL - creates a panel that is ready to use right now
//
//   example of use = to set the FG Color of a panel inside of a constructor (i.e before ApplySchemeSettings() has been run on the child)
//
#define CREATE_PANEL(type, parent, name) (SETUP_PANEL(new type(parent, name)))



//-----------------------------------------------------------------------------
// Purpose: For hudanimations.txt scripting of vars
//-----------------------------------------------------------------------------
class IPanelAnimationPropertyConverter
{
public:
	virtual void GetData( Panel *panel, KeyValues *kv, PanelAnimationMapEntry *entry ) = 0;
	virtual void SetData( Panel *panel, KeyValues *kv, PanelAnimationMapEntry *entry ) = 0;
	virtual void InitFromDefault( Panel *panel, PanelAnimationMapEntry *entry ) = 0;
};

//-----------------------------------------------------------------------------
// Purpose: Base interface to all vgui windows
//			All vgui controls that receive message and/or have a physical presence
//			on screen are be derived from Panel.
//			This is designed as an easy-access to the vgui-functionality; for more
//			low-level access to vgui functions use the IPanel/IClientPanel interfaces directly
//-----------------------------------------------------------------------------
class Panel : public IClientPanel
{


	
	// chain to map?
	//chaintomap
	// CHAIN TO MAP
	// (don't play with vgui2/controls/AnimationController.cpp, that shit will fuck you up [B])

	//DECLARE_CLASS_SIMPLE_NOBASE( Panel );
	typedef Panel ThisClass;
public:
	DECLARE_PANELANIMATION( Panel );
	

	//MODDD
	static char const *GetPanelClassName() { return "Panel"; }
	static char const *GetPanelBaseClassName() { return NULL; }
	




public:


	// returns pointer to Panel's vgui VPanel interface handle
	virtual VPANEL GetVPanel() { return _vpanel; }


	// virtual, yes or no??  found not virtual
	// private methods
	virtual void Think();
	virtual void PerformApplySchemeSettings();


	virtual void PaintTraverse(bool Repaint, bool allowForce = true);


		
	// painting
	virtual void Repaint();							// marks the panel as needing to be repainted
	virtual VPANEL IsWithinTraverse(int x, int y, bool traversePopups);	// recursive; returns a pointer to the panel at those coordinates


	virtual void GetInset(int &left, int &top, int &right, int &bottom);

	virtual void GetClipRect(int &x0, int &y0, int &x1, int &y1);


	// message handlers
	// override to get access to the message
	virtual void OnChildAdded(VPANEL child);		// called when a child has been added to this panel
	virtual void OnSizeChanged(int newWide, int newTall);	// called after the size of a panel has been changed
	virtual void InternalFocusChanged(bool lost);	// called when the focus gets changed
	
	// messaging
	virtual bool RequestInfo(KeyValues *outputData);				// returns true if output is successfully written.  You should always chain back to the base class if info request is not handled

	virtual void RequestFocus(int direction = 0);

	//MODDD - params changed
	//virtual bool RequestFocusPrev(Panel *panel = NULL);
	//virtual bool RequestFocusNext(Panel *panel = NULL);
	virtual bool RequestFocusPrev(VPANEL panel);
	virtual bool RequestFocusNext(VPANEL panel);
	virtual void OnMessage(KeyValues *params, VPANEL fromPanel);	// called when panel receives message; must chain back
	virtual VPANEL GetCurrentKeyFocus();

	virtual int GetTabPosition();

	const char *GetName();		// returns the name of this panel... never NULL
	const char *LOCAL_GetClassName(); // returns the class name of the panel (eg. Panel, Label, Button, etc.)

	// scheme access functions
	// !!!!!!!!!! only one, eh.  AHHH FUCK
	virtual HScheme GetScheme();
	virtual HScheme GetScheme(const char *tag);


		
	virtual bool IsProportional() { return m_bProportional; }
	virtual bool IsAutoDeleteSet();


	//virtual bool IsProportional(void);
	//virtual bool IsAutoDeleteSet(void);
	virtual void DeletePanel(void);

	virtual void *QueryInterface(EInterfaceID id);

	// used to get the Panel * for users with only IClientPanel
	virtual Panel *GetPanel() { return this; }

	// returns the name of the module that this instance of panel was compiled into
	virtual /*const*/ char *GetModuleName();

	virtual PanelMessageMap* GetMessageMap();

	// panel visibility
	// invisible panels and their children do not drawn, updated, or receive input messages
	virtual void SetVisible(bool state);
	virtual bool IsVisible();

	virtual void PostMessage(VPANEL target, KeyValues *message, float delaySeconds = 0.0f);


	virtual void OnMove(void);

	// panel hierarchy
	virtual Panel *GetParent();

	virtual VPANEL GetVParent();

	virtual void SetParent(Panel *newParent);
	virtual void SetParent(VPANEL newParent);
	virtual bool HasParent(VPANEL potentialParent);
	virtual void SetAutoDelete(bool state);		// if set to true, panel automatically frees itself when parent is deleted


	virtual void AddActionSignalTarget(Panel *messageTarget);
	// was there a 2nd one?? the fuck

	virtual void RemoveActionSignalTarget(Panel *oldTarget);
	virtual void PostActionSignal(KeyValues *message);			// sends a message to the current actionSignalTarget(s)
	virtual bool RequestInfoFromChild(const char *childName, KeyValues *outputData);
	virtual void PostMessageToChild(const char *childName, KeyValues *messsage);
	virtual void PostMessage(Panel *target, KeyValues *message, float delaySeconds = 0.0f);


	virtual bool SetInfo(KeyValues *inputData);						// sets a specified value in the control - inverse of the above

	// drawing state
	virtual void   SetEnabled(bool state);
	virtual bool   IsEnabled();
	virtual bool   IsPopup();	// has a parent, but is in it's own space
	virtual void   MoveToFront();

	// colors
	virtual void SetBgColor(Color color);
	virtual void SetFgColor(Color color);
	virtual Color GetBgColor();
	virtual Color GetFgColor();

	virtual void SetCursor(HCursor cursor);
	virtual HCursor GetCursor();
	virtual bool HasFocus();
	virtual void InvalidateLayout(bool layoutNow = false, bool reloadScheme = false);
	virtual void   SetTabPosition(int position);


	
	virtual void SetBorder(IBorder *border);
	virtual IBorder *GetBorder();
	virtual void SetPaintBorderEnabled(bool state);
	virtual void SetPaintBackgroundEnabled(bool state);
	virtual void SetPaintEnabled(bool state);
	virtual void SetPostChildPaintEnabled(bool state);
	
	virtual void GetPaintSize(int &wide, int &tall);
	virtual void SetBuildGroup(BuildGroup *buildGroup);
	virtual bool IsBuildGroupEnabled();
	virtual bool IsCursorNone();
	virtual bool IsCursorOver();		// returns true if the cursor is currently over the panel
	virtual void MarkForDeletion();		// object will free it's memory next tick
	virtual bool IsLayoutInvalid();		// does this object require a perform layout?
	virtual Panel *HasHotkey(wchar_t key);			// returns the panel that has this hotkey
	virtual bool IsOpaque();

	virtual void SetScheme(const char *tag);
	virtual void SetScheme(HScheme scheme);
	virtual Color GetSchemeColor(const char *keyName,IScheme *pScheme);
	virtual Color GetSchemeColor(const char *keyName, Color defaultColor,IScheme *pScheme);



	// called when scheme settings need to be applied; called the first time before the panel is painted
	virtual void ApplySchemeSettings(IScheme *pScheme);


	// interface to build settings
	// takes a group of settings and applies them to the control
	virtual void ApplySettings(KeyValues *inResourceData);

	// records the settings into the resource data
	virtual void GetSettings(KeyValues *outResourceData);

	// gets a description of the resource for use in the UI
	// format: <type><whitespace | punctuation><keyname><whitespace| punctuation><type><whitespace | punctuation><keyname>...
	// unknown types as just displayed as strings in the UI (for future UI expansion)
	virtual const char *GetDescription();

	// user configuration settings
	// this is used for any control details the user wants saved between sessions
	// eg. dialog positions, last directory opened, list column width
	virtual void ApplyUserConfigSettings(KeyValues *userConfig);
	// returns user config settings for this control
	virtual void GetUserConfigSettings(KeyValues *userConfig);
	virtual bool HasUserConfigSettings();
	virtual void OnThink();							// called every frame before painting, but only if panel is visible
	virtual void OnCommand(const char *command);	// called when a panel receives a command
	virtual void OnMouseCaptureLost();				// called after the panel loses mouse capture
	virtual void OnSetFocus();						// called after the panel receives the keyboard focus
	virtual void OnKillFocus();						// called after the panel loses the keyboard focus
	virtual void OnDelete();						// called to delete the panel; Panel::OnDelete() does simply { delete this; }



	// !!! REMOVO
	void InternalPerformLayout();
	void InternalSetCursor();

	
	// called every frame if ivgui()->AddTickSignal() is called
	virtual void OnTick();

	// input messages
	virtual void OnCursorMoved(int x, int y);
	virtual void OnCursorEntered();
	virtual void OnCursorExited();
	virtual void OnMousePressed(MouseCode code);
	virtual void OnMouseDoublePressed(MouseCode code);
	virtual void OnMouseReleased(MouseCode code);
	virtual void OnMouseWheeled(int delta);

	// base implementation forwards Key messages to the Panel's parent 
	// - override to 'swallow' the input
	virtual void OnKeyCodePressed(KeyCode code);
	virtual void OnKeyCodeTyped(KeyCode code);
	virtual void OnKeyTyped(wchar_t unichar);
	virtual void OnKeyCodeReleased(KeyCode code);
	virtual void OnKeyFocusTicked(); // every window gets key ticked events

	// forwards mouse messages to the panel's parent
	virtual void OnMouseFocusTicked(); // every window gets mouse ticked events

	// message handlers that don't go through the message pump
	virtual void PaintBackground();
	virtual void Paint();
	virtual void PaintBorder();
	virtual void PaintBuildOverlay();		// the extra drawing for when in build mode
	virtual void PostChildPaint();
	virtual void PerformLayout();


	// this enables message mapping for this class - requires matching IMPLEMENT_PANELDESC() in the .cpp file
	DECLARE_PANELMAP();
	//eat my balls
	//virtual PanelMap_t* GetPanelMap();


	virtual void SetProportional(bool state);

	// input interest
	virtual void SetMouseInputEnabled( bool state );
	virtual void SetKeyBoardInputEnabled( bool state );
	virtual bool IsMouseInputEnabled();

	virtual bool IsKeyBoardInputEnabled();
	virtual void OnRequestFocus(VPANEL subFocus, VPANEL defaultPanel);
	void InternalCursorMoved(int x,int y);
	void InternalCursorEntered();
	void InternalCursorExited();
	void InternalMousePressed(MouseCode code);
	void InternalMouseDoublePressed(MouseCode code);
	void InternalMouseReleased(MouseCode code);
	void InternalMouseWheeled(int delta);
	void InternalKeyCodePressed(KeyCode code);
	void InternalKeyCodeTyped(KeyCode code);
	void InternalKeyTyped(wchar_t unichar);
	void InternalKeyCodeReleased(KeyCode code);
	void InternalKeyFocusTicked();
	void InternalMouseFocusTicked();

	void InternalInvalidateLayout();
	void InternalMove();


	
public:

	// For property mapping
	static void InitPropertyConverters( void );
	static void AddPropertyConverter( char const *typeName, IPanelAnimationPropertyConverter *converter );

	//-----------------------------------------------------------------------------
	// CONSTRUCTORS
	// these functions deal with the creation of the Panel
	// the Panel automatically gets a handle to a vgui-internal panel, the ipanel(), upon construction
	// vgui interfaces deal only with ipanel(), not Panel directly
	Panel();
	Panel(Panel *parent);
	Panel(Panel *parent, const char *panelName);
	Panel(Panel *parent, const char *panelName, HScheme scheme);

	virtual ~Panel();

	//-----------------------------------------------------------------------------
	// PANEL METHODS
	// these functions all manipulate panels
	// they cannot be derived from
	void SetName(const char *panelName);  // sets the name of the panel - used as an identifier
	
	void MakeReadyForUse(); // fully construct this panel so its ready for use right now (i.e fonts loaded, colors set, default label text set, ...)

	// panel position & size
	// all units are in pixels
	void SetPos(int x,int y);		// sets position of panel, in local space (ie. relative to parent's position)
	void GetPos(int &x,int &y);		// gets local position of panel
	void SetSize(int wide,int tall);	// sets size of panel
	void GetSize(int &wide, int &tall);	// gets size of panel
	void SetBounds(int x, int y, int wide, int tall);		// combination of SetPos/SetSize
	void GetBounds(int &x, int &y, int &wide, int &tall);	// combination of GetPos/GetSize
	int  GetWide();	// returns width of panel
	void SetWide(int wide);	// sets width of panel
	int  GetTall();	// returns height of panel
	void SetTall(int tall);	// sets height of panel
	void SetMinimumSize(int wide,int tall);		// sets the minimum size the panel can go
	void GetMinimumSize(int& wide,int& tall);	// gets the minimum size
	bool IsBuildModeEditable();	  // editable in the buildModeDialog?
	void SetBuildModeEditable(bool state);  // set buildModeDialog editable
	bool IsBuildModeDeletable();  // deletable in the buildModeDialog?
	void SetBuildModeDeletable(bool state);	// set buildModeDialog deletable
	bool IsBuildModeActive();	// true if we're currently in edit mode
	void SetZPos(int z);	// sets Z ordering - lower numbers are always behind higher z's


	virtual bool   IsWithin(int x,int y); //in screen space
	virtual void   LocalToScreen(int& x,int& y);
	virtual void   ScreenToLocal(int& x,int& y);
	virtual void   MakePopup(bool showTaskbarIcon = true,bool disabled = false);		// turns the panel into a popup window (ie. can draw outside of it's parents space)
	
	virtual int GetChildCount();
	virtual Panel *GetChild(int index);
	virtual Panel *FindChildByName(const char *childName, bool recurseDown = false);



	// pin positions for auto-layout
	enum PinCorner_e 
	{
		PIN_TOPLEFT = 0,
		PIN_TOPRIGHT,
		PIN_BOTTOMLEFT,
		PIN_BOTTOMRIGHT,
	};
	// specifies the corner the panel is to be pinned to if a dialog is resized
	void SetPinCorner(PinCorner_e pinCorner);
	PinCorner_e GetPinCorner();

	// specifies the auto-resize directions for the panel
	enum AutoResize_e
	{
		AUTORESIZE_NO = 0,
		AUTORESIZE_RIGHT,
		AUTORESIZE_DOWN,
		AUTORESIZE_DOWNANDRIGHT,
	};
	void SetAutoResize(AutoResize_e resizeDir);
	AutoResize_e GetAutoResize();


    virtual bool CanBeDefaultButton(void);

	virtual void SetPaintBackgroundType(int type);  // 0 for normal(opaque), 1 for single texture from Texture1, and 2 for rounded box w/ four corner textures
	



	//int GetTabPosition(void);

	void SetTooltipText(const char *text);
	void ResetTooltipDelay();
	void SetTooltipFormatToSingleLine();
	void SetTooltipFormatToMultiLine();


	//MODDD
	/*
	virtual void DrawTexturedBox( int x, int y, int wide, int tall, Color color, float normalizedAlpha );
	virtual void DrawBox(int x, int y, int wide, int tall, Color color, float normalizedAlpha );
	*/












	// AMOS AMOS AMOS AMOS AMOS

	void Init(int x,int y,int wide,int tall);
	void PreparePanelMap(PanelMap_t *panelMap);

	bool InternalRequestInfo( PanelAnimationMap *map, KeyValues *outputData );
	bool InternalSetInfo( PanelAnimationMap *map, KeyValues *inputData );

	PanelAnimationMapEntry *FindPanelAnimationEntry( char const *scriptname, PanelAnimationMap *map );

	// Recursively invoke settings for PanelAnimationVars
	void InternalApplySettings( PanelAnimationMap *map, KeyValues *inResourceData);
	void InternalInitDefaultValues( PanelAnimationMap *map );














	// data
	VPANEL _vpanel;	// handle to a vgui panel
	HCursor _cursor;
	bool _markedForDeletion;
	IBorder	*_border;
	bool _needsRepaint;
	bool _enabled;
	BuildGroup *_buildGroup;
	Color _fgColor;		// foreground color
	Color _bgColor;		// background color
	bool _paintBorderEnabled;
	bool _paintBackgroundEnabled;
	bool _paintEnabled;
	bool _postChildPaintEnabled;
	char *_panelName;		// string name of the panel - only unique within the current context
	bool _needsLayout;
	bool _needsSchemeUpdate;
	bool _autoDelete;
	int	_tabPosition;		// the panel's place in the tab ordering
	Dar<HPanel> _actionSignalTargetDar;	// the panel to direct notify messages to ("Command", "TextChanged", etc.)

	PinCorner_e _pinCorner;	// the corner of the dialog this panel is pinned to
	AutoResize_e _autoResizeDirection; // the directions in which the panel will auto-resize to
	
	unsigned int m_iScheme; // handle to the scheme to use

	enum
	{
		BUILDMODE_EDITABLE					= 0x01,
		BUILDMODE_DELETABLE					= 0x02,
		BUILDMODE_SAVE_XPOS_RIGHTALIGNED	= 0x04,
		BUILDMODE_SAVE_XPOS_CENTERALIGNED	= 0x08,
		BUILDMODE_SAVE_YPOS_BOTTOMALIGNED	= 0x10,
		BUILDMODE_SAVE_YPOS_CENTERALIGNED	= 0x20,
	};
	unsigned char _buildModeFlags; // flags that control how the build mode dialog handles this panel
	bool m_bProportional;
	bool m_bInPerformLayout;
	Tooltip *m_pTooltips;


	/*
	// 1 == Textured (TextureId1 only)
	// 2 == Rounded Corner Box
	CPanelAnimationVar( int, m_nPaintBackgroundType, "PaintBackgroundType", "0" );

	CPanelAnimationVarAliasType( int, m_nBgTextureId1, "Texture1", "vgui/hud/800corner1", "textureid" );
	CPanelAnimationVarAliasType( int, m_nBgTextureId2, "Texture2", "vgui/hud/800corner2", "textureid" );
	CPanelAnimationVarAliasType( int, m_nBgTextureId3, "Texture3", "vgui/hud/800corner3", "textureid" );
	CPanelAnimationVarAliasType( int, m_nBgTextureId4, "Texture4", "vgui/hud/800corner4", "textureid" );
	*/

	friend class Panel;
	friend class BuildGroup;
	friend class BuildModeDialog;
	friend class PHandle;
};

} // namespace vgui


#endif // PANEL_H
