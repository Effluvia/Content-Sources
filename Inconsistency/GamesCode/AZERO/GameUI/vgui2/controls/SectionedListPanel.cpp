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

#include <vgui/IInput.h>
#include <vgui/IPanel.h>
#include <vgui/ILocalize.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <KeyValues.h>
#include <vgui/MouseCode.h>

#include <vgui_controls/SectionedListPanel.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/Controls.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/ScrollBar.h>
#include <vgui_controls/TextImage.h>
#include <vgui_controls/ImageList.h>

#include "UtlVector.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

enum
{
	BUTTON_HEIGHT_DEFAULT = 20,
	BUTTON_HEIGHT_SPACER = 7,
	SECTION_GAP = 8, 
	COLUMN_DATA_INDENT = 6,
	COLUMN_DATA_GAP = 2,
};

namespace vgui
{

//-----------------------------------------------------------------------------
// Purpose: header label that seperates and names each section
//-----------------------------------------------------------------------------
class CSectionHeader : public Label
{
public:
	CSectionHeader(SectionedListPanel *parent, const char *name, int sectionID) : Label(parent, name, "")
	{
		m_pListPanel = parent;
		m_iSectionID = sectionID;
		SetTextImageIndex(-1);
		ClearImages();
		SetPaintBackgroundEnabled( false );
	}

	CSectionHeader(SectionedListPanel *parent, const wchar_t *name, int sectionID) : Label(parent, "SectionHeader", "")
	{
		SetText(name);	
		SetVisible(false);
		m_pListPanel = parent;
		m_iSectionID = sectionID;
		SetTextImageIndex(-1);
		ClearImages();
	}

	void ApplySchemeSettings(IScheme *pScheme)
	{
		BaseClass::ApplySchemeSettings(pScheme);

		SetFgColor(GetSchemeColor("SectionTextColor", pScheme));
		m_SectionDividerColor = GetSchemeColor("SectionDividerColor", pScheme);
		SetBgColor(GetSchemeColor("BuddyListBgColor", GetBgColor(), pScheme));
		SetFont(pScheme->GetFont("DefaultVerySmall", IsProportional()));
	}

	void Paint()
	{
		BaseClass::Paint();

		int x, y, wide, tall;
		GetBounds(x, y, wide, tall);

		y = (tall - 2);	// draw the line under the panel

		surface()->DrawSetColor(m_SectionDividerColor);
		surface()->DrawFilledRect(1, y, GetWide() - 2, y + 1);
	}

	void SetColor(Color col)
	{
		m_SectionDividerColor = col;
		SetFgColor(col);
	}

	void PerformLayout()
	{
		BaseClass::PerformLayout();

		// set up the text in the header
		int colCount = m_pListPanel->GetColumnCountBySection(m_iSectionID);
		if (colCount != GetImageCount())
		{
			// rebuild the image list
			for (int i = 0; i < colCount; i++)
			{
				int columnFlags = m_pListPanel->GetColumnFlagsBySection(m_iSectionID, i);
				IImage *image = NULL;
				if (columnFlags & SectionedListPanel::HEADER_IMAGE)
				{
					//!! need some kind of image reference
					image = NULL;
				}
				else 
				{
					TextImage *textImage = new TextImage("");
					textImage->SetFont(GetFont());
					textImage->SetColor(GetFgColor());
					image = textImage;
				}

				SetImageAtIndex(i, image, 0);
			}
		}

		int xpos = 0;
		for (int i = 0; i < colCount; i++)
		{
			int columnFlags = m_pListPanel->GetColumnFlagsBySection(m_iSectionID, i);
			int maxWidth = m_pListPanel->GetColumnWidthBySection(m_iSectionID, i);

			IImage *image = GetImageAtIndex(i);
			if (!image)
			{
				// even with no column image, we need to move the xpos
			//	xpos += maxWidth;
				continue;
			}

			// set the image position within the label
			int contentWide, wide, tall;
			image->GetContentSize(wide, tall);
			contentWide = wide;

			// take up as much space as allowed
			for (int j=i+1; j < colCount; j++)
			{
				if (GetImageAtIndex(j) == NULL)
				{
					maxWidth += m_pListPanel->GetColumnWidthBySection(m_iSectionID, j);
				}
			}

			if (maxWidth >= 0)
			{
				wide = maxWidth;
			}

			if (columnFlags & SectionedListPanel::COLUMN_RIGHT)
			{
				SetImageBounds(i, xpos + wide - contentWide, xpos + contentWide - COLUMN_DATA_GAP);
			}
			else
			{
				SetImageBounds(i, xpos, xpos + wide - COLUMN_DATA_GAP);
			}
			xpos += wide;

			if (!(columnFlags & SectionedListPanel::HEADER_IMAGE))
			{
				Assert(dynamic_cast<TextImage *>(image) != NULL);
				TextImage *textImage = (TextImage *)image;
				textImage->SetText(m_pListPanel->GetColumnTextBySection(m_iSectionID, i));
                textImage->ResizeImageToContent();
			}
		}
	}

private:
	int m_iSectionID;
	Color m_SectionDividerColor;
	SectionedListPanel *m_pListPanel;

	typedef vgui::Label BaseClass;
};

//-----------------------------------------------------------------------------
// Purpose: Individual items in the list
//-----------------------------------------------------------------------------
class CItemButton : public Label
{
public:
	CItemButton(SectionedListPanel *parent, int itemID) : Label(parent, NULL, "< item >")
	{
		m_pListPanel = parent;
		m_iID = itemID;
		m_pData = NULL;
		m_bSelected = false;
		m_bOverrideColors = false;
		m_iSectionID = -1;
		SetPaintBackgroundEnabled( false );
		SetTextImageIndex(-1);
		ClearImages();
	}

	~CItemButton()
	{
		// free all the keyvalues
		if (m_pData)
		{
			m_pData->deleteThis();
		}

		// clear any section data
		SetSectionID(-1);
	}

