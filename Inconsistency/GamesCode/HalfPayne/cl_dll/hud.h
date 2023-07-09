/***
*
*	Copyright (c) 1999, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
//			
//  hud.h
//
// class CHud declaration
//
// CHud handles the message, calculation, and drawing the HUD
//


//#define RGB_YELLOWISH 0x00FFA000 //255,160,0
// this is rude
#define RGB_YELLOWISH 0x00C8C8C8 //200,200,200
#define RGB_REDISH 0x00FF1010 //255,160,0
#define RGB_GREENISH 0x0000A000 //0,160,0

#ifndef _WIN32
#define _cdecl 
#endif

#include "wrect.h"
#include "cl_dll.h"
#include "ammo.h"
#include <vector>
#include <memory>

#define CORNER_OFFSET 20
#define HEALTH_SPRITE_WIDTH 64
#define HEALTH_SPRITE_HEIGHT 128
#define HOURGLASS_SPRITE_WIDTH 16
#define PAINKILLER_SPRITE_WIDTH 16
#define BOTTOM_LEFT_SPACING 4

#define DHN_DRAWZERO 1
#define DHN_2DIGITS  2
#define DHN_3DIGITS  4
#define MIN_ALPHA	 100	

#define		HUDELEM_ACTIVE	1

typedef struct {
	int x, y;
} POSITION;

#include "global_consts.h"

typedef struct {
	unsigned char r,g,b,a;
} RGBA;

typedef struct cvar_s cvar_t;


#define HUD_ACTIVE	1
#define HUD_INTERMISSION 2

#define MAX_PLAYER_NAME_LENGTH		32

#define	MAX_MOTD_LENGTH				1536

//
//-----------------------------------------------------
//
class CHudBase
{
public:
	POSITION  m_pos;
	int   m_type;
	int	  m_iFlags; // active, moving, 
	virtual		~CHudBase() {}
	virtual int Init( void ) {return 0;}
	virtual int VidInit( void ) {return 0;}
	virtual int Draw(float flTime) {return 0;}
	virtual void Think(void) {return;}
	virtual void Reset(void) {return;}
	virtual void InitHUDData( void ) {}		// called every time a server is connected to

};

struct HUDLIST {
	CHudBase	*p;
	HUDLIST		*pNext;
};



//
//-----------------------------------------------------
//
#include "voice_status.h" // base voice handling class
#include "hud_spectator.h"


//
//-----------------------------------------------------
//
class CHudAmmo: public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	int Draw(float flTime);
	int DrawAimCoords();
	void Think(void);
	void Reset(void);
	int DrawWList(float flTime);
	int MsgFunc_CurWeapon(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_LckWeapon(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_WeaponList(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_AmmoX(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_AmmoPickup( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_WeapPickup( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_ItemPickup( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_HideWeapon( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_AimCoords( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_KillConf( const char *pszName, int iSize, void *pbuf );

	void SetCurrentWeaponCrosshair( int fOnTarget );
	void SetKillConfirmedCrosshair();

	void SlotInput( int iSlot );
	void _cdecl UserCmd_Slot1( void );
	void _cdecl UserCmd_Slot2( void );
	void _cdecl UserCmd_Slot3( void );
	void _cdecl UserCmd_Slot4( void );
	void _cdecl UserCmd_Slot5( void );
	void _cdecl UserCmd_Slot6( void );
	void _cdecl UserCmd_Slot7( void );
	void _cdecl UserCmd_Slot8( void );
	void _cdecl UserCmd_Slot9( void );
	void _cdecl UserCmd_Slot10( void );
	void _cdecl UserCmd_Close( void );
	void _cdecl UserCmd_NextWeapon( void );
	void _cdecl UserCmd_PrevWeapon( void );

	WEAPON *m_pWeapon; // previously private. I want to know my current weapon from WeaponsResource class

private:
	float m_fFade;
	RGBA  m_rgba;
	int	m_HUD_bucket0;
	int m_HUD_selection;

	float aimCoordX;
	float aimCoordY;
	float aimCoordZ;
	float aimCoordAngle;

	float killConfirmedTime;

};

//
//-----------------------------------------------------
//

class CHudAmmoSecondary: public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	void Reset( void );
	int Draw(float flTime);

	int MsgFunc_SecAmmoVal( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_SecAmmoIcon( const char *pszName, int iSize, void *pbuf );

private:
	enum {
		MAX_SEC_AMMO_VALUES = 4
	};

	int m_HUD_ammoicon; // sprite indices
	int m_iAmmoAmounts[MAX_SEC_AMMO_VALUES];
	float m_fFade;
};


#include "health.h"


#define FADE_TIME 100

class CHudPainkiller : public CHudBase
{
public:
	virtual int Init( void );
	virtual int VidInit( void );
	virtual int Draw( float fTime );
	int MsgFunc_PillCount( const char *pszName, int iSize, void *pbuf );

private:
	int painkillerCount;
	int painKillerSprite;
};

class CHudSlowMotion : public CHudBase
{
public:
	virtual int Init(void);
	virtual int VidInit( void );
	virtual int Draw(float fTime);
	int MsgFunc_SlowMotion(const char *pszName, int iSize, void *pbuf);

private:
	int slowMotionCharge;
	int hourglassStrokeSprite;
	int hourglassFillSprite;
};

class CHudEndCredits : public CHudBase
{
public:
	virtual int Init( void );
	virtual int VidInit( void );
	virtual void Reset( void );
	virtual int Draw( float fTime );
	int MsgFunc_EndCredits( const char *pszName, int iSize, void *pbuf );

	int	XPosition( float x, int width, int lineWidth );
	int YPosition( float y, int height );

private:
	bool ended;
	float timeStart;

	int creditSprites[10];
};

//
//-----------------------------------------------------
//
class CHudGeiger: public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	int Draw(float flTime);
	int MsgFunc_Geiger(const char *pszName, int iSize, void *pbuf);
	
private:
	int m_iGeigerRange;

};

//
//-----------------------------------------------------
//
class CHudTrain: public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	int Draw(float flTime);
	int MsgFunc_Train(const char *pszName, int iSize, void *pbuf);

private:
	SPRITE_HANDLE m_hSprite;
	int m_iPos;

};

//
//-----------------------------------------------------
//
class CHudStatusBar : public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	int Draw( float flTime );
	void Reset( void );
	void ParseStatusString( int line_num );

	int MsgFunc_StatusText( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_StatusValue( const char *pszName, int iSize, void *pbuf );

protected:
	enum { 
		MAX_STATUSTEXT_LENGTH = 128,
		MAX_STATUSBAR_VALUES = 8,
		MAX_STATUSBAR_LINES = 3,
	};

	char m_szStatusText[MAX_STATUSBAR_LINES][MAX_STATUSTEXT_LENGTH];  // a text string describing how the status bar is to be drawn
	char m_szStatusBar[MAX_STATUSBAR_LINES][MAX_STATUSTEXT_LENGTH];	// the constructed bar that is drawn
	int m_iStatusValues[MAX_STATUSBAR_VALUES];  // an array of values for use in the status bar

	int m_bReparseString; // set to TRUE whenever the m_szStatusBar needs to be recalculated

	// an array of colors...one color for each line
	float *m_pflNameColors[MAX_STATUSBAR_LINES];
};

struct extra_player_info_t 
{
	short frags;
	short deaths;
	short playerclass;
	short health; // UNUSED currently, spectator UI would like this
	bool dead; // UNUSED currently, spectator UI would like this
	short teamnumber;
	char teamname[MAX_TEAM_NAME];
};

struct team_info_t 
{
	char name[MAX_TEAM_NAME];
	short frags;
	short deaths;
	short ping;
	short packetloss;
	short ownteam;
	short players;
	int already_drawn;
	int scores_overriden;
	int teamnumber;
};

#include "player_info.h"

//
//-----------------------------------------------------
//
class CHudDeathNotice : public CHudBase
{
public:
	int Init( void );
	void InitHUDData( void );
	int VidInit( void );
	int Draw( float flTime );
	int MsgFunc_DeathMsg( const char *pszName, int iSize, void *pbuf );

private:
	int m_HUD_d_skull;  // sprite index of skull icon
};

//
//-----------------------------------------------------
//
class CHudMenu : public CHudBase
{
public:
	int Init( void );
	void InitHUDData( void );
	int VidInit( void );
	void Reset( void );
	int Draw( float flTime );
	int MsgFunc_ShowMenu( const char *pszName, int iSize, void *pbuf );

	void SelectMenuItem( int menu_item );

	int m_fMenuDisplayed;
	int m_bitsValidSlots;
	float m_flShutoffTime;
	int m_fWaitingForMore;
};

//
//-----------------------------------------------------
//
class CHudSayText : public CHudBase
{
public:
	int Init( void );
	void InitHUDData( void );
	int VidInit( void );
	int Draw( float flTime );
	int MsgFunc_SayText( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_SayText2( const char *pszName, int iSize, void *pbuf );
	void SayTextPrint( const char *pszBuf, int iBufSize, int clientIndex = -1 );
	void EnsureTextFitsInOneLineAndWrapIfHaveTo( int line );
friend class CHudSpectator;

private:

	struct cvar_s *	m_HUD_saytext;
	struct cvar_s *	m_HUD_saytext_time;
};

//
//-----------------------------------------------------
//
class CHudBattery: public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	int Draw(float flTime);
	int MsgFunc_Battery(const char *pszName,  int iSize, void *pbuf );
	
private:
	SPRITE_HANDLE m_hSprite1;
	SPRITE_HANDLE m_hSprite2;
	wrect_t *m_prc1;
	wrect_t *m_prc2;
	int	  m_iBat;	
	int	  m_iBatMax;
	float m_fFade;
	int	  m_iHeight;		// width of the battery innards
};


//
//-----------------------------------------------------
//
class CHudFlashlight: public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	int Draw(float flTime);
	void Reset( void );
	int MsgFunc_Flashlight(const char *pszName,  int iSize, void *pbuf );
	int MsgFunc_FlashBat(const char *pszName,  int iSize, void *pbuf );
	
private:
	SPRITE_HANDLE m_hSprite1;
	SPRITE_HANDLE m_hSprite2;
	SPRITE_HANDLE m_hBeam;
	wrect_t *m_prc1;
	wrect_t *m_prc2;
	wrect_t *m_prcBeam;
	float m_flBat;	
	int	  m_iBat;	
	int	  m_fOn;
	float m_fFade;
	int	  m_iWidth;		// width of the battery innards
};

//
//-----------------------------------------------------
//
const int maxHUDMessages = 16;
struct message_parms_t
{
	client_textmessage_t	*pMessage;
	float	time;
	int x, y;
	int	totalWidth, totalHeight;
	int width;
	int lines;
	int lineLength;
	int length;
	int r, g, b;
	int text;
	int fadeBlend;
	float charTime;
	float fadeTime;
};

//
//-----------------------------------------------------
//

class CHudTextMessage: public CHudBase
{
public:
	int Init( void );
	static char *LocaliseTextString( const char *msg, char *dst_buffer, int buffer_size );
	static char *BufferedLocaliseTextString( const char *msg );
	char *LookupString( const char *msg_name, int *msg_dest = NULL );
	int MsgFunc_TextMsg(const char *pszName, int iSize, void *pbuf);
};

//
//-----------------------------------------------------
//

class CHudMessage: public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	int Draw(float flTime);
	int MsgFunc_HudText(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_HudTextPro(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_GameTitle(const char *pszName, int iSize, void *pbuf);

	float FadeBlend( float fadein, float fadeout, float hold, float localTime );
	int	XPosition( float x, int width, int lineWidth );
	int YPosition( float y, int height );

	void MessageAdd( const char *pName, float time );
	void MessageAdd(client_textmessage_t * newMessage );
	void MessageDrawScan( client_textmessage_t *pMessage, float time );
	void MessageScanStart( void );
	void MessageScanNextChar( void );
	void Reset( void );

private:
	client_textmessage_t		*m_pMessages[maxHUDMessages];
	float						m_startTime[maxHUDMessages];
	message_parms_t				m_parms;
	float						m_gameTitleTime;
	client_textmessage_t		*m_pGameTitle;

	int m_HUD_title_half;
};

//
//-----------------------------------------------------
//
#define MAX_SPRITE_NAME_LENGTH	24

class CHudStatusIcons: public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	void Reset( void );
	int Draw(float flTime);
	int MsgFunc_StatusIcon(const char *pszName, int iSize, void *pbuf);

	enum { 
		MAX_ICONSPRITENAME_LENGTH = MAX_SPRITE_NAME_LENGTH,
		MAX_ICONSPRITES = 4,
	};

	
	//had to make these public so CHud could access them (to enable concussion icon)
	//could use a friend declaration instead...
	void EnableIcon( char *pszIconName, unsigned char red, unsigned char green, unsigned char blue );
	void DisableIcon( char *pszIconName );

private:

	typedef struct
	{
		char szSpriteName[MAX_ICONSPRITENAME_LENGTH];
		SPRITE_HANDLE spr;
		wrect_t rc;
		unsigned char r, g, b;
	} icon_sprite_t;

	icon_sprite_t m_IconList[MAX_ICONSPRITES];

};

//
//-----------------------------------------------------
//
class CHudBenchmark : public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	int Draw( float flTime );

	void SetScore( float score );

	void Think( void );

	void StartNextSection( int section );

	int MsgFunc_Bench(const char *pszName, int iSize, void *pbuf);

	void CountFrame( float dt );

	int GetObjects( void ) { return m_nObjects; };

	void SetCompositeScore( void );

	void Restart( void );

	int Bench_ScoreForValue( int stage, float raw );

private:
	float	m_fDrawTime;
	float	m_fDrawScore;
	float	m_fAvgScore;

	float   m_fSendTime;
	float	m_fReceiveTime;

	int		m_nFPSCount;
	float	m_fAverageFT;
	float	m_fAvgFrameRate;

	int		m_nSentFinish;
	float	m_fStageStarted;

	float	m_StoredLatency;
	float	m_StoredPacketLoss;
	int		m_nStoredHopCount;
	int		m_nTraceDone;

	int		m_nObjects;

	int		m_nScoreComputed;
	int 	m_nCompositeScore;
};

#define MESSAGE_BRIGHTENESS 200

class CHudTimer : public CHudBase
{
public:
	virtual int Init( void );
	virtual void Reset( void );
	virtual int Draw( float fTime );

	int MsgFunc_TimerDeact( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_TimerValue( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_TimerPause( const char *pszName, int iSize, void *pbuf );

private:
	int yOffset;

	std::string title;

	bool paused;

	float time;
	
	bool blinked;
	float nextTimerBlinkTime;
};

struct CounterValue {
	int count;
	int maxCount;

	std::string title;
};

class CHudCounter : public CHudBase
{
public:
	virtual int Init( void );
	virtual void Reset( void );
	virtual int Draw( float fTime );

	int MsgFunc_CountDeact( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_CountLen( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_CountOffse( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_CountValue( const char *pszName, int iSize, void *pbuf );

private:
	int yOffset;

	std::vector<CounterValue> values;
};

class CHudRandomGameplayMods : public CHudBase {
public:
	virtual int Init( void );
	virtual void Reset( void );
	virtual int Draw( float fTime );

	int MsgFunc_PropModVin( const char *pszName, int iSize, void *pbuf );

private:
	int highlightIndex;
	float timeUntilNextHighlight;

	bool ShouldDrawVotes();
	void HighlightRandomProposedMod();
};


class CHudScore : public CHudBase
{
public:
	virtual int Init( void );
	virtual void Reset( void );
	virtual int Draw( float fTime );

	int MsgFunc_ScoreDeact( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_ScoreValue( const char *pszName, int iSize, void *pbuf );

private:
	int yOffset;

	int currentScore;
	int comboMultiplier;
	float comboMultiplierReset;
};

class CHudCentralLabel : public CHudBase {
public:
	virtual int Init( void );
	virtual void Reset( void );
	virtual int Draw( float fTime );

	int MsgFunc_CLabelVal( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_CLabelGMod( const char *pszName, int iSize, void *pbuf );

private:
	float timeUntilStopDrawing;
	int alpha;

	std::string label;
	std::string subLabel;
};

typedef std::pair< std::string, float > GameLogMessage;

class CHudGameLog : public CHudBase
{
public:
	virtual int Init( void );
	virtual void Reset( void );
	virtual int Draw( float fTime );

	int MsgFunc_GLogDeact( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_GLogMsg( const char *pszName, int iSize, void *pbuf );

private:
	int yOffset;

	std::vector<GameLogMessage> messages;
};

struct GameLogWorldMessage {
	Vector coords;
	std::string message;
	std::string message2;

	float time;
	float removalTime;
	float flashTime1;
	float flashTime2;
};

class CHudGameLogWorld : public CHudBase
{
public:
	virtual int Init( void );
	virtual void Reset( void );
	virtual int Draw( float fTime );

	int MsgFunc_GLogWDeact( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_GLogWMsg( const char *pszName, int iSize, void *pbuf );

private:
	std::vector<GameLogWorldMessage> messages;
};

#define RUNTIME_SOUND_DURATION 0.072f
#define RUNTIME_UPDATE_TIME 0.02f

class CHudRunningAnimation
{
public:
	CHudRunningAnimation( float endValue, float stepFraction = 60.0f );
	void StartRunning();
	virtual int Draw( int x, int y, int r, int g, int b );

	bool isRunning;

	static float nextRuntimeSoundTime;

protected:
	float value;
	float endValue;
	float step;

	float nextUpdateTime;
};

class CHudRunningTimerAnimation : public CHudRunningAnimation {

public:
	CHudRunningTimerAnimation( float endValue, float stepFraction = 60.0f );
	virtual int Draw( int x, int y, int r, int g, int b ) override;
};

class CHudRunningScoreAnimation : public CHudRunningAnimation {

public:
	CHudRunningScoreAnimation( float endValue, float stepFraction = 60.0f );
	virtual int Draw( int x, int y, int r, int g, int b ) override;
};

struct EndScreenRunningAnimationLine {
	std::string label;
	std::unique_ptr<CHudRunningAnimation> value;

	int recordBeaten;
	std::string recordLabel;
	std::unique_ptr<CHudRunningAnimation> recordValue;
};

struct EndScreenStatisticsLine {
	std::string key;
	std::string value;
};

class CHudEndScreen : public CHudBase
{
public:
	virtual int Init( void );
	virtual void Reset( void );
	virtual int Draw( float fTime );

	int MsgFunc_EndActiv( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_EndTitle( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_EndTime( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_EndScore( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_EndStat( const char *pszName, int iSize, void *pbuf );

private:
	int cheated;
	int recordBeaten;
	std::vector< std::string> titleLines;

	std::vector<EndScreenRunningAnimationLine> animationLines;
	std::vector<EndScreenStatisticsLine> statLines;
};

//
//-----------------------------------------------------
//


class CHud
{
private:
	HUDLIST						*m_pHudList;
	SPRITE_HANDLE						m_hsprLogo;
	int							m_iLogo;
	client_sprite_t				*m_pSpriteList;
	int							m_iSpriteCount;
	int							m_iSpriteCountAllRes;
	float						m_flMouseSensitivity;
	int							m_iConcussionEffect; 

public:

	SPRITE_HANDLE						m_hsprCursor;
	float m_flTime;	   // the current client time
	float m_fOldTime;  // the time at which the HUD was last redrawn
	double m_flTimeDelta; // the difference between flTime and fOldTime
	Vector	m_vecOrigin;
	Vector	m_vecAngles;
	int		m_iKeyBits;
	int		m_iHideHUDDisplay;
	float	m_iFOV;
	int		m_Teamplay;
	int		m_iRes;
	cvar_t  *m_pCvarStealMouse;
	cvar_t	*m_pCvarDraw;

	int m_iFontHeight;
	void DrawDot( int x, int y, int r, int g, int b );
	void DrawColon( int x, int y, int r, int g, int b );
	void DrawDecimalSeparator( int x, int y, int r, int g, int b );
	int DrawHudNumber(int x, int y, int iFlags, int iNumber, int r, int g, int b );
	int DrawHudNumber( int x, int y, int iFlags, char iNumber, int r, int g, int b);
	int DrawFormattedTime( float time, int x, int y, int r, int g, int b );
	int DrawFormattedNumber( int number, int x, int y, int r, int g, int b );
	int DrawHudString(int x, int y, int iMaxX, const char *szString, int r, int g, int b );
	int DrawHudStringKeepRight( int x, int y, int iMaxX, const char *szString, int r, int g, int b );
	int DrawHudStringKeepCenter( int x, int y, int iMaxX, const char *szString, int r, int g, int b );
	int DrawHudStringReverse( int xpos, int ypos, int iMinX, const char *szString, int r, int g, int b );
	int DrawHudNumberString( int xpos, int ypos, int iMinX, int iNumber, int r, int g, int b );
	int GetNumWidth(int iNumber, int iFlags);
	int GetNumberSpriteWidth();
	int GetNumberSpriteHeight();
	int GetStringWidth( const char *string );

private:
	// the memory for these arrays are allocated in the first call to CHud::VidInit(), when the hud.txt and associated sprites are loaded.
	// freed in ~CHud()
	SPRITE_HANDLE *m_rghSprites;	/*[HUD_SPRITE_COUNT]*/			// the sprites loaded from hud.txt
	wrect_t *m_rgrcRects;	/*[HUD_SPRITE_COUNT]*/
	char *m_rgszSpriteNames; /*[HUD_SPRITE_COUNT][MAX_SPRITE_NAME_LENGTH]*/

	struct cvar_s *default_fov;
