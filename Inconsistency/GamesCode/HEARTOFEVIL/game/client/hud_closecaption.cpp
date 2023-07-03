//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//


#include "cbase.h"
#include <ctype.h>
#include "sentence.h"
#include "hud_closecaption.h"
#include "tier1/strtools.h"
#include <vgui_controls/Controls.h>
#include <vgui/IVGui.h>
#include <vgui/ISurface.h>
#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include "iclientmode.h"
#include "hud_macros.h"
#include "checksum_crc.h"
#include "filesystem.h"
#include "datacache/idatacache.h"
#include "SoundEmitterSystem/isoundemittersystembase.h"
#ifdef HOE_DLL
#include "hoe/closedcaption_shared.h"
#endif // HOE_DLL

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define CC_INSET		12

extern ISoundEmitterSystemBase *soundemitterbase;

// Marked as FCVAR_USERINFO so that the server can cull CC messages before networking them down to us!!!
ConVar closecaption( "closecaption", "0", FCVAR_ARCHIVE | FCVAR_ARCHIVE_XBOX | FCVAR_USERINFO, "Enable close captioning." );
extern ConVar cc_lang;
static ConVar cc_linger_time( "cc_linger_time", "1.0", FCVAR_ARCHIVE, "Close caption linger time." );
static ConVar cc_predisplay_time( "cc_predisplay_time", "0.25", FCVAR_ARCHIVE, "Close caption delay before showing caption." );
static ConVar cc_captiontrace( "cc_captiontrace", "1", 0, "Show missing closecaptions (0 = no, 1 = devconsole, 2 = show in hud)" );
static ConVar cc_subtitles( "cc_subtitles", "0", FCVAR_ARCHIVE | FCVAR_ARCHIVE_XBOX, "If set, don't show sound effect captions, just voice overs (i.e., won't help hearing impaired players)." );
ConVar english( "english", "1", FCVAR_USERINFO, "If set to 1, running the english language set of assets." );
static ConVar cc_smallfontlength( "cc_smallfontlength", "300", 0, "If text stream is this long, force usage of small font size." );

#define	MAX_CAPTION_CHARACTERS		4096

#define CAPTION_PAN_FADE_TIME		0.5			// The time it takes for a line to fade while panning over a large entry
#define CAPTION_PAN_SLIDE_TIME		0.5			// The time it takes for a line to slide on while panning over a large entry


// A work unit is a pre-processed chunk of CC text to display
// Any state changes (font/color/etc) cause a new work unit to be precomputed
// Moving onto a new line also causes a new Work Unit
// The width and height are stored so that layout can be quickly recomputed each frame
class CCloseCaptionWorkUnit
{
public:
	CCloseCaptionWorkUnit();
	~CCloseCaptionWorkUnit();

	void	SetWidth( int w );
	int		GetWidth() const;

	void	SetHeight( int h );
	int		GetHeight() const;

	void	SetPos( int x, int y );
	void	GetPos( int& x, int &y ) const;

#ifndef HOE_DLL
	void	SetFadeStart( float flTime );
	float	GetFadeStart( void ) const;
#endif // HOE_DLL

	void	SetBold( bool bold );
	bool	GetBold() const;

	void	SetItalic( bool ital );
	bool	GetItalic() const;

	void	SetStream( const wchar_t *stream );
	const wchar_t	*GetStream() const;

	void	SetColor( Color& clr );
	Color GetColor() const;

	vgui::HFont		GetFont() const
	{
		return m_hFont;
	}
	
	void		SetFont( vgui::HFont fnt )
	{
		m_hFont = fnt;
	}

	void Dump()
	{
		char buf[ 2048 ];
		g_pVGuiLocalize->ConvertUnicodeToANSI( GetStream(), buf, sizeof( buf ) );

		Msg( "x = %i, y = %i, w = %i h = %i text %s\n", m_nX, m_nY, m_nWidth, m_nHeight, buf );
	}

private:

	int				m_nX;
	int				m_nY;
	int				m_nWidth;
	int				m_nHeight;
#ifndef HOE_DLL
	float			m_flFadeStartTime;
#endif // HOE_DLL
	bool			m_bBold;
	bool			m_bItalic;
	wchar_t			*m_pszStream;
	vgui::HFont			m_hFont;
	Color			m_Color;
};

CCloseCaptionWorkUnit::CCloseCaptionWorkUnit() :
	m_nWidth(0),
	m_nHeight(0),
	m_bBold(false),
	m_bItalic(false),
	m_pszStream(0),
	m_Color( Color( 255, 255, 255, 255 ) ),
#ifdef HOE_DLL
	m_hFont( 0 )
#else
	m_hFont( 0 ),
	m_flFadeStartTime(0)
#endif // HOE_DLL
{
}

CCloseCaptionWorkUnit::~CCloseCaptionWorkUnit()
{
	delete[] m_pszStream;
	m_pszStream = NULL;
}

void CCloseCaptionWorkUnit::SetWidth( int w )
{
	m_nWidth = w;
}

int CCloseCaptionWorkUnit::GetWidth() const
{
	return m_nWidth;
}

void CCloseCaptionWorkUnit::SetHeight( int h )
{
	m_nHeight = h;
}

int CCloseCaptionWorkUnit::GetHeight() const
{
	return m_nHeight;
}

void CCloseCaptionWorkUnit::SetPos( int x, int y )
{
	m_nX = x;
	m_nY = y;
}

void CCloseCaptionWorkUnit::GetPos( int& x, int &y ) const
{
	x = m_nX;
	y = m_nY;
}

#ifndef HOE_DLL
void CCloseCaptionWorkUnit::SetFadeStart( float flTime )
{
	m_flFadeStartTime = flTime;
}

float CCloseCaptionWorkUnit::GetFadeStart( void ) const
{
	return m_flFadeStartTime;
}
#endif // HOE_DLL

void CCloseCaptionWorkUnit::SetBold( bool bold )
{
	m_bBold = bold;
}

bool CCloseCaptionWorkUnit::GetBold() const
{
	return m_bBold;
}

void CCloseCaptionWorkUnit::SetItalic( bool ital )
{
	m_bItalic = ital;
}

bool CCloseCaptionWorkUnit::GetItalic() const
{
	return m_bItalic;
}

void CCloseCaptionWorkUnit::SetStream( const wchar_t *stream )
{
	delete[] m_pszStream;
	m_pszStream = NULL;

	int len = wcslen( stream );
	Assert( len < 4096 );
	m_pszStream = new wchar_t[ len + 1 ];
	wcsncpy( m_pszStream, stream, len );
	m_pszStream[ len ] = L'\0';
}

const wchar_t *CCloseCaptionWorkUnit::GetStream() const
{
	return m_pszStream ? m_pszStream : L"";
}

void CCloseCaptionWorkUnit::SetColor( Color& clr )
{
	m_Color = clr;
}

Color CCloseCaptionWorkUnit::GetColor() const
{
	return m_Color;
}

#ifdef HOE_DLL
//-----------------------------------------------------------------------------
// A line is made up of 1 or more work units.
class CCloseCaptionLine
{
public:

	CCloseCaptionLine() :
		m_flFadeStartTime(0),
		m_nTotalWidth(0),
		m_nTotalHeight(0)
	{
	}

	~CCloseCaptionLine( void )
	{
		while ( m_Work.Count() > 0 )
		{
			CCloseCaptionWorkUnit *unit = m_Work[ 0 ];
			m_Work.Remove( 0 );
			delete unit;
		}
	}

	void	SetHeight( int h )
	{
		m_nTotalHeight = h;
	}

	int		GetHeight() const
	{
		return m_nTotalHeight;
	}

	void	SetWidth( int w )
	{
		m_nTotalWidth = w;
	}

	int		GetWidth() const
	{
		return m_nTotalWidth;
	}

	void SetPos( int x, int y )
	{
		m_nX = x;
		m_nY = y;
	}

	void GetPos( int& x, int &y ) const
	{
		x = m_nX;
		y = m_nY;
	}

	int GetTop( void ) const
	{
		return m_nY;
	}

	void SetFadeStart( float flTime )
	{
		m_flFadeStartTime = flTime;
	}

	float GetFadeStart( void ) const
	{
		return m_flFadeStartTime;
	}

	void	AddWork( CCloseCaptionWorkUnit *unit )
	{
		m_Work.AddToTail( unit );
	}

	int		GetNumWorkUnits() const
	{
		return m_Work.Count();
	}

	CCloseCaptionWorkUnit *GetWorkUnit( int index )
	{
		Assert( index >= 0 && index < m_Work.Count() );

		return m_Work[ index ];
	}

	void Dump()
	{
		for ( int i = 0; i < m_Work.Count(); i++)
			m_Work[i]->Dump();
	}

	int				m_nX;
	int				m_nY;
	int				m_nTotalWidth;
	int				m_nTotalHeight;
	float			m_flFadeStartTime;

	CUtlVector< CCloseCaptionWorkUnit * >	m_Work;
};
#endif // HOE_DLL

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CCloseCaptionItem
{
public:
	CCloseCaptionItem( 
		const wchar_t	*stream,
#ifdef HOE_DLL
		const char *imagename,
		short id,
		const ClosedCaptionInfo_t *pInfo,
#endif // HOE_DLL
		float timetolive,
		float addedtime,
		float predisplay,
		bool valid,
		bool fromplayer
	) :
		m_flTimeToLive( 0.0f ),
		m_flAddedTime( addedtime ),
		m_bValid( false ),
		m_nTotalWidth( 0 ),
		m_nTotalHeight( 0 ),
		m_bSizeComputed( false ),
#ifdef HOE_DLL
		m_nDisplayHeight( 0 ),
		m_bNextLineIsNew( true ),
		m_nFadedLines( 0 ),
		m_pInfo( pInfo ),
		m_nCaptionID( id ),
		m_bSFX( false ),
#endif // HOE_DLL
		m_bFromPlayer( fromplayer )

	{
		SetStream( stream );
		SetTimeToLive( timetolive );
		SetInitialLifeSpan( timetolive );
		SetPreDisplayTime( cc_predisplay_time.GetFloat() + predisplay );
		m_bValid = valid;
#ifdef HOE_DLL
		SetIcon( imagename );
#endif // HOE_DLL
	}

	CCloseCaptionItem( const CCloseCaptionItem& src )
	{
		SetStream( src.m_szStream );
		m_flTimeToLive = src.m_flTimeToLive;
		m_bValid = src.m_bValid;
		m_bFromPlayer = src.m_bFromPlayer;
		m_flAddedTime = src.m_flAddedTime;
#ifdef HOE_DLL
		m_pIcon = src.m_pIcon;
#endif // HOE_DLL
	}

	~CCloseCaptionItem( void )
	{
#ifdef HOE_DLL
		while ( m_Lines.Count() > 0 )
		{
			CCloseCaptionLine *line = m_Lines[ 0 ];
			m_Lines.Remove( 0 );
			delete line;
		}
#else // HOE_DLL
		while ( m_Work.Count() > 0 )
		{
			CCloseCaptionWorkUnit *unit = m_Work[ 0 ];
			m_Work.Remove( 0 );
			delete unit;
		}
#endif // HOE_DLL
	}

	void SetStream( const wchar_t *stream)
	{
		wcsncpy( m_szStream, stream, sizeof( m_szStream ) / sizeof( wchar_t ) );
	}

	const wchar_t *GetStream() const
	{
		return m_szStream;
	}

	void SetTimeToLive( float ttl )
	{
		m_flTimeToLive = ttl;
	}

	float GetTimeToLive( void ) const
	{
		return m_flTimeToLive;
	}

	void SetInitialLifeSpan( float t )
	{
		m_flInitialLifeSpan = t;
	}

	float GetInitialLifeSpan() const
	{
		return m_flInitialLifeSpan;
	}

	bool IsValid() const
	{
		return m_bValid;
	}

	void	SetHeight( int h )
	{
		m_nTotalHeight = h;
	}
	int		GetHeight() const
	{
		return m_nTotalHeight;
	}
	void	SetWidth( int w )
	{
		m_nTotalWidth = w;
	}
	int		GetWidth() const
	{
		return m_nTotalWidth;
	}

#ifdef HOE_DLL
	void	SetDisplayHeight( int h )
	{
		m_nDisplayHeight = h;
	}
	int		GetDisplayHeight() const
	{
		return m_nDisplayHeight;
	}

	void	SetNextLineIsNew( bool bNew )
	{
		m_bNextLineIsNew = bNew;
	}

	bool	GetNextLineIsNew( void ) const
	{
		return m_bNextLineIsNew;
	}

	void	AddLine( CCloseCaptionLine *line )
	{
		m_Lines.AddToTail( line );
	}

	int		GetNumLines() const
	{
		return m_Lines.Count();
	}

	CCloseCaptionLine *GetLine( int index )
	{
		Assert( index >= 0 && index < m_Lines.Count() );

		return m_Lines[ index ];
	}

	void	SetDisplayLines( int n )
	{
		m_nDisplayLines = n;
	}
	int		GetDisplayLines() const
	{
		return m_nDisplayLines;
	}

	void	SetCollapsedLines( int n )
	{
		m_nCollapsedLines = n;
	}
	int		GetCollapsedLines() const
	{
		return m_nCollapsedLines;
	}

	void	SetFadedLines( int n )
	{
		m_nFadedLines = n;
	}
	int		GetFadedLines() const
	{
		return m_nFadedLines;
	}

	void SetIcon( const char *imagename )
	{
		if ( imagename && imagename[0] )
			m_pIcon = gHUD.GetIcon( imagename );
		else
			m_pIcon = NULL;
	}
	CHudTexture *GetIcon( void ) const
	{
		return m_pIcon;
	}

	const ClosedCaptionInfo_t *GetInfo( void ) const
	{
		return m_pInfo;
	}

	short GetCaptionID( void ) const
	{
		return m_nCaptionID;
	}

	void SetSFX( bool sfx )
	{
		m_bSFX = sfx;
	}
	bool GetSFX( void ) const
	{
		return m_bSFX;
	}
#else // HOE_DLL
	void	AddWork( CCloseCaptionWorkUnit *unit )
	{
		m_Work.AddToTail( unit );
	}

	int		GetNumWorkUnits() const
	{
		return m_Work.Count();
	}

	CCloseCaptionWorkUnit *GetWorkUnit( int index )
	{
		Assert( index >= 0 && index < m_Work.Count() );

		return m_Work[ index ];
	}
#endif // HOE_DLL

	void		SetSizeComputed( bool computed )
	{
		m_bSizeComputed = computed;
	}

	bool		GetSizeComputed() const
	{
		return m_bSizeComputed;
	}

	void		SetPreDisplayTime( float t )
	{
		m_flPreDisplayTime = t;
	}

	float		GetPreDisplayTime() const
	{
		return m_flPreDisplayTime;
	}

	float		GetAlpha( float fadeintimehidden, float fadeintime, float fadeouttime )
	{
		float time_since_start = m_flInitialLifeSpan - m_flTimeToLive;
		float time_until_end =  m_flTimeToLive;

		float totalfadeintime = fadeintimehidden + fadeintime;

		if ( totalfadeintime > 0.001f && 
			time_since_start < totalfadeintime )
		{
			if ( time_since_start >= fadeintimehidden )
			{
				float f = 1.0f;
				if ( fadeintime > 0.001f )
				{
					f = ( time_since_start - fadeintimehidden ) / fadeintime;
				}
				f = clamp( f, 0.0f, 1.0f );
				return f;
			}
			
			return 0.0f;
		}

		if ( fadeouttime > 0.001f &&
			time_until_end < fadeouttime )
		{
			float f = time_until_end / fadeouttime;
			f = clamp( f, 0.0f, 1.0f );
			return f;
		}

		return 1.0f;
	}

	float	GetAddedTime() const
	{
		return m_flAddedTime;
	}

	void	SetAddedTime( float addt )
	{
		m_flAddedTime = addt;
	}

	bool	IsFromPlayer() const
	{
		return m_bFromPlayer;
	}

private:
	wchar_t				m_szStream[ MAX_CAPTION_CHARACTERS ];

	float				m_flPreDisplayTime;
	float				m_flTimeToLive;
	float				m_flInitialLifeSpan;
	float				m_flAddedTime;
	bool				m_bValid;
	int					m_nTotalWidth;
	int					m_nTotalHeight;

	bool				m_bSizeComputed;
	bool				m_bFromPlayer;

#ifdef HOE_DLL
	int					m_nDisplayHeight;
	bool				m_bNextLineIsNew;
	int					m_nDisplayLines;
	int					m_nCollapsedLines;
	int					m_nFadedLines;
	CUtlVector< CCloseCaptionLine * >	m_Lines;
	CHudTexture			*m_pIcon;
	const ClosedCaptionInfo_t *m_pInfo;
	short				m_nCaptionID;
	bool				m_bSFX;
#else // HOE_DLL
	CUtlVector< CCloseCaptionWorkUnit * >	m_Work;
#endif // HOE_DLL
};

