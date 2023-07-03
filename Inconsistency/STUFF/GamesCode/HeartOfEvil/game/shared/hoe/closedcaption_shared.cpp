#include "cbase.h"
#include <KeyValues.h>
#include "filesystem.h"
#include "closedcaption_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern void Hack_FixEscapeChars( char *str );

void CClosedCaptionsInfo::Init( void )
{
	ReadCaptionsInfoFile();
}

void CClosedCaptionsInfo::Reload( void )
{
	m_CaptionsInfo.PurgeAndDeleteElements();
	m_Colors.Purge();
	ReadCaptionsInfoFile();
}

void CClosedCaptionsInfo::ReadCaptionsInfoFile( void )
{
	char szFullName[512];
	Q_snprintf(szFullName, sizeof(szFullName), "scripts/closedcaption_info.txt" );
	KeyValues *pkv = new KeyValues( "captions" );
	if ( pkv->LoadFromFile( filesystem, szFullName, "GAME" ) )
	{
		for ( KeyValues *sub = pkv->GetFirstSubKey(); sub != NULL ; sub = sub->GetNextKey() )
		{
			if ( !Q_stricmp( sub->GetName(), "_colors" ) )
			{
				ReadColors( sub );
			}
			else
			{
				ReadCaption( sub );
			}
		}
	}
	pkv->deleteThis();
}

void CClosedCaptionsInfo::ReadColors( KeyValues *pkv )
{
	for ( KeyValues *sub = pkv->GetFirstSubKey(); sub != NULL ; sub = sub->GetNextKey() )
	{
		int r = 255, g = 255, b = 255;
		const char *s = sub->GetString( "clr", "255,255,255" );
		if ( 3 != sscanf( s, "%i,%i,%i", &r, &g, &b ) ||
			( r < 0 || r > 255 ) || ( g < 0 || g > 255 ) || ( b < 0 || b > 255 ) )
		{
			DevWarning( "bogus caption color '%s'\n", s );
		}

		m_Colors.Insert( sub->GetName(), Color(r,g,b,255) );
	}
}

void CClosedCaptionsInfo::ReadCaption( KeyValues *pkv )
{
	const char *token = pkv->GetName();

	char lowercase[ 256 ];
	Q_strncpy( lowercase, token, sizeof( lowercase ) );
	Q_strlower( lowercase );
	if ( Q_strstr( lowercase, "\\" ) )
	{
		Hack_FixEscapeChars( lowercase );
	}

	int captionIndex = m_CaptionsInfo.Find( lowercase );
	if ( captionIndex != m_CaptionsInfo.InvalidIndex() )
	{
		DevWarning( "duplicate caption '%s'\n", token );
		delete m_CaptionsInfo[captionIndex];
		m_CaptionsInfo[captionIndex] = NULL;
	}

	ClosedCaptionInfo_t *info = new ClosedCaptionInfo_t;

	if ( pkv->GetInt( "exclusive", 0 ) )
		info->flags |= ClosedCaptionInfo_t::CCI_EXCLUSIVE;

	if ( pkv->GetInt( "important", 0 ) )
		info->flags |= ClosedCaptionInfo_t::CCI_IMPORTANT;

	if ( pkv->GetInt( "nosfx", 0 ) )
		info->flags |= ClosedCaptionInfo_t::CCI_NOSFX;

	info->attenuation = pkv->GetInt( "attenuation", 5 );
	if ( info->attenuation < 1 || info->attenuation > 5 )
	{
		DevWarning( "bogus caption attenuation '%d'\n", info->attenuation );
		info->attenuation = 5;
	}

	const char *s = pkv->GetString( "icon" );
	info->icon = s[0] ? strdup( s ) : NULL;

	s = pkv->GetString( "clr", "" );
	if ( s[0] )
	{
		int r = 255, g = 255, b = 255;
		if ( strchr( s, ',' ) == NULL )
		{
			int colorIndex = m_Colors.Find( s );
			if ( colorIndex == m_Colors.InvalidIndex() )
			{
				DevWarning( "undefined caption color '%s'\n", s );
			}
			else
			{
				Color c = m_Colors[colorIndex];
				r = c.r(), g = c.g(), b = c.b();
			}
		}
		else
		{
			if ( 3 != sscanf( s, "%i,%i,%i", &r, &g, &b ) ||
				( r < 0 || r > 255 ) || ( g < 0 || g > 255 ) || ( b < 0 || b > 255 ) )
			{
				DevWarning( "bogus caption color '%s'\n", s );
			}
		}
		info->clr.SetColor( r, g, b, 255 );
		info->flags |= ClosedCaptionInfo_t::CCI_COLOR;
	}

	m_CaptionsInfo.Insert( lowercase, info );
}

const ClosedCaptionInfo_t *CClosedCaptionsInfo::GetCaptionInfo( const char *token )
{
	int captionIndex = m_CaptionsInfo.Find( token );
	if ( captionIndex != m_CaptionsInfo.InvalidIndex() )
		return m_CaptionsInfo[captionIndex];
	return NULL;
}

CClosedCaptionsInfo g_ClosedCaptionsInfo;
CClosedCaptionsInfo *closedcaptionsinfo = &g_ClosedCaptionsInfo;

class CClosedCaptionsInfoGameSystem : public CAutoGameSystem
{
public:
	CClosedCaptionsInfoGameSystem() : CAutoGameSystem( "CClosedCaptionsInfoGameSystem" )
	{
	}

	virtual bool Init( void )
	{
		closedcaptionsinfo->Init();
		return true;
	}
};

static CClosedCaptionsInfoGameSystem g_ClosedCaptionsInfoGameSystem;

#ifdef CLIENT_DLL
CON_COMMAND( cc_reload_info_cl, "HOE: reload scripts/closedcaption_info.txt (client)." )
#else
CON_COMMAND( cc_reload_info_sv, "HOE: reload scripts/closedcaption_info.txt (server)." )
#endif
{
	closedcaptionsinfo->Reload();
}
