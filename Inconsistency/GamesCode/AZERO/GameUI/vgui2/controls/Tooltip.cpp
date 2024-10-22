//=========== (C) Copyright 2000 Valve, L.L.C. All rights reserved. ===========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Purpose: 
// This class is a message box that has two buttons, ok and cancel instead of
// just the ok button of a message box. We use a message box class for the ok button
// and implement another button here.
//=============================================================================

#include <math.h>
#define PROTECTED_THINGS_DISABLE

#include <vgui/IInput.h>
#include <vgui/ISystem.h>
#include <vgui/ISurface.h>
#include <vgui/IScheme.h>
#include <vgui/IVGui.h>
#include <vgui/IPanel.h>

#include <vgui_controls/Tooltip.h>
#include <vgui_controls/TextEntry.h>
#include <vgui_controls/Controls.h>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

static TextEntry *s_pTooltipWindow = NULL;
static int s_iTooltipWindowCount = 0;

int Tooltip::_delay;
int Tooltip::_tooltipDelay;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
Tooltip::Tooltip(Panel *parent, const char *text) 
{
	if (!s_pTooltipWindow)
	{
		s_pTooltipWindow = new TextEntry(NULL, "tooltip");
	}
	s_iTooltipWindowCount++;

	// this line prevents the main window from losing focus
	// when a tooltip pops up
	s_pTooltipWindow->MakePopup(false,true);
	
	SetText(text);
	s_pTooltipWindow->SetText(m_Text.Base());
	s_pTooltipWindow->SetEditable(false);
	s_pTooltipWindow->SetMultiline(true);
	s_pTooltipWindow->SetVisible(false);

	_displayOnOneLine = false;
	_makeVisible = false;

	_tooltipDelay = 500; // default delay for opening tooltips
}