struct VisibleStreamItem
{
	int					height;
	int					width;
	CCloseCaptionItem	*item;
};

//-----------------------------------------------------------------------------
// Purpose: The only resource manager parameter we currently care about is the name 
//  of the .vcd to cache into memory
//-----------------------------------------------------------------------------
struct asynccaptionparams_t
{
	const char *dbfile;
	int			fileindex;
	int			blocktoload;
	int			blockoffset;
	int			blocksize;
};

// 16K of cache for close caption data
#define MAX_ASYNCCAPTION_MEMORY_CACHE (int)( 64.0 * 1024.0f )

void CaptionAsyncLoaderCallback( const FileAsyncRequest_t &request, int numReadBytes, FSAsyncStatus_t asyncStatus );

struct AsyncCaptionData_t
{
	int					m_nBlockNum;
	byte				*m_pBlockData;
	int					m_nFileIndex;
	int					m_nBlockSize;
	
	bool				m_bLoadPending : 1;
	bool				m_bLoadCompleted : 1;

	FSAsyncControl_t	m_hAsyncControl;

	AsyncCaptionData_t() :
		m_nBlockNum( -1 ),
		m_pBlockData( 0 ),
		m_nFileIndex( -1 ),
		m_nBlockSize( 0 ),
		m_bLoadPending( false ),
		m_bLoadCompleted( false ),
		m_hAsyncControl( NULL )
	{
	}

	// APIS required by CDataManager
	void DestroyResource()
	{
		if ( m_bLoadPending && !m_bLoadCompleted )
		{
			filesystem->AsyncFinish( m_hAsyncControl, true );
		}
		filesystem->AsyncRelease( m_hAsyncControl );

		WipeData();
		delete this;
	}

	void ReleaseData()
	{
		filesystem->AsyncRelease( m_hAsyncControl );
		m_hAsyncControl = 0;
		WipeData();
		m_bLoadCompleted = false;
		Assert( !m_bLoadPending );
	}

	void WipeData()
	{
		delete[] m_pBlockData;
		m_pBlockData = NULL;
	}

	AsyncCaptionData_t		*GetData()
	{ 
		return this; 
	}
	unsigned int	Size()
	{ 
		return sizeof( *this ) + m_nBlockSize; 
	}

	void AsyncLoad( const char *fileName, int blockOffset )
	{
		// Already pending
		Assert ( !m_hAsyncControl );

		// async load the file	
		FileAsyncRequest_t fileRequest;
		fileRequest.pContext    = (void *)this;
		fileRequest.pfnCallback = ::CaptionAsyncLoaderCallback;
		fileRequest.pData       = m_pBlockData;
		fileRequest.pszFilename = fileName;
		fileRequest.nOffset     = blockOffset;
		fileRequest.flags       = 0;
		fileRequest.nBytes      = m_nBlockSize;
		fileRequest.priority    = -1;
		fileRequest.pszPathID   = "GAME";
		
		// queue for async load
		MEM_ALLOC_CREDIT();
		filesystem->AsyncRead( fileRequest, &m_hAsyncControl );
	}

	// you must implement these static functions for the ResourceManager
	// -----------------------------------------------------------
	static AsyncCaptionData_t *CreateResource( const asynccaptionparams_t &params )
	{
		AsyncCaptionData_t *data = new AsyncCaptionData_t;
		data->m_nBlockNum = params.blocktoload;
		data->m_nFileIndex = params.fileindex;
		data->m_nBlockSize = params.blocksize;
		data->m_pBlockData = new byte[ data->m_nBlockSize ];
		return data;
	}

	static unsigned int EstimatedSize( const asynccaptionparams_t &params )
	{
		// The block size is assumed to be 4K
		return ( sizeof( AsyncCaptionData_t ) + params.blocksize );
	}
};

//-----------------------------------------------------------------------------
// Purpose: This manages the instanced scene memory handles.  We essentially grow a handle list by scene filename where
//  the handle is a pointer to a AsyncCaptionData_t defined above.  If the resource manager uncaches the handle, we reload the
//  .vcd from disk.  Precaching a .vcd calls into FindOrAddBlock which moves the .vcd to the head of the LRU if it's in memory
//  or it reloads it from disk otherwise.
//-----------------------------------------------------------------------------
class CAsyncCaptionResourceManager : public CAutoGameSystem, public CManagedDataCacheClient< AsyncCaptionData_t, asynccaptionparams_t >
{
public:
	CAsyncCaptionResourceManager() : CAutoGameSystem( "CAsyncCaptionResourceManager" )
	{
	}

	void		SetDbInfo( const CUtlVector< AsyncCaption_t > & info )
	{
		m_Db = info;
	}

	virtual bool Init()
	{
		CCacheClientBaseClass::Init( datacache, "Captions", MAX_ASYNCCAPTION_MEMORY_CACHE );
		return true;
	}
	virtual void Shutdown()
	{
		Clear();
		CCacheClientBaseClass::Shutdown();
	}

	//-----------------------------------------------------------------------------
	// Purpose: Spew a cache summary to the console
	//-----------------------------------------------------------------------------
	void SpewMemoryUsage()
	{
		GetCacheSection()->OutputReport();

		DataCacheStatus_t status;
		DataCacheLimits_t limits;
		GetCacheSection()->GetStatus( &status, &limits );
		int bytesUsed = status.nBytes;
		int bytesTotal = limits.nMaxBytes;

		float percent = 100.0f * (float)bytesUsed / (float)bytesTotal;

		int count = 0;
		for ( int i = 0; i < m_Db.Count(); ++i )
		{
			count += m_Db[ i ].m_RequestedBlocks.Count();
		}

		DevMsg( "CAsyncCaptionResourceManager:  %i blocks total %s, %.2f %% of capacity\n", count, Q_pretifymem( bytesUsed, 2 ), percent );
	}

	virtual void LevelInitPostEntity()
	{
	}

	void CaptionAsyncLoaderCallback( const FileAsyncRequest_t &request, int numReadBytes, FSAsyncStatus_t asyncStatus )
	{	
		// get our preserved data
		AsyncCaptionData_t *pData = ( AsyncCaptionData_t * )request.pContext;

		Assert( pData );

		// mark as completed in single atomic operation
		pData->m_bLoadCompleted = true;
	}

	int ComputeBlockOffset( int fileIndex, int blockNum )
	{
		return m_Db[ fileIndex ].m_Header.dataoffset + blockNum * m_Db[ fileIndex ].m_Header.blocksize;
	}

	void GetBlockInfo( int fileIndex, int blockNum, bool& entry, bool& pending, bool& loaded )
	{
		pending = false;
		loaded = false;
		AsyncCaption_t::BlockInfo_t search;
		search.fileindex = fileIndex;
		search.blocknum = blockNum;

		CUtlRBTree< AsyncCaption_t::BlockInfo_t, unsigned short >& requested = m_Db[ fileIndex ].m_RequestedBlocks;

		int idx = requested.Find( search );
		if ( idx == requested.InvalidIndex() )
		{
			entry = false;
			return;
		}
		entry = true;

		DataCacheHandle_t handle = requested[ idx ].handle;
		AsyncCaptionData_t	*pCaptionData = CacheLock( handle );
		if ( pCaptionData )
		{
			if ( pCaptionData->m_bLoadPending )
			{
				pending = true;
			}
			else if ( pCaptionData->m_bLoadCompleted )
			{
				loaded = true;
			}
			CacheUnlock( handle );
		}
	}

	// Either commences async loading or polls for async loading once per frame to wait for it to complete...
	void PollForAsyncLoading( CHudCloseCaption *hudCloseCaption, int dbFileIndex, int blockNum )
	{
		const char *dbname = m_Db[ dbFileIndex ].m_DataBaseFile.String();

		CUtlRBTree< AsyncCaption_t::BlockInfo_t, unsigned short >& requested = m_Db[ dbFileIndex ].m_RequestedBlocks;

		int idx = FindOrAddBlock( dbFileIndex, blockNum );
		if ( idx == requested.InvalidIndex() )
		{
			Assert( 0 );
			return;
		}

		DataCacheHandle_t handle = requested[ idx ].handle;

		AsyncCaptionData_t	*pCaptionData = CacheLock( handle );
		if ( !pCaptionData )
		{
			// Try and reload it
			char fn[ 256 ];
			Q_strncpy( fn, dbname, sizeof( fn ) );
			Q_FixSlashes( fn );

			asynccaptionparams_t params;
			params.dbfile		= fn;
			params.blocktoload	= blockNum;
			params.blocksize	= m_Db[ dbFileIndex ].m_Header.blocksize;
			params.blockoffset	= ComputeBlockOffset( dbFileIndex, blockNum );
			params.fileindex    = dbFileIndex;

			handle = requested[ idx ].handle = CacheCreate( params );
			pCaptionData = CacheLock( handle );
			if ( !pCaptionData )
			{
				Assert( pCaptionData );
				return;
			}
		}

		if ( pCaptionData->m_bLoadCompleted )
		{
			pCaptionData->m_bLoadPending = false;
			// Copy in data at this point
			Assert( hudCloseCaption );
			if ( hudCloseCaption )
			{
				hudCloseCaption->OnFinishAsyncLoad( requested[ idx ].fileindex, requested[ idx ].blocknum, pCaptionData );
			}

			// This finalizes the load (unlocks the handle)
			GetCacheSection()->BreakLock( handle );
			return;
		}

		if ( pCaptionData->m_bLoadPending )
		{
			CacheUnlock( handle );
			return;
		}

		// Commence load (locks handle for entire async load) (unlocked above)
		pCaptionData->m_bLoadPending = true;
		pCaptionData->AsyncLoad( dbname, ComputeBlockOffset( dbFileIndex, blockNum ) );
	}
	
	//-----------------------------------------------------------------------------
	// Purpose: Touch the cache or load the scene into the cache for the first time
	// Input  : *filename - 
	//-----------------------------------------------------------------------------
	int FindOrAddBlock( int dbFileIndex, int blockNum )
	{
		const char *dbname = m_Db[ dbFileIndex ].m_DataBaseFile.String();

		CUtlRBTree< AsyncCaption_t::BlockInfo_t, unsigned short >& requested = m_Db[ dbFileIndex ].m_RequestedBlocks;

		AsyncCaption_t::BlockInfo_t search;
		search.blocknum = blockNum;
		search.fileindex = dbFileIndex;

		int idx = requested.Find( search );
		if ( idx != requested.InvalidIndex() )
		{
			// Move it to head of LRU
			CacheTouch( requested[ idx ].handle );
			return idx;
		}

		char fn[ 256 ];
		Q_strncpy( fn, dbname, sizeof( fn ) );
		Q_FixSlashes( fn );

		asynccaptionparams_t params;
		params.dbfile		= fn;
		params.blocktoload	= blockNum;
		params.blockoffset	= ComputeBlockOffset( dbFileIndex, blockNum );
		params.blocksize	= m_Db[ dbFileIndex ].m_Header.blocksize;
		params.fileindex    = dbFileIndex;

		memhandle_t handle = CacheCreate( params );

		AsyncCaption_t::BlockInfo_t info;
		info.fileindex = dbFileIndex;
		info.blocknum = blockNum;
		info.handle = handle;
        
		// Add scene filename to dictionary
		idx = requested.Insert( info );
		return idx;
	}

	void Flush()
	{
		CacheFlush();
	}

