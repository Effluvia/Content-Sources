// mp3 support added by Killar

#ifndef MP3_H
#define MP3_H

#include "audiere.h"
#include "windows.h"

class CMP3
{
private:
	int		m_iIsPlaying;

public:
	int		Initialize();
	int		Shutdown();
	int		PlayMP3( const char *pszSong );
	int		PlayMP3NL( const char *pszSong );
	int		StopMP3();
};

extern CMP3 gMP3;
#endif
