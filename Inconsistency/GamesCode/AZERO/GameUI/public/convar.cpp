
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "basetypes.h"
#include "convar.h"
#include "vstdlib/strtools.h"
#include "tier0/dbg.h"
#include "tier0/memdbgon.h"


ConCommandBase			*ConCommandBase::s_pConCommandBases = NULL;
IConCommandBaseAccessor	*ConCommandBase::s_pAccessor = NULL;

// ----------------------------------------------------------------------------- //
// ConCommandBaseMgr.
// ----------------------------------------------------------------------------- //
void ConCommandBaseMgr::OneTimeInit( IConCommandBaseAccessor *pAccessor )
{
	ConCommandBase *pCur, *pNext;

	ConCommandBase::s_pAccessor = pAccessor;

	pCur = ConCommandBase::s_pConCommandBases;
	while ( pCur )
	{
		pNext = pCur->m_pNext;
		pCur->Init();
		pCur = pNext;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Default constructor
//-----------------------------------------------------------------------------
ConCommandBase::ConCommandBase( void )
{
	m_pParent				= NULL;
	m_bRegistered			= false;
	m_pszName				= NULL;
	m_pszHelpString			= NULL;

	m_nFlags				= 0;
	m_pNext					= NULL;
}

//-----------------------------------------------------------------------------
// Purpose: The base console invoked command/cvar interface
// Input  : *pName - name of variable/command
//			*pHelpString - help text
//			flags - flags
//-----------------------------------------------------------------------------
ConCommandBase::ConCommandBase( char const *pName, char const *pHelpString /*=0*/, int flags /*= 0*/ )
{
	Create( pName, pHelpString, flags );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
ConCommandBase::~ConCommandBase( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool ConCommandBase::IsCommand( void ) const
{ 
//	Assert( 0 ); This can't assert. . causes a recursive assert in Sys_Printf, etc.
	return true;
}
	
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pName - 
//			callback - 
//			*pHelpString - 
//			flags - 
//-----------------------------------------------------------------------------
void ConCommandBase::Create( char const *pName, char const *pHelpString /*= 0*/, int flags /*= 0*/ )
{
	static char *empty_string = "";

	m_pParent			= this;

	m_bRegistered		= false;

	// Name should be static data
	Assert( pName );
	m_pszName			= pName;
	m_pszHelpString		= pHelpString ? pHelpString : empty_string;

	m_nFlags = flags;

	if ( !( m_nFlags & FCVAR_UNREGISTERED ) )
	{
		m_pNext		= s_pConCommandBases;
		s_pConCommandBases	= this;
	}
	else
	{
		// It's unregistered
		m_pNext		= NULL;
	}

	// If s_pAccessor is already set (this ConVar is not a global variable),
	//  register it.
	if ( s_pAccessor )
	{
		Init();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Used internally by OneTimeInit to initialize.
//-----------------------------------------------------------------------------
void ConCommandBase::Init()
{
	if ( !s_pAccessor )
		return;
	
	if ( m_bRegistered )
		return;

	s_pAccessor->RegisterConCommandBase( this );

	m_bRegistered = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *name - 
// Output : ConCommandBase
//-----------------------------------------------------------------------------
ConCommandBase const *ConCommandBase::FindCommand( char const *name )
{
	ConCommandBase const *cmd = GetCommands();
	for ( ; cmd; cmd = cmd->GetNext() )
	{
		if ( !stricmp( name, cmd->GetName() ) )
			return cmd;
	}
	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : ConCommandBase
//-----------------------------------------------------------------------------
const ConCommandBase *ConCommandBase::GetCommands( void )
{
	return s_pConCommandBases;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *var - 
// Output : static
//-----------------------------------------------------------------------------
void ConCommandBase::AddToList( ConCommandBase *var )
{
	// This routine is only valid on root ConCommandBases
	Assert(var->m_pParent == var);	
	var->m_pNext = s_pConCommandBases;
	s_pConCommandBases = var;
}

//-----------------------------------------------------------------------------
// Purpose: Return name of the command/var
// Output : char const
//-----------------------------------------------------------------------------
char const *ConCommandBase::GetName( void ) const
{
	return m_pParent->m_pszName;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : flag - 
//-----------------------------------------------------------------------------
void ConCommandBase::RemoveFlaggedCommands( int flag )
{
	ConCommandBase	*pNewList;
	ConCommandBase  *pCommand, *pNext;

	pNewList = NULL;

	pCommand = s_pConCommandBases;
	while ( pCommand )
	{
		pNext = pCommand->m_pNext;
		if ( !( pCommand->m_nFlags & flag ) )
		{
			pCommand->m_pNext = pNewList;
			pNewList = pCommand;
		}
		else
		{
			// Unlink
			pCommand->m_pNext = NULL;
		}

		pCommand = pNext;
	}
	
	s_pConCommandBases = pNewList;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : flag - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool ConCommandBase::IsBitSet( int flag ) const
{
	return ( flag & m_pParent->m_nFlags ) ? true : false;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : flags - 
//-----------------------------------------------------------------------------
void ConCommandBase::AddFlags( int flags )
{
	m_pParent->m_nFlags |= flags;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : const ConCommandBase
//-----------------------------------------------------------------------------
const ConCommandBase *ConCommandBase::GetNext( void ) const
{
	return m_pNext;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *next - 
//-----------------------------------------------------------------------------
void ConCommandBase::SetNext( ConCommandBase *next )
{
	Assert(m_pParent == this);	// This routine is only valid on root cvars.
	m_pNext = next;
}

//-----------------------------------------------------------------------------
// Purpose: Copies string using local new/delete operators
// Input  : *from - 
// Output : char
//-----------------------------------------------------------------------------
char *ConCommandBase::CopyString( char const *from )
{
	int		len;
	char	*to;

	len = strlen( from );
	if ( len <= 0 )
	{
		to = new char[1];
		to[0] = 0;
	}
	else
	{
		to = new char[len+1];
		strcpy( to, from );
	}
	return to;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : char const
//-----------------------------------------------------------------------------
char const *ConCommandBase::GetHelpText( void ) const
{
	return m_pParent->m_pszHelpString;
}

//-----------------------------------------------------------------------------
// Purpose: Has this cvar been registered
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool ConCommandBase::IsRegistered( void ) const
{
	return m_pParent->m_bRegistered;
}

//-----------------------------------------------------------------------------
// Purpose: Constructs a console command
// Input  : *pName - name of command
//			callback - function to call upon execution
//			*pHelpString - help text for command
//			flags - command flags, if any
//-----------------------------------------------------------------------------
ConCommand::ConCommand( char const *pName, FnCommandCallback callback, char const *pHelpString /*= 0*/, int flags /*= 0*/, FnCommandCompletionCallback completionFunc /*= 0*/ )
{
	Create( pName, callback, pHelpString, flags, completionFunc );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
ConCommand::~ConCommand( void )
{

}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool ConCommand::IsCommand( void ) const
{ 
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Invoke the function if there is one
//-----------------------------------------------------------------------------
void ConCommand::Dispatch( void )
{
	if ( m_fnCommandCallback )
	{
		( *m_fnCommandCallback )();
	}
	else
	{
		// Command without callback!!!
		Assert( 0 );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *partial - 
//			context - 
//			longest - 
//			maxcommands - 
//			**commands - 
// Output : int
//-----------------------------------------------------------------------------
int DefaultCompletionFunc( char const *partial, char commands[ COMMAND_COMPLETION_MAXITEMS ][ COMMAND_COMPLETION_ITEM_LENGTH ] )
{
	return 0;
}

//-----------------------------------------------------------------------------
// Purpose: Create the named command
// Input  : *pName - 
//			callback - 
//			*pHelpString - 
//			flags - 
//-----------------------------------------------------------------------------
void ConCommand::Create( char const *pName, FnCommandCallback callback, char const *pHelpString /*= 0*/, int flags /*= 0*/, FnCommandCompletionCallback completionFunc /*=0*/ )
{
	// Set the callback
	m_fnCommandCallback = callback;

	m_fnCompletionCallback = completionFunc ? completionFunc : DefaultCompletionFunc;
	m_bHasCompletionCallback = completionFunc != 0 ? true : false;

	// Setup the rest
	BaseClass::Create( pName, pHelpString, flags );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *partial - 
//			context - 
//			longest - 
//			maxcommands - 
//			**commands - 
//-----------------------------------------------------------------------------
int	ConCommand::AutoCompleteSuggest( char const *partial, char commands[ COMMAND_COMPLETION_MAXITEMS ][ COMMAND_COMPLETION_ITEM_LENGTH ] )
{
	Assert( m_fnCompletionCallback );
	if ( !m_fnCompletionCallback )
		return 0;

	return ( m_fnCompletionCallback )( partial, commands );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool ConCommand::CanAutoComplete( void )
{
	return m_bHasCompletionCallback;
}

// ----------------------------------------------------------------------------- //
// ConVar.
// ----------------------------------------------------------------------------- //
ConVar::ConVar( char const *pName, char const *pDefaultValue, int flags /* = 0 */ )
{
	Create( pName, pDefaultValue, flags );
}

// ----------------------------------------------------------------------------- //
// ConVar.
// ----------------------------------------------------------------------------- //
ConVar::ConVar( char const *pName, char const *pDefaultValue, int flags, char const *pHelpString )
{
	Create( pName, pDefaultValue, flags, pHelpString );
}

// ----------------------------------------------------------------------------- //
// ConVar.
// ----------------------------------------------------------------------------- //
ConVar::ConVar( char const *pName, char const *pDefaultValue, int flags, char const *pHelpString, bool bMin, float fMin, bool bMax, float fMax )
{
	Create( pName, pDefaultValue, flags, pHelpString, bMin, fMin, bMax, fMax );
}

// ----------------------------------------------------------------------------- //
// ConVar.
// ----------------------------------------------------------------------------- //
ConVar::ConVar( char const *pName, char const *pDefaultValue, int flags, char const *pHelpString, FnChangeCallback callback )
{
	Create( pName, pDefaultValue, flags, pHelpString, false, 0.0, false, 0.0, callback );
}

// ----------------------------------------------------------------------------- //
// ConVar.
// ----------------------------------------------------------------------------- //
ConVar::ConVar( char const *pName, char const *pDefaultValue, int flags, char const *pHelpString, bool bMin, float fMin, bool bMax, float fMax, FnChangeCallback callback )
{
	Create( pName, pDefaultValue, flags, pHelpString, bMin, fMin, bMax, fMax, callback );
}

//-----------------------------------------------------------------------------
// Purpose: Free dynamic memory
//-----------------------------------------------------------------------------
ConVar::~ConVar( void )
{
	if ( m_pszString )
	{
		delete[] m_pszString;
		m_pszString = NULL;
	}
}


//-----------------------------------------------------------------------------
// Install a change callback (there shouldn't already be one....)
//-----------------------------------------------------------------------------

void ConVar::InstallChangeCallback( FnChangeCallback callback )
{
	Assert( !m_fnChangeCallback || !callback );
	m_fnChangeCallback = callback;

	if (m_fnChangeCallback)
	{
		// Call it immediately to set the initial value...
		m_fnChangeCallback( this, m_pszString );
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool ConVar::IsCommand( void ) const
{ 
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : 
//-----------------------------------------------------------------------------
void ConVar::Init()
{
	BaseClass::Init();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *value - 
//-----------------------------------------------------------------------------
void ConVar::InternalSetValue( char const *value )
{
	float fNewValue;
	char  tempVal[ 32 ];
	char  *val;

	Assert(m_pParent == this); // Only valid for root convars.

	val = (char *)value;
	fNewValue = ( float )atof( value );

	if ( ClampValue( fNewValue ) )
	{
		Q_snprintf( tempVal,sizeof(tempVal), "%f", fNewValue );
		val = tempVal;
	}
	
	// Redetermine value
	m_fValue		= fNewValue;
	m_nValue		= ( int )( m_fValue );

	ChangeStringValue( val );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *tempVal - 
//-----------------------------------------------------------------------------
void ConVar::ChangeStringValue( char const *tempVal )
{
	Assert( !( m_nFlags & FCVAR_NEVER_AS_STRING ) );

	//MODDD - avoid alloca
	//char* pszOldValue = (char*)stackalloc( m_StringLength );
	char* pszOldValue = (char*)malloc( m_StringLength );

	if ( m_fnChangeCallback )
	{
		memcpy( pszOldValue, m_pszString, m_StringLength );
	}
	
	int len = Q_strlen(tempVal) + 1;

	if ( len > m_StringLength)
	{
		if (m_pszString)
			delete[] m_pszString;

		m_pszString	= new char[len];
		m_StringLength = len;
	}

	memcpy( m_pszString, tempVal, len );

	// Invoke any necessary callback function
	if ( m_fnChangeCallback )
	{
		m_fnChangeCallback( this, pszOldValue );
	}

	//MODDD - need to free this normally now
	//stackfree( pszOldValue );
	free(pszOldValue);


}

//-----------------------------------------------------------------------------
// Purpose: Check whether to clamp and then perform clamp
// Input  : value - 
// Output : Returns true if value changed
//-----------------------------------------------------------------------------
bool ConVar::ClampValue( float& value )
{
	if ( m_bHasMin && ( value < m_fMinVal ) )
	{
		value = m_fMinVal;
		return true;
	}
	
	if ( m_bHasMax && ( value > m_fMaxVal ) )
	{
		value = m_fMaxVal;
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *value - 
//-----------------------------------------------------------------------------
void ConVar::InternalSetFloatValue( float fNewValue )
{
	Assert( m_pParent == this ); // Only valid for root convars.

	// Check bounds
	ClampValue( fNewValue );

	// Redetermine value
	m_fValue		= fNewValue;
	m_nValue		= ( int )m_fValue;

	if ( !( m_nFlags & FCVAR_NEVER_AS_STRING ) )
	{
		char tempVal[ 32 ];
		Q_snprintf( tempVal, sizeof( tempVal), "%f", m_fValue );
		ChangeStringValue( tempVal );
	}
	else
	{
		Assert( !m_fnChangeCallback );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *value - 
//-----------------------------------------------------------------------------
void ConVar::InternalSetIntValue( int nValue )
{
	Assert( m_pParent == this ); // Only valid for root convars.

	float fValue = (float)nValue;
	if ( ClampValue( fValue ) )
	{
		nValue = ( int )( fValue );
	}

	// Redetermine value
	m_fValue		= fValue;
	m_nValue		= nValue;

	if ( !( m_nFlags & FCVAR_NEVER_AS_STRING ) )
	{
		char tempVal[ 32 ];
		Q_snprintf( tempVal, sizeof( tempVal ), "%d", m_nValue );
		ChangeStringValue( tempVal );
	}
	else
	{
		Assert( !m_fnChangeCallback );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Private creation
//-----------------------------------------------------------------------------
void ConVar::Create( char const *pName, char const *pDefaultValue, int flags /*= 0*/,
	char const *pHelpString /*= NULL*/, bool bMin /*= false*/, float fMin /*= 0.0*/,
	bool bMax /*= false*/, float fMax /*= false*/, FnChangeCallback callback /*= NULL*/ )
{
	static char *empty_string = "";

	// Name should be static data
	m_pszDefaultValue	= pDefaultValue ? pDefaultValue : empty_string;
	Assert( pDefaultValue );

	m_StringLength = strlen( m_pszDefaultValue ) + 1;
	m_pszString = new char[m_StringLength];
	memcpy( m_pszString, m_pszDefaultValue, m_StringLength );
	
	m_bHasMin = bMin;
	m_fMinVal = fMin;
	m_bHasMax = bMax;
	m_fMaxVal = fMax;
	
	m_fnChangeCallback = callback;

	m_fValue = ( float )atof( m_pszString );

	// Bounds Check, should never happen, if it does, no big deal
	if ( m_bHasMin && ( m_fValue < m_fMinVal ) )
	{
		Assert( 0 );
	}

	if ( m_bHasMax && ( m_fValue > m_fMaxVal ) )
	{
		Assert( 0 );
	}

	m_nValue = ( int )m_fValue;

	BaseClass::Create( pName, pHelpString, flags );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *value - 
//-----------------------------------------------------------------------------
void ConVar::SetValue(char const *value)
{
	if ( m_pParent->IsCommand() )
	{
		Assert( !"Tried to SetValue a ConCommand!!!" );
		return;
	}

	ConVar *var = ( ConVar * )m_pParent;

	var->InternalSetValue( value );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : value - 
//-----------------------------------------------------------------------------
void ConVar::SetValue( float value )
{
	if ( m_pParent->IsCommand() )
	{
		Assert( !"Tried to SetValue a ConCommand!!!" );
		return;
	}

	if (value != m_fValue)
	{
		ConVar *var = ( ConVar * )m_pParent;

		var->InternalSetFloatValue( value );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : value - 
//-----------------------------------------------------------------------------
void ConVar::SetValue( int value )
{
	if ( m_pParent->IsCommand() )
	{
		Assert( !"Tried to SetValue a ConCommand!!!" );
		return;
	}

	if (value != m_nValue)
	{
		ConVar *var = ( ConVar * )m_pParent;

		var->InternalSetIntValue( value );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Reset to default value
//-----------------------------------------------------------------------------
void ConVar::Revert( void )
{
	// Force default value again
	Assert( !m_pParent->IsCommand() );
	ConVar *var = ( ConVar * )m_pParent;
	var->SetValue( var->m_pszDefaultValue );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ConVar::RevertAll( void )
{
	ConCommandBase *p = s_pConCommandBases;
	while ( p  )
	{
		if ( !p->IsCommand() )
		{
			ConVar *var = ( ConVar * )p;
			var->Revert();
		}
		p = p->m_pNext;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : minVal - 
// Output : true if there is a min set
//-----------------------------------------------------------------------------
bool ConVar::GetMin( float& minVal ) const
{
	Assert( !m_pParent->IsCommand() );
	ConVar *var = ( ConVar * )m_pParent;

	minVal = var->m_fMinVal;
	return var->m_bHasMin;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : maxVal - 
//-----------------------------------------------------------------------------
bool ConVar::GetMax( float& maxVal ) const
{
	Assert( !m_pParent->IsCommand() );
	ConVar *var = ( ConVar * )m_pParent;

	maxVal = var->m_fMaxVal;
	return var->m_bHasMax;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : char const
//-----------------------------------------------------------------------------
char const *ConVar::GetDefault( void ) const
{
	Assert( !m_pParent->IsCommand() );
	ConVar *var = ( ConVar * )m_pParent;

	return var->m_pszDefaultValue;
}