	void Clear()
	{
		for ( int file = 0; file < m_Db.Count(); ++file )
		{
			CUtlRBTree< AsyncCaption_t::BlockInfo_t, unsigned short >& requested = m_Db[ file ].m_RequestedBlocks;

			int c = requested.Count();
			for ( int i = 0; i  < c; ++i )
			{
				memhandle_t dat = requested[ i ].handle;
				CacheRemove( dat );
			}

			requested.RemoveAll();
		}
	}

private:
	
	CUtlVector< AsyncCaption_t >				m_Db;
};

CAsyncCaptionResourceManager g_AsyncCaptionResourceManager;

void CaptionAsyncLoaderCallback( const FileAsyncRequest_t &request, int numReadBytes, FSAsyncStatus_t asyncStatus )
{
	g_AsyncCaptionResourceManager.CaptionAsyncLoaderCallback( request, numReadBytes, asyncStatus );
}

DECLARE_HUDELEMENT( CHudCloseCaption );

DECLARE_HUD_MESSAGE( CHudCloseCaption, CloseCaption );
#ifdef HOE_DLL
DECLARE_HUD_MESSAGE( CHudCloseCaption, RescindClosedCaption );
#endif // HOE_DLL

CHudCloseCaption::CHudCloseCaption( const char *pElementName )
	: CHudElement( pElementName ), 
	vgui::Panel( NULL, "HudCloseCaption" ),
	m_CloseCaptionRepeats( 0, 0, CaptionTokenLessFunc ),
	m_CurrentLanguage( UTL_INVAL_SYMBOL ),
	m_bPaintDebugInfo( false )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	m_nGoalHeight = 0;
	m_nCurrentHeight = 0;
	m_flGoalAlpha = 1.0f;
	m_flCurrentAlpha = 1.0f;

	m_flGoalHeightStartTime = 0;
	m_flGoalHeightFinishTime = 0;

	m_bLocked = false;
	m_bVisibleDueToDirect = false;

	SetPaintBorderEnabled( false );
	SetPaintBackgroundEnabled( false );

	vgui::ivgui()->AddTickSignal( GetVPanel(), 0 );

	if ( !IsX360() )
	{
		g_pVGuiLocalize->AddFile( "resource/closecaption_%language%.txt", "GAME", true );
	}

	HOOK_HUD_MESSAGE( CHudCloseCaption, CloseCaption );
#ifdef HOE_DLL
	HOOK_HUD_MESSAGE( CHudCloseCaption, RescindClosedCaption );
