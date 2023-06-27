//========= Copyright � 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include "BaseFileSystem.h"
#include "characterset.h"
#include "tier0/dbg.h"
#include "tier0/vprof.h"
#include "vstdlib/ICommandLine.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"



//MODDD - avoid alloca
// (replacement among this file for sanity, _alloca -> CUSTOM_ALLOCA)
#define CUSTOM_ALLOCA malloc






#define BSPOUTPUT	0	// bsp output flag -- determines type of fs_log output to generate


CBaseFileSystem* CBaseFileSystem::CSearchPath::m_fs = 0;



//MODDD - ???
CBaseFileSystem* s_pFileSystem;




static void FixSlashes( char *str );
static void FixPath( char *str );
static void StripFilename (char *path);
static char s_pScratchFileName[ MAX_PATH ];


// Case-insensitive symbol table for path IDs.
CUtlSymbolTable g_PathIDTable( 0, 32, true );


//-----------------------------------------------------------------------------
// constructor
//-----------------------------------------------------------------------------

CBaseFileSystem::CBaseFileSystem()
//: m_OpenedFiles( 0, 32, OpenedFileLessFunc )
{
	// Clear out statistics
	memset(&m_Stats,0,sizeof(m_Stats));

	CSearchPath::m_fs = this;
	//MODDD - record me too?
	s_pFileSystem = this;

	m_fwLevel		= FILESYSTEM_WARNING_REPORTUNCLOSED;
	m_pfnWarning	= NULL;
	m_pLogFile			= NULL;
	m_bOutputDebugString = false;
	CUtlSymbol::DisableStaticSymbolTable();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CBaseFileSystem::~CBaseFileSystem()
{
}



/*
//-----------------------------------------------------------------------------
// Methods of IAppSystem
//-----------------------------------------------------------------------------
bool CBaseFileSystem::Connect( CreateInterfaceFn factory )
{
	return true;
}

void CBaseFileSystem::Disconnect()
{
}

void *CBaseFileSystem::QueryInterface( const char *pInterfaceName )
{
	return NULL;
}

InitReturnVal_t CBaseFileSystem::Init()
{
	if ( getenv( "fs_debug" ) )
	{
		m_bOutputDebugString = true;
	}


	const char* someTesto = getenv("PATH");
	
	
	const char *somethin = getenv( "PHP_PEAR_BIN_DIR" );
	const char *somethin2 = getenv( "php_pear_bin_dir" );


	const char *logFileName = getenv( "fs_log" );
	if( logFileName )
	{
		m_pLogFile = fopen( logFileName, "w" ); // STEAM OK
		if ( !m_pLogFile )
			return INIT_FAILED;
	}

	// Add the executable directory as a default search path for the executable.
	if ( CommandLine()->ParmCount() != 0 )
	{
		const char *pExeName = CommandLine()->GetParm( 0 );
		if ( pExeName )
		{
			char pExeDir[ MAX_PATH ];
			strcpy( pExeDir, pExeName );
			char *pSlash;
			char *pSlash2;
			pSlash = strrchr( pExeDir,'\\' );
			pSlash2 = strrchr( pExeDir,'/' );
			if ( pSlash2 > pSlash )
			{
				pSlash = pSlash2;
			}
			if (pSlash)
			{
				*pSlash = 0;
			}

			AddSearchPath( pExeDir, "EXECUTABLE_PATH"); //, PATH_ADD_TO_TAIL );
		}
	}

	return INIT_OK;
}

void CBaseFileSystem::Shutdown()
{
	if( m_pLogFile )
	{
		fclose( m_pLogFile ); // STEAM OK
	}

	RemoveAllSearchPaths();
	Trace_DumpUnclosedFiles();
}

*/



//-----------------------------------------------------------------------------
// Computes a full write path
//-----------------------------------------------------------------------------
inline void CBaseFileSystem::ComputeFullWritePath( char* pDest, const char *pRelativePath, const char *pWritePathID )
{
	strcpy( pDest, GetWritePath(pWritePathID) );
	strcat( pDest, pRelativePath );
	FixSlashes( pDest );
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : src1 - 
//			src2 - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBaseFileSystem::OpenedFileLessFunc( COpenedFile const& src1, COpenedFile const& src2 )
{
	return src1.m_pFile < src2.m_pFile;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *filename - 
//			*options - 
// Output : FILE
//-----------------------------------------------------------------------------
FILE *CBaseFileSystem::Trace_FOpen( const char *filename, const char *options )
{
	FILE *fp = FS_fopen( filename, options );
	if ( fp )
	{
		COpenedFile		file;

		file.SetName( filename );
		file.m_pFile	= fp;

		//m_OpenedFiles.Insert( file );
		m_OpenedFiles.AddToTail( file );

		if ( m_fwLevel >= FILESYSTEM_WARNING_REPORTALLACCESSES )
		{
			Warning( FILESYSTEM_WARNING_REPORTALLACCESSES, "---FS:  open %s %p %i\n", filename, fp, m_OpenedFiles.Count() );
		}
	}

	return fp;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *fp - 
//-----------------------------------------------------------------------------
void CBaseFileSystem::Trace_FClose( FILE *fp )
{
	if ( fp )
	{
		COpenedFile file;
		file.m_pFile = fp;

		int result = m_OpenedFiles.Find( file );
		if ( result != -1 /*m_OpenedFiles.InvalidIdx()*/ )
		{
			COpenedFile found = m_OpenedFiles[ result ];
			if ( m_fwLevel >= FILESYSTEM_WARNING_REPORTALLACCESSES )
			{
				Warning( FILESYSTEM_WARNING_REPORTALLACCESSES, "---FS:  close %s %p %i\n", found.GetName(), fp, m_OpenedFiles.Count() );
			}
			m_OpenedFiles.Remove( result );
		}
		else
		{
			Assert( 0 );

			if ( m_fwLevel >= FILESYSTEM_WARNING_REPORTALLACCESSES )
			{
				Warning( FILESYSTEM_WARNING_REPORTALLACCESSES, "Tried to close unknown file pointer %p\n", fp );
			}
		}

		FS_fclose( fp );
	}
}


void CBaseFileSystem::Trace_FRead( int size, FILE* fp )
{
	if ( !fp || m_fwLevel < FILESYSTEM_WARNING_REPORTALLACCESSES_READ )
		return;

	COpenedFile file;
	file.m_pFile = fp;

	int result = m_OpenedFiles.Find( file );

	if( result != -1 )
	{
			COpenedFile found = m_OpenedFiles[ result ];

			Warning( FILESYSTEM_WARNING_REPORTALLACCESSES_READ, "---FS:  read %s %i %p\n", found.GetName(), size, fp  );
	} 
	else
	{
			Warning( FILESYSTEM_WARNING_REPORTALLACCESSES_READ, "Tried to read %i bytes from unknown file pointer %p\n", size, fp );

	}
}

void CBaseFileSystem::Trace_FWrite( int size, FILE* fp )
{
	if ( !fp || m_fwLevel < FILESYSTEM_WARNING_REPORTALLACCESSES_READWRITE )
		return;

	COpenedFile file;
	file.m_pFile = fp;

	int result = m_OpenedFiles.Find( file );

	if( result != -1 )
	{
		COpenedFile found = m_OpenedFiles[ result ];

		Warning( FILESYSTEM_WARNING_REPORTALLACCESSES_READWRITE, "---FS:  write %s %i %p\n", found.GetName(), size, fp  );
	} 
	else
	{
		Warning( FILESYSTEM_WARNING_REPORTALLACCESSES_READWRITE, "Tried to write %i bytes from unknown file pointer %p\n", size, fp );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseFileSystem::Trace_DumpUnclosedFiles( void )
{
	for ( int i = 0 ; i < m_OpenedFiles.Count(); i++ )
	{
		COpenedFile *found = &m_OpenedFiles[ i ];

		if ( m_fwLevel >= FILESYSTEM_WARNING_REPORTUNCLOSED )
		{
			Warning( FILESYSTEM_WARNING_REPORTUNCLOSED, "File %s was never closed\n", found->GetName() );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseFileSystem::PrintOpenedFiles( void )
{
	Trace_DumpUnclosedFiles();
}

//-----------------------------------------------------------------------------
// Search path
//-----------------------------------------------------------------------------

typedef struct
{
	CUtlSymbol			m_Name;
	int					filepos;
	int					filelen;
} TmpFileInfo_t;

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : packfile - 
//			offsetofpackinmetafile - if the .pak is embedded in another file, then all of it's offsets
//				need this offset added to them
//-----------------------------------------------------------------------------
bool CBaseFileSystem::PreparePackFile( CSearchPath& packfile, int offsetofpackinmetafile, int fileLen )
{
	int						i;
	TmpFileInfo_t              *newfiles;

	Assert( packfile.m_bIsPackFile );
	Assert( packfile.m_hPackFile );

	Seek( (FileHandle_t)packfile.m_hPackFile, offsetofpackinmetafile, FILESYSTEM_SEEK_HEAD);

	int offset;
	ZIP_EndOfCentralDirRecord rec;
	rec.startOfCentralDirOffset = 0;
	rec.nCentralDirectoryEntries_Total = 0;

	bool foundEndOfCentralDirRecord = false;
	for( offset = fileLen - sizeof( ZIP_EndOfCentralDirRecord ); offset >= 0; offset-- )
	{
		Seek( (FileHandle_t)packfile.m_hPackFile, offsetofpackinmetafile + offset, FILESYSTEM_SEEK_HEAD);
		Read( (void *)&rec, sizeof(rec), (FileHandle_t)packfile.m_hPackFile );
		if( rec.signature == 0x06054b50 )
		{
			foundEndOfCentralDirRecord = true;
			break;
		}
	}
	Assert( foundEndOfCentralDirRecord );
	if( !foundEndOfCentralDirRecord )
	{
		return false;
	}
	
	Seek( (FileHandle_t)packfile.m_hPackFile, offsetofpackinmetafile + rec.startOfCentralDirOffset, FILESYSTEM_SEEK_HEAD );
	
	// Make sure there are some files to parse
	int numpackfiles = rec.nCentralDirectoryEntries_Total;
	if (  numpackfiles <= 0 )
	{
		// No files, sigh...
		return false;
	}
	
	newfiles = new TmpFileInfo_t[ numpackfiles ];
	Assert( newfiles );
	if ( !newfiles )
	{
		Warning( FILESYSTEM_WARNING, "%s out of memory allocating directoryf for %i files", packfile, numpackfiles );
		return false;
	}

	for( i = 0; i < rec.nCentralDirectoryEntries_Total; i++ )
	{
		ZIP_FileHeader fileHeader;
		Read( (void *)&fileHeader, sizeof( ZIP_FileHeader ), (FileHandle_t)packfile.m_hPackFile );
		Assert( fileHeader.signature == 0x02014b50 );
		Assert( fileHeader.compressionMethod == 0 );
		
		char tmpString[1024];		
		Read( (void *)tmpString, fileHeader.fileNameLength, (FileHandle_t)packfile.m_hPackFile );
		tmpString[fileHeader.fileNameLength] = '\0';
		strlwr( tmpString );
		newfiles[i].m_Name = g_PathIDTable.AddString( tmpString );
		newfiles[i].filepos = fileHeader.relativeOffsetOfLocalHeader;
		newfiles[i].filelen = fileHeader.compressedSize;
		Seek( (FileHandle_t)packfile.m_hPackFile, fileHeader.extraFieldLength + fileHeader.fileCommentLength, FILESYSTEM_SEEK_CURRENT);
	}

	for( i = 0; i < rec.nCentralDirectoryEntries_Total; i++ )
	{
		Seek( (FileHandle_t)packfile.m_hPackFile, offsetofpackinmetafile + newfiles[i].filepos, FILESYSTEM_SEEK_HEAD );
		ZIP_LocalFileHeader localFileHeader;
		Read( (void *)&localFileHeader, sizeof( ZIP_LocalFileHeader ), (FileHandle_t)packfile.m_hPackFile );
		Assert( localFileHeader.signature == 0x04034b50 );
		Seek( (FileHandle_t)packfile.m_hPackFile, localFileHeader.fileNameLength + localFileHeader.extraFieldLength, FILESYSTEM_SEEK_CURRENT);
		newfiles[i].filepos = Tell( ( FileHandle_t )packfile.m_hPackFile );
	}

	// Insert into rb tree
	for ( i=0 ; i<numpackfiles ; i++ )
	{
		CPackFileEntry	lookup;
		lookup.m_Name		= newfiles[ i ].m_Name;
		lookup.m_nPosition	= newfiles[i].filepos /* + offsetofpackinmetafile */;
		lookup.m_nLength	= newfiles[i].filelen;

		packfile.m_PackFiles.Insert( lookup );
	}
	
	packfile.m_nNumPackFiles = numpackfiles;

	delete[] newfiles;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Search pPath for pak?.pak files and add to search path if found
// Input  : *pPath - 
//-----------------------------------------------------------------------------
//MODDD - oh dear.
//void CBaseFileSystem::AddPackFiles( const char *pPath/*, SearchPathAdd_t addType*/ )
void CBaseFileSystem::AddPackFiles( char *pPath/*, SearchPathAdd_t addType*/ )
{
	int pakcount = 0;
	int i;
	for (i=0 ; ; i++)
	{
		char		pakfile[ 512 ];
		char		fullpath[ 512 ];

		sprintf( pakfile, "zip%i.zip", i );
		sprintf( fullpath, "%s%s", pPath, pakfile );

		FixSlashes( fullpath );

		struct	_stat buf;
		if( FS_stat( fullpath, &buf ) == -1 )
			break;

		++pakcount;
	}

	// Add any zip files in the format zip0.zip zip1.zip . . . 
	// Add them backwards so zip1 is higher priority than zip0, etc.
	char pakfile[ 512 ];
	char fullpath[ 512 ];
	int nCount = 0;
	for ( i=pakcount-1 ; i >= 0; i-- )
	{
		sprintf( pakfile, "zip%i.zip", i );
		sprintf( fullpath, "%s%s", pPath, pakfile );

		FixSlashes( fullpath );

		struct	_stat buf;
		// Should never happen
		if( FS_stat( fullpath, &buf ) == -1 )
		{
			Assert( 0 );
			continue;
		}

		int nIndex;


		//if ( addType == PATH_ADD_TO_TAIL )
		{
			nIndex = m_SearchPaths.AddToTail();
		}
		/*
		else
		{
			nIndex = m_SearchPaths.InsertBefore( nCount );
			++nCount;
		}
		*/

		CSearchPath *sp = &m_SearchPaths[ nIndex ];
		
		sp->m_Path			= pPath;
		sp->m_bIsPackFile	= true;
		sp->m_lPackFileTime = GetFileTime( pakfile );
		sp->m_hPackFile		= new CFileHandle;
		sp->m_hPackFile->m_pFile = Trace_FOpen( fullpath, "rb" );

		Seek( ( FILE * )sp->m_hPackFile->m_pFile, 0, FILESYSTEM_SEEK_TAIL );
		int len = FS_ftell( ( FILE * )sp->m_hPackFile->m_pFile );
		Seek( ( FILE * )sp->m_hPackFile->m_pFile, 0, FILESYSTEM_SEEK_HEAD );
		
		if ( PreparePackFile( *sp, 0, len ) )
		{
			m_PackFileHandles.AddToTail( sp->m_hPackFile->m_pFile );
		}
		else
		{
			// Failed for some reason, ignore it
			m_SearchPaths.Remove( nIndex );
			Trace_FClose( sp->m_hPackFile->m_pFile );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Wipe all map (.bsp) pak file search paths
//-----------------------------------------------------------------------------
void CBaseFileSystem::RemoveAllMapSearchPaths( void )
{
	for( int i = m_SearchPaths.Count() - 1; i >= 0; i-- )
	{
		if( !m_SearchPaths[i].m_bIsMapPath )
			continue;
		
		m_SearchPaths.Remove( i );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPath - 
//-----------------------------------------------------------------------------
void CBaseFileSystem::AddMapPackFile( const char *pPath/*, SearchPathAdd_t addType*/ )
{
	// Remove previous
	RemoveAllMapSearchPaths();

	char *newPath;
	// +2 for '\0' and potential slash added at end.
	newPath = ( char * )CUSTOM_ALLOCA( strlen( pPath ) + 2 );
	strcpy( newPath, pPath );
	strlwr( newPath );
	FixSlashes( newPath );

	// Open the .bsp and find the map lump
	char fullpath[ 512 ];
	if( strstr( newPath, ":\\" ) ) // If it's an absolute path, just use that.
	{
		strncpy( fullpath, newPath, sizeof(fullpath) );
		fullpath[ sizeof(fullpath) - 1 ] = 0;
	}
	else
	{
		//MODDD - GetLocalPath LAZY DEFAULT.  not so lazy now.
		if ( !GetLocalPath( newPath, fullpath, 512 ) )
		{
			// Couldn't find that .bsp file!!!
			return;
		}
	}

	FILE *fp = Trace_FOpen( fullpath, "rb" );
	if ( !fp )
	{
		// Couldn't open it
		Warning( FILESYSTEM_WARNING, "Couldn't open .bsp %s for embedded pack file check\n",
			fullpath );
		return;
	}

	// Get the .bsp file header
	dheader_t header;
	m_Stats.nBytesRead += FS_fread( &header, 1, sizeof( header ), fp );
	m_Stats.nReads++;

	if ( header.ident != IDBSPHEADER || header.version != BSPVERSION )
	{
		Trace_FClose( fp );
		return;
	}

	// Find the LUMP_PAKFILE offset
	lump_t *packfile = &header.lumps[ LUMP_PAKFILE ];
	if ( packfile->filelen <= sizeof( lump_t ) )
	{
		// It's empty or only contains a file header ( so there are no entries ), so don't add to search paths
		Trace_FClose( fp );
		return;
	}

	// Seek to correct position
//	Seek( fp, packfile->fileofs, FILESYSTEM_SEEK_HEAD );
	FS_fseek( fp, packfile->fileofs, FILESYSTEM_SEEK_HEAD );


	int nIndex;
	//MODDD - assume PATH_ADD_TO_TAIL
	//if ( addType == PATH_ADD_TO_TAIL )
	{
		nIndex = m_SearchPaths.AddToTail();	
	}
	/*
	else
	{
		nIndex = m_SearchPaths.AddToHead();	
	}
	*/

	CSearchPath *sp = &m_SearchPaths[ nIndex ];
	
	sp->m_Path			= g_PathIDTable.AddString( newPath );
	sp->m_bIsPackFile	= true;
	sp->m_bIsMapPath	= true;
	sp->m_lPackFileTime = GetFileTime( newPath );
	sp->m_hPackFile		= new CFileHandle;
	sp->m_hPackFile->m_pFile = fp;

	if ( PreparePackFile( *sp, packfile->fileofs, packfile->filelen ) )
	{
		m_PackFileHandles.AddToTail( sp->m_hPackFile->m_pFile );
	}
	else
	{
		// Failed for some reason, ignore it . .m_SearchPaths.Remove will close the file for us.
		m_SearchPaths.Remove( nIndex );
	}
}

void CBaseFileSystem::PrintSearchPaths( void )
{
	int i;
	Warning( FILESYSTEM_WARNING, "---------------\n" );
	Warning( FILESYSTEM_WARNING, "Paths:\n" );
	for( i = 0; i < m_SearchPaths.Count(); i++ )
	{
		CSearchPath *pSearchPath = &m_SearchPaths[i];
		Warning( FILESYSTEM_WARNING, "\"%s\" \"%s\"\n", ( const char * )pSearchPath->GetPathString(), ( const char * )pSearchPath->GetPathIDString() );
	}
}


//-----------------------------------------------------------------------------
// Purpose: This is where search paths are created.  map files are created at head of list (they occur after
//  file system paths have already been set ) so they get highest priority.  Otherwise, we add the disk (non-packfile)
//  path and then the paks if they exist for the path
// Input  : *pPath - 
//-----------------------------------------------------------------------------
void CBaseFileSystem::AddSearchPath( const char *pPath, const char *pathID/*, SearchPathAdd_t addType*/ )
{
	//NOTICE - old way never specified 'addType', assuming PATH_ADD_TO_TAIL

	// Map pak files have their own handler
	if ( strstr( pPath, ".bsp" ) )
	{
		//AddMapPackFile( pPath, addType );
		AddMapPackFile(pPath/*, PATH_ADD_TO_TAIL*/);
		return;
	}

	// Clean up the name
	char *newPath;
	// +2 for '\0' and potential slash added at end.
	newPath = ( char * )CUSTOM_ALLOCA( strlen( pPath ) + 2 );
	if ( pPath[0] == 0 )
	{
		newPath[0] = newPath[1] = 0;
	}
	else
	{
		strcpy( newPath, pPath );
#ifdef _WIN32
		strlwr( newPath );
#endif
		FixPath( newPath );
	}

	// Make sure that it doesn't already exist
	CUtlSymbol pathSym, pathIDSym;
	pathSym = g_PathIDTable.AddString( newPath );
	pathIDSym = g_PathIDTable.AddString( pathID );
	int i;
	for( i = 0; i < m_SearchPaths.Count(); i++ )
	{
		CSearchPath *pSearchPath = &m_SearchPaths[i];
		if( pSearchPath->m_Path == pathSym && pSearchPath->m_PathID == pathIDSym )
		{
			return;
		}
	}
	

	//MODDD - assume ADD_TO_TAIL now
	// Add to tail of list
	int nIndex;
	//if ( addType == PATH_ADD_TO_TAIL )
	{
		nIndex = m_SearchPaths.AddToTail();

		// Add pack files for this path next
		AddPackFiles( newPath/*, addType*/ );
	}
	/*
	else
	{
		// We'll end up here with path -> pak files in path
		AddPackFiles( newPath, addType );

		nIndex = m_SearchPaths.AddToHead();
	}
	*/

	// Grab last entry and set the path
	CSearchPath *sp = &m_SearchPaths[ nIndex ];
	
	sp->m_Path = pathSym;
	sp->m_PathID = pathIDSym;

#ifdef _DEBUG
	PrintSearchPaths();
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPath - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBaseFileSystem::RemoveSearchPath( const char *pPath/*, const char *pathID*/ )
{
	char *newPath = NULL;

	if ( pPath )
	{
		// +2 for '\0' and potential slash added at end.
		newPath = ( char * )CUSTOM_ALLOCA( strlen( pPath ) + 2 );
		strcpy( newPath, pPath );
#ifdef _WIN32
		strlwr( newPath );
#endif
		FixPath( newPath );
	}

	CUtlSymbol lookup = g_PathIDTable.AddString( newPath );

	//MODDD - pathID removed from this
	//CUtlSymbol id = g_PathIDTable.AddString( pathID );

	bool bret = false;

	// Count backward since we're possibly deleting one or more pack files, too
	int i;
	for( i = m_SearchPaths.Count() - 1; i >= 0; i-- )
	{
		if( newPath && m_SearchPaths[i].m_Path != lookup )
			continue;
		//if( pathID && m_SearchPaths[i].m_PathID != id )
		//	continue;
		
		m_SearchPaths.Remove( i );
		bret = true;
	}
	return bret;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CBaseFileSystem::CSearchPath *CBaseFileSystem::FindWritePath( const char *pathID )
{
	CUtlSymbol lookup = g_PathIDTable.AddString( pathID );

	// a pathID has been specified, find the first match in the path list
	for (int i = 0; i < m_SearchPaths.Count(); i++)
	{
		// pak files are not allowed to be written to...
		if (m_SearchPaths[i].m_bIsPackFile)
			continue;

		if ( !pathID || (m_SearchPaths[i].m_PathID == lookup) )
		{
			return &m_SearchPaths[i];
		}
	}

	return NULL;
}


//-----------------------------------------------------------------------------
// Purpose: Finds a search path that should be used for writing to, given a pathID
//-----------------------------------------------------------------------------
const char *CBaseFileSystem::GetWritePath( const char *pathID )
{
	CSearchPath *pSearchPath = NULL;
	if (pathID)
	{
		pSearchPath = FindWritePath( pathID );
		if (pSearchPath)
		{
			return pSearchPath->GetPathString();
		}

		Warning( FILESYSTEM_WARNING, "Requested non-existent write path %s!\n", pathID );
	}

	// Choose the topmost writeable path in the list to write to 
	// FIXME: Is this really what we want to be doing here?
	pSearchPath = FindWritePath( NULL ); // Should we find "DEFAULT_WRITE_PATH" ?
	if (pSearchPath)
	{
		return pSearchPath->GetPathString();
	}

	// Hope this is reasonable!!
	return ".\\";
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseFileSystem::RemoveAllSearchPaths( void )
{
	m_SearchPaths.Purge();
	m_PackFileHandles.Purge();
}


//-----------------------------------------------------------------------------
// Purpose: The base file search goes through here
// Input  : *path - 
//			*pFileName - 
//			*pOptions - 
//			packfile - 
//			*filetime - 
// Output : FileHandle_t
//-----------------------------------------------------------------------------
FileHandle_t CBaseFileSystem::FindFile( const CSearchPath *path, const char *pFileName, const char *pOptions )
{
	CFileHandle *fh;

	if ( path->m_bIsPackFile )
	{
		// Search the tree for the filename
		CPackFileEntry search;
		char *temp = (char *)CUSTOM_ALLOCA( strlen( pFileName ) + 1 );
		strcpy( temp, pFileName );
		strlwr( temp );

		search.m_Name = g_PathIDTable.AddString( temp );

		int searchresult = path->m_PackFiles.Find( search );

		if ( searchresult != path->m_PackFiles.InvalidIndex() )
		{
			CPackFileEntry result = path->m_PackFiles[ searchresult ];

			Seek( (FileHandle_t)path->m_hPackFile, result.m_nPosition, FILESYSTEM_SEEK_HEAD );

			fh = new CFileHandle;

			fh->m_pFile = ((CFileHandle *)path->m_hPackFile)->m_pFile;
			fh->m_nStartOffset = result.m_nPosition;
			fh->m_nLength = result.m_nLength;
			fh->m_nFileTime = path->m_lPackFileTime;
			fh->m_bPack = true;

			return (FileHandle_t)fh;
		}
	}
	else
	{
		// Is it an absolute path?
		char *pTmpFileName; 
		if ( strchr( pFileName, ':' ) )
		{
			pTmpFileName = ( char * )CUSTOM_ALLOCA( strlen( pFileName ) + 1 );
			strcpy( pTmpFileName, pFileName );
		}
		else
		{
			pTmpFileName = ( char * )CUSTOM_ALLOCA( strlen( path->GetPathString() ) + strlen( pFileName ) + 1 );
			strcpy( pTmpFileName, path->GetPathString() );
			strcat( pTmpFileName, pFileName );
			FixSlashes( pTmpFileName );
		}

		FILE *fp = Trace_FOpen( pTmpFileName, pOptions );
		if( fp )
		{
			if( m_pLogFile )
			{
				static char buf[1024];
#if BSPOUTPUT
				sprintf( buf, "%s\n%s\n", pFileName, pTmpFileName);
#else
				sprintf( buf, "copy \"\\srcdir\\%s\" \"\\targetdir\\%s\"\n", pTmpFileName, pTmpFileName );
				char *tmp = ( char * )CUSTOM_ALLOCA( strlen( pTmpFileName ) + 1 );
				strcpy( tmp, pTmpFileName );
				StripFilename( tmp );
				fprintf( m_pLogFile, "perl u:\\hl2\\bin\\makedirhier.pl \"\\targetdir\\%s\"\n", tmp ); // STEAM OK
#endif
				fprintf( m_pLogFile, "%s", buf ); // STEAM OK
			}

			if( m_bOutputDebugString )
			{
#ifdef _WIN32
				OutputDebugString( "fs_debug: " );
				OutputDebugString( pFileName );
				OutputDebugString( "\n" );
#elif _LINUX
				fprintf(stderr, "fs_debug: %s\n",pFileName);
#endif
			}


			fh = new CFileHandle;

			fh->m_pFile = fp;
			fh->m_bPack = false;
			struct	_stat buf;
			int sr = FS_stat( pTmpFileName, &buf );
			if ( sr == -1 )
			{
				Warning( FILESYSTEM_WARNING, "_stat on file %s which appeared to exist failed!!!\n",
					pTmpFileName );
			}
			fh->m_nFileTime = buf.st_mtime;
			fh->m_nLength = buf.st_size;
			fh->m_nStartOffset = 0;

			return ( FileHandle_t )fh;
		}
	}

	return (FileHandle_t)NULL;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
FileHandle_t CBaseFileSystem::OpenForRead( const char *pFileName, const char *pOptions, const char *pathID )
{
	CUtlSymbol lookup;
	if (pathID)
	{
		lookup = g_PathIDTable.AddString( pathID );
	}

	// Opening for READ needs to search search paths
	int i;
	for( i = 0; i < m_SearchPaths.Count(); i++ )
	{
		if (pathID && m_SearchPaths[i].m_PathID != lookup)
			continue;

		FileHandle_t filehandle = FindFile( &m_SearchPaths[ i ], pFileName, pOptions );
		if ( filehandle == 0 )
			continue;

		return filehandle;
	}

	return ( FileHandle_t )0;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
FileHandle_t CBaseFileSystem::OpenForWrite( const char *pFileName, const char *pOptions, const char *pathID )
{
	// Opening for write or append uses the write path
	// Unless an absolute path is specified...
	const char *pTmpFileName;
	if ( strstr( pFileName, ":" ) || pFileName[0] == '/' || pFileName[0] == '\\' )
	{
		pTmpFileName = pFileName;
	}
	else
	{
		ComputeFullWritePath( s_pScratchFileName, pFileName, pathID );
		pTmpFileName = s_pScratchFileName; 
	}

	FILE *fp = Trace_FOpen( pTmpFileName, pOptions );
	if( !fp )
		return ( FileHandle_t )0;

	CFileHandle *fh = new CFileHandle;

	struct	_stat buf;
	int sr = FS_stat( pTmpFileName, &buf );
	if ( sr == -1 )
	{
		Warning( FILESYSTEM_WARNING, "_stat on file %s which appeared to exist failed!!!\n",
			pTmpFileName );
	}
	fh->m_nFileTime = buf.st_mtime;
	fh->m_nLength = buf.st_size;
	fh->m_nStartOffset = 0;

	fh->m_bPack = false;
	fh->m_pFile = fp;

	return ( FileHandle_t )fh;
}




// This looks for UNC-type filename specifiers, which should be used instead of 
// passing in path ID. So if it finds //mod/cfg/config.cfg, it translates
// pFilename to "cfg/config.cfg" and pPathID to "mod" (mod is placed in tempPathID).
void CBaseFileSystem::ParsePathID( const char* &pFilename, const char* &pPathID, char tempPathID[MAX_PATH] )
{
	if ( pFilename[0] == 0 )
	{
		return;
	}
	else if ( PATHSEPARATOR( pFilename[0] ) && PATHSEPARATOR( pFilename[1] ) )
	{
		// They're specifying two path IDs. Ignore the one in the filename.
		if ( pPathID )
		{
			Assert( false );
			Warning( FILESYSTEM_WARNING, "FS:  Specified two path IDs (%s, %s).\n", pFilename, pPathID );
			return;
		}

		// Parse out the path ID.
		const char *pIn = &pFilename[2];
		char *pOut = tempPathID;
		while ( *pIn && !PATHSEPARATOR( *pIn ) && (pOut - tempPathID) < (MAX_PATH-1) )
		{
			*pOut++ = *pIn++;
		}

		*pOut = 0;

		if ( tempPathID[0] == '*' )
		{
			// * means NULL.
			pPathID = NULL;
		}
		else
		{
			pPathID = tempPathID;
		}

		// Move pFilename up past the part with the path ID.
		if ( *pIn == 0 )
			pFilename = pIn;
		else
			pFilename = pIn + 1;
	}
}



//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
FileHandle_t CBaseFileSystem::Open( const char *pFileName, const char *pOptions, const char *pathID )
{
	//CBaseFileSystem* selfRef = this;

	VPROF_BUDGET( "CBaseFileSystem::Open", VPROF_BUDGETGROUP_OTHER_FILESYSTEM );



	const char* daPath;
	if (pathID == NULL)daPath = "NULL"; else daPath = pathID;

	Warning( FILESYSTEM_WARNING, "CBaseFileSystem::Open %s %s %s\n", pFileName, pOptions, daPath );



	// Allow for UNC-type syntax to specify the path ID.
	//char tempPathID[MAX_PATH];
	//ParsePathID( pFileName, pathID, tempPathID );
	
	// Try each of the search paths in succession
	// FIXME: call createdirhierarchy upon opening for write.
	if( strstr( pOptions, "r" ) && !strstr( pOptions, "+" ) )
	{
		return OpenForRead( pFileName, pOptions, pathID );
	}

	return OpenForWrite( pFileName, pOptions, pathID );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseFileSystem::Close( FileHandle_t file )
{
	//return;
	VPROF_BUDGET( "CBaseFileSystem::Close", VPROF_BUDGETGROUP_OTHER_FILESYSTEM );
	CFileHandle *fh = ( CFileHandle *)file;
	if ( !fh )
	{
		Warning( FILESYSTEM_WARNING, "FS:  Tried to Close NULL file handle!\n" );
		return;
	}
	
	if ( !fh->m_pFile )
	{
		Warning( FILESYSTEM_WARNING, "FS:  Tried to Close NULL file pointer inside valid file handle!\n" );
		return;
	}

	// Don't close the underlying fp if this is a pack file file pointer
	bool isPackFilePointer = ( m_PackFileHandles.Find( fh->m_pFile ) != m_PackFileHandles.InvalidIndex() ) ? true : false;
	if ( !isPackFilePointer )
	{
		Trace_FClose( fh->m_pFile );
		fh->m_pFile = 0;
	}

	delete fh;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseFileSystem::Seek( FileHandle_t file, int pos, FileSystemSeek_t whence )
{
	VPROF_BUDGET( "CBaseFileSystem::Seek", VPROF_BUDGETGROUP_OTHER_FILESYSTEM );
	CFileHandle *fh = ( CFileHandle *)file;
	if ( !fh )
	{
		Warning( FILESYSTEM_WARNING, "Tried to Seek NULL file handle!\n" );
		return;
	}
	
	if ( !fh->m_pFile )
	{
		Warning( FILESYSTEM_WARNING, "FS:  Tried to Seek NULL file pointer inside valid file handle!\n" );
		return;
	}

	int seekType;
	if (whence == FILESYSTEM_SEEK_HEAD)
		seekType = SEEK_SET;
	else if (whence == FILESYSTEM_SEEK_CURRENT)
		seekType = SEEK_CUR;
	else
		seekType = SEEK_END;

	// Pack files get special handling
	if ( fh->m_bPack )
	{
		if ( whence == FILESYSTEM_SEEK_CURRENT )
		{
			// Just offset from current position
			FS_fseek( fh->m_pFile, pos, seekType );
		}
		else if ( whence == FILESYSTEM_SEEK_HEAD )
		{
			// Go to start and offset by pos
			FS_fseek( fh->m_pFile, pos + fh->m_nStartOffset, seekType );
		}
		else
		{
			// Go to end and offset by pos
			FS_fseek( fh->m_pFile, pos + fh->m_nStartOffset + fh->m_nLength, seekType );
		}
	}
	else
	{
		FS_fseek( fh->m_pFile, pos, seekType );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : file - 
// Output : unsigned int
//-----------------------------------------------------------------------------
unsigned int CBaseFileSystem::Tell( FileHandle_t file )
{
	VPROF_BUDGET( "CBaseFileSystem::Tell", VPROF_BUDGETGROUP_OTHER_FILESYSTEM );
	CFileHandle *fh = ( CFileHandle *)file;
	if ( !fh )
	{
		Warning( FILESYSTEM_WARNING, "FS:  Tried to Tell NULL file handle!\n" );
		return 0;
	}

	if ( !fh->m_pFile )
	{
		Warning( FILESYSTEM_WARNING, "FS:  Tried to Tell NULL file pointer inside valid file handle!\n" );
		return 0;
	}

	// Pack files are relative
	return FS_ftell( fh->m_pFile ) - fh->m_nStartOffset;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : file - 
// Output : unsigned int
//-----------------------------------------------------------------------------
unsigned int CBaseFileSystem::Size( FileHandle_t file )
{

	const char* daChar = (const char*)file;


	VPROF_BUDGET( "CBaseFileSystem::Size", VPROF_BUDGETGROUP_OTHER_FILESYSTEM );
	CFileHandle *fh = ( CFileHandle *)file;
	if ( !fh )
	{
		Warning( FILESYSTEM_WARNING, "FS:  Tried to Size NULL file handle!\n" );
		return 0;
	}

	if ( !fh->m_pFile )
	{
		Warning( FILESYSTEM_WARNING, "FS:  Tried to Size NULL file pointer inside valid file handle!\n" );
		return 0;
	}

	return fh->m_nLength;
}



//-----------------------------------------------------------------------------
// Purpose: 
// Input  : file - 
// Output : unsigned int
//-----------------------------------------------------------------------------
unsigned int CBaseFileSystem::Size( const char* pFileName/*, const char *pPathID*/ )
{
	VPROF_BUDGET( "CBaseFileSystem::Size", VPROF_BUDGETGROUP_OTHER_FILESYSTEM );
	// handle the case where no name passed...
	if ( !strcmp(pFileName,"") )
	{
		Warning( FILESYSTEM_WARNING, "FS:  Tried to Size NULL filename!\n" );
		return 0;
	}

	//MODDD - 'pathID' removed, not in linux decompile.

	// Allow for UNC-type syntax to specify the path ID.
	//char tempPathID[MAX_PATH];
	//ParsePathID( pFileName, pPathID, tempPathID );

	// Handle adding in searth paths
	int i;
	int iSize = 0;
	//CUtlSymbol id = g_PathIDTable.AddString( pPathID );

	for( i = 0; i < m_SearchPaths.Count(); i++ )
	{
		//if ( pPathID && m_SearchPaths[i].m_PathID != id )
		//	continue;

		iSize = FastFindFile( &m_SearchPaths[ i ], pFileName );
		if ( iSize > 0 )
		{
			break;
		}
	}
	return iSize;
}

int CBaseFileSystem::FastFindFile( const CSearchPath *path, const char *pFileName )
{
	struct	_stat buf;

	if ( path->m_bIsPackFile )
	{
		// Search the tree for the filename
		CPackFileEntry search;
		char *temp = (char *)CUSTOM_ALLOCA( strlen( pFileName ) + 1 );
		strcpy( temp, pFileName );
		strlwr( temp );

		search.m_Name = g_PathIDTable.AddString( temp );

		int searchresult = path->m_PackFiles.Find( search );

		if ( searchresult != path->m_PackFiles.InvalidIndex() )
		{
			CPackFileEntry result = path->m_PackFiles[ searchresult ];
			return (result.m_nLength);
		}
	}
	else
	{
		// Is it an absolute path?
		char *pTmpFileName; 
		if ( strchr( pFileName, ':' ) )
		{
			pTmpFileName = ( char * )CUSTOM_ALLOCA( strlen( pFileName ) + 1 );
			strcpy( pTmpFileName, pFileName );
		}
		else
		{
			pTmpFileName = ( char * )CUSTOM_ALLOCA( strlen( path->GetPathString() ) + strlen( pFileName ) + 1 );
			strcpy( pTmpFileName, path->GetPathString() );
			strcat( pTmpFileName, pFileName );
			FixSlashes( pTmpFileName );
		}

		if( _stat( pTmpFileName, &buf ) != -1 )
		{
			return buf.st_size;
		}
	}

	return ( -1 );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBaseFileSystem::EndOfFile( FileHandle_t file )
{
	CFileHandle *fh = ( CFileHandle *)file;
	if ( !fh )
	{
		Warning( FILESYSTEM_WARNING, "FS:  Tried to EndOfFile NULL file handle!\n" );
		return true;
	}

	if ( !fh->m_pFile )
	{
		Warning( FILESYSTEM_WARNING, "FS:  Tried to EndOfFile NULL file pointer inside valid file handle!\n" );
		return true;
	}

	if ( fh->m_bPack )
	{
		if ( FS_ftell( fh->m_pFile ) >=
			fh->m_nStartOffset + fh->m_nLength )
		{
			return true;
		}
		return false;
	}
	return !!FS_feof( fh->m_pFile );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CBaseFileSystem::Read( void *pOutput, int size, FileHandle_t file )
{
	VPROF_BUDGET( "CBaseFileSystem::Read", VPROF_BUDGETGROUP_OTHER_FILESYSTEM );
	CFileHandle *fh = ( CFileHandle *)file;
	if ( !fh )
	{
		Warning( FILESYSTEM_WARNING, "FS:  Tried to Read NULL file handle!\n" );
		return 0;
	}

	if ( !fh->m_pFile )
	{
		Warning( FILESYSTEM_WARNING, "FS:  Tried to Read NULL file pointer inside valid file handle!\n" );
		return 0;
	}

	size_t nBytesRead = FS_fread( pOutput, 1, size, fh->m_pFile  );
	m_Stats.nBytesRead += nBytesRead;
	m_Stats.nReads++;

	Trace_FRead(nBytesRead,fh->m_pFile);
	return nBytesRead;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CBaseFileSystem::Write( void const* pInput, int size, FileHandle_t file )
{
	VPROF_BUDGET( "CBaseFileSystem::Write", VPROF_BUDGETGROUP_OTHER_FILESYSTEM );
	CFileHandle *fh = ( CFileHandle *)file;
	if ( !fh )
	{
		Warning( FILESYSTEM_WARNING, "FS:  Tried to Write NULL file handle!\n" );
		return 0;
	}

	if ( !fh->m_pFile )
	{
		Warning( FILESYSTEM_WARNING, "FS:  Tried to Write NULL file pointer inside valid file handle!\n" );
		return 0;
	}

	size_t nBytesWritten = FS_fwrite( (void *)pInput, 1, size, fh->m_pFile  );
	m_Stats.nWrites++;
	m_Stats.nBytesWritten += nBytesWritten;

	Trace_FWrite(nBytesWritten,fh->m_pFile);


	return nBytesWritten;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CBaseFileSystem::FPrintf( FileHandle_t file, char *pFormat, ... )
{
	VPROF_BUDGET( "CBaseFileSystem::FPrintf", VPROF_BUDGETGROUP_OTHER_FILESYSTEM );
	CFileHandle *fh = ( CFileHandle *)file;
	if ( !fh )
	{
		Warning( FILESYSTEM_WARNING, "FS:  Tried to FPrintf NULL file handle!\n" );
		return 0;
	}

	if ( !fh->m_pFile )
	{
		Warning( FILESYSTEM_WARNING, "FS:  Tried to FPrintf NULL file pointer inside valid file handle!\n" );
		return 0;
	}

	va_list args;
	va_start( args, pFormat );
	int len = FS_vfprintf( fh->m_pFile , pFormat, args );
	va_end( args );
	return len;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBaseFileSystem::IsOk( FileHandle_t file )
{
	CFileHandle *fh = ( CFileHandle *)file;
	if ( !fh )
	{
		Warning( FILESYSTEM_WARNING, "FS:  Tried to IsOk NULL file handle!\n" );
		return false;
	}

	if ( !fh->m_pFile )
	{
		Warning( FILESYSTEM_WARNING, "FS:  Tried to IsOk NULL file pointer inside valid file handle!\n" );
		return false;
	}

	return FS_ferror(fh->m_pFile ) == 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseFileSystem::Flush( FileHandle_t file )
{
	VPROF_BUDGET( "CBaseFileSystem::Flush", VPROF_BUDGETGROUP_OTHER_FILESYSTEM );
	CFileHandle *fh = ( CFileHandle *)file;
	if ( !fh )
	{
		Warning( FILESYSTEM_WARNING, "FS:  Tried to Flush NULL file handle!\n" );
		return;
	}

	if ( !fh->m_pFile )
	{
		Warning( FILESYSTEM_WARNING, "FS:  Tried to Flush NULL file pointer inside valid file handle!\n" );
		return;
	}

	FS_fflush(fh->m_pFile );
}



/*
bool CBaseFileSystem::Precache( const char *pFileName, const char *pPathID)
{
	// Really simple, just open, the file, read it all in and close it. 
	// We probably want to use file mapping to do this eventually.
	FileHandle_t f = Open(pFileName,"rb", pPathID );
	if( !f )
		return false;


	char buffer[16384];
	while( sizeof(buffer) == Read(buffer,sizeof(buffer),f) )
		;

	Close(f);

	return true;
}
*/

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
char *CBaseFileSystem::ReadLine( char *pOutput, int maxChars, FileHandle_t file )
{
	VPROF_BUDGET( "CBaseFileSystem::ReadLine", VPROF_BUDGETGROUP_OTHER_FILESYSTEM );
	CFileHandle *fh = ( CFileHandle *)file;
	if ( !fh )
	{
		Warning( FILESYSTEM_WARNING, "FS:  Tried to ReadLine NULL file handle!\n" );
		return "";
	}

	if ( !fh->m_pFile )
	{
		Warning( FILESYSTEM_WARNING, "FS:  Tried to ReadLine NULL file pointer inside valid file handle!\n" );
		return "";
	}

	m_Stats.nReads++;

	char* s = FS_fgets( pOutput, maxChars, fh->m_pFile  ); // STEAM ???

	if( s )
	{
		m_Stats.nBytesRead += strlen(s);
	}
	return s;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pFileName - 
// Output : long
//-----------------------------------------------------------------------------
long CBaseFileSystem::GetFileTime( const char *pFileName/*, void* arg2, void* arg3*/         /*, const char *pPathID*/ )
{
	// Allow for UNC-type syntax to specify the path ID.

	//MODDD - pPathID removed
	//char tempPathID[MAX_PATH];
	//ParsePathID( pFileName, pPathID, tempPathID );


	//CUtlSymbol id = g_PathIDTable.AddString( pPathID );

	/*
	CFileHandle *fh = ( CFileHandle *)pFileName;
	int aman = (int)pFileName;

	CFileHandle *arg2_f = ( CFileHandle *)arg2;
	int aman2 = (int)arg2;

	CFileHandle *arg3_f = ( CFileHandle *)arg3;
	int aman3 = (int)arg3;
	*/

	VPROF_BUDGET( "CBaseFileSystem::GetFileTime", VPROF_BUDGETGROUP_OTHER_FILESYSTEM );
	int i;
	for( i = 0; i < m_SearchPaths.Count(); i++ )
	{
		/*
		if ( pPathID && m_SearchPaths[i].m_PathID != id )
			continue;
		*/

		FileHandle_t filehandle = FindFile( &m_SearchPaths[ i ], pFileName, "rb" );
		if ( filehandle == 0 )
			continue;

		CFileHandle *fh = (CFileHandle *)filehandle;
		if ( !fh )
			continue;

		long time = fh->m_nFileTime;
		
		// This function should really return a status. As it is, the callers 
		// assume it couldn't find the file if it returns 0. So return 1 instead of
		// 0 here...
		if( time == 0 )
			time = 1;
		
		Close( filehandle );

		return time;
	}
	return 0L;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pString - 
//			maxCharsIncludingTerminator - 
//			fileTime - 
//-----------------------------------------------------------------------------
void CBaseFileSystem::FileTimeToString( char *pString, int maxCharsIncludingTerminator, long fileTime )
{
	//MODDD - ??? ??? ??????
	//strncpy( pString, ctime( &fileTime ), maxCharsIncludingTerminator );
	time_t tempT = (time_t)fileTime;

	strncpy( pString, ctime( &tempT ), maxCharsIncludingTerminator );
	pString[maxCharsIncludingTerminator-1] = '\0';
}



//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pFileName - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBaseFileSystem::FileExists( const char *pFileName/*, void* arg2, void* arg3*/ /*, const char *pPathID*/ )
{
	// Allow for UNC-type syntax to specify the path ID.

	CFileHandle *arg1_f = ( CFileHandle *)pFileName;
	int arg1_i = (int)pFileName;
	const char* arg1_s = (const char*)pFileName;
	/*
	CFileHandle *arg2_f = ( CFileHandle *)arg2;
	int arg2_i= (int)arg2;
	const char* arg2_s = (const char*)arg2;

	CFileHandle *arg3_f = ( CFileHandle *)arg3;
	int arg3_i = (int)arg3;
	const char* arg3_s = (const char*)arg3;
	*/
	//MODDD - pPathID removed
	//char tempPathID[MAX_PATH];
	//ParsePathID( pFileName, pPathID, tempPathID );


	VPROF_BUDGET( "CBaseFileSystem::FileExists", VPROF_BUDGETGROUP_OTHER_FILESYSTEM );
	CUtlSymbol lookup;
	/*
	if (pPathID)
	{
		lookup = g_PathIDTable.AddString( pPathID );
	}
	*/

	int i;
	for( i = 0; i < m_SearchPaths.Count(); i++ )
	{
		/*
		if (pPathID && m_SearchPaths[i].m_PathID != lookup)
			continue;
		*/

		FileHandle_t filehandle = FindFile( &m_SearchPaths[ i ], pFileName, "rb" );
		if ( filehandle == 0 )
			continue;

		Close( filehandle );

		return true;
	}
	return false;
}

bool CBaseFileSystem::IsFileWritable( char const *pFileName, char const *pPathID /*=0*/ )
{
	CFileHandle *fh1 = ( CFileHandle *)pFileName;
	CFileHandle *fh2 = ( CFileHandle *)pPathID;
	int a1 = (int)pFileName;
	int a2 = (int)pPathID;


	// Allow for UNC-type syntax to specify the path ID.
	//char tempPathID[MAX_PATH];
	//ParsePathID( pFileName, pPathID, tempPathID );


	CUtlSymbol id = g_PathIDTable.AddString( pPathID );

	struct	_stat buf;
	int i;
	for( i = 0; i < m_SearchPaths.Size(); i++ )
	{
		// Pack files can't be directories
		CSearchPath *sp = &m_SearchPaths[ i ];
		if ( sp->m_bIsPackFile )
			continue;
		if ( pPathID && m_SearchPaths[i].m_PathID != id )
			continue;

		char *pTmpFileName;
		pTmpFileName = ( char * )CUSTOM_ALLOCA( strlen( m_SearchPaths[i].GetPathString() ) + strlen( pFileName ) + 1 );
		strcpy( pTmpFileName, m_SearchPaths[i].GetPathString() );
		strcat( pTmpFileName, pFileName );
		FixSlashes( pTmpFileName );
		if( FS_stat( pTmpFileName, &buf ) != -1 )
		{
#ifdef WIN32
			if( buf.st_mode & _S_IWRITE )
#elif LINUX
			if( buf.st_mode & S_IWRITE )
#else
			if( buf.st_mode & S_IWRITE )
#endif
			{
				return true;
			}
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pFileName - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBaseFileSystem::IsDirectory( const char *pFileName/*, const char *pathID*/ )
{
	// Allow for UNC-type syntax to specify the path ID.
	//char tempPathID[MAX_PATH];
	//ParsePathID( pFileName, pathID, tempPathID );

	//MODDD - pathID stuff removed
	//CUtlSymbol id = g_PathIDTable.AddString( pathID );

	struct	_stat buf;
	int i;
	for( i = 0; i < m_SearchPaths.Count(); i++ )
	{
		// Pack files can't be directories
		CSearchPath *sp = &m_SearchPaths[ i ];
		if ( sp->m_bIsPackFile )
			continue;
		//if ( pathID && m_SearchPaths[i].m_PathID != id )
		//	continue;

		char *pTmpFileName;
		pTmpFileName = ( char * )CUSTOM_ALLOCA( strlen( m_SearchPaths[i].GetPathString() ) + strlen( pFileName ) + 1 );
		strcpy( pTmpFileName, m_SearchPaths[i].GetPathString() );
		strcat( pTmpFileName, pFileName );
		FixSlashes( pTmpFileName );
		if( FS_stat( pTmpFileName, &buf ) != -1 )
		{
			if( buf.st_mode & _S_IFDIR )
			{
				return true;
			}
		}
	}
	return false;
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *path - 
//-----------------------------------------------------------------------------
void CBaseFileSystem::CreateDirHierarchy( const char *pRelativePath, const char *pathID )
{	
	// Allow for UNC-type syntax to specify the path ID.
	//char tempPathID[MAX_PATH];
	//ParsePathID( pRelativePath, pathID, tempPathID );


	ComputeFullWritePath( s_pScratchFileName, pRelativePath, pathID );
	int len = strlen( s_pScratchFileName ) + 1;

	char *s;
	char *end;
	
	end = s_pScratchFileName + len;
	s = s_pScratchFileName;

	while( s < end )
    {
		if( *s == CORRECT_PATH_SEPARATOR )
        {
			*s = '\0';
#ifdef _WIN32
			_mkdir( s_pScratchFileName );
#elif _LINUX
			mkdir( s_pScratchFileName, S_IRWXU |  S_IRGRP |  S_IROTH );// owner has rwx, rest have r
#endif
			*s = CORRECT_PATH_SEPARATOR;
        }
		s++;
    }
#ifdef _WIN32
	_mkdir( s_pScratchFileName );
#elif _LINUX
	mkdir( s_pScratchFileName, S_IRWXU |  S_IRGRP |  S_IROTH );
#endif

}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pWildCard - 
//			*pHandle - 
// Output : const char
//-----------------------------------------------------------------------------
//MODDD - pathID Added!   Does nothing yet though
const char *CBaseFileSystem::FindFirst( const char *pWildCard, FileFindHandle_t *pHandle, char *pathID )
{
	VPROF_BUDGET( "CBaseFileSystem::FindFirst", VPROF_BUDGETGROUP_OTHER_FILESYSTEM );
 	Assert(pWildCard);
 	Assert(pHandle);

	FileFindHandle_t hTmpHandle = m_FindData.Count();
	m_FindData.AddToTail();
	FindData_t *pFindData = &m_FindData[hTmpHandle];
	Assert(pFindData);
	
	pFindData->wildCardString.AddMultipleToTail( strlen( pWildCard ) + 1 );
	strcpy( pFindData->wildCardString.Base(), pWildCard );
	FixSlashes( pFindData->wildCardString.Base() );
	
	for(	pFindData->currentSearchPathID = 0; 
			pFindData->currentSearchPathID < m_SearchPaths.Count(); 
			pFindData->currentSearchPathID++ )
	{
		// FIXME:  Should findfirst/next work with pak files?
		if ( m_SearchPaths[pFindData->currentSearchPathID].m_bIsPackFile )
			continue;

		char *pTmpFileName;
		int size;
		size = strlen( m_SearchPaths[pFindData->currentSearchPathID].GetPathString() ) + pFindData->wildCardString.Count();
		pTmpFileName = ( char * )CUSTOM_ALLOCA( size + 1 ); // don't need two NULL terminators.
		strcpy( pTmpFileName, m_SearchPaths[pFindData->currentSearchPathID].GetPathString() );
		strcat( pTmpFileName, pFindData->wildCardString.Base() );
		FixSlashes( pTmpFileName );
		
		pFindData->findHandle = FS_FindFirstFile( pTmpFileName, &pFindData->findData );
		
		if( pFindData->findHandle != INVALID_HANDLE_VALUE )
		{
			*pHandle = hTmpHandle;
			return pFindData->findData.cFileName;
		}
	}
	
	// Handle failure here
	pFindData = 0;
	m_FindData.Remove(hTmpHandle);
	*pHandle = -1;

	return NULL;
}

// assumes that pSearchWildcard has the proper slash convention.
bool CBaseFileSystem::FileInSearchPaths( const char *pSearchWildcard, 
										   const char *pFileName, int minSearchPathID,
										   int maxSearchPathID )
{
	if( minSearchPathID > maxSearchPathID )
	{
		return false;
	}
	
	// FIGURE OUT THE FILENAME WITHOUT THE SEARCH PATH.
	const char *tmp = &pSearchWildcard[strlen( pSearchWildcard ) - 1];
	while( *tmp != CORRECT_PATH_SEPARATOR && tmp > pSearchWildcard )
	{
		tmp--;
	}
	tmp++;
	if( tmp <= pSearchWildcard )
	{
		Assert( 0 );
		return false;
	}
	int pathStrLen = tmp - pSearchWildcard;
	int fileNameStrLen = strlen( pFileName );
	char *pFileNameWithPath = ( char * )CUSTOM_ALLOCA( pathStrLen + fileNameStrLen + 1 );
	memcpy( pFileNameWithPath, pSearchWildcard, pathStrLen );
	pFileNameWithPath[pathStrLen] = '\0';
	strcat( pFileNameWithPath, pFileName );

	// Check each of the search paths and see if the file exists.
	int i;
	for( i = minSearchPathID; i <= maxSearchPathID; i++ )
	{
		// FIXME: Should this work with PAK files?
		if ( m_SearchPaths[i].m_bIsPackFile )
			continue;

		char *fullFilePath;
		int len = strlen( m_SearchPaths[i].GetPathString() ) + strlen( pFileNameWithPath );
		fullFilePath = ( char * )CUSTOM_ALLOCA( len + 1 );
		strcpy( fullFilePath, m_SearchPaths[i].GetPathString() );
		strcat( fullFilePath, pFileNameWithPath );
		struct	_stat buf;
		if( FS_stat( fullFilePath, &buf ) != -1 )
		{
			return true;
		}
	}
	return false;
}

// Get the next file, trucking through the path. . don't check for duplicates.
bool CBaseFileSystem::FindNextFileHelper( FindData_t *pFindData )
{
	// PAK files???

	// Try the same search path that we were already searching on.
	if( FS_FindNextFile( pFindData->findHandle, &pFindData->findData ) )
	{
		return true;
	}
	pFindData->currentSearchPathID++;

	if ( pFindData->findHandle != INVALID_HANDLE_VALUE )
	{
		FS_FindClose( pFindData->findHandle );
	}
	pFindData->findHandle = INVALID_HANDLE_VALUE;

	while( pFindData->currentSearchPathID < m_SearchPaths.Count() ) 
	{
		// FIXME: Should this work with PAK files?
		if ( m_SearchPaths[pFindData->currentSearchPathID].m_bIsPackFile )
		{
			pFindData->currentSearchPathID++;
			continue;
		}

		char *pTmpFileName;
		pTmpFileName = ( char * )CUSTOM_ALLOCA( strlen( m_SearchPaths[pFindData->currentSearchPathID].GetPathString() ) + 
			pFindData->wildCardString.Count() ); // don't need two NULL terminators.
		strcpy( pTmpFileName, m_SearchPaths[pFindData->currentSearchPathID].GetPathString() );
		strcat( pTmpFileName, pFindData->wildCardString.Base() );
		FixSlashes( pTmpFileName );
		
		pFindData->findHandle = FS_FindFirstFile( pTmpFileName, &pFindData->findData );
		if( pFindData->findHandle != INVALID_HANDLE_VALUE )
		{
			return true;
		}
		pFindData->currentSearchPathID++;
	}
	return false;
}	

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : handle - 
// Output : const char
//-----------------------------------------------------------------------------
const char *CBaseFileSystem::FindNext( FileFindHandle_t handle )
{
	VPROF_BUDGET( "CBaseFileSystem::FindNext", VPROF_BUDGETGROUP_OTHER_FILESYSTEM );
	FindData_t *pFindData = &m_FindData[handle];

	while( 1 )
	{
		if( FindNextFileHelper( pFindData ) )
		{
			if( !FileInSearchPaths( pFindData->wildCardString.Base(), 
									pFindData->findData.cFileName, 0, pFindData->currentSearchPathID - 1 ) )
			{
				return pFindData->findData.cFileName;
			}
		}
		else
		{
			return NULL;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : handle - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBaseFileSystem::FindIsDirectory( FileFindHandle_t handle )
{
	FindData_t *pFindData = &m_FindData[handle];
	return !!( pFindData->findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY );
}




//-----------------------------------------------------------------------------
// Purpose: 
// Input  : handle - 
//-----------------------------------------------------------------------------
void CBaseFileSystem::FindClose( FileFindHandle_t handle )
{

	if ((handle < 0) || (m_FindData.Count() == 0) || (handle >= m_FindData.Count()))
		return;

	FindData_t *pFindData = &m_FindData[handle];
	Assert(pFindData);

	if ( pFindData->findHandle != INVALID_HANDLE_VALUE)
	{
		FS_FindClose( pFindData->findHandle );
	}
	pFindData->findHandle = INVALID_HANDLE_VALUE;

	pFindData->wildCardString.Purge();
	m_FindData.FastRemove( handle );
}





//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pFileName - 
//-----------------------------------------------------------------------------
void CBaseFileSystem::GetLocalCopy( const char *pFileName )
{
	// do nothing. . everything is local.
}




/*
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pFileName - 
// Output : int
//-----------------------------------------------------------------------------
int CBaseFileSystem::GetLocalPathLen( const char *pFileName )
{
	struct	_stat buf;
	int i;
	for( i = 0; i < m_SearchPaths.Count(); i++ )
	{
		// FIXME: Should this work with PAK files?
		if ( m_SearchPaths[i].m_bIsPackFile )
			continue;

		char *pTmpFileName;
		int len = strlen( m_SearchPaths[i].GetPathString() ) + strlen( pFileName );
		pTmpFileName = ( char * )CUSTOM_ALLOCA( len + 1 );
		strcpy( pTmpFileName, m_SearchPaths[i].GetPathString() );
		strcat( pTmpFileName, pFileName );
		FixSlashes( pTmpFileName );
		if( FS_stat( pTmpFileName, &buf ) != -1 )
		{
			return len;
		}
	}
	return 0;
}
*/

typedef signed int int16_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;
typedef unsigned int uint16_t;

// !!! OpenFromCacheForRead !!!
FileHandle_t CBaseFileSystem::OpenFromCacheForRead(char *pFileName, char *pOptions,char *pathID){

	int *piVar1;
	FileHandle_t pvVar2;
	CSearchPath *path;
	int iVar3;
	int iVar4;
	CUtlSymbol CVar5;
	int iVar6;
	int local_30;
	CUtlSymbol local_1e [7];

	CVar5 = (CUtlSymbol)0xffff;
	if (pathID != (char *)0x0) {
		// ???  What was the intent of this?
		//CUtlSymbol(local_1e,pathID);
		local_1e[0] = pathID;

		CVar5 = local_1e[0];
	}
	iVar3 = (this->m_SearchPaths).m_Size;
	pvVar2 = (FileHandle_t)0x0;
	if (0 < iVar3) {
		if (pathID != (char *)0x0) {
			iVar4 = 0;
			iVar6 = 0;
			do {
				path = (CSearchPath *)
					((int)&(((this->m_SearchPaths).m_Memory.m_pMemory)->m_Path).m_Id + iVar4);
				if ((CUtlSymbol)(path->m_PathID).m_Id == CVar5) {
					pvVar2 = FindFile(path,pFileName,pOptions); //,true);
					if (pvVar2 != (FileHandle_t)0x0) {
						return pvVar2;
					}
					iVar3 = (this->m_SearchPaths).m_Size;
				}
				iVar6 = iVar6 + 1;
				iVar4 = iVar4 + 0x3c;
			} while (iVar6 < iVar3);
			return (FileHandle_t)0x0;
		}
		local_30 = 0;
		do {
			pvVar2 = FindFile((CSearchPath *)(pathID + (int)(this->m_SearchPaths).m_Memory.m_pMemory),pFileName,pOptions); //,true);
			if (pvVar2 != (FileHandle_t)0x0) {
				return pvVar2;
			}
			local_30 = local_30 + 1;
			pathID = pathID + 0x3c;
			piVar1 = &(this->m_SearchPaths).m_Size;
		} while (*piVar1 != local_30 && local_30 <= *piVar1);
	}
	return pvVar2;
	

	// ??????????


	/*
	int result = 32;

	int32_t v1 = (int32_t)result;
    uint32_t v2 = *(int32_t *)(v1 + 76); // 0xc73d
    if (v2 < 1) {
        // 0xc763
        return 0;
    }
    int32_t v3 = *(int32_t *)(v1 + 64);
    int32_t v4 = 0; // 0xc748
    int32_t v5 = 0; // 0xc748
    int32_t v6 = 0; // 0xc748
    int32_t v7 = 0; // 0xc748
    if (pathID == NULL) {
        int32_t result2 = v7 + v3; // 0xc7d5
        while (result2 == 0) {
            // 0xc7e4
            v6++;
            v7 += 60;
            if (v2 <= v6) {
                // break -> 0xc763
                break;
            }
            result2 = v7 + v3;
        }
        // 0xc763
        return (FileHandle_t)result2;
    }
    int32_t v8 = v4 + v3; // 0xc751
    uint16_t v9 = *(int16_t *)(v8 + 2); // 0xc753
    int32_t result3 = v8; // 0xc759
    uint32_t v10; // 0xc710
    while (v8 == 0 || (pathID == NULL ? 0xffff : v10 % 0x10000) != (int32_t)v9) {
        // 0xc75b
        v5++;
        v4 += 60;
        result3 = 0;
        if (v5 >= v2) {
            // break -> 0xc763
            break;
        }
        v8 = v4 + v3;
        v9 = *(int16_t *)(v8 + 2);
        result3 = v8;
    }
    // 0xc763
    return (FileHandle_t)result3;
	*/
}
// !!! AddSearchPathNoWrite !!!
void CBaseFileSystem::AddSearchPathNoWrite(char *pPath, char *pathID){
	int x = 45;
	//AddSearchPathInternal(pPath,pathID,false);
	AddSearchPath(pPath, pathID);
	return;
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pFileName - 
//			*pLocalPath - 
// Output : const char
//-----------------------------------------------------------------------------
const char *CBaseFileSystem::GetLocalPath( const char *pFileName, char *pLocalPath, int localPathBufferSize )
{


	CFileHandle *arg1_f = ( CFileHandle *)pFileName;
	int arg1_i = (int)pFileName;
	const char* arg1_s = (const char*)pFileName;

	CFileHandle *arg2_f = ( CFileHandle *)pLocalPath;
	int arg2_i= (int)pLocalPath;
	const char* arg2_s = (const char*)pLocalPath;

	CFileHandle *arg3_f = ( CFileHandle *)localPathBufferSize;
	int arg3_i = (int)localPathBufferSize;
	const char* arg3_s = (const char*)localPathBufferSize;




	struct	_stat buf;
	int i;
	for( i = 0; i < m_SearchPaths.Count(); i++ )
	{
		// FIXME: Should this work with PAK files?
		if ( m_SearchPaths[i].m_bIsPackFile )
			continue;

		char *pTmpFileName;
		int len = strlen( m_SearchPaths[i].GetPathString() ) + strlen( pFileName );
		pTmpFileName = ( char * )CUSTOM_ALLOCA( len + 1 );
		strcpy( pTmpFileName, m_SearchPaths[i].GetPathString() );
		strcat( pTmpFileName, pFileName );
		FixSlashes( pTmpFileName );
		if( FS_stat( pTmpFileName, &buf ) != -1 )
		{
			strcpy( pLocalPath, pTmpFileName );
			return pLocalPath;
		}
	}
	return NULL;
}



bool s_CharacterSetInitialized = false;
characterset_t g_BreakSet;
characterset_t g_BreakSetIncludingColons;


char g25 = 0; // 0x1ba78


char* CBaseFileSystem::ParseFile(char* pFileBytes, char* pToken, bool* pWasQuoted){

	/*
	int x = 45;
	int iVar1;
	char cVar2;
	int iVar3;

	if (pWasQuoted != (bool *)0x0) {
		*pWasQuoted = false;
	}
	if (pFileBytes == (char *)0x0) {
		return (char *)0x0;
	}
	if (s_CharacterSetInitialized == false) {
		CharacterSetBuild(&g_BreakSet,"{}()\'");
		CharacterSetBuild(&g_BreakSetIncludingColons,"{}()\':");
		s_CharacterSetInitialized = true;
	}
	*pToken = '\0';
	cVar2 = *pFileBytes;
	iVar1 = (int)cVar2;
joined_r0x00019498:
	if (iVar1 < 0x21) {
		if (iVar1 == 0) {
			return (char *)0x0;
		}
		cVar2 = pFileBytes[1];
		pFileBytes = pFileBytes + 1;
		goto LAB_000194d0;
	}
	if (iVar1 == 0x2f) {
		if (pFileBytes[1] == '/') goto code_r0x000194ab;
	}
	else {
		if (iVar1 == 0x22) {
			if (pWasQuoted != (bool *)0x0) {
				*pWasQuoted = true;
			}
			cVar2 = pFileBytes[1];
			pFileBytes = pFileBytes + 2;
			if ((cVar2 != '\"') && (iVar1 = 0, cVar2 != '\0')) goto LAB_00019595;
			iVar1 = 0;
			goto LAB_000195a4;
		}
	}
	if (g_BreakSetIncludingColons.set[iVar1] != '\0') {
		*pToken = cVar2;
		pToken[1] = '\0';
		return pFileBytes + 1;
	}
	iVar3 = 0;
	do {
		pToken[iVar3] = (char)iVar1;
		pFileBytes = pFileBytes + 1;
		iVar3 = iVar3 + 1;
		iVar1 = (int)*pFileBytes;
		if (g_BreakSetIncludingColons.set[iVar1] != '\0') break;
	} while (0x20 < iVar1);
	pToken[iVar3] = '\0';
	return pFileBytes;
code_r0x000194ab:
	if ((cVar2 != '\n') && (cVar2 != '\0')) {
		do {
			pFileBytes = pFileBytes + 1;
			cVar2 = *pFileBytes;
			if (cVar2 == '\n') break;
		} while (cVar2 != '\0');
	LAB_000194d0:
		iVar1 = (int)cVar2;
	}
	goto joined_r0x00019498;
	while (cVar2 != '\"') {
	LAB_00019595:
		pToken[iVar1] = cVar2;
		iVar1 = iVar1 + 1;
		cVar2 = *pFileBytes;
		pFileBytes = pFileBytes + 1;
		if (cVar2 == '\0') break;
	}
LAB_000195a4:
	pToken[iVar1] = '\0';
	return pFileBytes;
	*/

	if (pWasQuoted != NULL) {
        // 0x9476
        *(char *)pWasQuoted = 0;
    }
    int32_t result = (int32_t)pFileBytes;
    if (pFileBytes == NULL) {
        // 0x94e7
        return (char*)result;
    }
    // 0x947d
    if (g25 == 0) {
        // 0x9531
        g25 = 1;
    }
    int32_t v1 = (int32_t)pToken;
    *pToken = 0;
    unsigned char v2 = *pFileBytes; // 0x948d
    int32_t v3 = result; // 0x9490
    int32_t v4 = v2; // 0x9490
    int32_t v5 = v2; // 0x9490
    int32_t v6; // 0x9460
    int32_t v7; // 0x9460
    int32_t v8; // 0x9460
    int32_t v9; // 0x9460
    int32_t v10; // 0x9460
    while (true) {
      lab_0x9493:
        // 0x9493
        v6 = v3;
        v8 = v4;
        v10 = v5;
        v7 = v3;
        v9 = v4;
        if (v4 < 33) {
            goto lab_0x94d7;
        } else {
            goto lab_0x94a0;
        }
    }
  lab_0x94e7:
    // 0x94e7
    return 0;
  lab_0x94f3:;
    int32_t v11 = v3; // 0x94fc
    int32_t v12 = v4; // 0x94fc
    int32_t v13 = 0; // 0x94fc
    if (*(char *)(v4 + (int32_t)&g_BreakSetIncludingColons) != 0) {
        // 0x95af
        *pToken = (char)v5;
        *(char *)(v1 + 1) = 0;
        return (char*)(v3 + 1);
    }
    goto lab_0x9510;
  lab_0x94d7:
    // 0x94d7
    if (v9 == 0) {
        // break -> 0x94e7
        goto lab_0x94e7;
    }
    int32_t v24 = v7 + 1; // 0x94db
    char v25 = *(char *)v24; // 0x94e3
    goto lab_0x94d0_2;
  lab_0x94a0:
    // 0x94a0
    v5 = v10;
    v4 = v8;
    v3 = v6;
    if (v4 != 47) {
        if (v4 == 34) {
            if (pWasQuoted == NULL) {
                goto lab_0x9576;
            } else {
                // 0x9573
                *(char *)pWasQuoted = 1;
                goto lab_0x9576;
            }
        } else {
            goto lab_0x94f3;
        }
    }
    // 0x94a5
    if (*(char *)(v3 + 1) != 47) {
        goto lab_0x94f3;
    }
    char v29 = v5; // 0x94ab
    if (v29 != 10 == (v29 != 0)) {
        int32_t v30; // 0x94c4
        char v31; // 0x94c5
        while (true) {
          lab_0x94c4:;
            // 0x94c4
            int32_t v32; // 0x9460
            v30 = v32 + 1;
            v31 = *(char *)v30;
            v32 = v30;
            switch (v31) {
                case 10: {
                    goto lab_0x94d0;
                }
                case 0: {
                    goto lab_0x94d0;
                }
                default: {
                    goto lab_0x94c4;
                }
            }
        }
      lab_0x94d0:;
        int32_t v33 = v30; // 0x9460
        v25 = v31;
        goto lab_0x94d0_2;
    } else {
        goto lab_0x9493;
    }
  lab_0x94d0_2:;
    int32_t v26 = v25;
    v6 = v24;
    v8 = v26;
    v10 = v25;
    v7 = v24;
    v9 = v26;
    if (v25 > 32) {
        goto lab_0x94a0;
    } else {
        goto lab_0x94d7;
    }
  lab_0x9576:;
    char v27 = *(char *)(v3 + 1); // 0x9576
    int32_t v28 = v3 + 2; // 0x957a
    int32_t v21 = v28; // 0x9583
    int32_t v17 = 0; // 0x9583
    char v18 = v27; // 0x9583
    int32_t result3 = v28; // 0x9583
    int32_t v23 = 0; // 0x9583
    if (v27 != 34 == (v27 != 0)) {
        goto lab_0x9595;
    } else {
        goto lab_0x95a4;
    }
  lab_0x9510:
    // 0x9510
    *(char *)(v13 + v1) = (char)v12;
    int32_t result2 = v11 + 1; // 0x9513
    int32_t v14 = v13 + 1; // 0x9514
    char v15 = *(char *)result2; // 0x9515
    int32_t v16 = v15; // 0x9515
    v11 = result2;
    v12 = v16;
    v13 = v14;
    if (v15 > 32 != (*(char *)(v16 + (int32_t)&g_BreakSetIncludingColons) == 0)) {
        // 0x9526
        *(char *)(v14 + v1) = 0;
        return (char*)result2;
    }
    goto lab_0x9510;
  lab_0x9595:
    // 0x9595
    *(char *)(v17 + v1) = v18;
    int32_t v19 = v17 + 1; // 0x9598
    char v20 = *(char *)v21; // 0x9599
    int32_t v22 = v21 + 1; // 0x959c
    v21 = v22;
    v17 = v19;
    v18 = v20;
    result3 = v22;
    v23 = v19;
    switch (v20) {
        case 34: {
            goto lab_0x95a4;
        }
        case 0: {
            goto lab_0x95a4;
        }
        default: {
            goto lab_0x9595;
        }
    }
  lab_0x95a4:
    // 0x95a4
    *(char *)(v23 + v1) = 0;
    return (char*)result3;


}

//-----------------------------------------------------------------------------
// Purpose: Returns true on success ( based on current list of search paths, otherwise false if 
//  it can't be resolved )
// Input  : *pFullpath - 
//			*pRelative - 
//			maxRelativePathLength - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBaseFileSystem::FullPathToRelativePath( const char *pFullpath, char *pRelative )
{
	bool success = false;

	int inlen = strlen( pFullpath );
	if ( inlen <= 0 )
	{
		pRelative[ 0 ] = 0;
		return success;
	}

	strcpy( pRelative, pFullpath );

	char *inpath = (char *)CUSTOM_ALLOCA( inlen + 1 );
	Assert( inpath );
	strcpy( inpath, pFullpath );
	FixSlashes( inpath );
#ifdef _WIN32
	strlwr( inpath );
#endif

	for( int i = 0; i < m_SearchPaths.Count() && !success; i++ )
	{
		// FIXME: Should this work with embedded pak files
		if ( m_SearchPaths[i].m_bIsMapPath )
			continue;

		char *searchbase = new char[ strlen( m_SearchPaths[i].GetPathString() ) + 1 ];
		Assert( searchbase );
		strcpy( searchbase, m_SearchPaths[i].GetPathString() );
		FixSlashes( searchbase );
#ifdef _WIN32
		strlwr( searchbase );
#endif

		if ( !strnicmp( searchbase, inpath, strlen( searchbase ) ) )
		{
			success = true;
			strcpy( pRelative, &inpath[ strlen( searchbase ) ] );
		}

		delete[] searchbase;
	}

	return success;
}


//-----------------------------------------------------------------------------
// Deletes a file
//-----------------------------------------------------------------------------
void CBaseFileSystem::RemoveFile( char const* pRelativePath, const char *pathID )
{
	// Allow for UNC-type syntax to specify the path ID.
	//char tempPathID[MAX_PATH];
	//ParsePathID( pRelativePath, pathID, tempPathID );


	// Opening for write or append uses Write Path
	ComputeFullWritePath( s_pScratchFileName, pRelativePath, pathID );
	int fail = unlink( s_pScratchFileName );
	if (fail != 0)
	{
		Warning( FILESYSTEM_WARNING, "Unable to remove %s!\n", s_pScratchFileName );
	}
}


//-----------------------------------------------------------------------------
// Renames a file
//-----------------------------------------------------------------------------
void CBaseFileSystem::RenameFile( char const *pOldPath, char const *pNewPath, const char *pathID )
{
	Assert( pOldPath && pNewPath );

	char pNewFileName[ MAX_PATH ];

	ComputeFullWritePath( s_pScratchFileName, pOldPath, pathID );
	ComputeFullWritePath( pNewFileName, pNewPath, pathID );

	int fail = rename( s_pScratchFileName, pNewFileName );
	if (fail != 0)
	{
		Warning( FILESYSTEM_WARNING, "Unable to rename %s to %s!\n", s_pScratchFileName, pNewFileName );
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : **ppdir - 
//-----------------------------------------------------------------------------
bool CBaseFileSystem::GetCurrentDirectory( char* pDirectory, int maxlen )
{
#ifdef _WIN32
	if ( !::GetCurrentDirectoryA( maxlen, pDirectory ) )
#elif _LINUX
	if ( !getcwd( pDirectory, maxlen ) )
#endif
		return false;

	FixSlashes(pDirectory);

	// Strip the last slash
	int len = strlen(pDirectory);
	if ( pDirectory[ len-1 ] == CORRECT_PATH_SEPARATOR )
	{
		pDirectory[ len-1 ] = 0;
	}

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : pfnWarning - warning function callback
//-----------------------------------------------------------------------------
void CBaseFileSystem::SetWarningFunc( void (*pfnWarning)( const char *fmt, ... ) )
{
	m_pfnWarning = pfnWarning;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : level - 
//-----------------------------------------------------------------------------
void CBaseFileSystem::SetWarningLevel( FileWarningLevel_t level )
{
	m_fwLevel = level;
}

const FileSystemStatistics *CBaseFileSystem::GetFilesystemStatistics()
{
	return &m_Stats;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : level - 
//			*fmt - 
//			... - 
//-----------------------------------------------------------------------------
void CBaseFileSystem::Warning( FileWarningLevel_t level, const char *fmt, ... )
{
	if ( level > m_fwLevel )
		return;

	va_list argptr; 
    char warningtext[ 4096 ];
    
    va_start( argptr, fmt );
    _vsnprintf( warningtext, sizeof( warningtext ), fmt, argptr );
    va_end( argptr );

	// Dump to stdio
	printf( warningtext );
	if ( m_pfnWarning )
	{
		(*m_pfnWarning)( warningtext );
	}
	else
	{
#ifdef _WIN32
		OutputDebugString( warningtext );
#endif
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CBaseFileSystem::COpenedFile::COpenedFile( void )
{
	m_pFile = NULL;
	m_pName = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CBaseFileSystem::COpenedFile::~COpenedFile( void )
{
	delete[] m_pName;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : src - 
//-----------------------------------------------------------------------------
CBaseFileSystem::COpenedFile::COpenedFile( const COpenedFile& src )
{
	m_pFile = src.m_pFile;
	if ( src.m_pName )
	{
		m_pName = new char[ strlen( src.m_pName ) + 1 ];
		strcpy( m_pName, src.m_pName );
	}
	else
	{
		m_pName = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : src - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBaseFileSystem::COpenedFile::operator==( const CBaseFileSystem::COpenedFile& src ) const
{
	return src.m_pFile == m_pFile;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *name - 
//-----------------------------------------------------------------------------
void CBaseFileSystem::COpenedFile::SetName( char const *name )
{
	delete[] m_pName;
	m_pName = new char[ strlen( name ) + 1 ];
	strcpy( m_pName, name );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : char
//-----------------------------------------------------------------------------
char const *CBaseFileSystem::COpenedFile::GetName( void )
{
	return m_pName ? m_pName : "???";
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : src1 - 
//			src2 - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBaseFileSystem::CSearchPath::PackFileLessFunc( CPackFileEntry const& src1, CPackFileEntry const& src2 )
{
	return ( src1.m_Name < src2.m_Name );
}


const char* CBaseFileSystem::CSearchPath::GetPathString() const
{
	return g_PathIDTable.String( m_Path );
}


const char* CBaseFileSystem::CSearchPath::GetPathIDString() const
{
	return g_PathIDTable.String( m_PathID );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CBaseFileSystem::CSearchPath::CSearchPath( void )
	: m_PackFiles( 0, 32, PackFileLessFunc )
{
	m_Path				= g_PathIDTable.AddString( "" );
	m_bIsPackFile		= false;
	m_bIsMapPath		= false;
	m_lPackFileTime		= 0L;
	m_nNumPackFiles		= 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CBaseFileSystem::CSearchPath::~CSearchPath( void )
{
	if ( m_bIsPackFile && m_hPackFile )
	{
		// Allow closing to actually occur
		m_fs->m_PackFileHandles.FindAndRemove( m_hPackFile->m_pFile );

		m_fs->Close( (FileHandle_t)m_hPackFile );
	}
}


//-----------------------------------------------------------------------------
// Purpose: Load/unload a DLL
// Input  : *path 
//-----------------------------------------------------------------------------
CSysModule * CBaseFileSystem::LoadModule( const char *path )
{
	// old way was just this
	return Sys_LoadModule( path );


	// SRC2007 WAY
	/*
	//CHECK_DOUBLE_SLASHES( pFileName );

	//MODDD - not a parameter this time.
	const char* pPathID = NULL;

	//LogFileAccess( pFileName );
	if ( !pPathID )
	{
		pPathID = "EXECUTABLE_PATH"; // default to the bin dir
	}

	char tempPathID[ MAX_PATH ];
	ParsePathID( path, pPathID, tempPathID );
	
	CUtlSymbol lookup = g_PathIDTable.AddString( pPathID );

	// a pathID has been specified, find the first match in the path list
	int c = m_SearchPaths.Count();
	for ( int i = 0; i < c; i++ )
	{
		// pak files don't have modules
		if ( m_SearchPaths[i].m_bIsPackFile )
			continue;

		//if ( FilterByPathID( &m_SearchPaths[i], lookup ) )
		//	continue;

		Q_snprintf( tempPathID, sizeof(tempPathID), "%s%s", m_SearchPaths[i].GetPathString(), path ); // append the path to this dir.
		CSysModule *pModule = Sys_LoadModule( tempPathID );
		if ( pModule ) 
		{
			// we found the binary in one of our search paths
			return pModule;
		}
	}
	// couldn't load it from any of the search paths, let LoadLibrary try
	return Sys_LoadModule( path );
	*/
}


void CBaseFileSystem::UnloadModule( CSysModule *pModule )
{
	Sys_UnloadModule( pModule );
}


//-----------------------------------------------------------------------------
// Fixes slashes in the directory name
//-----------------------------------------------------------------------------
static void FixSlashes( char *str )
{
	while( *str )
	{
		if( *str == INCORRECT_PATH_SEPARATOR )
		{
			*str = CORRECT_PATH_SEPARATOR;
		}
		str++;
	}
}

//-----------------------------------------------------------------------------
// Make sure that slashes are of the right kind and that there is a slash at the 
// end of the filename.
// WARNING!!: assumes that you have an extra byte allocated in the case that you need
// a slash at the end.
//-----------------------------------------------------------------------------

static void FixPath( char *str )
{
	char *lastChar = &str[strlen( str ) - 1];
	if( *lastChar != CORRECT_PATH_SEPARATOR && *lastChar != INCORRECT_PATH_SEPARATOR )
	{
		lastChar[1] = CORRECT_PATH_SEPARATOR;
		lastChar[2] = '\0';
	}
	FixSlashes( str );
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *path - 
// Output : static void StripFilename
//-----------------------------------------------------------------------------
static void StripFilename (char *path)
{
	int             length;

	length = strlen(path)-1;
	while (length > 0 && !PATHSEPARATOR(path[length]))
		length--;
	path[length] = 0;
}



/*
void CBaseFileSystem::vagina(void){
	int x = 45;
}
*/

//MODDD - ??????????????  (new section)
////////////////////////////////////////////////////////////////////////////////////////////////////////////
float g_flDummyFloat = 0;

void CBaseFileSystem::Mount(void){
	g_flDummyFloat = g_flDummyFloat + g_flDummyFloat;

	
	/*
	CFileHandle *arg1_f = ( CFileHandle *)arg1;
	int aman = (int)arg1;
	const char* arg1_s = (const char*)arg1;

	CFileHandle *arg2_f = ( CFileHandle *)arg2;
	int aman2 = (int)arg2;
	const char* arg2_s = (const char*)arg2;

	CFileHandle *arg3_f = ( CFileHandle *)arg3;
	int aman3 = (int)arg3;
	const char* arg3_s = (const char*)arg3;
	*/

	return;
}


/*
void CBaseFileSystem::Unmount(void* arg1, void* arg2, void* arg3){

	CFileHandle *arg1_f = ( CFileHandle *)arg1;
	int arg1_i = (int)arg1;
	const char* arg1_s = (const char*)arg1;

	CFileHandle *arg2_f = ( CFileHandle *)arg2;
	int arg2_i= (int)arg2;
	const char* arg2_s = (const char*)arg2;

	CFileHandle *arg3_f = ( CFileHandle *)arg3;
	int arg3_i = (int)arg3;
	const char* arg3_s = (const char*)arg3;

}
*/

void CBaseFileSystem::Unmount(void){

	return;
}

void* CBaseFileSystem::GetReadBuffer(FileHandle_t file,int *outBufferSize,bool failIfNotInCache){
	*outBufferSize = 0;
	return (void *)0x0;
}



void CBaseFileSystem::ReleaseReadBuffer(FileHandle_t file,void *readBuffer){
	return;
}









//MODDD - ???? global accessor?  Nothing really uses this though
CBaseFileSystem* BaseFileSystem(){
	return s_pFileSystem;
}

/*
int __wrap_mount(char* source, char* target, char* filesystemtype, unsigned long mountflags, void* data){
	int x = 45;
	return 0;
}
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////



extern "C" int __cdecl _purecall();  


//MODDD - !!!
int __cdecl _purecall(void){
	// oh dear me!
	int x = 45;
	return 0;
}


