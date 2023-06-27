   //=========== (C) Copyright 2000 Valve, L.L.C. All rights reserved. ===========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Purpose: 
//=============================================================================

#include <windows.h>		// for WideCharToMultiByte and MultiByteToWideChar

#include <KeyValues.h>
#include "FileSystem.h"
#include <vstdlib/IKeyValuesSystem.h>

#include <Color.h>
#include <assert.h>
#include <stdlib.h>
#include <direct.h>			// for _mkdir
#include "tier0/mem.h"
#include "utlvector.h"
#include "utlbuffer.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

static char * s_LastFileLoadingFrom = "unknown"; // just needed for error messages

#define KEYVALUES_TOKEN_SIZE	1024

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
KeyValues::KeyValues( const char *setName )
{
	Init();
	SetName ( setName );
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
KeyValues::KeyValues( const char *setName, const char *firstKey, const char *firstValue )
{
	Init();
	SetName( setName );
	SetString( firstKey, firstValue );
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
KeyValues::KeyValues( const char *setName, const char *firstKey, const wchar_t *firstValue )
{
	Init();
	SetName( setName );
	SetWString( firstKey, firstValue );
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
KeyValues::KeyValues( const char *setName, const char *firstKey, int firstValue )
{
	Init();
	SetName( setName );
	SetInt( firstKey, firstValue );
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
KeyValues::KeyValues( const char *setName, const char *firstKey, const char *firstValue, const char *secondKey, const char *secondValue )
{
	Init();
	SetName( setName );
	SetString( firstKey, firstValue );
	SetString( secondKey, secondValue );
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
KeyValues::KeyValues( const char *setName, const char *firstKey, int firstValue, const char *secondKey, int secondValue )
{
	Init( );
	SetName( setName );
	SetInt( firstKey, firstValue );
	SetInt( secondKey, secondValue );
}

//-----------------------------------------------------------------------------
// Purpose: Initialize member variables
//-----------------------------------------------------------------------------
void KeyValues::Init()
{
	m_iKeyName = INVALID_KEY_SYMBOL;
	m_iDataType = TYPE_NONE;

	m_pSub = NULL;
	m_pPeer = NULL;
	m_pChain = NULL;

	m_sValue = NULL;
	m_wsValue = NULL;
	m_pValue = NULL;
	
	m_bHasEscapeSequences = false;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
KeyValues::~KeyValues()
{
	RemoveEverything();
}

//-----------------------------------------------------------------------------
// Purpose: remove everything
//-----------------------------------------------------------------------------
void KeyValues::RemoveEverything()
{
	KeyValues *dat;
	KeyValues *datNext = NULL;
	for ( dat = m_pSub; dat != NULL; dat = datNext )
	{
		datNext = dat->m_pPeer;
		dat->m_pPeer = NULL;
		delete dat;
	}

	for ( dat = m_pPeer; dat && dat != this; dat = datNext )
	{
		datNext = dat->m_pPeer;
		dat->m_pPeer = NULL;
		delete dat;
	}

	Init();	// reset all values
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *f - 
//-----------------------------------------------------------------------------

void KeyValues::RecursiveSaveToFile( CUtlBuffer& buf, int indentLevel )
{
	// write header
	WriteIndents( buf, indentLevel );
	buf.Printf( "\"%s\"\n", GetName() );
	WriteIndents( buf, indentLevel );
	buf.Printf( "{\n" );

	// loop through all our keys writing them to disk
	for ( KeyValues *dat = m_pSub; dat != NULL; dat = dat->m_pPeer )
	{
		if ( dat->m_pSub )
		{
			dat->RecursiveSaveToFile( buf, indentLevel + 1 );
		}
		else
		{
			// only write non-empty keys
			if ( dat->m_sValue && *(dat->m_sValue) )
			{
				WriteIndents( buf, indentLevel + 1 );
				buf.Printf( "\"%s\"\t\t\"%s\"\n", dat->GetName(), dat->m_sValue);
			}
		}
	}

	// write tail
	WriteIndents( buf, indentLevel );
	buf.Printf( "}\n" );
}

//-----------------------------------------------------------------------------
// Adds a chain... if we don't find stuff in this keyvalue, we'll look
// in the one we're chained to.
//-----------------------------------------------------------------------------

void KeyValues::ChainKeyValue( KeyValues* pChain )
{
	m_pChain = pChain;
}

//-----------------------------------------------------------------------------
// Purpose: Get the name of the current key section
//-----------------------------------------------------------------------------
const char *KeyValues::GetName( void )
{
	return KeyValuesSystem()->GetStringForSymbol(m_iKeyName);
}

//-----------------------------------------------------------------------------
// Purpose: Get the symbol name of the current key section
//-----------------------------------------------------------------------------
int KeyValues::GetNameSymbol()
{
	return m_iKeyName;
}

//-----------------------------------------------------------------------------
// Purpose: Read a single token from buffer (0 terminated)
//-----------------------------------------------------------------------------

const char * KeyValues::ReadToken( char **buffer, bool &wasQuoted )
{
	// if message text buffers go over this size
 	// change this value to make sure they will fit
 	// affects loading of last active chat window
	
 	static char buf[KEYVALUES_TOKEN_SIZE];

	if ( (*buffer) == NULL  || (**buffer) == 0)
		return NULL;  // read at least 1 character

	int bufC = 0;
	char c = 0;
	
	while ( true )	// eating white spaces and remarks loop
	{
		do // eat whitespaces
		{
			c = **buffer;
			(*buffer)++;
		}
		while ( (c > 0) && (c <= ' ') ); // while whitespaces
		
		if ( !c )
			return NULL;	// file ends after reading whitespaces

		// stop if it's not a comment
		if ( c != '/' )
		{
			break;	// new token starts here
		}
		else
		{
			c = **buffer;	// read next char
			(*buffer)++;

			if ( c != '/' )
			{
				// if not comments, undo move and start reading token
				(*buffer)--;
				c = '/';
				break;	// a real token starts here
				 
			}
			else
			{
				while (c > 0 && c != '\n') // read complete line
				{
					c = **buffer;
					(*buffer)++;
				}
			}
		}
	}

	// read quoted strings specially
	if ( c == '\"' )
	{
		wasQuoted = true;

		// read the token till we hit the endquote
		while ( true )
		{
			c = **buffer;
			(*buffer)++;

			if ( c == 0 )
			{
				// nope
				//DevMsg(1, "KeyValues::ReadToken unexpected EOF in quoted string in file %s.\n", s_LastFileLoadingFrom );
				return NULL;
			}

			if ( c == '\"'  )	// stop, when reading another quote
				break;

			// check for special chars
			if ( c == '\\' && m_bHasEscapeSequences )
			{
				// get the next char
				c = **buffer;
				(*buffer)++;

				switch ( c )
				{
					case 0  :	//nope //DevMsg(1, "KeyValues::ReadToken unexpected EOF after \\ in file %s.\n", s_LastFileLoadingFrom);
								return NULL;
					case 'n':	c = '\n'; break;	// add carriage return
					case '\\':	c = '\\'; break; // add backslash
					case 't':	c = '\t'; break; // add tabulator
					case '\"':	c = '\"'; break; // add quotes
					default:   
								//nope DevMsg(1, "KeyValues::ReadToken illegal character after \\ in file %s.\n", s_LastFileLoadingFrom);
								c = c;    break; // unknown, just add char
				}
			}

			if (bufC < (KEYVALUES_TOKEN_SIZE-1) )
			{
				buf[bufC++] = c;	// add char to buffer
			}
			else
			{
				// nope
				//DevMsg(1, "KeyValues::ReadToken overflow (>%i) in file %s.\n", KEYVALUES_TOKEN_SIZE, s_LastFileLoadingFrom );
			}
		}
	}
	else if ( c == '{' || c == '}' )
	{
		// it's a control char, just add this one char and stop reading
		wasQuoted = false;
		buf[bufC++] = c;	// add char to buffer
	}
	else
	{
		// read in the token until we hit a whitespace

		wasQuoted = false;

		while ( true )
		{
			if ( c == 0 )	// end of file
				break;

			if ( c == '"' || c == '{' || c == '}' )
				break;	// break if any control character appears in non quoted tokens

			if ( c <= ' ' )
				break;

			if (bufC < (KEYVALUES_TOKEN_SIZE-1) )
			{
				buf[bufC++] = c;	// add char to buffer
			}
			else
			{
				// nope
				//DevMsg(1, "KeyValues::ReadToken overflow (>%i) in file %s.\n", KEYVALUES_TOKEN_SIZE, s_LastFileLoadingFrom );
			}

			c = **buffer;
			(*buffer)++;
		};

		(*buffer)--;	// go back last white space or 0
	}

	// if empty token return empty token
	if ( bufC == 0 )
		return "";

	buf[bufC] = 0;	// terminate token string
	return buf;
}

//-----------------------------------------------------------------------------
// Purpose: if parser should translate escape sequences ( /n, /t etc), set to true
//-----------------------------------------------------------------------------

void KeyValues::UsesEscapeSequences(bool state)
{
	m_bHasEscapeSequences = state;
}

//-----------------------------------------------------------------------------
// Purpose: Load keyValues from disk
//-----------------------------------------------------------------------------

//MODDD - testy test
#include "../filesystem/filesystem_stdio/BaseFileSystem.h"

bool KeyValues::LoadFromFile( IBaseFileSystem *filesystem, const char *resourceName, const char *pathID )
{
	//MODDD - assert linker error
	//assert(filesystem);
	//assert(_heapchk() == _HEAPOK);

	FileHandle_t f = filesystem->Open(resourceName, "rb", pathID);
	if (!f)
		return false;

	s_LastFileLoadingFrom = (char*)resourceName;


	/*
	//MODDD.
	CFileHandle *fh = ( CFileHandle *)f;
	if ( !fh )
	{
		return 0;
	}
	if ( !fh->m_pFile )
	{
		return 0;
	}

	return fh->m_nLength;
	*/


	
	// load file into a null-terminated buffer
	int fileSize = filesystem->Size(f);
	char *buffer = (char*)MemAllocScratch(fileSize + 1);
	
	Assert(buffer);
	
	filesystem->Read(buffer, fileSize, f); // read into local buffer

	buffer[fileSize] = 0; // null terminate file as EOF

	filesystem->Close( f );	// close file after reading

	bool retOK = LoadFromBuffer( resourceName, buffer, filesystem );

	MemFreeScratch();

	return retOK;
}

//-----------------------------------------------------------------------------
// Purpose: Save the keyvalues to disk
//			Creates the path to the file if it doesn't exist 
//-----------------------------------------------------------------------------
bool KeyValues::SaveToFile( IBaseFileSystem *filesystem, const char *resourceName, const char *pathID )
{
	// create a write file
	FileHandle_t f = filesystem->Open(resourceName, "wb", pathID);

	// if the file won't create, make sure the paths are all created
	if (!f)
	{
		const char *currentFileName = resourceName;
		char szBuf[KEYVALUES_TOKEN_SIZE] = "";

		// Creates any necessary paths to create the file
		while (1)
		{
			const char *fileSlash = strchr(currentFileName, '\\');
			if (!fileSlash)
				break;

			// copy out the first string
			int pathSize = fileSlash - currentFileName;
			
			if (szBuf[0] != 0)
			{
				Q_strcat(szBuf, "\\");
			}

			Q_strncat(szBuf, currentFileName, pathSize);

			// make sure the first path is created
			_mkdir(szBuf);

			// increment the parse point to create the next directory (if any)
			currentFileName += (pathSize + 1);
		}

		// try opening the file again
		f = filesystem->Open(resourceName, "wb", pathID);
	}

	if (!f)
		return false;

	RecursiveSaveToFile(filesystem, f, 0);
	filesystem->Close(f);

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Write out a set of indenting
//-----------------------------------------------------------------------------
void KeyValues::WriteIndents( IBaseFileSystem *filesystem, FileHandle_t f, int indentLevel )
{
	for ( int i = 0; i < indentLevel; i++ )
	{
		filesystem->Write("\t", 1, f);
	}
}

//-----------------------------------------------------------------------------
// Purpose: writes out a set of indenting
// Input  : indentLevel - 
//-----------------------------------------------------------------------------

void KeyValues::WriteIndents( CUtlBuffer& buf, int indentLevel )
{
	for ( int i = 0; i < indentLevel; i++ )
	{
		buf.Printf( "\t" );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Write out a string where we convert the double quotes to backslash double quote
//-----------------------------------------------------------------------------
void KeyValues::WriteConvertedString( IBaseFileSystem *filesystem, FileHandle_t f, const char *pszString )
{
	// handle double quote chars within the string
	// the worst possible case is that the whole string is quotes
	int len = Q_strlen(pszString);

	//MODDD - avoid alloc
	//char *convertedString = (char *) _alloca ((len + 1)  * sizeof(char) * 2);
	char *convertedString = (char *) malloc ((len + 1)  * sizeof(char) * 2);

	int j=0;
	for (int i=0; i <= len; i++)
	{
		if (pszString[i] == '\"')
		{
			convertedString[j] = '\\';
			j++;
		}	
		convertedString[j] = pszString[i];
		j++;
	}		

	filesystem->Write(convertedString, strlen(convertedString), f);
}

//-----------------------------------------------------------------------------
// Purpose: Save keyvalues from disk, if subkey values are detected, calls
//			itself to save those
//-----------------------------------------------------------------------------
void KeyValues::RecursiveSaveToFile( IBaseFileSystem *filesystem, FileHandle_t f, int indentLevel )
{
	// write header
	WriteIndents( filesystem, f, indentLevel );
	filesystem->Write("\"", 1, f);
	filesystem->Write(GetName(), strlen(GetName()), f);
	filesystem->Write("\"\n", 2, f);
	WriteIndents( filesystem, f, indentLevel );
	filesystem->Write("{\n", 2, f);

	// loop through all our keys writing them to disk
	for ( KeyValues *dat = m_pSub; dat != NULL; dat = dat->m_pPeer )
	{
		if ( dat->m_pSub )
		{
			dat->RecursiveSaveToFile( filesystem, f, indentLevel + 1 );
		}
		else
		{
			// only write non-empty keys

			switch (dat->m_iDataType)
			{
			case TYPE_STRING:
				{
					if (dat->m_sValue && *(dat->m_sValue))
					{
						WriteIndents(filesystem, f, indentLevel + 1);
						filesystem->Write("\"", 1, f);
						filesystem->Write(dat->GetName(), Q_strlen(dat->GetName()), f);
						filesystem->Write("\"\t\t\"", 4, f);

						WriteConvertedString(filesystem, f, dat->m_sValue);	

						filesystem->Write("\"\n", 2, f);
					}
					break;
				}
			case TYPE_WSTRING:
				{
					if ( dat->m_wsValue )
					{
						static char buf[KEYVALUES_TOKEN_SIZE];
						// make sure we have enough space

						//MODDD - assert linker error
						//assert(::WideCharToMultiByte(CP_UTF8, 0, dat->m_wsValue, -1, NULL, 0, NULL, NULL) < KEYVALUES_TOKEN_SIZE);
						
						int result = ::WideCharToMultiByte(CP_UTF8, 0, dat->m_wsValue, -1, buf, KEYVALUES_TOKEN_SIZE, NULL, NULL);
						if (result)
						{
							WriteIndents(filesystem, f, indentLevel + 1);
							filesystem->Write("\"", 1, f);
							filesystem->Write(dat->GetName(), Q_strlen(dat->GetName()), f);
							filesystem->Write("\"\t\t\"", 4, f);

							WriteConvertedString(filesystem, f, buf);

							filesystem->Write("\"\n", 2, f);
						}
					}
					break;
				}

			case TYPE_INT:
				{
					WriteIndents(filesystem, f, indentLevel + 1);
					filesystem->Write("\"", 1, f);
					filesystem->Write(dat->GetName(), Q_strlen(dat->GetName()), f);
					filesystem->Write("\"\t\t\"", 4, f);

					char buf[32];
					Q_snprintf(buf, 32, "%d", dat->m_iValue);

					filesystem->Write(buf, Q_strlen(buf), f);
					filesystem->Write("\"\n", 2, f);
					break;
				}

			case TYPE_FLOAT:
				{
					WriteIndents(filesystem, f, indentLevel + 1);
					filesystem->Write("\"", 1, f);
					filesystem->Write(dat->GetName(), Q_strlen(dat->GetName()), f);
					filesystem->Write("\"\t\t\"", 4, f);

					char buf[48];
					Q_snprintf(buf, 48, "%f", dat->m_flValue);

					filesystem->Write(buf, Q_strlen(buf), f);
					filesystem->Write("\"\n", 2, f);
					break;
				}
			case TYPE_COLOR:
				// nope
				//DevMsg(1, "KeyValues::RecursiveSaveToFile: TODO, missing code for TYPE_COLOR.\n");
				break;

			default:
				break;
			}
		}
	}

	// write tail
	WriteIndents(filesystem, f, indentLevel);
	filesystem->Write("}\n", 2, f);
}

//-----------------------------------------------------------------------------
// Purpose: looks up a key by symbol name
//-----------------------------------------------------------------------------
KeyValues *KeyValues::FindKey(int keySymbol)
{
	for (KeyValues *dat = m_pSub; dat != NULL; dat = dat->m_pPeer)
	{
		if (dat->m_iKeyName == keySymbol)
			return dat;
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Find a keyValue, create it if it is not found.
//			Set bCreate to true to create the key if it doesn't already exist 
//			(which ensures a valid pointer will be returned)
//-----------------------------------------------------------------------------
KeyValues *KeyValues::FindKey(const char *keyName, bool bCreate)
{
	// return the current key if a NULL subkey is asked for
	if (!keyName || !keyName[0])
		return this;

	// look for '/' characters deliminating sub fields
	char szBuf[256];
	//MODDD - strstr now returns const char*, adjusted
	const char *subStr = strchr(keyName, '/');
	const char *searchStr = keyName;

	// pull out the substring if it exists
	if (subStr)
	{
		int size = subStr - keyName;
		Q_memcpy( szBuf, keyName, size );
		szBuf[size] = 0;
		searchStr = szBuf;
	}

	// lookup the symbol for the search string
	HKeySymbol iSearchStr = KeyValuesSystem()->GetSymbolForString(searchStr);

	KeyValues *lastItem = NULL;
	KeyValues *dat;
	// find the searchStr in the current peer list
	for (dat = m_pSub; dat != NULL; dat = dat->m_pPeer)
	{
		lastItem = dat;	// record the last item looked at (for if we need to append to the end of the list)

		// symbol compare
		if (dat->m_iKeyName == iSearchStr)
		{
			break;
		}
	}

	if ( !dat && m_pChain )
	{
		dat = m_pChain->FindKey(keyName, false);
	}

	// make sure a key was found
	if (!dat)
	{
		if (bCreate)
		{
			// we need to create a new key
			dat = new KeyValues( searchStr );
//			assert(dat != NULL);

			// insert new key at end of list
			if (lastItem)
			{
				lastItem->m_pPeer = dat;
			}
			else
			{
				m_pSub = dat;
			}
			dat->m_pPeer = NULL;

			// a key graduates to be a submsg as soon as it's m_pSub is set
			// this should be the only place m_pSub is set
			m_iDataType = TYPE_NONE;
		}
		else
		{
			return NULL;
		}
	}
	
	// if we've still got a subStr we need to keep looking deeper in the tree
	if ( subStr )
	{
		// recursively chain down through the paths in the string
		return dat->FindKey(subStr + 1, bCreate);
	}

	return dat;
}

//-----------------------------------------------------------------------------
// Purpose: Create a new key, with an autogenerated name.  
//			Name is guaranteed to be an integer, of value 1 higher than the highest 
//			other integer key name
//-----------------------------------------------------------------------------
KeyValues *KeyValues::CreateNewKey()
{
	int newID = 1;

	// search for any key with higher values
	for (KeyValues *dat = m_pSub; dat != NULL; dat = dat->m_pPeer)
	{
		// case-insensitive string compare
		int val = atoi(dat->GetName());
		if (newID <= val)
		{
			newID = val + 1;
		}
	}

	char buf[12];
	itoa(newID, buf, 10);

	return CreateKey( buf );
}

//-----------------------------------------------------------------------------
// Create a key
//-----------------------------------------------------------------------------

KeyValues* KeyValues::CreateKey( const char *keyName )
{
	// key wasn't found so just create a new one
	KeyValues* dat = new KeyValues( keyName );

	dat->UsesEscapeSequences( m_bHasEscapeSequences ); // use same format as parent does
	
	// add into subkey list
	if ( m_pSub == NULL )
	{
		m_pSub = dat;
	}
	else
	{
		KeyValues *pTempDat = m_pSub;
		while ( pTempDat->GetNextKey() != NULL )
		{
			pTempDat = pTempDat->GetNextKey();
		}

		pTempDat->SetNextKey( dat );
	}

	return dat;
}

//-----------------------------------------------------------------------------
// Purpose: Remove a subkey from the list
//-----------------------------------------------------------------------------
void KeyValues::RemoveSubKey(KeyValues *subKey)
{
	if (!subKey)
		return;

	// check the list pointer
	if (m_pSub == subKey)
	{
		m_pSub = subKey->m_pPeer;
	}
	else
	{
		// look through the list
		KeyValues *kv = m_pSub;
		while (kv->m_pPeer)
		{
			if (kv->m_pPeer == subKey)
			{
				kv->m_pPeer = subKey->m_pPeer;
				break;
			}
			
			kv = kv->m_pPeer;
		}
	}

	subKey->m_pPeer = NULL;
}



//-----------------------------------------------------------------------------
// Purpose: Return the first subkey in the list
//-----------------------------------------------------------------------------
KeyValues *KeyValues::GetFirstSubKey()
{
	return m_pSub;
}

//-----------------------------------------------------------------------------
// Purpose: Return the next subkey
//-----------------------------------------------------------------------------
KeyValues *KeyValues::GetNextKey()
{
	return m_pPeer;
}

//-----------------------------------------------------------------------------
// Purpose: Sets this key's peer to the KeyValues passed in
//-----------------------------------------------------------------------------
void KeyValues::SetNextKey( KeyValues *pDat )
{
	m_pPeer = pDat;
}

//-----------------------------------------------------------------------------
// Purpose: Get the integer value of a keyName. Default value is returned
//			if the keyName can't be found.
//-----------------------------------------------------------------------------
int KeyValues::GetInt( const char *keyName, int defaultValue )
{
	KeyValues *dat = FindKey( keyName, false );
	if ( dat )
	{
		switch ( dat->m_iDataType )
		{
		case TYPE_STRING:
			return atoi(dat->m_sValue);
		case TYPE_WSTRING:
			return _wtoi(dat->m_wsValue);
		case TYPE_FLOAT:
			return (int)dat->m_flValue;
		case TYPE_INT:
		case TYPE_PTR:
		default:
			return dat->m_iValue;
		};
	}
	return defaultValue;
}

//-----------------------------------------------------------------------------
// Purpose: Get the pointer value of a keyName. Default value is returned
//			if the keyName can't be found.
//-----------------------------------------------------------------------------
void *KeyValues::GetPtr( const char *keyName, void *defaultValue )
{
	KeyValues *dat = FindKey( keyName, false );
	if ( dat )
	{
		switch ( dat->m_iDataType )
		{
		case TYPE_PTR:
			return dat->m_pValue;

		case TYPE_WSTRING:
		case TYPE_STRING:
		case TYPE_FLOAT:
		case TYPE_INT:
		default:
			return NULL;
		};
	}
	return defaultValue;
}

//-----------------------------------------------------------------------------
// Purpose: Get the float value of a keyName. Default value is returned
//			if the keyName can't be found.
//-----------------------------------------------------------------------------
float KeyValues::GetFloat( const char *keyName, float defaultValue )
{
	KeyValues *dat = FindKey( keyName, false );
	if ( dat )
	{
		switch ( dat->m_iDataType )
		{
		case TYPE_STRING:
			return (float)atof(dat->m_sValue);
		case TYPE_WSTRING:
			return 0.0f;		// no wtof
		case TYPE_FLOAT:
			return dat->m_flValue;
		case TYPE_INT:
			return (float)dat->m_iValue;
		case TYPE_PTR:
		default:
			return 0.0f;
		};
	}
	return defaultValue;
}

//-----------------------------------------------------------------------------
// Purpose: Get the string pointer of a keyName. Default value is returned
//			if the keyName can't be found.
//-----------------------------------------------------------------------------
const char *KeyValues::GetString( const char *keyName, const char *defaultValue )
{
	KeyValues *dat = FindKey( keyName, false );
	if ( dat )
	{
		// convert the data to string form then return it
		char buf[64];
		switch ( dat->m_iDataType )
		{
		case TYPE_FLOAT:
			Q_snprintf( buf, 64, "%f", dat->m_flValue );
			SetString( keyName, buf );
			break;
		case TYPE_INT:
		case TYPE_PTR:
			Q_snprintf( buf, 64, "%d", dat->m_iValue );
			SetString( keyName, buf );
			break;
		case TYPE_WSTRING:
		{
			// convert the string to char *, set it for future use, and return it
			static char buf[512];
			int result = ::WideCharToMultiByte(CP_UTF8, 0, dat->m_wsValue, -1, buf, 512, NULL, NULL);
			if ( result )
			{
				SetString( keyName, buf );
			}
			else
			{
				return defaultValue;
			}
			break;
		}
		case TYPE_STRING:
			break;
		default:
			return defaultValue;
		};
		
		return dat->m_sValue;
	}
	return defaultValue;
}

const wchar_t *KeyValues::GetWString( const char *keyName, const wchar_t *defaultValue)
{
	//MODDD - CRITICAL CRITICAL CRITICAL.
	// Unsure how to replace 'swprintf' for now, looks like that came after VS6.
	// Maybe some 'printf' that prints each widechar in halves:
	// first half:  wchar & 00FF
	// second half: wchar & FF00

	// Basic demo:
	//
	//int i;
	//int sizeo;
	//char buf[32];
    //const wchar_t* someWideStr = L"some wide text";
    //
    //i = 0;
    //while(true){
    //    //take each widechar in two pieces
    //    wchar_t thisChar = someWideStr[i];
    //    buf[i * 2] = thisChar & 0x00FF;
    //    buf[i * 2 + 1] = thisChar & 0xFF00;
    //    if(thisChar == L'\0'){
    //        // terminating widechar?  end
    //        break;
    //    }
    //    i++;
    //}
    //sizeo = i;
    //
    ///*
    //// ALT TO WHILE LOOP:
    //sizeo = wcslen(someWideStr);
    //for(i = 0; i < sizeo; i++){
    //    //take each widechar in two pieces
    //    wchar_t thisChar = someWideStr[i];
    //    buf[i * 2] = thisChar & 0x00FF;
    //    buf[i * 2 + 1] = thisChar & 0xFF00;
    //    if(thisChar == L'\0'){
    //        // terminating widechar?  end
    //        break;
    //    }
    //}
    //// force to terminating
    //buf[sizeo*2] = 0;
    //buf[sizeo*2+1] = 0;
    //*/

	//// see the result.
    ////printf(buf);
    //wprintf((const wchar_t*)buf);
    //printf("\n");

    ////sizeo = wcslen((const wchar_t*)buf);
    //// times 2, because each widechar is two characters
    //for(i = 0; i < sizeo*2; i++){
    //    printf("i:%02d  -  %c (%d)\n", i, (int)buf[i], buf[i]);
    //}



	/*
	KeyValues *dat = FindKey( keyName, false );
	if ( dat )
	{
		wchar_t wbuf[64];
		switch ( dat->m_iDataType )
		{
		case TYPE_FLOAT:
			swprintf(wbuf, L"%f", dat->m_flValue);
			SetWString( keyName, wbuf);
			break;
		case TYPE_INT:
		case TYPE_PTR:
			swprintf( wbuf, L"%d", dat->m_iValue );
			SetWString( keyName, wbuf );
			break;
		case TYPE_WSTRING:
			break;
		case TYPE_STRING:
		{
			static wchar_t wbuftemp[512]; // convert to wide	
			int result = ::MultiByteToWideChar(CP_UTF8, 0, dat->m_sValue, -1, wbuftemp, 512);
			if ( result )
			{
				SetWString( keyName, wbuftemp);
			}
			else
			{
				return defaultValue;
			}
			break;
		}
		default:
			return defaultValue;
		};
		
		return (const wchar_t* )dat->m_wsValue;
	}
	return defaultValue;
	*/
	return 0;
}

//-----------------------------------------------------------------------------
// Purpose: Gets a color
//-----------------------------------------------------------------------------
Color KeyValues::GetColor( const char *keyName )
{
	Color color(0, 0, 0, 0);
	KeyValues *dat = FindKey( keyName, false );
	if ( dat )
	{
		if ( dat->m_iDataType == TYPE_COLOR )
		{
			color[0] = dat->m_Color[0];
			color[1] = dat->m_Color[1];
			color[2] = dat->m_Color[2];
			color[3] = dat->m_Color[3];
		}
		else if ( dat->m_iDataType == TYPE_FLOAT )
		{
			color[0] = dat->m_flValue;
		}
		else if ( dat->m_iDataType == TYPE_INT )
		{
			color[0] = dat->m_iValue;
		}
		else if ( dat->m_iDataType == TYPE_STRING )
		{
			// parse the colors out of the string
			float a, b, c, d;
			sscanf(dat->m_sValue, "%f %f %f %f", &a, &b, &c, &d);
			color[0] = (unsigned char)a;
			color[1] = (unsigned char)b;
			color[2] = (unsigned char)c;
			color[3] = (unsigned char)d;
		}
	}
	return color;
}

//-----------------------------------------------------------------------------
// Purpose: Sets a color
//-----------------------------------------------------------------------------
void KeyValues::SetColor( const char *keyName, Color value)
{
	KeyValues *dat = FindKey( keyName, true );

	if ( dat )
	{
		dat->m_iDataType = TYPE_COLOR;
		dat->m_Color[0] = value[0];
		dat->m_Color[1] = value[1];
		dat->m_Color[2] = value[2];
		dat->m_Color[3] = value[3];
	}
}

//-----------------------------------------------------------------------------
// Purpose: Set the string value of a keyName. 
//-----------------------------------------------------------------------------
void KeyValues::SetString( const char *keyName, const char *value )
{
	KeyValues *dat = FindKey( keyName, true );

	if ( dat )
	{
		// delete the old value
		delete [] dat->m_sValue;
		// make sure we're not storing the WSTRING  - as we're converting over to STRING
		delete [] dat->m_wsValue;
		dat->m_wsValue = NULL;

		if (!value)
		{
			// ensure a valid value
			value = "";
		}

		// allocate memory for the new value and copy it in
		int len = Q_strlen( value );
		dat->m_sValue = new char[len + 1];
		Q_memcpy( dat->m_sValue, value, len+1 );

		dat->m_iDataType = TYPE_STRING;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Set the string value of a keyName. 
//-----------------------------------------------------------------------------
void KeyValues::SetWString( const char *keyName, const wchar_t *value )
{
	KeyValues *dat = FindKey( keyName, true );

	if ( dat )
	{
		// delete the old value
		delete[] dat->m_wsValue;
		// make sure we're not storing the STRING  - as we're converting over to WSTRING
		delete [] dat->m_sValue;
		dat->m_sValue = NULL;

		if (!value)
		{
			// ensure a valid value
			value = L"";
		}

		// allocate memory for the new value and copy it in
		int len = wcslen( value );
		dat->m_wsValue = new wchar_t[len + 1];
		Q_memcpy( dat->m_wsValue, value, (len+1) * sizeof(wchar_t) );

		dat->m_iDataType = TYPE_WSTRING;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Set the integer value of a keyName. 
//-----------------------------------------------------------------------------
void KeyValues::SetInt( const char *keyName, int value )
{
	KeyValues *dat = FindKey( keyName, true );

	if ( dat )
	{
		dat->m_iValue = value;
		dat->m_iDataType = TYPE_INT;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Set the float value of a keyName. 
//-----------------------------------------------------------------------------
void KeyValues::SetFloat( const char *keyName, float value )
{
	KeyValues *dat = FindKey( keyName, true );

	if ( dat )
	{
		dat->m_flValue = value;
		dat->m_iDataType = TYPE_FLOAT;
	}
}

void KeyValues::SetName( const char * setName )
{
	m_iKeyName = KeyValuesSystem()->GetSymbolForString( setName );
}

//-----------------------------------------------------------------------------
// Purpose: Set the pointer value of a keyName. 
//-----------------------------------------------------------------------------
void KeyValues::SetPtr( const char *keyName, void *value )
{
	KeyValues *dat = FindKey( keyName, true );

	if ( dat )
	{
		dat->m_pValue = value;
		dat->m_iDataType = TYPE_PTR;
	}
}

void KeyValues::RecursiveCopyKeyValues( KeyValues& src )
{
	// garymcthack - need to check this code for possible buffer overruns.
	
	m_iKeyName = src.GetNameSymbol();

	if( !src.m_pSub )
	{
		m_iDataType = src.m_iDataType;
		char buf[256];
		switch( src.m_iDataType )
		{
		case TYPE_NONE:
			break;
		case TYPE_STRING:
			if( src.m_sValue )
			{
				m_sValue = new char[Q_strlen(src.m_sValue) + 1];
				Q_strcpy( m_sValue, src.m_sValue );
			}
			break;
		case TYPE_INT:
			m_iValue = src.m_iValue;
			Q_snprintf( buf,sizeof(buf), "%d", m_iValue );
			m_sValue = new char[strlen(buf) + 1];
			Q_strcpy( m_sValue, buf );
			break;
		case TYPE_FLOAT:
			m_flValue = src.m_flValue;
			Q_snprintf( buf,sizeof(buf), "%f", m_flValue );
			m_sValue = new char[strlen(buf) + 1];
			Q_strcpy( m_sValue, buf );
			break;
		case TYPE_PTR:
			m_pValue = src.m_pValue;
			break;
		case TYPE_COLOR:
			m_Color[0] = src.m_Color[0];
			m_Color[1] = src.m_Color[1];
			m_Color[2] = src.m_Color[2];
			m_Color[3] = src.m_Color[3];
			break;
			
		default:
			// do nothing . .what the heck is this?
			Assert( 0 );
			break;
		}

	}
#if 0
	KeyValues *pDst = this;
	for ( KeyValues *pSrc = src.m_pSub; pSrc; pSrc = pSrc->m_pPeer )
	{
		if ( pSrc->m_pSub )
		{
			pDst->m_pSub = new KeyValues( pSrc->m_pSub->getName() );
			pDst->m_pSub->RecursiveCopyKeyValues( *pSrc->m_pSub );
		}
		else
		{
			// copy non-empty keys
			if ( pSrc->m_sValue && *(pSrc->m_sValue) )
			{
				pDst->m_pPeer = new KeyValues( 
			}
		}
	}
#endif

	// Handle the immediate child
	if( src.m_pSub )
	{
		m_pSub = new KeyValues( NULL );
		m_pSub->RecursiveCopyKeyValues( *src.m_pSub );
	}

	// Handle the immediate peer
	if( src.m_pPeer )
	{
		m_pPeer = new KeyValues( NULL );
		m_pPeer->RecursiveCopyKeyValues( *src.m_pPeer );
	}
}

KeyValues& KeyValues::operator=( KeyValues& src )
{
	RemoveEverything();
	RecursiveCopyKeyValues( src );
	return *this;
}

//-----------------------------------------------------------------------------
// Purpose: Makes a copy of the whole key-value pair set
//-----------------------------------------------------------------------------
KeyValues *KeyValues::MakeCopy( void )
{
	KeyValues *newKeyValue = new KeyValues(GetName());

	// copy data
	newKeyValue->m_iDataType = m_iDataType;
	switch ( m_iDataType )
	{
	case TYPE_STRING:
		{
			if ( m_sValue )
			{
				int len = Q_strlen( m_sValue );
				//MODDD - assert linker error
				//assert( !newKeyValue->m_sValue );
				newKeyValue->m_sValue = new char[len + 1];
				Q_memcpy( newKeyValue->m_sValue, m_sValue, len+1 );
			}
		}
		break;
	case TYPE_WSTRING:
		{
			if ( m_wsValue )
			{
				int len = wcslen( m_wsValue );
				newKeyValue->m_wsValue = new wchar_t[len+1];
				Q_memcpy( newKeyValue->m_wsValue, m_wsValue, (len+1)*sizeof(wchar_t));
			}
		}
		break;

	case TYPE_INT:
		newKeyValue->m_iValue = m_iValue;
		break;

	case TYPE_FLOAT:
		newKeyValue->m_flValue = m_flValue;
		break;

	case TYPE_PTR:
		newKeyValue->m_pValue = m_pValue;
		break;
		
	case TYPE_COLOR:
		newKeyValue->m_Color[0] = m_Color[0];
		newKeyValue->m_Color[1] = m_Color[1];
		newKeyValue->m_Color[2] = m_Color[2];
		newKeyValue->m_Color[3] = m_Color[3];
		break;

	};

	// recursively copy subkeys
	// Also maintain ordering....
	KeyValues *pPrev = NULL;
	for ( KeyValues *sub = m_pSub; sub != NULL; sub = sub->m_pPeer )
	{
		// take a copy of the subkey
		KeyValues *dat = sub->MakeCopy();
		 
		// add into subkey list
		if (pPrev)
		{
			pPrev->m_pPeer = dat;
		}
		else
		{
			newKeyValue->m_pSub = dat;
		}
		dat->m_pPeer = NULL;
		pPrev = dat;
	}


	return newKeyValue;
}

//-----------------------------------------------------------------------------
// Purpose: Check if a keyName has no value assigned to it.
//-----------------------------------------------------------------------------
bool KeyValues::IsEmpty(const char *keyName)
{
	KeyValues *dat = FindKey(keyName, false);
	if (!dat)
		return true;

	if (dat->m_iDataType == TYPE_NONE && dat->m_pSub == NULL)
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Clear out all subkeys, and the current value
//-----------------------------------------------------------------------------
void KeyValues::Clear( void )
{
	delete m_pSub;
	m_pSub = NULL;
	m_iDataType = TYPE_NONE;
}

//-----------------------------------------------------------------------------
// Purpose: Get the data type of the value stored in a keyName
//-----------------------------------------------------------------------------
KeyValues::types_t KeyValues::GetDataType(const char *keyName)
{
	KeyValues *dat = FindKey(keyName, false);
	if (dat)
		return dat->m_iDataType;

	return TYPE_NONE;
}

//-----------------------------------------------------------------------------
// Purpose: Deletion, ensures object gets deleted from correct heap
//-----------------------------------------------------------------------------
void KeyValues::deleteThis()
{
	delete this;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : includedKeys - 
//-----------------------------------------------------------------------------
void KeyValues::AppendIncludedKeys( CUtlVector< KeyValues * >& includedKeys )
{
	// Append any included keys, too...
	int includeCount = includedKeys.Count();
	int i;
	for ( i = 0; i < includeCount; i++ )
	{
		KeyValues *kv = includedKeys[ i ];
		Assert( kv );

		KeyValues *insertSpot = this;
		while ( insertSpot->GetNextKey() )
		{
			insertSpot = insertSpot->GetNextKey();
		}

		insertSpot->SetNextKey( kv );
	}
}

void KeyValues::ParseIncludedKeys( char const *resourceName, const char *filetoinclude, 
		IBaseFileSystem* pFileSystem, const char *pPathID, CUtlVector< KeyValues * >& includedKeys )
{
	Assert( resourceName );
	Assert( filetoinclude );
	Assert( pFileSystem );
	
	// Load it...
	if ( !pFileSystem )
	{
		return;
	}

	// Get relative subdirectory
	char fullpath[ 512 ];
	Q_strncpy( fullpath, resourceName, sizeof( fullpath ) );

	// Strip off characters back to start or first /
	bool done = false;
	int len = Q_strlen( fullpath );
	while ( !done )
	{
		if ( len <= 0 )
		{
			break;
		}
		
		if ( fullpath[ len - 1 ] == '\\' || 
			 fullpath[ len - 1 ] == '/' )
		{
			break;
		}

		// zero it
		fullpath[ len - 1 ] = 0;
		--len;
	}

	// Append included file
	Q_strcat( fullpath, filetoinclude );

	KeyValues *newKV = new KeyValues( fullpath );

	// CUtlSymbol save = s_CurrentFileSymbol;	// did that had any use ???

	newKV->UsesEscapeSequences( m_bHasEscapeSequences );	// use same format as parent

	if ( newKV->LoadFromFile( pFileSystem, fullpath, pPathID ) )
	{
		includedKeys.AddToTail( newKV );
	}
	else
	{
		// nope
		//DevMsg( "KeyValues::ParseIncludedKeys: Couldn't load included keyvalue file %s\n", fullpath );
		newKV->deleteThis();
	}

	// s_CurrentFileSymbol = save;
}

//-----------------------------------------------------------------------------
// Read from a buffer...
//-----------------------------------------------------------------------------
bool KeyValues::LoadFromBuffer( char const *resourceName, const char *pBuffer, IBaseFileSystem* pFileSystem , const char *pPathID )
{
	char *pfile = const_cast<char *>(pBuffer);

	KeyValues *pPreviousKey = NULL;
	KeyValues *pCurrentKey = this;
	CUtlVector< KeyValues * > includedKeys;
	bool wasQuoted;
	
	while ( true )
	{
		// the first thing must be a key
		const char *s = ReadToken( &pfile, wasQuoted );
		
		if ( !pfile || !s || *s == 0 )
			break;

		if ( !Q_stricmp( s, "#include" ) )	// special include macro (not a key name)
		{
			s = ReadToken( &pfile, wasQuoted );
			// Name of subfile to load is now in s

			if ( !s || *s == 0 )
			{
				// nope
				//DevMsg( "KeyValues::LoadFromBuffer: #include is NULL in file %s\n", resourceName);
			}
			else
			{
				ParseIncludedKeys( resourceName, s, pFileSystem, pPathID, includedKeys );
			}

			continue;
		}

		if ( !pCurrentKey )
		{
			pCurrentKey = new KeyValues( s );

			pCurrentKey->UsesEscapeSequences( m_bHasEscapeSequences ); // same format has parent use

			if ( pPreviousKey )
			{
				pPreviousKey->SetNextKey( pCurrentKey );
			}
		}
		else
		{
			pCurrentKey->SetName( s );
		}

		// get the '{'
		s = ReadToken( &pfile, wasQuoted );

		if ( s && *s == '{' && !wasQuoted )
		{
			// header is valid so load the file
			pCurrentKey->RecursiveLoadFromBuffer( resourceName, &pfile );
		}
		else
		{
			// nope
			//DevMsg( "KeyValues::LoadFromBuffer: missing { in %s\n", resourceName);
		}

		pPreviousKey = pCurrentKey;
		pCurrentKey = NULL;
	}

	AppendIncludedKeys( includedKeys );

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void KeyValues::RecursiveLoadFromBuffer( char const *resourceName, char **pfile )
{
	bool wasQuoted;

	while ( 1 )
	{
		// get the key name
		const char * name = ReadToken( pfile, wasQuoted );

		if ( !name )	// EOF stop reading
			break;

		if ( !*name ) // empty token, maybe "" or EOF
		{
			Msg( "KeyValues::RecursiveLoadFromBuffer:  got empty keyname in section %s of %s\n",
				GetName() ? GetName() : "NULL",
				resourceName ? resourceName : "???" );
			break;
		}

		if ( *name == '}' && !wasQuoted )	// top level closed, stop reading
			break;

		// Always create the key; note that this could potentially
		// cause some duplication, but that's what we want sometimes
		KeyValues *dat = CreateKey( name );

		// get the value
		const char * value = ReadToken( pfile, wasQuoted );

		if ( !value )
		{
			Msg( "KeyValues::RecursiveLoadFromBuffer:  expecting value, got NULL in key %s of section %s in %s\n",
				m_pSub ? m_pSub->GetName() : "NULL",
				GetName() ? GetName() : "NULL",
				resourceName ? resourceName : "???" );
			break;
		}
		
		if ( *value == '}' && !wasQuoted )
		{
			Msg( "KeyValues::RecursiveLoadFromBuffer:  expecting value, got } in key %s of section %s in %s\n",
				m_pSub ? m_pSub->GetName() : "NULL",
				GetName() ? GetName() : "NULL",
				resourceName ? resourceName : "???" );
			break;
		}

		if ( *value == '{' && !wasQuoted )
		{
			// sub value list
			dat->RecursiveLoadFromBuffer( resourceName, pfile );
		}
		else 
		{
			if (dat->m_sValue)
			{
				delete[] dat->m_sValue;
			}

			int len = Q_strlen( value );
			dat->m_sValue = new char[len+1];
			Q_memcpy( dat->m_sValue, value, len+1 );

			// Here, let's determine if we got a float or an int....
			char* pIEnd;	// pos where int scan ended
			char* pFEnd;	// pos where float scan ended
			const char* pSEnd = value + len ; // pos where token ends

			int ival = strtol( value, &pIEnd, 10 );
			float fval = (float)strtod( value, &pFEnd );

			if ( *value == 0 )
			{
				dat->m_iDataType = TYPE_STRING;	
			}
			else if ( (pFEnd > pIEnd) && (pFEnd == pSEnd) )
			{
				dat->m_flValue = fval; 
				dat->m_iDataType = TYPE_FLOAT;
			}
			else if (pIEnd == pSEnd)
			{
				dat->m_iValue = ival; 
				dat->m_iDataType = TYPE_INT;
			}
			else
			{
				dat->m_iDataType = TYPE_STRING;
			}
		}
	}
}

#include "tier0/memdbgoff.h"

//-----------------------------------------------------------------------------
// Purpose: memory allocator
//-----------------------------------------------------------------------------
void *KeyValues::operator new( unsigned int iAllocSize )
{
	return KeyValuesSystem()->AllocKeyValuesMemory(iAllocSize);
}

void *KeyValues::operator new( unsigned int iAllocSize, int nBlockUse, const char *pFileName, int nLine )
{
	return KeyValuesSystem()->AllocKeyValuesMemory(iAllocSize);
}

//-----------------------------------------------------------------------------
// Purpose: deallocator
//-----------------------------------------------------------------------------
void KeyValues::operator delete( void *pMem )
{
	KeyValuesSystem()->FreeKeyValuesMemory(pMem);
}

#include "tier0/memdbgon.h"