#endif // HOE_DLL

	char uilanguage[ 64 ];
	engine->GetUILanguage( uilanguage, sizeof( uilanguage ) );

	if ( !Q_stricmp( uilanguage, "english" ) )
	{
		english.SetValue( 1 );
	}
	else
	{
		english.SetValue( 0 );
	}

	char dbfile [ 512 ];
	Q_snprintf( dbfile, sizeof( dbfile ), "resource/closecaption_%s.dat", uilanguage );
	InitCaptionDictionary( dbfile );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CHudCloseCaption::~CHudCloseCaption()
{
	m_CloseCaptionRepeats.RemoveAll();

	ClearAsyncWork();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *newmap - 
//-----------------------------------------------------------------------------
void CHudCloseCaption::LevelInit( void )
{
	CreateFonts();
	// Reset repeat counters per level
	m_CloseCaptionRepeats.RemoveAll();

	// Wipe any stale pending work items...
	ClearAsyncWork();
}

static ConVar cc_minvisibleitems( "cc_minvisibleitems", "1", 0, "Minimum number of caption items to show." );

void CHudCloseCaption::TogglePaintDebug()
{
	m_bPaintDebugInfo = !m_bPaintDebugInfo;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudCloseCaption::Paint( void )
{
	int w, h;
	GetSize( w, h );

	if ( m_bPaintDebugInfo )
	{
		int blockWide = 350;
		int startx = 50;
		
		int y = 0;
		int size = 8;
		int sizewithgap = size + 1;

		for ( int a = 0; a < m_AsyncCaptions.Count(); ++a )
		{
			int x = startx;

			int c = m_AsyncCaptions[ a ].m_Header.numblocks;
			for ( int i = 0 ; i < c; ++i )
			{
				bool entry, pending, loaded;
				g_AsyncCaptionResourceManager.GetBlockInfo( a, i, entry, pending, loaded );

				if ( !entry )
				{
					vgui::surface()->DrawSetColor( Color( 0, 0, 0, 127 ) );
				}
				else if ( pending )
				{
					vgui::surface()->DrawSetColor( Color( 0, 0, 255, 127 ) );
				}
				else if ( loaded )
				{
					vgui::surface()->DrawSetColor( Color( 0, 255, 0, 127 ) );
				}
				else
				{
					vgui::surface()->DrawSetColor( Color( 255, 255, 0, 127 ) );
				}

				vgui::surface()->DrawFilledRect( x, y, x + size, y + size );
				x += sizewithgap;
				if ( x >= startx + blockWide )
				{
					x = startx;
					y += sizewithgap;
				}
			}

			y += sizewithgap;
		}
	}

	wrect_t rcOutput;
	rcOutput.left = 0;
	rcOutput.right = w;
	rcOutput.bottom = h;
	rcOutput.top = m_nTopOffset;

	wrect_t rcText = rcOutput;

	int avail_width = rcText.right - rcText.left - 2 * CC_INSET;
#ifdef HOE_DLL
	avail_width -= GetIconSize();
#endif
	int avail_height = rcText.bottom - rcText.top - 2 * CC_INSET;

	int totalheight = 0;
	int i;
	CUtlVector< VisibleStreamItem > visibleitems;
	int c = m_Items.Count();
	int maxwidth = 0;

#ifdef HOE_DLL
	bool bNoSFX = false;
	CUtlVector< CCloseCaptionItem * > candidates;
	for  ( i = 0; i < c; i++ )
	{
		CCloseCaptionItem *item = m_Items[ i ];

		// Not ready for display yet.
		if ( item->GetPreDisplayTime() > 0.0f )
		{
			continue;
		}

		bool bExclusive = ( item->GetInfo() && (item->GetInfo()->flags & ClosedCaptionInfo_t::CCI_EXCLUSIVE) );
		if ( bExclusive )
		{
			candidates.RemoveAll();
		}

		if ( item->GetInfo() && (item->GetInfo()->flags & ClosedCaptionInfo_t::CCI_NOSFX) )
		{
			bNoSFX = true;
		}

		if ( !item->GetSizeComputed() )
		{
			ComputeStreamWork( avail_width, item );
		}

		// If we get more room than we had while previously drawing this item, use it.
		// But don't make room for lines that have already faded away.
		int nDisplayLines = item->GetNumLines() - item->GetFadedLines();
		nDisplayLines = min( nDisplayLines, 4 );
		item->SetDisplayLines( nDisplayLines );
		item->SetCollapsedLines( item->GetNumLines() - nDisplayLines );

		candidates.AddToTail( item );

		if ( bExclusive )
			break;
	}

	if ( bNoSFX )
	{
		for ( i = 0; i < candidates.Count(); i++ )
		{
			CCloseCaptionItem *item = m_Items[ i ];
			if ( item->GetSFX() == true )
			{
				candidates.Remove( i );
				--i;
			}
		}
	}

	int nCollapseAttempts = 1;
	while ( true )
	{
		int iTotalHeight = ComputeHeightOfItems( candidates );
		if ( iTotalHeight <= avail_height )
			break;
		if ( CollapseDisplayItems( candidates, nCollapseAttempts ) == false )
		{
			// Remove the oldest non-important item because it can't fit
			bool bRemoved = false;
			for ( i = 0; i < candidates.Count(); i++ )
			{
				if ( candidates[i]->GetInfo() && (candidates[i]->GetInfo()->flags & ClosedCaptionInfo_t::CCI_IMPORTANT) )
					continue;
				candidates.Remove( i );
				bRemoved = true;
				break;
			}
			// Couldn't remove any non-important items...
			if ( !bRemoved )
				break;

			// Reset the number of display lines of each item to maximum and try again
			for ( i = 0; i < candidates.Count(); i++ )
			{
				int nDisplayLines = candidates[i]->GetNumLines() - candidates[i]->GetFadedLines();
				nDisplayLines = min( nDisplayLines, 4 );
				candidates[i]->SetDisplayLines( nDisplayLines );
				candidates[i]->SetCollapsedLines( candidates[i]->GetNumLines() - nDisplayLines );
			}
			nCollapseAttempts = 1;
		}
	}

	for ( i = 0; i < candidates.Count(); i++ )
	{
		CCloseCaptionItem *item = candidates[i];

		int itemwidth = item->GetWidth();
		int itemheight = item->GetDisplayHeight();

		totalheight += itemheight;
		if ( itemwidth > maxwidth )
		{
			maxwidth = itemwidth;
		}

		VisibleStreamItem si;
		si.height = itemheight;
		si.width = itemwidth;
		si.item = item;

		visibleitems.AddToTail( si );
	}
#else // HOE_DLL
	for  ( i = 0; i < c; i++ )
	{
		CCloseCaptionItem *item = m_Items[ i ];

		// Not ready for display yet.
		if ( item->GetPreDisplayTime() > 0.0f )
		{
			continue;
		}

		if ( !item->GetSizeComputed() )
		{
			ComputeStreamWork( avail_width, item );
		}

		int itemwidth = item->GetWidth();
		int itemheight = item->GetHeight();

		totalheight += itemheight;
		if ( itemwidth > maxwidth )
		{
			maxwidth = itemwidth;
		}

		VisibleStreamItem si;
		si.height = itemheight;
		si.width = itemwidth;
		si.item = item;

		visibleitems.AddToTail( si );

		// Start popping really old items off the stack if we run out of space
		while ( itemheight <= avail_height && 
				totalheight > avail_height && 
				visibleitems.Count() > cc_minvisibleitems.GetInt() )
		{
			VisibleStreamItem & pop = visibleitems[ 0 ];
			totalheight -= pop.height;

			// And make it die right away...
			pop.item->SetTimeToLive( 0.0f );

			visibleitems.Remove( 0 );
		}	
	}
#endif // HOE_DLL

	float desiredAlpha = visibleitems.Count() >= 1 ? 1.0f : 0.0f;

	// Always return at least one line height for drawing the surrounding box
	totalheight = MAX( totalheight, m_nLineHeight ); 

	// Trigger box growing
	if ( totalheight != m_nGoalHeight )
	{
		m_nGoalHeight = totalheight;
		m_flGoalHeightStartTime = gpGlobals->curtime;
		m_flGoalHeightFinishTime = gpGlobals->curtime + m_flGrowTime;
	}
	if ( desiredAlpha != m_flGoalAlpha )
	{
		m_flGoalAlpha = desiredAlpha;
		m_flGoalHeightStartTime = gpGlobals->curtime;
		m_flGoalHeightFinishTime = gpGlobals->curtime + m_flGrowTime;
	}

	// If shrunk to zero and faded out, nothing left to do
	if ( !visibleitems.Count() &&
		m_nGoalHeight == m_nCurrentHeight &&
		m_flGoalAlpha == m_flCurrentAlpha )
	{
		m_flGoalHeightStartTime = 0;
		m_flGoalHeightFinishTime = 0;
		return;
	}

	bool growingDown = false;

	// Continue growth?
	if ( m_flGoalHeightFinishTime &&
		m_flGoalHeightStartTime &&
		m_flGoalHeightFinishTime > m_flGoalHeightStartTime )
	{
		float togo = m_nGoalHeight - m_nCurrentHeight;
		float alphatogo = m_flGoalAlpha - m_flCurrentAlpha;

		growingDown = togo < 0.0f ? true : false;

		float dt = m_flGoalHeightFinishTime - m_flGoalHeightStartTime;
		float frac = ( gpGlobals->curtime - m_flGoalHeightStartTime ) / dt;
		frac = clamp( frac, 0.0f, 1.0f );
		int newHeight = m_nCurrentHeight + (int)( frac * togo );
		m_nCurrentHeight = newHeight;
		float newAlpha = m_flCurrentAlpha + frac * alphatogo;
		m_flCurrentAlpha = clamp( newAlpha, 0.0f, 1.0f );
	}
	else
	{
		m_nCurrentHeight = m_nGoalHeight;
		m_flCurrentAlpha = m_flGoalAlpha;
	}

	rcText.top = rcText.bottom - m_nCurrentHeight - 2 * CC_INSET;
 
	Color bgColor = GetBgColor();
   	bgColor[3] = m_flBackgroundAlpha;
	DrawBox( rcText.left, MAX(rcText.top,0), rcText.right - rcText.left, rcText.bottom - MAX(rcText.top,0), bgColor, m_flCurrentAlpha );

	if ( !visibleitems.Count() )
	{
		return;
	}

	rcText.left += CC_INSET;
#ifdef HOE_DLL
	rcText.left += GetIconSize();
#endif
	rcText.right -= CC_INSET;

	int textHeight = m_nCurrentHeight;
	if ( growingDown )
	{
		// If growing downward, keep the text locked to the bottom of the window instead of anchored to the top
		textHeight = totalheight;
	}

	rcText.top = rcText.bottom - textHeight - CC_INSET;

	// Now draw them
	c = visibleitems.Count();
	for ( i = 0; i < c; i++ )
	{
		VisibleStreamItem *si = &visibleitems[ i ];

		// If the oldest/top item was created with additional time, we can remove that now
		if ( i == 0 )
		{
			if ( si->item->GetAddedTime() > 0.0f )
			{
				float ttl = si->item->GetTimeToLive();
				ttl -= si->item->GetAddedTime();
				ttl = MAX( 0.0f, ttl );
				si->item->SetTimeToLive( ttl );
				si->item->SetAddedTime( 0.0f );
			}
		}

		int height = si->height;
 		CCloseCaptionItem *item = si->item;
		 
		int iFadeLine = -1;
		float flFadeLineAlpha = 1.0;
	
#ifdef HOE_DLL
		int iTopOffset = 0;

		// If the item height is greater than the display height of the item, 
		// we need to slowly pan over this item. 
		if ( height < item->GetHeight() )
		{
			// Figure out how many lines we'll need to move to see the whole caption
#if 1
			int iLines = item->GetCollapsedLines();
#else
			int iLines = item->GetNumLines();
 			int iTotalMove = 0;
			for ( int j = 0; j < iLines; j++ )
			{
				CCloseCaptionLine *line = item->GetLine( j );
				iTotalMove += line->GetHeight();
				if ( iTotalMove >= (item->GetHeight() - height) )
				{
					iLines = j+1;
					break;
				}
			}
#endif
			// Figure out the delta between each point where we move the line
			float flAnimTime = 0 /*iLines * (CAPTION_PAN_SLIDE_TIME + CAPTION_PAN_FADE_TIME)*/;
			float flLifeSpan = item->GetInitialLifeSpan() - flAnimTime;
			float flMoveDelta = flLifeSpan / (float)item->GetNumLines();
 			float flCurMove = flLifeSpan - (item->GetTimeToLive() - flAnimTime);
 			int iHeightToMove = 0;

 			int iLinesToMove = clamp( floor( flCurMove / flMoveDelta ), 0, iLines );
			if ( iLinesToMove )
			{
 				int iCurrentLineHeight = 0;
				for ( int j = 0; j < iLinesToMove; j++ )
				{
					iHeightToMove = iCurrentLineHeight;

					CCloseCaptionLine *line = item->GetLine( j );
  					iCurrentLineHeight += line->GetHeight();
				}

				// Slide to the desired distance, once the fade is done
	 			float flTimePostMove = flCurMove - (flMoveDelta * iLinesToMove);
 				if ( flTimePostMove < CAPTION_PAN_FADE_TIME )
				{
					iFadeLine = iLinesToMove-1;

					// It's time to fade out the top line. If it hasn't started fading yet, start it.
					CCloseCaptionLine *line = item->GetLine(iFadeLine);
					if ( line->GetFadeStart() == 0 )
					{
						line->SetFadeStart( gpGlobals->curtime );
					}

					// Fade out quickly
					float flFadeTime = (gpGlobals->curtime - line->GetFadeStart()) /  CAPTION_PAN_FADE_TIME;
					flFadeLineAlpha = clamp( 1.0 - flFadeTime, 0, 1 );
				}
				else if ( flTimePostMove < (CAPTION_PAN_FADE_TIME+CAPTION_PAN_SLIDE_TIME) )
				{
					flTimePostMove -= CAPTION_PAN_FADE_TIME;
 					float flSlideTime = clamp( flTimePostMove / 0.25, 0, 1 );
 					iHeightToMove += ceil((iCurrentLineHeight - iHeightToMove) * flSlideTime);
				}
				else
				{
					item->SetFadedLines( iLinesToMove );
					iHeightToMove = iCurrentLineHeight;
				}
			}

 			iTopOffset = iHeightToMove;
		}

		rcText.bottom = rcText.top + height;

		wrect_t rcOut = rcText;
		rcOut.top -= iTopOffset;
		rcOut.right = rcOut.left + si->width + 6;
		
		CHudTexture *pIcon = item->GetIcon();
		if ( pIcon != NULL && pIcon->textureId != -1 )
		{
			int x = rcOut.left - GetIconSize() - CC_INSET / 2;
			int y = rcText.top + (rcText.bottom - rcText.top - GetIconSize()) / 2;

			float alpha = item->GetAlpha( m_flItemHiddenTime, m_flItemFadeInTime, m_flItemFadeOutTime );

			vgui::surface()->DrawSetColor( Color( 102, 92, 76, 200 * alpha ) );
			vgui::surface()->DrawFilledRect( x, y, x + GetIconSize(), y + GetIconSize() );

			vgui::surface()->DrawSetTexture( pIcon->textureId );
			Color clr = gHUD.m_clrNormal;
			clr[3] = 255 * alpha;
			vgui::surface()->DrawSetColor( clr );
			vgui::surface()->DrawTexturedSubRect( 
				x, y, 
				x + GetIconSize(), y + GetIconSize(), 
				0, 0, 
				1, 1 );
//			pIcon->DrawSelfCropped( rcOut.left - 24 - CC_INSET / 2, rcText.top, 0, 0, 48, 48, 24, 24, gHUD.m_clrNormal );
		}

		DrawStream( rcOut, rcText, item, iFadeLine, flFadeLineAlpha );

#else // HOE_DLL
		// If the height is greater than the total height of the element, 
		// we need to slowly pan over this item. 
		if ( height > avail_height )
		{
			// Figure out how many lines we'll need to move to see the whole caption
			int units = item->GetNumWorkUnits();
 			int iTotalMove = 0;
			for ( int j = 0 ; j < units; j++ )
			{
				CCloseCaptionWorkUnit *wu = item->GetWorkUnit( j );
				iTotalMove += wu->GetHeight();
				if ( iTotalMove >= (height - avail_height) )
				{
					units = j+1;
					break;
				}
			}

			// Figure out the delta between each point where we move the line
			float flMoveDelta = item->GetInitialLifeSpan() / (float)units;
 			float flCurMove = item->GetInitialLifeSpan() - item->GetTimeToLive();
 			int iHeightToMove = 0;

 			int iLinesToMove = clamp( Floor2Int( flCurMove / flMoveDelta ), 0, units );
			if ( iLinesToMove )
			{
 				int iCurrentLineHeight = 0;
				for ( int j = 0 ; j < iLinesToMove; j++ )
				{
					iHeightToMove = iCurrentLineHeight;

					CCloseCaptionWorkUnit *wu = item->GetWorkUnit( j );
  					iCurrentLineHeight += wu->GetHeight();
				}

				// Slide to the desired distance, once the fade is done
	 			float flTimePostMove = flCurMove - (flMoveDelta * iLinesToMove);
 				if ( flTimePostMove < CAPTION_PAN_FADE_TIME )
				{
					iFadeLine = iLinesToMove-1;

					// It's time to fade out the top line. If it hasn't started fading yet, start it.
					CCloseCaptionWorkUnit *wu = item->GetWorkUnit(iFadeLine);
					if ( wu->GetFadeStart() == 0 )
					{
						wu->SetFadeStart( gpGlobals->curtime );
					}

					// Fade out quickly
					float flFadeTime = (gpGlobals->curtime - wu->GetFadeStart()) /  CAPTION_PAN_FADE_TIME;
					flFadeLineAlpha = clamp( 1.0f - flFadeTime, 0.f, 1.f );
				}
				else if ( flTimePostMove < (CAPTION_PAN_FADE_TIME+CAPTION_PAN_SLIDE_TIME) )
				{
					flTimePostMove -= CAPTION_PAN_FADE_TIME;
 					float flSlideTime = clamp( flTimePostMove / 0.25f, 0.f, 1.f );
 					iHeightToMove += ceil((iCurrentLineHeight - iHeightToMove) * flSlideTime);
				}
				else
				{
					iHeightToMove = iCurrentLineHeight;
				}
			}

			// Minor adjustment to center the caption text within the window.
 			rcText.top = -iHeightToMove + 2;
		}

		rcText.bottom = rcText.top + height;
 
		wrect_t rcOut = rcText;
 
		rcOut.right = rcOut.left + si->width + 6;
		
		DrawStream( rcOut, rcOutput, item, iFadeLine, flFadeLineAlpha );
#endif // HOE_DLL

		rcText.top += height;
		rcText.bottom += height;

		if ( rcText.top >= rcOutput.bottom )
			break;
	}
}

void CHudCloseCaption::OnTick( void )
{
	// See if any async work has completed
	ProcessAsyncWork();


	float dt = gpGlobals->frametime;

	int c = m_Items.Count();
	int i;

	if ( m_bVisibleDueToDirect )
	{
		SetVisible( true );
		if ( !c )
		{
			// Don't clear our force visible if we're waiting for the caption to load
			if ( m_AsyncWork.Count() == 0 )
			{
				m_bVisibleDueToDirect = false;
			}
		}
	}
	else
	{
		SetVisible( closecaption.GetBool() );
	}

	// Pass one decay all timers
	for ( i = 0 ; i < c ; ++i )
	{
		CCloseCaptionItem *item = m_Items[ i ];

		float predisplay = item->GetPreDisplayTime();
		if ( predisplay > 0.0f )
		{
			predisplay -= dt;
			predisplay = MAX( 0.0f, predisplay );
			item->SetPreDisplayTime( predisplay );
		}
		else
		{
			// remove time from actual playback
			float ttl = item->GetTimeToLive();
			ttl -= dt;
			ttl = MAX( 0.0f, ttl );
			item->SetTimeToLive( ttl );
		}
	}

#ifdef HOE_DLL
	for ( i = 0 ; i < c ; ++i )
	{
		CCloseCaptionItem *item = m_Items[ i ];

		// Skip items not yet showing...
		float predisplay = item->GetPreDisplayTime();
		if ( predisplay > 0.0f )
		{
			continue;
		}

		float ttl = item->GetTimeToLive();
		if ( ttl > 0.0f )
		{
			continue;
		}

		delete item;
		m_Items.Remove( i );
		--i;
		--c;
	}
#else // HOE_DLL
	// Pass two, remove from head until we get to first item with time remaining
	bool foundfirstnondeletion = false;
	for ( i = 0 ; i < c ; ++i )
	{
		CCloseCaptionItem *item = m_Items[ i ];

		// Skip items not yet showing...
		float predisplay = item->GetPreDisplayTime();
		if ( predisplay > 0.0f )
		{
			continue;
		}

		float ttl = item->GetTimeToLive();
		if ( ttl > 0.0f )
		{
			foundfirstnondeletion = true;
			continue;
		}

		// Skip the remainder of the items after we find the first/oldest active item
		if ( foundfirstnondeletion )
		{
			continue;
		}

		delete item;
		m_Items.Remove( i );
		--i;
		--c;
	}
#endif // HOE_DLL
}

void CHudCloseCaption::Reset( void )
{
	while ( m_Items.Count() > 0 )
	{
		CCloseCaptionItem *i = m_Items[ 0 ];
		delete i;
		m_Items.Remove( 0 );
	}

	ClearAsyncWork();
	Unlock();
}

bool CHudCloseCaption::SplitCommand( wchar_t const **ppIn, wchar_t *cmd, wchar_t *args ) const
{
	const wchar_t *in = *ppIn;
	const wchar_t *oldin = in;

	if ( in[0] != L'<' )
	{
		*ppIn += ( oldin - in );
		return false;
	}

	args[ 0 ] = 0;
	cmd[ 0 ]= 0;
	wchar_t *out = cmd;
	in++;
	while ( *in != L'\0' && *in != L':' && *in != L'>' && !isspace( *in ) )
	{
		*out++ = *in++;
	}
	*out = L'\0';

	if ( *in != L':' )
	{
		*ppIn += ( in - oldin );
		return true;
	}

	in++;
	out = args;
	while ( *in != L'\0' && *in != L'>' )
	{
		*out++ = *in++;
	}
	*out = L'\0';

	//if ( *in == L'>' )
	//	in++;

	*ppIn += ( in - oldin );
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *stream - 
//			*findcmd - 
//			value - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CHudCloseCaption::GetFloatCommandValue( const wchar_t *stream, const wchar_t *findcmd, float& value ) const
{
	const wchar_t *curpos = stream;
	
	for ( ; curpos && *curpos != L'\0'; ++curpos )
	{
		wchar_t cmd[ 256 ];
		wchar_t args[ 256 ];

		if ( SplitCommand( &curpos, cmd, args ) )
		{
			if ( !wcscmp( cmd, findcmd ) )
			{
				value = (float)wcstod( args, NULL );
				return true;
			}
			continue;
		}
	}

	return false;
}


bool CHudCloseCaption::StreamHasCommand( const wchar_t *stream, const wchar_t *findcmd ) const
{
	const wchar_t *curpos = stream;
	
	for ( ; curpos && *curpos != L'\0'; ++curpos )
	{
		wchar_t cmd[ 256 ];
		wchar_t args[ 256 ];

		if ( SplitCommand( &curpos, cmd, args ) )
		{
			if ( !wcscmp( cmd, findcmd ) )
			{
				return true;
			}
			continue;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: It's blank or only comprised of whitespace/space characters...
// Input  : *stream - 
// Output : static bool
//-----------------------------------------------------------------------------
static bool IsAllSpaces( const wchar_t *stream )
{
	const wchar_t *p = stream;
	while ( *p != L'\0' )
	{
		if ( !iswspace( *p ) )
			return false;

		p++;
	}

	return true;
}

bool CHudCloseCaption::StreamHasCommand( const wchar_t *stream, const wchar_t *search )
{
	for ( const wchar_t *curpos = stream; curpos && *curpos != L'\0'; ++curpos )
	{
		wchar_t cmd[ 256 ];
		wchar_t args[ 256 ];

		if ( SplitCommand( &curpos, cmd, args ) )
		{
			if ( !wcscmp( cmd, search ) )
			{
				return true;
			}
		}
	}
	return false;
}

#ifdef HOE_DLL
void CHudCloseCaption::Process( const wchar_t *stream, float duration, const char *tokenstream, const char *image, short id, const ClosedCaptionInfo_t *pInfo, bool fromplayer, bool direct )
#else // HOE_DLL
void CHudCloseCaption::Process( const wchar_t *stream, float duration, const char *tokenstream, bool fromplayer, bool direct )
#endif // HOE_DLL
{
	if ( !direct )
	{
		if ( !closecaption.GetBool() )
		{
			Reset();
			return;
		}

		// If we're locked, ignore all closecaption commands
		if ( m_bLocked )
			return;
	}

	// Nothing to do...
	if ( IsAllSpaces( stream) )
	{
		return;
	}

	// If subtitling, don't show sfx captions at all
	if ( cc_subtitles.GetBool() && StreamHasCommand( stream, L"sfx" ) )
	{
		return;
	}

	bool valid = true;
	if ( !wcsncmp( stream, L"!!!", wcslen( L"!!!" ) ) )
	{
		// It's in the text file, but hasn't been translated...
		valid = false;
	}

	if ( !wcsncmp( stream, L"-->", wcslen( L"-->" ) ) )
	{
		// It's in the text file, but hasn't been translated...
		valid = false;

		if ( cc_captiontrace.GetInt() < 2 )
		{
			if ( cc_captiontrace.GetInt() == 1 )
			{
				Msg( "Missing caption for '%s'\n", tokenstream );
			}

			return;
		}
	}

	float lifespan = duration + cc_linger_time.GetFloat();
	
	float addedlife = 0.0f;

#ifndef HOE_DLL
	if ( m_Items.Count() > 0 )
	{
		// Get the remaining life span of the last item
		CCloseCaptionItem *final = m_Items[ m_Items.Count() - 1 ];
		float prevlife = final->GetTimeToLive();

		if ( prevlife > lifespan )
		{
			addedlife = prevlife - lifespan;
		}

		lifespan = MAX( lifespan, prevlife );
	}
#endif // HOE_DLL

	float delay = 0.0f;
	float override_duration = 0.0f;

	wchar_t phrase[ MAX_CAPTION_CHARACTERS ];
	wchar_t *out = phrase;

	for ( const wchar_t *curpos = stream; curpos && *curpos != L'\0'; ++curpos )
	{
		wchar_t cmd[ 256 ];
		wchar_t args[ 256 ];

		const wchar_t *prevpos = curpos;

		if ( SplitCommand( &curpos, cmd, args ) )
		{
			if ( !wcscmp( cmd, L"delay" ) )
			{

				// End current phrase
				*out = L'\0';

				if ( wcslen( phrase ) > 0 )
				{
#ifdef HOE_DLL
//					delay -= cc_predisplay_time.GetFloat() - m_flItemHiddenTime;
//					if ( delay < 0 ) delay = 0;
					CCloseCaptionItem *item = new CCloseCaptionItem( phrase, image, id, pInfo, lifespan, addedlife, delay, valid, fromplayer );
#else // HOE_DLL
					CCloseCaptionItem *item = new CCloseCaptionItem( phrase, lifespan, addedlife, delay, valid, fromplayer );
#endif // HOE_DLL
					m_Items.AddToTail( item );
					if ( StreamHasCommand( phrase, L"sfx" ) )
					{
						// SFX show up instantly.
						item->SetPreDisplayTime( 0.0f );
#ifdef HOE_DLL
						item->SetSFX( true );
#endif // HOE_DLL
					}
					
					if ( GetFloatCommandValue( phrase, L"len", override_duration ) )
					{
						item->SetTimeToLive( override_duration );
#ifdef HOE_DLL
						item->SetInitialLifeSpan( override_duration );
#endif // HOE_DLL
					}
				}

				// Start new phrase
				out = phrase;

				// Delay must be positive
				delay = MAX( 0.0f, (float)wcstod( args, NULL ) );

				continue;
			}

			int copychars = curpos - prevpos;
			while ( --copychars >= 0 )
			{
				*out++ = *prevpos++;
			}
		}

		*out++ = *curpos;
	}

	// End final phrase, if any
	*out = L'\0';
	if ( wcslen( phrase ) > 0 )
	{
#ifdef HOE_DLL
		CCloseCaptionItem *item = new CCloseCaptionItem( phrase, image, id, pInfo, lifespan, addedlife, delay, valid, fromplayer );
#else // HOE_DLL
		CCloseCaptionItem *item = new CCloseCaptionItem( phrase, lifespan, addedlife, delay, valid, fromplayer );
#endif // HOE_DLL
		m_Items.AddToTail( item );

		if ( StreamHasCommand( phrase, L"sfx" ) )
		{
			// SFX show up instantly.
			item->SetPreDisplayTime( 0.0f );
#ifdef HOE_DLL
			item->SetSFX( true );
#endif // HOE_DLL
		}

		if ( GetFloatCommandValue( phrase, L"len", override_duration ) )
		{
			item->SetTimeToLive( override_duration );
			item->SetInitialLifeSpan( override_duration );
		}
	}
}

void CHudCloseCaption::CreateFonts( void )
{
	vgui::IScheme *pScheme = vgui::scheme()->GetIScheme( GetScheme() );

	m_hFonts[CCFONT_NORMAL] = pScheme->GetFont( "CloseCaption_Normal" );

	if ( IsPC() )
	{
		m_hFonts[CCFONT_BOLD] = pScheme->GetFont( "CloseCaption_Bold" );
		m_hFonts[CCFONT_ITALIC] = pScheme->GetFont( "CloseCaption_Italic" );
		m_hFonts[CCFONT_ITALICBOLD] = pScheme->GetFont( "CloseCaption_BoldItalic" );
	}
	else
	{
		m_hFonts[CCFONT_SMALL] = pScheme->GetFont( "CloseCaption_Small" );
	}

	m_nLineHeight = MAX( 6, vgui::surface()->GetFontTall( m_hFonts[ CCFONT_NORMAL ] ) );
}

struct WorkUnitParams
{
	WorkUnitParams()
	{
		Q_memset( stream, 0, sizeof( stream ) );
		out = stream;
		x = 0;
		y = 0;
		width = 0;
		bold = italic = false;
		clr = Color( 255, 255, 255, 255 );
		newline = false;
		font = 0;
	}

	~WorkUnitParams()
	{
	}

	void Finalize( int lineheight )
	{
		*out = L'\0';
	}

	void Next( int lineheight )
	{
		// Restart output
		Q_memset( stream, 0, sizeof( stream ) );
		out = stream;

		x += width;

		width = 0;
		// Leave bold, italic and color alone!!!
		if ( newline )
		{
			newline = false;
			x = 0;
			y += lineheight;
		}
	}

	int GetFontNumber()
	{
		return CHudCloseCaption::GetFontNumber( bold, italic );
	}

	wchar_t	stream[ MAX_CAPTION_CHARACTERS ];
	wchar_t	*out;

	int		x;
	int		y;
	int		width;
	bool	bold;
	bool	italic;
	Color clr;
	bool	newline;
	vgui::HFont font;
};

void CHudCloseCaption::AddWorkUnit( CCloseCaptionItem *item,
	WorkUnitParams& params )
{
	params.Finalize( vgui::surface()->GetFontTall( params.font ) );

#ifdef WIN32
	if ( wcslen( params.stream ) > 0 )
#else
	// params.stream is still in ucs2 format here so just do a basic zero compare for length or just space
	if ( ((uint16 *)params.stream)[0] != 0 && ((uint16 *)params.stream)[0] != 32  )		
#endif
	{
		CCloseCaptionWorkUnit *wu = new CCloseCaptionWorkUnit();

		wu->SetStream( params.stream );
		wu->SetColor( params.clr );
		wu->SetBold( params.bold );
		wu->SetItalic( params.italic );
		wu->SetWidth( params.width );
		wu->SetHeight( vgui::surface()->GetFontTall( params.font ) );
		wu->SetPos( params.x, params.y );
		wu->SetFont( params.font );
#ifndef HOE_DLL
		wu->SetFadeStart( 0 );
#endif // HOE_DLL
		int curheight = item->GetHeight();
		int curwidth = item->GetWidth();

		curheight = MAX( curheight, params.y + wu->GetHeight() );
		curwidth = MAX( curwidth, params.x + params.width );

		item->SetHeight( curheight );
		item->SetWidth( curwidth );

		// Add it
#ifdef HOE_DLL
		CCloseCaptionLine *line;
		if ( item->GetNextLineIsNew() == false && item->GetNumLines() > 0 )
			line = item->GetLine( item->GetNumLines() - 1 );
		else
		{
			line = new CCloseCaptionLine;
			line->SetFadeStart( 0 );
			line->SetPos( params.x, params.y );
			item->AddLine( line );
		}

		curheight = line->GetHeight();
		curwidth = line->GetWidth();

		curheight = max( curheight, wu->GetHeight() );
		curwidth = max( curwidth, params.x + wu->GetWidth() );

		line->SetHeight( curheight );
		line->SetWidth( curwidth );

		line->AddWork( wu );

		item->SetNextLineIsNew( params.newline );

		params.Next( line->GetHeight() );
#else // HOE_DLL
		item->AddWork( wu );

		params.Next( vgui::surface()->GetFontTall( params.font ) );
#endif // HOE_DLL
	}
}

void CHudCloseCaption::ComputeStreamWork( int available_width, CCloseCaptionItem *item )
{
	// Start with a clean param block
	WorkUnitParams params;

	const wchar_t *curpos = item->GetStream();
	int streamlen = wcslen( curpos );
	CUtlVector< Color > colorStack;

	const wchar_t *most_recent_space = NULL;
	int	most_recent_space_w = -1;

	for ( ; curpos && *curpos != L'\0'; ++curpos )
	{
		wchar_t cmd[ 256 ];
		wchar_t args[ 256 ];

		if ( SplitCommand( &curpos, cmd, args ) )
		{
			if ( !wcscmp( cmd, L"cr" ) )
			{
				params.newline = true;
				AddWorkUnit( item, params);
			}
			else if ( !wcscmp( cmd, L"clr" ) )
			{
				AddWorkUnit( item, params );

				if ( args[0] == 0 && colorStack.Count()>= 2)
				{
					colorStack.Remove( colorStack.Count() - 1 );
					params.clr = colorStack[ colorStack.Count() - 1 ];
				}
				else
				{
					int r = 0, g = 0, b = 0;
					Color newcolor;
					if ( 3 == swscanf( args, L"%i,%i,%i", &r, &g, &b ) )
					{
						newcolor = Color( r, g, b, 255 );
						colorStack.AddToTail( newcolor );
						params.clr = colorStack[ colorStack.Count() - 1 ];
					}
				}
			}
			else if ( !wcscmp( cmd, L"playerclr" ) )
			{
				AddWorkUnit( item, params );

				if ( args[0] == 0 && colorStack.Count()>= 2)
				{
					colorStack.Remove( colorStack.Count() - 1 );
					params.clr = colorStack[ colorStack.Count() - 1 ];
				}
				else
				{
					// player and npc color selector
					// e.g.,. 255,255,255:200,200,200
					int pr = 0, pg = 0, pb = 0, nr = 0, ng = 0, nb = 0;
					Color newcolor;
					if ( 6 == swscanf( args, L"%i,%i,%i:%i,%i,%i", &pr, &pg, &pb, &nr, &ng, &nb ) )
					{
						newcolor = item->IsFromPlayer() ? Color( pr, pg, pb, 255 ) : Color( nr, ng, nb, 255 );
						colorStack.AddToTail( newcolor );
						params.clr = colorStack[ colorStack.Count() - 1 ];
					}
				}
			}
			else if ( !wcscmp( cmd, L"I" ) )
			{
				AddWorkUnit( item, params );
				params.italic = !params.italic;
			}
			else if ( !wcscmp( cmd, L"B" ) )
			{
				AddWorkUnit( item, params );
				params.bold = !params.bold;
			}
#ifdef HOE_DLL
			else if ( !wcscmp( cmd, L"icon" ) )
			{
				char imagename[128];
				Q_wcstostr( args, -1, imagename, sizeof(imagename) ),
				item->SetIcon( imagename );
			}
#endif // HOE_DLL
			continue;
		}

		int font;
		if ( IsPC() )
		{
			font = params.GetFontNumber();
		}
		else
		{
			font = streamlen >= cc_smallfontlength.GetInt() ? CCFONT_SMALL : CCFONT_NORMAL;
		}
		vgui::HFont useF = m_hFonts[font];
		params.font = useF;

		int w, h;

		wchar_t sz[2];
		sz[ 0 ] = *curpos;
		sz[ 1 ] = L'\0';
		vgui::surface()->GetTextSize( useF, sz, w, h );

		if ( ( params.x + params.width ) + w > available_width )
		{
			if ( most_recent_space && curpos >= most_recent_space + 1 )
			{
				// Roll back to previous space character if there is one...
				int goback = curpos - most_recent_space - 1;
				params.out -= ( goback + 1 );
				params.width = most_recent_space_w;
				
				wchar_t *extra = new wchar_t[ goback + 1 ];
				wcsncpy( extra, most_recent_space + 1, goback );
				extra[ goback ] = L'\0';

				params.newline = true;
				AddWorkUnit( item, params );

				wcsncpy( params.out, extra, goback );
				params.out += goback;
				int textw, texth;
				vgui::surface()->GetTextSize( useF, extra, textw, texth );

				params.width = textw;

				delete[] extra;

				most_recent_space = NULL;
				most_recent_space_w = -1;
			}
			else
			{
				params.newline = true;
				AddWorkUnit( item, params );
			}
		}
		*params.out++ = *curpos;
		params.width += w;

		if ( iswspace( *curpos ) )
		{
			most_recent_space = curpos;
			most_recent_space_w = params.width;
		}
	}

	// Add the final unit.
	params.newline = true;
	AddWorkUnit( item, params );

#ifdef HOE_DLL
	item->SetDisplayLines( item->GetNumLines() );
	item->SetCollapsedLines( 0 );
#endif // HOE_DLL

	item->SetSizeComputed( true );

	// DumpWork( item );
}

void CHudCloseCaption::	DumpWork( CCloseCaptionItem *item )
{
#ifdef HOE_DLL
	int c = item->GetNumLines();
	for ( int i = 0 ; i < c; ++i )
	{
		CCloseCaptionLine *line = item->GetLine( i );
		line->Dump();
	}
#else // HOE_DLL
	int c = item->GetNumWorkUnits();
	for ( int i = 0 ; i < c; ++i )
	{
		CCloseCaptionWorkUnit *wu = item->GetWorkUnit( i );
		wu->Dump();
	}
#endif // HOE_DLL
}

void CHudCloseCaption::DrawStream( wrect_t &rcText, wrect_t &rcWindow, CCloseCaptionItem *item, int iFadeLine, float flFadeLineAlpha )
{
#ifdef HOE_DLL

#if 1
	if ( item->GetNumLines() > 1 )
	{
		// For multiline items, highlight the "current" playing line based on display time.
		float flTimePerLine = item->GetInitialLifeSpan() / item->GetNumLines();
		int iFocusLine = floor((item->GetInitialLifeSpan() - item->GetTimeToLive()) / flTimePerLine);
		iFocusLine = clamp( iFocusLine, 0, item->GetNumLines() - 1 );
		wrect_t rcFocus = rcText;
		CCloseCaptionLine *line = item->GetLine( iFocusLine );
		rcFocus.top += line->GetTop();
		rcFocus.bottom = rcFocus.top + line->GetHeight();
		if ( rcFocus.top >= rcWindow.top && rcFocus.bottom <= rcWindow.bottom )
		{
			vgui::surface()->DrawSetColor( Color( 128, 128, 128, 64 ) );
			vgui::surface()->DrawFilledRect( rcFocus.left, rcFocus.top, rcFocus.right, rcFocus.bottom );

			// Draw an outline around all the lines
			vgui::surface()->DrawSetColor( Color( 255, 255, 255, 64 ) );
			vgui::surface()->DrawOutlinedRect( rcWindow.left - CC_INSET - GetIconSize(),
				rcWindow.top, rcWindow.right + CC_INSET, rcWindow.bottom );
		}
	}

	// Center the lines vertically in case the icon is taller than the text.
	int iTextOffsetY = 0;
	int iTextHeight = item->GetNumLines() * m_nLineHeight;
	if ( iTextHeight < rcText.bottom - rcText.top )
		iTextOffsetY = ((rcText.bottom - rcText.top) - iTextHeight) / 2;
#endif


	wrect_t rcOut;

	float alpha = item->GetAlpha( m_flItemHiddenTime, m_flItemFadeInTime, m_flItemFadeOutTime );

	int numLines = item->GetNumLines();
	for ( int i = 0; i < numLines; ++i )
	{
		CCloseCaptionLine *line = item->GetLine( i );

		int numUnits = line->GetNumWorkUnits();
		for ( int j = 0; j < numUnits; ++j )
		{
			CCloseCaptionWorkUnit *wu = line->GetWorkUnit( j );

			int x, y;
#else // HOE_DLL
	int c = item->GetNumWorkUnits();

	wrect_t rcOut;

	float alpha = item->GetAlpha( m_flItemHiddenTime, m_flItemFadeInTime, m_flItemFadeOutTime );

	for ( int i = 0 ; i < c; ++i )
	{
		int x = 0;
		int y = 0;

		CCloseCaptionWorkUnit *wu = item->GetWorkUnit( i );
#endif // HOE_DLL
		vgui::HFont useF = wu->GetFont();

		wu->GetPos( x, y );

		rcOut.left = rcText.left + x + 3;
		rcOut.right = rcOut.left + wu->GetWidth();
		rcOut.top = rcText.top + y;
#ifdef HOE_DLL
		rcOut.top += iTextOffsetY;
#endif
   		rcOut.bottom = rcOut.top + wu->GetHeight();

		// Adjust alpha to handle fade in/out at the top & bottom of the element.
		// Used for single commentary entries that are too big to fit into the element.
		float flLineAlpha = alpha;
		if ( i == iFadeLine )
		{
			flLineAlpha *= flFadeLineAlpha;
		}
#ifdef HOE_DLL
		else if ( rcOut.top < rcWindow.top || rcOut.bottom > rcWindow.bottom )
#else // HOE_DLL
		else if ( rcOut.top < rcWindow.top )
#endif // HOE_DLL
		{
			// We're off the top of the element, so don't draw
			continue;
		}
		else if ( rcOut.top > rcWindow.bottom )
		{
			float flFadeHeight = (float)wu->GetHeight() * 0.25;
			float flDist = (float)(rcOut.top - rcWindow.bottom) / flFadeHeight;
			flDist = Bias( flDist, 0.2 );
			if ( flDist > 1 )
				continue;

			flLineAlpha *= 1.0 - flDist;
		}

		Color useColor = wu->GetColor();

		useColor[ 3 ] *= flLineAlpha;

		if ( !item->IsValid() )
		{
			useColor = Color( 255, 255, 255, 255 * flLineAlpha );
			rcOut.right += 2;
			vgui::surface()->DrawSetColor( Color( 100, 100, 40, 255 * flLineAlpha ) );
			vgui::surface()->DrawFilledRect( rcOut.left, rcOut.top, rcOut.right, rcOut.bottom );
		}

		vgui::surface()->DrawSetTextFont( useF );
		vgui::surface()->DrawSetTextPos( rcOut.left, rcOut.top );
		vgui::surface()->DrawSetTextColor( useColor );
		vgui::surface()->DrawPrintText( wu->GetStream(), wcslen( wu->GetStream() ) );
	}
#ifdef HOE_DLL
	}
#endif // HOE_DLL
}

bool CHudCloseCaption::GetNoRepeatValue( const wchar_t *caption, float &retval )
{
	retval = 0.0f;
	const wchar_t *curpos = caption;
	
	for ( ; curpos && *curpos != L'\0'; ++curpos )
	{
		wchar_t cmd[ 256 ];
		wchar_t args[ 256 ];

		if ( SplitCommand( &curpos, cmd, args ) )
		{
			if ( !wcscmp( cmd, L"norepeat" ) )
			{
				retval = (float)wcstod( args, NULL );
				return true;
			}
			continue;
		}
	}
	return false;
}

bool CHudCloseCaption::CaptionTokenLessFunc( const CaptionRepeat &lhs, const CaptionRepeat &rhs )
{ 
	return ( lhs.m_nTokenIndex < rhs.m_nTokenIndex );	
}

static bool CaptionTrace( const char *token )
{
	static CUtlSymbolTable s_MissingCloseCaptions;

	// Make sure we only show the message once
	if ( UTL_INVAL_SYMBOL == s_MissingCloseCaptions.Find( token ) )
	{
		s_MissingCloseCaptions.AddString( token );
		return true;
	}

	return false;
}

static ConVar cc_sentencecaptionnorepeat( "cc_sentencecaptionnorepeat", "4", 0, "How often a sentence can repeat." );

int CRCString( const char *str )
{
	int len = Q_strlen( str );
	CRC32_t crc;
	CRC32_Init( &crc );
	CRC32_ProcessBuffer( &crc, str, len );
	CRC32_Final( &crc );

	return ( int )crc;
}

class CAsyncCaption
{
public:
	CAsyncCaption() : 
		m_flDuration( 0.0f ),
		m_bIsStream( false ),
		m_bFromPlayer( false )
	{
#ifdef HOE_DLL
		m_szImageName[0] = '\0';
		m_nCaptionID = 0;
#endif // HOE_DLL
	}

	~CAsyncCaption()
	{
		int c = m_Tokens.Count();
		for ( int i = 0; i < c; ++i )
		{
			delete m_Tokens[ i ];
		}
		m_Tokens.Purge();
	}

	void StartRequesting( CHudCloseCaption *hudCloseCaption, CUtlVector< AsyncCaption_t >& directories )
	{
		// Issue pending async requests for each token in string
		int c = m_Tokens.Count();
		for ( int i = 0; i < c; ++i )
		{
			caption_t *caption = m_Tokens[ i ];
			Assert( !caption->stream );
			Assert( caption->dirindex >= 0 );

			CaptionLookup_t& entry = directories[ caption->fileindex ].m_CaptionDirectory[ caption->dirindex ];

			// Request this block, and if it's there, it'll call OnDataLoaded immediately
			g_AsyncCaptionResourceManager.PollForAsyncLoading( hudCloseCaption, caption->fileindex, entry.blockNum );
		}
	}

	void OnDataArrived( CUtlVector< AsyncCaption_t >& directories, int nFileIndex, int nBlockNum, AsyncCaptionData_t *pData )
	{
		int c = m_Tokens.Count();
		for ( int i = 0; i < c; ++i )
		{
			caption_t *caption = m_Tokens[ i ];
			if ( caption->stream != NULL )
				continue;

#ifdef HOE_DLL // BUG IN SDK ???
			if ( caption->fileindex != nFileIndex )
				continue;
#endif // HOE_DLL

			// Lookup the data
			CaptionLookup_t &entry = directories[ nFileIndex ].m_CaptionDirectory[ caption->dirindex ];
			if ( entry.blockNum != nBlockNum )
				continue;

#ifdef WIN32
			const wchar_t *pIn = ( const wchar_t *)&pData->m_pBlockData[ entry.offset ];
			caption->stream = new wchar_t[ entry.length >> 1 ];
			memcpy( (void *)caption->stream, pIn, entry.length );
#else
			// we persist to disk as ucs2 so convert back to real unicode here
			caption->stream = new wchar_t[ entry.length ];
			V_UCS2ToUnicode( (ucs2 *)&pData->m_pBlockData[ entry.offset ], caption->stream, entry.length*sizeof(wchar_t) );	
#endif
		}
	}

	void ProcessAsyncWork( CHudCloseCaption *hudCloseCaption, CUtlVector< AsyncCaption_t >& directories )
	{
		int c = m_Tokens.Count();
		for ( int i = 0; i < c; ++i )
		{
			caption_t *caption = m_Tokens[ i ];
			if ( caption->stream != NULL )
				continue;

			CaptionLookup_t& entry = directories[ caption->fileindex].m_CaptionDirectory[ caption->dirindex ];

			// Request this block, and if it's there, it'll call OnDataLoaded immediately
			g_AsyncCaptionResourceManager.PollForAsyncLoading( hudCloseCaption, caption->fileindex, entry.blockNum );
		}
	}

	bool GetStream( OUT_Z_BYTECAP(bufSizeInBytes) wchar_t *buf, int bufSizeInBytes )
	{
		Assert( bufSizeInBytes >= sizeof(buf[0]) );
		buf[ 0 ] = L'\0';

		int c = m_Tokens.Count();
		for ( int i = 0; i < c; ++i )
		{
			caption_t *caption = m_Tokens[ i ];
			if ( caption->stream == NULL )
			{
				return false;
			}
		}

		unsigned int curlen = 0;
		unsigned int maxlen = bufSizeInBytes / sizeof( wchar_t );

		// Compose full stream from tokens
		for ( int i = 0; i < c; ++i )
		{
			caption_t *caption = m_Tokens[ i ];
			int len = wcslen( caption->stream ) + 1;
			if ( curlen + len >= maxlen )
				break;

			wcscat( buf, caption->stream );
			if ( i < c - 1 ) 
			{
				wcscat( buf, L" " );
			}

			curlen += len;
		}

		return true;
	}

	bool					IsStream() const
	{
		return m_bIsStream;
	}

	void					SetIsStream( bool state )
	{
		m_bIsStream = state;
	}

#ifdef HOE_DLL
	void SetImageName( const char *imagename )
	{
		if ( imagename == NULL )
			return;
		Q_strncpy( m_szImageName, imagename, sizeof(m_szImageName) );
	}
	const char *GetImageName( void ) const
	{
		return m_szImageName;
	}
	char m_szImageName[128];

	const ClosedCaptionInfo_t *GetInfo( void ) const
	{
		return m_pInfo;
	}
	const ClosedCaptionInfo_t *m_pInfo;

	void SetCaptionID( short id )
	{
		m_nCaptionID = id;
	}
	short GetCaptionID( void ) const
	{
		return m_nCaptionID;
	}
	short m_nCaptionID;

#endif // HOE_DLL

	void	AddRandomToken( CUtlVector< AsyncCaption_t >& directories )
	{
		int dc = directories.Count();
		int fileindex = RandomInt( 0, dc - 1 );

		int c = directories[ fileindex ].m_CaptionDirectory.Count();
		int idx = RandomInt( 0, c - 1 );

		caption_t *caption = new caption_t;
		char foo[ 32 ];
		Q_snprintf( foo, sizeof( foo ), "%d", idx );
		caption->token = strdup( foo );
		caption->dirindex = idx;
		caption->stream = NULL;
		caption->fileindex = fileindex;

		m_Tokens.AddToTail( caption );
	}

	bool AddToken
	( 
		CUtlVector< AsyncCaption_t >& directories, 
		const char *token 
	)
	{
		CaptionLookup_t search;
		search.SetHash( token );

		int idx = -1;
		int i;
		int dc = directories.Count();
		for ( i = 0; i < dc; ++i )
		{
            idx = directories[ i ].m_CaptionDirectory.Find( search );
			if ( idx == directories[ i ].m_CaptionDirectory.InvalidIndex() )
				continue;

			break;
		}

		if ( i >= dc || idx == -1 )
			return false;

		caption_t *caption = new caption_t;
		caption->token = strdup( token );
		caption->dirindex = idx;
		caption->stream = NULL;
		caption->fileindex = i;

#ifdef HOE_DLL
		if ( m_Tokens.Count() == 0 )
		{
			m_pInfo = closedcaptionsinfo->GetCaptionInfo( token );
		}
#endif // HOE_DLL

		m_Tokens.AddToTail( caption );
		return true;
	}

	int						Count() const
	{
		return m_Tokens.Count();
	}

	const char				*GetToken( int index )
	{
		return m_Tokens[ index ]->token;
	}

	void					GetOriginalStream( char *buf, size_t bufsize )
	{
		buf[ 0 ] = 0;
		int c = Count();
		for ( int i = 0 ; i < c; ++i )
		{
			Q_strncat( buf, GetToken( i ), bufsize, COPY_ALL_CHARACTERS );
			if ( i != c - 1 )
			{
				Q_strncat( buf, " ", bufsize, COPY_ALL_CHARACTERS );
			}
		}
	}

	void					SetDuration( float t )
	{
		m_flDuration = t;
	}

	float					GetDuration()
	{
		return m_flDuration;
	}

	bool					IsFromPlayer()
	{
		return m_bFromPlayer;
	}

	void					SetFromPlayer( bool state )
	{
		m_bFromPlayer = state;
	}

	bool					IsDirect()
	{
		return m_bDirect;
	}

	void					SetDirect( bool state )
	{
		m_bDirect = state;
	}
private:
	float					m_flDuration;
	bool					m_bIsStream : 1;
	bool					m_bFromPlayer : 1;
	bool					m_bDirect : 1;

	struct caption_t
	{
		caption_t() :
			token( 0 ),
			dirindex( -1 ),
			fileindex( -1 ),
			stream( 0 )
		{
		}

		~caption_t()
		{
			free( token );
			delete[] stream;
		}

		void		SetStream( const wchar_t *in )
		{
			delete[] stream;
			stream = 0;
			if ( !in )
				return;

			int len = wcslen( in );
			stream = new wchar_t[ len + 1 ];
			wcsncpy( stream, in, len + 1 );
		}
			
		char		*token;
		int			dirindex;
		int			fileindex;
		wchar_t		*stream;
	};

	CUtlVector< caption_t * > m_Tokens;
};

void CHudCloseCaption::ProcessAsyncWork()
{
	int i;
	for( i = m_AsyncWork.Head(); i != m_AsyncWork.InvalidIndex(); i = m_AsyncWork.Next( i ) )
	{
		// check for data arrival
		CAsyncCaption *item = m_AsyncWork[ i ];
		item->ProcessAsyncWork( this, m_AsyncCaptions );
	}
	// Now operate on any new data which arrived
	for( i = m_AsyncWork.Head(); i != m_AsyncWork.InvalidIndex();  )
	{
		int n = m_AsyncWork.Next( i );

		CAsyncCaption *item = m_AsyncWork[ i ];
		wchar_t stream[ MAX_CAPTION_CHARACTERS ];

		// If we get to the first item with pending async work, stop processing
		if ( !item->GetStream( stream, sizeof( stream ) ) )
		{
			break;
		}

		if ( stream[ 0 ] != L'\0' )
		{
			char original[ 512 ];
			item->GetOriginalStream( original, sizeof( original ) );

			// Process it now
			if ( item->IsStream() )
			{
#ifdef HOE_DLL
				_ProcessSentenceCaptionStream( item->Count(), original, item->GetImageName(), item->GetCaptionID(), item->GetInfo(), stream );
			}
			else
			{
				_ProcessCaption( stream, original, item->GetImageName(), item->GetCaptionID(), item->GetInfo(), item->GetDuration(), item->IsFromPlayer(), item->IsDirect() );
#else // HOE_DLL
				_ProcessSentenceCaptionStream( item->Count(), original, stream );
			}
			else
			{
				_ProcessCaption( stream, original, item->GetDuration(), item->IsFromPlayer(), item->IsDirect() );
#endif // HOE_DLL
			}
		}

		m_AsyncWork.Remove( i );
		delete item;

		i = n;
	}
}

void CHudCloseCaption::ClearAsyncWork()
{
	for ( int i = m_AsyncWork.Head(); i != m_AsyncWork.InvalidIndex(); i = m_AsyncWork.Next( i ) )
	{
        CAsyncCaption *item = m_AsyncWork[ i ];
		delete item;
	}
	m_AsyncWork.Purge();
}

extern void Hack_FixEscapeChars( char *str );

#ifdef HOE_DLL
void CHudCloseCaption::ProcessCaptionDirect( const char *tokenname, const char *imagename, float duration, bool fromplayer /* = false */ )
#else // HOE_DLL
void CHudCloseCaption::ProcessCaptionDirect( const char *tokenname, float duration, bool fromplayer /* = false */ )
#endif // HOE_DLL
{
	m_bVisibleDueToDirect = true;

	char token[ 512 ];
	Q_strncpy( token, tokenname, sizeof( token ) );
	if ( Q_strstr( token, "\\" ) )
	{
		Hack_FixEscapeChars( token );
	}

#ifdef HOE_DLL
	ProcessCaption( token, imagename, duration, fromplayer, true );
#else // HOE_DLL
	ProcessCaption( token, duration, fromplayer, true );
#endif // HOE_DLL
}

void CHudCloseCaption::PlayRandomCaption()
{
	CAsyncCaption *async = new CAsyncCaption;
	async->SetIsStream( false );
	async->AddRandomToken( m_AsyncCaptions );
	async->SetDuration( RandomFloat( 1.0f, 3.0f ) );
	async->SetFromPlayer( RandomInt( 0, 1 ) == 0 ? true : false );
	async->StartRequesting( this, m_AsyncCaptions );
	m_AsyncWork.AddToTail( async );
}

#ifdef HOE_DLL
bool CHudCloseCaption::AddAsyncWork( const char *tokenstream, const char *imagename, short id, bool bIsStream, float duration, bool fromplayer, bool direct /* = false */ )
#else // HOE_DLL
bool CHudCloseCaption::AddAsyncWork( const char *tokenstream, bool bIsStream, float duration, bool fromplayer, bool direct /* = false */ )
#endif // HOE_DLL
{
	bool bret = true;

	CAsyncCaption *async = new CAsyncCaption();
	async->SetIsStream( bIsStream );
	async->SetDirect( direct );
#ifdef HOE_DLL
	async->SetImageName( imagename );
	async->SetCaptionID( id );
#endif // HOE_DLL
	if ( !bIsStream )
	{
		bret = async->AddToken
		( 
			m_AsyncCaptions, 
			tokenstream 
		);
	}
	else
	{
		// The first token from the stream is the name of the sentence
		char tokenname[ 512 ];
		tokenname[ 0 ] = 0;
		const char *p = tokenstream;
		p = nexttoken( tokenname, p, ' ' );
		// p points to reset of sentence tokens, build up a unicode string from them...
		while ( p && Q_strlen( tokenname ) > 0 )
		{
			p = nexttoken( tokenname, p, ' ' );

			if ( Q_strlen( tokenname ) == 0 )
				break;

			async->AddToken
			( 
					m_AsyncCaptions, 
					tokenname 
			);
		}
	}

	m_AsyncWork.AddToTail( async );

	async->SetDuration( duration );
	async->SetFromPlayer( fromplayer );
	// Do this last as the block might be resident already and this will finish immediately...
	async->StartRequesting( this, m_AsyncCaptions );
	return bret;
}


#ifdef HOE_DLL
void CHudCloseCaption::ProcessSentenceCaptionStream( const char *tokenstream, const char *imagename, short id )
#else // HOE_DLL
void CHudCloseCaption::ProcessSentenceCaptionStream( const char *tokenstream )
#endif // HOE_DLL
{
	float interval = cc_sentencecaptionnorepeat.GetFloat();
	interval = clamp( interval, 0.1f, 60.0f );

	// The first token from the stream is the name of the sentence
	char tokenname[ 512 ];

	tokenname[ 0 ] = 0;

	const char *p = tokenstream;

	p = nexttoken( tokenname, p, ' ' );

	if ( Q_strlen( tokenname ) > 0 )
	{
		//  Use it to check for "norepeat" rules
		CaptionRepeat entry;
		entry.m_nTokenIndex = CRCString( tokenname );

		int idx = m_CloseCaptionRepeats.Find( entry );
		if ( m_CloseCaptionRepeats.InvalidIndex() == idx )
		{
			entry.m_flLastEmitTime = gpGlobals->curtime;
			entry.m_nLastEmitTick = gpGlobals->tickcount;
			entry.m_flInterval = interval;
			m_CloseCaptionRepeats.Insert( entry );
		}
		else
		{
			CaptionRepeat &entry = m_CloseCaptionRepeats[ idx ];
			if ( gpGlobals->curtime < ( entry.m_flLastEmitTime + entry.m_flInterval ) )
			{
				return;
			}

			entry.m_flLastEmitTime = gpGlobals->curtime;
			entry.m_nLastEmitTick = gpGlobals->tickcount;
		}
	}

#ifdef HOE_DLL
	AddAsyncWork( tokenstream, imagename, id, true, 0.0f, false );
#else // HOE_DLL
	AddAsyncWork( tokenstream, true, 0.0f, false );
#endif // HOE_DLL
}

#ifdef HOE_DLL
void CHudCloseCaption::_ProcessSentenceCaptionStream( int wordCount, const char *tokenstream, const char *imagename, short id, const ClosedCaptionInfo_t *pInfo, const wchar_t *caption_full )
{
	if ( wcslen( caption_full ) > 0 )
	{
		Process( caption_full, ( wordCount + 1 ) * 0.75f, tokenstream, imagename, id, pInfo, false /*never from player!*/ );
	}
}

bool CHudCloseCaption::ProcessCaption( const char *tokenname, const char *imagename, short id, float duration, bool fromplayer /* = false */, bool direct /* = false */ )
{
	return AddAsyncWork( tokenname, imagename, id, false, duration, fromplayer, direct );
}
#else // HOE_DLL
void CHudCloseCaption::_ProcessSentenceCaptionStream( int wordCount, const char *tokenstream, const wchar_t *caption_full )
{
	if ( wcslen( caption_full ) > 0 )
	{
		Process( caption_full, ( wordCount + 1 ) * 0.75f, tokenstream, false /*never from player!*/ );
	}
}

bool CHudCloseCaption::ProcessCaption( const char *tokenname, float duration, bool fromplayer /* = false */, bool direct /* = false */ )
{
	return AddAsyncWork( tokenname, false, duration, fromplayer, direct );
}
#endif // HOE_DLL

#ifdef HOE_DLL
void CHudCloseCaption::_ProcessCaption( const wchar_t *caption, const char *tokenname, const char *imagename, short id, const ClosedCaptionInfo_t *pInfo, float duration, bool fromplayer, bool direct )
#else // HOE_DLL
void CHudCloseCaption::_ProcessCaption( const wchar_t *caption, const char *tokenname, float duration, bool fromplayer, bool direct )
#endif // HOE_DLL
{
	// Get the string for the token
	float interval = 0.0f;
	bool hasnorepeat = GetNoRepeatValue( caption, interval );

	CaptionRepeat entry;
	entry.m_nTokenIndex = CRCString( tokenname );

	int idx = m_CloseCaptionRepeats.Find( entry );
	if ( m_CloseCaptionRepeats.InvalidIndex() == idx )
	{
		entry.m_flLastEmitTime = gpGlobals->curtime;
		entry.m_nLastEmitTick = gpGlobals->tickcount;
		entry.m_flInterval = interval;
		m_CloseCaptionRepeats.Insert( entry );
	}
	else
	{
		CaptionRepeat &entry = m_CloseCaptionRepeats[ idx ];

		// Interval of 0.0 means just don't double emit on same tick #
		if ( entry.m_flInterval <= 0.0f )
		{
			if ( gpGlobals->tickcount <= entry.m_nLastEmitTick )
			{
				return;
			}
		}
		else if ( hasnorepeat )
		{
			if ( gpGlobals->curtime < ( entry.m_flLastEmitTime + entry.m_flInterval ) )
			{
				return;
			}
		}

		entry.m_flLastEmitTime = gpGlobals->curtime;
		entry.m_nLastEmitTick = gpGlobals->tickcount;
	}

#ifdef HOE_DLL
	Process( caption, duration, tokenname, imagename, id, pInfo, fromplayer, direct );
#else // HOE_DLL
	Process( caption, duration, tokenname, fromplayer, direct );
#endif // HOE_DLL
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pszName - 
//			iSize - 
//			*pbuf - 
//-----------------------------------------------------------------------------
void CHudCloseCaption::MsgFunc_CloseCaption(bf_read &msg)
{
	char tokenname[ 512 ];
	msg.ReadString( tokenname, sizeof( tokenname ) );
#ifdef HOE_DLL
	char imagename[ 512 ];
	msg.ReadString( imagename, sizeof( imagename ) );
	short id = msg.ReadShort();
#endif // HOE_DLL
	float duration = msg.ReadShort() * 0.1f;
	byte flagbyte = msg.ReadByte();
	bool warnonmissing = flagbyte & CLOSE_CAPTION_WARNIFMISSING ? true : false;
	bool fromplayer = flagbyte & CLOSE_CAPTION_FROMPLAYER ? true : false;
	bool bIsMale = flagbyte & CLOSE_CAPTION_GENDER_MALE ? true : false;
	bool bIsFemale = flagbyte & CLOSE_CAPTION_GENDER_FEMALE ? true : false;

	if ( warnonmissing && !IsX360() )
	{
		wchar_t *pcheck = g_pVGuiLocalize->Find( tokenname );
		if ( !pcheck )
		{
			Warning( "No caption found for '%s'\n", tokenname );
		}
	}

	char szTestName[ 512 ];
	if ( bIsMale || bIsFemale )
	{
		Q_snprintf( szTestName, sizeof( szTestName ), "%s_%s", tokenname, bIsMale ? "male" : "female" );
		// If the gender-ified version exists, use it, otherwise fall through and pass the non-gender string to the cc system for processing
#ifdef HOE_DLL
		if ( ProcessCaption( szTestName, imagename, id, duration, fromplayer ) )
#else // HOE_DLL
		if ( ProcessCaption( szTestName , duration, fromplayer ) )
#endif // HOE_DLL
		{
			return;
		}
	}

#ifdef HOE_DLL
	ProcessCaption( tokenname, imagename, id, duration, fromplayer );	
#else // HOE_DLL
	ProcessCaption( tokenname, duration, fromplayer );	
#endif // HOE_DLL
}

#ifdef HOE_DLL
//-----------------------------------------------------------------------------
void CHudCloseCaption::MsgFunc_RescindClosedCaption(bf_read &msg)
{
	short id = msg.ReadShort();

	DevMsg( "RescindClosedCaption %i\n", id );

	for ( int i = m_AsyncWork.Head(); i != m_AsyncWork.InvalidIndex(); )
	{
		int n = m_AsyncWork.Next( i );

		CAsyncCaption *item = m_AsyncWork[ i ];
		if ( item->GetCaptionID() == id )
		{
			delete item;
			m_AsyncWork.Remove( i );
		}

		i = n;
	}

	int c = m_Items.Count();
	for ( int i = 0; i < c; i++ )
	{
		CCloseCaptionItem *item =  m_Items[i];
		if ( item->GetCaptionID() == id )
		{
			delete item; // FIXME: fade out or something...
			m_Items.Remove( i );
			--i;
			--c;
		}
	}
}
#endif // HOE_DLL

int CHudCloseCaption::GetFontNumber( bool bold, bool italic )
{
	if ( IsPC() && ( bold || italic ) )
	{
		if( bold && italic )
		{
			return CHudCloseCaption::CCFONT_ITALICBOLD;
		}

		if ( bold )
		{
			return CHudCloseCaption::CCFONT_BOLD;
		}

		if ( italic )
		{
			return CHudCloseCaption::CCFONT_ITALIC;
		}
	}

	return CHudCloseCaption::CCFONT_NORMAL;
}

void CHudCloseCaption::Flush()
{
	g_AsyncCaptionResourceManager.Flush();
}

#ifdef HOE_DLL
void CHudCloseCaption::InitCaptionDictionary( char const *dbfile, bool bForce )
{
	if ( !bForce && m_CurrentLanguage.IsValid() && !Q_stricmp( m_CurrentLanguage.String(), dbfile ) )
		return;
#else // HOE_DLL
void CHudCloseCaption::InitCaptionDictionary( const char *dbfile )
{
	if ( m_CurrentLanguage.IsValid() && !Q_stricmp( m_CurrentLanguage.String(), dbfile ) )
		return;
#endif // HOE_DLL

	m_CurrentLanguage = dbfile;

	m_AsyncCaptions.Purge();

	g_AsyncCaptionResourceManager.Clear();

	char searchPaths[4096];
	filesystem->GetSearchPath( "GAME", true, searchPaths, sizeof( searchPaths ) );

	for ( char *path = strtok( searchPaths, ";" ); path; path = strtok( NULL, ";" ) )
	{
		if ( IsX360() && ( filesystem->GetDVDMode() == DVDMODE_STRICT ) && !V_stristr( path, ".zip" ) )
		{
			// only want zip paths
			continue;
		} 

		char fullpath[MAX_PATH];
		Q_snprintf( fullpath, sizeof( fullpath ), "%s%s", path, dbfile );
		Q_FixSlashes( fullpath );

		if ( IsX360() )
		{
			char fullpath360[MAX_PATH];
			UpdateOrCreateCaptionFile( fullpath, fullpath360, sizeof( fullpath360 ) );
			Q_strncpy( fullpath, fullpath360, sizeof( fullpath ) );
		}

        FileHandle_t fh = filesystem->Open( fullpath, "rb" );
		if ( FILESYSTEM_INVALID_HANDLE != fh )
		{
			MEM_ALLOC_CREDIT();

			CUtlBuffer dirbuffer;

			AsyncCaption_t& entry = m_AsyncCaptions[ m_AsyncCaptions.AddToTail() ];

			// Read the header
			filesystem->Read( &entry.m_Header, sizeof( entry.m_Header ), fh );
			if ( entry.m_Header.magic != COMPILED_CAPTION_FILEID )
				Error( "Invalid file id for %s\n", fullpath );
			if ( entry.m_Header.version != COMPILED_CAPTION_VERSION )
				Error( "Invalid file version for %s\n", fullpath );
			if ( entry.m_Header.directorysize < 0 || entry.m_Header.directorysize > 64 * 1024 )
				Error( "Invalid directory size %d for %s\n", entry.m_Header.directorysize, fullpath );
			//if ( entry.m_Header.blocksize != MAX_BLOCK_SIZE )
			//	Error( "Invalid block size %d, expecting %d for %s\n", entry.m_Header.blocksize, MAX_BLOCK_SIZE, fullpath );

			int directoryBytes = entry.m_Header.directorysize * sizeof( CaptionLookup_t );
			entry.m_CaptionDirectory.EnsureCapacity( entry.m_Header.directorysize );
			dirbuffer.EnsureCapacity( directoryBytes );
			
			filesystem->Read( dirbuffer.Base(), directoryBytes, fh );
			filesystem->Close( fh );

			entry.m_CaptionDirectory.CopyArray( (const CaptionLookup_t *)dirbuffer.PeekGet(), entry.m_Header.directorysize );
			entry.m_CaptionDirectory.RedoSort( true );

			entry.m_DataBaseFile = fullpath;
		}
	}

	g_AsyncCaptionResourceManager.SetDbInfo( m_AsyncCaptions );
}

void CHudCloseCaption::OnFinishAsyncLoad( int nFileIndex, int nBlockNum, AsyncCaptionData_t *pData )
{
	// Fill in data for all users of pData->m_nBlockNum
	FOR_EACH_LL( m_AsyncWork, i )
	{
		CAsyncCaption *item = m_AsyncWork[ i ];
		item->OnDataArrived( m_AsyncCaptions, nFileIndex, nBlockNum, pData );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudCloseCaption::Lock( void )
{
	if ( !IsXbox() )
		m_bLocked = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudCloseCaption::Unlock( void )
{
	m_bLocked = false;
}

static int EmitCaptionCompletion( const char *partial, char commands[ COMMAND_COMPLETION_MAXITEMS ][ COMMAND_COMPLETION_ITEM_LENGTH ] )
{
	int current = 0;
	if ( !g_pVGuiLocalize || IsX360() )
		return current;

	const char *cmdname = "cc_emit";
	char *substring = NULL;
	int substringLen = 0;
	if ( Q_strstr( partial, cmdname ) && strlen(partial) > strlen(cmdname) + 1 )
	{
		substring = (char *)partial + strlen( cmdname ) + 1;
		substringLen = strlen(substring);
	}
	
	StringIndex_t i = g_pVGuiLocalize->GetFirstStringIndex();

	while ( i != INVALID_LOCALIZE_STRING_INDEX &&
		 current < COMMAND_COMPLETION_MAXITEMS )
	{
		const char *ccname = g_pVGuiLocalize->GetNameByIndex( i );
		if ( ccname )
		{
			if ( !substring || !Q_strncasecmp( ccname, substring, substringLen ) )
			{
				Q_snprintf( commands[ current ], sizeof( commands[ current ] ), "%s %s", cmdname, ccname );
				current++;
			}
		}
		i = g_pVGuiLocalize->GetNextStringIndex( i );
	}

	return current;
}

CON_COMMAND_F_COMPLETION( cc_emit, "Emits a closed caption", 0, EmitCaptionCompletion )
{
	if ( args.ArgC() != 2 )
	{
		Msg( "usage:  cc_emit tokenname\n" );
		return;
	}

	CHudCloseCaption *hudCloseCaption = GET_HUDELEMENT( CHudCloseCaption );
	if ( hudCloseCaption )
	{
#ifdef HOE_DLL
		hudCloseCaption->ProcessCaption( args[1], "", 0, 5.0f );	
#else // HOE_DLL
		hudCloseCaption->ProcessCaption( args[1], 5.0f );	
#endif // HOE_DLL
	}
}

CON_COMMAND( cc_random, "Emits a random caption" )
{
	int count = 1;
	if ( args.ArgC() == 2 )
	{
		count = MAX( 1, atoi( args[ 1 ] ) );
	}
	CHudCloseCaption *hudCloseCaption = GET_HUDELEMENT( CHudCloseCaption );
	if ( hudCloseCaption )
	{
		for ( int i = 0; i < count; ++i )
		{
			hudCloseCaption->PlayRandomCaption();
		}
	}
}


CON_COMMAND( cc_flush, "Flushes async'd captions." )
{
	CHudCloseCaption *hudCloseCaption = GET_HUDELEMENT( CHudCloseCaption );
	if ( hudCloseCaption )
	{
		hudCloseCaption->Flush();
	}
}

CON_COMMAND( cc_showblocks, "Toggles showing which blocks are pending/loaded async." )
{
	CHudCloseCaption *hudCloseCaption = GET_HUDELEMENT( CHudCloseCaption );
	if ( hudCloseCaption )
	{
		hudCloseCaption->TogglePaintDebug();
	}
}

void OnCaptionLanguageChanged( IConVar *pConVar, const char *pOldString, float flOldValue )
{
	if ( !g_pVGuiLocalize )
		return;

	ConVarRef var( pConVar );

	char fn[ 512 ];
	Q_snprintf( fn, sizeof( fn ), "resource/closecaption_%s.txt", var.GetString() );

	// Re-adding the file, even if it's "english" will overwrite the tokens as needed
	if ( !IsX360() )
	{
		g_pVGuiLocalize->AddFile( "resource/closecaption_%language%.txt", "GAME", true );
	}

	char uilanguage[ 64 ];
	engine->GetUILanguage( uilanguage, sizeof( uilanguage ) );

	CHudCloseCaption *hudCloseCaption = GET_HUDELEMENT( CHudCloseCaption );

	// If it's not the default, load the language on top of the user's default language
	if ( Q_strlen( var.GetString() ) > 0 && Q_stricmp( var.GetString(), uilanguage ) )
	{
		if ( !IsX360() )
		{
			if ( g_pFullFileSystem->FileExists( fn ) )
			{
				g_pVGuiLocalize->AddFile( fn, "GAME", true );
			}
			else
			{
				char fallback[ 512 ];
				Q_snprintf( fallback, sizeof( fallback ), "resource/closecaption_%s.txt", uilanguage );

				Msg( "%s not found\n", fn );
				Msg( "%s will be used\n", fallback );
			}
		}

		if ( hudCloseCaption )
		{
			char dbfile [ 512 ];
			Q_snprintf( dbfile, sizeof( dbfile ), "resource/closecaption_%s.dat", var.GetString() );
			hudCloseCaption->InitCaptionDictionary( dbfile );
		}
	}
	else
	{
		if ( hudCloseCaption )
		{
			char dbfile [ 512 ];
			Q_snprintf( dbfile, sizeof( dbfile ), "resource/closecaption_%s.dat", uilanguage );
			hudCloseCaption->InitCaptionDictionary( dbfile );
		}
	}
	DevMsg( "cc_lang = %s\n", var.GetString() );
}



ConVar cc_lang( "cc_lang", "", FCVAR_ARCHIVE, "Current close caption language (emtpy = use game UI language)", OnCaptionLanguageChanged );

#ifdef HOE_DLL
CON_COMMAND( cc_reload, "HOE: reload the close caption file(s)." )
{
	char uilanguage[ 64 ];
	engine->GetUILanguage( uilanguage, sizeof( uilanguage ) );

	CHudCloseCaption *hudCloseCaption = GET_HUDELEMENT( CHudCloseCaption );
	if ( hudCloseCaption )
	{
		char dbfile [ 512 ];
		Q_snprintf( dbfile, sizeof( dbfile ), "resource/closecaption_%s.dat", uilanguage );
		hudCloseCaption->InitCaptionDictionary( dbfile, true );
	}
}
#endif // HOE_DLL

CON_COMMAND( cc_findsound, "Searches for soundname which emits specified text." )
{
	if ( args.ArgC() != 2 )
	{
		Msg( "usage:  cc_findsound 'substring'\n" );
		return;
	}

	CHudCloseCaption *hudCloseCaption = GET_HUDELEMENT( CHudCloseCaption );
	if ( hudCloseCaption )
	{
		hudCloseCaption->FindSound( args.Arg( 1 ) );
	}
}

void CHudCloseCaption::FindSound( char const *pchANSI )
{
	// Now do the searching
	ucs2 stream[ 1024 ];
	char streamANSI[ 1024 ];

	for ( int i = 0 ; i < m_AsyncCaptions.Count(); ++i )
	{
		AsyncCaption_t &data = m_AsyncCaptions[ i ];

		byte *block = new byte[ data.m_Header.blocksize ];

		int nLoadedBlock = -1;

		Q_memset( block, 0, data.m_Header.blocksize );
		CaptionDictionary_t &dict = data.m_CaptionDirectory;
		for ( int j = 0; j < dict.Count(); ++j )
		{
			CaptionLookup_t &lu = dict[ j ];

			int blockNum = lu.blockNum;

			const char *dbname = data.m_DataBaseFile.String();

			// Try and reload it
			char fn[ 256 ];
			Q_strncpy( fn, dbname, sizeof( fn ) );
			Q_FixSlashes( fn );

			asynccaptionparams_t params;
			params.dbfile		= fn;
			params.blocktoload	= blockNum;
			params.blocksize	= data.m_Header.blocksize;
			params.blockoffset	= data.m_Header.dataoffset + blockNum *data.m_Header.blocksize;
			params.fileindex    = i;

			if ( blockNum != nLoadedBlock )
			{
				nLoadedBlock = blockNum;

				FileHandle_t fh = filesystem->Open( fn, "rb" );
				filesystem->Seek( fh, params.blockoffset, FILESYSTEM_SEEK_CURRENT );
				filesystem->Read( block, data.m_Header.blocksize, fh );
				filesystem->Close( fh );
			}

			// Now we have the data
			const ucs2 *pIn = ( const ucs2 *)&block[ lu.offset ];
			Q_memcpy( (void *)stream, pIn, MIN( lu.length, sizeof( stream ) ) );

			// Now search for search text
			V_UCS2ToUTF8( stream, streamANSI, sizeof( streamANSI ) );
			streamANSI[ sizeof( streamANSI ) - 1 ] = 0;

			if ( Q_stristr( streamANSI, pchANSI ) )
			{
				CaptionLookup_t search;

				Msg( "found '%s' in %s\n", streamANSI, fn );

				// Now find the sounds that will hash to this
				for ( int k = soundemitterbase->First(); k != soundemitterbase->InvalidIndex(); k = soundemitterbase->Next( k ) )
				{
					char const *pchSoundName = soundemitterbase->GetSoundName( k );

					// Hash it
					
					search.SetHash( pchSoundName );

					if ( search.hash == lu.hash )
					{
						Msg( "    '%s' matches\n", pchSoundName );
					}
				}

				if ( IsPC() )
				{
					for ( int r = g_pVGuiLocalize->GetFirstStringIndex(); r != INVALID_LOCALIZE_STRING_INDEX; r = g_pVGuiLocalize->GetNextStringIndex( r ) )
					{
						const char *strName = g_pVGuiLocalize->GetNameByIndex( r );

						search.SetHash( strName );

						if ( search.hash == lu.hash )
						{
							Msg( "    '%s' localization matches\n", strName );
						}
					}
				}
			}
		}

		delete[] block;
	}
}

#ifdef HOE_DLL
int CHudCloseCaption::ComputeHeightOfItems( CUtlVector< CCloseCaptionItem *>& items )
{
	int c = items.Count();
	int iTotalHeight = 0;

	for ( int i = 0; i < c; i++ )
	{
		CCloseCaptionItem *item = items[i];
		int iItemHeight = item->GetDisplayLines() * m_nLineHeight; // FIXME: get height of displayed lines
		if ( item->GetIcon() )
			iItemHeight = max( iItemHeight, GetIconSize() );
		item->SetDisplayHeight( iItemHeight );
		iTotalHeight += iItemHeight;
	}

	return iTotalHeight;
}

bool CHudCloseCaption::CollapseDisplayItems( CUtlVector< CCloseCaptionItem *>& items, int& nAttempt )
{
	int c = items.Count();
	bool tryAgain = false;

	for ( int i = 0; i < c; i++ )
	{
		CCloseCaptionItem *item = items[i];
		if ( item->GetDisplayLines() > 1 )
		{
			if ( item->GetCollapsedLines() < nAttempt )
			{
				item->SetDisplayLines( item->GetDisplayLines() - 1 );
				item->SetCollapsedLines( item->GetCollapsedLines() + 1 );
				return true;
			}
			tryAgain = true;
		}
	}

	if ( tryAgain )
		return CollapseDisplayItems( items, ++nAttempt );

	return false;
}
#endif // HOE_DLL
