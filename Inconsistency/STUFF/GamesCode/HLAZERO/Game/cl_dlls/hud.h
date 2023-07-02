/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
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

#ifndef HUD_H
#define HUD_H

#include "wrect.h"
#include "cl_dll.h"
#include "hudbase.h"
//MODDD
#include "custom_message.h"
#include "ammo.h"
#include "health.h"
#include "..\game_shared\voice_status.h"
#include "hud_spectator.h"
#include "pain.h"

//MODDD - The CHudBase class and a lot of things assumed provided for CHudBase subclasses has been moved to hudbase.h.
//
//-----------------------------------------------------
//

extern float globalPSEUDO_autoDeterminedFOV;

#define FADE_TIME 100


//NOTICE - sometime in the future.
// any mentions of TeamFortressViewport should probably be replaced with 'vgui::Panel'.  Fully gut any involvement of TFC.
// Any mentions of gViewPort can probably be canned at some point too, the 'ancient' SDK never needed that.

class CHud;
//class vgui::Panel;
class TeamFortressViewport;
struct extra_player_info_t;
struct team_info_t;



extern CHud gHUD;
//extern vgui::Panel* gViewPort;
extern TeamFortressViewport* gViewPort;

//MODDD - goodbye TFC hookups
//extern int g_iPlayerClass;
extern int g_iTeamNumber;
extern int g_iUser1;
extern int g_iUser2;
extern int g_iUser3;


extern hud_player_info_t	g_PlayerInfoList[MAX_PLAYERS + 1];	   // player info from the engine
extern extra_player_info_t  g_PlayerExtraInfo[MAX_PLAYERS + 1];   // additional player info sent directly to the client dll
extern team_info_t			g_TeamInfo[MAX_TEAMS + 1];
extern int				g_IsSpectator[MAX_PLAYERS + 1];




//MODDD - mirror
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct cl_mirror_s
{
	vec3_t origin;
	int enabled;
	float radius;
	int type;
} cl_mirror_t;
////////////////////////////////////////////////////////////////////////////////////////////



//CHudAmmo moved to ammo.h.

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


//MODDD - why? Why here?
//#include "health.h"


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
	SpriteHandle_t m_SpriteHandle_t;
	int m_iPos;

};

//
//-----------------------------------------------------
//
// REMOVED: Vgui has replaced this.
//
/*
class CHudMOTD : public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	int Draw( float flTime );
	void Reset( void );

	int MsgFunc_MOTD( const char *pszName, int iSize, void *pbuf );

protected:
	static int MOTD_DISPLAY_TIME;
	char m_szMOTD[ MAX_MOTD_LENGTH ];
	float m_flActiveRemaining;
	int m_iLines;
};
*/

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
		MAX_STATUSBAR_LINES = 2,
	};

	char m_szStatusText[MAX_STATUSBAR_LINES][MAX_STATUSTEXT_LENGTH];  // a text string describing how the status bar is to be drawn
	char m_szStatusBar[MAX_STATUSBAR_LINES][MAX_STATUSTEXT_LENGTH];	// the constructed bar that is drawn
	int m_iStatusValues[MAX_STATUSBAR_VALUES];  // an array of values for use in the status bar

	int m_bReparseString; // set to TRUE whenever the m_szStatusBar needs to be recalculated

	// an array of colors...one color for each line
	float *m_pflNameColors[MAX_STATUSBAR_LINES];
};

//
//-----------------------------------------------------
//
// REMOVED: Vgui has replaced this.
//
/*
class CHudScoreboard: public CHudBase
{
public:
	int Init( void );
	void InitHUDData( void );
	int VidInit( void );
	int Draw( float flTime );
	int DrawPlayers( int xoffset, float listslot, int nameoffset = 0, char *team = NULL ); // returns the ypos where it finishes drawing
	void UserCmd_ShowScores( void );
	void UserCmd_HideScores( void );
	int MsgFunc_ScoreInfo( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_TeamInfo( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_TeamScore( const char *pszName, int iSize, void *pbuf );
	void DeathMsg( int killer, int victim );

	int m_iNumTeams;

	int m_iLastKilledBy;
	int m_fLastKillTime;
	int m_iPlayerNum;
	int m_iShowscoresHeld;

	void GetAllPlayersInfo( void );
private:
	struct cvar_s *cl_showpacketloss;

};
*/

