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

#include <assert.h>

#define PROTECTED_THINGS_DISABLE

#include <vgui/Cursor.h>
#include <vgui/IInput.h>
#include <vgui/ILocalize.h>
#include <vgui/IPanel.h>
#include <vgui/IScheme.h>
#include <vgui/ISystem.h>
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui/KeyCode.h>
#include <KeyValues.h>
#include <vgui/MouseCode.h>

#include <vgui_controls/Button.h>
#include <vgui_controls/Controls.h>
#include <vgui_controls/ImageList.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/ListPanel.h>
#include <vgui_controls/ScrollBar.h>
#include <vgui_controls/TextImage.h>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

enum 
{
	SCROLLBAR_SIZE=18,  // the width of a scrollbar
	WINDOW_BORDER_WIDTH=2 // the width of the window's border
};


#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif

#ifndef clamp
#define clamp( val, min, max ) ( ((val) > (max)) ? (max) : ( ((val) < (min)) ? (min) : (val) ) )
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class ColumnButton : public Button
{
public:
	ColumnButton(vgui::Panel *parent, const char *name, const char *text) : Button(parent, name, text)
	{
	}

	virtual void SetBorder( IBorder *border )
	{
		BaseClass::SetBorder( border );
	}

	virtual void ApplySchemeSettings(IScheme *pScheme)
	{
		Button::ApplySchemeSettings(pScheme);

		SetContentAlignment(Label::a_west);
		SetFont(pScheme->GetFont("DefaultSmall", IsProportional()));
	}

	// Don't request focus.
	// This will keep items in the listpanel selected.
	virtual void OnMousePressed(MouseCode code)
	{
		if (!IsEnabled())
			return;
		
		if (!IsMouseClickEnabled(code))
			return;
		
		if (IsUseCaptureMouseEnabled())
		{
			{
				SetSelected(true);
				Repaint();
			}
			
			// lock mouse input to going to this button
			input()->SetMouseCapture(GetVPanel());
		}
	}
};

