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

#include <stdio.h>
#define PROTECTED_THINGS_DISABLE

#include <vgui/KeyCode.h>
#include <vgui/Cursor.h>
#include <vgui/MouseCode.h>
#include <KeyValues.h>
#include <vgui/IInput.h>
#include <vgui/ISystem.h>
#include <vgui/IVGui.h>
#include <vgui/ISurface.h>

#include <vgui_controls/BuildGroup.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/PHandle.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/MessageBox.h>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
BuildGroup::BuildGroup(Panel *parentPanel, Panel *contextPanel)
{
	_enabled=false;
	_snapX=1;
	_snapY=1;
	_cursor_sizenwse = dc_sizenwse;
	_cursor_sizenesw = dc_sizenesw;
	_cursor_sizewe = dc_sizewe;
	_cursor_sizens = dc_sizens;
	_cursor_sizeall = dc_sizeall;
	_currentPanel=null;
	_dragging=false;
	m_pResourceName=NULL;
	m_pResourcePathID = NULL;
	m_hBuildDialog=NULL;
	m_pParentPanel=parentPanel;
	for (int i=0; i<4; ++i)
		_rulerNumber[i] = NULL;
	SetContextPanel(contextPanel);
	_controlGroup = NULL;
	_groupDeltaX = 0;
	_groupDeltaX = 0;
	_showRulers = false;

}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
BuildGroup::~BuildGroup()
{
	if (m_hBuildDialog)
		delete m_hBuildDialog.Get();
	m_hBuildDialog = NULL;

	delete [] m_pResourceName;
	delete [] m_pResourcePathID;

	for (int i=0; i <4; ++i)
	{
		if (_rulerNumber[i])
		{
			delete _rulerNumber[i];
			_rulerNumber[i]= NULL;
		}
	}
	
}

//-----------------------------------------------------------------------------
// Purpose: Toggles build mode on/off
// Input  : state - new state
//-----------------------------------------------------------------------------
void BuildGroup::SetEnabled(bool state)
{
	if(_enabled != state)
	{
		_currentPanel = NULL;

		if ( state )
		{
			ActivateBuildDialog();
		}
		else
		{
			// hide the build dialog
			if ( m_hBuildDialog )
			{
				m_hBuildDialog->OnCommand("Close");
			}

			// request focus for our main panel
			m_pParentPanel->RequestFocus();
		}
	}

	_enabled = state;
}

//-----------------------------------------------------------------------------
// Purpose: Check if buildgroup is enabled
//-----------------------------------------------------------------------------
bool BuildGroup::IsEnabled()
{
	return _enabled;
}

//-----------------------------------------------------------------------------
// Purpose: Get the list of panels that are currently selected
//-----------------------------------------------------------------------------
CUtlVector<PHandle> *BuildGroup::GetControlGroup()
{
   return &_controlGroup;
}

//-----------------------------------------------------------------------------
// Purpose:	Check if ruler display is activated
//-----------------------------------------------------------------------------
bool BuildGroup::HasRulersOn()
{
   return _showRulers;
}

//-----------------------------------------------------------------------------
// Purpose:	Toggle ruler display 
//-----------------------------------------------------------------------------
void BuildGroup::ToggleRulerDisplay()
{
	_showRulers = !_showRulers;

	if (_rulerNumber[0] == NULL) // rulers haven't been initialized
	{
		_rulerNumber[0] = new Label(m_pBuildContext, NULL, "");
		_rulerNumber[1] = new Label(m_pBuildContext, NULL, "");
		_rulerNumber[2] = new Label(m_pBuildContext, NULL, "");
		_rulerNumber[3] = new Label(m_pBuildContext, NULL, "");
	}
    SetRulerLabelsVisible(_showRulers);

   m_pBuildContext->Repaint();
}


//-----------------------------------------------------------------------------
// Purpose:	Tobble visibility of ruler number labels
//-----------------------------------------------------------------------------
void BuildGroup::SetRulerLabelsVisible(bool state)
{
	_rulerNumber[0]->SetVisible(state);
	_rulerNumber[1]->SetVisible(state);
	_rulerNumber[2]->SetVisible(state);
	_rulerNumber[3]->SetVisible(state);
}

void BuildGroup::ApplySchemeSettings( IScheme *pScheme )
{
	DrawRulers();
}

