#include "cbase.h"
#include <cdll_client_int.h>

#include "letters_dialog.h"

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <KeyValues.h>
#include <vgui_controls/ImageList.h>
#include <filesystem.h>

#include <vgui_controls/Button.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/ListPanel.h>
#include <vgui_controls/RichText.h>

#include "IGameUIFuncs.h" // for key bindings
#include <igameresources.h>

#include "c_basehlplayer.h"

#ifndef _XBOX
extern IGameUIFuncs *gameuifuncs; // for key binding details
#endif

#include <stdlib.h> // MAX_PATH define
#include <stdio.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CLettersDialog::CLettersDialog(IViewPort *pViewPort) : Frame(NULL, PANEL_LETTERS )
{
	m_pViewPort = pViewPort;
	m_iTabKey = BUTTON_CODE_INVALID; // this is looked up in ShowPanel()

	// initialize dialog
	SetTitle("", true);

	// load the new scheme early!!
	SetScheme("SourceScheme");

	SetMoveable(true);
	SetSizeable(true);
	SetTitleBarVisible( true );
	SetMenuButtonVisible( false );
	SetMinimizeButtonVisible( false );
	SetMaximizeButtonVisible( false );

	// Don't resize with screen resolution
	SetProportional(false);

	LoadControlSettings("Resource/UI/LettersDialog.res");
	InvalidateLayout();

#if 1
	m_pLetterText = dynamic_cast< RichText * >(FindChildByName( "LetterText" ));
	m_pListOfLetters = dynamic_cast< ListPanel * >(FindChildByName( "ListOfLetters" ));
#else
	m_pLetterText = new RichText( this, "LetterText" );
#endif

	if ( m_pListOfLetters != NULL )
	{
		// Tell us when a list item is selected
		m_pListOfLetters->AddActionSignalTarget( this );

		m_pListOfLetters->AddColumnHeader( 0, "title", "Letters", 50, ListPanel::COLUMN_RESIZEWITHWINDOW );
		m_pListOfLetters->SetColumnSortable( 0, false );
	}

	m_szLetterName[0] = 0;
	m_bFirstTime = true;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CLettersDialog::~CLettersDialog()
{
}

//-----------------------------------------------------------------------------
// Purpose: Recalculate the position of all items
//-----------------------------------------------------------------------------
void CLettersDialog::PerformLayout()
{
	// chain back
	BaseClass::PerformLayout();

	int wide, tall;
	GetSize(wide, tall);

	Button *pButton = dynamic_cast< Button * >(FindChildByName( "frame_close" ));
	if ( pButton && pButton->IsVisible() )
	{
		pButton->SetPos( wide - pButton->GetWide() /*- 6*/, 6 );
	}
}

//-----------------------------------------------------------------------------
// Purpose: sets the text color of the map description field
//-----------------------------------------------------------------------------
void CLettersDialog::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
//	m_pLetterText->SetFgColor( pScheme->GetColor("MapDescriptionText", Color(255, 255, 255, 0)) );

	HFont hFont = pScheme->GetFont( "Default" );
	if ( m_pLetterText )
		m_pLetterText->SetFont( hFont );
	if ( m_pListOfLetters )
		m_pListOfLetters->SetFont( hFont );

	if ( *m_szLetterName )
	{
//		LoadLetter( m_szLetterName ); // reload the map description to pick up the color
	}
}

//-----------------------------------------------------------------------------
// Purpose: shows the team menu
//-----------------------------------------------------------------------------
void CLettersDialog::ShowPanel(bool bShow)
{
	if ( BaseClass::IsVisible() == bShow )
		return;

	if ( bShow )
	{
		SetListOfLetters();

		if ( m_bFirstTime )
		{
			MoveToCenterOfScreen();
			m_bFirstTime = false;
		}

		Activate();

		if ( m_pLetterText != NULL )
			m_pLetterText->GotoTextStart();

		SetMouseInputEnabled( true );

		// get key bindings if shown
		// you need to lookup the tab key AFTER the engine has loaded
		// FIXME: how to detect if the binding changes?
		/*if ( m_iTabKey == BUTTON_CODE_INVALID )*/
		{
			m_iTabKey = gameuifuncs->GetButtonCodeForBind( "showscores" );
		}
	}
	else
	{
		SetVisible( false );
		SetMouseInputEnabled( false );
	}

	m_pViewPort->ShowBackGround( bShow );
}

//-----------------------------------------------------------------------------
void CLettersDialog::Close( void )
{
	gViewPortInterface->ShowPanel( PANEL_LETTERS, false );
}

