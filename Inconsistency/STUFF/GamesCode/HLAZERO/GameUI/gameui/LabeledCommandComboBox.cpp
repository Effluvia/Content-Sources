//========= Copyright � 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include "LabeledCommandComboBox.h"
#include "EngineInterface.h"
#include <KeyValues.h>
#include <vgui/ILocalize.h>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

CLabeledCommandComboBox::CLabeledCommandComboBox( vgui::Panel *parent, const char *panelName,  const char* text ) : vgui::ComboBox( parent, panelName, 6, false )
{
	//MODDD - fitting, yes?
	//MODDD - NOTE.  This class has an ancestor, 'TextEntry'.  Does SetText need to be overridden to do anything 
	// special for this 'LabeledCommandComboBox' ?
	SetText(text);

	AddActionSignalTarget(this);
	m_iCurrentSelection = -1;
	m_iStartSelection = -1;
}

CLabeledCommandComboBox::~CLabeledCommandComboBox( void )
{
}

void CLabeledCommandComboBox::DeleteAllItems()
{
	BaseClass::DeleteAllItems();
	m_Items.RemoveAll();
}

void CLabeledCommandComboBox::AddItem( char const *text, char const *engineCommand )
{
	int idx = m_Items.AddToTail();
	COMMANDITEM *item = &m_Items[ idx ];

	item->comboBoxID = BaseClass::AddItem( text, NULL );

	strcpy( item->name, text );

	if (text[0] == '#')
	{
		// need to localize the string
		wchar_t *localized = vgui::localize()->Find(text);
		if (localized)
		{
			vgui::localize()->ConvertUnicodeToANSI(localized, item->name, sizeof(item->name));
		}
	}

	strcpy( item->command, engineCommand );
}

void CLabeledCommandComboBox::ActivateItem(int index)
{
	if ( index< m_Items.Count() )
	{
		int comboBoxID = m_Items[index].comboBoxID;
		BaseClass::ActivateItem(comboBoxID);
		m_iCurrentSelection = index;
	}
}

void CLabeledCommandComboBox::SetInitialItem(int index)
{
	if ( index< m_Items.Count() )
	{
		m_iStartSelection = index;
		int comboBoxID = m_Items[index].comboBoxID;
		ActivateItem(comboBoxID);
	}
}

void CLabeledCommandComboBox::OnTextChanged( char const *text )
{
	int i;
	for ( i = 0; i < m_Items.Size(); i++ )
	{
		COMMANDITEM *item = &m_Items[ i ];
		if ( !stricmp( item->name, text ) )
		{
		//	engine->pfnClientCmd( item->command );
			m_iCurrentSelection = i;
			break;
		}
	}

	if (HasBeenModified())
	{
		PostActionSignal(new KeyValues("ControlModified"));
	}
//	PostMessage( GetParent()->GetVPanel(), new vgui::KeyValues( "TextChanged", "text", text ) );
}

const char *CLabeledCommandComboBox::GetActiveItemCommand()
{
	if (m_iCurrentSelection == -1)
		return NULL;

	COMMANDITEM *item = &m_Items[ m_iCurrentSelection ];
	return item->command;
}

void CLabeledCommandComboBox::ApplyChanges()
{
	if (m_iCurrentSelection == -1)
		return;
	if (m_Items.Size() < 1)
		return;

	//MODDD - assert linker error
	//assert( m_iCurrentSelection < m_Items.Size() );

	COMMANDITEM *item = &m_Items[ m_iCurrentSelection ];
	engine->ClientCmd( item->command );
	m_iStartSelection = m_iCurrentSelection;
}

bool CLabeledCommandComboBox::HasBeenModified()
{
	return m_iStartSelection != m_iCurrentSelection;
}

void CLabeledCommandComboBox::Reset()
{
	if (m_iStartSelection != -1)
	{
		ActivateItem(m_iStartSelection);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Message mapping 
//-----------------------------------------------------------------------------
vgui::MessageMapItem_t CLabeledCommandComboBox::m_MessageMap[] =
{
	MAP_MESSAGE_CONSTCHARPTR( CLabeledCommandComboBox, "TextChanged", OnTextChanged, "text" ),	// custom message
};

IMPLEMENT_PANELMAP( CLabeledCommandComboBox, BaseClass );