	int GetID()
	{
		return m_iID;
	}

	int GetSectionID()
	{
		return m_iSectionID;
	}

	void SetSectionID(int sectionID)
	{
		if (sectionID != m_iSectionID)
		{
			// free any existing textimage list 
			ClearImages();
			// delete any images we've created
			for (int i = 0; i < m_TextImages.Count(); i++)
			{
				delete m_TextImages[i];
			}
			m_TextImages.RemoveAll();
			// mark the list as needing rebuilding
			InvalidateLayout();
		}
		m_iSectionID = sectionID;
	}

	void SetData(KeyValues *data)
	{
		if (m_pData)
		{
			m_pData->deleteThis();
		}

		m_pData = data->MakeCopy();
		InvalidateLayout();
	}

	KeyValues *GetData()
	{
		return m_pData;
	}

	virtual void PerformLayout()
	{
		// get our button text
		int colCount = m_pListPanel->GetColumnCountBySection(m_iSectionID);
		if (!m_pData || colCount < 1)
		{
			SetText("< unset >");
		}
		else
		{
			if (colCount != GetImageCount())
			{
				// rebuild the image list
				for (int i = 0; i < colCount; i++)
				{
					int columnFlags = m_pListPanel->GetColumnFlagsBySection(m_iSectionID, i);
					if (!(columnFlags & SectionedListPanel::COLUMN_IMAGE))
					{
						TextImage *image = new TextImage("");
						m_TextImages.AddToTail(image);
						image->SetFont( GetFont() );
						SetImageAtIndex(i, image, 0);
					}				
				}

				{for ( int i = GetImageCount(); i < colCount; i++ ) // make sure we have enough image slots
				{
					AddImage( NULL, 0 );
				}}
			}

			// set the text for each column
			int xpos = 0;
			for (int i = 0; i < colCount; i++)
			{
				const char *keyname = m_pListPanel->GetColumnNameBySection(m_iSectionID, i);

				int columnFlags = m_pListPanel->GetColumnFlagsBySection(m_iSectionID, i);
				int maxWidth = m_pListPanel->GetColumnWidthBySection(m_iSectionID, i);
			
				IImage *image = NULL;
				if (columnFlags & SectionedListPanel::COLUMN_IMAGE)
				{
					// lookup which image is being referred to
					if (m_pListPanel->m_pImageList)
					{
						int imageIndex = m_pData->GetInt(keyname, 0);
						if (m_pListPanel->m_pImageList->IsValidIndex(imageIndex))
						{
							// 0 is always the blank image
							if (imageIndex > 0)
							{
								image = m_pListPanel->m_pImageList->GetImage(imageIndex);
								SetImageAtIndex(i, image, 0);
							}
						}
						else
						{
							// this is mildly valid (CGamesList hits it because of the way it uses the image indices)
							// Assert(!("Image index out of range for ImageList in SectionedListPanel"));
						}
					}
					else
					{
						Assert(!("Images columns used in SectionedListPanel with no ImageList set"));
					}
				}
				else
				{
					// must be text
					Assert(dynamic_cast<TextImage *>(GetImageAtIndex(i)) != NULL);
					TextImage *textImage = (TextImage *)GetImageAtIndex(i);
					if (textImage)
					{
						textImage->SetText(m_pData->GetString(keyname, ""));
						textImage->ResizeImageToContent();

						// set the text color based on the selection state - if one of the children of the SectionedListPanel has focus, then 'we have focus' if we're selected
						VPANEL focus = input()->GetFocus();
						if ( !m_bOverrideColors )
						{
							if (IsSelected() && (HasFocus() || (focus && ipanel()->HasParent(focus, GetParent()->GetVPanel()))))
							{
								textImage->SetColor(m_ArmedFgColor2);
							}
							else if (columnFlags & SectionedListPanel::COLUMN_BRIGHT)
							{
								textImage->SetColor(m_ArmedFgColor1);
							}
							else
							{
								textImage->SetColor(m_FgColor2);
							}
						}
						else
						{
							textImage->SetColor(GetFgColor());					
						}
					}
					image = textImage;
				}

				// set the image position within the label
				int imageWide = 0, tall = 0;
				int wide;
				if (image)
				{
					image->GetContentSize(imageWide, tall);
				}
				if (maxWidth >= 0)
				{
					wide = maxWidth;
				}
				else
				{
					wide = imageWide;
				}

				if (i == 0 && !(columnFlags & SectionedListPanel::COLUMN_IMAGE))
				{
					// first column has an extra indent
					SetImageBounds(i, xpos + COLUMN_DATA_INDENT, xpos + wide - (COLUMN_DATA_INDENT + COLUMN_DATA_GAP));
				}
				else
				{
					if (columnFlags & SectionedListPanel::COLUMN_CENTER)
					{
						SetImageBounds(i, xpos + imageWide / 2, xpos + wide - COLUMN_DATA_GAP - imageWide / 2);
					}
					else if (columnFlags & SectionedListPanel::COLUMN_RIGHT)
					{
						SetImageBounds(i, xpos + wide - imageWide, xpos + wide - COLUMN_DATA_GAP);
					}
					else
					{
						SetImageBounds(i, xpos, xpos + wide - COLUMN_DATA_GAP);
					}
				}
				xpos += wide;
			}
		}

		BaseClass::PerformLayout();
	}

	virtual void ApplySchemeSettings(IScheme *pScheme)
	{
		BaseClass::ApplySchemeSettings(pScheme);

		//!! need to change these to be more generic colors
		m_ArmedFgColor1 = GetSchemeColor("BuddyButton/ArmedFgColor1", pScheme);
		m_ArmedFgColor2 = GetSchemeColor("BuddyButton/ArmedFgColor2", pScheme);
		m_ArmedBgColor = GetSchemeColor("BuddyButton/ArmedBgColor", pScheme);

		m_FgColor1 = GetSchemeColor("BuddyButton/FgColor1", pScheme);
		m_FgColor2 = GetSchemeColor("BuddyButton/FgColor2", pScheme);

		m_BgColor = GetSchemeColor("BuddyListBgColor", GetBgColor(), pScheme);
		m_SelectionBG2Color = GetSchemeColor("SelectionBG2", pScheme);
	}