//-----------------------------------------------------------------------------
// Purpose: updates the UI with a new map name and map html page, and sets up the team buttons
//-----------------------------------------------------------------------------
void CLettersDialog::SetListOfLetters()
{
	if ( m_pListOfLetters == NULL )
		return;

	m_pListOfLetters->RemoveAll();

	C_BaseHLPlayer *pPlayer = dynamic_cast<C_BaseHLPlayer *>(C_BasePlayer::GetLocalPlayer());
	if ( pPlayer == NULL )
		return;

	int itemID = -1;
	KeyValues *kv = new KeyValues("data");
	for ( int i = 0; pPlayer->m_HL2Local.m_iszLetterNames[i] != NULL_STRING; i++ )
	{
		kv->SetString("title", STRING( pPlayer->m_HL2Local.m_iszLetterNames[i]) );
		kv->SetString("file", STRING( pPlayer->m_HL2Local.m_iszLetterNames[i]) );
		itemID = m_pListOfLetters->AddItem( kv, 0, false, false );
	}
	kv->deleteThis();

	// Select the most-recent letter
	if ( itemID != -1 )
		m_pListOfLetters->SetSingleSelectedItem( itemID );
	else if ( m_pLetterText != NULL )
		m_pLetterText->SetText( "" );
}

//-----------------------------------------------------------------------------
// Purpose: read in resource/letters/xyz.txt
//-----------------------------------------------------------------------------
void CLettersDialog::LoadLetter( const char *letterName )
{
	if ( m_pLetterText == NULL )
		return;

	// Save off the map name so we can re-load the page in ApplySchemeSettings().
	Q_strncpy( m_szLetterName, letterName, strlen( letterName ) + 1 );

	m_pLetterText->SetVisible( true );
	
	char letterRES[ MAX_PATH ];

	char uilanguage[ 64 ];
	engine->GetUILanguage( uilanguage, sizeof( uilanguage ) );

	Q_snprintf( letterRES, sizeof( letterRES ), "resource/letters/%s_%s.txt", letterName, uilanguage );

	if ( !g_pFullFileSystem->FileExists( letterRES ) )
	{
		// try english
		Q_snprintf( letterRES, sizeof( letterRES ), "resource/letters/%s_english.txt", letterName );

		if ( !g_pFullFileSystem->FileExists( letterRES ) )
		{
			// try language-neutral
			Q_snprintf( letterRES, sizeof( letterRES ), "resource/letters/%s.txt", letterName );

			if ( !g_pFullFileSystem->FileExists( letterRES ) )
			{
				// fail
				m_pLetterText->SetText( "" );
				return; 
			}
		}
	}

	FileHandle_t f = g_pFullFileSystem->Open( letterRES, "r" );

	// read into a memory block
	int fileSize = g_pFullFileSystem->Size(f);
	int dataSize = fileSize + sizeof( wchar_t );
	if ( dataSize % 2 )
		++dataSize;
	wchar_t *memBlock = (wchar_t *)malloc(dataSize);
	memset( memBlock, 0x0, dataSize);
	int bytesRead = g_pFullFileSystem->Read(memBlock, fileSize, f);
	if ( bytesRead < fileSize )
	{
		// NULL-terminate based on the length read in, since Read() can transform \r\n to \n and
		// return fewer bytes than we were expecting.
		char *data = reinterpret_cast<char *>( memBlock );
		data[ bytesRead ] = 0;
		data[ bytesRead+1 ] = 0;
	}

	// null-terminate the stream (redundant, since we memset & then trimmed the transformed buffer already)
	memBlock[dataSize / sizeof(wchar_t) - 1] = 0x0000;

	// check the first character, make sure this a little-endian unicode file
	if (memBlock[0] != 0xFEFF)
	{
		// its a ascii char file
		// THIS TRUNCATES TEXT TO 1024 CHARS
		m_pLetterText->SetText( reinterpret_cast<char *>( memBlock ) );
	}
	else
	{
		m_pLetterText->SetText( memBlock+1 );
	}

	// FIXME: GotoTextStart() at this point (which is called within RichText::SetText) doesn't
	// go to the top of the text.
	g_pFullFileSystem->Close( f );
	free(memBlock);

//	InvalidateLayout();
//	Repaint();
}

void CLettersDialog::OnKeyCodePressed(KeyCode code)
{
	if ( m_iTabKey != BUTTON_CODE_INVALID && m_iTabKey == code )
	{
		gViewPortInterface->ShowPanel( PANEL_LETTERS, false );
	}
	else
	{
		BaseClass::OnKeyCodePressed( code );
	}
}

// Called when a list item is selected
void CLettersDialog::OnItemSelected( KeyValues *data )
{
	int numSelectedItems = m_pListOfLetters->GetSelectedItemsCount();
	if ( numSelectedItems > 0 )
	{
		int itemID = m_pListOfLetters->GetSelectedItem( 0 );
		if ( itemID != -1 )
		{
			KeyValues *kv = m_pListOfLetters->GetItem( itemID );
			if ( kv != NULL )
			{
				const char *letterName = kv->GetString( "file" );
				if ( letterName && letterName[0] )
				{
					LoadLetter( letterName );
					return;
				}
			}
		}
	}
	// FIXME: detect when no items are selected
	m_pLetterText->SetText( "" );
}