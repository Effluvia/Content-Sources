//========= Copyright © 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef BASEPANEL_H
#define BASEPANEL_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Panel.h>



class CBasePanel;

//MODDD - NEW
extern CBasePanel* BasePanel();
extern vgui::VPANEL GetGameUIBasePanel();


//-----------------------------------------------------------------------------
// Purpose: The panel at the top of the vgui panel hierarchy
//-----------------------------------------------------------------------------
class CBasePanel : public vgui::Panel
{
public:
	CBasePanel();
	//MODDD - new
	virtual ~CBasePanel();

	virtual void OnChildAdded(vgui::VPANEL child);
	virtual void PaintBackground();
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

	virtual void letsMaybeNot();

	enum EBackgroundState
	{
		BACKGROUND_NONE,
		BACKGROUND_BLACK,
		BACKGROUND_DESKTOPIMAGE,
		BACKGROUND_LOADING,
		BACKGROUND_LOADINGTRANSITION,
	};

	void SetBackgroundRenderState(EBackgroundState state);

private:
	void DrawBackgroundImage();

	EBackgroundState m_eBackgroundState;
	
	enum { BACKGROUND_ROWS = 3, BACKGROUND_COLUMNS = 4 };
	struct bimage_t
	{
		int imageID;
		int width, height;
	};
	bimage_t m_ImageID[BACKGROUND_ROWS][BACKGROUND_COLUMNS];
	typedef vgui::Panel BaseClass;
};


#endif // BASEPANEL_H