	virtual void PaintBackground()
	{
		int wide, tall;
		GetSize(wide, tall);

		if (IsSelected() && !m_pListPanel->IsInEditMode())
		{
            VPANEL focus = input()->GetFocus();
            // if one of the children of the SectionedListPanel has focus, then 'we have focus' if we're selected
            if (HasFocus() || (focus && ipanel()->HasParent(focus, GetParent()->GetVPanel())))
            {
			    surface()->DrawSetColor(m_ArmedBgColor);
            }
            else
            {
			    surface()->DrawSetColor(m_SelectionBG2Color);
            }
		}
		else
		{
			surface()->DrawSetColor(GetBgColor());
		}
		surface()->DrawFilledRect(0, 0, wide, tall);
	}

	virtual void OnMousePressed(MouseCode code)
	{
		if (code == MOUSE_LEFT)
		{
			m_pListPanel->PostActionSignal(new KeyValues("ItemLeftClick", "itemID", m_iID));
		}
		if (code == MOUSE_RIGHT)
		{
			KeyValues *msg = new KeyValues("ItemContextMenu", "itemID", m_iID);
			msg->SetPtr("SubPanel", this);
			m_pListPanel->PostActionSignal(msg);
		}

		m_pListPanel->SetSelectedItem(this);
	}

	void SetSelected(bool state)
	{
		if (m_bSelected != state)
		{
            if (state)
            {
                RequestFocus();
            }
			m_bSelected = state;
			SetPaintBackgroundEnabled( state );
			InvalidateLayout();
			Repaint();
		}
	}

	bool IsSelected()
	{
		return m_bSelected;
	}

    virtual void OnSetFocus()
    {
        InvalidateLayout(); // force the layout to be redone so we can change text color according to focus
        BaseClass::OnSetFocus();
    }

    virtual void OnKillFocus()
    {
        InvalidateLayout(); // force the layout to be redone so we can change text color according to focus
        BaseClass::OnSetFocus();
    }

	virtual void OnMouseDoublePressed(MouseCode code)
	{
		if (code == MOUSE_LEFT)
		{
			m_pListPanel->PostActionSignal(new KeyValues("ItemDoubleLeftClick", "itemID", m_iID));

			// post up an enter key being hit
			m_pListPanel->OnKeyCodeTyped(KEY_ENTER);
		}
		else
		{
			OnMousePressed(code);
		}

		m_pListPanel->SetSelectedItem(this);
	}

	void GetCellBounds(int column, int &xpos, int &columnWide)
	{
		xpos = 0, columnWide = 0;
		int colCount = m_pListPanel->GetColumnCountBySection(m_iSectionID);
		for (int i = 0; i < colCount; i++)
		{
			int maxWidth = m_pListPanel->GetColumnWidthBySection(m_iSectionID, i);

			IImage *image = GetImageAtIndex(i);
			if (!image)
				continue;

			// set the image position within the label
			int wide, tall;
			image->GetContentSize(wide, tall);
			if (maxWidth >= 0)
			{
				wide = maxWidth;
			}

			if (i == column)
			{
				// found the cell size, bail
				columnWide = wide;
				return;
			}

			xpos += wide;
		}
	}

	virtual void SetOverrideColors( bool state )
	{
		m_bOverrideColors = state;
	}

private:
	SectionedListPanel *m_pListPanel;
	int m_iID;
	int m_iSectionID;
	KeyValues *m_pData;
	Color m_FgColor1;
	Color m_FgColor2;
	Color m_BgColor;
	Color m_ArmedFgColor1;
	Color m_ArmedFgColor2;
	Color m_ArmedBgColor;
	Color m_SelectionBG2Color;
	CUtlVector<vgui::TextImage *> m_TextImages;

	bool m_bSelected;
	bool m_bOverrideColors;