//-----------------------------------------------------------------------------
// Purpose: Handles resizing of columns
//-----------------------------------------------------------------------------
class Dragger : public Panel
{
public:
	Dragger(int column);
	virtual void OnMousePressed(MouseCode code);
	virtual void OnMouseReleased(MouseCode code);
	virtual void OnCursorMoved(int x, int y);
	virtual void setMovable(bool state);

private:
	int m_iDragger;
	bool m_bDragging;
	int m_iDragPos;
	bool m_bMovable; // whether this dragger is movable using mouse or not
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
Dragger::Dragger(int column)
{
	m_iDragger = column;
	SetPaintBackgroundEnabled(false);
	SetPaintEnabled(false);
	SetPaintBorderEnabled(false);
	SetCursor(dc_sizewe);
	m_bDragging = false;
	m_bMovable = true; // movable by default
	m_iDragPos = 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Dragger::OnMousePressed(MouseCode code)
{
	if (m_bMovable)
	{
		input()->SetMouseCapture(GetVPanel());
		
		int x, y;
		input()->GetCursorPos(x, y);
		m_iDragPos = x;
		m_bDragging = true;
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Dragger::OnMouseReleased(MouseCode code)
{
	if (m_bMovable)
	{
		input()->SetMouseCapture(NULL);
		m_bDragging = false;
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Dragger::OnCursorMoved(int x, int y)
{
	if (m_bDragging)
	{
		input()->GetCursorPos(x, y);
		KeyValues *msg = new KeyValues("ColumnResized");
		msg->SetInt("column", m_iDragger);
		msg->SetInt("delta", x - m_iDragPos);
		m_iDragPos = x;
		if (GetParent())
		{
			ivgui()->PostMessage(GetParent()->GetVPanel(), msg, GetVPanel(), 0);
		}
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Dragger::setMovable(bool state)
{
	m_bMovable = state;
	// disable cursor change if the dragger is not movable
	if( IsVisible() )
	{
		if (state)
		{
			// if its not movable we stick with the default arrow
			// if parent windows Start getting fancy cursors we should probably retrive a parent
			// cursor and set it to that
			SetCursor(dc_sizewe); 
		}
		else 
		{
			SetCursor(dc_arrow); 
		}
	}
}

namespace vgui
{
// optimized for sorting
class FastSortListPanelItem : public ListPanelItem
{
public:
	// index into accessing item to sort
	CUtlVector<int> m_SortedTreeIndexes;

	// visibility flag (for quick hide/filter)
	bool visible;

		// precalculated sort orders
	int primarySortIndexValue;
	int secondarySortIndexValue;
};
}

static ListPanel *s_pCurrentSortingListPanel = NULL;
static const char *s_pCurrentSortingColumn = NULL;
static bool	s_currentSortingColumnTypeIsText = false;

static SortFunc *s_pSortFunc = NULL;
static bool s_bSortAscending = true;
static SortFunc *s_pSortFuncSecondary = NULL;
static bool s_bSortAscendingSecondary = true;

//-----------------------------------------------------------------------------
// Purpose: Basic sort function, for use in qsort
//-----------------------------------------------------------------------------
static int __cdecl AscendingSortFunc(const void *elem1, const void *elem2)
{
	int itemID1 = *((int *) elem1);
	int itemID2 = *((int *) elem2);

	// convert the item index into the ListPanelItem pointers
	vgui::ListPanelItem *p1, *p2;
	p1 = s_pCurrentSortingListPanel->GetItemData(itemID1);
	p2 = s_pCurrentSortingListPanel->GetItemData(itemID2);
	
	int result = s_pSortFunc(&p1, &p2);
	if (result == 0)
	{
		// use the secondary sort functino
		result = s_pSortFuncSecondary(&p1, &p2);

		if (!s_bSortAscendingSecondary)
		{
			result = -result;
		}

		if (result == 0)
		{
			// sort by the pointers to make sure we get consistent results
			if (p1 > p2)
			{
				result = 1;
			}
			else
			{
				result = -1;
			}
		}
	}
	else
	{
		// flip result if not doing an ascending sort
		if (!s_bSortAscending)
		{
			result = -result;
		}
	}

	return result;
}

//-----------------------------------------------------------------------------
// Purpose: Default column sorting function, puts things in alpabetical order
//          If images are the same returns 1, else 0
//-----------------------------------------------------------------------------
static int __cdecl DefaultSortFunc(const void *elem1, const void *elem2)
{
	vgui::ListPanelItem *p1, *p2;
	p1 = *(vgui::ListPanelItem **) elem1;
	p2 = *(vgui::ListPanelItem **) elem2;

	if ( !p1 || !p2 )  // No meaningful comparison
	{
		return 0;  
	}

	const char *col = s_pCurrentSortingColumn;
	if (s_currentSortingColumnTypeIsText) // textImage column
	{
		if (p1->kv->FindKey(col, true)->GetDataType() == KeyValues::TYPE_INT)
		{
			// compare ints
			int s1 = p1->kv->GetInt(col, 0);
			int s2 = p2->kv->GetInt(col, 0);

			return s1 < s2;
		}
		else
		{
			// compare as string
			const char *s1 = p1->kv->GetString(col, "");
			const char *s2 = p2->kv->GetString(col, "");

			return stricmp(s1, s2);
		}
	}
	else    // its an imagePanel column
	{
	   	const ImagePanel *s1 = (ImagePanel *)p1->kv->GetPtr(col, "");
		const ImagePanel *s2 = (ImagePanel *)p2->kv->GetPtr(col, "");

		return (s1 == s2);
	}
}


//-----------------------------------------------------------------------------
// Purpose: Sorts items by comparing precalculated list values
//-----------------------------------------------------------------------------
static int __cdecl FastSortFunc(const void *elem1, const void *elem2)
{
	vgui::FastSortListPanelItem *p1, *p2;
	p1 = *(vgui::FastSortListPanelItem **)elem1;
	p2 = *(vgui::FastSortListPanelItem **)elem2;

	Assert(p1 && p2);

	// compare the precalculated indices
	if (p1->primarySortIndexValue < p2->primarySortIndexValue)
	{
		return 1;
	}
	else if (p1->primarySortIndexValue > p2->primarySortIndexValue)
	{
		return -1;

	}

	// they're equal, compare the secondary indices
	if (p1->secondarySortIndexValue < p2->secondarySortIndexValue)
	{
		return 1;
	}
	else if (p1->secondarySortIndexValue > p2->secondarySortIndexValue)
	{
		return -1;

	}

	// still equal; just compare the pointers (so we get deterministic results)
	return (p1 < p2) ? 1 : -1;
}

static int s_iDuplicateIndex = 1;

//-----------------------------------------------------------------------------
// Purpose: sorting function used in the column index redblack tree
//-----------------------------------------------------------------------------
bool ListPanel::RBTreeLessFunc(vgui::ListPanel::IndexItem_t &item1, vgui::ListPanel::IndexItem_t &item2)
{
	int result = s_pSortFunc(&item1.dataItem, &item2.dataItem);
	if (result == 0)
	{
		// they're the same value, set their duplicate index to reflect that
		if (item1.duplicateIndex)
		{
			item2.duplicateIndex = item1.duplicateIndex;
		}
		else if (item2.duplicateIndex)
		{
			item1.duplicateIndex = item2.duplicateIndex;
		}
		else
		{
			item1.duplicateIndex = item2.duplicateIndex = s_iDuplicateIndex++;
		}
	}
	return (result > 0);
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
ListPanel::ListPanel(Panel *parent, const char *panelName) : Panel(parent, panelName)
{
	m_iHeaderHeight = 20;
	m_iRowHeight = 20;
	m_bCanSelectIndividualCells = false;
	m_iSelectedColumn = -1;

	m_hbar = new ScrollBar(this, "HorizScrollBar", false);
	m_hbar->AddActionSignalTarget(this);
	m_hbar->SetVisible(false);
	m_vbar = new ScrollBar(this, "VertScrollBar", true);
	m_vbar->SetVisible(false);
	m_vbar->AddActionSignalTarget(this);

	m_pLabel = new Label(this, NULL, "");
	m_pLabel->SetVisible(false);
	m_pLabel->SetPaintBackgroundEnabled(false);
	m_pLabel->SetContentAlignment(Label::a_west);

	m_pTextImage = new TextImage( "" );
	m_pImagePanel = new ImagePanel(NULL, "ListImage");
	m_pImagePanel->SetAutoDelete(false);

	m_iSortColumn = -1;
	m_iSortColumnSecondary = -1;
	m_bSortAscending = true;
	m_bSortAscendingSecondary = true;

	m_lastBarWidth = 0;
	m_iColumnDraggerMoved = -1;
	m_bNeedsSort = false;
	m_LastItemSelected = -1;

	m_pImageList = NULL;
	m_bDeleteImageListWhenDone = false;
	m_pEmptyListText = new TextImage("");
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
ListPanel::~ListPanel()
{
	// free data from table
	DeleteAllItems();

	// free column headers
	unsigned char i;
	for ( i = m_ColumnsData.Head(); i != m_ColumnsData.InvalidIndex(); i= m_ColumnsData.Next( i ) )
	{
		m_ColumnsData[i].m_pHeader->MarkForDeletion();
		m_ColumnsData[i].m_pResizer->MarkForDeletion();
	}
	m_ColumnsData.RemoveAll();

	delete m_pTextImage;
	delete m_pImagePanel;
	delete m_vbar;

	if ( m_bDeleteImageListWhenDone )
	{
		delete m_pImageList;
	}

	delete m_pEmptyListText;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ListPanel::SetImageList(ImageList *imageList, bool deleteImageListWhenDone)
{
	// get rid of existing list image if there's one and we're supposed to get rid of it
	if ( m_pImageList && m_bDeleteImageListWhenDone )
	{
		delete m_pImageList;
	}

	m_bDeleteImageListWhenDone = deleteImageListWhenDone;
	m_pImageList = imageList;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ListPanel::SetColumnHeaderHeight( int height )
{
	m_iHeaderHeight = height;
}

//-----------------------------------------------------------------------------
// Purpose: adds a column header. 
//			this->FindChildByName(columnHeaderName) can be used to retrieve a pointer to a header panel by name
//
// if minWidth and maxWidth are BOTH NOTRESIZABLE or RESIZABLE
// the min and max size will be calculated automatically for you with that attribute
// columns are resizable by default
// if min and max size are specified column is resizable
//
// A small note on passing numbers for minWidth and maxWidth, 
// If the initial window size is larger than the sum of the original widths of the columns,
// you can wind up with the columns "snapping" to size after the first window focus
// This is because the dxPerBar being calculated in PerformLayout()
// is making resizable bounded headers exceed thier maxWidths at the Start. 
// Solution is to either put in support for redistributing the extra dx being truncated and
// therefore added to the last column on window opening, which is what causes the snapping.
// OR to
// ensure the difference between the starting sum of widths is not too much smaller/bigger 
// than the starting window size so the starting dx doesn't cause snapping to occur.
// The easiest thing is to simply set it so your column widths add up to the starting size of the window on opening.
//
// Another note: Always give bounds for the last column you add or make it not resizable.
//
// Columns can have text headers or images for headers (e.g. password icon)
//-----------------------------------------------------------------------------
void ListPanel::AddColumnHeader(int index, const char *columnName, const char *columnText, int width, bool isTextImage, bool sliderResizable, bool windowResizable)
{
	if (sliderResizable)
	{
		AddColumnHeader( index, columnName, columnText, width, isTextImage, 20, 10000, windowResizable );
	}
	else
	{
		AddColumnHeader( index, columnName, columnText, width, isTextImage, width, width, windowResizable);
	}
}

void ListPanel::AddColumnHeader(int index, const char *columnName, const char *columnText, int width, bool isTextImage, int minWidth, int maxWidth, bool windowResizable)
{
	Assert (minWidth <= width);
	Assert (maxWidth >= width);

	// get our permanent index
	unsigned char columnDataIndex = m_ColumnsData.AddToTail();

	// put this index on the tail, so all item's m_SortedTreeIndexes have a consistent mapping
	m_ColumnsHistory.AddToTail(columnDataIndex);

	// put this column in the right place visually
	m_CurrentColumns.InsertBefore(index, columnDataIndex);

	// create the actual column object
	column_t &column = m_ColumnsData[columnDataIndex];

	// create the column header button
	Button *pButton = new ColumnButton(this, columnName, columnText); 
	pButton->SetSize(width, 24);
	pButton->AddActionSignalTarget(this);
	pButton->SetContentAlignment(Label::a_west);
	pButton->SetTextInset(5, 0);
	if (!isTextImage)	// images cannot be sorted
	{
		pButton->SetMouseClickEnabled(MOUSE_LEFT, 0);
	}
	column.m_pHeader = pButton;

	column.m_iMinWidth = minWidth;
	column.m_iMaxWidth = maxWidth;
	column.m_bResizesWithWindow = windowResizable;
	column.m_bTypeIsText = isTextImage;

	Dragger *dragger = new Dragger(index);
	dragger->SetParent(this);
	dragger->AddActionSignalTarget(this);
	dragger->MoveToFront();
	if (minWidth == maxWidth ) //not resizable so disable the slider 
	{
	   dragger->setMovable(false);
	}
	column.m_pResizer = dragger;

	// add default sort function
	column.m_pSortFunc = NULL;
	
	// Set the SortedTree less than func to the generic RBTreeLessThanFunc
	m_ColumnsData[columnDataIndex].m_SortedTree.SetLessFunc((bool (__cdecl *)(const struct vgui2::ListPanel::IndexItem_t &,const struct vgui2::ListPanel::IndexItem_t &))RBTreeLessFunc);

	// go through all the headers and make sure their Command has the right column ID
	ResetColumnHeaderCommands();

	// 
	ResortColumnRBTree(index);

	// ensure scroll bar is topmost compared to column headers
	m_vbar->MoveToFront();

	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: Recreates a column's RB Sorted Tree
//-----------------------------------------------------------------------------
void ListPanel::ResortColumnRBTree(int col)
{
	//MODDD - assert linker error
	//assert(m_CurrentColumns.IsValidIndex(col));

	unsigned char dataColumnIndex = m_CurrentColumns[col];
	int columnHistoryIndex = m_ColumnsHistory.Find(dataColumnIndex);
	column_t &column = m_ColumnsData[dataColumnIndex];

	IndexRBTree_t &rbtree = column.m_SortedTree;

	// remove all elements - we're going to create from scratch
	rbtree.RemoveAll();

	s_pCurrentSortingListPanel = this;
	s_currentSortingColumnTypeIsText = column.m_bTypeIsText; // type of data in the column
	SortFunc *sortFunc = column.m_pSortFunc;
	if ( !sortFunc )
	{
		sortFunc = DefaultSortFunc;
	}
	s_pSortFunc = sortFunc;
	s_bSortAscending = true;
	s_pSortFuncSecondary = NULL;

	// sort all current data items for this column
	FOR_EACH_LL( m_DataItems, i )
	{
		IndexItem_t item;
		item.dataItem = m_DataItems[i];
		item.duplicateIndex = 0;

		FastSortListPanelItem *dataItem = (FastSortListPanelItem*) m_DataItems[i];

		// if this item doesn't already have a SortedTreeIndex for this column,
		// if can only be because this is the brand new column, so add it to the SortedTreeIndexes
		if (dataItem->m_SortedTreeIndexes.Count() == m_ColumnsHistory.Count() - 1 &&
			columnHistoryIndex == m_ColumnsHistory.Count() - 1)
		{
			dataItem->m_SortedTreeIndexes.AddToTail();
		}


		//MODDD - assert linker error
		//assert( dataItem->m_SortedTreeIndexes.IsValidIndex(columnHistoryIndex) );

		dataItem->m_SortedTreeIndexes[columnHistoryIndex] = rbtree.Insert(item);
	}

}

//-----------------------------------------------------------------------------
// Purpose: Resets the "SetSortColumn" command for each column - in case columns were added or removed
//-----------------------------------------------------------------------------
void ListPanel::ResetColumnHeaderCommands()
{
	int i;
	for ( i = 0 ; i < m_CurrentColumns.Count() ; i++ )
	{
		Button *pButton = m_ColumnsData[m_CurrentColumns[i]].m_pHeader;
		pButton->SetCommand(new KeyValues("SetSortColumn", "column", i));
	}
}

//-----------------------------------------------------------------------------
// Purpose: Sets the header text for a particular column.
//-----------------------------------------------------------------------------
void ListPanel::SetColumnHeaderText(int col, const char *text)
{
	m_ColumnsData[m_CurrentColumns[col]].m_pHeader->SetText(text);
}
void ListPanel::SetColumnHeaderText(int col, wchar_t *text)
{
	m_ColumnsData[m_CurrentColumns[col]].m_pHeader->SetText(text);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ListPanel::SetColumnSortable(int col, bool sortable)
{
	if (sortable)
	{
		m_ColumnsData[m_CurrentColumns[col]].m_pHeader->SetCommand(new KeyValues("SetSortColumn", "column", col));
	}
	else
	{
		m_ColumnsData[m_CurrentColumns[col]].m_pHeader->SetCommand((const char *)NULL);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ListPanel::RemoveColumn(int col)
{
	// find the appropriate column data 
	unsigned char columnDataIndex = m_CurrentColumns[col];

	// remove it from the current columns
	m_CurrentColumns.Remove(col);

	// zero out this entry in m_ColumnsHistory
	unsigned char i;
	for ( i = 0 ; i < m_ColumnsHistory.Count() ; i++ )
	{
		if ( m_ColumnsHistory[i] == columnDataIndex )
		{
			m_ColumnsHistory[i] = m_ColumnsData.InvalidIndex();
			break;
		}
	}
	Assert( i != m_ColumnsHistory.Count() );

	// delete and remove the column data
	m_ColumnsData[columnDataIndex].m_SortedTree.RemoveAll();
	m_ColumnsData[columnDataIndex].m_pHeader->MarkForDeletion();
	m_ColumnsData[columnDataIndex].m_pResizer->MarkForDeletion();
	m_ColumnsData.Remove(columnDataIndex);

	ResetColumnHeaderCommands();
	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: Returns the index of a column by column->GetName()
//-----------------------------------------------------------------------------
int ListPanel::FindColumn(const char *columnName)
{
	for (int i = 0; i < m_CurrentColumns.Count(); i++)
	{
		if (!stricmp(columnName, m_ColumnsData[m_CurrentColumns[i]].m_pHeader->GetName()))
		{
			return i;
		}
	}
	return -1;
}


//-----------------------------------------------------------------------------
// Purpose: adds an item to the view
//			data->GetName() is used to uniquely identify an item
//			data sub items are matched against column header name to be used in the table
//-----------------------------------------------------------------------------
int ListPanel::AddItem( KeyValues *item, unsigned int userData, bool bScrollToItem, bool bSortOnAdd)
{
	FastSortListPanelItem *newitem = new FastSortListPanelItem;
	newitem->kv = item->MakeCopy();
	newitem->userData = userData;
	int itemID = m_DataItems.AddToTail(newitem);
	int displayRow = m_VisibleItems.AddToTail(itemID);
	newitem->visible = true;

	// put the item in each column's sorted Tree Index
	IndexItem(itemID);

	if ( bSortOnAdd )
	{
		m_bNeedsSort = true;
	}

	InvalidateLayout();
	
	if ( bScrollToItem )
	{
		// scroll to last item
		m_vbar->SetValue(displayRow);
	}
	return itemID;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ListPanel::SetUserData( int itemID, unsigned int userData )
{
	if ( !m_DataItems.IsValidIndex(itemID) )
		return;

	m_DataItems[itemID]->userData = userData;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int	ListPanel::GetItemCount( void )
{
	return m_VisibleItems.Count();
}

//-----------------------------------------------------------------------------
// Purpose: gets the item ID of an item by name (data->GetName())
//-----------------------------------------------------------------------------
int ListPanel::GetItem(const char *itemName)
{
	FOR_EACH_LL( m_DataItems, i )
	{
		if (!stricmp(m_DataItems[i]->kv->GetName(), itemName))
		{
			return i;
		}
	}

	// failure
	return -1;
}

//-----------------------------------------------------------------------------
// Purpose: returns pointer to data the itemID holds
//-----------------------------------------------------------------------------
KeyValues *ListPanel::GetItem(int itemID)
{
	if ( !m_DataItems.IsValidIndex(itemID) )
		return NULL;

	return m_DataItems[itemID]->kv;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int ListPanel::GetItemCurrentRow(int itemID)
{
	return m_VisibleItems.Find(itemID);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int ListPanel::GetItemIDFromRow(int currentRow)
{
	if (!m_VisibleItems.IsValidIndex(currentRow))
		return -1;

	return m_VisibleItems[currentRow];
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int ListPanel::InvalidItemID()
{
	return m_DataItems.InvalidIndex();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool ListPanel::IsValidItemID(int itemID)
{
	return m_DataItems.IsValidIndex(itemID);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
ListPanelItem *ListPanel::GetItemData( int itemID )
{
	if ( !m_DataItems.IsValidIndex(itemID) )
		return NULL;
	
	return m_DataItems[ itemID ];
}

//-----------------------------------------------------------------------------
// Purpose: returns user data for itemID
//-----------------------------------------------------------------------------
unsigned int ListPanel::GetItemUserData(int itemID)
{
	if ( !m_DataItems.IsValidIndex(itemID) )
		return 0;

	return m_DataItems[itemID]->userData;
}


//-----------------------------------------------------------------------------
// Purpose: updates the view with any changes to the data
// Input  : itemID - index to update
//-----------------------------------------------------------------------------
void ListPanel::ApplyItemChanges(int itemID)
{
	// reindex the item and then redraw
	IndexItem(itemID);
	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: Adds the item into the column indexes
//-----------------------------------------------------------------------------
void ListPanel::IndexItem(int itemID)
{
	int i;
	FastSortListPanelItem *newitem = (FastSortListPanelItem*) m_DataItems[itemID];

	// remove the item from the indexes and re-add
	int maxCount = min(m_ColumnsHistory.Count(), newitem->m_SortedTreeIndexes.Count());
	for (i = 0; i < maxCount; i++)
	{
		IndexRBTree_t &rbtree = m_ColumnsData[m_ColumnsHistory[i]].m_SortedTree;
		rbtree.RemoveAt(newitem->m_SortedTreeIndexes[i]);
	}

	// make sure it's all free
	newitem->m_SortedTreeIndexes.RemoveAll();

	// reserve one index per historical column - pad it out
	newitem->m_SortedTreeIndexes.AddMultipleToTail(m_ColumnsHistory.Count());

	// set the current sorting list (since the insert will need to sort)
	s_pCurrentSortingListPanel = this;

	// add the item into the RB tree for each column
	for (i = 0; i < m_ColumnsHistory.Count(); i++)
	{
		// skip over any removed columns
		if ( m_ColumnsHistory[i] == m_ColumnsData.InvalidIndex() )
			continue;

		column_t &column = m_ColumnsData[m_ColumnsHistory[i]];

		IndexItem_t item;
		item.dataItem = newitem;
		item.duplicateIndex = 0;

		IndexRBTree_t &rbtree = column.m_SortedTree;

		// setup sort state
		s_pCurrentSortingListPanel = this;
		s_pCurrentSortingColumn = column.m_pHeader->GetName(); // name of current column for sorting
		s_currentSortingColumnTypeIsText = column.m_bTypeIsText; // type of data in the column
		SortFunc *sortFunc = column.m_pSortFunc;
		if (!sortFunc)
		{
			sortFunc = DefaultSortFunc;
		}
		s_pSortFunc = sortFunc;
		s_bSortAscending = true;
		s_pSortFuncSecondary = NULL;

		// insert index		
		newitem->m_SortedTreeIndexes[i] = rbtree.Insert(item);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ListPanel::RereadAllItems()
{
	//!! need to make this more efficient
	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: Removes an item at the specified item
//-----------------------------------------------------------------------------
void ListPanel::RemoveItem(int itemID)
{
	FastSortListPanelItem *data = (FastSortListPanelItem*) m_DataItems[itemID];
	if (!data)
		return;

	// remove from column sorted indexes
	int i;
	for ( i = 0; i < m_ColumnsHistory.Count(); i++ )
	{
		if ( m_ColumnsHistory[i] == m_ColumnsData.InvalidIndex())
			continue;

		IndexRBTree_t &rbtree = m_ColumnsData[m_ColumnsHistory[i]].m_SortedTree;
		rbtree.RemoveAt(data->m_SortedTreeIndexes[i]);
	}

	// remove from selection
	m_SelectedItems.FindAndRemove(itemID);

	// remove from visible items
	m_VisibleItems.FindAndRemove(itemID);

	// remove from data
	m_DataItems.Remove(itemID);
	if (data->kv)
	{
		data->kv->deleteThis();
	}
	delete data;
	InvalidateLayout();

}

//-----------------------------------------------------------------------------
// Purpose: clears and deletes all the memory used by the data items
//-----------------------------------------------------------------------------
void ListPanel::DeleteAllItems()
{
	// remove all sort indexes
	for (int i = 0; i < m_ColumnsHistory.Count(); i++)
	{
		m_ColumnsData[m_ColumnsHistory[i]].m_SortedTree.RemoveAll();
	}

	FOR_EACH_LL( m_DataItems, index )
	{
		if ( m_DataItems[index] )
		{
			if ( m_DataItems[index]->kv )
			{
				m_DataItems[index]->kv->deleteThis();
			}
			delete m_DataItems[index];
		}
	}

	m_DataItems.RemoveAll();
	m_VisibleItems.RemoveAll();
	m_SelectedItems.RemoveAll();

	InvalidateLayout();
}

void ListPanel::ResetScrollBar()
{
	// delete and reallocate to besure the scroll bar's
	// information is correct.
	delete m_vbar;
	m_vbar = new ScrollBar(this, "VertScrollBar", true);
	m_vbar->SetVisible(false);
	m_vbar->AddActionSignalTarget(this);
}

//-----------------------------------------------------------------------------
// Purpose: returns the count of selected rows
//-----------------------------------------------------------------------------
int ListPanel::GetSelectedItemsCount()
{
	return m_SelectedItems.Count();
}

//-----------------------------------------------------------------------------
// Purpose: returns the selected item by selection index
// Input  : selectionIndex - valid in range [0, GetNumSelectedRows)
// Output : int - itemID
//-----------------------------------------------------------------------------
int ListPanel::GetSelectedItem(int selectionIndex)
{
	if ( m_SelectedItems.IsValidIndex(selectionIndex))
		return m_SelectedItems[selectionIndex];

	return -1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int ListPanel::GetSelectedColumn()
{
	return m_iSelectedColumn;
}

//-----------------------------------------------------------------------------
// Purpose: Clears all selected rows
//-----------------------------------------------------------------------------
void ListPanel::ClearSelectedItems()
{
	m_SelectedItems.RemoveAll();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ListPanel::AddSelectedItem(int itemID)
{
	if ( !m_DataItems.IsValidIndex(itemID) )
		return;

	//MODDD - assert linker error
	//assert(!m_SelectedItems.HasElement(itemID));

	m_SelectedItems.AddToTail(itemID);
	PostActionSignal(new KeyValues("ItemSelected"));
	Repaint();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ListPanel::SetSingleSelectedItem(int itemID)
{
	ClearSelectedItems();
	AddSelectedItem(itemID);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ListPanel::SetSelectedCell(int itemID, int col)
{
	// make sure it's a valid cell
	if ( !m_DataItems.IsValidIndex(itemID) )
		return;
	
	if ( !m_CurrentColumns.IsValidIndex(col) )
		return;
	
	m_iSelectedColumn = col;
	SetSingleSelectedItem(itemID);

	// notify all watchers
	PostActionSignal(new KeyValues("ItemSelected"));

	// redraw
	Repaint();
}

//-----------------------------------------------------------------------------
// Purpose: returns the data held by a specific cell
//-----------------------------------------------------------------------------
void ListPanel::GetCellText(int itemID, int col, wchar_t *wbuffer, int bufferSize)
{
	if ( !wbuffer || !bufferSize )
		return;

	wcscpy( wbuffer, L"" );

	KeyValues *itemData = GetItem( itemID );
	if ( !itemData )
	{
		return;
	}

	// Look up column header
	if ( col < 0 || col >= m_CurrentColumns.Count() )
	{
		return;
	}

	const char *key = m_ColumnsData[m_CurrentColumns[col]].m_pHeader->GetName();
	if ( !key || !key[ 0 ] )
	{
		return;
	}

	const wchar_t *val = itemData->GetWString( key, L"" );
	if ( !val || !key[ 0 ] )
		return;

	wcsncpy( wbuffer, val, bufferSize );
	wbuffer[ bufferSize - 1 ] = 0;
}

//-----------------------------------------------------------------------------
// Purpose: returns the data held by a specific cell
//-----------------------------------------------------------------------------
IImage *ListPanel::GetCellImage(int itemID, int col) //, ImagePanel *&buffer)
{
//	if ( !buffer )
//		return;

	KeyValues *itemData = GetItem( itemID );
	if ( !itemData )
	{
		return NULL;
	}

	// Look up column header
	if ( col < 0 || col >= m_CurrentColumns.Count() )
	{
		return NULL;
	}

	const char *key = m_ColumnsData[m_CurrentColumns[col]].m_pHeader->GetName();
	if ( !key || !key[ 0 ] )
	{
		return NULL;
	}

	if ( !m_pImageList )
	{
		return NULL;
	}

	int imageIndex = itemData->GetInt( key, 0 );
	if ( m_pImageList->IsValidIndex(imageIndex) )
	{
		if ( imageIndex > 0 )
		{
			return m_pImageList->GetImage(imageIndex);
		}
	}
	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Returns the panel to use to render a cell
//-----------------------------------------------------------------------------
Panel *ListPanel::GetCellRenderer(int itemID, int col)
{

	//MODDD - assert linker error
	//assert( m_pTextImage );
	//assert( m_pImagePanel );
	
	column_t& column = m_ColumnsData[ m_CurrentColumns[col] ];

	IScheme *pScheme = scheme()->GetIScheme( GetScheme() );

	if ( column.m_bTypeIsText ) 
	{
		wchar_t tempText[ 256 ];

		// Grab cell text
		GetCellText( itemID, col, tempText, 256 );
		m_pTextImage->SetText(tempText);
        int wide, tall;
        m_pTextImage->GetContentSize(wide, tall);

		// set cell size
		Panel *header = column.m_pHeader;
	    wide = header->GetWide();
		m_pTextImage->SetSize( wide - 5, tall);

		m_pLabel->SetImageAtIndex(0, m_pTextImage, 3);
			
		if ( m_SelectedItems.HasElement(itemID) && ( !m_bCanSelectIndividualCells || col == m_iSelectedColumn ) )
		{
            VPANEL focus = input()->GetFocus();
            // if one of the children of the SectionedListPanel has focus, then 'we have focus' if we're selected
            if (HasFocus() || (focus && ipanel()->HasParent(focus, GetParent()->GetVPanel())))
            {
                m_pLabel->SetBgColor(GetSchemeColor("Menu/ArmedBgColor", pScheme));
    			// selection
            }
            else
            {
                m_pLabel->SetBgColor(GetSchemeColor("SelectionBG2", pScheme));
            }
            m_pTextImage->SetColor(m_SelectionFgColor);
            m_pLabel->SetPaintBackgroundEnabled(true);
		}
		else
		{
			m_pTextImage->SetColor(m_LabelFgColor);
			m_pLabel->SetPaintBackgroundEnabled(false);
		}
		
		return m_pLabel;
	}
	else 	// if its an Image Panel
	{
		if ( m_SelectedItems.HasElement(itemID) && ( !m_bCanSelectIndividualCells || col == m_iSelectedColumn ) )
		{
            VPANEL focus = input()->GetFocus();
            // if one of the children of the SectionedListPanel has focus, then 'we have focus' if we're selected
            if (HasFocus() || (focus && ipanel()->HasParent(focus, GetParent()->GetVPanel())))
            {
                m_pLabel->SetBgColor(GetSchemeColor("Menu/ArmedBgColor", pScheme));
    			// selection
            }
            else
            {
                m_pLabel->SetBgColor(GetSchemeColor("SelectionBG2", pScheme));
            }
			// selection
			m_pLabel->SetPaintBackgroundEnabled(true);
		}
		else
		{
			m_pLabel->SetPaintBackgroundEnabled(false);
		}

		IImage *pIImage = GetCellImage(itemID, col);
		m_pLabel->SetImageAtIndex(0, pIImage, 3);

		return m_pLabel;
	}
}

//-----------------------------------------------------------------------------
// Purpose: relayouts out the panel after any internal changes
//-----------------------------------------------------------------------------
void ListPanel::PerformLayout()
{
	if (m_bNeedsSort)
	{
		SortList();
	}

	int rowsperpage = (int) GetRowsPerPage();

	// count the number of visible items
	int visibleItemCount = m_VisibleItems.Count();

	//!! need to make it recalculate scroll positions
	m_vbar->SetVisible(true);
	m_vbar->SetEnabled(false);
	m_vbar->SetRangeWindow( rowsperpage );
	m_vbar->SetRange( 0, visibleItemCount);	
	m_vbar->SetButtonPressedScrollValue( 1 );

	int wide, tall;
	GetSize( wide, tall );
	m_vbar->SetPos(wide - (SCROLLBAR_SIZE+WINDOW_BORDER_WIDTH), 0);
	m_vbar->SetSize(SCROLLBAR_SIZE, tall - 2);
	m_vbar->InvalidateLayout();

	int buttonMaxXPos = wide - (SCROLLBAR_SIZE+WINDOW_BORDER_WIDTH);
	
	int nColumns = m_CurrentColumns.Count();
	// number of bars that can be resized
	int numToResize=0;
	if (m_iColumnDraggerMoved != -1) // we're resizing in response to a column dragger
	{
		numToResize = 1; // only one column will change size, the one we dragged
	}
	else	// we're resizing in response to a window resize
	{
		for (int i = 0; i < nColumns; i++)
		{
			if ( m_ColumnsData[m_CurrentColumns[i]].m_bResizesWithWindow ) // column is resizable in response to window
				numToResize++;
		}
	}

	int dxPerBar; // zero on window first opening
	// location of the last column resizer
	int oldSizeX, oldSizeY;
	m_ColumnsData[m_CurrentColumns[nColumns-1]].m_pHeader->GetPos(oldSizeX, oldSizeY);	
	if (numToResize == 0)
	{
		dxPerBar = 0;
		m_lastBarWidth = buttonMaxXPos;
	}
	else if (oldSizeX != 0) // make sure this isnt the first time we opened the window
	{
		int dx = buttonMaxXPos - m_lastBarWidth;  // this is how much we grew or shrank.

		// see how many bars we have and now much each should grow/shrink
		dxPerBar=(int)((float)dx/(float)numToResize);
		m_lastBarWidth = buttonMaxXPos;
	}
	else // this is the first time we've opened the window, make sure all our colums fit! resize if needed
	{
		int startingBarWidth=0;
		for (int i = 0; i < nColumns; i++)
		{
			startingBarWidth += m_ColumnsData[m_CurrentColumns[i]].m_pHeader->GetWide();
		}
		int dx = buttonMaxXPos - startingBarWidth;  // this is how much we grew or shrank.
		// see how many bars we have and now much each should grow/shrink
		dxPerBar=(int)((float)dx/(float)numToResize);
		m_lastBarWidth = buttonMaxXPos;
	}

	while (1)
	{
		// try and place headers as is - before we have to force items to be minimum width
		int x = -1;
		int i;
		for ( i = 0; i < nColumns; i++)
		{
			column_t &column = m_ColumnsData[m_CurrentColumns[i]];
			Panel *header = column.m_pHeader;
			header->SetPos(x, 0);
				
			header->SetVisible(true);

			// if we couldn't fit this column - then we need to force items to be minimum width
			if ( x+column.m_iMinWidth >= buttonMaxXPos )
			{
				break;
			}
	
			int hWide = header->GetWide();
			
			// calculate the column's width
			// make it so the last column always attaches to the scroll bar
			if ( i == nColumns - 1 )
			{
				hWide = buttonMaxXPos-x; 
			}
			else if (i == m_iColumnDraggerMoved ) // column resizing using dragger
			{
				hWide += dxPerBar; // adjust width of column
			}
			else if ( m_iColumnDraggerMoved == -1 )		// window is resizing
			{
				if ( column.m_bResizesWithWindow )
				{
					Assert ( column.m_iMinWidth < column.m_iMaxWidth );
					hWide += dxPerBar; // adjust width of column
				}
			}
			
			// enforce column mins and max's
			if ( hWide < column.m_iMinWidth ) 
			{
				hWide = column.m_iMinWidth; // adjust width of column
			}
			else if ( hWide > column.m_iMaxWidth )
			{
				hWide = column.m_iMaxWidth;
			}
	
			header->SetSize(hWide, SCROLLBAR_SIZE);
			x += hWide;
	
			// set the resizers
			Panel *sizer = column.m_pResizer;
			if ( i == nColumns - 1 )
			{
				sizer->SetVisible(false);
			}
			sizer->MoveToFront();
			sizer->SetPos(x - 4, 0);
			sizer->SetSize(8, SCROLLBAR_SIZE);
		}

		// we made it all the way through
		if ( i == nColumns )
			break;
	
		// we do this AFTER trying first, to let as many columns as possible try and get to their
		// desired width before we forcing the minimum width on them

		// get the total desired width of all the columns
		int totalDesiredWidth = 0;
		for ( i = 0 ; i < nColumns ; i++ )
		{
			Panel *pHeader = m_ColumnsData[m_CurrentColumns[i]].m_pHeader;
			totalDesiredWidth += pHeader->GetWide();
		}

		// shrink from the most right column to minimum width until we can fit them all

		//MODDD - assert linker error
		//assert(totalDesiredWidth > buttonMaxXPos);

		for ( i = nColumns-1; i >= 0 ; i--)
		{
			column_t &column = m_ColumnsData[m_CurrentColumns[i]];
			Panel *pHeader = column.m_pHeader;

			totalDesiredWidth -= pHeader->GetWide();
			if (totalDesiredWidth + column.m_iMinWidth < buttonMaxXPos)
			{
				pHeader->SetSize(buttonMaxXPos - totalDesiredWidth, SCROLLBAR_SIZE);
				break;
			}

			totalDesiredWidth += column.m_iMinWidth;
			pHeader->SetSize(column.m_iMinWidth, SCROLLBAR_SIZE);
		}

		//MODDD - assert linker error
		//assert (i != -1);
	}

	Repaint();
	m_iColumnDraggerMoved = -1; // reset to invalid column
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ListPanel::OnSizeChanged(int wide, int tall)
{
	BaseClass::OnSizeChanged(wide, tall);
	InvalidateLayout();
	Repaint();
}


//-----------------------------------------------------------------------------
// Purpose: Renders the cells
//-----------------------------------------------------------------------------
void ListPanel::Paint()
{
	if (m_bNeedsSort)
	{
		SortList();
	}

	// draw selection areas if any
	int wide, tall;
  	GetSize( wide, tall );

	m_iTableStartX = 0; 
	m_iTableStartY = m_iHeaderHeight + 1;

	float totalrows = (float) m_VisibleItems.Count();
	int rowsperpage = (int) GetRowsPerPage();

	// find the first visible item to display
	int startitem = 0;
	if (rowsperpage <= totalrows)
	{
		startitem = m_vbar->GetValue();
	}

//	debug timing functions
//	double startTime, endTime;
//	startTime = system()->GetCurrentTime();

	// iterate through and draw each cell
	bool done = false;
	int drawcount = 0;
	for (int i = startitem; i < m_VisibleItems.Count() && !done; i++)
	{
		int x = 0;
		
		// iterate the columns
		for (int j = 0; j < m_CurrentColumns.Count(); j++)
		{
			int itemID = m_VisibleItems[i];
			Panel *header = m_ColumnsData[m_CurrentColumns[j]].m_pHeader;
			Panel *render = GetCellRenderer(itemID, j);

			if (!header->IsVisible())
				continue;

			int wide = header->GetWide();

			if (render)
			{
				// setup render panel
				if(render->GetParent() != this)
				{
					render->SetParent(this);
				}
				if( !render->IsVisible() )
				{
					render->SetVisible(true);
				}
				render->SetPos( x + m_iTableStartX + 2, (drawcount * m_iRowHeight) + m_iTableStartY);
				render->SetSize( wide, m_iRowHeight - 1 );

				// mark the panel to draw immediately (since it will probably be recycled to draw other cells)
				render->Repaint();
				surface()->SolveTraverse(render->GetVPanel());
				int x0, y0, x1, y1;
				render->GetClipRect(x0, y0, x1, y1);
				if ((y1 - y0) < (m_iRowHeight - 3))
				{
					done = true;
					break;
				}
				surface()->PaintTraverse(render->GetVPanel());
			}
			/*
			// work in progress, optimized paint for text
			else
			{
				// just paint it ourselves
				char tempText[256];
				// Grab cell text
				GetCellText(i, j, tempText, sizeof(tempText));
				surface()->DrawSetTextPos(x + m_iTableStartX + 2, (drawcount * m_iRowHeight) + m_iTableStartY);

				for (const char *pText = tempText; *pText != 0; pText++)
				{
					surface()->DrawUnicodeChar((wchar_t)*pText);
				}
			}
			*/

			x += wide;
		}

		drawcount++;
	}

	m_pLabel->SetVisible(false);

	// if the list is empty, draw some help text
	if (m_VisibleItems.Count() < 1 && m_pEmptyListText)
	{
		m_pEmptyListText->SetPos(m_iTableStartX + 8, m_iTableStartY + 4);
		m_pEmptyListText->SetSize(wide - 8, m_iRowHeight);
		m_pEmptyListText->Paint();
	}

//	endTime = system()->GetCurrentTime();
//	ivgui()->DPrintf2("ListPanel::Paint() (%.3f sec)\n", (float)(endTime - startTime));
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ListPanel::PaintBackground()
{
	BaseClass::PaintBackground();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ListPanel::OnMousePressed(MouseCode code)
{
	if (code == MOUSE_LEFT || code == MOUSE_RIGHT)
	{
		// determine where we were pressed
		int x, y, row, column;
		input()->GetCursorPos(x, y);
		GetCellAtPos(x, y, row, column);

		if ( m_LastItemSelected < 0 && m_VisibleItems.Count() > 0 && row < m_VisibleItems.Count())
		{	
			// we don't have a selected item, choose the first one
			m_LastItemSelected = 0;
		}

		if ( m_LastItemSelected != -1 )	// possible if there are no items
		{
			if ( code == MOUSE_RIGHT && m_SelectedItems.HasElement(row) )
			{
				// if we've right-clicked on a selection, don't change the selection
	
			}
			else if (row >= 0 && row < m_VisibleItems.Count())	// make sure we're clicking on a real item
			{
				int iSelectedColumn = -1;
				int itemID = m_VisibleItems[row];

				// if we're selecting individual cells, figure out the column
				if ( m_bCanSelectIndividualCells )
				{
					// only do column-specific positioning on single row select
					ScreenToLocal(x, y);

					// walk the columns, see where we are
					int i;
					for ( i = 0; i < m_CurrentColumns.Count() ; i++ )
					{
						// see if we are in this cell
						int cellX, cellY, w, t;
						GetCellBounds(row, i, cellX, cellY, w, t);
						if ( x >= cellX && x <= ( cellX + w ) )
						{
							iSelectedColumn = i;
							break;
						}
					}
				}

				// check for multi-select
				if ( input()->IsKeyDown(KEY_LSHIFT) || input()->IsKeyDown(KEY_RSHIFT) ) 
				{
					// no mulitple selecting of individual cells
					if ( m_bCanSelectIndividualCells )
					{
						if ( !m_SelectedItems.HasElement(itemID) )
						{
							ClearSelectedItems();
							AddSelectedItem(itemID);
							m_LastItemSelected = itemID;
						}
					}
					else	// deal with 'multiple' row selection
					{
						// convert the last item selected to a row so we can multiply select by rows NOT items
						int lastSelectedRow = m_VisibleItems.Find(m_LastItemSelected);
						int startRow, endRow;
						if ( row < lastSelectedRow )
						{
							startRow = row;
							endRow = lastSelectedRow;
						}
						else
						{
							startRow = lastSelectedRow;
							endRow = row;
						}
					
						// clear the selection if neither control key was down - we are going to readd ALL selected items
						// in case the user changed the 'direction' of the shift add
						if ( !input()->IsKeyDown(KEY_LCONTROL) && !input()->IsKeyDown(KEY_RCONTROL) )
						{
							ClearSelectedItems();
						}
	
						// add any items that we haven't added
						for (int i = startRow; i <= endRow; i++)
						{
							// get the item indexes for these rows
							int selectedItemID = m_VisibleItems[i];
							if ( !m_SelectedItems.HasElement(selectedItemID) )
							{
								AddSelectedItem(selectedItemID);
							}
						}
					}
				}
				// check for row-add select
				else if ( input()->IsKeyDown(KEY_LCONTROL) || input()->IsKeyDown(KEY_RCONTROL) )
				{
					// are we dealing with individual cell selection
					if ( m_bCanSelectIndividualCells )
					{
						// if we're already in this row, check if the columns match up
						if ( m_SelectedItems.HasElement(itemID) )
						{
							// we're ctrl selecting the same cell, clear it
							if ( m_iSelectedColumn == iSelectedColumn )
							{
								ClearSelectedItems();
							}
						}
						else
						{
							// new row, so it doesn't matter if the columns line up
							ClearSelectedItems();
							AddSelectedItem(itemID);
						}
					}
					else 	// dealing with row selection
					{
						if (m_SelectedItems.HasElement(itemID))
						{
							// this row is already selected, remove
							m_SelectedItems.FindAndRemove(itemID);
						}
						else
						{
							// add the row to the selection
							AddSelectedItem(itemID);
						}
					}
	
					m_LastItemSelected = itemID;
				}
				else	// no CTRL or SHIFT keys
				{
					// reset the selection Start point
					m_LastItemSelected = itemID;
					SetSingleSelectedItem(itemID);
				}
	
				// if we're selecting individual cells
				if ( m_SelectedItems.Count() == 1 && m_bCanSelectIndividualCells )
				{
					m_iSelectedColumn = iSelectedColumn;
					SetSelectedCell(itemID, m_iSelectedColumn);
				}
				else
				{
					Repaint();
				}	
			}
		}
		// get the key focus
		RequestFocus();
	}

	// check for context menu open
	if (code == MOUSE_RIGHT)
	{
		if (m_SelectedItems.Count() > 0)
		{
			PostActionSignal(new KeyValues("OpenContextMenu", "itemID", m_SelectedItems[0]));
		}
		else
		{
			// post it, but with the invalid row
			PostActionSignal(new KeyValues("OpenContextMenu", "itemID", -1));
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Scrolls the list according to the mouse wheel movement
//-----------------------------------------------------------------------------
void ListPanel::OnMouseWheeled(int delta)
{
	int val = m_vbar->GetValue();
	val -= (delta * 3);
	m_vbar->SetValue(val);
}

//-----------------------------------------------------------------------------
// Purpose: Double-click act like the the item under the mouse was selected
//			and then the enter key hit
//-----------------------------------------------------------------------------
void ListPanel::OnMouseDoublePressed(MouseCode code)
{
	if (code == MOUSE_LEFT)
	{
		// select the item
		OnMousePressed(code);

		// post up an enter key being hit if anything was selected
		if (GetSelectedItemsCount() > 0)
		{
			OnKeyCodeTyped(KEY_ENTER);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ListPanel::OnKeyCodeTyped(KeyCode code)
{
	if (m_VisibleItems.Count() == 0)
		return;

	// calculate info for adjusting scrolling
	int startitem = GetStartItem();
	float totalrows = (float) m_VisibleItems.Count();
	int rowsperpage = (int) GetRowsPerPage();

	int selectedRow = 0;
	if ( m_DataItems.IsValidIndex(m_LastItemSelected))
	{
		selectedRow = m_VisibleItems.Find(m_LastItemSelected);
	}

	switch (code)
	{
	case KEY_HOME:
		selectedRow = 0;
		break;

	case KEY_END:
		selectedRow = m_VisibleItems.Count() - 1;
		break;

	case KEY_PAGEUP:
		if (selectedRow <= startitem)
		{
			// move up a page
			selectedRow -= (rowsperpage - 1);
		}
		else
		{
			// move to the top of the current page
			selectedRow = startitem;
		}
		break;

	case KEY_PAGEDOWN:
		if (selectedRow >= (startitem + rowsperpage-1))
		{
			// move down a page
			selectedRow += (rowsperpage - 1);
		}
		else
		{
			// move to the bottom of the current page
			selectedRow = startitem + (rowsperpage - 1);
		}
		break;

	case KEY_UP:
		selectedRow -= 1;
		break;

	case KEY_DOWN:
		selectedRow += 1;
		break;

	case KEY_LEFT:
		if (m_bCanSelectIndividualCells && GetSelectedItemsCount() == 1 && m_iSelectedColumn >= 0)
		{
			m_iSelectedColumn--;
			if (m_iSelectedColumn < 0)
			{
				m_iSelectedColumn = 0;
			}
			break;
		}
		// fall through

	case KEY_RIGHT:
		if (m_bCanSelectIndividualCells && GetSelectedItemsCount() == 1 && m_iSelectedColumn >= 0)
		{
			m_iSelectedColumn++;
			if (m_iSelectedColumn >= m_CurrentColumns.Count())
			{
				m_iSelectedColumn = m_CurrentColumns.Count() - 1;
			}
			break;
		}
		// fall through

	default:
		// chain back
		BaseClass::OnKeyCodeTyped(code);
		return;
	};

	// make sure newly selected item is a valid range
	selectedRow = clamp(selectedRow, 0, m_VisibleItems.Count() - 1);

	m_LastItemSelected = m_VisibleItems[selectedRow];

	// make it so only one line is selected now
	SetSingleSelectedItem(m_LastItemSelected);

	// move the newly selected item to within the visible range
	if ( rowsperpage < totalrows )
	{
		int startitem = m_vbar->GetValue();
		if ( selectedRow < startitem )
		{
			// move the list back to match
			m_vbar->SetValue(selectedRow);
		}
		else if ( selectedRow >= startitem + rowsperpage )
		{
			// move list forward to match
			m_vbar->SetValue(selectedRow - rowsperpage + 1);
		}
	}

	// redraw
	InvalidateLayout();
	Repaint();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool ListPanel::GetCellBounds( int row, int col, int& x, int& y, int& wide, int& tall )
{
	if ( col < 0 || col >= m_CurrentColumns.Count() )
		return false;

	if ( row < 0 || row >= m_VisibleItems.Count() )
		return false;

	// Is row on screen?
	int startitem = GetStartItem();
	if ( row < startitem || row >= ( startitem + GetRowsPerPage() ) )
		return false;

	y = m_iTableStartY;
	y += ( row - startitem ) * m_iRowHeight;
	tall = m_iRowHeight;

	// Compute column cell
	x = m_iTableStartX;
	// walk columns
	int c = 0;
	while ( c < col)
	{
		x += m_ColumnsData[m_CurrentColumns[c]].m_pHeader->GetWide();
		c++;
	}
	wide = m_ColumnsData[m_CurrentColumns[c]].m_pHeader->GetWide();

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: returns true if any found, row and column are filled out
//-----------------------------------------------------------------------------
bool ListPanel::GetCellAtPos(int x, int y, int &row, int &col)
{
	// convert to local
	ScreenToLocal(x, y);

	// move to Start of table
	x -= m_iTableStartX;
	y -= m_iTableStartY;

	int startitem = GetStartItem();
	// make sure it's still in valid area
	if ( x >= 0 && y >= 0 )
	{
		// walk the rows (for when row height is independant each row)  
		// NOTE: if we do height independent rows, we will need to change GetCellBounds as well
		for ( row = startitem ; row < m_VisibleItems.Count() ; row++ )
		{
			if ( y < ( ( ( row - startitem ) + 1 ) * m_iRowHeight ) )
				break;
		}

		// walk columns
		int startx = 0;
		for ( col = 0 ; col < m_CurrentColumns.Count() ; col++ )
		{
			startx += m_ColumnsData[m_CurrentColumns[col]].m_pHeader->GetWide();

			if ( x < startx )
				break;
		}

		// make sure we're not out of range
		if ( ! ( row == m_VisibleItems.Count() || col == m_CurrentColumns.Count() ) )
		{
			return true;
		}
	}

	// out-of-bounds
	row = col = -1;
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ListPanel::ApplySchemeSettings(IScheme *pScheme)
{
	// force label to apply scheme settings now so we can override it
	m_pLabel->InvalidateLayout(true);

	BaseClass::ApplySchemeSettings(pScheme);

	SetBgColor(GetSchemeColor("WindowBgColor", pScheme));
	SetBorder(pScheme->GetBorder("ButtonDepressedBorder"));

	m_pLabel->SetBgColor(GetSchemeColor("Menu/ArmedBgColor", pScheme));

	SetBgColor(GetSchemeColor("ListBgColor", GetSchemeColor("WindowDisabledBgColor", pScheme), pScheme));

	m_LabelFgColor = GetSchemeColor("WindowFgColor", pScheme);
	m_SelectionFgColor = GetSchemeColor("ListSelectionFgColor", m_LabelFgColor, pScheme);
	m_pEmptyListText->SetColor(GetSchemeColor("LabelDimText", pScheme));
		
	SetFont( pScheme->GetFont("Default", IsProportional() ) );
	m_pEmptyListText->SetFont( pScheme->GetFont( "Default", IsProportional() ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ListPanel::SetSortFunc(int col, SortFunc *func)
{
	Assert(col < m_CurrentColumns.Count());
	unsigned char dataColumnIndex = m_CurrentColumns[col];

	if ( !m_ColumnsData[dataColumnIndex].m_bTypeIsText && func != NULL)
	{
		m_ColumnsData[dataColumnIndex].m_pHeader->SetMouseClickEnabled(MOUSE_LEFT, 1);
	}

	m_ColumnsData[dataColumnIndex].m_pSortFunc = func;

	// resort this column according to new sort func
    ResortColumnRBTree(col);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ListPanel::SetSortColumn(int column)
{
	m_iSortColumn = column;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ListPanel::SortList( void )
{
	m_bNeedsSort = false;

	if ( m_VisibleItems.Count() <= 1 )
	{
		return;
	}

	// check if the last selected item is on the screen - if so, we should try to maintain it on screen 
	int startItem = GetStartItem();
	int rowsperpage = (int) GetRowsPerPage();
	int screenPosition = -1;
	if ( m_LastItemSelected != -1 && m_SelectedItems.Count() > 0 )
	{
		int selectedItemRow = m_VisibleItems.Find(m_LastItemSelected);
		if ( selectedItemRow >= startItem && selectedItemRow <= ( startItem + rowsperpage ) )
		{
			screenPosition = selectedItemRow - startItem;
		}
	}

	// get the required sorting functions
	s_pCurrentSortingListPanel = this;

	// setup globals for use in qsort
	s_pSortFunc = FastSortFunc;
	s_bSortAscending = m_bSortAscending;
	s_pSortFuncSecondary = FastSortFunc;
	s_bSortAscendingSecondary = m_bSortAscendingSecondary;

	// walk the tree and set up the current indices
	if (m_CurrentColumns.IsValidIndex(m_iSortColumn))
	{
		IndexRBTree_t &rbtree = m_ColumnsData[m_CurrentColumns[m_iSortColumn]].m_SortedTree;
		unsigned int index = rbtree.FirstInorder();
		unsigned int lastIndex = rbtree.LastInorder();
		int prevDuplicateIndex = 0;
		int sortValue = 1;
		while (1)
		{
			FastSortListPanelItem *dataItem = (FastSortListPanelItem*) rbtree[index].dataItem;
			if (dataItem->visible)
			{
				// only increment the sort value if we're a different token from the previous
				if (!prevDuplicateIndex || prevDuplicateIndex != rbtree[index].duplicateIndex)
				{
					sortValue++;
				}
				dataItem->primarySortIndexValue = sortValue;
				prevDuplicateIndex = rbtree[index].duplicateIndex;
			}

			if (index == lastIndex)
				break;

			index = rbtree.NextInorder(index);
		}
	}

	// setup secondary indices
	if (m_CurrentColumns.IsValidIndex(m_iSortColumnSecondary))
	{
		IndexRBTree_t &rbtree = m_ColumnsData[m_CurrentColumns[m_iSortColumnSecondary]].m_SortedTree;
		unsigned int index = rbtree.FirstInorder();
		unsigned int lastIndex = rbtree.LastInorder();
		int sortValue = 1;
		int prevDuplicateIndex = 0;
		while (1)
		{
			FastSortListPanelItem *dataItem = (FastSortListPanelItem*) rbtree[index].dataItem;
			if (dataItem->visible)
			{
				// only increment the sort value if we're a different token from the previous
				if (!prevDuplicateIndex || prevDuplicateIndex != rbtree[index].duplicateIndex)
				{
					sortValue++;
				}
				dataItem->secondarySortIndexValue = sortValue;

				prevDuplicateIndex = rbtree[index].duplicateIndex;
			}

			if (index == lastIndex)
				break;

			index = rbtree.NextInorder(index);
		}
	}

	// quick sort the list
	qsort(m_VisibleItems.Base(), (size_t) m_VisibleItems.Count(), (size_t) sizeof(int), AscendingSortFunc);

	if ( screenPosition != -1 )
	{
		int selectedItemRow = m_VisibleItems.Find(m_LastItemSelected);

		// if we can put the last selected item in exactly the same spot, put it there, otherwise
		// we need to be at the top of the list
		if (selectedItemRow > screenPosition)
		{
			m_vbar->SetValue(selectedItemRow - screenPosition);
		}
		else
		{
			m_vbar->SetValue(0);
		}
	}

	InvalidateLayout();
	Repaint();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ListPanel::SetFont(HFont font)
{
	Assert( font );
	if ( !font )
		return;

	m_pTextImage->SetFont(font);
	m_iRowHeight = surface()->GetFontTall(font) + 2;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ListPanel::OnSliderMoved()
{
	InvalidateLayout();
	Repaint();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : deltax - deltas from current position
//-----------------------------------------------------------------------------
void ListPanel::OnColumnResized(int col, int delta)
{
	m_iColumnDraggerMoved = col;

	column_t& column = m_ColumnsData[m_CurrentColumns[col]];

	Panel *header = column.m_pHeader;
	int wide, tall;
	header->GetSize(wide, tall);


	wide += delta;

	// enforce minimum sizes for the header
	if ( wide < column.m_iMinWidth )
	{
		wide = column.m_iMinWidth;
	}
	// enforce maximum sizes for the header
	if ( wide > column.m_iMaxWidth )
	{
		wide = column.m_iMaxWidth;
	}

	// make sure we have enough space for the columns to our right
	int panelWide, panelTall;
	GetSize( panelWide, panelTall );
	int x, y;
	header->GetPos(x, y);
	int restColumnsMinWidth = 0;
	int i;
	for ( i = col+1 ; i < m_CurrentColumns.Count() ; i++ )
	{
		column_t& nextCol = m_ColumnsData[m_CurrentColumns[i]];
		restColumnsMinWidth += nextCol.m_iMinWidth;
	}
	panelWide -= ( x + restColumnsMinWidth + SCROLLBAR_SIZE + WINDOW_BORDER_WIDTH );
	if ( wide > panelWide )
	{
		wide = panelWide;
	}

	header->SetSize(wide, tall);

	// the adjacent header will be moved automatically in PerformLayout()
	header->InvalidateLayout();
	InvalidateLayout();
	Repaint();
}

//-----------------------------------------------------------------------------
// Purpose: sets which column we should sort with
//-----------------------------------------------------------------------------
void ListPanel::OnSetSortColumn(int column)
{
	// if it's the primary column already, flip the sort direction
	if (m_iSortColumn == column)
	{
		m_bSortAscending = !m_bSortAscending;
	}
	else
	{
		// switching sort columns, keep the old one as the secondary sort
		m_iSortColumnSecondary = m_iSortColumn;
		m_bSortAscendingSecondary = m_bSortAscending;
	}

	SetSortColumn(column);

	SortList();
}

//-----------------------------------------------------------------------------
// Purpose: sets whether the item is visible or not
//-----------------------------------------------------------------------------
void ListPanel::SetItemVisible(int itemID, bool state)
{
	if ( !m_DataItems.IsValidIndex(itemID) )
		return;

	FastSortListPanelItem *data = (FastSortListPanelItem*) m_DataItems[itemID];
	if (data->visible == state)
		return;

	m_bNeedsSort = true;

	data->visible = state;
	if (data->visible)
	{
		// add back to end of list
		m_VisibleItems.AddToTail(itemID);
	}
	else
	{
		// remove from selection if it is there.
		if (m_SelectedItems.HasElement(itemID))
		{
			m_SelectedItems.FindAndRemove(itemID);
		}

		// remove from data
		m_VisibleItems.FindAndRemove(itemID);
	
		InvalidateLayout();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Calculate number of rows per page 
//-----------------------------------------------------------------------------
float ListPanel::GetRowsPerPage()
{
	float rowsperpage = (float)( GetTall() - m_iHeaderHeight ) / (float)m_iRowHeight;
	return rowsperpage;
}

//-----------------------------------------------------------------------------
// Purpose: Calculate the item we should Start on
//-----------------------------------------------------------------------------
int ListPanel::GetStartItem()
{
	// if rowsperpage < total number of rows
	if ( GetRowsPerPage() < (float) m_VisibleItems.Count() )
	{
		return m_vbar->GetValue();
	}
	return 0;	// otherwise Start at top
}

//-----------------------------------------------------------------------------
// Purpose: whether or not to select specific cells (off by default)
//-----------------------------------------------------------------------------
void ListPanel::SetSelectIndividualCells(bool state)
{
	m_bCanSelectIndividualCells = state;
}

//-----------------------------------------------------------------------------
// Purpose: Sets the text which is displayed when the list is empty
//-----------------------------------------------------------------------------
void ListPanel::SetEmptyListText(const char *text)
{
	m_pEmptyListText->SetText(text);
	Repaint();
}

//-----------------------------------------------------------------------------
// Purpose: Sets the text which is displayed when the list is empty
//-----------------------------------------------------------------------------
void ListPanel::SetEmptyListText(const wchar_t *text)
{
	m_pEmptyListText->SetText(text);
	Repaint();
}

//-----------------------------------------------------------------------------
// Purpose: empty message map
//-----------------------------------------------------------------------------
MessageMapItem_t ListPanel::m_MessageMap[] =
{
	MAP_MESSAGE( ListPanel, "ScrollBarSliderMoved", OnSliderMoved ),
	MAP_MESSAGE_INT( ListPanel, "SetSortColumn", OnSetSortColumn, "column" ),
	MAP_MESSAGE_INT_INT( ListPanel, "ColumnResized", OnColumnResized, "column", "delta" ),
//	MAP_MESSAGE_INT_INT( ListPanel, "SetSelectedRow", SetSelectedRows, "startIndex", "endIndex" ),
};
IMPLEMENT_PANELMAP(ListPanel, Panel);