//-----------------------------------------------------------------------------
// Purpose:	Draw Rulers on screen if conditions are right
//-----------------------------------------------------------------------------
void BuildGroup::DrawRulers()
{		
	// don't draw if visibility is off
	if (!_showRulers)
	{
		return;
	}
	
	// no drawing if we selected the context panel
	if (m_pBuildContext == _currentPanel)
	{
		SetRulerLabelsVisible(false);
		return;
	}
	else
		SetRulerLabelsVisible(true);
	
	int x, y, wide, tall;
	// get base panel's postition
	m_pBuildContext->GetBounds(x, y, wide, tall);
	m_pBuildContext->ScreenToLocal(x,y);
	
	int cx, cy, cwide, ctall;
	_currentPanel->GetBounds (cx, cy, cwide, ctall);
	
	surface()->PushMakeCurrent(m_pBuildContext->GetVPanel(), false);	
	
	// draw rulers
	surface()->DrawSetColor(255, 255, 255, 255);	// white color
	
	surface()->DrawFilledRect(0, cy, cx, cy+1);           //top horiz left
	surface()->DrawFilledRect(cx+cwide, cy, wide, cy+1);  //top horiz right
	
	surface()->DrawFilledRect(0, cy+ctall-1, cx, cy+ctall);   //bottom horiz left
	surface()->DrawFilledRect(cx+cwide, cy+ctall-1, wide, cy+ctall);   //bottom	 horiz right
	
	surface()->DrawFilledRect(cx,0,cx+1,cy);         //top vert left
	surface()->DrawFilledRect(cx+cwide-1,0, cx+cwide, cy);  //top vert right
	
	surface()->DrawFilledRect(cx,cy+ctall, cx+1, tall); //bottom vert left
	surface()->DrawFilledRect(cx+cwide-1, cy+ctall, cx+cwide, tall); //bottom vert right   
	
	surface()->PopMakeCurrent(m_pBuildContext->GetVPanel());
	
	// now let's put numbers with the rulers
	char textstring[20];
	sprintf (textstring, "%d", cx);
	_rulerNumber[0]->SetText(textstring);
	int twide, ttall;
	_rulerNumber[0]->GetContentSize(twide,ttall);
	_rulerNumber[0]->SetSize(twide,ttall);
	_rulerNumber[0]->SetPos(cx/2-twide/2, cy-ttall+3);
	
	sprintf (textstring, "%d", cy);
	_rulerNumber[1]->SetText(textstring);
	_rulerNumber[1]->GetContentSize(twide,ttall);
	_rulerNumber[1]->SetSize(twide,ttall);
	_rulerNumber[1]->GetSize(twide,ttall);
	_rulerNumber[1]->SetPos(cx-twide + 3, cy/2-ttall/2);
	
	sprintf (textstring, "%d", cy);
	_rulerNumber[2]->SetText(textstring);
	_rulerNumber[2]->GetContentSize(twide,ttall);
	_rulerNumber[2]->SetSize(twide,ttall);
	_rulerNumber[2]->SetPos(cx+cwide+(wide-cx-cwide)/2 - twide/2,  cy+ctall-3);
	
	sprintf (textstring, "%d", cy);
	_rulerNumber[3]->SetText(textstring);
	_rulerNumber[3]->GetContentSize(twide,ttall);
	_rulerNumber[3]->SetSize(twide,ttall);
	_rulerNumber[3]->SetPos(cx+cwide, cy+ctall+(tall-cy-ctall)/2 - ttall/2);
	
}