public:
	SPRITE_HANDLE GetSprite( int index ) 
	{
		return (index < 0) ? 0 : m_rghSprites[index];
	}

	wrect_t& GetSpriteRect( int index )
	{
		return m_rgrcRects[index];
	}

	
	int GetSpriteIndex( const char *SpriteName );	// gets a sprite index, for use in the m_rghSprites[] array

	CHudAmmo		m_Ammo;
	CHudHealth		m_Health;
	CHudSpectator		m_Spectator;
	CHudGeiger		m_Geiger;
	CHudBattery		m_Battery;
	CHudTrain		m_Train;
	CHudFlashlight	m_Flash;
	CHudMessage		m_Message;
	CHudStatusBar   m_StatusBar;
	CHudDeathNotice m_DeathNotice;
	CHudSayText		m_SayText;
	CHudMenu		m_Menu;
	CHudAmmoSecondary	m_AmmoSecondary;
	CHudTextMessage m_TextMessage;
	CHudStatusIcons m_StatusIcons;
	CHudBenchmark	m_Benchmark;
	CHudSlowMotion  m_SlowMotion;
	CHudPainkiller  m_Painkiller;
	CHudTimer		m_Timer;
	CHudScore		m_Score;
	CHudRandomGameplayMods m_RandomGameplayMods;
	CHudCentralLabel	m_CentralLabel;
	CHudCounter		m_Counter;
	CHudGameLog		m_GameLog;
	CHudGameLogWorld m_GameLogWorld;
	CHudEndScreen	m_EndScreen;
	CHudEndCredits  m_endCredits;

	void Init( void );
	void VidInit( void );
	void Think(void);
	int Redraw( float flTime, int intermission );
	int UpdateClientData( client_data_t *cdata, float time );

	CHud() : m_iSpriteCount(0), m_pHudList(NULL) {}  
	~CHud();			// destructor, frees allocated memory

	// user messages
	int _cdecl MsgFunc_Damage(const char *pszName, int iSize, void *pbuf );
	int _cdecl MsgFunc_GameMode(const char *pszName, int iSize, void *pbuf );
	int _cdecl MsgFunc_Logo(const char *pszName,  int iSize, void *pbuf);
	int _cdecl MsgFunc_ResetHUD(const char *pszName,  int iSize, void *pbuf);
	void _cdecl MsgFunc_InitHUD( const char *pszName, int iSize, void *pbuf );
	void _cdecl MsgFunc_ViewMode( const char *pszName, int iSize, void *pbuf );
	int _cdecl MsgFunc_SetFOV(const char *pszName,  int iSize, void *pbuf);
	int  _cdecl MsgFunc_Concuss( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_SetSkin( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_AimOffset( const char *pszName, int iSize, void *pbuf );

	// Screen information
	SCREENINFO	m_scrinfo;

	int	m_iWeaponBits;
	int	m_fPlayerDead;
	int m_iIntermission;

	float aimOffsetX;
	float aimOffsetY;

	// sprite indexes
	int m_HUD_number_0;


	void AddHudElem(CHudBase *p);

	float GetSensitivity();

};

extern CHud gHUD;

extern int g_iPlayerClass;
extern int g_iTeamNumber;
extern int g_iUser1;
extern int g_iUser2;
extern int g_iUser3;

inline void ApplyAimOffset( vec3_t &angles ) {
	angles[0] -= gHUD.aimOffsetY;
	angles[1] -= gHUD.aimOffsetX;
}