struct extra_player_info_t 
{
	short frags;
	short deaths;
	//short playerclass;      MODDD - NO.
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
	SpriteHandle_t m_SpriteHandle_t1;
	SpriteHandle_t m_SpriteHandle_t2;

	wrect_t *m_prc1;
	wrect_t *m_prc2;

	int   m_iBat;
	float m_fFade;
	int   m_iHeight;		// width of the battery innards
	//MODDD - new sprite indexes
	int m_HUD_battery_empty;
	int m_HUD_battery_full;
	//int alphaCrossHairIndex;
	////////////////////////////////


	int Init( void );
	int VidInit( void );
	int Draw(float flTime);
	int MsgFunc_Battery(const char *pszName,  int iSize, void *pbuf );

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
	
	void drawFlashlightSidebarIcon(const int& x, const int& y);

private:
	SpriteHandle_t m_SpriteHandle_t1;
	SpriteHandle_t m_SpriteHandle_t2;
	SpriteHandle_t m_hBeam;
	wrect_t *m_prc1;
	wrect_t *m_prc2;
	wrect_t *m_prcBeam;
	float m_flBat;	
	int   m_iBat;	
	int   m_fOn;
	float m_fFade;
	int   m_iWidth;		// width of the battery innards

	//MODDD - new indexes
	int alphaFlashLightOnIndex;
	int alphaFlashLightOffIndex;

};

//
//-----------------------------------------------------
//
const int maxHUDMessages = 16;
struct message_parms_t
{
	client_textmessage_t	*pMessage;
	float time;
	int x, y;
	int totalWidth, totalHeight;
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
	int MsgFunc_GameTitle(const char *pszName, int iSize, void *pbuf);

	float FadeBlend( float fadein, float fadeout, float hold, float localTime );
	int XPosition( float x, int width, int lineWidth );
	int YPosition( float y, int height );

	void MessageAdd( const char *pName, float time );
	void MessageAdd(client_textmessage_t * newMessage );
	void MessageDrawScan( client_textmessage_t *pMessage, float time );
	void MessageScanStart( void );
	void MessageScanNextChar( void );
	void Reset( void );

private:
	client_textmessage_t		*m_pMessages[maxHUDMessages];
	float					m_startTime[maxHUDMessages];
	message_parms_t				m_parms;
	float					m_gameTitleTime;
	client_textmessage_t		*m_pGameTitle;

	int m_HUD_title_life;
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
		SpriteHandle_t spr;
		wrect_t rc;
		unsigned char r, g, b;
	} icon_sprite_t;

	icon_sprite_t m_IconList[MAX_ICONSPRITES];

};

//
//-----------------------------------------------------
//


//CHud plain
class CHud
{
private:
	HUDLIST					*m_pHudList;
	SpriteHandle_t			m_hsprLogo;

	//MODDD - OH MY
	SpriteHandle_t			m_hsprGNFOS;

	int						m_iLogo;
	client_sprite_t			*m_pSpriteList;
	int						m_iSpriteCount;
	int						m_iSpriteCountAllRes;
	float					m_flMouseSensitivity;
	int						m_iConcussionEffect; 
	
	// the memory for these arrays are allocated in the first call to CHud::VidInit(), when the hud.txt and associated sprites are loaded.
	// freed in ~CHud()
	SpriteHandle_t *m_rgSpriteHandle_ts;  /*[HUD_SPRITE_COUNT]*/    // the sprites loaded from hud.txt
	wrect_t *m_rgrcRects;	/*[HUD_SPRITE_COUNT]*/
	char *m_rgszSpriteNames; /*[HUD_SPRITE_COUNT][MAX_SPRITE_NAME_LENGTH]*/
	struct cvar_s* default_fov;

public:
	//MODDD
	int recentDamageBitmask;
	//MODDD
	float recentTime;
	int frozenMem;
	float crosshairMem;
	float allowAlphaCrosshairWithoutGunsMem;
	
	SpriteHandle_t						m_hsprCursor;
	float m_flTime;	   // the current client time
	float m_fOldTime;  // the time at which the HUD was last redrawn
	double m_flTimeDelta; // the difference between flTime and fOldTime
	Vector	m_vecOrigin;
	Vector	m_vecAngles;
	int	m_iKeyBits;
	int	m_iHideHUDDisplay;

	//MODDD - was "m_iFOV", renamed to "m_iPlayerFOV".
	// To differentiate it from the player class's (dlls/player.h) own "m_iFOV", which can also occur
	// since player.h is shared.
	int	m_iPlayerFOV;