//-----------------------------------------------------------------------------
// Purpose: respond to cursor movments
//-----------------------------------------------------------------------------
void BuildGroup::CursorMoved(int x, int y, Panel *panel)
{
	Assert(panel);

	// no moving uneditable panels
	// commented out because this has issues with panels moving 
	// to front and obscuring other panels
	//if (!panel->IsBuildModeEditable())
	//	return;

	if (_dragging)
	{
		input()->GetCursorPos(x, y);
		
		if (_dragMouseCode == MOUSE_RIGHT)
		{
			panel->SetSize(x - _dragStartCursorPos[0], y - _dragStartCursorPos[1]);
			ApplySnap(panel);
		}
		else
		{
			for (int i=0; i < _controlGroup.Size(); ++i)
			{
				// now fix offset of member panels with respect to the one we are dragging
				Panel *groupMember = _controlGroup[i].Get();
			   	groupMember->SetPos(_dragStartPanelPos[0] + _groupDeltaX[i] +(x-_dragStartCursorPos[0]), _dragStartPanelPos[1] + _groupDeltaY[i] +(y-_dragStartCursorPos[1]));
				ApplySnap(groupMember);				
			}
		}

		// update the build dialog
		if (m_hBuildDialog)
		{
			KeyValues *keyval = new KeyValues("UpdateControlData");
			keyval->SetPtr("panel", GetCurrentPanel());
			ivgui()->PostMessage(m_hBuildDialog->GetVPanel(), keyval, NULL, 0);

			keyval = new KeyValues("EnableSaveButton");	
			ivgui()->PostMessage(m_hBuildDialog->GetVPanel(), keyval, NULL, 0);	
		}
		
		panel->Repaint();
		Panel *parent = panel->GetParent();
		if (parent)
		{
			parent->Repaint();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void BuildGroup::MousePressed(MouseCode code, Panel *panel)
{
	Assert(panel);
	int i;

	// if people click on the base build dialog panel.
	if (panel == m_hBuildDialog)
	{
		// hide the click menu if its up
		ivgui()->PostMessage(m_hBuildDialog->GetVPanel(), new KeyValues("HideNewControlMenu"), NULL, 0);
		return;
	}

	// don't select unnamed items
	if (strlen(panel->GetName()) < 1)
		return;
	
	bool shift = ( input()->IsKeyDown(KEY_LSHIFT) || input()->IsKeyDown(KEY_RSHIFT) );	
	if (!shift)
	{
		_controlGroup.RemoveAll();	
	}

	if (code == MOUSE_LEFT)
	{	
		// don't respond if we click on ruler numbers
		if (_showRulers) // rulers are visible
		{
			for ( int i=0; i < 4; i++)
			{
				if ( panel == _rulerNumber[i])
					return;
			}
		}

		_dragging = true;
		_dragMouseCode = code;
		ivgui()->PostMessage(m_hBuildDialog->GetVPanel(), new KeyValues("HideNewControlMenu"), NULL, 0);
		
		int x, y;
		input()->GetCursorPos(x, y);
		
		_dragStartCursorPos[0] = x;
		_dragStartCursorPos[1] = y;
	
		
		input()->SetMouseCapture(panel->GetVPanel());
		
		_groupDeltaX.RemoveAll();
		_groupDeltaY.RemoveAll();

		// basepanel is the panel that all the deltas will be calculated from.
		// it is the last panel we clicked in because if we move the panels  as a group
		// it would be from that one
		Panel *basePanel = NULL;
		// find the panel we clicked in, that is the base panel
		// it might already be in the group
		for (i=0; i< _controlGroup.Size(); ++i)	
		{
			if (panel == _controlGroup[i].Get())
			{
				basePanel = panel;
				break;
			}
		}

		// if its not in the group we just added this panel. get it in the group 
		if (basePanel == NULL)
		{
			PHandle temp;
			temp = panel;
			_controlGroup.AddToTail(temp);
			basePanel = panel;
		}
		
		basePanel->GetPos(x,y);
		_dragStartPanelPos[0]=x;
		_dragStartPanelPos[1]=y;

		// figure out the deltas of the other panels from the base panel
		for (i=0; i<_controlGroup.Size(); ++i)
		{
			int cx, cy;
			_controlGroup[i].Get()->GetPos(cx, cy);
			_groupDeltaX.AddToTail(cx - x);
			_groupDeltaY.AddToTail(cy - y);
		}
						
		// if this panel wasn't already selected update the buildmode dialog controls to show its info
		if(_currentPanel != panel)
		{			
			_currentPanel = panel;
			
			if ( m_hBuildDialog )
			{
				// think this is taken care of by SetActiveControl.
				//ivgui()->PostMessage(m_hBuildDialog->GetVPanel(), new KeyValues("ApplyDataToControls"), NULL);
				
				KeyValues *keyval = new KeyValues("SetActiveControl");
				keyval->SetPtr("PanelPtr", GetCurrentPanel());
				ivgui()->PostMessage(m_hBuildDialog->GetVPanel(), keyval, NULL, 0);
			}		
		}		

		// store undo information upon panel selection.
		ivgui()->PostMessage(m_hBuildDialog->GetVPanel(), new KeyValues("StoreUndo"), NULL, 0);

		panel->RequestFocus();
	}

	// trigger a drop down menu to create new controls
	else if (code == MOUSE_RIGHT)
	{	
		ivgui()->PostMessage (m_hBuildDialog->GetVPanel(), new KeyValues("ShowNewControlMenu"), NULL, 0);	
	}	
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void BuildGroup::MouseReleased(MouseCode code, Panel *panel)
{
	Assert(panel);

	_dragging=false;
	input()->SetMouseCapture(null);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void BuildGroup::MouseDoublePressed(MouseCode code, Panel *panel)
{
	Assert(panel);
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void BuildGroup::KeyCodeTyped(KeyCode code, Panel *panel)
{
	Assert(panel);

	int dx=0;
	int dy=0;

	bool shift = ( input()->IsKeyDown(KEY_LSHIFT) || input()->IsKeyDown(KEY_RSHIFT) );
	bool ctrl = ( input()->IsKeyDown(KEY_LCONTROL) || input()->IsKeyDown(KEY_RCONTROL) );

	switch (code)
	{
		case KEY_LEFT:
		{
			dx-=_snapX;
			break;
		}
		case KEY_RIGHT:
		{
			dx+=_snapX;
			break;
		}
		case KEY_UP:
		{
			dy-=_snapY;
			break;
		}
		case KEY_DOWN:
		{
			dy+=_snapY;
			break;
		}
		case KEY_DELETE:
		{
			// delete the panel we have selected 
			ivgui()->PostMessage (m_hBuildDialog->GetVPanel(), new KeyValues ("DeletePanel"), NULL, 0);
			break;
		}

	}

	if (ctrl)
	{
		switch (code)
		{
		case KEY_Z:
			{
				ivgui()->PostMessage(m_hBuildDialog->GetVPanel(), new KeyValues("Undo"), NULL, 0);
				break;
			}

		case KEY_C:
			{
				ivgui()->PostMessage(m_hBuildDialog->GetVPanel(), new KeyValues("Copy"), NULL, 0);
				break;
			}
		case KEY_V:
			{
				ivgui()->PostMessage(m_hBuildDialog->GetVPanel(), new KeyValues("Paste"), NULL, 0);
				break;
			}
		}
	}

	if(dx||dy)
	{
		//TODO: make this stuff actually snap

		int x,y,wide,tall;

		panel->GetBounds(x,y,wide,tall);

		if(shift)
		{
			panel->SetSize(wide+dx,tall+dy);
		}
		else
		{
			panel->SetPos(x+dx,y+dy);
		}

		ApplySnap(panel);

		panel->Repaint();
		if (panel->GetParent() != null)
		{
			panel->GetParent()->Repaint();
		}

		// update the build dialog
		if (m_hBuildDialog)
		{
			// post that it's active
			KeyValues *keyval = new KeyValues("SetActiveControl");
			keyval->SetPtr("PanelPtr", GetCurrentPanel());
			ivgui()->PostMessage(m_hBuildDialog->GetVPanel(), keyval, NULL, 0);

			// post that it's been changed
			ivgui()->PostMessage(m_hBuildDialog->GetVPanel(), new KeyValues("PanelMoved"), NULL, 0);
		}
	}

}

//-----------------------------------------------------------------------------
// Purpose: Searches for a BuildModeDialog in the hierarchy
//-----------------------------------------------------------------------------
Panel *BuildGroup::CreateBuildDialog( void )
{
	// request the panel
	Panel *buildDialog = NULL;
	KeyValues *data = new KeyValues("BuildDialog");
	data->SetPtr("BuildGroupPtr", this);
	if (m_pBuildContext->RequestInfo(data))
	{
		buildDialog = (Panel *)data->GetPtr("PanelPtr");
	}

	// initialize the build dialog if found
	if ( buildDialog )
	{
		ivgui()->PostMessage(buildDialog->GetVPanel(), new KeyValues("SetTitle", "text", "Control Editor"), NULL, 0);
		input()->ReleaseAppModalSurface();
	}

	return buildDialog;
}

//-----------------------------------------------------------------------------
// Purpose: Activates the build mode settings dialog
//-----------------------------------------------------------------------------
void BuildGroup::ActivateBuildDialog( void )
{
	// create the build mode dialog first time through
	if (!m_hBuildDialog.Get())
	{
		m_hBuildDialog = CreateBuildDialog();

		if (!m_hBuildDialog.Get())
			return;
	}

	m_hBuildDialog->SetVisible( true );

	// send a message to set the initial dialog controls info
	_currentPanel = m_pParentPanel;
	KeyValues *keyval = new KeyValues("SetActiveControl");
	keyval->SetPtr("PanelPtr", GetCurrentPanel());
	ivgui()->PostMessage(m_hBuildDialog->GetVPanel(), keyval, NULL, 0);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
HCursor BuildGroup::GetCursor(Panel *panel)
{
	Assert(panel);
	
	int x,y,wide,tall;
	input()->GetCursorPos(x,y);
	panel->ScreenToLocal(x,y);
	panel->GetSize(wide,tall);

	if(x < 2)
	{
		if(y < 4)
		{
			return _cursor_sizenwse;
		}
		else
		if(y<(tall-4))
		{
			return _cursor_sizewe;
		}
		else
		{
			return _cursor_sizenesw;
		}
	}

	return _cursor_sizeall;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void BuildGroup::ApplySnap(Panel *panel)
{
	Assert(panel);
	
	int x,y,wide,tall;
	panel->GetBounds(x,y,wide,tall);

	x=(x/_snapX)*_snapX;
	y=(y/_snapY)*_snapY;
	panel->SetPos(x,y);
	
	int xx,yy;
	xx=x+wide;
	yy=y+tall;
	
	xx=(xx/_snapX)*_snapX;
	yy=(yy/_snapY)*_snapY;
	panel->SetSize(xx-x,yy-y);
}

//-----------------------------------------------------------------------------
// Purpose:	Return the currently selected panel
//-----------------------------------------------------------------------------
Panel *BuildGroup::GetCurrentPanel()
{
	return _currentPanel;
}

//-----------------------------------------------------------------------------
// Purpose: Add panel the list of panels that are in the build group
//-----------------------------------------------------------------------------
void BuildGroup::PanelAdded(Panel *panel)
{
	Assert(panel);

	PHandle temp;
	temp = panel;
	_panelDar.AddToTail(temp);
}

//-----------------------------------------------------------------------------
// Purpose: loads the control settings from file
//-----------------------------------------------------------------------------
void BuildGroup::LoadControlSettings(const char *controlResourceName, const char *pathID)
{
	KeyValues *rDat = new KeyValues(controlResourceName);

	// load the resource data from the file
	rDat->UsesEscapeSequences( true ); // VGUI may use them in .res files
	rDat->LoadFromFile( (IBaseFileSystem*)filesystem(), controlResourceName, pathID);

	// save off the resource name
	delete [] m_pResourceName;
	m_pResourceName = new char[strlen(controlResourceName) + 1];
	strcpy(m_pResourceName, controlResourceName);

	if (pathID)
	{
		delete [] m_pResourcePathID;
		m_pResourcePathID = new char[strlen(pathID) + 1];
		strcpy(m_pResourcePathID, pathID);
	}

	// loop through the resource data sticking info into controls
	ApplySettings(rDat);

	rDat->deleteThis();
}

//-----------------------------------------------------------------------------
// Purpose: reloads the control settings from file
//-----------------------------------------------------------------------------
void BuildGroup::ReloadControlSettings()
{
	delete m_hBuildDialog.Get(); 
	m_hBuildDialog = NULL;

	// loop though objects in the current control group and remove them all
	// the 0th panel is always the contextPanel which is not deletable 
	for( int i = 1; i < _panelDar.Size(); i++ )
	{	
		if (!_panelDar[i].Get()) // this can happen if we had two of the same handle in the list
		{
			_panelDar.Remove(i);
			--i;
			continue;
		}
		
		// only delete deletable panels, as the only deletable panels
		// are the ones created using the resource file
		if ( _panelDar[i].Get()->IsBuildModeDeletable())
		{
			delete _panelDar[i].Get();
			_panelDar.Remove(i);
			--i;
		}		
	}	

	if (m_pResourceName)
	{
		LoadControlSettings(m_pResourceName, m_pResourcePathID);
	}

	_controlGroup.RemoveAll();	

	ActivateBuildDialog();	
}


//-----------------------------------------------------------------------------
// Purpose: saves control settings to file
//-----------------------------------------------------------------------------
bool BuildGroup::SaveControlSettings( void )
{
	bool bSuccess = false;
	if ( m_pResourceName )
	{
		KeyValues *rDat = new KeyValues( m_pResourceName );

		// get the data from our controls
		GetSettings( rDat );
		
		// save the data out to a file
		bSuccess = rDat->SaveToFile( (IBaseFileSystem*)filesystem(), m_pResourceName, m_pResourcePathID);
		if (!bSuccess)
		{
			MessageBox *dlg = new MessageBox("BuildMode - Error saving file", "Error: Could not save changes.  File is most likely read only.");
			dlg->DoModal();
		}

		rDat->deleteThis();
	}

	return bSuccess;
}

//-----------------------------------------------------------------------------
// Purpose: serializes settings from a resource data container
//-----------------------------------------------------------------------------
void BuildGroup::ApplySettings( KeyValues *resourceData )
{
	// loop through all the keys, applying them wherever
	for (KeyValues *controlKeys = resourceData->GetFirstSubKey(); controlKeys != NULL; controlKeys = controlKeys->GetNextKey())
	{
		bool bFound = false;

		// Skip keys that are atomic..
		if (controlKeys->GetDataType() != KeyValues::TYPE_NONE)
			continue;

		// check to see if any buildgroup panels have this name
		for ( int i = 0; i < _panelDar.Size(); i++ )
		{
			Panel *panel = _panelDar[i].Get();

			if (!panel) // this can happen if we had two of the same handle in the list
			{
				_panelDar.Remove(i);
				--i;
				continue;
			}

			Assert (panel);

			// make the control name match CASE INSENSITIVE!
			if (!stricmp(panel->GetName(), controlKeys->GetName()))
			{
				// apply the settings
				panel->ApplySettings(controlKeys);
				bFound = true;
				break;
			}
		}

		if ( !bFound )
		{
			char const *missingName = controlKeys->GetName();

			// the key was not found in the registered list, check to see if we should create it
			if ( missingName /*controlKeys->GetInt("AlwaysCreate", false)*/ )
			{
				// create the control even though it wasn't registered
				NewControl( controlKeys );
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Create a new control in the context panel
// Input:	name: class name of control to create
//			controlKeys: keyvalues of settings for the panel.
//			name OR controlKeys should be set, not both.  
//			x,y position relative to base panel
// Output: Panel *newPanel, NULL if failed to create new control.
//-----------------------------------------------------------------------------
Panel *BuildGroup::NewControl( const char *name, int x, int y)
{
	Assert (name);
	
	Panel *newPanel = NULL;
	// returns NULL on failure
	newPanel = static_cast<EditablePanel *>(m_pParentPanel)->CreateControlByName(name);
	
	if (newPanel)
	{
		// panel successfully created
		newPanel->SetParent(m_pParentPanel);	
		newPanel->SetBuildGroup(this);
		newPanel->SetPos(x, y);

		char newFieldName[255];
		GetNewFieldName(newFieldName, newPanel);
		newPanel->SetName(newFieldName);
		
		newPanel->AddActionSignalTarget(m_pParentPanel);
		newPanel->SetBuildModeEditable(true);
		newPanel->SetBuildModeDeletable(true);	
		
		// make sure it gets freed
		newPanel->SetAutoDelete(true);
	}	

	return newPanel;
}

//-----------------------------------------------------------------------------
// Purpose: Create a new control in the context panel
// Input:	controlKeys: keyvalues of settings for the panel only works when applying initial settings.
// Output:	Panel *newPanel, NULL if failed to create new control.
//-----------------------------------------------------------------------------
Panel *BuildGroup::NewControl( KeyValues *controlKeys, int x, int y)
{
	Assert (controlKeys);
	
	Panel *newPanel = NULL;
	if (controlKeys)
	{
		KeyValues *keyVal = new KeyValues("ControlFactory", "ControlName", controlKeys->GetString("ControlName"));
		m_pBuildContext->RequestInfo(keyVal);
		// returns NULL on failure
		newPanel = (Panel *)keyVal->GetPtr("PanelPtr");
		keyVal->deleteThis();
	}
	else
	{
		return NULL;
	}

	if (newPanel)
	{
		// panel successfully created
		newPanel->SetParent(m_pParentPanel);	
		newPanel->SetBuildGroup(this);
		newPanel->SetPos(x, y);

		newPanel->SetName(controlKeys->GetName()); // name before applysettings :)
		newPanel->ApplySettings(controlKeys);

		newPanel->AddActionSignalTarget(m_pParentPanel);
		newPanel->SetBuildModeEditable(true);
		newPanel->SetBuildModeDeletable(true);	
		
		// make sure it gets freed
		newPanel->SetAutoDelete(true);
	}	

	return newPanel;
}

//-----------------------------------------------------------------------------
// Purpose: Get a new unique fieldname for a new control
//-----------------------------------------------------------------------------
void BuildGroup::GetNewFieldName(char *newFieldName, Panel *newPanel)
{
	int fieldNameNumber=1;
	char defaultName [25] =  "Unnamed dialog";
	strcpy ( defaultName, newPanel->LOCAL_GetClassName() );

	while (1)
	{
		sprintf (newFieldName, "%s%d", defaultName, fieldNameNumber);
		if ( FieldNameTaken(newFieldName) == NULL)
			break;
		++fieldNameNumber;
	}	
}

//-----------------------------------------------------------------------------
// Purpose: check to see if any buildgroup panels have this fieldname
// Input  : fieldName, name to check
// Output : ptr to a panel that has the name if it is taken
//-----------------------------------------------------------------------------
Panel *BuildGroup::FieldNameTaken(const char *fieldName)
{	 	
	for ( int i = 0; i < _panelDar.Size(); i++ )
	{
		Panel *panel = _panelDar[i].Get();
		if (!strcmp(panel->GetName(), fieldName) )
		{
			return panel;
		}
	}
	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: serializes settings to a resource data container
//-----------------------------------------------------------------------------
void BuildGroup::GetSettings( KeyValues *resourceData )
{
	// loop through all the objects getting their settings
	for( int i = 0; i < _panelDar.Size(); i++ )
	{
		Panel *panel = _panelDar[i].Get();
		if (!panel)
			continue;

		bool isRuler = false;
		// do not get setting for ruler labels.
		if (_showRulers) // rulers are visible
		{
			for (int i = 0; i < 4; i++)
			{
				if (panel == _rulerNumber[i])
				{
					isRuler = true;
					break;
				}
			}
			if (isRuler)
			{
				isRuler = false;
				continue;
			}
		}

		// Don't save the setting of the buildmodedialog
		if (!strcmp(panel->GetName(), "BuildDialog"))
			continue;

		// get the keys section from the data file
		if (panel->GetName() && *panel->GetName())
		{
			KeyValues *datKey = resourceData->FindKey(panel->GetName(), true);

			// get the settings
			panel->GetSettings(datKey);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: loop though objects in the current control group and remove them all
//-----------------------------------------------------------------------------
void BuildGroup::RemoveSettings()
{
	int i;
	// loop though objects in the current control group and remove them all
	for( i = 0; i < _controlGroup.Size(); i++ )
	{		
		// only delete delatable panels
		if ( _controlGroup[i].Get()->IsBuildModeDeletable())
		{
			delete _controlGroup[i].Get();
			_controlGroup.Remove(i);
			--i;
		}		
	}
	
	// remove deleted panels from the handle list
	for( i = 0; i < _panelDar.Size(); i++ )
	{
		if ( !_panelDar[i].Get() )	
		{	
		  _panelDar.Remove(i);
		  --i;
		}
	}

	_currentPanel = m_pBuildContext;
	_currentPanel->InvalidateLayout();
    m_pBuildContext->Repaint();
}

//-----------------------------------------------------------------------------
// Purpose: sets the panel from which the build group gets all it's object creation info
//-----------------------------------------------------------------------------
void BuildGroup::SetContextPanel(Panel *contextPanel)
{
	m_pBuildContext = contextPanel;
}

//-----------------------------------------------------------------------------
// Purpose: gets the panel from which the build group gets all it's object creation info
//-----------------------------------------------------------------------------
Panel *BuildGroup::GetContextPanel() 
{
	return m_pBuildContext;
}

//-----------------------------------------------------------------------------
// Purpose: get the list of panels in the buildgroup
//-----------------------------------------------------------------------------
CUtlVector<PHandle> *BuildGroup::GetPanelList() 
{
	return &_panelDar;
}