	typedef vgui::Label BaseClass;
};

}; // namespace vgui

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
SectionedListPanel::SectionedListPanel(vgui::Panel *parent, const char *name) : BaseClass(parent, name)
{
	m_pScrollBar = new ScrollBar(this, "SectionedScrollBar", true);
	m_pScrollBar->SetVisible(false);
	m_pScrollBar->AddActionSignalTarget(this);

	m_iEditModeItemID = 0;
	m_iEditModeColumn = 0;
	m_bSortNeeded = false;
	m_bVerticalScrollbarEnabled = true;
	m_iLineSpacing = 20;

	m_pImageList = NULL;
	m_bDeleteImageListWhenDone = false;

	if (IsProportional())
	{
		int width, height;
		int sw,sh;
		surface()->GetProportionalBase(width, height);
		surface()->GetScreenSize(sw, sh);
		
		m_iScrollbarSize = static_cast<int>( static_cast<float>( SCROLLBAR_SIZE )*( static_cast<float>( sw )/ static_cast<float>( width )));
	}
	else
	{
		m_iScrollbarSize = SCROLLBAR_SIZE;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
SectionedListPanel::~SectionedListPanel()
{
}

//-----------------------------------------------------------------------------
// Purpose: Sorts the list
//-----------------------------------------------------------------------------
void SectionedListPanel::ReSortList()
{
    m_SortedItems.RemoveAll();

	int sectionStart = 0;
	// layout the buttons
	for (int sectionIndex = 0; sectionIndex < m_Sections.Size(); sectionIndex++)
	{
		section_t &section = m_Sections[sectionIndex];
		sectionStart = m_SortedItems.Count();

		// find all the items in this section
		for( int i = m_Items.Head(); i != m_Items.InvalidIndex(); i = m_Items.Next( i ) )
		{
			if (m_Items[i]->GetSectionID() == m_Sections[sectionIndex].m_iID)
			{
				// insert the items sorted
				if (section.m_pSortFunc)
				{
					int insertionPoint = sectionStart;
					for (;insertionPoint < m_SortedItems.Count(); insertionPoint++)
					{
						if (section.m_pSortFunc(this, i, m_SortedItems[insertionPoint]->GetID()))
							break;
					}

					if (insertionPoint == m_SortedItems.Count())
					{
						m_SortedItems.AddToTail(m_Items[i]);
					}
					else
					{
						m_SortedItems.InsertBefore(insertionPoint, m_Items[i]);
					}
				}
				else
				{
					// just add to the end
					m_SortedItems.AddToTail(m_Items[i]);
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: iterates through and sets up the position of all the sections and items
//-----------------------------------------------------------------------------
void SectionedListPanel::PerformLayout()
{
	// lazy resort the list
	if (m_bSortNeeded)
	{
		ReSortList();
		m_bSortNeeded = false;
	}

	BaseClass::PerformLayout();
	
	LayoutPanels(m_iContentHeight);

	int cx, cy, cwide, ctall;
	GetBounds(cx, cy, cwide, ctall);
	if (m_iContentHeight > ctall && m_bVerticalScrollbarEnabled)
	{
		m_pScrollBar->SetVisible(true);
		m_pScrollBar->MoveToFront();

		m_pScrollBar->SetPos(cwide - m_iScrollbarSize - 2, 0);
		m_pScrollBar->SetSize(m_iScrollbarSize, ctall - 2);

		m_pScrollBar->SetRangeWindow(ctall);

		m_pScrollBar->SetRange(0, m_iContentHeight);
		m_pScrollBar->InvalidateLayout();
		m_pScrollBar->Repaint();

		// since we're just about to make the scrollbar visible, we need to re-layout
		// the buttons since they depend on the scrollbar size
		LayoutPanels(m_iContentHeight);
	}
	else
	{
		m_pScrollBar->SetValue(0);
		m_pScrollBar->SetVisible(false);
	}
}

//-----------------------------------------------------------------------------
// Purpose: lays out the sections and rows in the panel
//-----------------------------------------------------------------------------
void SectionedListPanel::LayoutPanels(int &contentTall)
{
	int i;
	int tall = GetSectionTall();
	int x = 5, wide = GetWide() - 10;
	int y = 5;
	
	if (m_pScrollBar->IsVisible())
	{
		y -= m_pScrollBar->GetValue();
		wide -= m_pScrollBar->GetWide();
	}
    
    int iStart = -1;
    int iEnd = -1;

	// layout the buttons
	for (int sectionIndex = 0; sectionIndex < m_Sections.Size(); sectionIndex++)
	{
		section_t &section = m_Sections[sectionIndex];

		iStart = -1;
		iEnd = -1;
        for (i = 0; i < m_SortedItems.Count(); i++)
        {
            if (m_SortedItems[i]->GetSectionID() == m_Sections[sectionIndex].m_iID)
            {
                if (iStart == -1)
                    iStart = i;
                iEnd = i;
            }
        }

		// don't draw this section at all if their are no item in it
		if (iStart == -1 && !section.m_bAlwaysVisible)
		{
			section.m_pHeader->SetVisible(false);
			continue;
		}

		// draw the header
		section.m_pHeader->SetBounds(x, y, wide, tall);
		section.m_pHeader->SetVisible(true);
		y += tall;

		if (iStart == -1 && section.m_bAlwaysVisible)
		{
		}
		else
		{
			// arrange all the items in this section underneath
			for (i = iStart; i <= iEnd; i++)
			{
				CItemButton *item = m_SortedItems[i]; //items[i];
				item->SetBounds(x, y, wide, m_iLineSpacing);
				
				// setup edit mode
				if (m_hEditModePanel.Get() && m_iEditModeItemID == item->GetID())
				{
					int cx, cwide;
					item->GetCellBounds(1, cx, cwide);
					m_hEditModePanel->SetBounds(cx, y, cwide, tall);
				}

				y += m_iLineSpacing;
			}
		}

		// add in a little boundry at the bottom
		y += SECTION_GAP;
	}

	// calculate height
	contentTall = y;
	if (m_pScrollBar->IsVisible())
	{
		contentTall += m_pScrollBar->GetValue();
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void SectionedListPanel::ScrollToItem(int iItem)
{
	int y = 5; //,tall = BUTTON_HEIGHT;
	int tall = GetSectionTall();

	int itemY = 0;

	// layout the buttons
	for (int sectionIndex = 0; sectionIndex < m_Sections.Size(); sectionIndex++)
	{
        int itemsCount = 0;
		FOR_EACH_LL( m_Items, i )
		{
			if (m_Items[i]->GetSectionID() == m_Sections[sectionIndex].m_iID)
			{
                itemsCount++;

                // if there's at least one item, then insert the header
                if (itemsCount == 1)
                    y += tall;

				if (i == iItem)
					itemY = y;

                y += tall;
			}
		}

		// don't draw this section at all if their are no item in it
		if (itemsCount < 1)
			continue;

		// add in a little boundry at the bottom
		y += SECTION_GAP;
	}

	int cx, cy, cwide, ctall;
	GetBounds(cx, cy, cwide, ctall);
	if (y > ctall)
	{
        int nCurrentValue = m_pScrollBar->GetValue();
        if (itemY < nCurrentValue)
            m_pScrollBar->SetValue(itemY);
        else if (itemY > nCurrentValue+ctall-tall)
            m_pScrollBar->SetValue(itemY - ctall + tall);
	}
	else
	{
		m_pScrollBar->SetValue(0);
	}
	Repaint();
}


//-----------------------------------------------------------------------------
// Purpose: sets background color & border
//-----------------------------------------------------------------------------
void SectionedListPanel::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	SetBgColor(GetSchemeColor("BuddyListBgColor", GetBgColor(), pScheme));
	SetBorder(pScheme->GetBorder("ButtonDepressedBorder"));
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void SectionedListPanel::ApplySettings(KeyValues *inResourceData)
{
	BaseClass::ApplySettings(inResourceData);
	m_iLineSpacing = inResourceData->GetInt("linespacing", 0);
	if (!m_iLineSpacing)
	{
		m_iLineSpacing = 20;
	}
	if (IsProportional())
	{
		m_iLineSpacing = scheme()->GetProportionalScaledValue(m_iLineSpacing);
	}
}

//-----------------------------------------------------------------------------
// Purpose: passes on proportional state to children
//-----------------------------------------------------------------------------
void SectionedListPanel::SetProportional(bool state)
{
	BaseClass::SetProportional(state);

	// now setup the section headers and items
	int i;
	for (i = 0; i < m_Sections.Count(); i++)
	{
		m_Sections[i].m_pHeader->SetProportional(state);
	}	
	FOR_EACH_LL( m_Items, j )
	{
		m_Items[j]->SetProportional(state);
	}	
}

//-----------------------------------------------------------------------------
// Purpose: sets whether or not the vertical scrollbar should ever be displayed
//-----------------------------------------------------------------------------
void SectionedListPanel::SetVerticalScrollbar(bool state)
{
	m_bVerticalScrollbarEnabled = state;
}

//-----------------------------------------------------------------------------
// Purpose: adds a new section
//-----------------------------------------------------------------------------
void SectionedListPanel::AddSection(int sectionID, const char *name, SectionSortFunc_t sortFunc)
{
	CSectionHeader *header = SETUP_PANEL(new CSectionHeader(this, name, sectionID));
	AddSectionHelper(sectionID, header, sortFunc);
}

//-----------------------------------------------------------------------------
// Purpose: adds a new section
//-----------------------------------------------------------------------------
void SectionedListPanel::AddSection(int sectionID, const wchar_t *name, SectionSortFunc_t sortFunc)
{
	CSectionHeader *header = SETUP_PANEL(new CSectionHeader(this, name, sectionID));
	AddSectionHelper(sectionID, header, sortFunc);
}

//-----------------------------------------------------------------------------
// Purpose: helper function for AddSection
//-----------------------------------------------------------------------------
void SectionedListPanel::AddSectionHelper(int sectionID, CSectionHeader *header, SectionSortFunc_t sortFunc)
{
	int index = m_Sections.AddToTail();
	m_Sections[index].m_iID = sectionID;
	m_Sections[index].m_pHeader = header;
	m_Sections[index].m_pSortFunc = sortFunc;
	m_Sections[index].m_bAlwaysVisible = false;
}

//-----------------------------------------------------------------------------
// Purpose: removes all the sections from the current panel
//-----------------------------------------------------------------------------
void SectionedListPanel::RemoveAllSections()
{
	for (int i = 0; i < m_Sections.Count(); i++)
	{
		if (!m_Sections.IsValidIndex(i))
			continue;

		m_Sections[i].m_pHeader->SetVisible(false);
		m_Sections[i].m_pHeader->MarkForDeletion();
	}

	m_Sections.RemoveAll();
	m_Sections.Purge();
    m_SortedItems.RemoveAll();

	InvalidateLayout();
    ReSortList();
}

//-----------------------------------------------------------------------------
// Purpose: adds a new column to a section
//-----------------------------------------------------------------------------
bool SectionedListPanel::AddColumnToSection(int sectionID, const char *columnName, const char *columnText, int columnFlags, int width)
{
	wchar_t wtext[64];
	wchar_t *pwtext = localize()->Find(columnText);
	if (!pwtext)
	{
		localize()->ConvertANSIToUnicode(columnText, wtext, sizeof(wtext));
		pwtext = wtext;
	}
	return AddColumnToSection(sectionID, columnName, pwtext, columnFlags, width);
}

//-----------------------------------------------------------------------------
// Purpose: as above but with wchar_t's
//-----------------------------------------------------------------------------
bool SectionedListPanel::AddColumnToSection(int sectionID, const char *columnName, const wchar_t *columnText, int columnFlags, int width)
{
	int index = FindSectionIndexByID(sectionID);
	if (index < 0)
		return false;
	section_t &section = m_Sections[index];

	// add the new column to the sections' list
	index = section.m_Columns.AddToTail();
	column_t &column = section.m_Columns[index];

	strncpy(column.m_szColumnName, columnName, sizeof(column.m_szColumnName));
	column.m_szColumnName[sizeof(column.m_szColumnName) - 1] = 0;
	wcsncpy(column.m_szColumnText, columnText,  sizeof(column.m_szColumnText) / sizeof(wchar_t));
	column.m_szColumnText[sizeof(column.m_szColumnText) / sizeof(wchar_t) - 1] = 0;
	column.m_iColumnFlags = columnFlags;
	column.m_iWidth = width;
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: modifies the text in an existing column
//-----------------------------------------------------------------------------
bool SectionedListPanel::ModifyColumn(int sectionID, const char *columnName, const wchar_t *columnText)
{
	int index = FindSectionIndexByID(sectionID);
	if (index < 0)
		return false;
	section_t &section = m_Sections[index];

	// find the specified column
	int columnIndex;
	for (columnIndex = 0; columnIndex < section.m_Columns.Count(); columnIndex++)
	{
		if (!stricmp(section.m_Columns[columnIndex].m_szColumnName, columnName))
			break;
	}
	if (!section.m_Columns.IsValidIndex(columnIndex))
		return false;
	column_t &column = section.m_Columns[columnIndex];

	// modify the text
	wcsncpy(column.m_szColumnText, columnText, sizeof(column.m_szColumnText) / sizeof(wchar_t));
	column.m_szColumnText[sizeof(column.m_szColumnText) / sizeof(wchar_t) - 1] = 0;
	section.m_pHeader->InvalidateLayout();
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: adds an item to the list; returns itemID
//-----------------------------------------------------------------------------
int SectionedListPanel::AddItem(int sectionID, KeyValues *data)
{
	int itemID = m_Items.AddToTail();
	m_Items[itemID] = SETUP_PANEL(new CItemButton(this, itemID));

	ModifyItem(itemID, sectionID, data);

	// not sorted but in list
	m_SortedItems.AddToTail(m_Items[itemID]);
    m_bSortNeeded = true;

	return itemID;
}

//-----------------------------------------------------------------------------
// Purpose: modifies an existing item; returns false if the item does not exist
//-----------------------------------------------------------------------------
bool SectionedListPanel::ModifyItem(int itemID, int sectionID, KeyValues *data)
{
	if ( !m_Items.IsValidIndex(itemID) )
		return false;

	InvalidateLayout();
	m_Items[itemID]->SetSectionID(sectionID);
	m_Items[itemID]->SetData(data);
	m_Items[itemID]->InvalidateLayout();
    m_bSortNeeded = true;
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void SectionedListPanel::SetItemFgColor( int itemID, Color color )
{
	Assert( m_Items.IsValidIndex(itemID) );
	if ( !m_Items.IsValidIndex(itemID) )
		return;

	m_Items[itemID]->SetFgColor( color );
	m_Items[itemID]->SetOverrideColors( true );
	m_Items[itemID]->InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: sets the color of a section text & underline
//-----------------------------------------------------------------------------
void SectionedListPanel::SetSectionFgColor(int sectionID, Color color)
{
	if (!m_Sections.IsValidIndex(sectionID))
		return;

	m_Sections[sectionID].m_pHeader->SetColor(color);
}

//-----------------------------------------------------------------------------
// Purpose: forces a section to always be visible
//-----------------------------------------------------------------------------
void SectionedListPanel::SetSectionAlwaysVisible(int sectionID)
{
	m_Sections[sectionID].m_bAlwaysVisible = true;
}

//-----------------------------------------------------------------------------
// Purpose: removes an item from the list; returns false if the item does not exist or is already removed
//-----------------------------------------------------------------------------
bool SectionedListPanel::RemoveItem(int itemID)
{
	if ( !m_Items.IsValidIndex(itemID) )
		return false;

	m_SortedItems.FindAndRemove(m_Items[itemID]);
    m_bSortNeeded = true;

	m_Items[itemID]->MarkForDeletion();
	m_Items.Remove(itemID);

	InvalidateLayout();
	
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: returns the number of columns in a section
//-----------------------------------------------------------------------------
int SectionedListPanel::GetColumnCountBySection(int sectionID)
{
	int index = FindSectionIndexByID(sectionID);
	if (index < 0)
		return NULL;

	return m_Sections[index].m_Columns.Size();
}

//-----------------------------------------------------------------------------
// Purpose: returns the name of a column by section and column index; returns NULL if there are no more columns
//			valid range of columnIndex is [0, GetColumnCountBySection)
//-----------------------------------------------------------------------------
const char *SectionedListPanel::GetColumnNameBySection(int sectionID, int columnIndex)
{
	int index = FindSectionIndexByID(sectionID);
	if (index < 0 || columnIndex >= m_Sections[index].m_Columns.Size())
		return NULL;

	return m_Sections[index].m_Columns[columnIndex].m_szColumnName;
}

//-----------------------------------------------------------------------------
// Purpose: returns the text for a column by section and column index
//-----------------------------------------------------------------------------
const wchar_t *SectionedListPanel::GetColumnTextBySection(int sectionID, int columnIndex)
{
	int index = FindSectionIndexByID(sectionID);
	if (index < 0 || columnIndex >= m_Sections[index].m_Columns.Size())
		return NULL;
	
	return m_Sections[index].m_Columns[columnIndex].m_szColumnText;
}

//-----------------------------------------------------------------------------
// Purpose: returns the type of a column by section and column index
//-----------------------------------------------------------------------------
int SectionedListPanel::GetColumnFlagsBySection(int sectionID, int columnIndex)
{
	int index = FindSectionIndexByID(sectionID);
	if (index < 0)
		return 0;

	if (columnIndex >= m_Sections[index].m_Columns.Size())
		return 0;

	return m_Sections[index].m_Columns[columnIndex].m_iColumnFlags;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int SectionedListPanel::GetColumnWidthBySection(int sectionID, int columnIndex)
{
	int index = FindSectionIndexByID(sectionID);
	if (index < 0)
		return 0;

	if (columnIndex >= m_Sections[index].m_Columns.Size())
		return 0;

	return m_Sections[index].m_Columns[columnIndex].m_iWidth;
}

//-----------------------------------------------------------------------------
// Purpose: returns -1 if section not found
//-----------------------------------------------------------------------------
int SectionedListPanel::FindSectionIndexByID(int sectionID)
{
	for (int i = 0; i < m_Sections.Size(); i++)
	{
		if (m_Sections[i].m_iID == sectionID)
		{
			return i;
		}
	}

	return -1;
}

//-----------------------------------------------------------------------------
// Purpose: Called when the scrollbar is moved
//-----------------------------------------------------------------------------
void SectionedListPanel::OnSliderMoved()
{
	InvalidateLayout();
	Repaint();
}

//-----------------------------------------------------------------------------
// Purpose: Scrolls the list according to the mouse wheel movement
//-----------------------------------------------------------------------------
void SectionedListPanel::OnMouseWheeled(int delta)
{
	if (m_hEditModePanel.Get())
	{
		// ignore mouse wheel in edit mode, forward right up to parent
		if (GetParent())
		{
			GetParent()->OnMouseWheeled(delta);
		}
		return;
	}

	// scroll the window based on the delta
	int val = m_pScrollBar->GetValue();
	val -= (delta * BUTTON_HEIGHT_DEFAULT * 3);
	m_pScrollBar->SetValue(val);
}

//-----------------------------------------------------------------------------
// Purpose: Resets the scrollbar position on size change
//-----------------------------------------------------------------------------
void SectionedListPanel::OnSizeChanged(int wide, int tall)
{
	BaseClass::OnSizeChanged(wide, tall);
	m_pScrollBar->SetValue(0);
	InvalidateLayout();
	Repaint();
}

//-----------------------------------------------------------------------------
// Purpose: deselects any items
//-----------------------------------------------------------------------------
void SectionedListPanel::OnMousePressed(MouseCode code)
{
	SetSelectedItem((CItemButton *)NULL);
}

//-----------------------------------------------------------------------------
// Purpose: arrow key movement handler
//-----------------------------------------------------------------------------
void SectionedListPanel::OnKeyCodeTyped(KeyCode code)
{
	if (m_hEditModePanel.Get())
	{
		// ignore arrow keys in edit mode
		// forward right up to parent so that tab focus change doesn't occur
		if (GetParent())
		{
			GetParent()->OnKeyCodeTyped(code);
		}
		return;
	}

	int buttonTall = GetSectionTall();
		
	if (code == KEY_DOWN)
	{
		int itemID = GetSelectedItem();
        Assert(itemID != -1);

		if (!m_SortedItems.Count()) // if the list has been emptied
			return;

        int i;
        for (i = 0; i < m_SortedItems.Count(); i++)
        {
            if (m_SortedItems[i]->GetID() == itemID)
                break;
        }

        Assert(i != m_SortedItems.Count());

        // we're already on the end
        if (i == m_SortedItems.Count() - 1)
            return;

        int newItemID = m_SortedItems[i + 1]->GetID();
        SetSelectedItem(m_Items[newItemID]);
        ScrollToItem(newItemID);
        return;
	}
	else if (code == KEY_UP)
	{
		int itemID = GetSelectedItem();
        Assert(itemID != -1);

		if (!m_SortedItems.Count()) // if the list has been emptied
			return;

        int i;
        for (i = 0; i < m_SortedItems.Count(); i++)
        {
            if (m_SortedItems[i]->GetID() == itemID)
                break;
        }

        Assert(i != m_SortedItems.Count());
	
        // we're already on the end
        if (i == 0)
            return;

        int newItemID = m_SortedItems[i - 1]->GetID();
        SetSelectedItem(m_Items[newItemID]);
        ScrollToItem(newItemID);
        return;
	}
    else if (code == KEY_PAGEDOWN)
    {
        // calculate info for # of rows
        int cx, cy, cwide, ctall;
        GetBounds(cx, cy, cwide, ctall);

        int rowsperpage = ctall/buttonTall;

        int itemID = GetSelectedItem();
        int lastValidItem = itemID;
        int secID = m_Items[itemID]->GetSectionID();
        int i=0;
		int row = m_SortedItems.Find(m_Items[itemID]);

		while ( i < rowsperpage )
        {
			if ( m_SortedItems.IsValidIndex(++row) )
            {
				itemID = m_SortedItems[row]->GetID();
                lastValidItem = itemID;
                i++;

                // if we switched sections, then count the section header as a row
                if (m_Items[itemID]->GetSectionID() != secID)
                {
                    secID = m_Items[itemID]->GetSectionID();
                    i++;
                }
            }
			else
            {
                itemID = lastValidItem;
                break;
            }
        }
        SetSelectedItem(m_Items[itemID]);
        ScrollToItem(itemID);
    }
    else if (code == KEY_PAGEUP)
    {
        // calculate info for # of rows
        int cx, cy, cwide, ctall;
        GetBounds(cx, cy, cwide, ctall);
        int rowsperpage = ctall/buttonTall;

        int itemID = GetSelectedItem();
        int lastValidItem = itemID;
        int secID = m_Items[itemID]->GetSectionID();
        int i=0;
		int row = m_SortedItems.Find(m_Items[itemID]);
        while ( i < rowsperpage )
        {
			if ( m_SortedItems.IsValidIndex(--row) )
            {
				itemID = m_SortedItems[row]->GetID();
                lastValidItem = itemID;
                i++;

                // if we switched sections, then count the section header as a row
                if (m_Items[itemID]->GetSectionID() != secID)
                {
                    secID = m_Items[itemID]->GetSectionID();
                    i++;
                }
            }
			else
            {
                SetSelectedItem(m_Items[lastValidItem]);
                m_pScrollBar->SetValue(0);
                return;
            }
        }
        SetSelectedItem(m_Items[itemID]);
        ScrollToItem(itemID);
    }
	else if (code == KEY_LEFT || code == KEY_RIGHT)
	{
	}
	else
	{
		BaseClass::OnKeyCodeTyped(code);
	}
}


//-----------------------------------------------------------------------------
// Purpose: Clears the list
//-----------------------------------------------------------------------------
void SectionedListPanel::DeleteAllItems()
{
	FOR_EACH_LL( m_Items, i )
	{
		m_Items[i]->SetVisible(false);
		m_Items[i]->MarkForDeletion();
	}

	m_Items.RemoveAll();
	m_SortedItems.RemoveAll();
	m_hSelectedItem = NULL;
	InvalidateLayout();
    m_bSortNeeded = true;
}

//-----------------------------------------------------------------------------
// Purpose: Changes the current list selection
//-----------------------------------------------------------------------------
void SectionedListPanel::SetSelectedItem(CItemButton *item)
{
	if (m_hSelectedItem.Get() == item)
		return;

	// deselect the current item
	if (m_hSelectedItem.Get())
	{
		m_hSelectedItem->SetSelected(false);
	}

	// set the new item
	m_hSelectedItem = item;
	if (m_hSelectedItem.Get())
	{
		m_hSelectedItem->SetSelected(true);
	}

	PostActionSignal(new KeyValues("ItemSelected", "itemID", m_hSelectedItem.Get() ? m_hSelectedItem->GetID() : -1));
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int SectionedListPanel::GetSelectedItem()
{
	if (m_hSelectedItem.Get())
	{
		return m_hSelectedItem->GetID();
	}
	return -1;
}

//-----------------------------------------------------------------------------
// Purpose: sets which item is currently selected
//-----------------------------------------------------------------------------
void SectionedListPanel::SetSelectedItem(int itemID)
{
	if ( m_Items.IsValidIndex(itemID) )
	{
		SetSelectedItem(m_Items[itemID]);
	}
}

//-----------------------------------------------------------------------------
// Purpose: returns the data of a selected item
//-----------------------------------------------------------------------------
KeyValues *SectionedListPanel::GetItemData(int itemID)
{
	if ( !m_Items.IsValidIndex(itemID) )
		return NULL;

	return m_Items[itemID]->GetData();
}

//-----------------------------------------------------------------------------
// Purpose: returns what section an item is in
//-----------------------------------------------------------------------------
int SectionedListPanel::GetItemSection(int itemID)
{
	if ( !m_Items.IsValidIndex(itemID) )
		return -1;

	return m_Items[itemID]->GetSectionID();
}

//-----------------------------------------------------------------------------
// Purpose: returns true if the itemID is valid for use
//-----------------------------------------------------------------------------
bool SectionedListPanel::IsItemIDValid(int itemID)
{
	return m_Items.IsValidIndex(itemID);
}

//-----------------------------------------------------------------------------
// Purpose: returns true if the itemID is valid for use
//-----------------------------------------------------------------------------
int SectionedListPanel::GetHighestItemID()
{
	return m_Items.MaxElementIndex();
}

//-----------------------------------------------------------------------------
// Purpose: item iterators
//-----------------------------------------------------------------------------
int SectionedListPanel::GetItemCount()
{
	return m_SortedItems.Count();
}

//-----------------------------------------------------------------------------
// Purpose: item iterators
//-----------------------------------------------------------------------------
int SectionedListPanel::GetItemIDFromRow(int row)
{
	if ( !m_SortedItems.IsValidIndex(row) )
		return -1;

	return m_SortedItems[row]->GetID();
}

//-----------------------------------------------------------------------------
// Purpose: gets the local coordinates of a cell
//-----------------------------------------------------------------------------
bool SectionedListPanel::GetCellBounds(int itemID, int column, int &x, int &y, int &wide, int &tall)
{
	x = y = wide = tall = 0;
	if ( !IsItemIDValid(itemID) )
		return false;

	// get the item
	CItemButton *item = m_Items[itemID];

	if ( !item->IsVisible() )
		return false;

	//!! ignores column for now
	item->GetBounds(x, y, wide, tall);
	item->GetCellBounds(column, x, wide);
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: forces an item to redraw
//-----------------------------------------------------------------------------
void SectionedListPanel::InvalidateItem(int itemID)
{
	if ( !IsItemIDValid(itemID) )
		return;

	m_Items[itemID]->InvalidateLayout();
	m_Items[itemID]->Repaint();
}

//-----------------------------------------------------------------------------
// Purpose: set up a field for editing
//-----------------------------------------------------------------------------
void SectionedListPanel::EnterEditMode(int itemID, int column, vgui::Panel *editPanel)
{
	m_hEditModePanel = editPanel;
	m_iEditModeItemID = itemID;
	m_iEditModeColumn = column;
	editPanel->SetParent(this);
	editPanel->SetVisible(true);
	editPanel->RequestFocus();
	editPanel->MoveToFront();
	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: leaves editing mode
//-----------------------------------------------------------------------------
void SectionedListPanel::LeaveEditMode()
{
	if (m_hEditModePanel.Get())
	{
		InvalidateItem(m_iEditModeItemID);
		m_hEditModePanel->SetVisible(false);
		m_hEditModePanel->SetParent((Panel *)NULL);
		m_hEditModePanel = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: returns true if we are currently in inline editing mode
//-----------------------------------------------------------------------------
bool SectionedListPanel::IsInEditMode()
{
	return (m_hEditModePanel.Get() != NULL);
}

//-----------------------------------------------------------------------------
// Purpose: list used to match indexes in image columns to image pointers
//-----------------------------------------------------------------------------
void SectionedListPanel::SetImageList(ImageList *imageList, bool deleteImageListWhenDone)
{
	m_bDeleteImageListWhenDone = deleteImageListWhenDone;
	m_pImageList = imageList;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void SectionedListPanel::OnSetFocus()
{
	if (m_hSelectedItem.Get())
	{
        m_hSelectedItem->RequestFocus();
	}
    else
	{
        BaseClass::OnSetFocus();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int SectionedListPanel::GetSectionTall()
{
	if (m_Sections.Count())
	{
		HFont font = m_Sections[0].m_pHeader->GetFont();
		if (font != INVALID_FONT)
		{
			return surface()->GetFontTall(font) + BUTTON_HEIGHT_SPACER;
		}
	}

	return BUTTON_HEIGHT_DEFAULT;
}

//-----------------------------------------------------------------------------
// Purpose: returns the size required to fully draw the contents of the panel
//-----------------------------------------------------------------------------
void SectionedListPanel::GetContentSize(int &wide, int &tall)
{
	// make sure our layout is done
	if (IsLayoutInvalid())
	{
		if (m_bSortNeeded)
		{
			ReSortList();
			m_bSortNeeded = false;
		}
		LayoutPanels(m_iContentHeight);
	}

	wide = GetWide();
	tall = m_iContentHeight;
}

//-----------------------------------------------------------------------------
// Purpose: Message map
//-----------------------------------------------------------------------------
MessageMapItem_t SectionedListPanel::m_MessageMap[] =
{
	MAP_MESSAGE( SectionedListPanel, "ScrollBarSliderMoved", OnSliderMoved ),
};
IMPLEMENT_PANELMAP(SectionedListPanel, Panel);

