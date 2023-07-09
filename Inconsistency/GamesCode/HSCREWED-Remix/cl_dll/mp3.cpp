//mp3 support added by Killar

#include "hud.h"
#include "cl_util.h"
#include "mp3.h"

using namespace audiere;

//AudioDevicePtr device;
OutputStreamPtr stream;

int CMP3::Initialize()
{
	AudioDevicePtr device(OpenDevice());
	if (!device)
	{
		gEngfuncs.Con_Printf("Audiere failed to load device!\n");
		return 0;
	}
	gEngfuncs.Con_Printf("Audiere loaded successfully!\n");
	return 1;
}

int CMP3::Shutdown()
{
	//if( m_hFMod )
	//{
	//	CLOSE();

	//	FreeLibrary( m_hFMod );
	//	m_hFMod = NULL;
	//	m_iIsPlaying = 0;
	//	return 1;
	//}
	//else
	//	return 0;
	return 1;
}

int CMP3::StopMP3( void )
{
	/*SCL( m_Stream );
	m_iIsPlaying = 0;
	return 1;*/
	return 1;
}

int CMP3::PlayMP3( const char *pszSong )
{
	//if( m_iIsPlaying )
	//{
		// sound system is already initialized
		//SCL( m_Stream );
	//} 
	//else
	//{
	//	SOP( FSOUND_OUTPUT_DSOUND );
	//	SBS( 200 );
	//	SDRV( 0 );
	//	INIT( 44100, 3, 0 ); // we need just one channel, multiple mp3s at a time would be, erm, strange...	
	//}//AJH not for really cool effects, say walking past cars in a street playing different tunes, might change this later.

	char song[256];

	sprintf( song, "%s/%s", gEngfuncs.pfnGetGameDirectory(), pszSong);

	gEngfuncs.Con_Printf("Attempting to play sound.\n");

	

	//if (SO)
	//{
	//	m_Stream = SO( song, FSOUND_NORMAL | FSOUND_LOOP_NORMAL, 0 ,0); //AJH new version fmod uses Open
	//}
	//else if( SOF )
	//{													
	//	m_Stream = SOF( song, FSOUND_NORMAL | FSOUND_LOOP_NORMAL, 1 ); //AJH old version fmod OpenFile
	//}
	//if(m_Stream)
	//{
	//	SPLAY( 0, m_Stream );
	//	m_iIsPlaying = 1;
	//	return 1;
	//}
	//else
	//{
	//	m_iIsPlaying = 0;
	//	gEngfuncs.Con_Printf("Error: Could not load %s\n",song);
	//	return 0;
	//}
	return 1;
}

int CMP3::PlayMP3NL( const char *pszSong )
{
	//if( m_iIsPlaying )
	//{
	//// sound system is already initialized
	//	SCL( m_Stream );
	//} 
	//else
	//{
	//	SOP( FSOUND_OUTPUT_DSOUND );
	//	SBS( 200 );
	//	SDRV( 0 );
	//	INIT( 44100, 3, 0 ); // we need just one channel, multiple mp3s at a time would be, erm, strange...	
	//}//AJH not for really cool effects, say walking past cars in a street playing different tunes, might change this later.

	//char song[256];

	//sprintf( song, "%s/%s", gEngfuncs.pfnGetGameDirectory(), pszSong);

	////gEngfuncs.Con_Printf("Using fmod.dll version %f\n",VER());

	//if (SO)
	//{
	//	m_Stream = SO( song, FSOUND_NORMAL | FSOUND_LOOP_OFF, 0 ,0); //AJH new version fmod uses Open
	//}
	//else if( SOF )
	//{													
	//	m_Stream = SOF( song, FSOUND_NORMAL | FSOUND_LOOP_OFF, 1 ); //AJH old version fmod OpenFile
	//}
	//if(m_Stream)
	//{
	//	SPLAY( 0, m_Stream );
	//	m_iIsPlaying = 1;
	//	return 1;
	//}
	//else
	//{
	//	m_iIsPlaying = 0;
	//	gEngfuncs.Con_Printf("Error: Could not load %s\n",song);
	//	return 0;
	//}
	return 1;
}