	int	m_Teamplay;
	int	m_iRes;
	cvar_t  *m_pCvarStealMouse;
	cvar_t	*m_pCvarDraw;

	int m_iFontWidth;
	int m_iFontHeight;
	int m_iFontWidthAlt;
	int m_iFontHeightAlt;

	CHudAmmo		m_Ammo;
	CHudHealth		m_Health;
	CHudSpectator	m_Spectator;
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
	//MODDD - new
	CHudPain m_Pain;


	// Screen information
	SCREENINFO m_scrinfo;

	float PESUDO_cameraModeMem;
	//MODDD - when the weapon-select is on, bottom-most stats (health, battery, ammo) are not drawn.
	bool canDrawBottomStats;

	int m_iWeaponBits;
	int m_fPlayerDead;
	int m_iIntermission;

	// sprite indexes
	int m_HUD_number_0;

	//MODDD - added
	int m_HUD_number_0_health;
	int m_HUD_number_1_tiny;

	int m_HUD_e_number_0;
	int m_HUD_e_number_0_health;

	//MODDD - altgui
	int m_HUD_number_0_E3R;
	int m_HUD_battery_empty_E3;
	int m_HUD_battery_full_E3;

	int m_HUD_brokentransparency;
	int brokenTransWidth;
	int brokenTransHeight;
	wrect_t* m_prc_brokentransparency;
	int m_HUD_brokentransparency0;
	int m_HUD_brokentransparencyw;
	int m_glockSilencerWpnIcoActive;
	int m_glockSilencerWpnIcoInactive;
	int alphaCrossHairIndex;

	//MODDDMIRROR
	////////////////////////////////////////////////////////////////////////////////////////////////////////
	Vector m_vecSkyPos; //LRC
	int	m_iSkyMode;  //LRC
	int	m_iSkyScale;  //AJH Allows parallax for the sky. 0 means no parallax, i.e infinitly large & far away.
	int	m_iCameraMode;//G-Cont. clipping thirdperson camera
	int m_iLastCameraMode;//save last mode

	int viewEntityIndex; // for trigger_viewset
	int viewFlags;
	cl_mirror_t Mirrors[MIRROR_MAX]; //Limit - 32 mirrors!   CHANGE - now "MIRROR_MAX", a macro.
	int numMirrors;
	/////////////////////////////////////////////////////////////////////////////////////////////////
	


	int canDrawSidebar(void);
	
	//MODDD - new filter for drawing images.
	void drawAdditiveFilter(int sprite, const int& r, const int& g, const int& b, int huh, int x, int y, wrect_t* rect);
	void drawAdditiveFilter(int sprite, const int& r, const int& g, const int& b, int huh, int x, int y, wrect_t* rect, const int& canDrawBrokenTrans);

	void drawPartialFromBottom(const SpriteHandle_t & arg_sprite, const wrect_t* arg_rect, const float arg_portion, const int & x, const int & y,  int & r, int & g, int & b);
	void drawPartialFromBottom(const SpriteHandle_t & arg_sprite, const wrect_t* arg_rect, const float arg_portion, const int & x, const int & y,  int & r, int & g, int & b, const int& canDrawBrokenTrans);

	void drawPartialFromLeft(const SpriteHandle_t & arg_sprite, const wrect_t* arg_rect, const float arg_portion, const int & x, const int & y,  int & r, int & g, int & b);
	void drawPartialFromLeft(const SpriteHandle_t & arg_sprite, const wrect_t* arg_rect, const float arg_portion, const int & x, const int & y,  int & r, int & g, int & b, const int& canDrawBrokenTrans);
	void drawPartialFromRight(const SpriteHandle_t & arg_sprite, const wrect_t* arg_rect, const float arg_portion, const int & x, const int & y,  int & r, int & g, int & b);
	void drawPartialFromRight(const SpriteHandle_t & arg_sprite, const wrect_t* arg_rect, const float arg_portion, const int & x, const int & y,  int & r, int & g, int & b, const int& canDrawBrokenTrans);

	void attemptDrawBrokenTrans(int arg_startx, int arg_starty, wrect_t* rect);
	void attemptDrawBrokenTransLight(int arg_startx, int arg_starty, wrect_t* rect);
	void attemptDrawBrokenTransWhite(int arg_startx, int arg_starty, wrect_t* rect);
	void attemptDrawBrokenTransLightAndWhite(int arg_startx, int arg_starty, wrect_t* rect);
	
