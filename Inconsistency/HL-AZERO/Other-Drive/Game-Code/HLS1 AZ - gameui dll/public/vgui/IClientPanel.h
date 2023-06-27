//========= Copyright © 1996-2003, Valve LLC, All rights reserved. ============
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

#ifndef ICLIENTPANEL_H
#define ICLIENTPANEL_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui/VGUI.h>

//MODDD
// no need for this 'undef' anymore, GetClassName methods renamed to LOCAL_GetClassName
/*
#ifdef GetClassName
#undef GetClassName
#endif
*/

class KeyValues;

namespace vgui
{

class Panel;
class SurfaceBase;

enum EInterfaceID
{
	ICLIENTPANEL_STANDARD_INTERFACE = 0,
};


// ?????????
struct PanelMessageMap {
	/*
	struct CUtlVector<vgui2--MessageMapItem_t> entries;
	bool processed;
	undefined field_0x15;
	undefined field_0x16;
	undefined field_0x17;
	struct PanelMessageMap * baseMap;
	*/
	char * (* pfnClassName)(void);
};




//-----------------------------------------------------------------------------
// Purpose: Interface from vgui panels -> Client panels
//			This interface cannot be changed without rebuilding all vgui projects
//			Primarily this interface handles dispatching messages from core vgui to controls
//			The additional functions are all their for debugging or optimization reasons
//			To add to this later, use QueryInterface() to see if they support new interfaces
//-----------------------------------------------------------------------------
class IClientPanel
{
public:
	virtual VPANEL GetVPanel() = 0;

	// straight interface to Panel functions
	virtual void Think() = 0;
	virtual void PerformApplySchemeSettings() = 0;
	virtual void PaintTraverse(bool forceRepaint, bool allowForce) = 0;
	virtual void Repaint() = 0;
	virtual VPANEL IsWithinTraverse(int x, int y, bool traversePopups) = 0;
	virtual void GetInset(int &top, int &left, int &right, int &bottom) = 0;
	virtual void GetClipRect(int &x0, int &y0, int &x1, int &y1) = 0;
	virtual void OnChildAdded(VPANEL child) = 0;
	virtual void OnSizeChanged(int newWide, int newTall) = 0;

	virtual void InternalFocusChanged(bool lost) = 0;
	virtual bool RequestInfo(KeyValues *outputData) = 0;
	virtual void RequestFocus(int direction = 0) = 0;


	virtual bool RequestFocusPrev(VPANEL panel) { return true; };// = 0;
	virtual bool RequestFocusNext(VPANEL panel){return true;};// = 0;



	virtual void OnMessage(KeyValues *params, VPANEL ifromPanel) = 0;
	virtual VPANEL GetCurrentKeyFocus() = 0;

	//MODDD - NEW
	virtual int GetTabPosition() { return 0; };// = 0;

	// for debugging purposes
	virtual const char *GetName() = 0;
	virtual const char *LOCAL_GetClassName() = 0;

	// get scheme handles from panels
	virtual HScheme GetScheme() = 0;

	//MODDD - NEW
	virtual bool IsProportional(void) { return false; };// = 0;
	virtual bool IsAutoDeleteSet(void) { return false; };// = 0;
	virtual void DeletePanel(void) {};// = 0;


	// interfaces
	virtual void *QueryInterface(EInterfaceID id) = 0;

	// returns a pointer to the vgui controls baseclass Panel *
	virtual Panel *GetPanel() = 0;

	// returns the name of the module this panel is part of
	virtual /*const*/ char *GetModuleName() = 0;


	virtual PanelMessageMap* GetMessageMap(void) { return NULL; };// = 0;

	virtual void SetVisible(bool param_2) {};// = 0;
	virtual bool IsVisible(void) { return false; };// = 0;

	virtual void PostMessage(VPANEL target,KeyValues *message,float delaySeconds) {};// = 0;

	// this should go below 'PostMessageToChild'?  allthough then again this class doesn't even have that.
	virtual void PostMessage(Panel *target,KeyValues *message,float delay) {};// = 0;

	virtual void OnMove(void) {};// = 0;

	/*
GetParent
GetVParent
SetParent
SetParent
HasParent
SetAutoDelete
AddActionSignalTarget
AddActionSignalTarget
RemoveActionSignalTarget
PostActionSignal
RequestInfoFromChild
PostMessageToChild
PostMessage
SetInfo
SetEnabled
IsEnabled
IsPopup
MoveToFront
SetBgColor
SetFgColor
GetBgColor
GetFgColor
SetCursor
GetCursor
HasFocus
InvalidateLayout
SetTabPosition
SetBorder
GetBorder
SetPaintBorderEnabled
SetPaintBackgroundEnabled
SetPaintEnabled
SetPostChildPaintEnabled
GetPaintSize
SetBuildGroup
IsBuildGroupEnabled
IsCursorNone
IsCursorOver
MarkForDeletion
IsLayoutInvalid
HasHotkey
IsOpaque
SetScheme
SetScheme
GetSchemeColor
GetSchemeColor
ApplySchemeSettings
ApplySettings
GetSettings
GetDescription
ApplyUserConfigSettings
GetUserConfigSettings
HasUserConfigSettings
OnThink
OnCommand
OnMouseCaptureLost
OnSetFocus
OnKillFocus
OnDelete
void __thiscall OnTick(Panel *this);

OnCursorMoved
OnCursorEntered
OnCursorExited
OnMousePressed
OnMouseDoublePressed
OnMouseReleased
OnMouseWheeled
OnKeyCodePressed
OnKeyCodeTyped
OnKeyTyped
OnKeyCodeReleased
OnKeyFocusTicked
OnMouseFocusTicked
PaintBackground
Paint
PaintBorder
PaintBuildOverlay
PostChildPaint
PerformLayout
GetPanelMap
SetProportional
SetMouseInputEnabled
SetKeyBoardInputEnabled
IsMouseInputEnabled
IsKeyBoardInputEnabled
OnRequestFocus
InternalCursorMoved
InternalCursorEntered
InternalCursorExited
InternalMousePressed
InternalMouseDoublePressed
InternalMouseReleased
InternalMouseWheeled
InternalKeyCodePressed
InternalKeyCodeTyped
InternalKeyTyped
InternalKeyCodeReleased
InternalKeyFocusTicked
InternalMouseFocusTicked
InternalInvalidateLayout
InternalMove
*/


};

} // namespace vgui


#endif // ICLIENTPANEL_H