//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
Tooltip::~Tooltip()
{
	if (--s_iTooltipWindowCount < 1)
	{
		delete s_pTooltipWindow;
		s_pTooltipWindow = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Get the tooltip text
// Output: text- the tooltip message
//-----------------------------------------------------------------------------
const char *Tooltip::GetText()
{
	return m_Text.Base();
}

//-----------------------------------------------------------------------------
// Purpose: Set the tooltip text
// Input: text- the tooltip message
//-----------------------------------------------------------------------------
void Tooltip::SetText(const char *text)
{
	if (m_Text.Size() > 0)
		m_Text.RemoveAll();

	for (unsigned int i = 0; i < strlen(text); i++)
	{
		m_Text.AddToTail(text[i]);
	}
	m_Text.AddToTail('\0');
	
	if( s_pTooltipWindow)
	{
		s_pTooltipWindow->SetText(m_Text.Base());
	}
}

void Tooltip::ApplySchemeSettings(IScheme *pScheme)
{
	s_pTooltipWindow->SetFont(pScheme->GetFont( "DefaultSmall", s_pTooltipWindow->IsProportional() ));
}

//-----------------------------------------------------------------------------
// Purpose: Reset the tooltip delay timer
//-----------------------------------------------------------------------------
void Tooltip::ResetDelay()
{
	_delay = system()->GetTimeMillis() + _tooltipDelay;
}

//-----------------------------------------------------------------------------
// Purpose: Set the tooltip delay before a tooltip comes up.
//-----------------------------------------------------------------------------
void Tooltip::SetTooltipDelay( int tooltipDelay )
{
	_tooltipDelay = tooltipDelay;
}

//-----------------------------------------------------------------------------
// Purpose: Get the tooltip delay before a tooltip comes up.
//-----------------------------------------------------------------------------
int Tooltip::GetTooltipDelay()
{
	return _tooltipDelay;
}

//-----------------------------------------------------------------------------
// Purpose: Display the tooltip
//-----------------------------------------------------------------------------
void Tooltip::ShowTooltip(Panel *currentPanel)
{
	s_pTooltipWindow->SetText(m_Text.Base());
	ivgui()->AddTickSignal(currentPanel->GetVPanel(), 0);

//	s_pTooltipWindow->SetVisible(true);

	s_pTooltipWindow->SetParent(currentPanel);
	_makeVisible = true;

	PerformLayout();
}

//-----------------------------------------------------------------------------
// Purpose: Display the tooltip
//-----------------------------------------------------------------------------
void Tooltip::PerformLayout()
{
//	ivgui()->DPrintf2("Perform Layout\n");

	if (!_makeVisible)
		return;
	if (_delay > system()->GetTimeMillis())
	{
		return;	
	}
	else
	{
		// get the panel with current focus
	
		// this line reports the wrong panel with focus
		//	Panel *currentFocus = ipanel()->Client(input()->GetFocus())->GetPanel();
		//Panel *currentFocus = s_pTooltipWindow->GetParent();

		s_pTooltipWindow->SetVisible(true);

	//	s_pTooltipWindow->MoveToFront();
	//	currentFocus->MoveToFront();
	
	}

	IScheme *pScheme = scheme()->GetIScheme( s_pTooltipWindow->GetScheme() );

	s_pTooltipWindow->SetBgColor(s_pTooltipWindow->GetSchemeColor("SelectionBG", s_pTooltipWindow->GetBgColor(), pScheme));
	s_pTooltipWindow->SetFgColor(s_pTooltipWindow->GetSchemeColor("BorderSelection", s_pTooltipWindow->GetFgColor(), pScheme));
	s_pTooltipWindow->SetBorder(pScheme->GetBorder("ToolTipBorder"));

	// get cursor position
	int cursorX, cursorY;
	input()->GetCursorPos(cursorX, cursorY);
	
	// relayout the textwindow immediately so that we know it's size
	//m_pTextEntry->InvalidateLayout(true);
	SizeTextWindow();
	int menuWide, menuTall;
	s_pTooltipWindow->GetSize(menuWide, menuTall);
	
	// work out where the cursor is and therefore the best place to put the menu
	int wide, tall;
	surface()->GetScreenSize(wide, tall);
	
	if (wide - menuWide > cursorX)
	{
		cursorY += 20;
		// menu hanging right
		if (tall - menuTall > cursorY)
		{
			// menu hanging down
			s_pTooltipWindow->SetPos(cursorX, cursorY);
		}
		else
		{
			// menu hanging up
			s_pTooltipWindow->SetPos(cursorX, cursorY - menuTall - 20);
		}
	}
	else
	{
		// menu hanging left
		if (tall - menuTall > cursorY)
		{
			// menu hanging down
			s_pTooltipWindow->SetPos(cursorX - menuWide, cursorY);
		}
		else
		{
			// menu hanging up
			s_pTooltipWindow->SetPos(cursorX - menuWide, cursorY - menuTall - 20 );
		}
	}	
}

//-----------------------------------------------------------------------------
// Purpose: Size the text window so all the text fits inside it.
//-----------------------------------------------------------------------------
void Tooltip::SizeTextWindow()
{
	if (_displayOnOneLine)
	{
		// We want the tool tip to be one line
		s_pTooltipWindow->SetMultiline(false);
		s_pTooltipWindow->SetToFullWidth();
	}
	else
	{
		// We want the tool tip to be one line
		s_pTooltipWindow->SetMultiline(false);
		s_pTooltipWindow->SetToFullWidth();
		int wide, tall;
		s_pTooltipWindow->GetSize( wide, tall );
		//MODDD - casted sqrt innards to double
		double newWide = sqrt( (double)((2/1) * wide * tall) );
		double newTall = (1/2) * newWide;
		s_pTooltipWindow->SetMultiline(true);
		s_pTooltipWindow->SetSize((int)newWide, (int)newTall );
		s_pTooltipWindow->SetToFullHeight();
		
		s_pTooltipWindow->GetSize( wide, tall );
		
		if (( wide < 100 ) && ( s_pTooltipWindow->GetNumLines() == 2) ) 
		{
			s_pTooltipWindow->SetMultiline(false);
			s_pTooltipWindow->SetToFullWidth();	
		}
		else
		{
			
			while ( (float)wide/(float)tall < 2 )
			{
				s_pTooltipWindow->SetSize( wide + 1, tall );
				s_pTooltipWindow->SetToFullHeight();
				s_pTooltipWindow->GetSize( wide, tall );
			}
		}
		s_pTooltipWindow->GetSize( wide, tall );
	//	ivgui()->DPrintf("End Ratio: %f\n", (float)wide/(float)tall);		
	}
}


//-----------------------------------------------------------------------------
// Purpose: Set the tool tip to display one one line only (true) or
//	on multiple lines.
// Default is multiple lines.
//-----------------------------------------------------------------------------
void Tooltip::SetDisplayMode(bool displayOnOneLine)
{
	_displayOnOneLine = displayOnOneLine;
}

//-----------------------------------------------------------------------------
// Purpose: Display the tooltip
//-----------------------------------------------------------------------------
void Tooltip::HideTooltip()
{
	s_pTooltipWindow->SetVisible(false);
	_makeVisible = false;
}