	void attemptDrawBrokenTrans(int arg_startx, int arg_starty, int arg_width, int arg_height);
	void attemptDrawBrokenTransLight(int arg_startx, int arg_starty, int arg_width, int arg_height);
	void attemptDrawBrokenTransWhite(int arg_startx, int arg_starty, int arg_width, int arg_height);
	void attemptDrawBrokenTransLightAndWhite(int arg_startx, int arg_starty, int arg_width, int arg_height);
	
	void playWeaponSelectMoveSound();

	int DrawHudNumber(int x, int y, int iFlags, int iNumber, const int& r, const int& g, const int& b );
	//MODDD - additional argument for "DrawHudNumber" : "useBoxedNumber".
	int DrawHudNumber(int x, int y, int iFlags, int iNumber, const int& r, const int& g, const int& b, int fontID );
	int DrawHudNumber(int x, int y, int iFlags, int iNumber, const int& r, const int& g, const int& b, int fontID, const int& canDrawBrokenTrans);

	int DrawHudString(int x, int y, int iMaxX, char *szString, int r, int g, int b );
	int DrawHudStringReverse( int xpos, int ypos, int iMinX, char *szString, int r, int g, int b );
	
	int DrawHUDNumber_widthOnly(int iFlags, int iNumber, int fontID);
	int DrawHudNumberString( int xpos, int ypos, int iMinX, int iNumber, int r, int g, int b );
	int GetNumWidth(int iNumber, int iFlags);

	
public:
	SpriteHandle_t GetSprite( int index ) 
	{
		return (index < 0) ? 0 : m_rgSpriteHandle_ts[index];
	}

	wrect_t& GetSpriteRect( int index )
	{
		return m_rgrcRects[index];
	}

	
	int GetSpriteIndex( const char *SpriteName );	// gets a sprite index, for use in the m_rgSpriteHandle_ts[] array

	void Init( void );
	void VidInit( void );
	void Think(void);
	int Redraw( float flTime, int intermission );
	int UpdateClientData( client_data_t *cdata, float time );

	//MODDD - constructor moved to implementation (hud.cpp).
	//CHud() : m_iSpriteCount(0), m_pHudList(NULL) {}  
	CHud();
	~CHud();			// destructor, frees allocated memory

	// user messages
	//MODDD - removed.  See notes in hud_msg.cpp.  In short, unhooked/ignored message.
	//int _cdecl MsgFunc_Damage(const char *pszName, int iSize, void *pbuf );

	// ALSO, these implementations since moved to custom_message.cpp instead.
	int _cdecl MsgFunc_GameMode(const char *pszName, int iSize, void *pbuf );
	int _cdecl MsgFunc_Logo(const char *pszName,  int iSize, void *pbuf);
	int _cdecl MsgFunc_ResetHUD(const char *pszName,  int iSize, void *pbuf);
	int _cdecl MsgFunc_InitHUD( const char *pszName, int iSize, void *pbuf );
	int _cdecl MsgFunc_ViewMode( const char *pszName, int iSize, void *pbuf );
	int _cdecl MsgFunc_SetFOV(const char *pszName,  int iSize, void *pbuf);
	int _cdecl MsgFunc_Concuss( const char *pszName, int iSize, void *pbuf );
	//MODDD - from health.h
	int _cdecl MsgFunc_Damage(const char* pszName, int iSize, void* pbuf);
	int _cdecl MsgFunc_Drowning(const char* pszName, int iSize, void* pbuf);

	//MODDD - new place for utility methods across the GUI:
	void getGenericGUIColor(int &r, int &g, int &b);
	void getGenericEmptyColor(int &r, int &g, int &b);
	void getGenericRedColor(int &r, int &g, int &b);
	void getGenericOrangeColor(int &r, int &g, int &b);
	void getGenericGreenColor(int &r, int &g, int &b);
	
	void AddHudElem(CHudBase *p);
	float GetSensitivity();

	//MODDD - ALSO, complementary method to go along player.h's "getBaseFOV", using our raw
	// access to the player's CVars instead of the player's serverside info.
	inline float getPlayerBaseFOV(void) {
		if (EASY_CVAR_GET_CLIENTONLY(auto_adjust_fov) == 0) {
			// don't use the auto one then.
			return EASY_CVAR_GET(default_fov);
		}
		else {
			// use the one related to screensize.
			return globalPSEUDO_autoDeterminedFOV;
		}
	}//END OF getBaseFOV

};



#endif //END OF HUD_H




